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
#include "system/Container.h"
#include "system/SolarSystem.h"
#include "system/SystemManager.h"

// Initialize ID Authority variables:
uint32 ItemFactory::m_nextEntityID = EVEMU_TEMP_ENTITY_ID;
uint32 ItemFactory::m_nextAsteroidID = EVEMU_ASTEROID_ID;
uint32 ItemFactory::m_nextMissileID = EVEMU_MISSILE_ID;

ItemFactory::ItemFactory(EntityList& el)
: entity_list(el)
{
    m_itemCount = 0;
    m_pClient = nullptr;
}

ItemFactory::~ItemFactory() {
    // items
    SaveItems();
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

    // Set Client pointer to NULL
    m_pClient = nullptr;

    /* close db service */
    sDatabase.Close();
}

void ItemFactory::SaveItems() {
    double startTime = GetTimeMSeconds();
    float total_item_count = (float)m_items.size(), items_saved = 0.0f;
    float current_percent_items_saved = 0.0f;
    for (auto cur : m_items) {
        // save attributes of item
        if (cur.second->itemID() >= EVEMU_MINIMUM_ENTITY_ID)
            cur.second->SaveItem();

        ++items_saved;
        if( (items_saved / total_item_count) > (current_percent_items_saved + 0.1) ) {
            current_percent_items_saved = items_saved / total_item_count;
            sLog.Warning( "     Saving Items", " %3.2f%%", (current_percent_items_saved * 100) );
        }
    }
    sLog.Warning("        SaveItems", "Saved %u of %u Loaded Items in %.2f seconds.", \
                        (uint32)items_saved, (uint32)total_item_count, (GetTimeMSeconds() -startTime) );
}

Inventory *ItemFactory::GetInventoryFromId(uint32 itemID, bool load /*true*/) {
    InventoryItemRef item;
    std::map<uint32, InventoryItemRef>::iterator res = m_items.find( itemID );
    if (res != m_items.end())
        item = res->second;
    else {
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
    if (res != m_items.end())
        item = res->second;
    else {
        if (load)
            item = GetItem( itemID );
    }

    if (item)
        return item;

    return InventoryItemRef();

}

InventoryItemRef ItemFactory::GetItemContainer(uint32 itemID, bool load /*true*/) {

}

void ItemFactory::RemoveItem(uint32 itemID) {
    std::map<uint32, InventoryItemRef>::iterator res = m_items.find( itemID );
    if (res == m_items.end())
        _log(ITEM__WARNING, "ItemFactory::_DeleteItem() - Item ID %u not found when requesting deletion", itemID );
    else {
        --m_itemCount;
        m_items.erase( res );
    }
}


const ItemCategory* ItemFactory::GetCategory(EVEItemCategories category) {
    std::map<EVEItemCategories, ItemCategory *>::iterator res = m_categories.find(category);
    if (res == m_categories.end()) {
        ItemCategory *cat = ItemCategory::Load(*this, category);
        if (cat == NULL)
            return NULL;

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
            return NULL;

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
            return NULL;

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
        return NULL;
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
    InventoryItemRef i = InventoryItem::Spawn(*this, data);
    if ( !i )
        return InventoryItemRef();

    // spawn successful; store the ref
    m_items.insert( std::make_pair( i->itemID(), i ) );
    ++m_itemCount;
    return i;
}

BlueprintRef ItemFactory::SpawnBlueprint(ItemData &data, BlueprintData &bpData) {
    BlueprintRef bi = Blueprint::Spawn(*this, data, bpData);
    if ( !bi )
        return BlueprintRef();

    m_items.insert( std::make_pair( bi->itemID(), bi ) );
    ++m_itemCount;
    return bi;
}

CharacterRef ItemFactory::SpawnCharacter(ItemData &data, CharacterData &charData, CorpMemberInfo &corpData) {
    CharacterRef c = Character::Spawn(*this, data, charData, corpData);
    if ( !c )
        return CharacterRef();

    //  do NOT add new char to item list to allow char to be selected and loaded normally after creation.
    //m_items.insert( std::make_pair( c->itemID(), c ) );
    //++m_itemCount;
    return c;
}

ShipItemRef ItemFactory::SpawnShip(ItemData &data) {
    ShipItemRef s = ShipItem::Spawn(*this, data);
    if ( !s )
        return ShipItemRef();

    m_items.insert( std::make_pair( s->itemID(), s ) );
    ++m_itemCount;
    return s;
}

SkillRef ItemFactory::SpawnSkill(ItemData &data)
{
    SkillRef s = Skill::Spawn( *this, data );
    if ( !s )
        return SkillRef();

    m_items.insert( std::make_pair( s->itemID(), s ) );
    ++m_itemCount;
    return s;
}

StructureItemRef ItemFactory::SpawnStructure(ItemData &data)
{
    StructureItemRef o = StructureItem::Spawn( *this, data );
    if ( !o )
        return StructureItemRef();

    m_items.insert( std::make_pair( o->itemID(), o ) );
    ++m_itemCount;
    return o;
}

CargoContainerRef ItemFactory::SpawnCargoContainer(ItemData &data)
{
    CargoContainerRef o = CargoContainer::Spawn( *this, data );
    if ( !o )
        return CargoContainerRef();

    m_items.insert( std::make_pair( o->itemID(), o ) );
    ++m_itemCount;
    return o;
}

WreckContainerRef ItemFactory::SpawnWreckContainer(ItemData &data)
{
    WreckContainerRef o = WreckContainer::Spawn( *this, data );
    if ( !o )
        return WreckContainerRef();

    m_items.insert( std::make_pair( o->itemID(), o ) );
    ++m_itemCount;
    return o;
}

uint32 ItemFactory::GetNextEntityID()
{
	// This algorithm should be improved to search for reusable IDs that are no longer used,
	// but for now, just implement a simple wrap-around method once IDs have reached the maximum value:
	if ( m_nextEntityID < EVEMU_MAXIMUM_ENTITY_ID )
		++m_nextEntityID;
	else
		m_nextEntityID = EVEMU_MINIMUM_ENTITY_ID;

	return m_nextEntityID;
}

uint32 ItemFactory::GetNextAsteroidID()
{
    return ++m_nextAsteroidID;
}

uint32 ItemFactory::GetNextMissileID()
{
    return ++m_nextMissileID;
}
