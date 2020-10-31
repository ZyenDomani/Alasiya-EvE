
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

#include "../eve-common/EVE_RAM.h"

#include "map/MapDB.h"
#include "system/cosmicMgrs/ManagerDB.h"

class StaticDataMgr
: public Singleton< StaticDataMgr >
{
public:
    StaticDataMgr();
    ~StaticDataMgr();

    int                 Initialize();

    void                Clear();
    void                Close();
    void                GetInfo();

    PyObject*           GetKeyMap()                     { PyIncRef(m_keyMap); return m_keyMap; }
    PyObjectEx*         GetAgents()                     { PyIncRef(m_agents); return m_agents; }
    PyObjectEx*         GetOperands()                   { PyIncRef(m_operands); return m_operands; }
    PyObject*           GetBillTypes()                  { PyIncRef(m_billTypes); return m_billTypes; }
    PyObject*           GetEntryTypes()                 { PyIncRef(m_entryTypes); return m_entryTypes; }
    PyTuple*            GetFactionInfo()                { PyIncRef(m_factionInfo); return m_factionInfo; }
    PyObject*           GetNPCDivisions()               { PyIncRef(m_npcDivisions); return m_npcDivisions; }

    void                GetMoonResouces(std::map<uint16, uint8>& data);

    bool                IsSkillTypeID(uint16 typeID);
    bool                GetSkillName(uint16 skillID, std::string& name);

    bool                GetNPCTypes(uint16 groupID, std::vector<uint16>& typeVec);
    bool                GetNPCGroups(uint32 factionID, std::map<uint8, uint16>& groupMap);
    bool                GetNPCClasses(uint8 sClass, std::vector<RatSpawnClass>& classMap);
    uint16              GetRandRatType(uint8 sClass, uint16 groupID);
    uint32              GetWreckID(uint32 typeID);  // returns wreck typeID based on given shipTypeID (incomplete, most ships done.)
    void                GetLoot(uint32 groupID, std::vector<LootList>& lootList);

    void                GetRamReturns(uint16 typeID, int8 activityID, std::vector< EvERam::RequiredItem >& ramReqs); // bp typeID/matls
    void                GetRamMaterials(uint16 typeID, std::vector<EvERam::RamMaterials>& ramMatls);    // bp productID/matls
    void                GetRamRequirements(uint16 typeID, std::vector< EvERam::RamRequirements >& ramReqs); // bp typeID/matls
    // this is for ALL needed materials for RAM activity as listed.  these are NOT modified.
    void                GetRamRequiredItems(const uint32 typeID, const int8 activity, std::vector<EvERam::RequiredItem> &into);

    bool                GetStaticInfo(uint32 itemID, StaticData& data);
    uint16              GetStaticType(uint32 itemID);

    // this specific cache method is designed to use EITHER a stationID OR a systemID to determine system data wanted.
    const char*         GetSystemName(uint32 locationID);       //  allan 3Aug16
    // this specific cache method is designed to use EITHER a stationID OR a systemID to determine system data wanted.
    bool                GetSystemInfo(uint32 locationID, SystemData& data);      //  allan 3Aug16

    PyRep*              GetStationCount();
    // return regionID for given stationID
    uint32              GetStationRegion(uint32 stationID);
    // return constellationID for given stationID
    uint32              GetStationConstellation(uint32 stationID);
    // return systemID for given stationID
    uint32              GetStationSystem(uint32 stationID);

    uint8               GetStationCount(uint32 systemID);
    bool                GetStationList(uint32 systemID, std::vector< uint32 >& data);

    bool                GetRoidDist(const char* secClass, std::unordered_multimap<float, uint16>& roids);
    uint8               GetRegionQuarter(uint32 regionID);
    uint32              GetRegionFaction(uint32 regionID);
    uint32              GetRegionRatFaction(uint32 regionID);

    uint8               GetWHSystemClass(uint32 systemID);

    void                GetDgmTypeAttrVec(uint32 typeID, std::vector< DmgTypeAttribute >& typeAttrVec);

    PyDict*             SetBPMatlType(int8 catID, uint16 typeID, uint16 prodID);
    PyDict*             GetBPMatlData(uint16 typeID);   //this is called on EVERY "show info" of a blueprint
    void                GetBpTypeData(uint32 typeID, BlueprintTypeData& bpData);

    uint32              GetCorpID(uint32 factionID);
    uint32              GetRaceFaction(uint8 raceID);
    uint8               GetFactionRace(uint32 factionID);
    std::string         GetCorpName(uint32 corpID);
    std::string         GetFactionName(uint32 factionID);
    const char*         GetRaceName(uint8 raceID);

    void                GetSalvage(uint32 factionID, std::vector<uint32> &itemList);

    const char*         GetFlagName(uint16 flag);
    const char*         GetFlagName(EVEItemFlags flag);

    const char*         GetRigSizeName(uint8 size);

    PyInt*              GetAgentSystemID(int32 agentID);

    // this will return owner's name from any type of ID...system, region, station, player item, etc.
    std::string         GetOwnerName(int32 ownerID);

    const char*         GetProcStateName(int8 state);

    uint32              GetWreckFaction(uint32 typeID);

    // common place for *FULL* DBRowDescriptor Header creation.
    //  this way all users are using the exact same data
    DBRowDescriptor*    CreateHeader();

protected:
    void                Populate();

    std::vector<uint16>                                 m_items;

private:
    PyTuple*                                            m_factionInfo;
    PyObject*                                           m_keyMap;
    PyObject*                                           m_entryTypes;
    PyObject*                                           m_billTypes;
    PyObject*                                           m_npcDivisions;
    PyObjectEx*                                         m_agents;
    PyObjectEx*                                         m_operands;

    std::map<uint16, PyDict*>                           m_bpMatlData;       // typeID/dict*
    std::map<uint32, uint8>                             m_whRegions;        // regionID/classID
    std::map<uint32, uint32>                            m_regions;          // regionID/ownerFactionID
    std::map<uint32, uint32>                            m_ratRegions;       // regionID/ratFactionID
    std::map<uint32, SystemData>                        m_systemData;       // systemID/data
    std::map<uint32, uint32>                            m_agentSystem;      // agentID/systemID
    std::map<uint32, uint8>                             m_stationCount;     // systemID/count
    std::map<uint32, std::vector<uint32>>               m_stationList;      // systemID/data<stationID>
    std::map<uint32, uint32>                            m_stationRegion;    // stationID/regionID
    std::map<uint32, uint32>                            m_stationConst;     // stationID/systemID
    std::map<uint32, uint32>                            m_stationSystem;    // stationID/systemID
    std::map<uint32, uint8>                             m_factionRaces;     // factionID/raceID
    std::map<uint16, BlueprintTypeData>                 m_bpTypeData;       // typeID/data
    std::map<uint16, uint8>                             m_moonGoo;          // typeID/rarity
    std::map<uint16, std::string>                       m_skills;           // typeID/name
    std::map<uint32, StaticData>                        m_staticData;       // itemID/data

    std::multimap<uint16, EvERam::RamMaterials>         m_ramMatl;          // itemTypeID/data
    std::multimap<uint16, EvERam::RamRequirements>      m_ramReq;           // bpTypeID/data
    std::multimap<std::string, OreTypeChance>           m_oreBySecClass;    // systemSecClass/data

    std::multimap<uint16, DmgTypeAttribute>             m_typeAttrMap;      // typeID/data<attrID, value>

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
    std::multimap<uint32, LootGroupType>                m_LootGroupTypeMap; // typeID/data

};

//Singleton
#define sDataMgr \
    ( StaticDataMgr::get() )


#endif  // _EVE_SERVER_STATIC_DATAMANAGER_H__
