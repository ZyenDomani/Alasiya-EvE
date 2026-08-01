#!/bin/bash
MYSQL_USER=allan
MYSQL_PASS=none
DB_NAME=EvE_AlasiyaDev

unset blacklist

_() { blacklist="${blacklist}${blacklist+}${*}"; }

# Add any tables that should be dumped with no data.
_ "agtOffers alnAlliance alnContacts alnLabels avatars avatar_colors"
_ "avatar_modifiers avatar_sculpts billsPayable billsReceivable bookmarks"
_ "bookmarkFolders cacheOwners channelChars channelMods channels"
_ "chrCertificates chrCharacters chrCharacterAttributes chrContacts chrEmployment"
_ "chrKillTable chrLabels chrNotes chrOwnerNote chrPausedSkillQueue"
_ "chrShipFittings chrSkillHistory chrSkillQueue chrVisitedSystems crpAdRegistry"
_ "crpApplications crpBulletins crpContacts crpLabels crpLockedItems"
_ "crpItemEvent crpRecruiters crpRoleHistroy crpRoleTitles crpShares"
_ "crpVoteItems crpWalletDivisons CruciblePriceHistory CruciblePriceHistory_Materials droneState dunActive entity"
_ "entity_attributes eveMailLabels eveMailMailingListAccess eveMailMailingListMembers eveMailMailingLists"
_ "eveMailMessages eveMailRecipients eveNotificationRecipients eveNotificationsinvBlueprints jnlCharacters"
_ "jnlCorporations mapDynamicData mktHistory mktOrders mktTransactions mktData mktHistory mktOrders mktTransactions"
_ "mktUpdates piCCPin piECUHeads piLaunches piLinks piPinContents piPinMap piPins piPlanets piRoutes piSchematics"
_ "piTypeMap posCustomsOfficeData posJumpBridgeData posStructureData posTowerData ramJobs rentalInfo repStandingChanges"
_ "shipInsurance srvStatisticData srvStatisticHistory staOffices sysAsteroids"
_ "sysCalendarEvents sysCalendarInvitees sysCalendarResponses sysSignatures tmp_chruker_index tmp_client_strings"
_ "tmp_eveinfo_index tmp_evesurvival_index tmp_eveuni_index sysSignatures webBounties"

SQL_STRING="SHOW TABLES;"
# Pipe the SQL into mysql
TABLES=$(echo $SQL_STRING | mysql -u$MYSQL_USER -p$MYSQL_PASS $DB_NAME -Bs)

#mkdir tables

#echo $TABLES

for i in ${TABLES} ; do
	if [[ ${blacklist} == *${i}* ]]; then
		echo "Dumping $i without data"
		mysqldump --add-drop-table -d -u $MYSQL_USER -p$MYSQL_PASS $DB_NAME $i > "tables/${i}.sql"
	else
		echo "Dumping $i"
		mysqldump --add-drop-table -u $MYSQL_USER -p$MYSQL_PASS $DB_NAME $i > "tables/${i}.sql"
	fi
done
