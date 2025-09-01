
 /**
  * @name SpawnMgr.h
  *     NPC Spawn management system for Alasiya EvEmu
  *
  * @Author:    Allan
  * @date:      15Jul15
  *
  */


#ifndef _EVE_NPC_SPAWNMGR_H__
#define _EVE_NPC_SPAWNMGR_H__

#include <unordered_map>
#include "../../../eve-common/EVE_Spawn.h"
#include "system/cosmicMgrs/ManagerDB.h"

class NPC;
class PyServiceMgr;
class SystemManager;
class SystemBubble;
class DungeonMgr;

class SpawnMgr
{
public:
    SpawnMgr(SystemManager* mgr, PyServiceMgr& svc);
    ~SpawnMgr()                                 { /* nothing do to yet */ }

    bool Init();

    void Process();
    void SetDungMgr(DungeonMgr* pDmgr)                  { m_dungMgr = pDmgr; }
    // not working... warp a spawned npc group from one location to another (change bubbles)
    void WarpOutSpawn(NPC* pNPC, SystemBubble* pBubble);
    // update SpawnMgr on npcs new location (change bubbles)
    void MoveSpawn(NPC* pNPC, SystemBubble* pBubble);

    const char* GetSpawnClassName(int8 sClass);
    const char* GetSpawnGroupName(int8 sGroup);

    uint8 DoSpawnForBubble(SystemBubble* pBubble);
    void DoSpawnForAnomaly(SystemBubble* pBubble, uint8 spawnClass);
    void DoSpawnForMission(SystemBubble* pBubble, uint32 regionID);
    void DoSpawnForIncursion(SystemBubble* pBubble, uint32 regionID);

    // primative test for chained spawns
    bool IsChaining(uint16 bubbleID);
    // this will be used for all spawn types
    void SpawnKilled(SystemBubble* pBubble, uint32 itemID);    // this DOES NOT remove entity from system or bubble.  user must do this BEFORE calling.

    void StartRatTimer();
    void StopRatTimer()                                 { m_ratTimer.Disable(); }
    void StartRatGroupTimer();
    void StopRatGroupTimer()                            { m_ratGroupTimer.Disable(); }

    bool IsInitialized()                                { return m_initalized; }
    bool IsRatTimerStarted()                            { return m_ratTimer.Enabled(); }


protected:
    bool FindSpawnForBubble(uint16 bubbleID);
    bool PrepSpawn(SystemBubble* pBubble, uint8 sClass = Spawn::Class::None, uint8 level = 0);
    void MakeSpawn(SystemBubble* pBubble, uint32 factionID, uint8 sClass, uint8 level, bool anomaly=false);
    void ReSpawn(SystemBubble* pBubble, Spawn::Entry& spawnEntry);
    void RemoveSpawn(uint16 bubbleID, uint32 itemID);

    uint8 GetSpawnGroup(uint8 sClass);

    uint16 GetRandTypeID(uint8 sClass);


private:
    SystemManager* m_system;    //we do not own this
    PyServiceMgr& m_services;    //we do not own this
    DungeonMgr* m_dungMgr;    //we do not own this

    Timer m_ratTimer;
    Timer m_ratGroupTimer;
    Timer m_missionTimer;
    Timer m_incursionTimer;
    Timer m_deadspaceTimer;

    bool m_initalized;      //allow spawning?

    uint16 m_groupTimerSetTime;     //ms - this is for hard-coding the respawn timer time.

    uint32 m_spawnID;       //in case i need to track a specific spawn group.
    
    std::map<uint32, uint8> m_liveCount;         // bubbleID/rats alive
    std::vector<NPC*> m_ratSpawns;                      //vector of NPC* to spawn
    std::vector<Spawn::toSpawn> m_toSpawn;
    std::map<uint8, uint16> m_factionGroups;            //shipClass/groupID
    std::vector<RatSpawnClass>  m_ratSpawnClass;               // spawn class for ?
    std::vector<RatFactionGroups> m_factionGroupVec;            // ?  not used
    std::multimap<uint16, Spawn::Entry> m_spawns;         //bubbleID/npc data  - each spawned npc has separate entry here
    //std::vector<uint32, SystemSpawnGroup>  m_spawnGroups;
};


#endif  // _EVE_NPC_SPAWNMGR_H__