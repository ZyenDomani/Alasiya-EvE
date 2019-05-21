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
#include "ConsoleCommands.h"
#include "EntityList.h"
#include "EVEServerConfig.h"
#include "ServiceDB.h"
#include "agents/Agent.h"
#include "market/MarketMgr.h"
//#include "market/MarketBotMgr.h"
#include "missions/MissionDataMgr.h"
#include "station/Station.h"
#include "system/DestinyManager.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/AnomalyMgr.h"
#include "system/cosmicMgrs/CivilianMgr.h"
#include "system/cosmicMgrs/WormholeMgr.h"
#include "system/cosmicMgrs/ManagerDB.h"

EntityList::EntityList()
: m_services( nullptr ),
m_stampTimer(0, true),
m_minutetimer(0, true),
m_updateTimer(0),
m_startTime(0)
{
    m_agents.clear();
    m_systems.clear();
    m_clients.clear();
    m_stations.clear();

    m_npcs = 0;
    m_stamp = 1000;   /* start at 1k.  in seconds.  used for destiny and client counters */
    m_minutes = 0;
    m_connections = 0;
    m_clientSeedID = 0;
    m_shipTracking = sConfig.debug.UseShipTracking;
}

EntityList::~EntityList() {
    sLog.Green("   ServerShutdown", " Complete.");
}

void EntityList::Initialize() {
    m_startTime = GetFileTimeNow();

    /* start the timers */
    m_stampTimer.Start(1000);
    m_minutetimer.Start(60000);
    m_updateTimer.Start(sConfig.rates.WebUpdate * 60000);   // change minutes to ms for timer

    m_clientSeedID = ServiceDB::SetClientSeed();

    if (is_log_enabled(SERVER__STACKTRACE))
        sConfig.server.StackTrace = true;

    sLog.Blue("       EntityList", "Entity Manager Initialized.");
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
    if (m_clients.size() > 0)
        sLog.Yellow("       EntityList", "Cleaning up %u clients, %u systems, %u agents, and %u stations", \
                    m_clients.size(), m_systems.size(), m_agents.size(), m_stations.size());
    else
        sLog.Green("       EntityList", "Cleaning up %u clients, %u systems, %u agents, and %u stations", \
                    m_clients.size(), m_systems.size(), m_agents.size(), m_stations.size());

    for (auto cur : m_clients)
        SafeDelete(cur);

    for (auto cur : m_agents)
        SafeDelete(cur.second);

    for (auto cur : m_systems) {
        cur.second->UnloadSystem();
        SafeDelete(cur.second);
    }

    m_systems.clear();
    m_clients.clear();
    m_players.clear();
    m_stations.clear();
}

/* m_clients is used to search for online players and numerous other things.
 *  the problem here is any searching is done thru iteration, which can get expensive.
 *  however, clients are added before their char is selected, so there is no charID for map placement.
 *    maybe use m_clients for basic Process() calls and use m_players for character/client searching
 */
void EntityList::Add( Client* pClient ) {
    ++m_connections;
    if (pClient != nullptr)
        m_clients.push_back(pClient);
}

void EntityList::Remove(Client* pClient) {
	/* note:  will get expensive for many clients  */
    std::vector<Client*>::iterator itr = m_clients.begin();
    for (; itr != m_clients.end(); ++itr)
        if ((*itr) == pClient) {
            m_clients.erase(itr);
            return;
        }
}

void EntityList::AddPlayer(Client* pClient)
{
    if (pClient != nullptr)
        m_players.emplace(pClient->GetCharacterID(), pClient);
}

void EntityList::RemovePlayer(Client* pClient)
{
    if (pClient != nullptr) {
        std::map<uint32, Client*>::iterator itr = m_players.find(pClient->GetCharacterID());
        if (itr != m_players.end())
            m_players.erase(itr);
    }
}


void EntityList::Process() {
    Client* pClient(nullptr);
    std::vector<Client*>::iterator itr = m_clients.begin();
    while (itr != m_clients.end()) {
        if ((*itr)->ProcessNet()) {
            ++itr;
        } else {
            pClient = *itr;
            itr = m_clients.erase(itr);
            SafeDelete(pClient);
        }
    }

    /* check for 1Hz timer tic */
    if (m_stampTimer.Check()) {
        double profileStartTime = GetTimeUSeconds();

        ++m_stamp;

        // these need 1Hz tics
        sCivMgr.Process();
        sBubbleMgr.Process();

        for (auto cur : m_clients)
            if (cur->IsLoaded())
                cur->ProcessClient();

        std::map<uint32, SystemManager*>::iterator itr = m_systems.begin();
        while (itr != m_systems.end()) {
            if (itr->second == nullptr) { /* this shouldnt happen.  log error to make note */
                sLog.Error(" EntityList::Proc", "Deleting System %u", itr->first);
                itr = m_systems.erase(itr);
                continue;
            } else if (!itr->second->ProcessTic()) {    /* Process each loaded system */
                itr->second->UnloadSystem();
                SafeDelete(itr->second);
                itr = m_systems.erase(itr);
                continue;
            }
            ++itr;
        }

        if (m_minutetimer.Check()) {
            // dont have a use for this yet, but have visions where this could be handy (like player online counters)
            ++m_minutes;
            sMissionDataMgr.Process();  // 1m

            // these do not need to be precise
            sWHMgr.Process();   // ~2m
            if (m_minutes % 60 == 0) // ~1h
                sMktMgr.Process();
            //sMktBotMgr.Process();  // 15m to 30m
            if (m_updateTimer.Check())  // 15m
                sConsole.UpdateStatus();

            //if (m_minutes % 10 == 0) // ~10m
            //    sDatabase.ping();
        }

        if (sConfig.debug.UseProfiling)
            sProfile.AddTime(_entitySProfile, GetTimeUSeconds() - profileStartTime);
    }
}

SystemManager* EntityList::FindOrBootSystem(uint32 systemID) {
    if (!IsSolarSystem(systemID)) {
        _log(SERVER__INIT_ERR, "BootSystem() called with invalid systemID (%u)", systemID);
        return nullptr;
    }

    std::map<uint32, SystemManager*>::iterator itr = m_systems.find(systemID);
    if (itr != m_systems.end())
        return itr->second;

    /** @todo test for adding OpenMP here to enable MP per system. */
    SystemManager* pSM = new SystemManager(systemID, *m_services);
    if ((pSM == nullptr) or (!pSM->BootSystem())) {
        _log(SERVER__INIT_ERR, "BootSystem() - Booting system %u failed", systemID);
        SafeDelete(pSM);
        return nullptr;
    }

    _log(SERVER__INIT, "BootSystem() - Booted system %u", systemID);
    m_systems[systemID] = pSM;
    return pSM;
}

Agent* EntityList::GetAgent(uint32 agentID) {
    std::map<uint32, Agent*>::iterator res = m_agents.find(agentID);
    if (res != m_agents.end())
        return res->second;

    Agent* aPtr = new Agent(agentID);
    if (!aPtr->Load()) {
        delete aPtr;
        return nullptr;
    }
    m_agents[agentID] = aPtr;
    return aPtr;
}

// this method is corrected, as stations have their own guestlist now.
void EntityList::GetStationGuestList(uint32 stationID, std::vector<Client*> &result) const {
    std::map<uint32, StationItemRef>::const_iterator itr = m_stations.find(stationID);
    if (itr != m_stations.end())
        itr->second->GetGuestList(result);
}

bool EntityList::IsOnline(uint32 charID)
{
    std::map<uint32, Client*>::const_iterator itr = m_players.find(charID);
    if (itr == m_players.end())
        return false;
    return true;
}

PyRep* EntityList::PyIsOnline(uint32 charID)
{
    std::map<uint32, Client*>::const_iterator itr = m_players.find(charID);
    if (itr == m_players.end())
        return PyStatic.NewFalse();

    return PyStatic.NewTrue();
}

Client* EntityList::FindClientByCharID(uint32 charID) const
{
    std::map<uint32, Client*>::const_iterator itr = m_players.find(charID);
    if (itr == m_players.end())
        return nullptr;

    return itr->second;
}

/** @todo @note NOTE: TODO: HACK: the Find* methods below can get very expensive for many players */
// used in mkt shit....is there a better way for this one?
//  yes!!  this should NOT be used.  fix mkt and delete this
void EntityList::FindByRegionID(uint32 regionID, std::vector<Client*> &result) const {
    for (auto cur : m_clients)
        if (cur->GetRegionID() == regionID)
            result.push_back(cur);
}

//used by gmCommands
Client* EntityList::FindClientByName(const char* name) const {
    for (auto cur : m_players) {
        CharacterRef cRef = cur.second->GetChar();
        if (cRef.get() != nullptr)
            if (strcmp(cRef->itemName().c_str(), name) == 0)
                return cur.second;
    }
    return nullptr;
}

void EntityList::Broadcast(const char* notifyType, const char* idType, PyTuple** payload) const {
    //build a little notification out of it.
    EVENotificationStream notify;
        notify.remoteObject = 1;
        notify.args = *payload;
    payload = nullptr;    //consumed

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
    for (; cur != result.end(); ++cur)
        (*cur)->SendNotification(dest, noti);
}

// updated to remove looping thru entire client list for each call....still needs work
void EntityList::Multicast( const char* notifyType, const char* idType, PyTuple** in_payload, NotificationDestination target, uint32 targID, bool seq )
{
    PyTuple* payload = *in_payload;
    in_payload = nullptr;

    std::vector<Client*> cVec;
    cVec.clear();
    switch( target ) {
        case NOTIF_DEST__LOCATION: {
            if (IsStation(targID))
                GetStationGuestList(targID, cVec);
            else if (IsSolarSystem(targID))
                FindOrBootSystem(targID)->GetClientList(cVec);
            else {
                sLog.Error("EntityList::Multicast 1", "DEST__LOCATION - location %u is neither station nor system", targID);
                EvE::traceStack();
            }
        } break;
        // not sure how to do this one yet.
        case NOTIF_DEST__CORPORATION: {
            for (auto cur : m_clients)
                if (cur->GetCorporationID() == targID)
                    cVec.push_back(cur);
        } break;
    };

    for (auto cur : cVec) {
        PyIncRef(payload);
        cur->SendNotification( notifyType, idType, &payload, seq );
    }

    PyDecRef( payload );
}

/** @todo this shit is nuts.  will have to revisit and come up with a better way to do this. */
void EntityList::Multicast(const char* notifyType, const char* idType, PyTuple** in_payload, const MulticastTarget &mcset, bool seq)
{
    // consume payload
    PyTuple* payload = *in_payload;
    in_payload = nullptr;
/*
    for (auto cur : mcset.characters) {
        Client* pClient(nullptr);
        if (IsCharacter(cur)) {
            PyIncRef(payload);
            // this will be slow as fuck
            pClient = FindClientByCharID(charID);
            if (pClient != nullptr)
                pClient->SendNotification( notifyType, idType, &payload, seq );
        }
    }
*/
    if (!mcset.characters.empty())
        for (auto cur : m_clients)
            if ( mcset.characters.find(cur->GetCharacterID()) != mcset.characters.end()) {
                PyIncRef(payload);
                cur->SendNotification( notifyType, idType, &payload, seq );
            }

    if (!mcset.locations.empty()) {
        std::vector<Client*> cVec;
        cVec.clear();
        for (auto cur : mcset.locations) {
            if (IsStation(cur))
                GetStationGuestList(cur, cVec);
            else if (IsSolarSystem(cur))
                FindOrBootSystem(cur)->GetClientList(cVec);
            else {
                sLog.Error("EntityList::Multicast 2", "location %u is neither station nor system", cur);
                EvE::traceStack();
            }
        }
        for (auto cur : cVec) {
            PyIncRef(payload);
            cur->SendNotification( notifyType, idType, &payload, seq );
        }
    }

    // this will need list of interested parties from corp.  not sure how to do it yet.
    // this one is crazy...loop thru all clients for each corp in set.  avoid if we can.
    if (!mcset.corporations.empty())
        for (auto cur : m_clients)
            if (mcset.corporations.find(cur->GetCorporationID()) != mcset.corporations.end()) {
    //    for (auto cur : mcset.corporations) {
                PyIncRef(payload);
                cur->SendNotification( notifyType, idType, &payload, seq );
            }

    PyDecRef( payload );
}

void EntityList::Multicast(const character_set &cset, const char* notifyType, const char* idType, PyTuple** in_payload, bool seq) const
{
    // consume payload
    PyTuple* payload = *in_payload;
    in_payload = nullptr;

    std::vector<Client*> cVec;
    GetClients(cset, cVec);
    for (auto cur : cVec) {
        PyIncRef(payload);
        cur->SendNotification(notifyType, idType, &payload, seq);
    }
    PyDecRef( payload );
}

void EntityList::Unicast(uint32 charID, const char* notifyType, const char* idType, PyTuple** payload, bool seq) {
    Client* pClient = FindClientByCharID(charID);
    if (pClient != nullptr)
        pClient->SendNotification( notifyType, idType, payload, seq );
}

void EntityList::GetClients(const character_set &cset, std::vector<Client*> &result) const {
    //this could likely be done better
    character_set::iterator res;
    for (auto cur : m_clients) {
        res = cset.find(cur->GetCharacterID());
        if (res != cset.end())
            result.push_back(cur);
    }
}

void EntityList::GetClients(std::vector<Client*> &result) const {
    for (auto cur : m_clients)
        result.push_back(cur);
}

void EntityList::AddStation(uint32 stationID, StationItemRef itemRef) {
    m_stations[stationID] = itemRef;
}

void EntityList::RemoveStation(uint32 stationID) {
    m_stations.erase(stationID);
}

StationItemRef EntityList::GetStationByID(uint32 stationID) {
    std::map<uint32, StationItemRef>::iterator res = m_stations.find(stationID);
    if (res != m_stations.end())
        return res->second;
    return StationItemRef(nullptr);
}

void EntityList::RegisterSID(int64 &sessionID) {
    /*  this whole method is just made up...eventually it will return a unique long long */
    /* max for int64 = 9223372036854775807 */
    if (sessionID >= EVEMU_MAX_LONG_ID) {
        sessionID /= EvE::Trig::Pi;
        RegisterSID(sessionID);
    }
    std::set<int64>::iterator cur = m_sessions.find(sessionID);
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
    // these should be totally unique.  design a way to enforce this
    std::string str1 = "", str2 = "";
    for (uint8 i = 0; i < 3; ++i) {
        str1 += alphaList[MakeRandomInt(0,25)];    //rand() % sizeof(alphaList) - 1
        str2 += itoa(MakeRandomInt(0,9));
    }

    std::string res = str1;
    res += "-";
    res += str2;
    // not sure if we need to keep track of these IDs...
    //m_anomIDs.push_back(res);
    return res;
}

