/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:     Zhur
    Rewrite:    Allan
    Updates:    James
*/

#include "../eve-server.h"

#include "Client.h"
#include "EVEServerConfig.h"
#include "account/AccountService.h"
#include "chat/LSCService.h"
#include "exploration/Probes.h"
#include "map/MapData.h"
#include "map/MapDB.h"
#include "npc/Civilian.h"
#include "npc/Drone.h"
#include "npc/NPC.h"
#include "npc/Sentry.h"
#include "packets/Destiny.h"
#include "planet/Planet.h"
#include "planet/Moon.h"
#include "planet/CustomsOffice.h"
#include "pos/Array.h"
#include "pos/Battery.h"
#include "pos/Module.h"
#include "pos/Structure.h"
#include "pos/Tower.h"
#include "pos/sovStructures/TCU.h"
#include "pos/sovStructures/SBU.h"
#include "pos/sovStructures/IHub.h"
#include "pos/Weapon.h"
#include "ship/Missile.h"
#include "ship/Ship.h"
#include "station/Station.h"
#include "system/Asteroid.h"
#include "system/Container.h"
#include "system/DestinyManager.h"
#include "system/SolarSystem.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/AnomalyMgr.h"
#include "system/cosmicMgrs/BeltMgr.h"
#include "system/cosmicMgrs/CivilianMgr.h"
#include "system/cosmicMgrs/DungeonMgr.h"
#include "system/cosmicMgrs/SpawnMgr.h"


SystemManager::SystemManager(uint32 systemID, PyServiceMgr &svc)
:m_services(svc),
m_bountyTimer(0),
m_minutetimer(0, true),
m_anomMgr(new AnomalyMgr(this, svc)),
m_beltMgr(new BeltMgr(this, svc)),
m_dungMgr(new DungeonMgr(this, svc)),
m_spawnMgr(new SpawnMgr(this, svc)),
m_loaded(false),
m_entityChanged(false),
m_docked(0),
m_players(0),
m_beltCount(0),
m_gateCount(0),
m_activityTime(0),
m_activeRatSpawns(0),
m_activeGateSpawns(0),
m_activeRoidSpawns(0),
m_secValue(1.1f),
m_minutes(0),
// zero-init our data containers
m_data(SystemData()),
m_killData(SystemKillData())
{
    sDataMgr.GetSystemData(systemID, m_data);   // system data is now an internal memory (cached) object.
    m_secValue -= m_data.security;  // range is 0.1 for 1.0 system to 2.0 for -0.9 system

    _log(COMMON__MESSAGE, "Created SystemManager %p for System %s(%u)", this, m_data.name.c_str(), m_data.systemID);
}

SystemManager::~SystemManager() {
    if (m_players or !m_clients.empty()) {
        _log(COMMON__ERROR, "D'tor called for System %u with %u players and/or %lu clients in mmaps", m_data.systemID, m_players, m_clients.size());
        for (auto &cur : m_clients)
            sEntityMgr.Remove(cur.second);
    }

    if (m_loaded)
        UnloadSystem();

    SafeDelete(m_dungMgr);
    SafeDelete(m_anomMgr);
    SafeDelete(m_beltMgr);
    SafeDelete(m_spawnMgr);
}

bool SystemManager::BootSystem() {
    // dont fuck with this order...

    m_solarSystemRef = sItemFactory.GetSolarSystemRef(m_data.systemID);
    assert(m_solarSystemRef.get() != nullptr);

    if (!LoadSystemStatics()) {
        _log(SERVICE__ERROR, "Unable to load System Statics during boot of system %u.", m_data.systemID);
        return false;
    }

    if (!LoadCosmicMgrs()) {
        _log(SERVICE__ERROR, "Unable to load Cosmic Managers during boot of system %u.", m_data.systemID);
        return false;
    }

    if (!LoadSystemDynamics()) {
        _log(SERVICE__ERROR, "Unable to load System Dynamics during boot of system %u.", m_data.systemID);
        return false;
    }

    if (!LoadPlayerDynamics()) {
        _log(SERVICE__ERROR, "Unable to load System Dynamics during boot of system %u.", m_data.systemID);
        return false;
    }

    if (sConfig.server.BountyPayoutDelayed)
        m_bountyTimer.Start(sConfig.server.BountyPayoutTimer * EvE::Timer::Minute);

    //create our chat channels
    m_services.lsc_service->CreateSystemChannel(m_data.regionID);
    m_services.lsc_service->CreateSystemChannel(m_data.constellationID);
    m_services.lsc_service->CreateSystemChannel(m_data.systemID);

    // inform MarketBot of loaded system and its stations
    //sMktBotMgr.AddSystem();

    // set system active for system status page
    MapDB::SetSystemActive(m_data.systemID, true);

    // load dynamic map data
    MapDB::LoadDynamicData(m_data.systemID, m_killData);

    //start minute timer
    m_minutetimer.Start(60000);

    return (m_loaded = true);
}

bool SystemManager::LoadCosmicMgrs()
{
    if (m_beltCount)
        m_beltMgr->Init(m_data.regionID);  //nothing to check for in this init.

    if (!m_spawnMgr->Init()) {
        _log(SERVICE__ERROR, "Unable to load Spawn Manager during boot of system %u.", m_data.systemID);
        return false;
    }

    if (!m_dungMgr->Init(m_anomMgr, m_spawnMgr)) {
        _log(SERVICE__ERROR, "Unable to load Dungeon Manager during boot of system %u.", m_data.systemID);
        return false;
    }

    if (!m_anomMgr->Init(m_beltMgr, m_dungMgr, m_spawnMgr)) {
        _log(SERVICE__ERROR, "Unable to load Anomaly Manager during boot of system %u.", m_data.systemID);
        return false;
    }

    return true;
}

//called on 1Hz tic from EntityMgr.
bool SystemManager::ProcessTic() {
    double profileStartTime(GetTimeUSeconds());

    for (auto &cur : m_deleteLater)
        SafeDelete(cur.second);
    m_deleteLater.clear();

    std::map<uint32, SystemEntity*>::iterator itr = m_ticEntities.begin(), end = m_ticEntities.end();
    while (itr != end) {
        /* main process call. */
        itr->second->Process();

    /* the idea here is entities map NEVER has invalid items in it, but our iterator may become invalid
     * when SE->Process() returns because it will add/remove from the map as needed for
     * new objects, destroyed objects, moved objects, etc
     *  std::map internally orders items by key(itemID here)
     * when map changes, reset itr & end for new map and continue (much faster than old way of iteration)
     */
        if (m_entityChanged) {
            m_entityChanged = false;
            // using .find() is MUCH faster than iterating thru entire list
            itr = m_ticEntities.find(itr->first);
            end = m_ticEntities.end();

            if (itr == end) {
                // cur SE is the iterator being deleted.  break out.
                // NOTE:  do NOT start over here, as that will allow all previous SE a second action in this tic.
                break;
            }
        }

        // move on to next one.
        ++itr;
    }

    // check bounty timer
    if (m_bountyTimer.Check(sConfig.server.BountyPayoutDelayed))
        PayBounties();

    /* the following are coded for single-tic calls */
    m_anomMgr->Process();
    if (m_beltCount)
        m_beltMgr->Process();
    m_dungMgr->Process();
    m_spawnMgr->Process();

    // process planets for PI
    if (m_minutetimer.Check()) {
        ++m_minutes;  // not used at this time
        for (auto &cur : m_planetMap)
            cur.second->Process();
    }

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::system, GetTimeUSeconds() - profileStartTime);

    return SystemActivity();
}

bool SystemManager::SystemActivity() {
    if (!sConfig.world.gridUnload)
        return true;
    if (m_activityTime == 0)
        return true;
    if ((sEntityMgr.GetStamp() - m_activityTime) > sConfig.world.gridUnloadTime)
        return false;

    return true;
}

// called from EntityMgr::Process() and EntityMgr::Close()
void SystemManager::UnloadSystem() {
    if (!m_loaded)
        return;

    sLog.Magenta("    SystemManager", "UnloadSystem() called for %s(%u).", m_data.name.c_str(), m_data.systemID);

    // system is being unloaded.  pay bounties now
    PayBounties();

    // set system inactive for system status page
    MapDB::SetSystemActive(m_data.systemID, false);

    /** @todo finish this for lsc */
    m_services.lsc_service->SystemUnload(m_data.systemID, m_data.constellationID, m_data.regionID);

    // remove dungeon shit for this system
    //  are we saving it for later or is it make-on-boot?
    ManagerDB::ClearDungeons(m_data.systemID);

    // unload belts, which saves and removes roids from system
    m_beltMgr->ClearAll();
    // close anomaly mgr, which saves and removes sigs from system
    m_anomMgr->Close();

    // this still needs some work... seems ok to me.  26Dec18
    sBubbleMgr.ClearSystemBubbles(m_data.systemID);

    // inform MarketBot of system unloading to remove system stations from proc loop.
    //sMktBotMgr.RemoveSystem();

    // remove all loaded entities
    std::map<uint32, SystemEntity*>::iterator itr = m_entities.begin();
    SystemEntity* pSE(nullptr);
    while (itr != m_entities.end()) {
        if ((itr->first == 0) or (itr->second == nullptr)) {
            // this will catch entities we deleted earlier (roids, dungeons, etc)
            itr = m_entities.erase(itr);
            continue;
        }
        pSE = itr->second;

        if (pSE->TargetMgr() != nullptr)
            pSE->TargetMgr()->Unload();

        if (pSE->IsStaticEntity() or pSE->isGlobal()) {
            if (pSE->IsStationSE()) {
                // tell all client this station is being unloaded  (elusive segfault fix)
                std::vector<Client*> cVec;
                sEntityMgr.GetClients(cVec);
                for (auto &cur : cVec)
                    cur->SetHangarLoaded(pSE->GetID(), false);

                pSE->GetStationSE()->UnloadStation();
                sEntityMgr.RemoveStation(itr->first);
            }
            m_staticEntities.erase(itr->first);
        } else if (pSE->IsShipSE()) {
            pSE->GetShipSE()->GetShipItemRef()->LogOut();
        } else if (pSE->IsNPCSE()) {
            sEntityMgr.RemoveNPC();    // this is for loaded npc count.
            m_npcs.erase(pSE->GetID());
            pSE->GetSelf()->Delete();
        } else if (pSE->IsProbeSE()) {
            sEntityMgr.RemoveProbe(itr->first);
        }

        sItemFactory.RemoveItem(itr->first);
        m_ticEntities.erase(itr->first);
        itr = m_entities.erase(itr);
        sBubbleMgr.Remove(pSE);
        SafeDelete(pSE);
    }

    for (auto &cur : m_deleteLater)
        SafeDelete(cur.second);

    // save items, then remove from system inventory, item factory and decrement item count
    m_solarSystemRef->GetMyInventory()->Unload();
    _log(PHYSICS__MESSAGE, "SystemManager::UnloadSystem() - map count after unload: %lu npcs, %lu entities, %lu statics.", \
                m_npcs.size(), m_entities.size(), m_staticEntities.size());

    // at this point, these lists should be clear
    m_npcs.clear();
    m_clients.clear();
    m_entities.clear();
    m_deleteLater.clear();
    m_ticEntities.clear();
    m_staticEntities.clear();
    m_opStaticEntities.clear();

    // remove solar system item from ItemFactory
    sItemFactory.RemoveItem(m_data.systemID);
    m_loaded = false;
}

bool SystemManager::LoadSystemStatics() {
    std::vector<DBSystemEntity> entities;

    if (!SystemDB::LoadSystemStaticEntities(m_data.systemID, entities)) {
        sLog.Error( "SystemManager::LoadSystemStatics()", "Unable to load celestial entities during boot of %s(%u).", m_data.name.c_str(), m_data.systemID);
        return false;
    }

    SystemEntity* pSE(nullptr);
    for (auto &cur : entities) {
        switch (cur.groupID) {
            case EVEDB::invGroups::Station: {
                /** @todo (Allan) outposts are group::station - may need to hack this */
                /*  types 12242 - 22298 in group 15 are outposts */
                /*  types 29323 - 29390 in group 15 are wrecked stations */
                StationItemRef itemRef = sItemFactory.GetStationRef(cur.itemID);
                StationSE *pSSE = new StationSE(itemRef, *(GetServiceMgr()), this);
                sEntityMgr.AddStation(cur.itemID, itemRef);
                pSE = pSSE;
            } break;
            case EVEDB::invGroups::Asteroid_Belt: {
                CelestialObjectRef itemRef = sItemFactory.GetCelestialRef(cur.itemID);
                BeltSE *pBSE = new BeltSE(itemRef, *(GetServiceMgr()), this);
                pBSE->SetBeltMgr(m_beltMgr);
                ++m_beltCount;
                pSE = pBSE;
            } break;
            case EVEDB::invGroups::Stargate: {
                CelestialObjectRef itemRef = sItemFactory.GetCelestialRef(cur.itemID);
                itemRef->SetAttribute(AttrRadius, cur.radius, false);
                StargateSE *pSSE = new StargateSE(itemRef, *(GetServiceMgr()), this);
                m_gateMap.insert(std::pair<uint32, SystemEntity*>(cur.itemID, pSSE));
                ++m_gateCount;
                pSE = pSSE;
            } break;
            case EVEDB::invGroups::Planet: {
                CelestialObjectRef itemRef = sItemFactory.GetCelestialRef(cur.itemID);
                itemRef->SetAttribute(AttrRadius, cur.radius, false);
                PlanetSE *pPSE = new PlanetSE(itemRef, *(GetServiceMgr()), this);
                m_planetMap.insert(std::pair<uint32, SystemEntity*>(cur.itemID, pPSE));
                pSE = pPSE;
            } break;
            case EVEDB::invGroups::Moon: {
                CelestialObjectRef itemRef = sItemFactory.GetCelestialRef(cur.itemID);
                itemRef->SetAttribute(AttrRadius, cur.radius, false);
                MoonSE *pMSE = new MoonSE(itemRef, *(GetServiceMgr()), this);
                m_moonMap.insert(std::pair<uint32, SystemEntity*>(cur.itemID, pMSE));
                pSE = pMSE;
            } break;
            case EVEDB::invGroups::Sun: {    // suns dont have anything special, so they are generic SSEs
                CelestialObjectRef itemRef = sItemFactory.GetCelestialRef(cur.itemID);
                itemRef->SetAttribute(AttrRadius, cur.radius, false);
                StaticSystemEntity *pSSE = new StaticSystemEntity(itemRef, *(GetServiceMgr()), this);
                pSE = pSSE;
            } break;
            default: {
                sLog.Error( "SystemManager::LoadSystemStatics()", "create static entity called for unhandled item %u (grp: %u, type %u)", cur.itemID, cur.groupID, cur.typeID);
                continue;
            }
        }
        if (pSE == nullptr) {
            sLog.Error( "SystemManager::LoadSystemStatics()", "Failed to create entity for item %u (grp: %u, type %u)", cur.itemID, cur.groupID, cur.typeID);
            continue;
        }
        if (pSE->IsGateSE() or pSE->IsStationSE())
            sBubbleMgr.Add(pSE);
        if (pSE->IsBeltSE()) {
            sBubbleMgr.Add(pSE);
            m_beltVector.push_back(cur.itemID);
        }
        if (!pSE->LoadExtras())
            _log(INV__WARNING, "SystemManager::LoadSystemStatics() - Failed to load additional data for entity %u. Continuing.", cur.itemID);

        m_entities[cur.itemID] = pSE;
        m_staticEntities[cur.itemID] = pSE;
        AddItemToInventory(pSE->GetSelf());
    }

    _log(SERVER__INIT, "SystemManager::LoadSystemStatics() - %lu Static System entities loaded for %s (%u)", entities.size(), m_data.name.c_str(), m_data.systemID);
    return true;
}

bool SystemManager::LoadSystemDynamics() {
    std::vector<DBSystemDynamicEntity> entityData;
    if (!SystemDB::LoadSystemDynamicEntities(m_data.systemID, entityData)) {
        sLog.Error( "SystemManager::LoadSystemDynamics()", "Unable to load dynamic entities during boot of %s(%u).", m_data.name.c_str(), m_data.systemID);
        return false;
    }

    SystemEntity* pSE(nullptr);
    for (auto &cur : entityData) {
        pSE = DynamicEntityFactory::BuildEntity(*this, cur);
        if (pSE == nullptr) {
            sLog.Error( "SystemManager::LoadSystemDynamics()", "Failed to create entity for item %u (grp: %u, type %u)",
                                cur.itemID, cur.groupID, cur.typeID);
            continue;
        }
        _log(ITEM__TRACE, "SystemManager::LoadSystemDynamics() - Loaded dynamic entity %u of type %u for %s(%u)", \
                    cur.itemID, cur.typeID, m_data.name.c_str(), m_data.systemID);
        if (pSE->GetPosition().isZero())
            pSE->SetPosition(sMapData.GetRandPointOnPlanet(m_data.systemID));
            //pSE->SetPosition(sMapData.GetRandPointOnMoon(m_data.systemID));
        AddEntity(pSE);
    }
    _log(SERVER__INIT, "SystemManager::LoadSystemDynamics - %lu Dynamic System entities loaded for %s(%u)", entityData.size(), m_data.name.c_str(),m_data.systemID);

    return true;
}

bool SystemManager::LoadPlayerDynamics() {
    std::vector<DBSystemDynamicEntity> entityData;
    if (!SystemDB::LoadPlayerDynamicEntities(m_data.systemID, entityData)) {
        sLog.Error( "SystemManager::LoadPlayerDynamics()", "Unable to load player dynamic entities in %s(%u).", m_data.name.c_str(), m_data.systemID);
        return false;
    }

    SystemEntity* pSE(nullptr);
    for (auto &cur : entityData) {
        pSE = DynamicEntityFactory::BuildEntity(*this, cur);
        if (pSE == nullptr) {
            sLog.Error( "SystemManager::LoadPlayerDynamics()", "Failed to create entity for item %u (grp: %u, type %u)", cur.itemID, cur.groupID, cur.typeID);
            continue;
        }
        _log(ITEM__TRACE, "SystemManager::LoadPlayerDynamics() - Loaded dynamic entity %u of type %u for %s(%u)", \
                    cur.itemID, cur.typeID, m_data.name.c_str(),m_data.systemID);
        if (pSE->GetPosition().isZero())
            pSE->SetPosition(sMapData.GetRandPointOnMoon(m_data.systemID));
            //pSE->SetPosition(sMapData.GetRandPointOnPlanet(m_data.systemID));
        AddEntity(pSE);
    }
    _log(SERVER__INIT, "SystemManager::LoadPlayerDynamics() - %lu Dynamic Player entities loaded for %s(%u)", \
                entityData.size(), m_data.name.c_str(),m_data.systemID);

    return true;
}

bool SystemManager::BuildDynamicEntity(const DBSystemDynamicEntity& data, uint32 launcherID/*0*/) {
    SystemEntity* pSE = DynamicEntityFactory::BuildEntity(*this, data);
    if (pSE == nullptr) {
        sLog.Error( "SystemManager::BuildDynamicEntity()", "Failed to create entity for item %u (grp: %u, type %u)", data.itemID, data.groupID, data.typeID);
        return false;
    }

    _log(ITEM__TRACE, "SystemManager::BuildDynamicEntity() - Created dynamic entity %u of type %u for %s(%u)", \
                data.itemID, data.typeID, m_data.name.c_str(),m_data.systemID);

    // this is only used for wrecks...
    if (launcherID) {
        WreckSE* pWE = pSE->GetWreckSE();
        pWE->SetLaunchedByID(launcherID);
        if (IsCharacterID(data.ownerID)) {
            Client* pClient = sEntityMgr.FindClientByCharID(data.ownerID);
            if (pClient->InFleet())
                pWE->SetFleetID(pClient->GetFleetID());
        }
    }

    AddEntity(pSE);

    return true;
}

void SystemManager::AddClient(Client* pClient, bool count/*false*/, bool jump/*false*/) {
    //called from Client::MoveToLocation() on login and when changing systems
    if (pClient == nullptr)
        return;
    if (m_clients.find(pClient->GetCharacterID()) == m_clients.end()) {
        m_clients[pClient->GetCharacterID()] = pClient;
        _log(PLAYER__TRACE, "%s(%u): Added to system manager for %s(%u) - %lu clients now in system. count %s", \
                    pClient->GetName(), pClient->GetCharacterID(), m_data.name.c_str(), m_data.systemID, m_clients.size(), count?"true":"false");
    } else {
        // error for player already in client map
        _log(PLAYER__ERROR, "%s(%u): Already in player map for %s(%u)", \
            pClient->GetName(), pClient->GetCharacterID(), m_data.name.c_str(), m_data.systemID);
    }

    m_activityTime = 0;

    if (count) {
        ++m_players;
        _log(PLAYER__INFO, "%s(%u): Added to player count for %s(%u) - new count: %u", \
                pClient->GetName(), pClient->GetCharacterID(), m_data.name.c_str(), m_data.systemID, m_players);
    }
    if (jump) {
        //add jump in this system
        MapDB::AddJump(m_data.systemID);

        _log(PLAYER__INFO, "%s(%u): Add Jump to %s(%u)", \
        pClient->GetName(), pClient->GetCharacterID(), m_data.name.c_str(), m_data.systemID);

        uint16 stamp = sEntityMgr.GetStamp();
        std::map<uint32, uint8>::iterator itr = m_jumpMap.find(stamp);
        if (itr != m_jumpMap.end()) {
            ++(itr->second);
        } else {
            m_jumpMap.emplace(stamp, 1);
        }
    }
}

void SystemManager::RemoveClient(Client* pClient, bool count/*false*/, bool jump/*false*/) {
    //called from Client::~Client() and Client::MoveToLocation() when changing systems
    if (pClient == nullptr)
        return;
    m_clients.erase(pClient->GetCharacterID());
    _log(PLAYER__TRACE, "%s(%u): Removed from system manager for %s(%u) - %lu clients still in system.", \
            pClient->GetName(), pClient->GetCharacterID(), m_data.name.c_str(), m_data.systemID, m_clients.size());

    if (count) {
        --m_players;
        if (m_players < 1)
            m_clients.clear();  // redundant but safe

        _log(PLAYER__INFO, "%s(%u): Removed from player count for %s(%u) - new count: %u", \
                pClient->GetName(), pClient->GetCharacterID(), m_data.name.c_str(), m_data.systemID, m_players);
    }
    if (jump) {
        //add jump in this system
        MapDB::AddJump(m_data.systemID);

        _log(PLAYER__INFO, "%s(%u): Add Jump to %s(%u)", \
                pClient->GetName(), pClient->GetCharacterID(), m_data.name.c_str(), m_data.systemID);

        uint16 stamp = sEntityMgr.GetStamp();
        std::map<uint32, uint8>::iterator itr = m_jumpMap.find(stamp);
        if (itr != m_jumpMap.end()) {
            ++(itr->second);
        } else {
            m_jumpMap.emplace(stamp, 1);
        }
    }
}

void SystemManager::SetDockCount(Client* pClient, bool docked/*false*/)
{
    if (docked) {
        ++m_docked;
    } else {
        --m_docked;
    }

    if (m_players > sEntityMgr.GetPlayerCount())
        GetPlayerCount();
    if (m_docked > m_players)
        GetDockedCount();

    MapDB::UpdatePilotCount(m_data.systemID, m_docked, (m_players - m_docked));

    _log(PLAYER__INFO, "%s(%u): %s docked count for %s(%u) - new count: %u", \
            pClient->GetName(), pClient->GetCharacterID(), docked ? "Added to" : "Removed from",  m_data.name.c_str(), m_data.systemID, m_docked);
}

void SystemManager::AddNPC(NPC* pNPC) {
    if ( pNPC == nullptr)
        return;
    uint32 itemID = pNPC->GetID();
    if (m_npcs.find(itemID) != m_npcs.end()) {
        _log(ITEM__WARNING, "%s(%u): Called AddNPC(), but they're already in %s(%u).  Check bubble.", pNPC->GetName(), itemID, m_data.name.c_str(), m_data.systemID);
    } else {
        m_npcs[itemID] = pNPC;
    }

    _log(NPC__TRACE, "%s(%u): Added to system manager for %s(%u)", pNPC->GetName(), pNPC->GetID(), m_data.name.c_str(), m_data.systemID);
    AddEntity(pNPC, false);
    sEntityMgr.AddNPC();
}

void SystemManager::RemoveNPC(NPC* pNPC) {
    if ( pNPC == nullptr)
        return;
    auto itr = m_npcs.find(pNPC->GetID());
    if (itr != m_npcs.end())
        m_npcs.erase(itr);

    _log(NPC__TRACE, "%s(%u): Removed from system manager for %s(%u)", pNPC->GetName(), pNPC->GetID(), m_data.name.c_str(), m_data.systemID);
    RemoveEntity(pNPC);
    sEntityMgr.RemoveNPC();    // this is for loaded npc count.
    pNPC->RemoveNPC();   // this deletes NPC from DB.  NPC's dont jump, so no reason to remove from system unless killed
}

void SystemManager::AddEntity(SystemEntity* pSE, bool addSignal/*true*/) {
    if (pSE == nullptr)
        return;
    uint32 itemID(pSE->GetID());
    if (m_entities.find(itemID) != m_entities.end()) {
        _log(ITEM__WARNING, "%s(%u): Called AddEntity(), but they're already in %s(%u).  Check bubble.", pSE->GetName(), itemID, m_data.name.c_str(), m_data.systemID);
        return;
    }

    _log(ITEM__TRACE, "%s(%u): Added to system manager for %s(%u)", pSE->GetName(), itemID, m_data.name.c_str(), m_data.systemID);
    m_entities[itemID] = pSE;

    if ((pSE->IsCOSE())
    or  (pSE->isGlobal())) {
        m_staticEntities[itemID] = pSE;
        if (pSE->IsOperSE())
            //m_opStaticEntities[itemID] = pSE;
            m_ticEntities[itemID] = pSE;
        if (m_loaded)   // only update when system is already loaded
            SendStaticBall(pSE);
    } else if (pSE->IsProbeSE()) {
        // probes are now running sub-hz tics, so dont add to proc list.
        sEntityMgr.AddProbe(itemID, pSE->GetProbeSE());
    } else if (!IsStaticItem(itemID)) {
        // *most* dynamic items need proc tics.  add to proc list
        m_entityChanged = true;
        m_ticEntities[itemID] = pSE;
    } else {
        addSignal = false;
    }

    // Add Entity's Item Ref to Solar System Dynamic Inventory:
    m_solarSystemRef->AddItemToInventory(pSE->GetSelf());

    sBubbleMgr.Add(pSE);
    // add item to our AnomalyMgr
    if (addSignal)
        m_anomMgr->AddSignal(pSE);
}

void SystemManager::RemoveEntity(SystemEntity* pSE) {
    if (pSE == nullptr)
        return;
    sBubbleMgr.Remove(pSE);
    // Remove Entity's Item Ref from Solar System Dynamic Inventory:
    RemoveItemFromInventory(pSE->GetSelf());
    // remove entity from our maps
    uint32 itemID(pSE->GetID());
    m_entityChanged = true;
    m_entities.erase(itemID);
    m_ticEntities.erase(itemID);
    m_staticEntities.erase(itemID);

    // remove from anomaly map, if exists
    m_anomMgr->RemoveSignal(itemID);

    m_entityChanged = true;
}

void SystemManager::AddMarker(SystemEntity* pSE, bool sendBall/*false*/, bool addSignal/*false*/) {
    if (pSE == nullptr)
        return;

    m_entities[pSE->GetID()] = pSE;
    sBubbleMgr.Add(pSE);
    if (addSignal)
        m_anomMgr->AddSignal(pSE);
    if (sendBall) {
        // modified from SendStaticBall()
        if (m_clients.empty())
            return;

        Buffer* destinyBuffer = new Buffer();
        //create AddBalls header
        Destiny::AddBall_header head = Destiny::AddBall_header();
            head.packet_type = 1;   // 0 = full state   1 = balls
            head.stamp = sEntityMgr.GetStamp();
        destinyBuffer->Append( head );

        AddBalls2 addballs2;
            addballs2.stateStamp = sEntityMgr.GetStamp();
            addballs2.extraBallData = new PyList();

        PyTuple* balls = new PyTuple(2);
            balls->SetItem(0, pSE->MakeSlimItem());
            balls->SetItem(1, pSE->MakeDamageState());
        addballs2.extraBallData->AddItem(balls);

        pSE->EncodeDestiny(*destinyBuffer);

        addballs2.state = new PyBuffer(&destinyBuffer); //consumed
        SafeDelete( destinyBuffer );

        if (is_log_enabled(DESTINY__BALL_DUMP))
            addballs2.Dump( DESTINY__BALL_DUMP, "    " );
        //send the update
        PyTuple* up = addballs2.Encode();
        for (auto &cur : m_clients) {
            PyIncRef(up);
            cur.second->QueueDestinyUpdate(&up, true);
        }
    }
}


void SystemManager::AddBounty(uint32 charID, BountyData& data)
{
    /*
struct BountyData {     // this is coming from rat killed.
    uint32 fromID;
    uint32 toID;
    double amount;
    uint8 refTypeID;
    uint16 fromKey;
    uint16 toKey;
    std::string reason;  this can stay blank for now.  populate during PayBounties
}; */
    _log(CLIENT__TEXT, "AddBounty called for charID %u in system %s(%u).", charID, m_data.name.c_str(), m_data.systemID);
    std::map<uint32, BountyData>::iterator itr = m_bountyMap.find(charID);
    if (itr != m_bountyMap.end()) {
        itr->second.amount += data.amount;
        std::map<uint32, RatDataMap>::iterator rItr = m_ratMap.find(charID);
        if (rItr != m_ratMap.end()) {
            RatDataMap::iterator it = rItr->second.begin();
            if (it == rItr->second.end()){
                RatDataMap vec;
                vec.emplace(data.fromID, 1);
                rItr->second = vec;
            } else {
                ++(it->second);
            }
        } else {
            RatDataMap vec;
            vec.emplace(data.fromID, 1);
            m_ratMap.emplace(charID, vec);
        }
    } else {
        m_bountyMap.emplace(charID, data);
        RatDataMap vec;
        vec.emplace(data.fromID, 1);
        m_ratMap.emplace(charID, vec);
    }
}

void SystemManager::PayBounties()
{
    _log(CLIENT__TEXT, "PayBounties called for system %s(%u).", m_data.name.c_str(), m_data.systemID);
    int8 count = 0;
        /* recDescNpcBountyList = 'NBL'                <-- descrives a full list of [typeID: qty]
         * recDescNpcBountyListTruncated = 'NBLT'      <-- describes a truncated list
recDescription = 'DESC'
recDescNpcBountyList = 'NBL'
recDescNpcBountyListTruncated = 'NBLT'
recStoreItems = 'STOREITEMS'
         */

    /** @todo  this isnt right...which is why it's not working.
     * see code in client/script/ui/shared/neocom/wallet.py for more info.  (lots to look over. no time tonite.)
     * more data....
     *
     *   ["description" => <92:         << jnlRef:CorpBountyTax?
            - 1630077495                << total bounties paid on this timer
            - 325275.0                  << corp taxes taken?
            NBL:                        << see above
                11030: 2
                11935: 1
                23323: 1
                23332: 2
                23340: 2
            > [WStr]]
     *
     *
     *
     */
    for (auto &cur : m_bountyMap) {
        std::string reason = "NBLT: "; //this needs to be populated as [NBL(T): type:amt, type:amt, ... ] to get proper shit in client
        std::map<uint32, RatDataMap>::iterator itr = m_ratMap.find(cur.first);
        if (itr != m_ratMap.end()) {
            count = itr->second.size();
            for (auto &cur : itr->second) {
                reason += std::to_string(cur.first);
                reason += ":";
                reason += std::to_string(cur.second);
                if (count > 1)
                    reason += ",";
                --count;
            }
            // will have to figure out how to *correctly* limit this data to count<20 or so...
        } //else {
            reason += ",...";    // this will show as "truncated" in client
        //}
            AccountService::TransferFunds(corpCONCORD, cur.first, cur.second.amount, std::move(reason), Journal::EntryType::BountyPrizes, m_data.systemID);
    }
    m_ratMap.clear();
    m_bountyMap.clear();
}

void SystemManager::DoSpawnForBubble(SystemBubble* pBubble, uint8 type/*normal*/) {
    if (!m_spawnMgr->IsInitialized())
        return;

    if (m_beltCount < 1)
        return;

    if (is_log_enabled(SPAWN__MESSAGE))
        _log(SPAWN__MESSAGE, "Spawn called for bubble %u(%u) in %s(%u)[%.4f], region %u.", \
            pBubble->GetID(), sBubbleMgr.GetBeltID(pBubble->GetID()), m_data.name.c_str(), \
            m_data.systemID, m_data.security, m_data.regionID);

    uint8 error(0);
    switch (type) {
        case Bubble::Type::Normal: {
            // we're still not spawning anything in normal bubbles
            error = Bubble::Error::NotAllowed;
        } break;
        case Bubble::Type::Ice:
        case Bubble::Type::Belt: {
            // asteroid belts share active rats
            if (m_activeRatSpawns < m_beltCount) {
                error = m_spawnMgr->DoSpawnForBubble(pBubble);
            }
        } break;
        case Bubble::Type::Gate: {
            if (m_activeGateSpawns < m_gateCount) {
                error = m_spawnMgr->DoSpawnForBubble(pBubble);
            }
        } break;
        case Bubble::Type::Anomaly:
        case Bubble::Type::Mission:
        case Bubble::Type::Incursion:
        case Bubble::Type::Escalation: {
            error = m_spawnMgr->DoSpawnForBubble(pBubble);
        } break;
    }

    if (error == Bubble::Error::None) {
        m_ratBubbles[pBubble->GetID()] = pBubble;
        if (is_log_enabled(SPAWN__TRACE))
            _log(SPAWN__TRACE, "SystemManager::DoSpawnForBubble() completed for bubble %u in %s(%u).  %u/%u active belt/gate spawns.", \
                        pBubble->GetID(), m_data.name.c_str(), m_data.systemID, m_activeRatSpawns, m_activeGateSpawns);
    } else if (is_log_enabled(SPAWN__TRACE)) {
        std::string msg = "Error: ";
        switch (error) {
            case Bubble::Error::BubbleNull: {
                msg += "Bubble Null";
            } break;
            case Bubble::Error::BeltDisabled: {
                msg += "Belt Spawns Disabled";
            } break;
            case Bubble::Error::RoamingDisabled: {
                msg += "Roaming Spawns Disabled";
            } break;
            case Bubble::Error::StaticDisabled: {
                msg += "Static Spawns Disabled";
            } break;
            case Bubble::Error::Spawned: {
                msg += "Belt Already Spawned";
            } break;
            case Bubble::Error::PrepFail: {
                msg += "Spawn Prep Fail";
            } break;
            case Bubble::Error::NotAllowed: {
                msg += "Spawn Not Allowed in this Bubble";
            } break;
            default: {
                msg += "Undefined";
            } break;
        }
        _log(SPAWN__TRACE, "SystemManager::DoSpawnForBubble() returned false for bubble %u.  %s", \
                pBubble->GetID(), msg.c_str());
    }
}

// not used
void SystemManager::GetSpawnBubbles(SpawnBubbleMap &bubbleMap) {
    if (is_log_enabled(SPAWN__MESSAGE))
        _log(SPAWN__MESSAGE, "SystemManager::GetSpawnBubbles() - called for %s(%u)", m_data.name.c_str(), m_data.systemID);
    for (auto &cur : m_ratBubbles)
        bubbleMap[cur.first] = cur.second;
}

void SystemManager::RemoveSpawnBubble(SystemBubble* pBubble) {
    if (pBubble->IsBelt()) {
        --m_activeRatSpawns;
    } else if (pBubble->IsGate()) {
        --m_activeGateSpawns;
    }

    m_ratBubbles.erase(pBubble->GetID());

    if (is_log_enabled(SPAWN__MESSAGE))
        _log(SPAWN__MESSAGE, "SystemManager::RemoveSpawnBubble() - called for bubble %u in %s(%u).  %u belt and %u gate spawns remain.", \
                pBubble->GetID(), m_data.name.c_str(), m_data.systemID, m_activeRatSpawns, m_activeGateSpawns);
}

uint32 SystemManager::GetRandBeltID() {
    return m_beltVector.at(MakeRandomUInt(0, m_beltCount - 1));
}

void SystemManager::MakeSetState(const SystemBubble* pBubble,  SetState& into) const {
    using namespace Destiny;
    Buffer* stateBuffer(new Buffer());

    AddBall_header head = AddBall_header();
        head.packet_type = 0;   // 0 = full state   1 = balls
        head.stamp = into.stamp;
    stateBuffer->Append( head );

    // use map as it automagically orders by itemID
    std::map<uint32, SystemEntity*> visibleEntities;

    // get all static entities for this system
    for (auto &cur : m_staticEntities)
        visibleEntities.emplace(cur.first, cur.second);

    // get our ship.  bubble->GetEntities() does not include cloaked items
    std::map<uint32, SystemEntity*>::const_iterator itr = m_ticEntities.find(into.ego);
    if (itr != m_ticEntities.end())
       visibleEntities.emplace(itr->first, itr->second);

    // query bubble to get dynamic entities
    pBubble->GetEntities(visibleEntities);

    into.slims = new PyList();
    //into.slims->clear();
    into.effectStates = new PyList();
    //into.effectStates->clear();
    into.allianceBridges = new PyList();
    //into.allianceBridges->clear();  //activeBeacon and activeBridge data found in fleetSvc.py

    //go through all visible entities and gather the info we need...
    for (auto &cur : visibleEntities) {
        if (!cur.second->IsMissileSE() or !cur.second->IsFieldSE())
            into.damageState[ cur.first ] = cur.second->MakeDamageState();

        into.slims->AddItem( new PyObject( "foo.SlimItem", cur.second->MakeSlimItem()));

        //append the destiny binary data...
        cur.second->EncodeDestiny(*stateBuffer);

        // get tower effect state (if applicable)
        if (cur.second->IsTowerSE())
            cur.second->GetTowerSE()->GetEffectState(*(into.effectStates));

        /**  @todo (allan)  this needs more work.  should be done same as damageState.  28.2.16
        //ss.aggressors is for players undocking/jumping with aggression (uses GetCriminalTimeStamps)  ** see notes in Client::GetAggressors()
        if (cur.second->HasPilot() and cur.second->HasAggression())
            ss.aggressors[ cur.first ] = cur.second->GetAggressors());
        */

        /** @todo (allan)  to be written   -jumpbridges is a PyList */
        //  if (cur.second->IsJumpBridgeSE)
        //ss.allianceBridges -- jumpbridges et al.
        //  [for shipID, toSolarsystemID, toBeaconID in bag.allianceBridges:]
    }

    into.destiny_state = new PyBuffer( &stateBuffer );
    into.droneState = pBubble->GetDroneState(); //SystemDB::GetSolDroneState( m_data.systemID );

    /* SolarSystem info.  this avoids the old way of a DB hit for every call.  */
    PyPackedRow* row = new PyPackedRow(sDataMgr.CreateHeader());
        row->SetFieldC("itemID",        new PyLong(m_data.systemID));
        row->SetFieldC("typeID",        PyStatic.NewFive());
        row->SetFieldC("ownerID",       PyStatic.NewOne());  // should this be owning factionID?  yes
        row->SetFieldC("locationID",    new PyInt(m_data.constellationID));
        row->SetFieldC("flagID",        PyStatic.NewZero());
        row->SetFieldC("quantity",      PyStatic.NewNegOne());
        row->SetFieldC("groupID",       PyStatic.NewFive());
        row->SetFieldC("categoryID",    PyStatic.NewTwo());
        row->SetFieldC("customInfo",    new PyString(""));
    into.solItem = row;

    if (is_log_enabled(DESTINY__SETSTATE)) {
        _log( DESTINY__SETSTATE, "Current State of %s", m_data.name.c_str() );
        into.Dump( DESTINY__SETSTATE, "    " );
    }

    if (is_log_enabled(DESTINY__SETSTATE_DECODE)) {
        _log( DESTINY__SETSTATE_DECODE, "    Decoded:" );
        Destiny::DumpUpdate( DESTINY__SETSTATE_DECODE, &( into.destiny_state->content() )[0], (uint32)into.destiny_state->content().size() );
    }

    //cleanup
    SafeDelete(stateBuffer);
}

void SystemManager::SendStaticBall(SystemEntity* pSE)
{
    if (m_clients.empty())
        return;

    if (is_log_enabled(DESTINY__MESSAGE)) {
        if (pSE->SysBubble() != nullptr) { //Don't attempt to log if bubble is null (ie, on static structure initial launch)
            GPoint bCenter(pSE->SysBubble()->GetCenter());
            _log(DESTINY__MESSAGE, "SystemManager::SendStaticBall() - Adding static entity %s(%u) to bubble %u.  Dist to center: %.2f", \
            pSE->GetName(), pSE->GetID(), pSE->SysBubble()->GetID(), bCenter.distance(pSE->GetPosition()));
        }
    }

    Buffer* destinyBuffer = new Buffer();
    //create AddBalls header
    Destiny::AddBall_header head = Destiny::AddBall_header();
        head.packet_type = 1;   // 0 = full state   1 = balls
        head.stamp = sEntityMgr.GetStamp();
    destinyBuffer->Append( head );

    AddBalls2 addballs2;
    addballs2.stateStamp = sEntityMgr.GetStamp();
    addballs2.extraBallData = new PyList();

    if (pSE->IsContainerSE()) {
        addballs2.extraBallData->AddItem(pSE->MakeSlimItem());
    } else {
        PyTuple* balls = new PyTuple(2);
        balls->SetItem(0, pSE->MakeSlimItem());
        balls->SetItem(1, pSE->MakeDamageState());
        addballs2.extraBallData->AddItem(balls);
    }

    if (addballs2.extraBallData->size() < 1) {
        SafeDelete( destinyBuffer );
        return;
    }

    pSE->EncodeDestiny(*destinyBuffer);
    addballs2.state = new PyBuffer(&destinyBuffer); //consumed

    if (is_log_enabled(DESTINY__BALL_DUMP))
        addballs2.Dump( DESTINY__BALL_DUMP, "    " );
    //send the update
    PyTuple* rsp = addballs2.Encode();
    // does this need to be incremented?  the others do...
    for (auto &cur : m_clients) {
        PyIncRef(rsp);
        cur.second->QueueDestinyUpdate(&rsp, true);
    }

    //cleanup
    SafeDelete(destinyBuffer);
}

void SystemManager::AddItemToInventory(InventoryItemRef iRef) {
    m_solarSystemRef->AddItemToInventory( iRef );
}

void SystemManager::RemoveItemFromInventory(InventoryItemRef iRef) {
    // just in case this is called from elsewhere (which it may be), make sure we remove entity from our map.
    auto itr = m_entities.find(iRef->itemID());
    if (itr != m_entities.end()) {
        _log(ITEM__TRACE, "%s(%u): Removed from system manager for %s(%u)", iRef->name(), iRef->itemID(), m_data.name.c_str(), m_data.systemID);
        m_entities.erase(itr);
    } else {
        _log(ITEM__WARNING, "%s(%u): Called RemoveEntity(), but they weren\'t found in system manager for %s(%u)", \
                iRef->name(), iRef->itemID(), m_data.name.c_str(), m_data.systemID);
    }

    m_solarSystemRef->RemoveItemFromInventory( iRef );
}

SystemEntity* SystemManager::GetSE(uint32 entityID) const {
    std::map<uint32, SystemEntity*>::const_iterator itr = m_entities.find(entityID);
    if (itr == m_entities.end())
        return nullptr;
    return itr->second;
}

NPC* SystemManager::GetNPCSE(uint32 entityID) const {
    std::map<uint32, NPC*>::const_iterator itr = m_npcs.find(entityID);
    if (itr == m_npcs.end())
        return nullptr;
    return itr->second;
}

ShipItemRef SystemManager::GetShipFromInventory(uint32 shipID) {
    return ShipItemRef::StaticCast( m_solarSystemRef->GetMyInventory()->GetByID( shipID ) );
}

CargoContainerRef SystemManager::GetContainerFromInventory(uint32 contID) {
    return CargoContainerRef::StaticCast( m_solarSystemRef->GetMyInventory()->GetByID( contID ) );
}

StationItemRef SystemManager::GetStationFromInventory(uint32 stationID) {
    return StationItemRef::StaticCast( m_solarSystemRef->GetMyInventory()->GetByID( stationID ) );
}

PlanetSE* SystemManager::GetPlanet(uint32 planetID) {
    std::map<uint32, SystemEntity*>::iterator itr = m_planetMap.find(planetID);
    if (itr != m_planetMap.end())
        return itr->second->GetPlanetSE();
    return nullptr;
}

uint32 SystemManager::GetClosestPlanetID(const GPoint& myPos) {
    std::map<double, SystemEntity*> sorted;
    for (auto &cur : m_planetMap)
        sorted.insert(std::pair<double, SystemEntity*>(myPos.distance(cur.second->GetPosition()), cur.second));

    std::map<double, SystemEntity*>::iterator itr = sorted.begin();

    return itr->second->GetID();
}

PlanetSE* SystemManager::GetClosestPlanetSE(const GPoint& myPos) {
    std::map<double, SystemEntity*> sorted;
    for (auto &cur : m_planetMap)
        sorted.insert(std::pair<double, SystemEntity*>(myPos.distance(cur.second->GetPosition()), cur.second));

    std::map<double, SystemEntity*>::iterator itr = sorted.begin();

    return itr->second->GetPlanetSE();
}

StargateSE* SystemManager::GetClosestGateSE(const GPoint& myPos) {
    std::map<double, SystemEntity*> sorted;
    for (auto &cur : m_gateMap)
        sorted.insert(std::pair<double, SystemEntity*>(myPos.distance(cur.second->GetPosition()), cur.second));

    std::map<double, SystemEntity*>::iterator itr = sorted.begin();

    return itr->second->GetGateSE();
}

MoonSE* SystemManager::GetClosestMoonSE(const GPoint& myPos) {
    std::map<double, SystemEntity*> sorted;
    for (auto &cur : m_moonMap)
        sorted.insert(std::pair<double, SystemEntity*>(myPos.distance(cur.second->GetPosition()), cur.second));

    std::map<double, SystemEntity*>::iterator itr = sorted.begin();

    return itr->second->GetMoonSE();
}

void SystemManager::GetClientList(std::vector< Client* >& cVec) {
    for (auto &cur : m_clients)
        cVec.push_back(cur.second);
}

SystemEntity* SystemManager::GetEntityByID(uint32 itemID) {
    // only used by Cmd_Inventory()
    if (m_entities.find(itemID) == m_entities.end())
        return nullptr;

    return m_entities.find(itemID)->second;
}

void SystemManager::DScan(int64 range, const GPoint& position, std::vector<SystemEntity*>& vector )
{
    /** @todo finish this for correct dscan entity reporting
     * all ships (not cloaked)
     * all celestials
     * all structures
     * wrecks
     * drones
     * probes (core and combat only, i think)
     * spheres and bubbles (ewar shit)
     * some npcs ('normal' like rats dont show)
     * clouds(groups 227 & 711) and asteroids are coded in client but not included here (yet)
     *   group 227 only shown if role_mod or role_gml
     *
     * may not be in this version, but check for "scan inhibitor" POS module; ships in it are invis to dscan
     * AttrDScanImmune is from rhea expansion.  may be able to implement here.
     */

    //  TO CHECK...do scan settings alter this or is entirely client-side?
    for (auto &cur : m_entities) {
        // these dont show on dscan
        if (IsTempItem(cur.first))
            continue;
        if (IsAsteroidID(cur.first))
            if (!sConfig.server.AsteroidsOnDScan)
                continue;
        if (IsNPC(cur.first))
            continue;
        if (cur.second->IsDeployableSE())       // not sure if this is right or not
            continue;
        if (cur.second->IsShipSE()) {
            if (cur.second->GetGroupID() == EVEDB::invGroups::CovertOps)
                continue;
            if (cur.second->GetGroupID() == EVEDB::invGroups::CombatRecon)
                continue;
        }
        if (cur.second->DestinyMgr() != nullptr)
            if (cur.second->DestinyMgr()->IsCloaked())
                continue;
        // made it this far.  add item to scan list
        if (position.distance(cur.second->GetPosition()) < range)
            vector.push_back(cur.second);
    }
}

PyRep* SystemManager::GetCurrentEntities() {
    /*  return list of dict
     * itemID, typeID, catID, name, pos[x,y,z]
     *
     * already have statics, so add players, empty ships, pos', npcs, drones  (anything that requires a tic)
     */

    PyList* list = new PyList();
    for (auto &cur : m_ticEntities) {
        PyDict* dict = new PyDict();
            dict->SetItemString("itemID", new PyInt(cur.first));
            dict->SetItemString("ownerName", new PyString(sDataMgr.GetOwnerName(cur.second->GetOwnerID())));
            dict->SetItemString("typeID", new PyInt(cur.second->GetTypeID()));
            dict->SetItemString("catID", new PyInt(cur.second->GetCategoryID()));
            dict->SetItemString("name", new PyString(cur.second->GetName()));
            dict->SetItemString("x", new PyLong(cur.second->x()));
            dict->SetItemString("y", new PyLong(cur.second->y()));
            dict->SetItemString("z", new PyLong(cur.second->z()));
        list->AddItem(dict);
    }
    return list;
}

void SystemManager::GetAllEntities(std::vector< CosmicSignature >& vector) {
    /** @todo this will need to put entity's sigID into anomaly map for Scan::WarpTo object */
    /** @todo this should be updated/current/correct in system's AnomalyMgr.  try to get data from there for this list  */
    for (auto &cur : m_ticEntities) {
        CosmicSignature sig = CosmicSignature();
        sig.bubbleID = 0;
        sig.dungeonType = Dungeon::Type::Anomaly;
        sig.ownerID = cur.second->GetOwnerID();
        sig.sigID = sEntityMgr.GetAnomalyID();         // result.id
        sig.sigItemID = cur.first;
        sig.sigStrength = 0.9f; // these arent warpable yet
        sig.systemID = m_data.systemID;
        sig.position = cur.second->GetPosition();
        sig.sigGroupID = cur.second->GetGroupID();      // result.groupID
        sig.sigTypeID = cur.second->GetTypeID();        // result.typeID
        // if scanGroupID is anom or sig, use scanAttributeID to determine site type (in client code)
        // scanGroupID must be one of the 5 groups coded in client (sig, anom, ship, drone, structure)
        // scanGroupID of sig and anom are cached on client side
        switch (cur.second->GetCategoryID()) {
            case EVEDB::invCategories::Drone:
            case EVEDB::invCategories::Charge: { // probes, missiles (at time of scan), and ??
                sig.scanAttributeID = AttrScanStrengthDronesProbes;   // result.strengthAttributeID
                sig.scanGroupID = Scanning::Group::DroneOrProbe;
            } break;
            case EVEDB::invCategories::Orbitals:
            case EVEDB::invCategories::Structure:
            case EVEDB::invCategories::StructureUpgrade:
            case EVEDB::invCategories::SovereigntyStructure: {
                sig.scanAttributeID = AttrScanStrengthStructures;   // result.strengthAttributeID
                sig.scanGroupID = Scanning::Group::Structure;
            } break;
            case EVEDB::invCategories::Ship: {
                sig.scanAttributeID = AttrScanStrengthShips;   // result.strengthAttributeID
                sig.scanGroupID = Scanning::Group::Ship;
            } break;
            case EVEDB::invCategories::Entity: {
                sig.scanAttributeID = AttrScanStrengthSignatures;       // result.strengthAttributeID
                sig.scanGroupID = Scanning::Group::Signature;    // Scrap(1) is for filter only
                sig.sigName = cur.second->GetName(); // result.DungeonName  -  only used when scanGroupID is sig or anom
            } break;
            case EVEDB::invCategories::Asteroid:
            case EVEDB::invCategories::Celestial:
            case EVEDB::invCategories::Deployable: // mobile warp disruptor
            default: {
                sig.scanAttributeID = AttrScanAllStrength;     // result.strengthAttributeID (Unknown)
                sig.scanGroupID = Scanning::Group::Anomaly; // Celestial(64) is only for filter
                sig.sigName = cur.second->GetName(); // result.DungeonName  -  only used when scanGroupID is sig or anom
            } break;
        }
        vector.push_back(sig);
    }
}


//  time related methods to manipulate hour/24hour map data
void SystemManager::UpdateData() {
    MapDB::UpdatePilotCount(m_data.systemID, m_docked, (m_players - m_docked));

    uint16 jumps = 0;
    uint16 stamp = sEntityMgr.GetStamp() - 60;
    std::map<uint32, uint8>::iterator itr = m_jumpMap.begin();
    while (itr != m_jumpMap.end()) {
        if (itr->first < stamp) {
            itr = m_jumpMap.erase(itr);
            continue;
        } else {
            jumps += itr->second;
            ++itr;
        }
    }

    // this is jumps/hour data
    //MapDB::UpdateJumps(m_data.systemID, jumps);

    // if system and jumpmap are both empty, set activity time for unload timer
    if (SafeToUnload())
        if (m_activityTime == 0)
            if (m_clients.empty())
                if (m_jumpMap.empty())
                    m_activityTime = sEntityMgr.GetStamp() - 50;

    // this needs work....current profile shows ~2s time on current code
    ManipulateTimeData();
}

// checks for if it is safe to mark the system for unloading
bool SystemManager::SafeToUnload() {
    for (auto &cur: GetOperationalStatics()) {
        //If there are any ongoing operations by operational static structures, we don't want to unload the system until this is complete
        if (cur.second->IsPOSSE())
            if (cur.second->GetPOSSE()->GetProcState() < EVEPOS::ProcState::Online)
                return false;
    }
    return true; //by default, its always safe to unload
}

void SystemManager::ResetAsteroids() {
    // player command to remove all asteroids in a system (roid reset)
    m_beltMgr->ClearAll(true);
    /*
    std::map<uint32, SystemEntity*>::iterator itr = m_entities.begin();
    SystemEntity* pSE(nullptr);
    while (itr != m_entities.begin()) {
        if (IsAsteroidID(itr->first)) {
            pSE = itr->second;
            sBubbleMgr.Remove(pSE);
            //RemoveItemFromInventory(pSE->GetSelf());
            pSE->GetSelf()->Delete();
            itr = m_entities.erase(itr);
        } else {
            ++itr;
        }
    }
    m_entityChanged = true;
    */
}

void SystemManager::UpdateContainerFleetID(uint32 ownerID, uint32 fleetID) {
    // this shouldnt hit very often, but may have a ton of objects to update...
    for (auto &cur : m_ticEntities) {
        if (cur.second->IsContainerSE() or cur.second->IsWreckSE()) {
            if (cur.second->GetOwnerID() == ownerID) {
                cur.second->SetFleetID(fleetID);
                //TODO:  this doesnt work right
                cur.second->MakeSlimItemChange();
            }
        }
    }
}


// not sure how to do this one yet...
void SystemManager::ManipulateTimeData()
{
    int64 timeNow = GetFileTimeNow();
    timeNow += EvE::Time::Hour;

    //if (m_killData.killsDateTime < timeNow)

    // disabled for now...crazy profile times from this....
    /** @todo ...
     * 10:40:07 W   Profile Manager: Long Profile Time on key DB, time 1960.498ms.  <<-- !!!  2 sec???
     * backtrace() returned 11 addresses
     * /srv/games/eve/Alasiya-EvE/bin/eve-server(_ZN3EvE10traceStackEv+0x27) [0x1185e22]
     * /srv/games/eve/Alasiya-EvE/bin/eve-server(_ZN8Profiler7AddTimeEhd+0x100) [0xadcb7a]
     * /srv/games/eve/Alasiya-EvE/bin/eve-server(_ZN6DBcore14DoQuery_lockedER7DBerrorPKcib+0x313) [0x117d0fb]
     * /srv/games/eve/Alasiya-EvE/bin/eve-server(_ZN6DBcore8RunQueryER7DBerrorPKcz+0x106) [0x117ca60]
     * /srv/games/eve/Alasiya-EvE/bin/eve-server(_ZN5MapDB14UpdateKillDataEjR14SystemKillData+0x11d) [0xd96d05]
     * /srv/games/eve/Alasiya-EvE/bin/eve-server(_ZN13SystemManager18ManipulateTimeDataEv+0x41) [0xf865af]
     * /srv/games/eve/Alasiya-EvE/bin/eve-server(_ZN13SystemManager10UpdateDataEv+0x17e) [0xf863be]
     * /srv/games/eve/Alasiya-EvE/bin/eve-server(_ZN10EntityMgr7ProcessEv+0x5e6) [0xabfcc2]
     *
     * 10:50:08 W   Profile Manager: Long Profile Time on key DB, time 2135.552ms.
     * backtrace() returned 11 addresses
     * /srv/games/eve/Alasiya-EvE/bin/eve-server(_ZN3EvE10traceStackEv+0x27) [0x1185e22]
     * /srv/games/eve/Alasiya-EvE/bin/eve-server(_ZN8Profiler7AddTimeEhd+0x100) [0xadcb7a]
     * /srv/games/eve/Alasiya-EvE/bin/eve-server(_ZN6DBcore14DoQuery_lockedER7DBerrorPKcib+0x313) [0x117d0fb]
     * /srv/games/eve/Alasiya-EvE/bin/eve-server(_ZN6DBcore8RunQueryER7DBerrorPKcz+0x106) [0x117ca60]
     * /srv/games/eve/Alasiya-EvE/bin/eve-server(_ZN5MapDB14UpdateKillDataEjR14SystemKillData+0x11d) [0xd96d05]
     * /srv/games/eve/Alasiya-EvE/bin/eve-server(_ZN13SystemManager18ManipulateTimeDataEv+0x41) [0xf865af]
     * /srv/games/eve/Alasiya-EvE/bin/eve-server(_ZN13SystemManager10UpdateDataEv+0x17e) [0xf863be]
     * /srv/games/eve/Alasiya-EvE/bin/eve-server(_ZN10EntityMgr7ProcessEv+0x5e6) [0xabfcc2]
     *
     */

    //MapDB::UpdateKillData(m_data.systemID, m_killData);
}

void SystemManager::GetDockedCount()
{
    m_docked = 0;
    for (auto &cur : m_clients)
        if (cur.second->IsDocked())
            ++m_docked;
}

void SystemManager::GetPlayerCount()
{
    // nothing here yet.  wip
}

/*
    uint16 killsHour;
    uint16 kills24Hour;
    uint16 factionKills;
    uint16 factionKills24Hour;
    uint16 podKillsHour;
    uint16 podKills24Hour;

    int64 killsDateTime;
    int64 kills24DateTime;
    int64 factionDateTime;
    int64 faction24DateTime;
    int64 podDateTime;
    int64 pod24DateTime;
    */


bool SystemManager::IsNull(std::map<uint32, SystemEntity*>::iterator& i)
{
    /* you have to change this parameter to the type of the container
     * you are using and the type of the element inside the container.
     *   if this finds use again, it should be changed to a template.
     */

    uint8 buffer[sizeof(i)];
    memset(buffer, 0, sizeof(i));
    memcpy(buffer, &i, sizeof(i));
    return *buffer == 0;
    /* I found that the size of any iterator is 12 bytes long.
     * I also found that if the first byte of the iterator that
     * is copy to the buffer is zero, then the iterator is invalid.
     * Otherwise it is valid. I like to call invalid iterators also as "null iterators".
     */
}
