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
    Author:     Zhur
*/

#include "eve-server.h"

#include "Client.h"
#include "EntityList.h"
#include "effects/EffectsProcessor.h"
#include "character/Skill.h"
#include "manufacturing/Blueprint.h"
#include "pos/Structure.h"
#include "ship/Ship.h"
#include "station/Station.h"
#include "system/Asteroid.h"
#include "system/Celestial.h"
#include "system/Container.h"

/*
 * InventoryItem
 */
InventoryItem::InventoryItem(
    ItemFactory &_factory,
    uint32 _itemID,
    const ItemType &_type,
    const ItemData &_data)
: RefObject( 0 ),
  mAttributeMap(*this),
  m_saveTimer(0),
  m_factory(_factory),
  m_itemID(_itemID),
  m_itemName(_data.name),
  m_type(_type),
  m_ownerID(_data.ownerID),
  m_locationID(_data.locationID),
  m_flag(_data.flag),
  m_contraband(_data.contraband),
  m_singleton(_data.singleton),
  m_quantity(_data.quantity),
  m_position(_data.position),
  m_customInfo(_data.customInfo)
{
    // assert for data consistency
    assert(_data.typeID == _type.id());

    //m_saveTimerExpiryTime = ITEM_DB_SAVE_TIMER_EXPIRY * 60 * 1000;      // 10 minutes in milliseconds
    //m_saveTimer.SetTimer(m_saveTimerExpiryTime);                        // set timer in milliseconds
    m_saveTimer.Disable();                                              // disable timer by default

    _log(ITEM__TRACE, "Created Generic Item %p for item %s (%u).", this, m_itemName.c_str(), m_itemID);
}

InventoryItem::~InventoryItem()
{
    // item should call save before object is removed.
    // if item is being removed during shutdow, item factory is responsible for saving loaded items
}

InventoryItemRef InventoryItem::Load(ItemFactory &factory, uint32 itemID)
{
    return InventoryItem::Load<InventoryItem>( factory, itemID );
}

InventoryItemRef InventoryItem::SpawnItem(ItemFactory &factory, uint32 itemID, const ItemData &data)
{
    const ItemType *type = factory.GetType( data.typeID );
	InventoryItemRef itemRef = InventoryItemRef( new InventoryItem(factory, itemID, *type, data) );
    itemRef->_Load();
	return itemRef;
}

uint32 InventoryItem::CreateItemID(ItemFactory &factory, ItemData &data) {
    // obtain type of new item
    const ItemType *t = factory.GetType(data.typeID);
    if (!t) {
        codelog(ITEM__ERROR, "Invalid type returned for typeID %u", data.typeID);
        return 0;
    }
    // fix the name (if empty)
    if (data.name.empty()) data.name = t->name();

    // insert new entry into DB
    return factory.db().NewItem(data);
}

/* This Spawn function is meant for in-memory only items created from the following categorys...
 *  EVEDB::invCategories::Entity (for npcs)
 *  EVEDB::invCategories::Charge (for launched missiles only)
 *
 * these items meant to never be saved to database
 * and be thrown away on server shutdown.
 * Updated 29May15 -Allan
 */
uint32 InventoryItem::CreateTempItemID(ItemFactory &factory, ItemData &data) {
    // obtain type of new item
    // this also checks that the type is valid
    const ItemType *t = factory.GetType(data.typeID);
    if (!t)
        return 0;

    // fix the name (if empty)
    if (data.name.empty())
        data.name = t->name();

    // Get a new Entity ID from ItemFactory's ID Authority:
    if (t->categoryID() == EVEDB::invCategories::Entity) // may need more testing to verify that ONLY NPC's use this method
        return factory.GetNextNPCID();
    else if (data.flag == EVEItemFlags::flagMissile)
        return factory.GetNextMissileID();
    else
        return factory.GetNextTempID();
}

bool InventoryItem::_Load() {
    if (!mAttributeMap.Load()) {
        _log(ITEM__WARNING, "%s (%u): Failed to load attribute map.", itemName().c_str(), itemID());
        return false;
    }

    return true;
}

template<class _Ty>
RefPtr<_Ty> InventoryItem::_LoadItem(ItemFactory &factory, uint32 itemID, const ItemType &type, const ItemData &data) {
    // See what to do next:
    switch( type.categoryID() ) {
        /* not handled yet...
        case EVEDB::invCategories::Material:
        case EVEDB::invCategories::Module:
        case EVEDB::invCategories::Charge:
        case EVEDB::invCategories::Trading:
        case EVEDB::invCategories::Bonus:
        case EVEDB::invCategories::Commodity:
        case EVEDB::invCategories::Implant:
        case EVEDB::invCategories::Deployable:
        case EVEDB::invCategories::Reaction:
            not needed...
        case EVEDB::invCategories::_System:
        case EVEDB::invCategories::Drone:
        case EVEDB::invCategories::Accessories:
            */
            /** @todo (Allan) these need work ...for now, load default item
             *        case EVEDB::invCategories::Asteroid:
             *            return AsteroidItem::_LoadItem<AsteroidItem>( factory, itemID, type, data );
             */
            case EVEDB::invCategories::Orbitals:
            case EVEDB::invCategories::Structure: {  // this is for all Orbital structure types (POS, customs offices, etc)
                return StructureItem::_LoadItem<StructureItem>( factory, itemID, type, data );
            }
            case EVEDB::invCategories::Station: {
                return StationItem::_LoadItem<StationItem>( factory, itemID, type, data );
            }
            case EVEDB::invCategories::Blueprint: {
                return Blueprint::_LoadItem<Blueprint>( factory, itemID, type, data );
            }
            case EVEDB::invCategories::Ship: {
                return ShipItem::_LoadItem<ShipItem>( factory, itemID, type, data );
            }
            case EVEDB::invCategories::Skill: {
                return Skill::_LoadItem<Skill>( factory, itemID, type, data );
            }
            case EVEDB::invCategories::Owner: {
                return Character::_LoadItem<Character>( factory, itemID, type, data );
            }
            case EVEDB::invCategories::Celestial: {
                if (type.groupID() == EVEDB::invGroups::Wreck)
                    return WreckContainer::_LoadItem<WreckContainer>( factory, itemID, type, data );
                else if ((type.groupID() == EVEDB::invGroups::Secure_Cargo_Container)
                    or (type.groupID() == EVEDB::invGroups::Audit_Log_Secure_Container)
                    or (type.groupID() == EVEDB::invGroups::Freight_Container)
                    or (type.groupID() == EVEDB::invGroups::Cargo_Container) )
                    return CargoContainer::_LoadItem<CargoContainer>( factory, itemID, type, data );
                else if (type.groupID() == EVEDB::invGroups::Force_Field)   // POS force fields  - not sure if these need their own class
                    return InventoryItemRef( new InventoryItem( factory, itemID, type, data ) );
                else
                    return CelestialObject::_LoadItem<CelestialObject>( factory, itemID, type, data );
            }
            case EVEDB::invCategories::Entity: {
                //  added checks for all npc's   -allan 26Dec14
                if ((type.groupID() == EVEDB::invGroups::Sentry_Gun)
                    or  (type.groupID() == EVEDB::invGroups::Protective_Sentry_Gun)
                    or  (type.groupID() == EVEDB::invGroups::Police_Drone)
                    or  (type.groupID() == EVEDB::invGroups::Pirate_Drone)
                    or  (type.groupID() == EVEDB::invGroups::LCO_Drone)
                    or  (type.groupID() == EVEDB::invGroups::Tutorial_Drone)
                    or  (type.groupID() == EVEDB::invGroups::Rogue_Drone)
                    or  (type.groupID() == EVEDB::invGroups::Faction_Drone)
                    or  (type.groupID() == EVEDB::invGroups::Convoy)
                    or  (type.groupID() == EVEDB::invGroups::Convoy_Drone)
                    or  (type.groupID() == EVEDB::invGroups::Concord_Drone)
                    or  (type.groupID() == EVEDB::invGroups::Mission_Drone)
                    or  (type.groupID() == EVEDB::invGroups::Destructible_Sentry_Gun)
                    or  (type.groupID() == EVEDB::invGroups::Deadspace_Overseer)
                    or  (type.groupID() == EVEDB::invGroups::Customs_Official)
                    or  (type.groupID() == EVEDB::invGroups::Deadspace_Overseer_s_Structure)
                    or  (type.groupID() == EVEDB::invGroups::Deadspace_Overseer_s_Sentry)
                    or  (type.groupID() == EVEDB::invGroups::Deadspace_Overseer_s_Belongings)
                    or  (type.groupID() == EVEDB::invGroups::Storyline_Frigate)
                    or  (type.groupID() == EVEDB::invGroups::Storyline_Cruiser)
                    or  (type.groupID() == EVEDB::invGroups::Storyline_Battleship)
                    or  (type.groupID() == EVEDB::invGroups::Storyline_Mission_Frigate)
                    or  (type.groupID() == EVEDB::invGroups::Storyline_Mission_Cruiser)
                    or  (type.groupID() == EVEDB::invGroups::Storyline_Mission_Battleship)
                    or  ((type.groupID() >= EVEDB::invGroups::Asteroid_Angel_Cartel_Frigate) and (type.groupID() <= EVEDB::invGroups::Asteroid_Serpentis_BattleCruiser))
                    or  ((type.groupID() >= EVEDB::invGroups::Deadspace_Angel_Cartel_BattleCruiser) and (type.groupID() <= EVEDB::invGroups::Deadspace_Angel_Cartel_Frigate))
                    or  ((type.groupID() >= EVEDB::invGroups::Deadspace_Blood_Raiders_BattleCruiser) and (type.groupID() <= EVEDB::invGroups::Deadspace_Blood_Raiders_Frigate))
                    or  ((type.groupID() >= EVEDB::invGroups::Deadspace_Guristas_BattleCruiser) and (type.groupID() <= EVEDB::invGroups::Deadspace_Guristas_Frigate))
                    or  ((type.groupID() >= EVEDB::invGroups::Deadspace_Sanshas_Nation_BattleCruiser) and (type.groupID() <= EVEDB::invGroups::Deadspace_Sanshas_Nation_Frigate))
                    or  ((type.groupID() >= EVEDB::invGroups::Deadspace_Serpentis_BattleCruiser) and (type.groupID() <= EVEDB::invGroups::Deadspace_Serpentis_Frigate))
                    or  ((type.groupID() >= EVEDB::invGroups::Mission_Amarr_Empire_Frigate) and (type.groupID() <= EVEDB::invGroups::Mission_Minmatar_Republic_Battleship))
                    or  (type.groupID() == EVEDB::invGroups::Destructible_Agents_In_Space)
                    or  ((type.groupID() >= EVEDB::invGroups::Asteroid_Rogue_Drone_Battlecruiser) and (type.groupID() <= EVEDB::invGroups::Asteroid_Rogue_Drone_Swarm))
                    or  (type.groupID() == EVEDB::invGroups::Large_Collidable_Ship)
                    or  ((type.groupID() >= EVEDB::invGroups::Asteroid_Angel_Cartel_Commander_Frigate) and (type.groupID() <= EVEDB::invGroups::Asteroid_Serpentis_Commander_Frigate))
                    or  ((type.groupID() >= EVEDB::invGroups::Mission_Generic_Battleships) and (type.groupID() <= EVEDB::invGroups::Mission_Generic_Destroyers))
                    or  ((type.groupID() >= EVEDB::invGroups::Asteroid_Rogue_Drone_Commander_Battlecruiser) and (type.groupID() <= EVEDB::invGroups::Asteroid_Serpentis_Commander_Battleship))
                    or  (type.groupID() == EVEDB::invGroups::Mission_Fighter_Drone)
                    or  ((type.groupID() >= EVEDB::invGroups::Mission_Amarr_Empire_Carrier) and (type.groupID() <= EVEDB::invGroups::Mission_Minmatar_Republic_Carrier))
                    or  (type.groupID() == EVEDB::invGroups::Mission_Faction_Transports)
                    or  (type.groupID() == EVEDB::invGroups::Mission_Faction_Industrials)
                    or  (type.groupID() == EVEDB::invGroups::Deadspace_Sleeper_Sleepless_Sentinel)
                    or  (type.groupID() == EVEDB::invGroups::Deadspace_Sleeper_Awakened_Sentinel)
                    or  (type.groupID() == EVEDB::invGroups::Deadspace_Sleeper_Emergent_Sentinel)
                    or  ((type.groupID() >= EVEDB::invGroups::Deadspace_Sleeper_Sleepless_Defender) and (type.groupID() <= EVEDB::invGroups::Deadspace_Sleeper_Emergent_Patroller))
                    or  (type.groupID() == EVEDB::invGroups::Mission_Faction_Cruiser)
                    or  (type.groupID() == EVEDB::invGroups::Mission_Faction_Frigate)
                    or  (type.groupID() == EVEDB::invGroups::Incursion_Sanshas_Nation_Industrial)
                    or  (type.groupID() == EVEDB::invGroups::Incursion_Sanshas_Nation_Capital)
                    or  (type.groupID() == EVEDB::invGroups::Incursion_Sanshas_Nation_Frigate)
                    or  (type.groupID() == EVEDB::invGroups::Incursion_Sanshas_Nation_Cruiser)
                    or  (type.groupID() == EVEDB::invGroups::Incursion_Sanshas_Nation_Battleship)) {
                    /*  these should probably get an NPCItem ItemType eventually */
                    return InventoryItemRef( new InventoryItem( factory, itemID, type, data ) );
                } else
                    return CelestialObject::_LoadItem<CelestialObject>( factory, itemID, type, data );
            }
            default:
                _log(ITEM__MESSAGE, "item %u (type %u, cat %u) is not handled in  InventoryItem::_LoadItem.", itemID, type.id(), type.categoryID());
                break;
    }
    // Generic item, create one:
    return InventoryItemRef( new InventoryItem( factory, itemID, type, data ) );
}

InventoryItemRef InventoryItem::Spawn(ItemFactory &factory, ItemData &data)
{
    // obtain type of new item
    const ItemType *t = factory.GetType( data.typeID );
    if (!t) return InventoryItemRef();

    switch( t->categoryID() ) {
        //! TODO not handled.
        case EVEDB::invCategories::_System:
        case EVEDB::invCategories::Material:
        case EVEDB::invCategories::Accessories:
        case EVEDB::invCategories::Trading:
        case EVEDB::invCategories::Bonus:
        case EVEDB::invCategories::Commodity:
        case EVEDB::invCategories::Implant:
        case EVEDB::invCategories::Reaction:
            break;
        case EVEDB::invCategories::Skill: {
            return Skill::Spawn( factory, data );
        }
        case EVEDB::invCategories::Owner: {
            return Character::Spawn( factory, data );
        }
        case EVEDB::invCategories::Ship: {
            return ShipItem::Spawn( factory, data );
        }
        case EVEDB::invCategories::Orbitals:
        case EVEDB::invCategories::Structure:
        case EVEDB::invCategories::SovereigntyStructure: {
            /*  this is for all Orbital and Structure items */
            return StructureItem::Spawn( factory, data );
        }
        case EVEDB::invCategories::Blueprint: {
            BlueprintData bpData;
                bpData.runs = -1;
                bpData.copy = false;
                bpData.mLevel = 0;
                bpData.pLevel = 0;
            return Blueprint::Spawn( factory, data, bpData );
        }
        case EVEDB::invCategories::Asteroid: {
            uint32 itemID = InventoryItem::CreateItemID( factory, data );
            if (!itemID)
                return InventoryItemRef();
            InventoryItemRef itemRef = InventoryItem::Load( factory, itemID );
            if (!itemRef)
                return InventoryItemRef();
            // THESE SHOULD BE MOVED INTO AN Asteroid::Spawn() function that does not exist yet
            // Create default dynamic attributes in the AttributeMap:
            itemRef->SetAttribute(AttrRadius,         itemRef->type().radius());       // Radius
            itemRef->SetAttribute(AttrVolume,         itemRef->type().volume());       // Volume
            return itemRef;
        }
        case EVEDB::invCategories::Module:
        case EVEDB::invCategories::Drone:
        case EVEDB::invCategories::Deployable: {
            // Spawn generic item:
            uint32 itemID = InventoryItem::CreateItemID( factory, data );
            if (!itemID)
                return InventoryItemRef();
            InventoryItemRef itemRef = InventoryItem::Load( factory, itemID );
            if (!itemRef)
                return InventoryItemRef();
            // THESE SHOULD BE MOVED INTO A _type::Spawn() function that does not exist yet
            itemRef->SetAttribute(AttrMass,           itemRef->type().mass());           // Mass
            itemRef->SetAttribute(AttrRadius,         itemRef->type().radius());       // Radius
            itemRef->SetAttribute(AttrVolume,         itemRef->type().volume());       // Volume
            itemRef->SetAttribute(AttrCapacity,       itemRef->type().capacity());   // Capacity
            return itemRef;
        }
        case EVEDB::invCategories::Charge: {
            switch (data.flag) {
                case EVEItemFlags::flagMissile: {
                    // Spawn launched missile item in EVEMU_MISSILE_ID range and does NOT save missile to db
                    uint32 itemID = InventoryItem::CreateTempItemID( factory, data );
                    if (!itemID)
                        return InventoryItemRef();

                    InventoryItemRef itemRef = InventoryItem::SpawnItem( factory, itemID, data );
                    if (!itemRef)
                        return InventoryItemRef();
                    // THESE SHOULD BE MOVED INTO A Charge::Spawn() function that does not exist yet
                    itemRef->SetAttribute(AttrMass,       itemRef->type().mass());           // Mass
                    itemRef->SetAttribute(AttrRadius,     itemRef->type().radius());       // Radius
                    itemRef->SetAttribute(AttrVolume,     itemRef->type().volume());       // Volume
                    itemRef->SetAttribute(AttrCapacity,   itemRef->type().capacity());   // Capacity
                    return itemRef;
                }
                default: {
                    // Spawn generic item:
                    uint32 itemID = InventoryItem::CreateItemID( factory, data );
                    if (!itemID)
                        return InventoryItemRef();

                    InventoryItemRef itemRef = InventoryItem::Load( factory, itemID );
                    if (!itemRef)
                        return InventoryItemRef();
                    // THESE SHOULD BE MOVED INTO A Charge::Spawn() function that does not exist yet
                    itemRef->SetAttribute(AttrMass,       itemRef->type().mass());           // Mass
                    itemRef->SetAttribute(AttrRadius,     itemRef->type().radius());       // Radius
                    itemRef->SetAttribute(AttrVolume,     itemRef->type().volume());       // Volume
                    itemRef->SetAttribute(AttrCapacity,   itemRef->type().capacity());   // Capacity
                    return itemRef;
                }
            }
            _log(ITEM__ERROR, "Unhandled charge spawn");
        }
        case EVEDB::invCategories::Entity: {
			// Spawn generic item for Entities at this time:
            uint32 itemID = InventoryItem::CreateItemID( factory, data );
            //uint32 itemID = InventoryItem::CreateTempItemID( factory, data );		// Use this to prevent entity from being stored in DB
			if (!itemID)
				return InventoryItemRef();
			 InventoryItemRef itemRef = InventoryItem::Load( factory, itemID );
            if (!itemRef)
                return InventoryItemRef();
			return itemRef;
        }
        case EVEDB::invCategories::Station: {
            uint32 itemID = StationItem::CreateItemID( factory, data );
            if (!itemID)
                return StationItemRef();
            StationItemRef stationRef = StationItem::Load( factory, itemID );
            if (!stationRef)
                return StationItemRef();
            // THESE SHOULD BE MOVED INTO A Station::Spawn() function that does not exist yet
            stationRef->SetAttribute(AttrShieldCharge,  stationRef->GetAttribute(AttrShieldCapacity));     // Shield Charge
            stationRef->SetAttribute(AttrArmorDamage,   0.0);                                         // Armor Damage
            stationRef->SetAttribute(AttrMass,           stationRef->type().mass());           // Mass
            stationRef->SetAttribute(AttrRadius,         stationRef->type().radius());       // Radius
            stationRef->SetAttribute(AttrVolume,         stationRef->type().volume());       // Volume
            stationRef->SetAttribute(AttrCapacity,       stationRef->type().capacity());   // Capacity
            return stationRef;
        }
        case EVEDB::invCategories::Celestial: {
            if ( (t->groupID() == EVEDB::invGroups::Secure_Cargo_Container)
                or (t->groupID() == EVEDB::invGroups::Cargo_Container)
                or (t->groupID() == EVEDB::invGroups::Freight_Container)
                or (t->groupID() == EVEDB::invGroups::Audit_Log_Secure_Container)
                or (t->groupID() == EVEDB::invGroups::Spawn_Container)
                or (t->groupID() == EVEDB::invGroups::Mission_Container))
            {
                // Spawn new Cargo Container
                uint32 itemID = CargoContainer::CreateItemID( factory, data );
                if (!itemID)
                    return CargoContainerRef();
                CargoContainerRef cargoRef = CargoContainer::Load( factory, itemID );
                if (!cargoRef)
                    return CargoContainerRef();
                // THESE SHOULD BE MOVED INTO A CargoContainer::Spawn() function that does not exist yet
                cargoRef->SetAttribute(AttrShieldCharge,  cargoRef->GetAttribute(AttrShieldCapacity));  // Shield Charge
                cargoRef->SetAttribute(AttrArmorDamage,   0.0);                                               // Armor Damage
                cargoRef->SetAttribute(AttrMass,          cargoRef->type().mass());          // Mass
                cargoRef->SetAttribute(AttrRadius,        cargoRef->type().radius());        // Radius
                cargoRef->SetAttribute(AttrVolume,        cargoRef->GetPackagedVolume());        // Volume
                cargoRef->SetAttribute(AttrCapacity,      cargoRef->type().capacity());      // Capacity
                return cargoRef;
            } else if (t->groupID() == EVEDB::invGroups::Wreck) {
                // Spawn new Wreck Container
                uint32 itemID = WreckContainer::CreateItemID( factory, data );
                if (!itemID)
                    return WreckContainerRef();
                WreckContainerRef wreckRef = WreckContainer::Load( factory, itemID );
                if (!wreckRef)
                    return WreckContainerRef();
                // THESE SHOULD BE MOVED INTO A WreckContainer::Spawn() function that does not exist yet
                wreckRef->SetAttribute(AttrShieldCharge,  wreckRef->GetAttribute(AttrShieldCapacity));  // Shield Charge
                wreckRef->SetAttribute(AttrArmorDamage,   0.0);                                               // Armor Damage
                wreckRef->SetAttribute(AttrMass,          wreckRef->type().mass());          // Mass
                wreckRef->SetAttribute(AttrRadius,        wreckRef->type().radius());        // Radius
                wreckRef->SetAttribute(AttrVolume,        wreckRef->type().volume());        // Volume
                wreckRef->SetAttribute(AttrCapacity,      wreckRef->type().capacity());      // Capacity
                return wreckRef;
            } else if (t->groupID() == EVEDB::invGroups::Force_Field) {
                // Spawn force field item in EVEMU_TEMP_ENTITY_ID range and does NOT save Force_Field to db
                uint32 itemID = InventoryItem::CreateTempItemID( factory, data );
                if (!itemID)
                    return InventoryItemRef();
                InventoryItemRef itemRef = InventoryItem::SpawnItem( factory, itemID, data );
                if (!itemRef)
                    return InventoryItemRef();
                return itemRef;
            } else {
                // Spawn new Celestial Object
                uint32 itemID = CelestialObject::CreateItemID( factory, data );
                if (!itemID)
                    return CelestialObjectRef();
                CelestialObjectRef celestialRef = CelestialObject::Load( factory, itemID );
                if (!celestialRef)
                    return CelestialObjectRef();
                if ((t->groupID() == EVEDB::invGroups::Beacon) or (t->groupID() == EVEDB::invGroups::Effect_Beacon))
                    celestialRef->SetAttribute(AttrIsGlobal,       1);
                return celestialRef;
            }
        }
    }

    // Spawn generic item:
    uint32 itemID = InventoryItem::CreateItemID( factory, data );
    if (!itemID)
        return InventoryItemRef();
    InventoryItemRef itemRef = InventoryItem::Load( factory, itemID );
    if (!itemRef)
        return InventoryItemRef();
    itemRef->SetAttribute(AttrMass,           itemRef->type().mass());           // Mass
    itemRef->SetAttribute(AttrRadius,         itemRef->type().radius());       // Radius
    itemRef->SetAttribute(AttrVolume,         itemRef->GetPackagedVolume());       // Volume
    itemRef->SetAttribute(AttrCapacity,       itemRef->type().capacity());   // Capacity
    return itemRef;
}

uint32 InventoryItem::GetPackagedVolume()
{
    if (m_singleton)
        return m_type.volume();

    if ((m_type.categoryID() == EVEDB::invCategories::Ship)
        or (m_type.categoryID() == EVEDB::invCategories::Celestial)) {
        // these volumes are hard-coded in client.
        switch (m_type.groupID()) {
            case 29:  //   Capsule
            case 31:  //   Shuttle
            case 1022: {  //     Prototype Exploration Ship
                return 500;
            }
            case 12:    //Cargo Container
            case 306:   //Spawn Container
            case 340:   //Secure Cargo Container
            case 448:   //Audit Log Secure Container
            case 649:   //Freight Container
            case 952: {  //Mission Container
                return 1000;
            }
            case 324: //    Assault Ship
            case 830: //      Covert Ops
            case 893: //      Electronic Attack Ship
            case 25:  //   Frigate
            case 831: //      Interceptor
            case 237: //      Rookie ship
            case 834: { //      Stealth Bomber
                return 2500;
            }
            case 543: //      Exhumer
            case 463: { //      Mining Barge
                return 3750;
            }
            case 541:  //      Interdictor
            case 420:  //      Destroyer
            case 963: { //      Strategic Cruiser
                return 5000;
            }
            case 906: //      Combat Recon Ship
            case 26:  //   Cruiser
            case 833: //      Force Recon Ship
            case 358: //      Heavy Assault Ship
            case 894: //      Heavy Interdictor
            case 832: { //      Logistics
                return 10000;
            }
            case 419: //      Battlecruiser
            case 540: { //      Command Ship
                return 15000;
            }
            case 28:  //   Industrial
            case 380: {  //      Transport Ship
                return 20000;
            }
            case 27:  //   Battleship
            case 900: //      Marauder
            case 898: //      Black Ops
            case 381: { //      Elite Battleship
                return 50000;
            }
            case 941: {  //      Industrial Command Ship
                return 500000;
            }
            case 883: //      Capital Industrial Ship
            case 547: //      Carrier
            case 485: //      Dreadnought
            case 513: //      Freighter
            case 902: //      Jump Freighter
            case 659: { //      Supercarrier
                return 1000000;
            }
            case 30: {  //   Titan
                return 10000000;
            }
        }
    }
    // catchall
    return m_type.volume();
}

void InventoryItem::Delete() {
    if (!IsNPCCorp(ownerID())) {
        //first, get out of client's sight.
        //this also removes us from our inventory.
        Move(0);
        ChangeOwner(2);
    }

    //take ourself out of the DB
    m_factory.db().DeleteItem( itemID() );

    mAttributeMap.Delete();

    //delete ourselves from factory cache
    m_factory.RemoveItem( itemID() );
}

PyPackedRow* InventoryItem::GetItemStatusRow() const {
    DBRowDescriptor* header = new DBRowDescriptor;
        header->AddColumn( "instanceID",    DBTYPE_I8 );
        header->AddColumn( "online",        DBTYPE_BOOL );
        header->AddColumn( "damage",        DBTYPE_R8 );
        header->AddColumn( "charge",        DBTYPE_R8 );
        header->AddColumn( "skillPoints",   DBTYPE_I4 );
        header->AddColumn( "armorDamage",   DBTYPE_R8 );
        header->AddColumn( "shieldCharge",  DBTYPE_R8 );
        header->AddColumn( "incapacitated", DBTYPE_BOOL );
    PyPackedRow* row = new PyPackedRow( header );
    GetItemStatusRow( row );
    return row;
}

void InventoryItem::GetItemStatusRow( PyPackedRow* into ) const {
    into->SetField( "instanceID",    new PyLong( itemID() ) );
    into->SetField( "online",        new PyBool( (mAttributeMap.HasAttribute(AttrIsOnline) ? GetAttribute(AttrIsOnline).get_int() : false) ) );
    into->SetField( "damage",        new PyFloat( (mAttributeMap.HasAttribute(AttrDamage) ? GetAttribute(AttrDamage).get_float() : 0) ) );
    into->SetField( "charge",        new PyFloat( (mAttributeMap.HasAttribute(AttrCapacitorCharge) ? GetAttribute(AttrCapacitorCharge).get_float() : 0) ) );
    into->SetField( "skillPoints",   new PyInt( (mAttributeMap.HasAttribute(AttrSkillPoints) ? GetAttribute(AttrSkillPoints).get_int() : 0) ) );
    into->SetField( "armorDamage",   new PyFloat( (mAttributeMap.HasAttribute(AttrArmorDamageAmount) ? GetAttribute(AttrArmorDamageAmount).get_float() : 0.0) ) );
    into->SetField( "shieldCharge",  new PyFloat( (mAttributeMap.HasAttribute(AttrShieldCharge) ? GetAttribute(AttrShieldCharge).get_float() : 0.0) ) );
    into->SetField( "incapacitated", new PyBool( (mAttributeMap.HasAttribute(AttrIsIncapacitated) ? GetAttribute(AttrIsIncapacitated).get_int() : false) ) );
}

/*  this is charge info for the module in question  */
PyPackedRow* InventoryItem::GetChargeStatusRow(uint32 shipID) const {
    DBRowDescriptor* header = new DBRowDescriptor;
        header->AddColumn( "instanceID", DBTYPE_I8 );
        header->AddColumn( "flagID",     DBTYPE_I2 );
        header->AddColumn( "typeID",     DBTYPE_I4 );
        header->AddColumn( "quantity",   DBTYPE_I4 );
    PyPackedRow* row = new PyPackedRow( header );
    GetChargeStatusRow(shipID, row);
    return row;
}

void InventoryItem::GetChargeStatusRow(uint32 shipID, PyPackedRow* into) const {
    into->SetField( "instanceID",    new PyLong( shipID ) );  /* this is shipID */
    into->SetField( "flagID",        new PyInt( m_flag ) );
    into->SetField( "typeID",        new PyInt( typeID() ) );
    into->SetField( "quantity",      new PyInt( quantity()) );
}

PyPackedRow* InventoryItem::GetModuleStatusRow() const {
    DBRowDescriptor* header = new DBRowDescriptor;
    header->AddColumn( "instanceID", DBTYPE_I8 );
    header->AddColumn( "flagID",     DBTYPE_I2 );
    header->AddColumn( "typeID",     DBTYPE_I4 );
    header->AddColumn( "quantity",   DBTYPE_I4 );
    PyPackedRow* row = new PyPackedRow( header );
    GetModuleStatusRow(row);
    return row;
}

void InventoryItem::GetModuleStatusRow(PyPackedRow* into) const {
    into->SetField( "instanceID",    new PyLong( m_itemID ) );
    into->SetField( "flagID",        new PyInt( m_flag ) );
    into->SetField( "typeID",        new PyInt( typeID() ) );
    into->SetField( "quantity",      new PyInt( m_singleton ? -1 : quantity()) );
}

PyPackedRow* InventoryItem::GetItemRow() const
{
    PyList *keywords = new PyList();
        keywords->AddItem(new_tuple(new PyString("stacksize"), new PyToken("util.StackSize")));
        keywords->AddItem(new_tuple(new PyString("singleton"), new PyToken("util.Singleton")));

    DBRowDescriptor* header = new DBRowDescriptor(keywords);
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
    GetItemRow( row );

    return row;
}

void InventoryItem::GetItemRow( PyPackedRow* into ) const
{
    into->SetField( "itemID",     new PyLong( m_itemID ) );
    into->SetField( "typeID",     new PyInt( m_type.id() ) );
    into->SetField( "ownerID",    new PyInt( m_ownerID ) );
    into->SetField( "locationID", new PyLong( m_locationID ) );
    into->SetField( "flagID",     new PyInt( m_flag ) );
    int32 qty = (m_singleton ? -1 : quantity());
    if (m_type.categoryID() == EVEDB::invCategories::Blueprint)
        if (m_factory.GetBlueprint(m_itemID)->copy())
            qty = -2;
    into->SetField( "quantity",   new PyInt( qty ) );
    into->SetField( "groupID",    new PyInt( type().groupID() ) );
    into->SetField( "categoryID", new PyInt( type().categoryID() ) );
    into->SetField( "customInfo", new PyString( m_customInfo ) );
}

bool InventoryItem::Populate( Rsp_CommonGetInfo_Entry& result )
{/** @todo  this may need to be reworked once POS and Outposts are implemented. */
    //invItem:
    PySafeDecRef( result.itemID );
    PySafeDecRef( result.invItem );

    if (groupID() == EVEDB::invCategories::Charge) {
        PyTuple* tuple = new PyTuple(3);
            tuple->SetItem(0, new PyInt(m_itemID));
            tuple->SetItem(1, new PyInt(m_flag));
            tuple->SetItem(2, new PyInt(typeID()));
        result.itemID = tuple;
        result.invItem = new PyNone();
    } else {
        result.itemID = new PyInt(m_itemID);
        result.invItem = GetItemRow();
        if (IsOnline()) {
            //there is an effect that goes along with this. We should
            //probably be properly tracking the effect due to some
            // timer things, but for now, were hacking it.
            EntityEffectState es;
                es.env_itemID = m_itemID;
                es.env_charID = m_ownerID;  //may not be quite right...
                es.env_shipID = m_locationID;
                es.env_target = 0;
                es.env_other = new PyNone();
                es.env_area = new PyNone();
                es.env_effectID = effectOnline;
                es.startTime = Win32TimeNow() - Win32Time_Minute; /** @todo fix this once we start tracking effects */
                es.duration = -1;
                es.repeat = 1;
                es.randomSeed = new PyNone();
            result.activeEffects[es.env_effectID] = es.Encode();
        }
    }

    if (categoryID() == EVEDB::invCategories::Skill) {
        result.attributes[AttrSkillTimeConstant] = new PyInt(mAttributeMap.GetAttribute(AttrSkillTimeConstant).get_int());
        result.attributes[AttrSkillPoints] = new PyInt(mAttributeMap.GetAttribute(AttrSkillPoints).get_int());
        result.attributes[AttrSkillLevel] = new PyInt(mAttributeMap.GetAttribute(AttrSkillLevel).get_int());
    } else {
        AttrMapItr itr = mAttributeMap.begin();
        for (; itr != mAttributeMap.end(); itr++) {
            //localization.GetByLabel('UI/Fitting/FittingWindow/WarpSpeed', distText=util.FmtDist(max(1.0, bws) * wsm * 3 * const.AU, 2))
            if ((*itr).first == AttrWarpSpeedMultiplier)
                result.attributes[(*itr).first] = new PyInt(mAttributeMap.GetAttribute(AttrWarpSpeedMultiplier).get_int() /3);
            else
                result.attributes[(*itr).first] = (*itr).second.GetPyObject();
        }
    }

    result.time = Win32TimeNow();
    return true;
}

PyList* InventoryItem::GetItemInfo() const
{
    PyList* itemInfo = new PyList;
        itemInfo->AddItem(GetItemRow());

    return itemInfo;
}

PyObject* InventoryItem::ItemGetInfo()
{
    Rsp_ItemGetInfo result;

    if (!Populate(result.entry))
        return NULL;

    return result.Encode();
}

void InventoryItem::Rename(const char *to)
{
    m_itemName = to;
    SaveItem();
}

void InventoryItem::MoveInto(Inventory &new_home, EVEItemFlags _flag, bool notify) {
    Move( new_home.m_inventoryID, _flag, notify );
}

void InventoryItem::Move(uint32 new_location, EVEItemFlags new_flag, bool notify) {
    uint32 old_location = m_locationID;
    EVEItemFlags old_flag = m_flag;

    if ((new_location == old_location) and (new_flag == old_flag))
        return; //nothing to do...

    //first, take myself out of my old inventory, if its loaded.
    if (!IsTradeCont(old_location)) {   // fix for trade containers segfault (as they dont have an inventory* object)
        Inventory *old_inventory = m_factory.GetInventoryFromId( old_location, false );
        if (old_inventory) {
            old_inventory->RemoveItem(InventoryItemRef(this));  //releases its ref
        } else {
            if (m_locationID)
                _log(INV__WARNING, "Inventory for %u not found. %s not removed from it's container's inventory.", itemName().c_str(), old_location);
        }
    }

    m_locationID = new_location;
    m_flag = new_flag;

    //then make sure that my new inventory is updated, if its loaded.
    if (!IsTradeCont(new_location)) {
        Inventory *new_inventory = m_factory.GetInventoryFromId( new_location, false );
        if (new_inventory) {
            new_inventory->AddItem(InventoryItemRef(this)); //makes a new ref
        } else {
            if (m_locationID)
                _log(INV__WARNING, "Inventory for %u not found. %s not added to it's container's inventory.", itemName().c_str(), new_location);
        }
    }

    SaveItem();
    if (IsNPCCorp(m_ownerID))
        return;

    //notify about the changes.
    if (notify) {
        std::map<int32, PyRep *> changes;
        if ( new_location != old_location )
            changes[ixLocationID] = new PyInt(old_location);
        if ( new_flag != old_flag )
            changes[ixFlag] = new PyInt(old_flag);
        SendItemChange( m_ownerID, changes );   //changes is consumed
    }
}

bool InventoryItem::AlterQuantity(int32 qty_change, bool notify) {
    if (qty_change == 0)
        return true;

    int32 new_qty = m_quantity + qty_change;

    if (new_qty < 0) {
        codelog(ITEM__ERROR, "%s (%u): Tried to remove %i quantity from stack of %i", m_itemName.c_str(), m_itemID, -qty_change, m_quantity);
        return false;
    }

    return SetQuantity(new_qty, notify);
}

bool InventoryItem::SetQuantity(int32 qty_new, bool notify) {
    //if an object is singleton, it shouldn't be able to add/remove qty
    if (m_singleton) {
        _log(ITEM__ERROR, "%s (%u): Failed to set quantity %i, the items singleton bit is set", m_itemName.c_str(), m_itemID, qty_new);
        return false;
    }

    int32 old_qty = m_quantity;

    m_quantity = qty_new;

    SaveItem();

    //notify about the changes.
    if (notify) {
        std::map<int32, PyRep *> changes;
        //send the notify to the new owner.
        // this informs client of a stack change
        changes[/*ixQuantity*/ixStackSize] = new PyInt(old_qty);
        SendItemChange(m_ownerID, changes); //changes is consumed
    }

    return true;
}
bool InventoryItem::SetFlag(EVEItemFlags new_flag, bool notify) {
    EVEItemFlags old_flag = m_flag;
    m_flag = new_flag;

    SaveItem();

    if (notify) {
        std::map<int32, PyRep *> changes;
        //send the notify to the new owner.
        changes[ixFlag] = new PyInt(old_flag);
        SendItemChange(m_ownerID, changes); //changes is consumed
    }
    return true;
}

InventoryItemRef InventoryItem::Split(int32 qty_to_take, bool notify) {
    if (qty_to_take <= 0) {
        _log(ITEM__ERROR, "%s (%u): Asked to split into a chunk of %d", itemName().c_str(), itemID(), qty_to_take);
        return InventoryItemRef();
    }
    if (!AlterQuantity(-qty_to_take, notify)) {
        _log(ITEM__ERROR, "%s (%u): Failed to remove quantity %d during split.", itemName().c_str(), itemID(), qty_to_take);
        return InventoryItemRef();
    }

    ItemData idata(
        m_type.id(),
        m_ownerID,
        0,
        m_flag,
        qty_to_take
    );

    InventoryItemRef res = m_factory.SpawnItem(idata);
    if (notify)
        res->Move( m_locationID, m_flag );

    return res;
}

bool InventoryItem::Merge(InventoryItemRef to_merge, uint32 qty/*0*/, bool notify/*true*/) {
    /*[00m14:51:55 [Error] Metal Scraps (140000072) in location 60014809, flag 4: Asked to merge with item 140000087 in location 60014809, flag 4.
     * 14:51:55 [Error] EMP S (140000108) in location 60014809, flag 4: Asked to merge with item 140000109 in location 60014809, flag 4.
     * 14:51:55 [Error] EMP S (140000108) in location 60014809, flag 4: Asked to merge with item 140000129 in location 60014809, flag 4.
     * 14:51:55 [Error] Tritanium (140000067) in location 60014809, flag 4: Asked to merge with item 140000131 in location 60014809, flag 4.
     */
    /*
    if (locationID() != to_merge->locationID()) {
        if (! (flag() == flagHangar) and ( (to_merge->flag() >= flagHiSlot0) or (to_merge->flag() <= flagHiSlot7) )) {
            _log(ITEM__ERROR, "%s (%u) in location %u asked to merge with item %u in location %u.", itemName().c_str(), itemID(), locationID(), to_merge->itemID(), to_merge->locationID());
            // remove item from SystemManager
            return false;
        }
    }
    if (locationID() == to_merge->locationID()) {
        if ((!( (flag() == flagCargoHold) and ( (to_merge->flag() >= flagHiSlot0) or (to_merge->flag() <= flagHiSlot7) ))) or \
            (!( (flag() == flagHangar) and (to_merge->flag() == flagHangar) ))) {
            _log(ITEM__ERROR, "%s (%u) in location %u, flag %u: Asked to merge with item %u in location %u, flag %u.", itemName().c_str(), itemID(), locationID(), flag(), to_merge->itemID(), to_merge->locationID(), to_merge->flag());
            return false;
        }
    }
    */
    if (singleton() or to_merge->singleton()) {
        throw PyException( MakeCustomError("You cannot stack unpackaged items."));
    }
    if (typeID() != to_merge->typeID()) {
        _log(ITEM__ERROR, "%s (%u): Asked to merge with %s (%u).", itemName().c_str(), itemID(), to_merge->itemName().c_str(), to_merge->itemID());
        return false;
    }

    if (qty < 0) {
        _log(ITEM__ERROR, "%s (%u): Asked to merge with %d units of item %u.", itemName().c_str(), itemID(), qty, to_merge->itemID());
        return false;
    } else if (qty == 0) {
        qty = to_merge->quantity();
    }

    if (qty == to_merge->quantity()) {
        to_merge->Delete();
    } else if (!to_merge->AlterQuantity(-qty, notify)) {
        _log(ITEM__ERROR, "%s (%u): Failed to remove quantity %d.", to_merge->itemName().c_str(), to_merge->itemID(), qty);
        return false;
    }

    if (!AlterQuantity(qty, notify)) {
        _log(ITEM__ERROR, "%s (%u): Failed to add quantity %d.", itemName().c_str(), itemID(), qty);
        return false;
    }

    return true;
}

bool InventoryItem::ChangeSingleton(bool new_singleton, bool notify) {
    bool old_singleton = m_singleton;

    if (new_singleton == old_singleton)
        return true;    //nothing to do...

    m_singleton = new_singleton;

    SetAttribute(AttrVolume, GetPackagedVolume(), false);

    SaveItem();

    //notify about the changes.
    if (notify) {
        std::map<int32, PyRep *> changes;
        changes[ixSingleton] = new PyInt(old_singleton);
        SendItemChange(m_ownerID, changes); //changes is consumed
    }

    return true;
}

void InventoryItem::ChangeOwner(uint32 new_owner, bool notify) {
    uint32 old_owner = m_ownerID;

    if (new_owner == old_owner)
        return; //nothing to do...

    m_ownerID = new_owner;

    SaveItem();

    //notify about the changes.
    if (notify) {
        std::map<int32, PyRep *> changes;

        //send the notify to the new owner.
        changes[ixOwnerID] = new PyInt(old_owner);
        SendItemChange(new_owner, changes); //changes is consumed

        //also send the notify to the old owner.
        changes[ixOwnerID] = new PyInt(old_owner);
        SendItemChange(old_owner, changes); //changes is consumed
    }
}

void InventoryItem::SaveItem() {
    m_factory.db().SaveItem(
        m_itemID,
        ItemData(
            itemName().c_str(),
            typeID(),
            m_ownerID,
            m_locationID,
            m_flag,
            m_contraband,
            m_singleton,
            m_quantity,
            m_position,
            customInfo().c_str()
        )
    );
    // item attributes are saved in ItemFactory.cpp:96  (save loop on shutdown for loaded items)
    // make call here for items saved after *some* change
    mAttributeMap.SaveAttributes();
}

//contents of changes are consumed and cleared
void InventoryItem::SendItemChange(uint32 toID, std::map<int32, PyRep *> &changes) const {
    if (IsNPCCorp(toID))
        return;
    //TODO: figure out the appropriate list of interested people...
    Client *c = sEntityList.FindClientByCharID(toID);
    if (!c) return; //not found or not online...

    NotifyOnItemChange change;
        change.itemRow = GetItemRow();
        change.changes = changes;
    changes.clear();    //reset change map for next update.
    PyTuple *tmp = change.Encode();  //this is consumed below
    // not sure what the difference is on the types below...
    c->SendNotification("OnItemChange", "clientID", &tmp, false); //unsequenced.
    //c->SendNotification("OnItemsChanged", "charid", &tmp, false); //unsequenced.
}

void InventoryItem::SetOnline(bool online, bool isRig/*false*/) {
    /** @note  this is only used by modules
     ** check for pos structures also!! **
     */
    _log(SHIP__MODULE_DEBUG, "InventoryItem::SetOnline() - set module %s(%u) to %s", \
                    m_itemName.c_str(), m_itemID, (online ? "Online" : "Offline"));
    if (!isRig)   // rigs DO NOT get isOnline attrib set.
        SetAttribute(AttrIsOnline, int(online));

    Client* pClient = sEntityList.FindClientByCharID(m_ownerID);
    if (!pClient) {
        _log(SHIP__MODULE_ERROR, "InventoryItem::SetOnline() - No client object found using m_ownerID (%u) for module %s(%u)", \
                            m_ownerID, m_itemName.c_str(), m_itemID );
        return;
    }

    GodmaEnvironment ge;
        ge.selfID = m_itemID;
        ge.charID = m_ownerID;
        ge.shipID = pClient->GetShipID();
        ge.targetID = 0;
        ge.other = new PyNone();
        ge.area = new PyList;
        ge.effectID = effectOnline;
    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = online;
        shipEff.active = online;
        shipEff.environment = ge.Encode();
        shipEff.startTime = shipEff.timeNow;
    if (mAttributeMap.HasAttribute(AttrDuration)) {
        shipEff.duration = (online ? mAttributeMap.GetAttribute(AttrDuration).get_float() : 0.0);
    } else if (mAttributeMap.HasAttribute(AttrSpeed)) {
        shipEff.duration = (online ? mAttributeMap.GetAttribute(AttrSpeed).get_float() : 0.0);
    } else {
        shipEff.duration = 0.0;
    }
        shipEff.repeat = (online ? 1000 : 0);
        shipEff.error = new PyNone();
    PyList* events = new PyList;
        events->AddItem(shipEff.Encode());
    Notify_OnMultiEvent multi;
        multi.events = events;
    PyTuple* tmp = multi.Encode();
    pClient->SendNotification("OnMultiEvent", "clientID", &tmp);
}

void InventoryItem::SetCustomInfo(const char *ci) {
    if (ci)
        m_customInfo = ci;
    else
        m_customInfo = "";
    SaveItem();
}

void InventoryItem::Relocate(const GPoint &pos)
{
    m_position = pos;
    _log(ITEM__TRACE, "%s(%u) Relocating to %.3f, %.3f, %.3f.", m_itemName.c_str(), m_itemID, m_position.x, m_position.y, m_position.z);
}

void InventoryItem::SetAttribute( uint16 attrID, int64 num, bool notify/*true*/)
{
    EvilNumber devil_number(num);
    mAttributeMap.SetAttribute(attrID, devil_number, notify);
}

void InventoryItem::SetAttribute( uint16 attrID, double num, bool notify/*true*/)
{
    EvilNumber devil_number(num);
    mAttributeMap.SetAttribute(attrID, devil_number, notify);
}

void InventoryItem::SetAttribute( uint16 attrID, EvilNumber num, bool notify/*true*/)
{
    mAttributeMap.SetAttribute(attrID, num, notify);
}

void InventoryItem::SetAttribute( uint16 attrID, int num, bool notify/*true*/)
{
    EvilNumber devil_number(num);
    mAttributeMap.SetAttribute(attrID, devil_number, notify);
}

void InventoryItem::SetAttribute( uint16 attrID, uint64 num, bool notify/*true*/)
{
    EvilNumber devil_number(num);
    mAttributeMap.SetAttribute(attrID, devil_number, notify);
}

void InventoryItem::SetAttribute( uint16 attrID, uint32 num, bool notify/*true*/)
{
    EvilNumber devil_number(num);
    mAttributeMap.SetAttribute(attrID, devil_number, notify);
}

// new effects system  -allan 4Feb17
bool InventoryItem::SkillCheck(InventoryItemRef refItem)
{
    EvilNumber need = 0, has = 0;
    uint16 attr = 182, skill = 277;
    for (int8 i = 0; i < 3; i++, attr++, skill++) {
        if ((refItem->HasAttribute(attr, need)) and (mAttributeMap.HasAttribute(skill, has))) {
            if (need > has)
                return false;
        }
    }
    if ((refItem->HasAttribute(1285, need)) and (mAttributeMap.HasAttribute(1286, has))) {
        if (need > has)
            return false;
    }

    attr = 1289; skill = 1287;
    for (int8 i = 0; i < 2; i++, attr++, skill++) {
        if ((refItem->HasAttribute(attr, need)) and (mAttributeMap.HasAttribute(skill, has))) {
            if (need > has)
                return false;
        }
    }
    // all skill requirement checks passed.
    return true;
}

void InventoryItem::AddModifier(fxData data)
{
    m_modifiers.emplace(std::pair<uint8, fxData>(data.math, data));
}

void InventoryItem::ReloadAttributes()
{
    mAttributeMap.Load(true);
    m_modifiers.clear();
}

// not sure if/how i can use these...maybe for Implants, boosters, modules(state>1)
void InventoryItem::ApplyEffect(int8 state)
{
}

void InventoryItem::RemoveEffect(int8 state)
{
}

void InventoryItem::GetEffectsInState(int8 state, std::vector< Effect >& effectRef)
{
}
