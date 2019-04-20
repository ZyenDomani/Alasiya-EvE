
 /**
  * @name CorpData.h
  *     Corporation Data containers for Alasiya EvEmu
  *
  * @Author:        Allan
  * @date:          25 December 2017
  */

#ifndef EVEMU_SRC_CORP_DATA_H_
#define EVEMU_SRC_CORP_DATA_H_


#include "../eve-server.h"

    /*
    "corpRole"      - corporation management-type roles (manager, officer, trader)  NOT access-type roles
    "rolesAtAll"    - access roles everywhere.  is joined with other access roles
    "rolesAtBase"   - access roles at base defined for this char. overrides hq if hq and base are same location for char
    "rolesAtHQ"     - access roles at corp HQ.
    "rolesAtOther"  - access roles for non-station containers with corp hangars
    */


namespace Corp {

    struct QueryData {
        // no longer used...
        //uint8 joinOp;
        uint8 queryType;
        uint8 searchOp;
        union {
            int64 value;
            std::string string;
        };
    };

    struct QueryMembers {
        uint32 characterID;
        int64 startDateTime;
        int64 titleMask;
        int64 blockRoles;
        int64 rolesAtAll;
        int64 rolesAtHQ;
        int64 rolesAtBase;
        int64 rolesAtOther;
        int64 grantableRoles;
        int64 grantableRolesAtHQ;
        int64 grantableRolesAtBase;
        int64 grantableRolesAtOther;
    };
}

#endif  // EVEMU_SRC_CORP_DATA_H_
