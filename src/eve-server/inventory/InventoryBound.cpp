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
#include "inventory/InventoryBound.h"
#include "system/SystemManager.h"

PyCallable_Make_InnerDispatcher(InventoryBound)

InventoryBound::InventoryBound( PyServiceMgr *mgr, InventoryItemRef item, EVEItemFlags flag)
: PyBoundObject(mgr),
m_dispatch(new Dispatcher(this)),
mInventory(item->GetInventory()),
mFlag(flag),
m_self(item)
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
    PyCallable_REG_CALL(InventoryBound, Voucher);
}

InventoryBound::~InventoryBound()
{
    delete m_dispatch;
}

PyResult InventoryBound::Handle_GetItem(PyCallArgs &call) {
    _log(INV__MESSAGE, "Calling InventoryBound::GetItem() for %s(%u)", m_self->itemName().c_str(), m_self->itemID());
    return m_self->GetItem();
}

PyResult InventoryBound::Handle_List(PyCallArgs &call) {
    /** @todo (allan)  this does not work for POS cargohold (flag 5) */
    /*
     *
     *    --found in starModeHandler.py:338 (ColorStarsByCargoIllegality)
        inv = invCache.GetInventoryFromId(activeShipID, locationID=session.stationid2)
        shipCargo = inv.List()

        */

    /*  this is a start for determining if char is in corp, corp is owner of items, and/or what ownerID to send to List()
     *
     *    InventoryItemRef rItem = call.client->services().item_factory->GetItem(mInventory.inventoryID());
     *    if (rItem.get()->categoryID() == EVEDB::invCategories::Structure)
     *        return mInventory->List( 5, call.client->GetCharacterID());
     *    else
     */

    /** @todo make sure we are allowed to list this inventory */
    _log(INV__MESSAGE, "Calling InventoryBound::List() for %s(%u)", m_self->itemName().c_str(), m_self->itemID());
    return mInventory->List( mFlag, call.client->GetCharacterID() );
}

PyResult InventoryBound::Handle_ReplaceCharges(PyCallArgs &call) {
    _log(INV__MESSAGE, "Calling InventoryBound::ReplaceCharges() for %s(%u)", m_self->itemName().c_str(), m_self->itemID());
    Inventory_CallReplaceCharges args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Unable to decode arguments");
        return nullptr;
    }

    //validate flag.
    if (args.flag < flagSlotFirst || args.flag > flagSlotLast) {
        _log(INV__ERROR, "%s: Invalid flag %d", call.client->GetName(), args.flag);
        return nullptr;
    }

    // returns new ref
    InventoryItemRef new_charge = mInventory->GetByID( args.itemID );
    if ( !new_charge ) {
        _log(INV__ERROR, "%s: Unable to find charge %d", call.client->GetName(), args.itemID);
        return nullptr;
    }

    if (new_charge->ownerID() != call.client->GetCharacterID()) {
        _log(INV__ERROR, "Character %u tried to load charge %u of character %u.", call.client->GetCharacterID(), new_charge->itemID(), new_charge->ownerID());
        return nullptr;
    }

    if (new_charge->quantity() < (uint32)args.quantity) {
        _log(INV__WARNING, "%s: Item %u: Requested quantity (%d) exceeds actual quantity (%d), using actual.", call.client->GetName(), args.itemID, args.quantity, new_charge->quantity());
    } else if (new_charge->quantity() > (uint32)args.quantity) {
        new_charge = new_charge->Split(args.quantity);  // split item
        if ( !new_charge ) {
            _log(INV__ERROR, "%s: Unable to split charge %d into %d", call.client->GetName(), args.itemID, args.quantity);
            return nullptr;
        }
    }

    // new ref is consumed, we don't release it
    call.client->GetShip()->ReplaceCharges( (EVEItemFlags) args.flag, (InventoryItemRef)new_charge );

    return(new PyInt(1));
}


PyResult InventoryBound::Handle_ListStations( PyCallArgs& call )
{
    _log(INV__MESSAGE, "Calling InventoryBound::ListStations() for %s(%u)", m_self->itemName().c_str(), m_self->itemID());

    util_Rowset rowset;

    rowset.header.push_back( "stationID" );
    rowset.header.push_back( "itemCount" );

    return rowset.Encode();
}

PyResult InventoryBound::Handle_Add(PyCallArgs &call) {
    _log(INV__MESSAGE, "Calling InventoryBound::Add() for %s(%u)", m_self->itemName().c_str(), m_self->itemID());
    if (sConfig.server.testServer) {
        sLog.Log( "InventoryBound::Handle_Add()", "size= %u", call.tuple->size());
        call.Dump(SERVICE__CALL_DUMP);
    }

    if (call.tuple->items.size() == 2) {
        // TODO: Add comments here to describe what kind of client action results in having
        // to use the 'Call_Add_2' packet structure     --ALL add calls.
        // * Moving cargo items from ship cargo bay to a container in space goes here
        // * Moving items from hangar to cargo bay or cargo bay to hangar
        // * Removing Module/Charges from ship (using 'remove' button on item slot)
        // * Adding Modules in a particular slot
        // * Qty missing, so query it from the itemRef
        Call_Add_2 args;
        //chances are its trying to transfer into a cargo container
        if (!args.Decode(&call.tuple)) {
            codelog(INV__ERROR, "Unable to decode arguments");
            return nullptr;
        }

        uint32 flag = flagAutoFit;
        if (call.byname.find("flag") == call.byname.end()) {
            if (IsStation(call.client->GetLocationID()))
               flag = flagHangar;
            else
               flag = flagCargoHold;    // hard-code the ship cargo to cargo container move flag since key 'flag' in client.byname does not exist
        } else
            flag = call.byname.find("flag")->second->AsInt()->value();

        uint32 quantity = 1;
        if (call.byname.find("qty") != call.byname.end()) {
            if (!call.byname.find("qty")->second->IsNone())
                quantity = call.byname.find("qty")->second->AsInt()->value();
        }

        /** @todo  this is module's charge capacity
         *  also used for cargohold capacity
         *  also used for slot capacity (??? got capacity=0 from moving rigs.)
         */
        float capacity = 0.0f;
        if (call.byname.find("capacity") != call.byname.end()) {
            if (!call.byname.find("capacity")->second->IsNone()) {
                if (call.byname.find("capacity")->second->IsFloat())
                    capacity = call.byname.find("capacity")->second->AsFloat()->value();
                else if(call.byname.find("capacity")->second->IsInt())
                    capacity = call.byname.find("capacity")->second->AsInt()->value();
            }
        }

        // TODO  check for 'dividing' byname bool.
        if (call.byname.find("dividing") != call.byname.end())
            _log(INV__ERROR, "[Add] byname.dividing found when adding itemID %u (flag %u)", args.itemID, flag);
        std::vector<int32> items;
        items.push_back(args.itemID);
        return _ExecAdd( call.client, items, quantity, (EVEItemFlags)flag );
    } else {
        _log(INV__ERROR, "[Add] Unknown number of elements in a tuple: %u.", call.tuple->items.size() );
        return nullptr;
    }
}

PyResult InventoryBound::Handle_MultiAdd(PyCallArgs &call) {
    _log(INV__MESSAGE, "Calling InventoryBound::MultiAdd() for %s(%u)", m_self->itemName().c_str(), m_self->itemID());
    if (sConfig.server.testServer) {
        sLog.Log( "InventoryBound::Handle_MultiAdd()", "size= %u", call.tuple->size());
        call.Dump(SERVICE__CALL_DUMP);
    }

    if ( call.tuple->items.size() == 2 ) {
        // TODO: Add comments here to describe what kind of client action results in having
        // to use the 'Call_MultiAdd_2' packet structure    --    --ALL MultiAdd() calls.
        Call_MultiAdd_2 args;
        if (!args.Decode(&call.tuple)) {
            codelog(INV__ERROR, "Unable to decode arguments");
            return nullptr;
        }

        uint32 flag = flagAutoFit;
        if ( call.byname.find("flag") == call.byname.end() ) {
            if (IsStation(call.client->GetLocationID()))
                flag = flagHangar;
            else
                flag = flagCargoHold;
        } else
            flag = call.byname.find("flag")->second->AsInt()->value();

        //bool byname(fromManyFlags):true == unload ALL charges from module (from all modules?? - test this)

        uint32 quantity = 1;
        if (call.byname.find("qty") != call.byname.end()) {
            if (!call.byname.find("qty")->second->IsNone())
                quantity = call.byname.find("qty")->second->AsInt()->value();
        }

        return _ExecAdd( call.client, args.itemIDs, quantity, (EVEItemFlags)flag );
    } else {
        _log(INV__ERROR, "[MultiAdd] Unknown number of elements in a tuple: %u.", call.tuple->items.size() );
        return nullptr;
    }
}

PyResult InventoryBound::Handle_MultiMerge(PyCallArgs &call) {
    _log(INV__MESSAGE, "Calling InventoryBound::MultiMerge() for %s(%u)", m_self->itemName().c_str(), m_self->itemID());
    //Decode Args
    Inventory_CallMultiMerge elements;

    if (!elements.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Unable to decode elements");
        return nullptr;
    }

    Inventory_CallMultiMergeElement element;

    std::vector<PyRep *>::const_iterator cur = elements.MMElements->begin();
    for (; cur != elements.MMElements->end(); cur++) {
        if (!element.Decode( *cur )) {
            _log(SERVICE__WARNING, "Unable to decode element. Skipping.");
            continue;
        }

        InventoryItemRef stationaryItem = m_manager->item_factory->GetItem( element.stationaryItemID );
        if ( !stationaryItem ) {
            _log(INV__WARNING, "Failed to load stationary item %u. Skipping.", element.stationaryItemID);
            continue;
        }

        InventoryItemRef draggedItem = m_manager->item_factory->GetItem( element.draggedItemID );
        if ( !draggedItem ) {
            _log(INV__WARNING, "Failed to load dragged item %u. Skipping.", element.draggedItemID);
            continue;
        }

        draggedItem->SetFlag(stationaryItem->flag());   // Set dragged item's flag to the stationary item's flag so merge can complete
        stationaryItem->Merge( draggedItem, element.draggedQty );
    }

    return nullptr;
}

PyResult InventoryBound::Handle_StackAll(PyCallArgs &call) {
    _log(INV__MESSAGE, "Calling InventoryBound::StackAll() for %s(%u)", m_self->itemName().c_str(), m_self->itemID());
    EVEItemFlags stackFlag = mFlag;

    if (call.tuple->items.size() != 0) {
        Call_SingleIntegerArg arg;
        if (!arg.Decode(&call.tuple)) {
            _log(SERVICE__ERROR, "Failed to decode args.");
            return nullptr;
        }

        stackFlag = (EVEItemFlags)arg.arg;
    }

    //Stack Items contained in this inventory
    mInventory->StackAll(stackFlag, call.client->GetCharacterID());

    return nullptr;
}

PyResult InventoryBound::Handle_StripFitting(PyCallArgs &call) {
    _log(INV__MESSAGE, "Calling InventoryBound::StripFitting() for %s(%u)", m_self->itemName().c_str(), m_self->itemID());
    if (sConfig.server.testServer) {
        sLog.Log( "InventoryBound::Handle_StripFitting()", "size= %u", call.tuple->size());
        call.Dump(SERVICE__CALL_DUMP);
    }

    return nullptr;
}

PyResult InventoryBound::Handle_DestroyFitting(PyCallArgs &call) {
    _log(INV__MESSAGE, "Calling InventoryBound::DestroyFitting() for %s(%u)", m_self->itemName().c_str(), m_self->itemID());
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)){
        sLog.Error("Destroy Fittings","Failed to decode args.");
    }

    //get the actual item
    InventoryItemRef item = m_manager->item_factory->GetItem(args.arg);
    //remove the rig effects from the ship
    call.client->GetShip()->RemoveRig(item);

    return nullptr;
}

PyResult InventoryBound::Handle_SetPassword(PyCallArgs &call) {
    _log(INV__MESSAGE, "Calling InventoryBound::SetPassword() for %s(%u)", m_self->itemName().c_str(), m_self->itemID());
    if (sConfig.server.testServer) {
        sLog.Log( "InventoryBound::Handle_SetPassword()", "size= %u", call.tuple->size());
        call.Dump(SERVICE__CALL_DUMP);
    }
    return nullptr;
}

//01:10:27 L InventoryBound::Handle_CreateBookmarkVouchers(): size= 3, 0 = List, 1 = Integer, 2 = Boolean
                                                            // size,       bmID,     flag,        ismove
PyResult InventoryBound::Handle_CreateBookmarkVouchers(PyCallArgs &call) {
    _log(INV__MESSAGE, "Calling InventoryBound::CreateBookmarkVouchers() for %s(%u)", m_self->itemName().c_str(), m_self->itemID());
  /**
00:39:12 [SvcCall]   Call Arguments:
00:39:12 [SvcCall]       Tuple: 3 elements
00:39:12 [SvcCall]         [ 0] List: 1 elements
00:39:12 [SvcCall]         [ 0]   [ 0] Integer field: 10    -this is bookmarkID(s)
00:39:12 [SvcCall]         [ 1] Integer field: 4            -flag  (?)
00:39:12 [SvcCall]         [ 2] Boolean field: true         -IsMove
00:39:12 [SvcCall]   Call Named Arguments:
00:39:12 [SvcCall]     Argument 'machoVersion':
00:39:12 [SvcCall]         Integer field: 1
00:39:13 L InventoryBound::Handle_CreateBookmarkVouchers(): 1 Vouchers created

00:43:37 [SvcCall]   Call Arguments:
00:43:37 [SvcCall]       Tuple: 3 elements
00:43:37 [SvcCall]         [ 0] List: 5 elements
00:43:37 [SvcCall]         [ 0]   [ 0] Integer field: 4
00:43:37 [SvcCall]         [ 0]   [ 1] Integer field: 6
00:43:37 [SvcCall]         [ 0]   [ 2] Integer field: 7
00:43:37 [SvcCall]         [ 0]   [ 3] Integer field: 11
00:43:37 [SvcCall]         [ 0]   [ 4] Integer field: 2
00:43:37 [SvcCall]         [ 1] Integer field: 4
00:43:37 [SvcCall]         [ 2] Boolean field: true
00:43:37 [SvcCall]   Call Named Arguments:
00:43:37 [SvcCall]     Argument 'machoVersion':
00:43:37 [SvcCall]         Integer field: 1
00:43:37 L InventoryBound::Handle_CreateBookmarkVouchers(): 5 Vouchers created

  sLog.Log( "InventoryBound::Handle_CreateBookmarkVouchers()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALL_DUMP);

      PyList *list = call.tuple->GetItem( 0 )->AsList();
      uint32 bookmarkID;
      char ci[3];

      DBQueryResult res;
      DBResultRow row;

/** @todo this needs work......vouchers in hangar will not show contents of hangar, but item count works. */
/*
      if ( list->size() > 0 ) {
          for (uint8 i = 0; i < (list->size()); i++) {
              bookmarkID = call.tuple->GetItem( 0 )->AsList()->GetItem(i)->AsInt()->value();
                              //ItemData ( typeID, ownerID, locationID, flag, quantity, customInfo, contraband)
              ItemData itemBookmarkVoucher( 51, call.client->GetCharacterID(), call.client->GetLocationID(), flagHangar, 1 );
              InventoryItemRef i = m_manager->item_factory->SpawnItem( itemBookmarkVoucher );

              if ( !i ) {
                  codelog(CLIENT__ERROR, "%s: Failed to spawn bookmark voucher for %u", call.client->GetName(), bookmarkID);
                  break;
              }
              sDatabase.RunQuery(res, "SELECT memo FROM bookmarks WHERE bookmarkID = %u", bookmarkID);
              res.GetRow(row);
              i->Rename(row.GetText(0));
              snprintf(ci, sizeof(ci), "%u", bookmarkID);
              i->SetCustomInfo(ci);  //<- use this to set bookmarkID to DB.entity.customInfo
          }
          sLog.Log( "InventoryBound::Handle_CreateBookmarkVouchers()", "%u Vouchers created", list->size() );
          //  when bm is copied to another players places tab, copy data from db using bookmarkID stored in ItemData.customInfo
      } else {
          sLog.Error( "InventoryBound::Handle_CreateBookmarkVouchers()", "%s: call.tuple->GetItem( 0 )->AsList()->size() == 0.  Expected size > 0.", call.client->GetName() );
          return nullptr;
      }

      /** @todo (allan) need to put check in here for isMove bool.  true=remove from PnP->bookmarks tab....false = leave

      /** @todo (allan) need to reload hangar to show newly created BM item.
*/
    return new PyInt( 0 );
}

PyResult InventoryBound::Handle_Voucher(PyCallArgs &call){
    _log(INV__MESSAGE, "Calling InventoryBound::Voucher() for %s(%u)", m_self->itemName().c_str(), m_self->itemID());
  sLog.Log( "InventoryBound::Handle_Voucher()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALL_DUMP);
    return nullptr;
}

PyRep* InventoryBound::_ExecAdd(Client *pClient, const std::vector<int32> &items, uint32 quantity, EVEItemFlags flag) {
    // method logic rewrite to handle all types and send a proper return, and added some error returns.   -allan 2Jan16
    /** @todo (allan)  handle droping items and modules on ship for cargohold, and removing items from slots to cargo */
    InventoryItemRef itemRef;
    EVEItemFlags old_flag;
    ShipItem* pShip = pClient->GetShip().get();
    std::vector<int32>::const_iterator cur = items.begin();
    for (; cur != items.end(); cur++) {
        itemRef = m_manager->item_factory->GetItem(*cur);
        old_flag = itemRef->flag();

        if (old_flag >= flagRigSlot0 && old_flag <= flagRigSlot7) {
            //  cant remove rigs like this.  send error.
            pClient->SendNotifyMsg("You cannot remove ship upgrades manually.");
            /** @todo (allan)  not all macho.ErrorResponse packet keys are complete.  this is one. */
            throw PyException( MakeUserError("CannotRemoveUpgradeManually"));
            return nullptr;

        }

        if (IsModuleSlot(old_flag)) {
            if (IsModuleSlot(flag)) {
                // we are wanting to change slots on a fitted module.
                // check for existance of module in new slot, switch modules, then return.
                pShip->MoveModuleSlot(old_flag, flag);
                Call_SingleIntegerArg result;
                    result.arg = itemRef->itemID();
                return result.Encode();
            } else
                pShip->RemoveItem(itemRef);
        }

        // check quantities.  correct as needed
        if (quantity != itemRef->quantity()) {
            // item is in stack
            InventoryItemRef newItem = itemRef->Split(quantity);
            if (!newItem) {
                sLog.Error("_ExecAdd", "Error splitting item %u. Skipping.", itemRef->itemID());
                return nullptr;
            }
            // set itemRef to newly created item from splitting.  this will allow common code later.
            itemRef = newItem;
        }

        if (IsCargoHoldFlag(old_flag))
            pShip->RemoveItem(itemRef);

        // check where to put item to be added.  use flags to find a spot for this item
        if (flag == flagAutoFit) {
            // 'flagAutoFit' means "put this module into which ever slot makes sense"
            EVEItemFlags openSlotFlag = pShip->FindAvailableModuleSlot(itemRef);
            if (openSlotFlag == flagIllegal) {
                sLog.Error( "InventoryBound::_ExecAdd()", "'flagIllegal' returned from FindAvailableModuleSlot()" );
                pClient->SendNotifyMsg("Your ship has no avalible slots to fit this module.");
                return nullptr;
            }
            flag = openSlotFlag;
        }

        if (IsModuleSlot(flag) || IsCargoHoldFlag(flag)) {
            // verify ship has room for this item.
            if (!pShip->AddItem(flag, itemRef)) {
                // cannot add item.  error sent from AddItem()
                return nullptr;
            }
        } else {
            // what else do we need to check for here?
            if (mInventory->ValidateAddItem(flag, itemRef)) {
                // all checks have passed.  move the item
                pClient->MoveItem(itemRef->itemID(), m_self->itemID(), flag);
            } else
                return nullptr;
        }
    }

    if (items.size() == 1) {
        //call returns itemID
        Call_SingleIntegerArg result;
            result.arg = itemRef->itemID();
        return result.Encode();
    }
    return nullptr;
}
