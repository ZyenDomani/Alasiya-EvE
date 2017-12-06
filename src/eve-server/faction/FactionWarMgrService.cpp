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
    Author:        Zhur (outline and 3 calls)
    Updates:    Allan
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "cache/ObjCacheService.h"
#include "faction/FactionWarMgrService.h"

/*
 * FACWAR__ERROR
 * FACWAR__WARNING
 * FACWAR__INFO
 * FACWAR__MESSAGE
 * FACWAR__TRACE
 * FACWAR__CALL
 * FACWAR__CALL_DUMP
 * FACWAR__RSP_DUMP
 */

PyCallable_Make_InnerDispatcher(FactionWarMgrService)

FactionWarMgrService::FactionWarMgrService(PyServiceMgr *mgr)
: PyService(mgr, "facWarMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(FactionWarMgrService, GetWarFactions);
    PyCallable_REG_CALL(FactionWarMgrService, GetFWSystems);
    PyCallable_REG_CALL(FactionWarMgrService, GetMyCharacterRankOverview);
    PyCallable_REG_CALL(FactionWarMgrService, GetMyCharacterRankInfo);
    PyCallable_REG_CALL(FactionWarMgrService, GetFactionMilitiaCorporation);
    PyCallable_REG_CALL(FactionWarMgrService, GetCharacterRankInfo);
    PyCallable_REG_CALL(FactionWarMgrService, GetFactionalWarStatus);
    PyCallable_REG_CALL(FactionWarMgrService, GetSystemStatus);
    PyCallable_REG_CALL(FactionWarMgrService, IsEnemyFaction);
    PyCallable_REG_CALL(FactionWarMgrService, JoinFactionAsCharacter);
    PyCallable_REG_CALL(FactionWarMgrService, GetCorporationWarFactionID);
    PyCallable_REG_CALL(FactionWarMgrService, IsEnemyCorporation);
    PyCallable_REG_CALL(FactionWarMgrService, GetSystemsConqueredThisRun);
    PyCallable_REG_CALL(FactionWarMgrService, GetFactionCorporations);
    PyCallable_REG_CALL(FactionWarMgrService, JoinFactionAsCharacterRecommendationLetter);
    PyCallable_REG_CALL(FactionWarMgrService, JoinFactionAsAlliance);
    PyCallable_REG_CALL(FactionWarMgrService, JoinFactionAsCorporation);
    PyCallable_REG_CALL(FactionWarMgrService, GetStats_FactionInfo);
    PyCallable_REG_CALL(FactionWarMgrService, GetStats_TopAndAllKillsAndVPs);
    PyCallable_REG_CALL(FactionWarMgrService, GetStats_Character);
    PyCallable_REG_CALL(FactionWarMgrService, GetStats_Alliance);
    PyCallable_REG_CALL(FactionWarMgrService, GetStats_Militia);
    PyCallable_REG_CALL(FactionWarMgrService, GetStats_CorpPilots);
    PyCallable_REG_CALL(FactionWarMgrService, LeaveFactionAsAlliance);
    PyCallable_REG_CALL(FactionWarMgrService, LeaveFactionAsCorporation);
    PyCallable_REG_CALL(FactionWarMgrService, WithdrawJoinFactionAsAlliance);
    PyCallable_REG_CALL(FactionWarMgrService, WithdrawJoinFactionAsCorporation);
    PyCallable_REG_CALL(FactionWarMgrService, WithdrawLeaveFactionAsAlliance);
    PyCallable_REG_CALL(FactionWarMgrService, WithdrawLeaveFactionAsCorporation);
    PyCallable_REG_CALL(FactionWarMgrService, RefreshCorps);

}

FactionWarMgrService::~FactionWarMgrService()
{
    delete m_dispatch;
}

PyResult FactionWarMgrService::Handle_GetWarFactions(PyCallArgs &call) {
    ObjectCachedMethodID method_id(GetName(), "GetWarFactions");

    if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
        PyRep *res = m_db.GetWarFactions();
        if(res == NULL)
            return nullptr;
        m_manager->cache_service->GiveCache(method_id, &res);
    }

    return m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id);
}

PyResult FactionWarMgrService::Handle_GetFWSystems( PyCallArgs& call )
{
    ObjectCachedMethodID method_id( GetName(), "GetFacWarSystems" );

    if( !m_manager->cache_service->IsCacheLoaded( method_id ) )
    {
        PyRep* res = m_db.GetFacWarSystems();
        if( res == NULL )
            return nullptr;

        m_manager->cache_service->GiveCache( method_id, &res );
    }

    return m_manager->cache_service->MakeObjectCachedMethodCallResult( method_id );
}

PyResult FactionWarMgrService::Handle_GetMyCharacterRankOverview( PyCallArgs& call ) {
    /**
     * 15:21:59 L FactionWarMgrService::Handle_GetMyCharacterRankOverview(): size= 0
     * 15:21:59 [PacketError] Encode util_Rowset: lines is NULL! hacking in an empty list.
     */

// will need data from DB...
  util_Rowset rs;

    rs.header.push_back( "currentRank" );
    rs.header.push_back( "highestRank" );
    rs.header.push_back( "factionID" );
    rs.header.push_back( "lastModified" );

    return rs.Encode();
}

PyResult FactionWarMgrService::Handle_GetMyCharacterRankInfo( PyCallArgs& call ) {
  sLog.White( "FactionWarMgrService::Handle_GetMyCharacterRankInfo()", "size= %u", call.tuple->size() );
  call.Dump(FACWAR__CALL_DUMP);
  util_Rowset rs;

    rs.header.push_back( "currentRank" );
    rs.header.push_back( "highestRank" );
    rs.header.push_back( "factionID" );
    rs.header.push_back( "lastModified" );

    return rs.Encode();
}

PyResult FactionWarMgrService::Handle_GetFactionMilitiaCorporation(PyCallArgs &call) {
    /* 05:39:07 [SvcCall] Service facWarMgr: calling GetFactionMilitiaCorporation
     * 05:39:07 FactionWarMgrService::Handle_GetFactionMilitiaCorporation(): size= 1
     * 05:39:07 [SvcCall]   Call Arguments:
     * 05:39:07 [SvcCall]       Tuple: 1 elements
     * 05:39:07 [SvcCall]         [ 0] Integer field: 500002
     * 05:39:07 [SvcCall]   Call Named Arguments:
     * 05:39:07 [SvcCall]     Argument 'machoVersion':
     * 05:39:07 [SvcCall]         Integer field: 1
     */
  sLog.White( "FactionWarMgrService::Handle_GetFactionMilitiaCorporation()", "size= %u", call.tuple->size() );
  call.Dump(FACWAR__CALL_DUMP);
    Call_SingleIntegerArg arg;
    if(!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }
    return (new PyInt(m_db.GetFactionMilitiaCorporation(arg.arg)));
}

PyResult FactionWarMgrService::Handle_GetCharacterRankInfo(PyCallArgs &call) {
  sLog.White( "FactionWarMgrService::Handle_GetCharacterRankInfo()", "size= %u", call.tuple->size() );
  call.Dump(FACWAR__CALL_DUMP);

  return nullptr;
}

//22:48:28 L FactionWarMgrService::Handle_GetFactionalWarStatus(): size= 0
PyResult FactionWarMgrService::Handle_GetFactionalWarStatus(PyCallArgs &call) {
  sLog.White( "FactionWarMgrService::Handle_GetFactionalWarStatus()", "size=%u ", call.tuple->size() );
  call.Dump(FACWAR__CALL_DUMP);

  return nullptr;
}

PyResult FactionWarMgrService::Handle_GetSystemStatus(PyCallArgs &call) {
    /*
contestionStateNone = 0
contestionStateContested = 1
contestionStateVulnerable = 2
contestionStateCaptured = 3
*/
    /*
status = self.facWarMgr.GetSystemStatus(session.solarsystemid2, session.warfactionid)
systemStatus = sm.StartService('facwar').GetSystemStatus()
xtra = ''
if systemStatus == const.contestionStateCaptured:
    xtra = localization.GetByLabel('UI/Neocom/SystemLost')
    elif systemStatus == const.contestionStateVulnerable:
    xtra = localization.GetByLabel('UI/Neocom/Vulnerable')
    elif systemStatus == const.contestionStateContested:
    xtra = localization.GetByLabel('UI/Neocom/Contested')
    elif systemStatus == const.contestionStateNone and returnNone:
    xtra = localization.GetByLabel('UI/Neocom/Uncontested')
    return xtra
    */

    sLog.White( "FactionWarMgrService::Handle_GetSystemStatus()", "size=%u ", call.tuple->size() );
    call.Dump(FACWAR__CALL_DUMP);
    return (new PyInt(0));
}

//22:48:28 L FactionWarMgrService::Handle_IsEnemyFaction(): size= 2
PyResult FactionWarMgrService::Handle_IsEnemyFaction(PyCallArgs &call) {
    /*[00m05:39:09 [SvcCall] Service facWarMgr: calling IsEnemyFaction
     * 05:39:09[00m L [37;01mFactionWarMgrService::Handle_IsEnemyFaction(): [00msize=2
     * [00m05:39:09 [SvcCall]   Call Arguments:
     * 05:39:09 [SvcCall]       Tuple: 2 elements
     * 05:39:09 [SvcCall]         [ 0] Integer field: 500002
     * 05:39:09 [SvcCall]         [ 1] Integer field: 500001   <- this one changes
     */
  sLog.White( "FactionWarMgrService::Handle_IsEnemyFaction()", "size=%u ", call.tuple->size() );
  call.Dump(FACWAR__CALL_DUMP);

  return nullptr;
}

PyResult FactionWarMgrService::Handle_JoinFactionAsCharacter(PyCallArgs &call) {
  sLog.White( "FactionWarMgrService::Handle_JoinFactionAsCharacter()", "size=%u ", call.tuple->size() );
  call.Dump(FACWAR__CALL_DUMP);

  return nullptr;
}

PyResult FactionWarMgrService::Handle_(PyCallArgs &call) {
    //ret = self.facWarMgr.GetCorporationWarFactionID(corpID)
    sLog.White( "FactionWarMgrService::Handle_()", "size=%u ", call.tuple->size() );
    call.Dump(FACWAR__CALL_DUMP);

    return nullptr;
}

PyResult FactionWarMgrService::Handle_(IsEnemyCorporationPyCallArgs &call) {
    //return self.facWarMgr.IsEnemyCorporation(enemyID, factionID)
    sLog.White( "FactionWarMgrService::Handle_IsEnemyCorporation()", "size=%u ", call.tuple->size() );
    call.Dump(FACWAR__CALL_DUMP);

    return nullptr;
}

PyResult FactionWarMgrService::Handle_GetSystemsConqueredThisRun(PyCallArgs &call) {
    //return self.facWarMgr.GetSystemsConqueredThisRun()
    sLog.White( "FactionWarMgrService::Handle_GetSystemsConqueredThisRun()", "size=%u ", call.tuple->size() );
    call.Dump(FACWAR__CALL_DUMP);

    return nullptr;
}

PyResult FactionWarMgrService::Handle_GetFactionCorporations(PyCallArgs &call) {
    //return self.facWarMgr.GetFactionCorporations(factionID)
    sLog.White( "FactionWarMgrService::Handle_GetFactionCorporations()", "size=%u ", call.tuple->size() );
    call.Dump(FACWAR__CALL_DUMP);

    return nullptr;
}

PyResult FactionWarMgrService::Handle_JoinFactionAsCharacterRecommendationLetter(PyCallArgs &call) {
    //self.facWarMgr.JoinFactionAsCharacterRecommendationLetter, factionID, itemID)
    sLog.White( "FactionWarMgrService::Handle_JoinFactionAsCharacterRecommendationLetter()", "size=%u ", call.tuple->size() );
    call.Dump(FACWAR__CALL_DUMP);

    return nullptr;
}

PyResult FactionWarMgrService::Handle_JoinFactionAsAlliance(PyCallArgs &call) {
    //self.facWarMgr.JoinFactionAsAlliance(factionID)
    sLog.White( "FactionWarMgrService::Handle_JoinFactionAsAlliance()", "size=%u ", call.tuple->size() );
    call.Dump(FACWAR__CALL_DUMP);

    return nullptr;
}

PyResult FactionWarMgrService::Handle_JoinFactionAsCorporation(PyCallArgs &call) {
    //self.facWarMgr.JoinFactionAsCorporation(factionID)
    sLog.White( "FactionWarMgrService::Handle_JoinFactionAsCorporation()", "size=%u ", call.tuple->size() );
    call.Dump(FACWAR__CALL_DUMP);

    return nullptr;
}

PyResult FactionWarMgrService::Handle_GetStats_FactionInfo(PyCallArgs &call) {
    //return self.facWarMgr.GetStats_FactionInfo()
    sLog.White( "FactionWarMgrService::Handle_GetStats_FactionInfo()", "size=%u ", call.tuple->size() );
    call.Dump(FACWAR__CALL_DUMP);

    return nullptr;
}

PyResult FactionWarMgrService::Handle_GetStats_TopAndAllKillsAndVPs(PyCallArgs &call) {
    //self.topStats = self.facWarMgr.GetStats_TopAndAllKillsAndVPs()
    sLog.White( "FactionWarMgrService::Handle_GetStats_TopAndAllKillsAndVPs()", "size=%u ", call.tuple->size() );
    call.Dump(FACWAR__CALL_DUMP);

    return nullptr;
}

PyResult FactionWarMgrService::Handle_GetStats_Character(PyCallArgs &call) {
    //for k, v in self.facWarMgr.GetStats_Character().items():
    sLog.White( "FactionWarMgrService::Handle_GetStats_Character()", "size=%u ", call.tuple->size() );
    call.Dump(FACWAR__CALL_DUMP);

    return nullptr;
}

PyResult FactionWarMgrService::Handle_GetStats_Corp(PyCallArgs &call) {
    // for k, v in self.facWarMgr.GetStats_Corp().items():
    sLog.White( "FactionWarMgrService::Handle_GetStats_Corp()", "size=%u ", call.tuple->size() );
    call.Dump(FACWAR__CALL_DUMP);

    return nullptr;
}

PyResult FactionWarMgrService::Handle_GetStats_Alliance(PyCallArgs &call) {
    //for k, v in self.facWarMgr.GetStats_Alliance().items():
    sLog.White( "FactionWarMgrService::Handle_GetStats_Alliance()", "size=%u ", call.tuple->size() );
    call.Dump(FACWAR__CALL_DUMP);

    return nullptr;
}

PyResult FactionWarMgrService::Handle_GetStats_Militia(PyCallArgs &call) {
    //return self.facWarMgr.GetStats_Militia()
    sLog.White( "FactionWarMgrService::Handle_GetStats_Militia()", "size=%u ", call.tuple->size() );
    call.Dump(FACWAR__CALL_DUMP);

    return nullptr;
}

PyResult FactionWarMgrService::Handle_GetStats_CorpPilots(PyCallArgs &call) {
    //return self.facWarMgr.GetStats_CorpPilots()
    sLog.White( "FactionWarMgrService::Handle_GetStats_CorpPilots()", "size=%u ", call.tuple->size() );
    call.Dump(FACWAR__CALL_DUMP);

    return nullptr;
}

PyResult FactionWarMgrService::Handle_LeaveFactionAsAlliance(PyCallArgs &call) {
    //self.facWarMgr.LeaveFactionAsAlliance(factionID)
    sLog.White( "FactionWarMgrService::Handle_LeaveFactionAsAlliance()", "size=%u ", call.tuple->size() );
    call.Dump(FACWAR__CALL_DUMP);

    return nullptr;
}

PyResult FactionWarMgrService::Handle_LeaveFactionAsCorporation(PyCallArgs &call) {
    //self.facWarMgr.LeaveFactionAsCorporation(factionID)
    sLog.White( "FactionWarMgrService::Handle_()", "size=%u ", call.tuple->size() );
    call.Dump(FACWAR__CALL_DUMP);

    return nullptr;
}

PyResult FactionWarMgrService::Handle_WithdrawJoinFactionAsAlliance(PyCallArgs &call) {
    //self.facWarMgr.WithdrawJoinFactionAsAlliance(factionID)
    sLog.White( "FactionWarMgrService::Handle_()", "size=%u ", call.tuple->size() );
    call.Dump(FACWAR__CALL_DUMP);

    return nullptr;
}

PyResult FactionWarMgrService::Handle_WithdrawJoinFactionAsCorporation(PyCallArgs &call) {
    //self.facWarMgr.WithdrawJoinFactionAsCorporation(factionID)
    sLog.White( "FactionWarMgrService::Handle_()", "size=%u ", call.tuple->size() );
    call.Dump(FACWAR__CALL_DUMP);

    return nullptr;
}

PyResult FactionWarMgrService::Handle_WithdrawLeaveFactionAsAlliance(PyCallArgs &call) {
    //self.facWarMgr.WithdrawLeaveFactionAsAlliance(factionID)
    sLog.White( "FactionWarMgrService::Handle_()", "size=%u ", call.tuple->size() );
    call.Dump(FACWAR__CALL_DUMP);

    return nullptr;
}

PyResult FactionWarMgrService::Handle_WithdrawLeaveFactionAsCorporation(PyCallArgs &call) {
    //self.facWarMgr.WithdrawLeaveFactionAsCorporation(factionID)
    sLog.White( "FactionWarMgrService::Handle_WithdrawLeaveFactionAsCorporation()", "size=%u ", call.tuple->size() );
    call.Dump(FACWAR__CALL_DUMP);

    return nullptr;
}

PyResult FactionWarMgrService::Handle_RefreshCorps(PyCallArgs &call) {
    //return self.facWarMgr.RefreshCorps()
    sLog.White( "FactionWarMgrService::Handle_RefreshCorps()", "size=%u ", call.tuple->size() );
    call.Dump(FACWAR__CALL_DUMP);

    return nullptr;
}

