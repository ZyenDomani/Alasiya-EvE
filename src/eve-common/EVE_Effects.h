/*
 *      this file is for cross referencing effect data.
 *      there are also checks where i use these fxIDs
 *
 */


#ifndef EVE_EFFECTS_H
#define EVE_EFFECTS_H


/** updated list from current server data...
 *  allan - 21 March 2017
 *  verified again - 3Feb23
 */

namespace EvE {
    namespace GFXID {

enum {
    shieldBoosting =   4,     // effects.ShieldBoosting
    missileLaunching =   9,     // effects.MissileDeployment
    targetAttack =   10,     // effects.Laser   ** this gfx is short bursts from module, with long pauses.  repeat=false
    loPower = 11,
    hiPower = 12,
    medPower = 13,
    Online = 16,
    mining = 17,     // effects.Mining    ** same gfx as effects.Laser
    shieldTransfer =   18,     // effects.ShieldTransfer
    structureRepair =   26,     // effects.StructureRepair
    armorRepair =   27,     // effects.ArmorRepair
    modifyTargetSpeed =   29,     // effects.ModifyTargetSpeed
    energyTransfer =   31,     // effects.EnergyTransfer
    energyVampire =   32,     // effects.EnergyDestabilization
    projectileFired =   34,     // effects.ProjectileFired
    energyDestabilization =   36,     // effects.EnergyDestabilization
    empWave =   38,     // effects.EMPWave
    warpScramble =   39,     // effects.WarpScramble
    launcherFitted = 40,
    turretFitted  = 42,
    cargoScan =   47,     // effects.CargoScan
    ecmBurst =   53,     // effects.ECMBurst
    miningLaser =   67,     // effects.miningLaser  ** this gfx is a long burst from module, with short pauses
    surveyScan =   81,     // effects.SurveyScan
    useMissiles = 101,          // not actually an effect, but means "use charge's effectID"
    fofMissileLaunching =   104,     // effects.MissileDeployment
    turretWeaponRangeTrackingSpeedMultiplyActivate =   123,     // effects.TurretWeaponRangeTrackingSpeedMultiplyActivate
    scanStrengthBonusTarget =   124,     // effects.ScanStrengthBonusTarget
    turretWeaponRangeTrackingSpeedMultiplyTarget =   126,     // effects.TurretWeaponRangeTrackingSpeedMultiplyTarget
    torpedoLaunching =   127,     // effects.TorpedoDeployment
    skillEffect = 132,
    barrage =   263,     // effects.Barrage
    warpScrambleForEntity =   563,     // effects.WarpScramble
    missileLaunchingForEntity =   569,     // effects.MissileDeployment
    modifyTargetSpeed2 =   575,     // effects.ModifyTargetSpeed
    decreaseTargetSpeed =   586,     // effects.ModifyTargetSpeed
    targetArmorRepair =   592,     // effects.RemoteArmourRepair
    targetedEMResonanceMultiply =   597,     // effects.EnergyDestabilization
    cloaking =   607,     // effects.Cloaking
    turretWeaponRangeTrackingSpeedMultiplyTargetHostile =   609,     // effects.ElectronicAttributeModifyTarget
    targetedKineticResonanceMultiply =   615,     // effects.EnergyDestabilization
    targetedThermalResonanceMultiply =   616,     // effects.EnergyDestabilization
    targetedExplosiveResonanceMultiply =   617,     // effects.EnergyDestabilization
    anchorDrop =   649,     // effects.AnchorDrop
    anchorLift =   650,     // effects.AnchorLift
    stealthActive =   713,     // effects.ElectronicAttributeModifyActivate
    sensorBoostTargeted =   716,     // effects.ElectronicAttributeModifyTarget
    sensorBoosterActive =   720,     // effects.ElectronicAttributeModifyActivate
    gallenteFrigateSkillLevelPreMulShipBonusGF2Ship =   751,     // gallenteFrigateSkillLevelPreMulShipBonusGF2Ship
    shipHybridDamageBonusCF =   754,     // shipHybridDamageBonusCF
    shipETDamageAF =   757,     // shipETDamageAF
    shipMissileSpeedBonusCF =   760,     // shipMissileSpeedBonusCF
    caldariBattleshipSkillLevelPreMulShipBonusCB3Ship =   797,     // caldariBattleshipSkillLevelPreMulShipBonusCB3Ship
    sensorBoostTargetedHostile =   837,     // effects.ElectronicAttributeModifyTarget
    shieldBoostingForEntities =   876,     // effects.ShieldBoosting
    armorRepairForEntities =   878,     // effects.ArmorRepair
    suicideBomb =   885,     // effects.EMPWave
    onlineForStructures =   901,     // effects.StructureOnline
    decloakWave =   902,     // effects.DecloakWave             -- this isnt gfx, but module effect radiating from origin
    cloakingWarpSafe =   980,     // effects.CloakingCovertOps       -- not in Crucible
    anchorDropForStructures =   1022,     // effects.AnchorDrop
    anchorLiftForStructures =   1023,     // effects.AnchorLift
    projectileFiredForEntities =   1086,     // effects.ProjectileFiredForEntities
    targetAttackForStructures =   1199,     // effects.ProjectileFiredForEntities
    speedBoostMassAddition =   1253,     // effects.Afterburner
    speedBoostMassSigRad =   1254,     // effects.MicroWarpDrive
    newEwTestscanStrengthBonusTargetHostile =   1271,     // effects.ScanStrengthBonusTarget
    newEwTestswarpScramble =   1272,     // effects.WarpScramble
    newEwTestssensorBoostTargetedHostile =   1273,     // effects.ElectronicAttributeModifyTarget
    newEwTeststurretWeaponRangeTrackingSpeedMultiplyTargetHostile =   1274,     // effects.TargetPaint
    newEwTestsdecreaseTargetSpeed =   1275,     // effects.ModifyTargetSpeed
    ewTestEffectRsd =   1354,     // effects.ElectronicAttributeModifyTarget
    ewTestEffectWs =   1355,     // effects.WarpScramble
    ewTestEffectJam =   1358,     // effects.ElectronicAttributeModifyTarget
    gangBonusSignature =   1411,     // effects.ElectronicAttributeModifyActivate
    gangArmorHardening =   1510,     // effects.ElectronicAttributeModifyActivate
    gangPropulsionJammingBoost =   1546,     // effects.ElectronicAttributeModifyActivate
    gangShieldHardening =   1548,     // effects.ElectronicAttributeModifyActivate
    ewTargetPaint =   1549,     // effects.TargetPaint
    gangECCMfixed =   1648,     // effects.ElectronicAttributeModifyActivate
    doHacking =   1738,     // effects.TargetScan
    siegeModeEffectOld =   1745,     // effects.SiegeMode
    gangArmorRepairSpeedAmplifier =   1746,     // effects.ElectronicAttributeModifyActivate
    gangArmorRepairCapReducer =   1747,     // effects.ElectronicAttributeModifyActivate
    entityCapacitorDrain =   1872,     // effects.EnergyVampire
    entityTrackingDisruptOld =   1877,     // effects.ElectronicAttributeModifyTarget
    entitySensorDampen =   1878,     // effects.ElectronicAttributeModifyTarget
    entityTargetPaint =   1879,     // effects.TargetPaint
    setActiveDamageResonanceMultiplier =   1938,     // effects.ModifyShieldResonance
    droneTrackingComputerMultiply =   2007,     // droneTrackingComputerMultiply
    empWaveGrid =   2071,     // effects.EMPWaveGrid
    modifyActiveArmorResonanceAndNullifyPassiveResonance =   2098,     // effects.ArmorHardening
    modifyActiveShieldResonanceAndNullifyPassiveResonance =   2118,     // effects.ModifyShieldResonance
    jumpPortalGeneration =   2152,     // effects.JumpPortal
    entityShieldBoostingSmall =   2192,     // effects.ShieldBoosting
    entityShieldBoostingMedium =   2193,     // effects.ShieldBoosting
    entityShieldBoostingLarge =   2194,     // effects.ShieldBoosting
    entityArmorRepairingSmall =   2195,     // effects.ArmorRepair
    entityArmorRepairingMedium =   2196,     // effects.ArmorRepair
    entityArmorRepairingLarge =   2197,     // effects.ArmorRepair
    scanStrengthBonusPercentActivate =   2231,     // effects.ScanStrengthBonusActivate
    tractorBeamCan =   2255,     // effects.TractorBeam
    scanStrengthBonusPercentPassive =   2298,     // effects.ScanStrengthBonusActivate
    damageControl  = 2302,
    energyDestabilizationNew =   2303,     // effects.EnergyDestabilization
    energyNosferatu =   2304,     // effects.EnergyVampire
    snowBallLaunching =   2413,     // effects.MissileDeployment
    decreaseTargetSpeedForStructures =   2480,     // effects.ModifyTargetSpeed
    warpScrambleForStructure =   2481,     // effects.WarpScramble
    torpedoLaunchingIsOffensive =   2576,     // effects.TorpedoDeployment
    entityEnvironmentalEffectDamageTest =   2662,     // effects.EMPWave
    rigSlot  = 2663,
    sensorBoosterActivePercentage =   2670,     // effects.ElectronicAttributeModifyActivate
    miningClouds =   2726,     // effects.CloudMining
    salvaging =   2757,     // effects.Salvaging
    energyDestabilizationNewForStructure =   2912,     // effects.EnergyDestabilization
    remoteEcmBurst =   2913,     // effects.RemoteECM
    bombLaunching =   2971,     // effects.MissileDeployment
    energyDestabilizationForStructure =   3003,     // effects.EnergyDestabilization
    remoteHullRepair =   3041,     // effects.RemoteArmourRepair
    triageModeEffectWithoutECMBurst =   3045,     // effects.TriageMode
    siegeModeEffect =   3062,     // effects.SiegeMode
    sensorBoostTargetedHostileKali2Test =   3161,     // effects.ElectronicAttributeModifyTarget
    triageModeEffect =   3162,     // effects.TriageMode
    gangArmorRepairCapReducerSelfAndProjected =   3165,     // effects.ElectronicAttributeModifyActivate
    gangArmorRepairSpeedAmplifierSelfAndProjected =   3167,     // effects.ElectronicAttributeModifyActivate
    warpDisruptSphere =   3380,     // effects.WarpDisruptFieldGenerating
    targetTurretWeaponMaxRangeAndTrackingSpeedBonusHostile =   3552,     // effects.ElectronicAttributeModifyTarget
    targetGunneryMaxRangeAndTrackingSpeedBonusHostile =   3555,     // effects.ElectronicAttributeModifyTarget
    targetGunneryMaxRangeAndTrackingSpeedBonusAssistance =   3556,     // effects.TurretWeaponRangeTrackingSpeedMultiplyTarget
    gunneryMaxRangeAndTrackingSpeedBonus =   3559,     // effects.TurretWeaponRangeTrackingSpeedMultiplyActivate
    targetMaxTargetRangeAndScanResolutionBonusAssistance =   3583,     // effects.ElectronicAttributeModifyTarget
    targetMaxTargetRangeAndScanResolutionBonusHostile =   3584,     // effects.ElectronicAttributeModifyTarget
    jumpPortalGenerationBO =   3674,     // effects.JumpPortalBO
    targetGunneryMaxRangeAndTrackingSpeedAndFalloffBonusHostile =   3690,     // effects.ElectronicAttributeModifyTarget
    turretWeaponRangeFalloffTrackingSpeedMultiplyTargetHostile =   3697,     // effects.ElectronicAttributeModifyTarget
    concordWarpScramble =   3713,     // effects.WarpScramble
    concordModifyTargetSpeed =   3714,     // effects.ModifyTargetSpeed
    warpScrambleTargetMWDBlockActivation =   3725,     // effects.WarpScramble
    subSystem  = 3772,
    probeLaunching =   3793,     // effects.MissileDeployment
    NPCRemoteArmorRepair =   3852,     // effects.RemoteArmourRepair
    NPCRemoteShieldBoost =   3855,     // effects.ShieldTransfer
    superWeaponTurret =   4481,     // effects.AnchorLift
    superWeaponAmarr =   4489,     // effects.SuperWeaponAmarr
    superWeaponCaldari =   4490,     // effects.SuperWeaponCaldari
    superWeaponGallente =   4491,     // effects.SuperWeaponGallente
    superWeaponMinmatar =   4492,     // effects.SuperWeaponMinmatar
    gunneryMaxRangeFalloffTrackingSpeedBonus =   4559,     // effects.TurretWeaponRangeTrackingSpeedMultiplyActivate
    targetGunneryMaxRangeFalloffTrackingSpeedBonusAssistance =   4560,     // effects.TurretWeaponRangeTrackingSpeedMultiplyTarget
    siegeModeEffect3 =   4568,     // effects.SiegeMode
    siegeModeEffect4 =   4573,     // effects.SiegeMode
    triageModeEffect2 =   4574,     // effects.TriageMode
    industrialCoreEffect2 =   4575,     // effects.SiegeMode
    NPCGroupShieldAssist =   4686,     // effects.ElectronicAttributeModifyActivate
    NPCGroupSpeedAssist =   4687,     // effects.ElectronicAttributeModifyActivate
    NPCGroupPropJamAssist =   4688,     // effects.ElectronicAttributeModifyActivate
    NPCGroupArmorAssist =   4689,     // effects.ElectronicAttributeModifyActivate
    fighterMissile =   4729,     // effects.Laser
    anchorDropForOrbitals =   4769,     // effects.AnchorDrop
    anchorLiftForOrbitals =   4770,     // effects.AnchorLift
    onlineOrbital =   4771,     // effects.StructureOnline
    hackOrbital =   4773,     // effects.TargetScan
    deployPledge =   4774,     // DeployPledge          -- no clue what this is...
    siegeModeEffect5 =   4838,     // effects.SiegeMode
    triageModeEffect3 =   4839,     // effects.TriageMode
    siegeModeEffect6 =   4877,     // effects.SiegeMode
    triageModeEffect7 =   4893,     // effects.TriageMode
    microJumpDrive =   4921,     // effects.MicroJumpDriveEngage        -- not in Crucible
    adaptiveArmorHardener =   4928,     // effects.ArmorHardening
    targetTrackingDisruptorCombinedGunneryAndMissileEffect =   4932,     // effects.ElectronicAttributeModifyTarget
    fueledShieldBoosting =   4936,     // effects.ShieldBoosting
    targetBreaker =   4942,     // effects.TargetBreaker      -- not in Crucible
    unusedEntityTrackingDisrupt4 =   4980,     // effects.ElectronicAttributeModifyTarget
    unusedEntityTrackingDisrupt5 =   4981,     // effects.ElectronicAttributeModifyTarget
    entityTrackingDisrupt =   4982,     // effects.ElectronicAttributeModifyTarget
    orbitalStrike =   5141,     // effects.OrbitalStrike      -- for dust514
    salvageDroneEffect =   5163,     // effects.Salvaging
    modifyActiveShieldResonancePostPercent =   5230,     // effects.ModifyShieldResonance
    modifyActiveArmorResonancePostPercent =   5231,     // effects.ArmorHardening
    fueledArmorRepair =   5275,     // effects.ArmorRepair
    gangSensorIntegrity =   5551,     // effects.ElectronicAttributeModifyActivate
    marauderModeEffect25 =   5643,     // effects.SiegeMode
    EssWarpScramble =   5768,     // effects.BeamCollecting    -- not in Crucible
    marauderModeEffect26 =   5788,     // effects.SiegeMode
    warpScrambleTargetMWDBlockActivationForEntity =   5928,     // effects.WarpScramble
    warpScrambleBlockMWDWithNPCEffect =   5934,     // effects.WarpScramble
    cloakingPrototype =   5945,     // effects.CloakingPrototype  -- not in Crucible
    shipModeScanStrengthPostDiv =   6012,     // effects.ScanStrengthBonusActivate
} EffectsID;
    }
}

#endif  //EVE_EFFECTS_H


/*
FX_TURRET_EFFECT_GUIDS = ['effects.Laser',
 'effects.ProjectileFiredForEntities',
 'effects.ProjectileFired',
 'effects.HybridFired',
 'effects.TractorBeam',
 'effects.Salvaging']
 */

/*  defined in client...
 e ffectAnchorDrop = 649          *
 effectAnchorDropForStructures = 1022
 effectAnchorLift = 650
 effectAnchorLiftForStructures = 1023
 effectBarrage = 263
 effectBombLaunching = 2971
 effectCloaking = 607
 effectCloakingWarpSafe = 980
 effectCloneVatBay = 2858
 effectCynosuralGeneration = 2857
 effectConcordWarpScramble = 3713
 effectConcordModifyTargetSpeed = 3714
 effectConcordTargetJam = 3710
 effectDecreaseTargetSpeed = 586
 effectDefenderMissileLaunching = 103
 effectDeployPledge = 4774
 effectECMBurst = 53
 effectEmpWave = 38
 effectEmpWaveGrid = 2071
 effectEnergyDestabilizationNew = 2303
 effectEntityCapacitorDrain = 1872
 effectEntitySensorDampen = 1878
 effectEntityTargetJam = 1871
 effectEntityTargetPaint = 1879
 effectEntityTrackingDisrupt = 1877
 effectEwTargetPaint = 1549
 effectEwTestEffectWs = 1355
 effectEwTestEffectJam = 1358
 effectFighterMissile = 4729
 effectFlagshipmultiRelayEffect = 1495
 effectFofMissileLaunching = 104
 effectGangBonusSignature = 1411
 effectGangShieldBoosterAndTransporterSpeed = 2415
 effectGangShieldBoosteAndTransporterCapacitorNeed = 2418
 effectGangIceHarvestingDurationBonus = 2441
 effectGangInformationWarfareRangeBonus = 2642
 effectGangArmorHardening = 1510
 effectGangPropulsionJammingBoost = 1546
 effectGangShieldHardening = 1548
 effectGangECCMfixed = 1648
 effectGangArmorRepairCapReducerSelfAndProjected = 3165
 effectGangArmorRepairSpeedAmplifierSelfAndProjected = 3167
 effectGangMiningLaserAndIceHarvesterAndGasCloudHarvesterMaxRangeBonus = 3296
 effectGangGasHarvesterAndIceHarvesterAndMiningLaserDurationBonus = 3302
 effectGangGasHarvesterAndIceHarvesterAndMiningLaserCapNeedBonus = 3307
 effectGangInformationWarfareSuperiority = 3647
 effectGangAbMwdFactorBoost = 1755
 effectHackOrbital = 4773
 effectHardPointModifier = 3773
 effectHiPower = 12
 effectIndustrialCoreEffect = 4575
 effectJumpPortalGeneration = 2152
 effectJumpPortalGenerationBO = 3674
 effectLauncherFitted = 40
 effectLeech = 3250
 effectLoPower = 11
 effectMedPower = 13
 effectMineLaying = 102
 effectMining = 17
 effectMiningClouds = 2726
 effectMiningLaser = 67
 effectMissileLaunching = 9
 effectMissileLaunchingForEntity = 569
 effectModifyTargetSpeed2 = 575
 effectNPCGroupArmorAssist = 4689
 effectNPCGroupPropJamAssist = 4688
 effectNPCGroupShieldAssist = 4686
 effectNPCGroupSpeedAssist = 4687
 effectNPCRemoteArmorRepair = 3852
 effectNPCRemoteShieldBoost = 3855
 effectNPCRemoteECM = 4656
 effectOffensiveDefensiveReduction = 4728
 effectOnline = 16
 effectOnlineForStructures = 901
 effectOpenSpawnContainer = 1738
 effectProbeLaunching = 3793
 effectProjectileFired = 34
 effectProjectileFiredForEntities = 1086
 effectRemoteHullRepair = 3041
 effectRemoteEcmBurst = 2913
 effectRigSlot = 2663
 effectSalvaging = 2757
 effectScanStrengthBonusTarget = 124
 effectscanStrengthTargetPercentBonus = 2246
 effectShieldResonanceMultiplyOnline = 105
 effectSiegeModeEffect = 4877
 effectSkillEffect = 132
 effectSlotModifier = 3774
 effectSnowBallLaunching = 2413
 effectStructureUnanchorForced = 1129
 effectSubSystem = 3772
 effectSuicideBomb = 885
 effectSuperWeaponAmarr = 4489
 effectSuperWeaponCaldari = 4490
 effectSuperWeaponGallente = 4491
 effectSuperWeaponMinmatar = 4492
 effectTargetAttack = 10
 effectTargetAttackForStructures = 1199
 effectTargetGunneryMaxRangeAndTrackingSpeedBonusHostile = 3555
 effectTargetGunneryMaxRangeAndTrackingSpeedAndFalloffBonusHostile = 3690
 effectTargetMaxTargetRangeAndScanResolutionBonusHostile = 3584
 effectTargetGunneryMaxRangeAndTrackingSpeedBonusAssistance = 3556
 effectTargetMaxTargetRangeAndScanResolutionBonusAssistance = 3583
 effectTargetPassively = 54
 effectTorpedoLaunching = 127
 effectTorpedoLaunchingIsOffensive = 2576
 effectTractorBeamCan = 2255
 effectTriageMode = 4839
 effectTurretFitted = 42
 effectTurretWeaponRangeFalloffTrackingSpeedMultiplyTargetHostile = 3697
 effectUseMissiles = 101
 effectWarpDisruptSphere = 3380
 effectWarpScramble = 39
 effectWarpScrambleForEntity = 563
 effectWarpScrambleTargetMWDBlockActivation = 3725
 effectModifyShieldResonancePostPercent = 2052
 effectModifyArmorResonancePostPercent = 2041
 effectModifyHullResonancePostPercent = 3791
 effectShipMaxTargetRangeBonusOnline = 3659
 effectSensorBoostTargetedHostile = 837
 effectmaxTargetRangeBonus = 2646
 */

/*
definitions = {'effects.AnchorDrop': (effects.AnchorDrop,
                        FX_TF_NONE,
                        FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                        None,
                        1,
                        10000),
 'effects.AnchorLift': (effects.AnchorLift,
                        FX_TF_NONE,
                        FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                        None,
                        1,
                        10000),
 'effects.ArmorHardening': (effects.ShipRenderEffect,
                            FX_TF_NONE,
                            FX_MERGE_SHIP | FX_MERGE_GUID,
                            'res:/dx9/Model/Effect/ArmorHardening.red',
                            1,
                            10000),
 'effects.ArmorRepair': (effects.ShipRenderEffect,
                         FX_TF_NONE,
                         FX_MERGE_SHIP | FX_MERGE_GUID,
                         'res:/dx9/Model/Effect/ArmorRepair.red',
                         1,
                         10000),
 'effects.Barrage': (effects.StandardWeapon,
                     FX_TF_NONE,
                     FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                     None,
                     1,
                     10000),
 'effects.CargoScan': (effects.StretchEffect,
                       FX_TF_NONE,
                       FX_MERGE_SHIP | FX_MERGE_GUID,
                       'res:/Model/Effect3/CargoScan.red',
                       1,
                       10000),
 'effects.Cloak': (effects.Cloak,
                   FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
                   FX_MERGE_SHIP | FX_MERGE_GUID,
                   'res:/Model/Effect3/Cloaking.red',
                   1,
                   6000),
 'effects.CloakNoAmim': (effects.CloakNoAmim,
                         FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
                         FX_MERGE_SHIP | FX_MERGE_GUID,
                         'res:/Model/Effect3/Cloaking.red',
                         1,
                         6000),
 'effects.CloakRegardless': (effects.CloakRegardless,
                             FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
                             FX_MERGE_SHIP | FX_MERGE_GUID,
                             'res:/Model/Effect3/Cloaking.red',
                             1,
                             6000),
 'effects.Cloaking': (effects.Cloaking,
                      FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
                      FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                      None,
                      1,
                      10000),
 'effects.CloudMining': (effects.CloudMining,
                         FX_TF_NONE,
                         FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                         None,
                         1,
                         10000),
 'effects.ECMBurst': (effects.ShipEffect,
                      FX_TF_POSITION_BALL,
                      FX_MERGE_SHIP | FX_MERGE_GUID,
                      'res:/Model/Effect3/EcmBurst.red',
                      1,
                      10000),
 'effects.EMPWave': (effects.EMPWave,
                     FX_TF_NONE,
                     FX_MERGE_SHIP | FX_MERGE_MODULE,
                     None,
                     1,
                     10000),
 'effects.ElectronicAttributeModifyActivate': (effects.ShipEffect,
                                               FX_TF_SCALE_RADIUS | FX_TF_POSITION_BALL,
                                               FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                                               'res:/Model/Effect3/ECM.red',
                                               1,
                                               10000),
 'effects.ElectronicAttributeModifyTarget': (effects.StretchEffect,
                                             FX_TF_NONE,
                                             FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
                                             'res:/Model/Effect3/SensorBoost.red',
                                             1,
                                             10000),
 'effects.EnergyDestabilization': (effects.StretchEffect,
                                   FX_TF_NONE,
                                   FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
                                   'res:/Model/Effect3/EnergyDestabilization.red',
                                   1,
                                   10000),
 'effects.EnergyTransfer': (effects.StretchEffect,
                            FX_TF_NONE,
                            FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
                            'res:/Model/Effect3/EnergyTransfer.red',
                            1,
                            10000),
 'effects.EnergyVampire': (effects.StretchEffect,
                           FX_TF_NONE,
                           FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
                           'res:/Model/Effect3/EnergyVampire.red',
                           1,
                           10000),
 'effects.GateActivity': (effects.GateActivity,
                          FX_TF_NONE,
                          FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                          None,
                          1,
                          10000),
 'effects.HybridFired': (effects.StandardWeapon,
                         FX_TF_NONE,
                         FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                         None,
                         1,
                         10000),
 'effects.Jettison': (effects.ShipEffect,
                      FX_TF_POSITION_BALL,
                      FX_MERGE_SHIP | FX_MERGE_GUID,
                      'res:/Model/Effect3/Jettison.red',
                      1,
                      10000),
 'effects.JumpDriveIn': (effects.JumpDriveIn,
                         FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
                         FX_MERGE_SHIP | FX_MERGE_GUID,
                         'res:\\Model\\Effect3\\JumpDrive_in.red',
                         1,
                         10000),
 'effects.JumpDriveInBO': (effects.JumpDriveInBO,
                           FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
                           FX_MERGE_SHIP | FX_MERGE_GUID,
                           'res:\\Model\\Effect3\\JumpDriveBO_in.red',
                           1,
                           10000),
 'effects.JumpDriveOut': (effects.JumpDriveOut,
                          FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL,
                          FX_MERGE_SHIP | FX_MERGE_GUID,
                          'res:\\Model\\Effect3\\JumpDrive_out.red',
                          1,
                          10000),
 'effects.JumpDriveOutBO': (effects.JumpDriveOutBO,
                            FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL,
                            FX_MERGE_SHIP | FX_MERGE_GUID,
                            'res:\\Model\\Effect3\\JumpDriveBO_out.red',
                            1,
                            10000),
 'effects.JumpIn': (effects.JumpIn,
                    FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
                    FX_MERGE_SHIP | FX_MERGE_GUID,
                    'res:/Model/Effect3/warpEntry.red',
                    1,
                    10000),
 'effects.JumpOut': (effects.JumpOut,
                     FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
                     FX_MERGE_SHIP | FX_MERGE_GUID,
                     'res:\\Model\\Effect3\\Jump_out.red',
                     1,
                     10000),
 'effects.JumpOutWormhole': (effects.JumpOutWormhole,
                             FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
                             FX_MERGE_SHIP | FX_MERGE_GUID,
                             'res:\\Model\\Effect3\\WormJump.red',
                             1,
                             10000),
 'effects.JumpPortal': (effects.JumpPortal,
                        FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
                        FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                        'res:/Model/Effect3/JumpPortal.red',
                        1,
                        10000),
 'effects.JumpPortalBO': (effects.JumpPortalBO,
                          FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
                          FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                          'res:/Model/Effect3/JumpPortal_BO.red',
                          1,
                          10000),
 'effects.Laser': (effects.StandardWeapon,
                   FX_TF_NONE,
                   FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                   None,
                   1,
                   10000),
 'effects.Mining': (effects.StandardWeapon,
                    FX_TF_NONE,
                    FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                    None,
                    1,
                    10000),
 'effects.MissileDeployment': (effects.ShipEffect,
                               FX_TF_POSITION_BALL,
                               FX_MERGE_SHIP | FX_MERGE_GUID,
                               'res:/Model/Effect3/missileLaunch.red',
                               1,
                               12000),
 'effects.ModifyShieldResonance': (effects.ShipRenderEffect,
                                   FX_TF_NONE,
                                   FX_MERGE_SHIP | FX_MERGE_GUID,
                                   'res:/dx9/Model/Effect/ShieldHardening.red',
                                   1,
                                   10000),
 'effects.ModifyTargetSpeed': (effects.ShipEffect,
                               FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_TARGET,
                               FX_MERGE_TARGET | FX_MERGE_GUID,
                               'res:/Model/Effect3/StasisWeb.red',
                               1,
                               10000),
 'effects.ProjectileFired': (effects.StandardWeapon,
                             FX_TF_NONE,
                             FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                             None,
                             1,
                             10000),
 'effects.ProjectileFiredForEntities': (effects.StandardWeapon,
                                        FX_TF_NONE,
                                        FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                                        None,
                                        1,
                                        10000),
 'effects.RemoteArmourRepair': (effects.StretchEffect,
                                FX_TF_NONE,
                                FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
                                'res:/Model/Effect3/RemoteArmorRepair.red',
                                1,
                                10000),
 'effects.RemoteECM': (effects.StretchEffect,
                       FX_TF_NONE,
                       FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
                       'res:/Model/Effect3/RemoteECM.red',
                       1,
                       10000),
 'effects.Salvaging': (effects.StandardWeapon,
                       FX_TF_NONE,
                       FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                       None,
                       1,
                       10000),
 'effects.ScanStrengthBonusActivate': (effects.ShipEffect,
                                       FX_TF_SCALE_RADIUS | FX_TF_POSITION_BALL,
                                       FX_MERGE_SHIP | FX_MERGE_GUID,
                                       'res:/Model/Effect3/ECCM.red',
                                       1,
                                       10000),
 'effects.ScanStrengthBonusTarget': (effects.ShipEffect,
                                     FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL,
                                     FX_MERGE_SHIP | FX_MERGE_GUID,
                                     'res:/Model/Effect3/ECCM.red',
                                     1,
                                     10000),
 'effects.ShieldBoosting': (effects.ShipRenderEffect,
                            FX_TF_NONE,
                            FX_MERGE_SHIP | FX_MERGE_GUID,
                            'res:/dx9/Model/Effect/ShieldBoosting.red',
                            0,
                            10000),
 'effects.ShieldTransfer': (effects.StretchEffect,
                            FX_TF_NONE,
                            FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
                            'res:/Model/Effect3/ShieldTransfer.red',
                            1,
                            10000),
 'effects.ShipScan': (effects.StretchEffect,
                      FX_TF_NONE,
                      FX_MERGE_SHIP | FX_MERGE_GUID,
                      'res:/Model/Effect3/ShipScan.red',
                      1,
                      10000),
 'effects.SiegeMode': (effects.SiegeMode,
                       FX_TF_NONE,
                       FX_MERGE_SHIP,
                       None,
                       1,
                       10000),
 'effects.SpeedBoost': (effects.GenericEffect,
                        FX_TF_NONE,
                        FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                        None,
                        1,
                        10000),
 'effects.StructureOffline': (effects.StructureOffline,
                              FX_TF_NONE,
                              FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                              None,
                              1,
                              10000),
 'effects.StructureOnline': (effects.StructureOnline,
                             FX_TF_NONE,
                             FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                             None,
                             1,
                             10000),
 'effects.StructureOnlined': (effects.StructureOnlined,
                              FX_TF_NONE,
                              FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                              None,
                              1,
                              10000),
 'effects.StructureRepair': (effects.ShipRenderEffect,
                             FX_TF_NONE,
                             FX_MERGE_SHIP | FX_MERGE_GUID,
                             'res:/dx9/Model/Effect/HullRepair.red',
                             1,
                             10000),
 'effects.SuperWeaponAmarr': (effects.StretchEffect,
                              FX_TF_NONE,
                              FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                              'res:/Model/Effect3/Superweapon/A_DoomsDay.red',
                              False,
                              10000),
 'effects.SuperWeaponCaldari': (effects.StretchEffect,
                                FX_TF_NONE,
                                FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                                'res:/Model/Effect3/Superweapon/C_DoomsDay.red',
                                False,
                                10000),
 'effects.SuperWeaponGallente': (effects.StretchEffect,
                                 FX_TF_NONE,
                                 FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                                 'res:/Model/Effect3/Superweapon/G_DoomsDay.red',
                                 False,
                                 10000),
 'effects.SuperWeaponMinmatar': (effects.StretchEffect,
                                 FX_TF_NONE,
                                 FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                                 'res:/Model/Effect3/Superweapon/M_DoomsDay.red',
                                 False,
                                 10000),
 'effects.SurveyScan': (effects.ShipEffect,
                        FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL,
                        FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
                        'res:/Model/Effect3/SurveyScan.red',
                        1,
                        10000),
 'effects.TargetPaint': (effects.StretchEffect,
                         FX_TF_NONE,
                         FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
                         'res:/Model/Effect3/TargetPaint.red',
                         1,
                         10000),
 'effects.TargetScan': (effects.StretchEffect,
                        FX_TF_NONE,
                        FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
                        'res:/Model/Effect3/SurveyScan2.red',
                        1,
                        10000),
 'effects.TorpedoDeployment': (effects.GenericEffect,
                               FX_TF_NONE,
                               FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                               None,
                               1,
                               10000),
 'effects.TractorBeam': (effects.StandardWeapon,
                         FX_TF_NONE,
                         FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                         None,
                         1,
                         10000),
 'effects.TriageMode': (effects.ShipRenderEffect,
                        FX_TF_NONE,
                        FX_MERGE_SHIP | FX_MERGE_GUID,
                        'res:/dx9/Model/Effect/TriageMode.red',
                        0,
                        10000),
 'effects.TurretWeaponRangeTrackingSpeedMultiplyActivate': (effects.ShipEffect,
                                                            FX_TF_POSITION_BALL,
                                                            FX_MERGE_SHIP | FX_MERGE_GUID,
                                                            'res:/Model/Effect3/TrackingBoost.red',
                                                            1,
                                                            10000),
 'effects.TurretWeaponRangeTrackingSpeedMultiplyTarget': (effects.StretchEffect,
                                                          FX_TF_NONE,
                                                          FX_MERGE_SHIP | FX_MERGE_GUID,
                                                          'res:/Model/Effect3/TrackingBoostTarget.red',
                                                          1,
                                                          10000),
 'effects.Uncloak': (effects.Uncloak,
                     FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
                     FX_MERGE_SHIP | FX_MERGE_GUID,
                     'res:/Model/Effect3/Cloaking.red',
                     1,
                     7500),
 'effects.WarpDisruptFieldGenerating': (effects.WarpDisruptFieldGenerating,
                                        FX_TF_POSITION_BALL,
                                        FX_MERGE_SHIP | FX_MERGE_GUID,
                                        'res:/Model/effect3/WarpDisruptorBubble.red',
                                        0,
                                        10000),
 'effects.WarpGateEffect': (effects.WarpGateEffect,
                            FX_TF_NONE,
                            FX_MERGE_SHIP | FX_MERGE_GUID,
                            None,
                            0,
                            10000),
 'effects.WarpScramble': (effects.StretchEffect,
                          FX_TF_NONE,
                          FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                          'res:/Model/Effect3/WarpScrambler.red',
                          1,
                          10000),
 'effects.Warping': (effects.Warping,
                     FX_TF_NONE,
                     FX_MERGE_SHIP | FX_MERGE_GUID,
                     'res:/Model/Effect3/warpTunnel2.red',
                     False,
                     1200000),
 'effects.WormholeActivity': (effects.WormholeActivity,
                              FX_TF_NONE,
                              FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
                              None,
                              1,
                              10000)}
                              */