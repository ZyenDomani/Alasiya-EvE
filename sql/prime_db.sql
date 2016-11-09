/*
 * This file is here to deal with some tables needing to contain both
 * static and dynamic information. The solution for ease of release is
 * to keep all the static info in a seperate copy of the tables such
 * that it can be loaded into the non-static table.
 */

/*
 * Truncate all the dynamic tables
 */
TRUNCATE TABLE billsPayable;
TRUNCATE TABLE billsReceivable;
TRUNCATE TABLE bookmarks;
TRUNCATE TABLE cacheLocations;
TRUNCATE TABLE cacheOwners;
TRUNCATE TABLE channelChars;
TRUNCATE TABLE channelMods;
TRUNCATE TABLE character_;
TRUNCATE TABLE chrApplications;
TRUNCATE TABLE chrEmployment;
TRUNCATE TABLE chrMissionState;
TRUNCATE TABLE chrNotes;
TRUNCATE TABLE chrOffers;
TRUNCATE TABLE chrOwnerNote;
TRUNCATE TABLE chrSkillQueue;
TRUNCATE TABLE crpAlliance;
TRUNCATE TABLE crpCharShares;
TRUNCATE TABLE crpOffices;
TRUNCATE TABLE droneState;
TRUNCATE TABLE entity;
TRUNCATE TABLE entity_attributes;
TRUNCATE TABLE eveMail;
TRUNCATE TABLE eveMailDetails;
TRUNCATE TABLE invBlueprints;
TRUNCATE TABLE market_history_old;
TRUNCATE TABLE market_journal;
TRUNCATE TABLE market_orders;
TRUNCATE TABLE market_transactions;
TRUNCATE TABLE ramAssemblyLineStationCostLogs;
TRUNCATE TABLE ramJobs;
TRUNCATE TABLE rentalInfo;

ALTER TABLE `corporation` CHANGE `corporationID` `corporationID` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT;
ALTER TABLE corporation  AUTO_INCREMENT=1001000;
/*
 * Copy over the static entities:
 * Static record of EVE System
 */
INSERT INTO entity (itemID, itemName, singleton, quantity)
 VALUES (1, 'EVE System', 1, 1);

ALTER TABLE entity AUTO_INCREMENT=140000000;
/*
 * Copy over the static owner info.
 * This is a bit hacky: we rebuild this table although it's static but it
 * allows us not to include its data in dump.
 */
TRUNCATE TABLE eveStaticOwners;
/*
 * Static record of EVE System
 */
INSERT INTO eveStaticOwners (ownerID, ownerName, typeID)
 VALUES (1, 'EVE System', 0);
/*
 * Insert agents
 */
INSERT INTO eveStaticOwners (ownerID, ownerName, typeID)
 SELECT characterID, characterName, typeID
 FROM chrNPCCharacters;
/*
 * Insert factions
 */
INSERT INTO eveStaticOwners (ownerID, ownerName, typeID)
 SELECT factionID, factionName, 30 AS typeID
 FROM chrFactions;
/*
 * Insert corporations
 */
INSERT INTO eveStaticOwners (ownerID, ownerName, typeID)
 SELECT corporationID, corporationName, 2 AS typeID
 FROM corporation;


/* non-static pos, ??  */
-- INSERT INTO cacheLocations(locationID, locationName, x, y, z, locationNameID)

/* this needs more work once factions and alliances are implemented */
/* non-static corp, faction, and alliance */   --not sure if this is used
INSERT INTO cacheOwners (ownerID, ownerName, typeID, ownerNameID)
 SELECT corporationID, corporationName, 2, 0
 FROM corporation;


/* insert null location into mapDenormalize */
INSERT INTO `mapDenormalize` (`itemID`, `typeID`, `groupID`, `solarSystemID`, `constellationID`, `regionID`, `orbitID`, `x`, `y`, `z`, `radius`, `itemName`, `security`, `celestialIndex`, `orbitIndex`)
VALUES ('0', NULL, NULL, NULL, NULL, NULL, NULL, '0', '0', '0', NULL, NULL, NULL, NULL, NULL);