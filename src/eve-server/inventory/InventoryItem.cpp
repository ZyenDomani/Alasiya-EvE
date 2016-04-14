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
#include "character/Skill.h"
#include "inventory/Owner.h"
#include "manufacturing/Blueprint.h"
#include "pos/Structure.h"
#include "ship/Ship.h"
#include "station/Station.h"
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
  //attributes(_factory, *this, true, true),
  mAttributeMap(*this),
  mDefaultAttributeMap(*this,true),
  m_saveTimer(0,true),
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

    _log(ITEM__TRACE, "Created object %p for item %s (%u).", this, itemName().c_str(), itemID());
}

InventoryItem::~InventoryItem()
{
    // Save this item's entity_attributes info to the Database before it is destroyed
    //mAttributeMap.SaveAttributes();

    // Save this item's entity table info to the Database before it is destroyed
    //SaveItem();
}

InventoryItemRef InventoryItem::Load(ItemFactory &factory, uint32 itemID)
{
    return InventoryItem::Load<InventoryItem>( factory, itemID );
}

InventoryItemRef InventoryItem::LoadEntity(ItemFactory &factory, uint32 itemID, const ItemData &data)
{
    const ItemType *type = factory.GetType( data.typeID );

	InventoryItemRef itemRef = InventoryItemRef( new InventoryItem(factory, itemID, *type, data) );

	itemRef->_Load();

	return itemRef;
}

template<class _Ty>
RefPtr<_Ty> InventoryItem::_LoadItem(ItemFactory &factory, uint32 itemID,
    // InventoryItem stuff:
    const ItemType &type, const ItemData &data)
{
    // See what to do next:
    switch( type.categoryID() ) {
        /* not handled yet...
        case EVEDB::invCategories::_System:
        case EVEDB::invCategories::Material:
        case EVEDB::invCategories::Accessories:
        case EVEDB::invCategories::Module:
        case EVEDB::invCategories::Charge:
        case EVEDB::invCategories::Trading:
        case EVEDB::invCategories::Bonus:
        case EVEDB::invCategories::Commodity:
        case EVEDB::invCategories::Implant:
        case EVEDB::invCategories::Deployable:
        case EVEDB::invCategories::Reaction:
            */
            /** @todo (Allan) these need work ...for now, load default item
             *        case EVEDB::invCategories::Asteroid:
             *            //return AsteroidItem::_LoadItem<AsteroidItem>( factory, itemID, type, data );
             *        case EVEDB::invCategories::Drone:
             *            //return DroneItem::_LoadItem<DroneItem>( factory, itemID, type, data );
             */
        case EVEDB::invCategories::Blueprint: {
            return Blueprint::_LoadItem<Blueprint>( factory, itemID, type, data );
        }
        case EVEDB::invCategories::Skill: {
            return Skill::_LoadItem<Skill>( factory, itemID, type, data );
        }
        case EVEDB::invCategories::Station: {
            return Station::_LoadItem<Station>( factory, itemID, type, data );
        }
        case EVEDB::invCategories::Ship: {
            return Ship::_LoadItem<Ship>( factory, itemID, type, data );
        }
        case EVEDB::invCategories::Owner: {
            return Owner::_LoadItem<Owner>( factory, itemID, type, data );
        }
        case EVEDB::invCategories::Celestial: {
            if (type.groupID() == EVEDB::invGroups::Wreck)
                return WreckContainerRef( new WreckContainer( factory, itemID, type, data) );
            else if ( (type.groupID() == EVEDB::invGroups::Secure_Cargo_Container)
                || (type.groupID() == EVEDB::invGroups::Audit_Log_Secure_Container)
                || (type.groupID() == EVEDB::invGroups::Freight_Container)
                || (type.groupID() == EVEDB::invGroups::Cargo_Container)
                // || (type.groupID() == EVEDB::invGroups::Wreck)
            )
                return CargoContainerRef( new CargoContainer( factory, itemID, type, data ) );
            else
                return CelestialObjectRef( new CelestialObject( factory, itemID, type, data ) );
        }
        case EVEDB::invCategories::Entity: {
            if (type.groupID() == EVEDB::invGroups::Spawn_Container)
                return CargoContainerRef( new CargoContainer( factory, itemID, type, data ) );
            else
                //  added checks for all npc's   -allan 26Dec14
                if (   (type.groupID() == EVEDB::invGroups::Sentry_Gun)
                ||  (type.groupID() == EVEDB::invGroups::Protective_Sentry_Gun)
                ||  (type.groupID() == EVEDB::invGroups::Police_Drone)
                ||  (type.groupID() == EVEDB::invGroups::Pirate_Drone)
                ||  (type.groupID() == EVEDB::invGroups::LCO_Drone)
                ||  (type.groupID() == EVEDB::invGroups::Tutorial_Drone)
                ||  (type.groupID() == EVEDB::invGroups::Rogue_Drone)
                ||  (type.groupID() == EVEDB::invGroups::Faction_Drone)
                ||  (type.groupID() == EVEDB::invGroups::Convoy)
                ||  (type.groupID() == EVEDB::invGroups::Convoy_Drone)
                ||  (type.groupID() == EVEDB::invGroups::Concord_Drone)
                ||  (type.groupID() == EVEDB::invGroups::Mission_Drone)
                ||  (type.groupID() == EVEDB::invGroups::Destructible_Sentry_Gun)
                ||  (type.groupID() == EVEDB::invGroups::Deadspace_Overseer)
                ||  (type.groupID() == EVEDB::invGroups::Customs_Official)
                ||  (type.groupID() == EVEDB::invGroups::Deadspace_Overseer_s_Structure)
                ||  (type.groupID() == EVEDB::invGroups::Deadspace_Overseer_s_Sentry)
                ||  (type.groupID() == EVEDB::invGroups::Deadspace_Overseer_s_Belongings)
                ||  (type.groupID() == EVEDB::invGroups::Storyline_Frigate)
                ||  (type.groupID() == EVEDB::invGroups::Storyline_Cruiser)
                ||  (type.groupID() == EVEDB::invGroups::Storyline_Battleship)
                ||  (type.groupID() == EVEDB::invGroups::Storyline_Mission_Frigate)
                ||  (type.groupID() == EVEDB::invGroups::Storyline_Mission_Cruiser)
                ||  (type.groupID() == EVEDB::invGroups::Storyline_Mission_Battleship)
                ||  ((type.groupID() >= EVEDB::invGroups::Asteroid_Angel_Cartel_Frigate) && (type.groupID() <= EVEDB::invGroups::Asteroid_Serpentis_BattleCruiser))
                ||  ((type.groupID() >= EVEDB::invGroups::Deadspace_Angel_Cartel_BattleCruiser) && (type.groupID() <= EVEDB::invGroups::Deadspace_Angel_Cartel_Frigate))
                ||  ((type.groupID() >= EVEDB::invGroups::Deadspace_Blood_Raiders_BattleCruiser) && (type.groupID() <= EVEDB::invGroups::Deadspace_Blood_Raiders_Frigate))
                ||  ((type.groupID() >= EVEDB::invGroups::Deadspace_Guristas_BattleCruiser) && (type.groupID() <= EVEDB::invGroups::Deadspace_Guristas_Frigate))
                ||  ((type.groupID() >= EVEDB::invGroups::Deadspace_Sanshas_Nation_BattleCruiser) && (type.groupID() <= EVEDB::invGroups::Deadspace_Sanshas_Nation_Frigate))
                ||  ((type.groupID() >= EVEDB::invGroups::Deadspace_Serpentis_BattleCruiser) && (type.groupID() <= EVEDB::invGroups::Deadspace_Serpentis_Frigate))
                ||  ((type.groupID() >= EVEDB::invGroups::Mission_Amarr_Empire_Frigate) && (type.groupID() <= EVEDB::invGroups::Mission_Minmatar_Republic_Battleship))
                ||  (type.groupID() == EVEDB::invGroups::Destructible_Agents_In_Space)
                ||  ((type.groupID() >= EVEDB::invGroups::Asteroid_Rogue_Drone_Battlecruiser) && (type.groupID() <= EVEDB::invGroups::Asteroid_Rogue_Drone_Swarm))
                ||  (type.groupID() == EVEDB::invGroups::Large_Collidable_Ship)
                ||  ((type.groupID() >= EVEDB::invGroups::Asteroid_Angel_Cartel_Commander_Frigate) && (type.groupID() <= EVEDB::invGroups::Asteroid_Serpentis_Commander_Frigate))
                ||  ((type.groupID() >= EVEDB::invGroups::Mission_Generic_Battleships) && (type.groupID() <= EVEDB::invGroups::Mission_Generic_Destroyers))
                ||  ((type.groupID() >= EVEDB::invGroups::Asteroid_Rogue_Drone_Commander_Battlecruiser) && (type.groupID() <= EVEDB::invGroups::Asteroid_Serpentis_Commander_Battleship))
                ||  (type.groupID() == EVEDB::invGroups::Mission_Fighter_Drone)
                ||  ((type.groupID() >= EVEDB::invGroups::Mission_Amarr_Empire_Carrier) && (type.groupID() <= EVEDB::invGroups::Mission_Minmatar_Republic_Carrier))
                ||  (type.groupID() == EVEDB::invGroups::Mission_Faction_Transports)
                ||  (type.groupID() == EVEDB::invGroups::Mission_Faction_Industrials)
                ||  (type.groupID() == EVEDB::invGroups::Deadspace_Sleeper_Sleepless_Sentinel)
                ||  (type.groupID() == EVEDB::invGroups::Deadspace_Sleeper_Awakened_Sentinel)
                ||  (type.groupID() == EVEDB::invGroups::Deadspace_Sleeper_Emergent_Sentinel)
                ||  ((type.groupID() >= EVEDB::invGroups::Deadspace_Sleeper_Sleepless_Defender) && (type.groupID() <= EVEDB::invGroups::Deadspace_Sleeper_Emergent_Patroller))
                ||  (type.groupID() == EVEDB::invGroups::Mission_Faction_Cruiser)
                ||  (type.groupID() == EVEDB::invGroups::Mission_Faction_Frigate)
                ||  (type.groupID() == EVEDB::invGroups::Incursion_Sanshas_Nation_Industrial)
                ||  (type.groupID() == EVEDB::invGroups::Incursion_Sanshas_Nation_Capital)
                ||  (type.groupID() == EVEDB::invGroups::Incursion_Sanshas_Nation_Frigate)
                ||  (type.groupID() == EVEDB::invGroups::Incursion_Sanshas_Nation_Cruiser)
                ||  (type.groupID() == EVEDB::invGroups::Incursion_Sanshas_Nation_Battleship))
					return InventoryItemRef( new InventoryItem(factory, itemID, type, data) );
				else
					return CelestialObjectRef( new CelestialObject( factory, itemID, type, data ) );
        }
        default: {
            _log(ITEM__MESSAGE, "item %u (type %u, cat %u) tried _LoadItem, but is not handled.", itemID, type.id(), type.categoryID());
        }
    }

    // Generic item, create one:
    return InventoryItemRef( new InventoryItem( factory, itemID, type, data ) );
}

bool InventoryItem::_Load()
{
    // load attributes
    mDefaultAttributeMap.Load();
    mAttributeMap.Load();

	// fill basic cargo hold data:
	m_cargoHoldsUsedVolumeByFlag.insert(std::pair<EVEItemFlags,double>(flagCargoHold,mAttributeMap.GetAttribute(AttrCapacity).get_float()));

    // update inventory
    Inventory *inventory = m_factory.GetInventory( locationID(), false );
    if ( inventory != NULL )
        inventory->AddItem( InventoryItemRef( this ) );

    return true;
}

InventoryItemRef InventoryItem::Spawn(ItemFactory &factory, ItemData &data)
{
    // obtain type of new item
    const ItemType *t = factory.GetType( data.typeID );
    if (!t) return InventoryItemRef();
    // See what to do next:
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
        case EVEDB::invCategories::Entity: {
			// Spawn generic item for Entities at this time:
			// (commented lines for _SpawnEntity and LoadEntity can be used alternatively to prevent Entities from being created and saved to the DB,
			//  however, this may be causing weird and bad targetting of NPC ships when they enter the bubble and your ship is already in it)
			//uint32 itemID = InventoryItem::_SpawnEntity( factory, data );		// Use this to prevent entity from being stored in DB
            uint32 itemID = InventoryItem::_Spawn( factory, data );
            if (!itemID) return InventoryItemRef();
			//InventoryItemRef itemRef = InventoryItem::LoadEntity( factory, itemID, data );		// Use this to prevent entity from being stored in DB
            InventoryItemRef itemRef = InventoryItem::Load( factory, itemID );
            if (!itemRef) return InventoryItemRef();
			return itemRef;
		}
        case EVEDB::invCategories::Blueprint: {
            BlueprintData bdata; // use default blueprint attributes
            BlueprintRef blueRef = Blueprint::Spawn( factory, data, bdata );
            blueRef->SaveAttributes();
            return blueRef;
        }
        case EVEDB::invCategories::Celestial: {
            if ( (t->groupID() == EVEDB::invGroups::Secure_Cargo_Container)
                || (t->groupID() == EVEDB::invGroups::Cargo_Container)
                || (t->groupID() == EVEDB::invGroups::Freight_Container)
                || (t->groupID() == EVEDB::invGroups::Audit_Log_Secure_Container)
                || (t->groupID() == EVEDB::invGroups::Spawn_Container)
                || (t->groupID() == EVEDB::invGroups::Wreck) )  //FIXME  wrecks should now use new Wreck:DynamicSystemEntitry class
            {
                // Spawn new Cargo Container
                uint32 itemID = CargoContainer::_Spawn( factory, data );
                if (!itemID) return InventoryItemRef();
                CargoContainerRef cargoRef = CargoContainer::Load( factory, itemID );
                if (!cargoRef) return InventoryItemRef();
                // THESE SHOULD BE MOVED INTO A CargoContainer::Spawn() function that does not exist yet
                // Create default dynamic attributes in the AttributeMap:
                cargoRef->SetAttribute(AttrIsOnline,      1);                                                 // Is Online
                cargoRef->SetAttribute(AttrDamage,        0.0);                                               // Structure Damage
                cargoRef->SetAttribute(AttrShieldCharge,  cargoRef->GetAttribute(AttrShieldCapacity));  // Shield Charge
                cargoRef->SetAttribute(AttrArmorDamage,   0.0);                                               // Armor Damage
                cargoRef->SetAttribute(AttrMass,          cargoRef->type().mass());          // Mass
                cargoRef->SetAttribute(AttrRadius,        cargoRef->type().radius());        // Radius
                cargoRef->SetAttribute(AttrVolume,        cargoRef->type().volume());        // Volume
                cargoRef->SetAttribute(AttrCapacity,      cargoRef->type().capacity());      // Capacity
                cargoRef->SaveAttributes();
                return cargoRef;
            }
            /*  put a check in here for beacons,
			 * groupid 310::typeid
			 *   Large_Collidable_Structure::
			 * groupid 226::typeid 10124
			 *   Large_Collidable_Object::
			 * groupID 319   Beacon   typeid 29189
			 * groupID 920    Effect_Beacon
			 *
			 *  then use SetAttribute(AttrIsGlobal, 1)    (1207)
			 */
            else {
                // Spawn new Celestial Object
                uint32 itemID = CelestialObject::_Spawn( factory, data );
                if (!itemID) return InventoryItemRef();
                CelestialObjectRef celestialRef = CelestialObject::Load( factory, itemID );
                if (!celestialRef) return InventoryItemRef();
                celestialRef->SaveAttributes();
                return celestialRef;
            }
        }
        case EVEDB::invCategories::Ship: {
            return Ship::Spawn( factory, data );
        }
        case EVEDB::invCategories::Skill: {
            return Skill::Spawn( factory, data );
        }
        case EVEDB::invCategories::Owner: {
            return Owner::Spawn( factory, data );
        }
        case EVEDB::invCategories::Charge: {
            switch (data.flag) {
                case EVEItemFlags::flagMissile: {
                    // Spawn launched missile item in EVEMU_MISSILE_ID range and does NOT save missile to db
                    uint32 itemID = InventoryItem::_SpawnEntity( factory, data );
                    if (!itemID) return InventoryItemRef();
                    InventoryItemRef itemRef = InventoryItem::LoadEntity( factory, itemID, data );
                    // THESE SHOULD BE MOVED INTO A Charge::Spawn() function that does not exist yet
                    // Create default dynamic attributes in the AttributeMap:
                    //itemRef->SetAttribute(AttrIsOnline,   1);                             // Is Online
                    //itemRef->SetAttribute(AttrDamage,     0.0);                           // Structure Damage
                    itemRef->SetAttribute(AttrMass,       itemRef->type().mass());           // Mass
                    itemRef->SetAttribute(AttrRadius,     itemRef->type().radius());       // Radius
                    itemRef->SetAttribute(AttrVolume,     itemRef->type().volume());       // Volume
                    itemRef->SetAttribute(AttrCapacity,   itemRef->type().capacity());   // Capacity
                    itemRef->SaveAttributes();
                    return itemRef;
                }
                default: {
                    // Spawn generic item:
                    uint32 itemID = InventoryItem::_Spawn( factory, data );
                    if (!itemID) return InventoryItemRef();
                    InventoryItemRef itemRef = InventoryItem::Load( factory, itemID );
                    if (!itemRef) return InventoryItemRef();
                    // THESE SHOULD BE MOVED INTO A Charge::Spawn() function that does not exist yet
                    // Create default dynamic attributes in the AttributeMap:
                    //itemRef->SetAttribute(AttrIsOnline,   1);                             // Is Online
                    //itemRef->SetAttribute(AttrDamage,     0.0);                           // Structure Damage
                    itemRef->SetAttribute(AttrMass,       itemRef->type().mass());           // Mass
                    itemRef->SetAttribute(AttrRadius,     itemRef->type().radius());       // Radius
                    itemRef->SetAttribute(AttrVolume,     itemRef->type().volume());       // Volume
                    itemRef->SetAttribute(AttrCapacity,   itemRef->type().capacity());   // Capacity
                    itemRef->SaveAttributes();
                    return itemRef;
                }
            }
		}
        case EVEDB::invCategories::Module:
        case EVEDB::invCategories::Drone:
        case EVEDB::invCategories::Deployable: {
            // Spawn generic item:
            uint32 itemID = InventoryItem::_Spawn( factory, data );
            if (!itemID) return InventoryItemRef();
            InventoryItemRef itemRef = InventoryItem::Load( factory, itemID );
            if (!itemRef) return InventoryItemRef();
            // THESE SHOULD BE MOVED INTO A _Item::Spawn() function that does not exist yet
            // Create default dynamic attributes in the AttributeMap:
            itemRef->SetAttribute(AttrIsOnline,   1);                                             // Is Online
            itemRef->SetAttribute(AttrDamage,     0.0);                                             // Structure Damage
            itemRef->SetAttribute(AttrMass,           itemRef->type().mass());           // Mass
            itemRef->SetAttribute(AttrRadius,         itemRef->type().radius());       // Radius
            itemRef->SetAttribute(AttrVolume,         itemRef->type().volume());       // Volume
            itemRef->SetAttribute(AttrCapacity,       itemRef->type().capacity());   // Capacity
            itemRef->SaveAttributes();
            return itemRef;
        }
        case EVEDB::invCategories::Asteroid: {
            //TODO  run checks here for asteroids (id in 70m range), and outposts (id in 61m range) and add to correct table, and retrieve correct id range.
            //  see ~/Desktop/cruc/misc_shit/classes_by_itemID      -done, but see note below.
            // Spawn generic item:
			// (commented lines for _SpawnEntity and LoadEntity can be used alternatively to prevent asteroids from being created and saved to the DB,
			//  however, initial testing of this throws a client exception when attempting to show brackets for these asteroid space objects when using
            //  these alternative functions.  more investigation into that is required before they can be used with Asteroids)
            //NOTE  roidID range fixed, but ore is spawned as invCategories::Asteroid, also.  must be saved in db, so continue with this.  -allan 31May15
            uint32 itemID = InventoryItem::_Spawn( factory, data );
            //uint32 itemID = InventoryItem::_SpawnEntity( factory, data ); // Use this to prevent Asteroids from being stored in DB
            if (!itemID) return InventoryItemRef();
            InventoryItemRef itemRef = InventoryItem::Load( factory, itemID );
            if (!itemRef) return InventoryItemRef();
            //InventoryItemRef itemRef = InventoryItem::LoadEntity( factory, itemID, data );		// Use this to prevent Asteroids from being stored in DB
            // THESE SHOULD BE MOVED INTO AN Asteroid::Spawn() function that does not exist yet
            // Create default dynamic attributes in the AttributeMap:
            //itemRef->SetAttribute(AttrMass,           itemRef->type().mass());           // Mass
            itemRef->SetAttribute(AttrRadius,         itemRef->type().radius());       // Radius
            itemRef->SetAttribute(AttrVolume,         itemRef->type().volume());       // Volume
            itemRef->SaveAttributes();
            return itemRef;
        }
        case EVEDB::invCategories::Structure: {
            // Spawn generic item:
            uint32 itemID = Structure::_Spawn( factory, data );
            if (!itemID) return InventoryItemRef();
            StructureRef itemRef = Structure::Load( factory, itemID );
            if (!itemRef) return InventoryItemRef();
            // THESE SHOULD BE MOVED INTO A Structure::Spawn() function that does not exist yet
            // Create default dynamic attributes in the AttributeMap:
            itemRef->SetAttribute(AttrIsOnline,       1);                                             // Is Online
            itemRef->SetAttribute(AttrDamage,         0.0);                                             // Structure Damage
            itemRef->SetAttribute(AttrShieldCharge,   itemRef->GetAttribute(AttrShieldCapacity));       // Shield Charge
            itemRef->SetAttribute(AttrArmorDamage,    0.0);                                       // Armor Damage
            itemRef->SetAttribute(AttrMass,           itemRef->type().mass());           // Mass
            itemRef->SetAttribute(AttrRadius,         itemRef->type().radius());       // Radius
            itemRef->SetAttribute(AttrVolume,         itemRef->type().volume());       // Volume
            itemRef->SetAttribute(AttrCapacity,       itemRef->type().capacity());   // Capacity
            itemRef->SaveAttributes();
            return itemRef;
        }
        case EVEDB::invCategories::Station: {
            uint32 itemID = Station::_Spawn( factory, data );
            if (!itemID) return InventoryItemRef();
            StationRef stationRef = Station::Load( factory, itemID );
            if (!stationRef) return InventoryItemRef();
            // THESE SHOULD BE MOVED INTO A Station::Spawn() function that does not exist yet
            // Create default dynamic attributes in the AttributeMap:
            stationRef->SetAttribute(AttrIsOnline,      1);                                              // Is Online
            stationRef->SetAttribute(AttrDamage,        0.0);                                              // Structure Damage
            stationRef->SetAttribute(AttrShieldCharge,  stationRef->GetAttribute(AttrShieldCapacity));     // Shield Charge
            stationRef->SetAttribute(AttrArmorDamage,   0.0);                                         // Armor Damage
            stationRef->SetAttribute(AttrMass,           stationRef->type().mass());           // Mass
            stationRef->SetAttribute(AttrRadius,         stationRef->type().radius());       // Radius
            stationRef->SetAttribute(AttrVolume,         stationRef->type().volume());       // Volume
            stationRef->SetAttribute(AttrCapacity,       stationRef->type().capacity());   // Capacity
            stationRef->SaveAttributes();
            return stationRef;
        }
    }
    // Spawn generic item:
    uint32 itemID = InventoryItem::_Spawn( factory, data );
    if (!itemID) return InventoryItemRef();
    InventoryItemRef itemRef = InventoryItem::Load( factory, itemID );
    if (!itemRef) return InventoryItemRef();
	// Create some basic attributes that are NOT found in dgmTypeAttributes for most items, yet most items DO need:
    itemRef->SetAttribute(AttrIsOnline,    1);                                              // Is Online
    itemRef->SetAttribute(AttrDamage,      0.0);                                              // Structure Damage
    itemRef->SetAttribute(AttrMass,           itemRef->type().mass());           // Mass
    itemRef->SetAttribute(AttrRadius,         itemRef->type().radius());       // Radius
    itemRef->SetAttribute(AttrVolume,         itemRef->type().volume());       // Volume
    itemRef->SetAttribute(AttrCapacity,       itemRef->type().capacity());   // Capacity
	itemRef->SaveAttributes();
    return itemRef;
}

uint32 InventoryItem::_Spawn(ItemFactory &factory,
    // InventoryItem stuff:
    ItemData &data
) {
    // obtain type of new item
    // this also checks that the type is valid
    const ItemType *t = factory.GetType(data.typeID);
    if (!t) return 0;

    // fix the name (if empty)
    if (data.name.empty())
        data.name = t->name();

    // insert new entry into DB
    return factory.db().NewItem(data);
}

/* This Spawn function is meant for in-memory only items created from the following categorys...
 *  EVEDB::invCategories::Entity
 *  EVEDB::invCategories::Charge (for launched missiles only)
 *  EVEDB::invCategories::Asteroid
 *
 * these items meant to never be saved to database
 * and be thrown away on server shutdown.
 * Updated 29May15 -Allan
 */
uint32 InventoryItem::_SpawnEntity(ItemFactory &factory,
    // InventoryItem stuff:
    ItemData &data
) {
    // obtain type of new item
    // this also checks that the type is valid
    const ItemType *t = factory.GetType(data.typeID);
    if (!t) return 0;

    // fix the name (if empty)
    if (data.name.empty())
        data.name = t->name();

    // Get a new Entity ID from ItemFactory's ID Authority:
    if (t->categoryID() == EVEDB::invCategories::Asteroid)  //cant use, as mined ore is of category:asteroid
        return factory.GetNextAsteroidID();
    else if (data.flag == EVEItemFlags::flagMissile)
        return factory.GetNextMissileID();
    else
        return factory.GetNextEntityID();
}

void InventoryItem::Delete() {
    //first, get out of client's sight.
    //this also removes us from our inventory.
    Move( 6 );
    ChangeOwner( 2 );

    //take ourself out of the DB
    //attributes.Delete();
    m_factory.db().DeleteItem( itemID() );

    mAttributeMap.Delete();
    mDefaultAttributeMap.Delete();

    //delete ourselves from factory cache
    m_factory._DeleteItem( itemID() );
}

PyPackedRow* InventoryItem::GetItemStatusRow() const
{
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
    into->SetField( "online",        new PyBool( (mAttributeMap.HasAttribute(AttrIsOnline) ? GetAttribute(AttrIsOnline).get_int() : 0) ) );
    into->SetField( "damage",        new PyFloat( (mAttributeMap.HasAttribute(AttrDamage) ? GetAttribute(AttrDamage).get_float() : 0) ) );
    into->SetField( "charge",        new PyFloat( (mAttributeMap.HasAttribute(AttrCapacitorCharge) ? GetAttribute(AttrCapacitorCharge).get_float() : 0) ) );
    into->SetField( "skillPoints",   new PyInt( (mAttributeMap.HasAttribute(AttrSkillPoints) ? GetAttribute(AttrSkillPoints).get_int() : 0) ) );
    into->SetField( "armorDamage",   new PyFloat( (mAttributeMap.HasAttribute(AttrArmorDamageAmount) ? GetAttribute(AttrArmorDamageAmount).get_float() : 0.0) ) );
    into->SetField( "shieldCharge",  new PyFloat( (mAttributeMap.HasAttribute(AttrShieldCharge) ? GetAttribute(AttrShieldCharge).get_float() : 0.0) ) );
    into->SetField( "incapacitated", new PyBool( (mAttributeMap.HasAttribute(AttrIsIncapacitated) ? GetAttribute(AttrIsIncapacitated).get_int() : 0) ) );
}

PyPackedRow* InventoryItem::GetModuleStatusRow() const
{
    DBRowDescriptor* header = new DBRowDescriptor;
        header->AddColumn( "instanceID", DBTYPE_I8 );
        header->AddColumn( "flagID",     DBTYPE_I2 );
        header->AddColumn( "typeID",     DBTYPE_I4 );
        header->AddColumn( "quantity",   DBTYPE_I4 );

    PyPackedRow* row = new PyPackedRow( header );
    GetModuleStatusRow( row );

    return row;
}

void InventoryItem::GetModuleStatusRow( PyPackedRow* into ) const {
    into->SetField( "instanceID",    new PyLong( itemID() ) );
    into->SetField( "flagID",        new PyInt( flag() ) );
    into->SetField( "typeID",        new PyInt( typeID() ) );
    into->SetField( "quantity",      new PyInt( singleton() ? -1 : quantity()) );
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

    //header->AddColumn( "singleton",  DBTYPE_BOOL );
    //header->AddColumn( "stacksize" , DBTYPE_I4 );

    PyPackedRow* row = new PyPackedRow( header );
    GetItemRow( row );

    return row;
}

void InventoryItem::GetItemRow( PyPackedRow* into ) const
{
    into->SetField( "itemID",     new PyLong( itemID() ) );
    into->SetField( "typeID",     new PyInt( typeID() ) );
    into->SetField( "ownerID",    new PyInt( ownerID() ) );
    into->SetField( "locationID", new PyLong( locationID() ) );
    into->SetField( "flagID",     new PyInt( flag() ) );
    into->SetField( "quantity",   new PyInt( singleton() ? -1 : quantity()) );
    into->SetField( "groupID",    new PyInt( groupID() ) );
    into->SetField( "categoryID", new PyInt( categoryID() ) );
    into->SetField( "customInfo", new PyString( customInfo() ) );

    //into->SetField( "singleton",  new PyBool( singleton() ) );
    //into->SetField( "stacksize",  new PyInt (quantity()) );
}

bool InventoryItem::Populate( Rsp_CommonGetInfo_Entry& result )
{//TODO FIXME  this will need to be reworked once POS and Outposts are implemented.
    /*  may not be quite right.  look into later...
              [PyString "shipInfo"]
              [PyDict 14 kvp]
                [PyIntegerVar 1006132995446]
                [PyObjectData Name: util.KeyVal]
                  [PyDict 5 kvp]
                    [PyString "itemID"]
                    [PyIntegerVar 1006132995446]
                    [PyString "attributes"]
                    [PyDict 17 kvp]
                      [PyInt 161]
                      [PyFloat 5]
                      ........
                      [PyInt 565]
                      [PyFloat 0.56]
                    [PyString "invItem"]
                    [PyPackedRow 37 bytes]
                      ["itemID" => <1006132995446> [I8]]
                      ["typeID" => <16301> [I4]]
                      ["ownerID" => <1661059544> [I4]]
                      ["locationID" => <1006132945754> [I8]]
                      ["flagID" => <12> [I2]]
                      ["quantity" => <-1> [I4]]
                      ["groupID" => <315> [I4]]
                      ["categoryID" => <7> [I4]]
                      ["customInfo" => <empty string> [Str]]
                    [PyString "time"]
                    [PyIntegerVar 129773015518415424]
                    [PyString "activeEffects"]
                    [PyDict 1 kvp]
                      [PyInt 16]
                      [PyList 11 items]
                        [PyIntegerVar 1006132995446]
                        [PyIntegerVar 1661059544]
                        [PyIntegerVar 1006132945754]
                        [PyNone]
                        [PyNone]
                        [PyList 0 items]
                        [PyInt 16]
                        [PyIntegerVar 129773015508502912]
                        [PyInt -1]
                        [PyInt 1]
                        [PyNone]
            */

    //itemID:
    result.itemID = itemID();

    //invItem:
    PySafeDecRef( result.invItem );
    result.invItem = GetItemRow();

    if ( IsOnline() )
    {
        //there is an effect that goes along with this. We should
        //probably be properly tracking the effect due to some
        // timer things, but for now, were hacking it.
        EntityEffectState es;
        es.env_itemID = itemID();
        es.env_charID = ownerID();  //may not be quite right...
        es.env_shipID = locationID();
        es.env_target = locationID();   //this is what they do.
        es.env_other = new PyNone;
        es.env_area = new PyNone;
        es.env_effectID = effectOnline;
        es.startTime = Win32TimeNow();
        es.duration = INT_MAX;
        es.repeat = 0;
        es.randomSeed = new PyNone;

        result.activeEffects[es.env_effectID] = es.Encode();
    }

    //activeEffects:
    //result..activeEffects[id] = List[11];

    //attributes:
    AttributeMap::AttrMapItr itr = mAttributeMap.begin();
    for (; itr != mAttributeMap.end(); itr++) {
        result.attributes[(*itr).first] = (*itr).second.GetPyObject();
    }

    result.time = Win32TimeNow();
    return true;
}

PyList* InventoryItem::GetItemInfo()
{
    PyList* itemInfo = new PyList;
        itemInfo->AddItem(GetItemRow());

    return itemInfo;
}

PyObject* InventoryItem::ItemGetInfo()
{/*
    [PyTuple 1 items]
      [PySubStream 556 bytes]
        [PyObjectData Name: util.KeyVal]
          [PyDict 5 kvp]
            [PyString "itemID"]
            [PyIntegerVar 1002331327445]
            [PyString "attributes"]
            [PyDict 33 kvp]
            [PyString "invItem"]
            [PyPackedRow 33 bytes]
              ["itemID" => <1002331327445> [I8]]
              ["typeID" => <453> [I4]]
              ["ownerID" => <1661059544> [I4]]
              ["locationID" => <1002331681462> [I8]]
              ["flagID" => <27> [I2]]
              ["quantity" => <-1> [I4]]
              ["groupID" => <53> [I2]]
              ["categoryID" => <7> [I2]]
              ["customInfo" => <empty string> [Str]]
            [PyString "time"]
            [PyIntegerVar 129515452804044553]
            [PyString "activeEffects"]
            [PyDict 0 kvp]
    [PyNone]
    */
    Rsp_ItemGetInfo result;

    if (!Populate(result.entry))
        return NULL;    //print already done.

    return(result.Encode());
}

void InventoryItem::Rename(const char *to) {

    m_itemName = to;
    //SaveItem();
}

void InventoryItem::MoveInto(Inventory &new_home, EVEItemFlags _flag, bool notify) {
    Move( new_home.inventoryID(), _flag, notify );
}

void InventoryItem::Move(uint32 new_location, EVEItemFlags new_flag, bool notify) {
    uint32 old_location = locationID();
    EVEItemFlags old_flag = flag();

    if ( new_location == old_location && new_flag == old_flag )
        return; //nothing to do...

    //first, take myself out of my old inventory, if its loaded.
    Inventory *old_inventory = m_factory.GetInventory( old_location, false );
    if (old_inventory != NULL)
        old_inventory->RemoveItem( InventoryItemRef( this ) );  //releases its ref

    m_locationID = new_location;
    m_flag = new_flag;

    //then make sure that my new inventory is updated, if its loaded.
    Inventory *new_inventory = m_factory.GetInventory( new_location, false );
    if ( new_inventory != NULL )
        new_inventory->AddItem( InventoryItemRef( this ) ); //makes a new ref

    SaveItem();

    //notify about the changes.
    if ( notify )
    {
        std::map<int32, PyRep *> changes;

        if ( new_location != old_location )
            changes[ixLocationID] = new PyInt(old_location);
        if ( new_flag != old_flag )
            changes[ixFlag] = new PyInt(old_flag);

        SendItemChange( ownerID(), changes );   //changes is consumed
    }
}

bool InventoryItem::AlterQuantity(int32 qty_change, bool notify) {
    if (qty_change == 0)
        return true;

    int32 new_qty = m_quantity + qty_change;

    if (new_qty < 0) {
        codelog(ITEM__ERROR, "%s (%u): Tried to remove %d quantity from stack of %u", m_itemName.c_str(), m_itemID, -qty_change, m_quantity);
        return false;
    }

    return(SetQuantity(new_qty, notify));
}

bool InventoryItem::SetQuantity(uint32 qty_new, bool notify) {
    //if an object has its singleton set then it shouldn't be able to add/remove qty
    if (m_singleton) {
        //Print error
        codelog(ITEM__ERROR, "%s (%u): Failed to set quantity %u , the items singleton bit is set", m_itemName.c_str(), m_itemID, qty_new);
        //return false
        return false;
    }

    uint32 old_qty = m_quantity;

    m_quantity = qty_new;

    SaveItem();

    //notify about the changes.
    if (notify) {
        std::map<int32, PyRep *> changes;

        //send the notify to the new owner.
        changes[ixQuantity] = new PyInt(old_qty);
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
	changes[ixFlag] = new PyInt(new_flag);
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
        typeID(),
        ownerID(),
        (notify ? 1 : locationID()), //temp location to cause the spawn via update
        flag(),
        qty_to_take
    );

    InventoryItemRef res = m_factory.SpawnItem(idata);
    if (notify)
        res->Move( locationID(), flag() );

    return( res );
}

bool InventoryItem::Merge(InventoryItemRef to_merge, int32 qty, bool notify) {
    /*[00m14:51:55 [Error] Metal Scraps (140000072) in location 60014809, flag 4: Asked to merge with item 140000087 in location 60014809, flag 4.
     * 14:51:55 [Error] EMP S (140000108) in location 60014809, flag 4: Asked to merge with item 140000109 in location 60014809, flag 4.
     * 14:51:55 [Error] EMP S (140000108) in location 60014809, flag 4: Asked to merge with item 140000129 in location 60014809, flag 4.
     * 14:51:55 [Error] Tritanium (140000067) in location 60014809, flag 4: Asked to merge with item 140000131 in location 60014809, flag 4.
     */
    /*
    if (locationID() != to_merge->locationID()) {
        if (! (flag() == flagHangar) && ( (to_merge->flag() >= flagHiSlot0) || (to_merge->flag() <= flagHiSlot7) )) {
            _log(ITEM__ERROR, "%s (%u) in locatio
            // remove item from SystemManagern %u asked to merge with item %u in location %u.", itemName().c_str(), itemID(), locationID(), to_merge->itemID(), to_merge->locationID());
            return false;
        }
    }
    if (locationID() == to_merge->locationID()) {
        if ((!( (flag() == flagCargoHold) && ( (to_merge->flag() >= flagHiSlot0) || (to_merge->flag() <= flagHiSlot7) ))) || \
            (!( (flag() == flagHangar) && (to_merge->flag() == flagHangar) ))) {
            _log(ITEM__ERROR, "%s (%u) in location %u, flag %u: Asked to merge with item %u in location %u, flag %u.", itemName().c_str(), itemID(), locationID(), flag(), to_merge->itemID(), to_merge->locationID(), to_merge->flag());
            return false;
        }
    }
    */
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

void InventoryItem::SaveItem()
{
    //_log( ITEM__TRACE, "Saving Item %u.", itemID() );

    //SaveAttributes();

    m_factory.db().SaveItem(
        itemID(),
        ItemData(
            itemName().c_str(),
            typeID(),
            m_ownerID,
            m_locationID,
            flag(),
            m_contraband,
            m_singleton,
            m_quantity,
            position(),
            customInfo().c_str()
        )
    );
}

void InventoryItem::SaveShipState()
{
    mAttributeMap.SaveShipState();
}

//contents of changes are consumed and cleared
void InventoryItem::SendItemChange(uint32 toID, std::map<int32, PyRep *> &changes) const {
    //TODO: figure out the appropriate list of interested people...
    Client *c = sEntityList.FindClientByCharID(toID);
    if (!c) return; //not found or not online...

    NotifyOnItemChange change;
        change.itemRow = GetItemRow();
        change.changes = changes;

    changes.clear();    //reset change map for next update.

    PyTuple *tmp = change.Encode();  //this is consumed below
    c->SendNotification("OnItemChange", "charid", &tmp, false); //unsequenced.
}

void InventoryItem::SetOnline(bool online) {
    //  this is only used by modules    ** check for pos structures also!! **
    if (!SetAttribute(AttrIsOnline, int(online))) {
        _log(ITEM__TRACE, "InventoryItem::SetOnline()", "module %s(%u) could not be set %s", \
        m_itemName.c_str(), m_itemID, (online ? "Online" : "Offline"));
        return;
    }

    Client* pClient = sEntityList.FindClientByCharID(m_ownerID);
    if (!pClient) {
        _log(ITEM__TRACE, "InventoryItem::SetOnline()", "No client object found using m_ownerID (%u) for module %s(%u)", \
             m_ownerID, m_itemName.c_str(), m_itemID );
        return;
    }
    if (pClient->IsDocked())
        return;

    GodmaEnvironment ge;
        ge.selfID = m_itemID;
        ge.charID = m_ownerID;
        ge.shipID = pClient->GetShipID();
        ge.targetID = 0;
        ge.other = new PyNone;
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
        shipEff.error = new PyNone;
    PyList* events = new PyList;
        events->AddItem(shipEff.Encode());
    Notify_OnMultiEvent multi;
        multi.events = events;
    PyTuple* tmp = multi.Encode();
    pClient->SendNotification("OnMultiEvent", "clientID", &tmp);
}

void InventoryItem::SetCustomInfo(const char *ci) {
    if (ci == NULL)
        m_customInfo = "";
    else
        m_customInfo = ci;
    //SaveItem();
}

void InventoryItem::Relocate(const GPoint &pos) {
    if (m_position == pos)
        return;
    m_position = pos;
    //SaveItem();
}

bool InventoryItem::SetAttribute( uint32 attributeID, int64 num, bool notify /* true */, bool shadow_copy_to_default_set /* false */ )
{
    EvilNumber devil_number(num);
	bool status = mAttributeMap.SetAttribute(attributeID, devil_number, notify);
	if (shadow_copy_to_default_set)
		status = status && mDefaultAttributeMap.SetAttribute(attributeID, devil_number, notify);
	return status;
}

bool InventoryItem::SetAttribute( uint32 attributeID, double num, bool notify /* true */, bool shadow_copy_to_default_set /* false */ )
{
    EvilNumber devil_number(num);
    bool status = mAttributeMap.SetAttribute(attributeID, devil_number, notify);
	if (shadow_copy_to_default_set)
		status = status && mDefaultAttributeMap.SetAttribute(attributeID, devil_number, notify);
	return status;
}

bool InventoryItem::SetAttribute( uint32 attributeID, EvilNumber num, bool notify /* true */, bool shadow_copy_to_default_set /* false */ )
{
    bool status = mAttributeMap.SetAttribute(attributeID, num, notify);
	if (shadow_copy_to_default_set)
		status = status && mDefaultAttributeMap.SetAttribute(attributeID, num, notify);
	return status;
}

bool InventoryItem::SetAttribute( uint32 attributeID, int num, bool notify /* true */, bool shadow_copy_to_default_set /* false */ )
{
    EvilNumber devil_number(num);
    bool status = mAttributeMap.SetAttribute(attributeID, devil_number, notify);
	if (shadow_copy_to_default_set)
		status = status && mDefaultAttributeMap.SetAttribute(attributeID, devil_number, notify);
	return status;
}

bool InventoryItem::SetAttribute( uint32 attributeID, uint64 num, bool notify /* true */, bool shadow_copy_to_default_set /* false */ )
{
    EvilNumber devil_number(*((int64*)&num));
    bool status = mAttributeMap.SetAttribute(attributeID, devil_number, notify);
	if (shadow_copy_to_default_set)
		status = status && mDefaultAttributeMap.SetAttribute(attributeID, devil_number, notify);
	return status;
}

bool InventoryItem::SetAttribute( uint32 attributeID, uint32 num, bool notify /* true */, bool shadow_copy_to_default_set /* false */ )
{
    EvilNumber devil_number((int64)num);
    bool status = mAttributeMap.SetAttribute(attributeID, devil_number, notify);
	if (shadow_copy_to_default_set)
		status = status && mDefaultAttributeMap.SetAttribute(attributeID, devil_number, notify);
	return status;
}

EvilNumber InventoryItem::GetAttribute( const uint32 attributeID ) const {
     return mAttributeMap.GetAttribute(attributeID);
}

EvilNumber InventoryItem::GetDefaultAttribute( const uint32 attributeID ) const {
     return mDefaultAttributeMap.GetAttribute(attributeID);
}
/*
EvilNumber InventoryItem::GetAttribute( const uint32 attributeID, const uint32 defaultValue ) const {
     return mAttributeMap.GetAttribute(attributeID, defaultValue);
}
*/
bool InventoryItem::HasAttribute(const uint32 attributeID) const {
    return mAttributeMap.HasAttribute(attributeID);
}

bool InventoryItem::HasAttribute(const uint32 attributeID, EvilNumber &value) const {
    return mAttributeMap.HasAttribute(attributeID, value);
}

bool InventoryItem::SaveAttributes() {
	return (mAttributeMap.SaveAttributes() && mDefaultAttributeMap.SaveAttributes());
}

bool InventoryItem::ResetAttribute(uint32 attrID, bool notify /* false */) {
    return mAttributeMap.ResetAttribute(attrID, notify);
}
