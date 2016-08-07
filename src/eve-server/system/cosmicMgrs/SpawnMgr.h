
 /**
  * @name SpawnMgr.h
  *     NPC Spawn managment system for Alasiya EvEmu
  *
  * @Author:    Allan
  * @date:      15Jul15
  *
  */


#ifndef _EVE_NPC_SPAWNMGR_H__
#define _EVE_NPC_SPAWNMGR_H__

#include <unordered_map>
#include "system/cosmicMgrs/ManagerDB.h"

// this class is a singleton object to have a common place for all spawn data
class SpawnDataMgr
: public Singleton< SpawnDataMgr >
{
public:
    SpawnDataMgr();
    virtual ~SpawnDataMgr();

    // Initializes the Table:
    int Initialize();

protected:
    void _Populate();

    typedef std::map<uint32, uint32> RegionFactionsDef;  //regionID is key, factionID is value
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
    virtual ~SpawnMgr()                                 { /* nothing do to yet */ }

    void Process();
    void DoSpawnForBubble(SystemBubble* pSysBubble, uint32 regionID, double secRating);

    void SpawnPopped(uint32 itemID);
    void SpawnDepopped(SystemBubble* pSysBubble, uint32 itemID);

    void StartMainTimer();

    bool IsEnabled()                                    { return m_enabled; }
    bool IsTimerStarted()                               { return m_mainTimer.Enabled(); }

protected:
    bool _FindSpawnForBubble(uint32 itemID);
    void PrepSpawn(SystemBubble* pSysBubble, uint32 regionID, double secRating);
    void MakeSpawn(SystemBubble* pSysBubble, uint32 factionID, uint8 type, uint8 subtype);
    void ReSpawn(SystemBubble* pSysBubble, SpawnEntry& spawnEntry);
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
    typedef std::unordered_multimap<uint32, SpawnEntry> SpawnEntryDef;    //bubbleID is key
    //typedef std::vector<uint32, SystemSpawnGroup> SystemSpawnGroupVec;  //systemID is key  *unused at this time*

private:
    SystemManager* m_system;    //we do not own this
    PyServiceMgr& m_services;    //we do not own this

    Timer m_mainTimer;
    Timer m_groupTimer;

    uint32 m_spawnID = 1;   //in case i need to track a specific spawn group.
    bool m_enabled;          //allow spawning?

    RatBubbleVec m_bubbles;
    SpawnEntryDef m_spawns;
    RatSpawningVec m_ratSpawns;
    RatSpawnGroupVec m_toSpawn;
    RatSpawnClassVec m_spawnClass;
    //SystemSpawnGroupVec m_spawnGroups;
    RatFactionGroupsMap m_factionGroups;
};


#endif  // _EVE_NPC_SPAWNMGR_H__