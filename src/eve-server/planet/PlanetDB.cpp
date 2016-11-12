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


PyRep* PlanetDB::GetPlanetsForChar(uint32 charID) {
  /** self.colonizationData = sm.RemoteSvc('planetMgr').GetPlanetsForChar()
        returns  (solarSystemID, planetID, typeID, numberOfPins)
    */
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT solarSystemID, planetID, typeID, numberOfPins"
        " FROM chrPlanets WHERE charID = %u", charID)) {
        _log(DATABASE__ERROR, "Error in GetPlanetsForChar query: %s", res.error.c_str());
        return nullptr;
         }
         _log(DATABASE__RESULTS, "GetPlanetsForChar returned %u items", res.GetRowCount());
    return DBResultToCRowset(res);
}

PyRep* PlanetDB::GetMyLaunchesDetails(uint32 charID) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT launchID, charID, itemID, solarSystemID, planetID, status, launchTime, x, y, z"
        " FROM chrPlanetLaunches WHERE charID = %u", charID)) {
        _log(DATABASE__ERROR, "Error in GetMyLaunchesDetails Query: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToRowset(res);
}


void PlanetDB::SaveCCLevel(uint32 pinID, uint8 level)
{
    DBerror err;
    if(!sDatabase.RunQuery(err, "UPDATE chrPlanetCCPin SET level = %u WHERE pinID = %u", pinID, level))
    {
        _log(DATABASE__ERROR, "Error in SaveCCLevel : %s", err.GetError());
    }
}

uint32 PlanetDB::MakeCommandCenter(uint32 charID, uint32 planetID, uint32 typeID, float latitude, float longitude)
{
    uint32 pinID = 0;
    DBerror err;
    if(!sDatabase.RunQueryLID(err, pinID,
        "INSERT INTO chrPlanetCCPin (charID, planetID, typeID, latitude, longitude, status, level, lastSimTime) "
        " VALUES (%u, %u, %u, %f, %f, 0, 0, %" PRIu64 " )",
        charID, planetID, typeID, latitude, longitude, Win32TimeNow()))
    {
        _log(DATABASE__ERROR, "Error in MakeCommandCenter query: %s", err.GetError());
    }
    return pinID;
}

bool PlanetDB::GetResourceData(uint32 planetID, DBResultRow &row)
{
    /* this will be part of Planet class, and resources will be calculated there */
    // data, numBands, proximity
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

void PlanetDB::DeleteColony(uint32 pinID)
{

}

void PlanetDB::GetPlanetData(DBQueryResult& res)
{
    // load info into PlanetDataMgr
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
