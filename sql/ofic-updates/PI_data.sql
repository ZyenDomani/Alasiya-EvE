


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
