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
*/

#include "eve-server.h"

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

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "    typeID, MAX(price) AS price, volRemaining, stationID "
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
    return(DBResultToIndexRowset(res, "typeID"));
}

PyRep *MarketDB::GetSystemAsks(uint32 solarSystemID) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "    typeID, MAX(price) AS price, volRemaining, stationID "
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
    return(DBResultToIndexRowset(res, "typeID"));
}

PyRep *MarketDB::GetRegionBest(uint32 regionID) {
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT"
        "    typeID, MIN(price) AS price, volRemaining, stationID "
        " FROM mktOrders "
        " WHERE regionID=%u AND bid=%d"
        " GROUP BY typeID", regionID, TransactionTypeSell))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    //NOTE: this SHOULD return a crazy dbutil.RowDict object which is
    //made up of packed blue.DBRow objects, but we do not understand
    //the marshalling of those well enough right now, and this object
    //provides the same interface. It is significantly bigger on the wire though.
    return(DBResultToIndexRowset(res, "typeID"));
}

PyRep *MarketDB::GetOrders( uint32 regionID, uint32 typeID )
{
    DBQueryResult res;

    PyList* tup = new PyList();

    /*
              [PyList 2 items]
                [PyObjectEx Type2]
                  [PyTuple 2 items]
                    [PyTuple 1 items]
                      [PyToken dbutil.RowList]
                    [PyDict 2 kvp]
                      [PyString "header"]
                      [PyObjectEx Normal]
                        [PyTuple 2 items]
                          [PyToken blue.DBRowDescriptor]
                          [PyTuple 1 items]
                      [PyString "columns"]
                      [PyList 14 items]
                        [PyString "price"]
                        [PyString "volRemaining"]
                        [PyString "typeID"]
                        [PyString "range"]
                        [PyString "orderID"]
                        [PyString "volEntered"]
                        [PyString "minVolume"]
                        [PyString "bid"]
                        [PyString "issued"]
                        [PyString "duration"]
                        [PyString "stationID"]
                        [PyString "regionID"]
                        [PyString "solarSystemID"]
                        [PyString "jumps"]
                  [PyPackedRow 57 bytes]
                    ["price" => <11200000000> [CY]]
                    ["volRemaining" => <1> [R8]]
                    ["typeID" => <3244> [I2]]
                    ["range" => <32767> [I2]]
                    ["orderID" => <2018034541> [I4]]
                    ["volEntered" => <1> [I4]]
                    ["minVolume" => <1> [I4]]
                    ["bid" => <0> [Bool]]
                    ["issued" => <129492629676100000> [FileTime]]
                    ["duration" => <90> [I2]]
                    ["stationID" => <60003910> [I4]]
                    ["regionID" => <10000033> [I4]]
                    ["solarSystemID" => <30002738> [I4]]
                    ["jumps" => <6> [I4]]*/

    //query sell orders
    //TODO: consider the `jumps` field... is it actually used? yes...sellers trade skill for maxSellJumps
    if(!sDatabase.RunQuery(res,
        "SELECT"
        "    price, volRemaining, typeID, orderRange AS `range`, orderID,"
        "   volEntered, minVolume, bid, issued as issueDate, duration,"
        "   stationID, regionID, solarSystemID, jumps"
        " FROM mktOrders "
        " WHERE regionID=%u AND typeID=%u AND bid=%d", regionID, typeID, TransactionTypeSell))
    {
        codelog( DATABASE__ERROR, "Error in query: %s", res.error.c_str() );

        PyDecRef( tup );
        return nullptr;
    }
    sLog.Debug("MarketDB::GetOrders", "Fetched %d sell orders for type %d", res.GetRowCount(), typeID);

    //this is wrong.
    tup->AddItem( DBResultToCRowset( res ) );

    //query buy orders
    if(!sDatabase.RunQuery(res,
        "SELECT"
        "    price, volRemaining, typeID, orderRange AS `range`, orderID,"
        "   volEntered, minVolume, bid, issued as issueDate, duration,"
        "   stationID, regionID, solarSystemID, jumps"
        " FROM mktOrders "
        " WHERE regionID=%u AND typeID=%u AND bid=%d", regionID, typeID, TransactionTypeBuy))
    {
        codelog( DATABASE__ERROR, "Error in query: %s", res.error.c_str() );

        PyDecRef( tup );
        return nullptr;
    }
    sLog.Debug("MarketDB::GetOrders", "Fetched %d buy orders for type %d", res.GetRowCount(), typeID);

    //this is wrong.
    tup->AddItem( DBResultToCRowset( res ) );

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

    if(!sDatabase.RunQuery(res,
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
    if(!res.GetRow(row)) {
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
    price = price + 0.01;
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT orderID"
        "    FROM mktOrders"
        "    WHERE bid=1"
        "        AND typeID=%u"
        "        AND stationID=%u"
        "        AND volRemaining >= %u"
        "        AND price < %.2f"
        "    ORDER BY price DESC"
        "    LIMIT 10",     /** @todo make config option for qty to return? */
        typeID,
        stationID,
        quantity,
        price))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if(!res.GetRow(row))
        return(0);    //no order found.

    return(row.GetUInt(0));
}

uint32 MarketDB::FindSellOrder(
    uint32 stationID,
    uint32 typeID,
    double price,
    uint32 quantity,
    uint32 orderRange
) {
    price = price + 0.01;
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT orderID"
        "    FROM mktOrders"
        "    WHERE bid=0"
        "        AND typeID=%u"
        "        AND stationID=%u"
        "        AND volRemaining >= %u"
        "        AND price < %.2f"
        "    LIMIT 10",       /** @todo make config option for qty to return? */
        typeID,
        stationID,
        quantity,
        price))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (res.GetRow(row))
        return (row.GetUInt(0));

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

    if(!sDatabase.RunQuery(res,
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
    if(!res.GetRow(row)) {
        _log(MARKET__ERROR, "Order %u not found.", orderID);
        return false;
    }

    if (quantity != nullptr)        *quantity = row.GetUInt(0);
    if (price != nullptr)           *price = row.GetDouble(1);
    if (typeID != nullptr)          *typeID = row.GetUInt(2);
    if (stationID != nullptr)       *stationID = row.GetUInt(3);
    if (ownerID != nullptr)         *ownerID = row.GetUInt(4);
    if (isBuy != nullptr)           *isBuy = row.GetBool(5);
    if (isCorp != nullptr)          *isCorp = row.GetBool(6);

    return true;
}

//NOTE: this logic needs some work if there are multiple concurrent market services running at once.
bool MarketDB::AlterOrderQuantity(uint32 orderID, uint32 new_qty) {
    DBerror err;
    if(!sDatabase.RunQuery(err, "UPDATE mktOrders SET volRemaining = %u WHERE orderID = %u",  new_qty, orderID)) {
        _log(DATABASE__ERROR, "Error in query: %s.", err.c_str());
        return false;
    }
    return true;
}

bool MarketDB::AlterOrderPrice(uint32 orderID, double new_price) {
    DBerror err;
    if(!sDatabase.RunQuery(err, "UPDATE mktOrders SET price = %.2f WHERE orderID = %u", new_price, orderID)) {
        _log(DATABASE__ERROR, "Error in query: %s.", err.c_str());
        return false;
    }
    return true;
}

bool MarketDB::DeleteOrder(uint32 orderID) {
    DBerror err;
    if(!sDatabase.RunQuery(err, "DELETE FROM mktOrders WHERE orderID = %u", orderID)) {
        _log(DATABASE__ERROR, "Error in query: %s.", err.c_str());
        return false;
    }
    return true;
}

bool MarketDB::RecordTransaction( uint32 typeID, uint32 quantity, double price, MktTransType transactionType, uint32 charID, uint32 regionID, uint32 stationID) {
    DBerror err;
    //TODO implement the accountKey field here
    if(!sDatabase.RunQuery(err,
        "INSERT INTO"
        " mktTransactions ("
        "    transactionID, transactionDate, typeID, quantity,"
        "    price, transactionType, clientID, regionID, stationID,"
        "    corpTransaction"
        " ) VALUES ("
        "    NULL, %" PRIu64 ", %u, %u,"
        "    %.2f, %d, %u, %u, %u, 0"
        " )",
            Win32TimeNow(), typeID, quantity,
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
    return(_StoreOrder(ownerID, accountID, stationID, typeID, price, quantity, orderRange, minVolume, duration, isCorp, true));
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
    return(_StoreOrder(ownerID, accountID, stationID, typeID, price, quantity, orderRange, minVolume, duration, isCorp, false));
}

uint32 MarketDB::_StoreOrder(
    uint32 ownerID, uint32 accountID, uint32 stationID, uint32 typeID, double price, uint32 quantity, int16 orderRange, uint32 minVolume, uint8 duration, bool isCorp, bool isBuy
) {

    // get the solar system and region IDs.
    // note:  GetSystemInfo can use either stationID OR solarSystemID.  -allan 3Aug16
    SystemData data;
    if (!sDataMgr.GetSystemInfo(stationID, data)) {
        codelog(MARKET__ERROR, "Char %u: Failed to find parents for station %u", ownerID, stationID);
        return(0);
    }

    //TODO: figure out what the orderState field means...
    //TODO: implement the contraband flag properly.
    //TODO: implement the isCorp flag properly.
    DBerror err;
    uint32 orderID;
    if(!sDatabase.RunQueryLID(err, orderID,
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
        return(0);
    }

    return(orderID);
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

    if(!sDatabase.RunQuery(res,
        "SELECT"
        " transactionID,transactionDate,typeID,quantity,price,transactionType,"
        " corpTransaction,clientID,stationID,keyID"
        " FROM mktTransactions "
        " WHERE clientID=%u AND (typeID=%u OR 0=%u) AND"
        " quantity>=%u AND price>=%.2f AND (price<=%.2f OR 0=%.2f) AND"
        " transactionDate>=%" PRIu64 " AND (transactionType=%d OR -1=%d)"
        " AND keyID=%u",
        characterID, typeID, typeID, quantity, minPrice, maxPrice, maxPrice, fromDate, buySell, buySell, accountKey))
    {
        codelog( DATABASE__ERROR, "Error in query: %s", res.error.c_str() );

        return nullptr;
    }

    return DBResultToRowset(res);
}

//helper routine for GetMarketGroups
static void _PropigateItems(std::map< int, std::set<uint32> > &types, std::map<int, int> &parentChild, std::map<int, std::set<int> > &childParent, int group) {
    std::map<int, std::set<int> >::iterator children_res;
    children_res = childParent.find(group);
    if(children_res != childParent.end()) {
        //recurse to all children first.
        std::set<int>::iterator ccur, cend;
        ccur = children_res->second.begin();
        cend = children_res->second.end();
        for(; ccur != cend; ccur++) {
            _PropigateItems(types, parentChild, childParent, *ccur);
        }
    }

    if(group == -1) {
        return;    //we are root, we have no parent
    }
    //find our parent.
    std::map<int, int>::iterator parent_res;
    parent_res = parentChild.find(group);
    if(parent_res == parentChild.end()) {
        codelog(MARKET__ERROR, "Failed to find parent group in parentChild for %d", group);
        return;    //should never happen...
    }
    int parentID = parent_res->second;
    if(parentID == -1) {
        return;    //do not propigate up to NULL, we dont need it, and it would contain ALL items..
    }

    //now propigate all our items (which now includes all children items) up to our parent.
    //find our items
    std::map< int, std::set<uint32> >::iterator self_res;
    self_res = types.find(group);
    if(self_res == types.end())
        return;    //we have nothing for this group??

        //add all of our items into parent.
        types[parentID].insert(self_res->second.begin(), self_res->second.end());
}

//this is a crap load of work... there HAS to be a better way to do this..
PyRep *MarketDB::GetMarketGroups() {

    DBQueryResult res;
    DBResultRow row;

    if(!sDatabase.RunQuery(res,
        "SELECT * "
        " FROM invMarketGroups"))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    DBRowDescriptor *header = new DBRowDescriptor(res);

    for( int i=0; i<header->ColumnCount(); i++) {
        sLog.Debug("MarketDB::GetMarketGroups", "  column %s type %d",
                   header->GetColumnName(i), header->GetColumnType(i));
    }

    CFilterRowSet *filterRowset = new CFilterRowSet(&header);

    PyDict *keywords = filterRowset->GetKeywords();
    keywords->SetItemString("allowDuplicateCompoundKeys", new PyBool(false));
    keywords->SetItemString("indexName", PyStatic.NewNone());
    keywords->SetItemString("columnName", new PyString("parentGroupID"));
    std::map< int, PyRep* > tt;

    while( res.GetRow(row) )
    {
        int parentGroupID = ( row.IsNull( 0 ) ? -1 : row.GetUInt( 0 ) );
        PyRep *pid;
        CRowSet *rowset;
        if(tt.count(parentGroupID) == 0) {
            pid = parentGroupID!=-1 ? (PyRep*)new PyInt(parentGroupID) : PyStatic.NewNone();
            tt.insert( std::pair<int, PyRep*>(parentGroupID, pid) );
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

    return filterRowset;
}
