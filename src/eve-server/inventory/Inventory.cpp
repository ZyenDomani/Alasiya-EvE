/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or(at your option) any later
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

#include "../eve-server.h"

#include "Client.h"
#include "ConsoleCommands.h"
#include "EVEServerConfig.h"
#include "StaticDataMgr.h"
#include "PyCallable.h"
#include "character/Character.h"
#include "inventory/Inventory.h"
#include "inventory/ItemDB.h"
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
Inventory::Inventory(InventoryItemRef iRef)
: m_self(iRef),
m_loadClient(nullptr),
mContentsLoaded(false),
m_myID(iRef->itemID()),
m_profileStartTime(0)
{
}

void Inventory::Reset()
{
    Unload();
    LoadContents();
}

void Inventory::Unload() {
    if (!mContentsLoaded)
        return;

    //  save contents on the off-chance they have changed, but not on shutdown.(saved in ItemFactory::Close())
    if (sConsole.IsShutdown()) {
        mContents.clear();
        m_contentsByFlag.clear();
        mContentsLoaded = false;
    }

    Inventory* inv(nullptr);
    std::vector<Inv::SaveData> items;
    std::map<uint32, InventoryItemRef>::iterator itr = mContents.begin();
    while(itr != mContents.end()) {
        // test for item contents and unload as required
        inv = itr->second->GetMyInventory();
        if (inv != nullptr)
            inv->Unload();
        if (IsPlayerItem(itr->first)) {   // only save player items(except skills - saved in Character::SaveAll())
            if (itr->second->flag() == flagSkill) {
                sItemFactory.RemoveItem(itr->first);
                itr = mContents.erase(itr);
                continue;
            }

            Inv::SaveData data = Inv::SaveData();
                data.itemID = itr->first;
                data.contraband = itr->second->contraband();
                data.flag = itr->second->flag();
                data.locationID = itr->second->locationID();
                data.ownerID = itr->second->ownerID();
                data.position = itr->second->position();
                data.quantity = itr->second->quantity();
                data.singleton = itr->second->isSingleton();
                data.typeID = itr->second->typeID();
                data.customInfo = itr->second->customInfo();
            items.push_back(data);
        }
        sItemFactory.RemoveItem(itr->first);
        itr = mContents.erase(itr);
    }

    ItemDB::SaveItems(items);
}

bool Inventory::GetItems(OwnerData &od, std::vector< uint32 >& into ) {
    return m_db.GetItemContents(od, into);
}

bool Inventory::LoadContents() {
    if (IsAgent(m_myID))
        return true;
    m_profileStartTime = GetTimeUSeconds();
    /* rewrote logic, optimized, and fixed "empty inventory" for new chars in existing systems  -allan 22.2.16 */
    m_loadClient = sItemFactory.GetUsingClient();

    // test for character creation(which throws errors) and station loading
    if (m_loadClient != nullptr) {
        if (m_loadClient->IsCharCreation())
            return true;
        if (sDataMgr.IsStation(m_myID)) {
            if (m_loadClient->IsHangarLoaded(m_myID))
                return true;
            m_loadClient->AddLoadedHangar(m_myID);
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
	// get owner data
    if (m_loadClient != nullptr) {
        if (m_loadClient->IsValidSession())
            od.corpID = m_loadClient->GetCorporationID();
        if (sDataMgr.IsStation(m_myID)) {
            if (!StationItemRef::StaticCast(m_self)->IsLoaded())
                StationDB::LoadOffices(od, items);
            if (IsPlayerCorp(od.corpID)) {
                /* this will load all non-NPC corp items in this station */
                od.ownerID = od.corpID;
                _log(INV__TRACE, "Inventory::LoadContents()::IsPlayerCorp() - Loading inventory %u(%p) with owner %u", m_myID, this , od.ownerID);
                GetItems(od, items);
            }
        } else if (IsOfficeID(m_myID)) {
            if (IsPlayerCorp(od.corpID)) {
                /* this will load corp hangars' inventory for this station */
                od.ownerID = od.corpID;
                _log(INV__TRACE, "Inventory::LoadContents() - Loading office inventory %u(%p) for corp %u in station %s",\
                            m_myID, this , od.ownerID,(m_loadClient->IsValidSession() ? itoa(m_loadClient->GetStationID()) : "(invalid)"));
                GetItems(od, items);
            } else {
                // make error for loading office and NOT a PC corp
                _log(INV__WARNING, "Inventory::LoadContents() - inventory of officeID %u using corpID %u. Continuing...", m_myID, od.corpID);
            }
        }
        // test for char creation (data set incomplete in char creation)
        if (m_loadClient->IsValidSession()) {
            od.ownerID = m_loadClient->GetCharacterID();
        } else {
            od.ownerID = m_loadClient->GetCharID();
        }
    }

    _log(INV__TRACE, "Inventory::LoadContents() - Loading inventory of %s(%u) with owner %u", \
            m_self->name(), m_myID, od.ownerID);
    if (!GetItems(od, items)) {
        _log(INV__ERROR, "Inventory::LoadContents() - Failed to get inventory items for %s(%u)", \
                m_self->name(), m_myID);
        if ((m_loadClient != nullptr) and sDataMgr.IsStation(m_myID))
            m_loadClient->RemoveStationHangar(m_myID);
        return false;
    }

    _log(INV__TRACE, "Inventory::LoadContents() - Adding %lu items to inventory of %s(%u) with owner %u", \
            items.size(), m_self->name(), m_myID, od.ownerID);
    for (auto &cur : items) {
        if ((cur == od.ownerID) or (cur == od.locID) or (cur == m_myID))
            continue;
        AddItem(sItemFactory.GetItemRef(cur));
    }

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::itemload, GetTimeUSeconds() - m_profileStartTime);

    mContentsLoaded = true;

    return mContentsLoaded;
}

void Inventory::AddItem(InventoryItemRef iRef) {
    //segfault check
    if (iRef.get() == nullptr)
        return;

    std::map<uint32, InventoryItemRef>::iterator itr = mContents.find(iRef->itemID());
    if (itr == mContents.end()) {
        mContents[iRef->itemID()] = iRef;
    } else if (m_self->categoryID() == EVEDB::invCategories::Orbitals) {
        // item found...update qty for certain containers...customs offices for now
        itr->second->AlterQuantity(iRef->quantity());
    }

    if (IsCharacterID(m_myID)) {
        if (iRef->categoryID() == EVEDB::invCategories::Skill) {
            m_contentsByFlag.emplace(flagSkill, iRef);
            return;
        }
    } else {
        /*  what if we're splitting stacks in our inventory?  this will fuck it up...
        auto range = m_contentsByFlag.equal_range(iRef->flag());
        if (range.first == range.second) {
            m_contentsByFlag.emplace(iRef->flag(), iRef);
        } else {
            for (auto &cur = range.first; cur != range.second; ++cur) {
                if (cur->second == iRef) {
                    cur->second->AlterQuantity(iRef->quantity());
                }

                // what if there are multiple stacks of same type?  nah, they can use StackAll() if they want
                //if (itr->second->typeID() == iRef->typeID())
                //    cur->second->AlterQuantity(iRef->quantity());
            }
        }
        */
        m_contentsByFlag.emplace(iRef->flag(), iRef);
    }
}

void Inventory::RemoveItem(InventoryItemRef iRef) {
    //segfault check
    if (iRef.get() == nullptr)
        return;

    std::map<uint32, InventoryItemRef>::iterator itr = mContents.find(iRef->itemID());
    if (itr != mContents.end()) {
        mContents.erase(itr);
        _log(INV__TRACE, "Inventory::RemoveItem(1) - Updated %s(%u) to no longer contain %s(%u).", \
                m_self->name(), m_myID, iRef->name(), iRef->itemID());
    } else {
        _log(INV__WARNING,"Inventory::RemoveItem(1) - %s(%u) contents does not contain %s(%u).", \
                m_self->name(), m_myID, iRef->name(), iRef->itemID());
    }

    /** @todo @note  this isnt working right, and im not sure why yet...  */
    auto range = m_contentsByFlag.equal_range(iRef->flag());
    for (auto &cur = range.first; cur != range.second; ++cur) {
        if (cur->second == iRef) {
            m_contentsByFlag.erase(cur);
            _log(INV__TRACE, "Inventory::RemoveItem(2) - %s(%u) removed from %s(%u) flagMap at %s.", \
                    iRef->name(), iRef->itemID(), m_self->name(), m_myID, sDataMgr.GetFlagName(iRef->flag()));
            return;
        }
    }

    _log(INV__WARNING,"Inventory::RemoveItem(2) - %s(%u) flagMap does not contain %s(%u) in %s.", \
            m_self->name(), m_myID, iRef->name(), iRef->itemID(), sDataMgr.GetFlagName(iRef->flag()));
    if (sConfig.server.StackTrace)
        EvE::traceStack();
}

void Inventory::DeleteContents()
{
    if (!mContentsLoaded)
        return;
    InventoryItemRef iRef(nullptr);
    std::map<uint32, InventoryItemRef>::iterator cur = mContents.begin();
    while(cur != mContents.end()) {
        iRef = cur->second;
        ++cur;
        iRef->Delete();
    }

    mContents.clear();
    m_contentsByFlag.clear();
    mContentsLoaded = false;
}

CRowSet* Inventory::List(EVEItemFlags flag, uint32 ownerID/*0*/) const
{
    DBRowDescriptor* header = sDataMgr.CreateHeader();
    CRowSet* rowset = new CRowSet(&header);
    List(rowset, flag, ownerID);

    if (is_log_enabled(INV__LIST))
        rowset->Dump(INV__LIST, "    ");
    return rowset;
}

void Inventory::List(CRowSet* into, EVEItemFlags flag, uint32 ownerID) const {
    //there has to be a better way to build this...
    PyPackedRow* row(nullptr);
    // office hangars list ALL items.  client separates by division flag
    if (IsOfficeID(m_myID) or IsCharacterID(m_myID)) {
        for (auto &cur : mContents) {
            row = into->NewRow();
            cur.second->GetItemRow(row);
        }
    } else if (m_self->categoryID() == EVEDB::invCategories::Ship) {
        bool space = sDataMgr.IsSolarSystem(m_self->locationID());
        for (auto &cur : mContents) {
            // this also fills module/charges in fit window when docked.
            //  charges not sent like this in space(uses subLocation sent via shipInfo())
            if (space and IsFittingSlot(cur.second->flag()))
                if (cur.second->categoryID() == EVEDB::invCategories::Charge)
                    continue;
            row = into->NewRow();
            cur.second->GetItemRow(row);
        }
    } else {
        for (auto &cur : mContents) {
            if (((ownerID == 0) or(cur.second->ownerID() == ownerID))
            and((flag == flagAutoFit) or(cur.second->flag() == flag))) {
                row = into->NewRow();
                cur.second->GetItemRow(row);
            }
        }
    }
}

// is this right?  should i change this to use flagMap?
void Inventory::GetCargoList(std::multimap< uint8, InventoryItemRef >& cargoMap) {
    for (auto &cur : mContents)
        cargoMap.emplace(cur.second->flag(), cur.second);
}

float Inventory::GetCorpHangerCapyUsed() const {
    float totalVolume(0.0f);
    for (auto &cur : mContents)
        if (IsHangarFlag(cur.second->flag()))
            totalVolume += cur.second->quantity() * cur.second->GetAttribute(AttrVolume).get_float();
    return totalVolume;
}

void Inventory::GetInventoryVec(std::vector<InventoryItemRef> &itemVec) {
    std::vector<InventoryItemRef> itemVecTmp;
    for (auto &cur : mContents)
        itemVecTmp.push_back(cur.second);
    /* sorting method to put modules first, charges second, and cargo last
     *  this is needed to correctly fit modules BEFORE trying to load charges
     */
    itemVec = SortVector(itemVecTmp);
}

std::vector<InventoryItemRef> Inventory::SortVector(std::vector<InventoryItemRef> &itemVec)
{
    // my sort
    //15:53:09 [ItemTrace] Inventory::SortVector: 41 items sorted in 0.177ms with 480 loops.
    //15:40:20 [ItemTrace] Inventory::SortVector() - 30 items sorted in 28.250us with 87 loops.i
    //22:09:28 [InvTrace] Inventory::SortVector() - 47 items sorted in 129.250us with 644 loops.

    // std::swap
    //12:57:36 [ItemTrace] Inventory::SortVector() - 21 items sorted in 16.000us with 60 loops.
    //13:00:55 [ItemTrace] Inventory::SortVector() - 15 items sorted in 20.750us with 28 loops.
    //13:01:20 [ItemTrace] Inventory::SortVector() - 19 items sorted in 46.000us with 90 loops.
    /* sorts a vector of items by category, with loaded modules first(in slot order), then loaded charges(in slot order), then cargo
     * if there is only one item, no sorting required...
     *  this should only be called by ships
     *   -allan
     */
    if (itemVec.size() < 2)
        return itemVec;

    uint16 count(0);
    double start(GetTimeUSeconds());

    //begin basic sort
    bool done(false);
    InventoryItemRef tmp(nullptr);

    while(!done) { //check if sorted
        done = true;  //assume sorted
        //iterate though list
        for (int i = 0, i2 = 1;(i < itemVec.size()) &&(i2 < itemVec.size()); i++, i2++) {
            if ((IsModuleSlot(itemVec[i]->flag())) &&(IsModuleSlot(itemVec[i2]->flag()))) {
                //check if each pair is sorted by category.  subsystems > charges > modules
                if (itemVec[i]->categoryID() > itemVec[i2]->categoryID()) {
                    //it's not, so flip the values
                    //std::swap(itemVec[i],itemVec[i2]);  // this is ~100x slower on dev server
                    tmp = itemVec[i];
                    itemVec[i] = itemVec[i2];
                    itemVec[i2] = tmp;
                    done = false;  //we weren't sorted, so now go back and check if we are
                }
            //check if each pair is sorted by flag.  cargo > module
            } else if ((IsCargoHoldFlag(itemVec[i]->flag())) &&(IsModuleSlot(itemVec[i2]->flag()))) {
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
        _log(INV__TRACE, "Inventory::SortVector() - %lu items sorted in %.3fus with %u loops.", itemVec.size(),(GetTimeUSeconds() - start), count);

    return itemVec;  //returns sorted list
}

InventoryItemRef Inventory::GetByID(uint32 id) const {
    std::map<uint32, InventoryItemRef>::const_iterator res = mContents.find(id);
    if (res != mContents.end())
        return res->second;

    return InventoryItemRef(nullptr);
}

void Inventory::UpdateFlag(EVEItemFlags newFlag, InventoryItemRef iRef) const
{
    sLog.Warning("Inv::UpdateFlag", "this is used...finish code here");
    // this method is for changing flags for existing items in our inventory
    /* wont compile
    auto range = m_contentsByFlag.equal_range(iRef->flag());
    for (auto &cur = range.first; cur != range.second; ++cur ) {
        if (cur->second == iRef)
            m_contentsByFlag.erase(cur);
    }
    m_contentsByFlag.emplace(newFlag, iRef);
    */
}

// for stations only...can get expensive for stations that have many players loaded
//  maybe create an inventory map by owner in station?
void Inventory::GetInvForOwner(uint32 ownerID, std::vector< InventoryItemRef >& items)
{
    if (!IsOfficeID(m_myID) and !sDataMgr.IsStation(m_myID)) {
        _log(INV__ERROR, "GetInvForOwner called on non-station item %s(%u)", m_self->name(), m_myID);
        EvE::traceStack();
    }
	// could this use flagMap?
    for (auto &cur : mContents)
        if (cur.second->ownerID() == ownerID)
            items.push_back(cur.second);
}

	// could this use flagMap?
InventoryItemRef Inventory::FindFirstByFlag(EVEItemFlags flag) const {
    for (auto &cur : mContents)
        if (cur.second->flag() == flag)
            return cur.second;

        return InventoryItemRef(nullptr);
}

InventoryItemRef Inventory::GetByTypeFlag(uint32 typeID, EVEItemFlags flag) const {
    auto range = m_contentsByFlag.equal_range(flag);
    for (auto itr = range.first; itr != range.second; ++itr )
        if (itr->second->typeID() == typeID)
            return itr->second;

    return InventoryItemRef(nullptr);
}

void Inventory::GetInventoryMap( std::map< uint32, InventoryItemRef >& invMap ) {
    //  call copy assign.  should be faster than my previous code
    invMap = mContents;
}

uint32 Inventory::GetItemsByFlag(EVEItemFlags flag, std::vector<InventoryItemRef> &items) const {
    auto range = m_contentsByFlag.equal_range(flag);
    for (auto itr = range.first; itr != range.second; ++itr )
        items.push_back(itr->second);
    return items.size();
}

bool Inventory::GetTypesByFlag(EVEItemFlags flag, std::map< uint16, InventoryItemRef >& items)
{
    auto range = m_contentsByFlag.equal_range(flag);
    for (auto itr = range.first; itr != range.second; ++itr )
        items.emplace(itr->second->typeID(), itr->second);

    return (items.size() > 0);
}

InventoryItemRef Inventory::GetItemByTypeFlag(uint16 typeID, EVEItemFlags flag)
{
    std::vector<InventoryItemRef> items;
    if (GetItemsByFlag(flag, items) < 1)
        return InventoryItemRef(nullptr);

    for (auto &cur : items)
        if (cur->typeID() == typeID )
            return cur;

    return InventoryItemRef(nullptr);
}

bool Inventory::IsEmptyByFlag(EVEItemFlags flag) const {
    return (m_contentsByFlag.find(flag) == m_contentsByFlag.end());
}

uint32 Inventory::GetItemsByFlagRange(EVEItemFlags lowflag, EVEItemFlags highflag, std::vector<InventoryItemRef> &items) const
{
    // i dont yet see a better way to do this one...
    uint32 count(0);
    for (auto &cur : mContents)
        if (cur.second->flag() >= lowflag && cur.second->flag() <= highflag) {
            items.push_back(cur.second);
            ++count;
        }
    return count;
}

uint32 Inventory::GetItemsByFlagSet(std::set<EVEItemFlags> flags, std::vector<InventoryItemRef> &items) const
{
    // i dont yet see a better way to do this one...
    uint32 count = 0;
    for (auto &cur : mContents)
        if (flags.find(cur.second->flag()) != flags.end()) {
            items.push_back(cur.second);
            ++count;
        }
    return count;
}

bool Inventory::ContainsTypeQty(uint16 typeID, uint32 qty/*0*/) const
{
    uint32 count(0);
    for (auto &cur : mContents) {
        if (cur.second->typeID() == typeID ) {
            if (cur.second->quantity() >= qty) {
                return true;
            } else {
                count += cur.second->quantity();
            }
        }
    }

    return(count >= qty);
}

bool Inventory::ContainsTypeQtyByFlag(uint16 typeID, EVEItemFlags flag, uint32 qty) const
{
    uint32 count(0);
    std::vector<InventoryItemRef> itemVec;
    if (GetItemsByFlag(flag, itemVec) < 1)
        return false;

    for (auto &cur : itemVec) {
        if (cur->quantity() >= qty) {
            return true;
        } else {
            count += cur->quantity();
        }
    }

    return(count >= qty);
}

bool Inventory::ContainsTypeByFlag(uint16 typeID, EVEItemFlags flag) const
{
    std::vector<InventoryItemRef> itemVec;
    if (GetItemsByFlag(flag, itemVec) < 1)
        return false;
    for (auto &cur : itemVec)
        if (cur->typeID() == typeID)
            return true;
    return false;
}


void Inventory::StackAll(EVEItemFlags locFlag, uint32 ownerID/*0*/)
{
    InventoryItemRef iRef(nullptr);
    std::vector<InventoryItemRef> delVec;
    std::map<uint16, InventoryItemRef> types;
    std::map<uint16, InventoryItemRef>::iterator tItr = types.end();

    auto range = m_contentsByFlag.equal_range(locFlag);
    for (auto itr = range.first; itr != range.second; ++itr) {
        iRef = itr->second;
        // check to avoid removing modules(and their charges) from ship(crazy error)
        if (IsModuleSlot(iRef->flag()))
            continue;
        // singletons dont stack
        if (iRef->isSingleton())
            continue;
        if ((ownerID == 0) or(ownerID == iRef->ownerID())) {
            tItr = types.find(iRef->typeID());
            if (tItr == types.end()) {
                // insert type into map for later comparison(existing stack to merge into)
                types.emplace(iRef->typeID(), iRef);
            } else {
                // found another stack of this type.
                delVec.push_back(iRef);
                // fake merge as calling II::Merge() will invalidate iterator
                tItr->second->SetQuantity(tItr->second->quantity() + iRef->quantity(), true, false);
            }
        }
    }

    for (auto &cur : delVec)
        cur->Delete();
}

float Inventory::GetStoredVolume(EVEItemFlags flag, bool combined/*true*/) const {
    float totalVolume(0.0f);
    if (IsHangarFlag(flag) and combined) {
        for (auto &cur : mContents)
            if (IsHangarFlag(cur.second->flag()))
                totalVolume += cur.second->quantity() * cur.second->GetAttribute(AttrVolume).get_float();
    } else {
        auto range = m_contentsByFlag.equal_range(flag);
        for (auto itr = range.first; itr != range.second; ++itr )
            totalVolume += itr->second->quantity() * itr->second->GetAttribute(AttrVolume).get_float();
            // This formula is a hybrid of both old and new ones...and it works \o/
    }
    return totalVolume;
}

bool Inventory::HasAvailableSpace(EVEItemFlags flag, InventoryItemRef iRef) const {
    float capacity(GetRemainingCapacity(flag));
    float volume(iRef->quantity() * iRef->GetAttribute(AttrVolume).get_float());

    if (is_log_enabled(INV__CAPY))
        _log(INV__CAPY, "Inventory::HasAvailableSpace() - Testing %s's %s available capy of %.2f to add %u %s at %.2f(%.3f each)", \
                m_self->name(), sDataMgr.GetFlagName(flag), capacity, iRef->quantity(), iRef->name(), \
                volume, iRef->GetAttribute(AttrVolume).get_float());

    return(volume < capacity);
}

float Inventory::GetCapacity(EVEItemFlags flag) const {
    // added hangar capy for all hangar types
    // are we missing any hangar types here?  POS types?  yes...see next line
    /** @todo  finish these for POS */
    //   IsFlagCapacityLocationWide   item.groupID in(const.groupCorporateHangarArray, const.groupAssemblyArray, const.groupMobileLaboratory):

    switch( flag ) {
        case flagOffice:
        case flagProperty:
        //case flagDelivery:
        case flagImpounded:
        case flagCorpMarket:                    return maxHangarCapy;
        case flagAutoFit:
        case flagCargoHold:                     return m_self->GetAttribute(AttrCapacity).get_float();
        case flagDroneBay:                      return m_self->GetAttribute(AttrDroneCapacity).get_float();
        case flagShipHangar:                    return m_self->GetAttribute(AttrShipMaintenanceBayCapacity).get_float();
        case flagSecondaryStorage:              return m_self->GetAttribute(AttrSecondaryCapacity).get_float();
        case flagFuelBay:                       return m_self->GetAttribute(AttrFuelBayCapacity).get_float();
        case flagOreHold:                       return m_self->GetAttribute(AttrOreHoldCapacity).get_float();
        case flagGasHold:                       return m_self->GetAttribute(AttrGasHoldCapacity).get_float();
        case flagAmmoHold:                      return m_self->GetAttribute(AttrAmmoHoldCapacity).get_float();
        case flagShipHold:                      return m_self->GetAttribute(AttrShipHoldCapacity).get_float();
        case flagMineralHold:                   return m_self->GetAttribute(AttrMineralHoldCapacity).get_float();
        case flagSalvageHold:                   return m_self->GetAttribute(AttrSalvageHoldCapacity).get_float();
        case flagSmallShipHold:                 return m_self->GetAttribute(AttrSmallShipHoldCapacity).get_float();
        case flagMediumShipHold:                return m_self->GetAttribute(AttrMediumShipHoldCapacity).get_float();
        case flagLargeShipHold:                 return m_self->GetAttribute(AttrLargeShipHoldCapacity).get_float();
        case flagIndustrialShipHold:            return m_self->GetAttribute(AttrIndustrialShipHoldCapacity).get_float();
        // for PI
        case flagCommandCenterHold:             return m_self->GetAttribute(AttrCommandCenterHoldCapacity).get_float();
        case flagPlanetaryCommoditiesHold:      return m_self->GetAttribute(AttrPlanetaryCommoditiesHoldCapacity).get_float();
        // for pos battery/array
        case flagHiSlot0:                       return m_self->GetAttribute(AttrAmmoCapacity).get_float();
        // still not sure where/why this is used....
        case flagQuafeBay:                      return m_self->GetAttribute(AttrQuafeHoldCapacity).get_float();
        // these can be in station OR on ship....
        case flagCorpHangar2:
        case flagCorpHangar3:
        case flagCorpHangar4:
        case flagCorpHangar5:
        case flagCorpHangar6:
        case flagCorpHangar7: {
            if (sDataMgr.IsStation(m_myID))
                return maxHangarCapy;
            //for ship, this is TOTAL capy for all corp hangars(they share capy)
            if (m_self->HasAttribute(AttrHasCorporateHangars))
                return m_self->GetAttribute(AttrCorporateHangarCapacity).get_float();
        } break;
        case flagHangar: {
            if (sDataMgr.IsStation(m_myID))
                return maxHangarCapy;
            if (m_self->HasAttribute(AttrHasCorporateHangars))
                return m_self->GetAttribute(AttrCorporateHangarCapacity).get_float();
            //for cargo container, this is 27k5m3.
            return m_self->GetAttribute(AttrCapacity).get_float();
        }
    }

    _log(INV__WARNING, "Inventory::GetCapacity() - Unsupported flag %s(%u) called for %s(%u)", \
                sDataMgr.GetFlagName(flag), flag, m_self->name(), m_myID);
    return 0.0f;
}


bool Inventory::ValidateAddItem(EVEItemFlags flag, InventoryItemRef iRef) const
{
    // i dont think we need to check shit in stations...yet
    if (m_self->categoryID() == EVEDB::invCategories::Station)
        return true;

    // can this be coded to check weapon capy?   im sure it can. just a flag, right?

    float capacity = GetRemainingCapacity(flag);
    float volume = iRef->GetAttribute(AttrVolume).get_float();
    float totalVolume = iRef->quantity() * volume;

    _log(INV__CAPY, "Inventory::ValidateAddItem() - Testing %s's %s available capy of %.2f to add %i %s at %.2f(%.3f each)",
         m_self->name(), sDataMgr.GetFlagName(flag), capacity, iRef->quantity(), iRef->name(), totalVolume, volume);

    /** modify checks for splitting items in same container or moving items between a container's corp hangars
     * flag and iRef->flag() will be same(or same type).
     * add requested move volume to container's available capy before other checks
     *   this will show item being removed from container to allow subsequent additions
     */
    if (m_self->itemID() == iRef->locationID())
        if ((flag == iRef->flag())
        or (IsHangarFlag(flag) and IsHangarFlag(iRef->flag()))) {
            // possible item split.  will have to molest the shit outta this one to verify nullablity of exploits
            capacity += totalVolume;
            _log(INV__CAPY, "Inventory::ValidateAddItem() - flag() %s(%u) == iRef->flag() %s(%u) - test capacity changed to %.2f",
                    sDataMgr.GetFlagName(flag), flag, sDataMgr.GetFlagName(iRef->flag()), iRef->flag(), capacity);
        }

    //NOTE:  .AddAmount[type]() does not work correctly for these...

    // check capy for single unit
    if (capacity < volume) { // smallest volume is 0.0025
        if (IsCargoHoldFlag(flag)) {
            throw UserError("NotEnoughCargoSpace")
            .AddFormatValue("volume", new PyFloat(volume))
            .AddFormatValue("available", new PyFloat(capacity));
        } else if (flag == flagShipHangar) {
            throw UserError("NotEnoughCargoSpaceFor1Unit")
            .AddFormatValue("type", new PyInt(iRef->typeID()))
            .AddFormatValue("required", new PyFloat(volume))
            .AddFormatValue("free", new PyFloat(capacity));
        } else if (IsSpecialHoldFlag(flag)) {
            throw UserError("NotEnoughSpecialBaySpaceOverload")
            .AddFormatValue("item", new PyString(iRef->itemName()))
            .AddFormatValue("maximum", new PyFloat(GetCapacity(flag)))
            .AddFormatValue("used", new PyFloat(GetStoredVolume(flag)));
        } else if (IsModuleSlot(flag)) {
            throw UserError("NotEnoughChargeSpace")
            .AddFormatValue("volume", new PyFloat(totalVolume))
            .AddFormatValue("capacity", new PyFloat(capacity));
        } else if (IsHangarFlag(flag)) {
            throw UserError("NotEnoughSpaceOverload")
            .AddFormatValue("item", new PyString(iRef->itemName()))
            .AddFormatValue("maximum", new PyFloat(GetCapacity(flag)))
            .AddFormatValue("used", new PyFloat(GetStoredVolume(flag)));
        } else if (flag == flagDroneBay) {
            throw UserError("NotEnoughDroneBaySpaceOverload")
            .AddFormatValue("item", new PyString(iRef->itemName()))
            .AddFormatValue("maximum", new PyFloat(GetCapacity(flag)))
            .AddFormatValue("used", new PyFloat(GetStoredVolume(flag)));
        } else {
            throw UserError("NoSpaceForThatOverload")
            .AddFormatValue("item", new PyInt(iRef->itemID()))
            .AddFormatValue("maximum", new PyFloat(GetCapacity(flag)))
            .AddFormatValue("used", new PyFloat(GetStoredVolume(flag)));
        }
        return false;
    }

    // check capy for all units
    if (totalVolume > capacity) {
        if (IsSpecialHoldFlag(flag)) {
            throw UserError("NotEnoughSpecialBaySpaceOverload")
            .AddFormatValue("item", new PyString(iRef->itemName()))
            .AddFormatValue("maximum", new PyFloat(GetCapacity(flag)))
            .AddFormatValue("used", new PyFloat(GetStoredVolume(flag)));
        } else if (flag == flagDroneBay) {
            throw UserError("NotEnoughDroneBaySpaceOverload")
            .AddFormatValue("item", new PyString(iRef->itemName()))
            .AddFormatValue("maximum", new PyFloat(GetCapacity(flag)))
            .AddFormatValue("used", new PyFloat(GetStoredVolume(flag)));
        } else if (IsHangarFlag(flag)) {
            throw UserError("NotEnoughCargoSpaceOverload")
            .AddFormatValue("item", new PyString(iRef->itemName()))
            .AddFormatValue("maximum", new PyFloat(GetCapacity(flag)))
            .AddFormatValue("used", new PyFloat(GetStoredVolume(flag)));
        } else if (IsCargoHoldFlag(flag)) {
            throw UserError("NotEnoughCargoSpace")
            .AddFormatValue("volume", new PyFloat(totalVolume))
            .AddFormatValue("available", new PyFloat(capacity));
        } else {
            throw UserError("NoSpaceForThat")
            .AddFormatValue("itemTypeName", new PyInt(iRef->typeID()))
            .AddFormatValue("itemVolume", new PyFloat(totalVolume))
            .AddFormatValue("volumeAvailable", new PyFloat(capacity));
        }
        return false;
    }

    return true;
}

// multimerge - NotEnoughCargoSpace', 'NotEnoughCargoSpaceOverload', 'NotEnoughDroneBaySpace', 'NotEnoughDroneBaySpaceOverload',
// 'NoSpaceForThat', 'NoSpaceForThatOverload', 'NotEnoughChargeSpace'):

// add - NotEnoughCargoSpace', 'NotEnoughCargoSpaceOverload', 'NotEnoughDroneBaySpace', 'NotEnoughDroneBaySpaceOverload',
// 'NoSpaceForThat', 'NoSpaceForThatOverload', 'NotEnoughChargeSpace', 'NotEnoughSpecialBaySpace', 'NotEnoughSpecialBaySpaceOverload',
//  'NotEnoughSpace'):
