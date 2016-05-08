/*
 *
 *
 *
 */


#ifndef EVE_SKILLS_H
#define EVE_SKILLS_H

typedef enum {
    skillEventGift                          = 24,
    skillEventCharCreation                  = 33,
    skillEventClonePenalty                  = 34,
    skillEventTaskMaster                    = 35,
    skillEventTrainingStarted               = 36,
    skillEventTrainingComplete              = 37,
    skillEventTrainingCancelled             = 38,
    skillEventGMGive                        = 39,
    skillEventQueueTrainingCompleted        = 53,
    skillEventSkillInjected                 = 56,
    skillEventRemoval                       = 177,
    /** @todo cant use these below, as current eventID is uint8 */
    skillEventHaltedAccountLapsed           = 260,
    skillEventSkillPointsApplied            = 307,
    skillEventGMReverseFreeSkillPointsUsed  = 309
} EVESkillEvent;

 /** Created with MYSQL query:
  * SELECT concat("\tskill", Replace(t.typeName, ' ', ''), "\t\t= ", t.typeID, ',\t\t// group = ', g.groupName)
  * FROM invTypes t, invGroups g, invCategories c
  * WHERE g.groupID = t.groupID AND c.categoryID = g.categoryID AND c.categoryID = 16
  * ORDER BY g.groupName
  */
 //  -allan 21Mar14
 typedef enum {  //thanks positron96 for this query
     skillCorporationManagement      = 3363,     // group = Corporation Management
     skillStationManagement      = 3364,     // group = Corporation Management
     skillStarbaseManagement     = 3365,     // group = Corporation Management
     skillFactoryManagement      = 3366,     // group = Corporation Management
     skillRefineryManagement     = 3367,     // group = Corporation Management
     skillEthnicRelations        = 3368,     // group = Corporation Management
     skillCFOTraining        = 3369,     // group = Corporation Management
     skillChiefScienceOfficer        = 3370,     // group = Corporation Management
     skillPublicRelations        = 3371,     // group = Corporation Management
     skillIntelligenceAnalyst        = 3372,     // group = Corporation Management
     skillStarbaseDefenseManagement      = 3373,     // group = Corporation Management
     skillMegacorpManagement     = 3731,     // group = Corporation Management
     skillEmpireControl      = 3732,     // group = Corporation Management
     skillAnchoring      = 11584,        // group = Corporation Management
     skillSovereignty        = 12241,        // group = Corporation Management
     skillDrones     = 3436,     // group = Drones
     skillScoutDroneOperation        = 3437,     // group = Drones
     skillMiningDroneOperation       = 3438,     // group = Drones
     skillRepairDroneOperation       = 3439,     // group = Drones
     skillSalvageDroneOperation      = 3440,     // group = Drones
     skillHeavyDroneOperation        = 3441,     // group = Drones
     skillDroneInterfacing       = 3442,     // group = Drones
     skillDroneNavigation        = 12305,        // group = Drones
     skillAmarrDroneSpecialization       = 12484,        // group = Drones
     skillMinmatarDroneSpecialization        = 12485,        // group = Drones
     skillGallenteDroneSpecialization        = 12486,        // group = Drones
     skillCaldariDroneSpecialization     = 12487,        // group = Drones
     skillTESTDroneSkill     = 22172,        // group = Drones
     skillMiningDroneSpecialization      = 22541,        // group = Drones
     skillFighters       = 23069,        // group = Drones
     skillElectronicWarfareDroneInterfacing      = 23566,        // group = Drones
     skillSentryDroneInterfacing     = 23594,        // group = Drones
     skillPropulsionJammingDroneInterfacing      = 23599,        // group = Drones
     skillDroneSharpshooting     = 23606,        // group = Drones
     skillDroneDurability        = 23618,        // group = Drones
     skillCombatDroneOperation       = 24241,        // group = Drones
     skillAdvancedDroneInterfacing       = 24613,        // group = Drones
     skillFighterBombers     = 32339,        // group = Drones
     skillElectronics        = 3426,     // group = Electronics
     skillElectronicWarfare      = 3427,     // group = Electronics
     skillLongRangeTargeting     = 3428,     // group = Electronics
     skillTargeting      = 3429,     // group = Electronics
     skillMultitasking       = 3430,     // group = Electronics
     skillSignatureAnalysis      = 3431,     // group = Electronics
     skillElectronicsUpgrades        = 3432,     // group = Electronics
     skillSensorLinking      = 3433,     // group = Electronics
     skillWeaponDisruption       = 3434,     // group = Electronics
     skillPropulsionJamming      = 3435,     // group = Electronics
     skillSurvey     = 3551,     // group = Electronics
     skillAdvancedSensorUpgrades     = 11208,        // group = Electronics
     skillCloaking       = 11579,        // group = Electronics
     skillHypereuclideanNavigation       = 12368,        // group = Electronics
     skillLongDistanceJamming        = 19759,        // group = Electronics
     skillFrequencyModulation        = 19760,        // group = Electronics
     skillSignalDispersion       = 19761,        // group = Electronics
     skillSignalSuppression      = 19766,        // group = Electronics
     skillTurretDestabilization      = 19767,        // group = Electronics
     skillTargetPainting     = 19921,        // group = Electronics
     skillSignatureFocusing      = 19922,        // group = Electronics
     skillCynosuralFieldTheory       = 21603,        // group = Electronics
     skillProjectedElectronicCounterMeasures     = 27911,        // group = Electronics
     skillTournamentObservation      = 28604,        // group = Electronics
     skillImperialNavySecurityClearance      = 28631,        // group = Electronics
     skillEngineering        = 3413,     // group = Engineering
     skillShieldOperation        = 3416,     // group = Engineering
     skillEnergySystemsOperation     = 3417,     // group = Engineering
     skillEnergyManagement       = 3418,     // group = Engineering
     skillShieldManagement       = 3419,     // group = Engineering
     skillTacticalShieldManipulation     = 3420,     // group = Engineering
     skillEnergyPulseWeapons     = 3421,     // group = Engineering
     skillShieldEmissionSystems      = 3422,     // group = Engineering
     skillEnergyEmissionSystems      = 3423,     // group = Engineering
     skillEnergyGridUpgrades     = 3424,     // group = Engineering
     skillShieldUpgrades     = 3425,     // group = Engineering
     skillAdvancedEnergyGridUpgrades     = 11204,        // group = Engineering
     skillAdvancedShieldUpgrades     = 11206,        // group = Engineering
     skillThermicShieldCompensation      = 11566,        // group = Engineering
     skillEMShieldCompensation       = 12365,        // group = Engineering
     skillKineticShieldCompensation      = 12366,        // group = Engineering
     skillExplosiveShieldCompensation        = 12367,        // group = Engineering
     skillShieldCompensation     = 21059,        // group = Engineering
     skillCapitalShieldOperation     = 21802,        // group = Engineering
     skillCapitalShieldEmissionSystems       = 24571,        // group = Engineering
     skillCapitalEnergyEmissionSystems       = 24572,        // group = Engineering
     skillStealthBombersFakeSkill        = 20127,        // group = Fake Skills
     skillGunnery        = 3300,     // group = Gunnery
     skillSmallHybridTurret      = 3301,     // group = Gunnery
     skillSmallProjectileTurret      = 3302,     // group = Gunnery
     skillSmallEnergyTurret      = 3303,     // group = Gunnery
     skillMediumHybridTurret     = 3304,     // group = Gunnery
     skillMediumProjectileTurret     = 3305,     // group = Gunnery
     skillMediumEnergyTurret     = 3306,     // group = Gunnery
     skillLargeHybridTurret      = 3307,     // group = Gunnery
     skillLargeProjectileTurret      = 3308,     // group = Gunnery
     skillLargeEnergyTurret      = 3309,     // group = Gunnery
     skillRapidFiring        = 3310,     // group = Gunnery
     skillSharpshooter       = 3311,     // group = Gunnery
     skillMotionPrediction       = 3312,     // group = Gunnery
     skillSurgicalStrike     = 3315,     // group = Gunnery
     skillControlledBursts       = 3316,     // group = Gunnery
     skillTrajectoryAnalysis     = 3317,     // group = Gunnery
     skillWeaponUpgrades     = 3318,     // group = Gunnery
     skillSmallRailgunSpecialization     = 11082,        // group = Gunnery
     skillSmallBeamLaserSpecialization       = 11083,        // group = Gunnery
     skillSmallAutocannonSpecialization      = 11084,        // group = Gunnery
     skillAdvancedWeaponUpgrades     = 11207,        // group = Gunnery
     skillSmallArtillerySpecialization       = 12201,        // group = Gunnery
     skillMediumArtillerySpecialization      = 12202,        // group = Gunnery
     skillLargeArtillerySpecialization       = 12203,        // group = Gunnery
     skillMediumBeamLaserSpecialization      = 12204,        // group = Gunnery
     skillLargeBeamLaserSpecialization       = 12205,        // group = Gunnery
     skillMediumRailgunSpecialization        = 12206,        // group = Gunnery
     skillLargeRailgunSpecialization     = 12207,        // group = Gunnery
     skillMediumAutocannonSpecialization     = 12208,        // group = Gunnery
     skillLargeAutocannonSpecialization      = 12209,        // group = Gunnery
     skillSmallBlasterSpecialization     = 12210,        // group = Gunnery
     skillMediumBlasterSpecialization        = 12211,        // group = Gunnery
     skillLargeBlasterSpecialization     = 12212,        // group = Gunnery
     skillSmallPulseLaserSpecialization      = 12213,        // group = Gunnery
     skillMediumPulseLaserSpecialization     = 12214,        // group = Gunnery
     skillLargePulseLaserSpecialization      = 12215,        // group = Gunnery
     skillCapitalEnergyTurret        = 20327,        // group = Gunnery
     skillCapitalHybridTurret        = 21666,        // group = Gunnery
     skillCapitalProjectileTurret        = 21667,        // group = Gunnery
     skillTacticalWeaponReconfiguration      = 22043,        // group = Gunnery
     skillIndustry       = 3380,     // group = Industry
     skillAmarrTech      = 3381,     // group = Industry
     skillCaldariTech        = 3382,     // group = Industry
     skillGallenteTech       = 3383,     // group = Industry
     skillMinmatarTech       = 3384,     // group = Industry
     skillRefining       = 3385,     // group = Industry
     skillMining     = 3386,     // group = Industry
     skillMassProduction     = 3387,     // group = Industry
     skillProductionEfficiency       = 3388,     // group = Industry
     skillRefineryEfficiency     = 3389,     // group = Industry
     skillMobileRefineryOperation        = 3390,     // group = Industry
     skillMobileFactoryOperation     = 3391,     // group = Industry
     skillDeepCoreMining     = 11395,        // group = Industry
     skillArkonorProcessing      = 12180,        // group = Industry
     skillBistotProcessing       = 12181,        // group = Industry
     skillCrokiteProcessing      = 12182,        // group = Industry
     skillDarkOchreProcessing        = 12183,        // group = Industry
     skillGneissProcessing       = 12184,        // group = Industry
     skillHedbergiteProcessing       = 12185,        // group = Industry
     skillHemorphiteProcessing       = 12186,        // group = Industry
     skillJaspetProcessing       = 12187,        // group = Industry
     skillKerniteProcessing      = 12188,        // group = Industry
     skillMercoxitProcessing     = 12189,        // group = Industry
     skillOmberProcessing        = 12190,        // group = Industry
     skillPlagioclaseProcessing      = 12191,        // group = Industry
     skillPyroxeresProcessing        = 12192,        // group = Industry
     skillScorditeProcessing     = 12193,        // group = Industry
     skillSpodumainProcessing        = 12194,        // group = Industry
     skillVeldsparProcessing     = 12195,        // group = Industry
     skillScrapmetalProcessing       = 12196,        // group = Industry
     skillIceHarvesting      = 16281,        // group = Industry
     skillIceProcessing      = 18025,        // group = Industry
     skillMiningUpgrades     = 22578,        // group = Industry
     skillSupplyChainManagement      = 24268,        // group = Industry
     skillAdvancedMassProduction     = 24625,        // group = Industry
     skillGasCloudHarvesting     = 25544,        // group = Industry
     skillDrugManufacturing      = 26224,        // group = Industry
     skillOreCompression     = 28373,        // group = Industry
     skillIndustrialReconfiguration      = 28585,        // group = Industry
     skillLeadership     = 3348,     // group = Leadership
     skillSkirmishWarfare        = 3349,     // group = Leadership
     skillSiegeWarfare       = 3350,     // group = Leadership
     skillSiegeWarfareSpecialist     = 3351,     // group = Leadership
     skillInformationWarfareSpecialist       = 3352,     // group = Leadership
     skillWarfareLinkSpecialist      = 3354,     // group = Leadership
     skillArmoredWarfareSpecialist       = 11569,        // group = Leadership
     skillSkirmishWarfareSpecialist      = 11572,        // group = Leadership
     skillWingCommand        = 11574,        // group = Leadership
     skillArmoredWarfare     = 20494,        // group = Leadership
     skillInformationWarfare     = 20495,        // group = Leadership
     skillMiningForeman      = 22536,        // group = Leadership
     skillMiningDirector     = 22552,        // group = Leadership
     skillFleetCommand       = 24764,        // group = Leadership
     skillMechanics      = 3392,     // group = Mechanics
     skillRepairSystems      = 3393,     // group = Mechanics
     skillHullUpgrades       = 3394,     // group = Mechanics
     skillFrigateConstruction        = 3395,     // group = Mechanics
     skillIndustrialConstruction     = 3396,     // group = Mechanics
     skillCruiserConstruction        = 3397,     // group = Mechanics
     skillBattleshipConstruction     = 3398,     // group = Mechanics
     skillOutpostConstruction        = 3400,     // group = Mechanics
     skillRemoteArmorRepairSystems       = 16069,        // group = Mechanics
     skillCapitalRepairSystems       = 21803,        // group = Mechanics
     skillCapitalShipConstruction        = 22242,        // group = Mechanics
     skillEMArmorCompensation        = 22806,        // group = Mechanics
     skillExplosiveArmorCompensation     = 22807,        // group = Mechanics
     skillKineticArmorCompensation       = 22808,        // group = Mechanics
     skillThermicArmorCompensation       = 22809,        // group = Mechanics
     skillCapitalRemoteArmorRepairSystems        = 24568,        // group = Mechanics
     skillSalvaging      = 25863,        // group = Mechanics
     skillJuryRigging        = 26252,        // group = Mechanics
     skillArmorRigging       = 26253,        // group = Mechanics
     skillAstronauticsRigging        = 26254,        // group = Mechanics
     skillDronesRigging      = 26255,        // group = Mechanics
     skillElectronicSuperiorityRigging       = 26256,        // group = Mechanics
     skillProjectileWeaponRigging        = 26257,        // group = Mechanics
     skillEnergyWeaponRigging        = 26258,        // group = Mechanics
     skillHybridWeaponRigging        = 26259,        // group = Mechanics
     skillLauncherRigging        = 26260,        // group = Mechanics
     skillShieldRigging      = 26261,        // group = Mechanics
     skillRemoteHullRepairSystems        = 27902,        // group = Mechanics
     skillTacticalLogisticsReconfiguration       = 27906,        // group = Mechanics
     skillCapitalRemoteHullRepairSystems     = 27936,        // group = Mechanics
     skillNaniteOperation        = 28879,        // group = Mechanics
     skillNaniteInterfacing      = 28880,        // group = Mechanics
     skillMissileLauncherOperation       = 3319,     // group = Missile Launcher Operation
     skillRockets        = 3320,     // group = Missile Launcher Operation
     skillLightMissiles       = 3321,     // group = Missile Launcher Operation
     skillFoFMissiles        = 3322,     // group = Missile Launcher Operation
     skillDefenderMissiles       = 3323,     // group = Missile Launcher Operation
     skillHeavyMissiles      = 3324,     // group = Missile Launcher Operation
     skillTorpedoes      = 3325,     // group = Missile Launcher Operation
     skillCruiseMissiles     = 3326,     // group = Missile Launcher Operation
     skillMissileBombardment     = 12441,        // group = Missile Launcher Operation
     skillMissileProjection      = 12442,        // group = Missile Launcher Operation
     skillRocketSpecialization       = 20209,        // group = Missile Launcher Operation
     skillLightMissileSpecialization      = 20210,        // group = Missile Launcher Operation
     skillHeavyMissileSpecialization     = 20211,        // group = Missile Launcher Operation
     skillCruiseMissileSpecialization        = 20212,        // group = Missile Launcher Operation
     skillTorpedoSpecialization      = 20213,        // group = Missile Launcher Operation
     skillGuidedMissilePrecision     = 20312,        // group = Missile Launcher Operation
     skillTargetNavigationPrediction     = 20314,        // group = Missile Launcher Operation
     skillWarheadUpgrades        = 20315,        // group = Missile Launcher Operation
     skillRapidLaunch        = 21071,        // group = Missile Launcher Operation
     skillCitadelTorpedoes       = 21668,        // group = Missile Launcher Operation
     skillHeavyAssaultMissileSpecialization      = 25718,        // group = Missile Launcher Operation
     skillHeavyAssaultMissiles       = 25719,        // group = Missile Launcher Operation
     skillBombDeployment     = 28073,        // group = Missile Launcher Operation
     skillCitadelCruiseMissiles      = 32435,        // group = Missile Launcher Operation
     skillNavigation     = 3449,     // group = Navigation
     skillAfterburner        = 3450,     // group = Navigation
     skillFuelConservation       = 3451,     // group = Navigation
     skillAccelerationControl        = 3452,     // group = Navigation
     skillEvasiveManeuvering     = 3453,     // group = Navigation
     skillHighSpeedManeuvering       = 3454,     // group = Navigation
     skillWarpDriveOperation     = 3455,     // group = Navigation
     skillJumpDriveOperation     = 3456,     // group = Navigation
     skillJumpFuelConservation       = 21610,        // group = Navigation
     skillJumpDriveCalibration       = 21611,        // group = Navigation
     skillAdvancedPlanetology        = 2403,     // group = Planet Management
     skillPlanetology        = 2406,     // group = Planet Management
     skillInterplanetaryConsolidation        = 2495,     // group = Planet Management
     skillCommandCenterUpgrades      = 2505,     // group = Planet Management
     skillRemoteSensing      = 13279,        // group = Planet Management
     skillScience        = 3402,     // group = Science
     skillResearch       = 3403,     // group = Science
     skillBiology        = 3405,     // group = Science
     skillLaboratoryOperation        = 3406,     // group = Science
     skillReverseEngineering     = 3408,     // group = Science
     skillMetallurgy     = 3409,     // group = Science
     skillAstrogeology       = 3410,     // group = Science
     skillCybernetics        = 3411,     // group = Science
     skillAstrometrics       = 3412,     // group = Science
     skillHighEnergyPhysics      = 11433,        // group = Science
     skillPlasmaPhysics      = 11441,        // group = Science
     skillNaniteEngineering      = 11442,        // group = Science
     skillHydromagneticPhysics       = 11443,        // group = Science
     skillAmarrianStarshipEngineering        = 11444,        // group = Science
     skillMinmatarStarshipEngineering        = 11445,        // group = Science
     skillGravitonPhysics        = 11446,        // group = Science
     skillLaserPhysics       = 11447,        // group = Science
     skillElectromagneticPhysics     = 11448,        // group = Science
     skillRocketScience      = 11449,        // group = Science
     skillGallenteanStarshipEngineering      = 11450,        // group = Science
     skillNuclearPhysics     = 11451,        // group = Science
     skillMechanicalEngineering      = 11452,        // group = Science
     skillElectronicEngineering      = 11453,        // group = Science
     skillCaldariStarshipEngineering     = 11454,        // group = Science
     skillQuantumPhysics     = 11455,        // group = Science
     skillAstronauticEngineering     = 11487,        // group = Science
     skillMolecularEngineering       = 11529,        // group = Science
     skillHypernetScience        = 11858,        // group = Science
     skillResearchProjectManagement      = 12179,        // group = Science
     skillArchaeology        = 13278,        // group = Science
     skillTalocanTechnology      = 20433,        // group = Science
     skillHacking        = 21718,        // group = Science
     skillSleeperTechnology      = 21789,        // group = Science
     skillCaldariEncryptionMethods       = 21790,        // group = Science
     skillMinmatarEncryptionMethods      = 21791,        // group = Science
     skillAmarrEncryptionMethods     = 23087,        // group = Science
     skillGallenteEncryptionMethods      = 23121,        // group = Science
     skillTakmahlTechnology      = 23123,        // group = Science
     skillYanJungTechnology      = 23124,        // group = Science
     skillInfomorphPsychology        = 24242,        // group = Science
     skillScientificNetworking       = 24270,        // group = Science
     skillJumpPortalGeneration       = 24562,        // group = Science
     skillDoomsdayOperation      = 24563,        // group = Science
     skillCloningFacilityOperation       = 24606,        // group = Science
     skillAdvancedLaboratoryOperation        = 24624,        // group = Science
     skillNeurotoxinRecovery     = 25530,        // group = Science
     skillNaniteControl      = 25538,        // group = Science
     skillAstrometricRangefinding        = 25739,        // group = Science
     skillAstrometricPinpointing     = 25810,        // group = Science
     skillAstrometricAcquisition     = 25811,        // group = Science
     skillThermodynamics     = 28164,        // group = Science
     skillDefensiveSubsystemTechnology       = 30324,        // group = Science
     skillEngineeringSubsystemTechnology     = 30325,        // group = Science
     skillElectronicSubsystemTechnology      = 30326,        // group = Science
     skillOffensiveSubsystemTechnology       = 30327,        // group = Science
     skillPropulsionSubsystemTechnology      = 30788,        // group = Science
     skillSocial     = 3355,     // group = Social
     skillNegotiation        = 3356,     // group = Social
     skillDiplomacy      = 3357,     // group = Social
     skillFastTalk       = 3358,     // group = Social
     skillConnections        = 3359,     // group = Social
     skillCriminalConnections        = 3361,     // group = Social
     skillDEDConnections     = 3362,     // group = Social
     skillMiningConnections      = 3893,     // group = Social
     skillDistributionConnections        = 3894,     // group = Social
     skillSecurityConnections        = 3895,     // group = Social
     skillOREIndustrial      = 3184,     // group = Spaceship Command
     skillSpaceshipCommand       = 3327,     // group = Spaceship Command
     skillGallenteFrigate        = 3328,     // group = Spaceship Command
     skillMinmatarFrigate        = 3329,     // group = Spaceship Command
     skillCaldariFrigate     = 3330,     // group = Spaceship Command
     skillAmarrFrigate       = 3331,     // group = Spaceship Command
     skillGallenteCruiser        = 3332,     // group = Spaceship Command
     skillMinmatarCruiser        = 3333,     // group = Spaceship Command
     skillCaldariCruiser     = 3334,     // group = Spaceship Command
     skillAmarrCruiser       = 3335,     // group = Spaceship Command
     skillGallenteBattleship     = 3336,     // group = Spaceship Command
     skillMinmatarBattleship     = 3337,     // group = Spaceship Command
     skillCaldariBattleship      = 3338,     // group = Spaceship Command
     skillAmarrBattleship        = 3339,     // group = Spaceship Command
     skillGallenteIndustrial     = 3340,     // group = Spaceship Command
     skillMinmatarIndustrial     = 3341,     // group = Spaceship Command
     skillCaldariIndustrial      = 3342,     // group = Spaceship Command
     skillAmarrIndustrial        = 3343,     // group = Spaceship Command
     skillGallenteTitan      = 3344,     // group = Spaceship Command
     skillMinmatarTitan      = 3345,     // group = Spaceship Command
     skillCaldariTitan       = 3346,     // group = Spaceship Command
     skillAmarrTitan     = 3347,     // group = Spaceship Command
     skillJoveFrigate        = 3755,     // group = Spaceship Command
     skillJoveCruiser        = 3758,     // group = Spaceship Command
     skillPolaris        = 9955,     // group = Spaceship Command
     skillConcord        = 10264,        // group = Spaceship Command
     skillJoveIndustrial     = 11075,        // group = Spaceship Command
     skillJoveBattleship     = 11078,        // group = Spaceship Command
     skillInterceptors       = 12092,        // group = Spaceship Command
     skillCovertOps      = 12093,        // group = Spaceship Command
     skillAssaultShips       = 12095,        // group = Spaceship Command
     skillLogistics      = 12096,        // group = Spaceship Command
     skillDestroyers     = 12097,        // group = Spaceship Command
     skillInterdictors       = 12098,        // group = Spaceship Command
     skillBattlecruisers     = 12099,        // group = Spaceship Command
     skillHeavyAssaultShips      = 16591,        // group = Spaceship Command
     skillMiningBarge        = 17940,        // group = Spaceship Command
     skillOmnipotent     = 19430,        // group = Spaceship Command
     skillTransportShips     = 19719,        // group = Spaceship Command
     skillAdvancedSpaceshipCommand       = 20342,        // group = Spaceship Command
     skillAmarrFreighter     = 20524,        // group = Spaceship Command
     skillAmarrDreadnought       = 20525,        // group = Spaceship Command
     skillCaldariFreighter       = 20526,        // group = Spaceship Command
     skillGallenteFreighter      = 20527,        // group = Spaceship Command
     skillMinmatarFreighter      = 20528,        // group = Spaceship Command
     skillCaldariDreadnought     = 20530,        // group = Spaceship Command
     skillGallenteDreadnought        = 20531,        // group = Spaceship Command
     skillMinmatarDreadnought        = 20532,        // group = Spaceship Command
     skillCapitalShips       = 20533,        // group = Spaceship Command
     skillExhumers       = 22551,        // group = Spaceship Command
     skillReconShips     = 22761,        // group = Spaceship Command
     skillCommandShips       = 23950,        // group = Spaceship Command
     skillAmarrCarrier       = 24311,        // group = Spaceship Command
     skillCaldariCarrier     = 24312,        // group = Spaceship Command
     skillGallenteCarrier        = 24313,        // group = Spaceship Command
     skillMinmatarCarrier        = 24314,        // group = Spaceship Command
     skillCapitalIndustrialShips     = 28374,        // group = Spaceship Command
     skillHeavyInterdictors      = 28609,        // group = Spaceship Command
     skillElectronicAttackShips      = 28615,        // group = Spaceship Command
     skillBlackOps       = 28656,        // group = Spaceship Command
     skillMarauders      = 28667,        // group = Spaceship Command
     skillJumpFreighters     = 29029,        // group = Spaceship Command
     skillIndustrialCommandShips     = 29637,        // group = Spaceship Command
     skillAmarrStrategicCruiser      = 30650,        // group = Spaceship Command
     skillCaldariStrategicCruiser        = 30651,        // group = Spaceship Command
     skillGallenteStrategicCruiser       = 30652,        // group = Spaceship Command
     skillMinmatarStrategicCruiser       = 30653,        // group = Spaceship Command
     skillAmarrDefensiveSystems      = 30532,        // group = Subsystems
     skillAmarrElectronicSystems     = 30536,        // group = Subsystems
     skillAmarrOffensiveSystems      = 30537,        // group = Subsystems
     skillAmarrPropulsionSystems     = 30538,        // group = Subsystems
     skillAmarrEngineeringSystems        = 30539,        // group = Subsystems
     skillGallenteDefensiveSystems       = 30540,        // group = Subsystems
     skillGallenteElectronicSystems      = 30541,        // group = Subsystems
     skillCaldariElectronicSystems       = 30542,        // group = Subsystems
     skillMinmatarElectronicSystems      = 30543,        // group = Subsystems
     skillCaldariDefensiveSystems        = 30544,        // group = Subsystems
     skillMinmatarDefensiveSystems       = 30545,        // group = Subsystems
     skillGallenteEngineeringSystems     = 30546,        // group = Subsystems
     skillMinmatarEngineeringSystems     = 30547,        // group = Subsystems
     skillCaldariEngineeringSystems      = 30548,        // group = Subsystems
     skillCaldariOffensiveSystems        = 30549,        // group = Subsystems
     skillGallenteOffensiveSystems       = 30550,        // group = Subsystems
     skillMinmatarOffensiveSystems       = 30551,        // group = Subsystems
     skillCaldariPropulsionSystems       = 30552,        // group = Subsystems
     skillGallentePropulsionSystems      = 30553,        // group = Subsystems
     skillMinmatarPropulsionSystems      = 30554,        // group = Subsystems
     skillTrade      = 3443,     // group = Trade
     skillRetail     = 3444,     // group = Trade
     skillBlackMarketTrading     = 3445,     // group = Trade
     skillBrokerRelations        = 3446,     // group = Trade
     skillVisibility     = 3447,     // group = Trade
     skillSmuggling      = 3448,     // group = Trade
     skillTest       = 11015,        // group = Trade
     skillGeneralFreight     = 12834,        // group = Trade
     skillStarshipFreight        = 13069,        // group = Trade
     skillMineralFreight     = 13070,        // group = Trade
     skillMunitionsFreight       = 13071,        // group = Trade
     skillDroneFreight       = 13072,        // group = Trade
     skillRawMaterialFreight     = 13073,        // group = Trade
     skillConsumableFreight      = 13074,        // group = Trade
     skillHazardousMaterialFreight       = 13075,        // group = Trade
     skillProcurement        = 16594,        // group = Trade
     skillDaytrading     = 16595,        // group = Trade
     skillWholesale      = 16596,        // group = Trade
     skillMarginTrading      = 16597,        // group = Trade
     skillMarketing      = 16598,        // group = Trade
     skillAccounting     = 16622,        // group = Trade
     skillTycoon     = 18580,        // group = Trade
     skillCorporationContracting     = 25233,        // group = Trade
     skillContracting        = 25235,        // group = Trade
     skillTaxEvasion     = 28261     // group = Trade
 } EVESkillID;

 #endif  //EVE_SKILLS_H