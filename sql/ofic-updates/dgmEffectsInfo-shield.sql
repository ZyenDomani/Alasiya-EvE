
-- shield effects data

INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenaltyApplied`, `effectAppliedInState`, `affectingID`, `affectingType`, `affectedType`)
VALUES
-- the following effects are working and sorted and state correct (40)
-- shield cap
(21, 72, 263, 1, 'Extenders - HP', 8, '6', 0, 3, 38, 2, 3),
(57, 146, 263, 5, 'PDU - HP', 25, '6', 0, 3, 766, 2, 3),
(57, 146, 263, 0, 'Flux Coil - HP', 40, '6', 0, 3, 770, 2, 3),
-- shield recharge
(50, 134, 479, 5, 'PDU - Recharge Rate', 25, '6', 0, 3, 766, 2, 3),
(50, 134, 479, 5, 'Flux Coil - Recharge Rate', 25, '6', 0, 3, 770, 2, 3),
(50, 134, 479, 5, 'Power Relay - Recharge Rate', 25, '6', 0, 3, 57, 2, 3),
(50, 134, 479, 5, 'Recharger - Recharge Rate', 25, '6', 0, 3, 39, 2, 3),

-- shield resists
-- passives
(2052, 984, 271, 50, 'Amplifier - Passive EM Resist', 51, '6', 1, 3, 295, 2, 3),
(2052, 985, 272, 50, 'Amplifier - Passive Exp Resist', 51, '6', 1, 3, 295, 2, 3),
(2052, 986, 273, 50, 'Amplifier - Passive Kin Resist', 51, '6', 1, 3, 295, 2, 3),
(2052, 987, 274, 50, 'Amplifier - Passive Therm Resist', 51, '6', 1, 3, 295, 2, 3),
-- actives
(2117, 994, 271, 50, 'Hardener - Passive EM Resist', 51, '6', 1, 2, 77, 2, 3),
(2117, 995, 272, 50, 'Hardener - Passive Exp Resist', 51, '6', 1, 2, 77, 2, 3),
(2117, 996, 273, 50, 'Hardener - Passive Kin Resist', 51, '6', 1, 2, 77, 2, 3),
(2117, 997, 274, 50, 'Hardener - Passive Therm Resist', 51, '6', 1, 2, 77, 2, 3),
(2118, 984, 271, 50, 'Hardener - Active EM Resist', 51, '6', 1, 12, 77, 2, 3),
(2118, 985, 272, 50, 'Hardener - Active Exp Resist', 51, '6', 1, 12, 77, 2, 3),
(2118, 986, 273, 50, 'Hardener - Active Kin Resist', 51, '6', 1, 12, 77, 2, 3),
(2118, 987, 274, 50, 'Hardener - Active Therm Resist', 51, '6', 1, 12, 77, 2, 3),
-- (3201, 68, 0, 56, 'Booster - Amount', 57, '6', 0, 12, 40, 2, 6),
-- actives overload bonus
(3035, 1208, 271, 54, 'Hardener - EM', 55, '6', 0, 8, 77, 2, 6),
(3035, 1208, 272, 54, 'Hardener - Exp', 55, '6', 0, 8, 77, 2, 6),
(3035, 1208, 273, 54, 'Hardener - Kin', 55, '6', 0, 8, 77, 2, 6),
(3035, 1208, 274, 54, 'Hardener - Therm', 55, '6', 0, 8, 77, 2, 6),
-- these overloads affect the module attribs
(3201, 1206, 73, 56, 'Booster - Duration', 57, '6', 0, 8, 40, 2, 6),
(3201, 1231, 68, 56, 'Booster - Amount', 57, '6', 0, 8, 40, 2, 6),

-- rigs (and implants)
(486, 338, 479, 56, 'Recharge Rate Bonus', 57, '6', 0, 3, 774, 2, 3),
(2795, 984, 271, 50, 'Modify Shield Resonance - EM', 51, '6', 1, 3, 774, 2, 3),
(2795, 985, 272, 50, 'Modify Shield Resonance - Exp', 51, '6', 1, 3, 774, 2, 3),
(2795, 986, 273, 50, 'Modify Shield Resonance - Kin', 51, '6', 1, 3, 774, 2, 3),
(2795, 987, 274, 50, 'Modify Shield Resonance - Therm', 51, '6', 1, 3, 774, 2, 3),


-- unsorted (and untested and unchecked)
(3002, 1206, 73, 0, 'Transporter Duration', 40, '0', 0, 8, 41, 2, 6),

-- affecting modules
(1720, 548, 68, 0, 'Boost Bonus', 0, '40;1156', 1, 14, 338, 2, 2),
(1720, 548, 68, 0, 'Boost Bonus', 0, '40', 1, 14, 767, 2, 2),
(4877, 548, 68, 0, 'Boost Bonus', 0, '40', 1, 4, 515, 1, 2),
(4893, 548, 68, 0, 'Boost Bonus', 0, '40', 1, 4, 515, 1, 2),
(4877, 897, 73, 0, 'Booster Duration', 0, '40', 0, 4, 515, 1, 2),
(4893, 897, 73, 0, 'Booster Duration', 0, '40', 0, 4, 515, 1, 2),
(4893, 1189, 68, 0, 'Transporter Amount Bonus', 0, '41', 0, 4, 515, 1, 2),
(4893, 1188, 73, 0, 'Transporter Duration Bonus', 0, '41', 0, 4, 515, 1, 2),
(2832, 312, 73, 0, 'Booster Duration Bonus', 40, '40', 0, 15, 774, 2, 2);
