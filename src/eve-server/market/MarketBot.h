
 /**
  * @name MarketBot.h
  *   system for automating/emulating NPC corps' buying and selling on the market.
  * idea and some code taken from AuctionHouseBot - Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
  * @Author:         Allan
  * @date:   10 August 2016
  */


#ifndef EVEMU_MARKET_MARKETBOT_H_
#define EVEMU_MARKET_MARKETBOT_H_


#include "eve-compat.h"
#include "eve-common.h"
#include "utils/Singleton.h"

class MarketBot
: public Singleton<MarketBot>
{
public:
    MarketBot();
    ~MarketBot();

};

//Singleton
#define sMarketBot \
( MarketBot::get() )







#endif  // EVEMU_MARKET_MARKETBOT_H_