-- phpMyAdmin SQL Dump
-- version 4.4.15.10
-- https://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Mar 25, 2021 at 05:48 PM
-- Server version: 10.0.36-MariaDB
-- PHP Version: 5.6.36

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `Alasiya_EvE_Copy`
--

-- --------------------------------------------------------

--
-- Table structure for table `account`
--

CREATE TABLE `account` (
  `accountID` int(10) unsigned NOT NULL,
  `clientID` int(10) unsigned NOT NULL DEFAULT '0',
  `accountName` varchar(43) NOT NULL DEFAULT '',
  `password` varchar(43) NOT NULL DEFAULT '',
  `hash` tinyblob,
  `type` tinyint(3) unsigned NOT NULL DEFAULT '23',
  `role` bigint(20) unsigned NOT NULL DEFAULT '0',
  `online` tinyint(1) NOT NULL DEFAULT '0',
  `banned` tinyint(1) NOT NULL DEFAULT '0',
  `logonCount` int(10) unsigned NOT NULL DEFAULT '0',
  `lastLogin` timestamp NULL DEFAULT '0000-00-00 00:00:00' ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `actKeyTypes`
--

CREATE TABLE `actKeyTypes` (
  `keyID` int(10) unsigned NOT NULL DEFAULT '0',
  `keyType` varchar(100) NOT NULL DEFAULT '',
  `keyName` varchar(100) NOT NULL DEFAULT '',
  `keyNameID` int(11) NOT NULL DEFAULT '0',
  `description` varchar(100) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `agtAgents`
--

CREATE TABLE `agtAgents` (
  `agentID` int(11) NOT NULL,
  `divisionID` tinyint(3) unsigned DEFAULT NULL,
  `corporationID` int(11) DEFAULT NULL,
  `locationID` int(11) DEFAULT NULL,
  `level` tinyint(4) DEFAULT NULL,
  `quality` smallint(6) DEFAULT NULL,
  `agentTypeID` int(11) DEFAULT NULL,
  `isLocator` tinyint(1) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `agtAgentTypes`
--

CREATE TABLE `agtAgentTypes` (
  `agentTypeID` int(11) NOT NULL,
  `agentType` varchar(50) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `agtMissions`
--

CREATE TABLE `agtMissions` (
  `id` int(5) NOT NULL DEFAULT '0',
  `briefingID` int(5) NOT NULL DEFAULT '0',
  `name` text,
  `level` tinyint(1) NOT NULL DEFAULT '0',
  `typeID` tinyint(1) NOT NULL DEFAULT '0',
  `important` bit(1) NOT NULL DEFAULT b'0',
  `storyline` bit(1) NOT NULL DEFAULT b'0',
  `raceID` tinyint(2) NOT NULL DEFAULT '0',
  `constellationID` int(10) NOT NULL DEFAULT '0',
  `corporationID` int(10) NOT NULL DEFAULT '0',
  `dungeonID` int(10) NOT NULL DEFAULT '0',
  `rewardISK` int(10) NOT NULL DEFAULT '0',
  `rewardItemID` int(11) NOT NULL DEFAULT '0',
  `rewardItemQty` int(11) NOT NULL DEFAULT '0',
  `bonusISK` int(11) NOT NULL DEFAULT '0',
  `bonusTime` int(10) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `agtOffers`
--

CREATE TABLE `agtOffers` (
  `offerID` int(10) NOT NULL,
  `agentID` int(10) NOT NULL DEFAULT '0',
  `characterID` int(10) NOT NULL DEFAULT '0',
  `missionID` int(10) NOT NULL DEFAULT '0',
  `stateID` tinyint(1) NOT NULL DEFAULT '0',
  `expiryTime` bigint(20) NOT NULL DEFAULT '0',
  `rewardLP` int(10) NOT NULL DEFAULT '0',
  `rewardISK` int(10) NOT NULL DEFAULT '0',
  `rewardItemID` int(10) NOT NULL DEFAULT '0',
  `rewardItemQty` smallint(6) NOT NULL DEFAULT '0',
  `originID` int(10) NOT NULL DEFAULT '0',
  `originOwnerID` int(10) NOT NULL DEFAULT '0',
  `originSystemID` int(10) NOT NULL DEFAULT '0',
  `destinationID` int(10) NOT NULL DEFAULT '0',
  `destinationTypeID` int(10) NOT NULL DEFAULT '0',
  `destinationOwnerID` int(10) NOT NULL DEFAULT '0',
  `destinationSystemID` int(10) NOT NULL DEFAULT '0',
  `dungeonLocationID` int(10) NOT NULL DEFAULT '0',
  `dungeonSolarSystemID` int(10) NOT NULL DEFAULT '0',
  `acceptFee` float NOT NULL DEFAULT '0',
  `courierTypeID` int(5) NOT NULL DEFAULT '0',
  `courierAmount` smallint(6) NOT NULL DEFAULT '0',
  `courierVolume` float NOT NULL DEFAULT '0.1',
  `dateIssued` bigint(20) unsigned NOT NULL DEFAULT '0',
  `dateAccepted` bigint(20) unsigned NOT NULL DEFAULT '0',
  `dateCompleted` bigint(20) unsigned NOT NULL DEFAULT '0',
  `important` bit(1) NOT NULL DEFAULT b'0',
  `name` text NOT NULL,
  `remoteCompletable` bit(1) NOT NULL DEFAULT b'0',
  `remoteOfferable` bit(1) NOT NULL DEFAULT b'0',
  `typeID` smallint(6) NOT NULL DEFAULT '0',
  `bonusISK` int(10) NOT NULL DEFAULT '0',
  `bonusTime` bigint(20) NOT NULL DEFAULT '0',
  `briefingID` int(11) NOT NULL DEFAULT '0',
  `storyline` bit(1) NOT NULL DEFAULT b'0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='char missions - current offers and history';

-- --------------------------------------------------------

--
-- Table structure for table `agtSkillLevel`
--

CREATE TABLE `agtSkillLevel` (
  `agentID` int(11) NOT NULL,
  `typeID` int(11) NOT NULL,
  `level` tinyint(2) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='agentID, skillID, skillLevel';

-- --------------------------------------------------------

--
-- Table structure for table `alnAlliance`
--

CREATE TABLE `alnAlliance` (
  `allianceID` int(11) unsigned NOT NULL,
  `allianceName` text NOT NULL,
  `typeID` int(11) NOT NULL DEFAULT '32',
  `shortName` varchar(20) NOT NULL,
  `executorCorpID` int(11) NOT NULL,
  `creatorCorpID` int(11) NOT NULL,
  `creatorCharID` int(11) NOT NULL,
  `startDate` int(20) NOT NULL,
  `memberCount` smallint(5) NOT NULL DEFAULT '0',
  `url` text NOT NULL,
  `deleted` tinyint(1) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `alnApplications`
--

CREATE TABLE `alnApplications` (
  `applicationID` int(5) unsigned NOT NULL,
  `corporationID` int(10) unsigned NOT NULL,
  `allianceID` int(10) unsigned NOT NULL,
  `applicationText` text NOT NULL,
  `state` int(10) unsigned NOT NULL DEFAULT '0',
  `applicationDateTime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `deleted` bit(1) NOT NULL DEFAULT b'0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `alnContacts`
--

CREATE TABLE `alnContacts` (
  `id` int(10) NOT NULL,
  `ownerID` int(10) NOT NULL,
  `contactID` int(10) NOT NULL,
  `inWatchlist` bit(1) NOT NULL DEFAULT b'0',
  `relationshipID` float NOT NULL,
  `labelMask` bigint(20) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Alliance Contacts Data';

-- --------------------------------------------------------

--
-- Table structure for table `alnLabels`
--

CREATE TABLE `alnLabels` (
  `labelID` int(10) NOT NULL,
  `ownerID` int(10) NOT NULL,
  `color` int(10) NOT NULL,
  `name` varchar(130) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Alliance Label Data';

-- --------------------------------------------------------

--
-- Table structure for table `avatars`
--

CREATE TABLE `avatars` (
  `charID` int(11) NOT NULL,
  `hairDarkness` float NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `avatar_colors`
--

CREATE TABLE `avatar_colors` (
  `charID` int(11) NOT NULL,
  `colorID` int(5) NOT NULL,
  `colorNameA` int(5) NOT NULL,
  `colorNameBC` int(5) NOT NULL,
  `weight` float NOT NULL,
  `gloss` float NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `avatar_modifiers`
--

CREATE TABLE `avatar_modifiers` (
  `charID` int(11) NOT NULL,
  `modifierLocationID` int(5) NOT NULL,
  `paperdollResourceID` int(5) NOT NULL,
  `paperdollResourceVariation` int(5) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `avatar_sculpts`
--

CREATE TABLE `avatar_sculpts` (
  `charID` int(11) NOT NULL,
  `sculptLocationID` int(5) NOT NULL,
  `weightUpDown` float DEFAULT NULL,
  `weightLeftRight` float DEFAULT NULL,
  `weightForwardBack` float DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `billsPayable`
--

CREATE TABLE `billsPayable` (
  `billID` int(10) unsigned NOT NULL DEFAULT '0',
  `billTypeID` int(10) unsigned DEFAULT NULL,
  `debtorID` int(10) unsigned DEFAULT NULL,
  `creditorID` int(10) unsigned DEFAULT NULL,
  `amount` text NOT NULL,
  `dueDateTime` text NOT NULL,
  `interest` text NOT NULL,
  `externalID` int(10) unsigned DEFAULT NULL,
  `paid` text NOT NULL,
  `externalID2` text NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `billsReceivable`
--

CREATE TABLE `billsReceivable` (
  `billID` int(10) unsigned NOT NULL DEFAULT '0',
  `billTypeID` int(10) unsigned DEFAULT NULL,
  `debtorID` int(10) unsigned DEFAULT NULL,
  `creditorID` int(10) unsigned DEFAULT NULL,
  `amount` text NOT NULL,
  `dueDateTime` text NOT NULL,
  `interest` text NOT NULL,
  `externalID` int(10) unsigned DEFAULT NULL,
  `paid` text NOT NULL,
  `externalID2` text NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `billTypes`
--

CREATE TABLE `billTypes` (
  `billTypeID` int(10) unsigned NOT NULL DEFAULT '0',
  `billTypeName` varchar(100) NOT NULL DEFAULT '',
  `description` varchar(100) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `bloodlineTypes`
--

CREATE TABLE `bloodlineTypes` (
  `bloodlineID` int(10) unsigned NOT NULL DEFAULT '0',
  `typeID` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `bookmarkFolders`
--

CREATE TABLE `bookmarkFolders` (
  `folderID` int(10) unsigned NOT NULL,
  `folderName` varchar(255) NOT NULL DEFAULT '',
  `ownerID` int(10) NOT NULL DEFAULT '0',
  `creatorID` int(10) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `bookmarks`
--

CREATE TABLE `bookmarks` (
  `bookmarkID` int(10) unsigned NOT NULL,
  `ownerID` int(10) unsigned NOT NULL DEFAULT '0',
  `itemID` int(10) unsigned NOT NULL DEFAULT '0',
  `typeID` int(10) unsigned NOT NULL DEFAULT '0',
  `memo` varchar(85) DEFAULT NULL,
  `created` bigint(20) unsigned NOT NULL DEFAULT '0',
  `x` double NOT NULL DEFAULT '0',
  `y` double NOT NULL DEFAULT '0',
  `z` double NOT NULL DEFAULT '0',
  `locationID` int(10) unsigned NOT NULL DEFAULT '0',
  `note` varchar(85) DEFAULT NULL,
  `creatorID` int(10) unsigned NOT NULL DEFAULT '0',
  `folderID` smallint(6) unsigned DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `cacheLocations`
--

CREATE TABLE `cacheLocations` (
  `locationID` int(10) unsigned NOT NULL DEFAULT '0',
  `locationName` varchar(100) NOT NULL DEFAULT '',
  `locationNameID` tinyint(4) NOT NULL DEFAULT '0',
  `x` double NOT NULL DEFAULT '0',
  `y` double NOT NULL DEFAULT '0',
  `z` double NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `cacheOwners`
--

CREATE TABLE `cacheOwners` (
  `ownerID` int(10) unsigned NOT NULL DEFAULT '0',
  `ownerName` varchar(100) NOT NULL DEFAULT '',
  `typeID` int(10) unsigned NOT NULL DEFAULT '0',
  `ownerNameID` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `careers`
--

CREATE TABLE `careers` (
  `id` int(11) NOT NULL,
  `raceID` tinyint(3) unsigned DEFAULT NULL,
  `careerID` int(11) DEFAULT NULL,
  `careerName` text COLLATE utf8_unicode_ci,
  `description` text COLLATE utf8_unicode_ci,
  `shortDescription` text COLLATE utf8_unicode_ci,
  `graphicID` int(11) DEFAULT NULL,
  `schoolID` tinyint(3) unsigned DEFAULT NULL,
  `iconID` int(11) DEFAULT NULL,
  `dataID` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `channelChars`
--

CREATE TABLE `channelChars` (
  `channelID` int(10) NOT NULL DEFAULT '0',
  `corpID` int(10) unsigned NOT NULL DEFAULT '0',
  `charID` int(10) unsigned NOT NULL DEFAULT '0',
  `allianceID` int(10) unsigned NOT NULL DEFAULT '0',
  `role` bigint(20) unsigned NOT NULL DEFAULT '0',
  `extra` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `channelMods`
--

CREATE TABLE `channelMods` (
  `id` int(10) unsigned NOT NULL,
  `channelID` int(10) NOT NULL DEFAULT '0',
  `accessor` int(10) unsigned DEFAULT NULL,
  `mode` int(10) unsigned NOT NULL DEFAULT '0',
  `untilWhen` bigint(20) unsigned DEFAULT NULL,
  `originalMode` int(10) unsigned DEFAULT NULL,
  `admin` varchar(85) DEFAULT NULL,
  `reason` text
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `channels`
--

CREATE TABLE `channels` (
  `channelID` int(10) NOT NULL,
  `ownerID` int(10) unsigned NOT NULL DEFAULT '0',
  `displayName` varchar(85) DEFAULT NULL,
  `motd` text,
  `comparisonKey` varchar(11) DEFAULT NULL,
  `memberless` tinyint(1) NOT NULL DEFAULT '0',
  `password` varchar(100) DEFAULT NULL,
  `mailingList` tinyint(1) NOT NULL DEFAULT '0',
  `cspa` smallint(4) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrAccessories`
--

CREATE TABLE `chrAccessories` (
  `accessoryID` int(10) unsigned NOT NULL DEFAULT '0',
  `accessoryName` varchar(100) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrAncestries`
--

CREATE TABLE `chrAncestries` (
  `ancestryID` int(11) NOT NULL DEFAULT '0',
  `ancestryName` text COLLATE utf8_unicode_ci,
  `bloodlineID` tinyint(3) unsigned DEFAULT NULL,
  `description` text COLLATE utf8_unicode_ci,
  `perception` tinyint(3) unsigned DEFAULT NULL,
  `willpower` tinyint(3) unsigned DEFAULT NULL,
  `charisma` tinyint(3) unsigned DEFAULT NULL,
  `memory` tinyint(3) unsigned DEFAULT NULL,
  `intelligence` tinyint(3) unsigned DEFAULT NULL,
  `shortDescription` text COLLATE utf8_unicode_ci,
  `iconID` int(11) DEFAULT NULL,
  `ancestryNameID` int(11) DEFAULT NULL,
  `descriptionID` int(11) DEFAULT NULL,
  `dataID` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `chrAttributes`
--

CREATE TABLE `chrAttributes` (
  `attributeID` tinyint(3) unsigned NOT NULL,
  `attributeName` varchar(100) DEFAULT NULL,
  `description` varchar(1000) DEFAULT NULL,
  `iconID` smallint(6) DEFAULT NULL,
  `shortDescription` varchar(500) DEFAULT NULL,
  `notes` varchar(500) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrBackgrounds`
--

CREATE TABLE `chrBackgrounds` (
  `backgroundID` int(10) unsigned NOT NULL DEFAULT '0',
  `backgroundName` varchar(100) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrBeards`
--

CREATE TABLE `chrBeards` (
  `beardID` int(10) unsigned NOT NULL DEFAULT '0',
  `beardName` varchar(100) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrBLAccessories`
--

CREATE TABLE `chrBLAccessories` (
  `bloodlineID` int(10) unsigned NOT NULL DEFAULT '0',
  `gender` int(10) unsigned NOT NULL DEFAULT '0',
  `accessoryID` int(10) unsigned NOT NULL DEFAULT '0',
  `npc` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrBLBackgrounds`
--

CREATE TABLE `chrBLBackgrounds` (
  `backgroundID` int(10) unsigned NOT NULL DEFAULT '0',
  `backgroundName` varchar(100) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrBLBeards`
--

CREATE TABLE `chrBLBeards` (
  `bloodlineID` int(10) unsigned NOT NULL DEFAULT '0',
  `gender` int(10) unsigned NOT NULL DEFAULT '0',
  `beardID` int(10) unsigned NOT NULL DEFAULT '0',
  `npc` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrBLCostumes`
--

CREATE TABLE `chrBLCostumes` (
  `bloodlineID` int(10) unsigned NOT NULL DEFAULT '0',
  `gender` int(10) unsigned NOT NULL DEFAULT '0',
  `costumeID` int(10) unsigned NOT NULL DEFAULT '0',
  `npc` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrBLDecos`
--

CREATE TABLE `chrBLDecos` (
  `bloodlineID` int(10) unsigned NOT NULL DEFAULT '0',
  `gender` int(10) unsigned NOT NULL DEFAULT '0',
  `decoID` int(10) unsigned NOT NULL DEFAULT '0',
  `npc` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrBLEyebrows`
--

CREATE TABLE `chrBLEyebrows` (
  `bloodlineID` int(10) unsigned NOT NULL DEFAULT '0',
  `gender` int(10) unsigned NOT NULL DEFAULT '0',
  `eyebrowsID` int(10) unsigned NOT NULL DEFAULT '0',
  `npc` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrBLEyes`
--

CREATE TABLE `chrBLEyes` (
  `bloodlineID` int(10) unsigned NOT NULL DEFAULT '0',
  `gender` int(10) unsigned NOT NULL DEFAULT '0',
  `eyesID` int(10) unsigned NOT NULL DEFAULT '0',
  `npc` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrBLHairs`
--

CREATE TABLE `chrBLHairs` (
  `bloodlineID` int(10) unsigned NOT NULL DEFAULT '0',
  `gender` int(10) unsigned NOT NULL DEFAULT '0',
  `hairID` int(10) unsigned NOT NULL DEFAULT '0',
  `npc` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrBLLights`
--

CREATE TABLE `chrBLLights` (
  `lightID` int(10) unsigned NOT NULL DEFAULT '0',
  `lightName` varchar(100) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrBLLipsticks`
--

CREATE TABLE `chrBLLipsticks` (
  `bloodlineID` int(10) unsigned NOT NULL DEFAULT '0',
  `gender` int(10) unsigned NOT NULL DEFAULT '0',
  `lipstickID` int(10) unsigned NOT NULL DEFAULT '0',
  `npc` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrBLMakeups`
--

CREATE TABLE `chrBLMakeups` (
  `bloodlineID` int(10) unsigned NOT NULL DEFAULT '0',
  `gender` int(10) unsigned NOT NULL DEFAULT '0',
  `makeupID` int(10) unsigned NOT NULL DEFAULT '0',
  `npc` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrBloodlineNames`
--

CREATE TABLE `chrBloodlineNames` (
  `nameID` int(11) NOT NULL DEFAULT '0',
  `bloodlineID` int(11) DEFAULT NULL,
  `lastName` text COLLATE utf8_unicode_ci
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `chrBloodlines`
--

CREATE TABLE `chrBloodlines` (
  `bloodlineID` int(11) NOT NULL DEFAULT '0',
  `bloodlineName` text COLLATE utf8_unicode_ci,
  `raceID` tinyint(3) unsigned DEFAULT NULL,
  `description` text COLLATE utf8_unicode_ci,
  `maleDescription` text COLLATE utf8_unicode_ci,
  `femaleDescription` text COLLATE utf8_unicode_ci,
  `shipTypeID` int(11) DEFAULT NULL,
  `corporationID` int(11) DEFAULT NULL,
  `perception` tinyint(3) unsigned DEFAULT NULL,
  `willpower` tinyint(3) unsigned DEFAULT NULL,
  `charisma` tinyint(3) unsigned DEFAULT NULL,
  `memory` tinyint(3) unsigned DEFAULT NULL,
  `intelligence` tinyint(3) unsigned DEFAULT NULL,
  `shortDescription` text COLLATE utf8_unicode_ci,
  `shortMaleDescription` text COLLATE utf8_unicode_ci,
  `shortFemaleDescription` text COLLATE utf8_unicode_ci,
  `iconID` int(11) DEFAULT NULL,
  `bloodlineNameID` int(11) DEFAULT NULL,
  `descriptionID` int(11) DEFAULT NULL,
  `dataID` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `chrBLSkins`
--

CREATE TABLE `chrBLSkins` (
  `bloodlineID` int(10) unsigned NOT NULL DEFAULT '0',
  `gender` int(10) unsigned NOT NULL DEFAULT '0',
  `skinID` int(10) unsigned NOT NULL DEFAULT '0',
  `npc` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrCertificates`
--

CREATE TABLE `chrCertificates` (
  `id` int(10) unsigned NOT NULL,
  `characterID` int(10) NOT NULL,
  `certificateID` tinyint(3) NOT NULL DEFAULT '0',
  `grantDate` bigint(20) NOT NULL,
  `visibilityFlags` bit(1) NOT NULL DEFAULT b'0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrCharacterAttributes`
--

CREATE TABLE `chrCharacterAttributes` (
  `charID` int(10) unsigned NOT NULL DEFAULT '0',
  `attributeID` int(10) unsigned NOT NULL DEFAULT '0',
  `valueInt` bigint(20) DEFAULT NULL,
  `valueFloat` double DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrCharacters`
--

CREATE TABLE `chrCharacters` (
  `characterID` int(10) unsigned NOT NULL,
  `accountID` int(10) unsigned DEFAULT NULL,
  `characterName` text NOT NULL,
  `title` text NOT NULL,
  `description` text NOT NULL,
  `typeID` int(6) NOT NULL DEFAULT '0',
  `flag` smallint(3) NOT NULL DEFAULT '0',
  `bounty` double NOT NULL DEFAULT '0',
  `balance` double NOT NULL DEFAULT '0',
  `aurBalance` double NOT NULL DEFAULT '0',
  `securityRating` float NOT NULL DEFAULT '0',
  `locationID` int(10) NOT NULL DEFAULT '0',
  `petitionMessage` text NOT NULL,
  `logonDateTime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `logoffDateTime` bigint(20) NOT NULL DEFAULT '0',
  `logonMinutes` int(10) unsigned NOT NULL DEFAULT '0',
  `skillPoints` bigint(20) NOT NULL DEFAULT '0',
  `skillQueueEndTime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `corporationID` int(10) unsigned NOT NULL DEFAULT '0',
  `baseID` int(10) NOT NULL DEFAULT '0',
  `corpAccountKey` smallint(4) NOT NULL DEFAULT '0',
  `corpRole` bigint(20) unsigned NOT NULL DEFAULT '0',
  `rolesAtAll` bigint(20) unsigned NOT NULL DEFAULT '0',
  `rolesAtHQ` bigint(20) unsigned NOT NULL DEFAULT '0',
  `rolesAtBase` bigint(20) unsigned NOT NULL DEFAULT '0',
  `rolesAtOther` bigint(20) unsigned NOT NULL DEFAULT '0',
  `grantableRoles` bigint(20) NOT NULL DEFAULT '0',
  `grantableRolesAtHQ` bigint(20) NOT NULL DEFAULT '0',
  `grantableRolesAtBase` bigint(20) NOT NULL DEFAULT '0',
  `grantableRolesAtOther` bigint(20) NOT NULL DEFAULT '0',
  `titleMask` bigint(20) NOT NULL DEFAULT '0',
  `blockRoles` bit(1) NOT NULL DEFAULT b'0',
  `startDateTime` bigint(20) NOT NULL DEFAULT '0',
  `createDateTime` bigint(20) NOT NULL DEFAULT '0',
  `ancestryID` int(10) unsigned NOT NULL DEFAULT '0',
  `bloodlineID` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `raceID` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `careerID` int(10) unsigned NOT NULL DEFAULT '0',
  `schoolID` int(10) unsigned NOT NULL DEFAULT '0',
  `careerSpecialityID` int(10) unsigned NOT NULL DEFAULT '0',
  `gender` tinyint(1) NOT NULL DEFAULT '0',
  `stationID` int(10) unsigned NOT NULL DEFAULT '0',
  `solarSystemID` int(10) unsigned NOT NULL DEFAULT '0',
  `constellationID` int(10) unsigned NOT NULL DEFAULT '0',
  `regionID` int(10) unsigned NOT NULL DEFAULT '0',
  `online` tinyint(1) NOT NULL DEFAULT '0',
  `freeRespecs` tinyint(1) unsigned NOT NULL DEFAULT '2',
  `lastRespecDateTime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `nextRespecDateTime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `deletePrepareDateTime` bigint(20) unsigned DEFAULT '0',
  `shipID` int(10) unsigned NOT NULL DEFAULT '0',
  `capsuleID` int(10) NOT NULL DEFAULT '0',
  `age` int(10) NOT NULL DEFAULT '0',
  `paperDollState` tinyint(2) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrContacts`
--

CREATE TABLE `chrContacts` (
  `id` int(10) NOT NULL,
  `ownerID` int(10) NOT NULL,
  `contactID` int(10) NOT NULL,
  `inWatchlist` bit(1) NOT NULL DEFAULT b'0',
  `relationshipID` float NOT NULL,
  `labelMask` bigint(20) NOT NULL,
  `blocked` bit(1) NOT NULL DEFAULT b'0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Character Contacts Data';

-- --------------------------------------------------------

--
-- Table structure for table `chrCostumes`
--

CREATE TABLE `chrCostumes` (
  `costumeID` int(10) unsigned NOT NULL DEFAULT '0',
  `costumeName` varchar(100) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrDecos`
--

CREATE TABLE `chrDecos` (
  `decoID` int(10) unsigned NOT NULL DEFAULT '0',
  `decoName` varchar(100) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrDefaultOverviewGroups`
--

CREATE TABLE `chrDefaultOverviewGroups` (
  `id` int(11) NOT NULL,
  `overviewID` int(11) DEFAULT NULL,
  `groupID` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `chrDefaultOverviews`
--

CREATE TABLE `chrDefaultOverviews` (
  `overviewID` int(11) NOT NULL DEFAULT '0',
  `overviewName` text COLLATE utf8_unicode_ci,
  `overviewShortName` text COLLATE utf8_unicode_ci,
  `overviewNameID` int(11) DEFAULT NULL,
  `dataID` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `chrDepartments`
--

CREATE TABLE `chrDepartments` (
  `schoolID` int(10) unsigned NOT NULL DEFAULT '0',
  `departmentID` int(10) unsigned NOT NULL DEFAULT '0',
  `departmentName` varchar(100) NOT NULL DEFAULT '',
  `description` mediumtext NOT NULL,
  `skillTypeID1` int(10) unsigned NOT NULL DEFAULT '0',
  `skillTypeID2` int(10) unsigned NOT NULL DEFAULT '0',
  `skillTypeID3` int(10) unsigned NOT NULL DEFAULT '0',
  `graphicID` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrEmployment`
--

CREATE TABLE `chrEmployment` (
  `characterID` int(10) unsigned NOT NULL DEFAULT '0',
  `corporationID` int(10) unsigned NOT NULL DEFAULT '0',
  `startDate` bigint(20) unsigned NOT NULL DEFAULT '0',
  `deleted` tinyint(4) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrEyebrows`
--

CREATE TABLE `chrEyebrows` (
  `eyebrowsID` int(10) unsigned NOT NULL DEFAULT '0',
  `eyebrowsName` varchar(100) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrEyes`
--

CREATE TABLE `chrEyes` (
  `eyesID` int(10) unsigned NOT NULL DEFAULT '0',
  `eyesName` varchar(100) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrHairs`
--

CREATE TABLE `chrHairs` (
  `hairID` int(10) unsigned NOT NULL DEFAULT '0',
  `hairName` varchar(100) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrKillTable`
--

CREATE TABLE `chrKillTable` (
  `killID` int(10) unsigned NOT NULL,
  `solarSystemID` int(10) unsigned NOT NULL DEFAULT '0',
  `victimCharacterID` int(10) unsigned NOT NULL DEFAULT '0',
  `victimCorporationID` int(10) unsigned NOT NULL,
  `victimAllianceID` int(10) NOT NULL,
  `victimFactionID` int(10) unsigned NOT NULL,
  `victimShipTypeID` smallint(4) unsigned NOT NULL DEFAULT '0',
  `victimDamageTaken` int(10) unsigned NOT NULL DEFAULT '0',
  `finalCharacterID` int(10) unsigned NOT NULL DEFAULT '0',
  `finalCorporationID` int(10) unsigned NOT NULL,
  `finalAllianceID` int(10) unsigned NOT NULL,
  `finalFactionID` int(10) unsigned NOT NULL,
  `finalShipTypeID` smallint(4) unsigned NOT NULL DEFAULT '0',
  `finalWeaponTypeID` smallint(4) unsigned NOT NULL,
  `finalSecurityStatus` double NOT NULL DEFAULT '0',
  `finalDamageDone` int(10) unsigned NOT NULL DEFAULT '0',
  `killBlob` blob NOT NULL,
  `killTime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `moonID` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrLabels`
--

CREATE TABLE `chrLabels` (
  `labelID` int(10) NOT NULL,
  `ownerID` int(10) NOT NULL,
  `color` int(10) NOT NULL,
  `name` varchar(130) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Character Label Data';

-- --------------------------------------------------------

--
-- Table structure for table `chrLights`
--

CREATE TABLE `chrLights` (
  `lightID` int(10) unsigned NOT NULL DEFAULT '0',
  `lightName` varchar(100) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrLipsticks`
--

CREATE TABLE `chrLipsticks` (
  `lipstickID` int(10) unsigned NOT NULL DEFAULT '0',
  `lipstickName` varchar(100) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrMakeups`
--

CREATE TABLE `chrMakeups` (
  `makeupID` int(10) unsigned NOT NULL DEFAULT '0',
  `makeupName` varchar(100) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrMedals`
--

CREATE TABLE `chrMedals` (
  `recepientID` int(11) NOT NULL,
  `medalID` int(11) NOT NULL,
  `corpID` int(11) NOT NULL,
  `issuerID` int(11) NOT NULL DEFAULT '0',
  `status` tinyint(1) NOT NULL DEFAULT '2',
  `reason` varchar(150) COLLATE utf8_unicode_ci NOT NULL,
  `date` bigint(20) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `chrNotes`
--

CREATE TABLE `chrNotes` (
  `itemID` int(10) unsigned NOT NULL DEFAULT '0',
  `ownerID` int(10) unsigned NOT NULL DEFAULT '0',
  `note` text
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrNPCCharacters`
--

CREATE TABLE `chrNPCCharacters` (
  `characterID` int(10) unsigned NOT NULL DEFAULT '0',
  `characterName` varchar(43) NOT NULL DEFAULT '',
  `accountID` int(10) unsigned DEFAULT NULL,
  `title` varchar(85) NOT NULL DEFAULT '',
  `description` text NOT NULL,
  `bounty` double NOT NULL DEFAULT '0',
  `balance` double NOT NULL DEFAULT '0',
  `securityRating` double NOT NULL DEFAULT '0',
  `petitionMessage` varchar(85) NOT NULL DEFAULT '',
  `logonMinutes` int(10) unsigned NOT NULL DEFAULT '0',
  `corporationID` int(10) unsigned NOT NULL DEFAULT '0',
  `corporationDateTime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `startDateTime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `createDateTime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `typeID` int(10) unsigned NOT NULL DEFAULT '0',
  `ancestryID` int(10) unsigned NOT NULL DEFAULT '0',
  `careerID` int(10) unsigned NOT NULL DEFAULT '0',
  `schoolID` int(10) unsigned NOT NULL DEFAULT '0',
  `careerSpecialityID` int(10) unsigned NOT NULL DEFAULT '0',
  `gender` tinyint(4) NOT NULL DEFAULT '0',
  `accessoryID` int(10) unsigned DEFAULT NULL,
  `beardID` int(10) unsigned DEFAULT NULL,
  `costumeID` int(10) unsigned NOT NULL DEFAULT '0',
  `decoID` int(10) unsigned DEFAULT NULL,
  `eyebrowsID` int(10) unsigned NOT NULL DEFAULT '0',
  `eyesID` int(10) unsigned NOT NULL DEFAULT '0',
  `hairID` int(10) unsigned NOT NULL DEFAULT '0',
  `lipstickID` int(10) unsigned DEFAULT NULL,
  `makeupID` int(10) unsigned DEFAULT NULL,
  `skinID` int(10) unsigned NOT NULL DEFAULT '0',
  `backgroundID` int(10) unsigned NOT NULL DEFAULT '0',
  `lightID` int(10) unsigned NOT NULL DEFAULT '0',
  `headRotation1` double NOT NULL DEFAULT '0',
  `headRotation2` double NOT NULL DEFAULT '0',
  `headRotation3` double NOT NULL DEFAULT '0',
  `eyeRotation1` double NOT NULL DEFAULT '0',
  `eyeRotation2` double NOT NULL DEFAULT '0',
  `eyeRotation3` double NOT NULL DEFAULT '0',
  `camPos1` double NOT NULL DEFAULT '0',
  `camPos2` double NOT NULL DEFAULT '0',
  `camPos3` double NOT NULL DEFAULT '0',
  `morph1e` double DEFAULT NULL,
  `morph1n` double DEFAULT NULL,
  `morph1s` double DEFAULT NULL,
  `morph1w` double DEFAULT NULL,
  `morph2e` double DEFAULT NULL,
  `morph2n` double DEFAULT NULL,
  `morph2s` double DEFAULT NULL,
  `morph2w` double DEFAULT NULL,
  `morph3e` double DEFAULT NULL,
  `morph3n` double DEFAULT NULL,
  `morph3s` double DEFAULT NULL,
  `morph3w` double DEFAULT NULL,
  `morph4e` double DEFAULT NULL,
  `morph4n` double DEFAULT NULL,
  `morph4s` double DEFAULT NULL,
  `morph4w` double DEFAULT NULL,
  `stationID` int(10) unsigned NOT NULL DEFAULT '0',
  `solarSystemID` int(10) unsigned NOT NULL DEFAULT '0',
  `constellationID` int(10) unsigned NOT NULL DEFAULT '0',
  `regionID` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrOwnerNote`
--

CREATE TABLE `chrOwnerNote` (
  `noteID` int(10) unsigned NOT NULL,
  `ownerID` int(10) unsigned NOT NULL,
  `label` text,
  `note` text
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrPausedSkillQueue`
--

CREATE TABLE `chrPausedSkillQueue` (
  `characterID` int(10) unsigned NOT NULL,
  `orderIndex` int(10) unsigned NOT NULL,
  `typeID` int(10) unsigned NOT NULL,
  `level` int(10) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrPortraitData`
--

CREATE TABLE `chrPortraitData` (
  `charID` int(10) NOT NULL DEFAULT '0',
  `backgroundID` int(10) NOT NULL DEFAULT '0',
  `lightID` int(10) NOT NULL DEFAULT '0',
  `lightColorID` int(10) NOT NULL DEFAULT '0',
  `cameraX` float NOT NULL DEFAULT '0',
  `cameraY` float NOT NULL DEFAULT '0',
  `cameraZ` float NOT NULL DEFAULT '0',
  `cameraPoiX` float NOT NULL DEFAULT '0',
  `cameraPoiY` float NOT NULL DEFAULT '0',
  `cameraPoiZ` float NOT NULL DEFAULT '0',
  `headLookTargetX` float NOT NULL DEFAULT '0',
  `headLookTargetY` float NOT NULL DEFAULT '0',
  `headLookTargetZ` float NOT NULL DEFAULT '0',
  `lightIntensity` float NOT NULL DEFAULT '0',
  `headTilt` float NOT NULL DEFAULT '0',
  `orientChar` float NOT NULL DEFAULT '0',
  `browLeftCurl` float NOT NULL DEFAULT '0',
  `browLeftTighten` float NOT NULL DEFAULT '0',
  `browLeftUpDown` float NOT NULL DEFAULT '0',
  `browRightCurl` float NOT NULL DEFAULT '0',
  `browRightTighten` float NOT NULL DEFAULT '0',
  `browRightUpDown` float NOT NULL DEFAULT '0',
  `eyeClose` float NOT NULL DEFAULT '0',
  `eyesLookVertical` float NOT NULL DEFAULT '0',
  `eyesLookHorizontal` float NOT NULL DEFAULT '0',
  `squintLeft` float NOT NULL DEFAULT '0',
  `squintRight` float NOT NULL DEFAULT '0',
  `jawSideways` float NOT NULL DEFAULT '0',
  `jawUp` float NOT NULL DEFAULT '0',
  `puckerLips` float NOT NULL DEFAULT '0',
  `frownLeft` float NOT NULL DEFAULT '0',
  `frownRight` float NOT NULL DEFAULT '0',
  `smileLeft` float NOT NULL DEFAULT '0',
  `smileRight` float NOT NULL DEFAULT '0',
  `cameraFieldOfView` float NOT NULL DEFAULT '0',
  `portraitPoseNumber` float NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='Portrait Data for Characters';

-- --------------------------------------------------------

--
-- Table structure for table `chrRaces`
--

CREATE TABLE `chrRaces` (
  `raceID` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `raceName` varchar(100) DEFAULT NULL,
  `description` varchar(1000) DEFAULT NULL,
  `iconID` smallint(6) NOT NULL DEFAULT '0',
  `shortDescription` varchar(500) DEFAULT NULL,
  `raceNameID` int(10) NOT NULL DEFAULT '0',
  `descriptionID` int(10) NOT NULL DEFAULT '0',
  `dataID` int(10) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrSchools`
--

CREATE TABLE `chrSchools` (
  `id` int(11) NOT NULL,
  `raceID` tinyint(3) unsigned DEFAULT NULL,
  `schoolID` tinyint(3) unsigned DEFAULT NULL,
  `schoolName` text COLLATE utf8_unicode_ci,
  `description` text COLLATE utf8_unicode_ci,
  `graphicID` int(11) DEFAULT NULL,
  `corporationID` int(11) DEFAULT NULL,
  `agentID` int(11) DEFAULT NULL,
  `newAgentID` int(11) DEFAULT NULL,
  `iconID` int(11) DEFAULT NULL,
  `schoolNameID` int(11) DEFAULT NULL,
  `descriptionID` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `chrShipFittings`
--

CREATE TABLE `chrShipFittings` (
  `id` int(10) unsigned NOT NULL,
  `characterID` int(10) unsigned NOT NULL,
  `shipID` int(10) unsigned NOT NULL,
  `shipDNA` tinytext NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Ship Stored Fittings, saved as ShipDNA';

-- --------------------------------------------------------

--
-- Table structure for table `chrSkillHistory`
--

CREATE TABLE `chrSkillHistory` (
  `ai` int(10) NOT NULL,
  `eventTypeID` smallint(3) NOT NULL,
  `characterID` int(10) NOT NULL,
  `logDate` bigint(20) NOT NULL,
  `skillTypeID` int(8) NOT NULL,
  `skillLevel` tinyint(4) NOT NULL,
  `absolutePoints` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Char Skill History';

-- --------------------------------------------------------

--
-- Table structure for table `chrSkillQueue`
--

CREATE TABLE `chrSkillQueue` (
  `characterID` int(10) unsigned NOT NULL,
  `orderIndex` tinyint(3) unsigned NOT NULL,
  `typeID` smallint(5) unsigned NOT NULL,
  `level` tinyint(2) unsigned NOT NULL,
  `startTime` bigint(20) NOT NULL DEFAULT '0',
  `endTime` bigint(20) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED;

-- --------------------------------------------------------

--
-- Table structure for table `chrSkins`
--

CREATE TABLE `chrSkins` (
  `skinID` int(10) unsigned NOT NULL DEFAULT '0',
  `skinName` varchar(100) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `chrVisitedSystems`
--

CREATE TABLE `chrVisitedSystems` (
  `characterID` int(20) NOT NULL,
  `solarSystemID` int(10) NOT NULL,
  `visits` int(10) NOT NULL DEFAULT '0',
  `lastDateTime` bigint(20) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `crpActivities`
--

CREATE TABLE `crpActivities` (
  `activityID` tinyint(3) unsigned NOT NULL,
  `activityName` varchar(100) DEFAULT NULL,
  `description` varchar(1000) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `crpAdGroupData`
--

CREATE TABLE `crpAdGroupData` (
  `groupID` int(10) NOT NULL,
  `groupName` text NOT NULL,
  `groupNameID` int(10) NOT NULL,
  `description` text NOT NULL,
  `descriptionID` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Corporate Advert Group Data';

-- --------------------------------------------------------

--
-- Table structure for table `crpAdRegistry`
--

CREATE TABLE `crpAdRegistry` (
  `adID` int(10) NOT NULL,
  `corporationID` int(10) NOT NULL,
  `allianceID` int(10) NOT NULL,
  `stationID` int(10) NOT NULL,
  `regionID` int(10) NOT NULL,
  `raceMask` tinyint(3) NOT NULL,
  `typeMask` int(10) NOT NULL,
  `createDateTime` bigint(20) NOT NULL,
  `expiryDateTime` bigint(20) NOT NULL,
  `description` text NOT NULL,
  `title` varchar(30) NOT NULL,
  `memberCount` smallint(5) NOT NULL,
  `channelID` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Corporate Advert Data';

-- --------------------------------------------------------

--
-- Table structure for table `crpAdTypeData`
--

CREATE TABLE `crpAdTypeData` (
  `typeMask` int(10) NOT NULL,
  `typeName` text NOT NULL,
  `typeNameID` int(10) NOT NULL,
  `groupID` int(10) NOT NULL,
  `description` text NOT NULL,
  `descriptionID` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Corporate Advert Type Data';

-- --------------------------------------------------------

--
-- Table structure for table `crpApplications`
--

CREATE TABLE `crpApplications` (
  `applicationID` int(5) unsigned NOT NULL,
  `corporationID` int(10) unsigned NOT NULL,
  `characterID` int(10) unsigned NOT NULL,
  `applicationText` text NOT NULL,
  `roles` bigint(20) unsigned NOT NULL DEFAULT '0',
  `grantableRoles` bigint(20) unsigned NOT NULL DEFAULT '0',
  `status` int(10) unsigned NOT NULL DEFAULT '0',
  `applicationDateTime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `deleted` bit(1) NOT NULL DEFAULT b'0',
  `lastCorpUpdaterID` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `crpAutoPay`
--

CREATE TABLE `crpAutoPay` (
  `corporationID` int(10) NOT NULL DEFAULT '0',
  `market` bit(1) NOT NULL DEFAULT b'0',
  `rental` bit(1) NOT NULL DEFAULT b'0',
  `broker` bit(1) NOT NULL DEFAULT b'0',
  `war` bit(1) NOT NULL DEFAULT b'0',
  `alliance` bit(1) NOT NULL DEFAULT b'0',
  `sov` bit(1) NOT NULL DEFAULT b'0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `crpBulletins`
--

CREATE TABLE `crpBulletins` (
  `bulletinID` int(10) unsigned NOT NULL,
  `corporationID` int(10) unsigned NOT NULL DEFAULT '0',
  `ownerID` int(10) unsigned NOT NULL DEFAULT '0',
  `createCharacterID` int(10) unsigned NOT NULL DEFAULT '0',
  `createDateTime` bigint(20) NOT NULL DEFAULT '0',
  `editCharacterID` int(10) unsigned NOT NULL DEFAULT '0',
  `editDateTime` bigint(20) NOT NULL DEFAULT '0',
  `title` varchar(130) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `body` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Corporation Bulletin Data';

-- --------------------------------------------------------

--
-- Table structure for table `crpContacts`
--

CREATE TABLE `crpContacts` (
  `id` int(10) NOT NULL,
  `ownerID` int(10) NOT NULL,
  `contactID` int(10) NOT NULL,
  `inWatchlist` bit(1) NOT NULL DEFAULT b'0',
  `relationshipID` float NOT NULL,
  `labelMask` int(20) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='Corporate Contacts Data';

-- --------------------------------------------------------

--
-- Table structure for table `crpCorporation`
--

CREATE TABLE `crpCorporation` (
  `corporationID` int(10) unsigned NOT NULL,
  `corporationType` int(3) NOT NULL DEFAULT '1',
  `corporationName` varchar(100) NOT NULL DEFAULT '',
  `description` mediumtext NOT NULL,
  `tickerName` varchar(8) NOT NULL DEFAULT '',
  `allianceID` int(10) NOT NULL DEFAULT '0',
  `warFactionID` int(10) NOT NULL DEFAULT '0',
  `memberCount` smallint(5) unsigned NOT NULL DEFAULT '0',
  `url` mediumtext NOT NULL,
  `taxRate` float NOT NULL DEFAULT '0.11',
  `minimumJoinStanding` float NOT NULL DEFAULT '0',
  `hasPlayerPersonnelManager` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `sendCharTerminationMessage` tinyint(1) unsigned NOT NULL DEFAULT '1',
  `creatorID` int(10) unsigned NOT NULL DEFAULT '0',
  `ceoID` int(10) unsigned NOT NULL DEFAULT '0',
  `stationID` int(10) unsigned NOT NULL DEFAULT '0',
  `raceID` int(10) unsigned DEFAULT NULL,
  `allianceMemberStartDate` bigint(20) NOT NULL,
  `shares` mediumint(8) unsigned NOT NULL DEFAULT '1000',
  `memberLimit` smallint(5) unsigned NOT NULL DEFAULT '10',
  `allowedMemberRaceIDs` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `graphicID` int(10) unsigned NOT NULL DEFAULT '0',
  `shape1` int(10) unsigned DEFAULT NULL,
  `shape2` int(10) unsigned DEFAULT NULL,
  `shape3` int(10) unsigned DEFAULT NULL,
  `color1` int(10) unsigned DEFAULT NULL,
  `color2` int(10) unsigned DEFAULT NULL,
  `color3` int(10) unsigned DEFAULT NULL,
  `typeface` varchar(11) DEFAULT NULL,
  `deleted` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `isRecruiting` tinyint(1) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `crpEmployment`
--

CREATE TABLE `crpEmployment` (
  `allianceID` int(10) unsigned NOT NULL DEFAULT '0',
  `corporationID` int(10) unsigned NOT NULL DEFAULT '0',
  `startDate` bigint(20) unsigned NOT NULL DEFAULT '0',
  `deleted` tinyint(4) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `crpItemEvent`
--

CREATE TABLE `crpItemEvent` (
  `eventID` int(10) NOT NULL,
  `corporationID` int(10) unsigned NOT NULL,
  `characterID` int(10) unsigned NOT NULL,
  `eventTypeID` smallint(5) unsigned NOT NULL DEFAULT '10',
  `eventDateTime` bigint(20) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Event History logging for Corp AuditMember';

-- --------------------------------------------------------

--
-- Table structure for table `crpLabels`
--

CREATE TABLE `crpLabels` (
  `labelID` int(10) NOT NULL,
  `ownerID` int(10) NOT NULL,
  `color` int(10) NOT NULL,
  `name` varchar(130) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Corporate Label Data';

-- --------------------------------------------------------

--
-- Table structure for table `crpLockedItems`
--

CREATE TABLE `crpLockedItems` (
  `itemID` int(10) unsigned NOT NULL,
  `ownerID` int(10) NOT NULL,
  `locationID` int(10) NOT NULL,
  `locked` bit(1) NOT NULL DEFAULT b'0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Corporate Locked Items';

-- --------------------------------------------------------

--
-- Table structure for table `crpMedalData`
--

CREATE TABLE `crpMedalData` (
  `medalID` int(11) NOT NULL,
  `part` tinyint(1) NOT NULL,
  `graphic` varchar(150) COLLATE utf8_unicode_ci NOT NULL,
  `color` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `crpMedals`
--

CREATE TABLE `crpMedals` (
  `medalID` int(11) NOT NULL,
  `ownerID` int(11) NOT NULL DEFAULT '0',
  `creatorID` int(11) NOT NULL DEFAULT '0',
  `noRecepients` int(11) NOT NULL DEFAULT '0',
  `date` bigint(20) NOT NULL DEFAULT '0',
  `title` varchar(50) COLLATE utf8_unicode_ci NOT NULL,
  `description` varchar(150) COLLATE utf8_unicode_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `crpMedalStatus`
--

CREATE TABLE `crpMedalStatus` (
  `ai` int(11) NOT NULL,
  `statusID` int(11) NOT NULL,
  `statusName` varchar(25) COLLATE utf8_unicode_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `crpNPCCorporationDivisions`
--

CREATE TABLE `crpNPCCorporationDivisions` (
  `corporationID` int(11) NOT NULL,
  `divisionID` tinyint(3) unsigned NOT NULL,
  `size` tinyint(4) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `crpNPCCorporationResearchFields`
--

CREATE TABLE `crpNPCCorporationResearchFields` (
  `skillID` int(11) NOT NULL,
  `corporationID` int(11) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `crpNPCCorporations`
--

CREATE TABLE `crpNPCCorporations` (
  `corporationID` int(11) NOT NULL,
  `size` char(1) DEFAULT NULL,
  `extent` char(1) DEFAULT NULL,
  `solarSystemID` int(11) DEFAULT NULL,
  `investorID1` int(11) DEFAULT NULL,
  `investorShares1` tinyint(4) DEFAULT NULL,
  `investorID2` int(11) DEFAULT NULL,
  `investorShares2` tinyint(4) DEFAULT NULL,
  `investorID3` int(11) DEFAULT NULL,
  `investorShares3` tinyint(4) DEFAULT NULL,
  `investorID4` int(11) DEFAULT NULL,
  `investorShares4` tinyint(4) DEFAULT NULL,
  `friendID` int(11) DEFAULT NULL,
  `enemyID` int(11) DEFAULT NULL,
  `publicShares` tinyint(4) DEFAULT NULL,
  `initialPrice` int(11) DEFAULT NULL,
  `minSecurity` double DEFAULT NULL,
  `scattered` tinyint(1) DEFAULT NULL,
  `fringe` tinyint(4) DEFAULT NULL,
  `corridor` tinyint(4) DEFAULT NULL,
  `hub` tinyint(4) DEFAULT NULL,
  `border` tinyint(4) DEFAULT NULL,
  `factionID` int(11) DEFAULT NULL,
  `sizeFactor` double DEFAULT NULL,
  `stationCount` smallint(6) DEFAULT NULL,
  `stationSystemCount` smallint(6) DEFAULT NULL,
  `description` varchar(4000) DEFAULT NULL,
  `iconID` smallint(6) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `crpNPCCorporationTrades`
--

CREATE TABLE `crpNPCCorporationTrades` (
  `corporationID` int(11) NOT NULL,
  `typeID` int(11) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `crpNPCDivisions`
--

CREATE TABLE `crpNPCDivisions` (
  `divisionID` tinyint(3) unsigned NOT NULL,
  `divisionName` varchar(100) DEFAULT NULL,
  `divisionNameID` int(10) DEFAULT NULL,
  `description` varchar(1000) DEFAULT NULL,
  `leaderType` varchar(100) DEFAULT NULL,
  `leaderTypeID` int(10) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `crpNPCWalletDivisons`
--

CREATE TABLE `crpNPCWalletDivisons` (
  `corporationID` int(10) unsigned NOT NULL,
  `balance1` double NOT NULL DEFAULT '0',
  `balance2` double NOT NULL DEFAULT '0',
  `balance3` double NOT NULL DEFAULT '0',
  `balance4` double NOT NULL DEFAULT '0',
  `balance5` double NOT NULL DEFAULT '0',
  `balance6` double NOT NULL DEFAULT '0',
  `balance7` double NOT NULL DEFAULT '0',
  `division1` varchar(100) DEFAULT '1st division',
  `division2` varchar(100) DEFAULT '2nd division',
  `division3` varchar(100) DEFAULT '3rd division',
  `division4` varchar(100) DEFAULT '4th division',
  `division5` varchar(100) DEFAULT '5th division',
  `division6` varchar(100) DEFAULT '6th division',
  `division7` varchar(100) DEFAULT '7th division',
  `walletDivision1` varchar(100) DEFAULT '1st wallet division',
  `walletDivision2` varchar(100) DEFAULT '2nd wallet division',
  `walletDivision3` varchar(100) DEFAULT '3rd wallet division',
  `walletDivision4` varchar(100) DEFAULT '4th wallet division',
  `walletDivision5` varchar(100) DEFAULT '5th wallet division',
  `walletDivision6` varchar(100) DEFAULT '6th wallet division',
  `walletDivision7` varchar(100) DEFAULT '7th wallet division'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='NPC Corporation Wallet Data';

-- --------------------------------------------------------

--
-- Table structure for table `crpRecruiters`
--

CREATE TABLE `crpRecruiters` (
  `adID` smallint(5) NOT NULL,
  `corpID` int(10) NOT NULL,
  `charID` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `crpRoleGroups`
--

CREATE TABLE `crpRoleGroups` (
  `roleGroupID` tinyint(2) NOT NULL,
  `roleGroupName` varchar(130) COLLATE utf8_unicode_ci NOT NULL,
  `roleMask` bigint(20) NOT NULL,
  `appliesTo` varchar(130) COLLATE utf8_unicode_ci NOT NULL,
  `appliesToGrantable` varchar(130) COLLATE utf8_unicode_ci NOT NULL,
  `isLocational` bit(1) NOT NULL DEFAULT b'0',
  `isDivisional` bit(1) NOT NULL DEFAULT b'0',
  `roleGroupNameID` mediumint(6) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `crpRoleHistroy`
--

CREATE TABLE `crpRoleHistroy` (
  `id` int(10) NOT NULL,
  `corporationID` int(10) unsigned NOT NULL DEFAULT '0',
  `characterID` int(10) NOT NULL,
  `issuerID` int(10) unsigned NOT NULL DEFAULT '0',
  `changeTime` bigint(20) NOT NULL DEFAULT '0',
  `oldRoles` bigint(20) NOT NULL DEFAULT '0',
  `newRoles` bigint(20) NOT NULL DEFAULT '0',
  `grantable` tinyint(1) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Role History logging for Corp AuditMember';

-- --------------------------------------------------------

--
-- Table structure for table `crpRoles`
--

CREATE TABLE `crpRoles` (
  `roleIID` smallint(6) NOT NULL,
  `roleID` bigint(20) NOT NULL,
  `roleName` varchar(100) COLLATE utf8_unicode_ci NOT NULL,
  `shortDescription` varchar(130) COLLATE utf8_unicode_ci NOT NULL,
  `shortDescriptionID` mediumint(6) NOT NULL,
  `description` varchar(130) COLLATE utf8_unicode_ci NOT NULL,
  `descriptionID` mediumint(6) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `crpRoleTitles`
--

CREATE TABLE `crpRoleTitles` (
  `corporationID` int(10) NOT NULL,
  `titleID` int(10) NOT NULL,
  `titleName` varchar(130) COLLATE utf8_unicode_ci NOT NULL,
  `roles` bigint(20) NOT NULL DEFAULT '0',
  `grantableRoles` bigint(20) NOT NULL DEFAULT '0',
  `rolesAtHQ` bigint(20) NOT NULL DEFAULT '0',
  `grantableRolesAtHQ` bigint(20) NOT NULL DEFAULT '0',
  `rolesAtBase` bigint(20) NOT NULL DEFAULT '0',
  `grantableRolesAtBase` bigint(20) DEFAULT '0',
  `rolesAtOther` bigint(20) NOT NULL DEFAULT '0',
  `grantableRolesAtOther` bigint(20) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `crpShares`
--

CREATE TABLE `crpShares` (
  `id` int(11) NOT NULL,
  `shareholderID` int(10) unsigned NOT NULL DEFAULT '0',
  `corporationID` int(10) unsigned NOT NULL DEFAULT '0',
  `shares` int(10) unsigned NOT NULL DEFAULT '0',
  `shareholderCorporationID` int(10) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `crpVoteItems`
--

CREATE TABLE `crpVoteItems` (
  `voteCaseID` int(11) NOT NULL,
  `corporationID` int(10) unsigned NOT NULL,
  `voteType` int(10) unsigned NOT NULL DEFAULT '0',
  `voteCaseText` varchar(50) NOT NULL DEFAULT 'No Label',
  `description` varchar(150) NOT NULL DEFAULT 'No Description',
  `inEffect` tinyint(1) unsigned NOT NULL DEFAULT '1',
  `status` tinyint(2) NOT NULL DEFAULT '2',
  `startDateTime` bigint(20) NOT NULL,
  `endDateTime` bigint(20) NOT NULL DEFAULT '0',
  `actedUpon` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `timeActedUpon` bigint(20) NOT NULL DEFAULT '0',
  `rescended` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `timeRescended` bigint(20) NOT NULL DEFAULT '0',
  `votesMade` smallint(5) NOT NULL DEFAULT '0',
  `votesProxied` smallint(5) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='GetVoteCasesByCorporation';

-- --------------------------------------------------------

--
-- Table structure for table `crpVoteOptions`
--

CREATE TABLE `crpVoteOptions` (
  `ai` int(11) NOT NULL,
  `voteCaseID` int(11) NOT NULL DEFAULT '0',
  `optionID` tinyint(2) DEFAULT NULL,
  `optionText` varchar(150) COLLATE utf8_unicode_ci NOT NULL,
  `parameter` int(10) DEFAULT NULL,
  `parameter1` int(10) DEFAULT NULL,
  `parameter2` int(10) DEFAULT NULL,
  `votesFor` int(10) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='Corp Vote Options';

-- --------------------------------------------------------

--
-- Table structure for table `crpVotes`
--

CREATE TABLE `crpVotes` (
  `charID` int(11) NOT NULL,
  `corpID` int(11) NOT NULL,
  `voteCaseID` mediumint(11) NOT NULL,
  `optionID` tinyint(1) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `crpWalletDivisons`
--

CREATE TABLE `crpWalletDivisons` (
  `corporationID` int(10) unsigned NOT NULL,
  `balance1` float NOT NULL DEFAULT '0',
  `balance2` float NOT NULL DEFAULT '0',
  `balance3` float NOT NULL DEFAULT '0',
  `balance4` float NOT NULL DEFAULT '0',
  `balance5` float NOT NULL DEFAULT '0',
  `balance6` float NOT NULL DEFAULT '0',
  `balance7` float NOT NULL DEFAULT '0',
  `division1` varchar(100) DEFAULT '1st division',
  `division2` varchar(100) DEFAULT '2nd division',
  `division3` varchar(100) DEFAULT '3rd division',
  `division4` varchar(100) DEFAULT '4th division',
  `division5` varchar(100) DEFAULT '5th division',
  `division6` varchar(100) DEFAULT '6th division',
  `division7` varchar(100) DEFAULT '7th division',
  `walletDivision1` varchar(100) DEFAULT 'Master Wallet',
  `walletDivision2` varchar(100) DEFAULT '2nd wallet division',
  `walletDivision3` varchar(100) DEFAULT '3rd wallet division',
  `walletDivision4` varchar(100) DEFAULT '4th wallet division',
  `walletDivision5` varchar(100) DEFAULT '5th wallet division',
  `walletDivision6` varchar(100) DEFAULT '6th wallet division',
  `walletDivision7` varchar(100) DEFAULT '7th wallet division'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Corporation Wallet Data';

-- --------------------------------------------------------

--
-- Table structure for table `crtCategories`
--

CREATE TABLE `crtCategories` (
  `categoryID` tinyint(3) unsigned NOT NULL,
  `description` varchar(500) DEFAULT NULL,
  `categoryName` varchar(256) DEFAULT NULL,
  `categoryNameID` int(10) unsigned DEFAULT '0',
  `dataID` int(10) unsigned DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `crtCertificates`
--

CREATE TABLE `crtCertificates` (
  `certificateID` int(11) NOT NULL,
  `categoryID` tinyint(3) unsigned DEFAULT NULL,
  `classID` int(11) DEFAULT NULL,
  `grade` tinyint(4) DEFAULT NULL,
  `corpID` int(11) DEFAULT NULL,
  `iconID` smallint(6) DEFAULT NULL,
  `description` varchar(500) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `crtClasses`
--

CREATE TABLE `crtClasses` (
  `classID` int(3) unsigned NOT NULL DEFAULT '0',
  `className` varchar(150) COLLATE utf8_unicode_ci DEFAULT NULL,
  `description` text COLLATE utf8_unicode_ci,
  `classNameID` int(6) unsigned DEFAULT '0',
  `dataID` int(6) unsigned DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `crtRecommendations`
--

CREATE TABLE `crtRecommendations` (
  `recommendationID` int(11) NOT NULL,
  `shipTypeID` int(11) DEFAULT NULL,
  `certificateID` int(11) DEFAULT NULL,
  `recommendationLevel` tinyint(4) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `crtRelationships`
--

CREATE TABLE `crtRelationships` (
  `relationshipID` int(11) NOT NULL,
  `parentID` int(11) DEFAULT NULL,
  `parentTypeID` int(11) DEFAULT NULL,
  `parentLevel` tinyint(4) DEFAULT NULL,
  `childID` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `CruciblePriceHistory`
--

CREATE TABLE `CruciblePriceHistory` (
  `regionID` int(10) unsigned NOT NULL DEFAULT '0',
  `typeID` int(10) unsigned NOT NULL DEFAULT '0',
  `entryDate` date DEFAULT NULL,
  `lowPrice` double NOT NULL DEFAULT '0',
  `highPrice` double NOT NULL DEFAULT '0',
  `avgPrice` double NOT NULL DEFAULT '0',
  `volume` int(10) unsigned NOT NULL DEFAULT '0',
  `orders` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `CruciblePriceHistory_Materials`
--

CREATE TABLE `CruciblePriceHistory_Materials` (
  `regionID` int(10) unsigned NOT NULL DEFAULT '0',
  `typeID` int(10) unsigned NOT NULL DEFAULT '0',
  `entryDate` date DEFAULT NULL,
  `lowPrice` double NOT NULL DEFAULT '0',
  `highPrice` double NOT NULL DEFAULT '0',
  `avgPrice` double NOT NULL DEFAULT '0',
  `volume` int(10) unsigned NOT NULL DEFAULT '0',
  `orders` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `dgmAttributeCategories`
--

CREATE TABLE `dgmAttributeCategories` (
  `categoryID` tinyint(3) unsigned NOT NULL,
  `categoryName` varchar(50) DEFAULT NULL,
  `categoryDescription` varchar(200) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `dgmAttributeTypes`
--

CREATE TABLE `dgmAttributeTypes` (
  `attributeID` smallint(6) NOT NULL,
  `attributeName` varchar(100) DEFAULT NULL,
  `attributeCategory` smallint(6) DEFAULT NULL,
  `description` varchar(1000) DEFAULT NULL,
  `maxAttributeID` smallint(6) DEFAULT NULL,
  `attributeIdx` tinyint(3) DEFAULT NULL,
  `chargeRechargeTimeID` smallint(6) DEFAULT NULL,
  `defaultValue` double DEFAULT NULL,
  `published` tinyint(1) DEFAULT NULL,
  `displayName` varchar(100) NOT NULL,
  `displayNameID` mediumint(6) DEFAULT NULL,
  `unitID` tinyint(3) unsigned DEFAULT NULL,
  `stackable` tinyint(1) DEFAULT NULL,
  `highIsGood` tinyint(1) DEFAULT NULL,
  `dataID` int(11) DEFAULT NULL,
  `iconID` int(11) DEFAULT NULL,
  `categoryID` tinyint(3) unsigned DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `dgmEffects`
--

CREATE TABLE `dgmEffects` (
  `effectID` smallint(6) NOT NULL,
  `effectName` varchar(400) NOT NULL DEFAULT '0',
  `effectCategory` smallint(6) NOT NULL DEFAULT '0',
  `preExpression` int(11) NOT NULL DEFAULT '0',
  `postExpression` int(11) NOT NULL DEFAULT '0',
  `description` varchar(1000) NOT NULL,
  `descriptionID` mediumint(7) NOT NULL DEFAULT '0',
  `guid` varchar(60) NOT NULL,
  `isOffensive` tinyint(1) NOT NULL DEFAULT '0',
  `isAssistance` tinyint(1) NOT NULL DEFAULT '0',
  `durationAttributeID` smallint(6) DEFAULT NULL,
  `trackingSpeedAttributeID` smallint(6) DEFAULT NULL,
  `dischargeAttributeID` smallint(6) DEFAULT NULL,
  `rangeAttributeID` smallint(6) DEFAULT NULL,
  `falloffAttributeID` smallint(6) DEFAULT NULL,
  `disallowAutoRepeat` tinyint(1) NOT NULL DEFAULT '0',
  `published` tinyint(1) NOT NULL DEFAULT '0',
  `displayName` varchar(100) NOT NULL,
  `displayNameID` mediumint(7) NOT NULL DEFAULT '0',
  `isWarpSafe` tinyint(1) NOT NULL DEFAULT '0',
  `rangeChance` tinyint(1) NOT NULL DEFAULT '0',
  `electronicChance` tinyint(1) NOT NULL DEFAULT '0',
  `propulsionChance` tinyint(1) NOT NULL DEFAULT '0',
  `distribution` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `sfxName` varchar(20) NOT NULL,
  `npcUsageChanceAttributeID` smallint(6) NOT NULL DEFAULT '0',
  `npcActivationChanceAttributeID` smallint(6) NOT NULL DEFAULT '0',
  `fittingUsageChanceAttributeID` smallint(6) NOT NULL DEFAULT '0',
  `iconID` int(11) NOT NULL DEFAULT '0',
  `dataID` int(10) NOT NULL DEFAULT '0',
  `modifierInfo` varchar(500) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `dgmExpressions`
--

CREATE TABLE `dgmExpressions` (
  `expressionID` int(11) NOT NULL,
  `operandID` smallint(3) unsigned NOT NULL,
  `arg1` smallint(5) unsigned NOT NULL,
  `arg2` smallint(5) unsigned NOT NULL,
  `expressionValue` varchar(100) NOT NULL,
  `description` varchar(1000) NOT NULL,
  `expressionName` varchar(500) NOT NULL,
  `expressionTypeID` smallint(5) unsigned NOT NULL,
  `expressionGroupID` smallint(5) unsigned NOT NULL,
  `expressionAttributeID` smallint(5) unsigned NOT NULL
) ENGINE=Aria DEFAULT CHARSET=utf8 PAGE_CHECKSUM=0 TRANSACTIONAL=0;

-- --------------------------------------------------------

--
-- Table structure for table `dgmOperands`
--

CREATE TABLE `dgmOperands` (
  `operandID` tinyint(3) unsigned NOT NULL,
  `operandKey` varchar(50) NOT NULL,
  `description` varchar(100) NOT NULL,
  `format` varchar(100) NOT NULL,
  `arg1categoryID` smallint(6) NOT NULL,
  `arg2categoryID` smallint(6) NOT NULL,
  `resultCategoryID` smallint(6) NOT NULL,
  `pythonFormat` varchar(100) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `dgmTypeAttributes`
--

CREATE TABLE `dgmTypeAttributes` (
  `typeID` int(10) NOT NULL,
  `attributeID` smallint(5) NOT NULL,
  `valueInt` int(10) DEFAULT NULL,
  `valueFloat` double DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `dgmTypeEffects`
--

CREATE TABLE `dgmTypeEffects` (
  `typeID` smallint(5) unsigned NOT NULL DEFAULT '0',
  `effectID` smallint(5) NOT NULL,
  `isDefault` bit(1) NOT NULL DEFAULT b'0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `dgmUnits`
--

CREATE TABLE `dgmUnits` (
  `unitID` tinyint(3) unsigned NOT NULL,
  `unitName` varchar(100) DEFAULT NULL,
  `displayName` varchar(50) DEFAULT NULL,
  `description` varchar(1000) DEFAULT NULL,
  `displayNameID` mediumint(7) NOT NULL DEFAULT '0',
  `descriptionID` mediumint(7) NOT NULL DEFAULT '0',
  `dataID` int(8) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `droneState`
--

CREATE TABLE `droneState` (
  `droneID` int(10) unsigned NOT NULL DEFAULT '0',
  `solarSystemID` int(10) unsigned NOT NULL DEFAULT '0',
  `ownerID` int(10) unsigned NOT NULL DEFAULT '0',
  `controllerID` int(10) unsigned NOT NULL DEFAULT '0',
  `activityState` int(10) unsigned NOT NULL DEFAULT '0',
  `typeID` int(10) unsigned NOT NULL DEFAULT '0',
  `controllerOwnerID` int(10) unsigned NOT NULL DEFAULT '0',
  `targetID` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `dunActive`
--

CREATE TABLE `dunActive` (
  `systemID` int(11) NOT NULL,
  `dungeonID` int(11) NOT NULL,
  `dunTemplateID` int(11) NOT NULL,
  `dunExpiryTime` bigint(20) NOT NULL DEFAULT '0',
  `state` int(11) NOT NULL DEFAULT '0',
  `xpos` float NOT NULL DEFAULT '0',
  `ypos` float NOT NULL DEFAULT '0',
  `zpos` float NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Table structure for table `dunEntryData`
--

CREATE TABLE `dunEntryData` (
  `dunEntryID` int(11) NOT NULL,
  `dunEntryName` varchar(85) COLLATE utf8_bin NOT NULL,
  `xpos` int(11) NOT NULL DEFAULT '0',
  `ypos` int(11) NOT NULL DEFAULT '0',
  `zpos` int(11) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Table structure for table `dunGroupData`
--

CREATE TABLE `dunGroupData` (
  `ai` int(11) NOT NULL,
  `dunGroupID` int(11) NOT NULL,
  `itemTypeID` int(11) NOT NULL,
  `itemGroupID` int(11) NOT NULL DEFAULT '0',
  `xpos` int(11) NOT NULL DEFAULT '0',
  `ypos` int(11) NOT NULL DEFAULT '0',
  `zpos` int(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Table structure for table `dunRoomData`
--

CREATE TABLE `dunRoomData` (
  `ai` int(11) NOT NULL,
  `dunRoomID` int(11) NOT NULL DEFAULT '0',
  `dunGroupID` int(11) NOT NULL DEFAULT '0',
  `xpos` int(11) NOT NULL DEFAULT '0',
  `ypos` int(11) NOT NULL DEFAULT '0',
  `zpos` int(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Table structure for table `dunSpawnType`
--

CREATE TABLE `dunSpawnType` (
  `dunSpawnTypeID` int(11) NOT NULL,
  `dunSpawnName` varchar(85) COLLATE utf8_bin NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Table structure for table `dunTemplates`
--

CREATE TABLE `dunTemplates` (
  `dunTemplateID` int(11) NOT NULL,
  `dunTemplateName` varchar(85) COLLATE utf8_bin NOT NULL,
  `dunEntryID` int(11) NOT NULL DEFAULT '0',
  `dunSpawnID` int(11) NOT NULL DEFAULT '0',
  `dunRoomID` int(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Table structure for table `entity`
--

CREATE TABLE `entity` (
  `itemID` int(10) unsigned NOT NULL,
  `itemName` varchar(85) NOT NULL DEFAULT '',
  `typeID` int(10) unsigned NOT NULL DEFAULT '0',
  `ownerID` int(10) unsigned NOT NULL DEFAULT '0',
  `locationID` int(10) unsigned NOT NULL DEFAULT '0',
  `flag` int(4) unsigned NOT NULL DEFAULT '0',
  `contraband` tinyint(1) NOT NULL DEFAULT '0',
  `singleton` tinyint(1) NOT NULL DEFAULT '0',
  `quantity` int(10) NOT NULL DEFAULT '0',
  `x` double NOT NULL DEFAULT '0',
  `y` double NOT NULL DEFAULT '0',
  `z` double NOT NULL DEFAULT '0',
  `customInfo` text
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `entity_attributes`
--

CREATE TABLE `entity_attributes` (
  `itemID` int(10) unsigned NOT NULL DEFAULT '0',
  `attributeID` int(10) unsigned NOT NULL DEFAULT '0',
  `valueInt` bigint(20) DEFAULT NULL,
  `valueFloat` double DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `eveGraphics`
--

CREATE TABLE `eveGraphics` (
  `graphicID` smallint(6) NOT NULL DEFAULT '0',
  `graphicFile` varchar(500) NOT NULL,
  `description` varchar(16000) NOT NULL,
  `obsolete` tinyint(1) NOT NULL,
  `graphicType` varchar(100) DEFAULT NULL,
  `collidable` tinyint(1) DEFAULT NULL,
  `explosionID` smallint(6) DEFAULT NULL,
  `directoryID` int(11) DEFAULT NULL,
  `graphicName` varchar(64) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `eveIcons`
--

CREATE TABLE `eveIcons` (
  `iconID` smallint(6) NOT NULL DEFAULT '0',
  `iconFile` varchar(500) NOT NULL,
  `description` varchar(16000) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `eveMail`
--

CREATE TABLE `eveMail` (
  `channelID` int(10) unsigned NOT NULL DEFAULT '0',
  `messageID` int(10) unsigned NOT NULL,
  `senderID` int(10) unsigned NOT NULL DEFAULT '0',
  `subject` varchar(255) NOT NULL DEFAULT '',
  `created` bigint(20) unsigned NOT NULL DEFAULT '0',
  `read` tinyint(3) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `eveMailDetails`
--

CREATE TABLE `eveMailDetails` (
  `attachmentID` int(10) unsigned NOT NULL,
  `messageID` int(10) unsigned NOT NULL DEFAULT '0',
  `mimeTypeID` int(10) unsigned NOT NULL DEFAULT '0',
  `attachment` longtext NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `eveMailMimeType`
--

CREATE TABLE `eveMailMimeType` (
  `mimeTypeID` int(10) unsigned NOT NULL,
  `mimeType` text NOT NULL,
  `binary` tinyint(3) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `eveStaticLocations`
--

CREATE TABLE `eveStaticLocations` (
  `locationID` int(10) unsigned NOT NULL DEFAULT '0',
  `locationName` mediumtext NOT NULL,
  `x` double NOT NULL DEFAULT '0',
  `y` double NOT NULL DEFAULT '0',
  `z` double NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `eveStaticOwners`
--

CREATE TABLE `eveStaticOwners` (
  `ownerID` int(10) unsigned NOT NULL DEFAULT '0',
  `ownerName` varchar(100) NOT NULL DEFAULT '',
  `typeID` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `eveUnits`
--

CREATE TABLE `eveUnits` (
  `unitID` tinyint(3) unsigned NOT NULL,
  `unitName` varchar(100) DEFAULT NULL,
  `displayName` varchar(50) DEFAULT NULL,
  `description` varchar(1000) DEFAULT NULL,
  `displayNameID` mediumint(7) NOT NULL DEFAULT '0',
  `descriptionID` mediumint(7) NOT NULL DEFAULT '0',
  `dataID` int(8) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `facFactions`
--

CREATE TABLE `facFactions` (
  `factionID` int(11) NOT NULL,
  `factionName` varchar(100) DEFAULT NULL,
  `description` varchar(1000) DEFAULT NULL,
  `raceIDs` int(11) DEFAULT NULL,
  `solarSystemID` int(11) DEFAULT NULL,
  `corporationID` int(11) DEFAULT NULL,
  `sizeFactor` double DEFAULT NULL,
  `stationCount` smallint(6) DEFAULT NULL,
  `stationSystemCount` smallint(6) DEFAULT NULL,
  `militiaCorporationID` int(11) DEFAULT NULL,
  `iconID` smallint(6) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `facRaces`
--

CREATE TABLE `facRaces` (
  `factionID` int(10) unsigned NOT NULL DEFAULT '0',
  `raceID` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `facSalvage`
--

CREATE TABLE `facSalvage` (
  `factionID` int(10) unsigned NOT NULL,
  `techLvl` tinyint(3) unsigned NOT NULL,
  `itemID` int(10) unsigned NOT NULL,
  `itemName` varchar(45) COLLATE utf8_unicode_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `facWarSystems`
--

CREATE TABLE `facWarSystems` (
  `systemID` int(10) NOT NULL,
  `occupierID` int(10) NOT NULL,
  `factionID` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Faction War Systems';

-- --------------------------------------------------------

--
-- Table structure for table `graphics`
--

CREATE TABLE `graphics` (
  `graphicID` int(11) NOT NULL DEFAULT '0',
  `graphicFile` text COLLATE utf8_unicode_ci,
  `graphicName` text COLLATE utf8_unicode_ci,
  `description` text COLLATE utf8_unicode_ci,
  `obsolete` bit(1) DEFAULT NULL,
  `graphicType` text COLLATE utf8_unicode_ci,
  `collisionFile` text COLLATE utf8_unicode_ci,
  `paperdollFile` text COLLATE utf8_unicode_ci,
  `animationTemplate` int(11) DEFAULT NULL,
  `collidable` bit(1) DEFAULT NULL,
  `explosionID` int(11) DEFAULT NULL,
  `directoryID` int(11) DEFAULT NULL,
  `graphicMinX` double DEFAULT NULL,
  `graphicMinY` double DEFAULT NULL,
  `graphicMinZ` double DEFAULT NULL,
  `graphicMaxX` double DEFAULT NULL,
  `graphicMaxY` double DEFAULT NULL,
  `graphicMaxZ` double DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `icons`
--

CREATE TABLE `icons` (
  `iconID` int(11) NOT NULL DEFAULT '0',
  `iconFile` text COLLATE utf8_unicode_ci,
  `description` text COLLATE utf8_unicode_ci,
  `obsolete` bit(1) DEFAULT NULL,
  `iconType` text COLLATE utf8_unicode_ci
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `intro`
--

CREATE TABLE `intro` (
  `langID` varchar(2) NOT NULL,
  `textgroup` int(10) unsigned NOT NULL,
  `textLabel` varchar(10) NOT NULL,
  `text` text NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `invBlueprints`
--

CREATE TABLE `invBlueprints` (
  `itemID` int(10) unsigned NOT NULL,
  `copy` bit(1) NOT NULL DEFAULT b'0',
  `mLevel` tinyint(3) NOT NULL DEFAULT '-1',
  `pLevel` tinyint(3) NOT NULL DEFAULT '-1',
  `runs` smallint(5) NOT NULL DEFAULT '-1'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `invBlueprintTypes`
--

CREATE TABLE `invBlueprintTypes` (
  `blueprintTypeID` int(11) NOT NULL DEFAULT '0',
  `parentBlueprintTypeID` int(11) NOT NULL DEFAULT '0',
  `productTypeID` int(11) NOT NULL DEFAULT '0',
  `productionTime` int(11) NOT NULL DEFAULT '0',
  `techLevel` smallint(6) NOT NULL DEFAULT '0',
  `researchProductivityTime` int(11) NOT NULL DEFAULT '0',
  `researchMaterialTime` int(11) NOT NULL DEFAULT '0',
  `researchCopyTime` int(11) NOT NULL DEFAULT '0',
  `researchTechTime` int(11) NOT NULL DEFAULT '0',
  `productivityModifier` int(11) NOT NULL DEFAULT '0',
  `materialModifier` smallint(6) NOT NULL DEFAULT '0',
  `wasteFactor` smallint(6) NOT NULL DEFAULT '0',
  `chanceOfRE` double NOT NULL DEFAULT '0',
  `maxProductionLimit` int(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `invCategories`
--

CREATE TABLE `invCategories` (
  `categoryID` int(10) NOT NULL,
  `categoryName` varchar(100) DEFAULT NULL,
  `description` varchar(3000) DEFAULT NULL,
  `published` bit(1) NOT NULL DEFAULT b'0',
  `iconID` smallint(6) NOT NULL DEFAULT '0',
  `categoryNameID` int(8) NOT NULL DEFAULT '0',
  `dataID` int(8) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `invContrabandTypes`
--

CREATE TABLE `invContrabandTypes` (
  `factionID` int(11) NOT NULL,
  `typeID` int(11) NOT NULL,
  `standingLoss` double DEFAULT NULL,
  `confiscateMinSec` double DEFAULT NULL,
  `fineByValue` double DEFAULT NULL,
  `attackMinSec` double DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `invControlTowerResourcePurposes`
--

CREATE TABLE `invControlTowerResourcePurposes` (
  `purpose` tinyint(4) NOT NULL,
  `purposeText` varchar(100) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `invControlTowerResources`
--

CREATE TABLE `invControlTowerResources` (
  `controlTowerTypeID` int(11) NOT NULL,
  `resourceTypeID` int(11) NOT NULL,
  `purpose` tinyint(4) NOT NULL,
  `quantity` int(11) NOT NULL,
  `minSecurityLevel` double DEFAULT NULL,
  `factionID` int(11) DEFAULT NULL,
  `wormholeClassID` tinyint(2) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `invFlags`
--

CREATE TABLE `invFlags` (
  `flagID` smallint(6) NOT NULL,
  `flagName` varchar(200) DEFAULT NULL,
  `flagText` varchar(100) DEFAULT NULL,
  `orderID` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `invGroups`
--

CREATE TABLE `invGroups` (
  `groupID` int(11) NOT NULL,
  `categoryID` int(11) DEFAULT NULL,
  `groupName` varchar(100) DEFAULT NULL,
  `description` varchar(3000) DEFAULT NULL,
  `iconID` smallint(6) DEFAULT NULL,
  `useBasePrice` bit(1) NOT NULL DEFAULT b'0',
  `allowManufacture` bit(1) NOT NULL DEFAULT b'0',
  `allowRecycler` bit(1) NOT NULL DEFAULT b'0',
  `anchored` bit(1) NOT NULL DEFAULT b'0',
  `anchorable` bit(1) NOT NULL DEFAULT b'0',
  `fittableNonSingleton` bit(1) NOT NULL DEFAULT b'0',
  `published` bit(1) NOT NULL DEFAULT b'0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `invMarketGroups`
--

CREATE TABLE `invMarketGroups` (
  `parentGroupID` int(11) DEFAULT NULL,
  `marketGroupID` int(11) NOT NULL DEFAULT '0',
  `marketGroupName` text COLLATE utf8_unicode_ci,
  `description` text COLLATE utf8_unicode_ci,
  `graphicID` int(11) DEFAULT NULL,
  `hasTypes` bit(1) NOT NULL DEFAULT b'0',
  `iconID` int(11) DEFAULT NULL,
  `dataID` int(11) DEFAULT NULL,
  `marketGroupNameID` int(11) DEFAULT NULL,
  `descriptionID` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `invMetaGroups`
--

CREATE TABLE `invMetaGroups` (
  `metaGroupID` smallint(6) NOT NULL,
  `metaGroupName` varchar(100) DEFAULT NULL,
  `description` varchar(1000) DEFAULT NULL,
  `iconID` smallint(6) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `invMetaTypes`
--

CREATE TABLE `invMetaTypes` (
  `typeID` int(11) NOT NULL,
  `parentTypeID` int(11) DEFAULT NULL,
  `metaGroupID` smallint(6) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `invTypeMaterials`
--

CREATE TABLE `invTypeMaterials` (
  `typeID` int(10) NOT NULL,
  `materialTypeID` int(10) NOT NULL,
  `quantity` int(10) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `invTypeReactions`
--

CREATE TABLE `invTypeReactions` (
  `reactionTypeID` int(11) NOT NULL,
  `input` tinyint(1) NOT NULL,
  `typeID` int(11) NOT NULL,
  `quantity` smallint(6) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `invTypes`
--

CREATE TABLE `invTypes` (
  `typeID` smallint(5) NOT NULL DEFAULT '0',
  `groupID` smallint(5) unsigned NOT NULL DEFAULT '0',
  `typeName` text COLLATE utf8_unicode_ci,
  `description` text COLLATE utf8_unicode_ci,
  `graphicID` int(11) DEFAULT NULL,
  `radius` float NOT NULL DEFAULT '0',
  `mass` float NOT NULL DEFAULT '0',
  `volume` float NOT NULL DEFAULT '0',
  `capacity` float NOT NULL DEFAULT '0',
  `portionSize` int(11) DEFAULT NULL,
  `raceID` tinyint(2) unsigned NOT NULL DEFAULT '0',
  `basePrice` float NOT NULL DEFAULT '0',
  `published` bit(1) NOT NULL DEFAULT b'0',
  `marketGroupID` int(11) DEFAULT NULL,
  `chanceOfDuplicating` float NOT NULL DEFAULT '0',
  `copyTypeID` int(5) NOT NULL DEFAULT '0',
  `soundID` int(11) DEFAULT NULL,
  `iconID` int(11) DEFAULT NULL,
  `dataID` int(11) DEFAULT NULL,
  `typeNameID` int(11) DEFAULT NULL,
  `descriptionID` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `invTypesToWrecks`
--

CREATE TABLE `invTypesToWrecks` (
  `typeID` int(11) NOT NULL,
  `typeName` text,
  `wreckTypeID` int(11) NOT NULL,
  `wreckName` text NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `jnlCharacters`
--

CREATE TABLE `jnlCharacters` (
  `transactionID` int(10) unsigned NOT NULL,
  `ownerID` int(10) unsigned NOT NULL DEFAULT '0',
  `entryTypeID` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `referenceID` int(10) unsigned NOT NULL DEFAULT '0',
  `ownerID1` int(10) unsigned NOT NULL DEFAULT '0',
  `ownerID2` int(10) unsigned NOT NULL DEFAULT '0',
  `transactionDate` bigint(20) DEFAULT NULL,
  `accountKey` smallint(5) unsigned NOT NULL DEFAULT '0',
  `currency` tinyint(1) NOT NULL DEFAULT '1',
  `amount` double NOT NULL DEFAULT '0',
  `balance` double NOT NULL DEFAULT '0',
  `description` text
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `jnlCorporations`
--

CREATE TABLE `jnlCorporations` (
  `transactionID` int(10) unsigned NOT NULL,
  `ownerID` int(10) unsigned NOT NULL DEFAULT '0',
  `entryTypeID` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `accountKey` smallint(5) unsigned NOT NULL DEFAULT '0',
  `transactionDate` bigint(20) DEFAULT NULL,
  `ownerID1` int(10) unsigned NOT NULL DEFAULT '0',
  `ownerID2` int(10) unsigned NOT NULL DEFAULT '0',
  `referenceID` int(10) unsigned NOT NULL DEFAULT '0',
  `currency` tinyint(1) NOT NULL DEFAULT '1',
  `amount` double NOT NULL DEFAULT '0',
  `balance` double NOT NULL DEFAULT '0',
  `description` text
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Corporation Journal Data';

-- --------------------------------------------------------

--
-- Table structure for table `jnlEntryTypeIDs`
--

CREATE TABLE `jnlEntryTypeIDs` (
  `entryTypeID` smallint(5) unsigned NOT NULL DEFAULT '0',
  `entryTypeName` varchar(100) NOT NULL DEFAULT '',
  `entryTypeNameID` mediumint(6) NOT NULL DEFAULT '0',
  `description` mediumtext NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `languages`
--

CREATE TABLE `languages` (
  `languageID` varchar(2) NOT NULL DEFAULT '',
  `languageName` varchar(100) NOT NULL DEFAULT '',
  `translatedLanguageName` varchar(22) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `liveupdates`
--

CREATE TABLE `liveupdates` (
  `updateID` int(11) NOT NULL DEFAULT '0',
  `updateName` varchar(100) CHARACTER SET latin1 DEFAULT NULL,
  `description` varchar(100) CHARACTER SET latin1 DEFAULT NULL,
  `machoVersionMin` int(11) DEFAULT NULL,
  `machoVersionMax` int(11) DEFAULT NULL,
  `buildNumberMin` int(11) DEFAULT NULL,
  `buildNumberMax` int(11) DEFAULT NULL,
  `methodName` varchar(100) CHARACTER SET latin1 DEFAULT NULL,
  `objectID` varchar(100) CHARACTER SET latin1 DEFAULT NULL,
  `codeType` varchar(100) CHARACTER SET latin1 DEFAULT NULL,
  `code` blob
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Table structure for table `locationScenes`
--

CREATE TABLE `locationScenes` (
  `locationID` int(11) NOT NULL DEFAULT '0',
  `sceneID` tinyint(3) unsigned DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `lootGroup`
--

CREATE TABLE `lootGroup` (
  `npcGroupID` int(11) DEFAULT NULL,
  `npcGroupName` text COLLATE utf8_bin,
  `groupDropChance` float(6,4) NOT NULL,
  `itemGroupID` int(11) DEFAULT NULL,
  `itemGroupName` text COLLATE utf8_bin
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Table structure for table `lootItemGroup`
--

CREATE TABLE `lootItemGroup` (
  `itemGroupID` int(11) DEFAULT NULL,
  `itemGroupName` text COLLATE utf8_bin,
  `itemID` int(11) DEFAULT NULL,
  `itemName` text COLLATE utf8_bin,
  `itemMetaLevel` int(11) DEFAULT NULL,
  `itemDropChance` float(6,4) NOT NULL,
  `minAmount` int(11) DEFAULT NULL,
  `maxAmount` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Table structure for table `lpRequiredItems`
--

CREATE TABLE `lpRequiredItems` (
  `parentID` int(10) NOT NULL,
  `typeID` int(10) NOT NULL,
  `quantity` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `lpStore`
--

CREATE TABLE `lpStore` (
  `storeID` int(5) unsigned NOT NULL,
  `corporationID` int(10) NOT NULL,
  `typeID` int(10) NOT NULL,
  `quantity` int(10) NOT NULL,
  `lpCost` int(10) NOT NULL,
  `iskCost` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `lpVerified`
--

CREATE TABLE `lpVerified` (
  `corporationID` int(10) NOT NULL,
  `verification` smallint(2) NOT NULL,
  `verifiedWith` int(10) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `mailLabel`
--

CREATE TABLE `mailLabel` (
  `id` int(10) NOT NULL,
  `bit` int(10) NOT NULL DEFAULT '0',
  `name` varchar(100) DEFAULT '0',
  `color` int(11) DEFAULT '0',
  `ownerID` bigint(20) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `mailList`
--

CREATE TABLE `mailList` (
  `id` int(10) unsigned NOT NULL,
  `displayName` varchar(30) COLLATE utf8_unicode_ci NOT NULL,
  `defaultAccess` tinyint(4) unsigned NOT NULL,
  `defaultMemberAccess` tinyint(4) unsigned NOT NULL,
  `cost` int(10) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `mailListUsers`
--

CREATE TABLE `mailListUsers` (
  `listID` int(10) unsigned NOT NULL,
  `characterID` int(10) unsigned NOT NULL,
  `role` tinyint(1) NOT NULL,
  `access` tinyint(1) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `mailMessage`
--

CREATE TABLE `mailMessage` (
  `messageID` int(10) unsigned NOT NULL,
  `senderID` int(10) unsigned NOT NULL,
  `toCharacterIDs` text COLLATE utf8_unicode_ci,
  `toListID` int(10) unsigned DEFAULT NULL,
  `toCorpOrAllianceID` int(10) unsigned DEFAULT NULL,
  `title` text COLLATE utf8_unicode_ci,
  `body` blob,
  `sentDate` bigint(20) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

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

-- --------------------------------------------------------

--
-- Table structure for table `mapCelestialDescriptions`
--

CREATE TABLE `mapCelestialDescriptions` (
  `celestialID` int(11) NOT NULL DEFAULT '0',
  `description` text COLLATE utf8_unicode_ci
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `mapCelestialStatistics`
--

CREATE TABLE `mapCelestialStatistics` (
  `celestialID` int(11) NOT NULL,
  `temperature` double DEFAULT NULL,
  `spectralClass` varchar(10) DEFAULT NULL,
  `luminosity` double DEFAULT NULL,
  `age` double DEFAULT NULL,
  `life` double DEFAULT NULL,
  `orbitRadius` double DEFAULT NULL,
  `eccentricity` double DEFAULT NULL,
  `massDust` double DEFAULT NULL,
  `massGas` double DEFAULT NULL,
  `fragmented` tinyint(1) DEFAULT NULL,
  `density` double DEFAULT NULL,
  `surfaceGravity` double DEFAULT NULL,
  `escapeVelocity` double DEFAULT NULL,
  `orbitPeriod` double DEFAULT NULL,
  `rotationRate` double DEFAULT NULL,
  `locked` tinyint(1) DEFAULT NULL,
  `pressure` double DEFAULT NULL,
  `radius` double DEFAULT NULL,
  `mass` double DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `mapConnections`
--

CREATE TABLE `mapConnections` (
  `ai` int(11) NOT NULL,
  `ctype` int(10) unsigned NOT NULL,
  `fromreg` int(10) unsigned NOT NULL,
  `fromcon` int(10) unsigned DEFAULT NULL,
  `fromsol` int(10) unsigned DEFAULT NULL,
  `stargateID` int(10) unsigned DEFAULT NULL,
  `celestialID` int(10) unsigned DEFAULT NULL,
  `tosol` int(10) unsigned DEFAULT NULL,
  `tocon` int(10) unsigned DEFAULT NULL,
  `toreg` int(10) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `mapConstellationJumps`
--

CREATE TABLE `mapConstellationJumps` (
  `fromRegionID` int(11) DEFAULT NULL,
  `fromConstellationID` int(11) NOT NULL,
  `toConstellationID` int(11) NOT NULL,
  `toRegionID` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `mapConstellations`
--

CREATE TABLE `mapConstellations` (
  `regionID` int(11) DEFAULT NULL,
  `constellationID` int(11) NOT NULL,
  `constellationName` varchar(100) DEFAULT NULL,
  `x` double DEFAULT NULL,
  `y` double DEFAULT NULL,
  `z` double DEFAULT NULL,
  `xMin` double DEFAULT NULL,
  `xMax` double DEFAULT NULL,
  `yMin` double DEFAULT NULL,
  `yMax` double DEFAULT NULL,
  `zMin` double DEFAULT NULL,
  `zMax` double DEFAULT NULL,
  `factionID` int(11) DEFAULT NULL,
  `radius` double DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `mapDenormalize`
--

CREATE TABLE `mapDenormalize` (
  `itemID` int(11) NOT NULL,
  `typeID` int(11) DEFAULT NULL,
  `groupID` int(11) DEFAULT NULL,
  `solarSystemID` int(11) DEFAULT NULL,
  `constellationID` int(11) DEFAULT NULL,
  `regionID` int(11) DEFAULT NULL,
  `orbitID` int(11) DEFAULT NULL,
  `x` double DEFAULT NULL,
  `y` double DEFAULT NULL,
  `z` double DEFAULT NULL,
  `radius` double DEFAULT NULL,
  `itemName` varchar(100) DEFAULT NULL,
  `itemNameID` mediumint(6) DEFAULT NULL,
  `security` double DEFAULT NULL,
  `celestialIndex` tinyint(4) DEFAULT NULL,
  `orbitIndex` tinyint(4) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `mapDynamicData`
--

CREATE TABLE `mapDynamicData` (
  `solarSystemID` int(10) NOT NULL,
  `active` bit(1) NOT NULL DEFAULT b'0',
  `moduleCnt` tinyint(4) NOT NULL DEFAULT '0',
  `structureCnt` tinyint(4) NOT NULL DEFAULT '0',
  `pilotsDocked` smallint(6) NOT NULL DEFAULT '0',
  `pilotsInSpace` smallint(6) NOT NULL DEFAULT '0',
  `jumpsHour` smallint(6) NOT NULL DEFAULT '0',
  `killsHour` smallint(6) NOT NULL DEFAULT '0',
  `kills24Hour` smallint(6) NOT NULL DEFAULT '0',
  `factionKills` smallint(6) NOT NULL DEFAULT '0',
  `factionKills24Hour` smallint(6) NOT NULL DEFAULT '0',
  `podKillsHour` smallint(6) NOT NULL DEFAULT '0',
  `podKills24Hour` smallint(6) NOT NULL DEFAULT '0',
  `killsDateTime` bigint(20) NOT NULL DEFAULT '0',
  `kills24DateTime` bigint(20) NOT NULL DEFAULT '0',
  `podDateTime` bigint(20) NOT NULL DEFAULT '0',
  `pod24DateTime` bigint(20) NOT NULL DEFAULT '0',
  `factionDateTime` bigint(20) NOT NULL DEFAULT '0',
  `faction24DateTime` bigint(20) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `mapJumps`
--

CREATE TABLE `mapJumps` (
  `stargateID` int(11) NOT NULL,
  `celestialID` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `mapLandmarks`
--

CREATE TABLE `mapLandmarks` (
  `landmarkID` smallint(6) NOT NULL,
  `landmarkName` varchar(100) DEFAULT NULL,
  `description` varchar(7000) DEFAULT NULL,
  `locationID` int(11) DEFAULT NULL,
  `x` double DEFAULT NULL,
  `y` double DEFAULT NULL,
  `z` double DEFAULT NULL,
  `radius` double DEFAULT NULL,
  `iconID` smallint(6) DEFAULT NULL,
  `importance` tinyint(4) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `mapLocationScenes`
--

CREATE TABLE `mapLocationScenes` (
  `locationID` int(11) NOT NULL,
  `graphicID` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `mapLocationWormholeClasses`
--

CREATE TABLE `mapLocationWormholeClasses` (
  `locationID` int(11) NOT NULL,
  `wormholeClassID` tinyint(3) unsigned DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `mapRegionJumps`
--

CREATE TABLE `mapRegionJumps` (
  `fromRegionID` int(11) NOT NULL,
  `toRegionID` int(11) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `mapRegions`
--

CREATE TABLE `mapRegions` (
  `regionID` int(11) NOT NULL,
  `regionName` varchar(100) DEFAULT NULL,
  `x` double DEFAULT NULL,
  `y` double DEFAULT NULL,
  `z` double DEFAULT NULL,
  `xMin` double DEFAULT NULL,
  `xMax` double DEFAULT NULL,
  `yMin` double DEFAULT NULL,
  `yMax` double DEFAULT NULL,
  `zMin` double DEFAULT NULL,
  `zMax` double DEFAULT NULL,
  `factionID` int(11) DEFAULT NULL,
  `ratFactionID` int(8) NOT NULL DEFAULT '0',
  `radius` double DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `mapSolarSystemJumps`
--

CREATE TABLE `mapSolarSystemJumps` (
  `fromRegionID` int(11) DEFAULT NULL,
  `fromConstellationID` int(11) DEFAULT NULL,
  `fromSolarSystemID` int(11) NOT NULL,
  `toSolarSystemID` int(11) NOT NULL,
  `toConstellationID` int(11) DEFAULT NULL,
  `toRegionID` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `mapSolarSystems`
--

CREATE TABLE `mapSolarSystems` (
  `regionID` int(11) DEFAULT NULL,
  `constellationID` int(11) DEFAULT NULL,
  `solarSystemID` int(11) NOT NULL,
  `solarSystemName` varchar(100) DEFAULT NULL,
  `x` double DEFAULT NULL,
  `y` double DEFAULT NULL,
  `z` double DEFAULT NULL,
  `xMin` double DEFAULT NULL,
  `xMax` double DEFAULT NULL,
  `yMin` double DEFAULT NULL,
  `yMax` double DEFAULT NULL,
  `zMin` double DEFAULT NULL,
  `zMax` double DEFAULT NULL,
  `luminosity` double DEFAULT NULL,
  `border` tinyint(1) DEFAULT NULL,
  `fringe` tinyint(1) DEFAULT NULL,
  `corridor` tinyint(1) DEFAULT NULL,
  `hub` tinyint(1) DEFAULT NULL,
  `international` tinyint(1) DEFAULT NULL,
  `regional` tinyint(1) DEFAULT NULL,
  `constellation` tinyint(1) DEFAULT NULL,
  `security` double DEFAULT NULL,
  `factionID` int(11) DEFAULT NULL,
  `radius` double DEFAULT NULL,
  `sunTypeID` int(11) DEFAULT NULL,
  `securityClass` varchar(2) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `mapSystemSovInfo`
--

CREATE TABLE `mapSystemSovInfo` (
  `solarSystemID` int(10) NOT NULL,
  `corporationID` int(10) NOT NULL,
  `allianceID` int(10) NOT NULL,
  `claimStructureID` int(10) NOT NULL,
  `claimTime` int(20) NOT NULL,
  `hubID` int(10) NOT NULL,
  `contested` bit(1) NOT NULL DEFAULT b'0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='SystemSovereigntyInfo';

-- --------------------------------------------------------

--
-- Table structure for table `mapUniverse`
--

CREATE TABLE `mapUniverse` (
  `universeID` int(11) NOT NULL,
  `universeName` varchar(100) DEFAULT NULL,
  `x` double DEFAULT NULL,
  `y` double DEFAULT NULL,
  `z` double DEFAULT NULL,
  `xMin` double DEFAULT NULL,
  `xMax` double DEFAULT NULL,
  `yMin` double DEFAULT NULL,
  `yMax` double DEFAULT NULL,
  `zMin` double DEFAULT NULL,
  `zMax` double DEFAULT NULL,
  `radius` double DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `mktData`
--

CREATE TABLE `mktData` (
  `ownerID` int(11) unsigned NOT NULL,
  `typeID` int(11) NOT NULL,
  `date` datetime NOT NULL,
  `isBid` bit(1) NOT NULL,
  `amtLeft` int(8) NOT NULL,
  `amtEntered` int(8) NOT NULL,
  `minVol` tinyint(2) NOT NULL,
  `price` double NOT NULL,
  `stationID` int(11) NOT NULL,
  `range` text NOT NULL,
  `days` int(5) NOT NULL,
  `regionID` int(11) NOT NULL,
  `orderSet` int(5) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `mktHistory`
--

CREATE TABLE `mktHistory` (
  `regionID` int(10) unsigned NOT NULL,
  `typeID` int(10) unsigned NOT NULL,
  `historyDate` bigint(20) NOT NULL DEFAULT '0',
  `lowPrice` double NOT NULL,
  `highPrice` double NOT NULL,
  `avgPrice` double NOT NULL,
  `volume` int(10) unsigned NOT NULL,
  `orders` mediumint(8) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `mktOrders`
--

CREATE TABLE `mktOrders` (
  `orderID` int(10) unsigned NOT NULL,
  `typeID` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ownerID` int(10) unsigned NOT NULL DEFAULT '0',
  `regionID` int(10) unsigned NOT NULL DEFAULT '0',
  `stationID` int(10) unsigned NOT NULL DEFAULT '0',
  `solarSystemID` int(10) NOT NULL DEFAULT '0',
  `orderRange` int(5) unsigned NOT NULL DEFAULT '0',
  `bid` bit(1) NOT NULL DEFAULT b'0',
  `price` float NOT NULL DEFAULT '0',
  `escrow` float unsigned NOT NULL DEFAULT '0',
  `minVolume` int(10) unsigned NOT NULL DEFAULT '0',
  `volEntered` int(10) unsigned NOT NULL DEFAULT '0',
  `volRemaining` int(10) unsigned NOT NULL DEFAULT '0',
  `issued` bigint(20) unsigned NOT NULL DEFAULT '0',
  `contraband` bit(1) NOT NULL DEFAULT b'0',
  `duration` smallint(5) unsigned NOT NULL DEFAULT '0',
  `jumps` smallint(4) NOT NULL DEFAULT '1',
  `isCorp` bit(1) NOT NULL DEFAULT b'0',
  `accountKey` int(10) NOT NULL DEFAULT '1000',
  `memberID` int(10) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `mktTransactions`
--

CREATE TABLE `mktTransactions` (
  `transactionID` int(10) unsigned NOT NULL,
  `transactionDate` bigint(20) unsigned NOT NULL DEFAULT '0',
  `transactionType` bit(1) NOT NULL DEFAULT b'0',
  `typeID` smallint(5) unsigned NOT NULL DEFAULT '0',
  `quantity` mediumint(10) unsigned NOT NULL DEFAULT '0',
  `price` float NOT NULL DEFAULT '0',
  `stationID` int(10) unsigned NOT NULL DEFAULT '0',
  `regionID` int(10) unsigned NOT NULL DEFAULT '0',
  `clientID` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'market user for this tx',
  `corpTransaction` bit(1) NOT NULL DEFAULT b'0',
  `keyID` smallint(4) NOT NULL DEFAULT '1000',
  `characterID` int(10) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `mktUpdates`
--

CREATE TABLE `mktUpdates` (
  `server` tinyint(1) NOT NULL,
  `timeStamp` bigint(20) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='market transaction timestamp saves to avoid over-populating history data';

-- --------------------------------------------------------

--
-- Table structure for table `npcClassGroup`
--

CREATE TABLE `npcClassGroup` (
  `shipClass` int(11) NOT NULL,
  `groupID` int(11) NOT NULL,
  `factionID` int(11) NOT NULL,
  `groupName` varchar(50) COLLATE utf8_unicode_ci NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `npcSpawnClass`
--

CREATE TABLE `npcSpawnClass` (
  `ai` smallint(4) NOT NULL,
  `type` tinyint(3) NOT NULL,
  `sub` tinyint(3) NOT NULL DEFAULT '0',
  `f` tinyint(3) NOT NULL DEFAULT '0',
  `af` tinyint(3) NOT NULL DEFAULT '0',
  `d` tinyint(3) NOT NULL DEFAULT '0',
  `c` tinyint(3) NOT NULL DEFAULT '0',
  `ac` tinyint(3) NOT NULL DEFAULT '0',
  `bc` tinyint(3) NOT NULL DEFAULT '0',
  `bs` tinyint(3) NOT NULL DEFAULT '0',
  `h` tinyint(3) NOT NULL DEFAULT '0',
  `o` tinyint(3) NOT NULL DEFAULT '0',
  `cf` tinyint(3) NOT NULL DEFAULT '0',
  `cd` tinyint(3) NOT NULL DEFAULT '0',
  `cc` tinyint(3) NOT NULL DEFAULT '0',
  `cbc` tinyint(3) NOT NULL DEFAULT '0',
  `cbs` tinyint(3) NOT NULL DEFAULT '0',
  `className` varchar(50) COLLATE utf8_unicode_ci NOT NULL,
  `notes` varchar(70) COLLATE utf8_unicode_ci DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `ownerIcons`
--

CREATE TABLE `ownerIcons` (
  `ownerID` int(11) NOT NULL DEFAULT '0',
  `iconID` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `paperdollColorNames`
--

CREATE TABLE `paperdollColorNames` (
  `colorNameID` int(11) NOT NULL DEFAULT '0',
  `colorName` text COLLATE utf8_unicode_ci
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `paperdollColorRestrictions`
--

CREATE TABLE `paperdollColorRestrictions` (
  `id` int(11) NOT NULL,
  `colorNameID` int(11) DEFAULT NULL,
  `gender` int(11) DEFAULT NULL,
  `restrictions` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `paperdollColors`
--

CREATE TABLE `paperdollColors` (
  `colorID` int(11) NOT NULL DEFAULT '0',
  `colorKey` text COLLATE utf8_unicode_ci,
  `hasSecondary` bit(1) NOT NULL DEFAULT b'0',
  `hasWeight` bit(1) NOT NULL DEFAULT b'0',
  `hasGloss` bit(1) NOT NULL DEFAULT b'0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `paperdollModifierLocations`
--

CREATE TABLE `paperdollModifierLocations` (
  `modifierLocationID` int(11) NOT NULL DEFAULT '0',
  `modifierKey` text COLLATE utf8_unicode_ci,
  `variationKey` text COLLATE utf8_unicode_ci
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `paperdollResources`
--

CREATE TABLE `paperdollResources` (
  `paperdollResourceID` int(11) NOT NULL DEFAULT '0',
  `resGender` bit(1) DEFAULT NULL,
  `resPath` text COLLATE utf8_unicode_ci,
  `restrictions` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `paperdollSculptingLocations`
--

CREATE TABLE `paperdollSculptingLocations` (
  `sculptLocationID` int(11) NOT NULL DEFAULT '0',
  `weightKeyCategory` text COLLATE utf8_unicode_ci,
  `weightKeyPrefix` text COLLATE utf8_unicode_ci
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `piCCPin`
--

CREATE TABLE `piCCPin` (
  `pinID` int(10) NOT NULL DEFAULT '0',
  `charID` int(10) NOT NULL DEFAULT '0',
  `planetID` int(10) NOT NULL DEFAULT '0',
  `typeID` smallint(6) NOT NULL DEFAULT '0',
  `latitude` double NOT NULL DEFAULT '0',
  `longitude` double NOT NULL DEFAULT '0',
  `state` tinyint(2) NOT NULL DEFAULT '1',
  `level` smallint(3) NOT NULL DEFAULT '0',
  `lastSimTime` bigint(20) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='CommandCenter pin data';

-- --------------------------------------------------------

--
-- Table structure for table `piECUHeads`
--

CREATE TABLE `piECUHeads` (
  `ccPinID` int(10) NOT NULL DEFAULT '0',
  `ownerID` int(10) NOT NULL DEFAULT '0',
  `ecuID` int(10) NOT NULL DEFAULT '0',
  `headID` smallint(3) NOT NULL DEFAULT '0',
  `typeID` smallint(6) NOT NULL DEFAULT '0',
  `latitude` double NOT NULL DEFAULT '0',
  `longitude` double NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='PI Colony ECU head data';

-- --------------------------------------------------------

--
-- Table structure for table `piLaunches`
--

CREATE TABLE `piLaunches` (
  `launchID` int(10) NOT NULL,
  `status` tinyint(2) NOT NULL DEFAULT '0',
  `itemID` int(10) NOT NULL DEFAULT '0',
  `charID` int(10) NOT NULL DEFAULT '0',
  `solarSystemID` int(10) NOT NULL DEFAULT '0',
  `planetID` int(10) NOT NULL DEFAULT '0',
  `launchTime` bigint(20) NOT NULL DEFAULT '0',
  `x` double NOT NULL DEFAULT '0',
  `y` double NOT NULL DEFAULT '0',
  `z` double NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `piLinks`
--

CREATE TABLE `piLinks` (
  `ccPinID` int(10) NOT NULL DEFAULT '0',
  `linkID` int(10) NOT NULL DEFAULT '0',
  `level` smallint(3) NOT NULL DEFAULT '0',
  `typeID` smallint(6) NOT NULL DEFAULT '0',
  `state` tinyint(2) NOT NULL DEFAULT '1',
  `endpoint1` int(10) NOT NULL DEFAULT '0',
  `endpoint2` int(10) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='PI Colony link data';

-- --------------------------------------------------------

--
-- Table structure for table `piPinContents`
--

CREATE TABLE `piPinContents` (
  `ccPinID` int(10) NOT NULL DEFAULT '0',
  `pinID` int(10) NOT NULL DEFAULT '0',
  `typeID` smallint(5) NOT NULL DEFAULT '0',
  `itemQty` int(10) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Colony pin contents data';

-- --------------------------------------------------------

--
-- Table structure for table `piPinMap`
--

CREATE TABLE `piPinMap` (
  `schematicID` smallint(6) NOT NULL,
  `pinTypeID` int(11) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `piPins`
--

CREATE TABLE `piPins` (
  `ccPinID` int(10) NOT NULL DEFAULT '0',
  `pinID` int(10) NOT NULL DEFAULT '0',
  `typeID` smallint(6) NOT NULL DEFAULT '0',
  `ownerID` int(10) NOT NULL DEFAULT '0',
  `state` tinyint(1) NOT NULL DEFAULT '1',
  `level` smallint(3) NOT NULL DEFAULT '0',
  `latitude` float NOT NULL DEFAULT '0',
  `longitude` float NOT NULL DEFAULT '0',
  `isCommandCenter` bit(1) NOT NULL DEFAULT b'0',
  `isLaunchable` bit(1) NOT NULL DEFAULT b'0',
  `isProcess` bit(1) NOT NULL DEFAULT b'0',
  `isStorage` bit(1) NOT NULL DEFAULT b'0',
  `isECU` bit(1) NOT NULL DEFAULT b'0',
  `hasReceivedInputs` bit(1) NOT NULL DEFAULT b'0',
  `receivedInputsLastCycle` bit(1) NOT NULL DEFAULT b'0',
  `schematicID` smallint(3) NOT NULL DEFAULT '0',
  `programType` smallint(3) NOT NULL DEFAULT '0',
  `cycleTime` bigint(20) NOT NULL DEFAULT '0',
  `launchTime` bigint(20) NOT NULL DEFAULT '0',
  `expiryTime` bigint(20) NOT NULL DEFAULT '0',
  `installTime` bigint(20) NOT NULL DEFAULT '0',
  `lastRunTime` bigint(20) NOT NULL DEFAULT '0',
  `headRadius` float NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Colony pin data';

-- --------------------------------------------------------

--
-- Table structure for table `piPlanets`
--

CREATE TABLE `piPlanets` (
  `charID` int(10) NOT NULL DEFAULT '0',
  `solarSystemID` int(10) NOT NULL DEFAULT '0',
  `planetID` int(10) NOT NULL DEFAULT '0',
  `typeID` smallint(6) NOT NULL DEFAULT '0',
  `numberOfPins` tinyint(3) NOT NULL DEFAULT '1',
  `ccPinID` int(10) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `piRoutes`
--

CREATE TABLE `piRoutes` (
  `ccPinID` int(10) NOT NULL DEFAULT '0',
  `routeID` smallint(6) unsigned NOT NULL,
  `srcPinID` int(10) NOT NULL DEFAULT '0',
  `destPinID` int(10) NOT NULL DEFAULT '0',
  `state` tinyint(2) NOT NULL DEFAULT '1',
  `priority` tinyint(2) NOT NULL DEFAULT '0',
  `path` varchar(200) NOT NULL DEFAULT '0',
  `typeID` smallint(5) NOT NULL DEFAULT '0',
  `itemQty` int(10) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Colony route data';

-- --------------------------------------------------------

--
-- Table structure for table `piSchematics`
--

CREATE TABLE `piSchematics` (
  `schematicID` smallint(6) NOT NULL,
  `schematicName` varchar(255) DEFAULT NULL,
  `cycleTime` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `piTypeMap`
--

CREATE TABLE `piTypeMap` (
  `schematicID` smallint(6) NOT NULL,
  `typeID` int(11) NOT NULL,
  `quantity` smallint(6) DEFAULT NULL,
  `isInput` bit(1) NOT NULL DEFAULT b'0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `posCustomsOfficeData`
--

CREATE TABLE `posCustomsOfficeData` (
  `itemID` int(11) NOT NULL DEFAULT '0',
  `ownerID` int(11) NOT NULL DEFAULT '0',
  `level` tinyint(2) NOT NULL DEFAULT '1',
  `state` tinyint(2) NOT NULL DEFAULT '0',
  `status` tinyint(2) NOT NULL DEFAULT '0',
  `orbitalHackerProgress` float NOT NULL DEFAULT '0',
  `orbitalHackerID` int(11) NOT NULL DEFAULT '0',
  `allowAlly` bit(1) NOT NULL DEFAULT b'0',
  `allowStandings` bit(1) NOT NULL DEFAULT b'0',
  `selectedHour` tinyint(2) NOT NULL DEFAULT '0',
  `standingValue` tinyint(2) NOT NULL DEFAULT '0',
  `corpTax` float NOT NULL DEFAULT '0',
  `allyTax` float NOT NULL DEFAULT '0',
  `highTax` float NOT NULL DEFAULT '0',
  `neutTax` float NOT NULL DEFAULT '0',
  `goodTax` float NOT NULL DEFAULT '0',
  `badTax` float NOT NULL DEFAULT '0',
  `horribleTax` float NOT NULL DEFAULT '0',
  `timestamp` bigint(20) NOT NULL DEFAULT '0',
  `rotX` float NOT NULL DEFAULT '0',
  `rotY` float NOT NULL DEFAULT '0',
  `rotZ` float NOT NULL DEFAULT '0',
  `rotW` float NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='POS Customs Office Data';

-- --------------------------------------------------------

--
-- Table structure for table `posJumpBridgeData`
--

CREATE TABLE `posJumpBridgeData` (
  `itemID` int(10) NOT NULL DEFAULT '0',
  `towerID` int(10) NOT NULL DEFAULT '0',
  `corpID` int(10) NOT NULL DEFAULT '0',
  `allyID` int(10) NOT NULL DEFAULT '0',
  `systemID` int(10) NOT NULL DEFAULT '0',
  `toItemID` int(10) NOT NULL DEFAULT '0',
  `toTypeID` int(6) NOT NULL DEFAULT '0',
  `toSystemID` int(10) NOT NULL DEFAULT '0',
  `password` varchar(50) NOT NULL,
  `allowCorp` bit(1) NOT NULL DEFAULT b'0',
  `allowAlliance` bit(1) NOT NULL DEFAULT b'0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='POS Jump Bridge Data';

-- --------------------------------------------------------

--
-- Table structure for table `posStructureData`
--

CREATE TABLE `posStructureData` (
  `itemID` int(10) NOT NULL DEFAULT '0',
  `towerID` int(10) NOT NULL DEFAULT '0',
  `moonID` int(10) NOT NULL DEFAULT '0',
  `state` tinyint(2) NOT NULL DEFAULT '-1',
  `canUse` bit(1) NOT NULL DEFAULT b'0',
  `canView` bit(1) NOT NULL DEFAULT b'0',
  `canTake` bit(1) NOT NULL DEFAULT b'0',
  `status` tinyint(2) NOT NULL DEFAULT '0',
  `timestamp` bigint(20) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='POS Basic Structure Data';

-- --------------------------------------------------------

--
-- Table structure for table `posTowerData`
--

CREATE TABLE `posTowerData` (
  `itemID` int(10) NOT NULL DEFAULT '0',
  `harmonic` int(10) NOT NULL DEFAULT '0',
  `password` varchar(50) NOT NULL DEFAULT '''''',
  `status` float NOT NULL DEFAULT '0',
  `standing` float NOT NULL DEFAULT '0',
  `standingOwnerID` int(10) NOT NULL DEFAULT '0',
  `corpWar` bit(1) NOT NULL DEFAULT b'0',
  `statusDrop` bit(1) NOT NULL DEFAULT b'0',
  `allyStandings` bit(1) NOT NULL DEFAULT b'0',
  `showInCalendar` bit(1) NOT NULL DEFAULT b'0',
  `sendFuelNotifications` bit(1) NOT NULL DEFAULT b'0',
  `allowCorp` bit(1) NOT NULL DEFAULT b'0',
  `allowAlliance` bit(1) NOT NULL DEFAULT b'0',
  `anchor` bit(1) NOT NULL DEFAULT b'0',
  `unanchor` bit(1) NOT NULL DEFAULT b'0',
  `online` bit(1) NOT NULL DEFAULT b'0',
  `offline` bit(1) NOT NULL DEFAULT b'0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='POS Tower Data';

-- --------------------------------------------------------

--
-- Table structure for table `qstCourier`
--

CREATE TABLE `qstCourier` (
  `id` int(5) NOT NULL DEFAULT '0',
  `briefingID` int(5) NOT NULL DEFAULT '0',
  `name` text,
  `level` tinyint(1) NOT NULL DEFAULT '0',
  `typeID` tinyint(1) NOT NULL DEFAULT '0',
  `sysRange` tinyint(2) NOT NULL DEFAULT '1',
  `important` bit(1) NOT NULL DEFAULT b'0',
  `storyline` bit(1) NOT NULL DEFAULT b'0',
  `raceID` tinyint(2) NOT NULL DEFAULT '0',
  `itemTypeID` int(6) NOT NULL DEFAULT '0',
  `itemQty` int(10) NOT NULL DEFAULT '0',
  `rewardISK` int(10) NOT NULL DEFAULT '0',
  `rewardItemID` int(11) NOT NULL DEFAULT '0',
  `rewardItemQty` int(11) NOT NULL DEFAULT '0',
  `bonusISK` int(11) NOT NULL DEFAULT '0',
  `bonusTime` int(5) NOT NULL DEFAULT '0',
  `collateral` int(7) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `qstMining`
--

CREATE TABLE `qstMining` (
  `id` int(5) NOT NULL DEFAULT '0',
  `briefingID` int(5) NOT NULL DEFAULT '0',
  `name` text,
  `level` tinyint(1) NOT NULL DEFAULT '0',
  `typeID` tinyint(1) NOT NULL DEFAULT '0',
  `sysRange` tinyint(2) NOT NULL DEFAULT '1',
  `important` bit(1) NOT NULL DEFAULT b'0',
  `storyline` bit(1) NOT NULL DEFAULT b'0',
  `raceID` tinyint(2) NOT NULL DEFAULT '0',
  `itemTypeID` int(6) NOT NULL DEFAULT '0',
  `itemQty` int(10) NOT NULL DEFAULT '0',
  `rewardISK` int(10) NOT NULL DEFAULT '0',
  `rewardItemID` int(11) NOT NULL DEFAULT '0',
  `rewardItemQty` int(11) NOT NULL DEFAULT '0',
  `bonusISK` int(11) NOT NULL DEFAULT '0',
  `bonusTime` int(10) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `ramActivities`
--

CREATE TABLE `ramActivities` (
  `activityID` tinyint(3) unsigned NOT NULL,
  `activityName` varchar(100) DEFAULT NULL,
  `iconNo` varchar(5) DEFAULT NULL,
  `description` varchar(1000) DEFAULT NULL,
  `published` bit(1) NOT NULL DEFAULT b'0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `ramAssemblyLines`
--

CREATE TABLE `ramAssemblyLines` (
  `assemblyLineID` int(11) NOT NULL,
  `assemblyLineTypeID` tinyint(3) unsigned DEFAULT NULL,
  `containerID` int(11) DEFAULT NULL,
  `nextFreeTime` bigint(20) NOT NULL DEFAULT '0',
  `UIGroupingID` tinyint(3) unsigned DEFAULT NULL,
  `costInstall` double DEFAULT NULL,
  `costPerHour` double DEFAULT NULL,
  `restrictionMask` tinyint(4) DEFAULT NULL,
  `discountPerGoodStandingPoint` float DEFAULT NULL,
  `surchargePerBadStandingPoint` double DEFAULT NULL,
  `minimumStanding` double DEFAULT NULL,
  `minimumCharSecurity` double DEFAULT NULL,
  `minimumCorpSecurity` double DEFAULT NULL,
  `maximumCharSecurity` double DEFAULT NULL,
  `maximumCorpSecurity` double DEFAULT NULL,
  `ownerID` int(11) DEFAULT NULL,
  `activityID` tinyint(3) unsigned DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `ramAssemblyLineStationCostLogs`
--

CREATE TABLE `ramAssemblyLineStationCostLogs` (
  `assemblyLineTypeID` int(11) NOT NULL DEFAULT '0',
  `stationID` int(11) NOT NULL DEFAULT '0',
  `logDateTime` char(20) NOT NULL DEFAULT '',
  `_usage` double NOT NULL DEFAULT '0',
  `costPerHour` float NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `ramAssemblyLineStations`
--

CREATE TABLE `ramAssemblyLineStations` (
  `stationID` int(11) NOT NULL,
  `assemblyLineTypeID` tinyint(3) unsigned NOT NULL,
  `quantity` tinyint(4) DEFAULT NULL,
  `stationTypeID` int(11) DEFAULT NULL,
  `ownerID` int(11) DEFAULT NULL,
  `solarSystemID` int(11) DEFAULT NULL,
  `regionID` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `ramAssemblyLineTypeDetailPerCategory`
--

CREATE TABLE `ramAssemblyLineTypeDetailPerCategory` (
  `assemblyLineTypeID` tinyint(3) unsigned NOT NULL,
  `categoryID` int(11) NOT NULL,
  `timeMultiplier` double DEFAULT NULL,
  `materialMultiplier` double DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `ramAssemblyLineTypeDetailPerGroup`
--

CREATE TABLE `ramAssemblyLineTypeDetailPerGroup` (
  `assemblyLineTypeID` tinyint(3) unsigned NOT NULL,
  `groupID` int(11) NOT NULL,
  `timeMultiplier` double DEFAULT NULL,
  `materialMultiplier` double DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `ramAssemblyLineTypes`
--

CREATE TABLE `ramAssemblyLineTypes` (
  `assemblyLineTypeID` tinyint(3) unsigned NOT NULL,
  `assemblyLineTypeName` varchar(100) DEFAULT NULL,
  `description` varchar(1000) DEFAULT NULL,
  `baseTimeMultiplier` double DEFAULT NULL,
  `baseMaterialMultiplier` double DEFAULT NULL,
  `volume` double DEFAULT NULL,
  `activityID` tinyint(3) unsigned DEFAULT NULL,
  `minCostPerHour` double DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `ramCompletedStatuses`
--

CREATE TABLE `ramCompletedStatuses` (
  `completedStatusID` int(10) unsigned NOT NULL DEFAULT '0',
  `completedStatusName` varchar(100) NOT NULL DEFAULT '',
  `completedStatusText` varchar(100) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `ramInstallationTypeContents`
--

CREATE TABLE `ramInstallationTypeContents` (
  `installationTypeID` int(11) NOT NULL,
  `assemblyLineTypeID` tinyint(3) unsigned NOT NULL,
  `quantity` tinyint(4) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `ramInstallationTypeDefaultContents`
--

CREATE TABLE `ramInstallationTypeDefaultContents` (
  `installationTypeID` int(11) NOT NULL DEFAULT '0',
  `assemblyLineTypeID` int(11) NOT NULL DEFAULT '0',
  `UIGroupingID` int(11) NOT NULL DEFAULT '0',
  `quantity` int(11) NOT NULL DEFAULT '0',
  `costInstall` float NOT NULL DEFAULT '0',
  `costPerHour` float NOT NULL DEFAULT '0',
  `restrictionMask` int(11) NOT NULL DEFAULT '0',
  `discountPerGoodStandingPoint` float NOT NULL DEFAULT '0',
  `surchargePerBadStandingPoint` float NOT NULL DEFAULT '0',
  `minimumStanding` float NOT NULL DEFAULT '0',
  `minimumCharSecurity` float NOT NULL DEFAULT '0',
  `minimumCorpSecurity` float NOT NULL DEFAULT '0',
  `maximumCharSecurity` float NOT NULL DEFAULT '0',
  `maximumCorpSecurity` float NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `ramJobs`
--

CREATE TABLE `ramJobs` (
  `jobID` int(10) unsigned NOT NULL,
  `eventID` int(10) unsigned NOT NULL DEFAULT '0',
  `ownerID` int(10) unsigned NOT NULL,
  `installerID` int(10) unsigned NOT NULL,
  `assemblyLineID` int(10) unsigned NOT NULL,
  `installedItemID` int(10) unsigned NOT NULL,
  `installTime` bigint(20) unsigned NOT NULL,
  `beginProductionTime` bigint(20) unsigned NOT NULL,
  `pauseProductionTime` bigint(20) unsigned DEFAULT NULL,
  `endProductionTime` bigint(20) unsigned NOT NULL,
  `runs` int(10) NOT NULL,
  `outputFlag` int(10) unsigned NOT NULL,
  `completedStatusID` int(10) unsigned NOT NULL,
  `licensedProductionRuns` int(10) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `ramTypeRequirements`
--

CREATE TABLE `ramTypeRequirements` (
  `typeID` int(10) NOT NULL,
  `activityID` int(11) NOT NULL,
  `requiredTypeID` int(10) NOT NULL,
  `quantity` int(10) DEFAULT NULL,
  `damagePerJob` double DEFAULT NULL,
  `extra` bit(1) NOT NULL DEFAULT b'0'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `rentalInfo`
--

CREATE TABLE `rentalInfo` (
  `stationID` int(10) unsigned NOT NULL DEFAULT '0',
  `slotNumber` int(10) unsigned NOT NULL DEFAULT '0',
  `renterID` int(10) unsigned NOT NULL DEFAULT '0',
  `typeID` int(10) unsigned NOT NULL DEFAULT '0',
  `rentPeriodInDays` int(10) unsigned NOT NULL DEFAULT '0',
  `periodCost` double NOT NULL DEFAULT '0',
  `billID` int(10) unsigned NOT NULL DEFAULT '0',
  `balanceDueDate` int(10) unsigned NOT NULL DEFAULT '0',
  `discontinue` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `publiclyAvailable` tinyint(3) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='I dont know what this table is for';

-- --------------------------------------------------------

--
-- Table structure for table `repFactions`
--

CREATE TABLE `repFactions` (
  `fromID` int(10) unsigned NOT NULL DEFAULT '0',
  `toID` int(10) unsigned NOT NULL DEFAULT '0',
  `standing` float NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Static Faction Standing Data';

-- --------------------------------------------------------

--
-- Table structure for table `repStandingChanges`
--

CREATE TABLE `repStandingChanges` (
  `eventID` int(10) unsigned NOT NULL,
  `eventTypeID` int(10) unsigned NOT NULL DEFAULT '0',
  `eventDateTime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `fromID` int(10) unsigned NOT NULL DEFAULT '0',
  `toID` int(10) unsigned NOT NULL DEFAULT '0',
  `modification` double NOT NULL DEFAULT '0',
  `originalFromID` int(10) unsigned NOT NULL DEFAULT '0',
  `originalToID` int(10) unsigned NOT NULL DEFAULT '0',
  `int_1` int(10) unsigned NOT NULL DEFAULT '0',
  `int_2` int(10) unsigned NOT NULL DEFAULT '0',
  `int_3` int(10) unsigned NOT NULL DEFAULT '0',
  `msg` text NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `repStandings`
--

CREATE TABLE `repStandings` (
  `fromID` int(10) unsigned NOT NULL DEFAULT '0',
  `toID` int(10) unsigned NOT NULL DEFAULT '0',
  `standing` float NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Data for All Standings';

-- --------------------------------------------------------

--
-- Table structure for table `roidDistribution`
--

CREATE TABLE `roidDistribution` (
  `AI` int(5) NOT NULL,
  `systemSec` varchar(2) NOT NULL,
  `roidID` int(10) unsigned NOT NULL,
  `roidName` varchar(20) NOT NULL,
  `percent` float NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `schematics`
--

CREATE TABLE `schematics` (
  `schematicID` int(11) NOT NULL DEFAULT '0',
  `schematicName` text COLLATE utf8_unicode_ci,
  `cycleTime` int(11) DEFAULT NULL,
  `dataID` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `schematicsPinMap`
--

CREATE TABLE `schematicsPinMap` (
  `id` int(11) NOT NULL,
  `schematicID` int(11) DEFAULT NULL,
  `pinTypeID` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `schematicsTypeMap`
--

CREATE TABLE `schematicsTypeMap` (
  `id` int(11) NOT NULL,
  `schematicID` int(11) DEFAULT NULL,
  `typeID` int(11) DEFAULT NULL,
  `quantity` smallint(6) DEFAULT NULL,
  `isInput` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `shipInsurance`
--

CREATE TABLE `shipInsurance` (
  `shipID` int(10) NOT NULL,
  `shipName` varchar(150) COLLATE utf8_unicode_ci NOT NULL,
  `ownerID` int(10) NOT NULL,
  `startDate` bigint(20) NOT NULL,
  `endDate` bigint(20) NOT NULL,
  `fraction` float(4,3) NOT NULL,
  `payOutAmount` int(10) NOT NULL DEFAULT '0',
  `isCorpItem` bit(1) NOT NULL DEFAULT b'0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `shipTypes`
--

CREATE TABLE `shipTypes` (
  `shipTypeID` int(11) NOT NULL DEFAULT '0',
  `weaponTypeID` int(11) DEFAULT NULL,
  `miningTypeID` int(11) DEFAULT NULL,
  `skillTypeID` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `shipWeaponGroups`
--

CREATE TABLE `shipWeaponGroups` (
  `shipID` int(10) unsigned NOT NULL DEFAULT '0',
  `masterID` int(10) unsigned NOT NULL DEFAULT '0',
  `slaveID` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='ship linked weapons table';

-- --------------------------------------------------------

--
-- Table structure for table `sklBaseSkills`
--

CREATE TABLE `sklBaseSkills` (
  `ID` tinyint(4) NOT NULL,
  `skillTypeID` smallint(6) NOT NULL,
  `level` tinyint(4) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='Basic Skills for All Races';

-- --------------------------------------------------------

--
-- Table structure for table `sklCareerSkills`
--

CREATE TABLE `sklCareerSkills` (
  `careerID` int(10) NOT NULL DEFAULT '0',
  `skillTypeID` int(10) NOT NULL DEFAULT '0',
  `level` tinyint(3) unsigned NOT NULL DEFAULT '1'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='skill and level list by careerID';

-- --------------------------------------------------------

--
-- Table structure for table `sklRaceSkills`
--

CREATE TABLE `sklRaceSkills` (
  `id` int(10) NOT NULL,
  `raceID` int(10) DEFAULT NULL,
  `skillTypeID` int(10) DEFAULT NULL,
  `level` tinyint(3) unsigned DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='skill and level list by raceID';

-- --------------------------------------------------------

--
-- Table structure for table `sounds`
--

CREATE TABLE `sounds` (
  `soundID` int(11) NOT NULL DEFAULT '0',
  `soundFile` text COLLATE utf8_unicode_ci,
  `description` text COLLATE utf8_unicode_ci,
  `obsolete` bit(1) NOT NULL DEFAULT b'0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `specialities`
--

CREATE TABLE `specialities` (
  `id` int(11) NOT NULL,
  `careerID` tinyint(3) unsigned DEFAULT NULL,
  `specialityID` int(11) DEFAULT NULL,
  `specialityName` text COLLATE utf8_unicode_ci,
  `description` text COLLATE utf8_unicode_ci,
  `shortDescription` text COLLATE utf8_unicode_ci,
  `graphicID` int(11) DEFAULT NULL,
  `iconID` int(11) DEFAULT NULL,
  `dataID` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `specialitySkills`
--

CREATE TABLE `specialitySkills` (
  `specialityID` int(11) NOT NULL DEFAULT '0',
  `skillTypeID` int(11) NOT NULL DEFAULT '0',
  `level` tinyint(3) unsigned DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `srvStatisticData`
--

CREATE TABLE `srvStatisticData` (
  `timeStamp` bigint(20) NOT NULL DEFAULT '0' COMMENT 'filetime',
  `timeSpan` mediumint(6) unsigned NOT NULL DEFAULT '0' COMMENT 'in minutes',
  `pcShots` int(10) unsigned NOT NULL DEFAULT '0',
  `pcMissiles` int(10) unsigned NOT NULL DEFAULT '0',
  `ramJobs` int(11) NOT NULL DEFAULT '0',
  `shipsSalvaged` smallint(5) unsigned NOT NULL DEFAULT '0',
  `pcBounties` float NOT NULL DEFAULT '0',
  `npcBounties` float NOT NULL DEFAULT '0',
  `oreMined` float NOT NULL DEFAULT '0',
  `iskMarket` float NOT NULL DEFAULT '0',
  `probesLaunched` mediumint(5) unsigned NOT NULL DEFAULT '0',
  `sitesScanned` mediumint(5) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Current Data for graphing player activity over period of time';

-- --------------------------------------------------------

--
-- Table structure for table `srvStatisticHistory`
--

CREATE TABLE `srvStatisticHistory` (
  `idx` int(2) unsigned NOT NULL DEFAULT '0',
  `pcShots` int(10) unsigned NOT NULL DEFAULT '0',
  `pcMissiles` int(10) unsigned NOT NULL DEFAULT '0',
  `ramJobs` int(11) NOT NULL DEFAULT '0',
  `shipsSalvaged` smallint(5) unsigned NOT NULL DEFAULT '0',
  `pcBounties` double NOT NULL DEFAULT '0',
  `npcBounties` double NOT NULL DEFAULT '0',
  `oreMined` double NOT NULL DEFAULT '0',
  `iskMarket` double NOT NULL DEFAULT '0',
  `probesLaunched` mediumint(5) unsigned NOT NULL DEFAULT '0',
  `sitesScanned` mediumint(5) unsigned NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Historical Data for graphing player activity from previous months';

-- --------------------------------------------------------

--
-- Table structure for table `srvStatus`
--

CREATE TABLE `srvStatus` (
  `AI` int(10) NOT NULL,
  `srvName` varchar(60) NOT NULL,
  `Online` bit(1) NOT NULL DEFAULT b'0',
  `startTime` bigint(20) NOT NULL,
  `ClientSeed` int(10) NOT NULL,
  `Connections` smallint(6) NOT NULL,
  `threads` tinyint(4) NOT NULL,
  `rss` float NOT NULL,
  `vm` float NOT NULL,
  `user` float NOT NULL,
  `kernel` float NOT NULL,
  `items` int(10) NOT NULL,
  `systems` int(10) NOT NULL,
  `bubbles` int(10) NOT NULL,
  `updateTime` int(10) NOT NULL,
  `npcs` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `staOffices`
--

CREATE TABLE `staOffices` (
  `itemID` int(10) NOT NULL,
  `name` text NOT NULL,
  `officeFolderID` int(10) NOT NULL DEFAULT '0',
  `corporationID` int(10) NOT NULL DEFAULT '0',
  `stationID` int(10) NOT NULL DEFAULT '0',
  `solarSystemID` int(10) NOT NULL,
  `typeID` int(10) NOT NULL DEFAULT '0',
  `stationTypeID` int(11) NOT NULL,
  `flag` int(5) NOT NULL,
  `lockDown` bit(1) NOT NULL DEFAULT b'0',
  `rentalFee` bigint(20) NOT NULL DEFAULT '0',
  `expiryDateTime` bigint(20) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Station Office Data';

-- --------------------------------------------------------

--
-- Table structure for table `staOperations`
--

CREATE TABLE `staOperations` (
  `activityID` tinyint(3) unsigned DEFAULT NULL,
  `operationID` tinyint(3) unsigned NOT NULL,
  `operationName` varchar(100) DEFAULT NULL,
  `operationNameID` int(6) NOT NULL,
  `description` varchar(1000) DEFAULT NULL,
  `descriptionID` int(3) DEFAULT NULL,
  `fringe` tinyint(4) DEFAULT NULL,
  `corridor` tinyint(4) DEFAULT NULL,
  `hub` tinyint(4) DEFAULT NULL,
  `border` tinyint(4) DEFAULT NULL,
  `ratio` tinyint(4) DEFAULT NULL,
  `caldariStationTypeID` int(11) DEFAULT NULL,
  `minmatarStationTypeID` int(11) DEFAULT NULL,
  `amarrStationTypeID` int(11) DEFAULT NULL,
  `gallenteStationTypeID` int(11) DEFAULT NULL,
  `joveStationTypeID` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `staOperationServices`
--

CREATE TABLE `staOperationServices` (
  `operationID` tinyint(3) unsigned NOT NULL,
  `serviceID` int(11) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `staServices`
--

CREATE TABLE `staServices` (
  `serviceID` int(11) NOT NULL,
  `serviceName` varchar(100) DEFAULT NULL,
  `description` varchar(1000) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `staStations`
--

CREATE TABLE `staStations` (
  `stationID` int(11) NOT NULL,
  `security` float NOT NULL DEFAULT '0',
  `dockingCostPerVolume` float NOT NULL DEFAULT '0',
  `maxShipVolumeDockable` int(10) NOT NULL DEFAULT '0',
  `officeSlots` tinyint(3) unsigned NOT NULL DEFAULT '24',
  `officeRentalCost` int(11) DEFAULT '10000',
  `operationID` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `stationTypeID` int(11) NOT NULL DEFAULT '0',
  `corporationID` int(11) NOT NULL DEFAULT '0',
  `solarSystemID` int(11) NOT NULL DEFAULT '0',
  `constellationID` int(11) NOT NULL DEFAULT '0',
  `regionID` int(11) NOT NULL DEFAULT '0',
  `stationName` text NOT NULL,
  `x` double NOT NULL DEFAULT '0',
  `y` double NOT NULL DEFAULT '0',
  `z` double NOT NULL DEFAULT '0',
  `reprocessingEfficiency` float NOT NULL DEFAULT '0',
  `reprocessingStationsTake` float NOT NULL DEFAULT '0',
  `reprocessingHangarFlag` tinyint(4) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `staStationTypes`
--

CREATE TABLE `staStationTypes` (
  `stationTypeID` int(11) NOT NULL,
  `dockEntryX` double DEFAULT NULL,
  `dockEntryY` double DEFAULT NULL,
  `dockEntryZ` double DEFAULT NULL,
  `dockOrientationX` double DEFAULT NULL,
  `dockOrientationY` double DEFAULT NULL,
  `dockOrientationZ` double DEFAULT NULL,
  `operationID` tinyint(3) unsigned DEFAULT NULL,
  `reprocessingEfficiency` double DEFAULT NULL,
  `conquerable` tinyint(1) DEFAULT NULL,
  `hangarGraphicID` int(10) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `sysAsteroids`
--

CREATE TABLE `sysAsteroids` (
  `itemID` int(10) unsigned NOT NULL,
  `itemName` varchar(25) NOT NULL,
  `typeID` int(10) NOT NULL,
  `systemID` int(10) NOT NULL,
  `beltID` int(10) NOT NULL,
  `quantity` double NOT NULL,
  `radius` double NOT NULL,
  `x` double NOT NULL,
  `y` double NOT NULL,
  `z` double NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `sysCalendarEvents`
--

CREATE TABLE `sysCalendarEvents` (
  `eventID` int(10) unsigned NOT NULL,
  `ownerID` int(10) NOT NULL DEFAULT '0',
  `creatorID` int(10) NOT NULL DEFAULT '0',
  `month` tinyint(2) NOT NULL DEFAULT '0',
  `year` smallint(4) NOT NULL DEFAULT '0',
  `eventDateTime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `eventDuration` smallint(4) unsigned DEFAULT NULL,
  `dateModified` bigint(20) DEFAULT NULL,
  `importance` bit(1) NOT NULL DEFAULT b'0',
  `isDeleted` bit(1) NOT NULL DEFAULT b'0',
  `flag` tinyint(2) unsigned NOT NULL DEFAULT '1',
  `autoEventType` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `eventTitle` varchar(80) NOT NULL,
  `eventText` varchar(500) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `sysCalendarInvitees`
--

CREATE TABLE `sysCalendarInvitees` (
  `eventID` int(11) NOT NULL DEFAULT '0',
  `inviteeList` varchar(300) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `sysCalendarResponses`
--

CREATE TABLE `sysCalendarResponses` (
  `eventID` int(10) NOT NULL DEFAULT '0',
  `charID` int(10) NOT NULL DEFAULT '0',
  `response` tinyint(1) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='response list for calendar events';

-- --------------------------------------------------------

--
-- Table structure for table `sysSignatures`
--

CREATE TABLE `sysSignatures` (
  `sigID` varchar(7) COLLATE utf8_unicode_ci NOT NULL,
  `sigItemID` int(11) NOT NULL DEFAULT '0',
  `dungeonType` int(2) NOT NULL DEFAULT '0',
  `sigName` varchar(50) COLLATE utf8_unicode_ci NOT NULL,
  `systemID` int(11) NOT NULL DEFAULT '0',
  `sigTypeID` int(11) NOT NULL DEFAULT '0',
  `sigGroupID` int(10) NOT NULL DEFAULT '0',
  `scanGroupID` int(11) NOT NULL DEFAULT '0',
  `scanAttributeID` int(10) NOT NULL DEFAULT '0',
  `x` double NOT NULL,
  `y` double NOT NULL,
  `z` double NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Table structure for table `test`
--

CREATE TABLE `test` (
  `ai` int(11) NOT NULL,
  `typeID` int(11) NOT NULL,
  `date` datetime NOT NULL,
  `ts` bigint(20) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `translationTables`
--

CREATE TABLE `translationTables` (
  `sourceTable` varchar(100) NOT NULL,
  `destinationTable` varchar(200) DEFAULT NULL,
  `translatedKey` varchar(100) NOT NULL,
  `tcGroupID` int(11) DEFAULT NULL,
  `tcID` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `trnTranslationColumns`
--

CREATE TABLE `trnTranslationColumns` (
  `tcGroupID` smallint(6) DEFAULT NULL,
  `tcID` smallint(6) NOT NULL,
  `tableName` varchar(256) NOT NULL,
  `columnName` varchar(128) NOT NULL,
  `masterID` varchar(128) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `trnTranslationLanguages`
--

CREATE TABLE `trnTranslationLanguages` (
  `numericLanguageID` int(11) NOT NULL,
  `languageID` varchar(50) DEFAULT NULL,
  `languageName` varchar(200) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `trnTranslations`
--

CREATE TABLE `trnTranslations` (
  `tcID` smallint(6) NOT NULL,
  `keyID` int(11) NOT NULL,
  `languageID` varchar(50) NOT NULL,
  `text` varchar(16000) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `tutorials`
--

CREATE TABLE `tutorials` (
  `tutorialID` int(10) unsigned NOT NULL DEFAULT '0',
  `tutorialName` varchar(100) NOT NULL,
  `nextTutorialID` int(10) unsigned DEFAULT NULL,
  `categoryID` int(10) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `tutorials_criterias`
--

CREATE TABLE `tutorials_criterias` (
  `tutorialID` int(10) unsigned NOT NULL DEFAULT '0',
  `criteriaID` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `tutorial_categories`
--

CREATE TABLE `tutorial_categories` (
  `categoryID` int(10) unsigned NOT NULL,
  `categoryName` varchar(100) NOT NULL,
  `description` varchar(200) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `tutorial_criteria`
--

CREATE TABLE `tutorial_criteria` (
  `criteriaID` int(10) unsigned NOT NULL DEFAULT '0',
  `criteriaName` varchar(100) NOT NULL DEFAULT '',
  `messageText` mediumtext NOT NULL,
  `audioPath` varchar(200) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `tutorial_pages`
--

CREATE TABLE `tutorial_pages` (
  `pageID` int(10) unsigned NOT NULL DEFAULT '0',
  `pageNumber` int(10) unsigned NOT NULL DEFAULT '0',
  `pageName` varchar(100) NOT NULL DEFAULT '',
  `text` mediumtext NOT NULL,
  `imagePath` varchar(200) DEFAULT NULL,
  `audioPath` varchar(200) NOT NULL DEFAULT '',
  `tutorialID` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `tutorial_page_criteria`
--

CREATE TABLE `tutorial_page_criteria` (
  `pageID` int(10) unsigned NOT NULL DEFAULT '0',
  `criteriaID` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 ROW_FORMAT=DYNAMIC;

-- --------------------------------------------------------

--
-- Table structure for table `warCombatZones`
--

CREATE TABLE `warCombatZones` (
  `combatZoneID` int(11) NOT NULL,
  `combatZoneName` varchar(100) DEFAULT NULL,
  `factionID` int(11) DEFAULT NULL,
  `centerSystemID` int(11) DEFAULT NULL,
  `description` varchar(500) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `warCombatZoneSystems`
--

CREATE TABLE `warCombatZoneSystems` (
  `solarSystemID` int(11) NOT NULL,
  `combatZoneID` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Table structure for table `webBounties`
--

CREATE TABLE `webBounties` (
  `characterID` int(10) NOT NULL,
  `ownerID` int(10) NOT NULL,
  `bounty` bigint(20) NOT NULL,
  `timePlaced` bigint(20) unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `account`
--
ALTER TABLE `account`
  ADD PRIMARY KEY (`accountID`),
  ADD UNIQUE KEY `accountName` (`accountName`);

--
-- Indexes for table `actKeyTypes`
--
ALTER TABLE `actKeyTypes`
  ADD PRIMARY KEY (`keyID`);

--
-- Indexes for table `agtAgents`
--
ALTER TABLE `agtAgents`
  ADD PRIMARY KEY (`agentID`),
  ADD KEY `agtAgents_IX_corporation` (`corporationID`),
  ADD KEY `agtAgents_IX_station` (`locationID`),
  ADD KEY `divisionID` (`divisionID`),
  ADD KEY `agentTypeID` (`agentTypeID`);

--
-- Indexes for table `agtAgentTypes`
--
ALTER TABLE `agtAgentTypes`
  ADD PRIMARY KEY (`agentTypeID`);

--
-- Indexes for table `agtMissions`
--
ALTER TABLE `agtMissions`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `agtOffers`
--
ALTER TABLE `agtOffers`
  ADD PRIMARY KEY (`offerID`);

--
-- Indexes for table `agtSkillLevel`
--
ALTER TABLE `agtSkillLevel`
  ADD PRIMARY KEY (`agentID`,`typeID`),
  ADD KEY `agtResearchAgents_IX_type` (`typeID`);

--
-- Indexes for table `alnAlliance`
--
ALTER TABLE `alnAlliance`
  ADD PRIMARY KEY (`allianceID`),
  ADD UNIQUE KEY `allianceID` (`allianceID`);

--
-- Indexes for table `alnApplications`
--
ALTER TABLE `alnApplications`
  ADD PRIMARY KEY (`applicationID`),
  ADD UNIQUE KEY `appID` (`applicationID`),
  ADD KEY `corporationID` (`corporationID`),
  ADD KEY `allianceID` (`allianceID`);

--
-- Indexes for table `alnContacts`
--
ALTER TABLE `alnContacts`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `alnLabels`
--
ALTER TABLE `alnLabels`
  ADD PRIMARY KEY (`labelID`);

--
-- Indexes for table `billsPayable`
--
ALTER TABLE `billsPayable`
  ADD PRIMARY KEY (`billID`);

--
-- Indexes for table `billsReceivable`
--
ALTER TABLE `billsReceivable`
  ADD PRIMARY KEY (`billID`);

--
-- Indexes for table `billTypes`
--
ALTER TABLE `billTypes`
  ADD PRIMARY KEY (`billTypeID`);

--
-- Indexes for table `bloodlineTypes`
--
ALTER TABLE `bloodlineTypes`
  ADD PRIMARY KEY (`bloodlineID`),
  ADD KEY `typeID` (`typeID`);

--
-- Indexes for table `bookmarkFolders`
--
ALTER TABLE `bookmarkFolders`
  ADD PRIMARY KEY (`folderID`),
  ADD KEY `ownerID` (`ownerID`);

--
-- Indexes for table `bookmarks`
--
ALTER TABLE `bookmarks`
  ADD PRIMARY KEY (`bookmarkID`);

--
-- Indexes for table `careers`
--
ALTER TABLE `careers`
  ADD PRIMARY KEY (`id`),
  ADD KEY `careerID` (`careerID`),
  ADD KEY `graphicID` (`graphicID`),
  ADD KEY `schoolID` (`schoolID`),
  ADD KEY `iconID` (`iconID`),
  ADD KEY `dataID` (`dataID`);

--
-- Indexes for table `channelMods`
--
ALTER TABLE `channelMods`
  ADD PRIMARY KEY (`id`),
  ADD KEY `FK_CHANNELMODS_CHANNELS` (`channelID`);

--
-- Indexes for table `channels`
--
ALTER TABLE `channels`
  ADD PRIMARY KEY (`channelID`);

--
-- Indexes for table `chrAccessories`
--
ALTER TABLE `chrAccessories`
  ADD PRIMARY KEY (`accessoryID`);

--
-- Indexes for table `chrAncestries`
--
ALTER TABLE `chrAncestries`
  ADD PRIMARY KEY (`ancestryID`),
  ADD KEY `bloodlineID` (`bloodlineID`),
  ADD KEY `iconID` (`iconID`),
  ADD KEY `ancestryNameID` (`ancestryNameID`),
  ADD KEY `descriptionID` (`descriptionID`),
  ADD KEY `dataID` (`dataID`);

--
-- Indexes for table `chrAttributes`
--
ALTER TABLE `chrAttributes`
  ADD PRIMARY KEY (`attributeID`),
  ADD KEY `iconID` (`iconID`);

--
-- Indexes for table `chrBackgrounds`
--
ALTER TABLE `chrBackgrounds`
  ADD PRIMARY KEY (`backgroundID`);

--
-- Indexes for table `chrBeards`
--
ALTER TABLE `chrBeards`
  ADD PRIMARY KEY (`beardID`);

--
-- Indexes for table `chrBLAccessories`
--
ALTER TABLE `chrBLAccessories`
  ADD PRIMARY KEY (`bloodlineID`,`gender`,`accessoryID`);

--
-- Indexes for table `chrBLBackgrounds`
--
ALTER TABLE `chrBLBackgrounds`
  ADD PRIMARY KEY (`backgroundID`);

--
-- Indexes for table `chrBLBeards`
--
ALTER TABLE `chrBLBeards`
  ADD PRIMARY KEY (`bloodlineID`,`gender`,`beardID`);

--
-- Indexes for table `chrBLCostumes`
--
ALTER TABLE `chrBLCostumes`
  ADD PRIMARY KEY (`bloodlineID`,`gender`,`costumeID`);

--
-- Indexes for table `chrBLDecos`
--
ALTER TABLE `chrBLDecos`
  ADD PRIMARY KEY (`bloodlineID`,`gender`,`decoID`);

--
-- Indexes for table `chrBLEyebrows`
--
ALTER TABLE `chrBLEyebrows`
  ADD PRIMARY KEY (`bloodlineID`,`gender`,`eyebrowsID`);

--
-- Indexes for table `chrBLEyes`
--
ALTER TABLE `chrBLEyes`
  ADD PRIMARY KEY (`bloodlineID`,`gender`,`eyesID`);

--
-- Indexes for table `chrBLHairs`
--
ALTER TABLE `chrBLHairs`
  ADD PRIMARY KEY (`bloodlineID`,`gender`,`hairID`);

--
-- Indexes for table `chrBLLights`
--
ALTER TABLE `chrBLLights`
  ADD PRIMARY KEY (`lightID`);

--
-- Indexes for table `chrBLLipsticks`
--
ALTER TABLE `chrBLLipsticks`
  ADD PRIMARY KEY (`bloodlineID`,`gender`,`lipstickID`);

--
-- Indexes for table `chrBLMakeups`
--
ALTER TABLE `chrBLMakeups`
  ADD PRIMARY KEY (`bloodlineID`,`gender`,`makeupID`);

--
-- Indexes for table `chrBloodlineNames`
--
ALTER TABLE `chrBloodlineNames`
  ADD PRIMARY KEY (`nameID`),
  ADD KEY `bloodlineID` (`bloodlineID`);

--
-- Indexes for table `chrBloodlines`
--
ALTER TABLE `chrBloodlines`
  ADD PRIMARY KEY (`bloodlineID`),
  ADD KEY `raceID` (`raceID`),
  ADD KEY `shipTypeID` (`shipTypeID`),
  ADD KEY `corporationID` (`corporationID`),
  ADD KEY `iconID` (`iconID`),
  ADD KEY `bloodlineNameID` (`bloodlineNameID`),
  ADD KEY `descriptionID` (`descriptionID`),
  ADD KEY `dataID` (`dataID`);

--
-- Indexes for table `chrBLSkins`
--
ALTER TABLE `chrBLSkins`
  ADD PRIMARY KEY (`bloodlineID`,`gender`,`skinID`);

--
-- Indexes for table `chrCertificates`
--
ALTER TABLE `chrCertificates`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `id` (`id`);

--
-- Indexes for table `chrCharacters`
--
ALTER TABLE `chrCharacters`
  ADD PRIMARY KEY (`characterID`),
  ADD KEY `FK_CHARACTER__ACCOUNTS` (`accountID`),
  ADD KEY `FK_CHARACTER__CHRANCESTRIES` (`ancestryID`),
  ADD KEY `FK_CHARACTER__CHRCAREERS` (`careerID`),
  ADD KEY `FK_CHARACTER__CHRCAREERSPECIALITIES` (`careerSpecialityID`),
  ADD KEY `FK_CHARACTER__CHRSCHOOLS` (`schoolID`),
  ADD KEY `characterID` (`characterID`,`accountID`),
  ADD KEY `characterID_2` (`characterID`),
  ADD KEY `accountID` (`accountID`);

--
-- Indexes for table `chrContacts`
--
ALTER TABLE `chrContacts`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `chrCostumes`
--
ALTER TABLE `chrCostumes`
  ADD PRIMARY KEY (`costumeID`);

--
-- Indexes for table `chrDecos`
--
ALTER TABLE `chrDecos`
  ADD PRIMARY KEY (`decoID`);

--
-- Indexes for table `chrDefaultOverviewGroups`
--
ALTER TABLE `chrDefaultOverviewGroups`
  ADD PRIMARY KEY (`id`),
  ADD KEY `groupID` (`groupID`);

--
-- Indexes for table `chrDefaultOverviews`
--
ALTER TABLE `chrDefaultOverviews`
  ADD PRIMARY KEY (`overviewID`),
  ADD KEY `overviewNameID` (`overviewNameID`),
  ADD KEY `dataID` (`dataID`);

--
-- Indexes for table `chrDepartments`
--
ALTER TABLE `chrDepartments`
  ADD PRIMARY KEY (`departmentID`);

--
-- Indexes for table `chrEmployment`
--
ALTER TABLE `chrEmployment`
  ADD PRIMARY KEY (`characterID`,`corporationID`,`startDate`) USING BTREE,
  ADD KEY `corporationID` (`corporationID`) USING BTREE;

--
-- Indexes for table `chrEyebrows`
--
ALTER TABLE `chrEyebrows`
  ADD PRIMARY KEY (`eyebrowsID`);

--
-- Indexes for table `chrEyes`
--
ALTER TABLE `chrEyes`
  ADD PRIMARY KEY (`eyesID`);

--
-- Indexes for table `chrHairs`
--
ALTER TABLE `chrHairs`
  ADD PRIMARY KEY (`hairID`);

--
-- Indexes for table `chrKillTable`
--
ALTER TABLE `chrKillTable`
  ADD PRIMARY KEY (`killID`),
  ADD KEY `victimCharacterID` (`victimCharacterID`),
  ADD KEY `finalCharacterID` (`finalCharacterID`);

--
-- Indexes for table `chrLabels`
--
ALTER TABLE `chrLabels`
  ADD PRIMARY KEY (`labelID`);

--
-- Indexes for table `chrLights`
--
ALTER TABLE `chrLights`
  ADD PRIMARY KEY (`lightID`);

--
-- Indexes for table `chrLipsticks`
--
ALTER TABLE `chrLipsticks`
  ADD PRIMARY KEY (`lipstickID`);

--
-- Indexes for table `chrMakeups`
--
ALTER TABLE `chrMakeups`
  ADD PRIMARY KEY (`makeupID`);

--
-- Indexes for table `chrNotes`
--
ALTER TABLE `chrNotes`
  ADD PRIMARY KEY (`itemID`,`ownerID`);

--
-- Indexes for table `chrNPCCharacters`
--
ALTER TABLE `chrNPCCharacters`
  ADD PRIMARY KEY (`characterID`),
  ADD UNIQUE KEY `characterName` (`characterName`),
  ADD KEY `accountID` (`accountID`),
  ADD KEY `typeID` (`typeID`),
  ADD KEY `accessoryID` (`accessoryID`),
  ADD KEY `ancestryID` (`ancestryID`),
  ADD KEY `beardID` (`beardID`),
  ADD KEY `careerID` (`careerID`),
  ADD KEY `careerSpecialityID` (`careerSpecialityID`),
  ADD KEY `costumeID` (`costumeID`),
  ADD KEY `decoID` (`decoID`),
  ADD KEY `eyebrowsID` (`eyebrowsID`),
  ADD KEY `eyesID` (`eyesID`),
  ADD KEY `hairID` (`hairID`),
  ADD KEY `lipstickID` (`lipstickID`),
  ADD KEY `makeupID` (`makeupID`),
  ADD KEY `schoolID` (`schoolID`),
  ADD KEY `skinID` (`skinID`),
  ADD KEY `backgroundID` (`backgroundID`),
  ADD KEY `lightID` (`lightID`),
  ADD KEY `characterID` (`characterID`);

--
-- Indexes for table `chrOwnerNote`
--
ALTER TABLE `chrOwnerNote`
  ADD UNIQUE KEY `noteID` (`noteID`);

--
-- Indexes for table `chrRaces`
--
ALTER TABLE `chrRaces`
  ADD PRIMARY KEY (`raceID`),
  ADD KEY `iconID` (`iconID`);

--
-- Indexes for table `chrSchools`
--
ALTER TABLE `chrSchools`
  ADD PRIMARY KEY (`id`),
  ADD KEY `schoolID` (`schoolID`),
  ADD KEY `graphicID` (`graphicID`),
  ADD KEY `corporationID` (`corporationID`),
  ADD KEY `agentID` (`agentID`),
  ADD KEY `newAgentID` (`newAgentID`),
  ADD KEY `iconID` (`iconID`),
  ADD KEY `schoolNameID` (`schoolNameID`),
  ADD KEY `descriptionID` (`descriptionID`);

--
-- Indexes for table `chrShipFittings`
--
ALTER TABLE `chrShipFittings`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `chrSkillHistory`
--
ALTER TABLE `chrSkillHistory`
  ADD UNIQUE KEY `ai` (`ai`);

--
-- Indexes for table `chrSkins`
--
ALTER TABLE `chrSkins`
  ADD PRIMARY KEY (`skinID`);

--
-- Indexes for table `chrVisitedSystems`
--
ALTER TABLE `chrVisitedSystems`
  ADD PRIMARY KEY (`characterID`,`solarSystemID`);

--
-- Indexes for table `crpActivities`
--
ALTER TABLE `crpActivities`
  ADD PRIMARY KEY (`activityID`);

--
-- Indexes for table `crpAdGroupData`
--
ALTER TABLE `crpAdGroupData`
  ADD PRIMARY KEY (`groupID`);

--
-- Indexes for table `crpAdRegistry`
--
ALTER TABLE `crpAdRegistry`
  ADD PRIMARY KEY (`adID`);

--
-- Indexes for table `crpAdTypeData`
--
ALTER TABLE `crpAdTypeData`
  ADD PRIMARY KEY (`typeMask`);

--
-- Indexes for table `crpApplications`
--
ALTER TABLE `crpApplications`
  ADD PRIMARY KEY (`applicationID`),
  ADD UNIQUE KEY `appID` (`applicationID`),
  ADD KEY `corporationID` (`corporationID`),
  ADD KEY `characterID` (`characterID`);

--
-- Indexes for table `crpAutoPay`
--
ALTER TABLE `crpAutoPay`
  ADD PRIMARY KEY (`corporationID`);

--
-- Indexes for table `crpBulletins`
--
ALTER TABLE `crpBulletins`
  ADD PRIMARY KEY (`bulletinID`);

--
-- Indexes for table `crpContacts`
--
ALTER TABLE `crpContacts`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `crpCorporation`
--
ALTER TABLE `crpCorporation`
  ADD PRIMARY KEY (`corporationID`),
  ADD KEY `stationID` (`stationID`),
  ADD KEY `corporationID` (`corporationID`),
  ADD KEY `allianceID` (`allianceID`);

--
-- Indexes for table `crpEmployment`
--
ALTER TABLE `crpEmployment`
  ADD PRIMARY KEY (`allianceID`,`corporationID`,`startDate`) USING BTREE,
  ADD KEY `allianceID` (`allianceID`) USING BTREE;

--
-- Indexes for table `crpItemEvent`
--
ALTER TABLE `crpItemEvent`
  ADD PRIMARY KEY (`eventID`),
  ADD UNIQUE KEY `id_2` (`eventID`),
  ADD KEY `id` (`eventID`);

--
-- Indexes for table `crpLabels`
--
ALTER TABLE `crpLabels`
  ADD PRIMARY KEY (`labelID`);

--
-- Indexes for table `crpLockedItems`
--
ALTER TABLE `crpLockedItems`
  ADD PRIMARY KEY (`itemID`);

--
-- Indexes for table `crpMedals`
--
ALTER TABLE `crpMedals`
  ADD PRIMARY KEY (`medalID`);

--
-- Indexes for table `crpMedalStatus`
--
ALTER TABLE `crpMedalStatus`
  ADD PRIMARY KEY (`ai`);

--
-- Indexes for table `crpNPCCorporationDivisions`
--
ALTER TABLE `crpNPCCorporationDivisions`
  ADD PRIMARY KEY (`corporationID`,`divisionID`),
  ADD KEY `divisionID` (`divisionID`);

--
-- Indexes for table `crpNPCCorporationResearchFields`
--
ALTER TABLE `crpNPCCorporationResearchFields`
  ADD PRIMARY KEY (`skillID`,`corporationID`),
  ADD KEY `corporationID` (`corporationID`);

--
-- Indexes for table `crpNPCCorporations`
--
ALTER TABLE `crpNPCCorporations`
  ADD PRIMARY KEY (`corporationID`),
  ADD KEY `solarSystemID` (`solarSystemID`),
  ADD KEY `iconID` (`iconID`),
  ADD KEY `factionID` (`factionID`),
  ADD KEY `investorID1` (`investorID1`),
  ADD KEY `investorID2` (`investorID2`),
  ADD KEY `investorID3` (`investorID3`),
  ADD KEY `investorID4` (`investorID4`),
  ADD KEY `friendID` (`friendID`),
  ADD KEY `enemyID` (`enemyID`),
  ADD KEY `corporationID` (`corporationID`);

--
-- Indexes for table `crpNPCCorporationTrades`
--
ALTER TABLE `crpNPCCorporationTrades`
  ADD PRIMARY KEY (`corporationID`,`typeID`),
  ADD KEY `typeID` (`typeID`);

--
-- Indexes for table `crpNPCDivisions`
--
ALTER TABLE `crpNPCDivisions`
  ADD PRIMARY KEY (`divisionID`);

--
-- Indexes for table `crpNPCWalletDivisons`
--
ALTER TABLE `crpNPCWalletDivisons`
  ADD PRIMARY KEY (`corporationID`);

--
-- Indexes for table `crpRecruiters`
--
ALTER TABLE `crpRecruiters`
  ADD KEY `adID` (`adID`),
  ADD KEY `corpID` (`corpID`);

--
-- Indexes for table `crpRoleGroups`
--
ALTER TABLE `crpRoleGroups`
  ADD PRIMARY KEY (`roleGroupID`),
  ADD UNIQUE KEY `roleGroupID` (`roleGroupID`);

--
-- Indexes for table `crpRoleHistroy`
--
ALTER TABLE `crpRoleHistroy`
  ADD KEY `id` (`id`);

--
-- Indexes for table `crpRoles`
--
ALTER TABLE `crpRoles`
  ADD PRIMARY KEY (`roleIID`),
  ADD UNIQUE KEY `roleIID` (`roleIID`);

--
-- Indexes for table `crpShares`
--
ALTER TABLE `crpShares`
  ADD PRIMARY KEY (`id`,`shareholderID`),
  ADD KEY `shareholderID` (`shareholderID`);

--
-- Indexes for table `crpVoteItems`
--
ALTER TABLE `crpVoteItems`
  ADD PRIMARY KEY (`voteCaseID`);

--
-- Indexes for table `crpVoteOptions`
--
ALTER TABLE `crpVoteOptions`
  ADD PRIMARY KEY (`ai`);

--
-- Indexes for table `crpWalletDivisons`
--
ALTER TABLE `crpWalletDivisons`
  ADD PRIMARY KEY (`corporationID`);

--
-- Indexes for table `crtCategories`
--
ALTER TABLE `crtCategories`
  ADD PRIMARY KEY (`categoryID`);

--
-- Indexes for table `crtCertificates`
--
ALTER TABLE `crtCertificates`
  ADD PRIMARY KEY (`certificateID`),
  ADD KEY `crtCertificates_IX_category` (`categoryID`),
  ADD KEY `crtCertificates_IX_class` (`classID`),
  ADD KEY `corpID` (`corpID`),
  ADD KEY `iconID` (`iconID`);

--
-- Indexes for table `crtClasses`
--
ALTER TABLE `crtClasses`
  ADD PRIMARY KEY (`classID`);

--
-- Indexes for table `crtRecommendations`
--
ALTER TABLE `crtRecommendations`
  ADD PRIMARY KEY (`recommendationID`),
  ADD KEY `crtRecommendations_IX_certificate` (`certificateID`),
  ADD KEY `crtRecommendations_IX_shipType` (`shipTypeID`);

--
-- Indexes for table `crtRelationships`
--
ALTER TABLE `crtRelationships`
  ADD PRIMARY KEY (`relationshipID`),
  ADD KEY `crtRelationships_IX_child` (`childID`),
  ADD KEY `crtRelationships_IX_parent` (`parentID`),
  ADD KEY `parentTypeID` (`parentTypeID`);

--
-- Indexes for table `CruciblePriceHistory`
--
ALTER TABLE `CruciblePriceHistory`
  ADD KEY `typeID` (`typeID`),
  ADD KEY `regionID` (`regionID`),
  ADD KEY `regionID_2` (`regionID`);

--
-- Indexes for table `CruciblePriceHistory_Materials`
--
ALTER TABLE `CruciblePriceHistory_Materials`
  ADD KEY `regionID` (`regionID`),
  ADD KEY `avgPrice` (`avgPrice`),
  ADD KEY `typeID` (`typeID`);

--
-- Indexes for table `dgmAttributeCategories`
--
ALTER TABLE `dgmAttributeCategories`
  ADD PRIMARY KEY (`categoryID`);

--
-- Indexes for table `dgmAttributeTypes`
--
ALTER TABLE `dgmAttributeTypes`
  ADD PRIMARY KEY (`attributeID`);

--
-- Indexes for table `dgmEffects`
--
ALTER TABLE `dgmEffects`
  ADD PRIMARY KEY (`effectID`);

--
-- Indexes for table `dgmExpressions`
--
ALTER TABLE `dgmExpressions`
  ADD PRIMARY KEY (`expressionID`);

--
-- Indexes for table `dgmOperands`
--
ALTER TABLE `dgmOperands`
  ADD PRIMARY KEY (`operandID`),
  ADD KEY `arg1categoryID` (`arg1categoryID`),
  ADD KEY `arg2categoryID` (`arg2categoryID`),
  ADD KEY `resultCategoryID` (`resultCategoryID`);

--
-- Indexes for table `dgmTypeAttributes`
--
ALTER TABLE `dgmTypeAttributes`
  ADD PRIMARY KEY (`typeID`,`attributeID`);

--
-- Indexes for table `dgmTypeEffects`
--
ALTER TABLE `dgmTypeEffects`
  ADD PRIMARY KEY (`typeID`,`effectID`);

--
-- Indexes for table `dgmUnits`
--
ALTER TABLE `dgmUnits`
  ADD PRIMARY KEY (`unitID`);

--
-- Indexes for table `droneState`
--
ALTER TABLE `droneState`
  ADD PRIMARY KEY (`droneID`);

--
-- Indexes for table `dunActive`
--
ALTER TABLE `dunActive`
  ADD KEY `systemID` (`systemID`);

--
-- Indexes for table `dunEntryData`
--
ALTER TABLE `dunEntryData`
  ADD KEY `dunEntryID` (`dunEntryID`);

--
-- Indexes for table `dunGroupData`
--
ALTER TABLE `dunGroupData`
  ADD PRIMARY KEY (`ai`),
  ADD KEY `dunGroupID` (`dunGroupID`);

--
-- Indexes for table `dunRoomData`
--
ALTER TABLE `dunRoomData`
  ADD PRIMARY KEY (`ai`),
  ADD KEY `dunRoomID` (`dunRoomID`);

--
-- Indexes for table `dunSpawnType`
--
ALTER TABLE `dunSpawnType`
  ADD KEY `dunSpawnTypeID` (`dunSpawnTypeID`);

--
-- Indexes for table `dunTemplates`
--
ALTER TABLE `dunTemplates`
  ADD UNIQUE KEY `dunTemplateID_2` (`dunTemplateID`),
  ADD KEY `dunTemplateID` (`dunTemplateID`);

--
-- Indexes for table `entity`
--
ALTER TABLE `entity`
  ADD PRIMARY KEY (`itemID`),
  ADD KEY `typeID` (`typeID`),
  ADD KEY `itemID` (`itemID`),
  ADD KEY `ownerID` (`ownerID`),
  ADD KEY `locationID` (`locationID`);

--
-- Indexes for table `entity_attributes`
--
ALTER TABLE `entity_attributes`
  ADD PRIMARY KEY (`itemID`,`attributeID`),
  ADD KEY `attributeID` (`attributeID`);

--
-- Indexes for table `eveGraphics`
--
ALTER TABLE `eveGraphics`
  ADD PRIMARY KEY (`graphicID`),
  ADD KEY `explosionID` (`explosionID`);

--
-- Indexes for table `eveIcons`
--
ALTER TABLE `eveIcons`
  ADD PRIMARY KEY (`iconID`);

--
-- Indexes for table `eveMail`
--
ALTER TABLE `eveMail`
  ADD PRIMARY KEY (`messageID`),
  ADD KEY `messageID` (`messageID`),
  ADD KEY `senderID` (`senderID`);

--
-- Indexes for table `eveMailDetails`
--
ALTER TABLE `eveMailDetails`
  ADD PRIMARY KEY (`attachmentID`),
  ADD KEY `messageID` (`messageID`);

--
-- Indexes for table `eveMailMimeType`
--
ALTER TABLE `eveMailMimeType`
  ADD PRIMARY KEY (`mimeTypeID`);

--
-- Indexes for table `eveStaticLocations`
--
ALTER TABLE `eveStaticLocations`
  ADD PRIMARY KEY (`locationID`);

--
-- Indexes for table `eveStaticOwners`
--
ALTER TABLE `eveStaticOwners`
  ADD PRIMARY KEY (`ownerID`),
  ADD KEY `typeID` (`typeID`);

--
-- Indexes for table `eveUnits`
--
ALTER TABLE `eveUnits`
  ADD PRIMARY KEY (`unitID`);

--
-- Indexes for table `facFactions`
--
ALTER TABLE `facFactions`
  ADD PRIMARY KEY (`factionID`),
  ADD KEY `militiaCorporationID` (`militiaCorporationID`),
  ADD KEY `corporationID` (`corporationID`),
  ADD KEY `solarSystemID` (`solarSystemID`),
  ADD KEY `iconID` (`iconID`);

--
-- Indexes for table `facRaces`
--
ALTER TABLE `facRaces`
  ADD PRIMARY KEY (`factionID`,`raceID`);

--
-- Indexes for table `facWarSystems`
--
ALTER TABLE `facWarSystems`
  ADD PRIMARY KEY (`systemID`),
  ADD UNIQUE KEY `systemID` (`systemID`);

--
-- Indexes for table `graphics`
--
ALTER TABLE `graphics`
  ADD PRIMARY KEY (`graphicID`),
  ADD KEY `explosionID` (`explosionID`),
  ADD KEY `directoryID` (`directoryID`);

--
-- Indexes for table `icons`
--
ALTER TABLE `icons`
  ADD PRIMARY KEY (`iconID`);

--
-- Indexes for table `intro`
--
ALTER TABLE `intro`
  ADD KEY `textLabel` (`textLabel`);

--
-- Indexes for table `invBlueprints`
--
ALTER TABLE `invBlueprints`
  ADD PRIMARY KEY (`itemID`),
  ADD KEY `itemID` (`itemID`);

--
-- Indexes for table `invBlueprintTypes`
--
ALTER TABLE `invBlueprintTypes`
  ADD PRIMARY KEY (`blueprintTypeID`),
  ADD KEY `parentBlueprintTypeID` (`parentBlueprintTypeID`),
  ADD KEY `productTypeID` (`productTypeID`);

--
-- Indexes for table `invCategories`
--
ALTER TABLE `invCategories`
  ADD PRIMARY KEY (`categoryID`),
  ADD UNIQUE KEY `categoryID` (`categoryID`);

--
-- Indexes for table `invContrabandTypes`
--
ALTER TABLE `invContrabandTypes`
  ADD PRIMARY KEY (`factionID`,`typeID`),
  ADD KEY `invContrabandTypes_IX_type` (`typeID`);

--
-- Indexes for table `invControlTowerResourcePurposes`
--
ALTER TABLE `invControlTowerResourcePurposes`
  ADD PRIMARY KEY (`purpose`);

--
-- Indexes for table `invControlTowerResources`
--
ALTER TABLE `invControlTowerResources`
  ADD PRIMARY KEY (`controlTowerTypeID`,`resourceTypeID`),
  ADD KEY `resourceTypeID` (`resourceTypeID`),
  ADD KEY `factionID` (`factionID`),
  ADD KEY `purpose` (`purpose`);

--
-- Indexes for table `invFlags`
--
ALTER TABLE `invFlags`
  ADD PRIMARY KEY (`flagID`);

--
-- Indexes for table `invGroups`
--
ALTER TABLE `invGroups`
  ADD PRIMARY KEY (`groupID`),
  ADD KEY `invGroups_IX_category` (`categoryID`),
  ADD KEY `iconID` (`iconID`),
  ADD KEY `groupID` (`groupID`);

--
-- Indexes for table `invMarketGroups`
--
ALTER TABLE `invMarketGroups`
  ADD PRIMARY KEY (`marketGroupID`),
  ADD KEY `parentGroupID` (`parentGroupID`),
  ADD KEY `marketGroupID` (`marketGroupID`),
  ADD KEY `graphicID` (`graphicID`),
  ADD KEY `iconID` (`iconID`),
  ADD KEY `dataID` (`dataID`),
  ADD KEY `marketGroupNameID` (`marketGroupNameID`),
  ADD KEY `descriptionID` (`descriptionID`);

--
-- Indexes for table `invMetaGroups`
--
ALTER TABLE `invMetaGroups`
  ADD PRIMARY KEY (`metaGroupID`),
  ADD KEY `iconID` (`iconID`);

--
-- Indexes for table `invMetaTypes`
--
ALTER TABLE `invMetaTypes`
  ADD PRIMARY KEY (`typeID`),
  ADD KEY `parentTypeID` (`parentTypeID`),
  ADD KEY `metaGroupID` (`metaGroupID`);

--
-- Indexes for table `invTypeMaterials`
--
ALTER TABLE `invTypeMaterials`
  ADD PRIMARY KEY (`typeID`,`materialTypeID`);

--
-- Indexes for table `invTypeReactions`
--
ALTER TABLE `invTypeReactions`
  ADD PRIMARY KEY (`reactionTypeID`,`input`,`typeID`),
  ADD KEY `typeID` (`typeID`);

--
-- Indexes for table `invTypes`
--
ALTER TABLE `invTypes`
  ADD PRIMARY KEY (`typeID`),
  ADD KEY `groupID` (`groupID`),
  ADD KEY `graphicID` (`graphicID`),
  ADD KEY `raceID` (`raceID`),
  ADD KEY `marketGroupID` (`marketGroupID`),
  ADD KEY `soundID` (`soundID`),
  ADD KEY `iconID` (`iconID`),
  ADD KEY `dataID` (`dataID`),
  ADD KEY `typeNameID` (`typeNameID`),
  ADD KEY `descriptionID` (`descriptionID`),
  ADD KEY `typeID` (`typeID`),
  ADD KEY `marketGroupID_2` (`marketGroupID`);

--
-- Indexes for table `invTypesToWrecks`
--
ALTER TABLE `invTypesToWrecks`
  ADD PRIMARY KEY (`typeID`),
  ADD KEY `wreckTypeID` (`wreckTypeID`),
  ADD KEY `typeID` (`typeID`);

--
-- Indexes for table `jnlCharacters`
--
ALTER TABLE `jnlCharacters`
  ADD PRIMARY KEY (`transactionID`),
  ADD KEY `ownerID` (`ownerID`);

--
-- Indexes for table `jnlCorporations`
--
ALTER TABLE `jnlCorporations`
  ADD PRIMARY KEY (`transactionID`),
  ADD KEY `ownerID` (`ownerID`);

--
-- Indexes for table `jnlEntryTypeIDs`
--
ALTER TABLE `jnlEntryTypeIDs`
  ADD PRIMARY KEY (`entryTypeID`);

--
-- Indexes for table `languages`
--
ALTER TABLE `languages`
  ADD PRIMARY KEY (`languageID`);

--
-- Indexes for table `liveupdates`
--
ALTER TABLE `liveupdates`
  ADD PRIMARY KEY (`updateID`);

--
-- Indexes for table `locationScenes`
--
ALTER TABLE `locationScenes`
  ADD PRIMARY KEY (`locationID`),
  ADD KEY `sceneID` (`sceneID`);

--
-- Indexes for table `lpRequiredItems`
--
ALTER TABLE `lpRequiredItems`
  ADD KEY `parentID` (`parentID`),
  ADD KEY `typeID` (`typeID`),
  ADD KEY `parentID_2` (`parentID`),
  ADD KEY `typeID_2` (`typeID`),
  ADD KEY `parentID_3` (`parentID`),
  ADD KEY `typeID_3` (`typeID`);

--
-- Indexes for table `lpStore`
--
ALTER TABLE `lpStore`
  ADD PRIMARY KEY (`storeID`),
  ADD KEY `corporationID` (`corporationID`),
  ADD KEY `typeID` (`typeID`),
  ADD KEY `typeID_2` (`typeID`),
  ADD KEY `corporationID_2` (`corporationID`);

--
-- Indexes for table `lpVerified`
--
ALTER TABLE `lpVerified`
  ADD PRIMARY KEY (`corporationID`),
  ADD KEY `verification` (`verification`);

--
-- Indexes for table `mailLabel`
--
ALTER TABLE `mailLabel`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `mailList`
--
ALTER TABLE `mailList`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `mailListUsers`
--
ALTER TABLE `mailListUsers`
  ADD PRIMARY KEY (`listID`);

--
-- Indexes for table `mailMessage`
--
ALTER TABLE `mailMessage`
  ADD PRIMARY KEY (`messageID`);

--
-- Indexes for table `mailStatus`
--
ALTER TABLE `mailStatus`
  ADD KEY `messageID` (`messageID`);

--
-- Indexes for table `mapCelestialDescriptions`
--
ALTER TABLE `mapCelestialDescriptions`
  ADD PRIMARY KEY (`celestialID`);

--
-- Indexes for table `mapCelestialStatistics`
--
ALTER TABLE `mapCelestialStatistics`
  ADD PRIMARY KEY (`celestialID`);

--
-- Indexes for table `mapConnections`
--
ALTER TABLE `mapConnections`
  ADD PRIMARY KEY (`ai`),
  ADD UNIQUE KEY `stargateID` (`stargateID`),
  ADD UNIQUE KEY `celestialID` (`celestialID`),
  ADD KEY `stargateID_2` (`stargateID`),
  ADD KEY `celestialID_2` (`celestialID`);

--
-- Indexes for table `mapConstellationJumps`
--
ALTER TABLE `mapConstellationJumps`
  ADD PRIMARY KEY (`fromConstellationID`,`toConstellationID`),
  ADD KEY `mapConstellationJumps_IX_fromRegion` (`fromRegionID`),
  ADD KEY `toConstellationID` (`toConstellationID`,`toRegionID`),
  ADD KEY `fromConstellationID` (`fromConstellationID`,`fromRegionID`);

--
-- Indexes for table `mapConstellations`
--
ALTER TABLE `mapConstellations`
  ADD PRIMARY KEY (`constellationID`),
  ADD UNIQUE KEY `constellationID` (`constellationID`,`regionID`),
  ADD KEY `mapConstellations_IX_region` (`regionID`),
  ADD KEY `factionID` (`factionID`);

--
-- Indexes for table `mapDenormalize`
--
ALTER TABLE `mapDenormalize`
  ADD PRIMARY KEY (`itemID`),
  ADD KEY `mapDenormalize_IX_constellation` (`constellationID`),
  ADD KEY `mapDenormalize_IX_groupConstellation` (`groupID`,`constellationID`),
  ADD KEY `mapDenormalize_IX_groupRegion` (`groupID`,`regionID`),
  ADD KEY `mapDenormalize_IX_groupSystem` (`groupID`,`solarSystemID`),
  ADD KEY `mapDenormalize_IX_orbit` (`orbitID`),
  ADD KEY `mapDenormalize_IX_region` (`regionID`),
  ADD KEY `mapDenormalize_IX_system` (`solarSystemID`),
  ADD KEY `typeID` (`typeID`),
  ADD KEY `itemID` (`itemID`);

--
-- Indexes for table `mapDynamicData`
--
ALTER TABLE `mapDynamicData`
  ADD UNIQUE KEY `solarSystemID` (`solarSystemID`);

--
-- Indexes for table `mapJumps`
--
ALTER TABLE `mapJumps`
  ADD PRIMARY KEY (`stargateID`),
  ADD KEY `celestialID` (`celestialID`),
  ADD KEY `stargateID` (`stargateID`);

--
-- Indexes for table `mapLandmarks`
--
ALTER TABLE `mapLandmarks`
  ADD PRIMARY KEY (`landmarkID`),
  ADD KEY `locationID` (`locationID`),
  ADD KEY `iconID` (`iconID`);

--
-- Indexes for table `mapLocationScenes`
--
ALTER TABLE `mapLocationScenes`
  ADD PRIMARY KEY (`locationID`);

--
-- Indexes for table `mapLocationWormholeClasses`
--
ALTER TABLE `mapLocationWormholeClasses`
  ADD PRIMARY KEY (`locationID`);

--
-- Indexes for table `mapRegionJumps`
--
ALTER TABLE `mapRegionJumps`
  ADD PRIMARY KEY (`fromRegionID`,`toRegionID`),
  ADD KEY `toRegionID` (`toRegionID`);

--
-- Indexes for table `mapRegions`
--
ALTER TABLE `mapRegions`
  ADD PRIMARY KEY (`regionID`),
  ADD KEY `factionID` (`factionID`);

--
-- Indexes for table `mapSolarSystemJumps`
--
ALTER TABLE `mapSolarSystemJumps`
  ADD PRIMARY KEY (`fromSolarSystemID`,`toSolarSystemID`),
  ADD KEY `mapSolarSystemJumps_IX_fromConstellation` (`fromConstellationID`),
  ADD KEY `mapSolarSystemJumps_IX_fromRegion` (`fromRegionID`),
  ADD KEY `fromSolarSystemID` (`fromSolarSystemID`,`fromConstellationID`,`fromRegionID`),
  ADD KEY `toSolarSystemID` (`toSolarSystemID`,`toConstellationID`,`toRegionID`);

--
-- Indexes for table `mapSolarSystems`
--
ALTER TABLE `mapSolarSystems`
  ADD PRIMARY KEY (`solarSystemID`),
  ADD UNIQUE KEY `solarSystemID` (`solarSystemID`,`constellationID`,`regionID`),
  ADD KEY `mapSolarSystems_IX_constellation` (`constellationID`),
  ADD KEY `mapSolarSystems_IX_region` (`regionID`),
  ADD KEY `mapSolarSystems_IX_security` (`security`),
  ADD KEY `factionID` (`factionID`),
  ADD KEY `sunTypeID` (`sunTypeID`),
  ADD KEY `solarSystemID_2` (`solarSystemID`),
  ADD KEY `factionID_2` (`factionID`);

--
-- Indexes for table `mapUniverse`
--
ALTER TABLE `mapUniverse`
  ADD PRIMARY KEY (`universeID`);

--
-- Indexes for table `mktData`
--
ALTER TABLE `mktData`
  ADD KEY `ownerID` (`ownerID`),
  ADD KEY `typeID` (`typeID`),
  ADD KEY `date` (`date`),
  ADD KEY `stationID` (`stationID`),
  ADD KEY `regionID` (`regionID`);

--
-- Indexes for table `mktHistory`
--
ALTER TABLE `mktHistory`
  ADD KEY `regionID` (`regionID`),
  ADD KEY `typeID` (`typeID`);

--
-- Indexes for table `mktOrders`
--
ALTER TABLE `mktOrders`
  ADD PRIMARY KEY (`orderID`),
  ADD KEY `typeID` (`typeID`),
  ADD KEY `regionID` (`regionID`),
  ADD KEY `stationID` (`stationID`),
  ADD KEY `orderID` (`orderID`);

--
-- Indexes for table `mktTransactions`
--
ALTER TABLE `mktTransactions`
  ADD PRIMARY KEY (`transactionID`),
  ADD KEY `regionID` (`regionID`),
  ADD KEY `transactionID` (`transactionID`),
  ADD KEY `typeID` (`typeID`),
  ADD KEY `transactionType` (`transactionType`),
  ADD KEY `clientID` (`clientID`);

--
-- Indexes for table `mktUpdates`
--
ALTER TABLE `mktUpdates`
  ADD PRIMARY KEY (`server`);

--
-- Indexes for table `npcSpawnClass`
--
ALTER TABLE `npcSpawnClass`
  ADD PRIMARY KEY (`ai`);

--
-- Indexes for table `ownerIcons`
--
ALTER TABLE `ownerIcons`
  ADD PRIMARY KEY (`ownerID`),
  ADD KEY `iconID` (`iconID`);

--
-- Indexes for table `paperdollColorNames`
--
ALTER TABLE `paperdollColorNames`
  ADD PRIMARY KEY (`colorNameID`);

--
-- Indexes for table `paperdollColorRestrictions`
--
ALTER TABLE `paperdollColorRestrictions`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `paperdollColors`
--
ALTER TABLE `paperdollColors`
  ADD PRIMARY KEY (`colorID`);

--
-- Indexes for table `paperdollModifierLocations`
--
ALTER TABLE `paperdollModifierLocations`
  ADD PRIMARY KEY (`modifierLocationID`);

--
-- Indexes for table `paperdollResources`
--
ALTER TABLE `paperdollResources`
  ADD PRIMARY KEY (`paperdollResourceID`);

--
-- Indexes for table `paperdollSculptingLocations`
--
ALTER TABLE `paperdollSculptingLocations`
  ADD PRIMARY KEY (`sculptLocationID`);

--
-- Indexes for table `piCCPin`
--
ALTER TABLE `piCCPin`
  ADD UNIQUE KEY `pinID` (`pinID`);

--
-- Indexes for table `piECUHeads`
--
ALTER TABLE `piECUHeads`
  ADD PRIMARY KEY (`ccPinID`,`ownerID`,`ecuID`,`headID`);

--
-- Indexes for table `piLaunches`
--
ALTER TABLE `piLaunches`
  ADD UNIQUE KEY `launchID` (`launchID`);

--
-- Indexes for table `piLinks`
--
ALTER TABLE `piLinks`
  ADD PRIMARY KEY (`linkID`);

--
-- Indexes for table `piPinContents`
--
ALTER TABLE `piPinContents`
  ADD PRIMARY KEY (`pinID`,`typeID`);

--
-- Indexes for table `piPinMap`
--
ALTER TABLE `piPinMap`
  ADD PRIMARY KEY (`schematicID`,`pinTypeID`),
  ADD KEY `pinTypeID` (`pinTypeID`);

--
-- Indexes for table `piPins`
--
ALTER TABLE `piPins`
  ADD PRIMARY KEY (`pinID`);

--
-- Indexes for table `piPlanets`
--
ALTER TABLE `piPlanets`
  ADD PRIMARY KEY (`charID`,`planetID`);

--
-- Indexes for table `piRoutes`
--
ALTER TABLE `piRoutes`
  ADD PRIMARY KEY (`routeID`);

--
-- Indexes for table `piSchematics`
--
ALTER TABLE `piSchematics`
  ADD PRIMARY KEY (`schematicID`);

--
-- Indexes for table `piTypeMap`
--
ALTER TABLE `piTypeMap`
  ADD PRIMARY KEY (`schematicID`,`typeID`),
  ADD KEY `typeID` (`typeID`);

--
-- Indexes for table `posCustomsOfficeData`
--
ALTER TABLE `posCustomsOfficeData`
  ADD PRIMARY KEY (`itemID`),
  ADD UNIQUE KEY `itemID` (`itemID`),
  ADD KEY `itemID_2` (`itemID`),
  ADD KEY `ownerID` (`ownerID`);

--
-- Indexes for table `posJumpBridgeData`
--
ALTER TABLE `posJumpBridgeData`
  ADD PRIMARY KEY (`itemID`),
  ADD KEY `corpID` (`corpID`),
  ADD KEY `allyID` (`allyID`),
  ADD KEY `systemID` (`systemID`);

--
-- Indexes for table `posStructureData`
--
ALTER TABLE `posStructureData`
  ADD PRIMARY KEY (`itemID`),
  ADD KEY `itemID` (`itemID`),
  ADD KEY `towerID` (`towerID`),
  ADD KEY `moonID` (`moonID`);

--
-- Indexes for table `posTowerData`
--
ALTER TABLE `posTowerData`
  ADD PRIMARY KEY (`itemID`),
  ADD KEY `itemID` (`itemID`);

--
-- Indexes for table `qstCourier`
--
ALTER TABLE `qstCourier`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `qstMining`
--
ALTER TABLE `qstMining`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `ramActivities`
--
ALTER TABLE `ramActivities`
  ADD PRIMARY KEY (`activityID`);

--
-- Indexes for table `ramAssemblyLines`
--
ALTER TABLE `ramAssemblyLines`
  ADD PRIMARY KEY (`assemblyLineID`),
  ADD KEY `ramAssemblyLines_IX_container` (`containerID`),
  ADD KEY `ramAssemblyLines_IX_owner` (`ownerID`),
  ADD KEY `assemblyLineTypeID` (`assemblyLineTypeID`),
  ADD KEY `activityID` (`activityID`),
  ADD KEY `assemblyLineID` (`assemblyLineID`);

--
-- Indexes for table `ramAssemblyLineStationCostLogs`
--
ALTER TABLE `ramAssemblyLineStationCostLogs`
  ADD PRIMARY KEY (`assemblyLineTypeID`),
  ADD KEY `stationID` (`stationID`);

--
-- Indexes for table `ramAssemblyLineStations`
--
ALTER TABLE `ramAssemblyLineStations`
  ADD PRIMARY KEY (`stationID`,`assemblyLineTypeID`),
  ADD KEY `ramAssemblyLineStations_IX_owner` (`ownerID`),
  ADD KEY `ramAssemblyLineStations_IX_region` (`regionID`),
  ADD KEY `assemblyLineTypeID` (`assemblyLineTypeID`),
  ADD KEY `stationTypeID` (`stationTypeID`),
  ADD KEY `solarSystemID` (`solarSystemID`);

--
-- Indexes for table `ramAssemblyLineTypeDetailPerCategory`
--
ALTER TABLE `ramAssemblyLineTypeDetailPerCategory`
  ADD PRIMARY KEY (`assemblyLineTypeID`,`categoryID`),
  ADD KEY `categoryID` (`categoryID`);

--
-- Indexes for table `ramAssemblyLineTypeDetailPerGroup`
--
ALTER TABLE `ramAssemblyLineTypeDetailPerGroup`
  ADD PRIMARY KEY (`assemblyLineTypeID`,`groupID`),
  ADD KEY `groupID` (`groupID`);

--
-- Indexes for table `ramAssemblyLineTypes`
--
ALTER TABLE `ramAssemblyLineTypes`
  ADD PRIMARY KEY (`assemblyLineTypeID`),
  ADD KEY `activityID` (`activityID`);

--
-- Indexes for table `ramCompletedStatuses`
--
ALTER TABLE `ramCompletedStatuses`
  ADD PRIMARY KEY (`completedStatusID`);

--
-- Indexes for table `ramInstallationTypeContents`
--
ALTER TABLE `ramInstallationTypeContents`
  ADD PRIMARY KEY (`installationTypeID`,`assemblyLineTypeID`),
  ADD KEY `assemblyLineTypeID` (`assemblyLineTypeID`);

--
-- Indexes for table `ramInstallationTypeDefaultContents`
--
ALTER TABLE `ramInstallationTypeDefaultContents`
  ADD PRIMARY KEY (`installationTypeID`),
  ADD KEY `assemblyLineTypeID` (`assemblyLineTypeID`);

--
-- Indexes for table `ramJobs`
--
ALTER TABLE `ramJobs`
  ADD PRIMARY KEY (`jobID`),
  ADD KEY `RAMJOBS_ASSEMBLYLINES` (`assemblyLineID`);

--
-- Indexes for table `ramTypeRequirements`
--
ALTER TABLE `ramTypeRequirements`
  ADD PRIMARY KEY (`typeID`,`activityID`,`requiredTypeID`);

--
-- Indexes for table `rentalInfo`
--
ALTER TABLE `rentalInfo`
  ADD PRIMARY KEY (`stationID`,`slotNumber`);

--
-- Indexes for table `repFactions`
--
ALTER TABLE `repFactions`
  ADD PRIMARY KEY (`fromID`,`toID`);

--
-- Indexes for table `repStandingChanges`
--
ALTER TABLE `repStandingChanges`
  ADD PRIMARY KEY (`eventID`),
  ADD KEY `fromID` (`fromID`),
  ADD KEY `toID` (`toID`);

--
-- Indexes for table `repStandings`
--
ALTER TABLE `repStandings`
  ADD PRIMARY KEY (`fromID`,`toID`);

--
-- Indexes for table `roidDistribution`
--
ALTER TABLE `roidDistribution`
  ADD PRIMARY KEY (`AI`),
  ADD KEY `systemSec` (`systemSec`);

--
-- Indexes for table `schematics`
--
ALTER TABLE `schematics`
  ADD PRIMARY KEY (`schematicID`),
  ADD KEY `dataID` (`dataID`);

--
-- Indexes for table `schematicsPinMap`
--
ALTER TABLE `schematicsPinMap`
  ADD PRIMARY KEY (`id`),
  ADD KEY `pinTypeID` (`pinTypeID`);

--
-- Indexes for table `schematicsTypeMap`
--
ALTER TABLE `schematicsTypeMap`
  ADD PRIMARY KEY (`id`),
  ADD KEY `typeID` (`typeID`);

--
-- Indexes for table `shipInsurance`
--
ALTER TABLE `shipInsurance`
  ADD PRIMARY KEY (`shipID`),
  ADD UNIQUE KEY `shipID` (`shipID`),
  ADD KEY `ownerID` (`ownerID`);

--
-- Indexes for table `shipTypes`
--
ALTER TABLE `shipTypes`
  ADD PRIMARY KEY (`shipTypeID`),
  ADD KEY `weaponTypeID` (`weaponTypeID`),
  ADD KEY `miningTypeID` (`miningTypeID`),
  ADD KEY `skillTypeID` (`skillTypeID`);

--
-- Indexes for table `sklBaseSkills`
--
ALTER TABLE `sklBaseSkills`
  ADD PRIMARY KEY (`ID`);

--
-- Indexes for table `sklCareerSkills`
--
ALTER TABLE `sklCareerSkills`
  ADD PRIMARY KEY (`careerID`,`skillTypeID`);

--
-- Indexes for table `sklRaceSkills`
--
ALTER TABLE `sklRaceSkills`
  ADD PRIMARY KEY (`id`),
  ADD KEY `skillTypeID` (`skillTypeID`);

--
-- Indexes for table `sounds`
--
ALTER TABLE `sounds`
  ADD PRIMARY KEY (`soundID`);

--
-- Indexes for table `specialities`
--
ALTER TABLE `specialities`
  ADD PRIMARY KEY (`id`),
  ADD KEY `specialityID` (`specialityID`),
  ADD KEY `graphicID` (`graphicID`),
  ADD KEY `iconID` (`iconID`),
  ADD KEY `dataID` (`dataID`);

--
-- Indexes for table `specialitySkills`
--
ALTER TABLE `specialitySkills`
  ADD PRIMARY KEY (`specialityID`,`skillTypeID`);

--
-- Indexes for table `srvStatisticData`
--
ALTER TABLE `srvStatisticData`
  ADD PRIMARY KEY (`timeStamp`);

--
-- Indexes for table `srvStatisticHistory`
--
ALTER TABLE `srvStatisticHistory`
  ADD UNIQUE KEY `timeStamp` (`idx`);

--
-- Indexes for table `srvStatus`
--
ALTER TABLE `srvStatus`
  ADD PRIMARY KEY (`AI`),
  ADD UNIQUE KEY `AI` (`AI`);

--
-- Indexes for table `staOffices`
--
ALTER TABLE `staOffices`
  ADD PRIMARY KEY (`itemID`),
  ADD KEY `officeID` (`itemID`);

--
-- Indexes for table `staOperations`
--
ALTER TABLE `staOperations`
  ADD PRIMARY KEY (`operationID`),
  ADD KEY `activityID` (`activityID`),
  ADD KEY `caldariStationTypeID` (`caldariStationTypeID`),
  ADD KEY `minmatarStationTypeID` (`minmatarStationTypeID`),
  ADD KEY `amarrStationTypeID` (`amarrStationTypeID`),
  ADD KEY `gallenteStationTypeID` (`gallenteStationTypeID`),
  ADD KEY `joveStationTypeID` (`joveStationTypeID`);

--
-- Indexes for table `staOperationServices`
--
ALTER TABLE `staOperationServices`
  ADD PRIMARY KEY (`operationID`,`serviceID`),
  ADD KEY `serviceID` (`serviceID`);

--
-- Indexes for table `staServices`
--
ALTER TABLE `staServices`
  ADD PRIMARY KEY (`serviceID`);

--
-- Indexes for table `staStations`
--
ALTER TABLE `staStations`
  ADD PRIMARY KEY (`stationID`),
  ADD KEY `staStations_IX_constellation` (`constellationID`),
  ADD KEY `staStations_IX_corporation` (`corporationID`),
  ADD KEY `staStations_IX_operation` (`operationID`),
  ADD KEY `staStations_IX_region` (`regionID`),
  ADD KEY `staStations_IX_system` (`solarSystemID`),
  ADD KEY `staStations_IX_type` (`stationTypeID`),
  ADD KEY `solarSystemID` (`solarSystemID`,`constellationID`,`regionID`),
  ADD KEY `stationID` (`stationID`);

--
-- Indexes for table `staStationTypes`
--
ALTER TABLE `staStationTypes`
  ADD PRIMARY KEY (`stationTypeID`),
  ADD KEY `operationID` (`operationID`);

--
-- Indexes for table `sysAsteroids`
--
ALTER TABLE `sysAsteroids`
  ADD PRIMARY KEY (`itemID`),
  ADD UNIQUE KEY `itemID` (`itemID`),
  ADD KEY `systemID` (`systemID`),
  ADD KEY `beltID` (`beltID`);

--
-- Indexes for table `sysCalendarEvents`
--
ALTER TABLE `sysCalendarEvents`
  ADD UNIQUE KEY `eventID` (`eventID`);

--
-- Indexes for table `sysSignatures`
--
ALTER TABLE `sysSignatures`
  ADD PRIMARY KEY (`sigID`),
  ADD UNIQUE KEY `sigID` (`sigID`),
  ADD KEY `systemID` (`systemID`),
  ADD KEY `sigItemID` (`sigItemID`);

--
-- Indexes for table `test`
--
ALTER TABLE `test`
  ADD PRIMARY KEY (`ai`);

--
-- Indexes for table `translationTables`
--
ALTER TABLE `translationTables`
  ADD PRIMARY KEY (`sourceTable`,`translatedKey`);

--
-- Indexes for table `trnTranslationColumns`
--
ALTER TABLE `trnTranslationColumns`
  ADD PRIMARY KEY (`tcID`);

--
-- Indexes for table `trnTranslationLanguages`
--
ALTER TABLE `trnTranslationLanguages`
  ADD PRIMARY KEY (`numericLanguageID`);

--
-- Indexes for table `trnTranslations`
--
ALTER TABLE `trnTranslations`
  ADD PRIMARY KEY (`tcID`,`keyID`,`languageID`);

--
-- Indexes for table `tutorials`
--
ALTER TABLE `tutorials`
  ADD PRIMARY KEY (`tutorialID`);

--
-- Indexes for table `tutorials_criterias`
--
ALTER TABLE `tutorials_criterias`
  ADD PRIMARY KEY (`tutorialID`,`criteriaID`),
  ADD KEY `criteriaID` (`criteriaID`);

--
-- Indexes for table `tutorial_categories`
--
ALTER TABLE `tutorial_categories`
  ADD PRIMARY KEY (`categoryID`);

--
-- Indexes for table `tutorial_criteria`
--
ALTER TABLE `tutorial_criteria`
  ADD PRIMARY KEY (`criteriaID`);

--
-- Indexes for table `tutorial_pages`
--
ALTER TABLE `tutorial_pages`
  ADD PRIMARY KEY (`pageID`),
  ADD UNIQUE KEY `tutorialID` (`tutorialID`,`pageNumber`);

--
-- Indexes for table `tutorial_page_criteria`
--
ALTER TABLE `tutorial_page_criteria`
  ADD PRIMARY KEY (`pageID`,`criteriaID`);

--
-- Indexes for table `warCombatZones`
--
ALTER TABLE `warCombatZones`
  ADD PRIMARY KEY (`combatZoneID`),
  ADD KEY `factionID` (`factionID`),
  ADD KEY `centerSystemID` (`centerSystemID`);

--
-- Indexes for table `warCombatZoneSystems`
--
ALTER TABLE `warCombatZoneSystems`
  ADD PRIMARY KEY (`solarSystemID`),
  ADD KEY `combatZoneID` (`combatZoneID`);

--
-- Indexes for table `webBounties`
--
ALTER TABLE `webBounties`
  ADD PRIMARY KEY (`timePlaced`),
  ADD KEY `ownerID` (`ownerID`),
  ADD KEY `timePlaced` (`timePlaced`),
  ADD KEY `bounty` (`bounty`),
  ADD KEY `characterID` (`characterID`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `account`
--
ALTER TABLE `account`
  MODIFY `accountID` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `agtOffers`
--
ALTER TABLE `agtOffers`
  MODIFY `offerID` int(10) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `alnAlliance`
--
ALTER TABLE `alnAlliance`
  MODIFY `allianceID` int(11) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `alnApplications`
--
ALTER TABLE `alnApplications`
  MODIFY `applicationID` int(5) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `alnContacts`
--
ALTER TABLE `alnContacts`
  MODIFY `id` int(10) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `alnLabels`
--
ALTER TABLE `alnLabels`
  MODIFY `labelID` int(10) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `bookmarkFolders`
--
ALTER TABLE `bookmarkFolders`
  MODIFY `folderID` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `bookmarks`
--
ALTER TABLE `bookmarks`
  MODIFY `bookmarkID` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `careers`
--
ALTER TABLE `careers`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `channelMods`
--
ALTER TABLE `channelMods`
  MODIFY `id` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `channels`
--
ALTER TABLE `channels`
  MODIFY `channelID` int(10) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `chrCertificates`
--
ALTER TABLE `chrCertificates`
  MODIFY `id` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `chrCharacters`
--
ALTER TABLE `chrCharacters`
  MODIFY `characterID` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `chrContacts`
--
ALTER TABLE `chrContacts`
  MODIFY `id` int(10) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `chrDefaultOverviewGroups`
--
ALTER TABLE `chrDefaultOverviewGroups`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `chrKillTable`
--
ALTER TABLE `chrKillTable`
  MODIFY `killID` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `chrLabels`
--
ALTER TABLE `chrLabels`
  MODIFY `labelID` int(10) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `chrOwnerNote`
--
ALTER TABLE `chrOwnerNote`
  MODIFY `noteID` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `chrSchools`
--
ALTER TABLE `chrSchools`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `chrShipFittings`
--
ALTER TABLE `chrShipFittings`
  MODIFY `id` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `chrSkillHistory`
--
ALTER TABLE `chrSkillHistory`
  MODIFY `ai` int(10) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `crpAdRegistry`
--
ALTER TABLE `crpAdRegistry`
  MODIFY `adID` int(10) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `crpApplications`
--
ALTER TABLE `crpApplications`
  MODIFY `applicationID` int(5) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `crpBulletins`
--
ALTER TABLE `crpBulletins`
  MODIFY `bulletinID` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `crpContacts`
--
ALTER TABLE `crpContacts`
  MODIFY `id` int(10) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `crpCorporation`
--
ALTER TABLE `crpCorporation`
  MODIFY `corporationID` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `crpItemEvent`
--
ALTER TABLE `crpItemEvent`
  MODIFY `eventID` int(10) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `crpLabels`
--
ALTER TABLE `crpLabels`
  MODIFY `labelID` int(10) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `crpMedals`
--
ALTER TABLE `crpMedals`
  MODIFY `medalID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `crpMedalStatus`
--
ALTER TABLE `crpMedalStatus`
  MODIFY `ai` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `crpRoleHistroy`
--
ALTER TABLE `crpRoleHistroy`
  MODIFY `id` int(10) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `crpShares`
--
ALTER TABLE `crpShares`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `crpVoteItems`
--
ALTER TABLE `crpVoteItems`
  MODIFY `voteCaseID` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `crpVoteOptions`
--
ALTER TABLE `crpVoteOptions`
  MODIFY `ai` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `dunGroupData`
--
ALTER TABLE `dunGroupData`
  MODIFY `ai` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `dunRoomData`
--
ALTER TABLE `dunRoomData`
  MODIFY `ai` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `entity`
--
ALTER TABLE `entity`
  MODIFY `itemID` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `eveMail`
--
ALTER TABLE `eveMail`
  MODIFY `messageID` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `eveMailDetails`
--
ALTER TABLE `eveMailDetails`
  MODIFY `attachmentID` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `eveMailMimeType`
--
ALTER TABLE `eveMailMimeType`
  MODIFY `mimeTypeID` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `jnlCharacters`
--
ALTER TABLE `jnlCharacters`
  MODIFY `transactionID` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `jnlCorporations`
--
ALTER TABLE `jnlCorporations`
  MODIFY `transactionID` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `lpStore`
--
ALTER TABLE `lpStore`
  MODIFY `storeID` int(5) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `mailLabel`
--
ALTER TABLE `mailLabel`
  MODIFY `id` int(10) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `mailList`
--
ALTER TABLE `mailList`
  MODIFY `id` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `mailMessage`
--
ALTER TABLE `mailMessage`
  MODIFY `messageID` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `mapConnections`
--
ALTER TABLE `mapConnections`
  MODIFY `ai` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `mktOrders`
--
ALTER TABLE `mktOrders`
  MODIFY `orderID` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `mktTransactions`
--
ALTER TABLE `mktTransactions`
  MODIFY `transactionID` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `npcSpawnClass`
--
ALTER TABLE `npcSpawnClass`
  MODIFY `ai` smallint(4) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `paperdollColorRestrictions`
--
ALTER TABLE `paperdollColorRestrictions`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `piLaunches`
--
ALTER TABLE `piLaunches`
  MODIFY `launchID` int(10) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `piRoutes`
--
ALTER TABLE `piRoutes`
  MODIFY `routeID` smallint(6) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `ramJobs`
--
ALTER TABLE `ramJobs`
  MODIFY `jobID` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `repStandingChanges`
--
ALTER TABLE `repStandingChanges`
  MODIFY `eventID` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `roidDistribution`
--
ALTER TABLE `roidDistribution`
  MODIFY `AI` int(5) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `schematicsPinMap`
--
ALTER TABLE `schematicsPinMap`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `schematicsTypeMap`
--
ALTER TABLE `schematicsTypeMap`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `sklBaseSkills`
--
ALTER TABLE `sklBaseSkills`
  MODIFY `ID` tinyint(4) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `sklRaceSkills`
--
ALTER TABLE `sklRaceSkills`
  MODIFY `id` int(10) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `specialities`
--
ALTER TABLE `specialities`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `srvStatus`
--
ALTER TABLE `srvStatus`
  MODIFY `AI` int(10) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `staOffices`
--
ALTER TABLE `staOffices`
  MODIFY `itemID` int(10) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `sysAsteroids`
--
ALTER TABLE `sysAsteroids`
  MODIFY `itemID` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `sysCalendarEvents`
--
ALTER TABLE `sysCalendarEvents`
  MODIFY `eventID` int(10) unsigned NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `test`
--
ALTER TABLE `test`
  MODIFY `ai` int(11) NOT NULL AUTO_INCREMENT;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
