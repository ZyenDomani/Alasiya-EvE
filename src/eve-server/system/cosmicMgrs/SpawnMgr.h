
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

class NPC;
class PyServiceMgr;
class SystemManager;
class SystemBubble;
class SpawnMgr
{
public:
    SpawnMgr(SystemManager* mgr, PyServiceMgr& svc);
    virtual ~SpawnMgr()                                 { /* nothing do to yet */ }

    bool Init();

    void Process();
    void DoSpawnForBubble(SystemBubble* pSysBubble, uint32 regionID, double secRating);
    void DoSpawnForAnomaly(int32 spawnID);

    void SpawnPopped(uint32 itemID);
    void SpawnDepopped(SystemBubble* pSysBubble, uint32 itemID);

    void StartMainTimer();

    bool IsEnabled()                                    { return m_enabled; }
    bool IsInitialized()                                { return m_initalized; }
    bool IsTimerStarted()                               { return m_mainTimer.Enabled(); }


protected:
    bool _FindSpawnForBubble(uint16 itemID);
    void PrepSpawn(SystemBubble* pSysBubble, uint32 regionID, double secRating);
    void MakeSpawn(SystemBubble* pSysBubble, uint32 factionID, uint8 type, uint8 subtype);
    void ReSpawn(SystemBubble* pSysBubble, SpawnEntry& spawnEntry);
    void RemoveSpawn(uint32 bubbleID, uint32 itemID);
    void MoveSpawn();

    uint32 GetRandTypeID(uint32 groupID);

    typedef std::vector<NPC*> RatSpawningVec;
    typedef std::vector<SystemBubble*> RatBubbleVec;
    typedef std::vector<SpawnGroup> RatSpawnGroupVec;
    typedef std::vector<RatSpawnClass> RatSpawnClassVec;
    typedef std::vector<RatFactionGroups> RatFactionGroupsVec;
    typedef std::map<uint8, uint32> RatFactionGroupsMap;    //map to enable 'find'  shipClass is key
    typedef std::unordered_multimap<uint16, SpawnEntry> SpawnEntryDef;    //bubbleID is key
    //typedef std::vector<uint32, SystemSpawnGroup> SystemSpawnGroupVec;  //systemID is key  *unused at this time*

private:
    SystemManager* m_system;    //we do not own this
    PyServiceMgr& m_services;    //we do not own this

    Timer m_mainTimer;
    Timer m_groupTimer;

    bool m_enabled;         //allow spawning?
    bool m_initalized;      //allow spawning?

    uint32 m_spawnID;       //in case i need to track a specific spawn group.

    RatBubbleVec m_bubbles;
    SpawnEntryDef m_spawns;
    RatSpawningVec m_ratSpawns;
    RatSpawnGroupVec m_toSpawn;
    RatSpawnClassVec m_spawnClass;
    //SystemSpawnGroupVec m_spawnGroups;
    RatFactionGroupsMap m_factionGroups;
};


#endif  // _EVE_NPC_SPAWNMGR_H__