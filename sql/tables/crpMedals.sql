-- phpMyAdmin SQL Dump
-- version 4.4.15.10
-- https://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: May 28, 2019 at 06:51 PM
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

--
-- Table structure for table `crpMedals`
--

DROP TABLE IF EXISTS `crpMedals`;
CREATE TABLE IF NOT EXISTS `crpMedals` (
  `medalID` int(11) NOT NULL,
  `ownerID` int(11) NOT NULL DEFAULT '0',
  `creatorID` int(11) NOT NULL DEFAULT '0',
  `noRecepients` int(11) NOT NULL DEFAULT '0',
  `date` bigint(20) NOT NULL DEFAULT '0',
  `title` varchar(50) COLLATE utf8_unicode_ci NOT NULL,
  `description` varchar(150) COLLATE utf8_unicode_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `crpMedals`
--
ALTER TABLE `crpMedals`
  ADD PRIMARY KEY (`medalID`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `crpMedals`
--
ALTER TABLE `crpMedals`
  MODIFY `medalID` int(11) NOT NULL AUTO_INCREMENT;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
