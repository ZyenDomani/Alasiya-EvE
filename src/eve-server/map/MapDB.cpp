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
    Author:        Zhur, Allan
*/

#include "eve-server.h"

#include "map/MapDB.h"

PyObject *MapDB::GetPseudoSecurities() {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res, "SELECT solarSystemID, security FROM mapSolarSystems")) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyObject *MapDB::GetStationExtraInfo() {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "   stationID,"
        "   solarSystemID,"
        "   operationID,"
        "   stationTypeID,"
        "   corporationID AS ownerID"
        " FROM staStations" )) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyObject *MapDB::GetStationOpServices() {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT operationID, serviceID FROM staOperationServices")) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

PyObject *MapDB::GetStationServiceInfo() {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT serviceID, serviceName FROM staServices ")) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

void MapDB::GetStationCount(DBQueryResult& res)
{
    if(!sDatabase.RunQuery(res,
        "SELECT map.solarSystemID, count(sta.stationID)"
        " FROM mapSolarSystems AS map"
        "  LEFT JOIN staStations AS sta USING(solarSystemID)"
        " GROUP BY map.solarSystemID"))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
    }
}

PyObject *MapDB::GetSolSystemVisits(uint32 charID)
{
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        " SELECT"
        "   solarSystemID,"
        "   visits,"
        "   lastDateTime"
        " FROM chrVisitedSystems"
        " WHERE characterID = %u", charID ))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);  //DBRowToKeyVal
}

//  called from MapService by multiple functions based on passed values.
///  added jumpsHour and numPilots data inserts.  16Mar14
///  added killsHour, factionKills, podKillsHour  24Mar14
///  NOTE: DB has fields for timing the *Hour and *24Hour parts. need to write checks for that once everything else is working.
///    NOTE:   use averages for *Hour based on current data and serverUpTime.   may be able to do 24Hour same way.
PyRep *MapDB::GetDynamicData(uint8 type, uint8 time) {
  /*   object#  0 = type   1 = timeframe
solarSystemID
moduleCnt
structureCnt
pilotsDocked
pilotsInSpace
jumpsHour
killsHour
kills24Hour
factionKills
factionKills24Hour
podKillsHour
podKills24Hour
pilotsDateTime
jumpsDateTime
killsDateTime
kills24DateTime
podDateTime
pod24DateTime
factionDateTime
faction24DateTime
    */
    DBQueryResult res;
    if( (type == 1) && (time == 1) )
        sDatabase.RunQuery(res, "SELECT solarSystemID, jumpsHour AS value1 FROM mapDynamicData" );
    else if (type == 2) {
        //DBResultRow row;
        sDatabase.RunQuery(res, "SELECT solarSystemID, moduleCnt, structureCnt FROM mapDynamicData" );
        //res.GetRow(row);
        //return DBResultToCRowset(res);
    } else if (type == 3) {
      if (time == 1)
          sDatabase.RunQuery(res, "SELECT solarSystemID, killsHour AS value1, factionKills AS value2, podKillsHour AS value3 FROM mapDynamicData" );
      else if (time == 24)
          sDatabase.RunQuery(res, "SELECT solarSystemID, kills24Hour AS value1, factionKills24Hour AS value2, podKills24Hour AS value3 FROM mapDynamicData" );
    } else if (type == 5)
        sDatabase.RunQuery(res, "SELECT solarSystemID, killsHour AS value1, factionKills AS value2, kills24Hour AS value3, factionKills24Hour AS value4, podKills24Hour AS value5 FROM mapDynamicData" );
    else
        return nullptr;

    return DBResultToRowset(res);
}

// for MapData class
void MapDB::GetSystemJumps(DBQueryResult& res)
{
    //sDatabase.RunQuery(res, "SELECT ctype, fromreg, fromcon, fromsol, tosol, tocon, toreg FROM mapConnections");
    sDatabase.RunQuery(res, "SELECT ctype, fromsol, tosol FROM mapConnections");
}

// methods and queries for Solar System Status page -- updated/moved 26Nov18
void MapDB::chkDynamicSystemID(uint32 solarSystemID) {
    /**  this ensures mapDynamicData.solarSystemID for `solarSystemID` is in the DB for later calls. -allan 16Mar14 */
    DBQueryResult chk;
    sDatabase.RunQuery(chk, "SELECT solarSystemID FROM mapDynamicData WHERE solarSystemID = %u", solarSystemID );

    DBResultRow row;
    if (!chk.GetRow(row)) {
        DBerror err;
        sDatabase.RunQuery(err, "INSERT INTO mapDynamicData (solarSystemID) VALUES (%u)", solarSystemID );
    }
}

/** the following functions rely on solarSystemID being in the mapDynamicData table.
 * the check is called before these are used, and solarSystemID is then verified for existance and added if needed.
 *   the function is as follows and is declared above...
 *         void SystemDB::chkDynamicSystemID(uint32 solarSystemID)
 *
 *  NOTE: these will have to be reset each server start for true dynamic data tracking
 *        really should trunicate table on restart after everything is working.
 */

void MapDB::SetSystemActive(uint32 sysID, bool active/*false*/)
{
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE mapDynamicData SET active = %u WHERE solarSystemID = %u", active?1:0, sysID );
}


void MapDB::AddJumpToDynamicData(uint32 solarSystemID) {
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE mapDynamicData SET jumpsHour = jumpsHour + 1 WHERE solarSystemID = %u", solarSystemID );
}

void MapDB::AddPilotToDynamicData(uint32 solarSystemID, bool isAdd/*false*/, bool isDocked/*false*/, bool isLogin/*false*/) {
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT pilotsDocked, pilotsInSpace FROM mapDynamicData WHERE solarSystemID = %u", solarSystemID );

    DBResultRow row;
    uint16 docked = 0, space = 0;
    if (res.GetRow(row)) {
        docked = row.GetUInt(0);
        space = row.GetUInt(1);
    }

    /* start clever coding   compounding booleans ftw!!  */
    (isLogin ? (isDocked ? ++docked : ++space) : (isDocked ? (isAdd ? docked++, space-- : docked--) : (isAdd ? docked--, space++ : space--)));
    /* end clever coding */

    if (docked < 0 || docked > 100) docked = 0;
    if (space < 0 || space > 100) space = 0;

    DBerror err;
    sDatabase.RunQuery(err,
                "UPDATE mapDynamicData SET pilotsDocked = %u, pilotsInSpace = %u, pilotsDateTime = %f WHERE solarSystemID = %u",
                docked, space, GetFileTimeNow(), solarSystemID );
}

//  client logs faction kills in total kills.  return is value1(total kills) - value2(faction kills) > 0:
void MapDB::AddKillToDynamicData(uint32 solarSystemID) {  /**killsHour, kills24Hours */
    DBerror err;
    sDatabase.RunQuery(err,
                       "UPDATE mapDynamicData SET killsHour = killsHour + 1, kills24Hour = kills24Hour + 1, kills24DateTime = %f WHERE solarSystemID = %u",
                       GetFileTimeNow(), solarSystemID );
}

void MapDB::AddFactionKillToDynamicData(uint32 solarSystemID) {     /**factionKills*/
    DBerror err;
    sDatabase.RunQuery(err,
                       "UPDATE mapDynamicData SET factionKills = factionKills + 1, factionKills24Hour = factionKills24Hour + 1, faction24DateTime = %f WHERE solarSystemID = %u",
                       GetFileTimeNow(), solarSystemID );
}

void MapDB::AddPodKillToDynamicData(uint32 solarSystemID) {   /**podKillsHour, podKills24Hour */
    DBerror err;
    sDatabase.RunQuery(err,
                       "UPDATE mapDynamicData SET podKillsHour = podKillsHour + 1, podKills24Hour = podKills24Hour + 1, pod24DateTime = %f WHERE solarSystemID = %u",
                       GetFileTimeNow(), solarSystemID );
}

void MapDB::GetActivePilotsFromDynamicData(uint32 solarSystemID, uint16 &pilotsDocked, uint16 &pilotsInSpace) {
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT pilotsDocked, pilotsInSpace FROM mapDynamicData WHERE solarSystemID = %u", solarSystemID );

    _log(DATABASE__RESULTS, "GetActivePilotsFromDynamicData query returned %u items", res.GetRowCount());

    DBResultRow row;
    if (res.GetRow(row)) {
        pilotsDocked = row.GetUInt(0);
        pilotsInSpace = row.GetUInt(1);
    }
}
