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
    Author:     Bloody.Rabbit
*/

#include "eve-server.h"

#include "StaticDataMgr.h"
#include "character/Character.h"
#include "effects/EffectsDataMgr.h"
#include "inventory/ItemType.h"
#include "manufacturing/Blueprint.h"
#include "ship/Ship.h"
#include "station/Station.h"

/*
 * CategoryData
 */
CategoryData::CategoryData(const char *_name, const char *_desc, bool _published)
: name(_name),
  description(_desc),
  published(_published)
{
}

/*
 * ItemCategory
 */
ItemCategory::ItemCategory(EVEItemCategories _id, const CategoryData &_data)
: m_id(_id),
  m_name(_data.name),
  m_description(_data.description),
  m_published(_data.published)
{
    _log(ITEM__TRACE, "Created object %p for category %s (%u).", this, m_name.c_str(), (uint32)m_id);
}

ItemCategory* ItemCategory::Load(ItemFactory &factory, EVEItemCategories category)
{
    ItemCategory* c = ItemCategory::_Load(factory, category);

    // ItemCategory has no virtual _Load()
    return c;
}

ItemCategory* ItemCategory::_Load(ItemFactory &factory, EVEItemCategories category)
{
    CategoryData data;
    if(!factory.db().GetCategory(category, data))
        return nullptr;

    return ItemCategory::_Load(factory, category, data);
}

ItemCategory* ItemCategory::_Load(ItemFactory &factory, EVEItemCategories category, const CategoryData &data)
{
    return new ItemCategory(category, data);
}

/*
 * GroupData
 */
GroupData::GroupData(
    EVEItemCategories _category,
    const char *_name,
    const char *_desc,
    bool _useBasePrice,
    bool _allowManufacture,
    bool _allowRecycler,
    bool _anchored,
    bool _anchorable,
    bool _fittableNonSingleton,
    bool _published)
: category(_category),
  name(_name),
  description(_desc),
  useBasePrice(_useBasePrice),
  allowManufacture(_allowManufacture),
  allowRecycler(_allowRecycler),
  anchored(_anchored),
  anchorable(_anchorable),
  fittableNonSingleton(_fittableNonSingleton),
  published(_published)
{
}

/*
 * ItemGroup
 */
ItemGroup::ItemGroup(
    uint32 _id,
    // ItemGroup stuff:
    const ItemCategory &_category,
    const GroupData &_data)
: m_id(_id),
  m_category(&_category),
  m_name(_data.name),
  m_description(_data.description),
  m_useBasePrice(_data.useBasePrice),
  m_allowManufacture(_data.allowManufacture),
  m_allowRecycler(_data.allowRecycler),
  m_anchored(_data.anchored),
  m_anchorable(_data.anchorable),
  m_fittableNonSingleton(_data.fittableNonSingleton),
  m_published(_data.published)
{
    // assert for data consistency
    assert(_data.category == _category.id());

    _log(ITEM__TRACE, "Created object %p for group %s (%u).", this, name().c_str(), id());
}

ItemGroup* ItemGroup::Load(ItemFactory &factory, uint32 groupID)
{
    ItemGroup* g = ItemGroup::_Load(factory, groupID);

    // ItemGroup has no virtual _Load()
    return g;
}

ItemGroup* ItemGroup::_Load(ItemFactory &factory, uint32 groupID)
{
    // pull data
    GroupData data;
    if(!factory.db().GetGroup(groupID, data))
        return nullptr;

    // retrieve category
    const ItemCategory *c = factory.GetCategory(data.category);
    if(c == nullptr)
        return nullptr;

    return ItemGroup::_Load(factory, groupID, *c, data);
}

ItemGroup* ItemGroup::_Load(ItemFactory &factory, uint32 groupID, const ItemCategory &category, const GroupData &data)
{
    return new ItemGroup(groupID, category, data);
}

/*
 * TypeData
 */
TypeData::TypeData(
    uint32 _groupID,
    const char *_name,
    const char *_desc,
    double _radius,
    double _mass,
    double _volume,
    double _capacity,
    uint32 _portionSize,
    EVERace _race,
    double _basePrice,
    bool _published,
    uint32 _marketGroupID,
    double _chanceOfDuplicating)
: groupID(_groupID),
  name(_name),
  description(_desc),
  radius(_radius),
  mass(_mass),
  volume(_volume),
  capacity(_capacity),
  portionSize(_portionSize),
  race(_race),
  basePrice(_basePrice),
  published(_published),
  marketGroupID(_marketGroupID),
  chanceOfDuplicating(_chanceOfDuplicating)
{
}

/*
 * ItemType
 */
ItemType::ItemType(
    uint32 _id,
    const ItemGroup &_group,
    const TypeData &_data)
: m_id(_id),
  m_group(&_group),
  m_name(_data.name),
  m_description(_data.description),
  m_portionSize(_data.portionSize),
  m_basePrice(_data.basePrice),
  m_published(_data.published),
  m_marketGroupID(_data.marketGroupID),
  m_chanceOfDuplicating(_data.chanceOfDuplicating),
  // set some attributes
  m_radius(_data.radius),
  m_mass(_data.mass),
  m_volume(_data.volume),
  m_capacity(_data.capacity),
  m_raceID(_data.race)
{
    // assert for data consistency
    assert(_data.groupID == _group.id());
    m_AttributeMap.clear();

    _log(ITEM__TRACE, "Created ItemType object %p for type %s (%u).", this, name().c_str(), id());
}

ItemType* ItemType::Load(ItemFactory &factory, uint32 typeID)
{
    return ItemType::Load<ItemType>( factory, typeID );
}

template<class _Ty>
_Ty* ItemType::_LoadType(ItemFactory &factory, uint32 typeID,  const ItemGroup &group, const TypeData &data)
{
    switch( group.categoryID() ) {
        /** @todo  really need planets and moons here to load true radius' (from mapDenormalize)
        case EVEDB::invCategories::Celestial:
        case EVEDB::invCategories::Skill:
        case EVEDB::invCategories::_System:
        case EVEDB::invCategories::Material:
        case EVEDB::invCategories::Accessories:
        case EVEDB::invCategories::Module:
        case EVEDB::invCategories::Charge:
        case EVEDB::invCategories::Trading:
        case EVEDB::invCategories::Entity:
        case EVEDB::invCategories::Bonus:
        case EVEDB::invCategories::Commodity:
        case EVEDB::invCategories::Drone:
        case EVEDB::invCategories::Implant:
        case EVEDB::invCategories::Deployable:
        case EVEDB::invCategories::Structure:
        case EVEDB::invCategories::Reaction:
        case EVEDB::invCategories::Asteroid:
        case EVEDB::invCategories::Orbitals:
            */
        case EVEDB::invCategories::Owner: {
            return CharacterType::_LoadType<CharacterType>( factory, typeID, group, data );
        }
        case EVEDB::invCategories::Station: {
            return StationType::_LoadType<StationType>( factory, typeID, group, data );
        }
        case EVEDB::invCategories::Blueprint: {
            return BlueprintType::_LoadType<BlueprintType>( factory, typeID, group, data );
        }
        case EVEDB::invCategories::Ship: {
            return ShipType::_LoadType<ShipType>( factory, typeID, group, data );
        }
        default:
            _log(ITEM__MESSAGE, "type %u (group: %u, cat: %u) called _LoadType, but is not handled.", typeID, group.id(), group.categoryID());
             break;
    }

    // Generic one, create it:
    return new ItemType( typeID, group, data );
}

bool ItemType::_Load(ItemFactory &factory)
{
    // load type attribs
    std::vector< DmgTypeAttribute > typeAttrVec;
    sDataMgr.GetDgmTypeAttrVec(m_id, typeAttrVec);
    for (auto cur : typeAttrVec)
        m_AttributeMap.insert(std::pair<uint16, EvilNumber>(cur.attributeID, cur.value));

    // load attributes that are needed but NOT in default DgmTypeAttributes set (but found in invTypes)
    if (m_mass)
        m_AttributeMap.insert(std::pair<uint16, EvilNumber>(AttrMass, m_mass));
    if (m_radius)
        m_AttributeMap.insert(std::pair<uint16, EvilNumber>(AttrRadius, m_radius));
    if (m_volume)
        m_AttributeMap.insert(std::pair<uint16, EvilNumber>(AttrVolume, m_volume));
    if (m_capacity)
        m_AttributeMap.insert(std::pair<uint16, EvilNumber>(AttrCapacity, m_capacity));
    if (m_raceID)
        m_AttributeMap.insert(std::pair<uint16, EvilNumber>(AttrRaceID, m_raceID));

    // load required skills and levels into their own map, for later checks
    if (HasAttribute(AttrRequiredSkill1))
        m_reqSkillMap.insert(std::pair<uint16, uint8>((uint16)GetAttribute(AttrRequiredSkill1).get_int(), (uint8)GetAttribute(AttrRequiredSkill1Level).get_int()));
    if (HasAttribute(AttrRequiredSkill2))
        m_reqSkillMap.insert(std::pair<uint16, uint8>((uint16)GetAttribute(AttrRequiredSkill2).get_int(), (uint8)GetAttribute(AttrRequiredSkill2Level).get_int()));
    if (HasAttribute(AttrRequiredSkill3))
        m_reqSkillMap.insert(std::pair<uint16, uint8>((uint16)GetAttribute(AttrRequiredSkill3).get_int(), (uint8)GetAttribute(AttrRequiredSkill3Level).get_int()));
    if (HasAttribute(AttrRequiredSkill4))
        m_reqSkillMap.insert(std::pair<uint16, uint8>((uint16)GetAttribute(AttrRequiredSkill4).get_int(), (uint8)GetAttribute(AttrRequiredSkill4Level).get_int()));
    if (HasAttribute(AttrRequiredSkill5))
        m_reqSkillMap.insert(std::pair<uint16, uint8>((uint16)GetAttribute(AttrRequiredSkill5).get_int(), (uint8)GetAttribute(AttrRequiredSkill5Level).get_int()));
    if (HasAttribute(AttrRequiredSkill6))
        m_reqSkillMap.insert(std::pair<uint16, uint8>((uint16)GetAttribute(AttrRequiredSkill6).get_int(), (uint8)GetAttribute(AttrRequiredSkill6Level).get_int()));

    LoadEffects();

    return true;
}

const void ItemType::CopyAttributes(InventoryItem& itemRef) const
{
    // set attributes in the item's own attrMap.
    for (auto cur : m_AttributeMap)
        itemRef.SetAttribute(cur.first, cur.second, false);
}

const bool ItemType::HasAttribute(const uint16 attributeID) const
{
    AttrMapConstItr itr = m_AttributeMap.find(attributeID);
    if (itr != m_AttributeMap.end())
        return true;
    return false;
}

EvilNumber ItemType::GetAttribute(const uint16 attributeID) const
{
    AttrMapConstItr itr = m_AttributeMap.find(attributeID);
    if (itr != m_AttributeMap.end())
        return itr->second;
    return 0;
}

bool ItemType::HasReqSkill(const uint16 skillID, ItemFactory& m_factory) const
{
    std::map<uint16, uint8>::const_iterator itr = m_reqSkillMap.find(skillID);
    if (itr != m_reqSkillMap.end())
        return true;
    for (auto cur : m_reqSkillMap) {
        if (m_factory.GetType(cur.first)->HasReqSkill(skillID, m_factory))
            return true;
        else
            return false;
    }
    return false;
}

void ItemType::LoadEffects()
{
    std::vector< TypeEffects > typeEffMap;
    sFxDataMgr.GetTypeEffect(m_id, typeEffMap);

    for (auto cur : typeEffMap) {
        Effect mEffect = sFxDataMgr.GetEffect(cur.effectID);
        m_stateFxMap.insert(std::pair<int8, Effect>(mEffect.effectState, mEffect));
    }
}

bool ItemType::HasEffect(uint16 effectID) const
{
    std::unordered_multimap<int8, Effect>::const_iterator itr = m_stateFxMap.begin();
    for (; itr != m_stateFxMap.end(); itr++)
        if (itr->second.effectID == effectID)
            return true;
    return false;
}

void ItemType::GetEffectMap(const int8 state, std::map<uint16, Effect>& effectMap) const
{
    auto itr = m_stateFxMap.equal_range(state);
    for (auto it = itr.first; it != itr.second; it++)
        effectMap.insert(std::pair<uint16, Effect>(it->second.effectID, it->second));
}


/*
 * ItemData
 */
ItemData::ItemData(
    const char *_name,
    uint32 _typeID,
    uint32 _ownerID,
    uint32 _locationID,
    EVEItemFlags _flag,
    bool _contraband,
    bool _singleton,
    uint32 _quantity,
    const GPoint &_position,
    const char *_customInfo)
: name(_name),
typeID(_typeID),
ownerID(_ownerID),
locationID(_locationID),
flag(_flag),
contraband(_contraband),
singleton(_singleton),
quantity(_quantity),
position(_position),
customInfo(_customInfo)
{
}

ItemData::ItemData(
    uint32 _typeID,
    uint32 _ownerID,
    uint32 _locationID,
    EVEItemFlags _flag,
    uint32 _quantity,
    const char *_customInfo,
    bool _contraband)
: name(""),
typeID(_typeID),
ownerID(_ownerID),
locationID(_locationID),
flag(_flag),
contraband(_contraband),
singleton(false),
quantity(_quantity),
position(0, 0, 0),
customInfo(_customInfo)
{
}

ItemData::ItemData(
    uint32 _typeID,
    uint32 _ownerID,
    uint32 _locationID,
    EVEItemFlags _flag,
    const char *_name,
    const GPoint &_position,
    const char *_customInfo,
    bool _contraband)
: name(_name),
typeID(_typeID),
ownerID(_ownerID),
locationID(_locationID),
flag(_flag),
contraband(_contraband),
singleton(true),
quantity(1),
position(_position),
customInfo(_customInfo)
{
}


