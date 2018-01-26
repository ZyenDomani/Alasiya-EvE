/*
 *
 *
 */


#ifndef EVE_CHARACTER_FITTING_MGR_H
#define EVE_CHARACTER_FITTING_MGR_H

#include "PyService.h"

class CharFittingMgr: public PyService
{
public:
    CharFittingMgr(PyServiceMgr *mgr);
    virtual ~CharFittingMgr();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(GetFittings);

};

#endif  // EVE_CHARACTER_FITTING_MGR_H