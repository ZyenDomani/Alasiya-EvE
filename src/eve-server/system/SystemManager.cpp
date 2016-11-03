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
#include "chat/LSCService.h"
#include "npc/NPC.h"
#include "planet/Planet.h"
#include "planet/Moon.h"
#include "pos/Structure.h"
#include "npc/Drone.h"
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
:  m_services(svc),
  m_anomMgr(new AnomalyMgr(this, svc)),
  m_beltMgr(new AsteroidBeltMgr(this, svc)),
  m_dunMgr(new DungeonMgr(this, svc)),
  m_spawnMgr(new SpawnMgr(this, svc))
  {
    m_clients.clear();
    m_entities.clear();
    m_ratBubbles.clear();
    m_entityChanged = false;
    m_players = 0;
    m_activityTime = 0;
    sDataMgr.GetSystemInfo(systemID, m_data);
    _log(COMMON__MESSAGE, "Created SystemManager %p for System %s(%u)", this, m_data.name.c_str(), m_data.systemID);
}

SystemManager::~SystemManager() {
    if (m_players or !m_clients.empty()) {
        _log(COMMON__ERROR, "D'tor called for System %u with %u players and/or %u clients in mmaps", m_data.systemID, m_players, m_clients.size());
        for (auto cur : m_clients)
            sEntityList.Remove(cur.second);
    }
    /*
    for (auto cur : m_entities) {
        if (IsStation(cur.first))
            sEntityList.RemoveStation(cur.first);
        else if (cur.second->IsNPCSE())
            cur.second->GetNPCSE()->SaveNPC();
        else if (cur.second->IsAsteroidSE())
            cur.second->GetSelf()->SaveItem();

        //itemFactory().RemoveItem(cur.first);

        // this is a failsafe, as there should be no piloted ships in this system now
        if (cur.second->HasPilot()) {
            Client* pClient = cur.second->GetPilot();
            SafeDelete(pClient);
        }
        SafeDelete(cur.second);
    } */

    m_clients.clear();
    m_entities.clear();
    m_ratBubbles.clear();

    SafeDelete(m_anomMgr);
    SafeDelete(m_beltMgr);
    SafeDelete(m_dunMgr);
    SafeDelete(m_spawnMgr);
}

static const int num_hack_sentry_locs = 8;
GPoint hack_sentry_locs[num_hack_sentry_locs] = {
    GPoint(-35000.0f, -35000.0f, -35000.0f),
    GPoint(-35000.0f, -35000.0f, 35000.0f),
    GPoint(-35000.0f, 35000.0f, -35000.0f),
    GPoint(-35000.0f, 35000.0f, 35000.0f),
    GPoint(35000.0f, -35000.0f, -35000.0f),
    GPoint(35000.0f, -35000.0f, 35000.0f),
    GPoint(35000.0f, 35000.0f, -35000.0f),
    GPoint(35000.0f, 35000.0f, 35000.0f)
};

void SystemManager::LoadCosmicMgrs()
{
    m_spawnMgr->Init();
    m_beltMgr->Init(m_data.regionID);
    m_dunMgr->Init(m_anomMgr, m_spawnMgr);
    m_anomMgr->Init(m_beltMgr, m_dunMgr, m_spawnMgr);
}

bool SystemManager::LoadSystemStatics() {
    std::vector<DBSystemEntity> entities;
    entities.clear();
    m_entities.clear();
    SystemDB m_db;
    if (!m_db.LoadSystemStaticEntities(m_data.systemID, entities)) {
        _log(INV__ERROR, "Unable to load celestial entities during boot of system %u.", m_data.systemID);
        return false;
    }

    SystemEntity* pSE;
    for (auto cur : entities) {
        switch (cur.groupID) {
            case EVEDB::invGroups::Station: {
                /** @todo (Allan) outposts are group::station - may need to hack this */
                /*  types 12242 - 22298 in group 15 are outposts */
                /*  types 29323 - 29390 in group 15 are wrecked stations */
                StationItemRef itemRef = itemFactory()->GetStation(cur.itemID);
                StationSE *se = new StationSE(itemRef, *(GetServiceMgr()), this);
                sEntityList.AddStation(cur.itemID, itemRef);
                pSE = se;
            } break;
            case EVEDB::invGroups::Asteroid_Belt: {
                CelestialObjectRef itemRef = itemFactory()->GetCelestialObject(cur.itemID);
                BeltSE *se = new BeltSE(itemRef, *(GetServiceMgr()), this);
                ++m_beltCount;
                pSE = se;
            } break;
            case EVEDB::invGroups::Stargate: {
                CelestialObjectRef itemRef = itemFactory()->GetCelestialObject(cur.itemID);
                StargateSE *se = new StargateSE(itemRef, *(GetServiceMgr()), this);
                ++m_gateCount;
                pSE = se;
            } break;
            case EVEDB::invGroups::Planet: {
                CelestialObjectRef itemRef = itemFactory()->GetCelestialObject(cur.itemID);
                PlanetSE *se = new PlanetSE(itemRef, *(GetServiceMgr()), this);
                pSE = se;
            } break;
            case EVEDB::invGroups::Moon: {
                CelestialObjectRef itemRef = itemFactory()->GetCelestialObject(cur.itemID);
                MoonSE *se = new MoonSE(itemRef, *(GetServiceMgr()), this);
                pSE = se;
            } break;
            default: /*sun*/ {    // suns dont have anything special, so they are generic StaticSystemEntitys
                CelestialObjectRef itemRef = itemFactory()->GetCelestialObject(cur.itemID);
                StaticSystemEntity *se = new StaticSystemEntity(itemRef, *(GetServiceMgr()), this);
                pSE = se;
            } break;
        }
        if (!pSE) {
            _log(INV__WARNING, "Failed to create entity for item %u (type %u)", cur.itemID, cur.typeID);
            continue;
        }
        if (pSE->IsBeltSE() or pSE->IsGateSE() or pSE->IsStationSE())
            sBubbleMgr.Add(pSE);
        m_entities[cur.itemID] = pSE;
        AddItemToInventory(pSE->GetSelf());
        SystemDB m_db;
        if (!pSE->LoadExtras(&m_db))
            _log(INV__WARNING, "Failed to load additional data for entity %u. Continuing.", cur.itemID);
    }

    if (m_entities.size())
        m_entityChanged = true;

    _log(SERVER__INIT, "%u Static System entities loaded for system %u", entities.size(), m_data.systemID);
    entities.clear();
    return m_entityChanged;
}

class DynamicEntityFactory {
public:
    static SystemEntity* BuildEntity(SystemManager &system, ItemFactory* factory, const DBSystemDynamicEntity &entity) {
        switch(entity.categoryID) {
            case EVEDB::invCategories::Asteroid: {
                InventoryItemRef asteroid = factory->GetItem( entity.itemID );
                if (!asteroid)
                    ; /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                AsteroidSE* aSE = new AsteroidSE(asteroid, *(system.GetServiceMgr()), &system);
                _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making AsteroidSE item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                return aSE;
            } break;
            case EVEDB::invCategories::Ship: {
                ShipItemRef ship = factory->GetShip( entity.itemID );
                if (!ship)
                    return nullptr;
                /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                /* if this ship has a pilot (not an abandonded ship in space), then cancel and return nothing.
                 *                if (!ship->GetInventory()->IsEmptyByFlag(flagPilot)) {
                 *                    factory.RemoveItem(entity.itemID);
                 *                    return nullptr;
            } */
                Ship* sSE = new Ship(ship, *(system.GetServiceMgr()), &system);
                _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making Ship item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                return sSE;
            } break;
            case EVEDB::invCategories::Deployable: {
                InventoryItemRef deployable = factory->GetItem( entity.itemID );
                if (!deployable)
                    return nullptr;
                /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                deployable->SetAttribute(AttrRadius, deployable->type().radius());     // Can you set this somehow from the type class ?
                DeployableSE* dSE = new DeployableSE(deployable, *(system.GetServiceMgr()), &system);
                _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making DeployableSE item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                return dSE;
            } break;
            case EVEDB::invCategories::Structure: {         // POS Structures of all kinds
                StructureItemRef structure = factory->GetStructure( entity.itemID );
                if (!structure)
                    return nullptr;
                /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                StructureSE* sSE = new StructureSE(structure, *(system.GetServiceMgr()), &system);
                structure->GetInventory()->LoadContents(factory);
                _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making StructureSE item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                return sSE;
            } break;
            case EVEDB::invCategories::Celestial: {
                // TODO: (just use CelestialEntity class for these until their own classes are written)
                // * WarpGateEntity  <-- Warp_Gate
                // * WormholeEntity  <-- Wormhole

                if (entity.groupID == EVEDB::invGroups::Wreck) {
                    WreckContainerRef wreck = factory->GetWreckContainer( entity.itemID );
                    if (!wreck)
                        return nullptr;
                    /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                    WreckSE* wSE = new WreckSE(wreck, *(system.GetServiceMgr()), &system);
                    wreck->GetInventory()->LoadContents(factory);
                    wreck->SetMySE(wSE);
                    _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making WreckSE item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    return wSE;
                } else if ( (entity.groupID == EVEDB::invGroups::Audit_Log_Secure_Container)
                    || (entity.groupID == EVEDB::invGroups::Secure_Cargo_Container)
                    || (entity.groupID == EVEDB::invGroups::Cargo_Container)
                    || (entity.groupID == EVEDB::invGroups::Freight_Container) )
                {
                    CargoContainerRef container = factory->GetCargoContainer( entity.itemID );
                    if (!container)
                        return nullptr;
                    /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                    ContainerSE* cSE = new ContainerSE(container, *(system.GetServiceMgr()), &system);
                    container->GetInventory()->LoadContents(factory);
                    _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making ContainerSE item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    return cSE;
                } else if ((entity.groupID == EVEDB::invGroups::Sun)
                    || (entity.groupID == EVEDB::invGroups::Planet)
                    || (entity.groupID == EVEDB::invGroups::Moon) )
                {
                    CelestialObjectRef celestial = factory->GetCelestialObject( entity.itemID );
                    if (!celestial)
                        return nullptr;
                    /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                    PlanetSE* pSE = new PlanetSE(celestial, *(system.GetServiceMgr()), &system);
                    _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making PlanetSE item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    return pSE;
                } else if (entity.groupID == EVEDB::invGroups::Stargate) {
                    CelestialObjectRef celestial = factory->GetCelestialObject( entity.itemID );
                    if (!celestial)
                        return nullptr;
                    /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                    StargateSE* sSE = new StargateSE(celestial, *(system.GetServiceMgr()), &system);
                    _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making StargateSE item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    return sSE;
                }
                /** @todo (Allan)  need to separate these by class to create proper SE (started) */
                else if ((entity.groupID == EVEDB::invGroups::Biomass)
                    || (entity.groupID == EVEDB::invGroups::Ring) || (entity.groupID == EVEDB::invGroups::Secondary_Sun)
                    || (entity.groupID == EVEDB::invGroups::Large_Collidable_Object)|| (entity.groupID == EVEDB::invGroups::Large_Collidable_Structure)
                    || (entity.groupID == EVEDB::invGroups::Cloud) || (entity.groupID == EVEDB::invGroups::Landmark)
                    || (entity.groupID == EVEDB::invGroups::Shipping_Crates) || (entity.groupID == EVEDB::invGroups::Cosmic_Signature)
                    || (entity.groupID == EVEDB::invGroups::Effect_Beacon) || (entity.groupID == EVEDB::invGroups::Cosmic_Anomaly)
                    || (entity.groupID == EVEDB::invGroups::Beacon) || (entity.groupID == EVEDB::invGroups::Covert_Beacon)
                    || (entity.groupID == EVEDB::invGroups::Comet) || (entity.groupID == EVEDB::invGroups::Destructable_Station_Services)
                    /* test these to see if they are POS types */
                    || (entity.groupID == EVEDB::invGroups::Construction_Platform)
                    || (entity.groupID == EVEDB::invGroups::Station_Improvement_Platform)  || (entity.groupID == EVEDB::invGroups::Global_Warp_Disruptor)
                    || (entity.groupID == EVEDB::invGroups::Station_Upgrade_Platform) || (entity.groupID == EVEDB::invGroups::Force_Field)
                    /* these will get their own class eventually */
                    || (entity.groupID == EVEDB::invGroups::Harvestable_Cloud) || (entity.groupID == EVEDB::invGroups::Planetary_Cloud)
                    || (entity.groupID == EVEDB::invGroups::Warp_Gate) || (entity.groupID == EVEDB::invGroups::Wormhole))
                {
                    CelestialObjectRef celestial = factory->GetCelestialObject( entity.itemID );
                    if (!celestial)
                        return nullptr;
                    /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                    CelestialSE* cSE = new CelestialSE(celestial, *(system.GetServiceMgr()), &system);
                    _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making CelestialSE item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    return cSE;
                }
            } break;
            case EVEDB::invCategories::Entity: {            // Entities
                if (entity.groupID == EVEDB::invGroups::Spawn_Container )
                {
                    // For category=Entity, group=Spawn Container, create a CargoContainer object:
                    CargoContainerRef container = factory->GetCargoContainer( entity.itemID );
                    if (!container)
                        return nullptr;
                    /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                    ContainerSE* cSE = new ContainerSE(container, *(system.GetServiceMgr()), &system);
                    container->GetInventory()->LoadContents(factory);
                    _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making ContainerEntity item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    return cSE;
                }
                // Check for NPC ships/drones here (category 11):   NOT player drones (different category [18])
                else if(    (entity.groupID == EVEDB::invGroups::Police_Drone)
                    ||  (entity.groupID == EVEDB::invGroups::Pirate_Drone)
                    ||  (entity.groupID == EVEDB::invGroups::LCO_Drone)
                    ||  (entity.groupID == EVEDB::invGroups::Tutorial_Drone)
                    ||  (entity.groupID == EVEDB::invGroups::Rogue_Drone)
                    ||  (entity.groupID == EVEDB::invGroups::Faction_Drone)
                    ||  (entity.groupID == EVEDB::invGroups::Convoy)
                    ||  (entity.groupID == EVEDB::invGroups::Convoy_Drone)
                    ||  (entity.groupID == EVEDB::invGroups::Concord_Drone)
                    ||  (entity.groupID == EVEDB::invGroups::Mission_Drone)
                    ||  (entity.groupID == EVEDB::invGroups::Deadspace_Overseer)
                    ||  (entity.groupID == EVEDB::invGroups::Customs_Official)
                    ||  (entity.groupID == EVEDB::invGroups::Deadspace_Overseer_s_Structure)
                    ||  (entity.groupID == EVEDB::invGroups::Deadspace_Overseer_s_Sentry)
                    ||  (entity.groupID == EVEDB::invGroups::Deadspace_Overseer_s_Belongings)
                    ||  (entity.groupID == EVEDB::invGroups::Storyline_Frigate)
                    ||  (entity.groupID == EVEDB::invGroups::Storyline_Cruiser)
                    ||  (entity.groupID == EVEDB::invGroups::Storyline_Battleship)
                    ||  (entity.groupID == EVEDB::invGroups::Storyline_Mission_Frigate)
                    ||  (entity.groupID == EVEDB::invGroups::Storyline_Mission_Cruiser)
                    ||  (entity.groupID == EVEDB::invGroups::Storyline_Mission_Battleship)
                    ||  ((entity.groupID >= EVEDB::invGroups::Asteroid_Angel_Cartel_Frigate) && (entity.groupID <= EVEDB::invGroups::Asteroid_Serpentis_BattleCruiser))
                    ||  ((entity.groupID >= EVEDB::invGroups::Deadspace_Angel_Cartel_BattleCruiser) && (entity.groupID <= EVEDB::invGroups::Deadspace_Angel_Cartel_Frigate))
                    ||  ((entity.groupID >= EVEDB::invGroups::Deadspace_Blood_Raiders_BattleCruiser) && (entity.groupID <= EVEDB::invGroups::Deadspace_Blood_Raiders_Frigate))
                    ||  ((entity.groupID >= EVEDB::invGroups::Deadspace_Guristas_BattleCruiser) && (entity.groupID <= EVEDB::invGroups::Deadspace_Guristas_Frigate))
                    ||  ((entity.groupID >= EVEDB::invGroups::Deadspace_Sanshas_Nation_BattleCruiser) && (entity.groupID <= EVEDB::invGroups::Deadspace_Sanshas_Nation_Frigate))
                    ||  ((entity.groupID >= EVEDB::invGroups::Deadspace_Serpentis_BattleCruiser) && (entity.groupID <= EVEDB::invGroups::Deadspace_Serpentis_Frigate))
                    ||  ((entity.groupID >= EVEDB::invGroups::Mission_Amarr_Empire_Frigate) && (entity.groupID <= EVEDB::invGroups::Mission_Minmatar_Republic_Battleship))
                    ||  (entity.groupID == EVEDB::invGroups::Destructible_Agents_In_Space)
                    ||  ((entity.groupID >= EVEDB::invGroups::Asteroid_Rogue_Drone_Battlecruiser) && (entity.groupID <= EVEDB::invGroups::Asteroid_Rogue_Drone_Swarm))
                    ||  (entity.groupID == EVEDB::invGroups::Large_Collidable_Ship)
                    ||  ((entity.groupID >= EVEDB::invGroups::Asteroid_Angel_Cartel_Commander_Frigate) && (entity.groupID <= EVEDB::invGroups::Asteroid_Serpentis_Commander_Frigate))
                    ||  ((entity.groupID >= EVEDB::invGroups::Mission_Generic_Battleships) && (entity.groupID <= EVEDB::invGroups::Mission_Generic_Destroyers))
                    ||  ((entity.groupID >= EVEDB::invGroups::Asteroid_Rogue_Drone_Commander_Battlecruiser) && (entity.groupID <= EVEDB::invGroups::Asteroid_Serpentis_Commander_Battleship))
                    ||  (entity.groupID == EVEDB::invGroups::Mission_Fighter_Drone)
                    ||  ((entity.groupID >= EVEDB::invGroups::Mission_Amarr_Empire_Carrier) && (entity.groupID <= EVEDB::invGroups::Mission_Minmatar_Republic_Carrier))
                    ||  (entity.groupID == EVEDB::invGroups::Mission_Faction_Transports)
                    ||  (entity.groupID == EVEDB::invGroups::Mission_Faction_Industrials)
                    ||  (entity.groupID == EVEDB::invGroups::Deadspace_Sleeper_Sleepless_Sentinel)
                    ||  (entity.groupID == EVEDB::invGroups::Deadspace_Sleeper_Awakened_Sentinel)
                    ||  (entity.groupID == EVEDB::invGroups::Deadspace_Sleeper_Emergent_Sentinel)
                    ||  ((entity.groupID >= EVEDB::invGroups::Deadspace_Sleeper_Sleepless_Defender) && (entity.groupID <= EVEDB::invGroups::Deadspace_Sleeper_Emergent_Patroller))
                    ||  (entity.groupID == EVEDB::invGroups::Mission_Faction_Cruiser)
                    ||  (entity.groupID == EVEDB::invGroups::Mission_Faction_Frigate)
                    ||  (entity.groupID == EVEDB::invGroups::Incursion_Sanshas_Nation_Industrial)
                    ||  (entity.groupID == EVEDB::invGroups::Incursion_Sanshas_Nation_Capital)
                    ||  (entity.groupID == EVEDB::invGroups::Incursion_Sanshas_Nation_Frigate)
                    ||  (entity.groupID == EVEDB::invGroups::Incursion_Sanshas_Nation_Cruiser)
                    ||  (entity.groupID == EVEDB::invGroups::Incursion_Sanshas_Nation_Battleship)
                    /** @todo (allan)  sentry guns will need their own class, seperate from NPC class here. */
                    /* these will get their own class eventually */
                    ||  (entity.groupID == EVEDB::invGroups::Sentry_Gun)
                    ||  (entity.groupID == EVEDB::invGroups::Protective_Sentry_Gun)
                    ||  (entity.groupID == EVEDB::invGroups::Destructible_Sentry_Gun))
                {
                    InventoryItemRef npcRef = factory->GetItem( entity.itemID );
                    if (!npcRef)
                        return nullptr;
                    /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                    /** @todo (allan)  corporationID and allianceID are both wrong here and for all entities. */
                    NPC* npcObj = new NPC(npcRef, *(system.GetServiceMgr()), &system, entity.corporationID, entity.allianceID);
                    _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making NPC item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    return npcObj;
                } else {
                    CelestialObjectRef celestial = factory->GetCelestialObject( entity.itemID );
                    if (!celestial)
                        return nullptr;
                    /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                    CelestialSE* cSE = new CelestialSE(celestial, *(system.GetServiceMgr()), &system);
                    _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making CelestialSE-2 item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                    return cSE;
                }
            } break;
            case EVEDB::invCategories::Drone: {             // Player Drones
                InventoryItemRef drone = factory->GetItem( entity.itemID );
                if (!drone)
                    return nullptr;
                /** @todo make error msg here */  //  PyException( MakeCustomError( "Unable to spawn item #%u:'%s' of type %u.", entity.itemID, entity.itemName.c_str(), entity.typeID ) );
                GPoint location(entity.x, entity.y, entity.z);
                Drone* dSE = new Drone(drone, *(system.GetServiceMgr()), &system, location);
                _log(ITEM__TRACE, "DynamicEntityFactory::BuildEntity() making Drone item for %s (%u)", entity.itemName.c_str(), entity.itemID);
                return dSE;
            } break;
            default: {
                codelog(SERVICE__ERROR, "Unhandled dynamic entity category %d for item %u of type %u", entity.categoryID, entity.itemID, entity.typeID);
            } break;
        }
        return nullptr;
    }
};

bool SystemManager::LoadSystemDynamics() {
    std::vector<DBSystemDynamicEntity> entities;
    SystemDB m_db;
    if (!m_db.LoadSystemDynamicEntities(m_data.systemID, entities)) {
        _log(SERVICE__ERROR, "Unable to load dynamic entities during boot of system %u.", m_data.systemID);
        return false;
    }

    SystemEntity* pSE;
    for (auto cur : entities) {
        pSE = DynamicEntityFactory::BuildEntity(*this, m_services.item_factory, cur);
        if (!pSE) {
            _log(ITEM__WARNING, "LoadSystemDynamics() Failed to create entity for item %u (type %u)", cur.itemID, cur.typeID);
            continue;
        }
        _log(ITEM__TRACE, "SystemManager::LoadSystemDynamics() - Loaded dynamic entity %u of type %u for system %u", cur.itemID, cur.typeID, m_data.systemID);
        AddEntity(pSE);
    }

    return true;
}

bool SystemManager::LoadPlayerDynamics(uint32 ownerID) {
    std::vector<DBSystemDynamicEntity> entities;
    SystemDB m_db;
    if (!m_db.LoadPlayerDynamicEntities(ownerID, m_data.systemID, entities)) {
        _log(SERVICE__ERROR, "Unable to load player dynamic entities in system %u.", m_data.systemID);
        return false;
    }

    SystemEntity* pSE;
    for (auto cur : entities) {
        pSE = DynamicEntityFactory::BuildEntity(*this, m_services.item_factory, cur);
        if (!pSE) {
            _log(ITEM__WARNING, "LoadSystemDynamics() Failed to create entity for item %u (type %u)", cur.itemID, cur.typeID);
            continue;
        }
        _log(ITEM__TRACE, "SystemManager::LoadPlayerDynamics() - Loaded dynamic entity %u of type %u for system %u", cur.itemID, cur.typeID, m_data.systemID);
        AddEntity(pSE);
    }

    return true;
}

bool SystemManager::BuildDynamicEntity(const DBSystemDynamicEntity& entity) {
    SystemEntity* se = DynamicEntityFactory::BuildEntity(*this, m_services.item_factory, entity );
    if (!se) {
        sLog.Error( "SystemManager::BuildDynamicEntity()", "Failed to create entity for item %u (type %u)", entity.itemID, entity.typeID );
        return false;
    }

    _log(ITEM__TRACE, "SystemManager::BuildDynamicEntity() - Created dynamic entity %u of type %u for system %u", entity.itemID, entity.typeID, m_data.systemID );
    AddEntity(se);
    return true;
}

bool SystemManager::BootSystem() {
    m_solarSystemRef = m_services.item_factory->GetSolarSystem(m_data.systemID);
    assert(m_solarSystemRef);

    LoadCosmicMgrs();

    if (!LoadSystemStatics()) {
        _log(SERVICE__ERROR, "Unable to load Statics during boot of system %u.", m_data.systemID);
        return false;
    }
    /* this only loads items owned by eve system (ownerID = 1) */
    if (!LoadSystemDynamics()) {
        _log(SERVICE__ERROR, "Unable to load Dynamics during boot of system %u.", m_data.systemID);
        return false;
    }

    //create our chat channel
    m_services.lsc_service->CreateSystemChannel(m_data.systemID);
    return true;
}

//called once per second from EntityList. (1Hz Tic)
bool SystemManager::ProcessTic() {
    /* the idea here is entities map NEVER has invalid items in it, but our iterator may become invalid when SE->Process() returns
     * because Process() will add/remove from the map as needed (new objects, destroyed objects, moved objects, etc)
     * std::map internally orders items by key(itemID here), so use an int var to hold last-processed itemID (mLast).
     *  when iteration starts over, increment until cur > mLast and continue from there to end of list.
     */
    std::map<uint32, SystemEntity*>::iterator cur = m_entities.begin();
    uint32 mLast = 0;
    while (cur != m_entities.end()) {
        if (mLast) {
            if (mLast >= cur->first) {
                ++cur;
                continue;
            }
            mLast = 0;
        }
        if (m_entityChanged) {
            mLast = cur->first;
            m_entityChanged = false;
            cur = m_entities.begin();
            continue;
        }
        cur->second->Process(); /* main process call. */
        if (m_entityChanged) {
            mLast = cur->first;
            m_entityChanged = false;
            cur = m_entities.begin();
            continue;
        }
        ++cur;
    }

    /* the following are coded for single-tic calls */
    m_anomMgr->Process();
    m_beltMgr->Process();
    m_spawnMgr->Process();

    return SystemActivity();
}

bool SystemManager::SystemActivity() {
    return true;
    // system destruction needs work for bubbles and items (but this works as intended)
    if (!m_activityTime)
        return true;
    if (sConfig.world.gridUnload)
        if (!m_players)
            if (sConfig.world.gridUnloadTime < (sEntityList.GetStamp() - m_activityTime))
                return false;

    return true;
}

void SystemManager::UnloadSystem() {
    // only called from EntityList::Process()
    // save any roids before unloading the system
    m_beltMgr->Save();
    /** @todo finish this */
    // use Inventory::DeleteContents(ItemFactory &factory) to remove system contents from memory.
    //Inventory::DeleteContents(m_services.item_factory);
    sLog.Success("SystemManager::UnloadSystem()", "UnloadSystem() called for empty system %s(%u).", \
        GetName().c_str(), m_data.systemID);
}

void SystemManager::AddClient(Client* who, bool docked, bool count) {
    //called from Client::MoveToLocation()
    if (!who) return;
    auto itr = m_clients.find(who->GetCharacterID());
    if (itr == m_clients.end()) {
        m_clients[who->GetCharacterID()] = who;
        _log(PLAYER__TRACE, "%s(%u): Added to system manager for %s(%u) - %u clients now in system.", \
                    who->GetName(), who->GetCharacterID(), m_data.name.c_str(), m_data.systemID, m_clients.size());
    }

    m_activityTime = 0;
    if (count) {
        ++m_players;
        if (m_spawnMgr->IsInitialized() and !m_spawnMgr->IsTimerStarted())
            m_spawnMgr->StartMainTimer();
        _log(PLAYER__INFO, "%s(%u): Added to player count for %s(%u) - new count: %u", \
                    who->GetName(), who->GetCharacterID(), m_data.name.c_str(), m_data.systemID, m_players);
    }
}

void SystemManager::RemoveClient(Client* who, bool docked, bool count) {
    //called from Client::~Client() and Client::MoveToLocation()
    if (!who) return;
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
        } else
            --m_players;
        if (!m_players) {
            m_clients.clear();
            m_activityTime = sEntityList.GetStamp();
        }
        _log(PLAYER__INFO, "%s(%u): Removed from player count for %s(%u) - new count: %u", \
                who->GetName(), who->GetCharacterID(), m_data.name.c_str(), m_data.systemID, m_players);
    }
}

void SystemManager::AddNPC(NPC* who) {
    if (!who) return;
    _log(NPC__TRACE, "%s(%u): Added to system manager for %s(%u)", who->GetName(), who->GetID(), m_data.name.c_str(), m_data.systemID);
    AddEntity(who);
    sEntityList.AddNPC();
}

void SystemManager::RemoveNPC(NPC* who) {
    if (!who) return;
    _log(NPC__TRACE, "%s(%u): Removed from system manager for %s(%u)", who->GetName(), who->GetID(), m_data.name.c_str(), m_data.systemID);
    RemoveEntity(who);
    sEntityList.RemoveNPC();    // this is for loaded npc count.
    who->RemoveNPC();   // this deletes NPC from DB.  NPC's dont jump, so no reason to remove from system unless killed
}

void SystemManager::AddEntity(SystemEntity* who) {
    if (!who) return;
    _log(ITEM__TRACE, "%s(%u): Added to system manager for %s(%u)", who->GetName(), who->GetID(), m_data.name.c_str(), m_data.systemID);
    m_entities[who->GetID()] = who;
    m_entityChanged = true;
    sBubbleMgr.Add(who);
    // Add Entity's Item Ref to Solar System Dynamic Inventory:
    AddItemToInventory( who->GetSelf() );
}

void SystemManager::RemoveEntity(SystemEntity* who) {
    if (!who) return;
    sBubbleMgr.Remove(who);
    auto itr = m_entities.find(who->GetID());
    if (itr != m_entities.end()) {
        _log(ITEM__TRACE, "%s(%u): Removed from system manager for %s(%u)", who->GetName(), who->GetID(), m_data.name.c_str(), m_data.systemID);
        who->TargetMgr()->DoDestruction();
        m_entities.erase(itr);
        m_entityChanged = true;
        // Remove Entity's Item Ref from Solar System Dynamic Inventory:
        RemoveItemFromInventory( who->GetSelf() );
        /* should we delete the entity pointer here??  no, it gets deleted in EntityList */
    } else
        _log(ITEM__WARNING, "%s(%u): Called RemoveEntity(), but they weren\'t found in system manager for %s(%u)", who->GetName(), who->GetID(), m_data.name.c_str(), m_data.systemID);
}

void SystemManager::DoSpawnForBubble(SystemBubble* pSysBubble)
{
    if (!m_spawnMgr->IsInitialized())
        return;

    if (!m_spawnMgr->IsEnabled()) {
        if (!m_spawnMgr->IsTimerStarted())
            m_spawnMgr->StartMainTimer();
        return;
    }

    _log(SPAWN__MESSAGE, "Spawn called for bubble %u in system %u(%.4f), region %u.",
         pSysBubble->GetID(), m_data.systemID, m_data.securityRating, m_data.regionID);
    uint8 count = m_beltCount;
    if (count > 5) count -= 2;
    if (m_activeRatSpawns < count ) {
        m_spawnMgr->DoSpawnForBubble(pSysBubble, m_data.regionID, m_data.securityRating);
        m_ratBubbles.push_back(pSysBubble->GetID());
        _log(SPAWN__TRACE, "DoSpawnForBubble() completed for bubble %u.  %u entries in m_ratBubbles", pSysBubble->GetID(), m_ratBubbles.size());
    }
}

void SystemManager::GetSpawnBubbles(SpawnBubbleVec* bubbleMap)
{
    _log(SPAWN__MESSAGE, "SystemManager::GetSpawnBubbles() - called for %s(%u)", GetName().c_str(), m_data.systemID);
    SpawnBubbleVec::iterator itr = m_ratBubbles.begin();
    while (itr != m_ratBubbles.end())
        bubbleMap->push_back(*itr);
}

void SystemManager::RemoveSpawnBubble()
{
    _log(SPAWN__MESSAGE, "SystemManager::RemoveSpawnBubble() - called for %s(%u), but needs to be written.", GetName().c_str(), m_data.systemID);
}

SystemEntity* SystemManager::GetSE(uint32 entityID) const {
    std::map<uint32, SystemEntity*>::const_iterator res = m_entities.find(entityID);
    if (res == m_entities.end())
        return nullptr;
    return (res->second);
}

void SystemManager::MakeSetState(const SystemBubble* bubble, DoDestiny_SetState& into, bool login) const {
    using namespace Destiny;
    Buffer* stateBuffer = new Buffer;

    AddBall_header head;
        head.packet_type = 0;   // 0 = full state   1 = balls
        head.eventStamp = into.stamp;
    stateBuffer->Append( head );

    std::vector<SystemEntity*> visibleEntities;

    for (auto cur : m_entities) {
        if (cur.second->IsStaticEntity())
            visibleEntities.push_back(cur.second);
    }

    if (bubble)
       bubble->GetEntities(visibleEntities);

    into.slims = new PyList;
    into.effectStates = new PyList;
    into.allianceBridges = new PyList;

    //go through all visible entities and gather the info we need...
    for (auto cur : visibleEntities) {
        if (!cur->IsMissileSE())
            into.damageState[ cur->GetID() ] = cur->MakeDamageState();

        into.slims->AddItem( new PyObject( "foo.SlimItem", cur->MakeSlimItem() ) );

        //append the destiny binary data...
        cur->EncodeDestiny( *stateBuffer );

        /**  @todo (allan)  this needs more work.  should be done same as damageState.  28.2.16
        //ss.aggressors is for players undocking/jumping with aggression (uses GetCriminalTimeStamps)
        if (cur->HasPilot() && cur->HasAggression())
            ss.aggressors[ cur->GetID() ] = cur->GetAggressors());
            */
        /** @todo (allan)  to be written (both)   -effectStates is a PyList */
        //  if ((cur->IsPOSSE()) || (cur->IsOutpost()))
        //ss.effectStates  --pos and other structures (using effects.StructureOnline and et.al.)
        /** @todo (allan)  to be written   -jumpbridges is a PyList */
        //  if (cur->IsJumpBridgeSE)
        //ss.allianceBridges -- jumpbridges et al.
        //  [for shipID, toSolarsystemID, toBeaconID in bag.allianceBridges:]
    }

    into.destiny_state = new PyBuffer( &stateBuffer );
    SafeDelete( stateBuffer );

    SystemDB m_db;
    into.droneState = m_db.GetSolDroneState( m_data.systemID );

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
        row->SetField( "quantity",     new PyInt(-1));
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

ItemFactory* SystemManager::itemFactory() const
{
    return m_services.item_factory;
}

void SystemManager::AddItemToInventory(InventoryItemRef item)
{
    m_solarSystemRef->AddItemToInventory( item );
}

void SystemManager::RemoveItemFromInventory(InventoryItemRef item)
{
    _log(ITEM__TRACE, "SystemManager::RemoveItemFromInventory() - removing item %s(%u) from inventory of %s(%u)", item->itemName().c_str(), item->itemID(), m_data.name.c_str(), m_data.systemID);
    m_solarSystemRef->RemoveItemFromInventory( item );
}

SystemEntity* SystemManager::GetSEFromInventory(uint32 itemID) {
    auto itr = m_entities.find(itemID);
    if (itr != m_entities.end())
        return itr->second;
    return nullptr;
}

ShipItemRef SystemManager::GetShipFromInventory(uint32 shipID)
{
    return RefPtr<ShipItem>::StaticCast( m_solarSystemRef->GetInventory()->GetByID( shipID ) );
}

CargoContainerRef SystemManager::GetContainerFromInventory(uint32 contID)
{
    return RefPtr<CargoContainer>::StaticCast( m_solarSystemRef->GetInventory()->GetByID( contID ) );
}

StationItemRef SystemManager::GetStationFromInventory(uint32 stationID)
{
    return RefPtr<StationItem>::StaticCast( m_solarSystemRef->GetInventory()->GetByID( stationID ) );
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
