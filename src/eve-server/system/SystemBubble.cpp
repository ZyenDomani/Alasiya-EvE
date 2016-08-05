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
#include "ship/DestinyManager.h"
#include "system/BubbleManager.h"
#include "system/SystemBubble.h"
#include "system/SystemEntity.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/BeltMgr.h"
#include "Client.h"

uint32 SystemBubble::m_bubbleIncrementer = 0;

SystemBubble::SystemBubble(SystemManager* pSystem, const GPoint& center, double radius)
: m_system(pSystem),
m_center(center),
m_radius(radius),
m_radius_hysteresis(radius+BUBBLE_HYSTERESIS_METERS),
m_spawnTimer(sConfig.npc.RoamingTimer)
{
    m_belt = false;
    m_gate = false;
    m_spawned= false;
    m_spawnTimer.Disable();
    m_systemID = pSystem->GetID();
	m_bubbleID = sBubbleMgr.GetBubbleID();
	_log(DESTINY__BUBBLE_DEBUG, "SystemBubble::Constructor - Created new bubble %u(%p) at (%.2f,%.2f,%.2f).",
	     m_bubbleID, this, m_center.x, m_center.y, m_center.z, m_radius);
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
        _log( DESTINY__BUBBLECAST, "Bubblecast %s update to %s(%u)", desc, cur->GetName(), cur->GetCharacterID() );
        if (is_log_enabled(DESTINY__BUBBLECAST_DUMP))
            up_dup->Dump(DESTINY__BUBBLECAST_DUMP, "    ");
		cur->QueueDestinyUpdate( &up_dup );
	}

	PySafeDecRef( up_dup );
	PyDecRef( up );
}

//send a destiny update to every client in the bubble EXCLUDING the given SystemEntity 'pEntity':
void SystemBubble::BubblecastDestinyUpdateExclusive( PyTuple** payload, const char* desc, SystemEntity* pEntity ) const
{
	PyTuple* up = *payload;
	*payload = nullptr;    //could optimize out one of the Clones in here...

	PyTuple* up_dup(nullptr);

	for (auto cur : m_players) {
		// Only queue a Destiny update for this bubble if the current SystemEntity is not 'pEntity':
		// (this is an update to all client objects in the bubble EXCLUDING 'pEntity')
        if( cur->GetShipSE()->GetID() != pEntity->GetID() ) {
			if (!up_dup)
                up_dup = new PyTuple( *up );
            _log( DESTINY__BUBBLECAST, "Exclusive Bubblecast %s update to %s(%u)", desc, cur->GetName(), cur->GetCharacterID() );
            if (is_log_enabled(DESTINY__BUBBLECAST_DUMP))
                up_dup->Dump(DESTINY__BUBBLECAST_DUMP, "    ");
			cur->QueueDestinyUpdate( &up_dup );
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
        _log( DESTINY__BUBBLECAST, "Bubblecast %s event to %s(%u)", desc, cur->GetName(), cur->GetCharacterID() );
        if (is_log_enabled(DESTINY__BUBBLECAST_DUMP))
            ev_dup->Dump(DESTINY__BUBBLECAST_DUMP, "    ");
		cur->QueueDestinyEvent( &ev_dup );
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
        _log( DESTINY__BUBBLECAST, "BubblecastNotify %s to %s(%u)", notifyType, cur->GetName(), cur->GetCharacterID() );
        if (is_log_enabled(DESTINY__BUBBLECAST_DUMP))
            ev_dup->Dump(DESTINY__BUBBLECAST_DUMP, "    ");
        cur->SendNotification( notifyType, idType, &ev_dup, seq );
    }
}

void SystemBubble::Process()
{
    if (m_system->GetSystemSecurityRating() > 0.90)
        return;
    if (m_spawned){
        m_spawnTimer.Disable();
        return;
    }

    if (m_spawnTimer.Enabled())
        if (m_spawnTimer.Check()) {
            if (HasPlayers()) {
                m_system->DoSpawnForBubble(this);
            } else {
                m_spawnTimer.Disable();
            }
        }
}

//called every 30s from the bubble manager.
//verifies that each entity is still in this bubble.
//if any entity is no longer in the bubble, they are removed
//from the bubble and stuck into the vector for re-classification.
void SystemBubble::ProcessWander(std::vector<SystemEntity*> &wanderers) {
	DynamicSystemEntity* pDSE(nullptr);
    for (auto cur : m_dynamicEntities) {
        pDSE = cur->GetDynamicSE();
		if (!pDSE) continue;
        if (pDSE->DestinyMgr() and pDSE->DestinyMgr()->IsWarping()) continue;
		if (!InBubble(pDSE->GetPosition()))
			wanderers.push_back(cur);
	}
    pDSE = nullptr;
}

void SystemBubble::Add(SystemEntity* pEntity) {
	//if they are already in this bubble, do not continue.
	if (m_entities.find(pEntity->GetID()) != m_entities.end()) {
        _log(DESTINY__BUBBLE_DEBUG, "SystemBubble::Add() - Tried to add entity %u to bubble %u, but it is already in here.",
		     pEntity->GetID(), GetID());
		return;
	}

	std::vector<SystemEntity*>::const_iterator cur = std::find(m_dynamicEntities.begin(), m_dynamicEntities.end(), pEntity);
	if (cur != m_dynamicEntities.end()) {
        _log(DESTINY__BUBBLE_DEBUG, "SystemBubble::Add() - Tried to add entity %u to bubble %u, but it is already in here.",
		     pEntity->GetID(), GetID());
		return;
	}

	if (is_log_enabled(DESTINY__BUBBLE_DEBUG)) {
        GPoint startPoint( pEntity->GetPosition() );
        GVector direction(startPoint, NULL_ORIGIN);
        double rangeToStar = direction.length();
        rangeToStar /= ONE_AU_IN_METERS;
        _log(DESTINY__BUBBLE_DEBUG, "SystemBubble::Add() - Adding entity %u to bubble %u. Distance to Star %.2f AU.  %u/%u Entities in bubble",
                pEntity->GetID(), GetID(), rangeToStar, m_entities.size(), m_dynamicEntities.size());
    }

	pEntity->m_bubble = this;

	//insert the global entitys into their own list
	if (pEntity->IsStaticEntity()) {
        _log(DESTINY__BUBBLE_DEBUG, "SystemBubble::Add() - Entity %s(%u) is static.", pEntity->GetName(), pEntity->GetID() );
		m_entities.insert(std::pair<uint32, SystemEntity*>(pEntity->GetID(), pEntity));
		return;
	}

    if (pEntity->HasPilot()) {
        Client* pClient = pEntity->GetPilot();
        SendAddBalls( pEntity );
        if (!pClient->GetShipSE()->DestinyMgr()->IsCloaked()) {
            if (HasPlayers())
                AddBallExclusive(pEntity);  // adds new player to all players in bubble, if any
        }
        m_players.push_back( pClient );   //add to bubble's player list

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
            AddBallExclusive(pEntity);
    }

    // all non-global entities (players, npcs, roids, containers, etc) are put into bubble's dynamicEntity map
    m_dynamicEntities.push_back(pEntity);
}

void SystemBubble::Remove(SystemEntity *pEntity) {
	//assume that the entity is properly registered for its ID
	//  also assume static system entities will not be removed.
	if (!pEntity->m_bubble)
		return;     // Get outta here in case this was called again

    _log(DESTINY__BUBBLE_DEBUG, "SystemBubble::Remove() - Removing entity %u from bubble %u", pEntity->GetID(), GetID());

    m_dynamicEntities.erase(std::remove(m_dynamicEntities.begin(), m_dynamicEntities.end(), pEntity), m_dynamicEntities.end());

    if (pEntity->HasPilot()) {
        m_players.erase(std::remove(m_players.begin(), m_players.end(), pEntity->GetPilot()), m_players.end());
        RemoveBalls(pEntity);
    }

    //regardless, notify everybody else in the bubble of the removal.
    if (!m_players.empty())
        RemoveBall(pEntity);
    pEntity->m_bubble = nullptr;
}

void SystemBubble::RemoveExclusive(SystemEntity *pEntity) {
	if (!pEntity->m_bubble)
		return;

    _log(DESTINY__BUBBLE_DEBUG, "SystemBubble::RemoveExclusive() - Removing entity %u from bubble %u", pEntity->GetID(), GetID());
	RemoveBallExclusive(pEntity);
}

void SystemBubble::clear() {
	m_entities.clear();
	m_dynamicEntities.clear();
	m_players.clear();
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
    if (sConfig.server.testServer)
        m_spawnTimer.Start(5000); /* 5s for testing */
    else {
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
	std::vector<SystemEntity*>::const_iterator cur = m_dynamicEntities.begin();
	for (; cur != m_dynamicEntities.end(); cur++ )
		if ((*cur)->GetID() == entityID)
			return (*cur);

		return nullptr;
}

void SystemBubble::GetEntities(std::vector<SystemEntity*> &into) const {
    /* updated to send ONLY dynamic entities to the following:         -allan 14Feb15
     *    SystemManager::MakeSetState()   --for player entering new system
     *    Command_killallnpcs()           --GM command
     */
    if (m_dynamicEntities.empty()) return;

    for (auto cur : m_dynamicEntities)
        into.push_back(cur);
}

void SystemBubble::GetPlayers(std::vector<Client*> &into) const {
	/* updated to send ONLY players to the following:         -allan 14Feb15
	 *    NPCAIMgr::Process()             --for npc targeting
	 *    SpawnEntry::Process()           --for npc spawning
	 */
	into.clear();
	if (m_players.empty()) return;

	for (auto cur : m_players)
		into.push_back(cur);
}

SystemEntity* SystemBubble::GetRandomEntity()
{
    if (m_dynamicEntities.empty())
        return nullptr;

    for (auto cur : m_dynamicEntities) {
        if (cur->IsWreckSE())
            return cur;
        if (cur->IsObjectEntity())
            return cur;
        return nullptr;
    }
}

uint32 SystemBubble::CountNPCs() {
    uint32 count = 0;
    for (auto cur : m_dynamicEntities) {
        if (cur->IsNPCSE())
            ++count;
    }

    return count;
}

bool SystemBubble::InBubble(const GPoint& pt) const
{
	// Return true when System Entity is still within BUBBLE_RADIUS_METERS + BUBBLE_HYSTERESIS_METERS from the center
	GVector distance(m_center, pt);
	if (distance.length() < m_radius_hysteresis)
		return true;
	return false;
}

void SystemBubble::PrintEntityList() {
    bool found = false;
    for (auto cur : m_dynamicEntities) {
        if (cur->IsVisibleSystemWide())  //this is only set on entities NOT in m_dynamicEntities list. (this should only hit beacons and cynos)
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Global.", cur->GetName(), cur->GetID() );
        if (cur->IsShipSE())
            if (cur->HasPilot()) {
                sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Player.", cur->GetName(), cur->GetID() ); found = true;
            } else {
                sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Ship.", cur->GetName(), cur->GetID() ); found = true;
            }
        if (cur->IsNPCSE())
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is NPC.", cur->GetName(), cur->GetID() ); found = true;
        if (cur->IsPOSSE())
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is POS.", cur->GetName(), cur->GetID() ); found = true;
        if (cur->IsJumpBridgeSE())
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is JumpBridge.", cur->GetName(), cur->GetID() ); found = true;
        if (cur->IsTCUSE())
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is TCU.", cur->GetName(), cur->GetID() ); found = true;
        if (cur->IsContainerSE())
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Container.", cur->GetName(), cur->GetID() ); found = true;
        if (cur->IsWreckSE())
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Wreck.", cur->GetName(), cur->GetID() ); found = true;
        if (cur->IsOutpostSE())
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Outpost.", cur->GetName(), cur->GetID() ); found = true;
        if (cur->IsAsteroidSE())
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Asteroid.", cur->GetName(), cur->GetID() ); found = true;
        if (cur->IsDeployableSE())
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Deployable.", cur->GetName(), cur->GetID() ); found = true;
        if (cur->IsStaticEntity() and !found)
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Static.", cur->GetName(), cur->GetID() ); found = true;
        if (cur->IsItemEntity() and !found)
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Item.", cur->GetName(), cur->GetID() ); found = true;
        if (cur->IsObjectEntity() and !found)
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Object.", cur->GetName(), cur->GetID() ); found = true;
        if (cur->IsDynamicEntity() and !found)
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Dynamic.", cur->GetName(), cur->GetID() ); found = true;
        if (!found)
            sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is None of the Above.", cur->GetName(), cur->GetID() );
    }
}

void SystemBubble::SendAddBalls(SystemEntity* to_who) {
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
        if (!cur->IsMissileSE())
            addballs.damageDict[ cur->GetID() ] = cur->MakeDamageState();
        addballs.slims->AddItem( new PyObject( "foo.SlimItem", cur->MakeSlimItem() ) );
        cur->EncodeDestiny( *destinyBuffer );
    }

    //addballs.slims->AddItem( new PyObject( "foo.SlimItem", to_who->MakeSlimItem() ) );

    if (addballs.slims->size() < 1) {
        SafeDelete( destinyBuffer );
        return;
    }

    addballs.state = new PyBuffer( &destinyBuffer );

    _log(DESTINY__MESSAGE, "SystemBubble::SendAddBalls() to %s", pClient->GetName());
    addballs.Dump( DESTINY__BALL_DUMP, "    " );
    _log( DESTINY__BALL_DECODE, "    Ball Decoded:" );
    Destiny::DumpUpdate( DESTINY__BALL_DECODE, &( addballs.state->content() )[0], (uint32)addballs.state->content().size() );
    PyTuple* t = addballs.Encode();
    pClient->QueueDestinyUpdate( &t );    //may consume, may not.
    PySafeDecRef( t );
}

void SystemBubble::SendAddBalls2( SystemEntity* to_who ) {
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
        if (cur->IsMissileSE() or cur->IsContainerSE()) {
            addballs2.extraBallData->AddItem(cur->MakeSlimItem());
        } else {
            PyTuple* balls = new PyTuple(2);
                balls->SetItem(0, cur->MakeSlimItem());
                balls->SetItem(1, cur->MakeDamageState());
            addballs2.extraBallData->AddItem(balls);
        }
        cur->EncodeDestiny(*destinyBuffer);
    }

    if (addballs2.extraBallData->size() < 1) {
        SafeDelete( destinyBuffer );
        return;
    }

    addballs2.state = new PyBuffer(&destinyBuffer); //consumed
    SafeDelete( destinyBuffer );

    _log( DESTINY__MESSAGE, "SystemBubble::SendAddBalls2() to %s", pClient->GetName());
    addballs2.Dump( DESTINY__BALL_DUMP, "    " );
    //_log( DESTINY__TRACE, "    Ball Binary:" );
    //_hex( DESTINY__TRACE, &( addballs2.state->content() )[0], (uint32)addballs2.state->content().size() );
    /*  note:  this shows up in valgrind as an uninitialized value   -allan 24Mar16
     * Conditional jump or move depends on uninitialised value(s)  SystemBubble.cpp:484 (uncorrected line#)
     * Uninitialised value was created by a heap allocation  SystemBubble.cpp:472
     */
    _log( DESTINY__BALL_DECODE, "    Ball Decoded:" );
    Destiny::DumpUpdate( DESTINY__BALL_DECODE, &( addballs2.state->content() )[0], (uint32)addballs2.state->content().size() );
    PyTuple* t = addballs2.Encode();
    pClient->QueueDestinyUpdate(&t, true);    //consumed
}

void SystemBubble::AddBallExclusive( SystemEntity* about_who ) {
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

    _log(DESTINY__BUBBLE_DEBUG, "SystemBubble::AddBallExclusive() - Adding entity %u to bubble %u", about_who->GetID(), GetID());
    addballs.Dump( DESTINY__BALL_DUMP, "    " );
    _log( DESTINY__BALL_DECODE, "    Ball Decoded:" );
    Destiny::DumpUpdate( DESTINY__BALL_DECODE, &( addballs.state->content() )[0], (uint32)addballs.state->content().size() );
	//bubblecast the update
	PyTuple* t = addballs.Encode();
	BubblecastDestinyUpdateExclusive( &t, "AddBall", about_who );
	PySafeDecRef( t );
}

/*  NOTE   lil insight into clients code for RemoveBall
 * RemoveBall is function to remove all data associated with a particular ballID.
 * this call is only effective when a SlimItem for that ball is currently active in clients bubble,
 * and the ballID is > destiny.dstLocalBalls
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
//TODO  update these based on above notes
void SystemBubble::RemoveBall(SystemEntity *about_who) {
	//DoDestiny_RemoveBall removeball;
    //    removeball.entityID = about_who->GetID();
    // using RemoveBalls instead of RemoveBall because client
    // seems not to trigger explosion on RemoveBall
    DoDestiny_RemoveBalls removeball;
    removeball.balls.push_back(about_who->GetID());

    _log(DESTINY__MESSAGE, "SystemBubble::RemoveBall()");
    removeball.Dump( DESTINY__BALL_DUMP, "    " );

    PyTuple *tmp = removeball.Encode();
	BubblecastDestinyUpdate(&tmp, "RemoveBall");    //consumed
	PySafeDecRef( tmp );
}


void SystemBubble::RemoveBallExclusive(SystemEntity *about_who) {
    //DoDestiny_RemoveBall removeball;
    //    removeball.entityID = about_who->GetID();
    // using RemoveBalls instead of RemoveBall because client
    // seems not to trigger explosion on RemoveBall
    DoDestiny_RemoveBalls removeball;
    removeball.balls.push_back(about_who->GetID());

    _log(DESTINY__MESSAGE, "SystemBubble::RemoveBallExclusive()");
    removeball.Dump( DESTINY__BALL_DUMP, "    " );

    PyTuple *tmp = removeball.Encode();
	BubblecastDestinyUpdateExclusive(&tmp, "RemoveBall", about_who);    //consumed
	PySafeDecRef( tmp );
}

void SystemBubble::RemoveBalls( SystemEntity* to_who ) {
    if (!to_who->HasPilot())
        return;
    if (m_dynamicEntities.empty())
        return;
    Client* pClient = to_who->GetPilot();
    if (!pClient)
        return;
    if (m_dynamicEntities.empty()) return;

    DoDestiny_RemoveBalls remove_balls;

    for (auto cur : m_dynamicEntities) {
        remove_balls.balls.push_back(cur->GetID());
    }

    if (remove_balls.balls.empty())
        return;

    _log( DESTINY__MESSAGE, "SystemBubble::RemoveBalls() to %s", pClient->GetName());
    remove_balls.Dump( DESTINY__BALL_DUMP, "    " );

    PyTuple* tmp = remove_balls.Encode();
    pClient->QueueDestinyUpdate( &tmp );    //may consume, but may not.
    PySafeDecRef( tmp );
}
