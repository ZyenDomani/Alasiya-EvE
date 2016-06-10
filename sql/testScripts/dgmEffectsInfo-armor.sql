
-- armor effects data (incomplete)

INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenaltyApplied`, `effectAppliedInState`, `affectingID`, `affectingType`, `affectedType`)
VALUES
-- the following effects are working and sorted (41)
-- armor hp
(63, 148, 265, 5, 'Coating - HP', 25, '6', 0, 3, 98, 2, 3),
(63, 148, 265, 5, 'Plating - HP', 25, '6', 0, 3, 326, 2, 3),
(2837, 1159, 265, 1, 'Plates - HP', 8, '6', 0, 3, 329, 2, 3),

-- armor resists
-- passives
(2041, 984, 267, 50, 'Plating - Passive EM Resist', 51, '6', 1, 3, 98, 2, 3),
(2041, 985, 268, 50, 'Plating - Passive Exp Resist', 51, '6', 1, 3, 98, 2, 3),
(2041, 986, 269, 50, 'Plating - Passive Kin Resist', 51, '6', 1, 3, 98, 2, 3),
(2041, 987, 270, 50, 'Plating - Passive Therm Resist', 51, '6', 1, 3, 98, 2, 3),
(2041, 984, 267, 50, 'Coating - Passive EM Resist', 51, '6', 1, 3, 326, 2, 3),
(2041, 985, 268, 50, 'Coating - Passive Exp Resist', 51, '6', 1, 3, 326, 2, 3),
(2041, 986, 269, 50, 'Coating - Passive Kin Resist', 51, '6', 1, 3, 326, 2, 3),
(2041, 987, 270, 50, 'Coating - Passive Therm Resist', 51, '6', 1, 3, 326, 2, 3),

-- actives
(2084, 994, 267, 50, 'Hardener - Passive EM Resist', 51, '6', 1, 2, 328, 2, 3),
(2084, 995, 268, 50, 'Hardener - Passive Exp Resist', 51, '6', 1, 2, 328, 2, 3),
(2084, 996, 269, 50, 'Hardener - Passive Kin Resist', 51, '6', 1, 2, 328, 2, 3),
(2084, 997, 270, 50, 'Hardener - Passive Therm Resist', 51, '6', 1, 2, 328, 2, 3),
(2098, 984, 267, 50, 'Hardener - Active EM Resist', 51, '6', 1, 12, 328, 2, 3),
(2098, 985, 268, 50, 'Hardener - Active Exp Resist', 51, '6', 1, 12, 328, 2, 3),
(2098, 986, 269, 50, 'Hardener - Active Kin Resist', 51, '6', 1, 12, 328, 2, 3),
(2098, 987, 270, 50, 'Hardener - Active Therm Resist', 51, '6', 1, 12, 328, 2, 3),

-- actives overload bonus
(3029, 1208, 113, 56, 'Hardener - EM', 57, '0', 0, 8, 328, 2, 6),
(3031, 1208, 111, 56, 'Hardener - Exp', 57, '0', 0, 8, 328, 2, 6),
(3032, 1208, 109, 56, 'Hardener - Kin', 57, '0', 0, 8, 328, 2, 6),
(3030, 1208, 110, 56, 'Hardener - Therm', 57, '0', 0, 8, 328, 2, 6),
(4039, 1208, 113, 56, 'Hardener - EM', 57, '0', 0, 8, 328, 2, 6),
(4039, 1208, 111, 56, 'Hardener - Exp', 57, '0', 0, 8, 328, 2, 6),
(4039, 1208, 109, 56, 'Hardener - Kin', 57, '0', 0, 8, 328, 2, 6),
(4039, 1208, 110, 56, 'Hardener - Therm', 57, '0', 0, 8, 328, 2, 6),

-- rigs (and implants)
(271, 335, 265, 0, 'Armor Rig - Hp Bonus', 40, '6', 0, 3, 773, 2, 3),
(2792, 984, 267, 50, 'Armor Rig - EM Resist', 51, '6', 1, 15, 773, 2, 3),
(2792, 985, 268, 50, 'Armor Rig - Exp Resist', 51, '6', 1, 15, 773, 2, 3),
(2792, 986, 269, 50, 'Armor Rig - Kin Resist', 51, '6', 1, 15, 773, 2, 3),
(2792, 987, 270, 50, 'Armor Rig - Therm Resist', 51, '6', 1, 15, 773, 2, 3),

-- unsorted (and untested and unchecked)
(3002, 1206, 73, 56, 'Overload Rep Duration Bonus', 57, '0', 0, 8, 325, 2, 6),
(3200, 84, 84, 5, 'Rep Amount', 25, '6', 0, 10, 62, 2, 6),
(3200, 1230, 84, 56, 'Rep Overload Amount', 57, '0', 0, 8, 62, 2, 6),
(3200, 1206, 73, 56, 'Overload Rep Duration', 57, '0', 0, 8, 62, 2, 6);

