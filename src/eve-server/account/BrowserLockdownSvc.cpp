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
    Author:        Captnoord
    Updates:    Allan
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "account/BrowserLockdownSvc.h"

// crap
PyCallable_Make_InnerDispatcher(BrowserLockdownService)

BrowserLockdownService::BrowserLockdownService( PyServiceMgr *mgr )
: PyService(mgr, "browserLockdownSvc"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(BrowserLockdownService, GetFlaggedSitesHash);
    PyCallable_REG_CALL(BrowserLockdownService, GetFlaggedSitesList);
    PyCallable_REG_CALL(BrowserLockdownService, GetDefaultHomePage);
    PyCallable_REG_CALL(BrowserLockdownService, IsBrowserInLockdown);
}

BrowserLockdownService::~BrowserLockdownService() {
    delete m_dispatch;
}

//02:40:18 L BrowserLockdownService::Handle_GetFlaggedSitesHash(): size= 0
PyResult BrowserLockdownService::Handle_GetFlaggedSitesHash(PyCallArgs &call)
{
    return new PyString("df98509e1e3f0dd839083e7be1d2b360");
}

//02:40:18 L BrowserLockdownService::Handle_GetFlaggedSitesList(): size= 0
PyResult BrowserLockdownService::Handle_GetFlaggedSitesList(PyCallArgs &call)
{
    return new PyNone();
}

PyResult BrowserLockdownService::Handle_GetDefaultHomePage(PyCallArgs &call) {
    /*
    [PyTuple 1 items]
      [PySubStream 134 bytes]
        [PyObjectData Name: objectCaching.CachedObject]
          [PyTuple 7 items]
            [PyTuple 2 items]
              [PyIntegerVar 129511422600825710]
              [PyInt 46587]
            [PyNone]
            [PyInt 704421]
            [PyInt 1]
            [PySubStream 34 bytes]
              [PyString "https://gate.eveonline.com/"]
            [PyInt 0]
            [PyTuple 3 items]
              [PyString "Method Call"]
              [PyString "server"]
              [PyTuple 2 items]
                [PyString "browserLockdownSvc"]
                [PyString "GetDefaultHomePage"]
    [PyNone]

    PyTuple* tuple = new PyTuple(7);
    tuple->SetItem(0, itr_1);
    tuple->SetItem(1, GenerateLockdownCachedObject());
    tuple->SetItem(2, new PyNone());

            */
    // build the tuple based on above packet...may not need....
    /*
    PyTuple* first = new PyTuple(2);
        first->SetItem(0, new PyLong(Win32TimeNow()));
        first->SetItem(1, new PyInt(46587)); //unknown
    PyTuple* second = new PyTuple(3);
        second->SetItem(0, new PyString("Method Call"));
        second->SetItem(1, new PyString("server"));
    PyTuple* third = new PyTuple(2);
        third->SetItem(0, new PyString("browserLockdownSvc"));
        third->SetItem(1, new PyString("GetDefaultHomePage"));
        second->SetItem(2, third);
    PyTuple* data = new PyTuple(7);
        data->SetItem(0, first);
        data->SetItem(1, new PyNone()); //unknown
        data->SetItem(2, new PyInt(704421)); //unknown
        data->SetItem(3, new PyInt(1)); //unknown
        data->SetItem(4, new PySubStream(new PyString("http:://eve.alasiya.net/")));
        data->SetItem(5, new PyInt(0)); //unknown
        data->SetItem(6, second);
    return new PyObject( "objectCaching.CachedMethodCallResult", data );
    */
    return new PyString("http:://eve.alasiya.net/");
}

//00:37:03 L BrowserLockdownService::Handle_IsBrowserInLockdown(): size= 0
PyResult BrowserLockdownService::Handle_IsBrowserInLockdown(PyCallArgs &call) {
    return new PyBool(sConfig.server.IsTestServer);
}
