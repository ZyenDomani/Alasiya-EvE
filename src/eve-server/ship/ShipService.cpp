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
    Author:     Zhur
    Updates:    Allan
*/

#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"

#include "EVEServerConfig.h"
#include "pos/Structure.h"
#include "system/DestinyManager.h"
#include "ship/ShipService.h"
#include "system/Container.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"

class ShipBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(ShipBound)

    ShipBound(PyServiceMgr *mgr, ShipDB& db)
    : PyBoundObject(mgr),
      m_db(db),
      m_dispatch(new Dispatcher(this))
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "ShipBound";

        PyCallable_REG_CALL(ShipBound, Board);
        PyCallable_REG_CALL(ShipBound, Eject);
        PyCallable_REG_CALL(ShipBound, LeaveShip);
        PyCallable_REG_CALL(ShipBound, ActivateShip);
        PyCallable_REG_CALL(ShipBound, Undock);
        PyCallable_REG_CALL(ShipBound, AssembleShip);
        PyCallable_REG_CALL(ShipBound, Drop);
        PyCallable_REG_CALL(ShipBound, Scoop);
        PyCallable_REG_CALL(ShipBound, ScoopDrone);
        PyCallable_REG_CALL(ShipBound, Jettison);
        PyCallable_REG_CALL(ShipBound, GetShipConfiguration);
        PyCallable_REG_CALL(ShipBound, SelfDestruct);
    }

    virtual ~ShipBound() {delete m_dispatch;}
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(Board);
    PyCallable_DECL_CALL(Eject);
    PyCallable_DECL_CALL(LeaveShip);
    PyCallable_DECL_CALL(ActivateShip);
    PyCallable_DECL_CALL(Undock);
    PyCallable_DECL_CALL(AssembleShip);
    PyCallable_DECL_CALL(Drop);
    PyCallable_DECL_CALL(Scoop);
    PyCallable_DECL_CALL(ScoopDrone);
    PyCallable_DECL_CALL(Jettison);
    PyCallable_DECL_CALL(GetShipConfiguration);
    PyCallable_DECL_CALL(SelfDestruct);

protected:
    ShipDB& m_db;
    Dispatcher *const m_dispatch;
};

PyCallable_Make_InnerDispatcher(ShipService)

ShipService::ShipService(PyServiceMgr *mgr)
: PyService(mgr, "ship"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    //PyCallable_REG_CALL(ShipService,);
}

ShipService::~ShipService() {
    delete m_dispatch;
}

PyBoundObject *ShipService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
    _log(CLIENT__MESSAGE, "ShipService bind request");
    bind_args->Dump(COLLECT__OTHER_DUMP, "    ");
    return new ShipBound(m_manager, m_db);
}

/* only called in space */
PyResult ShipBound::Handle_Board(PyCallArgs &call) {
    /*if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change already active.");
        return new PyNone();
    }*/
    sLog.Log("ShipBound::Handle_Board()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    Call_BoardShip args;
    //     .arg1 (newShipID) -  itemID of the ship to be boarded
    //     .arg2 (oldShipID) -  itemID of the current ship
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Handle_Board Failed to decode arguments");
        return nullptr;
    }

    Client* pClient = call.client;
    if (pClient->GetShipSE()->DestinyMgr()->IsMoving()) {
        throw PyException(MakeCustomError("You cannot change ships while moving."));
        return nullptr;
    }

    SystemManager* pSysMgr = pClient->SystemMgr();
    GPoint oldPosition(pClient->GetShipSE()->GetPosition());

    // Get ship ItemRefs
    ShipItemRef oldShipRef = pSysMgr->GetShipFromInventory(args.oldShipID);
    if (!oldShipRef)
        oldShipRef = pClient->services().item_factory->GetShip(args.oldShipID);
    ShipItemRef newShipRef = pSysMgr->GetShipFromInventory(args.newShipID);
    if (!newShipRef)
        newShipRef = pClient->services().item_factory->GetShip(args.newShipID);

    if (!newShipRef) {
        _log(SHIP__ERROR, "Handle_Board() - Failed to get new ship %u for %s.", args.newShipID, pClient->GetName());
        throw PyException(MakeCustomError("Something bad happened as you prepared to board the ship"));
        return nullptr;
    }

    if (newShipRef->typeID() == itemTypeCapsule) {
        codelog(ITEM__ERROR, "Empty Pod %u in space.  SystemID %u.", args.newShipID, pSysMgr->GetID());
        throw PyException(MakeCustomError("You already have a pod.  These cannot be boarded manally."));
        return nullptr;
    }

    if (!newShipRef->ValidateBoardShip(newShipRef, pClient->GetChar())) {
        // should we eject player here and deny boarding new ship, or just leave char in current ship and return?
        throw PyException(MakeCustomError("You do not have the skills to fly a %s.", newShipRef->itemName().c_str()));
        return nullptr;
    }

    bool isPod = pClient->InPod();

    // Change ownership of new ship to this character
    newShipRef->ChangeOwner(pClient->GetCharacterID());

    pClient->BoardShip(newShipRef);

    //  if not pod, bubblecast updated info for old ship
    if (!isPod) {
        oldShipRef->ChangeOwner(1);
        pClient->GetShipSE()->DestinyMgr()->SendBallInteractive(oldShipRef);
        pClient->GetShipSE()->DestinyMgr()->UpdateOldShip(oldShipRef);
        char ci[1];
        snprintf(ci, sizeof(ci), "");
        oldShipRef->SetCustomInfo(ci);
        oldShipRef->SetFlag(flagShipOffline);
    }

    /* missing something here.  blank space after boarding ship from pod.  */

    //response should be nodeid and timestamp
    return new PyLong(Win32TimeNow());
}

/* only called in space */
PyResult ShipBound::Handle_Eject(PyCallArgs &call) {
    /*if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change already active.");
        return nullptr;
    }*/
    sLog.Log("ShipBound::Handle_Eject()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);
    //no arguments.

    Client* pClient = call.client;
    /** @todo create and implement "Weapon Flag"....
     *      Weapon Flag --  the 60-sec timer started upon any offensive weapon activation
     *   this will be in client's criminaltimer object
     *
     * if (pClient->CrimMgr()->IsWeaponFlagActive())
     *  deny eject
     */

    SystemEntity* pShipSE = pClient->GetShipSE();
    /** @todo  check for active cyno (when we implement it...) and other things that affect eject */
    if (pShipSE->IsVisibleSystemWide()) { /* close enough.  cyno IS IsVisibleSystemWide(), so this will work */
        /* find proper error msg for this...im sure there is one  */
        call.client->SendNotifyMsg("You cannot eject with an active Cyno Field.");
        return nullptr;
    }

    if (pShipSE->DestinyMgr()->IsMoving()) {
        throw PyException(MakeCustomError("You cannot eject while moving."));
        return nullptr;
    }

    SystemManager* pSysMgr = pClient->SystemMgr();
    GPoint oldPosition(pShipSE->GetPosition());
    GPoint capsulePosition(oldPosition);
    capsulePosition.MakeRandomPointOnSphere(pClient->GetShip()->GetAttribute(AttrRadius).get_float() + (MakeRandomFloat(300, 400)));

    // Get ship ItemRefs
    ShipItemRef oldShipRef = pClient->GetShip();
    if (!oldShipRef)
        oldShipRef = pClient->services().item_factory->GetShip(pClient->GetShipID());
    ShipItemRef capsuleRef = pSysMgr->GetShipFromInventory(pClient->GetPodID());
    if (!capsuleRef)
        capsuleRef = pClient->services().item_factory->GetShip(pClient->GetPodID());
    capsuleRef->Relocate(capsulePosition);

    /* all previous SE and DestinyMgr objects are updated to new ship object here */
    pClient->BoardShip(capsuleRef);
    pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket(oldShipRef);
    pClient->GetShipSE()->DestinyMgr()->UpdateOldShip(oldShipRef);
    /* missing something here....capsule has no data. */

    char ci[1];
    snprintf(ci, sizeof(ci), "");
    oldShipRef->ChangeOwner(1);
    oldShipRef->SetCustomInfo(ci);
    oldShipRef->SetFlag(flagShipOffline);
    //response should be nodeid and timestamp
    return new PyLong(Win32TimeNow());
}

// NOTE  LeaveShip and ActivateShip are working.  dont fuck with them
/* only called when docked. */
PyResult ShipBound::Handle_LeaveShip(PyCallArgs &call) {
    /*if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change already active.");
        return nullptr;
    }*/
    sLog.Log("ShipBound::Handle_LeaveShip()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Handle_LeaveShip Failed to decode arguments");
        return nullptr;
    }

    //  sends itemID of ship to leave
    Client* pClient = call.client;
    ShipItemRef shipRef = pClient->SystemMgr()->GetShipFromInventory(arg.arg);
    uint32 podID = pClient->GetPodID();
    ShipItemRef podRef = pClient->SystemMgr()->GetShipFromInventory(podID);
    if (!podRef)
        podRef = pClient->services().item_factory->GetShip(podID);

    //verify owner (not sure why pod doenst have correct owner...)
    podRef->ChangeOwner(pClient->GetCharacterID(), false);
    //move capsule into the players hangar
    sLog.Log("ShipBound::Handle_LeaveShip()", "moving pod %u to station %u", podID, pClient->GetStationID());
    podRef->Move(pClient->GetStationID(), flagHangar);

    // capsuleID = shipsvc.LeaveShip(shipid)
    return new PyInt(podID);
}

/* only called when docked. */
PyResult ShipBound::Handle_ActivateShip(PyCallArgs &call) {
    /*if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change already active.");
        return nullptr;
    }*/
    Call_BoardShip args;
    //     .arg1 (newShipID) -  itemID of the ship to be boarded
    //     .arg2 (oldShipID) -  itemID of the current ship
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "Handle_ActivateShip Failed to decode arguments");
        return nullptr;
    }

    Client* pClient = call.client;
    ShipItemRef oldShipRef = pClient->SystemMgr()->GetShipFromInventory(args.oldShipID);
    if (!oldShipRef)
        oldShipRef = pClient->services().item_factory->GetShip(args.oldShipID);

    ShipItemRef newShipRef = pClient->SystemMgr()->GetShipFromInventory(args.newShipID);
    if (!newShipRef)
        newShipRef = pClient->services().item_factory->GetShip(args.newShipID);
    if (!newShipRef) {
        sLog.Error("ShipBound::Handle_ActivateShip()", "%s: Failed to get new ship %u.", pClient->GetName(), args.newShipID);
        throw PyException(MakeCustomError("Something bad happened as you prepared to board the ship.  Ref: ServerError 15173"));
        return nullptr;
    }

    if (oldShipRef->typeID() == itemTypeCapsule) {
        // take pod out of station hangar and relocate to system origin
        oldShipRef->SetFlag(flagCapsule);
    }

    pClient->BoardShip(newShipRef);

    // response should return ship modules and loaded charges
    PyDict* modules = pClient->GetShip()->GetShipState();
    PyDict* charges = pClient->GetShip()->GetChargeState();
    PyTuple* rsp = new PyTuple(3);
        rsp->SetItem(0, modules);    //dict of ship modules
        rsp->SetItem(1, charges);    //dict of ship charges
        rsp->items[2] = new BuiltinSet();
    return rsp;
}

PyResult ShipBound::Handle_Undock(PyCallArgs &call) {
    /*
    if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change already active.");
        return nullptr;
    }*/
  /**
       shipsvc.Undock(shipID, ignoreContraband, onlineModules=onlineModules)
       */

    Call_IntBoolArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        /** @todo throw exception */
        return nullptr;
    }

    Client* pClient = call.client;
    ShipItem* pShip = pClient->GetShip().get();

    char ci[35];
    snprintf(ci, sizeof(ci), "Undocking:%u", pClient->GetLocationID());
    pShip->SetCustomInfo(ci);

    bool ignoreContraband = args.arg2;

    //  get vector of online modules as (k,v) pair,
    //    where key is slotID, value is moduleID
    if (call.byname.find("onlineModules") != call.byname.end()) {
        if (is_log_enabled(SHIP__MODULE_INFO)) {
            _log(SHIP__MODULE_INFO, "Dumping 'onlineModules' List");
            call.byname["onlineModules"]->Dump(SHIP__MODULE_INFO, "   ");
        }
        PyDict* onlineModules = call.byname["onlineModules"]->AsDict();
        PyDict::const_iterator cur = onlineModules->begin();
        for (; cur != onlineModules->end(); cur++)
            pShip->AddModuleToOnlineVec(cur->second->AsInt()->value());
    }

    //do session change...
    pClient->UndockFromStation();

    //response should be nodeid and timestamp
    return new PyLong(Win32TimeNow());
}

PyResult ShipBound::Handle_AssembleShip(PyCallArgs &call) {
    /** @todo handle multiple-ship list and Return correct values */

    /* 13:05:41 [BindDump] NodeID: 888444 BindID: 129 calling AssembleShip in service manager 'ShipBound'
     * 13:05:41 [BindDump]   Call Arguments:
     * 13:05:41 [BindDump]       Tuple: 1 elements
     * 13:05:41 [BindDump]         [ 0] List: 5 elements
     * 13:05:41 [BindDump]         [ 0]   [ 0] Integer field: 140000073
     * 13:05:41 [BindDump]         [ 0]   [ 1] Integer field: 140000074
     * 13:05:41 [BindDump]         [ 0]   [ 2] Integer field: 140000075
     * 13:05:41 [BindDump]         [ 0]   [ 3] Integer field: 140000076
     * 13:05:41 [BindDump]         [ 0]   [ 4] Integer field: 140000077
     *
              [PyTuple 2 items]     << response to AssembleShip call
                [PyList 1 items]
                  [PyPackedRow 33 bytes]
                    ["itemID" => <1002333477860> [I8]]
                    ["typeID" => <24700> [I4]]
                    ["ownerID" => <1661059544> [I4]]
                    ["locationID" => <61000012> [I8]]
                    ["flagID" => <4> [I2]]
                    ["quantity" => <-1> [I4]]
                    ["groupID" => <419> [I2]]
                    ["categoryID" => <6> [I2]]
                    ["customInfo" => <empty string> [Str]]
                [PyDict 1 kvp]
                  [PyInt 10]        << flagdisconnect??
                  [PyInt 0]
            */
    Call_AssembleShip args;
    Call_AssembleShipTech3 argsT3;
    uint32 itemID = 0;
    std::vector<uint32> subSystemList;
    bool completeTech3Assembly = false;

    if (!call.tuple->IsTuple())
        return nullptr;

    if (!call.tuple->GetItem(0)->IsList()) {
        if (!call.tuple->GetItem(0)->IsInt()) {
            sLog.Error("ShipBound::Handle_AssembleShip()", "Failed to decode arguments: call.tuple->GetItem(0)->IsInt() == false");
            /** @todo  throw exception */
            return nullptr;
        } else {
        	// T3 managing is broken now - logging on with T3 assembled causes seg fault.
            /** @todo Re-work the T3 managing. For now, i'm commenting it and leaving an error message on assembly attempt. */
        	sLog.Error( "Handle_AssembleShip", "Modular ships are not implemented yet" );
        	throw PyException( MakeCustomError( "Modular ships are not implemented yet." ) );
        	return nullptr;
        	/*
            // Tuple contains single Integer, this is for Tech 3 Ship Assembly:
            if (!argsT3.Decode(&call.tuple)) {
                sLog.Error("ShipBound::Handle_AssembleShip()", "Failed to decode arguments: argsT3.Decode(&call.tuple) failed");
                return nullptr;
            }
            itemID = argsT3.item;
            if (call.byname.find("subSystems") != call.byname.end()) {
                PyList * list;
                if (call.byname.find("subSystems")->second->IsList()) {
                    list = call.byname.find("subSystems")->second->AsList();
                    for(uint32 i=0; i<list->size(); i++)
                        subSystemList.push_back(list->GetItem(i)->AsInt()->value());
                } else {
                    sLog.Error("ShipBound::Handle_AssembleShip()", "Failed to decode arguments: !call.byname.find(\"subSystems\")->second->IsList() failed");
                    return nullptr;
                }
            } else {
                sLog.Error("ShipBound::Handle_AssembleShip()", "Failed to decode arguments: call.byname.find(\"subSystems\") != call.byname.end() failed");
                return nullptr;
            }
            completeTech3Assembly = true;
            */
        }
    } else {
        if (!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "Failed to decode arguments");
            return nullptr;
        }
        itemID = args.items.front();
    }

    ShipItemRef ship = m_manager->item_factory->GetShip(itemID);

    if (!ship) {
        _log(ITEM__ERROR, "Failed to load ship %u to assemble.", itemID);
        return nullptr;
    }

    //check if the ship is a stack
    if (ship->quantity() > 1) {
        // Split the stack into a new inventory item (new_item) with quantity minus one,
        // original item (ship) will be left with quantity = 1, then will be assembled:
        //InventoryItemRef new_item = ship->Split(ship->quantity()-1,true);
		ship = RefPtr<ShipItem>::StaticCast(ship->Split(1, true));
		if (!ship) {
		    _log(ITEM__ERROR, "Failed to split stack to assemble ship %u.", itemID);
			return nullptr;
		}
    }

    ship->ChangeSingleton(true, true);

    if (completeTech3Assembly) {
        // Move the five specified subsystems to the newly assembled Tech 3 ship
        InventoryItemRef subSystemItem;
        for(uint32 index=0; index<subSystemList.size(); index++) {
            subSystemItem = m_manager->item_factory->GetItem(subSystemList.at(index));
            subSystemItem->Move(ship->itemID(), (EVEItemFlags)(subSystemItem->GetAttribute(AttrSubSystemSlot).get_int()));
        }
    }

    return nullptr;
}

/** @todo this needs work.... */
PyResult ShipBound::Handle_Drop(PyCallArgs &call) {
    if (IsStation(call.client->GetLocationID())) {
        _log(SERVICE__ERROR, "%s: Trying to drop items when not in space!", call.client->GetName());
        return(new PyList());
    }
    sLog.Log("ShipBound::Handle_Drop()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    if (call.tuple->size() != 3) {
        sLog.Error("ShipBound::Handle_Drop()", "call.tuple wrong size, expected 3 items, actual size = %u", call.tuple->size());
        //TODO: throw exception
        return nullptr;
    }

    Call_Drop3 drop3args;
    if (!drop3args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        //TODO: throw exception
        return nullptr;
    }
/*
20:06:51 [BindDump] NodeID: 888444 BindID: 124 calling Drop in service manager 'ShipBound'
20:06:51 [BindDump]   Call Arguments:
20:06:51 [BindDump]       Tuple: 3 elements
20:06:51 [BindDump]         [ 0] List: 1 elements
20:06:51 [BindDump]         [ 0]   [ 0] Tuple: 2 elements
20:06:51 [BindDump]         [ 0]   [ 0]   [ 0] Integer field: 140000078
20:06:51 [BindDump]         [ 0]   [ 0]   [ 1] Integer field: 1
20:06:51 [BindDump]         [ 1] Integer field: 1000172
20:06:51 [BindDump]         [ 2] Boolean field: false
20:06:51[00m L [37;01mShipBound::Handle_Drop(): [00msize=3
[00m20:06:51 [TargetInfo] Created TargMgr 0x39f0500 for Minmatar Control Tower Small(140000078)
20:06:51 [DestinyMsg] MakeSlimItem for StructureSE 140000078
20:06:51[36;01m W [37;01mClient::BeanCount: [36;01m(BeanCount/alert) BeanCount error reporting and handling is not implemented yet.
[00m20:06:51 [ClientCallRep] SendClientStackTraceAlert call made to alert
EXCEPTION #10 logged at  05/23/2016 20:06:51 Unhandled exception in <TaskletExt object at 35ff1df0, abps=1001, ctxt=None>
Caught at:
/common/lib/bluepy.py(98) CallWrapper
Thrown at:
/common/lib/bluepy.py(86) CallWrapper
/../carbon/client/script/ui/control/menu.py(517) <lambda>
/client/script/ui/services/menusvc.py(6235) CheckLocked
/client/script/ui/services/menusvc.py(6332) LaunchForCorp
/client/script/util/evemisc.py(119) LaunchFromShip
        errors = set()
        newIDs = {}
        ignoreWarning = False
        items = [<DBRow object [140000078L, 20066, 140000000, 140000068L, 5, 1, 365, 23, '', 1, 0]>]
        PackError = <function PackError at 0x0B392AB0>
        item = <DBRow object [140000078L, 20066, 140000000, 140000068L, 5, 1, 365, 23, '', 1, 0]>
        UnpackError = <function UnpackError at 0x38A355F0>
        ret = ([...],)
        whoseBehalfID = 1000172
        oldItems = [(...)]
AttributeError: 'tuple' object has no attribute 'iteritems'
*/
    PyList* PyToDropList = drop3args.toDrop;
    uint32 ownerID = drop3args.ownerID;
    //used for LaunchUpgradePlatformWarning
    bool ignoreWarning = drop3args.ignoreWarning;

    Call_SingleIntList successfully_dropped;
    Client* pClient = call.client;
    SystemManager* pSysMgr = pClient->SystemMgr();
    uint32 contID = 0, itemID = 0, itemQuantity = 0;

    GPoint location(pClient->GetShipSE()->GetPosition());
    location.MakeRandomPointOnSphereLayer(400.0,(1000.0 + pClient->GetShipSE()->GetRadius()));

    StructureItemRef structureRef;
    InventoryItemRef cargoItemRef;
    CargoContainerRef contRef;

    for (uint32 i = 0; i < PyToDropList->size(); i++) {
        itemID = (uint32)(PyToDropList->items.at(i)->AsTuple()->items.at(0)->AsInt()->value());
        itemQuantity = (uint32)(PyToDropList->items.at(i)->AsTuple()->items.at(1)->AsInt()->value());

        cargoItemRef = m_manager->item_factory->GetItem(itemID);
        if (!cargoItemRef) {
            sLog.Error("ShipBound::Handle_Drop()", "%s: Unable to find item %u to drop.", pClient->GetName(), itemID);
            continue;
        }

        //verify that this item is in fact in the player's ship.
        if (cargoItemRef->locationID() != pClient->GetShipID()) {
            sLog.Error("ShipBound::Handle_Drop()", "%s: Item %u is not in our ship (%u), it is in %u. Not dropping.",
                       pClient->GetName(), itemID, pClient->GetShipID(), cargoItemRef->locationID());
            continue;
        }

        // Check drop for char or corp
        if ((IsPlayerCorp(ownerID)) || (ownerID == pClient->GetCharacterID()))
            cargoItemRef->ChangeOwner(ownerID, true);
        else
            cargoItemRef->ChangeOwner(1, true);  //default to eve system

        // Get groupID and categoryID for item 'itemID' to determine if it is a kind of cargo container, structure, or deployable item
        uint32 groupID = m_manager->item_factory->GetItem(itemID)->groupID();

        if ((groupID == EVEDB::invGroups::Audit_Log_Secure_Container)
            || (groupID == EVEDB::invGroups::Secure_Cargo_Container)
            || (groupID == EVEDB::invGroups::Freight_Container)
            || (groupID == EVEDB::invGroups::Cargo_Container))
        {
            // This item is a cargo container, so move it from the ship's cargo into space:
            contRef = m_manager->item_factory->GetCargoContainer(itemID);

            if (!contRef)
                throw PyException(MakeCustomError("Unable to spawn item of type %u.", contRef->typeID()));

            // Move item from cargo bay to space:
            contRef->Move(pClient->GetLocationID(), flagAutoFit, true);
            ContainerData cargoData;
                cargoData.allianceID = pClient->GetAllianceID();
                cargoData.corporationID = pClient->GetCorporationID();
                cargoData.factionID = pClient->GetWarFactionID();
                cargoData.ownerID = pClient->GetCharacterID();
            ContainerSE* cSE = new ContainerSE(contRef, *m_manager, pSysMgr, cargoData);
            cSE->SetPosition(location);
            contRef->SetMySE(cSE);
            contRef->SaveItem();
            pSysMgr->AddEntity(cSE);

            // Send notification SFX effects.jettison for the jettisoned Container object:
            pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket(contRef);
            successfully_dropped.ints.push_back(contRef->itemID());
            continue;
        }

        uint32 categoryID = m_manager->item_factory->GetItem(itemID)->categoryID();
        if (categoryID == EVEDB::invCategories::Structure) {
            // This item is a POS structure of some kind, so move it from the ship's cargo into space
            structureRef = m_manager->item_factory->GetStructure(itemID);

            if (!structureRef)
                throw PyException(MakeCustomError("Unable to spawn Structure item of type %u.", structureRef->typeID()));

            // Move item from cargo bay to space:
            structureRef->Move(pClient->GetLocationID(), flagAutoFit, true);
            StructureSE* structureEnt = new StructureSE(structureRef, *m_manager, pSysMgr);
            structureEnt->SetPosition(location);
            structureRef->SaveItem();
            pSysMgr->AddEntity(structureEnt);

            // Send notification SFX effects.jettison for the jettisoned Structure object:
            pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket(structureRef);
            successfully_dropped.ints.push_back(structureRef->itemID());
            continue;
        } else if (categoryID == EVEDB::invCategories::Deployable) {
            // This item is a Deployable item of some kind, so move it from the ship's cargo into space

            //cargoItemRef = m_manager->item_factory->GetItem(itemID);
            if (!cargoItemRef)
                throw PyException(MakeCustomError("Unable to spawn Deployable item of type %u.", cargoItemRef->typeID()));

            // Move item from cargo bay to space:
            cargoItemRef->Move(pClient->GetLocationID(), flagAutoFit, true);
            //flagUnanchored: for some DUMB reason, this flag, 1023 yields a PyNone when notifications
            // are created inside InventoryItem::Move() from passing it into a PyInt() constructor...WTF?
            DeployableSE* deployableObj = new DeployableSE(cargoItemRef, *m_manager, pSysMgr);
            deployableObj->SetPosition(location);
            cargoItemRef->SaveItem();
            pSysMgr->AddEntity(deployableObj);

            // Send notification SFX effects.jettison for the jettisoned Deployable object:
            pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket(cargoItemRef);
            successfully_dropped.ints.push_back(cargoItemRef->itemID());
            continue;
        } else if (cargoItemRef->flag() == flagDroneBay && cargoItemRef->categoryID() == EVEDB::invCategories::Drone) {
            if (!sConfig.npc.EnableDrones) {
                pClient->SendNotifyMsg("Drones are disabled.");
                return nullptr;
            }
            // This item is a drone, so launch it into space:
            if (pClient->LaunchDrone(cargoItemRef))
                successfully_dropped.ints.push_back(cargoItemRef->itemID());
            continue;
        }
        //else if ()    // Handle other types of cargo, such as launching assembled ships?
        else {
            ;// Reject launch for this item
        }
    }
    return (successfully_dropped.Encode());
}

PyResult ShipBound::Handle_Scoop(PyCallArgs &call) {
    //change this to call singleintarg
    if (!(call.tuple->items.at(0)->IsInt())) {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        //TODO: throw exception
        return nullptr;
    }

    uint32 objectItemID = call.tuple->items.at(0)->AsInt()->value();

    Client* pClient = call.client;
    SystemManager *pSysMgr = pClient->SystemMgr();
    SystemEntity *object = pSysMgr->GetSE(objectItemID);
    if (object == NULL) {
        _log(SERVICE__ERROR, "%s: Unable to find object %u to scoop.", pClient->GetName(), objectItemID);
        return nullptr;
    }

    InventoryItemRef item = object->GetSelf();

    /** @todo check ownership of this object, ie does this character/corporation own this object? */
    // do we really need to do this for anything except for drones that are under control of another player?

    /** @todo  check to see if this object is anchored and if so, refuse to scoop it */

    // Check cargo bay capacity:
    double capacity = pClient->GetShip()->GetInventory()->GetCapacity(flagCargoHold);
    double volume = item->GetAttribute(AttrVolume).get_float();
    if (capacity < volume)
        throw PyException(MakeCustomError("%s is too large to fit in remaining Cargo bay capacity.", item->itemName().c_str()));
    else
    {
        // We have enough Cargo bay capacity to hold the item being scooped,
        // so take ownership of it and move it into the cargo bay:
        item->ChangeOwner(pClient->GetCharacterID(), true);

        pClient->MoveItem(item->itemID(), pClient->GetShipID(), flagCargoHold);
    }

    return nullptr;
}

PyResult ShipBound::Handle_ScoopDrone(PyCallArgs &call) {
    /*
            [PyString "ScoopDrone"]
            [PyTuple 1 items]
              [PyList 3 items]
                [PyIntegerVar 1540263056]
                [PyIntegerVar 1540263058]
                [PyIntegerVar 1530423394]

                those items are placed into drone hold, then just a RemoveBalls packet after this.
        */
    Call_SingleIntList args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        return nullptr;
    }

    Client* pClient = call.client;
    SystemManager* pSysMgr = pClient->SystemMgr();
    std::vector<int32>::const_iterator cur = args.ints.begin();
    for(; cur != args.ints.end(); cur++) {
        SystemEntity* pDroneSE = pSysMgr->GetSE(*cur);
        if (!pDroneSE) {
            _log(SERVICE__ERROR, "%s: Unable to find drone %u to scoop.", pClient->GetName(), *cur);
            continue;
        }

        InventoryItemRef item = pDroneSE->GetSelf();

        // Check to see that this is really a drone:
        pClient->GetShip()->ValidateAddItem(flagDroneBay, item);

        /** @todo check ownership/control. */

        // Check drone bay capacity:
        double capacity = pClient->GetShip()->GetInventory()->GetCapacity(flagDroneBay);
        double volume = item->GetAttribute(AttrVolume).get_float();
        if (capacity < volume)
            throw PyException(MakeCustomError("%s is too large to fit in remaining Drone bay capacity.", item->itemName().c_str()));
        else
        {
            // We have enough Drone bay capacity to hold the drone,
            // so take ownership of it and move it into the Drone bay:
            item->ChangeOwner(pClient->GetCharacterID(), true);

            pClient->MoveItem(item->itemID(), pClient->GetShipID(), flagDroneBay);
            pClient->GetShipSE()->SysBubble()->Remove(pDroneSE);

            // Remove drone entity from SystemManager:
            pSysMgr->RemoveEntity(pDroneSE);
            /** @todo  delete the SE for this drone. */
        }
    }

    return nullptr;
}

PyResult ShipBound::Handle_Jettison(PyCallArgs &call) {
    Call_SingleIntList args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode arguments");
        //TODO: throw exception
        return nullptr;
    }

    Client* pClient = call.client;
    if (!pClient->IsInSpace()) {
        _log(SERVICE__ERROR, "%s: Trying to jettison items when not in space!", pClient->GetName());
        return nullptr;
    }

    SystemManager* pSysMgr = pClient->SystemMgr();
    SystemEntity* pSysEntity(nullptr);
    //Get location of our ship
    GPoint location(pClient->GetShipSE()->GetPosition());

    InventoryItemRef cargoItemRef, invItemRef;
    CargoContainerRef newJetcanItem, cargoContainerItem;
    StructureItemRef structureItemRef;
    uint32 groupID = 0, categoryID = 0;

    //args contains id's of items to jettison
    std::vector<int32>::iterator cur = args.ints.begin();
    // loop thru items to see if there is a container in this list.
    for (; cur != args.ints.end(); cur++) {
        invItemRef = m_manager->item_factory->GetItem(*cur);
        if (!invItemRef) continue;
        groupID = invItemRef->groupID();

        if ((groupID == EVEDB::invGroups::Audit_Log_Secure_Container)
            || (groupID == EVEDB::invGroups::Secure_Cargo_Container)
            || (groupID == EVEDB::invGroups::Freight_Container))
        {
            /** @todo (allan)  check these for accuracy  */
            /** @todo (allan)  *****  there are stipulations on placement of these items.  *****  */
            cargoContainerItem = m_manager->item_factory->GetCargoContainer(*cur);
            if (!cargoContainerItem)
                throw PyException(MakeCustomError("Unable to spawn item of type %u.", cargoContainerItem->typeID()));

            // Move item from cargo bay to space:
            cargoContainerItem->Move(pClient->GetLocationID(), flagAutoFit, true);
            // and add to the system manager
            //ContainerEntity* containerObj = new ContainerEntity(cargoContainerItem, pSysMgr, *m_manager, location);
            pSysEntity = pSysMgr->GetSE(cargoContainerItem->itemID());
            location.MakeRandomPointOnSphere(500.0);
            pSysEntity->SetPosition(location);
            cargoContainerItem->SaveItem();
            pSysMgr->AddEntity(pSysEntity);
            pSysEntity = nullptr;

            // Send notification SFX effects.jettison for the jettisoned Container object:
            pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket(cargoContainerItem);

            // container found.  remove this item from list, then break out of here and use to contain all other non-pos items
            args.ints.erase(cur);
            cur = args.ints.end();
        }
    }

    // reset iterator and loop thru list.
    for (auto cur : args.ints) {
        // loop thru remaining items and determine if cur is a structure or deployable item
        invItemRef = m_manager->item_factory->GetItem(cur);
        if (!invItemRef)
            continue;
        categoryID = invItemRef->categoryID();

        if (categoryID == EVEDB::invCategories::Structure) {
            /** @todo (allan)  these need to be rewrote for correct class constructors  */

            // This item is a POS structure of some kind, so move it from the ship's cargo into space
            // whilst keeping ownership of it to the character not using the corporation the character belongs to:
            structureItemRef = m_manager->item_factory->GetStructure(cur);
            if (!structureItemRef)
                throw PyException(MakeCustomError("Unable to spawn Structure item of type %u.", structureItemRef->typeID()));

            structureItemRef->Move(pClient->GetLocationID(), flagAutoFit, true);
            pSysEntity = pSysMgr->GetSE(structureItemRef->itemID());
            location.MakeRandomPointOnSphere(1500.0 + structureItemRef->type().radius());
            pSysEntity->SetPosition(location);
            structureItemRef->SaveItem();
            pSysMgr->AddEntity(pSysEntity);
            pSysEntity = nullptr;
            pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket(structureItemRef);
            continue;
        } else if (categoryID == EVEDB::invCategories::Deployable) {
            // This item is a Deployable item of some kind, so move it from the ship's cargo into space
            // whilst keeping ownership of it to the character not using the corporation the character belongs to:
            cargoItemRef = m_manager->item_factory->GetItem(cur);
            if (!cargoItemRef)
                throw PyException(MakeCustomError("Unable to spawn Deployable item of type %u.", cargoItemRef->typeID()));

            cargoItemRef->Move(pClient->GetLocationID(), flagAutoFit, true);
            //flagUnanchored: for some DUMB reason, this flag, 1023 yields a PyNone when notifications
            // are created inside InventoryItem::Move() from passing it into a PyInt() constructor...WTF?
            pSysEntity = pSysMgr->GetSE(cargoItemRef->itemID());
            location.MakeRandomPointOnSphere(1500.0 + cargoItemRef->type().radius());
            pSysEntity->SetPosition(location);
            cargoItemRef->SaveItem();
            pSysMgr->AddEntity(pSysEntity);
            pSysEntity = nullptr;
            pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket(cargoItemRef);
            continue;
        } //else if ()    // Handle other types of cargo, such as jettisoning assembled ships?

        // TODO Check to see if this item is allowed to be jettisoned based on categoryID and/or groupID:
        // IDEAS:
        // * Modules, Charges, Skillbooks, Ore, Blueprints, Materials, Corpses...

        /** @todo  Handle NON-jettisonable cargo */

        // item isnt structure or deployable and can be jettisoned.  check if container was already created
        if ((!cargoContainerItem) or (!newJetcanItem)) {
            if (!pClient->IsJetcanAvalible()) {
                std::string msg = "A Jettison Container is currently being prepped in your cargo hold. \n";
                msg += "Your estimated wait time is ";
                msg += itoa(pClient->JetcanTime());
                msg += " seconds.";
                pClient->SendNotifyMsg(msg.c_str());
                return nullptr;
            }
            // Spawn jetcan then continue loop
            location.MakeRandomPointOnSphere(500.0);
            ItemData p_idata(
                            23,                         // 23 = cargo container
                            pClient->GetCharacterID(),  //owner is Character
                            pClient->GetLocationID(),
                            flagAutoFit,
                            "Jettisoned Cargo Container",
                            location);

            newJetcanItem = m_manager->item_factory->SpawnCargoContainer(p_idata);
            if (!newJetcanItem)
                throw PyException(MakeCustomError("Unable to spawn item of type %u.", 23));

            ContainerData jetcanData;
                jetcanData.allianceID = pClient->GetAllianceID();
                jetcanData.corporationID = pClient->GetCorporationID();
                jetcanData.factionID = pClient->GetWarFactionID();
                jetcanData.ownerID = pClient->GetCharacterID();
            // create new container
            ContainerSE* cSE = new ContainerSE(newJetcanItem, *m_manager, pSysMgr, jetcanData);
            newJetcanItem->SetMySE(cSE);
            pSysMgr->AddEntity(cSE);
            pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket(newJetcanItem);
            pClient->StartJetcanTimer();
        }
        /** @todo  check current can for capacity limits. */
        //if over limit create new can?  reject remainging cargo?  delete?  crash?  run thru station naked?
        // Move item into cargo Container
        pClient->MoveItem(cur, (cargoContainerItem ? cargoContainerItem->itemID() : newJetcanItem->itemID()), flagAutoFit);
        continue;
    }

    //response should be nodeid and timestamp
    return new PyLong(Win32TimeNow());
}


PyResult ShipBound::Handle_SelfDestruct(PyCallArgs &call) {
    /** @todo finish this later
     * 22:13:29 L ShipBound::Handle_SelfDestruct(): size=1
     * 22:13:29 [SvcCall]   Call Arguments:
     * 22:13:29 [SvcCall]       Tuple: 1 elements
     * 22:13:29 [SvcCall]         [ 0] Integer field: 140000378     <- ship id
     *
  sLog.Log("ShipBound::Handle_SelfDestruct()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);


    [PyTuple 1 items]
      [PyTuple 2 items]
        [PyInt 0]
        [PySubStream 60 bytes]
          [PyTuple 2 items]
            [PyInt 0]
            [PyTuple 2 items]
              [PyInt 1]
              [PyTuple 2 items]
                [PyString "SelfDestructTimer"]
                [PyDict 2 kvp]
                  [PyString "what"]
                  [PyTuple 2 items]
                    [PyInt 4]
                    [PyInt 24700]
                  [PyString "time"]
                  [PyString "2 Minutes"]
    *********  sends msg every 10 sec ************
    [PyTuple 1 items]
      [PyTuple 2 items]
        [PyInt 0]
        [PySubStream 70 bytes]
          [PyTuple 2 items]
            [PyInt 0]
            [PyTuple 2 items]
              [PyInt 1]
              [PyTuple 2 items]
                [PyString "SelfDestructTimer"]
                [PyDict 2 kvp]
                  [PyString "what"]
                  [PyTuple 2 items]
                    [PyInt 4]
                    [PyInt 24700]
                  [PyString "time"]
                  [PyString "1 Minute 49 Seconds"]

    ************  destruct immediate  ****************
    [PyTuple 1 items]
      [PyTuple 2 items]
        [PyInt 0]
        [PySubStream 47 bytes]
          [PyTuple 2 items]
            [PyInt 0]
            [PyTuple 2 items]
              [PyInt 1]
              [PyTuple 2 items]
                [PyString "SelfDestructImmediate"]
                [PyDict 1 kvp]
                  [PyString "what"]
                  [PyTuple 2 items]
                    [PyInt 4]
                    [PyInt 24700]

    ***********  not sure how to kill ship  *************

    Damage fatal_blow((static_cast<SystemEntity*>(who)),true);
    entity->Killed(fatal_blow);

    *************  cancel destruct  *****************
    [PyTuple 3 items]
      [PyInt 6]
      [PyInt 2]
      [PyTuple 1 items]
        [PySubStream 94 bytes]
          [PyObjectEx Normal]
            [PyTuple 3 items]
              [PyToken ccp_exceptions.UserError]
              [PyTuple 2 items]
                [PyString "SelfDestructAborted2"]
                [PyDict 1 kvp]
                  [PyString "when"]
                  [PyInt 83]
              [PyDict 2 kvp]
                [PyString "msg"]
                [PyString "SelfDestructAborted2"]
                [PyString "dict"]
                [PyDict 1 kvp]
                  [PyString "when"]
                  [PyInt 83]

        throw PyException(MakeUserError("SelfDestructAborted2"));

*/

    Client* pClient = call.client;
    PyRep *result = NULL;
    return result;
}

PyResult ShipBound::Handle_GetShipConfiguration(PyCallArgs &call) {
    /*      ******* no packet data ***********
     *
     13:15:58 L ShipBound::Handle_GetShipConfiguration(): size=0
     13:15:58 [SvcCall]   Call Arguments:
     13:15:58 [SvcCall]       Tuple: Empty

     sLog.Log("ShipBound::Handle_GetShipConfiguration()", "size=%u", call.tuple->size());
     call.Dump(SERVICE__CALL_DUMP);
     */

    /*
        self.conf = self.ship.GetShipConfiguration()
        self.sr.smballowfleet.SetChecked(self.conf['allowFleetSMBUsage']) (SMB = ShipMaintenanceBay)
    */

    Client* pClient = call.client;
    PyRep *result = NULL;
    return result;
}