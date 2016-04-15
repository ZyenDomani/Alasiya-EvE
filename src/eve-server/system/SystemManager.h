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
*/

#ifndef __SYSTEMMANAGER_H_INCL__
#define __SYSTEMMANAGER_H_INCL__

#include "system/BubbleManager.h"
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
class DoDestiny_SetState;
class DestinyManager;

class AnomalyMgr;
class BeltMgr;
class DungeonMgr;
class SpawnMgr;
class PyServiceMgr;

class SystemManager
//: public Inventory,
//  public InventoryItem
{
public:
    SystemManager(uint32 systemID, PyServiceMgr &svc);//, ItemData idata);
    virtual ~SystemManager();

    //bubble stuff:
    BubbleManager bubbles;

    uint32 GetID() const                { return m_systemID; }
    const std::string& GetName() const  { return m_systemName; }
    uint32 GetRegionID()                { return m_regionID; }

    bool BootSystem();
    void UnloadSystem();

    void Process();             // called around 40-60Hz on my system
    bool ProcessDestiny();      // called at 1Hz.

    bool BuildDynamicEntity(Client* who, const DBSystemDynamicEntity& entity);

    void AddClient(Client* who, bool docked=false, bool count=false);
    void RemoveClient(Client* who, bool docked=false, bool count=false);
    void AddNPC(NPC* who);
    void RemoveNPC(NPC* who);
    void AddEntity(SystemEntity* who);
    void RemoveEntity(SystemEntity* who);

	// Managing NPC Spawns and related tasks:
    void DoSpawnForBubble(SystemBubble* pSysBubble);

    SystemEntity* get(uint32 entityID) const;

    void MakeSetState(const SystemBubble *bubble, DoDestiny_SetState &into) const;

    SystemDB* GetSystemDB()                 { return &m_db; }
    const char* GetSystemSecurityClass()    { return m_securityClass.c_str(); }
    const double GetSystemSecurityRating()  { return m_securityRating; }

    ItemFactory* itemFactory() const;

    PyServiceMgr* GetServiceMgr() { return &m_services; }

    void AddItemToInventory(InventoryItemRef item);
    void RemoveItemFromInventory(InventoryItemRef item);
    ShipRef GetShipFromInventory(uint32 shipID);
    SystemEntity* GetShipSEFromInventory(uint32 shipID);
    CargoContainerRef GetContainerFromInventory(uint32 contID);
    StationRef GetStationFromInventory(uint32 stationID);

    // for spawn system     -allan 15July15
    typedef std::vector<uint32> SpawnBubbleVec;
    uint8 BeltCount()                       { return m_beltCount; }
    uint8 GetRatSpawnCount()                { return m_activeRatSpawns; }
    uint8 GetRoidSpawnCount()               { return m_activeRoidSpawns; }
    void GetSpawnBubbles(SpawnBubbleVec* bubbleMap);
    void RemoveSpawnBubble();

protected:
    // Solar System Inventory Functions:
    //uint32 inventoryID() const { return itemID(); }
    //PyRep *GetItem() const { return std::unique_ptr<PyNone>(new PyNone()); }
    //void AddItem(InventoryItemRef item);

    // Solar System Dynamic Inventory manager:
    SolarSystemRef m_solarSystemRef;    // we do not own this

    bool LoadSystemStatics();
    bool LoadSystemDynamics();

    const uint32 m_systemID;
    double m_securityRating;
    std::string m_systemName;
    std::string m_securityClass;
    uint32 m_regionID = 0;

    SystemDB m_db;
    PyServiceMgr& m_services;    //we do not own this


    AnomalyMgr* m_anomMgr;   //we own this, never NULL.
    BeltMgr* m_beltMgr;      //we own this, never NULL.
    DungeonMgr* m_dunMgr;    //we own this, never NULL.
    SpawnMgr* m_spawnMgr;    //we own this, never NULL.

    //overall system entity lists:
    bool m_entityChanged = false;
    std::map<uint32, SystemEntity*> m_entities;    //we own these, but they are also referenced in m_bubbles

private:
    // item to hold current number of loaded systems
    uint32 m_systems = 0;

    // for spawn systems     -allan 15July15
    uint8 m_beltCount = 0;
    uint8 m_activeRatSpawns = 0;
    uint8 m_activeRoidSpawns = 0;
    SpawnBubbleVec m_ratBubbles;  // map of ids of bubbles with rat spawns
    SpawnBubbleVec m_roidBubbles;  // map of ids of bubbles with roid spawns


    //check for deleting inactive systems using gridUnloading  -allan  27June2015
    bool SystemActivity();
    uint32 m_clients = 0;
    uint32 m_activityTime = 0;

    // check for null iterator.  this will need to be moved to a memory code file eventually.
    // unfortunely, this is very specific for which iterators it can check.  see notes in code.
    bool IsNull(std::map<uint32, SystemEntity*>::iterator& i);
};

#endif