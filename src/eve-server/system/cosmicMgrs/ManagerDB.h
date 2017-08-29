
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


class ManagerDB {
public:
    /* db methods for... */

    /* data manager */
    void GetOreBySSC(DBQueryResult& res);
    void GetSkillList(DBQueryResult& res);
    void GetSystemData(DBQueryResult& res);
    void GetStaticData(DBQueryResult& res); // static items in a solar system
    void GetStationInfo(DBQueryResult& res);
    void GetMoonResouces(DBQueryResult& res);
    void GetRAMMaterials(DBQueryResult& res);
    void GetBlueprintType(DBQueryResult& res);
    void GetStationSystem(DBQueryResult& res);
    void GetStationRegion(DBQueryResult& res);
    void GetTypeAttributes(DBQueryResult& res);
    void GetRAMRequirements(DBQueryResult& res);

    /* belt manager */
    void SaveRoid(AsteroidData& data);
    void SaveSystemRoids(uint32 systemID, std::vector< AsteroidData >& roids);
    void GetRegionFaction(DBQueryResult& res);
    bool LoadSystemRoids(uint32 systemID, uint32& beltID, std::vector< AsteroidData >& into);

    /* spawn manager */
    void DeleteSpawnedRats();
    void GetSpawnClasses(DBQueryResult& res);
    void GetFactionGroups(DBQueryResult& res);
    void GetRegionRatFaction(DBQueryResult& res);
    void GetGroupTypeIDs(uint32 groupID, DBQueryResult& res);

    /* dungeon manager */
    void GetDunRoomInfo(DBQueryResult& res);
    void GetDunRoomData(DBQueryResult& res);
    void GetDunTemplates(DBQueryResult& res);
    void GetDunGroupData(DBQueryResult& res);
    void GetDunSpawnInfo(DBQueryResult& res);
    void SaveActiveDungeon(ActiveDungeon& dun);
    void ClearDungeons();
    bool GetSavedDungeons(uint32 systemID, std::vector< ActiveDungeon >& into);

    /* anomaly manager */
    void SaveAnomaly(CosmicSignature& sig);
    void GetAnomalyList(DBQueryResult& res);
    void GetSystemAnomalies(uint32 systemID, DBQueryResult& res);
    void GetSystemAnomalies(uint32 systemID, std::vector< CosmicSignature >& sigs);
    static GPoint GetAnomalyPos(std::string& string);

    /* wormhole manager */
    void GetWHSystemClass(DBQueryResult& res);
    

protected:

private:


};



#endif  // _EVEMU_SYSTEM_COSMICMGRS_MANAGERDB_H