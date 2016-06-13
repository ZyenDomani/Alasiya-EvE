
-- rig effects data
INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenalty`, `effectState`, `targetType`, `targetGroup`)
VALUES
-- drawbacks

-- the following effects are working and sorted and state correct (except where noted) (40)

-- some of these drawbacks arent complete or working...
(2706, 1138, 30, 54, 'Rig Drawback - Power Need Laser', 55, '53', 0, 2, 1, 775),
(2707, 1138, 30, 54, 'Rig Drawback - Power Need Hybrid', 55, '74', 0, 2, 1, 776),
(2708, 1138, 30, 54, 'Rig Drawback - Power Need Projectile', 55, '55', 0, 2, 1, 777),
(2709, 1138, 30, 54, 'Rig Drawback - Power Need Launcher', 55, '511;524;506;510;771;507;508;509', 0, 2, 1, 779),
-- (2710, 1138, 30, 54, 'Rig Drawback - Power Need Gunnery', 55, '55', 0, 2, 1, 777),
(2712, 1138, 265, 54, 'Rig Drawback - Armor HP', 55, '6', 0, 2, 1, 782),
(2713, 1138, 48, 54, 'Rig Drawback - CPU Output', 55, '6', 0, 2, 1, 778),
(2714, 1138, 50, 54, 'Rig Drawback - CPU Need Launchers', 55, '511;524;506;510;771;507;508;509', 0, 2, 1, 779),
-- (2715, 1138, 50, 54, 'Rig Drawback - CPU Need Gunnery', 55, '', 0, 2, 1, 779),
(2716, 1138, 552, 54, 'Rig Drawback - Sig Radius', 55, '6', 1, 2, 1, 774),
(2717, 1138, 37, 54, 'Rig Drawback - Max Velocity', 55, '6', 1, 2, 1, 773),
(2718, 1138, 263, 54, 'Rig Drawback - Shield Capacity', 55, '6', 0, 2, 1, 786),
--  2719,2720,2721 - no rigs with these drawbacks
-- (2719, 1138, 30, 54, 'Rig Drawback - Shield Uniformity', 55, '6', 0, 2, 1, 777),
-- (2720, 1138, 30, 54, 'Rig Drawback - Capacitor Capacity', 55, '6', 0, 2, 1, 777),
-- (2721, 1138, 30, 54, 'Rig Drawback - Power Output', 55, '6', 0, 2, 1, 777),
-- (2722, 1138, 30, 54, 'Rig Drawback - ROF Gunnery', 55, '55', 0, 2, 1, 777),
(2723, 1138, 30, 54, 'Rig Drawback - ROF Launcher', 55, '511;524;506;510;771;507;508;509', 0, 2, 1, 779),
-- (2724, 1138, 30, 54, 'Rig Drawback - Bonus Effect', 55, '55', 0, 2, 1, 777),
-- (2725, 1138, 30, 54, 'Rig Drawback - Skill Effect', 55, '55', 0, 2, 1, 777),
(3528, 1138, 30, 54, 'Rig Drawback - Cap Recharge Rate', 55, '6', 0, 2, 1, 904),

-- cpu output
(397, 424, 48, 54, 'Engineering Rig - CPU Output Bonus', 55, '6', 0, 2, 1, 781),
-- cap recharge
(485, 314, 55, 54, 'Engineering Rig - Cap Recharge Rate', 55, '6', 0, 2, 1, 781),
-- pg
(490, 313, 11, 54, 'Engineering Rig - PG Output Bonus', 55, '6', 0, 2, 1, 781),
-- cap capacity
(2432, 1079, 482, 54, 'Engineering Rig - Cap Capacty', 55, '6', 0, 2, 1, 781),
-- velocity
(394, 315, 37, 54, 'Astronautic Rig - Velocity Bonus', 55, '6', 1, 2, 1, 782),
(3727, 1076, 37, 54, 'Astronautic Rig - Velocity Bonus', 55, '6', 1, 2, 1, 782),
-- inertia
(395, 151, 70, 54, 'Astronautic Rig - Inertia Bonus', 55, '6', 1, 2, 1, 782),
(3726, 169, 70, 54, 'Astronautic Rig - Inertia Bonus', 55, '6', 1, 2, 1, 782),
-- warp core
(494, 319, 153, 54, 'Astronautic Rig - Warp Core Bonus', 55, '6', 0, 2, 1, 782),
-- warp speed
(856, 624, 1281, 54, 'Astronautic Rig - Warp Speed Bonus', 55, '6', 0, 2, 1, 782),
-- cargo
(836, 614, 38, 54, 'Astronautic Rig - Cargohold Bonus', 55, '6', 0, 2, 1, 782),
-- shield rigs
(486, 338, 479, 56, 'Shield Rig - Shield Recharge Bonus', 57, '6', 0, 2, 1, 774),
(2795, 984, 271, 50, 'Shield Rig - EM Resist', 51, '6', 1, 2, 1, 774),
(2795, 985, 272, 50, 'Shield Rig - Exp Resist', 51, '6', 1, 2, 1, 774),
(2795, 986, 273, 50, 'Shield Rig - Kin Resist', 51, '6', 1, 2, 1, 774),
(2795, 987, 274, 50, 'Shield Rig - Therm Resist', 51, '6', 1, 2, 1, 774),
-- armor rigs
(271, 335, 265, 0, 'Armor Rig - Hp Bonus', 40, '6', 0, 2, 1, 773),
(2792, 984, 267, 50, 'Armor Rig - EM Resist', 51, '6', 1, 2, 1, 773),
(2792, 985, 268, 50, 'Armor Rig - Exp Resist', 51, '6', 1, 2, 1, 773),
(2792, 986, 269, 50, 'Armor Rig - Kin Resist', 51, '6', 1, 2, 1, 773),
(2792, 987, 270, 50, 'Armor Rig - Therm Resist', 51, '6', 1, 2, 1, 773);

-- these are not sorted yet
(504, 459, 459, 54, 'Drone Control Range Augmentor', 55, '6', 0,  2, 1, 778),
(854, 565, 564, 54, 'Targeting System Subcontroller', 55, '6', 1,  2, 1, 786),
(2852, 309, 76, 54, 'Ionic Field Projector', 55, '6', 1,  2, 1, 786),

--  this data isnt right...
-- (3591, 828, 309, 54, 'EWar - Max Range', 55, '208', 1, 2, 1, 786),

-- effects below affect other modules or objects and are NOT implemented yet...
(1882, 434, 77, 54, 'Mining Amount Bonus', 55, '54;483', 0,  2, 1, 904),
(3500, 1138, 30, 54, 'Mining Range Bonus', 55, '54;483', 0,  2, 1, 904),

(3586, 828, 566, 54, 'EWar - Scan Strength Bonus', 55, '208', 1,  2, 1, 786),

(1500, 851, 6, 0, 'Capacitor Need Bonus', 40, '3416', 0,  2, 1, 774),
(273, 323, 30, 0, 'Upgrade Power Need Bonus', 40, '3425', 0,  2, 1, 774),,
(2850, 66, 73, 54, 'AB/MWD Duration Bonus', 55, '46', 0,  2, 1, 782),
(212, 310, 50, 0, 'CPU Need Bonus', 0, '3432', 0,  2, 1, 780),
(4162, 846, 1371, 0, 'Sensor Strength Modifier', 0, '479', 0,  2, 1, 780),

(272, 312, 73, 0, 'Repair Systems - Duration Bonus', 0, '1049;1050;1051', 0,  2, 1, 773),
(227, 317, 6, 0, 'Acceration Control - Cap Need Bonus', 0, '46', 0,  2, 1, 782),
(623, 434, 77, 0, 'Mining Drone Operation - Mining Amount Bonus', 0, '18', 0,  2, 1, 778),

(2794, 902, 902, 54, 'Salvaging - Access Difficulty Bonus', 55, '25861;30836', 0, 2, 1, 773),

(1281, 806, 84, 54, 'Remote Repair Pump - Amount Bonus', 55, '1049;1050;1051', 1, 2, 1, 773),
(1030, 317, 6, 54, 'Remote Repair Augmentor - Cap Need Bonus', 55, '325', 0, 2, 1, 773),

(504, 459, 459, 54, 'EW Drone Range Augmentor', 55, '639', 0, 2, 1, 778),
(2015, 327, 263, 54, 'Drone Durability Enhancer - Shield', 55, '18', 0, 2, 1, 778),
(2016, 327, 265, 54, 'Drone Durability Enhancer - Armor', 55, '18', 0, 2, 1, 778),
(2017, 327, 9, 54, 'Drone Durability Enhancer - Hull', 55, '18', 0, 2, 1, 778),
(2019, 39, 68, 54, 'Drone Repair Augmentor - Shield', 55, '18', 0, 2, 1, 778),
(2020, 39, 84, 54, 'Drone Repair Augmentor - Armor', 55, '18', 0, 2, 1, 778),
(2014, 294, 54, 54, 'Drone Scope Chip', 55, '18', 0, 2, 1, 778),
(2013, 591, 37, 54, 'Drone Speed Augmentor', 55, '18', 1, 2, 1, 778),
(2867, 292, 64, 56, 'Sentry Damage Augmentor', 57, '911', 1, 2, 1, 778),
(4579, 1164, 20, 54, 'Stasis Drone Augmentor', 55, '18', 0, 2, 1, 778),
(2853, 619, 560, 54, 'Targeting Systems Stabiizer', 55, '330', 0, 2, 1, 786),
(3560, 828, 351, 54, 'Tracking Diagnostic Subroutines', 55, '291', 1, 2, 1, 786),
(3561, 828, 767, 54, 'Tracking Diagnostic Subroutines', 55, '291', 1, 2, 1, 786),
(1318, 828, 238, 54, 'Particle Dispersion Augmentor - Grav', 55, '201', 1, 2, 1, 786),
(1318, 828, 239, 54, 'Particle Dispersion Augmentor - Ladar', 55, '201', 1, 2, 1, 786),
(1318, 828, 240, 54, 'Particle Dispersion Augmentor - Mag', 55, '201', 1, 2, 1, 786),
(1318, 828, 241, 54, 'Particle Dispersion Augmentor - Radar', 55, '201', 1, 2, 1, 786),
(1445, 294, 54, 54, 'Particle Dispersion Projector – RSD Max Range', 55, '201;208;291;379', 1, 2, 1, 786),
(1446, 294, 54, 54, 'Particle Dispersion Projector – TP Max Range', 55, '201;208;291;379', 1, 2, 1, 786),
(1448, 294, 54, 54, 'Particle Dispersion Projector – TD Max Range', 55, '201;208;291;379', 1, 2, 1, 786),
(1452, 294, 54, 54, 'Particle Dispersion Projector – EW Max Range', 55, '201;208;291;379', 1, 2, 1, 786),
(2491, 294, 54, 54, 'Particle Dispersion Projector – ECM Burst Range', 55, '201;208;291;379', 1, 2, 1, 786),
(744, 308, 73, 54, 'Signal Focusing Kit', 55, '47;48;49', 0, 2, 1, 786),
(236, 317, 6, 54, 'Egress Port Maximizer', 55, '67;71', 0, 2, 1, 781),
(396, 310, 50, 54, 'Powergrid Subroutine Maximizer', 55, '43;61;766;767;768;769', 0, 2, 1, 781),
(2690, 310, 50, 54, 'Algid Energy Administrations Unit', 55, '53', 0, 2, 1, 775),
(2693, 349, 158, 54, 'Energy Ambit Extension', 55, '53', 1, 2, 1, 775),
(2801, 204, 51, 54, 'Energy Burst Aerator', 55, '53', 1, 2, 1, 775),
(2803, 64, 64, 56, 'Energy Collision Accelerator', 57, '53', 1, 2, 1, 775),
(2688, 317, 6, 54, 'Energy Discharge Elutriation', 55, '53', 1, 2, 1, 775),
(2696, 351, 54, 54, 'Energy Locus Coordinator', 55, '53', 1, 2, 1, 775),
(2699, 244, 160, 54, 'Energy Metastasis Adjuster', 55, '53', 1, 2, 1, 775),

(2691, 310, 50, 54, 'Algid Hybrid Administrations Unit', 55, '74', 0, 2, 1, 776),
(2694, 349, 158, 54, 'Hybrid Ambit Extension', 55, '74', 1, 2, 1, 776),
(2804, 204, 51, 54, 'Hybrid Burst Aerator', 55, '74', 1, 2, 1, 776),
(2802, 64, 64, 56, 'Hybrid Collision Accelerator', 57, '74', 1, 2, 1, 776),
(2689, 317, 6, 54, 'Hybrid Discharge Elutriation', 55, '74', 1, 2, 1, 776),
(2697, 351, 54, 54, 'Hybrid Locus Coordinator', 55, '74', 1, 2, 1, 776),
(2700, 244, 160, 5, 'Hybrid Metastasis Adjuster', 25, '74', 1, 2, 1, 776),

(2799, 204, 51, 54, 'Bay Loading Accelerator', 55, '511;524;506;510;771;507;508;509', 1, 2, 1, 779),
(1764, 20, 37, 54, 'Hydraulic Bay Thrusters', 55, '654;656;655;653;648;657;772;476;386;88;396;395;394;385;384;387;89', 2, 2, 1, 779),
(2851, 213, 114, 54, 'Warhead Calefaction Catalyst', 55, '654;656;655;653;648;657;772;476;386;88;396;395;394;385;384;387;89', 1, 2, 1, 779),
(2851, 213, 116, 54, 'Warhead Calefaction Catalyst', 55, '654;656;655;653;648;657;772;476;386;88;396;395;394;385;384;387;89', 1, 2, 1, 779),
(2851, 213, 117, 54, 'Warhead Calefaction Catalyst', 55, '654;656;655;653;648;657;772;476;386;88;396;395;394;385;384;387;89', 1, 2, 1, 779),
(2851, 213, 118, 54, 'Warhead Calefaction Catalyst', 55, '654;656;655;653;648;657;772;476;386;88;396;395;394;385;384;387;89', 1, 2, 1, 779),
(1590, 847, 653, 54, 'Warhead Flare Catalyst', 55, '654;656;655;653;648;657;772;476;386;88;396;395;394;385;384;387;89', 0, 2, 1, 779),
(1472, 848, 654, 54, 'Warhead Rigor Catalyst', 55, '656;655;653;386;396;395;394;385;384', 0, 2, 1, 779),

(2695, 349, 158, 54, 'Projectile Falloff Bonus', 55, '55', 1, 2, 1, 777),
(2698, 351, 54, 54, 'Projectile Range Bonus', 55, '55', 1, 2, 1, 777),
(2798, 64, 64, 56, 'Projectile Damager Bonus', 57, '55', 1, 2, 1, 777),
(2797, 204, 51, 54, 'Projectile ROF Bonus', 55, '55', 1, 2, 1, 777),
(2701, 244, 160, 54, 'Projectile Tracking Speed Bonus', 55, '55', 1, 2, 1, 777);
