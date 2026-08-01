
 /**
  * @name SpawnMgr.cpp
  *     NPC Spawn management system for Alasiya EvEmu
  *
  * @Author:         Allan
  * @date:          15 July 2015
  *
  */

#include "EVEServerConfig.h"
#include "PyServiceMgr.h"
#include "StaticDataMgr.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "system/DestinyManager.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"
#include "system/cosmicMgrs/SpawnMgr.h"
#include "system/cosmicMgrs/DungeonMgr.h"

// TODO:  UPDATE:  run thru this again, using lessons and insights learned over past 10 years to update this system

/** @todo  this can be updated to spawn mission, anomaly and deadspace rats.
 *   change all *roidRat* to *somethingelse* to better explain/describe the maps and how they're used.
 *  add objects for mission, anomaly and deadspace rats as needed to aid in tracking/deleting.
 *
 * add booleans for checking roam/static spawns and add anomaly checks to enable spawn mgr.
 *
 */

/*
 SPAWN__ERROR
 SPAWN__WARNING
 SPAWN__MESSAGE
 SPAWN__POP
 SPAWN__DEPOP
 SPAWN__TRACE
 */

/** @todo this class needs a bit more tweaking to work as designed...may/may not spawn all types correctly at this time.  */
SpawnMgr::SpawnMgr(SystemManager* mgr, PyServiceMgr& svc)
: m_system(mgr),
m_services(svc),
m_dungMgr(nullptr),
m_ratTimer(0),
m_ratGroupTimer(0),
m_missionTimer(0),
m_incursionTimer(0),
m_deadspaceTimer(0),
m_initalized(false),
m_squadID(1),
m_groupTimerSetTime(0),
m_spawnID(1)    // gotta start somewhere
{
}

bool SpawnMgr::Init() {
    // even if belt spawns arent activated, still allow anomaly spawning
    m_initalized = true;

    if (!sConfig.npc.RoamingSpawns and !sConfig.npc.StaticSpawns) {
        _log(COSMIC_MGR__INIT, "Belt Spawns Disabled while Initializing SpawnMgr for %s(%u) - config option off.", m_system->GetName(), m_system->GetID());
        return true;
    }

    if (m_system->BeltCount() < 1) {
        _log(COSMIC_MGR__INIT, "Belt Spawns Disabled for %s(%u) - no belts.", m_system->GetName(), m_system->GetID());
        return true;
    }

    m_groupTimerSetTime = 150;  // (in seconds) 2.5m default check time. this will allow a max wait time of 7.5m for respawn

    if (is_log_enabled(COSMIC_MGR__INIT)) {
        _log(COSMIC_MGR__INIT, "SpawnMgr Initialized for %s(%u)", m_system->GetName(), m_system->GetID());
        _log(COSMIC_MGR__INIT, "Roaming Belt Spawns are %s", sConfig.npc.RoamingSpawns ? "enabled" : "disabled");
        _log(COSMIC_MGR__INIT, "Static Gate Spawns are %s", sConfig.npc.StaticSpawns ? "enabled" : "disabled");
    }

    return m_initalized;
}

// this is only for rats.
void SpawnMgr::Process() {
    if (!m_initalized)
        return;

    double profileStartTime = GetTimeUSeconds();
    // called by SystemManager::Process() for each system.  this will need to be fast.  (it is max 187us  feb25)
    //  check timers and call appropriate functions as needed.

    // will have to think about this one a bit to implement properly (and quickly)
    /*  current implementation uses a single timer for entire system
     * timer goes off, do the following...
     *   loop thru all spawned bubbles in system
     *   spawn rats as needed to fill spawn groups
     *   kill timer after all spawns are full
     * this should be a single check for each iteration.  if multiple rats are killed, timer remains counting down from initial setting.
     * if all rats are respawned, then a rat is killed again (in this system), the timer is reset and the system begins again.
     *   each time the group timer is hit, it will check for missing rats in each spawn group.  if there is a missing rat, that rat is
     *   respawned and the timer is reset, to check again.  this *should* enable chain-ratting.
     *
     *  this will need work for correct operation as intended.
     * process should look at all entryDefs for each bubble, and spawn according to number/total for that bubble's spawnID
     * right now, that number isnt updated when rats are killed.  will have to work on coding that correctly
     */
    if (m_ratGroupTimer.Enabled())
        if (m_ratGroupTimer.Check()) {
            bool killTimer = true;
            std::multimap<uint16, Spawn::Entry>::iterator itr = m_spawns.begin(), end = m_spawns.end();
            while (itr != end) {
                if (itr->second.respawn) {
                    killTimer = false;
                    if (itr->second.stamp < sEntityMgr.GetStamp()) {
                        ++itr;
                        continue;
                    }
                    _log(SPAWN__TRACE, "Process() calling Respawn for SpawnEntryID %u  typeID %u", \
                            itr->second.spawnID, itr->second.typeID);
                    // this means check SpawnEntry for 'missing' SpawnGroup members and respawn as needed.
                    ReSpawn(sBubbleMgr.FindBubbleByID(itr->first), itr->second);
                    itr->second.respawn = false;
                }
                ++itr;
            }

            if (killTimer) {
                m_ratGroupTimer.Disable();
                _log(SPAWN__MESSAGE, "SpawnMgr::Process() - Rat Spawn Groups full (or no spawns) for %s(%u).  RatGroup Timer disabled.", \
                            m_system->GetName(), m_system->GetID());
            }
        }

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::spawn, GetTimeUSeconds() - profileStartTime);
}

bool SpawnMgr::FindSpawnForBubble(uint16 bubbleID) {
    return (m_spawns.find(bubbleID) != m_spawns.end());
}

void SpawnMgr::MoveSpawn(NPC* pNPC, SystemBubble* pBubble) {
    if (pNPC == nullptr)
        return;
    if (pBubble == nullptr)
        return;

    uint32 npcID(pNPC->GetID());
    _log(SPAWN__TRACE, "MoveSpawn() called by %s(%u) from bubbleID %u to bubbleID %u", \
            pNPC->GetName(), npcID, pNPC->SysBubble()->GetID(), pBubble->GetID() );

    // get npc spawn data and add to new location
    auto range = m_spawns.equal_range(pNPC->SysBubble()->GetID());
    // safety check
    if (range.first == range.second)
        return;
    auto itr = range.first;
    while (itr != range.second) {
        if (itr->second.itemID == npcID) {
            m_spawns.erase(itr);
            m_spawns.emplace(pBubble->GetID(), itr->second);
            break;
        }
        ++itr;
    }

    //find out if any rats remain before setting bubble spawned to false
    std::map<uint32, uint8>::iterator cItr = m_liveCount.find(pNPC->SysBubble()->GetID());
    if (cItr != m_liveCount.end()) {
        // decrement live counter for this bubble
        --(cItr->second);
        if (cItr->second < 1) {
            m_liveCount.erase(cItr);
            pNPC->SysBubble()->SetSpawned(false);
            _log(SPAWN__TRACE, "MoveSpawn():  bubbleID %u removed from spawn map", pNPC->SysBubble()->GetID());
        }
    }
}

void SpawnMgr::WarpOutSpawn(NPC* pNPC, SystemBubble* pBubble) {
    if ((pNPC == nullptr) or (pBubble == nullptr))
        return;
    if (is_log_enabled(SPAWN__TRACE))
        _log(SPAWN__TRACE, "WarpOutSpawn() called by %s(%u) from bubbleID %u to bubbleID %u", \
            pNPC->GetName(), pNPC->GetID(), pNPC->SysBubble()->GetID(), pBubble->GetID() );

    // set bubblespawn false before warping spawn
    pNPC->SysBubble()->SetSpawned(false);

    NPC* rNPC = nullptr;
    auto range = m_spawns.equal_range(pNPC->SysBubble()->GetID());
    // safety check
    if (range.first == range.second)
        return;

    std::unordered_multimap<uint16, Spawn::Entry> ratsToMove;
    auto itr = range.first;
    while (itr != range.second) {
        if (itr->second.respawn) {
            // should this negate the entire spawn move?
            ++itr;
            continue;
        }
        rNPC = m_system->GetNPCSE(itr->second.itemID);
        if (rNPC == nullptr) {
            // if npc is null, this should also be removed
            ++itr;
            continue;
        }
        rNPC->GetAI()->DisableWarpOutTimer();
        rNPC->DestinyMgr()->WarpTo(pBubble->GetCenter(), MakeRandomFloat(10, 30) * 100);
        ratsToMove.emplace(pBubble->GetID(), itr->second);
        ++itr;
    }

    m_spawns.erase(range.first, range.second);
    for (const auto& pair : ratsToMove)
        m_spawns.emplace(pair.first, pair.second);

    // set new bubblespawn true
    pBubble->SetSpawned(true);
}

// not used.  timer handled in SysBubble
void SpawnMgr::StartRatTimer() {
    if (m_ratTimer.Enabled())
        return;
    uint16 time = sConfig.npc.RoamingTimer * 1000;  //  s to ms
    if (sConfig.debug.SpawnTest)
        time = 5000; /* 5s for npc spawn testing */
    m_ratTimer.Start(time);

    if (is_log_enabled(SPAWN__MESSAGE))
        _log(SPAWN__MESSAGE, "SpawnMgr::StartRatTimer() - Main Spawn Timer started for %s(%u) at %u ms.", \
            m_system->GetName(), m_system->GetID(), time);
}

void SpawnMgr::StartRatGroupTimer() {
    if (m_ratGroupTimer.Enabled()) {
        if (is_log_enabled(SPAWN__MESSAGE))
            _log(SPAWN__MESSAGE, "SpawnMgr::StartRatGroupTimer() - Group Spawn Timer currently running.  Time left: %us", m_ratGroupTimer.GetRemainingTime() / 1000);
        return;
    }
    m_ratGroupTimer.Start(m_groupTimerSetTime * 1000);

    if (is_log_enabled(SPAWN__MESSAGE))
        _log(SPAWN__MESSAGE, "SpawnMgr::StartRatGroupTimer() - Group Spawn Timer started for %s(%u) at %us.", \
            m_system->GetName(), m_system->GetID(), m_groupTimerSetTime);
}

void SpawnMgr::SpawnKilled(SystemBubble* pBubble, uint32 itemID) {
    if (pBubble == nullptr)
        return;

    bool killed = true;
    std::map<uint32, uint8>::iterator cItr = m_liveCount.find(pBubble->GetID());
    if (cItr == m_liveCount.end()) {  // this should never hit
        // no entry for this bubble??
        m_liveCount[pBubble->GetID()] = pBubble->CountNPCs();
        cItr = m_liveCount.find(pBubble->GetID());
        _log(SPAWN__WARNING, "SpawnKilled() - bubble %u has no liveCount.  Hacking to %u", \
                pBubble->GetID(), cItr->second);
    }

    // this should be map to track squads here...what about multiple squads per bubble?
    NPCSquad* pSquad = nullptr; //GetSquad(pBubble->GetID());

    if (pSquad != nullptr) {
        // If the squad array has fallen completely silent, execute clean memory deletion
        if (pSquad->GetMembers().empty()) {
            _log(NPC__INFO, "SpawnManager: Reclaiming empty NPCSquad heap memory for bubble %u.", pBubble->GetID());

            // Remove the squad pointer from the manager's master index tracker list
            //UnlinkSquadIndex(pSquad);

            // The single point of clean, safe heap destruction!
            delete pSquad;
        }
    }

    if (pBubble->IsBelt()) {
        _log(SPAWN__DEPOP, "SpawnMgr::SpawnKilled::Belt - called by %u.", itemID);
        // if any SpawnEntry still exists for this bubble, reset group timer.
        // this enables chain ratting
        auto range = m_spawns.equal_range(pBubble->GetID());
        // safety check
        if (range.first == range.second)
            return;
        auto itr = range.first;
        while (itr != range.second) {
            if (itr->second.itemID == itemID) {
                // decrement live counter for this bubble
                --(cItr->second);
                if (itr->second.spawnClass < Spawn::Class::Hauler) {  // this will catch the 9 belt/gate spawn classes
                    itr->second.stamp = sEntityMgr.GetStamp() + sConfig.npc.RespawnTimer; // set respawn time, in seconds
                    itr->second.respawn = true;
                    killed = false;     // at least one rat left.
                } else if (itr->second.spawnClass < Spawn::Class::BeltSpawn) {
                    // hauler, commander or officer - no respawn
                    //   this is only to allow subsequent checks for non-belt/gate classes
                    RemoveSpawn(pBubble->GetID(), itemID);
                    //this threw the following error
                    //*** Error in `/srv/games/eve/Alasiya-EvE/bin/eve-server': double free or corruption (fasttop): 0x000000000e11fcf0 ***
                    //m_spawns.erase(itr);
                } else {
                    _log(SPAWN__DEPOP, "SpawnMgr::SpawnKilled::Belt - neither beltRat or special");
                }
                break;
            }

            ++itr;
        }

        // do we still have live ships for this bubble?
        if (cItr->second > 0) {
            _log(SPAWN__TRACE, "SpawnMgr::SpawnKilled::Belt - %u npcs in spawn with %u live.", \
                    m_spawns.count(pBubble->GetID()), cItr->second);
            StartRatGroupTimer();
        } else {
            _log(SPAWN__DEPOP, "SpawnMgr::SpawnKilled - Belt Spawn has been destroyed.  Resetting spawn checks for bubble %u.", pBubble->GetID());
            // spawn destroyed.  delete from list and reset bubble checks.
            m_spawns.erase(pBubble->GetID()); // just in case...but should be empty at this point
            pBubble->ResetBubbleRatSpawn();
            m_system->RemoveSpawnBubble(pBubble);
            m_ratGroupTimer.Disable();  // stop group timer, if enabled.
            return;
        }
    } else if (pBubble->IsGate()) {
        _log(SPAWN__DEPOP, "SpawnMgr::SpawnKilled::Gate - called by %u.", itemID);
        // decrement live counter for this bubble
        --(cItr->second);
        // we are not enabling rat chaining on gates.
        RemoveSpawn(pBubble->GetID(), itemID);
        // do we still have live ships for this bubble?
        if (cItr->second < 1) {
            // nope...this spawn is cleared.  reset
            //pBubble->SetSpawned(false);
            pBubble->ResetBubbleRatSpawn();
            m_system->RemoveSpawnBubble(pBubble);
        }
    } else if (pBubble->IsAnomaly()) {
        // this needs work...
        _log(SPAWN__DEPOP, "SpawnMgr::SpawnKilled::Anomaly - called by %u.", itemID);
        // decrement live counter for this bubble
        --(cItr->second);
        if (cItr->second == 1) {
            // last npc in this wave.  get data needed for next wave, if applicable.
            std::multimap<uint16, Spawn::Entry>::iterator itr = m_spawns.find(pBubble->GetID());
            if (itr == m_spawns.end())
                return; // this is an error.
            MakeSpawn(pBubble, itr->second.factionID, itr->second.spawnClass, itr->second.level);
            // now remove this spawn from map.
            m_spawns.erase(itr);
            // unlock warp gate if applicable
        } else  if (cItr->second < 1) {
            // this is an error...
        } else {
            // there are still npcs in this wave....continue.
        }

        /*  this needs to deal with multiple things.
         * 1- unlocking warp gates when needed per wave
         * 2- dropping loot according to (wave/dungeon/template)?
         * 3- after last spawn, possible escelation per dungeon type?   this should signal anomaly mgr to create the escelation
         * 4- spawn next wave, if applicable  (code above...currently testing)
         * 5- more/others?
         */
    } else if (pBubble->IsMission()) {
        _log(SPAWN__DEPOP, "SpawnMgr::SpawnKilled::Mission - called by %u.", itemID);
        // placeholder - not coded yet.
        /*  this needs to deal with multiple things.
         * 1- unlocking warp gates when needed per wave
         * 2- dropping loot according to (wave/mission/template)?
         * 3- setting mission completion status
         * 4- spawn next wave, if applicable
         * 5- more/others?
         */
        RemoveSpawn(pBubble->GetID(), itemID);
    } else if (pBubble->IsIncursion()) {
        _log(SPAWN__DEPOP, "SpawnMgr::SpawnKilled::Incursion - called by %u.", itemID);
        // placeholder - not coded yet.
        RemoveSpawn(pBubble->GetID(), itemID);
    } else {
        _log(SPAWN__DEPOP, "SpawnMgr::SpawnKilled::Other - called by %u.", itemID);
        RemoveSpawn(pBubble->GetID(), itemID);
    }
}

void SpawnMgr::DoSpawnForAnomaly(SystemBubble* pBubble, uint8 spawnClass) {
    if (pBubble == nullptr)
        return;
    pBubble->SetAnomaly();
    PrepSpawn(pBubble, spawnClass);
}

void SpawnMgr::DoSpawnForIncursion(SystemBubble* pBubble, uint32 regionID) {
    if (pBubble == nullptr)
        return;
    if (!IsRegionID(regionID))
        return;
    pBubble->SetIncursion();
    // unknown parameters at this time
}

void SpawnMgr::DoSpawnForMission(SystemBubble* pBubble, uint32 systemID) {
    if (pBubble == nullptr)
        return;
    if (!IsSolarSystemID(systemID))
        return;
    pBubble->SetMission();
    // unknown parameters at this time

    /*  this needs to deal with multiple things.
     * 1- npc types per template
     * 2- faction per template
     * 3- waves per template.
     * 4- more/others?
     */
}

uint8 SpawnMgr::DoSpawnForBubble(SystemBubble* pBubble) {
    if (pBubble == nullptr)
        return Bubble::Error::BubbleNull;
    if (pBubble->IsBelt()) {
        if (!sConfig.cosmic.BeltEnabled)
            return Bubble::Error::BeltDisabled;
        if (!sConfig.npc.RoamingSpawns)
            return Bubble::Error::RoamingDisabled;
    }

    if (pBubble->IsGate())
        if (!sConfig.npc.StaticSpawns)
            return Bubble::Error::StaticDisabled;

    if (FindSpawnForBubble(pBubble->GetID())) {
        _log(SPAWN__TRACE, "SpawnMgr::FindSpawnForBubble() returned true for bubble %u.", pBubble->GetID());
        pBubble->SetSpawned(true);  // bubble flag to avoid multiple spawns in same bubble.
        return Bubble::Error::Spawned;
    } else {
        if (PrepSpawn(pBubble)) {
            pBubble->SetSpawned(true);  // bubble flag to avoid multiple spawns in same bubble.
        } else {
            _log(SPAWN__TRACE, "SpawnMgr::PrepSpawn() returned false for bubble %u.", pBubble->GetID());
            return Bubble::Error::PrepFail;
        }
    }

    if (pBubble->IsBelt())
        m_system->IncRatSpawnCount();
    if (pBubble->IsGate())
        m_system->IncGateSpawnCount();

    return Bubble::Error::None;
}

void SpawnMgr::PlayerEnteredBubble(uint8 bubbleID, Client* pClient) {
    // this will only hit for gate and belt, and if spawntimer is enabled

    // get spawn, find rats, inform rats of new ship
    NPC* pNPC(nullptr);
    auto range = m_spawns.equal_range(bubbleID);
    auto itr = range.first;
    while (itr != range.second) {
        pNPC = m_system->GetNPCSE(itr->second.itemID);
        // not sure if this is needed or not...but just in case
        if (pNPC != nullptr)
            pNPC->GetAI()->ShipArrived(pClient);
        ++itr;
    }
}

bool SpawnMgr::PrepSpawn(SystemBubble* pBubble, uint8 sClass/*Spawn::Class::None*/, uint8 level/*0*/) {
    float secRating = m_system->GetSecurityRating();     // 1.0 to -0.9
    bool anomaly = false;
    // get faction for this region
    uint32 factionID = factionUnknown;  // default to rogue drones.
    if (sConfig.npc.RatFaction) {            // is RatFaction set in config?
        factionID = sConfig.npc.RatFaction;
    } else if (MakeRandomFloat() > 0.05f) {      // 5% chance for ANY spawn to be rogue drone.
        factionID = sDataMgr.GetRegionRatFaction(m_system->GetRegionID());
    }

    if (is_log_enabled(SPAWN__MESSAGE))
        _log(SPAWN__MESSAGE, "SpawnMgr::PrepSpawn() - faction: %s, region %u. (config set %s)", \
                sDataMgr.GetFactionName(factionID).c_str(), m_system->GetRegionID(), (sConfig.npc.RatFaction > 0?"true":"false"));

    // if rat, get possible spawn groups for this secRating.
    if (sClass == Spawn::Class::None) {
        if (pBubble->IsBelt()) {   // check for hauler, commander, officer spawn, but ONLY in a belt
            //NOTE  random checks here are for TESTING only....all rates are high.  make config option later?
            float rand = MakeRandomFloat();
            if (rand < sConfig.npc.OfficerChance) { // officer spawn
                if (factionID != factionUnknown) {  //but not for drones.  they dont have officers
                    sClass = Spawn::Class::Officer;
                } else {
                    //make this the rare drone hauler spawn (which isnt written yet)
                    //sClass = Spawn::Class::Hauler;
                    sClass = Spawn::Class::Crazy;
                }
            } else if (rand < sConfig.npc.CommanderChance) { // commander spawn
                sClass = Spawn::Class::Commander;
            } else if (rand < sConfig.npc.HaulerChance) { // hauler spawn
                if (factionID != factionUnknown) {
                    sClass = Spawn::Class::Hauler;
                } else {
                    //make this the rare drone hauler spawn (which isnt written yet and may not be...typeID out of range)
                    //sClass = Spawn::Class::Hauler;
                    sClass = Spawn::Class::Crazy;
                }
            // gonna be a 'regular' trusec-based spawn in a belt.
            } else if (secRating < -0.7f) {
                sClass = Spawn::Class::Insane;
            } else if (secRating < -0.4f) {
                sClass = Spawn::Class::Crazy;
            } else if (secRating < -0.1f) {
                sClass = Spawn::Class::Hard;
            } else if (secRating < 0.3f) {
                sClass = Spawn::Class::Medium;
            } else if (secRating < 0.6f) {
                sClass = Spawn::Class::Average;
            } else if (secRating < 0.8f) {
                sClass = Spawn::Class::Fair;
            } else {
                sClass = Spawn::Class::Easy;
            }

            if (secRating < 0.0f) {
                if (MakeRandomFloat() < 0.1)  // 10% chance to get hellspawn in nullsec
                    sClass = Spawn::Class::Hell;
            }
        } else if (pBubble->IsGate()) { // gate spawns are smaller/easier than roid spawns in hi-sec only
            if (secRating < -0.7f) {
                sClass = Spawn::Class::Crazy;
            } else if (secRating < -0.4f) {
                sClass = Spawn::Class::Hard;
            } else if (secRating < -0.1f) {
                sClass = Spawn::Class::Medium;
            } else if (secRating < 0.3f) {
                sClass = Spawn::Class::Average;
            } else if (secRating < 0.6f) {
                sClass = Spawn::Class::Fair;
            } else if (secRating < 0.8f) {
                sClass = Spawn::Class::Easy;
            } else {
                sClass = Spawn::Class::None;
            }
        } else {
            _log(SPAWN__WARNING, "Ratspawn location is neither belt nor gate in %s for bubble %u.", \
                    pBubble->GetSystem()->GetName(), pBubble->GetID());
            return false;
        }

        if ((sClass < Spawn::Class::Hell) and (secRating > 0.0f)) {
            // 15% chance to increase rat class in secure space
            if (MakeRandomFloat() < 0.15)
                ++sClass;
        }
    } else {
        // we were called from anomaly, incursion or mission to spawn npcs
        anomaly = true;
    }

    // get faction's shipClass and groupID map...is this feasible?  it's fine...there's only 21 at this time.
    if (sDataMgr.GetNPCGroups(factionID, m_factionGroups)) {
        if (is_log_enabled(SPAWN__MESSAGE))
            _log(SPAWN__MESSAGE, "SpawnMgr::PrepSpawn() - m_factionGroups size is %lu.", m_factionGroups.size());
    } else {
        _log(SPAWN__ERROR, "SpawnMgr::PrepSpawn() - No Faction data for %u.  Canceling spawn.", factionID);
        return false;
    }

    std::vector<RatSpawnClass> spawnEntry;
    if (!sDataMgr.GetNPCClasses(sClass, spawnEntry)) {
        _log(SPAWN__ERROR, "SpawnMgr::PrepSpawn() - No NPC Class data for %u(%s).  Canceling spawn.", \
                sClass, GetSpawnClassName(sClass));
        return false;
    }

    if (anomaly) {
        // anomaly, incursion or mission
        ++level;    // increment wave
        // check wave # vs possible waves.  (oob check)
        if (spawnEntry.size() < level) {
            _log(SPAWN__ERROR, "SpawnMgr::PrepSpawn() - spawnEntry.size (%lu) < level (%u) for anomaly class %s.  Canceling spawn.", \
                    spawnEntry.size(), level, GetSpawnClassName(sClass));
            return false;
        }
        /** @todo  test for overseer wave and spawn correct overseer for this anomaly  */
        /** @todo  make templates/functions/whatever for sending msgs to players local chat for waves */
    } else if (sClass == Spawn::Class::Hauler) {
        // split hauler spawns based on trusec
             if (secRating > 0.8f)   { level = MakeRandomUInt(0, 1); }
        else if (secRating > 0.6f)   { level = MakeRandomUInt(0, 3); }
        else if (secRating > 0.4f)   { level = MakeRandomUInt(1, 4); }
        else if (secRating > 0.2f)   { level = MakeRandomUInt(2, 5); }
        else if (secRating > 0.0f)   { level = MakeRandomUInt(2, 6); }
        else if (secRating > -0.2f)  { level = MakeRandomUInt(3, 7); }
        else if (secRating > -0.4f)  { level = MakeRandomUInt(3, 8); }
        else if (secRating > -0.6f)  { level = MakeRandomUInt(4, 9); }
        else if (secRating > -0.8f)  { level = MakeRandomUInt(5, 10); }
        else                         { level = MakeRandomUInt(6, 11); }
    } else if (sClass <= Spawn::Class::Hell) {
        level = MakeRandomUInt(0, spawnEntry.size() - 1);  // random belt/gate spawn type.
    } else if (sClass < Spawn::Class::BeltSpawn) {
        // determine level from secStatus  commander:31, officer:5
        level = MakeRandomUInt(0, spawnEntry.size() - 1);  // random commander/officer spawn type.
    } else {
        // do we need anything else here?
        _log(SPAWN__WARNING, "SpawnMgr::PrepSpawn() - spawnEntry.size(%lu) level(%u) class(%s).  !Anomaly and class > Hell.", \
                spawnEntry.size(), level, GetSpawnClassName(sClass));
        return false;
    }

    if (is_log_enabled(SPAWN__MESSAGE))
        _log(SPAWN__MESSAGE, "SpawnMgr::PrepSpawn() - spawnEntry - size: %lu, class: %s(%u), level: %u.", \
                spawnEntry.size(), GetSpawnClassName(sClass), sClass, level);

    // get ship class data from spawnEntry[subtype]
    // and put this spawn's group information in class designation
    uint8 f = spawnEntry[level].f;
    uint8 af = spawnEntry[level].af;
    uint8 d = spawnEntry[level].d;
    uint8 c = spawnEntry[level].c;
    uint8 ac = spawnEntry[level].ac;
    uint8 bc = spawnEntry[level].bc;
    uint8 bs = spawnEntry[level].bs;
    uint8 h = spawnEntry[level].h;
    uint8 o = spawnEntry[level].o;
    uint8 cf = spawnEntry[level].cf;
    uint8 cd = spawnEntry[level].cd;
    uint8 cc = spawnEntry[level].cc;
    uint8 cbc = spawnEntry[level].cbc;
    uint8 cbs = spawnEntry[level].cbs;
    std::string desc = spawnEntry[level].desc;

    // get typeIDs to spawn based on info in m_factionGroups and ship designators and put into Spawn Vector
    // figure out how to distinguish between roid, anomaly, incursion and mission defs for this....
    uint8 shipClass = 0;
    if (sClass > Spawn::Class::BeltSpawn)
        shipClass = 14;

    Spawn::toSpawn toSpawn = Spawn::toSpawn();
    // these types are for ALL spawn types.
    if (f > 0) {
        toSpawn.typeID = GetRandTypeID(1 + shipClass);
        toSpawn.quantity = f;
        m_toSpawn.push_back(toSpawn);
    }
    if (af > 0) {
        toSpawn.typeID = GetRandTypeID(2 + shipClass);
        toSpawn.quantity = af;
        m_toSpawn.push_back(toSpawn);
    }
    if (d > 0) {
        toSpawn.typeID = GetRandTypeID(3 + shipClass);
        toSpawn.quantity = d;
        m_toSpawn.push_back(toSpawn);
    }
    if (c > 0) {
        toSpawn.typeID = GetRandTypeID(4 + shipClass);
        toSpawn.quantity = c;
        m_toSpawn.push_back(toSpawn);
    }
    if (ac > 0) {
        toSpawn.typeID = GetRandTypeID(5 + shipClass);
        toSpawn.quantity = ac;
        m_toSpawn.push_back(toSpawn);
    }
    if (bc > 0) {
        toSpawn.typeID = GetRandTypeID(6 + shipClass);
        toSpawn.quantity = bc;
        m_toSpawn.push_back(toSpawn);
    }
    if (bs > 0) {
        toSpawn.typeID = GetRandTypeID(7 + shipClass);
        toSpawn.quantity = bs;
        m_toSpawn.push_back(toSpawn);
    }
    // end of possible non-roid rat types.  following are for belt/gate only
    if (sClass < Spawn::Class::BeltSpawn) {
        if (h > 0) {
            toSpawn.typeID = sDataMgr.GetHaulerTypeID(factionID, level);
            toSpawn.quantity = h;
            m_toSpawn.push_back(toSpawn);
        }
        if (o > 0) {
            toSpawn.typeID = GetRandTypeID(9);
            toSpawn.quantity = o;
            m_toSpawn.push_back(toSpawn);
        }
        if (cf > 0) {
            toSpawn.typeID = GetRandTypeID(10);
            toSpawn.quantity = cf;
            m_toSpawn.push_back(toSpawn);
        }
        if (cd > 0) {
            toSpawn.typeID = GetRandTypeID(11);
            toSpawn.quantity = cd;
            m_toSpawn.push_back(toSpawn);
        }
        if (cc > 0) {
            toSpawn.typeID = GetRandTypeID(12);
            toSpawn.quantity = cc;
            m_toSpawn.push_back(toSpawn);
        }
        if (cbc > 0) {
            toSpawn.typeID = GetRandTypeID(13);
            toSpawn.quantity = cbc;
            m_toSpawn.push_back(toSpawn);
        }
        if (cbs > 0) {
            toSpawn.typeID = GetRandTypeID(14);
            toSpawn.quantity = cbs;
            m_toSpawn.push_back(toSpawn);
        }
    }

    if (factionID == factionUnknown) {
        if ((bc > 0) or (bs > 0)) {
            if (sClass < Spawn::Class::BeltSpawn) {
                toSpawn.typeID = GetRandTypeID(9);
            } else if (sClass > Spawn::Class::BeltSpawn) {
                toSpawn.typeID = GetRandTypeID(22);
            }
            // spawn 4 swarm ships for each bc/bs
            toSpawn.quantity = ((bs > 0 ? bs : bc) * 4);
            m_toSpawn.push_back(toSpawn);
        } else if (o > 0) {
            // drones dont have officers.  spawn swarm x10
            if (sClass < Spawn::Class::BeltSpawn) {
                toSpawn.typeID = GetRandTypeID(9);
            } else if (sClass > Spawn::Class::BeltSpawn) {
                toSpawn.typeID = GetRandTypeID(22);
            }
            toSpawn.quantity = o * 10;
            m_toSpawn.push_back(toSpawn);
        }
    }

    //cleanup
    spawnEntry.clear();
    m_factionGroups.clear();

    if (m_toSpawn.size() > 0) {
        if (is_log_enabled(SPAWN__MESSAGE))
            _log(SPAWN__MESSAGE, "SpawnMgr::PrepSpawn() - Class: %s, Desc: %s - %lu ship class%s.", \
                   GetSpawnClassName(sClass), desc.c_str(), m_toSpawn.size(), m_toSpawn.size() > 1?"es":"");
        MakeSpawn(pBubble, factionID, sClass, level, anomaly);
        return true;
    }

    _log(SPAWN__ERROR, "SpawnMgr::PrepSpawn() - Nothing to spawn.");
    return false;
}

/*
struct Spawn::Entry {     // notes for me while creating/writing/testing
    bool respawn;       // is respawn enabled for this entry?  also provides conditional test for SpawnMgr::IsChaining() method
    uint8 spawnClass;   // spawn class.  0 = none, 1-7 = easy to insane based on sysSec, 8 = hauler, 9 = commander, 10 = officer
    uint8 spawnGroup;   // spawn group.   1 = roid rat, 2 = roaming, 3 = static, 4 = anomaly, 5 = mission, 6 = incursion, 7 = deadspace, 8 = sleeper
    uint8 total;        // total number of this group spawned
    uint8 number;       // this rat's number in group (to match up with above total)
    uint8 level;        // spawn data subtype/wave
    uint8 classID;      // spawn data class id (in case we have to look it up again)
    uint16 typeID;      // rat type id
    uint16 groupID;     // rat group id (may look into changing typeID within group later on respawn (for chaining))
    uint16 spawnID;     // spawn id (if needed to match up with other spawns of this group (multiple spawn types in this group))
    uint32 itemID;      // rat entity id
    uint32 corpID;      // rat corp id
    uint32 factionID;   // rat faction id
    uint16 stamp;       // entry stamp time to respawn (process conditional to allow for common timer and multiple respawn times)
};
*/
void SpawnMgr::MakeSpawn(SystemBubble* pBubble, uint32 factionID, uint8 sClass, uint8 level, bool anomaly/*false*/)
{
    _log(SPAWN__MESSAGE, "SpawnMgr::MakeSpawn() - Creating spawn class %s for %s in bubbleID %u (anomaly = %s).", \
            GetSpawnClassName(sClass), sDataMgr.GetFactionName(factionID).c_str(), pBubble->GetID(), anomaly?"true":"false");

    /*  the point here is to have all belt rats spawn outside their belt's bubble.
     * to make it 'realistic', they will need the appearance of warping in from some random point,
     *  to somewhere around bubble center.  this will make their origin appear elsewhere,
     * but not from same place every time.  they're pirates, they got other shit to do, too.
     *  eventually, when other systems are working, npcs will appear to 'warp in' from a hideout in the current system.
     *  this particular bit is not general knowledge and will have to be thought out a bit more before coding.
     *
     * for non-rat spawns, this is the intial spawn which and they will already be in pocket.
     *  waves will be spawned at structure (template positioning data), OR will warp in if no structure in pocket
     */
    Vector3d startPos = pBubble->GetCenter();
    Vector3d warpToPoint = startPos;

    std::string name = "BeltRat";
    if (anomaly) {
        name = GetSpawnClassName(sClass);
    } else {
        /** @todo  make method to get/use template positioning data for spawns here */
        // ratspawn will warp in, others will not.
        /*  NOTE:  rats warping into bubbles has been iffy at best for years.  discovered why today (16jul24)
         * npc accel dist < bubble radius.  WarpAccel.BubbleCheck is complete before npc is removed from bubble
         * warping from 1-1.5mm out isnt enough time to do things and get in new bubble decently.  ~55km from center
         */
        // 0.5AU from target bubble center.  this will give ship time to warp, change bubbles, warp-in target and be on overview >1s
        startPos.MakeRandomPointOnSphere(ONE_AU_IN_METERS * 0.5);
        SystemBubble* pBubble = sBubbleMgr.GetBubble(m_system, startPos);
        uint32 bubbleID = 0;
        if (pBubble != nullptr)
            bubbleID = pBubble->GetID();
        _log(SPAWN__POP, "SpawnMgr::MakeSpawn - NPC starting bubbleID %u", bubbleID);
    }

    // only create squad if there are more than 3 ships and it's beltrats
    //TODO:  this check isnt right...toSpawn is types, and each have qty...this only checks if there are >3 types
    NPCSquad* pCurrentSquad = nullptr;
    if ((sClass <= Spawn::Class::BeltSpawn) and (m_toSpawn.size() > 3)) {
        pCurrentSquad = new NPCSquad(m_squadID++);

        // Track the pointer under the Spawn Manager's absolute ownership map
        //m_activeSquads[pBubble->GetID()].push_back(pCurrentSquad);
    }

    uint32 corpID = sDataMgr.GetFactionCorp(factionID);
    FactionData data = FactionData();
        data.allianceID = factionID;    // this is to set wreck salvage correctly (tests for faction)
        data.corporationID = corpID;
        data.factionID = factionID;
        data.ownerID = corpID;

    NPC* pNPC = nullptr;
    InventoryItemRef iRef(nullptr);
    std::map<uint32, uint8>::iterator cItr = m_liveCount.find(pBubble->GetID());
    if (cItr == m_liveCount.end()) {
        // no entry for this bubble
        m_liveCount[pBubble->GetID()] = 0;
        cItr = m_liveCount.find(pBubble->GetID());
    }

    for (auto &cur : m_toSpawn) {
        /* ItemData(uint32 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, const char *_name = "",
         *          const Vector3d &_position = NULL_ORIGIN, const char *_customInfo = "", bool _contraband = false);
         */
        ItemData idata(cur.typeID, corpID, m_system->GetID(), flagAutoFit, "", startPos, name.c_str());
        for (uint8 x = 0; x < cur.quantity;) {
            iRef = sItemFactory.SpawnItem(idata);
            if (iRef.get() == nullptr) {
                _log(SPAWN__ERROR, "Failed to spawn item type %u.", cur.typeID);
                continue;
            }

            _log(SPAWN__POP, "SpawnMgr::MakeSpawn - Spawning NPC type %u (%u)", cur.typeID, iRef->itemID());

            pNPC = new NPC(iRef, m_services, m_system, data, this);
            if (pNPC == nullptr)
                continue;

            if (!pNPC->Load()) {
                _log(SPAWN__ERROR, "Failed to load NPC data for NPC %u with type %u, depoping.", pNPC->GetID(), pNPC->GetSelf()->typeID());
                pNPC->Delete();
                continue;
            }

            Spawn::Entry se = Spawn::Entry();
            se.respawn = false;
            se.itemID = iRef->itemID();
            se.groupID = iRef->type().groupID();
            se.total = cur.quantity;
            se.number = ++x;
            se.typeID = cur.typeID;
            se.spawnID = m_spawnID;
            se.corpID = corpID;
            se.factionID = factionID;
            se.spawnClass = sClass;
            se.spawnGroup = GetSpawnGroup(sClass);
            se.level = level;
            if (sClass < Spawn::Class::BeltSpawn) {  // this spawn is for rat.
                se.stamp = 0;   // this is for respawn time...do not set here.
            } else {
                se.stamp = sEntityMgr.GetStamp(); // set time of this spawn for ??
            }
            m_spawns.emplace(pBubble->GetID(), se);

            m_system->AddNPC(pNPC);

            uint8 tacticalTier = Squad::Tier::Rookie;
            float systemSec = m_system->GetSecurityRating();

            if (pCurrentSquad != nullptr) {
                // Register npc and invoke Rank Hierarchy checks to assign/promote leaders
                pCurrentSquad->RegisterMember(pNPC);
                // Determine tier based on security space and spawn difficulty class
                if (systemSec <= -0.5f || sClass == Spawn::Class::Insane || sClass == Spawn::Class::Hell || sClass == Spawn::Class::Sanctum) {
                    // Star Pilots: Fully coordinated null-sec squads or end-game anomalies
                    tacticalTier = Squad::Tier::Apex;
                } else if (systemSec <= 0.3f || (sClass >= Spawn::Class::Medium && sClass <= Spawn::Class::Crazy)) {
                    // Tactical Pilots: Low-sec, mid-sec, or moderate anomaly/belt groups
                    tacticalTier = Squad::Tier::Veteran;
                } else {
                    // Rookies: High-Sec space or low-tier belt rats
                    tacticalTier = Squad::Tier::Soldier;
                }

                // Assign the tracking parameters right onto the squad manager instance
              //  pCurrentSquad->SetTacticalTier(tacticalTier);
              //  pCurrentSquad->SetExpectsFormation(tacticalTier > 0);
            }

            //  begin warp.  this may have to be looked into later for timing of large spawns (>6)
            //  actually looks kinda cool when larger ships come in later...
            if (sClass <= Spawn::Class::BeltSpawn /*|| isFormation*/) {
                Vector3d warpTo = warpToPoint;
/*
                //this will need more work to properly set leader/follower positions

                // If Elite Tier (Null-Sec / High End Anomaly), they warp directly in formation!
                if (sConfig.npc.enableFormation and tacticalTier == 2 and x < formationOffsets.size()) {
                    warpTo.x += formationOffsets[x].x;
                    warpTo.y += formationOffsets[x].y;
                    warpTo.z += formationOffsets[x].z;

                    _log(SPAWN__TRACE, "NPC %u assigned to formation slot %u offset: (%.0f, %.0f, %.0f)",
                         pNPC->GetID(), x, formationOffsets[x].x, formationOffsets[x].y, formationOffsets[x].z);
                } else {*/
                    // Otherwise: Rookies and Mid-Tier squads use your beautiful staggered individual warp arrivals!
                    warpTo.MakeRandomPointOnSphere(sClass * 1000);

                //}

                pNPC->DestinyMgr()->WarpTo(warpTo, (MakeRandomInt(-5, 10) * 1000));
            }

            // increment live counter for this bubble
            ++(cItr->second);
            _log(SPAWN__TRACE, "MakeSpawn() adding %s as entry %u of %u. SpawnID: %u, Class: %s, Group: %s, Level: %u, Live: %u.", \
                    iRef->name(), x, cur.quantity, se.spawnID, GetSpawnClassName(se.spawnClass), \
                    GetSpawnGroupName(se.spawnGroup), level, cItr->second);
        }
    }

    ++m_spawnID;

    //cleanup
    m_toSpawn.clear();
    m_ratSpawns.clear();

    _log(SPAWN__TRACE, "MakeSpawn() completed in %s(%u) with %lu entities in m_spawns.", \
                m_system->GetName(), m_system->GetID(), m_spawns.size());
}

void SpawnMgr::ReSpawn(SystemBubble* pBubble, Spawn::Entry& spawnEntry) {
    //  we are NOT enabling spawn chaining for officer, hauler, or commander spawns.
    if (spawnEntry.spawnClass > Spawn::Class::Insane)
        return;

    Vector3d startPos = pBubble->GetCenter();
    Vector3d warpToPoint = startPos;
    startPos.MakeRandomPointOnSphere(MakeRandomInt(10, 15) * 100000); //1-1m5 km from bubble center
    _log(SPAWN__TRACE, "ReSpawn()  data for spawnEntryID %u  0x%X is type:%u, corp:%u, faction:%u, #:%u of %u", \
            spawnEntry.spawnID, &spawnEntry, spawnEntry.typeID, spawnEntry.corpID, \
            spawnEntry.factionID, spawnEntry.number, spawnEntry.total);
    /* ItemData( uint32 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, const char *_name = "",
     *           const Vector3d &_position = NULL_ORIGIN, const char *_customInfo = "", bool _contraband = false);
     */
    ItemData idata(spawnEntry.typeID, spawnEntry.corpID, m_system->GetID(), flagAutoFit, "", startPos, "BeltRat");
    InventoryItemRef iRef = sItemFactory.SpawnItem(idata);      // will have to work on this to NOT save npc to db.
    if (iRef.get() == nullptr) {
        _log(SPAWN__ERROR, "Failed to spawn item type %u.", spawnEntry.typeID);
        return;
    }
    // check for respawnable
    if (!iRef->GetAttribute(AttrEntityGroupRespawnChance).get_bool())
        return;

    _log(SPAWN__POP, "SpawnMgr::ReSpawn - Spawning NPC %s(%u)", iRef->name(), iRef->itemID());

    FactionData data = FactionData();
        data.allianceID = spawnEntry.factionID;   // this is to set wreck salvage correctly (tests for faction)
        data.corporationID = spawnEntry.corpID;
        data.factionID = spawnEntry.factionID;
        data.ownerID = spawnEntry.corpID;
    NPC* pNPC = new NPC(iRef, m_services, m_system, data, this);
    if (pNPC == nullptr) {
        _log(SPAWN__ERROR, "Failed to create NPC SE for item type %u.", spawnEntry.typeID);
        return;
    }

    if (!pNPC->Load()) {
        _log(SPAWN__ERROR, "Failed to load NPC data for NPC %u with type %u, depoping.", pNPC->GetID(), pNPC->GetSelf()->typeID());
        SafeDelete(pNPC);
        return;
    }

    m_system->AddNPC(pNPC);
    pNPC->DestinyMgr()->WarpTo(warpToPoint, (MakeRandomInt(-5, 10) * 1000));

    spawnEntry.stamp = 0;
    spawnEntry.respawn = false;

    std::map<uint32, uint8>::iterator cItr = m_liveCount.find(pBubble->GetID());
    if (cItr == m_liveCount.end()) {
        // no entry for this bubble...this should not hit here
        m_liveCount[pBubble->GetID()] = 1;
        _log(SPAWN__WARNING, "ReSpawn() - spawnEntryID %u in bubble %u has no liveCount.", \
                spawnEntry.spawnID, pBubble->GetID());
    } else {
        // increment live counter for this bubble
        ++(cItr->second);
    }

    _log(SPAWN__TRACE, "ReSpawn() completed for spawnEntryID %u 0x%X in bubble %u (%u live).", \
            spawnEntry.spawnID, &spawnEntry, pBubble->GetID(), cItr->second);
}

uint16 SpawnMgr::GetRandTypeID(uint8 sClass) {
    uint16 groupID = 0;
    std::map<uint8, uint16>::iterator itr = m_factionGroups.find(sClass);
    if (itr != m_factionGroups.end()) {
        groupID = itr->second;
        return sDataMgr.GetRandRatType(sClass, groupID);
    }

    _log(SPAWN__WARNING, "GetRandTypeID() - Spawn Class %s not found in m_factionGroups.", GetSpawnClassName(sClass));
    // default to actual npc type to avoid item spawn errors
    return Item::Type::GistiiHijacker;
}

bool SpawnMgr::IsChaining(uint16 bubbleID) {
    bool rsp = false;
    auto range = m_spawns.equal_range(bubbleID);
    auto itr = range.first;
    while (itr != range.second) {
        if (itr->second.respawn)
            rsp = true;
        ++itr;
    }

    return rsp;
}

void SpawnMgr::RemoveSpawn(uint16 bubbleID, uint32 itemID) {
    auto range = m_spawns.equal_range(bubbleID);
    auto itr = range.first;
    while (itr != range.second) {
        if (itr->second.itemID == itemID) {
            _log(SPAWN__TRACE, "RemoveSpawn() found item %u in spawnID %u and removed it.", itemID, itr->second.spawnID);
            m_spawns.erase(itr);
            return;
        }
        ++itr;
    }

    _log(SPAWN__TRACE, "RemoveSpawn() did not find item %u in bubble %u, out of %lu total spawns in the map.", itemID, bubbleID, m_spawns.size());
    return;
}

void SpawnMgr::NPCArrivedOnGrid(NPC* pNPC) {
    NPCSquad* pSquad = pNPC->GetSquad();
    if (pSquad == nullptr)
        return;
/*
    // Track arrivals against our expected wave size
    pSquad->DecrementArrivalsExpected();

    if (pSquad->AllMembersArrived()) {
        _log(SPAWN__TRACE, "Squad ID %u fully assembled on grid. Initiating tactical assembly broadcast.", pSquad->GetID());

        // 1. Assign the formation ID to the squad manager state
        pSquad->SetFormationID(EVEDB::Formations::Wedge);

        // 2. Build the exact addballs/formation notification package
        // that your Beyonce layer uses to tell the client: "We are a fleet now!"
        PyTuple* formationBcastPacket = pSquad->BuildClientFormationPacket();

        // 3. Broadcast it to all players currently residing in this bubble
        pNPC->SysBubble()->BubblecastDestinyUpdate(&formationBcastPacket, "NPC Formation Assembly");
        PySafeDecRef(formationBcastPacket);

        // 4. Wake up the AI: Command the squad members to begin their grouped combat orbits!
        pSquad->ExecuteGroupCombatDeployment();
    }*/
}

uint8 SpawnMgr::GetSpawnGroup(uint8 sClass) {
    switch (sClass) {
        case Spawn::Class::Extra:    // placeholder - not used yet
        case Spawn::Class::None:
            return Spawn::Group::None;
        case Spawn::Class::Easy:
        case Spawn::Class::Fair:
        case Spawn::Class::Average:
        case Spawn::Class::Medium:
        case Spawn::Class::Hard:
        case Spawn::Class::Crazy:
        case Spawn::Class::Insane:
        case Spawn::Class::Hell:
        case Spawn::Class::Hauler:
        case Spawn::Class::Commander:
        case Spawn::Class::Officer:
            return Spawn::Group::Roaming;
        case Spawn::Class::Hideaway:
        case Spawn::Class::Burrow:
        case Spawn::Class::Refuge:
        case Spawn::Class::Den:
        case Spawn::Class::Yard:
        case Spawn::Class::RallyPoint:
        case Spawn::Class::Port:
        case Spawn::Class::Hub:
        case Spawn::Class::Haven:
        case Spawn::Class::Sanctum:
        case Spawn::Class::Cluster:
        case Spawn::Class::Collection:
        case Spawn::Class::Assembly:
        case Spawn::Class::Gathering:
        case Spawn::Class::Surveillance:
        case Spawn::Class::Menagerie:
        case Spawn::Class::Herd:
        case Spawn::Class::Squad:
        case Spawn::Class::Patrol:
        case Spawn::Class::Horde:
          return Spawn::Group::Anomaly;
        case Spawn::Class::Hideout:
        case Spawn::Class::Lookout:
        case Spawn::Class::Watch:
        case Spawn::Class::Vigil:
        case Spawn::Class::Outpost:
        case Spawn::Class::Annex:
        case Spawn::Class::Base:
        case Spawn::Class::Fortress:
        case Spawn::Class::Complex:
        case Spawn::Class::StagingPoint:
        case Spawn::Class::HauntedYard:
        case Spawn::Class::DesolateSite:
        case Spawn::Class::ChemicalYard:
        case Spawn::Class::TrialYard:
        case Spawn::Class::DirtySite:
        case Spawn::Class::Ruins:
        case Spawn::Class::Independence:
        case Spawn::Class::Radiance:
        case Spawn::Class::Hierarchy:
            return Spawn::Group::Combat;
        case Spawn::Class::Crumbling:
        case Spawn::Class::Decayed:
        case Spawn::Class::Ruined:
        case Spawn::Class::Looted:
        case Spawn::Class::Ransacked:
        case Spawn::Class::Pristine:
        case Spawn::Class::Shard:
        case Spawn::Class::Tower:
        case Spawn::Class::Mainframe:
        case Spawn::Class::Center:
        case Spawn::Class::Server:
            return Spawn::Group::Deadspace;
    }
    // default
    return Spawn::Group::None;
}

const char* SpawnMgr::GetSpawnGroupName(int8 sGroup) {
    switch(sGroup) {
        case Spawn::Group::None:            return "None";
        case Spawn::Group::Roaming:         return "Roaming";
        case Spawn::Group::Static:          return "Static";
        case Spawn::Group::Anomaly:         return "Anomaly";
        case Spawn::Group::Combat:          return "Combat";
        case Spawn::Group::Deadspace:       return "Deadspace";
        case Spawn::Group::Mission:         return "Mission";
        case Spawn::Group::Incursion:       return "Incursion";
        case Spawn::Group::Sleeper:         return "Sleeper";
        case Spawn::Group::Escalation:      return "Escalation";
    }
    return "Undefined";
}

const char* SpawnMgr::GetSpawnClassName(int8 sClass) {
    switch(sClass) {
        case Spawn::Class::None:            return "None";
        case Spawn::Class::Easy:            return "Easy";
        case Spawn::Class::Fair:            return "Fair";
        case Spawn::Class::Average:         return "Average";
        case Spawn::Class::Medium:          return "Medium";
        case Spawn::Class::Hard:            return "Hard";
        case Spawn::Class::Crazy:           return "Crazy";
        case Spawn::Class::Insane:          return "Insane";
        case Spawn::Class::Hauler:          return "Hauler";
        case Spawn::Class::Commander:       return "Commander";
        case Spawn::Class::Officer:         return "Officer";
        case Spawn::Class::Hideaway:        return "Hideaway";
        case Spawn::Class::Burrow:          return "Burrow";
        case Spawn::Class::Refuge:          return "Refuge";
        case Spawn::Class::Den:             return "Den";
        case Spawn::Class::Yard:            return "Yard";
        case Spawn::Class::RallyPoint:      return "RallyPoint";
        case Spawn::Class::Port:            return "Port";
        case Spawn::Class::Hub:             return "Hub";
        case Spawn::Class::Haven:           return "Haven";
        case Spawn::Class::Sanctum:         return "Sanctum";
        case Spawn::Class::Cluster:         return "Cluster";
        case Spawn::Class::Collection:      return "Collection";
        case Spawn::Class::Assembly:        return "Assembly";
        case Spawn::Class::Gathering:       return "Gathering";
        case Spawn::Class::Surveillance:    return "Surveillance";
        case Spawn::Class::Menagerie:       return "Menagerie";
        case Spawn::Class::Herd:            return "Herd";
        case Spawn::Class::Squad:           return "Squad";
        case Spawn::Class::Patrol:          return "Patrol";
        case Spawn::Class::Horde:           return "Horde";
        case Spawn::Class::Hideout:         return "Hideout";
        case Spawn::Class::Lookout:         return "Lookout";
        case Spawn::Class::Watch:           return "Watch";
        case Spawn::Class::Vigil:           return "Vigil";
        case Spawn::Class::Outpost:         return "Outpost";
        case Spawn::Class::Annex:           return "Annex";
        case Spawn::Class::Base:            return "Base";
        case Spawn::Class::Fortress:        return "Fortress";
        case Spawn::Class::Complex:         return "Complex";
        case Spawn::Class::StagingPoint:    return "StagingPoint";
        case Spawn::Class::HauntedYard:     return "HauntedYard";
        case Spawn::Class::DesolateSite:    return "DesolateSite";
        case Spawn::Class::ChemicalYard:    return "ChemicalYard";
        case Spawn::Class::TrialYard:       return "TrialYard";
        case Spawn::Class::DirtySite:       return "DirtySite";
        case Spawn::Class::Ruins:           return "Ruins";
        case Spawn::Class::Independence:    return "Independence";
        case Spawn::Class::Radiance:        return "Radiance";
        case Spawn::Class::Hierarchy:       return "Hierarchy";
        case Spawn::Class::Crumbling:       return "Crumbling";
        case Spawn::Class::Decayed:         return "Decayed";
        case Spawn::Class::Ruined:          return "Ruined";
        case Spawn::Class::Looted:          return "Looted";
        case Spawn::Class::Ransacked:       return "Ransacked";
        case Spawn::Class::Pristine:        return "Pristine";
        case Spawn::Class::Shard:           return "Shard";
        case Spawn::Class::Tower:           return "Tower";
        case Spawn::Class::Mainframe:       return "Mainframe";
        case Spawn::Class::Center:          return "Center";
        case Spawn::Class::Server:          return "Server";
    }
    return "Undefined";
}
