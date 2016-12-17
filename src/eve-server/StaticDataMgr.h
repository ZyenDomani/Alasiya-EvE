
 /**
  * @name StaticDataMgr.h
  *   memory object caching system for managing and saving ingame data
  *
  * @Author:         Allan
  * @date:   1Jul15 / 1Aug16
  *
  * Original Idea  - 1 July 15
  * Code completion and implementation  - 1 August 2016
  *
  */


#ifndef _EVE_SERVER_STATIC_DATAMANAGER_H__
#define _EVE_SERVER_STATIC_DATAMANAGER_H__


#include <unordered_map>

#include "eve-server.h"
#include "POD_containers.h"

#include "system/cosmicMgrs/ManagerDB.h"

class StaticDataMgr
: public Singleton< StaticDataMgr >
{
public:
    StaticDataMgr();
    ~StaticDataMgr();

    int Initialize();

    void Clear();
    void GetInfo();

    bool GetStaticInfo(uint32 itemID, StaticData& data);
    bool GetStationInfo(uint32 stationID, StationData& data);
    bool GetSystemInfo(uint32 systemID, SystemData& data);

    bool GetRoidDist(const char* secClass, std::unordered_multimap< float, uint32 >& roids);
    uint8 GetRegionQuarter(uint32 regionID);
    uint16 GetRegionFaction(uint32 regionID);

protected:
    void Populate();

private:
    ManagerDB m_db;

    std::map<uint32, uint32> m_stationSystem;
    std::map<uint32, SystemData> m_systemData;
    std::map<uint32, StaticData> m_staticData;
    std::map<uint32, StationData> m_stationData;

    std::map<uint32, uint32> m_regions;   // this simple map holds k,v of regionID/factionID
    std::unordered_multimap<std::string, OreTypeChance> m_oreBySecClass;
};

//Singleton
#define sDataMgr \
    ( StaticDataMgr::get() )


#endif  // _EVE_SERVER_STATIC_DATAMANAGER_H__
