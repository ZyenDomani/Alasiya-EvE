-- phpMyAdmin SQL Dump
-- version 4.4.15.10
-- https://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: May 28, 2019 at 06:54 PM
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
-- Table structure for table `crpVoteOptions`
--

DROP TABLE IF EXISTS `crpVoteOptions`;
CREATE TABLE IF NOT EXISTS `crpVoteOptions` (
  `ai` int(11) NOT NULL,
  `voteCaseID` int(11) NOT NULL DEFAULT '0',
  `optionID` tinyint(2) DEFAULT NULL,
  `optionText` varchar(150) COLLATE utf8_unicode_ci NOT NULL,
  `parameter` int(10) DEFAULT NULL,
  `parameter1` int(10) DEFAULT NULL,
  `parameter2` int(10) DEFAULT NULL,
  `votesFor` int(10) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='Corp Vote Options';

--
-- Indexes for dumped tables
--

--
-- Indexes for table `crpVoteOptions`
--
ALTER TABLE `crpVoteOptions`
  ADD PRIMARY KEY (`ai`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `crpVoteOptions`
--
ALTER TABLE `crpVoteOptions`
  MODIFY `ai` int(11) NOT NULL AUTO_INCREMENT;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
