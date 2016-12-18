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
#include "pos/PosMgrService.h"

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
        // new call ids from comet0...untested, unverified
        PyCallable_REG_CALL(PosMgrBound, AnchorOrbital);
        PyCallable_REG_CALL(PosMgrBound, UnanchorOrbital);
        PyCallable_REG_CALL(PosMgrBound, OnlineOrbital);
        PyCallable_REG_CALL(PosMgrBound, GMUpgradeOrbital);
        PyCallable_REG_CALL(PosMgrBound, AnchorStructure);
        PyCallable_REG_CALL(PosMgrBound, UnanchorStructure);
        PyCallable_REG_CALL(PosMgrBound, AssumeStructureControl);
        PyCallable_REG_CALL(PosMgrBound, ChangeStructureProvisionType);
        PyCallable_REG_CALL(PosMgrBound, CompleteOrbitalStateChange);
        PyCallable_REG_CALL(PosMgrBound, GetMoonProcessInfoForTower);
        PyCallable_REG_CALL(PosMgrBound, LinkResourcesForTower);
        PyCallable_REG_CALL(PosMgrBound, RelinquishStructureControl);
        PyCallable_REG_CALL(PosMgrBound, RunMoonProcessCycleforTower);
        PyCallable_REG_CALL(PosMgrBound, SetStarbasePermissions);
        PyCallable_REG_CALL(PosMgrBound, SetTowerNotifications);
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
    PyCallable_DECL_CALL(ChangeStructureProvisionType);
    PyCallable_DECL_CALL(CompleteOrbitalStateChange);
    PyCallable_DECL_CALL(GetMoonProcessInfoForTower);
    PyCallable_DECL_CALL(RelinquishStructureControl);
    PyCallable_DECL_CALL(RunMoonProcessCycleforTower);
    PyCallable_DECL_CALL(SetStarbasePermissions);
    PyCallable_DECL_CALL(SetTowerNotifications);
    PyCallable_DECL_CALL(SetTowerSentrySettings);
    PyCallable_DECL_CALL(LinkResourcesForTower);

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
    //        jb = sm.RemoteSvc('posMgr').GetJumpArrays()
    sLog.White( "PosMgrService::Handle_GetJumpArrays()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    return NULL;
}

PyResult PosMgrService::Handle_GetControlTowers(PyCallArgs &call) {
    //    ct = sm.RemoteSvc('posMgr').GetControlTowers()

    sLog.White( "PosMgrService::Handle_GetControlTowers()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    return NULL;
}

PyResult PosMgrService::Handle_UninstallJumpBridgeLink(PyCallArgs &call) {
    /**
     *    def UnbridgePortal(self, itemID):
     *        posLocation = util.Moniker('posMgr', session.solarsystemid)
     *        posLocation.UninstallJumpBridgeLink(itemID)
     *
     */
    sLog.White( "PosMgrService::Handle_UninstallJumpBridgeLink()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    return NULL;
}

PyResult PosMgrService::Handle_InstallJumpBridgeLink(PyCallArgs &call) {
    /**
     *    def BridgePortals(self, localItemID, remoteSolarSystemID, remoteItemID):
     *        posLocation = util.Moniker('posMgr', session.solarsystemid)
     *        posLocation.InstallJumpBridgeLink(localItemID, remoteSolarSystemID, remoteItemID)
     *
     */
    sLog.White( "PosMgrService::Handle_InstallJumpBridgeLink()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    return NULL;
}

PyResult PosMgrService::Handle_GetControlTowerFuelRequirements(PyCallArgs &call) {
    sLog.White( "PosMgrService::Handle_GetControlTowerFuelRequirements()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    return m_db->GetControlTowerFuelRequirements();
}

PyResult PosMgrBound::Handle_GetMoonForTower( PyCallArgs &call ) {
  /*
13:13:06 L PosMgrBound::Handle_GetMoonForTower(): size= 1
13:13:06 [SvcCall]   Call Arguments:
13:13:06 [SvcCall]       Tuple: 1 elements
13:13:06 [SvcCall]         [ 0] Integer field: 140001260

  sLog.White( "PosMgrBound::Handle_GetMoonForTower()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALL_DUMP);
*/
  PyRep *result = NULL;

    return result;
}

PyResult PosMgrBound::Handle_SetTowerPassword( PyCallArgs &call ) {
  /*
13:10:09 L PosMgrBound::Handle_SetTowerPassword(): size= 2
13:10:09 [SvcCall]   Call Arguments:
13:10:09 [SvcCall]       Tuple: 2 elements
13:10:09 [SvcCall]         [ 0] Integer field: 140001260    << shipID
13:10:09 [SvcCall]         [ 1] WString: 'test'             << password

  sLog.White( "PosMgrBound::Handle_SetTowerPassword()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALL_DUMP);
*/
  PyRep *result = NULL;

    return result;
}

PyResult PosMgrBound::Handle_SetShipPassword( PyCallArgs &call ) {
  /*
13:16:17 L PosMgrBound::Handle_SetShipPassword(): size= 1
13:16:17 [SvcCall]   Call Arguments:
13:16:17 [SvcCall]       Tuple: 1 elements
13:16:17 [SvcCall]         [ 0] WString: 'test'             << password

  sLog.White( "PosMgrBound::Handle_SetShipPassword()", "size= %u", call.tuple->size() );
  call.Dump(SERVICE__CALL_DUMP);
*/

  PyRep *result = NULL;

    return result;
}

PyResult PosMgrBound::Handle_GetSiloCapacityByItemID(PyCallArgs &call) {
  sLog.White( "PosMgrBound::Handle_GetSiloCapacityByItemID()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    uint32 itemID = 0;

    return m_db->GetSiloCapacityByItemID(itemID);
}

PyResult PosMgrBound::Handle_AnchorStructure(PyCallArgs &call) {
    /*  structure state is queried by StructureEntity::GetPOSState()
     *    state is saved in StructureEntity::m_state (POSState)
     */
    sLog.White( "PosMgrBound::Handle_AnchorStructure()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    PyRep *result = NULL;

    return result;
}

PyResult PosMgrBound::Handle_UnanchorStructure(PyCallArgs &call) {
    sLog.White( "PosMgrBound::Handle_UnanchorStructure()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    PyRep *result = NULL;

    return result;
}

PyResult PosMgrBound::Handle_AnchorOrbital(PyCallArgs &call) {
    /*
     *  def AnchorOrbital(self, itemID):
     *      posMgr = util.Moniker('posMgr', session.solarsystemid)
     *      posMgr.AnchorOrbital(itemID)
     */

    sLog.White( "PosMgrBound::Handle_()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    PyRep *result = NULL;

    return result;
}

PyResult PosMgrBound::Handle_UnanchorOrbital(PyCallArgs &call) {
    /*
     *  def UnanchorOrbital(self, itemID):
     *      posMgr = util.Moniker('posMgr', session.solarsystemid)
     *      posMgr.UnanchorOrbital(itemID)
     */
    sLog.White( "PosMgrBound::Handle_UnanchorOrbital()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    PyRep *result = NULL;

    return result;
}

PyResult PosMgrBound::Handle_OnlineOrbital(PyCallArgs &call) {
    sLog.White( "PosMgrBound::Handle_OnlineOrbital()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    PyRep *result = NULL;

    return result;
}

PyResult PosMgrBound::Handle_GMUpgradeOrbital(PyCallArgs &call) {
    /*
     *  def GMUpgradeOrbital(self, itemID):
     *      posMgr = util.Moniker('posMgr', session.solarsystemid)
     *      posMgr.GMUpgradeOrbital(itemID)
     */
    sLog.White( "PosMgrBound::Handle_GMUpgradeOrbital()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    PyRep *result = NULL;

    return result;
}

PyResult PosMgrBound::Handle_AssumeStructureControl(PyCallArgs &call) {
    sLog.White( "PosMgrBound::Handle_AssumeStructureControl()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    PyRep *result = NULL;

    return result;
}

PyResult PosMgrBound::Handle_ChangeStructureProvisionType(PyCallArgs &call) {
    sLog.White( "PosMgrBound::Handle_ChangeStructureProvisionType()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    PyRep *result = NULL;

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
    call.Dump(SERVICE__CALL_DUMP);

    PyRep *result = NULL;

    return result;
}

PyResult PosMgrBound::Handle_SetTowerNotifications(PyCallArgs &call) {
    sLog.White( "PosMgrBound::Handle_SetTowerNotifications()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    PyRep *result = NULL;

    return result;
}

PyResult PosMgrBound::Handle_SetTowerSentrySettings(PyCallArgs &call) {
    sLog.White( "PosMgrBound::Handle_SetTowerSentrySettings()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    PyRep *result = NULL;

    return result;
}

/*
==================== Sent from Client 117 bytes

[PyObjectData Name: macho.CallReq]
  [PyTuple 6 items]
    [PyInt 6]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 2]
        [PyInt 0]
        [PyIntegerVar 517]
        [PyNone]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 1]
        [PyIntegerVar 700228]
        [PyNone]
        [PyNone]
    [PyInt 5654387]
    [PyTuple 1 items]
      [PyTuple 2 items]
        [PyInt 1]
        [PySubStream 68 bytes]
          [PyTuple 4 items]
            [PyString "N=700228:34279"]
            [PyString "SetTowerPassword"]
            [PyTuple 4 items]
              [PyIntegerVar 1002332856217]
              [PyString "password"]
              [PyBool True]
              [PyBool False]
            [PyDict 1 kvp]
              [PyString "machoVersion"]
              [PyInt 1]
    [PyNone]



==================== Sent from Server 60 bytes

[PyObjectData Name: macho.CallRsp]
  [PyTuple 6 items]
    [PyInt 7]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 1]
        [PyIntegerVar 700228]
        [PyNone]
        [PyNone]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 2]
        [PyIntegerVar 12501000001023]
        [PyIntegerVar 517]
        [PyNone]
    [PyInt 5654387]
    [PyTuple 1 items]
      [PySubStream 6 bytes]
        [PyNone]
    [PyNone]



==================== Sent from Server 214 bytes

[PyObjectData Name: macho.Notification]
  [PyTuple 6 items]
    [PyInt 12]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 1]
        [PyInt 700228]
        [PyNone]
        [PyNone]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 4]
        [PyString "DoDestinyUpdate"]
        [PyList 0 items]
        [PyString "clientID"]
    [PyInt 5654387]
    [PyTuple 1 items]
      [PyTuple 2 items]
        [PyInt 0]
        [PySubStream 160 bytes]
          [PyTuple 2 items]
            [PyInt 0]
            [PyTuple 2 items]
              [PyInt 1]
              [PyTuple 3 items]
                [PyList 1 items]
                  [PyTuple 2 items]
                    [PyInt 12193]
                    [PyTuple 2 items]
                      [PyString "PackagedAction"]
                      [PySubStream 123 bytes]
                        [PyList 1 items]
                          [PyTuple 2 items]
                            [PyInt 12193]
                            [PyTuple 2 items]
                              [PyString "AddBalls2"]
                              [PyTuple 1 items]
                                [PyTuple 2 items]
                                  [Destiny Header]
                                    [PacketType: 1]
                                    [Stamp: 12193]
                                  [Ball]
                                    [Name: ]
                                    [FormationId: 255]
                                    [Header]
                                      [ItemId: 9000000000000038313]
                                      [Mode: Stop (2)]
                                      [Flags: 0 (0)]
                                      [Radius: 30000]
                                      [Location: (-29259571200, -487710720, 55060439040)]
                                    [ExtraHeader]
                                      [AllianceId: -1]
                                      [CorporationId: 98038978]
                                      [CloakMode: 0]
                                      [Harmonic: NaN]
                                      [Mass: 10000000000]

                                  [PyList 1 items]
                                    [PyDict 3 kvp]
                                      [PyString "itemID"]
                                      [PyIntegerVar 9000000000000038313]
                                      [PyString "typeID"]
                                      [PyInt 16103]
                                      [PyString "ownerID"]
                                      [PyInt 98038978]
                [PyBool True]
                [PyList 0 items]
    [PyDict 1 kvp]
      [PyString "sn"]
      [PyIntegerVar 85]



==================== Sent from Server 181 bytes

[PyObjectData Name: macho.Notification]
  [PyTuple 6 items]
    [PyInt 12]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 1]
        [PyInt 700228]
        [PyNone]
        [PyNone]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 4]
        [PyString "DoDestinyUpdate"]
        [PyList 0 items]
        [PyString "clientID"]
    [PyInt 5654387]
    [PyTuple 1 items]
      [PyTuple 2 items]
        [PyInt 0]
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
    [PyDict 1 kvp]
      [PyString "sn"]
      [PyIntegerVar 86]



==================== Sent from Client 110 bytes

[PyObjectData Name: macho.CallReq]
  [PyTuple 6 items]
    [PyInt 6]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 2]
        [PyInt 0]
        [PyIntegerVar 518]
        [PyNone]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 1]
        [PyIntegerVar 700228]
        [PyNone]
        [PyNone]
    [PyInt 5654387]
    [PyTuple 1 items]
      [PyTuple 2 items]
        [PyInt 1]
        [PySubStream 61 bytes]
          [PyTuple 4 items]
            [PyString "N=700228:34279"]
            [PyString "GetTowerSentrySettings"]
            [PyTuple 1 items]
              [PyIntegerVar 1002332856217]
            [PyDict 1 kvp]
              [PyString "machoVersion"]
              [PyInt 1]
    [PyNone]



==================== Sent from Server 146 bytes

[PyObjectData Name: macho.CallRsp]
  [PyTuple 6 items]
    [PyInt 7]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 1]
        [PyIntegerVar 700228]
        [PyNone]
        [PyNone]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 2]
        [PyIntegerVar 12501000001023]
        [PyIntegerVar 518]
        [PyNone]
    [PyInt 5654387]
    [PyTuple 1 items]
      [PySubStream 92 bytes]
        [PyObjectData Name: util.Row]
          [PyDict 2 kvp]
            [PyString "header"]
            [PyList 6 items]
              [PyString "standing"]
              [PyString "status"]
              [PyString "statusDrop"]
              [PyString "aggression"]
              [PyString "corpWar"]
              [PyString "standingOwnerID"]
            [PyString "line"]
            [PyList 6 items]
              [PyNone]
              [PyNone]
              [PyNone]
              [PyNone]
              [PyNone]
              [PyNone]
    [PyNone]



==================== Sent from Client 133 bytes

[PyObjectData Name: macho.CallReq]
  [PyTuple 6 items]
    [PyInt 6]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 2]
        [PyInt 0]
        [PyIntegerVar 519]
        [PyNone]
    [PyObjectData Name: macho.MachoAddress]
      [PyTuple 4 items]
        [PyInt 1]
        [PyIntegerVar 700228]
        [PyNone]
        [PyNone]
    [PyInt 5654387]
    [PyTuple 1 items]
      [PyTuple 2 items]
        [PyInt 1]
        [PySubStream 84 bytes]
          [PyTuple 4 items]
            [PyString "N=700228:34279"]
            [PyString "SetTowerSentrySettings"]
            [PyTuple 7 items]
              [PyIntegerVar 1002332856217]
              [PyFloat 0.1]
              [PyFloat 0.2]
              [PyBool True]
              [PyBool True]
              [PyBool True]
              [PyInt 0]
            [PyDict 1 kvp]
              [PyString "machoVersion"]
              [PyInt 1]
    [PyNone]

*/

PyResult PosMgrBound::Handle_GetMoonProcessInfoForTower(PyCallArgs &call) {
    sLog.White( "PosMgrBound::Handle_GetMoonProcessInfoForTower()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    PyRep *result = NULL;

    return result;
}

PyResult PosMgrBound::Handle_LinkResourcesForTower(PyCallArgs &call) {
    sLog.White( "PosMgrBound::Handle_LinkResourcesForTower()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    PyRep *result = NULL;

    return result;
}

PyResult PosMgrBound::Handle_RelinquishStructureControl(PyCallArgs &call) {
    sLog.White( "PosMgrBound::Handle_RelinquishStructureControl()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    PyRep *result = NULL;

    return result;
}

PyResult PosMgrBound::Handle_RunMoonProcessCycleforTower(PyCallArgs &call) {
    sLog.White( "PosMgrBound::Handle_RunMoonProcessCycleforTower()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    PyRep *result = NULL;

    return result;
}

PyResult PosMgrBound::Handle_SetStarbasePermissions(PyCallArgs &call) {
    sLog.White( "PosMgrBound::Handle_SetStarbasePermissions()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    PyRep *result = NULL;

    return result;
}


/*
                state = slimItem.orbitalState
                if state in (entities.STATE_UNANCHORING,
                 entities.STATE_ONLINING,
                 entities.STATE_ANCHORING,
                 entities.STATE_OPERATING,
                 entities.STATE_OFFLINING,
                 entities.STATE_SHIELD_REINFORCE):
                    stateText = pos.DISPLAY_NAMES[pos.Entity2DB(state)]
                    gm.append(('End orbital state change (%s)' % stateText, self.CompleteOrbitalStateChange, (itemID,)))
                elif state == entities.STATE_ANCHORED:
                    upgradeType = sm.GetService('godma').GetTypeAttribute2(slimItem.typeID, const.attributeConstructionType)
                    if upgradeType is not None:
                        gm.append(('Upgrade to %s' % cfg.invtypes.Get(upgradeType).typeName, self.GMUpgradeOrbital, (itemID,)))
                gm.append(('GM: Take Control', self.TakeOrbitalOwnership, (itemID, slimItem.planetID)))
                */

/*
    def GetGMStructureStateMenu(self, itemID = None, slimItem = None, charID = None, invItem = None, mapItem = None):
        subMenu = []
        if hasattr(slimItem, 'posState') and slimItem.posState is not None:
            currentState = slimItem.posState
            if currentState not in pos.ONLINE_STABLE_STATES:
                if currentState == pos.STRUCTURE_ANCHORED:
                    subMenu.append(('Online', sm.RemoteSvc('slash').SlashCmd, ('/pos online ' + str(itemID),)))
                    subMenu.append(('Unanchor', sm.RemoteSvc('slash').SlashCmd, ('/pos unanchor ' + str(itemID),)))
                elif currentState == pos.STRUCTURE_UNANCHORED:
                    subMenu.append(('Anchor', sm.RemoteSvc('slash').SlashCmd, ('/pos anchor ' + str(itemID),)))
            else:
                if getattr(slimItem, 'posTimestamp', None) is not None:
                    subMenu.append(('Complete State', sm.RemoteSvc('slash').SlashCmd, ('/sov complete ' + str(itemID),)))
                subMenu.append(('Offline', sm.RemoteSvc('slash').SlashCmd, ('/pos offline ' + str(itemID),)))
        if hasattr(slimItem, 'structureState') and slimItem.structureState != None and slimItem.structureState in [pos.STRUCTURE_SHIELD_REINFORCE, pos.STRUCTURE_ARMOR_REINFORCE]:
            subMenu.append(('Complete State', sm.RemoteSvc('slash').SlashCmd, ('/sov complete ' + str(itemID),)))
        return subMenu
*/