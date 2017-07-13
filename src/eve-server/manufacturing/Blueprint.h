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
    static BlueprintType*   Load(ItemFactory& factory, uint32 typeID);

    /* Access functions  */
    const BlueprintType*    parentBlueprintType() const { return m_parentBlueprintType; }
    const ItemType&         productType()         const { return m_productType; }
    uint8                   techLevel()           const { return m_techLevel; }
    uint32                  productTypeID()       const { return productType().id(); }
    uint32                  productionTime()      const { return m_productionTime; }
    uint32                  researchCopyTime()    const { return m_researchCopyTime; }
    uint32                  researchTechTime()    const { return m_researchTechTime; }
    uint32                  materialModifier()    const { return m_materialModifier; }
    uint32                  maxProductionLimit()  const { return m_maxProductionLimit; }
    uint32                 researchMaterialTime() const { return m_researchMaterialTime; }
    uint32                 productivityModifier() const { return m_productivityModifier; }
    uint32             researchProductivityTime() const { return m_researchProductivityTime; }
    uint32                parentBlueprintTypeID() const { return (m_parentBlueprintType ? 0 : parentBlueprintType()->id()); }
    double                  wasteFactor()         const { return m_wasteFactor; }
    double           chanceOfReverseEngineering() const { return m_chanceOfReverseEngineering; }

protected:
    BlueprintType(uint32 _id, const ItemGroup& _group, const TypeData& _data, const BlueprintType *_parentBlueprintType, const ItemType& _productType, const BlueprintTypeData& _bpData);

    /*
     * Member functions
     */
    using ItemType::_Load;

    // Template loader:
    template<class _Ty>
    static _Ty *_LoadType(ItemFactory& factory, uint32 typeID, const ItemGroup& group, const TypeData& data)  {
        // check if we are really loading a blueprint
        if (group.categoryID() != EVEDB::invCategories::Blueprint ) {
            sLog.Error("Blueprint", "Load of blueprint type %u requested, but it's %s.", typeID, group.category().name().c_str() );
            return nullptr;
        }

        // pull additional blueprint data
        BlueprintTypeData bpData;
        sDataMgr.GetBpTypeData(typeID, bpData);

        // obtain parent blueprint type (might be NULL)
        const BlueprintType* parentBlueprintType(nullptr);
        if (bpData.parentBlueprintTypeID) {
            parentBlueprintType = factory.GetBlueprintType( bpData.parentBlueprintTypeID );
            if (!parentBlueprintType)
                return nullptr;
        }

        // obtain product type
        const ItemType* productType = factory.GetType( bpData.productTypeID );
        if (!productType)
            return nullptr;

        // create blueprint type
        return _Ty::template _LoadBlueprintType<_Ty>( factory, typeID, group, data, parentBlueprintType, *productType, bpData );
    }

    // Actual loading stuff:
    template<class _Ty>
    static _Ty *_LoadBlueprintType(ItemFactory& factory, uint32 typeID, const ItemGroup& group, const TypeData& data,
        const BlueprintType *parentBlueprintType, const ItemType& productType, const BlueprintTypeData& bpData)
    {
        return new BlueprintType(typeID, group, data, parentBlueprintType, productType, bpData );
    }

    /*
     * Data members
     */
    const BlueprintType *m_parentBlueprintType;
    const ItemType& m_productType;

    uint8 m_techLevel;
    uint32 m_productionTime;
    uint32 m_researchProductivityTime;
    uint32 m_researchMaterialTime;
    uint32 m_researchCopyTime;
    uint32 m_researchTechTime;
    uint32 m_productivityModifier;
    uint32 m_materialModifier;
    uint32 m_maxProductionLimit;
    double m_wasteFactor;
    double m_chanceOfReverseEngineering;
};


class Blueprint
: public InventoryItem
{
    friend class InventoryItem;    // to let it construct us
public:
    /* virtual functions default to base class and overridden as needed */
    virtual void            Delete();  //remove the item from the DB.
    virtual bool            Merge(InventoryItemRef to_merge, uint32 qty=0, bool notify=true);
    // overload to split the blueprints properly
    virtual InventoryItemRef Split(int32 qty_to_take, bool notify=true) { return SplitBlueprint( qty_to_take, notify ); }
    BlueprintRef            SplitBlueprint(int32 qty_to_take, bool notify);

    static BlueprintRef     Load(ItemFactory& factory, uint32 blueprintID);
    static BlueprintRef     Spawn(ItemFactory& factory, ItemData& data, BlueprintData& bpData);

    /*
     * Public fields:
     */
    const BlueprintType&    type()                const { return static_cast<const BlueprintType& >(InventoryItem::type()); }
    const ItemType&         productType()         const { return type().productType(); }
    uint32                  productTypeID()       const { return type().productTypeID(); }
    bool                    copy()                      { return m_copy; }
    int32                   materialLevel()             { return m_mLevel; }
    int32                   productivityLevel()         { return m_pLevel; }
    int32                   runsRemaining()             { return m_runs; }

    // some blueprint-related stuff
    void                    UpdateME(int32 change)      { m_mLevel += change;}
    void                    UpdatePE(int32 change)      { m_pLevel += change;}
    void                    UpdateRuns(int32 change)    { m_runs += change;}

    bool                    infinite()                  { return ((m_runs < 0) ? true : false); }
    double                  wasteFactor()         const { return (type().wasteFactor() / (1 + m_mLevel)); }
    double                  materialMultiplier()        { return (1.0 + wasteFactor()); }
    double                  timeSaved()           const { return (1.0 - (1.0 / (1 + m_pLevel))) * type().productivityModifier(); }
    double                  timeMultiplier()      const { return (1.0 - (timeSaved() / type().productionTime())); }

    void                    SetCopy(bool copy)          { m_copy = copy; }
    void                    SetME(uint32 me)            { m_mLevel = me; }
    void                    SetPE(int32 pe)             { m_pLevel = pe; }
    void                    SetRuns(int32 runs)         { m_runs = runs; }

    /*
     * Primary public packet builders:
     */
    PyDict*                 GetBlueprintAttributes();

private:
    FactoryDB m_db;

protected:
    Blueprint(ItemFactory& _factory, uint32 _blueprintID, const BlueprintType& _bpType, const ItemData& _data, BlueprintData& _bpData);

    /*
     * Member functions
     */
    using InventoryItem::_Load;

    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem(ItemFactory& factory, uint32 blueprintID, const ItemType& type, const ItemData& data)
    {
        if (type.categoryID() != EVEDB::invCategories::Blueprint )
        {
            sLog.Error("Blueprint", "Trying to load %s as Blueprint.", type.category().name().c_str() );
            return RefPtr<_Ty>();
        }
        const BlueprintType& bpType = static_cast<const BlueprintType& >( type );

        FactoryDB mdb;
        BlueprintData bpData;
        if (!mdb.GetBlueprint( blueprintID, bpData ) )
            return RefPtr<_Ty>();

        return _Ty::template _LoadBlueprint<_Ty>( factory, blueprintID, bpType, data, bpData );
    }

    // Actual loading stuff:
    template<class _Ty>
    static RefPtr<_Ty> _LoadBlueprint(ItemFactory& factory, uint32 blueprintID, const BlueprintType& bpType, const ItemData& data, BlueprintData& bpData)
    {
        return BlueprintRef( new Blueprint( factory, blueprintID, bpType, data, bpData ) );
    }

    void                    SaveBlueprint();
    static uint32           CreateItemID(ItemFactory& factory, ItemData& data, BlueprintData& bpData);

private:
    bool      m_copy;

    int32     m_mLevel;
    int32     m_pLevel;
    int32     m_runs;
};

#endif /* !__BLUEPRINT_ITEM__H__INCL__ */

