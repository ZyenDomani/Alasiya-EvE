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
    Author:     Zhur
*/

#include "PyServiceCD.h"
#include "StaticDataMgr.h"
#include "account/AccountService.h"
#include "cache/ObjCacheService.h"
#include "market/MarketMgr.h"
#include "market/MarketProxyService.h"
#include "station/StationDataMgr.h"
#include "system/SystemManager.h"

PyCallable_Make_InnerDispatcher(MarketProxyService)

MarketProxyService::MarketProxyService(PyServiceMgr *mgr)
: PyService(mgr, "marketProxy"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(MarketProxyService, StartupCheck);
    PyCallable_REG_CALL(MarketProxyService, GetStationAsks);
    PyCallable_REG_CALL(MarketProxyService, GetSystemAsks);
    PyCallable_REG_CALL(MarketProxyService, GetRegionBest);
    PyCallable_REG_CALL(MarketProxyService, GetMarketGroups);
    PyCallable_REG_CALL(MarketProxyService, GetOrders);
    PyCallable_REG_CALL(MarketProxyService, GetOldPriceHistory);
    PyCallable_REG_CALL(MarketProxyService, GetNewPriceHistory);
    PyCallable_REG_CALL(MarketProxyService, PlaceCharOrder);
    PyCallable_REG_CALL(MarketProxyService, GetCharOrders);
    PyCallable_REG_CALL(MarketProxyService, ModifyCharOrder);
    PyCallable_REG_CALL(MarketProxyService, CancelCharOrder);
    PyCallable_REG_CALL(MarketProxyService, CharGetNewTransactions);
    PyCallable_REG_CALL(MarketProxyService, CorpGetNewTransactions);
    PyCallable_REG_CALL(MarketProxyService, GetCorporationOrders);
}

MarketProxyService::~MarketProxyService() {
    delete m_dispatch;
}

/*
 * MARKET__ERROR
 * MARKET__WARNING
 * MARKET__MESSAGE
 * MARKET__DEBUG
 * MARKET__TRACE
 * MARKET__DB_ERROR
 * MARKET__DB_TRACE
 */

PyResult MarketProxyService::Handle_StartupCheck(PyCallArgs &call) {
    if (!sMktMgr.IsUpdated())
        sMktMgr.UpdatePriceHistory();
    return nullptr;
}

PyResult MarketProxyService::Handle_GetMarketGroups(PyCallArgs &call) {
    return sMktMgr.GetMarketGroups();
}

/** @todo update these to use market manager instead of hitting db */
PyResult MarketProxyService::Handle_GetStationAsks(PyCallArgs &call) {
    if (call.client->IsDocked())
        return m_db.GetStationAsks(call.client->GetStationID());
    return m_db.GetSystemAsks(call.client->GetSystemID());
}

PyResult MarketProxyService::Handle_GetSystemAsks(PyCallArgs &call) {
    return m_db.GetSystemAsks(call.client->GetSystemID());
}

PyResult MarketProxyService::Handle_GetRegionBest(PyCallArgs &call) {
    return m_db.GetRegionBest(call.client->GetRegionID());
}

PyResult MarketProxyService::Handle_GetCharOrders(PyCallArgs &call) {
    return m_db.GetOrdersForOwner(call.client->GetCharacterID());
}

PyResult MarketProxyService::Handle_GetCorporationOrders(PyCallArgs &call) {
    return m_db.GetOrdersForOwner(call.client->GetCorporationID());
}

// this is called 3x on every market transaction
/** @todo  make this static data, updated on a timer.  can be done in MarketMgr code  */
PyResult MarketProxyService::Handle_GetOldPriceHistory(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(MARKET__ERROR, "Invalid arguments");
        return nullptr;
    }

    return sMktMgr.GetOldPriceHistory(call.client->GetRegionID(), args.arg);
}

PyResult MarketProxyService::Handle_GetNewPriceHistory(PyCallArgs &call) {
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(MARKET__ERROR, "Invalid arguments");
        return nullptr;
    }

    return sMktMgr.GetNewPriceHistory(call.client->GetRegionID(), args.arg);
}

PyResult MarketProxyService::Handle_GetOrders(PyCallArgs &call) {
    Call_SingleIntegerArg args; //itemID
    if (!args.Decode(&call.tuple)) {
        codelog(MARKET__ERROR, "Invalid arguments");
        return nullptr;
    }

    PyRep* result(nullptr);
    std::string method_name ("GetOrders_");
    method_name += itoa(args.arg);
    ObjectCachedMethodID method_id(GetName(), method_name.c_str());
    //check to see if this method is in the cache already.
    if (!m_manager->cache_service->IsCacheLoaded(method_id))
    {
        //this method is not in cache yet, load up the contents and cache it.
        result = m_db.GetOrders(call.client->GetRegionID(), args.arg);
        if (result == nullptr) {
            codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
            result = PyStatic.NewNone();
        }
        m_manager->cache_service->GiveCache(method_id, &result);
    }

    //now we know its in the cache one way or the other, so build a
    //cached object cached method call result.
    result = m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id);

    return result;
}

PyResult MarketProxyService::Handle_PlaceCharOrder(PyCallArgs &call) {
  /**
17:15:42 [SvcCall]   Call Arguments:
17:15:42 [SvcCall]       Tuple: 11 elements
17:15:42 [SvcCall]         [ 0] Integer field: 60004594
17:15:42 [SvcCall]         [ 1] Integer field: 28409
17:15:42 [SvcCall]         [ 2] Real field: 2400.000000
17:15:42 [SvcCall]         [ 3] Integer field: 1000000
17:15:42 [SvcCall]         [ 4] Integer field: 1
17:15:42 [SvcCall]         [ 5] Integer field: -1
17:15:42 [SvcCall]         [ 6] (None)
17:15:42 [SvcCall]         [ 7] Integer field: 1
17:15:42 [SvcCall]         [ 8] Integer field: 1
17:15:42 [SvcCall]         [ 9] Boolean field: false
17:15:42 [SvcCall]         [10] (None)
*/
  Call_PlaceCharOrder args;
    if (!args.Decode(&call.tuple)) {
        codelog(MARKET__ERROR, "Invalid arguments");
        return nullptr;
    }

    args.Dump(MARKET__DUMP, "CharOrder    ");

    //TODO: verify the validity of args.stationID (range vs. skill)
    //TODO: handle located?  'located' is officeFolderID, officeID.  not sure how its' sent yet.
    //NOTE: I am not sure that useCorp is as simple as it is currently implemented...

    /** @todo  update for corporate use */

    if (args.bid) {  //buy order
        //try to satisfy immediately...
        uint32 order_id = m_db.FindSellOrder(args.stationID, args.typeID, args.price, args.quantity, args.orderRange);
        _log(MARKET__TRACE, "MarketProxyService::Handle_PlaceCharOrder - %s: Trying buy order %u to satisfy (type %u, station %u, price %.2f, qty %u, range %i)", \
                    call.client->GetName(), order_id, args.typeID, args.stationID, args.price, args.quantity, args.orderRange);

        if (order_id) {
            _log(MARKET__TRACE, "%s: Found sell order %u to satisfy (type %u, station %u, price %.2f, qty %u, range %i)", \
                        call.client->GetName(), order_id, args.typeID, args.stationID, args.price, args.quantity, args.orderRange);

            sMktMgr.ExecuteSellOrder(order_id, args.stationID, args.quantity, call.client, args.useCorp);
            return nullptr;
        }

        //unable to satisfy immediately...
        if (args.duration == 0) {
            _log(MARKET__ERROR, "%s: Failed to satisfy buy order for %d of type %d at %.2f ISK.", call.client->GetName(), args.quantity, args.typeID, args.price);
            call.client->SendErrorMsg("No such order found.");
            return nullptr;
        }

        //store the order in the DB.
        uint32 orderID = m_db.StoreBuyOrder(
            call.client->GetCharacterID(),
            call.client->GetUserID(),
            args.stationID,
            args.typeID,
            args.price,
            args.quantity,
            args.orderRange,
            args.minVolume,
            args.duration,
            args.useCorp);
        if (orderID == 0) {
            codelog(MARKET__ERROR, "%s: Failed to record order in the DB.", call.client->GetName());
            call.client->SendErrorMsg("Failed to record the order in the DB!");
            return nullptr;
        }

        //send notification of new order...
        InvalidateOrdersCache(args.typeID);
        sMktMgr.BroadcastOnOwnOrderChanged(call.client->GetRegionID(), orderID, "Add", args.useCorp);
    } else {
        //sell order

        //verify that they actually have the item in the quantity specified...
        InventoryItemRef item = sItemFactory.GetItem( args.itemID );
        if ( !item ) {
            codelog(MARKET__ERROR, "%s: Failed to find item %d for sell order.", call.client->GetName(), args.itemID);
            call.client->SendErrorMsg("Unable to find items %d to sell!", args.itemID);
            return nullptr;
        }
        //verify right to sell this thing..
        //TODO: this should be a much more complicated check with corp stuff....
        if (item->ownerID() != call.client->GetCharacterID()) {
            codelog(MARKET__ERROR, "%s: Char %d Tried to sell item %d with owner %d.", call.client->GetName(), call.client->GetCharacterID(), item->itemID(), item->ownerID());
            call.client->SendErrorMsg("You cannot sell items you do not own.");
            return nullptr;
        }

        //verify that they specified a valid station ID to sell from.
        if ((item->locationID() != args.stationID)   //item in station hanger
           and !(call.client->GetShip()->GetMyInventory()->ContainsItem( item->itemID() )  //item is in our ship
                and call.client->GetStationID() == args.stationID ))   //and our ship is in the station
        {
            codelog(MARKET__ERROR, "%s: Tried to sell item %d which is in location %d through station %d while in station %d", call.client->GetName(), item->itemID(), item->locationID(), args.stationID, call.client->GetStationID());
            call.client->SendErrorMsg("You cannot sell that item in that station.");
            return nullptr;
        }

        if ((item->singleton() && args.quantity != 1) or  item->quantity() < args.quantity ) {
            codelog(MARKET__ERROR, "%s: Tried to sell %d of item %d which has qty %d singleton %d", call.client->GetName(), args.quantity, item->itemID(), item->quantity(), item->singleton());
            call.client->SendErrorMsg("You cannot sell more than you have.");
            return nullptr;
        }

        if (item->typeID() != args.typeID) {
            codelog(MARKET__ERROR, "%s: Tried to sell item %d of type %d using type ID %d", call.client->GetName(), item->itemID(), item->typeID(), args.typeID);
            call.client->SendErrorMsg("Invalid sell order item type.");
            return nullptr;
        }

        //TODO: verify orderRange against their skills.   client may do this...verify

        //ok, we think they are allowed to sell this thing...

        //try to satisfy immediately...
        uint32 order_id = m_db.FindBuyOrder(args.stationID, args.typeID, args.price, args.quantity, args.orderRange);
        if (order_id) {
            _log(MARKET__TRACE, "%s: Found buy order %u to satisfy (type %u, station %u, price %.2f, qty %u, range %u)", call.client->GetName(), order_id, args.typeID, args.stationID, args.price, args.quantity, args.orderRange);

            sMktMgr.ExecuteBuyOrder(order_id, args.stationID, args.quantity, call.client, item, args.useCorp);
            return nullptr;
        }

        //else, unable to satisfy immediately...
        _log(MARKET__TRACE, "%s: Unable to find an immediate order to satisfy (type %u, station %u, price %.2f, qty %u, range %u)", call.client->GetName(), args.stationID, args.typeID, args.price, args.quantity, args.orderRange);

        if (args.duration == 0) {
            _log(MARKET__ERROR, "%s: Failed to satisfy sell order for %d of type %d at %.2f ISK.", call.client->GetName(), args.quantity, args.typeID,  args.price);
            return nullptr;
        }

        //TODO: take broker cost.

        //take item from seller
        if (item->quantity() == args.quantity) {
            call.client->SystemMgr()->RemoveItemFromInventory(item);
            item->Delete();
        } else {
            //update the item.
            if (!item->AlterQuantity(-args.quantity, true)) {
                codelog(MARKET__ERROR, "%s: Failed to consume %u units from item %u", call.client->GetName(), args.quantity, item->itemID());
                return nullptr;
            }
        }

        //store the order in the DB.
        uint32 orderID = m_db.StoreSellOrder(
            call.client->GetCharacterID(),
            call.client->GetUserID(),
            args.stationID,
            args.typeID,
            args.price,
            args.quantity,
            args.orderRange,
            args.minVolume,
            args.duration,
            args.useCorp);
        if (orderID == 0) {
            codelog(MARKET__ERROR, "%s: Failed to record order in the DB.", call.client->GetName());
            call.client->SendErrorMsg("Failed to record the order in the DB!");
            return nullptr;
        }

        //notify client about new order.
        InvalidateOrdersCache(args.typeID);
        sMktMgr.BroadcastOnOwnOrderChanged(call.client->GetRegionID(), orderID, "Add", args.useCorp);
    }

    //returns nothing.
    return nullptr;
}

PyResult MarketProxyService::Handle_ModifyCharOrder(PyCallArgs &call) {
    Call_ModifyCharOrder args;
    if (!args.Decode(&call.tuple)) {
        codelog(MARKET__ERROR, "Invalid arguments");
        return nullptr;
    }

    uint32 typeID = 0, quantity = 0;
    double price = 0;
    bool isBuy = false, isCorp = false;

    if (!m_db.GetOrderInfo(args.orderID, nullptr, &typeID, nullptr, &quantity, &price, &isBuy, &isCorp)) {
        codelog(MARKET__ERROR, "%s: Failed to get info about order %u.", call.client->GetName(), args.orderID);
        return nullptr;
    }

    if (price == args.newPrice)
        return nullptr;

    if (isBuy) {  // GiveCash
        double money = (price - args.newPrice) * quantity;
        if (!call.client->AddBalance(money, Account::CreditType::ISK))
            return nullptr;
    }

    if (!m_db.AlterOrderPrice(args.orderID, args.newPrice)) {
        codelog(MARKET__ERROR, "%s: Failed to modify price for order %u.", call.client->GetName(), args.orderID);
        return nullptr;
    }

    InvalidateOrdersCache(typeID);
    sMktMgr.BroadcastOnOwnOrderChanged(call.client->GetRegionID(), args.orderID, "Modify", isCorp); //force a refresh of market data.

    return nullptr;
}

PyResult MarketProxyService::Handle_CancelCharOrder(PyCallArgs &call) {
    Call_CancelCharOrder args;
    if (!args.Decode(&call.tuple)) {
        codelog(MARKET__ERROR, "Invalid arguments");
        return nullptr;
    }

    uint32 ownerID = 0, typeID = 0, stationID = 0, quantity = 0;
    double price = 0;
    bool isBuy = false, isCorp = false;

    if (!m_db.GetOrderInfo(args.orderID, &ownerID, &typeID, &stationID, &quantity, &price, &isBuy, &isCorp)) {
        codelog(MARKET__ERROR, "%s: Failed to get info about order %u.", call.client->GetName(), args.orderID);
        return nullptr;
    }


    if (isBuy) { // GiveCash
        double money = price * quantity;
        if (!call.client->AddBalance(money))
            return nullptr;
    } else {
        ItemData idata(typeID, 1, 0, flagHangar, quantity);
        InventoryItemRef new_item = sItemFactory.SpawnItem(idata);
        new_item->ChangeOwner(call.client->GetCharacterID());
        new_item->Move(stationID, flagHangar, true);
    }

    PyRep* order = m_db.GetOrderRow(args.orderID);
    if (!m_db.DeleteOrder(args.orderID)) {
        codelog(MARKET__ERROR, "Failed to delete order %u.", args.orderID);
        return nullptr;
    }
    InvalidateOrdersCache(typeID);
    sMktMgr.BroadcastOnOwnOrderChanged(call.client->GetRegionID(), args.orderID, "Expiry", isCorp, order); //force a refresh of market data.
    sMktMgr.BroadcastOnMarketRefresh(call.client->GetRegionID());

    return nullptr;
}

PyResult MarketProxyService::Handle_CharGetNewTransactions(PyCallArgs &call)
{
    Call_GetNewCharTransactions args;
    if (!args.Decode(&call.tuple)) {
        codelog(MARKET__ERROR, "Invalid arguments");
        return nullptr;
    }

    double minPrice = PyRep::IntegerValue(args.minPrice);

    return m_db.GetTransactions(args.clientID==0?call.client->GetCharacterID():args.clientID, \
            args.typeID, args.quantity, minPrice, args.maxPrice, args.fromDate, args.buySell);
}

PyResult MarketProxyService::Handle_CorpGetNewTransactions(PyCallArgs &call)
{
    Call_GetNewCorpTransactions args;
    if (!args.Decode(&call.tuple)) {
        codelog(MARKET__ERROR, "Invalid arguments");
        return nullptr;
    }

    double minPrice = PyRep::IntegerValue(args.minPrice);

    return m_db.GetTransactions(call.client->GetCorporationID(), args.typeID, args.quantity, minPrice,\
                                  args.maxPrice, args.fromDate, args.buySell, args.accountKey, args.memberID);
}

void MarketProxyService::InvalidateOrdersCache(uint32 typeID)
{
    std::string method_name ("GetOrders_");
    method_name += itoa(typeID);
    ObjectCachedMethodID method_id(GetName(), method_name.c_str());
    m_manager->cache_service->InvalidateCache( method_id );
}

