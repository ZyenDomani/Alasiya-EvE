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
    Author:        Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "scanning/ScanMgrService.h"
#include "system/DestinyManager.h"

class ScanBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(ScanBound)

    ScanBound(PyServiceMgr *mgr, Client *c)
    : PyBoundObject(mgr),
    m_dispatch(new Dispatcher(this))
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "ScanBound";

        PyCallable_REG_CALL(ScanBound, ConeScan);
        PyCallable_REG_CALL(ScanBound, RequestScans);
        PyCallable_REG_CALL(ScanBound, RecoverProbes);
        PyCallable_REG_CALL(ScanBound, ReconnectToLostProbes);
        PyCallable_REG_CALL(ScanBound, DestroyProbe);

    }
        /**
        return sm.RemoteSvc('scanMgr').GetSystemScanMgr().ConeScan(scanangle, scanRange, x, y, z)

        return sm.RemoteSvc('scanMgr').GetSystemScanMgr().ReconnectToLostProbes()

        successProbeIDs = sm.RemoteSvc('scanMgr').GetSystemScanMgr().RecoverProbes(probeIDs)

        scanMan = sm.RemoteSvc('scanMgr').GetSystemScanMgr()
        scanMan.RequestScans(probes)

        scanMan = sm.RemoteSvc('scanMgr').GetSystemScanMgr()
        scanMan.DestroyProbe(probeID)

        */

    virtual ~ScanBound() {delete m_dispatch;}
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(RequestScans);
    PyCallable_DECL_CALL(ConeScan);
    PyCallable_DECL_CALL(ReconnectToLostProbes);
    PyCallable_DECL_CALL(RecoverProbes);
    PyCallable_DECL_CALL(DestroyProbe);

protected:
    Dispatcher *const m_dispatch;
    Scan* m_scan;
    ScanningDB* m_db;
};

PyCallable_Make_InnerDispatcher(ScanMgrService)

ScanMgrService::ScanMgrService(PyServiceMgr *mgr)
: PyService(mgr, "scanMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(ScanMgrService, GetSystemScanMgr);

}

ScanMgrService::~ScanMgrService() {
    delete m_dispatch;
}

//02:17:50 L ScanMgrService::Handle_GetSystemScanMgr(): size= 0
PyResult ScanMgrService::Handle_GetSystemScanMgr( PyCallArgs& call ) {
    Client* pClient = call.client;
    DestinyManager* pDestiny = pClient->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return NULL;
    }

    ScanBound* pSB = new ScanBound(m_manager, pClient);
    PyRep* result = m_manager->BindObject(call.client, pSB);
    return result;
}

PyResult ScanBound::Handle_ConeScan( PyCallArgs& call ) {
    //result = sm.GetService('scanSvc').ConeScan(self.scanangle, rnge * 1000, vec.x, vec.y, vec.z)
    //return sm.RemoteSvc('scanMgr').GetSystemScanMgr().ConeScan(scanangle, scanRange, x, y, z)
    Call_ConeScan args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        //TODO: throw exception
        return NULL;
    }

    Client* pClient = call.client;
    DestinyManager* pDestiny = pClient->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return nullptr;
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't scan while warping");
        return nullptr;
    }

    if (!pClient->scan())
        pClient->SetScan(new Scan(pClient));

    return pClient->scan()->ConeScan(args);
}

PyResult ScanBound::Handle_RequestScans( PyCallArgs& call ) {
    sLog.Log( "ScanMgrService::Handle_RequestScans()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
    /*      // using ship scanner
            [PyString "RequestScans"]
            [PyTuple 1 items]
              [PyNone]
            // using probes
            [PyString "RequestScans"]
            [PyTuple 1 items]
              [PyDict 6 kvp] //dict of probe data
     */
    Client* pClient = call.client;
    DestinyManager* pDestiny = pClient->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return nullptr;
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't scan while warping");
        return nullptr;
    }

    if (!pClient->scan())
        pClient->SetScan(new Scan(pClient));

    PyDict* dict = nullptr;
    if (call.tuple->GetItem( 0 )->IsDict())
        dict = call.tuple->GetItem(0)->AsDict();

    pClient->scan()->RequestScans(dict);

    // this call returns a PyNone
    return new PyNone();
}

PyResult ScanBound::Handle_ReconnectToLostProbes( PyCallArgs& call ) {
    sLog.Log( "ScanMgrService::Handle_ReconnectToLostProbes()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
}

PyResult ScanBound::Handle_RecoverProbes( PyCallArgs& call ) {
    sLog.Log( "ScanMgrService::Handle_RecoverProbes()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
    //successProbeIDs = sm.RemoteSvc('scanMgr').GetSystemScanMgr().RecoverProbes(probeIDs)

    return nullptr;
}

PyResult ScanBound::Handle_DestroyProbe( PyCallArgs& call ) {
    sLog.Log( "ScanMgrService::Handle_DestroyProbe()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
    //scanMan = sm.RemoteSvc('scanMgr').GetSystemScanMgr()
    //scanMan.DestroyProbe(probeID)

    return nullptr;
}
