
 /**
  * @name MarketMgr.cpp
  *   singleton object for storing, manipulating and managing in-game market data
  *   this mgr keeps track of market data without abusing the db on every call. (vs old system)
  *
  * @Author:         Allan
  * @date:          19Dec17
  *
  */

#include "Client.h"
#include "StaticDataMgr.h"
#include "account/AccountService.h"
#include "inventory/InventoryItem.h"
#include "market/MarketMgr.h"

MarketMgr::MarketMgr()
: m_marketGroups(nullptr),
 m_timer(60 *60 *1000) // hour timer
{

}

MarketMgr::~MarketMgr()
{
    //PyDecRef(m_marketGroups);
}

void MarketMgr::Close()
{
    /** @todo put a save method here which will save anything changed before shutdown */
    PyDecRef(m_marketGroups);
}

int MarketMgr::Initialize()
{
    Populate();
    sLog.Blue("        MarketMgr", "Market Manager Initialized.");
    return 1;
}

void MarketMgr::Populate()
{
    double start = GetTimeMSeconds();
    m_marketGroups = m_db.GetMarketGroups();

    UpdatePriceHistory();

    // market orders stored as {regionID/typeID}    --do we want to store orders in memory for loaded region??
    // m_db.GetOrders(call.client->GetRegionID(), args.arg);

    sLog.Blue("        MarketMgr", "Market Manager loaded in %.3fms.", (GetTimeMSeconds() - start));
}

void MarketMgr::GetInfo()
{
    /* get info in current market data? */

}

void MarketMgr::Process()
{
    if (m_timer.Check())
        UpdatePriceHistory();
}

void MarketMgr::UpdatePriceHistory()
{
    DBerror err;

    int64 cutoff_time = Win32TimeNow();
    cutoff_time -= cutoff_time % Win32Time_Day;    //round down to an even day boundary.
    cutoff_time -= Win32Time_Day * 2;  //the cutoff between "new" and "old" price history in days

    //build the history record from the recent market transactions.
    sDatabase.RunQuery(err,
        "INSERT INTO"
        "    mktHistory"
        "     (regionID, typeID, historyDate, lowPrice, highPrice, avgPrice, volume, orders)"
        " SELECT"
        "    regionID,"
        "    typeID,"
        "    transactionDate - ( transactionDate %% %" PRId64 " ) AS historyDate,"
        "    MIN(price) AS lowPrice,"
        "    MAX(price) AS highPrice,"
        "    AVG(price) AS avgPrice,"
        "    SUM(quantity) AS volume,"
        "    COUNT(transactionID) AS orders"
        " FROM mktTransactions "
        " WHERE"
        "    transactionType=1 AND "    //both buy and sell transactions get recorded, only compound data for 'buy' orders.
        "    ( transactionDate - ( transactionDate %% %" PRId64 " ) ) < %" PRId64
        " GROUP BY regionID, typeID, historyDate",
        Win32Time_Day, Win32Time_Day, cutoff_time);

    /*  //now remove the transactions which have been aged out?
     *   sDatabase.RunQuery(err, "DELETE FROM mktTransactions WHERE historyDate < %" PRId64, cutoff_time);
     */
}

    /*DBColumnTypeMap colmap;
     *    colmap["historyDate"] = DBTYPE_FILETIME;
     *    colmap["lowPrice"] = DBTYPE_CY;
     *    colmap["highPrice"] = DBTYPE_CY;
     *    colmap["avgPrice"] = DBTYPE_CY;
     *    colmap["volume"] = DBTYPE_I8;
     *    colmap["orders"] = DBTYPE_I4;
     */

// there is a 1 day difference (from 0000UTC) between "Old" and "New" prices
PyRep *MarketMgr::GetNewPriceHistory(uint32 regionID, uint32 typeID) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT"
        "    transactionDate - ( transactionDate %% %" PRId64 " ) AS historyDate,"
        "    MIN(price) AS lowPrice,"
        "    MAX(price) AS highPrice,"
        "    AVG(price) AS avgPrice,"
        "    CAST(SUM(quantity) AS SIGNED INTEGER) AS volume,"
        "    CAST(COUNT(transactionID) AS SIGNED INTEGER) AS orders"
        " FROM mktTransactions "
        " WHERE regionID=%u AND typeID=%u"
        "    AND transactionType=%d "    //both buy and sell transactions get recorded, only compound one set of data... choice was arbitrary.
        " GROUP BY historyDate",
        Win32Time_Day, regionID, typeID, TransactionTypeBuy))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

PyRep *MarketMgr::GetOldPriceHistory(uint32 regionID, uint32 typeID) {
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT historyDate, lowPrice, highPrice, avgPrice, volume, orders"
        " FROM mktHistory "
        " WHERE regionID=%u AND typeID=%u AND historyDate < %llu", regionID, typeID, (Win32TimeNow() - (Win32Time_Day * 2))))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

void MarketMgr::SendOnOwnOrderChanged(Client *who, uint32 orderID, const char *action, bool isCorp/*false*/, PyRep* order/*nullptr*/) {
    Notify_OnOwnOrderChanged ooc;
    if (order != nullptr)
        ooc.order = order;
    else
        ooc.order = m_db.GetOrderRow(orderID);
    ooc.reason = action;
    ooc.isCorp = isCorp;
    PyTuple* tmp = ooc.Encode();
    who->SendNotification("OnOwnOrderChanged", "clientID", &tmp);   //tmp consumed.
}

void MarketMgr::BroadcastOnOwnOrderChanged(uint32 regionID, uint32 orderID, const char *action, bool isCorp/*false*/, PyRep* order/*nullptr*/) {
    std::vector<Client*> clients;
    sEntityList.FindByRegionID(regionID, clients);
    std::vector<Client*>::iterator cur = clients.begin();
    for (; cur != clients.end(); ++cur) {
        PySafeIncRef(order);
        SendOnOwnOrderChanged(*cur, orderID, action, isCorp, order);
    }
    PySafeDecRef(order);
}

void MarketMgr::BroadcastOnMarketRefresh(uint32 regionID) {
    std::vector<Client*> clients;
    sEntityList.FindByRegionID(regionID, clients);
    std::vector<Client*>::iterator cur = clients.begin();
    for (; cur != clients.end(); ++cur) {
        SendOnMarketRefresh(*cur);
    }
}
// cant find where this is used or referenced....
void MarketMgr::SendOnMarketRefresh(Client *who) {
    PyTuple* tmp = new PyTuple(0);
    who->SendNotification("OnMarketRefresh", "clientID", &tmp);   //tmp consumed.
}

/*
void MarketMgr::InvalidateOrdersCache(uint32 typeID)
{
    std::string method_name ("GetOrders_");
    method_name += itoa(typeID);
    ObjectCachedMethodID method_id(GetName(), method_name.c_str());
    m_manager->cache_service->InvalidateCache( method_id );
}
*/

/** @todo take off market overhead fees */


void MarketMgr::ExecuteBuyOrder(uint32 orderID, uint32 stationID, uint32 quantity, Client *seller, InventoryItemRef item, bool isCorp) {
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
        //InvalidateOrdersCache(typeID);
        BroadcastOnOwnOrderChanged(seller->GetRegionID(), orderID, "Expiry", isCorp, order);
        //BroadcastOnMarketRefresh(seller->GetRegionID());
    } else {
        _log(MARKET__TRACE, "%s: Partially satisfied order %u, altering quantity to %u.", seller->GetName(), orderID, qtyReq - quantity);
        if (!m_db.AlterOrderQuantity(orderID, qtyReq - quantity)) {
            codelog(MARKET__ERROR, "Failed to alter quantity of order %u.", orderID);
            return;
        }
        //InvalidateOrdersCache(typeID);
        BroadcastOnOwnOrderChanged(seller->GetRegionID(), orderID, "Modify", isCorp);
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

void MarketMgr::ExecuteSellOrder(uint32 orderID, uint32 stationID, uint32 quantity, Client *buyer, bool isCorp) {
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
        //InvalidateOrdersCache(typeID);
        BroadcastOnOwnOrderChanged(buyer->GetRegionID(), orderID, "Expiry", isCorp, order);
        //BroadcastOnMarketRefresh(buyer->GetRegionID());
    } else {
        _log(MARKET__TRACE, "%s: Partially satisfied order %u, altering quantity to %u.", buyer->GetName(), orderID, qtyAvail - quantity);
        if (!m_db.AlterOrderQuantity(orderID, qtyAvail - quantity)) {
            codelog(MARKET__ERROR, "Failed to alter quantity of order %u.", orderID);
            return;
        }
        //InvalidateOrdersCache(typeID);
        BroadcastOnOwnOrderChanged(buyer->GetRegionID(), orderID, "Modify", isCorp);
    }

    //record this transaction in market_transactions
    if (!m_db.RecordTransaction(typeID, quantity, price, TransactionTypeBuy, buyer->GetCharacterID(), sDataMgr.GetStationRegion(stationID), stationID)) {
        codelog(MARKET__ERROR, "%s: Failed to record buy side of transaction.", buyer->GetName());
    }
    if (!m_db.RecordTransaction(typeID, quantity, price, TransactionTypeSell, ownerID, sDataMgr.GetStationRegion(stationID), stationID)) {
        codelog(MARKET__ERROR, "%s: Failed to record sale side of transaction.", buyer->GetName());
    }
}
