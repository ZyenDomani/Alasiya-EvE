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
    Rewrite:    Allan
*/

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "EntityList.h"
#include "inventory/AttributeEnum.h"
#include "system/DestinyManager.h"
#include "npc/Drone.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"
#include "system/cosmicMgrs/AnomalyMgr.h"

Drone::Drone(InventoryItemRef drone, PyServiceMgr &services, SystemManager* pSystem, const FactionData& data)
: DynamicSystemEntity(drone, services, pSystem),
  m_AI(new DroneAIMgr(this)),
  m_system(pSystem)
{
    assert (m_AI != nullptr);

    m_online = false;

    m_warID = data.factionID;
    m_allyID = data.allianceID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;
    m_pClient = sEntityList.FindClientByCharID(data.ownerID);
    m_targetID = 0;
    m_controllerID = 0;
    m_controllerOwnerID = 0;

    m_orbitRange = m_self->GetAttribute(AttrOrbitRange).get_float();
    if (m_orbitRange < 1) {
        if (m_self->GetAttribute(AttrMaxRange) < m_self->GetAttribute(AttrFalloff))
            m_orbitRange = m_self->GetAttribute(AttrMaxRange).get_float();
        else
            m_orbitRange = m_self->GetAttribute(AttrFalloff).get_float();
    }

    // Create default dynamic attributes in the AttributeMap:
    m_self->SetAttribute(AttrInertia,             EvilOne, false);
    m_self->SetAttribute(AttrDamage,              EvilZero, false);
    m_self->SetAttribute(AttrArmorDamage,         EvilZero, false);
    m_self->SetAttribute(AttrWarpCapacitorNeed,   0.00001, false);
    m_self->SetAttribute(AttrOrbitRange,          m_orbitRange, false);
    m_self->SetAttribute(AttrMass,                m_self->type().mass(), false);
    m_self->SetAttribute(AttrRadius,              m_self->type().radius(), false);
    m_self->SetAttribute(AttrVolume,              m_self->type().volume(), false);
    m_self->SetAttribute(AttrCapacity,            m_self->type().capacity(), false);
    m_self->SetAttribute(AttrShieldCharge,        m_self->GetAttribute(AttrShieldCapacity), false);
    m_self->SetAttribute(AttrCapacitorCharge,     m_self->GetAttribute(AttrCapacitorCapacity), false);

    m_destiny->UpdateShipVariables();

    SetResists();

    /* Gets the value from the NPC and put on our own vars */
    m_emDamage = m_self->GetAttribute(AttrEmDamage).get_float(),
    m_kinDamage = m_self->GetAttribute(AttrKineticDamage).get_float(),
    m_therDamage = m_self->GetAttribute(AttrThermalDamage).get_float(),
    m_expDamage = m_self->GetAttribute(AttrExplosiveDamage).get_float(),
    m_hullDamage = m_self->GetAttribute(AttrDamage).get_float();
    m_armorDamage = m_self->GetAttribute(AttrArmorDamage).get_float();
    m_shieldCharge = m_self->GetAttribute(AttrShieldCharge).get_float();
    m_shieldCapacity = m_self->GetAttribute(AttrShieldCapacity).get_float();

    _log(DRONE__TRACE, "Created Drone object for %s (%u)", drone.get()->itemName().c_str(), drone.get()->itemID());
}

Drone::~Drone() {
    SafeDelete(m_AI);
}

void Drone::SetOwner(Client* pClient) {
    m_self->ChangeOwner(pClient->GetCharacterID());
    m_pClient = pClient;
    m_ownerID = pClient->GetCharacterID();
    m_corpID = pClient->GetCorporationID();
    m_allyID = pClient->GetAllianceID();
    m_warID = pClient->GetWarFactionID();
    m_pShipSE = pClient->GetShipSE();
    m_controllerID = pClient->GetShipID();
    m_controllerOwnerID = m_pShipSE->GetOwnerID();
}

void Drone::Process() {
    if (m_killed)
        return;
    double profileStartTime = GetTimeUSeconds();

    /*  Enable base call to Process Targeting and Movement  */
    SystemEntity::Process();

    /** @todo (allan) finish drone AI and processing */
    if (m_online)
        m_AI->Process();

    if (sConfig.debug.UseProfiling)
        sProfile.AddTime(droneProfile, GetTimeUSeconds() - profileStartTime);
}

void Drone::Orbit(SystemEntity *who) {
    if (who == nullptr)
        m_targetID = 0;
    else
        m_targetID = who->GetID();
}

void Drone::SaveDrone() {
    m_self->SaveItem();
}

void Drone::RemoveDrone() {
    /** @todo (Allan) this may need more here */
    m_self->Delete();
    delete this;
}

void Drone::Launch( Ship* pShipSE ) {
    m_online = true;
    m_pShipSE = pShipSE;

    m_controllerID = pShipSE->GetID();
    m_controllerOwnerID = pShipSE->GetOwnerID();

    m_system->AddEntity(this);
    m_system->GetAnomMgr()->AddAnomaly(m_self);
}

void Drone::Online() {
    StateChange(true);
    
    // set speed and begin orbit
    m_destiny->SetMaxVelocity(500);
    m_destiny->SetSpeedFraction(0.6f);
    m_destiny->Orbit(m_pShipSE, m_orbitRange);
}

/*   when drone is scooped up....
 *
 *                    [PyTuple 2 items]
 *                      [PyString "OnDroneStateChange"]
 *                      [PyList 7 items]
 *                        [PyIntegerVar 1540263056]
 *                        [PyNone]
 *                        [PyNone]
 *                        [PyNone]
 *                        [PyNone]
 *                        [PyNone]
 *                        [PyNone]
 */

void Drone::StateChange(bool online/*false*/) {
    //OnDroneStateChange(droneID, ownerID, controllerID, activityState, droneTypeID, controllerOwnerID, targetID)
    if (online) {
        OnDroneStateChange du;
            du.droneID = m_self->itemID();
            du.ownerID = m_ownerID;
            du.droneTypeID = m_self->typeID();
            du.controllerID = m_controllerID;
            du.controllerOwnerID = m_controllerOwnerID;
            du.activityState = m_AI->GetState();
            du.targetID = m_targetID;
        PyTuple* up = du.Encode();
        // bubblecast is faster than destiny::update
        m_bubble->BubblecastDestinyUpdate(&up, "destiny");
        //pShipSE->DestinyMgr()->SendSingleDestinyUpdate(&up);
    } else {
        PyList* list = new PyList();
            list->AddItemInt(m_self->itemID());
            list->AddItem(PyStatic.NewNone());
            list->AddItem(PyStatic.NewNone());
            list->AddItem(PyStatic.NewNone());
            list->AddItem(PyStatic.NewNone());
            list->AddItem(PyStatic.NewNone());
            list->AddItem(PyStatic.NewNone());
        PyTuple* tuple = new PyTuple(2);
            tuple->SetItem(0, new PyString("OnDroneStateChange"));
            tuple->SetItem(1, list);
        m_bubble->BubblecastDestinyUpdate(&tuple, "destiny");
    }
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

PyDict* Drone::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for Drone %u ", m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",           new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",           new PyInt(m_self->typeID()));
        slim->SetItemString("categoryID",       new PyInt(m_self->categoryID()));
        slim->SetItemString("groupID",          new PyInt(m_self->groupID()));
        slim->SetItemString("name",             new PyString(m_self->itemName()));
        slim->SetItemString("ownerID",          new PyInt(m_ownerID));
        slim->SetItemString("corpID",           IsCorp(m_corpID) ? new PyInt(m_corpID) : PyStatic.NewNone());
        slim->SetItemString("allianceID",       IsAlliance(m_allyID) ? new PyInt(m_allyID) : PyStatic.NewNone());
        slim->SetItemString("warFactionID",     IsFaction(m_warID) ? new PyInt(m_warID) : PyStatic.NewNone());
        slim->SetItemString("bounty",           new PyFloat(GetBounty()));
        slim->SetItemString("securityStatus",   new PyFloat(GetSecurityRating()));
    return slim;
}

void Drone::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;

    uint8 mode = Ball::Mode::STOP;
    if (m_destiny->IsWarping())
        mode = Ball::Mode::WARP;
    else if (m_destiny->IsFollowing())
        mode = Ball::Mode::FOLLOW;
    else if (m_destiny->IsOrbiting())
        mode = Ball::Mode::ORBIT;
    else if (m_destiny->IsMoving())
        mode = Ball::Mode::GOTO;

    // drone id's WILL be int64 (> 1000000000000L)
    BallHeader head = BallHeader();
        head.entityID = GetID();
        head.mode = mode;
        head.radius = GetRadius();
        head.x = x();
        head.y = y();
        head.z = z();
        head.flags = Ball::Flag::IsFree;
    into.Append( head );
    MassSector mass = MassSector();
        mass.mass = m_destiny->GetMass();
        mass.cloak = 0;
        mass.harmonic = m_harmonic;
        mass.corporationID = m_corpID;
        mass.allianceID = (m_allyID > 0 ? m_allyID : -1);
    into.Append( mass );
    DataSector data = DataSector();
        data.maxVelocity = m_destiny->GetMaxVelocity();
        data.velocity_x = m_destiny->GetVelocity().x;
        data.velocity_y = m_destiny->GetVelocity().y;
        data.velocity_z = m_destiny->GetVelocity().z;
        data.inertia = m_destiny->GetInertia();
        data.speedfraction = m_destiny->GetSpeedFraction();
    into.Append( data );
    switch (mode) {
        case Ball::Mode::WARP: {
            GPoint target = m_destiny->GetTargetPoint();
            WARP_Struct warp;
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
        case Ball::Mode::FOLLOW: {
            FOLLOW_Struct follow;
                follow.followID = m_destiny->GetTargetID();
                follow.followRange = m_destiny->GetFollowDistance();
                follow.formationID = 0xFF;
            into.Append( follow );
        }  break;
        case Ball::Mode::ORBIT: {
            ORBIT_Struct orbit;
                orbit.followID = m_destiny->GetTargetID();
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
        default: {
            STOP_Struct main;
                main.formationID = 0xFF;
            into.Append( main );
        } break;
    }
    _log(SE__DESTINY, "Drone::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void Drone::MakeDamageState(DoDestinyDamageState &into)
{
    into.shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float() +5;
    into.timestamp = GetFileTimeNow();
    into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float());
}

void Drone::SetResists() {
    /* fix for missing resist attribs -allan 18April16  */
    if (!m_self->HasAttribute(AttrShieldEmDamageResonance)) m_self->SetAttribute(AttrShieldEmDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrShieldExplosiveDamageResonance)) m_self->SetAttribute(AttrShieldExplosiveDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrShieldKineticDamageResonance)) m_self->SetAttribute(AttrShieldKineticDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrShieldThermalDamageResonance)) m_self->SetAttribute(AttrShieldThermalDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrArmorEmDamageResonance)) m_self->SetAttribute(AttrArmorEmDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrArmorExplosiveDamageResonance)) m_self->SetAttribute(AttrArmorExplosiveDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrArmorKineticDamageResonance)) m_self->SetAttribute(AttrArmorKineticDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrArmorThermalDamageResonance)) m_self->SetAttribute(AttrArmorThermalDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrEmDamageResonance)) m_self->SetAttribute(AttrEmDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrExplosiveDamageResonance)) m_self->SetAttribute(AttrExplosiveDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrKineticDamageResonance)) m_self->SetAttribute(AttrKineticDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrThermalDamageResonance)) m_self->SetAttribute(AttrThermalDamageResonance, EvilOne, false);
}

/*
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