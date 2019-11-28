
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

    EntityBound(PyServiceMgr* mgr, SystemManager* systemMgr, uint32 systemID);
    virtual ~EntityBound() { delete m_dispatch; }
    virtual void Release() {
        //I hate this statement
        delete this;
    }

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
    Dispatcher *const m_dispatch;
    SystemManager* m_sysMgr;

    uint32 m_systemID;
};

#endif  // __EVEMU_NPC_ENTITY_H

