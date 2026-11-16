/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Rewrite:    Allan
*/

#pragma once

#include "system/BubbleManager.h"
#include "system/EntityFactory.h"
#include "system/SolarSystem.h"
#include "system/SystemDB.h"


class PyRep;
class PyDict;
class PyTuple;
class PyList;
class Client;
class NPC;
class InventoryItem;
class SpaceEntity;
class SystemBubble;
class SetState;

class AnomalyMgr;
class BeltMgr;
class DungeonMgr;
class SpawnMgr;
class PyServiceMgr;

class MoonSE;
class PlanetSE;
class GateSE;

class Civilian;

class SystemManager
{
public:
    // default c'tor
    SystemManager() =delete;
    // main c'tor
    SystemManager(uint32 systemID, PyServiceMgr &svc);//, ItemData idata);
    // copy c'tor
    SystemManager(const SystemManager* oth) =delete;
    // move c'tor
    SystemManager(SystemManager&& oth) noexcept =default;
    // copy assignment
    SystemManager& operator=(SystemManager& oth) =delete;
    // move assignment
    SystemManager& operator=(SystemManager&& oth) =default;
    // d'tor
    ~SystemManager();

    bool ProcessTic();          // called at 1Hz.
    bool BootSystem();
    void UnloadSystem();
    void UpdateData();          // called from EntityMgr every 5m for active systems

    bool IsLoaded()                                     { return m_loaded; }

    SpaceEntity* GetSE(uint32 entityID) const;
    NPC* GetNPCSE(uint32 entityID) const;
    ShipItemRef GetShipFromInventory(uint32 shipID);
    StationItemRef GetStationFromInventory(uint32 stationID);
    CargoContainerRef GetContainerFromInventory(uint32 contID);

    uint32 GetID() const                                { return m_data.systemID; }
    uint32 GetRegionID()                                { return m_data.regionID; }
    uint32 GetConstellationID()                         { return m_data.constellationID; }
    const std::string& GetNameStr() const               { return m_data.name; }
    const char* GetName() const                         { return m_data.name.c_str(); }
    // this is the letter/number rating
    const char* GetSecurityClass()                      { return m_data.securityClass.c_str(); }
    // 1.0 to -0.9
    const float GetSecurityRating()                     { return m_data.security; }
    // range is 0.1 for 1.0 system to 2.0 for -0.9 system
    float GetSecValue()                                 { return m_secValue; }

    PyServiceMgr* GetServiceMgr()                       { return &m_services; }
    Inventory* GetSystemInv()                           { return m_solarSystemRef->GetMyInventory(); }
    SolarSystemRef GetSystemRef()                       { return m_solarSystemRef; }

    // for spawn system     -allan 15July15     (not complete)
    typedef std::map<uint32, SystemBubble*> SpawnBubbleMap;
    void RemoveSpawnBubble(SystemBubble* pBubble);
    void GetSpawnBubbles( SystemManager::SpawnBubbleMap& bubbleMap );    // not used
    void IncRatSpawnCount()                             { ++m_activeRatSpawns; }
    void DecRatSpawnCount()                             { --m_activeRatSpawns; }
    void IncGateSpawnCount()                            { ++m_activeGateSpawns; }
    void DecGateSpawnCount()                            { --m_activeGateSpawns; }
    void IncRoidSpawnCount()                            { ++m_activeRoidSpawns; }
    void DecRoidSpawnCount()                            { --m_activeRoidSpawns; }
    uint8 BeltCount()                                   { return m_beltCount; }
    uint8 GetRatSpawnCount()                            { return m_activeRatSpawns; }
    uint16 GetRoidSpawnCount()                          { return m_activeRoidSpawns; }
    uint32 PlayerCount()                                { return m_players; }
    uint32 GetSysNPCCount()                             { return m_npcs.size(); }

    int32 GetTicCount()                                 { return m_ticStamp; }

    // CosmicMgr interface
    BeltMgr* GetBeltMgr()                               { return m_beltMgr; }
    SpawnMgr* GetSpawnMgr()                             { return m_spawnMgr; }
    AnomalyMgr* GetAnomMgr()                            { return m_anomMgr; }
    DungeonMgr* GetDungMgr()                            { return m_dungMgr; }

    bool BuildDynamicEntity(const DBSystemDynamicEntity& data, uint32 launcherID=0);

    void AddNPC(NPC* pNPC);
    void RemoveNPC(NPC* pNPC);
    void AddEntity(SpaceEntity* pSE, bool addSignal=true);    // add entity to system, and (optionally) add signal to AnomalyMgr
    void AddTicEntity(SpaceEntity* pSE);  // this is ONLY to add an entity to the ProcessTic() loop
    void RemoveEntity(SpaceEntity* pSE);   // this also removes SE* from bubble and sig from AnomalyMgr (if applicable)
    void RemoveTicEntity(SpaceEntity* pSE);  // this is ONLY to remove an entity from the ProcessTic() loop
    void AddClient(Client* pClient, bool count=false, bool jump=false);
    void AddMarker(SpaceEntity* pSE, bool sendBall=false, bool addSignal=false);    // rather specific here.
    void RemoveClient(Client* pClient, bool count=false, bool jump=false);
    void SetDockCount(Client* pClient, bool docked=false);

    void DoSpawnForBubble(SystemBubble* pBubble, uint8 type=0);  // called from bubble on timer
    void AddItemToInventory(InventoryItemRef item);
    void RemoveItemFromInventory(InventoryItemRef item);

    /* this is to send new global ball added to system AFTER SetState has been sent */
    void SendStaticBall(SpaceEntity* pSE);

    uint32 GetRandBeltID();
    uint32 GetClosestPlanetID(const Vector3d& myPos);

    // system bounty timer system.  20m delay
    void AddBounty(uint32 charID, BountyData& data);

    MoonSE*     GetClosestMoonSE(const Vector3d& myPos);
    PlanetSE*   GetClosestPlanetSE(const Vector3d& myPos);
    StargateSE* GetClosestGateSE(const Vector3d& myPos);

    // this returns entities in range for display on dscan.
    void DScan(int64 range, const Vector3d& position, std::vector< SpaceEntity* >& vector);
    // this returns entities in system for display on Groove's Entity Map in client
    PyRep* GetCurrentEntities();
    // this returns entities in system for display on ship scanner when enabled.
    void GetAllEntities(std::vector<CosmicSignature>& vector);

    std::map<uint32, SpaceEntity*> GetOperationalStatics() { return m_opStaticEntities; }
    std::map<uint32, SpaceEntity*> GetGates()          { return m_gateMap; }

    SpaceEntity* GetEntityByID(uint32 itemID);

    void GetClientList(std::vector<Client*>& cVec);

    std::map< uint32, SpaceEntity* > GetEntities()     { return m_entities; }

    PlanetSE* GetPlanet(uint32 planetID);

    void ResetAsteroids();

    // adds SEs to deleteLater list, which is purged on next tic
    void AddToDeleteLater(SpaceEntity* pSE)            { m_deleteLater[pSE->GetID()] = pSE; }

    // find all cans and wrecks in this system owned by calling char and set new fleetID
    void UpdateContainerFleetID(uint32 ownerID, uint32 fleetID);           // this will allow fleet salvagers to tractor wrecks and cans

    // for new Civilian System
    void AddCiv(Civilian* pCiv)                         { m_civilians.push_back(pCiv); }
    // also handles deletion
    void RemoveCiv(Civilian* pCiv);
    void GetCivPath(SpaceEntity* pOrig, SpaceEntity* pDest);

    /* relocated ex-destiny methods and calls */
    void CreateShipMarker(SpaceEntity* pSE);
    void RemoveAllMarkers();
    void RemoveShipMarkers();
    void MarkPoint(SpaceEntity* pSE, std::string& name, std::string& desc, bool orbit=false);

    /* update and event queue methods */
    // this will not consume *ev
    void SendSingleEvent(SpaceEntity* mySE, PyTuple** ev, bool self_only=false) const;
    // this will not consume *up
    void SendSingleUpdate(SpaceEntity* mySE, PyTuple** up, bool self_only=false) const;
    // this will consume all updates in vector
    void SendDestinyUpdates(SpaceEntity* mySE, std::vector< PyTuple* >& updates, bool self_only=false) const;

    /* GFX sending methods */
    void SendJumpOut(SpaceEntity* mySE) const;
    void SendJumpOutWormhole(SpaceEntity* mySE) const;
    void SendGateActivity(SpaceEntity* mySE) const;
    void SendWormholeActivity(SpaceEntity* mySE) const;
    void SendJumpInEffect(SpaceEntity* mySE, std::string JumpEffect) const;
    void SendJumpOutEffect(SpaceEntity* mySE, std::string JumpEffect, uint32 shipID) const;
    void SendJettisonPacket(SpaceEntity* mySE) const;
    void SendTerminalExplosion(SpaceEntity* mySE, uint32 shipID, uint32 bubbleID, bool isGlobal=false) const;
    void SendBallInteractive(SpaceEntity* mySE, bool set = false) const;
    // this is for space-type effects - warping, GateActivity, jumping, jettison, cloak
    // jumpout sends stargateID as target - wormhole sends ?? as otherTypeID
    void SendGFX10(SpaceEntity* mySE, std::string guid,
                   int32 targetID = 0, int32 otherTypeID = 0 ) const;
    // this is for structure effects
    // refactor this to avoid creating copies of args
    void SendGFX14(SpaceEntity* mySE, int32 moduleID, int32 moduleTypeID, int32 targetID,
                   int32 chargeTypeID, std::string guid, bool isOffensive, bool start,
                   bool isActive, int32 duration, int32 repeat, int64 startTime=0, int32 graphicInfo=0,
                   Client* pClient=nullptr) const;

    /* this is current state of system */
    void SendBallPark(SpaceEntity* mySE) const;

protected:
    /** @todo  this needs more work */
    void PayBounties();

    /* hack to avoid errors with dock count */
    void GetDockedCount();
    void GetPlayerCount();

    bool LoadCosmicMgrs();
    bool LoadSystemStatics();
    bool LoadSystemDynamics();
    bool LoadPlayerDynamics();

private:
    // --- 8-byte aligned group (Pointers, Refs, Containers, Structs) ---
    AnomalyMgr*     m_anomMgr;          // we own this, never NULL.
    BeltMgr*        m_beltMgr;          // we own this, never NULL.
    DungeonMgr*     m_dungMgr;          // we own this, never NULL.
    SpawnMgr*       m_spawnMgr;         // we own this, never NULL.

    PyServiceMgr&   m_services;
    SolarSystemRef  m_solarSystemRef;

    // static & dynamic system structs
    SystemData      m_data;
    SystemKillData  m_killData;

    // system entity lists
    std::map<uint32, NPC*>          m_npcs;             // npcID/ptr  we do not own these
    std::map<uint32, Client*>       m_clients;          // charID/ptr  we do not own these
    std::map<uint32, SpaceEntity*>  m_entities;         // all entities in this system. we own these.
    std::map<uint32, SpaceEntity*>  m_ticEntities;      // entities that need process tics
    std::map<uint32, SpaceEntity*>  m_staticEntities;   // static entities to send in setstate
    std::map<uint32, SpaceEntity*>  m_opStaticEntities; // operational static entities
    std::map<uint32, SpaceEntity*>  m_deleteLater;      // SEs deferred for deletion
    std::map<uint32, SpaceEntity*>  m_shipMarkers;      // ship position marker cans. we do own these.

    // POS & celestial maps (we do not own the SE*)
    std::map<uint32, SpaceEntity*>  m_moonMap;
    std::map<uint32, SpaceEntity*>  m_planetMap;
    std::map<uint32, SpaceEntity*>  m_gateMap;

    // dynamic data, bounty, spawn & civilian containers
    typedef std::map<uint16, uint8> RatDataMap;         // typeID/amt
    std::map<uint32, uint8>         m_jumpMap;          // timestamp/jumps
    std::map<uint32, BountyData>    m_bountyMap;        // charID/data
    std::map<uint32, RatDataMap>    m_ratMap;           // charID/rat data
    std::vector<uint32>             m_beltVector;       // belts in system
    std::vector<Civilian*>          m_civilians;
    SpawnBubbleMap                  m_ratBubbles;       // map of id/bubble with rat spawns
    SpawnBubbleMap                  m_roidBubbles;      // map of id/bubble with roid spawns

    // --- 4-byte aligned group (Timers, float, int32/uint32) ---
    Timer           m_bountyTimer;      // 20m timer (16 bytes, 4-byte aligned)
    Timer           m_minutetimer;      // PI timing (16 bytes, 4-byte aligned)
    float           m_secValue;         // range is 0.1 for 1.0 system to 2.0 for -0.9 system
    int32           m_ticStamp;         // system tic counter
    uint32          m_activityTime;
    uint32          m_minutes;

    // --- 2-byte aligned group (uint16) ---
    uint16          m_docked;
    uint16          m_players;          // current total count
    uint16          m_activeRoidSpawns;

    // --- 1-byte aligned group (uint8, bool) ---
    uint8           m_beltCount;
    uint8           m_gateCount;
    uint8           m_activeRatSpawns;
    uint8           m_activeGateSpawns;
    uint8           m_civDensity;
    uint8           m_civSpawnTic;
    bool            m_loaded;
    bool            m_entityChanged;    // plain bool is faster than :1 bitfield

    // --- Private helper methods ---
    void        ManipulateTimeData();
    void        SpawnCivilian();
    bool        SystemActivity();
    bool        SafeToUnload();

};

