
/*
 *  EVE_Character.h
 *   Misc Character data
 *
 */

#ifndef EVE_CHARACTER_H
#define EVE_CHARACTER_H

namespace Char {

    struct AttrData {
        uint8 intelligence;
        uint8 charisma;
        uint8 perception;
        uint8 memory;
        uint8 willpower;
    };

    namespace Race {
        enum {                  // * = defined in client
            Caldari     = 1,    //*
            Minmatar    = 2,    //*
            Amarr       = 4,    //*
            Sansha      = 5,    // Caldari + Amarr
            Ammatar     = 6,    // Minmatar + Amarr
            Gallente    = 8,    //*
            Guristas    = 9,    // Caldari + Gallente
            Serpentis   = 10,   // Minmatar + Gallente
            Jove        = 16,
            Pirate      = 32,   //*             rogue drones set to race::pirate
            Sleeper     = 64,   //*
            ORE         = 128   //*
        };
    }

    namespace Type {
        enum {
            Amarr       = 1373,
            NiKunni     = 1374,
            Civire      = 1375,
            Deteis      = 1376,
            Gallente    = 1377,
            Intaki      = 1378,
            Sebiestor   = 1379,
            Brutor      = 1380,
            Static      = 1381,
            Modifier    = 1382,
            Achura      = 1383,
            JinMei      = 1384,
            Khanid      = 1385,
            Vherokior   = 1386
        };
    }

    namespace PDState {
        enum {
            //paperdoll state.  unused, but may be used later
            NoRecustomization           = 0,
            Resculpting                 = 1,
            NoExistingCustomization     = 2,
            FullRecustomizing           = 3,
            ForceRecustomize            = 4
        };
    }

    namespace Rookie {
        namespace Ship {
            enum {
                Amarr           = 596,
                Caldari         = 601,
                Gallente        = 606,
                Minmatar        = 588
            };
        }

        namespace Weapon {
            enum {
                Amarr           = 3634, //pulse lazor (energy)
                Caldari         = 3638, //railgun (energy)
                Gallente        = 3640, //electron blaster (hybrid)
                Minmatar        = 3636  //autocannon (projectile)
            };
        }
    }
}


#endif  // EVE_CHARACTER_H