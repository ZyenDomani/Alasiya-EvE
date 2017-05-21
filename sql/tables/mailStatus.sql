-- phpMyAdmin SQL Dump
-- version 4.4.15.5
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: May 21, 2017 at 04:54 PM
-- Server version: 10.0.24-MariaDB
-- PHP Version: 5.6.21

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";

--
-- Database: `alasiya-new`
--

-- --------------------------------------------------------

--
-- Table structure for table `mailStatus`
--

CREATE TABLE `mailStatus` (
  `messageID` int(10) unsigned NOT NULL,
  `characterID` int(10) unsigned NOT NULL,
  `statusMask` bigint(20) unsigned DEFAULT NULL,
  `labelMask` bigint(20) unsigned DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `mailStatus`
--
ALTER TABLE `mailStatus`
  ADD KEY `messageID` (`messageID`);

