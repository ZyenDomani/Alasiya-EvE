/*
 *
 *
 */

//work in progress


#include "eve-server.h"

#include "PyServiceCD.h"
#include "corporation/CorpFittingMgr.h"

PyCallable_Make_InnerDispatcher(CorpFittingMgr)

CorpFittingMgr::CorpFittingMgr(PyServiceMgr *mgr)
: PyService(mgr, "corpFittingMgr"),
m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(CorpFittingMgr, GetFittings);
}

CorpFittingMgr::~CorpFittingMgr() {
    delete m_dispatch;
}

PyResult CorpFittingMgr::Handle_GetFittings(PyCallArgs &call) {

    sLog.White( "CorpFittingMgr::Handle_GetFittings()", "size= %u from '%s'", call.tuple->size(), call.client->GetName() );
    call.Dump(SERVICE__CALL_DUMP);


    return nullptr;
}

