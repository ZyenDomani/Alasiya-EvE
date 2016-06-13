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

//  -allan 7Jul14
typedef enum {
    ProbeGroup_Scrap                = 1,
    ProbeGroup_Signatures           = 4,
    ProbeGroup_Ships                = 8,
    ProbeGroup_Structures           = 16,
    ProbeGroup_DronesAndProbes      = 32,
    ProbeGroup_Celestials           = 64,
    ProbeGroup_Anomalies            = 128
} ProbeScanGroup;


typedef enum {
    Probe_Inactive                  = 0,
    Probe_Idle                      = 1,
    Probe_Moving                    = 2,
    Probe_Warping                   = 3,
    Probe_Scanning                  = 4,
    Probe_Returning                 = 5
} ProbeState;

/*

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

#endif  // EVE_SCANNING_H