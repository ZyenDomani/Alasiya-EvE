/*
    Author:        Allan
*/


#ifndef EVEMU_POS_POSMGR_H_
#define EVEMU_POS_POSMGR_H_

#include "ServiceDB.h"
#include "../../eve-common/EVE_POS.h"

class PosMgrDB
: public ServiceDB
{
public:
    PyRep *GetControlTowerFuelRequirements();
    PyRep *GetSiloCapacityByItemID(uint16 typeID);

    // pos data methods
    bool GetPOSData(EVEPOS::SaveData& data);
    void SavePOSData(EVEPOS::SaveData& data);
    void UpdatePOSData(EVEPOS::SaveData& data);

    void UpdatePOSNotify(uint32 towerID, EVEPOS::TowerData& data);
    void UpdatePOSSentry(uint32 towerID, EVEPOS::TowerData& data);
    void UpdatePOSPassword(uint32 towerID, EVEPOS::TowerData& data);
    void UpdatePOSPermission(uint32 towerID, EVEPOS::TowerData& data);
    void UpdatePOSTimeStamp(uint32 towerID, uint64 timeStamp);

};

#endif  // EVEMU_POS_POSMGR_H_

/*environment/spaceObject/station.py*/
