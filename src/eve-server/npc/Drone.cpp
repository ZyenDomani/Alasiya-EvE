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

#include "../eve-server.h"

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
m_controllerOwnerID(0)
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
//AttrDroneMaxVelocityBonus
void DroneSE::Init() {
    // Create default dynamic attributes in the AttributeMgr:
    m_self->SetAttribute(AttrDamage,              EvilZero, false);
    m_self->SetAttribute(AttrArmorDamage,         EvilZero, false);
    m_self->SetAttribute(AttrMass,                m_self->type().mass(), false);
    m_self->SetAttribute(AttrRadius,              m_self->type().radius(), false);
    m_self->SetAttribute(AttrVolume,              m_self->type().volume(), false);
    m_self->SetAttribute(AttrShieldCharge,        m_self->type().GetAttribute(AttrShieldCapacity), false);
    m_self->SetAttribute(AttrCapacitorCharge,     m_self->type().GetAttribute(AttrCapacitorCapacity), false);

    // log missing cycle attrib  (only one im concerned with here)
    if (!m_self->HasAttribute(AttrSpeed) and !m_self->HasAttribute(AttrDuration))
        sLog.Warning("Drone::Init", "%s has no AttrSpeed and AttrDuration", m_self->name());

    SetResists();

    if (sConfig.drone.RegenShields)
        m_processTimer.Start(SHIP_PROCESS_TICK_MS);

    m_pClient = sEntityMgr.FindClientByCharID(m_ownerID);
    if (m_pClient == nullptr) {
        // client/owner is not online or is logging in.  abandon drone
        m_abandoned = true;
    } else if (m_pClient->IsLogin()) {
        // client/owner is not online or is logging in.  abandon drone
        m_abandoned = true;
    } else if (m_pClient->GetShip()->typeID() == EVEItemTypes::Capsule) {
        // cant control drone from capsule...abandon drone
        m_abandoned = true;
    }

    if (m_abandoned) {
        m_controllerID = 0;
        m_controlDistance = 0;
        m_controllerOwnerID = 0;
        m_AI->AssignShip(nullptr);
        StateChange();
    }

    // initialize drone's AI manager  controller not needed for this
    m_AI->Init();
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
    double profileStartTime = GetTimeUSeconds();

    if (m_online) {
        m_AI->Process();

        if (m_processTimer.Check())
            ChargeShield();
    }

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

void DroneSE::Launch(ShipSE* pShipSE) {
    m_abandoned = false;
    // set owning ship
    m_pShipSE = pShipSE;
    AssignShip(pShipSE);

    // this one's weird...gotta add to system either way, but can only online if ship has bandwidth
    //  however, we have to online before adding or the shields are wrong.
    if (pShipSE->AcquireBandwidth(this)) {
        Online();
        // add drone to system, and add signal to AnomalyMgr...drones are scannable
        m_system->AddEntity(this, true);
    } else {
        // add drone to system, and add signal to AnomalyMgr...drones are scannable
        m_system->AddEntity(this, true);
        // not enough bandwidth, so no online.  add to system and return
        return;
    }

    // ok, enough bandwidth, onlined, added to system, so now set idle.
    m_AI->SetIdle();
    if (m_pClient->AutoAttack())
        m_self->SetAttribute(AttrDroneIsChaotic, true, false);
}

void DroneSE::Online() {
    m_online = true;

    // would this ever be null??
    if (m_pShipSE == nullptr)
        sLog.Error("DroneSE::Online()", "m_pShipSE == null");
    if (m_pClient == nullptr)
        sLog.Error("DroneSE::Online()", "m_pClient == null");

    if (m_pShipSE != nullptr)
        m_controlDistance = m_pShipSE->GetPilot()->GetChar()->GetAttribute(AttrDroneControlDistance).get_uint32();

    UpdateDroneWithSkills();
    SendBallData();
    // do we need a state change here?  yes, set status to idle
    StateChange();
}

void DroneSE::OfflineDrone() {
    // this is called by abandon also
    m_AI->AssignShip(nullptr);
    m_online = false;

    if (!m_abandoned)
        m_self->ResetAttributes();
    StateChange();
}

void DroneSE::DisableDrone() {
    // should this remove bandwidth, since it's no longer online?
    //  speaking of, should ship bandwidth be changed when drones go out of control range?
    //        probably so as it's more realistic
    m_online = false;
    m_pShipSE->ReleaseBandwidth(this);
    m_self->ResetAttributes();
    StateChange();
}

void DroneSE::Abandon() {
    // if ship is docking with drones in space, ShipSE::AbandonDrone(DroneSE*) will invalidate the iterator in ShipSE::AbandonDrones()
    if ((m_pShipSE != nullptr) and (!m_pShipSE->GetShipItemRef()->IsDocking()))
        m_pShipSE->AbandonDrone(this);
    m_pClient = nullptr;
    m_abandoned = true;
    m_controllerID = 0;
    m_controllerOwnerID = 0;
    m_controlDistance = 0;

    // abandon before offline!
    m_AI->Abandon();
    OfflineDrone();
}

void DroneSE::AssignShip(ShipSE* pShipSE) {
    m_AI->AssignShip(pShipSE);
    m_controllerID = pShipSE->GetID();
    m_controllerOwnerID = m_ownerID;
    m_controlDistance = pShipSE->GetPilot()->GetChar()->GetAttribute(AttrDroneControlDistance).get_uint32();
}

void DroneSE::ReturnBay(PyDict* dict) {
    if (!CheckCommand(dict))
        return;

    m_AI->Engage(dict, DroneAI::State::ReturnBay);
    StateChange();
}

void DroneSE::ReturnHome(PyDict* dict) {
    if (!CheckCommand(dict))
        return;

    m_AI->Engage(dict, DroneAI::State::ReturnHome);
    StateChange();
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

    AssignShip(pShipSE);

    // check for bandwidth here before onlining
    if (pShipSE->ReconnectDrone(this)) {
        Online();
        m_AI->SetIdle();
        pShipSE->AddDroneToMap(this);
    } else {
        // make note about not enough bandwidth to online reconnected drones
        pShipSE->GetPilot()->SendNotifyMsg("Your %s tried reconnecting, but there is not enough bandwidth available to bring it online.<br>You can try scooping up some drones to free bandwidth, or scoop this one either to your cargo bay or drone bay.", m_self->name());
        StateChange();
    }
}

void DroneSE::Assist(SystemEntity* pTargetSE, PyDict* dict) {
    if (!CheckCommand(dict))
        return;
    if (!CheckTarget(pTargetSE, dict))
        return;

    // target, check mode and distances, look for threats
    m_AI->AssignShip(pTargetSE->GetShipSE());
    m_AI->Engage(dict, DroneAI::State::Assisting);
}

void DroneSE::Guard(SystemEntity* pTargetSE, PyDict* dict) {
    if (!CheckCommand(dict))
        return;
    if (!CheckTarget(pTargetSE, dict))
        return;

    // target, check mode and distances, look for threats
    m_AI->AssignShip(pTargetSE->GetShipSE());
    m_AI->Engage(dict, DroneAI::State::Guarding);
}

void DroneSE::Mine(SystemEntity* pTargetSE, PyDict* dict, bool repeat/*false*/) {
    if (!CheckCommand(dict))
        return;
    if (!CheckTarget(pTargetSE, dict))
        return;

    m_AI->Target(pTargetSE);
    m_AI->Engage(dict, DroneAI::State::Mining, repeat);
}

void DroneSE::Engage(SystemEntity* pTargetSE, PyDict* dict/*nullptr*/) {
    if (dict != nullptr)
        if (!CheckCommand(dict))
            return;

    switch (GetGroupID()) {
        case EVEDB::invGroups::Mining_Drone:  {
            if (pTargetSE->IsAsteroidSE()) {
                if (!m_bubble->IsIce()) {
                    m_AI->Target(pTargetSE);
                    m_AI->Engage(dict, DroneAI::State::Mining);
                    return;
                } // else trying to mine ice with drone...nope
            } // else command unknown
        } break;
        case EVEDB::invGroups::Combat_Drone:
        case EVEDB::invGroups::Fighter_Drone:
        case EVEDB::invGroups::Fighter_Bomber:
        case EVEDB::invGroups::Cap_Drain_Drone:
        case EVEDB::invGroups::Stasis_Webifying_Drone:
        case EVEDB::invGroups::Electronic_Warfare_Drone:  {
            if (pTargetSE->IsNPCSE() or pTargetSE->IsDroneSE()) {
                m_AI->Target(pTargetSE);
                m_AI->Engage(dict, DroneAI::State::Combat);
                return;
            } else if (pTargetSE->IsShipSE()) {
                // are we fighting or trying to repair?
                if (!IsFleetID(pTargetSE->GetFleetID())
                or (pTargetSE->GetFleetID() != m_pClient->GetFleetID())) {
                    // they're not in our fleet...this is combat call
                    m_AI->Target(pTargetSE);
                    m_AI->Engage(dict, DroneAI::State::Combat);
                    return;
                } // in our fleet...probably repair call
                if (pTargetSE->HasPilot()) {
                    if ((IsPlayerCorp(pTargetSE->GetPilot()->GetCorporationID())                        // is player corp
                        and (pTargetSE->GetPilot()->GetCorporationID() != m_pClient->GetCorporationID()))   //that we're not in
                    or (IsAllianceID(pTargetSE->GetPilot()->GetAllianceID())                      // or is alliance
                        and (pTargetSE->GetPilot()->GetAllianceID() != m_pClient->GetAllianceID()))) { // that we're not in
                        // oh, looks like war...
                        m_AI->Target(pTargetSE);
                        m_AI->Engage(dict, DroneAI::State::Combat);
                        return;
                    }// same corp/ally.  probably repair call
                } // npc or empty ship...
                else if ((IsPlayerCorp(pTargetSE->GetOwnerID())                        // is player corp
                    and (pTargetSE->GetOwnerID() != m_pClient->GetCorporationID()))   //that we're not in
                or (IsAllianceID(pTargetSE->GetAllianceID())                      // or is alliance
                    and (pTargetSE->GetAllianceID() != m_pClient->GetAllianceID()))) { // that we're not in
                    // oh, looks like war...
                    m_AI->Target(pTargetSE);
                    m_AI->Engage(dict, DroneAI::State::Combat);
                    return;
                }// same corp/ally.  probably repair call
            } else if (pTargetSE->IsPOSSE()) {
                // this is player structure.  is it ours and we're repairing or enemy and we're attacking?
                if ((IsPlayerCorp(pTargetSE->GetOwnerID())                        // is player corp
                    and (pTargetSE->GetOwnerID() != m_pClient->GetCorporationID()))   //that we're not in
                or (IsAllianceID(pTargetSE->GetAllianceID())                      // or is alliance
                    and (pTargetSE->GetAllianceID() != m_pClient->GetAllianceID()))) { // that we're not in
                    // oh, looks like war...
                    m_AI->Target(pTargetSE);
                    m_AI->Engage(dict, DroneAI::State::Combat);
                    return;
                }// same corp/ally.  probably repair call
            } // else command unknown
        } break;
        case EVEDB::invGroups::Logistic_Drone: {
            if (pTargetSE->IsShipSE()) {
                // are we fighting or trying to repair?
                if (IsFleetID(pTargetSE->GetFleetID())
                and (pTargetSE->GetFleetID() == m_pClient->GetFleetID())) {
                    // they're in our fleet...this is repair call
                    m_AI->Target(pTargetSE);
                    m_AI->Engage(dict, DroneAI::State::Repairing);
                    return;
                } // not in our fleet...probably combat call
            } else if (pTargetSE->IsPOSSE()) {
                // this is player structure.  is it ours and we're repairing or enemy and we're attacking?
                if (IsPlayerCorp(pTargetSE->GetOwnerID())) {
                    // ok, so its' a corp structure.  ours or theirs?
                    if (pTargetSE->GetOwnerID() == m_pClient->GetCorporationID()) {
                        // ok, our corp...allow repair
                        m_AI->Target(pTargetSE);
                        m_AI->Engage(dict, DroneAI::State::Repairing);
                        return;
                    } else if (IsAllianceID(pTargetSE->GetAllianceID())
                    and (pTargetSE->GetAllianceID() == m_pClient->GetAllianceID())) {
                        // oh, our alliance...allow repair
                        m_AI->Target(pTargetSE);
                        m_AI->Engage(dict, DroneAI::State::Repairing);
                        return;
                    }// diff corp/ally.  probably attack call
                } else if (pTargetSE->GetOwnerID() == m_pClient->GetCharID()) {
                    // ok, ours...allow repair
                    m_AI->Target(pTargetSE);
                    m_AI->Engage(dict, DroneAI::State::Repairing);
                    return;
                } else if (IsFleetID(pTargetSE->GetFleetID())
                and (pTargetSE->GetFleetID() == m_pClient->GetFleetID())) {
                    /* this is odd one....POS belonging to another corp/ally, but member is in our fleet.
                     *   what a dilemma...how about this:
                     * if no forcefield, allow it
                     * if we can pass the field, then allow it.
                     * if we cannot pass, then deny
                     *    actually, at this point, the forcefield checks have passed (in entitysevice call)
                     */
                    m_AI->Target(pTargetSE);
                    m_AI->Engage(dict, DroneAI::State::Repairing);
                    return;
                } // else this is an attack
            } // else command unknown
        } break;
    }

    if ((dict != nullptr) and !dict->empty()) {
        PyDict* data = new PyDict();
        data->SetItemString("targetTypeName", new PyString(GetName()));
        PyTuple* error = new PyTuple(2);
        error->SetItem(0, new PyString("EntityUnknownCommand"));
        error->SetItem(1, data);
        dict->SetItem(new PyInt(GetID()), error);
    }
}

void DroneSE::Delegate(SystemEntity* pSE, PyDict* dict) {
    if (!CheckCommand(dict))
        return;
    // need to do more checks here...
    if (!pSE->IsShipSE())
        return;

    // so we are handing control to another pilot...
    AssignShip(pSE->GetShipSE());
    m_AI->SetIdle();
    // how to we tell pilot they now have control?
}

/*  not sure if/how to implement these...
 * {'messageKey': 'MiningDronesDeactivatedAsteroidEmpty', 'dataID': 17883322, 'suppressable': False, 'bodyID': 259462, 'messageType': 'notify', 'urlAudio': 'wise:/msg_MiningDronesDeactivatedAsteroidEmpty_play', 'urlIcon': '', 'titleID': None, 'messageID': 1168}
 * {'messageKey': 'MiningDronesDeactivatedCargoHoldFull', 'dataID': 17883265, 'suppressable': False, 'bodyID': 259442, 'messageType': 'notify', 'urlAudio': 'wise:/msg_MiningDronesDeactivatedCargoHoldFull_play', 'urlIcon': '', 'titleID': None, 'messageID': 1169}
 * {'messageKey': 'MiningDronesDeactivatedCargoHoldNowFull', 'dataID': 17883243, 'suppressable': False, 'bodyID': 259434, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1170}
 * {'messageKey': 'MiningDronesDeactivatedOutOfRange', 'dataID': 17883208, 'suppressable': False, 'bodyID': 259422, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1171}
 * {'FullPath': u'UI/Messages', 'messageID': 257802, 'label': u'DronesDroppedBecauseOfBandwidthModificationBody'}(u'The drone control bandwidth of your ship has been modified causing you to lose the ability to control some drones.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 258393, 'label': u'EntityTargetWarpDisruptedBody'}(u'Control of the {[item]item.name} cannot be delegated to someone who the drones cannot warp to.', None, {u'{[item]item.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'item'}})
 *
 */

void DroneSE::SetAutoAttack(bool set) {
    m_self->SetAttribute(AttrDroneIsChaotic, set, false);
}

void DroneSE::TargetAdded(SystemEntity* pTargetSE) {
    if (!m_online)
        return;

    // for starters, are we currently idle?
    if (!m_AI->IsIdle())
        return;

    // are we auto-engage on target add?
    if (m_pClient->AutoAttack()) {
        Engage(pTargetSE);
        return;
    }

    // send it to AI for further processing
    m_AI->ShipAddedTarget(pTargetSE);
}

bool DroneSE::CheckCommand(PyDict* dict) {
    // * 259606, 'label': u'EntityHasSkillPrerequisitesBody'}(u'You do not have the required {[numeric]skillCount -> "skill", "skills"} to do that. To command that drone requires having learned the following {[numeric]skillCount -> "skill", "skills"}: {requiredSkills}.', None, {u'{[numeric]skillCount -> "skill", "skills"}': {'conditionalValues': [u'skill', u'skills'], 'variableType': 9, 'propertyName': None, 'args': 320, 'kwargs': {}, 'variableName': 'skillCount'}, u'{requiredSkills}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'requiredSkills'}})

    if (m_abandoned or !m_online) {
        PyDict* data = new PyDict();
        data->SetItemString("targetTypeName", new PyString(GetName()));
        PyTuple* error = new PyTuple(2);
        error->SetItem(0, new PyString("EntityIncapacitatedCommand"));
        error->SetItem(1, data);
        dict->SetItem(new PyInt(GetID()), error);
        return false;
    }

    if (m_damaged) {
        PyDict* data = new PyDict();
        data->SetItemString("targetTypeName", new PyString(GetName()));
        PyTuple* error = new PyTuple(2);
        error->SetItem(0, new PyString("EntityBrokenCommand"));
        error->SetItem(1, data);
        dict->SetItem(new PyInt(GetID()), error);
        return false;
    }

    if ((m_controllerID != m_AI->GetControllerID()) or (m_pShipSE == nullptr)) {
        PyDict* data = new PyDict();
        data->SetItemString("targetTypeName", new PyString(GetName()));
        PyTuple* error = new PyTuple(2);
        error->SetItem(0, new PyString("EntityNotYoursToCommand"));
        error->SetItem(1, data);
        dict->SetItem(new PyInt(GetID()), error);
        return false;
    }

    // check for target jammed
    // u'DroneTargetJammedBody'}(u'The drone is target jammed and cannot be commanded to do that.', None, None)


    if (!InControlDistance()) {
        _log(DRONE__INFO, "%s outside control distance of %u", GetName(), m_controlDistance);
        PyDict* data = new PyDict();
        data->SetItemString("targetTypeName", new PyString(GetName()));
        data->SetItemString("distance", new PyInt(m_controlDistance));
        PyTuple* error = new PyTuple(2);
        error->SetItem(0, new PyString("EntityDistantCommand"));
        error->SetItem(1, data);
        dict->SetItem(new PyInt(GetID()), error);
        return false;
    }

    return true;
}

bool DroneSE::CheckTarget(SystemEntity* pTargetSE, PyDict* dict) {
    // run generic target checks against drone type for verification
    switch (GetGroupID()) {
        case EVEDB::invGroups::Mining_Drone:  {
            if (pTargetSE->IsAsteroidSE())
                return true;
        } break;
        case EVEDB::invGroups::Combat_Drone:
        case EVEDB::invGroups::Fighter_Drone:
        case EVEDB::invGroups::Fighter_Bomber:
        case EVEDB::invGroups::Logistic_Drone:
        case EVEDB::invGroups::Cap_Drain_Drone:
        case EVEDB::invGroups::Stasis_Webifying_Drone:
        case EVEDB::invGroups::Electronic_Warfare_Drone:  {
            if (pTargetSE->IsNPCSE() or pTargetSE->IsDroneSE() or pTargetSE->IsShipSE())
                return true;
        } break;
    }

    PyDict* data = new PyDict();
    data->SetItemString("targetTypeName", new PyString(GetName()));
    PyTuple* error = new PyTuple(2);
    error->SetItem(0, new PyString("EntityUnknownCommand"));
    error->SetItem(1, data);
    dict->SetItem(new PyInt(GetID()), error);
    return false;
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

void DroneSE::RepairInBay() {
    //TODO:  finish this
    /* recharge time = (current hp / max hp) * [some timeframe...config maybe]
     *   then just do +% each tic
     */

}

void DroneSE::ChargeShield() {
    // on live, this is logarithmic, charging slower the more it fills up
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
        _log(DRONE__MESSAGE, "DroneSE::Process(): %s(%u) - New Shield Charge: %f", m_self->name(), m_self->itemID(), newCharge);
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
            m_bubble->BubblecastDestinyUpdate(&up, "Drone State Change (online)");
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
            m_bubble->BubblecastDestinyUpdate(&up, "Drone State Change (offline)");
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
        mass.mass = m_self->mass();
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
    float charge(m_self->GetAttribute(AttrShieldCharge).get_float());
    float capy(m_self->GetAttribute(AttrShieldCapacity).get_float());
    into.shield = (charge / capy);
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float() + 5;
    into.timestamp = GetFileTimeNow();
    into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float());
}

void DroneSE::SetResists() {
    /* fix for missing resist attribs -allan 18April16  */
    if (!m_self->HasAttribute(AttrShieldEmDamageResonance))
        m_self->SetAttribute(AttrShieldEmDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrShieldExplosiveDamageResonance))
        m_self->SetAttribute(AttrShieldExplosiveDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrShieldKineticDamageResonance))
        m_self->SetAttribute(AttrShieldKineticDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrShieldThermalDamageResonance))
        m_self->SetAttribute(AttrShieldThermalDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrArmorEmDamageResonance))
        m_self->SetAttribute(AttrArmorEmDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrArmorExplosiveDamageResonance))
        m_self->SetAttribute(AttrArmorExplosiveDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrArmorKineticDamageResonance))
        m_self->SetAttribute(AttrArmorKineticDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrArmorThermalDamageResonance))
        m_self->SetAttribute(AttrArmorThermalDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrEmDamageResonance))
        m_self->SetAttribute(AttrEmDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrExplosiveDamageResonance))
        m_self->SetAttribute(AttrExplosiveDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrKineticDamageResonance))
        m_self->SetAttribute(AttrKineticDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrThermalDamageResonance))
        m_self->SetAttribute(AttrThermalDamageResonance, EvilOne, false);
}

void DroneSE::UpdateDroneWithSkills() {
    bool update(!m_abandoned);

    // first, start with basic skills applicable to all drones...
    //Drone Sharpshooting     Increases drone optimal range. (maxrange)
    float newValue(m_self->GetAttribute(AttrMaxRange).get_float());
    newValue *= (1 + (0.05f * (m_pShipSE->GetPilot()->GetChar()->GetSkillLevel(EvESkill::DroneSharpshooting, true))));
    m_self->SetAttribute(AttrMaxRange, newValue, update);
    //Drone Navigation    5% increase in drone MicroWarpdrive speed per level.
    newValue = m_self->GetAttribute(AttrMaxVelocity).get_float();
    newValue *= (1 + (0.05f * (m_pShipSE->GetPilot()->GetChar()->GetSkillLevel(EvESkill::DroneNavigation, true))));
    m_self->SetAttribute(AttrMaxVelocity, newValue, update);
        //Drone Interfacing   20% bonus to drone damage, drone mining yield per level.
    if (m_self->groupID() == EVEDB::invGroups::Mining_Drone) {
        newValue = m_self->GetAttribute(AttrMiningAmount).get_float();
        newValue *= (1 + (0.2f * (m_pShipSE->GetPilot()->GetChar()->GetSkillLevel(EvESkill::DroneInterfacing, true))));
        m_self->SetAttribute(AttrMiningAmount, newValue, update);
    } else {
        newValue = m_self->GetAttribute(AttrDamageMultiplier).get_float();
        newValue *= (1 + (0.2f * (m_pShipSE->GetPilot()->GetChar()->GetSkillLevel(EvESkill::DroneInterfacing, true))));
        m_self->SetAttribute(AttrDamageMultiplier, newValue, update);
    }
    //Drone Durability    5% bonus to drone shield, armor and hull hit points per level.
    newValue = m_self->GetAttribute(AttrHP).get_float();
    newValue *= (1 + (0.05f * (m_pShipSE->GetPilot()->GetChar()->GetSkillLevel(EvESkill::DroneDurability, true))));
    m_self->SetAttribute(AttrHP, newValue, update);
    newValue = m_self->GetAttribute(AttrShieldCapacity).get_float();
    newValue *= (1 + (0.05f * (m_pShipSE->GetPilot()->GetChar()->GetSkillLevel(EvESkill::DroneDurability, true))));
    m_self->SetAttribute(AttrShieldCapacity, newValue, update);
    // set current shield amount to new capacity value also
    m_self->SetAttribute(AttrShieldCharge, newValue, update);
    newValue = m_self->GetAttribute(AttrArmorHP).get_float();
    newValue *= (1 + (0.05f * (m_pShipSE->GetPilot()->GetChar()->GetSkillLevel(EvESkill::DroneDurability, true))));
    m_self->SetAttribute(AttrArmorHP, newValue, update);

    // now, check types & groups for specific skills...
    // Repair Drone Operation  5% increased repair amount per level.
    if (m_self->groupID() == EVEDB::invGroups::Logistic_Drone) {
        if (m_self->HasAttribute(AttrEntityArmorRepairDuration)) {
            newValue = m_self->GetAttribute(AttrEntityArmorRepairDuration).get_float();
            newValue *= (1 + (0.05f * (m_pShipSE->GetPilot()->GetChar()->GetSkillLevel(EvESkill::RepairDroneOperation, true))));
            m_self->SetAttribute(AttrEntityArmorRepairDuration, newValue, update);
        } else if (m_self->HasAttribute(AttrEntityShieldBoostDuration)) {
            newValue = m_self->GetAttribute(AttrEntityShieldBoostDuration).get_float();
            newValue *= (1 + (0.05f * (m_pShipSE->GetPilot()->GetChar()->GetSkillLevel(EvESkill::RepairDroneOperation, true))));
            m_self->SetAttribute(AttrEntityShieldBoostDuration, newValue, update);
        }
    }

    if (m_self->groupID() == EVEDB::invGroups::Combat_Drone) {
        if (m_self->type().volume() < 20) {
            // 24241   Combat Drone Operation   5% Bonus to drone damage of light and medium drones per level.
            newValue = m_self->GetAttribute(AttrDamageMultiplier).get_float();
            newValue *= (1 + (0.05f * (m_pShipSE->GetPilot()->GetChar()->GetSkillLevel(EvESkill::CombatDroneOperation, true))));
            m_self->SetAttribute(AttrDamageMultiplier, newValue, update);
        } else if (m_self->type().volume() > 20) {
            // 3441    Heavy Drone Operation  5% Bonus to heavy drone damage per level.
            newValue = m_self->GetAttribute(AttrDamageMultiplier).get_float();
            newValue *= (1 + (0.05f * (m_pShipSE->GetPilot()->GetChar()->GetSkillLevel(EvESkill::HeavyDroneOperation, true))));
            m_self->SetAttribute(AttrDamageMultiplier, newValue, update);
        }
    }

    if (m_self->groupID() == EVEDB::invGroups::Fighter_Bomber) {
        // Fighter Bombers  20% increase in fighter bomber damage per level.
        newValue = m_self->GetAttribute(AttrDamageMultiplier).get_float();
        newValue *= (1 + (0.2f * (m_pShipSE->GetPilot()->GetChar()->GetSkillLevel(EvESkill::FighterBombers, true))));
        m_self->SetAttribute(AttrDamageMultiplier, newValue, update);
    }
    if (m_self->groupID() == EVEDB::invGroups::Fighter_Drone) {
        // Fighters   20% increase in fighter damage per level.
        newValue = m_self->GetAttribute(AttrDamageMultiplier).get_float();
        newValue *= (1 + (0.2f * (m_pShipSE->GetPilot()->GetChar()->GetSkillLevel(EvESkill::Fighters, true))));
        m_self->SetAttribute(AttrDamageMultiplier, newValue, update);
    }

    /*  not sure what these are yet...
    if (m_self->groupID() == EVEDB::invGroups::Sentry_Drone) {
        // Sentry Drone Interfacing   5% bonus to Sentry Drone damage per level. ?
        newValue = m_self->GetAttribute(AttrDamageMultiplier).get_float();
        newValue *= (1 + (0.2f * (m_pShipSE->GetPilot()->GetChar()->GetSkillLevel(EvESkill::SentryDroneInterfacing, true))));
        m_self->SetAttribute(AttrDamageMultiplier, newValue, update);
    } */


    // last, do specializations...not sure how im gonna do these without checking for every item for each type
    /* 12484   Amarr Drone Specialization         advanced Amarr drones. 2% bonus to advanced Amarr drone damage per level.
     * 12485   Minmatar Drone Specialization   advanced Minmatar drones. 2% bonus to advanced Minmatar drone damage per level.
     * 12486   Gallente Drone Specialization   advanced Gallente drones. 2% bonus to advanced Gallente drone damage per level.
     * 12487   Caldari Drone Specialization     advanced Caldari drones. 2% bonus to advanced Caldari drone damage per level.
     */

    // hit AI::Init() again to update drone data
    m_AI->Init();
}

void DroneSE::SendBallData() {
    // testing
    //return;
    //  testing....send ball data packets
    std::vector<PyTuple*> updates;
    SetBallAgility sbagility;
        sbagility.entityID = GetID();
        sbagility.agility = m_self->GetAttribute(AttrInertiaMod).get_double();
    updates.push_back(sbagility.Encode());
    SetBallMass sbmass;
        sbmass.entityID = GetID();
        sbmass.mass = m_self->mass();
    updates.push_back(sbmass.Encode());
    SetBallSpeed sbms;
        sbms.entityID = GetID();
        sbms.speed = m_self->GetAttribute(AttrMaxVelocity).get_double();
    updates.push_back(sbms.Encode());
    if (m_bubble != nullptr)
        m_bubble->BubblecastDestinyUpdate(updates, "Drone Ball Data");
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

void DroneSE::ShipWarping(ShipSE* pShipSE) {
    Abandon();
}
