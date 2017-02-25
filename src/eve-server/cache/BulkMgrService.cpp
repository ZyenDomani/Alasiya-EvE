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
    Author:        ozatomic (hacked for static client data)
    Updates:    Allan (added calls and (hacked) updates for new dgm data)
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "cache/BulkMgrService.h"

PyCallable_Make_InnerDispatcher(BulkMgrService)

BulkMgrService::BulkMgrService( PyServiceMgr *mgr )
: PyService(mgr, "bulkMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(BulkMgrService, GetChunk);
    PyCallable_REG_CALL(BulkMgrService, UpdateBulk);
    PyCallable_REG_CALL(BulkMgrService, GetVersion);
    PyCallable_REG_CALL(BulkMgrService, GetFullFiles);
    PyCallable_REG_CALL(BulkMgrService, GetAllBulkIDs);
    PyCallable_REG_CALL(BulkMgrService, GetFullFilesChunk);
    PyCallable_REG_CALL(BulkMgrService, GetUnsubmittedChunk);
    PyCallable_REG_CALL(BulkMgrService, GetUnsubmittedChanges);

}

BulkMgrService::~BulkMgrService() {
    delete m_dispatch;
}
/*
BULKDATA__ERROR=1
BULKDATA__WARNING=0
BULKDATA__MESSAGE=0
BULKDATA__DEBUG=0
BULKDATA__INFO=0
BULKDATA__TRACE=0
BULKDATA__DUMP=0
*/
PyResult BulkMgrService::Handle_UpdateBulk(PyCallArgs &call)
{
    sLog.White( "BulkMgrService::Handle_UpdateBulk()", "size= %u", call.tuple->size() );
    call.Dump(BULKDATA__DUMP);
    /*

    updateData = self.bulkMgr.UpdateBulk(changeID, hashValue, branch)

        updateType = updateData['type']
        self.allowUnsubmitted = updateData['allowUnsubmitted']
        if 'version' in updateData:
            serverVersion = updateData['version']
        if 'data' in updateData:        -- list of bulkdata fileID 'numbers' that have changed.
            updateInfo = updateData['data']


    */
    Call_UpdateBulk args;
    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "Invalid arguments");
	return NULL;
    }
    /*
    args.changeID;
    args.hashValue;
    args.branch;
    */

    _log(BULKDATA__INFO, "BulkMgrService::Handle_UpdateBulk(): changeID: %u, branch: %u, hashValue: %s", args.changeID, args.branch, args.hashValue.c_str() );

    PyDict* test = new PyDict();
    test->SetItemString("type", new PyInt(updateBulkStatusOK));
    test->SetItemString("allowUnsubmitted", new PyBool(false));
    /*
    test->SetItemString("version", new PyInt(0));
    version is used when 'type' = updateBulkStatusNeedToUpdate

    test->SetItemString("data", new PyList(0));
        data is PyList of fileIDs when 'type' =  updateBulkStatusHashMismatch
        data is PyDict of 'chunkCount','chunk','changedTablesKeys','toBeDeleted','changedTablesKeys','branch' when 'type' = updateBulkStatusNeedToUpdate
    */
    return test;
}

PyResult BulkMgrService::Handle_GetChunk(PyCallArgs &call)
{
    sLog.White( "BulkMgrService::Handle_GetChunk()", "size= %u", call.tuple->size() );
    call.Dump(BULKDATA__DUMP);
    /*
    toBeChanged = self.bulkMgr.GetChunk(changeID, chunkNumber)
    changeID is from GetVersion()
    chunkNumber is incremented during loop when bulkdata return 'type' =  updateBulkStatusNeedToUpdate
     */
    return new PyNone();
}

PyResult BulkMgrService::Handle_GetVersion(PyCallArgs &call)
{
    sLog.White( "BulkMgrService::Handle_GetVersion()", "size= %u", call.tuple->size() );
    call.Dump(BULKDATA__DUMP);
    /*
    serverChangeID, branch = self.bulkMgr.GetVersion()
     */
    return new PyNone();
}

PyResult BulkMgrService::Handle_GetAllBulkIDs(PyCallArgs &call)
{
    sLog.White( "BulkMgrService::Handle_GetAllBulkIDs()", "size= %u", call.tuple->size() );
    call.Dump(BULKDATA__DUMP);
    /*
    serverBulkIDs = self.bulkMgr.GetAllBulkIDs()
        PyList of server-cached bulkdata files (will need to get their IDs from typedefs.h)
     */

    // hard-code a list of 'new' dgm fileIDs here.
    // this can also be used to update other data files as needed
    return new PyNone();
}

PyResult BulkMgrService::Handle_GetFullFiles(PyCallArgs &call)
{
    sLog.White( "BulkMgrService::Handle_GetFullFiles()", "size= %u", call.tuple->size() );
    call.Dump(BULKDATA__DUMP);
    /*
        toBeChanged, bulksEndingInChunk, numberOfChunks, chunkSetID, self.allowUnsubmitted = self.bulkMgr.GetFullFiles(toGet)

        -- toGet is sent as PyList of fileIDs server should send back
     */
    return new PyNone();
}

PyResult BulkMgrService::Handle_GetFullFilesChunk(PyCallArgs &call)
{
    sLog.White( "BulkMgrService::Handle_GetFullFilesChunk()", "size= %u", call.tuple->size() );
    call.Dump(BULKDATA__DUMP);
    /*
        toBeChanged, bulksEndingInChunk = self.bulkMgr.GetFullFilesChunk(chunkSetID, chunkNumber)
            this breaks files up into ?kb chunks for sending to client.  client requests "chunkSetID" and "chunkNumber", where chunkSetID is the fileID
     */
    return new PyNone();
}

PyResult BulkMgrService::Handle_GetUnsubmittedChunk(PyCallArgs &call)
{
    sLog.White( "BulkMgrService::Handle_GetUnsubmittedChunk()", "size= %u", call.tuple->size() );
    call.Dump(BULKDATA__DUMP);
    /*
                toBeChanged = self.bulkMgr.GetUnsubmittedChunk(chunkNumber)
     */
    return new PyNone();
}

PyResult BulkMgrService::Handle_GetUnsubmittedChanges(PyCallArgs &call)
{
    sLog.White( "BulkMgrService::Handle_GetUnsubmittedChanges()", "size= %u", call.tuple->size() );
    call.Dump(BULKDATA__DUMP);
    /*
        unsubmitted = self.bulkMgr.GetUnsubmittedChanges()
        PyDict of 'toBeChanged','toBeDeleted','changedTablesKeys','chunkCount'
          this one is complicated.  will need work if we're allowing unsubmitted (whatever that means)
     */
    return new PyNone();
}
