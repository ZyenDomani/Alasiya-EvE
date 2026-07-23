
/**
 * @name StaticDataMgr.cpp
 *   memory object caching system for retrieving, managing and saving ingame data
 *
 * @Author:         Allan
 * @date:   1Jul15 / 1Aug16
 *
 * Original Idea  - 1 July 15
 * Code completion and implementation  - 1 August 2016
 *
 */


#include "StaticDataMgr.h"
#include "EVEServerConfig.h"

#include "character/CharacterDB.h"
#include "database/EVEDBUtils.h"
#include "manufacturing/FactoryDB.h"
#include "map/MapDB.h"
#include "station/StationDB.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/ManagerDB.h"

#include "../eve-common/EVE_Agent.h"
#include "../eve-common/EVE_Character.h"
#include "../eve-common/EVE_POS.h"
#include "../eve-common/EVE_Spawn.h"

/*
 * DATA__ERROR          # specific "data not found but should be there" msgs
 * DATA__WARNING        # misc "data not found but nbd" msgs
 * DATA__MESSAGE        # misc data msgs (mt)
 * DATA__INFO           # data loading msgs (container and amount) (mt)
 */


void StaticDataMgr::Close() {
    Clear();
    sLog.Warning("    StaticDataMgr", "Static Data Manager has been closed.");
}

int StaticDataMgr::Initialize() {
    sLog.Blue("    StaticDataMgr", "Static Data Manager Initialized.");

    m_bpMatlHeader = new DBRowDescriptor();
    m_bpMatlHeader->AddColumn("quantity",          DBTYPE_I4);
    m_bpMatlHeader->AddColumn("requiredTypeID",    DBTYPE_I4);
    m_bpMatlHeader->AddColumn("damagePerJob",      DBTYPE_R4);

    Populate();
    return 1;
}

void StaticDataMgr::Clear() {
    // order dependent!!
    for (auto &cur : m_bpMatlData)
        PyDecRef(cur.second);
    m_bpMatlData.clear();
    PyDecRef(m_bpMatlHeader);

    SafeDelete(m_pFactionInfo);

    m_ramReq.clear();
    m_moonGoo.clear();
    m_ramMatl.clear();
    m_regions.clear();
    m_attrTypeData.clear();
    m_minerals.clear();
    m_compounds.clear();
    m_staticData.clear();
    m_salvageMap.clear();
    m_agentSystem.clear();
    m_corpFaction.clear();
    m_typeAttrMap.clear();
    m_LootGroupMap.clear();
    m_stationConst.clear();
    m_stationRegion.clear();
    m_stationSystem.clear();
    m_oreBySecClass.clear();
    m_LootTypeMap.clear();
    m_WrecksToTypesMap.clear();

    PyDecRef(m_keyMap);
    PyDecRef(m_agents);
    PyDecRef(m_operands);
    PyDecRef(m_billTypes);
    PyDecRef(m_entryTypes);
    PyDecRef(m_factionIDs);
    PyDecRef(m_formations);
    PyDecRef(m_itemHeader);
    PyDecRef(m_npcDivisions);
}

void StaticDataMgr::Populate() {
    double beginTime(GetTimeMSeconds());
    double startTime(GetTimeMSeconds());

    m_itemHeader = CreateItemHeader();
    if (m_itemHeader == nullptr)
        sLog.Error("    StaticDataMgr", "m_itemHeader is null");

    m_formations = CreateFormationTuple();
    if (m_formations == nullptr)
        sLog.Error("    StaticDataMgr", "m_formations is null");

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

    m_factionIDs = ManagerDB::LoadFactionIDs();
    if (m_factionIDs == nullptr)
        sLog.Error("    StaticDataMgr", "m_factionIDs is null");

    sLog.Cyan("    StaticDataMgr", "Base Static data sets loaded in %.3fms.", (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();

    m_pFactionInfo = new GetFactionInfoRsp();
    ManagerDB::LoadCorpFactions(m_pFactionInfo->factionIDbyNPCCorpID);
    ManagerDB::LoadFactionRegions(m_pFactionInfo->factionRegions);
    ManagerDB::LoadFactionConstellations(m_pFactionInfo->factionConstellations);
    ManagerDB::LoadFactionSolarSystems(m_pFactionInfo->factionSolarSystems);
    ManagerDB::LoadFactionRaces(m_pFactionInfo->factionRaces);
    ManagerDB::LoadFactionStationCounts(m_pFactionInfo->factionStationCount);
    ManagerDB::LoadFactionSystemCounts(m_pFactionInfo->factionSolarSystemCount);
    m_pFactionInfo->npcCorpInfo = ManagerDB::LoadNPCCorpInfo();
    m_factionInfo = m_pFactionInfo->Encode();
    if (m_factionInfo == nullptr) {
        sLog.Error("    StaticDataMgr", "m_factionInfo is null");
    } else {
        sLog.Cyan("    StaticDataMgr", "Faction data sets loaded in %.3fms.", (GetTimeMSeconds() - startTime));
    }

    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;

    startTime = GetTimeMSeconds();
    ManagerDB::LoadCorpNames(*res);
    while (res->GetRow(row)) {
        //SELECT corporationID, corporationName FROM crpCorporations
        m_corpName.emplace(row.GetUInt(0), row.GetText(1));
    }
    sLog.Cyan("    StaticDataMgr", "%lu Corp Names loaded in %.3fms.", m_corpName.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::LoadNPCCorpFactionData(*res);
    while (res->GetRow(row)) {
        //SELECT corporationID, factionID FROM crpNPCCorporations
        m_corpFaction.emplace(row.GetUInt(0), row.GetUInt(1));
    }
    sLog.Cyan("    StaticDataMgr", "%lu Corps in NPC Corp Faction map loaded in %.3fms.", m_corpFaction.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetCategoryData(*res);
    m_catData.reserve(res->GetRowCount());
    while (res->GetRow(row)) {
        //SELECT categoryID, categoryName, description, published FROM invCategories
        Inv::CatData data = Inv::CatData();
            data.id                     = row.GetUInt8(0);
            data.name                   = row.GetText(1);
            data.description            = row.GetText(2);
            data.published              = (sConfig.server.AllowNonPublished ? true : row.GetBool(3));
        m_catData.emplace(row.GetUInt(0), std::move(data));
    }
    sLog.Cyan("    StaticDataMgr", "%lu Inventory Categories loaded in %.3fms.", m_catData.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetGroupData(*res);
    m_grpData.reserve(res->GetRowCount());
    while (res->GetRow(row)) {
        //SELECT groupID, categoryID, groupName, description, useBasePrice, allowManufacture, allowRecycler,
        //  anchored, anchorable, fittableNonSingleton, published FROM invGroups
        Inv::GrpData data               = Inv::GrpData();
            data.id                     = row.GetUInt(0);
            data.catID                  = row.GetUInt8(1);
            data.name                   = row.GetText(2);
            data.description            = row.GetText(3);
            data.useBasePrice           = row.GetBool(4);
            data.allowManufacture       = row.GetBool(5);
            data.allowRecycler          = row.GetBool(6);
            data.anchored               = row.GetBool(7);
            data.anchorable             = row.GetBool(8);
            data.fittableNonSingleton   = row.GetBool(9);
            data.published              = (sConfig.server.AllowNonPublished ? true : row.GetBool(10));
        m_grpData.emplace(row.GetUInt(0), std::move(data));
    }
    sLog.Cyan("    StaticDataMgr", "%lu Inventory Groups loaded in %.3fms.", m_grpData.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetTypeData(*res);
    m_typeData.reserve(res->GetRowCount());
    while (res->GetRow(row)) {
        Inv::TypeData data              = Inv::TypeData();
            data.id                     = row.GetUInt(0);
            data.groupID                = row.GetUInt(1);
            data.name                   = row.GetText(2);
            data.description            = row.GetText(3);
            data.radius                 = row.GetFloat(4);
            data.mass                   = row.GetFloat(5);
            data.volume                 = row.GetFloat(6);
            data.capacity               = row.GetFloat(7);
            data.portionSize            = row.GetUInt(8);
            data.race                   = row.GetUInt8(9);
            data.basePrice              = row.GetDouble(10);
            //data.published              = (sConfig.server.AllowNonPublished ? true : row.GetBool(11));
            data.published              = row.GetBool(11);
            data.marketGroupID          = (row.IsNull(12) ? 0 : row.GetUInt(12));
            data.chanceOfDuplicating    = row.GetFloat(13);
            data.metaLvl                = (row.IsNull(14) ? 0 : row.GetUInt(14));
            data.isRefinable            = row.GetUInt(15);
            data.isRecyclable           = row.GetUInt(16);
        m_typeData.emplace(row.GetUInt(0), std::move(data));
    }
    sLog.Cyan("    StaticDataMgr", "%lu Inventory Types loaded in %.3fms.", m_typeData.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetAttributeTypes(*res);
    m_attrTypeData.reserve(res->GetRowCount());
    while (res->GetRow(row)) {
        //SELECT attributeID, attributeName, attributeCategory, displayName, categoryID FROM dgmAttributeTypes
        Inv::AttrTypeData typeData      = Inv::AttrTypeData();
        typeData.attributeID            = row.GetUInt(0);
        typeData.attributeName          = (row.IsNull(1) ? "*none*" : row.GetText(1));
        typeData.attributeCategory      = (row.IsNull(2) ? 0        : row.GetUInt(2));
        typeData.displayName            = (row.IsNull(3) ? "*none*" : row.GetText(3));
        typeData.categoryID             = (row.IsNull(4) ? 0        : row.GetUInt(4));
        m_attrTypeData.emplace(row.GetUInt(0), std::move(typeData));
    }
    sLog.Cyan("    StaticDataMgr", "%lu Attribute data sets loaded in %.3fms.", m_attrTypeData.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetSystemData(*res);
    m_solSysData.reserve(res->GetRowCount());
    while (res->GetRow(row)) {
        // SELECT solarSystemID, constellationID, regionID, solarSystemName, x, y, z,
        // xMin, xMax, yMin, yMax, zMin, zMax, luminosity,
        // border, fringe, corridor, hub, international, regional, constellation,
        // security, factionID, radius, sunTypeID, securityClass, security FROM mapSolarSystems
        SolarSystemData sysData         = SolarSystemData();
        sysData.systemID                = row.GetUInt(0);
        sysData.constellationID         = row.GetUInt(1);
        sysData.regionID                = row.GetUInt(2);
        sysData.name                    = row.GetText(3);
        sysData.position                = Vector3d(row.GetDouble(4), row.GetDouble(5), row.GetDouble(6));
        sysData.minPosition             = Vector3d(row.GetDouble(7), row.GetDouble(8), row.GetDouble(9));
        sysData.maxPosition             = Vector3d(row.GetDouble(10), row.GetDouble(11), row.GetDouble(12));
        sysData.luminosity              = row.GetFloat(13);
        sysData.border                  = row.GetBool(14);
        sysData.fringe                  = row.GetBool(15);
        sysData.corridor                = row.GetBool(16);
        sysData.hub                     = row.GetBool(17);
        sysData.international           = row.GetBool(18);
        sysData.region                  = row.GetBool(19);
        sysData.constellation           = row.GetBool(20);
        sysData.security                = row.GetFloat(21);    // this gives system trueSec
        sysData.factionID               = (row.IsNull(22) ? 0 : row.GetUInt(22));
        sysData.radius                  = row.GetFloat(23);
        sysData.sunTypeID               = row.GetUInt(24);
        sysData.securityClass           = (row.IsNull(25) ? "0" : row.GetText(25));
        m_solSysData.emplace(row.GetUInt(0), std::move(sysData));
        m_constSystems.emplace(row.GetUInt(1), row.GetUInt(0));
        m_regionSystems.emplace(row.GetUInt(2), row.GetUInt(0));
    }
    sLog.Cyan("    StaticDataMgr", "%li Static SolarSystem data sets loaded in %.3fms.", m_solSysData.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetWHSystemClass(*res);
    while (res->GetRow(row)) {
        //SELECT locationID, wormholeClassID FROM mapLocationWormholeClasses
        m_whRegions[row.GetUInt(0)] = row.GetUInt(1);
    }
    sLog.Cyan("    StaticDataMgr", "%lu WH System Classes loaded in %.3fms.", m_whRegions.size(), (GetTimeMSeconds() - startTime));

    // Load wormhole destination classes into static memory object
    startTime = GetTimeMSeconds();
    int size = 0;
    for (int i = 1; i < 10; ++i) {
        ManagerDB::GetWHClassDestinations(i, *res);
        DBResultRow row;
        m_whClassDestinations[i];
        while (res->GetRow(row)) {
            m_whClassDestinations[i].push_back(row.GetUInt(0));
        }
        size += m_whClassDestinations[i].size();
    }

    sLog.Cyan("    StaticDataMgr", "%i WH Destination Classes loaded in %.3fms.",
              size, (GetTimeMSeconds() - startTime));

    // Load wormhole system classes into static memory object
    startTime = GetTimeMSeconds();
    size = 0;
    for (int i = 1; i < 10; ++i) {
        ManagerDB::GetWHClassSystems(i, *res);
        DBResultRow row;
        m_whClassSystems[i];
        while (res->GetRow(row)) {
            m_whClassSystems[i].push_back(row.GetUInt(0));
        }
        size += m_whClassSystems[i].size();
    }

    sLog.Cyan("    StaticDataMgr", "%i WH Class Systems loaded in %.3fms.",
              size, (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetStaticData(*res);
    m_staticData.reserve(res->GetRowCount());
    while (res->GetRow(row)) {
        //SELECT itemID, regionID, constellationID, solarSystemID, typeID, radius, x, y, z FROM mapDenormalize
        StaticData data         = StaticData();
        data.itemID             = row.GetUInt(0);
        data.regionID           = row.GetUInt(1);
        data.constellationID    = row.GetUInt(2);
        data.systemID           = row.GetUInt(3);
        data.typeID             = row.GetUInt(4);
        data.radius             = row.GetFloat(5);
        data.position           = Vector3d(row.GetDouble(6),row.GetDouble(7),row.GetDouble(8));
        //m_staticData[row.GetInt(0)] = data;
        m_staticData.emplace(row.GetUInt(0), std::move(data));
    }
    sLog.Cyan("    StaticDataMgr", "%lu Static Entity data sets loaded in %.3fms.", m_staticData.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    MapDB::GetStationCount(*res);
    while (res->GetRow(row)) {
        //SELECT map.solarSystemID, count(sta.stationID) FROM staStations sta
        m_stationCount.emplace(row.GetUInt(0), row.GetUInt(1));
    }
    StationDB::GetStationRegion(*res);
    while (res->GetRow(row)) {
        //SELECT stationID, regionID FROM staStations
        m_stationRegion.emplace(row.GetUInt(0), row.GetUInt(1));
    }
    StationDB::GetStationConstellation(*res);
    while (res->GetRow(row)) {
        //SELECT stationID, constellationID FROM staStations
        m_stationConst.emplace(row.GetUInt(0), row.GetUInt(1));
    }
    StationDB::GetStationSystem(*res);
    while (res->GetRow(row)) {
        //SELECT stationID, solarSystemID FROM staStations
        m_stationSystem.emplace(row.GetUInt(0), row.GetUInt(1));
    }

    std::unordered_map<uint32, std::vector<uint32>>::iterator itr = m_stationList.begin();
    for (auto &cur : m_stationSystem) {
        itr = m_stationList.find(cur.second);
        if (itr != m_stationList.end()) {
            itr->second.push_back(cur.first);
        } else {
            std::vector<uint32> sVec;
            sVec.push_back(cur.first);
            m_stationList.emplace(cur.second, sVec);
        }
    }
    sLog.Cyan("    StaticDataMgr", "%lu Static Station data sets loaded in %.3fms.", (m_stationConst.size() + m_stationRegion.size() + m_stationSystem.size() + m_stationList.size()), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetTypeAttributes(*res);
    while (res->GetRow(row)) {
        //SELECT typeID, attributeID, valueInt, valueFloat FROM dgmTypeAttributes
        Inv::DmgTypeAttribute typeAttr = Inv::DmgTypeAttribute();
        typeAttr.attributeID = row.GetUInt(1);
        if (row.IsNull(2)) {
            typeAttr.value = row.GetDouble(3);
        } else {
            typeAttr.value = row.GetInt64(2); // highest value seen is 2,000,000,000 (struct HP)
        }

        m_typeAttrMap.emplace(row.GetUInt(0), typeAttr);
    }

    CharacterDB::GetAttributesFromAncestry(*res);
    while (res->GetRow(row)) {
        //SELECT ancestryID, intelligence, charisma, perception, memory, willpower FROM chrAncestries
        Char::AttrData data     = Char::AttrData();
        data.intelligence       = row.GetUInt8(1);
        data.charisma           = row.GetUInt8(2);
        data.perception         = row.GetUInt8(3);
        data.memory             = row.GetUInt8(4);
        data.willpower          = row.GetUInt8(5);
        m_ancestryBonuses[row.GetUInt8(0)] = data;
    }

    CharacterDB::GetAttributesFromBloodline(*res);
    while (res->GetRow(row)) {
        //SELECT bloodlineID, intelligence, charisma, perception, memory, willpowerFROM chrBloodlines
        Char::AttrData data     = Char::AttrData();
        data.intelligence       = row.GetUInt8(1);
        data.charisma           = row.GetUInt8(2);
        data.perception         = row.GetUInt8(3);
        data.memory             = row.GetUInt8(4);
        data.willpower          = row.GetUInt8(5);
        m_bloodlineBonuses[row.GetUInt8(0)] = data;
    }

    sLog.Cyan("    StaticDataMgr", "%lu Type Attribute Sets loaded in %.3fms", m_typeAttrMap.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetSkillList(*res);
    while (res->GetRow(row)) {
        //SELECT typeID, typeName FROM invTypes [where type=skill]
        m_skills[row.GetUInt(0)] = row.GetText(1);
    }
    sLog.Cyan("    StaticDataMgr", "%lu Skills loaded in %.3fms.", m_skills.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    uint piCount = 0;
    FactoryDB::GetComponents(*res);     //766
    while (res->GetRow(row)) {
        //SELECT typeID, typeName FROM invTypes [where type=composite or component]
        m_components[row.GetUInt(0)] = row.GetText(1);
        ++piCount;
    }
    FactoryDB::GetMinerals(*res);       //8
    while (res->GetRow(row)) {
        //SELECT typeID, typeName FROM invTypes [where type=mineral]
        m_minerals[row.GetUInt(0)] = row.GetText(1);
        ++piCount;
    }
    FactoryDB::GetCompounds(*res);      //181
    while (res->GetRow(row)) {
        //SELECT typeID, typeName FROM invTypes [where type=compound]
        m_compounds[row.GetUInt(0)] = row.GetText(1);
        ++piCount;
    }
    FactoryDB::GetSalvage(*res);        //53
    while (res->GetRow(row)) {
        //SELECT typeID, typeName FROM invTypes [where type=salvage]
        m_salvage[row.GetUInt(0)] = row.GetText(1);
        ++piCount;
    }
    FactoryDB::GetResources(*res);      //15
    while (res->GetRow(row)) {
        //SELECT typeID, typeName FROM invTypes [where type=pi resource]
        m_resources[row.GetUInt(0)] = row.GetText(1);
        ++piCount;
    }
    FactoryDB::GetCommodities(*res);    //66
    while (res->GetRow(row)) {
        //SELECT typeID, typeName FROM invTypes [where type=pi commodity]
        m_commodities[row.GetUInt(0)] = row.GetText(1);
        ++piCount;
    }
    FactoryDB::GetMiscCommodities(*res);    //456
    while (res->GetRow(row)) {
        //SELECT typeID, typeName FROM invTypes [where type=misc commodity]
        m_miscCommodities[row.GetUInt(0)] = row.GetText(1);
        ++piCount;
    }
    sLog.Cyan("    StaticDataMgr", "%u PI datasets loaded in %.3fms.", piCount, (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    FactoryDB::GetRAMMaterials(*res);
    while (res->GetRow(row)) {
        //SELECT typeID, materialTypeID, quantity FROM invTypeMaterials
        EvERam::RamMaterials ramMatls = EvERam::RamMaterials();
        ramMatls.quantity       = row.GetUInt(2);
        ramMatls.materialTypeID = row.GetUInt(1);
        m_ramMatl.emplace(row.GetUInt(0), ramMatls);
    }
    FactoryDB::GetRAMRequirements(*res);
    while (res->GetRow(row)) {
        //SELECT typeID, activityID, requiredTypeID, quantity, damagePerJob, extra FROM ramTypeRequirements
        EvERam::RamRequirements ramReq = EvERam::RamRequirements();
        ramReq.activityID       = row.GetUInt(1);
        ramReq.requiredTypeID   = row.GetUInt(2);
        ramReq.quantity         = row.GetUInt(3);
        ramReq.damagePerJob     = row.GetFloat(4);
        ramReq.extra            = row.GetBool(5);
        m_ramReq.emplace(row.GetUInt(0), ramReq);
    }
    sLog.Cyan("    StaticDataMgr", "%lu R.A.M. defs loaded in %.3fms.", (m_ramMatl.size() + m_ramReq.size()), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    FactoryDB::GetBlueprintType(*res);
    while (res->GetRow(row)) {
        //SELECT blueprintTypeID, parentBlueprintTypeID, productTypeID, productionTime, techLevel, researchProductivityTime, researchMaterialTime, researchCopyTime,
        //  researchTechTime, productivityModifier, materialModifier, wasteFactor, maxProductionLimit, chanceOfRE, catID FROM invBlueprintTypes
            EvERam::bpTypeData bpTypeData = EvERam::bpTypeData();
            bpTypeData.parentBlueprintTypeID    = row.GetUInt(1);
            bpTypeData.productTypeID            = row.GetUInt(2);
            bpTypeData.productionTime           = row.GetUInt(3);
            bpTypeData.techLevel                = row.GetUInt(4);
            bpTypeData.researchProductivityTime = row.GetUInt(5);
            bpTypeData.researchMaterialTime     = row.GetUInt(6);
            bpTypeData.researchCopyTime         = row.GetUInt(7);
            bpTypeData.researchTechTime         = row.GetUInt(8);
            bpTypeData.productivityModifier     = row.GetUInt(9);
            bpTypeData.materialModifier         = row.GetUInt(10);
            bpTypeData.wasteFactor              = row.GetUInt(11);
            bpTypeData.maxProductionLimit       = row.GetUInt(12);
            bpTypeData.chanceOfRE               = row.GetFloat(13);
            bpTypeData.catID                    = (row.IsNull(14) ? 0 : row.GetUInt(14));
        m_bpProductData[row.GetUInt(2)] = bpTypeData;
        m_bpTypeData[row.GetUInt(0)] = bpTypeData;
    }

    _log(MANUF__DUMP, "m_bpTypeData.size() = %u", m_bpTypeData.size());
    for (auto &cur : m_bpTypeData)
        m_bpMatlData[cur.first] = SetBPMatlType(cur.second.catID, cur.first, cur.second.productTypeID);

    sLog.Cyan("    StaticDataMgr", "%lu BP Type defs loaded in %.3fms.", m_bpTypeData.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetMoonResouces(*res);
    while (res->GetRow(row)) {
        //SELECT typeID,volume FROM invTypes [where group=moongoo]
        m_moonGoo[row.GetUInt(0)] = (uint8)(row.GetFloat(1) * 10);
    }
    sLog.Cyan("    StaticDataMgr", "%lu Moon Resources loaded in %.3fms.", m_moonGoo.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetOreBySSC(*res);
    while (res->GetRow(row)) {
        //SELECT systemSec, roidID, percent FROM roidDistribution
        OreTypeChance oreChance         = OreTypeChance();
            oreChance.typeID            = row.GetUInt(1);
            oreChance.chance            = row.GetFloat(2);
        m_oreBySecClass.emplace(row.GetText(0), oreChance);
    }
    sLog.Cyan("    StaticDataMgr", "%lu Ore defs loaded in %.3fms.", m_oreBySecClass.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    //SELECT factionID, itemID FROM facSalvage
    ManagerDB::GetSalvageGroups(*res);
    while (res->GetRow(row))
        m_salvageMap.emplace(row.GetUInt(0), row.GetUInt(1));
    sLog.Cyan("    StaticDataMgr", "%lu salvage definitions loaded in %.3fms.", m_salvageMap.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetRegionFaction(*res);
    while (res->GetRow(row)) {
        //SELECT regionID, factionID FROM mapRegions
        m_regions.emplace(row.GetUInt(0), row.GetUInt(1));
    }

    ManagerDB::GetRegionRatFaction(*res);
    while (res->GetRow(row)) {
        //SELECT regionID, ratFactionID FROM mapRegions WHERE ratFactionID != 0
        m_ratRegions.emplace(row.GetUInt(0), row.GetUInt(1));
    }

    ManagerDB::GetFactionNames(*res);
    while (res->GetRow(row)) {
        //SELECT factionID, factionName FROM facFactions
        m_factionName.emplace(row.GetUInt(0), row.GetText(1));
    }

    sLog.Cyan("    StaticDataMgr", "%lu Region Faction Data Sets loaded in %.3fms.", (m_regions.size() + m_ratRegions.size()), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    ManagerDB::GetFactionGroups(*res);
    DBQueryResult* res2 = new DBQueryResult();
    DBResultRow row2;
    uint16 typeCount = 0;
    while (res->GetRow(row)) {
        //SELECT shipClass, groupID, factionID FROM npcClassGroup
        RatFactionGroups factionGroup = RatFactionGroups();
        factionGroup.shipClass  = row.GetUInt8(0);
        factionGroup.groupID    = row.GetUInt(1);
        m_npcGroups.emplace(row.GetUInt(2), factionGroup);

        rt_typeIDs rtt;
        ManagerDB::GetGroupTypeIDs(row.GetUInt8(0), row.GetUInt(1), row.GetUInt(2), *res2);
        while (res2->GetRow(row2)) {
            //SELECT typeID FROM invTypes WHERE groupID = %u (plus specific checks) ORDER BY typeID
            rtt.push_back(row2.GetUInt(0));
            ++typeCount;
        }
        rt_groups rtg;
            rtg.emplace(row.GetUInt(1), rtt);
        m_npcTypes.emplace(row.GetUInt8(0), rtg);
    }

    ManagerDB::GetSpawnClasses(*res);
    while (res->GetRow(row)) {
        //SELECT type, sub, f, af, d, c, ac, bc, bs, h, o, cf, cd, cc, cbc, cbs, className FROM npcSpawnClass
        RatSpawnClass spawnClass        = RatSpawnClass();
        spawnClass.type                 = row.GetUInt8(0);
        spawnClass.sub                  = row.GetUInt8(1);
        spawnClass.f                    = row.GetUInt8(2);
        spawnClass.af                   = row.GetUInt8(3);
        spawnClass.d                    = row.GetUInt8(4);
        spawnClass.c                    = row.GetUInt8(5);
        spawnClass.ac                   = row.GetUInt8(6);
        spawnClass.bc                   = row.GetUInt8(7);
        spawnClass.bs                   = row.GetUInt8(8);
        spawnClass.h                    = row.GetUInt8(9);
        spawnClass.o                    = row.GetUInt8(10);
        spawnClass.cf                   = row.GetUInt8(11);
        spawnClass.cd                   = row.GetUInt8(12);
        spawnClass.cc                   = row.GetUInt8(13);
        spawnClass.cbc                  = row.GetUInt8(14);
        spawnClass.cbs                  = row.GetUInt8(15);
        spawnClass.desc                 = row.GetText(16);
        m_npcClasses.emplace(row.GetUInt8(0), spawnClass);
    }
    sLog.Cyan("    StaticDataMgr", "%lu Rat Groups, %lu Rat Classes, and %u Rat Types for %lu regions loaded in %.3fms.",\
              m_npcGroups.size(), m_npcClasses.size(), typeCount, m_ratRegions.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    SystemDB::GetWrecksToTypes(*res);
    while (res->GetRow(row)) {
        //SELECT typeID, wreckTypeID FROM invTypesToWrecks
        m_WrecksToTypesMap[row.GetUInt(0)] = row.GetUInt(1);
    }
    sLog.Cyan("    StaticDataMgr", "%lu wreck objects loaded in %.3fms.", m_WrecksToTypesMap.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    SystemDB::GetLootGroups(*res);
    while (res->GetRow(row)) {
        //SELECT npcGroupID, itemGroupID, groupDropChance FROM lootGroup
        LootGroup lootGroup             = LootGroup();
        lootGroup.lootGroupID           = row.GetUInt(1);
        lootGroup.dropChance            = row.GetFloat(2);
        m_LootGroupMap.emplace(row.GetUInt(0), lootGroup);
    }

    startTime = GetTimeMSeconds();
    SystemDB::GetLootTypes(*res);
    while (res->GetRow(row)) {
        //SELECT itemGroupID, itemID, itemMetaLevel, itemDropChance, minAmount, maxAmount FROM lootItemGroup
        LootType lootType         = LootType();
        lootType.lootGroupID           = row.GetUInt(0);
        lootType.typeID                = row.GetUInt(1);
        lootType.metaLevel             = row.GetUInt(2);
        lootType.dropChance            = row.GetFloat(3);
        lootType.minQuantity           = row.GetUInt(4);
        lootType.maxQuantity           = row.GetUInt(5);
        m_LootTypeMap.emplace(row.GetUInt(0), lootType);
    }
    sLog.Cyan("    StaticDataMgr", "%lu loot groups and %lu loot types loaded in %.3fms.",
              m_LootGroupMap.size(), m_LootTypeMap.size(), (GetTimeMSeconds() - startTime));

    startTime = GetTimeMSeconds();
    uint32 locationID = 0;
    ManagerDB::GetAgentData(*res);
    while (res->GetRow(row)) {
        //SELECT agentID, corporationID, locationID FROM agtAgents
        locationID = row.GetUInt(2);
        if (IsStationID(locationID)) {
            locationID = GetStationSystem(locationID);
        }
        if (!IsSolarSystemID(locationID)) {
            _log(DATA__MESSAGE, "Failed to query info:  locationID %u is neither station nor system.", locationID);
            continue;
        }
        m_agentCorp[row.GetUInt(0)] = row.GetUInt(1);
        m_agentSystem[row.GetUInt(0)] = locationID;
    }
    sLog.Cyan("    StaticDataMgr", "%lu Agent Data Sets loaded in %.3fms.", m_agentCorp.size() + m_agentSystem.size(), (GetTimeMSeconds() - startTime));

    //cleanup
    SafeDelete(res);
    SafeDelete(res2);

    sLog.Cyan("    StaticDataMgr", "Static Data loaded in %.3fms.", (GetTimeMSeconds() - beginTime));
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

void StaticDataMgr::GetCategory(uint8 catID, Inv::CatData& into)
{
    std::unordered_map<uint16, Inv::CatData>::const_iterator itr = m_catData.find(catID);
    if (itr != m_catData.end())
        into = itr->second;
}

const char* StaticDataMgr::GetCategoryName(uint8 catID)
{
    std::unordered_map<uint16, Inv::CatData>::const_iterator itr = m_catData.find(catID);
    if (itr != m_catData.end())
        return itr->second.name.c_str();

    _log(DATA__ERROR, "GetCategoryName() - Category %u not found in map", catID);
    return "None";
}

void StaticDataMgr::GetGroup(uint16 grpID, Inv::GrpData& into)
{
    std::unordered_map<uint16, Inv::GrpData>::const_iterator itr = m_grpData.find(grpID);
    if (itr != m_grpData.end())
        into = itr->second;
}

const char* StaticDataMgr::GetGroupName(uint16 grpID)
{
    std::unordered_map<uint16, Inv::GrpData>::const_iterator itr = m_grpData.find(grpID);
    if (itr != m_grpData.end())
        return itr->second.name.c_str();

    _log(DATA__ERROR, "GetGroupName() - Group %u not found in map", grpID);
    return "None";
}

uint8 StaticDataMgr::GetMetaLevel(uint16 typeID) {
    std::unordered_map<uint16, Inv::TypeData>::const_iterator itr = m_typeData.find(typeID);
    if (itr != m_typeData.end())
        return itr->second.metaLvl;

    return 0;
}

void StaticDataMgr::GetType(uint16 typeID, Inv::TypeData& into) {
    std::unordered_map<uint16, Inv::TypeData>::const_iterator itr = m_typeData.find(typeID);
    if (itr != m_typeData.end())
        into = itr->second;
}

const char* StaticDataMgr::GetTypeName(uint16 typeID) {
    std::unordered_map<uint16, Inv::TypeData>::const_iterator itr = m_typeData.find(typeID);
    if (itr != m_typeData.end())
        return itr->second.name.c_str();

    _log(DATA__ERROR, "GetGroupName() - Group %u not found in map", typeID);
    return "None";
}

void StaticDataMgr::GetTypes(std::unordered_map< uint16, Inv::TypeData >& into) {
    into = m_typeData;
}

const char* StaticDataMgr::GetAttrName(uint16 attrID) {
    std::unordered_map<uint16, Inv::AttrTypeData>::const_iterator itr = m_attrTypeData.find(attrID);
    if (itr != m_attrTypeData.end())
        return itr->second.attributeName.c_str();
        //return itr->second.displayName.c_str();

    _log(DATA__ERROR, "GetAttrName() - Attribute %u not found in map", attrID);
    return "None";
}

uint32 StaticDataMgr::GetAgentCorpID(uint32 agentID) {
    std::unordered_map<uint32, uint32>::iterator itr = m_agentCorp.find(agentID);
    if (itr != m_agentCorp.end())
        return itr->second;
    return 0;
}

PyInt* StaticDataMgr::GetAgentSystemID(int32 agentID) {
    std::unordered_map<uint32, uint32>::iterator itr = m_agentSystem.find(agentID);
    if (itr != m_agentSystem.end())
        return new PyInt(itr->second);

    _log(DATA__WARNING, "Failed to query system info for agent %u: Agent not found.", agentID);
    return PyStatic.NewZero()->AsInt();
}

void StaticDataMgr::GetSalvage(uint32 factionID, std::vector<uint32> &into) {
    auto range = m_salvageMap.equal_range(factionID);
    uint16 count = std::distance(range.first, range.second);
    if (count == 0)
        return;
    into.reserve(into.size() + count);
    for (auto &it = range.first; it != range.second; ++it)
        into.push_back(it->second);
}

// new loot system part
void StaticDataMgr::LoadSalvageTables() {
    m_FactionToSalvageMap.clear();

    DBQueryResult* res = new DBQueryResult();
    // Query your legacy salvage data, utilizing techLvl to scale the relative weight
    //auto res = db.Query("SELECT factionID, techLvl, itemID FROM facSalvage;");

    DBResultRow row;
    while (res->GetRow(row)) {
        uint32 factionID = row.GetUInt(0);
        uint8 techLvl    = row.GetUInt8(1);

        LootItem item;
        item.typeID = row.GetUInt16(2);
        item.minQuantity = 1;
        item.maxQuantity = 1;

        // Define discrete distribution odds natively based on SDE Tech Tiering
        item.weight = (techLvl == 2) ? 50 : 1000;

        // Emplace item configurations safely into the faction's active pool cache
        // Assuming a standard single salvage pool model per faction family
        if (m_FactionToSalvageMap[factionID].empty()) {
            LootPool newPool;
            newPool.poolType = "SALVAGE";
            m_FactionToSalvageMap[factionID].push_back(newPool);
        }
        m_FactionToSalvageMap[factionID][0].items.push_back(item);
    }
}

bool StaticDataMgr::GetRoidDist(const char* secClass, std::unordered_multimap<float, uint16>& into) {
    auto range = m_oreBySecClass.equal_range(secClass);
    uint16 count = std::distance(range.first, range.second);
    if (count == 0)
        return false;
    into.reserve(into.size() + count);
    for (auto it = range.first; it != range.second; ++it) {
        _log(MINING__INFO, "GetRoidDist - adding %u with chance %.3f", it->second.typeID, it->second.chance);
        into.emplace(it->second.chance, it->second.typeID);
    }

    return !into.empty();
}

void StaticDataMgr::GetDgmTypeAttrVec(uint16 typeID, std::vector< Inv::DmgTypeAttribute >& into) {
    auto range = m_typeAttrMap.equal_range(typeID);
    uint16 count = std::distance(range.first, range.second);
    if (count == 0)
        return;
    into.reserve(into.size() + count);
    for (auto it = range.first; it != range.second; ++it)
        into.push_back(it->second);
}

bool StaticDataMgr::IsSkillTypeID(uint16 typeID) {
    return (m_skills.find(typeID) != m_skills.end());
}

const char* StaticDataMgr::GetSkillName(uint16 skillID) {
    std::unordered_map<uint16, std::string>::iterator itr = m_skills.find(skillID);
    if (itr != m_skills.end()) {
        return itr->second.c_str();
    }

    _log(DATA__MESSAGE, "Failed to query name for skill %u: Skill not found.", skillID);
    return nullptr;
}

void StaticDataMgr::GetComponentData(std::map< uint16, Market::matlData >& into) {
    for (auto &cur : m_components) {
        Market::matlData data   = Market::matlData();
        data.price              = 0.0f;
        data.typeID             = cur.first;
        data.name               = cur.second;
        into[cur.first]         = data;
    }
}

void StaticDataMgr::GetMineralData(std::map< uint16, Market::matlData >& into) {
    for (auto &cur : m_minerals) {
        Market::matlData data   = Market::matlData();
        data.price              = 0.0f;
        data.typeID             = cur.first;
        data.name               = cur.second;
        into[cur.first]         = data;
    }
}

void StaticDataMgr::GetCompoundData(std::map< uint16, Market::matlData >& into) {
    for (auto &cur : m_compounds) {
        Market::matlData data   = Market::matlData();
        data.price              = 0.0f;
        data.typeID             = cur.first;
        data.name               = cur.second;
        into[cur.first]         = data;
    }
}

void StaticDataMgr::GetSalvageData(std::map< uint16, Market::matlData >& into) {
    for (auto &cur : m_salvage) {
        Market::matlData data   = Market::matlData();
        data.price              = 0.0f;
        data.typeID             = cur.first;
        data.name               = cur.second;
        into[cur.first]         = data;
    }
}

void StaticDataMgr::GetPIResourceData(std::map< uint16, Market::matlData >& into) {
    for (auto &cur : m_resources) {
        Market::matlData data   = Market::matlData();
        data.price              = 0.0f;
        data.typeID             = cur.first;
        data.name               = cur.second;
        into[cur.first]         = data;
    }
}

void StaticDataMgr::GetPICommodityData(std::map< uint16, Market::matlData >& into) {
    for (auto &cur : m_commodities) {
        Market::matlData data   = Market::matlData();
        data.price              = 0.0f;
        data.typeID             = cur.first;
        data.name               = cur.second;
        into[cur.first]         = data;
    }
}

void StaticDataMgr::GetMiscCommodityData(std::map< uint16, Market::matlData >& into) {
    for (auto &cur : m_miscCommodities) {
        Market::matlData data   = Market::matlData();
        data.price              = 0.0f;
        data.typeID             = cur.first;
        data.name               = cur.second;
        into[cur.first]         = data;
    }
}

void StaticDataMgr::GetMoonResouces(std::map<uint16, uint8>& data) {
    // make copy
    for (auto &cur : m_moonGoo)
        data.emplace(cur.first, cur.second);
}

void StaticDataMgr::GetAncestryBonuses(uint8 ancestryID, Char::AttrData& into) {
    std::unordered_map<uint8, Char::AttrData>::iterator itr = m_ancestryBonuses.find(ancestryID);
    if (itr != m_ancestryBonuses.end())
        into = itr->second;
}

void StaticDataMgr::GetBloodlineBonuses(uint8 bloodlineID, Char::AttrData& into) {
    std::unordered_map<uint8, Char::AttrData>::iterator itr = m_bloodlineBonuses.find(bloodlineID);
    if (itr != m_bloodlineBonuses.end())
        into = itr->second;
}

uint16 StaticDataMgr::GetRandRatType(uint8 sClass, uint16 groupID) {
    if (groupID == 0)
        return 0;
    std::vector< uint16 > typeVec;
    auto range = m_npcTypes.equal_range(sClass);
    for (auto it = range.first; it != range.second; ++it) {
        for (auto &cur : it->second)
            if (cur.first == groupID) {
                for (auto &tCur : cur.second)
                    typeVec.push_back(tCur);
                break;
            }
    }
    if (!typeVec.empty())
        return typeVec.at(MakeRandomUInt(0, typeVec.size() - 1));

    _log(DATA__WARNING, "Failed to get random rat for sClass %u and groupID %u", sClass, groupID);
    return 0;
}

bool StaticDataMgr::GetNPCTypes(uint16 groupID, std::vector< uint16 >& typeVec) {
    /*  this is now invalid.....
     auto range = m_npcTypes.equal_range(groupID);
    for (auto it = range.first; it != range.second; ++it)
        typeVec.push_back(it->second);

    return !typeVec.empty();
    */
    return false;
}

bool StaticDataMgr::GetNPCGroups(uint32 factionID, std::map< uint8, uint16 >& groupMap) {
    auto range = m_npcGroups.equal_range(factionID);
    for (auto it = range.first; it != range.second; ++it)
        groupMap[it->second.shipClass] = it->second.groupID;

    return !groupMap.empty();
}

//TODO:  this is getting all levels for the class...we only need one level, but level isnt calculated yet
bool StaticDataMgr::GetNPCClasses(uint8 sClass, std::vector< RatSpawnClass >& classVec) {
    auto range = m_npcClasses.equal_range(sClass);
    uint16 count = std::distance(range.first, range.second);
    if (count == 0)
        return false;
    classVec.reserve(classVec.size() + count);
    for (auto it = range.first; it != range.second; ++it) {
        RatSpawnClass spawnClass   = RatSpawnClass();
            spawnClass.type        = it->second.type;
            spawnClass.sub         = it->second.sub;
            spawnClass.f           = it->second.f;
            spawnClass.af          = it->second.af;
            spawnClass.d           = it->second.d;
            spawnClass.c           = it->second.c;
            spawnClass.ac          = it->second.ac;
            spawnClass.bc          = it->second.bc;
            spawnClass.bs          = it->second.bs;
            spawnClass.h           = it->second.h;
            spawnClass.o           = it->second.o;
            spawnClass.cf          = it->second.cf;
            spawnClass.cd          = it->second.cd;
            spawnClass.cc          = it->second.cc;
            spawnClass.cbc         = it->second.cbc;
            spawnClass.cbs         = it->second.cbs;
            spawnClass.desc        = it->second.desc;
        classVec.push_back(spawnClass);
    }

    return !classVec.empty();
}

uint32 StaticDataMgr::GetWreckID(uint32 typeID) {
    std::map<uint32, uint32>::const_iterator itr = m_WrecksToTypesMap.find(typeID);
    if (itr != m_WrecksToTypesMap.end())
        return itr->second;
    return 0;
}

// new loot system part
void StaticDataMgr::LoadLoot() {
    m_ClassToProfileMap.clear();

    DBQueryResult* res = new DBQueryResult();

    /*
    // Query assignments utilizing our synchronized shipClass profile ids
    auto res = db.Query("SELECT p.profile_id, lpa.pool_id, lp.pool_type, lp.base_drop_chance, lp.selection_type " \
    "FROM dgm_npc_loot_profile p " \
    "JOIN dgm_loot_pool_assignment lpa ON p.profile_id = lpa.profile_id " \
    "JOIN dgm_loot_pool lp ON lpa.pool_id = lp.pool_id;");
*/
    DBResultRow row;
    uint32 factionID = 0;
    while (res->GetRow(row)) {
        uint32 shipClassID = row.GetUInt(0);

        LootPool pool;
        pool.poolID = row.GetUInt(1);
        pool.poolType = row.GetText(2);
        pool.calculatedDropChance = row.GetFloat(3);
        pool.rollAll = (row.GetText(4) == "ROLL_ALL");

        // Populate the specific item contents array
        DBQueryResult* res2 = new DBQueryResult();
        std::string sql = "SELECT type_id, min_quantity, max_quantity, selection_weight FROM dgm_loot_pool_item WHERE pool_id = " + std::to_string(pool.poolID);
  //      auto res = db.Query(sql);

        DBResultRow row2;
        while (res2->GetRow(row)) {
            uint16 itemTypeID = row2.GetUInt(0);
            uint8 meta = GetMetaLevel(itemTypeID); // Quick lookup against attribute 633

            LootItem item;
            item.typeID = itemTypeID;
            item.minQuantity = row2.GetUInt16(1);
            item.maxQuantity = row2.GetUInt16(2);
            item.weight = row2.GetUInt(3);
            pool.items.push_back(item);
        }

        // Your single boot parser now handles every drop category across the galaxy
        if (pool.poolType == "SALVAGE") {
            m_FactionToSalvageMap[factionID].push_back(pool);
        } else {
            m_ClassToProfileMap[shipClassID].assignedPools.push_back(pool);
        }

    }
    _log(LOOT__INFO, "Loot Engine V2 Synchronized. Loaded %lu Class profiles.", m_ClassToProfileMap.size());
}

std::vector<LootPool> StaticDataMgr::FetchPoolsForGroup(uint32 groupID, bool isAdvanced, bool isCommander) {
    std::vector<LootPool> pools;

    DBQueryResult* res = new DBQueryResult();
    // Direct SQL lookup query run at boot time
    // We fetch pool headers mapped to our profile structure
    std::string sql = "SELECT lp.pool_id, lp.pool_name, lp.pool_type, lp.base_drop_chance, lp.selection_type "
    "FROM dgm_npc_loot_profile p "
    "JOIN dgm_loot_pool_assignment lpa ON p.profile_id = lpa.profile_id "
    "JOIN dgm_loot_pool lp ON lpa.pool_id = lp.pool_id "
    "WHERE p.source_group_id = " + std::to_string(groupID);

   // auto res = db.Query(sql);
    DBResultRow row;
    while (res->GetRow(row)) {
        LootPool pool;
        pool.poolID = row.GetUInt(0);
        pool.poolType = row.GetText(1);
        pool.calculatedDropChance = row.GetFloat(2);
        pool.rollAll = (row.GetText(3) == "ROLL_ALL");

        // TIER MODIFICATION ARCHITECTURE:
        if (isCommander) {
            pool.calculatedDropChance *= 2.0f; // Commanders double their effective drops
        } else if (isAdvanced) {
            pool.calculatedDropChance *= 1.5f; // Advanced ships get a 50% baseline drop rate buff
        }

        // Hydrate items for this pool from dgm_loot_pool_item
        DBQueryResult* res2 = new DBQueryResult();
        std::string sql = "SELECT type_id, min_quantity, max_quantity, selection_weight FROM dgm_loot_pool_item WHERE pool_id = " + std::to_string(pool.poolID);
    //    auto res = db.Query(sql);

        DBResultRow row2;
        while (res2->GetRow(row)) {
            uint16 itemTypeID = row2.GetUInt(0);
            uint8 meta = GetMetaLevel(itemTypeID); // Quick lookup against attribute 633

            // Skip rare modules if this profile is for a standard T1 rat
            if (!isAdvanced && !isCommander && meta >= 4)
                continue;

            LootItem item;
            item.typeID = itemTypeID;
            item.minQuantity = row2.GetUInt(1);
            item.maxQuantity = row2.GetUInt(2);
            item.weight = row2.GetUInt(3);
            pool.items.push_back(item);
        }

        pools.push_back(pool);
    }
    return pools;
}

void StaticDataMgr::GetLootFinal(float trueSec, uint32 shipClassID, std::vector<LootList>& lootList) {
    auto it = m_ClassToProfileMap.find(shipClassID);
    if (it == m_ClassToProfileMap.end())
        return;

    const LootProfile& profile = it->second;

    for (const auto& pool : profile.assignedPools) {
        // Execute absolute gate check
        if (MakeRandomFloat() > pool.calculatedDropChance)
            continue;

        std::vector<double> weights;
        for (const auto& item : pool.items) {
            // Apply security location scalars natively
            weights.push_back(item.weight * (1.0f + (std::max(0.0f, 1.0f - trueSec) * 0.5f)));
        }

        if (pool.items.empty())
            continue;

        if (pool.rollAll) {
            for (const auto& item : pool.items) {
                LootList list;
                list.typeID = (uint16)item.typeID;
                list.minDrop = item.minQuantity;
                list.maxDrop = item.maxQuantity;
                lootList.push_back(list);
            }
        } else {
            std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
            //size_t idx = dist(gen);
            //const auto& selected = pool.items[idx];
            //lootList.push_back({ (uint16)selected.typeID, selected.minQuantity, selected.maxQuantity });
        }
    }
}


void StaticDataMgr::GetLoot(float secValue, uint32 groupID, std::vector<LootList>& lootList) {
    // called by SE::DropLoot()
    double profileStartTime = GetTimeUSeconds();

    float randChance(0.0f);
    uint8 metaLevel = 0;
    uint8 secModX10(static_cast<uint8>(secValue * 10));   //[1, 20]
    float secMod(secValue / 10); //[0.01, 0.2]
    // modified chances for metaLevel checks - lower security = higher chance
    //  meta level 0                      0.50 - 0.70
    float one(0.5f + secMod);           //0.51 - 0.70
    float two(0.3f + secMod);           //0.31 - 0.50
    float three(0.1f + secMod);         //0.11 - 0.30
    float four(0.0f + secMod);          //0.01 - 0.20

    std::vector<LootType> lootGrpVec;

    // Finds a range containing all elements whose key is k.
    // pair<iterator, iterator> equal_range(const key_type& k)
    auto range = m_LootGroupMap.equal_range(groupID);
    uint16 count = std::distance(range.first, range.second);
    if (count == 0)
        return;
    lootGrpVec.reserve(lootGrpVec.size() + count);

    for (auto it = range.first; it != range.second; ++it) {
        _log(LOOT__INFO, "checking GroupID %u with %.2f chance", it->second.lootGroupID, it->second.dropChance);
        // make lootMap of lootGroupID's
        if (MakeRandomFloat() < it->second.dropChance) {
            randChance = MakeRandomFloat();  // chance in 1.0 - chance in -0.9
            if (randChance < four) {            //01 - 20
                metaLevel = 4;
            } else if (randChance < three) {    //10 - 10
                metaLevel = 3;
            } else if (randChance < two) {      //20 - 20
                metaLevel = 2;
            } else if (randChance < one) {      //20 - 20
                metaLevel = 1;
            } else {
                metaLevel = 0;                  //49 - 30
            }

            /*need to figure out how to get faction loot for faction wrecks
             * test groupID for factions?
             *    elif meta_level == 7:
             *        drop_chance = 0.15   # Faction stuff = 15%
             *    elif meta_level == 8:
             *        drop_chance = 0.15   # Faction projectiles = 15%
             *    elif meta_level == 9:
             *        drop_chance = 0.15   # Faction SB's and Missile launchers
             */

            auto range2 = m_LootTypeMap.equal_range(it->second.lootGroupID);
            for (auto it2 = range2.first; it2 != range2.second; ++it2) {
                if (it2->second.metaLevel == metaLevel) {
                    _log(LOOT__INFO, "checking lootType %u, metaLevel %u,  with chance of %.2f", \
                            it2->second.typeID, metaLevel, it2->second.dropChance + secMod);
                    if (MakeRandomFloat() < (it2->second.dropChance + secMod)) {
                        lootGrpVec.push_back(it2->second);
                    }
                }
            }
        }
    }

    if (!lootGrpVec.empty()) {
        for (auto &cur : lootGrpVec) {
            LootList loot_list = LootList();
            loot_list.typeID        = cur.typeID;
            loot_list.minDrop       = cur.minQuantity;
            loot_list.maxDrop       = cur.maxQuantity;
            lootList.push_back(loot_list);
        }
        /*
        if ((groupID == EVEDB::invGroups::Asteroid_Angel_Cartel_Hauler)
        or (groupID == EVEDB::invGroups::Asteroid_Blood_Raiders_Hauler)
        or (groupID == EVEDB::invGroups::Asteroid_Guristas_Hauler)
        or (groupID == EVEDB::invGroups::Asteroid_Sansha_s_Nation_Hauler)
        or (groupID == EVEDB::invGroups::Asteroid_Serpentis_Hauler)
        or (groupID == EVEDB::invGroups::Asteroid_Rogue_Drone_Hauler)) {
            // get all items from list for hauler spawns
            for (auto &cur : lootGrpVec) {
                LootList loot_list = LootList();
                loot_list.typeID        = cur.typeID;
                loot_list.minDrop       = cur.minQuantity;
                loot_list.maxDrop       = cur.maxQuantity;
                lootList.push_back(loot_list);
            }
        } else {
            // get one random item from list of possibles for normal spawns
            LootList loot_list = LootList();
            uint16 i = MakeRandomUInt(0, lootGrpVec.size() - 1);
            loot_list.typeID        = lootGrpVec[i].typeID;
            loot_list.minDrop       = lootGrpVec[i].minQuantity;
            loot_list.maxDrop       = lootGrpVec[i].maxQuantity;
            lootList.push_back(loot_list);
            _log(LOOT__INFO, "added %u to basic lootList of %lu possible", lootGrpVec[i].typeID, lootGrpVec.size());
        }
        */
    }

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::loot, GetTimeUSeconds() - profileStartTime);
}

void StaticDataMgr::ProcessLootModifiers(uint8 classID, LootPool& pool) {
    switch (classID) {
        // High tier targets scale drop percentages dramatically
        case Rat::ShipClass::Asteroid_Officer:
        case Rat::ShipClass::Capital_Titan:
            pool.calculatedDropChance *= 3.0f;
            break;

        case Rat::ShipClass::Asteroid_CommanderBattleship:
        case Rat::ShipClass::Deadspace_CommanderBattleship:
            pool.calculatedDropChance *= 2.0f;
            break;

            // Wormhole Sleepers automatically trigger custom blue-loot salvage tracking
        case Rat::ShipClass::Sleeper_Battleship:
            InjectSleeperSalvage(pool);
            break;

        default:
            break;
    }
}

// part of new loot
void StaticDataMgr::InjectSleeperSalvage(LootPool& pool) {
    // Since the Python script already assigned generic/drone component pools,
    // we can explicitly override the drop behavior of this pool instance
    // to transform it into a dedicated, guaranteed Sleeper Salvage table.

    pool.calculatedDropChance = 1.00000f; // Ensure a salvage roll vector always exists

    // Optional: If you want to force specific rare salvage materials
    // (like Melted Nanoribbons) to ONLY drop in high-tier wormhole space,
    // you can manipulate the active memory pool parameters right here at boot:
    for (auto& item : pool.items) {
        if (item.typeID == 30259) //EVE_SDE::Items::MeltedNanoribbon)
            item.weight = 500; // Boost or throttle presence based on your design goals
    }
}

void StaticDataMgr::GetBpTypeData(uint16 typeID, EvERam::bpTypeData& tData)
{
    std::unordered_map<uint16, EvERam::bpTypeData>::iterator itr = m_bpTypeData.find(typeID);
    if (itr != m_bpTypeData.end()) {
        tData = itr->second;
    } else {
        _log(DATA__MESSAGE, "Failed to query info for bpType %u: Type not found.", typeID);
    }
}

bool StaticDataMgr::GetBpDataForItem(uint16 typeID, EvERam::bpTypeData& tData)
{
    std::map<uint16, EvERam::bpTypeData>::iterator itr = m_bpProductData.find(typeID);
    if (itr != m_bpProductData.end()) {
        tData = itr->second;
        return true;
    }
    return false;
}

bool StaticDataMgr::IsPublished(uint16 typeID) {
    std::unordered_map<uint16, Inv::TypeData>::iterator itr = m_typeData.find(typeID);
    if (itr != m_typeData.end())
        return itr->second.published;
    return false;
}

bool StaticDataMgr::IsRecyclable(uint16 typeID)
{
    std::unordered_map<uint16, Inv::TypeData>::iterator itr = m_typeData.find(typeID);
    if (itr != m_typeData.end())
        return itr->second.isRecyclable;
    return false;
}

bool StaticDataMgr::IsRefinable(uint16 typeID)
{
    std::unordered_map<uint16, Inv::TypeData>::iterator itr = m_typeData.find(typeID);
    if (itr != m_typeData.end())
        return itr->second.isRefinable;
    return false;
}

void StaticDataMgr::GetRamReturns(uint16 typeID, int8 activityID, std::vector< EvERam::RequiredItem >& ramReqs)
{
    auto range = m_ramReq.equal_range(typeID);
    for (auto it = range.first; it != range.second; ++it)
        if ((it->second.activityID == activityID) and (it->second.extra) and !(IsSkillTypeID(it->second.requiredTypeID))) {
            EvERam::RequiredItem data   = EvERam::RequiredItem();
            data.typeID                 = it->second.requiredTypeID;
            data.quantity               = it->second.quantity;
            data.damagePerJob           = it->second.damagePerJob;
            data.isSkill                = IsSkillTypeID(it->second.requiredTypeID);
            data.extra                  = it->second.extra;
            ramReqs.push_back(data);
        }
}

void StaticDataMgr::GetRamMaterials(uint16 typeID, std::vector< EvERam::RamMaterials >& ramMatls)
{
    auto range = m_ramMatl.equal_range(typeID);
    for (auto cur = range.first; cur != range.second; ++cur)
        ramMatls.push_back(cur->second);
}

void StaticDataMgr::GetRamRequirements(uint16 typeID, std::vector< EvERam::RamRequirements >& ramReqs)
{
    auto range = m_ramReq.equal_range(typeID);
    for (auto cur = range.first; cur != range.second; ++cur)
        ramReqs.push_back(cur->second);
}

void StaticDataMgr::GetRamRequiredItems(const uint32 typeID, const int8 activity, std::vector< EvERam::RequiredItem >& into)
{
    if (activity == EvERam::Activity::Manufacturing) {
        std::unordered_map<uint16, EvERam::bpTypeData>::iterator itr = m_bpTypeData.find(typeID);
        if (itr != m_bpTypeData.end()) {
            auto range = m_ramMatl.equal_range(itr->second.productTypeID);
            for (auto it = range.first; it != range.second; ++it) {
                EvERam::RequiredItem data = EvERam::RequiredItem();
                data.typeID             = it->second.materialTypeID;
                data.quantity           = it->second.quantity;
                into.push_back(data);
            }
        }
    }

    auto range = m_ramReq.equal_range(typeID);
    for (auto it = range.first; it != range.second; ++it)
        if (it->second.activityID == activity) {
            EvERam::RequiredItem data   = EvERam::RequiredItem();
            data.typeID                 = it->second.requiredTypeID;
            data.quantity               = it->second.quantity;
            data.damagePerJob           = it->second.damagePerJob;
            data.isSkill                = IsSkillTypeID(it->second.requiredTypeID);
            data.extra                  = it->second.extra;
            into.push_back(data);
        }
}

PyRep* StaticDataMgr::GetStationCount()
{
    PyList* list(new PyList());
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

    _log(DATA__MESSAGE, "Failed to query station count for system %u: System not found.", systemID);
    return 0;
}

bool StaticDataMgr::GetStationList(uint32 systemID, std::vector< uint32 >& data)
{
    std::unordered_map<uint32, std::vector<uint32>>::iterator itr = m_stationList.find(systemID);
    if (itr != m_stationList.end()) {
        data = itr->second;
        return true;
    }
    return false;
}

uint32 StaticDataMgr::GetStationRegion(uint32 stationID) {
    std::unordered_map<uint32, uint32>::iterator itr = m_stationRegion.find(stationID);
    if (itr != m_stationRegion.end())
        return itr->second;

    _log(DATA__MESSAGE, "Failed to get regionID for station %u.", stationID);
    return 0;
}

uint32 StaticDataMgr::GetStationConstellation(uint32 stationID) {
    std::unordered_map<uint32, uint32>::iterator itr = m_stationConst.find(stationID);
    if (itr != m_stationConst.end())
        return itr->second;

    _log(DATA__MESSAGE, "Failed to get constellationID for station %u.", stationID);
    return 0;
}

uint32 StaticDataMgr::GetStationSystem(uint32 stationID) {
    std::unordered_map<uint32, uint32>::iterator itr = m_stationSystem.find(stationID);
    if (itr != m_stationSystem.end())
        return itr->second;

    _log(DATA__MESSAGE, "Failed to get systemID for station %u.", stationID);
    return 0;
}

void StaticDataMgr::GetConstellationSystems(uint32 constellationID, std::vector<uint32>& into) {
    auto range = m_constSystems.equal_range(constellationID);
    uint16 count = std::distance(range.first, range.second);
    if (count == 0)
        return;
    into.reserve(into.size() + count);
    for (auto &it = range.first; it != range.second; ++it)
        into.push_back(it->second);
}

void StaticDataMgr::GetRegionSystems(uint32 regionID, std::vector<uint32>& into) {
    auto range = m_regionSystems.equal_range(regionID);
    uint16 count = std::distance(range.first, range.second);
    if (count == 0)
        return;
    into.reserve(into.size() + count);
    for (auto &it = range.first; it != range.second; ++it)
        into.push_back(it->second);
}

uint8 StaticDataMgr::GetWHSystemClass(uint32 systemID)
{
    std::unordered_map<uint32, uint8>::iterator itr = m_whRegions.find(systemID);
    if (itr != m_whRegions.end())
        return itr->second;

    SystemManager* pSysMgr(sEntityMgr.FindOrBootSystem(systemID));
    if (pSysMgr == nullptr)
        return 0;

    itr = m_whRegions.find(pSysMgr->GetRegionID());
    if (itr != m_whRegions.end())
        return itr->second;

    // dont have data for systemID nor regionID...throw error and ?something else?
    _log(DATA__MESSAGE, "Failed to query WH Class for systemID %u: System not found.", systemID);
    if (IsKSpaceID(systemID))
        return 0;
    if (IsWSpaceID(systemID))
        return 0;

    return 0;
}

bool StaticDataMgr::GetStaticInfo(uint32 itemID, StaticData& data)
{
    std::unordered_map<uint32, StaticData>::iterator itr = m_staticData.find(itemID);
    if (itr != m_staticData.end()) {
        data = itr->second;
        return true;
    }

    _log(DATA__MESSAGE, "Failed to query info for static item %u: Item not found.", itemID);
    return false;
}

uint16 StaticDataMgr::GetStaticType(uint32 itemID)
{
    std::unordered_map<uint32, StaticData>::iterator itr = m_staticData.find(itemID);
    if (itr != m_staticData.end())
        return itr->second.typeID;
    return 0;
}

uint32 StaticDataMgr::GetRegionFaction(uint32 regionID)
{
    std::unordered_map<uint32, uint32>::iterator itr = m_regions.find(regionID);
    if (itr != m_regions.end())
        return itr->second;

    _log(DATA__MESSAGE, "Failed to query faction for region %u: region not found.", regionID);
    return 0;
}

uint32 StaticDataMgr::GetRegionRatFaction(uint32 regionID)
{
    std::unordered_map<uint32, uint32>::iterator itr = m_ratRegions.find(regionID);
    if (itr != m_ratRegions.end())
        return itr->second;

    _log(DATA__MESSAGE, "Failed to query rat faction for region %u: region not found.", regionID);
    return 0;
}

std::string StaticDataMgr::GetCorpName(uint32 corpID)
{
    std::unordered_map<uint32, std::string>::iterator itr = m_corpName.find(corpID);
    if (itr != m_corpName.end())
        return itr->second;

    _log(DATA__ERROR, "Name not found for corp %u", corpID);
    return "No Name";
}

uint32 StaticDataMgr::GetCorpFaction(uint32 corpID)
{
    std::unordered_map<uint32, uint32>::iterator itr = m_corpFaction.find(corpID);
    if (itr != m_corpFaction.end())
        return itr->second;

    if (IsNPCCorp(corpID))
        _log(DATA__ERROR, "Faction not found for NPC corp %s", GetCorpName(corpID).c_str());

    return 0;
}

std::string StaticDataMgr::GetFactionName(uint32 factionID)
{
    std::unordered_map<uint32, std::string>::const_iterator itr = m_factionName.find(factionID);
    if (itr != m_factionName.end())
        return itr->second;

    return "Undefined";
}

bool StaticDataMgr::GetSystemData(uint32 locationID, SystemData& data)
{
    if (IsStation(locationID)) {
        locationID = GetStationSystem(locationID);
    }
    if (!IsSolarSystem(locationID)) {
        _log(DATA__MESSAGE, "Failed to query info:  locationID %u is neither station nor system.", locationID);
        return false;
    }

    std::unordered_map<uint32, SolarSystemData>::const_iterator itr = m_solSysData.find(locationID);
    if (itr != m_solSysData.end()) {
        data.systemID = itr->second.systemID;
        data.constellationID = itr->second.constellationID;
        data.regionID = itr->second.regionID;
        data.factionID = itr->second.factionID;
        data.radius = itr->second.radius;
        data.security = itr->second.security;
        data.name = itr->second.name;
        data.securityClass = itr->second.securityClass;
        return true;
    }

    _log(DATA__MESSAGE, "Failed to query info for system %u: System not found.", locationID);
    return false;
}

uint32 StaticDataMgr::GetSystemConstellation(uint32 systemID) {
    SystemData data;
    if (GetSystemData(systemID, data))
        return data.constellationID;

    _log(DATA__MESSAGE, "Failed to get constellationID for system %u.", systemID);
    return 0;
}

uint32 StaticDataMgr::GetSystemRegion(uint32 systemID) {
    SystemData data;
    if (GetSystemData(systemID, data))
        return data.regionID;

    _log(DATA__MESSAGE, "Failed to get regionID for system %u.", systemID);
    return 0;
}

bool StaticDataMgr::GetSolarSystemData(uint32 locationID, SolarSystemData& data) {
    if (IsStation(locationID))
        locationID = GetStationSystem(locationID);

    if (!IsSolarSystem(locationID)) {
        _log(DATA__MESSAGE, "Failed to query info:  locationID %u is neither station nor system.", locationID);
        return false;
    }
    std::unordered_map<uint32, SolarSystemData>::const_iterator itr = m_solSysData.find(locationID);
    if (itr == m_solSysData.end())
        return false;
    data = itr->second;
    return true;
}

const char* StaticDataMgr::GetSystemName(uint32 locationID) {
    if (IsStation(locationID)) {
        locationID = GetStationSystem(locationID);
    }
    if (!IsSolarSystem(locationID)) {
        _log(DATA__MESSAGE, "Failed to query info:  locationID %u is neither station nor system.", locationID);
        return "Error";
    }

    std::unordered_map<uint32, SolarSystemData>::const_iterator itr = m_solSysData.find(locationID);
    if (itr != m_solSysData.end())
        return itr->second.name.c_str();

    _log(DATA__MESSAGE, "Failed to query info for system %u: System not found.", locationID);
    return "Invalid";
}

bool StaticDataMgr::IsSolarSystem(uint32 systemID/*0*/) {
    // if systemID has entry here, it is valid
    std::unordered_map<uint32, SolarSystemData>::const_iterator itr = m_solSysData.find(systemID);
    return (itr != m_solSysData.end());
}

bool StaticDataMgr::IsConSystem(uint32 systemID) {
    std::unordered_map<uint32, SolarSystemData>::const_iterator itr = m_solSysData.find(systemID);
    if (itr != m_solSysData.end())
        return itr->second.constellation;
    return false;
}

bool StaticDataMgr::IsCorridorSystem(uint32 systemID) {
    std::unordered_map<uint32, SolarSystemData>::const_iterator itr = m_solSysData.find(systemID);
    if (itr != m_solSysData.end())
        return itr->second.corridor;
    return false;
}

bool StaticDataMgr::IsFringeSystem(uint32 systemID) {
    std::unordered_map<uint32, SolarSystemData>::const_iterator itr = m_solSysData.find(systemID);
    if (itr != m_solSysData.end())
        return itr->second.fringe;
    return false;
}

bool StaticDataMgr::IsHubSystem(uint32 systemID) {
    std::unordered_map<uint32, SolarSystemData>::const_iterator itr = m_solSysData.find(systemID);
    if (itr != m_solSysData.end())
        return itr->second.hub;
    return false;
}

bool StaticDataMgr::IsRegionSystem(uint32 systemID) {
    std::unordered_map<uint32, SolarSystemData>::const_iterator itr = m_solSysData.find(systemID);
    if (itr != m_solSysData.end())
        return itr->second.region;
    return false;
}

bool StaticDataMgr::IsStation(uint32 stationID/*0*/) {
    // if stationID has entry here, it is valid
    std::unordered_map<uint32, uint32>::const_iterator itr = m_stationRegion.find(stationID);
    return (itr != m_stationRegion.end());
}

DBRowDescriptor* StaticDataMgr::CreateItemHeader() {
    // this is correct data for crucible.  dont alter
    PyList *keywords(new PyList());
        keywords->AddItem(new_tuple(new PyString("stacksize"), new PyToken("util.StackSize")));
        keywords->AddItem(new_tuple(new PyString("singleton"), new PyToken("util.Singleton")));
    DBRowDescriptor* header(new DBRowDescriptor(keywords));
        header->AddColumn("itemID",     DBTYPE_I8);     // int64
        header->AddColumn("typeID",     DBTYPE_I4);     // int32
        header->AddColumn("ownerID",    DBTYPE_I4);     // int32
        header->AddColumn("locationID", DBTYPE_I4);     // this should be I8 (according to packets)
        header->AddColumn("flagID",     DBTYPE_I2);     // int16
        header->AddColumn("quantity",   DBTYPE_I4);     // int32
        header->AddColumn("groupID",    DBTYPE_I4);     // int32
        header->AddColumn("categoryID", DBTYPE_I4);     // int32
        header->AddColumn("customInfo", DBTYPE_STR);
    return header;
}

/*
 * Formation Type IDs:
 * The engine natively parses structural integer IDs mapping to known layout algorithms:
 * Point (ID 1)
 * Sphere (ID 2)
 * Arrow (ID 3)
 * Wall (ID 4)
 * Plane (ID 5)
 * Custom (ID 6+)
 */
PyTuple* StaticDataMgr::CreateFormationTuple() {
    // added missing formations and move here.  -allan
    PyTuple* res = new PyTuple(4);

    // ============================================================================
    // 1. THE DIAMOND LAYOUT (formationID = 0)
    // ============================================================================
    Beyonce_Formation diamond;
    diamond.name = "Diamond";
    // Slot 1: Starboard / Right Wing Anchor
    diamond.pos1.x =  150.0;
    diamond.pos1.x =    0.0;
    diamond.pos1.x =    0.0;
    // Slot 2: Vanguard Nose Anchor (Leads ahead)
    diamond.pos2.x =    0.0;
    diamond.pos2.x =    0.0;
    diamond.pos2.x =  150.0;
    // Slot 3: Port / Left Wing Anchor
    diamond.pos3.x = -150.0;
    diamond.pos3.x =    0.0;
    diamond.pos3.x =    0.0;
    // Slot 4: Rear Guard Tail Anchor (Trails behind)
    diamond.pos4.x =    0.0;
    diamond.pos4.x =    0.0;
    diamond.pos4.x = -150.0;
    res->SetItem(0, diamond.Encode());

    // ============================================================================
    // 2. THE ARROW / V-ECHELON LAYOUT (formationID = 1)
    // ============================================================================
    Beyonce_Formation arrow;
    arrow.name = "Arrow";
    // Slot 1: Inner Starboard Echelon
    arrow.pos1.x =  100.0;
    arrow.pos1.x =    0.0;
    arrow.pos1.x =  -50.0;
    // Slot 2: Outer Starboard Echelon
    arrow.pos2.x =  200.0;
    arrow.pos2.x =    0.0;
    arrow.pos2.x = -100.0;
    // Slot 3: Inner Port Echelon
    arrow.pos3.x = -100.0;
    arrow.pos3.x =    0.0;
    arrow.pos3.x =  -50.0;
    // Slot 4: Outer Port Echelon
    arrow.pos4.x = -200.0;
    arrow.pos4.x =    0.0;
    arrow.pos4.x = -100.0;
    res->SetItem(1, arrow.Encode());

    // ============================================================================
    // 3. THE WALL LAYOUT (formationID = 2)
    // ============================================================================
    // Ships line up rigidly shoulder-to-shoulder perpendicular to the flight path.
    // Exceptional for showing broadsides or massive capital grid displays.  (is 60m enough?)
    Beyonce_Formation wall;
    wall.name = "Wall";
    // Slot 1: Tight Inner Starboard Step
    wall.pos1.x =   60.0;
    wall.pos1.x =    0.0;
    wall.pos1.x =    0.0;
    // Slot 2: Outer Starboard Wing Tip
    wall.pos2.x =  120.0;
    wall.pos2.x =    0.0;
    wall.pos2.x =    0.0;
    // Slot 3: Tight Inner Port Step
    wall.pos3.x =  -60.0;
    wall.pos3.x =    0.0;
    wall.pos3.x =    0.0;
    // Slot 4: Outer Port Wing Tip
    wall.pos4.x = -120.0;
    wall.pos4.x =    0.0;
    wall.pos4.x =    0.0;
    res->SetItem(2, wall.Encode());

    // ============================================================================
    // 4. THE LINE / COLUMN LAYOUT (formationID = 3)
    // ============================================================================
    // Ships align single-file rigidly behind the leader's exhaust trail.
    // Classic convoy/highway style layout spacing. (is 80m enough?)
    Beyonce_Formation line;
    line.name = "Line";
    // Slot 1: Immediately behind Leader
    line.pos1.x =    0.0;
    line.pos1.x =    0.0;
    line.pos1.x =  -80.0;
    // Slot 2: Second in Column tail
    line.pos2.x =    0.0;
    line.pos2.x =    0.0;
    line.pos2.x = -160.0;
    // Slot 3: Third in Column tail
    line.pos3.x =    0.0;
    line.pos3.x =    0.0;
    line.pos3.x = -240.0;
    // Slot 4: Rear Anchor point caboose
    line.pos4.x =    0.0;
    line.pos4.x =    0.0;
    line.pos4.x = -320.0;
    res->SetItem(3, line.Encode());

    /** @todo  add 'new' npc formations here and method to get formation by id */
    return res;
}

PyDict* StaticDataMgr::GetBPMatlData(uint16 typeID)
{
    auto itr = m_bpMatlData.find(typeID);
    if (itr != m_bpMatlData.end()) {
        PyIncRef(itr->second);
        if (is_log_enabled(MANUF__DEBUG))
            itr->second->Dump(MANUF__DEBUG, "    ");
        return itr->second;
    }
    return nullptr;
}

PyDict* StaticDataMgr::SetBPMatlType(int8 catID, uint16 typeID, uint16 prodID)
{
    PyList* matlListManuf(new PyList());
    PyList* skillListManuf(new PyList());
    PyList* extraListManuf(new PyList());
    PyList* matlListTE(new PyList());
    PyList* skillListTE(new PyList());
    PyList* matlListME(new PyList());
    PyList* skillListME(new PyList());
    PyList* matlListCopy(new PyList());
    PyList* skillListCopy(new PyList());
    PyList* matlListDup(new PyList());
    PyList* skillListDup(new PyList());
    PyList* extraListDup(new PyList());
    PyList* matlListRE(new PyList());
    PyList* skillListRE(new PyList());
    PyList* matlListInvent(new PyList());
    PyList* skillListInvent(new PyList());

    // NOTE: manuf is always populated for blueprints but not ancient relics
    if (catID == EVEDB::invCategories::Blueprint) {
        // ramMaterials is only for manufacturing the bp product
        std::vector<EvERam::RamMaterials> ramMatls;
        GetRamMaterials(prodID, ramMatls);
        for (auto &cur : ramMatls) {
            if (!IsPublished(cur.materialTypeID))
                continue;
            PyPackedRow* row = new PyPackedRow(m_bpMatlHeader);
                row->SetFieldC("quantity",        new PyInt(cur.quantity));
                row->SetFieldC("requiredTypeID",  new PyInt(cur.materialTypeID));
                row->SetFieldC("damagePerJob",    new PyFloat(1.0f));
            matlListManuf->AddItem(row);
        }
    }

    // booleans to only set items that are populated
    bool manuf = false, copy = false, invent = false, dup = false, me = false, re = false, te = false, tech = false;
    // the ramRequirements table holds ALL skill/item data for all aspects of RAM per BlueprintTypeID.
    std::vector<EvERam::RamRequirements> ramReqs;
    GetRamRequirements(typeID, ramReqs);
    GetRamRequirements(prodID, ramReqs);
    for (auto &cur : ramReqs) {
        if (!IsPublished(cur.requiredTypeID))
            continue;

        PyPackedRow* row = new PyPackedRow(m_bpMatlHeader);
            row->SetFieldC("quantity",        new PyInt(cur.quantity));
            row->SetFieldC("requiredTypeID",  new PyInt(cur.requiredTypeID));
            row->SetFieldC("damagePerJob",    new PyFloat(cur.damagePerJob));

        using namespace EvERam;
        switch(cur.activityID) {
            case Activity::Manufacturing: {         //1
                /** @todo  this needs work.  dunno how to remove 'extra' materials from this list */
                manuf = true;
                if (IsSkillTypeID(cur.requiredTypeID)) {
                    skillListManuf->AddItem(row);
                } else if (cur.extra) {
                    extraListManuf->AddItem(row);
                } else {
                    matlListManuf->AddItem(row);
                }
            } break;
            case Activity::ResearchTech: {          //2
                // not used.  not defined in client.  no data for this activity
            } break;
            case Activity::ResearchTime: {          //3
                te = true;
                if (IsSkillTypeID(cur.requiredTypeID)) {
                    skillListTE->AddItem(row);
                } else {
                    matlListTE->AddItem(row);
                }
            } break;
            case Activity::ResearchMaterial: {      //4
                me = true;
                if (IsSkillTypeID(cur.requiredTypeID)) {
                    skillListME->AddItem(row);
                } else {
                    matlListME->AddItem(row);
                }
            } break;
            case Activity::Copying: {               //5
                copy = true;
                if (IsSkillTypeID(cur.requiredTypeID)) {
                    skillListCopy->AddItem(row);
                } else {
                    matlListCopy->AddItem(row);
                }
            } break;
            case Activity::Duplicating: {           //6
                dup = true;
                if (IsSkillTypeID(cur.requiredTypeID)) {
                    skillListDup->AddItem(row);
                } else if (cur.extra) {
                    extraListDup->AddItem(row);
                } else {
                    matlListDup->AddItem(row);
                }
            } break;
            case Activity::ReverseEngineering: {    //7
                re = true;
                if (IsSkillTypeID(cur.requiredTypeID)) {
                    skillListRE->AddItem(row);
                } else {
                    matlListRE->AddItem(row);
                }
            } break;
            case Activity::Invention: {             //8
                invent = true;
                if (IsSkillTypeID(cur.requiredTypeID)) {
                    skillListInvent->AddItem(row);
                } else {
                    matlListInvent->AddItem(row);
                }
            } break;
        }
    }

    // build the response packet.  test for items populated above and create an ItemString in the dict for that item.
    // items not populated will not be shown in the BP info.
    DBQueryResult mtRes;
    PyRep* mtCRowSet(DBResultToCRowset(mtRes));
    PyDict* rsp(new PyDict());

    if (manuf) {        //activityManufacturing = 1
        PyDict* Manufacturing = new PyDict();
            Manufacturing->SetItemString("skills", skillListManuf);
            Manufacturing->SetItemString("rawMaterials", matlListManuf);
        CRowSet *rowset = new CRowSet(m_bpMatlHeader);
        PyList::const_iterator itr = extraListManuf->begin();
        for (; itr != extraListManuf->end();++itr) {
            PyPackedRow* from = (*itr)->AsPackedRow();
            PyPackedRow* into = rowset->NewRow();
            PyRep* f0 = from->GetField(0); PyIncRef(f0); into->SetField(0,f0);
            PyRep* f1 = from->GetField(1); PyIncRef(f1); into->SetField(1,f1);
            PyRep* f2 = from->GetField(2); PyIncRef(f2); into->SetField(2,f2);
        }
        Manufacturing->SetItemString("extras", rowset);     // have to build a crowset for this
        PyObject* valObj = new PyObject("util.KeyVal", Manufacturing);
        rsp->SetItem(new PyInt(1), valObj);
    } else {
        PyDecRef(skillListManuf);
        PyDecRef(matlListManuf);
    }
    if (tech) {        //activityResearchingTechnology = 2
        // not used.  not defined in client.  no data for this activity
    }
    if (te) {        //activityResearchingTimeProductivity = 3
        PyIncRef(mtCRowSet);
        PyDict* ResearchTime = new PyDict();
            ResearchTime->SetItemString("skills", skillListTE);
            ResearchTime->SetItemString("rawMaterials", matlListTE);
            ResearchTime->SetItemString("extras", mtCRowSet);
        PyObject* valObj = new PyObject("util.KeyVal", ResearchTime);
        rsp->SetItem(new PyInt(3), valObj);
    } else {
        PyDecRef(skillListTE);
        PyDecRef(matlListTE);
    }
    if (me) {        //activityResearchingMaterialProductivity = 4
        PyIncRef(mtCRowSet);
        PyDict* ResearchMaterial = new PyDict();
            ResearchMaterial->SetItemString("skills", skillListME);
            ResearchMaterial->SetItemString("rawMaterials", matlListME);
            ResearchMaterial->SetItemString("extras", mtCRowSet);
        PyObject* valObj = new PyObject("util.KeyVal", ResearchMaterial);
        rsp->SetItem(new PyInt(4), valObj);
    } else {
        PyDecRef(skillListME);
        PyDecRef(matlListME);
    }
    if (copy) {        //activityCopying = 5
        PyIncRef(mtCRowSet);
        PyDict* Copying = new PyDict();
            Copying->SetItemString("skills", skillListCopy);
            Copying->SetItemString("rawMaterials", matlListCopy);
            Copying->SetItemString("extras", mtCRowSet);
        PyObject* valObj = new PyObject("util.KeyVal", Copying);
        rsp->SetItem(new PyInt(5), valObj);
    } else {
        PyDecRef(skillListCopy);
        PyDecRef(matlListCopy);
    }
    if (dup) {       //activityDuplicating = 6
        // no longer used...updated to "copying" after RMR
        PyDict* Duplicating = new PyDict();
            Duplicating->SetItemString("skills", skillListDup);
            Duplicating->SetItemString("rawMaterials", matlListDup);
        CRowSet *rowset = new CRowSet(m_bpMatlHeader);
        PyList::const_iterator itr = extraListDup->begin();
        for (; itr != extraListDup->end();++itr) {
            PyPackedRow* from = (*itr)->AsPackedRow();
            PyPackedRow* into = rowset->NewRow();
            PyRep* f0 = from->GetField(0); PyIncRef(f0); into->SetField(0,f0);
            PyRep* f1 = from->GetField(1); PyIncRef(f1); into->SetField(1,f1);
            PyRep* f2 = from->GetField(2); PyIncRef(f2); into->SetField(2,f2);
        }
        Duplicating->SetItemString("extras", rowset);    // have to build a crowset for this
        PyObject* valObj = new PyObject("util.KeyVal", Duplicating);
        rsp->SetItem(new PyInt(6), valObj);
    } else {
        PyDecRef(skillListDup);
        PyDecRef(matlListDup);
    }
    if (re) {        //activityReverseEngineering = 7
        PyIncRef(mtCRowSet);
        PyDict* ReverseEngineering = new PyDict();
            ReverseEngineering->SetItemString("skills", skillListRE);
            ReverseEngineering->SetItemString("rawMaterials", matlListRE);
            ReverseEngineering->SetItemString("extras", mtCRowSet);
        PyObject* valObj = new PyObject("util.KeyVal", ReverseEngineering);
        rsp->SetItem(new PyInt(7), valObj);
    } else {
        PyDecRef(skillListRE);
        PyDecRef(matlListRE);
    }
    if (invent) {     //activityInvention = 8
        PyIncRef(mtCRowSet);
        PyDict* Invention = new PyDict();
            Invention->SetItemString("skills", skillListInvent);
            Invention->SetItemString("rawMaterials", matlListInvent);
            Invention->SetItemString("extras", mtCRowSet);
        PyObject* valObj = new PyObject("util.KeyVal", Invention);
        rsp->SetItem(new PyInt(8), valObj);
    } else {
        PyDecRef(skillListInvent);
        PyDecRef(matlListInvent);
    }

    PyDecRef(mtCRowSet);
    PyDecRef(extraListDup);
    PyDecRef(extraListManuf);

    return rsp;
}

/** @todo  finish this.
 *      - only used by GetCurrentEntities().  custom call for alasiya eve
 */
std::string StaticDataMgr::GetOwnerName(int32 ownerID) {
    if (ownerID == 1)
        return "System";

    return "Unknown - WIP";
}

const char* StaticDataMgr::GetDmgRptName(uint8 type) {
    switch (type) {
        case 0:  { return "None"; }
        case 1:  { return "Half Shield"; }
        case 2:  { return "Zero Shield"; }
        case 3:  { return "Half Armor"; }
        case 4:  { return "Zero Armor"; }
        case 5:  { return "Half Hull"; }
        default: { return "Undefined"; }
    }
}

uint8 StaticDataMgr::GetRegionQuarter(uint32 regionID) {
    uint32 factionID = 0;
    std::unordered_map<uint32, uint32>::iterator itr = m_regions.find(regionID);
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
        case factionUnknown:        //Rogue Drones
            return 5;
    }
    // default to 'none'
    return 5;
}

uint32 StaticDataMgr::GetFactionCorp(uint32 factionID) {
    switch (factionID) {
        case factionAngel:          return corpArchangels;
        case factionSanshas:        return corpTruePower;
        case factionBloodRaider:    return corpBloodRaider;
        case factionGuristas:       return corpGuristas;
        case factionSerpentis:      return corpSerpentis;
        case factionUnknown:        return corpRogueDrones;

        case factionCONCORD:        return corpCONCORD;
        case factionInterBus:       return corpInterbus;
        case factionSociety:        return corpSocietyofCT;
        case factionMordusLegion:   return corpMordusLegion;

        case factionCaldari:        return corpCaldariNavy;
        case factionORE:            return corpORE;
        case factionAmarr:          return corpAmarrNavy;
        case factionGallente:       return corpGuristas;
        case factionJove:           return corpJovianDirectorate;
        case factionAmmatar:        return corpAmmatarConsulate;

        case factionKhanid:         return corpKhanidNavy;
        case factionThukker:        return corpThukkerMix;
        case factionSistersOfEVE:   return corpSoE;

        case factionNoFaction:
        default:                    return 0;

        //case factionSleeper:    return corpRogueDrones;
        //case factionMinmatar:        return corp;
        //case factionSyndicate:    return corps;
    }

    return 0;
}

const char* StaticDataMgr::GetRaceName(uint8 raceID) {
    switch (raceID) {
        case Char::Race::Caldari:       return "Caldari";       //1
        case Char::Race::Minmatar:      return "Minmatar";      //2
        case Char::Race::Amarr:         return "Amarr";         //4
        case Char::Race::Sansha:        return "Sansha";        //5
        case Char::Race::Ammatar:       return "Ammatar";       //6
        case Char::Race::Gallente:      return "Gallente";      //8
        case Char::Race::Guristas:      return "Guristas";      //9
        case Char::Race::Serpentis:     return "Serpentis";     //10
        case Char::Race::Jove:          return "Jove";          //16
        case Char::Race::Pirate:        return "Pirate";        //32
        case Char::Race::Sleeper:       return "Sleeper";       //64
        case Char::Race::ORE:           return "ORE";           //128
    }
    // default to none
    return "Race Not Defined";
}

uint32 StaticDataMgr::GetRaceFaction(uint8 raceID) {
    switch (raceID) {
        case Char::Race::Caldari:       return factionCaldari;
        case Char::Race::Minmatar:      return factionMinmatar;
        case Char::Race::Amarr:         return factionAmarr;
        case Char::Race::Sansha:        return factionSanshas;
        case Char::Race::Ammatar:       return factionAmmatar;
        case Char::Race::Gallente:      return factionGallente;
        case Char::Race::Guristas:      return factionGuristas;
        case Char::Race::Serpentis:     return factionSerpentis;
        case Char::Race::Jove:          return factionJove;
        case Char::Race::Pirate:        return factionNoFaction;
        case Char::Race::Sleeper:       return factionSleeper;
        case Char::Race::ORE:           return factionORE;
    }
    // default to none
    return factionNoFaction;
}

uint8 StaticDataMgr::GetFactionRace(uint32 factionID) {
    switch (factionID) {
        case factionCaldari:        return Char::Race::Caldari;
        case factionMinmatar:       return Char::Race::Minmatar;
        case factionAmarr:          return Char::Race::Amarr;
        case factionGallente:       return Char::Race::Gallente;
        case factionJove:           return Char::Race::Jove;
        case factionNoFaction:      return Char::Race::Pirate;
        case factionSleeper:        return Char::Race::Sleeper;
        case factionORE:            return Char::Race::ORE;
        case factionAmmatar:        return Char::Race::Ammatar;
    }
    // default to Gallente
    return Char::Race::Gallente;
}

const char* StaticDataMgr::GetRigSizeName(uint8 size) {
    switch (size) {
        case 0:      return "Undefined";
        case 1:      return "Small";
        case 2:      return "Medium";
        case 3:      return "Large";
        case 4:      return "Capitol";
    }
    return "Undefined";
}

const char* StaticDataMgr::GetProcStateName(int8 state) {
    using namespace EVEPOS;
    switch(state) {
        case ProcState::Invalid:            return "Invalid";
        case ProcState::Unanchoring:        return "Unanchoring";
        case ProcState::Anchoring:          return "Anchoring";
        case ProcState::Offlining:          return "Offlining";
        case ProcState::Onlining:           return "Onlining";
        case ProcState::Online:             return "Online";
        case ProcState::Operating:          return "Operating";
        case ProcState::Reinforcing:        return "Reinforcing";
        case ProcState::SheildReinforcing:  return "SheildReinforcing";
        case ProcState::ArmorReinforcing:   return "ArmorReinforcing";
    }
    return "Undefined";
}

const char* StaticDataMgr::GetFlagName(uint16 flag) {
    return GetFlagName((EVEItemFlags)flag);
}

const char* StaticDataMgr::GetFlagName(EVEItemFlags flag) {
    switch (flag) {
        case flagAutoFit:                          return "AutoFit";
        case flagWallet:                        return "Wallet";
        //case flagFactory:                       return "Factory";
        case flagWardrobe:                      return "Wardrobe";
        case flagHangar:                        return "Hangar";
        case flagCargoHold:                     return "Cargo Hold";
        case flagBriefcase:                     return "Briefcase";
        case flagSkill:                         return "Skill";
        case flagReward:                        return "Reward";
        case flagConnected:                     return "Connected";
        case flagDisconnected:                  return "Disconnected";
        case flagLowSlot0:                      return "First Low Slot";
        case flagLowSlot1:                      return "Second Low Slot";
        case flagLowSlot2:                      return "Third Low Slot";
        case flagLowSlot3:                      return "Fourth Low Slot";
        case flagLowSlot4:                      return "Fifth Low Slot";
        case flagLowSlot5:                      return "Sixth Low Slot";
        case flagLowSlot6:                      return "Seventh Low Slot";
        case flagLowSlot7:                      return "Eighth Low Slot";
        case flagMidSlot0:                      return "First Mid Slot";
        case flagMidSlot1:                      return "Second Mid Slot";
        case flagMidSlot2:                      return "Third Mid Slot";
        case flagMidSlot3:                      return "Fourth Mid Slot";
        case flagMidSlot4:                      return "Fifth Mid Slot";
        case flagMidSlot5:                      return "Sixth Mid Slot";
        case flagMidSlot6:                      return "Seventh Mid Slot";
        case flagMidSlot7:                      return "Eighth Mid Slot";
        case flagHiSlot0:                       return "First Hi Slot";
        case flagHiSlot1:                       return "Second Hi Slot";
        case flagHiSlot2:                       return "Third Hi Slot";
        case flagHiSlot3:                       return "Fourth Hi Slot";
        case flagHiSlot4:                       return "Fifth Hi Slot";
        case flagHiSlot5:                       return "Sixth Hi Slot";
        case flagHiSlot6:                       return "Seventh Hi Slot";
        case flagHiSlot7:                       return "Eighth Hi Slot";
        case flagFixedSlot:                     return "Fixed Slot";
        case flagFactoryBlueprint:              return "Factory Blueprint";
        case flagFactoryMinerals:               return "Factory Minerals";
        case flagFactoryOutput:                 return "Factory Output";
        case flagFactoryActive:                 return "Factory Active";
        //case flagPromenadeSlot1:                return "PromenadeSlot1";
        case flagCapsule:                       return "Capsule";
        case flagPilot:                         return "Pilot";
        case flagPassenger:                     return "Passenger";
        case flagBoardingGate:                  return "Boarding Gate";
        case flagCrew:                          return "Crew";
        case flagSkillInTraining:               return "Skill In Training";
        case flagCorpMarket:                    return "Corp Market";
        case flagLocked:                        return "Locked";
        case flagUnlocked:                      return "Unlocked";
        case flagOffice:                        return "Office";
        case flagImpounded:                     return "Impounded";
        case flagProperty:                      return "Property";
        //case flagDelivery:                      return "Delivery";
        case flagBonus:                         return "Bonus";
        case flagDroneBay:                      return "Drone Bay";
        case flagBooster:                       return "Booster";
        case flagImplant:                       return "Implant";
        case flagShipHangar:                    return "Ship Hangar";
        case flagShipOffline:                   return "Ship Offline";
        case flagRigSlot0:                      return "First Rig Slot";
        case flagRigSlot1:                      return "Second Rig Slot";
        case flagRigSlot2:                      return "Third Rig Slot";
        case flagRigSlot3:                      return "Fourth Rig Slot";
        case flagRigSlot4:                      return "Fifth Rig Slot";
        case flagRigSlot5:                      return "Sixth Rig Slot";
        case flagRigSlot6:                      return "Seventh Rig Slot";
        case flagRigSlot7:                      return "Eighth Rig Slot";
        case flagFactoryOperation:              return "Factory Operation";
        case flagCorpHangar2:                   return "CorpHangar2";
        case flagCorpHangar3:                   return "CorpHangar3";
        case flagCorpHangar4:                   return "CorpHangar4";
        case flagCorpHangar5:                   return "CorpHangar5";
        case flagCorpHangar6:                   return "CorpHangar6";
        case flagCorpHangar7:                   return "CorpHangar7";
        case flagSecondaryStorage:              return "Secondary Storage";
        case flagCaptainsQuarters:              return "Captains Quarters";
        case flagWisPromenade:                  return "Promenade";
        //case flagWorldSpace:                  return "WorldSpace";
        case flagSubSystem0:                    return "First SubSystem";
        case flagSubSystem1:                    return "Second SubSystem";
        case flagSubSystem2:                    return "Third SubSystem";
        case flagSubSystem3:                    return "Fourth SubSystem";
        case flagSubSystem4:                    return "Fifth SubSystem";
        case flagSubSystem5:                    return "Sixth SubSystem";
        case flagSubSystem6:                    return "Seventh SubSystem";
        case flagSubSystem7:                    return "Eighth SubSystem";
        case flagFuelBay:                       return "Fuel Bay";
        case flagOreHold:                       return "Ore Hold";
        case flagGasHold:                       return "Gas Hold";
        case flagMineralHold:                   return "Mineral Hold";
        case flagSalvageHold:                   return "Salvage Hold";
        case flagShipHold:                      return "Ship Hold";
        case flagSmallShipHold:                 return "Small Ship Hold";
        case flagMediumShipHold:                return "Medium Ship Hold";
        case flagLargeShipHold:                 return "Large Ship Hold";
        case flagIndustrialShipHold:            return "Industrial Ship Hold";
        case flagAmmoHold:                      return "Ammunition Hold";
        case flagStructureActive:               return "Structure Active";
        case flagStructureInactive:             return "Structure Inactive";
        case flagJunkyardReprocessed:           return "Junkyard Reprocessed";
        case flagJunkyardTrashed:               return "Junkyard Trashed";
        case flagCommandCenterHold:             return "Command Center Hold";
        case flagPlanetaryCommoditiesHold:      return "Planetary Commodities Hold";
        case flagPlanetSurface:                 return "Planet Surface";
        case flagMaterialBay:                   return "Material Bay";
        case flagDustCharacterBackpack:         return "DustCharacterBackpack";
        case flagDustCharacterBattle:           return "DustCharacterBattle";
        case flagQuafeBay:                      return "Quafe Bay";
        case flagFleetHangar:                   return "Fleet Hangar";
        case flagResearchFacilitySlotFirst:     return "ResearchFacilitySlotFirst";
        case flagResearchFacilitySlotLast:      return "ResearchFacilitySlotLast";
        case flagMissile:                       return "Missile";
        case flagClone:                         return "Clone";
        case flagIllegal:                       return "Illegal";
    }
    return "Undefined";
}

const char* StaticDataMgr::GetCorpDivisionName(uint8 divisionID) {
    switch (divisionID) {
        case Corp::Division::Accounting:        return "Accounting";
        case Corp::Division::Administration:    return "Administration";
        case Corp::Division::Advisory:          return "Advisory";
        case Corp::Division::Archives:          return "Archives";
        case Corp::Division::Astrosurveying:    return "Astrosurveying";
        case Corp::Division::Command:           return "Command";
        case Corp::Division::Distribution:      return "Distribution";
        case Corp::Division::Financial:         return "Financial";
        case Corp::Division::Intelligence:      return "Intelligence";
        case Corp::Division::InternalSecurity:  return "Internal Security";
        case Corp::Division::Legal:             return "Legal";
        case Corp::Division::Manufacturing:     return "Manufacturing";
        case Corp::Division::Marketing:         return "Marketing";
        case Corp::Division::Mining:            return "Mining";
        case Corp::Division::Personnel:         return "Personnel";
        case Corp::Division::Production:        return "Production";
        case Corp::Division::PublicRelations:   return "Public Relations";
        case Corp::Division::RnD:               return "R & D";
        case Corp::Division::Security:          return "Security";
        case Corp::Division::Storage:           return "Storage";
        case Corp::Division::Surveillance:      return "Surveillance";
        case Corp::Division::DistributionNew:   return "New Distribution";
        case Corp::Division::MiningNew:         return "New Mining";
        case Corp::Division::SecurityNew:       return "New Security";
    }
    return "Undefined";
}

const char* StaticDataMgr::GetAgentTypeName(uint8 typeID) {
    switch (typeID) {
        case Agents::Type::None:                return "None";
        case Agents::Type::Basic:               return "Basic";
        case Agents::Type::Tutorial:            return "Tutorial";
        case Agents::Type::Research:            return "Research";
        case Agents::Type::Unknown:             return "Unknown";
        case Agents::Type::GenericStoryLine:    return "Generic StoryLine";
        case Agents::Type::StoryLine:           return "Specific StoryLine";
        case Agents::Type::Event:               return "Event";
        case Agents::Type::FacWar:              return "Faction Warfare";
        case Agents::Type::EpicArc:             return "Epic Arc";
        case Agents::Type::Aura:                return "Security";
        case Agents::Type::Career:              return "Career";
    }
    return "Undefined";
}

uint16 StaticDataMgr::GetHaulerTypeID(uint32 faction, uint8 level) {
    switch (faction) {
        case factionAngel: {
            // this may be best option to spawn correct hauler...certainly the easiest to code
            // there are 12 types for each faction
            switch (level) {
                case 0: {       //Ferrier    1H 4F 1D
                   return 13687; //   Ferrier 80m3
                } break;
                case 1: {       //Gatherer    1H 4F 2D 2C
                    return  13688; //   Gatherer        110m3
                } break;
                case 2: {       //Harvester   1H 4F 3D 3C
                    return 13689; //  Harvester       120m3
                } break;
                case 3: {       //Loader     2H 4F 2D 2C 1BC
                    return 13686; //  Loader
                } break;
                case 4: {       // Courier     2H 4F 2D 3C 2BC
                    return 13685; //  Courier
                } break;
                case 5: {       //Trucker    2H 4F 3D 3C 3BC
                    return 13684; //  Trucker
                } break;
                case 6: {       //Bulker      3H 4F 4D 4C 4BC
                    return 13682; //  Bulker
                } break;
                case 7: {       //Transporter 3H 4F 2AF 3C 3BC
                    return 13683; //  Transporter
                } break;
                case 8: {       //Hauler      3H 4F 2AF 3C 3Bc
                    return 13681; //  Hauler
                } break;
                case 9: {       //Trailer           4H 6F 2AF 3C 2AC 1BC
                    return 13680; //  Trailer
                } break;
                case 10: {       //Convoy      4H 6F 2AF 3C 2AC 2BC 2BS
                    return 13679; //   Convoy
                } break;
                case 11: {       // Carrier          4H 6F 3AF 3C 3AC 3BC 3BS
                    return 13678; //   Carrier
                } break;
            }
        }

        case factionBloodRaider: {
            switch (level) {
                case 0: {       //Ferrier    1H 4F 1D
                    return 13699; //   Ferrier 135m3
                } break;
                case 1: {       //Gatherer    1H 4F 2D 2C
                    return 13700; //   Gatherer       235m3
                } break;
                case 2: {       //Harvester   1H 4F 3D 3C
                    return 13701; //  Harvester
                } break;
                case 3: {       //Loader     2H 4F 2D 2C 1BC
                    return 13698; //  Loader
                } break;
                case 4: {       // Courier     2H 4F 2D 3C 2BC
                    return 13697; //  Courier
                } break;
                case 5: {       //Trucker    2H 4F 3D 3C 3BC
                    return 13696; //  Trucker
                } break;
                case 6: {       //Bulker      3H 4F 4D 4C 4BC
                    return 13694; //  Bulker
                } break;
                case 7: {       //Transporter 3H 4F 2AF 3C 3BC
                    return 13695; //  Transporter
                } break;
                case 8: {       //Hauler      3H 4F 2AF 3C 3Bc
                    return 13693; //  Hauler
                } break;
                case 9: {       //Trailer           4H 6F 2AF 3C 2AC 1BC
                    return 13692; //  Trailer
                } break;
                case 10: {       //Convoy      4H 6F 2AF 3C 2AC 2BC 2BS
                    return 13691; //  Convoy
                } break;
                case 11: {       // Carrier          4H 6F 3AF 3C 3AC 3BC 3BS
                    return 13690; //   Carrier
                } break;
            }
        }

        case factionGuristas: {
            switch (level) {
                case 0: {       //Ferrier    1H 4F 1D
                    return 13723; //  Ferrier        125m3
                } break;
                case 1: {       //Gatherer    1H 4F 2D 2C
                    return 13724; //  Gatherer        130m3
                } break;
                case 2: {       //Harvester   1H 4F 3D 3C
                    return 13725; //  Harvester        250m3
                } break;
                case 3: {       //Loader     2H 4F 2D 2C 1BC
                    return 13722; //  Loader
                } break;
                case 4: {       // Courier     2H 4F 2D 3C 2BC
                    return 13721; //  Courier
                } break;
                case 5: {       //Trucker    2H 4F 3D 3C 3BC
                    return 13720; //  Trucker
                } break;
                case 6: {       //Bulker      3H 4F 4D 4C 4BC
                    return 13718; //  Bulker
                } break;
                case 7: {       //Transporter 3H 4F 2AF 3C 3BC
                    return 13719; //  Transporter
                } break;
                case 8: {       //Hauler      3H 4F 2AF 3C 3Bc
                    return 13717; //  Hauler
                } break;
                case 9: {       //Trailer           4H 6F 2AF 3C 2AC 1BC
                    return 13716; //  Trailer
                } break;
                case 10: {       //Convoy      4H 6F 2AF 3C 2AC 2BC 2BS
                    return 13715; //  Convoy
                } break;
                case 11: {       // Carrier          4H 6F 3AF 3C 3AC 3BC 3BS
                    return 13714; //  Carrier
                } break;
            }
        }

        case factionSanshas: {
            switch (level) {
                case 0: {       //Ferrier    1H 4F 1D
                    return 13735; //  Ferrier        200m3
                } break;
                case 1: {       //Gatherer    1H 4F 2D 2C
                    return 13736; //  Gatherer
                } break;
                case 2: {       //Harvester   1H 4F 3D 3C
                    return 13737; //  Harvester        235m3
                } break;
                case 3: {       //Loader     2H 4F 2D 2C 1BC
                    return 13734; //  Loader
                } break;
                case 4: {       // Courier     2H 4F 2D 3C 2BC
                    return 13733; //  Courier
                } break;
                case 5: {       //Trucker    2H 4F 3D 3C 3BC
                    return 13732; //  Trucker
                } break;
                case 6: {       //Bulker      3H 4F 4D 4C 4BC
                    return 13730; //  Bulker
                } break;
                case 7: {       //Transporter 3H 4F 2AF 3C 3BC
                    return 13731; //  Transporter
                } break;
                case 8: {       //Hauler      3H 4F 2AF 3C 3Bc
                    return 13729; //  Hauler
                } break;
                case 9: {       //Trailer           4H 6F 2AF 3C 2AC 1BC
                    return 13728; //  Trailer
                } break;
                case 10: {       //Convoy      4H 6F 2AF 3C 2AC 2BC 2BS
                    return 13727; //  Convoy
                } break;
                case 11: {       // Carrier          4H 6F 3AF 3C 3AC 3BC 3BS
                    return 13726; //  Carrier
                } break;
            }
        }

        case factionSerpentis: {
            switch (level) {
                case 0: {       //Ferrier    1H 4F 1D
                    return 13712; //   Ferrier        140m3
                } break;
                case 1: {       //Gatherer    1H 4F 2D 2C
                    return 13711; //   Gatherer   235m3
                } break;
                case 2: {       //Harvester   1H 4F 3D 3C
                    return 13713; //  Harvester      60m3
                } break;
                case 3: {       //Loader     2H 4F 2D 2C 1BC
                    return 13710; //  Loader
                } break;
                case 4: {       // Courier     2H 4F 2D 3C 2BC
                    return 13709; //  Courier
                } break;
                case 5: {       //Trucker    2H 4F 3D 3C 3BC
                    return 13708; //  Trucker
                } break;
                case 6: {       //Bulker      3H 4F 4D 4C 4BC
                    return 13706; //  Bulker           480m3
                } break;
                case 7: {       //Transporter 3H 4F 2AF 3C 3BC
                    return 13707; //  Transporter
                } break;
                case 8: {       //Hauler      3H 4F 2AF 3C 3Bc
                    return 13705; //  Hauler
                } break;
                case 9: {       //Trailer           4H 6F 2AF 3C 2AC 1BC
                    return 13704; //  Trailer
                } break;
                case 10: {       //Convoy      4H 6F 2AF 3C 2AC 2BC 2BS
                    return 13703; //   Convoy
                } break;
                case 11: {       // Carrier          4H 6F 3AF 3C 3AC 3BC 3BS
                    return 13702; //   Carrier
                } break;
            }
        }

        case factionUnknown: {
            switch (level) {
                case 0: {       //Ferrier    1H 4F 1D
                    return 32910; //   Ferrier        80m3
                } break;
                case 1: {       //Gatherer    1H 4F 2D 2C
                    return 32911; //   Gatherer
                } break;
                case 2: {       //Harvester   1H 4F 3D 3C
                    return 32912; //  Harvester
                } break;
                case 3: {       //Loader     2H 4F 2D 2C 1BC
                    return 32909; //  Loader             120m3
                } break;
                case 4: {       // Courier     2H 4F 2D 3C 2BC
                    return 32908; //  Courier
                } break;
                case 5: {       //Trucker    2H 4F 3D 3C 3BC
                    return 32907; //  Trucker
                } break;
                case 6: {       //Bulker      3H 4F 4D 4C 4BC
                    return 32905; //  Bulker
                } break;
                case 7: {       //Transporter 3H 4F 2AF 3C 3BC
                    return 32906; //  Transporter
                } break;
                case 8: {       //Hauler      3H 4F 2AF 3C 3Bc
                    return 32904; //  Hauler
                } break;
                case 9: {       //Trailer           4H 6F 2AF 3C 2AC 1BC
                    return 32903; //  Trailer
                } break;
                case 10: {       //Convoy      4H 6F 2AF 3C 2AC 2BC 2BS
                    return 32902; //  Convoy
                } break;
                case 11: {       // Carrier          4H 6F 3AF 3C 3AC 3BC 3BS
                    return 32901; //   Carrier
                } break;
            }
        }
    }
    return 22025;  // Degenerate Harvester - failsafe
}

uint32 StaticDataMgr::GetWreckFaction(uint32 typeID) {
    // these will need to be separated and updated after detailed salvage table is completed
    switch(typeID) {
        case 26469:  //   Amarr Battlecruiser Wreck
        case 26470:  //   Amarr Battleship Wreck
        case 26472:  //   Amarr Carrier Wreck
        case 26473:  //   Amarr Cruiser Wreck
        case 26474:  //   Amarr Destroyer Wreck
        case 26475:  //   Amarr Dreadnought Wreck
        case 26476:  //  Amarr Elite Battlecruiser Wreck
        case 26477:  //   Amarr Elite Battleship Wreck
        case 26478:  //   Amarr Elite Cruiser Wreck
        case 26479:   //  Amarr Elite Destroyer Wreck
        case 26480:   //  Amarr Elite Frigate Wreck
        case 26481:   //  Amarr Elite Industrial Wreck
        case 26482:   //  Amarr Elite Mining Barge Wreck
        case 26483:   //  Amarr Freighter Wreck
        case 26484:   //  Amarr Frigate Wreck
        case 26485:  //  Amarr Industrial Wreck
        case 26486:   //  Amarr Mining Barge Wreck
        case 26487:   //  Amarr Supercarrier Wreck
        case 26488:   //  Amarr Rookie ship Wreck
        case 26489:   //  Amarr Shuttle Wreck
        case 26490:   //  Amarr Titan Wreck
        case 27050:   //  Amarr Large Wreck
        case 27051:   // Amarr Medium Wreck
        case 27052:   //  Amarr Small Wreck
        case 29033:   // Amarr Elite Freighter Wreck
        case 27927:  //  Mission Amarr Carrier Wreck
        case 30822: { //   Amarr Advanced Cruiser Wreck
            return factionAmarr;
        } break;

        case 26491:  //    Caldari Battlecruiser Wreck
        case 26492:  //    Caldari Battleship Wreck
        case 26494:  //    Caldari Carrier Wreck
        case 26495:  //   Caldari Cruiser Wreck
        case 26496:  //    Caldari Destroyer Wreck
        case 26497:  //    Caldari Dreadnought Wreck
        case 26498:  //    Caldari Elite Battlecruiser Wreck
        case 26499:  //   Caldari Elite Battleship Wreck
        case 26500:  //   Caldari Elite Cruiser Wreck
        case 26501:  //   Caldari Elite Destroyer Wreck
        case 26502:  //   Caldari Elite Frigate Wreck
        case 26503:  //   Caldari Elite Industrial Wreck
        case 26504:  //   Caldari Elite Mining Barge Wreck
        case 26505:  //    Caldari Freighter Wreck
        case 26506:  //    Caldari Frigate Wreck
        case 26507:  //    Caldari Industrial Wreck
        case 26508:  //    Caldari Mining Barge Wreck
        case 26509:  //    Caldari Supercarrier Wreck
        case 26510:  //   Caldari Rookie ship Wreck
        case 26511:  //    Caldari Shuttle Wreck
        case 26512:  //    Caldari Titan Wreck
        case 27926:  //    Mission Caldari Carrier Wreck
        case 30823:  //    Caldari Advanced Cruiser Wreck
        case 29034:  //   Caldari Elite Freighter Wreck
        case 27047:  //    Caldari Large Wreck
        case 27048:  //    Caldari Medium Wreck
        case 27049: {  //    Caldari Small Wreck
            return factionCaldari;
        } break;

        case 29035:  //    Gallente Elite Freighter Wreck
        case 30824:  //    Gallente Advanced Cruiser Wreck
        case 27929:  //    Mission Gallente Carrier Wreck
        case 27053:  //    Gallente Large Wreck
        case 27054:  //    Gallente Medium Wreck
        case 27055:  //    Gallente Small Wreck
        case 26513:  //    Gallente Battlecruiser Wreck
        case 26514:  //    Gallente Battleship Wreck
        case 26516:  //    Gallente Carrier Wreck
        case 26517:  //    Gallente Cruiser Wreck
        case 26518:  //    Gallente Destroyer Wreck
        case 26519:  //    Gallente Dreadnought Wreck
        case 26520:  //    Gallente Elite Battlecruiser Wreck
        case 26521:  //    Gallente Elite Battleship Wreck
        case 26522:  //    Gallente Elite Cruiser Wreck
        case 26523:  //    Gallente Elite Destroyer Wreck
        case 26524:  //    Gallente Elite Frigate Wreck
        case 26525:  //    Gallente Elite Industrial Wreck
        case 26526:  //    Gallente Elite Mining Barge Wreck
        case 26527:  //    Gallente Freighter Wreck
        case 26528:  //    Gallente Frigate Wreck
        case 26529:  //    Gallente Industrial Wreck
        case 26530:  //    Gallente Mining Barge Wreck
        case 26531:  //    Gallente Supercarrier Wreck
        case 26532:  //    Gallente Rookie ship Wreck
        case 26533:  //    Gallente Shuttle Wreck
        case 26534: { //   Gallente Titan Wreck
            return factionGallente;
        } break;

        case 26535:  //    Minmatar Battlecruiser Wreck
        case 26536:  //    Minmatar Battleship Wreck
        case 26538:  //    Minmatar Carrier Wreck
        case 26539:  //    Minmatar Cruiser Wreck
        case 26540:  //    Minmatar Destroyer Wreck
        case 26541:  //    Minmatar Dreadnought Wreck
        case 26542:  //    Minmatar Elite Battlecruiser Wreck
        case 26543:  //    Minmatar Elite Battleship Wreck
        case 26544:  //    Minmatar Elite Cruiser Wreck
        case 26545:  //    Minmatar Elite Destroyer Wreck
        case 26546:  //    Minmatar Elite Frigate Wreck
        case 26547:  //    Minmatar Elite Industrial Wreck
        case 26548:  //    Minmatar Elite Mining Barge Wreck
        case 29036:  //    Minmatar Elite Freighter Wreck
        case 26549:  //    Minmatar Freighter Wreck
        case 26550:  //    Minmatar Frigate Wreck
        case 26551:  //    Minmatar Industrial Wreck
        case 26552:  //    Minmatar Mining Barge Wreck
        case 26553:  //    Minmatar Supercarrier Wreck
        case 26554:  //    Minmatar Rookie ship Wreck
        case 26555:  //    Minmatar Shuttle Wreck
        case 26556:  //    Minmatar Titan Wreck
        case 27928:  //    Mission Minmatar Carrier Wreck
        case 30825:  //    Minmatar Advanced Cruiser Wreck
        case 27041:  //    Minmatar Large Wreck
        case 27042:  //    Minmatar Medium Wreck
        case 27043: { //   Minmatar Small Wreck
            return factionMinmatar;
        } break;

        case 26972:  //    Faction Drone Wreck   - faction police drones
        case 26939:  //   CONCORD Large Wreck
        case 26940:  //    CONCORD Medium Wreck
        case 26941: { //    CONCORD Small Wreck
            return factionCONCORD;
        } break;

        case 27044:  //    Khanid Large Wreck
        case 27045:  //    Khanid Medium Wreck
        case 27046: { //   Khanid Small Wreck
            return factionKhanid;
        } break;

        case 27056:  //    Thukker Large Wreck
        case 27057:  //    Thukker Medium Wreck
        case 27058: { //    Thukker Small Wreck
            return factionThukker;
        } break;

        case 27060:  //   Mordu Large Wreck
        case 27061:  //   Mordu Medium Wreck
        case 27062: { //    Mordu Small Wreck
            return factionMordusLegion;
        } break;

        case 28603:  //   Rorqual Wreck
        case 29639: { //    Orca Wreck
            return factionORE;
        } break;

        case 30457:  //    Sleeper Small Advanced Wreck
        case 30458:  //    Sleeper Medium Advanced Wreck
        case 30459:  //    Sleeper Large Wreck
        case 30484:  //    Sleeper Small Basic Wreck
        case 30485:  //    Sleeper Small Intermediate Wreck
        case 30492:  //   Sleeper Medium Basic Wreck
        case 30493:  //    Sleeper Medium Intermediate Wreck
        case 30494:  //   Sleeper Large Basic Wreck
        case 30495:  //   Sleeper Large Intermediate Wreck
        case 30496: { //    Sleeper Large Advanced Wreck
            return factionSleeper;
        } break;

        case 26561:  //   Angel Small Wreck
        case 26562:  //   Angel Medium Wreck
        case 26563:  //   Angel Large Wreck
        case 26564:  //   Angel Small Commander Wreck
        case 26699:  //   Angel Medium Commander Wreck
        case 26565:  //   Angel Large Commander Wreck
        case 26566: { //   Angel Officer Wreck
            return factionAngel;
        } break;

        case 26567:  //   Blood Small Wreck
        case 26568:  //   Blood Medium Wreck
        case 26569:  //   Blood Large Wreck
        case 26570:  //   Blood Small Commander Wreck
        case 26571:  //   Blood Medium Commander Wreck
        case 26700:  //   Blood Large Commander Wreck
        case 26572: { //   Blood Officer Wreck
            return factionBloodRaider;
        } break;

        case 26573:  //   Guristas Small Wreck
        case 26574:  //   Guristas Medium Wreck
        case 26575:  //   Guristas Large Wreck
        case 26576:  //   Guristas Small Commander Wreck
        case 26577:  //   Guristas Medium Commander Wreck
        case 26701:  //   Guristas Large Commander Wreck
        case 26578: { //   Guristas Officer Wreck
            return factionGuristas;
        } break;

        case 26579:  //   Sanshas Small Wreck
        case 26580:  //   Sanshas Medium Wreck
        case 26581:  //   Sanshas Large Wreck
        case 26582:  //   Sanshas Small Commander Wreck
        case 26583:  //   Sanshas Medium Commander Wreck
        case 26702:  //   Sanshas Large Commander Wreck
        case 26584:  //   Sanshas Officer Wreck
        case 3260: { //   Sanshas Supercarrier Wreck
            return factionSanshas;
        } break;

        case 26585:  //   Serpentis Small Wreck
        case 26586:  //   Serpentis Medium Wreck
        case 26587:  //   Serpentis Large Wreck
        case 26588:  //   Serpentis Small Commander Wreck
        case 26589:  //   Serpentis Medium Commander Wreck
        case 26703:  //   Serpentis Large Commander Wreck
        case 26590: { //   Serpentis Officer Wreck
            return factionSerpentis;
        } break;

        case 26591:  //   Rogue Small Wreck
        case 26592:  //   Rogue Medium Wreck
        case 26593:  //   Rogue Large Wreck
        case 26594:  //   Rogue Elite Small Wreck
        case 26595:  //   Rogue Elite Medium Wreck
        case 26596:  //   Rogue Officer Wreck
        case 28221:  //   Rogue Large Commander Wreck
        case 28222:  //   Rogue Medium Commander Wreck
        case 28223: { //   Rogue Small Commander Wreck
            return factionUnknown;
        } break;

        // generic wrecks
        case 26468:  //   Capsule Wreck
        case 26557:  //   Frigate Wreck
        case 26558:  //   Cruiser Wreck
        case 26559:  //   Battleship Wreck
        case 26918:  //   Overseer Frigate Wreck
        case 26919:  //   Overseer Cruiser Wreck
        case 26920:  //   Overseer Battleship Wreck
        case 27202:  //   Convoy Wreck
        case 27286:  //   Pirate Drone Wreck
        case 26560: { //   Pirate Wreck
            return factionNoFaction;
        } break;
    }

    // safe default
    return factionNoFaction;

    /*
     *    28255 :  //   Mission Faction Freighter Wreck
     *    29347:  //    Mission Faction Vessel Wreck
     *    29365:  //    Mission Faction Industrial Wreck
     */
}

// Add a new outpost to the staticDataMgr
void StaticDataMgr::AddOutpost(StationData &stData)
{
    // Update m_stationCount
    std::map<uint32, uint8>::iterator itr = m_stationCount.lower_bound(stData.systemID);
    if (itr != m_stationCount.end() && !(m_stationCount.key_comp()(stData.systemID, itr->first))) {
        itr->second = itr->second + 1;
    } else {
        m_stationCount.emplace(stData.systemID, 1);
    }

    // Update m_stationRegion
    if (m_stationRegion.find(stData.stationID) == m_stationRegion.end())
        m_stationRegion.emplace(stData.stationID, stData.regionID);

    // Update m_stationConstellation
    if (m_stationConst.find(stData.stationID) == m_stationConst.end())
        m_stationConst.emplace(stData.stationID, stData.constellationID);

    // Update m_stationSystem
    if (m_stationSystem.find(stData.stationID) == m_stationSystem.end())
        m_stationSystem.emplace(stData.stationID, stData.systemID);
}

//  marketbot shit
bool StaticDataMgr::GetStationListForSystem(uint32 systemID, std::vector<uint32>& stations) const {
    auto itr = m_stationList.find(systemID);
    if (itr != m_stationList.end()) {
        stations = itr->second;
        return true;
    }

    return false;
}
