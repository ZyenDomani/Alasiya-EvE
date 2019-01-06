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
    Updates:    Allan
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
    Client* pClient = call.client;

    if (pClient->IsInSpace()) {
        DestinyManager* pDestiny = pClient->GetShipSE()->DestinyMgr();
        if (pDestiny == nullptr) {
            _log(PLAYER__ERROR, "%s: Client has no destiny manager!", pClient->GetName());
            return PyStatic.NewNone();
        } else if (pDestiny->IsWarping()) {
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
    Client* pClient = call.client;

    if (pClient->IsInSpace()) {
        DestinyManager* pDestiny = pClient->GetShipSE()->DestinyMgr();
        if (pDestiny == nullptr) {
            _log(PLAYER__ERROR, "%s: Client has no destiny manager!", pClient->GetName());
            return PyStatic.NewNone();
        } else if (pDestiny->IsWarping()) {
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
        return PyStatic.NewNone();
    }

    if (args.moduleIDs.empty())
        return PyStatic.NewNone();

    // Get Reference to Ship, Module, and Charge
    InventoryItemRef cRef = sItemFactory.GetItem(args.itemID);
    if (cRef.get() == nullptr) {
        sLog.Error("DogmaIMBound::Handle_LoadAmmoToModules()", "cant find charge %u.", args.itemID );
        throw PyException( MakeUserError( "CantFindChargeToAdd"));
    }

    ShipItemRef sRef = call.client->GetShip();
    InventoryItemRef mRef(nullptr);

    for (auto cur : args.moduleIDs) {
        mRef = sRef->GetModuleRef(cur);
        if (mRef.get() == nullptr) {
            sLog.Error("DogmaIMBound::Handle_LoadAmmoToModules()", "cant find module %u.", cur );
            throw PyException( MakeUserError( "ModuleNoLongerPresentForCharges"));
        }
        if (cRef->quantity() > 0)
            sRef->LoadCharge(cRef, mRef->flag());
        else
            return PyStatic.NewNone();
    }

    return PyStatic.NewNone();
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
        return PyStatic.NewNone();
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
        return PyStatic.NewNone();

	// Get Reference to Ship, Module, and Charge
    ShipItemRef sRef = call.client->GetShip();
    if (sRef->itemID() != args.shipID)
        sLog.Error("DogmaIMBound::Handle_LoadAmmoToBank()", "passed shipID %u != current shipID %u.", args.shipID, sRef->itemID() );

    GenericModule* pMod = sRef->GetModuleManager()->GetModule(args.masterID);
    if (pMod == nullptr)
        throw PyException( MakeUserError("ModuleNoLongerPresentForCharges"));

	if (pMod->IsLinked())
        sRef->LoadLinkedWeapons(pMod, args.itemIDs);
    else
        sRef->LoadCharge(sItemFactory.GetItem(args.itemIDs.at(0)), pMod->flag());
        //sRef->LoadChargesToBank(pMod->flag(), args.itemIDs);

	return PyStatic.NewNone();
}

PyResult DogmaIMBound::Handle_Activate(PyCallArgs& call)
{
    Client* pClient = call.client;

    if (!pClient->IsInSpace()) {
        pClient->SendNotifyMsg("You can't do this while docked");
        return PyStatic.NewNone();
    }

    if (call.tuple->size() == 2) {
        call.Dump(POS__DUMP);
        // anchor cargo and pos items
        // online pos items
        Call_TwoIntegerArgs args;
        if (!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
            return PyStatic.NewNone();
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
             return PyStatic.NewNone();
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
            // not sure what all calls are for containers yet
            pSE->GetContSE()->Activate(args.arg2);
        } else
            ; // make error here

    } else if (call.tuple->size() == 4) {
        // activate ship module
        Call_Dogma_Activate args;
        if (!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
            return PyStatic.NewNone();
        }

        pClient->GetShip()->Activate(args.itemID, args.effectName, args.target, args.repeat);
    }
    // are there any other cases to test for here?

    return PyStatic.NewNone();
}

PyResult DogmaIMBound::Handle_Deactivate(PyCallArgs& call)
{
    sLog.Warning("DogmaIMBound::Handle_Deactivate()", "size=%u", call.tuple->size());

    Client* pClient = call.client;

    if (!pClient->IsInSpace()) {
        pClient->SendNotifyMsg("You can't do this while docked");
        return PyStatic.NewNone();
    }

    if (call.tuple->items.at(1)->IsInt()) {
        // if effect is integer, call is for pos or container
        call.Dump(POS__DUMP);
        Call_TwoIntegerArgs args;
        if (!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
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
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
            return PyStatic.NewNone();
        }

        pClient->GetShip()->Deactivate(args.itemID, args.effectName);
    }
    // are there any other cases to test for?
    return PyStatic.NewNone();
}

    /*{'messageKey': 'CantTargetWhileCloaked', 'dataID': 17879126, 'suppressable': False, 'bodyID': 257890, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2436}
     * {'messageKey': 'CantTargetWhileEnteringWormhole', 'dataID': 17877231, 'suppressable': False, 'bodyID': 257172, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2798}
     * {'messageKey': 'CantTargetWhileJumping', 'dataID': 17885002, 'suppressable': False, 'bodyID': 260081, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 432}
     */
PyResult DogmaIMBound::Handle_AddTarget(PyCallArgs& call) {
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return PyStatic.NewNone();
    }
    Rsp_Dogma_AddTarget rsp;
        rsp.flag = false;
        rsp.targetList.push_back(args.arg);

    Client* pClient = call.client;
    if (!pClient->IsInSpace()) {
        pClient->SendNotifyMsg("You can't do this while docked");
        return rsp.Encode();
    }

    DestinyManager* pDestiny = pClient->GetShipSE()->DestinyMgr();
    if (pDestiny == nullptr) {
        _log(PLAYER__ERROR, "%s: Client has no destiny manager!", pClient->GetName());
        return rsp.Encode();
    } else if (pDestiny->IsWarping()) {
        pClient->SendNotifyMsg("You can't do this while warping");
        return rsp.Encode();
    }

    if (pClient->GetShipSE()->TargetMgr() == nullptr)
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
    if ((pClient->GetShipSE()->SysBubble() == nullptr) || (pTSE->SysBubble() == nullptr)) {
        _log(DESTINY__ERROR, "Client %u or Target %u does not have a bubble.", pClient->GetName(), pTSE->GetName());
        return rsp.Encode();
    }

    if (!pClient->GetShipSE()->TargetMgr()->StartTargeting(pTSE, pClient->GetShip())) {
        _log(TARGET__WARNING, "Handle_AddTarget - TargMgr.StartTargeting() failed.");
        return rsp.Encode();
    }

    if (sConfig.debug.IsTestServer)
        if (is_log_enabled(TARGET__MESSAGE)) {
            GVector vectorToTarget(pClient->GetShipSE()->GetPosition(), pTSE->GetPosition());
            _log(TARGET__MESSAGE, "Handle_AddTarget() - %s(%u) -> %s(%u) at range of %.2f meters.", \
                        pClient->GetName(), pClient->GetCharacterID(), pTSE->GetName(),pTSE->GetID(), vectorToTarget.length() );
        }

    rsp.flag = true;
    return rsp.Encode();
}

PyResult DogmaIMBound::Handle_RemoveTarget(PyCallArgs& call) {
    Client* pClient = call.client;

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
    Client* pClient = call.client;

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
    if (args.arg1) {
        PyDict* charResult = pClient->GetChar()->GetCharInfo();
        if (charResult == nullptr) {
            _log(SERVICE__ERROR, "Unable to build char info for char %u", pClient->GetCharacterID());
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
            return PyStatic.NewNone();
        }
        rsp->SetItemString("shipInfo", shipResult);
    } else
        rsp->SetItemString("shipInfo", new PyDict());

    /* Setting "shipState" in the Dictionary  -fixed 26Mar16 */
    if (pClient->GetShip().get() == nullptr) {
        _log(SERVICE__ERROR, "Unable to build shipState for %u", pClient->GetShipID());
        return PyStatic.NewNone();
    }
    PyTuple* rspShipState = new PyTuple(3);
        rspShipState->items[0] = pClient->GetShip()->GetShipState();
        rspShipState->items[1] = pClient->GetShip()->GetChargeState();
        rspShipState->items[2] = new BuiltinSet();
    rsp->SetItemString("shipState", rspShipState);
    if (is_log_enabled(CLIENT__INFO))
        rsp->Dump(CLIENT__INFO, "     ");
	return new PyObject("util.KeyVal", rsp );
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
    //UnlinkAllModules(shipID)
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


PyResult DogmaIMBound::Handle_Overload(PyCallArgs& call) {
    /*
     * 23:52:45 L DogmaIMBound::Handle_Overload(): size=2
     * 23:52:45 [SvcCallDump]   Call Arguments:
     * 23:52:45 [SvcCallDump]       Tuple: 2 elements
     * 23:52:45 [SvcCallDump]         [ 0] Integer field: 140002542
     * 23:52:45 [SvcCallDump]         [ 1] Integer field: 3035
     * 23:52:45 [SvcCallDump]   Call Named Arguments:
     * 23:52:45 [SvcCallDump]     Argument 'machoVersion':
     * 23:52:45 [SvcCallDump]         Integer field: 1
     */
    Client* pClient = call.client;
    if (pClient->IsInSpace()) {
        DestinyManager* pDestiny = pClient->GetShipSE()->DestinyMgr();
        if (pDestiny == nullptr) {
            _log(PLAYER__ERROR, "%s: Client has no destiny manager!", pClient->GetName());
            return PyStatic.NewNone();
        } else if (pDestiny->IsWarping()) {
            pClient->SendNotifyMsg("You can't do this while warping");
            return PyStatic.NewNone();
        }
    }

    sLog.Warning("DogmaIMBound::Handle_Overload()", "size=%u", call.tuple->size());
    call.Dump(SHIP__WARNING);

    Call_TwoIntegerArgs args;   //itemID, effectID
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return PyStatic.NewNone();
    }

    return nullptr;
}

PyResult DogmaIMBound::Handle_CancelOverloading(PyCallArgs& call) {
    Client* pClient = call.client;

    if (pClient->IsInSpace()) {
        DestinyManager* pDestiny = pClient->GetShipSE()->DestinyMgr();
        if (pDestiny == nullptr) {
            _log(PLAYER__ERROR, "%s: Client has no destiny manager!", pClient->GetName());
            return PyStatic.NewNone();
        } else if (pDestiny->IsWarping()) {
            pClient->SendNotifyMsg("You can't do this while warping");
            return PyStatic.NewNone();
        }
    }

    sLog.Warning("DogmaIMBound::Handle_CancelOverloading()", "size=%u", call.tuple->size());
    call.Dump(SHIP__WARNING);
    return nullptr;
}

PyResult DogmaIMBound::Handle_OverloadRack(PyCallArgs& call) {
    /*
     *    called thru rclick menu on module
     *    17:20:22 L DogmaIMBound::Handle_OverloadRack(): size=1
     *    17:20:22 [SvcCall]   Call Arguments:
     *    17:20:22 [SvcCall]       Tuple: 1 elements
     *    17:20:22 [SvcCall]         [ 0] Integer field: 140000223    <--  itemID in any slot of location to OL
     *
     *    called thru OL button on ship dashboard
     *    17:24:00 L DogmaIMBound::Handle_OverloadRack(): size=1
     *    17:24:00 [SvcCall]   Call Arguments:
     *    17:24:00 [SvcCall]       Tuple: 1 elements
     *    17:24:00 [SvcCall]         [ 0] Integer field: 140000213    <--  itemID in first slot of location to OL
     *
     *    returns - PyList of moduleIDs to OL
     *
     * /client/script/environment/godma.py(2407) OverloadRack
     *        itemID = 140001963L
     *        self = <godma.StateManager instance at 0x3CB717D8>
     *        moduleIDs = None
     * TypeError: 'NoneType' object is not iterable
     *
     *    sLog.Warning("DogmaIMBound::Handle_OverloadRack()", "size=%u", call.tuple->size());
     *    call.Dump(SHIP__INFO);
     */
    Client* pClient = call.client;

    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return PyStatic.NewNone();
    }

    return nullptr;
}

PyResult DogmaIMBound::Handle_StopOverloadRack(PyCallArgs& call) {
    /*
     */
    sLog.Warning("DogmaIMBound::Handle_StopOverloadRack()", "size=%u", call.tuple->size());
    call.Dump(SHIP__INFO);
    Client* pClient = call.client;

    return nullptr;
}

PyResult DogmaIMBound::Handle_InitiateModuleRepair(PyCallArgs& call) {
    /*  module ring turns white for this one.
     * 15:29:13 [SvcCall] Service DogmaIMBound::InitiateModuleRepair()
     * 15:29:13 [SvcCallDump]   Call Arguments:
     * 15:29:13 [SvcCallDump]      Tuple: 1 elements
     * 15:29:13 [SvcCallDump]       [ 0]    Integer: 140000742
     */
    sLog.Warning("DogmaIMBound::Handle_InitiateModuleRepair()", "size=%u", call.tuple->size());
    call.Dump(SHIP__INFO);

    return nullptr;
}

PyResult DogmaIMBound::Handle_StopModuleRepair(PyCallArgs& call) {
    /*
     * 15:29:08 [SvcCall] Service DogmaIMBound::StopModuleRepair()
     * 15:29:08 [SvcCallDump]   Call Arguments:
     * 15:29:08 [SvcCallDump]      Tuple: 1 elements
     * 15:29:08 [SvcCallDump]       [ 0]    Integer: 140000742
     */
    sLog.Warning("DogmaIMBound::Handle_StopModuleRepair()", "size=%u", call.tuple->size());
    call.Dump(SHIP__INFO);

    return nullptr;
}

PyResult DogmaIMBound::Handle_PeelAndLink(PyCallArgs& call) {
    //info = self.remoteDogmaLM.PeelAndLink(shipID, masterID, slaveID)
    sLog.Warning("DogmaIMBound::Handle_PeelAndLink()", "size=%u", call.tuple->size());
    call.Dump(SHIP__INFO);

    return nullptr;
}

PyResult DogmaIMBound::Handle_CheckSendLocationInfo(PyCallArgs& call)
{
    sLog.Warning("DogmaIMBound::Handle_CheckSendLocationInfo()", "size=%u", call.tuple->size());
    call.Dump(SHIP__INFO);

    return nullptr;
}



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
     *   sLog.Warning("DogmaIMBound::Handle_GetLocationInfo()", "size=%u", call.tuple->size());
     *   call.Dump(SHIP__INFO);
     */

    // dummy right now, don't have any meaningful packet logs
    // returns nodeID and timestamp
    PyTuple* tuple = new PyTuple(2);
    tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
    tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    return tuple;
}