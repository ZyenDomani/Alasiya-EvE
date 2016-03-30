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
    Author:        Allan, Comet0
*/

#include "eve-server.h"

#include "planet/PlanetDB.h"


PyRep* PlanetDB::GetPlanetInfo(uint32 planetID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT `solarSystemID`, `typeID` AS `planetTypeID`, `itemID` AS `planetID`, `radius`"
                " FROM mapDenormalize WHERE `itemID` = %u", planetID)) {
        codelog(SERVICE__ERROR, "Error in GetPlanetInfo query: %s", res.error.c_str());
        return NULL;
    }
    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(SERVICE__ERROR, "Error in GetPlanetInfo query, failed to get row");
        return NULL;
    }
    return DBRowToKeyVal(row);
}

PyRep* PlanetDB::GetPlanetsForChar(uint32 charID) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT `solarSystemID`, `planetID`, `typeID`, `numberOfPins`"
         " FROM `chrPlanets` WHERE `characterID` = %u", charID)) {
        codelog(SERVICE__ERROR, "Error in GetPlanetsForChar Query: %s", res.error.c_str());
        return NULL;
    }
    return DBResultToCRowset(res);
}

PyRep* PlanetDB::GetPlanetResourceInfo(uint32 planetID) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT `itemID1`, `itemID2`, `itemID3`, `itemID4`, `itemID5`,"
        " `quality1`, `quality2`, `quality3`, `quality4`, `quality5`"
        " FROM `planetResourceInfo` WHERE `planetID` = %u", planetID)) {
        codelog(SERVICE__ERROR, "Error in GetPlanetResourceInfo Query: %s", res.error.c_str());
        return NULL;
    }
    DBResultRow row;
    if(!res.GetRow(row)) {
        codelog(SERVICE__ERROR, "Error in GetPlanetResourceInfo Query, Returned 0 rows.");
        return NULL;
    }
    PyDict *rtn = new PyDict();
        rtn->SetItem(new PyInt(row.GetInt(0)), new PyFloat(row.GetFloat(5)));
        rtn->SetItem(new PyInt(row.GetInt(1)), new PyFloat(row.GetFloat(6)));
        rtn->SetItem(new PyInt(row.GetInt(2)), new PyFloat(row.GetFloat(7)));
        rtn->SetItem(new PyInt(row.GetInt(3)), new PyFloat(row.GetFloat(8)));
        rtn->SetItem(new PyInt(row.GetInt(4)), new PyFloat(row.GetFloat(9)));
    return rtn;
}

PyRep* PlanetDB::GetMyLaunchesDetails(uint32 charID) {
    //TODO, double check if this requires x,y,z, or if only Beyonce uses them.
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT `solarSystemID`, `planetID`, `launchTime`, `launchID`, `x`, `y`, `z`"
        " FROM `chrPlanetLaunches` WHERE `characterID` = %u", charID)) {
        codelog(SERVICE__ERROR, "Error in GetMyLaunchesDetails Query: %s", res.error.c_str());
        return NULL;
    }
    return DBResultToRowset(res);
}


PyRep* PlanetDB::GetExtractorsForPlanet(uint32 planetID) {
    /* Incomplete, Needs to retrieve data from tables that do not exist yet.
     * Currently stops the client from throwing errors.
     */
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT 2130 AS `typeID`, 0 as `ownerID`")) {
        codelog(SERVICE__ERROR, "Error in GetExtractorsForPlanet Query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

bool PlanetDB::GetResourceData(uint32 planetID, DBResultRow &row)
{
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT `itemID1`, `itemID2`, `itemID3`, `itemID4`, `itemID5`,"
        " `data1`, `data2`, `data3`, `data4`, `data5`,"
        " `numBands1`, `numBands2`, `numBands3`, `numBands4`, `numBands5`"
        " FROM `planetResourceInfo` WHERE `planetID` = %u", planetID)) {
        codelog(SERVICE__ERROR, "Error in GetResourceData Query: %s", res.error.c_str());
        return false;
    }

    if(!res.GetRow(row)) {
        codelog(SERVICE__ERROR, "Error in GetResourceData Query failed to get row.");
        return false;
    }

    return true;
}
