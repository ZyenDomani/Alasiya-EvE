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
#include "station/Station.h"
#include "system/DestinyManager.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/AnomalyMgr.h"
#include "system/cosmicMgrs/CivilianMgr.h"
#include "system/cosmicMgrs/WormholeMgr.h"
#include "system/cosmicMgrs/ManagerDB.h"

EntityList::EntityList()
: m_services( nullptr ),
m_stamp(1000),    /* start at 1k.  in seconds.  used for destiny and client counters */
m_stampTimer(0, true)
{
    m_systems.clear();
    m_clients.clear();
    m_stations.clear();

    m_connections = 0;
    m_shipTracking = sConfig.debug.UseShipTracking;
}

EntityList::~EntityList() {
    sLog.Green("   ServerShutdown", " Complete.");
}

void EntityList::Initialize() {
    /* start the timer */
    m_stampTimer.Start(1000);    // fudge the 1000ms a bit in hopes for more-accurate timing
    m_clientSeedID = ServiceDB::SetClientSeed();
    if (is_log_enabled(SERVER__STACKTRACE))
        sConfig.server.StackTrace = true;
    sLog.Blue("       EntityList", "EntityList Initialized.");
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
        sLog.Yellow("       EntityList", "Cleaning up %u clients, %u systems, and %u stations", \
                m_clients.size(), m_systems.size(), m_stations.size());
    else
        sLog.Green("       EntityList", "Cleaning up %u clients, %u systems, and %u stations", \
                m_clients.size(), m_systems.size(), m_stations.size());

    for (auto cur : m_clients)
        SafeDelete(cur);

    for (auto cur : m_systems) {
        cur.second->UnloadSystem();
        SafeDelete(cur.second);
    }

    m_systems.clear();
    m_clients.clear();
    m_stations.clear();
}

void EntityList::Add( Client* client ) {
    ++m_connections;
    if (client != nullptr)
        m_clients.push_back(client);
}

void EntityList::Remove(Client* client) {
	/* note:  will get expensive for many clients  */
    std::vector<Client*>::iterator itr = m_clients.begin();
    for (; itr != m_clients.end(); ++itr)
        if ((*itr) == client) {
            m_clients.erase(itr);
            return;
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
        //sLog.White("time check", "(%u) - ms: %f, win32: %" PRIi64 ", file: %.4f", m_stamp, GetTimeMSeconds(), Win32TimeNow(), GetFileTimeNow());
        sWHMgr.Process();
        sCivMgr.Process();
        sBubbleMgr.Process();
        //sMktBotMgr.Process();  // not used yet

        for (auto cur : m_clients)
            if (cur->GetLocationID())   /* hack to verify valid client */
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

// this method is corrected, as stations have their own guestlist now.
void EntityList::FindClientByStationID(uint32 stationID, std::vector<Client*> &result) const {
    std::map<uint32, StationItemRef>::const_iterator itr = m_stations.find(stationID);
    if (itr != m_stations.end())
        itr->second->GetGuestList(result);
}

/** @todo @note NOTE: TODO: HACK:...all of the Find* methods below can get very expensive for many players */
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
				if (cRef.get() != nullptr) {
						if (strcmp(cRef->itemName().c_str(), name) == 0) {
                return cur;
						}
				}
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

//in theory this could be written in terms of the more generic
//MulticastTarget function, but this is much more efficient.
/** @todo  this shit is nuts.  update to use system/station maps for existing clients, instead of looping thru entire fuckin list */
void EntityList::Multicast( const char* notifyType, const char* idType, PyTuple** in_payload, NotificationDestination target, uint32 target_id, bool seq )
{
    PyTuple* payload = *in_payload;
    in_payload = nullptr;

    for (auto cur : m_clients) {
        switch( target ) {
            case NOTIF_DEST__LOCATION: {
                if ( cur->GetLocationID() != target_id )
                    continue;
            } break;
            case NOTIF_DEST__CORPORATION: {
                if ( cur->GetCorporationID() != target_id )
                    continue;
            } break;
        }

        PyIncRef(payload);
        cur->SendNotification( notifyType, idType, &payload, seq );
    }

    PyDecRef( payload );
}

void EntityList::Multicast(const char* notifyType, const char* idType, PyTuple** in_payload, const MulticastTarget &mcset, bool seq)
{
    // consume payload
    PyTuple* payload = *in_payload;
    in_payload = nullptr;

    if (!mcset.characters.empty())
        for (auto cur : m_clients)
            if ( mcset.characters.find(cur->GetCharacterID()) != mcset.characters.end()) {
                PyIncRef(payload);
                cur->SendNotification( notifyType, idType, &payload, seq );
            }

    if (!mcset.locations.empty())
        for (auto cur : m_clients)
            if (mcset.locations.find(cur->GetLocationID()) != mcset.locations.end()) {
                PyIncRef(payload);
                cur->SendNotification( notifyType, idType, &payload, seq );
            }

    if (!mcset.corporations.empty())
        for (auto cur : m_clients)
            if (mcset.corporations.find(cur->GetCorporationID()) != mcset.corporations.end()) {
                PyIncRef(payload);
                cur->SendNotification( notifyType, idType, &payload, seq );
            }

    PyDecRef( payload );
}

void EntityList::Multicast(const character_set &cset, const char* notifyType, const char* idType, PyTuple** in_payload, bool seq) const {
    std::vector<Client*> result;
    GetClients(cset, result);

    // consume payload
    PyTuple* payload = *in_payload;
    in_payload = nullptr;

    std::vector<Client*>::iterator cur = result.begin();
    for (; cur != result.end(); ++cur) {
        PyIncRef(payload);
        (*cur)->SendNotification(notifyType, idType, &payload, seq);
    }
    PyDecRef( payload );
}

void EntityList::Unicast(uint32 charID, const char* notifyType, const char* idType, PyTuple** payload, bool seq) {
    //this could be implemented more efficiently, but I dont feel like it right now.
    character_set cset;
    cset.insert(charID);
    Multicast(cset, notifyType, idType, payload, seq);
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
        return StationItemRef::StaticCast(res->second);
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


uint32 EntityList::GetWreckFaction(uint32 typeID)
{
    // these will need to be separated and updated after detailed salvage table is completed

    switch(typeID) {
        case 26469:  //   Amarr Battlecruiser Wreck
        case 26470:  //   Amarr Battleship Wreck
        case 26472:  //   Amarr Carrier Wreck
        case 26473:  //   Amarr Cruiser Wreck
        case 26474:  //   Amarr Destroyer Wreck
        case 26475:  //   Amarr Dreadnought Wreck
        case 26476:  //  Amarr Elite Battlecruiser Wreck
        case 26477:  //   Amarr Elite Battleship Wreck
        case 26478:  //   Amarr Elite Cruiser Wreck
        case 26479:   //  Amarr Elite Destroyer Wreck
        case 26480:   //  Amarr Elite Frigate Wreck
        case 26481:   //  Amarr Elite Industrial Wreck
        case 26482:   //  Amarr Elite Mining Barge Wreck
        case 26483:   //  Amarr Freighter Wreck
        case 26484:   //  Amarr Frigate Wreck
        case 26485:  //  Amarr Industrial Wreck
        case 26486:   //  Amarr Mining Barge Wreck
        case 26487:   //  Amarr Supercarrier Wreck
        case 26488:   //  Amarr Rookie ship Wreck
        case 26489:   //  Amarr Shuttle Wreck
        case 26490:   //  Amarr Titan Wreck
        case 27050:   //  Amarr Large Wreck
        case 27051:   // Amarr Medium Wreck
        case 27052:   //  Amarr Small Wreck
        case 29033:   // Amarr Elite Freighter Wreck
        case 27927:  //  Mission Amarr Carrier Wreck
        case 30822: { //   Amarr Advanced Cruiser Wreck
            return factionAmarr;
        } break;

        case 26491:  //    Caldari Battlecruiser Wreck
        case 26492:  //    Caldari Battleship Wreck
        case 26494:  //    Caldari Carrier Wreck
        case 26495:  //   Caldari Cruiser Wreck
        case 26496:  //    Caldari Destroyer Wreck
        case 26497:  //    Caldari Dreadnought Wreck
        case 26498:  //    Caldari Elite Battlecruiser Wreck
        case 26499:  //   Caldari Elite Battleship Wreck
        case 26500:  //   Caldari Elite Cruiser Wreck
        case 26501:  //   Caldari Elite Destroyer Wreck
        case 26502:  //   Caldari Elite Frigate Wreck
        case 26503:  //   Caldari Elite Industrial Wreck
        case 26504:  //   Caldari Elite Mining Barge Wreck
        case 26505:  //    Caldari Freighter Wreck
        case 26506:  //    Caldari Frigate Wreck
        case 26507:  //    Caldari Industrial Wreck
        case 26508:  //    Caldari Mining Barge Wreck
        case 26509:  //    Caldari Supercarrier Wreck
        case 26510:  //   Caldari Rookie ship Wreck
        case 26511:  //    Caldari Shuttle Wreck
        case 26512:  //    Caldari Titan Wreck
        case 27926:  //    Mission Caldari Carrier Wreck
        case 30823:  //    Caldari Advanced Cruiser Wreck
        case 29034:  //   Caldari Elite Freighter Wreck
        case 27047:  //    Caldari Large Wreck
        case 27048:  //    Caldari Medium Wreck
        case 27049: {  //    Caldari Small Wreck
            return factionCaldari;
        } break;

        case 29035:  //    Gallente Elite Freighter Wreck
        case 30824:  //    Gallente Advanced Cruiser Wreck
        case 27929:  //    Mission Gallente Carrier Wreck
        case 27053:  //    Gallente Large Wreck
        case 27054:  //    Gallente Medium Wreck
        case 27055:  //    Gallente Small Wreck
        case 26513:  //    Gallente Battlecruiser Wreck
        case 26514:  //    Gallente Battleship Wreck
        case 26516:  //    Gallente Carrier Wreck
        case 26517:  //    Gallente Cruiser Wreck
        case 26518:  //    Gallente Destroyer Wreck
        case 26519:  //    Gallente Dreadnought Wreck
        case 26520:  //    Gallente Elite Battlecruiser Wreck
        case 26521:  //    Gallente Elite Battleship Wreck
        case 26522:  //    Gallente Elite Cruiser Wreck
        case 26523:  //    Gallente Elite Destroyer Wreck
        case 26524:  //    Gallente Elite Frigate Wreck
        case 26525:  //    Gallente Elite Industrial Wreck
        case 26526:  //    Gallente Elite Mining Barge Wreck
        case 26527:  //    Gallente Freighter Wreck
        case 26528:  //    Gallente Frigate Wreck
        case 26529:  //    Gallente Industrial Wreck
        case 26530:  //   Gallente Mining Barge Wreck
        case 26531:  //    Gallente Supercarrier Wreck
        case 26532:  //    Gallente Rookie ship Wreck
        case 26533:  //    Gallente Shuttle Wreck
        case 26534: { //   Gallente Titan Wreck
            return factionGallente;
        } break;

        case 26535:  //    Minmatar Battlecruiser Wreck
        case 26536:  //    Minmatar Battleship Wreck
        case 26538:  //    Minmatar Carrier Wreck
        case 26539:  //    Minmatar Cruiser Wreck
        case 26540:  //    Minmatar Destroyer Wreck
        case 26541:  //    Minmatar Dreadnought Wreck
        case 26542:  //    Minmatar Elite Battlecruiser Wreck
        case 26543:  //    Minmatar Elite Battleship Wreck
        case 26544:  //    Minmatar Elite Cruiser Wreck
        case 26545:  //    Minmatar Elite Destroyer Wreck
        case 26546:  //    Minmatar Elite Frigate Wreck
        case 26547:  //    Minmatar Elite Industrial Wreck
        case 26548:  //    Minmatar Elite Mining Barge Wreck
        case 26549:  //    Minmatar Freighter Wreck
        case 26550:  //    Minmatar Frigate Wreck
        case 26551:  //    Minmatar Industrial Wreck
        case 26552:  //    Minmatar Mining Barge Wreck
        case 26553:  //    Minmatar Supercarrier Wreck
        case 26554:  //    Minmatar Rookie ship Wreck
        case 26555:  //    Minmatar Shuttle Wreck
        case 26556:  //    Minmatar Titan Wreck
        case 27928:  //    Mission Minmatar Carrier Wreck
        case 30825:  //    Minmatar Advanced Cruiser Wreck
        case 27041:  //    Minmatar Large Wreck
        case 27042:  //    Minmatar Medium Wreck
        case 27043: { //   Minmatar Small Wreck
            return factionMinmatar;
        } break;

        case 26972:  //    Faction Drone Wreck   - faction police drones
        case 26939:  //   CONCORD Large Wreck
        case 26940:  //    CONCORD Medium Wreck
        case 26941: { //    CONCORD Small Wreck
            return factionCONCORD;
        } break;

        case 27044:  //    Khanid Large Wreck
        case 27045:  //    Khanid Medium Wreck
        case 27046: { //   Khanid Small Wreck
            return factionKhanid;
        } break;

        case 27056:  //    Thukker Large Wreck
        case 27057:  //    Thukker Medium Wreck
        case 27058: { //    Thukker Small Wreck
            return factionThukker;
        } break;

        case 27060:  //   Mordu Large Wreck
        case 27061:  //   Mordu Medium Wreck
        case 27062: { //    Mordu Small Wreck
            return factionMordusLegion;
        } break;

        case 28603:  //   Rorqual Wreck
        case 29639: { //    Orca Wreck
            return factionORE;
        } break;

        case 30457:  //    Sleeper Small Advanced Wreck
        case 30458:  //    Sleeper Medium Advanced Wreck
        case 30459:  //    Sleeper Large Wreck
        case 30484:  //    Sleeper Small Basic Wreck
        case 30485:  //    Sleeper Small Intermediate Wreck
        case 30492:  //   Sleeper Medium Basic Wreck
        case 30493:  //    Sleeper Medium Intermediate Wreck
        case 30494:  //   Sleeper Large Basic Wreck
        case 30495:  //   Sleeper Large Intermediate Wreck
        case 30496: { //    Sleeper Large Advanced Wreck
            return factionSleepers;
        } break;

        case 26561:  //   Angel Small Wreck
        case 26562:  //   Angel Medium Wreck
        case 26563:  //   Angel Large Wreck
        case 26564:  //   Angel Small Commander Wreck
        case 26699:  //   Angel Medium Commander Wreck
        case 26565:  //   Angel Large Commander Wreck
        case 26566: { //   Angel Officer Wreck
            return factionAngel;
        } break;

        case 26567:  //   Blood Small Wreck
        case 26568:  //   Blood Medium Wreck
        case 26569:  //   Blood Large Wreck
        case 26570:  //   Blood Small Commander Wreck
        case 26571:  //   Blood Medium Commander Wreck
        case 26700:  //   Blood Large Commander Wreck
        case 26572: { //   Blood Officer Wreck
            return factionBloodRaider;
        } break;

        case 26573:  //   Guristas Small Wreck
        case 26574:  //   Guristas Medium Wreck
        case 26575:  //   Guristas Large Wreck
        case 26576:  //   Guristas Small Commander Wreck
        case 26577:  //   Guristas Medium Commander Wreck
        case 26701:  //   Guristas Large Commander Wreck
        case 26578: { //   Guristas Officer Wreck
            return factionGuristas;
        } break;

        case 26579:  //   Sanshas Small Wreck
        case 26580:  //   Sanshas Medium Wreck
        case 26581:  //   Sanshas Large Wreck
        case 26582:  //   Sanshas Small Commander Wreck
        case 26583:  //   Sanshas Medium Commander Wreck
        case 26702:  //   Sanshas Large Commander Wreck
        case 26584:  //   Sanshas Officer Wreck
        case 3260: { //   Sanshas Supercarrier Wreck
            return factionSanshas;
        } break;

        case 26585:  //   Serpentis Small Wreck
        case 26586:  //   Serpentis Medium Wreck
        case 26587:  //   Serpentis Large Wreck
        case 26588:  //   Serpentis Small Commander Wreck
        case 26589:  //   Serpentis Medium Commander Wreck
        case 26703:  //   Serpentis Large Commander Wreck
        case 26590: { //   Serpentis Officer Wreck
            return factionSerpentis;
        } break;

        case 26591:  //   Rogue Small Wreck
        case 26592:  //   Rogue Medium Wreck
        case 26593:  //   Rogue Large Wreck
        case 26594:  //   Rogue Elite Small Wreck
        case 26595:  //   Rogue Elite Medium Wreck
        case 26596:  //   Rogue Officer Wreck
        case 28221:  //   Rogue Large Commander Wreck
        case 28222:  //   Rogue Medium Commander Wreck
        case 28223: { //   Rogue Small Commander Wreck
            return factionRogueDrones;
        } break;

        // generic wrecks
        case 26468:  //   Capsule Wreck
        case 26557:  //   Frigate Wreck
        case 26558:  //   Cruiser Wreck
        case 26559:  //   Battleship Wreck
        case 26918:  //   Overseer Frigate Wreck
        case 26919:  //   Overseer Cruiser Wreck
        case 26920:  //   Overseer Battleship Wreck
        case 27202:  //   Convoy Wreck
        case 27286:  //   Pirate Drone Wreck
        case 26560: { //   Pirate Wreck
            return factionUnknown;
        } break;

    }

    /*
     *    28255 :  //   Mission Faction Freighter Wreck
     *    29036 :  //   Minmatar Elite Freighter Wreck
     *    29347:  //    Mission Faction Vessels Wreck
     *    29365:  //    Mission Faction Industrials Wreck
     *
     */
}

