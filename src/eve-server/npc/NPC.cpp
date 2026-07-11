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
#include "system/Container.h"
#include "system/Damage.h"
#include "system/DestinyManager.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/AnomalyMgr.h"


NPC::NPC(InventoryItemRef self, PyServiceMgr& services, SystemManager* system, const FactionData& data, SpawnMgr* spawnMgr)
: DynamicSystemEntity(self, services, system),
m_AI(new NPCAIMgr(this)),
m_spawnMgr(spawnMgr),
m_squad(nullptr),
m_hauler(false),
m_squadLeader(false),
m_moduleCount(0),
m_rank(0),
m_orbitingID(0)
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

    // get 'module' count for this npc
    uint8 mininum = m_self->GetAttribute(AttrCapacitorCapacity).get_uint32();
    uint8 maximum = m_self->GetAttribute(AttrCapacitorCapacity).get_uint32();
    m_moduleCount = MakeRandomUInt(mininum, maximum);

    _log(NPC__TRACE, "Created NPC object for %s (%u) - %u Modules (%u/%u), Data: O:%u, C:%u, A:%u, W:%u", \
            m_self.get()->name(), m_self.get()->itemID(), m_moduleCount, mininum, maximum, \
            m_ownerID, m_corpID, m_allyID, m_warID);
}

NPC::~NPC() {
    SafeDelete(m_AI);
}

bool NPC::Load() {
    m_destiny->UpdateShipVariables();
    SetResists();

    // load data
    m_AI->Init();

    //dSE::Load() just returns true for now (no code)
    return DynamicSystemEntity::Load();
}

void NPC::Process() {
    if (m_killed)
        return;

    double profileStartTime = GetTimeUSeconds();

    /*  Process AI before moving */
    m_AI->Process();

    /*   Base call to Process Movement  */
    SystemEntity::Process();

    // make random chance to reset buffs (wip)

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::npc, GetTimeUSeconds() - profileStartTime);
}

void NPC::Orbit(SystemEntity *pTargetSE) {
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

void NPC::EncodeDestiny(Buffer& into)
{
    using namespace Destiny;

    uint8 mode = m_destiny->GetBallMode(); //Ball::Mode::STOP;

    BallHeader head = BallHeader();
        head.entityID = GetID();
        head.mode = mode;
        head.radius = GetRadius();
        head.posX = x();
        head.posY = y();
        head.posZ = z();
        head.flags = Ball::Flag::IsMassive | Ball::Flag::IsFree;
    into.Append( head );
    MassSector mass = MassSector();
        mass.mass = m_self->mass();
        mass.cloak = (m_destiny->IsCloaked() ? 1 : 0);
        mass.harmonic = m_harmonic;
        mass.corporationID = m_corpID;
        mass.allianceID = (IsAllianceID(m_allyID) ? m_allyID : -1);
    into.Append( mass );
    DataSector data = DataSector();
        data.maxSpeed = m_destiny->GetMaxVelocity();
        data.velX = m_destiny->GetVelocity().x;
        data.velY = m_destiny->GetVelocity().y;
        data.velZ = m_destiny->GetVelocity().z;
        data.inertia = m_self->GetAttribute(AttrInertiaMod).get_float();
        data.speedfraction = m_destiny->GetSpeedFraction();
    into.Append( data );
    switch (mode) {
        case Ball::Mode::WARP: {
            GPoint target = m_destiny->GetTargetPoint();
            WARP_Struct warp;
            warp.formationID = 0xFF;
            warp.targX = target.x;
            warp.targY = target.y;
            warp.targZ = target.z;
            warp.speed = m_destiny->GetWarpSpeed();       //ship warp speed x10  (dont ask...this is what it is...more dumb ccp shit)
            // warp timing.  see Ship::EncodeDestiny() for notes/updates
            if (m_destiny->IsWarping()) {
                warp.effectStamp = m_destiny->GetStateStamp();   //timestamp when warp started
                warp.distance = -1.0;
                warp.trackingFlags = 23000.0;
            } else {
                warp.effectStamp = -1;
                warp.distance = 0;
                warp.trackingFlags = 0.0;   //4802252820405690112
            }
            into.Append( warp );
        }  break;
        case Ball::Mode::FOLLOW: {
            FOLLOW_Struct follow;
            follow.followID = m_destiny->GetTargetID();
            follow.followRange = m_destiny->GetFollowDistance();
            follow.formationID = 0xFF;
            into.Append( follow );
        }  break;
        case Ball::Mode::ORBIT: {
            ORBIT_Struct orbit;
            orbit.targetID = m_destiny->GetTargetID();
            orbit.followRange = m_destiny->GetFollowDistance();
            orbit.formationID = 0xFF;
            into.Append( orbit );
        }  break;
        case Ball::Mode::GOTO: {
            GPoint target = m_destiny->GetTargetPoint();
            GOTO_Struct go;
            go.formationID = 0xFF;
            go.x = target.x;
            go.y = target.y;
            go.z = target.z;
            into.Append( go );
        }  break;
        case Ball::Mode::FORMATION: {
            // this implies squad
            assert (m_squad != nullptr);
            FORMATION_Struct form;
            form.formationID = m_squad->GetFormID();
            form.leaderID = m_squad->GetLeader()->GetID();
            form.spacing = m_squad->GetSpacing();
            form.syncIndex = 1;
            into.Append(form);
        }  break;
        default: {
            STOP_Struct main;
            main.formationID = 0xFF;
            into.Append( main );
        } break;
    }

    std::string modeStr = "Goto";
    switch (mode) {
        case 1: modeStr = "Follow"; break;
        case 2: modeStr = "Stop"; break;
        case 3: modeStr = "Warp"; break;
        case 4: modeStr = "Orbit"; break;
        case 5: modeStr = "Missile"; break;
        case 6: modeStr = "Mushroom"; break;
        case 7: modeStr = "Boid"; break;
        case 8: modeStr = "Troll"; break;
        case 9: modeStr = "Miniball"; break;
        case 10: modeStr = "Field"; break;
        case 11: modeStr = "Rigid"; break;
        case 12: modeStr = "Formation"; break;
    }

    _log(SE__DESTINY, "NPC::EncodeDestiny(): %s - id:%lli, mode:%s, flags:0x%X, Vel:%.1f, %.1f, %.1f", \
            GetName(), head.entityID, modeStr.c_str(), head.flags, data.velX, data.velY, data.velZ);
}

void NPC::Killed(Damage &fatal_blow) {
    if ((m_bubble == nullptr) or (m_destiny == nullptr) or (m_system == nullptr))
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
    uint32 killerID(0);
    Client* pClient(nullptr);
    SystemEntity* killer(fatal_blow.srcSE);

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

    GPoint wreckPosition(m_self->position());
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
    WreckContainerRef wreckItemRef = sItemFactory.SpawnWreckContainer( wreckItemData );
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
    m_destiny->SendJettisonPacket();
}

void NPC::CmdDropLoot()
{
    m_destiny->SendJettisonPacket();
    /** @todo finish this */
    //DropLoot(wreckItemRef, m_self->groupID());
}

void NPC::ApplyTrackingBoost(float mod/*1.0f*/) {

}


void NPCSquad::RegisterMember(NPC* pNPC) {
    if (!pNPC)
        return;
    m_members.push_back(pNPC);

    pNPC->SetSquad(this);

    // Dynamic Promotion Engine
    uint8 newRank = pNPC->GetCommandRank();

    if (m_squadLeader == nullptr) {
        // First ship on grid is leader by default
        m_squadLeader = pNPC;
        pNPC->SetSquadLeader(true);
    } else if (newRank > m_squadLeader->GetCommandRank()) {
        // A heavier tactical hull just dropped out of warp! Demote the old leader cleanly
        m_squadLeader->SetSquadLeader(false);
        // Promote the heavy reinforcement hull
        m_squadLeader = pNPC;
        pNPC->SetSquadLeader(true);

        _log(NPC__AI_MESSAGE, "Squad ID %u Hierarchy Shift: Heavy hull %u has assumed tactical fleet command.",
             m_squadID, pNPC->GetID());
    }
}

void NPCSquad::UnregisterMember(NPC* pNPC) {
    if (!pNPC)
        return;

    // 1. Linearly scan our vector stack to find and erase the dead pointer
    auto it = std::find(m_members.begin(), m_members.end(), pNPC);
    if (it != m_members.end()) {
        m_members.erase(it);
    }

    // 2. SQUAD LEADER RE-ALLOCATION
    if (m_squadLeader == pNPC) {
        m_squadLeader = nullptr;
        if (!m_members.empty()) {
            m_squadLeader = m_members.front();
            _log(NPC__AI_MESSAGE, "Squad Leader was killed! NPC %u is stepping up to take tactical command.",
                 m_squadLeader->GetID());
        }
    }

    // 3. SECURE STATE DETACHMENT
    if (m_members.empty()) {
        _log(NPC__AI_MESSAGE, "Squad ID %u has been completely wiped out from grid partition.", m_squadID);
        m_squadTarget = nullptr;
        m_squadLeader = nullptr;
    }
}

void NPCSquad::OnAllMembersArrived() {
    // this should be a switch
    if (m_tacticalTier == 1) {
        _log(NPC__AI_MESSAGE, "Squad ID %u fully landed. Assembling into formation for 5 seconds.", m_squadID);

        // Broadcast the formation ID to the client: watch them glide into position!
        //BroadcastFormationToGrid(EVEDB::Formations::Wedge);

        // Start a 5-second combat delay countdown timer
        m_formationBreakTimer.Start(5000);
    }
}
