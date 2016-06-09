
--
-- Table structure for table `dgmEffectsInfo`
--

CREATE TABLE IF NOT EXISTS `dgmEffectsInfo` (
  `effectID` int(11) DEFAULT NULL,
  `sourceAttributeID` int(11) NOT NULL,
  `targetAttributeID` int(11) NOT NULL,
  `calculationTypeID` int(11) NOT NULL,
  `description` varchar(500) NOT NULL,
  `reverseCalculationTypeID` int(11) NOT NULL,
  `targetGroupIDs` varchar(500) NOT NULL,
  `stackingPenaltyApplied` int(11) NOT NULL,
  `effectAppliedInState` int(11) NOT NULL,
  `affectingID` int(11) NOT NULL,
  `affectingType` int(11) NOT NULL,
  `affectedType` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `dgmEffectsInfo`
--

INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenaltyApplied`, `effectAppliedInState`, `affectingID`, `affectingType`, `affectedType`)
VALUES
-- the following effects are sorted and working (these also include overload effects that need to be removed due to separate sourceAttributeID)
(16, 30, 15, 1, 'PG -> PG_Used', 8, '6', 0, 2, 0, 0, 3),
(16, 50, 49, 1, 'CPU -> CPU_Used', 8, '6', 0, 2, 0, 0, 3),

-- hp
(60, 150, 9, 5, 'Structure HP Bonus', 25, '6', 0, 14, 763, 2, 3),
(60, 150, 9, 5, 'Structure HP Bonus', 25, '6', 0, 14, 78, 2, 3),
(3047, 150, 9, 5, 'Structure HP Penalty', 25, '6', 0, 14, 765, 2, 3),
-- cpu output
(536, 202, 48, 5, 'Co-Processor - CPU Output Bonus', 25, '6', 0, 14, 285, 2, 3),
-- pg
(627, 549, 11, 1, 'Auxiliary Power Core - PG', 8, '6', 0, 14, 339, 2, 3),
(56, 145, 11, 5, 'Power Diagnostic - PG', 25, '6', 0, 14, 766, 2, 3),
(56, 145, 11, 5, 'Reactor Control - PG', 25, '6', 0, 14, 769, 2, 3),
-- cap capacity
(25, 67, 482, 1, 'Capacitor Battery - Cap Capacity', 8, '6', 0, 14, 61, 2, 3),
(58, 147, 482, 5, 'AB/MWD Cap Penalty', 25, '6', 0, 14, 46, 2, 3),
(58, 147, 482, 5, 'Cap Flux Coil - Cap Capacity', 25, '6', 0, 14, 768, 2, 3),
(58, 147, 482, 5, 'Power Diagnostic - Cap Capacity', 25, '6', 0, 14, 766, 2, 3),
-- cap recharge
(51, 144, 55, 5, 'Cap Flux Coil - Cap Recharge Rate', 25, '6', 0, 14, 768, 2, 3),
(51, 144, 55, 5, 'Cap Power Relay - Cap Recharge Rate', 25, '6', 0, 14, 767, 2, 3),
(51, 144, 55, 5, 'Cap Recharger - Cap Recharge Rate', 25, '6', 0, 14, 43, 2, 3),
(51, 144, 55, 5, 'Power Diagnostic - Cap Recharge Rate', 25, '6', 0, 14, 766, 2, 3),
-- inertia mods
(657, 169, 70, 56, 'Inertia Bonus', 57, '6', 1, 14, 763, 2, 3),
(657, 169, 70, 56, 'Inertia Penalty', 57, '6', 1, 14, 78, 2, 3),
(657, 169, 70, 56, 'Inertia Modifier', 57, '6', 1, 14, 762, 2, 3),
-- sig radius mods
(1254, 554, 552, 54, 'AB/MWD Sig Radius Penalty', 55, '6', 1, 12, 46, 2, 3),
(2644, 554, 552, 54, 'Sig Radius Penalty', 55, '6', 1, 14, 762, 2, 3),
-- mass mods
(1254, 796, 4, 1, 'AB/MWD Mass addition', 8, '6', 0, 12, 46, 2, 3),
-- cargo cap
(59, 149, 38, 5, 'Cargo Capacity Bonus', 25, '6', 0, 14, 765, 2, 3),
(59, 149, 38, 5, 'Cargo Capacity Penalty', 25, '6', 0, 14, 764, 2, 3),
-- velocity
(710, 20, 37, 54, 'AB/MWD Velocity Bonus', 55, '6', 0, 12, 46, 2, 3),
(854, 306, 37, 5, 'Cloaking Device - Max Velocity Penalty', 25, '6', 0, 4, 330, 2, 3),
(2865, 1076, 37, 54, 'Max Velocity Bonus', 55, '6', 1, 14, 763, 2, 3),
(2859, 306, 37, 5, 'Max Velocity Penalty', 25, '6', 1, 14, 78, 2, 3),
(2865, 1076, 37, 54, 'Max Velocity Bonus', 55, '6', 1, 14, 764, 2, 3),
(3046, 306, 37, 5, 'Max Velocity Penalty', 25, '6', 1, 14, 765, 2, 3),
(4575, 20, 37, 54, 'Max Velocity Bonus', 55, '6', 0, 4, 28583, 1, 3),
(4877, 20, 37, 54, 'Max Velocity Bonus', 55, '6', 0, 4, 515, 1, 3),
(4893, 20, 37, 54, 'Max Velocity Bonus', 55, '6', 0, 4, 515, 1, 3),
-- warp core str
(670, 105, 21, 1, 'Warp Core Strength', 8, '6', 0, 14, 315, 2, 3),
(3725, 105, 21, 1, 'Warp Core Strength', 8, '6', 0, 14, 52, 2, 3),

-- Damage Control Unit
(2302, 267, 267, 30, 'Armor EM Resistance Bonus', 31, '6', 2, 12, 60, 2, 3),
(2302, 268, 268, 30, 'Armor Exp Resistance Bonus', 31, '6', 2, 12, 60, 2, 3),
(2302, 269, 269, 30, 'Armor Kin Resistance Bonus', 31, '6', 2, 12, 60, 2, 3),
(2302, 270, 270, 30, 'Armor Therm Resistance Bonus', 31, '6', 2, 12, 60, 2, 3),
(2302, 271, 271, 30, 'Shield EM Resistance Bonus', 31, '6', 0, 12, 60, 2, 3),
(2302, 272, 272, 30, 'Shield Exp Resistance Bonus', 31, '6', 0, 12, 60, 2, 3),
(2302, 273, 273, 30, 'Shield Kin Resistance Bonus', 31, '6', 0, 12, 60, 2, 3),
(2302, 274, 274, 30, 'Shield Therm Resistance Bonus', 31, '6', 0, 12, 60, 2, 3),
(2302, 974, 113, 30, 'Hull EM Resistance Bonus', 31, '6', 0, 12, 60, 2, 3),
(2302, 975, 111, 30, 'Hull Exp Resistance Bonus', 31, '6', 0, 12, 60, 2, 3),
(2302, 976, 109, 30, 'Hull Kin Resistance Bonus', 31, '6', 0, 12, 60, 2, 3),
(2302, 977, 110, 30, 'Hull Therm Resistance Bonus', 31, '6', 0, 12, 60, 2, 3),

-- unsorted
(118, 235, 192, 1, 'Auto Targeter - Max Locked Targets', 8, '6', 0, 14, 96, 2, 3),
(854, 565, 564, 5, 'Cloaking Device - Scan Resolution', 25, '6', 0, 15, 330, 2, 3),
(2303, 97, 6, 8, 'Energy Neutralized', 1, '0', 0, 16, 71, 2, 6),
(31, 90, 6, 8, 'Energy Transferred', 1, '0', 0, 16, 67, 2, 6),
(3250, 90, 6, 8, 'Energy Transferred', 1, '0', 0, 12, 68, 2, 6),
(1720, 548, 68, 0, 'Shield Boost Bonus', 40, '40', 1, 14, 767, 2, 2),
(2646, 309, 76, 0, 'Targeting Range Penalty', 0, '6', 1, 14, 315, 2, 3),
(2645, 565, 564, 5, 'Scan Resolution Penalty', 25, '6', 1, 14, 315, 2, 3),
(4162, 846, 1371, 0, 'Scan Probe Strength Bonus', 0, '479', 0, 4, 481, 2, 2),

-- these below are not correct and/or not working (or not working right, or not implemented)
-- strictly overload effects (not completely implemented)
(4044, 1223, 20, 54, 'AB/MWD Overload Speed Bonus', 55, '6', 1, 8, 46, 2, 6),
(3025, 1205, 51, 0, 'Energy Weapon ROF Bonus', 40, '0', 0, 8, 53, 2, 6),
(3025, 1210, 64, 0, 'Energy Weapon Damage Bonus', 40, '0', 0, 8, 53, 2, 6),

-- module effects also used by skills
(272, 312, 73, 0, 'Repair Systems - Duration Bonus', 0, '1049;1050;1051', 0, 15, 773, 2, 4),
(490, 313, 11, 0, 'Engineering - Power Output Bonus', 0, '6', 0, 15, 781, 2, 3),
(394, 315, 37, 0, 'Navigation - Velocity Bonus', 0, '6', 1, 15, 782, 2, 3),
(227, 317, 6, 0, 'Acceration Control - Cap Need Bonus', 0, '46', 0, 15, 782, 2, 2),
(271, 335, 265, 0, 'Hull Upgrade - Armor Hp Bonus', 40, '6', 0, 15, 773, 2, 3),
(623, 434, 77, 0, 'Mining Drone Operation - Mining Amount Bonus', 0, '18', 0, 15, 778, 2, 3),

-- Warp Disruption Field (899) and Script (908)
(804, 317, 6, 0, 'Warp Disruption Field – Capacitor Need', 0, '0', 0, 15, 908, 2, 6),
(3380, 1131, 4, 56, 'Warp Disruption Field – Mass Reduction', 57, '6', 1, 12, 899, 2, 3),
(3602, 66, 73, 0, 'Warp Disruption Field – Duration Bonus', 0, '0', 0, 15, 908, 2, 6),
(3615, 1227, 554, 1, 'Warp Disruption Field – Signature Radius', 8, '6', 0, 14, 899, 2, 3),
(3617, 1227, 554, 1, 'Warp Disruption Field – Signature Radius', 8, '0', 0, 15, 908, 2, 6),
(3618, 1324, 1131, 0, 'Warp Disruption Field – Mass Reduction', 0, '0', 0, 15, 908, 2, 6),
(3619, 1326, 1270, 0, 'Warp Disruption Field – AB/MWD Thrust', 0, '0', 0, 15, 908, 2, 6),
(3620, 1325, 1164, 0, 'Warp Disruption Field – AB/MWD Max Velocity', 0, '0', 0, 15, 908, 2, 6),
(3648, 1327, 103, 0, 'Warp Disruption Field – Scramble Range', 0, '0', 0, 15, 908, 2, 6),
-- 4894 effectID for type 899

-- NOTE:  effects below affect other modules or objects and are NOT implemented yet...
-- 'thrust' is an odd factor used in calculations
-- (710, 567, 0, -1, 'AB/MWD Thrust', -1, '6', 0, 12, 46, 2, 3),

-- drone effects
(2309, 567, 37, 0, 'Drone Navigation Computer - Drone Velocity', 0, '18', 0, 15, 644, 2, 3),
(2247, 353, 352, 1, 'Drone Control Unit - Drone Bonus', 8, '6', 0, 4, 407, 2, 3),
(2000, 459, 459, 1, 'Drone Link Augmenter - Drone Control Range Bonus', 8, '6', 0, 15, 647, 2, 3),
-- module range
(3174, 1222, 54, 0, 'Stasis Web Range Bonus', 0, '0', 0, 8, 65, 2, 6),
(3725, 1222, 54, 0, 'Warp Scrambler Range Bonus', 0, '0', 0, 8, 52, 2, 6),
-- module duration
(2850, 66, 73, 54, 'AB/MWD Duration Bonus', 55, '46', 0, 15, 782, 2, 2),
(3380, 1164, 20, 56, 'AB/MWD Max Velocity Penalty', 57, '46', 1, 12, 899, 2, 2),
(3380, 1270, 567, 56, 'AB/MWD Thrust Penalty', 57, '46', 1, 12, 899, 2, 2),
(3002, 1206, 73, 54, 'Cap Booster Duration', 55, '0', 0, 8, 76, 2, 6),
(3002, 1206, 73, 54, 'Energy Neut Duration', 55, '0', 1, 8, 71, 2, 6),
(3002, 1206, 73, 54, 'Energy Transfer Duration', 55, '0', 1, 8, 67, 2, 6),
(3002, 1206, 73, 54, 'Energy Vampire Duration', 55, '0', 1, 8, 68, 2, 6),
(3002, 1206, 73, 54, 'Hull Rep Duration', 55, '0', 1, 8, 63, 2, 6),
(1200, 782, 77, 5, 'Mining Crystal - Mining Amount', 25, '483', 0, 15, 482, 2, 6),
(804, 317, 6, 0, 'Mining Crystal - Capacitor Penalty', 0, '483', 0, 15, 482, 2, 6),
(1200, 782, 77, 5, 'Mercoxit Mining Crystal - Mining Amount', 25, '483', 0, 15, 663, 2, 6),
(804, 317, 6, 0, 'Mercoxit Mining Crystal - Capacitor Penalty', 0, '483', 0, 15, 663, 2, 6),
(4893, 1190, 6, 0, 'EW Capacitor Need Bonus', 0, '201;80;208;65;379;291;52', 0, 4, 515, 1, 2),
(4893, 1191, 352, 0, 'Drone Control Bonus', 0, '6', 0, 4, 515, 1, 3),
(4893, 1201, 83, 0, 'Remote Hull Repair Bonus', 40, '585', 0, 4, 515, 1, 2),
(4893, 1202, 73, 5, 'Remote Hull Repair Duration Bonus', 25, '585', 0, 4, 515, 1, 2),
(4893, 1203, 90, 0, 'Energy Transfer Bonus', 0, '67', 0, 4, 515, 1, 2),
(4893, 1204, 73, 0, 'Energy Transfer Duration Bonus', 0, '67', 0, 4, 515, 1, 2),
(4893, 1802, 6, 0, 'Remote Armor Repair Cap Use', 0, '325', 0, 4, 515, 1, 2),
(4893, 1802, 6, 0, 'Remote Hull Repair Cap Use', 0, '585', 0, 4, 515, 1, 2),
(4893, 1802, 6, 0, 'Energy Transfer Cap Use', 0, '67', 0, 4, 515, 1, 2),
(4559, 349, 158, 0, 'Falloff', 0, '55;53;74', 1, 4, 213, 2, 2),
(4559, 351, 54, 0, 'Optimal Range', 0, '55;53;74', 1, 4, 213, 2, 2),
(4559, 767, 160, 0, 'Tracking Speed', 0, '55;53;74', 1, 4, 213, 2, 2),
(4527, 349, 158, 0, 'Falloff', 0, '55;53;74', 1, 14, 211, 2, 2),
(3655, 351, 54, 0, 'Optimal Range', 0, '55;53;74', 1, 14, 211, 2, 2),
(3656, 767, 160, 0, 'Tracking Speed', 0, '55;53;74', 1, 14, 211, 2, 2),
(4560, 349, 158, 0, 'Falloff', 0, '55;53;74', 1, 16, 209, 2, 2),
(4560, 351, 54, 0, 'Optimal Range', 0, '55;53;74', 1, 16, 209, 2, 2),
(4560, 767, 160, 0, 'Tracking Speed', 0, '55;53;74', 1, 16, 209, 2, 2),
(2444, 1082, 50, 0, 'CPU Penalty', 0, '1039;1040', 0, 14, 546, 2, 4),
(2479, 780, 73, 0, 'Cycle Time', 0, '1038', 0, 14, 546, 2, 4),
(2445, 1082, 50, 0, 'CPU Penalty', 0, '1038', 0, 14, 546, 2, 4),
(212, 310, 50, 0, 'CPU Need Bonus', 0, '3432', 0, 15, 780, 2, 5),
(1500, 851, 6, 0, 'Capacitor Need Bonus', 40, '3416', 0, 15, 774, 2, 5),
(273, 323, 30, 0, 'Upgrade Power Need Bonus', 40, '3425', 0, 15, 774, 2, 5),
(4162, 846, 1371, 0, 'Sensor Strength Modifier', 0, '479', 0, 15, 780, 2, 2),
(2252, 1034, 669, 4, 'Cloak Reactivation Delay', 24, '330', 0, 15, 830, 3, 2);
