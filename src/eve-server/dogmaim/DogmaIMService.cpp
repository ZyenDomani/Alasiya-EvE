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
#include "ship/modules/GenericModule.h"
#include "system/SystemManager.h"

/** @todo this is actually DogmaLM (location manager)... */
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
        PyCallable_REG_CALL(DogmaIMBound, OverloadRack);
        PyCallable_REG_CALL(DogmaIMBound, StopOverloadRack);
        PyCallable_REG_CALL(DogmaIMBound, ShipGetInfo);
        PyCallable_REG_CALL(DogmaIMBound, CharGetInfo);
        PyCallable_REG_CALL(DogmaIMBound, ItemGetInfo);
		PyCallable_REG_CALL(DogmaIMBound, GetAllInfo);
		PyCallable_REG_CALL(DogmaIMBound, GetLocationInfo);
		PyCallable_REG_CALL(DogmaIMBound, GetWeaponBankInfoForShip);
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

        /*StopModuleRepair*/
        /*InitiateModuleRepair*/
        
    }
    virtual ~DogmaIMBound() {delete m_dispatch;}
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(ChangeDroneSettings);
    PyCallable_DECL_CALL(LinkWeapons);
    PyCallable_DECL_CALL(LinkAllWeapons);
    PyCallable_DECL_CALL(OverloadRack);
    PyCallable_DECL_CALL(StopOverloadRack);
    PyCallable_DECL_CALL(ShipGetInfo);
    PyCallable_DECL_CALL(CharGetInfo);
    PyCallable_DECL_CALL(ItemGetInfo);
	PyCallable_DECL_CALL(GetAllInfo);
	PyCallable_DECL_CALL(GetWeaponBankInfoForShip);
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

    /*
     * flag, targetList = self.GetDogmaLM().AddTargetOBO(sid, tid)
    self.GetDogmaLM().RemoveTargetOBO(sid, tid)
    UnlinkModule(shipID, moduleID)
    UnlinkAllModules(shipID)
    MergeModuleGroups(shipID, masterID, slaveID)
    PeelAndLink(shipID, masterID, slaveID)
    DestroyWeaponBank(shipID, itemID)
    */
protected:
    Dispatcher* const m_dispatch;

    uint32 m_locationID;
    uint32 m_groupID;
};

PyCallable_Make_InnerDispatcher(DogmaIMService)

DogmaIMService::DogmaIMService(PyServiceMgr* mgr)
: PyService(mgr, "dogmaIM"),
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
    /*
     * 18:20:13 [ClientMessage] DogmaIMService bind request for:
     * 18:20:13 [ClientMessage]     Tuple: 2 elements
     * 18:20:13 [ClientMessage]       [ 0] Integer field: 60014749      << itemID
     * 18:20:13 [ClientMessage]       [ 1] Integer field: 15            << groupID
     * 18:26:29 [ClientMessage] DogmaIMService bind request for:
     * 18:26:29 [ClientMessage]     Tuple: 2 elements
     * 18:26:29 [ClientMessage]       [ 0] Integer field: 30002547      << itemID
     * 18:26:29 [ClientMessage]       [ 1] Integer field: 5             << groupID
     *
     */
    DogmaLM_BindArgs args;
    //temp crap until I rework _CreateBoundObject's signature
    PyRep *t = bind_args->Clone();
    if(!args.Decode(&t)) {
        codelog(INV__ERROR, "Failed to decode bind args from '%s'", c->GetName());
        return NULL;
    }

    return new DogmaIMBound(m_manager, args.locationID, args.groupID);
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
     *    sLog.White("DogmaIMBound::Handle_ChangeDroneSettings()", "size=%u", call.tuple->size());
     *    call.Dump(SERVICE__CALL_DUMP);
     */

    return nullptr;
}

PyResult DogmaIMBound::Handle_LinkWeapons(PyCallArgs& call) {
    /* 12:54:01 [SvcCall] Service dogmaIM: handling MachoBindObject request directly
     * 12:54:01 DogmaIMBound::Handle_LinkWeapons(): [00msize=3
     * 12:54:01 [SvcCall]   Call Arguments:
     * 12:54:01 [SvcCall]       Tuple: 3 elements
     * 12:54:01 [SvcCall]         [ 0] Integer field: 140000069     <- shipID
     * 12:54:01 [SvcCall]         [ 1] Integer field: 140000078     <- weapon 2  *dropped ON*
     * 12:54:01 [SvcCall]         [ 2] Integer field: 140000079     <- weapon 1  *dragged*

    sLog.White("DogmaIMBound::Handle_LinkWeapons()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

     */

    Client* pClient = call.client;
    Call_Dogma_LinkWeapons args;
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode arguments");
        return new PyNone();
    }
    /* args.shipID
     * args.droppedID
     * args.draggedID
     */

    return nullptr;
}

PyResult DogmaIMBound::Handle_LinkAllWeapons(PyCallArgs& call) {
    /* 18:23:28 [SvcCall] Service dogmaIM: handling MachoBindObject request directly
     * 18:23:28 DogmaIMBound::Handle_LinkAllWeapons(): size=1
     * 18:23:28 [SvcCall]   Call Arguments:
     * 18:23:28 [SvcCall]       Tuple: 1 elements
     * 18:23:28 [SvcCall]         [ 0] Integer field: 140000069     <- shipID
     */

    Client* pClient = call.client;

    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode arguments");
        return new PyNone();
    }
    uint32 shipID = args.arg;

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
        if (!pDestiny) {
            _log(PLAYER__ERROR, "%s: Client has no destiny manager!", pClient->GetName());
            return new PyNone();
        } else if (pDestiny->IsWarping()) {
            pClient->SendNotifyMsg("You can't do this while warping");
            return new PyNone();
        }
    }

    sLog.White("DogmaIMBound::Handle_Overload()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);
    return nullptr;
}

PyResult DogmaIMBound::Handle_CancelOverloading(PyCallArgs& call) {
    Client* pClient = call.client;

    if (pClient->IsInSpace()) {
        DestinyManager* pDestiny = pClient->GetShipSE()->DestinyMgr();
        if (!pDestiny) {
            _log(PLAYER__ERROR, "%s: Client has no destiny manager!", pClient->GetName());
            return new PyNone();
        } else if (pDestiny->IsWarping()) {
            pClient->SendNotifyMsg("You can't do this while warping");
            return new PyNone();
        }
    }

    sLog.White("DogmaIMBound::Handle_CancelOverloading()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);
    return nullptr;
}

PyResult DogmaIMBound::Handle_OverloadRack(PyCallArgs& call) {
    /*
     *    c alled thru r*click menu on module
     *    17:20:22 L DogmaIMBound::Handle_OverloadRack(): [00msize=1
     *    17:20:22 [SvcCall]   Call Arguments:
     *    17:20:22 [SvcCall]       Tuple: 1 elements
     *    17:20:22 [SvcCall]         [ 0] Integer field: 140000223    <--  itemID in any slot of location to OL
     *
     *    called thru OL button on ship dashboard
     *    17:24:00 L DogmaIMBound::Handle_OverloadRack(): [00msize=1
     *    17:24:00 [SvcCall]   Call Arguments:
     *    17:24:00 [SvcCall]       Tuple: 1 elements
     *    17:24:00 [SvcCall]         [ 0] Integer field: 140000213    <--  itemID in first slot of location to OL
     *
     *    returns - list of moduleIDs to OL
     *
     *    sLog.White("DogmaIMBound::Handle_OverloadRack()", "size=%u", call.tuple->size());
     *    call.Dump(SERVICE__CALL_DUMP);
     */
    Client* pClient = call.client;

    return nullptr;
}

PyResult DogmaIMBound::Handle_StopOverloadRack(PyCallArgs& call) {
    /*
     */
    sLog.White("DogmaIMBound::Handle_OverloadRack()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);
    Client* pClient = call.client;

    return nullptr;
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

PyResult DogmaIMBound::Handle_GetLocationInfo(PyCallArgs& call)
{
    /*
              [PyTuple 3 items]         << call from client
                [PyString "GetLocationInfo"]
                [PyTuple 0 items]
                [PyDict 0 kvp]

    [PyTuple 1 items]                   << response from server
      [PySubStream 43 bytes]
        [PyTuple 2 items]
          [PySubStruct]
            [PySubStream 32 bytes]
              [PyTuple 2 items]
                [PyString "N=699771:17106"]     << nodeID
                [PyIntegerVar 129503265956883696]   << filetime.  sent in "ClientHasReleasedTheseObjects" packet later
          [PyDict 0 kvp]
    [PyDict 1 kvp]
      [PyString "OID+"]
      [PyDict 1 kvp]
        [PyString "N=699771:17106"]
        [PyIntegerVar 129503265956883696]


    sLog.White("DogmaIMBound::Handle_GetLocationInfo()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);
                */

    // dummy right now, don't have any meaningful packet logs
    //response should be node data and timestamp
    return new PyLong(Win32TimeNow());
}

PyResult DogmaIMBound::Handle_CharGetInfo(PyCallArgs& call) {
    //no arguments
    PyDict* result = call.client->GetChar()->GetCharInfo();
    if (!result ) {
        _log(SERVICE__ERROR, "Unable to build char info for char %u", call.client->GetCharacterID());
        return new PyNone();
    }

    return result;
}

PyResult DogmaIMBound::Handle_ShipGetInfo(PyCallArgs& call) {
    //no arguments
    PyDict* result = call.client->GetShip()->ShipGetInfo();
    if (!result ) {
        _log(SERVICE__ERROR, "Unable to build ship info for ship %u", call.client->GetShipID());
        return new PyNone();
    }

    return result;
}

PyResult DogmaIMBound::Handle_ItemGetInfo(PyCallArgs& call) {
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Failed to decode arguments");
        return new PyNone();
    }

    InventoryItemRef item = m_manager->item_factory->GetItem(args.arg);
    if ( !item ) {
        _log(SERVICE__ERROR, "Unable to load item %u", args.arg);
        return new PyNone();
    }

    return item->ItemGetInfo();
}

PyResult DogmaIMBound::Handle_CheckSendLocationInfo(PyCallArgs& call)
{
    sLog.White("DogmaIMBound::Handle_CheckSendLocationInfo()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);
    //no arguments
    Client* pClient = call.client;
    return nullptr;
}

PyResult DogmaIMBound::Handle_SetModuleOnline(PyCallArgs& call) {
    Client* pClient = call.client;

    if (pClient->IsInSpace()) {
        DestinyManager* pDestiny = pClient->GetShipSE()->DestinyMgr();
        if (!pDestiny) {
            _log(PLAYER__ERROR, "%s: Client has no destiny manager!", pClient->GetName());
            return new PyNone();
        } else if (pDestiny->IsWarping()) {
            pClient->SendNotifyMsg("You can't do this while warping");
            return new PyNone();
        }
    }

	Call_TwoIntegerArgs args; //locationID, moduleID

	if (!args.Decode(&call.tuple)) {
        _log( SERVICE__ERROR, "Unable to decode arguments from '%s'", pClient->GetName() );
        return new PyNone();
    }

	pClient->GetShip()->Online(args.arg2);

    //response should be node data and timestamp
    return new PyLong(Win32TimeNow());
}

PyResult DogmaIMBound::Handle_TakeModuleOffline(PyCallArgs& call) {
    Client* pClient = call.client;

    if (pClient->IsInSpace()) {
        DestinyManager* pDestiny = pClient->GetShipSE()->DestinyMgr();
        if (!pDestiny) {
            _log(PLAYER__ERROR, "%s: Client has no destiny manager!", pClient->GetName());
            return new PyNone();
        } else if (pDestiny->IsWarping()) {
            pClient->SendNotifyMsg("You can't do this while warping");
            return new PyNone();
        }
    }

	Call_TwoIntegerArgs args; //locationID, moduleID

	if (!args.Decode(&call.tuple)) {
        _log( SERVICE__ERROR, "Unable to decode arguments from '%s'", pClient->GetName() );
        return new PyNone();
    }

	pClient->GetShip()->Offline(args.arg2);

    //response should be node data and timestamp
    return new PyLong(Win32TimeNow());
}

PyResult DogmaIMBound::Handle_LoadAmmoToModules(PyCallArgs& call) {
    Client* pClient = call.client;

    if (pClient->IsInSpace()) {
        DestinyManager* pDestiny = pClient->GetShipSE()->DestinyMgr();
        if (!pDestiny) {
            _log(PLAYER__ERROR, "%s: Client has no destiny manager!", pClient->GetName());
            return new PyNone();
        } else if (pDestiny->IsWarping()) {
            pClient->SendNotifyMsg("You can't do this while warping");
            return new PyNone();
        }
    }

    //  self.remoteDogmaLM.LoadAmmoToModules(shipID, moduleIDs, chargeTypeID, itemID, ammoLocationID, qty=qty)
    /* 02:13:11 [SvcCall]       Tuple: 5 elements
     * 02:13:11 [SvcCall]         [ 0] Integer field: 140000602     <- ship
     * 02:13:11 [SvcCall]         [ 1] List: 1 elements
     * 02:13:11 [SvcCall]         [ 1]   [ 0] Integer field: 140000454  <- module
     * 02:13:11 [SvcCall]         [ 2] Integer field: 196   <- charge type
     * 02:13:11 [SvcCall]         [ 3] Integer field: 140000623 <- charge item
     * 02:13:11 [SvcCall]         [ 4] Integer field: 60014749  <- charge location
     * 02:13:11 [SvcCall]   Call Named Arguments:
     * 02:13:11 [SvcCall]     Argument 'qty':
     * 02:13:11 [SvcCall]         (None)
     */
    Call_Dogma_LoadAmmoToModules args;
    if (!args.Decode(&call.tuple)) {
        _log( SERVICE__ERROR, "Handle_LoadAmmoToModules(): Unable to decode arguments from '%s'", pClient->GetName() );
        return new PyNone();
    }

    if (args.moduleIDs.empty())
        return new PyNone();

    // Get Reference to Ship, Module, and Charge
    ShipItemRef shipRef = pClient->GetShip();
    InventoryItemRef chargeRef = m_manager->item_factory->GetItem(args.itemID);
    Call_SingleIntList chargeList;

    for (uint8 i=0; i<args.moduleIDs.size(); ++i) {
        InventoryItemRef moduleRef = shipRef->GetModule(args.moduleIDs.at(i));
        if (!moduleRef) {
            sLog.Error("DogmaIMBound::Handle_LoadAmmoToModules()", "ERROR: cannot find module into which charge should be loaded." );
            continue;
        }

        EVEItemFlags moduleFlag = moduleRef->flag();
        uint32 loadedChargeID = shipRef->AddItem( moduleFlag, chargeRef );
        //Create new item id return result
        if (loadedChargeID)
            chargeList.ints.push_back(loadedChargeID);
    }
    //Return new item result
    return chargeList.Encode();
}

PyResult DogmaIMBound::Handle_LoadAmmoToBank(PyCallArgs& call) {
    Client* pClient = call.client;

    if (pClient->IsInSpace()) {
        DestinyManager* pDestiny = pClient->GetShipSE()->DestinyMgr();
        if (!pDestiny) {
            _log(PLAYER__ERROR, "%s: Client has no destiny manager!", pClient->GetName());
            return new PyNone();
        } else if (pDestiny->IsWarping()) {
            pClient->SendNotifyMsg("You can't do this while warping");
            return new PyNone();
        }
    }

  /*
   * self.remoteDogmaLM.LoadAmmoToBank(shipID, masterID, chargeTypeID,  itemIDs,  chargeLocationID,   qty)
   *                                    ship,   module,  charge type, charge item, charge location, stack qty (usually none - havent found otherwise)
   *   *******    UPDATED VAR NAMES TO MATCH CLIENT CODE  -allan 26Jul14  *************
   */
	Call_Dogma_LoadAmmoToBank args;

	if (!args.Decode(&call.tuple)) {
        _log( SERVICE__ERROR, "Handle_LoadAmmoToBank(): Unable to decode arguments from '%s'", pClient->GetName() );
        return new PyNone();
    }

	// NOTES:
	// args.itemIDs will contain one or more entries, each of which is an itemID of a charge or stack of charges
	// presumably, this list allows the player to select more than one stack of exact same ammo and drag whole selection
	// onto the module to be loaded into it; then what can be loaded is, and a single stack of the remainder quantity is
	// created and returned to the inventory the original selection of charges was pulled from.
	// -- this still must be fully confirmed by testing on hybrid or projectile turrets or missile batteries

	// WARNING!  Initial Implementation ONLY handles the FIRST entry in the args.itemIDs,
	// which is basically supporting only single charge stacks applied to module!

	/** @todo  update this to check all charges in args.itemIDs to see if they can be loaded also. */
	// Get Reference to Ship, Module, and Charge
	ShipItemRef shipRef = pClient->GetShip();
	InventoryItemRef moduleRef = shipRef->GetModule(args.masterID);
	if (!moduleRef) {
		sLog.Error("DogmaIMBound::Handle_LoadAmmoToBank()", "ERROR: cannot find module into which charge should be loaded." );
		return new PyNone();
	}

	if (!args.itemIDs.empty()) {
	    InventoryItemRef chargeRef = m_manager->item_factory->GetItem(args.itemIDs.at(0));
	    EVEItemFlags moduleFlag = moduleRef->flag();
	    uint32 loadedChargeID = shipRef->AddItem( moduleFlag, chargeRef );

		//Create new item id return result
	    if (loadedChargeID) {
		    Call_SingleIntegerArg result;
		    result.arg = loadedChargeID;	//chargeRef->itemID();
		    //Return new item result
		    return result.Encode();
		}
	}

	return nullptr;
}

PyResult DogmaIMBound::Handle_Activate(PyCallArgs& call)
{
    Client* pClient = call.client;

    if (pClient->IsInSpace()) {
        DestinyManager* pDestiny = pClient->GetShipSE()->DestinyMgr();
        if (!pDestiny) {
            _log(PLAYER__ERROR, "%s: Client has no destiny manager!", pClient->GetName());
            return new PyNone();
        } else if (pDestiny->IsWarping()) {
            pClient->SendNotifyMsg("You can't do this while warping");
            return new PyNone();
        }
    } else {
        pClient->SendNotifyMsg("You can't do this while docked");
        return new PyNone();
    }
/*
    sLog.White("DogmaIMBound::Handle_Activate()", "size= %u from '%s'", call.tuple->size(), pClient->GetName() );
    call.Dump(SERVICE__CALL_DUMP);
*/
    uint32 callTupleSize = (uint32)call.tuple->size(), itemID = 0, effectID = 0;

    if (callTupleSize == 2) {
        Call_TwoIntegerArgs args;
        if (!args.Decode(&call.tuple)) {
            _log( SERVICE__ERROR, "Unable to decode arguments from '%s'", pClient->GetName() );
            return new PyNone();
        }
        /*
          [PyString "Activate"]     << onlining a pos module
          [PyTuple 2 items]
            [PyIntegerVar 1002332856217]    << module itemID
            [PyInt 901]             << effectOnlineForStructures
        */
        // This call is for Anchor/Unanchor an Orbital structure or Cargo Container,
        //   get the new flag value and change the item referenced:
        if (call.tuple->items.at(0)->IsInt()) {
            itemID = call.tuple->items.at(0)->AsInt()->value();
            if (call.tuple->items.at(1)->IsInt()) {
                effectID = call.tuple->items.at(1)->AsInt()->value();
                SystemEntity* se = pClient->SystemMgr()->GetSE(itemID);
                if (!se) {
                    sLog.Error("DogmaIMBound::Handle_Activate()", "%u is not a valid EntityID in this system.", itemID);
                    return new PyNone();
                }

                /** @todo somehow notify client with one of these effects:
                 *  effectAnchorDrop = 649
                 *  effectAnchorLift = 650
                 *  effectAnchorDropForStructures = 1022
                 *  effectAnchorLiftForStructures = 1023
                 *
                 ** @todo  many more effects to send for.....look into later.
                 * effectOnlineForStructures = 901
                 *
                 ** @note  also note there are timers involved here...
                 */

                switch(effectID) {
                    case 649: //effectAnchorDrop;
                        //pClient->GetShipSE()->DestinyMgr()->SendContainerAnchor( pClient->services().item_factory->GetCargoContainer( itemID ) );
                        break;
                    case 650: //effectAnchorLift
                        //pClient->GetShipSE()->DestinyMgr()->SendContainerUnanchor( pClient->services().item_factory->GetCargoContainer( itemID ) );
                        break;
                    case 1022: //effectAnchorDropForStructures
                        //pClient->GetShipSE()->DestinyMgr()->SendStructureAnchor( pClient->services().item_factory->GetStructure( itemID ) );
                        break;
                    case 1023: //effectAnchorLiftForStructures
                        //pClient->GetShipSE()->DestinyMgr()->SendStructureUnanchor( pClient->services().item_factory->GetStructure( itemID ) );
                        break;
                    default:
                        break;
                }
            } else
                sLog.Error("DogmaIMBound::Handle_Activate()", "call.tuple->items.at( 1 ) was not PyInt expected type." );
        } else
            sLog.Error("DogmaIMBound::Handle_Activate()", "call.tuple->items.at( 0 ) was not PyInt expected type." );
    } else if (callTupleSize == 4) {
        Call_Dogma_Activate args;
        if (!args.Decode(&call.tuple)) {
            _log( SERVICE__ERROR, "Unable to decode arguments from '%s'", pClient->GetName() );
            return new PyNone();
        }

        //TODO: make sure we are allowed to do this.
        pClient->GetShip()->Activate(args.itemID, args.effectName, args.target, args.repeat);
    }

    return new PyNone();
}

PyResult DogmaIMBound::Handle_Deactivate(PyCallArgs& call)
{
    Client* pClient = call.client;

    if (pClient->IsInSpace()) {
        DestinyManager* pDestiny = pClient->GetShipSE()->DestinyMgr();
        if (!pDestiny) {
            _log(PLAYER__ERROR, "%s: Client has no destiny manager!", pClient->GetName());
            return new PyNone();
        } else if (pDestiny->IsWarping()) {
            pClient->SendNotifyMsg("You can't do this while warping");
            return new PyNone();
        }
    }

    call.Dump(SERVICE__CALL_DUMP);
    //18:50:24 [PacketError] Decode Call_Dogma_Deactivate failed: effectName is not a wide string: Integer      << called by POS->SetOffline
    //  this is also used on POS items, so adjust as needed.  (will have to construct it like Activate())

    Call_Dogma_Deactivate args;
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Unable to decode arguments from '%s'", pClient->GetName());
        return new PyNone();
    }

    //TODO: make sure we are allowed to do this.
    pClient->GetShip()->Deactivate(args.itemID, args.effectName);

    return nullptr;
}

PyResult DogmaIMBound::Handle_AddTarget(PyCallArgs& call) {
    Rsp_Dogma_AddTarget rsp;
        rsp.flag = false;
        rsp.targetList.push_back(0);

    Client* pClient = call.client;
    if (pClient->IsInSpace()) {
        DestinyManager* pDestiny = pClient->GetShipSE()->DestinyMgr();
        if (!pDestiny) {
            _log(PLAYER__ERROR, "%s: Client has no destiny manager!", pClient->GetName());
            return rsp.Encode();
        } else if (pDestiny->IsWarping()) {
            pClient->SendNotifyMsg("You can't do this while warping");
            return rsp.Encode();
        }
    }

    if (!pClient->GetShipSE()->TargetMgr())
        return rsp.Encode();

    SystemManager* smgr = pClient->SystemMgr();
    if (!smgr) {
        _log(PLAYER__WARNING, "Unable to find system manager from '%s'", pClient->GetName());
        return rsp.Encode();
    }

    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Unable to decode arguments from '%s'", pClient->GetName());
        return rsp.Encode();
    }
    SystemEntity* target = smgr->GetSE(args.arg);
    if (!target) {
        _log(INV__WARNING, "Unable to find entity %u in system %u from '%s'", args.arg, smgr->GetID(), pClient->GetName());
        return rsp.Encode();
    }
    if ((!pClient->GetShipSE()->SysBubble()) || (!target->SysBubble())) {
        _log(DESTINY__ERROR, "Client %u or Target %u does not have a bubble.", pClient->GetName(), target->GetName());
        return rsp.Encode();
    }

    if (!pClient->GetShipSE()->TargetMgr()->StartTargeting(target, pClient->GetShip())) {
        _log(TARGET__WARNING, "Handle_AddTarget - TargMgr.StartTargeting() failed.");
        return rsp.Encode();
    }

    if (sConfig.server.IsTestServer)
        if (is_log_enabled(TARGET__MESSAGE)) {
            GVector vectorToTarget(pClient->GetShipSE()->GetPosition(), target->GetPosition());
            _log(TARGET__MESSAGE, "Handle_AddTarget() - %s(%u) -> %s(%u) at range of %.2f meters.", \
                        pClient->GetName(), pClient->GetCharacterID(), target->GetName(),target->GetID(), vectorToTarget.length() );
        }

    rsp.flag = true;
    rsp.targetList.clear();
    rsp.targetList.push_back(target->GetID());
    return rsp.Encode();
}

PyResult DogmaIMBound::Handle_RemoveTarget(PyCallArgs& call) {
    Client* pClient = call.client;

    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Unable to decode arguments for '%s'", pClient->GetName());
        return new PyNone();
    }

    SystemManager* smgr = pClient->SystemMgr();
    if (!smgr) {
        _log(SERVICE__ERROR, "Unable to find system manager for '%s'", pClient->GetName());
        return new PyNone();
    }
    SystemEntity* target = smgr->GetSE(args.arg);
    if (!target) {
        _log(SERVICE__ERROR, "Unable to find entity %u in system %u for '%s'", args.arg, smgr->GetID(), pClient->GetName());
        return new PyNone();
    }

    if (sConfig.server.IsTestServer)
        if (is_log_enabled(TARGET__MESSAGE)) {
            GVector vectorToTarget(pClient->GetShipSE()->GetPosition(), target->GetPosition());
            _log(TARGET__MESSAGE, "Handle_RemoveTarget() - Removed %s(%u) - Range to Target: %.2f meters.", \
                            target->GetName(),target->GetID(), vectorToTarget.length() );
        }

    pClient->GetShipSE()->TargetMgr()->ClearTarget(target);
    return nullptr;
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


PyResult DogmaIMBound::Handle_GetWeaponBankInfoForShip(PyCallArgs& call) {
    /** @todo  look into this...what is it used for?  */
    return new BuiltinSet();
}

PyResult DogmaIMBound::Handle_GetAllInfo(PyCallArgs& call)
{
    //sLog.White("DogmaIMBound::Handle_GetAllInfo()", "size=%u", call.tuple->size());
    //call.Dump(SERVICE__CALL_DUMP);
    /* added more return data and updated logic (almost complete and mostly accurate) -allan 26Mar16 */
    /* Start the Code */
    Client* pClient = call.client;

    Call_TwoBoolArgs args; /* arg1: getCharInfo, arg2: getShipInfo */
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Unable to decode arguments from '%s'", pClient->GetName());
        return new PyNone();
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
     */
    rsp->SetItemString("locationInfo", new PyNone());

    /* Setting "shipModifiedCharAttribs" in the Dictionary */
    /** @todo  havent found a populated item in packet logs */
    rsp->SetItemString("shipModifiedCharAttribs", new PyNone());

    /* Setting "charInfo" in the Dictionary  -fixed 24Mar16 */
    if (args.arg1) {
        PyDict* charResult = pClient->GetChar()->GetCharInfo();
        if (!charResult ) {
            _log(SERVICE__ERROR, "Unable to build char info for char %u", pClient->GetCharacterID());
            return new PyNone();
        }
        rsp->SetItemString("charInfo", charResult);
    } else  // fixed
        rsp->SetItemString("charInfo", new PyDict());

	/* Setting "shipInfo" in the Dictionary  -fixed 26Mar16 */
	if (args.arg2) {
        PyDict* shipResult = pClient->GetShip()->GetShipInfo();
        if (!shipResult ) {
            _log(SERVICE__ERROR, "Unable to build ship info for ship %u", pClient->GetShipID());
            return new PyNone();
        }
        rsp->SetItemString("shipInfo", shipResult);
    } else
        rsp->SetItemString("shipInfo", new PyDict());

    /* Setting "shipState" in the Dictionary  -fixed 26Mar16 */
    if (!pClient->GetShip()) {
        _log(SERVICE__ERROR, "Unable to build shipState for %u", pClient->GetShipID());
        return new PyNone();
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

