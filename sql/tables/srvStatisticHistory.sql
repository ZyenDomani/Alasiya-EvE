-- phpMyAdmin SQL Dump
-- version 4.4.15.10
-- https://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Jan 10, 2019 at 01:31 PM
-- Server version: 10.0.36-MariaDB
-- PHP Version: 5.6.36

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
-- Table structure for table `srvStatisticHistory`
--

DROP TABLE IF EXISTS `srvStatisticHistory`;
CREATE TABLE `srvStatisticHistory` (
  `idx` int(2) unsigned NOT NULL DEFAULT '0',
  `pcShots` int(10) unsigned NOT NULL DEFAULT '0',
  `pcMissiles` int(10) unsigned NOT NULL DEFAULT '0',
  `ramJobs` int(11) NOT NULL DEFAULT '0',
  `shipsSalvaged` smallint(5) unsigned NOT NULL DEFAULT '0',
  `pcBounties` double NOT NULL DEFAULT '0',
  `npcBounties` double NOT NULL DEFAULT '0',
  `oreMined` double NOT NULL DEFAULT '0',
  `iskMarket` double NOT NULL DEFAULT '0',
  `probesLaunched` mediumint(5) unsigned NOT NULL DEFAULT '0',
  `sitesScanned` mediumint(5) unsigned NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Historical Data for graphing player activity from previous months';

--
-- Indexes for dumped tables
--

--
-- Indexes for table `srvStatisticHistory`
--
ALTER TABLE `srvStatisticHistory`
  ADD UNIQUE KEY `timeStamp` (`idx`);

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
