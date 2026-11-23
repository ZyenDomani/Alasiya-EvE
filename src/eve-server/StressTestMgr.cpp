/**
 * StressTestMgr.cpp
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


#include "StressTestMgr.h"
#include "system/SystemManager.h"




StressTestMgr::~StressTestMgr() {

}

void StressTestMgr::Process() {

}

void StressTestMgr::Spawn(SystemManager* pSystem, SystemBubble* pBubble, uint16 count/*100*/,
                          uint16 typeID/*GistiiHijacker*/, uint16 spread/*1000*/) {
    const Vector3d spawnPt = NULL_ORIGIN;
    uint32 factionID = factionAngel;

    // get faction's shipClass and groupID map...is this feasible?  it's fine...there's only 21 at this time.
    if (sDataMgr.GetNPCGroups(factionID, m_factionGroups)) {
        if (is_log_enabled(SPAWN__MESSAGE))
            _log(SPAWN__MESSAGE, "StressTestMgr::SpawnNPC() - m_factionGroups size is %zu.", m_factionGroups.size());
    }

    const uint32 corpID = sDataMgr.GetFactionCorp(factionID);

    FactionData data;
    data.allianceID    = factionID;
    data.corporationID = corpID;
    data.factionID     = factionID;
    data.ownerID       = corpID;

    StressFleet newFleet;

    /*
    std::vector<SpaceEntity*> entities;
    Vector3d                  anchorPoint = NULL_ORIGIN;
    SystemManager*            pSystem = nullptr;
    Timer                     ttlTimer;
    int64                     startTime = 0;
    uint32                    cursor  = 0;    //staggered branch index
    uint32                    faction = 0;
    uint8                     mode = Mode::Idle;
    bool                      isNPC = true;   // npc/shipSE
    */

    NPC* pNPC = SpawnNPC(pSystem, pBubble, spawnPt, typeID, data);
    if (pNPC == nullptr)
        return; // continue

    newFleet.entities.push_back(pNPC);

    m_fleets.emplace(pSystem->GetID(), newFleet);
    m_factionGroups.clear();
}


NPC* StressTestMgr::SpawnNPC(SystemManager* pSystem, SystemBubble* pBubble, const Vector3d& spawnPt,
                             uint16 typeID, FactionData& data, bool warpIn/*false*/)
{
    if ((pSystem == nullptr) or (pBubble == nullptr))
        return nullptr;

    Vector3d startPos = spawnPt;
    if (warpIn) {
        // Offset 1.0 AU away along a random unit vector so the fleet warps onto the grid
        Vector3d dir(MakeRandomFloat(-1.0f, 1.0f), MakeRandomFloat(-1.0f, 1.0f), MakeRandomFloat(-1.0f, 1.0f));
        dir.Normalize();
        startPos += (dir * ONE_AU_IN_METERS);
    }

    ItemData idata(typeID, data.ownerID, pSystem->GetID(), flagAutoFit, "", startPos, "StressNPC");
    InventoryItemRef iRef = sItemFactory.SpawnTempItem(idata);
    if (iRef.get() == nullptr) {
        sLog.Error("StressTestMgr", "SpawnNPC: Failed to spawn temp item for typeID %u.", typeID);
        return nullptr;
    }

    NPC* pNPC = new NPC(iRef, *(pSystem->GetServiceMgr()), pSystem, data);
    if (pNPC == nullptr)
        return nullptr;

    if (!pNPC->Load()) {
        sLog.Error("StressTestMgr", "SpawnNPC: Failed to load NPC data for typeID %u.", typeID);
        SafeDelete(pNPC);
        return nullptr;
    }

    pSystem->AddEntity(pNPC);

    if (warpIn && pBubble != nullptr)
        pNPC->WarpTo(pBubble, spawnPt, (MakeRandomInt(-5, 10) * 1000));

    return pNPC;
}

void StressTestMgr::Clear(SystemManager* pSystem)
{
    auto itr = m_fleets.find(pSystem->GetID());
    if (itr == m_fleets.end())
        return;

    StressFleet& fleet = itr->second;
    for (SpaceEntity* pSE : fleet.entities) {
        if (pSE == nullptr)
            continue;
        if (fleet.pSystem != nullptr) {
            fleet.pSystem->RemoveEntity(pSE);
        }
        SafeDelete(pSE);
    }

    m_fleets.erase(itr);
}

void StressTestMgr::SetMode() {

}

void StressTestMgr::Status() {

}

