
-- rig effects data
INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenalty`, `effectState`, `targetType`, `targetGroup`)
VALUES
-- drawbacks

-- the following effects are working and sorted and state correct (except where noted) (40)

-- some of these drawbacks arent complete or working...
(2706, 1138, 30, 5, 'Rig Drawback - Power Need Laser', 6, '53', 0, 2, 1, 775),
(2707, 1138, 30, 5, 'Rig Drawback - Power Need Hybrid', 6, '74', 0, 2, 1, 776),
(2708, 1138, 30, 5, 'Rig Drawback - Power Need Projectile', 6, '55', 0, 2, 1, 777),
(2709, 1138, 30, 5, 'Rig Drawback - Power Need Launcher', 6, '511;524;506;510;771;507;508;509', 0, 2, 1, 779),
-- (2710, 1138, 30, 5, 'Rig Drawback - Power Need Gunnery', 6, '55', 0, 2, 1, 777),
(2712, 1138, 265, 5, 'Rig Drawback - Armor HP', 6, '', 0, 2, 1, 782),
(2713, 1138, 48, 5, 'Rig Drawback - CPU Output', 6, '', 0, 2, 1, 778),
(2714, 1138, 50, 5, 'Rig Drawback - CPU Need Launchers', 6, '511;524;506;510;771;507;508;509', 0, 2, 1, 779),
-- (2715, 1138, 50, 5, 'Rig Drawback - CPU Need Gunnery', 6, '', 0, 2, 1, 779),
(2716, 1138, 552, 5, 'Rig Drawback - Sig Radius', 6, '', 1, 2, 1, 774),
(2717, 1138, 37, 5, 'Rig Drawback - Max Velocity', 6, '', 1, 2, 1, 773),
(2718, 1138, 263, 5, 'Rig Drawback - Shield Capacity', 6, '', 0, 2, 1, 786),
--  2719,2720,2721 - no rigs with these drawbacks
-- (2719, 1138, 30, 5, 'Rig Drawback - Shield Uniformity', 6, '', 0, 2, 1, 777),
-- (2720, 1138, 30, 5, 'Rig Drawback - Capacitor Capacity', 6, '', 0, 2, 1, 777),
-- (2721, 1138, 30, 5, 'Rig Drawback - Power Output', 6, '', 0, 2, 1, 777),
-- (2722, 1138, 30, 5, 'Rig Drawback - ROF Gunnery', 6, '55', 0, 2, 1, 777),
(2723, 1138, 30, 5, 'Rig Drawback - ROF Launcher', 6, '511;524;506;510;771;507;508;509', 0, 2, 1, 779),
-- (2724, 1138, 30, 5, 'Rig Drawback - Bonus Effect', 6, '55', 0, 2, 1, 777),
-- (2725, 1138, 30, 5, 'Rig Drawback - Skill Effect', 6, '55', 0, 2, 1, 777),
(3528, 1138, 30, 5, 'Rig Drawback - Cap Recharge Rate', 6, '', 0, 2, 1, 904),

-- cpu output
(397, 424, 48, 5, 'Engineering Rig - CPU Output Bonus', 6, '', 0, 2, 1, 781),
-- cap recharge
(485, 314, 55, 5, 'Engineering Rig - Cap Recharge Rate', 6, '', 0, 2, 1, 781),
-- pg
(490, 313, 11, 5, 'Engineering Rig - PG Output Bonus', 6, '', 0, 2, 1, 781),
-- cap capacity
(2432, 1079, 482, 5, 'Engineering Rig - Cap Capacty', 6, '', 0, 2, 1, 781),
-- velocity
(394, 315, 37, 5, 'Astronautic Rig - Velocity Bonus', 6, '', 1, 2, 1, 782),
(3727, 1076, 37, 5, 'Astronautic Rig - Velocity Bonus', 6, '', 1, 2, 1, 782),
-- inertia
(395, 151, 70, 5, 'Astronautic Rig - Inertia Bonus', 6, '', 1, 2, 1, 782),
(3726, 169, 70, 5, 'Astronautic Rig - Inertia Bonus', 6, '', 1, 2, 1, 782),
-- warp cap need
(494, 319, 153, 5, 'Astronautic Rig - Warp Cap Need Bonus', 6, '', 0, 2, 1, 782),
-- warp speed
(856, 624, 1281, 5, 'Astronautic Rig - Warp Speed Bonus', 6, '', 0, 2, 1, 782),
-- cargo
(836, 614, 38, 5, 'Astronautic Rig - Cargohold Bonus', 6, '', 0, 2, 1, 782),
-- shield rigs
(486, 338, 479, 5, 'Shield Rig - Shield Recharge Bonus', 6, '', 0, 2, 1, 774),
(2795, 984, 271, 5, 'Shield Rig - EM Resist', 6, '', 1, 2, 1, 774),
(2795, 985, 272, 5, 'Shield Rig - Exp Resist', 6, '', 1, 2, 1, 774),
(2795, 986, 273, 5, 'Shield Rig - Kin Resist', 6, '', 1, 2, 1, 774),
(2795, 987, 274, 5, 'Shield Rig - Therm Resist', 6, '', 1, 2, 1, 774),
-- armor rigs
(271, 335, 265, 5, 'Armor Rig - Hp Bonus', 6, '', 0, 2, 1, 773),
(2792, 984, 267, 5, 'Armor Rig - EM Resist', 6, '', 1, 2, 1, 773),
(2792, 985, 268, 5, 'Armor Rig - Exp Resist', 6, '', 1, 2, 1, 773),
(2792, 986, 269, 5, 'Armor Rig - Kin Resist', 6, '', 1, 2, 1, 773),
(2792, 987, 270, 5, 'Armor Rig - Therm Resist', 6, '', 1, 2, 1, 773);

-- Energy Weapon rigs
-- Hybrid Weapon
-- Projectile Weapon
-- Drones
-- Launcher
-- Electronics
-- Energy Grid
-- Astronautic
-- Electronics Superiority
-- Mining

-- these are not sorted yet
(504, 459, 459, 5, 'Drone Control Range Augmentor', 6, '', 0,  2, 1, 778),
(854, 565, 564, 5, 'Targeting System Subcontroller', 6, '', 1,  2, 1, 786),
(2852, 309, 76, 5, 'Ionic Field Projector', 6, '', 1,  2, 1, 786),

--  this data isnt right...
-- (3591, 828, 309, 5, 'EWar - Max Range', 6, '208', 1, 2, 1, 786),

-- effects below affect other modules or objects and are NOT implemented yet...
(1882, 434, 77, 5, 'Mining Amount Bonus', 6, '54;483', 0,  2, 1, 904),
(3500, 1138, 30, 5, 'Mining Range Bonus', 6, '54;483', 0,  2, 1, 904),

(3586, 828, 566, 5, 'EWar - Scan Strength Bonus', 6, '208', 1,  2, 1, 786),

(1500, 851, 6, 0, 'Capacitor Need Bonus', 40, '3416', 0,  2, 1, 774),
(273, 323, 30, 0, 'Upgrade Power Need Bonus', 40, '3425', 0,  2, 1, 774),,
(2850, 66, 73, 5, 'AB/MWD Duration Bonus', 6, '46', 0,  2, 1, 782),
(212, 310, 50, 0, 'CPU Need Bonus', 0, '3432', 0,  2, 1, 780),
(4162, 846, 1371, 0, 'Sensor Strength Modifier', 0, '479', 0,  2, 1, 780),

(272, 312, 73, 0, 'Repair Systems - Duration Bonus', 0, '1049;1050;1051', 0,  2, 1, 773),
(227, 317, 6, 0, 'Acceration Control - Cap Need Bonus', 0, '46', 0,  2, 1, 782),
(623, 434, 77, 0, 'Mining Drone Operation - Mining Amount Bonus', 0, '18', 0,  2, 1, 778),

(2794, 902, 902, 5, 'Salvaging - Access Difficulty Bonus', 6, '25861;30836', 0, 2, 1, 773),

(1281, 806, 84, 5, 'Remote Repair Pump - Amount Bonus', 6, '1049;1050;1051', 1, 2, 1, 773),
(1030, 317, 6, 5, 'Remote Repair Augmentor - Cap Need Bonus', 6, '325', 0, 2, 1, 773),

(504, 459, 459, 5, 'EW Drone Range Augmentor', 6, '639', 0, 2, 1, 778),
(2015, 327, 263, 5, 'Drone Durability Enhancer - Shield', 6, '18', 0, 2, 1, 778),
(2016, 327, 265, 5, 'Drone Durability Enhancer - Armor', 6, '18', 0, 2, 1, 778),
(2017, 327, 9, 5, 'Drone Durability Enhancer - Hull', 6, '18', 0, 2, 1, 778),
(2019, 39, 68, 5, 'Drone Repair Augmentor - Shield', 6, '18', 0, 2, 1, 778),
(2020, 39, 84, 5, 'Drone Repair Augmentor - Armor', 6, '18', 0, 2, 1, 778),
(2014, 294, 54, 5, 'Drone Scope Chip', 6, '18', 0, 2, 1, 778),
(2013, 591, 37, 5, 'Drone Speed Augmentor', 6, '18', 1, 2, 1, 778),
(2867, 292, 64, 56, 'Sentry Damage Augmentor', 57, '911', 1, 2, 1, 778),
(4579, 1164, 20, 5, 'Stasis Drone Augmentor', 6, '18', 0, 2, 1, 778),
(2853, 619, 560, 5, 'Targeting Systems Stabiizer', 6, '330', 0, 2, 1, 786),
(3560, 828, 351, 5, 'Tracking Diagnostic Subroutines', 6, '291', 1, 2, 1, 786),
(3561, 828, 767, 5, 'Tracking Diagnostic Subroutines', 6, '291', 1, 2, 1, 786),
(1318, 828, 238, 5, 'Particle Dispersion Augmentor - Grav', 6, '201', 1, 2, 1, 786),
(1318, 828, 239, 5, 'Particle Dispersion Augmentor - Ladar', 6, '201', 1, 2, 1, 786),
(1318, 828, 240, 5, 'Particle Dispersion Augmentor - Mag', 6, '201', 1, 2, 1, 786),
(1318, 828, 241, 5, 'Particle Dispersion Augmentor - Radar', 6, '201', 1, 2, 1, 786),
(1445, 294, 54, 5, 'Particle Dispersion Projector – RSD Max Range', 6, '201;208;291;379', 1, 2, 1, 786),
(1446, 294, 54, 5, 'Particle Dispersion Projector – TP Max Range', 6, '201;208;291;379', 1, 2, 1, 786),
(1448, 294, 54, 5, 'Particle Dispersion Projector – TD Max Range', 6, '201;208;291;379', 1, 2, 1, 786),
(1452, 294, 54, 5, 'Particle Dispersion Projector – EW Max Range', 6, '201;208;291;379', 1, 2, 1, 786),
(2491, 294, 54, 5, 'Particle Dispersion Projector – ECM Burst Range', 6, '201;208;291;379', 1, 2, 1, 786),
(744, 308, 73, 5, 'Signal Focusing Kit', 6, '47;48;49', 0, 2, 1, 786),
(236, 317, 6, 5, 'Egress Port Maximizer', 6, '67;71', 0, 2, 1, 781),
(396, 310, 50, 5, 'Powergrid Subroutine Maximizer', 6, '43;61;766;767;768;769', 0, 2, 1, 781),
(2690, 310, 50, 5, 'Algid Energy Administrations Unit', 6, '53', 0, 2, 1, 775),
(2693, 349, 158, 5, 'Energy Ambit Extension', 6, '53', 1, 2, 1, 775),
(2801, 204, 51, 5, 'Energy Burst Aerator', 6, '53', 1, 2, 1, 775),
(2803, 64, 64, 56, 'Energy Collision Accelerator', 57, '53', 1, 2, 1, 775),
(2688, 317, 6, 5, 'Energy Discharge Elutriation', 6, '53', 1, 2, 1, 775),
(2696, 351, 54, 5, 'Energy Locus Coordinator', 6, '53', 1, 2, 1, 775),
(2699, 244, 160, 5, 'Energy Metastasis Adjuster', 6, '53', 1, 2, 1, 775),

(2691, 310, 50, 5, 'Algid Hybrid Administrations Unit', 6, '74', 0, 2, 1, 776),
(2694, 349, 158, 5, 'Hybrid Ambit Extension', 6, '74', 1, 2, 1, 776),
(2804, 204, 51, 5, 'Hybrid Burst Aerator', 6, '74', 1, 2, 1, 776),
(2802, 64, 64, 56, 'Hybrid Collision Accelerator', 57, '74', 1, 2, 1, 776),
(2689, 317, 6, 5, 'Hybrid Discharge Elutriation', 6, '74', 1, 2, 1, 776),
(2697, 351, 54, 5, 'Hybrid Locus Coordinator', 6, '74', 1, 2, 1, 776),
(2700, 244, 160, 5, 'Hybrid Metastasis Adjuster', 25, '74', 1, 2, 1, 776),

(2799, 204, 51, 5, 'Bay Loading Accelerator', 6, '511;524;506;510;771;507;508;509', 1, 2, 1, 779),
(1764, 20, 37, 5, 'Hydraulic Bay Thrusters', 6, '654;656;655;653;648;657;772;476;386;88;396;395;394;385;384;387;89', 2, 2, 1, 779),
(2851, 213, 114, 5, 'Warhead Calefaction Catalyst', 6, '654;656;655;653;648;657;772;476;386;88;396;395;394;385;384;387;89', 1, 2, 1, 779),
(2851, 213, 116, 5, 'Warhead Calefaction Catalyst', 6, '654;656;655;653;648;657;772;476;386;88;396;395;394;385;384;387;89', 1, 2, 1, 779),
(2851, 213, 117, 5, 'Warhead Calefaction Catalyst', 6, '654;656;655;653;648;657;772;476;386;88;396;395;394;385;384;387;89', 1, 2, 1, 779),
(2851, 213, 118, 5, 'Warhead Calefaction Catalyst', 6, '654;656;655;653;648;657;772;476;386;88;396;395;394;385;384;387;89', 1, 2, 1, 779),
(1590, 847, 653, 5, 'Warhead Flare Catalyst', 6, '654;656;655;653;648;657;772;476;386;88;396;395;394;385;384;387;89', 0, 2, 1, 779),
(1472, 848, 654, 5, 'Warhead Rigor Catalyst', 6, '656;655;653;386;396;395;394;385;384', 0, 2, 1, 779),

(2695, 349, 158, 5, 'Projectile Falloff Bonus', 6, '55', 1, 2, 1, 777),
(2698, 351, 54, 5, 'Projectile Range Bonus', 6, '55', 1, 2, 1, 777),
(2798, 64, 64, 56, 'Projectile Damager Bonus', 57, '55', 1, 2, 1, 777),
(2797, 204, 51, 5, 'Projectile ROF Bonus', 6, '55', 1, 2, 1, 777),
(2701, 244, 160, 5, 'Projectile Tracking Speed Bonus', 6, '55', 1, 2, 1, 777);
