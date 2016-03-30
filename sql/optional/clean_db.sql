/* clean_db.sql
 *
 * this will erase all player-created data in db, and *SHOULD* result in a clean install.
 *  NOTE:  this WILL NOT delete accounts.  un-comment first line to delete accounts also.
 */

-- DELETE FROM `account` WHERE 1;
DELETE FROM `cacheOwners` WHERE 1;
DELETE FROM `character_` WHERE 1;
DELETE FROM `chrMissionState` WHERE 1;
DELETE FROM `chrNotes` WHERE 1;
DELETE FROM `chrOffers` WHERE 1;
DELETE FROM `chrOwnerNote` WHERE 1;
DELETE FROM `chrPausedSkillQueue` WHERE 1;
DELETE FROM `chrPlanetLaunches` WHERE 1;
DELETE FROM `chrPlanets` WHERE 1;
DELETE FROM `chrSkillHistory` WHERE 1;
DELETE FROM `chrSkillQueue` WHERE 1;
DELETE FROM `chrVisitedSystems` WHERE 1;
DELETE FROM `corporation` WHERE `corporationID` > 1000999;
ALTER TABLE `corporation` auto_increment = 1001000;
DELETE FROM `crpAlliance` WHERE 1;
DELETE FROM `crpCharShares` WHERE 1;
DELETE FROM `crpOffices` WHERE 1;
DELETE FROM `droneState` WHERE 1;
DELETE FROM `dungeonsspawned` WHERE 1;
DELETE FROM `entity` WHERE `itemID` > 100000000;
ALTER TABLE `entity` auto_increment = 140000000;
DELETE FROM `entity_attributes` WHERE `itemID` > 100000000;
DELETE FROM `entity_default_attributes` WHERE `itemID` > 100000000;
DELETE FROM `eveMail` WHERE 1;
DELETE FROM `eveMailDetails` WHERE 1;
DELETE FROM `invBlueprints` WHERE 1;
DELETE FROM `mailLabel` WHERE 1;
DELETE FROM `mailMessage` WHERE 1;
DELETE FROM `mapDynamicData` WHERE 1;
DELETE FROM `market_history_old` WHERE 1;
DELETE FROM `market_journal` WHERE 1;
DELETE FROM `market_orders` WHERE 1;
DELETE FROM `market_transactions` WHERE 1;
DELETE FROM `ramJobs` WHERE 1;
DELETE FROM `rentalInfo` WHERE 1;
DELETE FROM `repAgent` WHERE 1;
DELETE FROM `repAlliance` WHERE 1;
DELETE FROM `repChar` WHERE 1;
DELETE FROM `repCorp` WHERE 1;
DELETE FROM `repNPCCorp` WHERE 1;
DELETE FROM `repStandingChanges` WHERE 1;
DELETE FROM `serverStatistic` WHERE 1;
DELETE FROM `shipInsurance` WHERE 1;
