
 /**
  * @name EntityService.cpp
  *   Drone Control class
  * @Author:    Allan
  * @date:      06 November 2016
  * @rewrite:   04 February 2025 (begin implementation)
  */


#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"

#include "../EVEServerConfig.h"
#include "npc/EntityService.h"
#include "npc/Drone.h"
#include "pos/Tower.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"


PyCallable_Make_InnerDispatcher(EntityService)

EntityService::EntityService(PyServiceMgr *mgr)
: PyService(mgr, "entity"),
m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);
}

EntityService::~EntityService() {
    delete m_dispatch;
}

/*
DRONE__ERROR
DRONE__WARNING
DRONE__MESSAGE
DRONE__INFO
DRONE__TRACE
DRONE__DUMP
DRONE__AI_TRACE
*/

/** @todo  will need to make sure this object is deleted when changing systems  */
PyBoundObject *EntityService::CreateBoundObject(Client* pClient, const PyRep* bind_args) {
    _log(DRONE__DUMP, "EntityService bind request");
    bind_args->Dump(DRONE__DUMP, "    ");
    if (!bind_args->IsInt()) {
        codelog(SERVICE__ERROR, "%s: Non-integer bind argument '%s'", pClient->GetName(), bind_args->TypeString());
        return nullptr;
    }

    uint32 systemID = bind_args->AsInt()->value();
    if (!sDataMgr.IsSolarSystem(systemID)) {
        codelog(SERVICE__ERROR, "%s: Expected systemID, but got %u.", pClient->GetName(), systemID);
        return nullptr;
    }

    return new EntityBound(m_manager, pClient);
}

EntityBound::EntityBound(PyServiceMgr *mgr, Client* pClient)
: PyBoundObject(mgr),
m_sysMgr(pClient->SystemMgr()),
m_pClient(pClient),
m_shipSE(pClient->GetShipSE()),
m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    m_strBoundObjectName = "EntityBound";

    PyCallable_REG_CALL(EntityBound, CmdEngage);                // called from EngageTarget
    PyCallable_REG_CALL(EntityBound, CmdAssist);
    PyCallable_REG_CALL(EntityBound, CmdGuard);
    PyCallable_REG_CALL(EntityBound, CmdMine);
    PyCallable_REG_CALL(EntityBound, CmdUnanchor);
    PyCallable_REG_CALL(EntityBound, CmdMineRepeatedly);
    PyCallable_REG_CALL(EntityBound, CmdDelegateControl);
    PyCallable_REG_CALL(EntityBound, CmdRelinquishControl);
    PyCallable_REG_CALL(EntityBound, CmdReturnBay);
    PyCallable_REG_CALL(EntityBound, CmdReturnHome);            // called from ReturnAndOrbit
    PyCallable_REG_CALL(EntityBound, CmdAbandonDrone);
    PyCallable_REG_CALL(EntityBound, CmdReconnectToDrones);
}

PyResult EntityBound::Handle_CmdEngage(PyCallArgs &call) {
    int32 targID(PyRep::IntegerValueI32(call.tuple->AsTuple()->GetItem(1)));
    SystemEntity* pTarget = m_sysMgr->GetEntityByID(targID);
    PyList* droneList = call.tuple->AsTuple()->GetItem(0)->AsList();

    PyDict* errorDict = new PyDict();
    CheckTarget(pTarget, droneList, errorDict);
    if (!errorDict->empty())
        return errorDict;
    CheckTower(pTarget, droneList, errorDict);
    if (!errorDict->empty())
        return errorDict;

    SystemEntity* pSE(nullptr);
    PyList::const_iterator itr = droneList->begin();
    while (itr != droneList->end()) {
        pSE = m_sysMgr->GetEntityByID(PyRep::IntegerValueU32(*itr));
        if (pSE != nullptr)
            pSE->GetDroneSE()->Engage(pTarget, errorDict);
        ++itr;
    }

    // returns dict of error msg
    return errorDict;
}

PyResult EntityBound::Handle_CmdRelinquishControl(PyCallArgs &call) {
    // return control to owner

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return PyStatic.mtDict();

    if (0) {
    PyList* droneList = call.tuple->AsTuple()->GetItem(0)->AsList();
    PyDict* errorDict = new PyDict();

    SystemEntity* pSE(nullptr);
    PyList::const_iterator itr = droneList->begin();
    while (itr != droneList->end()) {
        pSE = m_sysMgr->GetEntityByID(PyRep::IntegerValueU32(*itr));
        if (pSE != nullptr)
            pSE->GetDroneSE()->ReturnHome(errorDict);
        ++itr;
    }

    // make note about drones being returned

    // returns dict of error msg
    return errorDict;
    }
}

// give control to target
PyResult EntityBound::Handle_CmdDelegateControl(PyCallArgs &call) {
    // this DOES assign control of drones to target ship
    int32 targID = PyRep::IntegerValueI32(call.tuple->AsTuple()->GetItem(1));
    SystemEntity* pTarget = m_sysMgr->GetEntityByID(targID);
    PyList* droneList = call.tuple->AsTuple()->GetItem(0)->AsList();

    PyDict* errorDict = new PyDict();
    CheckTarget(pTarget, droneList, errorDict);
    if (!errorDict->empty())
        return errorDict;
    CheckTower(pTarget, droneList, errorDict);
    if (!errorDict->empty())
        return errorDict;
    CheckMisc(pTarget, droneList, errorDict);
    if (!errorDict->empty())
        return errorDict;
    CheckSkills(pTarget, droneList, errorDict);
    if (!errorDict->empty())
        return errorDict;
    CheckFleet(pTarget, droneList, errorDict);
    if (!errorDict->empty())
        return errorDict;

    SystemEntity* pSE(nullptr);
    PyList::const_iterator itr = droneList->begin();
    while (itr != droneList->end()) {
        pSE = m_sysMgr->GetEntityByID(PyRep::IntegerValueU32(*itr));
        if (pSE != nullptr)
            pSE->GetDroneSE()->Delegate(pTarget, errorDict);
        ++itr;
    }

    // returns dict of error msg
    return errorDict;
}

// begin attack on given ship's active target
PyResult EntityBound::Handle_CmdAssist(PyCallArgs &call) {
    // this does NOT assign control of drones to target ship   only as target for engaging
    int32 targID = PyRep::IntegerValueI32(call.tuple->AsTuple()->GetItem(1));
    SystemEntity* pTarget = m_sysMgr->GetEntityByID(targID);
    PyList* droneList = call.tuple->AsTuple()->GetItem(0)->AsList();

    PyDict* errorDict = new PyDict();
    CheckTarget(pTarget, droneList, errorDict);
    if (!errorDict->empty())
        return errorDict;
    CheckTower(pTarget, droneList, errorDict);
    if (!errorDict->empty())
        return errorDict;
    CheckMisc(pTarget, droneList, errorDict);
    if (!errorDict->empty())
        return errorDict;
    CheckFleet(pTarget, droneList, errorDict);
    if (!errorDict->empty())
        return errorDict;

    SystemEntity* pSE(nullptr);
    PyList::const_iterator itr = droneList->begin();
    while (itr != droneList->end()) {
        pSE = m_sysMgr->GetEntityByID(PyRep::IntegerValueU32(*itr));
        if (pSE != nullptr)
            pSE->GetDroneSE()->Assist(pTarget, errorDict);
        ++itr;
    }

    // returns dict of error msg
    return errorDict;
}

// defend given ship
PyResult EntityBound::Handle_CmdGuard(PyCallArgs &call) {
    // this does NOT assign target ship to drone for control.  only as target for engaging
    int32 targID = PyRep::IntegerValueI32(call.tuple->AsTuple()->GetItem(1));
    SystemEntity* pTarget = m_sysMgr->GetEntityByID(targID);
    PyList* droneList = call.tuple->AsTuple()->GetItem(0)->AsList();

    PyDict* errorDict = new PyDict();
    CheckTarget(pTarget, droneList, errorDict);
    if (!errorDict->empty())
        return errorDict;
    CheckTower(pTarget, droneList, errorDict);
    if (!errorDict->empty())
        return errorDict;
    CheckMisc(pTarget, droneList, errorDict);
    if (!errorDict->empty())
        return errorDict;
    CheckFleet(pTarget, droneList, errorDict);
    if (!errorDict->empty())
        return errorDict;

    SystemEntity* pSE(nullptr);
    PyList::const_iterator itr = droneList->begin();
    while (itr != droneList->end()) {
        pSE = m_sysMgr->GetEntityByID(PyRep::IntegerValueU32(*itr));
        if (pSE != nullptr)
            pSE->GetDroneSE()->Guard(pTarget, errorDict);
        ++itr;
    }

    // returns dict of error msg
    return errorDict;
}

PyResult EntityBound::Handle_CmdMine(PyCallArgs &call) {
    int32 targID = PyRep::IntegerValueI32(call.tuple->AsTuple()->GetItem(1));
    SystemEntity* pTarget = m_sysMgr->GetEntityByID(targID);
    PyList* droneList = call.tuple->AsTuple()->GetItem(0)->AsList();

    PyDict* errorDict = new PyDict();
    CheckTarget(pTarget, droneList, errorDict);
    if (!errorDict->empty())
        return errorDict;

    if (!pTarget->IsAsteroidSE()) {
        sLog.Error("CmdMine()", "Target %s is not an asteroid.", pTarget->GetName());
        // no generic msgs, so roll my own here
        return errorDict;
    } else if (pTarget->GetGroupID() == EVEDB::invGroups::Ice) {
        sLog.Error("CmdMine()", "Target %s is Ice.", pTarget->GetName());
        // no generic msgs, so roll my own here
        return errorDict;
    }

    SystemEntity* pSE(nullptr);
    PyList::const_iterator itr = droneList->begin();
    while (itr != droneList->end()) {
        pSE = m_sysMgr->GetEntityByID(PyRep::IntegerValueU32(*itr));
        if (pSE != nullptr)
            pSE->GetDroneSE()->Mine(pTarget, errorDict);
        ++itr;
    }

    // returns dict of error msg
    return errorDict;
}

PyResult EntityBound::Handle_CmdMineRepeatedly(PyCallArgs &call) {
    int32 targID = PyRep::IntegerValueI32(call.tuple->AsTuple()->GetItem(1));
    SystemEntity* pTarget = m_sysMgr->GetEntityByID(targID);
    PyList* droneList = call.tuple->AsTuple()->GetItem(0)->AsList();

    PyDict* errorDict = new PyDict();
    CheckTarget(pTarget, droneList, errorDict);
    if (!errorDict->empty())
        return errorDict;

    if (!pTarget->IsAsteroidSE()) {
        sLog.Error("CmdMine()", "Target %s is not an asteroid.", pTarget->GetName());
        // no generic msgs, so roll my own here
        return errorDict;
    } else if (pTarget->GetGroupID() == EVEDB::invGroups::Ice) {
        sLog.Error("CmdMine()", "Target %s is Ice.", pTarget->GetName());
        // no generic msgs, so roll my own here
        return errorDict;
    }


    SystemEntity* pSE(nullptr);
    PyList::const_iterator itr = droneList->begin();
    while (itr != droneList->end()) {
        pSE = m_sysMgr->GetEntityByID(PyRep::IntegerValueU32(*itr));
        if (pSE != nullptr)
            pSE->GetDroneSE()->Mine(pTarget, errorDict, true);
        ++itr;
    }

    // returns dict of error msg
    return errorDict;
}

PyResult EntityBound::Handle_CmdReturnHome(PyCallArgs &call) {
    // this is return and orbit command
    PyDict* errorDict = new PyDict();
    PyList* droneList = call.tuple->AsTuple()->GetItem(0)->AsList();

    SystemEntity* pSE(nullptr);
    PyList::const_iterator itr = droneList->begin();
    while (itr != droneList->end()) {
        pSE = m_sysMgr->GetEntityByID(PyRep::IntegerValueU32(*itr));
        if (pSE != nullptr)
            pSE->GetDroneSE()->ReturnHome(errorDict);
        ++itr;
    }

    // returns dict of error msg
    return errorDict;
}

PyResult EntityBound::Handle_CmdReturnBay(PyCallArgs &call) {
    PyDict* errorDict = new PyDict();
    PyList* droneList = call.tuple->AsTuple()->GetItem(0)->AsList();

    SystemEntity* pSE(nullptr);
    PyList::const_iterator itr = droneList->begin();
    while (itr != droneList->end()) {
        pSE = m_sysMgr->GetEntityByID(PyRep::IntegerValueU32(*itr));
        if (pSE != nullptr)
            pSE->GetDroneSE()->ReturnBay(errorDict);
        ++itr;
    }

    // returns dict of error msg
    return errorDict;
}

PyResult EntityBound::Handle_CmdAbandonDrone(PyCallArgs &call) {
    PyList* droneList = call.tuple->AsTuple()->GetItem(0)->AsList();

    SystemEntity* pSE(nullptr);
    PyList::const_iterator itr = droneList->begin();
    while (itr != droneList->end()) {
        pSE = m_sysMgr->GetEntityByID(PyRep::IntegerValueU32(*itr));
        if (pSE != nullptr)
            pSE->GetDroneSE()->Abandon();
        ++itr;
    }

    call.client->SendNotifyMsg("You have abandoned %u drone%s.", (uint32) droneList->size(), droneList->size() > 1 ? "s" : "");

    // returns dict of error msg
    return PyStatic.mtDict();
}

PyResult EntityBound::Handle_CmdReconnectToDrones(PyCallArgs &call) {
    // MAX_DRONE_RECONNECTS = 25
    // is this only abandoned or offline also?  both
    PyDict* errorDict = new PyDict();
    PyList* droneList = call.tuple->AsTuple()->GetItem(0)->AsList();

    SystemEntity* pSE(nullptr);
    PyList::const_iterator itr = droneList->begin();
    while (itr != droneList->end()) {
        pSE = m_sysMgr->GetEntityByID(PyRep::IntegerValueU32(*itr));
        if (pSE != nullptr)
            pSE->GetDroneSE()->Reconnect(m_pClient->GetShipSE(), errorDict);
        ++itr;
    }

    // NOTE:  drone will online after reconnect if available bandwidth

    // returns dict of error msg
    return errorDict;
}

PyResult EntityBound::Handle_CmdUnanchor(PyCallArgs &call) {
    // fx for forced unanchoring   1129 structureUnanchorForced
    call.client->SendNotifyMsg("Unanchor is not implemented yet.");
    int32 targID = PyRep::IntegerValueI32(call.tuple->AsTuple()->GetItem(1));
    SystemEntity* pTarget = m_sysMgr->GetEntityByID(targID);
    PyList* droneList = call.tuple->AsTuple()->GetItem(0)->AsList();

    //CheckTarget();
    //CheckTower();
    //CheckMisc();

    // returns dict of error msg
    return PyStatic.mtDict();
}


// helper methods  (these are not checked in client)
// TODO: if errors, these should be returned for every drone in list as applicable
// will have to check each drone in list, then populate dicts to return to client for drones that fail
/*
          [PyDict 5 kvp]
            [PyIntegerVar 1005909162494]
            [PyTuple 2 items]
              [PyString "EntityTargetTooDistant"]
              [PyDict 2 kvp]
                [PyString "targetTypeName"]
                [PyTuple 2 items]
                  [PyInt 7]
                  [PyInt 561]
                [PyString "distance"]
                [PyFloat 45000]
    */

//TODO:  all these distances need radius data
void EntityBound::CheckTarget(SystemEntity* pTarget, PyList* droneList, PyDict* errorDict) {
    // check for valid target
    uint32 droneID(0);
    PyList::const_iterator itr = droneList->begin();
    for ( ; itr != droneList->end(); ++itr) {
        droneID = PyRep::IntegerValueU32(*itr);
        SystemEntity* pSE = m_sysMgr->GetEntityByID(droneID);
        InventoryItemRef iRef(sItemFactory.GetItemRefFromID(droneID));

        if (pTarget == nullptr) {
            PyDict* data = new PyDict();
            if (pSE != nullptr) {
                data->SetItemString("targetTypeName", new PyString(pSE->GetName()));
            } else if (iRef.get() != nullptr) {
                data->SetItemString("targetTypeName", new PyString(iRef->itemName()));
            } else  {
                data->SetItemString("targetTypeName", new PyString("NULL"));
            }
            PyTuple* error = new PyTuple(2);
            error->SetItem(0, new PyString("EntityTargetNotPresent"));
            error->SetItem(1, data);
            errorDict->SetItem(new PyInt(droneID), error);
            continue;
        }

        if (pTarget->HasPilot()) {
            // check for assigning drone to player not in system
            if (pTarget->SystemMgr()->GetID() != m_shipSE->SystemMgr()->GetID()) {
                PyDict* data = new PyDict();
                data->SetItemString("targetChar", new PyString(pTarget->GetPilot()->GetName()));
                PyTuple* error = new PyTuple(2);
                error->SetItem(0, new PyString("EntityTargetCharNotPresent"));
                error->SetItem(1, data);
                errorDict->SetItem(new PyInt(droneID), error);
                continue;
            }
        }

        if (pSE == nullptr) {
            PyDict* data = new PyDict();
            PyTuple* error = new PyTuple(2);
            if (iRef.get() != nullptr) {
                data->SetItemString("targetTypeName", new PyString(iRef->itemName()));
            } else  {
                data->SetItemString("targetTypeName", new PyString("NULL"));
            }
            error->SetItem(0, new PyString("EntityNotPresent"));
            error->SetItem(1, data);
            errorDict->SetItem(new PyInt(droneID), error);
            continue;
        }
        if (!m_shipSE->TargetMgr()->IsTargeting(pTarget)) {
            PyDict* data = new PyDict();
            data->SetItemString("targetTypeName", new PyString(pTarget->GetName()));
            PyTuple* error = new PyTuple(2);
            error->SetItem(0, new PyString("EntityTargetMustBeTargeted"));
            error->SetItem(1, data);
            errorDict->SetItem(new PyInt(droneID), error);
            continue;
        }

        // this will limit drone targets to m_controlDistance   set to distance from target    config option?
        if ((pSE->GetPosition().distance(pTarget->GetPosition()) - pTarget->GetRadius()) > pSE->GetDroneSE()->GetControlDistance()) {
        //if (pSE->GetPosition().distance(pSE->GetDroneSE()->GetAI()->GetAssignedShipSE()->GetPosition()) > pSE->GetDroneSE()->GetControlDistance()) {
            PyDict* data = new PyDict();
            data->SetItemString("targetTypeName", new PyString(pSE->GetName()));
            data->SetItemString("distance", new PyInt(pSE->GetDroneSE()->GetControlDistance()));
            PyTuple* error = new PyTuple(2);
            error->SetItem(0, new PyString("EntityTargetTooDistant"));
            error->SetItem(1, data);
            errorDict->SetItem(new PyInt(droneID), error);
            continue;
        }
    }
}

void EntityBound::CheckTower(SystemEntity* pTarget, PyList* droneList, PyDict* errorDict) {
    // various solSystem checks
    if (pTarget->SysBubble()->HasTower()) {
        TowerSE* ptSE = pTarget->SysBubble()->GetTowerSE();
        if (ptSE->HasForceField()) {
            if ((ptSE->GetPosition().distance(m_shipSE->GetPosition()) - m_shipSE->GetRadius()) < ptSE->GetSOI()) {
                // controller in field
                throw UserError("EntityDelegateInsideField");
            }
            if ((pTarget->GetPosition().distance(ptSE->GetPosition()) - ptSE->GetRadius()) < ptSE->GetSOI()) {
                // target in field
                throw UserError("DeniedDroneTargetForceField")
                .AddFormatValue("target", new PyInt(pTarget->GetID()));
            }
        }
    }

/*  not sure what this is for yet
 * 259636, 'label': u'EntityActionSecurityLevelRestrictionsBody'}(u'You cannot do that. That drone command requires you to be in a solar system with a security level less than {[numeric]needed, decimalPlaces=1}.'
 */
}

void EntityBound::CheckMisc(SystemEntity* pTarget, PyList* droneList, PyDict* errorDict) {
    // verify target is not in capsule
    if (pTarget->HasPilot()) {
        // for delegating command
        if (pTarget->GetShipSE()->GetTypeID() == EVEDB::invTypes::Capsule) {
            throw UserError("EntityTargetCharInCapsule")
            .AddFormatValue("targetChar", new PyString(pTarget->GetPilot()->GetName()));
        }
    }

    /* loop thru drone list to do a few checks...
     *    char does not currently have control
     *    target distance is within control range
     */
    InventoryItemRef iRef(sItemFactory.GetItemRefFromID(PyRep::IntegerValueU32(*(droneList->begin()))));
    if (iRef.get() == nullptr) {
        throw UserError("EntityNotPresent")
        .AddFormatValue("targetTypeName", new PyString("NULL"));
    }
    SystemEntity* pSE1(m_sysMgr->GetEntityByID(PyRep::IntegerValueU32(*(droneList->begin()))));
    if (pSE1 == nullptr) {
        throw UserError("EntityNotPresent")
        .AddFormatValue("targetTypeName", new PyString(iRef->itemName()));
    } else if (pSE1->GetDroneSE()->GetControllerID() == pTarget->GetID()) {
        throw UserError("EntityTargetAlreadyHasControl")
        .AddFormatValue("item", new PyString(iRef->itemName()))
        .AddFormatValue("whom", new PyString(pTarget->GetName()));
    }

    if (droneList->size() < 2)
        return;

    SystemEntity* pSE2(nullptr);
    PyList::const_iterator itr = droneList->begin();
    while (itr != droneList->end()) {
        pSE2 = m_sysMgr->GetEntityByID(PyRep::IntegerValueU32(*itr));
        if (pSE2 == nullptr) {
            iRef = sItemFactory.GetItemRefFromID(PyRep::IntegerValueU32(*(itr)));
            if (iRef.get() == nullptr) {
                throw UserError("EntityNotPresent")
                .AddFormatValue("targetTypeName", new PyString("NULL"));
            }
            throw UserError("EntityNotPresent")
            .AddFormatValue("targetTypeName", new PyString(iRef->itemName()));
        }

        ++itr;
    }
}

void EntityBound::CheckSkills(SystemEntity* pTarget, PyList* droneList, PyDict* errorDict) {
    if (!sConfig.drone.StrictSkills)
        return;
    /*
     * skill
     * 259702, 'label': u'EntityNoTargetDroneManagementAbilitiesBody'}(u'Control of the {item} cannot be delegated to {whom} because they do not have the skill to control any drones.', None, {u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{whom}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'whom'}})
     * 259703, 'label': u'EntityNoTargetDroneManagementAbilitiesLeftBody'}(u'Control of the {item} cannot be delegated to {whom} because they only have the skill to control {[numeric]limit} drones and they are already controlling that many.', None, {u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{[numeric]limit}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'limit'}, u'{whom}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'whom'}})
     */
}

void EntityBound::CheckFleet(SystemEntity* pTarget, PyList* droneList, PyDict* errorDict) {
    if (!sConfig.drone.FleetOnly)
        return;
/*
 *
 * fleet members
 * 259607, 'label': u'EntityTargetMustBeFleetMemberBody'}(u'Drones can only accept that command if {targetOwner} is a member of your fleet, which they are not.', None, {u'{targetOwner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetOwner'}})
 *
 */
}


//IDEA:  create drone channel or private message "i am sending my location".
// binary:  01101001 00100000 01100001 01101101 00100000 01110011 01100101 01101110 01100100 01101001 01101110 01100111 00100000 01101101 01111001 00100000 01101100 01101111 01100011 01100001 01110100 01101001 01101111 01101110
//   lost drones will ping solmap using bookmark
//   distance 15-20km from actual, modded by skills

//std::string text = "sending location"; //01110011 01100101 01101110 01100100 01101001 01101110 01100111 00100000 01101100 01101111 01100011 01100001 01110100 01101001 01101111 01101110
// convert string to binary
//BinString(text);
