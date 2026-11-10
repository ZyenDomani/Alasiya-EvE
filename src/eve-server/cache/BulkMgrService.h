/**
 * @name BulkMgrService.h
 *   This file is for processing bulkdata
 *   Copyright 2017  Alasiya-EVEmu Team
 *
 * @Author:    ozatomic (hacked for static client data)
 * @Author:    Allan (added calls and (hacked) updates for new dgm data)
 * @date:      24 January 2017
 *
 */

#pragma once

#include "PyService.h"

// bulkdata version
static const int32  bulkDataBranch = 4;
//  bulkDataChangeID rddmmyyyy
//  where r=revision, dd=day, mm=month, yyyy=year
static const int32  bulkDataChangeID = 430012026;

namespace Bulk {
    namespace Status {
        enum {
            OK               = 0,   // client data == server data  - no change
            WrongBranch      = 1,   // client != server.  calls GetFullFiles then GetVersion
            HashMismatch     = 2,   // client missing files  - compares server (returned) fileIDs with local fileIDs
            ClientNewer      = 3,   // client != server.  calls GetFullFiles then GetVersion
            NeedToUpdate     = 4,   // this one will be complicated.  see notes in cpp
            TooManyRevisions = 5    // server has too many updates to bring client files up-to-date.  calls GetFullFiles then GetVersion
        };
    }
}

class BulkMgrService : public PyService
{
public:
    BulkMgrService(PyServiceMgr *mgr);
    virtual ~BulkMgrService();

    BulkMgrService(BulkMgrService&&) =delete;
    BulkMgrService(const BulkMgrService&) =delete;
    BulkMgrService& operator=(BulkMgrService&&) =delete;
    BulkMgrService& operator=(const BulkMgrService&) =delete;

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
    //PyRep* LoadBulkCacheFile( std::string& objName );

    std::map<uint32, PyRep*> m_bulkMemoryCache;
    //void PreloadBulkCache();

};
