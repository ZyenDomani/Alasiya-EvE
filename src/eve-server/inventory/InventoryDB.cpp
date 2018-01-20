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
    Author:     Zhur
    Updates:    Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "PyCallable.h"
#include "character/Character.h"
#include "inventory/ItemType.h"
#include "manufacturing/Blueprint.h"
#include "ship/Ship.h"
#include "station/Station.h"
#include "system/Asteroid.h"
#include "system/SolarSystem.h"

bool InventoryDB::GetCategory(EVEItemCategories category, CategoryData &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  categoryName,"
        "  description,"
        "  published "
        " FROM invCategories "
        " WHERE categoryID=%u",
        (uint32)category))
    {
        codelog(DATABASE__ERROR, "Error in GetCategory query: %s.", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "Category %u not found.", (uint32)category);
        return false;
    }

    into.name = row.GetText(0);
    into.description = row.GetText(1);
    into.published = (row.GetInt(2) ? true : false);

    return true;
}

bool InventoryDB::GetGroup(uint32 groupID, GroupData &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  categoryID,"
        "  groupName,"
        "  description,"
        "  useBasePrice,"
        "  allowManufacture,"
        "  allowRecycler,"
        "  anchored,"
        "  anchorable,"
        "  fittableNonSingleton,"
        "  published "
        " FROM invGroups "
        " WHERE groupID=%u",
        groupID))
    {
        codelog(DATABASE__ERROR, "Failed to query group %u: %s.", groupID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "Group %u not found.", groupID);
        return false;
    }

    into.category = (EVEItemCategories)row.GetUInt(0);
    into.name = row.GetText(1);
    into.description = row.GetText(2);
    into.useBasePrice = (row.GetInt(3) ? true : false);
    into.allowManufacture = (row.GetInt(4) ? true : false);
    into.allowRecycler = (row.GetInt(5) ? true : false);
    into.anchored = (row.GetInt(6) ? true : false);
    into.anchorable = (row.GetInt(7) ? true : false);
    into.fittableNonSingleton = (row.GetInt(8) ? true : false);
    into.published = (row.GetInt(9) ? true : false);

    return true;
}

bool InventoryDB::GetType(uint32 typeID, TypeData &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  groupID,"
        "  typeName,"
        "  description,"
        "  radius,"
        "  mass,"
        "  volume,"
        "  capacity,"
        "  portionSize,"
        "  raceID,"
        "  basePrice,"
        "  published,"
        "  marketGroupID,"
        "  chanceOfDuplicating "
        " FROM invTypes "
        " WHERE typeID=%u",
        typeID))
    {
        codelog(DATABASE__ERROR, "Failed to query type %u: %s.", typeID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "Type %u not found.", typeID);
        return false;
    }

    into.groupID = row.GetUInt(0);
    into.name = row.GetText(1);
    into.description = row.GetText(2);
    into.radius = row.GetDouble(3);
    into.mass = row.GetDouble(4);
    into.volume = row.GetDouble(5);
    into.capacity = row.GetDouble(6);
    into.portionSize = row.GetUInt(7);
    into.race = EVERace(row.IsNull(8) ? 0 : row.GetUInt(8));
	into.basePrice = row.GetInt64(9) /1000;
    into.published = (row.GetInt(10) ? true : false);
    into.marketGroupID = (row.IsNull(11) ? 0 : row.GetUInt(11));
    into.chanceOfDuplicating = row.GetDouble(12);

    return true;
}

bool InventoryDB::GetCharacterType(uint32 bloodlineID, CharacterTypeData &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  bloodlineName,"
        "  raceID,"
        "  description,"
        "  maleDescription,"
        "  femaleDescription,"
        "  shipTypeID,"
        "  corporationID,"
        "  perception,"
        "  willpower,"
        "  charisma,"
        "  memory,"
        "  intelligence,"
        "  shortDescription,"
        "  shortMaleDescription,"
        "  shortFemaleDescription "
        " FROM chrBloodlines "
        " WHERE bloodlineID = %u",
        bloodlineID))
    {
        codelog(DATABASE__ERROR, "Failed to query bloodline %u: %s.", bloodlineID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "No data found for bloodline %u.", bloodlineID);
        return false;
    }

    into.bloodlineName = row.GetText(0);
    into.race = (EVERace)row.GetUInt(1);
    into.description = row.GetText(2);
    into.maleDescription = row.GetText(3);
    into.femaleDescription = row.GetText(4);
    into.shipTypeID = row.GetUInt(5);
    into.corporationID = row.GetUInt(6);
    into.perception = row.GetUInt(7);
    into.willpower = row.GetUInt(8);
    into.charisma = row.GetUInt(9);
    into.memory = row.GetUInt(10);
    into.intelligence = row.GetUInt(11);
    into.shortDescription = row.GetText(12);
    into.shortMaleDescription = row.GetText(13);
    into.shortFemaleDescription = row.GetText(14);

    return true;
}

bool InventoryDB::GetCharacterTypeByBloodline(uint32 bloodlineID, uint32 &characterTypeID) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  typeID "
        " FROM bloodlineTypes"
        " WHERE bloodlineID = %u",
        bloodlineID))
    {
        codelog(DATABASE__ERROR, "Failed to query bloodline %u: %s.", bloodlineID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "No data for bloodline %u.", bloodlineID);
        return false;
    }

    characterTypeID = row.GetUInt(0);

    return true;
}

bool InventoryDB::GetBloodlineByCharacterType(uint32 characterTypeID, uint32 &bloodlineID) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  bloodlineID"
        " FROM bloodlineTypes"
        " WHERE typeID = %u",
        characterTypeID))
    {
        codelog(DATABASE__ERROR, "Failed to query character type %u: %s.", characterTypeID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "No data for character type %u.", characterTypeID);
        return false;
    }

    bloodlineID = row.GetUInt(0);

    return true;
}

bool InventoryDB::GetCharacterType(uint32 characterTypeID, uint32 &bloodlineID, CharacterTypeData &into) {
    if(!GetBloodlineByCharacterType(characterTypeID, bloodlineID))
        return false;
    return GetCharacterType(bloodlineID, into);
}

bool InventoryDB::GetCharacterTypeByBloodline(uint32 bloodlineID, uint32 &characterTypeID, CharacterTypeData &into) {
    if(!GetCharacterTypeByBloodline(bloodlineID, characterTypeID))
        return false;
    return GetCharacterType(bloodlineID, into);
}

bool InventoryDB::GetShipType(uint32 shipTypeID, ShipTypeData &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "   weaponTypeID, miningTypeID, skillTypeID"
        " FROM shipTypes"
        " WHERE shipTypeID = %u",
        shipTypeID))
    {
        codelog(DATABASE__ERROR, "Failed to query ship type %u: %s.", shipTypeID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "Ship type %u not found.", shipTypeID);
        return false;
    }

    into.mWeaponTypeID = row.IsNull(0) ? 0 : row.GetUInt(0);
    into.mMiningTypeID = row.IsNull(1) ? 0 : row.GetUInt(1);
    into.mSkillTypeID = row.IsNull(2) ? 0 : row.GetUInt(2);

    return true;
}

bool InventoryDB::GetItem(uint32 itemID, ItemData &into) {
    /* called by RefPtr<_Ty> _Load() at InventoryItem.h:189 */
    DBQueryResult res;

    // For ranges of itemIDs we use specialized tables:
    if (IsRegion(itemID)) {
        //region
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  regionName, 3 AS typeID, factionID, 1 AS locationID, 0 AS flag, 0 AS contraband,"
            "  1 AS singleton, 1 AS quantity, x, y, z, '' AS customInfo"
            " FROM mapRegions"
            " WHERE regionID=%u", itemID))
        {
            codelog(DATABASE__ERROR, "Error in query for region %u: %s", itemID, res.error.c_str());
            return false;
        }
    } else if (IsConstellation(itemID)) {
        //contellation
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  constellationName, 4 AS typeID, factionID, regionID, 0 AS flag, 0 AS contraband,"
            "  1 AS singleton, 1 AS quantity, x, y, z, '' AS customInfo"
            " FROM mapConstellations"
            " WHERE constellationID=%u", itemID))
        {
            codelog(DATABASE__ERROR, "Error in query for contellation %u: %s", itemID, res.error.c_str());
            return false;
        }
    } else if (IsSolarSystem(itemID)) {
        //solar system
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  solarSystemName, 5 AS typeID, factionID, constellationID, 0 AS flag, 0 AS contraband,"
            "  1 AS singleton, 1 AS quantity, x, y, z, '' AS customInfo"
            " FROM mapSolarSystems"
            " WHERE solarSystemID=%u", itemID))
        {
            codelog(DATABASE__ERROR, "Error in query for solar system %u: %s", itemID, res.error.c_str());
            return false;
        }
    } else if (IsStargate(itemID)) {
        //use mapDenormalize LEFT-JOIN-ing mapSolarSystems to get factionID
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  itemName, typeID, factionID, solarSystemID, 0 AS flag, 0 AS contraband,"
            "  1 AS singleton, 1 AS quantity, mapDenormalize.x, mapDenormalize.y, mapDenormalize.z, '' AS customInfo"
            " FROM mapDenormalize"
            " LEFT JOIN mapSolarSystems USING (solarSystemID)"
            " WHERE itemID=%u", itemID))
        {
            codelog(DATABASE__ERROR, "Error in query for stargate %u: %s", itemID, res.error.c_str());
            return false;
        }
    } else if (IsStation(itemID)) {
        //station
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  stationName, stationTypeID, corporationID, solarSystemID, 0 AS flag, 0 AS contraband,"
            "  1 AS singleton, 1 AS quantity, x, y, z, '' AS customInfo"
            " FROM staStations"
            " WHERE stationID=%u", itemID))
        {
            codelog(DATABASE__ERROR, "Error in query for station %u: %s", itemID, res.error.c_str());
            return false;
        }
    } else if (IsCelestial(itemID)) {
        //use mapDenormalize
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  itemName, typeID, 1 AS ownerID, solarSystemID, 0 AS flag, 0 AS contraband,"
            "  1 AS singleton, 1 AS quantity, x, y, z, '' AS customInfo"
            " FROM mapDenormalize"
            " WHERE itemID=%u", itemID))
        {
            codelog(DATABASE__ERROR, "Error in query for universe celestial %u: %s", itemID, res.error.c_str());
            return false;
        }
    } else if (IsAsteroid(itemID)) {
        //use sysAsteroids
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  itemName, typeID, 1 AS ownerID, systemID, 0 AS flag, 0 AS contraband,"
            "  1 AS singleton, quantity, x, y, z, '' AS customInfo"
            " FROM sysAsteroids"
            " WHERE itemID=%u", itemID))
        {
            codelog(DATABASE__ERROR, "Error in query for asteroid %u: %s", itemID, res.error.c_str());
            return false;
        }
    } else if (IsCharacter(itemID)) {
        //use chrCharacters
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  name, typeID, 1 AS ownerID, solarSystemID, flag, 0 AS contraband,"
            "  1 AS singleton, 1 AS quantity, 0 AS x, 0 AS y, 0 AS z, '' AS customInfo"
            " FROM chrCharacters"
            " WHERE characterID=%u", itemID))
        {
            codelog(DATABASE__ERROR, "Error in query for character %u: %s", itemID, res.error.c_str());
            return false;
        }
    } else if (IsOffice(itemID)) {
        //use staOffices
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  name, typeID, corporationID, solarSystemID, flag, 0 AS contraband,"
            "  1 AS singleton, 1 AS quantity, 0 AS x, 0 AS y, 0 AS z, '' AS customInfo"
            " FROM staOffices"
            " WHERE itemID=%u", itemID))
        {
            codelog(DATABASE__ERROR, "Error in query for character %u: %s", itemID, res.error.c_str());
            return false;
        }
    } else {
        //fallback to entity
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  itemName, typeID, ownerID, locationID, flag, contraband,"
            "  singleton, quantity, x, y, z, customInfo"
            " FROM entity WHERE itemID=%u", itemID))
        {
            codelog(DATABASE__ERROR, "Error in query for item %u: %s", itemID, res.error.c_str());
            return false;
        }
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "Item %u not found.", itemID);
        return false;
    }

    into.name = row.GetText(0);
    into.typeID = row.GetUInt(1);
    into.ownerID = (row.IsNull(2) ? 1 : row.GetUInt(2));
    into.locationID = (row.IsNull(3) ? 0 : row.GetUInt(3));
    into.flag = (EVEItemFlags)row.GetUInt(4);
    into.contraband = (row.GetInt(5) ? true : false);
    into.singleton = (row.GetInt(6) ? true : false);
    into.quantity = row.GetUInt(7);

    into.position.x = row.GetDouble(8);
    into.position.y = row.GetDouble(9);
    into.position.z = row.GetDouble(10);

    into.customInfo = (row.IsNull(11) ? "" : row.GetText(11));

    return true;
}

uint32 InventoryDB::NewItem(const ItemData &data) {
    DBerror err;
    uint32 uid = 0;

    std::string nameEsc, customInfoEsc;
    sDatabase.DoEscapeString(nameEsc, data.name);
    sDatabase.DoEscapeString(customInfoEsc, data.customInfo);

    if(!sDatabase.RunQueryLID(err, uid,
        "INSERT INTO entity ("
        "   itemName, typeID, ownerID, locationID, flag,"
        "   contraband, singleton, quantity, x, y, z,"
        "   customInfo) "
        "VALUES('%s', %u, %u, %u, %u,"
        "   %u, %u, %u, %f, %f, %f,"
        "   '%s' )",
        nameEsc.c_str(), data.typeID, data.ownerID, data.locationID, data.flag,
        data.contraband?1:0, data.singleton?1:0, data.quantity, data.position.x, data.position.y, data.position.z,
        customInfoEsc.c_str()
        )) {
        codelog(DATABASE__ERROR, "Failed to insert new entity: %s", err.c_str());
        return 0;
    }

    return uid;
}

bool InventoryDB::SaveItem(uint32 itemID, const ItemData &data) {
    // First check whether they are trying to save proper item:
    if (IsStaticMapItem(itemID)) {
        _log(ITEM__ERROR, "Refusing to modify static map object %u.", itemID);
        return false;
    }

    DBerror err;

    std::string nameEsc, customInfoEsc;
    sDatabase.DoEscapeString(nameEsc, data.name);
    sDatabase.DoEscapeString(customInfoEsc, data.customInfo);

    if(!sDatabase.RunQuery(err,
        "UPDATE entity"
        " SET"
        "  itemName = '%s',"
        "  ownerID = %u,"
        "  locationID = %u,"
        "  flag = %u,"
        "  singleton = %u,"
        "  quantity = %u,"
        "  x = %.2f, y = %.2f, z = %.2f,"
        "  customInfo = '%s'"
        " WHERE itemID = %u",
        nameEsc.c_str(),
        data.ownerID,
        data.locationID,
        (uint16)data.flag,
        (data.singleton?1:0),
        data.quantity,
        data.position.x, data.position.y, data.position.z,
        customInfoEsc.c_str(),
        itemID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s.", err.c_str());
        return false;
    }

    return true;
}

void InventoryDB::SaveItems(std::vector<SaveData>& data)
{
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO entity";
    Inserts << " (itemID, typeID, ownerID, locationID, flag, contraband, singleton, quantity, x, y, z, customInfo)";
    bool first = true;
    for (auto cur : data) {
        if (first) {
            Inserts << " VALUES ";
            first = false;
        } else
            Inserts << ", ";
        Inserts << "(" << cur.itemID << ", " << cur.typeID << ", " << cur.ownerID << ", " << cur.locationID << ", ";
        Inserts << cur.flag << ", " << cur.contraband << ", " << cur.singleton << ", ";
        Inserts << cur.quantity << ", " << cur.position.x << ", " << cur.position.y << ", " << cur.position.z << ", '" << cur.customInfo << "')";
    }

    if (!first) {
        Inserts << "ON DUPLICATE KEY UPDATE ";
        Inserts << "quantity=VALUES(quantity), ";
        Inserts << "ownerID=VALUES(ownerID), ";
        Inserts << "locationID=VALUES(locationID), ";
        Inserts << "flag=VALUES(flag), ";
        Inserts << "singleton=VALUES(singleton), ";
        Inserts << "quantity=VALUES(quantity), ";
        Inserts << "x=VALUES(x), ";
        Inserts << "y=VALUES(y), ";
        Inserts << "z=VALUES(z), ";
        Inserts << "customInfo=VALUES(customInfo) ";
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SaveItems - unable to save data - %s", err.c_str());
    }
}

void InventoryDB::SaveAttributes(bool isChar, std::vector<AttrData>& data)
{
    std::ostringstream Inserts;
    // start the insert into command.
    if (isChar) {
        DBerror err;
        sDatabase.RunQuery(err, "DELETE FROM chrCharacterAttributes WHERE charID = %u", data[0].itemID);
        Inserts << "INSERT INTO chrCharacterAttributes";
        Inserts << " (charID, attributeID, valueInt, valueFloat)";
    } else {
        Inserts << "INSERT INTO entity_attributes";
        Inserts << " (itemID, attributeID, valueInt, valueFloat)";
    }
    bool first = true;
    for (auto cur : data) {
        if (first) {
            Inserts << " VALUES ";
            first = false;
        } else
            Inserts << ", ";
        Inserts << "(" << cur.itemID << ", " << cur.attrID << ", ";
        if (cur.valueInt)
            Inserts << cur.valueInt << ", NULL)";
        else
            Inserts << "NULL, " << cur.valueFloat << ")";
    }

    if (!first) {
        Inserts << "ON DUPLICATE KEY UPDATE ";
        Inserts << "valueInt=VALUES(valueInt), ";
        Inserts << "valueFloat=VALUES(valueFloat)";
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SaveItems - unable to save data - %s", err.c_str());
    }
}


bool InventoryDB::DeleteItem(uint32 itemID) {
    if (IsStaticMapItem(itemID)) {
        _log(ITEM__ERROR, "Refusing to delete static map object %u.", itemID);
        return false;
    }

    DBerror err;
    if (!sDatabase.RunQuery(err, "DELETE FROM entity WHERE itemID=%u", itemID)) {
        codelog(DATABASE__ERROR, "Failed to delete item %u: %s", itemID, err.c_str());
        return false;
    }

    if (!sDatabase.RunQuery(err, "DELETE FROM entity_attributes WHERE itemID=%u", itemID)) {
        codelog(DATABASE__ERROR, "Failed to delete item %u: %s", itemID, err.c_str());
        return false;
    }
    return true;
}

/* this is only called by Inventory::LoadContents()
 * it is optimized for specific calling objects, to avoid multiple db hits while loading,
 * and to load only things needed for this object at the time of the call.
 */
bool InventoryDB::GetItemContents(OwnerData &od, std::vector<uint32> &into) {
    std::stringstream query;
    query << "SELECT itemID FROM entity WHERE locationID = ";
    query << od.locID;

    if (IsSolarSystem(od.locID)) {
        query << " AND ownerID = " << od.ownerID;
    } else if (IsStation(od.locID)) {
        if (od.ownerID == 1) {
            /* this will get agents in station */
            query << " AND ownerID < " << maxNPCItem;
            query << " AND itemID < " << maxNPCItem;
        } else {
            if (IsPlayerCorp(od.corpID)) {
                // check for items owned by corp in players cargo/hangar
                query << " AND ((ownerID = " << od.ownerID << ") OR (ownerID = " << od.corpID << "))";
            } else {
                query << " AND ownerID = " << od.ownerID;
            }
        }
    } else if (IsCharacter(od.locID)) {
        if (od.ownerID == 1) {
            // not sure what to do here....
        } else if (IsPlayerCorp(od.corpID)) {
            // check for items owned by corp in players cargo/hangar...this *should* work for cap ships' cargo, also
           query << " AND ((ownerID = " << od.ownerID << ") OR (ownerID = " << od.corpID << "))";
        } else {
            query << " AND ownerID = " << od.ownerID;
        }
    } else if (IsOffice(od.locID)) {
        // may not need this, as location is officeID, but items MAY be owned by players in corp hangar.
        //query << " AND ownerID = " << od.ownerID;
    }

    query << " ORDER BY itemID";

    DBQueryResult res;
    if(!sDatabase.RunQuery(res,query.str().c_str() )) {
        codelog(DATABASE__ERROR, "Error in GetItemContents query for locationID %u: %s", od.locID, res.error.c_str());
        return false;
    }

    _log(DATABASE__RESULTS, "GetItemContents: '%s' returned %u items", query.str().c_str(), res.GetRowCount());
    DBResultRow row;
    while( res.GetRow( row ) )
        into.push_back( row.GetUInt( 0 ) );

    return true;
}

/*  not used? */
bool InventoryDB::GetItemContents(uint32 itemID, EVEItemFlags flag, std::vector<uint32> &into)
{
    DBQueryResult res;

    if( !sDatabase.RunQuery( res,
        "SELECT "
        "  itemID"
        " FROM entity "
        " WHERE locationID=%u"
        "  AND flag=%d",
        itemID, (int)flag ) )
    {
        codelog(DATABASE__ERROR, "Error in GetItemContents query for item %u: %s", itemID, res.error.c_str());
        return false;
    }

    _log(DATABASE__RESULTS, "GetItemContents for item %u returned %u items", itemID, res.GetRowCount());
    DBResultRow row;
    while( res.GetRow( row ) )
        into.push_back( row.GetUInt( 0 ) );

    return true;
}

/*  not used? */
bool InventoryDB::GetItemContents(uint32 itemID, EVEItemFlags flag, uint32 ownerID, std::vector<uint32> &into)
{
    DBQueryResult res;

    if( !sDatabase.RunQuery( res,
        "SELECT "
        "  itemID"
        " FROM entity "
        " WHERE locationID=%u"
        "  AND flag=%d"
        "  AND ownerID=%u",
        itemID, (int)flag, ownerID ) )
    {
        codelog(DATABASE__ERROR, "Error in GetItemContents query for item %u with flag %u: %s", itemID, (int)flag, res.error.c_str());
        return false;
    }

    _log(DATABASE__RESULTS, "GetItemContents for item %u with flag %u returned %u items", itemID, flag, res.GetRowCount());
    DBResultRow row;
    while( res.GetRow( row ) )
        into.push_back( row.GetUInt( 0 ) );

    return true;
}

void InventoryDB::DeleteTrackingCans()
{
    DBerror err;
    std::string query = "'%Position Test%'";
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE customInfo LIKE %s", query.c_str());
    query = "'%Bubble%'";
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE customInfo LIKE %s", query.c_str());
}

bool InventoryDB::GetCharacterData(uint32 characterID, CharacterData &into) {
    DBQueryResult res;

    if (IsAgent(characterID)) {
        if(!sDatabase.RunQuery(res,
            "SELECT"
            "   0 as accountID,"
            "   title,"
            "   description,"
            "   gender,"
            "   bounty,"
            "   0 as balance,"
            "   0 as aurBalance,"
            "   securityRating,"
            "   0 as logonMinutes,"
            "   stationID,"
            "   solarSystemID,"
            "   constellationID,"
            "   regionID,"
            "   ancestryID,"
            "   0 AS bloodlineID,"      /** @todo fix these */
            "   0 AS raceID,"
            "   careerID,"
            "   schoolID,"
            "   careerSpecialityID,"
            "   createDateTime,"
            "   0 as shipID,"       // update this when agents are in space
            "   0 as capsuleID,"
            "   flag,"
            "   name,"
            "   0 AS skillPoints,"
            "   typeID"
			" FROM chrNPCCharacters AS chr"
            " WHERE characterID = %u", characterID)) {
            codelog(DATABASE__ERROR, "Error in GetCharacter query: %s", res.error.c_str());
            return false;
            }
    } else {
        if(!sDatabase.RunQuery(res,
            "SELECT"
            "   accountID,"
            "   title,"
            "   description,"
            "   gender,"
            "   bounty,"
            "   balance,"
            "   aurBalance,"
            "   securityRating,"
            "   logonMinutes,"
            "   stationID,"
            "   solarSystemID,"
            "   constellationID,"
            "   regionID,"
            "   ancestryID,"
            "   bloodlineID,"
            "   raceID,"
            "   careerID,"
            "   schoolID,"
            "   careerSpecialityID,"
            "   createDateTime,"
            "   shipID,"
            "   capsuleID,"
            "   flag,"
            "   name,"
            "   skillPoints,"
            "   typeID"
            " FROM chrCharacters"
            " WHERE characterID = %u", characterID))
        {
            codelog(DATABASE__ERROR, "Error in GetCharacter query: %s", res.error.c_str());
            return false;
        }
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "No data found for character %u.", characterID);
        return false;
    }

    into.accountID = row.IsNull( 0 ) ? 0 : row.GetUInt( 0 );
    into.title = row.GetText( 1 );
    into.description = row.GetText( 2 );
    into.gender = row.GetUInt( 3 ) ? true : false;
    into.bounty = row.GetDouble( 4 );
    into.balance = row.GetDouble( 5 );
    into.aurBalance = row.GetDouble( 6 );
    into.securityRating = row.GetDouble( 7 );
    into.logonMinutes = row.GetUInt( 8 );
    into.stationID = row.GetUInt( 9 );
    into.solarSystemID = row.GetUInt( 10 );
    into.locationID = (into.stationID == 0 ? into.solarSystemID : into.stationID);
    into.constellationID = row.GetUInt( 11 );
    into.regionID = row.GetUInt( 12 );
    into.ancestryID = row.GetUInt( 13 );
    into.bloodlineID =  row.GetUInt( 14 );
    into.raceID = row.GetUInt( 15 );
    into.careerID =row.GetUInt( 16 );
    into.schoolID = row.GetUInt( 17 );
    into.careerSpecialityID = row.GetUInt( 18 );
    into.createDateTime = row.GetInt64( 19 );
    into.shipID = row.GetUInt( 20 );
    into.capsuleID = row.GetUInt( 21 );
    into.flag = row.GetUInt(22);
    into.name = row.GetText(23);
    into.skillPoints = row.GetDouble(24);
    into.typeID = row.GetUInt(25);

    return true;
}

bool InventoryDB::GetCorpData(uint32 characterID, CorpData &into) {
    DBQueryResult res;
    DBResultRow row;

    if (IsAgent(characterID)) {
        into.corpAccountKey = 1001;
        into.corpRole = 0;
        into.rolesAtAll = 0;
        into.rolesAtBase = 0;
        into.rolesAtHQ = 0;
        into.rolesAtOther = 0;
        into.grantableRoles = 0;
        into.grantableRolesAtBase = 0;
        into.grantableRolesAtHQ = 0;
        into.grantableRolesAtOther = 0;
        into.startDateTime = 0;

        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  corporationID, locationID"
            " FROM agtAgents AS a"
            " WHERE agentID = %u",
            characterID))
        {
            codelog(DATABASE__ERROR, "Failed to query corp member info of character %u: %s.", characterID, res.error.c_str());
            return false;
        }

        if (!res.GetRow(row)) {
            _log(DATABASE__MESSAGE, "No corp member info found for character %u.", characterID);
            return false;
        }
        into.corporationID = row.GetInt(0);
        into.baseID = row.GetInt(1);
    } else {
        if (!sDatabase.RunQuery(res,
            "SELECT"
            "  startDateTime,"
            "  corporationID,"
            "  corpAccountKey,"
            "  corpRole,"
            "  rolesAtAll,"
            "  rolesAtBase,"
            "  rolesAtHQ,"
            "  rolesAtOther,"
            "  grantableRoles,"
            "  grantableRolesAtBase,"
            "  grantableRolesAtHQ,"
            "  grantableRolesAtOther,"
            "  baseID"
            " FROM chrCharacters"
            " WHERE characterID = %u",
            characterID))
        {
            codelog(DATABASE__ERROR, "Failed to query corp member info of character %u: %s.", characterID, res.error.c_str());
            return false;
        }
        if (!res.GetRow(row)) {
            _log(DATABASE__MESSAGE, "No corp member info found for character %u.", characterID);
            return false;
        }

        into.startDateTime = row.GetInt64(0);
        into.corporationID = row.GetInt(1);
        into.corpAccountKey = row.GetInt(2);
        into.corpRole = row.GetInt64(3);
        into.rolesAtAll = row.GetInt64(4);
        into.rolesAtBase = row.GetInt64(5);
        into.rolesAtHQ = row.GetInt64(6);
        into.rolesAtOther = row.GetInt64(7);
        into.grantableRoles = row.GetInt64(8);
        into.grantableRolesAtBase = row.GetInt64(9);
        into.grantableRolesAtHQ = row.GetInt64(10);
        into.grantableRolesAtOther = row.GetInt64(11);
        into.baseID = row.GetInt(12);
    }

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  taxRate,"
        "  stationID,"
        "  allianceID,"
        "  warFactionID,"
        "  corporationName,"
        "  tickerName"
        " FROM crpCorporation"
        " WHERE corporationID = %u", into.corporationID))
    {
        codelog(DATABASE__ERROR, "Failed to query HQ of character's %u corporation %u: %s.", characterID, into.corporationID, res.error.c_str());
        return false;
    }

    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "No HQ found for character's %u corporation.", characterID);
        return false;
    }

    into.taxRate = row.GetDouble(0);
    into.corpHQ = (row.IsNull(1) ? 0 : row.GetUInt(1));
    into.allianceID = (row.IsNull(2) ? 0 : row.GetUInt(2));
    into.warFactionID = (row.IsNull(3) ? 0 : row.GetUInt(3));
    into.name = row.GetText(4);
    into.ticker = row.GetText(5);

    return true;
}
/*
 * This macro checks given CharacterAppearance object (app) if given value (v) is NULL:
 *  if yes, macro evaulates to string "NULL"
 *  if no, macro evaulates to call to function _ToStr, which turns given value to string.
 *
 * This macro is needed when saving CharacterAppearance values into DB (NewCharacter, SaveCharacterAppearance).
 * Resulting value must be const char *.
 */
#define _VoN(app, v) \
    ((const char *)(app.IsNull_##v() ? "NULL" : _ToStr(app.Get_##v()).c_str()))

/* a 32 bits unsigned integer can be max 0xFFFFFFFF.
   this results in a text string: '4294967295' which
   is 10 long. Including the '\0' at the end of the
   string it is max 11.
 */
static std::string _ToStr(uint32 v) {
    char buf[11];
    snprintf(buf, 11, "%u", v);
    return(buf);
}

static std::string _ToStr(double v) {
    char buf[32];
    snprintf(buf, 32, "%.13f", v);
    return(buf);
}

// Undefine the macro used only in SaveCharacterAppearance.
#undef _VoN

bool InventoryDB::GetCelestialObject(uint32 celestialID, CelestialObjectData &into) {
    DBQueryResult res;

    if( IsStaticMapItem(celestialID)) {
        // This Celestial object is a static celestial, so get its data from the 'mapDenormalize' table:
        if(!sDatabase.RunQuery(res,
            "SELECT"
            "  security, radius, celestialIndex, orbitIndex"
            " FROM mapDenormalize"
            " WHERE itemID = %u",
            celestialID))
        {
            codelog(DATABASE__ERROR, "Failed to query celestial object %u: %s.", celestialID, res.error.c_str());
            return false;
        }

        DBResultRow row;
        if(!res.GetRow(row)) {
            _log(DATABASE__MESSAGE, "Static Celestial object %u not found.", celestialID);
            return false;
        }

        into.security = (row.IsNull(0) ? 0 : row.GetDouble(0));
        into.radius = row.GetDouble(1);
        into.celestialIndex = (row.IsNull(2) ? 0 : row.GetUInt(2));
        into.orbitIndex = (row.IsNull(3) ? 0 : row.GetUInt(3));
    } else {
        // Quite possibly, this Celestial object is a dynamic one, so try to get its data from the 'entity' table,
        // and if it's not there either, then flag an error.
        if(!sDatabase.RunQuery(res,
            "SELECT"
            "  entity.itemID, "
            "  invTypes.radius "
            " FROM entity "
            "  LEFT JOIN invTypes USING (typeID)"
            " WHERE entity.itemID = %u",
            celestialID))
        {
            codelog(DATABASE__ERROR, "Failed to query celestial object %u: %s.", celestialID, res.error.c_str());
            return false;
        }

        DBResultRow row;
        if(!res.GetRow(row)) {
            _log(DATABASE__MESSAGE, "Dynamic Celestial object %u not found.", celestialID);
            return false;
        }

        into.security = 1.0;
        into.radius = (row.IsNull(1) ? 1 : row.GetDouble(1));
        into.celestialIndex = 0;
        into.orbitIndex = 0;
    }

    return true;
}

bool InventoryDB::GetSolarSystem(uint32 solarSystemID, SolarSystemData &into) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  xMin, yMin, zMin,"
        "  xMax, yMax, zMax,"
        "  luminosity,"
        "  border, fringe, corridor, hub, international, regional, constellation,"
        "  security, factionID, radius, sunTypeID, securityClass"
        " FROM mapSolarSystems"
        " WHERE solarSystemID=%u", solarSystemID))
    {
        codelog(DATABASE__ERROR, "Error in GetSolarSystem query for system %u: %s.", solarSystemID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "No data found for solar system %u.", solarSystemID);
        return false;
    }

    into.minPosition = GPoint(row.GetDouble(0), row.GetDouble(1), row.GetDouble(2));
    into.maxPosition = GPoint(row.GetDouble(3), row.GetDouble(4), row.GetDouble(5));
    into.luminosity = row.GetDouble(6);

    into.border = (row.GetInt(7) ? true : false);
    into.fringe = (row.GetInt(8) ? true : false);
    into.corridor = (row.GetInt(9) ? true : false);
    into.hub = (row.GetInt(10) ? true : false);
    into.international = (row.GetInt(11) ? true : false);
    into.regional = (row.GetInt(12) ? true : false);
    into.constellation = (row.GetInt(13) ? true : false);

    into.security = row.GetDouble(14);
    into.factionID = (row.IsNull(15) ? 0 : row.GetUInt(15));
    into.radius = row.GetDouble(16);
    into.sunTypeID = row.GetUInt(17);
    into.securityClass = (row.IsNull(18) ? (std::string("")) : row.GetText(18));

    return true;
}

bool InventoryDB::GetModulePowerSlotByTypeID(uint32 typeID, uint32 &into)
{
    /** @todo only used by gmcommands.  update and remove. */
    DBQueryResult res;
    DBResultRow row;

    if(!sDatabase.RunQuery(res,
        " SELECT "
        "  groupID "
        " FROM invTypes "
        " WHERE typeID = '%u' ",
        typeID))
    {
        codelog(DATABASE__ERROR, "Failed to get groupID for typeID = %u", typeID);
    }

    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "Item of type %u not found.", typeID);
        return false;
    }

    uint32 groupID = row.GetUInt(0);

    //TODO: put in invCat
    switch( groupID) {
        case EVEDB::invGroups::Rig_Armor:
        case EVEDB::invGroups::Rig_Astronautic:
        case EVEDB::invGroups::Rig_Drones:
        case EVEDB::invGroups::Rig_Electronics:
        case EVEDB::invGroups::Rig_Electronics_Superiority:
        case EVEDB::invGroups::Rig_Energy_Grid:
        case EVEDB::invGroups::Rig_Energy_Weapon:
        case EVEDB::invGroups::Rig_Hybrid_Weapon:
        case EVEDB::invGroups::Rig_Launcher:
        case EVEDB::invGroups::Rig_Mining:
        case EVEDB::invGroups::Rig_Projectile_Weapon:
        case EVEDB::invGroups::Rig_Security_Transponder:
        case EVEDB::invGroups::Rig_Shield:

            into = 0;
            return true;
    }

    if(!sDatabase.RunQuery(res,
        " SELECT "
        "  effectID "
        " FROM dgmTypeEffects "
        " WHERE typeID = '%u' AND ( effectID = 11 OR effectID = 12 OR effectID = 13 ) ",
        typeID))
    {
        codelog(DATABASE__ERROR, "Failed to get slot for typeID = %u", typeID);
    }

    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "Item of type %u not found.", typeID);
        return false;
    }

    uint32 slotType = row.GetUInt(0);

    //such crap...
    if( slotType == 11 ) {
        into = 1;
        return true;
    } else if( slotType == 12 ) {
        into = 3;
        return true;
    } else if( slotType == 13 ){
        into = 2;
        return true;
    } else
        return false;
}

bool InventoryDB::GetOpenPowerSlots(uint32 slotType, ShipItemRef ship, uint32 &into)
{
    /** @todo only used by gmcommands.  update and remove. */
    DBQueryResult res;
    uint32 attributeID = 0;
    uint32 firstFlag;
    DBResultRow row;
    uint32 slotsOnShip;

    if( slotType == 0 )
    {
        attributeID = 1137;
        firstFlag = 92; //rigslot0
        //slotsOnShip = ship->rigSlots();
        slotsOnShip = static_cast<uint32>(ship->GetAttribute(AttrRigSlots).get_int());
    }
    else if( slotType == 1 )
    {
        attributeID = 12;
        firstFlag = 11; //lowslot0
        //slotsOnShip = ship->lowSlots();
        slotsOnShip = static_cast<uint32>(ship->GetAttribute(AttrLowSlots).get_int());
    }
    else if( slotType == 2 )
    {
        attributeID = 13;
        firstFlag = 19; //medslot0
        //slotsOnShip = ship->medSlots();
        slotsOnShip = static_cast<uint32>(ship->GetAttribute(AttrMedSlots).get_int());
    }
    else if( slotType == 3 )
    {
        attributeID = 14;
        firstFlag = 27; //hislot0
        //slotsOnShip = ship->hiSlots();
        slotsOnShip = static_cast<uint32>(ship->GetAttribute(AttrHiSlots).get_int());
    }

    for( uint32 flag = firstFlag; flag < (firstFlag + slotsOnShip); flag++ ) {
        // this is far from efficient as we are iterating through all of the ships item slots.... every iteration... so this will be slow when you got loads of players with a single free slot.
        if(ship->GetMyInventory()->IsEmptyByFlag((EVEItemFlags)flag)) {
            into = flag;
            return true;
        }
    }

    //Only time it should make it this far...
    if (ship->HasPilot())
        if (ship->GetPilot()->CanThrow())
            throw PyException( MakeCustomError( "There are no available slots" ));

    return false;

}
