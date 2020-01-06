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
    Author:        Zhur, Aknor Jaden
    Rewrite:    Allan
*/

/** @todo  this class is db-heavy.
 * all bm returns use python db structures
 * need to create mem objects in character.cpp for all bm/folder data
 * then create packet structure for sending data to client to avoid db hits
 */

#include "eve-server.h"

#include "PyServiceCD.h"
#include "PyBoundObject.h"
#include "packets/Bookmarks.h"
#include "system/BookmarkService.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/ManagerDB.h"

/*
BOOKMARK__ERROR
BOOKMARK__WARNING
BOOKMARK__MESSAGE
BOOKMARK__DEBUG
BOOKMARK__INFO
BOOKMARK__TRACE
BOOKMARK__CALL_DUMP
BOOKMARK__RSP_DUMP
*/


PyCallable_Make_InnerDispatcher(BookmarkService)

BookmarkService::BookmarkService(PyServiceMgr *mgr)
: PyService(mgr, "bookmark"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(BookmarkService, GetBookmarks);
    PyCallable_REG_CALL(BookmarkService, CreateFolder);
    PyCallable_REG_CALL(BookmarkService, UpdateFolder);
    PyCallable_REG_CALL(BookmarkService, DeleteFolder);
    PyCallable_REG_CALL(BookmarkService, BookmarkLocation);
    PyCallable_REG_CALL(BookmarkService, BookmarkScanResult);
    PyCallable_REG_CALL(BookmarkService, DeleteBookmarks);
    PyCallable_REG_CALL(BookmarkService, MoveBookmarksToFolder);
    /*  NOT WORKING YET  */
    PyCallable_REG_CALL(BookmarkService, CopyBookmarks);
    PyCallable_REG_CALL(BookmarkService, AddBookmarkFromVoucher);

}

BookmarkService::~BookmarkService() {
    delete m_dispatch;
}

bool BookmarkService::LookupBookmark(uint32 bookmarkID, uint32& itemID, uint32& typeID, uint32& locationID, double& x, double& y, double& z) {
    return m_db.GetBookmarkInformation(bookmarkID, itemID, typeID, locationID, x, y, z);
}

PyResult BookmarkService::Handle_GetBookmarks(PyCallArgs &call) {
    PyTuple* result = new PyTuple(2);
        result->SetItem(0, m_db.GetBookmarks(call.client->GetCharacterID()));
        result->SetItem(1, m_db.GetFolders(call.client->GetCharacterID()));
    result->Dump(BOOKMARK__RSP_DUMP, "    ");
    return result;
}

PyResult BookmarkService::Handle_CreateFolder(PyCallArgs &call) {
    std::string name = call.tuple->GetItem( 0 )->AsWString()->content();
    uint32 ownerID = call.client->GetCharacterID();
    Rsp_CreateFolder result;
        result.ownerID = ownerID;
        result.folderID = m_db.SaveNewFolderToDatabase(name, ownerID);
        result.folderName = name;
        result.creatorID = ownerID;

    result.Dump(BOOKMARK__RSP_DUMP, "    ");
    return result.Encode();
}

PyResult BookmarkService::Handle_UpdateFolder(PyCallArgs &call) {
    Call_UpdateFolder args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return new PyBool(false);
    }
    args.Dump(BOOKMARK__CALL_DUMP, "    ");

    if (!m_db.UpdateFolderInDatabase(args.folderID, PyRep::StringContent(args.folderName)))
        return new PyBool(false); // make client error here to let them know updating failed

    return new PyBool(true);
}

PyResult BookmarkService::Handle_DeleteFolder(PyCallArgs &call) {
    // this also deletes ALL bookmarks in the folder to be deleted.
    //   this returns bm data for bookmarks deleted with this folder, if any

    call.Dump(BOOKMARK__CALL_DUMP);

    uint32 folderID = PyRep::IntegerValue(call.tuple->GetItem( 0 ));

    if (!m_db.DeleteFolderFromDatabase(folderID))
        return nullptr;   // make client error here to let them know deletion failed

    // call db to get list of bmIDs in deleted folder.  return result with this data
    std::vector< int32 > bmIDs;
    bmIDs.clear();
    m_db.GetBookmarkByFolderID(folderID, bmIDs);
    m_db.DeleteBookmarksFromDatabase(&bmIDs);

    if (bmIDs.size() < 1) {
        Rsp_BMData data;
            data.bookmarkID = 0;
            data.ownerID = 0;
            data.memo = "";
            data.comment = "";
            data.folderID = folderID;
        data.Dump(BOOKMARK__RSP_DUMP, "    ");
        return data.Encode();
    }

    PyTuple* result = new PyTuple(bmIDs.size());
    for (size_t i = 0; i < bmIDs.size(); ++i) {
        Rsp_BMData data;
            data.bookmarkID = bmIDs.at(i);
            data.ownerID = 0;
            data.memo = "";
            data.comment = "";
            data.folderID = folderID;
        result->SetItem(i, data.Encode());
    }
    result->Dump(BOOKMARK__RSP_DUMP, "    ");
    //return result;
    /** @todo this needs more work */
    return nullptr;
}

PyResult BookmarkService::Handle_BookmarkLocation(PyCallArgs &call) {
  /*  bookmarkID, itemID, typeID, x, y, z, locationID = sm.RemoteSvc('bookmark').BookmarkLocation(itemID, ownerID, memo, comment, folderID)  */
    Call_BookmarkLocation args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    args.Dump(BOOKMARK__CALL_DUMP, "    ");
    uint32 itemID = 0, typeID = 0, locationID = 0, folderID = 0;
    GPoint point(NULL_ORIGIN);

	// Check for presence of folderID in the packet
	if (call.byname.find("folderID") != call.byname.cend())
        folderID = PyRep::IntegerValue(call.byname.find("folderID")->second);

    if (IsPlayerItem(args.itemID)) {      // entity #'s above 140m are player-owned.  player is in ship
        typeID = EVEDB::invTypes::SolarSystem;
        point = call.client->GetShipSE()->GetPosition();       // Get x,y,z location.  bm type is coordinate as "spot in xxx system"
        locationID = call.client->GetLocationID();       // locationID of bm is current sol system
        itemID = locationID;      //  itemID = locationID for coord bm.  shows jumps, s/c/r in bm window, green in system
    } else if (IsStation(args.itemID)) {  // not player-owned, check for station.
        SystemEntity* pSE = call.client->SystemMgr()->GetSE(args.itemID);
        if (pSE == nullptr) {
            // send player error also
            return nullptr;
        }
        typeID = pSE->GetTypeID();
        itemID =  args.itemID;  // this is stationID
        locationID = call.client->GetSystemID();       // get sol system of current station
    } else {      // char is passing systemID from map.  char is marking a solar systemID for bm
        if (IsRegion(args.itemID))
            typeID = EVEDB::invTypes::Region;
        else if (IsConstellation(args.itemID))
            typeID = EVEDB::invTypes::Constellation;
        else if (IsSolarSystem(args.itemID))
            typeID = EVEDB::invTypes::SolarSystem;
        locationID = args.itemID;  // this is systemID from map
        itemID = locationID;      //  itemID = locationID for coord bm.  shows jumps, s/c/r in bm window, green in system
    }

    uint32 bookmarkID = m_db.SaveNewBookmarkToDatabase(args.ownerID, itemID, typeID, PyRep::StringContent(args.memo), point, locationID, \
                                                        PyRep::StringContent(args.comment), call.client->GetCharacterID(), folderID );

    // (bookmarkID, itemID, typeID, x, y, z, locationID)
    Rsp_BookmarkLocation result;
        result.bookmarkID  = bookmarkID;
        result.itemID      = (typeID == EVEDB::invTypes::SolarSystem ? 0 : itemID);     // itemID = 0 when typeID is SolarSystem
        result.typeID      = typeID;
        result.x           = point.x;
        result.y           = point.y;
        result.z           = point.z;
        result.locationID  = locationID;
    result.Dump(BOOKMARK__RSP_DUMP, "    ");
    return result.Encode();
}

PyResult BookmarkService::Handle_BookmarkScanResult(PyCallArgs &call)
{
    //  bookmarkID, itemID, typeID, x, y, z, locationID = self.bookmarkMgr.BookmarkScanResult(locationID, memo, comment, resultID, ownerID, folderID=0)

    Call_BookmarkScanResult args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    args.Dump(BOOKMARK__CALL_DUMP, "    ");
    /*
    * 22:25:58 [SvcCallDump]       Tuple: 5 elements
    * 22:25:58 [SvcCallDump]         [ 0] Integer field: 30000053
    * 22:25:58 [SvcCallDump]         [ 1] String: 'test dungeon 2 '
    * 22:25:58 [SvcCallDump]         [ 2] WString: 'dungeon notes here'
    * 22:25:58 [SvcCallDump]         [ 3] String: 'XIG-040'
    * 22:25:58 [SvcCallDump]         [ 4] Integer field: 140000000
    */
    uint32 typeID = EVEDB::invTypes::SolarSystem, folderID = 0;

    // Check for presence of folderID in the packet
    if (call.byname.find("folderID") != call.byname.cend())
        folderID = PyRep::IntegerValue(call.byname.find("folderID")->second);

    GPoint point(ManagerDB::GetAnomalyPos(args.scanID));

    uint32 bookmarkID = m_db.SaveNewBookmarkToDatabase(args.ownerID, args.locationID, typeID, PyRep::StringContent(args.memo), point, args.locationID,\
                                                        PyRep::StringContent(args.comment), call.client->GetCharacterID(), folderID );

    // (bookmarkID, itemID, typeID, x, y, z, locationID)
    Rsp_BookmarkLocation result;
        result.bookmarkID  = bookmarkID;
        result.itemID      = 0;             // scan bm returns none here
        result.typeID      = typeID;
        result.x           = point.x;
        result.y           = point.y;
        result.z           = point.z;
        result.locationID  = args.locationID;
    result.Dump(BOOKMARK__RSP_DUMP, "    ");
    return result.Encode();
}

PyResult BookmarkService::Handle_DeleteBookmarks(PyCallArgs &call) {
    call.Dump(BOOKMARK__CALL_DUMP);
    Call_DeleteBookmarks args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    //args.Dump(BOOKMARK__CALL_DUMP, "    ");

    if (args.object->IsNone())
        return PyStatic.NewNone();
    PyList* bmList = args.object->header()->AsTuple()->GetItem(1)->AsTuple()->GetItem(0)->AsList();

    std::vector<int32> bmIDs;
    for (size_t i = 0; i < bmList->size(); i++)
        bmIDs.push_back(bmList->GetItem(i)->AsInt()->value());

    m_db.DeleteBookmarksFromDatabase(&bmIDs);
    return PyStatic.NewNone();
}


PyResult BookmarkService::Handle_MoveBookmarksToFolder(PyCallArgs &call) {
    // rows = bookmarkMgr.MoveBookmarksToFolder(folderID, bookmarkIDs)
    //call.Dump(BOOKMARK__CALL_DUMP);
    Call_MoveBookmarksToFolder args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    args.Dump(BOOKMARK__CALL_DUMP, "    ");

    if (args.object->IsNone())
        return PyStatic.NewNone();

    PyList* bmList = args.object->header()->AsTuple()->GetItem(1)->AsTuple()->GetItem(0)->AsList();

    std::vector<int32> bmIDs;
    for (size_t i = 0; i < bmList->size(); i++)
        bmIDs.push_back(bmList->GetItem(i)->AsInt()->value());

    m_db.MoveBookmarkToFolder(args.folderID, &bmIDs);

    return m_db.GetBMData(args.folderID);
}

PyResult BookmarkService::Handle_CopyBookmarks(PyCallArgs &call) {
    //newBookmarks, message = bookmarkMgr.CopyBookmarks(bookmarksToCopy, folderID)

  //{'FullPath': u'UI/Messages', 'messageID': 258505, 'label': u'CantTradeMissionBookmarksBody'}(u'You cannot trade or copy mission bookmarks.', None, None)

    sLog.Error( "BookmarkService::Handle_CopyBookmarks()", "Service is not handled yet.  Returning NULL.");
    call.Dump(BOOKMARK__CALL_DUMP);

    return PyStatic.NewNone();
}

PyResult BookmarkService::Handle_AddBookmarkFromVoucher(PyCallArgs &call) {
  /**
	 sm.RemoteSvc('bookmark').AddBookmarkFromVoucher, itemID, ownerID, folderID, violateSafetyTimer=True)
            */

    sLog.Error( "BookmarkService::Handle_AddBookmarkFromVoucher()", "Service is not handled yet.  Returning NULL.");
    call.client->SendInfoModalMsg("Creating Bookmarks from Vouchers is currently broken.");
    call.Dump(BOOKMARK__CALL_DUMP);

    return PyStatic.NewNone();
}

