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
    Rewrite:    Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "ConsoleCommands.h"
#include "EVEServerConfig.h"
#include "StaticDataMgr.h"
#include "PyCallable.h"
#include "character/Character.h"
#include "inventory/Inventory.h"
#include "inventory/ItemFactory.h"
#include "pos/Structure.h"
#include "ship/Ship.h"
#include "station/Station.h"
#include "station/StationDB.h"
#include "system/Container.h"
#include "system/SolarSystem.h"

/*
 * Inventory
 */
Inventory::Inventory(InventoryItemRef item)
{
    mContentsLoaded = false;
    m_self = item;
    m_myID = item->itemID();
}

void Inventory::Reset()
{
    Unload();
    LoadContents();
}

void Inventory::Unload()
{
    if (!mContentsLoaded)
        return;

    //  save contents on the off-chance they have changed, but not on shutdown. (saved in ItemFactory::Close())
    if (!sConsole.IsShutdown()) {
        std::vector<SaveData> items;
        items.clear();
        std::map<uint32, InventoryItemRef>::iterator itr = mContents.begin(), end = mContents.end();
        while (itr != end) {
            if (IsPlayerItem(itr->first) and (itr->second->flag() != flagSkill)) {   // only save player items (except skills - saved in Character::SaveAll())
                SaveData data = SaveData();
                    data.itemID = itr->first;
                    data.contraband = itr->second->contraband();
                    data.flag = itr->second->flag();
                    data.locationID = itr->second->locationID();
                    data.ownerID = itr->second->ownerID();
                    data.position = itr->second->position();
                    data.quantity = itr->second->quantity();
                    data.singleton = itr->second->singleton();
                    data.typeID = itr->second->typeID();
                    data.customInfo = itr->second->customInfo();
                items.push_back(data);
            }
            sItemFactory.RemoveItem(itr->first);
            itr = mContents.erase(itr);
        }

        m_db.SaveItems(items);
    }
    mContents.clear();
    mContentsLoaded = false;
}

bool Inventory::GetItems(OwnerData od, std::vector< uint32 >& into ) {
    return m_db.GetItemContents(od, into);
}

bool Inventory::LoadContents() {
    if (IsAgent(m_myID))
        return true;
    double profileStartTime = GetTimeUSeconds();
    /* rewrote logic, optimized, and fixed "empty inventory" for new chars in existing systems  -allan 22.2.16 */
    Client* pClient = sItemFactory.GetUsingClient();
    // test for character creation (which throws errors here and isnt really needed)
    if ((pClient != nullptr) and pClient->IsCharCreation())
        return true;
    if (IsStation(m_myID)) {
        if (pClient != nullptr) {
            if (pClient->IsHangarLoaded(m_myID))
                return true;
            pClient->AddStationHangar(m_myID);
            mContentsLoaded = false;
        }
    }

    // check if the contents has already been loaded
    if (mContentsLoaded) {
        _log(INV__INFO, "Inventory::LoadContents() - inventory %u(%p) already loaded.", m_myID, this);
        return true;
    }

    OwnerData od = OwnerData();
        od.ownerID = 1;
        od.locID = m_myID;

    std::vector<uint32> items;
    if (pClient != nullptr) {
        if (pClient->IsValidSession())
            od.corpID = pClient->GetCorporationID();
        if (IsStation(m_myID)) {
            if (!StationItemRef::StaticCast(m_self)->IsLoaded())
                StationDB::LoadOffices(od, items);
            if (IsPlayerCorp(od.corpID)) {
                /* this will load all non-NPC corp items in this station */
                od.ownerID = od.corpID;
                _log(INV__TRACE, "Inventory::LoadContents()::IsPlayerCorp() - Loading inventory %u(%p) with owner %u", m_myID, this , od.ownerID);
                GetItems(od, items);
            }
        } else if (IsOffice(m_myID)) {
            if (IsPlayerCorp(od.corpID)) {
                /* this will load corp hangars' inventory for this station */
                od.ownerID = od.corpID;
                _log(INV__TRACE, "Inventory::LoadContents() - Loading office inventory %u(%p) for corp %u in station %s",\
                            m_myID, this , od.ownerID, (pClient->IsValidSession() ? itoa(pClient->GetStationID()) : "(invalid)"));
                GetItems(od, items);
            } else {
                // make error for loading office and NOT a PC corp
                _log(INV__WARNING, "Inventory::LoadContents() - inventory of officeID %u using corpID %u. Continuing...", m_myID, od.corpID);
            }
        }
        if (pClient->IsValidSession())
            od.ownerID = pClient->GetCharacterID();
        else
            od.ownerID = pClient->GetCharID();
    }

    _log(INV__TRACE, "Inventory::LoadContents() - Loading inventory %u(%p) with owner %u", m_myID, this , od.ownerID);
    if (!GetItems(od, items)) {
        _log(INV__ERROR, "Inventory::LoadContents() - Failed to get items of inventory %u", m_myID);
        if ((pClient != nullptr) and IsStation(m_myID))
            pClient->RemoveStationHangar(m_myID);
        return false;
    }

    for (auto cur : items) {
        if ((cur == od.ownerID) or (cur == od.locID) or (cur == m_myID))
            continue;
        InventoryItemRef iRef = sItemFactory.GetItem(cur);
        if (iRef.get() == nullptr) {
            _log(INV__WARNING, "Inventory::LoadContents() - Failed to load item %u contained in %u. Skipping.", cur, m_myID);
            continue;
        } else
            AddItem(iRef);
    }

    if (sConfig.debug.UseProfiling)
        sProfile.AddTime(itemloadProfile, GetTimeUSeconds() - profileStartTime);

    return (mContentsLoaded = true);
}

void Inventory::AddItem(InventoryItemRef iRef) {
    if (iRef.get() == nullptr)
        return;    //segfault check
    std::map<uint32, InventoryItemRef>::iterator itr = mContents.find(iRef->itemID());
    std::pair <std::_Rb_tree_iterator <std::pair <const uint32, InventoryItemRef > >, bool > test;
    if (itr == mContents.end())
        test = mContents.insert(std::make_pair(iRef->itemID(), iRef));

    if (test.second)
        _log(INV__TRACE, "Inventory::AddItem()  Updated %s(%u) to contain %s(%u) in %s.", \
                m_self->itemName().c_str(), m_myID, iRef->itemName().c_str(), iRef->itemID(), sDataMgr.GetFlagName(iRef->flag()));
    else
        _log(INV__TRACE, "Inventory::AddItem()  %s(%u) already contains %s(%u) in %s.", \
                m_self->itemName().c_str(), m_myID, iRef->itemName().c_str(), iRef->itemID(), sDataMgr.GetFlagName(iRef->flag()));
}

void Inventory::RemoveItem(InventoryItemRef iRef) {
    if (iRef.get() == nullptr)
        return;    //segfault check
    std::map<uint32, InventoryItemRef>::iterator itr = mContents.find(iRef->itemID());
    if (itr != mContents.end()) {
        mContents.erase(itr);
        _log(INV__TRACE, "Inventory::RemoveItem()  Updated %s(%u) to no longer contain %s(%u) at %s.", \
                m_self->itemName().c_str(), m_myID, iRef->itemName().c_str(), iRef->itemID(), sDataMgr.GetFlagName(iRef->flag()));
    } else
        _log(INV__TRACE,"Inventory::RemoveItem()  %s(%u) does not contain %s(%u) at %s.", \
                m_self->itemName().c_str(), m_myID, iRef->itemName().c_str(), iRef->itemID(), sDataMgr.GetFlagName(iRef->flag()));
}

void Inventory::DeleteContents()
{
    if (!mContentsLoaded)
        return;
    InventoryItemRef iRef(nullptr);
    std::map<uint32, InventoryItemRef>::iterator cur = mContents.begin();
    while (cur != mContents.end()) {
        iRef = cur->second;
        ++cur;
        iRef->Delete();
    }

    mContents.clear();
    mContentsLoaded = false;
}

CRowSet* Inventory::List(EVEItemFlags flag, uint32 ownerID/*0*/) const
{
    PyList *keywords = new PyList();
        keywords->AddItem(new_tuple(new PyString("stacksize"), new PyToken("util.StackSize")));
        keywords->AddItem(new_tuple(new PyString("singleton"), new PyToken("util.Singleton")));
    DBRowDescriptor* header = new DBRowDescriptor(keywords);
        header->AddColumn("itemID",     DBTYPE_I8);
        header->AddColumn("typeID",     DBTYPE_I4);
        header->AddColumn("ownerID",    DBTYPE_I4);
        header->AddColumn("locationID", DBTYPE_I4);
        header->AddColumn("flagID",     DBTYPE_I2);
        header->AddColumn("quantity",   DBTYPE_I4);
        header->AddColumn("groupID",    DBTYPE_I2);
        header->AddColumn("categoryID", DBTYPE_I2);
        header->AddColumn("customInfo", DBTYPE_STR);
    CRowSet* rowset = new CRowSet(&header);
    List(rowset, flag, ownerID);
    if (is_log_enabled(INV__DUMP))
        rowset->Dump(INV__DUMP, "    ");
    return rowset;
}

void Inventory::List(CRowSet* into, EVEItemFlags flag, uint32 ownerID) const {
    //there has to be a better way to build this...
    PyPackedRow* row(nullptr);
    // office hangars list ALL items.  client separates by division flag
    if (IsOffice(m_myID) or IsCharacter(m_myID)) {
        for (auto cur : mContents) {
            row = into->NewRow();
            cur.second->GetItemRow(row);
        }
    } else {
        for (auto cur : mContents) {
            if (((ownerID == 0)        or (cur.second->ownerID() == ownerID))
            and ((flag == flagAnywhere) or (cur.second->flag() == flag))) {
                row = into->NewRow();
                cur.second->GetItemRow(row);
            }
        }
    }
}

// for stations only
void Inventory::GetInvForOwner(uint32 ownerID, std::vector< InventoryItemRef >& items)
{
    if (!IsOffice(m_myID) and !IsStation(m_myID)) {
        _log(INV__ERROR, "GetInvForOwner called on non-station item %s(%u)", m_self->itemName().c_str(), m_myID);
        EvE::traceStack();
    }
    for (auto cur : mContents)
        if (cur.second->ownerID() == ownerID)
            items.push_back(cur.second);
}

InventoryItemRef Inventory::FindFirstByFlag(EVEItemFlags flag) const {
    for (auto cur : mContents)
        if (cur.second->flag() == flag)
            return cur.second;

    return InventoryItemRef(nullptr);
}

InventoryItemRef Inventory::GetByID(uint32 id) const {
    std::map<uint32, InventoryItemRef>::const_iterator res = mContents.find(id);
    if (res != mContents.end())
        return res->second;

    return InventoryItemRef(nullptr);
}

InventoryItemRef Inventory::GetByTypeFlag(uint32 typeID, EVEItemFlags flag) const {
    for (auto cur : mContents)
        if ((cur.second->typeID() == typeID)
        and (cur.second->flag() == flag))
            return cur.second;

    return InventoryItemRef(nullptr);
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
    itemVec = SortVector(itemVecTmp);
}

std::vector<InventoryItemRef> Inventory::SortVector(std::vector<InventoryItemRef> &itemVec)
{
    //15:53:09 L Inventory::SortVector: 41 items sorted in 0.177us with 480 loops.

    /* sorts a vector of items by category, with loaded modules first (in slot order), then loaded charges (in slot order), then cargo
     * if there is only one item, no sorting required...
     *  this should only be called by ships
     *   -allan
     */
    if (itemVec.size() < 2)
        return itemVec;

    uint16 count = 0;
    double start = 0.0;
    if (sConfig.debug.IsTestServer)
        if (sConfig.debug.UseProfiling)
            start = GetTimeUSeconds();

    //begin basic sort
    bool done = false;
    InventoryItemRef tmp;

    while (!done) { //check if sorted
        done = true;  //assume sorted
        for (int i = 0, i2 = 1; (i < itemVec.size()) && (i2 < itemVec.size()); ++i, ++i2) { //iterate though list
            if ((IsModuleSlot(itemVec[i]->flag())) && (IsModuleSlot(itemVec[i2]->flag()))) {
                if (itemVec[i]->categoryID() > itemVec[i2]->categoryID()) {  //check if each pair is sorted by category.  subsystems > charges > modules
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
            ++count;
        }
    }

    if (sConfig.debug.IsTestServer)
        if (sConfig.debug.UseProfiling)
            _log(ITEM__TRACE, "Inventory::SortVector() - %u items sorted in %.3fus with %u loops.", itemVec.size(), (GetTimeUSeconds() - start), count);
    return itemVec;  //returns sorted list
}

uint32 Inventory::GetItemsByFlag(EVEItemFlags flag, std::vector<InventoryItemRef> &items) const {
    for (auto cur : mContents)
        if (cur.second->flag() == flag)
            items.push_back(cur.second);
    return items.size();
}

bool Inventory::GetTypesByFlag(EVEItemFlags flag, std::map< uint16, InventoryItemRef >& items)
{
    for (auto cur : mContents)
        if (cur.second->flag() == flag)
            items.emplace(cur.second->typeID(), cur.second);

    if (items.size() > 0)
        return true;
    return false;
}

InventoryItemRef Inventory::GetItemByTypeFlag(uint16 typeID, EVEItemFlags flag)
{
    std::vector<InventoryItemRef> items;
    if (GetItemsByFlag(flag, items) < 1)
        return InventoryItemRef(nullptr);

    for (auto cur : items)
        if (cur->typeID() == typeID )
            return cur;

    return InventoryItemRef(nullptr);
}

bool Inventory::GetSingleItemByFlag(EVEItemFlags flag, InventoryItemRef& iRef) const {
    for (auto cur : mContents)
        if (cur.second->flag() == flag) {
            iRef = cur.second;
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

uint32 Inventory::GetItemsByFlagRange(EVEItemFlags lowflag, EVEItemFlags highflag, std::vector<InventoryItemRef> &items) const {
    uint32 count = 0;
    for (auto cur : mContents)
        if (cur.second->flag() >= lowflag && cur.second->flag() <= highflag) {
            items.push_back(cur.second);
            ++count;
        }
    return count;
}

uint32 Inventory::GetItemsByFlagSet(std::set<EVEItemFlags> flags, std::vector<InventoryItemRef> &items) const {
    uint32 count = 0;
    for (auto cur : mContents)
        if (flags.find(cur.second->flag()) != flags.end()) {
            items.push_back(cur.second);
            ++count;
        }
    return count;
}

bool Inventory::ContainsTypeQty(uint16 typeID, uint32 qty/*0*/) const
{
    uint16 count = 0;
    for (auto cur : mContents) {
        if (cur.second->typeID() == typeID )
            if (cur.second->quantity() >= qty) {
                return true;
            } else {
                count += cur.second->quantity();
            }
    }
    if (count >= qty)
        return true;
    return false;
}

bool Inventory::ContainsTypeByFlag(uint16 typeID, EVEItemFlags flag) const
{
    std::vector<InventoryItemRef> itemVec;
    if (GetItemsByFlag(flag, itemVec) < 1)
        return false;
    for (auto cur : itemVec)
        if (cur->typeID() == typeID )
                return true;
    return false;
}


void Inventory::StackAll(EVEItemFlags locFlag, uint32 ownerID/*0*/)
{
    InventoryItemRef iRef(nullptr);
    std::map<uint16, InventoryItemRef> types;
    std::map<uint16, InventoryItemRef>::iterator tItr = types.end();
    std::map<uint32, InventoryItemRef>::iterator lItr = mContents.begin();
    while (lItr != mContents.end()) {
        iRef = lItr->second;
        ++lItr;
        if (IsModuleSlot(iRef->flag()))    // check to avoid removing loaded modules from ship
            continue;
        if (iRef->singleton())
            continue;
        if ((ownerID == 0) or (ownerID == iRef->ownerID())) {
            tItr = types.find(iRef->typeID());
            if (tItr == types.end())
                types.insert(std::make_pair(iRef->typeID(), iRef));
            else // found another stack of this type.  merge it.
                tItr->second->Merge(iRef);  // this call will remove item from mContents.  does not invalidate the iterator.
        }
    }
}

double Inventory::GetStoredVolume(EVEItemFlags locationFlag) const
{
    double totalVolume(0.0f);
    for (auto cur : mContents)
        if (cur.second->flag() == locationFlag)
            totalVolume += cur.second->quantity() * cur.second->GetAttribute(AttrVolume).get_float();
            // This formula is a hybrid of both old and new ones...and it works \o/

    return totalVolume;
}

bool Inventory::ValidateAddItem(EVEItemFlags flag, InventoryItemRef iRef) const
{
    float volume = iRef->quantity() * iRef->GetAttribute(AttrVolume).get_float();
    double capacity = GetRemainingCapacity(flag);
    if (volume > capacity) {
        Client* pClient = sItemFactory.GetUsingClient();
        if (pClient != nullptr) {
            std::map<std::string, PyRep *> args;
            args["volume"] = new PyFloat(volume);
            sItemFactory.UnsetUsingClient();
            if (IsCargoHoldFlag(flag)) {
                args["available"] = new PyFloat(capacity);
                throw PyException(MakeUserError("NotEnoughCargoSpace", args));
            } else if (IsSpecialHoldFlag(flag)) {
                args["item"] = new PyString(iRef->itemName());
                args["maximum"] = new PyFloat(capacity);
                args["used"] = new PyFloat(capacity);
                throw PyException(MakeUserError("NotEnoughSpecialBaySpaceOverload", args));
            } else if (IsModuleSlot(flag)) {
                args["capacity"] = new PyFloat(capacity);
                throw PyException(MakeUserError("NotEnoughChargeSpace", args));
            } else if (IsHangarFlag(flag)) {
                args["item"] = new PyString(iRef->itemName());
                args["maximum"] = new PyFloat(capacity);
                args["used"] = new PyFloat(capacity);
                throw PyException(MakeUserError("NotEnoughSpaceOverload", args));
            } else if (flag == flagDroneBay) {
                args["available"] = new PyFloat(capacity);
                throw PyException(MakeUserError("NotEnoughDroneBaySpace", args));
            } else {
                args["itemTypeName"] = new PyString(iRef->type().name());
                args["itemVolume"] = new PyFloat(volume);
                args["volumeAvailable"] = new PyFloat(capacity);
                throw PyException(MakeUserError("NoSpaceForThat", args));
            }
        }
        return false;
    }
    return true;
}
//{'FullPath': u'UI/Messages', 'messageID': 259141, 'label': u'NotEnoughCargoSpaceFor1UnitBody'}(u'One unit of {[item]type.name} would take {[numeric]required, decimalPlaces=2} units of space. This container only has {[numeric]free, decimalPlaces=2} units free.', None, {u'{[numeric]free, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'free'}, u'{[item]type.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'type'}, u'{[numeric]required, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'required'}})
//{'FullPath': u'UI/Messages', 'messageID': 259208, 'label': u'NotEnoughCargoSpaceOverloadBody'}(u'The cargo bay is overloaded and cannot be made to fit {item}. It is currently only capable of fitting {maximum} units and it is currently jammed full with {used} units.', None, {u'{maximum}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'maximum'}, u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{used}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'used'}})
//{'FullPath': u'UI/Messages', 'messageID': 259210, 'label': u'NotEnoughDroneBaySpaceOverloadBody'}(u'The drone bay is overloaded and cannot be made to fit {item}. It is currently only capable of fitting {maximum} units and it is currently jammed full with {used} units.', None, {u'{maximum}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'maximum'}, u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{used}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'used'}})
//{'FullPath': u'UI/Messages', 'messageID': 259213, 'label': u'NotEnoughSpaceOverloadBody'}(u'The storage area is overloaded and cannot be made to fit any {item}. It is currently only capable of fitting {maximum} units and it is currently jammed full with {used} units.', None, {u'{maximum}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'maximum'}, u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{used}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'used'}})
//{'FullPath': u'UI/Messages', 'messageID': 257337, 'label': u'NotEnoughSpecialBaySpaceOverloadBody'}(u'That cargo bay is overloaded and cannot be made to fit {item}. It is currently only capable of fitting {maximum} units and it is currently jammed full with {used}units.', None, {u'{maximum}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'maximum'}, u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{used}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'used'}})
//{'FullPath': u'UI/Messages', 'messageID': 259154, 'label': u'NoSpaceForThatBody'}(u"You can't add the {[item]itemTypeName.name} as there simply isn't enough room for it to fit. It takes up {[numeric]itemVolume, decimalPlaces=2} units of volume, and there are only {[numeric]volumeAvailable} left.", None, {u'{[item]itemTypeName.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'itemTypeName'}, u'{[numeric]volumeAvailable}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'volumeAvailable'}, u'{[numeric]itemVolume, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'itemVolume'}})


double Inventory::GetCapacity(EVEItemFlags flag) const {
    // added hangar capy for all hangar types
    // are we missing any hangar types here?  POS types?
    switch( flag ) {
        case flagHangar:
        case flagOffice:
        case flagProperty:
        case flagDelivery:
        case flagImpounded:
        case flagCorpMarket:
        case flagCorpHangar2:
        case flagCorpHangar3:
        case flagCorpHangar4:
        case flagCorpHangar5:
        case flagCorpHangar6:
        case flagCorpHangar7:                   return maxHangarCapy;
        case flagAutoFit:
        case flagCargoHold:                     return m_self->GetAttribute(AttrCapacity).get_float();
        case flagDroneBay:                      return m_self->GetAttribute(AttrDroneCapacity).get_float();
        case flagShipHangar:                    return m_self->GetAttribute(AttrShipMaintenanceBayCapacity).get_float();
        case flagSecondaryStorage:              return m_self->GetAttribute(AttrCapacitySecondary).get_float();
        case flagFuelBay:                       return m_self->GetAttribute(AttrFuelBayCapacity).get_float();
        case flagOreHold:                       return m_self->GetAttribute(AttrOreHoldCapacity).get_float();
        case flagGasHold:                       return m_self->GetAttribute(AttrGasHoldCapacity).get_float();
        case flagAmmoHold:                      return m_self->GetAttribute(AttrAmmoHoldCapacity).get_float();
        case flagShipHold:                      return m_self->GetAttribute(AttrShipHoldCapacity).get_float();
        case flagMineralHold:                   return m_self->GetAttribute(AttrMineralHoldCapacity).get_float();
        case flagSalvageHold:                   return m_self->GetAttribute(AttrSalvageHoldCapacity).get_float();
        case flagSmallShipHold:                 return m_self->GetAttribute(AttrSmallShipHoldCapacity).get_float();
        case flagLargeShipHold:                 return m_self->GetAttribute(AttrLargeShipHoldCapacity).get_float();
        case flagIndustrialShipHold:            return m_self->GetAttribute(AttrIndustrialShipHoldCapacity).get_float();
        // for PI
        case flagCommandCenterHold:             return m_self->GetAttribute(AttrCommandCenterHoldCapacity).get_float();
        case flagPlanetaryCommoditiesHold:      return m_self->GetAttribute(AttrPlanetaryCommoditiesHoldCapacity).get_float();
        // for pos battery/array
        case flagHiSlot0:                       return m_self->GetAttribute(AttrAmmoCapacity).get_float();
    }

    _log(INV__WARNING, "Inventory::GetCapacity() - Unsupported flag %s(%u) called for %s(%u)", \
                sDataMgr.GetFlagName(flag), flag, m_self->itemName().c_str(), m_myID);
    return 0.0;
}

