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
    Updates:    Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "EntityList.h"
#include "EVEServerConfig.h"
#include "ServiceDB.h"
#include "market/MarketBotMgr.h"
#include "system/DestinyManager.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/WormholeMgr.h"
#include "system/cosmicMgrs/ManagerDB.h"

EntityList::EntityList()
: m_services( nullptr ),
m_stamp(1000),    /* start at 1k.  in seconds.  used for destiny and client counters */
m_stampTimer(1000, true)    /* in ms */
{
    m_systems.clear();
    m_clients.clear();
    m_stations.clear();

    m_connections = 0;
    m_shipTracking = sConfig.server.UseShipTracking;
}

EntityList::~EntityList() {
    sLog.Success("   ServerShutdown", " Complete.");
}

void EntityList::Init() {
    /* start the timer */
    m_stampTimer.Start(1000);
    ServiceDB m_db;
    m_clientSeedID = m_db.SetClientSeed();
}

void EntityList::Shutdown() {
    /** @todo finish this....
     * halt server called from admin client. (gm command ingame)
     * call d'tor on all connected clients
     * server run loop will exit after control is returned from this function, which will clean up remaining items.
     */
    for (auto cur : m_clients)
        SafeDelete(cur);

    m_clients.clear();
}

void EntityList::Close()
{
    sLog.Log(" EntityList::Close()", "Cleaning up %u clients, %u systems, and %u stations", \
                m_clients.size(), m_systems.size(), m_stations.size());

    for (auto cur : m_clients)
        SafeDelete(cur);

    for (auto cur : m_systems)
        SafeDelete(cur.second);

    m_systems.clear();
    m_clients.clear();
    m_stations.clear();
}

void EntityList::Add( Client* client ) {
    ++m_connections;
    if (client)
        m_clients.push_back(client);
}

void EntityList::Remove(Client* client) {
    /*  this has to use a 'real' iterator for erase() to work. */
	/* note:  will get expensive for many clients  */
    for (std::vector<Client*>::iterator cur; cur != m_clients.end(); ++cur)
        if ((*cur) == client)
            m_clients.erase(cur);
}

void EntityList::Process() {
    double profileStartTime = 0.0;
    if (sConfig.server.UseProfiling)
        profileStartTime = GetTimeUSeconds();

    Client* pClient(nullptr);
    std::vector<Client*>::iterator cur_client = m_clients.begin();
    while (cur_client != m_clients.end()) {
        if ((*cur_client)->ProcessNet())
            ++cur_client;
        else {
            pClient = *cur_client;
            cur_client = m_clients.erase(cur_client);
            if (pClient)
                SafeDelete(pClient);
        }
    }

    if (sConfig.server.UseProfiling) {
        sProfile.AddTime(_entityCProfile, GetTimeUSeconds() - profileStartTime);
        profileStartTime = GetTimeUSeconds();
    }

    /* check for 1Hz timer tic */
    if (m_stampTimer.Check()) {
        ++m_stamp;
        sWHMgr.Process();
        sBubbleMgr.Process();
        sMktBotMgr.Process();

        for (auto cur : m_clients)
            if (cur->GetLocationID())
                cur->ProcessClient();

        std::map<uint32, SystemManager*>::iterator itr = m_systems.begin();
        while (itr != m_systems.end()) {
            if (!itr->second) { /* this shouldnt happen.  log error to make note */
                sLog.Error(" EntityList::Proc", "Deleting System %u", itr->first);
                SafeDelete(itr->second);
                itr = m_systems.erase(itr);
                continue;
            } else if (!itr->second->ProcessTic()) {    /* Process each loaded system */
                itr->second->UnloadSystem();
                //SafeDelete(itr->second);      /* comment this until system unloading is finished...this is working as intended */
                //itr = m_systems.erase(itr);
                continue;
            }
            ++itr;
        }
        if (sConfig.server.UseProfiling)
            sProfile.AddTime(_entitySProfile, GetTimeUSeconds() - profileStartTime);
    }
}

SystemManager* EntityList::FindOrBootSystem(uint32 systemID) {
    if (!IsSolarSystem(systemID)) {
        _log(SERVER__INIT_ERR, "BootSystem() called with invalid systemID (%u)", systemID);
        return nullptr;
    }

    std::map<uint32, SystemManager*>::iterator res = m_systems.find(systemID);
    if (res != m_systems.end())
        return res->second;

    SystemManager* mgr = new SystemManager(systemID, *m_services);
    if ((!mgr) || (!mgr->BootSystem())) {
        _log(SERVER__INIT_ERR, "BootSystem() - Booting system %u failed", systemID);
        SafeDelete(mgr);
        return nullptr;
    }

    _log(SERVER__INIT, "BootSystem() - Booted system %u", systemID);
    m_systems[systemID] = mgr;
    return mgr;
}

/* note...all of the Find* methods below can get very expensive for many players */
Client* EntityList::FindClientByCharID(uint32 char_id) const {
    for (auto cur : m_clients) {
        if (cur->GetCharacterID() == char_id)
            return cur;
    }
    return nullptr;
}

Client* EntityList::FindClientByName(const char* name) const {
    for (auto cur : m_clients) {
        CharacterRef cRef = cur->GetChar();
		if (cRef)
			if (cRef->itemName().c_str() == name)
                return cur;
    }
    return nullptr;
}

Client* EntityList::FindClientByShip(uint32 ship_id) const {
    for (auto cur : m_clients) {
        if (cur->GetShipID() == ship_id)
            return cur;
    }
    return nullptr;
}

Client* EntityList::FindClientByAccount(uint32 account_id) const {
    for (auto cur : m_clients) {
        if (cur->GetUserID() == account_id)
            return cur;
    }
    return nullptr;
}

void EntityList::FindClientByStationID(uint32 stationID, std::vector<Client*> &result) const {
    for (auto cur : m_clients)
        if (cur->GetStationID() == stationID)
            result.push_back(cur);
}

void EntityList::FindByRegionID(uint32 regionID, std::vector<Client*> &result) const {
    for (auto cur : m_clients)
        if (cur->GetRegionID() == regionID)
            result.push_back(cur);
}

void EntityList::Broadcast(const char* notifyType, const char* idType, PyTuple** payload) const {
    //build a little notification out of it.
    EVENotificationStream notify;
        notify.remoteObject = 1;
        notify.args = *payload;
    *payload = nullptr;    //consumed

    //now sent it to the client
    PyAddress dest;
        dest.type = PyAddress::Broadcast;
        dest.service = notifyType;
        dest.bcast_idtype = idType;
    Broadcast(dest, notify);
}

void EntityList::Broadcast(const PyAddress &dest, EVENotificationStream &noti) const {
    for (auto cur : m_clients)
        cur->SendNotification(dest, noti);
}

void EntityList::Multicast(const character_set &cset, const PyAddress &dest, EVENotificationStream &noti) const {
    //this could likely be done better
    std::vector<Client*> result;
    GetClients(cset, result);

    std::vector<Client*>::iterator cur = result.begin();
    for (; cur != result.end(); cur++)
        (*cur)->SendNotification(dest, noti);
}

//in theory this could be written in terms of the more generic
//MulticastTarget function, but this is much more efficient.
void EntityList::Multicast( const char* notifyType, const char* idType, PyTuple** payload, NotificationDestination target, uint32 target_id, bool seq )
{
    PyTuple* p = *payload;
    *payload = nullptr;

    for (auto cur : m_clients) {
        switch( target ) {
        case NOTIF_DEST__LOCATION:
            if( cur->GetLocationID() != target_id )
                continue;
            break;
        case NOTIF_DEST__CORPORATION:
            if( cur->GetCorporationID() != target_id )
                continue;
            break;
        }

        PyTuple* temp = new PyTuple(* p );
        cur->SendNotification( notifyType, idType, &temp, seq );
    }

    PyDecRef( p );
}

void EntityList::Multicast(const char* notifyType, const char* idType, PyTuple** in_payload, const MulticastTarget &mcset, bool seq)
{
    // consume payload
    PyTuple* payload = *in_payload;
    *in_payload = nullptr;

    //cache these locally to avoid calling empty each iteration.
    const bool chars_empty = mcset.characters.empty();
    const bool locs_empty = mcset.locations.empty();
    const bool corps_empty = mcset.corporations.empty();

    if ( !chars_empty || !locs_empty || !corps_empty ) {
        for (auto cur : m_clients) {
            if(      !chars_empty
                 && mcset.characters.find(cur->GetCharacterID()) != mcset.characters.end() )
            {
                //found, carry on...
            } else if(   !locs_empty
                      && mcset.locations.find(cur->GetLocationID()) != mcset.locations.end() )
            {
                //found, carry on...
            } else if(   !corps_empty
                      && mcset.corporations.find(cur->GetCorporationID()) != mcset.corporations.end() )
            {
                //found, carry on...
            } else {
                //not found in any of the above sets.
                continue;
            }

            PyTuple* temp = new PyTuple(*payload);
            cur->SendNotification( notifyType, idType, &temp, seq );
        }
    }

    PyDecRef( payload );
}

void EntityList::Multicast(const character_set &cset, const char* notifyType, const char* idType, PyTuple** in_payload, bool seq) const {
    std::vector<Client*> result;
    GetClients(cset, result);

    size_t num_remaining = result.size();

    std::vector<Client*>::iterator cur = result.begin();
    PyTuple* payload;
    for (; cur != result.end(); cur++, num_remaining--) {
        //keep a counter to eliminate an extra copy of in_payload
        if (num_remaining < 2) {
            payload = *in_payload;
            *in_payload = nullptr;
        } else {
            if (*in_payload == nullptr)
                payload = nullptr;
            else
                payload = (PyTuple*) (*in_payload)->Clone();
        }

        (*cur)->SendNotification(notifyType, idType, &payload, seq);
    }
}

void EntityList::Unicast(uint32 charID, const char* notifyType, const char* idType, PyTuple** payload, bool seq) {
    //this could be implemented more efficiently, but I dont feel like it right now.
    character_set cset;
    cset.insert(charID);
    Multicast(cset, notifyType, idType, payload, seq);
}

void EntityList::GetClients(const character_set &cset, std::vector<Client*> &result) const {
    //this could likely be done better
    character_set::const_iterator res;
    for (auto cur : m_clients) {
        res = cset.find(cur->GetCharacterID());
        if (res != cset.end()) {
            result.push_back(cur);
        }
    }
}

void EntityList::GetClients(std::vector<Client*> &result) const {
    for (auto cur : m_clients)
        result.push_back(cur);
}

void EntityList::AddStation(uint32 stationID, InventoryItemRef itemRef) {
    m_stations[stationID] = itemRef;
}

void EntityList::RemoveStation(uint32 stationID) {
    m_stations.erase(stationID);
}

InventoryItemRef EntityList::GetStationByID(uint32 stationID) {
    std::map<uint32, InventoryItemRef>::iterator res = m_stations.find(stationID);
    if (res != m_stations.end())
        return res->second;
    return InventoryItemRef();
}

void EntityList::RegisterSID(int64 &sessionID) {
    /*  this whole method is just made up...eventually it will return a unique long long */
    /* max for int64 = 9223372036854775807 */
    if (sessionID >= EVEMU_MAX_LONG_ID) {
        sessionID /= EvE_Pi;
        RegisterSID(sessionID);
    }
    std::set<int64>::const_iterator cur = m_sessions.find(sessionID);
    std::pair<std::_Rb_tree_const_iterator<int64>, bool > test;
    if (cur == m_sessions.end())
        test = m_sessions.insert(sessionID);
    if (test.second)
        return;

    sessionID *= 1.25;
    RegisterSID(sessionID);
}

void EntityList::RemoveSID ( int64 sessionID ) {
    m_sessions.erase(sessionID);
}

std::string EntityList::GetAnomalyID()
{
    std::string str1 = "", str2 = "";
    for (uint8 i = 0; i < 3; ++i) {
        str1 += alphaList[MakeRandomInt(0,25)];    //rand() % sizeof(alphaList) - 1
        str2 += itoa(MakeRandomInt(0,8));
    }

    std::string res = str1;
    res += "-";
    res += str2;
    // not sure if we need to keep track of these IDs...
    //m_anomIDs.push_back(res);
    return res;
}

