/*
    Author:        Allan
*/


#ifndef EVEMU_POS_POSMGR_H_
#define EVEMU_POS_POSMGR_H_

#include "ServiceDB.h"

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

};

#endif  // EVEMU_POS_POSMGR_H_

/*environment/spaceObject/station.py*/
