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
