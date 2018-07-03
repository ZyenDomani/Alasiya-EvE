-- phpMyAdmin SQL Dump
-- version 4.4.15.10
-- https://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Jul 03, 2018 at 04:49 AM
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
-- Table structure for table `qstCourier`
--

CREATE TABLE IF NOT EXISTS `qstCourier` (
  `id` int(5) NOT NULL DEFAULT '0',
  `descID` int(5) NOT NULL DEFAULT '0',
  `name` text,
  `level` tinyint(1) NOT NULL DEFAULT '0',
  `typeID` tinyint(1) NOT NULL DEFAULT '0',
  `important` tinyint(1) NOT NULL DEFAULT '0',
  `storyline` tinyint(1) NOT NULL DEFAULT '0',
  `raceID` tinyint(2) NOT NULL DEFAULT '0',
  `itemTypeID` int(6) NOT NULL DEFAULT '0',
  `itemQty` int(10) NOT NULL DEFAULT '0',
  `rewardISK` int(10) NOT NULL DEFAULT '0',
  `rewardItemID` int(11) NOT NULL DEFAULT '0',
  `rewardItemQty` int(11) NOT NULL DEFAULT '0',
  `bonusISK` int(11) NOT NULL DEFAULT '0',
  `bonusTime` int(10) NOT NULL DEFAULT '0',
  `collateral` int(7) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Dumping data for table `qstCourier`
--

INSERT INTO `qstCourier` (`id`, `descID`, `name`, `level`, `typeID`, `important`, `storyline`, `raceID`, `itemTypeID`, `itemQty`, `rewardISK`, `rewardItemID`, `rewardItemQty`, `bonusISK`, `bonusTime`, `collateral`) VALUES
(54833, 0, 'Need A Lift', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(55005, 130400, 'Transaction Data Delivery', 1, 3, 0, 1, 0, 0, 2, 0, 0, 1, 27000, 28, 100000),
(55006, 130404, 'Transaction Data Delivery', 2, 3, 0, 1, 0, 0, 5, 0, 0, 1, 56000, 21, 200000),
(55007, 130408, 'Transaction Data Delivery', 2, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(55008, 130412, 'Transaction Data Delivery', 3, 3, 0, 1, 0, 0, 0, 4000000, 0, 0, 1000000, 60, 2500000),
(55009, 130416, 'Transaction Data Delivery', 3, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(55010, 130420, 'Transaction Data Delivery', 4, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(55011, 130424, 'Transaction Data Delivery', 4, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(55012, 130428, 'Transaction Data Delivery', 5, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(55184, 0, 'A Special Breed', 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(55186, 0, 'A Humble Gift', 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(55187, 0, 'Miracle Drug', 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(55188, 0, 'Bio-Engineering Pays', 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(55189, 0, 'Army Recruits', 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(55190, 130966, 'Supplies For The Needy', 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(55191, 0, 'Breakdown', 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(55192, 0, 'Party Goods', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(55193, 0, 'Cool Cat', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(55194, 0, 'Save The Children', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(55205, 130997, 'Broken Reactor', 1, 3, 0, 0, 0, 0, 75, 0, 0, 0, 0, 0, 0),
(55206, 0, 'Homeless People Everywhere', 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(55207, 131000, 'High Command', 1, 3, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0),
(55208, 0, 'Dirt', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(55320, 131498, 'Move The Goods', 1, 3, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0),
(55321, 131502, 'Move The Goods', 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(55322, 131504, 'Move The Goods', 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(55325, 131525, 'A Special Delivery', 1, 3, 0, 1, 0, 0, 5, 0, 0, 0, 0, 0, 0),
(55326, 131529, 'A Special Delivery', 2, 3, 1, 1, 0, 0, 10, 0, 0, 0, 0, 0, 0),
(55327, 131534, 'A Special Delivery', 3, 3, 0, 1, 0, 0, 20, 0, 0, 0, 0, 0, 0),
(55328, 131539, 'A Special Delivery', 4, 3, 0, 1, 0, 0, 30, 0, 0, 0, 0, 0, 0),
(55329, 131544, 'A Special Delivery', 5, 3, 0, 1, 0, 0, 40, 0, 0, 0, 0, 0, 0),
(55472, 132243, 'A Special Delivery', 1, 3, 1, 1, 0, 0, 20, 0, 0, 0, 0, 0, 0),
(55473, 132247, 'A Special Delivery', 2, 3, 1, 1, 0, 0, 40, 0, 0, 0, 0, 0, 0),
(56901, 0, 'Slave Shipment', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(56903, 0, 'Send in the Marines', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(56911, 0, 'Hunting a Heretic - Book Burning (2 of 3)', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(56912, 0, 'Hunting a Heretic - Crackpot Captured! (3 of 3)', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57114, 0, 'Medical Delivery', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57117, 145160, 'Wee Bug Problem', 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57824, 143755, 'Beefing Up', 1, 3, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0),
(57825, 143443, 'Future Leaders', 1, 3, 0, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0),
(57826, 0, 'Giving Shelter', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57827, 143459, 'Hunger Strikes', 1, 3, 0, 0, 0, 0, 25, 0, 0, 0, 0, 0, 0),
(57828, 0, 'Incriminating Evidence', 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57829, 143450, 'Marriage', 2, 3, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0),
(57830, 143452, 'Rat Problem', 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57831, 143449, 'Stolen Arms', 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57832, 0, 'Under Construction', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57833, 0, 'Air', 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57834, 0, 'Arms Dealer', 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57835, 0, 'Cigarettes', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57836, 143447, 'Ridiculous Oil Prices', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57837, 143446, 'Shipping Problems', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57838, 0, 'Tightening Security', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57839, 145156, 'Tourists', 1, 3, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0),
(57840, 0, 'A Bit of Terraforming', 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57841, 0, 'A Feast Fit for a King', 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57842, 0, 'A Feast Fit for a King', 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57843, 0, 'A Total Mess', 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57844, 143756, 'Beefing Up', 2, 3, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0),
(57847, 0, 'Breakdown', 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57850, 0, 'Equipped For The Job', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57851, 0, 'Fertile Ground', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57852, 0, 'From Bad To Worse', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57853, 143444, 'Future Leaders', 2, 3, 0, 0, 0, 0, 25, 0, 0, 0, 0, 0, 0),
(57854, 0, 'Garbage Man', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57855, 0, 'Giving Shelter', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57856, 0, 'Good Harvest', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57858, 130987, 'Human Problem', 1, 3, 0, 0, 0, 0, 75, 0, 0, 0, 0, 0, 0),
(57859, 144413, 'Hunger Strikes', 2, 3, 0, 0, 0, 0, 50, 0, 0, 0, 0, 0, 0),
(57860, 0, 'Important Launch', 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57862, 0, 'Incriminating Evidence', 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57863, 0, 'Keeping The Grunts Happy', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57864, 143454, 'Marriage', 2, 3, 0, 0, 0, 0, 40, 0, 0, 0, 0, 0, 0),
(57865, 131013, 'Miner Problems', 1, 3, 0, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0),
(57866, 131506, 'Move The Goods', 4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57867, 131012, 'My Little Girl', 1, 3, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0),
(57868, 131014, 'Our Boys Are Getting Bored', 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57869, 131030, 'Our Boys Are Getting Bored', 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57870, 143455, 'Rat Problem', 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57871, 0, 'Silly Miners', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57872, 145178, 'Stolen Arms', 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57873, 0, 'Stress Reliever', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57874, 131010, 'Take This Away', 1, 3, 0, 0, 0, 0, 25, 0, 0, 0, 0, 0, 0),
(57877, 130981, 'Troublemakers', 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57878, 0, 'Under Construction', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57880, 0, 'Equipped For The Job', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57881, 0, 'Fertile Ground', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57882, 0, 'Good Harvest', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57883, 130990, 'Human Problem', 2, 3, 0, 0, 0, 0, 150, 0, 0, 0, 0, 0, 0),
(57884, 0, 'Silly Miners', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57885, 0, 'Stress Reliever', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57886, 130982, 'Troublemakers', 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57887, 131027, 'Miner Problems', 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57888, 131026, 'My Little Girl', 2, 3, 0, 0, 0, 0, 40, 0, 0, 0, 0, 0, 0),
(57889, 131025, 'Take This Away', 2, 3, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0),
(57890, 0, 'A Bit of Terraforming', 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57891, 0, 'A Feast Fit for a King', 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57892, 0, 'A Total Mess', 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57894, 0, 'Important Launch', 4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57896, 0, 'Keeping The Grunts Happy', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57897, 131047, 'Our Boys Are Getting Bored', 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57899, 0, 'A Feast Fit for a King', 4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57901, 0, 'Breakdown', 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57904, 0, 'From Bad To Worse', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57905, 0, 'Garbage Man', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57907, 131048, 'Our Boys Are Getting Bored', 4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57910, 0, 'A Humble Gift', 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57911, 0, 'A Special Breed', 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57912, 0, 'Army Recruits', 4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57913, 0, 'Bio-Engineering Pays', 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57914, 0, 'Breakdown', 4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57915, 0, 'Cool Cat', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57916, 0, 'Party Goods', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57917, 0, 'Save The Children', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57918, 130967, 'Supplies For The Needy', 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57919, 131016, 'Broken Reactor', 2, 3, 0, 0, 0, 0, 150, 0, 0, 0, 0, 0, 0),
(57920, 0, 'Dirt', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57921, 131020, 'High Command', 2, 3, 0, 0, 0, 0, 200, 0, 0, 0, 0, 0, 0),
(57922, 0, 'Homeless People Everywhere', 4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57985, 0, 'Air', 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57986, 0, 'Arms Dealer', 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57987, 0, 'Cigarettes', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57990, 0, 'Miracle Drug', 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57991, 143453, 'Ridiculous Oil Prices', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57992, 0, 'Salve for Sis', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57993, 143448, 'Shipping Problems', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57995, 0, 'Tightening Security', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(57996, 145157, 'Tourists', 2, 3, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0),
(57997, 0, 'Corporate Documents', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(58057, 0, 'A Piece of History', 3, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(58061, 0, 'Their Secret Defense', 0, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(58062, 143840, 'A Cargo With Attitude', 3, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(58079, 0, 'Ditanium', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(58083, 0, 'Hunting Black Dog', 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(58084, 0, 'Operation Doorstop', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(58090, 0, 'Their Secret Defense', 0, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(58091, 0, 'The State of the Empire', 0, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(58107, 144234, 'A Cargo With Attitude', 4, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(58114, 0, 'A Piece of History', 4, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(58118, 0, 'Hunting Black Dog', 4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(58120, 0, 'Operation Doorstop', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(58123, 0, 'The State of the Empire', 0, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(58350, 0, 'Ditanium', 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
(58354, 144459, 'Materials For War Preparation', 1, 3, 0, 1, 0, 0, 999, 14000, 0, 1, 14000, 45, 0),
(58355, 144461, 'Materials For War Preparation', 2, 3, 0, 1, 0, 0, 1665, 24000, 0, 1, 24000, 23, 0),
(58356, 144550, 'Materials For War Preparation', 3, 3, 0, 1, 0, 0, 4500, 40000, 0, 1, 40000, 18, 0),
(58357, 144553, 'Materials For War Preparation', 4, 3, 0, 1, 0, 0, 8000, 60000, 0, 1, 60000, 16, 0),
(58358, 144556, 'Materials For War Preparation', 5, 3, 0, 1, 0, 0, 12000, 95000, 0, 1, 95000, 12, 0),
(58089, 144084, 'The Governor’s Ball', 2, 3, 0, 0, 4, 0, 7, 40000, 0, 0, 44000, 23, 100000),
(58124, 144381, 'The Governor’s Ball', 3, 3, 0, 0, 4, 0, 12, 65000, 0, 0, 65000, 18, 150000);

--
-- Indexes for dumped tables
--

--
-- Indexes for table `qstCourier`
--
ALTER TABLE `qstCourier`
  ADD PRIMARY KEY (`id`);

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
