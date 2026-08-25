/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    Copyright 2016 - 2026 Alasiya-EvE by Allan
    For the latest implementation status visit http://eve.alasiya.net/?p=op_status
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
    Rewrite:    Allan
*/

#include <boost/algorithm/string.hpp>

#include "eve-server.h"
#include "Client.h"
#include "chat/LSCDB.h"
#include "chat/LSCService.h"


int32 LSCDB::GetHighestChannelIDFromDB() {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT MAX(channelID) FROM channels")) {
        return 0;
    }

    DBResultRow row;
    if (res.GetRow(row) && !row.IsNull(0))
        return row.GetInt(0);

    return 0;
}

void LSCDB::UpdateChannelInfo(LSCChannel *channel) {
    if (channel == nullptr)
        return;

    int32 rawChannelID = channel->GetChannelID();
    int32 absoluteChannelID = (rawChannelID < 0) ? (rawChannelID & 0x7FFFFFFF) : rawChannelID;
    DBerror err;

    // PATH A: Target is an Authoritative Mailing List Structure
    if (IsMailList(absoluteChannelID)) {
        std::string escapedListName;
        sDatabase.DoEscapeString(escapedListName, channel->GetDisplayName());
        sDatabase.RunQuery(err,
                           "INSERT INTO eveMailLists (listID, listName, ownerID, mailCost) "
                           "VALUES (%i, '%s', %u, %u) "
                           "ON DUPLICATE KEY UPDATE listName = VALUES(listName), ownerID = VALUES(ownerID), mailCost = VALUES(mailCost)",
                           absoluteChannelID, escapedListName.c_str(), channel->GetOwnerID(), channel->GetCSPA());
        return;
    }

    // PATH B: Target is a Standard Chat Room Space
    std::string escapedName;
    sDatabase.DoEscapeString(escapedName, channel->GetDisplayName());
    std::string escapedMOTD;
    sDatabase.DoEscapeString(escapedMOTD, channel->GetMOTD());
    std::string escapedKey;
    sDatabase.DoEscapeString(escapedKey, channel->GetComparisonKey());
    std::string escapedPass;
    sDatabase.DoEscapeString(escapedPass, channel->GetPassword());

    std::string passwordValue = channel->GetPassword().empty() ? "NULL" : "'" + escapedPass + "'";
    sDatabase.RunQuery(err,
                       "INSERT INTO channels (channelID, ownerID, displayName, motd, comparisonKey, memberless, password, cspa) "
                       "VALUES (%i, %u, '%s', '%s', '%s', %u, %s, %u) "
                       "ON DUPLICATE KEY UPDATE ownerID = VALUES(ownerID), displayName = VALUES(displayName), motd = VALUES(motd), "
                       "comparisonKey = VALUES(comparisonKey), memberless = VALUES(memberless), password = VALUES(password), cspa = VALUES(cspa)",
                       absoluteChannelID, channel->GetOwnerID(), escapedName.c_str(), escapedMOTD.c_str(), escapedKey.c_str(),
                       (channel->GetMemberless() ? 1 : 0), passwordValue.c_str(), channel->GetCSPA());
}

bool LSCDB::IsChannelNameAvailable(const std::string& name) {
    // Generate a clean comparison key
    std::string compKey = name;
    boost::algorithm::trim(compKey);
    boost::algorithm::to_lower(compKey);
    compKey.erase(std::remove(compKey.begin(), compKey.end(), ' '), compKey.end());

    if (compKey.empty())
        return false;

    DBQueryResult res;
    // Perform a quick pass against your primary key text index
    if (!sDatabase.RunQuery(res, "SELECT channelID FROM channels WHERE comparisonKey = '%s'", compKey.c_str())) {
        return false;
    }

    return (res.GetRowCount() < 1);
}

bool LSCDB::IsChannelIDAvailable(int32 channelID) {
    int32 absoluteChannelID = (channelID < 0) ? (channelID & 0x7FFFFFFF) : channelID;

    DBQueryResult res;
    if (IsMailList(absoluteChannelID)) {
        if (!sDatabase.RunQuery(res,
            "SELECT listID FROM eveMailLists WHERE listID = %i", absoluteChannelID))
        {
            return false;
        }

        if (res.GetRowCount())
            return false;
    }

    if (!sDatabase.RunQuery(res, "SELECT channelID FROM channels WHERE channelID = %i", absoluteChannelID)) {
        return false;
    }

    if (res.GetRowCount())
        return false;

    return true;
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

void LSCDB::UpdateSubscription(int32 channelID, Client* pClient) {
    if (pClient == nullptr) return;

    int32 absoluteChannelID = (channelID < 0) ? (channelID & 0x7FFFFFFF) : channelID;
    DBerror err;
    int64 currentTimestamp = GetFileTimeNow();

    // PATH A: Roster sync for a 105M Mailing List Group
    if (IsMailList(absoluteChannelID)) {
        sDatabase.RunQuery(err,
                           "INSERT INTO eveMailListMembers (listID, characterID, joinDate, roleStatus) "
                           "VALUES (%i, %u, %lli, 0) "
                           "ON DUPLICATE KEY UPDATE roleStatus = VALUES(roleStatus)",
                           absoluteChannelID, pClient->GetCharacterID(), currentTimestamp);
        return;
    }

    // PATH B: Roster sync for standard active chat channels
    sDatabase.RunQuery(err,
                       "INSERT INTO channelChars (channelID, corpID, charID, allianceID, role, extra) "
                       "VALUES (%i, %u, %u, %u, %lli, 0) "
                       "ON DUPLICATE KEY UPDATE corpID = VALUES(corpID), allianceID = VALUES(allianceID), role = VALUES(role)",
                       absoluteChannelID, pClient->GetCorporationID(), pClient->GetCharacterID(), pClient->GetAllianceID(), pClient->GetAccountRole());
}

bool LSCDB::IsChannelSubscribedByThisChar(uint32 characterID, int32 channelID) {
    DBQueryResult res;
    if (IsMailList(channelID)) {
        sDatabase.RunQuery(res, "SELECT 1 FROM eveMailListMembers WHERE listID = %i AND characterID = %u;",
                           channelID, characterID);

        if (res.GetRowCount()) {
            return true;
        }
    } else {
        sDatabase.RunQuery(res, "SELECT 1 FROM channelChars WHERE channelID = %i AND charID = %u;",
                           channelID, characterID);

        if (res.GetRowCount()) {
            return true;
        }
    }

    return false;
}


void LSCDB::ForgetChannel(int32 charID, int32 channelID) {
    int32 absoluteChannelID = (channelID < 0) ? (channelID & 0x7FFFFFFF) : channelID;

    DBerror err;
    if (IsMailList(absoluteChannelID)) {
        //this probably will never hit, but just in case...
        sDatabase.RunQuery(err, "DELETE FROM eveMailLists WHERE listID = %i", absoluteChannelID);
        sDatabase.RunQuery(err, "DELETE FROM eveMailListMembers WHERE listID = %i", absoluteChannelID);
    } else if (!IsStaticChannel(absoluteChannelID)) {
        sDatabase.RunQuery(err, "DELETE FROM channels WHERE channelID = %i", absoluteChannelID);
        sDatabase.RunQuery(err, "DELETE FROM channelAcl WHERE channelID = %i", absoluteChannelID);
        sDatabase.RunQuery(err, "DELETE FROM channelChars WHERE channelID = %i", absoluteChannelID);
    } else {
        sDatabase.RunQuery(err, "DELETE FROM channelChars WHERE channelID = %i AND charID = %i", \
        absoluteChannelID, charID);
    }
}

void LSCDB::DeleteChannel(int32 channelID) {
    int32 absoluteChannelID = (channelID < 0) ? (channelID & 0x7FFFFFFF) : channelID;

    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM channels WHERE channelID = %i", absoluteChannelID);
    sDatabase.RunQuery(err, "DELETE FROM channelAcl WHERE channelID = %i", absoluteChannelID);
    sDatabase.RunQuery(err, "DELETE FROM channelChars WHERE channelID = %i", absoluteChannelID);
}

void LSCDB::DeleteSubscription(int32 channelID, int32 characterID) {
    int32 absoluteChannelID = (channelID < 0) ? (channelID & 0x7FFFFFFF) : channelID;

    DBerror err;
    sDatabase.RunQuery(err,
                       "DELETE FROM channelChars WHERE channelID = %i AND charID = %i",
                       absoluteChannelID, characterID);
}

void LSCDB::UpdateChannelMode(int32 channelID, int32 rawModeVal) {
    int32 absoluteChannelID = (channelID < 0) ? (channelID & 0x7FFFFFFF) : channelID;

    DBerror err;
    sDatabase.RunQuery(err, "UPDATE channels SET defaultAccess = %i WHERE channelID = %i",
                       rawModeVal, absoluteChannelID);
}

void LSCDB::UpdateUserChannelAccess(int32 channelID, uint32 targetCharID, int32 rawModeVal) {
    int32 absoluteChannelID = (channelID < 0) ? (channelID & 0x7FFFFFFF) : channelID;

    DBerror err;
    sDatabase.RunQuery(err, " INSERT INTO channels (listID, entityID, accessLevel) VALUES (%i, %u, %i) "
    " ON DUPLICATE KEY UPDATE accessLevel = VALUES(accessLevel)",
                       absoluteChannelID, targetCharID, rawModeVal);
}

bool LSCDB::LoadChannelACL(int32 channelID, std::unordered_map<uint32, AclEntry*>& aclMap) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res, "SELECT accessorID, mode, untilWhen, originalMode, reason, adminID "
        " FROM channelAcl WHERE channelID = %i", channelID))
    {
        _log(LSC__ERROR, "LSCDB::LoadChannelACL() - Database execution failure for room %i.", channelID);
        return false;
    }

    DBResultRow row;
    while (res.GetRow(row)) {
        // remove expired bans, if any
        int64 banTime = row.GetInt64(2);
        if ((banTime > 0) && (GetFileTimeNow() >= banTime)) {
            RemoveChannelACL(channelID, row.GetUInt(0));
            continue;
        }

        aclMap.emplace(row.GetInt(0),  new AclEntry(
            row.GetInt(0),
                                                    row.GetInt8(1),
                                                    banTime,
                                                    row.GetInt8(3),
                                                    row.GetText(4),
                                                    row.GetInt(5))
        );
    }

    _log(LSC__CHANNELS, "LSCDB: Loaded %lli active access control entries into memory cache for channel %i",
         aclMap.size(), channelID);

    return true;
}

bool LSCDB::SaveChannelACL(int32 channelID, const AclEntry* acl) {
    if (acl == nullptr) return false;

    DBQueryResult res;
    char queryStr[1024];

    // CRITICAL: Strict MariaDB 10.0.3 legacy VALUES() syntax required for upserts
    snprintf(queryStr, sizeof(queryStr),
             "INSERT INTO channelAcl (channelID, accessorID, mode, untilWhen, originalMode, reason, adminID) "
             "VALUES (%d, %u, %i, %lli, %i, '%s', %u) "
             "ON DUPLICATE KEY UPDATE "
             "mode = VALUES(mode), "
             "untilWhen = VALUES(untilWhen), "
             "originalMode = VALUES(originalMode), "
             "reason = VALUES(reason), "
             "adminID = VALUES(adminID);",
             channelID, acl->accessorID, acl->mode,
             acl->untilWhen, acl->originalMode,
             acl->reason.c_str(), acl->adminID);

    return sDatabase.RunQuery(res, queryStr);
}

bool LSCDB::RemoveChannelACL(int32 channelID, uint32 accessorID) {
    DBQueryResult res;
    char queryStr[256];

    snprintf(queryStr, sizeof(queryStr),
             "DELETE FROM channelAcl WHERE channelID = %i AND accessorID = %u;",
             channelID, accessorID);

    return sDatabase.RunQuery(res, queryStr);
}
