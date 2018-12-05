/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Zhur
    Updates:        Allan
*/

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "character/Character.h"
#include "character/CharacterDB.h"

CharacterDB::CharacterDB()
{
    load_name_validation_set();
}

uint32 CharacterDB::NewCharacter(const CharacterData& data, const CorpData& corpData) {
    DBerror err;
    std::string nameEsc, titleEsc, descriptionEsc;
    sDatabase.DoEscapeString(nameEsc, data.name);
    sDatabase.DoEscapeString(titleEsc, data.title);
    sDatabase.DoEscapeString(descriptionEsc, data.description);

    uint32 uid = 0;
    if (!sDatabase.RunQueryLID(err, uid,
        "INSERT INTO chrCharacters"
        "  (accountID, name, typeID, locationID, description, balance, aurBalance,"
        "   logonDateTime, corporationID, baseID, corpAccountKey, startDateTime, createDateTime, "
        "   ancestryID, bloodlineID, raceID, careerID, schoolID, careerSpecialityID, gender,"
        "   stationID, solarSystemID, constellationID, regionID, freeRespecs)"
        " VALUES"
        "  (%u,'%s', %u, %u, '%s', %f, %f,"
        "   %f, %u, %u, %u, %f, %f,"
        "   %u, %u, %u, %u, %u, %u, %u,"
        "   %u, %u, %u, %u, 2)",
        data.accountID, nameEsc.c_str(), data.typeID, data.locationID, descriptionEsc.c_str(), data.balance, data.aurBalance,
        GetFileTimeNow(), corpData.corporationID, corpData.baseID, corpData.corpAccountKey, GetFileTimeNow(), GetFileTimeNow(),
        data.ancestryID, data.bloodlineID, data.raceID, data.careerID, data.schoolID, data.careerSpecialityID, data.gender,
        data.stationID, data.solarSystemID, data.constellationID, data.regionID))
    {
        codelog(DATABASE__ERROR, "Failed to insert character %s.", err.c_str());
        return 0;
    }

    // And one more member to the corporation
    if (!sDatabase.RunQuery(err,
        "UPDATE crpCorporation"
        "  SET memberCount = memberCount + 1"
        " WHERE corporationID = %u", corpData.corporationID))
    {
        _log(DATABASE__MESSAGE, "Failed to raise member count of corporation %u: %s.", uid, err.c_str());
    }

    return uid;
}

bool CharacterDB::SaveCharacter(uint32 characterID, const CharacterData &data) {
    DBerror err;

    std::string titleEsc, descriptionEsc;
    sDatabase.DoEscapeString(titleEsc, data.title);
    sDatabase.DoEscapeString(descriptionEsc, data.description);

    if (!sDatabase.RunQuery(err,
        "UPDATE chrCharacters"
        " SET"
        "  title = '%s',"
        "  description = '%s',"
        "  bounty = %f,"
        "  balance = %f,"
        "  aurBalance = %f,"
        "  securityRating = %f,"
        "  logonMinutes = %u,"
        "  skillPoints = %f,"
        "  locationID = %u,"
        "  stationID = %u,"
        "  solarSystemID = %u,"
        "  constellationID = %u,"
        "  regionID = %u"
        " WHERE characterID = %u",
            titleEsc.c_str(), descriptionEsc.c_str(), data.bounty, data.balance, data.aurBalance, data.securityRating, data.logonMinutes,
            data.skillPoints, data.locationID, data.stationID, data.solarSystemID, data.constellationID, data.regionID, characterID))
    {
        codelog(DATABASE__ERROR, "Failed to save character %u: %s.", characterID, err.c_str());
        return false;
    }

    return true;
}

bool CharacterDB::SaveCorpData(uint32 characterID, const CorpData &data) {
    DBerror err;

    if (!sDatabase.RunQuery(err,
        "UPDATE chrCharacters"
        " SET"
        "  corporationID = %u, "
        "  baseID = %u,"
        "  corpRole = %lli,"
        "  corpAccountKey = %i,"
        "  rolesAtAll = %lli,"
        "  rolesAtBase = %lli,"
        "  rolesAtHQ = %lli,"
        "  rolesAtOther = %lli,"
        "  grantableRoles = %lli,"
        "  grantableRolesAtBase = %lli,"
        "  grantableRolesAtHQ = %lli,"
        "  grantableRolesAtOther = %" PRIi64
        " WHERE characterID = %u",
        data.corporationID, data.baseID, data.corpRole, data.corpAccountKey, data.rolesAtAll, data.rolesAtBase, data.rolesAtHQ, data.rolesAtOther,
        data.grantableRoles, data.grantableRolesAtBase, data.grantableRolesAtHQ, data.grantableRolesAtOther, characterID))
    {
        codelog(DATABASE__ERROR, "Failed to update corp member info of character %u: %s.", characterID, err.c_str());
        return false;
    }

    return true;
}

void CharacterDB::DeleteCharacter(uint32 characterID) {

    //  just a small list of possible locations to delete char references from....
    /**
     *        6 matches in avatar_colors
     *        15 matches in avatar_modifiers
     *        13 matches in avatar_sculpts
     *        1 match in avatars
     *        0 matches in bookmarkFolders
     *        0 matches in bookmarkVouchers
     *        1 match in bookmarks
     *        0 matches in bounties
     *        1 match in cacheOwners
     *        4 matches in channelChars
     *        0 matches in crpApplications
     *        1 match in chrCertificates
     *        1 match in chrEmployment
     *        31 matches in chrSkillHistory
     *        0 matches in chrSkillQueue
     *        0 matches in chrOwnerNote
     *        0 matches in chrPausedSkillQueue
     *        1 match in entity
     *        61 matches in entity_attributes
     *        0 matches in ramJobs
     */

    DBerror err;

    sDatabase.RunQuery(err, "DELETE FROM eveMailDetails"
                            " USING eveMail, eveMailDetails"
                            " WHERE eveMail.messageID = eveMailDetails.messageID"
                            " AND (senderID = %u OR channelID = %u)", characterID, characterID);

    sDatabase.RunQuery(err, "DELETE FROM eveMail WHERE (senderID = %u OR channelID = %u)", characterID, characterID);

    sDatabase.RunQuery(err, "DELETE FROM bookmarks WHERE ownerID = %u",  characterID);

    sDatabase.RunQuery(err, "DELETE FROM mktOrders WHERE ownerID = %u", characterID);

    sDatabase.RunQuery(err, "DELETE FROM mktTransactions WHERE clientID = %u", characterID);

    sDatabase.RunQuery(err, "DELETE FROM repStandings, repStandingChanges"
                            " WHERE (fromID = %u OR toID = %u)", characterID, characterID);

    sDatabase.RunQuery(err, "DELETE FROM chrCertificates, chrCharacters, chrEmployment, chrJournal, crpCharShares"
                            " WHERE characterID=%u", characterID);

    sDatabase.RunQuery(err, "DELETE FROM chrCharactersAttributes WHERE charID = %u", characterID);

    sDatabase.RunQuery(err, "DELETE FROM entity_attributes"
                            " WHERE itemID IN (SELECT itemID FROM entity WHERE ownerID = %u)", characterID);

    sDatabase.RunQuery(err, "DELETE FROM entity WHERE ownerID = %u", characterID);
}

bool CharacterDB::ReportRespec(uint32 characterId)
{
    DBerror error;
    if (!sDatabase.RunQuery(error, "UPDATE chrCharacters SET freeRespecs = freeRespecs - 1, lastRespecDateTime = %f, nextRespecDateTime = %lli WHERE characterId = %u",
        GetFileTimeNow(), Win32TimeNow() + Win32Time_Month *3, characterId))
        return false;
    return true;
}

PyRep* CharacterDB::GetRespecInfo(uint32 characterId)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT freeRespecs, lastRespecDateTime, nextRespecDateTime FROM chrCharacters WHERE characterID = %u", characterId))
        return nullptr;
    DBResultRow row;
    if (!res.GetRow(row))
        return nullptr;

    PyDict* result = new PyDict();
    result->SetItemString( "freeRespecs", new PyInt( row.GetInt(0) ) );
    result->SetItemString( "lastRespecDate", new PyInt( row.GetInt64(1) ) );
    result->SetItemString( "nextTimedRespec", new PyLong( row.GetInt64(2) ) );

    return result;
}

int64 CharacterDB::PrepareCharacterForDelete(uint32 accountID, uint32 charID)
{
    // calculate the point in time from which this character may be deleted
    int64 deleteTime = GetFileTimeNow() + (Win32Time_Second * sConfig.character.terminationDelay);

    // note: the queries relating to character deletion have been specifically designed to avoid wreaking havoc when used by a malicious client
    // the client can't lie to us about accountID, only charID

    DBerror error;
    uint32 affectedRows;
    sDatabase.RunQuery(error, affectedRows, "UPDATE chrCharacters SET deletePrepareDateTime = %lli WHERE accountID = %u AND characterID = %u", deleteTime, accountID, charID);
    if (affectedRows != 1)
        return 0;

    return deleteTime;
}

void CharacterDB::CancelCharacterDeletePrepare(uint32 accountID, uint32 charID)
{
    DBerror error;
    uint32 affectedRows;
    sDatabase.RunQuery(error, affectedRows, "UPDATE chrCharacters SET deletePrepareDateTime = 0 WHERE accountID = %u AND characterID = %u", accountID, charID);
    if (affectedRows != 1)
        codelog(CLIENT__ERROR, "Failed to cancel character deletion, affected rows: %u", affectedRows);
}

PyRep *CharacterDB::GetCharacterList(uint32 accountID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  characterID,"
        "  name AS characterName,"
        "  deletePrepareDateTime,"
        "  gender,"
        "  typeID"
        " FROM chrCharacters"
        " WHERE accountID=%u", accountID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

PyRep* CharacterDB::ValidateCharName(const char *name)
{
    /** @todo
            validStates = {-1: localization.GetByLabel('UI/CharacterCreation/InvalidName/TooShort'),
             -2: localization.GetByLabel('UI/CharacterCreation/InvalidName/TooLong'),
             -5: localization.GetByLabel('UI/CharacterCreation/InvalidName/IllegalCharacter'),
             -6: localization.GetByLabel('UI/CharacterCreation/InvalidName/TooManySpaces'),
             -7: localization.GetByLabel('UI/CharacterCreation/InvalidName/ConsecutiveSpaces'),
             -101: localization.GetByLabel('UI/CharacterCreation/InvalidName/Unavailable'),
             -102: localization.GetByLabel('UI/CharacterCreation/InvalidName/Unavailable')}
             */
    // *name  is sent from client WITHOUT leading space, if there is one, and will not allow more than one space.

    if (name == nullptr or (*name == '\0'))
        return new PyInt(-1);

    // verify that NO ONE tries to use "CCP *name*"
    if ((name[0] == 'C') or (name[0] == 'c'))
        if ((name[1] == 'C') or (name[1] == 'c'))
            if ((name[2] == 'P') or (name[2] == 'p'))
                return new PyInt(-5);

    // check for length?   30 max?   funky hack...not used....client caps at 24  (but this works)
    int8 length = 0;
    while (name[length] != '\0')
        length++;
    //_log(CLIENT__ERROR, "length is %i", length);

    if (length == 0)
        return new PyInt(-1);
    else if (length > 30)
        return new PyInt(-2);

    /* hash the name */
    uint32 hash = djb2_hash(name);

    /* check if its in our std::set */
    CharValidationSetItr itr = mNameValidation.find(hash);

    /* if itr is not equal to the end of the set it means that the same hash has been found */
    if (itr != mNameValidation.end())
        return new PyInt(-101);

    /* if we got here the name is "new" */
    return new PyInt(1);
}

void CharacterDB::AddEmployment(uint32 charID, uint32 corpID) {
    // Add new employment history record and update character's corp start date   -allan  25Mar14
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "INSERT INTO chrEmployment"
        "  (characterID, corporationID, startDate, deleted)"
        " VALUES (%u, %u, %f, 0)", charID, corpID, GetFileTimeNow()))
    {
        codelog(DATABASE__ERROR, "Error in employment insert query: %s", err.c_str());
    }

    if (!sDatabase.RunQuery(err, "UPDATE chrCharacters SET startDateTime = %f WHERE characterID = %u", GetFileTimeNow(), charID))
        codelog(DATABASE__ERROR, "Error in employment insert query: %s", err.c_str());
}

PyRep *CharacterDB::GetCharSelectInfo(uint32 characterID) {
    //  this shows char on select screen....fixed/updated  -allan 20Jan15
    std::string shipName = "My Ship";
    uint32 shipTypeID = 606;  //arbitrary default.

    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT itemName, typeID FROM entity WHERE itemID = (SELECT shipID FROM chrCharacters WHERE characterID = %u)", characterID)) {
        _log(CHARACTER__WARNING, "Unable to get current ship: %s", res.error.c_str());
    } else {
        DBResultRow row;
        /** @todo  need to make proper error here. */
        // this causes blanks on char sel screen if there is no ship, or shipID is wrong.
        if (!res.GetRow(row))
            return new PyNone();

        sDatabase.DoEscapeString(shipName, row.GetText(0));
        shipTypeID = row.GetUInt(1);
    }

    uint32 unreadMailCount = 0;
    uint32 upcomingEventCount = 0;
    uint32 unprocessedNotifications = 0;

    res.Reset();
    if (!sDatabase.RunQuery(res,
        "SELECT " // fixed DB query per client code -allan 18Jan15
        "  %u AS unreadMailCount,"
        "  %u AS upcomingEventCount, "
        "  %u AS unprocessedNotifications, "
        "  characterID,"
        "  petitionMessage, "
        "  gender, "
        "  bloodlineID, "
        "  createDateTime, "    //this is char create date
        "  startDateTime, "     //this is char joined corp date
        "  corporationID, "
        "  0 AS worldSpaceID, " /* this gives "walking around in [station xxxx] msgs on login screen when !=0 */
        "  stationID, "
        "  solarSystemID, "
        "  constellationID, "
        "  regionID, "
        "  0 AS allianceID, "       /** @todo (allan) reset this once alliances are implemented (already in corpDB) */
        "  0 AS allianceMemberStartDate,"
        "  'none' AS shortName, "   /** @todo (allan) this is alliance tickerName. reset this once alliances are implemented */
        "  bounty, "
        "  skillQueueEndTime, "
        "  skillPoints, "
        "  %u AS shipTypeID, "
        "  '%s' AS shipName, "
        "  securityRating, "
        "  title, "
        "  balance, "
        "  aurBalance, "
        "  15 AS daysLeft, "    /* this calls a subscription renewal warning on char select screen (see pic in gallery) when <= 10 */
        "  30 AS userType,"     /* 23 is trial acct.  30 is normal */
        "  paperDollState"      // used for re-customization.  see paperDollState:: in packet_types.h
        " FROM chrCharacters"
        " WHERE characterID=%u",
        unreadMailCount, upcomingEventCount, unprocessedNotifications,
        shipTypeID, shipName.c_str(), characterID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}

PyRep *CharacterDB::GetCharPublicInfo(uint32 characterID) {
    if (IsAgent(characterID)) {
        sLog.Error("CharacterDB::GetCharPublicInfo()", "Character %u is NPC.", characterID);
        return nullptr;
    }
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT "       // fixed DB Query   -allan 11Jan14   -update 20April16
        "  ch.typeID,"
        "  ch.name AS characterName,"
        "  ch.corporationID,"
        "  ch.raceID,"
        "  ch.bloodlineID,"
        "  ch.ancestryID,"
        "  ch.careerID,"
        "  ch.schoolID,"
        "  cs.schoolNameID, "
        "  ch.careerSpecialityID,"
        "  ch.age,"
        "  ch.createDateTime,"
        "  ch.gender,"
        "  ch.characterID,"
        "  ch.description,"
        "  ch.startDateTime"
        " FROM chrCharacters AS ch"
        "  LEFT JOIN chrSchools AS cs USING (schoolID) "
        " WHERE ch.characterID=%u", characterID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(DATABASE__ERROR, "Error in GetCharPublicInfo query: no data for char %d", characterID);
        return nullptr;
    }
    return(DBRowToKeyVal(row));

}

void CharacterDB::GetCharacterData(uint32 characterID, std::map<std::string, int64> &characterDataMap) {

    DBQueryResult res;
    DBResultRow row;

    if (!sDatabase.RunQuery(res,
        "SELECT "       // fixed DB Query   -allan 11Jan14      UD: 04Dec17
        "  ch.corporationID, "
        "  ch.stationID, "
        "  ch.solarSystemID, "
        "  ch.constellationID, "
        "  ch.regionID, "
        "  co.stationID, "  //5
        "  ch.corpRole, "
        "  ch.corpAccountKey, "
        "  ch.rolesAtAll, "
        "  ch.rolesAtBase, "
        "  ch.rolesAtHQ, "  //10
        "  ch.rolesAtOther, "
        "  ch.shipID, "
        "  ch.gender, "
        "  ch.bloodlineID, "
        "  ch.raceID, "     //15
        "  ch.locationID, "
        "  ch.baseID"
        " FROM chrCharacters AS ch"
        "    LEFT JOIN crpCorporation AS co USING (corporationID) "
        " WHERE characterID = %u", characterID))
    {
        sLog.Error("CharacterDB::GetCharacterData()", "Failed to query HQ of character's %u corporation: %s.", characterID, res.error.c_str());
    }

    if (!res.GetRow(row))
    {
        sLog.Error("CharacterDB::GetCharacterData()", "No valid rows were returned by the database query.");
        return;
    }

    characterDataMap["corporationID"] = row.GetUInt(0);
    characterDataMap["stationID"] = row.GetUInt(1);
    characterDataMap["solarSystemID"] = row.GetUInt(2);
    characterDataMap["constellationID"] = row.GetUInt(3);
    characterDataMap["regionID"] = row.GetUInt(4);
    characterDataMap["corporationHQ"] = row.GetUInt(5);
    characterDataMap["corpRole"] = row.GetInt64(6);
    characterDataMap["corpAccountKey"] = row.GetInt(7);
    characterDataMap["rolesAtAll"] = row.GetInt64(8);
    characterDataMap["rolesAtBase"] = row.GetInt64(9);
    characterDataMap["rolesAtHQ"] = row.GetInt64(10);
    characterDataMap["rolesAtOther"] = row.GetInt64(11);
    characterDataMap["shipID"] = row.GetUInt(12);
    characterDataMap["gender"] = row.GetUInt(13);
    characterDataMap["bloodlineID"] = row.GetUInt(14);
    characterDataMap["raceID"] = row.GetUInt(15);
    characterDataMap["locationID"] = row.GetUInt(16);
    characterDataMap["baseID"] = row.GetInt(17);
    uint32 stationID = row.GetInt(17);
    if (!GetCharHomeStation(characterID, stationID)) {
        ItemData iData( itemCloneAlpha, characterID, stationID, flagClone, 1 );
        iData.customInfo="active";
        InventoryItemRef initInvItem = sItemFactory.SpawnItem( iData );
    }
    characterDataMap["cloneStationID"] = stationID;
}

PyRep* CharacterDB::GetCharPublicInfo3(uint32 characterID) {
    // bounty, title, startDateTime, description, corporationID
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT "
        "  bounty,"
        "  title,"
        "  startDateTime,"
        "  description,"
        "  corporationID"
        " FROM chrCharacters "
        " WHERE characterID=%u", characterID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

PyRep *CharacterDB::GetInfoWindowDataForChar(uint32 characterID) {
    //corpID, allianceID, title
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT "
        "  ch.corporationID AS corpID,"
        "  co.allianceID,"
        "  ch.title"
        " FROM chrCharacters AS ch"
        "    LEFT JOIN crpCorporation AS co USING (corporationID)"
        " WHERE characterID=%u", characterID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(DATABASE__ERROR, "Expected at least one row when getting character corp info\n");
        return nullptr;
    }

    return DBRowToKeyVal(row);
}

std::string CharacterDB::GetCharName(uint32 characterID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT name FROM chrCharacters WHERE characterID=%u", characterID)) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return "";
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "Name not found for CharacterID %u.", characterID);
        return "";
    }

    return row.GetText(0);

}

PyRep* CharacterDB::GetContacts(uint32 charID, bool blocked)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery( res,
        "SELECT contactID, inWatchlist, relationshipID, labelMask"
        " FROM chrContacts WHERE ownerID = %u and blocked = %u", charID, blocked))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToIndexRowset(res, "contactID");
}

void CharacterDB::AddContact(uint32 charID)
{

}

void CharacterDB::UpdateContact(uint32 charID)
{

}

//just return all itemIDs which has ownerID set to characterID
bool CharacterDB::GetCharItems(uint32 characterID, std::vector<uint32> &into) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  itemID"
        " FROM entity"
        " WHERE ownerID = %u",
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to query items of char %u: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    while(res.GetRow(row))
        into.push_back(row.GetUInt(0));

    return true;
}

uint32 CharacterDB::PickAlternateShip(uint32 charID, uint32 locationID)
{   // this picks first ship that db finds belonging to charID in locationID
    DBQueryResult res;
    sDatabase.RunQuery(res,
        "SELECT e.itemID"
        " FROM entity AS e"
        "  LEFT JOIN invTypes USING (typeID)"
        "  LEFT JOIN invGroups USING (groupID)"
        " WHERE e.ownerID = %u"
        "  AND locationID = %u"
        "  AND categoryID = %u"
        "  LIMIT 1", charID, locationID, EVEDB::invCategories::Ship);

    DBResultRow row;
    if (res.GetRow(row))
        return row.GetUInt(0);
    else
        return 0;
}

void CharacterDB::SetCurrentShip(uint32 charID, uint32 shipID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE chrCharacters SET shipID = %u WHERE characterID = %u", shipID, charID))
        _log(DATABASE__ERROR, "Failed to save ship %u for character %u: %s.", shipID, charID, err.c_str());
}

void CharacterDB::SetCurrentPod(uint32 charID, uint32 podID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE chrCharacters SET capsuleID = %u WHERE characterID = %u", podID, charID))
        _log(DATABASE__ERROR, "Failed to save pod %u for character %u: %s.", podID, charID, err.c_str());
}

//returns a list of the itemID for all the clones belonging to the character
bool CharacterDB::GetCharClones(uint32 characterID, std::vector<uint32> &into) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT itemID FROM entity WHERE ownerID = %u AND flag='400'", characterID)) {
        _log(DATABASE__ERROR, "Failed to query clones of char %u: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    while(res.GetRow(row))
        into.push_back(row.GetUInt(0));

    return true;
}

//returns the itemID of the active clone
//if you want to get the typeID of the clone, please use GetActiveCloneType
bool CharacterDB::GetActiveClone(uint32 characterID, uint32 &itemID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  itemID"
        " FROM entity"
        " WHERE ownerID = %u"
        "  AND flag='400'"
        "  AND customInfo='active'",
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to query active clone of char %u: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (res.GetRow(row))
        itemID=row.GetUInt(0);
    else
        return false;
    return true;
}

//we use this function because, when we change the clone type,
//the cached item type doesn't change, so we need to read it
//directly from the db
bool CharacterDB::GetActiveCloneType(uint32 characterID, uint32 &typeID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  typeID"
        " FROM entity"
        " WHERE ownerID = %u"
        "  AND flag='400'"
        "  AND customInfo='active'",
        characterID))
    {
        _log(DATABASE__ERROR, "Failed to query active clone of char %u: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    res.GetRow(row);
    typeID=row.GetUInt(0);

    return true;
}

// Return the Home station of the char based on the active clone
bool CharacterDB::GetCharHomeStation(uint32 characterID, uint32 &stationID) {
	uint32 activeCloneID = 0;
	if (!GetActiveClone(characterID, activeCloneID)) {
        _log( CHARACTER__ERROR, "Could't get the active clone for char %u", characterID );
		return false;
	}

	DBQueryResult res;
	if ( !sDatabase.RunQuery(res,
		"SELECT locationID "
		"FROM entity "
		"WHERE itemID = %u",
		activeCloneID ))
	{
        _log(CHARACTER__ERROR, "Could't get clone location for char %u", characterID );
		return false;
	}

	DBResultRow row;
    if (res.GetRow(row))
        stationID = row.GetUInt(0);
	return true;
}

//replace all the typeID of the character's clones
bool CharacterDB::ChangeCloneType(uint32 characterID, uint32 typeID)
{
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT "
        " typeName "
        "FROM invTypes "
        "WHERE typeID = %u",
        typeID))
    {
        _log(DATABASE__ERROR, "Failed to change clone type of char %u: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row))
    {
        sLog.Error( "CharacterDB::ChangeCloneType()", "Could not find Clone typeID = %u in invTypes table.", typeID );
        return false;
    }
    std::string typeNameString = row.GetText(0);

    if (!sDatabase.RunQuery(res.error,
        "UPDATE "
        "entity "
        "SET typeID=%u, itemName='%s' "
        "WHERE ownerID=%u "
        "AND flag=400",
        typeID,
        typeNameString.c_str(),
                           characterID))
    {
        _log(DATABASE__ERROR, "Failed to change clone type of char %u: %s.", characterID, res.error.c_str());
        return false;
    }
    sLog.Debug( "CharacterDB", "Clone upgrade successful" );
    return true;
}

bool CharacterDB::GetAttributesFromAncestry(uint32 ancestryID, uint8 &intelligence, uint8 &charisma, uint8 &perception, uint8 &memory, uint8 &willpower) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        " SELECT "
        "  intelligence, charisma, perception, memory, willpower "
        " FROM chrAncestries "
        " WHERE ancestryID = %u ", ancestryID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return (false);
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(DATABASE__ERROR, "Failed to find ancestry information for ancestry %u", ancestryID);
        return false;
    }

    intelligence += row.GetUInt(0);
    charisma += row.GetUInt(1);
    perception += row.GetUInt(2);
    memory += row.GetUInt(3);
    willpower += row.GetUInt(4);

    return (true);
}

bool CharacterDB::GetCareerBySchool(uint32 schoolID, uint8 &raceID, uint32 &careerID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
     "SELECT raceID, careerID FROM careers WHERE schoolID = %u", schoolID))  {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(DATABASE__ERROR, "Failed to find matching career for school %u", schoolID);
        return false;
    }
    raceID = row.GetInt(0);
    careerID = row.GetInt(1);
    return true;
}

bool CharacterDB::GetCorporationBySchool(uint32 schoolID, uint32 &corporationID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res, "SELECT corporationID FROM chrSchools WHERE schoolID = %u", schoolID)) {
        codelog(DATABASE__ERROR, "Error in query: %S", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(DATABASE__ERROR, "Failed to find matching corporation for school %u", schoolID);
        return false;
    }
    corporationID = row.GetInt(0);
    return true;
}

/**
  * @todo Here should come a call to Corp??::CharacterJoinToCorp or what the heck... for now we only put it there
  */
bool CharacterDB::GetLocationCorporationByCareer(CharacterData& cdata, uint32& corporationID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
     "SELECT "      // fixed DB Query   -allan 01/02/14
     "  co.corporationID, "
     "  cs.schoolID, "
     "  co.stationID, "
     "  st.solarSystemID, "
     "  st.constellationID, "
     "  st.regionID "
     " FROM staStations AS st"
     "    LEFT JOIN crpCorporation AS co USING (stationID)"
     "    LEFT JOIN chrSchools AS cs ON cs.corporationID = co.corporationID"
     "    LEFT JOIN careers AS c USING (schoolID)"
     " WHERE c.careerID = %u", cdata.careerID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(DATABASE__ERROR, "Failed to find career %u", cdata.careerID);
        return false;
    }

    corporationID = row.GetUInt(0);
    cdata.schoolID = row.GetUInt(1);
    cdata.stationID = row.GetUInt(2);
    cdata.solarSystemID = row.GetUInt(3);
    cdata.constellationID = row.GetUInt(4);
    cdata.regionID = row.GetUInt(5);

    return true;
}

bool CharacterDB::GetCareerStationByCorporation(uint32 corporationID, uint32 &stationID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT stationID FROM crpCorporation WHERE corporationID = %u", corporationID)) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }
    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(DATABASE__ERROR, "Failed to find corporation %u", corporationID);
        return false;
    }

    stationID = row.GetUInt(0);
    return true;
}

bool CharacterDB::DoesCorporationExist(uint32 corpID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
     "SELECT "
     "  corporationID"
     " FROM crpCorporation"
     " WHERE corporationID = %u", corpID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    if (!res.ColumnCount() > 0)
        return true;
    return false;
}

void CharacterDB::SetAvatar(uint32 charID, PyRep* hairDarkness) {
	//populate the DB with avatar information
	DBerror err;
	if (!sDatabase.RunQuery(err,
		"INSERT INTO avatars (charID, hairDarkness)"
		" VALUES (%u, %f)",
		charID, hairDarkness->AsFloat()->value()))
	{
		codelog(DATABASE__ERROR, "Error in query: %s", err.c_str());
	}
}

void CharacterDB::SetAvatarColors(uint32 charID, uint32 colorID, uint32 colorNameA, uint32 colorNameBC, double weight, double gloss) {
	//add avatar colors to the DB
	DBerror err;
	if (!sDatabase.RunQuery(err,
		"INSERT INTO avatar_colors (charID, colorID, colorNameA, colorNameBC, weight, gloss)"
		" VALUES (%u, %u, %u, %u, %f, %f)",
		charID, colorID, colorNameA, colorNameBC, weight, gloss))
	{
		codelog(DATABASE__ERROR, "Error in query: %s", err.c_str());
	}
}

void CharacterDB::SetAvatarModifiers(uint32 charID, PyRep* modifierLocationID,  PyRep* paperdollResourceID, PyRep* paperdollResourceVariation) {
	//add avatar modifiers to the DB
	DBerror err;
	if (!sDatabase.RunQuery(err,
		"INSERT INTO avatar_modifiers (charID, modifierLocationID, paperdollResourceID, paperdollResourceVariation)"
		" VALUES (%u, %u, %u, %u)",
		charID,
		modifierLocationID->AsInt()->value(),
		paperdollResourceID->AsInt()->value(),
		paperdollResourceVariation->IsInt() ? paperdollResourceVariation->AsInt()->value() : 0 ))
	{
		codelog(DATABASE__ERROR, "Error in query: %s", err.c_str());
	}
}

void CharacterDB::SetAvatarSculpts(uint32 charID, PyRep* sculptLocationID, PyRep* weightUpDown, PyRep* weightLeftRight, PyRep* weightForwardBack) {
	//add avatar sculpts to the DB
	DBerror err;
	if (!sDatabase.RunQuery(err,
		"INSERT INTO avatar_sculpts (charID, sculptLocationID, weightUpDown, weightLeftRight, weightForwardBack)"
		" VALUES (%u, %u, %f, %f, %f)",
		charID,
		sculptLocationID->AsInt()->value(),
		weightUpDown->IsFloat() ? weightUpDown->AsFloat()->value() : 0.0,
		weightLeftRight->IsFloat() ? weightLeftRight->AsFloat()->value() : 0.0,
		weightForwardBack->IsFloat() ? weightForwardBack->AsFloat()->value() : 0.0))
	{
		codelog(DATABASE__ERROR, "Error in query: %s", err.c_str());
	}
}

bool CharacterDB::GetBaseSkills(std::map<uint32, uint32> &into) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT skillTypeID, level FROM sklBaseSkills"))  {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    while (res.GetRow(row))
        into[row.GetUInt(0)] = row.GetUInt(1);

    return true;
}

bool CharacterDB::GetSkillsByRace(uint32 raceID, std::map<uint32, uint32> &into) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT skillTypeID, level FROM sklRaceSkills WHERE raceID = %u ", raceID)) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    while (res.GetRow(row)) {
        if (into.find(row.GetUInt(0)) == into.end())
            into[row.GetUInt(0)] = row.GetUInt(1);
        else
            into[row.GetUInt(0)] += row.GetUInt(1);
        //check to avoid more than 5 levels of a skill
        if (into[row.GetUInt(0)] > 5)
            into[row.GetUInt(0)] = 5;
    }

    return true;
}

bool CharacterDB::GetSkillsByCareer(uint32 careerID, std::map<uint32, uint32> &into) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT "
        "  skillTypeID, level"
        " FROM sklCareerSkills"
        " WHERE careerID = %u", careerID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    while (res.GetRow(row)) {
        if (into.find(row.GetUInt(0)) == into.end())
            into[row.GetUInt(0)] = row.GetUInt(1);
        else
            into[row.GetUInt(0)] += row.GetUInt(1);
        //check to avoid more than 5 levels of a skill
        if (into[row.GetUInt(0)] > 5)
            into[row.GetUInt(0)] = 5;
    }

    return true;
}

PyString *CharacterDB::GetNote(uint32 ownerID, uint32 itemID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT note FROM chrNotes WHERE ownerID = %u AND itemID = %u",  ownerID, itemID))  {
        codelog(DATABASE__ERROR, "Error on query: %s", res.error.c_str());
        return nullptr;
    }
    DBResultRow row;
    if (!res.GetRow(row))
        return nullptr;

    return (new PyString(row.GetText(0)));
}

bool CharacterDB::SetNote(uint32 ownerID, uint32 itemID, const char *str) {
    DBerror err;
    if (str[0] == '\0') {
        // str is empty
        if (!sDatabase.RunQuery(err,
            "DELETE FROM `chrNotes` "
            " WHERE itemID = %u AND ownerID = %u LIMIT 1",
             itemID, ownerID)
            )
        {
            codelog(DATABASE__ERROR, "Error on query: %s", err.c_str());
            return false;
        }
    } else {
        // escape it before insertion
        std::string escaped;
        sDatabase.DoEscapeString(escaped, str);

        if (!sDatabase.RunQuery(err,
            "REPLACE INTO `chrNotes` ( ownerID, itemID, note)"
            " VALUES (%u, %u, '%s')",
            ownerID, itemID, escaped.c_str())
            )
        {
            codelog(DATABASE__ERROR, "Error on query: %s", err.c_str());
            return false;
        }
    }

    return true;
}

uint32 CharacterDB::AddOwnerNote(uint32 charID, const std::string & label, const std::string & content) {
    DBerror err;
    uint32 id;
    std::string lblS;
    sDatabase.DoEscapeString(lblS, label);

    std::string contS;
    sDatabase.DoEscapeString(contS, content);

    if (!sDatabase.RunQueryLID(err, id,
        "INSERT INTO chrOwnerNote (ownerID, label, note) VALUES (%u, '%s', '%s');",
        charID, lblS.c_str(), contS.c_str()))
    {
        codelog(DATABASE__ERROR, "Error on query: %s", err.c_str());
        return 0;
    }

    return id;
}

bool CharacterDB::EditOwnerNote(uint32 charID, uint32 noteID, const std::string & label, const std::string & content) {
    DBerror err;
    std::string contS;
    sDatabase.DoEscapeString(contS, content);

    if (!sDatabase.RunQuery(err,
        "UPDATE chrOwnerNote SET note = '%s' WHERE ownerID = %u AND noteID = %u;",
        contS.c_str(), charID, noteID))
    {
        codelog(DATABASE__ERROR, "Error on query: %s", err.c_str());
        return false;
    }

    return true;
}

PyRep *CharacterDB::GetOwnerNoteLabels(uint32 charID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT noteID, label FROM chrOwnerNote WHERE ownerID = %u", charID))
    {
        codelog(DATABASE__ERROR, "Error on query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

PyRep *CharacterDB::GetOwnerNote(uint32 charID, uint32 noteID) {
    /*
                    [PyTuple 6 items]
                      [PyTuple 2 items]
                        [PyString "noteDate"]
                        [PyInt 64]
                      [PyTuple 2 items]
                        [PyString "typeID"]
                        [PyInt 2]
                      [PyTuple 2 items]
                        [PyString "referenceID"]
                        [PyInt 3]
                      [PyTuple 2 items]
                        [PyString "note"]
                        [PyInt 130]
                      [PyTuple 2 items]
                        [PyString "userID"]
                        [PyInt 3]
                      [PyTuple 2 items]
                        [PyString "label"]
                        [PyInt 130]
          [PyPackedRow 19 bytes]
            ["noteDate" => <129041092800000000> [FileTime]]
            ["typeID" => <1> [I2]]
            ["referenceID" => <1661059544> [I4]]
            ["note" => <1::F::0::Main|> [WStr]]
            ["userID" => <0> [I4]]
            ["label" => <S:Folders> [WStr]]
            */
    DBQueryResult res;

    if (!sDatabase.RunQuery(res, "SELECT note FROM chrOwnerNote WHERE ownerID = %u AND noteID = %u", charID, noteID))
    {
        codelog(DATABASE__ERROR, "Error on query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

void CharacterDB::EditLabel(uint32 charID, uint32 labelID, uint32 color, std::string name)
{
    std::string eName;
    sDatabase.DoEscapeString(eName, name);

    DBQueryResult res;
    sDatabase.RunQuery(res, "UPDATE chrLabels SET color = %u, name = '%s' WHERE ownerID = %u AND labelID = %u", color, eName.c_str(), charID, labelID);
}

PyRep* CharacterDB::GetLabels(uint32 charID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT labelID, color, name FROM chrLabels WHERE ownerID = %u", charID)) {
        codelog(DATABASE__ERROR, "Error on query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCIndexedRowset(res, "labelID");
}

void CharacterDB::SetLabel(uint32 charID, uint32 color, std::string name)
{
    std::string eName;
    sDatabase.DoEscapeString(eName, name);

    DBQueryResult res;
    sDatabase.RunQuery(res, "INSERT INTO chrLabels (color, name, ownerID) VALUES (%u, '%s', %u)", color, eName.c_str(), charID);
}

void CharacterDB::DeleteLabel(uint32 charID, uint32 labelID)
{

}


uint32 CharacterDB::djb2_hash( const char* str )
{
    uint32 hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

void CharacterDB::load_name_validation_set()
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT characterID, name FROM chrCharacters")) {
        codelog(DATABASE__ERROR, "Error in query for %s", res.error.c_str());
        return;
    }

    DBResultRow row;
    while (res.GetRow(row)) {
        uint32 characterID = row.GetUInt(0);
        const char* name = row.GetText(1);

        //printf("initializing name validation: %s\n", name);
        uint32 hash = djb2_hash(name);

        mNameValidation.insert(hash);
        mIdNameContainer.insert(std::make_pair(characterID, name));
    }
}

bool CharacterDB::add_name_validation_set( const char* name, uint32 characterID )
{
    if (name == NULL || *name == '\0')
        return false;

    uint32 hash = djb2_hash(name);
    /* check if the name is already present ( this should not be possible but we all know how hackers are ) */
    if (mNameValidation.find(hash) != mNameValidation.end())
    {
        printf("CharacterDB::add_name_validation_set: unable to add: %s as its a dupe", name);
        return false;
    }

    mNameValidation.insert(hash);
    mIdNameContainer.insert(std::make_pair(characterID, name));
    return true;
}

bool CharacterDB::del_name_validation_set( uint32 characterID )
{
    CharIdNameMapItr helper_itr = mIdNameContainer.find(characterID);
    /* if we are unable to find the entry... return.
     * @note we do risk keeping the name in the name validation.
     * which I am willing to take.
     */
    if (helper_itr == mIdNameContainer.end())
        return false;

    const char* name = helper_itr->second.c_str();
    if (name == NULL || *name == '\0')
        return false;

    uint32 hash = djb2_hash(name);

    CharValidationSetItr name_itr = mNameValidation.find(hash);
    if (name_itr != mNameValidation.end())
    {
        // we found the name hash... deleting
        mNameValidation.erase(name_itr);
        mIdNameContainer.erase(helper_itr);
        return true;
    }
    else
    {
        /* normally this should never happen... */
        printf("CharacterDB::del_name_validation_set: unable to remove: %s as its not in the set", name);
        return false;
    }
}

bool CharacterDB::LoadSkillQueue(uint32 characterID, SkillQueue &into) {
    DBQueryResult res;
    if (!sDatabase.RunQuery( res, "SELECT typeID, level FROM chrSkillQueue WHERE characterID = %u ORDER BY orderIndex ASC", characterID)) {
        _log(DATABASE__ERROR, "Failed to query skill queue of character %u: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    while (res.GetRow(row)) {
        QueuedSkill qs;
            qs.typeID = row.GetUInt( 0 );
            qs.level = row.GetUInt( 1 );
        into.push_back( qs );
    }

    return true;
}

bool CharacterDB::LoadPausedSkillQueue(uint32 characterID, SkillQueue &into) {
    DBQueryResult res;
    if (!sDatabase.RunQuery( res, "SELECT typeID, level FROM chrPausedSkillQueue WHERE characterID = %u ORDER BY orderIndex ASC", characterID)) {
        _log(DATABASE__ERROR, "Failed to query paused skill queue of character %u: %s.", characterID, res.error.c_str());
        return false;
    }

    DBResultRow row;
    while (res.GetRow(row)) {
        QueuedSkill qs;
            qs.typeID = row.GetUInt( 0 );
            qs.level = row.GetUInt( 1 );
        into.push_back( qs );
    }

    // now, delete paused queue because subsquent pressess of 'apply' button will add paused queue again, and again, etc...
    DBerror err;
    if (!sDatabase.RunQuery(err,"DELETE FROM chrPausedSkillQueue WHERE characterID = %u", characterID)) {
        _log(DATABASE__ERROR, "Failed to delete skill queue of character %u: %s.", characterID, err.c_str());
        return false;
    }

    return true;
}

bool CharacterDB::SaveSkillQueue(uint32 characterID, SkillQueue &data) {
    DBerror err;
    if (!sDatabase.RunQuery(err, "DELETE FROM chrSkillQueue WHERE characterID = %u", characterID)) {
        _log(DATABASE__ERROR, "Failed to delete skill queue of character %u: %s.", characterID, err.c_str());
        return false;
    }

    if (data.empty())
        return true;

    std::string query;

    for (uint8 i = 0; i < data.size(); i++) {
        const QueuedSkill &qs = data[ i ];
        char buf[ 64 ];
        snprintf( buf, 64, "(%u, %u, %u, %u)", characterID, i, qs.typeID, qs.level );

        if (i > 0)
            query += ',';
        query += buf;
    }

    if ( !sDatabase.RunQuery( err,
        "INSERT INTO chrSkillQueue (characterID, orderIndex, typeID, level)"
        " VALUES %s",query.c_str()))
    {
        _log(DATABASE__ERROR, "SaveSkillQueue - unable to save data - %s", err.c_str());
    }
    return true;
}

bool CharacterDB::SavePausedSkillQueue(uint32 characterID, SkillQueue &data) {
    DBerror err;
    if (!sDatabase.RunQuery(err, "DELETE FROM chrPausedSkillQueue WHERE characterID = %u", characterID)) {
        _log(DATABASE__ERROR, "Failed to delete paused skill queue of character %u: %s.", characterID, err.c_str());
        return false;
    }

    if (data.empty())
        return true;

    std::string query;

    for (uint8 i = 0; i < data.size(); i++) {
        const QueuedSkill &qs = data[ i ];
        char buf[ 64 ];
        snprintf( buf, 64, "(%u, %u, %u, %u)", characterID, i, qs.typeID, qs.level );

        if (i > 0)
            query += ',';
        query += buf;
    }

    if ( !sDatabase.RunQuery( err,
        "INSERT INTO chrPausedSkillQueue (characterID, orderIndex, typeID, level)"
        " VALUES %s",query.c_str()))
    {
        _log(DATABASE__ERROR, "SavePausedSkillQueue - unable to save data - %s", err.c_str());
    }

    return true;
}

void CharacterDB::SaveSkillHistory(uint16 eventID, double logDate, uint32 characterID, uint32 skillTypeID, uint8 skillLevel, double absolutePoints) {
    DBerror err;
    if ( !sDatabase.RunQuery( err,
        "INSERT INTO chrSkillHistory (eventTypeID, logDate, characterID, skillTypeID, skillLevel, absolutePoints)"
        " VALUES (%u, %f, %u, %u, %u, %f)", eventID, logDate, characterID, skillTypeID, skillLevel, absolutePoints ))
            _log(DATABASE__ERROR, "Failed to set chrSkillHistory for character %u: %s", characterID, err.c_str());
}

PyRep* CharacterDB::GetSkillHistory(uint32 characterID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT logDate, eventTypeID, skillTypeID, absolutePoints"
        " FROM chrSkillHistory"
        " WHERE characterID = %u"
        " ORDER BY logDate DESC"
        " LIMIT 50",
        characterID )) {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

void CharacterDB::UpdateSkillQueueEndTime(int64 endtime, uint32 charID) {
    DBerror err;
    sDatabase.RunQuery( err, "UPDATE chrCharacters SET skillQueueEndTime = %lli WHERE characterID = %u ", endtime, charID );
}

void CharacterDB::SetLogInTime(uint32 charID)
{
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE chrCharacters SET logonDateTime = %f WHERE characterID = %u", GetFileTimeNow(), charID );
}

void CharacterDB::SetLogOffTime(uint32 charID)
{
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE chrCharacters SET logoffDateTime = %f WHERE characterID = %u", GetFileTimeNow(), charID );
}

void CharacterDB::addOwnerCache(uint32 ownerID, std::string ownerName, uint32 typeID) {
    DBerror err;
    sDatabase.RunQuery(err,
        "INSERT INTO cacheOwners(ownerID, ownerName, typeID)"
        " VALUES (%u, '%s', %u)",
        ownerID, ownerName.c_str(), typeID);
}

PyRep* CharacterDB::GetBounty(uint32 charID, uint32 ownerID) {
    DBQueryResult res;
    sDatabase.RunQuery(res, "SELECT characterID, bounty FROM webBounties WHERE characterID = %u OR ownerID = %u", charID, ownerID);
    return DBResultToRowset(res);
}

PyRep* CharacterDB::GetTopBounties() {
    DBQueryResult res;
    sDatabase.RunQuery(res,
                       "SELECT c.characterID, c.bounty, c.online, o.name AS ownerName"
                       " FROM webBounties AS b"
                       " LEFT JOIN chrCharacters AS c ON c.characterID = b.characterID"
                       " LEFT JOIN chrCharacters AS o ON o.characterID = b.ownerID"
                       " ORDER BY c.bounty DESC"
                       " LIMIT 15");
    return DBResultToRowset(res);
}

void CharacterDB::AddBounty(uint32 charID, uint32 ownerID, uint32 amount) {
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE chrCharacters SET bounty = bounty + %u WHERE characterID = %u", amount, charID);
    sDatabase.RunQuery(err,
        "INSERT INTO webBounties(characterID, ownerID, bounty, timePlaced)"
        " VALUES (%u, %u, %u, UNIX_TIMESTAMP(CURRENT_TIMESTAMP) )",
        charID, ownerID, amount );
}

PyRep* CharacterDB::GetKillOrLoss(uint32 charID) {
    /*
     *    def GetKillsRecentKills(self, num, startIndex):
     *        shipKills = sm.RemoteSvc('charMgr').GetRecentShipKillsAndLosses(num, startIndex)
     *        return [ k for k in shipKills if k.finalCharacterID == eve.session.charid ]
     *
     *    def GetKillsRecentLosses(self, num, startIndex):
     *        shipKills = sm.RemoteSvc('charMgr').GetRecentShipKillsAndLosses(num, startIndex)
     *        return [ k for k in shipKills if k.victimCharacterID == eve.session.charid ]
     */
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  killID,"
        "  solarSystemID,"
        "  victimCharacterID,"
        "  victimCorporationID,"
        "  victimAllianceID,"
        "  victimFactionID,"
        "  victimShipTypeID,"
        "  finalCharacterID,"
        "  finalCorporationID,"
        "  finalAllianceID,"
        "  finalFactionID,"
        "  finalShipTypeID,"
        "  finalWeaponTypeID,"
        "  killBlob,"
        "  killTime,"
        "  victimDamageTaken,"
        "  finalSecurityStatus,"
        "  finalDamageDone,"
        "  moonID"
        " FROM chrKillTable"
        " WHERE ((victimCharacterID = %u) OR (finalCharacterID = %u))", charID, charID))
        /* should we limit this? */
    {
        codelog(DATABASE__ERROR, "Error on query: %s", res.error.c_str());
        return nullptr;
    }

    _log(DATABASE__RESULTS, "GetKillOrLoss for %u returned %u items", charID, res.GetRowCount());

    return DBResultToCRowset(res);
}

void CharacterDB::SetCorpRole(uint32 charID, int64 role)
{
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE chrCharacters SET corpRole = %lli WHERE characterID = %u", role, charID);
}

int64 CharacterDB::GetCorpRole(uint32 charID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT corpRole FROM chrCharacters WHERE characterID = %u", charID)) {
        sLog.Error("CharacterDB::GetCorpRole()", "Failed to query role of character %u: %s.", charID, res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        sLog.Error("CharacterDB::GetCorpRole()", "No valid rows were returned by the database query.");
        return 0;
    }

    return row.GetUInt(0);
}

uint32 CharacterDB::GetCorpID(uint32 charID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT corporationID FROM chrCharacters WHERE characterID = %u", charID)) {
        sLog.Error("CharacterDB::GetCorpID()", "Failed to query corpID of character %u: %s.", charID, res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        sLog.Error("CharacterDB::GetCorpID()", "No valid rows were returned by the database query.");
        return 0;
    }

    return row.GetUInt(0);
}

float CharacterDB::GetCorpTaxRate(uint32 charID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT taxRate FROM crpCorporation WHERE corporationID = (SELECT corporationID FROM chrCharacters WHERE characterID = %u)", charID))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row))
        return 0.0f;

    return row.GetFloat(0);
}

PyRep* CharacterDB::GetMyCorpMates(uint32 corpID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT solarSystemID AS locationID, characterID FROM chrCharacters WHERE corporationID = %u", corpID)) {
        sLog.Error("CharacterDB::GetMyCorpMates()", "Failed to query corpMates for corpID %u: %s.", corpID, res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

void CharacterDB::VisitSystem(uint32 solarSystemID, uint32 charID) {
    DBerror err;
    sDatabase.RunQuery(err,
            "INSERT INTO chrVisitedSystems (characterID, solarSystemID, visits, lastDateTime)"
            "VALUES (%u, %u, 1, %f)"
            " ON DUPLICATE KEY UPDATE"
            " visits = visits +1,"
            " lastDateTime = %f", charID, solarSystemID, GetFileTimeNow(), GetFileTimeNow());
}
