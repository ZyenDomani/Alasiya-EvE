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

namespace EVESCAN {
//  -allan 7Jul14
enum ScanGroup {
    ScanGroupScrap         = 1,
    ScanGroupSignature     = 4,
    ScanGroupShip          = 8,
    ScanGroupStructure     = 16,
    ScanGroupDroneOrProbe  = 32,
    ScanGroupCelestial     = 64,
    ScanGroupAnomaly       = 128
} ;


enum ProbeState {
    ProbeStateInactive     = 0,
    ProbeStateIdle         = 1,
    ProbeStateMoving       = 2,
    ProbeStateWarping      = 3,
    ProbeStateScanning     = 4,
    ProbeStateReturning    = 5
};

}
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