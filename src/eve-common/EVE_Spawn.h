
 /**
  * @name EVE_Spawn.h
  *     NPC Spawn system data for Alasiya EvEmu
  *
  * @Author:    Allan
  * @date:      25Jan18
  *
  */


#ifndef EVE_SPAWN_H
#define EVE_SPAWN_H


namespace Spawn {

    namespace Class {
        enum {
            None        = 0,
            Easy        = 1,
            Fair        = 2,
            Average     = 3,
            Medium      = 4,
            Hard        = 5,
            Crazy       = 6,
            Insane      = 7,
            Hauler      = 8,
            Commander   = 9,
            Officer     = 10
        };
    }

    namespace Group {
        enum {
            Roid        = 1,
            Roaming     = 2,
            Static      = 3,
            Anomaly     = 4,
            Mission     = 5,
            Incursion   = 6,
            Deadspace   = 7,
            Sleeper     = 8
        };

    }
}


#endif  // EVE_SPAWN_H