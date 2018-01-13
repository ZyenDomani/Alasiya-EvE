
 /**
  * @name FleetObject.cpp
  *     Fleet Object code for Alasiya EvEmu
  *
  * @Author:        Allan
  * @date:          05 August 2014 (original skeleton outline)
  * @update:        21 November 2017 (begin actual implementation)
  *
  */

#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "fleet/FleetObject.h"
#include "fleet/FleetBound.h"

PyCallable_Make_InnerDispatcher(FleetObject)

FleetObject::FleetObject(PyServiceMgr *mgr)
: PyService(mgr, "fleetObjectHandler"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(FleetObject, CreateFleet);
}

FleetObject::~FleetObject()
{
    delete m_dispatch;
}

PyBoundObject* FleetObject::_CreateBoundObject( Client* c, const PyRep* bind_args )
{
    if (is_log_enabled(FLEET__BIND_DUMP)) {
        _log( FLEET__BIND_DUMP, "FleetObject bind request for:" );
        bind_args->Dump( FLEET__BIND_DUMP, "    " );
    }

    if (!bind_args->IsInt()) {
        _log(FLEET__ERROR, "%s Service: invalid bind argument type %s", GetName(), bind_args->TypeString());
        return nullptr;
    }
    /*  do we need to bind object like this?   probably not, cause it works as-is
    //we just bind up a new inventory object for container requested and give it back to them.
    InventoryBound *ib = new InventoryBound(m_manager, item, flag);
    PyRep *result = m_manager->BindObject(call.client, ib);
    */
    return new FleetBound( m_manager, bind_args->AsInt()->value());
}

// FOH::CreateFleet, FOH::
PyResult FleetObject::Handle_CreateFleet(PyCallArgs &call) {
    //self.fleet = sm.RemoteSvc('fleetObjectHandler').CreateFleet()
    FleetBindRSP fbr;
        fbr.nodeID = 888444;
        fbr.fleetID = sFltSvc.CreateFleet(call.client);
        fbr.unknown = 0;
    if (fbr.fleetID == 0)
        return nullptr;
    return fbr.Encode();
}
