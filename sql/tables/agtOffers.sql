-- phpMyAdmin SQL Dump
-- version 4.4.15.10
-- https://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Nov 16, 2018 at 07:44 AM
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

DROP TABLE IF EXISTS `agtOffers`;
CREATE TABLE `agtOffers` (
  `offerID` int(10) NOT NULL,
  `agentID` int(10) NOT NULL DEFAULT '0',
  `characterID` int(10) NOT NULL DEFAULT '0',
  `missionID` int(10) NOT NULL DEFAULT '0',
  `stateID` tinyint(1) NOT NULL DEFAULT '0',
  `expiryTime` bigint(20) NOT NULL DEFAULT '0',
  `rewardLP` int(10) NOT NULL DEFAULT '0',
  `rewardISK` int(10) NOT NULL DEFAULT '0',
  `rewardItemID` int(10) NOT NULL DEFAULT '0',
  `rewardItemQty` smallint(6) NOT NULL DEFAULT '0',
  `originID` int(10) NOT NULL DEFAULT '0',
  `originOwnerID` int(10) NOT NULL DEFAULT '0',
  `originSystemID` int(10) NOT NULL DEFAULT '0',
  `destinationID` int(10) NOT NULL DEFAULT '0',
  `destinationTypeID` int(10) NOT NULL DEFAULT '0',
  `destinationOwnerID` int(10) NOT NULL DEFAULT '0',
  `destinationSystemID` int(10) NOT NULL DEFAULT '0',
  `dungeonLocationID` int(10) NOT NULL DEFAULT '0',
  `dungeonSolarSystemID` int(10) NOT NULL DEFAULT '0',
  `acceptFee` float NOT NULL DEFAULT '0',
  `courierTypeID` int(5) NOT NULL DEFAULT '0',
  `courierAmount` smallint(6) NOT NULL DEFAULT '0',
  `courierVolume` float NOT NULL DEFAULT '0.1',
  `dateIssued` bigint(20) unsigned NOT NULL DEFAULT '0',
  `dateAccepted` bigint(20) unsigned NOT NULL DEFAULT '0',
  `dateCompleted` bigint(20) unsigned NOT NULL DEFAULT '0',
  `important` tinyint(1) NOT NULL DEFAULT '0',
  `name` text NOT NULL,
  `remoteCompletable` tinyint(1) NOT NULL DEFAULT '0',
  `remoteOfferable` tinyint(1) NOT NULL DEFAULT '0',
  `typeID` smallint(6) NOT NULL DEFAULT '0',
  `bonusISK` int(10) NOT NULL DEFAULT '0',
  `bonusTime` bigint(20) NOT NULL DEFAULT '0',
  `briefingID` int(11) NOT NULL DEFAULT '0',
  `storyline` tinyint(1) NOT NULL DEFAULT '0'
) ENGINE=MyISAM AUTO_INCREMENT=10 DEFAULT CHARSET=utf8 COMMENT='char missions - current offers and history';

--
-- Dumping data for table `agtOffers`
--

INSERT INTO `agtOffers` (`offerID`, `agentID`, `characterID`, `missionID`, `stateID`, `expiryTime`, `rewardLP`, `rewardISK`, `rewardItemID`, `rewardItemQty`, `originID`, `originOwnerID`, `originSystemID`, `destinationID`, `destinationTypeID`, `destinationOwnerID`, `destinationSystemID`, `dungeonLocationID`, `dungeonSolarSystemID`, `acceptFee`, `courierTypeID`, `courierAmount`, `courierVolume`, `dateIssued`, `dateAccepted`, `dateCompleted`, `important`, `name`, `remoteCompletable`, `remoteOfferable`, `typeID`, `bonusISK`, `bonusTime`, `briefingID`, `storyline`) VALUES
(1, 3012447, 90000000, 55205, 5, 131867989685683312, 0, 16500, 0, 0, 60005044, 1000053, 30002507, 60005728, 2497, 1000059, 30002507, 0, 0, 0, 3687, 75, 1, 131867125685681776, 0, 0, 0, 'Broken Reactor', 0, 0, 3, 0, 0, 130997, 0),
(2, 3012447, 90000000, 55207, 5, 131868029558643072, 0, 19500, 0, 0, 60005044, 1000053, 30002507, 60005047, 2502, 1000053, 30002506, 0, 0, 0, 3814, 100, 0.1, 131867165558642192, 0, 0, 0, 'High Command', 0, 0, 3, 0, 0, 131000, 0),
(3, 3014109, 90000000, 57827, 5, 131868032626436432, 0, 16500, 0, 0, 60005044, 1000053, 30002507, 60009121, 3870, 1000094, 30002507, 0, 0, 0, 2610, 2, 40, 131867168626435616, 0, 0, 0, 'Hunger Strikes', 0, 0, 3, 0, 0, 143459, 0),
(4, 3014108, 90000000, 57831, 2, 131868062806290736, 0, 12000, 0, 0, 60005044, 1000053, 30002507, 60009121, 3870, 1000094, 30002507, 0, 0, 0, 21044, 10, 2, 131867198806289984, 131867749320938192, 0, 0, 'Stolen Arms', 0, 0, 3, 0, 0, 143449, 0),
(5, 3014109, 90000000, 55205, 2, 131868097646212880, 0, 16500, 0, 0, 60005044, 1000053, 30002507, 60004591, 2502, 1000049, 30002507, 0, 0, 0, 3687, 75, 1, 131867233646212032, 131867746551001552, 0, 0, 'Broken Reactor', 0, 0, 3, 0, 0, 130997, 0),
(6, 3012447, 90000000, 55207, 5, 131868098810349920, 0, 19500, 0, 0, 60005044, 1000053, 30002507, 60005047, 2502, 1000053, 30002506, 0, 0, 0, 3814, 100, 0.1, 131867234810349104, 131867728837265296, 0, 0, 'High Command', 0, 0, 3, 0, 0, 131000, 0),
(7, 3014112, 90000000, 57868, 5, 131868099197061552, 0, 17500, 0, 0, 60005044, 1000053, 30002507, 60004588, 2498, 1000049, 30002510, 0, 0, 0, 3647, 250, 0.5, 131867235197060656, 0, 0, 0, 'Our Boys Are Getting Bored', 0, 0, 3, 0, 0, 131014, 0),
(8, 3014110, 90000000, 57864, 1, 131868124181075504, 0, 41000, 0, 0, 60005044, 1000053, 30002507, 60005728, 2497, 1000059, 30002507, 0, 0, 0, 2666, 1, 40, 131867260181074864, 0, 0, 0, 'Marriage', 0, 0, 3, 0, 0, 143454, 0),
(9, 3014112, 90000000, 55325, 2, 131868664867419408, 0, 31000, 0, 0, 60005044, 1000053, 30002507, 60005047, 2502, 1000053, 30002506, 0, 0, 0, 16042, 8, 50, 131867800867418080, 131867830868870656, 0, 0, 'A Special Delivery', 0, 0, 3, 0, 0, 131525, 0);

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
  MODIFY `offerID` int(10) NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=10;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
