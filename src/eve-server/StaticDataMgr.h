
 /**
  * @name StaticDataMgr.h
  *   memory object caching system for retrieving, managing and saving ingame data
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


#include "eve-server.h"
#include "POD_containers.h"

#include "../eve-common/EVE_Character.h"

#include "../eve-common/EVE_RAM.h"
#include "../eve-common/EVE_Market.h"


class StaticDataMgr
: public Singleton< StaticDataMgr >
{
public:
    int                 Initialize();

    void                Clear();
    void                Close();
    void                GetInfo();

    PyObject*           GetKeyMap()                     { PyIncRef(m_keyMap); return m_keyMap; }
    PyObjectEx*         GetAgents()                     { PyIncRef(m_agents); return m_agents; }
    PyObjectEx*         GetOperands()                   { PyIncRef(m_operands); return m_operands; }
    PyObject*           GetBillTypes()                  { PyIncRef(m_billTypes); return m_billTypes; }
    PyObject*           GetEntryTypes()                 { PyIncRef(m_entryTypes); return m_entryTypes; }
    PyObjectEx*         GetFactionIDs()                 { PyIncRef(m_factionIDs); return m_factionIDs; }
    PyTuple*            GetFormations()                 { PyIncRef(m_formations); return m_formations; }
    PyTuple*            GetFactionInfo()                { PyIncRef(m_factionInfo); return m_factionInfo; }
    PyObject*           GetNPCDivisions()               { PyIncRef(m_npcDivisions); return m_npcDivisions; }


    void                GetCategory(uint8 catID, Inv::CatData &into);
    void                GetGroup(uint16 grpID, Inv::GrpData &into);
    void                GetType(uint16 typeID, Inv::TypeData &into);
    void                GetTypes(std::unordered_map<uint16, Inv::TypeData> &into);
    uint8               GetMetaLevel(uint16 typeID);
    const char*         GetAttrName(uint16 attrID);
    const char*         GetTypeName(uint16 typeID);     // not sure if this will be needed
    const char*         GetGroupName(uint16 grpID);
    const char*         GetCategoryName(uint8 catID);
    const char*         GetCorpDivisionName(uint8 divisionID);

    void                GetAncestryBonuses(uint8 ancestryID, Char::AttrData& into);
    void                GetBloodlineBonuses(uint8 bloodlineID, Char::AttrData& into);

    void                GetMineralData(std::map< uint16, Market::matlData >& into);
    void                GetSalvageData(std::map< uint16, Market::matlData >& into);
    void                GetCompoundData(std::map< uint16, Market::matlData >& into);
    void                GetComponentData(std::map< uint16, Market::matlData >& into);
    void                GetPIResourceData(std::map< uint16, Market::matlData >& into);
    void                GetPICommodityData(std::map< uint16, Market::matlData >& into);
    void                GetMiscCommodityData(std::map< uint16, Market::matlData >& into);
    void                GetMoonResouces(std::map<uint16, uint8>& data);

    bool                IsSkillTypeID(uint16 typeID);
    const char*         GetSkillName(uint16 skillID);

    bool                GetNPCTypes(uint16 groupID, std::vector<uint16>& typeVec);
    bool                GetNPCGroups(uint32 factionID, std::map<uint8, uint16>& groupMap);
    bool                GetNPCClasses(uint8 sClass, std::vector<RatSpawnClass>& classMap);
    uint16              GetRandRatType(uint8 sClass, uint16 groupID);
    uint32              GetWreckID(uint32 typeID);  // returns wreck typeID based on given shipTypeID (incomplete, most ships done.)
    void                GetLoot(float secValue, uint32 groupID, std::vector<LootList>& lootList);

    bool                IsPublished(uint16 typeID);
    bool                IsRefinable(uint16 typeID);
    bool                IsRecyclable(uint16 typeID);
    void                GetRamReturns(uint16 typeID, int8 activityID, std::vector< EvERam::RequiredItem >& ramReqs); // bp typeID/data
    void                GetRamMaterials(uint16 typeID, std::vector<EvERam::RamMaterials>& ramMatls);    // bp productTypeID/data{typeID/qty}
    void                GetRamRequirements(uint16 typeID, std::vector< EvERam::RamRequirements >& ramReqs); // bp typeID/data
    // this is for ALL needed materials for RAM activity from BP.  these are NOT modified.
    void                GetRamRequiredItems(const uint32 typeID, const int8 activity, std::vector<EvERam::RequiredItem> &into);

    bool                GetStaticInfo(uint32 itemID, StaticData& data);
    uint16              GetStaticType(uint32 itemID);

    // this specific cache method is designed to use either a stationID or a systemID to determine system data wanted.
    const char*         GetSystemName(uint32 locationID);       //  allan 3Aug16
    // this specific cache method is designed to use either a stationID or a systemID to determine system data wanted.
    bool                GetSystemData(uint32 locationID, SystemData& data);      //  allan 3Aug16

    PyRep*              GetStationCount();
    // return regionID for given stationID
    uint32              GetStationRegion(uint32 stationID);
    // return constellationID for given stationID
    uint32              GetStationConstellation(uint32 stationID);
    // return systemID for given stationID
    uint32              GetStationSystem(uint32 stationID);

    // return regionID for given systemID
    uint32              GetSystemRegion(uint32 stationID);
    // return constellationID for given systemID
    uint32              GetSystemConstellation(uint32 stationID);
    // return vector of all systems in given region
    void                GetRegionSystems(uint32 regionID, std::vector<uint32>& into);
    // return vector of all systems in given constellation
    void                GetConstellationSystems(uint32 constellationID, std::vector<uint32>& into);

    // get system data for system via either systemID or stationID
    bool                GetSolarSystemData(uint32 locationID, SolarSystemData &into);

    // not sure if we wanna put this in static data....503k items
    //bool                GetCelestialObjectData(uint32 celestialID, CelestialObjectData &into);

    uint8               GetStationCount(uint32 systemID);
    bool                GetStationList(uint32 systemID, std::vector< uint32 >& data);

    bool                GetRoidDist(const char* secClass, std::unordered_multimap<float, uint16>& roids);
    uint8               GetRegionQuarter(uint32 regionID);
    uint32              GetRegionFaction(uint32 regionID);
    uint32              GetRegionRatFaction(uint32 regionID);

    uint8               GetWHSystemClass(uint32 systemID);
    std::vector<uint32> GetWHDestinationTypes(uint32 classID) { return m_whClassDestinations[classID]; }
    std::vector<uint32> GetWHClassSystems(uint8 classID) { return m_whClassSystems[classID]; }

    void                GetDgmTypeAttrVec(uint16 typeID, std::vector< Inv::DmgTypeAttribute >& typeAttrVec);

    PyDict*             SetBPMatlType(int8 catID, uint16 typeID, uint16 prodID);
    PyDict*             GetBPMatlData(uint16 typeID);   //this is called on EVERY "show info" of a blueprint
    void                GetBpTypeData(uint16 typeID, EvERam::bpTypeData& tData);
    bool                GetBpDataForItem(uint16 typeID, EvERam::bpTypeData& tData);

    uint32              GetFactionCorp(uint32 factionID);
    uint32              GetCorpFaction(uint32 corpID);
    uint32              GetRaceFaction(uint8 raceID);
    uint8               GetFactionRace(uint32 factionID);
    std::string         GetCorpName(uint32 corpID);
    std::string         GetFactionName(uint32 factionID);
    const char*         GetRaceName(uint8 raceID);

    void                GetSalvage(uint32 factionID, std::vector<uint32> &itemList);

    const char*         GetFlagName(uint16 flag);
    const char*         GetFlagName(EVEItemFlags flag);
    const char*         GetRigSizeName(uint8 size);

    uint32              GetAgentCorpID(uint32 agentID);
    PyInt*              GetAgentSystemID(int32 agentID);
    const char*         GetAgentTypeName(uint8 typeID);

    // this will return owner's name from any type of ID...system, region, station, player item, etc.
    std::string         GetOwnerName(int32 ownerID);

    const char*         GetProcStateName(int8 state);

    uint32              GetWreckFaction(uint32 typeID);

    // methods to verify valid locationID
    bool                IsStation(uint32 stationID=0);
    bool                IsSolarSystem(uint32 systemID=0);

    // solar system methods for map system
    bool                IsHubSystem(uint32 systemID=0);       // Hub: 3+ connections to this system
    bool                IsFringeSystem(uint32 systemID=0);    // Fringe: 1 connection to this system (dead end system)
    bool                IsCorridorSystem(uint32 systemID=0);  // Corridor: 2 connections to this system (in one side and out the other)
    bool                IsRegionSystem(uint32 systemID=0);    // Regional: borders another region
    bool                IsConSystem(uint32 systemID=0);       // Constellation: borders another constellation

    // common place for *FULL* DBRowDescriptor Header creation.
    //  this way all users have the exact same data
    DBRowDescriptor*    CreateHeader()                  { return m_itemHeader; }
    DBRowDescriptor*    CreateItemHeader();
    PyTuple*            CreateFormationTuple();

    void                AddOutpost(StationData &stData);

    const char*         GetDmgRptName(uint8 type);

    // get specific hauler type for spawn
    uint16              GetHaulerTypeID(uint32 faction, uint8 level);

    bool                GetStationListForSystem(uint32 systemID, std::vector<uint32>& stations) const;

protected:
    void                Populate();
    void                ProcessLootModifiers(uint8 classID, LootPool& pool);
    void                InjectSleeperSalvage(LootPool& pool);
    void LoadLoot();
    void LoadSalvageTables();
    std::vector<LootPool> FetchPoolsForGroup(uint32 groupID, bool isAdvanced, bool isCommander);
    void GetLootFinal(float trueSec, uint32 shipClassID, std::vector<LootList>& lootList);


private:
    PyTuple*                                            m_factionInfo;
    PyTuple*                                            m_formations;
    PyObject*                                           m_keyMap;
    PyObject*                                           m_entryTypes;
    PyObject*                                           m_billTypes;
    PyObject*                                           m_npcDivisions;
    PyObjectEx*                                         m_agents;
    PyObjectEx*                                         m_operands;
    PyObjectEx*                                         m_factionIDs;
    DBRowDescriptor*                                    m_itemHeader;
    DBRowDescriptor*                                    m_bpMatlHeader;
    GetFactionInfoRsp*                                  m_pFactionInfo;

    std::map<uint32, uint8>                             m_stationCount;     // systemID/count

    std::unordered_map<uint16, Inv::CatData>            m_catData;
    std::unordered_map<uint16, Inv::GrpData>            m_grpData;
    std::unordered_map<uint16, Inv::TypeData>           m_typeData;

    std::unordered_map<uint16, PyDict*>                 m_bpMatlData;       // typeID/dict*
    std::unordered_map<uint32, uint8>                   m_whRegions;        // regionID/classID
    std::unordered_map<uint32, std::vector<uint32>>     m_whClassDestinations; //classID/typeID
    std::unordered_map<uint32, std::vector<uint32>>     m_whClassSystems;   // classID/systemID
    std::unordered_map<uint32, uint32>                  m_regions;          // regionID/ownerFactionID
    std::unordered_map<uint32, uint32>                  m_ratRegions;       // regionID/ratFactionID
    std::unordered_map<uint32, uint32>                  m_agentCorp;        // agentID/corpID
    std::unordered_map<uint32, uint32>                  m_agentSystem;      // agentID/systemID
    std::unordered_map<uint32, std::string>             m_factionName;      // factionID/name
    std::unordered_map<uint32, std::string>             m_corpName;         // corpID/name
    std::unordered_map<uint32, uint32>                  m_corpFaction;      // corpID/factionID
    std::unordered_map<uint32, std::vector<uint32>>     m_stationList;      // systemID/data<stationID>
    std::unordered_map<uint32, uint32>                  m_stationRegion;    // stationID/regionID
    std::unordered_map<uint32, uint32>                  m_stationConst;     // stationID/constellationID
    std::unordered_map<uint32, uint32>                  m_stationSystem;    // stationID/systemID
    std::unordered_map<uint32, SolarSystemData>         m_solSysData;       // systemID/data
    std::unordered_map<uint32, uint8>                   m_factionRaces;     // factionID/raceID
    std::unordered_map<uint16, EvERam::bpTypeData>      m_bpTypeData;       // typeID/data
    std::unordered_map<uint16, uint8>                   m_moonGoo;          // typeID/rarity
    std::unordered_map<uint16, std::string>             m_skills;           // typeID/name
    std::unordered_map<uint32, StaticData>              m_staticData;       // itemID/data
    std::unordered_map<uint16, Inv::AttrTypeData>       m_attrTypeData;     // attrID/data
    std::unordered_map<uint8, Char::AttrData>           m_ancestryBonuses;  // ancestryID/data
    std::unordered_map<uint8, Char::AttrData>           m_bloodlineBonuses; // bloodlineID/data

    std::multimap<uint16, EvERam::RamMaterials>         m_ramMatl;          // itemTypeID/data
    std::multimap<uint16, EvERam::RamRequirements>      m_ramReq;           // bpTypeID/data
    std::multimap<std::string, OreTypeChance>           m_oreBySecClass;    // systemSecClass/data

    std::multimap<uint16, Inv::DmgTypeAttribute>        m_typeAttrMap;      // typeID/data
    std::multimap<uint32, uint32>                       m_constSystems;     // constID/systemID
    std::multimap<uint32, uint32>                       m_regionSystems;    // regionID/systemID

    /* spawn data */
    // roid rats
    typedef std::vector<uint16>                         rt_typeIDs;
    typedef std::map<uint16, rt_typeIDs>                rt_groups;          // groupID/[typeIDs]
    std::multimap<uint8, rt_groups>                     m_npcTypes;         // spawnClass/[ratGroupID/(ratTypeID)]
    std::multimap<uint8, RatSpawnClass>                 m_npcClasses;       // spawnType/data
    std::multimap<uint32, RatFactionGroups>             m_npcGroups;        // factionID/data
    // deadspace

    // incursion

    /* salvage data */
    std::multimap<uint32, uint32>                       m_salvageMap;       // factionID/itemID

    /* ship types to wreck types data */
    std::map<uint32, uint32>                            m_WrecksToTypesMap; // typeID/wreckTypeID

    /* loot data */
    std::multimap<uint32, LootGroup>                    m_LootGroupMap;     // typeID/data
    std::multimap<uint32, LootType>                     m_LootTypeMap;      // typeID/data
    std::unordered_map<uint32, LootProfile>             m_ClassToProfileMap;
    std::unordered_map<uint32, std::vector<LootPool>>   m_FactionToSalvageMap;
    std::unordered_map<uint32, LootProfile>             m_TypeToProfileMap;  // Fast O(1) typeID lookup
    std::unordered_map<uint32, LootProfile>             m_GroupToProfileMap; // Fallback O(1) groupID lookup

    /* for pricing methods */
    std::map<uint16, std::string>                       m_salvage;          // typeID/name
    std::map<uint16, std::string>                       m_minerals;         // typeID/name
    std::map<uint16, std::string>                       m_compounds;        // typeID/name
    std::map<uint16, std::string>                       m_resources;        // typeID/name
    std::map<uint16, std::string>                       m_components;       // typeID/name
    std::map<uint16, std::string>                       m_commodities;      // typeID/name
    std::map<uint16, std::string>                       m_miscCommodities;  // typeID/name
    std::map<uint16, EvERam::bpTypeData>                m_bpProductData;    // productTypeID/data
};

//Singleton
#define sDataMgr \
    ( StaticDataMgr::get() )


#endif  // _EVE_SERVER_STATIC_DATAMANAGER_H__

/*   module (meta) types
 * metaGroupID  metaGroupName   description     iconID
 *      1       Tech I
 *      2       Tech II
 *      3       Storyline
 *      4       Faction
 *      5       Officer
 *      6       Deadspace
 *      7       Frigate
 *      8       Elite Frigate
 *      9       Commander Frigate
 *      10      Destroyer
 *      11      Cruiser
 *      12      Elite Cruiser
 *      13      Commander Cruiser
 *      14      Tech III
 *
 */
