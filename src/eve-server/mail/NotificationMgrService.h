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
    Author:        caytchen
    Update:     Allan
*/

#ifndef __NOTIFICATIONMGRSERVICE__H__INCL__
#define __NOTIFICATIONMGRSERVICE__H__INCL__

#include "PyService.h"

class NotificationMgrService : public PyService {
public:
    NotificationMgrService(PyServiceMgr* mgr);
    virtual ~NotificationMgrService();

private:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(GetByGroupID);
    PyCallable_DECL_CALL(GetUnprocessed);
    PyCallable_DECL_CALL(MarkGroupAsProcessed);
    PyCallable_DECL_CALL(MarkAllAsProcessed);
    PyCallable_DECL_CALL(MarkAsProcessed);
    PyCallable_DECL_CALL(DeleteGroupNotifications);
    PyCallable_DECL_CALL(DeleteAllNotifications);
    PyCallable_DECL_CALL(DeleteNotifications);
};

class NotificationDataBuilder {
private:
    std::stringstream m_ss;

public:
    NotificationDataBuilder() = default;
    ~NotificationDataBuilder() = default;

    // Formats integers cleanly into the stream (e.g., "charID: 90000001\n")
    void AddInt(const char* key, int32 value) {
        m_ss << key << ": " << value << "\n";
    }

    // Handles your explicit int64 time tracks and massive ISK totals safely
    void AddInt64(const char* key, int64 value) {
        m_ss << key << ": " << value << "\n";
    }

    // Encases strings in quotes to prevent structural characters from breaking the client parser
    void AddString(const char* key, const std::string& value) {
        m_ss << key << ": \"" << value << "\"\n";
    }

    // Handles floating point data types for tax percentages or fine ISK decimals
    void AddFloat(const char* key, double value) {
        m_ss << key << ": " << std::fixed << value << "\n";
    }

    // Extracts the final combined configuration text block to feed into the database write
    std::string GetString() const {
        return m_ss.str();
    }
};

class NotificationFactory {
public:
    /**
     * Dispatch an authoritative notification to a group of recipients.
     * Handles both persistent DB logging and real-time zero-clone network blasting.
     */
    static void Dispatch(
        int32 senderID,
        int32 typeID,
        const std::string& serializedData,
        const std::vector<int32>& receiverIDs);
};

#endif


/*
 * (236000, `Insurance Contract Issued`)
 * (236001, `Report: Starbase low on resources in {[location]solarSystemID.name}`)
 * (236003, `Report: "{[item]typeID.name, linkify}" at "{[location]stationID.name, linkify}" has been disabled`)
 * (236006, `{[character]newCeoID.name} is the new CEO of {corporationName}`)
 * (236007, `Welcome to {corporationName}`)
 * (236008, `Rejected application to join {corporationName}`)
 * (236009, `Report: "{[item]typeID.name, linkify}" at "{[location]stationID.name, linkify}" has been reenabled`)
 * (236014, `Report: Infrastructure hub %22{name}%22 has been conquered`)
 * (236018, `Report: Starbase in {[location]solarSystemID.name, linkify} is under attack`)
 * (236019, `Report: Station '{[location]stationID.name}' has been conquered`)
 */



/***************** example calls for notifications *******************
void MarketManager::OnOrderCompleted(int32 traderCharID, int32 corpID, int32 stationID, int32 quantity) {
    // 1. Build the client-required YAML dictionary string literal
    NotificationDataBuilder data;
    data.AddInt("stationID", stationID);
    data.AddInt("quantity", quantity);

    // 2. Fetch the roster of accountants or traders who care about market updates
    std::vector<int32> listeners = m_db->GetCorpMembersWithRole(corpID, Corp::Role::Trader);

    // 3. Dispatch atomically
    NotificationFactory::Dispatch(traderCharID, Notify::Types::MarketOrder, data.GetString(), listeners);
}
void IndustryEngine::OnJobFinished(int32 jobID, int32 ownerCharID, int32 corpID, int32 blueprintTypeID) {
    NotificationDataBuilder data;
    data.AddInt("jobID", jobID);
    data.AddInt("blueprintTypeID", blueprintTypeID);
    data.AddString("status", "Completed");

    // Send alert strictly to the character who started it, or all factory executives
    std::vector<int32> listeners = { ownerCharID };

    // System acts as sender (e.g., passing 1M NPC Agent or system 0 context)
    NotificationFactory::Dispatch(0, Notify::Types::FactoryJob, data.GetString(), listeners);
}
void WalletService::ModifyCorpBalance(int32 executiveID, int32 corpID, int32 divisionID, double amountChanged) {
    NotificationDataBuilder data;
    data.AddInt("divisionID", divisionID);
    data.AddFloat("delta", amountChanged);

    // Target your corporate accountants and auditors
    std::vector<int32> listeners = m_db->GetCorpAccountants(corpID);

    NotificationFactory::Dispatch(executiveID, Notify::Types::WalletChange, data.GetString(), listeners);
}

***********************************/

