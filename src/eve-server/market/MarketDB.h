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

#ifndef __MARKETDB_H_INCL__
#define __MARKETDB_H_INCL__

#include "ServiceDB.h"

class PyRep;

typedef enum
{
    TransactionTypeSell = 0,
    TransactionTypeBuy = 1
} MktTransType;

class MarketDB
: public ServiceDB
{
public:
    PyRep* GetMarketGroups();
    PyRep* GetOrders(uint32 regionID, uint32 typeID);
    PyRep* GetOrderRow(uint32 orderID);
    PyRep* GetRegionBest(uint32 regionID);
    PyRep* GetSystemAsks(uint32 solarSystemID);
    PyRep* GetStationAsks(uint32 stationID);
    PyRep* GetOrdersForOwner(uint32 ownerID);

    PyRep* GetTransactions(uint32 characterID, uint32 typeID, uint32 quantity, double minPrice, double maxPrice, int64 fromDate, int buySell, uint32 accountKey = 1000, uint32 memberID = 0);

    bool DeleteOrder(uint32 orderID);
    bool GetOrderInfo(uint32 orderID, uint32* ownerID, uint32* typeID, uint32* stationID, uint32* quantity, double* price, bool* isBuy, bool* isCorp);
    bool AlterOrderPrice(uint32 orderID, double new_price);
    bool RecordTransaction(uint32 typeID, uint32 quantity, double price, MktTransType transactionType, uint32 charID, uint32 regionID, uint32 stationID);
    bool AlterOrderQuantity(uint32 orderID, uint32 new_qty);

    uint32 FindBuyOrder(uint32 stationID, uint32 typeID, double price, uint32 quantity, uint32 orderRange);
    uint32 FindSellOrder(uint32 stationID, uint32 typeID, double price, uint32 quantity, uint32 orderRange);
    uint32 StoreBuyOrder(uint32 ownerID, uint32 accountID, uint32 stationID, uint32 typeID, double price, uint32 quantity, int16 orderRange, uint32 minVolume, uint8 duration, bool isCorp);
    uint32 StoreSellOrder(uint32 ownerID, uint32 accountID, uint32 stationID, uint32 typeID, double price, uint32 quantity, int16 orderRange, uint32 minVolume, uint8 duration, bool isCorp);

    /* for marketMgr update service */
    static int64 GetUpdateTime();
    static void SetUpdateTime();

    static void UpdateHistory();

protected:

    uint32 _StoreOrder(uint32 ownerID, uint32 accountID, uint32 stationID, uint32 typeID, double price, uint32 quantity, int16 orderRange, uint32 minVolume, uint8 duration, bool isCorp, bool isBuy);

};





#endif


