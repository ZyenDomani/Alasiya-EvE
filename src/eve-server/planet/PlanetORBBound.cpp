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

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "planet/PlanetORBBound.h"

class PlanetORBBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(PlanetORBBound)

    PlanetORBBound(PyServiceMgr *mgr)
    : PyBoundObject(mgr),
    m_dispatch(new Dispatcher(this))
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "PlanetORBBound";

        PyCallable_REG_CALL(PlanetORBBound, GMChangeSpaceObjectOwner);
    }
    virtual ~PlanetORBBound() { delete m_dispatch; }
    virtual void Release() {
        //He hates this statement
        delete this;
    }

    PyCallable_DECL_CALL(GMChangeSpaceObjectOwner);

protected:
    Dispatcher* const m_dispatch;
    PlanetDB* m_db;
};

PyCallable_Make_InnerDispatcher(planetORB)


planetORB::planetORB(PyServiceMgr *mgr)
: PyService(mgr, "planetOrbitalRegistryBroker"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    //PyCallable_REG_CALL(planetORB, );
    //PyCallable_REG_CALL(planetORB, );
}

planetORB::~planetORB() {
    delete m_dispatch;
}

PyBoundObject* planetORB::_CreateBoundObject(Client *c, const PyRep *bind_args) {
    _log(PLANET__INFO, "planetORB bind request for:");
    bind_args->Dump(PLANET__INFO, "    ");
    if(!bind_args->IsInt()) {
        codelog(SERVICE__ERROR, "%s Service: invalid bind argument type %s", GetName(), bind_args->TypeString());
        return NULL;
    }
    return new PlanetORBBound(m_manager);
}


PyResult PlanetORBBound::Handle_GMChangeSpaceObjectOwner( PyCallArgs& call )
{
    // this is called when taking ownership of control tower
    // sends itemID, corpID
    /*
    def TakeOrbitalOwnership(self, itemID, planetID):
        registry = moniker.GetPlanetOrbitalRegistry(session.solarsystemid)
        registry.GMChangeSpaceObjectOwner(itemID, session.corpid)
    */
    sLog.Log( "PlanetORBBound", "Handle_GMChangeSpaceObjectOwner" );
    call.Dump(SERVICE__CALL_DUMP);

    return new PyNone();
}



/**
    def ConfigureOrbital(self, item):
        sm.GetService('planetUI').OpenConfigureWindow(item)

        */