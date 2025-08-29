
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
    uint8 total(0);
    std::vector<DBGPointEntity> planetIDs;
    MapDB::GetPlanets(systemID, planetIDs, total);
}

// todo:  these celestials should be in static data
void MapData::GetMoons(uint32 systemID) {
    uint8 total(0);
    std::vector<DBGPointEntity> moonIDs;
    MapDB::GetMoons(systemID, moonIDs, total);
}

// todo:  these celestials should be in static data
const GPoint MapData::GetRandPointOnPlanet(uint32 systemID) {
    uint8 total(0);
    std::vector<DBGPointEntity> planetIDs;
    MapDB::GetPlanets(systemID, planetIDs, total);

    if (planetIDs.empty())
        return NULL_ORIGIN;

    uint16 i = MakeRandomInt(0, total - 1);
    return (planetIDs[i].position + planetIDs[i].radius + 50000);
}

// todo:  these celestials should be in static data
const GPoint MapData::GetRandPointOnMoon(uint32 systemID) {
    uint8 total(0);
    std::vector<DBGPointEntity> moonIDs;
    MapDB::GetMoons(systemID, moonIDs, total);

    if (moonIDs.empty())
        return NULL_ORIGIN;

    uint16 i = MakeRandomInt(0, total - 1);
    return (moonIDs[i].position + moonIDs[i].radius + 10000);
}

// todo:  these celestials should be in static data
uint32 MapData::GetRandPlanet(uint32 systemID) {
    uint8 total(0);
    std::vector<DBGPointEntity> planetIDs;
    MapDB::GetPlanets(systemID, planetIDs, total);

    if (planetIDs.empty())
        return 0;

    uint16 i = MakeRandomInt(0, total - 1);
    return planetIDs[i].itemID;
}

// todo:  these celestials should be in static data
const GPoint MapData::Get2RandPlanets(uint32 systemID) {
    uint8 total(0);
    std::vector<DBGPointEntity> planetIDs;
    MapDB::GetPlanets(systemID, planetIDs, total);
    /** @todo finish this */
    return NULL_ORIGIN;
}

// todo:  these celestials should be in static data
const GPoint MapData::Get3RandPlanets(uint32 systemID) {
    uint8 total(0);
    std::vector<DBGPointEntity> planetIDs;
    MapDB::GetPlanets(systemID, planetIDs, total);
    /** @todo finish this */

    return NULL_ORIGIN;
}

// todo:  these celestials should be in static data
uint32 MapData::GetRandMoon(uint32 systemID) {
    uint8 total(0);
    std::vector<DBGPointEntity> moonIDs;
    MapDB::GetMoons(systemID, moonIDs, total);

    if (moonIDs.empty())
        return 0;

    uint16 i = MakeRandomInt(0, total);
    return moonIDs[i].itemID;
}

const GPoint MapData::GetRandPointInSystem(uint32 systemID, int64 distance/*0*/) {
    // get system max diameter, verify distance is within system.
    SolarSystemData data = SolarSystemData();
    sDataMgr.GetSolarSystemData(systemID, data);

    // check given distance is within system boundary
    if (distance > data.radius)
        return NULL_ORIGIN;

    // get random distance from origin, unless given
    if (distance == 0)
        distance = MakeRandomInt(data.radius / 10, data.radius);

    // get random angle (for x,z)
    double theta = MakeRandomFloat(0, (EvE::Trig::Pi * 2));

    // set x,z based on random angle and distance from origin
    GPoint pos(NULL_ORIGIN);
    pos.x = distance * cos(theta);
    pos.z = distance * sin(theta);

    // get random elevation (y)
    pos.y = MakeRandomFloat(-10000, 10000);

    // should we verify proximity to celestial objects?

    return pos;
}

const GPoint MapData::GetAnomalyPoint(SystemManager* pSys)
{
    uint8 total(0);
    std::vector<DBGPointEntity> planetIDs;
    MapDB::GetPlanets(pSys->GetID(), planetIDs, total);

    SystemEntity* pSE(pSys->GetSE(planetIDs[MakeRandomInt(0, total - 1)].itemID));

    GPoint pos(pSE->GetPosition());
    pos.MakeRandomPointOnSphereLayer(ONE_AU_IN_METERS / 3, ONE_AU_IN_METERS * 4);
    return pos;
}

const GPoint MapData::GetAnomalyPoint(uint32 systemID)
{
    uint8 total(0);
    std::vector<DBGPointEntity> planetIDs;
    MapDB::GetPlanets(systemID, planetIDs, total);
    GPoint pos(planetIDs[MakeRandomInt(0, total - 1)].position);
    pos.MakeRandomPointOnSphereLayer(ONE_AU_IN_METERS / 3, ONE_AU_IN_METERS * 4);
    return pos;
}

bool MapData::GetSystemJumps(uint8 step, uint32 sysID, std::multimap<uint8, uint32>& jumpMap) {
    auto JumpItr = m_systemJumps.equal_range(sysID);
    auto it = JumpItr.first;
    if (it == JumpItr.second)
        return false;
    for (; it != JumpItr.second; ++it)
        jumpMap.emplace(step, it->second);
    return true;
}

/**  @TODO
 * this has errors
 *
 * (not actual msg)
 * MapData::GetMissionDestination(run) - no station found within 1 jump of systemID
 */

void MapData::GetMissionDestination(Agent* pAgent, MissionOffer& offer) {
    uint8 destRange(pAgent->GetLevel());
    if (pAgent->GetTypeID() != Agents::Type::Tutorial)
        ++destRange;
    bool station(true), ship(false), deadspace(false);    // will have to tweak this later for particular mission events

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
    uint32 systemID(pAgent->GetSystemID());
    SystemManager* pSysMgr = sEntityMgr.FindOrBootSystem(systemID);
    if (pSysMgr == nullptr) {
        // should never hit, but whatever
        StationData data = StationData();
        stDataMgr.GetStationData(pAgent->GetStationID(), data);
        offer.destinationOwnerID    = data.corporationID;
        offer.destinationSystemID   = data.systemID;
        offer.destinationTypeID     = data.typeID;
        sLog.Error("MapData::GetMissionDestination()", "pSysMgr = null for systemID %u", systemID);
        return;
    }

    //TODO: once i get this working, check security  (does it matter?)
    bool run(true);
    std::vector<uint32> sysList;
    switch(destRange) {
        case 0:
        case Agents::Range::SameSystem: { //1
            if (station or deadspace) {
                if (sDataMgr.GetStationCount(systemID) < 2) {
                    // no other station here; get gates out of this system for single jump
                    auto JumpItr = m_systemJumps.equal_range(systemID);
                    for (auto it = JumpItr.first; it != JumpItr.second; ++it) {
                        // make sure this is not border jump
                        if ((sDataMgr.GetSystemConstellation(it->second) == sDataMgr.GetSystemConstellation(systemID))
                        and (sDataMgr.GetSystemRegion(it->second) == sDataMgr.GetSystemRegion(systemID)))
                            sysList.push_back(it->second);
                    }
                }
            } else if (ship) {
                // not sure how this is gonna work yet
            } else {
                // what other checks should we make here?
                sLog.Error("MapData::GetMissionDestination(1)", "check is 'else' for systemID %u", systemID);
            }
        } break;

        case Agents::Range::SameOrNeighboringSystem: { //2
            if (station or deadspace) {
                if (IsEven(MakeRandomInt(0, 20))) {
                    // neighboring system...get gates out of this system...single jump
                    auto JumpItr = m_systemJumps.equal_range(systemID);
                    for (auto it = JumpItr.first; it != JumpItr.second; ++it) {
                        // make sure this is not border jump
                        if ((sDataMgr.GetSystemConstellation(it->second) == sDataMgr.GetSystemConstellation(systemID))
                        and (sDataMgr.GetSystemRegion(it->second) == sDataMgr.GetSystemRegion(systemID)))
                            sysList.push_back(it->second);
                    }
                }
                // else same system...do nothing here.  sysList.empty() check will set needed variables
            } else if (ship) {
                // not sure how this is gonna work yet
            } else {
                // what other checks should we make here?
                sLog.Error("MapData::GetMissionDestination(2)", "check is 'else' for systemID %u", systemID);
            }
        } break;

        case Agents::Range::NeighboringSystem: {  //3
            if (station or deadspace) {
                // get gates out of this system...single jump
                auto JumpItr = m_systemJumps.equal_range(systemID);
                for (auto it = JumpItr.first; it != JumpItr.second; ++it) {
                    // make sure this is not border jump
                    if ((sDataMgr.GetSystemConstellation(it->second) == sDataMgr.GetSystemConstellation(systemID))
                    and (sDataMgr.GetSystemRegion(it->second) == sDataMgr.GetSystemRegion(systemID)))
                        sysList.push_back(it->second);
                }
            } else if (ship) {
                // not sure how this is gonna work yet
            } else {
                // what other checks should we make here?
                sLog.Error("MapData::GetMissionDestination(3)", "check is 'else' for systemID %u", systemID);
            }
        } break;

        // dont know how to get route/jumps from start to constellation edges...yet
        case Agents::Range::SameConstellation:  {  //4
            if (station or deadspace) {
                // get all systems in this constellation
                uint32 constellationID(sDataMgr.GetSystemConstellation(systemID));
                std::vector<uint32> systemVec;
                sDataMgr.GetConstellationSystems(constellationID, systemVec);
                for (auto &cur : systemVec) {
                    // get systems that are not borders
                    if (!sDataMgr.IsConSystem(cur) and !sDataMgr.IsRegionSystem(cur))
                        sysList.push_back(cur);
                }
                // now, run these systems for next jump
                systemVec.clear();
                for (auto &cur : sysList) {
                    // loop thru systems in this constellation
                    auto JumpItr = m_systemJumps.equal_range(cur);  // get their jumps
                        // loop thru jumps from systems in this constellation
                    for (auto it = JumpItr.first; it != JumpItr.second; ++it) {
                        // may not need to test this...we checked for borders already
                        if (sDataMgr.GetSystemConstellation(it->second) == sDataMgr.GetSystemConstellation(systemID)){
                            // if the jump is not to another constellation, add to list
                            systemVec.push_back(it->second);
                        }
                    }
                }
                // systemVec should now have all jumps out of this constellation and into neighboring constellation
                //   clear out sysList and copy systemVec to it for further processing
                sysList.clear();
                sysList = systemVec;
            } else if (ship) {
                // not sure how this is gonna work yet
            } else {
                sLog.Error("MapData::GetMissionDestination(4)", "check is 'else' for systemID %u", systemID);

            }
        } break;

        case Agents::Range::NeighboringConstellation: {    //5
            // this will be a border jump.
            uint32 regionID(sDataMgr.GetSystemRegion(systemID));
            std::vector<uint32> systemVec;
            // get all systems in this constellation
            sDataMgr.GetConstellationSystems(regionID, systemVec);
            for (auto &cur : systemVec) {
                // get all constellation borders in this constellation
                if (sDataMgr.IsConSystem(cur))
                    sysList.push_back(cur);
            }
            systemVec.clear();
            for (auto &cur : sysList) {
                // loop thru systems in this constellation
                auto JumpItr = m_systemJumps.equal_range(cur);  // get their jumps
                for (auto it = JumpItr.first; it != JumpItr.second; ++it) {
                    // loop thru jumps from systems in this constellation
                    if (sDataMgr.GetSystemConstellation(it->second) != sDataMgr.GetSystemConstellation(systemID)){
                        // if the jump is to another constellation, add to list
                        systemVec.push_back(it->second);
                    }
                }
            }
            // systemVec should now have all jumps out of this constellation and into neighboring constellation
            //   clear out sysList and copy systemVec to it for further processing
            sysList.clear();
            sysList = systemVec;
        } break;

        case Agents::Range::NeighboringRegion: {    //6
            // this will be a border jump.
            uint32 regionID = sDataMgr.GetStationRegion(pAgent->GetStationID());
            std::vector<uint32> systemVec;
            sDataMgr.GetRegionSystems(regionID, systemVec);
            for (auto &cur : systemVec) {
                if (sDataMgr.IsRegionSystem(cur))
                    sysList.push_back(cur);     // systems in this region
            }
            systemVec.clear();
            for (auto &cur : sysList) {
                // loop thru systems in this region
                auto JumpItr = m_systemJumps.equal_range(cur);  // get their jumps
                for (auto it = JumpItr.first; it != JumpItr.second; ++it) {
                    // loop thru jumps from systems in this region
                    if (sDataMgr.GetSystemRegion(it->second) != sDataMgr.GetSystemRegion(systemID)){
                        // if the jump is to another region, add to list
                        systemVec.push_back(it->second);
                    }
                }
            }
            // systemVec should now have all jumps out of this region and into neighboring region
            //   clear out sysList and copy systemVec to it for further processing
            sysList.clear();
            sysList = systemVec;
        } break;

        // not sure how to do these two yet...
        case Agents::Range::NearestEnemyCombatZone: {  //10
        } break;
        case Agents::Range::NearestCareerHub: {    //11
        } break;
    }

    if (sysList.empty()) {
        run = false;
        offer.destinationID = pAgent->GetStationID();
        sLog.Error("MapData::GetMissionDestination(sysList)", "sysList empty for systemID %u.", systemID);
    }

    uint8 count(0);
    while (run) {
        run = false;
        systemID = sysList.at(MakeRandomInt(0, (sysList.size() - 1)));
        if (station and (sDataMgr.GetStationCount(systemID) < 1)) {
            run = true;
            ++count;
        }
        if (run and (count > sysList.size())) {
            // problem....no station found within one jump
            offer.destinationID = pAgent->GetStationID();
            sLog.Error("MapData::GetMissionDestination(run)", "no station found within 1 jump of systemID %u.", systemID);
        }
    }

    if (station) {
        // get random station in given system
        std::vector<uint32> list;
        sDataMgr.GetStationList(systemID, list);
        if (list.size() < 2) {
            offer.destinationID = list[0];
        } else {
            bool run(true);
            while (run) {
                offer.destinationID = list.at(MakeRandomInt(0, (list.size() - 1)));
                if (offer.destinationID != pAgent->GetStationID())
                    run = false;
            }
        }
    } else if (deadspace) {
        offer.destinationID = systemID;
    } else if (ship) {
        // not sure how this is gonna work yet
    }


    if (sDataMgr.IsStation(offer.destinationID)) {
        StationData data = StationData();
        stDataMgr.GetStationData(offer.destinationID, data);
        offer.destinationOwnerID    = data.corporationID;
        offer.destinationSystemID   = data.systemID;
        offer.destinationTypeID     = data.typeID;
        if (offer.destinationID == pAgent->GetStationID())
            sLog.Error("MapData::GetMissionDestination(last)", "destination=agentStation for systemID %u.", systemID);
    } else if (ship) {
        offer.destinationSystemID   = offer.destinationID;
        offer.destinationTypeID     = sDataMgr.GetStaticType(offer.destinationID);
        offer.dungeonLocationID     = offer.destinationID;
        offer.dungeonSolarSystemID  = offer.destinationID;
    } else {
        offer.destinationSystemID   = offer.destinationID;
        offer.destinationTypeID     = sDataMgr.GetStaticType(offer.destinationID);
        offer.dungeonLocationID     = offer.destinationID;
        offer.dungeonSolarSystemID  = offer.destinationID;
    }
}
