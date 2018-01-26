
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

#include "market/MarketDB.h"


class MarketMgr
: public Singleton< MarketMgr >
{
public:
    MarketMgr();
    ~MarketMgr();

    int                 Initialize();

    void                Clear();
    void                Close();
    void                GetInfo();

    PyRep*              GetMarketGroups()               { PyIncRef(m_marketGroups); return m_marketGroups; }

protected:
    void                Populate();


private:
    MarketDB m_db;

    PyRep* m_marketGroups;  // static market group data


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
