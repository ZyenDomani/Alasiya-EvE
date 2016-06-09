
-- t3 shit (groups 954-958)  these need more work and coresponding code for 'affectingID' (the module group)

INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenaltyApplied`, `effectAppliedInState`,`affectingID`, `affectingType`, `affectedType`)
VALUES
-- (0, 796, 4, 1, 'Mass', 8, '6', 0, 15, 32, 3, 3),     -- affectingID '32' means categoryID 32 "subsystem"
(3771, 1159, 265, 1, 'Armor HP Bonus', 8, '6', 0, 15, 32, 3, 3),   -- targetGroupIDs '6' means categoryID 6 "ship"
(3773, 1368, 102, 1, 'Turret Slot Modifier', 8, '6', 0, 15, 32, 3, 3),
(3773, 1368, 102, 1, 'Turret Slot Modifier', 8, '6', 0, 15, 32, 3, 3),
(3773, 1369, 101, 1, 'Launcher Slot Modifier', 8, '6', 0, 15, 32, 3, 3),
(3773, 1369, 101, 1, 'Launcher Slot Modifier', 8, '6', 0, 15, 32, 3, 3),
(3774, 1374, 14, 1, 'Hi Slot Modifier', 8, '6', 0, 15, 32, 3, 3),
(3774, 1375, 13, 1, 'Mid Slot Modifier', 8, '6', 0, 15, 32, 3, 3),
(3774, 1376, 12, 1, 'Low Slot Modifier', 8, '6', 0, 15, 32, 3, 3),
(3782, 11, 11, 1, 'PG Output', 8, '6', 0, 15, 32, 3, 3),
(3783, 48, 48, 1, 'CPU Output', 8, '6', 0, 15, 32, 3, 3),
(3784, 37, 37, 1, 'Max Velocity', 8, '6', 0, 15, 32, 3, 3),
(3797, 1271, 1271, 1, 'Drone Bandwidth', 8, '6', 0, 15, 32, 3, 3),
(3799, 283, 283, 1, 'Drone Capacity', 8, '6', 0, 15, 32, 3, 3),
(3806, 208, 208, 1, 'Radar Sensor Strength', 8, '6', 0, 32, 32, 3, 3),
(3806, 209, 209, 1, 'Ladar Sensor Strength', 8, '6', 0, 32, 32, 3, 3),
(3806, 210, 210, 1, 'Mag Sensor Strength', 8, '6', 0, 15, 32, 3, 3),
(3806, 211, 211, 1, 'Grav Sensor Strength', 8, '6', 0, 15, 32, 3, 3),
(3807, 76, 76, 1, 'Max Targeting Range', 8, '6', 0, 15, 32, 3, 3),
(3809, 564, 564, 1, 'Scan Resolution', 8, '6', 0, 15, 32, 3, 3),
(3811, 482, 482, 1, 'Capacitor Capacity', 8, '6', 0, 15, 32, 3, 3),
(3831, 263, 263, 1, 'Shield Capacity', 8, '6', 0, 15, 32, 3, 3),
(3853, 55, 55, 1, 'Capacitor Recharge', 8, '6', 0, 15, 32, 3, 3),
(3856, 479, 479, 1, 'Shield Recharge', 8, '6', 0, 15, 32, 3, 3),
(4240, 1418, 267, 0, 'Armor EM Resist', 40, '6', 0, 15, 32, 3, 3),
(4240, 1419, 270, 0, 'Armor Thermal Resist', 40, '6', 0, 15, 32, 3, 3),
(4240, 1420, 269, 0, 'Armor Kinetic Resist', 40, '6', 0, 15, 32, 3, 3),
(4240, 1421, 268, 0, 'Armor Explosive Resist', 40, '6', 0, 15, 32, 3, 3),
(4247, 1422, 272, 0, 'Shield Explosive Resist', 40, '6', 0, 15, 32, 3, 3),
(4247, 1423, 271, 0, 'Shield EM Resist', 40, '6', 0, 15, 32, 3, 3),
(4247, 1424, 273, 0, 'Shield Kinetic Resist', 40, '6', 0, 15, 32, 3, 3),
(4247, 1425, 274, 0, 'Shield Thermal Resist', 40, '6', 0, 15, 32, 3, 3),
(4281, 70, 70, 4, 'Inertia', 24, '6', 0, 15, 32, 3, 3),
(4409, 552, 552, 4, 'Signature Radius', 24, '6', 0, 15, 32, 3, 3);
