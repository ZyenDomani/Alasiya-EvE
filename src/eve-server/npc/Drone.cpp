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
    Author:     Aknor Jaden
    Rewrite:    Allan
*/

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "EntityMgr.h"
#include "inventory/AttributeEnum.h"
#include "npc/Drone.h"
#include "system/Container.h"
#include "system/Damage.h"
#include "system/DestinyManager.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"
#include "system/cosmicMgrs/AnomalyMgr.h"

DroneSE::DroneSE(InventoryItemRef drone, PyServiceMgr &services, SystemManager* pSystem, const FactionData& data)
: ObjectSystemEntity(drone, services, pSystem),
m_system(pSystem),
m_AI(new DroneAIMgr(this)),
m_pClient(nullptr),
m_pShipSE(nullptr),
m_processTimer(0),
m_online(false),
m_damaged(false),
m_controlDistance(20000),       // 20km default
m_controllerID(0),
m_controllerOwnerID(0),
m_emDamage(0.0f),
m_expDamage(0.0f),
m_kinDamage(0.0f),
m_therDamage(0.0f),
m_hullDamage(0.0f),
m_armorDamage(0.0f),
m_shieldCharge(0.0f),
m_shieldCapacity(0.0f)
{
    assert (m_AI != nullptr);
    assert (m_system != nullptr);

    m_warID = data.factionID;
    m_allyID = data.allianceID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;

    _log(DRONE__TRACE, "Created Drone object for %s (%u)", drone->name(), drone->itemID());
}

DroneSE::~DroneSE() {
    SafeDelete(m_AI);
}

void DroneSE::Init() {
    // Create default dynamic attributes in the AttributeMap:
    m_self->SetAttribute(AttrInertiaMod,          EvilOne, false);
    m_self->SetAttribute(AttrDamage,              EvilZero, false);
    m_self->SetAttribute(AttrArmorDamage,         EvilZero, false);
    m_self->SetAttribute(AttrWarpCapacitorNeed,   0.0000001, false);
    m_self->SetAttribute(AttrMass,                m_self->type().mass(), false);
    m_self->SetAttribute(AttrRadius,              m_self->type().radius(), false);
    m_self->SetAttribute(AttrVolume,              m_self->type().volume(), false);
    m_self->SetAttribute(AttrCapacity,            m_self->type().capacity(), false);
    m_self->SetAttribute(AttrShieldCharge,        m_self->GetAttribute(AttrShieldCapacity), false);
    m_self->SetAttribute(AttrCapacitorCharge,     m_self->GetAttribute(AttrCapacitorCapacity), false);

    // some drones dont have this...check and set as needed
    if (!m_self->HasAttribute(AttrOrbitRange))
        m_self->SetAttribute(AttrOrbitRange, m_self->GetAttribute(AttrFalloff), false);

    // log missing cycle attrib  (only one im concerned with here)
    if (!m_self->HasAttribute(AttrSpeed) and !m_self->HasAttribute(AttrDuration))
        sLog.Warning("Drone::Init", "%s has no AttrSpeed and AttrDuration", m_self->name());

    SetResists();

    /* Gets the value from the Drone and put on our own vars */
    m_emDamage = m_self->GetAttribute(AttrEmDamage).get_float();
    m_kinDamage = m_self->GetAttribute(AttrKineticDamage).get_float();
    m_therDamage = m_self->GetAttribute(AttrThermalDamage).get_float();
    m_expDamage = m_self->GetAttribute(AttrExplosiveDamage).get_float();
    m_hullDamage = m_self->GetAttribute(AttrDamage).get_float();
    m_armorDamage = m_self->GetAttribute(AttrArmorDamage).get_float();
    m_shieldCharge = m_self->GetAttribute(AttrShieldCharge).get_float();
    m_shieldCapacity = m_self->GetAttribute(AttrShieldCapacity).get_float();

    if (sConfig.drone.RegenShields)
        m_processTimer.Start(SHIP_PROCESS_TICK_MS);

    // initialize drone's AI manager  controller not needed for this
    m_AI->Init();

    m_pClient = sEntityMgr.FindClientByCharID(m_ownerID);
    if (m_pClient == nullptr) {
        // client/owner is not online or is logging in.  abandon drone
        m_abandoned = true;
    } else if (m_pClient->IsLogin()) {
        // client/owner is not online or is logging in.  abandon drone
        m_abandoned = true;
    } else if (m_pClient->GetShip()->typeID() == EVEDB::invTypes::Capsule) {
        // cant control drone from capsule...abandon drone
        m_abandoned = true;
    }

    if (m_abandoned) {
        m_controllerID = 0;
        m_controlDistance = 0;
        m_controllerOwnerID = 0;
        m_AI->AssignShip(nullptr);
        m_AI->Abandon();
    }
}

void DroneSE::AssignTo(Client* pClient) {
    // does drone use skills of new pilot?  config option but should
    m_pClient = pClient;
    m_controllerID = pClient->GetShipID();
    m_controllerOwnerID = m_pShipSE->GetOwnerID();
    m_AI->AssignShip(m_pShipSE);

    StateChange();
}

void DroneSE::Process() {
    if (m_killed)
        return;
    double profileStartTime(GetTimeUSeconds());

    if (m_online)
        m_AI->Process();

    if (m_processTimer.Check())
        ChargeShield();

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::drone, GetTimeUSeconds() - profileStartTime);
}

void DroneSE::SaveDrone() {
    m_self->SaveItem();
}

void DroneSE::RemoveDrone() {
    m_self->Delete();
    delete this;
}

void DroneSE::Launched(ShipSE* pShipSE) {
    // set owning ship
    m_pShipSE = pShipSE;
    AssignShip(pShipSE);

    // check for bandwidth here before onlining
    if (!pShipSE->UpdateBandwidth(this))
        return;

    SendBallData();

    // just to be sure...
    m_abandoned = false;
    m_online = true;
    m_AI->SetIdle();
}

void DroneSE::Online(ShipSE* pShipSE/*nullptr*/) {
    m_online = true;
    m_pShipSE = pShipSE;
    m_AI->AssignShip(pShipSE);

    // would this ever be null??
    if (pShipSE == nullptr)
        sLog.Error("DroneSE::Online()", "pShipSE == null");
    if (m_pClient == nullptr)
        sLog.Error("DroneSE::Online()", "m_pClient == null");

    if (pShipSE != nullptr)
        m_controlDistance = pShipSE->GetPilot()->GetChar()->GetAttribute(AttrDroneControlDistance).get_uint32();

    // do we need a state change here?  yes, set status to idle
    StateChange();
}

void DroneSE::OfflineDrone() {
    // this is called by abandon also
    m_AI->AssignShip(nullptr);
    m_online = false;

    StateChange();
}

void DroneSE::DisableDrone() {
    // should this remove bandwidth, since it's no longer online?
    //  speaking of, should ship bandwidth be changed when drones go out of control range?
    //        probably so as it's more realistic
    m_online = false;
    m_pShipSE->UpdateBandwidth(this);
    StateChange();
}

void DroneSE::Abandon() {
    m_abandoned = true;
    m_controllerID = 0;
    m_controllerOwnerID = 0;
    m_controlDistance = 0;

    // abandon before offline!
    m_AI->Abandon();
    OfflineDrone();
    StateChange();
}

void DroneSE::AssignShip(ShipSE* pShipSE) {
    m_AI->AssignShip(pShipSE);
    m_controllerID = pShipSE->GetID();
    m_controllerOwnerID = m_ownerID;
    m_controlDistance = pShipSE->GetPilot()->GetChar()->GetAttribute(AttrDroneControlDistance).get_uint32();
}

void DroneSE::ReturnBay(PyDict* dict) {
    CheckCommand(dict);
    if (!dict->empty())
        return;

    m_AI->Engage(dict, DroneAI::State::ReturnBay);
    StateChange();
}

void DroneSE::ReturnHome(PyDict* dict) {
    CheckCommand(dict);
    if (!dict->empty())
        return;

    m_AI->Engage(dict, DroneAI::State::ReturnHome);
    StateChange();
}

void DroneSE::Mine(SystemEntity* pTarget, PyDict* dict, bool repeat/*false*/) {
    CheckCommand(dict);
    if (!dict->empty())
        return;

    _log(DRONE__TRACE, "%s's %s begin mining", m_pClient->GetName(), GetName());

    m_AI->Target(pTarget);
    m_AI->Engage(dict, DroneAI::State::Mining, repeat);
}

void DroneSE::Engage(SystemEntity* pTarget, PyDict* dict) {
    CheckCommand(dict);
    if (!dict->empty())
        return;

    // target, check distances, begin attack
    m_AI->Target(pTarget);
    if (pTarget->IsNPCSE()) {
        m_AI->Engage(dict, DroneAI::State::Combat);
    } else {
        m_AI->Engage(dict, DroneAI::State::Repairing);
    }
}

void DroneSE::Assist(SystemEntity* pTarget, PyDict* dict) {
    CheckCommand(dict);
    if (!dict->empty())
        return;

    // target, check distances, begin attack
    m_AI->Target(pTarget);
    m_AI->Engage(dict, DroneAI::State::Assisting);
}

void DroneSE::Guard(SystemEntity* pTarget, PyDict* dict) {
    CheckCommand(dict);
    if (!dict->empty())
        return;

    // target, check distances, begin attack
    m_AI->Target(pTarget);
    m_AI->Engage(dict, DroneAI::State::Guarding);
}

void DroneSE::Delegate(SystemEntity* pTarget, PyDict* dict) {
    CheckCommand(dict);
    // gonna get other shit working first...
    // then deal with changing controller
}

void DroneSE::Reconnect(ShipSE* pShipSE, PyDict* dict) {
    if (m_abandoned)
        m_abandoned = false;

    // reset everything, just in case...
    m_pShipSE = pShipSE;
    m_pClient = pShipSE->GetPilot();
    m_warID = m_pClient->GetWarFactionID();
    m_allyID = m_pClient->GetAllianceID();
    m_corpID = m_pClient->GetCorporationID();
    m_ownerID = m_pClient->GetCharacterID();

    AssignShip(m_pShipSE);

    // check for bandwidth here before onlining
    if (m_pShipSE->ReconnectDrone(this)) {
        m_online = true;
        m_AI->SetIdle();
    } else {
        // make note about not enough bandwidth to online reconnected drones
        m_pShipSE->GetPilot()->SendNotifyMsg("Your %s tried reconnecting, but there is not enough bandwidth available to bring it online.<br>You can try scooping up some drones to free bandwidth, or scoop this one to cargo or drone bay.", m_self->name());
        StateChange();
    }
}

/*
 * {'messageKey': 'MiningDronesDeactivatedAsteroidEmpty', 'dataID': 17883322, 'suppressable': False, 'bodyID': 259462, 'messageType': 'notify', 'urlAudio': 'wise:/msg_MiningDronesDeactivatedAsteroidEmpty_play', 'urlIcon': '', 'titleID': None, 'messageID': 1168}
 * {'messageKey': 'MiningDronesDeactivatedCargoHoldFull', 'dataID': 17883265, 'suppressable': False, 'bodyID': 259442, 'messageType': 'notify', 'urlAudio': 'wise:/msg_MiningDronesDeactivatedCargoHoldFull_play', 'urlIcon': '', 'titleID': None, 'messageID': 1169}
 * {'messageKey': 'MiningDronesDeactivatedCargoHoldNowFull', 'dataID': 17883243, 'suppressable': False, 'bodyID': 259434, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1170}
 * {'messageKey': 'MiningDronesDeactivatedOutOfRange', 'dataID': 17883208, 'suppressable': False, 'bodyID': 259422, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1171}
 */

void DroneSE::CheckCommand(PyDict* dict) {
    // * {'FullPath': u'UI/Messages', 'messageID': 257802, 'label': u'DronesDroppedBecauseOfBandwidthModificationBody'}(u'The drone control bandwidth of your ship has been modified causing you to lose the ability to control some drones.', None, None)
    // * {'FullPath': u'UI/Messages', 'messageID': 259705, 'label': u'EntityUnknownCommandBody'}(u'{targetTypeName} does not recognize the command you are trying to give it.', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
    // * {'FullPath': u'UI/Messages', 'messageID': 259503, 'label': u'DroneTargetJammedBody'}(u'The drone is target jammed and cannot be commanded to do that.', None, None)
    // * {'FullPath': u'UI/Messages', 'messageID': 258393, 'label': u'EntityTargetWarpDisruptedBody'}(u'Control of the {[item]item.name} cannot be delegated to someone who the drones cannot warp to.', None, {u'{[item]item.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'item'}})

    // * 259606, 'label': u'EntityHasSkillPrerequisitesBody'}(u'You do not have the required {[numeric]skillCount -> "skill", "skills"} to do that. To command that drone requires having learned the following {[numeric]skillCount -> "skill", "skills"}: {requiredSkills}.', None, {u'{[numeric]skillCount -> "skill", "skills"}': {'conditionalValues': [u'skill', u'skills'], 'variableType': 9, 'propertyName': None, 'args': 320, 'kwargs': {}, 'variableName': 'skillCount'}, u'{requiredSkills}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'requiredSkills'}})

    if (m_abandoned or !m_online) {
        PyDict* data = new PyDict();
        data->SetItemString("targetTypeName", new PyString(GetName()));
        PyTuple* error = new PyTuple(2);
        error->SetItem(0, new PyString("EntityIncapacitatedCommand"));
        error->SetItem(1, data);
        dict->SetItem(new PyInt(GetID()), error);
        return;
    }

    if (m_damaged) {
        PyDict* data = new PyDict();
        data->SetItemString("targetTypeName", new PyString(GetName()));
        PyTuple* error = new PyTuple(2);
        error->SetItem(0, new PyString("EntityBrokenCommand"));
        error->SetItem(1, data);
        dict->SetItem(new PyInt(GetID()), error);
        return;
    }

    if ((m_controllerID != m_AI->GetControllerID()) or (m_pShipSE == nullptr)) {
        PyDict* data = new PyDict();
        data->SetItemString("targetTypeName", new PyString(GetName()));
        PyTuple* error = new PyTuple(2);
        error->SetItem(0, new PyString("EntityNotYoursToCommand"));
        error->SetItem(1, data);
        dict->SetItem(new PyInt(GetID()), error);
        return;
    }

    if (!InControlDistance()) {
        _log(DRONE__INFO, "%s outside control distance of %u", GetName(), m_controlDistance);
        PyDict* data = new PyDict();
        data->SetItemString("targetTypeName", new PyString(GetName()));
        data->SetItemString("distance", new PyInt(m_controlDistance));
        PyTuple* error = new PyTuple(2);
        error->SetItem(0, new PyString("EntityDistantCommand"));
        error->SetItem(1, data);
        dict->SetItem(new PyInt(GetID()), error);
        return;
    }
}


/* drone distance depends on the action
 * if drone has valid command, it is autonomous and will carry out command
 * if drone is outside control distance, communication is disabled, but drone will remain on task
 *    this is only checked when command issued
 * if task is complete, drone will return to controlling ship
 * once inside control distance, communication will begin again
 */
bool DroneSE::InControlDistance() {
    if (!sConfig.drone.StrictDistance)
        return true;

    double distance(m_AI->GetAssignedShipSE()->GetPosition().distance(GetPosition()));
    distance -= m_AI->GetAssignedShipSE()->GetRadius();
    return (distance < m_controlDistance);
}

void DroneSE::ChargeShield() {
    /* this can probably be simplified using:  (ships also)
     * recharge time = (current shield / max shield) * [479 shieldRechargeRate]    250000
     *   then just do +% each tic
     *
     * however, on live, its logarithmic, charging slower the more it fills up
     *   the above hack will be linear...same amount each tic
     */
    float Charge = m_self->GetAttribute(AttrShieldCharge).get_float();
    float Capacity = m_self->GetAttribute(AttrShieldCapacity).get_float();
    if (Charge < Capacity) {
        float newCharge = Charge + ((SHIP_PROCESS_TICK_MS / 1000) * CalculateRechargeRate(Capacity, Charge, m_self->GetAttribute(AttrShieldRechargeRate).get_float()));
        if (newCharge > Capacity) {
            newCharge = Capacity;
        } else if ((Capacity - newCharge) < 0.3) {
            newCharge = Capacity;
        }
        m_self->SetAttribute(AttrShieldCharge, newCharge);
        SendDamageStateChanged();
        _log(DRONE__MESSAGE, "DroneSE::Process(): %s(%u) - New Shield Charge: %f", m_self->GetPilot()->GetName(), m_self->itemID(), newCharge);
    }
}

float DroneSE::CalculateRechargeRate(float Capacity, float Current, float RechargeTimeMS) {
    // C = Cmax * [ 1 + ( SQRT(C0/Cmax) - 1) * EXP((t0-t1)/tau) ] ^ 2
    // dC/dt = (SQRT(C/Cmax) - C/Cmax) * 2 * Cmax / tau
    // tau = "Shield Recharge Time" / 5.0

    // prevent divide by zero.
    RechargeTimeMS = (RechargeTimeMS < 1 ? 1 : RechargeTimeMS);
    Current = (Current < 1 ? 1 : Current);
    float Cmax = (Capacity < 1 ? 1 : Capacity);

    // tau = "cap recharge time" / 5.0
    float tau = (RechargeTimeMS / 5000.0); // (50)
    // (2*Cmax) / tau
    float Cmax2_tau = ((Cmax * 2) / tau);
    // Current / Cmax
    float C_Cmax = (Current/ Cmax);
    // sqrt( Current / Cmax)
    float sC_Cmax = sqrt(C_Cmax);
    // charge rate in Gj / sec
    return (Cmax2_tau * (sC_Cmax - C_Cmax));
}


// destiny methods below...

uint32 DroneSE::GetTargetID() {
    if  (m_AI->GetTargetID() == m_controllerID)
        return 0;

    return m_AI->GetTargetID();
}

void DroneSE::StateChange() {
    //OnDroneStateChange(droneID, ownerID, controllerID, activityState, droneTypeID, controllerOwnerID, targetID)
    if (m_online) {
        OnDroneStateChange du;
            du.droneID = m_self->itemID();
            du.ownerID = m_ownerID;
            du.controllerID = m_controllerID;
            du.activityState = m_AI->GetState();
            du.droneTypeID = m_self->typeID();
            du.controllerOwnerID = m_controllerOwnerID;
            du.targetID = (GetTargetID() == 0 ? PyStatic.NewNone() : new PyInt(GetTargetID()));
        PyTuple* up(du.Encode());
        if (m_bubble != nullptr)
            m_bubble->BubblecastDestinyUpdate(&up, "destiny");
    } else {
        PyList* list = new PyList();
            list->AddItemInt(m_self->itemID());
            list->AddItem(PyStatic.NewNone());
            list->AddItem(PyStatic.NewNone());
            list->AddItem(PyStatic.NewNone());
            list->AddItem(PyStatic.NewNone());
            list->AddItem(PyStatic.NewNone());
            list->AddItem(PyStatic.NewNone());
        PyTuple* up(new PyTuple(2));
            up->SetItem(0, new PyString("OnDroneStateChange"));
            up->SetItem(1, list);
        if (m_bubble != nullptr)
            m_bubble->BubblecastDestinyUpdate(&up, "destiny");
    }
}

PyDict* DroneSE::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for Drone %u ", m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",           new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",           new PyInt(m_self->typeID()));
        slim->SetItemString("categoryID",       new PyInt(m_self->categoryID()));
        slim->SetItemString("groupID",          new PyInt(m_self->groupID()));
        slim->SetItemString("name",             new PyString(m_self->itemName()));
        slim->SetItemString("ownerID",          new PyInt(m_ownerID));
        slim->SetItemString("corpID",           IsCorpID(m_corpID) ? new PyInt(m_corpID) : PyStatic.NewNone());
        slim->SetItemString("allianceID",       IsAllianceID(m_allyID) ? new PyInt(m_allyID) : PyStatic.NewNone());
        slim->SetItemString("warFactionID",     IsFactionID(m_warID) ? new PyInt(m_warID) : PyStatic.NewNone());
        slim->SetItemString("bounty",           new PyFloat(GetBounty()));
        slim->SetItemString("securityStatus",   new PyFloat(GetSecurityRating()));
    return slim;
}

void DroneSE::EncodeDestiny(Buffer& into) {
    using namespace Destiny;

    uint8 mode(Ball::Mode::STOP);
    // drones only have 2 states...stop and orbit, and is always orbit in space
    if (m_AI->GetState() > DroneAI::State::Idle)
        mode = Ball::Mode::ORBIT;
    if (m_AI->GetState() < DroneAI::State::Combat)
        mode = Ball::Mode::STOP;

    BallHeader head = BallHeader();
        head.entityID = GetID();
        head.mode = mode;
        head.radius = GetRadius();
        head.posX = x();
        head.posY = y();
        head.posZ = z();
        head.flags = Ball::Flag::IsFree;
    into.Append(head);
    MassSector mass = MassSector();
        mass.mass = m_self->GetAttribute(AttrMass).get_double();
        mass.cloak = 0;
        mass.harmonic = m_harmonic;     // is this ever set for drones?
        mass.corporationID = m_corpID;
        mass.allianceID = (IsAllianceID(m_allyID) ? m_allyID : -1);
    into.Append(mass);
    DataSector data = DataSector();
        data.maxSpeed = m_AI->GetMaxSpeed();
        data.velX = m_AI->GetVelocityX();
        data.velY = m_AI->GetVelocityY();
        data.velZ = m_AI->GetVelocityZ();
        data.inertia = m_self->GetAttribute(AttrInertiaMod).get_float();
        data.speedfraction = m_AI->GetSpeedFraction();
    into.Append(data);
    switch (mode) {
        case Ball::Mode::ORBIT: {
            ORBIT_Struct orbit;
            orbit.targetID = m_AI->GetTargetID();
            orbit.followRange = m_AI->GetFollowDistance();
            orbit.formationID = 0xFF;
            into.Append(orbit);
       }  break;
        default: {
            STOP_Struct main;
            main.formationID = 0xFF;
            into.Append(main);
        } break;
    }

    _log(SE__DESTINY, "DroneSE::EncodeDestiny(): %s - id:%lli, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void DroneSE::MakeDamageState(DoDestinyDamageState &into) {
    into.shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float() + 5;
    into.timestamp = GetFileTimeNow();
    into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float());
}

void DroneSE::SetResists() {
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

void DroneSE::SendBallData() {
    // testing
    return;
    //  testing....send ball data packets
    std::vector<PyTuple*> updates;
    SetBallAgility sbagility;
        sbagility.entityID = GetID();
        sbagility.agility = m_self->GetAttribute(AttrInertiaMod).get_double();
    updates.push_back(sbagility.Encode());
    SetBallMass sbmass;
        sbmass.entityID = GetID();
        sbmass.mass = m_self->GetAttribute(AttrMass).get_double();
    updates.push_back(sbmass.Encode());
    SetBallSpeed sbms;
        sbms.entityID = GetID();
        sbms.speed = m_self->GetAttribute(AttrMaxVelocity).get_double();
    updates.push_back(sbms.Encode());
    if (m_bubble != nullptr)
        m_bubble->BubblecastDestinyUpdate(updates, "destiny");
}


void DroneSE::Killed(Damage &fatal_blow) {
    if ((m_bubble == nullptr) or (m_system == nullptr))
        return; // make error here?

    uint32 killerID(0);
    Client* pClient(nullptr);
    SystemEntity *killer(fatal_blow.srcSE);

    if (killer->HasPilot()) {
        pClient = killer->GetPilot();
        killerID = pClient->GetCharacterID();
    } else if (killer->IsDroneSE()) {
        pClient = sEntityMgr.FindClientByCharID( killer->GetSelf()->ownerID() );
        if (pClient == nullptr) {
            sLog.Error("DroneSE::Killed()", "killer == IsDrone and pPlayer == nullptr");
        } else {
            killerID = pClient->GetCharacterID();
        }
    } else {
        killerID = killer->GetID();
    }

    // if mining drone and carrying ore, delete ore (it is lost when drone destroyed)
    if (m_self->groupID() == EVEDB::invGroups::Mining_Drone) {
        if (m_AI->m_ore.get() != nullptr)
            m_AI->m_ore->Delete();
    }

    // are we making wrecks from drones??  nah...this will just be little bits after being shot and they dont drop anything
}


//  Notifications for AI Processing
// these should take current settings into account
/* AttrDroneIsAgressive = 1275,
 * AttrDroneIsChaotic = 1278,           // disabled in crucible
 * AttrFightersAttackAndFollow = 1283,
 * AttrDroneFocusFire = 1297,
 */

void DroneSE::ShipWarping(ShipSE* pShipSE) {

}

void DroneSE::TargetDestroyed(SystemEntity* pSE) {
    m_AI->TargetLost(pSE);
    // if aggressive, look for and engage another target
    // if focus fire, ai should communicate with each other for next target
}

void DroneSE::TargetAdded(SystemEntity* pSE) {
    // pass  we alredy know when we added a target
}

void DroneSE::TargetLost(SystemEntity* pSE) {
    m_AI->TargetLost(pSE);
    // if aggressive, look for and engage another target
    // if focus fire, ai should communicate with each other for next target
}

void DroneSE::TargetedAdd(SystemEntity* pSE) {
    m_AI->Targeted(pSE);
    // if aggressive or chaotic, attack this target
}

void DroneSE::TargetedLost(SystemEntity* pSE) {
    // pass  dont care if we were unlocked
}

void DroneSE::MissileLaunched(Missile* pMissile) {
    //SystemEntity::MissileLaunched(pMissile);
    m_AI->MissileLaunched(pMissile);
}


void DroneSE::ReportDamage(uint8 type/*0*/) {
    //SystemEntity::ReportDamage(type);
    m_AI->ReportDamage(type);
}

// will need a notification method of assigned ship being fired upon
