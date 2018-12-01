/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
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
    Updates:        Allan
*/


#include "eve-server.h"

#include "Client.h"
#include "EntityList.h"
#include "EVEServerConfig.h"
#include "StaticDataMgr.h"
#include "inventory/AttributeEnum.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "system/Container.h"
#include "system/Damage.h"
#include "system/DestinyManager.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"


NPC::NPC(InventoryItemRef self, PyServiceMgr& services, SystemManager* system, const FactionData& data, SpawnMgr* spawnMgr)
: DynamicSystemEntity(self, services, system),
m_spawnMgr(spawnMgr)
{
    killed = false;

    m_allyID = data.allianceID;
    m_warID = data.factionID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;

    // Create default dynamic attributes in the AttributeMap:
    m_self->SetAttribute(AttrDamage,              0);
    m_self->SetAttribute(AttrArmorDamage,         0);
    m_self->SetAttribute(AttrInertia,             1);
    m_self->SetAttribute(AttrWarpCapacitorNeed,   0.00001);
    m_self->SetAttribute(AttrMass,                m_self->type().mass());
    m_self->SetAttribute(AttrRadius,              m_self->type().radius());
    m_self->SetAttribute(AttrVolume,              m_self->type().volume());
    m_self->SetAttribute(AttrCapacity,            m_self->type().capacity());
    m_self->SetAttribute(AttrShieldCharge,        m_self->GetAttribute(AttrShieldCapacity), false);
    m_self->SetAttribute(AttrCapacitorCharge,     m_self->GetAttribute(AttrCapacitorCapacity), false);

    /* Gets the value from the NPC and put on our own vars */
    m_emDamage = m_self->GetAttribute(AttrEmDamage).get_float(),
    m_kinDamage = m_self->GetAttribute(AttrKineticDamage).get_float(),
    m_therDamage = m_self->GetAttribute(AttrThermalDamage).get_float(),
    m_expDamage = m_self->GetAttribute(AttrExplosiveDamage).get_float(),
    m_hullDamage = m_self->GetAttribute(AttrDamage).get_float();
    m_armorDamage = m_self->GetAttribute(AttrArmorDamage).get_float();
    m_shieldCharge = m_self->GetAttribute(AttrShieldCharge).get_float();
    m_shieldCapacity = m_self->GetAttribute(AttrShieldCapacity).get_float();

   // _log(NPC__TRACE, "Created NPC object for %s (%u)", m_self.get()->itemName().c_str(), m_self.get()->itemID());
}

NPC::~NPC() {
    SafeDelete(m_AI);
}

bool NPC::Load()
{
    m_destiny->SetShipCapabilities(m_self);

    SetResists();

    m_AI = new NPCAIMgr(this);
    return DynamicSystemEntity::Load();
}


void NPC::Process() {
    if (killed)
        return;

    double profileStartTime = 0.0;
    if (sConfig.debug.UseProfiling)
        profileStartTime = GetTimeUSeconds();

    /*  Enable base call to Process Targeting and Movement  */
    SystemEntity::Process();
    m_AI->Process();

    if (sConfig.debug.UseProfiling)
        sProfile.AddTime(_npcProfile, GetTimeUSeconds() - profileStartTime);
}

void NPC::Orbit(SystemEntity *who) {
    if (who == nullptr)
        m_orbitingID = 0;
    else
        m_orbitingID = who->GetID();
}

void NPC::TargetLost(SystemEntity *who) {
    m_AI->TargetLost(who);
}

void NPC::TargetedAdd(SystemEntity *who) {
    m_AI->Targeted(who);
}

void NPC::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;

    uint8 mode = DSTBALL_STOP;
    if (m_destiny->IsWarping())
        mode = DSTBALL_WARP;
    else if (m_destiny->IsFollowing())
        mode = DSTBALL_FOLLOW;
    else if (m_destiny->IsOrbiting())
        mode = DSTBALL_ORBIT;
    else if (m_destiny->IsMoving())
        mode = DSTBALL_GOTO;
    BallHeader head;
        head.entityID = GetID();
        head.mode = mode;
        head.radius = GetRadius();
        head.x = x();
        head.y = y();
        head.z = z();
        head.flags = IsMassive | IsFree;
    into.Append( head );
    MassSector mass;
        mass.mass = m_destiny->GetMass();
        mass.cloak = (m_destiny->IsCloaked() ? 1 : 0);
        mass.harmonic = m_harmonic;
        mass.corporationID = m_corpID;
        mass.allianceID = (m_allyID > 0 ? m_allyID : -1);
    into.Append( mass );
    DataSector data;
        data.maxVelocity = m_destiny->GetMaxVelocity();
        data.velocity_x = m_destiny->GetVelocity().x;
        data.velocity_y = m_destiny->GetVelocity().y;
        data.velocity_z = m_destiny->GetVelocity().z;
        data.inertia = m_destiny->GetInertia();
        data.speedfraction = m_destiny->GetSpeedFraction();
    into.Append( data );
    switch (mode) {
        case DSTBALL_WARP: {
            GPoint target = m_destiny->GetTargetPoint();
            DSTBALL_WARP_Struct warp;
                warp.formationID = 0xFF;
                warp.x = target.x;
                warp.y = target.y;
                warp.z = target.z;
                warp.ownerID = m_destiny->GetWarpSpeed();       //ship warp speed x10  (dont ask...this is what it is...more dumb ccp shit)
                // warp timing.  see Ship::EncodeDestiny() for notes/updates
                warp.effectStamp = -1; //m_destiny->GetStateStamp();   //timestamp when warp started
                warp.followRange = 0;   //this isnt right
                warp.followID = 0;  //this isnt right
            into.Append( warp );
        }  break;
        case DSTBALL_FOLLOW: {
            DSTBALL_FOLLOW_Struct follow;
                follow.followID = m_destiny->GetTargetID();
                follow.followRange = m_destiny->GetFollowDistance();
                follow.formationID = 0xFF;
            into.Append( follow );
        }  break;
        case DSTBALL_ORBIT: {
            DSTBALL_ORBIT_Struct orbit;
                orbit.followID = m_destiny->GetTargetID();
                orbit.followRange = m_destiny->GetFollowDistance();
                orbit.formationID = 0xFF;
            into.Append( orbit );
        }  break;
        case DSTBALL_GOTO: {
            GPoint target = m_destiny->GetTargetPoint();
            DSTBALL_GOTO_Struct go;
                go.formationID = 0xFF;
                go.x = target.x;
                go.y = target.y;
                go.z = target.z;
            into.Append( go );
        }  break;
        default: {
            DSTBALL_STOP_Struct main;
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

    _log(SE__DESTINY, "NPC::EncodeDestiny(): %s - id:%u, mode:%s, flags:0x%X, Vel:%.1f, %.1f, %.1f", \
            GetName(), head.entityID, modeStr.c_str(), head.flags, data.velocity_x, data.velocity_y, data.velocity_z);
}

void NPC::UseShieldRecharge()
{
    // We recharge our shield until it's full.
    if (m_self->GetAttribute(AttrShieldCapacity) > m_shieldCharge) {
        m_shieldCharge += m_self->GetAttribute(AttrEntityShieldBoostAmount).get_float();
        if (m_shieldCharge > m_self->GetAttribute(AttrShieldCapacity).get_float())
            m_shieldCharge = m_self->GetAttribute(AttrShieldCapacity).get_float();
        m_self->SetAttribute(AttrShieldCharge, m_shieldCharge);
    } else
        m_AI->DisableRepTimers(true, false);
    // TODO: Need to send SpecialFX / amount update
    UpdateDamage();
}

void NPC::UseArmorRepairer()
{
    if (m_armorDamage > 0) {
        m_armorDamage -= m_self->GetAttribute(AttrEntityArmorRepairAmount).get_float();
        if (m_armorDamage < 0.0)
            m_armorDamage = 0.0;
        m_self->SetAttribute(AttrArmorDamage, m_armorDamage);
    } else
        m_AI->DisableRepTimers(false, true);
    // TODO: Need to send SpecialFX / amount update
    UpdateDamage();
}

void NPC::UseHullRepairer()
{
    if (m_hullDamage > 0) {
        //m_hullDamage -= m_self->GetAttribute(AttrEntityhullRepairAmount).get_float();  << NSA - create later
        if (m_hullDamage < 0.0)
            m_hullDamage = 0.0;
        m_self->SetAttribute(AttrDamage, m_hullDamage);
    } else
        m_AI->DisableRepTimers(false, false);
    // TODO: Need to send SpecialFX / amount update
    // gfxBoosterID
    UpdateDamage();
}

void NPC::MissileLaunched(Missile* pMissile)
{
    m_AI->MissileLaunched(pMissile);
}

void NPC::SaveNPC()
{
	m_self->SaveItem();
}

void NPC::RemoveNPC()
{
    //this is called from SystemManager::RemoveNPC() which calls other SE* methods as needed
    m_self->Delete();
}

void NPC::SetResists() {
    /* fix for missing resist attribs -allan 18April16  */
    if (!m_self->HasAttribute(AttrShieldEmDamageResonance)) m_self->SetAttribute(AttrShieldEmDamageResonance, 1.0);
    if (!m_self->HasAttribute(AttrShieldExplosiveDamageResonance)) m_self->SetAttribute(AttrShieldExplosiveDamageResonance, 1.0);
    if (!m_self->HasAttribute(AttrShieldKineticDamageResonance)) m_self->SetAttribute(AttrShieldKineticDamageResonance, 1.0);
    if (!m_self->HasAttribute(AttrShieldThermalDamageResonance)) m_self->SetAttribute(AttrShieldThermalDamageResonance, 1.0);
    if (!m_self->HasAttribute(AttrArmorEmDamageResonance)) m_self->SetAttribute(AttrArmorEmDamageResonance, 1.0);
    if (!m_self->HasAttribute(AttrArmorExplosiveDamageResonance)) m_self->SetAttribute(AttrArmorExplosiveDamageResonance, 1.0);
    if (!m_self->HasAttribute(AttrArmorKineticDamageResonance)) m_self->SetAttribute(AttrArmorKineticDamageResonance, 1.0);
    if (!m_self->HasAttribute(AttrArmorThermalDamageResonance)) m_self->SetAttribute(AttrArmorThermalDamageResonance, 1.0);
    if (!m_self->HasAttribute(AttrEmDamageResonance)) m_self->SetAttribute(AttrEmDamageResonance, 1.0);
    if (!m_self->HasAttribute(AttrExplosiveDamageResonance)) m_self->SetAttribute(AttrExplosiveDamageResonance, 1.0);
    if (!m_self->HasAttribute(AttrKineticDamageResonance)) m_self->SetAttribute(AttrKineticDamageResonance, 1.0);
    if (!m_self->HasAttribute(AttrThermalDamageResonance)) m_self->SetAttribute(AttrThermalDamageResonance, 1.0);
}

void NPC::Killed(Damage &fatal_blow) {
    if ((m_bubble == nullptr) or (m_destiny == nullptr))
        return; // make error here?

    if (killed)
        return; // fix for multiple wrecks created at same time.
    killed = true;

    m_destiny->Halt();
    m_destiny->SendTerminalExplosion(m_self->itemID(), m_bubble->GetID());
    m_system->RemoveEntity(this);  //this also removes from db

    //notify our spawn manager that we are gone.
    if ((m_spawnMgr != nullptr) and (m_self.get() != nullptr))
        m_spawnMgr->SpawnKilled(m_bubble, m_self->itemID());

    uint32 killerID = 0;
    Client* pClient(nullptr);
    SystemEntity* killer = fatal_blow.srcSE;
    
    if (killer->HasPilot()) {
        pClient = killer->GetPilot();
        killerID = pClient->GetCharacterID();
    } else if (killer->IsDroneSE()) {
        pClient = sEntityList.FindClientByCharID( killer->GetSelf()->ownerID() );
        if (pClient == nullptr) {
            sLog.Error("NPC::Killed()", "killer == IsDrone and pPlayer == nullptr");
        } else
            killerID = pClient->GetCharacterID();
    } else
        killerID = killer->GetID();

    uint32 locationID = GetLocationID();
    //  log faction kill in dynamic data   -allan
    MapDB::AddKillToDynamicData(locationID);
    MapDB::AddFactionKillToDynamicData(locationID);

    if (pClient != nullptr) {
        //award kill bounty.
        AwardBounty( pClient );
        if (m_system->GetSystemSecurityRating() > 0)
            AwardSecurityStatus(m_self, pClient->GetChar().get());  // this awards secStatusChange for npcs in empire space
    }

    GPoint wreckPosition = m_destiny->GetPosition();
    uint32 wreckTypeID = sDataMgr.GetWreckID(m_self->typeID());
    if (wreckTypeID < 1) {
        sLog.Error("NPC::Killed()", "Could not get wreckType for %s of type %u", m_self->itemName().c_str(), m_self->typeID());
        // default to generic frigate wreck till i get better checks and/or complete wreck data
        wreckTypeID = 26557;
    }

    std::string wreck_name = m_self->itemName();
    wreck_name += " Wreck";
    const char* faction = itoa(m_allyID);
    ItemData wreckItemData(wreckTypeID, killerID, locationID, flagAutoFit, wreck_name.c_str(), wreckPosition, faction);
    WreckContainerRef wreckItemRef = sItemFactory.SpawnWreckContainer( wreckItemData );
    if (wreckItemRef.get() == nullptr) {
        sLog.Error("NPC::Killed()", "Creating Wreck Item Failed for %s of type %u", wreck_name.c_str(), wreckTypeID);
        return;
    }

    DropLoot(wreckItemRef, m_self->groupID(), killerID);

    DBSystemDynamicEntity wreckEntity;
        wreckEntity.allianceID = (killer->GetAllianceID() == 0 ? m_allyID : killer->GetAllianceID());
        wreckEntity.categoryID = EVEDB::invCategories::Celestial;
        wreckEntity.corporationID = killer->GetCorporationID();
        wreckEntity.factionID = (killer->GetWarFactionID() == 0 ? m_warID : killer->GetWarFactionID());
        wreckEntity.groupID = EVEDB::invGroups::Wreck;
        wreckEntity.itemID = wreckItemRef->itemID();
        wreckEntity.itemName = wreck_name;
        wreckEntity.ownerID = killerID;
        wreckEntity.typeID = wreckTypeID;
        wreckEntity.x = wreckPosition.x;
        wreckEntity.y = wreckPosition.y;
        wreckEntity.z = wreckPosition.z;

    if (!m_system->BuildDynamicEntity(wreckEntity)) {
        sLog.Error("NPC::Killed()", "Spawning Wreck Failed for typeID %u", wreckTypeID);
        wreckItemRef->Delete();
        return;
    }

    if (is_log_enabled(PHYSICS__TRACE))
        _log(PHYSICS__TRACE, "NPC::Killed() - NPC %s(%u) Position: %.2f,%.2f,%.2f.  Wreck %s(%u) Position: %.2f,%.2f,%.2f.", \
                GetName(), GetID(), x(), y(), z(), wreckItemRef->itemName().c_str(), wreckItemRef->itemID(), wreckPosition.x, wreckPosition.y, wreckPosition.z);
}
