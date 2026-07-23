
/**
 * @name MapData.cpp
 *   a group of methods and functions to get map info.
 *     this is mostly used for getting random points in system, system jumps, and misc mission destination info
 *  - added static data for StationExtraInfo (from mapservice)
 * @Author:         Allan
 * @date:   13 November 2018
 */

#include "../StaticDataMgr.h"
#include "../EntityMgr.h"
#include "agents/Agent.h"
#include "map/MapData.h"
#include "map/MapDB.h"
#include "math/Trig.h"
#include "station/StationDataMgr.h"
#include "system/SystemManager.h"
#include "system/SystemEntity.h"

#include "../../eve-common/EVE_Map.h"


MapData::MapData()
: m_stationExtraInfo(nullptr),
m_pseudoSecurities(nullptr)
{
}

void MapData::Close()
{
    PySafeDecRef(m_stationExtraInfo);
    PySafeDecRef(m_pseudoSecurities);
}

int MapData::Initialize()
{
    sLog.Blue("          MapData", "Map Data Manager Initialized.");
    Populate();
    return 1;
}

void MapData::Clear()
{
    m_regionJumps.clear();
    m_constJumps.clear();
    m_systemJumps.clear();
}

void MapData::GetInfo()
{
    // print out list of bad jumps
    // m_badJumps
}

void MapData::Populate()
{
    m_pseudoSecurities = MapDB::GetPseudoSecurities();

    double start = GetTimeMSeconds();

    m_stationExtraInfo = new PyTuple(3);
    m_stationExtraInfo->items[0] = MapDB::GetStationExtraInfo();
    m_stationExtraInfo->items[1] = MapDB::GetStationOpServices();
    m_stationExtraInfo->items[2] = MapDB::GetStationServiceInfo();
    sLog.Cyan("          MapData", "StationExtraInfo loaded in %.3fms.",(GetTimeMSeconds() - start));

    // load system/constellation/region connections for MapData class
    start = GetTimeMSeconds();
    DBQueryResult* res = new DBQueryResult();
    MapDB::GetSystemJumps(*res);
    DBResultRow row;
    while (res->GetRow(row)) {
        //SELECT ctype, fromsol, tosol FROM mapConnections
        switch (row.GetInt(0)) {
            case Map::Jumptype::Region:
                m_regionJumps.emplace(row.GetInt(1), row.GetInt(2));
                break;
            case Map::Jumptype::Constellation:
                m_constJumps.emplace(row.GetInt(1), row.GetInt(2));
                break;
            default:
                m_systemJumps.emplace(row.GetInt(1), row.GetInt(2));
        }
    }

    sLog.Cyan("          MapData", "%lu Region jumps, %lu Constellation jumps and %lu System jumps loaded in %.3fms.", //
              m_regionJumps.size(), m_constJumps.size(), m_systemJumps.size(), (GetTimeMSeconds() - start));

    // cleanup
    SafeDelete(res);
}


// todo:  these celestials should be in static data
void MapData::GetPlanets(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBVector3dEntity> planetIDs;
    MapDB::GetPlanets(systemID, planetIDs, total);
}

// todo:  these celestials should be in static data
void MapData::GetMoons(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBVector3dEntity> moonIDs;
    MapDB::GetMoons(systemID, moonIDs, total);
}

// todo:  these celestials should be in static data
const Vector3d MapData::GetRandPointOnPlanet(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBVector3dEntity> planetIDs;
    MapDB::GetPlanets(systemID, planetIDs, total);

    if (planetIDs.empty())
        return NULL_ORIGIN;

    uint16 i = MakeRandomUInt(0, total - 1);
    Vector3d randPoint = planetIDs[i].position;
    randPoint.MakeRandomPointOnSphere(planetIDs[i].radius + 50000);
    return randPoint;
}

// todo:  these celestials should be in static data
const Vector3d MapData::GetRandPointOnMoon(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBVector3dEntity> moonIDs;
    MapDB::GetMoons(systemID, moonIDs, total);

    if (moonIDs.empty())
        return NULL_ORIGIN;

    uint16 i = MakeRandomUInt(0, total - 1);
    Vector3d randPoint = moonIDs[i].position;
    randPoint.MakeRandomPointOnSphere(moonIDs[i].radius + 10000);
    return randPoint;
}

// todo:  these celestials should be in static data
uint32 MapData::GetRandPlanet(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBVector3dEntity> planetIDs;
    MapDB::GetPlanets(systemID, planetIDs, total);

    if (planetIDs.empty())
        return 0;

    uint16 i = MakeRandomUInt(0, total - 1);
    return planetIDs[i].itemID;
}

// todo:  these celestials should be in static data
const Vector3d MapData::Get2RandPlanets(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBVector3dEntity> planetIDs;
    MapDB::GetPlanets(systemID, planetIDs, total);
    /** @todo finish this */
    return NULL_ORIGIN;
}

// todo:  these celestials should be in static data
const Vector3d MapData::Get3RandPlanets(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBVector3dEntity> planetIDs;
    MapDB::GetPlanets(systemID, planetIDs, total);
    /** @todo finish this */

    return NULL_ORIGIN;
}

// todo:  these celestials should be in static data
uint32 MapData::GetRandMoon(uint32 systemID) {
    uint8 total = 0;
    std::vector<DBVector3dEntity> moonIDs;
    MapDB::GetMoons(systemID, moonIDs, total);

    if (moonIDs.empty())
        return 0;

    uint16 i = MakeRandomUInt(0, total - 1);
    return moonIDs[i].itemID;
}

const Vector3d MapData::GetRandPointInSystem(uint32 systemID, int64 distance/*0*/) {
    // get system max diameter, verify distance is within system.
    SolarSystemData data = SolarSystemData();
    sDataMgr.GetSolarSystemData(systemID, data);

    // check given distance is within system boundary
    if (distance > data.radius)
        return NULL_ORIGIN;

    // get random distance from origin, unless given
    if (distance == 0)
        distance = MakeRandomLong(data.radius / 10, data.radius);

    // get random angle (for x,z)
    double theta = MakeRandomFloat(0, (EvE::Trig::Pi * 2));

    // set x,z based on random angle and distance from origin
    Vector3d pos = NULL_ORIGIN;
    pos.x = distance * cos(theta);
    pos.z = distance * sin(theta);

    // get random elevation (y)
    pos.y = MakeRandomFloat(-10000, 10000);

    // should we verify proximity to celestial objects?

    return pos;
}

const Vector3d MapData::GetAnomalyPoint(SystemManager* pSys)
{
    uint8 total = 0;
    std::vector<DBVector3dEntity> planetIDs;
    MapDB::GetPlanets(pSys->GetID(), planetIDs, total);

    SystemEntity* pSE(pSys->GetSE(planetIDs[MakeRandomUInt(0, total - 1)].itemID));

    Vector3d pos = pSE->GetPosition();
    pos.MakeRandomPointOnSphereLayer(ONE_AU_IN_METERS / 3, ONE_AU_IN_METERS * 4);
    return pos;
}

const Vector3d MapData::GetAnomalyPoint(uint32 systemID)
{
    uint8 total = 0;
    std::vector<DBVector3dEntity> planetIDs;
    MapDB::GetPlanets(systemID, planetIDs, total);
    Vector3d pos = planetIDs[MakeRandomInt(0, total - 1)].position;
    pos.MakeRandomPointOnSphereLayer(ONE_AU_IN_METERS / 3, ONE_AU_IN_METERS * 4);
    return pos;
}

bool MapData::GetSystemJumps(uint8 step, uint32 sysID, std::multimap<uint8, uint32>& jumpMap) {
    auto range = m_systemJumps.equal_range(sysID);
    for (auto it = range.first; it != range.second; ++it)
        jumpMap.emplace(step, it->second);
    return !jumpMap.empty();
}

void MapData::GetMissionDestination(Agent* pAgent, MissionOffer& offer) {
    uint8 destRange(pAgent->GetLevel());
    if (pAgent->GetTypeID() != Agents::Type::Tutorial)
        ++destRange;
    bool station = true, ship = false, deadspace = false;    // will have to tweak this later for particular mission events

    // determine distance based on preset range from db or in some cases, mission type and agent level
    switch(offer.typeID) {
        case Mission::Type::Courier: {
            // destination will always be station
        } break;
        case Mission::Type::Tutorial: {
            station = false;
            // always same system?  yes
            destRange = Agents::Range::SameSystem;
        } break;
        case Mission::Type::Data:
        case Mission::Type::Trade:
        case Mission::Type::Research: {
            // not sure on this one yet
        } break;
        case Mission::Type::Arc:
        case Mission::Type::Cosmos: {
            // not sure if this is right yet...
            ship = true;
        }
        case Mission::Type::Anomic:
        case Mission::Type::Burner:
        case Mission::Type::Circle:
        case Mission::Type::Mining:
        case Mission::Type::Encounter:
        case Mission::Type::Storyline: {
            station = false;
            deadspace = true;
        } break;
    }

    /*    Border = Borders another Region or Constellation
     *    Fringe = 1 connection to this system (dead end system)
     *    Corridor = 2 connections to this system (in one side and out the other)
     *    Hub = 3+ connections to this system
     *    Regional = borders another region
     *    Constellation = borders another constellation
     *  97 regions
     *  1109 constellations
     *  7929 systems
     */
    uint32 agentSystemID = pAgent->GetSystemID();
    SystemManager* pSysMgr = sEntityMgr.FindOrBootSystem(agentSystemID);
    if (pSysMgr == nullptr) {
        // should never hit, but whatever
        StationData data = StationData();
        stDataMgr.GetStationData(pAgent->GetStationID(), data);
        offer.destinationOwnerID    = data.corporationID;
        offer.destinationSystemID   = data.systemID;
        offer.destinationTypeID     = data.typeID;
        sLog.Error("MapData::GetMissionDestination()", "pSysMgr = null for systemID %u", agentSystemID);
        return;
    }

    bool run = false;
    std::vector<uint32> sysList;
    switch(destRange) {
        case 0:
        case Agents::Range::SameSystem: {
            if (station || deadspace) {
                // If we need a station but this system has no other stations, look 1 jump out
                if (station && sDataMgr.GetStationCount(agentSystemID) < 2) {
                    auto range = m_systemJumps.equal_range(agentSystemID);
                    for (auto it = range.first; it != range.second; ++it) {
                        if ((sDataMgr.GetSystemConstellation(it->second) == sDataMgr.GetSystemConstellation(agentSystemID))
                            && (sDataMgr.GetSystemRegion(it->second) == sDataMgr.GetSystemRegion(agentSystemID))) {
                            sysList.push_back(it->second);
                            }
                    }
                } else {
                    sysList.push_back(agentSystemID);
                }
            }
        } break;

        case Agents::Range::SameOrNeighboringSystem:
        case Agents::Range::NeighboringSystem: {
            // Get all neighboring systems exactly 1 jump out
            auto JumpItr = m_systemJumps.equal_range(agentSystemID);
            for (auto it = JumpItr.first; it != JumpItr.second; ++it) {
                if ((sDataMgr.GetSystemConstellation(it->second) == sDataMgr.GetSystemConstellation(agentSystemID))
                    && (sDataMgr.GetSystemRegion(it->second) == sDataMgr.GetSystemRegion(agentSystemID))) {
                    sysList.push_back(it->second);
                    }
            }
            // For SameOrNeighboring, occasionally allow picking the home system
            if (destRange == Agents::Range::SameOrNeighboringSystem && MakeRandomInt() > 70) {
                sysList.push_back(agentSystemID);
            }
        } break;

        // dont know how to get route/jumps from start to constellation edges...yet
        case Agents::Range::SameConstellation: {
            uint32 constellationID = sDataMgr.GetSystemConstellation(agentSystemID);
            std::vector<uint32> systemVec;
            sDataMgr.GetConstellationSystems(constellationID, systemVec);
            for (auto &cur : systemVec) {
                if (!sDataMgr.IsConSystem(cur) && !sDataMgr.IsRegionSystem(cur)) {
                    sysList.push_back(cur);
                }
            }
        } break;

        case Agents::Range::NeighboringConstellation: {
            uint32 constellationID = sDataMgr.GetSystemConstellation(agentSystemID);
            std::vector<uint32> systemVec;
            // FIX: Pass the correct constellation ID array tracker, not Region!
            sDataMgr.GetConstellationSystems(constellationID, systemVec);
            for (auto &cur : systemVec) {
                if (sDataMgr.IsConSystem(cur)) {
                    auto JumpItr = m_systemJumps.equal_range(cur);
                    for (auto it = JumpItr.first; it != JumpItr.second; ++it) {
                        if (sDataMgr.GetSystemConstellation(it->second) != constellationID) {
                            sysList.push_back(it->second);
                        }
                    }
                }
            }
        } break;

        case Agents::Range::NeighboringRegion: {
            uint32 regionID = sDataMgr.GetSystemRegion(agentSystemID);
            std::vector<uint32> systemVec;
            sDataMgr.GetRegionSystems(regionID, systemVec);
            for (auto &cur : systemVec) {
                if (sDataMgr.IsRegionSystem(cur)) {
                    auto JumpItr = m_systemJumps.equal_range(cur);
                    for (auto it = JumpItr.first; it != JumpItr.second; ++it) {
                        if (sDataMgr.GetSystemRegion(it->second) != regionID) {
                            sysList.push_back(it->second);
                        }
                    }
                }
            }
        } break;

        // not sure how to do these two yet...
        case Agents::Range::NearestEnemyCombatZone: {  //10
        } break;
        case Agents::Range::NearestCareerHub: {    //11
        } break;
        default:
            sysList.push_back(agentSystemID); // Safe fallback
            break;
    }

    if (sysList.empty()) {
        offer.destinationID = pAgent->GetStationID();
        sLog.Error("MapData::GetMissionDestination", "sysList completely empty for systemID %u", agentSystemID);
        return;
    }

    // --- SAFE SELECTION VALVE ---
    uint32 selectedSystemID = 0;
    bool foundValidSystem = false;

    // Determine the security profile of the agent's home system
    float agentSecurity = pSysMgr->GetSecurityRating();
    bool isAgentInHighsec = (agentSecurity >= 0.5f);

    // Shuffle or randomly sample candidate systems until one passes your criteria
    while (!sysList.empty()) {
        uint32 randIdx = MakeRandomUInt(0, sysList.size() - 1);
        uint32 candidateSystemID = sysList.at(randIdx);

        if (station && sDataMgr.GetStationCount(candidateSystemID) < 1) {
            // Fails station check, strip it out and try a different system
            sysList.erase(sysList.begin() + randIdx);
            continue;
        }

        // 2. CRITICAL MATCH: Security Status Boundaries Check
        /*  dont have a 'good' way of testing this yet without booting solsystem
        float candidateSecurity = sDataMgr.GetSystemSecurityRating(candidateSystemID);

        if (isAgentInHighsec && candidateSecurity < 0.5f) {
            // Highsec agents are forbidden from forcing couriers into Lowsec/Nullsec!
            // Strip the system out of the options and keep looking.
            sysList.erase(sysList.begin() + randIdx);
            continue;
        } */

        // Found a winner!
        selectedSystemID = candidateSystemID;
        foundValidSystem = true;
        break;
    }

    if (!foundValidSystem) {
        // Absolute safety fallback to avoid freezing or empty items
        selectedSystemID = agentSystemID;
        offer.destinationID = pAgent->GetStationID();
        sLog.Error("MapData::GetMissionDestination", "No system passed station constraints. Falling back to home.");
    }

    // --- STATION RESOLUTION ---
    if (station) {
        std::vector<uint32> stationList;
        sDataMgr.GetStationList(selectedSystemID, stationList);

        if (stationList.empty()) {
            offer.destinationID = pAgent->GetStationID();
        } else {
            // Filter out the agent's own station if multiple options exist
            std::vector<uint32> filteredStations;
            for (uint32 stID : stationList) {
                if (stID != pAgent->GetStationID()) {
                    filteredStations.push_back(stID);
                }
            }

            if (!filteredStations.empty()) {
                offer.destinationID = filteredStations.at(MakeRandomUInt(0, filteredStations.size() - 1));
            } else {
                offer.destinationID = stationList[0]; // Fallback if it's the only station
            }
        }
    } else {
        offer.destinationID = selectedSystemID;
    }

    // --- FINAL DATA PACKING ---
    if (sDataMgr.IsStation(offer.destinationID)) {
        StationData data = StationData();
        stDataMgr.GetStationData(offer.destinationID, data);
        offer.destinationOwnerID    = data.corporationID;
        offer.destinationSystemID   = data.systemID;
        offer.destinationTypeID     = data.typeID;
    } else {
        offer.destinationSystemID   = offer.destinationID;
        offer.destinationTypeID     = sDataMgr.GetStaticType(offer.destinationID);
        offer.dungeonLocationID     = offer.destinationID;
        offer.dungeonSolarSystemID  = offer.destinationID;
    }
}
