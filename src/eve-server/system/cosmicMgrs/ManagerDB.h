
 /**
  * @name ManagerDB.h
  *   memory object caching system for managing and saving ingame data
  * @Author:         Allan
  * @date:   17 April 2016
  */

#ifndef _EVEMU_SYSTEM_COSMICMGRS_MANAGERDB_H
#define _EVEMU_SYSTEM_COSMICMGRS_MANAGERDB_H


#include "EVE_Scanning.h"
#include "EntityList.h"
#include "POD_containers.h"
#include "system/SystemDB.h"

class ItemData;

class ManagerDB {
public:
    /* db methods for... */

    /* statistic manager */
    static void GetStatisticData(DBQueryResult& res, int64 starttime);
    static void SaveStatisticData(StatisticData& data);
    static void UpdateStatisticHistory(StatisticData& data);

    /* data manager */
    static void GetOreBySSC(DBQueryResult& res);
    static void GetSkillList(DBQueryResult& res);
    static void GetSystemData(DBQueryResult& res);
    static void GetStaticData(DBQueryResult& res); // static items in a solar system
    static void GetMoonResouces(DBQueryResult& res);
    static void GetRAMMaterials(DBQueryResult& res);
    static void GetAgentLocation(DBQueryResult& res);
    static void GetSalvageGroups(DBQueryResult& res);
    static void GetBlueprintType(DBQueryResult& res);
    static void GetTypeAttributes(DBQueryResult& res);
    static void GetRAMRequirements(DBQueryResult& res);

    static void ListAllCorpFactions(std::map<uint32, uint32> &into);
    static void ListAllFactionStationCounts(std::map<uint32, uint32> &into);
    static void ListAllFactionSystemCounts(std::map<uint32, uint32> &into);
    static void ListAllFactionRegions(std::map<int32, PyRep *> &into);
    static void ListAllFactionConstellations(std::map<int32, PyRep *> &into);
    static void ListAllFactionSolarSystems(std::map<int32, PyRep *> &into);
    static void ListAllFactionRaces(std::map<int32, PyRep *> &into);

    static PyDict* ListAllNPCCorpInfo();
    static PyObject* GetEntryTypes();
    static PyObject* GetKeyMap();
    static PyObject* GetNPCDivisions();
    static PyObject* GetBillTypes();
    static PyObjectEx* GetAgents();
    static PyObjectEx* GetOperands();

    /* belt manager */
    void ClearAsteroids();
    void SaveRoid(AsteroidData& data);
    void SaveSystemRoids(uint32 systemID, std::vector< AsteroidData >& roids);
    static void GetRegionFaction(DBQueryResult& res);
    bool RemoveAsteroid(uint32 asteroidID);
    static bool GetAsteroidData(uint32 asteroidID, AsteroidData& dbData);
    bool LoadSystemRoids(uint32 systemID, uint32& beltID, std::vector< AsteroidData >& into);
    static uint32 CreateRoidItemID(ItemData& idata, AsteroidData& adata);

    /* spawn manager */
    static void DeleteSpawnedRats();
    static void GetSpawnClasses(DBQueryResult& res);
    static void GetFactionGroups(DBQueryResult& res);
    static void GetRegionRatFaction(DBQueryResult& res);
    static void GetGroupTypeIDs(uint8 shipClass, uint16 groupID, uint32 factionID, DBQueryResult& res);

    /* dungeon manager */
    static void GetDunRoomData(DBQueryResult& res);
    static void GetDunRoomInfo(DBQueryResult& res);
    static void GetDunEntryData(DBQueryResult& res);
    static void GetDunTemplates(DBQueryResult& res);
    static void GetDunGroupData(DBQueryResult& res);
    static void GetDunSpawnInfo(DBQueryResult& res);
    static void SaveActiveDungeon(ActiveDungeon& dun);
    static void ClearDungeons();
    static void ClearDungeons(uint32 systemID);
    static bool GetSavedDungeons(uint32 systemID, std::vector< ActiveDungeon >& into);

    /* anomaly manager */
    void SaveAnomaly(CosmicSignature& sig);
    void GetAnomalyList(DBQueryResult& res);
    void GetSystemAnomalies(uint32 systemID, DBQueryResult& res);
    void GetSystemAnomalies(uint32 systemID, std::vector< CosmicSignature >& sigs);
    static GPoint GetAnomalyPos(std::string& string);

    /* wormhole manager */
    static void GetWHSystemClass(DBQueryResult& res);

};


#endif  // _EVEMU_SYSTEM_COSMICMGRS_MANAGERDB_H