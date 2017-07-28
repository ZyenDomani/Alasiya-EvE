-- phpMyAdmin SQL Dump
-- version 4.4.15.5
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Jul 27, 2017 at 07:41 PM
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
-- Table structure for table `posStructureData`
--

CREATE TABLE IF NOT EXISTS `posStructureData` (
  `itemID` int(10) NOT NULL DEFAULT '0',
  `towerID` int(10) NOT NULL DEFAULT '0',
  `planetID` int(10) NOT NULL DEFAULT '0',
  `harmonic` int(10) NOT NULL DEFAULT '0',
  `password` varchar(50) NOT NULL,
  `standingOwnerID` int(10) NOT NULL DEFAULT '0',
  `state` tinyint(1) NOT NULL DEFAULT '1',
  `level` smallint(3) NOT NULL DEFAULT '0',
  `standing` double NOT NULL DEFAULT '0',
  `status` double NOT NULL DEFAULT '0',
  `timestamp` bigint(20) NOT NULL DEFAULT '0',
  `rotationX` double NOT NULL DEFAULT '0',
  `rotationY` double NOT NULL DEFAULT '0',
  `rotationZ` double NOT NULL DEFAULT '0',
  `statusDrop` tinyint(1) NOT NULL DEFAULT '0',
  `corpWar` tinyint(1) NOT NULL DEFAULT '0',
  `showInCalendar` tinyint(1) NOT NULL DEFAULT '0',
  `sendFuelNotifications` tinyint(1) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='POS - Structure Data';

--
-- Indexes for dumped tables
--

--
-- Indexes for table `posStructureData`
--
ALTER TABLE `posStructureData`
  ADD PRIMARY KEY (`itemID`);

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
