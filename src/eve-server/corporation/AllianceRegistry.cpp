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
    Author:        Allan
*/

//work in progress
/** @note  this is a bound object!  */

#include "eve-server.h"

#include "PyServiceCD.h"
#include "corporation/AllianceRegistry.h"

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

PyCallable_Make_InnerDispatcher(AllianceRegistry)

AllianceRegistry::AllianceRegistry(PyServiceMgr *mgr)
: PyService(mgr, "allianceRegistry"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(AllianceRegistry, GetAlliance);     //  bound object call
    PyCallable_REG_CALL(AllianceRegistry, GetRankedAlliances);
    PyCallable_REG_CALL(AllianceRegistry, GetAllianceApplications);
    PyCallable_REG_CALL(AllianceRegistry, GetEmploymentRecord);
    PyCallable_REG_CALL(AllianceRegistry, GetAllianceMembers);

    /*
    PyCallable_REG_CALL(CorpRegistryBound, CreateLabel);
    PyCallable_REG_CALL(CorpRegistryBound, GetLabels);
    PyCallable_REG_CALL(CorpRegistryBound, DeleteLabel);
    PyCallable_REG_CALL(CorpRegistryBound, EditLabel);
    PyCallable_REG_CALL(CorpRegistryBound, AssignLabels);
    PyCallable_REG_CALL(CorpRegistryBound, RemoveLabels);
    */

    /*
     bound objects
     self.members = self.GetMoniker().GetMembers()
     self.GetMoniker().DeclareExecutorSupport(corpID)
     self.GetMoniker().DeleteMember(corpID)
     self.applications = self.GetMoniker().GetApplications()
     return self.GetMoniker().UpdateApplication(corpID, applicationText, state)
     success = moniker.GetAlliance().AddToVoiceChat(vivoxChannelName)
     return self.GetMoniker().PayBill(billID, fromAccountKey)
     return self.GetMoniker().GetBillBalance(billID)
     return self.GetMoniker().GetBills()
     return self.GetMoniker().GetBillsReceivable()
     sm.GetService('alliance').GetMoniker().AddBulletin(title, body)
     self.bulletins = self.GetMoniker().GetBulletins()
     return self.GetMoniker().GetAllianceContacts()
     self.GetMoniker().AddAllianceContact(contactID, relationshipID)
     self.GetMoniker().EditAllianceContact(contactID, relationshipID)
     self.GetMoniker().RemoveAllianceContacts(contactIDs)
     self.GetMoniker().EditContactsRelationshipID(contactIDs, relationshipID)
     return self.GetMoniker().GetLabels()
     return self.GetMoniker().CreateLabel(name, color)
     self.GetMoniker().DeleteLabel(labelID)
     self.GetMoniker().EditLabel(labelID, name, color)
     self.GetMoniker().AssignLabels(contactIDs, labelMask)
     self.GetMoniker().RemoveLabels(contactIDs, labelMask)
     return self.GetMoniker().UpdateAlliance(description, url)
            */
}

AllianceRegistry::~AllianceRegistry()
{
    delete m_dispatch;
}

// this is the bind call.  do it like fleet
PyResult AllianceRegistry::Handle_GetAlliance(PyCallArgs &call) {
    //alliance = sm.RemoteSvc('allianceRegistry').GetAlliance(allianceID)
    sLog.White("AllianceRegistry", "Handle_GetAlliance() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);

    return nullptr;
}

PyResult AllianceRegistry::Handle_GetAllianceMembers(PyCallArgs &call) {
    // members = sm.RemoteSvc('allianceRegistry').GetAllianceMembers(itemID)  <-- returns dict of corpIDs
    sLog.White("AllianceRegistry", "Handle_GetAllianceMembers() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);

    return nullptr;
}

PyResult AllianceRegistry::Handle_GetRankedAlliances(PyCallArgs &call) {
    /*
            self.rankedAlliances.alliances = sm.RemoteSvc('allianceRegistry').GetRankedAlliances(maxLen)
            self.rankedAlliances.standings = {}
            for a in self.rankedAlliances.alliances:
                s = sm.GetService('standing').GetStanding(eve.session.corpid, a.allianceID)
                self.rankedAlliances.standings[a.allianceID] = s
         */

    sLog.White("AllianceRegistry", "Handle_GetRankedAlliances() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);

    return nullptr;
}

PyResult AllianceRegistry::Handle_GetAllianceApplications(PyCallArgs &call) {
    //
    sLog.White("AllianceRegistry", "Handle_GetAllianceApplications() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);

    return nullptr;
}

PyResult AllianceRegistry::Handle_GetEmploymentRecord(PyCallArgs &call) {
    //  allianceHistory = sm.RemoteSvc('allianceRegistry').GetEmploymentRecord(itemID)
    sLog.White("AllianceRegistry", "Handle_GetEmploymentRecord() size=%u", call.tuple->size() );
    call.Dump(ALLY__CALL_DUMP);

    return nullptr;
}

/*
PyResult AllianceRegistryBound::Handle_AddBulletin(PyCallArgs &call) {
    //  sm.GetService('alliance').GetMoniker().AddBulletin(title, body)
    //  sm.GetService('alliance').GetMoniker().AddBulletin(title, body, bulletinID=bulletinID, editDateTime=editDateTime)  <-- this is to update bulletin

    sLog.White( "CorpRegistryBound::Handle_AddBulletin()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    Call_AddBulletin args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    m_db.AddBulletin(*need list of corpIDs here*, m_allyID, call.client->GetCharacterID(), PyRep::StringContent(args.title), PyRep::StringContent(args.body));

    return nullptr;
}
*/

/*
PyResult CorpRegistryBound::Handle_GetLabels(PyCallArgs &call) {
    sLog.White("CorpRegistryBound", "Handle_GetLabels() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_CreateLabel(PyCallArgs &call) {
    // return self.GetCorpRegistry().CreateLabel(name, color)
    sLog.White("CorpRegistryBound", "Handle_CreateLabel() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_DeleteLabel(PyCallArgs &call) {
    // self.GetCorpRegistry().DeleteLabel(labelID)
    sLog.White("CorpRegistryBound", "Handle_DeleteLabel() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_EditLabel(PyCallArgs &call) {
    // self.GetCorpRegistry().EditLabel(labelID, name, color)
    sLog.White("CorpRegistryBound", "Handle_EditLabel() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_AssignLabels(PyCallArgs &call) {
    // self.GetCorpRegistry().AssignLabels(contactIDs, labelMask)
    sLog.White("CorpRegistryBound", "Handle_AssignLabels() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryBound::Handle_RemoveLabels(PyCallArgs &call) {
    // self.GetCorpRegistry().RemoveLabels(contactIDs, labelMask)
    sLog.White("CorpRegistryBound", "Handle_RemoveLabels() size=%u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}
*/
