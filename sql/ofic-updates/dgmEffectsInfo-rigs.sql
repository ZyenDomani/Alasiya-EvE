
-- rig effects data

INSERT INTO `dgmEffectsInfo` (`effectID`, `sourceAttributeID`, `targetAttributeID`, `calculationTypeID`, `description`, `reverseCalculationTypeID`, `targetGroupIDs`, `stackingPenaltyApplied`, `effectAppliedInState`, `affectingID`, `affectingType`, `affectedType`)
VALUES

(2663, 1153, 1152, 1, 'Cal -> Cal-Used', 8, '6', 0, 14, 0, 0, 3),

(2706, 1138, 30, 54, 'Rig Drawback - Power Need Laser', 55, '53', 0, 15, 775, 2, 2),
(2707, 1138, 30, 54, 'Rig Drawback - Power Need Hybrid', 55, '74', 0, 15, 776, 2, 2),
(2708, 1138, 30, 54, 'Rig Drawback - Power Need Projectile', 55, '55', 0, 15, 777, 2, 2),
(2709, 1138, 30, 54, 'Rig Drawback - Power Need Launcher', 55, '511;524;506;510;771;507;508;509', 0, 15, 779, 2, 2),
-- dont have all the data for these yet...
-- (2710, 1138, 30, 54, 'Rig Drawback - Power Need Gunnery', 55, '55', 0, 15, 777, 2, 2),
(2712, 1138, 265, 54, 'Rig Drawback - Armor HP', 55, '6', 0, 15, 782, 2, 3),
(2713, 1138, 48, 54, 'Rig Drawback - CPU Output', 55, '6', 0, 15, 778, 2, 3),
(2714, 1138, 50, 54, 'Rig Drawback - CPU Need Launchers', 55, '511;524;506;510;771;507;508;509', 0, 15, 779, 2, 2),
-- (2715, 1138, 50, 54, 'Rig Drawback - CPU Need Gunnery', 55, '', 0, 15, 779, 2, 2),
(2716, 1138, 552, 54, 'Rig Drawback - Sig Radius', 55, '6', 1, 15, 774, 2, 3),
(2717, 1138, 37, 54, 'Rig Drawback - Max Velocity', 55, '6', 2, 15, 773, 2, 3),
(2718, 1138, 263, 54, 'Rig Drawback - Shield Capacity', 55, '6', 0, 15, 786, 2, 3),
-- (2719, 1138, 30, 54, 'Rig Drawback - Shield Uniformity', 55, '55', 0, 15, 777, 2, 2),
-- (2720, 1138, 30, 54, 'Rig Drawback - Capacitor Capacity', 55, '55', 0, 15, 777, 2, 2),
-- (2721, 1138, 30, 54, 'Rig Drawback - Power Output', 55, '55', 0, 15, 777, 2, 2),
-- (2722, 1138, 30, 54, 'Rig Drawback - ROF Gunnery', 55, '55', 0, 15, 777, 2, 2),
(2723, 1138, 30, 54, 'Rig Drawback - ROF Launcher', 55, '511;524;506;510;771;507;508;509', 0, 15, 779, 2, 2),
-- (2724, 1138, 30, 54, 'Rig Drawback - Bonus Effect', 55, '55', 0, 15, 777, 2, 2),
-- (2725, 1138, 30, 54, 'Rig Drawback - Skill Effect', 55, '55', 0, 15, 777, 2, 2),
(3528, 1138, 30, 54, 'Rig Drawback - Cap Recharge Rate', 55, '6', 0, 15, 904, 2, 2),

(486, 338, 479, 0, 'Shield Recharge Rate Bonus', 40, '6', 0, 15, 774, 2, 3),

(3500, 1138, 30, 54, 'Mining Laser Range Bonus', 55, '54;483', 0, 15, 904, 2, 2),
(1882, 434, 77, 54, 'Mining Amount Bonus', 55, '54;483', 0, 14, 904, 2, 4),
(1882, 434, 77, 54, 'Mining Amount Bonus', 55, '54;483', 0, 14, 546, 2, 4),

(3586, 828, 566, 54, 'EWar - Scan Strength Bonus', 55, '208', 1, 15, 786, 2, 2),
--  this data isnt right...
-- (3591, 828, 309, 54, 'EWar - Max Range', 55, '208', 1, 15, 786, 2, 2),

(2794, 902, 902, 54, 'Salvaging - Access Difficulty Bonus', 55, '25861;30836', 0, 15, 773, 2, 1),

(1281, 806, 84, 54, 'Remote Repair Pump - Amount Bonus', 55, '1049;1050;1051', 1, 15, 773, 2, 4),
(1030, 317, 6, 54, 'Remote Repair Augmentor - Cap Need Bonus', 55, '325', 0, 15, 773, 2, 2),

(836, 614, 38, 54, 'Cargohold Bonus', 55, '6', 0, 15, 782, 2, 3),

(856, 624, 1281, 54, 'Hyperspatial Velocity Optimizer', 55, '6', 0, 15, 782, 2, 3),
(395, 151, 70, 54, 'Low Friction Nozzle Joints', 55, '6', 1, 15, 782, 2, 3),
(3727, 1076, 37, 54, 'Polycarbon Engine Housing - Velocity', 55, '6', 1, 15, 782, 2, 3),
(3726, 169, 70, 54, 'Polycarbon Engine Housing - Inertia', 55, '6', 1, 15, 782, 2, 3),
(494, 319, 153, 54, 'Warp Core Optimizer', 55, '6', 0, 15, 782, 2, 3),

(504, 459, 459, 54, 'Drone Control Range Augmentor', 55, '6', 0, 15, 778, 2, 3),
(504, 459, 459, 54, 'EW Drone Range Augmentor', 55, '639', 0, 15, 778, 2, 2),
(2015, 327, 263, 54, 'Drone Durability Enhancer - Shield', 55, '18', 0, 15, 778, 2, 3),
(2016, 327, 265, 54, 'Drone Durability Enhancer - Armor', 55, '18', 0, 15, 778, 2, 3),
(2017, 327, 9, 54, 'Drone Durability Enhancer - Hull', 55, '18', 0, 15, 778, 2, 3),
(2019, 39, 68, 54, 'Drone Repair Augmentor - Shield', 55, '18', 0, 15, 778, 2, 3),
(2020, 39, 84, 54, 'Drone Repair Augmentor - Armor', 55, '18', 0, 15, 778, 2, 3),
(2014, 294, 54, 54, 'Drone Scope Chip', 55, '18', 0, 15, 778, 2, 3),
(2013, 591, 37, 54, 'Drone Speed Augmentor', 55, '18', 1, 15, 778, 2, 3),
(2867, 292, 64, 56, 'Sentry Damage Augmentor', 57, '911', 1, 15, 778, 2, 4),
(4579, 1164, 20, 54, 'Stasis Drone Augmentor', 55, '18', 0, 15, 778, 2, 3),

(2852, 309, 76, 54, 'Ionic Field Projector', 55, '6', 1, 15, 786, 2, 3),
(1318, 828, 238, 54, 'Particle Dispersion Augmentor - Grav', 55, '201', 1, 15, 786, 2, 2),
(1318, 828, 239, 54, 'Particle Dispersion Augmentor - Ladar', 55, '201', 1, 15, 786, 2, 2),
(1318, 828, 240, 54, 'Particle Dispersion Augmentor - Mag', 55, '201', 1, 15, 786, 2, 2),
(1318, 828, 241, 54, 'Particle Dispersion Augmentor - Radar', 55, '201', 1, 15, 786, 2, 2),
(1445, 294, 54, 54, 'Particle Dispersion Projector – RSD Max Range', 55, '201;208;291;379', 1, 15, 786, 2, 2),
(1446, 294, 54, 54, 'Particle Dispersion Projector – TP Max Range', 55, '201;208;291;379', 1, 15, 786, 2, 2),
(1448, 294, 54, 54, 'Particle Dispersion Projector – TD Max Range', 55, '201;208;291;379', 1, 15, 786, 2, 2),
(1452, 294, 54, 54, 'Particle Dispersion Projector – EW Max Range', 55, '201;208;291;379', 1, 15, 786, 2, 2),
(2491, 294, 54, 54, 'Particle Dispersion Projector – ECM Burst Range', 55, '201;208;291;379', 1, 15, 786, 2, 2),
(744, 308, 73, 54, 'Signal Focusing Kit', 55, '47;48;49', 0, 15, 786, 2, 2),
(854, 565, 564, 54, 'Targeting System Subcontroller', 55, '6', 1, 15, 786, 2, 3),
(2853, 619, 560, 54, 'Targeting Systems Stabiizer', 55, '330', 0, 15, 786, 2, 2),
(3560, 828, 351, 54, 'Tracking Diagnostic Subroutines', 55, '291', 1, 15, 786, 2, 2),
(3561, 828, 767, 54, 'Tracking Diagnostic Subroutines', 55, '291', 1, 15, 786, 2, 2),

(485, 314, 55, 54, 'Capacitor Control Circuit', 55, '6', 0, 15, 781, 2, 3),
(236, 317, 6, 54, 'Egress Port Maximizer', 55, '67;71', 0, 15, 781, 2, 2),
(396, 310, 50, 54, 'Powergrid Subroutine Maximizer', 55, '43;61;766;767;768;769', 0, 15, 781, 2, 2),
(2432, 1079, 482, 54, 'Semiconductor Memory Cell', 55, '6', 0, 15, 781, 2, 3),

(2690, 310, 50, 54, 'Algid Energy Administrations Unit', 55, '53', 0, 15, 775, 2, 2),
(2693, 349, 158, 54, 'Energy Ambit Extension', 55, '53', 1, 15, 775, 2, 2),
(2801, 204, 51, 54, 'Energy Burst Aerator', 55, '53', 1, 15, 775, 2, 2),
(2803, 64, 64, 56, 'Energy Collision Accelerator', 57, '53', 1, 15, 775, 2, 2),
(2688, 317, 6, 54, 'Energy Discharge Elutriation', 55, '53', 1, 15, 775, 2, 2),
(2696, 351, 54, 54, 'Energy Locus Coordinator', 55, '53', 1, 15, 775, 2, 2),
(2699, 244, 160, 54, 'Energy Metastasis Adjuster', 55, '53', 1, 15, 775, 2, 2),

(2691, 310, 50, 54, 'Algid Hybrid Administrations Unit', 55, '74', 0, 15, 776, 2, 2),
(2694, 349, 158, 54, 'Hybrid Ambit Extension', 55, '74', 1, 15, 776, 2, 2),
(2804, 204, 51, 54, 'Hybrid Burst Aerator', 55, '74', 1, 15, 776, 2, 2),
(2802, 64, 64, 56, 'Hybrid Collision Accelerator', 57, '74', 1, 15, 776, 2, 2),
(2689, 317, 6, 54, 'Hybrid Discharge Elutriation', 55, '74', 1, 15, 776, 2, 2),
(2697, 351, 54, 54, 'Hybrid Locus Coordinator', 55, '74', 1, 15, 776, 2, 2),
(2700, 244, 160, 5, 'Hybrid Metastasis Adjuster', 25, '74', 1, 15, 776, 2, 2),

(2799, 204, 51, 54, 'Bay Loading Accelerator', 55, '511;524;506;510;771;507;508;509', 1, 15, 779, 2, 2),
(1764, 20, 37, 54, 'Hydraulic Bay Thrusters', 55, '654;656;655;653;648;657;772;476;386;88;396;395;394;385;384;387;89', 2, 15, 779, 2, 2),
(2851, 213, 114, 54, 'Warhead Calefaction Catalyst', 55, '654;656;655;653;648;657;772;476;386;88;396;395;394;385;384;387;89', 1, 15, 779, 2, 2),
(2851, 213, 116, 54, 'Warhead Calefaction Catalyst', 55, '654;656;655;653;648;657;772;476;386;88;396;395;394;385;384;387;89', 1, 15, 779, 2, 2),
(2851, 213, 117, 54, 'Warhead Calefaction Catalyst', 55, '654;656;655;653;648;657;772;476;386;88;396;395;394;385;384;387;89', 1, 15, 779, 2, 2),
(2851, 213, 118, 54, 'Warhead Calefaction Catalyst', 55, '654;656;655;653;648;657;772;476;386;88;396;395;394;385;384;387;89', 1, 15, 779, 2, 2),
(1590, 847, 653, 54, 'Warhead Flare Catalyst', 55, '654;656;655;653;648;657;772;476;386;88;396;395;394;385;384;387;89', 0, 15, 779, 2, 2),
(1472, 848, 654, 54, 'Warhead Rigor Catalyst', 55, '656;655;653;386;396;395;394;385;384', 0, 15, 779, 2, 2),

(2695, 349, 158, 54, 'Projectile Falloff Bonus', 55, '55', 1, 15, 777, 2, 2),
(2698, 351, 54, 54, 'Projectile Range Bonus', 55, '55', 1, 15, 777, 2, 2),
(2798, 64, 64, 56, 'Projectile Damager Bonus', 57, '55', 1, 15, 777, 2, 2),
(2797, 204, 51, 54, 'Projectile ROF Bonus', 55, '55', 1, 15, 777, 2, 2),
(2701, 244, 160, 54, 'Projectile Tracking Speed Bonus', 55, '55', 1, 15, 777, 2, 2);
