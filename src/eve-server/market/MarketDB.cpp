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
    Author:        Zhur
    Updates:    Allan
*/

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "StaticDataMgr.h"
#include "market/MarketDB.h"

/*
 * MARKET__ERROR
 * MARKET__WARNING
 * MARKET__MESSAGE
 * MARKET__DEBUG
 * MARKET__TRACE
 * MARKET__DB_ERROR
 * MARKET__DB_TRACE
 */

PyRep *MarketDB::GetStationAsks(uint32 stationID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "    typeID, MIN(price) AS price, volRemaining, stationID "
        " FROM mktOrders "
        " WHERE stationID=%u"
        " GROUP BY typeID", stationID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    //NOTE: this SHOULD return a crazy dbutil.RowDict object which is
    //made up of packed blue.DBRow objects, but we do not understand
    //the marshalling of those well enough right now, and this object
    //provides the same interface. It is significantly bigger on the wire though.
    return DBResultToIndexRowset(res, "typeID");
}

PyRep *MarketDB::GetSystemAsks(uint32 solarSystemID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "    typeID, MIN(price) AS price, volRemaining, stationID "
        " FROM mktOrders "
        " WHERE solarSystemID=%u"
        " GROUP BY typeID", solarSystemID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    //NOTE: this SHOULD return a crazy dbutil.RowDict object which is
    //made up of packed blue.DBRow objects, but we do not understand
    //the marshalling of those well enough right now, and this object
    //provides the same interface. It is significantly bigger on the wire though.
    return DBResultToIndexRowset(res, "typeID");
}

PyRep *MarketDB::GetRegionBest(uint32 regionID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "    typeID, MIN(price) AS price, volRemaining, stationID "
        " FROM mktOrders "
        " WHERE regionID=%u AND bid=%u"
        " GROUP BY typeID", regionID, TransactionTypeSell))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    //NOTE: this SHOULD return a crazy dbutil.RowDict object which is
    //made up of packed blue.DBRow objects, but we do not understand
    //the marshalling of those well enough right now, and this object
    //provides the same interface. It is significantly bigger on the wire though.
    return DBResultToIndexRowset(res, "typeID");
}

PyRep *MarketDB::GetOrders( uint32 regionID, uint32 typeID )
{
    // returns a tuple (sell, buy) of PyObjectEx with data in PyPackedRows

    PyTuple* tup = new PyTuple(2);
    DBQueryResult res;
    //query sell orders
    //TODO: consider the `jumps` field... is it actually used? yes...sellers trade skill for maxSellJumps
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "    price, volRemaining, typeID, orderRange AS `range`, orderID,"
        "   volEntered, minVolume, bid, issued as issueDate, duration,"
        "   stationID, regionID, solarSystemID, jumps"
        " FROM mktOrders "
        " WHERE regionID=%u AND typeID=%u AND bid=%u", regionID, typeID, TransactionTypeSell))
    {
        codelog( DATABASE__ERROR, "Error in query: %s", res.error.c_str() );
        return nullptr;
    }
    _log(MARKET__DB_TRACE, "MarketDB::GetOrders() - Fetched %u sell orders for type %u", res.GetRowCount(), typeID);
    tup->SetItem(0, DBResultToCRowset( res ) );

    //res.Reset();
    //query buy orders
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "    price, volRemaining, typeID, orderRange AS `range`, orderID,"
        "   volEntered, minVolume, bid, issued as issueDate, duration,"
        "   stationID, regionID, solarSystemID, jumps"
        " FROM mktOrders "
        " WHERE regionID=%u AND typeID=%u AND bid=%u", regionID, typeID, TransactionTypeBuy))
    {
        codelog( DATABASE__ERROR, "Error in query: %s", res.error.c_str() );
        PyDecRef( tup );
        return nullptr;
    }
    _log(MARKET__DB_TRACE, "MarketDB::GetOrders() - Fetched %u buy orders for type %u", res.GetRowCount(), typeID);
    tup->SetItem(1, DBResultToCRowset( res ) );

    if (is_log_enabled(MARKET__DUMP))
        tup->Dump(MARKET__DUMP, "    ");
    return tup;
}

PyRep* MarketDB::GetOrdersForOwner(uint32 ownerID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "   orderID, typeID, ownerID, regionID, stationID,"
        "   orderRange AS `range`, bid, price, volEntered, volRemaining,"
        "   issued as issueDate, orderState, minVolume, contraband,"
        "   accountID, duration, isCorp, solarSystemID,"
        "   escrow"
        " FROM mktOrders "
        " WHERE ownerID=%u", ownerID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

PyRep *MarketDB::GetOrderRow(uint32 orderID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "    price, volRemaining, typeID, orderRange AS `range`, orderID,"
        "   volEntered, minVolume, bid, issued as issueDate, duration,"
        "   stationID, regionID, solarSystemID, jumps"
        " FROM mktOrders"
        " WHERE orderID=%u", orderID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(MARKET__ERROR, "Order %u not found.", orderID);
        return nullptr;
    }

    return DBRowToPackedRow(row);
}

//NOTE: needs a lot of work to implement orderRange
uint32 MarketDB::FindBuyOrder(
    uint32 stationID,
    uint32 typeID,
    double price,
    uint32 quantity,
    uint32 orderRange
) {
    price += 0.01;

    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT orderID"
        "    FROM mktOrders"
        "    WHERE bid=1"
        "        AND typeID=%u"
        "        AND stationID=%u"
        "        AND volRemaining >= %u"
        "        AND price < %.2f"
        "    ORDER BY price DESC"
        "    LIMIT %u",
        typeID,
        stationID,
        quantity,
        price,
        sConfig.market.FindBuyOrder))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (!res.GetRow(row))
        return 0;    //no order found.

    return row.GetUInt(0);
}

uint32 MarketDB::FindSellOrder(
    uint32 stationID,
    uint32 typeID,
    double price,
    uint32 quantity,
    uint32 orderRange
) {
    price += 0.01;

    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT orderID"
        "    FROM mktOrders"
        "    WHERE bid=0"
        "        AND typeID=%u"
        "        AND stationID=%u"
        "        AND volRemaining >= %u"
        "        AND price < %.2f"
        "    LIMIT %u",
        typeID,
        stationID,
        quantity,
        price,
        sConfig.market.FindSellOrder))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (res.GetRow(row))
        return row.GetUInt(0);

    return 0;
}

/** @todo  what the fuck is this????  */
bool MarketDB::GetOrderInfo(
    uint32 orderID,
    uint32 *ownerID,
    uint32 *typeID,
    uint32 *stationID,
    uint32 *quantity,
    double *price,
    bool *isBuy,
    bool *isCorp
) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        " volRemaining,"
        " price,"
        " typeID,"
        " stationID,"
        " ownerID,"
        " bid,"
        " isCorp"
        " FROM mktOrders"
        " WHERE orderID=%u",
        orderID))
    {
        _log(DATABASE__ERROR, "Error in query: %s.", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(MARKET__ERROR, "Order %u not found.", orderID);
        return false;
    }

    if (quantity != nullptr)        *quantity   = row.GetUInt(0);
    if (price != nullptr)           *price      = row.GetDouble(1);
    if (typeID != nullptr)          *typeID     = row.GetUInt(2);
    if (stationID != nullptr)       *stationID  = row.GetUInt(3);
    if (ownerID != nullptr)         *ownerID    = row.GetUInt(4);
    if (isBuy != nullptr)           *isBuy      = row.GetInt(5) ? true : false;
    if (isCorp != nullptr)          *isCorp     = row.GetBool(6);

    return true;
}

//NOTE: this logic needs some work if there are multiple concurrent market services running at once.
bool MarketDB::AlterOrderQuantity(uint32 orderID, uint32 new_qty) {
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE mktOrders SET volRemaining = %u WHERE orderID = %u",  new_qty, orderID)) {
        _log(DATABASE__ERROR, "Error in query: %s.", err.c_str());
        return false;
    }
    return true;
}

bool MarketDB::AlterOrderPrice(uint32 orderID, double new_price) {
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE mktOrders SET price = %.2f WHERE orderID = %u", new_price, orderID)) {
        _log(DATABASE__ERROR, "Error in query: %s.", err.c_str());
        return false;
    }
    return true;
}

bool MarketDB::DeleteOrder(uint32 orderID) {
    DBerror err;
    if (!sDatabase.RunQuery(err, "DELETE FROM mktOrders WHERE orderID = %u", orderID)) {
        _log(DATABASE__ERROR, "Error in query: %s.", err.c_str());
        return false;
    }
    return true;
}

bool MarketDB::RecordTransaction( uint32 typeID, uint32 quantity, double price, MktTransType transactionType, uint32 charID, uint32 regionID, uint32 stationID) {
    DBerror err;
    /** @todo implement the accountKey field here */
    if (!sDatabase.RunQuery(err,
        "INSERT INTO"
        " mktTransactions ("
        "    transactionDate, typeID, quantity,"
        "    price, transactionType, clientID, regionID, stationID,"
        "    corpTransaction"
        " ) VALUES ("
        "    %f, %u, %u,"
        "    %.2f, %d, %u, %u, %u, 0"
        " )",
            GetFileTimeNow(), typeID, quantity,
            price, transactionType, charID, regionID, stationID
            ))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", err.c_str());
        return false;
    }
    return true;
}

uint32 MarketDB::StoreBuyOrder(
    uint32 ownerID,
    uint32 accountID,
    uint32 stationID,
    uint32 typeID,
    double price,
    uint32 quantity,
    int16 orderRange,
    uint32 minVolume,
    uint8 duration,
    bool isCorp
) {
    return _StoreOrder(ownerID, accountID, stationID, typeID, price, quantity, orderRange, minVolume, duration, isCorp, true);
}

uint32 MarketDB::StoreSellOrder(
    uint32 ownerID,
    uint32 accountID,
    uint32 stationID,
    uint32 typeID,
    double price,
    uint32 quantity,
    int16 orderRange,
    uint32 minVolume,
    uint8 duration,
    bool isCorp
) {
    return _StoreOrder(ownerID, accountID, stationID, typeID, price, quantity, orderRange, minVolume, duration, isCorp, false);
}

uint32 MarketDB::_StoreOrder(
    uint32 ownerID, uint32 accountID, uint32 stationID, uint32 typeID, double price, uint32 quantity, int16 orderRange, uint32 minVolume, uint8 duration, bool isCorp, bool isBuy
) {
    // get the solar system and region IDs.
    // note:  GetSystemInfo can use either stationID OR solarSystemID.  -allan 3Aug16
    SystemData data;
    if (!sDataMgr.GetSystemInfo(stationID, data)) {
        codelog(MARKET__ERROR, "Char %u: Failed to find parents for station %u", ownerID, stationID);
        return 0;
    }

    //TODO: figure out what the orderState field means...
    //TODO: implement the contraband flag properly.
    //TODO: implement the isCorp flag properly.
    DBerror err;
    uint32 orderID;
    if (!sDatabase.RunQueryLID(err, orderID,
        "INSERT INTO mktOrders ("
        "    typeID, ownerID, regionID, stationID,"
        "    orderRange, bid, price, volEntered, volRemaining, issued,"
        "    orderState, minVolume, contraband, accountID, duration,"
        "    isCorp, solarSystemID, escrow, jumps "
        " ) VALUES ("
        "    %u, %u, %u, %u, "
        "    %i, %u, %.2f, %u, %u, %f, "
        "    1, %u, 0, %u, %u, "
        "    %u, %u, 0, 1"
        " )",
            typeID, ownerID, data.regionID, stationID,
            orderRange, isBuy?1:0, price, quantity, quantity, GetFileTimeNow(),
            minVolume, accountID, duration,
            isCorp?1:0, data.systemID
        ))

    {
        codelog(DATABASE__ERROR, "Error in query: %s", err.c_str());
        return 0;
    }

    return orderID;
}

PyRep *MarketDB::GetTransactions(
    uint32 characterID,
    uint32 typeID,
    uint32 quantity,
    double minPrice,
    double maxPrice,
    int64 fromDate,
    int buySell,
    uint32 accountKey,
    uint32 memberID
) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        " transactionID,transactionDate,typeID,quantity,price,transactionType,"
        " corpTransaction,clientID,stationID,keyID"
        " FROM mktTransactions "
        " WHERE clientID=%u AND (typeID=%u OR 0=%u) AND"
        " quantity>=%u AND price>=%.2f AND (price<=%.2f OR 0=%.2f) AND"
        " transactionDate>=%lli AND (transactionType=%d OR -1=%d)"
        " AND keyID=%u",
        characterID, typeID, typeID, quantity, minPrice, maxPrice, maxPrice, fromDate, buySell, buySell, accountKey))
    {
        codelog( DATABASE__ERROR, "Error in query: %s", res.error.c_str() );
        return nullptr;
    }

    return DBResultToRowset(res);
}

PyRep *MarketDB::GetMarketGroups() {

    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT * FROM invMarketGroups"))  {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    DBRowDescriptor *header = new DBRowDescriptor(res);

    _log(MARKET__DEBUG, "MarketDB::GetMarketGroups header has %u columns.", header->ColumnCount());

    CFilterRowSet *filterRowset = new CFilterRowSet(&header);
    PyDict *keywords = filterRowset->GetKeywords();
    keywords->SetItemString("allowDuplicateCompoundKeys", PyStatic.NewFalse());
    keywords->SetItemString("indexName", PyStatic.NewNone());
    keywords->SetItemString("columnName", new PyString("parentGroupID"));

    DBResultRow row;
    std::map< int, PyRep* > tt;
    while( res.GetRow(row) ) {
        int parentGroupID = ( row.IsNull( 0 ) ? -1 : row.GetUInt( 0 ) );
        PyRep* pid(nullptr);
        CRowSet*rowset(nullptr);
        if (tt.count(parentGroupID) == 0) {
            pid = parentGroupID != -1 ? (PyRep*)new PyInt(parentGroupID) : PyStatic.NewNone();
            tt[parentGroupID] = pid;
            rowset = filterRowset->NewRowset(pid);
        } else {
            pid = tt[parentGroupID];
            rowset = filterRowset->GetRowset(pid);
        }

        PyPackedRow* pyrow = rowset->NewRow();
        pyrow->SetField((uint32)0, pid); //prentGroupID
        pyrow->SetField(1, new PyInt(row.GetUInt( 1 ) ) ); //marketGroupID
        pyrow->SetField(2, new PyString(row.GetText( 2 ) ) ); //marketGroupName
        pyrow->SetField(3, new PyString(row.GetText( 3 ) ) ); //description
        pyrow->SetField(4, row.IsNull( 4 ) ? PyStatic.NewNone() : new PyInt(row.GetUInt( 4 ))  ); //graphicID
        pyrow->SetField(5, new PyBool(row.GetBool( 5 ) ) ); //hasTypes
        pyrow->SetField(6, row.IsNull( 6 ) ? PyStatic.NewNone() : new PyInt(row.GetUInt( 6 ))  ); // iconID
        pyrow->SetField(7, new PyInt( row.GetUInt(7) )  ); //dataID
        pyrow->SetField(8, new PyInt( row.GetUInt(8) )  ); //marketGroupNameID
        pyrow->SetField(9, new PyInt( row.GetUInt(9) )  ); //descriptionID
    }

    _log(MARKET__DEBUG, "MarketDB::GetMarketGroups returned %u keys.", filterRowset->GetKeyCount());
    //if (is_log_enabled(MARKET__DB_TRACE))
    //    filterRowset->Dump(MARKET__DB_TRACE, "    ");

    return filterRowset;
}

int64 MarketDB::GetUpdateTime()
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT timeStamp FROM mktUpdates WHERE server = 1"))  {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }
    DBResultRow row;
    if (!res.GetRow(row))
        return 0;
    return row.GetInt64(0);
}

void MarketDB::SetUpdateTime(int64 setTime)
{
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE mktUpdates SET timeStamp = %lli WHERE server = 1", setTime);
}

void MarketDB::UpdateHistory()
{
    DBerror err;
    sDatabase.RunQuery(err,
                   "INSERT INTO"
                   "    mktHistory"
                   "     (regionID, typeID, historyDate, lowPrice, highPrice, avgPrice, volume, orders)"
                   " SELECT"
                   "    regionID,"
                   "    typeID,"
                   "    ((UNIX_TIMESTAMP(date) +11644473600) *10000000),"
                   "    price,"
                   "    price,"
                   "    price,"
                   "    amtEntered,"
                   "    COUNT(DISTINCT typeID)"
                   " FROM mktData");
}