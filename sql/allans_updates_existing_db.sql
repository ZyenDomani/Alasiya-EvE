
ALTER TABLE `srvStatus` ADD `npcs` INT NOT NULL ;

/* update character_ to add capsuleID */
ALTER TABLE `character_` ADD `capsuleID` INT(10) NOT NULL DEFAULT '0' AFTER `shipID`;
ALTER TABLE `character_` ADD `bloodlineID` TINYINT UNSIGNED NOT NULL DEFAULT '0' AFTER `ancestryID`;
ALTER TABLE `character_` ADD `raceID` TINYINT UNSIGNED NOT NULL DEFAULT '0' AFTER `bloodlineID`;
/* update to add unique interger `clientID` to account */
ALTER TABLE `account` ADD `clientID` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `accountID`;
/* update for client seed for making a unique clientID */
ALTER TABLE `srvStatus` ADD `ClientSeed` INT(10) NOT NULL;


CREATE TABLE `roidItems` (
  `itemID` int(10) unsigned NOT NULL,
  `itemName` varchar(25) NOT NULL,
  `typeID` int(11) NOT NULL,
  `systemID` int(11) NOT NULL,
  `beltID` int(11) NOT NULL,
  `quantity` double NOT NULL,
  `radius` double NOT NULL,
  `x` double NOT NULL,
  `y` double NOT NULL,
  `z` double NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Indexes for table `roidItems`
--
ALTER TABLE `roidItems`
  ADD PRIMARY KEY (`itemID`),
  ADD UNIQUE KEY `itemID` (`itemID`),
  ADD KEY `systemID` (`systemID`),
  ADD KEY `beltID` (`beltID`);
