/*
 *
 *
 *
 */


#ifndef EVE_EFFECTS_H
#define EVE_EFFECTS_H


//these come from dgmEffects.
//  -allan 18Aug14
//  incomplete  3jan15
typedef enum {
    effectNeedsTarget                   = 2,    // added for target checking
    effectShieldBoosting                = 4,    //effects.ShieldBoosting
    effectSpeedBoost                    = 7,    //effects.SpeedBoost
    effectMissileLaunching              = 9,    //effects.MissileDeployment
    effectTargetAttack                  = 10,   //effects.Laser
    effectLoPower                       = 11,
    effectHiPower                       = 12,
    effectMedPower                      = 13,
    effectOnline                        = 16,
    effectMining                        = 17,   //effects.Mining
    effectShieldTransfer                = 18,   //effects.ShieldTransfer
    effectStructureRepair               = 26,   //effects.StructureRepair
    effectArmorRepair                   = 27,   //effects.ArmorRepair
    effectModifyTargetSpeed             = 29,   //effects.ModifyTargetSpeed
    effectEnergyTransfer                = 31,   //effects.EnergyTransfer
    effectEnergyVampire                 = 32,   //effects.EnergyDestabilization
    effectProjectileFired               = 34,   //effects.ProjectileFired
    effectEnergyDestabilization         = 36,   //effects.EnergyDestabilization
    effectEMPWave                       = 38,   //effects.EMPWave
    effectWarpScramble                  = 39,   //effects.WarpScramble
    effectLauncherFitted                = 40,
    effectTurretFitted                  = 42,
    effectCargoScan                     = 47,   //effects.CargoScan
    effectECMBurst                      = 53,   //effects.ECMBurst
    effectCapacitorCapacityMultiply     = 58,
    effectMiningLaser                   = 67,   //effects.Laser
    effectEmpFieldRange                 = 99,
    effectSurveyScan                    = 81,   //effects.SurveyScan
    effectUseMissiles                   = 101,  //effects.useMissiles
    effectFoFMissileLaunching           = 104,  //effects.MissileDeployment
    effectTurretWeaponRangeTrackingSpeedMultiplyActivate        = 123,  //effects.TurretWeaponRangeTrackingSpeedMultiplyActi...
    effectScanStrengthBonusTarget       = 124,  //effects.ScanStrengthBonusTarget
    effectTurretWeaponRangeTrackingSpeedMultiplyTarget          = 126,  //effects.TurretWeaponRangeTrackingSpeedMultiplyTarg...
    effectTorpedoLaunching              = 127,  //effects.TorpedoDeployment
    effectSkillEffect                   = 132,
    effectBarrage                       = 263,  //effects.Barrage
    effectWarpScrambleForEntity         = 563,  //effects.WarpScramble
    effectModifyTargetSpeed2            = 575,  //effects.ModifyTargetSpeed
    effectDecreaseTargetSpeed           = 586,  //effects.ModifyTargetSpeed
    effectTargetArmorRepair             = 592,  //effects.RemoteArmourRepair
    effectTargetedEMResonanceMultiply   = 597,  //effects.EnergyDestabilization
    effectCloaking                      = 607,  //effects.Cloaking
    effectTurretWeaponRangeTrackingSpeedMultiplyTargetHostile   = 609,  //effects.ElectronicAttributeModifyTarget
    effectTargetedKineticResonanceMultiply          = 615,  //effects.EnergyDestabilization
    effectTargetedThermalResonanceMultiply          = 616,  //effects.EnergyDestabilization
    effectTargetedExplosiveResonanceMultiply        = 617,  //effects.EnergyDestabilization
    effectAnchorDrop                    = 649,  //effects.AnchorDrop
    effectAnchorLift                    = 650,  //effects.AnchorLift
    effectStealthActive                 = 713,  //effects.ElectronicAttributeModifyActivate
    effectSensorBoostTargeted           = 716,  //effects.ElectronicAttributeModifyTarget
    effectSensorBoosterActive           = 720,  //effects.ElectronicAttributeModifyActivate
    effectSensorBoostTargetedHostile    = 837,  //effects.ElectronicAttributeModifyTarget
    effectShieldBoostingForEntities     = 876,  //effects.ShieldBoosting
    effectArmorRepairForEntities        = 878,  //effects.ArmorRepair
    effectSuicideBomb                   = 885,  //effects.EMPWave
    effectOnlineForStructures           = 901,  //effects.StructureOnline
    effectDecloakWave                   = 902,  //effects.DecloakWave
    effectCloakingWarpSafe              = 980,  //effects.Cloaking
    effectAnchorDropForStructures       = 1022, //effects.AnchorDrop
    effectAnchorLiftForStructures       = 1023, //effects.AnchorLift
    effectProjectileFiredForEntities    = 1086, //effects.ProjectileFiredForEntities
    effectTargetAttackForStructures     = 1199, //effects.ProjectileFiredForEntities
    effectSpeedBoostMassAddition        = 1253, //effects.speedBoostMassAddition
    effectSpeedBoostMassSigRad          = 1254, //effects.speedBoostMassSigRad
    effectNewEwTestscanStrengthBonusTargetHostile   = 1271, //effects.ScanStrengthBonusTarget
    effectNewEwTestswarpScramble        = 1272, //effects.WarpScramble
    effectNewEwTestssensorBoostTargetedHostile      = 1273, //effects.ElectronicAttributeModifyTarget
    effectNewEwTeststurretWeaponRangeTrackingSpeedMultiplyTargetHostile         = 1274, //effects.Target_paint
    effectNewEwTestsdecreaseTargetSpeed = 1275, //effects.ModifyTargetSpeed
    effectEWTestEffectRsd               = 1354, //effects.ElectronicAttributeModifyTarget
    effectEWTestEffectWs                = 1355, //effects.WarpScramble
    effectGangBonusSignature            = 1411, //effects.ElectronicAttributeModifyActivate
    effectGangArmorHardening            = 1510, //effects.ElectronicAttributeModifyActivate
    effectGangPropulsionJammingBoost    = 1546, //effects.ElectronicAttributeModifyActivate
    effectGangShieldHardening           = 1548, //effects.ElectronicAttributeModifyActivate
    effectEWTargetPaint                 = 1549, //effects.TargetPaint
    effectGangECCMfixed                 = 1648, //effects.ElectronicAttributeModifyActivate
    effectOpenSpawnContainer            = 1738, //effects.TargetScan
    effectSiegeModeEffectOld            = 1745, //effects.SiegeMode
    effectGangArmorRepairSpeedAmplifier = 1746, //effects.ElectronicAttributeModifyActivate
    effectGangArmorRepairCapReducer     = 1747, //effects.ElectronicAttributeModifyActivate
    effectEntityCapacitorDrain          = 1872, //effects.EnergyVampire
    effectEntityTrackingDisrupt         = 1877, //effects.ElectronicAttributeModifyTarget
    effectEntitySensorDampen            = 1878, //effects.ElectronicAttributeModifyTarget
    effectSetActiveDamageResonanceMultiplier    = 1938, //effects.ModifyShieldResonance
    effecteEMPWaveGrid                  = 2071, //effects.EMPWaveGrid
    effectModifyActiveArmorResonanceAndNullifyPassiveResonance  = 2098, //effects.ArmorHardening
    effectModifyActiveShieldResonanceAndNullifyPassiveResonance = 2118, //effects.ModifyShieldResonance
    effectJumpPortalGeneration          = 2152, //effects.JumpPortal
    effectEntityShieldBoostingSmall     = 2192, //effects.ShieldBoosting
    effectEntityShieldBoostingMedium    = 2193, //effects.ShieldBoosting
    effectEntityShieldBoostingLarge     = 2194, //effects.ShieldBoosting
    effectEntityArmorRepairingSmall     = 2195, //effects.ArmorRepair
    effectEntityArmorRepairingMedium    = 2196, //effects.ArmorRepair
    effectEntityArmorRepairingLarge     = 2197, //effects.ArmorRepair
    effectScanStrengthBonusPercentActivate      = 2231, //effects.ScanStrengthBonusActivate
    effectTractorBeam                   = 2255, //effects.TractorBeam
    effectScanStrengthBonusPercentPassive       = 2298, //effects.ScanStrengthBonusActivate
    effectDamageControl                 = 2302,
    effectEnergyDestabilizationNew      = 2303, //effects.EnergyDestabilization
    effectEnergyNosferatu               = 2304, //effects.EnergyVampire
    effectSnowBallLaunching             = 2413, //effects.MissileDeployment
    effectDecreaseTargetSpeedForStructures      = 2480, //effects.ModifyTargetSpeed
    effectWarpScrambleForStructure      = 2481, //effects.WarpScramble
    effectTorpedoLaunchingIsOffensive   = 2576, //effects.TorpedoDeployment
    effectEntityEnvironmentalEffectDamageTest   = 2662, //effects.EMPWave
    effectRigSlot                       = 2663,
    effectSensorBoosterActivePercentage = 2670, //effects.ElectronicAttributeModifyActivate
    effectMiningClouds                  = 2726, //effects.CloudMining
    effectSalvaging                     = 2757, //effects.Salvaging
    effectEnergyDestabilizationNewForStructure  = 2912, //effects.EnergyDestabilization
    effectRemoteEcmBurst                = 2913, //effects.RemoteECM
    effectBombLaunching                 = 2971, //effects.MissileDeployment
    effectEnergyDestabilizationForStructure     = 3003, //effects.EnergyDestabilization
    effectRemoteHullRepair              = 3041, //effects.RemoteArmourRepair
    effectTriageModeEffectWithoutECMBurst       = 3045, //effects.TriageMode
    effectSiegeModeEffect               = 3062, //effects.SiegeMode
    effectSensorBoostTargetedHostileKali2Test   = 3161, //effects.ElectronicAttributeModifyTarget
    effectTriageModeEffect              = 3162, //effects.TriageMode
    effectGangArmorRepairCapReducerSelfAndProjected         = 3165, //effects.ElectronicAttributeModifyActivate
    effectGangArmorRepairSpeedAmplifierSelfAndProjected     = 3167, //effects.ElectronicAttributeModifyActivate
    effectLeech                         = 3250, //effects.EnergyVampire
    effectIndustrialCoreEffectOLD       = 3282, //effects.SiegeMode
    effectLeechNpc                      = 3332, //effects.EnergyVampire
    effectWarpDisruptSphere             = 3380, //effects.WarpDisruptFieldGenerating
    effectIndustrialCoreEffect          = 3492, //effects.SiegeMode
    effectBonusBlackOpsAgiliy1          = 3530, //effects.eliteBonusBlackOpsAgiliy1
    effectTargetTurretWeaponMaxRangeAndTrackingSpeedBonusHostile    = 3552, //effects.ElectronicAttributeModifyTarget
    effectTargetGunneryMaxRangeAndTrackingSpeedBonusHostile         = 3555, //effects.ElectronicAttributeModifyTarget
    effectTargetGunneryMaxRangeAndTrackingSpeedBonusAssistance      = 3556, //effects.TurretWeaponRangeTrackingSpeedMultiplyTarg...
    effectGunneryMaxRangeAndTrackingSpeedBonus                      = 3559, //effects.TurretWeaponRangeTrackingSpeedMultiplyActi...
    effectTargetMaxTargetRangeAndScanResolutionBonusAssistance      = 3583, //effects.ElectronicAttributeModifyTarget
    effectTargetMaxTargetRangeAndScanResolutionBonusHostile         = 3584, //effects.ElectronicAttributeModifyTarget
    effectTargetSetWarpScrambleStatusHidden                         = 3604, //effects.WarpScramble
    effectJumpPortalGenerationBO        = 3674, //effects.JumpPortalBO
    effectTargetGunneryMaxRangeAndTrackingSpeedAndFalloffBonusHostile       = 3690, //effects.ElectronicAttributeModifyTarget
    effectTurretWeaponRangeFalloffTrackingSpeedMultiplyTargetHostile        = 3697, //effects.ElectronicAttributeModifyTarget
    effectConcordWarpScramble           = 3713, //effects.WarpScramble
    effectConcordModifyTargetSpeed      = 3714, //effects.ModifyTargetSpeed
    effectWarpScrambleTargetMWDBlockActivation                      = 3725, //effects.WarpScramble
    effectSubSystem                     = 3772,
    effectProbeLaunching                = 3793, //effects.MissileDeployment
    effectNPCRemoteArmorRepair          = 3852, //effects.RemoteArmourRepair
    effectNPCRemoteShieldBoost          = 3855, //effects.ShieldTransfer
    effectSuperWeaponTurret             = 4481, //effects.AnchorLift
    effectSuperWeaponAmarr              = 4489, //effects.SuperWeaponAmarr
    effectSuperWeaponCaldari            = 4490, //effects.SuperWeaponCaldari
    effectSuperWeaponGallente           = 4491, //effects.SuperWeaponGallente
    effectSuperWeaponMinmatar           = 4492, //effects.SuperWeaponMinmatar
    effectGunneryMaxRangeFalloffTrackingSpeedBonus                  = 4559, //effects.TurretWeaponRangeTrackingSpeedMultiplyActi...
    effectTargetGunneryMaxRangeFalloffTrackingSpeedBonusAssistance  = 4560, //effects.TurretWeaponRangeTrackingSpeedMultiplyTarg...
    effectSiegeModeEffect3              = 4568, //effects.SiegeMode
    effectSiegeModeEffect4              = 4573, //effects.SiegeMode
    effectTriageModeEffect2             = 4574, //effects.TriageMode
    effectIndustrialCoreEffect2         = 4575, //effects.SiegeMode
    effectNPCGroupShieldAssist          = 4686, //effects.ElectronicAttributeModifyActivate
    effectNPCGroupSpeedAssist           = 4687, //effects.ElectronicAttributeModifyActivate
    effectNPCGroupPropJamAssist         = 4688, //effects.ElectronicAttributeModifyActivate
    effectNPCGroupArmorAssist           = 4689, //effects.ElectronicAttributeModifyActivate
    effectFighterMissile                = 4729, //effects.Lasereffect
} EVEEffectID;

#endif  //EVE_EFFECTS_H