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
    Author:        Bloody.Rabbit
    Updated:        Allan (Zhy)   18Jan14
*/

/** @todo  update this for better code and make sure folderID=0 is changed to NULL in db */

#include "eve-server.h"

#include "system/BookmarkDB.h"
#include "system/BookmarkService.h"

PyRep* BookmarkDB::GetBMData(uint32 folderID)
{
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  bookmarkID,"
        "  ownerID,"
        "  memo,"
        "  note,"
        "  folderID" // NULLIF(exp1, exp2) {IF expr1 = expr2 THEN NULL ELSE expr1 END}
        " FROM bookmarks"
        " WHERE folderID = %u",
        folderID))
    {
        sLog.Error( "BookmarkDB::GetBMData()", "Failed to query bookmarks for folderID %u: %s.", folderID, res.error.c_str() );
        return nullptr;
    }

    return DBResultToCRowset(res);
}

void BookmarkDB::GetBookmarkByFolderID(int32 folderID, std::vector< int32 >& bmIDs)
{
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  bookmarkID"
        " FROM bookmarks"
        " WHERE folderID = %u",
        folderID))
    {
        sLog.Error( "BookmarkDB::GetBookmarkByFolderID()", "Failed to query bookmarks for folderID %u: %s.", folderID, res.error.c_str() );
        return;
    }

    DBResultRow row;
    while (res.GetRow(row))
        bmIDs.push_back(row.GetInt(0));
}

PyRep *BookmarkDB::GetBookmarks(uint32 ownerID) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  bookmarkID,"
        "  ownerID,"
        "  itemID,"
        "  typeID,"
        "  memo,"
        "  created,"
        "  x, y, z,"
        "  locationID,"
        "  note,"
        "  creatorID,"
        "  NULLIF(folderID, 0)"    //return NULL if folderID = 0
        " FROM bookmarks"
        " WHERE ownerID = %u",
        ownerID))
    {
        sLog.Error( "BookmarkDB::GetBookmarks()", "Failed to query bookmarks for owner %u: %s.", ownerID, res.error.c_str() );
        return nullptr;
    }

    if (IsPlayerCorp(ownerID))
        return DBResultToPackedRowDict(res, "bookmarkID");
    else
        return DBResultToCRowset(res);
}

PyRep *BookmarkDB::GetFolders(uint32 ownerID) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  ownerID,"
        "  folderID,"
        "  folderName,"
        "  creatorID"
        " FROM bookmarkFolders"
        " WHERE ownerID = %u",
        ownerID))
    {
        sLog.Error( "BookmarkDB::GetFolders()", "Failed to query bookmarks for owner %u: %s.", ownerID, res.error.c_str() );
        return nullptr;
    }

    if (IsPlayerCorp(ownerID))
        return DBResultToPackedRowDict(res, "folderID");
    else
        return DBResultToCRowset(res);
}

const char* BookmarkDB::GetBookmarkName(uint32 bookmarkID)
{

    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  memo"
        " FROM bookmarks"
        " WHERE bookmarkID = %u",
        bookmarkID))
    {
        sLog.Error( "BookmarkDB::GetBookmarkName()", "Failed to query bookmarkID %u: %s.", bookmarkID, res.error.c_str() );
        return nullptr;
    }

    DBResultRow row;
    if (!res.GetRow(row))
        return nullptr;

    return row.GetText(0);
}

bool BookmarkDB::GetBookmarkInformation(uint32 bookmarkID, uint32& itemID, uint32& typeID, uint32& locationID, double& x, double& y, double& z)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  itemID,"
        "  typeID,"
        "  locationID,"
        "  x, y, z"
        " FROM bookmarks "
        " WHERE bookmarkID = %u ", bookmarkID))
    {
        sLog.Error( "BookmarkDB::GetBookmarkInformation()", "Error in query: %s", res.error.c_str() );
        return false;
    }

    // Query went through, but check to see if there were zero rows, ie bookmarkID was invalid:
    DBResultRow row;
    if (!res.GetRow(row))
        return false;

    // Bookmark 'bookmarkID' was found, Send back bookmark information:
    itemID = row.GetUInt(0);
    typeID = row.GetUInt(1);
    locationID = row.GetUInt(2);
    x = row.GetDouble(3);
    y = row.GetDouble(4);
    z = row.GetDouble(5);

    return true;
}


uint32 BookmarkDB::SaveNewBookmarkToDatabase(uint32 ownerID, uint32 itemID, uint32 typeID, std::string memo, GPoint point, uint32 locationID, std::string note, uint32 creatorID, uint32 folderID)
{
    std::string eMemo, eNote;
    sDatabase.DoEscapeString(eMemo, memo.c_str());
    sDatabase.DoEscapeString(eNote, note.c_str());
    uint32 bookmarkID = 0;

    DBerror err;
    if (folderID > 0) {
        if (!sDatabase.RunQueryLID(err, bookmarkID,
            " INSERT INTO bookmarks "
            " (ownerID, itemID, typeID, memo, created, x, y, z, locationID, note, creatorID, folderID)"
            " VALUES (%u, %u, %u, '%s', %f, %f, %f, %f, %u, '%s', %u, %u) ",
            ownerID, itemID, typeID, eMemo.c_str(), GetFileTimeNow(), point.x, point.y, point.z, locationID, eNote.c_str(), creatorID, folderID ))
        {
            sLog.Error( "BookmarkDB::SaveNewBookmarkToDatabase(1)", "Error in query, Bookmark content couldn't be saved: %s", err.c_str() );
            return 0;
        }
    } else {
        if (!sDatabase.RunQueryLID(err, bookmarkID,
            " INSERT INTO bookmarks "
            " (ownerID, itemID, typeID, memo, created, x, y, z, locationID, note, creatorID)"
            " VALUES (%u, %u, %u, '%s', %f, %f, %f, %f, %u, '%s', %u) ",
            ownerID, itemID, typeID, eMemo.c_str(), GetFileTimeNow(), point.x, point.y, point.z, locationID, eNote.c_str(), creatorID ))
        {
            sLog.Error( "BookmarkDB::SaveNewBookmarkToDatabase(2)", "Error in query, Bookmark content couldn't be saved: %s", err.c_str() );
            return 0;
        }
    }

    return bookmarkID;
}

bool BookmarkDB::DeleteBookmarkFromDatabase(uint32 ownerID, uint32 bookmarkID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        " DELETE FROM bookmarks "
        " WHERE ownerID = %u AND bookmarkID = %u", ownerID, bookmarkID
        ))
    {
        sLog.Error( "BookmarkDB::DeleteBookmarkFromDatabase()", "Error in query: %s", err.c_str() );
        return false;
    }

    return true;
}

bool BookmarkDB::DeleteBookmarksFromDatabase(std::vector<int32>* bookmarkList)
{
    std::stringstream st;
    std::string listString;

    std::size_t size = bookmarkList->size();
    for (int8 i=0; i<size; i++) {
        st << bookmarkList->at(i);
        if (i < (size-1))
            st << ", ";
    }

    DBerror err;
    if (!sDatabase.RunQuery(err,
        " DELETE FROM bookmarks "
        " WHERE bookmarkID IN (%s)", st.str().c_str()
        ))
    {
        sLog.Error( "BookmarkDB::DeleteBookmarksFromDatabase()", "Error in query: %s", err.c_str() );
        return false;
    }

    return true;
}

bool BookmarkDB::UpdateBookmarkInDatabase(uint32 bookmarkID, uint32 ownerID, std::string memo, std::string note, uint32 folderID/*0*/)
{
    std::string memo_fixed = "";
    sDatabase.DoEscapeString(memo_fixed, memo.c_str());

    DBerror err;
    if (!sDatabase.RunQuery(err,
        " UPDATE bookmarks "
        " SET "
        " memo = '%s', note = '%s', folderID = %u"
        " WHERE bookmarkID = %u AND ownerID = %u",
        memo_fixed.c_str(), note.c_str(), folderID, bookmarkID, ownerID
        ))
    {
        sLog.Error( "BookmarkDB::UpdateBookmarkInDatabase()", "Error in query: %s", err.c_str() );
        return false;
    }

    return true;
}

uint32 BookmarkDB::SaveNewFolderToDatabase(std::string folderName, uint32 ownerID)
{
    uint32 folderID = 0;
    std::string folderName_fixed = "";
    sDatabase.DoEscapeString(folderName_fixed, folderName.c_str());

    DBerror err;
    if (!sDatabase.RunQueryLID(err, folderID,
        " INSERT INTO bookmarkFolders"
        " (folderName, ownerID, creatorID)"
        " VALUES ('%s', %u, %u) ",
          folderName_fixed.c_str(), ownerID, ownerID ))
    {
        sLog.Error( "BookmarkDB::SaveNewFolderToDatabase()", "Error in query, Folder couldn't be saved: %s", err.c_str() );
        return 0;
    }

    return folderID;
}

bool BookmarkDB::UpdateFolderInDatabase(int32 folderID, std::string folderName)
{
    std::string folderName_fixed = "";
    sDatabase.DoEscapeString(folderName_fixed, folderName.c_str());

    DBerror err;
    if (!sDatabase.RunQuery(err,
        " UPDATE bookmarkFolders"
        "  SET  folderName = '%s'"
        " WHERE folderID = %u",
        folderName_fixed.c_str(), folderID))
    {
        sLog.Error( "BookmarkDB::UpdateFolderInDatabase()", "Error in query, Folder couldn't be saved: %s", err.c_str() );
        return false;
    }

    return true;
}

bool BookmarkDB::DeleteFolderFromDatabase(int32 folderID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        " DELETE FROM bookmarkFolders "
        " WHERE folderID = %u",
        folderID))
    {
        sLog.Error( "BookmarkDB::DeleteFolderFromDatabase()", "Error in query: %s", err.c_str() );
        return false;
    }

    return true;
}

void BookmarkDB::MoveBookmarkToFolder(int32 folderID, std::vector<int32>* bookmarkList)
{
    std::stringstream st;
    std::string listString;

    std::size_t size = bookmarkList->size();
    for (int8 i=0; i<size; i++) {
        st << bookmarkList->at(i);
        if (i < (size-1))
            st << ", ";
    }

    DBerror err;
    if (!sDatabase.RunQuery(err,
        " UPDATE bookmarks"
        "  SET  folderID = %i"
        " WHERE bookmarkID IN (%s)", folderID, st.str().c_str() ))
    {
        sLog.Error( "BookmarkDB::MoveBookmarkToFolder()", "Error in query, couldn't move bookmarks: %s", err.c_str() );
    }
}
