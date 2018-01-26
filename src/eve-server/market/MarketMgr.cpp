
 /**
  * @name MarketMgr.cpp
  *   singleton object for storing, manipulating and managing in-game market data
  *
  * @Author:         Allan
  * @date:          19Dec17
  *
  */


#include "market/MarketMgr.h"

MarketMgr::MarketMgr()
: m_marketGroups(nullptr)
{
    Clear();
}

MarketMgr::~MarketMgr()
{
    //Clear();
}

void MarketMgr::Clear()
{

}

void MarketMgr::Close()
{
    /** @todo put a save method here which will save anything changed before shutdown */
    Clear();
}

int MarketMgr::Initialize()
{
    Populate();
    return 1;
}

void MarketMgr::Populate()
{
    double start = GetTimeMSeconds();
    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;

    m_marketGroups = m_db.GetMarketGroups();

    // market orders stored as {regionID/typeID}
    // m_db.GetOrders(call.client->GetRegionID(), args.arg);


    sLog.Cyan("        MarketMgr", "Market Manager loaded in %.3fms.", (GetTimeMSeconds() - start));

    //cleanup
    SafeDelete(res);
}

/*
//NOTE: there are a lot of race conditions to deal with here if we ever
//allow multiple market services to run at the same time.
void MarketProxyService::_ExecuteBuyOrder(uint32 orderID, uint32 stationID, uint32 quantity, Client *seller, InventoryItemRef item, bool isCorp) {
    uint32 ownerID = 0, typeID = 0, qtyReq = 0;
    double price = 0;

    if (!m_db.GetOrderInfo(orderID, &ownerID, &typeID, nullptr, &qtyReq, &price, nullptr, nullptr)) {
        codelog(MARKET__ERROR, "%s: Failed to get info about buy order %u.", seller->GetName(), orderID);
        return;
    }

    if (typeID != item->typeID()) {
        //should never happen.
        codelog(MARKET__ERROR, "%s: Type mismatch executing order %u: order %u item %u", seller->GetName(), orderID, typeID, item->typeID());
        seller->SendErrorMsg("Order type mismatch.");
        return;
    }

    if (quantity > qtyReq) {
        codelog(MARKET__ERROR, "%s: Tried to sell more (%u) than %u required (%u). Selling only what required.", seller->GetName(), quantity, ownerID, qtyReq);
        quantity = qtyReq;
    }

    if (ownerID == item->ownerID()) {
        //I just have a bad feeling that this is not going to work very well...
        codelog(MARKET__WARNING, "%s: Selling an item to ourself... this may not work...", seller->GetName());
    }

    if (item->singleton() or item->quantity() == quantity) {
        //entire item is moving...move out of first inventory
        item->Move(0, flagAutoFit, false);
        item->ChangeOwner(ownerID);
        item->Move(stationID, flagHangar, true);
    } else {
        //need to split item up...
        InventoryItemRef iRef = item->Split(quantity, true);
        if (iRef.get() == nullptr) {
            codelog(MARKET__ERROR, "Failed to split item %u.", item->itemID());
            return;
        }
        //use the owner change packet to alert the buyer of the new item
        iRef->ChangeOwner(ownerID);
        iRef->Move(stationID, flagHangar, true);
    }

    //the buyer has already paid out the money before the buy order was recorded in the database.
    //give the money to the seller...
    double money = price * quantity;
    // send wallet blink event and record the transaction in their journal.
    std::string reason = "DESC:  Selling items in ";
    reason += itoa(stationID);
    AccountService::TranserFunds(
        ownerID,
        seller->GetCharacterID(),
                                 money,
                                 reason.c_str(),
                                 Journal::EntryType::MarketTransaction,
                                 orderID,
                                 Account::KeyType::Cash);

    if (quantity == qtyReq) {
        _log(MARKET__TRACE, "%s: Completely satisfied order %u, deleting.", seller->GetName(), orderID);
        PyRep* order = m_db.GetOrderRow(orderID);
        if (!m_db.DeleteOrder(orderID)) {
            codelog(MARKET__ERROR, "Failed to delete order %u.", orderID);
            return;
        }
        _InvalidateOrdersCache(typeID);
        _BroadcastOnOwnOrderChanged(seller->GetRegionID(), orderID, "Expiry", isCorp, order);
        _BroadcastOnMarketRefresh(seller->GetRegionID());
    } else {
        _log(MARKET__TRACE, "%s: Partially satisfied order %u, altering quantity to %u.", seller->GetName(), orderID, qtyReq - quantity);
        if (!m_db.AlterOrderQuantity(orderID, qtyReq - quantity)) {
            codelog(MARKET__ERROR, "Failed to alter quantity of order %u.", orderID);
            return;
        }
        _InvalidateOrdersCache(typeID);
        _BroadcastOnOwnOrderChanged(seller->GetRegionID(), orderID, "Modify", isCorp);
    }

    //record this transaction in market_transactions
    //NOTE: regionID may not be accurate here...
    if (!m_db.RecordTransaction(typeID, quantity, price, TransactionTypeSell, seller->GetCharacterID(), seller->GetRegionID(), stationID)) {
        codelog(MARKET__ERROR, "%s: Failed to record sale side of transaction.", seller->GetName());
    }
    //FIXME:  for orderOwnerID == 1, reset owner to npc corp of stationID
    if (!m_db.RecordTransaction(typeID, quantity, price, TransactionTypeBuy, ownerID, seller->GetRegionID(), stationID)) {
        codelog(MARKET__ERROR, "%s: Failed to record buy side of transaction.", seller->GetName());
    }
}

//NOTE: there are a lot of race conditions to deal with here if we ever
//allow multiple market services to run at the same time.
void MarketProxyService::_ExecuteSellOrder(uint32 orderID, uint32 stationID, uint32 quantity, Client *buyer, bool isCorp) {
    uint32 ownerID = 0, typeID = 0, qtyAvail = 0;
    double price = 0;

    if (!m_db.GetOrderInfo(orderID, &ownerID, &typeID, nullptr, &qtyAvail, &price, nullptr, nullptr)) {
        codelog(MARKET__ERROR, "%s: Failed to get info about sell order %u.", buyer->GetName(), orderID);
        return;
    }

    if (quantity > qtyAvail) {
        codelog(MARKET__ERROR, "%s: Tried to buy more (%u) than available (%u). Buying all available.", buyer->GetName(), quantity, qtyAvail);
        quantity = qtyAvail;
    }

    if (ownerID == buyer->GetCharacterID()) {
        //I just have a bad feeling that this is not going to work very well...
        codelog(MARKET__WARNING, "%s: Buying an item from ourself... this may not work...", buyer->GetName());
    }

    //  check for sellerID == EVESystem, and change to station owner (npcCorpID)
    if (ownerID == 1) {
        ServiceDB s_db;
        ownerID = s_db.GetStationOwner(stationID);
    }

    //spawn the item in the buyer's hangar.
    ItemData idata(typeID, ownerID, stationID, flagAutoFit, quantity);
    InventoryItemRef new_item = sItemFactory.SpawnItem(idata);
    if (new_item.get() == nullptr) {
        // item not created.  make error msg
        return;
    }

    double money = price * quantity;

    //take the money from the buyer after we spawn the item. (verify the item is created.)
    // send wallet blink event and record the transaction in their journal.
    std::string reason = "DESC:  Buying items in ";
    reason += itoa(stationID);
    AccountService::TranserFunds(
        buyer->GetCharacterID(),
                                 ownerID,
                                 money,
                                 reason.c_str(),
                                 Journal::EntryType::MarketTransaction,
                                 orderID,
                                 Account::KeyType::Cash);

    //use the owner change packet to alert the buyer of the new item
    new_item->ChangeOwner(buyer->GetCharacterID());
    if (buyer->IsDocked())
        new_item->Move(buyer->GetStationID(), flagHangar, true);
    else
        new_item->Move(stationID, flagHangar, true);

    if (quantity == qtyAvail) {
        _log(MARKET__TRACE, "%s: Completely satisfied order %u, deleting.", buyer->GetName(), orderID);
        PyRep* order = m_db.GetOrderRow(orderID);
        if (!m_db.DeleteOrder(orderID)) {
            codelog(MARKET__ERROR, "Failed to delete order %u.", orderID);
            return;
        }
        _InvalidateOrdersCache(typeID);
        _BroadcastOnOwnOrderChanged(buyer->GetRegionID(), orderID, "Expiry", isCorp, order);
        _BroadcastOnMarketRefresh(buyer->GetRegionID());
    } else {
        _log(MARKET__TRACE, "%s: Partially satisfied order %u, altering quantity to %u.", buyer->GetName(), orderID, qtyAvail - quantity);
        if (!m_db.AlterOrderQuantity(orderID, qtyAvail - quantity)) {
            codelog(MARKET__ERROR, "Failed to alter quantity of order %u.", orderID);
            return;
        }
        _InvalidateOrdersCache(typeID);
        _BroadcastOnOwnOrderChanged(buyer->GetRegionID(), orderID, "Modify", isCorp);
    }

    //record this transaction in market_transactions
    if (!m_db.RecordTransaction(typeID, quantity, price, TransactionTypeBuy, buyer->GetCharacterID(), sDataMgr.GetStationRegion(stationID), stationID)) {
        codelog(MARKET__ERROR, "%s: Failed to record buy side of transaction.", buyer->GetName());
    }
    if (!m_db.RecordTransaction(typeID, quantity, price, TransactionTypeSell, ownerID, sDataMgr.GetStationRegion(stationID), stationID)) {
        codelog(MARKET__ERROR, "%s: Failed to record sale side of transaction.", buyer->GetName());
    }
}
*/