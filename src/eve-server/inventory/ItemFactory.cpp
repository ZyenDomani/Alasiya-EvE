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
#include "EVEServerConfig.h"
#include "character/Character.h"
#include "inventory/InventoryDB.h"
#include "inventory/ItemFactory.h"
#include "inventory/ItemType.h"
#include "manufacturing/Blueprint.h"
#include "pos/Structure.h"
#include "ship/Missile.h"
#include "ship/Ship.h"
#include "station/Station.h"
#include "station/StationOffice.h"
#include "system/Asteroid.h"
#include "system/Container.h"
#include "system/SolarSystem.h"
#include "system/SystemManager.h"

ItemFactory::ItemFactory()
:m_pClient(nullptr),
// Initialize ID Authority variables:
m_nextTempID(EVEMU_TEMP_ENTITY_ID),
m_nextMissileID(EVEMU_MISSILE_ID),
m_nextNPCID(EVEMU_NPC_ID),
m_itemCount(0)
{
    m_db = new InventoryDB();
}

ItemFactory::~ItemFactory()
{
    SafeDelete(m_db);
}

int ItemFactory::Initialize()
{
    ManagerDB::DeleteSpawnedRats(); // takes ~31.2s to run on main, 0.005s on dev

    if (sConfig.debug.DeleteTrackingCans)
        InventoryDB::DeleteTrackingCans();

    sLog.Blue("      ItemFactory", "Item Factory Initialized.");
    return 1;
}

void ItemFactory::Close()
{
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
    m_items.clear();
    // Set Client pointer to NULL
    m_pClient = nullptr;
}

void ItemFactory::SaveItems() {
    if (sConfig.debug.DeleteTrackingCans)
        m_db->DeleteTrackingCans();
    uint32 count = 0;
    double startTime = GetTimeMSeconds();
    std::vector<SaveData> items;
    items.clear();
    for (auto cur : m_items) {
        if (cur.second->quantity() < 1)
            continue;
        if (IsAsteroid(cur.first))
            continue;
        if (IsCharacter(cur.first))
            continue;
        if (IsPlayerItem(cur.first)) { // this is a hack for now.  will eventually move to static/dynamic item maps
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
    m_db->SaveItems(items);
    sLog.Warning("        SaveItems", "Saved %u Dynamic Items in %.3fms.", count, (GetTimeMSeconds() -startTime) );
}

Inventory *ItemFactory::GetInventoryFromId(uint32 itemID, bool load /*true*/) {
    // do we need to check trade containers here?
    if (!IsValidLocation(itemID))
        return nullptr;
    InventoryItemRef iRef;
    std::map<uint32, InventoryItemRef>::iterator itr = m_items.find( itemID );
    if (itr != m_items.end()) {
        iRef = itr->second;
    } else {
        if (load)
            iRef = GetItem( itemID );
    }

    if (iRef.get() == nullptr)
        return nullptr;

    return iRef->GetMyInventory();
}

InventoryItemRef ItemFactory::GetInventoryItemFromID( uint32 itemID, bool load /*true*/) {
    InventoryItemRef iRef;
    std::map<uint32, InventoryItemRef>::iterator itr = m_items.find( itemID );
    if (itr != m_items.end()) {
        iRef = itr->second;
    } else {
        if (load)
            iRef = GetItem( itemID );
    }

    return iRef;
}

InventoryItemRef ItemFactory::GetItemContainer(uint32 itemID, bool load/*true*/)
{
    InventoryItemRef iRef;
    std::map<uint32, InventoryItemRef>::iterator itr = m_items.find( itemID );
    if (itr != m_items.end()) {
        iRef = itr->second;
        itr = m_items.find( iRef->locationID() );
        iRef = itr->second;
    } else {
        if (load) {
            iRef = GetItem( itemID );
            itr = m_items.find( itemID );
            if (itr != m_items.end()) {
                iRef = itr->second;
                itr = m_items.find( iRef->locationID() );
                iRef = itr->second;
            }
        }
    }

    if (iRef.get() == nullptr)
        return InventoryItemRef(nullptr);

    return iRef;
}

Inventory* ItemFactory::GetItemContainerInventory(uint32 itemID, bool load/*true*/)
{
    InventoryItemRef iRef;
    std::map<uint32, InventoryItemRef>::iterator itr = m_items.find( itemID );
    if (itr != m_items.end()) {
        iRef = itr->second;
        itr = m_items.find( iRef->locationID() );
        iRef = itr->second;
    } else {
        if (load) {
            iRef = GetItem( itemID );
            itr = m_items.find( itemID );
            if (itr != m_items.end()) {
                iRef = itr->second;
                itr = m_items.find( iRef->locationID() );
                iRef = itr->second;
            }
        }
    }

    if (iRef.get() == nullptr)
        return nullptr;

    return iRef->GetMyInventory();
}

void ItemFactory::RemoveItem(uint32 itemID) {
    std::map<uint32, InventoryItemRef>::iterator itr = m_items.find( itemID );
    if (itr == m_items.end()) {
        _log(ITEM__MESSAGE, "ItemFactory::RemoveItem() - Item ID %u not found when requesting removal", itemID );
    } else {
        --m_itemCount;
        m_items.erase( itr );
    }
}

const ItemCategory* ItemFactory::GetCategory(EVEItemCategories category) {
    std::map<EVEItemCategories, ItemCategory *>::iterator itr = m_categories.find(category);
    if (itr == m_categories.end()) {
        ItemCategory *cat = ItemCategory::Load(category);
        if (cat == nullptr)
            return nullptr;

        // insert it into our cache
        itr = m_categories.insert(
            std::make_pair(category, cat)
        ).first;
    }
    return itr->second;
}

const ItemGroup* ItemFactory::GetGroup(uint32 groupID) {
    std::map<uint32, ItemGroup*>::iterator itr = m_groups.find(groupID);
    if (itr == m_groups.end()) {
        ItemGroup* group = ItemGroup::Load(groupID);
        if (group == nullptr)
            return nullptr;

        // insert it into cache
        itr = m_groups.insert(
            std::make_pair(groupID, group)
        ).first;
    }
    return itr->second;
}

template<class _Ty>
const _Ty* ItemFactory::_GetType(uint32 typeID) {
    std::map<uint32, ItemType*>::iterator itr = m_types.find(typeID);
    if (itr == m_types.end()) {
        _Ty* type = _Ty::Load(typeID);
        if (type == nullptr)
            return nullptr;

        // insert into cache
        itr = m_types.insert(
            std::make_pair(typeID, type)
        ).first;
    }
    return static_cast<const _Ty *>(itr->second);
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
    if (!m_db->GetCharacterTypeByBloodline(bloodlineID, characterTypeID))
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
    std::map<uint32, InventoryItemRef>::iterator itr = m_items.find( itemID );
    if (itr == m_items.end())
    {
        if (IsTempItem(itemID)) {
            _log(ITEM__WARNING, "ItemFactory::_GetItem() called on tempItem %u", itemID);
            return RefPtr<_Ty>();
        }

        // load the item
        RefPtr<_Ty> item = _Ty::Load(itemID );
        if (!item)
            return RefPtr<_Ty>();

        //we keep the original ref.
        itr = m_items.insert( std::make_pair( itemID, item ) ).first;
        ++m_itemCount;

    }
    // return to the user.
    return RefPtr<_Ty>::StaticCast( itr->second );
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

AsteroidItemRef ItemFactory::GetAsteroid(uint32 asteroidID)
{
    return _GetItem<AsteroidItem>( asteroidID );
}

StationOfficeRef ItemFactory::GetOffice(uint32 officeID)
{
    return _GetItem<StationOffice>( officeID );
}

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
    InventoryItemRef iRef = InventoryItem::Spawn(data);
    if (iRef.get() == nullptr)
        return iRef;

    // spawn successful; store the ref
    m_items.insert( std::make_pair( iRef->itemID(), iRef ) );
    ++m_itemCount;
    return iRef;
}

InventoryItemRef ItemFactory::SpawnTempItem(ItemData &data) {
    InventoryItemRef iRef = InventoryItem::SpawnTemp(data);
    if (iRef.get() == nullptr)
        return iRef;

    // spawn successful; store the ref
    m_items.insert( std::make_pair( iRef->itemID(), iRef ) );
    ++m_itemCount;
    return iRef;
}

BlueprintRef ItemFactory::SpawnBlueprint(ItemData &data, BlueprintData &bpData) {
    BlueprintRef iRef = Blueprint::Spawn(data, bpData);
    if (iRef.get() == nullptr)
        return iRef;

    m_items.insert( std::make_pair( iRef->itemID(), iRef ) );
    ++m_itemCount;
    return iRef;
}

CharacterRef ItemFactory::SpawnCharacter(CharacterData &charData, CorpData &corpData) {
    CharacterRef iRef = Character::Spawn(charData, corpData);
    if (iRef.get() == nullptr)
        return iRef;

    return iRef;
}

ShipItemRef ItemFactory::SpawnShip(ItemData &data) {
    ShipItemRef iRef = ShipItem::Spawn(data);
    if (iRef.get() == nullptr)
        return iRef;

    m_items.insert( std::make_pair( iRef->itemID(), iRef ) );
    ++m_itemCount;
    return iRef;
}

SkillRef ItemFactory::SpawnSkill(ItemData &data)
{
    SkillRef iRef = Skill::Spawn( data );
    if (iRef.get() == nullptr)
        return iRef;

    m_items.insert( std::make_pair( iRef->itemID(), iRef ) );
    ++m_itemCount;
    return iRef;
}

StructureItemRef ItemFactory::SpawnStructure(ItemData &data)
{
    StructureItemRef iRef = StructureItem::Spawn( data );
    if (iRef.get() == nullptr)
        return iRef;

    m_items.insert( std::make_pair( iRef->itemID(), iRef ) );
    ++m_itemCount;
    return iRef;
}

AsteroidItemRef ItemFactory::SpawnAsteroid(ItemData &idata, AsteroidData& adata)
{
    AsteroidItemRef iRef = AsteroidItem::Spawn( idata, adata );
    if (iRef.get() == nullptr)
        return iRef;

    m_items.insert( std::make_pair( iRef->itemID(), iRef ) );
    ++m_itemCount;
    return iRef;
}

StationOfficeRef ItemFactory::SpawnOffice(ItemData &idata, OfficeData& odata)
{
    StationOfficeRef iRef = StationOffice::Spawn( idata, odata );
    if (iRef.get() == nullptr)
        return iRef;

    m_items.insert( std::make_pair( iRef->itemID(), iRef ) );
    ++m_itemCount;
    return iRef;
}

CargoContainerRef ItemFactory::SpawnCargoContainer(ItemData &data)
{
    CargoContainerRef iRef = CargoContainer::Spawn( data );
    if (iRef.get() == nullptr)
        return iRef;

    m_items.insert( std::make_pair( iRef->itemID(), iRef ) );
    ++m_itemCount;
    return iRef;
}

WreckContainerRef ItemFactory::SpawnWreckContainer(ItemData &data)
{
    WreckContainerRef iRef = WreckContainer::Spawn( data );
    if (iRef.get() == nullptr)
        return iRef;

    m_items.insert( std::make_pair( iRef->itemID(), iRef ) );
    ++m_itemCount;
    return iRef;
}

void ItemFactory::AddItem(InventoryItemRef iRef)
{
    m_items.emplace(iRef->itemID(), iRef);
}

uint32 ItemFactory::GetNextTempID()
{
    if ( m_nextTempID < EVEMU_PLANET_PIN_ID )
		++m_nextTempID;
	else
        m_nextTempID = EVEMU_TEMP_ENTITY_ID;

	return m_nextTempID;
}

uint32 ItemFactory::GetNextMissileID()
{
    return ++m_nextMissileID;
}

uint32 ItemFactory::GetNextNPCID()
{
    return ++m_nextNPCID;
}

