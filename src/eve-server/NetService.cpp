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
    Author:        Zhur, Allan
*/

#include "eve-server.h"

#include "NetService.h"
#include "PyServiceCD.h"
#include "cache/ObjCacheService.h"

PyCallable_Make_InnerDispatcher(NetService)

NetService::NetService(PyServiceMgr *mgr)
: PyService(mgr, "machoNet"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(NetService, GetTime);
    PyCallable_REG_CALL(NetService, GetInitVals);
    PyCallable_REG_CALL(NetService, GetClusterSessionStatistics);
}

NetService::~NetService() {
    delete m_dispatch;
}

PyResult NetService::Handle_GetTime(PyCallArgs &call) {
    return new PyLong(GetFileTimeNow());
}

PyResult NetService::Handle_GetClusterSessionStatistics(PyCallArgs &call)
{
    // got this shit working once i understood what the client wanted....only took 4 years
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT solarSystemID, pilotsDocked, pilotsInSpace FROM mapDynamicData WHERE active = 1");
    /** @todo  instead of hitting db, call solarsystem to get docked/inspace and NOT afk
     *   client already has IsAFK()
     */

    uint16 system = 0;
    PyDict* sol = new PyDict();
    PyDict* sta = new PyDict();

    DBResultRow row;
    while (res.GetRow(row)) {
        system = row.GetUInt(0) - 30000000;
        sol->SetItem(new PyInt(system), new PyInt(row.GetUInt(1) + row.GetUInt(2)));    // inspace + docked = total
        sta->SetItem(new PyInt(system), new PyInt(row.GetUInt(2)));                     // total - docked
    }

    PyTuple *result = new PyTuple(3);
    result->SetItem(0, sol);
    result->SetItem(1, sta);
    result->SetItem(2, new PyFloat(1)); //statDivisor

    return result;
}

/** @note:  wtf is this used for???  */
PyResult NetService::Handle_GetInitVals(PyCallArgs &call) {
    PyString* str = new PyString( "machoNet.serviceInfo" );

    if (!m_manager->cache_service->IsCacheLoaded(str)) {
        PyDict *dict = new PyDict();
        /* ServiceCallGPCS.py:197
        where = self.machoNet.serviceInfo[service]
        if where:
            for (k, v,) in self.machoNet.serviceInfo.iteritems():
                if ((k != service) and (v and (v.startswith(where) or where.startswith(v)))):
                    nodeID = self.services.get(k, None)
                    break
        */
		dict->SetItemString("account", new PyString("station"));
		dict->SetItemString("bookmark", new PyString("station"));
		dict->SetItemString("contractMgr", new PyString("station"));
		dict->SetItemString("gangSvc", new PyString("station"));
        dict->SetItemString("trademgr", new PyString("station"));
        dict->SetItemString("tutorialSvc", new PyString("station"));
        dict->SetItemString("slash", new PyString("station"));
        dict->SetItemString("wormholeMgr", new PyString("station"));
        dict->SetItemString("LSC", new PyString("location"));
        dict->SetItemString("station", new PyString("location"));
        dict->SetItemString("config", new PyString("locationPreferred"));
        dict->SetItemString("scanMgr", new PyString("solarsystem"));
        dict->SetItemString("keeper", new PyString("solarsystem"));
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
		dict->SetItemString("CalendarProxy", PyStatic.NewNone());
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

        //register it
        m_manager->cache_service->GiveCache(str, (PyRep **)&dict);
    }

    PyRep* serverinfo = m_manager->cache_service->GetCacheHint(str);
    PyDecRef( str );

    PyDict* initvals = new PyDict();

    PyTuple* result = new PyTuple( 2 );
        result->SetItem( 0, serverinfo );
        result->SetItem( 1, initvals );
    return result;
}
