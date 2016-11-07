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
    Author:     Allan
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

    //overloaded in order to support bound objects:
    virtual PyBoundObject *_CreateBoundObject(Client* pClient, const PyRep* bind_args);
};

class EntityBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(EntityBound)

    EntityBound(PyServiceMgr* mgr, SystemManager* systemMgr, uint32 systemID, uint32 unknown);
    virtual ~EntityBound() { delete m_dispatch; }
    virtual void Release() {
        //I hate this statement
        delete this;
    }

protected:
    Dispatcher *const m_dispatch;
    SystemManager* m_sysMgr;

    uint32 m_systemID;
    uint32 m_unknown;
};

#endif  // __EVEMU_NPC_ENTITY_H

