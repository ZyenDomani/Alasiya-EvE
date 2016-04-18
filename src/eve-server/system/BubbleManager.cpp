/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Zhur
*/

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "system/BubbleManager.h"
#include "system/SystemBubble.h"
#include "system/SystemEntity.h"
#include "system/SystemManager.h"
#include "Client.h"

BubbleManager::BubbleManager()
: m_wanderTimer(30000)
{
    m_wanderTimer.Start();
    m_wanderers.clear();
}

BubbleManager::~BubbleManager() {
    clear();
}

void BubbleManager::clear() {
    for (auto cur : m_bubbles)
        SafeDelete(cur);

    m_bubbles.clear();
}

void BubbleManager::Process() {
    double profileStartTime = 0.0;
    if (sConfig.server.UseProfiling)
        profileStartTime = GetTimeUSeconds();
    // process each bubble
    for (auto cur : m_bubbles) {
        if (cur->IsBelt() or cur->IsGate())
            cur->Process();
    }
    // run wander check every 30 sec for active bubbles
    if (m_wanderTimer.Check()) {
        m_wanderers.clear();
        std::vector<SystemBubble *>::iterator itr = m_bubbles.begin();
        for (; itr != m_bubbles.end(); itr++) {
            if ((*itr)->IsEmpty() and (!(*itr)->IsSpawned())) {
                // Remove this bubble now that it is empty of ALL dynamic entities
                _log(DESTINY__BUBBLE_DEBUG, "BubbleManager::Process() - Bubble %u is empty and is being deleted from the system.", (*itr)->GetID() );
                itr = m_bubbles.erase(itr);
            } else
                (*itr)->ProcessWander(m_wanderers);
        }

        if (!m_wanderers.empty())
            for (auto cur : m_wanderers) {
                _log(DESTINY__WARNING, "BubbleManager::Process() - Wanderer '%s' being added to a bubble.", cur->GetName() );
                Add(cur);
            }
    }
    if (sConfig.server.UseProfiling)
        sProfile.AddTime(_bubblesProfile, GetTimeUSeconds() - profileStartTime);
}

void BubbleManager::CheckBubble(SystemEntity *ent, bool isWarping, bool isPostWarp) {
    SystemBubble *b = ent->Bubble();
    if (b) {
        if ((b->InBubble(ent->GetPosition())) && (!isWarping)) {
            _log(DESTINY__BUBBLE_TRACE, "BubbleManager::CheckBubble() - Entity '%s'(%u) at (%.2f,%.2f,%.2f) is still located in bubble %u at %.2f,%.2f,%.2f.",
                 ent->GetName(), ent->GetID(), ent->GetPosition().x, ent->GetPosition().y, ent->GetPosition().z,
                 b->GetID(), b->m_center.x, b->m_center.y, b->m_center.z);
            //still in bubble...
            sLog.Debug( "BubbleManager::CheckBubble()", "SystemEntity '%s' is still located in Bubble %u",
                        ent->GetName(), b->GetID() );
            return;
        } else if (isWarping) {
            //entity is in warp, therefore, no bubble needed.
            _log(DESTINY__BUBBLE_TRACE, "BubbleManager::CheckBubble() - Warping Entity '%s'(%u) is no longer located in bubble %u.  Removing...",
                 ent->GetName(), ent->GetID(), b->GetID());
            b->Remove(ent);
            return;
        }

        _log(DESTINY__BUBBLE_TRACE, "BubbleManager::CheckBubble() - Entity '%s'(%u) at (%.2f,%.2f,%.2f) is no longer located in bubble %u at %.2f,%.2f,%.2f.  Removing...",
             ent->GetName(), ent->GetID(), ent->GetPosition().x, ent->GetPosition().y, ent->GetPosition().z,
             b->GetID(), b->m_center.x, b->m_center.y, b->m_center.z);
        b->Remove(ent);
    }

    _log(DESTINY__BUBBLE_TRACE, "BubbleManager::CheckBubble() - SystemEntity '%s'(%u) not currently in any Bubble...adding",
         ent->GetName(), ent->GetID() );
    Add(ent, isPostWarp);
}

void BubbleManager::Add(SystemEntity* ent, bool isPostWarp) {
    DynamicSystemEntity* pDSE = static_cast<DynamicSystemEntity*>(ent);
    if (!pDSE) return;
    const GPoint &pos(pDSE->GetPosition());
    if (pos.isZero()) ;

    GPoint newCenter(pos);
    // Calculate new bubble's center based on entity's velocity and current position
    NewBubbleCenter( pDSE->GetVelocity(), newCenter );

    SystemBubble* pBubble = nullptr;
    if (isPostWarp)
        pBubble = FindBubble(newCenter);
    else
        pBubble = FindBubble(pDSE->GetPosition());

    if (pBubble) {
         _log(DESTINY__BUBBLE_TRACE, "BubbleManager::Add() - Entity %s (%u) being added to existing Bubble %u", ent->GetName(), ent->GetID(), pBubble->GetID() );
        pBubble->Add(ent, isPostWarp);
       return;
    }
    // this System Entity is not in any existing bubble, so let's make a new bubble
    // TODO check edges of bubbles....should NOT overlap.
    pBubble = new SystemBubble(ent->System(), newCenter, BUBBLE_RADIUS_METERS);

	_log(DESTINY__BUBBLE_TRACE, "BubbleManager::Add() - Entity '%s'(%u) being added to NEW Bubble %u", ent->GetName(), ent->GetID(), pBubble->GetID() );
    m_bubbles.push_back(pBubble);
    pBubble->Add(ent, isPostWarp);
}

void BubbleManager::NewBubbleCenter(GVector shipVelocity, GPoint &newCenter) {
    shipVelocity.normalize();
    newCenter += (shipVelocity * (BUBBLE_RADIUS_METERS /2));
}

void BubbleManager::Remove(SystemEntity *ent) {
    SystemBubble *b = ent->Bubble();
    if (b) {
        _log(DESTINY__BUBBLE_TRACE, "BubbleManager::Remove() - Entity '%s' (%u) being removed from Bubble %u", ent->GetName(), ent->GetID(), b->GetID() );
        b->Remove(ent);
    } else //not in any bubble.
        _log(DESTINY__BUBBLE_TRACE, "BubbleManager::Remove() - Entity %u is not located in any bubble. Nothing to remove.", ent->GetID());
}

//NOTE: this should probably eventually be optimized to use a
//spacial partitioning scheme to speed up this search.
SystemBubble* BubbleManager::FindBubble(SystemEntity *ent) const {
    GPoint pos = ent->GetPosition();
    for (auto cur : m_bubbles) {
        if (cur->InBubble(pos)) {
            return cur;
        }
    }
    //not in any existing bubble.
    return nullptr;
}

//NOTE: this should probably eventually be optimized to use a
//spacial partitioning scheme to speed up this search.
SystemBubble* BubbleManager::FindBubble(const GPoint &pos) const {
    for (auto cur : m_bubbles) {
        if (cur->InBubble(pos)) {
            return cur;
        }
    }
    //not in any existing bubble.
    return nullptr;
}

/* for beltmgr */
uint32 BubbleManager::GetSpawnID(uint16 bubbleID)
{
    std::map<uint16, uint32>::iterator itr = m_spawnIDs.find(bubbleID);
    if (itr == m_spawnIDs.end())
        return 0;
    return itr->second;
}

void BubbleManager::AddSpawnID(uint16 bubbleID, uint32 spawnID)
{
    m_spawnIDs.emplace(std::pair<uint16, uint32>(bubbleID, spawnID));
}
