
 /**
  * @name EntityService.cpp
  *   Drone Control class
  * @Author:    Allan
  * @date:      06 November 2016
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
     * 02:26:02 [DroneTrace] EntityBound::Handle_CmdEngage()
     * 02:26:02 [DroneDump]   Call Arguments:
     * 02:26:02 [DroneDump]      Tuple: 2 elements
     * 02:26:02 [DroneDump]       [ 0]   List: 5 elements
     * 02:26:02 [DroneDump]       [ 0]   [ 0]    Integer: 140024775
     * 02:26:02 [DroneDump]       [ 0]   [ 1]    Integer: 140024776
     * 02:26:02 [DroneDump]       [ 0]   [ 2]    Integer: 140024777
     * 02:26:02 [DroneDump]       [ 0]   [ 3]    Integer: 140024778
     * 02:26:02 [DroneDump]       [ 0]   [ 4]    Integer: 140024779
     * 02:26:02 [DroneDump]       [ 1]    Integer: 450002489
     * 02:26:02 [DroneDump]       List: 5 elements
     * 02:26:02 [DroneDump]       [ 0]    Integer: 140024775
     * 02:26:02 [DroneDump]       [ 1]    Integer: 140024776
     * 02:26:02 [DroneDump]       [ 2]    Integer: 140024777
     * 02:26:02 [DroneDump]       [ 3]    Integer: 140024778
     * 02:26:02 [DroneDump]       [ 4]    Integer: 140024779
     * 02:26:02 C DroneEngage: targetID 450002489
     *
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

    int32 targID(PyRep::IntegerValueI32(call.tuple->AsTuple()->GetItem(1)));

    PyList* droneList = call.tuple->AsTuple()->GetItem(0)->AsList();

    SystemEntity* pTarget = m_sysMgr->GetEntityByID(targID);
    if (pTarget == nullptr) {
        // get name of first drone in list and return it...close enough
        InventoryItemRef iRef(sItemFactory.GetItemRefFromID(PyRep::IntegerValueU32(*(droneList->begin()))));
        // this may not be right...
        throw UserError("EntityTargetNotPresent")
            .AddFormatValue("targetTypeName", new PyString(iRef->itemName()));

    }

    if (pTarget->SysBubble()->HasTower()) {
        TowerSE* ptSE = pTarget->SysBubble()->GetTowerSE();
        if (ptSE->HasForceField())
            if (pTarget->GetPosition().distance(ptSE->GetPosition()) < ptSE->GetSOI()) {
                throw UserError("DeniedDroneTargetForceField")
                        .AddFormatValue("target", new PyInt(targID));
            }
    }

    SystemEntity* pSE(nullptr);
    PyList::const_iterator itr = droneList->begin();
    while (itr != droneList->end()) {
        pSE = m_sysMgr->GetEntityByID(PyRep::IntegerValueU32(*itr));
        if (pSE != nullptr)
            pSE->GetDroneSE()->Engage(pTarget);
        ++itr;
    }

    return PyStatic.mtDict();
}

PyResult EntityBound::Handle_CmdRelinquishControl(PyCallArgs &call) {
 // ret = entity.CmdRelinquishControl(IDs)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdRelinquishControl()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    // return control to owner

    return PyStatic.mtDict();
}

PyResult EntityBound::Handle_CmdDelegateControl(PyCallArgs &call) {
 // ret = entity.CmdDelegateControl(droneIDs, controllerID)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdDelegateControl()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    // give control to controllerID

    return PyStatic.mtDict();
}

PyResult EntityBound::Handle_CmdAssist(PyCallArgs &call) {
 // ret = entity.CmdAssist(assistID, droneIDs)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdAssist()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("not implemented yet.");
    // begin attack on given ship's active target

    return PyStatic.mtDict();
}

PyResult EntityBound::Handle_CmdGuard(PyCallArgs &call) {
 // ret = entity.CmdGuard(guardID, droneIDs)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdGuard()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("not implemented yet.");
    // defend given ship

    return PyStatic.mtDict();
}

PyResult EntityBound::Handle_CmdMine(PyCallArgs &call) {
 // ret = entity.CmdMine(droneIDs, targetID)
    /*
     * 02:18:30 [DroneTrace] EntityBound::Handle_CmdMine()
     * 02:18:30 [DroneDump]   Call Arguments:
     * 02:18:30 [DroneDump]      Tuple: 2 elements
     * 02:18:30 [DroneDump]       [ 0]   List: 5 elements
     * 02:18:30 [DroneDump]       [ 0]   [ 0]    Integer: 140024770
     * 02:18:30 [DroneDump]       [ 0]   [ 1]    Integer: 140024771
     * 02:18:30 [DroneDump]       [ 0]   [ 2]    Integer: 140024772
     * 02:18:30 [DroneDump]       [ 0]   [ 3]    Integer: 140024773
     * 02:18:30 [DroneDump]       [ 0]   [ 4]    Integer: 140024774
     * 02:18:30 [DroneDump]       [ 1]    Integer: 450002489
     *
     * 02:18:30 [DroneDump]       List: 5 elements
     * 02:18:30 [DroneDump]       [ 0]    Integer: 140024770
     * 02:18:30 [DroneDump]       [ 1]    Integer: 140024771
     * 02:18:30 [DroneDump]       [ 2]    Integer: 140024772
     * 02:18:30 [DroneDump]       [ 3]    Integer: 140024773
     * 02:18:30 [DroneDump]       [ 4]    Integer: 140024774
     * 02:18:30 C DroneMine: targetID 450002489
     *
     */

    PyList* droneList = call.tuple->AsTuple()->GetItem(0)->AsList();

    int32 targID = PyRep::IntegerValueI32(call.tuple->AsTuple()->GetItem(1));
    SystemEntity* pTarget = m_sysMgr->GetEntityByID(targID);
    if (pTarget == nullptr) {
        // get name of first drone in list and return it...close enough
        InventoryItemRef iRef(sItemFactory.GetItemRefFromID(PyRep::IntegerValueU32(*(droneList->begin()))));
        // this may not be right...
        throw UserError("EntityTargetNotPresent")
                .AddFormatValue("targetTypeName", new PyString(iRef->itemName()));
    }

    //TODO:  checks here ... verify asteroid here et.al.
    // NOTE:  no drone can mine ice in crucible release
    
    SystemEntity* pSE(nullptr);
    PyList::const_iterator itr = droneList->begin();
    while (itr != droneList->end()) {
        pSE = m_sysMgr->GetEntityByID(PyRep::IntegerValueU32(*itr));
        if (pSE != nullptr)
            pSE->GetDroneSE()->BeginMining(pTarget);
        ++itr;
    }

    return PyStatic.mtDict();
}

PyResult EntityBound::Handle_CmdMineRepeatedly(PyCallArgs &call) {
 // ret = entity.CmdMineRepeatedly(droneIDs, targetID)
    /*)
     * 16:20:28 [DroneTrace] EntityBound::Handle_CmdMineRepeatedly()
     * 16:20:28 [DroneDump]   Call Arguments:
     * 16:20:28 [DroneDump]      Tuple: 2 elements
     * 16:20:28 [DroneDump]       [ 0]   List: 5 elements
     * 16:20:28 [DroneDump]       [ 0]   [ 0]    Integer: 140024264
     * 16:20:28 [DroneDump]       [ 0]   [ 1]    Integer: 140024265
     * 16:20:28 [DroneDump]       [ 0]   [ 2]    Integer: 140024261
     * 16:20:28 [DroneDump]       [ 0]   [ 3]    Integer: 140024262
     * 16:20:28 [DroneDump]       [ 0]   [ 4]    Integer: 140024263
     * 16:20:28 [DroneDump]       [ 1]    Integer: 450000587
     */
    _log(DRONE__TRACE, "EntityBound::Handle_CmdMineRepeatedly()");
    call.Dump(DRONE__DUMP);

    /** @todo MAKE CHECKS IN MINING LASER FOR DRONES BEFORE COMPLETING THIS FUNCTION  **/

    PyList* droneList = call.tuple->AsTuple()->GetItem(0)->AsList();
    droneList->Dump(DRONE__DUMP, "    ");

    int32 targID = PyRep::IntegerValueI32(call.tuple->AsTuple()->GetItem(1));
    sLog.Cyan("DroneMineRepeat", "targetID %i", targID);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return PyStatic.mtDict();
}

PyResult EntityBound::Handle_CmdUnanchor(PyCallArgs &call) {
 // ret = entity.CmdUnanchor(droneIDs, targetID)
    _log(DRONE__TRACE, "EntityBound::Handle_CmdUnanchor()");
    call.Dump(DRONE__DUMP);

    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return PyStatic.mtDict();
}

PyResult EntityBound::Handle_CmdReturnHome(PyCallArgs &call) {
 // ret = entity.CmdReturnHome(droneIDs)
    // this is return and orbit command
    /*
     * 02:20:05 [Bound] EntityBound::CmdReturnHome()
     * 02:20:05 [DroneDump]   Call Arguments:
     * 02:20:05 [DroneDump]      Tuple: 1 elements
     * 02:20:05 [DroneDump]       [ 0]   List: 5 elements
     * 02:20:05 [DroneDump]       [ 0]   [ 0]    Integer: 140024770
     * 02:20:05 [DroneDump]       [ 0]   [ 1]    Integer: 140024771
     * 02:20:05 [DroneDump]       [ 0]   [ 2]    Integer: 140024772
     * 02:20:05 [DroneDump]       [ 0]   [ 3]    Integer: 140024773
     * 02:20:05 [DroneDump]       [ 0]   [ 4]    Integer: 140024774
     *
*/
    call.Dump(DRONE__DUMP);

    //Drone* pDrone = m_sysMgr->GetSE()->GetDroneSE();
    //pDrone->DestinyMgr()->InitOrbit(pShipSE, 800);


    call.client->SendNotifyMsg("Drone Control is not implemented yet.");
    return PyStatic.mtDict();
}

PyResult EntityBound::Handle_CmdReturnBay(PyCallArgs &call) {
 // ret = entity.CmdReturnBay(droneIDs)
    /*
     * 02:20:49 [DroneTrace] EntityBound::Handle_CmdReturnBay()
     * 02:20:49 [DroneDump]   Call Arguments:
     * 02:20:49 [DroneDump]      Tuple: 1 elements
     * 02:20:49 [DroneDump]       [ 0]   List: 5 elements
     * 02:20:49 [DroneDump]       [ 0]   [ 0]    Integer: 140024770
     * 02:20:49 [DroneDump]       [ 0]   [ 1]    Integer: 140024771
     * 02:20:49 [DroneDump]       [ 0]   [ 2]    Integer: 140024772
     * 02:20:49 [DroneDump]       [ 0]   [ 3]    Integer: 140024773
     * 02:20:49 [DroneDump]       [ 0]   [ 4]    Integer: 140024774
     * 02:20:49 [DroneDump]       List: 5 elements
     * 02:20:49 [DroneDump]       [ 0]    Integer: 140024770
     * 02:20:49 [DroneDump]       [ 1]    Integer: 140024771
     * 02:20:49 [DroneDump]       [ 2]    Integer: 140024772
     * 02:20:49 [DroneDump]       [ 3]    Integer: 140024773
     * 02:20:49 [DroneDump]       [ 4]    Integer: 140024774
     *

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

    PyList* droneList = call.tuple->AsTuple()->GetItem(0)->AsList();


    //ship:scoopdrone()

    // returns nodeID and timestamp and dict of error msg
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
    return PyStatic.mtDict();
}

PyResult EntityBound::Handle_CmdAbandonDrone(PyCallArgs &call) {
 // ret = entity.CmdAbandonDrone(droneIDs)
    /*
     * 02:22:47 [DroneTrace] EntityBound::Handle_CmdAbandonDrone()
     * 02:22:47 [DroneDump]   Call Arguments:
     * 02:22:47 [DroneDump]      Tuple: 1 elements
     * 02:22:47 [DroneDump]       [ 0]   List: 5 elements
     * 02:22:47 [DroneDump]       [ 0]   [ 0]    Integer: 140024770
     * 02:22:47 [DroneDump]       [ 0]   [ 1]    Integer: 140024771
     * 02:22:47 [DroneDump]       [ 0]   [ 2]    Integer: 140024772
     * 02:22:47 [DroneDump]       [ 0]   [ 3]    Integer: 140024773
     * 02:22:47 [DroneDump]       [ 0]   [ 4]    Integer: 140024774
     *
     */
    _log(DRONE__TRACE, "EntityBound::Handle_CmdAbandonDrone()");
    call.Dump(DRONE__DUMP);

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
    return PyStatic.mtDict();
}

PyResult EntityBound::Handle_CmdReconnectToDrones(PyCallArgs &call) {
    // MAX_DRONE_RECONNECTS = 25
    // ret = entity.CmdReconnectToDrones(droneCandidates)
    //     for errStr, dicty in ret.iteritems():
    // this sends a list of drones in local space owned by calling character but not in droneState
    /*
     * 09:09:48 [DroneDump]   Call Arguments:
     * 09:09:48 [DroneDump]      Tuple: 1 elements
     * 09:09:48 [DroneDump]       [ 0]   List: 1 elements
     * 09:09:48 [DroneDump]       [ 0]   [ 0]    Integer: 140007055
     */
    _log(DRONE__TRACE, "EntityBound::Handle_CmdReconnectToDrones()");
    call.Dump(DRONE__DUMP);

    PyList* droneList = call.tuple->AsTuple()->GetItem(0)->AsList();

    SystemEntity* pSE(nullptr);
    PyList::const_iterator itr = droneList->begin();
    while (itr != droneList->end()) {
        pSE = m_sysMgr->GetEntityByID(PyRep::IntegerValueU32(*itr));
        if (pSE != nullptr)
            pSE->GetDroneSE()->Reconnect(m_pClient->GetShipSE());
        ++itr;
    }

    // NOTE:  drone will online after reconnect if available bandwidth

    return PyStatic.mtDict();
}


    //IDEA:  create drone channel or private message "i am sending my location".
    // binary:  01101001 00100000 01100001 01101101 00100000 01110011 01100101 01101110 01100100 01101001 01101110 01100111 00100000 01101101 01111001 00100000 01101100 01101111 01100011 01100001 01110100 01101001 01101111 01101110
    //   lost drones will ping solmap using bookmark
    //   distance 15-20km from actual, modded by skills

    //std::string text = "sending location"; //01110011 01100101 01101110 01100100 01101001 01101110 01100111 00100000 01101100 01101111 01100011 01100001 01110100 01101001 01101111 01101110
    // convert string to binary
    //BinString(text);
