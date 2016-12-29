
 /**
  * @name StaticDataMgr.cpp
  *   memory object caching system for managing and saving ingame data
  *
  * @Author:         Allan
  * @date:   1Jul15 / 1Aug16
  *
  * Original Idea  - 1 July 15
  * Code completion and implementation  - 1 August 2016
  *
  */


#include "StaticDataMgr.h"


StaticDataMgr::StaticDataMgr()
{
    Clear();
}

StaticDataMgr::~StaticDataMgr()
{
    Clear();
}

int StaticDataMgr::Initialize()
{
    Populate();
    return 1;
}

void StaticDataMgr::Clear()
{
    m_regions.clear();
    m_systemData.clear();
    m_staticData.clear();
    m_stationData.clear();
    m_stationSystem.clear();
    m_oreBySecClass.clear();
}

void StaticDataMgr::Populate()
{
    double start = GetTimeUSeconds();
    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;

    m_db.GetOreBySSC(*res);
    OreTypeChance oreChance;
    oreChance.typeID  = 0;
    oreChance.chance  = 0;
    while (res->GetRow(row)) {
        //SELECT systemSec, roidID, percent FROM roidDistribution
        oreChance.typeID  = row.GetInt(1);
        oreChance.chance  = row.GetFloat(2);
        m_oreBySecClass.insert(std::pair<std::string, OreTypeChance>(row.GetText(0), oreChance));
    }

    res->Reset();
    m_db.GetRegionFaction(*res);
    while (res->GetRow(row)) {
        //SELECT regionID, factionID FROM mapRegions
        m_regions.insert(std::pair<uint32, uint32>(row.GetInt(0), row.GetInt(1)));
    }

    //cleanup
    SafeDelete(res);
    sLog.Cyan("    StaticDataMgr", "%u ore data sets and %u region factions loaded in %.3fms.", m_oreBySecClass.size(), m_regions.size(), (GetTimeUSeconds() - start));
}

void StaticDataMgr::GetInfo()
{
    /* return info about loaded items? */
    /*
     * m_stationSystem
     * m_systemData;
     * m_staticData;
     * m_stationData;
     *
     */
}

//  the system data is cached on initial boot of system
bool StaticDataMgr::GetSystemInfo(uint32 locationID, SystemData& data)
{
    // this specific cache method is designed to use EITHER a stationID OR a systemID to determine system data wanted.
    if (IsStation(locationID)) {
        std::map<uint32, uint32>::iterator itr = m_stationSystem.find(locationID);
        if (itr != m_stationSystem.end()) {
            locationID = itr->second;
        } else {
            DBQueryResult res;
            if (!sDatabase.RunQuery(res, "SELECT solarSystemID FROM staStations WHERE stationID = %u", locationID)) {
                codelog(DATABASE__ERROR, "Failed to query info for station %u: %s.", locationID, res.error.c_str());
                return false;
            }

            DBResultRow row;
            if (!res.GetRow(row)) {
                _log(DATABASE__MESSAGE, "Failed to query info for station %u: Station not found.", locationID);
                return false;
            }
            m_stationSystem.insert(std::pair<uint32, uint32>(locationID, (locationID = row.GetUInt(0))));
        }
    } else if (!IsSolarSystem(locationID)) {
        _log(SERVICE__WARNING, "Failed to query info:  locationID %u is neither station nor system.", locationID);
        return false;
    }

    std::map<uint32, SystemData>::iterator itr = m_systemData.find(locationID);
    if (itr != m_systemData.end()) {
        data = itr->second;
    } else {
        DBQueryResult res;
        if (!sDatabase.RunQuery(res,
            "SELECT"
            " solarSystemName,"
            " constellationID,"
            " regionID,"
            " securityClass,"
            " security"
            " FROM mapSolarSystems"
            " WHERE solarSystemID = %u",
                                locationID))
        {
            codelog(DATABASE__ERROR, "Failed to query info for system %u: %s.", locationID, res.error.c_str());
            return false;
        }

        DBResultRow row;
        if (!res.GetRow(row)) {
            _log(DATABASE__MESSAGE, "Failed to query info for system %u: System not found.", locationID);
            return false;
        }

        data.systemID          = locationID;
        data.name              = row.GetText(0);
        data.constellationID   = row.GetUInt(1);
        data.regionID          = row.GetUInt(2);
        if (row.IsNull(3))
            data.securityClass = "0";
        else
            data.securityClass = row.GetText(3);
        data.securityRating    = row.GetFloat(4);
        m_systemData.insert(std::pair<uint32, SystemData>(locationID, data));
    }
    return true;
}

bool StaticDataMgr::GetStaticInfo(uint32 itemID, StaticData& data)
{
    std::map<uint32, StaticData>::iterator itr = m_staticData.find(itemID);
    if (itr != m_staticData.end()) {
        data = itr->second;
    } else {
        DBQueryResult res;
        if (!sDatabase.RunQuery(res,
            "SELECT"
            " regionID,"
            " constellationID,"
            " solarSystemID,"
            " x, y, z"
            " FROM mapDenormalize"
            " WHERE itemID = %u",
            itemID))
        {
            codelog(DATABASE__ERROR, "Failed to query info for static item %u: %s.", itemID, res.error.c_str());
            return false;
        }

        DBResultRow row;
        if (!res.GetRow(row)) {
            _log(DATABASE__MESSAGE, "Failed to query info for static item %u: Item not found.", itemID);
            return false;
        }

        data.itemID          = itemID;
        data.regionID        = row.GetUInt(0);
        data.constellationID = row.GetUInt(1);
        data.systemID        = row.GetUInt(2);
        data.position        = GPoint(row.GetDouble(3),row.GetDouble(4),row.GetDouble(5));

        m_staticData.insert(std::pair<uint32, StaticData>(itemID, data));
    }
    return true;
}

bool StaticDataMgr::GetStationInfo(uint32 stationID, StationData& data)
{
    std::map<uint32, StationData>::iterator itr = m_stationData.find(stationID);
    if (itr != m_stationData.end()) {
        data = itr->second;
    } else {
        DBQueryResult res;
        if (!sDatabase.RunQuery(res,
            "SELECT"
            " s.x, s.y, s.z,"
            " st.dockEntryX, st.dockEntryY, st.dockEntryZ,"
            " st.dockOrientationX, st.dockOrientationY, st.dockOrientationZ"
            " FROM staStations AS s"
            " LEFT JOIN staStationTypes AS st USING (stationTypeID)"
            " WHERE s.stationID = %u",
            stationID))
        {
            codelog(DATABASE__ERROR, "Failed to query info for station %u: %s.", stationID, res.error.c_str());
            return false;
        }

        DBResultRow row;
        if (!res.GetRow(row)) {
            _log(DATABASE__MESSAGE, "Failed to query info for station %u: Station not found.", stationID);
            return false;
        }

        data.stationID       = stationID;
        data.position        = GPoint(row.GetDouble(0),row.GetDouble(1),row.GetDouble(2));
        data.dockPosition    = GPoint(row.GetDouble(3) + row.GetDouble(0),
                                      row.GetDouble(4) + row.GetDouble(1),
                                      row.GetDouble(5) + row.GetDouble(2));
        data.dockOrientation = GVector(row.GetDouble(6),row.GetDouble(7),row.GetDouble(8));

        m_stationData.insert(std::pair<uint32, StationData>(stationID, data));
    }
    return true;
}

uint16 StaticDataMgr::GetRegionFaction(uint32 regionID)
{
    std::map<uint32, uint32>::iterator itr = m_regions.find(regionID);
    if (itr != m_regions.end())
        return (*itr).second;
    return 0;
}

uint8 StaticDataMgr::GetRegionQuarter(uint32 regionID)
{
    uint32 factionID = 0;
    std::map<uint32, uint32>::iterator itr = m_regions.find(regionID);
    if (itr != m_regions.end())
        factionID = (*itr).second;

    // caldari=1, minmatar=2, amarr=3, gallente=4, none=5
    switch (factionID) {
        case factionCaldari:    //Caldari State
        case factionGuristas:   //Guristas Pirates
            return 1; break;
        case factionMinmatar:   //Minmatar Republic
        case factionAngel:      //Angel Cartel
            return 2; break;
        case 500003:    //Amarr Empire
        case 500007:    //Ammatar Mandate
        case 500008:    //Khanid Kingdom
        case 500012:    //Blood Raider Covenant
        case 500019:    //Sansha's Nation
            return 3; break;
        case 500004:    //Gallente Federation
        case 500020:    //Serpentis
            return 4; break;
        case 500005:    //Jove Empire
        case 500006:    //CONCORD Assembly
        case 500009:    //The Syndicate
        case factionInterBus:    //The InterBus
        case 500014:    //ORE
        case 500015:    //Thukker Tribe
        case 500016:    //Servant Sisters of EVE
        case 500017:    //The Society of Conscious Thought
        case 500018:    //Mordu's Legion Command
            return 5; break;
    }
}

bool StaticDataMgr::GetRoidDist(const char* secClass, std::unordered_multimap< float, uint32 >& roids) {
    auto groupRange = m_oreBySecClass.equal_range(secClass);
    for (auto it = groupRange.first; it != groupRange.second; ++it) {
        _log(COSMIC_MGR__MESSAGE, "GetRoidDist - adding %u with chance %.3f", it->second.typeID, it->second.chance);
        roids.insert(std::pair<float, uint32>(it->second.chance, it->second.typeID));
    }

    return !roids.empty();
}
