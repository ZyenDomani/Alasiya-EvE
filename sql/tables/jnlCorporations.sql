-- phpMyAdmin SQL Dump
-- version 4.4.15.5
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Dec 09, 2017 at 02:08 AM
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
-- Table structure for table `jnlCorporations`
--

CREATE TABLE IF NOT EXISTS `jnlCorporations` (
  `transactionID` int(10)  NOT NULL,
  `ownerID` int(10)  NOT NULL DEFAULT '0',
  `entryTypeID` tinyint(3)  NOT NULL DEFAULT '0',
  `accountKey` smallint(5)  NOT NULL DEFAULT '0',
  `transactionDate` bigint(20) DEFAULT NULL,
  `ownerID1` int(10)  NOT NULL DEFAULT '0',
  `ownerID2` int(10)  NOT NULL DEFAULT '0',
  `referenceID` int(10)  NOT NULL DEFAULT '0',
  `currency` tinyint(1) NOT NULL DEFAULT '1',
  `amount` double NOT NULL DEFAULT '0',
  `balance` double NOT NULL DEFAULT '0',
  `description` text
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Corporation Journal Data';

--
-- Indexes for dumped tables
--

--
-- Indexes for table `jnlCorporations`
--
ALTER TABLE `jnlCorporations`
  ADD PRIMARY KEY (`transactionID`),
  ADD KEY `ownerID` (`ownerID`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `jnlCorporations`
--
ALTER TABLE `jnlCorporations`
  MODIFY `transactionID` int(10)  NOT NULL AUTO_INCREMENT;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
