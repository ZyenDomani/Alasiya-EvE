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
    Updates:    Allan
*/

#include <algorithm>
#include <functional>
#include "eve-server.h"

#include "EVEServerConfig.h"
#include "system/BubbleManager.h"
#include "system/SystemBubble.h"
#include "system/SystemEntity.h"
#include "system/SystemManager.h"
#include "Client.h"


struct bubbleDeleter {
    void operator()(SystemBubble*& bRef) { // take pointer by reference
        if (bRef->IsEmpty()) {
            _log(DESTINY__BUBBLE_DEBUG, "BubbleManager::Process() - Bubble %u is empty and is being deleted from the system.", bRef->GetID() );
            sBubbleMgr.RemoveBubble(bRef->GetSystem()->GetID(), bRef);
            SafeDelete(bRef);
        }
    }
};

BubbleManager::BubbleManager()
: m_wanderTimer(30000)
{
    m_bubbles.clear();
    m_bubbleMap.clear();
    m_wanderers.clear();
    m_wanderTimer.Start(30000);
}

BubbleManager::~BubbleManager() {
    clear();
}

void BubbleManager::Initialize() {
    /* just to create the singleton here */
    sLog.Blue("   Bubble Manager", "Bubble Manager Initialized.");
}

void BubbleManager::clear() {
    for (auto cur : m_bubbles)
        SafeDelete(cur);

    m_bubbles.clear();
    m_bubbleMap.clear();
}

void BubbleManager::Process() {
    double profileStartTime = 0.0;
    if (sConfig.server.UseProfiling)
        profileStartTime = GetTimeUSeconds();
    // process each belt and gate bubble for spawns
    for (auto cur : m_bubbles) {
        if (cur->IsBelt() or cur->IsGate())
            cur->Process();
    }
    // run wander check every 30 sec for all active bubbles
    if (m_wanderTimer.Check()) {
        // STL-friendly pointer delteter and remover   - clever code here.
        std::for_each(m_bubbles.begin(), m_bubbles.end(), bubbleDeleter());
        std::vector<SystemBubble*>::iterator new_end = remove(m_bubbles.begin(), m_bubbles.end(), static_cast<SystemBubble*>(nullptr));
        m_bubbles.erase(new_end, m_bubbles.end());

        m_wanderers.clear();
        std::vector<SystemBubble*>::iterator itr = m_bubbles.begin();
        while (itr != m_bubbles.end()) {
            if ((*itr)->HasDynamics())
                (*itr)->ProcessWander(m_wanderers);
            ++itr;
        }

        if (!m_wanderers.empty()) {
            for (auto cur : m_wanderers) {
                _log(DESTINY__WARNING, "BubbleManager::Process() - Wanderer %s(%u) in system %s(%u) is being added to a bubble.", \
                        cur->GetName(), cur->GetID(), cur->SystemMgr()->GetName().c_str(), cur->SystemMgr()->GetID());
                CheckBubble(cur);
            }
            m_wanderers.clear();
        }
    }
    if (sConfig.server.UseProfiling)
        sProfile.AddTime(_bubblesProfile, GetTimeUSeconds() - profileStartTime);
}

void BubbleManager::CheckBubble(SystemEntity *pSE) {
    SystemBubble *pBubble = pSE->SysBubble();
    if (pBubble != nullptr) {
        if (pBubble->InBubble(pSE->GetPosition())) {
            _log(DESTINY__BUBBLE_DEBUG, "BubbleManager::CheckBubble() - Entity '%s'(%u) at (%.2f,%.2f,%.2f) is still located in bubble %u at %.2f,%.2f,%.2f.",\
                 pSE->GetName(), pSE->GetID(), pSE->GetPosition().x, pSE->GetPosition().y, pSE->GetPosition().z,\
                 pBubble->GetID(), pBubble->x(), pBubble->y(), pBubble->z());
            return;
        }

        _log(DESTINY__BUBBLE_DEBUG, "BubbleManager::CheckBubble() - Entity '%s'(%u) at (%.2f,%.2f,%.2f) is no longer located in bubble %u at %.2f,%.2f,%.2f.  Removing...",\
             pSE->GetName(), pSE->GetID(), pSE->GetPosition().x, pSE->GetPosition().y, pSE->GetPosition().z,\
             pBubble->GetID(), pBubble->x(), pBubble->y(), pBubble->z());
        pBubble->Remove(pSE);
    }

    _log(DESTINY__BUBBLE_DEBUG, "BubbleManager::CheckBubble() - SystemEntity '%s'(%u) not currently in any Bubble...adding", pSE->GetName(), pSE->GetID() );
    Add(pSE);
}

void BubbleManager::Add(SystemEntity* pSE, bool isPostWarp /*false*/) {
    if (!pSE) return;
    const GPoint &pos(pSE->GetPosition());
    if (pos.isZero())
        ; /** @todo do something constructive here */

    SystemBubble* pBubble(nullptr);
    GPoint newCenter(pos);
    if (isPostWarp) {
        // Calculate new bubble's center based on entity's velocity and current position
        NewBubbleCenter( pSE->GetVelocity(), newCenter );
    }

    pBubble = FindBubble(pSE->SystemMgr()->GetID(), newCenter);
    if (pBubble != nullptr) {
        if (pBubble->GetSystem()->GetID() != pSE->SystemMgr()->GetID()) {
            // this is an error.  bad bubble
            _log(DESTINY__BUBBLE_TRACE, "BubbleManager::Add(): bubble SysID %u != pSE SysID %u", pSE->GetName(), pSE->GetID(), pBubble->GetSystem()->GetID(), pSE->SystemMgr()->GetID() );
        }
        _log(DESTINY__BUBBLE_TRACE, "BubbleManager::Add(): Entity %s (%u) being added to existing Bubble %u", pSE->GetName(), pSE->GetID(), pBubble->GetID() );
        pBubble->Add(pSE);
        return;
    }
    // this System Entity is not in any existing bubble, so let's make a new bubble
    // TODO check edges of bubbles....should NOT overlap.
    pBubble = new SystemBubble(pSE->SystemMgr(), newCenter, BUBBLE_RADIUS_METERS);
    m_bubbles.push_back(pBubble);
    // testing....W.I.P.
    m_bubbleMap.emplace(pSE->SystemMgr()->GetID(), pBubble);

    _log(DESTINY__BUBBLE_TRACE, "BubbleManager::Add(): Entity '%s'(%u) being added to NEW Bubble %u", pSE->GetName(), pSE->GetID(), pBubble->GetID() );
    pBubble->Add(pSE);
}

void BubbleManager::NewBubbleCenter(GVector shipVelocity, GPoint &newCenter) {
    /** @todo  need to write a method that will check for other bubbles within (radius) of this one, and if found, move centers to (2r) away. */
    shipVelocity.normalize();
    newCenter += (shipVelocity * (BUBBLE_RADIUS_METERS /2));
}

void BubbleManager::Remove(SystemEntity *ent) {
    // planets and moons arent in bubbles
    if (ent->IsPlanetSE() or ent->IsMoonSE())
        return;
    SystemBubble *pBubble = ent->SysBubble();
    if (pBubble != nullptr) {
        _log(DESTINY__BUBBLE_TRACE, "BubbleManager::Remove(): Entity '%s' (%u) being removed from Bubble %u", ent->GetName(), ent->GetID(), b->GetID() );
        pBubble->Remove(ent);
    }
}

/** UPDATE  large items, (sun, planet, moons) are no longer in bubbles.
 * this cuts number of bubbles drastically, dropping average to 10bbl/system,
 * and allowing a much larger amount of bubbles per system without
 * introducing lag from bubble processing.
 *
 * to further reduce bubble-finding process times, testing an unordered multimap
 * with bubbles entered by <systemID, SystemBubble*> to search only bubbles in desired system,
 * greatly reducing the search time for many loaded systems.
 *
 * NOTE:  these are only used here...
 */
SystemBubble* BubbleManager::FindBubble(SystemEntity *ent) const {
    return FindBubble(ent->SystemMgr()->GetID(), ent->GetPosition());
}

SystemBubble* BubbleManager::FindBubble(uint32 systemID, const GPoint &pos) const {
    // Finds a range containing all elements whose key is k.
    // pair<iterator, iterator> equal_range(const key_type& k)
    auto range = m_bubbleMap.equal_range(systemID);
    for ( auto itr = range.first; itr != range.second; itr++ )
        if (itr->second->InBubble(pos))
            return itr->second;

    //not in any existing bubble.
    return nullptr;
}

void BubbleManager::ClearSystemBubbles(uint32 systemID)
{
    m_bubbleMap.erase(systemID);
}

void BubbleManager::RemoveBubble(uint32 systemID, SystemBubble* pSB)
{
    auto range = m_bubbleMap.equal_range(systemID);
    for ( auto itr = range.first; itr != range.second; itr++ )
        if (itr->second = pSB) {
            m_bubbleMap.erase(itr);
            return;
        }
}

/* for beltmgr */
void BubbleManager::AddSpawnID(uint16 bubbleID, uint32 spawnID)
{
    m_spawnIDs.emplace(bubbleID, spawnID);
}

void BubbleManager::RemoveSpawnID(uint16 bubbleID, uint32 spawnID)
{
    auto range = m_spawnIDs.equal_range(bubbleID);
    for ( auto itr = range.first; itr != range.second; itr++ )
        if (itr->second = spawnID) {
            m_spawnIDs.erase(itr);
            return;
        }
}

uint32 BubbleManager::GetBeltID(uint16 bubbleID)
{
    std::map<uint16, uint32>::const_iterator itr = m_spawnIDs.find(bubbleID);
    if (itr == m_spawnIDs.end())
        return 0;
    return itr->second;
}
