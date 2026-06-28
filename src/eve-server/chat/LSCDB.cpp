/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
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
    Author:        Zhur, Aknor Jaden
    Rewrite:    Allan  (incomplete)
*/

/** @todo this entire file needs review */

#include "../eve-server.h"
#include "Client.h"
#include "chat/LSCDB.h"
#include "chat/LSCService.h"



// Clean, structured signature:
LSC::CharMetaData LSCDB::GetChannelNames(uint32 charID)
{
    DBQueryResult res;

    // Executing an optimized, explicitly structured query
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "    ch.characterName, "
        "    COALESCE(cr.corporationName, '') " // Protect against character-less corps
        " FROM chrCharacters AS ch"
        " LEFT JOIN crpCorporation AS cr USING (corporationID) "
        " WHERE ch.characterID = %u", charID))
    {
        _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return LSC::CharMetaData("", ""); // Return safe empty defaults
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(SERVICE__ERROR, "CharID %u isn't present in the database", charID);
        return LSC::CharMetaData("", "");
    }

    // Safely extract text pointers and handle null safety idiomatic to modern C++
    std::string charName = (row.GetText(0) == nullptr ? "" : row.GetText(0));
    std::string corpName = (row.GetText(1) == nullptr ? "" : row.GetText(1));

    // Return the structure natively. Modern compilers perform RVO (Return Value Optimization)
    return LSC::CharMetaData(charName, corpName);

	// LSC::CharMetaData meta = m_db->GetChannelNames(charID);
	// _log(LSC__INFO, "Loaded login names for %s of corp %s", meta.characterName.c_str(), meta.corporationName.c_str());

}

void LSCDB::UpdateChannelInfo(LSCChannel *channel) {
    // Sanitize user-facing inputs to securely escape characters like apostrophes

    std::string escapedName;
    sDatabase.DoEscapeString(escapedName, channel->GetDisplayName());
    std::string escapedMOTD;
    sDatabase.DoEscapeString(escapedMOTD, channel->GetMOTD());
    std::string escapedKey;
    sDatabase.DoEscapeString(escapedKey, channel->GetComparisonKey());

    // Clean, safe parameterization logic for the password field
    std::string passwordValue;
    if (channel->GetPassword().empty()) {
        passwordValue = "NULL"; // True SQL NULL token (no single quotes)
    } else {
        std::string escapedPass;
        sDatabase.DoEscapeString(escapedPass, channel->GetPassword());
        passwordValue = "'" + escapedPass + "'"; // Securely quoted & escaped
    }

    DBerror err;
    // Optimized query utilizing a modern Row Alias (AS target) and safe string injections
    if (!sDatabase.RunQuery(err,
        " INSERT INTO channels"
        "   (channelID, ownerID, displayName, motd, comparisonKey, memberless, password, mailingList, cspa)"
        " VALUES (%i, %u, '%s', '%s', '%s', %u, %s, %u, %u)"
        " ON DUPLICATE KEY UPDATE"
        "  ownerID = VALUES(ownerID),"
        "  displayName = VALUES(displayName),"
        "  motd = VALUES(motd),"
        "  comparisonKey = VALUES(comparisonKey),"
        "  memberless = VALUES(memberless),"
        "  password = VALUES(password),"
        "  mailingList = VALUES(mailingList),"
        "  cspa = VALUES(cspa)",
        channel->GetChannelID(),
        channel->GetOwnerID(),
        escapedName.c_str(),
        escapedMOTD.c_str(),
        escapedKey.c_str(),
        (channel->GetMemberless() ? 1 : 0),
        passwordValue.c_str(), // Formatted dynamic SQL token string
        (channel->GetMailingList() ? 1 : 0),
        channel->GetCSPA()))
    {
        _log(DATABASE__ERROR, "Error in UpdateChannelInfo query: %s", err.c_str());
    }
}

void LSCDB::UpdateSubscription(int32 channelID, Client* pClient) {
    DBerror err;
    // Corrected signed (%i) specifiers to unsigned (%u) to prevent negative ID wraps
    sDatabase.RunQuery(err,
        " INSERT INTO channelChars "
        " (channelID, corpID, charID, allianceID, role, extra) "
        " VALUES (%i, %u, %u, %u, %lli, 0) "
        " ON DUPLICATE KEY UPDATE role = VALUES(role)", // Prevent duplicate primary key insertion locks
        channelID,
        pClient->GetCorporationID(),
        pClient->GetCharacterID(),
        pClient->GetAllianceID(),
        pClient->GetAccountRole());
}

void LSCDB::DeleteChannel(int32 channelID)
{
    DBerror err;
	if (!sDatabase.RunQuery(err, "DELETE FROM channels WHERE channelID=%i", channelID))
    	_log(DATABASE__ERROR, "Failed to delete channel %i from DB: %s", channelID, err.c_str());
}

void LSCDB::DeleteSubscription(int32 channelID, uint32 charID)
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM channelChars WHERE channelID=%i AND charID=%u", channelID, charID );
	/*ALTER TABLE channelChars
ADD CONSTRAINT fk_channel
FOREIGN KEY (channelID) REFERENCES channels(channelID)
ON DELETE CASCADE;
*/
}

int32 LSCDB::GetChannelID(std::string &name) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT channelID FROM channels WHERE displayName RLIKE '%s'", name.c_str())) {
        _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(SERVICE__ERROR, "Channel named '%s' isn't present in the database", name.c_str() );
        return 0;
    }

    return row.GetInt(0);
}

bool LSCDB::GetChannelInformation(int32 channelID, LSC::ChannelData& data) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT"
        "   channelID, "
        "   displayName, "
        "   motd, "
        "   ownerID, "
        "   comparisonKey, "
        "   memberless, "
        "   password, "
        "   mailingList, "
        "   cspa "
        " FROM channels "
        " WHERE channelID = %i", channelID))
    {
        _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(SERVICE__ERROR, "Channel %i isn't present in the database", channelID);
        return false;
    }

    // Populate the structured data payload using modern nullptr checking rules
    data.channelID     = row.GetInt(0);
    data.displayName   = (row.GetText(1) == nullptr ? "" : row.GetText(1));
    data.motd          = (row.GetText(2) == nullptr ? "" : row.GetText(2));
    data.ownerID       = row.GetUInt(3);
    data.comparisonKey = (row.GetText(4) == nullptr ? "" : row.GetText(4));
    data.memberless    = (row.GetUInt(5) != 0); // Convert directly to boolean
    data.password      = (row.GetText(6) == nullptr ? "" : row.GetText(6));
    data.mailingList   = (row.GetUInt(7) != 0); // Convert directly to boolean
    data.cspa          = row.GetUInt(8);

    return true; // Execution succeeded, caller can safely use the struct data
}

void LSCDB::GetChannelSubscriptions(uint32 charID, std::vector<LSC::ChannelData>& subscriptions) {
    // Clear out the vector to prevent appending to stale data if reused
    subscriptions.clear();

    DBQueryResult res;
    // Optimized using an INNER JOIN instead of a nested subquery loop
    if (!sDatabase.RunQuery(res,
        "SELECT"
        "   c.channelID, "
        "   c.displayName, "
        "   c.motd, "
        "   c.ownerID, "
        "   c.comparisonKey, "
        "   c.memberless, "
        "   c.password, "
        "   c.mailingList, "
        "   c.cspa "
        " FROM channels c"
        " INNER JOIN channelChars cc ON c.channelID = cc.channelID "
        " WHERE cc.charID = %u", charID))
    {
        _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return;
    }

    DBResultRow row;
    subscriptions.reserve(res.GetRowCount());
    while (res.GetRow(row))  {
	    // Construct the item straight inside the vector array memory!
        subscriptions.emplace_back(LSC::ChannelData{
            row.GetInt(0),                                      // channelID
            (row.GetText(1) == nullptr ? "" : row.GetText(1)),  // displayName
            (row.GetText(2) == nullptr ? "" : row.GetText(2)),  // motd
            row.GetUInt(3),                                     // ownerID
            (row.GetText(4) == nullptr ? "" : row.GetText(4)),  // comparisonKey
            (row.GetUInt(5) != 0),                              // memberless
            (row.GetText(6) == nullptr ? "" : row.GetText(6)),  // password
            (row.GetUInt(7) != 0),                              // mailingList
            row.GetUInt(8)                                      // cspa
        });
    }
}

//TODO  check these next 2 calls to solve error/warning in console...
bool LSCDB::GetChannelInfo(int32 channelID, std::string &name, std::string &motd)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT displayName, motd FROM channels WHERE channelID = %i ", channelID)) {
        _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        // _log(SERVICE__ERROR, "Couldn't find %u in table channels", channelID);
        return false;
    }
name = (row.GetText(0) == nullptr ? "" : row.GetText(0));
motd = (row.GetText(1) == nullptr ? "" : row.GetText(1));


    return true;
}

//TODO: replace this database hit entirely by having your memory lookup scan for the matching comparison key via GetChannelByName
int32 LSCDB::GetChannelIDFromComparisonKey(std::string compkey)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT channelID FROM channels WHERE comparisonKey = '%s'", compkey.c_str())) {
        _log(DATABASE__ERROR, "Error in GetChannelIDFromComparisonKey query: %s", res.error.c_str());
        return 0;
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(SERVICE__ERROR, "Couldn't find %s in table channels", compkey.c_str());
        return 0;
    }

    // Protect string conversion from underlying SQL null pointers
    return (row.IsNull(0) ? 0 : row.GetInt(0));
}

std::string LSCDB::GetChannelName(uint32 id, const char * table, const char * column, const char * key) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,"SELECT %s FROM %s WHERE %s = %u ", column, table, key, id)) {
        _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return "";
    }

    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(SERVICE__ERROR, "Couldn't find %s %u in table %s", key, id, table);
        return "";
    }

    return row.GetText(0);
}

uint32 LSCDB::StoreMail(uint32 senderID, uint32 recipID, const char * subject, const char * message, int64 sentTime) {
    DBQueryResult res;
    DBerror err;
    DBResultRow row;

    std::string escaped;
    // Escape message header
    sDatabase.DoEscapeString(escaped, subject);

    // Store message header
    uint32 messageID;
    if (!sDatabase.RunQueryLID(err, messageID,
        " INSERT INTO eveMail "
        " (channelID, senderID, subject, created) "
        " VALUES (%u, %u, '%s', %lli) ",
                               recipID, senderID, escaped.c_str(), sentTime ))
    {
        _log(DATABASE__ERROR, "Error in query, message header couldn't be saved: %s", err.c_str());
        return (0);
    }

    _log(SERVICE__MESSAGE, "New messageID: %u", messageID);

    // Escape message content
    sDatabase.DoEscapeString(escaped, message);

    // Store message content
    if (!sDatabase.RunQuery(err,
        " INSERT INTO eveMailDetails "
        " (messageID, mimeTypeID, attachment) VALUES (%u, 1, '%s') ",
                            messageID, escaped.c_str()
    ))
    {
        _log(DATABASE__ERROR, "Error in query, message content couldn't be saved: %s", err.c_str());
        // Delete message header
        if (!sDatabase.RunQuery(err, "DELETE FROM `eveMail` WHERE `messageID` = %u;", messageID))
        {
            _log(DATABASE__ERROR, "Failed to remove invalid header data for messgae id %u: %s", messageID, err.c_str());
        }
        return (0);
    }


    return (messageID);
}


PyObject *LSCDB::GetMailHeaders(uint32 recID) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        "SELECT channelID, messageID, senderID, subject, created, `read` "
        " FROM eveMail "
        " WHERE channelID=%u", recID))
    {
        _log(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToRowset(res);
}


PyRep *LSCDB::GetMailDetails(uint32 messageID, uint32 readerID) {
    DBQueryResult result;
    DBResultRow row;

    //we need to query out the primary message here... not sure how to properly
    //grab the "main message" though... the text/plain clause is pretty hackish.
    if (!sDatabase.RunQuery(result,
        " SELECT eveMail.messageID, eveMail.senderID, eveMail.subject, " // need messageID as char*
        "   eveMailDetails.attachment, eveMailDetails.mimeTypeID, "
        "   eveMailMimeType.mimeType, eveMailMimeType.`binary`, "
        "   eveMail.created, eveMail.channelID "
        " FROM eveMail "
        " LEFT JOIN eveMailDetails ON eveMailDetails.messageID = eveMail.messageID "
        " LEFT JOIN eveMailMimeType ON eveMailMimeType.mimeTypeID = eveMailDetails.mimeTypeID "
        " WHERE eveMail.messageID=%u AND channelID=%u",
        messageID, readerID
    ))
    {
        _log(DATABASE__ERROR, "Error in query: %s", result.error.c_str());
        return nullptr;
    }

    if (!result.GetRow(row)) {
        codelog(SERVICE__MESSAGE, "No message with messageID %u", messageID);
        return nullptr;
    }

    Rsp_GetEVEMailDetails details;
    details.messageID = row.GetUInt(0);
    details.senderID = row.GetUInt(1);
    details.subject = row.GetText(2);
    details.body = row.GetText(3);
    details.created = row.GetInt64(7);
    details.channelID = row.GetUInt(8);
    details.deleted = 0; // If a message's details are sent, then it isn't deleted. If it's deleted, details cannot be sent
    details.mimeTypeID = row.GetInt(4);
    details.mimeType = row.GetText(5);
    details.binary = row.GetInt(6);

    return details.Encode();
}


bool LSCDB::MarkMessageRead(uint32 messageID) {
    DBerror err;

    if (!sDatabase.RunQuery(err,
        " UPDATE eveMail "
        " SET `read` = 1 "
        " WHERE messageID=%u", messageID
    ))
    {
        _log(DATABASE__ERROR, "Error in query: %s", err.c_str());
        return false;
    }

    return true;
}


bool LSCDB::DeleteMessage(uint32 messageID, uint32 readerID) {
    DBerror err;
    bool ret = true;

    if (!sDatabase.RunQuery(err,
        " DELETE FROM eveMail "
        " WHERE messageID=%u AND channelID=%u", messageID, readerID
    ))
    {
        _log(DATABASE__ERROR, "Error in query: %s", err.c_str());
        ret = false;
    }
    if (!sDatabase.RunQuery(err,
        " DELETE FROM eveMailDetails "
        " WHERE messageID=%u", messageID
    ))
    {
        _log(DATABASE__ERROR, "Error in query: %s", err.c_str());
        ret = false;
    }

    return ret;
}
