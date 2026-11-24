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

#include <fstream>
#include <cstdio>
#include <iomanip>

#include "Client.h"
#include "Profiler.h"
#include "StressTestMgr.h"
#include "system/BallPark.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"


StressTestMgr::~StressTestMgr() {
    ClearAll();
}

void StressTestMgr::Process() {
    if (m_fleets.empty())
        return;

    for (auto itr = m_fleets.begin(); itr != m_fleets.end(); ) {
        StressFleet& fleet = itr->second;

        // Check TTL expiration if enabled
        if (fleet.ttlTimer.Enabled() && fleet.ttlTimer.Check()) {
            ClearFleet(fleet);
            itr = m_fleets.erase(itr);
            continue;
        }

        // Prune dead ships (O(1) swap-and-pop) and tally active count
        size_t totalLive = 0;
        for (auto& gPair : fleet.groups) {
            auto& vec = gPair.second.entities;
            for (size_t i = 0; i < vec.size(); ) {
                if (vec[i] == nullptr || vec[i]->IsDead()) {
                    ++gPair.second.losses;
                    vec[i] = vec.back();
                    vec.pop_back();

                    if (fleet.reinforce && fleet.pSystem != nullptr) {
                        // Spawn a random replacement rat for this faction warping onto grid
                        SpawnReinforcement(fleet, gPair.second, itr->first);
                    }
                } else {
                    ++i;
                }
            }
            totalLive += vec.size();
        }

        if (totalLive == 0 || fleet.mode == Mode::Idle) {
            ++itr;
            continue;
        }

        // Process ~10% of the fleet per 1Hz tick to avoid single-frame spikes
        const size_t batchSize = std::max<size_t>(1, totalLive / 10);
        ProcessFleetBatch(fleet, batchSize);

        ++itr;
    }

    // Refresh HTML dashboard every 2 seconds while fleets are active
    if (m_htmlTimer.Check()) {
        WriteStatusHtml();
    }
}

void StressTestMgr::Spawn(SystemManager* pSystem, SystemBubble* pBubble, uint16 count/*100*/,
                          uint32 factionID/*factionUnknown*/, uint16 spread/*1000*/) {
    if ((pSystem == nullptr) or (pBubble == nullptr))
        return; // continue

    if (!IsFactionID(factionID))
        factionID = sDataMgr.GetRandomRatFactionID();

    if (spread < 1000)
        spread = 1000;

    const uint32 corpID = sDataMgr.GetFactionCorp(factionID);
    FactionData data;
        data.allianceID    = factionID;
        data.corporationID = corpID;
        data.factionID     = factionID;
        data.ownerID       = corpID;

    const Vector3d centerPt = pBubble->GetCenter();

    // Create or append to this bubble's StressFleet
    StressFleet& fleet = m_fleets[pBubble->GetID()];
    if (fleet.pSystem == nullptr) {
        fleet.pSystem     = pSystem;
        fleet.systemID    = pSystem->GetID();
        fleet.bubbleID    = pBubble->GetID();
        fleet.anchorPoint = centerPt;
        fleet.startTime   = GetFileTimeNow();
        fleet.isNPC       = true;
    }

    StressGroup& grp = fleet.groups[factionID];
    grp.factionID = factionID;
    grp.entities.reserve(grp.entities.size() + count);

    for (uint16 i = 0; i < count; ++i) {
        uint16 spawnTypeID = sDataMgr.GetRandFactionRat(factionID);

        // Scatter landing points within `spread` meters of bubble center
        Vector3d destPt = centerPt;
        if (spread > 0) {
            Vector3d jitter(MakeRandomFloat(-1.0f, 1.0f), MakeRandomFloat(-1.0f, 1.0f), MakeRandomFloat(-1.0f, 1.0f));
            jitter.Normalize();
            destPt += (jitter * MakeRandomFloat(500.0f, static_cast<float>(spread)));
        }

        NPC* pNPC = SpawnNPC(pSystem, pBubble, destPt, spawnTypeID, data, true);
        if (pNPC != nullptr) {
            grp.entities.push_back(pNPC);
            ++grp.spawnedCount;
        }
    }
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

    if (warpIn && pBubble != nullptr) {
        pNPC->WarpTo(pBubble, spawnPt, (MakeRandomInt(-5, 10) * 1000));
    } else {
        pSystem->AddNPC(pNPC);
    }

    return pNPC;
}

void StressTestMgr::ClearFleet(StressFleet& fleet)
{
    if (fleet.pSystem != nullptr) {
        fleet.pSystem->ClearStressWarps();
    }

    for (auto& gPair : fleet.groups) {
        for (SpaceEntity* pSE : gPair.second.entities) {
            if (pSE == nullptr)
                continue;
            if (pSE->SysBubble() != nullptr) {
                pSE->SysBubble()->Remove(pSE);
            }
            if (fleet.pSystem != nullptr) {
                fleet.pSystem->RemoveEntity(pSE);
            }
            SafeDelete(pSE);
        }
        gPair.second.entities.clear();
    }
    fleet.groups.clear();
}

// .stresstest clear (Current Bubble or Current System):
//      • Clears only the active test fleet in the GM's current bubble (pClient->GetShipSE()->SysBubble()->GetID())—or if the GM isn't in an active test bubble, clears all test fleets in pClient->SystemMgr().
void StressTestMgr::Clear(uint32 bubbleID) {
    auto itr = m_fleets.find(bubbleID);
    if (itr == m_fleets.end())
        return;

    ClearFleet(itr->second);
    m_fleets.erase(itr);
}

void StressTestMgr::ClearAll()
{
    for (auto& pair : m_fleets) {
        ClearFleet(pair.second);
    }
    m_fleets.clear();
    WriteStatusHtml(); // refresh dashboard to show 0 active fleets
}

void StressTestMgr::SetMode(uint32 bubbleID, uint8 mode, SpaceEntity* pAnchorSE /*nullptr*/)
{
    auto itr = m_fleets.find(bubbleID);
    if (itr == m_fleets.end())
        return;

    StressFleet& fleet = itr->second;
    fleet.mode   = mode;
    fleet.cursor = 0;

    if (pAnchorSE != nullptr) {
        fleet.anchorPoint = pAnchorSE->GetPosition();
    }

    for (auto& gPair : fleet.groups) {
        gPair.second.mode = mode;
    }

    // Immediate stop when switching to Idle
    if (mode == Mode::Idle) {
        for (auto& gPair : fleet.groups) {
            for (SpaceEntity* pSE : gPair.second.entities) {
                if (pSE != nullptr && pSE->GetMoveData() != nullptr) {
                    BallPark::Stop(pSE->GetMoveData());
                }
            }
        }
    }
}

void StressTestMgr::SpawnReinforcement(StressFleet& fleet, StressGroup& group, uint32 bubbleID) {

}

void StressTestMgr::ProcessFleetBatch(StressFleet& fleet, size_t batchSize)
{
    if (fleet.pSystem == nullptr || fleet.groups.empty())
        return;

    // Collect pointers to non-empty groups so Combat mode can pair opposing factions
    std::vector<StressGroup*> activeGroups;
    activeGroups.reserve(fleet.groups.size());
    size_t totalEntities = 0;
    for (auto& gPair : fleet.groups) {
        if (!gPair.second.entities.empty()) {
            activeGroups.push_back(&gPair.second);
            totalEntities += gPair.second.entities.size();
        }
    }

    if (totalEntities == 0)
        return;

    // Wrap cursor if entities were pruned during combat
    if (fleet.cursor >= totalEntities)
        fleet.cursor = 0;

    // Resolve an anchor entity in the bubble for Orbit / Follow modes
    SystemBubble* pBubble = sBubbleMgr.FindBubbleByID(fleet.bubbleID);
    SpaceEntity* pAnchorSE = (pBubble != nullptr) ? pBubble->GetRandomEntity() : nullptr;

    for (size_t step = 0; step < batchSize; ++step) {
        size_t idx = (fleet.cursor + step) % totalEntities;

        // Map flat index `idx` into (StressGroup*, SpaceEntity*)
        StressGroup* pMyGroup = nullptr;
        SpaceEntity* pSE      = nullptr;
        for (StressGroup* pGrp : activeGroups) {
            if (idx < pGrp->entities.size()) {
                pMyGroup = pGrp;
                pSE      = pGrp->entities[idx];
                break;
            }
            idx -= pGrp->entities.size();
        }

        if (pSE == nullptr || pSE->IsDead())
            continue;

        MoveData* mData = pSE->GetMoveData();
        if (mData == nullptr || mData->GetBallMode() == Ball::Mode::WARP)
            continue; // never interrupt an active warp

            uint8 activeMode = fleet.mode;
        if (activeMode == Mode::Random) {
            activeMode = static_cast<uint8>(MakeRandomInt(Mode::Orbit, Mode::Combat));
        }

        switch (activeMode) {
            case Mode::Orbit: {
                if (pAnchorSE != nullptr && pAnchorSE != pSE) {
                    mData->SetTargetSE(pAnchorSE);
                    mData->SetDesiredDistance(MakeRandomInt(5, 45) * 1000);
                    BallPark::SetSpeedFraction(mData, 1.0f);
                    BallPark::OrbitBall(mData);
                }
            } break;

            case Mode::Swarm: {
                Vector3d dir(MakeRandomFloat(-1.0f, 1.0f), MakeRandomFloat(-1.0f, 1.0f), MakeRandomFloat(-1.0f, 1.0f));
                dir.Normalize();
                mData->SetHeading(dir);
                BallPark::SetSpeedFraction(mData, MakeRandomFloat(0.5f, 1.0f));
                BallPark::GotoDirection(mData);
            } break;

            case Mode::Follow: {
                if (pAnchorSE != nullptr && pAnchorSE != pSE) {
                    mData->SetTargetSE(pAnchorSE);
                    mData->SetDesiredDistance(MakeRandomInt(2, 15) * 1000);
                    BallPark::SetSpeedFraction(mData, 1.0f);
                    BallPark::FollowBall(mData);
                }
            } break;

            case Mode::Warp: {
                // Warp batch to a different belt/bubble in the system (or back to fleet's home bubble)
                uint32 destBeltID = fleet.pSystem->GetRandBeltID();
                if (destBeltID == pSE->SysBubble()->GetID())
                    destBeltID = fleet.pSystem->GetRandBeltID();
                SpaceEntity* pDestSE = fleet.pSystem->GetSE(destBeltID);
                if (pDestSE != nullptr && pDestSE->SysBubble() != nullptr) {
                    SystemBubble* pDestBubble = pDestSE->SysBubble();
                    if (pDestBubble != nullptr && pDestBubble != pSE->SysBubble() && pSE->IsNPCSE()) {
                        static_cast<NPC*>(pSE)->WarpTo(pDestBubble, pDestBubble->GetCenter(), MakeRandomInt(2, 15) * 1000);
                    }
                }
            } break;

            case Mode::Combat: {
                if (!pSE->IsNPCSE() || activeGroups.size() < 2)
                    break;

                NPC* pNPC = static_cast<NPC*>(pSE);
                // Only assign a new enemy if this NPC doesn't already have a live target
                if (!pNPC->TargetMgr()->HasNoTargets())
                    break;

                // Pick a random opposing faction group
                StressGroup* pEnemyGrp = nullptr;
                for (size_t attempt = 0; attempt < activeGroups.size(); ++attempt) {
                    StressGroup* pCand = activeGroups[MakeRandomUInt(0, static_cast<uint32>(activeGroups.size() - 1))];
                    if (pCand->factionID != pMyGroup->factionID && !pCand->entities.empty()) {
                        pEnemyGrp = pCand;
                        break;
                    }
                }

                if (pEnemyGrp != nullptr) {
                    SpaceEntity* pEnemy = pEnemyGrp->entities[MakeRandomUInt(0, static_cast<uint32>(pEnemyGrp->entities.size() - 1))];
                    if (pEnemy != nullptr && !pEnemy->IsDead()) {
                        pNPC->GetAI()->Target(pEnemy);
                    }
                }
            } break;
        }
    }

    fleet.cursor = static_cast<uint32>((fleet.cursor + batchSize) % totalEntities);
}

void StressTestMgr::Status(Client* pClient) {
    WriteStatusHtml();
    //pClient->OpenBrowser(sConfig.files.StressHtmlUrl);
    //  <a href="http://localhost/stresstest.html">Open StressTest Monitor</a>

}

void StressTestMgr::WriteStatusHtml() {
    const std::string finalPath = sConfig.files.StressHtmlPath; // e.g. "/var/www/html/stresstest.html"
    if (finalPath.empty())
        return;

    const std::string tmpPath = finalPath + ".tmp";
    std::ofstream out(tmpPath.c_str(), std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        sLog.Error("StressTestMgr", "WriteStatusHtml: Unable to open %s for writing.", tmpPath.c_str());
        return;
    }

    // Collect and sort active fleets by (systemID, bubbleID)
    std::vector<const StressFleet*> sortedFleets;
    sortedFleets.reserve(m_fleets.size());
    for (const auto& pair : m_fleets) {
        sortedFleets.push_back(&pair.second);
    }
    std::sort(sortedFleets.begin(), sortedFleets.end(),
              [](const StressFleet* a, const StressFleet* b) {
                  if (a->systemID != b->systemID)
                      return a->systemID < b->systemID;
                  return a->bubbleID < b->bubbleID;
              });

    out << "<!DOCTYPE html><html><head><meta charset='utf-8'>"
    << "<meta http-equiv='refresh' content='2'>"
    << "<title>Alasiya-EvE StressTest Monitor</title>"
    << "<style>"
    << "body{background:#0b0e11;color:#cfd8dc;font-family:Consolas,monospace;font-size:12px;margin:12px;}"
    << "h2{color:#4fc3f7;border-bottom:1px solid #263238;padding-bottom:4px;margin:10px 0 6px;}"
    << "table{width:100%;border-collapse:collapse;margin-bottom:14px;}"
    << "th{background:#161d24;color:#81d4fa;text-align:left;padding:6px;border:1px solid #263238;}"
    << "td{padding:5px 6px;border:1px solid #1c252c;}"
    << "tr:nth-child(even){background:#10151a;}"
    << ".win{color:#66bb6a;font-weight:bold;} .warn{color:#ffa726;} .bad{color:#ef5350;}"
    << "</style></head><body>";

    // Top Profiler Telemetry Banner
    out << "<h2>Alasiya-EvE :: StressTest Live Monitor</h2>"
    << "<table><tr>"
    << "<th>Active Bubbles</th><th>EntityMgr Avg/Hi</th><th>SystemMgr Avg/Hi</th>"
    << "<th>Bubbles Avg/Hi</th><th>BallPark Avg/Hi</th><th>NPC AI Avg/Hi</th>"
    << "</tr><tr>"
    << "<td>" << sortedFleets.size() << "</td>"
    << "<td>" << sProfiler.GetAvg(Profile::Type::entityS) << " / " << sProfiler.GetHi(Profile::Type::entityS) << " us</td>"
    << "<td>" << sProfiler.GetAvg(Profile::Type::system)  << " / " << sProfiler.GetHi(Profile::Type::system)  << " us</td>"
    << "<td>" << sProfiler.GetAvg(Profile::Type::bubbles) << " / " << sProfiler.GetHi(Profile::Type::bubbles) << " us</td>"
    << "<td>" << sProfiler.GetAvg(Profile::Type::destiny) << " / " << sProfiler.GetHi(Profile::Type::destiny) << " us</td>"
    << "<td>" << sProfiler.GetAvg(Profile::Type::npc)     << " / " << sProfiler.GetHi(Profile::Type::npc)     << " us</td>"
    << "</tr></table>";

    // Active Theaters Sorted by systemID : bubbleID
    out << "<h2>Active Test Bubbles (Sorted by SystemID : BubbleID)</h2>"
    << "<table><tr>"
    << "<th>System : Bubble</th><th>System Name</th><th>Mode</th><th>Entity Type</th>"
    << "<th>Live / Spawned</th><th>Pending Warp-Ins</th><th>Uptime</th>"
    << "</tr>";

    const int64 now = GetFileTimeNow();
    for (const StressFleet* pFleet : sortedFleets) {
        uint32 liveTotal = 0;
        uint32 spawnedTotal = 0;
        for (const auto& gPair : pFleet->groups) {
            liveTotal    += static_cast<uint32>(gPair.second.entities.size());
            spawnedTotal += gPair.second.spawnedCount;
        }

        const uint32 pendingWarps = (pFleet->pSystem != nullptr) ? pFleet->pSystem->GetPendingWarpCount() : 0;
        const int64 elapsedSec = (now - pFleet->startTime) / EvE::Time::Second;
        const char* sysName = (pFleet->pSystem != nullptr) ? pFleet->pSystem->GetName() : "Unknown";

        out << "<tr>"
        << "<td>" << pFleet->systemID << " : " << pFleet->bubbleID << "</td>"
        << "<td>" << sysName << "</td>"
        << "<td>" << GetModeName(pFleet->mode) << "</td>"
        << "<td>" << (pFleet->isNPC ? "NPC" : "ShipSE") << "</td>"
        << "<td>" << liveTotal << " / " << spawnedTotal << "</td>"
        << "<td>" << pendingWarps << "</td>"
        << "<td>" << elapsedSec << "s</td>"
        << "</tr>";
    }
    out << "</table>";

    // Faction Combat Breakdown
    out << "<h2>Faction Group Breakdown &amp; Combat Metrics</h2>"
    << "<table><tr>"
    << "<th>System : Bubble</th><th>Faction</th><th>Group Mode</th>"
    << "<th>Live / Spawned</th><th>Kills</th><th>Losses</th><th>Reinforcements</th><th>Shots</th><th>Missiles</th><th>Status</th>"
    << "</tr>";

    for (const StressFleet* pFleet : sortedFleets) {
        // Determine leading faction by highest live count in this bubble
        uint32 leadFactionID = 0;
        size_t highestLive = 0;
        for (const auto& gPair : pFleet->groups) {
            if (gPair.second.entities.size() > highestLive) {
                highestLive = gPair.second.entities.size();
                leadFactionID = gPair.first;
            }
        }

        for (const auto& gPair : pFleet->groups) {
            const StressGroup& grp = gPair.second;
            const bool isLeading = (pFleet->groups.size() > 1 && grp.factionID == leadFactionID && !grp.entities.empty());

            out << "<tr>"
            << "<td>" << pFleet->systemID << " : " << pFleet->bubbleID << "</td>"
            << "<td>" << sDataMgr.GetFactionName(grp.factionID) << "</td>"
            << "<td>" << GetModeName(grp.mode) << "</td>"
            << "<td>" << grp.entities.size() << " / " << grp.spawnedCount << "</td>"
            << "<td>" << grp.kills << "</td>"
            << "<td>" << grp.losses << "</td>"
            << "<td>" << grp.reinforcements << "</td>"
            << "<td>" << grp.shotsFired << "</td>"
            << "<td>" << grp.missilesFired << "</td>"
            << "<td class='" << (isLeading ? "win" : "") << "'>"
            << (grp.entities.empty() ? "WIPED" : (isLeading ? "LEADING" : "ACTIVE"))
            << "</td></tr>";
        }
    }

    out << "</table></body></html>";
    out.close();

    std::rename(tmpPath.c_str(), finalPath.c_str());
}

std::string StressTestMgr::GetModeName(uint8 mode) {
    switch (mode) {
        case 0: return "Idle";   //Idle     = 0,
        case 1: return "Orbit";  //Orbit    = 1,
        case 2: return "Swarm";  //Swarm    = 2,
        case 3: return "Warp";   //Warp     = 3,
        case 4: return "Combat"; //Combat   = 4,
        case 5: return "Follow"; //Follow   = 5,
        case 6: return "Random"; //Random   = 6
        default: return "Invalid";
    }
}
