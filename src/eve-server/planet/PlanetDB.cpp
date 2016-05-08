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
              if (!sDatabase.RunQuery(res, "SELECT solarSystemID, typeID AS planetTypeID, %u, radius"
                  " FROM mapDenormalize WHERE itemID = %u", planetID, planetID)) {
        _log(DATABASE__ERROR, "Error in GetPlanetInfo query: %s", res.error.c_str());
        return NULL;
    }
    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "GetPlanetInfo failed to get row");
        return NULL;
    }
    return DBRowToKeyVal(row);
}

PyRep* PlanetDB::GetPlanetsForChar(uint32 charID) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT solarSystemID, planetID, typeID, numberOfPins"
         " FROM chrPlanets WHERE characterID = %u", charID)) {
        _log(DATABASE__ERROR, "Error in GetPlanetsForChar query: %s", res.error.c_str());
        return NULL;
         }
         _log(DATABASE__RESULTS, "GetPlanetsForChar returned %u items", res.GetRowCount());
    return DBResultToCRowset(res);
}

PyRep* PlanetDB::GetPlanetResourceInfo(uint32 planetID) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT itemID1, itemID2, itemID3, itemID4, itemID5,"
        " quality1, quality2, quality3, quality4, quality5"
        " FROM planetResourceInfo WHERE planetID = %u", planetID)) {
        _log(DATABASE__ERROR, "Error in GetPlanetResourceInfo query: %s", res.error.c_str());
        return NULL;
        }
        _log(DATABASE__RESULTS, "GetPlanetResourceInfo returned %u items", res.GetRowCount());
    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "GetPlanetResourceInfo failed to get row.");
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
    /** @todo   finish these...
                      [PyTuple 2 items]
                        [PyString "launchID"]
                        [PyInt 3]
                      [PyTuple 2 items]
                        [PyString "charID"]
                        [PyInt 3]
                      [PyTuple 2 items]
                        [PyString "itemID"]
                        [PyInt 20]
                      [PyTuple 2 items]
                        [PyString "solarSystemID"]
                        [PyInt 3]
                      [PyTuple 2 items]
                        [PyString "planetID"]
                        [PyInt 3]
                      [PyTuple 2 items]
                        [PyString "status"]
                        [PyInt 17]
                      [PyTuple 2 items]
                        [PyString "launchTime"]
                        [PyInt 64]
                      [PyTuple 2 items]
                        [PyString "x"]
                        [PyInt 5]
                      [PyTuple 2 items]
                        [PyString "y"]
                        [PyInt 5]
                      [PyTuple 2 items]
                        [PyString "z"]
                        [PyInt 5]
                        */
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT solarSystemID, planetID, launchTime, launchID, x, y, z"
        " FROM chrPlanetLaunches WHERE characterID = %u", charID)) {
        _log(DATABASE__ERROR, "Error in GetMyLaunchesDetails Query: %s", res.error.c_str());
        return NULL;
    }
    return DBResultToRowset(res);
}


PyRep* PlanetDB::GetExtractorsForPlanet(uint32 planetID) {
    /** @todo Incomplete, Needs to retrieve data from tables that do not exist yet.
     * Currently stops the client from throwing errors.
     */
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT 2130 AS typeID, 0 as ownerID")) {
        _log(DATABASE__ERROR, "Error in GetExtractorsForPlanet Query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

bool PlanetDB::GetResourceData(uint32 planetID, DBResultRow &row)
{
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT itemID1, itemID2, itemID3, itemID4, itemID5,"
        " data1, data2, data3, data4, data5,"
        " numBands1, numBands2, numBands3, numBands4, numBands5"
        " FROM planetResourceInfo WHERE planetID = %u", planetID)) {
        _log(DATABASE__ERROR, "Error in GetResourceData Query: %s", res.error.c_str());
        return false;
    }

    _log(DATABASE__RESULTS, "GetResourceData returned %u items", res.GetRowCount());
    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "GetResourceData failed to get row.");
        return false;
    }

    return true;
}
