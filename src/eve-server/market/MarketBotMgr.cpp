
 /**
  * @name MarketBotMgr.h
  *   system for automating/emulating buy and sell orders on the market.
  * idea and some code taken from AuctionHouseBot - Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
  * @Author:         Allan
  * @date:   10 August 2016
  * @version:  0.3 (config version)
  */

#include "eve-server.h"
#include "EVEServerConfig.h"
#include "market/MarketBotConf.h"
#include "market/MarketBotMgr.h"
#include "market/MarketDB.h"
#include "market/MarketProxyService.h"


static const char* const BOT_CONFIG_FILE = EVEMU_ROOT "/etc/MarketBot.xml";


MarketBotDataMgr::MarketBotDataMgr()
{
    m_initalized = false;
}

void MarketBotDataMgr::Initialize()
{
    m_initalized = true;

    sLog.Green("   Market Bot Mgr", "Market Bot Data Manager Initialized.");
    /* load current data */

}



MarketBotMgr::MarketBotMgr()
:  m_updateTimer(120000)    // arbitrary 2m default
{
    m_updateTimer.Disable();
    m_initalized = false;
}


void MarketBotMgr::Initialize()
{
    if (!sConfig.server.UseMarketBot) {
        sLog.Warning("   Market Bot Mgr", "Market Bot Disabled.");
        return;
    }

    if (!sMBotConf.ParseFile(BOT_CONFIG_FILE)) {
        sLog.Error("       ServerInit", "Loading Market Bot Config file '%s' failed.", BOT_CONFIG_FILE);
        return;
    }

    m_initalized = true;
    sMktBotDataMgr.Initialize();

    sLog.Blue("   Market Bot Mgr", "Market Bot Manager Initialized.");
    /* start timers, process current data, and create new orders, if needed */

}

void MarketBotMgr::Process()
{
    if (!m_initalized) return;
    if (m_updateTimer.Check(false)) {
    /* process current data, process orders, xfer funds, reset timers, create new orders */
    }
}

