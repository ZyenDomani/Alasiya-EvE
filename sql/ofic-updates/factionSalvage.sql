
CREATE TABLE `factionSalvage` (
  `factionID` int(10) unsigned NOT NULL,
  `techLvl` TINYINT UNSIGNED NOT NULL,
  `itemID` int(10) unsigned NOT NULL,
  `itemName` varchar(45) COLLATE utf8_unicode_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;


INSERT INTO `factionSalvage` (`factionID`, `techLvl`, `itemID`, `itemName`)
  VALUES

-- T1 salvage
-- Caldari
(500001,1,25588,'Scorched Telemetry Processor'),
(500001,1,25589,'Malfunctioning Shield Emitter'),
(500001,1,25598,'Tripped Power Circuit'),
(500001,1,25606,'Ward Console'),

-- Minmitar
(500002,1,25593,'Smashed Trigger Unit'),
(500002,1,25595,'Alloyed Tritanium Bar'),
(500002,1,25599,'Charred Micro Circuit'),
(500002,1,25600,'Burned Logic Circuit'),
(500002,1,25602,'Thruster Console'),

-- Amarr
(500003,1,25590,'Contaminated Nanite Compound'),
(500003,1,25592,'Defective Current Pump'),
(500003,1,25601,'Fried Interface Circuit'),
(500003,1,25603,'Melted Capacitor Console'),

-- Gallente
(500004,1,25591,'Contaminated Lorentz Fluid'),
(500004,1,25596,'Broken Drone Transceiver'),
(500004,1,25597,'Damaged Artificial Neural Network'),
(500004,1,25604,'Conductive Polymer'),

-- angel
(500011,1,25595,'Alloyed Tritanium Bar'),
(500011,1,25605,'Armor Plates'),
(500011,1,25600,'Burned Logic Circuit'),
(500011,1,25599,'Charred Micro Circuit'),
(500011,1,25590,'Contaminated Nanite Compound'),
(500011,1,25601,'Fried Interface Circuit'),
(500011,1,25589,'Malfunctioning Shield Emitter'),
(500011,1,25593,'Smashed Trigger Unit'),
(500011,1,25602,'Thruster Console'),
(500011,1,25598,'Tripped Power Circuit'),

-- raider
(500012,1,25605,'Armor Plates'),
(500012,1,25600,'Burned Logic Circuit'),
(500012,1,25599,'Charred Micro Circuit'),
(500012,1,25590,'Contaminated Nanite Compound'),
(500012,1,25592,'Defective Current Pump'),
(500012,1,25601,'Fried Interface Circuit'),
(500012,1,25603,'Melted Capacitor Console'),
(500012,1,25594,'Tangled Power Conduit'),
(500012,1,25598,'Tripped Power Circuit'),

-- gurista
(500010,1,25600,'Burned Logic Circuit'),
(500010,1,25599,'Charred Micro Circuit'),
(500010,1,25604,'Conductive Polymer'),
(500010,1,25597,'Damaged Artificial Neural Network'),
(500010,1,25601,'Fried Interface Circuit'),
(500010,1,25589,'Malfunctioning Shield Emitter'),
(500010,1,25588,'Scorched Telemetry Processor'),
(500010,1,25598,'Tripped Power Circuit'),
(500010,1,25606,'Ward Console'),

-- sansha
(500019,1,25605,'Armor Plates'),
(500019,1,25600,'Burned Logic Circuit'),
(500019,1,25599,'Charred Micro Circuit'),
(500019,1,25590,'Contaminated Nanite Compound'),
(500019,1,25592,'Defective Current Pump'),
(500019,1,25601,'Fried Interface Circuit'),
(500019,1,25603,'Melted Capacitor Console'),
(500019,1,25594,'Tangled Power Conduit'),
(500019,1,25598,'Tripped Power Circuit'),

-- serpentis
(500020,1,25605,'Armor Plates'),
(500020,1,25596,'Broken Drone Transceiver'),
(500020,1,25600,'Burned Logic Circuit'),
(500020,1,25599,'Charred Micro Circuit'),
(500020,1,25604,'Conductive Polymer'),
(500020,1,25591,'Contaminated Lorentz Fluid'),
(500020,1,25590,'Contaminated Nanite Compound'),
(500020,1,25597,'Damaged Artificial Neural Network'),
(500020,1,25601,'Fried Interface Circuit'),

-- concord T1
(500006,1,25595,'Alloyed Tritanium Bar'),
(500006,1,25605,'Armor Plates'),
(500006,1,25599,'Charred Micro Circuit'),
(500006,1,25604,'Conductive Polymer'),
(500006,1,25591,'Contaminated Lorentz Fluid'),
(500006,1,25597,'Damaged Artificial Neural Network'),
(500006,1,25592,'Defective Current Pump'),
(500006,1,25601,'Fried Interface Circuit'),
(500006,1,25589,'Malfunctioning Shield Emitter'),
(500006,1,25603,'Melted Capacitor Console'),
(500006,1,25593,'Smashed Trigger Unit'),
(500006,1,25594,'Tangled Power Conduit'),
(500006,1,25602,'Thruster Console'),
(500006,1,25598,'Tripped Power Circuit'),

-- drone
(500022,1,25595,'Alloyed Tritanium Bar'),
(500022,1,25605,'Armor Plates'),
(500022,1,25596,'Broken Drone Transceiver'),
(500022,1,25600,'Burned Logic Circuit'),
(500022,1,25599,'Charred Micro Circuit'),
(500022,1,25604,'Conductive Polymer'),
(500022,1,25591,'Contaminated Lorentz Fluid'),
(500022,1,25590,'Contaminated Nanite Compound'),
(500022,1,25592,'Defective Current Pump'),
(500022,1,25601,'Fried Interface Circuit'),
(500022,1,25589,'Malfunctioning Shield Emitter'),
(500022,1,25603,'Melted Capacitor Console'),
(500022,1,25588,'Scorched Telemetry Processor'),
(500022,1,25593,'Smashed Trigger Unit'),
(500022,1,25594,'Tangled Power Conduit'),
(500022,1,25602,'Thruster Console'),
(500022,1,25598,'Tripped Power Circuit'),
(500022,1,25606,'Ward Console'),

-- default 'unknown' faction catch-all for T1
(500021,1,25595,'Alloyed Tritanium Bar'),
(500021,1,25605,'Armor Plates'),
(500021,1,25596,'Broken Drone Transceiver'),
(500021,1,25600,'Burned Logic Circuit'),
(500021,1,25599,'Charred Micro Circuit'),
(500021,1,25604,'Conductive Polymer'),
(500021,1,25591,'Contaminated Lorentz Fluid'),
(500021,1,25590,'Contaminated Nanite Compound'),
(500021,1,25597,'Damaged Artificial Neural Network'),
(500021,1,25592,'Defective Current Pump'),
(500021,1,25601,'Fried Interface Circuit'),
(500021,1,25589,'Malfunctioning Shield Emitter'),
(500021,1,25603,'Melted Capacitor Console'),
(500021,1,25588,'Scorched Telemetry Processor'),
(500021,1,25593,'Smashed Trigger Unit'),
(500021,1,25594,'Tangled Power Conduit'),
(500021,1,25602,'Thruster Console'),
(500021,1,25598,'Tripped Power Circuit'),
(500021,1,25606,'Ward Console'),

-- factions todo
/*    do any of these have ships?
    factionJove          = 500005,
    factionAmmatar       = 500007,
    factionKhanid        = 500008,
    factionSyndicate     = 500009,
    factionInterBus      = 500013,
    factionThukker       = 500015,
    factionSistersOfEVE  = 500016,
    factionSociety       = 500017,
    factionMordusLegion  = 500018,
*/

-- T2 salvage
-- Caldari
(500001,2,25607,'Telemetry Processor'),
(500001,2,25608,'Intact Shield Emitter'),
(500001,2,25625,'Enhanced Ward Console'),

-- Minmitar
(500002,2,25612,'Trigger Unit'),
(500002,2,25617,'Power Circuit'),
(500002,2,25624,'Intact Armor Plates'),

-- Amarr
(500003,2,25609,'Nanite Compound'),
(500003,2,25611,'Current Pump'),
(500003,2,25622,'Capacitor Console'),

-- Gallente
(500004,2,25610,'Lorentz Fluid'),
(500004,2,25615,'Drone Transceiver'),
(500004,2,25616,'Artificial Neural Network'),

-- raider
(500012,2,25622,'Capacitor Console'),
(500012,2,25613,'Power Conduit'),
(500012,2,25617,'Power Circuit'),
(500012,2,25619,'Logic Circuit'),
(500012,2,25624,'Intact Armor Plates'),

-- ore/dcm
(500014,2,25607,'Telemetry Processor'),
(500014,2,25608,'Intact Shield Emitter'),
(500014,2,25609,'Nanite Compound'),
(500014,2,25610,'Lorentz Fluid'),
(500014,2,25611,'Current Pump'),
(500014,2,25612,'Trigger Unit'),
(500014,2,25613,'Power Conduit'),
(500014,2,25614,'Single-crystal Superalloy I-beam'),
(500014,2,25615,'Drone Transceiver'),
(500014,2,25616,'Artificial Neural Network'),
(500014,2,25617,'Power Circuit'),
(500014,2,25618,'Micro Circuit'),
(500014,2,25619,'Logic Circuit'),
(500014,2,25620,'Interface Circuit'),
(500014,2,25621,'Impetus Console'),
(500014,2,25622,'Capacitor Console'),
(500014,2,25623,'Conductive Thermoplastic'),
(500014,2,25624,'Intact Armor Plates'),
(500014,2,25625,'Enhanced Ward Console'),

-- concord T2
(500006,2,25607,'Telemetry Processor'),
(500006,2,25608,'Intact Shield Emitter'),
(500006,2,25609,'Nanite Compound'),
(500006,2,25610,'Lorentz Fluid'),
(500006,2,25611,'Current Pump'),
(500006,2,25612,'Trigger Unit'),
(500006,2,25613,'Power Conduit'),
(500006,2,25614,'Single-crystal Superalloy I-beam'),
(500006,2,25615,'Drone Transceiver'),
(500006,2,25616,'Artificial Neural Network'),
(500006,2,25617,'Power Circuit'),
(500006,2,25618,'Micro Circuit'),
(500006,2,25619,'Logic Circuit'),
(500006,2,25620,'Interface Circuit'),
(500006,2,25621,'Impetus Console'),
(500006,2,25622,'Capacitor Console'),
(500006,2,25623,'Conductive Thermoplastic'),
(500006,2,25624,'Intact Armor Plates'),
(500006,2,25625,'Enhanced Ward Console'),

-- default 'unknown' faction catch-all for T2
(500021,2,25607,'Telemetry Processor'),
(500021,2,25608,'Intact Shield Emitter'),
(500021,2,25609,'Nanite Compound'),
(500021,2,25610,'Lorentz Fluid'),
(500021,2,25611,'Current Pump'),
(500021,2,25612,'Trigger Unit'),
(500021,2,25613,'Power Conduit'),
(500021,2,25614,'Single-crystal Superalloy I-beam'),
(500021,2,25615,'Drone Transceiver'),
(500021,2,25616,'Artificial Neural Network'),
(500021,2,25617,'Power Circuit'),
(500021,2,25618,'Micro Circuit'),
(500021,2,25619,'Logic Circuit'),
(500021,2,25620,'Interface Circuit'),
(500021,2,25621,'Impetus Console'),
(500021,2,25622,'Capacitor Console'),
(500021,2,25623,'Conductive Thermoplastic'),
(500021,2,25624,'Intact Armor Plates'),
(500021,2,25625,'Enhanced Ward Console'),


-- T3 salvage
-- sleeper (T3)
(500023,3,30018,'Fused Nanomechanical Engines'),
(500023,3,30019,'Powdered C-540 Graphite'),
(500023,3,30021,'Modified Fluid Router'),
(500023,3,30022,'Heuristic Selfassemblers'),
(500023,3,30024,'Cartesian Temporal Coordinator'),
(500023,3,30248,'Emergent Combat Analyzer'),
(500023,3,30251,'Neurovisual Input Matrix'),
(500023,3,30252,'Thermoelectric Catalysts'),
(500023,3,30254,'Electromechanical Hull Sheeting'),
(500023,3,30258,'Resonance Calibration Matrix'),
(500023,3,30259,'Melted Nanoribbons'),
(500023,3,30268,'Jump Drive Control Nexus'),
(500023,3,30269,'Defensive Control Node'),
(500023,3,30270,'Central System Controller'),
(500023,3,30271,'Emergent Combat Intelligence'),
(500023,3,30497,'Reinforced Metal Scraps'); -- loot, not salvage
