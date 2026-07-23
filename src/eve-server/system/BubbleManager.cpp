/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    Copyright 2016 - 2026 Alasiya-EvE by Allan
    For the latest implementation status visit http://eve.alasiya.net/?p=op_status
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
    Rewrite:    Allan
*/

#include <algorithm>
#include <functional>
#include "../eve-server.h"

#include "Client.h"
#include "EntityMgr.h"
#include "EVE_Scanning.h"
#include "EVEServerConfig.h"
#include "map/MapData.h"
#include "system/BubbleManager.h"
#include "system/Container.h"
#include "system/SystemBubble.h"
#include "system/SystemEntity.h"
#include "system/SystemManager.h"


BubbleManager::BubbleManager()
: m_wanderTimer(0), m_bubbleID(0), m_profileStartTime(0)
{
}

int BubbleManager::Initialize() {
    // start timers
    m_wanderTimer.Start(sConfig.rates.WanderTimer * EvE::Timer::Minute);

    sLog.Blue("        BubbleMgr", "Bubble Manager Initialized.");
    return 1;
}

void BubbleManager::clear() {
    for (auto &cur : m_bubbles)
        SafeDelete(cur);

    sLog.Warning("        BubbleMgr", "Bubble Manager has been closed." );
}

void BubbleManager::Process() {
    m_profileStartTime = GetTimeUSeconds();

    bool bump = sConfig.cosmic.BumpEnabled;
    // this needs to be fast
    for (auto &cur : m_bubbles) {
        // process each bubble for spawns
        cur->Process();
        //process collisions
        if (bump)
            cur->ProcessCollisions();
    }

    if (m_wanderTimer.Check()) {    //600s
        _log(BUBBLE__INFO, "Wander Timer - %u", sEntityMgr.GetStamp());
        std::list<SystemBubble*>::iterator itr = m_bubbles.begin();
        while (itr != m_bubbles.end()) {
            if (*itr == nullptr) {
                itr = m_bubbles.erase(itr);
                continue;
            }
            if ((*itr)->HasDynamics())
                (*itr)->ProcessWander(m_wanderers);
            ++itr;
        }

        if (!m_wanderers.empty()) {
            SystemEntity* pSE = nullptr;
            // these are never null
            std::vector<SystemEntity*>::iterator itr = m_wanderers.begin();
            while (itr != m_wanderers.end()) {
                pSE = *itr;
                // do we really want to check this?  yes.  have found errors where position isNaN
                if (pSE->GetPosition().isNaN() or pSE->GetPosition().isInf() or pSE->GetPosition().isZero()) {
                    // position error.  this will screw things up.  if haspilot, send error.
                    if (pSE->HasPilot()) {
                        pSE->GetPilot()->SendErrorMsg("Internal Server Error.<br>Invalid Position.  Sending you to your home station.");
                        pSE->GetPilot()->MoveToLocation(pSE->GetPilot()->GetCloneStationID(), NULL_ORIGIN);
                    } else if (pSE->IsNPCSE()) {
                        pSE->Delete();
                        SafeDelete(pSE);
                    } else { //TODO: add items to check here
                        sLog.Error("BubbleMgr", "Wanderer %s(%s) position invalid.", pSE->GetName(), pSE->GetSEType());
                    }

                    itr = m_wanderers.erase(itr);
                    continue;
                }
                _log(BUBBLE__WARNING, "BubbleManager::Process() - Calling Checkbubble() for Wanderer %s(%u) in %s(%u).", \
                        pSE->GetName(), pSE->GetID(), pSE->SystemMgr()->GetName(), pSE->SystemMgr()->GetID());
                CheckBubble(pSE);
                itr = m_wanderers.erase(itr);
            }
            //m_wanderers.clear();
        }

        RemoveEmpty();
    }

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::bubbles, GetTimeUSeconds() - m_profileStartTime);
}

void BubbleManager::CheckBubble(SystemEntity* pSE) {
    SystemBubble *pBubble = pSE->SysBubble();
    if (pBubble != nullptr) {
        if (pBubble->InBubble(pSE->GetPosition())) {
            _log(BUBBLE__DEBUG, "BubbleManager::CheckBubble() - Entity '%s'(%u) at (%.2f,%.2f,%.2f) is still located in bubble %u at %.2f,%.2f,%.2f.",\
                 pSE->GetName(), pSE->GetID(), pSE->GetPosition().x, pSE->GetPosition().y, pSE->GetPosition().z,\
                 pBubble->GetID(), pBubble->x(), pBubble->y(), pBubble->z());
            return;
        }

        _log(BUBBLE__WARNING, "BubbleManager::CheckBubble() - Entity '%s'(%u) at (%.2f,%.2f,%.2f) is no longer located in bubble %u at %.2f,%.2f,%.2f.  Removing...",\
             pSE->GetName(), pSE->GetID(), pSE->GetPosition().x, pSE->GetPosition().y, pSE->GetPosition().z,\
             pBubble->GetID(), pBubble->x(), pBubble->y(), pBubble->z());
        pBubble->Remove(pSE);
    }

    _log(BUBBLE__DEBUG, "BubbleManager::CheckBubble() - SystemEntity '%s'(%u) not currently in any Bubble...adding", pSE->GetName(), pSE->GetID() );
    Add(pSE);
}

void BubbleManager::RemoveEmpty() {
    std::list<SystemBubble*>::iterator itr = m_bubbles.begin();
    while (itr != m_bubbles.end()) {
        if ((*itr)->IsEmpty()) {
            _log(BUBBLE__DEBUG, "BubbleManager::RemoveEmpty() - Bubble %u is empty and is being deleted from the system.", (*itr)->GetID() );
            _log(BUBBLE__TRACE, "BubbleManager::RemoveEmpty() - Entity list of bubble %u as follows...", (*itr)->GetID());
            //(*itr)->PrintEntityList();  // for debugging
            RemoveBubble((*itr)->GetSystem()->GetID(), (*itr));
            itr = m_bubbles.erase(itr);
        } else {
            ++itr;
        }
    }
}

void BubbleManager::Add(SystemEntity* pSE, bool isPostWarp /*false*/) {
    if (pSE == nullptr)
        return;

    if (pSE->GetPosition().isZero())
        pSE->DestinyMgr()->SetPosition(sMapData.GetRandPointOnPlanet(pSE->SystemMgr()->GetID()));

    Vector3d center = pSE->GetPosition();
    if (isPostWarp) {
        // Calculate new bubble's center based on entity's velocity and current position
        NewBubbleCenter(pSE->GetVelocity(), center);
    }

    SystemBubble* pBubble = GetBubble(pSE->SystemMgr(), center);
    if (pBubble == nullptr) {
        _log(BUBBLE__ERROR, "BubbleManager::Add(): GetBubble() returned nullptr for %s:%u, at (%.2f, %.2f, %.2f).", \
                    pSE->SystemMgr()->GetName(), pSE->SystemMgr()->GetID(), center.x, center.y, center.z );
        return;
    }

    if (pSE->SysBubble() != nullptr) {
        if (pBubble->GetSystemID() != pSE->SystemMgr()->GetID()) {
            // this is an error.  bad bubble
            _log(BUBBLE__ERROR, "BubbleManager::Add(): bubble SysID %u != pSE SysID %u", pBubble->GetSystemID(), pSE->SystemMgr()->GetID() );
            pSE->SysBubble()->Remove(pSE);
        } else if (pSE->SysBubble() != pBubble) {
            _log(BUBBLE__TRACE, "BubbleManager::Add(): bubbleID %u != pSE bubbleID %u", pBubble->GetID(), pSE->SysBubble()->GetID() );
            pSE->SysBubble()->Remove(pSE);
        } else if (pSE->SysBubble()->InBubble(pSE->GetPosition()))  {
            _log(BUBBLE__TRACE, "BubbleManager::Add(): Entity %s(%u) still in Bubble %u", pSE->GetName(), pSE->GetID(), pBubble->GetID() );
            return;
        }
    }

    _log(BUBBLE__TRACE, "BubbleManager::Add(): Entity %s(%u) being added to Bubble %u", pSE->GetName(), pSE->GetID(), pBubble->GetID() );
    pBubble->Add(pSE);
}

void BubbleManager::NewBubbleCenter(Vector3d shipVelocity, Vector3d& newCenter) {
    // check this...is this right?  create at 1/2 radius?
    shipVelocity.Normalize();
    float dist = BUBBLE_RADIUS_METERS * 0.5f;
    newCenter.x += (shipVelocity.x * dist);
    newCenter.y += (shipVelocity.y * dist);
    newCenter.z += (shipVelocity.z * dist);
}

void BubbleManager::Remove(SystemEntity* pSE) {
    if (pSE->SysBubble() != nullptr) {
        _log(BUBBLE__TRACE, "BubbleManager::Remove(): Entity %s(%u) being removed from Bubble %u", pSE->GetName(), pSE->GetID(), pSE->SysBubble()->GetID() );
        pSE->SysBubble()->Remove(pSE);
    }
}

/** UPDATE  large items, (sun, planet, moons) are no longer in bubbles.
 * this cuts number of bubbles drastically, dropping average to 10bbl/system,
 * and allowing a much larger amount of bubbles per system without
 * introducing lag from bubble processing.
 *
 * NOTE:  these are only used here...
 */
SystemBubble* BubbleManager::FindBubble(SystemEntity* pSE) const {
    return FindBubble(pSE->SystemMgr()->GetID(), pSE->GetPosition());
}

SystemBubble* BubbleManager::FindBubble(uint32 systemID, const Vector3d& position) const {
    // Finds a range containing all elements whose key is k.
    // pair<iterator, iterator> equal_range(const key_type& k)
    _log(BUBBLE__DEBUG, "BubbleManager::FindBubble() - Searching point %.1f, %.1f, %.1f in system %u.", \
                position.x, position.y, position.z, systemID);

    auto range = m_sysBubbleMap.equal_range(systemID);
    for ( auto itr = range.first; itr != range.second; ++itr )
        if (itr->second->InBubble(position))
            return itr->second;

    //not in any existing bubble in given systemID
    return nullptr;
}

SystemBubble* BubbleManager::GetBubble(SystemManager* sysMgr, const Vector3d& position) {
    SystemBubble* pBubble = FindBubble(sysMgr->GetID(), position);
    if (pBubble == nullptr)
        pBubble = MakeBubble(sysMgr, position);

    return pBubble;
}

SystemBubble* BubbleManager::MakeBubble(SystemManager* sysMgr, Vector3d position) {
    // determine if new center (pos) is within 2x radius of another bubble center. (overlap)
    Vector3d dir, centerPt;
    auto range = m_sysBubbleMap.equal_range(sysMgr->GetID());
    for (auto itr = range.first; itr != range.second; ++itr) {
        if (itr->second->IsOverlap(position)) {
            dir = itr->second->GetCenter();
            dir.x -= position.x;
            dir.y -= position.y;
            dir.z -= position.z;
            dir.Normalize();
            _log(BUBBLE__DEBUG, "BubbleManager::MakeBubble()::IsOverlap() - dir: %.3f,%.3f,%.3f", dir.x, dir.y, dir.z);
            // move pos away from center
            centerPt = itr->second->GetCenter();
            position.x = centerPt.x + (dir.x * (BUBBLE_RADIUS_METERS * 2));
            position.y = centerPt.y + (dir.y * (BUBBLE_RADIUS_METERS * 2));
            position.z = centerPt.z + (dir.z * (BUBBLE_RADIUS_METERS * 2));
            break;
        }
    }

    SystemBubble* pBubble = new SystemBubble(sysMgr, position, BUBBLE_RADIUS_METERS);
    if (pBubble != nullptr) {
        m_bubbles.push_back(pBubble);
        m_bubbleIDMap.emplace(pBubble->GetID(), pBubble);
        m_sysBubbleMap.emplace(sysMgr->GetID(), pBubble);
        if (sConfig.debug.BubbleTrack)
            pBubble->MarkCenter();
    }
    return pBubble;
}

SystemBubble* BubbleManager::FindBubbleByID(uint16 bubbleID) {
    std::map<uint32, SystemBubble*>::iterator itr = m_bubbleIDMap.find(bubbleID);
    if (itr != m_bubbleIDMap.end())
        return itr->second;
    return nullptr;
}

void BubbleManager::ClearSystemBubbles(uint32 systemID) {
    auto range = m_sysBubbleMap.equal_range(systemID);
    for (auto itr = range.first; itr != range.second; ++itr) {
        m_bubbles.remove(itr->second);
        m_bubbleIDMap.erase(itr->second->GetID());
    }

    m_sysBubbleMap.erase(systemID);
}

void BubbleManager::RemoveBubble(uint32 systemID, SystemBubble* pSB) {
    auto range = m_sysBubbleMap.equal_range(systemID);
    for (auto itr = range.first; itr != range.second; ++itr)
        if (itr->second == pSB) {
            m_sysBubbleMap.erase(itr);
            return;
        }
    std::map<uint32, SystemBubble*>::iterator itr = m_bubbleIDMap.find(pSB->GetID());
    if (itr != m_bubbleIDMap.end())
        m_bubbleIDMap.erase(itr);
}

/* for beltmgr */
void BubbleManager::AddSpawnID(uint16 bubbleID, uint32 spawnID) {
    m_spawnIDs.emplace(bubbleID, spawnID);
}

void BubbleManager::RemoveSpawnID(uint16 bubbleID, uint32 spawnID) {
    // is this right??
    auto range = m_spawnIDs.equal_range(bubbleID);
    for (auto itr = range.first; itr != range.second; ++itr) {
        if (itr->second == spawnID) {
            m_spawnIDs.erase(itr);
            return;
        }
    }
}

uint32 BubbleManager::GetBeltID(uint16 bubbleID) {
    std::map<uint16, uint32>::iterator itr = m_spawnIDs.find(bubbleID);
    if (itr == m_spawnIDs.end())
        return 0;
    return itr->second;
}

uint32 BubbleManager::GetBubbleCount(uint32 systemID) {
    uint32 count = 0;
    auto range = m_sysBubbleMap.equal_range(systemID);
    for (auto itr = range.first; itr != range.second; ++itr)
        ++count;
    return count;
}

void BubbleManager::GetBubbleCenterMarkers(std::vector<CosmicSignature>& anom) {
    ContainerSE* cSE = nullptr;
    for (auto &cur : m_sysBubbleMap) {
        cSE = cur.second->GetCenterMarker();
        if (cSE == nullptr)
            continue;
        CosmicSignature sig = CosmicSignature();
            sig.ownerID = cSE->GetOwnerID();
            sig.sigID = cSE->GetSelf()->customInfo();           // result.id
            sig.sigItemID = cSE->GetID();
            sig.sigName = cSE->GetName();                       // result.DungeonName
            //sig.sigGroupID = EVEDB::invGroups::Cosmic_Anomaly;  // result.groupID
            sig.sigStrength = 1.0;
            //sig.sigTypeID = EVEDB::invTypes::CosmicAnomaly;     // result.typeID
            sig.systemID = cur.first;
            sig.position = cSE->GetPosition();
            sig.scanAttributeID = AttrScanMagnetometricStrength;   // result.strengthAttributeID
            sig.scanGroupID = Scanning::Group::Signature;
            sig.dungeonType = Dungeon::Type::None;
        anom.push_back(sig);
        //cSE = nullptr;
    }
}

void BubbleManager::GetBubbleCenterMarkers(uint32 systemID, std::vector<CosmicSignature>& anom) {
    ContainerSE* cSE = nullptr;
    auto range = m_sysBubbleMap.equal_range(systemID);
    for (auto itr = range.first; itr != range.second; ++itr) {
        cSE = itr->second->GetCenterMarker();
        if (cSE == nullptr)
            continue;
        CosmicSignature sig = CosmicSignature();
            sig.ownerID = cSE->GetOwnerID();
            sig.sigID = cSE->GetSelf()->customInfo();
            sig.sigItemID = cSE->GetID();
            sig.sigName = cSE->GetName();
            //sig.sigGroupID = EVEDB::invGroups::Cosmic_Anomaly;
            sig.sigStrength = 1.0;
            //sig.sigTypeID = EVEDB::invTypes::CosmicAnomaly;
            sig.systemID = systemID;
            sig.position = cSE->GetPosition();
            sig.scanAttributeID = AttrScanMagnetometricStrength;
            sig.scanGroupID = Scanning::Group::Signature;
            sig.dungeonType = Dungeon::Type::None;
        anom.push_back(sig);
        //cSE = nullptr;
    }
}


void BubbleManager::MarkCenters() {
    for (auto &cur : m_sysBubbleMap)
        cur.second->MarkCenter();
}

void BubbleManager::RemoveMarkers() {
    for (auto &cur : m_sysBubbleMap)
        cur.second->RemoveMarkers();
}

void BubbleManager::MarkCenters(uint32 systemID) {
    auto range = m_sysBubbleMap.equal_range(systemID);
    for (auto itr = range.first; itr != range.second; ++itr)
        itr->second->MarkCenter();
}

void BubbleManager::RemoveMarkers(uint32 systemID) {
    auto range = m_sysBubbleMap.equal_range(systemID);
    for (auto itr = range.first; itr != range.second; ++itr)
        itr->second->RemoveMarkers();
}
