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

#include "eve-server.h"

#include "Client.h"
#include "EVEServerConfig.h"
#include "Profile.h"
#include "account/AccountService.h"
#include "chat/LSCService.h"
#include "npc/NPC.h"
#include "planet/Planet.h"
#include "planet/Moon.h"
#include "pos/Array.h"
#include "pos/Battery.h"
#include "pos/Module.h"
#include "pos/Structure.h"
#include "pos/Tower.h"
#include "pos/Weapon.h"
#include "npc/Drone.h"
#include "npc/Sentry.h"
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
#include "system/cosmicMgrs/DungeonMgr.h"
#include "system/cosmicMgrs/SpawnMgr.h"


SystemManager::SystemManager(uint32 systemID, PyServiceMgr &svc)
:m_services(svc),
m_bountyTimer(20 * 60 * 1000),      // 20m  default
m_anomMgr(new AnomalyMgr(this, svc)),
m_beltMgr(new BeltMgr(this, svc)),
m_dunMgr(new DungeonMgr(this, svc)),
m_spawnMgr(new SpawnMgr(this, svc))
{
    m_loaded = false;
    m_entityChanged = false;

    m_players = 0;
    m_beltCount = 0;
    m_gateCount = 0;
    m_activityTime = 0;
    m_activeRatSpawns = 0;
    m_activeGateSpawns = 0;
    m_activeRoidSpawns = 0;

    m_npcs.clear();
    m_clients.clear();
    m_moonMap.clear();
    m_entities.clear();
    m_planetMap.clear();
    m_ratBubbles.clear();
    m_beltVector.clear();
    m_roidBubbles.clear();
    m_ticEntities.clear();
    m_staticEntities.clear();

    m_bountyTimer.Disable();

    m_secValue = 1.1 - GetSystemSecurityRating();  // range is 0.1 for 1.0 system to 2.0 for -0.9 system

    sDataMgr.GetSystemInfo(systemID, m_data);   // system data is now an internal memory (cached) object.  db is hit once at system boot.
    _log(COMMON__MESSAGE, "Created SystemManager %p for System %s(%u)", this, m_data.name.c_str(), m_data.systemID);
}

SystemManager::~SystemManager() {
    if (m_players or !m_clients.empty()) {
        _log(COMMON__ERROR, "D'tor called for System %u with %u players and/or %u clients in mmaps", m_data.systemID, m_players, m_clients.size());
        for (auto cur : m_clients)
            sEntityList.Remove(cur.second);
    }

    if (m_loaded)
        UnloadSystem();

    m_npcs.clear();
    m_clients.clear();
    m_moonMap.clear();
    m_entities.clear();
    m_planetMap.clear();
    m_ratBubbles.clear();
    m_beltVector.clear();
    m_roidBubbles.clear();
    m_ticEntities.clear();

    SafeDelete(m_dunMgr);
    SafeDelete(m_anomMgr);
    SafeDelete(m_beltMgr);
    SafeDelete(m_spawnMgr);
}

static const int num_hack_sentry_locs = 8;
GPoint hack_sentry_locs[num_hack_sentry_locs] = {
    GPoint(35000.0f, 35000.0f, 35000.0f),
    GPoint(35000.0f, 35000.0f, -35000.0f),
    GPoint(35000.0f, -35000.0f, 35000.0f),
    GPoint(35000.0f, -35000.0f, -35000.0f),
    GPoint(-35000.0f, 35000.0f, 35000.0f),
    GPoint(-35000.0f, 35000.0f, -35000.0f),
    GPoint(-35000.0f, -35000.0f, 35000.0f),
    GPoint(-35000.0f, -35000.0f, -35000.0f)
};

bool SystemManager::BootSystem() {
    m_solarSystemRef = sItemFactory.GetSolarSystem(m_data.systemID);
    assert(m_solarSystemRef.get() != nullptr);

    if (!LoadCosmicMgrs())
        return false;

    if (!LoadSystemStatics()) {
        _log(SERVICE__ERROR, "Unable to load System Statics during boot of system %u.", m_data.systemID);
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

    // system is loaded.  check for items that need initialization
    for (auto cur : m_ticEntities)
        if (cur.second->IsPOSSE())
            cur.second->GetPOSSE()->Init(cur.second->GetSelf(), cur.second->SysBubble());

    if (sConfig.server.BountyPayoutDelayed)
        m_bountyTimer.Start(sConfig.server.BountyPayoutTimer * 60 * 1000);

    //create our chat channels
    m_services.lsc_service->CreateSystemChannel(m_data.regionID);
    m_services.lsc_service->CreateSystemChannel(m_data.constellationID);
    m_services.lsc_service->CreateSystemChannel(m_data.systemID);

    return (m_loaded = true);
}

//called once per second from EntityList. (1Hz Tic)
bool SystemManager::ProcessTic() {
    double profileStartTime = GetTimeUSeconds();

    /* the idea here is entities map NEVER has invalid items in it, but our iterator may become invalid when SE->Process() returns
     *      because Process() will add/remove from the map as needed (new objects, destroyed objects, moved objects, etc)
     *  std::map internally orders items by key(itemID here), so use an int var to hold last-processed itemID (mLast).
     *  when iteration starts over, increment until cur > mLast and continue from there to end of list.
     */
    std::map<uint32, SystemEntity*>::iterator itr = m_ticEntities.begin();
    uint32 mLast = 0;
    while (itr != m_ticEntities.end()) {
        if (mLast >= itr->first) {
            ++itr;
            continue;
        }
        itr->second->Process(); /* main process call. */
        if (m_entityChanged) {
            mLast = itr->first;
            m_entityChanged = false;
            itr = m_ticEntities.begin();
            continue;
        }
        ++itr;
    }

    // check bounty timer
    if (m_bountyTimer.Check(sConfig.server.BountyPayoutDelayed))
        PayBounties();
    /* the following are coded for single-tic calls */
    m_anomMgr->Process();
    m_beltMgr->Process();
    m_spawnMgr->Process();

    if (sConfig.debug.UseProfiling)
        sProfile.AddTime(_systemProfile, GetTimeUSeconds() - profileStartTime);

    return SystemActivity();
}

bool SystemManager::SystemActivity() {
    if (m_activityTime == 0)
        return true;
    if (sConfig.world.gridUnload)
        if (m_players < 1)
            if (sConfig.world.gridUnloadTime < (sEntityList.GetStamp() - m_activityTime))
                return false;

    return true;
}

// called from EntityList::Process() and EntityList::Close()
void SystemManager::UnloadSystem() {
    if (!m_loaded)
        return;

    sLog.Magenta("    SystemManager", "UnloadSystem() called for %s(%u).", GetName().c_str(), m_data.systemID);

    // system is being unloaded.  pay bounties now
    PayBounties();
    m_beltMgr->ClearAll();

    std::map<uint32, SystemEntity*>::iterator itr = m_entities.begin();
    while (itr != m_entities.end()) {
        if ((itr->first > 0) and (itr->second != nullptr)) {
            if (itr->second->IsStationSE()) {
                itr->second->GetStationSE()->UnloadStation();
                sEntityList.RemoveStation(itr->first);
            } else if (itr->second->IsNPCSE()) {
                sEntityList.RemoveNPC();    // this is for loaded npc count.
                itr->second->GetNPCSE()->RemoveNPC();
            }
            sBubbleMgr.Remove(itr->second);
            if (itr->second->TargetMgr() != nullptr)
                itr->second->TargetMgr()->ClearAllTargets(false);
            m_solarSystemRef->RemoveItemFromInventory( itr->second->GetSelf() );
        }

        sItemFactory.RemoveItem(itr->first);
        SafeDelete(itr->second);
        itr = m_entities.erase(itr);
        m_entityChanged = true;
    }
    // at this point, system entity list should be clear...but just in case, hit it again
    m_npcs.clear();
    m_entities.clear();
    m_ticEntities.clear();
    m_staticEntities.clear();

    // this still needs some work...
    sBubbleMgr.ClearSystemBubbles(m_data.systemID);
    // save items, then remove from system inventory, item factory and decrement item count
    m_solarSystemRef->GetMyInventory()->Unload();

    /** @todo finish this */
    m_services.lsc_service->SystemUnload(m_data.systemID, m_data.constellationID, m_data.regionID);
    m_loaded = false;
}

bool SystemManager::LoadCosmicMgrs()
{
    if (!m_spawnMgr->Init()) {
        _log(SERVICE__ERROR, "Unable to load Spawn Manager during boot of system %u.", m_data.systemID);
        return false;
    }

    if (!m_dunMgr->Init(m_anomMgr, m_spawnMgr)) {
        _log(SERVICE__ERROR, "Unable to load Dungeon Manager during boot of system %u.", m_data.systemID);
        return false;
    }

    m_beltMgr->Init(m_data.regionID);  //nothing to check for in this init.

    if (!m_anomMgr->Init(m_beltMgr, m_dunMgr, m_spawnMgr)) {
        _log(SERVICE__ERROR, "Unable to load Anomaly Manager during boot of system %u.", m_data.systemID);
        return false;
    }

    return true;
}

bool SystemManager::LoadSystemStatics() {
    std::vector<DBSystemEntity> entities;
    entities.clear();
    m_entities.clear();
    m_staticEntities.clear();
    if (!SystemDB::LoadSystemStaticEntities(m_data.systemID, entities)) {
        _log(INV__ERROR, "Unable to load celestial entities during boot of system %u.", m_data.systemID);
        return false;
    }

    SystemEntity* pSE(nullptr);
    for (auto cur : entities) {
        switch (cur.groupID) {
            case EVEDB::invGroups::Station: {
                /** @todo (Allan) outposts are group::station - may need to hack this */
                /*  types 12242 - 22298 in group 15 are outposts */
                /*  types 29323 - 29390 in group 15 are wrecked stations */
                StationItemRef itemRef = sItemFactory.GetStation(cur.itemID);
                StationSE *pSSE = new StationSE(itemRef, *(GetServiceMgr()), this);
                sEntityList.AddStation(cur.itemID, itemRef);
                pSE = pSSE;
            } break;
            case EVEDB::invGroups::Asteroid_Belt: {
                CelestialObjectRef itemRef = sItemFactory.GetCelestialObject(cur.itemID);
                BeltSE *pBSE = new BeltSE(itemRef, *(GetServiceMgr()), this);
                pBSE->SetBeltMgr(m_beltMgr);
                ++m_beltCount;
                pSE = pBSE;
            } break;
            case EVEDB::invGroups::Stargate: {
                CelestialObjectRef itemRef = sItemFactory.GetCelestialObject(cur.itemID);
                itemRef->SetAttribute(AttrRadius, cur.radius);
                StargateSE *pSSE = new StargateSE(itemRef, *(GetServiceMgr()), this);
                ++m_gateCount;
                pSE = pSSE;
            } break;
            case EVEDB::invGroups::Planet: {
                CelestialObjectRef itemRef = sItemFactory.GetCelestialObject(cur.itemID);
                itemRef->SetAttribute(AttrRadius, cur.radius);
                PlanetSE *pPSE = new PlanetSE(itemRef, *(GetServiceMgr()), this);
                m_planetMap.insert(std::pair<uint32, SystemEntity*>(cur.itemID, pPSE));
                pSE = pPSE;
            } break;
            case EVEDB::invGroups::Moon: {
                CelestialObjectRef itemRef = sItemFactory.GetCelestialObject(cur.itemID);
                itemRef->SetAttribute(AttrRadius, cur.radius);
                MoonSE *pMSE = new MoonSE(itemRef, *(GetServiceMgr()), this);
                m_moonMap.insert(std::pair<uint32, SystemEntity*>(cur.itemID, pMSE));
                pSE = pMSE;
            } break;
            default: /*sun*/ {    // suns dont have anything special, so they are generic StaticSystemEntitys
                CelestialObjectRef itemRef = sItemFactory.GetCelestialObject(cur.itemID);
                itemRef->SetAttribute(AttrRadius, cur.radius);
                StaticSystemEntity *pSSE = new StaticSystemEntity(itemRef, *(GetServiceMgr()), this);
                pSE = pSSE;
            } break;
        }
        if (pSE == nullptr) {
            _log(INV__WARNING, "Failed to create entity for item %u (type %u)", cur.itemID, cur.typeID);
            continue;
        }
        if (pSE->IsGateSE() or pSE->IsStationSE())
            sBubbleMgr.Add(pSE);
        if (pSE->IsBeltSE()) {
            sBubbleMgr.Add(pSE);
            m_beltVector.push_back(cur.itemID);
        }
        if (!pSE->LoadExtras())
            _log(INV__WARNING, "Failed to load additional data for entity %u. Continuing.", cur.itemID);

        m_entities[cur.itemID] = pSE;
        m_staticEntities[cur.itemID] = pSE;
        AddItemToInventory(pSE->GetSelf());
    }

    if (m_entities.size())
        m_entityChanged = true;

    _log(SERVER__INIT, "%u Static System entities loaded for system %u", entities.size(), m_data.systemID);
    entities.clear();
    return m_entityChanged;
}

bool SystemManager::LoadSystemDynamics() {
    std::vector<DBSystemDynamicEntity> entities;
    entities.clear();
    if (!SystemDB::LoadSystemDynamicEntities(m_data.systemID, entities)) {
        _log(SERVICE__ERROR, "Unable to load dynamic entities during boot of system %u.", m_data.systemID);
        return false;
    }

    SystemEntity* pSE(nullptr);
    for (auto cur : entities) {
        pSE = DynamicEntityFactory::BuildEntity(*this, cur);
        if (pSE == nullptr) {
            _log(ITEM__WARNING, "LoadSystemDynamics() Failed to create entity for item %u (type %u)", cur.itemID, cur.typeID);
            continue;
        }
        _log(ITEM__TRACE, "SystemManager::LoadSystemDynamics() - Loaded dynamic entity %u of type %u for system %u", cur.itemID, cur.typeID, m_data.systemID);
        AddEntity(pSE);
    }

    return true;
}

bool SystemManager::LoadPlayerDynamics() {
    std::vector<DBSystemDynamicEntity> entities;
    entities.clear();
    if (!SystemDB::LoadPlayerDynamicEntities(m_data.systemID, entities)) {
        _log(SERVICE__ERROR, "Unable to load player dynamic entities in system %u.", m_data.systemID);
        return false;
    }

    SystemEntity* pSE(nullptr);
    for (auto cur : entities) {
        pSE = DynamicEntityFactory::BuildEntity(*this, cur);
        if (pSE == nullptr) {
            _log(ITEM__WARNING, "LoadSystemDynamics() Failed to create entity for item %u (type %u)", cur.itemID, cur.typeID);
            continue;
        }
        _log(ITEM__TRACE, "SystemManager::LoadPlayerDynamics() - Loaded dynamic entity %u of type %u for system %u", cur.itemID, cur.typeID, m_data.systemID);
        AddEntity(pSE);
    }

    return true;
}

bool SystemManager::BuildDynamicEntity(const DBSystemDynamicEntity& entity) {
    SystemEntity* pSE = DynamicEntityFactory::BuildEntity(*this, entity );
    if (pSE == nullptr) {
        sLog.Error( "SystemManager::BuildDynamicEntity()", "Failed to create entity for item %u (type %u)", entity.itemID, entity.typeID );
        return false;
    }

    _log(ITEM__TRACE, "SystemManager::BuildDynamicEntity() - Created dynamic entity %u of type %u for system %u", entity.itemID, entity.typeID, m_data.systemID );
    AddEntity(pSE);
    return true;
}

/** @todo  this needs updating with better/correct checks */
SystemEntity* DynamicEntityFactory::BuildEntity(SystemManager& system, const DBSystemDynamicEntity& entity)
{
    FactionData data;
        data.allianceID = entity.allianceID;
        data.corporationID = entity.corporationID;
        data.factionID = entity.factionID;
        data.ownerID = entity.ownerID;

    switch (entity.categoryID) {
        case EVEDB::invCategories::Asteroid: {
            InventoryItemRef asteroid = sItemFactory.GetItem( entity.itemID );
            if (asteroid.get() == nullptr)
                ; /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
            AsteroidSE* aSE = new AsteroidSE(asteroid, *(system.GetServiceMgr()), &system);
            _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making AsteroidSE item for %s (%u)", entity.itemName.c_str(), entity.itemID);
            return aSE;
        } break;
        case EVEDB::invCategories::Ship: {
            ShipItemRef ship = sItemFactory.GetShip( entity.itemID );
            if (ship.get() == nullptr)
                return nullptr;
            /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
            Ship* sSE = new Ship(ship, *(system.GetServiceMgr()), &system, data);
            _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making Ship item for %s (%u)", entity.itemName.c_str(), entity.itemID);
            return sSE;
        } break;
        case EVEDB::invCategories::Deployable: {
            InventoryItemRef deployable = sItemFactory.GetItem( entity.itemID );
            if (deployable.get() == nullptr)
                return nullptr;
            /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
            deployable->SetAttribute(AttrRadius, deployable->type().radius());     // Can you set this somehow from the type class ?
            DeployableSE* dSE = new DeployableSE(deployable, *(system.GetServiceMgr()), &system, data);
            _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making DeployableSE item for %s (%u)", entity.itemName.c_str(), entity.itemID);
            return dSE;
        } break;
        case EVEDB::invCategories::StructureUpgrade:
        case EVEDB::invCategories::SovereigntyStructure:// SOV structures   these should go into m_staticEntities
        case EVEDB::invCategories::Orbitals:            // planet orbitals   these should go into m_staticEntities
        case EVEDB::invCategories::Structure: {         // POS items
            StructureItemRef structure = sItemFactory.GetStructure( entity.itemID );
            if (structure.get() == nullptr)
                return nullptr;
            /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
            StructureSE* pSSE(nullptr);
            switch(entity.groupID) {
                case EVEDB::invGroups::Control_Tower: {
                    TowerSE* tSE = new TowerSE(structure, *(system.GetServiceMgr()), &system, data);
                    _log(POS__TRACE, "DynamicEntityFactory::BuildEntity() making TowerSE item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    pSSE = tSE;
                } break;
                case EVEDB::invGroups::Mobile_Missile_Sentry:
                case EVEDB::invGroups::Mobile_Projectile_Sentry:
                case EVEDB::invGroups::Mobile_Laser_Sentry:
                case EVEDB::invGroups::Mobile_Hybrid_Sentry: {
                    WeaponSE* wSE = new WeaponSE(structure, *(system.GetServiceMgr()), &system, data);
                    _log(POS__TRACE, "DynamicEntityFactory::BuildEntity() making WeaponSE item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    pSSE =  wSE;
                } break;
                case EVEDB::invGroups::Electronic_Warfare_Battery:
                case EVEDB::invGroups::Sensor_Dampening_Battery:
                case EVEDB::invGroups::Stasis_Webification_Battery:
                case EVEDB::invGroups::Warp_Scrambling_Battery:
                case EVEDB::invGroups::Energy_Neutralizing_Battery:
                case EVEDB::invGroups::Target_Painting_Battery: {
                    BatterySE* bSE = new BatterySE(structure, *(system.GetServiceMgr()), &system, data);
                    _log(POS__TRACE, "DynamicEntityFactory::BuildEntity() making BatterySE item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    pSSE = bSE;
                } break;
                case EVEDB::invGroups::Refining_Array:
                case EVEDB::invGroups::Ship_Maintenance_Array:
                case EVEDB::invGroups::Assembly_Array:
                case EVEDB::invGroups::Shield_Hardening_Array:
                case EVEDB::invGroups::Force_Field_Array:         // created based on tower status...not checked here (never hits)
                case EVEDB::invGroups::Corporate_Hangar_Array:
                case EVEDB::invGroups::Stealth_Emitter_Array:
                case EVEDB::invGroups::Scanner_Array:
                case EVEDB::invGroups::Logistics_Array:
                case EVEDB::invGroups::Cynosural_Generator_Array:
                case EVEDB::invGroups::Structure_Repair_Array: {
                    ArraySE* aSE = new ArraySE(structure, *(system.GetServiceMgr()), &system, data);
                    _log(POS__TRACE, "DynamicEntityFactory::BuildEntity() making ArraySE item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    pSSE = aSE;
                } break;
                case EVEDB::invGroups::Silo:
                case EVEDB::invGroups::Moon_Mining:
                case EVEDB::invGroups::Mobile_Reactor: {
                    ReactorSE* rSE = new ReactorSE(structure, *(system.GetServiceMgr()), &system, data);
                    _log(POS__TRACE, "DynamicEntityFactory::BuildEntity() making ReactorSE item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    pSSE = rSE;
                } break;
                default: {
                    StructureSE* sSE = new StructureSE(structure, *(system.GetServiceMgr()), &system, data);
                    //structure->GetMyInventory()->LoadContents();  this is called during structureItem creation
                    if ((entity.planetID) and (entity.groupID != EVEDB::invGroups::Test_Orbitals)) {
                        sSE->SetPlanet(entity.planetID);
                        SystemEntity* pPE = system.GetSE(entity.planetID);
                        if ((pPE != nullptr) and pPE->IsPlanetSE()) {
                            GVector dir(structure->position(), pPE->GetPosition());
                            dir.normalize();
                            sSE->SetRotation(dir);
                            pPE->GetPlanetSE()->SetCustomsOffice(sSE);
                        }
                    }
                    _log(POS__TRACE, "DynamicEntityFactory::BuildEntity() making StructureSE item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    pSSE = sSE;
                } break;
            }
            //if (pSSE == nullptr)
            //    return nullptr; // make error and return here
            //if (!system.IsLoaded()) // only init here on system boot  -crash when structureID < towerID (load is ordered by itemID)
            //    pSSE->Init(structure, nullptr);
            return pSSE;
        } break;
        case EVEDB::invCategories::Celestial: {
            // TODO: (just use CelestialEntity class for these until their own classes are written)
            // * WarpGateEntity  <-- Warp_Gate
            // * WormholeEntity  <-- Wormhole
            switch (entity.groupID) {
                case EVEDB::invGroups::Wreck: {
                    WreckContainerRef wreck = sItemFactory.GetWreckContainer( entity.itemID );
                    if (wreck.get() == nullptr)
                        return nullptr;
                    /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                    WreckSE* wSE = new WreckSE(wreck, *(system.GetServiceMgr()), &system, data);
                    wreck->GetMyInventory()->LoadContents();
                    wreck->SetMySE(wSE);
                    _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making WreckSE item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    return wSE;
                } break;
                case EVEDB::invGroups::Audit_Log_Secure_Container:
                case EVEDB::invGroups::Secure_Cargo_Container:
                case EVEDB::invGroups::Cargo_Container:
                case EVEDB::invGroups::Freight_Container:
                case EVEDB::invGroups::Shipping_Crates: {
                    CargoContainerRef contRef = sItemFactory.GetCargoContainer( entity.itemID );
                    if (contRef.get() == nullptr)
                        return nullptr;
                    /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                    ContainerSE* cSE = new ContainerSE(contRef, *(system.GetServiceMgr()), &system, data);
                    contRef->GetMyInventory()->LoadContents();
                    contRef->SetMySE(cSE);
                    _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making ContainerSE item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    return cSE;
                } break;
            /** @todo (Allan)  need to separate these by class to create proper SE (started) */
                case EVEDB::invGroups::Biomass:
                case EVEDB::invGroups::Ring:/*wtf*/
                case EVEDB::invGroups::Secondary_Sun:
                case EVEDB::invGroups::Large_Collidable_Object:
                case EVEDB::invGroups::Large_Collidable_Structure:
                case EVEDB::invGroups::Large_Collidable_Ship:  // this will not hit.  category 11, entity
                case EVEDB::invGroups::Cloud:
                case EVEDB::invGroups::Landmark:
                case EVEDB::invGroups::Comet:
                case EVEDB::invGroups::Destructable_Station_Services:
                /* test these to see if they are POS types */
                case EVEDB::invGroups::Construction_Platform:
                case EVEDB::invGroups::Station_Improvement_Platform:
                case EVEDB::invGroups::Global_Warp_Disruptor:
                case EVEDB::invGroups::Station_Upgrade_Platform:
                case EVEDB::invGroups::Force_Field:  // <<<  this one is POS type but it IS a plain CSE
                /* these will get their own class eventually */
                case EVEDB::invGroups::Effect_Beacon:
                case EVEDB::invGroups::Beacon:
                case EVEDB::invGroups::Covert_Beacon:
                case EVEDB::invGroups::Harvestable_Cloud:
                case EVEDB::invGroups::Planetary_Cloud: {
                    CelestialObjectRef celestial = sItemFactory.GetCelestialObject( entity.itemID );
                    if (celestial.get() == nullptr)
                        return nullptr;
                    /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                    CelestialSE* cSE = new CelestialSE(celestial, *(system.GetServiceMgr()), &system);
                    _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making CelestialSE item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    return cSE;
                } break;
                case EVEDB::invGroups::Wormhole: {
                    CelestialObjectRef celestial = sItemFactory.GetCelestialObject( entity.itemID );
                    if (celestial.get() == nullptr)
                        return nullptr;
                    /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                    WormholeSE* wSE = new WormholeSE(celestial, *(system.GetServiceMgr()), &system);
                    _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making WormholeSE item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    return wSE;
                } break;
                case EVEDB::invGroups::Cosmic_Anomaly:
                case EVEDB::invGroups::Cosmic_Signature: {
                    CelestialObjectRef celestial = sItemFactory.GetCelestialObject( entity.itemID );
                    if (celestial.get() == nullptr)
                        return nullptr;
                    /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                    AnomalySE* aSE = new AnomalySE(celestial, *(system.GetServiceMgr()), &system);
                    _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making AnomalySE item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    return aSE;
                } break;
                case EVEDB::invGroups::Warp_Gate: { //accel gate
                    // does this need own item, or celestial, or generic or other?
                    InventoryItemRef iRef = sItemFactory.GetItem( entity.itemID );
                    //CelestialObjectRef celestial = sItemFactory.GetCelestialObject( entity.itemID );
                    if (iRef.get() == nullptr)
                        return nullptr;
                    /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                    ItemSystemEntity* aSE = new ItemSystemEntity(iRef, *(system.GetServiceMgr()), &system);
                    _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making ItemSystemEntity item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    return aSE;
                } break;
            } break;
        } break;
        case EVEDB::invCategories::Entity: {            // Entities
            if (entity.groupID == EVEDB::invGroups::Spawn_Container ) {     // these are destructible objects found in dungeons
                // For category=Entity, group=Spawn Container, create a CargoContainer object:
                /** @todo  this needs its own class....there are 477 types, spawing everything..rats, modules, items, etc. */
                CargoContainerRef contRef = sItemFactory.GetCargoContainer( entity.itemID );
                if (contRef.get() == nullptr)
                    return nullptr;
                /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                ContainerSE* cSE = new ContainerSE(contRef, *(system.GetServiceMgr()), &system, data);
                contRef->GetMyInventory()->LoadContents();
                contRef->SetMySE(cSE);
                _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making ContainerEntity item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                return cSE;
            }
            // Check for NPC ships/drones here (category 11):   NOT player drones (different category [18])
            else if((entity.groupID == EVEDB::invGroups::Police_Drone) or (entity.groupID == EVEDB::invGroups::Pirate_Drone) or (entity.groupID == EVEDB::invGroups::LCO_Drone)
                or (entity.groupID == EVEDB::invGroups::Tutorial_Drone) or (entity.groupID == EVEDB::invGroups::Rogue_Drone) or (entity.groupID == EVEDB::invGroups::Faction_Drone)
                or (entity.groupID == EVEDB::invGroups::Convoy) or (entity.groupID == EVEDB::invGroups::Convoy_Drone) or (entity.groupID == EVEDB::invGroups::Concord_Drone)
                or (entity.groupID == EVEDB::invGroups::Mission_Drone) or (entity.groupID == EVEDB::invGroups::Deadspace_Overseer) or (entity.groupID == EVEDB::invGroups::Customs_Official)
                or (entity.groupID == EVEDB::invGroups::Deadspace_Overseer_s_Structure) or (entity.groupID == EVEDB::invGroups::Deadspace_Overseer_s_Sentry)
                or (entity.groupID == EVEDB::invGroups::Deadspace_Overseer_s_Belongings) or (entity.groupID == EVEDB::invGroups::Storyline_Frigate)
                or (entity.groupID == EVEDB::invGroups::Storyline_Cruiser) or (entity.groupID == EVEDB::invGroups::Storyline_Battleship) or (entity.groupID == EVEDB::invGroups::Storyline_Mission_Frigate)
                or (entity.groupID == EVEDB::invGroups::Storyline_Mission_Cruiser) or (entity.groupID == EVEDB::invGroups::Storyline_Mission_Battleship)
                or ((entity.groupID >= EVEDB::invGroups::Asteroid_Angel_Cartel_Frigate) and (entity.groupID <= EVEDB::invGroups::Asteroid_Serpentis_BattleCruiser))
                or ((entity.groupID >= EVEDB::invGroups::Deadspace_Angel_Cartel_BattleCruiser) and (entity.groupID <= EVEDB::invGroups::Deadspace_Angel_Cartel_Frigate))
                or ((entity.groupID >= EVEDB::invGroups::Deadspace_Blood_Raiders_BattleCruiser) and (entity.groupID <= EVEDB::invGroups::Deadspace_Blood_Raiders_Frigate))
                or ((entity.groupID >= EVEDB::invGroups::Deadspace_Guristas_BattleCruiser) and (entity.groupID <= EVEDB::invGroups::Deadspace_Guristas_Frigate))
                or ((entity.groupID >= EVEDB::invGroups::Deadspace_Sanshas_Nation_BattleCruiser) and (entity.groupID <= EVEDB::invGroups::Deadspace_Sanshas_Nation_Frigate))
                or ((entity.groupID >= EVEDB::invGroups::Deadspace_Serpentis_BattleCruiser) and (entity.groupID <= EVEDB::invGroups::Deadspace_Serpentis_Frigate))
                or ((entity.groupID >= EVEDB::invGroups::Mission_Amarr_Empire_Frigate) and (entity.groupID <= EVEDB::invGroups::Mission_Minmatar_Republic_Battleship))
                or (entity.groupID == EVEDB::invGroups::Destructible_Agents_In_Space)
                or ((entity.groupID >= EVEDB::invGroups::Asteroid_Rogue_Drone_Battlecruiser) and (entity.groupID <= EVEDB::invGroups::Asteroid_Rogue_Drone_Swarm))
                or (entity.groupID == EVEDB::invGroups::Large_Collidable_Ship) //  this is wreck?  abandoned ship?
                or ((entity.groupID >= EVEDB::invGroups::Asteroid_Angel_Cartel_Commander_Frigate) and (entity.groupID <= EVEDB::invGroups::Asteroid_Serpentis_Commander_Frigate))
                or ((entity.groupID >= EVEDB::invGroups::Mission_Generic_Battleships) and (entity.groupID <= EVEDB::invGroups::Mission_Generic_Destroyers))
                or ((entity.groupID >= EVEDB::invGroups::Asteroid_Rogue_Drone_Commander_Battlecruiser) and (entity.groupID <= EVEDB::invGroups::Asteroid_Serpentis_Commander_Battleship))
                or (entity.groupID == EVEDB::invGroups::Mission_Fighter_Drone)
                or ((entity.groupID >= EVEDB::invGroups::Mission_Amarr_Empire_Carrier) and (entity.groupID <= EVEDB::invGroups::Mission_Minmatar_Republic_Carrier))
                or (entity.groupID == EVEDB::invGroups::Mission_Faction_Transports) or (entity.groupID == EVEDB::invGroups::Mission_Faction_Industrials)
                or (entity.groupID == EVEDB::invGroups::Deadspace_Sleeper_Sleepless_Sentinel) or (entity.groupID == EVEDB::invGroups::Deadspace_Sleeper_Awakened_Sentinel)
                or (entity.groupID == EVEDB::invGroups::Deadspace_Sleeper_Emergent_Sentinel)
                or ((entity.groupID >= EVEDB::invGroups::Deadspace_Sleeper_Sleepless_Defender) and (entity.groupID <= EVEDB::invGroups::Deadspace_Sleeper_Emergent_Patroller))
                or (entity.groupID == EVEDB::invGroups::Mission_Faction_Cruiser) or (entity.groupID == EVEDB::invGroups::Mission_Faction_Frigate)
                or (entity.groupID == EVEDB::invGroups::Incursion_Sanshas_Nation_Industrial) or (entity.groupID == EVEDB::invGroups::Incursion_Sanshas_Nation_Capital)
                or (entity.groupID == EVEDB::invGroups::Incursion_Sanshas_Nation_Frigate) or (entity.groupID == EVEDB::invGroups::Incursion_Sanshas_Nation_Cruiser)
                or (entity.groupID == EVEDB::invGroups::Incursion_Sanshas_Nation_Battleship))
            {
                InventoryItemRef npcRef = sItemFactory.GetItem( entity.itemID );
                if (npcRef.get() == nullptr)
                    return nullptr;
                /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                NPC* npcSE = new NPC(npcRef, *(system.GetServiceMgr()), &system, data);
                npcSE->Load();
                sEntityList.AddNPC();
                _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making NPC item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                return npcSE;
            } else if ((entity.groupID == EVEDB::invGroups::Sentry_Gun) or (entity.groupID == EVEDB::invGroups::Protective_Sentry_Gun)
                or (entity.groupID == EVEDB::invGroups::Destructible_Sentry_Gun) or (entity.groupID == EVEDB::invGroups::Mobile_Sentry_Gun))
            {
                InventoryItemRef sentryRef = sItemFactory.GetItem( entity.itemID );
                if (sentryRef.get() == nullptr)
                    return nullptr;
                /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                Sentry* SentrySE = new Sentry(sentryRef, *(system.GetServiceMgr()), &system, data);
                _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making Sentry item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                return SentrySE;
            }
            // may have to create unique class for Billboard (EVEDB::invGroups::Billboard)
            else {
                InventoryItemRef iRef = sItemFactory.GetItem( entity.itemID );
                if (iRef.get() == nullptr)
                    return nullptr;
                /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                ItemSystemEntity* cSE = new ItemSystemEntity(iRef, *(system.GetServiceMgr()), &system);
                _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making ItemSystemEntity item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                return cSE;
            }
        } break;
        case EVEDB::invCategories::Drone: {             // Player Drones
            InventoryItemRef drone = sItemFactory.GetItem( entity.itemID );
            if (drone.get() == nullptr)
                return nullptr;
            /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
            GPoint location(entity.x, entity.y, entity.z);
            Drone* dSE = new Drone(drone, *(system.GetServiceMgr()), &system, location, data);
            _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making Drone item for %s (%u)", entity.itemName.c_str(), entity.itemID);
            return dSE;
        } break;
        default: {
            codelog(SERVICE__ERROR, "Unhandled dynamic entity category %d for item %u of type %u", entity.categoryID, entity.itemID, entity.typeID);
        } break;
    }
    return nullptr;
}

void SystemManager::AddClient(Client* who, bool docked, bool count) {
    //called from Client::MoveToLocation()
    if (who == nullptr)
        return;
    auto itr = m_clients.find(who->GetCharacterID());
    if (itr == m_clients.end()) {
        m_clients[who->GetCharacterID()] = who;
        if (m_spawnMgr->IsInitialized() and !m_spawnMgr->IsRatTimerStarted())
            m_spawnMgr->StartRatTimer();
        _log(PLAYER__TRACE, "%s(%u): Added to system manager for %s(%u) - %u clients now in system.", \
                    who->GetName(), who->GetCharacterID(), m_data.name.c_str(), m_data.systemID, m_clients.size());
    }

    m_activityTime = 0;
    if (count) {
        ++m_players;
        _log(PLAYER__INFO, "%s(%u): Added to player count for %s(%u) - new count: %u", \
                    who->GetName(), who->GetCharacterID(), m_data.name.c_str(), m_data.systemID, m_players);
    }
}

void SystemManager::RemoveClient(Client* who, bool docked, bool count) {
    //called from Client::~Client() and Client::MoveToLocation()
    if (who == nullptr)
        return;
    auto itr = m_clients.find(who->GetCharacterID());
    if (itr != m_clients.end()) {
        m_clients.erase(itr);
        _log(PLAYER__TRACE, "%s(%u): Removed from system manager for %s(%u) - %u clients still in system.", \
                    who->GetName(), who->GetCharacterID(), m_data.name.c_str(), m_data.systemID, m_clients.size());
    }

    if (count) {
        if (m_players < 1) {
            m_players = 0;
            _log(PLAYER__ERROR, "%s(%u): player count for %s(%u) is <1  -- new count: %u", \
                    who->GetName(), who->GetCharacterID(), m_data.name.c_str(), m_data.systemID, m_players);
        } else {
            --m_players;
        }
        if (!m_players) {
            m_clients.clear();
            m_activityTime = sEntityList.GetStamp();
        }
        _log(PLAYER__INFO, "%s(%u): Removed from player count for %s(%u) - new count: %u", \
                who->GetName(), who->GetCharacterID(), m_data.name.c_str(), m_data.systemID, m_players);
    }
}

void SystemManager::AddNPC(NPC* who) {
    if (who == nullptr)
        return;
    uint32 itemID = who->GetID();
    if (m_npcs.find(itemID) != m_npcs.end()) {
        _log(ITEM__WARNING, "%s(%u): Called AddNPC(), but they're already in %s(%u).  Check bubble.", who->GetName(), itemID, m_data.name.c_str(), m_data.systemID);
    } else {
        m_npcs[itemID] = who;
    }
    _log(NPC__TRACE, "%s(%u): Added to system manager for %s(%u)", who->GetName(), who->GetID(), m_data.name.c_str(), m_data.systemID);
    AddEntity(who);
    sEntityList.AddNPC();
}

void SystemManager::RemoveNPC(NPC* who) {
    if (who == nullptr)
        return;
    auto itr = m_npcs.find(who->GetID());
    if (itr != m_npcs.end())
        m_npcs.erase(itr);

    _log(NPC__TRACE, "%s(%u): Removed from system manager for %s(%u)", who->GetName(), who->GetID(), m_data.name.c_str(), m_data.systemID);
    RemoveEntity(who);
    sEntityList.RemoveNPC();    // this is for loaded npc count.
    who->RemoveNPC();   // this deletes NPC from DB.  NPC's dont jump, so no reason to remove from system unless killed
}

void SystemManager::AddEntity(SystemEntity* who) {
    if (who == nullptr)
        return;
    uint32 itemID = who->GetID();
    if (m_entities.find(itemID) != m_entities.end()) {
        _log(ITEM__WARNING, "%s(%u): Called AddEntity(), but they're already in %s(%u).  Check bubble.", who->GetName(), itemID, m_data.name.c_str(), m_data.systemID);
    } else {
        _log(ITEM__TRACE, "%s(%u): Added to system manager for %s(%u)", who->GetName(), itemID, m_data.name.c_str(), m_data.systemID);
        m_entities[itemID] = who;
        m_entityChanged = true;
        // *most* dynamic items need proc tics.  add to proc list
        if (!IsStaticItem(itemID))
            m_ticEntities[itemID] = who;
        // Add Entity's Item Ref to Solar System Dynamic Inventory:
        AddItemToInventory( who->GetSelf() );
    }
    sBubbleMgr.Add(who);
}

void SystemManager::RemoveEntity(SystemEntity* who) {
    if (who == nullptr)
        return;
    sBubbleMgr.Remove(who);
    uint32 itemID = who->GetID();

    auto itr = m_entities.find(itemID);
    if (itr != m_entities.end()) {
        _log(ITEM__TRACE, "%s(%u): Removed from system manager for %s(%u)", who->GetName(), itemID, m_data.name.c_str(), m_data.systemID);
        if (who->TargetMgr() != nullptr)
            who->TargetMgr()->ClearAllTargets(false);
        m_entities.erase(itr);
        m_entityChanged = true;
        // Remove Entity's Item Ref from Solar System Dynamic Inventory:
        RemoveItemFromInventory( who->GetSelf() );
    } else
        _log(ITEM__WARNING, "%s(%u): Called RemoveEntity(), but they weren\'t found in system manager for %s(%u)", who->GetName(), itemID, m_data.name.c_str(), m_data.systemID);

    auto sItr = m_ticEntities.find(itemID);
    if (sItr != m_ticEntities.end())
        m_ticEntities.erase(sItr);
}

void SystemManager::AddBounty(uint32 charID, BountyData& data)
{
    /*
struct BountyData {     // this is comming from rat killed.
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
         * recDescNpcBountyListTruncated = 'NBLT'      <-- describes a trunicated list
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
    for (auto cur : m_bountyMap) {
        std::string reason = "NBLT: "; //this needs to be populated as [NBL(T): type:amt, type:amt, ... ] to get proper shit in client
        std::map<uint32, RatDataMap>::iterator itr = m_ratMap.find(cur.first);
        if (itr != m_ratMap.end()) {
            count = itr->second.size();
            for (auto cur : itr->second) {
                reason += itoa(cur.first);
                reason += ":";
                reason += itoa(cur.second);
                if (count > 1)
                    reason += ",";
                --count;
            }
            // will have to figure out how to *correctly* limit this data to count<20 or so...
        } //else {
            reason += ",...";    // this will show as "truncated" in client
        //}
        AccountService::TranserFunds(ownerCONCORD, cur.first, cur.second.amount, reason, Journal::EntryType::BountyPrizes, m_data.systemID);
        count = 0;
    }
    m_ratMap.clear();
    m_bountyMap.clear();
}

void SystemManager::DoSpawnForBubble(SystemBubble* pBubble)
{
    if (!m_spawnMgr->IsInitialized())
        return;

    if (!m_spawnMgr->IsRatSpawnEnabled()) {
        if (!m_spawnMgr->IsRatTimerStarted())
            m_spawnMgr->StartRatTimer();
        return;
    }

    if (is_log_enabled(SPAWN__MESSAGE))
        _log(SPAWN__MESSAGE, "Spawn called for bubble %u(%u) in %s(%u)[%.4f], region %u.",
             pBubble->GetID(), sBubbleMgr.GetBeltID(pBubble->GetID()), m_data.name.c_str(), m_data.systemID, m_data.securityRating, m_data.regionID);
    uint8 count = m_beltCount;
    if (count > 15)
        count = 15;
    if ((m_activeRatSpawns < count ) or (pBubble->IsGate())) {
        if (m_spawnMgr->DoSpawnForBubble(pBubble, m_data.regionID, m_data.securityRating)) {
            m_ratBubbles.emplace(pBubble->GetID(), pBubble);
            if (is_log_enabled(SPAWN__TRACE))
                _log(SPAWN__TRACE, "SystemManager::DoSpawnForBubble() completed for %s(%u) in bubble %u.  %u items in m_ratBubbles", \
                        m_data.name.c_str(), m_data.systemID, pBubble->GetID(), m_ratBubbles.size());
        } else {
            m_spawnMgr->StopRatTimer();
            if (is_log_enabled(SPAWN__TRACE))
                _log(SPAWN__TRACE, "SystemManager::DoSpawnForBubble() returned false for bubble %u.", pBubble->GetID());
        }
    }
}

void SystemManager::GetSpawnBubbles(SpawnBubbleMap* bubbleMap)
{
    if (is_log_enabled(SPAWN__MESSAGE))
        _log(SPAWN__MESSAGE, "SystemManager::GetSpawnBubbles() - called for %s(%u)", GetName().c_str(), m_data.systemID);
    SpawnBubbleMap::iterator itr = m_ratBubbles.begin();
    while (itr != m_ratBubbles.end())
        bubbleMap->emplace(itr->first, itr->second);
}

void SystemManager::RemoveSpawnBubble(SystemBubble* pBubble)
{
    if (pBubble->IsBelt()) {
        m_ratBubbles.erase(pBubble->GetID());
        --m_activeRatSpawns;
    } else if (pBubble->IsGate()) {
        m_ratBubbles.erase(pBubble->GetID());
        --m_activeGateSpawns;
    }
    if (is_log_enabled(SPAWN__MESSAGE))
        _log(SPAWN__MESSAGE, "SystemManager::RemoveSpawnBubble() - called for bubbleID %u in %s(%u).", pBubble->GetID(), GetName().c_str(), m_data.systemID);
}

uint32 SystemManager::GetRandBeltID()
{
    uint8 randID = MakeRandomInt(0, m_beltCount);
    return m_beltVector.at(randID);
}

void SystemManager::MakeSetState(const SystemBubble* bubble,  SetState& into) const {
    using namespace Destiny;
    Buffer* stateBuffer = new Buffer();

    AddBall_header head;
        head.packet_type = 0;   // 0 = full state   1 = balls
        head.eventStamp = into.stamp;
    stateBuffer->Append( head );

    std::vector<SystemEntity*> visibleEntities;

    for (auto cur : m_staticEntities)
        visibleEntities.push_back(cur.second);

    /*  setstate only sends static global entities and your ship.  addballs sends dynamics
    if (bubble != nullptr)
       bubble->GetEntities(visibleEntities);
    */

    std::map<uint32, SystemEntity*>::const_iterator itr = m_ticEntities.find(into.ego);
    if (itr != m_ticEntities.end())
        visibleEntities.push_back(itr->second);

    into.slims = new PyList();
    into.slims->clear();
    into.effectStates = new PyList();
    into.effectStates->clear();
    into.allianceBridges = new PyList();
    into.allianceBridges->clear();  //activeBeacon and activeBridge data found in fleetSvc.py

    //go through all visible entities and gather the info we need...
    for (auto cur : visibleEntities) {
        if (!cur->IsMissileSE() or !cur->IsFieldSE())
            into.damageState[ cur->GetID() ] = cur->MakeDamageState();

        into.slims->AddItem( new PyObject( "foo.SlimItem", cur->MakeSlimItem() ) );

        //append the destiny binary data...
        cur->EncodeDestiny( *stateBuffer );

        // get tower effect state (if applicable)
        if (cur->IsTowerSE())
            cur->GetTowerSE()->GetEffectState(*(into.effectStates));

        /**  @todo (allan)  this needs more work.  should be done same as damageState.  28.2.16
        //ss.aggressors is for players undocking/jumping with aggression (uses GetCriminalTimeStamps)  ** see notes in Client::GetAggressors()
        if (cur->HasPilot() and cur->HasAggression())
            ss.aggressors[ cur->GetID() ] = cur->GetAggressors());
        */

        /** @todo (allan)  to be written   -jumpbridges is a PyList */
        //  if (cur->IsJumpBridgeSE)
        //ss.allianceBridges -- jumpbridges et al.
        //  [for shipID, toSolarsystemID, toBeaconID in bag.allianceBridges:]
    }

    into.destiny_state = new PyBuffer( &stateBuffer );
    into.droneState = SystemDB::GetSolDroneState( m_data.systemID );

    /* SolarSystem info.  this avoids the old way of a DB hit for every call.  */
    DBRowDescriptor* header = new DBRowDescriptor;
        header->AddColumn( "itemID",     DBTYPE_I8 );
        header->AddColumn( "typeID",     DBTYPE_I4 );
        header->AddColumn( "ownerID",    DBTYPE_I4 );
        header->AddColumn( "locationID", DBTYPE_I8 );
        header->AddColumn( "flagID",     DBTYPE_I2 );
        header->AddColumn( "quantity",   DBTYPE_I4 );
        header->AddColumn( "groupID",    DBTYPE_I2 );
        header->AddColumn( "categoryID", DBTYPE_I4 );
        header->AddColumn( "customInfo", DBTYPE_STR );
    PyPackedRow* row = new PyPackedRow( header );
        row->SetField( "itemID",        new PyLong(m_data.systemID));
        row->SetField( "typeID",        new PyInt(5));
        row->SetField( "ownerID",       new PyInt(1));  // should this be owning factionID?
        row->SetField( "locationID",    new PyLong(m_data.constellationID));
        row->SetField( "flagID",        new PyInt(0));
        row->SetField( "quantity",      new PyInt(-1));
        row->SetField( "groupID",       new PyInt(5));
        row->SetField( "categoryID",    new PyInt(2));
        row->SetField( "customInfo",    new PyString(""));
    into.solItem = row;

    if (is_log_enabled(DESTINY__SETSTATE)) {
        _log( DESTINY__SETSTATE, "Current State of %s", GetName().c_str() );
        into.Dump( DESTINY__SETSTATE, "    " );
    }

    if (is_log_enabled(DESTINY__SETSTATE_DECODE)) {
        _log( DESTINY__SETSTATE_DECODE, "    Decoded:" );
        Destiny::DumpUpdate( DESTINY__SETSTATE_DECODE, &( into.destiny_state->content() )[0], (uint32)into.destiny_state->content().size() );
    }
}

void SystemManager::AddItemToInventory(InventoryItemRef item)
{
    m_solarSystemRef->AddItemToInventory( item );
}

void SystemManager::RemoveItemFromInventory(InventoryItemRef item)
{
    // just in case this is called from elsewhere (which it may be), make sure we remove entity from our map.
    auto itr = m_entities.find(item->itemID());
    if (itr != m_entities.end()) {
        m_entities.erase(itr);
        m_entityChanged = true;
    }
    _log(ITEM__TRACE, "SystemManager::RemoveItemFromInventory() - removing item %s(%u) from inventory of %s(%u)", item->itemName().c_str(), item->itemID(), m_data.name.c_str(), m_data.systemID);
    m_solarSystemRef->RemoveItemFromInventory( item );
}

SystemEntity* SystemManager::GetSE(uint32 entityID) const {
    std::map<uint32, SystemEntity*>::const_iterator itr = m_entities.find(entityID);
    if (itr == m_entities.end())
        return nullptr;
    return itr->second;
}

NPC* SystemManager::GetNPCSE(uint32 entityID) const
{
    std::map<uint32, NPC*>::const_iterator itr = m_npcs.find(entityID);
    if (itr == m_npcs.end())
        return nullptr;
    return itr->second;
}

ShipItemRef SystemManager::GetShipFromInventory(uint32 shipID)
{
    return RefPtr<ShipItem>::StaticCast( m_solarSystemRef->GetMyInventory()->GetByID( shipID ) );
}

CargoContainerRef SystemManager::GetContainerFromInventory(uint32 contID)
{
    return RefPtr<CargoContainer>::StaticCast( m_solarSystemRef->GetMyInventory()->GetByID( contID ) );
}

StationItemRef SystemManager::GetStationFromInventory(uint32 stationID)
{
    return RefPtr<StationItem>::StaticCast( m_solarSystemRef->GetMyInventory()->GetByID( stationID ) );
}

uint32 SystemManager::GetClosestPlanetID(const GPoint& myPos)
{
    std::map<double, SystemEntity*> sorted;
    for (auto cur : m_planetMap) {
        sorted.insert(std::pair<double, SystemEntity*>(myPos.distance(cur.second->GetPosition()), cur.second));
    }
    std::map<double, SystemEntity*>::iterator itr = sorted.begin();

    return itr->second->GetID();
}

SystemEntity* SystemManager::GetClosestMoonSE(const GPoint& myPos)
{
    std::map<double, SystemEntity*> sorted;
    for (auto cur : m_moonMap) {
        sorted.insert(std::pair<double, SystemEntity*>(myPos.distance(cur.second->GetPosition()), cur.second));
    }
    std::map<double, SystemEntity*>::iterator itr = sorted.begin();

    return itr->second;
}



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
     * if copy to the buffer is zero, then the iterator is invalid.
     * Otherwise it is valid. I like to call invalid iterators also as "null iterators".
     */
}
