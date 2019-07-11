-- phpMyAdmin SQL Dump
-- version 4.4.15.10
-- https://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: May 21, 2019 at 12:48 AM
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
-- Table structure for table `crpVotes`
--

DROP TABLE IF EXISTS `crpVotes`;
CREATE TABLE IF NOT EXISTS `crpVotes` (
  `charID` int(11) NOT NULL,
  `corpID` int(11) NOT NULL,
  `voteCaseID` mediumint(11) NOT NULL,
  `optionID` tinyint(1) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Indexes for dumped tables
--

-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `crpVoteData`
--

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
