/*
 *
 *
 *
 */

#ifndef EVE_SCANNING_H
#define EVE_SCANNING_H

static float probeResultPerfect     = 1.0f;
static float probeResultInformative = 0.75f;
static float probeResultGood        = 0.25f;
static float probeResultUnusable    = 0.001f;

namespace Scanning {
//  -allan 7Jul14
    namespace Group {
        enum {
            Scrap         = 1,      //wrecks in system (unused - throws error)
            Signature     = 4,      //advanced anomaly.  need probes to scan
            Ship          = 8,      //abandoned ships
            Structure     = 16,     //all pos structures
            DroneOrProbe  = 32,     //player items
            Celestial     = 64,     //unknown  (unused - throws error)
            Anomaly       = 128     //detected using ship sensors
        };
    }
}

namespace Probe {
    namespace State {
        enum {
            Inactive     = 0,
            Idle         = 1,
            Moving       = 2,
            Warping      = 3,
            Scanning     = 4,
            Returning    = 5
        };
    }
}

/*
EXPLORATION_SITE_TYPES = {attributeScanGravimetricStrength: 'UI/Inflight/Scanner/Gravimetric',
    attributeScanLadarStrength: 'UI/Inflight/Scanner/Ladar',
    attributeScanMagnetometricStrength: 'UI/Inflight/Scanner/Magnetometric',
    attributeScanRadarStrength: 'UI/Inflight/Scanner/Radar',
    attributeScanAllStrength: 'UI/Common/Unknown'}
    */
/*
 *
 probeScanGroupScrap = 1
 probeScanGroupSignatures = 4
 probeScanGroupShips = 8
 probeScanGroupStructures = 16
 probeScanGroupDronesAndProbes = 32
 probeScanGroupCelestials = 64
 probeScanGroupAnomalies = 128
 probeScanGroups = {}
 probeScanGroups[probeScanGroupScrap] = set([groupBiomass,
                                            groupCargoContainer,
                                            groupWreck,
                                            groupSecureCargoContainer,
                                            groupAuditLogSecureContainer])
 probeScanGroups[probeScanGroupSignatures] = set([groupCosmicSignature])
 probeScanGroups[probeScanGroupAnomalies] = set([groupCosmicAnomaly])
 probeScanGroups[probeScanGroupShips] = set([groupAssaultShip,
                                            groupBattlecruiser,
                                            groupBattleship,
                                            groupBlackOps,
                                            groupCapitalIndustrialShip,
                                            groupCapsule,
                                            groupCarrier,
                                            groupCombatReconShip,
                                            groupCommandShip,
                                            groupCovertOps,
                                            groupCruiser,
                                            groupDestroyer,
                                            groupDreadnought,
                                            groupElectronicAttackShips,
                                            groupEliteBattleship,
                                            groupExhumer,
                                            groupForceReconShip,
                                            groupFreighter,
                                            groupFrigate,
                                            groupHeavyAssaultShip,
                                            groupHeavyInterdictors,
                                            groupIndustrial,
                                            groupIndustrialCommandShip,
                                            groupInterceptor,
                                            groupInterdictor,
                                            groupJumpFreighter,
                                            groupLogistics,
                                            groupMarauders,
                                            groupMiningBarge,
                                            groupSupercarrier,
                                            groupPrototypeExplorationShip,
                                            groupRookieship,
                                            groupShuttle,
                                            groupStealthBomber,
                                            groupTitan,
                                            groupTransportShip,
                                            groupStrategicCruiser])
 probeScanGroups[probeScanGroupStructures] = set([groupConstructionPlatform,
                                                 groupStationUpgradePlatform,
                                                 groupStationImprovementPlatform,
                                                 groupMobileWarpDisruptor,
                                                 groupAssemblyArray,
                                                 groupControlTower,
                                                 groupCorporateHangarArray,
                                                 groupElectronicWarfareBattery,
                                                 groupEnergyNeutralizingBattery,
                                                 groupForceFieldArray,
                                                 groupJumpPortalArray,
                                                 groupLogisticsArray,
                                                 groupMobileHybridSentry,
                                                 groupMobileLaboratory,
                                                 groupMobileLaserSentry,
                                                 groupMobileMissileSentry,
                                                 groupMobilePowerCore,
                                                 groupMobileProjectileSentry,
                                                 groupMobileReactor,
                                                 groupMobileShieldGenerator,
                                                 groupMobileStorage,
                                                 groupMoonMining,
                                                 groupRefiningArray,
                                                 groupScannerArray,
                                                 groupSensorDampeningBattery,
                                                 groupShieldHardeningArray,
                                                 groupShipMaintenanceArray,
                                                 groupSilo,
                                                 groupStasisWebificationBattery,
                                                 groupStealthEmitterArray,
                                                 groupTrackingArray,
                                                 groupWarpScramblingBattery,
                                                 groupCynosuralSystemJammer,
                                                 groupCynosuralGeneratorArray,
                                                 groupInfrastructureHub,
                                                 groupSovereigntyClaimMarkers,
                                                 groupSovereigntyDisruptionStructures,
                                                 groupOrbitalConstructionPlatforms,
                                                 groupPlanetaryCustomsOffices])
 probeScanGroups[probeScanGroupDronesAndProbes] = set([groupCapDrainDrone,
                                                      groupCombatDrone,
                                                      groupElectronicWarfareDrone,
                                                      groupFighterDrone,
                                                      groupFighterBomber,
                                                      groupLogisticDrone,
                                                      groupMiningDrone,
                                                      groupProximityDrone,
                                                      groupRepairDrone,
                                                      groupStasisWebifyingDrone,
                                                      groupUnanchoringDrone,
                                                      groupWarpScramblingDrone,
                                                      groupScannerProbe,
                                                      groupSurveyProbe,
                                                      groupWarpDisruptionProbe])
 probeScanGroups[probeScanGroupCelestials] = set([groupAsteroidBelt,
                                                 groupForceField,
                                                 groupMoon,
                                                 groupPlanet,
                                                 groupStargate,
                                                 groupSun,
                                                 groupStation])
 */


// this really doesnt belong here, but dont know where else to put it yet.
namespace WormHole {
    namespace Class {
        enum {
            Unknown1    = 0,
            Unknown2    = 1,
            Unknown3    = 2,
            Unknown4    = 3,
            Dangerous1  = 4,
            Dangerous2  = 5,
            Deadly      = 6,
            HiSec       = 7,
            LoSec       = 8,
            NullSec     = 9
        };
    }

    namespace Age {
        enum {
            New = 0,
            Adolescent = 1,
            Decaying = 2,
            Closing = 3
        };
    }

    namespace Size {
        // these are fuzzy logic
        enum {
            Full = 10,
            Reduced = 5,
            Disrupted = 1
        };
    }
}


#endif  // EVE_SCANNING_H