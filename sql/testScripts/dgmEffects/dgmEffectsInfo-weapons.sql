
-- weapon effects data
-- not used yet

INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenaltyApplied`, `effectAppliedInState`, `affectingID`, `affectingType`, `affectedType`)
VALUES

-- missile ship mods (t2)
(3191, 554, 552, 1, 'Light - Signature Radius Penalty', 8, '6', 0, 15, 653, 2, 3),
(3191, 554, 552, 1, 'Heavy - Signature Radius Penalty', 8, '6', 0, 15, 655, 2, 3),
(3191, 554, 552, 1, 'Cruise - Signature Radius Penalty', 8, '6', 0, 15, 656, 2, 3),
(3191, 554, 552, 1, 'Assault - Signature Radius Penalty', 8, '6', 0, 15, 654, 2, 3),
(3191, 554, 552, 1, 'Rocket - Signature Radius Penalty', 8, '6', 0, 15, 648, 2, 3),
(3191, 554, 552, 1, 'Torpedo - Signature Radius Penalty', 8, '6', 0, 15, 657, 2, 3),
(3191, 306, 37, 0, 'Light - Max Velocity Penalty', 40, '6', 0, 15, 653, 2, 3),
(3191, 306, 37, 0, 'Heavy - Max Velocity Penalty', 40, '6', 0, 15, 655, 2, 3),
(3191, 306, 37, 0, 'Assault - Max Velocity Penalty', 40, '6', 0, 15, 654, 2, 3),
(3191, 306, 37, 0, 'Cruise - Max Velocity Penalty', 40, '6', 0, 15, 656, 2, 3),
(3191, 306, 37, 0, 'Rocket - Max Velocity Penalty', 40, '6', 0, 15, 648, 2, 3),
(3191, 306, 37, 0, 'Torpedo - Max Velocity Penalty', 40, '6', 0, 15, 657, 2, 3),

-- effects below affect other modules or objects and are NOT implemented yet...

-- missile damage  these affect charge attribs
(763, 213, 114, 0, 'Damage Bonus - EM', 40, '654;656;655;653;648;657;772;386;385;384;387;89;476;88;394;395;396', 1, 14, 367, 2, 2),
(763, 213, 116, 0, 'Damage Bonus - Exp', 40, '654;656;655;653;648;657;772;386;385;384;387;89;476;88;394;395;396', 1, 14, 367, 2, 2),
(763, 213, 117, 0, 'Damage Bonus - Kin', 40, '654;656;655;653;648;657;772;386;385;384;387;89;476;88;394;395;396', 1, 14, 367, 2, 2),
(763, 213, 118, 0, 'Damage Bonus - Therm', 40, '654;656;655;653;648;657;772;386;385;384;387;89;476;88;394;395;396', 1, 14, 367, 2, 2),
(784, 557, 281, 0, 'Flight Time Bonus', 40, '654;656;655;653;648;657;772;476;386;88;396;395;394;385;384;387;89', 1, 15, 779, 2, 2),

-- missile ROF  these affect launchers
(3001, 1205, 51, 0, 'Assault Launcher ROF Bonus', 40, '0', 0, 8, 511, 2, 6),
(3001, 1205, 51, 0, 'Citadel Launcher ROF Bonus', 40, '0', 0, 8, 524, 2, 6),
(3001, 1205, 51, 0, 'Cruise Launcher ROF Bonus', 40, '0', 0, 8, 506, 2, 6),
(3001, 1205, 51, 0, 'Heavy Launcher ROF Bonus', 40, '0', 0, 8, 510, 2, 6),
(3001, 1205, 51, 0, 'Heavy Assault Launcher ROF Bonus', 40, '0', 0, 8, 771, 2, 6),
(3001, 1205, 51, 0, 'Rocket Launcher ROF Bonus', 40, '0', 0, 8, 507, 2, 6),
(3001, 1205, 51, 0, 'Siege Launcher ROF Bonus', 40, '0', 0, 8, 508, 2, 6),
(3001, 1205, 51, 0, 'Standard Launcher ROF Bonus', 40, '0', 0, 8, 509, 2, 6),
-- turrent ROF
(3001, 1205, 51, 0, 'Hybrid Weapon ROF Bonus', 40, '0', 0, 8, 74, 2, 6),
(3025, 1205, 51, 0, 'Projectile Weapon ROF Bonus', 40, '0', 0, 8, 55, 2, 6),
(3025, 1205, 51, 0, 'Energy Weapon ROF Bonus', 40, '0', 0, 8, 53, 2, 6),
(889, 204, 51, 0, 'ROF Bonus', 40, '511;524;506;510;771;507;508;501;509', 1, 14, 367, 2, 2),

-- these are not sorted yet  (87)

(596, 120, 54, 5, 'Freq Crystals - Optimal Range', 25, '0', 0, 15, 86, 2, 6),
(596, 120, 54, 5, 'Hybrid Ammo - Optimal Range', 25, '0', 0, 15, 85, 2, 6),
(596, 120, 54, 5, 'Projectile Ammo - Optimal Range', 25, '0', 0, 15, 83, 2, 6),

(3001, 1210, 64, 0, 'Hybrid Weapon Damage Bonus', 40, '0', 0, 8, 74, 2, 6),
(3025, 1210, 64, 0, 'Projectile Weapon Damage Bonus', 40, '0', 0, 8, 55, 2, 6),
(3025, 1210, 64, 0, 'Energy Weapon Damage Bonus', 40, '0', 0, 8, 53, 2, 6),
(804, 317, 6, 0, 'Freq Crystals - Activation Cost', 0, '0', 0, 15, 86, 2, 6),
(804, 317, 6, 0, 'Hybrid Ammo - Activation Cost', 0, '0', 0, 15, 85, 2, 6),
(804, 317, 6, 0, 'Projectile Ammo - Activation Cost', 0, '0', 0, 15, 83, 2, 6),
(600, 244, 160, 5, 'Projectile Ammo - Tracking Bonus', 25, '0', 0, 15, 83, 2, 6),
(91, 64, 64, 5, 'Damage Bonus', 25, '53', 1, 14, 205, 2, 2),
(92, 64, 64, 5, 'Damage Bonus', 25, '55', 1, 14, 59, 2, 2),
(93, 64, 64, 5, 'Damage Bonus', 25, '74', 1, 14, 302, 2, 2),

(89, 204, 51, 0, 'ROF', 40, '55', 1, 14, 59, 2, 2),
(95, 204, 51, 0, 'ROF', 40, '53', 1, 14, 205, 2, 2),
(96, 204, 51, 0, 'ROF', 40, '74', 1, 14, 302, 2, 2),

-- T2 weapon shit...not implemented
(596, 120, 54, 5, 'T2 Beam - Optimal Range', 25, '0', 0, 15, 374, 2, 6),
(804, 317, 6, 0, 'T2 Beam - Activation Cost', 0, '0', 0, 15, 374, 2, 6),
(834, 144, 55, 0, 'T2 Beam - Capacitor Recharge Rate', 0, '6', 0, 15, 374, 2, 3),
(600, 244, 160, 5, 'T2 Beam - Tracking Speed', 25, '0', 0, 15, 374, 2, 6),
(598, 306, 37, 0, 'T2 Beam - Max Velocity', 0, '6', 0, 15, 374, 2, 3),
(599, 517, 158, 5, 'T2 Beam - Falloff Modifier', 25, '0', 0, 15, 374, 2, 6),
(596, 120, 54, 5, 'T2 Pulse - Optimal Range', 25, '0', 0, 15, 375, 2, 6),
(804, 317, 6, 0, 'T2 Pulse - Activation Cost', 0, '0', 0, 15, 375, 2, 6),
(834, 144, 55, 0, 'T2 Pulse - Capacitor Recharge Rate', 0, '6', 0, 15, 375, 2, 3),
(600, 244, 160, 5, 'T2 Pulse - Tracking Speed', 25, '0', 0, 15, 375, 2, 6),
(598, 306, 37, 0, 'T2 Pulse - Max Velocity', 0, '6', 0, 15, 375, 2, 3),
(599, 517, 158, 5, 'T2 Pulse - Falloff Modifier', 25, '0', 0, 15, 375, 2, 6),
(596, 120, 54, 5, 'T2 Blaster - Optimal Range', 25, '0', 0, 15, 377, 2, 6),
(804, 317, 6, 0, 'T2 Blaster - Activation Cost', 0, '0', 0, 15, 377, 2, 6),
(834, 144, 55, 0, 'T2 Blaster - Capacitor Recharge Rate', 0, '6', 0, 15, 377, 2, 3),
(600, 244, 160, 5, 'T2 Blaster - Tracking Speed', 25, '0', 0, 15, 377, 2, 6),
(598, 306, 37, 0, 'T2 Blaster - Max Velocity', 0, '6', 0, 15, 377, 2, 3),
(599, 517, 158, 5, 'T2 Blaster - Falloff Modifier', 25, '0', 0, 15, 377, 2, 6),
(596, 120, 54, 5, 'T2 Railgun - Optimal Range', 25, '0', 0, 15, 373, 2, 6),
(804, 317, 6, 0, 'T2 Railgun - Activation Cost', 0, '0', 0, 15, 373, 2, 6),
(834, 144, 55, 0, 'T2 Railgun - Capacitor Recharge Rate', 0, '6', 0, 15, 373, 2, 3),
(600, 244, 160, 5, 'T2 Railgun - Tracking Speed', 25, '0', 0, 15, 373, 2, 6),
(598, 306, 37, 0, 'T2 Railgun - Max Velocity', 0, '6', 0, 15, 373, 2, 3),
(599, 517, 158, 5, 'T2 Railgun - Falloff Modifier', 25, '0', 0, 15, 373, 2, 6),
(596, 120, 54, 5, 'T2 Artillery - Optimal Range', 25, '0', 0, 15, 376, 2, 6),
(804, 317, 6, 0, 'T2 Artillery - Activation Cost', 0, '0', 0, 15, 376, 2, 6),
(834, 144, 55, 0, 'T2 Artillery - Capacitor Recharge Rate', 0, '6', 0, 15, 376, 2, 3),
(600, 244, 160, 5, 'T2 Artillery - Tracking Speed', 25, '0', 0, 15, 376, 2, 6),
(598, 306, 37, 0, 'T2 Artillery - Max Velocity', 0, '6', 0, 15, 376, 2, 3),
(599, 517, 158, 5, 'T2 Artillery - Falloff Modifier', 25, '0', 0, 15, 376, 2, 6),
(596, 120, 54, 5, 'T2 Autocannon - Optimal Range', 25, '0', 0, 15, 372, 2, 6),
(804, 317, 6, 0, 'T2 Autocannon - Activation Cost', 0, '0', 0, 15, 372, 2, 6),
(834, 144, 55, 0, 'T2 Autocannon - Capacitor Recharge Rate', 0, '6', 0, 15, 372, 2, 3),
(600, 244, 160, 5, 'T2 Autocannon - Tracking Speed', 25, '0', 0, 15, 372, 2, 6),
(598, 306, 37, 0, 'T2 Autocannon - Max Velocity', 0, '6', 0, 15, 372, 2, 3),
(599, 517, 158, 5, 'T2 Autocannon - Falloff Modifier', 25, '0', 0, 15, 372, 2, 6);
