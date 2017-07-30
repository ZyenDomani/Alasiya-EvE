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
 *    Author:        Zhur
 *    Updates:        Allan
 */

#include <algorithm>

#include "eve-server.h"
#include "EVEServerConfig.h"

#include "Client.h"
#include "EntityList.h"
#include "system/BubbleManager.h"
#include "system/DestinyManager.h"
#include "system/SystemBubble.h"
#include "system/SystemEntity.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/BeltMgr.h"

uint32 SystemBubble::m_bubbleIncrementer = 0;

SystemBubble::SystemBubble(SystemManager* pSystem, const GPoint& center, double radius)
: m_system(pSystem),
m_center(center),
m_radius(radius),
m_radius_hysteresis(radius+BUBBLE_HYSTERESIS_METERS),
m_spawnTimer(sConfig.npc.RoamingTimer)
{
    clear();
    m_spawnTimer.Disable();
    m_systemID = pSystem->GetID();
	m_bubbleID = sBubbleMgr.GetBubbleID();
	_log(DESTINY__BUBBLE_TRACE, "SystemBubble::Constructor - Created new bubble %u(%p) at (%.2f,%.2f,%.2f).",\
	     m_bubbleID, this, m_center.x, m_center.y, m_center.z, m_radius);
}

SystemBubble::~SystemBubble()
{
    clear();
}

void SystemBubble::clear() {
    m_ice = false;
    m_belt = false;
    m_gate = false;
    m_spawned = false;
    m_players.clear();
    m_entities.clear();
    m_dynamicEntities.clear();
}

void SystemBubble::Process()
{
    if (m_system->GetSystemSecurityRating() > 0.90)
        return;
    if (m_spawned){
        m_spawnTimer.Disable();
        return;
    }

    // this must run a second time for spawn to actually hit.  first time only sets main system spawn timer.
    // may be nuts, but will remain enabled as long as player in bubble and bubble has no rats.
    if (m_spawnTimer.Enabled())
        if (m_spawnTimer.Check())
            if (HasPlayers()) {
                m_system->DoSpawnForBubble(this);
            } else {
                m_spawnTimer.Disable();
            }
}

//called every 30s from the bubble manager.
//verifies that each entity is still in this bubble.
//if any entity is no longer in the bubble, they are removed
//from the bubble and stuck into the vector for re-classification.
void SystemBubble::ProcessWander(std::vector<SystemEntity*> &wanderers) {
	DynamicSystemEntity* pDSE(nullptr);
    std::map<uint32, SystemEntity*>::iterator itr = m_dynamicEntities.begin();
    while (itr != m_dynamicEntities.end()) {
        if (!itr->second) {
            ++itr;
            continue;
        }
        pDSE = itr->second->GetDynamicSE();
        if (!pDSE) {
            ++itr;
            continue;
        }
        if (pDSE->DestinyMgr() and pDSE->DestinyMgr()->IsWarping()) {
            ++itr;
            continue;
        }
        if (pDSE->SystemMgr()->GetID() != m_systemID) {
            // this entity is in a different system!  this shouldnt happen....
            _log(DESTINY__WARNING, "SystemBubble::ProcessWander() - entity %u is in %u but this is %u.", \
                                pDSE->GetID(), pDSE->SystemMgr()->GetID(), m_systemID);
            // make sure we're still a valid iterator, then remove this entity, insert into wanderers, and continue
            if (itr != m_dynamicEntities.end()) {
                wanderers.push_back(itr->second);
                itr = m_dynamicEntities.erase(itr);
            }
            pDSE = nullptr;
            continue;
        }
		if (!InBubble(pDSE->GetPosition())) {
            wanderers.push_back(itr->second);
            if (pDSE->m_bubble != this) {
                //17:38:57 [DestinyWarning] SystemBubble::ProcessWander() - entity 140006173(sys:30002507) not in bubble 1 for systemID 30002510.
                _log(DESTINY__WARNING, "SystemBubble::ProcessWander() - entity %u(sys:%u) not in bubble %u for systemID %u.", \
                        pDSE->GetID(), pDSE->SystemMgr()->GetID(), m_bubbleID, m_systemID);
                if (itr != m_dynamicEntities.end()) {
                    itr = m_dynamicEntities.erase(itr);
                    pDSE = nullptr;
                    continue;
                }
            }
        }
        ++itr;
	}
    pDSE = nullptr;
}

void SystemBubble::Add(SystemEntity* pSE) {
	//if they are already in this bubble, do not continue.
	if (m_entities.find(pSE->GetID()) != m_entities.end()) {
        _log(DESTINY__BUBBLE_TRACE, "SystemBubble::Add() - Tried to add Static Entity %u to bubble %u, but it is already in here.",\
		     pSE->GetID(), GetID());
		return;
	}

	if (m_dynamicEntities.find(pSE->GetID()) != m_dynamicEntities.end()) {
        _log(DESTINY__BUBBLE_TRACE, "SystemBubble::Add() - Tried to add Dynamic Entity %u to bubble %u, but it is already in here.",\
		     pSE->GetID(), GetID());
		return;
	}

	if (is_log_enabled(DESTINY__BUBBLE_DEBUG)) {
        GPoint startPoint( pSE->GetPosition() );
        GVector direction(startPoint, NULL_ORIGIN);
        double rangeToStar = direction.length();
        rangeToStar /= ONE_AU_IN_METERS;
        _log(DESTINY__BUBBLE_TRACE, "SystemBubble::Add() - Adding entity %u to bubble %u. Distance to Star %.2f AU.  %u/%u Entities in bubble",\
                pSE->GetID(), GetID(), rangeToStar, m_entities.size(), m_dynamicEntities.size());
        if (sConfig.server.StackTrace)
            EvE::traceStack();
    }

	pSE->m_bubble = this;

	//insert the global entitys into their own list
	if (pSE->IsStaticEntity()) {
        _log(DESTINY__BUBBLE_TRACE, "SystemBubble::Add() - Entity %s(%u) is static.", pSE->GetName(), pSE->GetID() );
		m_entities[pSE->GetID()] = pSE;
		return;
	}

    if (pSE->HasPilot()) {
        Client* pClient = pSE->GetPilot();
        SendAddBalls( pSE );
        if (!pClient->IsJump()) {
            if (HasPlayers())
                AddBallExclusive(pSE);  // adds new player to all players in bubble, if any
        }
        m_players[pClient->GetCharacterID()] = pClient;   //add to bubble's player list

        // Set spawn timer for this bubble, if needed
        if (m_belt) {
            // check for roids and load/spawn as needed.
            m_system->GetBeltMgr()->CheckSpawn(m_bubbleID);
            if (sConfig.npc.RoamingSpawns)
                if (!m_spawnTimer.Enabled())
                    SetSpawnTimer(true);
        }
        if (m_gate and sConfig.npc.StaticSpawns) /* m_gate = false.  will fix when gate spawns are finished */
            if (!m_spawnTimer.Enabled())
                SetSpawnTimer(false);
    } else {
        if (HasPlayers())
            AddBallExclusive(pSE);
    }

    // all non-global entities (players, npcs, roids, containers, etc) are put into bubble's dynamicEntity map
    m_dynamicEntities[pSE->GetID()] = pSE;
}

void SystemBubble::Remove(SystemEntity *pSE) {
	//assume that the entity is properly registered for its ID
	if (!pSE->m_bubble)
		return;

    _log(DESTINY__BUBBLE_TRACE, "SystemBubble::Remove() - Removing entity %u from bubble %u", pSE->GetID(), GetID());

    std::map<uint32, SystemEntity*>::iterator itr = m_entities.find(pSE->GetID());
    if (itr != m_entities.end())
        m_entities.erase(itr);

    itr = m_dynamicEntities.find(pSE->GetID());
    if (itr != m_dynamicEntities.end())
        m_dynamicEntities.erase(itr);

    if (pSE->HasPilot()) {
        std::map<uint32, Client*>::iterator itr = m_players.find(pSE->GetPilot()->GetCharacterID());
        if (itr != m_players.end())
            m_players.erase(itr);
        RemoveBalls(pSE);
    }

    //notify everybody else in the bubble of the removal.
    if (!m_players.empty())
        RemoveBall(pSE);

    if (is_log_enabled(DESTINY__BUBBLE_DEBUG)) {
        sLog.Warning("SystemBubble::Remove()", "Removing entity %u from bubble %u", pSE->GetID(), GetID());
    }
    if (sConfig.server.StackTrace)
        EvE::traceStack();
    pSE->m_bubble = nullptr;
}

void SystemBubble::RemoveExclusive(SystemEntity *pSE) {
	if (!pSE->m_bubble)
		return;

    _log(DESTINY__BUBBLE_TRACE, "SystemBubble::RemoveExclusive() - Removing entity %u from bubble %u", pSE->GetID(), GetID());
	RemoveBallExclusive(pSE);
}

void SystemBubble::ResetBubbleRatSpawn()
{
    /* the current spawn in this bubble was killed off, so reset timers accordingly
     *   once the timer hits, it will do all needed checks for players and respawn as needed.
     *  this enables creating a new spawn after previous group was killed off
     */
    m_spawned = false;
    if (m_belt and sConfig.npc.RoamingSpawns)
        if (!m_spawnTimer.Enabled())
            SetSpawnTimer(true);
    if (m_gate and sConfig.npc.StaticSpawns) /* m_gate = false.  will fix when gate spawns are finished */
        if (!m_spawnTimer.Enabled())
            SetSpawnTimer(false);
}

void SystemBubble::SetSpawnTimer(bool isBelt/*false*/)
{
    if (m_system->GetSystemSecurityRating() > 0.90) return;
    if (sConfig.server.IsTestServer and sConfig.npc.SpawnTest) {
        m_spawnTimer.Start(5000); /* 5s for testing */
    } else {
        if (isBelt)
            m_spawnTimer.Start(sConfig.npc.RoamingTimer *60 *1000);
        else
            m_spawnTimer.Start(sConfig.npc.StaticTimer *60 *1000);
    }
}

void SystemBubble::SetBelt(InventoryItemRef itemRef)
{
    m_belt = true;
    sBubbleMgr.AddSpawnID(m_bubbleID, itemRef->itemID());
    m_system->GetBeltMgr()->RegisterBelt(itemRef);
    if (itemRef->typeID() == 17774)
        m_ice = true;
}

void SystemBubble::SetGate(uint32 gateID)
{
    m_gate = true;
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

void SystemBubble::GetEntities(std::vector<SystemEntity*> &into) const {
    /* updated to send ONLY dynamic entities to the following:         -allan 14Feb15
     *    SystemManager::MakeSetState()   --for player entering new system
     *    Command_killallnpcs()           --GM command
     *    StructureSE::InitData()         --Get TowerSE for pos items
     */
    if (m_dynamicEntities.empty())
        return;

    for (auto cur : m_dynamicEntities)
        into.push_back(cur.second);
}

void SystemBubble::GetPlayers(std::vector<Client*> &into) const {
	/* updated to send ONLY players to the following:         -allan 14Feb15
	 *    NPCAIMgr::Process()             --for npc targeting
	 *    SpawnEntry::Process()           --for npc spawning
     *
     * this will also send player drones once that system is completed
	 */
	into.clear();
	if (m_players.empty()) return;

	for (auto cur : m_players)
		into.push_back(cur.second);
}

SystemEntity* SystemBubble::GetRandomEntity()
{
    // this is used for idle npc's as a orbit target while waiting for something to pewpew
    if (m_dynamicEntities.empty())
        return nullptr;

    for (auto cur : m_dynamicEntities) {
        if (cur.second->IsWreckSE())
            return cur.second;
        if (cur.second->IsObjectEntity())
            return cur.second;
        return nullptr;
    }
}

uint32 SystemBubble::CountNPCs() {
    uint32 count = 0;
    for (auto cur : m_dynamicEntities)
        if (cur.second->IsNPCSE())
            ++count;

    return count;
}

bool SystemBubble::InBubble(const GPoint& pt) const
{
	if (m_center.distance(pt) <= m_radius_hysteresis)
		return true;
	return false;
}

void SystemBubble::PrintEntityList() {
    bool found = false;
    for (auto cur : m_dynamicEntities) {
        if (cur.second->isGlobal())  //this should only hit beacons and cynos as global AND not static
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Global.", cur.second->GetName(), cur.second->GetID() );
        if (cur.second->IsShipSE())
            if (cur.second->HasPilot()) {
                sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Player.", cur.second->GetName(), cur.second->GetID() ); found = true;
            } else {
                sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Ship.", cur.second->GetName(), cur.second->GetID() ); found = true;
            }
        if (cur.second->IsNPCSE())
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is NPC.", cur.second->GetName(), cur.second->GetID() ); found = true;
        if (cur.second->IsJumpBridgeSE())
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is JumpBridge.", cur.second->GetName(), cur.second->GetID() ); found = true;
        if (cur.second->IsTCUSE())
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is TCU.", cur.second->GetName(), cur.second->GetID() ); found = true;
        if (cur.second->IsCOSE())
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Customs Office.", cur.second->GetName(), cur.second->GetID() ); found = true;
        if (cur.second->IsSBUSE())
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is SBU.", cur.second->GetName(), cur.second->GetID() ); found = true;
        if (cur.second->IsPOSSE())
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is POS.", cur.second->GetName(), cur.second->GetID() ); found = true;
        if (cur.second->IsContainerSE())
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Container.", cur.second->GetName(), cur.second->GetID() ); found = true;
        if (cur.second->IsWreckSE())
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Wreck.", cur.second->GetName(), cur.second->GetID() ); found = true;
        if (cur.second->IsOutpostSE())
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Outpost.", cur.second->GetName(), cur.second->GetID() ); found = true;
        if (cur.second->IsAsteroidSE())
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Asteroid.", cur.second->GetName(), cur.second->GetID() ); found = true;
        if (cur.second->IsDeployableSE())
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Deployable.", cur.second->GetName(), cur.second->GetID() ); found = true;
        if (cur.second->IsStaticEntity() and !found)
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Static.", cur.second->GetName(), cur.second->GetID() ); found = true;
        if (cur.second->IsItemEntity() and !found)
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Item.", cur.second->GetName(), cur.second->GetID() ); found = true;
        if (cur.second->IsObjectEntity() and !found)
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Object.", cur.second->GetName(), cur.second->GetID() ); found = true;
        if (cur.second->IsDynamicEntity() and !found)
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Dynamic.", cur.second->GetName(), cur.second->GetID() ); found = true;
        if (!found)
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is None of the Above.", cur.second->GetName(), cur.second->GetID() );
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
    if (!pClient)
        return;
    if (is_log_enabled(DESTINY__TRACE))
        PrintEntityList();

    Buffer* destinyBuffer = new Buffer;

    Destiny::AddBall_header head;
        head.packet_type = 1;   // 0 = full state   1 = balls
        head.eventStamp = sEntityList.GetStamp();
    destinyBuffer->Append(head);

    DoDestiny_AddBalls addballs;
    addballs.slims = new PyList;

    for (auto cur : m_dynamicEntities) {
        if (!cur.second->IsMissileSE())
            addballs.damageDict[cur.first] = cur.second->MakeDamageState();
        addballs.slims->AddItem( new PyObject( "foo.SlimItem", cur.second->MakeSlimItem() ) );
        cur.second->EncodeDestiny( *destinyBuffer );
    }

    //addballs.slims->AddItem( new PyObject( "foo.SlimItem", to_who->MakeSlimItem() ) );

    if (addballs.slims->size() < 1) {
        SafeDelete( destinyBuffer );
        return;
    }

    addballs.state = new PyBuffer( &destinyBuffer );

    _log(DESTINY__MESSAGE, "SystemBubble::SendAddBalls() to %s", pClient->GetName());
    if (is_log_enabled(DESTINY__BALL_DUMP))
        addballs.Dump( DESTINY__BALL_DUMP, "    " );
    _log( DESTINY__BALL_DECODE, "    Ball Decoded:" );
    if (is_log_enabled(DESTINY__BALL_DECODE))
        Destiny::DumpUpdate( DESTINY__BALL_DECODE, &( addballs.state->content() )[0], (uint32)addballs.state->content().size() );
    PyTuple* t = addballs.Encode();
    pClient->QueueDestinyUpdate( &t );    //may consume, may not.
    PySafeDecRef( t );
}

void SystemBubble::SendAddBalls2( SystemEntity* to_who ) {
    if (!m_system->IsLoaded())
        return;
    if (m_dynamicEntities.empty())
        return;
    if (!to_who->HasPilot())
        return;
    Client* pClient = to_who->GetPilot();
    if (!pClient)
        return;
    if (is_log_enabled(DESTINY__TRACE))
        PrintEntityList();

	Buffer* destinyBuffer = new Buffer;

	Destiny::AddBall_header head;
        head.packet_type = 1;   // 0 = full state   1 = balls
        head.eventStamp = sEntityList.GetStamp();
	destinyBuffer->Append(head);

    DoDestiny_AddBalls2 addballs2;
        addballs2.stateStamp = sEntityList.GetStamp();
        addballs2.extraBallData = new PyList();

    for (auto cur : m_dynamicEntities) {
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
        SafeDelete( destinyBuffer );
        return;
    }

    addballs2.state = new PyBuffer(&destinyBuffer); //consumed
    SafeDelete( destinyBuffer );

    _log( DESTINY__MESSAGE, "SystemBubble::SendAddBalls2() to %s", pClient->GetName());
    if (is_log_enabled(DESTINY__BALL_DUMP))
        addballs2.Dump( DESTINY__BALL_DUMP, "    " );
    //_log( DESTINY__TRACE, "    Ball Binary:" );
    //_hex( DESTINY__TRACE, &( addballs2.state->content() )[0], (uint32)addballs2.state->content().size() );
    /*  note:  this shows up in valgrind as an uninitialized value   -allan 24Mar16
     * Conditional jump or move depends on uninitialised value(s)  SystemBubble.cpp:484 (uncorrected line#)
     * Uninitialised value was created by a heap allocation  SystemBubble.cpp:472
     */
    _log( DESTINY__BALL_DECODE, "    Ball Decoded:" );
    if (is_log_enabled(DESTINY__BALL_DECODE))
        Destiny::DumpUpdate( DESTINY__BALL_DECODE, &( addballs2.state->content() )[0], (uint32)addballs2.state->content().size() );
    PyTuple* t = addballs2.Encode();
    pClient->QueueDestinyUpdate(&t, true);    //consumed
}

void SystemBubble::AddBallExclusive( SystemEntity* about_who ) {
    if (!m_system->IsLoaded())
        return;
	Buffer* destinyBuffer = new Buffer;

	//create AddBalls header
	Destiny::AddBall_header head;
        head.packet_type = 1;   // 0 = full state   1 = balls
        head.eventStamp = sEntityList.GetStamp();
	destinyBuffer->Append( head );

	DoDestiny_AddBalls addballs;
        addballs.slims = new PyList;

	//encode destiny binary
	about_who->EncodeDestiny( *destinyBuffer );
        addballs.state = new PyBuffer( &destinyBuffer );
	//encode damage state
        addballs.damageDict[ about_who->GetID() ] = about_who->MakeDamageState();
	//encode SlimItem
        addballs.slims->AddItem( new PyObject( "foo.SlimItem", about_who->MakeSlimItem() ) );

    _log(DESTINY__BUBBLE_TRACE, "SystemBubble::AddBallExclusive() - Adding entity %u to bubble %u", about_who->GetID(), GetID());
    if (is_log_enabled(DESTINY__BALL_DUMP))
        addballs.Dump( DESTINY__BALL_DUMP, "    " );
    _log( DESTINY__BALL_DECODE, "    Ball Decoded:" );
    if (is_log_enabled(DESTINY__BALL_DECODE))
        Destiny::DumpUpdate( DESTINY__BALL_DECODE, &( addballs.state->content() )[0], (uint32)addballs.state->content().size() );
	//bubblecast the update
	PyTuple* t = addballs.Encode();
	BubblecastDestinyUpdateExclusive( &t, "AddBall", about_who );
	PySafeDecRef( t );
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
 *      NOTE  RemoveBall doesnt not work as i thought it should....doesnt trigger explosion.
 */
//TODO  update these based on above notes   (also look into better (non-ambigious) naming)
void SystemBubble::RemoveBall(SystemEntity *about_who) {
	//DoDestiny_RemoveBall removeball;
    //    removeball.entityID = about_who->GetID();
    // using RemoveBalls instead of RemoveBall because client
    // seems not to trigger explosion on RemoveBall
    if (!m_system->IsLoaded())
        return;
    DoDestiny_RemoveBalls removeball;
    removeball.balls.push_back(about_who->GetID());

    _log(DESTINY__MESSAGE, "SystemBubble::RemoveBall()");
    if (is_log_enabled(DESTINY__BALL_DUMP))
        removeball.Dump( DESTINY__BALL_DUMP, "    " );

    PyTuple *tmp = removeball.Encode();
	BubblecastDestinyUpdate(&tmp, "RemoveBall");    //consumed
	PySafeDecRef( tmp );
}


// this *should* only be called from DestinyManager::Cloak()
void SystemBubble::RemoveBallExclusive(SystemEntity *about_who) {
    DoDestiny_RemoveBall removeball;
        removeball.entityID = about_who->GetID();
    //DoDestiny_RemoveBalls removeball;
    //removeball.balls.push_back(about_who->GetID());

    _log(DESTINY__MESSAGE, "SystemBubble::RemoveBallExclusive()");
    if (is_log_enabled(DESTINY__BALL_DUMP))
        removeball.Dump( DESTINY__BALL_DUMP, "    " );

    PyTuple *tmp = removeball.Encode();
	BubblecastDestinyUpdateExclusive(&tmp, "RemoveBall", about_who);    //consumed
	PySafeDecRef( tmp );
}

void SystemBubble::RemoveBalls( SystemEntity* to_who ) {
    if (!m_system->IsLoaded())
        return;
    if (m_dynamicEntities.empty())
        return;
    if ((!to_who->HasPilot()) or (!to_who->SysBubble()))
        return;
    Client* pClient = to_who->GetPilot();
    if (!pClient or pClient->IsDock() or pClient->IsDocked())
        return;

    DoDestiny_RemoveBalls remove_balls;

    for (auto cur : m_dynamicEntities)
        remove_balls.balls.push_back(cur.first);

    if (remove_balls.balls.empty())
        return;

    _log( DESTINY__MESSAGE, "SystemBubble::RemoveBalls() - sending to %s", pClient->GetName());
    if (is_log_enabled(DESTINY__BALL_DUMP))
        remove_balls.Dump( DESTINY__BALL_DUMP, "    " );

    PyTuple* tmp = remove_balls.Encode();
    pClient->QueueDestinyUpdate( &tmp );    //may consume, but may not.
    PySafeDecRef( tmp );
}

//send a set of destiny events and updates to every client in the bubble.
void SystemBubble::BubblecastDestiny(std::vector<PyTuple *> &updates, std::vector<PyTuple *> &events, const char *desc) const {
    BubblecastDestinyUpdate(updates, desc);
    BubblecastDestinyEvent(events, desc);
}

//send a set of destiny updates to every client in the bubble.
void SystemBubble::BubblecastDestinyUpdate(std::vector<PyTuple *> &updates, const char *desc) const {
    std::vector<PyTuple *>::iterator cur = updates.begin();
    for (; cur != updates.end(); cur++) {
        PyTuple *up = *cur;
        BubblecastDestinyUpdate(&up, desc); //update is consumed.
    }
    updates.clear();
}

//send a set of destiny events to every client in the bubble.
void SystemBubble::BubblecastDestinyEvent(std::vector<PyTuple *> &events, const char *desc) const {
    std::vector<PyTuple *>::iterator cur = events.begin();
    for (; cur != events.end(); cur++) {
        PyTuple *ev = *cur;
        BubblecastDestinyEvent(&ev, desc); //event is consumed.
    }
    events.clear();
}

//send a destiny update to every client in the bubble.
void SystemBubble::BubblecastDestinyUpdate( PyTuple** payload, const char* desc ) const
{
    PyTuple* up = *payload;
    *payload = nullptr;    //could optimize out one of the Clones in here...

    PyTuple* up_dup(nullptr);

    for (auto cur : m_players) {
        if (!up_dup)
            up_dup = new PyTuple( *up );
        _log( DESTINY__BUBBLECAST, "Bubblecast %s update to %s(%u)", desc, cur.second->GetName(), cur.first );
        if (is_log_enabled(DESTINY__BUBBLECAST_DUMP))
            up_dup->Dump(DESTINY__BUBBLECAST_DUMP, "    ");
        cur.second->QueueDestinyUpdate( &up_dup );
    }

    PySafeDecRef( up_dup );
    PyDecRef( up );
}

//send a destiny update to every client in the bubble EXCLUDING the given SystemEntity 'pSE':
void SystemBubble::BubblecastDestinyUpdateExclusive( PyTuple** payload, const char* desc, SystemEntity* pSE ) const
{
    PyTuple* up = *payload;
    *payload = nullptr;    //could optimize out one of the Clones in here...

    PyTuple* up_dup(nullptr);

    for (auto cur : m_players) {
        // Only queue a Destiny update for this bubble if the current SystemEntity is not 'pSE':
        // (this is an update to all client objects in the bubble EXCLUDING 'pSE')
        if (cur.second->GetShipSE() != pSE) {
            if (!up_dup)
                up_dup = new PyTuple( *up );
            _log( DESTINY__BUBBLECAST, "Exclusive Bubblecast %s update to %s(%u)", desc, cur.second->GetName(), cur.first );
            if (is_log_enabled(DESTINY__BUBBLECAST_DUMP))
                up_dup->Dump(DESTINY__BUBBLECAST_DUMP, "    ");
            cur.second->QueueDestinyUpdate( &up_dup );
        }
    }

    PySafeDecRef( up_dup );
    PyDecRef( up );
}

//send a destiny event to every client in the bubble.
void SystemBubble::BubblecastDestinyEvent( PyTuple** payload, const char* desc ) const
{
    PyTuple* ev = *payload;
    *payload = nullptr;    //could optimize out one of the Clones in here...

    PyTuple* ev_dup(nullptr);

    for (auto cur : m_players) {
        if (!ev_dup)
            ev_dup = new PyTuple( *ev );
        _log( DESTINY__BUBBLECAST, "Bubblecast %s event to %s(%u)", desc, cur.second->GetName(), cur.first );
        if (is_log_enabled(DESTINY__BUBBLECAST_DUMP))
            ev_dup->Dump(DESTINY__BUBBLECAST_DUMP, "    ");
        cur.second->QueueDestinyEvent( &ev_dup );
    }

    PySafeDecRef( ev_dup );
    PyDecRef( ev );
}

void SystemBubble::BubblecastSendNotification(const char* notifyType, const char* idType, PyTuple** payload, bool seq)
{
    PyTuple* ev = *payload;
    *payload = nullptr;    //could optimize out one of the Clones in here...

    PyTuple* ev_dup(nullptr);

    for (auto cur : m_players) {
        if (!ev_dup)
            ev_dup = new PyTuple( *ev );
        _log( DESTINY__BUBBLECAST, "BubblecastNotify %s to %s(%u)", notifyType, cur.second->GetName(), cur.first );
        if (is_log_enabled(DESTINY__BUBBLECAST_DUMP))
            ev_dup->Dump(DESTINY__BUBBLECAST_DUMP, "    ");
        cur.second->SendNotification( notifyType, idType, &ev_dup, seq );
    }
}
