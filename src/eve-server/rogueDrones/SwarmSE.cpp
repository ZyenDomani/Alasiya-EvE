/**
 * SwarmSE.cpp
 *      this class is for rogue drones
 *
 * @Author:     Allan
 * @Version:    1.03
 * @AI Version: 0.95
 * @Date:       14Jul26  (copied from NPC.cpp)
 */



#include "../eve-server.h"

#include "Client.h"
#include "EntityMgr.h"
#include "EVEServerConfig.h"
#include "StaticDataMgr.h"
#include "fleet/FleetService.h"
#include "inventory/AttributeEnum.h"
#include "map/MapDB.h"
#include "rogueDrones/SwarmSE.h"
#include "system/Container.h"
#include "system/Damage.h"
#include "system/DestinyManager.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/AnomalyMgr.h"


SwarmSE::SwarmSE(InventoryItemRef self, PyServiceMgr& services, SystemManager* system, const FactionData& data, SpawnMgr* spawnMgr)
: DynamicSystemEntity(self, services, system),
m_AI(new SwarmAI(this)),
m_spawnMgr(spawnMgr),
m_hauler(false),
m_moduleCount(0),
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

    // sightrange is arbitrary now.  live uses 'can see all on grid'
    if (EvE::icontains(m_self->type().groupName(), "Swarm")) {
        m_sightRange = 20000;
        m_size = Swarm::Size::Swarm;
    } else if (EvE::icontains(m_self->type().groupName(), "Frigate")) {
        m_sightRange = 35000;
        m_size = Swarm::Size::Frigate;
    } else if (EvE::icontains(m_self->type().groupName(), "Destroyer")) {
        m_sightRange = 50000;
        m_size = Swarm::Size::Destroyer;
    } else if (EvE::icontains(m_self->type().groupName(), "Cruiser")) {
        m_sightRange = 70000;
        m_size = Swarm::Size::Cruiser;
    } else if (EvE::icontains(m_self->type().groupName(), "BattleCruiser")) {
        m_sightRange = 95000;
        m_size = Swarm::Size::BCruiser; //22824
    } else if (EvE::icontains(m_self->type().groupName(), "BattleShip")) {
        m_sightRange = 130000;
        m_size = Swarm::Size::BShip;
    } else if (EvE::icontains(m_self->type().groupName(), "Hauler")) {
        m_sightRange = 10000;
        m_size = Swarm::Size::Indy;
    } else {
        // not sure what to do here.
    	_log(NPC__WARNING, "%s(%u): groupName %s (ID %u) - no match found in 'contains' test ", \
                m_self->name(), m_self->itemID(), m_self->type().groupName().c_str(), m_self->groupID());
    }

    _log(NPC__TRACE, "Created NPC object for %s (%u) - %u Modules (%u/%u), Data: O:%u, C:%u, A:%u, W:%u", \
            m_self.get()->name(), m_self.get()->itemID(), m_moduleCount, mininum, maximum, \
            m_ownerID, m_corpID, m_allyID, m_warID);
}

SwarmSE::~SwarmSE() {
    SafeDelete(m_AI);
}

bool SwarmSE::Load() {
    m_destiny->UpdateShipVariables();
    SetResists();

    // load data
    m_AI->Init();

    //dSE::Load() just returns true for now (no code)
    return DynamicSystemEntity::Load();
}

void SwarmSE::Process() {
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

void SwarmSE::Orbit(SystemEntity *pTargetSE) {
    if (pTargetSE == nullptr) {
        m_orbitingID = 0;
    } else {
        m_orbitingID = pTargetSE->GetID();
    }
}

void SwarmSE::UseShieldRecharge() {
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

void SwarmSE::UseArmorRepairer() {
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

void SwarmSE::SetResists() {
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

void SwarmSE::EncodeDestiny(Buffer& into)
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
            Vector3d target = m_destiny->GetTargetPoint();
            WARP_Struct warp;
            warp.formationID = -1;
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
            follow.formationID = -1;
            into.Append( follow );
        }  break;
        case Ball::Mode::ORBIT: {
            ORBIT_Struct orbit;
            orbit.targetID = m_destiny->GetTargetID();
            orbit.followRange = m_destiny->GetFollowDistance();
            orbit.formationID = -1;
            into.Append( orbit );
        }  break;
        case Ball::Mode::GOTO: {
            Vector3d target = m_destiny->GetTargetPoint();
            GOTO_Struct go;
            go.formationID = -1;
            go.x = target.x;
            go.y = target.y;
            go.z = target.z;
            into.Append( go );
        }  break;
        case Ball::Mode::FORMATION: {
            // this implies squad
			/*
            assert (m_squad != nullptr);
            FORMATION_Struct form;
            form.formationID = m_squad->GetFormID();
            form.leaderID = m_squad->GetLeader()->GetID();
            form.spacing = m_squad->GetSpacing();
			*/
            form.syncIndex = 1;
            into.Append(form);
        }  break;
        default: {
            STOP_Struct main;
            main.formationID = -1;
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

    _log(SE__DESTINY, "SwarmSE::EncodeDestiny(): %s - id:%lli, mode:%s, flags:0x%X, Vel:%.1f, %.1f, %.1f", \
            GetName(), head.entityID, modeStr.c_str(), head.flags, data.velX, data.velY, data.velZ);
}

void SwarmSE::Killed(Damage &fatal_blow) {
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
    uint32 killerID = 0;
    Client* pClient(nullptr);
    SystemEntity* killer(fatal_blow.srcSE);

    if (killer->HasPilot()) {
        pClient = killer->GetPilot();
        killerID = pClient->GetCharacterID();
    } else if (killer->IsDroneSE()) {
        pClient = sEntityMgr.FindClientByCharID(killer->GetSelf()->ownerID());
        if (pClient == nullptr) {
            sLog.Error("SwarmSE::Killed()", "killer == IsDrone and pPlayer == nullptr");
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

    Vector3d wreckPosition(m_self->position());
    if (wreckPosition.isNaN()) {
        sLog.Error("SwarmSE::Killed()", "Wreck Position is NaN");
        return;
    }
    uint32 wreckTypeID(sDataMgr.GetWreckID(m_self->typeID()));
    if (!IsWreckTypeID(wreckTypeID)) {
        sLog.Error("SwarmSE::Killed()", "Could not get wreckType for %s of type %u", m_self->name(), m_self->typeID());
        // default to generic frigate wreck till i get better checks and/or complete wreck data
        wreckTypeID = 26557;
    }

    std::string wreck_name = m_self->itemName();
    wreck_name += " Wreck";
    ItemData wreckItemData(wreckTypeID, killerID, locationID, flagAutoFit, wreck_name.c_str(), wreckPosition, itoa(m_allyID));
    WreckContainerRef wreckItemRef = sItemFactory.SpawnWreckContainer( wreckItemData );
    if (wreckItemRef.get() == nullptr) {
        sLog.Error("SwarmSE::Killed()", "Creating Wreck Item Failed for %s of type %u", wreck_name.c_str(), wreckTypeID);
        return;
    }

    if (is_log_enabled(PHYSICS__TRACE))
        _log(PHYSICS__TRACE, "SwarmSE::Killed() - NPC %s(%u) Position: %.2f,%.2f,%.2f.  Wreck %s(%u) Position: %.2f,%.2f,%.2f.", \
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
        sLog.Error("SwarmSE::Killed()", "Spawning Wreck Failed for typeID %u", wreckTypeID);
        wreckItemRef->Delete();
        return;
    }
    m_destiny->SendJettisonPacket();
}

void SwarmSE::CmdDropLoot()
{
    m_destiny->SendJettisonPacket();
    /** @todo finish this */
    //DropLoot(wreckItemRef, m_self->groupID());
}

void SwarmSE::ApplyTrackingBoost(float mod/*1.0f*/) {

}

