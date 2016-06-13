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
    Author:     Aknor Jaden
    Updates:    Allan
*/

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "inventory/AttributeEnum.h"
#include "ship/DestinyManager.h"
#include "ship/Drone.h"
#include "ship/DroneAI.h"
#include "system/SystemManager.h"

using namespace Destiny;

Drone::Drone(InventoryItemRef drone, PyServiceMgr &services, SystemManager* pSystem, const GPoint &position)
: DynamicSystemEntity(drone, services, pSystem)
{
    _droneRef = drone;
    m_AI = new DroneAIMgr(this);
    m_destiny = new DestinyManager(this);
    m_owner = nullptr;

    drone->SetAttribute(AttrIsOnline,            1, false);                                          // Is Online
    drone->SetAttribute(AttrShieldCharge,        drone->GetAttribute(AttrShieldCapacity), false);     // Shield Charge
    drone->SetAttribute(AttrArmorDamage,         0.0, false);                                            // Armor Damage
    drone->SetAttribute(AttrMass,                drone->type().mass(), false);                // Mass     --check these functions.
    drone->SetAttribute(AttrRadius,              drone->type().radius(), false);          // Radius
    drone->SetAttribute(AttrVolume,              drone->type().volume(), false);          // Volume
    drone->SetAttribute(AttrCapacity,            drone->type().capacity(), false);            // Capacity
    drone->SetAttribute(AttrInertia,             1, false);  //WARNING!  NO NPC Ships have Inertia, so we're setting it to 1 for ALL NPC ships
    drone->SetAttribute(AttrCapacitorCharge,     drone->GetAttribute(AttrCapacitorCapacity), false);  // Set Capacitor Charge to the Capacitor Capacity
    drone->SetAttribute(AttrWarpCapacitorNeed,   drone->GetAttribute(AttrWarpCapacitorNeed), false);      // Shield Charge

    if (!drone->HasAttribute(AttrInetia))
        drone->SetAttribute(AttrInetia, 1, false);

    if (!drone->HasAttribute(AttrDamage))
        drone->SetAttribute(AttrDamage, 0, true );

    m_orbitRange = (m_self->GetAttribute(AttrOrbitRange).get_int());
    if (!m_orbitRange) {
        if (m_self->GetAttribute(AttrMaxRange) < m_self->GetAttribute(AttrFalloff))
            m_orbitRange = m_self->GetAttribute(AttrMaxRange).get_float();
        else
            m_orbitRange = m_self->GetAttribute(AttrFalloff).get_float();
    }
    drone->SetAttribute(AttrOrbitRange, GetOrbitRange());

    m_emDamage = drone->GetAttribute(AttrEmDamage).get_float(),
    m_kinDamage = drone->GetAttribute(AttrKineticDamage).get_float(),
    m_therDamage = drone->GetAttribute(AttrThermalDamage).get_float(),
    m_expDamage = drone->GetAttribute(AttrExplosiveDamage).get_float(),

    m_destiny->SetPosition(position, false);
    m_destiny->SetShipCapabilities(drone);

    /* Gets the value from the NPC and put on our own vars */
    m_hullDamage = drone->GetAttribute(AttrDamage).get_float();
    m_armorDamage = drone->GetAttribute(AttrArmorDamage).get_float();
    m_shieldCharge = drone->GetAttribute(AttrShieldCharge).get_float();
    m_shieldCapacity = drone->GetAttribute(AttrShieldCapacity).get_float();

    _log(NPC__TRACE, "Created Drone object for %s (%u)", drone.get()->itemName().c_str(), drone.get()->itemID());
}

Drone::~Drone() {
    m_targMgr->DoDestruction();
    SafeDelete(m_destiny);
    SafeDelete(m_AI);
}

void Drone::SetOwner(Client* pClient) {
    m_self->ChangeOwner(pClient->GetCharacterID());
    m_owner = pClient;
    m_corpID = pClient->GetCorporationID();
    m_allyID = pClient->GetAllianceID();
    m_warID = pClient->GetWarFactionID();
}

void Drone::Process() {
    double profileStartTime = 0.0;
    if (sConfig.server.UseProfiling)
        profileStartTime = GetTimeUSeconds();

    SystemEntity::Process();
    /** @todo (allan) finish drone AI and processing */
    m_AI->Process();

    if (sConfig.server.UseProfiling)
        sProfile.AddTime(_droneProfile, GetTimeUSeconds() - profileStartTime);
}

void Drone::Orbit(SystemEntity *who) {
    if (!who)
        m_orbitingID = 0;
    else
        m_orbitingID = who->GetID();
}

void Drone::TargetAdded(SystemEntity* who) {
    /** @todo (Allan) will need code once drones are implemented */
}

void Drone::TargetLost(SystemEntity *who) {
    m_AI->TargetLost(who);
}

void Drone::TargetedAdd(SystemEntity *who) {
    m_AI->Targeted(who);
}

void Drone::TargetedLost(SystemEntity* who) {
    /** @todo (Allan) will need code once drones are implemented */
}

void Drone::UseShieldRecharge() {
    if (m_self->GetAttribute(AttrShieldCapacity) > m_shieldCharge) {
        m_shieldCharge += m_self->GetAttribute(AttrEntityShieldBoostAmount).get_float();
        if (m_shieldCharge > m_self->GetAttribute(AttrShieldCapacity).get_float())
            m_shieldCharge = m_self->GetAttribute(AttrShieldCapacity).get_float();
    } else
        m_AI->DisableRepTimers();
    /** @todo (allan) Need to send SpecialFX / amount update */
    UpdateDamage();
}

void Drone::UseArmorRepairer() {
    if (m_armorDamage) {
        m_armorDamage -= m_self->GetAttribute(AttrEntityArmorRepairAmount).get_float();
        if( m_armorDamage < 0.0 )
            m_armorDamage = 0.0;
    } else
        m_AI->DisableRepTimers();
    /** @todo (allan) Need to send SpecialFX / amount update */
    UpdateDamage();
}

void Drone::SaveDrone() {
    m_self->SaveItem();
}

void Drone::RemoveDrone() {
    /** @todo (Allan) this may need more here */
    m_self->Delete();
}

PyDict* Drone::MakeSlimItem() {
    _log(COMMON__WARNING, "MakeSlimItem for Drone %u ", m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",           new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",           new PyInt(m_self->typeID()));
        slim->SetItemString("categoryID",       new PyInt(m_self->categoryID()));
        slim->SetItemString("groupID",          new PyInt(m_self->groupID()));
        slim->SetItemString("name",             new PyString(m_self->itemName()));
        slim->SetItemString("ownerID",          new PyInt(GetOwnerID()));
        slim->SetItemString("corpID",           new PyInt(GetCorporationID()));
        slim->SetItemString("allianceID",       new PyInt(GetAllianceID()));
        slim->SetItemString("warFactionID",     new PyInt(GetWarFactionID()));
        slim->SetItemString("bounty",           new PyFloat(GetBounty()));
        slim->SetItemString("securityStatus",   new PyFloat(GetSecurityRating()));
    return slim;
}

void Drone::EncodeDestiny( Buffer& into )
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

    // drone id's WILL be int64 (> 1000000000000L)
    BallHeader head;
    head.entityID = GetID();
        head.mode = mode;
        head.radius = GetRadius();
        head.x = x();
        head.y = y();
        head.z = z();
        head.flags = IsFree | IsInteractive;
    into.Append( head );

    MassSector mass;
        mass.mass = m_destiny->GetMass();
        mass.cloak = 0;
        mass.Harmonic = 1.0f;
        mass.corporationID = GetCorporationID();
        mass.allianceID = GetAllianceID();
    into.Append( mass );

    DataSector data;
        data.maxVelocity = m_destiny->GetMaxVelocity();
        data.velocity_x = m_destiny->GetVelocity().x;
        data.velocity_y = m_destiny->GetVelocity().y;
        data.velocity_z = m_destiny->GetVelocity().z;
        data.intertia = m_destiny->GetInertia();
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
            go.x = target.x;
            go.y = target.y;
            go.z = target.z;
        into.Append( go );
    } else {
        DSTBALL_STOP_Struct main;
            main.formationID = 0xFF;
        into.Append( main );
    }

    _log(COMMON__WARNING, "Drone::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void Drone::MakeDamageState(DoDestinyDamageState &into)
{
    into.shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float() +5;
    into.timestamp = Win32TimeNow();
    into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float());
}

/*   when drone is scooped up....
 *
                    [PyTuple 2 items]
                      [PyString "OnDroneStateChange"]
                      [PyList 7 items]
                        [PyIntegerVar 1540263056]
                        [PyNone]
                        [PyNone]
                        [PyNone]
                        [PyNone]
                        [PyNone]
                        [PyNone]
                        */

//OnDroneStateChange(droneID, ownerID, controllerID, activityState, droneTypeID, controllerOwnerID, targetID)

/*
 * 21:59:29 L Server: ChangeDroneSettings call made to
 * 21:59:29 L DogmaIMBound::Handle_ChangeDroneSettings(): size=1
 * 21:59:29 [SvcCall]   Call Arguments:
 * 22:04:44 [SvcCall]       Tuple: 1 elements
 * 22:04:44 [SvcCall]         [ 0] Dictionary: 3 entries
 * 22:04:44 [SvcCall]         [ 0]   [ 0] Key: Integer field: 1283 <-- AttrFightersAttackAndFollow
 * 22:04:44 [SvcCall]         [ 0]   [ 0] Value: Integer field: 1
 * 22:04:44 [SvcCall]         [ 0]   [ 1] Key: Integer field: 1275 <-- AttrDroneIsAgressive
 * 22:04:44 [SvcCall]         [ 0]   [ 1] Value: Integer field: 1
 * 22:04:44 [SvcCall]         [ 0]   [ 2] Key: Integer field: 1297 <-- AttrDroneFocusFire
 * 22:04:44 [SvcCall]         [ 0]   [ 2] Value: Integer field: 1
 *
 *    sLog.Log("DogmaIMBound::Handle_ChangeDroneSettings()", "size=%u", call.tuple->size());
 *    call.Dump(SERVICE__CALLS);
 */

/*
            [PyString "ChangeDroneSettings"]
            [PyTuple 1 items]
              [PyDict 3 kvp]
                [PyInt 1283]
                [PyFloat 1]
                [PyInt 1297]
                [PyFloat 0]
                [PyInt 1275]
                [PyFloat 1]
        */

/*
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 3 items]
        [PyInt 8]
        [PyString "entity"]
        [PyNone]
    [PyInt 5654387]
    [PyTuple 1 items]
      [PyTuple 2 items]
        [PyInt 0]
        [PySubStream 40 bytes]
          [PyTuple 4 items]
            [PyInt 1]
            [PyString "MachoResolveObject"]
            [PyTuple 2 items]
              [PyInt 30000302]
              [PyInt 0]
            [PyDict 1 kvp]
              [PyString "machoVersion"]
              [PyInt 1]


    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 1]
        [PyInt 790408]
        [PyString "entity"]
        [PyNone]
    [PyInt 5654387]
    [PyTuple 1 items]
      [PyTuple 2 items]
        [PyInt 0]
        [PySubStream 88 bytes]
          [PyTuple 4 items]
            [PyInt 1]
            [PyString "MachoBindObject"]
            [PyTuple 2 items]
              [PyInt 30000302]
              [PyTuple 3 items]
                [PyString "CmdEngage"]
                [PyTuple 2 items]
                  [PyList 3 items]
                    [PyIntegerVar 1005909240632]
                    [PyIntegerVar 1005909240642]
                    [PyIntegerVar 1005902745093]
                  [PyIntegerVar 9000000000001190094]
                [PyDict 0 kvp]
            [PyDict 1 kvp]
              [PyString "machoVersion"]
              [PyInt 1]

    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 1]
        [PyInt 790408]
        [PyString "entity"]
        [PyNone]
    [PyInt 5654387]
    [PyTuple 1 items]
      [PyTuple 2 items]
        [PyInt 0]
        [PySubStream 81 bytes]
          [PyTuple 4 items]
            [PyInt 1]
            [PyString "MachoBindObject"]
            [PyTuple 2 items]
              [PyInt 30000302]
              [PyTuple 3 items]
                [PyString "CmdReturnBay"]
                [PyTuple 1 items]
                  [PyList 3 items]
                    [PyIntegerVar 1005909240632]
                    [PyIntegerVar 1005909240642]
                    [PyIntegerVar 1005902745093]
                [PyDict 0 kvp]
            [PyDict 1 kvp]
              [PyString "machoVersion"]
              [PyInt 1]
        */