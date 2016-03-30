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
#include "inventory/AttributeEnum.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "ship/DestinyManager.h"
#include "system/SystemManager.h"


NPC::NPC(
    SystemManager* system,
    PyServiceMgr& services,
    InventoryItemRef self,
    uint32 corporationID,
    uint32 allianceID,
    const GPoint& position,
    SpawnMgr* spawnMgr)
: DynamicSystemEntity(new DestinyManager(this, system), self),
  m_system(system),
  m_services(services),
  m_spawnMgr(spawnMgr),
  m_corporationID(corporationID),
  m_allianceID(allianceID),
  m_orbitingID(0)
{
    //NOTE: this is bad if we inherit NPC!
    m_AI = new NPCAIMgr(this);

	// SET ALL ATTRIBUTES MISSING FROM DATABASE BEFORE USING THEM FOR ANYTHING:
    // Create default dynamic attributes in the AttributeMap:
    self->SetAttribute(AttrIsOnline,            1, false);											// Is Online
    self->SetAttribute(AttrShieldCharge,        self->GetAttribute(AttrShieldCapacity), false);		// Shield Charge
    self->SetAttribute(AttrArmorDamage,         0.0, false);											// Armor Damage
    self->SetAttribute(AttrMass,                self->type().mass(), false);				// Mass		--check these functions.
    self->SetAttribute(AttrRadius,              self->type().radius(), false);			// Radius
    self->SetAttribute(AttrVolume,              self->type().volume(), false);			// Volume
    self->SetAttribute(AttrCapacity,            self->type().capacity(), false);			// Capacity
    self->SetAttribute(AttrInertia,             1, false);	//WARNING!  NO NPC Ships have Inertia, so we're setting it to 1 for ALL NPC ships
    self->SetAttribute(AttrCapacitorCharge,     self->GetAttribute(AttrCapacitorCapacity), false);	// Set Capacitor Charge to the Capacitor Capacity
    self->SetAttribute(AttrWarpCapacitorNeed,   self->GetAttribute(AttrWarpCapacitorNeed), false);      // Shield Charge

	// Agility
	if (!self->HasAttribute(AttrAgility))
        self->SetAttribute(AttrAgility, 1, false);

	// AttrOrbitRange
    self->SetAttribute(AttrOrbitRange, GetOrbitRange(), false);

    // Hull Damage
    if (!self->HasAttribute(AttrDamage))
        self->SetAttribute(AttrDamage, 0, true );

    m_emDamage = self->GetAttribute(AttrEmDamage).get_float(),
    m_kinDamage = self->GetAttribute(AttrKineticDamage).get_float(),
    m_therDamage = self->GetAttribute(AttrThermalDamage).get_float(),
    m_expDamage = self->GetAttribute(AttrExplosiveDamage).get_float(),

	// Set internal and Destiny values FROM these Attributes, now that they are all setup:
    m_destiny->SetPosition(position, false);
	m_destiny->SetShipCapabilities(self);

    /* Gets the value from the NPC and put on our own vars */
    m_hullDamage = self->GetAttribute(AttrDamage).get_float();
    m_armorDamage = self->GetAttribute(AttrArmorDamage).get_float();
    m_shieldCharge = self->GetAttribute(AttrShieldCharge).get_float();
    m_shieldCapacity = self->GetAttribute(AttrShieldCapacity).get_float();
   // _log(NPC__TRACE, "Created NPC object for %s (%u)", self.get()->itemName().c_str(), self.get()->itemID());
}

NPC::~NPC() {
    //it is so dangerous to do this stuff in a destructor, with the
    //possibility of any of these things making virtual calls...
    //
    // this makes inheriting NPC a bad idea (see constructor)

    //m_system->RemoveNPC(this);

    //if (m_spawner)
        //m_spawner->SpawnDepoped(m_self->itemID());

    TargMgr.DoDestruction();
    SafeDelete(m_AI);
}

void NPC::Process() {
    double profileStartTime = 0.0;
    if (sConfig.misc.UseProfiling)
        profileStartTime = GetTimeUSeconds();

    SystemEntity::Process();
    m_AI->Process();

    if (sConfig.misc.UseProfiling)
        sProfile.AddTime(_npcProfile, GetTimeUSeconds() - profileStartTime);
}

void NPC::Orbit(SystemEntity *who) {
    if (!who)
        m_orbitingID = 0;
    else
        m_orbitingID = who->GetID();
}

double NPC::GetOrbitRange()
{
    double orbitRange = (Item()->GetAttribute(AttrOrbitRange).get_int());
    if (!orbitRange) {
        if (Item()->GetAttribute(AttrMaxRange) < Item()->GetAttribute(AttrFalloff))
            orbitRange = Item()->GetAttribute(AttrMaxRange).get_float();
        else
            orbitRange = Item()->GetAttribute(AttrFalloff).get_float();
        /*
        if (Item()->GetAttribute(AttrRadius) < 30)
            orbitRange = 1500;
        else if (Item()->GetAttribute(AttrRadius) < 60)
            orbitRange = 2500;
        else if (Item()->GetAttribute(AttrRadius) < 150)
            orbitRange = 4000;
        else if (Item()->GetAttribute(AttrRadius) < 280)
            orbitRange = 6000;
        else if (Item()->GetAttribute(AttrRadius) < 550)
            orbitRange = 8000;
        else
            orbitRange = 13000;
        */
    }
    return orbitRange;
}

void NPC::ForcedSetPosition(const GPoint &pt) {
    m_destiny->SetPosition(pt, false);
}

bool NPC::Load(ServiceDB &from) {
    //The old purpose for this was eliminated. But we might find
    //something else to stick in here eventually, so it stays for now.
    return true;
}

void NPC::TargetLost(SystemEntity *who) {
    m_AI->TargetLost(who);
}

void NPC::TargetedAdd(SystemEntity *who) {
    m_AI->Targeted(who);
}

void NPC::EncodeDestiny( Buffer& into ) const
{
    const GPoint& position = GetPosition();

    uint8 mode = Destiny::DSTBALL_STOP;
    if (Destiny()->IsWarping())
        mode = Destiny::DSTBALL_WARP;
    else if (Destiny()->IsFollowing())
        mode = Destiny::DSTBALL_FOLLOW;
    else if (Destiny()->IsOrbiting())
        mode = Destiny::DSTBALL_ORBIT;
    else if (Destiny()->IsMoving())
        mode = Destiny::DSTBALL_GOTO;

    Destiny::BallHeader head;
    head.entityID = GetID();
        head.mode = mode;
        head.radius = GetRadius();
        head.x = position.x;
        head.y = position.y;
        head.z = position.z;
        head.flags = Destiny::IsMassive | Destiny::IsFree;
    into.Append( head );

    Destiny::MassSector mass;
        mass.mass = GetMass();
        mass.cloak = 0;
        mass.Harmonic = -1.0f;
        mass.corporationID = GetCorporationID();
        mass.allianceID = GetAllianceID();
    into.Append( mass );

    Destiny::ShipSector ship;
        ship.maxVelocity = GetMaxVelocity();
        ship.velocity_x = GetVelocity().x;
        ship.velocity_y = GetVelocity().y;
        ship.velocity_z = GetVelocity().z;
        ship.agility = GetAgility();
        ship.speedfraction = m_destiny->GetSpeedFraction();
    into.Append( ship );

    if (mode == Destiny::DSTBALL_WARP) {
        GPoint target = m_destiny->GetTargetPoint();
        Destiny::DSTBALL_WARP_Struct warp;
        warp.effectStamp = -1;   //unknown value  seen many -1, few other random 4-5 digits
        warp.unknown_x = target.x;
        warp.unknown_y = target.y;
        warp.unknown_z = target.z;
        warp.ownerID = m_destiny->GetWarpSpeed();       //ship warp speed x10  (dont ask...this is what it is...more dumb ccp shit)
        warp.unk_1 = 0;      //unknown 64bit number.  seen 4666723172467343360 once....others are 0
        warp.unk_2 = 0;         //unknown 64bit number
        into.Append( warp );
    } else if (mode == Destiny::DSTBALL_FOLLOW) {
        Destiny::DSTBALL_FOLLOW_Struct follow;
        follow.followID = m_destiny->GetTargetID();
        follow.followRange = m_destiny->GetFollowDistance();
        follow.formationID = 0xFF;
        into.Append( follow );
    } else if (mode == Destiny::DSTBALL_ORBIT) {
        Destiny::DSTBALL_ORBIT_Struct orbit;
        orbit.followID = m_destiny->GetTargetID();
        orbit.followRange = m_destiny->GetFollowDistance();
        orbit.formationID = 0xFF;
        into.Append( orbit );
    } else if (mode == Destiny::DSTBALL_GOTO) {
        GPoint target = m_destiny->GetTargetPoint();
        Destiny::DSTBALL_GOTO_Struct go;
        go.x = target.x;
        go.y = target.y;
        go.z = target.z;
        into.Append( go );
    } else {
        Destiny::DSTBALL_STOP_Struct main;
        main.formationID = 0xFF;
        into.Append( main );
    }

    _log(COMMON__WARNING, "NPC::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}


void NPC::MakeDamageState(DoDestinyDamageState &into) const {
    into.shield = m_shieldCharge / m_self->GetAttribute(AttrShieldCapacity).get_float();
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float() +8;
    into.timestamp = Win32TimeNow();
    into.armor = 1.0 - (m_armorDamage / m_self->GetAttribute(AttrArmorHP).get_float());
    into.structure = 1.0 - (m_hullDamage / m_self->GetAttribute(AttrHP).get_float());
}

void NPC::UseShieldRecharge()
{
    // We recharge our shield until it's reaches the shield capacity.
    if (Item()->GetAttribute(AttrShieldCapacity) > m_shieldCharge)
    {
        m_shieldCharge += Item()->GetAttribute(AttrEntityShieldBoostAmount).get_float();
        if (m_shieldCharge > Item()->GetAttribute(AttrShieldCapacity).get_float())
            m_shieldCharge = Item()->GetAttribute(AttrShieldCapacity).get_float();
    } else
        AI()->DisableRepTimers();
    // TODO: Need to send SpecialFX / amount update
    _UpdateDamage();
}

void NPC::UseArmorRepairer()
{
    if( m_armorDamage > 0 )
    {
        m_armorDamage -= Item()->GetAttribute(AttrEntityArmorRepairAmount).get_float();
        if( m_armorDamage < 0.0 )
            m_armorDamage = 0.0;
    } else
        AI()->DisableRepTimers();
    // TODO: Need to send SpecialFX / amount update
    _UpdateDamage();
}

void NPC::_UpdateDamage()
{
    DoDestiny_DamageDetails dmgState;
        dmgState.shield = m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float();
        dmgState.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float();
        dmgState.timestamp = Win32TimeNow();
        dmgState.armor = 1.0 - m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float();
        dmgState.structure = 1.0 - m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float();
    DoDestiny_OnDamageStateChange dmgChange;
        dmgChange.entityID = GetID();
        dmgChange.state = dmgState.Encode();
    PyTuple *up = dmgChange.Encode();
    //source->QueueDestinyUpdate(&up);
}

void NPC::SaveNPC()
{
	// Save all data for this NPC to the database:
	Item()->SaveItem();
}

void NPC::RemoveNPC()
{
    //this is called from SystemManager::RemoveNPC() - no need to RemoveEntity()
    // Remove all data for this NPC from the database:
    Item()->Delete();
}

