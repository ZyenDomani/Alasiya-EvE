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
    Author:        Bloody.Rabbit
*/

/** @todo  load the bp material list from invTypeMaterials */

#include "eve-server.h"

#include "packets/Manufacturing.h"
#include "manufacturing/Blueprint.h"

/*
 * BlueprintType
 */
BlueprintType::BlueprintType(
    uint32 _id,
    const ItemGroup& _group,
    const TypeData& _data,
    const BlueprintType *_parentBlueprintType,
    const ItemType& _productType,
    const BlueprintTypeData& _bpData)
: ItemType(_id, _group, _data),
  m_parentBlueprintType(_parentBlueprintType),
  m_productType(_productType),
  m_productionTime(_bpData.productionTime),
  m_techLevel(_bpData.techLevel),
  m_researchProductivityTime(_bpData.researchProductivityTime),
  m_researchMaterialTime(_bpData.researchMaterialTime),
  m_researchCopyTime(_bpData.researchCopyTime),
  m_researchTechTime(_bpData.researchTechTime),
  m_productivityModifier(_bpData.productivityModifier),
  m_wasteFactor(_bpData.wasteFactor),
  m_chanceOfReverseEngineering(_bpData.chanceOfReverseEngineering),
  m_maxProductionLimit(_bpData.maxProductionLimit)
{   // asserts for data consistency
    assert(_bpData.productTypeID == _productType.id());
    if (_parentBlueprintType)
        assert(_bpData.parentBlueprintTypeID == _parentBlueprintType->id());
}

BlueprintType *BlueprintType::Load(ItemFactory& factory, uint32 typeID)
{
    return ItemType::Load<BlueprintType>( factory, typeID );
}

template<class _Ty>
_Ty *BlueprintType::_LoadBlueprintType(ItemFactory& factory, uint32 typeID,
    // ItemType stuff:
    const ItemGroup& group, const TypeData& data,
    // BlueprintType stuff:
    const BlueprintType *parentBlueprintType, const ItemType& productType, const BlueprintTypeData& bpData)
{
    return new BlueprintType(typeID, group, data, parentBlueprintType, productType, bpData );
}

/*
 * Blueprint
 */
Blueprint::Blueprint(
    ItemFactory& _factory,
    uint32 _blueprintID,
    // InventoryItem stuff:
    const BlueprintType& _bpType,
    const ItemData& _data,
    // Blueprint stuff:
    BlueprintData& _bpData)
: InventoryItem(_factory, _blueprintID, _bpType, _data)
{
    // data consistency asserts
    assert(_bpType.categoryID() == EVEDB::invCategories::Blueprint);
    m_copy   = _bpData.copy;
    m_runs   = _bpData.runs;
    m_mLevel = _bpData.mLevel;
    m_pLevel = _bpData.pLevel;
}

BlueprintRef Blueprint::Load(ItemFactory& factory, uint32 blueprintID)
{
    return InventoryItem::Load<Blueprint>( factory, blueprintID );
}

template<class _Ty>
RefPtr<_Ty> Blueprint::_LoadBlueprint(ItemFactory& factory, uint32 blueprintID,
    // InventoryItem stuff:
    const BlueprintType& bpType, const ItemData& data,
    // Blueprint stuff:
    BlueprintData& bpData)
{
    // we have enough data, construct the item
    return BlueprintRef( new Blueprint( factory, blueprintID, bpType, data, bpData ) );
}

BlueprintRef Blueprint::Spawn(ItemFactory& factory, ItemData& data, BlueprintData& bpData) {
    uint32 blueprintID = Blueprint::CreateItemID(factory, data, bpData);
    if (blueprintID == 0)
        return BlueprintRef();
    return Blueprint::Load(factory, blueprintID);
}

uint32 Blueprint::CreateItemID(ItemFactory& factory, ItemData& data, BlueprintData& bpData) {
    // make sure it's a blueprint type
    const BlueprintType *bt = factory.GetBlueprintType(data.typeID);
    if (!bt)
        return 0;

    // get the blueprintID
    uint32 blueprintID = InventoryItem::CreateItemID(factory, data);
    if (blueprintID == 0)
        return 0;

    // insert blueprint data into DB
    if (!factory.db().SaveBlueprintData(blueprintID, bpData)) {
        // delete item
        factory.db().DeleteItem(blueprintID);
        return 0;
    }

    return blueprintID;
}

void Blueprint::Delete() {
    // delete our blueprint data
    m_factory.db().DeleteBlueprint(m_itemID);
    // redirect to parent
    InventoryItem::Delete();
}

BlueprintRef Blueprint::SplitBlueprint(int32 qty_to_take, bool notify) {
    // split item
    BlueprintRef res = BlueprintRef::StaticCast( InventoryItem::Split( qty_to_take, notify ) );
    if ( !res )
        return BlueprintRef();

    // copy our attributes
    res->SetCopy(m_copy);
    res->SetME(m_mLevel);
    res->SetPE(m_pLevel);
    res->SetRuns(m_runs);
    res->SaveBlueprint();
    return res;
}

bool Blueprint::Merge(InventoryItemRef itemRef, uint32 qty, bool notify) {
    /** @todo  check for packaged, ME, PE, runs, etc before merge. */
    /*  singleton is checked and error thrown in InventoryItem::Merge()
    if (singleton() or itemRef->singleton())
        return false;
    */
    BlueprintRef bpRef = BlueprintRef::StaticCast(itemRef);
    if (m_mLevel != bpRef->materialLevel())
        return false;
    if (m_pLevel != bpRef->productivityLevel())
        return false;
    if (m_runs != bpRef->runsRemaining())
        return false;
    if ( !InventoryItem::Merge( itemRef, qty, notify ) )
        return false;
    return true;
}

void Blueprint::SaveBlueprint() {
    _log( MANUF__TRACE, "Saving blueprint %u.", itemID() );

    BlueprintData data;
        data.copy   = m_copy;
        data.runs   = m_runs;
        data.mLevel = m_mLevel;
        data.pLevel = m_pLevel;
    m_factory.db().SaveBlueprintData(itemID(), data);
}

PyDict* Blueprint::GetBlueprintAttributes() {
    Rsp_GetBlueprintAttributes rsp;
        rsp.blueprintID = itemID();
        rsp.copy = m_copy;
        rsp.productivityLevel = m_pLevel;
        rsp.materialLevel = m_mLevel;
        rsp.licensedProductionRunsRemaining = m_runs;
        rsp.wastageFactor = wasteFactor();
        rsp.productTypeID = type().productTypeID();
        rsp.manufacturingTime = type().productionTime();
        rsp.maxProductionLimit = type().maxProductionLimit();
        rsp.researchMaterialTime = type().researchMaterialTime();
        rsp.researchTechTime = type().researchTechTime();
        rsp.researchProductivityTime = type().researchProductivityTime();
        rsp.researchCopyTime = type().researchCopyTime();
    return rsp.Encode();
}
