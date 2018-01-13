-- phpMyAdmin SQL Dump
-- version 4.4.15.5
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Dec 09, 2017 at 02:06 AM
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
-- Table structure for table `crpWalletDivisons`
--

CREATE TABLE `crpWalletDivisons` (
  `corporationID` int(10)  NOT NULL,
  `balance1` float NOT NULL DEFAULT '0',
  `balance2` float NOT NULL DEFAULT '0',
  `balance3` float NOT NULL DEFAULT '0',
  `balance4` float NOT NULL DEFAULT '0',
  `balance5` float NOT NULL DEFAULT '0',
  `balance6` float NOT NULL DEFAULT '0',
  `balance7` float NOT NULL DEFAULT '0',
  `division1` varchar(100) DEFAULT '1st division',
  `division2` varchar(100) DEFAULT '2nd division',
  `division3` varchar(100) DEFAULT '3rd division',
  `division4` varchar(100) DEFAULT '4th division',
  `division5` varchar(100) DEFAULT '5th division',
  `division6` varchar(100) DEFAULT '6th division',
  `division7` varchar(100) DEFAULT '7th division',
  `walletDivision1` varchar(100) DEFAULT '1st wallet division',
  `walletDivision2` varchar(100) DEFAULT '2nd wallet division',
  `walletDivision3` varchar(100) DEFAULT '3rd wallet division',
  `walletDivision4` varchar(100) DEFAULT '4th wallet division',
  `walletDivision5` varchar(100) DEFAULT '5th wallet division',
  `walletDivision6` varchar(100) DEFAULT '6th wallet division',
  `walletDivision7` varchar(100) DEFAULT '7th wallet division'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Corporation Wallet Data';

--
-- Indexes for dumped tables
--

--
-- Indexes for table `crpWalletDivisons`
--
ALTER TABLE `crpWalletDivisons`
  ADD PRIMARY KEY (`corporationID`);

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
