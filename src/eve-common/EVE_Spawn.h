
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

    /* POD structure for spawn groups */
    struct SystemGroup { //reference to this bubble's data for spawn groups.  may need later.
        //SystemBubble* pSysBubble;   //cant use reference or pointer here...
        uint32 bubbleID=0;
        uint32 systemID=0;
        uint32 regionID=0;
        double secRating=0;
    };

    /* POD structure for spawn groups */
    struct toSpawn {
        uint8 quantity=0; //quantity to spawn for this typeID
        uint16 typeID=0;  //typeID to spawn
    };

    /* POD structure for spawn entries */
    struct Entry {     // this is a single entry for a particular spawn.  it is probably one of many
        bool respawn=false;   // is respawn enabled for this entry?  also provides conditional test for SpawnMgr::IsChaining() method
        uint8 spawnClass=0;   // spawn class.  0 = none, 1-7 = easy to insane based on sysSec, 8 = hauler, 9 = commander, 10 = officer  - 20+ are anomalies
        uint8 spawnGroup=0;   // spawn group.   1 = roaming, 2 = static, 3 = anomaly, 4 = combat, 5 = deadspace, 6 = mission, 7 = incursion, 8 = sleeper, 9 = escalation
        uint8 total=0;        // total number of this group spawned
        uint8 number=0;       // this rat's number in group (to match up with above total)
        uint8 level=0;        // spawn data subtype/wave
        uint8 classID=0;      // spawn data class id (in case we have to look it up again)
        uint16 typeID=0;      // rat type id
        uint16 groupID=0;     // rat group id (may look into changing typeID within group later on respawn (for chaining))
        uint16 spawnID=0;     // spawn id (if needed to match up with other spawns of this group (multiple spawn types in this group))
        uint16 stamp=0;       // entry stamp time to respawn (process conditional to allow for common timer and multiple respawn times)
        uint32 itemID=0;      // rat entity id
        uint32 corpID=0;      // rat corp id
        uint32 factionID=0;   // rat faction id
    };

    // these class names correspond to the type of spawn - data found in db.npcSpawnClass.notes
    namespace Class {
        enum {
            // belt and grav site spawns - grav rat class is system rat class +1
            None         = 0,
            Easy         = 1,
            Fair         = 2,
            Average      = 3,
            Medium       = 4,
            Hard         = 5,
            Crazy        = 6,
            Insane       = 7,
            Hell         = 8,
            Extra        = 9,    // placeholder - not used yet
            Hauler       = 10,
            Commander    = 11,
            Officer      = 12,

            BeltSpawn    = 19,   // test spot for non-belt/gate

            // W.I.P.
            // anomaly faction spawns...these have waves
            Hideaway     = 20,
            Burrow       = 21,
            Refuge       = 22,
            Den          = 23,
            Yard         = 24,
            RallyPoint   = 25,
            Port         = 26,
            Hub          = 27,
            Haven        = 28,
            Sanctum      = 29,
            // anomaly drone spawns...these have waves
            Cluster      = 30,
            Collection   = 31,
            Assembly     = 32,
            Gathering    = 33,
            Surveillance = 34,
            Menagerie    = 35,
            Herd         = 36,
            Squad        = 37,
            Patrol       = 38,
            Horde        = 39,
            // unrated faction spawns..these have waves and pockets
            Hideout      = 40,
            Lookout      = 41,
            Watch        = 42,
            Vigil        = 43,
            Outpost      = 44,
            Annex        = 45,
            Base         = 46,
            Fortress     = 47,
            Complex      = 48,
            StagingPoint = 49,
            // unrated drone spawns..these have waves and pockets
            HauntedYard  = 50,
            DesolateSite = 51,
            ChemicalYard = 52,
            TrialYard    = 53,
            DirtySite    = 54,
            Ruins        = 55,
            Independence = 56,
            Radiance     = 57,
            Hierarchy    = 58,
            // mag site spawns  -rats dont spawn in these sites on live.
            //   they will here.
            // salvage site (mag subcategory)
            Crumbling = 60,
            Decayed = 61,
            Ruined = 62,
            // relic site (mag subcategory)
            Looted = 65,
            Ransacked = 66,
            Pristine = 67,
            // radar site spawns    - 5 groups in each, from easy to deadly (Easy, Average, Medium, Hard, Insane)
            Shard = 70,
            Tower = 71,
            Mainframe = 72,
            Center = 73,
            Server  = 74,

            // ladar site spawns
            // lots of weird names for these....not sure how im gonna do them yet.

            // ded rated spawns  5 factions, 9 sites each, waves, pockets and gates.  drones have 3 sites on live.  (5-6 here)
            // these will start at ??

            // escalation spawns  -this will be a fair amount of data.
            // these will start at 1c

            // Mission spawns   -this will be a LOT of data...650+ missions (that i know of at this time)
            // these will start at 1k
        };
    }

    // this is currently unused, but have plans for future expansion.
    namespace Group {
        enum {
            None        = 0,
            Roaming     = 1,    //roid/gate/rats
            Static      = 2,    //ded sites are 'static'
            Anomaly     = 3,    //basic combat site
            Combat      = 4,    //non-anomaly combat site
            Deadspace   = 5,    //all non-static, non-combat sites
            Mission     = 6,
            Incursion   = 7,
            Sleeper     = 8,
            Escalation  = 9
        };
    }
}

namespace Rat {
    namespace ShipClass {
        enum {
            None                                = 0,

            // --- Legacy Asteroid Belt Progression ---
            Asteroid_Frigate                    = 1,
            Asteroid_AdvancedFrigate            = 2,
            Asteroid_Destroyer                  = 3,
            Asteroid_Cruiser                    = 4,
            Asteroid_AdvancedCruiser            = 5,
            Asteroid_Battlecruiser              = 6,
            Asteroid_Battleship                 = 7,
            Asteroid_Hauler                     = 8,
            Asteroid_Officer                    = 9,

            // --- Unified Asteroid Commander Layer ---
            Asteroid_CommanderFrigate          = 10,
            Asteroid_CommanderDestroyer        = 11,
            Asteroid_CommanderCruiser          = 12,
            Asteroid_CommanderBattlecruiser    = 13,
            Asteroid_CommanderBattleship       = 14,

            // --- Legacy Deadspace / Anomaly Progression ---
            Deadspace_Frigate                   = 15,
            Deadspace_AdvancedFrigate           = 16,
            Deadspace_Destroyer                 = 17,
            Deadspace_Cruiser                   = 18,
            Deadspace_AdvancedCruiser           = 19,
            Deadspace_Battlecruiser             = 20,
            Deadspace_Battleship                = 21,
            Deadspace_Swarm                     = 22,

            // --- New: Missing Deadspace Commander Layer ---
            Deadspace_CommanderFrigate          = 23,
            Deadspace_CommanderDestroyer        = 24,
            Deadspace_CommanderCruiser          = 25,
            Deadspace_CommanderBattlecruiser    = 26,
            Deadspace_CommanderBattleship       = 27,

            // --- New: Missing Capital & Supercapital Layers ---
            Capital_Dreadnought                 = 30,
            Capital_Carrier                     = 31,
            Capital_Supercarrier                = 32,
            Capital_Titan                       = 33,

            // --- New: Missing Wormhole Sleeper Matrices ---
            Sleeper_Frigate                     = 40,  // Emergent
            Sleeper_Cruiser                     = 41,  // Awakened
            Sleeper_Battleship                  = 42,  // Sleepless
            Sleeper_Sentry                      = 43,

            // --- New: Missing Incursion Fleet Matrix ---
            Incursion_Frigate                   = 50,
            Incursion_Cruiser                   = 51,
            Incursion_Battlecruiser             = 52,
            Incursion_Battleship                = 53,
            Incursion_Supercapital              = 54
        };
    }
}

/*
 * these are belt/anomaly site spawn class
class 1 f,d
class 2 f,d,c
class 3 f,d,c
class 4 d,c,bc
class 5 d,c,bc,bs
class 6 c,bc,bs
class 7 bc,bs

 1.0    class 1
 0.9    class 1
 0.8    class 1
 0.7    class 2
 0.6    class 2
 0.5    class 2
 0.4    class 3
 0.3    class 3
 0.2    class 3
 0.1    class 4
 0.0    class 4
-0.1    class 4
-0.2    class 5
-0.3    class 5
-0.4    class 5
-0.5    class 6
-0.6    class 6
-0.7    class 6
-0.8    class 7
-0.9    class 7
-1.0    class 7
*/

#endif  // EVE_SPAWN_H