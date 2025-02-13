
 /**
  * @name EntityService.h
  *   Drone Control class
  * @Author:    Allan
  * @date:      06 November 2016
  */


#ifndef __EVEMU_NPC_ENTITY_H
#define __EVEMU_NPC_ENTITY_H

#include "../eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"

class SystemManager;

class EntityService
: public PyService
{
public:
    EntityService(PyServiceMgr *mgr);
    virtual ~EntityService();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    //overloaded in order to support bound objects:
    virtual PyBoundObject *CreateBoundObject(Client* pClient, const PyRep* bind_args);
};

class EntityBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(EntityBound)

    EntityBound(PyServiceMgr* mgr, Client* pClient);
    virtual ~EntityBound() { delete m_dispatch; }

    PyCallable_DECL_CALL(CmdEngage);
    PyCallable_DECL_CALL(CmdRelinquishControl);
    PyCallable_DECL_CALL(CmdDelegateControl);
    PyCallable_DECL_CALL(CmdAssist);
    PyCallable_DECL_CALL(CmdGuard);
    PyCallable_DECL_CALL(CmdMine);
    PyCallable_DECL_CALL(CmdMineRepeatedly);
    PyCallable_DECL_CALL(CmdUnanchor);
    PyCallable_DECL_CALL(CmdReturnHome);
    PyCallable_DECL_CALL(CmdReturnBay);
    PyCallable_DECL_CALL(CmdAbandonDrone);
    PyCallable_DECL_CALL(CmdReconnectToDrones);

protected:
    // helper methods for multiple checks
    void CheckTarget(SystemEntity* pTarget, PyList* droneList, PyDict* errorDict);  // verify valid target
    void CheckTower(SystemEntity* pTarget, PyList* droneList, PyDict* errorDict);   // control tower checks
    void CheckFleet(SystemEntity* pTarget, PyList* droneList, PyDict* errorDict);   // fleet checks
    void CheckMisc(SystemEntity* pTarget, PyList* droneList, PyDict* errorDict);    // delegating control checks
    void CheckSkills(SystemEntity* pTarget, PyList* droneList, PyDict* errorDict);  // delegating control checks

private:
    Dispatcher *const m_dispatch;
    SystemManager* m_sysMgr;
    Client* m_pClient;
    ShipSE* m_shipSE;

    bool m_attack;
    bool m_delegate;
};

#endif  // __EVEMU_NPC_ENTITY_H

/*  not sure about these...
 * {'FullPath': u'UI/Messages', 'messageID': 259638, 'label': u'EntityToolRequirementsBody'}(u'You cannot do that. The drone you use on the {tGroup} needs to have the following in its skill requirements: {skills}.', None, {u'{tGroup}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'tGroup'}, u'{skills}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'skills'}})
 * {'FullPath': u'UI/Messages', 'messageID': 259504, 'label': u'DronesNotLoadableInSpaceBody'}(u'You cannot load to or unload from the drone bay while in space.', None, None)
 * {'FullPath': u'UI/Messages', 'messageID': 259711, 'label': u'EntityInvalidTargetBody'}(u'{targetTypeName} can only perform that action on an item of group  {desiredTarget}.', None, {u'{targetTypeName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetTypeName'}, u'{desiredTarget}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'desiredTarget'}})
 */
