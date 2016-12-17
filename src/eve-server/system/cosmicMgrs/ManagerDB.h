
 /**
  * @name ManagerDB.h
  *   memory object caching system for managing and saving ingame data
  * @Author:         Allan
  * @date:   17 April 2016
  */

#ifndef _EVEMU_SYSTEM_COSMICMGRS_MANAGERDB_H
#define _EVEMU_SYSTEM_COSMICMGRS_MANAGERDB_H


#include "EntityList.h"
#include "POD_containers.h"
#include "system/SystemDB.h"

/* more data for signatures...
 * this will have to be checked and set in the code.
 * this is def for scanGroupID:
 *
typedef enum {
    ScanGroupScrap                = 1,
    ScanGroupSignature            = 4,
    ScanGroupShip                 = 8,
    ScanGroupStructure            = 16,
    ScanGroupDroneOrProbe         = 32,
    ScanGroupCelestial            = 64,
    ScanGroupAnomaly              = 128
} ScanGroup;
 *
 *  for strengthAttributeID, use these attributes to indicate site type:

 AttrScanRadarStrength = 208,
 AttrScanLadarStrength = 209,
 AttrScanMagnetometricStrength = 210,
 AttrScanGravimetricStrength = 211,
 AttrScanAllStrength = 1136     - unknown

    */

class ManagerDB {
public:

    /* db methods for all cosmic managers */

    /* data manager */
    void GetOreBySSC(DBQueryResult& res);

    /* belt manager */
    void SaveRoid(AsteroidData& data);
    void SaveSystemRoids(uint32 systemID, std::vector< AsteroidData >& roids);
    void GetRegionFaction(DBQueryResult& res);
    bool LoadSystemRoids(uint32 systemID, uint32& beltID, std::vector< AsteroidData >& into);

    /* spawn manager */
    void DeleteSpawnedRats();
    void GetSpawnClasses(DBQueryResult& res);
    void GetGroupTypeIDs(uint32 groupID, DBQueryResult& res);
    void GetFactionGroups(DBQueryResult& res);
    void GetRegionRatFaction(DBQueryResult& res);

    /* dungeon manager */
    void GetDunTemplates(DBQueryResult& res);
    void GetDunRoomInfo(DBQueryResult& res);
    void GetDunRoomData(DBQueryResult& res);
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
    GPoint GetAnomalyPos(std::string& string);

    /* wormhole manager */

protected:

private:


};



#endif  // _EVEMU_SYSTEM_COSMICMGRS_MANAGERDB_H