/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
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

/*
 * PLANET__ERROR
 * PLANET__WARNING
 * PLANET__MESSAGE
 * PLANET__DEBUG
 * PLANET__INFO
 * PLANET__TRACE
 * PLANET__DUMP
 * PLANET__RES_DUMP
 * PLANET__GC_DUMP
 * PLANET__PKT_TRACE
 * PLANET__DB_ERROR
 * PLANET__DB_WARNING
 */


#include "planet/Colony.h"
#include "planet/PlanetDB.h"
#include "planet/PlanetDataMgr.h"


bool PlanetDB::LoadPlanetResourceData(uint32 planetID, PlanetResourceData& data) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res, "SELECT replenishTime,Resource1, Resource2, Resource3, Resource4, Resource5,"
            " Distributon1, Distributon2, Distributon3, Distributon4, Distributon5,"
            " curBuffer1, curBuffer2, curBuffer3, curBuffer4, curBuffer5,"
            " origBuffer1, origBuffer2, origBuffer3, origBuffer4, origBuffer5"
            " FROM PlanetData"
            " WHERE PlanetID=%u", planetID)) {
        _log(DATABASE__ERROR, "Error in LoadPlanetResourceData Query: %s", res.error.c_str());
        return false;
    }

    // no data, no error, just return
    if (res.GetRowCount() < 1)
        return false;

    DBResultRow row;
    if (!res.GetRow(row))
        return false;

    data.replenishTime = row.GetInt64(0);
    data.type_1 = row.GetUInt16(1);
    data.type_2 = row.GetUInt16(2);
    data.type_3 = row.GetUInt16(3);
    data.type_4 = row.GetUInt16(4);
    data.type_5 = row.GetUInt16(5);
    data.dist_1 = row.GetFloat(6);
    data.dist_2 = row.GetFloat(7);
    data.dist_3 = row.GetFloat(8);
    data.dist_4 = row.GetFloat(9);
    data.dist_5 = row.GetFloat(10);
    data.buffer_1 = sPlanetDataMgr.HexToBinary(row.GetText(11));
    data.buffer_2 = sPlanetDataMgr.HexToBinary(row.GetText(12));
    data.buffer_3 = sPlanetDataMgr.HexToBinary(row.GetText(13));
    data.buffer_4 = sPlanetDataMgr.HexToBinary(row.GetText(14));
    data.buffer_5 = sPlanetDataMgr.HexToBinary(row.GetText(15));
    data.origBuf_1 = sPlanetDataMgr.HexToBinary(row.GetText(16));
    data.origBuf_2 = sPlanetDataMgr.HexToBinary(row.GetText(17));
    data.origBuf_3 = sPlanetDataMgr.HexToBinary(row.GetText(18));
    data.origBuf_4 = sPlanetDataMgr.HexToBinary(row.GetText(19));
    data.origBuf_5 = sPlanetDataMgr.HexToBinary(row.GetText(20));

    return true;
}

void PlanetDB::SavePlanetResourceData(uint32 planetID, PlanetResourceData& data) {
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "INSERT INTO PlanetData "
            " (PlanetID, replenishTime, Resource1,Resource2,Resource3,Resource4,Resource5,"
            "Distributon1,Distributon2,Distributon3,Distributon4,Distributon5,"
            " curBuffer1, curBuffer2, curBuffer3, curBuffer4, curBuffer5,"
            " origBuffer1, origBuffer2, origBuffer3, origBuffer4, origBuffer5)"
            " VALUES"
            " (%u,%lli,%u,%u,%u,%u,%u,"
            "%f,%f,%f,%f,%f,"
            "'%s','%s','%s','%s','%s',"
            "'%s','%s','%s','%s','%s')"
            " ON DUPLICATE KEY UPDATE"
            " replenishTime=VALUES(replenishTime),"
            " Distributon1=VALUES(Distributon1),"
            " Distributon2=VALUES(Distributon2),"
            " Distributon3=VALUES(Distributon3),"
            " Distributon4=VALUES(Distributon4),"
            " Distributon5=VALUES(Distributon5),"
            " curBuffer1=VALUES(curBuffer1),"
            " curBuffer2=VALUES(curBuffer2),"
            " curBuffer3=VALUES(curBuffer3),"
            " curBuffer4=VALUES(curBuffer4),"
            " curBuffer5=VALUES(curBuffer5),"
            " origBuffer1=VALUES(origBuffer1),"
            " origBuffer2=VALUES(origBuffer2),"
            " origBuffer3=VALUES(origBuffer3),"
            " origBuffer4=VALUES(origBuffer4),"
            " origBuffer5=VALUES(origBuffer5);",
            planetID, data.replenishTime, data.type_1, data.type_2, data.type_3, data.type_4, data.type_5,
            data.dist_1, data.dist_2, data.dist_3, data.dist_4, data.dist_5,
            sPlanetDataMgr.BinaryToHex(data.buffer_1).c_str(), sPlanetDataMgr.BinaryToHex(data.buffer_2).c_str(),
            sPlanetDataMgr.BinaryToHex(data.buffer_3).c_str(), sPlanetDataMgr.BinaryToHex(data.buffer_4).c_str(),
            sPlanetDataMgr.BinaryToHex(data.buffer_5).c_str(), sPlanetDataMgr.BinaryToHex(data.origBuf_1).c_str(),
            sPlanetDataMgr.BinaryToHex(data.origBuf_2).c_str(), sPlanetDataMgr.BinaryToHex(data.origBuf_3).c_str(),
            sPlanetDataMgr.BinaryToHex(data.origBuf_4).c_str(), sPlanetDataMgr.BinaryToHex(data.origBuf_5).c_str()))
        _log(DATABASE__ERROR, "SavePlanetResourceData - Unable to update planetID %u : %s", planetID, err.GetError());
}

void PlanetDB::GetSchematicData(DBQueryResult& res)
{
    // load info into PIDataMgr
    if (!sDatabase.RunQuery(res, "SELECT schematicID, typeID, quantity, isInput FROM piTypeMap" ))
        _log(DATABASE__ERROR, "Error in GetSchematicData Query: %s", res.error.c_str());
}

void PlanetDB::GetSchematicTimes(DBQueryResult& res)
{
    // load info into PIDataMgr
    if (!sDatabase.RunQuery(res, "SELECT schematicID, cycleTime FROM piSchematics"))
        _log(DATABASE__ERROR, "Error in GetSchematicTimes Query: %s", res.error.c_str());
}

void PlanetDB::GetPlanetData(DBQueryResult& res)
{
    // load info into PlanetDataMgr
    if (!sDatabase.RunQuery(res,
        "SELECT planet.typeID,"
        " resource.typeID"
        " FROM invTypes planet, invTypes resource, dgmTypeAttributes dgm1, dgmTypeAttributes dgm2 "
        " WHERE dgm1.typeID = dgm2.typeID AND dgm1.attributeID = 1632 AND dgm1.valueFloat = planet.typeID AND dgm2.attributeID = 709"
        " AND dgm2.valueFloat = resource.typeID ORDER BY planet.typeID ")) {
        _log(DATABASE__ERROR, "Error in GetPlanetData Query: %s", res.error.c_str());
        }
}


PyRep* PlanetDB::GetPlanetsForChar(uint32 charID) {
  /** self.colonizationData = sm.RemoteSvc('planetMgr').GetPlanetsForChar()
        returns  (solarSystemID, planetID, typeID, numberOfPins)
    */
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT solarSystemID, planetID, typeID, numberOfPins"
        " FROM piPlanets WHERE charID = %u", charID)) {
        _log(DATABASE__ERROR, "Error in GetPlanetsForChar query: %s", res.error.c_str());
        return nullptr;
    }
    _log(DATABASE__RESULTS, "GetPlanetsForChar returned %zu items", res.GetRowCount());
    return DBResultToCRowset(res);
}

void PlanetDB::AddPlanetForChar(uint32 solarSystemID, uint32 planetID, uint32 charID, uint32 colonyID, uint16 typeID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "INSERT INTO piPlanets (solarSystemID, planetID, charID, typeID, colonyID)"
        " VALUES (%u, %u, %u, %u, %u)", solarSystemID, planetID, charID, typeID, colonyID))
    {
        _log(DATABASE__ERROR, "AddPlanetForChar - Unable to add planet %u for char %u: %s", planetID, charID, err.GetError());
    }
}

void PlanetDB::UpdatePlanetPins(uint32 colonyID, uint8 pins)
{
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE piPlanets SET numberOfPins = %u WHERE colonyID = %u ", pins, colonyID))
        _log(DATABASE__ERROR, "UpdatePlanetPins - Unable to update colonyID %u : %s", colonyID, err.GetError());
}

//  expired = not blue.os.GetWallclockTime() - launch.launchTime < const.piLaunchOrbitDecayTime (5d)
PyRep* PlanetDB::GetMyLaunchesDetails(uint32 charID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT itemID AS launchID, status, itemID, solarSystemID, planetID, launchTime, x, y, z"
        " FROM piLaunches WHERE charID = %u", charID)) {
        _log(DATABASE__ERROR, "Error in GetMyLaunchesDetails Query: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToRowset(res);
}

Vector3d PlanetDB::GetLaunchPos(uint32 launchID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT x,y,z FROM piLaunches WHERE launchID = %u", launchID)) {
        _log(DATABASE__ERROR, "Error in GetLaunchPos query: %s", res.error.c_str());
    }
    DBResultRow row;
    if (!res.GetRow(row))
        return NULL_ORIGIN;

    Vector3d pos(row.GetDouble(0), row.GetDouble(1), row.GetDouble(2));
    return pos;
}

uint32 PlanetDB::GetLaunchPlanet(uint32 launchID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT planetID FROM piLaunches WHERE launchID = %u", launchID)) {
        _log(DATABASE__ERROR, "Error in GetLaunchPlanet query: %s", res.error.c_str());
    }
    DBResultRow row;
    if (!res.GetRow(row))
        return 0;

    return row.GetUInt(0);
}

void PlanetDB::GetLaunchDetails(Launch::Data& data)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT itemID, status, solarSystemID, planetID, launchTime, x, y, z"
        " FROM piLaunches WHERE launchID = %u", data.launchID)) {
        _log(DATABASE__ERROR, "Error in GetLaunchDetails query: %s", res.error.c_str());
    }
    DBResultRow row;
    if (!res.GetRow(row))
        return;

    data.itemID = row.GetUInt(0);
    data.status = row.GetUInt8(1);
    data.solarSystemID = row.GetUInt(2);
    data.planetID = row.GetUInt(3);
    data.launchTime = row.GetInt64(4);
    data.x = row.GetDouble(5);
    data.y = row.GetDouble(6);
    data.z = row.GetDouble(7);
}

uint32 PlanetDB::GetLaunchItemID(uint32 launchID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT itemID FROM piLaunches WHERE launchID = %u", launchID)) {
        _log(DATABASE__ERROR, "Error in GetLaunchItemID query: %s", res.error.c_str());
    }
    DBResultRow row;
    if (!res.GetRow(row))
        return 0;

    return row.GetUInt(0);
}

int64 PlanetDB::GetLastLaunchTime(uint32 colonyID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT expiryTime FROM piPins WHERE pinID = %u", colonyID )) {
        _log(DATABASE__ERROR, "Error in GetLastLaunchTime Query: %s", res.error.c_str());
        return 0;
    }
    DBResultRow row;
    if (res.GetRow(row))
        return row.GetInt64(0);
    return 0;
}

void PlanetDB::UpdateLaunchStatus(uint32 itemID, uint8 status)
{
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE piLaunches SET status=%u WHERE itemID = %u", status, itemID);
}

void PlanetDB::GetExtractorsForPlanet(uint32 planetID, DBQueryResult& res)
{
    //SELECT pinID, typeID, ownerID, latitude, longitude FROM piPins WHERE isECU = 1
    if (!sDatabase.RunQuery(res,
        "SELECT pinID, typeID, ownerID, latitude, longitude"
        " FROM piPins"
        " WHERE isECU = 1"
        " AND colonyID IN"
        " (SELECT colonyID FROM piCCPin"
        " WHERE planetID = %u)",
        planetID))
    {
        _log(DATABASE__ERROR, "Error in GetExtractorsForPlanet Query: %s", res.error.c_str());
        return;
    }

    _log(DATABASE__RESULTS, "GetExtractorsForPlanet returned %zu items", res.GetRowCount());
}

bool PlanetDB::LoadColony(uint32 charID, uint32 planetID, PI_CCData* pData)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT colonyID,level"
        " FROM piCCPin"
        " WHERE charID = %u"
        " AND planetID = %u",
        charID, planetID ))
    {
        _log(DATABASE__ERROR, "Error in LoadColony Query: %s", res.error.c_str());
        return false;
    }
    DBResultRow row;
    if (res.GetRow(row)) {
        pData->level = row.GetUInt8(1);
        pData->colonyID = row.GetUInt(0);
        //pData->launchTime = GetLastLaunchTime(row.GetUInt(0));
        return true;
    }
    return false;
}

void PlanetDB::LoadPins(uint32 colonyID, std::unordered_map<uint32, PI::PinData>& pins)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT pinID, typeID, ownerID, state, level, latitude, longitude, "
        " isCommandCenter, isLaunchable, isProcess, isStorage, isECU,"
        " schematicID, cycleTime, installTime, lastRunTime, qtyPerCycle, expiryTime"
        " FROM piPins"
        " WHERE colonyID = %u", colonyID))
    {
        _log(DATABASE__ERROR, "Error in LoadPins Query: %s", res.error.c_str());
        return;
    }

    _log(DATABASE__RESULTS, "LoadPins returned %zu items", res.GetRowCount());

    DBResultRow row;
    while (res.GetRow(row)) {
        PI::PinData pin = PI::PinData();
            pin.typeID                  = row.GetUInt16(1);
            pin.ownerID                 = row.GetUInt(2);
            pin.state                   = row.GetInt8(3);
            pin.level                   = row.GetUInt8(4);
            pin.latitude                = row.GetDouble(5);
            pin.longitude               = row.GetDouble(6);
            pin.isCommandCenter         = row.GetBool(7);
            pin.isLaunchable            = row.GetBool(8);
            pin.isProcess               = row.GetBool(9);
            pin.isStorage               = row.GetBool(10);
            pin.isECU                   = row.GetBool(11);
            pin.schematicID             = row.GetUInt16(12);
            pin.cycleTime               = row.GetInt64(13);
            pin.installTime             = row.GetInt64(14);
            pin.lastRunTime             = row.GetInt64(15);
            pin.qtyPerCycle             = row.GetInt(16);
            pin.launchTime              = row.GetInt64(17);

        if (pin.isStorage or pin.isProcess)
            LoadContents(row.GetUInt(0), pin.contents);

        pins.emplace(row.GetUInt(0), pin);
    }
}

void PlanetDB::LoadLinks(uint32 colonyID, std::unordered_map<uint32, PI::Link >& links)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT linkID, level, state, endpoint1, endpoint2"
        " FROM piLinks WHERE colonyID = %u",
        colonyID ))
    {
        _log(DATABASE__ERROR, "Error in LoadLinks Query: %s", res.error.c_str());
        return;
    }

    _log(DATABASE__RESULTS, "LoadLinks returned %zu items", res.GetRowCount());

    DBResultRow row;
    while (res.GetRow(row)) {
        PI::Link link = PI::Link();
            link.level = row.GetUInt8(1);
            link.state = row.GetInt8(2);
            link.typeID = 2280; // only link type in game
            link.endpoint1 = row.GetUInt(3);
            link.endpoint2 = row.GetUInt(4);
        links.emplace(row.GetUInt(0), link);
    }
}

void PlanetDB::LoadRoutes(uint32 colonyID, std::unordered_map<uint16, PI::Route >& routes) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT routeID, srcPinID, destPinID, state, priority, path, typeID, itemQty"
        " FROM piRoutes WHERE colonyID = %u",
        colonyID ))
    {
        _log(DATABASE__ERROR, "Error in LoadRoutes Query: %s", res.error.c_str());
        return;
    }

    _log(DATABASE__RESULTS, "LoadRoutes returned %zu items", res.GetRowCount());

    DBResultRow row;
    std::string tempPath;
    while (res.GetRow(row)) {
        tempPath.clear();
        PI::Route route = PI::Route();
            route.srcPinID = row.GetUInt(1);
            route.destPinID = row.GetUInt(2);
            route.state = row.GetInt8(3);
            route.priority = row.GetInt8(4);
            route.commodityTypeID = row.GetUInt16(6);
            route.commodityQuantity = row.GetUInt16(7);

        tempPath = row.GetText(5);
        if (!tempPath.empty()) {
            // route string is not empty, so extract one number at a time until it is
            int pos = 0;
            std::string tempString = "";

            while ((pos = tempPath.find_first_of(':')) > 0 ) {
                tempString = tempPath.substr(0,pos);
                tempPath = tempPath.substr(pos + 1,tempPath.length() - 1);
                route.path.insert(route.path.end(), (atoi(tempString.c_str())));
            }
            // insert last pinID
            route.path.insert(route.path.end(), (atoi(tempPath.c_str())));
        }

        routes.emplace(row.GetUInt16(0), route);
    }
}

void PlanetDB::LoadContents(uint32 pinID, std::map<uint16, uint32>& contents)
{
    //SELECT colonyID, pinID, typeID, itemQty FROM piPinContents
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT typeID, itemQty"
        " FROM piPinContents WHERE pinID = %u",
        pinID ))
    {
        _log(DATABASE__ERROR, "Error in LoadContents Query: %s", res.error.c_str());
        return;
    }

    _log(DATABASE__RESULTS, "LoadContents returned %zu items", res.GetRowCount());

    DBResultRow row;
    while (res.GetRow(row)) {
        contents.emplace(row.GetUInt16(0), row.GetUInt(1));
    }
}

void PlanetDB::LoadECU(uint32 ecuID, DBQueryResult& res) {
    if (!sDatabase.RunQuery(res,
        "SELECT expiryTime, headRadius, programType, cycleCount"
        " FROM piPins"
        " WHERE pinID = %u", ecuID))
    {
        _log(DATABASE__ERROR, "Error in LoadECU Query: %s", res.error.c_str());
        return;
    }

    _log(DATABASE__RESULTS, "LoadECU returned %zu items", res.GetRowCount());
}

void PlanetDB::LoadHeads(uint32 ecuID, std::unordered_map< uint16, PI::Heads >& heads)
{
    //SELECT ecuID, headID, typeID, latitude, longitude FROM piECUHeads
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT headID, typeID, latitude, longitude"
        " FROM piECUHeads WHERE ecuID = %u",
        ecuID ))
    {
        _log(DATABASE__ERROR, "Error in LoadHeads Query: %s", res.error.c_str());
        return;
    }

    _log(DATABASE__RESULTS, "LoadHeads returned %zu items", res.GetRowCount());

    DBResultRow row;
    while (res.GetRow(row)) {
        PI::Heads head = PI::Heads();
            head.typeID = row.GetUInt16(1);
            head.ecuPinID = ecuID;
            head.latitude = row.GetDouble(2);
            head.longitude = row.GetDouble(3);
        heads.emplace(row.GetUInt16(0), head);
    }
}

void PlanetDB::SaveLaunch(uint32 contID, uint32 charID, uint32 systemID, uint32 planetID, Vector3d& pos)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "INSERT INTO piLaunches (itemID, status, charID, solarSystemID, planetID, launchTime, x, y, z) "
        " VALUES (%u, 0, %u, %u, %u, %lli, %f, %f, %f)",
        contID, charID, systemID, planetID, GetFileTimeNow(), pos.x, pos.y, pos.z))
        {
            _log(DATABASE__ERROR, "SaveLaunch() - Unable to save Launch: %s", err.GetError());
        }
}

void PlanetDB::SaveCommandCenter(uint32 colonyID, uint32 charID, uint32 planetID, uint32 typeID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "INSERT INTO piCCPin (colonyID, charID, planetID, typeID) "
        " VALUES (%u, %u, %u, %u)", colonyID, charID, planetID, typeID))
    {
        _log(DATABASE__ERROR, "SaveCommandCenter - Unable to save CommandCenter: %s", err.GetError());
    }
}

void PlanetDB::SaveCCLevel(uint32 colonyID, uint8 level)
{
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE piCCPin SET level = %u WHERE colonyID = %u", level, colonyID))
        _log(DATABASE__ERROR, "SaveCCLevel - Unable to save CCLevel(1): %s", err.GetError());
    if (!sDatabase.RunQuery(err, "UPDATE piPins SET level = %u WHERE colonyID = %u", level, colonyID))
        _log(DATABASE__ERROR, "SaveCCLevel - Unable to save CCLevel(2): %s", err.GetError());
}

void PlanetDB::UpdateCCLaunch(uint32 pinID, int64 launchTime) {
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE piPin SET expiryTime = %lli WHERE pinID = %u", launchTime, pinID))
        _log(DATABASE__ERROR, "UpdateCCLaunch - Unable to save CCLevel(1): %s", err.GetError());
}

void PlanetDB::CreatePin(uint32 colonyID, uint32 pinID, PI::PinData &data) {
    // save newly-created pin data
    DBerror err;
    if (!sDatabase.RunQuery(err,
    "INSERT INTO piPins"
    " (colonyID, pinID, typeID, ownerID, level, latitude, longitude,"
    " isCommandCenter, isLaunchable, isProcess, isStorage, isECU,"
    " cycleTime, installTime)"
    " VALUES"
    " (%u, %u, %u, %u, %u, %f, %f,"
    " %u, %u, %u, %u, %u,"
    " %lli, %lli)",
    colonyID, pinID, data.typeID, data.ownerID, data.level, data.latitude, data.longitude,
    data.isCommandCenter, data.isLaunchable, data.isProcess, data.isStorage, data.isECU,
    data.cycleTime, data.installTime))
    {
        _log(DATABASE__ERROR, "CreatePin - unable to save pins: %s", err.c_str());
    }
}

void PlanetDB::SavePins(PI_CCData* pData) {
    // only called on Save()
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO piPins";
    Inserts << " (pinID, level, schematicID, cycleTime, installTime, lastRunTime, expiryTime)";
    Inserts << " VALUES ";
    bool save = false;
    for (auto &cur : pData->pins) {
        if (save) {
            Inserts << ", ";
        } else {
            save = true;
        }
        Inserts << "(" << cur.first << ", " << (uint16)cur.second.level << ", " << cur.second.schematicID << ", ";
        Inserts << cur.second.cycleTime << ", " << cur.second.installTime << ", " << cur.second.lastRunTime;
        Inserts << ", " << cur.second.launchTime << ")";
    }

    if (save) {
        // finish creating the command.
        Inserts << " ON DUPLICATE KEY UPDATE";
        Inserts << " level=VALUES(level),";
        Inserts << " schematicID=VALUES(schematicID),";
        Inserts << " cycleTime=VALUES(cycleTime),";
        Inserts << " installTime=VALUES(installTime),";
        Inserts << " lastRunTime=VALUES(lastRunTime),";
        Inserts << " expiryTime=VALUES(expiryTime);";
        // execute the command.
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SavePins - unable to save pins: %s", err.c_str());
    }
}

void PlanetDB::UpdateECUPin(uint32 ecuID, PI_CCData* pData) {
    std::unordered_map<uint32, PI::PinData>::iterator pinItr = pData->pins.find(ecuID);
    if (pinItr == pData->pins.end()) {
        _log(PLANET__ERROR, "PlanetDB::UpdateECUPin() - pinID %u not found in data.pins map", ecuID);
        return;
    }

    std::unordered_map<uint32, PI::ECU>::iterator ecuItr = pData->ecus.find(ecuID);
    if (ecuItr == pData->ecus.end()) {
        _log(PLANET__ERROR, "PlanetDB::UpdateECUPin() - ecuID %u not found in data.ecus map", ecuID);
        return;
    }

    DBerror err;
    if (!sDatabase.RunQuery(err,
        "UPDATE piPins SET"
        "  programType = %u,"
        "  headRadius = %f,"
        "  qtyPerCycle = %i,"
        "  schematicID = %u,"
        "  expiryTime = %lli,"
        "  cycleTime = %lli,"
        "  installTime = %lli,"
        "  lastRunTime = %lli,"
        "  cycleCount = %u"
        " WHERE pinID = %u",
        ecuItr->second.programType, ecuItr->second.headRadius, pinItr->second.qtyPerCycle, pinItr->second.schematicID,
        ecuItr->second.expiryTime, pinItr->second.cycleTime, pinItr->second.installTime, pinItr->second.lastRunTime,
        ecuItr->second.cycleCount, ecuID))
    {
        _log(DATABASE__ERROR, "UpdateECUPin - Unable to update pin: %s", err.GetError());
    }
}

void PlanetDB::UpdatePinTimes(PI_CCData* pData)
{
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO piPins";
    Inserts << " (pinID, installTime, lastRunTime, expiryTime) VALUES ";

    bool save = false;
    for (auto &cur : pData->pins) {
        if (save) {
            Inserts << ", ";
        } else {
            save = true;
        }
        Inserts << "(" << cur.first << ", " << cur.second.installTime << ", ";
        Inserts << cur.second.lastRunTime << ", " << cur.second.launchTime << ")";
    }

    if (save) {
        // finish creating the command.
        Inserts << " ON DUPLICATE KEY UPDATE ";
        Inserts << " installTime=VALUES(installTime),";
        Inserts << " lastRunTime=VALUES(lastRunTime);";
        Inserts << " expiryTime=VALUES(expiryTime);";
        // execute the command.
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "UpdatePinTimes - unable to save data: %s", err.c_str());
    }
}

void PlanetDB::SaveHeads(uint32 colonyID, uint32 ownerID, uint32 ecuID, std::unordered_map< uint16, PI::Heads >& heads)
{
    if (heads.empty())
        return;

    DBerror err;
    for (auto &cur : heads) {
        // save the head data separately
        if (!sDatabase.RunQuery(err,
            "INSERT INTO piECUHeads (colonyID, ownerID, ecuID, headID, typeID, latitude, longitude)"
            " VALUES (%u, %u, %u, %u, %u, %f, %f)"
            " ON DUPLICATE KEY UPDATE "
            " typeID=VALUES(typeID),"
            " latitude=VALUES(latitude),"
            " longitude=VALUES(longitude)",
            colonyID, ownerID, ecuID, cur.first, cur.second.typeID, cur.second.latitude, cur.second.longitude))
        {
            _log(DATABASE__ERROR, "SaveHeads - Unable to save heads: %s", err.GetError());
        }
    }
}

void PlanetDB::SavePinLevel(uint32 pinID, uint8 level)
{
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE piPins SET level = %u WHERE pinID = %u", level, pinID))
    {
        _log(DATABASE__ERROR, "SavePinLevel - Unable to save PinLevel: %s", err.GetError());
    }
}

void PlanetDB::SaveLinks(PI_CCData* pData)
{
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO piLinks";
    Inserts << " (colonyID, linkID, level, endpoint1, endpoint2) VALUES ";

    bool save = false;
    for (auto &cur : pData->links) {
        if (save) {
            Inserts << ", ";
        } else {
            save = true;
        }
        Inserts << "(" << pData->colonyID << ", " << cur.first << ", " << (uint16)cur.second.level << ", " << cur.second.endpoint1 << ", " << cur.second.endpoint2 << ")";
    }

    if (save) {
        // finish creating the command.
        Inserts << " ON DUPLICATE KEY UPDATE";
        Inserts << " level=VALUES(level), ";
        Inserts << " endpoint1=VALUES(endpoint1), ";
        Inserts << " endpoint2=VALUES(endpoint2)";
        // execute the command.
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SaveLinks - unable to save links - %s", err.c_str());
    }
}

void PlanetDB::SaveLinkLevel(uint32 linkID, uint8 level)
{
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE piLinks SET level = %u WHERE linkID = %u", level, linkID))
    {
        _log(DATABASE__ERROR, "SaveLinkLevel - Unable to save LinkLevel: %s", err.GetError());
    }
}

uint16 PlanetDB::SaveRoute(uint32 colonyID, PI::Route& route)
{
    DBerror err;
    uint32 routeID(0);
    std::string path;
    path.clear();
    std::list<uint32>::iterator itr = route.path.begin();
    while (itr != route.path.end()) {
        path += std::to_string(*itr);
        ++itr;
        if (itr != route.path.end())
            path += ":";
    }
    if (!sDatabase.RunQueryLID(err, routeID,
        "INSERT INTO piRoutes(colonyID, srcPinID, destPinID, state, priority, path, typeID, itemQty) "
        " VALUES (%u, %u, %u, %i, %i, '%s', %u, %u)",
                colonyID, route.srcPinID, route.destPinID, route.state, route.priority, path.c_str(),
                route.commodityTypeID, route.commodityQuantity))
    {
        _log(DATABASE__ERROR, "SaveRoute - Unable to save routes: %s", err.GetError());
    }
    return (uint16)routeID;
}

void PlanetDB::SaveRoutes(PI_CCData* pData)
{
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO piRoutes";
    Inserts << " (colonyID, routeID, srcPinID, destPinID, path, typeID, itemQty) VALUES ";

    bool save = false;
    std::string path;
    std::list<uint32>::iterator itr;
    for (auto &cur : pData->routes) {
        path.clear();
        if (save) {
            Inserts << ", ";
        } else {
            save = true;
        }
        itr = cur.second.path.begin();
        while (itr != cur.second.path.end()) {
            path += std::to_string(*itr);
            ++itr;
            if (itr != cur.second.path.end())
                path += ":";
        }
        Inserts << "(" << pData->colonyID << ", " << cur.first << ", " << cur.second.srcPinID << ", " << cur.second.destPinID << ", '" << path << "', ";
        Inserts << cur.second.commodityTypeID << ", " << cur.second.commodityQuantity << ")";
    }

    if (save) {
        // finish creating the command.
        Inserts << " ON DUPLICATE KEY UPDATE";
        Inserts << " path=VALUES(path),";
        Inserts << " srcPinID=VALUES(srcPinID),";
        Inserts << " destPinID=VALUES(destPinID),";
        Inserts << " typeID=VALUES(typeID),";
        Inserts << " itemQty=VALUES(itemQty);";
        // execute the command.
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SaveRoutes - unable to save route - %s", err.c_str());
    }
}

void PlanetDB::SavePinContents(uint32 pinID, PI_CCData* pData) {
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO piPinContents";
    Inserts << " (colonyID, pinID, typeID, itemQty) VALUES ";

    bool first = false;
    std::unordered_map<uint32, PI::PinData>::iterator srcItr = pData->pins.find(pinID);
    std::map<uint16, uint32>::iterator itemItr;
    for (itemItr = srcItr->second.contents.begin(); itemItr != srcItr->second.contents.end(); ++itemItr) {
        if (first) {
            Inserts << ", ";
        } else {
            first = true;
        }

        Inserts << "(" << pData->colonyID << ", " << srcItr->first << ", " << itemItr->first << ", " << itemItr->second << ")";
    }

    if (first) {
        // finish creating the command.
        Inserts << " ON DUPLICATE KEY UPDATE";
        Inserts << " itemQty=VALUES(itemQty);";
        // execute the command.
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SavePinContents - unable to save contents - %s", err.c_str());
    }
}

void PlanetDB::SaveAllContents(PI_CCData* pData) {
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO piPinContents";
    Inserts << " (colonyID, pinID, typeID, itemQty) VALUES ";

    bool first = false;
    std::map<uint16, uint32>::iterator itemItr;         //typeID, qty
    for (auto &cur : pData->pins) {
        if (cur.second.update) {
            // delete existing contents in pin
            RemoveContents(cur.first);
            for (itemItr = cur.second.contents.begin(); itemItr != cur.second.contents.end(); ++itemItr) {
                // somehow we're getting qty=0 here and saving that shit...no clue where/why/when yet
                if (itemItr->second < 1)
                    continue;
                if (first) {
                    Inserts << ", ";
                } else {
                    first = true;
                }
                Inserts << "(" << pData->colonyID << ", " << cur.first << ", " << itemItr->first << ", " << itemItr->second << ")";
            }
            cur.second.update = false;
        }
    }

    if (first) {
        // finish creating the command.
        Inserts << " ON DUPLICATE KEY UPDATE";
        Inserts << " itemQty=VALUES(itemQty);";
        // execute the command.
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SaveAllContents - unable to save contents - %s", err.c_str());
    }
}

void PlanetDB::RemovePin(uint32 pinID) {
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM piPins WHERE pinID = %u", pinID);
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE itemID = %u", pinID);
    sDatabase.RunQuery(err, "DELETE FROM entity_attributes WHERE itemID = %u", pinID);
}

void PlanetDB::RemoveHead(uint32 ecuID, uint32 headID) {
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM piECUHeads WHERE ecuID = %u AND headID = %u", ecuID, headID);
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE itemID = %u", headID);
    sDatabase.RunQuery(err, "DELETE FROM entity_attributes WHERE itemID = %u", headID);
}

void PlanetDB::RemoveLink(uint32 linkID) {
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM piLinks WHERE linkID = %u", linkID);
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE itemID = %u", linkID);
    sDatabase.RunQuery(err, "DELETE FROM entity_attributes WHERE itemID = %u", linkID);
}

void PlanetDB::RemoveRoute(uint16 routeID) {
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM piRoutes WHERE routeID = %u", routeID);
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE itemID = %u", routeID);
    sDatabase.RunQuery(err, "DELETE FROM entity_attributes WHERE itemID = %u", routeID);
}

void PlanetDB::RemoveContents(uint32 pinID) {
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM piPinContents WHERE pinID = %u", pinID);
}

void PlanetDB::DeleteLaunch(uint32 contID) {
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM piLaunches WHERE itemID = %u", contID);
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE itemID = %u", contID);
    sDatabase.RunQuery(err, "DELETE FROM entity_attributes WHERE itemID = %u", contID);
}

void PlanetDB::DeleteColony(uint32 colonyID, uint32 planetID, uint32 charID) {
    /** @todo  remove items from entity* table... */
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE locationID = %u AND ownerID = %u", planetID, charID);
    sDatabase.RunQuery(err, "DELETE FROM piPins WHERE colonyID = %u", colonyID);
    sDatabase.RunQuery(err, "DELETE FROM piCCPin WHERE colonyID = %u", colonyID);
    sDatabase.RunQuery(err, "DELETE FROM piLinks WHERE colonyID = %u", colonyID);
    sDatabase.RunQuery(err, "DELETE FROM piRoutes WHERE colonyID = %u", colonyID);
    sDatabase.RunQuery(err, "DELETE FROM piPlanets WHERE colonyID = %u", colonyID);
    sDatabase.RunQuery(err, "DELETE FROM piECUHeads WHERE colonyID = %u", colonyID);
    sDatabase.RunQuery(err, "DELETE FROM piPinContents WHERE colonyID = %u", colonyID);
}
