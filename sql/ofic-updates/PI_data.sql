


CREATE TABLE `chrPlanetCCPin` (
  `pinID` int(10) NOT NULL AUTO_INCREMENT,
  `charID` int(10) NOT NULL,
  `planetID` int(10) NOT NULL,
  `typeID` int(10) NOT NULL,
  `latitude` float NOT NULL,
  `longitude` float NOT NULL,
  `status` tinyint(2) NOT NULL,
  `level` tinyint(2) NOT NULL,
  `lastSimTime` bigint(20) NOT NULL,
  PRIMARY KEY (`pinID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=130000000 COMMENT='Table CommandCenter pin data with AI';


CREATE TABLE `chrPlanetLaunches` (
  `launchID` int(10) NOT NULL AUTO_INCREMENT,
  `charID` int(10) NOT NULL,
  `itemID` int(10) NOT NULL,
  `solarSystemID` int(10) NOT NULL,
  `planetID` int(10) NOT NULL,
  `status` varchar(17) CHARACTER SET utf8 NOT NULL,
  `launchTime` bigint(20) NOT NULL,
  `x` double NOT NULL,
  `y` double NOT NULL,
  `z` double NOT NULL,
  UNIQUE KEY `launchID` (`launchID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1;


CREATE TABLE `chrPlanetPins` (
  `pinID` int(10) NOT NULL DEFAULT '0',
  `typeID` int(10) NOT NULL,
  `ownerID` int(10) NOT NULL,
  `state` tinyint(1) NOT NULL,
  `latitude` double NOT NULL,
  `longitude` double NOT NULL,
  `isCommandCenter` tinyint(1) NOT NULL DEFAULT '0',
  `isLaunchable` tinyint(1) NOT NULL DEFAULT '0',
  `isProcess` tinyint(1) NOT NULL DEFAULT '0',
  `isExtractor` tinyint(1) NOT NULL DEFAULT '0',
  `heads` int(2) DEFAULT NULL,
  `schematicID` int(10) DEFAULT NULL,
  `hasRecievedInputs` tinyint(1) DEFAULT NULL,
  `recievedInputsLastCycle` tinyint(1) DEFAULT NULL,
  `cycleTime` int(10) DEFAULT NULL,
  `programType` int(10) DEFAULT NULL,
  `qtyPerCycle` int(10) DEFAULT NULL,
  `launchTime` bigint(20) DEFAULT NULL,
  `expiryTime` bigint(20) DEFAULT NULL,
  `installTime` bigint(20) DEFAULT NULL,
  `lastRunTime` bigint(20) DEFAULT NULL,
  `headRadius` float DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `chrPlanets` (
  `charID` int(10) NOT NULL,
  `planetID` int(10) NOT NULL,
  `solarSystemID` int(10) NOT NULL,
  `typeID` int(10) NOT NULL,
  `numberOfPins` int(10) NOT NULL,
  PRIMARY KEY (`charID`,`planetID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


CREATE TABLE `planetResourceInfo` (
  `planetID` int(10) NOT NULL,
  `itemID1` int(10) NOT NULL,
  `itemID2` int(10) NOT NULL,
  `itemID3` int(10) NOT NULL,
  `itemID4` int(10) NOT NULL,
  `itemID5` int(10) NOT NULL,
  `quality1` float NOT NULL,
  `quality2` float NOT NULL,
  `quality3` float NOT NULL,
  `quality4` float NOT NULL,
  `quality5` float NOT NULL,
  `data1` varchar(255) NOT NULL,
  `data2` varchar(255) NOT NULL,
  `data3` varchar(255) NOT NULL,
  `data4` varchar(255) NOT NULL,
  `data5` varchar(255) NOT NULL,
  `numBands1` int(10) NOT NULL,
  `numBands2` int(10) NOT NULL,
  `numBands3` int(10) NOT NULL,
  `numBands4` int(10) NOT NULL,
  `numBands5` int(10) NOT NULL,
  UNIQUE KEY `planetID` (`planetID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


INSERT INTO `planetResourceInfo` SELECT `itemID` AS `planetID`, 2073 AS `itemID1`, 2268 AS `itemID2`, 2287 AS `itemID3`, 2288 AS `itemID4`, 2305 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapDenormalize` WHERE `typeID` = 11 AND `groupID` = 7;
INSERT INTO `planetResourceInfo` SELECT `itemID` AS `planetID`, 2268 AS `itemID1`, 2272 AS `itemID2`, 2073 AS `itemID3`, 2286 AS `itemID4`, 2310 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapDenormalize` WHERE `typeID` = 12 AND `groupID` = 7;
INSERT INTO `planetResourceInfo` SELECT `itemID` AS `planetID`, 2268 AS `itemID1`, 2267 AS `itemID2`, 2309 AS `itemID3`, 2310 AS `itemID4`, 2311 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapDenormalize` WHERE `typeID` = 13 AND `groupID` = 7;
INSERT INTO `planetResourceInfo` SELECT `itemID` AS `planetID`, 2268 AS `itemID1`, 2287 AS `itemID2`, 2073 AS `itemID3`, 2286 AS `itemID4`, 2288 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapDenormalize` WHERE `typeID` = 2014 AND `groupID` = 7;
INSERT INTO `planetResourceInfo` SELECT `itemID` AS `planetID`, 2267 AS `itemID1`, 2272 AS `itemID2`, 2308 AS `itemID3`, 2307 AS `itemID4`, 2306 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapDenormalize` WHERE `typeID` = 2015 AND `groupID` = 7;
INSERT INTO `planetResourceInfo` SELECT `itemID` AS `planetID`, 2268 AS `itemID1`, 2267 AS `itemID2`, 2073 AS `itemID3`, 2270 AS `itemID4`, 2288 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapDenormalize` WHERE `typeID` = 2016 AND `groupID` = 7;
INSERT INTO `planetResourceInfo` SELECT `itemID` AS `planetID`, 2268 AS `itemID1`, 2267 AS `itemID2`, 2308 AS `itemID3`, 2309 AS `itemID4`, 2310 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapDenormalize` WHERE `typeID` = 2017 AND `groupID` = 7;
INSERT INTO `planetResourceInfo` SELECT `itemID` AS `planetID`, 2267 AS `itemID1`, 2272 AS `itemID2`, 2270 AS `itemID3`, 2308 AS `itemID4`, 2306 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapDenormalize` WHERE `typeID` = 2063 AND `groupID` = 7;
