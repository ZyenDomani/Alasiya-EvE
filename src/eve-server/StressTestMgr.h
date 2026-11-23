/**
 * StressTestMgr.h
 *      this class is for stress-testing server processing using temp NPC ships
 *  this manager will:
 *      use temporary itemIDs upon creation to avoid saves and other db hits.  this keeps db out of test
 *      create and maintain up to 2000 ships over multiple bubbles, either random points or belts/gates
 *      switch fleet's mode based on command (orbit, swarm, warp, combat, follow, random)
 *      handle all fleet movement and directions
 *      remove all fleet ships on command (clear)
 *      use existing class methods for all movement and combat (including new bump mechanics)
 *
 * @Author:     Allan
 * @Version:    0.10
 * @AI Version: 0.10
 * @Date:       20Nov26
 */

#pragma once

#include "eve-common.h"
#include "../eve-core/utils/timer.h"

#include "npc/NPC.h"

/*  design notes:
 *      .stresstest spawn <npc|ship> <count> [typeid] [spread]
 *      .stresstest mode <idle|orbit|swarm|warp|combat|follow|random>
 *      .stresstest status
 *      .stresstest clear [all]
 *      .stresstest ttl <minutes>
 *
 *
 * use addballs2 and avoid single-sending packets
 *  command handler found in SystemCommands
 */

class SpaceEntity;
class SystemManager;

class StressTestMgr
: public Singleton<StressTestMgr> {
public:

    struct Mode {
        enum {
            Idle     = 0,
            Orbit    = 1,
            Swarm    = 2,
            Warp     = 3,
            Combat   = 4,
            Follow   = 5,
            Random   = 6
        };
    };

    struct StressFleet {        // 84 bytes
        std::vector<SpaceEntity*> entities;
        Vector3d                  anchorPoint = NULL_ORIGIN;
        SystemManager*            pSystem = nullptr;
        Timer                     ttlTimer;
        int64                     startTime = 0;
        uint32                    cursor  = 0;    //staggered branch index
        uint32                    faction = 0;
        uint8                     mode = Mode::Idle;
        bool                      isNPC = true;   // npc/shipSE
    };

    // default c'tor
    StressTestMgr() =default;
    // copy c'tor
    StressTestMgr(const StressTestMgr& oth) =delete;
    // move c'tor
    StressTestMgr(StressTestMgr&& oth) noexcept =delete;
    // copy assignment
    StressTestMgr& operator=(const StressTestMgr& oth) =delete;
    // move assignment
    StressTestMgr& operator=(StressTestMgr&& oth) =delete;
    // d'tor
    ~StressTestMgr();

    void Process();

    void Spawn(SystemManager* pSystem, SystemBubble* pBubble, uint16 count=100,
               uint16 typeID=Item::Type::GistiiHijacker, uint16 spread=1000);
    void SetMode();
    void Clear(SystemManager* pSystem);

    void Status();

    bool HasActiveFleet()       { return !m_fleets.empty(); }


protected:
    NPC* SpawnNPC(SystemManager* pSystem, SystemBubble* pBubble, const Vector3d& spawnPt,
                  uint16 typeID, FactionData& data, bool warpIn=true);

private:
    std::unordered_map<uint32, StressFleet>     m_fleets;         // <sysID, data>
    std::unordered_map<uint8, uint16>     m_factionGroups;            //shipClass/groupID

};

#define sStressTest \
( StressTestMgr::get() )

