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
#include "system/SystemBubble.h"

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

    struct StressGroup {
        std::vector<SpaceEntity*>   entities;           // 24 bytes
        uint32                      factionID     = 0;  // 4 bytes
        uint32                      spawnedCount  = 0;  // 4 bytes
        uint32                      kills         = 0;  // 4 bytes
        uint32                      losses        = 0;  // 4 bytes
        uint32                      reinforcements      = 0;  // 4 bytes
        uint32                      shotsFired    = 0;  // 4 bytes
        uint32                      missilesFired = 0;  // 4 bytes
        uint8                       mode          = Mode::Idle; // 1 byte
    };

    struct StressFleet {
        std::map<uint32, StressGroup> groups;               // 48 bytes (keyed by factionID)
        Vector3d                      anchorPoint = NULL_ORIGIN; // 24 bytes
        SystemManager*                pSystem     = nullptr;  // 8 bytes
        Timer                         ttlTimer;                   // 8 bytes
        int64                         startTime   = 0;        // 8 bytes
        uint32                        systemID    = 0;        // 4 bytes
        uint32                        bubbleID    = 0;        // 4 bytes
        uint32                        cursor      = 0;        // 4 bytes
        uint8                         mode        = Mode::Idle; // 1 byte
        bool                          isNPC       = true;     // 1 byte
        bool                                              reinforce   = true;
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
               uint32 factionID=factionUnknown, uint16 spread=1000);

    void SetMode(uint32 bubbleID, uint8 mode, SpaceEntity* pAnchorSE = nullptr);

    void ClearAll();
    void ClearFleet(StressFleet& fleet);
    void Clear(uint32 bubbleID);

    void Status(Client* pClient);

    bool HasActiveFleet()       { return !m_fleets.empty(); }

    void RecordShot(SpaceEntity* pShooter) {
        if (m_fleets.empty() || pShooter == nullptr || pShooter->SysBubble() == nullptr)
            return;
        auto itr = m_fleets.find(pShooter->SysBubble()->GetID());
        if (itr != m_fleets.end()) {
            auto gItr = itr->second.groups.find(pShooter->GetWarFactionID());
            if (gItr != itr->second.groups.end()) {
                ++gItr->second.shotsFired;
            }
        }
    }

    void RecordMissile(SpaceEntity* pShooter) {
        if (m_fleets.empty() || pShooter == nullptr || pShooter->SysBubble() == nullptr)
            return;
        auto itr = m_fleets.find(pShooter->SysBubble()->GetID());
        if (itr != m_fleets.end()) {
            auto gItr = itr->second.groups.find(pShooter->GetWarFactionID());
            if (gItr != itr->second.groups.end()) {
                ++gItr->second.missilesFired;
            }
        }
    }

    void RecordKill(SpaceEntity* pKiller) {
        if (m_fleets.empty() || pKiller == nullptr || pKiller->SysBubble() == nullptr)
            return;
        auto itr = m_fleets.find(pKiller->SysBubble()->GetID());
        if (itr != m_fleets.end()) {
            auto gItr = itr->second.groups.find(pKiller->GetWarFactionID());
            if (gItr != itr->second.groups.end()) {
                ++gItr->second.kills;
            }
        }
    }


protected:
    void WriteStatusHtml();
    void ProcessFleetBatch(StressFleet& fleet, size_t batchSize);
    void SpawnReinforcement(StressFleet& fleet, StressGroup& group, uint32 bubbleID);

    std::string GetModeName(uint8 mode);

    NPC* SpawnNPC(SystemManager* pSystem, SystemBubble* pBubble, const Vector3d& spawnPt,
                  uint16 typeID, FactionData& data, bool warpIn=true);

private:
    std::unordered_map<uint32, StressFleet>     m_fleets;         // <bubbleID, data>

    Timer m_htmlTimer;

};

#define sStressTest \
( StressTestMgr::get() )
