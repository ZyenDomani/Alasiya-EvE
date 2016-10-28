
 /**
  * @name MarketBotMgr.h
  *   system for automating/emulating buy and sell orders on the market.
  * idea and some code taken from AuctionHouseBot - Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
  * @Author:         Allan
  * @date:   10 August 2016
  */


#ifndef EVEMU_MARKET_MARKETBOT_H_
#define EVEMU_MARKET_MARKETBOT_H_


#include "eve-compat.h"
#include "eve-common.h"
#include "utils/Singleton.h"

class MarketBotMgr
: public Singleton<MarketBotMgr>
{
public:
    MarketBotMgr();
    ~MarketBotMgr();

    void Init();
    void Process();

private:
    Timer m_updateTimer;
    
    bool m_initalized;

};


//Singleton
#define sMarketBotMgr \
( MarketBotMgr::get() )


#endif  // EVEMU_MARKET_MARKETBOT_H_