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

#ifndef __SERVICEDB_H_INCL__
#define __SERVICEDB_H_INCL__

#include "eve-server.h"
#include "ServiceStruct.h"
#include "EntityList.h"

/**
 * This object is the home for common DB operations which may be needed by many
 * different service objects. It should be inherited by each serviceDB
 * implementation.
 *
 */

class PyPackedRow;
class PyObject;
class ItemFactory;

class CharacterData;
class CorpMemberInfo;

struct AccountInfo
{
    int32 id;
    uint64 role;
    int32 visits;
    int32 clientID;
    std::string name;
    std::string hash;
    std::string password;
    std::string last_login;
    bool online;
    bool banned;
};

class ServiceDB
{
public:

    bool GetAccountInformation( const char* username, const char* password, AccountInfo &account_info );
    bool UpdateAccountHash( const char* username, std::string &hash );
    bool UpdateAccountInformation( const char* username, bool isOnline );

    //destiny setstate stuff:
    PyPackedRow *GetSolItem(uint32 systemID) const;
    PyObject *GetSolDroneState(uint32 systemID) const;

    bool GetSystemInfo(uint32 systemID, uint32 *constellationID, uint32 *regionID, std::string *name, std::string *securityClass, double *securityRating);    // mapSolarSystems
    bool GetStaticItemInfo(uint32 itemID, uint32 *systemID, uint32 *constellationID, uint32 *regionID, GPoint *position);    // mapDenormalize
    bool GetStationInfo(uint32 stationID, uint32 *systemID, uint32 *constellationID, uint32 *regionID, GPoint *position,
                        GPoint *dockPosition, GVector *dockOrientation);    // staStations
    uint32 GetStationOwner(uint32 stationID);

    uint32 GetDestinationStargateID(uint32 fromSystem, uint32 toSystem);

    bool GetConstant(const char *name, uint32 &into);

    //these really want to move back into AccountDB
    bool GiveCash( uint32 characterID, JournalRefType refTypeID, uint32 ownerFromID, uint32 ownerToID, const char *argID1,
                   uint32 accountID, EVEAccountKeys accountKey, double amount, double balance, const char *reason);
    double GetCorpBalance(uint32 corpID, uint16 accountKey);
    bool AddBalanceToCorp(uint32 corpID, double amount);

    void SetServerOnlineStatus(bool online);
    void SetCharacterOnlineStatus(uint32 char_id, bool online);
    void SetAccountOnlineStatus(uint32 accountID, bool online);
    void SetAccountBanStatus(uint32 accountID, bool banned);

    void SaveServerStats(double threads, float rss, float vm, float user, float kernel, uint32 items, uint32 systems, uint32 bubbles, uint32 npcs);

    uint32 SetClientSeed();

protected:
    void ProcessStringChange(const char * key, const std::string & oldValue, const std::string & newValue, PyDict * notif, std::vector<std::string> & dbQ);
    void ProcessRealChange(const char * key, double oldValue, double newValue, PyDict * notif, std::vector<std::string> & dbQ);
    void ProcessIntChange(const char * key, uint32 oldValue, uint32 newValue, PyDict * notif, std::vector<std::string> & dbQ);

    /**
     * CreateNewAccount
     *
     * This method is part of the "autoAccount" creation patch by firefoxpdm. This
     * will insert a new account row into the database if the account name doesn't
     * exist at login.
     *
     * @param login is a const char string containing the name.
     * @param pass is a const char string containing the password.
     * @param role is the users role in the game.
     * @author firefoxpdm, xanarox
     */
    uint32 CreateNewAccount( const char* login, const char* pass, uint64 role );

private:
    //sServiceStruct sSDB;
};

#endif


