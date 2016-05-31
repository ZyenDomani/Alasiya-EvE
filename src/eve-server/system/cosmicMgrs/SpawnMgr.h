/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2011 The EVEmu Team
 *    For the latest information visit http://evemu.org
 *    ------------------------------------------------------------------------------------
 *    This program is free software; you can redistribute it and/or modify it under
 *    the terms of the GNU Lesser General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option) any later
 *    version.
 *
 *    This program is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License along with
 *    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 *    http://www.gnu.org/copyleft/lesser.txt.
 *    ------------------------------------------------------------------------------------
 *    Author:   Allan
 */

#ifndef _EVE_NPC_SPAWNMGR_H__
#define _EVE_NPC_SPAWNMGR_H__

#include <unordered_map>
#include "system/cosmicMgrs/ManagerDB.h"

struct SystemSpawnGroup { //reference to this bubble's data for spawn groups.  may need later.
    //SystemBubble* pSysBubble;   //cant use reference or pointer here...
    uint32 bubbleID;
    uint32 systemID;
    uint32 regionID;
    double secRating;
};

struct SpawnEntry {     // notes for me while creating/writing/testing
    bool enabled;       // is group timer enabled for this entry?
    uint8 spawnType;    // spawn type.  1 = roaming, 2 = static
    uint8 total;        // total number of this group spawned
    uint8 number;       // this rats number in group (to match up with above total)
    uint8 sub;          // spawn data subtype
    uint8 type;         // spawn data class id (in case we have to look it up again)
    uint16 typeID;      // rat type id
    uint32 itemID;      // rat entity id
    uint32 groupID;     // rat group id (may look into changing typeID within group later on respawn (for chaining))
    uint32 corpID;      // rat corp id
    uint32 factionID;   // rat faction id
    uint32 spawnID;     // spawn id (if needed to match up with other spawns of this group (multiple spawn types in this group))
    uint32 time;        // spawn group timer start time
};

struct SpawnGroup {
    uint32 typeID;  //typeID to spawn
    uint8 quantity; //quantity to spawn for this typeID
};

struct RatFactionGroups {  // notes for me while creating/writing/testing
    uint8 shipClass;      // shipclass - arbitrary
    uint32 groupID;     // item groupID
};

struct RatSpawnClass { // notes for me while creating/writing/testing
    uint8 type;     // this is spawn type.  see notes in SpawnDB.h
    uint8 sub;      // this is spawn class id.  see notes in SpawnDB.h
    uint8 f;        // frigate
    uint8 d;        // destroyer
    uint8 c;        // cruiser
    uint8 bc;       // battlecruiser
    uint8 bs;       // battleship
    uint8 h;        // hauler
    uint8 o;        // officer - swarm for rogue drones
    uint8 cf;       // commander frigate
    uint8 cd;       // commander destroyer
    uint8 cc;       // commander cruiser
    uint8 cbc;      // commander battlecruiser
    uint8 cbs;      // commander battleship
};

// this class is a singleton object to have a common place for all spawn data
class SpawnDataMgr
: public Singleton< SpawnDataMgr >
{
public:
    SpawnDataMgr();
    virtual ~SpawnDataMgr() { /* nothing do to yet */ }

    // Initializes the Table:
    int Initialize();

protected:
    void _Populate();

    typedef std::map<uint32, uint32> RegionFactionsDef;  // simple (k,v pair of regionID and factionID
    typedef std::unordered_multimap<uint32, uint32> RatGroupTypesDef;    //groupID is key, typeID is value
    typedef std::unordered_multimap<uint8, RatSpawnClass> RatSpawnClassDef;  // type is key
    typedef std::unordered_multimap<uint32, RatFactionGroups> RatFactionGroupsDef;    //factionID is key

public:
    RatGroupTypesDef m_types;       // this unordered_multimap holds the invType ids for rats, keyed by rat groupID
    RatSpawnClassDef m_classes;     // this unordered_multimap holds the spawn type data for individual spawns, keyed by type
    RegionFactionsDef m_regions;    // this simple map holds regionID/factionID data, keyed by regionID
    RatFactionGroupsDef m_groups;   // this unordered_multimap holds the groupIDs for each faction, keyed by factionID

private:
    ManagerDB m_db;
};

#define sSpawnDataMgr \
    ( SpawnDataMgr::get() )

class NPC;
class PyServiceMgr;
class SystemManager;
class SystemBubble;
class SpawnMgr
{
public:
    SpawnMgr(SystemManager* mgr, PyServiceMgr& svc);
    virtual ~SpawnMgr() { /* nothing do to yet */ }

    void Process();
    void DoSpawnForBubble(SystemBubble* pSysBubble, uint32 regionID, double secRating);

    void SpawnPopped(uint32 itemID);
    void SpawnDepopped(SystemBubble* pSysBubble, uint32 itemID);

    void StartMainTimer();
    bool IsTimerStarted() { return (m_mainTimer.Enabled()); }

protected:
    bool _FindSpawnForBubble(uint32 itemID);
    void PrepSpawn(SystemBubble* pSysBubble, uint32 regionID, double secRating);
    void MakeSpawn(SystemBubble* pSysBubble, uint32 factionID, uint8 type, uint8 subtype);
    void ReSpawn(SystemBubble* pSysBubble, SpawnEntry* spawnEntry);
    void RemoveSpawn(uint32 bubbleID, uint32 itemID);
    void MoveSpawn();

    uint32 GetRandTypeID(uint32 groupID);
    uint32 GetCorpID(uint32 factionID);

    typedef std::vector<NPC*> RatSpawningVec;
    typedef std::vector<SystemBubble*> RatBubbleVec;
    typedef std::vector<SpawnGroup> RatSpawnGroupVec;
    typedef std::vector<RatSpawnClass> RatSpawnClassVec;
    typedef std::vector<RatFactionGroups> RatFactionGroupsVec;
    typedef std::map<uint8, uint32> RatFactionGroupsMap;    //map to enable 'find'  shipClass is key
    typedef std::unordered_multimap<uint32, SpawnEntry*> SpawnEntryDef;    //bubbleID is key
    //typedef std::vector<uint32, SystemSpawnGroup> SystemSpawnGroupVec;  //systemID is key  *unused at this time*

private:
    SystemManager* m_system;    //we do not own this
    PyServiceMgr& m_services;    //we do not own this

    Timer m_mainTimer;
    Timer m_groupTimer;

    uint32 m_spawnID = 1;   //in case i need to track a specific spawn group.
    bool m_enabled = false;     //allow spawning?

    RatBubbleVec m_bubbles;
    SpawnEntryDef m_spawns;
    RatSpawningVec m_ratSpawns;
    RatSpawnGroupVec m_toSpawn;
    RatSpawnClassVec m_spawnClass;
    //SystemSpawnGroupVec m_spawnGroups;
    RatFactionGroupsMap m_factionGroups;
};


#endif  // _EVE_NPC_SPAWNMGR_H__