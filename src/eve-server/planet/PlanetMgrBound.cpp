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
    Author:        Reve, Comet0
    Updates:    Allan
*/

//work in progress


#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "packets/Planet.h"
#include "planet/PlanetMgrBound.h"
#include "planet/Colony.h"
#include "planet/Planet.h"
#include "planet/PlanetMgr.h"
#include "planet/PlanetMgrBound.h"
#include "system/SystemManager.h"

class PlanetMgrBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(PlanetMgrBound)

    PlanetMgrBound(PyServiceMgr *mgr, Client* pClient, PlanetSE* pPlanet)
    : PyBoundObject(mgr),
    m_dispatch(new Dispatcher(this)),
    m_planet(pPlanet)
    {
        _SetCallDispatcher(m_dispatch);
        m_colony = new Colony(mgr, pClient, m_planet->GetID());
        m_colony->Init();

        m_planetMgr = new PlanetMgr(mgr, pClient, pPlanet, m_colony);

        m_strBoundObjectName = "PlanetMgrBound";

        PyCallable_REG_CALL(PlanetMgrBound, GetPlanetInfo);
        PyCallable_REG_CALL(PlanetMgrBound, GetPlanetResourceInfo);
        PyCallable_REG_CALL(PlanetMgrBound, GetCommandPinsForPlanet);
        PyCallable_REG_CALL(PlanetMgrBound, GetExtractorsForPlanet);
        PyCallable_REG_CALL(PlanetMgrBound, GetProgramResultInfo);
        PyCallable_REG_CALL(PlanetMgrBound, GetResourceData);
        PyCallable_REG_CALL(PlanetMgrBound, GMAddCommodity);
        PyCallable_REG_CALL(PlanetMgrBound, GMConvertCommandCenter);
        PyCallable_REG_CALL(PlanetMgrBound, GMForceInstallProgram);
        PyCallable_REG_CALL(PlanetMgrBound, GMGetLocalDistributionReport);
        PyCallable_REG_CALL(PlanetMgrBound, GMGetSynchedServerState);
        PyCallable_REG_CALL(PlanetMgrBound, GMRunDepletionSim);
        PyCallable_REG_CALL(PlanetMgrBound, UserAbandonPlanet);
        PyCallable_REG_CALL(PlanetMgrBound, UserLaunchCommodities);
        PyCallable_REG_CALL(PlanetMgrBound, UserTransferCommodities);
        PyCallable_REG_CALL(PlanetMgrBound, UserUpdateNetwork);
    }

    virtual ~PlanetMgrBound() {
        delete m_dispatch;
        SafeDelete(m_colony);
        SafeDelete(m_planetMgr);
    }

    virtual void Release() {
        //He hates this statement
        delete this;
    }

    PyCallable_DECL_CALL(GetPlanetInfo);
    PyCallable_DECL_CALL(GetPlanetResourceInfo);
    PyCallable_DECL_CALL(GetCommandPinsForPlanet);
    PyCallable_DECL_CALL(GetExtractorsForPlanet);
    PyCallable_DECL_CALL(GetProgramResultInfo);
    PyCallable_DECL_CALL(GetResourceData);
    PyCallable_DECL_CALL(GMAddCommodity);
    PyCallable_DECL_CALL(GMConvertCommandCenter);
    PyCallable_DECL_CALL(GMForceInstallProgram);
    PyCallable_DECL_CALL(GMGetLocalDistributionReport);
    PyCallable_DECL_CALL(GMGetSynchedServerState);
    PyCallable_DECL_CALL(GMRunDepletionSim);
    PyCallable_DECL_CALL(UserAbandonPlanet);
    PyCallable_DECL_CALL(UserLaunchCommodities);
    PyCallable_DECL_CALL(UserTransferCommodities);
    PyCallable_DECL_CALL(UserUpdateNetwork);
    /*
     *
    data = planet.remoteHandler.GMGetCompleteResource(resourceTypeID, layer)
        sh = builder.CreateSHFromBuffer(data.data, data.numBands)

    self.planet.remoteHandler.GMCreateNuggetLayer(self.planetID, typeID)
        self.GMShowResource(typeID, 'nuggets')      {{ 'nuggets' = layer here }}

     "sm.GetService('planetSvc').GetPlanet(planetID)"  is a bound call.
        self.pin = sm.GetService('planetSvc').GetPlanet(planetID).GetPin(self.pin.id)
        self.pin.id
        self.pin.typeID

        for typeID, amount in self.pin.contents.iteritems():
        for typeID, amount in self.pin.GetProductMaxOutput().iteritems():

        amount=self.pin.GetCpuUsage())))
        amount=self.pin.GetCpuOutput())))
        amount=self.pin.GetPowerUsage())))
        amount=self.pin.GetPowerOutput())))
        self.currRouteCycleTime = self.pin.GetCycleTime()
        if self.pin.IsStorage():

            pin.InstallProgram(typeID, cycleTime, endTime, maxValue, headRadius)
    pin = currentPlanet.CancelInstallProgram(pinID, pinData)


    def LoadDestComboOptions(self):
    colony = self.planet.GetColony(session.charid)
    if colony is None:
        self.sr.spaceportCombo.LoadOptions([(localization.GetByLabel('UI/PI/Common/NoDestinationsFound'), None)])
        return
        self.endpoints = colony.GetImportEndpoints()
        if len(self.endpoints) < 1:
            self.sr.spaceportCombo.LoadOptions([(localization.GetByLabel('UI/PI/Common/NoDestinationsFound'), None)])
            return
            options = []
            for endpoint in self.endpoints:
                pin = self.planet.GetPin(endpoint.id)
                options.append((planetCommon.GetGenericPinName(pin.typeID, pin.id), endpoint.id))

                if self.spaceportPinID is None:
                    self.spaceportPinID = options[0][1]
                    self.sr.spaceportCombo.LoadOptions(options, select=self.spaceportPinID)



    self.remoteHandler = moniker.GetPlanet(self.planetID)
        self.remoteHandler.UserAbandonPlanet()
        updatedColony = self.remoteHandler.UserUpdateNetwork(serializedChanges)

        qtyToDistribute, cycleTime, numCycles = self.remoteHandler.GetProgramResultInfo(pinID, typeID, pin.heads, headRadius)

        */

protected:
    Colony* m_colony;
    PlanetDB* m_db;
    PlanetSE* m_planet;
    PlanetMgr* m_planetMgr;
    Dispatcher* const m_dispatch;
};

PyCallable_Make_InnerDispatcher(PlanetMgrService)

PlanetMgrService::PlanetMgrService(PyServiceMgr *mgr)
: PyService(mgr, "planetMgr"),  /*planetBaseBroker*/
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(PlanetMgrService, GetPlanet);
    PyCallable_REG_CALL(PlanetMgrService, DeleteLaunch);
    PyCallable_REG_CALL(PlanetMgrService, GetPlanetsForChar);
    PyCallable_REG_CALL(PlanetMgrService, GetMyLaunchesDetails);
}

PlanetMgrService::~PlanetMgrService() {
    delete m_dispatch;
}

PyBoundObject* PlanetMgrService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
    /* sends planetID */
    _log(PLANET__INFO, "PlanetMgrService bind request for:");
    bind_args->Dump(PLANET__INFO, "    ");
    if (!bind_args->IsInt()) {
        _log(PLANET__ERROR, "%s Service: invalid bind argument type %s", GetName(), bind_args->TypeString());
        return nullptr;
    }
    SystemEntity* pSE = c->SystemMgr()->GetSE(bind_args->AsInt()->value());
    if (!pSE->IsPlanetSE()) {
        c->SendErrorMsg("itemID is not planetID");
        return nullptr;
    }
    return new PlanetMgrBound(m_manager, c, pSE->GetPlanetSE());
}

PyResult PlanetMgrService::Handle_GetPlanet(PyCallArgs &call) {
    _log(PLANET__DEBUG, "PlanetMgrService::Handle_GetPlanet() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    return nullptr;
}

PyResult PlanetMgrService::Handle_DeleteLaunch(PyCallArgs &call) {
    _log(PLANET__DEBUG, "PlanetMgrService::Handle_DeleteLaunch() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    return nullptr;
}

PyResult PlanetMgrService::Handle_GetPlanetsForChar(PyCallArgs &call) {
  return m_db->GetPlanetsForChar(call.client->GetCharacterID());
}

PyResult PlanetMgrService::Handle_GetMyLaunchesDetails(PyCallArgs &call) {
    return m_db->GetMyLaunchesDetails(call.client->GetCharacterID());
}

PyResult PlanetMgrBound::Handle_GetPlanetInfo(PyCallArgs &call) {
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_GetPlanetInfo() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);
    return m_planet->GetPlanetInfo(m_colony);
}

PyResult PlanetMgrBound::Handle_GetPlanetResourceInfo(PyCallArgs &call) {
    return m_planet->GetPlanetResourceInfo();
}

PyResult PlanetMgrBound::Handle_GetExtractorsForPlanet(PyCallArgs &call) {
    /*
     * 15:25:57 [PlanetDebug] PlanetMgrBound::Handle_GetExtractorsForPlanet() size=1
     * 15:25:57 [PlanetCallDump]   Call Arguments:
     * 15:25:57 [PlanetCallDump]       Tuple: 1 elements
     * 15:25:57 [PlanetCallDump]         [ 0] Integer field: 40159411   <<<  planetID - duh
     */
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_GetExtractorsForPlanet() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    Call_SingleIntegerArg args;
    if (!args.Decode(call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode args for GetResourceData()");
        return nullptr;
    }

    return m_planet->GetExtractorsForPlanet(args.arg);
}

PyResult PlanetMgrBound::Handle_GetCommandPinsForPlanet(PyCallArgs &call) {
    /*  called by "get other character's networks" in planet menu
     * 16:42:42 [PlanetDebug] PlanetMgrBound::Handle_GetCommandPinsForPlanet() size=1
     * 16:42:42 [PlanetCallDump]   Call Arguments:
     * 16:42:42 [PlanetCallDump]       Tuple: 1 elements
     * 16:42:42 [PlanetCallDump]         [ 0] Integer field: 40159411
     */
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_GetCommandPinsForPlanet() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    // returns empty dict if none
    return new PyDict();
}

PyResult PlanetMgrBound::Handle_GetProgramResultInfo(PyCallArgs &call) {
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_GetProgramResultInfo() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    return nullptr;
}

PyResult PlanetMgrBound::Handle_GetResourceData(PyCallArgs &call) {
    //_log(PLANET__DEBUG, "PlanetMgrBound::Handle_GetResourceData() size=%u", call.tuple->size() );
    //call.Dump(PLANET__DUMP);
    /* TODO, Figure out how to populate PyBuffer with more than char.
     *         and figure out the client buffer structure, etc.
     * TODO, optimise this function maybe?
     */
    /*
        inRange, sh = planet.GetResourceData(resourceTypeID)        << check packets for this call --- none.  :(

            /*  this is called by planet view page, by "resource filter" for given typeID
             *     this is dump before dict decoding
             * 12:23:45 [PlanetCallDump]    Dictionary: 8 entries
             * 12:23:45 [PlanetCallDump]      [ 0] Key: String: 'proximity'
             * 12:23:45 [PlanetCallDump]      [ 0] Value: Integer field: 4
             * 12:23:45 [PlanetCallDump]      [ 1] Key: String: 'updateTime'
             * 12:23:45 [PlanetCallDump]      [ 1] Value: Integer field: 0
             * 12:23:45 [PlanetCallDump]      [ 2] Key: String: 'advancedPlanetology'
             * 12:23:45 [PlanetCallDump]      [ 2] Value: Integer field: 0
             * 12:23:45 [PlanetCallDump]      [ 3] Key: String: 'remoteSensing'
             * 12:23:45 [PlanetCallDump]      [ 3] Value: Integer field: 3
             * 12:23:45 [PlanetCallDump]      [ 4] Key: String: 'newBand'
             * 12:23:45 [PlanetCallDump]      [ 4] Value: Integer field: 15
             * 12:23:45 [PlanetCallDump]      [ 5] Key: String: 'planetology'
             * 12:23:45 [PlanetCallDump]      [ 5] Value: Integer field: 0
             * 12:23:45 [PlanetCallDump]      [ 6] Key: String: 'oldBand'
             * 12:23:45 [PlanetCallDump]      [ 6] Value: Integer field: 0
             * 12:23:45 [PlanetCallDump]      [ 7] Key: String: 'resourceTypeID'
             * 12:23:45 [PlanetCallDump]      [ 7] Value: Integer field: 2268
             *
*/
    Call_ResourceDataDict dict;
    PyDict* input = call.tuple->AsTuple()->GetItem(0)->AsObject()->arguments()->AsDict();
    input->Dump(PLANET__DUMP, "   ");
    if (!dict.Decode(&input)) {
        _log(SERVICE__ERROR, "Failed to decode args for GetResourceData()");
        return nullptr;
    }
    return m_planet->GetResourceData(dict);
}

//01:52:23 [PlanetDebug] PlanetMgrBound::Handle_UserAbandonPlanet() size=0
PyResult PlanetMgrBound::Handle_UserAbandonPlanet(PyCallArgs &call) {
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_UserAbandonPlanet() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    return nullptr;
}

PyResult PlanetMgrBound::Handle_UserLaunchCommodities(PyCallArgs &call) {
    /*
            lastLaunchTime = self.remoteHandler.UserLaunchCommodities(commandPinID, commoditiesToLaunch)
            for typeID, qty in commoditiesToLaunch.iteritems():
            */
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_UserLaunchCommodities() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);
    /* 20:00:35 L PlanetMgrBound: Handle_UserLaunchCommodities() size=2
     * 20:00:35 [PlanetCallDump]   Call Arguments:
     * 20:00:35 [PlanetCallDump]       Tuple: 2 elements
     * 20:00:35 [PlanetCallDump]         [ 0] Integer field: 140000083
     * 20:00:35 [PlanetCallDump]         [ 1] Dictionary: 1 entries
     * 20:00:35 [PlanetCallDump]         [ 1]   [ 0] Key: Integer field: 2268
     * 20:00:35 [PlanetCallDump]         [ 1]   [ 0] Value: Integer field: 1
     * 20:00:35 [PlanetCallDump]   Call Named Arguments:
     * 20:00:35 [PlanetCallDump]     Argument 'machoVersion':
     * 20:00:35 [PlanetCallDump]         Integer field: 1
     */

    return nullptr;
}

PyResult PlanetMgrBound::Handle_UserTransferCommodities(PyCallArgs &call) {
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_UserTransferCommodities() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);
/*

        simTime, sourceRunTime = self.remoteHandler.UserTransferCommodities(path, commodities)    {{ simTime = time to stop (complete time), sourceRunTime = previous runtime}}

        */
    return nullptr;
}
/*
            Orbital_Infrastructure = 1025,
            Extractors = 1026,
            Command_Centers = 1027,
            Processors = 1028,
            Storage_Facilities = 1029,
            Spaceports = 1030,
            Planetary_Resources = 1031,
            Planet_Solid = 1032,
            Planet_Liquid_Gas = 1033,
            Refined_Commodities = 1034,
            Planet_Organic = 1035,
            Planetary_Links = 1036,
            Specialized_Commodities = 1040,
            Advanced_Commodities = 1041,
            Basic_Commodities = 1042,
            Planet_Management = 1044,
            Extractor_Control_Units = 1063,
        */
PyResult PlanetMgrBound::Handle_UserUpdateNetwork(PyCallArgs &call) {
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_UserUpdateNetwork() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    UUNCommandList uuncl;
    if (!uuncl.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode args for UUNCommandList");
        return nullptr;
    }

    return m_planetMgr->UpdateNetwork(uuncl);
}

PyResult PlanetMgrBound::Handle_GMAddCommodity(PyCallArgs &call) {
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_GMAddCommodity() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    return nullptr;
}

PyResult PlanetMgrBound::Handle_GMConvertCommandCenter(PyCallArgs &call) {
    //self.remoteHandler.GMConvertCommandCenter(pinID)
    //  this is an option in the GM planet menu.  no clue what it's for or what it does.....
    /*
     * 02:48:32 [PlanetDebug] PlanetMgrBound::Handle_GMConvertCommandCenter() size=1
     * 02:48:32 [PlanetCallDump]   Call Arguments:
     * 02:48:32 [PlanetCallDump]       Tuple: 1 elements
     * 02:48:32 [PlanetCallDump]         [ 0] Integer field: 140006224      << CC PinID (currently wrong)
     */

    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_GMConvertCommandCenter() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    return nullptr;
}

PyResult PlanetMgrBound::Handle_GMForceInstallProgram(PyCallArgs &call) {
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_GMForceInstallProgram() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);
    /*
     *        if typeID not in resourceInfo or qtyPerCycle < 0 or cycleTime < 10 * SEC or lifetimeHours < 1 or headRadius <= 0.0:
     *            return
     *        self.remoteHandler.GMForceInstallProgram(pinID, typeID, cycleTime, lifetimeHours, qtyPerCycle, headRadius)
     *
     * 16:40:57 L PlanetMgrBound: Handle_GMForceInstallProgram() size=6
     * 16:40:57 [PlanetCallDump]   Call Arguments:
     * 16:40:57 [PlanetCallDump]       Tuple: 6 elements
     * 16:40:57 [PlanetCallDump]         [ 0] Tuple: 2 elements
     * 16:40:57 [PlanetCallDump]         [ 0]   [ 0] Integer field: 1
     * 16:40:57 [PlanetCallDump]         [ 0]   [ 1] Integer field: 1
     * 16:40:57 [PlanetCallDump]         [ 1] Integer field: 2272
     * 16:40:57 [PlanetCallDump]         [ 2] Integer field: 600000000
     * 16:40:57 [PlanetCallDump]         [ 3] Integer field: 24
     * 16:40:57 [PlanetCallDump]         [ 4] Integer field: 100
     * 16:40:57 [PlanetCallDump]         [ 5] Real field: 1.000000
     */
    return nullptr;
}

//15:15:02[00m L [37;01mPlanetMgrBound: [00mHandle_GMGetLocalDistributionReport() size=2
PyResult PlanetMgrBound::Handle_GMGetLocalDistributionReport(PyCallArgs &call) {
    /*
     *      return self.remoteHandler.GMGetLocalDistributionReport(self.planetID, (surfacePoint.theta, surfacePoint.phi))
     */
    /*
     *     1 5*:15:02 [PlanetCallDump]   Call Arguments:
     *     15:15:02 [PlanetCallDump]       Tuple: 2 elements
     *     15:15:02 [PlanetCallDump]         [ 0] Integer field: 40216265      << planetID
     *     15:15:02 [PlanetCallDump]         [ 1] Tuple: 2 elements
     *     15:15:02 [PlanetCallDump]         [ 1]   [ 0] Real field: 0.359286  << theta
     *     15:15:02 [PlanetCallDump]         [ 1]   [ 1] Real field: 1.014020  << phi
     *     _log(PLANET__DEBUG, "PlanetMgrBound::Handle_GMGetLocalDistributionReport() size=%u", call.tuple->size() );
     *     call.Dump(PLANET__DUMP);
     */

    return nullptr;
}

PyResult PlanetMgrBound::Handle_GMGetSynchedServerState(PyCallArgs &call) {
    /*
     *    def GMVerifySimulation(self):
     *        self.LogNotice('VerifySimulation -- starting')
     *        simulationDuration, remoteColonyData = self.remoteHandler.GMGetSynchedServerState(session.charid)
     *        simEndTime = remoteColonyData.currentSimTime
     *        colony = self.GetColony(session.charid)
     *        startTime = blue.os.GetWallclockTimeNow()
     *        colony.RunSimulation(runSimUntil=simEndTime)
     *        clientSimulationRuntime = blue.os.GetWallclockTimeNow() - startTime
     *        pins = remoteColonyData.pins
     *        self.LogNotice('simulation ran for', clientSimulationRuntime, 'on client, ', simulationDuration, 'on server')
     *        for pin in pins:
     *            clientPin = colony.GetPin(pin.id)
     *            if clientPin is None:
     *                self.LogError(pin.id, 'exists on server but not on client')
     *                continue
     *            for key, value in pin.__dict__.iteritems():
     *                if not hasattr(clientPin, key):
     *                    self.LogError(pin.id, 'on client does not have attribute ', key)
     *                    continue
     *                clientValue = getattr(clientPin, key)
     *                if clientValue != value:
     *                    self.LogError(pin.id, 'does not agree on a value for', key, 'Client says ', clientValue, 'but server', value)
     *
     *        self.LogNotice('VerifySimulation -- finished')
     */
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_GMGetSynchedServerState() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    return nullptr;
}

PyResult PlanetMgrBound::Handle_GMRunDepletionSim(PyCallArgs &call) {
    _log(PLANET__DEBUG, "PlanetMgrBound::Handle_GMRunDepletionSim() size=%u", call.tuple->size() );
    call.Dump(PLANET__DUMP);

    return nullptr;
}
