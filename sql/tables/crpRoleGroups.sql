-- phpMyAdmin SQL Dump
-- version 4.4.15.5
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Jul 20, 2017 at 11:52 PM
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
-- Table structure for table `crpRoleGroups`
--

CREATE TABLE IF NOT EXISTS `crpRoleGroups` (
  `roleGroupID` tinyint(2) NOT NULL,
  `roleGroupName` varchar(130) COLLATE utf8_unicode_ci NOT NULL,
  `roleMask` int(20) NOT NULL,
  `appliesTo` varchar(130) COLLATE utf8_unicode_ci NOT NULL,
  `appliesToGrantable` varchar(130) COLLATE utf8_unicode_ci NOT NULL,
  `isLocational` tinyint(1) NOT NULL,
  `isDivisional` tinyint(1) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Dumping data for table `crpRoleGroups`
--

INSERT INTO `crpRoleGroups` (`roleGroupID`, `roleGroupName`, `roleMask`, `appliesTo`, `appliesToGrantable`, `isLocational`, `isDivisional`) VALUES
(1, 'General', 2147483647, 'roles', 'grantableRoles', 0, 0),
(2, 'Station Service', 2147483647, 'roles', 'grantableRoles', 0, 0),
(3, 'Accounting (Divisional)', 2147483647, 'roles', 'grantableRoles', 0, 1),
(4, 'Hangar Access (Headquarters)', 134209536, 'rolesAtHQ', 'grantableRolesAtHQ', 1, 1),
(5, 'Container Access (Headquarters)', 2147483647, 'rolesAtHQ', 'grantableRolesAtHQ', 1, 1),
(6, 'Hangar Access (Based at)', 134209536, 'rolesAtBase', 'grantableRolesAtBase', 1, 1),
(7, 'Container Access (Based at)', 2147483647, 'rolesAtBase', 'grantableRolesAtBase', 1, 1),
(8, 'Hangar Access (Other)', 134209536, 'rolesAtOther', 'grantableRolesAtOther', 1, 1),
(9, 'Container Access (Other)', 2147483647, 'rolesAtOther', 'grantableRolesAtOther', 1, 1);

--
-- Indexes for dumped tables
--

--
-- Indexes for table `crpRoleGroups`
--
ALTER TABLE `crpRoleGroups`
  ADD PRIMARY KEY (`roleGroupID`),
  ADD UNIQUE KEY `roleGroupID` (`roleGroupID`);

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
