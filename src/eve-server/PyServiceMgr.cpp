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
    Author:        Zhur
    Updates:    Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "EntityMgr.h"
#include "cache/ObjCacheService.h"
#include "chat/LSCService.h"
#include "PyService.h"
#include "PyServiceMgr.h"
#include "PyBoundObject.h"

PyServiceMgr::PyServiceMgr(uint32 nodeID, EntityMgr& elist)
: lsc_service(nullptr),
  cache_service(nullptr),
  m_nextBindID(100),
  m_nodeID(nodeID)
{
    elist.SetService(this);
}

void PyServiceMgr::Close() {
    for (auto &cur : m_svcList)
        SafeDelete(cur.second);
    m_svcList.clear();

    PyBoundObject* bo(nullptr);
    auto it = m_boundObjects.begin();
    while (it != m_boundObjects.end()) {
        bo = it->second.object;
        if (bo != nullptr) {
            if (is_log_enabled(SERVICE__MESSAGE))
                _log(SERVICE__MESSAGE, "Service Mgr Destructor:  Deleting %s at node %u:%u", \
                    bo->GetName(), bo->m_nodeID, bo->m_bindID);
                it->second.object = nullptr;
            SafeDelete(bo);
        }
        ++it;
    }
    m_boundObjects.clear();

    sLog.Warning("     PyServiceMgr", "Services Manager has been closed." );
}

void PyServiceMgr::Initalize(double startTime) {
    //  look into this.  what's it for?  are we using it right?  missing anything?
    // client calls this, then loads cached data upon return.  not sure how this is used yet
    PyString* str = new PyString("machoNet.serviceInfo");
    if (!cache_service->IsCacheLoaded(str)) {
        PyDict *dict = new PyDict();
        /* ServiceCallGPCS.py:197
         *        where = self.machoNet.serviceInfo[service]
         *        if where:
         *            for (k, v,) in self.machoNet.serviceInfo.iteritems():
         *                if ((k != service) and (v and (v.startswith(where) or where.startswith(v)))):
         *                    nodeID = self.services.get(k, None)
         *                    break
         */
        PyString* station = new PyString("station");
        dict->SetItemString("account", station);
        PyIncRef(station);
        dict->SetItemString("bookmark", station);
        PyIncRef(station);
        dict->SetItemString("contractMgr", station);
        PyIncRef(station);
        dict->SetItemString("gangSvc", station);
        PyIncRef(station);
        dict->SetItemString("trademgr", station);
        PyIncRef(station);
        dict->SetItemString("tutorialSvc", station);
        PyIncRef(station);
        dict->SetItemString("slash", station);

        PyString* location = new PyString("location");
        dict->SetItemString("wormholeMgr", location);
        PyIncRef(location);
        dict->SetItemString("LSC", location);
        PyIncRef(location);
        dict->SetItemString("station", location);

        PyString* locationPreferred = new PyString("locationPreferred");
        dict->SetItemString("config", locationPreferred);

        PyString* solarsystem = new PyString("solarsystem");
        dict->SetItemString("scanMgr", solarsystem);
        PyIncRef(solarsystem);
        dict->SetItemString("keeper", solarsystem);

        dict->SetItemString("agentMgr", PyStatic.NewNone());
        dict->SetItemString("aggressionMgr", PyStatic.NewNone());
        dict->SetItemString("alert", PyStatic.NewNone());
        dict->SetItemString("allianceRegistry", PyStatic.NewNone());
        dict->SetItemString("authentication", PyStatic.NewNone());
        dict->SetItemString("billMgr", PyStatic.NewNone());
        dict->SetItemString("billingMgr", PyStatic.NewNone());
        dict->SetItemString("beyonce", PyStatic.NewNone());
        dict->SetItemString("BSD", PyStatic.NewNone());
        dict->SetItemString("cache", PyStatic.NewNone());
        dict->SetItemString("calendarProxy", PyStatic.NewNone());
        dict->SetItemString("corporationSvc", PyStatic.NewNone());
        dict->SetItemString("corpStationMgr", PyStatic.NewNone());
        dict->SetItemString("corpmgr", PyStatic.NewNone());
        dict->SetItemString("corpRegistry", PyStatic.NewNone());
        dict->SetItemString("counter", PyStatic.NewNone());
        dict->SetItemString("certificateMgr", PyStatic.NewNone());
        dict->SetItemString("charFittingMgr", PyStatic.NewNone());
        dict->SetItemString("charmgr", PyStatic.NewNone());
        dict->SetItemString("charUnboundMgr", PyStatic.NewNone());
        dict->SetItemString("clientStatLogger", PyStatic.NewNone());
        dict->SetItemString("clientStatsMgr", PyStatic.NewNone());
        dict->SetItemString("clones", PyStatic.NewNone());
        dict->SetItemString("damageTracker", PyStatic.NewNone());
        dict->SetItemString("dataconfig", PyStatic.NewNone());
        dict->SetItemString("DB", PyStatic.NewNone());
        dict->SetItemString("DB2", PyStatic.NewNone());
        dict->SetItemString("debug", PyStatic.NewNone());
        dict->SetItemString("director", PyStatic.NewNone());
        dict->SetItemString("dogma", PyStatic.NewNone());
        dict->SetItemString("dogmaIM", PyStatic.NewNone());
        dict->SetItemString("droneMgr", PyStatic.NewNone());
        dict->SetItemString("dungeon", PyStatic.NewNone());
        dict->SetItemString("dungeonExplorationMgr", PyStatic.NewNone());
        dict->SetItemString("effectCompiler", PyStatic.NewNone());
        dict->SetItemString("emailreader", PyStatic.NewNone());
        dict->SetItemString("entity", PyStatic.NewNone());
        dict->SetItemString("factory", PyStatic.NewNone());
        dict->SetItemString("facWarMgr", PyStatic.NewNone());
        dict->SetItemString("fleetMgr", PyStatic.NewNone());
        dict->SetItemString("fleetObjectHandler", PyStatic.NewNone());
        dict->SetItemString("fleetProxy", PyStatic.NewNone());
        dict->SetItemString("gagger", PyStatic.NewNone());
        dict->SetItemString("gangSvcObjectHandler", PyStatic.NewNone());
        dict->SetItemString("http", PyStatic.NewNone());
        dict->SetItemString("i2", PyStatic.NewNone());
        dict->SetItemString("infoGatheringMgr", PyStatic.NewNone());
        dict->SetItemString("insuranceSvc", PyStatic.NewNone());
        dict->SetItemString("invbroker", PyStatic.NewNone());
        dict->SetItemString("jumpbeaconsvc", PyStatic.NewNone());
        dict->SetItemString("jumpCloneSvc", PyStatic.NewNone());
        dict->SetItemString("languageSvc", PyStatic.NewNone());
        dict->SetItemString("lien", PyStatic.NewNone());
        dict->SetItemString("lookupSvc", PyStatic.NewNone());
        dict->SetItemString("lootSvc", PyStatic.NewNone());
        dict->SetItemString("LPSvc", PyStatic.NewNone());
        dict->SetItemString("LPStore", PyStatic.NewNone());
        dict->SetItemString("machoNet", PyStatic.NewNone());
        dict->SetItemString("map", PyStatic.NewNone());
        dict->SetItemString("market", PyStatic.NewNone());
        dict->SetItemString("npcSvc", PyStatic.NewNone());
        dict->SetItemString("objectCaching", PyStatic.NewNone());
        dict->SetItemString("onlineStatus", PyStatic.NewNone());
        dict->SetItemString("posMgr", PyStatic.NewNone());
        dict->SetItemString("ram", PyStatic.NewNone());
        dict->SetItemString("repairSvc", PyStatic.NewNone());
        dict->SetItemString("reprocessingSvc", PyStatic.NewNone());
        dict->SetItemString("pathfinder", PyStatic.NewNone());
        dict->SetItemString("petitioner", PyStatic.NewNone());
        dict->SetItemString("planetMgr", PyStatic.NewNone());
        dict->SetItemString("search", PyStatic.NewNone());
        dict->SetItemString("sessionMgr", PyStatic.NewNone());
        dict->SetItemString("ship", PyStatic.NewNone());
        dict->SetItemString("skillMgr", PyStatic.NewNone());
        dict->SetItemString("sovMgr", PyStatic.NewNone());
        dict->SetItemString("standing2", PyStatic.NewNone());
        dict->SetItemString("stationSvc", PyStatic.NewNone());
        dict->SetItemString("userSvc", PyStatic.NewNone());
        dict->SetItemString("voiceMgr", PyStatic.NewNone());
        dict->SetItemString("voucher", PyStatic.NewNone());
        dict->SetItemString("warRegistry", PyStatic.NewNone());
        dict->SetItemString("watchdog", PyStatic.NewNone());
        dict->SetItemString("zsystem", PyStatic.NewNone());
        dict->SetItemString("encounterSpawnServer", PyStatic.NewNone());
        dict->SetItemString("netStateServer", PyStatic.NewNone());
        dict->SetItemString("zActionServer", PyStatic.NewNone());

        //register it
        PyRep* pDictPayload = dict;
        cache_service->GiveCache(str, (PyRep **)&pDictPayload);
    }

    sLog.Cyan("     PyServiceMgr", "%lu services registered in %.3fms", m_svcList.size(),(GetTimeMSeconds() - startTime));
}

void PyServiceMgr::Process() {
    //well... we used to have something to do, but not right now...
}

void PyServiceMgr::RegisterService(const std::string &name, PyService* svc) {
    m_svcList[name] = svc;
}

PyService* PyServiceMgr::LookupService(const std::string &name) {
    std::map<std::string, PyService*>::const_iterator itr = m_svcList.find(name);
    if (itr != m_svcList.end())
        return itr->second;

    _log(SERVICE__ERROR, "PyServiceMgr::LookupService() - Service %s not found in list.", name.c_str());
    return nullptr;
}

PySubStruct* PyServiceMgr::BindObject(Client* pClient, PyBoundObject* pObj, PyDict* dict/*nullptr*/, PyDict *oid/*nullptr*/) {
    if (pClient == nullptr) {
        _log(SERVICE__ERROR, "PyServiceMgr::BindObject() - Tried to bind a NULL client.");
        return new PySubStruct(PyStatic.NewNone());
    }

    if (pObj == nullptr) {
        _log(SERVICE__ERROR, "PyServiceMgr::BindObject() - Tried to bind a NULL object.");
        return new PySubStruct(PyStatic.NewNone());
    }

    pObj->_SetNodeBindID(m_nodeID, ++m_nextBindID);

    // save bindID in client for ease of release later
    pClient->AddBindID(m_nextBindID);

    BoundObj obj = BoundObj();
        obj.client = pClient;
        obj.object = pObj;
    m_boundObjects[pObj->bindID()] = obj;

    std::string bindStr = pObj->GetBindStr();
    _log(SERVICE__MESSAGE, "Service Mgr Binding %s to node %u:%u for %s", \
                pObj->GetName(), pObj->m_nodeID, pObj->m_bindID, pClient->GetName());

    PyLong* time = new PyLong(GetFileTimeNow());

    PyTuple* tuple(nullptr);
    if (dict == nullptr) {
        tuple = new PyTuple(2);
        tuple->items[0] = new PyString(bindStr);
        tuple->items[1] = time;
    } else {
        tuple = new PyTuple(3);
        tuple->items[0] = new PyString(bindStr);
        tuple->items[1] = dict;
        tuple->items[2] = time;
    }

    if (oid != nullptr) {
        PyDict *oidDict = new PyDict();
        PyIncRef(time);
        oidDict->SetItemString(bindStr.c_str(), time);
        oid->SetItemString("OID+", oidDict);
    }

    return new PySubStruct(new PySubStream(tuple));
}

PyBoundObject* PyServiceMgr::FindBoundObject(uint32 bindID) {
    std::map<uint32, BoundObj>::iterator itr = m_boundObjects.find(bindID);
    if (itr != m_boundObjects.end())
        return itr->second.object;
    return nullptr;
}

void PyServiceMgr::ClearBoundObject(uint32 bindID)
{
    std::map<uint32, BoundObj>::iterator itr = m_boundObjects.find(bindID);
    if (itr == m_boundObjects.end()) {
        _log(SERVICE__ERROR, "PyServiceMgr::ClearBoundObject() - Unable to find bound object %u to release.", bindID);
        return;
    }

    PyBoundObject* bo = itr->second.object;

    _log(SERVICE__MESSAGE, "Service Mgr Clearing bound object %s at %s", bo->GetName(), bo->GetBindStr().c_str());

    m_boundObjects.erase(itr);
    SafeDelete(bo);
}

void PyServiceMgr::BoundObjectVec(std::vector< BoundObj >& vec)
{
    for (auto &cur : m_boundObjects)
        vec.push_back(cur.second);
}

