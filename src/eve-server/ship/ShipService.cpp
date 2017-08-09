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
        PyCallable_REG_CALL(ShipBound, ScoopToSMA);
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
    PyCallable_DECL_CALL(ScoopToSMA);
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

    /* return error msg from this call, if applicable */
    //sm.StartService('sessionMgr').PerformSessionChange('board', ship.BoardStoredShip, structureID, shipID)
    //sm.StartService('sessionMgr').PerformSessionChange('storeVessel', ship.StoreVessel, destID)
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
    sLog.White("ShipBound::Handle_Board()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    Call_BoardShip args;
    //     .arg1 (newShipID) -  itemID of the ship to be boarded
    //     .arg2 (oldShipID) -  itemID of the current ship
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    Client* pClient = call.client;
    if (pClient->GetShipSE()->DestinyMgr()->IsMoving()) {
        throw PyException(MakeCustomError("You cannot change ships while moving."));
        return nullptr;
    }

    SystemManager* pSystem = pClient->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return new PyNone();
    }
    GPoint oldPosition(pClient->GetShipSE()->GetPosition());

    // Get ship ItemRefs
    ShipItemRef oldShipRef = pSystem->GetShipFromInventory(args.oldShipID);
    if (oldShipRef.get() == nullptr)
        oldShipRef = pClient->services().item_factory->GetShip(args.oldShipID);
    ShipItemRef newShipRef = pSystem->GetShipFromInventory(args.newShipID);
    if (newShipRef.get() == nullptr)
        newShipRef = pClient->services().item_factory->GetShip(args.newShipID);

    if (newShipRef.get() == nullptr) {
        _log(SHIP__ERROR, "Handle_Board() - Failed to get new ship %u for %s.", args.newShipID, pClient->GetName());
        throw PyException(MakeCustomError("Something bad happened as you prepared to board the ship"));
        return nullptr;
    }

    if (newShipRef->typeID() == itemTypeCapsule) {
        codelog(ITEM__ERROR, "Empty Pod %u in space.  SystemID %u.", args.newShipID, pSystem->GetID());
        throw PyException(MakeCustomError("You already have a pod.  These cannot be boarded manally."));
        return nullptr;
    }

    if (!newShipRef->ValidateBoardShip(newShipRef, pClient->GetChar())) {
        // should we eject player here and deny boarding new ship, or just leave char in current ship and return?
        throw PyException(MakeCustomError("You do not have the skills to fly a %s.", newShipRef->itemName().c_str()));
        return nullptr;
    }

    /* all previous SE and DestinyMgr objects are updated to new ship object here */
    // note:  this isnt right...hackish and funky, but works.
    pClient->BoardShip(newShipRef);

    pClient->SendNotifyMsg("This function is hacked.  You will need to warp, wait a couple minutes, then send '/update' command to correct destiny state (or relog).  This will be fixed eventually");
    
    /* return error msg from this call, if applicable, else nodeid and timestamp */
    return new PyLong(Win32TimeNow());
}

/* only called in space */
PyResult ShipBound::Handle_Eject(PyCallArgs &call) {
    /*if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change already active.");
        return nullptr;
    }*/
    sLog.White("ShipBound::Handle_Eject()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);
    //no arguments.

    Client* pClient = call.client;
    /** @todo create and implement "Weapon Flag"....
     *      Weapon Flag --  the 60-sec timer started upon any offensive weapon activation
     *   this will be in client's criminaltimer object
     *
     * if (pClient->CrimeMgr()->IsWeaponFlagActive())
     *  deny eject
     */

    SystemEntity* pShipSE = pClient->GetShipSE();
    /** @todo  check for active cyno (when we implement it...) and other things that affect eject */
    if (pShipSE->isGlobal()) { /* close enough.  cyno (isGlobal() = true), so this will work */
        /* find proper error msg for this...im sure there is one  */
        call.client->SendNotifyMsg("You cannot eject with an active Cyno Field.");
        return nullptr;
    }

    //  do we need this?
    if (pShipSE->DestinyMgr()->IsMoving()) {
        throw PyException(MakeCustomError("You cannot eject while moving."));
        return nullptr;
    }

    SystemManager* pSystem = pClient->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return new PyNone();
    }

    GPoint oldPosition(pShipSE->GetPosition());
    GPoint capsulePosition(oldPosition);
    capsulePosition.MakeRandomPointOnSphere(pClient->GetShip()->GetAttribute(AttrRadius).get_float() + (MakeRandomFloat(300, 400)));

    // Get ship ItemRefs
    ShipItemRef oldShipRef = pClient->GetShip();
    if (oldShipRef.get() == nullptr)
        oldShipRef = pClient->services().item_factory->GetShip(pClient->GetShipID());
    ShipItemRef capsuleRef = pSystem->GetShipFromInventory(pClient->GetPodID());
    if (capsuleRef.get() == nullptr)
        capsuleRef = pClient->services().item_factory->GetShip(pClient->GetPodID());

    if (capsuleRef.get() == nullptr) {
        _log(SHIP__ERROR, "Handle_Eject() - Failed to get podItem for %s.", pClient->GetName());
        throw PyException(MakeCustomError("Something bad happened as you prepared to eject."));
        return nullptr;
    }

    capsuleRef->Relocate(capsulePosition);

    /* all previous SE and DestinyMgr objects are updated to new ship object here */
    // note:  this isnt right...hackish and funky, but works.
    pClient->BoardShip(capsuleRef);

    pClient->SendNotifyMsg("This function is hacked.  You will need to warp, wait a couple minutes, then send '/update' command to correct destiny state (or relog).  This will be fixed eventually");

    /* return error msg from this call, if applicable, else nodeid and timestamp */
    return new PyLong(Win32TimeNow());
}

// NOTE  LeaveShip and ActivateShip are working.  dont fuck with them
/* only called when docked. */
PyResult ShipBound::Handle_LeaveShip(PyCallArgs &call) {
    /*if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change already active.");
        return nullptr;
    }*/
    sLog.White("ShipBound::Handle_LeaveShip()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    //  sends itemID of ship to leave
    Client* pClient = call.client;
    ShipItemRef shipRef = pClient->SystemMgr()->GetShipFromInventory(arg.arg);
    uint32 podID = pClient->GetPodID();
    ShipItemRef podRef = pClient->SystemMgr()->GetShipFromInventory(podID);
    if (podRef.get() == nullptr)
        podRef = pClient->services().item_factory->GetShip(podID);

    //verify owner (not sure why pod doenst have correct owner...)
    podRef->ChangeOwner(pClient->GetCharacterID(), false);
    //move capsule into the players hangar
    sLog.White("ShipBound::Handle_LeaveShip()", "moving pod %u to station %u", podID, pClient->GetStationID());
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
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    Client* pClient = call.client;
    ShipItemRef oldShipRef = pClient->SystemMgr()->GetShipFromInventory(args.oldShipID);
    if (oldShipRef.get() == nullptr)
        oldShipRef = pClient->services().item_factory->GetShip(args.oldShipID);

    ShipItemRef newShipRef = pClient->SystemMgr()->GetShipFromInventory(args.newShipID);
    if (newShipRef.get() == nullptr)
        newShipRef = pClient->services().item_factory->GetShip(args.newShipID);
    if (newShipRef.get() == nullptr) {
        sLog.Error("ShipBound::Handle_ActivateShip()", "%s: Failed to get new ship %u.", pClient->GetName(), args.newShipID);
        throw PyException(MakeCustomError("Something bad happened as you prepared to board the ship.  Ref: ServerError 15173"));
        return nullptr;
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
    Call_IntBoolArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        /** @todo throw exception */
        return nullptr;
    }

    Client* pClient = call.client;
    ShipItemRef pShip = pClient->GetShip();
    if (pShip.get() == nullptr) {
        sLog.Error("ShipBound::Handle_ActivateShip()", "%s: Failed to get ship item.", pClient->GetName());
        call.client->SendNotifyMsg("Internal Server Error - Ref:?????   -undock failed.");
        return nullptr;
    }

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
                codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
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
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
            return nullptr;
        }
        itemID = args.items.front();
    }

    ShipItemRef ship = m_manager->item_factory->GetShip(itemID);
    if (ship.get() == nullptr) {
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

PyResult ShipBound::Handle_Drop(PyCallArgs &call) {
    // currently, sendstate screws up when POS tower is in bubble with player
    /** @todo  fix state when tower is in bubble */
    call.client->SendNotifyMsg("Launching Items is currently disabled");
    return new PyDict();

    if (IsStation(call.client->GetLocationID())) {
        _log(SERVICE__ERROR, "%s: Trying to drop items when not in space!", call.client->GetName());
        return new PyDict();
    }
    sLog.White("ShipBound::Handle_Drop()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    if (call.tuple->size() != 3) {
        sLog.Error("ShipBound::Handle_Drop()", "call.tuple wrong size, expected 3 items, actual size = %u", call.tuple->size());
        //TODO: throw exception
        return new PyDict();
    }

    Call_Drop3 drop3args;
    if (!drop3args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        //TODO: throw exception
        return new PyDict();
    }

    PyList* PyToDropList = drop3args.toDrop;
    uint32 ownerID = drop3args.ownerID;
    //used for LaunchUpgradePlatformWarning
    bool ignoreWarning = drop3args.ignoreWarning, dropped = false;

    Client* pClient = call.client;
    SystemManager* pSystem = pClient->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return new PyNone();
    }

    FactionData data;
        data.allianceID = pClient->GetAllianceID();
        data.corporationID = pClient->GetCorporationID();
        data.factionID = pClient->GetWarFactionID();
        data.ownerID = ownerID;

    DBSystemDynamicEntity entity;
        entity.itemID = 0;
        entity.itemName = "";
        entity.typeID = 0;
        entity.groupID = 0;
        entity.categoryID = EVEDB::invCategories::_System;
        entity.ownerID = data.ownerID;
        entity.factionID = data.factionID;
        entity.allianceID = data.allianceID;
        entity.corporationID = data.corporationID;
        entity.planetID = 0;
        entity.x = 0.0;
        entity.y = 0.0;
        entity.z = 0.0;

    uint32 itemQuantity = 0;
    double radius = pClient->GetShipSE()->GetRadius();

    GPoint location(pClient->GetShipSE()->GetPosition());

    InventoryItemRef itemRef;

    PyDict* dict = new PyDict();

    for (uint32 i = 0; i < PyToDropList->size(); i++) {
        location.MakeRandomPointOnSphereLayer((300.0 +radius),(800.0 + radius));
        entity.itemID = (uint32)(PyToDropList->items.at(i)->AsTuple()->items.at(0)->AsInt()->value());
        itemQuantity = (uint32)(PyToDropList->items.at(i)->AsTuple()->items.at(1)->AsInt()->value());

        itemRef = m_manager->item_factory->GetItem(entity.itemID);
        if (itemRef.get() == nullptr) {
            sLog.Error("ShipBound::Handle_Drop()", "%s: Unable to find item %u to drop.", pClient->GetName(), entity.itemID);
            continue;
        }

        /**@todo  deal with changing quantities as needed */

        PyList* list = new PyList();
        if ((itemRef->flag() == flagDroneBay) and (itemRef->categoryID() == EVEDB::invCategories::Drone)) {
            // This item is a drone, so launch it into space:
            if (pClient->LaunchDrone(itemRef)) {
                dropped = true;
                list->AddItem(new PyInt(entity.itemID));
            }
        } else {
            //location += itemRef->radius();
            // Move item from cargo bay to space:
            itemRef->Move(pClient->GetLocationID(), flagAutoFit);
            itemRef->Relocate(location);
            itemRef->ChangeOwner(entity.ownerID);

            entity.itemName = itemRef->itemName();
            entity.typeID = itemRef->typeID();
            entity.groupID = itemRef->groupID();
            entity.categoryID = itemRef->categoryID();
            if (entity.groupID == EVEDB::invGroups::Orbital_Infrastructure)
                entity.planetID = pSystem->GetNearestPlanet(location);
            entity.x = itemRef->position().x;
            entity.y = itemRef->position().y;
            entity.z = itemRef->position().z;
            SystemEntity* pSE = DynamicEntityFactory::BuildEntity(*pSystem, m_manager->item_factory, entity);
            if (pSE != nullptr) {
                dropped = true;
                itemRef->SetFlag(flagStructureInactive);
                list->AddItem(new PyInt(entity.itemID));
                pSystem->AddEntity(pSE);
                if (pSE->IsPOSSE())
                    pSE->GetPOSSE()->InitData();
            }
        }
        if (dropped) {
            itemRef->ChangeSingleton(true);
            dict->SetItem(new PyInt(entity.itemID), list);
        }
    }

    if (dropped)
        pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket();
    else
        dict->clear();

    return dict;
}

PyResult ShipBound::Handle_Scoop(PyCallArgs &call) {
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        //TODO: throw exception
        return nullptr;
    }

    Client* pClient = call.client;
    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return new PyNone();
    }
    SystemEntity* pSE = pSystem->GetSE(arg.arg);
    if (pSE == nullptr) {
        _log(SERVICE__ERROR, "%s: Unable to find object %u to scoop.", pClient->GetName(), arg.arg);
        return nullptr;
    }

    InventoryItemRef iRef = pSE->GetSelf();
    if (iRef.get() == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return new PyNone();
    }

    /** @todo check ownership of this object, ie does this character/corporation own this object? */
    // do we really need to do this for anything except for drones that are under control of another player?

    /** @todo  check to see if this object is anchored and if so, refuse to scoop it */

    // Check cargo bay capacity:
    double capacity = pClient->GetShip()->GetMyInventory()->GetCapacity(flagCargoHold);
    double volume = iRef->GetAttribute(AttrVolume).get_float();
    if (capacity < volume)
        throw PyException(MakeCustomError("%s is too large to fit in remaining Cargo bay capacity.", iRef->itemName().c_str()));
    else {
        // We have enough Cargo bay capacity to hold the item being scooped,
        // so take ownership of it and move it into the cargo bay:
        iRef->ChangeOwner(pClient->GetCharacterID(), true);
        pClient->MoveItem(iRef->itemID(), pClient->GetShipID(), flagCargoHold);
        pSystem->RemoveEntity(pSE);
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
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
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
        double capacity = pClient->GetShip()->GetMyInventory()->GetCapacity(flagDroneBay);
        double volume = item->GetAttribute(AttrVolume).get_float();
        if (capacity < volume)
            throw PyException(MakeCustomError("%s is too large to fit in remaining Drone bay capacity.", item->itemName().c_str()));
        else {
            // We have enough Drone bay capacity to hold the drone,
            // so take ownership of it and move it into the Drone bay:
            item->ChangeOwner(pClient->GetCharacterID(), true);
            pClient->MoveItem(item->itemID(), pClient->GetShipID(), flagDroneBay);
            pSysMgr->RemoveEntity(pDroneSE);
        }
    }

    return nullptr;
}

PyResult ShipBound::Handle_ScoopToSMA(PyCallArgs &call) {
    /*      ******* no packet data ***********     */

    sLog.White("ShipBound::Handle_ScoopToSMA()", "size=%u", call.tuple->size());
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
}

PyResult ShipBound::Handle_Jettison(PyCallArgs &call) {
    Call_SingleIntList args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        //TODO: throw exception
        return nullptr;
    }

    Client* pClient = call.client;
    if (!pClient->IsInSpace()) {
        _log(SERVICE__ERROR, "%s: Trying to jettison items when not in space!", pClient->GetName());
        return nullptr;
    }

    SystemManager* pSysMgr = pClient->SystemMgr();
    //Get location of our ship
    GPoint location(pClient->GetShipSE()->GetPosition());

    InventoryItemRef cargoItemRef, invItemRef;
    CargoContainerRef newJetcanItem, cargoContainerItem;
    StructureItemRef structureItemRef;
    uint32 groupID = 0, categoryID = 0;

    FactionData data;
        data.allianceID = pClient->GetAllianceID();
        data.corporationID = pClient->GetCorporationID();
        data.factionID = pClient->GetWarFactionID();
        /** @todo  determine if this is char or corp here */
        data.ownerID = pClient->GetCharacterID();

    //args contains id's of items to jettison
    std::vector<int32>::iterator cur = args.ints.begin();
    // loop thru items to see if there is a container in this list.
    for (; cur != args.ints.end(); cur++) {
        invItemRef = m_manager->item_factory->GetItem(*cur);
        if (invItemRef.get() == nullptr)
            continue;
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

            cargoContainerItem->Move(pClient->GetLocationID(), flagAutoFit, true);
            ContainerSE* cSE = new ContainerSE(cargoContainerItem, *m_manager, pSysMgr, data);
            location.MakeRandomPointOnSphere(500.0);
            cSE->SetPosition(location);
            cargoContainerItem->SaveItem();
            pSysMgr->AddEntity(cSE);

            // container found.  remove this item from list, then break out of here and use to contain all other non-pos items
            args.ints.erase(cur);
            cur = args.ints.end();
        }
    }

    // reset iterator and loop thru list.
    for (auto cur : args.ints) {
        invItemRef = m_manager->item_factory->GetItem(cur);
        if (invItemRef.get() == nullptr)
            continue;
        categoryID = invItemRef->categoryID();

        if ((categoryID == EVEDB::invCategories::Structure)
            or (categoryID == EVEDB::invCategories::Orbitals)) {
            structureItemRef = m_manager->item_factory->GetStructure(cur);
            if (structureItemRef.get() == nullptr)
                throw PyException(MakeCustomError("Unable to spawn Structure item of type %u.", structureItemRef->typeID()));

            structureItemRef->Move(pClient->GetLocationID(), flagAutoFit, true);
            StructureSE* sSE = new StructureSE(structureItemRef, *m_manager, pSysMgr, data);
            location.MakeRandomPointOnSphere(1500.0 + structureItemRef->type().radius());
            sSE->SetPosition(location);
            structureItemRef->SaveItem();
            pSysMgr->AddEntity(sSE);
            continue;
        } else if (categoryID == EVEDB::invCategories::Deployable) {
            cargoItemRef = m_manager->item_factory->GetItem(cur);
            if (cargoItemRef.get() == nullptr)
                throw PyException(MakeCustomError("Unable to spawn Deployable item of type %u.", cargoItemRef->typeID()));

            cargoItemRef->Move(pClient->GetLocationID(), flagAutoFit, true);
            //flagUnanchored: for some DUMB reason, this flag, 1023 yields a PyNone when notifications
            // are created inside InventoryItem::Move() from passing it into a PyInt() constructor...WTF?
            DeployableSE* dSE = new DeployableSE(cargoItemRef, *m_manager, pSysMgr, data);
            location.MakeRandomPointOnSphere(1500.0 + cargoItemRef->type().radius());
            dSE->SetPosition(location);
            cargoItemRef->SaveItem();
            pSysMgr->AddEntity(dSE);
            continue;
        } //else if ()
        /** @todo  Handle other cargo */

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
            if (newJetcanItem.get() == nullptr)
                throw PyException(MakeCustomError("Unable to spawn item of type %u.", 23));
            // create new container
            ContainerSE* cSE = new ContainerSE(newJetcanItem, *m_manager, pSysMgr, data);

            newJetcanItem->SetMySE(cSE);
            pSysMgr->AddEntity(cSE);
            pClient->StartJetcanTimer();
        }
        /** @todo  check current can for capacity limits. */
        //if over limit create new can?  reject remainging cargo?  delete?  crash?  run thru station naked?
        pClient->MoveItem(cur, (cargoContainerItem ? cargoContainerItem->itemID() : newJetcanItem->itemID()), flagAutoFit);
        continue;
    }

    pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket();

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
  sLog.White("ShipBound::Handle_SelfDestruct()", "size=%u", call.tuple->size());
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
                  //if (mySE->HasPilot() and mySE->GetPilot()->CanThrow())
        throw PyException(MakeUserError("SelfDestructAborted2"));
*/

    /* return error msg from this call, if applicable, else nodeid and timestamp */
    return new PyLong(Win32TimeNow());
}

PyResult ShipBound::Handle_GetShipConfiguration(PyCallArgs &call) {
    /*      ******* no packet data ***********
     *
     13:15:58 L ShipBound::Handle_GetShipConfiguration(): size=0
     13:15:58 [SvcCall]   Call Arguments:
     13:15:58 [SvcCall]       Tuple: Empty
     sLog.White("ShipBound::Handle_GetShipConfiguration()", "size=%u", call.tuple->size());
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
