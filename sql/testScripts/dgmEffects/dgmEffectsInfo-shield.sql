
-- shield effects data (incomplete)

INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenalty`, `effectState`, `targetType`, `targetGroup`)
VALUES
-- shield cap
(21, 72, 263, 1, 'Extenders - HP', 8, '6', 0, 2, 1, 38),
(57, 146, 263, 5, 'PDU - HP', 25, '6', 0, 2, 1, 766),
(57, 146, 263, 0, 'Flux Coil - HP', 40, '6', 0, 2, 1, 770),
-- shield recharge
(50, 134, 479, 5, 'PDU - Recharge Rate', 25, '6', 1, 2, 1, 766),
(50, 134, 479, 5, 'Flux Coil - Recharge Rate', 25, '6', 1, 2, 1, 770),
(50, 134, 479, 5, 'Power Relay - Recharge Rate', 25, '6', 1, 2, 1, 57),
(50, 134, 479, 5, 'Recharger - Recharge Rate', 25, '6', 1, 2, 1, 39),
-- passive shield resists
(2052, 984, 271, 50, 'Amplifier - Passive EM Resist', 51, '6', 1, 2, 1, 295),
(2052, 985, 272, 50, 'Amplifier - Passive Exp Resist', 51, '6', 1, 2, 1, 295),
(2052, 986, 273, 50, 'Amplifier - Passive Kin Resist', 51, '6', 1, 2, 1, 295),
(2052, 987, 274, 50, 'Amplifier - Passive Therm Resist', 51, '6', 1, 2, 1, 295),
-- active shield resists
(2117, 994, 271, 50, 'Hardener - Passive EM Resist', 51, '6', 1, 2, 1, 77),
(2117, 995, 272, 50, 'Hardener - Passive Exp Resist', 51, '6', 1, 2, 1, 77),
(2117, 996, 273, 50, 'Hardener - Passive Kin Resist', 51, '6', 1, 2, 1, 77),
(2117, 997, 274, 50, 'Hardener - Passive Therm Resist', 51, '6', 1, 2, 1, 77),
(2118, 984, 271, 50, 'Hardener - Active EM Resist', 51, '6', 1, 12, 1, 77),
(2118, 985, 272, 50, 'Hardener - Active Exp Resist', 51, '6', 1, 12, 1, 77),
(2118, 986, 273, 50, 'Hardener - Active Kin Resist', 51, '6', 1, 12, 1, 77),
(2118, 987, 274, 50, 'Hardener - Active Therm Resist', 51, '6', 1, 12, 1, 77),
-- actives overload bonus
(3035, 1208, 271, 54, 'Hardener - Overload EM Resist', 55, '6', 0, 8, 1, 77),
(3035, 1208, 272, 54, 'Hardener - Overload Exp Resist', 55, '6', 0, 8, 1, 77),
(3035, 1208, 273, 54, 'Hardener - Overload Kin Resist', 55, '6', 0, 8, 1, 77),
(3035, 1208, 274, 54, 'Hardener - Overload Therm Resist', 55, '6', 0, 8, 1, 77);

-- effects below affect other modules or objects and are NOT implemented yet...
(1720, 548, 68, 0, 'Boost Bonus', 0, '40;1156', 1, 14, 1, 338),
(1720, 548, 68, 0, 'Boost Bonus', 0, '40', 1, 14, 1, 767),
(2832, 312, 73, 0, 'Booster Duration Bonus', 40, '40', 0, 15, 1, 774),

-- unsorted (and untested and unchecked)
(3002, 1206, 73, 0, 'Transporter Duration', 40, '0', 0, 8, 1, 41);
