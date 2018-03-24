/*
 *  EVE_Dungeon.h
 *   dungeon-specific enumerators
 *
 */

#ifndef EVE_DUNGEON_H
#define EVE_DUNGEON_H

namespace Dungeon {
    namespace Type {
        enum {
            Mission             = 1, // npc mission
            Gravimetric         = 2, // roids
            Magnetometric       = 3, // salvage and archeology
            Radar               = 4, // hacking
            Ladar               = 5, // gas mining
            Wormhole            = 6, // wtf is a 'wormhole'??
            Anomaly             = 7, // non-rated dungeon that isnt required to scan with probes
            Unrated             = 8, // non-rated dungeon  no waves, possible escalation to complex
            Escalation          = 9, // new dungeon from previous site. very limited access
            Rated               = 10 // DED rated dungeon
        };
    }

    namespace Event {
        namespace Msg {
            enum {
                ImminentDanger      = 1,
                NPC                 = 2,
                Environment         = 3,
                Mood                = 4,
                Story               = 5,
                MissionObjective    = 6,
                MissionInstruction  = 7,
                Warning             = 8
            };
        }

        namespace Trigger {
            enum {
                ActivateGate = 1,
                SpawnGuards = 2,
                SpawnGuardObject = 3,
                RangedNPCHealing = 4,
                RangedPlayerDamageEM = 5,
                RangedPlayerDamageExplosive = 6,
                RangedPlayerDamageKinetic = 7,
                RangedPlayerDamageThermal = 8,
                MissionCompletion = 9,
                Msg = 10,
                DungeonCompletion = 11,
                EffectBeaconActivate = 13,
                EffectBeaconDeactivate = 14,
                ObjectDespawn = 15,
                ObjectExplode = 16,
                EntityDespawn = 18,
                EntityExplode = 19,
                FacWarVictoryPointsGranted = 20,
                AgentTalkTo = 22,
                AgentMsg = 23,
                DropLoot = 24,
                RangedPlayerHealing = 25,
                RangedNPCDamageEM = 26,
                RangedNPCDamageExplosive = 27,
                RangedNPCDamageKinetic = 28,
                RangedNPCDamageThermal = 29,
                SpawnItemInCargo = 30,
                MissionFailure = 31,
                CounterAdd = 32,
                CounterSubtract = 33,
                CounterMultiply = 34,
                CounterDivide = 35,
                CounterSet = 36,
                GrantGroupReward = 37,
                GrantDelayedGroupReward = 38,
                AdjustSystemInfluence = 39,
                WarpShipAwayDespawn = 40,
                WarpShipAwayAndComeBack = 41,
                SupressAllRespawn = 42,
                GrantGroupRewardLimitedRestrictions = 45,
                OpenTutorial = 46,
                SpawnShip = 47
            };
        }
    }

    namespace Trigger {
        enum {
            Attacked = 1,
            ShipEnteredProximity = 2,
            Exploding = 3,
            ShieldConditionLevel = 4,
            ArmorConditionLevel = 5,
            StructureConditionLevel = 6,
            Mined = 7,
            RoomEntered = 8,
            RoomMinedOut = 9,
            RoomMined = 10,
            HackingSuccess = 11,
            HackingFailure = 12,
            SalvagingSuccess = 13,
            SalvagingFailure = 14,
            ArchaeologySuccess = 15,
            ArchaeologyFailure = 16,
            ShipsEnteredRoom = 17,
            RoomCapturedCorp = 18,
            RoomCapturedAlliance = 19,
            RoomCapturedFacWar = 20,
            FWShipEnteredProximity = 21,
            ItemPlacedInMissionContainer = 23,
            PlayerKilled = 26,
            EffectActivated = 27,
            ShipsLeftRoom = 28,
            ShipLeftProximity = 29,
            FWShipLeftProximity = 30,
            RoomWipedOut = 31,
            ItemRemovedFromSpawnContainer = 32,
            ItemInCargo = 33,
            CounterEQ = 34,
            CounterGT = 35,
            CounterGE = 36,
            CounterLT = 37,
            CounterLE = 38,
            FacWarLoyaltyPointsGranted = 48
        };
    }
}

/*
dunArchetypeAgentMissionDungeon = 20
dunArchetypeFacwarDefensive = 32
dunArchetypeFacwarOffensive = 35
dunArchetypeFacwarDungeons = (dunArchetypeFacwarDefensive, dunArchetypeFacwarOffensive)
dunArchetypeWormhole = 38
dunArchetypeZTest = 19
dunExpirationDelay = 48
dungeonGateUnlockPeriod = 66
DUNGEON_EVENT_TYPE_AFFECTS_ENTITY = [dunTriggerEventEntityExplode,
 dunTriggerEventEntityDespawn,
 dunTriggerEventSpawnGuards,
 dunTriggerEventWarpShipAwayDespawn,
 dunTriggerEventWarpShipAwayAndComeBack]
DUNGEON_EVENT_TYPE_AFFECTS_OBJECT = [dunTriggerEventSpawnGuardObject,
 dunTriggerEventEffectBeaconActivate,
 dunTriggerEventEffectBeaconDeactivate,
 dunTriggerEventObjectExplode,
 dunTriggerEventObjectDespawn,
 dunTriggerEventActivateGate]
DUNGEON_ORIGIN_UNDEFINED = None
DUNGEON_ORIGIN_STATIC = 1
DUNGEON_ORIGIN_AGENT = 2
DUNGEON_ORIGIN_PLAYTEST = 3
DUNGEON_ORIGIN_EDIT = 4
DUNGEON_ORIGIN_DISTRIBUTION = 5
DUNGEON_ORIGIN_PATH = 6
DUNGEON_ORIGIN_TUTORIAL = 7
dungeonSpawnBelts = 0
dungeonSpawnGate = 1
dungeonSpawnNear = 2
dungeonSpawnDeep = 3
dungeonSpawnReinforcments = 4
dungeonSpawnStations = 5
dungeonSpawnFaction = 6
dungeonSpawnConcord = 7
*/

#endif  // EVE_DUNGEON_H