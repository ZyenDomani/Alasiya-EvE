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
    Author:        Allan, based on original design by Comet0
*/

#include "eve-server.h"

#include "planet/PlanetDB.h"


PyRep* PlanetDB::GetPlanetInfo(uint32 planetID) {
    /* this will be part of Planet class, and info will be handled there */
    /*
              [PyString "planetTypeID"]
              [PyInt 2016]
              [PyString "solarSystemID"]
              [PyInt 30001984]
              [PyString "radius"]
              [PyFloat 2170000]
              [PyString "planetID"]
              [PyInt 40126699]

              'currentSimTime' and 'pins'  are populated for planets that are colonlized
    */
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT solarSystemID, typeID AS planetTypeID, itemID AS planetID, radius"
        " FROM mapDenormalize WHERE itemID = %u", planetID)) {
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
    if(!sDatabase.RunQuery(res,
        "SELECT solarSystemID, planetID, typeID, numberOfPins"
        " FROM chrPlanets WHERE charID = %u", charID)) {
        _log(DATABASE__ERROR, "Error in GetPlanetsForChar query: %s", res.error.c_str());
        return NULL;
         }
         _log(DATABASE__RESULTS, "GetPlanetsForChar returned %u items", res.GetRowCount());
    return DBResultToCRowset(res);
}

PyRep* PlanetDB::GetPlanetResourceInfo(uint32 planetID) {
    /* this will be part of Planet class, and resources will be calculated there */
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
    /*
            [PySubStream 67 bytes]
              [PyDict 5 kvp]
                [PyInt 2288]
                [PyFloat 100.11311298535]
                [PyInt 2073]
                [PyFloat 93.8896871755585]
                [PyInt 2267]
                [PyFloat 88.2820365560074]
                [PyInt 2268]
                [PyFloat 50.6448014279542]
                [PyInt 2270]
                [PyFloat 66.0059005883846]
    */
    PyDict *rtn = new PyDict();
        rtn->SetItem(new PyInt(row.GetInt(0)), new PyFloat(row.GetFloat(5)));
        rtn->SetItem(new PyInt(row.GetInt(1)), new PyFloat(row.GetFloat(6)));
        rtn->SetItem(new PyInt(row.GetInt(2)), new PyFloat(row.GetFloat(7)));
        rtn->SetItem(new PyInt(row.GetInt(3)), new PyFloat(row.GetFloat(8)));
        rtn->SetItem(new PyInt(row.GetInt(4)), new PyFloat(row.GetFloat(9)));
    return rtn;
}

PyRep* PlanetDB::GetMyLaunchesDetails(uint32 charID) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT launchID, charID, itemID, solarSystemID, planetID, status, launchTime, x, y, z"
        " FROM chrPlanetLaunches WHERE charID = %u", charID)) {
        _log(DATABASE__ERROR, "Error in GetMyLaunchesDetails Query: %s", res.error.c_str());
        return NULL;
    }
    return DBResultToRowset(res);
}


PyRep* PlanetDB::GetExtractorsForPlanet(uint32 planetID) {
    /** @todo Incomplete, Needs to retrieve data from tables that do not exist yet.
     * Currently stops the client from throwing errors.
     */
    /* this will be part of Planet class, and resources will be calculated there */
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT 2130 AS typeID, 0 as ownerID")) {
        _log(DATABASE__ERROR, "Error in GetExtractorsForPlanet Query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

bool PlanetDB::GetResourceData(uint32 planetID, DBResultRow &row)
{
    /* this will be part of Planet class, and resources will be calculated there */
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

void PlanetDB::LoadColony()
{

}

void PlanetDB::SaveColony()
{

}

void PlanetDB::GetPlanetData(DBQueryResult& res)
{
    if(!sDatabase.RunQuery(res,
        "SELECT planet.typeID AS planetTypeID,"
        " resource.typeID AS resourceID"
        " FROM invTypes planet, invTypes resource, dgmTypeAttributes dgm1, dgmTypeAttributes dgm2 "
        " WHERE dgm1.typeID = dgm2.typeID AND dgm1.attributeID = 1632 AND dgm1.valueFloat = planet.typeID AND dgm2.attributeID = 709 AND dgm2.valueFloat = resource.typeID ORDER BY planet.typeID ")) {
        _log(DATABASE__ERROR, "Error in GetPlanetData Query: %s", res.error.c_str());
    }
}

GPoint PlanetDB::GetLaunchPos(uint32 launchID)
{
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT x,y,z FROM `planetlaunches` WHERE `launchID` = %u", launchID)) {
        _log(DATABASE__ERROR, "Error in GetLaunchPos query: %s", res.error.c_str());
    }
    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "Error in GetLaunchPos query: %s", res.error.c_str());
        return NULL_ORIGIN;
    }
    GPoint pos(row.GetFloat(0), row.GetFloat(1), row.GetFloat(2));
    return pos;
}
