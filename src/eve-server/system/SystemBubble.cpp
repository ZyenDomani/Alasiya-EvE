/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2016 The EVEmu Team
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
 *    Author:        Zhur
 *   Rewrite:   Allan
 */

#include <algorithm>

#include "EVEServerConfig.h"

#include "Client.h"
#include "EntityMgr.h"
#include "npc/Drone.h"
#include "npc/NPC.h"
#include "ship/modules/ActiveModule.h"
#include "system/BubbleManager.h"
#include "system/Container.h"
#include "system/DestinyManager.h"
#include "system/SystemBubble.h"
#include "system/SystemEntity.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/BeltMgr.h"


SystemBubble::SystemBubble(SystemManager* pSystem, const GPoint& center, double radius)
: m_tcuSE(nullptr), m_sbuSE(nullptr), m_ihubSE(nullptr), m_towerSE(nullptr),
m_system(pSystem), m_center(center), m_radius(radius),
m_centerSE(nullptr), m_hasMarkers(false),m_hasBubble(false),
m_spawnTimer(0),
m_type(Bubble::Type::Normal),
m_spawned(false), m_bubbleID(sBubbleMgr.GetBubbleID())
{
    _log(BUBBLE__TRACE, "SysBubble::Constructor - Created new bubble %u(%p) at (%.2f,%.2f,%.2f)[%.1f m radius].",\
	     m_bubbleID, this, m_center.x, m_center.y, m_center.z, m_radius);
}

SystemBubble::~SystemBubble() {
    // delete marker cans here
    for (auto &cur : m_markers) {
        cur.second->Delete();
        SafeDelete(cur.second);
    }
}

void SystemBubble::clear() {
    for (auto &cur : m_markers) {
        cur.second->Delete(); // delete marker cans here
        SafeDelete(cur.second);
    }

    m_spawned = false;
    m_hasBubble = false;
    m_hasMarkers = false;

    m_markers.clear();
    m_players.clear();
    m_entities.clear();
    m_dynamicEntities.clear();

    m_type = Bubble::Type::Normal;
}

void SystemBubble::Process() {
    /* this will need to process:
     *    belt and gate for spawn/respawn
     *    missions for ??
     *    incursions for ??
     */

    // rats in secure empire space?  nope.  its secure
    if (m_system->GetSecurityRating() > 0.9f)
        return;
    if (!m_spawnTimer.Enabled())
        return;

    switch (m_type) {
        case Bubble::Type::Normal: {
            // we're not doing any spawns in normal Bubbles
            return;
        } break;
        case Bubble::Type::Ice:
        case Bubble::Type::Belt:
        case Bubble::Type::Gate: {
            if (m_spawned or m_players.empty()) {
                m_spawnTimer.Disable();
                return;
            }
        } break;
        case Bubble::Type::Anomaly:
        case Bubble::Type::Mission:
        case Bubble::Type::Incursion:
        case Bubble::Type::Escalation: {

        } break;
    }

    if (m_spawnTimer.Check())
        m_system->DoSpawnForBubble(this, m_type);
}

//called from the bubble manager.
//if any entity is no longer in their registered bubble, they are added to the vector for re-classification.
void SystemBubble::ProcessWander(std::vector<SystemEntity*> &wanderers) {
    SystemEntity* pSE(nullptr);
    std::map<uint32, SystemEntity*>::iterator itr = m_dynamicEntities.begin();
    while (itr != m_dynamicEntities.end()) {
        pSE = itr->second;
        if (pSE == nullptr) {
            itr = m_dynamicEntities.erase(itr);
            continue;
        }

        // need to check for jumping ships here also. (not written yet)
        if ((pSE->DestinyMgr() != nullptr) and pSE->DestinyMgr()->IsWarping()) {
            ++itr;
            continue;
        }
        if (!InBubble(pSE->GetPosition())) {
            wanderers.push_back(pSE);
            //17:38:57 [DestinyWarning] SysBubble::ProcessWander() - entity 140006173(sys:30002507) not in bubble 1 for systemID 30002510.
            _log(DESTINY__WARNING, "SysBubble::ProcessWander() - entity %u(sys:%u) not in bubble %u for systemID %u.", \
                        pSE->GetID(), pSE->SystemMgr()->GetID(), m_bubbleID, m_system->GetID());
        }
        ++itr;
    }

    // why is this here?   why are we trying to reset a spawned bubble?
    //if (!m_players.empty() and m_spawned)
    //    ResetBubbleRatSpawn();
}

void SystemBubble::Add(SystemEntity* pSE) {
    //if they are already in this bubble, do not continue.
    if (m_entities.find(pSE->GetID()) != m_entities.end()) {
        _log(BUBBLE__MESSAGE, "SysBubble::Add() - Tried to add Static Entity %u to bubble %u, but it is already in here.",\
             pSE->GetID(), m_bubbleID);
        return;
    }

    pSE->m_bubble = this;
    // global entities also in SystemMgr's static list.  this is used for SystemBubble->IsEmpty() deletion check
    if (pSE->IsStaticEntity() or pSE->isGlobal()) {
        _log(BUBBLE__TRACE, "SysBubble::Add() - Entity %s(%u) is static or global or both.", pSE->GetName(), pSE->GetID());
        // all static and global entities (stations, gates, asteroid fields, cyno fields, etc) are put into bubble's staticEntity map
        m_entities[pSE->GetID()] = pSE;
        return;
    }

    //if they are already in this bubble, do not continue.
    if (m_dynamicEntities.find(pSE->GetID()) != m_dynamicEntities.end()) {
        _log(BUBBLE__MESSAGE, "SysBubble::Add() - Tried to add Dynamic Entity %u to bubble %u, but it is already in here.",\
                pSE->GetID(), m_bubbleID);
        return;
    }

    if (pSE->HasPilot()) {
        // Set spawn timer for this bubble, if needed
        switch (m_type) {
            case Bubble::Type::Normal: {
            } break;
            case Bubble::Type::Gate: {
                if (sConfig.npc.StaticSpawns)
                    if (!m_spawnTimer.Enabled())
                        SetSpawnTimer();
            } break;
            case Bubble::Type::Ice:
            case Bubble::Type::Belt: {
                // check for roids and load/spawn as needed, but only on first player to enter
                m_system->GetBeltMgr()->CheckSpawn(m_bubbleID);
                // if belt is spawned, inform npcs of new player entering bubble
                if (m_spawned)
                    m_system->GetSpawnMgr()->PlayerEnteredBubble(m_bubbleID, pSE->GetPilot());
                if (sConfig.npc.RoamingSpawns)
                    if (!m_spawnTimer.Enabled())
                        SetSpawnTimer();
            } break;
            case Bubble::Type::Anomaly: {
                m_system->GetSpawnMgr()->PlayerEnteredBubble(m_bubbleID, pSE->GetPilot());
            } break;
            case Bubble::Type::Mission: {
                sLog.Warning("SysBubble::Add()", "%s has entered a mission bubble", \
                        pSE->GetPilot()->GetName());
            } break;
            case Bubble::Type::Incursion: {
                sLog.Warning("SysBubble::Add()", "%s has entered a incursion bubble", \
                        pSE->GetPilot()->GetName());
            } break;
            case Bubble::Type::Escalation: {
                sLog.Warning("SysBubble::Add()", "%s has entered a escalation bubble", \
                        pSE->GetPilot()->GetName());
            } break;
        }

        Client* pClient(pSE->GetPilot());
        // this is sent in state when undocking
        if (!pClient->IsUndock())
            SendAddBalls2(pSE);
        if (!m_players.empty())
            AddBallExclusive(pSE);  // adds new player to all players in bubble, if any

        //check to see if any ships are using gfx.  if so, send all gfx to new ship
        for (auto &cur : m_activeModules)
            cur.second->SendGFX(false, pClient);
        // will need to do same thing for active drones, if any
        for (auto &cur : m_drones)
            cur.second->GetAI()->SendGFX(pClient);
        // and npcs, if any
        for (auto &cur : m_npcs) {
            cur.second->GetAI()->SendGFX(pClient);
            // Notify all active rats in this bubble
            cur.second->GetAI()->ShipArrived(pClient);
        }

        m_players[pClient->GetCharacterID()] = pClient;   //add to bubble's player list
    } else {
        // do we need to check bubble types here?
        if (!m_players.empty())
            AddBallExclusive(pSE);
        if (pSE->IsDroneSE())
            if (!pSE->IsAbandoned())
                m_drones[pSE->GetID()] = pSE->GetDroneSE();
    }

    // all non-global entities (players, npcs, roids, containers, etc) are put into bubble's dynamicEntity map
    m_dynamicEntities[pSE->GetID()] = pSE;

    if (is_log_enabled(BUBBLE__DEBUG)) {
        GPoint startPoint(pSE->GetPosition());
        GVector direction(startPoint, NULL_ORIGIN);
        double rangeToStar = direction.length();
        rangeToStar /= ONE_AU_IN_METERS;
        _log(BUBBLE__DEBUG, "SysBubble::Add() - Added entity %u to bubble %u.  Dist to center: %.2f", \
                pSE->GetID(), m_bubbleID, m_center.distance(pSE->GetPosition()));
        _log(BUBBLE__DEBUG, "SysBubble::Add() - Distance to Star %.2f AU.  %lu/%lu Entities in bubble %u",\
                rangeToStar, m_entities.size(), m_dynamicEntities.size(), m_bubbleID);
    } else {
        _log(BUBBLE__TRACE, "SysBubble::Add() - Added entity %u to bubble %u.  Dist to center: %.2f", \
                pSE->GetID(), m_bubbleID, m_center.distance(pSE->GetPosition()));
    }
}

void SystemBubble::Remove(SystemEntity *pSE) {
    //assume that the entity is properly registered for its ID
    if (pSE->m_bubble == nullptr) {
       // if (sConfig.debug.StackTrace)
            EvE::traceStack();
            return;
    }

    _log(BUBBLE__TRACE, "SysBubble::Remove() - Removing entity %u from bubble %u", pSE->GetID(), m_bubbleID);

    m_entities.erase(pSE->GetID());
    m_dynamicEntities.erase(pSE->GetID());

    if (pSE->HasPilot()) {
        m_players.erase(pSE->GetPilot()->GetCharacterID());
        RemoveBalls(pSE);
    }

    //notify everybody else in the bubble of the removal.
    if (!m_players.empty())
        RemoveBall(pSE);

    if (pSE->IsDroneSE())
        m_drones.erase(pSE->GetID());

    if (is_log_enabled(BUBBLE__DEBUG))
        sLog.Warning("SysBubble::Remove()", "Removing entity %u from bubble %u", pSE->GetID(), m_bubbleID);

    pSE->m_bubble = nullptr;
}

void SystemBubble::RemoveExclusive(SystemEntity *pSE) {
    if (pSE->m_bubble == nullptr)
        return;

    _log(BUBBLE__TRACE, "SysBubble::RemoveExclusive() - Removing entity %u from bubble %u", pSE->GetID(), m_bubbleID);
    RemoveBallExclusive(pSE);
}

void SystemBubble::ResetBubbleRatSpawn() {
    /* the current spawn in this bubble was killed off, so reset timers accordingly
     *   once the timer hits, it will do all needed checks for players and respawn as needed.
     *  this enables creating a new spawn after previous group was killed off
     */
    m_spawned = false;
    switch (m_type) {
        case Bubble::Type::Normal: {
            _log(SPAWN__WARNING, "ResetBubbleRatSpawn() called for normal bubble %u in %s.", \
                    m_bubbleID, m_system->GetID());
        } break;
        case Bubble::Type::Ice:
        case Bubble::Type::Belt: {
            if (sConfig.npc.RoamingSpawns)
                if (!m_spawnTimer.Enabled())
                    SetSpawnTimer();
        } break;
        case Bubble::Type::Gate: {
            if (sConfig.npc.StaticSpawns)
                if (!m_spawnTimer.Enabled())
                    SetSpawnTimer();
        } break;
        case Bubble::Type::Anomaly:
        case Bubble::Type::Mission:
        case Bubble::Type::Incursion: {
            // these will check for escalation...where?
        } break;
        case Bubble::Type::Escalation: {
        } break;
    }
}

void SystemBubble::SetSpawnTimer() {
    if (m_system->GetSecurityRating() > 0.9)
        return;

    if (sConfig.debug.SpawnTest) {
        // if we're testing, set to 5s and continue.
        m_spawnTimer.Start(5000);
    } else {
        switch (m_type) {
            case Bubble::Type::Normal: {
                // no spawn in normal bubble
                return;
            } break;
            case Bubble::Type::Ice:
            case Bubble::Type::Belt: {
                m_spawnTimer.Start(MakeRandomInt(30, sConfig.npc.RoamingTimer) * 1000);
            } break;
            case Bubble::Type::Gate: {
                m_spawnTimer.Start(MakeRandomInt(60, sConfig.npc.StaticTimer) * 1000);
            } break;
            case Bubble::Type::Anomaly:
            case Bubble::Type::Mission:
            case Bubble::Type::Incursion:
            case Bubble::Type::Escalation: {
                // this will need specific timers set in ??
                m_spawnTimer.Start(MakeRandomInt(60, sConfig.npc.StaticTimer) * 1000);
            } break;
        }
    }
}

void SystemBubble::SetBelt(InventoryItemRef itemRef) {
    sBubbleMgr.AddSpawnID(m_bubbleID, itemRef->itemID());
    m_system->GetBeltMgr()->RegisterBelt(itemRef);
    if (itemRef->typeID() == 17774) {
        m_type = Bubble::Type::Ice;
    } else {
        m_type = Bubble::Type::Belt;
    }
}

void SystemBubble::SetGate(uint32 gateID) {
    m_type = Bubble::Type::Gate;
    sBubbleMgr.AddSpawnID(m_bubbleID, gateID);
}

SystemEntity* const SystemBubble::GetEntity(uint32 entityID) const {
    /* updated to send ONLY dynamic entities to the following:          -allan 17Apr15
     *     ModuleManager::Activate()       --for module activation (with a target)
     */
    std::map<uint32, SystemEntity*>::const_iterator itr = m_dynamicEntities.find(entityID);
    if (itr != m_dynamicEntities.end())
        return itr->second;

    return nullptr;
}

void SystemBubble::AddNPC(NPC* pNPC) {
    m_npcs[pNPC->GetID()] = pNPC;
}

void SystemBubble::RemoveNPC(NPC* pNPC) {
    m_npcs.erase(pNPC->GetID());
}

void SystemBubble::AddActiveModule(ActiveModule* pMod) {
    m_activeModules[pMod->itemID()] = pMod;
}

void SystemBubble::RemoveActiveModule(ActiveModule* pMod) {
    m_activeModules.erase(pMod->itemID());
}

void SystemBubble::GetEntities(std::map<uint32, SystemEntity*> &into) const {
    /* updated to send non-cloaked dynamic entities to the following:         -allan 14Feb15
     *    SystemManager::MakeSetState()   --for player entering new system
     *    Command_killallnpcs()           --GM command
     *    StructureSE::InitData()         --Get TowerSE for pos items
     *    DroneAI::FindTarget()           --Drone AI target finding
     *    NPCAIMgr::EvaluateGridThreats() --advanced npc AI targeting
     */
    if (m_dynamicEntities.empty())
        return;

    for (auto &cur : m_dynamicEntities) {
        if (cur.second->DestinyMgr() != nullptr)
            if (cur.second->DestinyMgr()->IsCloaked())
                continue;
        into.emplace(cur.first, cur.second);
    }
}

void SystemBubble::GetAllEntities(std::map< uint32, SystemEntity* >& into) const {
    if (m_dynamicEntities.empty())
        return;

    into = m_dynamicEntities;
    //for (auto &cur : m_dynamicEntities)
    //    into.emplace(cur.first, cur.second);
}


void SystemBubble::GetEntityVec(std::vector< SystemEntity* >& into) const {
    if (m_players.empty())
        return;

    for (auto &cur : m_dynamicEntities)
        into.push_back(cur.second);
}

void SystemBubble::GetPlayers(std::vector<Client*> &into) const {
    /* updated to send ONLY players to the following:         -allan 14Feb15
     *    NPCAIMgr::Process()             --for npc targeting
     *    SpawnEntry::Process()           --for npc spawning
     */
    into.clear();
    if (m_players.empty())
        return;

    for (auto &cur : m_players)
        into.push_back(cur.second);
}

void SystemBubble::GetNPCs(std::vector<NPC*> &into) const {
    /* updated to send ONLY players to the following:         -allan 14Feb15
     *    NPCAIMgr::Process()             --for npc targeting
     *    SpawnEntry::Process()           --for npc spawning
     */
    into.clear();
    if (m_npcs.empty())
        return;

    for (auto &cur : m_npcs)
        into.push_back(cur.second);
}

void SystemBubble::GetDrones(std::vector<DroneSE*> &into) const {
    /* updated to send ONLY players to the following:         -allan 14Feb15
     *    NPCAIMgr::Process()             --for npc targeting
     *    SpawnEntry::Process()           --for npc spawning
     */
    into.clear();
    if (m_drones.empty())
        return;

    for (auto &cur : m_drones)
        into.push_back(cur.second);
}

SystemEntity* SystemBubble::GetRandomEntity() {
    // this is used for idle npc's as a orbit target while waiting for something to pewpew
    if (m_dynamicEntities.empty())
        return nullptr;

    for (auto &cur : m_dynamicEntities) {
        if (cur.second->IsWreckSE())
            return cur.second;
        if (cur.second->IsObjectEntity())
            return cur.second;
    }
    return nullptr;
}

uint32 SystemBubble::GetSystemID() {
    return m_system->GetID();
}

bool SystemBubble::InBubble(const GPoint& pt, bool inWarp/*false*/) const {
    return (m_center.distance(pt) < m_radius);
}

bool SystemBubble::IsOverlap(const GPoint& pt) const {
    return (m_center.distance(pt) < (m_radius * 2));
}

void SystemBubble::PrintEntityList() {
    if (m_entities.empty() and m_dynamicEntities.empty()) {
        sLog.Blue("SysBubble::PrintEntityList()", "Bubble %u in %s is empty", m_bubbleID, m_system->GetName());
        return;
    }

    bool found(false);
    std::vector<SystemEntity*> SElist;
    // load all entities visible in this bubble
    for (auto &cur : m_entities)
        SElist.push_back(cur.second);
    for (auto &cur : m_dynamicEntities)
        SElist.push_back(cur.second);

    for (auto &cur : SElist) {
        found = false;
        if (cur->isGlobal()) {
            if (cur->IsStaticEntity()) {
                sLog.Warning("SysBubble::PrintEntityList()", "entity %s(%u) is Global and Static.", cur->GetName(), cur->GetID());
                found = true;
            } else {
                //this should only hit beacons and cynos as global and not static
                sLog.Warning("SysBubble::PrintEntityList()", "entity %s(%u) is Global and not Static.", cur->GetName(), cur->GetID());
                found = true;
            }
        }
        if (cur->IsShipSE()) {
            if (cur->DestinyMgr()->IsCloaked()) {
                sLog.Warning("SysBubble::PrintEntityList()", "entity %s(%u) is Cloaked Ship.", cur->GetName(), cur->GetID()); found = true;
            } else if (cur->HasPilot()) {
                sLog.Warning("SysBubble::PrintEntityList()", "entity %s(%u) is Player Ship.", cur->GetName(), cur->GetID()); found = true;
            } else {
                sLog.Warning("SysBubble::PrintEntityList()", "entity %s(%u) is Empty Player Ship.", cur->GetName(), cur->GetID()); found = true;
            }
        }
        if (cur->IsNPCSE()) {
            sLog.Warning("SysBubble::PrintEntityList()", "entity %s(%u) is NPC.", cur->GetName(), cur->GetID()); found = true;
        }
        if (cur->IsJumpBridgeSE()) {
            sLog.Warning("SysBubble::PrintEntityList()", "entity %s(%u) is JumpBridge.", cur->GetName(), cur->GetID()); found = true;
        }
        if (cur->IsTCUSE()) {
            sLog.Warning("SysBubble::PrintEntityList()", "entity %s(%u) is TCU.", cur->GetName(), cur->GetID()); found = true;
        }
        if (cur->IsCOSE()) {
            sLog.Warning("SysBubble::PrintEntityList()", "entity %s(%u) is Customs Office.", cur->GetName(), cur->GetID()); found = true;
        }
        if (cur->IsSBUSE()) {
            sLog.Warning("SysBubble::PrintEntityList()", "entity %s(%u) is SBU.", cur->GetName(), cur->GetID()); found = true;
        }
        if (cur->IsTowerSE()) {
            sLog.Warning("SysBubble::PrintEntityList()", "entity %s(%u) is Tower.", cur->GetName(), cur->GetID()); found = true;
        }
        if (cur->IsPOSSE() and !found) {
            sLog.Warning("SysBubble::PrintEntityList()", "entity %s(%u) is other POS and !found.", cur->GetName(), cur->GetID()); found = true;
        }
        if (cur->IsContainerSE()) {
            sLog.Warning("SysBubble::PrintEntityList()", "entity %s(%u) is Container.", cur->GetName(), cur->GetID()); found = true;
        }
        if (cur->IsWreckSE()) {
            sLog.Warning("SysBubble::PrintEntityList()", "entity %s(%u) is Wreck.", cur->GetName(), cur->GetID()); found = true;
        }
        if (cur->IsOutpostSE()) {
            sLog.Warning("SysBubble::PrintEntityList()", "entity %s(%u) is Outpost.", cur->GetName(), cur->GetID()); found = true;
        }
        if (cur->IsAsteroidSE()) {
            sLog.Warning("SysBubble::PrintEntityList()", "entity %s(%u) is Asteroid.", cur->GetName(), cur->GetID()); found = true;
        }
        if (cur->IsDeployableSE()) {
            sLog.Warning("SysBubble::PrintEntityList()", "entity %s(%u) is Deployable.", cur->GetName(), cur->GetID()); found = true;
        }
        if (cur->IsStaticEntity() and !found) {
            sLog.Warning("SysBubble::PrintEntityList()", "entity %s(%u) is Static and !found.", cur->GetName(), cur->GetID()); found = true;
        }
        if (cur->IsItemEntity() and !found) {
            sLog.Warning("SysBubble::PrintEntityList()", "entity %s(%u) is Item and !found.", cur->GetName(), cur->GetID()); found = true;
        }
        if (cur->IsObjectEntity() and !found) {
            sLog.Warning("SysBubble::PrintEntityList()", "entity %s(%u) is Object and !found.", cur->GetName(), cur->GetID()); found = true;
        }
        if (cur->IsDynamicEntity() and !found) {
            sLog.Warning("SysBubble::PrintEntityList()", "entity %s(%u) is Dynamic and !found.", cur->GetName(), cur->GetID()); found = true;
        }
        if (!found)
            sLog.Warning("SysBubble::PrintEntityList()", "entity %s(%u) is None of the Above.", cur->GetName(), cur->GetID());
    }
}

void SystemBubble::SendAddBalls(SystemEntity* to_who) {
    if (!m_system->IsLoaded())
        return;
    if (m_dynamicEntities.empty())
        return;
    if (!to_who->HasPilot())
        return;
    Client* pClient = to_who->GetPilot();
    if (pClient == nullptr)
        return;
    if (is_log_enabled(BUBBLE__TRACE))
        PrintEntityList();

    Buffer* destinyBuffer = new Buffer();

    Destiny::AddBall_header head = Destiny::AddBall_header();
        head.packet_type = 1;   // 0 = full state   1 = balls
        head.stamp = sEntityMgr.GetStamp();
    destinyBuffer->Append(head);

    AddBalls addballs;
    addballs.slims = new PyList();

    for (auto &cur : m_dynamicEntities) {
        if (cur.second->DestinyMgr() != nullptr)
            if (cur.second->DestinyMgr()->IsCloaked())
                continue;
        if (!cur.second->IsMissileSE() or !cur.second->IsFieldSE())
            addballs.damageDict[cur.first] = cur.second->MakeDamageState();
        addballs.slims->AddItem(new PyObject("foo.SlimItem", cur.second->MakeSlimItem()));
        cur.second->EncodeDestiny(*destinyBuffer);
    }

    if (addballs.slims->empty()) {
        SafeDelete(destinyBuffer);
        return;
    }

    addballs.state = new PyBuffer(&destinyBuffer);

    _log(DESTINY__MESSAGE, "SysBubble::SendAddBalls() to %s", pClient->GetName());
    if (is_log_enabled(DESTINY__BALL_DUMP))
        addballs.Dump(DESTINY__BALL_DUMP, "    ");
    _log(DESTINY__BALL_DECODE, "    Ball Decoded:");
    if (is_log_enabled(DESTINY__BALL_DECODE))
        Destiny::DumpUpdate(DESTINY__BALL_DECODE, &(addballs.state->content())[0], (uint32)addballs.state->content().size());
    PyTuple* t = addballs.Encode();
    pClient->QueueDestinyUpdate(&t);    //consumed
}

void SystemBubble::SendAddBalls2(SystemEntity* to_who) {
    if (!m_system->IsLoaded())
        return;
    if (m_dynamicEntities.empty())
        return;
    if (!to_who->HasPilot())
        return;
    Client* pClient = to_who->GetPilot();
    if (pClient == nullptr)
        return;
    if (is_log_enabled(BUBBLE__TRACE))
        PrintEntityList();

    Buffer* destinyBuffer = new Buffer();

    Destiny::AddBall_header head = Destiny::AddBall_header();
        head.packet_type = 1;   // 0 = full state   1 = balls
        head.stamp = sEntityMgr.GetStamp();
    destinyBuffer->Append(head);

    AddBalls2 addballs2;
    addballs2.stateStamp = sEntityMgr.GetStamp();
    addballs2.extraBallData = new PyList();

    for (auto &cur : m_dynamicEntities) {
        if (cur.second->IsMissileSE() or cur.second->IsContainerSE()) {
            addballs2.extraBallData->AddItem(cur.second->MakeSlimItem());
        } else {
            PyTuple* balls = new PyTuple(2);
                balls->SetItem(0, cur.second->MakeSlimItem());
                balls->SetItem(1, cur.second->MakeDamageState());
            addballs2.extraBallData->AddItem(balls);
        }
        cur.second->EncodeDestiny(*destinyBuffer);
    }

    if (addballs2.extraBallData->size() < 1) {
        SafeDelete(destinyBuffer);
        return;
    }

    addballs2.state = new PyBuffer(&destinyBuffer); //consumed
    //SafeDelete(destinyBuffer);

    _log(DESTINY__MESSAGE, "SysBubble::SendAddBalls2() to %s", pClient->GetName());
    if (is_log_enabled(DESTINY__BALL_DUMP))
        addballs2.Dump(DESTINY__BALL_DUMP, "    ");
    //_log(DESTINY__TRACE, "    Ball Binary:");
    //_hex(DESTINY__TRACE, &(addballs2.state->content())[0], (uint32)addballs2.state->content().size());
    /*  note:  this shows up in valgrind as an uninitialized value   -allan 24Mar16
     * Conditional jump or move depends on uninitialised value(s)  SystemBubble.cpp:484 (uncorrected line#)
     * Uninitialised value was created by a heap allocation  SystemBubble.cpp:472
     */
    _log(DESTINY__BALL_DECODE, "    Ball Decoded:");
    if (is_log_enabled(DESTINY__BALL_DECODE))
        Destiny::DumpUpdate(DESTINY__BALL_DECODE, &(addballs2.state->content())[0], (uint32)addballs2.state->content().size());
    PyTuple* tmp = addballs2.Encode();
    pClient->QueueDestinyUpdate(&tmp, true);    //consumed
}

void SystemBubble::AddBallExclusive(SystemEntity* pSE) {
    if (!m_system->IsLoaded())
        return;
    if (pSE->DestinyMgr() != nullptr)
        if (pSE->DestinyMgr()->IsCloaked())
            return;

    Buffer* destinyBuffer = new Buffer();

    //create AddBalls header
    Destiny::AddBall_header head = Destiny::AddBall_header();
        head.packet_type = 1;   // 0 = full state   1 = balls
        head.stamp = sEntityMgr.GetStamp();
    destinyBuffer->Append(head);

    AddBalls addballs;
    //encode destiny binary
    pSE->EncodeDestiny(*destinyBuffer);
    addballs.state = new PyBuffer(&destinyBuffer);
	//encode damage state
    addballs.damageDict[ pSE->GetID() ] = pSE->MakeDamageState();
	//encode SlimItem
    addballs.slims = new PyList();
    addballs.slims->AddItem(new PyObject("foo.SlimItem", pSE->MakeSlimItem()));

    _log(BUBBLE__TRACE, "SysBubble::AddBallExclusive() - Adding entity %u to bubble %u", pSE->GetID(), m_bubbleID);
    if (is_log_enabled(DESTINY__BALL_DUMP))
        addballs.Dump(DESTINY__BALL_DUMP, "    ");
    _log(DESTINY__BALL_DECODE, "    Ball Decoded:");
    if (is_log_enabled(DESTINY__BALL_DECODE))
        Destiny::DumpUpdate(DESTINY__BALL_DECODE, &(addballs.state->content())[0], (uint32)addballs.state->content().size());
    //bubblecast the update
    PyTuple* t = addballs.Encode();
    BubblecastDestinyUpdateExclusive(&t, "AddBall", pSE);
    PySafeDecRef(t);
}

/*  NOTE   lil insight into clients code for RemoveBall
 * RemoveBall is function to remove all data associated with a particular ballID.
 * this call is only effective when a SlimItem for that ball is currently active in clients bubble,
 * and the ballID is > destiny.dstLocalBalls (which i dont know exactly what that is yet)
 * RemoveBalls is called when there is an associated TerminalExplosion with that ballID
 * [code]
 *          if funcName == 'RemoveBalls':
                exploders = [ x[1][1][0] for x in state if x[1][0] == 'TerminalExplosion' ]
 * [/code]
 * RemoveBalls is then called on the entire group, and will call RemoveBall(ballID, terminal) on each ball.
 *  the bool 'terminal' is initially false, then set to true if there is an associated TerminalExplosion for that ballID.
 *
 * see also DestinyManager::SendTerminalExplosion()
 *      NOTE  RemoveBall doesnt not work as i thought it should....doesnt trigger explosion.
 */
//TODO  update these based on above notes   (also look into better (non-ambigious) naming)
void SystemBubble::RemoveBall(SystemEntity *pSE) {
    if (!m_system->IsLoaded())
        return;
    RemoveBallsFromBP removeball;
    removeball.balls.push_back(pSE->GetID());

    _log(DESTINY__MESSAGE, "SysBubble::RemoveBall()");
    if (is_log_enabled(DESTINY__BALL_DUMP))
        removeball.Dump(DESTINY__BALL_DUMP, "    ");

    PyTuple *tmp = removeball.Encode();
    BubblecastDestinyUpdate(&tmp, "RemoveBall");
    PySafeDecRef(tmp);
}

// this *should* only be called from DestinyMgr::Cloak() and DestinyMgr::Jump()
void SystemBubble::RemoveBallExclusive(SystemEntity *pSE) {
    RemoveBallFromBP removeball;
        removeball.entityID = pSE->GetID();
    // RemoveBalls removeball;
    //removeball.balls.push_back(pSE->GetID());

    _log(DESTINY__MESSAGE, "SysBubble::RemoveBallExclusive()");
    if (is_log_enabled(DESTINY__BALL_DUMP))
        removeball.Dump(DESTINY__BALL_DUMP, "    ");

    PyTuple *tmp = removeball.Encode();
    BubblecastDestinyUpdateExclusive(&tmp, "RemoveBall", pSE);
    PySafeDecRef(tmp);
}

void SystemBubble::RemoveBalls(SystemEntity* pSE) {
    if (!m_system->IsLoaded())
        return;
    if (m_dynamicEntities.empty())
        return;
    if ((!pSE->HasPilot()) or (pSE->SysBubble() == nullptr))
        return;
    Client* pClient = pSE->GetPilot();
    if ((pClient == nullptr) or pClient->IsDock() or pClient->IsDocked())
        return;

    RemoveBallsFromBP remove_balls;

    for (auto &cur : m_dynamicEntities)
        remove_balls.balls.push_back(cur.first);

    if (remove_balls.balls.empty())
        return;

    _log(DESTINY__MESSAGE, "SysBubble::RemoveBalls() - sending to %s", pClient->GetName());
    if (is_log_enabled(DESTINY__BALL_DUMP))
        remove_balls.Dump(DESTINY__BALL_DUMP, "    ");

    PyTuple* tmp = remove_balls.Encode();
    pClient->QueueDestinyUpdate(&tmp);
}

PyObject* SystemBubble::GetDroneState() const {
    PyList* header = new PyList(7);
        header->SetItemString(0, "droneID");
        header->SetItemString(1, "ownerID");
        header->SetItemString(2, "controllerID");
        header->SetItemString(3, "activityState");
        header->SetItemString(4, "typeID");
        header->SetItemString(5, "controllerOwnerID");
        header->SetItemString(6, "targetID");
    PyList* lines = new PyList();
    for (auto &cur : m_drones) {
        PyList* line = new PyList(7);
            line->SetItem(0, new PyInt(cur.first));
            line->SetItem(1, new PyInt(cur.second->GetOwnerID()));
            line->SetItem(2, new PyInt(cur.second->GetControllerID()));
            line->SetItem(3, new PyInt(cur.second->GetState()));
            line->SetItem(4, new PyInt(cur.second->GetSelf()->typeID()));
            line->SetItem(5, new PyInt(cur.second->GetControllerOwnerID()));
            line->SetItem(6, new PyInt(cur.second->GetTargetID()));
        lines->AddItem(line);
    }

    PyDict* dict = new PyDict();
        dict->SetItemString("header", header);
        dict->SetItemString("RowClass", new PyToken("util.Row"));
        dict->SetItemString("lines", lines);

    return new PyObject("util.Rowset", dict);
}

void SystemBubble::SyncPos() {
    // send positions of all dSE in bubble to all players in bubble
    for (auto &player : m_players)
        for (auto &dse : m_dynamicEntities) {
            SetBallPosition du;
                du.entityID = dse.first;
                du.x = dse.second->GetPosition().x;
                du.y = dse.second->GetPosition().y;
                du.z = dse.second->GetPosition().z;
            PyTuple* up = du.Encode();
            player.second->GetShipSE()->DestinyMgr()->SendSingleDestinyUpdate(&up);
            PyDecRef(up);
        }
}

void SystemBubble::CmdDropLoot() {
    for (auto &cur : m_npcs)
            cur.second->CmdDropLoot();
}

void SystemBubble::RemoveMarkers() {
    if (m_hasMarkers) {
        SystemEntity* pSE(nullptr);
        for (auto &cur : m_markers) {
            pSE = cur.second;
            pSE->Delete();
            SafeDelete(pSE);
        }
    }
    m_markers.clear();
    m_centerSE = nullptr;
    m_hasMarkers = false;
}


void SystemBubble::MarkCenter()
{
    // we are not creating markers on system boot.
    if (!m_system->IsLoaded())
        return;
    if (m_hasMarkers)
        return;
    // create jetcan to mark bubble center
    std::string str = "Center Marker for Bubble #", desc = "Bubble Center";
    str += std::to_string(m_bubbleID);
    MarkBubble(m_center, str, desc, true);

    // create jetcan to mark bubble x
    GPoint center = m_center;
    center.x += BUBBLE_RADIUS_METERS - 5;
    str.clear();
    str = "Bubble #";
    str += std::to_string(m_bubbleID);
    str += " +X";
    desc = "Bubble x";
    MarkBubble(center, str, desc);

    // create jetcan to mark bubble -x
    center = m_center;
    center.x -= BUBBLE_RADIUS_METERS - 5;
    str.clear();
    str = "Bubble #";
    str += std::to_string(m_bubbleID);
    str += " -X";
    desc = "Bubble -x";
    MarkBubble(center, str, desc);

    // create jetcan to mark bubble y
    center = m_center;
    center.y += BUBBLE_RADIUS_METERS - 5;
    str.clear();
    str = "Bubble #";
    str += std::to_string(m_bubbleID);
    str += " +Y";
    desc = "Bubble y";
    MarkBubble(center, str, desc);

    // create jetcan to mark bubble -y
    center = m_center;
    center.y -= BUBBLE_RADIUS_METERS - 5;
    str.clear();
    str = "Bubble #";
    str += std::to_string(m_bubbleID);
    str +=  " -Y";
    desc = "Bubble -y";
    MarkBubble(center, str, desc);

    // create jetcan to mark bubble z
    center = m_center;
    center.z += BUBBLE_RADIUS_METERS - 5;
    str.clear();
    str = "Bubble #";
    str += std::to_string(m_bubbleID);
    str += " +Z";
    desc = "Bubble z";
    MarkBubble(center, str, desc);

    // create jetcan to mark bubble -z
    center = m_center;
    center.z -= BUBBLE_RADIUS_METERS - 5;
    str.clear();
    str = "Bubble #";
    str += std::to_string(m_bubbleID);
    str +=  " -Z";
    desc = "Bubble -z";
    MarkBubble(center, str, desc);

    m_hasMarkers = true;
}

void SystemBubble::MarkBubble(const GPoint& position, std::string& name, std::string& desc, bool center/*false*/)
{
    // create new container item
    ItemData idata(23, ownerSystem, m_system->GetID(), flagAutoFit, name.c_str(), position, desc.c_str());
    CargoContainerRef cRef = CargoContainerRef::StaticCast(InventoryItem::SpawnTemp(idata));
    if (cRef.get() == nullptr) {
        _log(DESTINY__WARNING, "MarkBubble() could not create Item for %s (%s)", name.c_str(), desc.c_str());
        return;
    }

    // create SE for item
    FactionData jetcanData = FactionData();
    ContainerSE* cSE = new ContainerSE(cRef, *(m_system->GetServiceMgr()), m_system, jetcanData);
    if (cSE == nullptr) {
        _log(DESTINY__WARNING, "MarkBubble() could not create SE for %s (%s)", name.c_str(), desc.c_str());
        return;
    }
    cRef->SetMySE(cSE);
    cSE->AnchorContainer();
    if (center) {
        // only setting centers as global
        cSE->SetGlobal(true);
        m_centerSE = cSE;
    }
    m_markers.emplace(cRef->itemID(), cSE);
    m_system->AddEntity(cSE, center);
}


void SystemBubble::BubblecastDestinyUpdate(std::vector<PyTuple *> &updates, const char *desc) const {
    for (std::vector<PyTuple *>::iterator cur = updates.begin(); cur != updates.end(); cur++)
        BubblecastDestinyUpdate(&(*cur), desc);

    updates.clear();
}

void SystemBubble::BubblecastDestinyEvent(std::vector<PyTuple *> &events, const char *desc) const {
    for (std::vector<PyTuple *>::iterator cur = events.begin(); cur != events.end(); cur++)
        BubblecastDestinyEvent(&(*cur), desc);

    events.clear();
}

void SystemBubble::BubblecastDestinyUpdate(PyTuple** payload, const char* desc) const
{
    if (is_log_enabled(BUBBLE__CAST_DUMP))
        (*payload)->Dump(BUBBLE__CAST_DUMP, "    ");
    for (auto &cur : m_players) {
        _log(BUBBLE__CAST, "Bubblecast %s update to %s(%u)", desc, cur.second->GetName(), cur.first);
        cur.second->QueueDestinyUpdate(payload);
    }
}

void SystemBubble::BubblecastDestinyUpdateExclusive(PyTuple** payload, const char* desc, SystemEntity* pSE) const
{
    for (auto &cur : m_players) {
        // Only queue a Destiny update for this bubble if the current SystemEntity is not 'pSE':
        // (this is an update to all client objects in the bubble EXCLUDING 'pSE')
        if (cur.second->GetShipSE() != pSE) {
            _log(BUBBLE__CAST, "Exclusive Bubblecast %s update to %s(%u)", desc, cur.second->GetName(), cur.first);
            cur.second->QueueDestinyUpdate(payload);
        }
    }
}

void SystemBubble::BubblecastDestinyEvent(PyTuple** payload, const char* desc) const
{
    if (is_log_enabled(BUBBLE__CAST_DUMP))
        (*payload)->Dump(BUBBLE__CAST_DUMP, "    ");
    for (auto &cur : m_players) {
        _log(BUBBLE__CAST, "Bubblecast %s event to %s(%u)", desc, cur.second->GetName(), cur.first);
        cur.second->QueueDestinyEvent(payload);
    }
}

void SystemBubble::BubblecastSendNotification(const char* notifyType, const char* idType, PyTuple** payload, bool seq)
{
    for (auto &cur : m_players) {
        _log(BUBBLE__CAST, "BubblecastNotify %s to %s(%u)", notifyType, cur.second->GetName(), cur.first);
        cur.second->SendNotification(notifyType, idType, payload, seq);
    }
}

/*
    switch (m_type) {
        case Bubble::Type::Normal: {
        } break;
        case Bubble::Type::Ice:
        case Bubble::Type::Belt:
        case Bubble::Type::Gate: {
        } break;
        case Bubble::Type::Anomaly:
        case Bubble::Type::Mission:
        case Bubble::Type::Incursion:
        case Bubble::Type::Escalation: {
        } break;
    }
*/