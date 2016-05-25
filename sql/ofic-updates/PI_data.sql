

CREATE TABLE `chrPlanetColonies` (
  `charID` int(11) NOT NULL,
  `planetID` int(11) NOT NULL,
  `status` varchar(17) CHARACTER SET utf8 NOT NULL,
  `launchTime` bigint(20) NOT NULL,
  `x` double NOT NULL,
  `y` double NOT NULL,
  `z` double NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1;

CREATE TABLE `chrPlanetLaunches` (
  `launchID` int(11) NOT NULL AUTO_INCREMENT,
  `charID` int(11) NOT NULL,
  `itemID` int(11) NOT NULL,
  `solarSystemID` int(11) NOT NULL,
  `planetID` int(11) NOT NULL,
  `status` varchar(17) CHARACTER SET utf8 NOT NULL,
  `launchTime` bigint(20) NOT NULL,
  `x` double NOT NULL,
  `y` double NOT NULL,
  `z` double NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1;

ALTER TABLE `chrPlanetLaunches`
  ADD UNIQUE KEY `launchID` (`launchID`);

CREATE TABLE `chrPlanetPins` (
  `id` int(11) NOT NULL DEFAULT '0',
  `typeID` int(11) NOT NULL,
  `ownerID` int(11) NOT NULL,
  `state` tinyint(2) NOT NULL,
  `latitude` double NOT NULL,
  `longitude` double NOT NULL,
  `isCommandCenter` tinyint(2) NOT NULL DEFAULT '0',
  `isLaunchable` tinyint(2) NOT NULL DEFAULT '0',
  `isProcess` tinyint(2) NOT NULL DEFAULT '0',
  `isExtractor` tinyint(2) NOT NULL DEFAULT '0',
  `heads` int(4) DEFAULT NULL,
  `schematicID` int(11) DEFAULT NULL,
  `hasRecievedInputs` tinyint(1) DEFAULT NULL,
  `recievedInputsLastCycle` tinyint(1) DEFAULT NULL,
  `cycleTime` int(11) DEFAULT NULL,
  `programType` int(11) DEFAULT NULL,
  `qtyPerCycle` int(11) DEFAULT NULL,
  `launchTime` bigint(20) DEFAULT NULL,
  `expiryTime` bigint(20) DEFAULT NULL,
  `installTime` bigint(20) DEFAULT NULL,
  `lastRunTime` bigint(20) DEFAULT NULL,
  `headRadius` float DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `chrPlanets` (
  `characterID` int(11) NOT NULL,
  `planetID` int(11) NOT NULL,
  `solarSystemID` int(11) NOT NULL,
  `typeID` int(11) NOT NULL,
  `numberOfPins` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

ALTER TABLE `chrPlanets`
  ADD PRIMARY KEY (`characterID`,`planetID`);


CREATE TABLE `planetResourceInfo` (
  `planetID` int(11) NOT NULL,
  `itemID1` int(11) NOT NULL,
  `itemID2` int(11) NOT NULL,
  `itemID3` int(11) NOT NULL,
  `itemID4` int(11) NOT NULL,
  `itemID5` int(11) NOT NULL,
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
  `numBands1` int(11) NOT NULL,
  `numBands2` int(11) NOT NULL,
  `numBands3` int(11) NOT NULL,
  `numBands4` int(11) NOT NULL,
  `numBands5` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

ALTER TABLE `planetResourceInfo`
  ADD UNIQUE KEY `planetID` (`planetID`);

INSERT INTO `planetResourceInfo` SELECT `itemID` AS `planetID`, 2073 AS `itemID1`, 2268 AS `itemID2`, 2287 AS `itemID3`, 2288 AS `itemID4`, 2305 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapDenormalize` WHERE `typeID` = 11 AND `groupID` = 7;
INSERT INTO `planetResourceInfo` SELECT `itemID` AS `planetID`, 2268 AS `itemID1`, 2272 AS `itemID2`, 2073 AS `itemID3`, 2286 AS `itemID4`, 2310 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapDenormalize` WHERE `typeID` = 12 AND `groupID` = 7;
INSERT INTO `planetResourceInfo` SELECT `itemID` AS `planetID`, 2268 AS `itemID1`, 2267 AS `itemID2`, 2309 AS `itemID3`, 2310 AS `itemID4`, 2311 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapDenormalize` WHERE `typeID` = 13 AND `groupID` = 7;
INSERT INTO `planetResourceInfo` SELECT `itemID` AS `planetID`, 2268 AS `itemID1`, 2287 AS `itemID2`, 2073 AS `itemID3`, 2286 AS `itemID4`, 2288 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapDenormalize` WHERE `typeID` = 2014 AND `groupID` = 7;
INSERT INTO `planetResourceInfo` SELECT `itemID` AS `planetID`, 2267 AS `itemID1`, 2272 AS `itemID2`, 2308 AS `itemID3`, 2307 AS `itemID4`, 2306 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapDenormalize` WHERE `typeID` = 2015 AND `groupID` = 7;
INSERT INTO `planetResourceInfo` SELECT `itemID` AS `planetID`, 2268 AS `itemID1`, 2267 AS `itemID2`, 2073 AS `itemID3`, 2270 AS `itemID4`, 2288 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapDenormalize` WHERE `typeID` = 2016 AND `groupID` = 7;
INSERT INTO `planetResourceInfo` SELECT `itemID` AS `planetID`, 2268 AS `itemID1`, 2267 AS `itemID2`, 2308 AS `itemID3`, 2309 AS `itemID4`, 2310 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapDenormalize` WHERE `typeID` = 2017 AND `groupID` = 7;
INSERT INTO `planetResourceInfo` SELECT `itemID` AS `planetID`, 2267 AS `itemID1`, 2272 AS `itemID2`, 2270 AS `itemID3`, 2308 AS `itemID4`, 2306 AS `itemID5`, 154.275 AS `quality1`, 154.275 AS `quality2`, 154.275 AS `quality3`, 154.275 AS `quality4`, 154.275 AS `quality5`, '~' AS `data1`, '~' AS `data2`, '~' AS `data3`, '~' AS `data4`, '~' AS `data5`, 1 AS `numBands1`, 1 AS `numBands2`, 1 AS `numBands3`, 1 AS `numBands4`, 1 AS `numBands5` FROM `mapDenormalize` WHERE `typeID` = 2063 AND `groupID` = 7;
