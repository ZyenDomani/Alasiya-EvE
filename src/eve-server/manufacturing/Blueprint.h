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
    Updates:    Allan
*/

#ifndef __BLUEPRINT_ITEM__H__INCL__
#define __BLUEPRINT_ITEM__H__INCL__

#include "EVEServerConfig.h"
#include "StaticDataMgr.h"
#include "inventory/ItemType.h"
#include "inventory/InventoryItem.h"
#include "manufacturing/FactoryDB.h"

/*
 * Class which contains blueprint type data.
 */
class BlueprintType
: public ItemType
{
    friend class ItemType;    // To let our parent redirect construction to our _Load().
public:
    static BlueprintType*   Load( uint32 typeID);

    /* Access functions  */
    const BlueprintType*    parentBlueprintType() const { return m_parentBlueprintType; }
    const ItemType&         productType()         const { return m_productType; }
    uint8                   techLevel()           const { return m_data.techLevel; }
    uint16                  wasteFactor()         const { return m_data.wasteFactor; }
    uint16                  productTypeID()       const { return productType().id(); }
    uint32                  productionTime()      const { return m_data.productionTime; }
    uint32                  researchCopyTime()    const { return m_data.researchCopyTime; }
    uint32                  researchTechTime()    const { return m_data.researchTechTime; }
    uint32                  materialModifier()    const { return m_data.materialModifier; }
    uint32                  maxProductionLimit()  const { return m_data.maxProductionLimit; }
    uint32                 researchMaterialTime() const { return m_data.researchMaterialTime; }
    uint32                 productivityModifier() const { return m_data.productivityModifier; }
    uint32             researchProductivityTime() const { return m_data.researchProductivityTime; }
    uint16                parentBlueprintTypeID() const { return (m_parentBlueprintType == nullptr ? 0 : parentBlueprintType()->id()); }
    float            chanceOfReverseEngineering() const { return m_data.chanceOfReverseEngineering; }

protected:
    BlueprintType(uint32 _id, const ItemGroup& _group, const TypeData& _data,
                  const BlueprintType *_parentBlueprintType, const ItemType& _productType,
                  const EvERam::bpTypeData& _tData);

    /*
     * Member functions
     */
    using ItemType::_Load;

    // Template loader:
    template<class _Ty>
    static _Ty *_LoadType( uint32 typeID, const ItemGroup& group, const TypeData& data)  {
        // check if we are really loading a blueprint
        if (group.categoryID() != EVEDB::invCategories::Blueprint ) {
            sLog.Error("Blueprint", "Load of blueprint type %u requested, but it's %s.", typeID, group.category().name().c_str() );
            return nullptr;
        }

        // pull additional blueprint data
        EvERam::bpTypeData tData = EvERam::bpTypeData();
        sDataMgr.GetBpTypeData(typeID, tData);

        // obtain parent blueprint type (might be NULL)
        const BlueprintType* parentBlueprintType(nullptr);
        if (tData.parentBlueprintTypeID) {
            parentBlueprintType = sItemFactory.GetBlueprintType( tData.parentBlueprintTypeID );
            if (parentBlueprintType == nullptr)
                return nullptr;
        }

        // obtain product type
        const ItemType* productType = sItemFactory.GetType( tData.productTypeID );
        if (productType == nullptr)
            return nullptr;

        return new BlueprintType(typeID, group, data, parentBlueprintType, *productType, tData );
    }

    /*
     * Data members
     */
    const BlueprintType *m_parentBlueprintType;
    const ItemType& m_productType;

    EvERam::bpTypeData m_data;
};


class Blueprint
: public InventoryItem
{
    friend class InventoryItem;    // to let it construct us
public:
    /* virtual functions default to base class and overridden as needed */
    virtual void            Delete();  //remove the item from the DB.
    // overload to merge the blueprints properly
    virtual bool            Merge(InventoryItemRef to_merge, uint32 qty=0, bool notify=true);
    // overload to split the blueprints properly
    virtual InventoryItemRef Split(int32 qty_to_take, bool notify=true)
                                                        { return SplitBlueprint( qty_to_take, notify ); }

    BlueprintRef            SplitBlueprint(int32 qty_to_take, bool notify=true);

    static BlueprintRef     Load( uint32 blueprintID);
    static BlueprintRef     Spawn( ItemData& data, EvERam::bpData& bdata);

    /*
     * Public fields:
     */
    const BlueprintType&    type()                const { return m_bpType; }
    const ItemType&         productType()         const { return m_bpType.productType(); }
    uint32                  productTypeID()       const { return m_bpType.productTypeID(); }
    bool                    copy()                      { return m_data.copy; }
    int32                   materialLevel()             { return m_data.mLevel; }
    int32                   productivityLevel()         { return m_data.pLevel; }
    int32                   runsRemaining()             { return m_data.runs; }

    // some blueprint-related stuff
    void                    UpdateME(int32 change)      { m_data.mLevel += change;}
    void                    UpdatePE(int32 change)      { m_data.pLevel += change;}
    void                    UpdateRuns(int32 change)    { m_data.runs += change;}

    void                    SetME(int32 me)             { m_data.mLevel = me; }
    void                    SetPE(int32 pe)             { m_data.pLevel = pe; }
    void                    SetCopy(bool copy)          { m_data.copy = copy; }
    void                    SetRuns(int32 runs)         { m_data.runs = runs; }

    float                   GetPE()                     { return m_data.pLevel / (1 + m_data.pLevel); }
    float                   GetME();

    bool                    infinite()                  { return (m_data.runs < 0); }

    /*
     * Primary public packet builders:
     */
    PyDict*                 GetBlueprintAttributes();


protected:
    Blueprint(  uint32 _blueprintID, const BlueprintType& _bpType, const ItemData& _data, EvERam::bpData& _bpData);

    /*
     * Member functions
     */
    using InventoryItem::_Load;

    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem( uint32 blueprintID, const ItemType& type, const ItemData& data)
    {
        if (type.categoryID() != EVEDB::invCategories::Blueprint )
        {
            sLog.Error("Blueprint", "Trying to load %s(%u) as Blueprint.", type.category().name().c_str(), blueprintID);
            if (sConfig.server.StackTrace)
                EvE::traceStack();
            return RefPtr<_Ty>();
        }
        const BlueprintType& bpType = static_cast<const BlueprintType& >( type );

        EvERam::bpData bdata = EvERam::bpData();
        if (!FactoryDB::GetBlueprint( blueprintID, bdata ) )
            return RefPtr<_Ty>();

        return BlueprintRef( new Blueprint( blueprintID, bpType, data, bdata ) );
    }

    void                    SaveBlueprint();
    static uint32           CreateItemID( ItemData& data, EvERam::bpData& bdata);

private:
    const BlueprintType &m_bpType;
    EvERam::bpData m_data;

};

#endif /* !__BLUEPRINT_ITEM__H__INCL__ */

