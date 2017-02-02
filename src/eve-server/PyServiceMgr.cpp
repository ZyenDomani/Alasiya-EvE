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
    Author:        Zhur
*/

#include "eve-server.h"

#include "Client.h"
#include "EntityList.h"
#include "cache/ObjCacheService.h"
#include "chat/LSCService.h"
#include "PyService.h"
#include "PyServiceMgr.h"
#include "PyBoundObject.h"

PyServiceMgr::PyServiceMgr( uint32 nodeID, EntityList& elist, ItemFactory* ifactory )
: item_factory(ifactory),
  lsc_service(nullptr),
  cache_service(nullptr),
  m_nextBindID(100),
  m_nodeID(nodeID)
{
    elist.SetService(this);
}

PyServiceMgr::~PyServiceMgr() {
    Close();
}

void PyServiceMgr::Close() {
    for (auto cur : m_svcList)
        SafeDelete(cur.second);
    m_svcList.clear();

    PyBoundObject* bo = nullptr;
    for ( auto itr : m_boundObjects) {
        bo = itr.second.destination;
        _log(SERVICE__MESSAGE, "Service Mgr Destructor:  Deleting %s at node %u:%u", \
        bo->GetBoundObjectClassStr().c_str(), bo->m_nodeID, bo->m_bindID);
        SafeDelete(bo);
    }
    m_boundObjects.clear();
}

void PyServiceMgr::Initalize(double startTime)
{
    sLog.Cyan("     PyServiceMgr", "%u services registered in %.3fms", m_svcList.size(),(GetTimeMSeconds() - startTime));
}

void PyServiceMgr::Process() {
    //well... we used to have something to do, but not right now...
}

void PyServiceMgr::RegisterService(const std::string name, PyService* svc)
{
    m_svcList[name] = svc;
}

PyService* PyServiceMgr::LookupService(const std::string &name) {
    std::map<std::string, PyService*>::iterator itr = m_svcList.find(name);
    if (itr != m_svcList.end())
        return itr->second;

    sLog.Error("Service Mgr", "Service %s not in list.", name.c_str());
    return nullptr;
}

PySubStruct* PyServiceMgr::BindObject(Client* who, PyBoundObject* pObj, PyDict** dict) {
    if (!pObj) {
        sLog.Error("Service Mgr", "Tried to bind a NULL object!");
        return new PySubStruct(new PyNone());
    }

    pObj->_SetNodeBindID(GetNodeID(), _GetBindID());    //tell the object what its bind ID is.

    BoundObject obj;
    obj.client = who;
    obj.destination = pObj;

    m_boundObjects[pObj->bindID()] = obj;

    std::string bind_str = pObj->GetBindStr();
    _log(SERVICE__MESSAGE, "Service Mgr Binding %s to node %u:%u for %s", \
                pObj->GetBoundObjectClassStr().c_str(), pObj->m_nodeID, pObj->m_bindID, who->GetName());

    //not sure what this really is...
    uint64 expiration = Win32TimeNow() + Win32Time_Hour;

    PyTuple *objt(nullptr);
    if (!dict or !*dict) {
        objt = new PyTuple(2);
        objt->items[0] = new PyString(bind_str);
        objt->items[1] = new PyLong(expiration);    //expiration?
    } else {
        objt = new PyTuple(3);
        objt->items[0] = new PyString(bind_str);
        objt->items[1] = *dict; *dict = nullptr;    //consumed
        objt->items[2] = new PyLong(expiration);    //expiration?
    }

    return new PySubStruct(new PySubStream(objt));
}

void PyServiceMgr::ClearBoundObjects(Client* who) {
    /** @todo (Allan) make this better...could be quite expensive with many players */
    ObjectsBoundMapItr cur = m_boundObjects.begin();
    while (cur != m_boundObjects.end()) {
        if (cur->second.client == who) {
            PyBoundObject *bo = cur->second.destination;
            _log(SERVICE__MESSAGE, "Service Mgr Releasing bound object %s at %s for %s", \
                            bo->GetBoundObjectClassStr().c_str(), bo->GetBindStr().c_str(), who->GetName());
            bo->Release();
            m_boundObjects.erase(cur);
            cur = m_boundObjects.begin();
        } else
            ++cur;
    }
}

PyBoundObject* PyServiceMgr::FindBoundObject(uint32 bindID) {
    std::map<uint32, BoundObject>::iterator res = m_boundObjects.find(bindID);
    if (res != m_boundObjects.end())
        return res->second.destination;
    return nullptr;
}

void PyServiceMgr::ClearBoundObject(uint32 bindID)
{
    std::map<uint32, BoundObject>::iterator res = m_boundObjects.find(bindID);
    if (res == m_boundObjects.end()) {
        sLog.Error("Service Mgr", "Unable to find bound object %u to release.", bindID);
        return;
    }

    PyBoundObject *bo = res->second.destination;

    _log(SERVICE__MESSAGE, "Service Mgr Clearing bound object %s at %s", bo->GetBoundObjectClassStr().c_str(), bo->GetBindStr().c_str());

    m_boundObjects.erase(res);
    bo->Release();
}
