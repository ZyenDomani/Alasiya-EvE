
ALTER TABLE `srvStatus` ADD `npcs` INT NOT NULL ;

/* update character_ to add capsuleID */
ALTER TABLE `character_` ADD `capsuleID` INT(10) NOT NULL DEFAULT '0' AFTER `shipID`;
/* update to add unique interger `clientID` to account */
ALTER TABLE `account` ADD `clientID` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `accountID`;
/* update for client seed for making a unique clientID */
ALTER TABLE `srvStatus` ADD `ClientSeed` INT(10) NOT NULL;