  /**
   * @name EntityFactory.cpp
   *     Method to create dynamic entities for Alasiya EvEmu
   *     split from SystemManager
   *
   * @Author:        Allan
   * @date:          29 August 2025
   *
   */


#include "system/EntityFactory.h"

#include "exploration/Probes.h"
#include "npc/Drone.h"
#include "npc/NPC.h"
#include "npc/Sentry.h"
#include "planet/CustomsOffice.h"
#include "planet/Planet.h"
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
#include "system/SystemManager.h"

EntityFactory::EntityFactory() : m_routingArray(nullptr) {
    m_routingArray = new EntityClass[255];
    // set the entire array to 'Unknown' state
    std::memset(m_routingArray, 0, 255 * sizeof(m_routingArray));
}

EntityFactory::~EntityFactory() {
    //SafeDeleteArray(m_routingArray);
    delete[] m_routingArray;
}

SystemEntity* EntityFactory::BuildEntity(SystemManager& sysMgr, const DBSystemDynamicEntity& eData) {
    FactionData fData = FactionData();
        fData.allianceID = eData.allianceID;
        fData.corporationID = eData.corporationID;
        fData.factionID = eData.factionID;
        fData.ownerID = eData.ownerID;

    switch (eData.categoryID) {
        case EVEDB::invCategories::Asteroid: {
            InventoryItemRef asteroid = sItemFactory.GetItemRef(eData.itemID);
            if (asteroid.get() == nullptr) {
                _log(ITEM__ERROR, "EntityFactory - iRef = nullptr for %s (%u)", eData.itemName.c_str(), eData.itemID);
                return nullptr;
            }
            AsteroidSE* aSE = new AsteroidSE(asteroid, *(sysMgr.GetServiceMgr()), &sysMgr);
            _log(ITEM__TRACE, "EntityFactory making AsteroidSE for %s (%u)", eData.itemName.c_str(), eData.itemID);
            return aSE;
        } break;
        case EVEDB::invCategories::Ship: {
            ShipItemRef ship = sItemFactory.GetShipRef(eData.itemID);
            if (ship.get() == nullptr) {
                _log(ITEM__ERROR, "EntityFactory - iRef = nullptr for %s (%u)", eData.itemName.c_str(), eData.itemID);
                return nullptr;
            }
            ShipSE* sSE = new ShipSE(ship, *(sysMgr.GetServiceMgr()), &sysMgr, fData);
            _log(ITEM__TRACE, "EntityFactory making ShipSE for %s (%u)", eData.itemName.c_str(), eData.itemID);
            if (fData.ownerID == 1) {
                // initialize abandoned ship here.
                ship->Init();
                sSE->DestinyMgr()->UpdateShipVariables();
            }
            return sSE;
        } break;
        case EVEDB::invCategories::Deployable: {
            InventoryItemRef deployable = sItemFactory.GetItemRef(eData.itemID);
            if (deployable.get() == nullptr) {
                _log(ITEM__ERROR, "EntityFactory - iRef = nullptr for %s (%u)", eData.itemName.c_str(), eData.itemID);
                return nullptr;
            }
            deployable->SetAttribute(AttrRadius, deployable->type().radius());     // Can you set this somehow from the type class ?
            DeployableSE* dSE = new DeployableSE(deployable, *(sysMgr.GetServiceMgr()), &sysMgr, fData);
            _log(ITEM__TRACE, "EntityFactory making DeployableSE for %s (%u)", eData.itemName.c_str(), eData.itemID);
            return dSE;
        } break;
        //  these should go into m_staticEntities
        case EVEDB::invCategories::StructureUpgrade: // SOV upgrade structures   these may need their own class one day.
        case EVEDB::invCategories::Structure: {         // POS items
            StructureItemRef structure = sItemFactory.GetStructureRef(eData.itemID);
            if (structure.get() == nullptr) {
                _log(ITEM__ERROR, "EntityFactory - iRef = nullptr for %s (%u)", eData.itemName.c_str(), eData.itemID);
                return nullptr;
            }
            StructureSE* pSSE(nullptr);
            switch(eData.groupID) {
                case EVEDB::invGroups::Control_Tower: {
                    TowerSE* tSE = new TowerSE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, fData);
                    _log(POS__TRACE, "EntityFactory making TowerSE for %s (%u)", eData.itemName.c_str(), eData.itemID);
                    pSSE = tSE;
                } break;
                case EVEDB::invGroups::Mobile_Missile_Sentry:
                case EVEDB::invGroups::Mobile_Projectile_Sentry:
                case EVEDB::invGroups::Mobile_Laser_Sentry:
                case EVEDB::invGroups::Mobile_Hybrid_Sentry: {
                    WeaponSE* wSE = new WeaponSE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, fData);
                    _log(POS__TRACE, "EntityFactory making WeaponSE for %s (%u)", eData.itemName.c_str(), eData.itemID);
                    pSSE =  wSE;
                } break;
                case EVEDB::invGroups::Electronic_Warfare_Battery:
                case EVEDB::invGroups::Sensor_Dampening_Battery:
                case EVEDB::invGroups::Stasis_Webification_Battery:
                case EVEDB::invGroups::Warp_Scrambling_Battery:
                case EVEDB::invGroups::Energy_Neutralizing_Battery:
                case EVEDB::invGroups::Target_Painting_Battery: {
                    BatterySE* bSE = new BatterySE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, fData);
                    _log(POS__TRACE, "EntityFactory making BatterySE for %s (%u)", eData.itemName.c_str(), eData.itemID);
                    pSSE = bSE;
                } break;
                case EVEDB::invGroups::Refining_Array:
                case EVEDB::invGroups::Ship_Maintenance_Array:
                case EVEDB::invGroups::Assembly_Array:
                case EVEDB::invGroups::Shield_Hardening_Array:
                case EVEDB::invGroups::Force_Field_Array:
                case EVEDB::invGroups::Corporate_Hangar_Array:
                case EVEDB::invGroups::Stealth_Emitter_Array:
                case EVEDB::invGroups::Scanner_Array:
                case EVEDB::invGroups::Logistics_Array:
                case EVEDB::invGroups::Cynosural_Generator_Array:
                case EVEDB::invGroups::Structure_Repair_Array: {
                    ArraySE* aSE = new ArraySE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, fData);
                    _log(POS__TRACE, "EntityFactory making ArraySE for %s (%u)", eData.itemName.c_str(), eData.itemID);
                    pSSE = aSE;
                } break;
                case EVEDB::invGroups::Silo:
                case EVEDB::invGroups::Moon_Mining:
                case EVEDB::invGroups::Mobile_Reactor: {
                    ReactorSE* rSE = new ReactorSE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, fData);
                    _log(POS__TRACE, "EntityFactory making ReactorSE for %s (%u)", eData.itemName.c_str(), eData.itemID);
                    pSSE = rSE;
                } break;
                default: {
                    StructureSE* sSE = new StructureSE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, fData);
                    _log(POS__TRACE, "EntityFactory making StructureSE for %s (%u)", eData.itemName.c_str(), eData.itemID);
                    pSSE = sSE;
                } break;
            }
            pSSE->Init();
            return pSSE;
        } break;
        case EVEDB::invCategories::SovereigntyStructure: {// SOV structures
            //Create item ref
            StructureItemRef structure = sItemFactory.GetStructureRef(eData.itemID);
            if (structure.get() == nullptr) {
                _log(ITEM__ERROR, "EntityFactory - iRef = nullptr for %s (%u)", eData.itemName.c_str(), eData.itemID);
                return nullptr;
            }
            StructureSE* sSSE(nullptr);
            //Test for type of sov structures
            switch(eData.groupID) {
                case EVEDB::invGroups::Territorial_Claim_Units: {
                    TCUSE* sSE = new TCUSE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, fData);
                    _log(POS__TRACE, "EntityFactory making TCUSE for %s (%u)", eData.itemName.c_str(), eData.itemID);
                    sSSE = sSE;
                } break;
                case EVEDB::invGroups::Sovereignty_Blockade_Units: {
                    SBUSE* sSE = new SBUSE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, fData);
                    _log(POS__TRACE, "EntityFactory making SBUSE for %s (%u)", eData.itemName.c_str(), eData.itemID);
                    sSSE = sSE;
                } break;
                case EVEDB::invGroups::Infrastructure_Hubs: {
                    IHubSE* sSE = new IHubSE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, fData);
                    _log(POS__TRACE, "EntityFactory making IHubSE for %s (%u)", eData.itemName.c_str(), eData.itemID);
                    sSSE = sSE;
                } break;
                default: {
                    StructureSE* sSE = new StructureSE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, fData);
                    _log(POS__ERROR, "EntityFactory Default StructureSE created for %s (%u)", eData.itemName.c_str(), eData.itemID);
                    sSSE = sSE;
                } break;
            }
            sSSE->Init();
            return sSSE;
        } break;
        case EVEDB::invCategories::Orbitals: {           // planet orbitals   these should go into m_staticEntities
            StructureItemRef structure = sItemFactory.GetStructureRef(eData.itemID);
            if (structure.get() == nullptr) {
                _log(ITEM__ERROR, "EntityFactory - iRef = nullptr for %s (%u)", eData.itemName.c_str(), eData.itemID);
                return nullptr;
            }
            CustomsSE* pCoSE(nullptr);
            switch(eData.groupID) {
                case EVEDB::invGroups::Test_Orbitals:
                case EVEDB::invGroups::Orbital_Construction_Platform:
                case EVEDB::invGroups::Orbital_Infrastructure: {
                    pCoSE = new CustomsSE(structure, *(sysMgr.GetServiceMgr()), &sysMgr, fData);
                    //structure->GetMyInventory()->LoadContents();  this is called during structureItem creation
                    if ((eData.planetID) and (eData.groupID != EVEDB::invGroups::Test_Orbitals)) {
                        pCoSE->SetPlanet(eData.planetID);
                        SystemEntity* pPE = sysMgr.GetSE(eData.planetID);
                        if ((pPE != nullptr) and pPE->IsPlanetSE())
                            pPE->GetPlanetSE()->SetCustomsOffice(pCoSE);
                    }
                    pCoSE->Init();
                    _log(POS__TRACE, "EntityFactory making CustomsSE for %s (%u)", eData.itemName.c_str(), eData.itemID);
                } break;
            }
            return pCoSE;
        } break;
        case EVEDB::invCategories::Celestial: {
            // TODO: (just use CelestialEntity class for these until their own classes are written)
            // * WarpGateEntity  <-- Warp_Gate
            // * WormholeEntity  <-- Wormhole
            switch (eData.groupID) {
                case EVEDB::invGroups::Wreck: {
                    WreckContainerRef wreck = sItemFactory.GetWreckContainer(eData.itemID);
                    if (wreck.get() == nullptr) {
                		_log(ITEM__ERROR, "EntityFactory - iRef = nullptr for %s (%u)", eData.itemName.c_str(), eData.itemID);
                		return nullptr;
            		}
                    WreckSE* wSE = new WreckSE(wreck, *(sysMgr.GetServiceMgr()), &sysMgr, fData);
                    wreck->GetMyInventory()->LoadContents();
                    wreck->SetMySE(wSE);
                    _log(ITEM__TRACE, "EntityFactory making WreckSE for %s (%u)", eData.itemName.c_str(), eData.itemID);
                    return wSE;
                } break;
                case EVEDB::invGroups::Audit_Log_Secure_Container:
                case EVEDB::invGroups::Secure_Cargo_Container:
                case EVEDB::invGroups::Cargo_Container:
                case EVEDB::invGroups::Freight_Container:
                case EVEDB::invGroups::Shipping_Crates: {
                    CargoContainerRef contRef = sItemFactory.GetCargoRef(eData.itemID);
                    if (contRef.get() == nullptr) {
                		_log(ITEM__ERROR, "EntityFactory - iRef = nullptr for %s (%u)", eData.itemName.c_str(), eData.itemID);
                		return nullptr;
            		}
                    ContainerSE* cSE = new ContainerSE(contRef, *(sysMgr.GetServiceMgr()), &sysMgr, fData);
                    contRef->GetMyInventory()->LoadContents();
                    contRef->SetMySE(cSE);
                    _log(ITEM__TRACE, "EntityFactory making ContainerSE for %s (%u)", eData.itemName.c_str(), eData.itemID);
                    return cSE;
                } break;
            /** @todo (Allan)  need to separate these by class to create proper SE (started) */
                case EVEDB::invGroups::Biomass:
                case EVEDB::invGroups::Ring:/*wtf*/
                case EVEDB::invGroups::Secondary_Sun:
                case EVEDB::invGroups::Large_Collidable_Object:
                case EVEDB::invGroups::Large_Collidable_Structure:
                case EVEDB::invGroups::Cloud:
                case EVEDB::invGroups::Landmark:
                case EVEDB::invGroups::Comet:
                case EVEDB::invGroups::Destructable_Station_Services:
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
                    CelestialObjectRef celestial = sItemFactory.GetCelestialRef(eData.itemID);
                    if (celestial.get() == nullptr) {
                		_log(ITEM__ERROR, "EntityFactory - iRef = nullptr for %s (%u)", eData.itemName.c_str(), eData.itemID);
                		return nullptr;
            		}
                    CelestialSE* cSE = new CelestialSE(celestial, *(sysMgr.GetServiceMgr()), &sysMgr);
                    _log(ITEM__TRACE, "EntityFactory making CelestialSE for %s (%u)", eData.itemName.c_str(), eData.itemID);
                    return cSE;
                } break;
                case EVEDB::invGroups::Wormhole: {
                    CelestialObjectRef celestial = sItemFactory.GetCelestialRef(eData.itemID);
                    if (celestial.get() == nullptr) {
                		_log(ITEM__ERROR, "EntityFactory - iRef = nullptr for %s (%u)", eData.itemName.c_str(), eData.itemID);
                		return nullptr;
            		}
                    WormholeSE* wSE = new WormholeSE(celestial, *(sysMgr.GetServiceMgr()), &sysMgr);
                    _log(ITEM__TRACE, "EntityFactory making WormholeSE for %s (%u)", eData.itemName.c_str(), eData.itemID);
                    return wSE;
                } break;
                case EVEDB::invGroups::Cosmic_Anomaly:
                case EVEDB::invGroups::Cosmic_Signature: {
                    CelestialObjectRef celestial = sItemFactory.GetCelestialRef(eData.itemID);
                    if (celestial.get() == nullptr) {
                		_log(ITEM__ERROR, "EntityFactory - iRef = nullptr for %s (%u)", eData.itemName.c_str(), eData.itemID);
                		return nullptr;
            		}
                    AnomalySE* aSE = new AnomalySE(celestial, *(sysMgr.GetServiceMgr()), &sysMgr);
                    _log(ITEM__TRACE, "EntityFactory making AnomalySE for %s (%u)", eData.itemName.c_str(), eData.itemID);
                    return aSE;
                } break;
                case EVEDB::invGroups::Warp_Gate: { //accel gate
                    // does this need own item, or celestial, or generic or other?
                    InventoryItemRef iRef = sItemFactory.GetItemRef(eData.itemID);
                    //CelestialObjectRef celestial = sItemFactory.GetCelestialObject(entity.itemID);
                    if (iRef.get() == nullptr) {
                		_log(ITEM__ERROR, "EntityFactory - iRef = nullptr for %s (%u)", eData.itemName.c_str(), eData.itemID);
                		return nullptr;
            		}
                    ItemSystemEntity* iSE = new ItemSystemEntity(iRef, *(sysMgr.GetServiceMgr()), &sysMgr);
                    _log(ITEM__TRACE, "EntityFactory making ISE for %s (%u)", eData.itemName.c_str(), eData.itemID);
                    return iSE;
                } break;
            }
        } break;
        case EVEDB::invCategories::Entity: {            // Entities
            EntityClass eClass = m_routingArray[eData.groupID];

            switch (eClass) {
        	case EntityClass::Container: {     // these are destructible objects found in dungeons
                    // For category=Entity, group=Spawn Container, create a CargoContainer object:
                    /** @todo  this needs its own class....there are 477 types, spawning everything..rats, modules, items, etc. */
                    CargoContainerRef contRef = sItemFactory.GetCargoRef(eData.itemID);
                    if (contRef.get() == nullptr) {
               		_log(ITEM__ERROR, "EntityFactory - iRef = nullptr for %s (%u)", eData.itemName.c_str(), eData.itemID);
               		return nullptr;
                    }
                    ContainerSE* cSE = new ContainerSE(contRef, *(sysMgr.GetServiceMgr()), &sysMgr, fData);
                    contRef->GetMyInventory()->LoadContents();
                    contRef->SetMySE(cSE);
                    _log(ITEM__TRACE, "EntityFactory making ContainerSE for %s (%u)", eData.itemName.c_str(), eData.itemID);
                    return cSE;
                } break;
                case EntityClass::Sentry: {
                    InventoryItemRef sentryRef = sItemFactory.GetItemRef(eData.itemID);
                    if (sentryRef.get() == nullptr) {
               		_log(ITEM__ERROR, "EntityFactory - iRef = nullptr for %s (%u)", eData.itemName.c_str(), eData.itemID);
               		return nullptr;
                    }
                    Sentry* SentrySE = new Sentry(sentryRef, *(sysMgr.GetServiceMgr()), &sysMgr, fData);
                    _log(ITEM__TRACE, "EntityFactory making SentrySE for %s (%u)", eData.itemName.c_str(), eData.itemID);
                    return SentrySE;
                } break;
                // Check for NPC ships/drones here (category 11):   NOT player drones (different category [18])
                case EntityClass::NPC: {
                    InventoryItemRef npcRef = sItemFactory.GetItemRef(eData.itemID);
                    if (npcRef.get() == nullptr) {
               		_log(ITEM__ERROR, "EntityFactory - iRef = nullptr for %s (%u)", eData.itemName.c_str(), eData.itemID);
               		return nullptr;
                    }
                    NPC* npcSE = new NPC(npcRef, *(sysMgr.GetServiceMgr()), &sysMgr, fData);
                    npcSE->Load();
                    sEntityMgr.AddNPC();
                    _log(ITEM__TRACE, "EntityFactory making NPCSE for %s (%u)", eData.itemName.c_str(), eData.itemID);
                    return npcSE;
                } break;
                // may have to create unique class for Billboard (EVEDB::invGroups::Billboard)
                default:
        	case EntityClass::Unknown: {
                    InventoryItemRef iRef = sItemFactory.GetItemRef(eData.itemID);
                    if (iRef.get() == nullptr) {
               		_log(ITEM__ERROR, "EntityFactory - iRef = nullptr for %s (%u)", eData.itemName.c_str(), eData.itemID);
               		return nullptr;
                    }
                    ItemSystemEntity* cSE = new ItemSystemEntity(iRef, *(sysMgr.GetServiceMgr()), &sysMgr);
                    _log(ITEM__TRACE, "EntityFactory making ISE item for %s (%u)", eData.itemName.c_str(), eData.itemID);
                    return cSE;
                } break;
            }
        } break;
        case EVEDB::invCategories::Drone: {             // Player Drones
            InventoryItemRef drone = sItemFactory.GetItemRef(eData.itemID);
            if (drone.get() == nullptr) {
                _log(ITEM__ERROR, "EntityFactory - iRef = nullptr for %s (%u)", eData.itemName.c_str(), eData.itemID);
                return nullptr;
            }
            DroneSE* dSE = new DroneSE(drone, *(sysMgr.GetServiceMgr()), &sysMgr, fData);
            dSE->Init();
            _log(ITEM__TRACE, "EntityFactory making DroneSE for %s (%u)", eData.itemName.c_str(), eData.itemID);
            return dSE;
        } break;
        case EVEDB::invCategories::Charge: {
            switch (eData.groupID) {
                case EVEDB::invGroups::Scanner_Probe: {
                    ProbeItemRef pRef = sItemFactory.GetProbeRef(eData.itemID);
                    if (pRef.get() == nullptr) {
                        _log(ITEM__ERROR, "EntityFactory - iRef = nullptr for %s (%u)", eData.itemName.c_str(), eData.itemID);
                        return nullptr;
                    }
                    // make sure these are owned by eve system
                    pRef->ChangeOwner(1);
                    //these probes are abandoned and offline.  give them 5h lifetime
                    ProbeSE* pSE = new ProbeSE(pRef, *(sysMgr.GetServiceMgr()), &sysMgr);
                    _log(ITEM__TRACE, "EntityFactory making ProbeSE for %s (%u)", eData.itemName.c_str(), eData.itemID);
                    return pSE;
                } break;
                case EVEDB::invGroups::Survey_Probe: {
                    sLog.Warning("BuildEntity", "Called for Survey_Probe");
                } break;
                case EVEDB::invGroups::Warp_Disruption_Probe: {
                    sLog.Warning("BuildEntity", "Called for Warp_Disruption_Probe");
                } break;
            }
        } break;
    }
    sLog.Warning("BuildEntity", "Unhandled dynamic entity category %u for item %s(%u) [type: %u]", eData.categoryID, eData.itemName.c_str(), eData.itemID, eData.typeID);

    if (sConfig.server.StackTrace)
        EvE::traceStack();

    return nullptr;
}

void EntityFactory::Initialize() {
    // npc ships in catID:11
    m_routingArray[EVEDB::invGroups::Police_Drone] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Pirate_Drone] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::LCO_Drone] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Tutorial_Drone] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Rogue_Drone] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Faction_Drone] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Convoy] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Convoy_Drone] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Concord_Drone] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Mission_Drone] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Deadspace_Overseer] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Customs_Official] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Deadspace_Overseer_s_Structure] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Deadspace_Overseer_s_Sentry] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Deadspace_Overseer_s_Belongings] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Storyline_Frigate] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Storyline_Cruiser] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Storyline_Battleship] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Storyline_Mission_Frigate] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Storyline_Mission_Cruiser] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Storyline_Mission_Battleship] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Destructible_Agents_In_Space] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Large_Collidable_Ship] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Mission_Fighter_Drone] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Mission_Faction_Transports] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Mission_Faction_Industrials] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Deadspace_Sleeper_Sleepless_Sentinel] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Deadspace_Sleeper_Awakened_Sentinel] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Deadspace_Sleeper_Emergent_Sentinel] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Mission_Faction_Cruiser] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Mission_Faction_Frigate] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Incursion_Sanshas_Nation_Industrial] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Incursion_Sanshas_Nation_Capital] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Incursion_Sanshas_Nation_Frigate] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Incursion_Sanshas_Nation_Cruiser] = EntityClass::NPC;
    m_routingArray[EVEDB::invGroups::Incursion_Sanshas_Nation_Battleship] = EntityClass::NPC;

    // Use fast loops to flatten out all database range brackets
    for (int i = EVEDB::invGroups::Asteroid_Angel_Cartel_Frigate; i <= EVEDB::invGroups::Asteroid_Serpentis_BattleCruiser; ++i)
        m_routingArray[i] = EntityClass::NPC;

    for (int i = EVEDB::invGroups::Asteroid_Rogue_Drone_Battlecruiser; i <= EVEDB::invGroups::Asteroid_Rogue_Drone_Swarm; ++i)
        m_routingArray[i] = EntityClass::NPC;

    for (int i = EVEDB::invGroups::Asteroid_Angel_Cartel_Commander_Frigate; i <= EVEDB::invGroups::Asteroid_Serpentis_Commander_Frigate; ++i)
        m_routingArray[i] = EntityClass::NPC;

    for (int i = EVEDB::invGroups::Asteroid_Rogue_Drone_Commander_Battlecruiser; i <= EVEDB::invGroups::Asteroid_Serpentis_Commander_Battleship; ++i)
        m_routingArray[i] = EntityClass::NPC;


    for (int i = EVEDB::invGroups::Deadspace_Angel_Cartel_BattleCruiser; i <= EVEDB::invGroups::Deadspace_Angel_Cartel_Frigate; ++i)
        m_routingArray[i] = EntityClass::NPC;

    for (int i = EVEDB::invGroups::Deadspace_Blood_Raiders_BattleCruiser; i <= EVEDB::invGroups::Deadspace_Blood_Raiders_Frigate; ++i)
        m_routingArray[i] = EntityClass::NPC;

    for (int i = EVEDB::invGroups::Deadspace_Guristas_BattleCruiser; i <= EVEDB::invGroups::Deadspace_Guristas_Frigate; ++i)
        m_routingArray[i] = EntityClass::NPC;

    for (int i = EVEDB::invGroups::Deadspace_Sanshas_Nation_BattleCruiser; i <= EVEDB::invGroups::Deadspace_Sanshas_Nation_Frigate; ++i)
        m_routingArray[i] = EntityClass::NPC;

    for (int i = EVEDB::invGroups::Deadspace_Serpentis_BattleCruiser; i <= EVEDB::invGroups::Deadspace_Serpentis_Frigate; ++i)
        m_routingArray[i] = EntityClass::NPC;

    for (int i = EVEDB::invGroups::Deadspace_Sleeper_Sleepless_Defender; i <= EVEDB::invGroups::Deadspace_Sleeper_Emergent_Patroller; ++i)
        m_routingArray[i] = EntityClass::NPC;

    for (int i = EVEDB::invGroups::Mission_Amarr_Empire_Frigate; i <= EVEDB::invGroups::Mission_Minmatar_Republic_Battleship; ++i)
        m_routingArray[i] = EntityClass::NPC;

    for (int i = EVEDB::invGroups::Mission_Generic_Battleships; i <= EVEDB::invGroups::Mission_Generic_Destroyers; ++i)
        m_routingArray[i] = EntityClass::NPC;

    for (int i = EVEDB::invGroups::Mission_Amarr_Empire_Carrier; i <= EVEDB::invGroups::Mission_Minmatar_Republic_Carrier; ++i)
        m_routingArray[i] = EntityClass::NPC;

    for (int i = EVEDB::invGroups::Incursion_Sanshas_Nation_Industrial; i <= EVEDB::invGroups::Incursion_Sanshas_Nation_Battleship; ++i)
        m_routingArray[i] = EntityClass::NPC;

	// others in catID 11:
    m_routingArray[EVEDB::invGroups::Spawn_Container] = EntityClass::Container;
    m_routingArray[EVEDB::invGroups::Sentry_Gun] = EntityClass::Sentry;
    m_routingArray[EVEDB::invGroups::Protective_Sentry_Gun] = EntityClass::Sentry;
    m_routingArray[EVEDB::invGroups::Destructible_Sentry_Gun] = EntityClass::Sentry;
    m_routingArray[EVEDB::invGroups::Mobile_Sentry_Gun] = EntityClass::Sentry;
    m_routingArray[EVEDB::invGroups::Billboard] = EntityClass::Unknown;
}
