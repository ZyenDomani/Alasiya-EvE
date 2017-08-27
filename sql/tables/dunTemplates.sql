-- phpMyAdmin SQL Dump
-- version 4.4.15.5
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Aug 27, 2017 at 06:09 PM
-- Server version: 10.0.24-MariaDB
-- PHP Version: 5.6.30

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `alasiya-new`
--

-- --------------------------------------------------------

--
-- Table structure for table `dunTemplates`
--

CREATE TABLE IF NOT EXISTS `dunTemplates` (
  `dunTemplateID` int(11) NOT NULL,
  `dunTemplateName` varchar(85) COLLATE utf8_bin NOT NULL,
  `dunEntryID` int(11) NOT NULL DEFAULT '0',
  `dunTypeID` int(11) NOT NULL DEFAULT '0',
  `dunSpawnType` int(11) NOT NULL DEFAULT '0',
  `dunRoomID` int(11) NOT NULL DEFAULT '0',
  `dunRooms` int(11) NOT NULL DEFAULT '0',
  `dunRoomTypeID` int(11) NOT NULL DEFAULT '0',
  `dunRoomCategoryID` int(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

--
-- Dumping data for table `dunTemplates`
--

INSERT INTO `dunTemplates` (`dunTemplateID`, `dunTemplateName`, `dunEntryID`, `dunTypeID`, `dunSpawnType`, `dunRoomID`, `dunRooms`, `dunRoomTypeID`, `dunRoomCategoryID`) VALUES
(1001, 'test dungeon', 0, 8, 0, 1001, 0, 0, 0),
(1002, 'test data center', 0, 8, 0, 1002, 0, 0, 0),
(1003, 'test dungeon 2', 0, 8, 0, 1003, 0, 0, 0),
(11110, 'Mission Space', 0, 1, 0, 1, 0, 0, 0),
(21010, 'Small Omber Deposit', 0, 2, 0, 2, 0, 0, 0),
(21110, 'Small Kernite and Omber Deposit', 0, 2, 0, 3, 0, 0, 0),
(21210, 'Small Jaspet, Kernite and Omber Deposit', 0, 2, 0, 4, 0, 0, 0),
(21310, 'Small Hemorphite, Jaspet and Kernite Deposit', 0, 2, 0, 5, 0, 0, 0),
(21410, 'Small Hedbergite, Hemorphite and Jaspet Deposit', 0, 2, 0, 6, 0, 0, 0),
(21510, 'Small Hedbergite and Hemorphite Deposit', 0, 2, 0, 7, 0, 0, 0),
(21020, 'Average Omber Deposit', 0, 2, 0, 16, 0, 0, 0),
(21120, 'Average Kernite and Omber Deposit', 0, 2, 0, 17, 0, 0, 0),
(21220, 'Average Jaspet, Kernite and Omber Deposit', 0, 2, 0, 18, 0, 0, 0),
(21320, 'Average Hemorphite, Jaspet and Kernite Deposit', 0, 2, 0, 19, 0, 0, 0),
(21420, 'Average Hedbergite, Hemorphite and Jaspet Deposit', 0, 2, 0, 20, 0, 0, 0),
(21520, 'Average Hedbergite and Hemorphite Deposit', 0, 2, 0, 21, 0, 0, 0),
(21030, 'Large Omber Deposit', 0, 2, 0, 30, 0, 0, 0),
(21130, 'Large Kernite and Omber Deposit', 0, 2, 0, 31, 0, 0, 0),
(21230, 'Large Jaspet, Kernite and Omber Deposit', 0, 2, 0, 32, 0, 0, 0),
(21330, 'Large Hemorphite, Jaspet and Kernite Deposit', 0, 2, 0, 33, 0, 0, 0),
(21430, 'Large Hedbergite, Hemorphite and Jaspet Deposit', 0, 2, 0, 34, 0, 0, 0),
(21530, 'Large Hedbergite and Hemorphite Deposit', 0, 2, 0, 35, 0, 0, 0),
(22010, 'Small Gneiss Deposit', 0, 2, 0, 8, 0, 0, 0),
(22110, 'Small Dark Ochre and Gneiss Deposit', 0, 2, 0, 9, 0, 0, 0),
(22210, 'Small Crokite, Dark Ochre and Gneiss Deposit', 0, 2, 0, 10, 0, 0, 0),
(22310, 'Small Spodumain, Crokite and Dark Ochre Deposit', 0, 2, 0, 11, 0, 0, 0),
(22020, 'Average Gneiss Deposit', 0, 2, 0, 22, 0, 0, 0),
(22120, 'Average Dark Ochre and Gneiss Deposit', 0, 2, 0, 23, 0, 0, 0),
(22220, 'Average Crokite, Dark Ochre and Gneiss Deposit', 0, 2, 0, 24, 0, 0, 0),
(22320, 'Average Spodumain, Crokite and Dark Ochre Deposit', 0, 2, 0, 25, 0, 0, 0),
(22030, 'Large Gneiss Deposit', 0, 2, 0, 36, 0, 0, 0),
(22130, 'Large Dark Ochre and Gneiss Deposit', 0, 2, 0, 37, 0, 0, 0),
(22230, 'Large Crokite, Dark Ochre and Gneiss Deposit', 0, 2, 0, 38, 0, 0, 0),
(22330, 'Large Spodumain, Crokite and Dark Ochre Deposit', 0, 2, 0, 39, 0, 0, 0),
(23010, 'Small Bistot Deposit', 0, 2, 0, 12, 0, 0, 0),
(23110, 'Small Arkanor and Bistot Deposit', 0, 2, 0, 13, 0, 0, 0),
(23210, 'Small Mercoxit, Arkonor and Bistot Deposit', 0, 2, 0, 14, 0, 0, 0),
(23020, 'Average Bistot Deposit', 0, 2, 0, 26, 0, 0, 0),
(23120, 'Average Arkanor and Bistot Deposit', 0, 2, 0, 27, 0, 0, 0),
(23220, 'Average Mercoxit, Arkonor and Bistot Deposit', 0, 2, 0, 28, 0, 0, 0),
(23030, 'Large Bistot Deposit', 0, 2, 0, 40, 0, 0, 0),
(23130, 'Large Arkanor and Bistot Deposit', 0, 2, 0, 41, 0, 0, 0),
(23230, 'Large Mercoxit, Arkonor and Bistot Deposit', 0, 2, 0, 42, 0, 0, 0),
(23510, 'Small Asteroid Cluster', 0, 2, 0, 1, 0, 0, 0),
(23520, 'Moderate Asteroid Cluster', 0, 2, 0, 15, 0, 0, 0),
(23530, 'Large Asteroid Cluster', 0, 2, 0, 29, 0, 0, 0),
(23540, 'Enormous Asteroid Cluster', 0, 2, 0, 29, 0, 0, 0),
(23550, 'Colossal Asteroid Cluster', 0, 2, 0, 29, 0, 0, 0),
(31110, 'Crumbling (faction) Antiquated Outpost', 0, 3, 0, 1, 0, 0, 0),
(31210, 'Crumbling (faction) Crystal Quarry', 0, 3, 0, 1, 0, 0, 0),
(31310, 'Crumbling (faction) Explosive Debris', 0, 3, 0, 1, 0, 0, 0),
(31410, 'Crumbling (faction) Abandoned Colony', 0, 3, 0, 1, 0, 0, 0),
(31510, 'Crumbling (faction) Excavation', 0, 3, 0, 1, 0, 0, 0),
(31610, 'Crumbling (faction) Solar Harvesters', 0, 3, 0, 1, 0, 0, 0),
(31710, 'Crumbling (faction) Stone Formation', 0, 3, 0, 1, 0, 0, 0),
(31810, 'Crumbling (faction) Mining Installation', 0, 3, 0, 1, 0, 0, 0),
(31120, 'Looted (faction) Collision Site', 0, 3, 0, 1, 0, 0, 0),
(31220, 'Looted (faction) Abandoned Station', 0, 3, 0, 1, 0, 0, 0),
(31320, 'Looted (faction) Lone Vessel', 0, 3, 0, 1, 0, 0, 0),
(31420, 'Looted (faction) Ruined Station', 0, 3, 0, 1, 0, 0, 0),
(31520, 'Looted (faction) Explosive Debris', 0, 3, 0, 1, 0, 0, 0),
(31620, 'Looted (faction) Battle Remnants', 0, 3, 0, 1, 0, 0, 0),
(31720, 'Looted (faction) Pod Cluster', 0, 3, 0, 1, 0, 0, 0),
(31820, 'Looted (faction) Ship Graveyard', 0, 3, 0, 1, 0, 0, 0),
(32110, 'Decayed (faction) Excavation', 0, 3, 0, 1, 0, 0, 0),
(32210, 'Decayed (faction) Collision Site', 0, 3, 0, 1, 0, 0, 0),
(32310, 'Decayed (faction) Lone Vessel', 0, 3, 0, 1, 0, 0, 0),
(32410, 'Decayed (faction) Mining Installation', 0, 3, 0, 1, 0, 0, 0),
(32510, 'Decayed (faction) Particle Accelerator', 0, 3, 0, 1, 0, 0, 0),
(32610, 'Decayed (faction) Mass Grave', 0, 3, 0, 1, 0, 0, 0),
(32710, 'Decayed (faction) Rock Formations', 0, 3, 0, 1, 0, 0, 0),
(32810, 'Decayed (faction) Quarry', 0, 3, 0, 1, 0, 0, 0),
(32120, 'Ransacked (faction) Explosive Debris', 0, 3, 0, 1, 0, 0, 0),
(32220, 'Ransacked (faction) Abandoned Station', 0, 3, 0, 1, 0, 0, 0),
(32320, 'Ransacked (faction) Collision Site', 0, 3, 0, 1, 0, 0, 0),
(32420, 'Ransacked (faction) Ruined Station', 0, 3, 0, 1, 0, 0, 0),
(32520, 'Ransacked (faction) Ship Remnants', 0, 3, 0, 1, 0, 0, 0),
(32620, 'Ransacked (faction) Dumped Cargo', 0, 3, 0, 1, 0, 0, 0),
(32720, 'Ransacked (faction) Demolished Station', 0, 3, 0, 1, 0, 0, 0),
(32820, 'Ransacked (faction) Ship Graveyard', 0, 3, 0, 1, 0, 0, 0),
(33110, 'Pristine (Faction) Ship Remnants', 0, 3, 0, 1, 0, 0, 0),
(33210, 'Pristine (Faction) Pod Cluster', 0, 3, 0, 1, 0, 0, 0),
(33310, 'Pristine (Faction) Dumped Cargo', 0, 3, 0, 1, 0, 0, 0),
(33410, 'Pristine (Faction) Ship Graveyard', 0, 3, 0, 1, 0, 0, 0),
(33510, 'Pristine (Faction) Battle Remnants', 0, 3, 0, 1, 0, 0, 0),
(33610, 'Pristine (Faction) Abandoned Colony', 0, 3, 0, 1, 0, 0, 0),
(33710, 'Pristine (Faction) Collision Site', 0, 3, 0, 1, 0, 0, 0),
(33810, 'Pristine (Faction) Explosive Debris', 0, 3, 0, 1, 0, 0, 0),
(33120, 'Pristine (Faction) Collision Site', 0, 3, 0, 1, 0, 0, 0),
(33220, 'Pristine (Faction) Explosive Debris', 0, 3, 0, 1, 0, 0, 0),
(33320, 'Ruined (Faction) Monument Site', 0, 3, 0, 1, 0, 0, 0),
(33420, 'Ruined (Faction) Temple Site', 0, 3, 0, 1, 0, 0, 0),
(33520, 'Ruined (Faction) Science Outpost', 0, 3, 0, 1, 0, 0, 0),
(33620, 'Ruined (Faction) Crystal Quarry', 0, 3, 0, 1, 0, 0, 0),
(33130, 'Bloated Ruins', 0, 3, 0, 1, 0, 0, 0),
(33230, 'Whispy Ruins', 0, 3, 0, 1, 0, 0, 0),
(33330, 'Forgotten Ruins', 0, 3, 0, 1, 0, 0, 0),
(33430, 'Crumbling Ruins', 0, 3, 0, 1, 0, 0, 0),
(33530, 'Ancient Ruins', 0, 3, 0, 1, 0, 0, 0),
(33630, 'Festering Ruins', 0, 3, 0, 1, 0, 0, 0),
(33730, 'Hidden Ruins', 0, 3, 0, 1, 0, 0, 0),
(41110, 'Local (faction) Mainframe', 0, 4, 0, 1, 0, 0, 0),
(41210, 'Local (faction) Data Processing Center', 0, 4, 0, 1, 0, 0, 0),
(41310, 'Local (faction) Data Terminal', 0, 4, 0, 1, 0, 0, 0),
(41410, 'Local (faction) Backup Server', 0, 4, 0, 1, 0, 0, 0),
(41510, 'Local (faction) Virus Test Site', 0, 4, 0, 1, 0, 0, 0),
(41610, 'Local (faction) Shattered Life-Support Unit', 0, 4, 0, 1, 0, 0, 0),
(41710, 'Local (faction) Production Installation', 0, 4, 0, 1, 0, 0, 0),
(41810, 'Local (faction) Minor Shipyard', 0, 4, 0, 1, 0, 0, 0),
(42110, 'Regional (faction) Data Fortress', 0, 4, 0, 1, 0, 0, 0),
(42210, 'Regional (faction) Command Center', 0, 4, 0, 1, 0, 0, 0),
(42310, 'Regional (faction) Data Mining Site', 0, 4, 0, 1, 0, 0, 0),
(42410, 'Regional (faction) Backup Server', 0, 4, 0, 1, 0, 0, 0),
(42510, 'Regional (faction) Mainframe', 0, 4, 0, 1, 0, 0, 0),
(42610, 'Regional (faction) Data Processing Center', 0, 4, 0, 1, 0, 0, 0),
(42710, 'Regional (faction) Data Terminal', 0, 4, 0, 1, 0, 0, 0),
(42810, 'Regional (faction) Secure Server', 0, 4, 0, 1, 0, 0, 0),
(43110, 'Central (Faction) Sparking Transmitter', 0, 4, 0, 1, 0, 0, 0),
(43210, 'Central (Faction) Survey Site', 0, 4, 0, 1, 0, 0, 0),
(43310, 'Central (Faction) Command Center', 0, 4, 0, 1, 0, 0, 0),
(43410, 'Central (Faction) Data Mining Site', 0, 4, 0, 1, 0, 0, 0),
(43510, 'Central (faction) Mainframe', 0, 4, 0, 1, 0, 0, 0),
(43610, 'Central (faction) Data Processing Center', 0, 4, 0, 1, 0, 0, 0),
(43710, 'Central (faction) Data Terminal', 0, 4, 0, 1, 0, 0, 0),
(43810, 'Central (faction) Secure Server', 0, 4, 0, 1, 0, 0, 0),
(43120, 'Digital Network', 0, 4, 0, 1, 0, 0, 0),
(43220, 'Digital Matrix', 0, 4, 0, 1, 0, 0, 0),
(43320, 'Digital Complex', 0, 4, 0, 1, 0, 0, 0),
(43420, 'Digital Convolution', 0, 4, 0, 1, 0, 0, 0),
(43520, 'Digital Plexus', 0, 4, 0, 1, 0, 0, 0),
(43620, 'Digital Circuitry', 0, 4, 0, 1, 0, 0, 0),
(43720, 'Digital Compound', 0, 4, 0, 1, 0, 0, 0),
(43820, 'Digital Tessellation', 0, 4, 0, 1, 0, 0, 0),
(51110, 'Wild Nebula', 0, 5, 0, 1, 0, 0, 0),
(51210, 'Pipe Nebula', 0, 5, 0, 1, 0, 0, 0),
(51310, 'Calabash Nebula', 0, 5, 0, 1, 0, 0, 0),
(51410, 'Blackeye Nebula', 0, 5, 0, 1, 0, 0, 0),
(51510, 'Smoking Nebula', 0, 5, 0, 1, 0, 0, 0),
(51610, 'Glass Nebula', 0, 5, 0, 1, 0, 0, 0),
(51710, 'Flame Nebula', 0, 5, 0, 1, 0, 0, 0),
(51810, 'Ghost Nebula ', 0, 5, 0, 1, 0, 0, 0),
(52110, 'Sister Nebula', 0, 5, 0, 1, 0, 0, 0),
(52210, 'Bright Nebula', 0, 5, 0, 1, 0, 0, 0),
(52310, 'Crimson Nebula', 0, 5, 0, 1, 0, 0, 0),
(52410, 'Phoenix Nebula', 0, 5, 0, 1, 0, 0, 0),
(52510, 'Eagle Nebula', 0, 5, 0, 1, 0, 0, 0),
(52610, 'Rapture Nebula', 0, 5, 0, 1, 0, 0, 0),
(52710, 'Ring Nebula', 0, 5, 0, 1, 0, 0, 0),
(52810, 'Sparkling Nebula', 0, 5, 0, 1, 0, 0, 0),
(53110, 'Forgotten Nebula', 0, 5, 0, 1, 0, 0, 0),
(53210, 'Diabolo Nebula', 0, 5, 0, 1, 0, 0, 0),
(53310, 'Saintly Nebula', 0, 5, 0, 1, 0, 0, 0),
(53410, 'Sunspark Nebula', 0, 5, 0, 1, 0, 0, 0),
(53510, 'Emerald Nebula', 0, 5, 0, 1, 0, 0, 0),
(53610, 'Swarm Nebula', 0, 5, 0, 1, 0, 0, 0),
(53710, 'Boisterous Nebula', 0, 5, 0, 1, 0, 0, 0),
(53810, 'Thick Nebula', 0, 5, 0, 1, 0, 0, 0),
(71110, 'Hideaway', 0, 7, 0, 1, 0, 0, 0),
(71120, 'Hidden Hideaway', 0, 7, 0, 1, 0, 0, 0),
(71130, 'Forsaken Hideaway', 0, 7, 0, 1, 0, 0, 0),
(71140, 'Forlorn Hideaway', 0, 7, 0, 1, 0, 0, 0),
(71210, 'Burrow', 0, 7, 0, 1, 0, 0, 0),
(71310, 'Refuge', 0, 7, 0, 1, 0, 0, 0),
(71410, 'Den', 0, 7, 0, 1, 0, 0, 0),
(72110, 'Refuge', 0, 7, 0, 1, 0, 0, 0),
(72210, 'Den', 0, 7, 0, 1, 0, 0, 0),
(72220, 'Hidden Den', 0, 7, 0, 1, 0, 0, 0),
(72230, 'Forsaken Den', 0, 7, 0, 1, 0, 0, 0),
(72240, 'Forlorn Den', 0, 7, 0, 1, 0, 0, 0),
(72310, 'Yard', 0, 7, 0, 1, 0, 0, 0),
(72410, 'Rally Point', 0, 7, 0, 1, 0, 0, 0),
(72420, 'Hidden Rally Point', 0, 7, 0, 1, 0, 0, 0),
(72430, 'Forsaken Rally Point', 0, 7, 0, 1, 0, 0, 0),
(72440, 'Forlorn Rally Point', 0, 7, 0, 1, 0, 0, 0),
(72510, 'Port', 0, 7, 0, 1, 0, 0, 0),
(73110, 'Rally Point', 0, 7, 0, 1, 0, 0, 0),
(73120, 'Hidden Rally Point', 0, 7, 0, 1, 0, 0, 0),
(73130, 'Forsaken Rally Point', 0, 7, 0, 1, 0, 0, 0),
(73140, 'Forlorn Rally Point', 0, 7, 0, 1, 0, 0, 0),
(73210, 'Port', 0, 7, 0, 1, 0, 0, 0),
(73310, 'Hub', 0, 7, 0, 1, 0, 0, 0),
(73320, 'Hidden Hub', 0, 7, 0, 1, 0, 0, 0),
(73330, 'Forsaken Hub', 0, 7, 0, 1, 0, 0, 0),
(73340, 'Forlorn Hub', 0, 7, 0, 1, 0, 0, 0),
(73410, 'Haven', 0, 7, 0, 1, 0, 0, 0),
(73510, 'Sanctum', 0, 7, 0, 1, 0, 0, 0),
(81110, 'Hideout', 0, 8, 0, 1, 0, 0, 0),
(81210, 'Lookout', 0, 8, 0, 1, 0, 0, 0),
(81310, 'Watch', 0, 8, 0, 1, 0, 0, 0),
(81410, 'Vigil', 0, 8, 0, 1, 0, 0, 0),
(81510, 'Outpost', 0, 8, 0, 1, 0, 0, 0),
(82110, 'Vigil', 0, 8, 0, 1, 0, 0, 0),
(82210, 'Outpost', 0, 8, 0, 1, 0, 0, 0),
(82310, 'Annex', 0, 8, 0, 1, 0, 0, 0),
(82410, 'Base', 0, 8, 0, 1, 0, 0, 0),
(82510, 'Fortress', 0, 8, 0, 1, 0, 0, 0),
(83110, 'Base', 0, 8, 0, 1, 0, 0, 0),
(83210, 'Fortress', 0, 8, 0, 1, 0, 0, 0),
(83310, 'Complex', 0, 8, 0, 1, 0, 0, 0),
(83410, 'Headquarters', 0, 8, 0, 1, 0, 0, 0),
(83510, 'Staging Point', 0, 8, 0, 1, 0, 0, 0);

--
-- Indexes for dumped tables
--

--
-- Indexes for table `dunTemplates`
--
ALTER TABLE `dunTemplates`
  ADD KEY `dunTemplateID` (`dunTemplateID`);

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
