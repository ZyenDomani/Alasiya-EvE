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

#ifndef __INVENTORY__H__INCL__
#define __INVENTORY__H__INCL__

#include "inventory/InventoryItem.h"

class CRowSet;
class OwnerData;

/* this class is content management for items that can contain other items */
class Inventory
{
    friend class InventoryItem;
public:
    Inventory(InventoryItemRef item);
    virtual ~Inventory()                                { /* do nothing here*/ }

    void Reset(ItemFactory* factory);
    void AddItem(InventoryItemRef item);
    void RemoveItem(InventoryItemRef item);
    void DeleteContents();
    void GetInventoryList(std::map<uint32, InventoryItemRef> &inventory);
    void GetInventoryVec(std::vector<InventoryItemRef> &itemVec);
    void StackAll(EVEItemFlags flag, uint32 forOwner = 0);

    bool IsEmpty()                                      { return mContents.empty(); }
    bool LoadContents(ItemFactory* factory);
    bool ValidateAddItem(EVEItemFlags flag, InventoryItemRef item) const;
    bool ContentsLoaded() const                         { return mContentsLoaded; }
    bool Contains(uint32 itemID) const                  { return mContents.find( itemID ) != mContents.end(); }
    bool GetItems(OwnerData od, std::vector< uint32 >& into) const;

    double GetCapacity(EVEItemFlags flag) const;
    double GetStoredVolume(EVEItemFlags flag) const;
    double GetRemainingCapacity(EVEItemFlags flag) const { return GetCapacity( flag ) - GetStoredVolume( flag ); }

    //uint32 inventoryID() const                          { return m_inventoryID; }

    InventoryItemRef GetByID(uint32 id) const;
    InventoryItemRef GetByTypeFlag(uint32 typeID, EVEItemFlags flag) const;

    /* Inventory-by-Flag methods */
    bool IsEmptyByFlag(EVEItemFlags flag) const;
    bool FindSingleByFlag(EVEItemFlags flag, InventoryItemRef &item) const;
    uint32 FindByFlag(EVEItemFlags flag, std::vector<InventoryItemRef> &items) const;
    uint32 ListByFlag(EVEItemFlags flag, std::vector<InventoryItemRef> &items) const;
    uint32 FindByFlagRange(EVEItemFlags low_flag, EVEItemFlags high_flag, std::vector<InventoryItemRef> &items) const;
    uint32 FindByFlagSet(std::set<EVEItemFlags> flags, std::vector<InventoryItemRef> &items) const;
    InventoryItemRef FindFirstByFlag(EVEItemFlags flag) const;

    /* Primary packet builders */
    CRowSet* List( EVEItemFlags flag, uint32 forOwner = 0 ) const;
    void List( CRowSet* into, EVEItemFlags flag, uint32 forOwner = 0 ) const;

    /**
     * Casts given InventoryItemRef to Inventory.
     *
     * @return Pointer to Inventory; NULL if given item isn't a valid inventory.
     */
    Inventory *Cast(InventoryItemRef item);


protected:
    InventoryDB* m_db;
    InventoryItemRef m_self;

    bool mContentsLoaded;

    uint32 m_inventoryID;

    std::vector<InventoryItemRef> _sortVector(std::vector<InventoryItemRef> &itemVec);
    std::map<uint32, InventoryItemRef> mContents;    //maps item ID to its instance. we own a ref to all of these.
};

#endif /* !__INVENTORY__H__INCL__ */

