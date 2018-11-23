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


#include "inventory/InventoryDB.h"


class CRowSet;
class OwnerData;

/* this class is content management for items that can contain other items */
class Inventory
{
    friend class InventoryItem;
public:
    Inventory(InventoryItemRef item);
    virtual ~Inventory() noexcept                       { /* do nothing here*/ }

    void Reset();
    void Unload();  // used by stations and solar systems for item saving and unloading
    void AddItem(InventoryItemRef item);
    void RemoveItem(InventoryItemRef item);
    void DeleteContents();
    void GetInventoryList(std::map<uint32, InventoryItemRef> &inventory);
    void GetInventoryVec(std::vector<InventoryItemRef> &itemVec);
    void StackAll(EVEItemFlags flag, uint32 forOwner = 0);

    bool IsEmpty()                                      { return mContents.empty(); }
    bool HasShip();
    bool LoadContents();
    // this will throw if it fails.
    bool ValidateAddItem(EVEItemFlags flag, InventoryItemRef item) const;
    bool ContentsLoaded() const                         { return mContentsLoaded; }
    bool ContainsItem(uint32 itemID) const                  { return mContents.find( itemID ) != mContents.end(); }
    bool ContainsTypeQty(uint16 typeID, uint32 qty) const;

    double GetCapacity(EVEItemFlags flag) const;
    double GetStoredVolume(EVEItemFlags flag) const;
    double GetRemainingCapacity(EVEItemFlags flag) const { return GetCapacity( flag ) - GetStoredVolume( flag ); }

    InventoryItemRef GetByID(uint32 id) const;
    InventoryItemRef GetByTypeFlag(uint32 typeID, EVEItemFlags flag) const;

    /* Inventory-by-Flag methods */
    /** @todo update to use m_usedVolumeByFlag container? */
    bool IsEmptyByFlag(EVEItemFlags flag) const;
    bool FindSingleByFlag(EVEItemFlags flag, InventoryItemRef &item) const;
    bool FindTypesByFlag(EVEItemFlags flag, std::map<uint16, InventoryItemRef> &items);
    uint32 FindByFlag(EVEItemFlags flag, std::vector<InventoryItemRef> &items) const;
    uint32 FindByFlagRange(EVEItemFlags low_flag, EVEItemFlags high_flag, std::vector<InventoryItemRef> &items) const;
    uint32 FindByFlagSet(std::set<EVEItemFlags> flags, std::vector<InventoryItemRef> &items) const;
    InventoryItemRef FindFirstByFlag(EVEItemFlags flag) const;

    /* Primary packet builders */
    CRowSet* List( EVEItemFlags flag, uint32 forOwner = 0 ) const;
    void List( CRowSet* into, EVEItemFlags flag, uint32 forOwner = 0 ) const;


protected:
    bool GetItems(OwnerData od, std::vector< uint32 >& into);

    InventoryDB m_db;
    InventoryItemRef m_self;

    bool mContentsLoaded;

    uint32 m_myID;

    std::map<EVEItemFlags, double> m_itemsByFlag;

    std::vector<InventoryItemRef> SortVector(std::vector<InventoryItemRef> &itemVec);
    std::map<uint32, InventoryItemRef> mContents;    //maps item ID to its instance. we own a ref to all of these.
};

#endif /* !__INVENTORY__H__INCL__ */

