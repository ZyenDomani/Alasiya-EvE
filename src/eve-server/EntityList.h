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
    Updates:    Allan (rewrite)
*/

#ifndef EVE_ENTITY_LIST_H
#define EVE_ENTITY_LIST_H

#include <vector>
#include <map>
#include <set>
#include "eve-compat.h"
#include "eve-common.h"
#include "utils/Singleton.h"
#include "threading/Mutex.h"

class Agent;
class Client;
class PyAddress;
class EVENotificationStream;
class SystemManager;
class PyTuple;
class PyServiceMgr;

typedef enum {
    NOTIF_DEST__LOCATION,
    NOTIF_DEST__CORPORATION
} NotificationDestination;

//this object is a set of "or"s, matching any criteria is sufficient.
/** @todo  can loc/corp be changed to single variable?  */
class MulticastTarget {
public:
    std::set<uint32> characters;
    std::set<uint32> locations;
    std::set<uint32> corporations;
};

class EntityList
: public Singleton<EntityList>
{
public:
    EntityList();
    ~EntityList();

    typedef std::set<uint32> character_set;

    void Initialize();
    void Close();
    void Process();
    void Shutdown();
    void Add(Client* pClient);
    void Remove(Client* pClient);
    // this must be called AFTER a character is selected, after Client and Character class construction is complete. (need complete data)
    void AddPlayer(Client* pClient);
    //  this must only be called for a logged-in character.
    void RemovePlayer(Client* pClient);
    void AddNPC()                                       { ++m_npcs; }
    void RemoveNPC()                                    { --m_npcs; }
    void SetService(PyServiceMgr* svc)                  { m_services = svc; }
    void FindByRegionID(uint32 regionID, std::vector<Client* > &result) const;
    // updated to use station guest list instead of full clientlist loop
    void GetStationGuestList(uint32 stationID, std::vector<Client* > &result) const;

    // for main loop thread sleeping
    bool HasClients()                                   { return (m_players.size() == 0 ? false : true); }

    Agent* GetAgent(uint32 agentID);

    Client* FindClientByName(const char* name) const;
    Client* FindClientByCharID(uint32 charID) const;

    // this will return nullptr and throw console msg on failure.
    SystemManager* FindOrBootSystem(uint32 systemID);

    bool IsOnline(uint32 charID);
    PyRep* PyIsOnline(uint32 charID);

    uint32 GetNPCCount()                                { return m_npcs; }
    uint32 GetClientCount() const                       { return m_clients.size(); }
    uint32 GetSystemCount() const                       { return m_systems.size(); }
    uint32 GetStationCount() const                      { return m_stations.size(); }
    uint32 GetClientSeed()                              { return ++m_clientSeedID; }

    /* stamp shit here */
    uint32 GetStamp()                                   { return m_stamp; }
    bool IsTicActive()                                  { return m_stampTimer.Check(false); }
    void TicCompleted()                                 { ++m_stamp; }
    uint32 GetMinutes()                                 { return m_minutes; }

    void Broadcast(const char* notifyType, const char* idType, PyTuple** payload) const;
    void Broadcast(const PyAddress &dest, EVENotificationStream &noti) const;
    void Multicast(const char* notifyType, const char* idType, PyTuple** in_payload, NotificationDestination target, uint32 target_id, bool seq = true);
    void Multicast(const char* notifyType, const char* idType, PyTuple** payload, const MulticastTarget &mcset, bool seq=true);
    void Multicast(const character_set &cset, const PyAddress &dest, EVENotificationStream &noti) const;
    void Multicast(const character_set &cset, const char* notifyType, const char* idType, PyTuple** payload, bool seq=true) const;
    void Unicast(uint32 charID, const char* notifyType, const char* idType, PyTuple** payload, bool seq=true);
    // gets Client* for all ingame players
    void GetClients(std::vector<Client* > &result) const;

    uint32 GetConnections()                             { return m_connections; }

    void AddStation(uint32 stationID, StationItemRef itemRef);
    void RemoveStation(uint32 stationID);
    StationItemRef GetStationByID(uint32 stationID);

    void RegisterSID(int64& sessionID);
    void RemoveSID(int64 sessionID);

    std::string GetAnomalyID();

    bool GetTracking()                                  { return m_shipTracking; }
    void SetTracking(bool set=false)                    { m_shipTracking = set; }

    void ResetStartTime()                               { m_startTime = GetFileTimeNow(); }
    int64 GetStartTime()                                { return m_startTime; }

    // new shit for replacing current crazy notification sending
    // this method will send notification to online members that have the role required for the notification sent.
    void CorpNotify(uint32 corpID, uint8 type, const char* notifyType, const char* idType, PyTuple* payload) const;


protected:
    PyServiceMgr* m_services;    //we do not own this, only used for booting systems.

    Mutex mMutex;

private:
    Timer m_stampTimer;
    Timer m_minutetimer;

    // connected clients (incomplete client class data)
    //  use this to delete Client*
    std::vector<Client*> m_clients;
    // logged-in players (complete client class data)
    // DO NOT delete this Client*  use m_clients instead.
    std::map<uint32, Client*> m_players;
    std::set<int64> m_sessions;
    std::map<uint32, SystemManager*> m_systems;
    std::map<uint32, StationItemRef> m_stations;
    std::vector<std::string> m_anomIDs;
    std::map<uint32, Agent*> m_agents;

    // make list for corp members and their roles for easy access of notifications etc.
    typedef std::map<Client*, int64> corpRole;
    std::map<uint32, corpRole> m_corpMembers;     //corpID/{Client*/corpRole}

    bool m_shipTracking;

    uint32 m_npcs;
    uint32 m_stamp;
    uint32 m_minutes;
    uint32 m_connections;
    uint32 m_clientSeedID;

    int64 m_startTime;
};

//Singleton
#define sEntityList \
    ( EntityList::get() )


#endif

