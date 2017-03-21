
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
  `z` double NOT NULL,
  PRIMARY KEY (`itemID`),
  UNIQUE KEY `itemID` (`itemID`),
  KEY `systemID` (`systemID`),
  KEY `beltID` (`beltID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


 /* Table structure for table `shipInsurance` */

CREATE TABLE IF NOT EXISTS `shipInsurance` (
  `shipID` int(10) NOT NULL,
  `shipName` varchar(150) COLLATE utf8_unicode_ci NOT NULL,
  `ownerID` int(10) NOT NULL,
  `startDate` bigint(20) NOT NULL,
  `endDate` bigint(20) NOT NULL,
  `fraction` float(4,3) NOT NULL,
  `payOutAmount` int(10) NOT NULL DEFAULT '0',
  `isCorpItem` tinyint(1) NOT NULL,
  PRIMARY KEY (`shipID`),
  UNIQUE KEY `shipID` (`shipID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;


/* Table structure for table `chrPausedSkillQueue` */

CREATE TABLE IF NOT EXISTS `chrPausedSkillQueue` (
  `characterID` int(10) unsigned NOT NULL,
  `orderIndex` int(10) unsigned NOT NULL,
  `typeID` int(10) unsigned NOT NULL,
  `level` int(10) unsigned NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `chrPausedSkillQueue` */

/* Table structure for table `webBounties` */

CREATE TABLE IF NOT EXISTS `webBounties` (
  `characterID` int(10) NOT NULL,
  `ownerID` int(10) NOT NULL,
  `bounty` bigint(20) NOT NULL,
  `timePlaced` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`timePlaced`),
  KEY `ownerID` (`ownerID`),
  KEY `timePlaced` (`timePlaced`),
  KEY `bounty` (`bounty`),
  KEY `characterID` (`characterID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

/* Table structure for table `mapDynamicData`  */

CREATE TABLE IF NOT EXISTS `mapDynamicData` (
  `solarSystemID` int(10) NOT NULL,
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
  `pilotsDateTime` bigint(20) DEFAULT NULL,
  `jumpsDateTime` bigint(20) DEFAULT NULL,
  `killsDateTime` bigint(20) DEFAULT NULL,
  `kills24DateTime` bigint(20) DEFAULT NULL,
  `podDateTime` bigint(20) DEFAULT NULL,
  `pod24DateTime` bigint(20) DEFAULT NULL,
  `factionDateTime` bigint(20) DEFAULT NULL,
  `faction24DateTime` bigint(20) DEFAULT NULL,
  UNIQUE KEY `solarSystemID` (`solarSystemID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

/*  Table structure for table `chrVisitedSystems` */

CREATE TABLE IF NOT EXISTS `chrVisitedSystems` (
  `idx` int(10) NOT NULL AUTO_INCREMENT,
  `characterID` int(20) NOT NULL,
  `solarSystemID` int(10) NOT NULL,
  `visits` int(10) NOT NULL DEFAULT '0',
  `lastDateTime` bigint(20) unsigned NOT NULL,
  PRIMARY KEY `idx` (`idx`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

/*  Table structure for table `chrSkillHistory`  */

CREATE TABLE IF NOT EXISTS `chrSkillHistory` (
  `eventTypeID` smallint(6) NOT NULL,
  `characterID` int(10) NOT NULL,
  `logDate` bigint(20) NOT NULL,
  `skillTypeID` int(8) NOT NULL,
  `skillLevel` tinyint(4) NOT NULL,
  `relativePoints` bigint(20) NOT NULL,
  `absolutePoints` bigint(20) NOT NULL,
  `AI` int(10) NOT NULL AUTO_INCREMENT,
  UNIQUE KEY `AI` (`AI`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COMMENT='Char Skill History' AUTO_INCREMENT=1 ;

/* Table structure for table `mapConnections`  */

CREATE TABLE IF NOT EXISTS `mapConnections` (
  `ctype` int(10) unsigned NOT NULL,
  `fromreg` int(10) unsigned NOT NULL,
  `fromcon` int(10) unsigned DEFAULT NULL,
  `fromsol` int(10) unsigned DEFAULT NULL,
  `stargateID` int(10) unsigned DEFAULT NULL,
  `celestialID` int(10) unsigned DEFAULT NULL,
  `tosol` int(10) unsigned DEFAULT NULL,
  `tocon` int(10) unsigned DEFAULT NULL,
  `toreg` int(10) unsigned NOT NULL,
  UNIQUE KEY `stargateID` (`stargateID`),
  UNIQUE KEY `celestialID` (`celestialID`),
  KEY `stargateID_2` (`stargateID`),
  KEY `celestialID_2` (`celestialID`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci ;

/* Table structure for table `roidDistribution`  */

CREATE TABLE IF NOT EXISTS `roidDistribution` (
  `AI` int(5) NOT NULL AUTO_INCREMENT,
  `systemSec` varchar(2) NOT NULL,
  `roidID` int(10) unsigned NOT NULL,
  `roidName` varchar(20) NOT NULL,
  `percent` float NOT NULL,
  PRIMARY KEY (`AI`),
  KEY `systemSec` (`systemSec`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=1;


DROP TABLE IF EXISTS `srvStatus`;

/* Table structure for table `srvStatus */

CREATE TABLE IF NOT EXISTS `srvStatus` (
  `AI` int(10) NOT NULL AUTO_INCREMENT,
  `srvName` varchar(60) NOT NULL,
  `Online` tinyint(1) NOT NULL,
  `startTime` bigint(20) NOT NULL,
  `ClientSeed` INT(10) NOT NULL,
  `Connections` smallint(6) NOT NULL,
  `threads` tinyint(4) NOT NULL,
  `rss` decimal(6,3) NOT NULL,
  `vm` decimal(6,3) NOT NULL,
  `user` decimal(4,2) NOT NULL,
  `kernel` decimal(4,2) NOT NULL,
  `items` int(10) NOT NULL,
  `systems` int(10) NOT NULL,
  `bubbles` int(10) NOT NULL,
  `updateTime` int(10) NOT NULL,
  `npcs` int(10) NOT NULL,
  PRIMARY KEY (`AI`),
  UNIQUE KEY `AI` (`AI`)
  ) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

/* Table structure for table `sklBaseSkills` */

CREATE TABLE IF NOT EXISTS `sklBaseSkills` (
  `ID` tinyint(4) NOT NULL AUTO_INCREMENT,
  `skillTypeID` smallint(6) NOT NULL,
  `level` tinyint(4) NOT NULL,
  PRIMARY KEY (`ID`)
  ) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='Basic Skills for All Races' AUTO_INCREMENT=10 ;

/* Table structure for table `sklCareerSkills` */

CREATE TABLE IF NOT EXISTS `sklCareerSkills` (
  `careerID` int(10) NOT NULL DEFAULT '0',
  `skillTypeID` int(10) NOT NULL DEFAULT '0',
  `level` tinyint(3) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`careerID`,`skillTypeID`)
  ) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='skill and level list by careerID';

/* Table structure for table `sklRaceSkills` */

CREATE TABLE IF NOT EXISTS `sklRaceSkills` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `raceID` int(10) DEFAULT NULL,
  `skillTypeID` int(10) DEFAULT NULL,
  `level` tinyint(3) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `skillTypeID` (`skillTypeID`)
  ) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='skill and level list by raceID' AUTO_INCREMENT=33 ;

CREATE TABLE IF NOT EXISTS `mapSystemSovInfo` (
  `solarSystemID` int(10) NOT NULL,
  `corporationID` int(10) NOT NULL,
  `allianceID` int(10) NOT NULL,
  `claimStructureID` int(10) NOT NULL,
  `claimTime` int(20) NOT NULL,
  `hubID` int(10) NOT NULL,
  `contested` tinyint(1) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='SystemSovereigntyInfo';

/* Table structure for table `crpAlliance` */

CREATE TABLE IF NOT EXISTS `crpAlliance` (
  `allianceID` int(10) NOT NULL,
  `allianceType` int(10) NOT NULL,
  `allianceShortName` varchar(20) NOT NULL,
  PRIMARY KEY (`allianceID`),
  KEY `allianceID` (`allianceID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*  new tables for Standings */

CREATE TABLE IF NOT EXISTS `repAgent` (
  `fromID` int(10) unsigned NOT NULL DEFAULT '0',
  `toID` int(10) unsigned NOT NULL DEFAULT '0',
  `standing` double NOT NULL DEFAULT '0',
  PRIMARY KEY (`toID`,`fromID`),
  KEY `fromID` (`fromID`),
  KEY `toID` (`toID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='from agents to characters';

CREATE TABLE IF NOT EXISTS `repAlliance` (
  `fromID` int(10) unsigned NOT NULL DEFAULT '0',
  `toID` int(10) unsigned NOT NULL DEFAULT '0',
  `standing` double NOT NULL DEFAULT '0',
  PRIMARY KEY (`toID`,`fromID`),
  KEY `fromID` (`fromID`),
  KEY `toID` (`toID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `repChar` (
  `fromID` int(10) unsigned NOT NULL DEFAULT '0',
  `toID` int(10) unsigned NOT NULL DEFAULT '0',
  `standing` double NOT NULL DEFAULT '0',
  PRIMARY KEY (`toID`,`fromID`),
  KEY `fromID` (`fromID`),
  KEY `toID` (`toID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='char to others';

CREATE TABLE IF NOT EXISTS `repCorp` (
  `fromID` int(10) unsigned NOT NULL DEFAULT '0',
  `toID` int(10) unsigned NOT NULL DEFAULT '0',
  `standing` double NOT NULL DEFAULT '0',
  PRIMARY KEY (`toID`,`fromID`),
  KEY `fromID` (`fromID`),
  KEY `toID` (`toID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `repNPCCorp` (
  `fromID` int(10) unsigned NOT NULL DEFAULT '0',
  `toID` int(10) unsigned NOT NULL DEFAULT '0',
  `standing` double NOT NULL DEFAULT '0',
  PRIMARY KEY (`toID`,`fromID`),
  KEY `fromID` (`fromID`),
  KEY `toID` (`toID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='npc corp to char for refining, agents, and pos charters';

CREATE TABLE IF NOT EXISTS `repFactions` (
  `fromID` int(10) unsigned NOT NULL DEFAULT '0',
  `toID` int(10) unsigned NOT NULL DEFAULT '0',
  `standing` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`fromID`,`toID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `repStandingChanges` (
  `eventID` int(10) unsigned NOT NULL AUTO_INCREMENT,
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
  `msg` text NOT NULL,
  PRIMARY KEY (`eventID`),
  KEY `fromID` (`fromID`),
  KEY `toID` (`toID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

CREATE TABLE `chrKillTable` (
  `killID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `solarSystemID` int(10) unsigned NOT NULL DEFAULT '0',
  `victimCharacterID` int(10) unsigned NOT NULL DEFAULT '0',
  `victimCorporationID` int(10) unsigned NOT NULL DEFAULT '0',
  `victimAllianceID` int(10) unsigned NOT NULL DEFAULT '0',
  `victimFactionID` int(10) unsigned NOT NULL DEFAULT '0',
  `victimShipTypeID` smallint(4) unsigned NOT NULL DEFAULT '0',
  `victimDamageTaken` int(10) unsigned NOT NULL DEFAULT '0',
  `finalCharacterID` int(10) unsigned NOT NULL DEFAULT '0',
  `finalCorporationID` int(10) unsigned NOT NULL DEFAULT '0',
  `finalAllianceID` int(10) unsigned NOT NULL DEFAULT '0',
  `finalFactionID` int(10) unsigned NOT NULL DEFAULT '0',
  `finalShipTypeID` smallint(4) unsigned NOT NULL DEFAULT '0',
  `finalWeaponTypeID` smallint(4) unsigned NOT NULL DEFAULT '0',
  `finalSecurityStatus` double NOT NULL DEFAULT '0',
  `finalDamageDone` int(10) unsigned NOT NULL DEFAULT '0',
  `killBlob` blob NOT NULL,
  `killTime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `moonID` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`killID`),
  KEY `victimCharacterID` (`victimCharacterID`),
  KEY `finalCharacterID` (`finalCharacterID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1;

-- Table structure for table `chrShipFittings`
--
CREATE TABLE `chrShipFittings` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `characterID` int(10) unsigned NOT NULL,
  `shipID` int(10) unsigned NOT NULL,
  `shipDNA` tinytext NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 COMMENT='Ship Stored Fittings, saved as ShipDNA';

DROP TABLE IF EXISTS `chrRaces`;
CREATE TABLE `chrRaces` (
  `raceID` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `raceName` varchar(100) DEFAULT NULL,
  `description` varchar(1000) DEFAULT NULL,
  `iconID` smallint(6) NOT NULL DEFAULT '0',
  `shortDescription` varchar(500) DEFAULT NULL,
  `raceNameID` int(10) NOT NULL DEFAULT '0',
  `descriptionID` int(10) NOT NULL DEFAULT '0',
  `dataID` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`raceID`),
  KEY `iconID` (`iconID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `invCategories`;
CREATE TABLE `invCategories` (
  `categoryID` int(10) NOT NULL,
  `categoryName` varchar(100) DEFAULT NULL,
  `description` varchar(3000) DEFAULT NULL,
  `published` tinyint(1) NOT NULL DEFAULT '0',
  `iconID` smallint(6) NOT NULL DEFAULT '0',
  `categoryNameID` int(8) NOT NULL DEFAULT '0',
  `dataID` int(8) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;


/* set initial client seed in db */
UPDATE `srvStatus` SET `ClientSeed` = '10101' WHERE `AI` = 1;
/* fix for db type error (datetime not handled in evemu) and add default '0' NOT NULL */
ALTER TABLE `ramAssemblyLines` CHANGE `nextFreeTime` `nextFreeTime` BIGINT(20) NOT NULL DEFAULT 0;
  /*  hack for minor client error...we dont have the real data for this yet  */
ALTER TABLE `staOperations` ADD `descriptionID` INT(3) NOT NULL DEFAULT '0' AFTER `description`;

/* update `crtCategories` to add categoryNameID and dataID found in cache */
ALTER TABLE `crtCategories` ADD `categoryNameID` int(10) unsigned DEFAULT '0';
ALTER TABLE `crtCategories` ADD `dataID` int(10) unsigned DEFAULT '0';

/* fix radius' in mapDenormalize */
UPDATE `mapDenormalize` SET `radius`=1 WHERE `groupID`=9;
UPDATE `mapDenormalize` AS md INNER JOIN `invTypes` AS it USING (typeID) SET md.radius = it.radius WHERE md.groupID = 10;

/* add rat factions to mapRegions table for belt rat spawns */
ALTER TABLE `mapRegions` ADD `ratFactionID` INT(8) NOT NULL DEFAULT '0' AFTER `factionID`;

/* set correct radius for these laser modules.  more missing data also */
UPDATE `invTypes` SET `radius` = '100' WHERE `typeID` = 12346;
UPDATE `invTypes` SET `radius` = '1000' WHERE `typeID` = 12356;

/* not sure how many of these are wrong... */
UPDATE `mapDenormalize` SET `radius` = '343000' WHERE `mapDenormalize`.`itemID` = 40003343;