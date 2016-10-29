
/* Dumping data for table `chrRaces` */

INSERT INTO `chrRaces` (`raceID`, `raceName`, `description`, `iconID`, `shortDescription`, `raceNameID`, `descriptionID`, `dataID`) VALUES
(1, 'Caldari', 'Founded on the tenets of patriotism and hard work that carried its ancestors through hardships on an inhospitable homeworld, the Caldari State is today a corporate dictatorship, led by rulers who are determined to see it return to the meritocratic ideals of old. Ruthless and efficient in the boardroom as well as on the battlefield, the Caldari are living emblems of strength, persistence, and dignity.', 1439, 'The Caldari State is the epitome of civic duty and ruthless efficiency.', 59592, 59586, 16544347),
(2, 'Minmatar', 'Once a thriving tribal civilization, the Minmatar were enslaved by the Amarr Empire for more than 700 years until a massive rebellion freed most, but not all, of those held in servitude. The Minmatar people today are resilient, ingenious, and hard-working. Many of them believe that democracy, though it has served them well for a long time, can never restore what was taken from them so long ago. For this reason they have formed a government truly reflective of their tribal roots. They will forever resent the Amarrians, and yearn for the days before the Empire’s accursed ships ever reached their home skies.', 1440, 'Breaking free of Amarrian subjugation, the Minmatar Republic is a nation of resilient, ingenious, hard-working people who thrive in a tribal culture.', 59593, 59587, 16544348),
(4, 'Amarr', 'The Amarr Empire is the largest and oldest of the four empires. Ruled by a mighty God-Emperor, this vast theocratic society is supported by a broad foundation of slave labor. Amarrian citizens tend to be highly educated and fervent individuals, and as a culture Amarr adheres to the basic tenet that what others call slavery is in fact one step on a indentured person’s spiritual path toward fully embracing their faith. Despite several setbacks in recent history, the Empire remains arguably the most stable and militarily powerful nation-state in New Eden', 1442, 'Amarr is the largest empire in New Eden, solely devoted to God, Emperor, and the spread of their faith.', 59594, 59588, 16544349),
(8, 'Gallente', 'Champions of liberty and defenders of the downtrodden, the Gallente play host to the only true democracy in New Eden. Some of the most progressive leaders, scientists, and businessmen of the era have emerged from its diverse peoples. A pioneer of artificial intelligence, the Federation relies heavily on drones and other automated systems. This is not to detract from the skill of their pilots, though: the Gallente Federation is known for producing some of the best and bravest the universe has to offer.', 1441, 'Championing freedom and liberty across the universe, the Gallente Federation is the only true democracy of New Eden.', 59595, 59589, 16544350),
(16, 'Jove', 'The most mysterious and elusive of all the universe''s peoples, the Jovians number only a fraction of any of their neighbors, but their technological superiority makes them powerful beyond all proportion.', 0, '', 59590, 59585, 16544345),
(32, 'Pirate', '', 0, '', 59591, 0, 16544346),
(64, 'Sleepers', '', 0, '', 234419, 0, 16544351),
(128, 'ORE', '', 0, 'ORE', 277200, 277201, 52615708);

/* Dumping data for table `invCategories` */

INSERT INTO `invCategories` (`categoryID`, `categoryName`, `description`, `published`, `iconID`, `categoryNameID`, `dataID`) VALUES
(0, '#System', '', 0, 0, 63539, 16545519),
(1, 'Owner', '', 0, 0, 63540, 16545520),
(2, 'Celestial', '', 1, 0, 63541, 16545521),
(3, 'Station', '', 0, 0, 63542, 16545522),
(4, 'Material', '', 1, 22, 63543, 16545523),
(5, 'Accessories', '', 1, 33, 63560, 16545540),
(6, 'Ship', '', 1, 0, 63544, 16545524),
(7, 'Module', '', 1, 67, 63545, 16545525),
(8, 'Charge', '', 1, 0, 63546, 16545526),
(9, 'Blueprint', '', 1, 21, 63547, 16545527),
(10, 'Trading', '', 0, 0, 63548, 16545528),
(11, 'Entity', '', 0, 0, 63549, 16545529),
(14, 'Bonus', 'Character creation bonuses. Like innate skills but genetic rather than learned.', 0, 0, 63550, 16545530),
(16, 'Skill', 'Where all the skills go under.', 1, 33, 63551, 16545531),
(17, 'Commodity', '', 1, 0, 63552, 16545532),
(18, 'Drone', 'Player owned and controlled drones.', 1, 0, 63553, 16545533),
(20, 'Implant', 'Implant', 1, 0, 63554, 16545534),
(22, 'Deployable', '', 1, 0, 63555, 16545535),
(23, 'Structure', 'Player owned structure related objects', 1, 0, 63556, 16545536),
(24, 'Reaction', '', 1, 0, 63557, 16545537),
(25, 'Asteroid', '', 1, 0, 63558, 16545538),
(26, 'WorldSpace', 'Worldspaces and related stuff', 0, 0, 63568, 16545548),
(29, 'Abstract', 'Abstract grouping, global types and groups for the UI, such as Ranks, Ribbons and Medals.', 0, 0, 63559, 16545539),
(30, 'Apparel', '1. clothing, especially outerwear; garments; attire; raiment.\n2. anything that decorates or covers.\n3. superficial appearance; aspect; guise. ', 1, 0, 63572, 16545551),
(32, 'Subsystem', 'Subsystems for tech 3 ships', 1, 0, 63562, 16545542),
(34, 'Ancient Relics', '', 1, 0, 63561, 16545541),
(35, 'Decryptors', '', 1, 0, 63563, 16545543),
(39, 'Infrastructure Upgrades', '', 1, 0, 63565, 16545545),
(40, 'Sovereignty Structures', '', 1, 0, 63564, 16545544),
(41, 'Planetary Interaction', 'Stuff for planetary interaction', 1, 0, 63569, 16545549),
(42, 'Planetary Resources', 'These are Items that can be extracted from a planet. ', 1, 0, 63566, 16545546),
(43, 'Planetary Commodities', '', 1, 0, 63567, 16545547),
(46, 'Orbitals', 'Anchorable/Onlinable objects that operate similar to POS/SOV structures, but do not link to towers or sovereignty. Each class of orbital defines its own valid anchoring locations via Python code.', 1, 0, 63570, 16545555),
(49, 'Placeables', 'Placeables are things you can put into rooms. ', 0, 0, 63571, 16545550),
(53, 'Effects', '', 0, 0, 63573, 16545552),
(54, 'Lights', '', 0, 0, 63574, 22244434),
(59, 'Cells', '', 0, 0, 235965, 16545554),
(350001, 'Catma', '', 0, 0, 267649, 60304602);


/* Dumping data for table `sklBaseSkills` */

INSERT INTO `sklBaseSkills` (`ID`, `skillTypeID`, `level`) VALUES
(1, 3300, 2),(2, 3327, 2),(3, 3386, 1),(4, 3392, 2),(5, 3402, 1),(6, 3413, 2),(7, 3416, 1),(8, 3426, 2),(9, 3449, 2);

/* Dumping data for table `sklRaceSkills`  */

INSERT INTO `sklRaceSkills` (`id`, `raceID`, `skillTypeID`, `level`) VALUES
(1, 1, 3301, 2),(2, 1, 3330, 2),(3, 1, 3319, 2),(4, 1, 3321, 3),(5, 1, 3413, 2),(6, 1, 3432, 1),(7, 1, 3416, 1),
(8, 1, 3426, 2),(9, 1, 21059, 2),(10, 1, 3425, 1),(11, 2, 3302, 2),(12, 2, 3329, 2),(13, 2, 3416, 2),(14, 2, 3413, 2),
(15, 2, 3426, 2),(16, 2, 3300, 2),(17, 2, 3393, 3),(18, 2, 3394, 4),(19, 4, 3303, 2),(20, 4, 3331, 2),(21, 4, 3392, 2),
(22, 4, 3393, 3),(23, 4, 3394, 3),(24, 4, 3417, 2),(25, 4, 3418, 2),(26, 8, 3301, 2),(27, 8, 3328, 2),(28, 8, 3436, 4),
(29, 8, 3437, 3),(30, 8, 3442, 2),(31, 8, 12305, 2),(32, 8, 3392, 2);

/* Dumping data for table `sklCareerSkills`  */

INSERT INTO `sklCareerSkills` (`careerID`, `skillTypeID`, `level`) VALUES
(11, 3300, 3),(11, 3301, 3),(11, 3311, 3),(11, 3319, 3),(11, 3321, 2),(11, 3327, 2),(11, 3330, 1),(11, 3425, 1),(11, 3432, 2),
(11, 12441, 2),(11, 21059, 2),(14, 3348, 4),(14, 3349, 1),(14, 3350, 3),(14, 3355, 4),(14, 3357, 5),(14, 3363, 5),(14, 3368, 2),
(14, 3443, 5),(14, 16595, 3),(14, 20494, 4),(14, 20495, 2),(17, 3348, 4),(17, 3380, 3),(17, 3385, 3),(17, 3386, 4),(17, 3392, 1),
(17, 3402, 2),(17, 3406, 3),(17, 3425, 1),(17, 3432, 2),(17, 21059, 1),(17, 22536, 1),(21, 3300, 2),(21, 3302, 2),(21, 3310, 4),
(21, 3311, 3),(21, 3319, 3),(21, 3320, 2),(21, 3321, 3),(21, 3329, 2),(21, 3392, 1),(21, 3413, 1),(21, 3416, 2),(21, 3424, 2),
(21, 3425, 3),(21, 3426, 1),(21, 3431, 3),(21, 3436, 2),(21, 3437, 2),(21, 3449, 1),(21, 3450, 2),(21, 3455, 2),(24, 3348, 4),
(24, 3349, 2),(24, 3350, 1),(24, 3355, 3),(24, 3357, 3),(24, 3363, 4),(24, 3368, 3),(24, 3431, 2),(24, 3443, 3),(24, 3449, 1),
(24, 3450, 2),(24, 3455, 2),(24, 16595, 3),(24, 20494, 3),(24, 20495, 3),(27, 3319, 2),(27, 3320, 1),(27, 3321, 2),(27, 3348, 3),
(27, 3380, 4),(27, 3385, 4),(27, 3386, 4),(27, 3387, 3),(27, 3392, 1),(27, 3402, 4),(27, 3403, 3),(27, 3406, 3),(27, 3413, 1),
(27, 3443, 1),(27, 22536, 1),(41, 3300, 2),(41, 3303, 2),(41, 3310, 4),(41, 3311, 3),(41, 3312, 3),(41, 3316, 3),(41, 3317, 2),
(41, 3331, 2),(41, 3392, 1),(41, 3393, 1),(41, 3394, 1),(41, 3413, 2),(41, 3417, 2),(41, 3418, 2),(44, 3348, 4),(44, 3349, 2),
(44, 3350, 1),(44, 3355, 4),(44, 3356, 3),(44, 3357, 5),(44, 3363, 5),(44, 3368, 4),(44, 3443, 5),(44, 3446, 4),(44, 16595, 3),
(44, 20494, 3),(44, 20495, 3),(47, 3348, 3),(47, 3380, 3),(47, 3385, 3),(47, 3386, 4),(47, 3387, 2),(47, 3392, 1),(47, 3393, 1),
(47, 3394, 1),(47, 3402, 3),(47, 3403, 1),(47, 3406, 1),(47, 3443, 2),(47, 22536, 1),(81, 3300, 2),(81, 3301, 2),(81, 3312, 1),
(81, 3316, 2),(81, 3317, 2),(81, 3319, 3),(81, 3323, 3),(81, 3328, 2),(81, 3393, 3),(81, 3426, 2),(81, 3449, 1),(81, 3450, 2),
(81, 3455, 2),(84, 3348, 4),(84, 3349, 2),(84, 3350, 1),(84, 3355, 5),(84, 3356, 3),(84, 3357, 3),(84, 3363, 4),(84, 3368, 3),
(84, 3443, 5),(84, 3444, 3),(84, 3446, 4),(84, 16595, 4),(84, 20494, 3),(84, 20495, 3),(87, 3348, 3),(87, 3380, 4),(87, 3385, 4),
(87, 3386, 3),(87, 3402, 3),(87, 3403, 1),(87, 3406, 1),(87, 3413, 3),(87, 3417, 3),(87, 3418, 3),(87, 3443, 2),(87, 22536, 1);

INSERT INTO `srvStatus` (`AI`, `Online`, `Connections`) VALUES
(1, 0, 0);

INSERT INTO `repFactions` (`fromID`, `toID`, `standing`) VALUES
(500001, 500002, -2),(500001, 500003, 7),(500001, 500004, -5),(500001, 500005, 1.75),(500001, 500006, 3),(500001, 500007, 4),
(500001, 500008, 4.5),(500001, 500009, -2),(500001, 500010, -9),(500001, 500011, 0),(500001, 500012, -7),(500001, 500013, 1.5),
(500001, 500014, -2),(500001, 500015, -2),(500001, 500016, -0.25),(500001, 500017, 1),(500001, 500018, 9),(500001, 500019, -7),
(500001, 500020, 0),(500002, 500001, -2),(500002, 500003, -5),(500002, 500004, 8),(500002, 500005, 2.5),(500002, 500006, 3),
(500002, 500007, -3),(500002, 500008, -5),(500002, 500009, -1),(500002, 500010, 0),(500002, 500011, -9),(500002, 500012, 0),
(500002, 500013, 1.25),(500002, 500014, 4),(500002, 500015, 4),(500002, 500016, 2.5),(500002, 500017, 2),(500002, 500018, -4),
(500002, 500019, 0),(500002, 500020, -7),(500003, 500001, 5),(500003, 500002, -5),(500003, 500004, -2),(500003, 500005, -0.5),
(500003, 500006, 3),(500003, 500007, 9),(500003, 500008, 0.5),(500003, 500009, -2),(500003, 500010, -7),(500003, 500011, 0),
(500003, 500012, -8),(500003, 500013, 0.25),(500003, 500014, -1.25),(500003, 500015, -5),(500003, 500016, -1),(500003, 500017, -0.25),
(500003, 500018, 5),(500003, 500019, -8),(500003, 500020, 0),(500004, 500001, -5),(500004, 500002, 8),(500004, 500003, -2),
(500004, 500005, -0.25),(500004, 500006, 3),(500004, 500007, -2),(500004, 500008, -1),(500004, 500009, -2.5),(500004, 500010, 0),
(500004, 500011, -8),(500004, 500012, 0),(500004, 500013, 3),(500004, 500014, 2),(500004, 500015, -1),(500004, 500016, 8),
(500004, 500017, 1.5),(500004, 500018, -2),(500004, 500019, 0),(500004, 500020, -9),(500005, 500001, 1.75),(500005, 500002, 2.5),
(500005, 500003, -0.5),(500005, 500004, -0.25),(500005, 500006, 0),(500005, 500007, -0.5),(500005, 500008, -1),(500005, 500009, -1),
(500005, 500010, 0),(500005, 500011, 0),(500005, 500012, 0),(500005, 500013, 1.25),(500005, 500014, 1),(500005, 500015, 0.25),
(500005, 500016, 2),(500005, 500017, 9),(500005, 500018, 0.75),(500005, 500019, 0),(500005, 500020, 0),(500006, 500001, 0),
(500006, 500002, 0),(500006, 500003, 0),(500006, 500004, 0),(500006, 500005, 0),(500006, 500007, 0),(500006, 500008, 0),
(500006, 500009, 0),(500006, 500010, -2),(500006, 500011, -1),(500006, 500012, -2),(500006, 500013, 0),(500006, 500014, 0),
(500006, 500015, 0),(500006, 500016, 0),(500006, 500017, 0),(500006, 500018, 0),(500006, 500019, -3),(500006, 500020, -1),
(500007, 500001, 6),(500007, 500002, -6),(500007, 500003, 9),(500007, 500004, -3),(500007, 500005, -0.5),(500007, 500006, 3),
(500007, 500008, 0.5),(500007, 500009, -3),(500007, 500010, -6),(500007, 500011, -2),(500007, 500012, -9),(500007, 500013, 0.75),
(500007, 500014, -1),(500007, 500015, -7),(500007, 500016, -0.5),(500007, 500017, -0.25),(500007, 500018, 4),(500007, 500019, -7),
(500007, 500020, 0),(500008, 500001, 6),(500008, 500002, -4),(500008, 500003, 6),(500008, 500004, -2),(500008, 500005, -1),
(500008, 500006, 3),(500008, 500007, 0.5),(500008, 500009, -1),(500008, 500010, -3),(500008, 500011, 0),(500008, 500012, -7),
(500008, 500013, 0.75),(500008, 500014, -2),(500008, 500015, -3),(500008, 500016, -0.25),(500008, 500017, 0.25),
(500008, 500018, 6),(500008, 500019, -6),(500008, 500020, 0),(500009, 500001, 0),(500009, 500002, -5),(500009, 500003, 0),
(500009, 500004, -6),(500009, 500005, -1),(500009, 500006, 0),(500009, 500007, -3),(500009, 500008, -1),(500009, 500010, -2),
(500009, 500011, 5),(500009, 500012, -1),(500009, 500013, 0),(500009, 500014, 5),(500009, 500015, 0),(500009, 500016, -3),
(500009, 500017, -3),(500009, 500018, -6),(500009, 500019, -1),(500009, 500020, 7),(500010, 500001, 0),(500010, 500002, 0),
(500010, 500003, -7),(500010, 500004, 0),(500010, 500005, 0),(500010, 500006, -2),(500010, 500007, -0.75),(500010, 500008, -2),
(500010, 500009, -1),(500010, 500011, -3),(500010, 500012, 4),(500010, 500013, 0),(500010, 500014, 0),(500010, 500015, -2),
(500010, 500016, 0),(500010, 500017, -1),(500010, 500018, -8),(500010, 500019, 7),(500010, 500020, -3),(500011, 500001, 0),
(500011, 500002, -8),(500011, 500003, 0),(500011, 500004, -7),(500011, 500005, 0),(500011, 500006, -1),(500011, 500007, -2.5),
(500011, 500008, -0.25),(500011, 500009, 4),(500011, 500010, -2),(500011, 500012, -4),(500011, 500013, 0),(500011, 500014, -9),
(500011, 500015, 0),(500011, 500016, -2),(500011, 500017, -0.75),(500011, 500018, 0),(500011, 500019, -2),(500011, 500020, 8),
(500012, 500001, -7),(500012, 500002, 0),(500012, 500003, -8),(500012, 500004, 0),(500012, 500005, 0),(500012, 500006, -2),
(500012, 500007, -4.5),(500012, 500008, -3),(500012, 500009, -1),(500012, 500010, 5),(500012, 500011, -2),(500012, 500013, 0),
(500012, 500014, 0),(500012, 500015, -2),(500012, 500016, 0),(500012, 500017, -1.5),(500012, 500018, -6),(500012, 500019, 4.5),
(500012, 500020, -2),(500013, 500001, 1.5),(500013, 500002, 1.25),(500013, 500003, 0.25),(500013, 500004, 3),(500013, 500005, 1.25),
(500013, 500006, 9),(500013, 500007, 0.75),(500013, 500008, 0.75),(500013, 500009, -1),(500013, 500010, -2),(500013, 500011, -3.5),
(500013, 500012, -3),(500013, 500014, 0.5),(500013, 500015, -0.25),(500013, 500016, 5),(500013, 500017, 1.5),(500013, 500018, 1.5),
(500013, 500019, -1.75),(500013, 500020, -1),(500014, 500001, -2),(500014, 500002, 4),(500014, 500003, -3),(500014, 500004, 5),
(500014, 500005, 1),(500014, 500006, 3),(500014, 500007, -1),(500014, 500008, -3.75),(500014, 500009, -1),(500014, 500010, 0),
(500014, 500011, -3),(500014, 500012, 0),(500014, 500013, 0.5),(500014, 500015, -1),(500014, 500016, 0.25),(500014, 500017, -1.5),
(500014, 500018, 0),(500014, 500019, 0),(500014, 500020, -6),(500015, 500001, -3),(500015, 500002, 3),(500015, 500003, -4),
(500015, 500004, -2),(500015, 500005, 0.25),(500015, 500006, 0),(500015, 500007, -7),(500015, 500008, -2.5),(500015, 500009, 0),(500015, 500010, -2),
(500015, 500011, 0),(500015, 500012, -4),(500015, 500013, 0),(500015, 500014, -1),(500015, 500016, -1),(500015, 500017, -0.5),
(500015, 500018, -1),(500015, 500019, -3),(500015, 500020, 0),(500016, 500001, 0),(500016, 500002, 4),(500016, 500003, 0),
(500016, 500004, 8),(500016, 500005, 2),(500016, 500006, 5),(500016, 500007, 0),(500016, 500008, 0),(500016, 500009, -3),
(500016, 500010, -1),(500016, 500011, -7),(500016, 500012, -3),(500016, 500013, 5),(500016, 500014, 3),(500016, 500015, -0.25),
(500016, 500017, 6),(500016, 500018, 0),(500016, 500019, -4),(500016, 500020, -7),(500017, 500001, 1),(500017, 500002, 2),
(500017, 500003, -0.25),(500017, 500004, 1.5),(500017, 500005, 9),(500017, 500006, 0),(500017, 500007, -0.25),(500017, 500008, 0.25),
(500017, 500009, -3),(500017, 500010, -1),(500017, 500011, -0.75),(500017, 500012, -1.5),(500017, 500013, 1.5),(500017, 500014, -1.5),
(500017, 500015, -0.5),(500017, 500016, 6),(500017, 500018, 1.5),(500017, 500019, -1.75),(500017, 500020, -1.25),(500018, 500001, 9),
(500018, 500002, -2),(500018, 500003, 5),(500018, 500004, -3),(500018, 500005, 0.75),(500018, 500006, 3),(500018, 500007, 0.25),
(500018, 500008, 1),(500018, 500009, -6),(500018, 500010, -7),(500018, 500011, -0.75),(500018, 500012, 0),(500018, 500013, 1.5),
(500018, 500014, 0),(500018, 500015, -1),(500018, 500016, 0),(500018, 500017, 1.5),(500018, 500019, -4),(500018, 500020, 0),
(500019, 500001, -7),(500019, 500002, 0),(500019, 500003, -7),(500019, 500004, 0),(500019, 500005, 0),(500019, 500006, -3),
(500019, 500007, -5),(500019, 500008, -5),(500019, 500009, -1),(500019, 500010, 3),(500019, 500011, -2),(500019, 500012, 5),
(500019, 500013, 0),(500019, 500014, 0),(500019, 500015, -3),(500019, 500016, 0),(500019, 500017, -1.75),(500019, 500018, -5),
(500019, 500020, -2),(500020, 500001, 0),(500020, 500002, -6),(500020, 500003, 0),(500020, 500004, -9),(500020, 500005, 0),
(500020, 500006, -1),(500020, 500007, -1),(500020, 500008, -2),(500020, 500009, 5),(500020, 500010, -1),(500020, 500011, 8),
(500020, 500012, -4),(500020, 500013, 0),(500020, 500014, -7),(500020, 500015, 0),(500020, 500016, -3),(500020, 500017, -1.25),
(500020, 500018, 0),(500020, 500019, -2);


INSERT INTO `roidDistribution` (`AI`, `systemSec`, `roidID`, `roidName`, `percent`) VALUES
(1, 'A', 1230, 'Veldspar', 0.65),(2, 'A', 1228, 'Scordite', 0.35),
(3, 'B', 1230, 'Veldspar', 0.55),(4, 'B', 1228, 'Scordite', 0.25),(5, 'B', 1224, 'Pyroxeres', 0.2),
(6, 'B1', 1230, 'Veldspar', 0.4),(7, 'B1', 1228, 'Scordite', 0.3),(8, 'B1', 1224, 'Pyroxeres', 0.2),(9, 'B1', 20, 'Kernite', 0.1),
(10, 'B2', 1230, 'Veldspar', 0.3),(11, 'B2', 1228, 'Scordite', 0.3),(12, 'B2', 1224, 'Pyroxeres', 0.2),(13, 'B2', 20, 'Kernite', 0.1),
(14, 'B2', 1226, 'Jaspet', 0.1),(15, 'B3', 1230, 'Veldspar', 0.2),(16, 'B3', 1228, 'Scordite', 0.2),(17, 'B3', 1224, 'Pyroxeres', 0.2),
(18, 'B3', 20, 'Kernite', 0.2),(19, 'B3', 1226, 'Jaspet', 0.1),(20, 'B3', 1231, 'Hemorphite', 0.1),(21, 'C', 1230, 'Veldspar', 0.4),
(22, 'C', 1228, 'Scordite', 0.4),(23, 'C', 18, 'Plagioclase', 0.1),(24, 'C', 1224, 'Pyroxeres', 0.1),(25, 'C1', 1230, 'Veldspar', 0.3),
(26, 'C1', 1228, 'Scordite', 0.2),(27, 'C1', 18, 'Plagioclase', 0.2),(28, 'C1', 1224, 'Pyroxeres', 0.2),(29, 'C1', 20, 'Kernite', 0.1),
(30, 'C2', 1230, 'Veldspar', 0.2),(31, 'C2', 1228, 'Scordite', 0.2),(32, 'C2', 18, 'Plagioclase', 0.1),(33, 'C2', 1224, 'Pyroxeres', 0.1),
(34, 'C2', 20, 'Kernite', 0.2),(35, 'C2', 21, 'Hedbergite', 0.2),(36, 'D', 1230, 'Veldspar', 0.4),(37, 'D', 1228, 'Scordite', 0.4),
(38, 'D', 18, 'Plagioclase', 0.2),(39, 'D1', 1230, 'Veldspar', 0.4),(40, 'D1', 1228, 'Scordite', 0.2),(41, 'D1', 18, 'Plagioclase', 0.2),
(42, 'D1', 1227, 'Omber', 0.2),(43, 'D2', 1230, 'Veldspar', 0.3),(44, 'D2', 1228, 'Scordite', 0.2),(45, 'D2', 18, 'Plagioclase', 0.2),
(46, 'D2', 1227, 'Omber', 0.2),(47, 'D2', 1226, 'Jaspet', 0.1),(48, 'D3', 1230, 'Veldspar', 0.2),(49, 'D3', 1228, 'Scordite', 0.2),
(50, 'D3', 18, 'Plagioclase', 0.2),(51, 'D3', 1227, 'Omber', 0.2),(52, 'D3', 1226, 'Jaspet', 0.1),(53, 'D3', 1231, 'Hemorphite', 0.1),
(54, 'E', 1230, 'Veldspar', 0.3),(55, 'E', 1228, 'Scordite', 0.3),(56, 'E', 18, 'Plagioclase', 0.2),(57, 'E', 1227, 'Omber', 0.1),
(58, 'E', 20, 'Kernite', 0.1),(59, 'E1', 1230, 'Veldspar', 0.3),(60, 'E1', 1228, 'Scordite', 0.2),(61, 'E1', 18, 'Plagioclase', 0.2),
(62, 'E1', 1227, 'Omber', 0.1),(63, 'E1', 20, 'Kernite', 0.1),(64, 'E1', 21, 'Hedbergite', 0.1),(65, 'F', 1230, 'Veldspar', 0.3),
(66, 'F', 1228, 'Scordite', 0.3),(67, 'F', 21, 'Hedbergite', 0.2),(68, 'F', 1231, 'Hemorphite', 0.1),(69, 'F', 1227, 'Omber', 0.1),
(70, 'F1', 1230, 'Veldspar', 0.3),(71, 'F1', 1228, 'Scordite', 0.2),(72, 'F1', 21, 'Hedbergite', 0.2),(73, 'F1', 1231, 'Hemorphite', 0.2),
(74, 'F1', 1227, 'Omber', 0.1),(75, 'F1', 19, 'Spodumain', 0.05),(76, 'F2', 1230, 'Veldspar', 0.3),(77, 'F2', 1228, 'Scordite', 0.2),
(78, 'F2', 21, 'Hedbergite', 0.1),(79, 'F2', 1231, 'Hemorphite', 0.1),(80, 'F2', 1227, 'Omber', 0.15),(81, 'F2', 19, 'Spodumain', 0.05),
(82, 'F2', 1229, 'Gneiss', 0.1),(83, 'F3', 1230, 'Veldspar', 0.2),(84, 'F3', 1228, 'Scordite', 0.2),(85, 'F3', 21, 'Hedbergite', 0.1),
(86, 'F3', 1231, 'Hemorphite', 0.1),(87, 'F3', 1227, 'Omber', 0.15),(88, 'F3', 19, 'Spodumain', 0.05),(89, 'F3', 1229, 'Gneiss', 0.15),
(90, 'F3', 1223, 'Bistot', 0.1),(91, 'F4', 1230, 'Veldspar', 0.2),(92, 'F4', 1228, 'Scordite', 0.2),(93, 'F4', 21, 'Hedbergite', 0.1),
(94, 'F4', 1231, 'Hemorphite', 0.1),(95, 'F4', 1227, 'Omber', 0.1),(96, 'F4', 19, 'Spodumain', 0.05),(97, 'F4', 1229, 'Gneiss', 0.15),
(98, 'F4', 1223, 'Bistot', 0.1),(99, 'F4', 22, 'Arkonor', 0.1),(100, 'F5', 1230, 'Veldspar', 0.25),(101, 'F5', 1228, 'Scordite', 0.15),
(102, 'F5', 21, 'Hedbergite', 0.1),(103, 'F5', 1231, 'Hemorphite', 0.1),(104, 'F5', 1227, 'Omber', 0.15),(105, 'F5', 19, 'Spodumain', 0.05),
(106, 'F5', 1229, 'Gneiss', 0.1),(107, 'F5', 1223, 'Bistot', 0.1),(108, 'F5', 22, 'Arkonor', 0.1),(109, 'F5', 1224, 'Pyroxeres', 0.1),
(110, 'F6', 1230, 'Veldspar', 0.15),(111, 'F6', 1228, 'Scordite', 0.1),(112, 'F6', 21, 'Hedbergite', 0.1),(113, 'F6', 1231, 'Hemorphite', 0.1),
(114, 'F6', 1227, 'Omber', 0.1),(115, 'F6', 19, 'Spodumain', 0.05),(116, 'F6', 1229, 'Gneiss', 0.1),(117, 'F6', 1223, 'Bistot', 0.1),(118, 'F6', 22, 'Arkonor', 0.1),
(119, 'F6', 1224, 'Pyroxeres', 0.1),(120, 'F7', 1230, 'Veldspar', 0.1),(121, 'F7', 1228, 'Scordite', 0.1),(122, 'F7', 21, 'Hedbergite', 0.1),
(123, 'F7', 1231, 'Hemorphite', 0.1),(124, 'F7', 1227, 'Omber', 0.1),(125, 'F7', 19, 'Spodumain', 0.05),(126, 'F7', 1229, 'Gneiss', 0.15),
(127, 'F7', 1223, 'Bistot', 0.1),(128, 'F7', 22, 'Arkonor', 0.1),(129, 'F7', 1224, 'Pyroxeres', 0.1),(130, 'F7', 18, 'Plagioclase', 0.1),
(131, 'G', 1230, 'Veldspar', 0.3),(132, 'G', 1228, 'Scordite', 0.2),(133, 'G', 18, 'Plagioclase', 0.2),(134, 'G', 1227, 'Omber', 0.1),
(135, 'G', 20, 'Kernite', 0.1),(136, 'G1', 1230, 'Veldspar', 0.25),(137, 'G1', 1228, 'Scordite', 0.2),(138, 'G1', 18, 'Plagioclase', 0.15),
(139, 'G1', 1227, 'Omber', 0.15),(140, 'G1', 20, 'Kernite', 0.15),(141, 'G1', 1229, 'Gneiss', 0.1),(142, 'G2', 1230, 'Veldspar', 0.3),
(143, 'G2', 1228, 'Scordite', 0.2),(144, 'G2', 18, 'Plagioclase', 0.1),(145, 'G2', 1227, 'Omber', 0.1),(146, 'G2', 20, 'Kernite', 0.1),
(147, 'G2', 1229, 'Gneiss', 0.1),(148, 'G2', 1224, 'Pyroxeres', 0.1),(149, 'G3', 1230, 'Veldspar', 0.25),(150, 'G3', 1228, 'Scordite', 0.15),
(151, 'G3', 18, 'Plagioclase', 0.1),(152, 'G3', 1227, 'Omber', 0.15),(153, 'G3', 20, 'Kernite', 0.15),(154, 'G3', 1229, 'Gneiss', 0.1),
(155, 'G3', 1224, 'Pyroxeres', 0.1),(156, 'G3', 19, 'Spodumain', 0.05),(157, 'G4', 1230, 'Veldspar', 0.1),(158, 'G4', 1228, 'Scordite', 0.1),
(159, 'G4', 18, 'Plagioclase', 0.1),(160, 'G4', 1227, 'Omber', 0.1),(161, 'G4', 20, 'Kernite', 0.1),(162, 'G4', 1229, 'Gneiss', 0.1),
(163, 'G4', 1224, 'Pyroxeres', 0.1),(164, 'G4', 19, 'Spodumain', 0.05),(165, 'G4', 1223, 'Bistot', 0.1),(166, 'G5', 1230, 'Veldspar', 0.1),
(167, 'G5', 1228, 'Scordite', 0.1),(168, 'G5', 18, 'Plagioclase', 0.1),(169, 'G5', 1227, 'Omber', 0.1),(170, 'G5', 20, 'Kernite', 0.1),
(171, 'G5', 1229, 'Gneiss', 0.1),(172, 'G5', 1224, 'Pyroxeres', 0.1),(173, 'G5', 19, 'Spodumain', 0.05),(174, 'G5', 1223, 'Bistot', 0.1),
(175, 'G5', 1225, 'Crokite', 0.1),(176, 'G6', 1230, 'Veldspar', 0.1),(177, 'G6', 1228, 'Scordite', 0.1),(178, 'G6', 18, 'Plagioclase', 0.1),
(179, 'G6', 1227, 'Omber', 0.1),(180, 'G6', 20, 'Kernite', 0.1),(181, 'G6', 1229, 'Gneiss', 0.1),(182, 'G6', 1224, 'Pyroxeres', 0.1),
(183, 'G6', 19, 'Spodumain', 0.05),(184, 'G6', 1223, 'Bistot', 0.1),(185, 'G6', 1225, 'Crokite', 0.15),(186, 'G7', 1230, 'Veldspar', 0.1),
(187, 'G7', 1228, 'Scordite', 0.1),(188, 'G7', 18, 'Plagioclase', 0.1),(189, 'G7', 1227, 'Omber', 0.1),(190, 'G7', 20, 'Kernite', 0.1),
(191, 'G7', 1229, 'Gneiss', 0.1),(192, 'G7', 1224, 'Pyroxeres', 0.1),(193, 'G7', 19, 'Spodumain', 0.05),(194, 'G7', 1223, 'Bistot', 0.1),
(195, 'G7', 1225, 'Crokite', 0.2),(196, 'G7', 1232, 'Dark Ochre', 0.2),(197, 'H', 1230, 'Veldspar', 0.2),(198, 'H', 1228, 'Scordite', 0.2),
(199, 'H', 1224, 'Pyroxeres', 0.2),(200, 'H', 1231, 'Hemorphite', 0.2),(201, 'H', 1226, 'Jaspet', 0.2),(202, 'H1', 1230, 'Veldspar', 0.2),
(203, 'H1', 1228, 'Scordite', 0.2),(204, 'H1', 1224, 'Pyroxeres', 0.2),(205, 'H1', 1231, 'Hemorphite', 0.1),(206, 'H1', 1226, 'Jaspet', 0.2),
(207, 'H1', 21, 'Hedbergite', 0.1),(208, 'H2', 1230, 'Veldspar', 0.2),(209, 'H2', 1228, 'Scordite', 0.2),(210, 'H2', 1224, 'Pyroxeres', 0.2),
(211, 'H2', 1231, 'Hemorphite', 0.1),(212, 'H2', 1226, 'Jaspet', 0.1),(213, 'H2', 21, 'Hedbergite', 0.1),(214, 'H2', 1232, 'Dark Ochre', 0.1),
(215, 'H3', 1230, 'Veldspar', 0.1),(216, 'H3', 1228, 'Scordite', 0.1),(217, 'H3', 1224, 'Pyroxeres', 0.1),(218, 'H3', 1231, 'Hemorphite', 0.1),
(219, 'H3', 1226, 'Jaspet', 0.2),(220, 'H3', 21, 'Hedbergite', 0.1),(221, 'H3', 1232, 'Dark Ochre', 0.1),(222, 'H3', 20, 'Kernite', 0.2),
(223, 'H4', 1230, 'Veldspar', 0.1),(224, 'H4', 1228, 'Scordite', 0.1),(225, 'H4', 1224, 'Pyroxeres', 0.1),(226, 'H4', 1231, 'Hemorphite', 0.1),
(227, 'H4', 1226, 'Jaspet', 0.1),(228, 'H4', 21, 'Hedbergite', 0.1),(229, 'H4', 1232, 'Dark Ochre', 0.1),(230, 'H4', 20, 'Kernite', 0.1),
(231, 'H4', 1225, 'Crokite', 0.2),(232, 'H5', 1230, 'Veldspar', 0.1),(233, 'H5', 1228, 'Scordite', 0.1),(234, 'H5', 1224, 'Pyroxeres', 0.1),
(235, 'H5', 1231, 'Hemorphite', 0.1),(236, 'H5', 1226, 'Jaspet', 0.1),(237, 'H5', 21, 'Hedbergite', 0.1),(238, 'H5', 1232, 'Dark Ochre', 0.15),
(239, 'H5', 20, 'Kernite', 0.1),(240, 'H5', 1225, 'Crokite', 0.1),(241, 'H5', 19, 'Spodumain', 0.05),(242, 'H6', 1230, 'Veldspar', 0.1),(243, 'H6', 1228, 'Scordite', 0.1),
(244, 'H6', 1224, 'Pyroxeres', 0.1),(245, 'H6', 1231, 'Hemorphite', 0.1),(246, 'H6', 1226, 'Jaspet', 0.1),(247, 'H6', 21, 'Hedbergite', 0.1),
(248, 'H6', 1232, 'Dark Ochre', 0.1),(249, 'H6', 20, 'Kernite', 0.1),(250, 'H6', 1225, 'Crokite', 0.15),(251, 'H6', 19, 'Spodumain', 0.05),
(252, 'H7', 1230, 'Veldspar', 0.1),(253, 'H7', 1228, 'Scordite', 0.1),(254, 'H7', 1224, 'Pyroxeres', 0.1),(255, 'H7', 1231, 'Hemorphite', 0.1),
(256, 'H7', 1226, 'Jaspet', 0.1),(257, 'H7', 21, 'Hedbergite', 0.1),(258, 'H7', 1232, 'Dark Ochre', 0.1),(259, 'H7', 20, 'Kernite', 0.1),
(260, 'H7', 1225, 'Crokite', 0.1),(261, 'H7', 19, 'Spodumain', 0.05),(262, 'H7', 1223, 'Bistot', 0.05),(263, 'I', 1230, 'Veldspar', 0.3),
(264, 'I', 1228, 'Scordite', 0.2),(265, 'I', 21, 'Hedbergite', 0.2),(266, 'I', 1231, 'Hemorphite', 0.2),(267, 'I', 1227, 'Omber', 0.1),
(268, 'I1', 1230, 'Veldspar', 0.2),(269, 'I1', 1228, 'Scordite', 0.2),(270, 'I1', 21, 'Hedbergite', 0.2),(271, 'I1', 1231, 'Hemorphite', 0.2),
(272, 'I1', 1227, 'Omber', 0.1),(273, 'I1', 1226, 'Jaspet', 0.1),(274, 'I2', 1230, 'Veldspar', 0.2),(275, 'I2', 1228, 'Scordite', 0.2),
(276, 'I2', 21, 'Hedbergite', 0.15),(277, 'I2', 1231, 'Hemorphite', 0.15),(278, 'I2', 1227, 'Omber', 0.1),(279, 'I2', 1226, 'Jaspet', 0.1),
(280, 'I2', 19, 'Spodumain', 0.05),(281, 'I3', 1230, 'Veldspar', 0.1),(282, 'I3', 1228, 'Scordite', 0.05),(283, 'I3', 21, 'Hedbergite', 0.1),
(284, 'I3', 1231, 'Hemorphite', 0.1),(285, 'I3', 1227, 'Omber', 0.1),(286, 'I3', 1226, 'Jaspet', 0.1),(287, 'I3', 19, 'Spodumain', 0.05),
(288, 'I3', 1229, 'Gneiss', 0.1),(289, 'I4', 1230, 'Veldspar', 0.1),(290, 'I4', 1228, 'Scordite', 0.1),(291, 'I4', 21, 'Hedbergite', 0.1),
(292, 'I4', 1231, 'Hemorphite', 0.1),(293, 'I4', 1227, 'Omber', 0.1),(294, 'I4', 1226, 'Jaspet', 0.1),(295, 'I4', 19, 'Spodumain', 0.05),
(296, 'I4', 1229, 'Gneiss', 0.1),(297, 'I4', 1232, 'Dark Ochre', 0.1),(298, 'I5', 1230, 'Veldspar', 0.1),(299, 'I5', 1228, 'Scordite', 0.1),
(300, 'I5', 21, 'Hedbergite', 0.1),(301, 'I5', 1231, 'Hemorphite', 0.1),(302, 'I5', 1227, 'Omber', 0.1),(303, 'I5', 1226, 'Jaspet', 0.1),
(304, 'I5', 19, 'Spodumain', 0.05),(305, 'I5', 1229, 'Gneiss', 0.1),(306, 'I5', 1232, 'Dark Ochre', 0.1),(307, 'I5', 22, 'Arkonor', 0.1),
(308, 'I6', 1230, 'Veldspar', 0.1),(309, 'I6', 1228, 'Scordite', 0.1),(310, 'I6', 21, 'Hedbergite', 0.1),(311, 'I6', 1231, 'Hemorphite', 0.1),
(312, 'I6', 1227, 'Omber', 0.1),(313, 'I6', 1226, 'Jaspet', 0.1),(314, 'I6', 19, 'Spodumain', 0.05),(315, 'I6', 1229, 'Gneiss', 0.1),
(316, 'I6', 1232, 'Dark Ochre', 0.1),(317, 'I6', 22, 'Arkonor', 0.1),(318, 'I7', 1230, 'Veldspar', 0.1),(319, 'I7', 1228, 'Scordite', 0.1),
(320, 'I7', 21, 'Hedbergite', 0.1),(321, 'I7', 1231, 'Hemorphite', 0.1),(322, 'I7', 1227, 'Omber', 0.1),(323, 'I7', 1226, 'Jaspet', 0.1),
(324, 'I7', 19, 'Spodumain', 0.05),(325, 'I7', 1229, 'Gneiss', 0.1),(326, 'I7', 1232, 'Dark Ochre', 0.1),(327, 'I7', 22, 'Arkonor', 0.1),
(328, 'I7', 20, 'Kernite', 0.1),(329, 'J', 1230, 'Veldspar', 0.2),(330, 'J', 1228, 'Scordite', 0.2),(331, 'J', 1224, 'Pyroxeres', 0.2),
(332, 'J', 18, 'Plagioclase', 0.2),(333, 'J', 1226, 'Jaspet', 0.2),(334, 'J1', 1230, 'Veldspar', 0.2),(335, 'J1', 1228, 'Scordite', 0.2),
(336, 'J1', 1224, 'Pyroxeres', 0.2),(337, 'J1', 18, 'Plagioclase', 0.2),(338, 'J1', 1226, 'Jaspet', 0.2),(339, 'J1', 1232, 'Dark Ochre', 0.1),(340, 'J2', 1230, 'Veldspar', 0.2),
(341, 'J2', 1228, 'Scordite', 0.1),(342, 'J2', 1224, 'Pyroxeres', 0.1),(343, 'J2', 18, 'Plagioclase', 0.1),(344, 'J2', 1226, 'Jaspet', 0.1),
(345, 'J2', 1232, 'Dark Ochre', 0.1),(346, 'J2', 1225, 'Crokite', 0.1),(347, 'J3', 1230, 'Veldspar', 0.1),(348, 'J3', 1228, 'Scordite', 0.2),
(349, 'J3', 1224, 'Pyroxeres', 0.1),(350, 'J3', 18, 'Plagioclase', 0.1),(351, 'J3', 1226, 'Jaspet', 0.1),(352, 'J3', 1232, 'Dark Ochre', 0.1),
(353, 'J3', 1225, 'Crokite', 0.1),(354, 'J3', 1223, 'Bistot', 0.1),(355, 'J4', 1230, 'Veldspar', 0.1),(356, 'J4', 1228, 'Scordite', 0.1),
(357, 'J4', 1224, 'Pyroxeres', 0.1),(358, 'J4', 18, 'Plagioclase', 0.1),(359, 'J4', 1226, 'Jaspet', 0.1),(360, 'J4', 1232, 'Dark Ochre', 0.1),
(361, 'J4', 1225, 'Crokite', 0.1),(362, 'J4', 1223, 'Bistot', 0.1),(363, 'J4', 1231, 'Hemorphite', 0.1),(364, 'J5', 1230, 'Veldspar', 0.1),
(365, 'J5', 1228, 'Scordite', 0.1),(366, 'J5', 1224, 'Pyroxeres', 0.1),(367, 'J5', 18, 'Plagioclase', 0.1),(368, 'J5', 1226, 'Jaspet', 0.1),
(369, 'J5', 1232, 'Dark Ochre', 0.1),(370, 'J5', 1225, 'Crokite', 0.1),(371, 'J5', 1223, 'Bistot', 0.1),(372, 'J5', 1231, 'Hemorphite', 0.1),(373, 'J5', 21, 'Hedbergite', 0.1),
(374, 'J6', 1230, 'Veldspar', 0.1),(375, 'J6', 1228, 'Scordite', 0.1),(376, 'J6', 1224, 'Pyroxeres', 0.1),(377, 'J6', 18, 'Plagioclase', 0.1),
(378, 'J6', 1226, 'Jaspet', 0.1),(379, 'J6', 1232, 'Dark Ochre', 0.1),(380, 'J6', 1225, 'Crokite', 0.1),(381, 'J6', 1223, 'Bistot', 0.1),
(382, 'J6', 1231, 'Hemorphite', 0.1),(383, 'J6', 21, 'Hedbergite', 0.1),(384, 'J7', 1230, 'Veldspar', 0.1),(385, 'J7', 1228, 'Scordite', 0.1),
(386, 'J7', 1224, 'Pyroxeres', 0.1),(387, 'J7', 18, 'Plagioclase', 0.1),(388, 'J7', 1226, 'Jaspet', 0.1),(389, 'J7', 1232, 'Dark Ochre', 0.1),
(390, 'J7', 1225, 'Crokite', 0.1),(391, 'J7', 1223, 'Bistot', 0.1),(392, 'J7', 1231, 'Hemorphite', 0.1),(393, 'J7', 21, 'Hedbergite', 0.1),
(394, 'J7', 22, 'Arkonor', 0.1),(395, 'K', 1230, 'Veldspar', 0.2),(396, 'K', 1228, 'Scordite', 0.2),(397, 'K', 21, 'Hedbergite', 0.2),
(398, 'K', 1231, 'Hemorphite', 0.2),(399, 'K', 1227, 'Omber', 0.2),(400, 'K1', 1230, 'Veldspar', 0.2),(401, 'K1', 1228, 'Scordite', 0.2),
(402, 'K1', 21, 'Hedbergite', 0.2),(403, 'K1', 1231, 'Hemorphite', 0.2),(404, 'K1', 1227, 'Omber', 0.1),(405, 'K1', 1232, 'Dark Ochre', 0.1),
(406, 'K2', 1230, 'Veldspar', 0.1),(407, 'K2', 1228, 'Scordite', 0.1),(408, 'K2', 21, 'Hedbergite', 0.1),(409, 'K2', 1231, 'Hemorphite', 0.1),
(410, 'K2', 1227, 'Omber', 0.2),(411, 'K2', 1232, 'Dark Ochre', 0.2),(412, 'K2', 19, 'Spodumain', 0.05),(413, 'K3', 1230, 'Veldspar', 0.15),
(414, 'K3', 1228, 'Scordite', 0.15),(415, 'K3', 21, 'Hedbergite', 0.1),(416, 'K3', 1231, 'Hemorphite', 0.1),(417, 'K3', 1227, 'Omber', 0.15),
(418, 'K3', 1232, 'Dark Ochre', 0.15),(419, 'K3', 19, 'Spodumain', 0.05),(420, 'K3', 1225, 'Crokite', 0.15),(421, 'K4', 1230, 'Veldspar', 0.1),
(422, 'K4', 1228, 'Scordite', 0.1),(423, 'K4', 21, 'Hedbergite', 0.1),(424, 'K4', 1231, 'Hemorphite', 0.1),(425, 'K4', 1227, 'Omber', 0.1),
(426, 'K4', 1232, 'Dark Ochre', 0.2),(427, 'K4', 19, 'Spodumain', 0.05),(428, 'K4', 1225, 'Crokite', 0.1),(429, 'K4', 1223, 'Bistot', 0.15),
(430, 'K5', 1230, 'Veldspar', 0.1),(431, 'K5', 1228, 'Scordite', 0.1),(432, 'K5', 21, 'Hedbergite', 0.1),(433, 'K5', 1231, 'Hemorphite', 0.1),
(434, 'K5', 1227, 'Omber', 0.1),(435, 'K5', 1232, 'Dark Ochre', 0.1),(436, 'K5', 19, 'Spodumain', 0.05),(437, 'K5', 1225, 'Crokite', 0.15),
(438, 'K5', 1223, 'Bistot', 0.1),(439, 'K5', 22, 'Arkonor', 0.1),(440, 'K6', 1230, 'Veldspar', 0.1),(441, 'K6', 1228, 'Scordite', 0.1),
(442, 'K6', 21, 'Hedbergite', 0.1),(443, 'K6', 1231, 'Hemorphite', 0.1),(444, 'K6', 1227, 'Omber', 0.1),(445, 'K6', 1232, 'Dark Ochre', 0.1),
(446, 'K6', 19, 'Spodumain', 0.05),(447, 'K6', 1225, 'Crokite', 0.1),(448, 'K6', 1223, 'Bistot', 0.15),(449, 'K6', 22, 'Arkonor', 0.1),
(450, 'K7', 1230, 'Veldspar', 0.1),(451, 'K7', 1228, 'Scordite', 0.05),(452, 'K7', 21, 'Hedbergite', 0.1),(453, 'K7', 1231, 'Hemorphite', 0.1),
(454, 'K7', 1227, 'Omber', 0.1),(455, 'K7', 1232, 'Dark Ochre', 0.1),(456, 'K7', 19, 'Spodumain', 0.05),(457, 'K7', 1225, 'Crokite', 0.1),
(458, 'K7', 1223, 'Bistot', 0.1),(459, 'K7', 22, 'Arkonor', 0.1),(460, 'K7', 1229, 'Gneiss', 0.1);

UPDATE `crtCategories` SET `categoryNameID` = 16, `dataID` = 22250862 WHERE `categoryID` = 3;
UPDATE `crtCategories` SET `categoryNameID` = 15, `dataID` = 16559510 WHERE `categoryID` = 4;
UPDATE `crtCategories` SET `categoryNameID` = 7, `dataID` = 34288261 WHERE `categoryID` = 5;
UPDATE `crtCategories` SET `categoryNameID` = 8, `dataID` = 16559503 WHERE `categoryID` = 6;
UPDATE `crtCategories` SET `categoryNameID` = 9, `dataID` = 16559504 WHERE `categoryID` = 7;
UPDATE `crtCategories` SET `categoryNameID` = 10, `dataID` = 16559505 WHERE `categoryID` = 8;
UPDATE `crtCategories` SET `categoryNameID` = 11, `dataID` = 16559506 WHERE `categoryID` = 9;
UPDATE `crtCategories` SET `categoryNameID` = 12, `dataID` = 16559507 WHERE `categoryID` = 10;
UPDATE `crtCategories` SET `categoryNameID` = 17, `dataID` = 22250878 WHERE `categoryID` = 11;
UPDATE `crtCategories` SET `categoryNameID` = 13, `dataID` = 16559508 WHERE `categoryID` = 12;
UPDATE `crtCategories` SET `categoryNameID` = 14, `dataID` = 16559509 WHERE `categoryID` = 13;
UPDATE `crtCategories` SET `categoryNameID` = 19, `dataID` = 16559511 WHERE `categoryID` = 18;


UPDATE `mapRegions` SET `ratFactionID` = 500011 WHERE `regionID` = 10000001;
UPDATE `mapRegions` SET `ratFactionID` = 500010 WHERE `regionID` = 10000002;
UPDATE `mapRegions` SET `ratFactionID` = 500010 WHERE `regionID` = 10000003;
UPDATE `mapRegions` SET `ratFactionID` = 500011 WHERE `regionID` = 10000005;
UPDATE `mapRegions` SET `ratFactionID` = 500011 WHERE `regionID` = 10000006;
UPDATE `mapRegions` SET `ratFactionID` = 500011 WHERE `regionID` = 10000007;
UPDATE `mapRegions` SET `ratFactionID` = 500011 WHERE `regionID` = 10000008;
UPDATE `mapRegions` SET `ratFactionID` = 500011 WHERE `regionID` = 10000009;
UPDATE `mapRegions` SET `ratFactionID` = 500010 WHERE `regionID` = 10000010;
UPDATE `mapRegions` SET `ratFactionID` = 500011 WHERE `regionID` = 10000011;
UPDATE `mapRegions` SET `ratFactionID` = 500011 WHERE `regionID` = 10000012;
UPDATE `mapRegions` SET `ratFactionID` = 500021 WHERE `regionID` = 10000013;
UPDATE `mapRegions` SET `ratFactionID` = 500019 WHERE `regionID` = 10000014;
UPDATE `mapRegions` SET `ratFactionID` = 500010 WHERE `regionID` = 10000015;
UPDATE `mapRegions` SET `ratFactionID` = 500010 WHERE `regionID` = 10000016;
UPDATE `mapRegions` SET `ratFactionID` = 500020 WHERE `regionID` = 10000017;
UPDATE `mapRegions` SET `ratFactionID` = 500021 WHERE `regionID` = 10000018;
UPDATE `mapRegions` SET `ratFactionID` = 500021 WHERE `regionID` = 10000019;
UPDATE `mapRegions` SET `ratFactionID` = 500019 WHERE `regionID` = 10000020;
UPDATE `mapRegions` SET `ratFactionID` = 500021 WHERE `regionID` = 10000021;
UPDATE `mapRegions` SET `ratFactionID` = 500019 WHERE `regionID` = 10000022;
UPDATE `mapRegions` SET `ratFactionID` = 500010 WHERE `regionID` = 10000023;
UPDATE `mapRegions` SET `ratFactionID` = 500011 WHERE `regionID` = 10000025;
UPDATE `mapRegions` SET `ratFactionID` = 500021 WHERE `regionID` = 10000027;
UPDATE `mapRegions` SET `ratFactionID` = 500011 WHERE `regionID` = 10000028;
UPDATE `mapRegions` SET `ratFactionID` = 500010 WHERE `regionID` = 10000029;
UPDATE `mapRegions` SET `ratFactionID` = 500011 WHERE `regionID` = 10000030;
UPDATE `mapRegions` SET `ratFactionID` = 500011 WHERE `regionID` = 10000031;
UPDATE `mapRegions` SET `ratFactionID` = 500020 WHERE `regionID` = 10000032;
UPDATE `mapRegions` SET `ratFactionID` = 500010 WHERE `regionID` = 10000033;
UPDATE `mapRegions` SET `ratFactionID` = 500021 WHERE `regionID` = 10000034;
UPDATE `mapRegions` SET `ratFactionID` = 500010 WHERE `regionID` = 10000035;
UPDATE `mapRegions` SET `ratFactionID` = 500019 WHERE `regionID` = 10000036;
UPDATE `mapRegions` SET `ratFactionID` = 500020 WHERE `regionID` = 10000037;
UPDATE `mapRegions` SET `ratFactionID` = 500019 WHERE `regionID` = 10000038;
UPDATE `mapRegions` SET `ratFactionID` = 500019 WHERE `regionID` = 10000039;
UPDATE `mapRegions` SET `ratFactionID` = 500021 WHERE `regionID` = 10000040;
UPDATE `mapRegions` SET `ratFactionID` = 500020 WHERE `regionID` = 10000041;
UPDATE `mapRegions` SET `ratFactionID` = 500010 WHERE `regionID` = 10000042;
UPDATE `mapRegions` SET `ratFactionID` = 500019 WHERE `regionID` = 10000043;
UPDATE `mapRegions` SET `ratFactionID` = 500020 WHERE `regionID` = 10000044;
UPDATE `mapRegions` SET `ratFactionID` = 500010 WHERE `regionID` = 10000045;
UPDATE `mapRegions` SET `ratFactionID` = 500020 WHERE `regionID` = 10000046;
UPDATE `mapRegions` SET `ratFactionID` = 500019 WHERE `regionID` = 10000047;
UPDATE `mapRegions` SET `ratFactionID` = 500020 WHERE `regionID` = 10000048;
UPDATE `mapRegions` SET `ratFactionID` = 500012 WHERE `regionID` = 10000049;
UPDATE `mapRegions` SET `ratFactionID` = 500012 WHERE `regionID` = 10000050;
UPDATE `mapRegions` SET `ratFactionID` = 500020 WHERE `regionID` = 10000051;
UPDATE `mapRegions` SET `ratFactionID` = 500012 WHERE `regionID` = 10000052;
UPDATE `mapRegions` SET `ratFactionID` = 500021 WHERE `regionID` = 10000053;
UPDATE `mapRegions` SET `ratFactionID` = 500020 WHERE `regionID` = 10000054;
UPDATE `mapRegions` SET `ratFactionID` = 500010 WHERE `regionID` = 10000055;
UPDATE `mapRegions` SET `ratFactionID` = 500011 WHERE `regionID` = 10000056;
UPDATE `mapRegions` SET `ratFactionID` = 500020 WHERE `regionID` = 10000057;
UPDATE `mapRegions` SET `ratFactionID` = 500020 WHERE `regionID` = 10000058;
UPDATE `mapRegions` SET `ratFactionID` = 500019 WHERE `regionID` = 10000059;
UPDATE `mapRegions` SET `ratFactionID` = 500012 WHERE `regionID` = 10000060;
UPDATE `mapRegions` SET `ratFactionID` = 500011 WHERE `regionID` = 10000061;
UPDATE `mapRegions` SET `ratFactionID` = 500011 WHERE `regionID` = 10000062;
UPDATE `mapRegions` SET `ratFactionID` = 500012 WHERE `regionID` = 10000063;
UPDATE `mapRegions` SET `ratFactionID` = 500020 WHERE `regionID` = 10000064;
UPDATE `mapRegions` SET `ratFactionID` = 500012 WHERE `regionID` = 10000065;
UPDATE `mapRegions` SET `ratFactionID` = 500021 WHERE `regionID` = 10000066;
UPDATE `mapRegions` SET `ratFactionID` = 500020 WHERE `regionID` = 10000067;
UPDATE `mapRegions` SET `ratFactionID` = 500020 WHERE `regionID` = 10000068;
UPDATE `mapRegions` SET `ratFactionID` = 500010 WHERE `regionID` = 10000069;

-- dummy corp for rogue drones
INSERT INTO `corporation` (`corporationID`, `corporationName`, `description`, `tickerName`, `url`, `taxRate`, `minimumJoinStanding`, `corporationType`, `hasPlayerPersonnelManager`, `sendCharTerminationMessage`, `creatorID`, `ceoID`, `stationID`, `raceID`, `allianceID`, `allianceMemberStartDate`, `shares`, `memberCount`, `memberLimit`, `allowedMemberRaceIDs`, `graphicID`, `shape1`, `shape2`, `shape3`, `color1`, `color2`, `color3`, `typeface`, `division1`, `division2`, `division3`, `division4`, `division5`, `division6`, `division7`, `walletDivision1`, `walletDivision2`, `walletDivision3`, `walletDivision4`, `walletDivision5`, `walletDivision6`, `walletDivision7`, `balance`, `deleted`, `isRecruiting`, `warFactionID`) VALUES ('1000001', 'Rogue Drone', 'Corporation Placeholder for Rogue Drones', 'RD', '', '0', '0', '0', '0', '1', '0', '0', '0', NULL, '0', '', '1000', '0', '10', '0', '0', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, '0', '0', '0', '0');
