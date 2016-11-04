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
    Updates:    Allan
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "PyBoundObject.h"
#include "packets/Bookmarks.h"
#include "system/BookmarkService.h"
#include "system/SystemManager.h"


PyCallable_Make_InnerDispatcher(BookmarkService)

BookmarkService::BookmarkService(PyServiceMgr *mgr)
: PyService(mgr, "bookmark"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(BookmarkService, GetBookmarks);
    PyCallable_REG_CALL(BookmarkService, BookmarkLocation);
    PyCallable_REG_CALL(BookmarkService, UpdateBookmark);
    PyCallable_REG_CALL(BookmarkService, CreateFolder);
    PyCallable_REG_CALL(BookmarkService, UpdateFolder);
    PyCallable_REG_CALL(BookmarkService, DeleteFolder);

    /*  NOT WORKING YET  */
    PyCallable_REG_CALL(BookmarkService, DeleteBookmarks);
    PyCallable_REG_CALL(BookmarkService, MoveBookmarksToFolder);
    PyCallable_REG_CALL(BookmarkService, CopyBookmarks);
    PyCallable_REG_CALL(BookmarkService, AddBookmarkFromVoucher);
    PyCallable_REG_CALL(BookmarkService, BookmarkScanResult);

}

BookmarkService::~BookmarkService() {
    delete m_dispatch;
}

bool BookmarkService::LookupBookmark(uint32 bookmarkID, uint32& itemID, uint32& typeID, uint32 locationID, double& x, double& y, double& z) {
    return m_db.GetBookmarkInformation(bookmarkID, itemID, typeID, locationID, x, y, z);
}

PyResult BookmarkService::Handle_GetBookmarks(PyCallArgs &call) {
    PyTuple* result = new PyTuple(2);
        result->SetItem(0, m_db.GetBookmarks(call.client->GetCharacterID()));
        result->SetItem(1, m_db.GetFolders(call.client->GetCharacterID()));
    return result;
}

PyResult BookmarkService::Handle_BookmarkScanResult(PyCallArgs &call) {
    /* bookmarkID, itemID, typeID, x, y, z, locationID = self.bookmarkMgr.BookmarkScanResult(locationID, memo, comment, resultID, ownerID, folderID=folderID)
     */
    /* 22:24:39 [SvcCall] Service bookmark: calling BookmarkScanResult
     * 22:25:58 L BookmarkService: Handle_BookmarkScanResult() size=5
     * 22:25:58 [SvcCallDump]   Call Arguments:
     * 22:25:58 [SvcCallDump]       Tuple: 5 elements
     * 22:25:58 [SvcCallDump]         [ 0] Integer field: 30000053
     * 22:25:58 [SvcCallDump]         [ 1] String: 'test dungeon 2 '
     * 22:25:58 [SvcCallDump]         [ 2] WString: 'dungeon notes here'
     * 22:25:58 [SvcCallDump]         [ 3] String: 'XIG-040'
     * 22:25:58 [SvcCallDump]         [ 4] Integer field: 140000000
     * 22:25:58 [SvcCallDump]   Call Named Arguments:
     * 22:25:58 [SvcCallDump]     Argument 'folderID':
     * 22:25:58 [SvcCallDump]         (None)
     * 22:25:58 [SvcCallDump]     Argument 'machoVersion':
     * 22:25:58 [SvcCallDump]         Integer field: 1
     */
    sLog.Log("BookmarkService", "Handle_BookmarkScanResult() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
    Call_BookmarkScanResult args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode args");
        return nullptr;
    }

    // (bookmarkID, itemID, typeID, x, y, z, locationID)
    Rsp_BookmarkLocation ret;
    ret.bookmarkID  = 0;
    ret.itemID      = new PyNone();     // unsure if/when this is populated
    ret.typeID      = 0;
    ret.x           = 0;
    ret.y           = 0;
    ret.z           = 0;
    ret.locationID  = 0;

    return ret.Encode();
}

PyResult BookmarkService::Handle_BookmarkLocation(PyCallArgs &call) {
  /*  bookmarkID, itemID, typeID, x, y, z, locationID = sm.RemoteSvc('bookmark').BookmarkLocation(itemID, ownerID, memo, comment, folderID)  */
    Call_BookmarkLocation args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode args");
        return nullptr;
    }

    uint32 itemID = 0, typeID = 0, locationID = 0, folderID = 0;
    GPoint point(NULL_ORIGIN);

	/**  will need more research when trade is implemented for sharing bm's between chars....corp/friend/etc.
	 *  this should be the original characterID that made the bm.
	 *  mission bm's are created by the agent giving the mission  -allan 27Jul14
     *     if hasattr(bookmark, 'locationType') and bookmark.locationType in ('agenthomebase', 'objective'):
	 *
     *  checkIfAgentBookmark = bookmark and getattr(bookmark, 'agentID', 0) and hasattr(bookmark, 'locationNumber')
     *  checkIfReadonlyBookmark = bookmark and type(getattr(bookmark, 'bookmarkID', 0)) == types.TupleType
     *  checkBookmarkDeadspace = bool(getattr(bookmark, 'deadspace', 0))
	 *
            if getattr(bookmark, 'agentID', 0) and hasattr(bookmark, 'locationNumber'):
                referringAgentID = getattr(bookmark, 'referringAgentID', None)
                sm.StartService('agents').GetAgentMoniker(bookmark.agentID).WarpToLocation(bookmark.locationType, bookmark.locationNumber, warpRange, fleet, referringAgentID)

	 */

	// Check for presence of folderID in the packet
	if (call.byname.find("folderID") != call.byname.cend()) {
        if ( !(call.byname.find("folderID")->second->IsNone()) ) {
            folderID = call.byname.find("folderID")->second->AsInt()->value();
        }
    }

    SystemEntity* pSE = call.client->SystemMgr()->GetSE(args.itemID);
    if (!pSE) {
        // make error here
        return nullptr;
    }
    typeID = pSE->GetGroupID();    // get bm typeID

    if (IsNotStaticItem(args.itemID)) {      // entity #'s above 140m are player-owned
        point = call.client->GetShipSE()->GetPosition();       // Get x,y,z location.  bm type is coordinate as "spot in xxx system"
        locationID = call.client->GetLocationID();       // locationID of bm is current sol system
        itemID = locationID;      //  locationID = itemID for coord bm.  shows jumps, s/c/r in bm window, green in system
    } else if (IsStation(args.itemID)) {  // not player-owned, check for station.
        itemID =  args.itemID;  // this is stationID
        locationID = call.client->GetSystemID();       // get sol system of current station
    } else {      // char is passing systemID from map.  char is marking a solar systemID for bm
        locationID = args.itemID;  // this is systemID from map
        itemID = locationID;      //  locationID = itemID for coord bm.  shows jumps, s/c/r in bm window, green in system
    }

    uint32 bookmarkID = m_db.SaveNewBookmarkToDatabase(args.ownerID, itemID, typeID, args.memo, point, locationID, args.comment, call.client->GetCharacterID(), folderID );

    // (bookmarkID, itemID, typeID, x, y, z, locationID)
    Rsp_BookmarkLocation ret;
        ret.bookmarkID  = bookmarkID;
        ret.itemID      = new PyNone();     // unsure if/when this is populated
        ret.typeID      = typeID;
        ret.x           = point.x;
        ret.y           = point.y;
        ret.z           = point.z;
        ret.locationID  = locationID;

    return ret.Encode();
}

PyResult BookmarkService::Handle_UpdateBookmark(PyCallArgs &call)
{
    Call_UpdateBookmark args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode args");
        return nullptr;
    }

    if (!m_db.UpdateBookmarkInDatabase(args.bookmarkID, args.ownerID, args.memo, args.note, args.folderID))
        ;   // make client error here to let them know updating failed

    return new PyNone();
}

PyResult BookmarkService::Handle_CreateFolder(PyCallArgs &call) {
    return new PyInt(m_db.SaveNewFolderToDatabase(call.tuple->GetItem( 0 )->AsWString()->content(), call.client->GetCharacterID()));
}

PyResult BookmarkService::Handle_UpdateFolder(PyCallArgs &call) {
    Call_UpdateFolder args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode args");
        return nullptr;
    }

    if (!m_db.UpdateFolderInDatabase(args.ownerID, args.folderID, args.name))
        ; // make client error here to let them know updating failed

    return new PyNone();
}

PyResult BookmarkService::Handle_DeleteFolder(PyCallArgs &call) {
    if (!m_db.DeleteFolderFromDatabase(call.tuple->GetItem( 0 )->AsInt()->value()))
        ;   // make client error here to let them know deltion failed
    return new PyNone();
}

PyResult BookmarkService::Handle_DeleteBookmarks(PyCallArgs &call)          //not working
{
    call.client->SendInfoModalMsg("Deleting Bookmarks is currently broken.");
    call.Dump(SERVICE__CALL_DUMP);

    if (call.tuple->IsList()) {
      sLog.Log( "BookmarkService::Handle_DeleteBookmarks()", "Call is PyList");
      PyList *list = call.tuple->GetItem( 0 )->AsList();
      uint32 bookmarkID;
      std::vector<uint32> bookmarkIDs;

      if (list->size()) {
          for (int8 i=0; i<(list->size()); i++) {
              bookmarkID = call.tuple->GetItem( 0 )->AsList()->GetItem(i)->AsInt()->value();
              bookmarkIDs.push_back( bookmarkID );
          }
          m_db.DeleteBookmarksFromDatabase( call.client->GetCharacterID(),&bookmarkIDs );
      } else {
          sLog.Error( "BookmarkService::Handle_DeleteBookmarks()", "%s: call.tuple->GetItem( 0 )->AsList()->size() == 0.  Expected size >= 1.", call.client->GetName() );
      }
    } else if (call.tuple->IsObjectEx()) {
        //call.tuple->GetItem( 0 )->AsObjectEx();
      sLog.Error( "BookmarkService::Handle_DeleteBookmarks()", "Call is ObjectEx.");
    } else if (call.tuple->IsTuple()) {
      sLog.Log( "BookmarkService::Handle_DeleteBookmarks()", "Call is PyTuple");
      //uint32 bookmarkID;
      //bookmarkID = call.tuple->GetItem( 0 )->AsObjectEx()->GetItem( 1 )->AsList();
      //m_db.DeleteBookmarkFromDatabase( call.client->GetCharacterID(), bookmarkID );
    } else {
      sLog.Error( "BookmarkService::Handle_DeleteBookmarks()", "Service is not handled yet.  Returning NULL.");
    }

    // returns none
    return new PyNone();
}


PyResult BookmarkService::Handle_MoveBookmarksToFolder(PyCallArgs &call) {
  /**
            rows = bookmarkMgr.MoveBookmarksToFolder(folderID, bookmarkIDs)

23:39:40 E BookmarkService::Handle_MoveBookmarksToFolder(): Service is not handled yet.  Returning NULL.
23:39:40 [SvcCall]   Call Arguments:
23:39:40 [SvcCall]       Tuple: 2 elements
23:39:40 [SvcCall]         [ 0] Integer field: 2  <-folderID
23:39:40 [SvcCall]         [ 1] ObjectEx:
23:39:40 [SvcCall]         [ 1] Header:
23:39:40 [SvcCall]         [ 1]   Tuple: 2 elements
23:39:40 [SvcCall]         [ 1]     [ 0] Token: '__builtin__.set'
23:39:40 [SvcCall]         [ 1]     [ 1] Tuple: 1 elements
23:39:40 [SvcCall]         [ 1]     [ 1]   [ 0] List: 1 elements  <-bookmarkIDs as list
23:39:40 [SvcCall]         [ 1]     [ 1]   [ 0]   [ 0] Integer field: 13
23:39:40 [SvcCall]         [ 1] List data:
23:39:40 [SvcCall]         [ 1]   Empty
23:39:40 [SvcCall]         [ 1] Dict data:
23:39:40 [SvcCall]         [ 1]   Empty
*/
  /*
    Call_MoveBookmarksToFolder args;

    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode args.");
        return NULL;
    }

    uint16 folderID = call.byname["folderID"]->AsInt()->value();
    //  need to get bookmarks as a list using call.byname["bookmarkID"]->AsList();
*/
  sLog.Error( "BookmarkService::Handle_MoveBookmarksToFolder()", "Service is not handled yet.  Returning NULL.");
  call.client->SendInfoModalMsg("Moving Bookmarks to a folder is currently broken.");
  call.Dump(SERVICE__CALL_DUMP);
      return NULL;
  //return(new PyNone());
  // needs either a 'real' return or nothing.....*SRVERROR* TypeError: 'NoneType' object is not iterable

    uint32 bookmarkID = 0;
    uint32 ownerID = call.client->GetCharacterID();
    uint32 folderID = call.tuple->GetItem( 0 )->AsInt()->value();

    PyTuple* res = new PyTuple( 3 );
        res->items[ 0 ] = new PyInt( bookmarkID );           // Bookmark ID from Database 'bookmarks' table
        res->items[ 1 ] = new PyInt( ownerID);                // itemID
        res->items[ 2 ] = new PyInt( folderID );           // systemID

    return res;
}

PyResult BookmarkService::Handle_CopyBookmarks(PyCallArgs &call) {
  /**
            newBookmarks, message = bookmarkMgr.CopyBookmarks(bookmarksToCopy, folderID)
            */

      sLog.Error( "BookmarkService::Handle_CopyBookmarks()", "Service is not handled yet.  Returning NULL.");
  call.Dump(SERVICE__CALL_DUMP);

    return(new PyNone());
}

PyResult BookmarkService::Handle_AddBookmarkFromVoucher(PyCallArgs &call) {
  /**
	 sm.RemoteSvc('bookmark').AddBookmarkFromVoucher, itemID, ownerID, folderID, violateSafetyTimer=True)
            */

  sLog.Error( "BookmarkService::Handle_AddBookmarkFromVoucher()", "Service is not handled yet.  Returning NULL.");
  call.client->SendInfoModalMsg("Creating Bookmarks from Vouchers is currently broken.");
  call.Dump(SERVICE__CALL_DUMP);

    return(new PyNone());
}

/**
  bookmarkID, itemID, typeID, x, y, z, locationID = self.bookmarkMgr.BookmarkScanResult(locationID, memo, comment, resultID, ownerID, folderID=0)

*/