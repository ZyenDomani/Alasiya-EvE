-- phpMyAdmin SQL Dump
-- version 4.4.15.5
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Dec 10, 2017 at 01:03 PM
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
-- Table structure for table `alnAlliance`
--

CREATE TABLE IF NOT EXISTS `alnAlliance` (
  `allianceID` int(11)  NOT NULL,
  `name` text NOT NULL,
  `typeID` int(11) NOT NULL DEFAULT '32',
  `shortName` varchar(20) NOT NULL,
  `executorCorpID` int(11) NOT NULL,
  `creatorCorpID` int(11) NOT NULL,
  `creatorCharID` int(11) NOT NULL,
  `startDate` int(20) NOT NULL,
  `url` text NOT NULL
) ENGINE=InnoDB AUTO_INCREMENT=99000000 DEFAULT CHARSET=utf8;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `alnAlliance`
--
ALTER TABLE `alnAlliance`
  ADD PRIMARY KEY (`allianceID`),
  ADD UNIQUE KEY `allianceID` (`allianceID`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `alnAlliance`
--
ALTER TABLE `alnAlliance`
  MODIFY `allianceID` int(11)  NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=99000000;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
