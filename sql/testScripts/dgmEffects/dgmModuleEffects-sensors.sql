
-- sensor and scanning effects data
-- not used yet

INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenalty`, `effectState`, `targetType`, `targetGroup`)
VALUES
-- ID, src, targ, calc, des, rcalc, tgrpID, stack, state, targetType, targetGroup

-- ship scan strength
(2231, 1030, 208, 5, 'ECCM - Radar Strength', 6, '6', 1, 2, 1, 202),
(2231, 1028, 209, 5, 'ECCM - Ladar Strength', 6, '6', 1, 2, 1, 202),
(2231, 1029, 210, 5, 'ECCM - Mag Strength', 6, '6', 1, 2, 1, 202),
(2231, 1027, 211, 5, 'ECCM - Grav Strength', 6, '6', 1, 2, 1, 202),
(2232, 1030, 208, 5, 'Backup Array - Radar Strength', 6, '6', 1, 2, 1, 203),
(2232, 1028, 209, 5, 'Backup Array - Ladar Strength', 6, '6', 1, 2, 1, 203),
(2232, 1029, 210, 5, 'Backup Array - Mag Strength', 6, '6', 1, 2, 1, 203),
(2232, 1027, 211, 5, 'Backup Array - Grav Strength', 6, '6', 1, 2, 1, 203),

(2246, 1030, 208, 5, 'ECCM Projector - Radar Strength', 6, '6', 1, 2, 4, 289),
(2246, 1028, 209, 5, 'ECCM Projector - Ladar Strength', 6, '6', 1, 2, 4, 289),
(2246, 1029, 210, 5, 'ECCM Projector - Mag Strength', 6, '6', 1, 2, 4, 289),
(2246, 1027, 211, 5, 'ECCM Projector - Grav Strength', 6, '6', 1, 2, 4, 289),

-- ship target range
(2670, 309, 76, 5, 'Sensor Booster - Max Targeting Range', 6, '6', 1, 2, 1, 212),
(3583, 309, 76, 5, 'Remote Sensor Booster - Max Targeting Range', 6, '6', 1, 2, 1, 290),
(3584, 309, 76, 5, 'Remote Sensor Dampner - Max Targeting Range', 6, '6', 1, 2, 4, 208),
(3659, 309, 76, 5, 'Signal Amplifier - Max Targeting Range', 6, '6', 1, 2, 1, 210),

-- overloaded
(3182, 1225, 238, 5, 'ECM Overload Bonus - Grav Strength', 6, '6', 1, 8, 2, 201),
(3182, 1225, 239, 5, 'ECM Overload Bonus - Ladar Strength', 6, '6', 1, 8, 2, 201),
(3182, 1225, 240, 5, 'ECM Overload Bonus - Mag Strength', 6, '6', 1, 8, 2, 201),
(3182, 1225, 241, 5, 'ECM Overload Bonus - Radar Strength', 6, '6', 1, 8, 2, 201),
(3189, 1226, 1027, 5, 'ECCM Overload Bonus - Grav Strength', 6, '6', 1, 8, 2, 202),
(3189, 1226, 1028, 5, 'ECCM Overload Bonus - Ladar Strength', 6, '6', 1, 8, 2, 202),
(3189, 1226, 1029, 5, 'ECCM Overload Bonus - Mag Strength', 6, '6', 1, 8, 2, 202),
(3189, 1226, 1030, 5, 'ECCM Overload Bonus - Radar Strength', 6, '6', 1, 8, 2, 202),


-- these are not sorted yet  (40)

(2007, 243, 54, 5, 'Omni Tracking Link - Max Range', 25, '18', 1, 15, 646, 2, 3),

(2007, 244, 160, 5, 'Omni Tracking Link - Tracking Speed', 25, '18', 1, 15, 646, 2, 3),


(3598, 1313, 309, 0, 'Sensor Booster - Max Target Range', 0, '6', 0, 15, 910, 2, 6),
(3598, 1313, 309, 0, 'Sensor Dampener - Max Target Range', 0, '6', 0, 15, 911, 2, 6),

(3597, 1314, 566, 0, 'Sensor Booster - Scan Resolution', 0, '6', 0, 15, 910, 2, 6),
(3597, 1314, 566, 0, 'Sensor Dampener - Scan Resolution', 0, '6', 0, 15, 911, 2, 6),

(3599, 1316, 767, 0, 'Tracking Disruptor - Tracking Speed', 0, '6', 0, 15, 909, 2, 6),
(3600, 1315, 351, 0, 'Tracking Disruptor - Optimal Range', 0, '6', 0, 15, 909, 2, 6),
(3686, 1332, 349, 0, 'Tracking Disruptor- Falloff', 0, '6', 0, 15, 909, 2, 6),

(3600, 1315, 351, 0, 'Tracking Computer - Optimal Range', 0, '6', 0, 15, 907, 2, 6),
(3599, 1316, 767, 0, 'Tracking Computer - Tracking Speed', 0, '6', 0, 15, 907, 2, 6),
(3686, 1332, 349, 0, 'Tracking Computer - Falloff', 0, '6', 0, 15, 907, 2, 6),


(1358, 238, 0, 0, 'Grav Strength', 0, '6', 0, 14, 201, 2, 3),
(1358, 239, 0, 0, 'Ladar Strength', 0, '6', 0, 14, 201, 2, 3),
(1358, 240, 0, 0, 'Mag Strength', 0, '6', 0, 14, 201, 2, 3),
(1358, 241, 0, 0, 'Radar Strength', 0, '6', 0, 14, 201, 2, 3),
(4809, 1130, 238, 1, 'Grav Strength', 8, '201', 1, 15, 514, 2, 2),
(4810, 1130, 239, 1, 'Ladar Strength', 8, '201', 1, 15, 514, 2, 2),
(4811, 1130, 240, 1, 'Mag Strength', 8, '201', 1, 15, 514, 2, 2),
(4812, 1130, 241, 1, 'Radar Strength', 8, '201', 1, 15, 514, 2, 2),
(4358, 1536, 54, 0, 'ECM Range', 0, '201', 1, 15, 514, 2, 2),
(53, 238, 0, 0, 'Grav Strength', 0, '6', 0, 14, 80, 2, 3),
(53, 239, 0, 0, 'Ladar Strength', 0, '6', 0, 14, 80, 2, 3),
(53, 240, 0, 0, 'Mag Strength', 0, '6', 0, 14, 80, 2, 3),
(53, 241, 0, 0, 'Radar Strength', 0, '6', 0, 14, 80, 2, 3),
(3584, 566, 564, 0, 'Scan Resolution', 0, '6', 1, 16, 208, 2, 3),
(586, 20, 37, 0, 'Max Velocity Penalty', 0, '6', 1, 16, 65, 2, 3),
(1549, 554, 552, 1, 'Signature Radius Penalty', 8, '6', 1, 16, 379, 2, 3),
(3687, 349, 158, 0, 'Falloff', 40, '55;53;74', 0, 16, 291, 2, 2),
(3690, 351, 54, 0, 'Optimal Range', 40, '55;53;74', 1, 16, 291, 2, 2),
(3690, 767, 160, 0, 'Tracking Speed', 40, '55;53;74', 1, 16, 291, 2, 2),
(3380, 554, 552, 1, 'Signature Radius Penalty', 8, '6', 1, 12, 899, 2, 3),
(3583, 566, 564, 0, 'Remote Sensor Booster - Scan Resolution', 0, '6', 1, 16, 290, 2, 3),
(2670, 566, 564, 0, 'Sensor Booster - Scan Resolution', 0, '6', 1, 4, 212, 2, 3),
(3660, 235, 192, 1, 'Signal Amplifier - Max Locked Targets', 8, '6', 0, 14, 210, 2, 3),
(3657, 566, 564, 0, 'Signal Amplifier - Scan Resolution', 0, '6', 1, 14, 210, 2, 3),
(2492, 317, 6, 0, 'Signal Disruption Amplifier', 0, '201;80', 0, 15, 780, 2, 2);