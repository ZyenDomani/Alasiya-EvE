/*
    Author:        Allan
*/


#ifndef EVEMU_POS_POSMGR_H_
#define EVEMU_POS_POSMGR_H_

#include "ServiceDB.h"

namespace EVEPOS {
    struct TowerData {
        // tower management
        float standing;
        float status;
        bool statusDrop :1;
        bool corpWar :1;
        uint32 standingOwnerID; // corp/ally
        bool showInCalendar :1;
        bool sendFuelNotifications :1;
    };

    struct SaveData {
        uint32 itemID;
        int32 harmonic;       /* this is POS ForceField status */

        uint8 state;          /* used to hold POS state (online, reinforced, operating, etc) */
        uint32 towerID;       /* this is the controlling towerID for POS modules */
        uint64 timestamp;     /* used to track start time on POS states (onlining, reinforced, etc) */

        // for orbital infrastructure (customs office)
        GPoint rotation;      /* direction to planet (for correct orientation) */
        uint32 planetID;

        // tower management
        float standing;
        float status;
        bool statusDrop :1;
        bool corpWar :1;
        uint32 standingOwnerID; // corp/ally
        bool showInCalendar :1;
        bool sendFuelNotifications :1;
    };

    enum ForceField {
        inactive = -1,
        offline = 0,
        online = 1
    };
}

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
