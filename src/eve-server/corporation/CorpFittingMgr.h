/*
 *
 */

#ifndef EVE_COPORATION_FITTING_MGR_H
#define EVE_COPORATION_FITTING_MGR_H

#include "PyService.h"

class CorpFittingMgr: public PyService
{
public:
    CorpFittingMgr(PyServiceMgr *mgr);
    virtual ~CorpFittingMgr();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(GetFittings);

};

#endif  // EVE_COPORATION_FITTING_MGR_H
