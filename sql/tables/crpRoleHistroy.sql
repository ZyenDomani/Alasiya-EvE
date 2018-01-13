-- phpMyAdmin SQL Dump
-- version 4.4.15.5
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Dec 05, 2017 at 04:49 PM
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
-- Table structure for table `crpRoleHistroy`
--

CREATE TABLE `crpRoleHistroy` (
  `id` int(10) NOT NULL,
  `corporationID` int(10)  NOT NULL DEFAULT '0',
  `charID` int(10)  NOT NULL DEFAULT '0',
  `issuerID` int(10)  NOT NULL DEFAULT '0',
  `changeTime` bigint(20) NOT NULL DEFAULT '0',
  `oldRoles` bigint(20) NOT NULL DEFAULT '0',
  `newRoles` bigint(20) NOT NULL DEFAULT '0',
  `grantable` tinyint(1)  NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Role History logging for Corp AuditMember';

--
-- Indexes for dumped tables
--

--
-- Indexes for table `crpRoleHistroy`
--
ALTER TABLE `crpRoleHistroy`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `id_2` (`id`),
  ADD KEY `id` (`id`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `crpRoleHistroy`
--
ALTER TABLE `crpRoleHistroy`
  MODIFY `id` int(10) NOT NULL AUTO_INCREMENT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
