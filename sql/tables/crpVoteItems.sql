-- phpMyAdmin SQL Dump
-- version 4.4.15.5
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Dec 09, 2017 at 02:07 AM
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
-- Table structure for table `crpVoteItems`
--

CREATE TABLE IF NOT EXISTS `crpVoteItems` (
  `corporationID` int(10)  NOT NULL,
  `parameter` int(10)  NOT NULL,
  `timeActedUpon` bigint(20) NOT NULL DEFAULT '0',
  `timeRescended` bigint(20) NOT NULL DEFAULT '0',
  `voteCaseID` int(10)  NOT NULL DEFAULT '0',
  `voteType` int(10)  NOT NULL DEFAULT '0',
  `title` varchar(50) DEFAULT NULL,
  `description` varchar(150) DEFAULT NULL,
  `expires` bigint(20) NOT NULL DEFAULT '0',
  `actedUpon` tinyint(1)  NOT NULL DEFAULT '0',
  `inEffect` tinyint(1)  NOT NULL DEFAULT '0',
  `rescended` tinyint(1)  NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='GetVoteCasesByCorporation';

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
