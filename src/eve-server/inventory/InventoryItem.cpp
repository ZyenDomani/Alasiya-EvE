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
    Updates:     Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "ConsoleCommands.h"
#include "EntityList.h"
#include "character/Skill.h"
#include "effects/EffectsProcessor.h"
#include "exploration/Probes.h"
#include "manufacturing/Blueprint.h"
#include "pos/Structure.h"
#include "ship/Ship.h"
#include "ship/modules/ModuleItem.h"
#include "station/Station.h"
#include "station/StationOffice.h"
#include "system/Asteroid.h"
#include "system/Celestial.h"
#include "system/Container.h"

/*
 * InventoryItem
 */
InventoryItem::InventoryItem(uint32 _itemID, const ItemType& _type, const ItemData& _data)
: RefObject(0),
pAttributeMap(new AttributeMap(*this)),
pInventory(nullptr),      // this is created/destroyed in derived classes as needed.
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

    m_modifiers.clear();

    // placeholder for fx timestamp, once implemented
    m_timestamp = 0;

    _log(ITEM__TRACE, "Created Generic Item %p for item %s (%u).", this, m_itemName.c_str(), m_itemID);
}

InventoryItem::~InventoryItem() noexcept
{
    SafeDelete(pAttributeMap);
}

InventoryItem::InventoryItem(const InventoryItem& oth)
: RefObject(0),
m_itemID(oth.m_itemID),
m_type(oth.m_type)
{
    sLog.Error("InventoryItem()", "InventoryItem copy c'tor called.");
    EvE::traceStack();
    assert(0);
}

/* see notes about these in header
InventoryItem::InventoryItem(const InventoryItemRef oth)
: RefObject(0)
{
    std::map<uint16, EvilNumber> attrMap;
    oth->GetAttributeMap()->CopyAttributes(attrMap);

    m_itemID = oth->m_itemID;
    m_type = oth->m_type;
}
*/

InventoryItem::InventoryItem(InventoryItem&& oth) noexcept
: RefObject(0),
m_itemID(oth.m_itemID),
m_type(oth.m_type)
{
    sLog.Error("InventoryItem()", "InventoryItem move c'tor called.");
    EvE::traceStack();
    assert(0);
}
/*
InventoryItem& InventoryItem::operator=(InventoryItem&& oth) noexcept
{
    sLog.Error("InventoryItem()", "InventoryItem move op called.");
    EvE::traceStack();
    assert(0);
}
*/

InventoryItemRef InventoryItem::Load( uint32 itemID)
{
    return InventoryItem::Load<InventoryItem>(itemID);
}

InventoryItemRef InventoryItem::SpawnItem( uint32 itemID, const ItemData &data)
{
    if (data.quantity == 0)
        return InventoryItemRef(nullptr);
    const ItemType *iType = sItemFactory.GetType( data.typeID);
    if (iType == nullptr)
        return InventoryItemRef(nullptr);
    InventoryItemRef iRef = InventoryItemRef( new InventoryItem(itemID, *iType, data));
    if (iRef.get() == nullptr)
        return iRef;

    iRef->_Load();
	return iRef;
}

uint32 InventoryItem::CreateItemID( ItemData &data) {
    // obtain type of new item
    const ItemType *iType = sItemFactory.GetType(data.typeID);
    if (iType == nullptr) {
        codelog(ITEM__ERROR, "Invalid type returned for typeID %u", data.typeID);
        return 0;
    }
    // fix the name (if empty)
    if (data.name.empty())
        data.name = iType->name();

    if (data.locationID == 0)
        if (is_log_enabled(ITEM__TRACE)) {
            _log(ITEM__MESSAGE, "LocationID = 0 for item");
            EvE::traceStack();
        }

    // insert new entry into DB
    return ItemDB::NewItem(data);
}

/* This Spawn function is meant for in-memory only items created from the following categorys...
 *  EVEDB::invCategories::Entity (for npcs)
 *  EVEDB::invCategories::Charge (for launched missiles only)
 *  EVEDB::invCategories::Container (for Position Tracking only)
 *
 * these items meant to never be saved to database
 * and be thrown away on server shutdown.
 * Updated 29May15 -Allan
 */
uint32 InventoryItem::CreateTempItemID( ItemData &data) {
    // obtain type of new item
    // this also checks that the type is valid
    const ItemType *iType = sItemFactory.GetType(data.typeID);
    if (iType == nullptr) {
        codelog(ITEM__ERROR, "Invalid ItemType returned for typeID %u", data.typeID);
        return 0;
    }

    // fix the name (if empty)
    if (data.name.empty())
        data.name = iType->name();

    // Get a new Entity ID from ItemFactory's ID Authority:
    if (iType->categoryID() == EVEDB::invCategories::Entity) // may need more testing to verify that ONLY NPC's and jetcan markers use this method
        return sItemFactory.GetNextNPCID();

    if (data.flag == EVEItemFlags::flagMissile)
        return sItemFactory.GetNextMissileID();

    return sItemFactory.GetNextTempID();
}

bool InventoryItem::_Load() {
    if (!pAttributeMap->Load()) {
        _log(ITEM__WARNING, "%s (%u): Failed to load attribute map.", m_itemName.c_str(), m_itemID);
        return false;
    }

    return true;
}

template<class _Ty>
RefPtr<_Ty> InventoryItem::_LoadItem( uint32 itemID, const ItemType &type, const ItemData &data) {
    switch( type.categoryID() ) {
        case EVEDB::invCategories::_System:
        case EVEDB::invCategories::Material:    // includes minerals
        case EVEDB::invCategories::Trading:
        case EVEDB::invCategories::Bonus:
        case EVEDB::invCategories::PlanetaryInteraction:
        case EVEDB::invCategories::PlanetaryResources:
        case EVEDB::invCategories::PlanetaryCommodities:
        case EVEDB::invCategories::Commodity:
        case EVEDB::invCategories::Accessories: { // this is for bookmark vouchers
            // Generic item, create one
            return InventoryItemRef( new InventoryItem(itemID, type, data ));
        } break;
        case EVEDB::invCategories::Deployable: // these may need their own class.  not sure yet
        case EVEDB::invCategories::Drone:
        case EVEDB::invCategories::Implant:
        case EVEDB::invCategories::Reaction: {
            // create Generic item for now
            return InventoryItemRef( new InventoryItem(itemID, type, data ));
        } break;
        case EVEDB::invCategories::Module: {
            return ModuleItem::_LoadItem<ModuleItem>(itemID, type, data);
        } break;
        case EVEDB::invCategories::Owner: {
            return Character::_LoadItem<Character>(itemID, type, data);
        } break;
        case EVEDB::invCategories::Skill: {
            return Skill::_LoadItem<Skill>(itemID, type, data);
        } break;
        case EVEDB::invCategories::Blueprint: {
            return Blueprint::_LoadItem<Blueprint>(itemID, type, data);
        } break;
        case EVEDB::invCategories::Asteroid: {
            if (IsAsteroid(itemID))
                return AsteroidItem::_LoadItem<AsteroidItem>(itemID, type, data);
            // mined ore.  create default item
            return InventoryItemRef( new InventoryItem(itemID, type, data ));
        } break;
        case EVEDB::invCategories::Ship: {
            return ShipItem::_LoadItem<ShipItem>(itemID, type, data);
        } break;
        case EVEDB::invCategories::Structure:
        case EVEDB::invCategories::Orbitals:
        case EVEDB::invCategories::SovereigntyStructure:
        case EVEDB::invCategories::StructureUpgrade: {
            return StructureItem::_LoadItem<StructureItem>(itemID, type, data);
        } break;
        case EVEDB::invCategories::Charge: {      // probes are charges.
            switch (type.groupID()) {
                case EVEDB::invGroups::Scanner_Probe:
                case EVEDB::invGroups::Survey_Probe:
                case EVEDB::invGroups::Warp_Disruption_Probe:
                case EVEDB::invGroups::Obsolete_Probes: {   // make error for these?
                    return ProbeItem::_LoadItem<ProbeItem>(itemID, type, data);
                } break;
                default: {
                    // create generic item for other charge types
                    return InventoryItemRef( new InventoryItem(itemID, type, data ));
                } break;
            }
        } break;
        case EVEDB::invCategories::Station: {
            // test for office first
            if (type.id() == 27) {
                return StationOffice::_LoadItem<StationOffice>(itemID, type, data);
            } else if (type.groupID() != EVEDB::invGroups::Station) {
                return StationItem::_LoadItem<StationItem>(itemID, type, data);
            }/* else if (type.groupID() != EVEDB::invGroups::Station_Services) {
               this isnt written yet...
            } */
        } break;
        case EVEDB::invCategories::Celestial: {
            switch (type.groupID()) {
                case EVEDB::invGroups::Solar_System: {
                    return SolarSystem::_LoadItem<SolarSystem>(itemID, type, data);
                } break;
                case EVEDB::invGroups::Secure_Cargo_Container:
                case EVEDB::invGroups::Cargo_Container:
                case EVEDB::invGroups::Freight_Container:
                case EVEDB::invGroups::Audit_Log_Secure_Container:
                case EVEDB::invGroups::Mission_Container: {
                    return CargoContainer::_LoadItem<CargoContainer>(itemID, type, data);
                } break;
                case EVEDB::invGroups::Wreck: {
                    return WreckContainer::_LoadItem<WreckContainer>(itemID, type, data);
                } break;
                case EVEDB::invGroups::Force_Field:
                default: {
                    // generic Celestial Object
                    return CelestialObject::_LoadItem<CelestialObject>(itemID, type, data);
                } break;
            }
        } break;
        // there are 216 groups in the entity category.  im not testing all of them.
        case EVEDB::invCategories::Entity: {
            switch (type.groupID()) {
                case EVEDB::invGroups::Spawn_Container: {
                    return CargoContainer::_LoadItem<CargoContainer>(itemID, type, data);
                } break;
                case EVEDB::invGroups::Temporary_Cloud: {
                    return CelestialObject::_LoadItem<CelestialObject>(itemID, type, data);
                } break;
                case EVEDB::invGroups::Billboard:
                // the rest are drones and npcs....i think
                // they are *somewhat* separated for eventual classification into their own itemtypes
                case EVEDB::invGroups::Convoy:
                case EVEDB::invGroups::Convoy_Drone:
                //  added checks for all npc's   -allan 26Dec14
                case EVEDB::invGroups::Drones:
                case EVEDB::invGroups::Police_Drone:
                case EVEDB::invGroups::Concord_Drone:
                case EVEDB::invGroups::Customs_Official:

                case EVEDB::invGroups::Sentry_Gun:
                case EVEDB::invGroups::Mobile_Sentry_Gun:
                case EVEDB::invGroups::Protective_Sentry_Gun:
                case EVEDB::invGroups::Destructible_Sentry_Gun:

                case EVEDB::invGroups::Pirate_Drone:
                case EVEDB::invGroups::Tutorial_Drone:

                case EVEDB::invGroups::Storyline_Frigate:
                case EVEDB::invGroups::Storyline_Cruiser:
                case EVEDB::invGroups::Storyline_Battleship:
                case EVEDB::invGroups::Storyline_Mission_Frigate:
                case EVEDB::invGroups::Storyline_Mission_Cruiser:
                case EVEDB::invGroups::Storyline_Mission_Battleship:

                case EVEDB::invGroups::Deadspace_Sleeper_Sleepless_Sentinel:
                case EVEDB::invGroups::Deadspace_Sleeper_Awakened_Sentinel:
                case EVEDB::invGroups::Deadspace_Sleeper_Emergent_Sentinel:
                 /* start at EVEDB::invGroups::Deadspace_Sleeper_Sleepless_Defender:
                end at EVEDB::invGroups::Deadspace_Sleeper_Emergent_Patroller:
                */
                case EVEDB::invGroups::Incursion_Sanshas_Nation_Industrial:
                case EVEDB::invGroups::Incursion_Sanshas_Nation_Capital:
                case EVEDB::invGroups::Incursion_Sanshas_Nation_Frigate:
                case EVEDB::invGroups::Incursion_Sanshas_Nation_Cruiser:
                case EVEDB::invGroups::Incursion_Sanshas_Nation_Battleship: {
                    _log(ITEM__WARNING, "item %u (type %u, group %u) defaulting to generic InventoryItem.", itemID, type.id(), type.groupID());
                } break;
                default: {
                    _log(ITEM__WARNING, "item %u (type %u, group %u,  cat %u) is not handled in InventoryItem::_LoadItem::Entity.", itemID, type.id(), type.groupID(), type.categoryID());
                }
            }
        }
        default: {
            _log(ITEM__WARNING, "item %u (type %u, group %u,  cat %u) is not handled in InventoryItem::_LoadItem.", itemID, type.id(), type.groupID(), type.categoryID());
        } break;
    }
    // Generic item, create one:
    return InventoryItemRef( new InventoryItem(itemID, type, data ));
}

InventoryItemRef InventoryItem::SpawnTemp(ItemData& data)
{
    /** @todo will need to update this to get 'proper' item creation for temps... */
    //  for now, return generic item
    return InventoryItem::SpawnItem(InventoryItem::CreateTempItemID(data), data);
}

// called from generic SpawnItem()
InventoryItemRef InventoryItem::Spawn( ItemData &data)
{
    // obtain type of new item
    const ItemType *iType = sItemFactory.GetType( data.typeID);
    if (iType == nullptr) {
        codelog(ITEM__ERROR, "Invalid type returned for typeID %u", data.typeID);
        return InventoryItemRef(nullptr);
    }

    switch( iType->categoryID() ) { //23
        case EVEDB::invCategories::Owner: {
            assert(0);  // this needs to make a serious error here....these CANNOT be called from here using the generic InventoryItem::Spawn() method
        } break;
        //! TODO not handled....use generic item
        case EVEDB::invCategories::Subsystem:
        case EVEDB::invCategories::Decryptors:
        case EVEDB::invCategories::StructureUpgrade:
            // these next 3 *may* need their own class
        case EVEDB::invCategories::PlanetaryInteraction:
        case EVEDB::invCategories::PlanetaryResources:
        case EVEDB::invCategories::PlanetaryCommodities:
        case EVEDB::invCategories::Material:    // includes minerals
        case EVEDB::invCategories::Trading:
        case EVEDB::invCategories::Bonus:
        case EVEDB::invCategories::Commodity:
        case EVEDB::invCategories::Implant:
        case EVEDB::invCategories::AncientRelics:   // t3 bpc from sleepers
        case EVEDB::invCategories::Accessories: // this is for bookmark vouchers
        case EVEDB::invCategories::Asteroid:  // ore is "asteroid" also.....i forgot about that
        case EVEDB::invCategories::Reaction: {
            _log(ITEM__WARNING, "InventoryItem::Spawn creating generic item for type %u, cat %u.", iType->id(), iType->categoryID());
            // Spawn generic item:
            uint32 itemID = InventoryItem::CreateItemID(data);
            return InventoryItem::SpawnItem(itemID, data);
        } break;
        case EVEDB::invCategories::Orbitals:
        case EVEDB::invCategories::Structure:
        case EVEDB::invCategories::SovereigntyStructure: {
            return StructureItem::Spawn(data);
        } break;
        case EVEDB::invCategories::Blueprint: {
            BlueprintData bpData = BlueprintData();
                bpData.runs = -1;
            return Blueprint::Spawn(data, bpData);
        } break;
        case EVEDB::invCategories::Skill: {
            return Skill::Spawn(data);
        } break;
        case EVEDB::invCategories::Ship: {
            return ShipItem::Spawn(data);
        } break;
        case EVEDB::invCategories::Entity: {
            switch (iType->groupID()) {
                case EVEDB::invGroups::Spawn_Container: {
                    return CargoContainer::Spawn(data);
                } break;
                case EVEDB::invGroups::Billboard:
                case EVEDB::invGroups::Control_Bunker:
                case EVEDB::invGroups::Capture_Point: {
                    uint32 itemID = InventoryItem::CreateItemID(data);
                    return InventoryItem::SpawnItem(itemID, data);
                } break;
                case EVEDB::invGroups::Sentry_Gun:
                case EVEDB::invGroups::Temporary_Cloud:
                default: {  // *should*  be all npcs
                    // use temp items and NOT save to db.
                    uint32 itemID = InventoryItem::CreateTempItemID(data);
                    return InventoryItem::SpawnItem(itemID, data);
                } break;
            }
        } break;
        case EVEDB::invCategories::Drone: {
            if (!sConfig.testing.EnableDrones) {
                // disable drones
                return InventoryItemRef(nullptr);
            }
        }   // allow fallthru if drones are enabled  - may need specific DroneItem later.
        case EVEDB::invCategories::Module:
        case EVEDB::invCategories::Deployable: {
            // Spawn generic item:
            uint32 itemID = InventoryItem::CreateItemID(data);
            InventoryItemRef itemRef = InventoryItem::SpawnItem(itemID, data);
            if (itemRef.get() == nullptr)
                return InventoryItemRef(nullptr);
            // THESE SHOULD BE MOVED INTO A _type::Spawn() function that does not exist yet
            itemRef->SetAttribute(AttrMass,           iType->mass(), false);           // Mass
            itemRef->SetAttribute(AttrRadius,         iType->radius(), false);       // Radius
            itemRef->SetAttribute(AttrVolume,         iType->volume(), false);       // Volume
            itemRef->SetAttribute(AttrCapacity,       iType->capacity(), false);   // Capacity
            return itemRef;
        } break;
        case EVEDB::invCategories::Charge: {
            uint32 itemID = 0;
            InventoryItemRef itemRef;
            switch (data.flag) {
                case EVEItemFlags::flagMissile: {
                    // Spawn launched missile item in EVEMU_MISSILE_ID range and does NOT save missile to db
                    itemID = InventoryItem::CreateTempItemID(data);
                    itemRef = InventoryItem::SpawnItem(itemID, data);
                } break;
                default: {
                    switch (iType->groupID()) {
                        case EVEDB::invGroups::Scanner_Probe:
                        case EVEDB::invGroups::Survey_Probe:
                        case EVEDB::invGroups::Warp_Disruption_Probe:
                        case EVEDB::invGroups::Obsolete_Probes: {   // make error for these?
                            return ProbeItem::Spawn(data);
                        } break;
                        default: {
                            // create generic item for other charge types
                            itemID = InventoryItem::CreateItemID(data);
                            itemRef = InventoryItem::SpawnItem(itemID, data);
                        } break;
                    }
                }
            }
            if (itemRef.get() == nullptr)
                return InventoryItemRef(nullptr);
            itemRef->SetAttribute(AttrMass,       iType->mass(), false);           // Mass
            itemRef->SetAttribute(AttrRadius,     iType->radius(), false);       // Radius
            return itemRef;
        } break;
        case EVEDB::invCategories::Station: {
            if (iType->groupID() == EVEDB::invGroups::Station) {
            uint32 itemID = StationItem::CreateItemID(data);
            if (itemID == 0)
                return StationItemRef(nullptr);
            StationItemRef stationRef = StationItem::Load(itemID);
            if (stationRef.get() == nullptr)
                return StationItemRef(nullptr);
            // THESE SHOULD BE MOVED INTO A Station::Spawn() function that does not exist yet
            stationRef->SetAttribute(AttrShieldCharge,  stationRef->GetAttribute(AttrShieldCapacity), false);     // Shield Charge
            stationRef->SetAttribute(AttrArmorDamage,   EvilZero, false);                                         // Armor Damage
            stationRef->SetAttribute(AttrMass,          iType->mass(), false);           // Mass
            stationRef->SetAttribute(AttrRadius,        iType->radius(), false);       // Radius
            stationRef->SetAttribute(AttrVolume,        iType->volume(), false);       // Volume
            stationRef->SetAttribute(AttrCapacity,      iType->capacity(), false);   // Capacity
            return stationRef;
            } else if (iType->groupID() == EVEDB::invGroups::Station_Services) {
                // this should never hit...throw error
                codelog(INV__ERROR, "InventoryItem::Spawn called for unhandled item type %u, cat %u in locID: %u.", iType->id(), iType->categoryID(), data.locationID);
            }
        } break;
        case EVEDB::invCategories::Celestial: {
            if ( (iType->groupID() == EVEDB::invGroups::Secure_Cargo_Container)
            or (iType->groupID() == EVEDB::invGroups::Cargo_Container)
            or (iType->groupID() == EVEDB::invGroups::Freight_Container)
            or (iType->groupID() == EVEDB::invGroups::Audit_Log_Secure_Container)
            or (iType->groupID() == EVEDB::invGroups::Mission_Container)) {
                return CargoContainer::Spawn(data);
            } else if (iType->groupID() == EVEDB::invGroups::Wreck) {
                return WreckContainer::Spawn(data);
            } else if (iType->groupID() == EVEDB::invGroups::Force_Field) {
                // Spawn force field item in EVEMU_TEMP_ENTITY_ID range and does NOT save Force_Field to db
                uint32 itemID = InventoryItem::CreateTempItemID(data);
                return InventoryItem::SpawnItem(itemID, data);
            } else {
                // Spawn new Celestial Object
                return CelestialObject::Spawn(data);
            }
        } break;
    }

    // log error and return nullref for items not handled here
    _log(ITEM__WARNING, "InventoryItem::Spawn item not handled - type %u, grp %u, cat %u.", iType->id(), iType->groupID(), iType->categoryID());
    return InventoryItemRef(nullptr);
}

// item manipulation methods
void InventoryItem::AddItem(InventoryItemRef iRef)
{
    // make error for invalid inventory?
    // only happens on char creation, when system isnt loaded yet, so we really dont need it.
    if (pInventory != nullptr)
        pInventory->AddItem(iRef);
}

void InventoryItem::RemoveItem(InventoryItemRef iRef)
{
    if (pInventory != nullptr)  // just in case
        pInventory->RemoveItem(iRef);
}

void InventoryItem::Delete() {
    if (!IsNPCCorp(ownerID())) {
        //first, get out of client's sight.
        //this also removes us from our inventory.
        Move(0, flagAutoFit, true);
    }

    if (pAttributeMap != nullptr)   // should never be null, but just in case
        pAttributeMap->Delete();
    //take ourself out of the DB
    ItemDB::DeleteItem(m_itemID);
    //delete ourselves from factory cache
    sItemFactory.RemoveItem(m_itemID);
}

void InventoryItem::ToVirtual(uint32 locationID)
{
    if (pInventory != nullptr)  // just in case
        pInventory->RemoveItem(InventoryItemRef(this));
    if (pAttributeMap != nullptr)   // should never be null, but just in case
        pAttributeMap->Delete();

    //notify about the changes.
    std::map<int32, PyRep *> changes;
    changes[Inv::Update::Location] = new PyInt(m_locationID);
    SendItemChange( m_ownerID, changes);   //changes is consumed
    m_locationID = locationID;

    //take ourself out of the DB
    ItemDB::DeleteItem(m_itemID);
    //delete ourselves from factory cache
    sItemFactory.RemoveItem(m_itemID);
}

void InventoryItem::Rename(std::string name)
{
    m_itemName = name;
    SaveItem();

    PyList* list = new PyList();
        list->AddItem(new PyInt(m_itemID));
        list->AddItem(new PyString(name));
        list->AddItem(new PyFloat(0));
        list->AddItem(new PyFloat(0));
        list->AddItem(new PyFloat(0));
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString("evelocations"));
        tuple->SetItem(1, list);

    // get owner
    if (IsCharacter(m_ownerID)) {
        // this will be the most-used case
        Client* pClient = sEntityList.FindClientByCharID(m_ownerID);
        if (pClient == nullptr)
            return;  //  make error here?
        if (pClient->IsDocked())
            pClient->SendNotification("OnCfgDataChanged", "charid", &tuple, false); //unsequenced.
        else // client in space.  sent update to all clients in bubble
            pClient->GetShipSE()->SysBubble()->BubblecastSendNotification("OnCfgDataChanged", "solarsystemid", &tuple, false);
    } else if (IsPlayerCorp(m_ownerID))
        // bcast to all online corp members
        ;

}

void InventoryItem::Donate(uint32 new_owner, uint32 new_location, EVEItemFlags new_flag, bool notify/*true*/)
{
    if ((new_location == m_locationID) and (new_flag == m_flag) and (new_owner == m_ownerID))
        return; //nothing to do...

    InventoryItemRef iRef(nullptr);
    uint32 old_location = m_locationID, old_owner = m_ownerID;
    EVEItemFlags old_flag = m_flag;

    if (new_location != m_locationID) {
        // remove from current location
        if (IsValidLocation(m_locationID)) {
            iRef = sItemFactory.GetItem(m_locationID);
            if (iRef.get() != nullptr)
                iRef->RemoveItem(InventoryItemRef(this));
            else
                _log(INV__WARNING, "Donate(): Location %u not found. Could not remove %s from it's inventory.", m_locationID, itemName().c_str());
        }
    }

    // update data
    m_flag = new_flag;
    m_ownerID = new_owner;
    m_locationID = new_location;

    if (old_location != m_locationID) {
        // add to new location
        if (IsValidLocation(m_locationID)) {
            iRef = sItemFactory.GetItem(m_locationID);
            if (iRef.get() != nullptr)
                iRef->AddItem(InventoryItemRef(this));
            else
                _log(INV__WARNING, "Donate(): Location %u not found. Could not add %s to it's inventory.", m_locationID, itemName().c_str());
        }
    }

    if ((old_flag != new_flag) and is_log_enabled(INV__TRACE))
        _log(INV__TRACE, "InventoryItem::Donate()  Updated flag on %s(%u) from %s to %s.", \
                itemName().c_str(), itemID(), sDataMgr.GetFlagName(old_flag), sDataMgr.GetFlagName(new_flag));

    //if (sConfig.world.saveOnUpdate or sConfig.world.saveOnMove)
    //    SaveItem();
    ItemDB::UpdateLocation(m_itemID, m_locationID, m_flag);

    // changes are cleared after sending, so make 2 sets to send to old owner and new owner
    if (notify) {
        std::map<int32, PyRep *> changes, changes2;
        if (new_flag != old_flag) {
            changes[Inv::Update::Flag] = new PyInt(old_flag);
            changes2[Inv::Update::Flag] = new PyInt(old_flag);
        }
        if (new_owner != old_owner) {
            changes[Inv::Update::Owner] = new PyInt(old_owner);
            changes2[Inv::Update::Owner] = new PyInt(old_owner);
        }
        if (new_location != old_location) {
            changes[Inv::Update::Location] = new PyInt(old_location);
            changes2[Inv::Update::Location] = new PyInt(old_location);
        }
        SendItemChange(m_ownerID, changes);
        if (new_owner != old_owner)
            SendItemChange(old_owner, changes2);
    }
}

void InventoryItem::Move(uint32 new_location, EVEItemFlags new_flag/*flagAutoFit*/, bool notify/*false*/) {
    if ((new_location == m_locationID) and (new_flag == m_flag))
        return; //nothing to do...

    InventoryItemRef iRef(nullptr);
    uint32 old_location = m_locationID;
    EVEItemFlags old_flag = m_flag;

    if (new_location != m_locationID) {
        // remove from current location
        if (IsValidLocation(m_locationID)) {
            iRef = sItemFactory.GetItem(m_locationID);
            if (iRef.get() != nullptr)
                iRef->RemoveItem(InventoryItemRef(this));
            else
                _log(INV__WARNING, "Move(): Location %u not found. Could not remove %s from it's inventory.", m_locationID, itemName().c_str());
        }
    }

    // update data
    m_flag = new_flag;
    m_locationID = new_location;

    if (old_location != m_locationID) {
        // add to new location
        if (IsValidLocation(m_locationID)) {
            iRef = sItemFactory.GetItem(m_locationID);
            if (iRef.get() != nullptr)
                iRef->AddItem(InventoryItemRef(this));
            else
                _log(INV__WARNING, "Move(): Location %u not found. Could not add %s to it's inventory.", m_locationID, itemName().c_str());
        }
    }

    if ((old_flag != new_flag) and is_log_enabled(INV__TRACE))
        _log(INV__TRACE, "InventoryItem::Move()  Updated flag on %s(%u) from %s to %s.", \
                itemName().c_str(), itemID(), sDataMgr.GetFlagName(old_flag), sDataMgr.GetFlagName(new_flag));

    if (IsTempItem(m_itemID))
        return;

    ItemDB::UpdateLocation(m_itemID, m_locationID, m_flag);

    //notify about the changes.
    if (notify) {
        std::map<int32, PyRep *> changes;
        if ( m_locationID != old_location )
            changes[Inv::Update::Location] = new PyInt(old_location);
        if ( m_flag != old_flag )
            changes[Inv::Update::Flag] = new PyInt(old_flag);
        SendItemChange( m_ownerID, changes);   //changes is consumed
    }
}

InventoryItemRef InventoryItem::Split(int32 qty, bool notify/*false*/) {
    if (qty < 1) {
        _log(ITEM__ERROR, "%s (%u): Asked to split into a chunk of %i", m_itemName.c_str(), m_itemID, qty);
        return InventoryItemRef(nullptr);
    }
    if (!AlterQuantity(-qty, notify)) {
        _log(ITEM__ERROR, "%s (%u): Failed to remove quantity %i during split.", m_itemName.c_str(), m_itemID, qty);
        return InventoryItemRef(nullptr);
    }

    ItemData idata(m_type.id(), m_ownerID, 0, flagAutoFit, qty);
    InventoryItemRef iRef = sItemFactory.SpawnItem(idata);
    if (iRef.get() == nullptr)
        return InventoryItemRef(nullptr);  // couldnt spawn new item...we'll get over it.

    iRef->Move(m_locationID, m_flag, true);
    return iRef;
}

bool InventoryItem::Merge(InventoryItemRef to_merge, int32 qty/*0*/, bool notify/*true*/) {
    if (to_merge.get() == nullptr)
        return false;

    if (m_singleton or to_merge->singleton())
        throw PyException( MakeCustomError("You cannot stack assembled items."));

    if (m_type.id() != to_merge->typeID()) {
        _log(ITEM__WARNING, "%s (%u): Asked to merge with %s (%u).", m_itemName.c_str(), m_itemID, to_merge->itemName().c_str(), to_merge->itemID());
        return false;
    }

    if (qty < 1)
        qty = to_merge->quantity();

    // AlterQuantity will delete items with qty < 1
    if (!to_merge->AlterQuantity(-qty, notify)) {
        _log(ITEM__ERROR, "%s (%u): Failed to remove quantity %u.", to_merge->itemName().c_str(), to_merge->itemID(), qty);
        if (IsCharacter(m_ownerID)) {
            Client* pClient = sEntityList.FindClientByCharID(m_ownerID);
            if (pClient != nullptr)
                pClient->SendErrorMsg("Internal Server Error.  Ref: ServerError 63138");
        }
        return false;
    }

    if (!AlterQuantity(qty, notify)) {
        _log(ITEM__ERROR, "%s (%u): Failed to add quantity %u.", m_itemName.c_str(), m_itemID, qty);
        if (IsCharacter(m_ownerID)) {
            Client* pClient = sEntityList.FindClientByCharID(m_ownerID);
            if (pClient != nullptr)
                pClient->SendErrorMsg("Internal Server Error.  Ref: ServerError 63238");
        }
        return false;
    }

    return true;
}

void InventoryItem::MergeTypesInCargo(ShipItem* pShip, EVEItemFlags flag/*flagAutoFit*/)
{
    // get existing type in cargo
    InventoryItemRef iRef = pShip->GetMyInventory()->GetByTypeFlag(m_type.id(), flag);
    if (iRef.get() == nullptr) {
        Move(pShip->itemID(), flag, true);
        return;
    }
    // fix for elusive error when using IB::Add() to remove loaded modules (charge trying to add to module item)
    if (iRef->typeID() != typeID()) {
        Move(pShip->itemID(), flag, true);
        return;
    }

    // if either item is assembled, just move item (merge will throw on assembled items)
    if (iRef->singleton() or m_singleton) {
        Move(pShip->itemID(), flag, true);
        return;
    }
    // here we 'merge' with stack already in cargo (this is a feature in Alasiya)
    //  if it dont work, just move item.
    if (!iRef->Merge(InventoryItemRef(this)))
        Move(pShip->itemID(), flag, true);
}

void InventoryItem::AlterChargeQuantity(int16 qty/*0*/, bool loaded/*true*/) {
    // update qty here and in attribMap
    pAttributeMap->AlterChargeQuantity(qty, loaded);
    m_quantity += qty;
}

bool InventoryItem::AlterQuantity(int32 qty, bool notify/*false*/) {
    if (qty == 0)
        return true;

    int32 new_qty = m_quantity + qty;
    if (new_qty < 0) {
        codelog(ITEM__ERROR, "%s (%u): Tried to remove %i from stack of %i for ownerID %u.", m_itemName.c_str(), m_itemID, qty, m_quantity, m_ownerID);
        // make player error msg here.....
        return false;
    }

    return SetQuantity(new_qty, notify);
}

bool InventoryItem::SetQuantity(int32 qty, bool notify/*false*/) {
    //if an object is singleton, there is only one, and it shouldn't be able to alter qty
    if (m_singleton) {
        _log(ITEM__ERROR, "%s (%u): Failed to set quantity %i; the items singleton bit is set", m_itemName.c_str(), m_itemID, qty);
        // make player error msg here.....
        return false;
    }
    int32 old_qty = m_quantity;
    m_quantity = qty;

    if (m_quantity > EVEMU_MAX_SHORT_ID) {
        codelog(ITEM__ERROR, "%s (%u): quantity overflow", m_itemName.c_str(), m_itemID);
        m_quantity = EVEMU_MAX_SHORT_ID -1;
        if (IsCharacter(m_ownerID)) {
            Client* pClient = sEntityList.FindClientByCharID(m_ownerID);
            if (pClient != nullptr)
                pClient->SendInfoModalMsg("Your %s has reached quantity limits of this server.<br>If you try to add any more to this stack, you will lose items.  This is your only warning.", itemName().c_str());
        }
    }

    if (notify) {
        std::map<int32, PyRep *> changes;
        if (IsModuleSlot(m_flag))
            changes[Inv::Update::Quantity] = new PyInt(old_qty);    // this one is to trigger ship module button fx
        else
            changes[Inv::Update::StackSize] = new PyInt(old_qty);
        SendItemChange(m_ownerID, changes); //changes is consumed
    }

    if (m_quantity < 1)
        Delete();
    else if (sConfig.world.saveOnUpdate)
        SaveItem();

    return true;
}

bool InventoryItem::SetFlag(EVEItemFlags flag, bool notify/*false*/) {
    if (m_flag == flag)
        return true;

    EVEItemFlags old_flag = m_flag;
    m_flag = flag;

    ItemDB::UpdateLocation(m_itemID, m_locationID, m_flag);

    if (notify) {
        std::map<int32, PyRep *> changes;
        changes[Inv::Update::Flag] = new PyInt(old_flag);
        SendItemChange(m_ownerID, changes); //changes is consumed
    }

    return true;
}

bool InventoryItem::ChangeSingleton(bool singleton, bool notify/*false*/) {
    if (singleton == m_singleton)
        return true;    //nothing to do...

    bool old_singleton = m_singleton;
    m_singleton = singleton;

    if (sConfig.world.saveOnUpdate)
        SaveItem();

    if (notify) {
        std::map<int32, PyRep *> changes;
        changes[Inv::Update::Singleton] = new PyInt(old_singleton);
        SendItemChange(m_ownerID, changes); //changes is consumed
    }

    // must update volume when singleton (packaged state) changes for (mostly) ship items.
    SetAttribute(AttrVolume, GetPackagedVolume(), notify);
    return true;
}

void InventoryItem::ChangeOwner(uint32 new_owner, bool notify/*false*/) {
    if (new_owner == m_ownerID)
        return; //nothing to do...

    uint32 old_owner = m_ownerID;
    m_ownerID = new_owner;

    if (sConfig.world.saveOnUpdate)
        SaveItem();

    //notify about the changes.
    if (notify) {
        std::map<int32, PyRep *> changes;
        //send the notify to the new owner.
        changes[Inv::Update::Owner] = new PyInt(old_owner);
        SendItemChange(new_owner, changes); //changes is consumed
        //also send the notify to the old owner.
        changes[Inv::Update::Owner] = new PyInt(old_owner);
        SendItemChange(old_owner, changes); //changes is consumed
    }
}

//contents of changes are consumed and cleared
void InventoryItem::SendItemChange(uint32 toID, std::map<int32, PyRep *> &changes) {
    if (IsNPCCorp(toID) or (toID == 1) or IsFaction(toID))   //IsValidOwner()
        return;
    if (sConsole.IsShutdown())
        return;
    NotifyOnItemChange change;
        change.itemRow = GetItemRow();
        change.changes = changes;
    PyTuple *tmp = change.Encode();

    if (is_log_enabled(ITEM__CHANGE)) {
        _log(ITEM__CHANGE, "Sending Item changes for %s(%u)", m_itemName.c_str(), m_itemID);
        tmp->Dump(ITEM__CHANGE, "    ");
    }

    //TODO: figure out the appropriate list of interested people...
    if (IsCharacter(toID)) {
        Client* pClient = sEntityList.FindClientByCharID(toID);
        if (pClient == nullptr)
            return;
        if (pClient->IsCharCreation())
            return;
        //if (IsShipItem()) //(pClient->IsBoard())
        //    pClient->SendNotification("OnItemsChanged", "charid", &tmp, false); //unsequenced.  <<--  this is called for multiple items
        //else
            pClient->SendNotification("OnItemChange", "clientID", &tmp, false); //unsequenced.  <<-- this is for single items
    } else if (IsPlayerCorp(toID)) {
        // there is more to this.  not sure what else or how yet.
        MulticastTarget mct;
        mct.corporations.insert(toID);
        if (IsStation(m_locationID)) {
            mct.locations.insert(m_locationID);
            sEntityList.Multicast("OnItemChange", "*stationid&corpid", &tmp, mct);
        } else {
            sEntityList.Multicast("OnItemChange", "corpid", &tmp, mct);
        }
    }
    PySafeDecRef(tmp);
    /** @todo change isnt being PyDecRef'd as it should here */
   // for (auto cur : changes)
    //    PySafeDecRef(cur.second);
}

void InventoryItem::SaveItem()
{
    ItemData data(m_itemName.c_str(),
                 m_type.id(),
                 m_ownerID,
                 m_locationID,
                 m_flag,
                 m_contraband,
                 m_singleton,
                 m_quantity,
                 m_position,
                 customInfo().c_str()
                );

    ItemDB::SaveItem(m_itemID, data);
    // item attributes are saved in ItemFactory.cpp:96  (save loop on shutdown for loaded items)
    // make call here for items saved after *some* change
    //SaveAttributes();
    pAttributeMap->Save();
}

void InventoryItem::UpdateLocation()
{
    ItemDB::UpdateLocation(m_itemID, m_locationID, m_flag);
}

PyPackedRow* InventoryItem::GetItemStatusRow() const {
    DBRowDescriptor* header = new DBRowDescriptor;
        header->AddColumn( "instanceID",    DBTYPE_I8);
        header->AddColumn( "online",        DBTYPE_BOOL);
        header->AddColumn( "damage",        DBTYPE_R8);
        header->AddColumn( "charge",        DBTYPE_R8);
        header->AddColumn( "skillPoints",   DBTYPE_I4);
        header->AddColumn( "armorDamage",   DBTYPE_R8);
        header->AddColumn( "shieldCharge",  DBTYPE_R8);
        header->AddColumn( "incapacitated", DBTYPE_BOOL);
    PyPackedRow* row = new PyPackedRow( header);
    GetItemStatusRow( row);
    return row;
}

void InventoryItem::GetItemStatusRow( PyPackedRow* into ) const {
    into->SetField( "instanceID",    new PyLong( m_itemID ));
    into->SetField( "online",        new PyBool( (HasAttribute(AttrOnline) ? GetAttribute(AttrOnline).get_bool() : false) ));
    into->SetField( "damage",        new PyFloat( (HasAttribute(AttrDamage) ? GetAttribute(AttrDamage).get_float() : 0) ));
    into->SetField( "charge",        new PyFloat( (HasAttribute(AttrCapacitorCharge) ? GetAttribute(AttrCapacitorCharge).get_float() : 0) ));
    into->SetField( "skillPoints",   new PyInt( (HasAttribute(AttrSkillPoints) ? GetAttribute(AttrSkillPoints).get_uint32() : 0) ));
    into->SetField( "armorDamage",   new PyFloat( (HasAttribute(AttrArmorDamageAmount) ? GetAttribute(AttrArmorDamageAmount).get_float() : 0.0) ));
    into->SetField( "shieldCharge",  new PyFloat( (HasAttribute(AttrShieldCharge) ? GetAttribute(AttrShieldCharge).get_float() : 0.0) ));
    into->SetField( "incapacitated", new PyBool( (HasAttribute(AttrIsIncapacitated) ? GetAttribute(AttrIsIncapacitated).get_bool() : false) ));
}

/*  this is charge info for the module in question  */
PyPackedRow* InventoryItem::GetChargeStatusRow(uint32 shipID) const {
    DBRowDescriptor* header = new DBRowDescriptor;
        header->AddColumn("instanceID", DBTYPE_I8);
        header->AddColumn("flagID",     DBTYPE_I2);
        header->AddColumn("typeID",     DBTYPE_I4);
        header->AddColumn("quantity",   DBTYPE_I4);
    PyPackedRow* row = new PyPackedRow( header);
    GetChargeStatusRow(shipID, row);
    return row;
}

void InventoryItem::GetChargeStatusRow(uint32 shipID, PyPackedRow* into) const {
    into->SetField("instanceID",     new PyLong(shipID));  /* this is shipID (locationID) */
    into->SetField("flagID",         new PyInt(m_flag));
    into->SetField("typeID",         new PyInt(m_type.id()));
    into->SetField("quantity",       pAttributeMap->GetAttribute(AttrQuantity).GetPyObject()); // AttrQuantity is used for loaded charges
}

PyPackedRow* InventoryItem::GetItemRow() const
{
    PyList *keywords = new PyList();
        keywords->AddItem(new_tuple(new PyString("stacksize"), new PyToken("util.StackSize")));
        keywords->AddItem(new_tuple(new PyString("singleton"), new PyToken("util.Singleton")));

    DBRowDescriptor* header = new DBRowDescriptor(keywords);
        header->AddColumn( "itemID",     DBTYPE_I8);
        header->AddColumn( "typeID",     DBTYPE_I4);
        header->AddColumn( "ownerID",    DBTYPE_I4);
        header->AddColumn( "locationID", DBTYPE_I4);
        header->AddColumn( "flagID",     DBTYPE_I2);
        header->AddColumn( "quantity",   DBTYPE_I4);
        header->AddColumn( "groupID",    DBTYPE_I2);
        header->AddColumn( "categoryID", DBTYPE_I4);
        header->AddColumn( "customInfo", DBTYPE_STR);

    PyPackedRow* row = new PyPackedRow( header);
    GetItemRow(row);

    return row;
}

void InventoryItem::GetItemRow( PyPackedRow* into ) const
{
    int32 qty = (m_singleton ? -1 : m_quantity);
    if (m_type.categoryID() == EVEDB::invCategories::Blueprint)
        if (sItemFactory.GetBlueprint(m_itemID)->copy())
            qty = -2;

    into->SetField( "itemID",       new PyLong( m_itemID ));
    into->SetField( "typeID",       new PyInt( m_type.id() ));
    into->SetField( "ownerID",      new PyInt( m_ownerID ));
    into->SetField( "locationID",   new PyInt( m_locationID ));
    into->SetField( "flagID",       new PyInt( m_flag ));
    into->SetField( "quantity",     new PyInt( qty ));
    into->SetField( "groupID",      new PyInt( type().groupID() ));
    into->SetField( "categoryID",   new PyInt( type().categoryID() ));
    into->SetField( "customInfo",   new PyString( m_customInfo ));
}

bool InventoryItem::Populate( Rsp_CommonGetInfo_Entry& result )
{
    /** @todo  this may need to be reworked once POS and Outposts are implemented. */

    //make sure trash data is removed from &result
    result.attributes.clear();
    PySafeDecRef(result.itemID);
    PySafeDecRef(result.invItem);
    result.time = GetFileTimeNow();
    result.invItem = GetItemRow();
    // this is for me, to display item name in logs.  not sure if client will react to it being here...nope
    result.description = m_itemName;

    // updated charge info...again  -allan 8Jan20
    if ((m_type.categoryID() == EVEDB::invCategories::Charge)
    and IsModuleSlot(m_flag)) {
        PyTuple* tuple = new PyTuple(3);
            tuple->SetItem(0, new PyInt(m_locationID));
            tuple->SetItem(1, new PyInt(m_flag));
            tuple->SetItem(2, new PyInt(m_type.id()));
        result.itemID = tuple;
        //result.invItem = PyStatic.NewNone();
        for (AttrMapItr itr = pAttributeMap->begin(), end = pAttributeMap->end(); itr != end; ++itr)
            result.attributes[(*itr).first] = (*itr).second.GetPyObject();
        return true;
    }

    result.itemID = new PyInt(m_itemID);

    if (m_type.categoryID() == EVEDB::invCategories::Skill) {
        result.attributes[AttrSkillTimeConstant] = GetAttribute(AttrSkillTimeConstant).GetPyObject();
        result.attributes[AttrSkillPoints] = GetAttribute(AttrSkillPoints).GetPyObject();
        result.attributes[AttrSkillLevel] = GetAttribute(AttrSkillLevel).GetPyObject();
        return true;
    }
    //} else if (m_type.id() == 51) { // for vouchers
    //    result.description = m_itemName;

    if (pAttributeMap->GetAttribute(AttrOnline).get_bool()) {
        EntityEffectState es;
            es.env_itemID = m_itemID;
            es.env_charID = m_ownerID;
            es.env_shipID = m_locationID;
            es.env_target = m_locationID;
            es.env_other = PyStatic.NewNone();
            es.env_area = PyStatic.NewNone();
            es.env_effectID = 16;
            /** @todo fix this once we start tracking effects */
            // on login, this is current time
            es.startTime = GetFileTimeNow() - EvE::Time::Minute; // m_timestamp
            es.duration = -1;
            es.repeat = 0;
            es.randomSeed = PyStatic.NewNone();
        result.activeEffects[es.env_effectID] = es.Encode();
    }

    for (AttrMapItr itr = pAttributeMap->begin(), end = pAttributeMap->end(); itr != end; ++itr) {
        //localization.GetByLabel('UI/Fitting/FittingWindow/WarpSpeed', distText=util.FmtDist(max(1.0, bws) * wsm * 3 * const.AU, 2))
        //if ((*itr).first == AttrWarpSpeedMultiplier)
        //    result.attributes[AttrWarpSpeedMultiplier] = new PyFloat((*itr).second.get_float() /3);
        //else
            result.attributes[(*itr).first] = (*itr).second.GetPyObject();
    }

    return true;
}

PyList* InventoryItem::GetItemInfo() const
{
    PyList* itemInfo = new PyList();
        itemInfo->AddItem(GetItemRow());
    return itemInfo;
}

PyObject* InventoryItem::ItemGetInfo()
{   // called from dogmaBound at least once (usually 2x) on every weapon update
    Rsp_ItemGetInfo result;
    if (!Populate(result.entry))
        return nullptr;
    return result.Encode();
}

void InventoryItem::SetCustomInfo(const char *ci) {
    if (ci != nullptr)
        m_customInfo = ci;
    else
        m_customInfo = "";

    if (sConfig.world.saveOnUpdate)
        SaveItem();
}

void InventoryItem::SetPosition(const GPoint pos)
{
    m_position = pos;
    _log(ITEM__RELOCATE, "%s(%u) Relocating to %.2f, %.2f, %.2f.", m_itemName.c_str(), m_itemID, m_position.x, m_position.y, m_position.z);
}

void InventoryItem::SetAttribute( uint16 attrID, float num, bool notify/*true*/)
{
    EvilNumber eNum(num);
    pAttributeMap->SetAttribute(attrID, eNum, notify);
}

void InventoryItem::SetAttribute( uint16 attrID, double num, bool notify/*true*/)
{
    EvilNumber eNum(num);
    pAttributeMap->SetAttribute(attrID, eNum, notify);
}

void InventoryItem::SetAttribute( uint16 attrID, EvilNumber num, bool notify/*true*/)
{
    pAttributeMap->SetAttribute(attrID, num, notify);
}

void InventoryItem::SetAttribute( uint16 attrID, int num, bool notify/*true*/)
{
    EvilNumber eNum(num);
    pAttributeMap->SetAttribute(attrID, eNum, notify);
}

void InventoryItem::SetAttribute( uint16 attrID, int64 num, bool notify/*true*/)
{
    EvilNumber eNum(num);
    pAttributeMap->SetAttribute(attrID, eNum, notify);
}

void InventoryItem::SetAttribute( uint16 attrID, uint32 num, bool notify/*true*/)
{
    EvilNumber eNum(num);
    pAttributeMap->SetAttribute(attrID, eNum, notify);
}

void InventoryItem::MultiplyAttribute(uint16 attrID, EvilNumber num, bool notify/*false*/)
{
    pAttributeMap->MultiplyAttribute(attrID, num, notify);
}

double InventoryItem::GetPackagedVolume()
{
    if (m_singleton)
        return m_type.volume();

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

    // return basic volume for non-ship or non-container objects
    return m_type.volume();
}

bool InventoryItem::SkillCheck(InventoryItemRef refItem)
{
    EvilNumber need = 0, has = 0;
    uint16 attr = 182, skill = 277;
    for (int8 i = 0; i < 3; ++i, ++attr, ++skill) {
        if (refItem->HasAttribute(attr, need) and HasAttribute(skill, has)) {
            if (need > has)
                return false;
        }
    }
    if (refItem->HasAttribute(1285, need) and HasAttribute(1286, has)) {
        if (need > has)
            return false;
    }

    attr = 1289; skill = 1287;
    for (int8 i = 0; i < 2; ++i, ++attr, ++skill) {
        if (refItem->HasAttribute(attr, need) and HasAttribute(skill, has)) {
            if (need > has)
                return false;
        }
    }
    // all skill requirement checks passed.
    return true;
}

// new effects system  -allan 4Feb17
void InventoryItem::AddModifier(fxData &data)
{
    m_modifiers.emplace(data.math, data);
}

void InventoryItem::RemoveModifier(fxData &data)
{
    switch (data.math) {
        case FX::Math::PreMul:         data.math = FX::Math::PreDiv;          break;
        case FX::Math::PreDiv:         data.math = FX::Math::PreMul;          break;
        case FX::Math::ModAdd:         data.math = FX::Math::ModSub;          break;
        case FX::Math::ModSub:         data.math = FX::Math::ModAdd;          break;
        case FX::Math::PostMul:        data.math = FX::Math::PostDiv;         break;
        case FX::Math::PostDiv:        data.math = FX::Math::PostMul;         break;
        case FX::Math::PostPercent:    data.math = FX::Math::RevPostPercent;  break;
        case FX::Math::PreAssignment:  data.math = FX::Math::PostAssignment;  break;
        case FX::Math::PostAssignment: data.math = FX::Math::PreAssignment;   break;
    }
    m_modifiers.emplace(data.math, data);
}

void InventoryItem::ClearModifiers()
{
    _log(EFFECTS__TRACE, "Clearing modifier map for %s", m_itemName.c_str());
    m_modifiers.clear();
}

void InventoryItem::ResetAttributes() {
    _log(EFFECTS__TRACE, "Resetting attrib map for %s", m_itemName.c_str());
    pAttributeMap->Load(true);
}
