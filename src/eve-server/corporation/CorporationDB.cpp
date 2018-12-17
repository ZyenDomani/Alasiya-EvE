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
    Author:     Zhur
*/

#include "eve-server.h"

#include "Client.h"
#include "StaticDataMgr.h"
#include "character/Character.h"
#include "corporation/CorporationDB.h"

/*
 * CORP__DB_ERROR
 * CORP__DB_WARNING
 * CORP__DB_INFO
 * CORP__DB_MESSAGE
 */

PyObject *CorporationDB::ListCorpStations(uint32 corp_id) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT "
        "   stationID, stationTypeID AS typeID"
        " FROM staStations"
        " WHERE corporationID=%u",
            corp_id
    ))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}
/*
PyObject *CorporationDB::ListStationOffices(uint32 station_id) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT officeID, officeFolderID, corporationID, stationID, typeID, lockDown, rentalFee, expiryDateTime"
        " FROM staOffices"
        " WHERE stationID = %u", station_id ))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}
*/
PyObject *CorporationDB::ListStationCorps(uint32 station_id) {

    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT "
        "   corporationID,corporationName,description,shares,graphicID,"
        "   memberCount,ceoID,stationID,raceID,corporationType,creatorID,"
        "   hasPlayerPersonnelManager,tickerName,sendCharTerminationMessage,"
        "   shape1,shape2,shape3,color1,color2,color3,typeface,memberLimit,"
        "   allowedMemberRaceIDs,url,taxRate,minimumJoinStanding,division1,"
        "   division2,division3,division4,division5,division6,division7,"
        "   allianceID,deleted,isRecruiting"
        " FROM crpCorporation"
//no idea what the criteria should be here...
        " WHERE stationID=%u",
            station_id
    ))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

PyObject *CorporationDB::ListStationOwners(uint32 stationID) {
    /** @todo  this needs work.... */
    DBQueryResult res;
  if (IsStation(stationID)) {
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  s.corporationID AS ownerID,"
        "  c.corporationName AS ownerName,"
        "  c.corporationType AS typeID,"
        //"  c.corporationID AS ownerNameID,"
        "  cs.gender"
        " FROM staStations AS s"
        "  LEFT JOIN crpCorporation AS c ON cs.corporationID = c.corporationID"
        "  LEFT JOIN chrNPCCharacters AS cs ON cs.characterID = c.ceoID"
        " WHERE s.stationID = %u", stationID ))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
  } else {
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  c.corporationID AS ownerID,"
        "  c.corporationName AS ownerName,"
        "  c.corporationType AS typeID,"
        //"  c.corporationID AS ownerNameID,"
        "  cs.gender"
        " FROM entity AS e"
        "  LEFT JOIN crpCorporation AS c USING ( corporationID )"
        "  LEFT JOIN chrCharacters AS cs ON cs.characterID = c.ceoID"
        " WHERE e.itemID = %u", stationID ))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
  }

    return DBResultToRowset(res);
}

PyRep *CorporationDB::GetCorpInfo(uint32 corpID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT corporationID, corporationName, description, creatorID, url, allianceID, deleted, stationID, ceoID"
        " FROM crpCorporation WHERE corporationID = %u", corpID))
    {
        codelog(CORP__DB_ERROR, "Error in retrieving corporation's data (%u)", corpID);
        return nullptr;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(CORP__DB_WARNING, "Unable to find corporation's data (%u)", corpID);
        return nullptr;
    }

    //return DBResultToRowset(res);
    return DBRowToKeyVal(row);
}

PyObject *CorporationDB::GetCorporation(uint32 corpID) {
    std::string table = "crpWalletDivisons";
    if (IsNPCCorp(corpID))
        table = "crpNPCWalletDivisons";
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT "
        "   c.corporationID,c.corporationName,c.description,c.tickerName,c.url,"
        "   c.taxRate,c.minimumJoinStanding,c.corporationType,c.hasPlayerPersonnelManager,"
        "   c.sendCharTerminationMessage,c.creatorID,c.ceoID,c.stationID,c.raceID,"
        "   c.allianceID,c.shares,c.memberCount,c.memberLimit,c.allowedMemberRaceIDs,"
        "   c.shape1,c.shape2,c.shape3,c.color1,c.color2,c.color3,c.typeface,c.deleted,c.isRecruiting,c.warFactionID,"
        "   w.division1,w.division2,w.division3,w.division4,w.division5,w.division6,"
        "   w.division7,w.walletDivision1,w.walletDivision2,w.walletDivision3,w.walletDivision4,"
        "   w.walletDivision5,w.walletDivision6,w.walletDivision7"
        " FROM crpCorporation AS c"
        "  LEFT JOIN %s AS w USING (corporationID)"
        " WHERE corporationID = %u", table.c_str(), corpID))
    {
        codelog(CORP__DB_ERROR, "Error in retrieving corporation's data (%u)", corpID);
        return nullptr;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(CORP__DB_WARNING, "Unable to find corporation's data (%u)", corpID);
        return nullptr;
    }

    return DBRowToRow(row);
    //return DBResultToRowset(res);
}

PyRep *CorporationDB::GetCorporations(uint32 corpID) {
    std::string table = "crpWalletDivisons";
    if (IsNPCCorp(corpID))
        table = "crpNPCWalletDivisons";
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "   c.corporationID,c.corporationName,c.description,c.tickerName,c.url,"
        "   c.taxRate,c.minimumJoinStanding,c.corporationType,c.hasPlayerPersonnelManager,"
        "   c.sendCharTerminationMessage,c.creatorID,c.ceoID,c.stationID,c.raceID,"
        "   c.allianceID,c.shares,c.memberCount,c.memberLimit,c.allowedMemberRaceIDs,"
        "   c.shape1,c.shape2,c.shape3,c.color1,c.color2,c.color3,c.typeface,c.deleted,c.isRecruiting,c.warFactionID,"
        "   w.division1,w.division2,w.division3,w.division4,w.division5,w.division6,"
        "   w.division7,w.walletDivision1,w.walletDivision2,w.walletDivision3,w.walletDivision4,"
        "   w.walletDivision5,w.walletDivision6,w.walletDivision7"
        " FROM crpCorporation AS c"
        "  LEFT JOIN %s AS w USING (corporationID)"
        " WHERE corporationID = %u", table.c_str(), corpID))
    {
        codelog(CORP__DB_ERROR, "Error in retrieving corporation's data (%u)", corpID);
        return nullptr;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(CORP__DB_WARNING, "Unable to find corporation's data (%u)", corpID);
        return nullptr;
    }

    return DBRowToPackedRow(row);
}

PyObject* CorporationDB::GetCorporationBills(uint32 corpID, bool payable)
{
    DBQueryResult res;
    if (payable) {
        if (!sDatabase.RunQuery(res,
            "SELECT billID, billTypeID, debtorID, creditorID, amount, dueDateTime, interest,"
            "externalID, paid externalID2 FROM billsPayable WHERE debtorID = %u", corpID))
        {
            codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
            return nullptr;
        }
    } else {
        if (!sDatabase.RunQuery(res,
            "SELECT billID, billTypeID, debtorID, creditorID, amount, dueDateTime, interest,"
            "externalID, paid externalID2 FROM billsReceivable WHERE creditorID = %u", corpID))
        {
            codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
            return nullptr;
        }
    }

    return DBResultToRowset(res);
}

PyObject *CorporationDB::GetEmploymentRecord(uint32 charID) {
    DBQueryResult res;
    //do we really need this order by??
    if (!sDatabase.RunQuery(res,
        "SELECT startDate, corporationID, deleted "
        "   FROM chrEmployment "
        "   WHERE characterID = %u "
        "   ORDER BY startDate DESC", charID
        ))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return (DBResultToRowset(res));
}

PyObject* CorporationDB::GetMedalsReceived( uint32 charID )
{
    sLog.Debug( "CorporationDB", "Called GetMedalsReceived stub." );

    util_Rowset rs;

    rs.header.push_back( "medalID" );
    rs.header.push_back( "title" );
    rs.header.push_back( "description" );
    rs.header.push_back( "ownerID" );
    rs.header.push_back( "" );    //wtf??
    rs.header.push_back( "issuerID" );
    rs.header.push_back( "date" );
    rs.header.push_back( "reason" );
    rs.header.push_back( "status" );

    return rs.Encode();
}

PyObject* CorporationDB::GetMedalDetails(uint32 medalID)
{
    sLog.Debug( "CorporationDB", "Called GetMedalDetails stub." );

    util_Rowset rs;

    rs.header.push_back( "medalID" );
    rs.header.push_back( "title" );
    rs.header.push_back( "description" );
    rs.header.push_back( "ownerID" );
    rs.header.push_back( "" );    //wtf??
    rs.header.push_back( "issuerID" );
    rs.header.push_back( "date" );
    rs.header.push_back( "reason" );
    rs.header.push_back( "status" );

    return rs.Encode();
}

static std::string _IoN( PyRep* r )
{
    if ( !r->IsInt() )
        return "NULL";
    return itoa( r->AsInt()->value() );
}

bool CorporationDB::AddCorporation(Call_AddCorporation & corpInfo, Client* pClient, uint32 & corpID) {
    std::string cName, cDesc, cTick, cURL;
    sDatabase.DoEscapeString(cName, corpInfo.corpName);
    sDatabase.DoEscapeString(cDesc, corpInfo.description);
    sDatabase.DoEscapeString(cTick, corpInfo.corpTicker);
    sDatabase.DoEscapeString(cURL, corpInfo.url);

    Character* pChar = pClient->GetChar().get();
    uint32 charID = pClient->GetCharacterID();
    uint8 raceID = (uint8)pChar->race();

    // set member limit based on creator's skills
    uint16 mLimit = (pChar->GetSkillLevel(skillCorporationManagement) * 20);//101
    mLimit += (pChar->GetSkillLevel(skillMegacorpManagement) * 100);        //601
    mLimit += (pChar->GetSkillLevel(skillEmpireControl) * 400);             //2601
    mLimit += (pChar->GetSkillLevel(skillSovereignty) * 2000);              //12601

    /** @todo  allowedMemberRaceIDs will need to be bitwise...it is a flag. */
    DBerror err;
    if (!sDatabase.RunQueryLID(err, corpID,
        " INSERT INTO crpCorporation ( "
        "   corporationName, description, tickerName, url, taxRate, corporationType, hasPlayerPersonnelManager, "
        "   creatorID, ceoID, stationID, raceID, shares, memberLimit, allowedMemberRaceIDs,"
        "   graphicID, color1, color2, color3, shape1, shape2, shape3, "
        "   isRecruiting ) "
        " VALUES "
        "   ('%s', '%s', '%s', '%s', %f, 2, 1, "
        "    %u, %u, %u, %u, 1000, %u, %u,"
        "    0, %s, %s, %s, %s, %s, %s, "
        "    %u) ",
        cName.c_str(), cDesc.c_str(), cTick.c_str(), cURL.c_str(), corpInfo.taxRate,
        charID, charID, pClient->GetStationID(), raceID, mLimit, raceID,
	_IoN(corpInfo.color1).c_str(),
	_IoN(corpInfo.color2).c_str(),
	_IoN(corpInfo.color3).c_str(),
	_IoN(corpInfo.shape1).c_str(),
	_IoN(corpInfo.shape2).c_str(),
	_IoN(corpInfo.shape3).c_str(),
	corpInfo.applicationEnabled))
    {
        codelog(CORP__DB_ERROR, "Error in AddCorporation query: %s", err.c_str());
        return false;
    }

    // It has to go into the eveStaticOwners too
    if (!sDatabase.RunQuery(err, " INSERT INTO eveStaticOwners (ownerID,ownerName,typeID) VALUES (%u, '%s', 2)", corpID, cName.c_str()))  {
        codelog(CORP__DB_ERROR, "Error in query: %s", err.c_str());
        return false;
    }

    // create default wallet info
    sDatabase.RunQuery(err, "INSERT INTO crpWalletDivisons (corporationID) VALUES (%u)", corpID);

    // create corp-owned shares
    sDatabase.RunQuery(err, "INSERT INTO crpShares (corporationID, shareholderID, shares, shareholderCorporationID)"
                            " VALUES (%i, %i, %i, %u)", corpID, corpID, 1000, corpID);

    return true;
}

#define _NI(a, b) if (row.IsNull(b)) { cc.a = PyStatic.NewNone(); } else { cc.a = new PyInt(row.GetUInt(b)); }
#define _NS(a, b) if (row.IsNull(b)) { cc.a = PyStatic.NewNone(); } else { cc.a = new PyString(row.GetText(b)); }

bool CorporationDB::CreateCorporationChangePacket(OnCorporationChanged & cc, uint32 oldCorpID, uint32 newCorpID) {
    std::string table = "crpWalletDivisons";
    if (IsNPCCorp(newCorpID))
        table = "crpNPCWalletDivisons";
    DBQueryResult res;
    DBResultRow row;
    if (!sDatabase.RunQuery(res,
        " SELECT "
        "  c.corporationID, c.corporationName, c.description, c.tickerName, c.allianceID, c.warFactionID, c.url, c.taxRate,"
        "  c.minimumJoinStanding, c.corporationType, c.hasPlayerPersonnelManager, c.sendCharTerminationMessage, c.creatorID, c.ceoID, c.stationID, c.raceID, "
        "  c.shares, c.memberCount, c.memberLimit, c.allowedMemberRaceIDs, c.shape1, c.shape2, c.shape3, c.color1, c.color2, c.color3,"
        "  c.typeface, w.division1, w.division2, w.division3, w.division4, w.division5, w.division6, w.division7, w.walletDivision1, w.walletDivision2,"
        "  w.walletDivision3, w.walletDivision4, w.walletDivision5, w.walletDivision6, w.walletDivision7, c.deleted, c.isRecruiting "
        " FROM crpCorporation AS c"
        "  LEFT JOIN %s AS w USING (corporationID)"
        " WHERE corporationID = %u ",table.c_str(), newCorpID
        ))
    {
        codelog(CORP__DB_ERROR, "Error in retrieving new corporation's data (%u)", newCorpID);
        return false;
    }

    if (!res.GetRow(row)) {
        codelog(CORP__DB_WARNING, "Unable to find new corporation's data (%u)", newCorpID);
        return false;
    }

    cc.corporationIDNew = row.GetUInt(0);
    cc.corporationNameNew = row.GetText(1);
    cc.descriptionNew = row.GetText(2);
    cc.tickerNameNew = row.GetText(3);
    _NI(allianceIDNew, 4);
    _NI(warFactionIDNew, 5);
    cc.urlNew = row.GetText(6);
    cc.taxRateNew = row.GetDouble(7);
    cc.minimumJoinStandingNew = row.GetDouble(8);
    cc.corporationTypeNew = row.GetUInt(9);
    cc.hasPlayerPersonnelManagerNew = row.GetUInt(10);
    cc.sendCharTerminationMessageNew = row.GetUInt(11);
    cc.creatorIDNew = row.GetUInt(12);
    cc.ceoIDNew = row.GetUInt(13);
    cc.stationIDNew = row.GetUInt(14);
    _NI(raceIDNew, 15);
    cc.sharesNew = row.GetInt64(16);
    cc.memberCountNew = row.GetUInt(17);
    cc.memberLimitNew = row.GetUInt(18);
    cc.allowedMemberRaceIDsNew = row.GetUInt(19);
    _NI(shape1New, 20);
    _NI(shape2New, 21);
    _NI(shape3New, 22);
    _NI(color1New, 23);
    _NI(color2New, 24);
    _NI(color3New, 25);
    _NI(typefaceNew, 26);
    _NS(division1New, 27);
    _NS(division2New, 28);
    _NS(division3New, 29);
    _NS(division4New, 30);
    _NS(division5New, 31);
    _NS(division6New, 32);
    _NS(division7New, 33);
    _NS(walletDivision1New, 34);
    _NS(walletDivision2New, 35);
    _NS(walletDivision3New, 36);
    _NS(walletDivision4New, 37);
    _NS(walletDivision5New, 38);
    _NS(walletDivision6New, 39);
    _NS(walletDivision7New, 40);
    cc.deletedNew = row.GetUInt(41);
    cc.isRecruitingNew = row.GetUInt(42);

    if (IsPlayerCorp(oldCorpID))
        table = "crpWalletDivisons";
    if (IsNPCCorp(oldCorpID))
        table = "crpNPCWalletDivisons";

    if (!sDatabase.RunQuery(res,
        " SELECT "
        "  c.corporationID, c.corporationName, c.description, c.tickerName, c.allianceID, c.warFactionID, c.url, c.taxRate,"
        "  c.minimumJoinStanding, c.corporationType, c.hasPlayerPersonnelManager, c.sendCharTerminationMessage, c.creatorID, c.ceoID, c.stationID, c.raceID, "
        "  c.shares, c.memberCount, c.memberLimit, c.allowedMemberRaceIDs, c.shape1, c.shape2, c.shape3, c.color1, c.color2, c.color3,"
        "  c.typeface, w.division1, w.division2, w.division3, w.division4, w.division5, w.division6, w.division7, w.walletDivision1, w.walletDivision2,"
        "  w.walletDivision3, w.walletDivision4, w.walletDivision5, w.walletDivision6, w.walletDivision7, c.deleted, c.isRecruiting "
        " FROM crpCorporation AS c"
        "  LEFT JOIN %s AS w USING (corporationID)"
        " WHERE corporationID = %u ", table.c_str(), oldCorpID
        ))
    {
        codelog(CORP__DB_ERROR, "Error in retrieving old corporation's data (%u)", oldCorpID);
        return false;
    }

    if (!res.GetRow(row)) {
        codelog(CORP__DB_WARNING, "Unable to find old corporation's data (%u)", oldCorpID);
        return false;
    }

    cc.corporationIDOld = new PyInt(row.GetInt(0));
    cc.corporationNameOld = new PyString(row.GetText(1));
    cc.descriptionOld = new PyString(row.GetText(2));
    cc.tickerNameOld = new PyString(row.GetText(3));
    _NI(allianceIDOld, 4);
    _NI(warFactionIDOld, 5);
    cc.urlOld = new PyString(row.GetText(6));
    cc.taxRateOld = new PyFloat(row.GetDouble(7));
    cc.minimumJoinStandingOld = new PyFloat(row.GetDouble(8));
    cc.corporationTypeOld = new PyInt(row.GetInt(9));
    cc.hasPlayerPersonnelManagerOld = new PyInt(row.GetInt(10));
    cc.sendCharTerminationMessageOld = new PyInt(row.GetInt(11));
    cc.creatorIDOld = new PyInt(row.GetInt(12));
    cc.ceoIDOld = new PyInt(row.GetInt(13));
    cc.stationIDOld = new PyInt(row.GetInt(14));
    _NI(raceIDOld, 15);
    cc.sharesOld = new PyLong(row.GetInt64(16));
    cc.memberCountOld = new PyInt(row.GetInt(17));
    cc.memberLimitOld = new PyInt(row.GetInt(18));
    cc.allowedMemberRaceIDsOld = new PyInt(row.GetInt(19));
    _NI(shape1Old, 20);
    _NI(shape2Old, 21);
    _NI(shape3Old, 22);
    _NI(color1Old, 23);
    _NI(color2Old, 24);
    _NI(color3Old, 25);
    _NI(typefaceOld, 26);
    _NS(division1Old, 27);
    _NS(division2Old, 28);
    _NS(division3Old, 29);
    _NS(division4Old, 30);
    _NS(division5Old, 31);
    _NS(division6Old, 32);
    _NS(division7Old, 33);
    _NS(walletDivision1Old, 34);
    _NS(walletDivision2Old, 35);
    _NS(walletDivision3Old, 36);
    _NS(walletDivision4Old, 37);
    _NS(walletDivision5Old, 38);
    _NS(walletDivision6Old, 39);
    _NS(walletDivision7Old, 40);
    cc.deletedOld = new PyInt(row.GetUInt(41));
    cc.isRecruitingOld = new PyInt(row.GetUInt(42));

    return true;
}

bool CorporationDB::JoinCorporation(uint32 charID, uint32 corpID, uint32 oldCorpID, const CorpData &data) {
    DBerror err;
    // Decrease previous corp's member count
    if (!sDatabase.RunQuery(err,
        "UPDATE crpCorporation "
        "   SET memberCount = memberCount-1"
        "   WHERE corporationID = %u",
            oldCorpID
        ))
    {
        codelog(CORP__DB_ERROR, "Error in prev corp member decrease query: %s", err.c_str());
        return false;
    }

    if (!sDatabase.RunQuery(err,
        "UPDATE chrCharacters SET "
        "   corporationID = %u, startDateTime = %f, corpAccountKey = %i,"
        "   corpRole = %lli, rolesAtAll = %lli, rolesAtBase = %lli, rolesAtHQ = %lli, rolesAtOther = %lli, "
        "   grantableRoles = %lli, grantableRolesAtBase = %lli, grantableRolesAtHQ = %lli, grantableRolesAtOther = %lli "
        "   WHERE characterID = %u",
            corpID, GetFileTimeNow(), data.corpAccountKey,
            data.corpRole, data.rolesAtAll, data.rolesAtBase, data.rolesAtHQ, data.rolesAtOther,
            data.grantableRoles, data.grantableRolesAtBase, data.grantableRolesAtHQ, data.grantableRolesAtOther,
            charID
        ))
    {
        codelog(CORP__DB_ERROR, "Error in char update query: %s", err.c_str());
        //TODO: undo previous member count decrement.
        return false;
    }

    // Increase new corp's member number...
    if (!sDatabase.RunQuery(err,
        "UPDATE crpCorporation "
        "   SET memberCount = memberCount+1"
        "   WHERE corporationID = %u",
            corpID
        ))
    {
        codelog(CORP__DB_ERROR, "Error in new corp member decrease query: %s", err.c_str());
        //dont stop now, we are already moved... else we need to undo everything we just did.
    }

    /** @todo  check if this member has any shares, and update to new corpID */
    return true;
}

bool CorporationDB::CreateCorporationCreatePacket(OnCorporationChanged & cc, uint32 oldCorpID, uint32 newCorpID) {
    DBQueryResult res;
    DBResultRow row;
    if (!sDatabase.RunQuery(res,
        " SELECT "
        "  c.corporationID, c.corporationName, c.description, c.tickerName, c.allianceID, c.warFactionID, c.url, c.taxRate,"
        "  c.minimumJoinStanding, c.corporationType, c.hasPlayerPersonnelManager, c.sendCharTerminationMessage, c.creatorID, c.ceoID, c.stationID, c.raceID, "
        "  c.shares, c.memberCount, c.memberLimit, c.allowedMemberRaceIDs, c.shape1, c.shape2, c.shape3, c.color1, c.color2,c. color3,"
        "  c.typeface, w.division1, w.division2, w.division3, w.division4, w.division5, w.division6, w.division7, w.walletDivision1, w.walletDivision2,"
        "  w.walletDivision3, w.walletDivision4, w.walletDivision5, w.walletDivision6, w.walletDivision7, c.deleted, c.isRecruiting "
        " FROM crpCorporation AS c"
        "  LEFT JOIN crpWalletDivisons AS w USING (corporationID)"
        " WHERE corporationID = %u ", newCorpID
        ))
    {
        codelog(CORP__DB_ERROR, "Error in retrieving new corporation's data (%u)", newCorpID);
        return false;
    }

    if (!res.GetRow(row)) {
        codelog(CORP__DB_WARNING, "Unable to find corporation's data (%u)", newCorpID);
        return false;
    }

    cc.corporationIDNew = row.GetUInt(0);
    cc.corporationNameNew = row.GetText(1);
    cc.descriptionNew = row.GetText(2);
    cc.tickerNameNew = row.GetText(3);
    _NI(allianceIDNew, 4);
    _NI(warFactionIDNew, 5);
    cc.urlNew = row.GetText(6);
    cc.taxRateNew = row.GetDouble(7);
    cc.minimumJoinStandingNew = row.GetDouble(8);
    cc.corporationTypeNew = row.GetUInt(9);
    cc.hasPlayerPersonnelManagerNew = row.GetUInt(10);
    cc.sendCharTerminationMessageNew = row.GetUInt(11);
    cc.creatorIDNew = row.GetUInt(12);
    cc.ceoIDNew = row.GetUInt(13);
    cc.stationIDNew = row.GetUInt(14);
    _NI(raceIDNew, 15);
    cc.sharesNew = row.GetInt64(16);
    cc.memberCountNew = row.GetUInt(17);
    cc.memberLimitNew = row.GetUInt(18);
    cc.allowedMemberRaceIDsNew = row.GetUInt(19);
    _NI(shape1New, 20);
    _NI(shape2New, 21);
    _NI(shape3New, 22);
    _NI(color1New, 23);
    _NI(color2New, 24);
    _NI(color3New, 25);
    _NI(typefaceNew, 26);
    _NS(division1New, 27);
    _NS(division2New, 28);
    _NS(division3New, 29);
    _NS(division4New, 30);
    _NS(division5New, 31);
    _NS(division6New, 32);
    _NS(division7New, 33);
    _NS(walletDivision1New, 34);
    _NS(walletDivision2New, 35);
    _NS(walletDivision3New, 36);
    _NS(walletDivision4New, 37);
    _NS(walletDivision5New, 38);
    _NS(walletDivision6New, 39);
    _NS(walletDivision7New, 40);
    cc.deletedNew = row.GetUInt(41);
    cc.isRecruitingNew = row.GetUInt(42);

    cc.allianceIDOld = PyStatic.NewNone();
    cc.warFactionIDOld = PyStatic.NewNone();
    cc.allowedMemberRaceIDsOld = PyStatic.NewNone();
    cc.ceoIDOld = PyStatic.NewNone();
    cc.color1Old = PyStatic.NewNone();
    cc.color2Old = PyStatic.NewNone();
    cc.color3Old = PyStatic.NewNone();
    cc.corporationIDOld = PyStatic.NewNone();
    cc.corporationNameOld = PyStatic.NewNone();
    cc.corporationTypeOld = PyStatic.NewNone();
    cc.creatorIDOld = PyStatic.NewNone();
    cc.deletedOld = PyStatic.NewNone();
    cc.descriptionOld = PyStatic.NewNone();
    cc.division1Old = PyStatic.NewNone();
    cc.division2Old = PyStatic.NewNone();
    cc.division3Old = PyStatic.NewNone();
    cc.division4Old = PyStatic.NewNone();
    cc.division5Old = PyStatic.NewNone();
    cc.division6Old = PyStatic.NewNone();
    cc.division7Old = PyStatic.NewNone();
    cc.walletDivision1Old = PyStatic.NewNone();
    cc.walletDivision2Old = PyStatic.NewNone();
    cc.walletDivision3Old = PyStatic.NewNone();
    cc.walletDivision4Old = PyStatic.NewNone();
    cc.walletDivision5Old = PyStatic.NewNone();
    cc.walletDivision6Old = PyStatic.NewNone();
    cc.walletDivision7Old = PyStatic.NewNone();
    cc.hasPlayerPersonnelManagerOld = PyStatic.NewNone();
    cc.memberCountOld = PyStatic.NewNone();
    cc.memberLimitOld = PyStatic.NewNone();
    cc.minimumJoinStandingOld = PyStatic.NewNone();
    cc.raceIDOld = PyStatic.NewNone();
    cc.sendCharTerminationMessageOld = PyStatic.NewNone();
    cc.shape1Old = PyStatic.NewNone();
    cc.shape2Old = PyStatic.NewNone();
    cc.shape3Old = PyStatic.NewNone();
    cc.sharesOld = PyStatic.NewNone();
    cc.stationIDOld = PyStatic.NewNone();
    cc.taxRateOld = PyStatic.NewNone();
    cc.tickerNameOld = PyStatic.NewNone();
    cc.typefaceOld = PyStatic.NewNone();
    cc.urlOld = PyStatic.NewNone();
    cc.isRecruitingOld = PyStatic.NewNone();

    return true;
}

PyRep* CorporationDB::GetMember(uint32 charID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT "
        "  characterID, "
        "  corporationID,"
        "  0 AS divisionID,"
        "  0 AS squadronID,"
        "  title, "
        "  corpRole AS roles,"
        "  rolesAtAll, "
        "  grantableRoles, "
        "  startDateTime, "
        "  baseID,"
        "  rolesAtHQ, "
        "  grantableRolesAtHQ, "
        "  rolesAtBase, "
        "  grantableRolesAtBase,"
        "  rolesAtOther, "
        "  grantableRolesAtOther, "
        "  titleMask,"
        "  corpAccountKey, "
        "  startDateTime AS rowDate,"
        "  blockRoles,"
        "  name"
        " FROM chrCharacters"
        " WHERE characterID = %u", charID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    DBResultRow row;
    if (!res.GetRow(row))
        return nullptr;
    return DBRowToPackedRow(row);
}

void CorporationDB::GetMembers(uint32 corpID, DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        " SELECT "
        "  characterID, "
        "  corporationID,"
        //"  0 AS divisionID,"
        //"  0 AS squadronID,"
        "  title, "
        "  rolesAtAll, "
        "  grantableRoles, "
        "  startDateTime, "
        "  baseID,"
        "  rolesAtHQ, "
        "  grantableRolesAtHQ, "
        "  rolesAtBase, "
        "  grantableRolesAtBase,"
        "  rolesAtOther, "
        "  grantableRolesAtOther, "
        "  titleMask,"
        "  corpAccountKey,"
        //"  %f AS rowDate,"
        "  blockRoles,"
        "  name"
        " FROM chrCharacters"
        " WHERE corporationID = %u", corpID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
    }
}

void CorporationDB::GetMembersForQuery(uint32 corpID, DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        " SELECT "
        "  characterID, "
        "  rolesAtAll, "
        "  grantableRoles, "
        "  rolesAtHQ, "
        "  grantableRolesAtHQ, "
        "  rolesAtBase, "
        "  grantableRolesAtBase,"
        "  rolesAtOther, "
        "  grantableRolesAtOther, "
        "  titleMask,"
        "  blockRoles,"
        "  name"
        " FROM chrCharacters"
        " WHERE corporationID = %u", corpID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
    }
}

void CorporationDB::GetMembersPaged(uint32 corpID, uint8 page, DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        " SELECT "
        "  characterID, "
        "  corporationID,"
        //"  0 AS divisionID,"
        //"  0 AS squadronID,"
        "  title, "
        "  rolesAtAll, "
        "  grantableRoles, "
        "  startDateTime, "
        //"  clone.locationID,"
        "  rolesAtHQ, "
        "  grantableRolesAtHQ, "
        "  rolesAtBase, "
        "  grantableRolesAtBase,"
        "  rolesAtOther, "
        "  grantableRolesAtOther, "
        "  titleMask,"
        "  corpAccountKey, "
        //"  %f AS rowDate,"
        "  blockRoles,"
        "  name"
        " FROM chrCharacters"
        //"  LEFT JOIN entity AS clone ON clone.ownerID = characterID"
        " WHERE corporationID = %u"
        " LIMIT %u", corpID, page * 10))
        //"  AND clone.flag='400'"
        //"  AND clone.customInfo='active'"
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
    }
}

uint16 CorporationDB::GetMemberCount(uint32 corpID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT COUNT(characterID) FROM chrCharacters WHERE corporationID = %u", corpID )) {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (res.GetRow(row))
        return (row.GetInt(0));

    return 0;
}

void CorporationDB::GetMemberIDs(uint32 corpID, std::vector< uint32 >& ids, bool online/*true*/)
{
    ids.clear();
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT characterID FROM chrCharacters WHERE corporationID = %u AND online = %u", corpID, (online?1:0) )) {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return;
    }

    DBResultRow row;
    while (res.GetRow(row))
        ids.push_back(row.GetInt(0));
}

PyRep* CorporationDB::GetCorpRoles()
{
    DBQueryResult res;
    if (!sDatabase.RunQuery( res, "SELECT roleID, roleName, shortDescriptionID, descriptionID, roleIID FROM crpRoles")) {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

PyRep* CorporationDB::GetCorpRoleGroups()
{
    DBQueryResult res;
    if (!sDatabase.RunQuery( res,
        "SELECT roleGroupID, roleGroupName, roleGroupNameID, roleMask, appliesTo, appliesToGrantable, isLocational, isDivisional"
        " FROM crpRoleGroups"))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

void CorporationDB::DeleteTitle(uint32 corpID, uint16 titleID)
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM crpRoleTitles WHERE corpID = %u AND titleID = %u", corpID, titleID);
    std::string title = "Untitled ";
    title += itoa(log2(titleID)+1);
    sDatabase.RunQuery(err, "INSERT INTO crpRoleTitles (corporationID, titleID, titleName) VALUES (%u,%u,'%s')", corpID, titleID, title.c_str());
}

bool CorporationDB::UpdateTitle(uint32 corpID, Call_UpdateTitleData& args, PyDict* updates)
{
    if (IsNPCCorp(corpID))
        return false;

    DBQueryResult res;
    if (!sDatabase.RunQuery( res,
        "SELECT titleName, roles, grantableRoles, rolesAtHQ, grantableRolesAtHQ, rolesAtBase, grantableRolesAtBase, rolesAtOther, grantableRolesAtOther "
        " FROM crpRoleTitles WHERE corporationID = %u AND titleID = %u", corpID, args.titleID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(CORP__DB_WARNING, "No title data for titleID %u in corpID %u.", args.titleID, corpID);
        return false;
    }

    std::vector<std::string> dbQ;
    ProcessStringChange("titleName",            row.GetText(0), args.titleName,             updates,   dbQ);
    ProcessLongChange("roles",                 row.GetInt64(1), args.roles,                 updates,   dbQ);
    ProcessLongChange("grantableRoles",        row.GetInt64(2), args.grantableRoles,        updates,   dbQ);
    ProcessLongChange("rolesAtHQ",             row.GetInt64(3), args.rolesAtHQ,             updates,   dbQ);
    ProcessLongChange("grantableRolesAtHQ",    row.GetInt64(4), args.grantableRolesAtHQ,    updates,   dbQ);
    ProcessLongChange("rolesAtBase",           row.GetInt64(5), args.rolesAtBase,           updates,   dbQ);
    ProcessLongChange("grantableRolesAtBase",  row.GetInt64(6), args.grantableRolesAtBase,  updates,   dbQ);
    ProcessLongChange("rolesAtOther",          row.GetInt64(7), args.rolesAtOther,          updates,   dbQ);
    ProcessLongChange("grantableRolesAtOther", row.GetInt64(8), args.grantableRolesAtOther, updates,   dbQ);

    std::string query = "UPDATE crpRoleTitles SET ";

    int N = dbQ.size();
    for (int i = 0; i < N; ++i) {
        query += dbQ[i];
        if (i < N - 1)
            query += ", ";
    }

    query += " WHERE corporationID = ";
    query += itoa(corpID);
    query += " AND titleID = ";
    query += itoa(args.titleID);

    _log(CORP__DB_MESSAGE, "DB Query: %s", query.c_str());

    DBerror err;
    // We are here, so something must have changed...
    if (N < 1)
    return false;

    if (!sDatabase.RunQuery(err, query.c_str())) {
        codelog(CORP__DB_ERROR, "Error in query: %s", err.c_str());
        return false;
    }
    return true;
}

PyRep* CorporationDB::GetTitles(uint32 corpID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery( res,
        "SELECT corporationID, titleID, titleName, roles, grantableRoles, rolesAtHQ, grantableRolesAtHQ, rolesAtBase, grantableRolesAtBase, rolesAtOther, grantableRolesAtOther "
        " FROM crpRoleTitles WHERE corporationID = %u", corpID)) {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
    PyDict* dict = new PyDict();
    DBResultRow row;
    while (res.GetRow(row)) {
        PyDict* dict1 = new PyDict();
            dict1->SetItemString("corporationID", new PyInt(row.GetInt(0)));
            dict1->SetItemString("titleID", new PyInt(row.GetInt(1)));
            dict1->SetItemString("titleName", new PyString(row.GetText(2)));
            dict1->SetItemString("roles", new PyLong(row.GetInt64(3)));
            dict1->SetItemString("grantableRoles", new PyLong(row.GetInt64(4)));
            dict1->SetItemString("rolesAtHQ", new PyLong(row.GetInt64(5)));
            dict1->SetItemString("grantableRolesAtHQ", new PyLong(row.GetInt64(6)));
            dict1->SetItemString("rolesAtBase", new PyLong(row.GetInt64(7)));
            dict1->SetItemString("grantableRolesAtBase", new PyLong(row.GetInt64(8)));
            dict1->SetItemString("rolesAtOther", new PyLong(row.GetInt64(9)));
            dict1->SetItemString("grantableRolesAtOther", new PyLong(row.GetInt64(10)));
        dict->SetItem(new PyInt(row.GetInt(1)), new PyObject("util.KeyVal", dict1));
    }

    return dict;
}

void CorporationDB::CreateTitleData(uint32 corpID)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "INSERT INTO crpRoleTitles (corporationID, titleID, titleName) VALUES"
        "(%u,1,'Untitled 1'),(%u,2,'Untitled 2'),(%u,4,'Untitled 3'),(%u,8,'Untitled 4'),(%u,16,'Untitled 5'),(%u,32,'Untitled 6'),"
        "(%u,64,'Untitled 7'),(%u,128,'Untitled 8'),(%u,256,'Untitled 9'),(%u,512,'Untitled 10'),(%u,1024,'Untitled 11'),(%u,2048,'Untitled 12'),"
        "(%u,4096,'Untitled 13'),(%u,8192,'Untitled 14'),(%u,16384,'Untitled 15'),(%u,32768,'Untitled 16')",
        corpID,corpID,corpID,corpID,corpID,corpID,corpID,corpID,corpID,corpID,corpID,corpID,corpID,corpID,corpID,corpID);
}

/* * UPDATE crpRoleTitles SET corporationID=[value-1],titleID=[value-2],titleName=[value-3],roles=[value-4],grantableRoles=[value-5],rolesAtHQ=[value-6],grantableRolesAtHQ=[value-7],rolesAtBase=[value-8],grantableRolesAtBase=[value-9],rolesAtOther=[value-10],grantableRolesAtOther=[value-11] WHERE corporationID
 */

PyRep* CorporationDB::GetContacts(uint32 corpID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery( res,
        "SELECT contactID, inWatchlist, relationshipID, labelMask"
        " FROM crpContacts WHERE ownerID = %u", corpID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    PyObject* obj = DBResultToIndexRowset(res, "contactID");
    if (is_log_enabled(CORP__RSP_DUMP))
        obj->Dump(CORP__RSP_DUMP, "");

    return obj;
}

void CorporationDB::AddContact(uint32 corpID)
{

}

void CorporationDB::UpdateContact(uint32 corpID)
{

}

// should this be cached?     ...yes
PyObject *CorporationDB::GetEveOwners(uint32 corpID) {
    DBQueryResult res;
/*
            [PyPackedRow 9 bytes]
              ["ownerID" => <90046865> [I4]]
              ["ownerName" => <Septure> [WStr]]
              ["typeID" => <1375> [I4]]
              ["gender" => <1> [Bool]]
              */

    if (!sDatabase.RunQuery( res,
        "SELECT"
        " characterID AS ownerID,"
        " name AS ownerName,"
        " typeID,"
        " gender"
        " FROM chrCharacters"
        " WHERE corporationID = %u", corpID
        ))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

PyObject *CorporationDB::GetStations(uint32 corpID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        " SELECT "
        " stationID, stationTypeID as typeID "
        " FROM staStations "
        " WHERE corporationID = %u ", corpID
        ))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToRowset(res);
}

PyRep *CorporationDB::Fetch(uint32 corpID, uint32 from, uint32 count) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT itemID, officeFolderID, corporationID, stationID, typeID, lockDown, rentalFee, expiryDateTime"
        " FROM staOffices "
        " WHERE corporationID = %u "
        " LIMIT %u, %u ", corpID, from, count
        ))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    // Have to send back a list that contains a tuple that contains an int and a list...
    // params probably needs the following stuff: stationID, typeID, officeID, officeFolderID
    Reply_FetchOffice reply;
    reply.params = new PyList();

    DBResultRow row;
    if (res.GetRow(row)) {
        reply.params->AddItemInt( row.GetInt(0) );
        reply.params->AddItemInt( row.GetInt(1) );
        reply.officeID = row.GetInt(2);
        reply.params->AddItemInt( reply.officeID );
        reply.params->AddItemInt( row.GetInt(3) );
    }
    return reply.Encode();
}

//NOTE: it makes sense to push this up to ServiceDB, since others will likely need this too.
uint32 CorporationDB::GetStationOwner(uint32 stationID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT corporationID "
        " FROM staStations "
        " WHERE stationID = %u ", stationID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(CORP__DB_WARNING, "Missing stationID: %u", stationID);
        return 0;
    }
    return row.GetUInt(0);
}

void CorporationDB::AddBulletin(uint32 corpID, uint32 ownerID, uint32 cCharID, std::string& title, std::string& body)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "INSERT INTO crpBulletins (corporationID, ownerID, createCharacterID, createDateTime, editCharacterID, editDateTime, title, body)"
        " VALUES (%u, %u, %u, %f, %u, %f, '%s', '%s')", corpID, ownerID, cCharID, GetFileTimeNow(), cCharID, GetFileTimeNow(), title.c_str(), body.c_str());
}

void CorporationDB::EditBulletin(uint32 bulletinID, uint32 eCharID, int64 eDataTime, std::string& title, std::string& body)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "UPDATE crpBulletins SET editCharacterID = %u, editDateTime = %lli, title = '%s', body = '%s'"
        " WHERE bulletinID = %u", eCharID, eDataTime, title.c_str(), body.c_str(), bulletinID);
}

PyRep* CorporationDB::GetBulletins(uint32 corpID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT corporationID, bulletinID, ownerID, createCharacterID, createDateTime, editCharacterID, editDateTime, title, body"
        " FROM crpBulletins"
        " WHERE corporationID = %u ", corpID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}

void CorporationDB::AddRecruiters(int32 adID, int32 corpID, std::vector< int32 >& charVec)
{
    std::ostringstream str;
    str << "INSERT INTO crpRecruiters (adID, corpID, charID) VALUES ";

    bool first = true;
    for (auto cur : charVec) {
        if (!IsCharacter(cur))
            continue;
        if (first)
            first = false;
        else
            str << ",";
        str << "(" << itoa(adID) << "," << itoa(corpID) << "," << itoa(cur) << ")";
    }

    if (!first ) {
        DBerror err;
        sDatabase.RunQuery(err, "DELETE FROM crpRecruiters WHERE adID = %u", adID);
        sDatabase.RunQuery(err, str.str().c_str());
    }
}

PyRep* CorporationDB::GetRecruiters(uint16 adID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT charID FROM crpRecruiters WHERE adID = %u", adID)) {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}

PyRep* CorporationDB::GetAdTypeData()
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT typeMask, typeName, typeNameID, groupID, description, descriptionID FROM crpAdTypeData")) {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}

PyRep* CorporationDB::GetAdGroupData()
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT groupID, groupName, groupNameID, description, descriptionID FROM crpAdGroupData")) {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}

PyRep* CorporationDB::GetAdRegistryData(int64 typeMask/*0*/, bool inAlliance/*false*/, int16 minMembers/*0*/, uint16 maxMembers/*12602*/)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  adID, corporationID, allianceID, stationID, regionID, raceMask, typeMask,"
        "  createDateTime, expiryDateTime, title, description, memberCount, channelID"
        " FROM crpAdRegistry"
        "  WHERE allianceID %s 0"
        "   AND (memberCount >= %u AND memberCount < %u)"
        " AND typeMask & %llu = %llu",
        (inAlliance ? ">" : "="), minMembers, maxMembers, typeMask, typeMask))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}

int32 CorporationDB::CreateAdvert(Client* pClient, uint32 corpID, int64 typeMask, int8 days, uint16 members, std::string description, uint32 channelID, std::string title)
{
    uint32 adID = 0;
    DBerror err;
    sDatabase.RunQueryLID(err, adID, "INSERT INTO crpAdRegistry"
    " (corporationID, allianceID, stationID, regionID, raceMask, typeMask,"
    "  createDateTime, expiryDateTime, description, title, memberCount, channelID)"
    " VALUES (%u,%u,%u,%u,%u,%lli,%f,%f,'%s','%s',%u,%u)",
        corpID, pClient->GetAllianceID(), pClient->GetStationID(), pClient->GetRegionID(), 15, typeMask, // raceMask isnt implemented yet
        GetFileTimeNow(), (GetFileTimeNow() + (EvE::Time::Day * days)), description.c_str(), title.c_str(), members, channelID);

    return (int32)adID;
}

PyRep *CorporationDB::GetMyApplications(uint32 charID) {
    //    header = [applicationID, corporationID, characterID, applicationText, roles, grantableRoles, status, applicationDateTime, deleted, lastCorpUpdaterID]
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT applicationID, corporationID, characterID, applicationText, roles, grantableRoles,"
        " status, applicationDateTime, deleted, lastCorpUpdaterID"
        " FROM crpApplications"
        " WHERE characterID = %u ", charID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    PyObjectEx* obj = DBResultToCRowset(res);
    if (is_log_enabled(CORP__RSP_DUMP))
        obj->Dump(CORP__RSP_DUMP, "");

    return obj;
}

PyRep *CorporationDB::GetApplications(uint32 corpID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT"
        " applicationID, corporationID, characterID, applicationText, roles, grantableRoles, status,"
        " applicationDateTime, deleted, lastCorpUpdaterID"
        " FROM crpApplications"
        " WHERE corporationID = %u", corpID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
    PyObjectEx* obj = DBResultToCIndexedRowset(res, "characterID");
    if (is_log_enabled(CORP__RSP_DUMP))
        obj->Dump(CORP__RSP_DUMP, "");

    return obj;
}

bool CorporationDB::GetCurrentApplicationInfo(uint32 charID, uint32 corpID, ApplicationInfo & aInfo) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT"
        " applicationID, applicationText, roles, grantableRoles, status,"
        " applicationDateTime, lastCorpUpdaterID, deleted"
        " FROM crpApplications"
        " WHERE characterID = %u AND corporationID = %u",
        charID, corpID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return (aInfo.valid = false);
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(CORP__DB_WARNING, "There's no previous application.");
        return (aInfo.valid = false);
    }

    aInfo.appID = row.GetInt(0);
    aInfo.charID = charID;
    aInfo.corpID = corpID;
    aInfo.appText = row.GetText(1);
    aInfo.role = row.GetInt64(2);
    aInfo.grantRole = row.GetInt64(3);
    aInfo.status = row.GetInt(4);
    aInfo.appTime = row.GetInt64(5);
    aInfo.lastCID = row.GetInt(6);
    aInfo.deleted = row.GetInt(7);
    return (aInfo.valid = true);
}

bool CorporationDB::InsertApplication(ApplicationInfo& aInfo) {
    if (!aInfo.valid) {
        _log(CORP__DB_WARNING, "InsertApplication(): aInfo contains invalid data");
        return false;
    }

    std::string safeMessage;
    sDatabase.DoEscapeString(safeMessage, aInfo.appText);
    DBerror err;
    if (!sDatabase.RunQueryLID(err, aInfo.appID,
        " INSERT INTO crpApplications"
        " (corporationID, characterID, applicationText, roles, grantableRoles, status,"
        " applicationDateTime, deleted, lastCorpUpdaterID)"
        " VALUES"
        " (%u, %u, '%s', %llu, %llu, %u, %llu, %u, %u)",
            aInfo.corpID, aInfo.charID, safeMessage.c_str(), aInfo.role,
            aInfo.grantRole, aInfo.status, aInfo.appTime, aInfo.deleted, aInfo.lastCID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", err.c_str());
        return false;
    }

    return true;
}

bool CorporationDB::UpdateApplication(const ApplicationInfo& aInfo) {
    if (!aInfo.valid) {
        _log(CORP__DB_WARNING, "UpdateApplication(): info contains invalid data");
        return false;
    }

    DBerror err;
    std::string clear;
    sDatabase.DoEscapeString(clear, aInfo.appText);
    if (!sDatabase.RunQuery(err,
        " UPDATE crpApplications"
        " SET status = %u, lastCorpUpdaterID = %u, applicationText = '%s'"
        " WHERE applicationID = %u", aInfo.status, aInfo.lastCID, clear.c_str(), aInfo.appID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", err.c_str());
        return false;
    }
    return true;
}

bool CorporationDB::DeleteApplication(const ApplicationInfo& aInfo) {
    DBerror err;
    if (!sDatabase.RunQuery(err,
        " DELETE FROM crpApplications"
        " WHERE applicationID = %u", aInfo.corpID, aInfo.appID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", err.c_str());
        return false;
    }
    return true;
}

uint32 CorporationDB::GetStationCorporationCEO(uint32 stationID) {
    if (!IsStation(stationID))
        return 0;
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT c.ceoID "
        " FROM crpCorporation AS c"
        " LEFT JOIN staStations USING (corporationID) "
        " WHERE staStations.stationID = %u ", stationID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }
    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(CORP__DB_WARNING, "There's either no such station or the station has no corp owner or the corporation has no ceo.");
        return 0;
    }
    return row.GetUInt(0);
}

uint32 CorporationDB::GetCorporationCEO(uint32 corpID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT ceoID "
        " FROM crpCorporation "
        " WHERE corporationID = %u ", corpID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }
    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(CORP__DB_WARNING, "There's either no such corp owner or the corporation has no ceo.");
        return 0;
    }
    return row.GetUInt(0);
}

uint16 CorporationDB::GetCorpMemberCount(uint32 corpID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT memberCount"
        " FROM crpCorporation"
        " WHERE corporationID = %u ", corpID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }
    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(CORP__DB_WARNING, "CAnnot find memberCount for corpID %u", corpID);
        return 0;
    }
    return row.GetInt(0);
}

double CorporationDB::GetCloneTypeCostByID(uint32 cloneTypeID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT basePrice "
        " FROM invTypes "
        " WHERE typeID = %u ", cloneTypeID))
    {
        codelog(CORP__DB_ERROR, "Failed to retrieve basePrice of typeID = %u",cloneTypeID);
    }
    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(CORP__DB_WARNING, "GetCloneTypeCostByID returned no results");
        return 0;
    }
    return row.GetDouble(0);
}

bool CorporationDB::CreateMemberAttributeUpdate(uint32 newCorpID, uint32 charID, MemberAttributeUpdate& attrib) {
    // What are we doing here exactly?
    // Corporation gets a new member
    // it's new to it

    DBQueryResult res;
    DBResultRow row;
    if (!sDatabase.RunQuery(res,
        " SELECT "
        "   title, startDateTime, corporationID, "
        "   corpRole, rolesAtAll, rolesAtBase, "
        "   rolesAtHQ, rolesAtOther, titleMask, baseID "
        " FROM chrCharacters "
        " WHERE characterID = %u ", charID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    if (!res.GetRow(row)) {
        codelog(CORP__DB_WARNING, "Cannot find character in database");
        return false;
    }

    // this could be stored in the db
#define PRN PyStatic.NewNone()
#define PRI(i) new PyInt(i)
#define PRL(i) new PyLong(i)
#define PRS(s) new PyString(s)
#define PRNI(i) (row.IsNull(i) ? PRU(0) : PRU(row.GetInt64(i)))
#define F(name, o, n) \
    attrib.name##Old = o; \
    attrib.name##New = n

    //element                   Old Value               New Value
    F(accountKey,               PRN,                    PRN);
    F(baseID,                   PRI(row.GetInt(9)),     PRI(0));
    F(characterID,              PRN,                    PRI(charID));
    F(corporationID,            PRI(row.GetUInt(2)),    PRI(newCorpID));
    F(divisionID,               PRN,                    PRN);
    F(roles,                    PRL(row.GetInt64(3)),   PRI(0));
    F(grantableRoles,           PRL(row.GetInt64(4)),   PRI(0));
    F(grantableRolesAtBase,     PRL(row.GetInt64(5)),   PRI(0));
    F(grantableRolesAtHQ,       PRL(row.GetInt64(6)),   PRI(0));
    F(grantableRolesAtOther,    PRL(row.GetInt64(7)),   PRI(0));
    F(squadronID,               PRN,                    PRN);
    F(startDateTime,            PRL(row.GetInt64(1)),   PRL(GetFileTimeNow()));
    F(titleMask,                PRL(row.GetInt64(8)),   PRI(0));

#undef F
#undef PRN
#undef PRI
#undef PRS
#undef PRL
#undef PRNI

    return true;
}

bool CorporationDB::UpdateDivisionNames(uint32 corpID, const Call_UpdateDivisionNames& divs, PyDict* notif) {
    //  updated to update wallet names, too.   -allan 14Jan15
    if (IsNPCCorp(corpID))
        return false; // make error here for changing npc corp data?  nah, should NEVER hit
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "  division1, division2, division3, division4, division5, division6, division7,"
        "  walletDivision2, walletDivision3, walletDivision4, walletDivision5, walletDivision6, walletDivision7"
        " FROM crpWalletDivisons"
        " WHERE corporationID = %u", corpID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(CORP__DB_WARNING, "Corporation %u doesn't exist.", corpID);
        return false;
    }

    std::vector<std::string> dbQ;
    ProcessStringChange("division1", row.GetText(0), divs.div1, notif, dbQ);
    ProcessStringChange("division2", row.GetText(1), divs.div2, notif, dbQ);
    ProcessStringChange("division3", row.GetText(2), divs.div3, notif, dbQ);
    ProcessStringChange("division4", row.GetText(3), divs.div4, notif, dbQ);
    ProcessStringChange("division5", row.GetText(4), divs.div5, notif, dbQ);
    ProcessStringChange("division6", row.GetText(5), divs.div6, notif, dbQ);
    ProcessStringChange("division7", row.GetText(6), divs.div7, notif, dbQ);
    //ProcessStringChange("walletDivision1", row.GetText(7), divs.wallet1, notif, dbQ);  cannot change this one
    ProcessStringChange("walletDivision2", row.GetText(7), divs.wallet2, notif, dbQ);
    ProcessStringChange("walletDivision3", row.GetText(8), divs.wallet3, notif, dbQ);
    ProcessStringChange("walletDivision4", row.GetText(9), divs.wallet4, notif, dbQ);
    ProcessStringChange("walletDivision5", row.GetText(10), divs.wallet5, notif, dbQ);
    ProcessStringChange("walletDivision6", row.GetText(11), divs.wallet6, notif, dbQ);
    ProcessStringChange("walletDivision7", row.GetText(12), divs.wallet7, notif, dbQ);

    std::string query = "UPDATE crpWalletDivisons SET ";

    int N = dbQ.size();
    for (int i = 0; i < N; ++i) {
        query += dbQ[i];
        if (i < N - 1)
            query += ", ";
    }

    query += " WHERE corporationID = ";
    query += itoa(corpID);

    _log(CORP__DB_MESSAGE, "DB Query: %s", query.c_str());

    DBerror err;
    // We are here, so something must have changed...
    if (N > 0)
        if (!sDatabase.RunQuery(err, query.c_str())) {
            codelog(CORP__DB_ERROR, "Error in query: %s", err.c_str());
            return false;
        }

    return true;
}

std::string CorporationDB::GetDivisionName(uint32 corpID, uint16 acctKey)
{
    std::string acctKeyName = "";
    switch (acctKey) {
        case Account::KeyType::Cash:    acctKeyName = "division1"; break;
        case Account::KeyType::Cash2:   acctKeyName = "division2"; break;
        case Account::KeyType::Cash3:   acctKeyName = "division3"; break;
        case Account::KeyType::Cash4:   acctKeyName = "division4"; break;
        case Account::KeyType::Cash5:   acctKeyName = "division5"; break;
        case Account::KeyType::Cash6:   acctKeyName = "division6"; break;
        case Account::KeyType::Cash7:   acctKeyName = "division7"; break;
    }

    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT %s FROM crpWalletDivisons WHERE corporationID = %u", corpID)) {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return "Unknown";
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(CORP__DB_WARNING, "Corporation %u - division name for acctKey %u not found.", corpID, acctKey);
        return "Unknown";
    }

    return row.GetText(0);
}


bool CorporationDB::UpdateCorporation(uint32 corpID, const Call_UpdateCorporation & upd, PyDict * notif) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        " SELECT description, url, taxRate "
        " FROM crpCorporation "
        " WHERE corporationID = %u ", corpID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(CORP__DB_WARNING, "Corporation %u doesn't exists.", corpID);
        return false;
    }

    std::vector<std::string> dbQ;
    ProcessStringChange("description", row.GetText(0), upd.description, notif, dbQ);
    ProcessStringChange("url", row.GetText(1), upd.address, notif, dbQ);
    ProcessRealChange("taxRate", row.GetDouble(2), upd.tax, notif, dbQ);

    std::string query = " UPDATE crpCorporation SET ";

    int N = dbQ.size();
    for (int i = 0; i < N; i++) {
        query += dbQ[i];
        if (i < N - 1) query += ", ";
    }

    query += " WHERE corporationID = %u";

    // only update if there is anything to update
    if ((N > 0) && (!sDatabase.RunQuery(res.error, query.c_str(), corpID))) {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    return true;

}
#define NI(i) row.IsNull(i) ? 0 : row.GetInt(i)
bool CorporationDB::UpdateLogo(uint32 corpID, const Call_UpdateLogo & upd, PyDict * notif) {
    DBQueryResult res;

    /** @todo  update this  */
    if (!sDatabase.RunQuery(res,
        " SELECT shape1, shape2, shape3, color1, color2, color3, typeface "
        " FROM crpCorporation "
        " WHERE corporationID = %u ", corpID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        codelog(CORP__DB_WARNING, "Corporation %u doesn't exists.", corpID);
        return false;
    }

    std::vector<std::string> dbQ;
    ProcessIntChange("shape1", NI(0), upd.shape1, notif, dbQ);
    ProcessIntChange("shape2", NI(1), upd.shape2, notif, dbQ);
    ProcessIntChange("shape3", NI(2), upd.shape3, notif, dbQ);

    ProcessIntChange("color1", NI(3), upd.color1, notif, dbQ);
    ProcessIntChange("color2", NI(4), upd.color2, notif, dbQ);
    ProcessIntChange("color3", NI(5), upd.color3, notif, dbQ);

    std::string query = " UPDATE crpCorporation SET ";

    int N = dbQ.size();
    for (int i = 0; i < N; i++) {
        query += dbQ[i];
        if (i < N - 1) query += ", ";
    }

    query += " WHERE corporationID = %u ";
    if ((N > 0) && (!sDatabase.RunQuery(res.error, query.c_str(), corpID))) {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    return true;
}
#undef NI

PyRep* CorporationDB::GetMemberTrackingInfo(uint32 corpID)
{
    //  lastOnline needs update based on char logoffDateTime using GetElapsedHours();
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT c.characterID, c.corporationID, c.title, c.startDateTime, c.corpRole AS roles, c.baseID, c.grantableRoles, c.blockRoles, c.logonDateTime,"
        "  c.logoffDateTime, c.locationID, IFNULL(e.typeID, 0) AS shipTypeID, -1 AS lastOnline"
        " FROM chrCharacters AS c"
        "  LEFT JOIN entity AS e ON c.shipID = e.itemID"
        " WHERE c.corporationID = %u", corpID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}

PyRep* CorporationDB::GetMemberTrackingInfoSimple(uint32 corpID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT c.characterID, c.corporationID, c.logoffDateTime, c.logonDateTime, c.title, c.startDateTime, c.corpRole AS roles,"
        " c.baseID, c.blockRoles, IFNULL(e.typeID, 0) AS shipTypeID"
        " FROM chrCharacters AS c"
        "  LEFT JOIN entity AS e ON c.shipID = e.itemID"
        " WHERE corporationID = %u", corpID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}

void CorporationDB::AddItemEvent(uint32 corpID, uint32 charID, uint16 eTypeID)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "INSERT INTO crpItemEvent (corporationID, characterID, eventTypeID, eventDateTime)"
        " VALUES (%u, %u, %u, %f)", corpID, charID, eTypeID, GetFileTimeNow());
}

PyRep* CorporationDB::GetItemEvents(uint32 corpID, uint32 charID, int64 fromDate, int64 toDate, uint8 rowsPerPage)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT eventID, corporationID, characterID, eventTypeID, eventDateTime"
        " FROM crpItemEvent"
        " WHERE corporationID = %u AND characterID = %u AND eventDateTime > %lli AND eventDateTime <= %lli "
        " LIMIT %u", corpID, charID, fromDate, toDate, rowsPerPage))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);

    PyList* list = new PyList();
    if (res.ColumnCount() > 0) {
        DBResultRow row;
        while (res.GetRow(row)) {
            PyDict* dict = new PyDict();
                dict->SetItemString("eventID", new PyInt(row.GetInt(0)));
                dict->SetItemString("corporationID", new PyInt(row.GetInt(1)));
                dict->SetItemString("characterID", new PyInt(row.GetInt(2)));
                dict->SetItemString("eventTypeID", new PyInt(row.GetInt(3)));
                dict->SetItemString("eventDateTime", new PyLong(row.GetInt64(4)));
            list->AddItem(dict);
        }
    } else {
        PyDict* dict = new PyDict();
        dict->SetItemString("eventID", new PyInt(0));
        dict->SetItemString("corporationID", new PyInt(0));
        dict->SetItemString("characterID", new PyInt(0));
        dict->SetItemString("eventTypeID", new PyInt(0));
        dict->SetItemString("eventDateTime", new PyInt(0));
        list->AddItem(dict);
    }

    return new PyObject("util.KeyVal", list);
}

void CorporationDB::AddRoleHistory(uint32 corpID, uint32 charID, uint32 issuerID, int64 oldRoles, int64 newRoles, bool grantable)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "INSERT INTO crpRoleHistroy (corporationID, characterID, issuerID, changeTime, oldRoles, newRoles, grantable)"
        " VALUES (%u, %u, %u, %f, %lli, %lli, %i)", corpID, charID, issuerID, GetFileTimeNow(), oldRoles, newRoles, (grantable ? 1 : 0));
}

PyRep* CorporationDB::GetRoleHistroy(uint32 corpID, uint32 charID, int64 fromDate, int64 toDate, uint8 rowsPerPage)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT corporationID, characterID, issuerID, changeTime, oldRoles, newRoles, grantable"
        " FROM crpRoleHistroy"
        " WHERE corporationID = %u and characterID = %u AND changeTime > %lli AND changeTime <= %lli "
        " LIMIT %u", corpID, charID, fromDate, toDate, rowsPerPage))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);

    PyList* list = new PyList();
    if (res.ColumnCount() > 0) {
        DBResultRow row;
        while (res.GetRow(row)) {
        PyDict* dict = new PyDict();
            dict->SetItemString("corporationID", new PyInt(row.GetInt(0)));
            dict->SetItemString("charID", new PyInt(row.GetInt(1)));
            dict->SetItemString("issuerID", new PyInt(row.GetInt(2)));
            dict->SetItemString("changeTime", new PyLong(row.GetInt64(3)));
            dict->SetItemString("oldRoles", new PyLong(row.GetInt64(4)));
            dict->SetItemString("newRoles", new PyLong(row.GetInt64(5)));
            dict->SetItemString("grantable", new PyBool(row.GetBool(6)));
        list->AddItem(dict);
        }
    } else {
        PyDict* dict = new PyDict();
            dict->SetItemString("corporationID", new PyInt(0));
            dict->SetItemString("charID", new PyInt(0));
            dict->SetItemString("issuerID", new PyInt(0));
            dict->SetItemString("changeTime", new PyInt(0));
            dict->SetItemString("oldRoles", new PyInt(0));
            dict->SetItemString("newRoles", new PyInt(0));
            dict->SetItemString("grantable", new PyBool(false));
        list->AddItem(dict);
    }

    return new PyObject("util.KeyVal", list);
}

void CorporationDB::AddVoteCase(uint32 corpID, uint32 charID, Call_InsertVoteCase& args)
{
    PyDict* dict = args.voteCaseOptions->arguments()->AsDict();
    dict->Dump(CORP__TRACE, "    ");
    PyRep* rep = dict->GetItemString("lines");
    if (!rep->IsList()) {
        codelog(CORP__ERROR, "'lines' item is not PyList: %s", rep->TypeString());
        return;
    }

    std::vector<VoteCaseData> data;
    PyList* list = rep->AsList();
    for (PyList::const_iterator itr = list->begin(); itr != list->end(); ++itr) {
        if (!(*itr)->IsList()) {
            _log(CORP__ERROR, "itr item is not PyList: %s", rep->TypeString());
            continue;
        }
        PyList* list2 = (*itr)->AsList();
        VoteCaseData args2;
        //vote decesion option
        args2.choice = PyRep::StringContent(list2->GetItem(0));
        //for kick, ceo this is charID.  for war, this is corpID.  for lock/unlock, this is itemID  for shares/general, this is boolean
        args2.itemID = PyRep::IntegerValue(list2->GetItem(1));
        //typeID of above itemID.  for shares/general, this is 0
        args2.typeID = PyRep::IntegerValue(list2->GetItem(2));
        //for lock/unlock, this is items locationID.  for others, this is 0
        args2.locationID = PyRep::IntegerValue(list2->GetItem(3));
        data.push_back(args2);
    }

    /*
    DBerror err;
    sDatabase.RunQuery(err,
        " SELECT parameter, timeActedUpon, timeRescended, voteCaseID,"
        " voteType, title, description, expires, actedUpon, inEffect, rescended"
        " FROM crpVoteItems"
        " WHERE corporationID = %u ", corpID);
*/
}


PyRep* CorporationDB::GetVoteItems(uint32 corpID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT parameter, timeActedUpon, timeRescended, voteCaseID,"
        " voteType, title, description, expires, actedUpon, inEffect, rescended"
        " FROM crpVoteItems"
        " WHERE corporationID = %u ", corpID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToPackedRowDict(res, "voteCaseID");
}

void CorporationDB::MoveShares(uint32 ownerID, uint32 corpID, Call_MoveShares& args)
{
    //MoveShares(corporationID, toShareholderID, numberOfShares)
    //  this will also send share update notifications as its' easier to do here..

    // get old owner data
    bool isCorp = false;
    if (IsPlayerCorp(ownerID))
        isCorp = true;
    DBQueryResult res;
    sDatabase.RunQuery(res,"SELECT shares FROM crpShares WHERE corporationID = %u AND shareholderID = %u ", corpID, ownerID);
    DBResultRow row;
    if (res.GetRow(row)) {
        // add to notification
        OnCorpShareChange corpUpdate;
            corpUpdate.ownerID = ownerID;
            corpUpdate.corpID = corpID;
            corpUpdate.oldShares = row.GetInt(0);
            corpUpdate.newShares = row.GetInt(0) - args.numberOfShares;
        MulticastTarget mct;
            mct.characters.insert((isCorp ? ownerID : corpID));
        if (isCorp)
            mct.corporations.insert(corpID);
        PyTuple* tuple = corpUpdate.Encode();
        sEntityList.Multicast("OnShareChange", "*corpid&corprole", &tuple, mct);
    }

    DBerror err;
    // remove from old owner
    sDatabase.RunQuery(err,
        "UPDATE crpShares SET shares = shares - %i"
        " WHERE corporationID = %u AND shareholderID = %u ", args.numberOfShares, args.corporationID, ownerID);

    // get new owner data
    uint16 oldShares = 0;
    uint32 oldCorpID = 0;
    Client* pClient(nullptr);
    if (IsCharacter(args.toShareholderID)) {
        pClient = sEntityList.FindClientByCharID(args.toShareholderID);
        if (pClient == nullptr)
            oldCorpID = CharacterDB::GetCorpID(args.toShareholderID);
        else
            oldCorpID = pClient->GetCorporationID();
    }
    OnCharShareChange charUpdate;
    charUpdate.ownerID = args.toShareholderID;
    charUpdate.corpID = corpID;
    charUpdate.newShares = args.numberOfShares; // plus existing shares this owner has of this corp
    //res.Reset();
    sDatabase.RunQuery(res,"SELECT shares, shareholderCorporationID FROM crpShares WHERE corporationID = %u AND shareholderID = %u ", corpID, args.toShareholderID);
    if (res.GetRow(row)) {
        charUpdate.oldShares = (oldShares = row.GetInt(0));
        charUpdate.oldCorpID = row.GetInt(1);
        charUpdate.oldOwnerID = args.toShareholderID;
        charUpdate.newOwnerOldCorpID = (isCorp ? 0 : oldCorpID);
    } else {
        charUpdate.oldShares = 0;
        charUpdate.oldCorpID = 0;
        charUpdate.oldOwnerID = 0;
        charUpdate.newOwnerOldCorpID = 0;
    }

    charUpdate.newShares = (oldShares + args.numberOfShares);
    charUpdate.newCorpID = corpID;
    charUpdate.newOwnerID = args.toShareholderID;
    charUpdate.newOwnerNewCorpID = (isCorp ? 0 : oldCorpID);
    pClient->SendNotification("OnShareChange", "charid", charUpdate.Encode());

    // add to new owner
    sDatabase.RunQuery(err,
        "INSERT INTO crpShares (corporationID, shareholderID, shares, shareholderCorporationID)"
        " VALUES (%i, %i, %i, %u)"
        " ON DUPLICATE KEY UPDATE shares = shares + %i", args.corporationID, args.toShareholderID, args.numberOfShares, corpID, args.numberOfShares);
}

PyRep* CorporationDB::GetShares(uint32 corpID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT corporationID, shareholderID, shares, shareholderCorporationID"
        " FROM crpShares"
        " WHERE corporationID = %u ", corpID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToIndexRowset(res, "shareholderID");
}

PyRep *CorporationDB::GetMyShares(uint32 ownerID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT corporationID, shareholderID, shares, shareholderCorporationID"
        " FROM crpShares"
        " WHERE shareholderID = %u ", ownerID))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToRowset(res);
}

PyRep* CorporationDB::GetAssetInventory(uint32 corpID, uint8 flag)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT locationID FROM entity"
        " WHERE ownerID = %u AND flag = %u", corpID, flag))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}

PyRep* CorporationDB::GetAssetInventoryForLocation(uint32 corpID, uint32 locationID, uint8 flag)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT locationID FROM entity "
        " WHERE ownerID = %u AND locationID = %u AND flag = %u", corpID, locationID, flag))
    {
        codelog(CORP__DB_ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }
    return DBResultToCRowset(res);
}

PyRep* CorporationDB::GetKillsAndLosses(uint32 corpID, uint32 number, uint32 offset)
{
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
        " WHERE ((victimCorporationID = %u) OR (finalCorporationID = %u))", corpID, corpID))
    {
        codelog(CORP__DB_ERROR, "Error on query: %s", res.error.c_str());
        return nullptr;
    }

    _log(DATABASE__RESULTS, "CorporationDB::GetKillsAndLosses for corpID: %u returned %u items", corpID, res.GetRowCount());

    return DBResultToCRowset(res);
}

PyRep* CorporationDB::GetMktInfo(uint32 corpID)
{
    // bid = buy order
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT sell.typeID AS typeID, sell.price AS sellPrice, sell.volRemaining AS sellQuantity, sell.issued AS sellDate, sell.stationID AS sellStationID, "
        " buy.price AS buyPrice, buy.volRemaining AS buyQuantity, buy.issued AS buyDate, buy.stationID AS buyStationID"
        " FROM mktOrders AS sell, mktOrders AS buy"
        " WHERE sell.ownerID = %u AND buy.ownerID = %u AND sell.typeID = buy.typeID AND sell.bid=0 AND buy.bid=1", corpID, corpID))    // bid=buy order
    {
        codelog(CORP__DB_ERROR, "Error on query: %s", res.error.c_str());
    }

    _log(DATABASE__RESULTS, "CorporationDB::GetMktBuyInfo for corpID: %u returned %u items", corpID, res.GetRowCount());

    return DBResultToCRowset(res);
}

void CorporationDB::GetCorpData(CorpData& data)
{
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  taxRate,"
        "  stationID,"
        "  allianceID,"
        "  warFactionID,"
        "  corporationName,"
        "  tickerName"
        " FROM crpCorporation"
        " WHERE corporationID = %u", data.corporationID))
    {
        codelog(DATABASE__ERROR, "Failed to query data of corporation %u: %s.", data.corporationID, res.error.c_str());
        return;
    }

    DBResultRow row;
    if(!res.GetRow(row)) {
        _log(DATABASE__MESSAGE, "No data found for character's %u corporation.", data.corporationID);
        return;
    }

    data.taxRate = row.GetDouble(0);
    data.corpHQ = (row.IsNull(1) ? 0 : row.GetUInt(1));
    data.allianceID = (row.IsNull(2) ? 0 : row.GetUInt(2));
    data.warFactionID = (row.IsNull(3) ? 0 : row.GetUInt(3));
    data.name = row.GetText(4);
    data.ticker = row.GetText(5);
}

void CorporationDB::UpdateCorpHQ(uint32 corpID, uint32 stationID)
{
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE crpCorporation SET stationID = %u WHERE corporationID = %u", stationID, corpID))
        codelog(CORP__DB_ERROR, "Error in UpdateCorpHQ query: %s", err.c_str());
}

void CorporationDB::EditLabel(uint32 corpID, uint32 labelID, uint32 color, std::string name)
{
    std::string eName;
    sDatabase.DoEscapeString(eName, name);

    DBQueryResult res;
    sDatabase.RunQuery(res, "UPDATE crpLabels SET color = %u, name = '%s' WHERE ownerID = %u AND labelID = %u", color, eName.c_str(), corpID, labelID);
}

PyRep* CorporationDB::GetLabels(uint32 corpID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT labelID, color, name FROM crpLabels WHERE ownerID = %u", corpID)) {
        codelog(DATABASE__ERROR, "Error on query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCIndexedRowset(res, "labelID");
}

void CorporationDB::SetLabel(uint32 corpID, uint32 color, std::string name)
{
    std::string eName;
    sDatabase.DoEscapeString(eName, name);

    DBQueryResult res;
    sDatabase.RunQuery(res, "INSERT INTO crpLabels (color, name, ownerID) VALUES (%u, '%s', %u)", color, eName.c_str(), corpID);
}

void CorporationDB::DeleteLabel(uint32 corpID, uint32 labelID)
{

}
