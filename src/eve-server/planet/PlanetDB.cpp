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


#include "planet/Colony.h"
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

void PlanetDB::UpdatePlanetsForChar(uint32 solarSystemID, uint32 planetID, uint32 charID, uint16 typeID, uint8 pins/*1*/)
{
    DBerror err;
    if(!sDatabase.RunQuery(err,
        "INSERT INTO chrPlanets (solarSystemID, planetID, charID, typeID, numberOfPins)"
        " VALUES (%u, %u, %u, %u, %u)"
        " ON DUPLICATE KEY UPDATE "
        " numberOfPins=VALUES(numberOfPins)",
         solarSystemID, planetID, charID, typeID, pins))
    {
        _log(DATABASE__ERROR, "SaveCommandCenter - Unable to save CommandCenter: %s", err.GetError());
    }
}


PyRep* PlanetDB::GetMyLaunchesDetails(uint32 charID) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT launchID, solarSystemID, planetID, launchTime, x, y, z"
        " FROM chrPlanetLaunches WHERE charID = %u", charID)) {
        _log(DATABASE__ERROR, "Error in GetMyLaunchesDetails Query: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToRowset(res);
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

void PlanetDB::GetExtractorsForPlanet(uint32 planetID, DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT `typeID`, `ownerID`,`latitude`, `longitude`"
        " FROM `chrPlanetPins`"
        " WHERE ccPinID IN"
        " (SELECT `pinID` FROM `chrPlanetCCPin`"
        " WHERE `planetID` = %u)"
        " AND `isExtractor` = 1",
        planetID))
    {
        _log(DATABASE__ERROR, "Error in LoadPins Query: %s", res.error.c_str());
        return;
    }

    _log(DATABASE__RESULTS, "GetExtractorsForPlanet returned %u items", res.GetRowCount());
}

bool PlanetDB::LoadColony(uint32 charID, uint32 planetID, PI_CCPin* ccPin)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT `pinID`,`level`, `lastSimTime`"
        " FROM `chrPlanetCCPin`"
        " WHERE `charID` = %u"
        " AND `planetID` = %u",
        charID, planetID ))
    {
        _log(DATABASE__ERROR, "Error in LoadPins Query: %s", res.error.c_str());
        return false;
    }
    DBResultRow row;
    if (res.GetRow(row)) {
        ccPin->level = row.GetInt(1);
        ccPin->ccPinID = row.GetInt(0);
        ccPin->currentSimTime = row.GetUInt64(2);
        return true;
    } else
        return false;
}

void PlanetDB::LoadPins(uint32 ccPinID, std::map<uint32, PI_Pin>& pins)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT `pinID`, `typeID`, `ownerID`, `state`, `level`, `latitude`, `longitude`, "
        " `isCommandCenter`, `isLaunchable`, `isProcess`, `isExtractor`, `isStorage`, `isLink`, `isECU`,"
        " `hasReceivedInputs`, `receivedInputsLastCycle`,"
        " `heads`, `schematicID`, `resTypeID`, `qtyPerCycle`, `headRadius`,"
        " `launchTime`, `cycleTime`, `expiryTime`, `installTime`, `lastRunTime`"
        " FROM `chrPlanetPins`"
        " WHERE ccPinID = %u", ccPinID))
    {
        _log(DATABASE__ERROR, "Error in LoadPins Query: %s", res.error.c_str());
        return;
    }

    _log(DATABASE__RESULTS, "LoadPins returned %u items", res.GetRowCount());

    /** @todo  load saved contents to pin */

    DBResultRow row;
    PI_Pin pin;
    while (res.GetRow(row)) {
        pin.typeID                  = row.GetInt(1);
        pin.ownerID                 = row.GetInt(2);
        pin.state                   = row.GetInt(3);
        pin.level                   = row.GetInt(4);
        pin.latitude                = row.GetFloat(5);
        pin.longitude               = row.GetFloat(6);
        pin.isCommandCenter         = row.GetBool(7);
        pin.isLaunchable            = row.GetBool(8);
        pin.isProcess               = row.GetBool(9);
        pin.isExtractor             = row.GetBool(10);
        pin.isStorage               = row.GetBool(11);
        pin.isLink                  = row.GetBool(12);
        pin.isECU                   = row.GetBool(13);
        pin.hasReceivedInputs       = row.GetBool(14);
        pin.receivedInputsLastCycle = row.GetBool(15);
        pin.schematicID             = row.GetInt(17);
        pin.resTypeID               = row.GetInt(18);
        pin.qtyPerCycle             = row.GetFloat(19);
        pin.headRadius              = row.GetFloat(20);
        pin.lastLaunchTime          = row.GetUInt64(21);
        pin.cycleTime               = row.GetUInt64(22);
        pin.expiryTime              = row.GetUInt64(23);
        pin.installTime             = row.GetUInt64(24);
        pin.lastRunTime             = row.GetUInt64(25);

        std::string headIDs;
        headIDs.clear();
        std::list<uint32> headList;
        headList.clear();
        headIDs = row.GetText(16);
        if (!headIDs.empty()) {
            // headIDs string is not empty, so extract one number at a time until it is
            int pos = 0;
            std::string tempString = "";

            pos = headIDs.find_first_of(':');
            if (pos < 0)
                pos = headIDs.length()-1;    // we did not find any ';' characters, so headIDs contains only one number
                tempString = headIDs.substr(0,pos);

            while ((pos = headIDs.find_first_of(';')) > 0 ) {
                tempString = headIDs.substr(0,pos);
                headList.insert(headList.begin(), (atoi(tempString.c_str())));
                headIDs = headIDs.substr(pos+1,headIDs.length()-1);
            }

            // Get final number now that there are no more separators to find:
            if (!headIDs.empty())
                headList.insert(headList.end(), (atoi(headIDs.c_str())));

            for (auto cur : headList) {
                PI_Heads piHead;
                pin.heads[cur] = piHead;
            }
        }

        pins.emplace(std::pair<uint32, PI_Pin>(row.GetInt(0), pin));
    }
}

void PlanetDB::LoadLinks(uint32 ccPinID, std::map<uint32, PI_Link >& links)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT `linkID`, `level`, `state`, `typeID`, `endpoint1`, `endpoint2`"
        " FROM `chrPlanetLinks` WHERE `ccPinID` = %u",
        ccPinID ))
    {
        _log(DATABASE__ERROR, "Error in LoadPins Query: %s", res.error.c_str());
        return;
    }

    _log(DATABASE__RESULTS, "LoadLinks returned %u items", res.GetRowCount());

    DBResultRow row;
    PI_Link link;
    if (res.GetRow(row)) {
        link.level = row.GetInt(1);
        link.state = row.GetInt(2);
        link.typeID = row.GetInt(3);
        link.endpoint1 = row.GetInt(4);
        link.endpoint2 = row.GetInt(5);
    }

    links.insert(std::pair<uint32, PI_Link>(row.GetInt(0), link));
}

void PlanetDB::LoadRoutes(uint32 ccPinID, std::map<uint32, PI_Route >& routes)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT `routeID`, `state`, `priority`, `path`, `itemID`, `itemQty`"
        " FROM `chrPlanetRoutes` WHERE `ccPinID` = %u",
        ccPinID ))
    {
        _log(DATABASE__ERROR, "Error in LoadPins Query: %s", res.error.c_str());
        return;
    }

    _log(DATABASE__RESULTS, "LoadRoutes returned %u items", res.GetRowCount());

    DBResultRow row;
    std::string tempPath;
    std::list<uint32> headList;
    while (res.GetRow(row)) {
        tempPath.clear();
        headList.clear();
        PI_Route route;
            route.id = row.GetInt(0);
            route.state = row.GetInt(1);
            route.priority = row.GetInt(2);
            route.commodityTypeID = row.GetInt(4);
            route.commodityQuantity = row.GetInt(5);

        tempPath = row.GetText(3);
        if (!tempPath.empty()) {
            // headIDs string is not empty, so extract one number at a time until it is
            int pos = 0;
            std::string tempString = "";

            while ((pos = tempPath.find_first_of(':')) > 0 ) {
                tempString = tempPath.substr(0,pos);
                tempPath = tempPath.substr(pos+1,tempPath.length()-1);
                headList.insert(headList.end(), (atoi(tempString.c_str())));
            }
        }
        routes.insert(std::pair<uint32, PI_Route>(headList.back(), route));
    }
}

void PlanetDB::SaveLaunch(uint32 charID, uint32 systemID, uint32 planetID, GPoint& pos)
{
    DBerror err;
    if(!sDatabase.RunQuery(err,
        "INSERT INTO `chrPlanetLaunches`(`charID`, `solarSystemID`, `planetID`, `launchTime`, `x`, `y`, `z`) "
        " VALUES (%u, %u, %u, %" PRIu64 ", %f, %f, %f)",
        charID, systemID, planetID, Win32TimeNow(), pos.x, pos.y, pos.z))
        {
            _log(DATABASE__ERROR, "SaveLaunch - Unable to save Launch: %s", err.GetError());
        }
}

void PlanetDB::SaveCommandCenter(uint32 pinID, uint32 charID, uint32 planetID, uint32 typeID, float latitude, float longitude)
{
    DBerror err;
    if(!sDatabase.RunQuery(err,
        "INSERT INTO chrPlanetCCPin (pinID, charID, planetID, typeID, latitude, longitude, state, level, lastSimTime) "
        " VALUES (%u, %u, %u, %u, %f, %f, 1, 0, %" PRIu64 " )",
        pinID, charID, planetID, typeID, latitude, longitude, Win32TimeNow()))
    {
        _log(DATABASE__ERROR, "SaveCommandCenter - Unable to save CommandCenter: %s", err.GetError());
    }
}

void PlanetDB::SaveCCLevel(uint32 pinID, uint8 level)
{
    DBerror err;
    if(!sDatabase.RunQuery(err, "UPDATE chrPlanetCCPin SET level = %u WHERE pinID = %u", level, pinID))
    {
        _log(DATABASE__ERROR, "SaveCCLevel - Unable to save CCLevel: %s", err.GetError());
    }
}

void PlanetDB::SavePins(PI_CCPin* ccPin)
{
    /** @todo  add contents to saved pin */
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO `chrPlanetPins`";
    Inserts << " (`ccPinID`, `pinID`, `typeID`, `ownerID`, `level`, `latitude`, `longitude`,";
    Inserts << " `isCommandCenter`, `isLaunchable`, `isProcess`, `isExtractor`, `isStorage`, `isECU`, `isLink`,";
    Inserts << " `hasReceivedInputs`, `receivedInputsLastCycle`,";
    Inserts << " `schematicID`, `resTypeID`, `qtyPerCycle`, `headRadius`,";
    Inserts << " `launchTime`, `cycleTime`, `expiryTime`, `installTime`, `lastRunTime`)";

    bool first = true;
    uint32 ccPinID = ccPin->ccPinID;
    for (auto cur : ccPin->pins) {
        if (first) {
            Inserts << " VALUES ";
            first = false;
        } else
            Inserts << ", ";
        Inserts << "(" << ccPinID << ", " << cur.first << ", " << cur.second.typeID << ", " << cur.second.ownerID << ", " << (cur.second.level ? cur.second.level : 0) << ", " << cur.second.latitude << ", " << cur.second.longitude << ", ";
        Inserts << cur.second.isCommandCenter << ", " << cur.second.isLaunchable << ", " << cur.second.isProcess << ", " << cur.second.isExtractor << ", " << cur.second.isStorage <<", " << cur.second.isECU << ", " << cur.second.isLink << ", ";
        Inserts << cur.second.hasReceivedInputs << ", " << cur.second.receivedInputsLastCycle << ", ";
        Inserts << cur.second.schematicID << ", " << cur.second.resTypeID << ", " << cur.second.qtyPerCycle << ", " << cur.second.headRadius << ", ";
        Inserts << cur.second.lastLaunchTime << ", " << cur.second.cycleTime << ", " << cur.second.expiryTime << ", " << cur.second.installTime << ", " << cur.second.lastRunTime << ")";
    }

    if (!first) {
        // finish creating the command.
        Inserts << " ON DUPLICATE KEY UPDATE ";
        Inserts << " hasReceivedInputs=VALUES(hasReceivedInputs),";
        Inserts << " receivedInputsLastCycle=VALUES(receivedInputsLastCycle),";
        Inserts << " heads=VALUES(heads),";
        Inserts << " schematicID=VALUES(schematicID), ";
        Inserts << " resTypeID=VALUES(resTypeID),";
        Inserts << " qtyPerCycle=VALUES(qtyPerCycle), ";
        Inserts << " headRadius=VALUES(headRadius),";
        Inserts << " launchTime=VALUES(launchTime), ";
        Inserts << " cycleTime=VALUES(cycleTime),";
        Inserts << " expiryTime=VALUES(expiryTime), ";
        Inserts << " installTime=VALUES(installTime),";
        Inserts << " lastRunTime=VALUES(lastRunTime);";
        // execute the command.
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SavePins - unable to save pins: %s", err.c_str());
    }
}

void PlanetDB::SaveHeads(std::map< uint32, PI_Heads >& heads)
{
    if (heads.empty())
        return;

    std::ostringstream head;
    head.clear();
    bool first = true;
    std::map<uint32, PI_Heads>::iterator itr = heads.begin();
    for (; itr != heads.end(); itr++) {
        if (first) {
            first = false;
            head << itr->first;
        } else
            head << ":" << itr->first;
    }
    itr = heads.begin();
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "INSERT INTO `chrPlanetPins` (`pinID`, `heads`)"
        " VALUES (%u, '%s')"
        " ON DUPLICATE KEY UPDATE "
        " heads=VALUES(heads)",
        itr->first, head.str().c_str()))
    {
        _log(DATABASE__ERROR, "SaveHeads - Unable to save heads: %s", err.GetError());
    }
}

void PlanetDB::SavePinLevel(uint32 pinID, uint8 level)
{
    DBerror err;
    if(!sDatabase.RunQuery(err, "UPDATE chrPlanetPins SET level = %u WHERE pinID = %u", level, pinID))
    {
        _log(DATABASE__ERROR, "SavePinLevel - Unable to save PinLevel: %s", err.GetError());
    }
}

void PlanetDB::SaveLinks(PI_CCPin* ccPin)
{
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO `chrPlanetLinks`";
    Inserts << " (`ccPinID`, `linkID`, `level`, `typeID`, `endpoint1`, `endpoint2`)";

    bool first = true;
    uint32 ccPinID = ccPin->ccPinID;
    for (auto cur : ccPin->links) {
        if (first) {
            Inserts << " VALUES ";
            first = false;
        } else
            Inserts << ":";
        Inserts << "(" << ccPinID << ", " << cur.first << ", " << (cur.second.level ? cur.second.level : 0) << ", ";
        Inserts << cur.second.typeID << ", " << cur.second.endpoint1 << ", " << cur.second.endpoint2 << ")";

        if (!first) {
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
}

void PlanetDB::SaveLinkLevel(uint32 linkID, uint8 level)
{
    DBerror err;
    if(!sDatabase.RunQuery(err, "UPDATE chrPlanetLinks SET level = %u WHERE linkID = %u", level, linkID))
    {
        _log(DATABASE__ERROR, "SaveLinkLevel - Unable to save LinkLevel: %s", err.GetError());
    }
}

void PlanetDB::SaveRoutes(PI_CCPin* ccPin)
{
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO `chrPlanetRoutes`";
    Inserts << " (`ccPinID`, `routeID`, `priority`, `path`, `itemID`, `itemQty`)";

    bool first = true;
    std::string path;
    std::list<uint32>::iterator itr;
    uint32 ccPinID = ccPin->ccPinID;
    for (auto cur : ccPin->routes) {
        path.clear();
        if (first) {
            Inserts << " VALUES ";
            first = false;
        } else
            Inserts << ":";
        for (itr = cur.second.path.begin(); itr != cur.second.path.end(); itr++) {
            path += itoa(*itr);
            if ((itr++) != cur.second.path.end())
                path += ":";
        }
        Inserts << "(" << ccPinID << ", " << cur.first << ", '" << cur.second.priority << ", '";
        Inserts << path << "', " << cur.second.commodityTypeID << ", " << cur.second.commodityQuantity << ")";

        if (!first) {
            // finish creating the command.
            Inserts << " ON DUPLICATE KEY UPDATE";
            Inserts << " path=VALUES(path),";
            Inserts << " itemID=VALUES(itemID),";
            Inserts << " itemQty=VALUES(itemQty);";
            // execute the command.
            DBerror err;
            if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
                _log(DATABASE__ERROR, "SaveRoutes - unable to save route - %s", err.c_str());
        }
    }
}

void PlanetDB::RemovePin(uint32 pinID)
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM `chrPlanetPins` WHERE `pinID` = %u", pinID);
}

void PlanetDB::RemoveHead(uint32 pinID)
{
    // will have to delete current db data then save new data after head deletion

}

void PlanetDB::RemoveLink(uint32 linkID)
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM `chrPlanetLinks` WHERE `linkID` = %u", linkID);
}

void PlanetDB::RemoveRoute(uint8 routeID)
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM `chrPlanetRoutes` WHERE `routeID` = %u", routeID);
}

void PlanetDB::DeleteColony(uint32 ccPinID, uint32 planetID, uint32 charID)
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM `entity` WHERE `locationID` = %u AND `ownerID` = %u", planetID, charID);
    sDatabase.RunQuery(err, "DELETE FROM `chrPlanets` WHERE `planetID` = %u AND `charID` = %u", planetID, charID);
    sDatabase.RunQuery(err, "DELETE FROM `chrPlanetCCPin` WHERE `pinID` = %u", ccPinID);
    sDatabase.RunQuery(err, "DELETE FROM `chrPlanetPins` WHERE `ccPinID` = %u", ccPinID);
    sDatabase.RunQuery(err, "DELETE FROM `chrPlanetLinks` WHERE `ccPinID` = %u", ccPinID);
    sDatabase.RunQuery(err, "DELETE FROM `chrPlanetRoutes` WHERE `ccPinID` = %u", ccPinID);
}
