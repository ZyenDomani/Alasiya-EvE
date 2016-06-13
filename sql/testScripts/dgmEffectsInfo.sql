
--
-- Table structure for table `dgmEffectsInfo`
--

CREATE TABLE `dgmEffectsInfo` (
  `effectID` int(11) DEFAULT NULL,
  `sourceAttributeID` int(11) NOT NULL,
  `targetAttributeID` int(11) NOT NULL,
  `calculationTypeID` int(11) NOT NULL,
  `description` varchar(500) NOT NULL,
  `reverseCalculationTypeID` int(11) NOT NULL,
  `targetGroupIDs` varchar(500) NOT NULL,
  `stackingPenalty` int(11) NOT NULL,
  `effectState` int(11) NOT NULL,
  `targetType` int(11) NOT NULL,
  `targetGroup` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `dgmEffectsInfo`
--
/*          this is the data for 'targetType' field.
    // 0: zero value.  undefined
    EFFECT_UNDEFINED            = 0,
    // 1: the target is the ship to which the module is fitted
    EFFECT_SHIP                 = 1,
    // 2: the target is a module fit to same ship. use 'targetGroupIDs' to decode affected groups
    EFFECT_MODULE               = 2,
    // 3: the target is a loaded charge.  use 'targetGroupIDs' to decode affected groups of loaded charges
    EFFECT_LOADED_CHARGE        = 3,
    // 4: the target is the current target of the ship to which the module is fitted
    EFFECT_TARGET               = 4,
    // 5: the target is a loaded module  - this could use EFFECT_MODULE
    EFFECT_CHARGE               = 5,
    // 6: the target of the effect is the module's own attribute(s)  - maybe unused
    EFFECT_TARGET_SELF          = 6,
    // 7: the effect acts upon the character's attribute specific to the effect  - maybe unused.
    EFFECT_CHARACTER            = 7
    */
INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenalty`, `effectState`, `targetType`, `targetGroup`)
VALUES
-- ID, src, targ, calc, des, rcalc, tgrpID, stack, state, targetType, targetGroup
-- common for all modules
(16, 30, 2, 1, 'PG -> PG_Used', 8, '6', 0, 2, 1, 0),
(16, 50, 49, 1, 'CPU -> CPU_Used', 8, '6', 0, 2, 1, 0),
(2663, 1153, 1152, 1, 'Cal -> Cal-Used', 8, '6', 0, 2, 1, 0),
-- hp
(60, 150, 9, 5, 'Reinforced Bulkhead - HP Bonus', 25, '6', 0, 2, 1, 78),
(60, 150, 9, 5, 'NF Structure - HP Bonus', 25, '6', 0, 2, 1, 763),
(3047, 150, 9, 5, 'Exp Cargohold - HP Penalty', 25, '6', 0, 2, 1, 765),
-- cpu output
(536, 202, 48, 5, 'Co-Processor - CPU Output Bonus', 25, '6', 0, 2, 1, 285),
-- pg
(56, 145, 11, 5, 'Power Diagnostic - PG', 25, '6', 0, 2, 1, 766),
(56, 145, 11, 5, 'Reactor Control - PG', 25, '6', 0, 2, 1, 769),
(627, 549, 11, 1, 'Auxiliary Power Core - PG', 8, '6', 0, 2, 1, 339),
-- cap capacity
(25, 67, 482, 1, 'Capacitor Battery - Cap Capacity', 8, '6', 0, 2, 1, 61),
(58, 147, 482, 5, 'AB/MWD Cap Penalty', 25, '6', 0, 2, 1, 46),
(58, 147, 482, 5, 'Cap Flux Coil - Cap Capacity', 25, '6', 0, 2, 1, 768),
(58, 147, 482, 5, 'Power Diagnostic - Cap Capacity', 25, '6', 0, 2, 1, 766),
-- cap recharge
(51, 144, 55, 5, 'Cap Flux Coil - Cap Recharge Rate', 25, '6', 0, 2, 1, 768),
(51, 144, 55, 5, 'Cap Power Relay - Cap Recharge Rate', 25, '6', 0, 2, 1, 767),
(51, 144, 55, 5, 'Cap Recharger - Cap Recharge Rate', 25, '6', 0, 2, 1, 43),
(51, 144, 55, 5, 'Power Diagnostic - Cap Recharge Rate', 25, '6', 0, 2, 1, 766),
(51, 144, 55, 5, 'Power Relay - Cap Recharge Rate', 25, '6', 0, 2, 1, 57),
-- inertia mods
(657, 169, 70, 56, 'NF Structure - Inertia Bonus', 57, '6', 1, 2, 1, 763),
(657, 169, 70, 56, 'Reinforced Bulkhead - Inertia Penalty', 57, '6', 1, 2, 1, 78),
(657, 169, 70, 56, 'IStab - Inertia Modifier', 57, '6', 1, 2, 1, 762),
-- sig radius mods
(1254, 554, 552, 54, 'MWD Sig Radius Penalty', 55, '6', 1, 12, 1, 46),
(2029, 983, 552, 1, 'Shield Extenders - Sig Radius Penality', 8, '6', 1, 2, 1, 38),
(2644, 554, 552, 54, 'IStab - Sig Radius Penalty', 55, '6', 1, 2, 1, 762),
-- mass mods
(1254, 796, 4, 1, 'AB/MWD Mass addition', 8, '6', 1, 12, 1, 46),
(1959, 796, 4, 1, 'Armor Plates - Mass Addition', 8, '6', 0, 2, 1, 329),
-- cargo cap
(59, 149, 38, 5, 'Exp Cargohold - Cargo Cap Bonus', 25, '6', 0, 2, 1, 765),
(59, 149, 38, 5, 'OD Injector - Cargo Cap Penalty', 25, '6', 0, 2, 1, 764),
-- velocity
(710, 20, 37, 54, 'AB/MWD Velocity Bonus', 55, '6', 1, 12, 1, 46),
(854, 306, 37, 5, 'Cloaking Device - Velocity Penalty', 25, '6', 1, 2, 1, 330),
(2865, 1076, 37, 54, 'NF Structure - Velocity Bonus', 55, '6', 1, 2, 1, 763),
(2859, 306, 37, 5, 'Reinforced Bulkhead - Velocity Penalty', 25, '6', 1, 2, 1, 78),
(2865, 1076, 37, 54, 'OD Injector - Velocity Bonus', 55, '6', 1, 2, 1, 764),
(3046, 306, 37, 5, 'Exp Cargohold - Velocity Penalty', 25, '6', 1, 2, 1, 765),
-- warp core str
(670, 105, 21, 1, 'Warp Stab - Strength Bonus', 8, '6', 0, 2, 1, 315),
(3725, 105, 21, 1, 'Warp Core Strength', 8, '6', 0, 2, 1, 52),
-- target range
(2646, 309, 76, 56, 'Warp Stab - Targeting Range Penalty', 57, '6', 1, 2, 1, 315),
-- scan res
(854, 565, 564, 5, 'Cloaking Device - Scan Resolution', 25, '6', 1, 2, 1, 330),
(2645, 565, 564, 5, 'Warp Stab - Scan Resolution Penalty', 25, '6', 1, 2, 1, 315),

-- Damage Control Unit
(2302, 267, 267, 30, 'Armor EM Resistance Bonus', 31, '6', 0, 12, 1, 60),
(2302, 268, 268, 30, 'Armor Exp Resistance Bonus', 31, '6', 0, 12, 1, 60),
(2302, 269, 269, 30, 'Armor Kin Resistance Bonus', 31, '6', 0, 12, 1, 60),
(2302, 270, 270, 30, 'Armor Therm Resistance Bonus', 31, '6', 0, 12, 1, 60),
(2302, 271, 271, 30, 'Shield EM Resistance Bonus', 31, '6', 0, 12, 1, 60),
(2302, 272, 272, 30, 'Shield Exp Resistance Bonus', 31, '6', 0, 12, 1, 60),
(2302, 273, 273, 30, 'Shield Kin Resistance Bonus', 31, '6', 0, 12, 1, 60),
(2302, 274, 274, 30, 'Shield Therm Resistance Bonus', 31, '6', 0, 12, 1, 60),
(2302, 974, 113, 30, 'Hull EM Resistance Bonus', 31, '6', 0, 12, 1, 60),
(2302, 975, 111, 30, 'Hull Exp Resistance Bonus', 31, '6', 0, 12, 1, 60),
(2302, 976, 109, 30, 'Hull Kin Resistance Bonus', 31, '6', 0, 12, 1, 60),
(2302, 977, 110, 30, 'Hull Therm Resistance Bonus', 31, '6', 0, 12, 1, 60);

-- unsorted (and untested and unchecked)
(118, 235, 192, 1, 'Auto Targeter - Max Locked Targets', 8, '6', 0, 14, 1, 96),
(31, 90, 6, 8, 'Energy Transferred', 1, '0', 0, 16, 1, 67),
(2303, 97, 6, 8, 'Energy Transferred', 1, '0', 0, 16, 1, 71),
(3250, 90, 6, 8, 'Energy Transferred', 1, '0', 0, 12, 1, 68),

-- these below are not correct and/or not working (or not working right, or not implemented)
-- strictly overload effects (not completely implemented)
(4044, 1223, 20, 54, 'AB/MWD Overload Speed Bonus', 55, '6', 1, 8, 1, 46),

-- Warp Disruption Field (899) and Script (908)
(804, 317, 6, 0, 'Warp Disruption Field – Capacitor Need', 0, '0', 0, 15, 1, 908),
(3380, 1131, 4, 56, 'Warp Disruption Field – Mass Reduction', 57, '6', 1, 12, 1, 899),
(3602, 66, 73, 0, 'Warp Disruption Field – Duration Bonus', 0, '0', 0, 15, 1, 908),
(3615, 1227, 554, 1, 'Warp Disruption Field – Signature Radius', 8, '6', 0, 14, 1, 899),
(3617, 1227, 554, 1, 'Warp Disruption Field – Signature Radius', 8, '0', 0, 15, 1, 908),
(3618, 1324, 1131, 0, 'Warp Disruption Field – Mass Reduction', 0, '0', 0, 15, 1, 908),
(3619, 1326, 1270, 0, 'Warp Disruption Field – AB/MWD Thrust', 0, '0', 0, 15, 1, 908),
(3620, 1325, 1164, 0, 'Warp Disruption Field – AB/MWD Max Velocity', 0, '0', 0, 15, 1, 908),
(3648, 1327, 103, 0, 'Warp Disruption Field – Scramble Range', 0, '0', 0, 15, 1, 908),
(3380, 1164, 20, 56, 'AB/MWD Max Velocity Penalty', 57, '46', 1, 12, 1, 899),
(3380, 1270, 567, 56, 'AB/MWD Thrust Penalty', 57, '46', 1, 12, 1, 899),
-- 4894 effectID for type 899

-- effects below affect other modules or objects and are NOT implemented yet...

-- 'thrust' is an odd factor used in calculations
-- (710, 567, 0, -1, 'AB/MWD Thrust', -1, '6', 0, 12, 1, 46),

-- drone effects
(2309, 567, 37, 0, 'Drone Navigation Computer - Drone Velocity', 0, '18', 0, 15, 1, 644),
(2247, 353, 352, 1, 'Drone Control Unit - Drone Bonus', 8, '6', 0, 4, 1, 407),
(2000, 459, 459, 1, 'Drone Link Augmenter - Drone Control Range Bonus', 8, '6', 0, 15, 1, 647),
-- module range
(3174, 1222, 54, 0, 'Stasis Web Range Bonus', 0, '0', 0, 8, 1, 65),
(3725, 1222, 54, 0, 'Warp Scrambler Range Bonus', 0, '0', 0, 8, 1, 52),
(4162, 846, 1371, 0, 'Scan Probe Strength Bonus', 0, '479', 0, 4, 1, 481),
-- module duration
(3002, 1206, 73, 54, 'Cap Booster Duration', 55, '0', 0, 8, 1, 76),
(3002, 1206, 73, 54, 'Energy Neut Duration', 55, '0', 1, 8, 1, 71),
(3002, 1206, 73, 54, 'Energy Transfer Duration', 55, '0', 1, 8, 1, 67),
(3002, 1206, 73, 54, 'Energy Vampire Duration', 55, '0', 1, 8, 1, 68),
(3002, 1206, 73, 54, 'Hull Rep Duration', 55, '0', 1, 8, 1, 63),
(1200, 782, 77, 5, 'Mining Crystal - Mining Amount', 25, '483', 0, 15, 1, 482),
(804, 317, 6, 0, 'Mining Crystal - Capacitor Penalty', 0, '483', 0, 15, 1, 482),
(1200, 782, 77, 5, 'Mercoxit Mining Crystal - Mining Amount', 25, '483', 0, 15, 1, 663),
(804, 317, 6, 0, 'Mercoxit Mining Crystal - Capacitor Penalty', 0, '483', 0, 15, 1, 663),
(4559, 349, 158, 0, 'Falloff', 0, '55;53;74', 1, 4, 1, 213),
(4559, 351, 54, 0, 'Optimal Range', 0, '55;53;74', 1, 4, 1, 213),
(4559, 767, 160, 0, 'Tracking Speed', 0, '55;53;74', 1, 4, 1, 213),
(4527, 349, 158, 0, 'Falloff', 0, '55;53;74', 1, 14, 1, 211),
(3655, 351, 54, 0, 'Optimal Range', 0, '55;53;74', 1, 14, 1, 211),
(3656, 767, 160, 0, 'Tracking Speed', 0, '55;53;74', 1, 14, 1, 211),
(4560, 349, 158, 0, 'Falloff', 0, '55;53;74', 1, 16, 1, 209),
(4560, 351, 54, 0, 'Optimal Range', 0, '55;53;74', 1, 16, 1, 209),
(4560, 767, 160, 0, 'Tracking Speed', 0, '55;53;74', 1, 16, 1, 209),
(2444, 1082, 50, 0, 'CPU Penalty', 0, '1039;1040', 0, 14, 1, 546),
(2479, 780, 73, 0, 'Cycle Time', 0, '1038', 0, 14, 1, 546),
(2445, 1082, 50, 0, 'CPU Penalty', 0, '1038', 0, 14, 1, 546),
(2252, 1034, 669, 4, 'Cloak Reactivation Delay', 24, '330', 0, 15, 1, 830),
-- mining upgrades (module-affecting)
(1882, 434, 77, 54, 'Mining Amount Bonus', 55, '54;483', 0, 3, 1, 546),
(2445, 1082, 50, 56, 'Module CPU Usage Penality', 57, '54;483', 0, 3, 1, 546),
(2479, 780, 73, 56, 'Module Cycle Time Bonus', 57, '54;483', 0, 3, 1, 546);
