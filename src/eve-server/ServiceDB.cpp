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
    Updates:     Allan
*/
#include "eve-server.h"

#include "EntityList.h"
#include "EVEServerConfig.h"
#include "ServiceDB.h"

uint32 ServiceDB::SetClientSeed()
{
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT ClientSeed FROM srvStatus WHERE AI = 1");
    DBResultRow row;
    res.GetRow(row);
    sLog.Green( "       ServerInit", "ClientSeed Initialized." );
    return row.GetInt(0);
}

bool ServiceDB::GetAccountInformation( const char* username, const char* password, AccountData &account_info )
{			//added auto account    -allan 18Jan14
    std::string _username = username;
    std::string _escaped_username;

    sDatabase.DoEscapeString(_escaped_username, _username);

    DBQueryResult res;
    if ( !sDatabase.RunQuery( res,
        "SELECT accountID, clientID, password, hash, role, online, banned, logonCount, lastLogin"
        " FROM account WHERE accountName = '%s'", _escaped_username.c_str() ) )
    {
        sLog.Error( "ServiceDB", "Error in query: %s.", res.error.c_str() );
        return false;
    }

    DBResultRow row;
    if (!res.GetRow( row )) {
        // account not found, create new one if autoAccountRole is not zero (0)
        if (sConfig.account.autoAccountRole > 0) {
            uint32 accountID = CreateNewAccount( _username.c_str(), password, sConfig.account.autoAccountRole);
            if ( accountID > 0 ) {
                // add new account successful, get account info again
                bool ret = GetAccountInformation(username, password, account_info);
                return ret;
            } else
                return false;
        } else
            return false;
    }

    account_info.id         = row.GetInt(0);
    account_info.clientID   = row.GetInt(1);

    if (!row.IsNull(2))
        account_info.password = row.GetText(2);

    if (!row.IsNull(3))
        account_info.hash   = row.GetText(3);

    account_info.name       = _escaped_username;
    account_info.role       = row.GetUInt64(4);
    account_info.online     = row.GetBool(5);
    account_info.banned     = row.GetBool(6);
    account_info.visits     = row.GetInt(7);

    if (!row.IsNull(8))
        account_info.last_login = row.GetText(8);

    return true;
}

bool ServiceDB::UpdateAccountHash( const char* username, std::string & hash )
{
    DBerror err;
    std::string user_name = username;
    std::string escaped_hash;
    std::string escaped_username;

    sDatabase.DoEscapeString(escaped_hash, hash);
    sDatabase.DoEscapeString(escaped_username, user_name);

    if (!sDatabase.RunQuery(err, "UPDATE account SET password='',hash='%s' where accountName='%s'",
                                escaped_hash.c_str(), escaped_username.c_str())) {
        sLog.Error( "AccountDB", "Unable to update account information for: %s.", username );
        return false;
    }

    return true;
}

bool ServiceDB::UpdateAccountInformation( const char* username, bool isOnline )
{
    DBerror err;
    std::string user_name = username;
    std::string escaped_username;

    sDatabase.DoEscapeString(escaped_username, user_name);
    if (!sDatabase.RunQuery(err, "UPDATE account SET lastLogin=now(), logonCount=logonCount+1, online=%u where accountName='%s'", isOnline, escaped_username.c_str())) {
        sLog.Error( "AccountDB", "Unable to update account information for: %s.", username );
        return false;
    }

    return true;
}

uint32 ServiceDB::CreateNewAccount( const char* login, const char* pass, uint64 role )
{
    uint32 accountID = 0;
    uint32 clientID = sEntityList.GetClientSeed();

    DBerror err;
    if ( !sDatabase.RunQueryLID( err, accountID,
        "INSERT INTO account ( accountName, hash, role, clientID )"
        " VALUES ( '%s', '%s', %" PRIu64 ", %i )",
        login, pass, role, clientID ) )
    {
        sLog.Error( "ServiceDB", "Failed to create a new account '%s': %s.", login, err.c_str() );
        return 0;
    }

    sDatabase.RunQuery(err, "UPDATE srvStatus SET ClientSeed = ClientSeed + 1 WHERE AI = 1");
    return accountID;
}

/** @todo  all of the following bullshit needs to be checked/updated/deleted as appropriate */
//this function is temporary, I dont plan to keep this crap in the DB.
//   will make mem object for droneState later...   test with this.
PyObject *ServiceDB::GetSolDroneState(uint32 systemID) const {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        //not sure if this is gunna be valid all the time...
        "SELECT "
        "    droneID, solarSystemID, ownerID, controllerID,"
        "    activityState, typeID, controllerOwnerID, targetID"
        " FROM droneState "
        " WHERE solarSystemID=%u",
        systemID)) {
        codelog(DATABASE__ERROR, "Error in GetSolDroneState query: %s", res.error.c_str());
        return NULL;
    }

    return DBResultToRowset(res);
}

uint32 ServiceDB::GetStationOwner(uint32 stationID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT corporationID FROM staStations WHERE stationID = %u", stationID)) {
        codelog(DATABASE__ERROR, "Failed to query info for station %u: %s.", stationID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (res.GetRow(row))
        return row.GetInt(0);
    else
        return 1;
}

bool ServiceDB::GetConstant(const char *name, uint32 &into)
{
    DBQueryResult res;

    std::string escaped;
    sDatabase.DoEscapeString(escaped, name);

    if (!sDatabase.RunQuery(res, "SELECT constantValue FROM eveConstants WHERE constantID='%s'", escaped.c_str() ))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "Unable to find constant %s", name);
        return false;
    }

    into = row.GetUInt(0);

    return true;
}

void ServiceDB::ProcessStringChange(const char * key, const std::string & oldValue, const std::string & newValue, PyDict * notif, std::vector<std::string> & dbQ) {
    if (oldValue != newValue) {
        std::string newEscValue;
        std::string qValue(key);

        sDatabase.DoEscapeString(newEscValue, newValue);

        // add to notification
        PyTuple * val = new PyTuple(2);
        val->items[0] = new PyString(oldValue);
        val->items[1] = new PyString(newValue);
        notif->SetItemString(key, val);

        qValue += " = " + newEscValue;
        dbQ.push_back(qValue);
    }
}

void ServiceDB::ProcessRealChange(const char * key, double oldValue, double newValue, PyDict * notif, std::vector<std::string> & dbQ) {
    if (oldValue != newValue) {
        // add to notification
        std::string qValue(key);

        PyTuple * val = new PyTuple(2);
        val->items[0] = new PyFloat(oldValue);
        val->items[1] = new PyFloat(newValue);
        notif->SetItemString(key, val);

        char cc[10];
        snprintf(cc, 9, "'%5.3lf'", newValue);
        qValue += " = ";
        qValue += cc;
        dbQ.push_back(qValue);
    }
}

void ServiceDB::ProcessIntChange(const char * key, uint32 oldValue, uint32 newValue, PyDict * notif, std::vector<std::string> & dbQ) {
    if (oldValue != newValue) {
        // add to notification
        PyTuple * val = new PyTuple(2);
        std::string qValue(key);

        val->items[0] = new PyInt(oldValue);
        val->items[1] = new PyInt(newValue);
        notif->SetItemString(key, val);

        char cc[10];
        snprintf(cc, 9, "%u", newValue);
        qValue += " = ";
        qValue += cc;
        dbQ.push_back(qValue);
    }
}

void ServiceDB::SetCharacterOnlineStatus(uint32 char_id, bool online) {
    _log(CLIENT__TRACE, "ServiceDB:  Setting character %u %s.", char_id, online ? "Online" : "Offline");
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE chrCharacter SET online = %d WHERE characterID = %u", online, char_id);

    if ( online )
        sDatabase.RunQuery(err, "UPDATE srvStatus SET Connections = Connections + 1");
}

void ServiceDB::SetServerOnlineStatus(bool online) {
    DBerror err;
    sDatabase.RunQuery(err,
        "UPDATE srvStatus SET Online = %d, Connections = 0, startTime = %s WHERE AI = 1",
        online, online ? "UNIX_TIMESTAMP(CURRENT_TIMESTAMP)" : 0);

    //this is only called on startup/shutdown.  reset all char online counts/status'
    sDatabase.RunQuery(err,
        "UPDATE chrCharacter, account"
        " SET chrCharacter.online = 0,"
        "     account.online = 0");

    sDatabase.RunQuery( err,
        "DELETE FROM chrPausedSkillQueue"
        " WHERE 1");
}

void ServiceDB::SetAccountOnlineStatus(uint32 accountID, bool online) {
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "UPDATE account "
        " SET online = %d"
        " WHERE accountID= %u ",
        online, accountID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", err.c_str());
    }
}

void ServiceDB::SetAccountBanStatus(uint32 accountID, bool banned) {
    DBerror err;
    if (!sDatabase.RunQuery(err,
        " UPDATE account"
        " SET banned = %d"
        " WHERE accountID = %u",
        banned, accountID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", err.c_str());
    }
}

void ServiceDB::SaveServerStats(double threads, float rss, float vm, float user, float kernel, uint32 items, uint32 bubbles) {
  DBerror err;
  sDatabase.RunQuery(err,
	"UPDATE srvStatus"
	" SET threads = %f,"
	"     rss = %f,"
	"     vm = %f,"
	"     user = %f,"
	"     kernel = %f,"
	"     items = %u,"
    "     bubbles = %u,"
	"     systems = %u,"
    "     npcs = %u,"
    //"     Connections = %u,"
	"     updateTime = UNIX_TIMESTAMP(CURRENT_TIMESTAMP)"
	" WHERE AI = 1",
	    threads, rss, vm, user, kernel, items, bubbles, sEntityList.GetSystemCount(), sEntityList.GetNPCCount()/*, sEntityList.GetConnections()*/);

  if (sConfig.server.UseProfiling)
      _log(DATABASE__MESSAGE, "Server Stats Saved");
}

