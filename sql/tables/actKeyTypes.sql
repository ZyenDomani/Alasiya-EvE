-- phpMyAdmin SQL Dump
-- version 4.4.15.5
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Dec 09, 2017 at 02:09 AM
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
-- Table structure for table `actKeyTypes`
--

CREATE TABLE IF NOT EXISTS `actKeyTypes` (
  `keyID` int(10)  NOT NULL DEFAULT '0',
  `keyType` varchar(100) NOT NULL DEFAULT '',
  `keyName` varchar(100) NOT NULL DEFAULT '',
  `description` varchar(100) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `actKeyTypes`
--

INSERT INTO `actKeyTypes` (`keyID`, `keyType`, `keyName`, `description`) VALUES
(1000, 'A', 'cash', ''),
(1001, 'A', 'cash2', ''),
(1002, 'A', 'cash3', ''),
(1003, 'A', 'cash4', ''),
(1004, 'A', 'cash5', ''),
(1005, 'A', 'cash6', ''),
(1006, 'A', 'cash7', ''),
(1100, 'A', 'property', ''),
(1200, 'A', 'aurum', ''),
(1201, 'A', 'aurum2', ''),
(1202, 'A', 'aurum3', ''),
(1203, 'A', 'aurum4', ''),
(1204, 'A', 'aurum5', ''),
(1205, 'A', 'aurum6', ''),
(1206, 'A', 'aurum7', ''),
(1500, 'A', 'escrow', ''),
(1800, 'A', 'receivables', ''),
(2000, 'L', 'payables', ''),
(2010, 'L', 'gold', ''),
(2900, 'L', 'equity', ''),
(3000, 'R', 'sales', ''),
(4000, 'C', 'purchases', '');

--
-- Indexes for dumped tables
--

--
-- Indexes for table `actKeyTypes`
--
ALTER TABLE `actKeyTypes`
  ADD PRIMARY KEY (`keyID`);

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
