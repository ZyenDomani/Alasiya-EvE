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
    Updates:    Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "EntityList.h"
#include "cache/ObjCacheService.h"
#include "chat/LSCService.h"
#include "PyService.h"
#include "PyServiceMgr.h"
#include "PyBoundObject.h"

PyServiceMgr::PyServiceMgr( uint32 nodeID, EntityList& elist )
: lsc_service(nullptr),
  cache_service(nullptr),
  m_nextBindID(100),
  m_nodeID(nodeID)
{
    elist.SetService(this);
}

PyServiceMgr::~PyServiceMgr() {
    // these crash (segfault) on exit, and i dont know why
    //SafeDelete(lsc_service);
    //SafeDelete(cache_service);

    Close();
}

void PyServiceMgr::Close() {
    for (auto cur : m_svcList)
        SafeDelete(cur.second);
    m_svcList.clear();

    PyBoundObject* bo(nullptr);
    for (auto cur : m_boundObjects) {
        bo = cur.second.destination;
        if (is_log_enabled(SERVICE__MESSAGE))
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

void PyServiceMgr::RegisterService(const std::string &name, PyService* svc)
{
    m_svcList[name] = svc;
}

PyService* PyServiceMgr::LookupService(const std::string &name) {
    std::map<std::string, PyService*>::const_iterator itr = m_svcList.find(name);
    if (itr != m_svcList.end())
        return itr->second;

    _log(SERVICE__ERROR,  "PyServiceMgr::LookupService() - Service %s not found in list.", name.c_str());
    return nullptr;
}

PySubStruct* PyServiceMgr::BindObject(Client* pClient, PyBoundObject* pObj, PyDict* dict) {
    if (pClient == nullptr) {
        _log(SERVICE__ERROR,  "PyServiceMgr::BindObject() - Tried to bind a NULL client.");
        return new PySubStruct(PyStatic.NewNone());
    }

    if (pObj == nullptr) {
        _log(SERVICE__ERROR,  "PyServiceMgr::BindObject() - Tried to bind a NULL object.");
        return new PySubStruct(PyStatic.NewNone());
    }

    pObj->_SetNodeBindID(m_nodeID, ++m_nextBindID);    //tell the object what its bind ID is.

    BoundObject obj = BoundObject();
        obj.client = pClient;
        obj.destination = pObj;
    m_boundObjects[pObj->bindID()] = obj;

    std::string bindStr = pObj->GetBindStr();
    _log(SERVICE__MESSAGE, "Service Mgr Binding %s to node %u:%u for %s", \
                pObj->GetBoundObjectClassStr().c_str(), pObj->m_nodeID, pObj->m_bindID, pClient->GetName());

    PyTuple* tuple(nullptr);
    if (dict == nullptr) {
        tuple = new PyTuple(2);
        tuple->items[0] = new PyString(bindStr);
        tuple->items[1] = new PyLong(GetFileTimeNow());
    } else {
        tuple = new PyTuple(3);
        tuple->items[0] = new PyString(bindStr);
        tuple->items[1] = dict;
        tuple->items[2] = new PyLong(GetFileTimeNow());
    }

    return new PySubStruct(new PySubStream(tuple));
}

void PyServiceMgr::ClearBoundObjects(Client* pClient) {
    /** @todo (Allan) make this better...could be quite expensive with many players */
    ObjectsBoundMapItr itr = m_boundObjects.begin();
    while (itr != m_boundObjects.end()) {
        if (itr->second.client == pClient) {
            PyBoundObject *bo(itr->second.destination);
            _log(SERVICE__MESSAGE, "Service Mgr Releasing bound object %s at %s for %s", \
                            bo->GetBoundObjectClassStr().c_str(), bo->GetBindStr().c_str(), pClient->GetName());
            bo->Release();
            itr = m_boundObjects.erase(itr);
        } else
            ++itr;
    }
}

PyBoundObject* PyServiceMgr::FindBoundObject(uint32 bindID) {
    std::map<uint32, BoundObject>::iterator itr = m_boundObjects.find(bindID);
    if (itr != m_boundObjects.end())
        return itr->second.destination;
    return nullptr;
}

void PyServiceMgr::ClearBoundObject(uint32 bindID)
{
    std::map<uint32, BoundObject>::iterator itr = m_boundObjects.find(bindID);
    if (itr == m_boundObjects.end()) {
        _log(SERVICE__ERROR,  "PyServiceMgr::ClearBoundObject() - Unable to find bound object %u to release.", bindID);
        return;
    }

    PyBoundObject *bo(itr->second.destination);

    _log(SERVICE__MESSAGE, "Service Mgr Clearing bound object %s at %s", bo->GetBoundObjectClassStr().c_str(), bo->GetBindStr().c_str());

    m_boundObjects.erase(itr);
    bo->Release();
}
