/*
 *
 *
 *
 */

#ifndef EVE_DUNGEON_H
#define EVE_DUNGEON_H

namespace EVEDUNG {
    enum dunTypes {
        typeMission             = 1, // npc mission
        typeGravimetric         = 2, // roids
        typeMagnetometric       = 3, // salvage/archeology
        typeRadar               = 4, // hacking
        typeLadar               = 5, // gas mining
        typeWormhole            = 6, // ?
        typeAnomaly             = 7, //
        typeUnrated             = 8, // non-rated dungeon  no waves, possible escalation to complex
        typeEscalation          = 9, // extra rooms from previous sute
        typeDED_Complex         = 10 // DED rated dungeon
    };

    enum dunEvents {
        msgImminentDanger       = 1,
        msgNPC                  = 2,
        msgEnvironment          = 3,
        msgMood                 = 4,
        msgStory                = 5,
        msgMissionObjective     = 6,
        msgMissionInstruction   = 7,
        msgWarning              = 8
    };

    enum dunEventTrigger {
        trigEventActivateGate = 1,
        trigEventSpawnGuards = 2,
        trigEventSpawnGuardObject = 3,
        trigEventRangedNPCHealing = 4,
        trigEventRangedPlayerDamageEM = 5,
        trigEventRangedPlayerDamageExplosive = 6,
        trigEventRangedPlayerDamageKinetic = 7,
        trigEventRangedPlayerDamageThermal = 8,
        trigEventMissionCompletion = 9,
        trigEventMsg = 10,
        trigEventDungeonCompletion = 11,
        trigEventEffectBeaconActivate = 13,
        trigEventEffectBeaconDeactivate = 14,
        trigEventObjectDespawn = 15,
        trigEventObjectExplode = 16,
        trigEventEntityDespawn = 18,
        trigEventEntityExplode = 19,
        trigFacWarVictoryPointsGranted = 20,
        trigEventAgentTalkTo = 22,
        trigEventAgentMsg = 23,
        trigEventDropLoot = 24,
        trigEventRangedPlayerHealing = 25,
        trigEventRangedNPCDamageEM = 26,
        trigEventRangedNPCDamageExplosive = 27,
        trigEventRangedNPCDamageKinetic = 28,
        trigEventRangedNPCDamageThermal = 29,
        trigEventSpawnItemInCargo = 30,
        trigEventMissionFailure = 31,
        trigEventCounterAdd = 32,
        trigEventCounterSubtract = 33,
        trigEventCounterMultiply = 34,
        trigEventCounterDivide = 35,
        trigEventCounterSet = 36,
        trigEventGrantGroupReward = 37,
        trigEventGrantDelayedGroupReward = 38,
        trigEventAdjustSystemInfluence = 39,
        trigEventWarpShipAwayDespawn = 40,
        trigEventWarpShipAwayAndComeBack = 41,
        trigEventSupressAllRespawn = 42,
        trigEventGrantGroupRewardLimitedRestrictions = 45,
        trigEventOpenTutorial = 46,
        trigEventSpawnShip = 47
    };

    enum dunTrigger {
        trigAttacked = 1,
        trigShipEnteredProximity = 2,
        trigExploding = 3,
        trigShieldConditionLevel = 4,
        trigArmorConditionLevel = 5,
        trigStructureConditionLevel = 6,
        trigMined = 7,
        trigRoomEntered = 8,
        trigRoomMinedOut = 9,
        trigRoomMined = 10,
        trigHackingSuccess = 11,
        trigHackingFailure = 12,
        trigSalvagingSuccess = 13,
        trigSalvagingFailure = 14,
        trigArchaeologySuccess = 15,
        trigArchaeologyFailure = 16,
        trigShipsEnteredRoom = 17,
        trigRoomCapturedCorp = 18,
        trigRoomCapturedAlliance = 19,
        trigRoomCapturedFacWar = 20,
        trigFWShipEnteredProximity = 21,
        trigItemPlacedInMissionContainer = 23,
        trigPlayerKilled = 26,
        trigEffectActivated = 27,
        trigShipsLeftRoom = 28,
        trigShipLeftProximity = 29,
        trigFWShipLeftProximity = 30,
        trigRoomWipedOut = 31,
        trigItemRemovedFromSpawnContainer = 32,
        trigItemInCargo = 33,
        trigCounterEQ = 34,
        trigCounterGT = 35,
        trigCounterGE = 36,
        trigCounterLT = 37,
        trigCounterLE = 38,
        trigFacWarLoyaltyPointsGranted = 48
    };

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