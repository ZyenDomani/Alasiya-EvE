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
    Author:        Zhur
    Updates:    Allan (rewrite)
*/

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "cache/ObjCacheService.h"
#include "dogmaim/DogmaIMService.h"
#include "pos/Structure.h"
#include "ship/modules/GenericModule.h"
#include "system/Container.h"
#include "system/SystemManager.h"

/** @todo this is actually DogmaLM (Location Manager) for bound objecs... */
class DogmaIMBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(DogmaIMBound)

    DogmaIMBound(PyServiceMgr* mgr, uint32 locationID, uint32 groupID)
    : PyBoundObject(mgr),
    m_dispatch(new Dispatcher(this)),
    m_locationID(locationID),
    m_groupID(groupID)
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "DogmaIMBound";

        PyCallable_REG_CALL(DogmaIMBound, ChangeDroneSettings);
        PyCallable_REG_CALL(DogmaIMBound, LinkWeapons);
        PyCallable_REG_CALL(DogmaIMBound, LinkAllWeapons);
        PyCallable_REG_CALL(DogmaIMBound, UnlinkModule);
        PyCallable_REG_CALL(DogmaIMBound, UnlinkAllModules);
        PyCallable_REG_CALL(DogmaIMBound, OverloadRack);
        PyCallable_REG_CALL(DogmaIMBound, StopOverloadRack);
        PyCallable_REG_CALL(DogmaIMBound, ShipGetInfo);
        PyCallable_REG_CALL(DogmaIMBound, CharGetInfo);
        PyCallable_REG_CALL(DogmaIMBound, ItemGetInfo);
		PyCallable_REG_CALL(DogmaIMBound, GetAllInfo);
		PyCallable_REG_CALL(DogmaIMBound, GetLocationInfo);
        PyCallable_REG_CALL(DogmaIMBound, DestroyWeaponBank);
        PyCallable_REG_CALL(DogmaIMBound, GetCharacterBaseAttributes);
		PyCallable_REG_CALL(DogmaIMBound, CheckSendLocationInfo);
        PyCallable_REG_CALL(DogmaIMBound, Activate);
        PyCallable_REG_CALL(DogmaIMBound, Deactivate);
        PyCallable_REG_CALL(DogmaIMBound, Overload);
        PyCallable_REG_CALL(DogmaIMBound, StopOverload);
        PyCallable_REG_CALL(DogmaIMBound, CancelOverloading);
		PyCallable_REG_CALL(DogmaIMBound, SetModuleOnline);
        PyCallable_REG_CALL(DogmaIMBound, TakeModuleOffline);
        PyCallable_REG_CALL(DogmaIMBound, LoadAmmoToBank);
        PyCallable_REG_CALL(DogmaIMBound, LoadAmmoToModules);
        PyCallable_REG_CALL(DogmaIMBound, GetTargets);
        PyCallable_REG_CALL(DogmaIMBound, GetTargeters);
        PyCallable_REG_CALL(DogmaIMBound, AddTarget);       //AddTargetOBO
        PyCallable_REG_CALL(DogmaIMBound, RemoveTarget);
        PyCallable_REG_CALL(DogmaIMBound, ClearTargets);
        PyCallable_REG_CALL(DogmaIMBound, InitiateModuleRepair);
        PyCallable_REG_CALL(DogmaIMBound, StopModuleRepair);
        PyCallable_REG_CALL(DogmaIMBound, MergeModuleGroups);
        PyCallable_REG_CALL(DogmaIMBound, PeelAndLink);
    }
    virtual ~DogmaIMBound() {delete m_dispatch;}
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(ChangeDroneSettings);
    PyCallable_DECL_CALL(LinkWeapons);
    PyCallable_DECL_CALL(LinkAllWeapons);
    PyCallable_DECL_CALL(UnlinkModule);
    PyCallable_DECL_CALL(UnlinkAllModules);
    PyCallable_DECL_CALL(OverloadRack);
    PyCallable_DECL_CALL(StopOverloadRack);
    PyCallable_DECL_CALL(ShipGetInfo);
    PyCallable_DECL_CALL(CharGetInfo);
    PyCallable_DECL_CALL(ItemGetInfo);
	PyCallable_DECL_CALL(GetAllInfo);
    PyCallable_DECL_CALL(DestroyWeaponBank);
    PyCallable_DECL_CALL(GetLocationInfo);
    PyCallable_DECL_CALL(GetCharacterBaseAttributes);
	PyCallable_DECL_CALL(CheckSendLocationInfo);
    PyCallable_DECL_CALL(Activate);
    PyCallable_DECL_CALL(Deactivate);
    PyCallable_DECL_CALL(Overload);
    PyCallable_DECL_CALL(StopOverload);
    PyCallable_DECL_CALL(CancelOverloading);
	PyCallable_DECL_CALL(SetModuleOnline);
    PyCallable_DECL_CALL(TakeModuleOffline);
    PyCallable_DECL_CALL(LoadAmmoToBank);
    PyCallable_DECL_CALL(LoadAmmoToModules);
    PyCallable_DECL_CALL(GetTargets);
    PyCallable_DECL_CALL(GetTargeters);
    PyCallable_DECL_CALL(AddTarget);
    PyCallable_DECL_CALL(RemoveTarget);
    PyCallable_DECL_CALL(ClearTargets);
    PyCallable_DECL_CALL(InitiateModuleRepair);
    PyCallable_DECL_CALL(StopModuleRepair);
    PyCallable_DECL_CALL(MergeModuleGroups);
    PyCallable_DECL_CALL(PeelAndLink);

    /*  OBO == ??  (pos targeting)
     * flag, targetList = self.GetDogmaLM().AddTargetOBO(sid, tid) (structureID, targetID)
     * self.GetDogmaLM().RemoveTargetOBO(sid, tid)  (structureID, targetID)
    */
protected:
    Dispatcher* const m_dispatch;

    uint32 m_locationID;
    uint32 m_groupID;
};

PyCallable_Make_InnerDispatcher(DogmaIMService)


DogmaIMService::DogmaIMService(PyServiceMgr* mgr)
: PyService(mgr, "dogmaIM"),  // IM = Instance Manager
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(DogmaIMService, GetAttributeTypes);
}

DogmaIMService::~DogmaIMService() {
    delete m_dispatch;
}

PyResult DogmaIMService::Handle_GetAttributeTypes(PyCallArgs& call) {
    PyString* str = new PyString("dogmaIM.attributesByName" );
    PyRep* result = m_manager->cache_service->GetCacheHint( str );
    PyDecRef( str );
    return result;
}

PyBoundObject* DogmaIMService::_CreateBoundObject(Client* c, const PyRep* bind_args) {
    _log(CLIENT__MESSAGE, "DogmaIMService bind request for:");
    bind_args->Dump(CLIENT__MESSAGE, "    ");

    DogmaLM_BindArgs args;
    //temp crap until I rework _CreateBoundObject's signature
    PyRep *t = bind_args->Clone();
    if(!args.Decode(&t)) {
        codelog(INV__ERROR, "Failed to decode bind args from '%s'", c->GetName());
        return nullptr;
    }

    return new DogmaIMBound(m_manager, args.locationID, args.groupID);
}

PyResult DogmaIMBound::Handle_CharGetInfo(PyCallArgs& call) {
    return call.client->GetChar()->GetCharInfo();
}

PyResult DogmaIMBound::Handle_ShipGetInfo(PyCallArgs& call) {
    return call.client->GetShip()->ShipGetInfo();
}

PyResult DogmaIMBound::Handle_ClearTargets(PyCallArgs& call) {
    call.client->GetShipSE()->TargetMgr()->ClearTargets();
    return nullptr;
}

PyResult DogmaIMBound::Handle_GetTargets(PyCallArgs& call) {
    return call.client->GetShipSE()->TargetMgr()->GetTargets();
}

PyResult DogmaIMBound::Handle_GetTargeters(PyCallArgs& call) {
    return call.client->GetShipSE()->TargetMgr()->GetTargeters();
}

PyResult DogmaIMBound::Handle_GetCharacterBaseAttributes(PyCallArgs& call)
{
    CharacterRef cref = call.client->GetChar();
    uint8 mod = sConfig.character.statMultiplier;

    PyDict* result = new PyDict();
    result->SetItem(new PyInt(AttrIntelligence), new PyInt(static_cast<int32>(cref->GetAttribute(AttrIntelligence).get_int() * mod)));
    result->SetItem(new PyInt(AttrPerception), new PyInt(static_cast<int32>(cref->GetAttribute(AttrPerception).get_int() * mod)));
    result->SetItem(new PyInt(AttrCharisma), new PyInt(static_cast<int32>(cref->GetAttribute(AttrCharisma).get_int() * mod)));
    result->SetItem(new PyInt(AttrWillpower), new PyInt(static_cast<int32>(cref->GetAttribute(AttrWillpower).get_int() * mod)));
    result->SetItem(new PyInt(AttrMemory), new PyInt(static_cast<int32>(cref->GetAttribute(AttrMemory).get_int() * mod)));
    return result;
}


PyResult DogmaIMBound::Handle_ItemGetInfo(PyCallArgs& call) {
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return PyStatic.NewNone();
    }

    InventoryItemRef itemRef = sItemFactory.GetItem(args.arg);
    if (itemRef.get() == nullptr ) {
        _log(INV__ERROR, "Unable to load item %u", args.arg);
        return PyStatic.NewNone();
    }

    return itemRef->ItemGetInfo();
}

PyResult DogmaIMBound::Handle_SetModuleOnline(PyCallArgs& call) {
    Client* pClient(call.client);

    if (pClient->IsInSpace()) {
        DestinyManager* pDestiny = pClient->GetShipSE()->DestinyMgr();
        if (pDestiny == nullptr) {
            _log(PLAYER__ERROR, "%s: Client has no destiny manager!", pClient->GetName());
            return PyStatic.NewNone();
        } else if (pDestiny->IsWarping()) {
            /** @todo  warpsafe modules can be activated while warping.  fix this */
            pClient->SendNotifyMsg("You can't do this while warping");
            return PyStatic.NewNone();
        }
    }

	Call_TwoIntegerArgs args; //locationID, moduleID

	if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return PyStatic.NewNone();
    }

	pClient->GetShip()->Online(args.arg2);

    // returns nodeID and timestamp
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
        tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    return tuple;
}

PyResult DogmaIMBound::Handle_TakeModuleOffline(PyCallArgs& call) {
    Client* pClient(call.client);

    if (pClient->IsInSpace()) {
        DestinyManager* pDestiny = pClient->GetShipSE()->DestinyMgr();
        if (pDestiny == nullptr) {
            _log(PLAYER__ERROR, "%s: Client has no destiny manager!", pClient->GetName());
            return PyStatic.NewNone();
        } else if (pDestiny->IsWarping()) {
            /** @todo  warpsafe modules can be deactivated while warping.  fix this */
            pClient->SendNotifyMsg("You can't do this while warping");
            return PyStatic.NewNone();
        }
    }

	Call_TwoIntegerArgs args; //locationID, moduleID
	if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return PyStatic.NewNone();
    }

	pClient->GetShip()->Offline(args.arg2);

    // returns nodeID and timestamp
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
        tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    return tuple;
}

PyResult DogmaIMBound::Handle_LoadAmmoToModules(PyCallArgs& call) {
    //  self.remoteDogmaLM.LoadAmmoToModules(shipID, moduleIDs, chargeTypeID, itemID, ammoLocationID, qty=qty)
    //  NOTE:  this call seems to be a list of moduleIDs with ONLY a single module.
    /* 02:13:11 [SvcCall]       Tuple: 5 elements
     * 02:13:11 [SvcCall]         [ 0] Integer field: 140000602     <- ship
     * 02:13:11 [SvcCall]         [ 1] List: 1 elements
     * 02:13:11 [SvcCall]         [ 1]   [ 0] Integer field: 140000454  <- moduleList
     * 02:13:11 [SvcCall]         [ 2] Integer field: 196   <- charge type
     * 02:13:11 [SvcCall]         [ 3] Integer field: 140000623 <- charge item
     * 02:13:11 [SvcCall]         [ 4] Integer field: 60014749  <- charge location
     * 02:13:11 [SvcCall]   Call Named Arguments:
     * 02:13:11 [SvcCall]     Argument 'qty':
     * 02:13:11 [SvcCall]         (None)
     */
    call.Dump(SHIP__MODULE_TRACE);
    Call_Dogma_LoadAmmoToModules args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    if (args.moduleIDs.empty())
        return nullptr;
    if (args.moduleIDs.size() > 1) {
        sLog.Error("DogmaIMBound::Handle_LoadAmmoToModules()", "args.moduleIDs.size = %u.", args.moduleIDs.size() );
        call.Dump(SHIP__MODULE_WARNING);
    }

    // Get Reference to Ship and Charge
    ShipItemRef sRef = call.client->GetShip();
    InventoryItemRef cRef = sItemFactory.GetItem(args.itemID);
    if (cRef.get() == nullptr)
        throw PyException( MakeUserError( "CantFindChargeToAdd"));
    GenericModule* pMod = sRef->GetModule(sItemFactory.GetItem(args.moduleIDs[0])->flag());
    if (pMod == nullptr)
        throw PyException( MakeUserError( "ModuleNoLongerPresentForCharges"));

    if (pMod->IsLinked())
        sRef->LoadLinkedWeapons(cRef, pMod);
    else
        sRef->LoadCharge(cRef, pMod->flag());

    return nullptr;
}

PyResult DogmaIMBound::Handle_LoadAmmoToBank(PyCallArgs& call) {
  /*   NOTE:  this to load ALL modules in weapon bank, if possible.
   * self.remoteDogmaLM.LoadAmmoToBank(shipID, masterID, chargeTypeID,  itemIDs,  chargeLocationID,   qty)
   *                                    ship,   module,  charge type, charge item, charge location, stack qty (usually none - havent found otherwise)
   *   *******    UPDATED VAR NAMES TO MATCH CLIENT CODE  -allan 26Jul14  *************
   */
  call.Dump(SHIP__MODULE_TRACE);
	Call_Dogma_LoadAmmoToBank args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }
    /*
    args.chargeLocationID
    args.chargeTypeID
    args.itemIDs
    args.masterID
    args.shipID
    args.qty
    */
    if (args.itemIDs.empty())
        return nullptr;

	// Get Reference to Ship, Module, and Charge
    ShipItemRef sRef = call.client->GetShip();
    if (sRef->itemID() != args.shipID)
        sLog.Error("DogmaIMBound::Handle_LoadAmmoToBank()", "passed shipID %u != current shipID %u.", args.shipID, sRef->itemID() );

    GenericModule* pMod = sRef->GetModule(sItemFactory.GetItem(args.masterID)->flag());
    if (pMod == nullptr)
        throw PyException( MakeUserError("ModuleNoLongerPresentForCharges"));

	if (pMod->IsLinked())
        sRef->LoadLinkedWeapons(pMod, args.itemIDs);
    else
        sRef->LoadCharge(sItemFactory.GetItem(args.itemIDs.at(0)), pMod->flag());
        //sRef->LoadChargesToBank(pMod->flag(), args.itemIDs);

    return nullptr;
}

    /* {'messageKey': 'DeniedDroneTargetForceField', 'dataID': 17877756, 'suppressable': False, 'bodyID': 257373, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2880}
     * {'messageKey': 'DeniedInvulnerable', 'dataID': 17883409, 'suppressable': False, 'bodyID': 259494, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 779}
     * {'messageKey': 'DeniedStationInvulnerableSovereign', 'dataID': 17883713, 'suppressable': False, 'bodyID': 259600, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 780}
     * {'messageKey': 'DeniedTargetAfterCloak', 'dataID': 17883412, 'suppressable': False, 'bodyID': 259495, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 781}
     * {'messageKey': 'DeniedTargetEvadesSensors', 'dataID': 17883870, 'suppressable': False, 'bodyID': 259657, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 782}
     * {'messageKey': 'DeniedTargetForceField', 'dataID': 17883882, 'suppressable': False, 'bodyID': 259661, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 783}
     * {'messageKey': 'DeniedTargetInvulnerable', 'dataID': 17883415, 'suppressable': False, 'bodyID': 259496, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 784}
     * {'messageKey': 'DeniedTargetOtherFrozen', 'dataID': 17883876, 'suppressable': False, 'bodyID': 259659, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 785}
     * {'messageKey': 'DeniedTargetOtherWarping', 'dataID': 17883809, 'suppressable': False, 'bodyID': 259635, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 786}
     * {'messageKey': 'DeniedTargetReinforcedStructure', 'dataID': 17883888, 'suppressable': False, 'bodyID': 259663, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 787}
     * {'messageKey': 'DeniedTargetSelf', 'dataID': 17883418, 'suppressable': False, 'bodyID': 259497, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 788}
     * {'messageKey': 'DeniedTargetSelfFrozen', 'dataID': 17883873, 'suppressable': False, 'bodyID': 259658, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 789}
     * {'messageKey': 'DeniedTargetSelfWarping', 'dataID': 17883879, 'suppressable': False, 'bodyID': 259660, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 790}
     * {'messageKey': 'DeniedTargetUntargetable', 'dataID': 17880323, 'suppressable': False, 'bodyID': 258348, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2170}
     * {'messageKey': 'DeniedTargetingAttemptFailed', 'dataID': 17883942, 'suppressable': False, 'bodyID': 259683, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 791}
     * {'messageKey': 'DeniedTargetingCloaked', 'dataID': 17883664, 'suppressable': False, 'bodyID': 259583, 'messageType': 'notify', 'urlAudio': 'wise:/msg_DeniedTargetingCloaked_play', 'urlIcon': '', 'titleID': None, 'messageID': 792}
     * {'messageKey': 'DeniedTargetingInsideField', 'dataID': 17883885, 'suppressable': False, 'bodyID': 259662, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 793}
     * {'messageKey': 'DeniedTargetingTargetCloaked', 'dataID': 17883421, 'suppressable': False, 'bodyID': 259498, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 794}
     * {'FullPath': u'UI/Messages', 'messageID': 259494, 'label': u'DeniedInvulnerableBody'}(u'Your ship is realigning its magnetic field, please wait a moment.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 259495, 'label': u'DeniedTargetAfterCloakBody'}(u'You cannot perform that action at this time as your systems are still recalibrating after the use of a cloaking device.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 259496, 'label': u'DeniedTargetInvulnerableBody'}(u'Target is invulnerable.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 259497, 'label': u'DeniedTargetSelfBody'}(u'You cannot target your own ship.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 259498, 'label': u'DeniedTargetingTargetCloakedBody'}(u'You failed to target nothing.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 257373, 'label': u'DeniedDroneTargetForceFieldBody'}(u'Your drones cannot engage {[item]target.name} since it is within the range of a forcefield you are barred from entering.', None, {u'{[item]target.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'target'}})
     * {'FullPath': u'UI/Messages', 'messageID': 259600, 'label': u'DeniedStationInvulnerableSovereignBody'}(u'{[item]module.name} deactivates as the target {targetname} is invulnerable because its owning alliance holds sovereignty in this solar system.', None, {u'{targetname}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetname'}, u'{[item]module.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'module'}})
     * {'FullPath': u'UI/Messages', 'messageID': 258348, 'label': u'DeniedTargetUntargetableBody'}(u'You are unable to target {targetName} as it has been made untargetable by a GM.', None, {u'{targetName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetName'}})
     * {'FullPath': u'UI/Messages', 'messageID': 259583, 'label': u'DeniedTargetingCloakedBody'}(u'You cannot target anything while you are cloaked.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 259635, 'label': u'DeniedTargetOtherWarpingBody'}(u"Interference from {targetName}'s warp prevents your sensors from locking the target.", None, {u'{targetName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetName'}})
     * {'FullPath': u'UI/Messages', 'messageID': 259657, 'label': u'DeniedTargetEvadesSensorsBody'}(u'You are unable to target the {targetName} as your sensors are unable to lock onto it.', None, {u'{targetName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetName'}})
     * {'FullPath': u'UI/Messages', 'messageID': 259658, 'label': u'DeniedTargetSelfFrozenBody'}(u'You are unable to target {targetName} because you have been frozen by a GM.', None, {u'{targetName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetName'}})
     * {'FullPath': u'UI/Messages', 'messageID': 259659, 'label': u'DeniedTargetOtherFrozenBody'}(u'You are unable to target {targetName} because they are currently frozen by a GM.', None, {u'{targetName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetName'}})
     * {'FullPath': u'UI/Messages', 'messageID': 259660, 'label': u'DeniedTargetSelfWarpingBody'}(u'Interference from the warp you are doing is preventing your sensors from getting a target lock on {targetName}.', None, {u'{targetName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetName'}})
     * {'FullPath': u'UI/Messages', 'messageID': 259661, 'label': u'DeniedTargetForceFieldBody'}(u'You failed to target {[item]target.name}, they are within range {[numeric]range.distance} of a {[item]item.name} and with you being outside of it, it is preventing you from holding a lock on them.', None, {u'{[item]target.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]item.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{[numeric]range.distance}': {'conditionalValues': [], 'variableType': 9, 'propertyName': 'distance', 'args': 256, 'kwargs': {}, 'variableName': 'range'}})
     * {'FullPath': u'UI/Messages', 'messageID': 259662, 'label': u'DeniedTargetingInsideFieldBody'}(u'You cannot target the {[item]target.name} while you are inside a force field.', None, {u'{[item]target.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'target'}})
     * {'FullPath': u'UI/Messages', 'messageID': 259663, 'label': u'DeniedTargetReinforcedStructureBody'}(u'You failed to target {[item]target.name} as it is locked down in reinforced mode.', None, {u'{[item]target.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'target'}})
     * {'FullPath': u'UI/Messages', 'messageID': 259683, 'label': u'DeniedTargetingAttemptFailedBody'}(u'Your attempt to target {[item]target.name} failed.', None, {u'{[item]target.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'target'}})
     *
     */
PyResult DogmaIMBound::Handle_AddTarget(PyCallArgs& call) {
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return PyStatic.NewNone();
    }
    Client* pClient(call.client);
    if (pClient->IsJump())
        throw PyException( MakeUserError( "CantTargetWhileJumping"));

    if (pClient->GetShipID() == args.arg)
        throw PyException( MakeUserError( "DeniedTargetSelf"));

    Rsp_Dogma_AddTarget rsp;
        rsp.flag = false;
        rsp.targetList.push_back(args.arg);

    if (!pClient->IsInSpace()) {
        pClient->SendNotifyMsg("You can't do this while docked");
        return rsp.Encode();
    }
    Ship* pShip = pClient->GetShipSE();

    DestinyManager* pDestiny = pShip->DestinyMgr();
    if (pDestiny == nullptr) {
        _log(PLAYER__ERROR, "%s: Client has no destiny manager!", pClient->GetName());
        return rsp.Encode();
    }
    if (pDestiny->IsWarping())
        throw PyException( MakeUserError( "DeniedTargetSelfWarping"));

    if (pDestiny->IsCloaked())
        throw PyException( MakeUserError( "CantTargetWhileCloaked")); //DeniedTargetingCloaked

    if (pShip->TargetMgr() == nullptr)
        return rsp.Encode();

    SystemManager* pSysMgr = pClient->SystemMgr();
    if (pSysMgr == nullptr) {
        _log(PLAYER__WARNING, "Unable to find system manager from '%s'", pClient->GetName());
        return rsp.Encode();
    }

    SystemEntity* pTSE = pSysMgr->GetSE(args.arg);
    if (pTSE == nullptr) {
        _log(INV__WARNING, "Unable to find entity %u in system %u from '%s'", args.arg, pSysMgr->GetID(), pClient->GetName());
        return rsp.Encode();
    }
    if ((pShip->SysBubble() == nullptr)
    or (pTSE->SysBubble() == nullptr)) {
        _log(DESTINY__ERROR, "Client %u or Target %u does not have a bubble.", pClient->GetName(), pTSE->GetName());
        return rsp.Encode();
    }

    if (pTSE->HasPilot())
        if (pTSE->GetPilot()->IsInvul())
            throw PyException( MakeUserError( "DeniedTargetInvulnerable"));

    if (!pShip->TargetMgr()->StartTargeting(pTSE, pClient->GetShip())) {
        _log(TARGET__WARNING, "Handle_AddTarget - TargMgr.StartTargeting() failed.");
        throw PyException( MakeUserError( "DeniedTargetingAttemptFailed"));
        //return rsp.Encode();
    }

    if (sConfig.debug.IsTestServer)
        if (is_log_enabled(TARGET__MESSAGE)) {
            GVector vectorToTarget(pShip->GetPosition(), pTSE->GetPosition());
            _log(TARGET__MESSAGE, "Handle_AddTarget() - %s(%u) -> %s(%u) at range of %.2f meters.", \
                        pClient->GetName(), pClient->GetCharacterID(), pTSE->GetName(),pTSE->GetID(), vectorToTarget.length() );
        }

    rsp.flag = true;
    return rsp.Encode();
}

PyResult DogmaIMBound::Handle_RemoveTarget(PyCallArgs& call) {
    Client* pClient(call.client);

    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return PyStatic.NewNone();
    }

    SystemManager* pSysMgr = pClient->SystemMgr();
    if (pSysMgr == nullptr) {
        _log(SERVICE__ERROR, "Unable to find system manager for '%s'", pClient->GetName());
        return PyStatic.NewNone();
    }
    SystemEntity* pTSE = pSysMgr->GetSE(args.arg);
    if (pTSE == nullptr) {
        _log(SERVICE__ERROR, "Unable to find entity %u in system %u for '%s'", args.arg, pSysMgr->GetID(), pClient->GetName());
        return PyStatic.NewNone();
    }

    if (sConfig.debug.IsTestServer)
        if (is_log_enabled(TARGET__MESSAGE)) {
            GVector vectorToTarget(pClient->GetShipSE()->GetPosition(), pTSE->GetPosition());
            _log(TARGET__MESSAGE, "Handle_RemoveTarget() - Removed %s(%u) - Range to Target: %.2f meters.", \
                        pTSE->GetName(),pTSE->GetID(), vectorToTarget.length() );
        }

    pClient->GetShipSE()->TargetMgr()->ClearTarget(pTSE);
    return nullptr;
}


PyResult DogmaIMBound::Handle_GetAllInfo(PyCallArgs& call)
{
    //sLog.Warning("DogmaIMBound::Handle_GetAllInfo()", "size=%u", call.tuple->size());
    //call.Dump(SERVICE__CALL_DUMP);
    /* added more return data and updated logic (almost complete and mostly accurate) -allan 26Mar16 */
    /* Start the Code */
    Client* pClient(call.client);

    Call_TwoBoolArgs args; /* arg1: getCharInfo, arg2: getShipInfo */
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return PyStatic.NewNone();
    }

	/* Create the response dictionary */
    PyDict* rsp = new PyDict();
    rsp->SetItemString("activeShipID", new PyInt(pClient->GetShipID()));
    /* Setting "locationInfo" in the Dictionary */
    /** @todo  havent found a populated item in packet logs
     *
        def ProcessLocationInfo(self, cData):
            for locationID, datas in cData.iteritems():
        --still dont know what 'datas' are
        ** this has *something* to do with POS
     */
    rsp->SetItemString("locationInfo", PyStatic.NewNone());

    /* Setting "shipModifiedCharAttribs" in the Dictionary */
    /** @todo  havent found a populated item in packet logs */
    rsp->SetItemString("shipModifiedCharAttribs", PyStatic.NewNone());

    /* Setting "charInfo" in the Dictionary  -fixed 24Mar16 */
    sItemFactory.SetUsingClient(pClient);
    if (args.arg1) {
        PyDict* charResult = pClient->GetChar()->GetCharInfo();
        if (charResult == nullptr) {
            _log(SERVICE__ERROR, "Unable to build char info for char %u", pClient->GetCharacterID());
            sItemFactory.UnsetUsingClient();
            return PyStatic.NewNone();
        }
        rsp->SetItemString("charInfo", charResult);
    } else  // fixed
        rsp->SetItemString("charInfo", new PyDict());

	/* Setting "shipInfo" in the Dictionary  -fixed 26Mar16 */
	if (args.arg2) {
        PyDict* shipResult = pClient->GetShip()->GetShipInfo();
        if (shipResult == nullptr) {
            _log(SERVICE__ERROR, "Unable to build ship info for ship %u", pClient->GetShipID());
            sItemFactory.UnsetUsingClient();
            return PyStatic.NewNone();
        }
        rsp->SetItemString("shipInfo", shipResult);
    } else
        rsp->SetItemString("shipInfo", new PyDict());

    /* Setting "shipState" in the Dictionary  -fixed 26Mar16  -UD to add linked weapons 7Jan19 */
    if (pClient->GetShip().get() == nullptr) {
        _log(SERVICE__ERROR, "Unable to build shipState for %u", pClient->GetShipID());
        return PyStatic.NewNone();
    }
    PyTuple* rspShipState = new PyTuple(3);
        rspShipState->items[0] = pClient->GetShip()->GetShipState();        // fitted module list
        rspShipState->items[1] = pClient->GetShip()->GetChargeState();      // loaded charges
        rspShipState->items[2] = pClient->GetShip()->GetLinkedWeapons();    // linked weapons
    rsp->SetItemString("shipState", rspShipState);
    if (is_log_enabled(CLIENT__INFO))
        rsp->Dump(CLIENT__INFO, "     ");
    sItemFactory.UnsetUsingClient();
	return new PyObject("util.KeyVal", rsp );
}

PyResult DogmaIMBound::Handle_LinkWeapons(PyCallArgs& call) {
    /*  data = self.remoteDogmaLM.LinkWeapons(shipID, masterID, fromID)
     *
     *    def SetWeaponBanks(self, shipID, data):
     *        self.slaveModulesByMasterModule[shipID] = defaultdict(set)
     *        if data is None:
     *            return
     *        for masterID, slaveIDs in data.iteritems():
     *            for slaveID in slaveIDs:
     *                self.slaveModulesByMasterModule[shipID][masterID].add(slaveID)
     */

    Call_Dogma_LinkWeapons args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return PyStatic.NewNone();
    }
    /* args.shipID
     * args.masterID
     * args.slaveID
     */
    if (!IsPlayerItem(args.shipID))
        return nullptr;
    ShipItemRef sRef = call.client->SystemMgr()->GetShipFromInventory(args.shipID);
    if (sRef.get() == nullptr)
        return nullptr;
    sRef->LinkWeapon(args.masterID, args.slaveID);
    return sRef->GetLinkedWeapons();
}

PyResult DogmaIMBound::Handle_LinkAllWeapons(PyCallArgs& call) {
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return PyStatic.NewNone();
    }

    if (!IsPlayerItem(arg.arg))
        return nullptr;
    ShipItemRef sRef = call.client->SystemMgr()->GetShipFromInventory(arg.arg);
    if (sRef.get() == nullptr)
        return nullptr;
    // locate and link all weapons on ship, if possible.
    sRef->LinkAllWeapons();
    return sRef->GetLinkedWeapons();
}

PyResult DogmaIMBound::Handle_DestroyWeaponBank(PyCallArgs& call) {
    //self.remoteDogmaLM.DestroyWeaponBank(shipID, itemID)
    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return PyStatic.NewNone();
    }

    if (!IsPlayerItem(args.arg1) or !IsPlayerItem(args.arg2))
        return nullptr;
    ShipItemRef sRef = call.client->SystemMgr()->GetShipFromInventory(args.arg1);
    if (sRef.get() == nullptr)
        return nullptr;
    sRef->UnlinkGroup(args.arg2);
    return nullptr;
}

PyResult DogmaIMBound::Handle_UnlinkAllModules(PyCallArgs& call) {
    //info = self.remoteDogmaLM.UnlinkAllModules(shipID)
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return PyStatic.NewNone();
    }

    if (!IsPlayerItem(arg.arg))
        return nullptr;
    ShipItemRef sRef = call.client->SystemMgr()->GetShipFromInventory(arg.arg);
    if (sRef.get() == nullptr)
        return nullptr;
    sRef->UnlinkAllWeapons();
    return nullptr;
}

PyResult DogmaIMBound::Handle_UnlinkModule(PyCallArgs& call) {
    //UnlinkModule(shipID, moduleID)
    sLog.Warning("DogmaIMBound::Handle_UnlinkModule()", "size=%u", call.tuple->size());
    call.Dump(SHIP__INFO);

    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return new PyInt(0);
    }

    if (!IsPlayerItem(args.arg1) or !IsPlayerItem(args.arg2))
        return new PyInt(0);
    ShipItemRef sRef = call.client->SystemMgr()->GetShipFromInventory(args.arg1);
    if (sRef.get() == nullptr)
        return new PyInt(0);

    return new PyInt(sRef->UnlinkWeapon(args.arg2));
}

PyResult DogmaIMBound::Handle_MergeModuleGroups(PyCallArgs& call) {
    //info = self.remoteDogmaLM.MergeModuleGroups(shipID, masterID, slaveID)
    sLog.Warning("DogmaIMBound::Handle_MergeModuleGroups()", "size=%u", call.tuple->size());
    call.Dump(SHIP__INFO);

    Call_Dogma_LinkWeapons args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return PyStatic.NewNone();
    }
    /* args.shipID
     * args.masterID
     * args.slaveID
     */
    if (!IsPlayerItem(args.shipID))
        return nullptr;
    ShipItemRef sRef = call.client->SystemMgr()->GetShipFromInventory(args.shipID);
    if (sRef.get() == nullptr)
        return nullptr;

    // not sure what to do here
    return nullptr;
}

PyResult DogmaIMBound::Handle_Activate(PyCallArgs& call)
{
    // ret = self.GetDogmaLM().Activate(itemID, effectName, target, repeat)  - i cant find where this return is used but is "1" in packet logs
    // dogmaLM.Activate(itemID, const.effectOnlineForStructures)
    // dogmaLM.Activate(itemID, const.effectAnchorDrop)
    // dogmaLM.Activate(itemID, const.effectAnchorLift)
    // dogmaLM.Activate(itemID, const.effectAnchorLiftForStructures)
    Client* pClient(call.client);

    if (!pClient->IsInSpace()) {
        pClient->SendNotifyMsg("You can't do this while docked");
        return PyStatic.NewZero();
    }
    /* {'messageKey': 'DeniedActivateCloaked', 'dataID': 17883388, 'suppressable': False, 'bodyID': 259487, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 771}
     * {'messageKey': 'DeniedActivateControlling', 'dataID': 17880010, 'suppressable': False, 'bodyID': 258228, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2230}
     * {'messageKey': 'DeniedActivateFrozen', 'dataID': 17883391, 'suppressable': False, 'bodyID': 259488, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 772}
     * {'messageKey': 'DeniedActivateInJump', 'dataID': 17883394, 'suppressable': False, 'bodyID': 259489, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 773}
     * {'messageKey': 'DeniedActivateInWarp', 'dataID': 17883704, 'suppressable': False, 'bodyID': 259597, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 774}
     * {'messageKey': 'DeniedActivateTargetAssistDisallowed', 'dataID': 17883397, 'suppressable': False, 'bodyID': 259490, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 775}
     * {'messageKey': 'DeniedActivateTargetModuleDisallowed', 'dataID': 17883400, 'suppressable': False, 'bodyID': 259491, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 776}
     * {'messageKey': 'DeniedActivateTargetNotPresent', 'dataID': 17883403, 'suppressable': False, 'bodyID': 259492, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 777}
     * {'messageKey': 'DeniedActivateTargetOffModDisallowed', 'dataID': 17883406, 'suppressable': False, 'bodyID': 259493, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 778}
     * {'FullPath': u'UI/Messages', 'messageID': 259487, 'label': u'DeniedActivateCloakedBody'}(u'Interference from the cloaking you are doing is preventing your systems from functioning at this time.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 259488, 'label': u'DeniedActivateFrozenBody'}(u'You are unable to activate any modules because you have been frozen by a GM.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 259489, 'label': u'DeniedActivateInJumpBody'}(u'Interference from the jump you are doing is preventing your systems from functioning at this time.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 259490, 'label': u'DeniedActivateTargetAssistDisallowedBody'}(u'You cannot activate that module on the target as interference prevents assistance from being given to them.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 259491, 'label': u'DeniedActivateTargetModuleDisallowedBody'}(u'You cannot activate that module on the target as interference prevents modules of that type from being used on them.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 259492, 'label': u'DeniedActivateTargetNotPresentBody'}(u'You cannot activate that module as the target is no longer present.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 259493, 'label': u'DeniedActivateTargetOffModDisallowedBody'}(u'You cannot activate that module on the target as interference prevents modules of that type from being used on them.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 259597, 'label': u'DeniedActivateInWarpBody'}(u'Interference from your warp prevents your systems from functioning at this time.', None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 258228, 'label': u'DeniedActivateControllingBody'}(u'You are unable to activate any modules while you are controlling some other objects.', None, None)
     *
     */

    if (call.tuple->size() == 2) {
        call.Dump(POS__DUMP);
        // anchor cargo and pos items
        // online pos items
        Call_TwoIntegerArgs args;
        if (!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
            return PyStatic.NewZero();
        }
        /*      this is deactivate call....
            22:06:59 W DogmaIMBound::Handle_Activate(): size=2
            22:06:59 [POS:Dump]   Call Arguments:
            22:06:59 [POS:Dump]      Tuple: 2 elements
            22:06:59 [POS:Dump]       [ 0]    Integer: 140000061
            22:06:59 [POS:Dump]       [ 1]    Integer: 1023     << deactivate

            anchorDrop =   649,     // effects.AnchorDrop
            anchorLift =   650,     // effects.AnchorLift
            onlineForStructures =   901,     // effects.StructureOnline
            anchorDropForStructures =   1022,     // effects.AnchorDrop
            anchorLiftForStructures =   1023,     // effects.AnchorLift
            anchorDropOrbital =   4769,     // effects.AnchorDrop
            anchorLiftOrbital =   4770,     // effects.AnchorLift
        */

        SystemEntity* pSE = pClient->SystemMgr()->GetSE(args.arg1);
        if (pSE == nullptr) {
            sLog.Error("DogmaIMBound::Handle_Activate()", "%u is not a valid EntityID in this system.", args.arg1);
            return PyStatic.NewZero();
        }
        // determine if this pSE is pos or cont.
        //call (de)activate on pSE, pass effectID, send effect to clients (bubblecast) then set timers.
        if (pSE->IsPOSSE()) {
            /*
            if ((args.arg2 == anchorDropForStructures)
            or (args.arg2 == anchorDropOrbital))
                pSE->GetPOSSE()->SetAnchor(args.arg2);
            else */ if (args.arg2 == anchorLiftForStructures)
                pSE->GetPOSSE()->PullAnchor();
            else if (args.arg2 == onlineForStructures)
                pSE->GetPOSSE()->Activate(args.arg2);
        } else if (pSE->IsContainerSE()) {
            // not sure what calls are for containers yet
            pSE->GetContSE()->Activate(args.arg2);
        } else
            ; // make error here

    } else if (call.tuple->size() == 4) {
        // activate ship module
        Call_Dogma_Activate args;
        if (!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
            return PyStatic.NewZero();
        }

        pClient->GetShip()->Activate(args.itemID, args.effectName, args.target, args.repeat);
    }
    // are there any other cases to test for here?

    // returns 1 (from what i've seen in logs, but dont know why)
    return PyStatic.NewOne();
}

// this one is called from Deactivate() when module is OL
PyResult DogmaIMBound::Handle_StopOverload(PyCallArgs& call)
{
    // return self.GetDogmaLM().StopOverload(itemID, effectID)

    Client* pClient(call.client);
    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", pClient->GetName());
        return PyStatic.NewNone();
    }

    //  cancel overload then deactivate module
    pClient->GetShip()->CancelOverloading(args.arg1);
    pClient->GetShip()->Deactivate(args.arg1, sFxDataMgr.GetEffectName(args.arg2));
}


PyResult DogmaIMBound::Handle_Deactivate(PyCallArgs& call)
{
    //  return self.statemanager.Deactivate(self.itemID, self.effectName)
    //  dogmaLM.Deactivate(itemID, const.effectOnlineForStructures)
    sLog.Warning("DogmaIMBound::Handle_Deactivate()", "size=%u", call.tuple->size());

    Client* pClient(call.client);

    if (!pClient->IsInSpace()) {
        pClient->SendNotifyMsg("You can't do this while docked");
        return PyStatic.NewNone();
    }

    if (call.tuple->items.at(1)->IsInt()) {
        // if effect is integer, call is for pos or container
        call.Dump(POS__DUMP);
        Call_TwoIntegerArgs args;
        if (!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", pClient->GetName());
            return PyStatic.NewNone();
        }
        SystemEntity* pSE = pClient->SystemMgr()->GetSE(args.arg1);
        if (pSE == nullptr) {
            sLog.Error("DogmaIMBound::Handle_Deactivate()", "%u is not a valid EntityID in this system.", args.arg1);
             return PyStatic.NewNone();
        }
        /*
         * 22:24:28 W DogmaIMBound::Handle_Deactivate(): size=2
         * 22:24:28 [POS:Dump]   Call Arguments:
         * 22:24:28 [POS:Dump]      Tuple: 2 elements
         * 22:24:28 [POS:Dump]       [ 0]    Integer: 140000061
         * 22:24:28 [POS:Dump]       [ 1]    Integer: 901
         */
        // determine if this pSE is pos or cont.
        //call activate on pSE, pass effectID, send effect to clients (bubblecast) then set timers.
        if (pSE->IsPOSSE())
            pSE->GetPOSSE()->Deactivate(args.arg2);
        else if (pSE->IsContainerSE())
            pSE->GetContSE()->Deactivate(args.arg2);
        else
            ; // make error here

    } else if (call.tuple->items.at(1)->IsWString()) {
        //if effect is wide string, then call is for module
        Call_Dogma_Deactivate args;
        if (!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", pClient->GetName());
            return PyStatic.NewNone();
        }

        pClient->GetShip()->Deactivate(args.itemID, args.effectName);
    }
    // are there any other cases to test for?

    // returns None()
    return PyStatic.NewNone();
}

PyResult DogmaIMBound::Handle_Overload(PyCallArgs& call) {
    /*
     * 23:52:45 L DogmaIMBound::Handle_Overload(): size=2
     * 23:52:45 [SvcCallDump]   Call Arguments:
     * 23:52:45 [SvcCallDump]       Tuple: 2 elements
     * 23:52:45 [SvcCallDump]         [ 0] Integer field: 140002542
     * 23:52:45 [SvcCallDump]         [ 1] Integer field: 3035
     */
    // self.GetDogmaLM().Overload(itemID, effectID)

    Client* pClient(call.client);
    Call_TwoIntegerArgs args;   //itemID, effectID
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", pClient->GetName());
        return nullptr;
    }

    //TODO:  need to verify pilot can OL modules
    // AttrRequiredThermoDynamicsSkill
    pClient->GetShip()->Overload(args.arg1);
    return nullptr;
}

PyResult DogmaIMBound::Handle_CancelOverloading(PyCallArgs& call) {
    // self.dogmaLM.CancelOverloading(itemID)

    Client* pClient(call.client);
    Call_SingleIntegerArg args;   //itemID
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", pClient->GetName());
        return nullptr;
    }

    pClient->GetShip()->CancelOverloading(args.arg);
    return nullptr;
}

PyResult DogmaIMBound::Handle_OverloadRack(PyCallArgs& call) {
    /* moduleIDs = self.GetDogmaLM().OverloadRack(itemID)
     *   moduleIDs is list of modules in rack.
     */
    Client* pClient(call.client);
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", pClient->GetName());
        return new PyList();
    }

    // not supported yet
    PyList* list = new PyList();
    return list;
}

PyResult DogmaIMBound::Handle_StopOverloadRack(PyCallArgs& call) {
    /* moduleIDs = self.GetDogmaLM().StopOverloadRack(itemID)
     *   moduleIDs is list of modules in rack.
     */
    Client* pClient(call.client);
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", pClient->GetName());
        return new PyList();
    }

    // not supported yet
    PyList* list = new PyList();
    return list;
}

PyResult DogmaIMBound::Handle_InitiateModuleRepair(PyCallArgs& call) {
    //  this is for repairing modules using nanite paste (button's ring turns white).  return bool.
    //  res = self.GetDogmaLM().InitiateModuleRepair(itemID)
    // see notes in ModuleManager::ModuleRepair()

    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return PyStatic.NewFalse();
    }

    return call.client->GetShip()->ModuleRepair(args.arg);
}

PyResult DogmaIMBound::Handle_StopModuleRepair(PyCallArgs& call) {
    //  self.GetDogmaLM().StopModuleRepair(itemID)

    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    call.client->GetShip()->StopModuleRepair(args.arg);

    // returns nothing
    return nullptr;
}

// dunno what this is
PyResult DogmaIMBound::Handle_PeelAndLink(PyCallArgs& call) {
    //info = self.remoteDogmaLM.PeelAndLink(shipID, masterID, slaveID)
    sLog.Warning("DogmaIMBound::Handle_PeelAndLink()", "size=%u", call.tuple->size());
    call.Dump(SHIP__INFO);

    return nullptr;
}

// dunno what this is
PyResult DogmaIMBound::Handle_CheckSendLocationInfo(PyCallArgs& call)
{
    sLog.Warning("DogmaIMBound::Handle_CheckSendLocationInfo()", "size=%u", call.tuple->size());
    call.Dump(SHIP__INFO);

    return nullptr;
}

// dunno what this is
PyResult DogmaIMBound::Handle_GetLocationInfo(PyCallArgs& call)
{
    // oldOwnerID, oldLocationID, oldFlagID = oldInfo = dogmaItem.GetLocationInfo()
    /*
     *             [PyTuple 3 items]         << call from client
     *               [PyString "GetLocationInfo"]
     *               [PyTuple 0 items]
     *               [PyDict 0 kvp]
     *   [PyTuple 1 items]                   << response from server
     *     [PySubStream 43 bytes]
     *       [PyTuple 2 items]
     *         [PySubStruct]
     *           [PySubStream 32 bytes]
     *             [PyTuple 2 items]
     *               [PyString "N=699771:17106"]     << nodeID
     *               [PyIntegerVar 129503265956883696]   << filetime.  sent in "ClientHasReleasedTheseObjects" packet later
     *         [PyDict 0 kvp]
     *   [PyDict 1 kvp]
     *     [PyString "OID+"]
     *     [PyDict 1 kvp]
     *       [PyString "N=699771:17106"]
     *       [PyIntegerVar 129503265956883696]
     */

    // dummy right now, don't have any meaningful packet logs
    // returns nodeID and timestamp
    PyTuple* tuple = new PyTuple(2);
    tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
    tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    return tuple;
}

PyResult DogmaIMBound::Handle_ChangeDroneSettings(PyCallArgs& call) {
    /*
     * 21:59:29 L Server: ChangeDroneSettings call made to
     * 21:59:29 L DogmaIMBound::Handle_ChangeDroneSettings(): size=1
     * 21:59:29 [SvcCall]   Call Arguments:
     * 22:04:44 [SvcCall]       Tuple: 1 elements
     * 22:04:44 [SvcCall]         [ 0] Dictionary: 3 entries
     * 22:04:44 [SvcCall]         [ 0]   [ 0] Key: Integer field: 1283 <-- AttrFightersAttackAndFollow
     * 22:04:44 [SvcCall]         [ 0]   [ 0] Value: Integer field: 1
     * 22:04:44 [SvcCall]         [ 0]   [ 1] Key: Integer field: 1275 <-- AttrDroneIsAgressive
     * 22:04:44 [SvcCall]         [ 0]   [ 1] Value: Integer field: 1
     * 22:04:44 [SvcCall]         [ 0]   [ 2] Key: Integer field: 1297 <-- AttrDroneFocusFire
     * 22:04:44 [SvcCall]         [ 0]   [ 2] Value: Integer field: 1
     *
     *    sLog.Warning("DogmaIMBound::Handle_ChangeDroneSettings()", "size=%u", call.tuple->size());
     *    call.Dump(SHIP__INFO);
     */

    return nullptr;
}