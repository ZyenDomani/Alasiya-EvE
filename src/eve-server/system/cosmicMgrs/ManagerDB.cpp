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
    Author:        Allan
*/

#include "eve-server.h"

#include "system/cosmicMgrs/ManagerDB.h"

void ManagerDB::GetRegionFactionInfo(DBQueryResult& res) {
    if (!sDatabase.RunQuery(res, "SELECT regionID, ratFactionID FROM mapRegions WHERE ratFactionID != 0")) {
        _log(DATABASE__ERROR, "Error in GetLootGroupTypes query: %s", res.error.c_str());
    }
}

void ManagerDB::GetFactionGroups(DBQueryResult& res) {
    if (!sDatabase.RunQuery(res, "SELECT shipClass, groupID, factionID FROM roidRatClassGroup")) {
        _log(DATABASE__ERROR, "Error in GetLootGroupTypes query: %s", res.error.c_str());
    }
}

void ManagerDB::GetSpawnClasses(DBQueryResult& res) {
    if (!sDatabase.RunQuery(res, "SELECT type, sub, f, d, c, bc, bs, h, o, cf, cd, cc, cbc, cbs FROM roidRatSpawnClass")) {
        _log(DATABASE__ERROR, "Error in GetLootGroupTypes query: %s", res.error.c_str());
    }
}

void ManagerDB::GetGroupTypeIDs(uint32 groupID, DBQueryResult& res) {
    if (!sDatabase.RunQuery(res, "SELECT typeID FROM invTypes WHERE groupID = %u", groupID)) {
        _log(DATABASE__ERROR, "Error in GetLootGroupTypes query: %s", res.error.c_str());
    }
}

void ManagerDB::DeleteSpawnedRats()
{
    DBerror err;
    std::string query = "'beltrat'";
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE customInfo LIKE %s", query.c_str());
}


bool ManagerDB::GetRoidDist(const char * sec, std::map<float, uint32> &roids) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        " SELECT roidID, percent FROM roidDistribution WHERE systemSec = '%s' ", sec))
    {
        codelog(DATABASE__ERROR, "Error in GetRoidDist query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    float tot = 0.0;
    while (res.GetRow(row)) {
        tot += row.GetFloat(1);
        roids[tot] = row.GetUInt(0);
    }

    return !roids.empty();
}

bool ManagerDB::LoadSystemRoids(uint32 systemID, uint32 beltID, std::vector<DBAsteroidSE>& into)
{
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "   itemID,"
        "   itemName,"
        "   typeID,"
        "   systemID,"
        "   beltID,"
        "   quantity,"
        "   radius,"
        "   x, y, z"
        " FROM roidItems" //sysAsteroids
        " WHERE systemID = %u"
        "  AND beltID = %u", systemID, beltID)) {
        _log(DATABASE__ERROR, "Error in LoadSystemRoids query: %s", res.error.c_str());
        return false;
    }

    _log(DATABASE__RESULTS, "LoadSystemRoids returned %u items", res.GetRowCount());
    DBResultRow row;
    DBAsteroidSE entry;
    while(res.GetRow(row)) {
        entry.itemID = row.GetInt(0);
        entry.itemName = row.GetText(1);
        entry.typeID = row.GetInt(2);
        entry.systemID = row.GetInt(3);
        entry.beltID = row.GetInt(4);
        entry.quantity = row.GetDouble(5);
        entry.radius = row.GetDouble(6);
        entry.x = row.GetDouble(7);
        entry.y = row.GetDouble(8);
        entry.z = row.GetDouble(9);
        into.push_back(entry);
    }

    return !into.empty();
}

void ManagerDB::SaveSystemRoids(uint32 systemID, std::vector<DBAsteroidSE> roids)
{
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO roidItems"; //sysAsteroids
    Inserts << " (itemID,itemName,typeID,systemID,beltID,quantity,radius,x, y, z)";
    bool first = true;
    for (auto cur : roids) {
        if (first) {
            Inserts << " VALUES ";
            first = false;
        } else
            Inserts << ", ";
        // itemID and attributeID keys.
        Inserts << "(" << cur.itemID << ", '" << cur.itemName << "', " << cur.typeID << ", " << systemID << ", " << cur.beltID << ", ";
        Inserts << cur.quantity << ", " << cur.radius << ", " << cur.x << ", " << cur.y << ", " << cur.z << ")";
    }
    // did we get at least 1 insert?
    if (!first) {
        // finish creating the command.
        Inserts << "ON DUPLICATE KEY UPDATE ";
        Inserts << "quantity=VALUES(quantity), ";
        Inserts << "radius=VALUES(radius)";
        // execute the command.
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SaveSystemRoids - unable to save roids");
    }
}


void ManagerDB::GetDunGroupData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT d.itemTypeID, t.groupID, g.categoryID, d.xpos, d.ypos, d.zpos"
        " FROM dunGroupData AS d"
        "  LEFT JOIN invTypes AS t ON d.itemTypeID = t.typeID"
        "  LEFT JOIN invGroups AS g ON g.groupID = t.groupID" )) {
        _log(DATABASE__ERROR, "Error in GetDunGroupData query: %s", res.error.c_str());
    }
}

void ManagerDB::GetDunRoomData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT dunRoomID, dunGroupID, xpos, ypos, zpos FROM dunRoomData"))
        _log(DATABASE__ERROR, "Error in GetDunRoomData query: %s", res.error.c_str());
}

void ManagerDB::GetDunRoomInfo(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT dunRoomID, dunRoomType, dunRoomCategory, dunRoomSpawnID, dunRoomSpawnType FROM dunRoomInfo"))
        _log(DATABASE__ERROR, "Error in GetDunRoomInfo query: %s", res.error.c_str());
}

void ManagerDB::GetDunSpawnInfo(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT dunRoomSpawnID, dunRoomSpawnType, xpos, ypos, zpos FROM dunRoomSpawnInfo"))
        _log(DATABASE__ERROR, "Error in GetDunSpawnInfo query: %s", res.error.c_str());
}

void ManagerDB::GetDunTemplates(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT dunTemplateID, dunRoomID, dunEntryID, dunTypeID, dunSpawnType, dunRooms, dunRoomTypeID, dunRoomCategoryID FROM dunTemplates"))
        _log(DATABASE__ERROR, "Error in GetDunTemplates query: %s", res.error.c_str());
}

bool ManagerDB::GetActiveDungeons(uint32 systemID, std::vector<DBActiveDungeon>& into)
{
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "   systemID,"
        "   state,"
        "   dungeonID,"
        "   dunTemplateID,"
        "   dunExpiryTime,"
        "   xpos, ypos, zpos"
        " FROM dunActive"   //Active Dungeons
        " WHERE systemID = %u", systemID)) {
        _log(DATABASE__ERROR, "Error in GetActiveDungeons query: %s", res.error.c_str());
        return false;
    }

    _log(DATABASE__RESULTS, "GetActiveDungeons returned %u items", res.GetRowCount());
    DBResultRow row;
    DBActiveDungeon entry;
    while(res.GetRow(row)) {
        entry.systemID = row.GetInt(0);
        entry.state = row.GetInt(1);
        entry.dungeonID = row.GetInt(2);
        entry.dunTemplateID = row.GetInt(3);
        entry.dunExpiryTime = row.GetInt64(4);
        entry.x = row.GetInt(5);
        entry.y = row.GetInt(6);
        entry.z = row.GetInt(7);
        into.push_back(entry);
    }

    return !into.empty();
}

void ManagerDB::SaveActiveDungeon(DBActiveDungeon& dun)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "INSERT INTO dunActive" //Active Dungeons
        " (systemID, state, dungeonID, dunTemplateID, dunExpiryTime, xpos, ypos, zpos)"
        " VALUES "
        "(%u, %u, %u, %u, " PRIu64 ", %f, %f, %f)",
        dun.systemID, dun.state, dun.dungeonID, dun.dunTemplateID, dun.dunExpiryTime, dun.x, dun.y, dun.z )) {
        _log(DATABASE__ERROR, "SaveActiveDungeon - unable to save dungeonID %u", dun.dungeonID);
    }
}
