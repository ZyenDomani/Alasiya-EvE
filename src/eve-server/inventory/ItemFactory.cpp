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
    Updates:    Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "character/Character.h"
#include "inventory/ItemFactory.h"
#include "manufacturing/Blueprint.h"
#include "pos/Structure.h"
#include "ship/Missile.h"
#include "ship/Ship.h"
#include "station/Station.h"
#include "system/Asteroid.h"
#include "system/Container.h"
#include "system/SolarSystem.h"
#include "system/SystemManager.h"

// Initialize ID Authority variables:
uint32 ItemFactory::m_nextTempID = EVEMU_TEMP_ENTITY_ID;
uint32 ItemFactory::m_nextAsteroidID = EVEMU_ASTEROID_ID;
uint32 ItemFactory::m_nextMissileID = EVEMU_MISSILE_ID;
uint32 ItemFactory::m_nextNPCID = EVEMU_NPC_ID;
uint32 ItemFactory::m_nextPlanetPinID = EVEMU_PLANET_PIN_ID;

ItemFactory::ItemFactory()
{
    m_itemCount = 0;
    m_pClient = nullptr;
}

ItemFactory::~ItemFactory() {
    // types
    for (auto cur : m_types)
        SafeDelete(cur.second);
    m_types.clear();
    // groups
    for (auto cur : m_groups)
        SafeDelete(cur.second);
    m_groups.clear();
    // categories
    for (auto cur : m_categories)
        SafeDelete(cur.second);
    m_categories.clear();
    // items
    /*
    for (auto cur : m_items)
        delete(cur.second.get());
    */
    // Set Client pointer to NULL
    m_pClient = nullptr;
}

void ItemFactory::SaveItems() {
    uint32 count = 0;
    double startTime = GetTimeMSeconds();
    std::vector<SaveData> items;
    items.clear();
    for (auto cur : m_items) {
        if (IsNotStaticItem(cur.first)) { // this is a hack for now.  will eventually move to static/dynamic item maps
            SaveData data;
                data.itemID = cur.first;
                data.contraband = cur.second->contraband();
                data.flag = cur.second->flag();
                data.locationID = cur.second->locationID();
                data.ownerID = cur.second->ownerID();
                data.position = cur.second->position();
                data.quantity = cur.second->quantity();
                data.singleton = cur.second->singleton();
                data.typeID = cur.second->typeID();
                data.customInfo = cur.second->customInfo();
            items.push_back(data);
            ++count;
            // attribMap has been updated to save relevant attributes.  this call is safe and desirable here
            cur.second->SaveAttributes();
        }
    }
    m_db.SaveItems(items);
    sLog.Warning("        SaveItems", "Saved %u Dynamic Items in %.3fms.", count, (GetTimeMSeconds() -startTime) );
}

Inventory *ItemFactory::GetInventoryFromId(uint32 itemID, bool load /*true*/) {
    InventoryItemRef item;
    std::map<uint32, InventoryItemRef>::iterator res = m_items.find( itemID );
    if (res != m_items.end()){
        item = res->second;
    } else {
        if (load)
            item = GetItem( itemID );
    }

    if (item)
        return item->GetInventory();

    return nullptr;
}

InventoryItemRef ItemFactory::GetInventoryItemFromID( uint32 itemID, bool load /*true*/) {
    InventoryItemRef item;
    std::map<uint32, InventoryItemRef>::iterator res = m_items.find( itemID );
    if (res != m_items.end()) {
        item = res->second;
    } else {
        if (load)
            item = GetItem( itemID );
    }

    return item;
}

InventoryItemRef ItemFactory::GetItemContainer(uint32 itemID, bool load /*true*/)
{
    InventoryItemRef item;
    std::map<uint32, InventoryItemRef>::iterator res = m_items.find( itemID );
    if (res != m_items.end()) {
        item = res->second;
        res = m_items.find( item->locationID() );
        item = res->second;
    } else {
        if (load) {
            item = GetItem( itemID );
            std::map<uint32, InventoryItemRef>::iterator res = m_items.find( itemID );
            if (res != m_items.end()) {
                item = res->second;
                res = m_items.find( item->locationID() );
                item = res->second;
            }
        }
    }

    return item;
}

Inventory* ItemFactory::GetItemContainerInventory(uint32 itemID, bool load)
{
    InventoryItemRef item;
    std::map<uint32, InventoryItemRef>::iterator res = m_items.find( itemID );
    if (res != m_items.end()) {
        item = res->second;
        res = m_items.find( item->locationID() );
        item = res->second;
    } else {
        if (load) {
            item = GetItem( itemID );
            std::map<uint32, InventoryItemRef>::iterator res = m_items.find( itemID );
            if (res != m_items.end()) {
                item = res->second;
                res = m_items.find( item->locationID() );
                item = res->second;
            }
        }
    }

    if (item)
        return item->GetInventory();

    return nullptr;
}

void ItemFactory::RemoveItem(uint32 itemID) {
    std::map<uint32, InventoryItemRef>::const_iterator res = m_items.find( itemID );
    if (res == m_items.end()) {
        _log(ITEM__WARNING, "ItemFactory::RemoveItem() - Item ID %u not found when requesting removal", itemID );
    } else {
        --m_itemCount;
        m_items.erase( res );
    }
}

const ItemCategory* ItemFactory::GetCategory(EVEItemCategories category) {
    std::map<EVEItemCategories, ItemCategory *>::iterator res = m_categories.find(category);
    if (res == m_categories.end()) {
        ItemCategory *cat = ItemCategory::Load(*this, category);
        if (!cat)
            return nullptr;

        // insert it into our cache
        res = m_categories.insert(
            std::make_pair(category, cat)
        ).first;
    }
    return res->second;
}

const ItemGroup* ItemFactory::GetGroup(uint32 groupID) {
    std::map<uint32, ItemGroup*>::iterator res = m_groups.find(groupID);
    if (res == m_groups.end()) {
        ItemGroup* group = ItemGroup::Load(*this, groupID);
        if (!group)
            return nullptr;

        // insert it into cache
        res = m_groups.insert(
            std::make_pair(groupID, group)
        ).first;
    }
    return res->second;
}

template<class _Ty>
const _Ty* ItemFactory::_GetType(uint32 typeID) {
    std::map<uint32, ItemType*>::iterator res = m_types.find(typeID);
    if (res == m_types.end()) {
        _Ty* type = _Ty::Load(*this, typeID);
        if (!type)
            return nullptr;

        // insert into cache
        res = m_types.insert(
            std::make_pair(typeID, type)
        ).first;
    }
    return static_cast<const _Ty *>(res->second);
}

const ItemType* ItemFactory::GetType(uint32 typeID) {
    return _GetType<ItemType>(typeID);
}

const BlueprintType* ItemFactory::GetBlueprintType(uint32 blueprintTypeID) {
    return _GetType<BlueprintType>(blueprintTypeID);
}

const CharacterType* ItemFactory::GetCharacterType(uint32 characterTypeID) {
    return _GetType<CharacterType>(characterTypeID);
}

const CharacterType* ItemFactory::GetCharacterTypeByBloodline(uint32 bloodlineID) {
    // Unfortunately, we have it indexed by typeID, so we must get it ...
    uint32 characterTypeID;
    if (!db().GetCharacterTypeByBloodline(bloodlineID, characterTypeID))
        return nullptr;
    return GetCharacterType(characterTypeID);
}

const ShipType* ItemFactory::GetShipType(uint32 shipTypeID) {
    return _GetType<ShipType>(shipTypeID);
}

const StationType* ItemFactory::GetStationType(uint32 stationTypeID) {
    return _GetType<StationType>(stationTypeID);
}

template<class _Ty>
RefPtr<_Ty> ItemFactory::_GetItem(uint32 itemID)
{
    std::map<uint32, InventoryItemRef>::iterator res = m_items.find( itemID );
    if (res == m_items.end())
    {
        // load the item
        RefPtr<_Ty> item = _Ty::Load( *this, itemID );
        if (!item)
            return RefPtr<_Ty>();

        //we keep the original ref.
        res = m_items.insert( std::make_pair( itemID, item ) ).first;
        ++m_itemCount;

    }
    // return to the user.
    return RefPtr<_Ty>::StaticCast( res->second );
}

InventoryItemRef ItemFactory::GetItem(uint32 itemID)
{
    return _GetItem<InventoryItem>( itemID );
}

BlueprintRef ItemFactory::GetBlueprint(uint32 blueprintID)
{
    return _GetItem<Blueprint>( blueprintID );
}

CharacterRef ItemFactory::GetCharacter(uint32 characterID)
{
    return _GetItem<Character>( characterID );
}

ShipItemRef ItemFactory::GetShip(uint32 shipID)
{
    return _GetItem<ShipItem>( shipID );
}

CelestialObjectRef ItemFactory::GetCelestialObject(uint32 celestialID)
{
    return _GetItem<CelestialObject>( celestialID );
}

SolarSystemRef ItemFactory::GetSolarSystem(uint32 solarSystemID)
{
    return _GetItem<SolarSystem>( solarSystemID );
}

StationItemRef ItemFactory::GetStation(uint32 stationID)
{
    return _GetItem<StationItem>( stationID );
}

SkillRef ItemFactory::GetSkill(uint32 skillID)
{
    return _GetItem<Skill>( skillID );
}
/*
AsteroidItemRef ItemFactory::GetAsteroid(uint32 asteroidID)
{
    return _GetItem<AsteroidItem>( asteroidID );
}
*/
StructureItemRef ItemFactory::GetStructure(uint32 structureID)
{
    return _GetItem<StructureItem>( structureID );
}

CargoContainerRef ItemFactory::GetCargoContainer(uint32 containerID)
{
    return _GetItem<CargoContainer>( containerID );
}

WreckContainerRef ItemFactory::GetWreckContainer(uint32 containerID)
{
    return _GetItem<WreckContainer>( containerID );
}

InventoryItemRef ItemFactory::SpawnItem(ItemData &data) {
    InventoryItemRef itemRef = InventoryItem::Spawn(*this, data);
    if ( !itemRef )
        return InventoryItemRef();

    // spawn successful; store the ref
    m_items.insert( std::make_pair( itemRef->itemID(), itemRef ) );
    ++m_itemCount;
    return itemRef;
}

BlueprintRef ItemFactory::SpawnBlueprint(ItemData &data, BlueprintData &bpData) {
    BlueprintRef itemRef = Blueprint::Spawn(*this, data, bpData);
    if ( !itemRef )
        return BlueprintRef();

    m_items.insert( std::make_pair( itemRef->itemID(), itemRef ) );
    ++m_itemCount;
    return itemRef;
}

CharacterRef ItemFactory::SpawnCharacter(ItemData &data, CharacterData &charData, CorpData &corpData) {
    CharacterRef itemRef = Character::Spawn(*this, data, charData, corpData);
    if ( !itemRef )
        return CharacterRef();

    //  do NOT add new char to item list to allow char to be selected and loaded normally after creation.
    //m_items.insert( std::make_pair( c->itemID(), c ) );
    //++m_itemCount;
    return itemRef;
}

ShipItemRef ItemFactory::SpawnShip(ItemData &data) {
    ShipItemRef itemRef = ShipItem::Spawn(*this, data);
    if ( !itemRef )
        return ShipItemRef();

    m_items.insert( std::make_pair( itemRef->itemID(), itemRef ) );
    ++m_itemCount;
    return itemRef;
}

SkillRef ItemFactory::SpawnSkill(ItemData &data)
{
    SkillRef itemRef = Skill::Spawn( *this, data );
    if ( !itemRef )
        return SkillRef();

    m_items.insert( std::make_pair( itemRef->itemID(), itemRef ) );
    ++m_itemCount;
    return itemRef;
}

StructureItemRef ItemFactory::SpawnStructure(ItemData &data)
{
    StructureItemRef itemRef = StructureItem::Spawn( *this, data );
    if ( !itemRef )
        return StructureItemRef();

    m_items.insert( std::make_pair( itemRef->itemID(), itemRef ) );
    ++m_itemCount;
    return itemRef;
}
/*
AsteroidItemRef ItemFactory::SpawnAsteroid(ItemData &idata, AsteroidData& adata)
{
    AsteroidItemRef itemRef = AsteroidItem::Spawn( *this, idata, adata );
    if ( !itemRef )
        return AsteroidItemRef();

    m_items.insert( std::make_pair( itemRef->itemID(), itemRef ) );
    ++m_itemCount;
    return itemRef;
}
*/
CargoContainerRef ItemFactory::SpawnCargoContainer(ItemData &data)
{
    CargoContainerRef itemRef = CargoContainer::Spawn( *this, data );
    if ( !itemRef )
        return CargoContainerRef();

    m_items.insert( std::make_pair( itemRef->itemID(), itemRef ) );
    ++m_itemCount;
    return itemRef;
}

WreckContainerRef ItemFactory::SpawnWreckContainer(ItemData &data)
{
    WreckContainerRef itemRef = WreckContainer::Spawn( *this, data );
    if ( !itemRef )
        return WreckContainerRef();

    m_items.insert( std::make_pair( itemRef->itemID(), itemRef ) );
    ++m_itemCount;
    return itemRef;
}

uint32 ItemFactory::GetNextTempID()
{
    if ( m_nextTempID < EVEMU_PLANET_PIN_ID )
		++m_nextTempID;
	else
        m_nextTempID = EVEMU_TEMP_ENTITY_ID;

	return m_nextTempID;
}

uint32 ItemFactory::GetNextAsteroidID()
{
    return ++m_nextAsteroidID;
}

uint32 ItemFactory::GetNextMissileID()
{
    return ++m_nextMissileID;
}

uint32 ItemFactory::GetNextNPCID()
{
    return ++m_nextNPCID;
}

uint32 ItemFactory::GetNextPlanetPinID()
{
    return ++m_nextPlanetPinID;
}

