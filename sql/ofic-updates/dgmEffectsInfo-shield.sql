
-- shield effects data

INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenaltyApplied`, `effectAppliedInState`, `affectingID`, `affectingType`, `affectedType`)
VALUES
-- the following effects are working and sorted
-- shield cap
(57, 146, 263, 5, 'PDU - Shield HP', 25, '6', 0, 14, 766, 2, 3),
-- shield recharge
(50, 134, 479, 5, 'PDU - Shield Recharge Rate', 25, '6', 0, 14, 766, 2, 3),

(2117, 994, 271, 50, 'Shield Hardeners - Passive EM Resist', 51, '6', 1, 2, 77, 2, 3),
(2117, 995, 272, 50, 'Shield Hardeners - Passive Exp Resist', 51, '6', 1, 2, 77, 2, 3),
(2117, 996, 273, 50, 'Shield Hardeners - Passive Kin Resist', 51, '6', 1, 2, 77, 2, 3),
(2117, 997, 274, 50, 'Shield Hardeners - Passive Therm Resist', 51, '6', 1, 2, 77, 2, 3),
(2118, 984, 271, 50, 'Shield Hardeners - Active EM Resist', 51, '6', 1, 12, 77, 2, 3),
(2118, 985, 272, 50, 'Shield Hardeners - Active Exp Resist', 51, '6', 1, 12, 77, 2, 3),
(2118, 986, 273, 50, 'Shield Hardeners - Active Kin Resist', 51, '6', 1, 12, 77, 2, 3),
(2118, 987, 274, 50, 'Shield Hardeners - Active Therm Resist', 51, '6', 1, 12, 77, 2, 3),

(2052, 984, 271, 50, 'Shield Amplifiers - Passive EM Resist', 51, '6', 1, 14, 295, 2, 3),
(2052, 985, 272, 50, 'Shield Amplifiers - Passive Exp Resist', 51, '6', 1, 14, 295, 2, 3),
(2052, 986, 273, 50, 'Shield Amplifiers - Passive Kin Resist', 51, '6', 1, 14, 295, 2, 3),
(2052, 987, 274, 50, 'Shield Amplifiers - Passive Therm Resist', 51, '6', 1, 14, 295, 2, 3),

(2795, 984, 271, 50, 'Modify Shield Resonance - EM', 51, '6', 1, 15, 774, 2, 3),
(2795, 985, 272, 50, 'Modify Shield Resonance - Explosive', 51, '6', 1, 15, 774, 2, 3),
(2795, 986, 273, 50, 'Modify Shield Resonance - Kinetic', 51, '6', 1, 15, 774, 2, 3),
(2795, 987, 274, 50, 'Modify Shield Resonance - Thermal', 51, '6', 1, 15, 774, 2, 3),


-- unsorted
(1720, 548, 68, 0, 'Shield Boost Bonus', 0, '40;1156', 1, 14, 338, 2, 2),
(21, 72, 263, 1, 'Shield Extenders - Shield HP', 8, '6', 0, 14, 38, 2, 3),
(2029, 983, 552, 1, 'Shield Extenders - Signature Radius', 8, '6', 0, 14, 38, 2, 3),
(57, 146, 263, 0, 'Shield Flux Coil - Shield HP', 40, '6', 0, 14, 770, 2, 3),
(50, 134, 479, 5, 'Shield Flux Coil - Shield Recharge Rate', 25, '6', 0, 14, 770, 2, 3),
(50, 134, 479, 5, 'Shield Power Relay - Shield Recharge Rate', 25, '6', 0, 14, 57, 2, 3),
(51, 144, 55, 5, 'Shield Power Relay - Capacitor Recharge Rate', 25, '6', 0, 14, 57, 2, 3),
(50, 134, 479, 5, 'Shield Recharger - Shield Recharge Rate', 25, '6', 0, 14, 39, 2, 3),
(3201, 1231, 68, 5, 'Shield Booster Boost Amount', 25, '0', 0, 8, 40, 2, 6),
(3201, 1206, 73, 0, 'Shield Booster Duration', 40, '0', 0, 8, 40, 2, 6),
(3035, 1208, 271, 1, 'Shield Hardener - EM', 8, '0', 0, 8, 77, 2, 6),
(3035, 1208, 272, 1, 'Shield Hardener - Exp', 8, '0', 0, 8, 77, 2, 6),
(3035, 1208, 273, 1, 'Shield Hardener - Kin', 8, '0', 0, 8, 77, 2, 6),
(3035, 1208, 274, 1, 'Shield Hardener - Therm', 8, '0', 0, 8, 77, 2, 6),
(3002, 1206, 73, 0, 'Shield Transporter Duration', 40, '0', 0, 8, 41, 2, 6),
(4877, 548, 68, 0, 'Shield Boost Bonus', 0, '40', 1, 4, 515, 1, 2),

(4877, 897, 73, 0, 'Shield Booster Duration', 0, '40', 0, 4, 515, 1, 2),
(4893, 548, 68, 0, 'Shield Boost Bonus', 0, '40', 1, 4, 515, 1, 2),

(4893, 897, 73, 0, 'Shield Booster Duration', 0, '40', 0, 4, 515, 1, 2),
(4893, 1188, 73, 0, 'Shield Transporter Duration Bonus', 0, '41', 0, 4, 515, 1, 2),
(4893, 1189, 68, 0, 'Shield Transporter Amount Bonus', 0, '41', 0, 4, 515, 1, 2),

(4893, 1802, 6, 0, 'Shield Transporter Cap Use', 0, '41', 0, 4, 515, 1, 2),

(446, 337, 263, 0, 'Shield Capacity Bonus', 40, '6', 0, 15, 774, 2, 3),
(2832, 312, 73, 0, 'Booster Duration Bonus', 40, '40', 0, 15, 774, 2, 2);