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

#include "config/ConfigDB.h"

PyRep *ConfigDB::GetMultiOwnersEx(const std::vector<int32> &entityIDs) {
    // separate list of ids into respective groups
    std::vector<int32> player, corp, ally;
    player.clear();
    corp.clear();
    ally.clear();

    for (auto cur : entityIDs) {
        if (IsCorp(cur))
            corp.push_back(cur);
        else if (IsAlliance(cur))
            ally.push_back(cur);
        else
            player.push_back(cur);
    }

    DBQueryResult res;
    std::string ids = "";

    if (corp.size()) {
        ListToINString(corp, ids, "0");

        if (!sDatabase.RunQuery(res,
            "SELECT "
            "  corporationID as ownerID,"
            "  corporationName as ownerName,"
            "  2 AS typeID,"                    // corp typeID
            "  NULL AS gender,"
            "  NULL AS ownerNameID"
            " FROM corporation"
            " WHERE corporationID IN (%s)", ids.c_str()))
        {
            codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        }
        ids = "";
    }

    if (ally.size()) {
        ListToINString(ally, ids, "0");

        if (!sDatabase.RunQuery(res,
            "SELECT "
            "  allianceID as ownerID,"
            "  allianceShortName as ownerName,"
            "  16159 AS typeID,"                 // alliance typeID.
            "  NULL AS gender,"
            "  NULL AS ownerNameID"
            " FROM crpAlliance"
            " WHERE allianceID IN (%s)", ids.c_str()))
        {
            codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        }
        ids = "";
    }

    if (player.size()) {
        ListToINString(player, ids, "0");

        if (!sDatabase.RunQuery(res,
            "SELECT "
            "  c.characterID as ownerID,"
            "  e.itemName as ownerName,"
            "  e.typeID,"
            "  c.gender,"
            "  NULL AS ownerNameID"
            " FROM character_ AS c"
            "  LEFT JOIN entity AS e ON e.itemID = c.characterID"
            " WHERE characterID IN (%s)", ids.c_str()))
        {
            codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        }
    }

    return DBResultToTupleSet(res);
}

PyRep *ConfigDB::GetMultiAllianceShortNamesEx(const std::vector<int32> &entityIDs) {
    std::string ids;
    ListToINString(entityIDs, ids, "0");

    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT allianceID, allianceShortName FROM crpAlliance"
        ))
    {
        codelog(DATABASE__ERROR, "Error in GetMultiAllianceShortNamesEx query: %s", res.error.c_str());
        return new PyInt(0);
    }

    return DBResultToTupleSet(res);
}


PyRep *ConfigDB::GetMultiLocationsEx(const std::vector<int32> &entityIDs) {
    // separate list of ids into respective groups
    std::vector<int32> staticItem, dynamicItem;
    staticItem.clear();
    dynamicItem.clear();

    for (auto cur : entityIDs) {
        if (IsStaticMapItem(cur) or (cur == 0))
            staticItem.push_back(cur);
        else
            dynamicItem.push_back(cur);
    }

    DBQueryResult res;
    std::string ids = "";

    if (staticItem.size()) {
        ListToINString(staticItem, ids, "0");
        if (!sDatabase.RunQuery(res,
            "SELECT "
            " itemID AS locationID,"
            " itemName AS locationName,"
            " x, y, z,"
            " NULL AS locationNameID"
            " FROM mapDenormalize"
            " WHERE itemID in (%s)", ids.c_str()))
        {
            codelog(DATABASE__ERROR, "Error in GetMultiLocationsEx query: %s", res.error.c_str());
        }
        ids = "";
    }

    if (dynamicItem.size()) {
        ListToINString(dynamicItem, ids, "0");
        if (!sDatabase.RunQuery(res,
                "SELECT "
                " itemID AS locationID,"
                " itemName AS locationName,"
                " x, y, z,"
                " NULL AS locationNameID"
                " FROM entity"
                " WHERE itemID in (%s)", ids.c_str()))
        {
            codelog(DATABASE__ERROR, "Error in GetMultiLocationsEx query: %s", res.error.c_str());
        }
    }

    return DBResultToTupleSet(res);
}

PyRep *ConfigDB::GetMultiCorpTickerNamesEx(const std::vector<int32> &entityIDs) {

    std::string ids;
    ListToINString(entityIDs, ids, "0");

    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT "
        "   corporationID, tickerName,"
        "   shape1, shape2, shape3,"
        "   color1, color2, color3 "
        " FROM corporation"
        " WHERE corporationID in (%s)", ids.c_str()))
    {
        codelog(DATABASE__ERROR, "Error in GetMultiCorpTickerNamesEx query: %s", res.error.c_str());
        return new PyInt(0);
    }

    //return DBResultToRowList(res);
    //return DBResultToPackedRowList(res);
    return DBResultToPackedRowListTuple(res);
}


PyRep *ConfigDB::GetMultiGraphicsEx(const std::vector<int32> &entityIDs) {

    std::string ids;
    ListToINString(entityIDs, ids, "0");

    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        "   graphicID, url3D, urlWeb, icon, urlSound, explosionID"
        " FROM eveGraphics "
        " WHERE graphicID in (%s)", ids.c_str()))
    {
        codelog(DATABASE__ERROR, "Error in GetMultiGraphicsEx query: %s", res.error.c_str());
        return new PyInt(0);
    }

    return DBResultToRowList(res);
}

PyObject *ConfigDB::GetUnits() {

    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT "
        "   unitID, unitName, displayName"
        " FROM eveUnits "))
    {
        codelog(DATABASE__ERROR, "Error in GetUnits query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToIndexRowset(res, "unitID");
}

PyObjectEx *ConfigDB::GetMapObjects(uint32 entityID, bool wantRegions,
    bool wantConstellations, bool wantSystems, bool wantStations)
{       //  corrected query  -allan 8Dec14
    const char *key = "solarSystemID";
    if (wantRegions) {
        entityID = 3;   /* a little hackish... */
        key = "typeID";
    } else if (wantConstellations) {
        key = "regionID";
    } else if (wantSystems) {
        key = "constellationID";
    } else if (wantStations) {
        key = "solarSystemID";
    }

    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT "
        "   groupID, typeID, itemID, itemName,"
        "   solarSystemID AS locationID, IFNULL(orbitID, 0) AS orbitID,"
        "   x, y, z"
        " FROM mapDenormalize"
        " WHERE %s=%u", key, entityID )) {
        codelog(DATABASE__ERROR, "Error in GetMapObjects query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

PyObject *ConfigDB::GetMap(uint32 solarSystemID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT "
        "   s.solarSystemID AS locationID,"
        "   s.xMin, s.xMax, s.yMin,"
        "   s.yMax, s.zMin, s.zMax,"
        "   s.luminosity,"
        "   d.x, d.y, d.z,"
        "   d.itemID,"
        "   d.itemName,"
        "   d.typeID,"
        "   d.orbitID,"
        "   j.celestialID AS destinations"
        " FROM mapSolarSystems AS s"
        "  LEFT JOIN mapDenormalize AS d USING (solarSystemID)"
        "  LEFT JOIN mapJumps AS j ON j.stargateID = d.itemID"
        " WHERE solarSystemID=%u", solarSystemID
    )) {
        codelog(DATABASE__ERROR, "Error in GetMap query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

PyObject *ConfigDB::ListLanguages() {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT "
        "   languageID,languageName,translatedLanguageName"
        " FROM languages"
    )) {
        codelog(DATABASE__ERROR, "Error in ListLanguages query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}


PyRep *ConfigDB::GetMultiInvTypesEx(const std::vector<int32> &entityIDs) {

    std::string ids;
    ListToINString(entityIDs, ids, "0");

    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        "   typeID,groupID,typeName,description,graphicID,radius,"
        "   mass,volume,capacity,portionSize,raceID,basePrice,"
        "   published,marketGroupID,chanceOfDuplicating "
        " FROM invTypes "
        " WHERE typeID in (%s)", ids.c_str()))
    {
        codelog(DATABASE__ERROR, "Error in GetMultiInvTypesEx query: %s", res.error.c_str());
        return new PyInt(0);
    }

    return DBResultToRowList(res);
}

PyRep *ConfigDB::GetStationSolarSystemsByOwner(uint32 ownerID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        " SELECT "
        " stationID, solarSystemID "
        " FROM staStations "
        " WHERE corporationID = %u ", ownerID
        ))
    {
        codelog(DATABASE__ERROR, "Error in GetStationSolarSystemsByOwner query: %s", res.error.c_str());
        return new PyInt(0);
    }

    return DBResultToRowset(res);
}

PyRep *ConfigDB::GetCelestialStatistic(uint32 celestialID) {
    //  corrected db query  -allan 8Dec14
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "   celestialID,"
        "   temperature,"
        "   spectralClass,"
        "   luminosity,"
        "   age,"
        "   life,"
        "   orbitRadius,"
        "   eccentricity,"
        "   massDust,"
        "   massGas,"
        "   fragmented,"
        "   density,"
        "   surfaceGravity,"
        "   escapeVelocity,"
        "   orbitPeriod,"
        "   rotationRate,"
        "   locked,"
        "   pressure,"
        "   radius,"
        "   mass"
        " FROM mapCelestialStatistics"
        " WHERE celestialID = %u", celestialID))
        {
            codelog(DATABASE__ERROR, "Error in GetCelestialStatistic query: %s", res.error.c_str());
            return new PyInt(0);
        }

    return DBResultToCRowset(res);
}

PyRep *ConfigDB::GetDynamicCelestials(uint32 solarSystemID) {
    //  corrected query and return type per packet info
    //      this returns ONLY OUTPOSTS.  -allan 8Dec14
    /* this packet is from a crowded (73 items) system (including pos'), yet is only return from this call.
              [PyObjectEx Type2]
                [PyTuple 2 items]
                  [PyTuple 1 items]
                    [PyToken dbutil.CRowset]
                  [PyDict 1 kvp]
                    [PyString "header"]
                [PyPackedRow 50 bytes]
                  ["groupID" => <15> [I2]]
                  ["typeID" => <21645> [I4]]    <-- Gallente Administrative Outpost
                  ["itemID" => <61000562> [I4]]
                  ["itemName" => <5E-EZC VI - X333 GEORGIOU'S PLACE> [WStr]]
                  ["locationID" => <30004168> [I4]]
                  ["orbitID" => <40264214> [I4]]
                  ["connector" => <0> [I4]]
                  ["x" => <-435188244480> [R8]]
                  ["y" => <27742740480> [R8]]
                  ["z" => <430524948480> [R8]]
                  ["celestialIndex" => <6> [UI1]]
                  ["orbitIndex" => <0> [UI1]]
                  */

    DBQueryResult result;

    if (!sDatabase.RunQuery(result,
        "SELECT"
        "   groupID,"
        "   typeID,"
        "   itemID,"
        "   itemName,"
        "   solarSystemID AS locationID,"
        "   IFNULL(orbitID, 0) AS orbitID,"
        "   0 AS connector,"     //this is connector field....have only seen 0 in server packets.
        "   x, y, z,"
        "   celestialIndex,"    //  this index denotes which planet this item orbits (PLANET #....NOT moon #)
        "   orbitIndex"         //  this index denotes which asteroid belt this item belongs to
        " FROM mapDenormalize"
        " WHERE solarSystemID = %u"
        " AND groupID = %d"
        " AND itemID > %d",   //this is min itemid for outposts   (pos' do NOT go here)
        solarSystemID, EVEDB::invGroups::Station, maxNPCStation )) {
        codelog(DATABASE__ERROR, "Error in GetDynamicCelestials query: %s", result.error.c_str());
            return new PyInt(0);
    }

    return DBResultToCRowset(result);
}

PyRep *ConfigDB::GetTextsForGroup(const std::string & langID, uint32 textgroup) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT textLabel, `text` FROM intro WHERE langID = '%s' AND textgroup = %u", langID.c_str(), textgroup))
    {
        codelog(DATABASE__ERROR, "Error in GetTextsForGroup query: %s", res.error.c_str());
        return new PyInt(0);
    }

    return DBResultToRowset(res);
}

PyObject *ConfigDB::GetMapOffices(uint32 solarSystemID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT "
        "  corporationID,"
        "  stationID"
        " FROM staStations"
        " WHERE solarSystemID=%u", solarSystemID
    ))
    {
        codelog(DATABASE__ERROR, "Error in GetMapOffices query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

PyObject *ConfigDB::GetMapConnections(uint32 id, bool sol, bool reg, bool con, uint16 cel, uint16 _c) {
  sLog.Warning ("ConfigDB::GetMapConnections", "DB query - System:%u, Sol:%u, Reg:%u, Con:%u, Cel:%u, _c:%u", id, sol, reg, con, cel, _c);

    const char *key = "fromsol";
    if (sol) key = "fromsol";
    else if (reg) key = "fromreg";
    else if (con) key = "fromcon";
    else sLog.Error ("ConfigDB::GetMapConnections()", "Bad argument (id: %u, sol: %u, reg: %u, con: %u) passed to key.", id, sol, reg, con );

    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT ctype, fromreg, fromcon, fromsol, stargateID, celestialID, tosol, tocon, toreg"
        " FROM mapConnections"
        " WHERE %s = %u", key, id )) {
        codelog(DATABASE__ERROR, "Error in GetMapConnections query: %s", res.error.c_str());
            sLog.Error ("ConfigDB::GetMapConnections()", "No Data for key: %s, id: %u.", key, id);
            return nullptr;
    }
    return DBResultToRowset(res);
}

PyObject *ConfigDB::GetMapLandmarks() {    // working 29June14
    DBQueryResult res;

      if (!sDatabase.RunQuery(res,
          "SELECT"
          "   landmarkID,"
          "   landmarkName,"
          "   0 AS landmarkNameID,"
          "   x, y, z,"
          "   radius"
          " FROM mapLandmarks" ))
      {
          codelog(DATABASE__ERROR, "Error in GetMapLandmarks query: %s", res.error.c_str());
          return nullptr;
      }
    return DBResultToRowset(res);
}

