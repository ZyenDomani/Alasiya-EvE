
 /**
  * @name PlanetDataMgr.cpp
  *   Specific Class for managing planet and pi data
  * @Author:         Allan
  * @date:   30 November 2016
  */


#include "eve-server.h"

#include "planet/PlanetDataMgr.h"


PlanetDataMgr::PlanetDataMgr()
{
}

int PlanetDataMgr::Initialize()
{
    _Populate();
    sLog.Blue("    PlanetDataMgr", "Planet Data Manager Initialized.");
    return 1;
}

void PlanetDataMgr::_Populate()
{
    double start = GetTimeMSeconds();
    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;

    m_db.GetPlanetData(*res);
    while (res->GetRow(row)) {
        // SELECT planet.typeID, resource.typeID
        m_planetData.insert(std::pair<uint32, uint32>(row.GetInt(0), row.GetInt(1)));
    }

    //cleanup
    SafeDelete(res);
    sLog.Cyan("    PlanetDataMgr", "%u planet data groups in %u buckets loaded in %.3fms.",\
            m_planetData.size(), m_planetData.bucket_count(), (GetTimeMSeconds() - start));
}

void PlanetDataMgr::GetPlanetData(uint32 planetID, std::vector<uint32> &typeIDs)
{
    auto itr = m_planetData.equal_range(planetID);
    for (auto it = itr.first; it != itr.second; it++)
        typeIDs.push_back(it->second);
}


PIDataMgr::PIDataMgr()
{
}

int PIDataMgr::Initialize()
{
    _Populate();
    sLog.Blue("        PIDataMgr", "Planet Interaction Data Manager Initialized.");
    return 1;
}

// do we need anything from piPinMap?
void PIDataMgr::_Populate()
{
    double start = GetTimeMSeconds();
    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;

    m_db.GetSchematicData(*res);
    while (res->GetRow(row)) {
        // SELECT `schematicID`, `typeID`, `quantity`, `isInput`
        std::map<uint8, PI_Schematic>::iterator itr = m_schematicData.find(row.GetInt(0));
        if (itr != m_schematicData.end()) {
            if (row.GetInt(3))
                itr->second.inputs.insert(std::pair<uint16, uint16>(row.GetInt(1), row.GetInt(2)));
            else {
                itr->second.outputType = row.GetInt(1);
                itr->second.outputQty = row.GetInt(2);
            }
        } else {
            PI_Schematic data;
            if (row.GetInt(3))
                data.inputs.insert(std::pair<uint16, uint16>(row.GetInt(1), row.GetInt(2)));
            else {
                data.outputType = row.GetInt(1);
                data.outputQty = row.GetInt(2);
            }
            m_schematicData.insert(std::pair<uint8, PI_Schematic>(row.GetInt(0), data));
        }
    }

    //clear previous data set
    res->Reset();

    m_db.GetSchematicTimes(*res);
    while (res->GetRow(row)) {
        // SELECT `schematicID`, `cycleTime`
        std::map<uint8, PI_Schematic>::iterator itr = m_schematicData.find(row.GetInt(0));
        if (itr != m_schematicData.end())
            itr->second.cycleTime = (row.GetInt(1) * Win32Time_Minute);
    }

    //cleanup
    SafeDelete(res);
    sLog.Cyan("        PIDataMgr", "%u PI Schematic data groups loaded in %.3fms.", m_schematicData.size(), (GetTimeMSeconds() - start));
}

void PIDataMgr::GetSchematicData(uint16 schematicID, PI_Schematic& data)
{
    std::map<uint8, PI_Schematic>::iterator itr = m_schematicData.find(schematicID);
    if (itr != m_schematicData.end()) {
        data = itr->second;
        return;
    }
    // make error here
}



