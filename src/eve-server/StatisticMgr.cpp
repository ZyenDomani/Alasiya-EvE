
 /**
  * @name StatisticMgr.cpp
  *   server information system for managing and saving trivial game data
  *
  * @Author:    Allan
  * @date:      8 March 2018
  *
  */


#include "StatisticMgr.h"
#include "system/cosmicMgrs/ManagerDB.h"


StatisticMgr::StatisticMgr()
{

}

void StatisticMgr::Close()
{
    // save current data to db before exit.
    SaveData();
    ClearAll();
}

int StatisticMgr::Initialize()
{
    ClearAll();
    sLog.Blue( "Statistic Manager", "Statistics Manager Initialized." );
}

void StatisticMgr::ClearAll()
{
    m_data.span             = 0;
    m_data.pcShots          = 0;
    m_data.pcMissiles       = 0;
    m_data.pcBounties       = 0;
    m_data.npcBounties      = 0;
    m_data.oreMined         = 0;
    m_data.iskMarket        = 0;
    m_data.shipsSalvaged    = 0;
}

void StatisticMgr::GetInfo()
{
    // return current data?
}

// called every 15m by ConsoleCommands::UpdateStatus() from EntityList::Process()
void StatisticMgr::Process()
{
    // check timers and manipulate data accordingly...
    SaveData();


/*
SELECT timeStamp, period, 1, 2, 3, 4, 5, 6, 7, 8, 9 FROM srvStatisticData
SELECT month, 1, 2, 3, 4, 5, 6, 7, 8, 9 FROM srvStatisticDataHistory

dataID     dataName    dataDescription
1   Turret Shots Fired  Shots fired from turrents on player ships.
2   Missiles Launched   Missiles fired from launchers on player ships.
3   Ships Salvaged?
4
5   PC Bounties   Amount of ISK paid for PC Bounties
6   NPC Bounties        Amount of ISK paid for NPC Bounties
7   Ore Mined   M3 of ore mined.
8   ISK Spent In Market     ISK spent in the market, not including broker fees...
9

*/
}

void StatisticMgr::Add(uint8 key, double value)
{
    m_data.span = sEntityList.GetMinutes();
    switch(key) {
        case Stat::pcBounties:
            m_data.pcBounties += value;
            break;
        case Stat::npcBounties:
            m_data.npcBounties += value;
            break;
        case Stat::oreMined:
            m_data.oreMined += value;
            break;
        case Stat::iskMarket:
            m_data.iskMarket += value;
            break;
        default:
            sLog.Error("StatisticMgr::Add()", "Default reached for key %u.", key );
            break;
    }
}

void StatisticMgr::Increment(uint8 key)
{
    m_data.span = sEntityList.GetMinutes();
    switch(key) {
        case Stat::pcShots:
            ++m_data.pcShots;
            break;
        case Stat::pcMissiles:
            ++m_data.pcMissiles;
            break;
        case Stat::shipsSalvaged:
            ++m_data.shipsSalvaged;
            break;
        default:
            sLog.Error("StatisticMgr::Increment()", "Default reached for key %u.", key );
            break;
    }
}

void StatisticMgr::PrintInfo()
{
    m_data.span = sEntityList.GetMinutes();
    sLog.Cyan("     StatisticMgr", " Time Span: %u minutes", m_data.span);
    sLog.Cyan("     StatisticMgr", " PC Shots Fired: %u", m_data.pcShots);
    sLog.Cyan("     StatisticMgr", " PC Missiles Fired: %u", m_data.pcMissiles);
    sLog.Cyan("     StatisticMgr", " PC Bounties Paid: %.2f isk", m_data.pcBounties);
    sLog.Cyan("     StatisticMgr", " NPC Bounties Paid: %.2f isk", m_data.npcBounties);
    sLog.Cyan("     StatisticMgr", " Ore Mined: %.2f m3", m_data.oreMined);
    sLog.Cyan("     StatisticMgr", " ISK Spent in Market: %.2f isk", m_data.iskMarket);
    sLog.Cyan("     StatisticMgr", " Ships Salvaged: %u", m_data.shipsSalvaged);
}

void StatisticMgr::SaveData()
{
    m_data.span = sEntityList.GetMinutes();
    ManagerDB::SaveStatisticData(m_data);
}


