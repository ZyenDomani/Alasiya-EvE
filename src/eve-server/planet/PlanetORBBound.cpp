/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
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

#include "../eve-server.h"

#include "EntityMgr.h"
#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "packets/Planet.h"
#include "planet/PlanetORBBound.h"
#include "planet/CustomsOffice.h"
#include "system/SystemManager.h"

class PlanetORBBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(PlanetORBBound)

    PlanetORBBound(PyServiceMgr *mgr, uint32 systemID)
    : PyBoundObject(mgr),
    m_dispatch(new Dispatcher(this)),
    m_pSysMgr(nullptr)
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "PlanetORBBound";

        PyCallable_REG_CALL(PlanetORBBound, GetTaxRate);
        PyCallable_REG_CALL(PlanetORBBound, UpdateSettings);
        PyCallable_REG_CALL(PlanetORBBound, GetSettingsInfo);

        m_pSysMgr = sEntityMgr.FindOrBootSystem(systemID);
    }
    virtual ~PlanetORBBound() { delete m_dispatch; }

    PyCallable_DECL_CALL(GetTaxRate);
    PyCallable_DECL_CALL(UpdateSettings);
    PyCallable_DECL_CALL(GetSettingsInfo);
    PyCallable_DECL_CALL(GMChangeSpaceObjectOwner);

protected:
    Dispatcher* const m_dispatch;

private:
    SystemManager* m_pSysMgr;
};

PyCallable_Make_InnerDispatcher(PlanetORB)


PlanetORB::PlanetORB(PyServiceMgr *mgr)
: PyService(mgr, "planetOrbitalRegistryBroker"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    //PyCallable_REG_CALL(PlanetORB, );
}

PlanetORB::~PlanetORB() {
    delete m_dispatch;
}

PyBoundObject* PlanetORB::CreateBoundObject(Client *pClient, const PyRep *bind_args) {
    _log(PLANET__INFO, "PlanetORB bind request for:");  // sends systemID
    bind_args->Dump(PLANET__INFO, "    ");
    if (!bind_args->IsInt()) {
        codelog(SERVICE__ERROR, "%s Service: invalid bind argument type %s", GetName(), bind_args->TypeString());
        return nullptr;
    }

    return new PlanetORBBound(m_manager, bind_args->AsInt()->value());
}

PyResult PlanetORBBound::Handle_GetTaxRate(PyCallArgs& call)
{
    //  taxRate = moniker.GetPlanetOrbitalRegistry(session.solarsystemid).GetTaxRate(itemID)
    // NOTE:  return "PyNone()" to deny customs office access.

    SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    CustomsSE* pCOSE = m_pSysMgr->GetSE(args.arg)->GetCOSE();
    if (pCOSE == nullptr) {
        _log(PLANET__ERROR, "COSE %u not found in %s(%u)", args.arg, m_pSysMgr->GetName(), m_pSysMgr->GetID());
        call.client->SendInfoModalMsg("This Customs Office is unavailable.");
        return PyStatic.NewNone();
    }

    return pCOSE->GetTaxRate(call.client);
}

PyResult PlanetORBBound::Handle_GetSettingsInfo(PyCallArgs& call)
{
    /*   self.orbitalData = self.remoteOrbitalRegistry.GetSettingsInfo(self.orbitalID)  << for customs offices
     *   self.selectedHour, self.taxRateValues, self.standingLevel, self.allowAlliance, self.allowStandings = self.orbitalData
     */
    SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    CustomsSE* pCOSE = m_pSysMgr->GetSE(args.arg)->GetCOSE();
    if (pCOSE == nullptr) {
        _log(PLANET__ERROR, "COSE %u not found in %s(%u)", args.arg, m_pSysMgr->GetName(), m_pSysMgr->GetID());
        call.client->SendInfoModalMsg("This Customs Office is unavailable.");
        return nullptr;
    }
    return pCOSE->GetSettingsInfo();
}

PyResult PlanetORBBound::Handle_UpdateSettings(PyCallArgs& call)
{
    //remoteOrbitalRegistry.UpdateSettings(self.orbitalID, reinforceValue, taxRateValues, standingValue, allowAllianceValue, allowStandingsValue)
    /*
     * 20:29:55 [SvcMsg] planetOrbitalRegistryBroker Service: MachoBindObject also contains call to UpdateSettings
     * 20:29:55 [PlanetCallDump]   Call Arguments:
     * 20:29:55 [PlanetCallDump]      Tuple: 6 elements
     * 20:29:55 [PlanetCallDump]       [ 0]    Integer: 140028093
     * 20:29:55 [PlanetCallDump]       [ 1]    Integer: 0
     * 20:29:55 [PlanetCallDump]       [ 2] Object:
     * 20:29:55 [PlanetCallDump]       [ 2]   Type:     String: 'util.KeyVal'
     * 20:29:55 [PlanetCallDump]       [ 2]   Args:  Dictionary: 7 entries
     * 20:29:55 [PlanetCallDump]       [ 2]   Args:   [ 0]   Key:     String: 'standingNeutral'
     * 20:29:55 [PlanetCallDump]       [ 2]   Args:   [ 0] Value:       Real: 0.080000
     * 20:29:55 [PlanetCallDump]       [ 2]   Args:   [ 1]   Key:     String: 'corporation'
     * 20:29:55 [PlanetCallDump]       [ 2]   Args:   [ 1] Value:       Real: 0.050000
     * 20:29:55 [PlanetCallDump]       [ 2]   Args:   [ 2]   Key:     String: 'standingHorrible'
     * 20:29:55 [PlanetCallDump]       [ 2]   Args:   [ 2] Value:       Real: 0.200000
     * 20:29:55 [PlanetCallDump]       [ 2]   Args:   [ 3]   Key:     String: 'standingGood'
     * 20:29:55 [PlanetCallDump]       [ 2]   Args:   [ 3] Value:       Real: 0.050000
     * 20:29:55 [PlanetCallDump]       [ 2]   Args:   [ 4]   Key:     String: 'alliance'
     * 20:29:55 [PlanetCallDump]       [ 2]   Args:   [ 4] Value:       Real: 0.070000
     * 20:29:55 [PlanetCallDump]       [ 2]   Args:   [ 5]   Key:     String: 'standingBad'
     * 20:29:55 [PlanetCallDump]       [ 2]   Args:   [ 5] Value:       Real: 0.100000
     * 20:29:55 [PlanetCallDump]       [ 2]   Args:   [ 6]   Key:     String: 'standingHigh'
     * 20:29:55 [PlanetCallDump]       [ 2]   Args:   [ 6] Value:       Real: 0.020000
     * 20:29:55 [PlanetCallDump]       [ 3]    Integer: -5
     * 20:29:55 [PlanetCallDump]       [ 4]    Integer: 1
     * 20:29:55 [PlanetCallDump]       [ 5]    Integer: 1
     */
    _log(INV__MESSAGE, "Calling PlanetORBBound::UpdateSettings()");
    call.Dump(PLANET__DUMP);

    Call_UpdateSettings args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    PyDict* input = args.taxRateValues->AsObject()->arguments()->AsDict();
    Call_TaxRateValuesDict dict;
    if (!dict.Decode(input)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    CustomsSE* pCOSE = m_pSysMgr->GetSE(args.orbitalID)->GetCOSE();
    if (pCOSE == nullptr) {
        _log(PLANET__ERROR, "COSE %u not found in %s(%u)", args.orbitalID, m_pSysMgr->GetName(), m_pSysMgr->GetID());
        call.client->SendInfoModalMsg("This Customs Office is unavailable.");
        return nullptr;
    }

    pCOSE->UpdateSettings(args.reinforceValue, args.standingValue, args.allowAlliance, args.allowStandings, dict);

    return nullptr;
}
