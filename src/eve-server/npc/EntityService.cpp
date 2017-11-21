
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

    PyCallable_REG_CALL(EntityService, CmdEngage);
    PyCallable_REG_CALL(EntityService, CmdRelinquishControl);
    PyCallable_REG_CALL(EntityService, CmdDelegateControl);
    PyCallable_REG_CALL(EntityService, CmdAssist);
    PyCallable_REG_CALL(EntityService, CmdGuard);
    PyCallable_REG_CALL(EntityService, CmdMine);
    PyCallable_REG_CALL(EntityService, CmdMineRepeatedly);
    PyCallable_REG_CALL(EntityService, CmdUnanchor);
    PyCallable_REG_CALL(EntityService, CmdReturnHome);
    PyCallable_REG_CALL(EntityService, CmdReturnBay);
    PyCallable_REG_CALL(EntityService, CmdAbandonDrone);
    PyCallable_REG_CALL(EntityService, CmdReconnectToDrones);
}

EntityService::~EntityService() {
    delete m_dispatch;
}

/** @todo  will need to make sure this object is deleted when changing systems  */
PyBoundObject *EntityService::_CreateBoundObject(Client* pClient, const PyRep* bind_args) {
    _log(NPC__INFO, "EntityService bind request");
    bind_args->Dump(NPC__INFO, "    ");
    return new EntityBound(m_manager, pClient->SystemMgr(), bind_args->AsTuple()->GetItem(0)->AsInt()->value(), bind_args->AsTuple()->GetItem(1)->AsInt()->value());
}

EntityBound::EntityBound(PyServiceMgr *mgr, SystemManager* systemMgr, uint32 systemID, uint32 unknown)
: PyBoundObject(mgr),
m_sysMgr(systemMgr),
m_systemID(systemID),
m_unknown(unknown),
m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    m_strBoundObjectName = "EntityBound";


}

PyResult EntityService::Handle_CmdEngage(PyCallArgs &call) {
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

    sLog.White("EntityService::Handle_CmdEngage()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return nullptr;
}

PyResult EntityService::Handle_CmdRelinquishControl(PyCallArgs &call) {
 // ret = entity.CmdRelinquishControl(IDs)
    sLog.White("EntityService::Handle_CmdRelinquishControl()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return nullptr;
}

PyResult EntityService::Handle_CmdDelegateControl(PyCallArgs &call) {
 // ret = entity.CmdDelegateControl(droneIDs, controllerID)
    sLog.White("EntityService::Handle_CmdDelegateControl()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return nullptr;
}

PyResult EntityService::Handle_CmdAssist(PyCallArgs &call) {
 // ret = entity.CmdAssist(assistID, droneIDs)
    sLog.White("EntityService::Handle_CmdAssist()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return nullptr;
}

PyResult EntityService::Handle_CmdGuard(PyCallArgs &call) {
 // ret = entity.CmdGuard(guardID, droneIDs)
    sLog.White("EntityService::Handle_CmdGuard()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return nullptr;
}

PyResult EntityService::Handle_CmdMine(PyCallArgs &call) {
 // ret = entity.CmdMine(droneIDs, targetID)
    sLog.White("EntityService::Handle_CmdMine()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return nullptr;
}

PyResult EntityService::Handle_CmdMineRepeatedly(PyCallArgs &call) {
 // ret = entity.CmdMineRepeatedly(droneIDs, targetID)
    sLog.White("EntityService::Handle_CmdMineRepeatedly()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return nullptr;
}

PyResult EntityService::Handle_CmdUnanchor(PyCallArgs &call) {
 // ret = entity.CmdUnanchor(droneIDs, targetID)
    sLog.White("EntityService::Handle_CmdUnanchor()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return nullptr;
}

PyResult EntityService::Handle_CmdReturnHome(PyCallArgs &call) {
 // ret = entity.CmdReturnHome(droneIDs)
    sLog.White("EntityService::Handle_CmdReturnHome()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return nullptr;
}

PyResult EntityService::Handle_CmdReturnBay(PyCallArgs &call) {
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
    sLog.White("EntityService::Handle_CmdReturnBay()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return nullptr;
}

PyResult EntityService::Handle_CmdAbandonDrone(PyCallArgs &call) {
 // ret = entity.CmdAbandonDrone(droneIDs)
    sLog.White("EntityService::Handle_CmdAbandonDrone()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return nullptr;
}

PyResult EntityService::Handle_CmdReconnectToDrones(PyCallArgs &call) {
 // ret = entity.CmdReconnectToDrones(droneCandidates)
    sLog.White("EntityService::Handle_CmdReconnectToDrones()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return nullptr;
}



