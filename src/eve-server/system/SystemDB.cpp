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

#include "system/SystemDB.h"

PyObject* SystemDB::ListFactions() {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT factionID FROM facFactions")) {
        codelog(DATABASE__ERROR, "Error in ListFactions query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

PyObject* SystemDB::ListJumps(uint32 stargateID) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT "
        "   celestialID AS toCelestialID,"
        "   solarSystemID AS locationID"
        " FROM mapJumps "
        "  LEFT JOIN mapDenormalize ON celestialID=itemID"
        " WHERE stargateID=%u", stargateID))
    {
        codelog(DATABASE__ERROR, "Error in ListJumps query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

PyPackedRow* SystemDB::GetSolarSystem(uint32 ssid) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT "
        " mss.solarSystemID,"
        " mss.solarSystemName,"
        " mss.x, mss.y, mss.z,"
        " mss.radius,"
        " mss.security,"
        " mss.constellationID,"
        " mss.factionID,"
        " mss.sunTypeID,"
        " mss.regionID,"
        " mlwc.wormholeClassID"
        " FROM mapSolarSystems AS mss"
        " LEFT JOIN mapLocationWormholeClasses AS mlwc ON mlwc.locationID = mss.regionID"
        " WHERE solarSystemID=%u", ssid ))
    {
        codelog(DATABASE__ERROR, "Error in GetSolarSystem query: %s", res.error.c_str());
        return nullptr;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        codelog(DATABASE__ERROR, "Error in GetSolarSystem query: no solarsystem for id %d", ssid);
        return nullptr;
    }

    return DBRowToPackedRow(row);
}

bool SystemDB::LoadSystemStaticEntities(uint32 systemID, std::vector<DBSystemEntity>& into) {
    std::stringstream query;
    query << "SELECT itemID,typeID,groupID,radius";
    query << " FROM mapDenormalize WHERE solarSystemID=%u ORDER BY itemID";

    DBQueryResult res;
    if(!sDatabase.RunQuery(res, query.str().c_str(), systemID )) {
        codelog(DATABASE__ERROR, "Error in LoadSystemStaticEntities query: %s", res.error.c_str());
        return false;
    }

    _log(DATABASE__RESULTS, "LoadSystemStaticEntities returned %u items", res.GetRowCount());

    DBResultRow row;
    DBSystemEntity entry;
    while(res.GetRow(row)) {
        entry.itemID = row.GetUInt(0);
        entry.typeID = row.GetInt(1);
        entry.groupID = row.GetInt(2);
        entry.radius = row.GetDouble(3);
        into.push_back(entry);
    }

    return true;
}

/* load system dynamics owned by the EvE systemID  */
bool SystemDB::LoadSystemDynamicEntities(uint32 systemID, std::vector<DBSystemDynamicEntity>& into) {
    using namespace EVEDB::invCategories;
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "   e.itemID,"
        "   e.itemName,"
        "   e.typeID,"
        "   t.groupID,"
        "   g.categoryID,"
        "   e.x, e.y, e.z,"
        "   IFNULL(e.customInfo, '0')"
        " FROM entity AS e"
        "  LEFT JOIN invTypes AS t ON t.typeID = e.typeID"
        "  LEFT JOIN invGroups AS g ON g.groupID = t.groupID"
        " WHERE e.locationID = %u"
        "  AND (g.categoryID NOT IN (%d, %d, %d, %d)"
        "  AND e.ownerID = 1)"  // get dynamics owned by the system -include abandonded ships
        "  OR g.categoryID = %u"    // - include orbitals (owned by npc corps)
        "  ORDER BY e.itemID",
        systemID,
        //exclude categories not applicable for in-space system entities or owned by player/corp :
        _System/*0*/, /*Character*/1, /*Station*/3, Asteroid/*25*/, //asteroids are owned/controlled by BeltMgr.
        Orbitals/*46*/ )) {
            codelog(DATABASE__ERROR, "Error in LoadSystemDynamicEntities query: %s", res.error.c_str());
            return false;
        }

    _log(DATABASE__RESULTS, "LoadSystemDynamicEntities returned %u items", res.GetRowCount());

    DBResultRow row;
    DBSystemDynamicEntity entry;
    while(res.GetRow(row)) {
        entry.itemID = row.GetUInt(0);
        entry.itemName = row.GetText(1);
        entry.typeID = row.GetInt(2);
        entry.ownerID = 1;
        entry.groupID = row.GetInt(3);
        entry.categoryID = (EVEItemCategories)row.GetInt(4);
        entry.corporationID = 0;
        entry.allianceID = 0;
        entry.factionID = 0;
        entry.x = row.GetDouble(5);
        entry.y = row.GetDouble(6);
        entry.z = row.GetDouble(7);
        entry.planetID = atoi(row.GetText(8));
        into.push_back(entry);
    }

    return true;
}

/* load system dynamics owned by players  */
bool SystemDB::LoadPlayerDynamicEntities(uint32 systemID, std::vector<DBSystemDynamicEntity>& into)
{
    using namespace EVEDB::invCategories;
    DBQueryResult res, res2;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        "   e.itemID,"
        "   e.itemName,"
        "   e.typeID,"
        "   e.ownerID,"
        "   t.groupID,"
        "   g.categoryID,"  //5
        "   e.x, e.y, e.z" //8
        " FROM entity AS e"
        "  LEFT JOIN invTypes AS t ON t.typeID = e.typeID"
        "  LEFT JOIN invGroups AS g ON g.groupID = t.groupID"
        " WHERE e.locationID = %u"
        "  AND g.categoryID IN (%d, %d, %d, %d, %d, %d, %d)"
        "  AND e.ownerID != 1"  // get dynamics not owned by the system
        //"  AND e.itemID NOT IN (c.shipID,c.capsuleID)"  // this is a problem....returns NOTHING because of this line.
        " ORDER BY e.itemID",
        systemID, Celestial/*2*/,   // Celestial is for containers (wrecks, jetcans, lsc)
        Deployable/*22*/,           // include deployed items owned by players or corps
        Drone/*18*/, Entity/*11*/,  // Entity also contains NPCs, sentrys, LCOs, and other destructible objects
        /*Structure*/23, StructureUpgrade/*39*/, SovereigntyStructure/*40*/ )) {
            codelog(DATABASE__ERROR, "Error in LoadPlayerDynamicEntities query: %s", res.error.c_str());
            return false;
    }

    _log(DATABASE__RESULTS, "LoadPlayerDynamicEntities returned %u items", res.GetRowCount());
    DBResultRow row, row2;
    DBSystemDynamicEntity entry;
    while(res.GetRow(row)) {
        entry.factionID = 0;
        entry.allianceID = 0;
        entry.corporationID = 0;
        entry.itemID = row.GetUInt(0);
        entry.itemName = row.GetText(1);
        entry.typeID = row.GetInt(2);
        entry.ownerID = row.GetInt(3);
        entry.groupID = row.GetInt(4);
        entry.categoryID = (EVEItemCategories)row.GetInt(5);
        entry.x = row.GetDouble(6);
        entry.y = row.GetDouble(7);
        entry.z = row.GetDouble(8);
        if (IsCorp(entry.ownerID)) {
            entry.corporationID = entry.ownerID;
            if (sDatabase.RunQuery(res2, "SELECT allianceID, warFactionID FROM crpCorporation WHERE corporationID = %u", entry.corporationID))
                if (res2.GetRow(row2))
                    entry.allianceID = row2.GetUInt(0);
                    entry.factionID = row2.GetUInt(1);
        } else {
            //  should we test for ownerID is NOT corp/player here?  dunno...what other entities own things?
            if (sDatabase.RunQuery(res2,
                        "SELECT c.corporationID, co.allianceID, co.warFactionID FROM chrCharacters AS c"
                        " LEFT JOIN crpCorporation AS co USING (corporationID)"
                        " WHERE c.characterID = %u", entry.ownerID))
            {
                if (res2.GetRow(row2)) {
                    entry.corporationID = row2.GetUInt(0);
                    entry.allianceID = row2.GetUInt(1);
                    entry.factionID = row2.GetUInt(2);
                }
            }
        }
        res2.Reset();
        into.push_back(entry);
    }
    return true;
}

uint32 SystemDB::GetObjectLocationID(uint32 itemID) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT locationID FROM entity WHERE itemID=%u", itemID )) {
        codelog(DATABASE__ERROR, "Error in GetObjectLocationID query: %s", res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (res.GetRow(row))
        return (row.GetUInt(0));
    return 0;
}

double SystemDB::GetItemTypeRadius(uint32 typeID) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT radius FROM invTypes WHERE typeID=%u", typeID )) {
        codelog(DATABASE__ERROR, "Error in GetItemTypeRadius query: %s", res.error.c_str());
        return 0.0;
    }

    DBResultRow row;
    if (res.GetRow(row))
        return (row.GetDouble(0));
    return 0.0;
}

double SystemDB::GetCelestialRadius(uint32 itemID) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT radius FROM mapDenormalize WHERE itemID=%u", itemID )) {
        codelog(DATABASE__ERROR, "Error in GetItemTypeRadius query: %s", res.error.c_str());
        return 10.1;
    }

    DBResultRow row;
    if (res.GetRow(row))
        return (row.GetDouble(0));
    return 12.3;
}

bool SystemDB::GetWrecksToTypes(DBQueryResult& res) {
    if(!sDatabase.RunQuery(res, "SELECT typeID, wreckTypeID FROM invTypesToWrecks")) {
        codelog(DATABASE__ERROR, "Error in GetWrecksToTypes query: %s", res.error.c_str());
        return false;
    }
    return true;
}

void SystemDB::GetLootGroups(DBQueryResult& res) {
    //if(!sDatabase.RunQuery(res, "SELECT groupID, lootGroupID, dropChance FROM npcLootGroup")) {
    if(!sDatabase.RunQuery(res, "SELECT npcGroupID, itemGroupID, groupDropChance FROM lootGroup")) {
        codelog(DATABASE__ERROR, "Error in GetLootGroups query: %s", res.error.c_str());
        return;
    }
}

void SystemDB::GetLootGroupTypes(DBQueryResult& res) {
    //if(!sDatabase.RunQuery(res, "SELECT lootGroupID, typeID, chance, minQuantity, maxQuantity FROM npcLootGroupType")) {
    if(!sDatabase.RunQuery(res, "SELECT itemGroupID, itemID, itemMetaLevel, minAmount, maxAmount FROM lootItemGroup")) {
        codelog(DATABASE__ERROR, "Error in GetLootGroupTypes query: %s", res.error.c_str());
        return;
    }
}

void SystemDB::GetPlanets(uint32 systemID, std::vector<DBGPointEntity> &planetIDs, uint8 &total) {
// groupID = 7
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT itemID, x, y, z, radius FROM mapDenormalize WHERE solarSystemID = %u AND groupID = 7", systemID);

    DBResultRow row;
    DBGPointEntity entry;
    while(res.GetRow(row)) {
        entry.idx = total;
        entry.itemID = row.GetUInt(0);
        entry.position = GPoint (
			row.GetDouble(1),
			row.GetDouble(2),
			row.GetDouble(3)
        );
        entry.radius = row.GetDouble(4);
        planetIDs.push_back(entry);
        ++total;
    }
}

void SystemDB::GetMoons(uint32 systemID, std::vector<DBGPointEntity> &moonIDs, uint8 &total) {
// groupID = 8
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT itemID, x, y, z, radius FROM mapDenormalize WHERE solarSystemID = %u AND groupID = 8", systemID);

    DBResultRow row;
    DBGPointEntity entry;
    while(res.GetRow(row)) {
        entry.idx = total;
        entry.itemID = row.GetUInt(0);
        entry.position = GPoint (
			row.GetDouble(1),
			row.GetDouble(2),
			row.GetDouble(3)
			);
        entry.radius = row.GetDouble(4);

        moonIDs.push_back(entry);
        ++total;
    }
}

void SystemDB::GetBelts(uint32 systemID, std::vector< DBGPointEntity > &beltIDs, uint8 &total)
{
    // groupID = 9
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT itemID, x, y, z, radius FROM mapDenormalize WHERE solarSystemID = %u AND groupID = 9", systemID);

    DBResultRow row;
    DBGPointEntity entry;
    while(res.GetRow(row)) {
        entry.idx = total;
        entry.itemID = row.GetUInt(0);
        entry.position = GPoint (
            row.GetDouble(1),
            row.GetDouble(2),
            row.GetDouble(3)
        );
        entry.radius = row.GetDouble(4);

        beltIDs.push_back(entry);
        ++total;
    }
}

void SystemDB::GetGates(uint32 systemID, std::vector< DBGPointEntity > &gateIDs, uint8 &total)
{
    // groupID = 10
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT itemID, x, y, z, radius FROM mapDenormalize WHERE solarSystemID = %u AND groupID = 10", systemID);

    DBResultRow row;
    DBGPointEntity entry;
    while(res.GetRow(row)) {
        entry.idx = total;
        entry.itemID = row.GetUInt(0);
        entry.position = GPoint (
            row.GetDouble(1),
            row.GetDouble(2),
            row.GetDouble(3)
        );
        entry.radius = row.GetDouble(4);

        gateIDs.push_back(entry);
        ++total;
    }
}
