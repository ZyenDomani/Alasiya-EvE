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
    Author:        Ubiquitatis
    Rewrite:    Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "PyServiceCD.h"
#include "packets/Bookmarks.h"
#include "cache/ObjCacheService.h"
#include "corporation/CorpBookmarkMgr.h"

PyCallable_Make_InnerDispatcher(CorpBookmarkMgr)

CorpBookmarkMgr::CorpBookmarkMgr(PyServiceMgr* mgr)
: PyService(mgr, "corpBookmarkMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(CorpBookmarkMgr, GetBookmarks);
    PyCallable_REG_CALL(CorpBookmarkMgr, UpdateBookmark);
    PyCallable_REG_CALL(CorpBookmarkMgr, UpdatePlayerBookmark);
    PyCallable_REG_CALL(CorpBookmarkMgr, MoveBookmarksToFolder);
}

CorpBookmarkMgr::~CorpBookmarkMgr()
{
    delete m_dispatch;
}

PyResult CorpBookmarkMgr::Handle_GetBookmarks(PyCallArgs& call)
{
    /*
              [PyTuple 2 items]
                [PyDict 238 kvp]        << bookmarks
                  [PyInt 715518945]
                  [PyPackedRow 65 bytes]
                    ["bookmarkID" => <715518945> [I4]]
                    ["ownerID" => <1630077495> [I4]]
                    ["itemID" => <0> [I8]]
                    ["typeID" => <5> [I4]]
                    ["memo" => <Class 1 - 5/10  > [WStr]]
                    ["created" => <129811403510000000> [FileTime]]
                    ["x" => <2537095137040> [R8]]
                    ["y" => <383826124800> [R8]]
                    ["z" => <-3172263813120> [R8]]
                    ["locationID" => <30003263> [I4]]
                    ["note" => <empty string> [WStr]]
                    ["creatorID" => <447642544> [I4]]
                    ["folderID" => <111571> [I4]]
                [PyDict 9 kvp]          << folders
                  [PyInt 521456]
                  [PyPackedRow 13 bytes]
                    ["folderID" => <521456> [I4]]
                    ["ownerID" => <1630077495> [I4]]
                    ["folderName" => <Bomb Spots> [WStr]]
                    ["creatorID" => <1610990724> [I4]]
            */
    ObjectCachedMethodID method_id(GetName(), "GetBookmarks");
    if(!m_manager->cache_service->IsCacheLoaded(method_id)) {
        PyTuple *tuple = new PyTuple(2);
            tuple->SetItem(0, m_db.GetBookmarks(call.client->GetCorporationID()));
            tuple->SetItem(1, m_db.GetFolders(call.client->GetCorporationID()));
        PyRep* rep = tuple;

        m_manager->cache_service->GiveCache(method_id, &rep);
    }

    return(m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id));
}

PyResult CorpBookmarkMgr::Handle_UpdateBookmark(PyCallArgs& call) {
    sLog.White( "CorpBookmarkMgr::Handle_UpdateBookmark()", "size=%u ", call.tuple->size() );
    call.Dump(COMMON__INFO);
    Call_UpdateBookmark args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    std::string memo = "";
    if ( args.memo->IsString() )
        memo = args.memo->AsString()->content();
    else if ( args.memo->IsWString() )
        memo = args.memo->AsWString()->content();
    else {
        sLog.Error( "BookmarkService::Handle_BookmarkLocation()", "args.memo is of the wrong type: '%s'.  Expected PyString or PyWString.", args.memo->TypeString() );
        return PyStatic.NewNone();
    }

    std::string comment = "";
    if ( args.comment->IsString() )
        comment = args.comment->AsString()->content();
    else if ( args.comment->IsWString() )
        comment = args.comment->AsWString()->content();
    else {
        sLog.Error( "BookmarkService::Handle_BookmarkLocation()", "args.comment is of the wrong type: '%s'.  Expected PyString or PyWString.", args.comment->TypeString() );
        return PyStatic.NewNone();
    }

    if (!m_db.UpdateBookmarkInDatabase(args.bookmarkID, args.ownerID, memo, comment, args.folderID))
        ;   // make client error here to let them know updating failed

    return PyStatic.NewNone();
}

PyResult CorpBookmarkMgr::Handle_UpdatePlayerBookmark(PyCallArgs& call) {
    sLog.White( "CorpBookmarkMgr::Handle_UpdatePlayerBookmark()", "size=%u ", call.tuple->size() );
    call.Dump(COMMON__INFO);
    Call_UpdateBookmark args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    std::string memo = "";
    if ( args.memo->IsString() )
        memo = args.memo->AsString()->content();
    else if ( args.memo->IsWString() )
        memo = args.memo->AsWString()->content();
    else {
        sLog.Error( "BookmarkService::Handle_BookmarkLocation()", "args.memo is of the wrong type: '%s'.  Expected PyString or PyWString.", args.memo->TypeString() );
        return PyStatic.NewNone();
    }

    std::string comment = "";
    if ( args.comment->IsString() )
        comment = args.comment->AsString()->content();
    else if ( args.comment->IsWString() )
        comment = args.comment->AsWString()->content();
    else {
        sLog.Error( "BookmarkService::Handle_BookmarkLocation()", "args.comment is of the wrong type: '%s'.  Expected PyString or PyWString.", args.comment->TypeString() );
        return PyStatic.NewNone();
    }

    if (!m_db.UpdateBookmarkInDatabase(args.bookmarkID, args.ownerID, memo, comment, args.folderID))
        ;   // make client error here to let them know updating failed

        return PyStatic.NewNone();
}

PyResult CorpBookmarkMgr::Handle_MoveBookmarksToFolder(PyCallArgs& call) {
        return PyStatic.NewNone();
}
