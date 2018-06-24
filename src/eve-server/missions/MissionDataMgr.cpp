
 /**
  * @name MissionDataMgr.h
  *   memory object caching system for managing and saving ingame data specific to missions
  *
  * @Author:        Allan
  * @date:      24 June 2018
  *
  */


#include "missions/MissionDataMgr.h"
#include "database/EVEDBUtils.h"

MissionDataMgr::MissionDataMgr()
{

}

MissionDataMgr::~MissionDataMgr()
{

}

void MissionDataMgr::Clear()
{

}

int MissionDataMgr::Initialize()
{
    Populate();
    return 1;

}

void MissionDataMgr::GetInfo()
{

}

void MissionDataMgr::Populate()
{
    double start = GetTimeMSeconds();

    sLog.Cyan("   MissionDataMgr", "0 Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));
}
