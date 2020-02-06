
 /**
  * @name EntityService.cpp
  *   Drone Control class
  * @Author:    Allan
  * @date:      06 November 2016
  */


#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"

#include "EVEServerConfig.h"
#include "npc/EntityService.h"
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

/*  drone states...
namespace DroneAI {
    namespace State {
        enum {
            Invalid           = -1,
            // defined in client
            Idle              = 0,  // not doing anything....idle.
            Combat            = 1,  // fighting - needs targetID
            Mining            = 2,  // unsure - needs targetID
            Approaching       = 3,  // too close to chase, but to far to engage
            Departing         = 4,  // return to ship
            Departing2        = 5,  // leaving.  different from Departing
            Pursuit           = 6,  // target out of range to attack/follow, but within npc sight range....use mwd/ab if equiped
            Fleeing           = 7,  // running away
            Operating         = 9,  // whats diff from engaged here?
            Engaged           = 10, // non-combat? - needs targetID
            // internal only
            Unknown           = 8,  // as stated
            Guarding          = 11,
            Assisting         = 12,
            Incapacicated     = 13  //
        };
    }
}
*/

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
    if (!IsSolarSystem(systemID)) {
        codelog(SERVICE__ERROR, "%s: Expected systemID, but got %u.", pClient->GetName(), systemID);
        return nullptr;
    }

    return new EntityBound(m_manager, pClient->SystemMgr(), systemID);
}

EntityBound::EntityBound(PyServiceMgr *mgr, SystemManager* systemMgr, uint32 systemID)
: PyBoundObject(mgr),
m_sysMgr(systemMgr),
m_systemID(systemID),
m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    m_strBoundObjectName = "EntityBound";

    PyCallable_REG_CALL(EntityBound, CmdEngage);
    PyCallable_REG_CALL(EntityBound, CmdRelinquishControl);
    PyCallable_REG_CALL(EntityBound, CmdDelegateControl);
    PyCallable_REG_CALL(EntityBound, CmdAssist);
    PyCallable_REG_CALL(EntityBound, CmdGuard);
    PyCallable_REG_CALL(EntityBound, CmdMine);
    PyCallable_REG_CALL(EntityBound, CmdMineRepeatedly);
    PyCallable_REG_CALL(EntityBound, CmdUnanchor);
    PyCallable_REG_CALL(EntityBound, CmdReturnHome);
    PyCallable_REG_CALL(EntityBound, CmdReturnBay);
    PyCallable_REG_CALL(EntityBound, CmdAbandonDrone);
    PyCallable_REG_CALL(EntityBound, CmdReconnectToDrones);
}

PyResult EntityBound::Handle_CmdEngage(PyCallArgs &call) {
 // ret = entity.CmdEngage(droneIDs, targetID)
    /*
        [PySubStream 104 bytes]
          [PyTuple 4 items]
            [PyInt 1]
            [PyString "MachoBindObject"]
            [PyTuple 2 items]
              [PyInt 30000302]
              [PyTuple 3 items]
                [PyString "CmdEngage"]
                [PyTuple 2 items]
                  [PyList 5 items]
                    [PyIntegerVar 1005909162494]
                    [PyIntegerVar 1005902743336]
                    [PyIntegerVar 1005909162497]
                    [PyIntegerVar 1005909162499]
                    [PyIntegerVar 1005909162492]
                  [PyIntegerVar 9000000000001190095]
                [PyDict 0 kvp]
                */
    /*
      [PySubStream 258 bytes]
        [PyTuple 2 items]
          [PySubStruct]
            [PySubStream 31 bytes]
              [PyTuple 2 items]
                [PyString "N=790408:2886"]
                [PyIntegerVar 129756560501538126]
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
    /*
      [PySubStream 42 bytes]
        [PyTuple 2 items]
          [PySubStruct]
            [PySubStream 31 bytes]
              [PyTuple 2 items]
                [PyString "N=790408:2886"]
                [PyIntegerVar 129756560847182701]
          [PyDict 0 kvp]
          */

    _log(DRONE__TRACE, "EntityBound::Handle_CmdEngage()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return new PyDict();
}

PyResult EntityBound::Handle_CmdRelinquishControl(PyCallArgs &call) {
 // ret = entity.CmdRelinquishControl(IDs)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdRelinquishControl()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return new PyDict();
}

PyResult EntityBound::Handle_CmdDelegateControl(PyCallArgs &call) {
 // ret = entity.CmdDelegateControl(droneIDs, controllerID)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdDelegateControl()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return new PyDict();
}

PyResult EntityBound::Handle_CmdAssist(PyCallArgs &call) {
 // ret = entity.CmdAssist(assistID, droneIDs)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdAssist()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return new PyDict();
}

PyResult EntityBound::Handle_CmdGuard(PyCallArgs &call) {
 // ret = entity.CmdGuard(guardID, droneIDs)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdGuard()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return new PyDict();
}

PyResult EntityBound::Handle_CmdMine(PyCallArgs &call) {
 // ret = entity.CmdMine(droneIDs, targetID)
    /*
02:20:04 [DroneTrace] EntityBound::Handle_CmdMine()
02:20:04 [DroneDump]   Call Arguments:
02:20:04 [DroneDump]      Tuple: 2 elements
02:20:04 [DroneDump]       [ 0]   List: 1 elements
02:20:04 [DroneDump]       [ 0]   [ 0]    Integer: 140001526
02:20:04 [DroneDump]       [ 1]    Integer: 140000029
*/
    _log(DRONE__TRACE, "EntityBound::Handle_CmdMine()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return new PyDict();
}

PyResult EntityBound::Handle_CmdMineRepeatedly(PyCallArgs &call) {
 // ret = entity.CmdMineRepeatedly(droneIDs, targetID)
    /*
02:20:29 [DroneTrace] EntityBound::Handle_CmdMineRepeatedly()
02:20:29 [DroneDump]   Call Arguments:
02:20:29 [DroneDump]      Tuple: 2 elements
02:20:29 [DroneDump]       [ 0]   List: 1 elements
02:20:29 [DroneDump]       [ 0]   [ 0]    Integer: 140001526
02:20:29 [DroneDump]       [ 1]    Integer: 140000029
*/
    _log(DRONE__TRACE, "EntityBound::Handle_CmdMineRepeatedly()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return new PyDict();
}

PyResult EntityBound::Handle_CmdUnanchor(PyCallArgs &call) {
 // ret = entity.CmdUnanchor(droneIDs, targetID)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdUnanchor()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return new PyDict();
}

PyResult EntityBound::Handle_CmdReturnHome(PyCallArgs &call) {
 // ret = entity.CmdReturnHome(droneIDs)
    // this is return and orbit command
    /*
02:18:26 [DroneTrace] EntityBound::Handle_CmdReturnHome()
02:18:26 [DroneDump]   Call Arguments:
02:18:26 [DroneDump]      Tuple: 1 elements
02:18:26 [DroneDump]       [ 0]   List: 1 elements
02:18:26 [DroneDump]       [ 0]   [ 0]    Integer: 140001219
*/
    call.Dump(DRONE__DUMP);

    //Drone* pDrone = m_sysMgr->GetSE()->GetDroneSE();
    //pDrone->DestinyMgr()->Orbit(pShipSE, 800);


    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return new PyDict();
}

PyResult EntityBound::Handle_CmdReturnBay(PyCallArgs &call) {
 // ret = entity.CmdReturnBay(droneIDs)
    /*
        [PySubStream 97 bytes]
          [PyTuple 4 items]
            [PyInt 1]
            [PyString "MachoBindObject"]
            [PyTuple 2 items]
              [PyInt 30000302]
              [PyTuple 3 items]
                [PyString "CmdReturnBay"]
                [PyTuple 1 items]
                  [PyList 5 items]
                    [PyIntegerVar 1005909162494]
                    [PyIntegerVar 1005902743336]
                    [PyIntegerVar 1005909162497]
                    [PyIntegerVar 1005909162499]
                    [PyIntegerVar 1005909162492]
                [PyDict 0 kvp]

    [PyTuple 1 items]
      [PySubStream 42 bytes]
        [PyTuple 2 items]
          [PySubStruct]
            [PySubStream 31 bytes]
              [PyTuple 2 items]
                [PyString "N=790408:2886"]
                [PyIntegerVar 129756563162318175]
          [PyDict 0 kvp]
          */
    _log(DRONE__TRACE, "EntityBound::Handle_CmdReturnBay()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");

    // returns nodeID and timestamp and dict of ?
    /*
    PyDict* dict = new PyDict();
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
        tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    PySubStruct* str = new PySubStruct(new PySubStream(tuple));
    PyTuple* tuple1 = new PyTuple(2);
        tuple1->SetItem(0, str);
        tuple1->SetItem(1, dict);
    return tuple1;
    */
    return new PyDict();
}

PyResult EntityBound::Handle_CmdAbandonDrone(PyCallArgs &call) {
 // ret = entity.CmdAbandonDrone(droneIDs)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdAbandonDrone()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return new PyDict();
}

PyResult EntityBound::Handle_CmdReconnectToDrones(PyCallArgs &call) {
 // ret = entity.CmdReconnectToDrones(droneCandidates)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdReconnectToDrones()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return new PyDict();
}

/*{'FullPath': u'UI/Messages', 'messageID': 259652, 'label': u'EntityBrokenCommandBody'}(u'{targetTypeName} seems to be defective and does not respond to the command you are giving it.', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259653, 'label': u'EntityDistantCommandBody'}(u'{targetTypeName} is too far away and will not respond to the command you are giving it (must be within {distance} meters from it).', None, {u'{distance}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'distance'}, u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259668, 'label': u'EntityTargetTooDistantBody'}(u'The drones fail to execute your commands as the target {targetTypeName} is not within your {distance} m drone command range.', None, {u'{distance}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'distance'}, u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259694, 'label': u'EntityIncapacitatedCommandBody'}(u'{targetTypeName} is incapacitated due to damage or abandonment and will not respond to the command you are giving it (try scooping it).', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259695, 'label': u'EntityNotYoursToCommandBody'}(u'{targetTypeName} does not respond to your commands.', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259698, 'label': u'EntityTargetMustBeTargetedBody'}(u'{targetTypeName} requires the target be locked onto by you, which it is not.', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259699, 'label': u'EntityTargetAlreadyHasControlBody'}(u'Control of the {item} cannot be delegated to {whom} because they already have control of it.', None, {u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{whom}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'whom'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259702, 'label': u'EntityNoTargetDroneManagementAbilitiesBody'}(u'Control of the {item} cannot be delegated to {whom} because they do not have the skill to control any drones.', None, {u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{whom}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'whom'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259703, 'label': u'EntityNoTargetDroneManagementAbilitiesLeftBody'}(u'Control of the {item} cannot be delegated to {whom} because they only have the skill to control {[numeric]limit} drones and they are already controlling that many.', None, {u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{[numeric]limit}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'limit'}, u'{whom}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'whom'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259704, 'label': u'EntityTargetNotPresentBody'}(u'{targetTypeName} cannot be commanded to work on a target that is no longer present.', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259705, 'label': u'EntityUnknownCommandBody'}(u'{targetTypeName} does not recognize the command you are trying to give it.', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259706, 'label': u'EntityNotPresentBody'}(u'{targetTypeName} cannot be commanded as it is not actually present.', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259711, 'label': u'EntityInvalidTargetBody'}(u'{targetTypeName} can only perform that action on an item of group  {desiredTarget}.', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}, u'{desiredTarget}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'desiredTarget'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258393, 'label': u'EntityTargetWarpDisruptedBody'}(u'Control of the {[item]item.name} cannot be delegated to someone who the drones cannot warp to.', None, {u'{[item]item.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'item'}})
 * {'FullPath': u'UI/Messages', 'messageID': 258349, 'label': u'EntityTargetCharInCapsuleBody'}(u'The drone cannot be commanded with respect to {targetChar} because the pilot is in a capsule.', None, {u'{targetChar}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetChar'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259601, 'label': u'EntityTargetCharNotPresentBody'}(u'The drone cannot be commanded with respect to {[character]targetChar.name} because they are not present in this solar system.', None, {u'{[character]targetChar.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'targetChar'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259606, 'label': u'EntityHasSkillPrerequisitesBody'}(u'You do not have the required {[numeric]skillCount -> "skill", "skills"} to do that. To command that drone requires having learned the following {[numeric]skillCount -> "skill", "skills"}: {requiredSkills}.', None, {u'{[numeric]skillCount -> "skill", "skills"}': {'conditionalValues': [u'skill', u'skills'], 'variableType': 9, 'propertyName': None, 'args': 320, 'kwargs': {}, 'variableName': 'skillCount'}, u'{requiredSkills}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'requiredSkills'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259607, 'label': u'EntityTargetMustBeFleetMemberBody'}(u'Drones can only accept that command if {targetOwner} is a member of your fleet, which they are not.', None, {u'{targetOwner}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetOwner'}})
 */
