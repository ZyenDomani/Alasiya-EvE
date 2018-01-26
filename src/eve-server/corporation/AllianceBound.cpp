
 /**
  * @name AllianceBound.cpp
  *     Alliance Bound code
  *
  * @Author:        Allan
  * @date:          16 January 2018
  */

#include <string>

#include "EVE_Corp.h"
#include "StaticDataMgr.h"
#include "account/AccountService.h"
#include "cache/ObjCacheService.h"
#include "chat/LSCService.h"
#include "corporation/AllianceBound.h"
#include "packets/CorporationPkts.h"
#include "station/StationDB.h"
#include "station/StationDataMgr.h"

/*
 * ALLY__ERROR
 * ALLY__WARNING
 * ALLY__INFO
 * ALLY__MESSAGE
 * ALLY__TRACE
 * ALLY__CALL
 * ALLY__CALL_DUMP
 * ALLY__RSP_DUMP
 */




AllianceBound::AllianceBound(PyServiceMgr *mgr)
: PyBoundObject(mgr),
m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    m_strBoundObjectName = "AllianceBound";

    PyCallable_REG_CALL(AllianceBound, GetAlliance);
    PyCallable_REG_CALL(AllianceBound, AddBulletin);
    PyCallable_REG_CALL(AllianceBound, GetBulletins);
    PyCallable_REG_CALL(AllianceBound, CreateLabel);
    PyCallable_REG_CALL(AllianceBound, GetLabels);
    PyCallable_REG_CALL(AllianceBound, DeleteLabel);
    PyCallable_REG_CALL(AllianceBound, EditLabel);
    PyCallable_REG_CALL(AllianceBound, AssignLabels);
    PyCallable_REG_CALL(AllianceBound, RemoveLabels);
    PyCallable_REG_CALL(AllianceBound, GetMembers);
    PyCallable_REG_CALL(AllianceBound, DeclareExecutorSupport);
    PyCallable_REG_CALL(AllianceBound, DeleteMember);
    PyCallable_REG_CALL(AllianceBound, GetApplications);
    PyCallable_REG_CALL(AllianceBound, UpdateApplication);
    PyCallable_REG_CALL(AllianceBound, AddToVoiceChat);
    PyCallable_REG_CALL(AllianceBound, PayBill);
    PyCallable_REG_CALL(AllianceBound, GetBills);
    PyCallable_REG_CALL(AllianceBound, GetBillsReceivable);
    PyCallable_REG_CALL(AllianceBound, GetAllianceContacts);
    PyCallable_REG_CALL(AllianceBound, AddAllianceContact);
    PyCallable_REG_CALL(AllianceBound, EditAllianceContact);
    PyCallable_REG_CALL(AllianceBound, RemoveAllianceContacts);
    PyCallable_REG_CALL(AllianceBound, EditContactsRelationshipID);
    PyCallable_REG_CALL(AllianceBound, UpdateAlliance);

}


PyResult AllianceBound::Handle_GetAlliance(PyCallArgs &call) {
    sLog.White("AllianceBound", "Handle_GetAlliance() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);

    return nullptr;
}

PyResult AllianceBound::Handle_GetMembers(PyCallArgs &call) {
    //   self.members = self.GetMoniker().GetMembers()
    sLog.White("AllianceBound", "Handle_GetMembers() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_DeclareExecutorSupport(PyCallArgs &call) {
    //   self.GetMoniker().DeclareExecutorSupport(corpID)
    sLog.White("AllianceBound", "Handle_DeclareExecutorSupport() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_DeleteMember(PyCallArgs &call) {
    //  self.GetMoniker().DeleteMember(corpID)
    sLog.White("AllianceBound", "Handle_DeleteMember() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_GetApplications(PyCallArgs &call) {
    //   self.applications = self.GetMoniker().GetApplications()
    sLog.White("AllianceBound", "Handle_GetApplications() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_UpdateApplication(PyCallArgs &call) {
    //    return self.GetMoniker().UpdateApplication(corpID, applicationText, state)
    sLog.White("AllianceBound", "Handle_UpdateApplication() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_AddToVoiceChat(PyCallArgs &call) {
    //    success = moniker.GetAlliance().AddToVoiceChat(vivoxChannelName)
    sLog.White("AllianceBound", "Handle_AddToVoiceChat() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_PayBill(PyCallArgs &call) {
    //   return self.GetMoniker().PayBill(billID, fromAccountKey)
    sLog.White("AllianceBound", "Handle_PayBill() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_GetBillBalance(PyCallArgs &call) {
    //   return self.GetMoniker().GetBillBalance(billID)
    sLog.White("AllianceBound", "Handle_GetBillBalance() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_GetBills(PyCallArgs &call) {
    //   return self.GetMoniker().GetBills()
    sLog.White("AllianceBound", "Handle_GetBills() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_GetBillsReceivable(PyCallArgs &call) {
    //   return self.GetMoniker().GetBillsReceivable()
    sLog.White("AllianceBound", "Handle_GetBillsReceivable() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_AddBulletin(PyCallArgs &call) {
    //   sm.GetService('alliance').GetMoniker().AddBulletin(title, body)
    //  sm.GetService('alliance').GetMoniker().AddBulletin(title, body, bulletinID=bulletinID, editDateTime=editDateTime)  <-- this is to update bulletin
    sLog.White("AllianceBound", "Handle_AddBulletin() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);

    Call_AddBulletin args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    //m_db.AddBulletin(*need list of corpIDs here*, m_allyID, call.client->GetCharacterID(), PyRep::StringContent(args.title), PyRep::StringContent(args.body));

    return nullptr;
}

PyResult AllianceBound::Handle_GetBulletins(PyCallArgs &call) {
    //   self.bulletins = self.GetMoniker().GetBulletins()
    sLog.White("AllianceBound", "Handle_GetBulletins() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_GetAllianceContacts(PyCallArgs &call) {
    //    return self.GetMoniker().GetAllianceContacts()
    sLog.White("AllianceBound", "Handle_GetAllianceContacts() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_AddAllianceContact(PyCallArgs &call) {
    //   self.GetMoniker().AddAllianceContact(contactID, relationshipID)
    sLog.White("AllianceBound", "Handle_AddAllianceContact() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_EditAllianceContact(PyCallArgs &call) {
    //   self.GetMoniker().EditAllianceContact(contactID, relationshipID)
    sLog.White("AllianceBound", "Handle_EditAllianceContact() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_RemoveAllianceContacts(PyCallArgs &call) {
    //   self.GetMoniker().RemoveAllianceContacts(contactIDs)
    sLog.White("AllianceBound", "Handle_RemoveAllianceContacts() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_EditContactsRelationshipID(PyCallArgs &call) {
    //    self.GetMoniker().EditContactsRelationshipID(contactIDs, relationshipID)
    sLog.White("AllianceBound", "Handle_EditContactsRelationshipID() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_GetLabels(PyCallArgs &call) {
    //   return self.GetMoniker().GetLabels()
    sLog.White("AllianceBound", "Handle_GetLabels() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_CreateLabel(PyCallArgs &call) {
    //   return self.GetMoniker().CreateLabel(name, color)
    sLog.White("AllianceBound", "Handle_CreateLabel() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_DeleteLabel(PyCallArgs &call) {
    //   self.GetMoniker().DeleteLabel(labelID)
    sLog.White("AllianceBound", "Handle_DeleteLabel() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_EditLabel(PyCallArgs &call) {
    //   self.GetMoniker().EditLabel(labelID, name, color)
    sLog.White("AllianceBound", "Handle_EditLabel() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_AssignLabels(PyCallArgs &call) {
    //   self.GetMoniker().AssignLabels(contactIDs, labelMask)
    sLog.White("AllianceBound", "Handle_AssignLabels() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_RemoveLabels(PyCallArgs &call) {
    //   self.GetMoniker().RemoveLabels(contactIDs, labelMask)
    sLog.White("AllianceBound", "Handle_RemoveLabels() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}

PyResult AllianceBound::Handle_UpdateAlliance(PyCallArgs &call) {
    //    return self.GetMoniker().UpdateAlliance(description, url)
    sLog.White("AllianceBound", "Handle_UpdateAlliance() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);
    return nullptr;
}
