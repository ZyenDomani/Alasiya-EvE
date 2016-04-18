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
#include "Client.h"

uint32 SystemBubble::m_bubbleIncrementer = 0;

SystemBubble::SystemBubble(SystemManager* pSystem, const GPoint& center, double radius)
: m_system(pSystem),
m_center(center),
m_radius(radius),
m_radius_hysteresis(radius+BUBBLE_HYSTERESIS_METERS),
m_spawnTimer(sConfig.npc.RoamingTimer)
{
    m_spawned= false;
    m_spawnTimer.Disable();
    m_systemID = pSystem->GetID();
	m_bubbleID = m_bubbleIncrementer++;
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
		BubblecastDestinyEvent(&ev, desc); //update is consumed.
	}
	events.clear();
}

//send a destiny update to every client in the bubble.
//assume that static entities are not interested in destiny updates.
void SystemBubble::BubblecastDestinyUpdate( PyTuple** payload, const char* desc ) const
{
	PyTuple* up = *payload;
	*payload = nullptr;    //could optimize out one of the Clones in here...

	PyTuple* up_dup = nullptr;

    /*  may not need to update these.
     * this is a check to signal QueueDestinyUpdate() to use the PackagedAction header, and package all updates in a single tuple
    bool addball = false, removeball = false;
    if (desc == "AddBall") addball = true;
    if (desc == "RemoveBall") removeball = true;
    */
	for (auto cur : m_players) {
		if (!up_dup)
			up_dup = new PyTuple( *up );

		_log( DESTINY__BUBBLE_TRACE, "Bubblecast %s update to %s(%u)", desc, cur->GetName(), cur->GetID() );
		cur->QueueDestinyUpdate( &up_dup );
	}

	PySafeDecRef( up_dup );
	PySafeDecRef( up );
}

//send a destiny update to every client in the bubble EXCLUDING the given SystemEntity 'pEntity':
//assume that static entities are also not interested in destiny updates.
void SystemBubble::BubblecastDestinyUpdateExclusive( PyTuple** payload, const char* desc, SystemEntity* pEntity ) const
{
	PyTuple* up = *payload;
	*payload = nullptr;    //could optimize out one of the Clones in here...

	PyTuple* up_dup = nullptr;

    /*  may not need to update these.
     * this is a check to signal QueueDestinyUpdate() to use the PackagedAction header, and package all updates in a single tuple
    bool addball = false, removeball = false;
    if (desc == "AddBall") addball = true;
    if (desc == "RemoveBall") removeball = true;
    */
	for (auto cur : m_players) {
		// Only queue a Destiny update for this bubble if the current SystemEntity is not 'pEntity':
		// (this is an update to all SystemEntity objects in the bubble EXCLUDING 'pEntity')
		if( cur->GetID() != pEntity->GetID() ) {
			if (!up_dup)
				up_dup = new PyTuple( *up );

			cur->QueueDestinyUpdate( &up_dup );
			_log( DESTINY__BUBBLE_TRACE, "Exclusive Bubblecast %s update to %s(%u)", desc, cur->GetName(), cur->GetID() );
		}
	}

	PySafeDecRef( up_dup );
	PySafeDecRef( up );
}

//send a destiny event to every client in the bubble.
//assume that static entities are also not interested in destiny updates.
void SystemBubble::BubblecastDestinyEvent( PyTuple** payload, const char* desc ) const
{
	PyTuple* ev = *payload;
	*payload = nullptr;    //could optimize out one of the Clones in here...

	PyTuple* ev_dup = nullptr;

	for (auto cur : m_players) {
		if (!ev_dup)
			ev_dup = new PyTuple( *ev );

		cur->QueueDestinyEvent( &ev_dup/*, addball, removeball */ );
		_log( DESTINY__BUBBLE_TRACE, "Bubblecast %s event to %s(%u)", desc, cur->GetName(), cur->GetID() );
	}

	PySafeDecRef( ev_dup );
	PySafeDecRef( ev );
}

void SystemBubble::Process()
{
    if (m_spawnTimer.Enabled())
        if (m_spawnTimer.Check(false))
            if (HasPlayers())
                m_system->DoSpawnForBubble(this);
            else
                m_spawnTimer.Disable();
}

//called every 30s from the bubble manager.
//verifies that each entity is still in this bubble.
//if any entity is no longer in the bubble, they are removed
//from the bubble and stuck into the vector for re-classification.
void SystemBubble::ProcessWander(std::vector<SystemEntity*> &wanderers) {
	//the wanderers array may have other stuff in it, so use a local first.
	std::vector<SystemEntity*> found_wandering;
    found_wandering.clear();
	DynamicSystemEntity* pDSE(nullptr);
	for (auto cur : m_dynamicEntities) {
		pDSE = static_cast<DynamicSystemEntity*>(cur);
		if (!pDSE) continue;
		if (!InBubble(pDSE->GetPosition())) {
			//we cannot use Remove directly here because it will invalidate
			//our iterator, so store them away for now.
			found_wandering.push_back(cur);
			wanderers.push_back(cur);
		}
	}

	if (found_wandering.size() > 0)
		for (auto curw : found_wandering) {
			_log( DESTINY__BUBBLE_TRACE, "SystemBubble::ProcessWander() - entity %s(%u) found wandering. removing from bubble %u.",
			      curw->GetName(), curw->GetID(), GetID() );
			Remove(curw);
		}
}

void SystemBubble::Add(SystemEntity* pEntity, bool isPostWarp) {
	//if they are already in this bubble, do not continue.
	if (m_entities.find(pEntity->GetID()) != m_entities.end()) {
		_log(DESTINY__BUBBLE_TRACE, "SystemBubble::Add() - Tried to add entity %u to bubble %u, but it is already in here.",
		     pEntity->GetID(), GetID());
		return;
	}

	std::vector<SystemEntity*>::const_iterator cur = std::find(m_dynamicEntities.begin(), m_dynamicEntities.end(), pEntity);
	if (cur != m_dynamicEntities.end()) {
		_log(DESTINY__BUBBLE_TRACE, "SystemBubble::Add() - Tried to add entity %u to bubble %u, but it is already in here.",
		     pEntity->GetID(), GetID());
		return;
	}

	GPoint startPoint( pEntity->GetPosition() );
	GVector direction(startPoint, NULL_ORIGIN);
	double rangeToStar = direction.length();
	rangeToStar /= ONE_AU_IN_METERS;

	_log(DESTINY__BUBBLE_DEBUG, "SystemBubble::Add() - Adding entity %u at %.2f,%.2f,%.2f to bubble %u at %.2f,%.2f,%.2f. Distance to Star %.2f AU.  %u/%u Entities in bubble",
	     pEntity->GetID(), startPoint.x, startPoint.y, startPoint.z,
	     GetID(), m_center.x, m_center.y, m_center.z,
	     rangeToStar, m_entities.size(), m_dynamicEntities.size());

	pEntity->m_bubble = this;

	//insert the global entitys into their own list
	if (pEntity->IsStaticEntity()) {
		_log(DESTINY__BUBBLE_DEBUG, "SystemBubble::Add() - Entity %s(%u) is static.", pEntity->GetName(), pEntity->GetID() );
		m_entities.insert(std::pair<uint32, SystemEntity*>(pEntity->GetID(), pEntity));
		return;
	}

	// all non-global entities (players, npcs, roids, containers, etc) are put into bubble's dynamicEntity map
	m_dynamicEntities.push_back(pEntity);
	_log(DESTINY__BUBBLE_DEBUG, "SystemBubble::Add() - Entity %s(%u) is not static.", pEntity->GetName(), pEntity->GetID() );

	if (pEntity->IsClient()) {
		Client *pClient = pEntity->CastToClient();
		m_players.push_back(pClient);   //add to bubble's player list
		if (!pClient->IsUndock())
			_SendAddBalls(pClient);     //adds all entities in bubble to new player
        if (!pClient->Destiny()->IsCloaked()) {
            if (!m_players.empty()) {
                _BubblecastAddBallExclusive(pEntity);  // adds new player to all entities in bubble
                // Trigger SpawnManager for this bubble to generate NPC Spawn, if needed
                if (IsBelt() && (!IsSpawned()) && sConfig.npc.RoamingSpawns /*&& !pClient->IsLogin()*/) {
                    if (!m_spawnTimer.Enabled())
                        SetSpawnTimer(true);
                    //pClient->System()->DoSpawnForBubble(this);
                }
                if (IsGate() && (!IsSpawned()) && sConfig.npc.StaticSpawns) { /* IsGate returns false.  will fix when gate spawns are finished */
                    if (!m_spawnTimer.Enabled())
                        SetSpawnTimer(false);
                    //pClient->System()->DoSpawnForBubble(this);
                }
            }
        }
	} else {
		//if (!m_players.empty())
		_BubblecastAddBallExclusive(pEntity);
    }
}

void SystemBubble::Remove(SystemEntity *pEntity) {
	//assume that the entity is properly registered for its ID
	//  also assume static system entities will not be removed.
	if (!pEntity->m_bubble)
		return;     // Get outta here in case this was called again

    _log(DESTINY__BUBBLE_DEBUG, "SystemBubble::Remove() - Removing entity %u at %.2f,%.2f,%.2f from bubble %u at %.2f,%.2f,%.2f", \
        pEntity->GetID(), pEntity->GetPosition().x, pEntity->GetPosition().y, pEntity->GetPosition().z, \
        GetID(), m_center.x, m_center.y, m_center.z);

    m_dynamicEntities.erase(std::remove(m_dynamicEntities.begin(), m_dynamicEntities.end(), pEntity), m_dynamicEntities.end());

	if (pEntity->IsClient()) {
		m_players.erase(std::remove(m_players.begin(), m_players.end(), pEntity), m_players.end());
		_SendRemoveBalls(pEntity);
	}

	//regardless, notify everybody else in the bubble of the removal.
	//if (!m_players.empty())
	_BubblecastRemoveBall(pEntity);
	pEntity->m_bubble = nullptr;
	pEntity->TargMgr.ClearTargets();
}

void SystemBubble::AddExclusive(SystemEntity *pEntity) {
	_log(DESTINY__BUBBLE_DEBUG, "SystemBubble::AddExclusive() - Adding entity %u at %.2f,%.2f,%.2f to bubble %u at %.2f,%.2f,%.2f",
	     pEntity->GetID(), pEntity->GetPosition().x, pEntity->GetPosition().y, pEntity->GetPosition().z,
	     GetID(), m_center.x, m_center.y, m_center.z);
	_BubblecastAddBallExclusive(pEntity);
}

void SystemBubble::RemoveExclusive(SystemEntity *pEntity) {
	if (!pEntity->m_bubble)
		return;

	_log(DESTINY__BUBBLE_DEBUG, "SystemBubble::RemoveExclusive() - Removing entity %u at %.2f,%.2f,%.2f from bubble %u at %.2f,%.2f,%.2f",
	     pEntity->GetID(), pEntity->GetPosition().x, pEntity->GetPosition().y, pEntity->GetPosition().z,
	     GetID(), m_center.x, m_center.y, m_center.z, m_radius);
	_BubblecastRemoveBallExclusive(pEntity);
	pEntity->TargMgr.ClearTargets();
}

void SystemBubble::clear() {
	m_entities.clear();
	m_dynamicEntities.clear();
	m_players.clear();
}

void SystemBubble::SetSpawnTimer(bool isBelt /*false*/)
{
    if (sConfig.server.testServer)
        m_spawnTimer.Start(5000); /* 5s for testing */
    else
        if (isBelt)
            m_spawnTimer.Start(sConfig.npc.RoamingTimer *1000);
        else
            m_spawnTimer.Start(sConfig.npc.StaticTimer *1000);
}

/* i dont really need this here.... */
uint32 SystemBubble::GetSpawnID(uint16 bubbleID)
{
    return m_system->bubbles.GetSpawnID(bubbleID);
}

void SystemBubble::SetBelt(uint32 beltID)
{
    m_belt = true;
    m_system->bubbles.AddSpawnID(GetID(), beltID);
}

void SystemBubble::SetGate(uint32 gateID)
{
    m_gate = true;
    m_system->bubbles.AddSpawnID(GetID(), gateID);
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
	/* updated to send ONLY non-global entities to the following:         -allan 14Feb15
	 *    SystemManager::MakeSetState()   --for player entering new system
	 *    Command_killallnpcs()           --GM command
	 */
	if (m_dynamicEntities.empty()) return;

	std::vector<SystemEntity*>::const_iterator cur = m_dynamicEntities.begin();
	for (; cur != m_dynamicEntities.end(); cur++ )
		into.push_back((*cur));
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

uint32 SystemBubble::CountNPCs() {
	uint32 count = 0;
	for (auto cur : m_dynamicEntities) {
		if (cur->IsNPC())
			count++;
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

void SystemBubble::AppendBalls(SystemEntity* about_who) const
{

}


void SystemBubble::PrintEntityList() {
	bool found = false;
	for (auto cur : m_dynamicEntities) {
		if (cur->IsVisibleSystemWide())  //this is only set of entities NOT in m_dynamicEntities list.
			sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Global.", cur->GetName(), cur->GetID() );
		if (cur->IsStaticEntity())
			sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Static.", cur->GetName(), cur->GetID() ); found = true;
		if (cur->IsCelestial())
			sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Celestial.", cur->GetName(), cur->GetID() ); found = true;
		if (cur->IsNPC())
			sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is NPC.", cur->GetName(), cur->GetID() ); found = true;
		if (cur->IsClient())
			sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Client.", cur->GetName(), cur->GetID() ); found = true;
		if (cur->IsPOS())
			sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is POS.", cur->GetName(), cur->GetID() ); found = true;
		if (cur->IsJumpBridge())
			sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is JumpBridge.", cur->GetName(), cur->GetID() ); found = true;
		if (cur->IsTCU())
			sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is TCU.", cur->GetName(), cur->GetID() ); found = true;
		if (cur->IsContainer())
			sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Container.", cur->GetName(), cur->GetID() ); found = true;
		if (cur->IsWreck())
			sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Wreck.", cur->GetName(), cur->GetID() ); found = true;
		if (cur->IsOutpost())
			sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Outpost.", cur->GetName(), cur->GetID() ); found = true;
		if (cur->IsAsteroid())
			sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Asteroid.", cur->GetName(), cur->GetID() ); found = true;
		if (cur->IsShip())
			sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Ship.", cur->GetName(), cur->GetID() ); found = true;
		if (cur->IsDeployable())
			sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is Deployable.", cur->GetName(), cur->GetID() ); found = true;
		if (!found)
			sLog.Warning( "SystemBubble::_PrintEntityList()", "entity %s(%u) is None of the Above.", cur->GetName(), cur->GetID() );
	}
}

void SystemBubble::_SendAddBalls( SystemEntity* to_who ) {
	if (m_dynamicEntities.empty()) return;

	PrintEntityList();

	Buffer* destinyBuffer = new Buffer;

	Destiny::AddBall_header head;
	head.packet_type = 1;   // 0 = full state   1 = balls
	head.sequence = sEntityList.GetStamp();

	destinyBuffer->Append( head );

	DoDestiny_AddBalls addballs;
	addballs.slims = new PyList;

	for (auto cur : m_dynamicEntities) {
		if (cur == to_who)
			continue;
		//damageState
		if (!cur->IsMissile())
			addballs.damages[ cur->GetID() ] = cur->MakeDamageState();
		addballs.slims->AddItem( new PyObject( "foo.SlimItem", cur->MakeSlimItem() ) );
		//append the destiny binary data...
		cur->EncodeDestiny( *destinyBuffer );
	}

	if (addballs.slims->size() < 1)
		return;

	addballs.destiny_binary = new PyBuffer( &destinyBuffer );
	SafeDelete( destinyBuffer );

	_log( DESTINY__DEBUG, "SystemBubble::_SendAddBalls():" );
	addballs.Dump( DESTINY__DEBUG, "    " );
	//_log( DESTINY__TRACE, "    Ball Binary:" );
    //_hex( DESTINY__TRACE, &( addballs.destiny_binary->content() )[0], (uint32)addballs.destiny_binary->content().size() );
	_log( DESTINY__DEBUG, "    Ball Decoded:" );
	Destiny::DumpUpdate( DESTINY__TRACE, &( addballs.destiny_binary->content() )[0],
			     (uint32)addballs.destiny_binary->content().size() );

	PyTuple* t = addballs.Encode();
	to_who->QueueDestinyUpdate( &t );    //may consume, but may not.
	PySafeDecRef( t );
}

void SystemBubble::_SendRemoveBalls( SystemEntity* to_who ) {
	if (m_dynamicEntities.empty()) return;

	DoDestiny_RemoveBalls remove_balls;

	for (auto cur : m_dynamicEntities) {
		remove_balls.balls.push_back(cur->GetID());
	}

	if (remove_balls.balls.empty())
		return;

	_log( DESTINY__DEBUG, "SystemBubble::_SendRemoveBalls():" );
	remove_balls.Dump( DESTINY__DEBUG, "    " );

	PyTuple* tmp = remove_balls.Encode();
	to_who->QueueDestinyUpdate( &tmp );    //may consume, but may not.
	PySafeDecRef( tmp );
}

//  this isnt right....
/*  TODO FIXME     need to update AddBalls to AddBalls2
 *
 *    use the following format for building the packet
 *
 *
 *  list is in DoDestiny_AddBalls2 packet xmlp.  it contains list of tuples of each item being added
                                  [PyList 16 items]
                                    [PyTuple 2 items]               <<<  tuple contains 2 items...dict and list
                                      [PyDict 10 kvp]               <<< dict of item values...will have to write different ones
                                        [PyString "itemID"]
                                        [PyIntegerVar 1002332964652]
                                        [PyString "typeID"]
                                        [PyInt 17771]
                                        [PyString "incapacitated"]
                                        [PyFloat 0]
                                        [PyString "posTimestamp"]
                                        [PyNone]
                                        [PyString "posState"]
                                        [PyInt 1]
                                        [PyString "warFactionID"]
                                        [PyNone]
                                        [PyString "allianceID"]
                                        [PyNone]
                                        [PyString "corpID"]
                                        [PyInt 98038978]
                                        [PyString "ownerID"]
                                        [PyInt 98038978]
                                        [PyString "controlTowerID"]
                                        [PyIntegerVar 1002332856217]
                                      [PyList 3 items]                <<<  list of damageState
                                        [PyTuple 3 items]
                                          [PyFloat 1]
                                          [PyFloat 1000000]
                                          [PyIntegerVar 129527508117147467]
                                        [PyFloat 1]
                                        [PyFloat 1]
 *
 */
void SystemBubble::_BubblecastAddBall( SystemEntity* about_who ) {
	if (m_players.empty()) return;

	Buffer* destinyBuffer = new Buffer;

	//create AddBalls header
	Destiny::AddBall_header head;
        head.packet_type = 1;   // 0 = full state   1 = balls
        head.sequence = sEntityList.GetStamp();
	destinyBuffer->Append( head );

	DoDestiny_AddBalls addballs;
        addballs.slims = new PyList;

	//encode destiny binary
	about_who->EncodeDestiny( *destinyBuffer );
	addballs.destiny_binary = new PyBuffer( &destinyBuffer );
	SafeDelete( destinyBuffer );
/*  TODO this needs more work...
    PyTuple* ballState = new PyTuple(2);
        ballState->SetItem(0, about_who->MakeSlimItem());
        ballState->SetItem(1, about_who->MakeDamageStateList());
    */
	//encode damage state
	addballs.damages[ about_who->GetID() ] = about_who->MakeDamageState();
	//encode SlimItem
	addballs.slims->AddItem( new PyObject( "foo.SlimItem", about_who->MakeSlimItem() ) );

	//bubblecast the update
	PyTuple* t = addballs.Encode();
	BubblecastDestinyUpdate( &t, "AddBall" );
	PySafeDecRef( t );
}

void SystemBubble::_BubblecastAddBallExclusive( SystemEntity* about_who ) {
	Buffer* destinyBuffer = new Buffer;

	//create AddBalls header
	Destiny::AddBall_header head;
        head.packet_type = 1;   // 0 = full state   1 = balls
        head.sequence = sEntityList.GetStamp();
	destinyBuffer->Append( head );

	DoDestiny_AddBalls addballs;
        addballs.slims = new PyList;

	//encode destiny binary
	about_who->EncodeDestiny( *destinyBuffer );
        addballs.destiny_binary = new PyBuffer( &destinyBuffer );
	SafeDelete( destinyBuffer );

	//encode damage state
        addballs.damages[ about_who->GetID() ] = about_who->MakeDamageState();
	//encode SlimItem
        addballs.slims->AddItem( new PyObject( "foo.SlimItem", about_who->MakeSlimItem() ) );

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
void SystemBubble::_BubblecastRemoveBall(SystemEntity *about_who) {
	//DoDestiny_RemoveBall removeball;
    //    removeball.entityID = about_who->GetID();
    // using RemoveBalls instead of RemoveBall because client
    // seems not to trigger explosion on RemoveBall
    DoDestiny_RemoveBalls removeball;
    removeball.balls.push_back(about_who->GetID());
/*
    PyTuple* paTuple = new PyTuple(1);
        paTuple->SetItem(0, removeball.Encode());

    PackagedAction pa;
        pa.substream = new PySubStream(paTuple);
*/
	_log(DESTINY__DEBUG, "SystemBubble::_BubblecastRemoveBall():");
    //paTuple->Dump(DESTINY__DEBUG, "    ");

    PyTuple *tmp = removeball.Encode();
    //PyTuple *tmp = pa.Encode();
	BubblecastDestinyUpdate(&tmp, "RemoveBall");    //consumed
	PySafeDecRef( tmp );
}


void SystemBubble::_BubblecastRemoveBallExclusive(SystemEntity *about_who) {
    //DoDestiny_RemoveBall removeball;
    //    removeball.entityID = about_who->GetID();
    // using RemoveBalls instead of RemoveBall because client
    // seems not to trigger explosion on RemoveBall
    DoDestiny_RemoveBalls removeball;
    removeball.balls.push_back(about_who->GetID());
/*
    PyTuple* paTuple = new PyTuple(1);
        paTuple->SetItem(0, removeball.Encode());

    PackagedAction pa;
        pa.substream = new PySubStream(paTuple);
*/
    _log(DESTINY__DEBUG, "SystemBubble::_BubblecastRemoveBallExclusive():");
    //paTuple->Dump(DESTINY__DEBUG, "    ");

    PyTuple *tmp = removeball.Encode();
    //PyTuple *tmp = pa.Encode();
	BubblecastDestinyUpdateExclusive(&tmp, "RemoveBall", about_who);    //consumed
	PySafeDecRef( tmp );
}
