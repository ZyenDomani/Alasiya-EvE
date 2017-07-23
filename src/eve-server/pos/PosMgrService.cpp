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
#include "planet/Moon.h"
#include "pos/PosMgrService.h"
#include "pos/Structure.h"
#include "system/SystemManager.h"

class PosMgrBound
    : public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(PosMgrBound);

    PosMgrBound(PyServiceMgr* mgr)
    : PyBoundObject(mgr),
      m_dispatch(new Dispatcher(this))
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "PosMgrBound";

        PyCallable_REG_CALL(PosMgrBound, GetMoonForTower);
        PyCallable_REG_CALL(PosMgrBound, SetTowerPassword);
        PyCallable_REG_CALL(PosMgrBound, SetShipPassword);
        PyCallable_REG_CALL(PosMgrBound, GetSiloCapacityByItemID);
        PyCallable_REG_CALL(PosMgrBound, AnchorOrbital);
        PyCallable_REG_CALL(PosMgrBound, UnanchorOrbital);
        PyCallable_REG_CALL(PosMgrBound, OnlineOrbital);
        PyCallable_REG_CALL(PosMgrBound, GMUpgradeOrbital);
        PyCallable_REG_CALL(PosMgrBound, AnchorStructure);
        PyCallable_REG_CALL(PosMgrBound, UnanchorStructure);
        PyCallable_REG_CALL(PosMgrBound, AssumeStructureControl);
        PyCallable_REG_CALL(PosMgrBound, RelinquishStructureControl);
        PyCallable_REG_CALL(PosMgrBound, ChangeStructureProvisionType);
        PyCallable_REG_CALL(PosMgrBound, CompleteOrbitalStateChange);
        PyCallable_REG_CALL(PosMgrBound, GetMoonProcessInfoForTower);
        PyCallable_REG_CALL(PosMgrBound, LinkResourcesForTower);
        PyCallable_REG_CALL(PosMgrBound, RunMoonProcessCycleforTower);
        PyCallable_REG_CALL(PosMgrBound, GetStarbasePermissions);
        PyCallable_REG_CALL(PosMgrBound, SetStarbasePermissions);
        PyCallable_REG_CALL(PosMgrBound, GetTowerNotificationSettings);
        PyCallable_REG_CALL(PosMgrBound, SetTowerNotifications);
        PyCallable_REG_CALL(PosMgrBound, GetTowerSentrySettings);
        PyCallable_REG_CALL(PosMgrBound, SetTowerSentrySettings);

    }

    virtual ~PosMgrBound() {delete m_dispatch;}
    virtual void Release() {
        delete this;
    }

    PyCallable_DECL_CALL(SetTowerPassword);
    PyCallable_DECL_CALL(SetShipPassword);
    PyCallable_DECL_CALL(GetMoonForTower);
    PyCallable_DECL_CALL(GetSiloCapacityByItemID);
    PyCallable_DECL_CALL(AnchorOrbital);
    PyCallable_DECL_CALL(UnanchorOrbital);
    PyCallable_DECL_CALL(OnlineOrbital);
    PyCallable_DECL_CALL(GMUpgradeOrbital);
    PyCallable_DECL_CALL(AnchorStructure);
    PyCallable_DECL_CALL(UnanchorStructure);
    PyCallable_DECL_CALL(AssumeStructureControl);
    PyCallable_DECL_CALL(RelinquishStructureControl);
    PyCallable_DECL_CALL(ChangeStructureProvisionType);
    PyCallable_DECL_CALL(CompleteOrbitalStateChange);
    PyCallable_DECL_CALL(GetMoonProcessInfoForTower);
    PyCallable_DECL_CALL(LinkResourcesForTower);
    PyCallable_DECL_CALL(RunMoonProcessCycleforTower);
    PyCallable_DECL_CALL(GetStarbasePermissions);
    PyCallable_DECL_CALL(SetStarbasePermissions);
    PyCallable_DECL_CALL(GetTowerNotificationSettings);
    PyCallable_DECL_CALL(SetTowerNotifications);
    PyCallable_DECL_CALL(GetTowerSentrySettings);
    PyCallable_DECL_CALL(SetTowerSentrySettings);

protected:
    Dispatcher* const m_dispatch;
    PosMgrDB* m_db;        //we do not own this

};

PyCallable_Make_InnerDispatcher(PosMgrService)

PosMgrService::PosMgrService(PyServiceMgr *mgr)
: PyService(mgr, "posMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(PosMgrService, GetJumpArrays);
    PyCallable_REG_CALL(PosMgrService, GetControlTowers);
    PyCallable_REG_CALL(PosMgrService, InstallJumpBridgeLink);
    PyCallable_REG_CALL(PosMgrService, UninstallJumpBridgeLink);
    PyCallable_REG_CALL(PosMgrService, GetControlTowerFuelRequirements);
}

PosMgrService::~PosMgrService() {
    delete m_dispatch;
}

PyBoundObject* PosMgrService::_CreateBoundObject( Client* c, const PyRep* bind_args ) {
    _log( CLIENT__MESSAGE, "PosMgrService bind request for:" );
    bind_args->Dump( COLLECT__OTHER_DUMP, "    " );

    return new PosMgrBound( m_manager );
}

PyResult PosMgrService::Handle_GetJumpArrays(PyCallArgs &call) {
    /*        jb = sm.RemoteSvc('posMgr').GetJumpArrays()
     *
            for data in jb:
                solarSystemID, subData = data (fromSystem, solarSystemData)
                    ssid = subData.keys()[0]
                    tsid = subData.values()[0][1]

                    fromStructure = solarSystemData.keys()[0]
                    toSystem = cfg.evelocations.Get(solarSystemData.values()[0][0])
                    toStructure = solarSystemData.values()[0][1]
                    toStructureType = solarSystemData.values()[0][2]
                    */
    sLog.White( "PosMgrService::Handle_GetJumpArrays()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    return nullptr;
}

PyResult PosMgrService::Handle_GetControlTowers(PyCallArgs &call) {
    /*  ct = sm.RemoteSvc('posMgr').GetControlTowers()
        for row in ct:
            typeID, structureID, solarSystemID = row[0:3]
    */

    sLog.White( "PosMgrService::Handle_GetControlTowers()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    return nullptr;
}

PyResult PosMgrService::Handle_InstallJumpBridgeLink(PyCallArgs &call) {
    /**
     *    def BridgePortals(self, localItemID, remoteSolarSystemID, remoteItemID):
     *        posLocation = util.Moniker('posMgr', session.solarsystemid)
     *        posLocation.InstallJumpBridgeLink(localItemID, remoteSolarSystemID, remoteItemID)
     *
     */
    sLog.White( "PosMgrService::Handle_InstallJumpBridgeLink()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    return nullptr;
}

PyResult PosMgrService::Handle_UninstallJumpBridgeLink(PyCallArgs &call) {
    /**
     *    def UnbridgePortal(self, itemID):
     *        posLocation = util.Moniker('posMgr', session.solarsystemid)
     *        posLocation.UninstallJumpBridgeLink(itemID)
     *
     */
    sLog.White( "PosMgrService::Handle_UninstallJumpBridgeLink()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    return nullptr;
}

PyResult PosMgrService::Handle_GetControlTowerFuelRequirements(PyCallArgs &call) {
    sLog.White( "PosMgrService::Handle_GetControlTowerFuelRequirements()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    return m_db->GetControlTowerFuelRequirements();
}

PyResult PosMgrBound::Handle_GetTowerNotificationSettings(PyCallArgs &call) {
    /*
     *        notifySettings = self.posMgr.GetTowerNotificationSettings(self.slimItem.itemID)
     *        self.fuelNotifyCheckbox.SetChecked(notifySettings.sendFuelNotifications, 0)
     *        self.calendarCheckbox.SetChecked(notifySettings.showInCalendar, 0)
     */
    sLog.White( "PosMgrBound::Handle_GetTowerNotificationSettings()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    PyRep *result(nullptr);

    return result;
}

PyResult PosMgrBound::Handle_SetTowerNotifications(PyCallArgs &call) {
    //self.posMgr.SetTowerNotifications(self.slimItem.itemID, showInCalendar, sendFuelNotifications)
    sLog.White( "PosMgrBound::Handle_SetTowerNotifications()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    PyRep *result(nullptr);

    return result;
}

PyResult PosMgrBound::Handle_GetTowerSentrySettings(PyCallArgs &call) {
    //  standing, status, statusDrop, war, standingOwnerID = self.sentrySettings = self.posMgr.GetTowerSentrySettings(self.slimItem.itemID)
    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return new PyNone();
    }

    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    StructureSE* pSSE = pSystem->GetSE(arg.arg)->GetPOSSE();
    if (pSSE == nullptr)
        return nullptr;

    PyDict* data = new PyDict();
    PyList* headerList = new PyList();
        headerList->AddItem(new PyString("standing"));
        headerList->AddItem(new PyString("status"));
        headerList->AddItem(new PyString("statusDrop"));
        headerList->AddItem(new PyString("corpWar"));
        headerList->AddItem(new PyString("standingOwnerID"));
        data->SetItemString("header", headerList);
    PyList* lineList = new PyList();
        lineList->AddItem(new PyFloat(pSSE->GetStanding()));
        lineList->AddItem(new PyFloat(pSSE->GetStatus()));
        lineList->AddItem(new PyBool(pSSE->GetStatusDrop()));
        lineList->AddItem(new PyBool(pSSE->GetCorpWar()));
        lineList->AddItem(new PyInt(pSSE->GetStandingOwnerID()));
        data->SetItemString("line", lineList);

    return new PyObject("util.Row", data);
}

PyResult PosMgrBound::Handle_SetTowerSentrySettings(PyCallArgs &call) {
    //  self.posMgr.SetTowerSentrySettings(self.slimItem.itemID, standing, status, statusDrop, war, useAllianceStandings)
    /*
            [PyString "SetTowerSentrySettings"]
            [PyTuple 7 items]
              [PyIntegerVar 1002332856217]
              [PyFloat 0.1]
              [PyFloat 0.2]
              [PyBool True]
              [PyBool True]
              [PyBool True]
              [PyInt 0]
              */
    sLog.White( "PosMgrBound::Handle_SetTowerSentrySettings()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    PyRep *result(nullptr);

    return result;
}

PyResult PosMgrBound::Handle_GetStarbasePermissions(PyCallArgs &call) {
    //  deployFlags, usageFlagsList = self.posMgr.GetStarbasePermissions(self.slimItem.itemID)
    sLog.White( "PosMgrBound::Handle_GetStarbasePermissions()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    PyRep *result(nullptr);

    return result;
}

PyResult PosMgrBound::Handle_SetStarbasePermissions(PyCallArgs &call) {
    //  self.posMgr.SetStarbasePermissions(self.slimItem.itemID, self.sr.deployFlags, self.sr.usageFlagsList)
    sLog.White( "PosMgrBound::Handle_SetStarbasePermissions()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    PyRep *result(nullptr);

    return result;
}

PyResult PosMgrBound::Handle_GetMoonForTower( PyCallArgs &call ) {
  /*
13:13:06 L PosMgrBound::Handle_GetMoonForTower(): size= 1
13:13:06 [SvcCall]   Call Arguments:
13:13:06 [SvcCall]       Tuple: 1 elements
13:13:06 [SvcCall]         [ 0] Integer field: 140001260

self.moonID = self.moon[0]
if self.moon[1] is not None:
    for typeID, quantity in self.moon[1]:

    returns
        tuple
            moonID
            tuple
                resource typeID
                resource quantity

  sLog.White( "PosMgrBound::Handle_GetMoonForTower()", "size= %u", call.tuple->size() );
  call.Dump(POS__DUMP);
  */

  SystemManager* pSystem = call.client->SystemMgr();
  if (pSystem == nullptr) {
      codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
      return new PyNone();
  }

    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    StructureSE* pSSE = pSystem->GetSE(arg.arg)->GetPOSSE();
    if (pSSE == nullptr)
        return nullptr;
    MoonSE* pMSE = pSSE->GetMoonEntity()->GetMoonSE();
    if (pMSE == nullptr)
        return nullptr;

    std::map<uint16, uint8>::iterator itr = pMSE->GooBegin();
    std::map<uint16, uint8>::iterator end = pMSE->GooEnd();

    PyList* list = new PyList();

    while (itr != end) {
        PyTuple* resource = new PyTuple(2);
            resource->SetItem(0, new PyInt(itr->first));
            resource->SetItem(1, new PyInt(itr->second));
        list->AddItem(resource);
        ++itr;
    }

    PyTuple* item = new PyTuple(2);
        item->SetItem(0, new PyInt(pMSE->GetID()));
        item->SetItem(1, list);

    return item;
}

PyResult PosMgrBound::Handle_SetTowerPassword( PyCallArgs &call ) {
  /*
13:10:09 L PosMgrBound::Handle_SetTowerPassword(): size= 2
13:10:09 [SvcCall]   Call Arguments:
13:10:09 [SvcCall]       Tuple: 2 elements
13:10:09 [SvcCall]         [ 0] Integer field: 140001260    << towerID
13:10:09 [SvcCall]         [ 1] WString: 'test'             << password

self.posMgr.SetTowerPassword(self.slimItem.itemID, password, allowCorp, allowAlliance)
            [PyString "SetTowerPassword"]
            [PyTuple 4 items]
              [PyIntegerVar 1002332856217]
              [PyString "password"]
              [PyBool True]
              [PyBool False]

  sLog.White( "PosMgrBound::Handle_SetTowerPassword()", "size= %u", call.tuple->size() );
  call.Dump(POS__DUMP);
*/
  PyRep *result(nullptr);

    return result;
}

PyResult PosMgrBound::Handle_SetShipPassword( PyCallArgs &call ) {
  /*
13:16:17 L PosMgrBound::Handle_SetShipPassword(): size= 1
13:16:17 [SvcCall]   Call Arguments:
13:16:17 [SvcCall]       Tuple: 1 elements
13:16:17 [SvcCall]         [ 0] WString: 'test'             << password

  sLog.White( "PosMgrBound::Handle_SetShipPassword()", "size= %u", call.tuple->size() );
  call.Dump(POS__DUMP);
*/

  PyRep *result(nullptr);

    return result;
}

PyResult PosMgrBound::Handle_GetSiloCapacityByItemID(PyCallArgs &call) {
  sLog.White( "PosMgrBound::Handle_GetSiloCapacityByItemID()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    uint16 typeID = 0;

    return m_db->GetSiloCapacityByItemID(typeID);
}

PyResult PosMgrBound::Handle_AnchorStructure(PyCallArgs &call) {
    /*  structure state is queried by StructureEntity::GetPOSState()
     *    state is saved in StructureEntity::m_state (POSState)
     */
    sLog.White( "PosMgrBound::Handle_AnchorStructure()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    PyRep *result(nullptr);

    return result;
}

PyResult PosMgrBound::Handle_UnanchorStructure(PyCallArgs &call) {
    sLog.White( "PosMgrBound::Handle_UnanchorStructure()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    PyRep *result(nullptr);

    return result;
}

PyResult PosMgrBound::Handle_AnchorOrbital(PyCallArgs &call) {
    /*
     *  def AnchorOrbital(self, itemID):
     *      posMgr = util.Moniker('posMgr', session.solarsystemid)
     *      posMgr.AnchorOrbital(itemID)
     */

    sLog.White( "PosMgrBound::Handle_()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    PyRep *result(nullptr);

    return result;
}

PyResult PosMgrBound::Handle_UnanchorOrbital(PyCallArgs &call) {
    /*
     *  def UnanchorOrbital(self, itemID):
     *      posMgr = util.Moniker('posMgr', session.solarsystemid)
     *      posMgr.UnanchorOrbital(itemID)
     */
    sLog.White( "PosMgrBound::Handle_UnanchorOrbital()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    PyRep *result(nullptr);

    return result;
}

PyResult PosMgrBound::Handle_OnlineOrbital(PyCallArgs &call) {
    sLog.White( "PosMgrBound::Handle_OnlineOrbital()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    PyRep *result(nullptr);

    return result;
}

PyResult PosMgrBound::Handle_GMUpgradeOrbital(PyCallArgs &call) {
    /*
     *  def GMUpgradeOrbital(self, itemID):
     *      posMgr = util.Moniker('posMgr', session.solarsystemid)
     *      posMgr.GMUpgradeOrbital(itemID)
     */
    sLog.White( "PosMgrBound::Handle_GMUpgradeOrbital()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    PyRep *result(nullptr);

    return result;
}

PyResult PosMgrBound::Handle_AssumeStructureControl(PyCallArgs &call) {
    // NOTE:  this is for controlling pos defences
    /*
        posMgr = moniker.GetPOSMgr()
        posMgr.AssumeStructureControl(item.itemID)
    */
    sLog.White( "PosMgrBound::Handle_AssumeStructureControl()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    PyRep *result(nullptr);

    return result;
}

PyResult PosMgrBound::Handle_RelinquishStructureControl(PyCallArgs &call) {
    /*
        posMgr = moniker.GetPOSMgr()
        posMgr.RelinquishStructureControl(item.itemID)
    */
    sLog.White( "PosMgrBound::Handle_RelinquishStructureControl()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    PyRep *result(nullptr);

    return result;
}

PyResult PosMgrBound::Handle_ChangeStructureProvisionType(PyCallArgs &call) {
    sLog.White( "PosMgrBound::Handle_ChangeStructureProvisionType()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    PyRep *result(nullptr);

    return result;
}

PyResult PosMgrBound::Handle_CompleteOrbitalStateChange(PyCallArgs &call) {
    /*
     *  def CompleteOrbitalStateChange(self, itemID):
     *      posMgr = util.Moniker('posMgr', session.solarsystemid)
     *      posMgr.CompleteOrbitalStateChange(itemID)
     *
     */
    sLog.White( "PosMgrBound::Handle_CompleteOrbitalStateChange()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    PyRep *result(nullptr);

    return result;
}

/*

        [PyString "DoDestinyUpdate"]
        [PySubStream 127 bytes]
          [PyTuple 2 items]
            [PyInt 0]
            [PyTuple 2 items]
              [PyInt 1]
              [PyTuple 2 items]
                [PyList 3 items]
                  [PyTuple 2 items]
                    [PyInt 12193]
                    [PyTuple 2 items]
                      [PyString "SetBallHarmonic"]
                      [PyTuple 5 items]
                        [PyIntegerVar 1002330621081]
                        [PyIntegerVar 8039077077960405911]
                        [PyInt 98038978]
                        [PyInt -1]
                        [PyInt 0]
                  [PyTuple 2 items]
                    [PyInt 12193]
                    [PyTuple 2 items]
                      [PyString "SetBallMassive"]
                      [PyTuple 2 items]
                        [PyIntegerVar 9000000000000038313]
                        [PyInt 1]
                  [PyTuple 2 items]
                    [PyInt 12193]
                    [PyTuple 2 items]
                      [PyString "SetBallHarmonic"]
                      [PyTuple 5 items]
                        [PyIntegerVar 9000000000000038313]
                        [PyIntegerVar 8039077077960405911]
                        [PyInt 98038978]
                        [PyInt -1]
                        [PyInt 1]
                [PyBool False]


*/

PyResult PosMgrBound::Handle_GetMoonProcessInfoForTower(PyCallArgs &call) {
    sLog.White( "PosMgrBound::Handle_GetMoonProcessInfoForTower()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    PyRep *result(nullptr);

    return result;
}

PyResult PosMgrBound::Handle_LinkResourcesForTower(PyCallArgs &call) {
    sLog.White( "PosMgrBound::Handle_LinkResourcesForTower()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    PyRep *result(nullptr);

    return result;
}

PyResult PosMgrBound::Handle_RunMoonProcessCycleforTower(PyCallArgs &call) {
    sLog.White( "PosMgrBound::Handle_RunMoonProcessCycleforTower()", "size=%u", call.tuple->size());
    call.Dump(POS__DUMP);

    PyRep *result(nullptr);

    return result;
}

