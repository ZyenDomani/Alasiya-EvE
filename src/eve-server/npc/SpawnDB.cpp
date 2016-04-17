/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2011 The EVEmu Team
 *    For the latest information visit http://evemu.org
 *    ------------------------------------------------------------------------------------
 *    This program is free software; you can redistribute it and/or modify it under
 *    the terms of the GNU Lesser General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option) any later
 *    version.
 *
 *    This program is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License along with
 *    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 *    http://www.gnu.org/copyleft/lesser.txt.
 *    ------------------------------------------------------------------------------------
 *    Author:   Allan
 */


#include "eve-server.h"

#include "npc/SpawnDB.h"

void SpawnDB::GetRegionFactionInfo(DBQueryResult& res) {
    if (!sDatabase.RunQuery(res, "SELECT regionID, ratFactionID FROM mapRegions WHERE ratFactionID != 0")) {
        sLog.Error("SystemDB::GetLootGroupTypes()", "Error in query: %s", res.error.c_str());
        return;
    }
}

void SpawnDB::GetFactionGroups(DBQueryResult& res) {
    if (!sDatabase.RunQuery(res, "SELECT shipClass, groupID, factionID FROM roidRatClassGroup")) {
        sLog.Error("SystemDB::GetLootGroupTypes()", "Error in query: %s", res.error.c_str());
        return;
    }
}

void SpawnDB::GetSpawnClasses(DBQueryResult& res) {
    if (!sDatabase.RunQuery(res, "SELECT type, sub, f, d, c, bc, bs, h, o, cf, cd, cc, cbc, cbs FROM roidRatSpawnClass")) {
        sLog.Error("SystemDB::GetLootGroupTypes()", "Error in query: %s", res.error.c_str());
        return;
    }
}

void SpawnDB::GetGroupTypeIDs(uint32 groupID, DBQueryResult& res) {
    if (!sDatabase.RunQuery(res, "SELECT typeID FROM invTypes WHERE groupID = %u", groupID)) {
        sLog.Error("SystemDB::GetLootGroupTypes()", "Error in query: %s", res.error.c_str());
        return;
    }
}

void SpawnDB::DeleteSpawnedRats()
{
    DBerror err;
    std::string query = "beltrat";
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE customInfo LIKE '%s'", query.c_str());
}

