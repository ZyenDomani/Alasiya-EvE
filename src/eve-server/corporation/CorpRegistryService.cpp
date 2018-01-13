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
    Author:     Zhur, Allan
*/

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "cache/ObjCacheService.h"
#include "corporation/CorpRegistryService.h"
#include "corporation/CorpRegistryBound.h"

/*
 * CORP__ERROR
 * CORP__WARNING
 * CORP__INFO
 * CORP__MESSAGE
 * CORP__TRACE
 * CORP__CALL
 * CORP__CALL_DUMP
 * CORP__RSP_DUMP
 * CORP__DB_ERROR
 * CORP__DB_WARNING
 * CORP__DB_INFO
 * CORP__DB_MESSAGE
 */


PyCallable_Make_InnerDispatcher(CorpRegistryService)

CorpRegistryService::CorpRegistryService(PyServiceMgr *mgr)
: PyService(mgr, "corpRegistry"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    /** @note: all of these are skeleton code only */
    PyCallable_REG_CALL(CorpRegistryService, GetSuggestedAllianceShortNames);
    PyCallable_REG_CALL(CorpRegistryService, CreateAlliance);
    PyCallable_REG_CALL(CorpRegistryService, ApplyToJoinAlliance);
    PyCallable_REG_CALL(CorpRegistryService, GetAllianceApplications);
    PyCallable_REG_CALL(CorpRegistryService, DeleteAllianceApplication);
    PyCallable_REG_CALL(CorpRegistryService, GetRecentKillsAndLosses);
    PyCallable_REG_CALL(CorpRegistryService, GetCorporateContacts);
    PyCallable_REG_CALL(CorpRegistryService, AddCorporateContact);
    PyCallable_REG_CALL(CorpRegistryService, EditCorporateContact);
    PyCallable_REG_CALL(CorpRegistryService, RemoveCorporateContacts);
    PyCallable_REG_CALL(CorpRegistryService, EditContactsRelationshipID);
    PyCallable_REG_CALL(CorpRegistryService, GetLabels);
    PyCallable_REG_CALL(CorpRegistryService, CreateLabel);
    PyCallable_REG_CALL(CorpRegistryService, DeleteLabel);
    PyCallable_REG_CALL(CorpRegistryService, EditLabel);
    PyCallable_REG_CALL(CorpRegistryService, AssignLabels);
    PyCallable_REG_CALL(CorpRegistryService, RemoveLabels);
    PyCallable_REG_CALL(CorpRegistryService, ResignFromCEO);
}

CorpRegistryService::~CorpRegistryService() {
    delete m_dispatch;
}

PyBoundObject* CorpRegistryService::_CreateBoundObject( Client* pClient, const PyRep* bind_args )
{
    if (!bind_args->IsTuple()){
        sLog.Error( "CorpRegistryService::_CreateBoundObject", "%s: bind_args is not tuple: '%s'. ", pClient->GetName(), bind_args->TypeString() );
        pClient->SendErrorMsg("Could not bind object for Corp Registry.  Ref: ServerError 02808.");
        return nullptr;
    }

    return new CorpRegistryBound(m_manager, m_db, bind_args->AsTuple()->GetItem(0)->AsInt()->value());
}

PyResult CorpRegistryService::Handle_GetCorporateContacts(PyCallArgs &call)
{
    return m_db.GetContacts(call.client->GetCorporationID());
}

/**     ***********************************************************************
 * @note   these below are not coded or partially coded
 */

PyResult CorpRegistryService::Handle_ResignFromCEO(PyCallArgs &call) {
    //    self.GetCorpRegistry().ResignFromCEO(newCeoID)
    sLog.White( "CorpRegistryService::Handle_ResignFromCEO()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryService::Handle_GetSuggestedAllianceShortNames(PyCallArgs &call) {
    sLog.White( "CorpRegistryService::Handle_GetSuggestedAllianceShortNames()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryService::Handle_CreateAlliance(PyCallArgs &call) {
    sLog.White( "CorpRegistryService::Handle_CreateAlliance()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryService::Handle_ApplyToJoinAlliance(PyCallArgs &call) {
    sLog.White( "CorpRegistryService::Handle_ApplyToJoinAlliance()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryService::Handle_GetAllianceApplications(PyCallArgs &call) {
    sLog.White( "CorpRegistryService::Handle_GetAllianceApplications()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryService::Handle_DeleteAllianceApplication(PyCallArgs &call) {
    sLog.White( "CorpRegistryService::Handle_DeleteAllianceApplication()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}

PyResult CorpRegistryService::Handle_GetRecentKillsAndLosses(PyCallArgs &call)
{
    sLog.White( "CorpRegistryService::Handle_GetRecentKillsAndLosses()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return nullptr;
}


PyResult CorpRegistryService::Handle_AddCorporateContact(PyCallArgs &call) {
    sLog.White( "CorpRegistryService::Handle_AddCorporateContact()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

 /*    def AddCorporateContact(self, contactID, relationshipID):
 *        self.GetCorpRegistry().AddCorporateContact(contactID, relationshipID)
 */
    return nullptr;
}

PyResult CorpRegistryService::Handle_EditCorporateContact(PyCallArgs &call) {
    sLog.White( "CorpRegistryService::Handle_EditCorporateContact)", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

 /*    def EditCorporateContact(self, contactID, relationshipID):
 *        self.GetCorpRegistry().EditCorporateContact(contactID, relationshipID)
 */
    return nullptr;
}

PyResult CorpRegistryService::Handle_RemoveCorporateContacts(PyCallArgs &call) {
    sLog.White( "CorpRegistryService::Handle_RemoveCorporateContacts()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

 /*    def RemoveCorporateContacts(self, contactIDs):
 *        self.GetCorpRegistry().RemoveCorporateContacts(contactIDs)
 */
    return nullptr;
}

PyResult CorpRegistryService::Handle_EditContactsRelationshipID(PyCallArgs &call) {
    sLog.White( "CorpRegistryService::Handle_EditContactsRelationshipID()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

 /*    def EditContactsRelationshipID(self, contactIDs, relationshipID):
 *        self.GetCorpRegistry().EditContactsRelationshipID(contactIDs, relationshipID)
 */
    return nullptr;
}

PyResult CorpRegistryService::Handle_GetLabels(PyCallArgs &call) {
    sLog.White( "CorpRegistryService::Handle_GetLabels()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

    return m_db.GetLabels(call.client->GetCorporationID());
}

PyResult CorpRegistryService::Handle_CreateLabel(PyCallArgs &call) {
    sLog.White( "CorpRegistryService::Handle_CreateLabel()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

 /*    def CreateLabel(self, name, color = 0):
 *        return self.GetCorpRegistry().CreateLabel(name, color)
 */
    return nullptr;
}

PyResult CorpRegistryService::Handle_DeleteLabel(PyCallArgs &call) {
    sLog.White( "CorpRegistryService::Handle_DeleteLabel()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

 /*    def DeleteLabel(self, labelID):
 *        self.GetCorpRegistry().DeleteLabel(labelID)
 */
    return nullptr;
}

PyResult CorpRegistryService::Handle_EditLabel(PyCallArgs &call) {
    sLog.White( "CorpRegistryService::Handle_EditLabel()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

 /*    def EditLabel(self, labelID, name = None, color = None):
 *        self.GetCorpRegistry().EditLabel(labelID, name, color)
 */
    return nullptr;
}

PyResult CorpRegistryService::Handle_AssignLabels(PyCallArgs &call) {
    sLog.White( "CorpRegistryService::Handle_AssignLabels()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

 /*    def AssignLabels(self, contactIDs, labelMask):
 *        self.GetCorpRegistry().AssignLabels(contactIDs, labelMask)
 */
    return nullptr;
}

PyResult CorpRegistryService::Handle_RemoveLabels(PyCallArgs &call) {
    sLog.White( "CorpRegistryService::Handle_RemoveLabels()", "size= %u", call.tuple->size() );
    call.Dump(CORP__CALL_DUMP);

/*
 *    def RemoveLabels(self, contactIDs, labelMask):
 *        self.GetCorpRegistry().RemoveLabels(contactIDs, labelMask)
 */

    return nullptr;
}
