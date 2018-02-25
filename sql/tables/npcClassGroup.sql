-- phpMyAdmin SQL Dump
-- version 4.4.15.10
-- https://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Feb 17, 2018 at 01:02 PM
-- Server version: 10.0.33-MariaDB
-- PHP Version: 5.6.33

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `EVE_Crucible`
--

-- --------------------------------------------------------

--
-- Table structure for table `npcClassGroup`
--

CREATE TABLE IF NOT EXISTS `npcClassGroup` (
  `shipClass` int(11) NOT NULL,
  `groupID` int(11) NOT NULL,
  `factionID` int(11) NOT NULL,
  `groupName` varchar(50) COLLATE utf8_unicode_ci NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Dumping data for table `npcClassGroup`
--

INSERT INTO `npcClassGroup` (`shipClass`, `groupID`, `factionID`, `groupName`) VALUES
(1, 550, 500011, 'Asteroid Angel Cartel Frigate'),
(2, 550, 500011, 'Asteroid Angel Cartel Advanced Frigate'),
(3, 575, 500011, 'Asteroid Angel Cartel Destroyer'),
(4, 551, 500011, 'Asteroid Angel Cartel Cruiser'),
(5, 551, 500011, 'Asteroid Angel Cartel Advanced Cruiser'),
(6, 576, 500011, 'Asteroid Angel Cartel BattleCruiser'),
(7, 552, 500011, 'Asteroid Angel Cartel Battleship'),
(8, 554, 500011, 'Asteroid Angel Cartel Hauler'),
(9, 553, 500011, 'Asteroid Angel Cartel Officer'),
(10, 789, 500011, 'Asteroid Angel Cartel Commander Frigate'),
(11, 794, 500011, 'Asteroid Angel Cartel Commander Destroyer'),
(12, 790, 500011, 'Asteroid Angel Cartel Commander Cruiser'),
(13, 793, 500011, 'Asteroid Angel Cartel Commander BattleCruiser'),
(14, 848, 500011, 'Asteroid Angel Cartel Commander Battleship'),
(1, 557, 500012, 'Asteroid Blood Raiders Frigate'),
(2, 557, 500012, 'Asteroid Blood Raiders Advanced Frigate'),
(3, 577, 500012, 'Asteroid Blood Raiders Destroyer'),
(4, 555, 500012, 'Asteroid Blood Raiders Cruiser'),
(5, 555, 500012, 'Asteroid Blood Raiders Advanced Cruiser'),
(6, 578, 500012, 'Asteroid Blood Raiders BattleCruiser'),
(7, 556, 500012, 'Asteroid Blood Raiders Battleship'),
(8, 558, 500012, 'Asteroid Blood Raiders Hauler'),
(9, 559, 500012, 'Asteroid Blood Raiders Officer'),
(10, 792, 500012, 'Asteroid Blood Raiders Commander Frigate'),
(11, 796, 500012, 'Asteroid Blood Raiders Commander Destroyer'),
(12, 791, 500012, 'Asteroid Blood Raiders Commander Cruiser'),
(13, 795, 500012, 'Asteroid Blood Raiders Commander BattleCruiser'),
(14, 849, 500012, 'Asteroid Blood Raiders Commander Battleship'),
(1, 562, 500010, 'Asteroid Guristas Frigate'),
(2, 562, 500010, 'Asteroid Guristas Advanced Frigate'),
(3, 579, 500010, 'Asteroid Guristas Destroyer'),
(4, 561, 500010, 'Asteroid Guristas Cruiser'),
(5, 561, 500010, 'Asteroid Guristas Advanced Cruiser'),
(6, 580, 500010, 'Asteroid Guristas BattleCruiser'),
(7, 560, 500010, 'Asteroid Guristas Battleship'),
(8, 563, 500010, 'Asteroid Guristas Hauler'),
(9, 564, 500010, 'Asteroid Guristas Officer'),
(10, 800, 500010, 'Asteroid Guristas Commander Frigate'),
(11, 799, 500010, 'Asteroid Guristas Commander Destroyer'),
(12, 798, 500010, 'Asteroid Guristas Commander Cruiser'),
(13, 797, 500010, 'Asteroid Guristas Commander BattleCruiser'),
(14, 850, 500010, 'Asteroid Guristas Commander Battleship'),
(1, 567, 500019, 'Asteroid Sansha''s Nation Frigate'),
(2, 567, 500019, 'Asteroid Sansha''s Nation Advanced Frigate'),
(3, 581, 500019, 'Asteroid Sansha''s Nation Destroyer'),
(4, 566, 500019, 'Asteroid Sansha''s Nation Cruiser'),
(5, 566, 500019, 'Asteroid Sansha''s Nation Advanced Cruiser'),
(6, 582, 500019, 'Asteroid Sansha''s Nation BattleCruiser'),
(7, 565, 500019, 'Asteroid Sansha''s Nation Battleship'),
(8, 568, 500019, 'Asteroid Sansha''s Nation Hauler'),
(9, 569, 500019, 'Asteroid Sansha''s Nation Officer'),
(10, 810, 500019, 'Asteroid Sansha''s Nation Commander Frigate'),
(11, 809, 500019, 'Asteroid Sansha''s Nation Commander Destroyer'),
(12, 808, 500019, 'Asteroid Sansha''s Nation Commander Cruiser'),
(13, 807, 500019, 'Asteroid Sansha''s Nation Commander BattleCruiser'),
(14, 851, 500019, 'Asteroid Sansha''s Nation Commander Battleship'),
(1, 572, 500020, 'Asteroid Serpentis Frigate'),
(2, 572, 500020, 'Asteroid Serpentis Advanced Frigate'),
(3, 583, 500020, 'Asteroid Serpentis Destroyer'),
(4, 571, 500020, 'Asteroid Serpentis Cruiser'),
(5, 571, 500020, 'Asteroid Serpentis Advanced Cruiser'),
(6, 584, 500020, 'Asteroid Serpentis BattleCruiser'),
(7, 570, 500020, 'Asteroid Serpentis Battleship'),
(8, 573, 500020, 'Asteroid Serpentis Hauler'),
(9, 574, 500020, 'Asteroid Serpentis Officer'),
(10, 814, 500020, 'Asteroid Serpentis Commander Frigate'),
(11, 813, 500020, 'Asteroid Serpentis Commander Destroyer'),
(12, 812, 500020, 'Asteroid Serpentis Commander Cruiser'),
(13, 811, 500020, 'Asteroid Serpentis Commander BattleCruiser'),
(14, 852, 500020, 'Asteroid Serpentis Commander Battleship'),
(1, 759, 500022, 'Asteroid Rogue Drone Frigate'),
(2, 759, 500022, 'Asteroid Rogue Drone Advanced Frigate'),
(3, 758, 500022, 'Asteroid Rogue Drone Destroyer'),
(4, 757, 500022, 'Asteroid Rogue Drone Cruiser'),
(5, 757, 500022, 'Asteroid Rogue Drone Advanced Cruiser'),
(6, 755, 500022, 'Asteroid Rogue Drone BattleCruiser'),
(7, 756, 500022, 'Asteroid Rogue Drone Battleship'),
(8, 760, 500022, 'Asteroid Rogue Drone Hauler'),
(9, 761, 500022, 'Asteroid Rogue Drone Swarm'),
(10, 847, 500022, 'Asteroid Rogue Drone Commander Frigate'),
(11, 846, 500022, 'Asteroid Rogue Drone Commander Destroyer'),
(12, 845, 500022, 'Asteroid Rogue Drone Commander Cruiser'),
(13, 843, 500022, 'Asteroid Rogue Drone Commander BattleCruiser'),
(14, 844, 500022, 'Asteroid Rogue Drone Commander Battleship');

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
