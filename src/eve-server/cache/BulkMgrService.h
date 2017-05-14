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

#ifndef __BULKMGR_SERVICE_H_INCL__
#define __BULKMGR_SERVICE_H_INCL__

#include "PyService.h"
#include "cache/BulkDB.h"

class BulkMgrService : public PyService
{
public:
    BulkMgrService(PyServiceMgr *mgr);
    virtual ~BulkMgrService();
public:

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(UpdateBulk);
    PyCallable_DECL_CALL(GetAllBulkIDs);
    PyCallable_DECL_CALL(GetVersion);
    PyCallable_DECL_CALL(GetChunk);
    PyCallable_DECL_CALL(GetFullFiles);
    PyCallable_DECL_CALL(GetFullFilesChunk);
    PyCallable_DECL_CALL(GetUnsubmittedChanges);
    PyCallable_DECL_CALL(GetUnsubmittedChunk);

private:

    enum bulkStatus {
        updateBulkStatusOK               = 0,   // client data == server data  - no change
        updateBulkStatusWrongBranch      = 1,   // client != server.  calls GetFullFiles then GetVersion
        updateBulkStatusHashMismatch     = 2,   // client missing files  - compares server (returned) fileIDs with local fileIDs
        updateBulkStatusClientNewer      = 3,   // client != server.  calls GetFullFiles then GetVersion
        updateBulkStatusNeedToUpdate     = 4,   // this one will be complicated.  see notes in cpp
        updateBulkStatusTooManyRevisions = 5    // server has too many updates to bring client files up-to-date.  calls GetFullFiles then GetVersion
    };
};

#endif

