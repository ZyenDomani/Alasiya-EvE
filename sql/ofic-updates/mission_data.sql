
/*Table structure for table `qstCourierMissions` */

CREATE TABLE `qstCourierMissions` (
  `missionID` int(10) unsigned NOT NULL auto_increment,
  `kind` tinyint(3) unsigned NOT NULL default '0',
  `state` tinyint(3) unsigned NOT NULL default '0',
  `availabilityID` int(10) unsigned default NULL,
  `inOrder` tinyint(3) unsigned NOT NULL default '0',
  `description` text NOT NULL,
  `issuerID` int(10) unsigned default NULL,
  `assigneeID` int(10) unsigned default NULL,
  `acceptFee` double NOT NULL default '0',
  `acceptorID` int(10) unsigned default NULL,
  `dateIssued` int(10) unsigned NOT NULL default '0',
  `dateExpires` int(10) unsigned NOT NULL default '0',
  `dateAccepted` int(10) unsigned NOT NULL default '0',
  `dateCompleted` int(10) unsigned NOT NULL default '0',
  `totalReward` double NOT NULL default '0',
  `tracking` tinyint(3) unsigned NOT NULL default '0',
  `pickupStationID` int(10) unsigned default NULL,
  `craterID` int(10) unsigned default NULL,
  `dropStationID` int(10) unsigned default NULL,
  `volume` double NOT NULL default '0',
  `pickupSolarSystemID` int(10) unsigned default NULL,
  `pickupRegionID` int(10) unsigned default NULL,
  `dropSolarSystemID` int(10) unsigned default NULL,
  `dropRegionID` int(10) unsigned default NULL,
  PRIMARY KEY  (`missionID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `qstCourierMissions` */

