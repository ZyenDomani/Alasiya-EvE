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

#ifndef __SYSTEMMANAGER_H_INCL__
#define __SYSTEMMANAGER_H_INCL__

#include "system/BubbleManager.h"
#include "system/SolarSystem.h"
#include "system/SystemDB.h"


class PyRep;
class PyDict;
class PyTuple;
class PyList;
class Client;
class NPC;
class InventoryItem;
class SystemEntity;
class SystemBubble;
class  SetState;
class DestinyManager;

class AnomalyMgr;
class BeltMgr;
class DungeonMgr;
class SpawnMgr;
class PyServiceMgr;

class DynamicEntityFactory {
public:
    // you MUST call (your SystemManager)->AddEntity() after this to actually put the entity in space
    static SystemEntity* BuildEntity(SystemManager &system, const DBSystemDynamicEntity &entity);
};

class SystemManager
{
public:
    SystemManager(uint32 systemID, PyServiceMgr &svc);//, ItemData idata);
    virtual ~SystemManager();

    SystemEntity* GetSE(uint32 entityID) const;

    PyServiceMgr* GetServiceMgr()                       { return &m_services; }
    Inventory* GetSystemInv()                           { return m_solarSystemRef->GetMyInventory(); }

    ShipItemRef GetShipFromInventory(uint32 shipID);
    StationItemRef GetStationFromInventory(uint32 stationID);
    CargoContainerRef GetContainerFromInventory(uint32 contID);

    bool ProcessTic();          // called at 1Hz.
    bool BootSystem();
    void UnloadSystem();

    bool IsLoaded()                                     { return m_loaded; }

    uint32 GetClosestPlanetID(const GPoint& myPos);
    SystemEntity* GetClosestMoonSE(const GPoint& myPos);

    uint32 GetID() const                                { return m_data.systemID; }
    uint32 GetRegionID()                                { return m_data.regionID; }
    const std::string& GetName() const                  { return m_data.name; }
    const char* GetSystemSecurityClass()                { return m_data.securityClass.c_str(); }
    const double GetSystemSecurityRating()              { return m_data.securityRating; }

    bool BuildDynamicEntity(const DBSystemDynamicEntity& entity);

    void AddClient(Client* who, bool docked=false, bool count=false);
    void RemoveClient(Client* who, bool docked=false, bool count=false);
    void AddNPC(NPC* who);
    void RemoveNPC(NPC* who);
    void AddEntity(SystemEntity* who);
    void RemoveEntity(SystemEntity* who);

    void AddItemToInventory(InventoryItemRef item);
    void RemoveItemFromInventory(InventoryItemRef item);
    void DoSpawnForBubble(SystemBubble* pSysBubble);

    void MakeSetState(const SystemBubble* bubble,  SetState& into) const;

    // for spawn system     -allan 15July15     (not complete)
    typedef std::vector<uint32> SpawnBubbleVec;
    void RemoveSpawnBubble();
    void GetSpawnBubbles(SpawnBubbleVec* bubbleMap);
    void IncRatSpawnCount()                             { ++m_activeRatSpawns; }
    void DecRatSpawnCount()                             { --m_activeRatSpawns; }
    void IncRoidSpawnCount()                            { ++m_activeRoidSpawns; }
    void DecRoidSpawnCount()                            { --m_activeRoidSpawns; }
    uint8 BeltCount()                                   { return m_beltCount; }
    uint8 GetRatSpawnCount()                            { return m_activeRatSpawns; }
    uint16 GetRoidSpawnCount()                          { return m_activeRoidSpawns; }
    uint32 PlayerCount()                                { return m_players; }
    uint32 GetRandBeltID();

    // CosmicMgr interface
    BeltMgr* GetBeltMgr()                               { return m_beltMgr; }
    SpawnMgr* GetSpawnMgr()                             { return m_spawnMgr; }
    AnomalyMgr* GetAnomMgr()                            { return m_anomMgr; }
    DungeonMgr* GetDungMgr()                            { return m_dunMgr; }

    float GetSecValue()                                 { return m_secValue; }

protected:
    AnomalyMgr* m_anomMgr;      //we own this, never NULL.
    BeltMgr* m_beltMgr;         //we own this, never NULL.
    DungeonMgr* m_dunMgr;       //we own this, never NULL.
    SpawnMgr* m_spawnMgr;       //we own this, never NULL.

    SystemData m_data;
    PyServiceMgr& m_services;
    SolarSystemRef m_solarSystemRef;

    void PayBounties();

    bool LoadCosmicMgrs();
    bool LoadSystemStatics();
    bool LoadSystemDynamics();
    bool LoadPlayerDynamics();

    // system entity lists:
    bool m_entityChanged;
    std::map<uint32, NPC*> m_npcs;
    std::map<uint32, Client*> m_clients;
    std::map<uint32, SystemEntity*> m_entities;         // this list is all entities in this system.  we own these.
    std::map<uint32, SystemEntity*> m_ticEntities;      // this list is for entities that need process tics (objects, npc, client ships)
    std::map<uint32, SystemEntity*> m_staticEntities;   // this list is for static entities to send in setstate

private:
    // for spawn systems     -allan 15July15
    uint8 m_beltCount;
    uint8 m_gateCount;
    uint8 m_activeRatSpawns;
    uint16 m_activeRoidSpawns;
    std::vector<uint32> m_beltVector;
    SpawnBubbleVec m_ratBubbles;  // map of ids of bubbles with rat spawns
    SpawnBubbleVec m_roidBubbles;  // map of ids of bubbles with roid spawns

    // for POS system       -allan 23July17
    std::map<uint32, SystemEntity*> m_moonMap;        // our container, but we DONT own the SE*
    std::map<uint32, SystemEntity*> m_planetMap;      // our container, but we DONT own the SE*

    // for grid Unloading system  -allan  27June2015
    bool m_loaded;
    bool SystemActivity();
    uint32 m_players;
    uint32 m_activityTime;

    float m_secValue;

    // for bounty processing (20m timer)
    Timer m_bountyTimer;
    std::multimap<uint32, BountyData> m_bountyMap;  // charID/data

    // check for null iterator.  this will need to be moved to a memory code file eventually.
    // unfortunely, this is very specific for which iterators it can check.  see notes in code.
    bool IsNull(std::map<uint32, SystemEntity*>::iterator& i);
};

#endif