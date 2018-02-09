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
    Author:     Zhur, Captnoord
    Updates:    Allan
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "EVEServerConfig.h"
#include "StaticDataMgr.h"
#include "inventory/InventoryBound.h"
#include "pos/Structure.h"
#include "system/BookmarkDB.h"
#include "system/Container.h"
#include "system/SystemManager.h"

PyCallable_Make_InnerDispatcher(InventoryBound)

InventoryBound::InventoryBound( PyServiceMgr* mgr, InventoryItemRef item, EVEItemFlags flag, uint32 ownerID, bool passive)
: PyBoundObject(mgr),
m_dispatch(new Dispatcher(this)),
pInventory(item->GetMyInventory()),
mFlag(flag),
m_self(item),
m_itemID(item->itemID()),
m_ownerID(ownerID),
m_passive(passive)
{
    _SetCallDispatcher(m_dispatch);


    m_strBoundObjectName = "InventoryBound";

    PyCallable_REG_CALL(InventoryBound, List);
    PyCallable_REG_CALL(InventoryBound, Add);
    PyCallable_REG_CALL(InventoryBound, MultiAdd);
    PyCallable_REG_CALL(InventoryBound, GetItem);
    PyCallable_REG_CALL(InventoryBound, ListStations);
    PyCallable_REG_CALL(InventoryBound, ReplaceCharges);
    PyCallable_REG_CALL(InventoryBound, MultiMerge);
    PyCallable_REG_CALL(InventoryBound, StackAll);
    PyCallable_REG_CALL(InventoryBound, StripFitting);
    PyCallable_REG_CALL(InventoryBound, DestroyFitting);
    PyCallable_REG_CALL(InventoryBound, SetPassword);
    PyCallable_REG_CALL(InventoryBound, CreateBookmarkVouchers);
    PyCallable_REG_CALL(InventoryBound, RunRefiningProcess);
    PyCallable_REG_CALL(InventoryBound, Voucher);

    _log(INV__BIND, "Created InventoryBound object %p for %s(%u) and ownerID %u with flag %s  (passive: %s)", \
            this, m_self->itemName().c_str(), m_itemID, ownerID, sDataMgr.GetFlagName(flag).c_str(), (m_passive ? "true" : "false"));
}

InventoryBound::~InventoryBound()
{
    delete m_dispatch;
}

PyResult InventoryBound::Handle_GetItem(PyCallArgs &call) {
    _log(INV__MESSAGE, "Calling InventoryBound::GetItem() for %s(%u)", m_self->itemName().c_str(), m_itemID);
    return m_self->GetItem();
}

PyResult InventoryBound::Handle_ListStations( PyCallArgs& call )
{
    _log(INV__MESSAGE, "Calling InventoryBound::ListStations() for %s(%u)", m_self->itemName().c_str(), m_itemID);

    util_Rowset rowset;

    rowset.header.push_back( "stationID" );
    rowset.header.push_back( "itemCount" );

    return rowset.Encode();
}

PyResult InventoryBound::Handle_SetPassword(PyCallArgs &call) {
    _log(INV__MESSAGE, "Calling InventoryBound::SetPassword() for %s(%u)", m_self->itemName().c_str(), m_itemID);
    call.Dump(INV__DUMP);
    return nullptr;
}

PyResult InventoryBound::Handle_StripFitting(PyCallArgs &call)
{
    call.client->GetShip()->StripFitting();
    return nullptr;
}

PyResult InventoryBound::Handle_DestroyFitting(PyCallArgs &call) {
    _log(INV__MESSAGE, "Calling InventoryBound::DestroyFitting() for %s(%u)", m_self->itemName().c_str(), m_itemID);
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)){
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
    }

    InventoryItemRef item = sItemFactory.GetItem(args.arg);
    call.client->GetShip()->RemoveRig(item);

    return nullptr;
}


PyResult InventoryBound::Handle_List(PyCallArgs &call) {
    uint32 ownerID = m_ownerID;
    // this item was originally boudn to this flag, but can send specific flag on rare occasions...not sure of criteria
    EVEItemFlags flag = mFlag, oldFlag = mFlag;
    if (call.byname.find("flag") != call.byname.end())
        flag = (EVEItemFlags)PyRep::IntegerValue(call.byname.find("flag")->second);

    if (call.tuple->size() > 0) {
        Call_List arg;
        if (!arg.Decode(&call.tuple))
            _log(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        if (flag != arg.flag)
            flag = (EVEItemFlags)arg.flag;
    }

    // check for owner type of this inventory for reference checks
    if (IsOffice(m_itemID)) {
        // office owned by corp in station
        // check for owner or corp
        if (call.client->GetCorporationID() != m_ownerID)
            ; // calling char is not member of corp.  send error?
    } else if (IsPlayerItem(m_itemID)) {
        // this is probably a ship calling for all items
        flag = flagAnywhere;
        /*
    } else if (IsPlayerCorp(m_itemID)) {
        // this one probably will not be used.
        //  what items in a corp would be listed?  corpItem dont have inventory
    } else if (IsCharacter(m_itemID)) { // this is chccked in Inventory::List()
        // this is asking for skill list...char is a container for skills
        flag = flagAnywhere;
    } else if (IsSolarSystem(m_itemID)) {
        //  not sure how to do this one...will have to check on WHEN system listing would be called
        */
    } else if (IsStation(m_itemID)) {
        // this will get owners items only, including corps

    } else if (IsControlBunker(m_itemID)) {
        // not sure what this is yet

    }
/*
    if (IsCargoHoldFlag(flag)) {
        // check for owner or corpID

    } else if (IsHangarFlag(flag)) {
        // check for owner or corpID

        flag = flagAnywhere;
        if (call.client->GetCorporationID() != m_ownerID)
            ; // calling char is not member of corp.  send error?
    } else if (IsOfficeFlag(flag)) {
        // flags for npc station containers, owned by station, but items owned by corp
        //  this is market deliveries, impounded, etc.
        // check for corpID

        flag = flagAnywhere;
        if (call.client->GetCorporationID() != m_ownerID)
            ; // calling char is not member of corp.  send error?
    }
*/
    _log(INV__MESSAGE, "InventoryBound::List() called by %s with ownerID %u for %s(%u:%s%s) - origFlag: %s", \
                call.client->GetName(), ownerID, m_self->itemName().c_str(), m_itemID, sDataMgr.GetFlagName(flag).c_str(), (m_passive ? ":passive" : ""), \
                sDataMgr.GetFlagName(oldFlag).c_str());

    return pInventory->List(flag, ownerID);
}

PyResult InventoryBound::Handle_ReplaceCharges(PyCallArgs &call) {
    _log(INV__MESSAGE, "Calling InventoryBound::ReplaceCharges() for %s(%u)", m_self->itemName().c_str(), m_itemID);
    Inventory_CallReplaceCharges args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    //validate flag.
    if (!IsModuleSlot(args.flag)) {
        _log(INV__ERROR, "%s: Invalid flag %i", call.client->GetName(), args.flag);
        return nullptr;
    }

    // returns new ref
    InventoryItemRef iRef = pInventory->GetByID( args.itemID );
    if (iRef.get() == nullptr) {
        _log(INV__ERROR, "%s: Unable to find charge %i", call.client->GetName(), args.itemID);
        return nullptr;
    }

    if ((iRef->ownerID() != call.client->GetCharacterID())
    or (iRef->ownerID() != call.client->GetCorporationID())) {
        _log(INV__ERROR, "Character %u tried to load charge %u of character %u.", call.client->GetCharacterID(), iRef->itemID(), iRef->ownerID());
        return nullptr;
    }

    if (iRef->quantity() < args.quantity) {
        _log(INV__WARNING, "%s: Item %u: Requested quantity (%i) exceeds actual quantity (%i), using actual.", call.client->GetName(), args.itemID, args.quantity, iRef->quantity());
    } else if (iRef->quantity() > args.quantity) {
        iRef = iRef->Split(args.quantity);  // split item and get new item reference
        if (iRef.get() == nullptr) {
            _log(INV__ERROR, "%s: Unable to split charge %i into %i", call.client->GetName(), args.itemID, args.quantity);
            return nullptr;
        }
    }

    call.client->GetShip()->ReplaceCharges( (EVEItemFlags)args.flag, iRef );

    return new PyInt(1);
}

PyResult InventoryBound::Handle_CreateBookmarkVouchers(PyCallArgs &call) {
    /*
    bookmarksDeleted, newVouchers = self.CreateBookmarkVouchers(bookmarkIDs, flag, isMove)
    */
    sLog.White( "InventoryBound::Handle_CreateBookmarkVouchers()", "size= %u", call.tuple->size() );
    Call_CreateVouchers args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }
    args.Dump(COMMON__INFO);

    // CanOnlyCreateVoucherInPersonalHangar     << dunno if i wanna pull an iRef JUST for this check....make voucher itemID group?
    // m_self is *this itemRef

    /**
     * 00:39:12 [SvcCall]   Call Arguments:
     * 00:39:12 [SvcCall]       Tuple: 3 elements
     * 00:39:12 [SvcCall]         [ 0] List: 1 elements
     * 00:39:12 [SvcCall]         [ 0]   [ 0] Integer field: 10    -this is bookmarkID(s)
     * 00:39:12 [SvcCall]         [ 1] Integer field: 4            -flag  (?)
     * 00:39:12 [SvcCall]         [ 2] Boolean field: true         -IsMove
     * 00:39:12 [SvcCall]   Call Named Arguments:
     * 00:39:12 [SvcCall]     Argument 'machoVersion':
     * 00:39:12 [SvcCall]         Integer field: 1
     * 00:39:13 L InventoryBound::Handle_CreateBookmarkVouchers(): 1 Vouchers created
     *
     * 00:43:37 [SvcCall]   Call Arguments:
     * 00:43:37 [SvcCall]       Tuple: 3 elements
     * 00:43:37 [SvcCall]         [ 0] List: 5 elements
     * 00:43:37 [SvcCall]         [ 0]   [ 0] Integer field: 4
     * 00:43:37 [SvcCall]         [ 0]   [ 1] Integer field: 6
     * 00:43:37 [SvcCall]         [ 0]   [ 2] Integer field: 7
     * 00:43:37 [SvcCall]         [ 0]   [ 3] Integer field: 11
     * 00:43:37 [SvcCall]         [ 0]   [ 4] Integer field: 2
     * 00:43:37 [SvcCall]         [ 1] Integer field: 4
     * 00:43:37 [SvcCall]         [ 2] Boolean field: true
     * 00:43:37 [SvcCall]   Call Named Arguments:
     * 00:43:37 [SvcCall]     Argument 'machoVersion':
     * 00:43:37 [SvcCall]         Integer field: 1
     * 00:43:37 L InventoryBound::Handle_CreateBookmarkVouchers(): 5 Vouchers created
     */

    PyList* vouchers = new PyList();
    PyList* deletedIDs = new PyList();

    uint32 locationID = call.client->GetLocationID();
    if (args.flag == flagCargoHold)
        locationID = call.client->GetShipID();

    if ( args.bmIDs->size() < 1 ) {
        sLog.Error( "InventoryBound::Handle_CreateBookmarkVouchers()", "%s: args.bmIDs->size() == 0.  Expected size > 0.", call.client->GetName() );
    } else {
        PyList::const_iterator itr = args.bmIDs->begin();
        for (; itr != args.bmIDs->end(); ++itr) {
            //ItemData ( typeID, ownerID, locationID, flag, quantity, customInfo, contraband)
            ItemData iData( 51, call.client->GetCharacterID(), 0, flagAutoFit, 1, itoa((*itr)->AsInt()->value()));
            InventoryItemRef iRef = sItemFactory.SpawnItem( iData );
            if (iRef.get() == nullptr) {
                codelog(ITEM__ERROR, "%s: Failed to spawn bookmark voucher for bmID %u", call.client->GetName(), (*itr)->AsInt()->value());
                continue;
            }
            //iRef->Rename(itoa(BookmarkDB::GetBookmarkName((*itr)->AsInt()->value())));
            iRef->Move(locationID, (EVEItemFlags)args.flag, true);
            vouchers->AddItem(iRef->ItemGetInfo());
            if (args.isMove)
                deletedIDs->AddItem(new PyInt((*itr)->AsInt()->value()));
        }
    }

    //  when bm is copied to another players places tab, copy data from db using bookmarkID stored in ItemData.customInfo

    call.client->SendInfoModalMsg("Creating Vouchers from Bookmarks isn't complete.  You will have to dock or relog to show the container inventory.");

    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, deletedIDs);
        tuple->SetItem(1, vouchers);
    return tuple;
}

PyResult InventoryBound::Handle_RunRefiningProcess(PyCallArgs &call){
    _log(INV__MESSAGE, "Calling InventoryBound::RunRefiningProcess() for %s(%u)", m_self->itemName().c_str(), m_itemID);
    sLog.White( "InventoryBound::Handle_RunRefiningProcess()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
    return nullptr;
}

PyResult InventoryBound::Handle_Voucher(PyCallArgs &call){
    _log(INV__MESSAGE, "Calling InventoryBound::Voucher() for %s(%u)", m_self->itemName().c_str(), m_itemID);
    sLog.White( "InventoryBound::Handle_Voucher()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
    return nullptr;
}

PyResult InventoryBound::Handle_MultiMerge(PyCallArgs &call) {
    _log(INV__MESSAGE, "Calling InventoryBound::MultiMerge() for %s(%u)", m_self->itemName().c_str(), m_itemID);
    //Decode Args
    Inventory_CallMultiMerge args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    Inventory_CallMultiMergeElement element;

    sItemFactory.SetUsingClient(call.client);
    std::vector<PyRep *>::const_iterator cur = args.MMElements->begin();
    for (; cur != args.MMElements->end(); ++cur) {
        if (!element.Decode( *cur )) {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
            continue;
        }

        InventoryItemRef stationaryItem = sItemFactory.GetItem( element.stationaryItemID );
        if (stationaryItem.get() == nullptr) {
            _log(INV__WARNING, "Failed to load stationary item %u. Skipping.", element.stationaryItemID);
            continue;
        }

        InventoryItemRef draggedItem = sItemFactory.GetItem( element.draggedItemID );
        if (draggedItem.get() == nullptr) {
            _log(INV__WARNING, "Failed to load dragged item %u. Skipping.", element.draggedItemID);
            continue;
        }

        if (sItemFactory.GetItemContainerInventory(stationaryItem->itemID())->ValidateAddItem(stationaryItem->flag(), draggedItem)) {
            // this shits not right.....
            draggedItem->ChangeOwner(m_ownerID);
            stationaryItem->Merge( draggedItem, element.draggedQty );
        } // if false, error is thrown in ValidateAddItem() call
    }
    sItemFactory.UnsetUsingClient();

    return nullptr;
}

PyResult InventoryBound::Handle_StackAll(PyCallArgs &call) {
    EVEItemFlags stackFlag = mFlag;

    if (call.tuple->items.size() != 0) {
        Call_SingleIntegerArg arg;
        if (!arg.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
            return nullptr;
        }

        stackFlag = (EVEItemFlags)arg.arg;
    }

    _log(INV__MESSAGE, "Calling InventoryBound::StackAll() for %s(%u) with flag %s", m_self->itemName().c_str(), m_itemID, sDataMgr.GetFlagName(stackFlag).c_str());

    //Stack Items contained in this inventory
    pInventory->StackAll(stackFlag, m_ownerID);

    return nullptr;
}

/* this call is used for moving an item to *THIS* inventory
 * Moving items to/from containers
 * Removing Module/Charges from ship (using 'remove' button on item slot)
 * Adding Modules to a specific slot on ship
 */
PyResult InventoryBound::Handle_Add(PyCallArgs &call) {
    if (is_log_enabled(INV__DUMP)) {
        _log(INV__DUMP, "InventoryBound::Handle_Add() size= %u", call.tuple->size());
        call.Dump(INV__DUMP);
    }

    if (call.tuple->items.size() != 2) {
        _log(INV__ERROR, "InventoryBound::Handle_Add()  Unexpected number of elements in tuple: %u (should be 2).", call.tuple->items.size() );
        return nullptr;
    }

    Call_Add_2 args;    // item and location
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    uint16 toFlag = mFlag;
    if (call.byname.find("flag") != call.byname.end())
        toFlag = PyRep::IntegerValue(call.byname.find("flag")->second);
    if (toFlag == flagLocked) {
        // corp role 'equip config' can move locked items (per client)
        _log(INV__MESSAGE, "InventoryBound::Handle_Add() - item %u from %u sent flagLocked.", args.itemID, args.containerID);
        toFlag = flagCargoHold;
    }

    InventoryItemRef iRef = sItemFactory.GetItem(args.itemID);

    bool manyFlags = false;
    int32 quantity = 0;
    if (call.byname.find("qty") != call.byname.end())
        quantity = PyRep::IntegerValue(call.byname.find("qty")->second);

    if (call.byname.find("dividing") != call.byname.end()) {
        // split stack, move original ref, leave remainder here with new item
        InventoryItemRef newItem = iRef->Split(iRef->quantity() -quantity);
        if (newItem.get() == nullptr) {
            _log(INV__ERROR, "InventoryBound::Handle_Add() - Error splitting item %u. Skipping.", iRef->itemID());
            return nullptr;
        }
        iRef = newItem;
        args.itemID = iRef->itemID();
    // we're not dividing the stack, so check for removing loaded charges
    } else if ((iRef->categoryID() == EVEDB::invCategories::Charge) and (IsModuleSlot(toFlag)))
        manyFlags = true;

    float capacity = 0.0f;
    if (call.byname.find("capacity") != call.byname.end())
        capacity = PyRep::IntegerValue(call.byname.find("capacity")->second);

    if (quantity < 1)
        quantity = 1;

    _log(INV__MESSAGE, "InventoryBound::Handle_Add() - moving %u of item %u from (%u:%s) to me(%s:%u:%s).", \
            quantity, args.itemID, args.containerID, sDataMgr.GetFlagName(iRef->flag()).c_str(), m_self->itemName().c_str(), m_itemID, sDataMgr.GetFlagName(toFlag).c_str());

    std::vector<int32> items;
    items.push_back(args.itemID);

    return MoveItems(call.client, items, (EVEItemFlags)toFlag, quantity, manyFlags, capacity);
}

// this call is for moving items to *THIS* inventory
PyResult InventoryBound::Handle_MultiAdd(PyCallArgs &call) {
    if (is_log_enabled(INV__DUMP)) {
        _log(INV__DUMP, "InventoryBound::Handle_MultiAdd() size= %u", call.tuple->size());
        call.Dump(INV__DUMP);
    }

    if (call.tuple->items.size() != 2) {
        _log(INV__ERROR, "InventoryBound::Handle_MultiAdd()  Unexpected number of elements in tuple: %u (should be 2).", call.tuple->items.size() );
        return nullptr;
    }

    Call_MultiAdd_2 args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    uint16 toFlag = mFlag;
    if (call.byname.find("flag") != call.byname.end())
        toFlag = PyRep::IntegerValue(call.byname.find("flag")->second);

    int32 quantity = 1;
    if (call.byname.find("qty") != call.byname.end())
        quantity = PyRep::IntegerValue(call.byname.find("qty")->second);

    //bool byname(fromManyFlags):true == unload charges from module referenced
    bool manyFlags = false;
    if (call.byname.find("fromManyFlags") != call.byname.end())
        if (!call.byname.find("fromManyFlags")->second->IsNone())
            manyFlags = true;

    float capacity = 0.0f;
    if (call.byname.find("capacity") != call.byname.end())
        capacity = PyRep::IntegerValue(call.byname.find("capacity")->second);

    if (capacity > 1)
        manyFlags = true;
    else if (quantity < 1)
        manyFlags = true;

    // moving TO hangar...move all items in stack, if applicable
    if (IsHangarFlag(toFlag))
        manyFlags = true;

    _log(INV__MESSAGE, "InventoryBound::Handle_MultiAdd() - moving %u items from (%u:%s) to me(%s:%u:%s).", \
                args.itemIDs.size(), args.containerID, sDataMgr.GetFlagName(mFlag).c_str(), m_self->itemName().c_str(), m_itemID, sDataMgr.GetFlagName(toFlag).c_str());

    return MoveItems( call.client, args.itemIDs, (EVEItemFlags)toFlag, quantity, manyFlags, capacity);
}

PyRep* InventoryBound::MoveItems(Client* pClient, std::vector< int32 >& items, EVEItemFlags toFlag, int32 quantity, bool manyFlags, float capacity)
{   // complete method rewrite -allan 21Dec17
    InventoryItemRef contRef(nullptr);
    ShipItem* pShip = pClient->GetShip().get();
    bool donating = false, ship = false;
    int32 origQty = quantity;

    // we will need to check *this for specific item-moving rules
    switch (m_self->categoryID()) {
        // specific container-type categories that may move items in/out
        case EVEDB::invCategories::Trading: {
            // this shouldnt hit.  trading is handled in separate system
            codelog(INV__ERROR, "InventoryBound::MoveItems() - Trading Category called.");
            EvE::traceStack();
        } break;
        case EVEDB::invCategories::Structure: {
            // this is all POS groups.  use corp donating checks
            donating = true;
            // may have to reset flags based on type
            switch (m_self->groupID()) {
                case EVEDB::invGroups::Control_Tower: {
                    // flag 0 = fuel bay, flag = 122 stronium bay (2nd storage)
                } break;
                case EVEDB::invGroups::Mobile_Missile_Sentry:
                case EVEDB::invGroups::Mobile_Projectile_Sentry:
                case EVEDB::invGroups::Mobile_Laser_Sentry:
                case EVEDB::invGroups::Mobile_Hybrid_Sentry: {
                } break;
                case EVEDB::invGroups::Electronic_Warfare_Battery:
                case EVEDB::invGroups::Sensor_Dampening_Battery:
                case EVEDB::invGroups::Stasis_Webification_Battery:
                case EVEDB::invGroups::Warp_Scrambling_Battery:
                case EVEDB::invGroups::Energy_Neutralizing_Battery:
                case EVEDB::invGroups::Target_Painting_Battery: {
                } break;
                case EVEDB::invGroups::Refining_Array:
                case EVEDB::invGroups::Ship_Maintenance_Array:
                case EVEDB::invGroups::Assembly_Array:
                case EVEDB::invGroups::Shield_Hardening_Array:
                case EVEDB::invGroups::Corporate_Hangar_Array:
                case EVEDB::invGroups::Stealth_Emitter_Array:
                case EVEDB::invGroups::Scanner_Array:
                case EVEDB::invGroups::Logistics_Array:
                case EVEDB::invGroups::Cynosural_Generator_Array:
                case EVEDB::invGroups::Structure_Repair_Array: {
                } break;
                default: {
                } break;
            }
        } break;
        case EVEDB::invCategories::Station: {
            switch (m_self->groupID()) {
                case EVEDB::invGroups::Station: {
                    // standard station hangar
                    if ((toFlag == flagCorpMarket)
                    or (toFlag == flagImpounded)
                    or (toFlag == flagDelivery)) {
                        donating = true;
                    } else if (toFlag == flagLocked) {
                        ; // do tests here for moving locked items (corp role config and ??)
                    }
                } break;
                case EVEDB::invGroups::Station_Services: {
                    // station offices, office and factory folders.
                    if (m_self->typeID() == EVEDB::invTypes::typeOffice) //office.  use corp donating checks
                        donating = true;
                } break;
            }
        } break;
        case EVEDB::invCategories::Orbitals: {
            //may not have to do anything here.....test it
            /*
            if ((m_self->typeID() == EVEDB::invTypes::typePlanetaryCustomsOffice)
            or (m_self->typeID() == EVEDB::invTypes::typeInterbusCustomsOffice)
            or (m_self->typeID() == EVEDB::invTypes::typePlanetaryOfficeGantry))
                donating = true;
            */
        } break;
        case EVEDB::invCategories::Celestial: {
            // containers, wrecks, construction platform, station improve/upgrade platform,
            if (IsPlayerCorp(m_ownerID))
                donating = true;
        } break;
        case EVEDB::invCategories::Ship: {
            // do module checks for these items
            ship = true;
        } break;
    }

    EVEItemFlags old_flag(flagAutoFit);
    InventoryItemRef iRef(nullptr);
    sItemFactory.SetUsingClient(pClient);

    std::vector<int32>::const_iterator cur = items.begin();
    for (; cur != items.end(); ++cur) {
        quantity = origQty;
        iRef = sItemFactory.GetItem(*cur);
        if (iRef.get() == nullptr) {
            _log(INV__ERROR, "InventoryBound::MoveItems() - item %i not found.  continuing.", (*cur));
            continue;
        }
        //ALL items *should* have a loaded container item.
        contRef = sItemFactory.GetItemContainer(*cur); // item container should be loaded at this point.
        if (contRef.get() == nullptr) {
            _log(INV__ERROR, "InventoryBound::MoveItems() - container for item %i not found.  continuing.", (*cur));
            continue;
        }

        old_flag = iRef->flag();

        if (manyFlags)
            quantity = iRef->quantity();

        // if client send capy, check here and throw on fail
        if (capacity > 0) {
            if (quantity < 1)
                quantity = iRef->quantity();    // assume all.

            float volume = quantity * iRef->GetAttribute(AttrVolume).get_float();
            if (volume > capacity) {
                std::map<std::string, PyRep *> args;
                args["available"] = new PyFloat(capacity);
                args["volume"] = new PyFloat(volume);
                if (toFlag == flagCargoHold)
                    throw PyException(MakeUserError("NotEnoughCargoSpace", args));
                else if (toFlag == flagDroneBay)
                    throw PyException(MakeUserError("NotEnoughDroneBaySpace", args));
                else if (IsModuleSlot(toFlag))
                    throw PyException(MakeUserError("NotEnoughChargeSpace", args));
                else
                    throw PyException(MakeUserError("NoSpaceForThat", args));
            }
        } else if (quantity < 1) {
            _log(INV__ERROR, "InventoryBound::MoveItems() - Quantity < 1.  Setting quantity = 1.");
            quantity = 1;
        }

        // remove item from current location
        if (IsRigSlot(old_flag)) { //  cant remove rigs like this.  send error.
            throw PyException(MakeUserError("CannotRemoveUpgradeManually"));
        } else if (IsModuleSlot(old_flag)) {
            // can we remove modules from an inative ship?  no.
            if (pShip == nullptr)
                throw PyException( MakeCustomError("Ship not found. The %s wasnt moved.  Ref: ServerError 63290", iRef->itemName().c_str()));

            if (IsModuleSlot(toFlag)) {
                // we are wanting to change slots on a fitted module.
                pShip->MoveModuleSlot(old_flag, toFlag);
                Call_SingleIntegerArg result;
                result.arg = iRef->itemID();
                return result.Encode();
            } else {
                pShip->RemoveItem(iRef);
            }
        } else {
            if (!manyFlags and (quantity < iRef->quantity())) {
                InventoryItemRef newItem = iRef->Split(quantity);
                if (newItem.get() == nullptr) {
                    _log(INV__ERROR, "InventoryBound::MoveItems() - Error splitting item %u. Skipping.", iRef->itemID());
                    continue;
                }
                iRef = newItem;
                if (iRef.get() == nullptr) {
                    _log(INV__ERROR, "InventoryBound::MoveItems() - Error getting split item. Skipping.");
                    continue;
                }
                if (iRef->quantity() > quantity)
                    _log(INV__ERROR, "InventoryBound::MoveItems() - Split item %u qty(%u) > requested qty of %u.  Continuing.", iRef->itemID(), iRef->quantity(), quantity);
            }

            contRef->RemoveItem(iRef);
        }

        // add item to new location
        if (donating) {
            pInventory->ValidateAddItem(toFlag, iRef);  // this will throw if it fails
            iRef->Donate(m_ownerID, m_itemID, toFlag);
            continue;
        }

        if (ship) {
            // are we adding module to ship using autoFit?
            if (toFlag == flagAutoFit) {
                assert(iRef->categoryID() != EVEDB::invCategories::Charge); // crash here...this should NOT happen.
                if (iRef->categoryID() == EVEDB::invCategories::Module) {
                    toFlag = pShip->FindAvailableModuleSlot(iRef);
                    if (toFlag == flagIllegal) {
                        pClient->SendNotifyMsg("Your ship has no avalible slots to fit this module.  Putting the %u in your CargoHold.", iRef->itemName().c_str());
                        toFlag = flagCargoHold;
                    }
                } else {
                    toFlag = mFlag;
                }
            }

            if (iRef->categoryID() == EVEDB::invCategories::Module)
                m_self->GetShipItem()->TryModuleLimitChecks(toFlag, iRef); // this will throw if it fails

            if (IsCargoHoldFlag(toFlag))
                m_self->GetShipItem()->TryHoldCapacity(toFlag, iRef); // this will throw if it fails
            // check adding item to ship...if it fails, return to previous container
            if (m_self->GetShipItem()->AddItem(toFlag, iRef) < 1)
                contRef->AddItem(iRef);
            else
                iRef->ChangeOwner(m_ownerID);
        } else {
            pInventory->ValidateAddItem(toFlag, iRef);  // this will throw if it fails...how do we return the item if this fails?
            iRef->Donate(m_ownerID, m_itemID, toFlag);
        }
    }

    sItemFactory.UnsetUsingClient();

    if (items.size() == 1) {
        //call returns itemID for single-item adds
        Call_SingleIntegerArg result;
        result.arg = iRef->itemID();
        return result.Encode();
    }

    return nullptr;
}
