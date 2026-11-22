/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Zhur
    Rewrite:    Allan
*/


#include "../eve-server.h"

#include "Client.h"
#include "EntityMgr.h"
#include "EVEServerConfig.h"
#include "StaticDataMgr.h"
#include "fleet/FleetService.h"
#include "inventory/AttributeEnum.h"
#include "map/MapDB.h"
#include "npc/NPC.h"
#include "system/BallPark.h"
#include "system/Container.h"
#include "system/Damage.h"
#include "system/Squad.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/AnomalyMgr.h"


NPC::NPC(InventoryItemRef self, PyServiceMgr& services, SystemManager* system, const FactionData& data, SpawnMgr* spawnMgr)
: DynamicSpaceEntity(self, services, system),
m_AI(new NPCAI(this)),
m_squad(nullptr),
m_spawnMgr(spawnMgr),
m_orbitingID(0),
m_rank(Rank::None),
m_moduleCount(0),
m_hauler(false)
{
    m_allyID = data.allianceID;
    m_warID = data.factionID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;

    // Create default dynamic attributes in the AttributeMgr:
    m_self->SetAttribute(AttrInertiaMod,          EvilOne, false);
    m_self->SetAttribute(AttrDamage,              EvilZero, false);
    m_self->SetAttribute(AttrArmorDamage,         EvilZero, false);
    m_self->SetAttribute(AttrMass,                m_self->type().mass(), false);
    m_self->SetAttribute(AttrRadius,              m_self->type().radius(), false);
    m_self->SetAttribute(AttrVolume,              m_self->type().volume(), false);
    m_self->SetAttribute(AttrCapacity,            m_self->type().capacity(), false);
    m_self->SetAttribute(AttrShieldCharge,        m_self->GetAttribute(AttrShieldCapacity), false);
    m_self->SetAttribute(AttrCapacitorCharge,     m_self->GetAttribute(AttrCapacitorCapacity), false);

    // get 'module' count for this npc;  i dont know where i saw data for this, but cant find it now
    uint8 mininum = 0; //m_self->GetAttribute(AttrCapacitorCapacity).get_uint32();
    uint8 maximum = 5; //m_self->GetAttribute(AttrCapacitorCapacity).get_uint32();
    m_moduleCount = MakeRandomUInt(mininum, maximum);

    _log(NPC__TRACE, "Created NPC object for %s (%u) - %u Modules (%u/%u), Data: O:%u, C:%u, A:%u, W:%u", \
            m_self->name(), m_self->itemID(), m_moduleCount, mininum, maximum, \
            m_ownerID, m_corpID, m_allyID, m_warID);
}

NPC::~NPC() {
    SafeDelete(m_AI);
}

bool NPC::Load() {
    SetResists();

    // load data for AI
    m_AI->Init();

    CreateMoveData();

    //dSE::Load() just returns true for now (no code)
    return DynamicSpaceEntity::Load();
}

void NPC::Process() {
    if (m_killed)
        return;

    const double profileStartTime = (sConfig.debug.UseProfiling ? GetProfileTime() : 0.0);

    // if npc is warping, do not call ai::process
    if (m_moveData->GetBallMode() != Ball::Mode::WARP)
        m_AI->Process();

    // make random chance to reset buffs (wip)

    // do other shit later here if needed.

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::Type::npc, GetProfileTime() - profileStartTime);
}

void NPC::Orbit(SpaceEntity *pTargetSE) {
    if (pTargetSE == nullptr) {
        m_orbitingID = 0;
    } else {
        m_orbitingID = pTargetSE->GetID();
    }
}

void NPC::UseShieldRecharge() {
    if (!sConfig.npc.UseRegen) {
        m_AI->DisableRepTimers(true, false);
        return;
    }
    if (!m_self->HasAttribute(AttrEntityShieldBoostAmount)) {
        m_AI->DisableRepTimers(true, false);
        return;
    }
    // We recharge our shield until it's full.
    float shieldCharge = m_self->GetAttribute(AttrShieldCharge).get_float();

    if (m_self->GetAttribute(AttrShieldCapacity) > (shieldCharge - 0.1f)) {
        shieldCharge += m_self->GetAttribute(AttrEntityShieldBoostAmount).get_float();
        if (shieldCharge > m_self->GetAttribute(AttrShieldCapacity).get_float())
            shieldCharge = m_self->GetAttribute(AttrShieldCapacity).get_float();
        m_self->SetAttribute(AttrShieldCharge, shieldCharge);
    } else {
        m_AI->DisableRepTimers(true, false);
    }

    UpdateDamage();
}

void NPC::UseArmorRepairer() {
    if (!sConfig.npc.UseRepair) {
        m_AI->DisableRepTimers(false, true);
        return;
    }
    if (!m_self->HasAttribute(AttrEntityArmorRepairAmount)) {
        m_AI->DisableRepTimers(false, true);
        return;
    }

    float armorDamage(0.0f);
    if (m_self->GetAttribute(AttrArmorDamage) > 0) {
        armorDamage -= m_self->GetAttribute(AttrEntityArmorRepairAmount).get_float();
        if (armorDamage < 0.0f)
            armorDamage = 0.0f;
        m_self->SetAttribute(AttrArmorDamage, armorDamage);
    } else {
        m_AI->DisableRepTimers(false, true);
    }

    UpdateDamage();
}

// do npcs warpto anything other than a point?
void NPC::WarpTo(SystemBubble* pBubble, Vector3d destPoint, int32 distance) {
    MoveData* mData = GetMoveData();
    if (destPoint.isZero()) {
        BallPark::Stop(mData);
        sLog.Error("NPC::WarpTo()", "DestPoint is zero and pSE is null.");
        return;
    }

    mData->massive = false;
    mData->warpState.Reset();

    Vector3d warp_vector = destPoint - mData->mySE->GetPosition();
    int64 dist = warp_vector.Length();
    // expand normal to avoid dupe sqrt since we have dist already
    const double invDist = 1.0 / dist;
    Vector3d head = (warp_vector * invDist);

    // required warp data
    mData->SetHeading(head);
    mData->SetDistance(distance);
    mData->SetTargetDistance(dist);
    mData->SetTargetPoint(destPoint);
    mData->warpState.targBubble = pBubble;

    BallPark::WarpTo(mData);

    if (is_log_enabled(NPC__MESSAGE))
        _log(NPC__MESSAGE, "NPC::WarpTo() %s(%u) %u -> %u, targetPoint: %0.2f,%0.2f,%0.2f  stop distance: %i  targetDistance: %llim", \
                GetName(), GetID(), SysBubble()->GetID(), mData->warpState.targBubble->GetID(), \
                mData->targetPoint.x, mData->targetPoint.y, mData->targetPoint.z, distance, mData->targetDistance);
}

void NPC::SetResists() {
    /* fix for missing resist attribs -allan 18April16  */
    // Shield Resonance
    if (!m_self->HasAttribute(AttrShieldEmDamageResonance))
        m_self->SetAttribute(AttrShieldEmDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrShieldExplosiveDamageResonance))
        m_self->SetAttribute(AttrShieldExplosiveDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrShieldKineticDamageResonance))
        m_self->SetAttribute(AttrShieldKineticDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrShieldThermalDamageResonance))
        m_self->SetAttribute(AttrShieldThermalDamageResonance, EvilOne, false);
    // Armor Resonance
    if (!m_self->HasAttribute(AttrArmorEmDamageResonance))
        m_self->SetAttribute(AttrArmorEmDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrArmorExplosiveDamageResonance))
        m_self->SetAttribute(AttrArmorExplosiveDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrArmorKineticDamageResonance))
        m_self->SetAttribute(AttrArmorKineticDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrArmorThermalDamageResonance))
        m_self->SetAttribute(AttrArmorThermalDamageResonance, EvilOne, false);
    // Hull Resonance
    if (!m_self->HasAttribute(AttrEmDamageResonance))
        m_self->SetAttribute(AttrEmDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrExplosiveDamageResonance))
        m_self->SetAttribute(AttrExplosiveDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrKineticDamageResonance))
        m_self->SetAttribute(AttrKineticDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrThermalDamageResonance))
        m_self->SetAttribute(AttrThermalDamageResonance, EvilOne, false);
}

void NPC::EncodeStateData(Buffer& into) {
    MoveData* mData = GetMoveData();
    using namespace Destiny;
    uint8 mode = mData->GetBallMode(); //Ball::Mode::STOP;
    BallHeader head = BallHeader();
        head.entityID = GetID();
        head.mode = mode;
        head.radius = GetRadius();
        head.posX = x();
        head.posY = y();
        head.posZ = z();
        head.flags = Ball::Flag::IsFree;
        if (mode != Ball::Mode::WARP)
            head.flags = head.flags | Ball::Flag::IsMassive;
    into.Append(head);
    MassSector mass = MassSector();
        mass.mass = m_self->mass();
        mass.cloak = mData->IsCloaked();
        mass.harmonic = m_harmonic;
        mass.corporationID = m_corpID;
        mass.allianceID = (IsAllianceID(m_allyID) ? m_allyID : -1);
    into.Append(mass);
    Vector3d& velocity = mData->myVelocity;
    DataSector data = DataSector();
        data.velX = velocity.x;
        data.velY = velocity.y;
        data.velZ = velocity.z;
        data.maxSpeed = mData->GetMaxSpeed();
        data.inertia = m_self->GetAttribute(AttrInertiaMod).get_float();
        data.speedfraction = mData->GetASF();
    into.Append(data);
    switch (mode) {
        case Ball::Mode::WARP: {
            const Vector3d& target = mData->GetTargetPoint();
            WARP_Struct warp;
            warp.formationID = -1;  // fixme after squads implemented
            warp.targX = target.x;
            warp.targY = target.y;
            warp.targZ = target.z;
            warp.warpFactor = mData->warpState.warpSpeedAU * 10;    //ship warp speed in AU x10  (x * 0.1 in client)
            // warp timing and data.   see Ship::EncodeStateData() for notes
            if (mData->warpState.active) {
                warp.effectStamp    = mData->GetTicStamp();
                warp.warpDistance   = mData->GetTargetDistance();
                warp.minRange       = mData->GetDesiredDistance();
            } else {
                // these are correct 14Nov26
                warp.effectStamp    = -1;
                warp.warpDistance   = -1.0;
                warp.minRange       = 0.0;
            }
            into.Append(warp);
        }  break;
        case Ball::Mode::FOLLOW: {
            FOLLOW_Struct follow;
            follow.formationID = -1;
            follow.followID = mData->GetTargetSE()->GetID();
            follow.followRange = mData->GetDesiredDistance();
            into.Append(follow);
        }  break;
        case Ball::Mode::ORBIT: {
            ORBIT_Struct orbit;
            orbit.formationID = -1;
            orbit.targetID = mData->GetTargetSE()->GetID();
            orbit.followRange = mData->GetDesiredDistance();
            into.Append(orbit);
        }  break;
        case Ball::Mode::GOTO: {
            Vector3d targPt = mData->GetTargetPoint();
            GOTO_Struct go;
                go.formationID = -1;
                go.x = targPt.x;
                go.y = targPt.y;
                go.z = targPt.z;
            into.Append(go);
        }  break;
        case Ball::Mode::FORMATION: {
            // this implies squad
            assert (m_squad != nullptr);
            FORMATION_Struct form;
            form.formationID = m_squad->GetFormID();
            form.leaderID = m_squad->GetLeader()->GetID();
            form.spacing = m_squad->GetSpacing();
            form.slotID = 1;
            into.Append(form);
        }  break;
        default: {
            STOP_Struct main;
            main.formationID = -1;
            into.Append(main);
        } break;
    }

    if (is_log_enabled(SE__DESTINY))
        _log(SE__DESTINY, "NPC::EncodeStateData(): %s - id:%lli, mode:%s, flags:0x%X, Vel:%.1f, %.1f, %.1f", \
                GetName(), head.entityID, sDataMgr.GetBallModeName(mode), head.flags, data.velX, data.velY, data.velZ);
}

void NPC::Killed(Damage &fatal_blow) {
    if ((m_bubble == nullptr) or (m_system == nullptr))
        return; // make error here?

    // remove from bubble's gfx map (may/may not be in here)
    m_bubble->RemoveNPC(this);

    //notify our spawn manager that we are gone.
    if ((m_spawnMgr != nullptr) and (m_self.get() != nullptr))
        m_spawnMgr->SpawnKilled(m_bubble, m_self->itemID());

    //  log faction kill in dynamic data
    uint32 locationID(GetLocationID());
    MapDB::AddKill(locationID);
    MapDB::AddFactionKill(locationID);

    // set killer info
    uint32 killerID = 0;
    Client* pClient(nullptr);
    SpaceEntity* killer(fatal_blow.srcSE);

    if (killer->HasPilot()) {
        pClient = killer->GetPilot();
        killerID = pClient->GetCharacterID();
    } else if (killer->IsDroneSE()) {
        pClient = sEntityMgr.FindClientByCharID(killer->GetSelf()->ownerID());
        if (pClient == nullptr) {
            sLog.Error("NPC::Killed()", "killer == IsDrone and pPlayer == nullptr");
        } else {
            killerID = pClient->GetCharacterID();
        }
    } else {
        killerID = killer->GetID();
    }

    if (pClient != nullptr) {
        //award kill bounty.
        AwardBounty( pClient );
        if (m_system->GetSecurityRating() > 0) {
            if (pClient->InFleet()) {
                // also distribute to fleet members in local space
                std::vector<Client*> mVec;
                sFltSvc.GetMemeberVec(pClient->GetFleetID(), mVec);
                for (auto &cur : mVec) {
                    if (cur->IsInSpace()) {
                        if (cur->GetShipSE()->SysBubble()->GetID() == pClient->GetShipSE()->SysBubble()->GetID()) {
                            AwardSecurityStatus(m_self, cur->GetChar().get());
                        }
                    }
                }
            } else {
                AwardSecurityStatus(m_self, pClient->GetChar().get());  // this awards secStatusChange for npcs in empire space
            }
        }
    }

    Vector3d wreckPosition = m_self->position();
    if (wreckPosition.isNaN()) {
        sLog.Error("NPC::Killed()", "Wreck Position is NaN");
        return;
    }
    uint32 wreckTypeID(sDataMgr.GetWreckID(m_self->typeID()));
    if (!IsWreckTypeID(wreckTypeID)) {
        sLog.Error("NPC::Killed()", "Could not get wreckType for %s of type %u", m_self->name(), m_self->typeID());
        // default to generic frigate wreck till i get better checks and/or complete wreck data
        wreckTypeID = 26557;
    }

    std::string wreck_name = m_self->itemName();
    wreck_name += " Wreck";
    ItemData wreckItemData(wreckTypeID, killerID, locationID, flagAutoFit, wreck_name.c_str(), wreckPosition, itoa(m_allyID));
    WreckContainerRef wreckItemRef = sItemFactory.SpawnWreckContainer(wreckItemData);
    if (wreckItemRef.get() == nullptr) {
        sLog.Error("NPC::Killed()", "Creating Wreck Item Failed for %s of type %u", wreck_name.c_str(), wreckTypeID);
        return;
    }

    if (is_log_enabled(PHYSICS__TRACE))
        _log(PHYSICS__TRACE, "NPC::Killed() - NPC %s(%u) Position: %.2f,%.2f,%.2f.  Wreck %s(%u) Position: %.2f,%.2f,%.2f.", \
                GetName(), GetID(), x(), y(), z(), wreckItemRef->name(), wreckItemRef->itemID(), wreckPosition.x, wreckPosition.y, wreckPosition.z);

    // need to determine if this is a hauler, and get size for appropriate loot
    if ((MakeRandomFloat() < sConfig.npc.LootDropChance) or (m_allyID == factionUnknown) or m_hauler)
        DropLoot(wreckItemRef, m_self->groupID(), killerID);

    DBSystemDynamicEntity wreckData = DBSystemDynamicEntity();
        wreckData.allianceID = (killer->GetAllianceID() == 0 ? m_allyID : killer->GetAllianceID());
        wreckData.categoryID = EVEDB::invCategories::Celestial;
        wreckData.corporationID = killer->GetCorporationID();
        wreckData.factionID = (killer->GetWarFactionID() == 0 ? m_warID : killer->GetWarFactionID());
        wreckData.groupID = EVEDB::invGroups::Wreck;
        wreckData.itemID = wreckItemRef->itemID();
        wreckData.itemName = std::move(wreck_name);
        wreckData.ownerID = killerID;
        wreckData.typeID = wreckTypeID;
        wreckData.position = wreckPosition;

    if (!m_system->BuildDynamicEntity(wreckData, m_self->itemID())) {
        sLog.Error("NPC::Killed()", "Spawning Wreck Failed for typeID %u", wreckTypeID);
        wreckItemRef->Delete();
        return;
    }
    m_system->SendJettisonPacket(this);
}

void NPC::CmdDropLoot()
{
    m_system->SendJettisonPacket(this);
    /** @todo finish this */
    //DropLoot(wreckItemRef, m_self->groupID());
}

void NPC::ApplyTrackingBoost(float mod/*1.0f*/) {

}
