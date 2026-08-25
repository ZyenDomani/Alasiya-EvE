/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    Copyright 2016 - 2026 Alasiya-EvE by Allan
    For the latest implementation status visit http://eve.alasiya.net/?p=op_status
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
    Author:        Zhur, Aknor Jaden
    Rewrite:    Allan
*/


#ifndef __LSCDB_H_INCL__
#define __LSCDB_H_INCL__

#include "ServiceDB.h"
#include "EVE_LSC.h"

struct AclEntry;

class Client;
class LSCService;
class LSCChannel;

class LSCDB
: public ServiceDB
{
public:
    LSCDB() = default;
    LSCDB(LSCDB&&) =delete;
    LSCDB(const LSCDB&) =delete;
    LSCDB& operator=(LSCDB&&) =delete;
    LSCDB& operator=(const LSCDB&) =delete;
    ~LSCDB() = default;

    // load channelID data
    int32 GetHighestChannelIDFromDB();


    //TODO:  update this bullshit
    std::string GetRegionName(uint32 id) { return GetChannelName(id, "mapRegions", "regionName", "regionID"); }
    std::string GetConstellationName(uint32 id) { return GetChannelName(id, "mapConstellations", "constellationName", "constellationID"); }
    std::string GetSolarSystemName(uint32 id) { return GetChannelName(id, "mapSolarSystems", "solarSystemName", "solarSystemID"); }
    std::string GetCorporationName(uint32 id) { return GetChannelName(id, "crpCorporation", "corporationName", "corporationID"); }
    std::string GetAllianceName(uint32 id) { return GetChannelName(id, "alnAlliance", "shortName", "allianceID"); }
    std::string GetCharacterName(uint32 id) { return GetChannelName(id, "chrCharacters", "characterName", "charID"); }

    void GetChannelSubscriptions(uint32 charID, std::vector<LSC::ChannelData>& subscriptions);

    ///************  new
    void UpdateChannelInfo(LSCChannel* channel);
    bool SaveChannelACL(int32 channelID, const AclEntry* acl);
    bool RemoveChannelACL(int32 channelID, uint32 accessorID);
    bool LoadChannelACL(int32 channelID, std::unordered_map<uint32, AclEntry*>& aclMap);
    void UpdateChannelMode(int32 channelID, int32 rawModeVal);
    void UpdateUserChannelAccess(int32 channelID, uint32 targetCharID, int32 rawModeVal);
    bool IsChannelSubscribedByThisChar(uint32 characterID, int32 channelID);
    void UpdateSubscription(int32 channelID, Client* pClient);

    // --- 1. DYNAMIC AUTO-INCREMENT KEY ASSIGNMENTS ---
    bool IsChannelNameAvailable(const std::string& name);
    bool IsChannelIDAvailable(int32 channelID);

/*
    // --- 2. FAST RUNTIME DISCOVERY QUERIES (Executed ONLY on Initial Node Boot Pass) ---
    bool IsChannelSubscribedByThisChar(uint32 characterID, int32 channelID);
    bool GetChannelData(int32 channelID, ChannelData& data);
    bool GetChannelSubscriptionData(int32 channelID, SubscriptionData& data);
    bool LoadChannelACL(int32 channelID, std::unordered_map<uint32, AclEntry*>& aclMap);

    // Bulk-extracts active subscriptions for a pilot during character connection passes [1.0]
    void GetChannelSubscriptions(uint32 characterID, std::vector<SubscriptionData>& subscriptions);

    // --- 3. DYNAMIC DATA SYNCHRONIZATION AND STATE PERSISTENCE ---
    // Persists custom folder modifications, topic banners, or password updates safely
    void UpdateChannelInfo(LSCChannel* channel);
    void UpdateChannelMode(int32 channelID, int32 rawModeVal);
    void UpdateUserChannelAccess(int32 channelID, uint32 targetCharID, int32 rawModeVal);
    void UpdateSubscription(int32 channelID, Client* pClient);
    bool SaveChannelACL(int32 channelID, const AclEntry* acl);
    bool RemoveChannelACL(int32 channelID, uint32 accessorID);

*/

    // --- 4. DATA PURGE ENDPOINTS ---
    void DeleteChannel(int32 channelID);
    void DeleteSubscription(int32 channelID, int32 charID);
    void ForgetChannel(int32 charID, int32 channelID);

protected:
    std::string GetChannelName(uint32 id, const char* table, const char* column, const char* key);

};


#endif
