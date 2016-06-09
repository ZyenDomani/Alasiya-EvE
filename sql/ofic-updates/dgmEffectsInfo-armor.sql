
-- armor effects data

INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenaltyApplied`, `effectAppliedInState`, `affectingID`, `affectingType`, `affectedType`)
VALUES
-- the following effects are working and sorted
(1959, 796, 4, 1, 'Armor Plates - Mass Addition', 8, '6', 0, 14, 329, 2, 3),
(2837, 1159, 265, 1, 'Armor Plates - Armor HP', 8, '6', 0, 14, 329, 2, 3),

(63, 148, 265, 54, 'Armor Amplifiers - Armor HP Bonus', 55, '6', 0, 14, 326, 2, 3),
(63, 148, 265, 54, 'Armor Plating - Armor HP Bonus', 55, '6', 0, 14, 98, 2, 3),

(2084, 994, 267, 50, 'Armor Hardener - Passive EM Resist', 51, '6', 1, 2, 328, 2, 3),
(2084, 995, 268, 50, 'Armor Hardener - Passive Exp Resist', 51, '6', 1, 2, 328, 2, 3),
(2084, 996, 269, 50, 'Armor Hardener - Passive Kin Resist', 51, '6', 1, 2, 328, 2, 3),
(2084, 997, 270, 50, 'Armor Hardener - Passive Therm Resist', 51, '6', 1, 2, 328, 2, 3),
(2098, 984, 267, 50, 'Armor Hardener - Active EM Resist', 51, '6', 1, 12, 328, 2, 3),
(2098, 985, 268, 50, 'Armor Hardener - Active Exp Resist', 51, '6', 1, 12, 328, 2, 3),
(2098, 986, 269, 50, 'Armor Hardener - Active Kin Resist', 51, '6', 1, 12, 328, 2, 3),
(2098, 987, 270, 50, 'Armor Hardener - Active Therm Resist', 51, '6', 1, 12, 328, 2, 3),

-- 3029  overload em hardner bonus

(4039, 1208, 113, 1, 'Armor Hardener - EM', 8, '0', 0, 8, 328, 2, 6),
(4039, 1208, 111, 1, 'Armor Hardener - Exp', 8, '0', 0, 8, 328, 2, 6),
(4039, 1208, 109, 1, 'Armor Hardener - Kin', 8, '0', 0, 8, 328, 2, 6),
(4039, 1208, 110, 1, 'Armor Hardener - Therm', 8, '0', 0, 8, 328, 2, 6),

(2041, 984, 267, 50, 'Armor Amplifiers - Passive EM Resist', 51, '6', 1, 14, 326, 2, 3),
(2041, 985, 268, 50, 'Armor Amplifiers - Passive Exp Resist', 51, '6', 1, 14, 326, 2, 3),
(2041, 986, 269, 50, 'Armor Amplifiers - Passive Kin Resist', 51, '6', 1, 14, 326, 2, 3),
(2041, 987, 270, 50, 'Armor Amplifiers - Passive Therm Resist', 51, '6', 1, 14, 326, 2, 3),

(2041, 984, 267, 50, 'Armor Plating - Passive EM Resist', 51, '6', 1, 14, 98, 2, 3),
(2041, 985, 268, 50, 'Armor Plating - Passive Exp Resist', 51, '6', 1, 14, 98, 2, 3),
(2041, 986, 269, 50, 'Armor Plating - Passive Kin Resist', 51, '6', 1, 14, 98, 2, 3),
(2041, 987, 270, 50, 'Armor Plating - Passive Therm Resist', 51, '6', 1, 14, 98, 2, 3),

(2792, 984, 267, 50, 'Modify Armor Resonance - EM', 51, '6', 1, 15, 773, 2, 3),
(2792, 985, 268, 50, 'Modify Armor Resonance - Explosive', 51, '6', 1, 15, 773, 2, 3),
(2792, 986, 269, 50, 'Modify Armor Resonance - Kinetic', 51, '6', 1, 15, 773, 2, 3),
(2792, 987, 270, 50, 'Modify Armor Resonance - Thermal', 51, '6', 1, 15, 773, 2, 3),

(3002, 1206, 73, 56, 'Overload Rep Duration Bonus', 57, '0', 0, 8, 325, 2, 6),
(3200, 84, 84, 5, 'Armor Rep Amount', 25, '6', 0, 10, 62, 2, 6),
(3200, 1230, 84, 56, 'Armor Rep Overload Amount', 57, '0', 0, 8, 62, 2, 6),
(3200, 1206, 73, 56, 'Overload Rep Duration', 57, '0', 0, 8, 62, 2, 6),

(4877, 895, 84, 0, 'Armor Repair Amount', 0, '62', 0, 4, 515, 1, 2),
(4877, 896, 73, 0, 'Armor Repair Duration', 0, '62', 0, 4, 515, 1, 2),

(4893, 895, 84, 0, 'Armor Repair Amount', 0, '62', 0, 4, 515, 1, 2),
(4893, 896, 73, 0, 'Armor Repair Duration', 0, '62', 0, 4, 515, 1, 2),
(4893, 1186, 84, 0, 'Remote Armor Repair Bonus', 0, '325', 0, 4, 515, 1, 2),
(4893, 1187, 73, 0, 'Remote Armor Repair Duration Bonus', 0, '325', 0, 4, 515, 1, 2);