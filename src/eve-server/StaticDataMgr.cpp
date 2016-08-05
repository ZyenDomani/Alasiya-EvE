
 /**
  * @name StaticDataMgr.cpp
  * caching system for solarSystem, station, and static celestial items.
  *  initial call will hit db to aquire data, but this system caches the data after that, avoiding subsquent db hits for same data.
  *  ..which is often (for EVERY login, dock, undock, jump, and market action)
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
}

void StaticDataMgr::Init()
{
    sLog.Success("    StaticDataMgr", "StaticDataMgr Initialized.");
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
bool StaticDataMgr::GetSystemInfo(uint32 systemID, SystemData& data)
{
    // this specific cache method is designed to use EITHER a stationID OR a systemID to determine system data wanted.
    if (IsStation(systemID)) {
        std::map<uint32, uint32>::iterator itr = m_stationSystem.find(systemID);
        if (itr != m_stationSystem.end()) {
            systemID = itr->second;
        } else {
            DBQueryResult res;
            if (!sDatabase.RunQuery(res, "SELECT solarSystemID FROM staStations WHERE stationID = %u", systemID)) {
                codelog(DATABASE__ERROR, "Failed to query info for station %u: %s.", systemID, res.error.c_str());
                return false;
            }

            DBResultRow row;
            if (!res.GetRow(row)) {
                _log(DATABASE__MESSAGE, "Failed to query info for station %u: Station not found.", systemID);
                return false;
            }
            m_stationSystem.insert(std::pair<uint32, uint32>(systemID, (systemID = row.GetUInt(0))));
        }
    }

    std::map<uint32, SystemData>::iterator itr = m_systemData.find(systemID);
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
            systemID))
        {
            codelog(DATABASE__ERROR, "Failed to query info for system %u: %s.", systemID, res.error.c_str());
            return false;
        }

        DBResultRow row;
        if (!res.GetRow(row)) {
            _log(DATABASE__MESSAGE, "Failed to query info for system %u: System not found.", systemID);
            return false;
        }

        data.systemID         = systemID;
        data.name             = row.GetText(0);
        data.constellationID  = row.GetUInt(1);
        data.regionID         = row.GetUInt(2);
        data.securityClass    = row.GetText(3);
        data.securityRating   = row.GetFloat(4);
        m_systemData.insert(std::pair<uint32, SystemData>(systemID, data));
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
