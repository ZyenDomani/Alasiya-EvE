-- phpMyAdmin SQL Dump
-- version 4.4.15.10
-- https://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: May 31, 2019 at 10:11 AM
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
-- Table structure for table `crpMedalStatus`
--

DROP TABLE IF EXISTS `crpMedalStatus`;
CREATE TABLE IF NOT EXISTS `crpMedalStatus` (
  `ai` int(11) NOT NULL,
  `statusID` int(11) NOT NULL,
  `statusName` varchar(25) COLLATE utf8_unicode_ci NOT NULL
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Dumping data for table `crpMedalStatus`
--

INSERT INTO `crpMedalStatus` (`ai`, `statusID`, `statusName`) VALUES
(1, 0, 'invalid'),
(2, 1, 'Remove'),
(3, 2, 'Private'),
(4, 3, 'Public');

--
-- Indexes for dumped tables
--

--
-- Indexes for table `crpMedalStatus`
--
ALTER TABLE `crpMedalStatus`
  ADD PRIMARY KEY (`ai`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `crpMedalStatus`
--
ALTER TABLE `crpMedalStatus`
  MODIFY `ai` int(11) NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=5;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
