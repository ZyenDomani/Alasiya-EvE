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

#include "PyServiceCD.h"
#include "corporation/CorpMgrService.h"

PyCallable_Make_InnerDispatcher(CorpMgrService)

CorpMgrService::CorpMgrService(PyServiceMgr *mgr)
: PyService(mgr, "corpmgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(CorpMgrService, GetPublicInfo);
    PyCallable_REG_CALL(CorpMgrService, GetCorporations);
    PyCallable_REG_CALL(CorpMgrService, GetAssetInventory);
    PyCallable_REG_CALL(CorpMgrService, GetCorporationStations);
    PyCallable_REG_CALL(CorpMgrService, GetCorporationIDForCharacter);
    PyCallable_REG_CALL(CorpMgrService, GetAssetInventoryForLocation);
    PyCallable_REG_CALL(CorpMgrService, AuditMember);
}

CorpMgrService::~CorpMgrService() {
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


PyResult CorpMgrService::Handle_GetPublicInfo(PyCallArgs &call) {
    sLog.White("CorpMgrService", "Handle_GetPublicInfo() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    return m_db.GetCorpInfo(arg.arg);
}

PyResult CorpMgrService::Handle_GetCorporations(PyCallArgs &call) {
    sLog.White("CorpMgrService", "Handle_GetPublicInfo() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

  Call_SingleIntegerArg arg;
  if (!arg.Decode(&call.tuple)) {
      codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
      return nullptr;
  }

  return m_db.GetCorporations(arg.arg);
}

//  started...still needs work
PyResult CorpMgrService::Handle_GetAssetInventory(PyCallArgs &call) {
    // rows = sm.RemoteSvc('corpmgr').GetAssetInventory(eve.session.corpid, which)

    sLog.White( "CorpMgrService::Handle_GetAssetInventory()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_GetAssetInventory args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    uint8 flag = flagNone;
    if (args.flag.compare("offices") == 0)
        flag = flagOffice;
    else if (args.flag.compare("junk") == 0)
        flag = flagImpounded;
    else if (args.flag.compare("property") == 0)    // this is 'inSpace' tab...LSC, POS, etc
        flag = flagProperty;
    else if (args.flag.compare("deliveries") == 0)
        flag = flagCorpMarket;
    else
        _log(CORP__ERROR, "CorpMgrService::Handle_GetAssetInventory: flag is %s", args.flag.c_str());

    //  returns a CRowSet
    return m_db.GetAssetInventory(args.corpID, flag);
}

PyResult CorpMgrService::Handle_GetAssetInventoryForLocation(PyCallArgs &call) {
    //  items = sm.RemoteSvc('corpmgr').GetAssetInventoryForLocation(eve.session.corpid, stationID, which)
    sLog.White( "CorpMgrService::Handle_GetAssetInventoryForLocation()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_GetAssetInventoryForLocation args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    uint8 flag = flagNone;
    if (args.flag.compare("offices") == 0)
        flag = flagOffice;
    else if (args.flag.compare("junk") == 0)
        flag = flagImpounded;
    else if (args.flag.compare("property") == 0)    // this is 'inSpace' tab...LSC, POS, etc
        flag = flagProperty;
    else if (args.flag.compare("deliveries") == 0)
        flag = flagCorpMarket;
    else
        _log(CORP__ERROR, "CorpMgrService::Handle_GetAssetInventoryForLocation: flag is %s", args.flag.c_str());

    //  returns a CRowSet
    //  this can be called on system/station/office that isnt loaded, so must hit db for info.
    return m_db.GetAssetInventoryForLocation(args.corpID, args.locationID, flag);
}

PyResult CorpMgrService::Handle_GetCorporationStations(PyCallArgs &call) {
  /**           this is called from trademgr.py
        stations = sm.RemoteSvc('corpmgr').GetCorporationStations()
        for station in stations:
            if station.itemID in self.shell.GetStationIDs():
                continue
            stationListing.append([localization.GetByLabel('UI/PVPTrade/StationInSolarsystem', station=station.itemID, solarsystem=station.locationID), station.itemID, station.typeID])
*/

  sLog.White( "CorpMgrService::Handle_GetCorporationStations()", "size= %u", call.tuple->size() );
  call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpMgrService::Handle_GetCorporationIDForCharacter(PyCallArgs &call) {
/**        returns corpID for given charID  */

  sLog.White( "CorpMgrService::Handle_GetCorporationIDForCharacter()", "size= %u", call.tuple->size() );
  call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpMgrService::Handle_AuditMember(PyCallArgs &call) {
    /**
     * logItemEventRows, crpRoleHistroyRows = sm.RemoteSvc('corpmgr').AuditMember(memberID, fromDate, toDate, rowsPerPage)
     */

    sLog.White( "CorpMgrService::Handle_AuditMember()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_AuditMember args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, m_db.GetItemEvents(call.client->GetCorporationID(), args.charID, args.fromDate, args.toDate, args.rowsPerPage));
        tuple->SetItem(1, m_db.GetRoleHistroy(call.client->GetCorporationID(), args.charID, args.fromDate, args.toDate, args.rowsPerPage));

    if (is_log_enabled(CORP__RSP_DUMP))
        tuple->Dump(CORP__RSP_DUMP, "    ");

    return tuple;
}

