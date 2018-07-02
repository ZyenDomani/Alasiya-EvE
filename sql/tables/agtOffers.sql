-- phpMyAdmin SQL Dump
-- version 4.4.15.10
-- https://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Jul 02, 2018 at 12:33 PM
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
-- Table structure for table `agtOffers`
--

CREATE TABLE IF NOT EXISTS `agtOffers` (
  `offerID` int(10) NOT NULL,
  `agentID` int(10) NOT NULL DEFAULT '0',
  `characterID` int(10) NOT NULL DEFAULT '0',
  `missionID` int(10) NOT NULL DEFAULT '0',
  `stateID` tinyint(1) NOT NULL DEFAULT '0',
  `expiryTime` bigint(20) NOT NULL DEFAULT '0',
  `rewardLP` int(10) NOT NULL DEFAULT '0',
  `rewardISK` int(10) NOT NULL DEFAULT '0',
  `rewardItemID` int(10) NOT NULL DEFAULT '0',
  `rewardItemAmount` smallint(6) NOT NULL DEFAULT '0',
  `originID` int(10) NOT NULL DEFAULT '0',
  `destinationID` int(10) NOT NULL DEFAULT '0',
  `acceptFee` float NOT NULL DEFAULT '0',
  `courierItemID` int(5) NOT NULL DEFAULT '0',
  `courierAmount` smallint(6) NOT NULL DEFAULT '0',
  `dateIssued` int(10) unsigned NOT NULL DEFAULT '0',
  `dateAccepted` int(10) unsigned NOT NULL DEFAULT '0',
  `dateCompleted` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='char missions - current offers and history';

--
-- Indexes for dumped tables
--

--
-- Indexes for table `agtOffers`
--
ALTER TABLE `agtOffers`
  ADD PRIMARY KEY (`offerID`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `agtOffers`
--
ALTER TABLE `agtOffers`
  MODIFY `offerID` int(10) NOT NULL AUTO_INCREMENT;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
