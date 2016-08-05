
 /**
  * @name StaticDataMgr.h
  * caching system for solarSystem, station, and static celestial items.
  *  this is to avoid db hits EVERYTIME the shit is called...which is often (for EVERY login, dock, undock, jump, and market action)
  * @Author:         Allan
  * @date:   1Jul15 / 1Aug16
  *
  * Original Idea  - 1 July 15
  * Code completion and implementation  - 1 August 2016
  *
  */


#ifndef EVE_SERVER_SERVICESTRUCT_H__
#define EVE_SERVER_SERVICESTRUCT_H__

#include "eve-server.h"
#include "POD_containers.h"


class StaticDataMgr
: public Singleton< StaticDataMgr >
{
public:
    StaticDataMgr();
    ~StaticDataMgr()                                    { /* Do nothing here */ }

    void Init();
    void GetInfo();

    bool GetStaticInfo(uint32 itemID, StaticData& data);
    bool GetStationInfo(uint32 stationID, StationData& data);
    bool GetSystemInfo(uint32 systemID, SystemData& data);

private:
    std::map<uint32, uint32> m_stationSystem;
    std::map<uint32, SystemData> m_systemData;
    std::map<uint32, StaticData> m_staticData;
    std::map<uint32, StationData> m_stationData;

};

//Singleton
#define sDataMgr \
    ( StaticDataMgr::get() )


#endif  // EVE_SERVER_SERVICESTRUCT_H__
