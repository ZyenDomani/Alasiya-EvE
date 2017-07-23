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

#include "PyServiceCD.h"
#include "StaticDataMgr.h"
#include "cache/ObjCacheService.h"
#include "map/MapService.h"

PyCallable_Make_InnerDispatcher(MapService)

MapService::MapService(PyServiceMgr *mgr)
: PyService(mgr, "map"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(MapService, GetStationExtraInfo);
    PyCallable_REG_CALL(MapService, GetSolarSystemPseudoSecurities);
    PyCallable_REG_CALL(MapService, GetSolarSystemVisits);
    PyCallable_REG_CALL(MapService, GetBeaconCount);
    PyCallable_REG_CALL(MapService, GetHistory);
    PyCallable_REG_CALL(MapService, GetStationCount);    //ColorStarsByStationCount

    /**  not handled yet...these are empty calls  */
    PyCallable_REG_CALL(MapService, GetStuckSystems);
    PyCallable_REG_CALL(MapService, GetRecentSovActivity);
    PyCallable_REG_CALL(MapService, GetDeadspaceAgentsMap);
    PyCallable_REG_CALL(MapService, GetDeadspaceComplexMap);
    PyCallable_REG_CALL(MapService, GetIncursionGlobalReport);
    PyCallable_REG_CALL(MapService, GetSystemsInIncursions);
    PyCallable_REG_CALL(MapService, GetSystemsInIncursionsGM);
    PyCallable_REG_CALL(MapService, GetVictoryPoints);
    PyCallable_REG_CALL(MapService, GetMyExtraMapInfo);
    PyCallable_REG_CALL(MapService, GetMyExtraMapInfoAgents);  //ColorStarsByMyAgents
    PyCallable_REG_CALL(MapService, GetAllianceJumpBridges);
    PyCallable_REG_CALL(MapService, GetAllianceBeacons);
    PyCallable_REG_CALL(MapService, GetLinkableJumpArrays);
    PyCallable_REG_CALL(MapService, GetCurrentSovData);

}

MapService::~MapService() {
    delete m_dispatch;
}

PyResult MapService::Handle_GetStationExtraInfo(PyCallArgs &call) {
    //takes no arguments
    //returns tuple(
    //     stations: rowset stationID,solarSystemID,operationID,stationTypeID,ownerID
    //     opservices: rowset: (operationID, serviceID) (from staOperationServices)
    //     services: rowset: (serviceID,serviceName) (from staServices)
    // )

    PyRep *result = NULL;

    ObjectCachedMethodID method_id(GetName(), "GetStationExtraInfo");

    //uint32 systemID = call.client->GetSystemID();

    //check to see if this method is in the cache already.
    if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
        //this method is not in cache yet, load up the contents and cache it.

        PyTuple *resultt = new PyTuple(3);

        resultt->items[0] = m_db.GetStationExtraInfo();
        if(resultt->items[0] == NULL) {
            codelog(SERVICE__ERROR, "Failed to query station info");
            return NULL;
        }

        resultt->items[1] = m_db.GetStationOpServices();
        if(resultt->items[1] == NULL) {
            codelog(SERVICE__ERROR, "Failed to query op services");
            return NULL;
        }

        resultt->items[2] = m_db.GetStationServiceInfo();
        if(resultt->items[2] == NULL) {
            codelog(SERVICE__ERROR, "Failed to query service info");
            return NULL;
        }

        result = resultt;
        m_manager->cache_service->GiveCache(method_id, &result);
    }

    //now we know its in the cache one way or the other, so build a
    //cached object cached method call result.
    result = m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id);

    return result;
}


PyResult MapService::Handle_GetSolarSystemPseudoSecurities(PyCallArgs &call) {
    PyRep *result = NULL;

    ObjectCachedMethodID method_id(GetName(), "GetSolarSystemPseudoSecurities");

    //check to see if this method is in the cache already.
    if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
        //this method is not in cache yet, load up the contents and cache it.
        result = m_db.GetPseudoSecurities();
        if(result == NULL)
            result = new PyNone();
        m_manager->cache_service->GiveCache(method_id, &result);
    }

    //now we know its in the cache one way or the other, so build a
    //cached object cached method call result.
    result = m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id);

    return result;
}

PyResult MapService::Handle_GetSolarSystemVisits(PyCallArgs &call)
{       // passes no args
    uint32 charID = call.client->GetCharacterID();
    return (m_db.GetSolSystemVisits(charID));
}

PyResult MapService::Handle_GetHistory(PyCallArgs &call) {
    uint32 int1 = call.tuple->GetItem(0)->AsInt()->value();
    uint32 int2 = call.tuple->GetItem(1)->AsInt()->value();
      sLog.White( "MapService::Handle_GetHistory()", "size= %u, type: (%u), timeframe: (%u)", call.tuple->size(), int1, int2 );

    return (m_db.GetDynamicData(int1, int2));
}

//02:38:07 L MapService::Handle_GetBeaconCount(): size= 0
PyResult MapService::Handle_GetBeaconCount(PyCallArgs &call) {
  /**
    def GetActiveCynos(self, itemID):
        data = sm.RemoteSvc('map').GetBeaconCount()
        systems = set(sm.GetService('map').IterateSolarSystemIDs(itemID))
        totalModules = 0
        totalStructures = 0
        for solarSystemID, counts in data.iteritems():
            if solarSystemID in systems:
                moduleCount, structureCount = counts
                totalModules += moduleCount
                totalStructures += structureCount

        return util.KeyVal(cynoModules=totalModules, cynoStructures=totalStructures)
*/
  /*
  PyTuple* res = new PyTuple();
  PyTuple* counts = new PyTuple();
  PyTuple* system = new PyTuple();
  */
    return (m_db.GetDynamicData(2, 24));
}

//02:51:49 L MapService::Handle_GetStationCount(): size= 0
PyResult MapService::Handle_GetStationCount(PyCallArgs &call)
{  // cached on client side.  if cache is empty, this call is made.
  sLog.White( "MapService::Handle_GetStationCount()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
    return sDataMgr.GetStationCount();
}

/** not handled */

PyResult MapService::Handle_GetStuckSystems(PyCallArgs &call)
{
  sLog.White( "MapService::Handle_GetStuckSystems()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    uint8 none = 0;

    PyTuple* res = NULL;
    PyTuple* tuple0 = new PyTuple( 1 );

    tuple0->items[ 0 ] = new PyInt( none );

    res = tuple0;

    return res;
}

//22:49:23 L MapService::Handle_GetRecentSovActivity(): size= 0
PyResult MapService::Handle_GetRecentSovActivity(PyCallArgs &call)
{
    /** no packet data
        data = sm.RemoteSvc('map').GetRecentSovActivity()
        */
  sLog.White( "MapService::Handle_GetRecentSovActivity()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    PyRep *result = NULL;

    result = new PyDict();

    return result;
}

//   DED Agent Site Report
PyResult MapService::Handle_GetDeadspaceAgentsMap(PyCallArgs &call)
{/* no packet data
        dungeons = sm.RemoteSvc('map').GetDeadspaceAgentsMap(eve.session.languageID)
        solarSystemID, dungeonID, difficulty, dungeonName = dungeons
  sLog.White( "MapService::Handle_GetDeadspaceAgentsMap()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
*/
    PyRep *result = NULL;

    result = new PyDict();

    return result;
}

//  DED Deadspace Report
//22:37:54 L MapService::Handle_GetDeadspaceComplexMap(): size= 1
PyResult MapService::Handle_GetDeadspaceComplexMap(PyCallArgs &call)
{/* no packet data
        dungeons = sm.RemoteSvc('map').GetDeadspaceComplexMap(eve.session.languageID)
        solarSystemID, dungeonID, difficulty, dungeonName = dungeons
*/
  sLog.White( "MapService::Handle_GetDeadspaceComplexMap()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
    PyRep *result = NULL;

    result = new PyDict();

    return result;
}

//05:52:07 L MapService::Handle_GetIncursionGlobalReport(): size= 0
PyResult MapService::Handle_GetIncursionGlobalReport(PyCallArgs &call) {
  /**
            report = sm.RemoteSvc('map').GetIncursionGlobalReport()
            rewardGroupIDs = [ r.rewardGroupID for r in report ]
            delayedRewards = sm.GetService('incursion').GetDelayedRewardsByGroupIDs(rewardGroupIDs)
            scrolllist = []
            factionsToPrime = set()
            for data in report:
                data.jumps = GetJumps(data.stagingSolarSystemID)
                data.influenceData = util.KeyVal(influence=data.influence, lastUpdated=data.lastUpdated, graceTime=data.graceTime, decayRate=data.decayRate)
                ssitem = map.GetItem(data.stagingSolarSystemID)
                data.stagingSolarSystemName = ssitem.itemName
                data.security = map.GetSecurityStatus(data.stagingSolarSystemID)
                data.constellationID = ssitem.locationID
                data.constellationName = map.GetItem(ssitem.locationID).itemName
                data.factionID = ssitem.factionID or starmap.GetAllianceSolarSystems().get(data.stagingSolarSystemID, None)
                factionsToPrime.add(data.factionID)
                rewards = delayedRewards.get(data.rewardGroupID, None)
                data.loyaltyPoints = rewards[0].rewardQuantity if rewards else 0
                scrolllist.append(listentry.Get('GlobalIncursionReportEntry', data))
*/
  /*
      [PySubStream 1128 bytes]
        [PyObjectData Name: objectCaching.CachedMethodCallResult]
          [PyTuple 3 items]
            [PyDict 1 kvp]
              [PyString "versionCheck"]
              [PyTuple 3 items]
                [PyNone]
                [PyNone]
                [PyString "15 minutes"]
            [PySubStream 1027 bytes]
              [PyList 7 items]
                [PyObjectData Name: util.KeyVal]
                  [PyDict 9 kvp]
                    [PyString "graceTime"]
                    [PyFloat 30]
                    [PyString "decayRate"]
                    [PyFloat 0.00999999977648258]
                    [PyString "influence"]
                    [PyFloat 0.0145000005140901]
                    [PyString "lastUpdated"]
                    [PyIntegerVar 129492976800000000]
                    [PyString "state"]
                    [PyInt 1]
                    [PyString "hasBoss"]
                    [PyInt 0]
                    [PyString "stagingSolarSystemID"]
                    [PyInt 30004323]
                    [PyString "rewardGroupID"]
                    [PyInt 192]
                    [PyString "taleID"]
                    [PyInt 192]
            [PyList 2 items]
              [PyIntegerVar 129493861959830226]
              [PyInt -950263469]
              */
  sLog.White( "MapService::Handle_GetIncursionGlobalReport()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return NULL;
}

PyResult MapService::Handle_GetSystemsInIncursions(PyCallArgs &call) {
  /**  solarSystemID, sceneType
                    sceneType = staging, vanguard

  */
  sLog.White( "MapService::Handle_GetSystemsInIncursions()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return NULL;
}

//20:38:53 L MapService::Handle_GetSystemsInIncursionsGM(): size= 0
PyResult MapService::Handle_GetSystemsInIncursionsGM(PyCallArgs &call) {
  /**  solarSystemID, sceneType
                    sceneType = staging, vanguard, assault, headquarters

  */
    call.Dump(SERVICE__CALL_DUMP);
    return NULL;
}

//   factional warfare shit
//https://wiki.eveonline.com/en/wiki/Victory_Points_and_Command_Bunker
PyResult MapService::Handle_GetVictoryPoints(PyCallArgs &call)
{/**           factionID, viewmode, solarsystemid, threshold, current in oldhistory.iteritems():
                 */
  sLog.White( "MapService::Handle_GetVictoryPoints()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return NULL;
}

//06:04:50 L MapService::Handle_GetMyExtraMapInfoAgents(): size= 0
PyResult MapService::Handle_GetMyExtraMapInfoAgents(PyCallArgs &call)  //ColorStarsByMyAgents
{
     /**
    standingInfo = sm.RemoteSvc('map').GetMyExtraMapInfoAgents().Index('fromID')        <<<  .Index(columnName) means a rowset or crowset
              fromID, (to)factionID, (to)corporationID, (to)agentID
     */
  sLog.White( "MapService::Handle_GetMyExtraMapInfoAgents()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return NULL;
}

PyResult MapService::Handle_GetMyExtraMapInfo(PyCallArgs &call)
{
     /**       ColorStarsByCorpMembers
              locationID, characterID
     */
  sLog.White( "MapService::Handle_GetMyExtraMapInfo()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return NULL;
}

PyResult MapService::Handle_GetAllianceJumpBridges(PyCallArgs &call)
{
     /**

        bridgesByLocation = m.GetAllianceJumpBridges()
        for toLocID, fromLocID in bridgesByLocation:
            */
  sLog.White( "MapService::Handle_GetAllianceJumpBridges()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return NULL;
}

PyResult MapService::Handle_GetAllianceBeacons(PyCallArgs &call)
{/**
            beacons = sm.RemoteSvc('map').GetAllianceBeacons()
            for solarSystemID, structureID, structureTypeID in beacons:
                if solarSystemID != session.solarsystemid:
                    solarsystem = cfg.evelocations.Get(solarSystemID)
                    invType = cfg.invtypes.Get(structureTypeID)
                    structureName = uiutil.MenuLabel('UI/Menusvc/BeaconLabel', {'name': invType.name,
                     'system': solarSystemID})
                    allianceMenu.append((solarsystem.name, (solarSystemID, structureID, structureName)))
                    */
  sLog.White( "MapService::Handle_GetAllianceBeacons()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return NULL;
}

PyResult MapService::Handle_GetLinkableJumpArrays(PyCallArgs &call)
{
    /*
    for solarSystemID, structureID in sm.RemoteSvc('map').GetLinkableJumpArrays():
             */
  sLog.White( "MapService::Handle_GetLinkableJumpArrays()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return NULL;
}

PyResult MapService::Handle_GetCurrentSovData(PyCallArgs &call)
{/**
            data = sm.RemoteSvc('map').GetCurrentSovData(constellationID)
            returns locationID, ?
            return sm.RemoteSvc('map').GetCurrentSovData(locationID)
             */
  sLog.White( "MapService::Handle_GetCurrentSovData()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return NULL;
}
