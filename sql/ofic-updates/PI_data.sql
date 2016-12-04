
DROP TABLE IF EXISTS chrPlanets, chrPlanetCCPin, chrPlanetPins, chrPlanetLinks, chrPlanetRoutes, chrPlanetLaunches;

CREATE TABLE `chrPlanets` (
  `charID` int(10) NOT NULL DEFAULT '0',
  `solarSystemID` int(10) NOT NULL DEFAULT '0',
  `planetID` int(10) NOT NULL DEFAULT '0',
  `typeID` SMALLINT(6) NOT NULL DEFAULT '0',
  `numberOfPins` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`charID`,`planetID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `chrPlanetCCPin` (
  `pinID` int(10) NOT NULL DEFAULT '0',
  `charID` int(10) NOT NULL DEFAULT '0',
  `planetID` int(10) NOT NULL DEFAULT '0',
  `typeID` SMALLINT(6) NOT NULL DEFAULT '0',
  `latitude` double NOT NULL DEFAULT '0',
  `longitude` double NOT NULL DEFAULT '0',
  `state` tinyint(2) NOT NULL DEFAULT '1',
  `level` SMALLINT(3) NOT NULL DEFAULT '0',
  `lastSimTime` bigint(20) NOT NULL DEFAULT '0',
  UNIQUE KEY `pinID` (`pinID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='CommandCenter pin data';

CREATE TABLE `chrPlanetPins` (
  `ccPinID` int(10) NOT NULL DEFAULT '0',
  `pinID` int(10) NOT NULL DEFAULT '0',
  `typeID` SMALLINT(6) NOT NULL DEFAULT '0',
  `ownerID` int(10) NOT NULL DEFAULT '0',
  `state` tinyint(1) NOT NULL DEFAULT '1',
  `level` SMALLINT(3) NOT NULL DEFAULT '0',
  `latitude` double NOT NULL DEFAULT '0',
  `longitude` double NOT NULL DEFAULT '0',
  `isCommandCenter` tinyint(1) NOT NULL DEFAULT '0',
  `isLaunchable` tinyint(1) NOT NULL DEFAULT '0',
  `isProcess` tinyint(1) NOT NULL DEFAULT '0',
  `isStorage` tinyint(1) NOT NULL DEFAULT '0',
  `isECU` tinyint(1) NOT NULL DEFAULT '0',
  `schematicID` SMALLINT(3) NOT NULL DEFAULT '0',
  `programType` SMALLINT(3) NOT NULL DEFAULT '0',
  `hasReceivedInputs` tinyint(1) NOT NULL DEFAULT '0',
  `receivedInputsLastCycle` tinyint(1) NOT NULL DEFAULT '0',
  `cycleTime` bigint(20) NOT NULL DEFAULT '0',
  `launchTime` bigint(20) NOT NULL DEFAULT '0',
  `expiryTime` bigint(20) NOT NULL DEFAULT '0',
  `installTime` bigint(20) NOT NULL DEFAULT '0',
  `lastRunTime` bigint(20) NOT NULL DEFAULT '0',
  `headRadius` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`pinID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Colony pin data';

CREATE TABLE `chrPlanetLinks` (
  `ccPinID` int(10) NOT NULL DEFAULT '0',
  `linkID` INT(10) NOT NULL DEFAULT '0',
  `level` SMALLINT(3) NOT NULL DEFAULT '0',
  `typeID` SMALLINT(6) NOT NULL DEFAULT '0',
  `state` TINYINT(2) NOT NULL DEFAULT '1',
  `endpoint1` int(10) NOT NULL DEFAULT '0',
  `endpoint2` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`linkID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Colony link data';

CREATE TABLE `chrPlanetRoutes` (
  `ccPinID` int(10) NOT NULL DEFAULT '0',
  `routeID` SMALLINT(6) UNSIGNED NOT NULL AUTO_INCREMENT,
  `srcPinID` INT(10) NOT NULL DEFAULT '0',
  `destPinID` INT(10) NOT NULL DEFAULT '0',
  `state` TINYINT(2) NOT NULL DEFAULT '1',
  `priority` TINYINT(2) NOT NULL DEFAULT '0',
  `path` VARCHAR(200) CHARACTER SET utf8 NOT NULL DEFAULT '0',
  `itemID` int(10) NOT NULL DEFAULT '0',
  `itemQty` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY( `routeID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 COMMENT='Colony route data';

CREATE TABLE `chrPlanetECUHeads` (
  `ccPinID` int(10) NOT NULL DEFAULT '0',
  `ownerID` INT(10) NOT NULL DEFAULT '0',
  `ecuID` int(10) NOT NULL DEFAULT '0',
  `headID` SMALLINT(3) NOT NULL DEFAULT '0',
  `typeID` SMALLINT(6) NOT NULL DEFAULT '0',
  `latitude` double NOT NULL DEFAULT '0',
  `longitude` double NOT NULL DEFAULT '0',
  PRIMARY KEY ( `ccPinID`, `ownerID`, `ecuID`, `headID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Colony ECU head data';

CREATE TABLE `chrPlanetPinContents` (
  `ccPinID` int(10) NOT NULL DEFAULT '0',
  `pinID` int(10) NOT NULL DEFAULT '0',
  `itemID` int(10) NOT NULL DEFAULT '0',
  `itemQty` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`pinID`,`itemID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Colony pin contents data';

CREATE TABLE `chrPlanetLaunches` (
  `launchID` int(10) NOT NULL AUTO_INCREMENT,
  `containerID` INT(10) NOT NULL DEFAULT '0',
  `charID` int(10) NOT NULL DEFAULT '0',
  `solarSystemID` int(10) NOT NULL DEFAULT '0',
  `planetID` INT(10) NOT NULL DEFAULT '0',
  `launchTime` bigint(20) NOT NULL DEFAULT '0',
  `x` double NOT NULL DEFAULT '0',
  `y` double NOT NULL DEFAULT '0',
  `z` double NOT NULL DEFAULT '0',
  UNIQUE KEY `launchID` (`launchID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1;
