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
    into.published = row.GetBool(2);

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
    into.useBasePrice = row.GetBool(3);
    into.allowManufacture = row.GetBool(4);
    into.allowRecycler = row.GetBool(5);
    into.anchored = row.GetBool(6);
    into.anchorable = row.GetBool(7);
    into.fittableNonSingleton = row.GetBool(8);
    into.published = row.GetBool(9);

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
	into.basePrice = row.GetDouble(9);
    into.published = (sConfig.server.AllowNonPublished ? true : row.GetBool(10));
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
    if(!sDatabase.RunQuery(res, "SELECT typeID FROM bloodlineTypes WHERE bloodlineID = %u", bloodlineID)) {
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
    if(!sDatabase.RunQuery(res, "SELECT bloodlineID FROM bloodlineTypes WHERE typeID = %u", characterTypeID)) {
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
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE customInfo LIKE '%Position Test%'");  // 90.63s on main, 0.037s on dev
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE customInfo LIKE '%Bubble%'");         // 66.75s on main, 0.036s on dev
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
    into.gender = row.GetInt( 3 ) ? true : false;
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
    into.skillPoints = row.GetUInt(24);
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
            "SELECT corporationID, locationID"
            " FROM agtAgents"
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
    return buf;
}

static std::string _ToStr(double v) {
    char buf[32];
    snprintf(buf, 32, "%.13f", v);
    return buf;
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

    into.border = row.GetBool(7);
    into.fringe = row.GetBool(8);
    into.corridor = row.GetBool(9);
    into.hub = row.GetBool(10);
    into.international = row.GetBool(11);
    into.regional = row.GetBool(12);
    into.constellation = row.GetBool(13);

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
    uint32 attributeID = 0, firstFlag = 0;
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
            throw PyException(MakeCustomError("There are no available slots" ));

    return false;
}
