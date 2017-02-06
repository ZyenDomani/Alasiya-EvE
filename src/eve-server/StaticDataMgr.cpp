
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
#include "database/EVEDBUtils.h"


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
    m_ramReq.clear();
    m_ramMatl.clear();
    m_regions.clear();
    m_systemData.clear();
    m_staticData.clear();
    m_stationData.clear();
    m_stationCount.clear();
    m_oreBySecClass.clear();
    m_stationSystem.clear();
    m_stationPyData.clear();
}

void StaticDataMgr::Populate()
{
    double start = GetTimeMSeconds();
    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;

    m_db.GetOreBySSC(*res);
    OreTypeChance oreChance;
    oreChance.typeID = oreChance.chance  = 0;
    while (res->GetRow(row)) {
        //SELECT systemSec, roidID, percent FROM roidDistribution
        oreChance.typeID  = row.GetInt(1);
        oreChance.chance  = row.GetFloat(2);
        m_oreBySecClass.insert(std::pair<std::string, OreTypeChance>(row.GetText(0), oreChance));
    }
    sLog.Cyan("    StaticDataMgr", "%u Ore defs loaded in %.3fms.", m_oreBySecClass.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    m_db.GetRAMRequirements(*res);
    ramRequirements ramReq;
    ramReq.activityID = ramReq.requiredTypeID = ramReq.quantity = ramReq.damagePerJob = ramReq.recycle = 0;
    while (res->GetRow(row)) {
        //SELECT typeID, activityID, requiredTypeID, quantity, damagePerJob, recycle FROM ramTypeRequirements
        ramReq.activityID = row.GetInt(1);
        ramReq.requiredTypeID = row.GetInt(2);
        ramReq.quantity = row.GetInt(3);
        ramReq.damagePerJob = row.GetFloat(4);
        ramReq.recycle = (row.GetInt(5) ? true : false);
        m_ramReq.insert(std::pair<uint16, ramRequirements>(row.GetInt(0), ramReq));
    }

    res->Reset();
    m_db.GetRAMMaterials(*res);
    ramMaterials ramMatls;
    ramMatls.quantity = ramMatls.materialTypeID = 0;
    while (res->GetRow(row)) {
        //SELECT typeID, materialTypeID, quantity FROM invTypeMaterials
        ramMatls.quantity = row.GetInt(2);
        ramMatls.materialTypeID = row.GetInt(1);
        m_ramMatl.insert(std::pair<uint16, ramMaterials>(row.GetInt(0), ramMatls));
    }
    sLog.Cyan("    StaticDataMgr", "%u R.A.M. defs loaded in %.3fms.", (m_ramMatl.size() + m_ramReq.size()), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    m_mdb.GetStationCount(*res);
    while (res->GetRow(row)) {
        //SELECT map.solarSystemID, count(sta.stationID) FROM staStations sta
        m_stationCount.insert(std::pair<uint32, uint8>(row.GetInt(0), row.GetInt(1)));
    }

    res->Reset();
    m_sdb.GetStationIDs(*res);
    while (res->GetRow(row)) {
        //SELECT stationID FROM staStations   (then convert it into Python Data...)
        m_stationPyData.insert(std::pair<uint32, PyObject*>(row.GetInt(0), m_sdb.DoGetStation(row.GetInt(0))));
    }
    sLog.Cyan("    StaticDataMgr", "%u Static Station data sets loaded in %.3fms.", m_stationPyData.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    m_db.GetRegionFaction(*res);
    while (res->GetRow(row)) {
        //SELECT regionID, factionID FROM mapRegions
        m_regions.insert(std::pair<uint32, uint32>(row.GetInt(0), row.GetInt(1)));
    }

    res->Reset();
    m_db.GetSkillList(*res);
    while (res->GetRow(row)) {
        //SELECT typeID, typeName FROM invTypes [where type=skill]
        m_skills.insert(std::pair<uint16, std::string>(row.GetInt(0), row.GetText(1)));
    }
    sLog.Cyan("    StaticDataMgr", "%u misc data sets loaded in %.3fms.", (m_regions.size() + m_skills.size()), (GetTimeMSeconds() - start));

/*  this was for a test.  is not needed for normal run
    res->Reset();
    start = GetTimeMSeconds();
    GetItems(*res);
    while (res->GetRow(row)) {
        //SELECT typeID FROM invTypes [where type=player-usable items]
        m_items.push_back(row.GetInt(0));
    }
    sLog.Cyan("    StaticDataMgr", "%u game items loaded in %.3fms.", m_items.size(), (GetTimeMSeconds() - start));
*/
    //cleanup
    SafeDelete(res);
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

bool StaticDataMgr::IsSkillTypeID(uint16 typeID)
{
    if (m_skills.find(typeID) != m_skills.end())
        return true;
    return false;
}

bool StaticDataMgr::GetSkillName(uint16 skillID, std::string& name)
{
    std::map<uint16, std::string>::iterator itr = m_skills.find(skillID);
    if (itr != m_skills.end()) {
        name = itr->second;
        return true;
    }
    return false;
}

bool StaticDataMgr::GetRamMaterials(uint16 typeID, std::vector< ramMaterials >& ramMatls)
{
    auto itr = m_ramMatl.equal_range(typeID);
    for (auto it = itr.first; it != itr.second; it++)
        ramMatls.push_back(it->second);
}

bool StaticDataMgr::GetRamRequirements(uint16 typeID, std::vector< ramRequirements >& ramReqs)
{
    auto itr = m_ramReq.equal_range(typeID);
    for (auto it = itr.first; it != itr.second; it++)
        ramReqs.push_back(it->second);
}

PyObject* StaticDataMgr::GetStationData(uint32 stationID)
{
    std::map<uint32, PyObject*>::iterator itr = m_stationPyData.find(stationID);
    if (itr != m_stationPyData.end()) {
        PyIncRef(itr->second);
        return itr->second;
    }
    return nullptr;
}

PyRep* StaticDataMgr::GetStationCount()
{
    PyList* list = new PyList();
    std::map<uint32, uint8>::iterator itr = m_stationCount.begin();
    while (itr != m_stationCount.end()) {
        PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyInt(itr->first));
        tuple->SetItem(1, new PyInt(itr->second));
        list->AddItem(tuple);
        ++itr;
    }
    return list;
}

void StaticDataMgr::GetItems(DBQueryResult& res)
{
    /*Ship = 6,Module = 7,Charge = 8,Skill = 16,Drone = 18,Implant = 20,Deployable = 22,Structure = 23,Subsystem = 32,
        AncientRelics = 34,Decryptors = 35,StructureUpgrade = 39,SovereigntyStructure = 40,*/
    if (!sDatabase.RunQuery(res,
        "SELECT t.typeID FROM invTypes AS t"
        " LEFT JOIN `dgmTypeEffects` AS te USING (typeID)"
        " WHERE groupID IN (SELECT groupID FROM invGroups WHERE categoryID IN (6,7,8,16,18,20,22,23,32,34,35,39,40))"
        " AND te.effectID > 0")) {
        codelog(DATABASE__ERROR, "Error in GetItems query: %s", res.error.c_str());
    }
}

uint32 StaticDataMgr::GetStationRegion(uint32 stationID)
{
    uint32 regionID = 0;
    std::map<uint32, uint32>::iterator itr = m_stationRegion.find(stationID);
    if (itr != m_stationRegion.end()) {
        regionID = itr->second;
    } else {
        DBQueryResult res;
        if (!sDatabase.RunQuery(res, "SELECT regionID FROM staStations WHERE stationID = %u", stationID)) {
            codelog(DATABASE__ERROR, "Failed to query info for station %u: %s.", stationID, res.error.c_str());
            return 0;
        }

        DBResultRow row;
        if (!res.GetRow(row)) {
            _log(DATABASE__MESSAGE, "Failed to query info for station %u: Station not found.", stationID);
            return 0;
        }
        m_stationRegion[stationID] = (regionID = row.GetUInt(0));
    }
    return regionID;
}

uint32 StaticDataMgr::GetStationSystem(uint32 stationID)
{
    uint32 systemID = 0;
    std::map<uint32, uint32>::iterator itr = m_stationSystem.find(stationID);
        if (itr != m_stationSystem.end()) {
            systemID = itr->second;
        } else {
            DBQueryResult res;
            if (!sDatabase.RunQuery(res, "SELECT solarSystemID FROM staStations WHERE stationID = %u", stationID)) {
                codelog(DATABASE__ERROR, "Failed to query info for station %u: %s.", stationID, res.error.c_str());
                return 0;
            }

            DBResultRow row;
            if (!res.GetRow(row)) {
                _log(DATABASE__MESSAGE, "Failed to query info for station %u: Station not found.", stationID);
                return 0;
            }
            m_stationSystem[stationID] = (systemID = row.GetUInt(0));
        }
    return systemID;
}

//  the system data is cached on initial boot of system
bool StaticDataMgr::GetSystemInfo(uint32 locationID, SystemData& data)
{
    // this specific cache method is designed to use EITHER a stationID OR a systemID to determine system data wanted.
    if (IsStation(locationID)) {
        locationID = GetStationSystem(locationID);
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
