--  NOTE:  this is the currently working module effects (129 total)
--          it includes all modules that affect the ship
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
-- common for all modules
(16, 30, 15, 1, 'PG -> PG_Used', 8, '6', 0, 2, 0, 0, 3),
(16, 50, 49, 1, 'CPU -> CPU_Used', 8, '6', 0, 2, 0, 0, 3),
(2663, 1153, 1152, 1, 'Cal -> Cal-Used', 8, '6', 0, 3, 0, 0, 3),
-- hp
(60, 150, 9, 5, 'Reinforced Bulkhead - HP Bonus', 25, '6', 0, 3, 78, 2, 3),
(60, 150, 9, 5, 'NF Structure - HP Bonus', 25, '6', 0, 3, 763, 2, 3),
(3047, 150, 9, 5, 'Exp Cargohold - HP Penalty', 25, '6', 0, 3, 765, 2, 3),
-- cpu output
(536, 202, 48, 5, 'Co-Processor - CPU Output Bonus', 25, '6', 0, 3, 285, 2, 3),
-- pg
(56, 145, 11, 5, 'Power Diagnostic - PG', 25, '6', 0, 3, 766, 2, 3),
(56, 145, 11, 5, 'Reactor Control - PG', 25, '6', 0, 3, 769, 2, 3),
(627, 549, 11, 1, 'Auxiliary Power Core - PG', 8, '6', 0, 3, 339, 2, 3),
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
(51, 144, 55, 5, 'Power Relay - Cap Recharge Rate', 25, '6', 0, 14, 57, 2, 3),
-- inertia mods
(657, 169, 70, 56, 'NF Structure - Inertia Bonus', 57, '6', 1, 14, 763, 2, 3),
(657, 169, 70, 56, 'Reinforced Bulkhead - Inertia Penalty', 57, '6', 1, 14, 78, 2, 3),
(657, 169, 70, 56, 'IStab - Inertia Modifier', 57, '6', 1, 14, 762, 2, 3),
-- sig radius mods
(1254, 554, 552, 54, 'MWD Sig Radius Penalty', 55, '6', 1, 12, 46, 2, 3),
(2029, 983, 552, 1, 'Shield Extenders - Sig Radius Penality', 8, '6', 0, 14, 38, 2, 3),
(2644, 554, 552, 54, 'IStab - Sig Radius Penalty', 55, '6', 1, 14, 762, 2, 3),
-- mass mods
(1254, 796, 4, 1, 'AB/MWD Mass addition', 8, '6', 0, 12, 46, 2, 3),
(1959, 796, 4, 1, 'Armor Plates - Mass Addition', 8, '6', 0, 14, 329, 2, 3),
-- cargo cap
(59, 149, 38, 5, 'Exp Cargohold - Cargo Cap Bonus', 25, '6', 0, 14, 765, 2, 3),
(59, 149, 38, 5, 'OD Injector - Cargo Cap Penalty', 25, '6', 0, 14, 764, 2, 3),
-- velocity
(710, 20, 37, 54, 'AB/MWD Velocity Bonus', 55, '6', 0, 12, 46, 2, 3),
(854, 306, 37, 5, 'Cloaking Device - Velocity Penalty', 25, '6', 0, 4, 330, 2, 3),
(2865, 1076, 37, 54, 'NF Structure - Velocity Bonus', 55, '6', 1, 14, 763, 2, 3),
(2859, 306, 37, 5, 'Reinforced Bulkhead - Velocity Penalty', 25, '6', 1, 14, 78, 2, 3),
(2865, 1076, 37, 54, 'OD Injector - Velocity Bonus', 55, '6', 1, 14, 764, 2, 3),
(3046, 306, 37, 5, 'Exp Cargohold - Velocity Penalty', 25, '6', 1, 14, 765, 2, 3),
-- warp core str
(670, 105, 21, 1, 'Warp Stab - Strength Bonus', 8, '6', 0, 14, 315, 2, 3),
(3725, 105, 21, 1, 'Warp Core Strength', 8, '6', 0, 14, 52, 2, 3),
-- target range
(2646, 309, 76, 56, 'Warp Stab - Targeting Range Penalty', 57, '6', 1, 14, 315, 2, 3),
-- scan res
(854, 565, 564, 5, 'Cloaking Device - Scan Resolution', 25, '6', 0, 15, 330, 2, 3),
(2645, 565, 564, 5, 'Warp Stab - Scan Resolution Penalty', 25, '6', 1, 14, 315, 2, 3),

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
(2302, 977, 110, 30, 'Hull Therm Resistance Bonus', 31, '6', 0, 12, 60, 2, 3);


-- shield effects data

INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenaltyApplied`, `effectAppliedInState`, `affectingID`, `affectingType`, `affectedType`)
VALUES
-- shield cap
(21, 72, 263, 1, 'Extenders - HP', 8, '6', 0, 3, 38, 2, 3),
(57, 146, 263, 5, 'PDU - HP', 25, '6', 0, 3, 766, 2, 3),
(57, 146, 263, 0, 'Flux Coil - HP', 40, '6', 0, 3, 770, 2, 3),
-- shield recharge
(50, 134, 479, 5, 'PDU - Recharge Rate', 25, '6', 0, 3, 766, 2, 3),
(50, 134, 479, 5, 'Flux Coil - Recharge Rate', 25, '6', 0, 3, 770, 2, 3),
(50, 134, 479, 5, 'Power Relay - Recharge Rate', 25, '6', 0, 3, 57, 2, 3),
(50, 134, 479, 5, 'Recharger - Recharge Rate', 25, '6', 0, 3, 39, 2, 3),
-- passive shield resists
(2052, 984, 271, 50, 'Amplifier - Passive EM Resist', 51, '6', 1, 3, 295, 2, 3),
(2052, 985, 272, 50, 'Amplifier - Passive Exp Resist', 51, '6', 1, 3, 295, 2, 3),
(2052, 986, 273, 50, 'Amplifier - Passive Kin Resist', 51, '6', 1, 3, 295, 2, 3),
(2052, 987, 274, 50, 'Amplifier - Passive Therm Resist', 51, '6', 1, 3, 295, 2, 3),
-- active shield resists
(2117, 994, 271, 50, 'Hardener - Passive EM Resist', 51, '6', 1, 2, 77, 2, 3),
(2117, 995, 272, 50, 'Hardener - Passive Exp Resist', 51, '6', 1, 2, 77, 2, 3),
(2117, 996, 273, 50, 'Hardener - Passive Kin Resist', 51, '6', 1, 2, 77, 2, 3),
(2117, 997, 274, 50, 'Hardener - Passive Therm Resist', 51, '6', 1, 2, 77, 2, 3),
(2118, 984, 271, 50, 'Hardener - Active EM Resist', 51, '6', 1, 12, 77, 2, 3),
(2118, 985, 272, 50, 'Hardener - Active Exp Resist', 51, '6', 1, 12, 77, 2, 3),
(2118, 986, 273, 50, 'Hardener - Active Kin Resist', 51, '6', 1, 12, 77, 2, 3),
(2118, 987, 274, 50, 'Hardener - Active Therm Resist', 51, '6', 1, 12, 77, 2, 3),
-- actives overload bonus
(3035, 1208, 271, 54, 'Hardener - EM', 55, '6', 0, 8, 77, 2, 6),
(3035, 1208, 272, 54, 'Hardener - Exp', 55, '6', 0, 8, 77, 2, 6),
(3035, 1208, 273, 54, 'Hardener - Kin', 55, '6', 0, 8, 77, 2, 6),
(3035, 1208, 274, 54, 'Hardener - Therm', 55, '6', 0, 8, 77, 2, 6);

-- armor effects data
INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenaltyApplied`, `effectAppliedInState`, `affectingID`, `affectingType`, `affectedType`)
VALUES
-- armor hp
(63, 148, 265, 5, 'Coating - HP', 25, '6', 0, 3, 98, 2, 3),
(63, 148, 265, 5, 'Plating - HP', 25, '6', 0, 3, 326, 2, 3),
(2837, 1159, 265, 1, 'Plates - HP', 8, '6', 0, 3, 329, 2, 3),
-- passive armor resists
(2041, 984, 267, 50, 'Plating - Passive EM Resist', 51, '6', 1, 3, 98, 2, 3),
(2041, 985, 268, 50, 'Plating - Passive Exp Resist', 51, '6', 1, 3, 98, 2, 3),
(2041, 986, 269, 50, 'Plating - Passive Kin Resist', 51, '6', 1, 3, 98, 2, 3),
(2041, 987, 270, 50, 'Plating - Passive Therm Resist', 51, '6', 1, 3, 98, 2, 3),
(2041, 984, 267, 50, 'Coating - Passive EM Resist', 51, '6', 1, 3, 326, 2, 3),
(2041, 985, 268, 50, 'Coating - Passive Exp Resist', 51, '6', 1, 3, 326, 2, 3),
(2041, 986, 269, 50, 'Coating - Passive Kin Resist', 51, '6', 1, 3, 326, 2, 3),
(2041, 987, 270, 50, 'Coating - Passive Therm Resist', 51, '6', 1, 3, 326, 2, 3),
-- active armor resists
(2084, 994, 267, 50, 'Hardener - Passive EM Resist', 51, '6', 1, 2, 328, 2, 3),
(2084, 995, 268, 50, 'Hardener - Passive Exp Resist', 51, '6', 1, 2, 328, 2, 3),
(2084, 996, 269, 50, 'Hardener - Passive Kin Resist', 51, '6', 1, 2, 328, 2, 3),
(2084, 997, 270, 50, 'Hardener - Passive Therm Resist', 51, '6', 1, 2, 328, 2, 3),
(2098, 984, 267, 50, 'Hardener - Active EM Resist', 51, '6', 1, 12, 328, 2, 3),
(2098, 985, 268, 50, 'Hardener - Active Exp Resist', 51, '6', 1, 12, 328, 2, 3),
(2098, 986, 269, 50, 'Hardener - Active Kin Resist', 51, '6', 1, 12, 328, 2, 3),
(2098, 987, 270, 50, 'Hardener - Active Therm Resist', 51, '6', 1, 12, 328, 2, 3),
-- active overload bonus
(3029, 1208, 113, 56, 'Hardener - EM', 57, '0', 0, 8, 328, 2, 6),
(3031, 1208, 111, 56, 'Hardener - Exp', 57, '0', 0, 8, 328, 2, 6),
(3032, 1208, 109, 56, 'Hardener - Kin', 57, '0', 0, 8, 328, 2, 6),
(3030, 1208, 110, 56, 'Hardener - Therm', 57, '0', 0, 8, 328, 2, 6),
(4039, 1208, 113, 56, 'Hardener - EM', 57, '0', 0, 8, 328, 2, 6),
(4039, 1208, 111, 56, 'Hardener - Exp', 57, '0', 0, 8, 328, 2, 6),
(4039, 1208, 109, 56, 'Hardener - Kin', 57, '0', 0, 8, 328, 2, 6),
(4039, 1208, 110, 56, 'Hardener - Therm', 57, '0', 0, 8, 328, 2, 6);

-- rig effects data
INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenaltyApplied`, `effectAppliedInState`, `affectingID`, `affectingType`, `affectedType`)
VALUES
-- drawbacks
(2712, 1138, 265, 54, 'Rig Drawback - Armor HP', 55, '6', 0, 3, 782, 2, 3),
(2713, 1138, 48, 54, 'Rig Drawback - CPU Output', 55, '6', 0, 3, 778, 2, 3),
(2716, 1138, 552, 54, 'Rig Drawback - Sig Radius', 55, '6', 1, 3, 774, 2, 3),
(2717, 1138, 37, 54, 'Rig Drawback - Max Velocity', 55, '6', 2, 3, 773, 2, 3),
(2718, 1138, 263, 54, 'Rig Drawback - Shield Capacity', 55, '6', 0, 3, 786, 2, 3),
(3528, 1138, 30, 54, 'Rig Drawback - Cap Recharge Rate', 55, '6', 0, 3, 904, 2, 2),
-- cpu output
(397, 424, 48, 54, 'Engineering Rig - CPU Output Bonus', 55, '6', 0, 3, 781, 2, 3),
-- cap recharge
(485, 314, 55, 54, 'Engineering Rig - Cap Recharge Rate', 55, '6', 0, 3, 781, 2, 3),
-- pg
(490, 313, 11, 54, 'Engineering Rig - PG Output Bonus', 55, '6', 0, 3, 781, 2, 3),
-- cap capacity
(2432, 1079, 482, 54, 'Engineering Rig - Cap Capacty', 55, '6', 0, 3, 781, 2, 3),
-- velocity
(394, 315, 37, 54, 'Astronautic Rig - Velocity Bonus', 55, '6', 1, 3, 782, 2, 3),
(3727, 1076, 37, 54, 'Astronautic Rig - Velocity Bonus', 55, '6', 1, 3, 782, 2, 3),
-- inertia
(395, 151, 70, 54, 'Astronautic Rig - Inertia Bonus', 55, '6', 1, 3, 782, 2, 3),
(3726, 169, 70, 54, 'Astronautic Rig - Inertia Bonus', 55, '6', 1, 3, 782, 2, 3),
-- warp core
(494, 319, 153, 54, 'Astronautic Rig - Warp Core Bonus', 55, '6', 0, 3, 782, 2, 3),
-- warp speed
(856, 624, 1281, 54, 'Astronautic Rig - Warp Speed Bonus', 55, '6', 0, 3, 782, 2, 3),
-- cargo
(836, 614, 38, 54, 'Astronautic Rig - Cargohold Bonus', 55, '6', 0, 3, 782, 2, 3),
-- shield rigs
(486, 338, 479, 56, 'Shield Rig - Shield Recharge Bonus', 57, '6', 0, 3, 774, 2, 3),
(2795, 984, 271, 50, 'Shield Rig - EM Resist', 51, '6', 1, 3, 774, 2, 3),
(2795, 985, 272, 50, 'Shield Rig - Exp Resist', 51, '6', 1, 3, 774, 2, 3),
(2795, 986, 273, 50, 'Shield Rig - Kin Resist', 51, '6', 1, 3, 774, 2, 3),
(2795, 987, 274, 50, 'Shield Rig - Therm Resist', 51, '6', 1, 3, 774, 2, 3),
-- armor rigs
(271, 335, 265, 0, 'Armor Rig - Hp Bonus', 40, '6', 0, 3, 773, 2, 3),
(2792, 984, 267, 50, 'Armor Rig - EM Resist', 51, '6', 1, 15, 773, 2, 3),
(2792, 985, 268, 50, 'Armor Rig - Exp Resist', 51, '6', 1, 15, 773, 2, 3),
(2792, 986, 269, 50, 'Armor Rig - Kin Resist', 51, '6', 1, 15, 773, 2, 3),
(2792, 987, 270, 50, 'Armor Rig - Therm Resist', 51, '6', 1, 15, 773, 2, 3);


