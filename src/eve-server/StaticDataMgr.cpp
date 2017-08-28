
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
    m_moonGoo.clear();
    m_ramMatl.clear();
    m_regions.clear();
    m_systemData.clear();
    m_staticData.clear();
    m_stationData.clear();
    m_typeAttrMap.clear();
    m_stationCount.clear();
    m_oreBySecClass.clear();
    for (auto cur : m_stationPyData)
        PyDecRef(cur.second);
    m_stationPyData.clear();
    m_stationRegion.clear();
    m_stationSystem.clear();
}

void StaticDataMgr::Populate()
{
    double start = GetTimeMSeconds();
    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;

    m_db.GetTypeAttributes(*res);
    DmgTypeAttribute typeAttr;
    while (res->GetRow(row)) {
        //SELECT typeID, attributeID, valueInt, valueFloat FROM dgmTypeAttributes
        typeAttr.attributeID = row.GetUInt(1);
        if (row.IsNull(2))
            typeAttr.value = row.GetDouble(3);
        else
            typeAttr.value = row.GetInt(2);

        m_typeAttrMap.insert(std::pair<uint16, DmgTypeAttribute>(row.GetUInt(0), typeAttr));
    }
    sLog.Cyan("    StaticDataMgr", "%u Type Attribute Sets loaded in %.3fms", m_typeAttrMap.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
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
    m_db.GetBlueprintType(*res);
    BlueprintTypeData bpTypeData;
    while (res->GetRow(row)) {
        //SELECT blueprintTypeID, parentBlueprintTypeID, productTypeID, productionTime, techLevel, researchProductivityTime, researchMaterialTime, researchCopyTime,
        //  researchTechTime, productivityModifier, materialModifier, wasteFactor, maxProductionLimit, chanceOfRE FROM invBlueprintTypes
        bpTypeData.parentBlueprintTypeID    = row.GetInt(1);
        bpTypeData.productTypeID            = row.GetInt(2);
        bpTypeData.productionTime           = row.GetInt(3);
        bpTypeData.techLevel                = row.GetInt(4);
        bpTypeData.researchProductivityTime = row.GetInt(5);
        bpTypeData.researchMaterialTime     = row.GetInt(6);
        bpTypeData.researchCopyTime         = row.GetInt(7);
        bpTypeData.researchTechTime         = row.GetInt(8);
        bpTypeData.productivityModifier     = row.GetInt(9);
        bpTypeData.materialModifier         = row.GetInt(10);
        bpTypeData.wasteFactor              = row.GetInt(11);
        bpTypeData.maxProductionLimit       = row.GetInt(12);
        bpTypeData.chanceOfReverseEngineering = row.GetFloat(13);
        m_bpTypeData.insert(std::pair<uint16, BlueprintTypeData>(row.GetInt(0), bpTypeData));
    }
    sLog.Cyan("    StaticDataMgr", "%u BP Type defs loaded in %.3fms.", m_bpTypeData.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    m_db.GetRAMRequirements(*res);
    ramRequirements ramReq;
    ramReq.activityID = ramReq.requiredTypeID = ramReq.quantity = ramReq.damagePerJob = ramReq.recycle = 0;
    while (res->GetRow(row)) {
        //SELECT typeID, activityID, requiredTypeID, quantity, damagePerJob, recycle FROM ramTypeRequirements
        ramReq.activityID       = row.GetInt(1);
        ramReq.requiredTypeID   = row.GetInt(2);
        ramReq.quantity         = row.GetInt(3);
        ramReq.damagePerJob     = row.GetFloat(4);
        ramReq.recycle          = (row.GetInt(5) ? true : false);
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
    m_db.GetSystemData(*res);
    SystemData sysData;
    while (res->GetRow(row)) {
        //SELECT solarSystemID, solarSystemName, constellationID, regionID, securityClass, security FROM mapSolarSystems
        sysData.systemID          = row.GetUInt(0);
        sysData.name              = row.GetText(1);
        sysData.constellationID   = row.GetUInt(2);
        sysData.regionID          = row.GetUInt(3);
        if (row.IsNull(4))
            sysData.securityClass = "0";
        else
            sysData.securityClass = row.GetText(4);
        sysData.securityRating    = row.GetFloat(5);
        m_systemData.insert(std::pair<uint32, SystemData>(row.GetUInt(0), sysData));
    }

    res->Reset();
    m_db.GetStationRegion(*res);
    while (res->GetRow(row)) {
        //SELECT stationID, regionID FROM staStations
        m_stationRegion.insert(std::pair<uint32, uint32>(row.GetUInt(0), row.GetUInt(1)));
    }

    res->Reset();
    m_db.GetStationSystem(*res);
    while (res->GetRow(row)) {
        //SELECT stationID, solarSystemID FROM staStations
        m_stationSystem.insert(std::pair<uint32, uint32>(row.GetUInt(0), row.GetUInt(1)));
    }
    sLog.Cyan("    StaticDataMgr", "%u Static System data sets loaded in %.3fms.", (m_systemData.size() + m_stationRegion.size() + m_stationSystem.size()), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    m_db.GetStaticData(*res);
    StaticData staticData;
    while (res->GetRow(row)) {
        //SELECT itemID, regionID, constellationID, solarSystemID, x, y, z FROM mapDenormalize
        staticData.itemID          = row.GetUInt(0);
        staticData.regionID        = row.GetUInt(1);
        staticData.constellationID = row.GetUInt(2);
        staticData.systemID        = row.GetUInt(3);
        staticData.position        = GPoint(row.GetDouble(4),row.GetDouble(5),row.GetDouble(6));
        m_staticData.insert(std::pair<uint32, StaticData>(row.GetUInt(0), staticData));
    }
    sLog.Cyan("    StaticDataMgr", "%u Static Entity data sets loaded in %.3fms.", m_staticData.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    m_mdb.GetStationCount(*res);
    while (res->GetRow(row)) {
        //SELECT map.solarSystemID, count(sta.stationID) FROM staStations sta
        m_stationCount.insert(std::pair<uint32, uint8>(row.GetInt(0), row.GetInt(1)));
    }

    res->Reset();
    m_db.GetStationInfo(*res);
    StationData staData;
    while (res->GetRow(row)) {
        //SELECT s.stationID, s.x, s.y, s.z, st.dockEntryX, st.dockEntryY, st.dockEntryZ, st.dockOrientationX, st.dockOrientationY, st.dockOrientationZ FROM staStations
        staData.stationID       = row.GetUInt(0);
        staData.position        = GPoint(row.GetDouble(1),row.GetDouble(2),row.GetDouble(3));
        staData.dockPosition    = GPoint(row.GetDouble(4) + row.GetDouble(1),
                                         row.GetDouble(5) + row.GetDouble(2),
                                         row.GetDouble(6) + row.GetDouble(3));
        staData.dockOrientation = GVector(row.GetDouble(7),row.GetDouble(8),row.GetDouble(9));
        m_stationData.insert(std::pair<uint32, StationData>(row.GetUInt(0), staData));
    }

    res->Reset();
    m_sdb.GetStationIDs(*res);
    while (res->GetRow(row)) {
        //SELECT stationID FROM staStations   (then convert it into Python Data...)
        m_stationPyData.insert(std::pair<uint32, PyObject*>(row.GetInt(0), m_sdb.DoGetStation(row.GetInt(0))));
    }
    sLog.Cyan("    StaticDataMgr", "%u Static Station data sets loaded in %.3fms.", (m_stationCount.size() + m_stationData.size() + m_stationPyData.size()), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    m_db.GetMoonResouces(*res);
    while (res->GetRow(row)) {
        //SELECT typeID,volume FROM invTypes [where group=moongoo]
        m_moonGoo.insert(std::pair<uint16, uint8>(row.GetInt(0), (uint8)(row.GetFloat(1) *10)));
    }
    sLog.Cyan("    StaticDataMgr", "%u Moon Resources loaded in %.3fms.", m_moonGoo.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    m_db.GetRegionFaction(*res);
    while (res->GetRow(row)) {
        //SELECT regionID, factionID FROM mapRegions
        m_regions.insert(std::pair<uint32, uint32>(row.GetInt(0), row.GetInt(1)));
    }
    sLog.Cyan("    StaticDataMgr", "%u Region Factions loaded in %.3fms.", m_regions.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    m_db.GetRegionRatFaction(*res);
    while (res->GetRow(row)) {
        //SELECT regionID, ratFactionID FROM mapRegions WHERE ratFactionID != 0
        m_ratRegions.insert(std::pair<uint32, uint32>(row.GetInt(0), row.GetInt(1)));
    }

    res->Reset();
    start = GetTimeMSeconds();
    m_db.GetSkillList(*res);
    while (res->GetRow(row)) {
        //SELECT typeID, typeName FROM invTypes [where type=skill]
        m_skills.insert(std::pair<uint16, std::string>(row.GetInt(0), row.GetText(1)));
    }
    sLog.Cyan("    StaticDataMgr", "%u skills loaded in %.3fms.", m_skills.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    m_db.GetFactionGroups(*res);
    DBQueryResult* res2 = new DBQueryResult();
    DBResultRow row2;
    RatFactionGroups factionGroup;
    while (res->GetRow(row)) {
        //SELECT shipClass, groupID, factionID FROM roidRatClassGroup
        factionGroup.shipClass = row.GetInt(0);
        factionGroup.groupID = row.GetInt(1);
        m_groups.emplace(row.GetInt(2), factionGroup);

        m_db.GetGroupTypeIDs(row.GetInt(1), *res2);
        while (res2->GetRow(row2)) {
            //SELECT typeID FROM invTypes WHERE groupID = %u ORDER BY typeID LIMIT 10
            m_types.emplace(row.GetInt(1), row2.GetInt(0));
        }
    }

    res->Reset();
    m_db.GetSpawnClasses(*res);
    RatSpawnClass spawnClass;
    while (res->GetRow(row)) {
        //SELECT type, sub, f, d, c, bc, bs, h, o, cf, cd, cc, cbc, cbs FROM roidRatSpawnClass
        spawnClass.type = row.GetInt(0);
        spawnClass.sub = row.GetInt(1);
        spawnClass.f = row.GetInt(2);
        spawnClass.d = row.GetInt(3);
        spawnClass.c = row.GetInt(4);
        spawnClass.bc = row.GetInt(5);
        spawnClass.bs = row.GetInt(6);
        spawnClass.h = row.GetInt(7);
        spawnClass.o = row.GetInt(8);
        spawnClass.cf = row.GetInt(9);
        spawnClass.cd = row.GetInt(10);
        spawnClass.cc = row.GetInt(11);
        spawnClass.cbc = row.GetInt(12);
        spawnClass.cbs = row.GetInt(13);
        m_classes.emplace(row.GetInt(0), spawnClass);
    }

    sLog.Cyan("    StaticDataMgr", "%u groups in %u buckets, %u classes in %u buckets, and %u types for %u regions loaded in %.3fms.",
              m_groups.size(), m_groups.bucket_count(), m_classes.size(), m_classes.bucket_count(), m_types.size(), m_ratRegions.size(), (GetTimeMSeconds() - start));

    //cleanup
    SafeDelete(res);
    SafeDelete(res2);
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

void StaticDataMgr::GetDgmTypeAttrVec(uint32 typeID, std::vector< DmgTypeAttribute >& typeAttrVec)
{
    auto itr = m_typeAttrMap.equal_range(typeID);
    for (auto it = itr.first; it != itr.second; ++it)
        typeAttrVec.push_back(it->second);
}

bool StaticDataMgr::IsSkillTypeID(uint16 typeID)
{
    if (m_skills.find(typeID) != m_skills.end())
        return true;
    return false;
}

bool StaticDataMgr::GetSkillName(uint16 skillID, std::string& name)
{
    std::map<uint16, std::string>::const_iterator itr = m_skills.find(skillID);
    if (itr != m_skills.end()) {
        name = itr->second;
        return true;
    } else
        _log(DATABASE__MESSAGE, "Failed to query name for skill %u: Skill not found.", skillID);
    return false;
}

void StaticDataMgr::GetMoonResouces(std::map<uint16, uint8>& data)
{
    // make copy
    for (auto cur : m_moonGoo)
        data.insert(std::pair<uint16, uint8>(cur.first, cur.second));
}

void StaticDataMgr::GetBpTypeData(uint32 typeID, BlueprintTypeData& bpData)
{
    std::map<uint16, BlueprintTypeData>::const_iterator itr = m_bpTypeData.find(typeID);
    if (itr != m_bpTypeData.end())
        bpData = itr->second;
    else
        _log(DATABASE__MESSAGE, "Failed to query info for bpType %u: Type not found.", typeID);
}

bool StaticDataMgr::GetRamMaterials(uint16 typeID, std::vector< ramMaterials >& ramMatls)
{
    auto itr = m_ramMatl.equal_range(typeID);
    for (auto it = itr.first; it != itr.second; ++it)
        ramMatls.push_back(it->second);
}

bool StaticDataMgr::GetRamRequirements(uint16 typeID, std::vector< ramRequirements >& ramReqs)
{
    auto itr = m_ramReq.equal_range(typeID);
    for (auto it = itr.first; it != itr.second; ++it)
        ramReqs.push_back(it->second);
}

PyObject* StaticDataMgr::GetStationData(uint32 stationID)
{
    std::map<uint32, PyObject*>::const_iterator itr = m_stationPyData.find(stationID);
    if (itr != m_stationPyData.end()) {
        PyIncRef(itr->second);
        return itr->second;
    } else
        _log(DATABASE__MESSAGE, "Failed to query data for station %u: Station not found.", stationID);
    return nullptr;
}

PyRep* StaticDataMgr::GetStationCount()
{
    PyList* list = new PyList();
    std::map<uint32, uint8>::const_iterator itr = m_stationCount.begin();
    while (itr != m_stationCount.end()) {
        PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyInt(itr->first));
        tuple->SetItem(1, new PyInt(itr->second));
        list->AddItem(tuple);
        ++itr;
    }
    return list;
}

uint32 StaticDataMgr::GetStationRegion(uint32 stationID)
{
    std::map<uint32, uint32>::const_iterator itr = m_stationRegion.find(stationID);
    if (itr != m_stationRegion.end())
        return itr->second;
    else
        _log(DATABASE__MESSAGE, "Failed to query info for station %u: Station not found.", stationID);
    return 0;
}

uint32 StaticDataMgr::GetStationSystem(uint32 stationID)
{
    std::map<uint32, uint32>::const_iterator itr = m_stationSystem.find(stationID);
    if (itr != m_stationSystem.end())
        return itr->second;
    else
        _log(DATABASE__MESSAGE, "Failed to query info for station %u: Station not found.", stationID);
    return 0;
}

bool StaticDataMgr::GetSystemInfo(uint32 locationID, SystemData& data)
{
    // this specific cache method is designed to use EITHER a stationID OR a systemID to determine system data wanted.
    if (IsStation(locationID)) {
        locationID = GetStationSystem(locationID);
    } else if (!IsSolarSystem(locationID)) {
        _log(SERVICE__WARNING, "Failed to query info:  locationID %u is neither station nor system.", locationID);
        return false;
    }

    std::map<uint32, SystemData>::const_iterator itr = m_systemData.find(locationID);
    if (itr != m_systemData.end()) {
        data = itr->second;
        return true;
    } else {
        _log(DATABASE__MESSAGE, "Failed to query info for system %u: System not found.", locationID);
    }
    return false;
}

bool StaticDataMgr::GetStaticInfo(uint32 itemID, StaticData& data)
{
    std::map<uint32, StaticData>::const_iterator itr = m_staticData.find(itemID);
    if (itr != m_staticData.end()) {
        data = itr->second;
        return true;
    } else {
        _log(DATABASE__MESSAGE, "Failed to query info for static item %u: Item not found.", itemID);
    }
    return false;
}

bool StaticDataMgr::GetStationInfo(uint32 stationID, StationData& data)
{
    std::map<uint32, StationData>::const_iterator itr = m_stationData.find(stationID);
    if (itr != m_stationData.end()) {
        data = itr->second;
        return true;
    } else {
        _log(DATABASE__MESSAGE, "Failed to query info for station %u: Station not found.", stationID);
    }
    return true;
}

uint32 StaticDataMgr::GetRegionFaction(uint32 regionID)
{
    std::map<uint32, uint32>::const_iterator itr = m_regions.find(regionID);
    if (itr != m_regions.end())
        return (*itr).second;
    else
        _log(DATABASE__MESSAGE, "Failed to query faction for region %u: region not found.", regionID);
    return 0;
}

uint32 StaticDataMgr::GetRegionRatFaction(uint32 regionID)
{
    std::map<uint32, uint32>::iterator itr = m_ratRegions.find(regionID);
    if (itr != m_ratRegions.end())
        return (*itr).second;
    return 0;
}

uint8 StaticDataMgr::GetRegionQuarter(uint32 regionID)
{
    uint32 factionID = 0;
    std::map<uint32, uint32>::const_iterator itr = m_regions.find(regionID);
    if (itr != m_regions.end())
        factionID = (*itr).second;

    // caldari=1, minmatar=2, amarr=3, gallente=4, none=5
    switch (factionID) {
        case factionCaldari:        //Caldari State
        case factionGuristas:       //Guristas Pirates
            return 1; break;
        case factionMinmatar:       //Minmatar Republic
        case factionAngel:          //Angel Cartel
            return 2; break;
        case factionAmarr:          //Amarr Empire
        case factionAmmatar:        //Ammatar Mandate
        case factionKhanid:         //Khanid Kingdom
        case factionBloodRaider:    //Blood Raider Covenant
        case factionSanshas:        //Sansha's Nation
            return 3; break;
        case factionGallente:       //Gallente Federation
        case factionSerpentis:      //Serpentis
            return 4; break;
        case factionJove:           //Jove Empire
        case factionCONCORD:        //CONCORD Assembly
        case factionSyndicate:      //The Syndicate
        case factionInterBus:       //The InterBus
        case factionORE:            //ORE
        case factionThukker:        //Thukker Tribe
        case factionSistersOfEVE:   //Servant Sisters of EVE
        case factionSociety:        //The Society of Conscious Thought
        case factionMordusLegion:   //Mordu's Legion Command
            return 5; break;
    }
}

uint32 StaticDataMgr::GetCorpID(uint32 factionID)
{
    switch (factionID) {
        case factionAngel:          return corpAngel;
        case factionSanshas:        return corpSanshas;
        case factionBloodRaider:    return corpBloodRaider;
        case factionGuristas:       return corpGuristas;
        case factionSerpentis:      return corpSerpentis;
        case factionRogueDrones:    return corpRogueDrones;
    }
}

std::string StaticDataMgr::GetCorpName(uint32 corpID)
{
    switch (corpID) {
        case corpAngel:          return "Angel";
        case corpSanshas:        return "Sansha";
        case corpBloodRaider:    return "Blood";
        case corpGuristas:       return "Guristas";
        case corpSerpentis:      return "Serpentis";
        case corpRogueDrones:    return "Drone";
    }
}

std::string StaticDataMgr::GetFactionName(uint32 factionID)
{
    switch (factionID) {
        case factionAngel:          return "Angel";
        case factionSanshas:        return "Sansha";
        case factionBloodRaider:    return "Blood";
        case factionGuristas:       return "Guristas";
        case factionSerpentis:      return "Serpentis";
        case factionRogueDrones:    return "Drone";
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
