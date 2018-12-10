
 /**
  * @name MarketMgr.h
  *   singleton object for storing, manipulating and managing in-game market data
  *
  * @Author:         Allan
  * @date:          19Dec17
  *
  */


#ifndef _EVE_SERVER_MARKET_MANAGER_H__
#define _EVE_SERVER_MARKET_MANAGER_H__


#include "../eve-server.h"

#include "EntityList.h"
#include "market/MarketDB.h"

/* market range
rangeStation = -1
rangeSolarSystem = 0
rangeConstellation = 4
rangeRegion = 32767
*/

class Client;

class MarketMgr
: public Singleton< MarketMgr >
{
public:
    MarketMgr();
    ~MarketMgr();

    int Initialize();

    void Close();
    void GetInfo();
    void Process();

    bool NeedsUpdate();

    void UpdatePriceHistory();

    void ExecuteBuyOrder(uint32 buy_order_id, uint32 stationID, uint32 quantity, Client *seller, InventoryItemRef item, bool isCorp=false);
    void ExecuteSellOrder(uint32 sell_order_id, uint32 stationID, uint32 quantity, Client *buyer, bool isCorp=false);
    void SendOnOwnOrderChanged(Client *who, uint32 orderID, const char *action, bool isCorp=false, PyRep* order = nullptr);
    void BroadcastOnOwnOrderChanged(uint32 regionID, uint32 orderID, const char *action, bool isCorp=false, PyRep* order = nullptr);
    void SendOnMarketRefresh(Client *who);
    
    //void InvalidateOrdersCache(uint32 regionID);

    PyRep* GetMarketGroups()               { PyIncRef(m_marketGroups); return m_marketGroups; }
    PyRep* GetNewPriceHistory(uint32 regionID, uint32 typeID);
    PyRep* GetOldPriceHistory(uint32 regionID, uint32 typeID);

protected:
    void Populate();

private:
    MarketDB m_db;

    PyRep* m_marketGroups;  // static market group data

    int64 m_timeStamp;

    // markets are regional.  there are 66 regions.
    // market orders are stored as {regionID/typeID}
    //  load market data by region, sorted by system/station.
    //  station data will also store StationRef to owning/containing station for easier access later
    //  will be able to implement 'jumps' and other market conditionals
};

//Singleton
#define sMktMgr \
( MarketMgr::get() )


#endif  // _EVE_SERVER_MARKET_MANAGER_H__
