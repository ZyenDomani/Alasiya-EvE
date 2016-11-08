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
    Updates:    Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "EVEServerConfig.h"
#include "PyCallable.h"
#include "character/Character.h"
#include "inventory/Inventory.h"
#include "pos/Structure.h"
#include "ship/Ship.h"
#include "station/Station.h"
#include "system/Container.h"
#include "system/SolarSystem.h"

/*
 * Inventory
 */
Inventory::Inventory(InventoryItemRef item)
{
    mContentsLoaded = false;
    m_self = item;
    m_inventoryID = item->itemID();
}

Inventory* Inventory::Cast(InventoryItemRef item) {
    if (!item) return nullptr;
    return this;
}

void Inventory::Reset(ItemFactory* factory)
{
    mContentsLoaded = false;
    LoadContents(factory);
}

bool Inventory::GetItems(OwnerData od, std::vector< uint32 >& into ) const {
    return m_db->GetItemContents(od, into);
}

bool Inventory::LoadContents(ItemFactory* factory) {
    if (IsAgent(m_inventoryID)) return true;
    double profileStartTime = 0.0;
    if (sConfig.server.UseProfiling)
        profileStartTime = GetTimeUSeconds();
    /* rewrote logic, optimized, and fixed "empty inventory" for new chars in existing systems  -allan 22.2.16 */
    Client* pClient = factory->GetUsingClient();
    if (IsStation(m_inventoryID)) {
        if (pClient) {
            if (pClient->IsHangarLoaded(m_inventoryID))
                return true;
            pClient->AddStationHangar(m_inventoryID);
            mContentsLoaded = false;
        }
    }
    
    // check if the contents has already been loaded
    if (mContentsLoaded) {
        _log(INV__INFO, "Inventory::LoadContents() - inventory %u(%p) already loaded.", m_inventoryID, this);
        return true;
    }

    OwnerData od;
        od.ownerID = 1;
        od.locID = m_inventoryID;

    std::vector<uint32> items;
    if (pClient) {
        if (IsStation(m_inventoryID)) {
            if (IsPlayerCorp(pClient->GetCorporationID())){
                /* this will load all non-NPC corp items in this station */
                od.ownerID = pClient->GetCorporationID();
                _log(INV__TRACE, "Inventory::LoadContents() - Loading inventory %u(%p) with owner %u", m_inventoryID, this , od.ownerID);
                GetItems(od, items);
            }
        }
        od.ownerID = pClient->GetCharacterID();
    }

    _log(INV__TRACE, "Inventory::LoadContents() - Loading inventory %u(%p) with owner %u", m_inventoryID, this , od.ownerID);
    if (!GetItems(od, items)) {
        _log(INV__ERROR, "Inventory::LoadContents() - Failed to get items of inventory %u", m_inventoryID);
        if (pClient and IsStation(m_inventoryID))
            pClient->RemoveStationHangar(m_inventoryID);
        return false;
    }

    for (auto cur : items) {
        if ((cur == od.ownerID) or (cur == od.locID) or (cur == m_inventoryID)) continue;
        InventoryItemRef i = factory->GetItem(cur);
        if (!i) {
            _log(INV__WARNING, "Inventory::LoadContents() - Failed to load item %u contained in %u. Skipping.", cur, m_inventoryID);
            continue;
        } else
            AddItem(i);
    }

    if (sConfig.server.UseProfiling)
        sProfile.AddTime(_itemloadProfile, GetTimeUSeconds() - profileStartTime);

    return (mContentsLoaded = true);
}

void Inventory::AddItem(InventoryItemRef item) {
    if (!item.get()) return;    //segfault check
    std::map<uint32, InventoryItemRef>::iterator res = mContents.find(item->itemID());
    std::pair <std::_Rb_tree_iterator <std::pair <const uint32, InventoryItemRef > >, bool > test;
    if (res == mContents.end())
        test = mContents.insert(std::make_pair(item->itemID(), item));

    if (test.second)
        _log(INV__TRACE, "Inventory::AddItem()  Updated location %u(%p) to contain item %u with flag %d.", m_inventoryID, this, item->itemID(), (int)item->flag());
    else
        _log(INV__TRACE, "Inventory::AddItem()  location %u already contains item %u with flag %d.", m_inventoryID, item->itemID(), (int)item->flag());
}

void Inventory::RemoveItem(InventoryItemRef item) {
    if (!item.get()) return;    //segfault check
    std::map<uint32, InventoryItemRef>::iterator res = mContents.find(item->itemID());
    if (res != mContents.end()) {
        mContents.erase(res->first);
        _log(INV__TRACE, "Inventory::RemoveItem()  Updated location %u(%p) to no longer contain item %u.", m_inventoryID, this, item->itemID());
    } else
        _log(INV__TRACE,"Inventory::RemoveItem()  location %u does not contain item %u.", m_inventoryID, item->itemID());
}

void Inventory::DeleteContents()
{
    if (!mContentsLoaded) return;

    std::map<uint32, InventoryItemRef>::iterator cur = mContents.begin();
    while (cur != mContents.end()) {
        InventoryItemRef i = cur->second;
        ++cur;
        i->Delete();
    }

    mContents.clear();
    mContentsLoaded = false;
}

CRowSet* Inventory::List(EVEItemFlags _flag, uint32 forOwner) const
{
    PyList *keywords = new PyList();
        keywords->AddItem(new_tuple(new PyString("stacksize"), new PyToken("util.StackSize")));
        keywords->AddItem(new_tuple(new PyString("singleton"), new PyToken("util.Singleton")));
    DBRowDescriptor* header = new DBRowDescriptor(keywords);
        header->AddColumn("itemID",     DBTYPE_I8);
        header->AddColumn("typeID",     DBTYPE_I4);
        header->AddColumn("ownerID",    DBTYPE_I4);
        header->AddColumn("locationID", DBTYPE_I8);
        header->AddColumn("flagID",     DBTYPE_I2);
        header->AddColumn("quantity",   DBTYPE_I4);
        header->AddColumn("groupID",    DBTYPE_I2);
        header->AddColumn("categoryID", DBTYPE_I2);
        header->AddColumn("customInfo", DBTYPE_STR);
    CRowSet* rowset = new CRowSet(&header);
    List(rowset, _flag, forOwner);
    return rowset;
}

void Inventory::List(CRowSet* into, EVEItemFlags _flag, uint32 forOwner) const {
    //there has to be a better way to build this...
    /** @todo  need to verify changing owners when trading non-empty containers */
    PyPackedRow* row = nullptr;
    for (auto cur : mContents) {
        if (  (cur.second->flag() == _flag        || _flag == flagAnywhere)
            && (cur.second->ownerID() == forOwner || forOwner == 0))
        {
            row = into->NewRow();
            cur.second->GetItemRow(row);
        }
    }
}

InventoryItemRef Inventory::FindFirstByFlag(EVEItemFlags _flag) const {
    for (auto cur : mContents)
        if (cur.second->flag() == _flag)
            return cur.second;

    return InventoryItemRef();
}

InventoryItemRef Inventory::GetByID(uint32 id) const {
    std::map<uint32, InventoryItemRef>::const_iterator res = mContents.find(id);
    if (res != mContents.end())
        return res->second;

    return InventoryItemRef();
}

InventoryItemRef Inventory::GetByTypeFlag(uint32 typeID, EVEItemFlags flag) const {
    for (auto cur : mContents)
        if (cur.second->typeID() == typeID
            && cur.second->flag() == flag)
            return cur.second;

    return InventoryItemRef();
}

void Inventory::GetInventoryList(std::map<uint32, InventoryItemRef> &inventory) {
    for (auto cur : mContents)
        inventory.insert(std::pair<uint32, InventoryItemRef>(cur.first, cur.second));
}

void Inventory::GetInventoryVec(std::vector<InventoryItemRef> &itemVec) {
    std::vector<InventoryItemRef> itemVecTmp;
    itemVecTmp.clear();
    for (auto cur : mContents)
        itemVecTmp.push_back(cur.second);
    /* sorting method to put modules first, charges second, and cargo last
     *  this is needed to correctly online modules BEFORE trying to load charges
     */
    itemVec = _sortVector(itemVecTmp);
}

std::vector<InventoryItemRef> Inventory::_sortVector(std::vector<InventoryItemRef> &itemVec)
{
    //15:53:09 L Inventory::_sortVector: 41 items sorted in 0.177us with 480 loops.

    /* sorts a vector of items by category, with loaded modules first (in slot order), then loaded charges (in slot order), then cargo
     * if there is only one item, no sorting required...
     *  this should only be called by ships
     *   -allan
     */
    if (itemVec.size() < 2)
        return itemVec;

    uint16 count = 0;
    double start = 0.0;
    if (sConfig.server.IsTestServer)
        if (sConfig.server.UseProfiling)
            start = GetTimeUSeconds();

    //begin basic sort
    bool done = false;
    InventoryItemRef tmp;

    while (!done) { //check if sorted
        done = true;  //assume sorted
        for (int i = 0, i2 = 1; (i < itemVec.size()) && (i2 < itemVec.size()); i++, i2++) { //iterate though list
            if ((IsModuleSlot(itemVec[i]->flag())) && (IsModuleSlot(itemVec[i2]->flag()))) {
                if (itemVec[i]->categoryID() > itemVec[i2]->categoryID()) {  //check if each pair is sorted by category.  charges > modules
                    //it's not, so flip the values
                    tmp = itemVec[i];
                    itemVec[i] = itemVec[i2];
                    itemVec[i2] = tmp;
                    done = false;  //we weren't sorted, so now go back and check if we are
                }
            } else if ((IsCargoHoldFlag(itemVec[i]->flag())) && (IsModuleSlot(itemVec[i2]->flag()))) { //check if each pair is sorted by flag.  cargo > module
                //it's not, so flip the values
                tmp = itemVec[i];
                itemVec[i] = itemVec[i2];
                itemVec[i2] = tmp;
                done = false;  //we weren't sorted, so now go back and check if we are
            }
            count++;
        }
    }

    if (sConfig.server.IsTestServer)
        if (sConfig.server.UseProfiling)
            sLog.Log("Inventory::_sortVector", "%u items sorted in %.3fus with %u loops.", itemVec.size(), (GetTimeUSeconds() - start), count);

    return itemVec;  //return sorted list
}

uint32 Inventory::FindByFlag(EVEItemFlags _flag, std::vector<InventoryItemRef> &items) const {
    for (auto cur : mContents)
        if (cur.second->flag() == _flag)
            items.push_back(cur.second);

    return items.size();
}

uint32 Inventory::ListByFlag(EVEItemFlags _flag, std::vector<InventoryItemRef> &items) const {
    for (auto cur : mContents)
        if (cur.second->flag() == _flag)
            items.push_back(cur.second);

    return items.size();
}

bool Inventory::FindSingleByFlag(EVEItemFlags flag, InventoryItemRef &item) const {
    for (auto cur : mContents)
        if (cur.second->flag() == flag) {
            item = cur.second;
            return true;
        }

    return false;
}

bool Inventory::IsEmptyByFlag(EVEItemFlags flag) const {
    for (auto cur : mContents)
        if (cur.second->flag() == flag)
            return false;

    return true;
}

uint32 Inventory::FindByFlagRange(EVEItemFlags low_flag, EVEItemFlags high_flag, std::vector<InventoryItemRef> &items) const {
    uint32 count = 0;
    for (auto cur : mContents)
        if (cur.second->flag() >= low_flag && cur.second->flag() <= high_flag) {
            items.push_back(cur.second);
            ++count;
        }

    return count;
}

uint32 Inventory::FindByFlagSet(std::set<EVEItemFlags> flags, std::vector<InventoryItemRef> &items) const {
    uint32 count = 0;
    for (auto cur : mContents)
        if (flags.find(cur.second->flag()) != flags.end()) {
            items.push_back(cur.second);
            ++count;
        }

    return count;
}

void Inventory::StackAll(EVEItemFlags locFlag, uint32 forOwner)
{
    InventoryItemRef i;
    std::map<uint32, InventoryItemRef> types;

    std::map<uint32, InventoryItemRef>::const_iterator cur = mContents.begin();
    while (cur != mContents.end()) {
        // Iterator becomes invalid when the item
        // is moved out; we have to increment before
        // calling Merge().
        i = cur->second;
        ++cur;
        if (IsModuleSlot(i->flag()))
            continue;
        if ((!i->singleton()) && (forOwner == 0 || forOwner == i->ownerID())) {
            std::map<uint32, InventoryItemRef>::iterator res = types.find(i->typeID());
            if (res == types.end())
                types.insert(std::make_pair(i->typeID(), i));
            else
                res->second->Merge(i);
        }
    }
}

double Inventory::GetStoredVolume(EVEItemFlags locationFlag) const
{
    EvilNumber totalVolume(0.0f);

    for (auto cur : mContents)
        if (cur.second->flag() == locationFlag)
            totalVolume += cur.second->quantity() * cur.second->GetAttribute(AttrVolume);
            // This formula is a hybrid of both old and new ones...and it works \o/

    // this is crap... bleh... as it should return a EvilNumber
    return totalVolume.get_float();
}

bool Inventory::ValidateAddItem(EVEItemFlags flag, InventoryItemRef item) const
{
    EvilNumber volume = EvilNumber(item->quantity()) * item->GetAttribute(AttrVolume);
    double capacity = GetRemainingCapacity(flag);
    if (volume > capacity) {
        std::map<std::string, PyRep *> args;
            args["available"] = new PyFloat(capacity);
            args["volume"] = volume.GetPyObject();

        /** @todo  check for throwable status here */
        throw PyException(MakeUserError("NotEnoughCargoSpace", args));
        return false;
    }
    return true;
}

double Inventory::GetCapacity(EVEItemFlags flag) const {
    /** @todo verify the *hangar types and make sure flagHangar is for STATIONS ONLY  */
    switch( flag ) {
        case flagAutoFit:
        case flagCargoHold:
        case flagHangar:                        return m_self->GetAttribute(AttrCapacity).get_float();
        case flagDroneBay:                      return m_self->GetAttribute(AttrDroneCapacity).get_float();
        case flagShipHangar:                    return m_self->GetAttribute(AttrShipMaintenanceBayCapacity).get_float();
        case flagSecondaryStorage:              return m_self->GetAttribute(AttrCapacitySecondary).get_float();
        case flagSpecializedFuelBay:            return m_self->GetAttribute(AttrSpecialFuelBayCapacity).get_float();
        case flagSpecializedOreHold:            return m_self->GetAttribute(AttrSpecialOreHoldCapacity).get_float();
        case flagSpecializedGasHold:            return m_self->GetAttribute(AttrSpecialGasHoldCapacity).get_float();
        case flagSpecializedAmmoHold:           return m_self->GetAttribute(AttrSpecialAmmoHoldCapacity).get_float();
        case flagSpecializedShipHold:           return m_self->GetAttribute(AttrSpecialShipHoldCapacity).get_float();
        case flagSpecializedMineralHold:        return m_self->GetAttribute(AttrSpecialMineralHoldCapacity).get_float();
        case flagSpecializedSalvageHold:        return m_self->GetAttribute(AttrSpecialSalvageHoldCapacity).get_float();
        case flagSpecializedSmallShipHold:      return m_self->GetAttribute(AttrSpecialSmallShipHoldCapacity).get_float();
        case flagSpecializedLargeShipHold:      return m_self->GetAttribute(AttrSpecialLargeShipHoldCapacity).get_float();
        case flagSpecializedIndustrialShipHold: return m_self->GetAttribute(AttrSpecialIndustrialShipHoldCapacity).get_float();
    }

    _log(INV__WARNING, "Inventory::GetCapacity() - Unsupported flag %u called for item %u", flag, m_inventoryID);
    return 0.0;
}

