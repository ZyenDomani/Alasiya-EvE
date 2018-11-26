
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
#include "station/StationDataMgr.h"
#include "system/SystemManager.h"


StaticDataMgr::StaticDataMgr()
: m_keyMap(nullptr),
m_agents(nullptr),
m_operands(nullptr),
m_billTypes(nullptr),
m_entryTypes(nullptr),
m_factionInfo(nullptr),
m_npcDivisions(nullptr)
{
    m_ramReq.clear();
    m_moonGoo.clear();
    m_ramMatl.clear();
    m_regions.clear();
    m_bpMatlData.clear();
    m_systemData.clear();
    m_staticData.clear();
    m_salvageMap.clear();
    m_agentSystem.clear();
    m_typeAttrMap.clear();
    m_LootGroupMap.clear();
    m_stationCount.clear();
    m_stationConst.clear();
    m_stationRegion.clear();
    m_stationSystem.clear();
    m_oreBySecClass.clear();
    m_LootGroupTypeMap.clear();
    m_WrecksToTypesMap.clear();

}

StaticDataMgr::~StaticDataMgr()
{
    //Clear();
}

int StaticDataMgr::Initialize()
{
    Populate();
    sLog.Blue("    StaticDataMgr", "Static Data Manager Initialized.");
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
    m_salvageMap.clear();
    m_agentSystem.clear();
    m_typeAttrMap.clear();
    m_stationCount.clear();
    m_stationConst.clear();
    m_LootGroupMap.clear();
    m_stationRegion.clear();
    m_stationSystem.clear();
    m_oreBySecClass.clear();
    m_LootGroupTypeMap.clear();
    m_WrecksToTypesMap.clear();

    PySafeDecRef(m_keyMap);
    PySafeDecRef(m_agents);
    PySafeDecRef(m_operands);
    PySafeDecRef(m_billTypes);
    PySafeDecRef(m_entryTypes);
    PySafeDecRef(m_factionInfo);
    PySafeDecRef(m_npcDivisions);

    for (auto cur : m_bpMatlData)
        PySafeDecRef(cur.second);
    m_bpMatlData.clear();
}

void StaticDataMgr::Populate()
{
    double begin = GetTimeMSeconds();

    m_keyMap = ManagerDB::GetKeyMap();
    if (m_keyMap == nullptr)
        sLog.Error("    StaticDataMgr", "m_keyMap is null");

    m_agents = ManagerDB::GetAgents();
    if (m_agents == nullptr)
        sLog.Error("    StaticDataMgr", "m_agents is null");

    m_operands = ManagerDB::GetOperands();
    if (m_operands == nullptr)
        sLog.Error("    StaticDataMgr", "m_operands is null");

    m_billTypes = ManagerDB::GetBillTypes();
    if (m_billTypes == nullptr)
        sLog.Error("    StaticDataMgr", "m_billTypes is null");

    m_entryTypes = ManagerDB::GetEntryTypes();
    if (m_entryTypes == nullptr)
        sLog.Error("    StaticDataMgr", "m_entryTypes is null");

    m_npcDivisions = ManagerDB::GetNPCDivisions();
    if (m_npcDivisions == nullptr)
        sLog.Error("    StaticDataMgr", "m_npcDivisions is null");

    GetFactionInfoRsp rsp;
    ManagerDB::ListAllCorpFactions(rsp.factionIDbyNPCCorpID);
    ManagerDB::ListAllFactionStationCounts(rsp.factionStationCount);
    ManagerDB::ListAllFactionSystemCounts(rsp.factionSolarSystemCount);
    ManagerDB::ListAllFactionRegions(rsp.factionRegions);
    ManagerDB::ListAllFactionConstellations(rsp.factionConstellations);
    ManagerDB::ListAllFactionSolarSystems(rsp.factionSolarSystems);
    ManagerDB::ListAllFactionRaces(rsp.factionRaces);
    rsp.npcCorpInfo = ManagerDB::ListAllNPCCorpInfo();
    m_factionInfo = rsp.Encode();
    if (m_factionInfo == nullptr)
        sLog.Error("    StaticDataMgr", "m_factionInfo is null");

    double start = GetTimeMSeconds();
    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;

    ManagerDB::GetSystemData(*res);
    while (res->GetRow(row)) {
        //SELECT solarSystemID, solarSystemName, constellationID, regionID, securityClass, security FROM mapSolarSystems
        SystemData sysData;
        sysData.systemID          = row.GetInt(0);
        sysData.name              = row.GetText(1);
        sysData.constellationID   = row.GetInt(2);
        sysData.regionID          = row.GetInt(3);
        sysData.securityClass     = (row.IsNull(4) ? "0" : row.GetText(4));
        sysData.securityRating    = row.GetFloat(5);
        m_systemData.emplace(row.GetInt(0), sysData);
    }
    sLog.Cyan("    StaticDataMgr", "%u Static System data sets loaded in %.3fms.", m_systemData.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    ManagerDB::GetWHSystemClass(*res);
    while (res->GetRow(row)) {
        //SELECT locationID, wormholeClassID FROM mapLocationWormholeClasses
        m_whRegions.emplace(row.GetInt(0), row.GetInt(1));
    }
    sLog.Cyan("    StaticDataMgr", "%u WH System Classes loaded in %.3fms.", m_whRegions.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    ManagerDB::GetStaticData(*res);
    while (res->GetRow(row)) {
        //SELECT itemID, regionID, constellationID, solarSystemID, typeID, x, y, z FROM mapDenormalize
        StaticData staticData;
        staticData.itemID          = row.GetInt(0);
        staticData.regionID        = row.GetInt(1);
        staticData.constellationID = row.GetInt(2);
        staticData.systemID        = row.GetInt(3);
        staticData.typeID          = row.GetInt(4);
        staticData.position        = GPoint(row.GetDouble(5),row.GetDouble(6),row.GetDouble(7));
        m_staticData.emplace(row.GetInt(0), staticData);
    }
    sLog.Cyan("    StaticDataMgr", "%u Static Entity data sets loaded in %.3fms.", m_staticData.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    MapDB::GetStationCount(*res);
    while (res->GetRow(row)) {
        //SELECT map.solarSystemID, count(sta.stationID) FROM staStations sta
        m_stationCount.emplace(row.GetInt(0), row.GetInt(1));
    }

    res->Reset();
    StationDB::GetStationRegion(*res);
    while (res->GetRow(row)) {
        //SELECT stationID, regionID FROM staStations
        m_stationRegion.emplace(row.GetInt(0), row.GetInt(1));
    }
    res->Reset();
    StationDB::GetStationConstellation(*res);
    while (res->GetRow(row)) {
        //SELECT stationID, constellationID FROM staStations
        m_stationConst.emplace(row.GetInt(0), row.GetInt(1));
    }
    res->Reset();
    StationDB::GetStationSystem(*res);
    while (res->GetRow(row)) {
        //SELECT stationID, solarSystemID FROM staStations
        m_stationSystem.emplace(row.GetInt(0), row.GetInt(1));
    }

    for (auto cur : m_stationSystem) {
        std::map<uint32, std::vector<uint32>>::iterator itr = m_stationList.find(cur.second), end = m_stationList.end();
        if (itr != end) {
            itr->second.push_back(cur.first);
        } else {
            std::vector<uint32> sVec;
            sVec.push_back(cur.first);
            m_stationList.emplace(std::pair<uint32, std::vector<uint32>>(cur.second, sVec));
        }
    }
    sLog.Cyan("    StaticDataMgr", "%u Static Station query sets loaded in %.3fms.", (m_stationConst.size() + m_stationRegion.size() + m_stationSystem.size() + m_stationList.size()), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    ManagerDB::GetTypeAttributes(*res);
    DmgTypeAttribute typeAttr;
    while (res->GetRow(row)) {
        //SELECT typeID, attributeID, valueInt, valueFloat FROM dgmTypeAttributes
        typeAttr.attributeID = row.GetInt(1);
        if (row.IsNull(2))
            typeAttr.value = row.GetDouble(3);
        else
            typeAttr.value = row.GetInt(2);

        m_typeAttrMap.emplace(row.GetInt(0), typeAttr);
    }
    sLog.Cyan("    StaticDataMgr", "%u Type Attribute Sets loaded in %.3fms", m_typeAttrMap.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    ManagerDB::GetSkillList(*res);
    while (res->GetRow(row)) {
        //SELECT typeID, typeName FROM invTypes [where type=skill]
        m_skills.insert(std::pair<uint16, std::string>(row.GetInt(0), row.GetText(1)));
    }
    sLog.Cyan("    StaticDataMgr", "%u Skills loaded in %.3fms.", m_skills.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    ManagerDB::GetRAMMaterials(*res);
    EvERam::RamMaterials ramMatls;
    while (res->GetRow(row)) {
        //SELECT typeID, materialTypeID, quantity FROM invTypeMaterials
        ramMatls.quantity       = row.GetInt(2);
        ramMatls.materialTypeID = row.GetInt(1);
        m_ramMatl.emplace(row.GetInt(0), ramMatls);
    }
    res->Reset();
    ManagerDB::GetRAMRequirements(*res);
    EvERam::RamRequirements ramReq;
    while (res->GetRow(row)) {
        //SELECT typeID, activityID, requiredTypeID, quantity, damagePerJob, extra FROM ramTypeRequirements
        ramReq.activityID       = row.GetInt(1);
        ramReq.requiredTypeID   = row.GetInt(2);
        ramReq.quantity         = row.GetInt(3);
        ramReq.damagePerJob     = row.GetFloat(4);
        ramReq.extra            = row.GetBool(5);
        m_ramReq.emplace(row.GetInt(0), ramReq);
    }

    sLog.Cyan("    StaticDataMgr", "%u R.A.M. defs loaded in %.3fms.", (m_ramMatl.size() + m_ramReq.size()), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    ManagerDB::GetBlueprintType(*res);
    BlueprintTypeData bpTypeData;
    while (res->GetRow(row)) {
        //SELECT blueprintTypeID, parentBlueprintTypeID, productTypeID, productionTime, techLevel, researchProductivityTime, researchMaterialTime, researchCopyTime,
        //  researchTechTime, productivityModifier, materialModifier, wasteFactor, maxProductionLimit, chanceOfRE, catID FROM invBlueprintTypes
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
        bpTypeData.catID                    = row.GetInt(14);
        m_bpTypeData.emplace(row.GetInt(0), bpTypeData);
    }
    for (auto cur : m_bpTypeData)
        m_bpMatlData[cur.first] = SetBPMatlType(cur.second.catID, cur.first, cur.second.productTypeID);

    sLog.Cyan("    StaticDataMgr", "%u BP Type defs loaded in %.3fms.", (m_bpTypeData.size() + m_bpMatlData.size()), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    ManagerDB::GetMoonResouces(*res);
    while (res->GetRow(row)) {
        //SELECT typeID,volume FROM invTypes [where group=moongoo]
        m_moonGoo.emplace(row.GetInt(0), (uint8)(row.GetFloat(1) *10));
    }
    sLog.Cyan("    StaticDataMgr", "%u Moon Resources loaded in %.3fms.", m_moonGoo.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    ManagerDB::GetOreBySSC(*res);
    OreTypeChance oreChance;
    while (res->GetRow(row)) {
        //SELECT systemSec, roidID, percent FROM roidDistribution
        oreChance.typeID  = row.GetInt(1);
        oreChance.chance  = row.GetFloat(2);
        m_oreBySecClass.emplace(row.GetText(0), oreChance);
    }
    sLog.Cyan("    StaticDataMgr", "%u Ore defs loaded in %.3fms.", m_oreBySecClass.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    //SELECT factionID, itemID FROM facSalvage
    ManagerDB::GetSalvageGroups(*res);
    while( res->GetRow(row) )
        m_salvageMap.emplace(row.GetInt(0), row.GetInt(1));
    sLog.Cyan("    StaticDataMgr", "%u salvage definitions loaded in %.3fms.", m_salvageMap.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    ManagerDB::GetRegionFaction(*res);
    while (res->GetRow(row)) {
        //SELECT regionID, factionID FROM mapRegions
        m_regions.emplace(row.GetInt(0), row.GetInt(1));
    }

    res->Reset();
    ManagerDB::GetRegionRatFaction(*res);
    while (res->GetRow(row)) {
        //SELECT regionID, ratFactionID FROM mapRegions WHERE ratFactionID != 0
        m_ratRegions.emplace(row.GetInt(0), row.GetInt(1));
    }
    sLog.Cyan("    StaticDataMgr", "%u Region Faction Data Sets loaded in %.3fms.", (m_regions.size() + m_ratRegions.size()), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    ManagerDB::GetFactionGroups(*res);
    DBQueryResult* res2 = new DBQueryResult();
    DBResultRow row2;
    RatFactionGroups factionGroup;
    uint16 typeCount = 0;
    while (res->GetRow(row)) {
        //SELECT shipClass, groupID, factionID FROM npcClassGroup
        factionGroup.shipClass = row.GetInt(0);
        factionGroup.groupID = (uint16)row.GetInt(1);
        m_npcGroups.emplace(row.GetInt(2), factionGroup);

        rt_typeIDs rtt;
        ManagerDB::GetGroupTypeIDs((uint8)row.GetInt(0), (uint16)row.GetInt(1), row.GetInt(2), *res2);
        while (res2->GetRow(row2)) {
            //SELECT typeID FROM invTypes WHERE groupID = %u (plus specific checks) ORDER BY typeID
            rtt.push_back((uint16)row2.GetInt(0));
            ++typeCount;
        }
        rt_groups rtg;
        rtg.emplace((uint16)row.GetInt(1), rtt);
        m_npcTypes.emplace((uint8)row.GetInt(0), rtg);
        res2->Reset();
    }

    res->Reset();
    ManagerDB::GetSpawnClasses(*res);
    RatSpawnClass spawnClass;
    while (res->GetRow(row)) {
        //SELECT type, sub, f, af, d, c, ac, bc, bs, h, o, cf, cd, cc, cbc, cbs FROM npcSpawnClass
        spawnClass.type = row.GetInt(0);
        spawnClass.sub = row.GetInt(1);
        spawnClass.f = row.GetInt(2);
        spawnClass.af = row.GetInt(3);
        spawnClass.d = row.GetInt(4);
        spawnClass.c = row.GetInt(5);
        spawnClass.ac = row.GetInt(6);
        spawnClass.bc = row.GetInt(7);
        spawnClass.bs = row.GetInt(8);
        spawnClass.h = row.GetInt(9);
        spawnClass.o = row.GetInt(10);
        spawnClass.cf = row.GetInt(11);
        spawnClass.cd = row.GetInt(12);
        spawnClass.cc = row.GetInt(13);
        spawnClass.cbc = row.GetInt(14);
        spawnClass.cbs = row.GetInt(15);
        m_npcClasses.emplace((uint8)row.GetInt(0), spawnClass);
    }
    sLog.Cyan("    StaticDataMgr", "%u Rat Groups, %u Rat Classes, and %u Rat Types for %u regions loaded in %.3fms.",\
              m_npcGroups.size(), m_npcClasses.size(), typeCount, m_ratRegions.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    SystemDB::GetWrecksToTypes(*res);
    while (res->GetRow(row)) {
        //SELECT typeID, wreckTypeID FROM invTypesToWrecks
        m_WrecksToTypesMap[row.GetInt(0)] = row.GetInt(1);
    }
    sLog.Cyan("    StaticDataMgr", "%u wreck objects loaded in %.3fms.", m_WrecksToTypesMap.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    SystemDB::GetLootGroups(*res);
    LootGroup lootGroup;
    while (res->GetRow(row)) {
        //SELECT npcGroupID, itemGroupID, groupDropChance FROM lootGroup
        lootGroup.lootGroupID = row.GetInt(1);
        lootGroup.dropChance = row.GetDouble(2);
        m_LootGroupMap.emplace(row.GetInt(0), lootGroup);
    }

    res->Reset();
    start = GetTimeMSeconds();
    SystemDB::GetLootGroupTypes(*res);
    LootGroupType GroupType;
    while (res->GetRow(row)) {
        //SELECT itemGroupID, itemID, itemMetaLevel, minAmount, maxAmount FROM lootItemGroup
        GroupType.lootGroupID = row.GetInt(0);
        GroupType.typeID =  row.GetInt(1);
        GroupType.metaLevel = row.GetInt(2);
        GroupType.minQuantity = row.GetInt(3);
        GroupType.maxQuantity = row.GetInt(4);
        m_LootGroupTypeMap.emplace(row.GetInt(0), GroupType);
    }
    sLog.Cyan("    StaticDataMgr", "%u loot groups and %u loot group types loaded in %.3fms.",
              m_LootGroupMap.size(), m_LootGroupTypeMap.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    uint32 locationID = 0;
    ManagerDB::GetAgentLocation(*res);
    while (res->GetRow(row)) {
        //SELECT agentID, locationID FROM agtAgents
        locationID = row.GetInt(1);
        if (IsStation(locationID)) {
            locationID = GetStationSystem(locationID);
        } else if (!IsSolarSystem(locationID)) {
            _log(SERVICE__WARNING, "Failed to query info:  locationID %u is neither station nor system.", locationID);
            continue;
        }

        m_agentSystem.emplace(row.GetInt(0), locationID);
    }
    sLog.Cyan("    StaticDataMgr", "%u Agent Data Sets loaded in %.3fms.", m_agentSystem.size(), (GetTimeMSeconds() - start));

    //cleanup
    SafeDelete(res);
    SafeDelete(res2);

    sLog.Cyan("    StaticDataMgr", "Static Data loaded in %.3fms.", (GetTimeMSeconds() - begin));
}

void StaticDataMgr::GetInfo()
{
    /* return info about loaded items? */
    /*
     * m_systemData;
     * m_staticData;
     *
     */
}

PyInt* StaticDataMgr::GetAgentSystemID(int32 agentID)
{
    std::map<uint32, uint32>::iterator itr = m_agentSystem.find(agentID);
    if (itr != m_agentSystem.end())
        return new PyInt(itr->second);

    _log(DATABASE__MESSAGE, "Failed to query system info for agent %u: Agent not found.", agentID);
    return new PyInt(0);
}

void StaticDataMgr::GetSalvage(uint32 factionID, std::vector<uint32> &itemList) {
    auto itr = m_salvageMap.equal_range(factionID);
    for (auto it = itr.first; it != itr.second; ++it)
        itemList.push_back(it->second);
}

bool StaticDataMgr::GetRoidDist(const char* secClass, std::unordered_multimap<float, uint16>& roids) {
    auto groupRange = m_oreBySecClass.equal_range(secClass);
    for (auto it = groupRange.first; it != groupRange.second; ++it) {
        _log(MINING__INFO, "GetRoidDist - adding %u with chance %.3f", it->second.typeID, it->second.chance);
        roids.insert(std::pair<float, uint32>(it->second.chance, it->second.typeID));
    }

    return !roids.empty();
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
    std::map<uint16, std::string>::iterator itr = m_skills.find(skillID);
    if (itr != m_skills.end()) {
        name = itr->second;
        return true;
    }

    _log(DATABASE__MESSAGE, "Failed to query name for skill %u: Skill not found.", skillID);
    return false;
}

void StaticDataMgr::GetMoonResouces(std::map<uint16, uint8>& data)
{
    // make copy
    for (auto cur : m_moonGoo)
        data.emplace(cur.first, cur.second);
}

uint16 StaticDataMgr::GetRandRatType(uint8 sClass, uint16 groupID)
{
    if (groupID == 0)
        return 0;
    std::vector< uint16 > typeVec;
    auto classRange = m_npcTypes.equal_range(sClass);
    for (auto it = classRange.first; it != classRange.second; ++it) {
        for (auto itr : it->second)
            if (itr.first == groupID) {
                for (auto tItr : itr.second)
                    typeVec.push_back(tItr);
                break;
            }
    }
    if (typeVec.size() < 1)
        return 0;
    return typeVec.at(MakeRandomInt(0, typeVec.size()));
}

bool StaticDataMgr::GetNPCTypes(uint16 groupID, std::vector< uint16 >& typeVec)
{
    /*  this is now invalid.....
    auto groupRange = m_npcTypes.equal_range(groupID);
    for (auto it = groupRange.first; it != groupRange.second; ++it)
        typeVec.push_back(it->second);

    return !typeVec.empty();
    */
    return false;
}

bool StaticDataMgr::GetNPCGroups(uint32 factionID, std::map< uint8, uint16 >& groupMap)
{
    auto groupRange = m_npcGroups.equal_range(factionID);
    for (auto it = groupRange.first; it != groupRange.second; ++it)
        groupMap.emplace(it->second.shipClass, it->second.groupID);

    return !groupMap.empty();
}

bool StaticDataMgr::GetNPCClasses(uint8 sClass, std::vector< RatSpawnClass >& classMap)
{
    auto classRange = m_npcClasses.equal_range(sClass);
    for (auto it = classRange.first; it != classRange.second; ++it) {
        RatSpawnClass spawnClass;
        spawnClass.type = it->second.type;
        spawnClass.sub = it->second.sub;
        spawnClass.f = it->second.f;
        spawnClass.af = it->second.af;
        spawnClass.d = it->second.d;
        spawnClass.c = it->second.c;
        spawnClass.ac = it->second.ac;
        spawnClass.bc = it->second.bc;
        spawnClass.bs = it->second.bs;
        spawnClass.h = it->second.h;
        spawnClass.o = it->second.o;
        spawnClass.cf = it->second.cf;
        spawnClass.cd = it->second.cd;
        spawnClass.cc = it->second.cc;
        spawnClass.cbc = it->second.cbc;
        spawnClass.cbs = it->second.cbs;
        classMap.push_back(spawnClass);
    }

    return !classMap.empty();
}

uint32 StaticDataMgr::GetWreckID(uint32 typeID)
{
    std::map<uint32, uint32>::const_iterator itr = m_WrecksToTypesMap.find(typeID);
    if (itr != m_WrecksToTypesMap.end())
        return itr->second;
    return 0;
}

void StaticDataMgr::GetLoot(uint32 groupID, std::vector<LootList>& lootList) {
    double profileStartTime = GetTimeUSeconds();

    float randChance = 0.0f;
    uint8 metaLevel = 0;
    std::vector<LootGroupType> lootGrpVec;
    lootGrpVec.clear();

    // Finds a range containing all elements whose key is k.
    // pair<iterator, iterator> equal_range(const key_type& k)
    auto range = m_LootGroupMap.equal_range(groupID);
    for ( auto it = range.first; it != range.second; ++it ) {
        // make lootMap of lootGroupID's
        if (MakeRandomFloat(0, 1) < it->second.dropChance) {
            randChance = MakeRandomFloat(0, 1);
            metaLevel = 0;
            if (randChance < 0.1)
                metaLevel = 4;
            else if (randChance < 0.25)
                metaLevel = 3;
            else if (randChance < 0.4)
                metaLevel = 2;
            else if (randChance < 0.6)
                metaLevel = 1;
            /*need to figure out how to get faction loot for faction wrecks
             *    elif meta_level == 7:
             *        drop_chance = 0.15   # Faction stuff = 15%
             *    elif meta_level == 8:
             *        drop_chance = 0.15   # Faction projectiles = 15%
             *    elif meta_level == 9:
             *        drop_chance = 0.15   # Faction SB's and Missile launchers
             */

            auto range2 = m_LootGroupTypeMap.equal_range(it->second.lootGroupID);
            for (auto it2 = range2.first; it2 != range2.second; ++it2)
                if (it2->second.metaLevel == metaLevel)
                    lootGrpVec.push_back(it2->second);

            if (!lootGrpVec.empty()) {
                LootList loot_list;
                uint16 i = MakeRandomInt(0, lootGrpVec.size());
                loot_list.itemID = lootGrpVec[i].typeID;
                loot_list.minDrop = lootGrpVec[i].minQuantity;
                loot_list.maxDrop = lootGrpVec[i].maxQuantity;
                lootList.push_back(loot_list);
                lootGrpVec.clear();
            }
        }
    }

    if (sConfig.debug.UseProfiling)
        sProfile.AddTime(_lootProfile, GetTimeUSeconds() - profileStartTime);
}

void StaticDataMgr::GetBpTypeData(uint32 typeID, BlueprintTypeData& bpData)
{
    std::map<uint16, BlueprintTypeData>::iterator itr = m_bpTypeData.find(typeID);
    if (itr != m_bpTypeData.end())
        bpData = itr->second;
    else
        _log(DATABASE__MESSAGE, "Failed to query info for bpType %u: Type not found.", typeID);
}

void StaticDataMgr::GetRamReturns(uint16 typeID, int8 activityID, std::vector< EvERam::RequiredItem >& ramReqs)
{
    auto itr = m_ramReq.equal_range(typeID);
    for (auto it = itr.first; it != itr.second; ++it)
        if ((it->second.activityID = activityID) and (it->second.extra))
            ramReqs.push_back(EvERam::RequiredItem(it->second.requiredTypeID, it->second.quantity, it->second.damagePerJob, IsSkillTypeID(it->second.requiredTypeID), it->second.extra));
}

void StaticDataMgr::GetRamMaterials(uint16 typeID, std::vector< EvERam::RamMaterials >& ramMatls)
{
    auto itr = m_ramMatl.equal_range(typeID);
    for (auto it = itr.first; it != itr.second; ++it)
        ramMatls.push_back(it->second);
}

void StaticDataMgr::GetRamRequirements(uint16 typeID, std::vector< EvERam::RamRequirements >& ramReqs)
{
    auto itr = m_ramReq.equal_range(typeID);
    for (auto it = itr.first; it != itr.second; ++it)
        ramReqs.push_back(it->second);
}

void StaticDataMgr::GetRamRequiredItems(const uint32 typeID, const int8 activity, std::vector< EvERam::RequiredItem >& into)
{
    if (activity == EvERam::Activity::Manufacturing) {
        std::map<uint16, BlueprintTypeData>::iterator itr = m_bpTypeData.find(typeID);
        if (itr != m_bpTypeData.end()) {
            auto range = m_ramMatl.equal_range(itr->second.productTypeID);
            for (auto it = range.first; it != range.second; ++it)
                into.push_back(EvERam::RequiredItem(it->second.materialTypeID, it->second.quantity, 0, false, false));
        }
    }

    auto itr = m_ramReq.equal_range(typeID);
    for (auto it = itr.first; it != itr.second; ++it)
        if (activity == it->second.activityID)
            into.push_back(EvERam::RequiredItem(it->second.requiredTypeID, it->second.quantity, it->second.damagePerJob, IsSkillTypeID(it->second.requiredTypeID), it->second.extra));
}

PyRep* StaticDataMgr::GetStationCount()
{
    PyList* list = new PyList();
    std::map<uint32, uint8>::iterator itr = m_stationCount.begin(), end = m_stationCount.end();
    while (itr != end) {
        PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyInt(itr->first));
        tuple->SetItem(1, new PyInt(itr->second));
        list->AddItem(tuple);
        ++itr;
    }
    return list;
}

uint8 StaticDataMgr::GetStationCount(uint32 systemID)
{
    std::map<uint32, uint8>::iterator itr = m_stationCount.find(systemID);
    if (itr != m_stationCount.end())
        return itr->second;

    _log(DATABASE__MESSAGE, "Failed to query station count for system %u: System not found.", systemID);
    return 0;
}

bool StaticDataMgr::GetStationList(uint32 systemID, std::vector< uint32 >& data)
{
    std::map<uint32, std::vector<uint32>>::iterator itr = m_stationList.find(systemID);
    if (itr != m_stationList.end()) {
        data = itr->second;
        return true;
    }
    return false;
}

uint32 StaticDataMgr::GetStationRegion(uint32 stationID)
{
    std::map<uint32, uint32>::iterator itr = m_stationRegion.find(stationID);
    if (itr != m_stationRegion.end())
        return itr->second;

    _log(DATABASE__MESSAGE, "Failed to query region info for station %u: Station not found.", stationID);
    return 0;
}

uint32 StaticDataMgr::GetStationConstellation(uint32 stationID)
{
    std::map<uint32, uint32>::iterator itr = m_stationConst.find(stationID);
    if (itr != m_stationConst.end())
        return itr->second;

    _log(DATABASE__MESSAGE, "Failed to query constellation info for station %u: Station not found.", stationID);
    return 0;
}

uint32 StaticDataMgr::GetStationSystem(uint32 stationID)
{
    std::map<uint32, uint32>::iterator itr = m_stationSystem.find(stationID);
    if (itr != m_stationSystem.end())
        return itr->second;

    _log(DATABASE__MESSAGE, "Failed to query system info for station %u: Station not found.", stationID);
    return 0;
}

uint8 StaticDataMgr::GetWHSystemClass(uint32 systemID)
{
    std::map<uint32, uint8>::iterator itr = m_whRegions.find(systemID);
    if (itr != m_whRegions.end())
        return itr->second;

    itr = m_whRegions.find(sEntityList.FindOrBootSystem(systemID)->GetRegionID());
    if (itr != m_whRegions.end())
        return itr->second;

    // dont have data for systemID nor regionID...throw error and ?something else?
    _log(DATABASE__MESSAGE, "Failed to query WH Class for systemID %u: System not found.", systemID);
    if (IsKSpace(systemID))
        return 0;
    if (IsWSpace(systemID))
        return 0;
}

bool StaticDataMgr::GetSystemInfo(uint32 locationID, SystemData& data)
{
    if (IsStation(locationID)) {
        locationID = GetStationSystem(locationID);
    } else if (!IsSolarSystem(locationID)) {
        _log(SERVICE__WARNING, "Failed to query info:  locationID %u is neither station nor system.", locationID);
        return false;
    }

    std::map<uint32, SystemData>::iterator itr = m_systemData.find(locationID);
    if (itr != m_systemData.end()) {
        data = itr->second;
        return true;
    }

    _log(DATABASE__MESSAGE, "Failed to query info for system %u: System not found.", locationID);
    return false;
}

bool StaticDataMgr::GetStaticInfo(uint32 itemID, StaticData& data)
{
    std::map<uint32, StaticData>::iterator itr = m_staticData.find(itemID);
    if (itr != m_staticData.end()) {
        data = itr->second;
        return true;
    }

    _log(DATABASE__MESSAGE, "Failed to query info for static item %u: Item not found.", itemID);
    return false;
}

uint16 StaticDataMgr::GetStaticType(uint32 itemID)
{
    std::map<uint32, StaticData>::iterator itr = m_staticData.find(itemID);
    if (itr != m_staticData.end())
        return itr->second.typeID;
    return 0;
}

uint32 StaticDataMgr::GetRegionFaction(uint32 regionID)
{
    std::map<uint32, uint32>::iterator itr = m_regions.find(regionID);
    if (itr != m_regions.end())
        return itr->second;

    _log(DATABASE__MESSAGE, "Failed to query faction for region %u: region not found.", regionID);
    return 0;
}

uint32 StaticDataMgr::GetRegionRatFaction(uint32 regionID)
{
    std::map<uint32, uint32>::iterator itr = m_ratRegions.find(regionID);
    if (itr != m_ratRegions.end())
        return itr->second;

    _log(DATABASE__MESSAGE, "Failed to query rat faction for region %u: region not found.", regionID);
    return 0;

/*
def GetPirateFactionsOfRegion(self, regionID):
return {10000001: (500019,),
    10000002: (500010,),
    10000003: (500010,),
    10000005: (500011,),
    10000006: (500011,),
    10000007: (500011,),
    10000008: (500011,),
    10000009: (500011,),
    10000010: (500010,),
    10000011: (500011,),
    10000012: (500011,),
    10000014: (500019,),
    10000015: (500010,),
    10000016: (500010,),
    10000020: (500019,),
    10000022: (500019,),
    10000023: (500010,),
    10000025: (500011,),
    10000028: (500011,),
    10000029: (500010,),
    10000030: (500011,),
    10000031: (500011,),
    10000032: (500020,),
    10000033: (500010,),
    10000035: (500010,),
    10000036: (500019,),
    10000037: (500020,),
    10000038: (500012,),
    10000039: (500019,),
    10000041: (500020,),
    10000042: (500011,),
    10000043: (500019,),
    10000044: (500020,),
    10000045: (500010,),
    10000046: (500020,),
    10000047: (500019,),
    10000048: (500020,),
    10000049: (500012, 500019),
    10000050: (500012,),
    10000051: (500020,),
    10000052: (500012,),
    10000054: (500012,),
    10000055: (500010,),
    10000056: (500011,),
    10000057: (500020,),
    10000058: (500020,),
    10000059: (500019,),
    10000060: (500012,),
    10000061: (500011,),
    10000062: (500011,),
    10000063: (500012,),
    10000064: (500020,),
    10000065: (500012,),
    10000067: (500012,),
    10000068: (500020,)}
    */
}

PyDict* StaticDataMgr::GetBPMatlData(uint16 typeID)
{
    auto itr = m_bpMatlData.find(typeID);
    if (itr != m_bpMatlData.end()) {
        PyIncRef(itr->second);
        //itr->second->Dump(MANUF__DUMP, "    ");
        return itr->second;
    }
    return nullptr;
}

PyDict* StaticDataMgr::SetBPMatlType(int8 catID, uint16 typeID, uint16 prodID)
{
    // dunno how to do this part better...
    PyList* matlListManuf = new PyList();
    PyList* skillListManuf = new PyList();
    PyList* extraListManuf = new PyList();
    PyList* matlListTE = new PyList();
    PyList* skillListTE = new PyList();
    PyList* matlListME = new PyList();
    PyList* skillListME = new PyList();
    PyList* matlListCopy = new PyList();
    PyList* skillListCopy = new PyList();
    PyList* matlListDup = new PyList();
    PyList* skillListDup = new PyList();
    PyList* extraListDup = new PyList();
    PyList* matlListRE = new PyList();
    PyList* skillListRE = new PyList();
    PyList* matlListInvent = new PyList();
    PyList* skillListInvent = new PyList();

    DBRowDescriptor* header = new DBRowDescriptor;
        header->AddColumn( "quantity",          DBTYPE_I4 );
        header->AddColumn( "requiredTypeID",    DBTYPE_I4 );
        header->AddColumn( "damagePerJob",      DBTYPE_R4 );

    // NOTE: this is for BLUEPRINTS ONLY and is always populated (ancient relic error fix)
    if (catID == EVEDB::invCategories::Blueprint) {
        // ramMaterials is only for manufacturing the bp product
        std::vector<EvERam::RamMaterials> ramMatls;
        ramMatls.clear();
        GetRamMaterials(prodID, ramMatls);
        for (auto cur : ramMatls) {
            PyPackedRow* row = new PyPackedRow( header );
                row->SetField( "quantity",        new PyInt(cur.quantity));
                row->SetField( "requiredTypeID",  new PyInt(cur.materialTypeID));
                row->SetField( "damagePerJob",    new PyFloat(1.0));
            matlListManuf->AddItem(row);
        }
    }

    // booleans to only set items that are populated  NOTE: manuf is always populated for blueprints
    bool manuf = false, copy = false, invent = false, dup = false, me = false, re = false, te = false, tech = false;
    //  get R.A.M. skills and materials for both bp typeID and product typeID
    // the ramRequirements table holds ALL skill/item data for all aspects of RAM per BlueprintTypeID.
    std::vector<EvERam::RamRequirements> ramReqs;
    ramReqs.clear();
    GetRamRequirements(typeID, ramReqs);
    //GetRamRequirements(prodID, ramReqs);
    for (auto cur : ramReqs) {
        PyPackedRow* row = new PyPackedRow( header );
            row->SetField("quantity",        new PyInt(cur.quantity));
            row->SetField("requiredTypeID",  new PyInt(cur.requiredTypeID));
            row->SetField("damagePerJob",    new PyFloat(cur.damagePerJob));

        using namespace EvERam;
        switch(cur.activityID) {
            case Activity::Manufacturing: {         //1
                /** @todo  this needs work.  dunno how to remove 'extra' materials from this list */
                manuf = true;
                if (IsSkillTypeID(cur.requiredTypeID))
                    skillListManuf->AddItem(row);
                else if (cur.extra)
                    extraListManuf->AddItem(row);
                else
                    matlListManuf->AddItem(row);
            } break;
            case Activity::ResearchTech: {          //2
                // not used.  not defined in client.  no data for this activity
            } break;
            case Activity::ResearchTime: {          //3
                te = true;
                if (IsSkillTypeID(cur.requiredTypeID))
                    skillListTE->AddItem(row);
                else
                    matlListTE->AddItem(row);
            } break;
            case Activity::ResearchMaterial: {      //4
                me = true;
                if (IsSkillTypeID(cur.requiredTypeID))
                    skillListME->AddItem(row);
                else
                    matlListME->AddItem(row);
            } break;
            case Activity::Copying: {               //5
                copy = true;
                if (IsSkillTypeID(cur.requiredTypeID))
                    skillListCopy->AddItem(row);
                else
                    matlListCopy->AddItem(row);
            } break;
            case Activity::Duplicating: {           //6
                dup = true;
                if (IsSkillTypeID(cur.requiredTypeID))
                    skillListDup->AddItem(row);
                else if (cur.extra)
                    extraListDup->AddItem(row);
                else
                    matlListDup->AddItem(row);
            } break;
            case Activity::ReverseEngineering: {    //7
                re = true;
                if (IsSkillTypeID(cur.requiredTypeID))
                    skillListRE->AddItem(row);
                else
                    matlListRE->AddItem(row);
            } break;
            case Activity::Invention: {             //8
                invent = true;
                if (IsSkillTypeID(cur.requiredTypeID))
                    skillListInvent->AddItem(row);
                else
                    matlListInvent->AddItem(row);
            } break;
        }
    }

    // this is the response.  test for items populated above and create an ItemString in the dict for that item.
    // items not populated will not be shown in the BP info.
    DBQueryResult mtRes;
    PyRep* mtCRowSet = DBResultToCRowset(mtRes);
    PyDict* rsp = new PyDict();
    // activity '0' should stay empty
    //activityNone = 0
    //rsp->SetItem(0, new PyDict());

    if (manuf) {        //activityManufacturing = 1
        PyDict* Manufacturing = new PyDict();
            Manufacturing->SetItemString("skills", skillListManuf);
            Manufacturing->SetItemString("rawMaterials", matlListManuf);
            CRowSet *rowset = new CRowSet( &header );
            PyList::const_iterator itr = extraListManuf->begin();
            for (; itr != extraListManuf->end(); ++itr) {
                PyPackedRow* from = (*itr)->AsPackedRow();
                PyPackedRow* into = rowset->NewRow();
                into->SetField((uint32)0, from->GetField(0));
                into->SetField((uint32)1, from->GetField(1));
                into->SetField((uint32)2, from->GetField(2));
            }
            Manufacturing->SetItemString("extras", rowset);     // have to build a crowset for this
        rsp->SetItem(new PyInt(1), new PyObject("util.KeyVal", Manufacturing));
    }
    if (tech) {        //activityResearchingTechnology = 2
        // not used.  not defined in client.  no data for this activity
    }
    if (te) {        //activityResearchingTimeProductivity = 3
        PyDict* ResearchTime = new PyDict();
            ResearchTime->SetItemString("skills", skillListTE);
            ResearchTime->SetItemString("rawMaterials", matlListTE);
            PyIncRef(mtCRowSet);
            ResearchTime->SetItemString("extras", mtCRowSet);
        rsp->SetItem(new PyInt(3), new PyObject("util.KeyVal", ResearchTime));
    }
    if (me) {        //activityResearchingMaterialProductivity = 4
        PyDict* ResearchMaterial = new PyDict();
            ResearchMaterial->SetItemString("skills", skillListME);
            ResearchMaterial->SetItemString("rawMaterials", matlListME);
            PyIncRef(mtCRowSet);
            ResearchMaterial->SetItemString("extras", mtCRowSet);
        rsp->SetItem(new PyInt(4), new PyObject("util.KeyVal", ResearchMaterial));
    }
    if (copy) {        //activityCopying = 5
        PyDict* Copying = new PyDict();
            Copying->SetItemString("skills", skillListCopy);
            Copying->SetItemString("rawMaterials", matlListCopy);
            PyIncRef(mtCRowSet);
            Copying->SetItemString("extras", mtCRowSet);
        rsp->SetItem(new PyInt(5), new PyObject("util.KeyVal", Copying));
    }
    if (dup) {       //activityDuplicating = 6
        // no longer used...updated to "copying" after RMR
        PyDict* Duplicating = new PyDict();
            Duplicating->SetItemString("skills", skillListDup);
            Duplicating->SetItemString("rawMaterials", matlListDup);
            CRowSet *rowset = new CRowSet( &header );
            PyList::const_iterator itr = extraListDup->begin();
            for (; itr != extraListDup->end(); ++itr) {
                PyPackedRow* from = (*itr)->AsPackedRow();
                PyPackedRow* into = rowset->NewRow();
                into->SetField((uint32)0, from->GetField(0));
                into->SetField((uint32)1, from->GetField(1));
                into->SetField((uint32)2, from->GetField(2));
            }
            Duplicating->SetItemString("extras", rowset);    // have to build a crowset for this
        rsp->SetItem(new PyInt(6), new PyObject("util.KeyVal", Duplicating));
    }
    if (re) {        //activityReverseEngineering = 7
        PyDict* ReverseEngineering = new PyDict();
            ReverseEngineering->SetItemString("skills", skillListRE);
            ReverseEngineering->SetItemString("rawMaterials", matlListRE);
            PyIncRef(mtCRowSet);
            ReverseEngineering->SetItemString("extras", mtCRowSet);
        rsp->SetItem(new PyInt(7), new PyObject("util.KeyVal", ReverseEngineering));
    }
    if (invent) {     //activityInvention = 8
        PyDict* Invention = new PyDict();
            Invention->SetItemString("skills", skillListInvent);
            Invention->SetItemString("rawMaterials", matlListInvent);
            PyIncRef(mtCRowSet);
            Invention->SetItemString("extras", mtCRowSet);
        rsp->SetItem(new PyInt(8), new PyObject("util.KeyVal", Invention));
    }
    return rsp;
}

uint8 StaticDataMgr::GetRegionQuarter(uint32 regionID)
{
    uint32 factionID = 0;
    std::map<uint32, uint32>::iterator itr = m_regions.find(regionID);
    if (itr != m_regions.end())
        factionID = (*itr).second;

    // caldari=1, minmatar=2, amarr=3, gallente=4, none=5
    switch (factionID) {
        case factionCaldari:        //Caldari State
        case factionGuristas:       //Guristas Pirates
            return 1;
        case factionMinmatar:       //Minmatar Republic
        case factionAngel:          //Angel Cartel
            return 2;
        case factionAmarr:          //Amarr Empire
        case factionAmmatar:        //Ammatar Mandate
        case factionKhanid:         //Khanid Kingdom
        case factionBloodRaider:    //Blood Raider Covenant
        case factionSanshas:        //Sansha's Nation
            return 3;
        case factionGallente:       //Gallente Federation
        case factionSerpentis:      //Serpentis
            return 4;
        case factionJove:           //Jove Empire
        case factionCONCORD:        //CONCORD Assembly
        case factionSyndicate:      //The Syndicate
        case factionInterBus:       //The InterBus
        case factionORE:            //ORE
        case factionThukker:        //Thukker Tribe
        case factionSistersOfEVE:   //Servant Sisters of EVE
        case factionSociety:        //The Society of Conscious Thought
        case factionMordusLegion:   //Mordu's Legion Command
            return 5;
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

        case factionCONCORD:          return corpCONCORD;
        case factionInterBus:       return corpInterbus;
        case factionSociety:          return corpSociety;
        case factionMordusLegion:        return corpMordusLegion;

        /**  @todo  these are waiting for corp def updates...
        case factionSleepers:    return corpRogueDrones;

        case factionCaldari:          return corpAngel;
        case factionMinmatar:        return corpSanshas;
        case factionAmarr:    return corpBloodRaider;
        case factionGallente:       return corpGuristas;
        case factionJove:      return corpSerpentis;
        case factionAmmatar:    return corpRogueDrones;

        case factionKhanid:        return corpSanshas;
        case factionSyndicate:    return corpBloodRaider;
        case factionORE:      return corpSerpentis;
        case factionThukker:    return corpRogueDrones;
        case factionSistersOfEVE:    return corpRogueDrones;
        */

        case factionNoFaction:
        default:                return 0;
    }
}

std::string StaticDataMgr::GetCorpName(uint32 corpID)
{
    switch (corpID) {
        case corpAngel:             return "Angel";
        case corpSanshas:           return "Sansha";
        case corpBloodRaider:       return "Blood";
        case corpGuristas:          return "Guristas";
        case corpSerpentis:         return "Serpentis";
        case corpRogueDrones:       return "Drone";
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

uint32 StaticDataMgr::GetRaceFaction(EVERace raceID)
{
    switch (raceID) {
        case raceCaldari:       return factionCaldari;
        case raceMinmatar:      return factionMinmatar;
        case raceAmarr:         return factionAmarr;
        case raceGallente:      return factionGallente;
        case raceJove:          return factionJove;
        case racePirate:        return factionNoFaction;
        case raceSleepers:      return factionSleepers;
        case raceORE:           return factionORE;
    }
}

EVERace StaticDataMgr::GetFactionRace(uint32 factionID)
{
    switch (factionID) {
        case factionCaldari:        return raceCaldari;
        case factionMinmatar:       return raceMinmatar;
        case factionAmarr:          return raceAmarr;
        case factionGallente:       return raceGallente;
        case factionJove:           return raceJove;
        case factionNoFaction:      return racePirate;
        case factionSleepers:       return raceSleepers;
        case factionORE:            return raceORE;
        case factionAmmatar:        return raceAmmatar;
    }
}

std::string StaticDataMgr::GetRigSizeName(uint8 size)
{
    switch (size) {
        case 0:      return "Undefined";
        case 1:      return "Small";
        case 2:      return "Medium";
        case 3:      return "Large";
        case 4:      return "Capitol";
    }
}

std::string StaticDataMgr::GetFlagName(uint16 flag)
{
    return GetFlagName((EVEItemFlags)flag);
}

std::string StaticDataMgr::GetFlagName(EVEItemFlags flag)
{
    switch (flag) {
        case flagAutoFit:                               return "flagAutoFit";
        case flagWallet:                                return "flagWallet";
        case flagFactory:                               return "flagFactory";
        case flagWardrobe:                              return "flagWardrobe";
        case flagHangar:                                return "flagHangar";
        case flagCargoHold:                             return "flagCargoHold";
        case flagBriefcase:                             return "flagBriefcase";
        case flagSkill:                                 return "flagSkill";
        case flagReward:                                return "flagReward";
        case flagConnected:                             return "flagConnected";
        case flagDisconnected:                          return "flagDisconnected";
        case flagLowSlot0:                              return "flagLowSlot0";
        case flagLowSlot1:                              return "flagLowSlot1";
        case flagLowSlot2:                              return "flagLowSlot2";
        case flagLowSlot3:                              return "flagLowSlot3";
        case flagLowSlot4:                              return "flagLowSlot4";
        case flagLowSlot5:                              return "flagLowSlot5";
        case flagLowSlot6:                              return "flagLowSlot6";
        case flagLowSlot7:                              return "flagLowSlot7";
        case flagMedSlot0:                              return "flagMedSlot0";
        case flagMedSlot1:                              return "flagMedSlot1";
        case flagMedSlot2:                              return "flagMedSlot2";
        case flagMedSlot3:                              return "flagMedSlot3";
        case flagMedSlot4:                              return "flagMedSlot4";
        case flagMedSlot5:                              return "flagMedSlot5";
        case flagMedSlot6:                              return "flagMedSlot6";
        case flagMedSlot7:                              return "flagMedSlot7";
        case flagHiSlot0:                               return "flagHiSlot0";
        case flagHiSlot1:                               return "flagHiSlot1";
        case flagHiSlot2:                               return "flagHiSlot2";
        case flagHiSlot3:                               return "flagHiSlot3";
        case flagHiSlot4:                               return "flagHiSlot4";
        case flagHiSlot5:                               return "flagHiSlot5";
        case flagHiSlot6:                               return "flagHiSlot6";
        case flagHiSlot7:                               return "flagHiSlot7";
        case flagFixedSlot:                             return "flagFixedSlot";
        case flagFactoryBlueprint:                      return "flagFactoryBlueprint";
        case flagFactoryMinerals:                       return "flagFactoryMinerals";
        case flagFactoryOutput:                         return "flagFactoryOutput";
        case flagFactoryActive:                         return "flagFactoryActive";
        case flagPromenadeSlot1:                        return "flagPromenadeSlot1";
        case flagCapsule:                               return "flagCapsule";
        case flagPilot:                                 return "flagPilot";
        case flagPassenger:                             return "flagPassenger";
        case flagBoardingGate:                          return "flagBoardingGate";
        case flagCrew:                                  return "flagCrew";
        case flagSkillInTraining:                       return "flagSkillInTraining";
        case flagCorpMarket:                            return "flagCorpMarket";
        case flagLocked:                                return "flagLocked";
        case flagUnlocked:                              return "flagUnlocked";
        case flagOffice:                                return "flagOffice";
        case flagImpounded:                             return "flagImpounded";
        case flagProperty:                              return "flagProperty";
        case flagDelivery:                              return "flagDelivery";
        case flagBonus:                                 return "flagBonus";
        case flagDroneBay:                              return "flagDroneBay";
        case flagBooster:                               return "flagBooster";
        case flagImplant:                               return "flagImplant";
        case flagShipHangar:                            return "flagShipHangar";
        case flagShipOffline:                           return "flagShipOffline";
        case flagRigSlot0:                              return "flagRigSlot0";
        case flagRigSlot1:                              return "flagRigSlot1";
        case flagRigSlot2:                              return "flagRigSlot2";
        case flagRigSlot3:                              return "flagRigSlot3";
        case flagRigSlot4:                              return "flagRigSlot4";
        case flagRigSlot5:                              return "flagRigSlot5";
        case flagRigSlot6:                              return "flagRigSlot6";
        case flagRigSlot7:                              return "flagRigSlot7";
        case flagFactoryOperation:                      return "flagFactoryOperation";
        case flagCorpHangar2:                           return "flagCorpHangar2";
        case flagCorpHangar3:                           return "flagCorpHangar3";
        case flagCorpHangar4:                           return "flagCorpHangar4";
        case flagCorpHangar5:                           return "flagCorpHangar5";
        case flagCorpHangar6:                           return "flagCorpHangar6";
        case flagCorpHangar7:                           return "flagCorpHangar7";
        case flagSecondaryStorage:                      return "flagSecondaryStorage";
        case flagCaptainsQuarters:                      return "flagCaptainsQuarters";
        case flagWisPromenade:                          return "flagWisPromenade";
        //case flagWorldSpace:                          return "flagWorldSpace";
        case flagSubSystem0:                            return "flagSubSystem0";
        case flagSubSystem1:                            return "flagSubSystem1";
        case flagSubSystem2:                            return "flagSubSystem2";
        case flagSubSystem3:                            return "flagSubSystem3";
        case flagSubSystem4:                            return "flagSubSystem4";
        case flagSubSystem5:                            return "flagSubSystem5";
        case flagSubSystem6:                            return "flagSubSystem6";
        case flagSubSystem7:                            return "flagSubSystem7";
        case flagSpecializedFuelBay:                    return "flagSpecializedFuelBay";
        case flagSpecializedOreHold:                    return "flagSpecializedOreHold";
        case flagSpecializedGasHold:                    return "flagSpecializedGasHold";
        case flagSpecializedMineralHold:                return "flagSpecializedMineralHold";
        case flagSpecializedSalvageHold:                return "flagSpecializedSalvageHold";
        case flagSpecializedShipHold:                   return "flagSpecializedShipHold";
        case flagSpecializedSmallShipHold:              return "flagSpecializedSmallShipHold";
        case flagSpecializedMediumShipHold:             return "flagSpecializedMediumShipHold";
        case flagSpecializedLargeShipHold:              return "flagSpecializedLargeShipHold";
        case flagSpecializedIndustrialShipHold:         return "flagSpecializedIndustrialShipHold";
        case flagSpecializedAmmoHold:                   return "flagSpecializedAmmoHold";
        case flagStructureActive:                       return "flagStructureActive";
        case flagStructureInactive:                     return "flagStructureInactive";
        case flagJunkyardReprocessed:                   return "flagJunkyardReprocessed";
        case flagJunkyardTrashed:                       return "flagJunkyardTrashed";
        case flagSpecializedCommandCenterHold:          return "flagSpecializedCommandCenterHold";
        case flagSpecializedPlanetaryCommoditiesHold:   return "flagSpecializedPlanetaryCommoditiesHold";
        case flagPlanetSurface:                         return "flagPlanetSurface";
        case flagSpecializedMaterialBay:                return "flagSpecializedMaterialBay";
        case flagDustCharacterBackpack:                 return "flagDustCharacterBackpack";
        case flagDustCharacterBattle:                   return "flagDustCharacterBattle";
        case flagQuafeBay:                              return "flagQuafeBay";
        case flagFleetHangar:                           return "flagFleetHangar";
        case flagResearchFacilitySlotFirst:             return "flagResearchFacilitySlotFirst";
        case flagResearchFacilitySlotLast:              return "flagResearchFacilitySlotLast";
        case flagMissile:                               return "flagMissile";
        case flagClone:                                 return "flagClone";
        case flagIllegal:                               return "flagIllegal";
    }
}


