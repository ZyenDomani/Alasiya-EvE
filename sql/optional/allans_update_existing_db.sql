
DROP TABLE `bookmarks`;
DROP TABLE `bookmarkFolders`;


/*Table structure for table `bookmarks` */

CREATE TABLE `bookmarks` (
  `bookmarkID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `ownerID` int(10) unsigned NOT NULL DEFAULT '0',
  `itemID` int(10) unsigned NOT NULL DEFAULT '0',
  `typeID` int(10) unsigned NOT NULL DEFAULT '0',
  `flag` int(10) unsigned NOT NULL DEFAULT '0',
  `memo` varchar(85) NOT NULL DEFAULT '',
  `created` bigint(20) unsigned NOT NULL DEFAULT '0',
  `x` double NOT NULL DEFAULT '0',
  `y` double NOT NULL DEFAULT '0',
  `z` double NOT NULL DEFAULT '0',
  `locationID` int(10) unsigned NOT NULL DEFAULT '0',
  `note` varchar(85) NOT NULL DEFAULT '',
  `creatorID` int(10) unsigned NOT NULL DEFAULT '0',
  `folderID` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`bookmarkID`)
) ENGINE=InnoDB AUTO_INCREMENT=600000000 DEFAULT CHARSET=utf8;

CREATE TABLE `bookmarkFolders` (
  `folderID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `folderName` varchar(255) NOT NULL DEFAULT '',
  `ownerID` int(10) NOT NULL DEFAULT '0',
  `creatorID` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`folderID`),
  KEY `ownerID` (`ownerID`)
) ENGINE=InnoDB AUTO_INCREMENT=100000 DEFAULT CHARSET=utf8;
