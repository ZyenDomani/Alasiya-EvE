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
#include "cache/BulkDB.h"
#include "cache/BulkMgrService.h"
#include "packets/BulkDataPkts.h"

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
    /*
     * 23:08:27 W BulkMgrService::Handle_UpdateBulk(): size= 3
     * 23:08:27 [BulkDump]   Call Arguments:
     * 23:08:27 [BulkDump]       Tuple: 3 elements
     * 23:08:27 [BulkDump]         [ 0] Integer field: 65761                        <- changeID
     * 23:08:27 [BulkDump]         [ 1] String: '084a2e9a53ca5636e27b0c452c243437'  <- hashValue
     * 23:08:27 [BulkDump]         [ 2] Integer field: 3                            <- branch
     * 23:08:27 [BulkDump]   Call Named Arguments:
     * 23:08:27 [BulkDump]     Argument 'machoVersion':
     * 23:08:27 [BulkDump]         Integer field: 1
     *
     */
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
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        return nullptr;
    }

    //_log(BULKDATA__INFO, "BulkMgrService::Handle_UpdateBulk(): changeID: %u, branch: %u, hashValue: %s", args.changeID, args.branch, args.hashValue.c_str() );

    PyDict* res = new PyDict();
    // bulkDataChangeID found in eve-common/EVE_Defines.h and defines the serverVersion of this set of bulkdata
    if (1/*args.changeID == bulkDataChangeID*/) {
        res->SetItemString("type", new PyInt(updateBulkStatusOK));
    } else {
        // not right response, but easiest to hack, as it compares servers fileIDs to local fileIDs.
        // will change to 'updateBulkStatusNeedToUpdate' when i get the syntax figure out.
        res->SetItemString("type", new PyInt(updateBulkStatusHashMismatch));
        // make list of fileIDs to send to client.
        PyList* list = new PyList();
            list->AddItem(new PyInt(800001));
            list->AddItem(new PyInt(800002));
            list->AddItem(new PyInt(800003));
            list->AddItem(new PyInt(800004));
            list->AddItem(new PyInt(800005));
            list->AddItem(new PyInt(800006));
            list->AddItem(new PyInt(800007));
        res->SetItemString("data", list);
        res->SetItemString("version", new PyInt(bulkDataChangeID));
    }

    //bulkDataBranch

    res->SetItemString("allowUnsubmitted", new PyBool(false));

    /*
    res->SetItemString("data", new PyList(0));
        data is PyList of fileIDs when 'type' =  updateBulkStatusHashMismatch
        data is PyDict of 'chunkCount','chunk','changedTablesKeys','toBeDeleted','changedTablesKeys','branch' when 'type' = updateBulkStatusNeedToUpdate
    */
    return res;
}

PyResult BulkMgrService::Handle_GetVersion(PyCallArgs &call)
{
    // changeID, branch = self.bulkMgr.GetVersion()

    sLog.White( "BulkMgrService::Handle_GetVersion()", "size= %u", call.tuple->size() );
    call.Dump(BULKDATA__DUMP);
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyInt(bulkDataChangeID));
        tuple->SetItem(1, new PyInt(bulkDataBranch));
    return tuple;
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
    Call_GetChunk args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        return nullptr;
    }
    args.changeID;
    args.chunkNumber;

    return new PyNone();
}

PyResult BulkMgrService::Handle_GetAllBulkIDs(PyCallArgs &call)
{
    sLog.White( "BulkMgrService::Handle_GetAllBulkIDs()", "size= %u", call.tuple->size() );
    call.Dump(BULKDATA__DUMP);
    /*
    serverBulkIDs = self.bulkMgr.GetAllBulkIDs()
        PyList of fileIDs of updated data files to be sent to client in bulk
     */

    // hard-code a list of 'new' dgm fileIDs here.
    // this can also be used to update other data files as needed
    PyList* list = new PyList();
        list->AddItem(new PyInt(800001));   //cacheDogmaExpressionCategories
        list->AddItem(new PyInt(800002));   //cacheDogmaOperands
        list->AddItem(new PyInt(800003));   //cacheDogmaExpressions
        list->AddItem(new PyInt(800004));   //cacheDogmaAttributes
        list->AddItem(new PyInt(800005));   //cacheDogmaEffects
        list->AddItem(new PyInt(800006));   //cacheDogmaTypeAttributes
        list->AddItem(new PyInt(800007));   //cacheDogmaTypeEffects
    return list;
}

PyResult BulkMgrService::Handle_GetFullFiles(PyCallArgs &call)
{
    /*)
     * 00:29:16 W BulkMgrService::Handle_GetFullFiles(): size= 1
     * 00:29:16 [BulkDump]   Call Arguments:
     * 00:29:16 [BulkDump]       Tuple: 1 elements
     * 00:29:16 [BulkDump]         [ 0] List: 2 elements
     * 00:29:16 [BulkDump]         [ 0]   [ 0] Integer field: 800001
     * 00:29:16 [BulkDump]         [ 0]   [ 1] Integer field: 800002
     */
    sLog.White( "BulkMgrService::Handle_GetFullFiles()", "size= %u", call.tuple->size() );
    call.Dump(BULKDATA__DUMP);
    /*
        toBeChanged, bulksEndingInChunk, numberOfChunks, chunkSetID, self.allowUnsubmitted = self.bulkMgr.GetFullFiles(toGet)

        -- toGet is sent as PyList of fileIDs server should send back

    -- response
    [PyTuple 1 items]
      [PySubStream 151253 bytes]
        [PyTuple 5 items]
          [PyDict 3 kvp]        << toBeChanged
            [PyInt 800001]      << fileID
            [PyObjectEx Type2]  << file data
              [PyTuple 2 items]
                [PyTuple 1 items]
                  [PyToken dbutil.CRowset]
                [PyDict 1 kvp]
                [PyString "header"]
          [PyList 2 items]      << bulksEndingInChunk
            [PyInt 800001]
            [PyInt 800002]
          [PyInt 197]           << numberOfChunks
          [PyInt 0]             << chunkSetID
          [PyBool False]        << allowUnsubmitted
          */
    Call_SingleIntList args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        return nullptr;
    }
    if (args.ints.size() < 1)
        ; // toGet = null.  this means get all bulkdata files

    PyTuple* response = new PyTuple(5);
    PyDict* toBeChanged = new PyDict();
    // bulksEndingInChunk is populated when the last data of a file has been sent.
    //   each complete (or completed) data file's ID is put into this list.
    //   multiple files can be sent in this call, with their listIDs inserted into bulksEndingInChunk list.
    PyList* bulksEndingInChunk = new PyList();  // bulksEndingIn(this)Chunk

    response->SetItem(0, toBeChanged);
    response->SetItem(1, bulksEndingInChunk);
    response->SetItem(2, new PyInt(0)); //numberOfChunks
    response->SetItem(3, new PyInt(0)); //chunkSetID
    response->SetItem(4, new PyBool(false)); //allowUnsubmitted
    return response;
}

PyResult BulkMgrService::Handle_GetFullFilesChunk(PyCallArgs &call)
{
    sLog.White( "BulkMgrService::Handle_GetFullFilesChunk()", "size= %u", call.tuple->size() );
    call.Dump(BULKDATA__DUMP);
    /*
        toBeChanged, bulksEndingInChunk = self.bulkMgr.GetFullFilesChunk(chunkSetID, chunkNumber)
            this breaks files up into ?kb chunks for sending to client.  client requests "chunkSetID" and "chunkNumber", where chunkSetID is ???
            */
    Call_GetFullFilesChunk args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        return nullptr;
    }
    args.chunkNumber;
    args.chunkSetID;

    PyTuple* response = new PyTuple(2);
    PyDict* toBeChanged = new PyDict();
    PyList* bulksEndingInChunk = new PyList();

    response->SetItem(0, toBeChanged);
    response->SetItem(1, bulksEndingInChunk);
    return response;
}

PyResult BulkMgrService::Handle_GetUnsubmittedChunk(PyCallArgs &call)
{
    sLog.White( "BulkMgrService::Handle_GetUnsubmittedChunk()", "size= %u", call.tuple->size() );
    call.Dump(BULKDATA__DUMP);
    /*
                toBeChanged = self.bulkMgr.GetUnsubmittedChunk(chunkNumber)
     */
    Call_GetUnsubmittedChunk args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        return nullptr;
    }
    args.chunkNumber;

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
