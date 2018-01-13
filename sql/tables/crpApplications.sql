-- phpMyAdmin SQL Dump
-- version 4.4.15.5
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Dec 05, 2017 at 04:46 PM
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
-- Table structure for table `crpApplications`
--

CREATE TABLE `crpApplications` (
  `appID` int(5)  NOT NULL,
  `corporationID` int(10)  NOT NULL,
  `characterID` int(10)  NOT NULL,
  `applicationText` text NOT NULL,
  `roles` bigint(20)  NOT NULL,
  `grantableRoles` bigint(20)  NOT NULL,
  `status` int(10)  NOT NULL,
  `applicationDateTime` bigint(20)  NOT NULL,
  `deleted` tinyint(3)  NOT NULL,
  `lastCorpUpdaterID` int(10)  NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `crpApplications`
--
ALTER TABLE `crpApplications`
  ADD PRIMARY KEY (`appID`),
  ADD UNIQUE KEY `appID` (`appID`),
  ADD KEY `corporationID` (`corporationID`),
  ADD KEY `characterID` (`characterID`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `crpApplications`
--
ALTER TABLE `crpApplications`
  MODIFY `appID` int(5)  NOT NULL AUTO_INCREMENT;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
