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
    Author:        ozatomic
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

    PyCallable_REG_CALL(BulkMgrService, UpdateBulk);
}

BulkMgrService::~BulkMgrService() {
    delete m_dispatch;
}

PyResult BulkMgrService::Handle_UpdateBulk(PyCallArgs &call)
{
    Call_UpdateBulk args;
    if(!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "Invalid arguments");
	return NULL;
    }

    PyDict* test = new PyDict();
    test->SetItemString("type", new PyInt(updateBulkStatusOK));
    test->SetItemString("allowUnsubmitted", new PyBool(false));

    return test;
}
/*


==================== Sent from Client 352 bytes [Compressed]

[PyObjectData Name: macho.CallReq]
  [PyTuple 7 items]
    [PyInt 6]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 2]
        [PyInt 0]
        [PyIntegerVar 5]
        [PyNone]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 1]
        [PyInt 810144]
        [PyString "bulkMgr"]
        [PyNone]
    [PyInt 5654387]
    [PyTuple 1 items]
      [PyTuple 2 items]
        [PyInt 0]
        [PySubStream 435 bytes]
          [PyTuple 4 items]
            [PyInt 1]
            [PyString "GetFullFiles"]
            [PyTuple 1 items]
              [PyList 81 items]
                [PyInt 2002600004]
                [PyInt 2002400001]
                [PyInt 2001600002]
                [PyInt 2001600003]
                [PyInt 2002400004]
                [PyInt 2002400005]
                [PyInt 2001600006]
                [PyInt 2001600007]
                [PyInt 800003]
                [PyInt 800009]
                [PyInt 2000001]
                [PyInt 3200011]
                [PyInt 600005]
                [PyInt 2001800002]
                [PyInt 3200012]
                [PyInt 3200015]
                [PyInt 800005]
                [PyInt 1800007]
                [PyInt 2002400003]
                [PyInt 3200001]
                [PyInt 800004]
                [PyInt 1400010]
                [PyInt 2001600004]
                [PyInt 600002]
                [PyInt 2002600005]
                [PyInt 2001600005]
                [PyInt 1200001]
                [PyInt 2002500001]
                [PyInt 2002500002]
                [PyInt 7300003]
                [PyInt 7300004]
                [PyInt 2002500005]
                [PyInt 2002600001]
                [PyInt 1400002]
                [PyInt 800007]
                [PyInt 2209987]
                [PyInt 1400008]
                [PyInt 600001]
                [PyInt 1400009]
                [PyInt 1800004]
                [PyInt 600004]
                [PyInt 2002600010]
                [PyInt 6400004]
                [PyInt 2002200001]
                [PyInt 2002200002]
                [PyInt 2001700035]
                [PyInt 2001800004]
                [PyInt 2001800005]
                [PyInt 2002200006]
                [PyInt 600007]
                [PyInt 600008]
                [PyInt 2002200009]
                [PyInt 2002200010]
                [PyInt 2002200011]
                [PyInt 2002600012]
                [PyInt 2003100002]
                [PyInt 2209999]
                [PyInt 1400016]
                [PyInt 2002600011]
                [PyInt 2003100003]
                [PyInt 3200016]
                [PyInt 1800005]
                [PyInt 2800006]
                [PyInt 2002400002]
                [PyInt 1800001]
                [PyInt 7300005]
                [PyInt 1800003]
                [PyInt 2003100001]
                [PyInt 2001900002]
                [PyInt 2001900003]
                [PyInt 5100004]
                [PyInt 1400011]
                [PyInt 1800006]
                [PyInt 600010]
                [PyInt 800006]
                [PyInt 3200010]
                [PyInt 5100001]
                [PyInt 2002600002]
                [PyInt 3200002]
                [PyInt 600006]
                [PyInt 2809992]
            [PyDict 1 kvp]
              [PyString "machoVersion"]
              [PyInt 1]
    [PyNone]
    [PyNone]



==================== Sent from Server 57921 bytes [Compressed]

[PyObjectData Name: macho.CallRsp]
  [PyTuple 7 items]
    [PyInt 7]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 1]
        [PyInt 810144]
        [PyString "bulkMgr"]
        [PyNone]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 2]
        [PyIntegerVar 238691000002101]
        [PyIntegerVar 5]
        [PyNone]
    [PyInt 5654387]
    [PyTuple 1 items]
      [PySubStream 151253 bytes]
        [PyTuple 5 items]
          [PyDict 3 kvp]
            [PyInt 600001]
            [PyObjectEx Type2]
              [PyTuple 2 items]
                [PyTuple 1 items]
                  [PyToken dbutil.CRowset]
                [PyDict 1 kvp]
                  [PyString "header"]
                  [PyObjectEx Normal]
                    [PyTuple 2 items]
                      [PyToken blue.DBRowDescriptor]
                      [PyTuple 1 items]
                        [PyTuple 7 items]
                          [PyTuple 2 items]
                            [PyString "categoryID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "categoryName"]
                            [PyInt 130]
                          [PyTuple 2 items]
                            [PyString "description"]
                            [PyInt 130]
                          [PyTuple 2 items]
                            [PyString "published"]
                            [PyInt 11]
                          [PyTuple 2 items]
                            [PyString "iconID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "categoryNameID"]
                            [PyInt 3]
                          [PyTuple 2 items]
                            [PyString "dataID"]
                            [PyInt 3]
              [PyPackedRow 17 bytes]
                ["categoryID" => <0> [I4]]
                ["categoryName" => <23-53-79-73-74-65-6D> [WStr]]
                ["description" => <empty string> [WStr]]
                ["published" => <0> [Bool]]
                ["iconID" => <0> [I4]]
                ["categoryNameID" => <63539> [I4]]
                ["dataID" => <16545519> [I4]]
              [PyPackedRow 17 bytes]
                ["categoryID" => <1> [I4]]
                ["categoryName" => <Owner> [WStr]]
                ["description" => <empty string> [WStr]]
                ["published" => <0> [Bool]]
                ["iconID" => <0> [I4]]
                ["categoryNameID" => <63540> [I4]]
                ["dataID" => <16545520> [I4]]

                */