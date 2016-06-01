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

#include "EVEServerConfig.h"
#include "character/Character.h"
#include "inventory/AttributeEnum.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "ship/DestinyManager.h"
#include "system/SystemManager.h"


NPC::NPC(InventoryItemRef self, PyServiceMgr &services, SystemManager* pSystem, uint32 corpID, uint32 allyID, SpawnMgr* spawnMgr)
: DynamicSystemEntity(self, services, pSystem),
  m_spawnMgr(spawnMgr)
{
    m_corpID = corpID;
    m_allyID = allyID;
    m_destiny = new DestinyManager(this);
    m_AI = new NPCAIMgr(this);

    Init();
}

void NPC::Init()
{
	// SET ALL ATTRIBUTES MISSING FROM DATABASE BEFORE USING THEM FOR ANYTHING:
    // Create default dynamic attributes in the AttributeMap:
    //m_self->SetAttribute(AttrIsOnline,            1, false);											// Is Online
    m_self->SetAttribute(AttrShieldCharge,        m_self->GetAttribute(AttrShieldCapacity), false);		// Shield Charge
    m_self->SetAttribute(AttrDamage,              0, false);
    m_self->SetAttribute(AttrArmorDamage,         0, false);											// Armor Damage
    m_self->SetAttribute(AttrMass,                m_self->type().mass(), false);				// Mass		--check these functions.
    m_self->SetAttribute(AttrRadius,              m_self->type().radius(), false);			// Radius
    m_self->SetAttribute(AttrVolume,              m_self->type().volume(), false);			// Volume
    m_self->SetAttribute(AttrCapacity,            m_self->type().capacity(), false);			// Capacity
    m_self->SetAttribute(AttrInertia,             1, false);	//WARNING!  NO NPC Ships have Inertia, so we're setting it to 1 for ALL NPC ships
    m_self->SetAttribute(AttrCapacitorCharge,     m_self->GetAttribute(AttrCapacitorCapacity), false);	// Set Capacitor Charge to the Capacitor Capacity
    m_self->SetAttribute(AttrWarpCapacitorNeed,   m_self->GetAttribute(AttrWarpCapacitorNeed), false);      // Shield Charge
    m_self->SetAttribute(AttrOrbitRange,          GetOrbitRange(), false);

    // Agility
    if (!m_self->HasAttribute(AttrAgility))
        m_self->SetAttribute(AttrAgility, 1, false);

    SetResists();

    m_emDamage = m_self->GetAttribute(AttrEmDamage).get_float(),
    m_kinDamage = m_self->GetAttribute(AttrKineticDamage).get_float(),
    m_therDamage = m_self->GetAttribute(AttrThermalDamage).get_float(),
    m_expDamage = m_self->GetAttribute(AttrExplosiveDamage).get_float(),

	m_destiny->SetShipCapabilities(m_self);

    /* Gets the value from the NPC and put on our own vars */
    m_hullDamage = m_self->GetAttribute(AttrDamage).get_float();
    m_armorDamage = m_self->GetAttribute(AttrArmorDamage).get_float();
    m_shieldCharge = m_self->GetAttribute(AttrShieldCharge).get_float();
    m_shieldCapacity = m_self->GetAttribute(AttrShieldCapacity).get_float();
   // _log(NPC__TRACE, "Created NPC object for %s (%u)", m_self.get()->itemName().c_str(), m_self.get()->itemID());
}

NPC::~NPC() {
    //it is so dangerous to do this stuff in a destructor, with the
    //possibility of any of these things making virtual calls...
    //
    // this makes inheriting NPC a bad idea (see constructor)

    //m_system->RemoveNPC(this);

    //if (m_spawner)
        //m_spawner->SpawnDepoped(m_self->itemID());

    SafeDelete(m_destiny);
    SafeDelete(m_AI);
}

void NPC::Process() {
    double profileStartTime = 0.0;
    if (sConfig.server.UseProfiling)
        profileStartTime = GetTimeUSeconds();

    SystemEntity::Process();
    m_AI->Process();

    if (sConfig.server.UseProfiling)
        sProfile.AddTime(_npcProfile, GetTimeUSeconds() - profileStartTime);
}

void NPC::Orbit(SystemEntity *who) {
    if (!who)
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
        mass.cloak = 0;
        mass.Harmonic = -1.0f;
        mass.corporationID = GetCorporationID();
        mass.allianceID = GetAllianceID();
    into.Append( mass );
    DataSector data;
        data.maxVelocity = m_destiny->GetMaxVelocity();
        data.velocity_x = m_destiny->GetVelocity().x;
        data.velocity_y = m_destiny->GetVelocity().y;
        data.velocity_z = m_destiny->GetVelocity().z;
        data.agility = m_destiny->GetAgility();
        data.speedfraction = m_destiny->GetSpeedFraction();
        into.Append( data );
    if (mode == DSTBALL_WARP) {
        GPoint target = m_destiny->GetTargetPoint();
        DSTBALL_WARP_Struct warp;
            warp.formationID = 0xFF;
            warp.effectStamp = -1; // m_destiny->GetStateStamp();   //timestamp when warp started
            warp.x = target.x;
            warp.y = target.y;
            warp.z = target.z;
            warp.ownerID = m_destiny->GetWarpSpeed();       //ship warp speed x10  (dont ask...this is what it is...more dumb ccp shit)
            warp.followRange = 0;
            warp.followID = (m_destiny->GetTargetID() ? m_destiny->GetTargetID() : 0);
        into.Append( warp );
    } else if (mode == DSTBALL_FOLLOW) {
        DSTBALL_FOLLOW_Struct follow;
            follow.followID = m_destiny->GetTargetID();
            follow.followRange = m_destiny->GetFollowDistance();
            follow.formationID = 0xFF;
        into.Append( follow );
    } else if (mode == DSTBALL_ORBIT) {
        DSTBALL_ORBIT_Struct orbit;
            orbit.followID = m_destiny->GetTargetID();
            orbit.followRange = m_destiny->GetFollowDistance();
            orbit.formationID = 0xFF;
        into.Append( orbit );
    } else if (mode == DSTBALL_GOTO) {
        GPoint target = m_destiny->GetTargetPoint();
        DSTBALL_GOTO_Struct go;
            go.formationID = 0xFF;
            go.x = target.x;
            go.y = target.y;
            go.z = target.z;
        into.Append( go );
    } else {
        DSTBALL_STOP_Struct main;
            main.formationID = 0xFF;
        into.Append( main );
    }

    _log(COMMON__WARNING, "NPC::EncodeDestiny: %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void NPC::UseShieldRecharge()
{
    // We recharge our shield until it's reaches the shield capacity.
    if (m_self->GetAttribute(AttrShieldCapacity) > m_shieldCharge)
    {
        m_shieldCharge += m_self->GetAttribute(AttrEntityShieldBoostAmount).get_float();
        if (m_shieldCharge > m_self->GetAttribute(AttrShieldCapacity).get_float())
            m_shieldCharge = m_self->GetAttribute(AttrShieldCapacity).get_float();
        m_self->SetAttribute(AttrShieldCharge, m_shieldCharge, false);
    } else
        m_AI->DisableRepTimers();
    // TODO: Need to send SpecialFX / amount update
    UpdateDamage();
}

void NPC::UseArmorRepairer()
{
    if( m_armorDamage > 0 )
    {
        m_armorDamage -= m_self->GetAttribute(AttrEntityArmorRepairAmount).get_float();
        if( m_armorDamage < 0.0 )
            m_armorDamage = 0.0;
        m_self->SetAttribute(AttrArmorDamage, m_armorDamage, false);
    } else
        m_AI->DisableRepTimers();
    // TODO: Need to send SpecialFX / amount update
    UpdateDamage();
}

void NPC::UseHullRepairer()
{
    if( m_hullDamage > 0 )
    {
        //m_hullDamage -= m_self->GetAttribute(AttrEntityhullRepairAmount).get_float();  << NSA - create later
        if( m_hullDamage < 0.0 )
            m_hullDamage = 0.0;
        m_self->SetAttribute(AttrDamage, m_hullDamage, false);
    } else
        m_AI->DisableRepTimers();
    // TODO: Need to send SpecialFX / amount update
    UpdateDamage();
}

void NPC::SaveNPC()
{
	m_self->SaveItem();
}

void NPC::RemoveNPC()
{
    //this is called from SystemManager::RemoveNPC() - no need to RemoveEntity()
    m_self->Delete();
}

void NPC::SetResists() {
    /* fix for missing resist attribs -allan 18April16  */
    if (!m_self->HasAttribute(AttrShieldEmDamageResonance)) m_self->SetAttribute(AttrShieldEmDamageResonance, 1.0, false);
    if (!m_self->HasAttribute(AttrShieldExplosiveDamageResonance)) m_self->SetAttribute(AttrShieldExplosiveDamageResonance, 1.0, false);
    if (!m_self->HasAttribute(AttrShieldKineticDamageResonance)) m_self->SetAttribute(AttrShieldKineticDamageResonance, 1.0, false);
    if (!m_self->HasAttribute(AttrShieldThermalDamageResonance)) m_self->SetAttribute(AttrShieldThermalDamageResonance, 1.0, false);
    if (!m_self->HasAttribute(AttrArmorEmDamageResonance)) m_self->SetAttribute(AttrArmorEmDamageResonance, 1.0, false);
    if (!m_self->HasAttribute(AttrArmorExplosiveDamageResonance)) m_self->SetAttribute(AttrArmorExplosiveDamageResonance, 1.0, false);
    if (!m_self->HasAttribute(AttrArmorKineticDamageResonance)) m_self->SetAttribute(AttrArmorKineticDamageResonance, 1.0, false);
    if (!m_self->HasAttribute(AttrArmorThermalDamageResonance)) m_self->SetAttribute(AttrArmorThermalDamageResonance, 1.0, false);
    if (!m_self->HasAttribute(AttrEmDamageResonance)) m_self->SetAttribute(AttrEmDamageResonance, 1.0, false);
    if (!m_self->HasAttribute(AttrExplosiveDamageResonance)) m_self->SetAttribute(AttrExplosiveDamageResonance, 1.0, false);
    if (!m_self->HasAttribute(AttrKineticDamageResonance)) m_self->SetAttribute(AttrKineticDamageResonance, 1.0, false);
    if (!m_self->HasAttribute(AttrThermalDamageResonance)) m_self->SetAttribute(AttrThermalDamageResonance, 1.0, false);
}
