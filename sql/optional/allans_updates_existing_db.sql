
--
-- Table structure for table `chrKillTable`
--

CREATE TABLE `chrKillTable` (
  `killID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `solarSystemID` int(10) unsigned NOT NULL DEFAULT '0',
  `victimCharacterID` int(10) unsigned NOT NULL DEFAULT '0',
  `victimCorporationID` int(10) unsigned NOT NULL DEFAULT '0',
  `victimAllianceID` int(10) unsigned NOT NULL DEFAULT '0',
  `victimFactionID` int(10) unsigned NOT NULL DEFAULT '0',
  `victimShipTypeID` smallint(4) unsigned NOT NULL DEFAULT '0',
  `finalCharacterID` int(10) unsigned NOT NULL DEFAULT '0',
  `finalCorporationID` int(10) unsigned NOT NULL DEFAULT '0',
  `finalAllianceID` int(10) unsigned NOT NULL DEFAULT '0',
  `finalFactionID` int(10) unsigned NOT NULL DEFAULT '0',
  `finalShipTypeID` smallint(4) unsigned NOT NULL DEFAULT '0',
  `finalWeaponTypeID` smallint(4) unsigned NOT NULL DEFAULT '0',
  `killBlob` blob NOT NULL,
  `killTime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `victimDamageTaken` int(10) unsigned NOT NULL DEFAULT '0',
  `finalSecurityStatus` double NOT NULL DEFAULT '0',
  `moonID` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`killID`),
  KEY `victimCharacterID` (`victimCharacterID`),
  KEY `finalCharacterID` (`finalCharacterID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

ALTER TABLE `srvStatus` ADD `npcs` INT NOT NULL ;

/* update character_ to add capsuleID */
ALTER TABLE `character_` ADD `capsuleID` INT(10) NOT NULL DEFAULT '0' AFTER `shipID`;
ALTER TABLE `character_` ADD `bloodlineID` TINYINT UNSIGNED NOT NULL DEFAULT '0' AFTER `ancestryID`;
ALTER TABLE `character_` ADD `raceID` TINYINT UNSIGNED NOT NULL DEFAULT '0' AFTER `bloodlineID`;
/* update to add unique interger `clientID` to account */
ALTER TABLE `account` ADD `clientID` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `accountID`;
/* update for client seed for making a unique clientID */
ALTER TABLE `srvStatus` ADD `ClientSeed` INT(10) NOT NULL;
/*  change skill time constanant to int from float */
UPDATE `entity_attributes` SET `valueInt`=`valueFloat`, `valueFloat`=NULL WHERE `attributeID`=275 AND `valueFloat` IS NOT NULL;

