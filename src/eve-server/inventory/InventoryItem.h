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
#include "inventory/ItemType.h"
#include "inventory/ItemFactory.h"
#include "inventory/EVEAttributeMgr.h"

class PyRep;
class PyDict;
class PyObject;
class ServiceDB;
class ItemContainer;
class Rsp_CommonGetInfo_Entry;
class ItemRowset_Row;
class Inventory;

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
    InventoryItem(ItemFactory &_factory, uint32 _itemID, const ItemType &_type, const ItemData &_data);
    virtual ~InventoryItem();

    /* begin rewrite */

    /* generic access functions handled here */
    Inventory*              GetInventory()              { return m_inventory; }
    ItemFactory*            GetItemFactory()            { return &m_factory; }

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
    uint32                  typeID() const              { return type().id(); }
    uint32                  groupID() const             { return type().groupID(); }
    double                  radius() const              { return (HasAttribute(AttrRadius) ? GetAttribute(AttrRadius).get_float() : 1.0); }
    const ItemGroup &       group() const               { return type().group(); }
    const ItemCategory &    category() const            { return type().category(); }
    EVEItemCategories       categoryID() const          { return type().categoryID(); }

    /* public-access generic functions handled in base class. */
    void                    Rename(const char *to);
    void                    Relocate(const GPoint &pos);
    void                    SetCustomInfo(const char *ci);
    void                    ChangeOwner(uint32 new_owner, bool notify=true);
    void                    Move(uint32 location, EVEItemFlags flag=flagAutoFit, bool notify=true);
    void                    MoveInto(Inventory &new_home, EVEItemFlags flag=flagAutoFit, bool notify=true);
    void                    SendItemChange(uint32 toID, std::map<int32, PyRep *> &changes) const;

    bool                    ChangeSingleton(bool singleton, bool notify=true);
    bool                    AlterQuantity(int32 qty_change, bool notify=true);
    bool                    SetQuantity(int32 qty_new, bool notify=true);
    bool                    SetFlag(EVEItemFlags new_flag, bool notify=true);

    void                    SetOnline(bool online, bool isRig);
    void                    PutOnline(bool isRig=false) { SetOnline(true, isRig); }
    void                    PutOffline(bool isRig=false){ SetOnline(false, isRig); }
    bool                    IsOnline()                  { return (GetAttribute(AttrIsOnline).get_int() ? true : false); }

    /* public-access data functions handled in base class. */
    void                    SaveItem();  //save the item to the DB.
    void                    SetSaveTimerExpiry(uint32 saveTimerExpiry) \
                                { m_saveTimerExpiryTime = saveTimerExpiry; }
    void                    EnableSaveTimer() \
                                { m_saveTimer.Start( m_saveTimerExpiryTime * 1000, true ); }
    void                    DisableSaveTimer()          { m_saveTimer.Disable(); }
    bool                    CheckSaveTimer(bool iReset = true) \
                                { return m_saveTimer.Check( iReset ); }
    bool                    IsSaveTimerEnabled()        { return m_saveTimer.Enabled(); }
    uint32                  GetSaveTimerExpiry()        { return m_saveTimerExpiryTime; }

    /* virtual functions default to base class and overridden as needed */
    virtual void            Delete();  //remove the item from the DB.
    virtual InventoryItemRef Split(int32 qty_to_take, bool notify=true);
    virtual bool            Merge(InventoryItemRef to_merge, uint32 qty=0, bool notify=true);

    /* specific functions handled here */
    /* returns uID for new item.  saves item data to db */
    static uint32           CreateItemID(ItemFactory &factory, ItemData &data);
    /* returns uID for temp item, without saving to db */
    static uint32           CreateTempItemID(ItemFactory &factory, ItemData &data);
    /* loads attributes for this item */
    //bool LoadAttributes();
    uint32 GetPackagedVolume();

    /* specific funtions for ShipItem, virtual here to allow generic class access */
    virtual void            SetPlayer(Client* pClient)  { /* do nothing here */ }
    virtual bool            HasPilot()                  { return false; }
    virtual Client*         GetPilot()                  { return nullptr; }

    /**********************************************************************************************
     * TEMPLATED LOADING INVOKATION EXPLANATION:
     * ItemCategory, ItemGroup, ItemType and Item classes and their children have special loading.
     *   Every such type has following methods: (with ShipItem being the exception)
     *
     *  static Load(ItemFactory &factory, <identifier>):
     *    Merges static and virtual loading trees.
     *    First calls static _Load() to create desired object and
     *    then calls its virtual _Load()
     *
     *  static _Load(ItemFactory &factory, <identifier>[, <data-argument>, ...]):
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
    static InventoryItemRef Load(ItemFactory &factory, uint32 itemID);
    /* creates new Item and calls item::_Load() */
    static InventoryItemRef SpawnItem(ItemFactory &factory, uint32 itemID, const ItemData &data);
    /* Spawns new Item.  whats difference here?? */
    static InventoryItemRef Spawn(ItemFactory &factory, ItemData &data);

    virtual bool _Load();

protected:
    /* template helper, calls template loader then class loader */
    template<class _Ty>
    static RefPtr<_Ty> Load(ItemFactory &factory, uint32 itemID)
    {
        // static load
        RefPtr<_Ty> i = _Ty::template _Load<_Ty>( factory, itemID );
        if( !i )
            return RefPtr<_Ty>();

        // virtual load (load attributes)
        if( !i->_Load() )
            return RefPtr<_Ty>();

        return i;
    }

    /* template loader, calls class _LoadItem */
    template<class _Ty>
    static RefPtr<_Ty> _Load(ItemFactory &factory, uint32 itemID)
    {
        // pull the item info
        ItemData data;
        if( !factory.db().GetItem( itemID, data ) )
            return RefPtr<_Ty>();

        // obtain type
        const ItemType *type = factory.GetType( data.typeID );
        if( type == NULL )
            return RefPtr<_Ty>();

        return _Ty::template _LoadItem<_Ty>( factory, itemID, *type, data );
    }

    /* template class _LoadItem.  defined in derived class */
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem(ItemFactory &factory, uint32 itemID, const ItemType &type, const ItemData &data);


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
    Inventory* m_inventory = nullptr;

    ItemFactory& m_factory;

    Timer m_saveTimer;

    uint32 m_saveTimerExpiryTime;

    std::map<EVEItemFlags, double> m_cargoHoldsUsedVolumeByFlag;

    const uint32            m_itemID;
    std::string             m_itemName;

private:
    // our item data:
    bool                    m_contraband;
    bool                    m_singleton;
    int32                   m_quantity;
    uint32                  m_locationID;
    uint32                  m_ownerID;
    std::string             m_customInfo;
    const ItemType &        m_type;
    EVEItemFlags            m_flag;
    GPoint                  m_position;

    // for asteroid item:
    AsteroidData m_roidData;



/* end rewrite...originals follow */

/************************************************************************/
/* start experimental new attribute system ( semi-operational )         */
/************************************************************************/
protected:
    AttributeMap mAttributeMap;
    AttributeMap mDefaultAttributeMap;
public:
    bool SetAttribute(uint32 attributeID, int num, bool notify = true, bool shadow_copy_to_default_set = false);
    bool SetAttribute(uint32 attributeID, uint32 num, bool notify = true, bool shadow_copy_to_default_set = false);
    bool SetAttribute(uint32 attributeID, int64 num, bool notify = true, bool shadow_copy_to_default_set = false);
    bool SetAttribute(uint32 attributeID, uint64 num, bool notify = true, bool shadow_copy_to_default_set = false);
    bool SetAttribute(uint32 attributeID, double num, bool notify = true, bool shadow_copy_to_default_set = false);
    bool SetAttribute(uint32 attributeID, EvilNumber num, bool notify = true, bool shadow_copy_to_default_set = false);

    /**
     * GetAttribute
     * Retrieves the attribute of the entity.
     * @param attributeID the attribute to check for.
     * @returns the attribute value
     * @note a value of zero is returned and an error message generated if the value is not found.
     *
     * @note this function should be used very infrequently and only for specific reasons
     */
    EvilNumber GetAttribute(const uint32 attributeID) const;
    EvilNumber GetDefaultAttribute(const uint32 attributeID) const;
    /**
     * GetAttribute
     * Retrieves the attribute of the entity.
     * @note Should only be used when the attribute might not be defined.
     * @param attributeID the attribute to check for.
     * @param defaultValue a default value to return if no attribute is found.
     * @returns the attribute value or the default value.
     * @note does not generate an error message if the value is not found.
     *
     * @note this function should be used very infrequently and only for specific reasons
     */
    //EvilNumber GetAttribute(const uint32 attributeID, const uint32 defaultValue);

    /**
     * HasAttribute
     * Checks to see if the entity has the specified attribute.
     * value not altered if attribute not found.  This could be useful for preserving a default value.
     * @param attributeID the attribute to check for.
     * @returns true if this item has the attribute 'attributeID', false if it does not have this attribute
     *
     * returns true if this item has the attribute 'attributeID', false if it does not have this attribute
     * @note this function should be used very infrequently and only for specific reasons
     */
    bool HasAttribute(const uint32 attributeID) const;
    /**
     * HasAttribute
     * Checks to see if the entity has the specified attribute.
     * @param attributeID the attribute to check for.
     * @param value the location to return the attribute if it exist.
     * @returns true if this item has the attribute 'attributeID', false if it does not have this attribute
     *
     * @note this function should be used very infrequently and only for specific reasons
     */
    bool HasAttribute(const uint32 attributeID, EvilNumber& value) const;

    /**
     * SaveAttributes
     *
     * save all the attributes from a Item.
     *
     * @note this should be incorporated into the normal save function and only save when things have changes.
     */
    bool SaveAttributes();

    /*
     * ResetAttribute
     *
     *@note this function will force reload the default value for the specified attribute
     */
    bool ResetAttribute(uint32 attrID, bool notify=false);

    bool DeleteAttribute(uint32 attrID);

/************************************************************************/
/* end experimental new attribute system                                */
/************************************************************************/

};

#endif
