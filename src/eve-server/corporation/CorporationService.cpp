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

#include "PyServiceCD.h"
#include "StaticDataMgr.h"
#include "account/AccountService.h"
#include "corporation/CorporationService.h"

PyCallable_Make_InnerDispatcher(CorporationService)

CorporationService::CorporationService(PyServiceMgr *mgr)
: PyService(mgr, "corporationSvc"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(CorporationService, GetFactionInfo);
    PyCallable_REG_CALL(CorporationService, GetCorpInfo);
    PyCallable_REG_CALL(CorporationService, GetNPCDivisions);
    PyCallable_REG_CALL(CorporationService, GetEmploymentRecord);
    PyCallable_REG_CALL(CorporationService, GetMedalsReceived);
    PyCallable_REG_CALL(CorporationService, GetMedalDetails);
    PyCallable_REG_CALL(CorporationService, GetAllCorpMedals);
    PyCallable_REG_CALL(CorporationService, GetRecruitmentAdsByCriteria);
    PyCallable_REG_CALL(CorporationService, GetRecruitmentAdRegistryData);
    PyCallable_REG_CALL(CorporationService, GetRecruitmentAdsForCorporation);
    PyCallable_REG_CALL(CorporationService, IsEnemyFaction);
    PyCallable_REG_CALL(CorporationService, GetVoteCasesByCorporation);
    PyCallable_REG_CALL(CorporationService, CreateMedal);
}

CorporationService::~CorporationService() {
    delete m_dispatch;
}

/*
 * CORP__ERROR
 * CORP__WARNING
 * CORP__INFO
 * CORP__MESSAGE
 * CORP__TRACE
 * CORP__CALL
 * CORP__CALL_DUMP
 * CORP__RSP_DUMP
 * CORP__DB_ERROR
 * CORP__DB_WARNING
 * CORP__DB_INFO
 * CORP__DB_MESSAGE
 */

PyResult CorporationService::Handle_GetNPCDivisions(PyCallArgs &call)
{
    return sDataMgr.GetNPCDivisions();
}

PyResult CorporationService::Handle_GetVoteCasesByCorporation(PyCallArgs &call) {
    sLog.White( "CorporationService", "Handle_GetVoteCasesByCorporation() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    // table crpVoteItems
    return m_db.GetVoteItems(0);
}

PyResult CorporationService::Handle_GetEmploymentRecord(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    return m_db.GetEmploymentRecord(args.arg);
}

PyResult CorporationService::Handle_GetFactionInfo(PyCallArgs &call) {
    /*self.factionIDbyNPCCorpID, self.factionRegions, self.factionConstellations, self.factionSolarSystems,
     * self.factionRaces, self.factionStationCount, self.factionSolarSystemCount, self.npcCorpInfo = sm.RemoteSvc('corporationSvc').GetFactionInfo()
     *        for corpID, factionID in self.factionIDbyNPCCorpID.iteritems():
     *            if factionID not in self.corpsByFactionID:
     *                self.corpsByFactionID[factionID] = []
     *            if corpID not in self.corpsByFactionID[factionID]:
     *                self.corpsByFactionID[factionID].append(corpID)
     *
     *        owners = {}
     *        for k, v in self.factionIDbyNPCCorpID.iteritems():
     *            owners[k] = 0
     *            owners[v] = 0
     */

    return sDataMgr.GetFactionInfo();
}

PyResult CorporationService::Handle_GetCorpInfo(PyCallArgs &call)
{
    //corpmktinfo = sm.RemoteSvc('corporationSvc').GetCorpInfo(itemID)
    // this wants corp market info
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    return m_db.GetMktInfo(args.arg);
}

PyResult CorporationService::Handle_GetRecruitmentAdRegistryData( PyCallArgs& call )
{   // working
    sLog.White( "CorporationService", "Handle_GetRecruitmentAdRegistryData() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    PyDict* dict = new PyDict();
        dict->SetItemString("types", m_db.GetAdTypeData());
        dict->SetItemString("groups", m_db.GetAdGroupData());
    PyObject* args = new PyObject("util.KeyVal", dict);
    if (is_log_enabled(CORP__RSP_DUMP))
        args->Dump(CORP__RSP_DUMP, "");
    return args;
}

PyResult CorporationService::Handle_GetRecruitmentAdsByCriteria( PyCallArgs& call )
{    //   return sm.RemoteSvc('corporationSvc').GetRecruitmentAdsByCriteria(typeMask, isInAlliance, minMembers, maxMembers)
    sLog.White( "CorporationService", "Handle_GetRecruitmentAdsByCriteria() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_GetRecruitmentAdsByCriteria args;
    if ( !args.Decode( &call.tuple ) )   {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    return m_db.GetAdRegistryData(args.typeMask, args.inAlliance, args.minMembers, args.maxMembers);
}

PyResult CorporationService::Handle_GetRecruitmentAdsForCorporation( PyCallArgs& call )
{
    // recruitments = self.GetCorpRegistry().GetRecruitmentAdsForCorporation()
    sLog.White( "CorporationService", "Handle_GetRecruitmentAdsForCorporation() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return m_db.GetAdRegistryData();
}


/**     ***********************************************************************
 * @note   these below are not coded or partially coded
 */

PyResult CorporationService::Handle_GetMedalsReceived(PyCallArgs &call) {
    sLog.White( "CorporationService", "Handle_GetMedalsReceived() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);
    Call_SingleIntegerArg arg;

    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }
    // dont know the details for this return yet.....
    PyTuple *t = new PyTuple(2);
    t->items[0] = m_db.GetMedalsReceived(arg.arg);
    t->items[1] = new PyList();
    return t;
}

PyResult CorporationService::Handle_GetMedalDetails(PyCallArgs &call) {
    sLog.White( "CorporationService", "Handle_GetMedalDetails() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);
    Call_SingleIntegerArg arg;

    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }
    // dont know the details for this return yet.....
    PyTuple *t = new PyTuple(2);
    t->items[0] = m_db.GetMedalDetails(arg.arg);
    t->items[1] = new PyList();
    return t;
}

PyResult CorporationService::Handle_GetAllCorpMedals( PyCallArgs& call )
{
    // medals, medalDetails = sm.RemoteSvc('corporationSvc').GetAllCorpMedals(corpID)
    sLog.White( "CorporationService", "Handle_GetAllCorpMedals() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);
    Call_SingleIntegerArg arg;
    if ( !arg.Decode( &call.tuple ) )
    {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    PyList* res = new PyList();

    util_Rowset rs;

    rs.header.push_back( "medalID" );
    rs.header.push_back( "ownerID" );
    rs.header.push_back( "title" );
    rs.header.push_back( "description" );
    rs.header.push_back( "creatorID" );
    rs.header.push_back( "date" );
    rs.header.push_back( "noRecepients" );
    res->AddItem( rs.Encode() );

    rs.header.clear();

    rs.header.push_back( "medalID" );
    rs.header.push_back( "part" );
    rs.header.push_back( "layer" );
    rs.header.push_back( "graphic" );
    rs.header.push_back( "color" );
    res->AddItem( rs.Encode() );

    return res;
}

/** not handled */
PyResult CorporationService::Handle_IsEnemyFaction(PyCallArgs &call)
{
    sLog.White( "CorporationService", "Handle_IsEnemyFaction() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}
PyResult CorporationService::Handle_CreateMedal(PyCallArgs &call)
{
    sLog.White( "CorporationService", "Handle_CreateMedal() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);


    //AccountService::TranserFunds(Journal::EntryType::MedalCreation);

    return nullptr;
}
