--  Original file and idea by Luck and Aknor Jaden.
--  Updates/Rewrites and Implementation by Allan

--  NOTE:  this is the currently working module effects (129 total)

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
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `dgmEffectsInfo`
--

INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenalty`, `effectState`, `targetType`, `targetGroup`)
VALUES
-- ID, src, targ, calc, des, rcalc, tgrpID, stack, state, targetType, targetGroup
-- common for all modules
(16, 30, 15, 1, 'PG -> PG_Used', 2, '6', 0, 2, 1, 0),
(16, 50, 49, 1, 'CPU -> CPU_Used', 2, '6', 0, 2, 1, 0),
(2663, 1153, 1152, 1, 'Cal -> Cal-Used', 2, '6', 0, 2, 1, 0),
-- hp
(60, 150, 9, 3, 'Reinforced Bulkhead - HP Bonus', 4, '6', 0, 2, 1, 78),
(60, 150, 9, 3, 'NF Structure - HP Bonus', 4, '6', 0, 2, 1, 763),
(3047, 150, 9, 3, 'Exp Cargohold - HP Penalty', 4, '6', 0, 2, 1, 765),
-- cpu output
(536, 202, 48, 3, 'Co-Processor - CPU Output Bonus', 4, '6', 0, 2, 1, 285),
-- pg
(56, 145, 11, 3, 'Power Diagnostic - PG', 4, '6', 0, 2, 1, 766),
(56, 145, 11, 3, 'Reactor Control - PG', 4, '6', 0, 2, 1, 769),
(627, 549, 11, 1, 'Auxiliary Power Core - PG', 2, '6', 0, 2, 1, 339),
-- cap capacity
(25, 67, 482, 1, 'Capacitor Battery - Cap Capacity', 2, '6', 0, 2, 1, 61),
(58, 147, 482, 3, 'AB/MWD Cap Penalty', 4, '6', 0, 2, 1, 46),
(58, 147, 482, 3, 'Cap Flux Coil - Cap Capacity', 4, '6', 0, 2, 1, 768),
(58, 147, 482, 3, 'Power Diagnostic - Cap Capacity', 4, '6', 0, 2, 1, 766),
-- cap recharge
(51, 144, 55, 3, 'Cap Flux Coil - Cap Recharge Rate', 4, '6', 0, 2, 1, 768),
(51, 144, 55, 3, 'Cap Power Relay - Cap Recharge Rate', 4, '6', 0, 2, 1, 767),
(51, 144, 55, 3, 'Cap Recharger - Cap Recharge Rate', 4, '6', 0, 2, 1, 43),
(51, 144, 55, 3, 'Power Diagnostic - Cap Recharge Rate', 4, '6', 0, 2, 1, 766),
(51, 144, 55, 3, 'Power Relay - Cap Recharge Rate', 4, '6', 0, 2, 1, 57),
-- inertia mods
(657, 169, 70, 5, 'NF Structure - Inertia Bonus', 6, '6', 1, 2, 1, 763),
(657, 169, 70, 3, 'Reinforced Bulkhead - Inertia Penalty', 4, '6', 1, 2, 1, 78),
(657, 169, 70, 5, 'IStab - Inertia Modifier', 6, '6', 1, 2, 1, 762),
-- sig radius mods
(1254, 554, 552, 5, 'MWD Sig Radius Penalty', 6, '6', 1, 12, 1, 46),
(2029, 983, 552, 5, 'Shield Extenders - Sig Radius Penality', 6, '6', 1, 2, 1, 38),
(2644, 554, 552, 5, 'IStab - Sig Radius Penalty', 6, '6', 1, 2, 1, 762),
-- mass mods
(1254, 796, 4, 1, 'AB/MWD Mass addition', 2, '6', 0, 12, 1, 46),
(1959, 796, 4, 1, 'Armor Plates - Mass Addition', 2, '6', 0, 2, 1, 329),
-- cargo cap
(59, 149, 38, 3, 'Exp Cargohold - Cargo Cap Bonus', 4, '6', 0, 2, 1, 765),
(59, 149, 38, 3, 'OD Injector - Cargo Cap Penalty', 4, '6', 0, 2, 1, 764),
-- velocity
(710, 20, 37, 5, 'AB/MWD Velocity Bonus', 6, '6', 1, 12, 1, 46),
(854, 306, 37, 3, 'Cloaking Device - Velocity Penalty', 4, '6', 1, 2, 1, 330),
(2865, 1076, 37, 5, 'NF Structure - Velocity Bonus', 6, '6', 1, 2, 1, 763),
(2859, 306, 37, 3, 'Reinforced Bulkhead - Velocity Penalty', 4, '6', 1, 2, 1, 78),
(2865, 1076, 37, 5, 'OD Injector - Velocity Bonus', 6, '6', 1, 2, 1, 764),
(3046, 306, 37, 3, 'Exp Cargohold - Velocity Penalty', 4, '6', 1, 2, 1, 765),
-- warp core str
(670, 105, 21, 1, 'Warp Stab - Strength Bonus', 2, '6', 0, 2, 1, 315),
(3725, 105, 21, 1, 'Warp Scram - Strength Penality', 2, '6', 0, 2, 1, 52),
-- target range
(2646, 309, 76, 5, 'Warp Stab - Targeting Range Penalty', 6, '6', 1, 2, 1, 315),
-- scan res
(854, 565, 564, 3, 'Cloaking Device - Scan Resolution', 4, '6', 1, 2, 1, 330),
(2645, 565, 564, 3, 'Warp Stab - Scan Resolution Penalty', 4, '6', 1, 2, 1, 315),

-- Damage Control Unit
(2302, 267, 267, 9, 'Armor EM Resistance Bonus', 10, '6', 0, 12, 1, 60),
(2302, 268, 268, 9, 'Armor Exp Resistance Bonus', 10, '6', 0, 12, 1, 60),
(2302, 269, 269, 9, 'Armor Kin Resistance Bonus', 10, '6', 0, 12, 1, 60),
(2302, 270, 270, 9, 'Armor Therm Resistance Bonus', 10, '6', 0, 12, 1, 60),
(2302, 271, 271, 9, 'Shield EM Resistance Bonus', 10, '6', 0, 12, 1, 60),
(2302, 272, 272, 9, 'Shield Exp Resistance Bonus', 10, '6', 0, 12, 1, 60),
(2302, 273, 273, 9, 'Shield Kin Resistance Bonus', 10, '6', 0, 12, 1, 60),
(2302, 274, 274, 9, 'Shield Therm Resistance Bonus', 10, '6', 0, 12, 1, 60),
(2302, 974, 113, 9, 'Hull EM Resistance Bonus', 10, '6', 0, 12, 1, 60),
(2302, 975, 111, 9, 'Hull Exp Resistance Bonus', 10, '6', 0, 12, 1, 60),
(2302, 976, 109, 9, 'Hull Kin Resistance Bonus', 10, '6', 0, 12, 1, 60),
(2302, 977, 110, 9, 'Hull Therm Resistance Bonus', 10, '6', 0, 12, 1, 60);


-- shield effects data
INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenalty`, `effectState`, `targetType`, `targetGroup`)
VALUES
-- shield cap
(21, 72, 263, 1, 'Extenders - HP', 2, '6', 0, 2, 1, 38),
(57, 146, 263, 3, 'PDU - HP', 4, '6', 0, 2, 1, 766),
(57, 146, 263, 3, 'Flux Coil - HP', 4, '6', 0, 2, 1, 770),
-- shield recharge
(50, 134, 479, 3, 'PDU - Recharge Rate', 4, '6', 1, 2, 1, 766),
(50, 134, 479, 3, 'Flux Coil - Recharge Rate', 4, '6', 1, 2, 1, 770),
(50, 134, 479, 3, 'Power Relay - Recharge Rate', 4, '6', 1, 2, 1, 57),
(50, 134, 479, 3, 'Recharger - Recharge Rate', 4, '6', 1, 2, 1, 39),
-- active shield resists
(2052, 984, 271, 5, 'Amplifier - Passive EM Resist', 6, '6', 1, 2, 1, 295),
(2052, 985, 272, 5, 'Amplifier - Passive Exp Resist', 6, '6', 1, 2, 1, 295),
(2052, 986, 273, 5, 'Amplifier - Passive Kin Resist', 6, '6', 1, 2, 1, 295),
(2052, 987, 274, 5, 'Amplifier - Passive Therm Resist', 6, '6', 1, 2, 1, 295),
(2118, 984, 271, 5, 'Hardener - Active EM Resist', 6, '6', 1, 12, 1, 77),
(2118, 985, 272, 5, 'Hardener - Active Exp Resist', 6, '6', 1, 12, 1, 77),
(2118, 986, 273, 5, 'Hardener - Active Kin Resist', 6, '6', 1, 12, 1, 77),
(2118, 987, 274, 5, 'Hardener - Active Therm Resist', 6, '6', 1, 12, 1, 77),
-- passive shield resists
(2117, 994, 271, 7, 'Hardener - Passive EM Resist', 8, '6', 0, 2, 1, 77),
(2117, 995, 272, 7, 'Hardener - Passive Exp Resist', 8, '6', 0, 2, 1, 77),
(2117, 996, 273, 7, 'Hardener - Passive Kin Resist', 8, '6', 0, 2, 1, 77),
(2117, 997, 274, 7, 'Hardener - Passive Therm Resist', 8, '6', 0, 2, 1, 77),
-- overload bonus
(3035, 1208, 271, 5, 'Hardener - Overload EM Resist', 6, '6', 0, 8, 1, 77),
(3035, 1208, 272, 5, 'Hardener - Overload Exp Resist', 6, '6', 0, 8, 1, 77),
(3035, 1208, 273, 5, 'Hardener - Overload Kin Resist', 6, '6', 0, 8, 1, 77),
(3035, 1208, 274, 5, 'Hardener - Overload Therm Resist', 6, '6', 0, 8, 1, 77);

-- armor effects data
INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenalty`, `effectState`, `targetType`, `targetGroup`)
VALUES
-- armor hp
(63, 148, 265, 3, 'Coating - HP', 4, '6', 0, 2, 1, 98),
(63, 148, 265, 3, 'Plating - HP', 4, '6', 0, 2, 1, 326),
(2837, 1159, 265, 1, 'Plates - HP', 2, '6', 0, 2, 1, 329),
-- active armor resists
(2041, 984, 267, 5, 'Plating - Passive EM Resist', 6, '6', 1, 2, 1, 98),
(2041, 985, 268, 5, 'Plating - Passive Exp Resist', 6, '6', 1, 2, 1, 98),
(2041, 986, 269, 5, 'Plating - Passive Kin Resist', 6, '6', 1, 2, 1, 98),
(2041, 987, 270, 5, 'Plating - Passive Therm Resist', 6, '6', 1, 2, 1, 98),
(2041, 984, 267, 5, 'Coating - Passive EM Resist', 6, '6', 1, 2, 1, 326),
(2041, 985, 268, 5, 'Coating - Passive Exp Resist', 6, '6', 1, 2, 1, 326),
(2041, 986, 269, 5, 'Coating - Passive Kin Resist', 6, '6', 1, 2, 1, 326),
(2041, 987, 270, 5, 'Coating - Passive Therm Resist', 6, '6', 1, 2, 1, 326),
(2098, 984, 267, 5, 'Hardener - Active EM Resist', 6, '6', 1, 12, 1, 328),
(2098, 985, 268, 5, 'Hardener - Active Exp Resist', 6, '6', 1, 12, 1, 328),
(2098, 986, 269, 5, 'Hardener - Active Kin Resist', 6, '6', 1, 12, 1, 328),
(2098, 987, 270, 5, 'Hardener - Active Therm Resist', 6, '6', 1, 12, 1, 328),
-- passive armor resists
(2084, 994, 267, 7, 'Hardener - Passive EM Resist', 8, '6', 0, 2, 1, 328),
(2084, 995, 268, 7, 'Hardener - Passive Exp Resist', 8, '6', 0, 2, 1, 328),
(2084, 996, 269, 7, 'Hardener - Passive Kin Resist', 8, '6', 0, 2, 1, 328),
(2084, 997, 270, 7, 'Hardener - Passive Therm Resist', 8, '6', 0, 2, 1, 328),
-- overload bonus
(3029, 1208, 267, 5, 'Hardener - Overload EM Resist', 6, '6', 0, 8, 1, 328),
(3031, 1208, 268, 5, 'Hardener - Overload Exp Resist', 6, '6', 0, 8, 1, 328),
(3032, 1208, 269, 5, 'Hardener - Overload Kin Resist', 6, '6', 0, 8, 1, 328),
(3030, 1208, 270, 5, 'Hardener - Overload Therm Resist', 6, '6', 0, 8, 1, 328),
(4039, 1208, 267, 5, 'Hardener - Overload EM Resist', 6, '6', 0, 8, 1, 328),
(4039, 1208, 268, 5, 'Hardener - Overload Exp Resist', 6, '6', 0, 8, 1, 328),
(4039, 1208, 269, 5, 'Hardener - Overload Kin Resist', 6, '6', 0, 8, 1, 328),
(4039, 1208, 270, 5, 'Hardener - Overload Therm Resist', 6, '6', 0, 8, 1, 328);

-- rig effects data
INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenalty`, `effectState`, `targetType`, `targetGroup`)
VALUES
-- drawbacks
(2712, 1138, 265, 5, 'Rig Drawback - Armor HP', 6, '6', 0, 2, 1, 782),
(2713, 1138, 48, 5, 'Rig Drawback - CPU Output', 6, '6', 0, 2, 1, 778),
(2716, 1138, 552, 5, 'Rig Drawback - Sig Radius', 6, '6', 1, 2, 1, 774),
(2717, 1138, 37, 5, 'Rig Drawback - Max Velocity', 6, '6', 1, 2, 1, 773),
(2718, 1138, 263, 5, 'Rig Drawback - Shield Capacity', 6, '6', 0, 2, 1, 786),
(3528, 1138, 30, 5, 'Rig Drawback - Cap Recharge Rate', 6, '6', 0, 2, 1, 904),
-- cpu output
(397, 424, 48, 5, 'Engineering Rig - CPU Output Bonus', 6, '6', 0, 2, 1, 781),
-- cap recharge
(485, 314, 55, 5, 'Engineering Rig - Cap Recharge Rate', 6, '6', 0, 2, 1, 781),
-- pg
(490, 313, 11, 5, 'Engineering Rig - PG Output Bonus', 6, '6', 0, 2, 1, 781),
-- cap capacity
(2432, 1079, 482, 5, 'Engineering Rig - Cap Capacty', 6, '6', 0, 2, 1, 781),
-- velocity
(394, 315, 37, 5, 'Astronautic Rig - Velocity Bonus', 6, '6', 1, 2, 1, 782),
(3727, 1076, 37, 5, 'Astronautic Rig - Velocity Bonus', 6, '6', 1, 2, 1, 782),
-- inertia
(395, 151, 70, 5, 'Astronautic Rig - Inertia Bonus', 6, '6', 1, 2, 1, 782),
(3726, 169, 70, 5, 'Astronautic Rig - Inertia Bonus', 6, '6', 1, 2, 1, 782),
-- warp cap need
(494, 319, 153, 5, 'Astronautic Rig - Warp Cap Need Bonus', 6, '6', 0, 2, 1, 782),
-- warp speed
(856, 624, 1281, 5, 'Astronautic Rig - Warp Speed Bonus', 6, '6', 0, 2, 1, 782),
-- cargo
(836, 614, 38, 5, 'Astronautic Rig - Cargohold Bonus', 6, '6', 0, 2, 1, 782),
-- shield rigs
(486, 338, 479, 5, 'Shield Rig - Shield Recharge Bonus', 6, '6', 0, 2, 1, 774),
(2795, 984, 271, 5, 'Shield Rig - EM Resist', 6, '6', 1, 2, 1, 774),
(2795, 985, 272, 5, 'Shield Rig - Exp Resist', 6, '6', 1, 2, 1, 774),
(2795, 986, 273, 5, 'Shield Rig - Kin Resist', 6, '6', 1, 2, 1, 774),
(2795, 987, 274, 5, 'Shield Rig - Therm Resist', 6, '6', 1, 2, 1, 774),
-- armor rigs
(271, 335, 265, 5, 'Armor Rig - Hp Bonus', 6, '6', 0, 2, 1, 773),
(2792, 984, 267, 5, 'Armor Rig - EM Resist', 6, '6', 1, 2, 1, 773),
(2792, 985, 268, 5, 'Armor Rig - Exp Resist', 6, '6', 1, 2, 1, 773),
(2792, 986, 269, 5, 'Armor Rig - Kin Resist', 6, '6', 1, 2, 1, 773),
(2792, 987, 270, 5, 'Armor Rig - Therm Resist', 6, '6', 1, 2, 1, 773);


