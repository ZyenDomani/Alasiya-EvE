
-- armor effects data (incomplete)

-- armor effects data
INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenalty`, `effectState`, `targetType`, `targetGroup`)
VALUES
-- armor hp
(63, 148, 265, 5, 'Coating - HP', 25, '6', 0, 2, 1, 98),
(63, 148, 265, 5, 'Plating - HP', 25, '6', 0, 2, 1, 326),
(2837, 1159, 265, 1, 'Plates - HP', 8, '6', 0, 2, 1, 329),
-- passive armor resists
(2041, 984, 267, 50, 'Plating - Passive EM Resist', 51, '6', 1, 2, 1, 98),
(2041, 985, 268, 50, 'Plating - Passive Exp Resist', 51, '6', 1, 2, 1, 98),
(2041, 986, 269, 50, 'Plating - Passive Kin Resist', 51, '6', 1, 2, 1, 98),
(2041, 987, 270, 50, 'Plating - Passive Therm Resist', 51, '6', 1, 2, 1, 98),
(2041, 984, 267, 50, 'Coating - Passive EM Resist', 51, '6', 1, 2, 1, 326),
(2041, 985, 268, 50, 'Coating - Passive Exp Resist', 51, '6', 1, 2, 1, 326),
(2041, 986, 269, 50, 'Coating - Passive Kin Resist', 51, '6', 1, 2, 1, 326),
(2041, 987, 270, 50, 'Coating - Passive Therm Resist', 51, '6', 1, 2, 1, 326),
-- active armor resists
(2084, 994, 267, 50, 'Hardener - Passive EM Resist', 51, '6', 1, 2, 1, 328),
(2084, 995, 268, 50, 'Hardener - Passive Exp Resist', 51, '6', 1, 2, 1, 328),
(2084, 996, 269, 50, 'Hardener - Passive Kin Resist', 51, '6', 1, 2, 1, 328),
(2084, 997, 270, 50, 'Hardener - Passive Therm Resist', 51, '6', 1, 2, 1, 328),
(2098, 984, 267, 50, 'Hardener - Active EM Resist', 51, '6', 1, 12, 1, 328),
(2098, 985, 268, 50, 'Hardener - Active Exp Resist', 51, '6', 1, 12, 1, 328),
(2098, 986, 269, 50, 'Hardener - Active Kin Resist', 51, '6', 1, 12, 1, 328),
(2098, 987, 270, 50, 'Hardener - Active Therm Resist', 51, '6', 1, 12, 1, 328),
-- active overload bonus
(3029, 1208, 113, 56, 'Hardener - Overload EM Resist', 57, '6', 0, 8, 1, 328),
(3031, 1208, 111, 56, 'Hardener - Overload Exp Resist', 57, '6', 0, 8, 1, 328),
(3032, 1208, 109, 56, 'Hardener - Overload Kin Resist', 57, '6', 0, 8, 1, 328),
(3030, 1208, 110, 56, 'Hardener - Overload Therm Resist', 57, '6', 0, 8, 1, 328),
(4039, 1208, 113, 56, 'Hardener - Overload EM Resist', 57, '6', 0, 8, 1, 328),
(4039, 1208, 111, 56, 'Hardener - Overload Exp Resist', 57, '6', 0, 8, 1, 328),
(4039, 1208, 109, 56, 'Hardener - Overload Kin Resist', 57, '6', 0, 8, 1, 328),
(4039, 1208, 110, 56, 'Hardener - Overload Therm Resist', 57, '6', 0, 8, 1, 328);

-- unsorted (and untested and unchecked)
(3002, 1206, 73, 56, 'Overload Rep Duration Bonus', 57, '6', 0, 8, 1, 325),
(3200, 84, 84, 5, 'Rep Amount', 25, '6', 0, 10, 1, 62),
(3200, 1230, 84, 56, 'Rep Overload Amount', 57, '6', 0, 8, 1, 62),
(3200, 1206, 73, 56, 'Overload Rep Duration', 57, '6', 0, 8, 1, 62);

