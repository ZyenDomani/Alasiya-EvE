
/*
 *  EVE_Missions.h
 *   mission-specific enumerators
 *
 */

#ifndef EVE_MISSIONS_H
#define EVE_MISSIONS_H

namespace Mission {
    namespace State {
        enum {
            //  -allan 7Jul14
            Allocated    = 0,
            Offered      = 1,
            Accepted     = 2,
            Failed       = 3
        };
    }


}





#endif  // EVE_MISSIONS_H