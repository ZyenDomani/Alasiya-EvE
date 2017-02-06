
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

#include "map/MapDB.h"
#include "station/StationDB.h"
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

    bool IsSkillTypeID(uint16 typeID);
    bool GetSkillName(uint16 skillID, std::string& name);

    bool GetRamMaterials(uint16 typeID, std::vector<ramMaterials>& ramMatls);
    bool GetRamRequirements(uint16 typeID, std::vector<ramRequirements>& ramReqs);

    bool GetStaticInfo(uint32 itemID, StaticData& data);
    bool GetStationInfo(uint32 stationID, StationData& data);
    bool GetSystemInfo(uint32 locationID, SystemData& data);

    PyRep* GetStationCount();
    uint32 GetStationRegion(uint32 stationID);
    uint32 GetStationSystem(uint32 stationID);
    PyObject* GetStationData(uint32 stationID);

    bool GetRoidDist(const char* secClass, std::unordered_multimap< float, uint32 >& roids);
    uint8 GetRegionQuarter(uint32 regionID);
    uint16 GetRegionFaction(uint32 regionID);


    void GetItems(DBQueryResult& res);
    std::vector<uint16> m_items;

protected:
    void Populate();

private:
    MapDB m_mdb;
    ManagerDB m_db;
    StationDB m_sdb;

    std::map<uint32, uint32>        m_regions;          // regionID/factionID
    std::map<uint32, uint32>        m_stationRegion;    // stationID/regionID
    std::map<uint32, uint32>        m_stationSystem;    // stationID/systemID
    std::map<uint32, SystemData>    m_systemData;       // systemID/data
    std::map<uint32, StaticData>    m_staticData;       // itemID/data
    std::map<uint32, StationData>   m_stationData;      // stationID/data
    std::map<uint16, std::string>   m_skills;           // typeID/name

    std::unordered_multimap<uint16, ramMaterials>           m_ramMatl;          // itemTypeID/data
    std::unordered_multimap<uint16, ramRequirements>        m_ramReq;           // bpTypeID/data
    std::unordered_multimap<std::string, OreTypeChance>     m_oreBySecClass;    // systemSecClass/data

    std::map<uint32, PyObject*>     m_stationPyData;

    /* map data */
    std::map<uint32, uint8>         m_stationCount;
};

//Singleton
#define sDataMgr \
    ( StaticDataMgr::get() )


#endif  // _EVE_SERVER_STATIC_DATAMANAGER_H__
