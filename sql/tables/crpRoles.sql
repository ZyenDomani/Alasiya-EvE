-- phpMyAdmin SQL Dump
-- version 4.4.15.5
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Jul 20, 2017 at 11:53 PM
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
-- Table structure for table `crpRoles`
--

CREATE TABLE IF NOT EXISTS `crpRoles` (
  `roleIID` smallint(6) NOT NULL,
  `roleID` int(20) NOT NULL,
  `roleName` varchar(130) COLLATE utf8_unicode_ci NOT NULL,
  `shortDescription` varchar(130) COLLATE utf8_unicode_ci NOT NULL,
  `shortDescriptionID` mediumint(6) NOT NULL,
  `description` varchar(130) COLLATE utf8_unicode_ci NOT NULL,
  `descriptionID` mediumint(6) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Dumping data for table `crpRoles`
--

INSERT INTO `crpRoles` (`roleIID`, `roleID`, `roleName`, `shortDescription`, `shortDescriptionID`, `description`, `descriptionID`) VALUES
(1, 1, 'roleDirector', 'Director', 60174, 'Can do anything like a CEO. Can assign any role.', 60126),
(2, 128, 'rolePersonnelManager', 'Personnel Manager', 60175, 'Can accept applications to join the corporation.', 60127),
(3, 256, 'roleAccountant', 'Accountant', 60176, 'Can view/use corporation accountancy info.', 60128),
(4, 512, 'roleSecurityOfficer', 'Security Officer', 60177, 'Can view the content of others hangars', 60129),
(5, 1024, 'roleFactoryManager', 'Factory Manager', 60178, 'Can perform factory management tasks.', 60130),
(6, 2048, 'roleStationManager', 'Station Manager', 60179, 'Can perform station management for a corporation.', 60131),
(7, 4096, 'roleAuditor', 'Auditor', 60180, 'Can perform audits on corporation security event logs', 60132),
(8, 8192, 'roleHangarCanTake1', 'Hangar Take [1]', 60181, 'Can take items from this divisions hangar', 60133),
(9, 16384, 'roleHangarCanTake2', 'Hangar Take [2]', 60182, 'Can take items from this divisions hangar', 60134),
(10, 32768, 'roleHangarCanTake3', 'Hangar Take [3]', 60183, 'Can take items from this divisions hangar', 60135),
(11, 65536, 'roleHangarCanTake4', 'Hangar Take [4]', 60184, 'Can take items from this divisions hangar', 60136),
(12, 131072, 'roleHangarCanTake5', 'Hangar Take [5]', 60185, 'Can take items from this divisions hangar', 60137),
(13, 262144, 'roleHangarCanTake6', 'Hangar Take [6]', 60186, 'Can take items from this divisions hangar', 60138),
(14, 524288, 'roleHangarCanTake7', 'Hangar Take [7]', 60187, 'Can take items from this divisions hangar', 60139),
(15, 1048576, 'roleHangarCanQuery1', 'Hangar Query [1]', 60188, 'Can query the content of this divisions hangar', 60140),
(16, 2097152, 'roleHangarCanQuery2', 'Hangar Query [2]', 60189, 'Can query the content of this divisions hangar', 60141),
(17, 4194304, 'roleHangarCanQuery3', 'Hangar Query [3]', 60190, 'Can query the content of this divisions hangar', 60142),
(18, 8388608, 'roleHangarCanQuery4', 'Hangar Query [4]', 60191, 'Can query the content of this divisions hangar', 60143),
(19, 16777216, 'roleHangarCanQuery5', 'Hangar Query [5]', 60192, 'Can query the content of this divisions hangar', 60144),
(20, 33554432, 'roleHangarCanQuery6', 'Hangar Query [6]', 60193, 'Can query the content of this divisions hangar', 60145),
(21, 67108864, 'roleHangarCanQuery7', 'Hangar Query [7]', 60194, 'Can query the content of this divisions hangar', 60146),
(22, 134217728, 'roleAccountCanTake1', 'Account Take [1]', 60195, 'Can take funds from this divisions account', 60147),
(23, 268435456, 'roleAccountCanTake2', 'Account Take [2]', 60196, 'Can take funds from this divisions account', 60148),
(24, 536870912, 'roleAccountCanTake3', 'Account Take [3]', 60197, 'Can take funds from this divisions account', 60149),
(25, 1073741824, 'roleAccountCanTake4', 'Account Take [4]', 60198, 'Can take funds from this divisions account', 60150),
(26, 2147483647, 'roleAccountCanTake5', 'Account Take [5]', 60199, 'Can take funds from this divisions account', 60151),
(27, 2147483647, 'roleAccountCanTake6', 'Account Take [6]', 60200, 'Can take funds from this divisions account', 60152),
(28, 2147483647, 'roleAccountCanTake7', 'Account Take [7]', 60201, 'Can take funds from this divisions account', 60153),
(29, 2147483647, 'roleDiplomat', 'Diplomat', 60202, 'Can set standings for the corporation.', 60154),
(36, 2147483647, 'roleEquipmentConfig', 'Config Equipment', 60203, 'Can deploy and configure equipment in space.', 60155),
(37, 2147483647, 'roleContainerCanTake1', 'Container Take [1]', 60204, 'Can take containers from this divisional hangar', 60156),
(38, 2147483647, 'roleContainerCanTake2', 'Container Take [2]', 60205, 'Can take containers from this divisional hangar', 60157),
(39, 2147483647, 'roleContainerCanTake3', 'Container Take [3]', 60206, 'Can take containers from this divisional hangar', 60158),
(40, 2147483647, 'roleContainerCanTake4', 'Container Take [4]', 60207, 'Can take containers from this divisional hangar', 60159),
(41, 2147483647, 'roleContainerCanTake5', 'Container Take [5]', 60208, 'Can take containers from this divisional hangar', 60160),
(42, 2147483647, 'roleContainerCanTake6', 'Container Take [6]', 60209, 'Can take containers from this divisional hangar', 60161),
(43, 2147483647, 'roleContainerCanTake7', 'Container Take [7]', 60210, 'Can take containers from this divisional hangar', 60162),
(44, 2147483647, 'roleCanRentOffice', 'Rent Office', 60211, 'When assigned to a member, the member can rent offices on behalf of the corporation', 60163),
(45, 2147483647, 'roleCanRentFactorySlot', 'Rent Factory Slot', 60212, 'When assigned to a member, the member can rent factory slots on behalf of the corporation', 60164),
(46, 2147483647, 'roleCanRentResearchSlot', 'Rent Research Slot', 60213, 'When assigned to a member, the member can rent research facilities on behalf of the corporation', 60165),
(47, 2147483647, 'roleJuniorAccountant', 'Junior Accountant', 60214, 'Can view corporation accountancy info.', 60166),
(48, 2147483647, 'roleStarbaseConfig', 'Config Starbase Equipment', 60215, 'Can deploy and configure starbase structures in space.', 60167),
(49, 2147483647, 'roleTrader', 'Trader', 60216, 'Can buy and sell things for the corporation', 60168),
(50, 2147483647, 'roleChatManager', 'Communications Officer', 60217, 'Can moderate corporation/alliance communications channels', 60169),
(51, 2147483647, 'roleContractManager', 'Contract Manager', 60218, 'Can create, edit and oversee all contracts made on behalf of the corportation as well as accept contracts on behalf of the corpora', 60170),
(52, 2147483647, 'roleInfrastructureTacticalOfficer', 'Starbase Defense Operator', 60219, 'Can operate defensive starbase structures', 60171),
(53, 2147483647, 'roleStarbaseCaretaker', 'Starbase Fuel Technician', 60220, 'Can refuel starbases and take from silo bins', 60172),
(54, 2147483647, 'roleFittingManager', 'Fitting Manager', 60221, 'Can add and delete fittings', 60173);

--
-- Indexes for dumped tables
--

--
-- Indexes for table `crpRoles`
--
ALTER TABLE `crpRoles`
  ADD PRIMARY KEY (`roleIID`),
  ADD UNIQUE KEY `roleIID` (`roleIID`);

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
