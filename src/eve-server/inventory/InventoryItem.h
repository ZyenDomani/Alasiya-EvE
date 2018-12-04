/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2011 The EVEmu Team
 *    For the latest information visit http://evemu.org
 *    ------------------------------------------------------------------------------------
 *    This program is free software; you can redistribute it and/or modify it under
 *    the terms of the GNU Lesser General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option) any later
 *    version.
 *
 *    This program is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License along with
 *    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 *    http://www.gnu.org/copyleft/lesser.txt.
 *    ------------------------------------------------------------------------------------
 *    Author:     Zhur
 *    Updates:    Allan
 */
#ifndef EVE_INVENTORY_ITEM_H
#define EVE_INVENTORY_ITEM_H


#include "POD_containers.h"
#include "inventory/Inventory.h"
#include "inventory/InventoryDB.h"
#include "inventory/ItemType.h"
#include "inventory/ItemFactory.h"
#include "inventory/AttributeMap.h"

class PyRep;
class PyDict;
class PyObject;
class ShipItem;
class ServiceDB;
class ItemContainer;
class Rsp_CommonGetInfo_Entry;

/*
 * NOTE:
 * this object system should somehow be merged with the SystemEntity stuff
 * and class hierarchy built from it (Client, NPC, etc..) in the system manager...
 * however, the creation and destruction time logic is why it has not been done.
 *
 *  NOTE:
 * this system cannot and should not be merged with SE class, as not all Items are SE.
 * doing so would cause unnecessiary SEs to be created for items that are NOT SE.
 * keeping this as a seperate class is cleaner and better for creation and destruction
 * for both base and derived classes.       -allan 23.2.16
 */

/*
 * Class which maintains generic Inventory item.
 */
class InventoryItem
: public RefObject
{
public:
    InventoryItem(uint32 _itemID, const ItemType &_type, const ItemData &_data);
    virtual ~InventoryItem() noexcept;

    /* begin rewrite and update */
    /** @todo  derive and execute tests to determing if these are needed... */
    // copy c'tor
    InventoryItem(const InventoryItem& oth);
    // move c'tor
    InventoryItem(InventoryItem&& oth) noexcept;
    // assignment op
    //InventoryItem& operator= (const InventoryItem& oth);
    // move op
    InventoryItem& operator= (InventoryItem&& oth) noexcept;


    /* class type pointer querys. */
    virtual ShipItem* GetShipItem()                     { return nullptr; }
    /* class type tests. */
    virtual bool IsShipItem()                           { return false; }

    /* generic access functions handled here */
    Inventory*              GetMyInventory()            { return pInventory; }

    /* common functions for all entities handled here */
    /* public data queries  */
    bool                    contraband() const          { return m_contraband; }
    bool                    singleton() const           { return m_singleton; }
    int32                   quantity() const            { return m_quantity; }
    uint32                  itemID() const              { return m_itemID; }
    uint32                  ownerID() const             { return m_ownerID; }
    uint32                  locationID() const          { return m_locationID; }
    EVEItemFlags            flag() const                { return m_flag; }
    const GPoint &          position() const            { return m_position; }
    const ItemType &        type() const                { return m_type; }
    const std::string &     itemName() const            { return m_itemName; }
    const std::string &     customInfo() const          { return m_customInfo; }

    /* public type queries  */
    uint16                  typeID() const              { return m_type.id(); }
    uint16                  groupID() const             { return m_type.groupID(); }
    double                  radius() const              { return (HasAttribute(AttrRadius) ? GetAttribute(AttrRadius).get_double() : 1.0); }
    const ItemGroup &       group() const               { return m_type.group(); }
    const ItemCategory &    category() const            { return m_type.category(); }
    EVEItemCategories       categoryID() const          { return m_type.categoryID(); }
    bool                    isGlobal() const            { return (HasAttribute(AttrIsGlobal) ? true : false); }

    /* public-access generic functions handled in base class. */
    void                    Rename(std::string name);
    void                    Relocate(const GPoint pos);
    void                    SetCustomInfo(const char *ci);
    void                    ChangeOwner(uint32 new_owner, bool notify=false);
    // Move() will remove item from old location, add to new location and (optionally) notify client of changes
    void                    Move(uint32 new_location, EVEItemFlags flag=flagAutoFit, bool notify=false);
    // same as Move() but xfer ownership also
    void                    Donate(uint32 new_owner, uint32 new_location, EVEItemFlags new_flag, bool notify=true);
    void                    SendItemChange(uint32 toID, std::map<int32, PyRep *> &changes) const;
    // this is for stacking unloading charges in ships cargo
    void                    MergeTypesInCargo(ShipItem* pShip);
    bool                    ChangeSingleton(bool singleton, bool notify=false);
    bool                    AlterQuantity(int32 qty_change, bool notify=false);
    bool                    SetQuantity(int32 qty_new, bool notify=false);
    bool                    SetFlag(EVEItemFlags new_flag, bool notify=false);

private:
    /* this should ONLY be called from within InventoryItem */
    void                    SetOnline(bool online, bool isRig);

public:
    void                    PutOnline(bool isRig=false) { SetOnline(true, isRig); }
    void                    PutOffline(bool isRig=false){ SetOnline(false, isRig); }
    bool                    IsOnline()                  { return (GetAttribute(AttrIsOnline).get_int() ? true : false); }

    /* public-access data functions handled in base class. */
    void                    SaveItem();  //save the item to the DB.

    /* virtual functions default to base class and overridden as needed */
    virtual void            Delete();  //totally removes item from game and deletes from the DB.
    // makes new stack of 'qty' and returns ref of new stack, or null if failed
    virtual InventoryItemRef Split(int32 qty, bool notify=true);
    virtual bool            Merge(InventoryItemRef to_merge, uint32 qty=0, bool notify=true);

    virtual void            AddItem(InventoryItemRef item);
    virtual void            RemoveItem(InventoryItemRef item);


    /* specific functions handled here */
    /* returns uID for new item.  saves item data to db */
    static uint32           CreateItemID( ItemData &data);
    /* returns uID for temp item, without saving to db */
    static uint32           CreateTempItemID( ItemData &data);
    /* loads attributes for this item */
    //bool LoadAttributes();
    double GetPackagedVolume();

    /* specific funtions for ShipItem, virtual here to allow generic class access */
    virtual void            SetPlayer(Client* pClient)  { /* do nothing here */ }
    virtual bool            HasPilot()                  { return false; }
    virtual Client*         GetPilot()                  { return nullptr; }

    /**********************************************************************************************
     * TEMPLATED LOADING INVOKATION EXPLANATION:
     * ItemCategory, ItemGroup, ItemType and Item classes and their children have special loading.
     *   Every such type has following methods: (with ShipItem being the exception)
     *
     *  static Load( <identifier>):
     *    Merges static and virtual loading trees.
     *    First calls static _Load() to create desired object and
     *    then calls its virtual _Load()
     *
     *  static _Load( <identifier>[, <data-argument>, ...]):
     *    creates item data and type info, then calls _Ty::LoadItem(), which then
     *    creates any additional data needed, and calls the item constructor
     *
     *  virtual _Load():
     *    Performs post-construction loading (container contents) if needed,
     *    then calls InventoryItem::_Load() to load the item's attributes and add the
     *    created item to it's location's inventory.
     */

    /*  Item Loading methods */
    /* calls _Ty::Load<_Ty>.  */
    static InventoryItemRef Load( uint32 itemID);
    /* creates new Item and calls item::_Load() */
    static InventoryItemRef SpawnItem( uint32 itemID, const ItemData &data);
    /* Spawns new Item.  whats difference here?? */
    static InventoryItemRef Spawn( ItemData &data);

    virtual bool _Load();

protected:
    /* template helper, calls template loader then class loader */
    template<class _Ty>
    static RefPtr<_Ty> Load( uint32 itemID)
    {
        // static load
        RefPtr<_Ty> i = _Ty::template _Load<_Ty>( itemID );
        if( !i )
            return RefPtr<_Ty>();

        // virtual load (load attributes)
        if( !i->_Load() )
            return RefPtr<_Ty>();

        return i;
    }

    /* template loader, calls class _LoadItem */
    template<class _Ty>
    static RefPtr<_Ty> _Load( uint32 itemID)
    {
        // pull the item info
        ItemData data;
        if( !sItemFactory.db()->GetItem( itemID, data ) )
            return RefPtr<_Ty>();

        // obtain type
        const ItemType *type = sItemFactory.GetType( data.typeID );
        if( type == nullptr )
            return RefPtr<_Ty>();

        return _Ty::template _LoadItem<_Ty>( itemID, *type, data );
    }

    /* template class _LoadItem.  defined in derived class. calls class c'tor */
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem( uint32 itemID, const ItemType &type, const ItemData &data);


public:
    /* Primary public packet builders  */
    PyRep*                  GetItem() const             { return GetItemRow(); }

    void                    GetItemRow( PyPackedRow* into ) const;
    void                    GetItemStatusRow( PyPackedRow* into ) const;
    void                    GetModuleStatusRow( PyPackedRow* into ) const;
    void                    GetChargeStatusRow( uint32 shipID, PyPackedRow* into ) const;

    bool                    Populate(Rsp_CommonGetInfo_Entry &into);

    PyList*                 GetItemInfo() const;
    PyObject*               ItemGetInfo();
    PyPackedRow*            GetItemRow() const;
    PyPackedRow*            GetItemStatusRow() const;
    PyPackedRow*            GetModuleStatusRow() const;
    PyPackedRow*            GetChargeStatusRow(uint32 shipID) const;

protected:
    Inventory* pInventory;

    // our item data:
    const uint32            m_itemID;
    std::string             m_itemName;
    bool                    m_contraband;
    bool                    m_singleton;
    int32                   m_quantity;
    uint32                  m_locationID;
    uint32                  m_ownerID;
    std::string             m_customInfo;
    const ItemType &        m_type;
    EVEItemFlags            m_flag;
    GPoint                  m_position;

/* new effects processing system */
public:
    /*  this checks this item's required skills against callers' current skills.
     *  returns true if all pass */
    bool SkillCheck(InventoryItemRef refItem);

    // this deletes all attributes, reloads default attribs from itemType and clears m_modifiers
    //  when called at the wrong time, this will really fuck up ship attributes.  ;)
    void ClearModifiers();
    void AddModifier(fxData data);
    void RemoveModifier(fxData data);

    //  if itemType requires skill(skillID) return true else return false
    bool HasReqSkill(const uint16 skillID)              { return m_type.HasReqSkill(skillID); }

    // gotta make this public for now...
    std::multimap<int8, fxData> m_modifiers;     // k,v of math, data<math, src, targLoc, targAttr, srcAttr, grpID, typeID>, ordered by key (mathMethod)

    /*  new attribute system */
    AttributeMap* GetAttributeMap()                     { return pAttributeMap; }

    void SetAttribute(uint16 attrID, int num, bool notify=true);
    void SetAttribute(uint16 attrID, uint32 num, bool notify=true);
    void SetAttribute(uint16 attrID, int64 num, bool notify=true);
    void SetAttribute(uint16 attrID, double num, bool notify=true);
    void SetAttribute(uint16 attrID, EvilNumber num, bool notify=true);
    void MultiplyAttribute(uint16 attrID, EvilNumber num, bool notify=false);
    bool HasAttribute(const uint16 attrID) const                       { return pAttributeMap->HasAttribute(attrID); }
    bool HasAttribute(const uint16 attrID, EvilNumber &value) const    { return pAttributeMap->HasAttribute(attrID, value); }
    bool SaveAttributes()                                              { return pAttributeMap->SaveAttributes(); }
    void ResetAttribute(uint16 attrID, bool notify=false)              { pAttributeMap->ResetAttribute(attrID, notify); }
    void DeleteAttribute(uint16 attrID)                                { pAttributeMap->DeleteAttribute(attrID); }

    EvilNumber GetAttribute(const uint16 attrID) const                 { return pAttributeMap->GetAttribute(attrID); }
    EvilNumber GetDefaultAttribute(const uint16 attrID) const          { return m_type.GetAttribute(attrID); }

protected:
    AttributeMap* pAttributeMap;

};

#endif
