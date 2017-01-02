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
*/

#include "eve-server.h"

#include "station/StationDB.h"

/** @todo this needs updating and optimizing.  --in progress 1Jan17  -allan */


PyPackedRow *StationDB::GetSolarSystem(uint32 solarSystemID) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT "
        " mss.solarSystemID,"            // nr
        " mss.solarSystemName,"            // string
        " mss.x, mss.y, mss.z,"                    // double
        " mss.radius,"                    // double
        " mss.security,"                // double
        " mss.constellationID,"            // nr
        " mss.factionID,"                // nr
        " mss.sunTypeID,"                // nr
        " mss.regionID,"
        " mlwc.wormholeClassID"
        " FROM mapSolarSystems AS mss"
        " LEFT JOIN mapLocationWormholeClasses AS mlwc ON mlwc.locationID = mss.regionID"
        " WHERE solarSystemID=%u", solarSystemID ))
    {
        codelog(DATABASE__ERROR, "Error in GetSolarSystem query: %s", res.error.c_str());
        return NULL;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        codelog(DATABASE__ERROR, "Error in GetSolarSystem query: no solarsystem for id %d", solarSystemID);
        return NULL;
    }

    return DBRowToPackedRow(row);
}

void StationDB::GetStationIDs(DBQueryResult& res)
{
    sDatabase.RunQuery(res, "SELECT stationID FROM staStations");
}

// this one needs some love
//int henk = "bla";
PyObject* StationDB::DoGetStation(uint32 stationID)
{
    /** @todo      update in progress..  -allan 04May16
              [PyObjectData Name: util.KeyVal]
                [PyDict 34 kvp]
                  [PyString "hangarGraphicID"]
                  [PyInt 241]
                  [PyString "maxShipVolumeDockable"]
                  [PyFloat 50000000]
                  [PyString "dockOrientationY"]
                  [PyFloat 0.193091094494]
                  [PyString "radius"]
                  [PyFloat 26782]
                  [PyString "conquerable"]
                  [PyBool False]
                  [PyString "stationTypeID"]
                  [PyInt 1530]
                  [PyString "description"]
                  [PyString "Stores product and freights goods to external retailers."]
                  [PyString "graphicID"]
                  [PyInt 1018]
                  [PyString "regionID"]
                  [PyInt 10000016]
                  [PyString "dockingBayGraphicID"]
                  [PyNone]
                  [PyString "upgradeLevel"]
                  [PyInt 0]
                  [PyString "standingOwnerID"]
                  [PyInt 1000039]
                  [PyString "reprocessingEfficiency"]
                  [PyFloat 0.5]
                  [PyString "officeRentalCost"]
                  [PyInt 27893494]
                  [PyString "dockEntryZ"]
                  [PyFloat 270.25189209]
                  [PyString "dockEntryY"]
                  [PyFloat 7499.12792969]
                  [PyString "dockEntryX"]
                  [PyFloat 11384.4287109]
                  [PyString "reprocessingStationsTake"]
                  [PyFloat 0.05]
                  [PyString "orbitID"]
                  [PyInt 40090915]
                  [PyString "stationName"]
                  [PyString "Torrinos V - Moon 16 - Home Guard Logistic Support"]
                  [PyString "dockOrientationZ"]
                  [PyFloat -1.37698270564E-08]
                  [PyString "ownerID"]
                  [PyInt 1000039]
                  [PyString "security"]
                  [PyInt 500]
                  [PyString "operationID"]
                  [PyInt 15]
                  [PyString "z"]
                  [PyFloat 745839452160]
                  [PyString "constellationID"]
                  [PyInt 20000209]
                  [PyString "dockingCostPerVolume"]
                  [PyFloat 0]
                  [PyString "stationID"]
                  [PyInt 60004045]
                  [PyString "serviceMask"]
                  [PyInt 58005077]
                  [PyString "y"]
                  [PyFloat 9607618560]
                  [PyString "x"]
                  [PyFloat 242305474560]
                  [PyString "solarSystemID"]
                  [PyInt 30001429]
                  [PyString "dockOrientationX"]
                  [PyFloat 0.981180846691]
                  [PyString "reprocessingHangarFlag"]
                  [PyInt 4]
                  */
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
    "SELECT"
    " staStations.stationID,"
    " staStations.x,"
    " staStations.y,"
    " staStations.z,"
    " mapDenormalize.orbitID,"
    " staStationTypes.hangarGraphicID,"
    " 0 AS upgradelevel,"
    " invTypes.graphicID,"
    " staStations.regionID,"
    " staStations.security,"
    " staStations.stationTypeID,"
    " staStationTypes.dockingBayGraphicID,"
    " staStations.officeRentalCost,"
    " staStations.stationName,"
    " staOperations.descriptionID,"
    " staStations.constellationID,"
    " staStations.operationID,"
    " staStations.solarSystemID,"
    " staStationTypes.dockOrientationX,"
    " staStationTypes.dockOrientationY,"
    " staStationTypes.dockOrientationZ,"
    " staStations.corporationID AS standingOwnerID,"
    " CAST(SUM(staOperationServices.serviceID) as UNSIGNED INTEGER) AS serviceMask, "
    " staStations.dockingCostPerVolume,"
    " staStations.reprocessingHangarFlag,"
    " staStations.reprocessingEfficiency,"
    " staStations.reprocessingStationsTake,"
    " staStationTypes.dockEntryX,"
    " staStationTypes.dockEntryY,"
    " staStationTypes.dockEntryZ,"
    " staStations.maxShipVolumeDockable,"
    " staStations.corporationID AS ownerID,"
    " staStationTypes.conquerable,"
    " mapDenormalize.radius"
    " FROM staStations "
    " LEFT JOIN staStationTypes ON "
    "   staStations.stationTypeID=staStationTypes.stationTypeID "
    " LEFT JOIN mapDenormalize ON "
    "   staStations.stationID=mapDenormalize.itemID "
    " LEFT JOIN invTypes ON "
    "   staStations.stationTypeID=invTypes.typeID "
    " LEFT JOIN staOperations ON "
    "   staStations.operationID=staOperations.operationID "
    " LEFT JOIN staOperationServices ON "
    "   staStations.operationID=staOperationServices.operationID "
    " WHERE staStations.stationID = %u"
    " GROUP BY staStations.stationID", stationID ))
    {
        _log(DATABASE__ERROR, "Error in DoGetStation query: %s", res.error.c_str());
        return nullptr;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        codelog(DATABASE__ERROR, "Error in DoGetStation query: no station for id %d", stationID);
        return nullptr;
    }

    return DBRowToKeyVal(row);

}

PyRep *StationDB::GetStationItemBits(uint32 stationID) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        " SELECT "
        " staStations.stationID, "
        " staStations.stationTypeID, staStations.corporationID AS ownerID, "
        " staStationTypes.hangarGraphicID, "
        // damn mysql returns the result of the sum as string and so it is sent to the client as string and so it freaks out...
        " CAST(SUM(staOperationServices.serviceID) as UNSIGNED INTEGER) AS serviceMask "
        " FROM staStations "
        " LEFT JOIN staStationTypes ON staStations.stationTypeID = staStationTypes.stationTypeID "
        " LEFT JOIN staOperationServices ON staStations.operationID = staOperationServices.operationID "
        " WHERE staStations.stationID = %u "
        " GROUP BY staStations.stationID ", stationID
    ))
    {
        codelog(DATABASE__ERROR, "Error in GetStationItemBits query: %s", res.error.c_str());
        return NULL;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        codelog(DATABASE__ERROR, "Error in GetStationItemBits query: no station for id %d", stationID);
        return NULL;
    }

    PyTuple * result = new PyTuple(5);
        result->SetItem(0, new PyInt(row.GetUInt(3)));
        result->SetItem(1, new PyInt(row.GetUInt(2)));
        result->SetItem(2, new PyInt(row.GetUInt(0)));
        result->SetItem(3, new PyInt(row.GetUInt(4)));
        result->SetItem(4, new PyInt(row.GetUInt(1)));
    return result;
}
