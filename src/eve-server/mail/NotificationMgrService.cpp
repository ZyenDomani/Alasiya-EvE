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
    Author:     caytchen
    Update:     Allan & Gemini
*/

#include "../eve-server.h"

#include "PyServiceCD.h"
#include "../EntityMgr.h"

#include "mail/MailDB.h"
#include "mail/NotificationMgrService.h"

PyCallable_Make_InnerDispatcher(NotificationMgrService)

// this service is part of mail and used with the 'notifications' tab of mail window

NotificationMgrService::NotificationMgrService(PyServiceMgr* mgr)
: PyService(mgr, "notificationMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(NotificationMgrService, GetByGroupID);
    PyCallable_REG_CALL(NotificationMgrService, GetUnprocessed);
    PyCallable_REG_CALL(NotificationMgrService, MarkGroupAsProcessed);
    PyCallable_REG_CALL(NotificationMgrService, MarkAllAsProcessed);
    PyCallable_REG_CALL(NotificationMgrService, MarkAsProcessed);
    PyCallable_REG_CALL(NotificationMgrService, DeleteGroupNotifications);
    PyCallable_REG_CALL(NotificationMgrService, DeleteAllNotifications);
    PyCallable_REG_CALL(NotificationMgrService, DeleteNotifications);
}

NotificationMgrService::~NotificationMgrService() {
    delete m_dispatch;
}

PyResult NotificationMgrService::Handle_GetByGroupID(PyCallArgs &call) {
    SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode GetByGroupID arguments.", GetName());
        return nullptr;
    }

    int32 groupID = args.arg;
    int32 characterID = call.client->GetCharacterID();

    sLog.Debug("NotificationMgrService", "Character %i clicked tab GroupID: %i", characterID, groupID);

    PyObject* rowsetResult = MailDB::GetNotificationsByGroup(characterID, groupID);
    if (!rowsetResult) {
        return new PyTuple(0); // Safely return empty tuple instead of nullptr to prevent client exceptions
    }

    return rowsetResult;
}

PyResult NotificationMgrService::Handle_GetUnprocessed(PyCallArgs &call) {
    int32 characterID = call.client->GetCharacterID();

    // Client invokes this to count or load notifications where processed = 0
    PyObject* rowsetResult = MailDB::GetUnprocessedNotifications(characterID);
    if (!rowsetResult) {
        return new PyTuple(0);
    }
    return rowsetResult;
}

PyResult NotificationMgrService::Handle_MarkAsProcessed(PyCallArgs &call) {
    // MarkAsProcessed(notificationIDs) -> Expects a PyList of IDs
    PyList* notificationIDs = call.tuple->GetItem(0)->AsList();
    int32 characterID = call.client->GetCharacterID();

    std::vector<int32> targets;
    size_t size = notificationIDs->size();
    for (int i = 0; i < size; ++i) {
        targets.push_back(PyRep::IntegerValueI32(notificationIDs->GetItem(i)));
    }

    if (!targets.empty()) {
        MailDB::UpdateNotificationProcessedState(characterID, targets, 1); // 1 = Read/Processed
    }
    return PyStatic.NewNone();
}

PyResult NotificationMgrService::Handle_MarkGroupAsProcessed(PyCallArgs &call) {
    // MarkGroupAsProcessed(groupID)
    int32 groupID = PyRep::IntegerValueI32(call.tuple->GetItem(0));
    int32 characterID = call.client->GetCharacterID();

    MailDB::UpdateGroupProcessedState(characterID, groupID, 1);
    return PyStatic.NewNone();
}

PyResult NotificationMgrService::Handle_MarkAllAsProcessed(PyCallArgs &call) {
    int32 characterID = call.client->GetCharacterID();

    MailDB::UpdateAllProcessedState(characterID, 1);
    return PyStatic.NewNone();
}

PyResult NotificationMgrService::Handle_DeleteNotifications(PyCallArgs &call) {
    // DeleteNotifications(notificationIDs) -> Soft-delete from timeline view
    PyList* notificationIDs = call.tuple->GetItem(0)->AsList();
    int32 characterID = call.client->GetCharacterID();

    std::vector<int32> targets;
    size_t size = notificationIDs->size();
    for (int i = 0; i < size; ++i) {
        targets.push_back(PyRep::IntegerValueI32(notificationIDs->GetItem(i)));
    }

    if (!targets.empty()) {
        MailDB::UpdateNotificationDeletedState(characterID, targets, 1); // 1 = Soft-deleted
    }
    return PyStatic.NewNone();
}

PyResult NotificationMgrService::Handle_DeleteGroupNotifications(PyCallArgs &call) {
    int32 groupID = PyRep::IntegerValueI32(call.tuple->GetItem(0));
    int32 characterID = call.client->GetCharacterID();

    MailDB::UpdateGroupDeletedState(characterID, groupID, 1);
    return PyStatic.NewNone();
}

PyResult NotificationMgrService::Handle_DeleteAllNotifications(PyCallArgs &call) {
    int32 characterID = call.client->GetCharacterID();

    MailDB::UpdateAllDeletedState(characterID, 1);
    return PyStatic.NewNone();
}

void NotificationFactory::Dispatch(int32 senderID, int32 typeID, const std::string& serializedData, const std::vector< int32 >& receiverIDs) {
    if (receiverIDs.empty())
        return;

    int64 currentWinTime = GetFileTimeNow(); // Your centralized int64 time track
    std::string escData;
    sDatabase.DoEscapeString(escData, serializedData);

    // 1. Bulk DB Archive Write (Atomic Transaction to prevent thread locking)
    DBQueryResult res;
    sDatabase.RunQuery(res, "START TRANSACTION");
    for (int32 receiverID : receiverIDs) {
        sDatabase.RunQuery(res,
                           "INSERT INTO eveNotifications (typeID, senderID, receiverID, processed, created, data, deleted) "
                           "VALUES (%d, %d, %d, 0, %I64d, '%s', 0)",
                           typeID, senderID, receiverID, currentWinTime, escData.c_str()
        );
    }
    sDatabase.RunQuery(res, "COMMIT");

    // 2. Build the Real-Time Event Packet Payload
    // Crucible expects a precise row tuple layout: [notificationID, typeID, senderID, created, data]
    PyTuple* realTimePayload = new PyTuple(5);
    realTimePayload->SetItem(0, new PyInt(0));                    // 0 handles transient notifications safely
    realTimePayload->SetItem(1, new PyInt(typeID));               // typeID enum
    realTimePayload->SetItem(2, new PyInt(senderID));             // senderID entity context
    realTimePayload->SetItem(3, new PyLong(currentWinTime));      // created time tracker (int64)
    realTimePayload->SetItem(4, new PyString(serializedData));    // YAML textual configuration string

    /* this needs more work...
    // 3. Assemble target roster for network distribution
    character_set activeOnlineRoster;
    for (int32 receiverID : receiverIDs) {
        // Only add players currently authenticated on the node to minimize memory footprints
        if (sSessionMgr.IsClientOnline(receiverID)) {
            activeOnlineRoster.insert(receiverID);
        }
    }

    // 4. Fire the shared zero-clone multicast alert natively!
    if (!activeOnlineRoster.empty()) {
        sEntityMgr.Multicast("notificationSvc", "clientID", &realTimePayload, false);
    }
    */

    PySafeDecRef(realTimePayload); // Clean up master reference pointer allocations
}
