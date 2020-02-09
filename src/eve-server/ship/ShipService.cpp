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
    Rewrite:    Allan
*/

#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"

#include "EVEServerConfig.h"
#include "npc/Drone.h"
#include "planet/CustomsOffice.h"
#include "planet/Moon.h"
#include "pos/Structure.h"
#include "ship/ShipService.h"
#include "system/Container.h"
#include "system/DestinyManager.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/AnomalyMgr.h"


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
        PyCallable_REG_CALL(ShipBound, LaunchFromContainer);
        PyCallable_REG_CALL(ShipBound, Jettison);
        PyCallable_REG_CALL(ShipBound, GetShipConfiguration);
        PyCallable_REG_CALL(ShipBound, SelfDestruct);

        PyCallable_REG_CALL(ShipBound, BoardStoredShip);
        PyCallable_REG_CALL(ShipBound, StoreVessel);
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
    PyCallable_DECL_CALL(LaunchFromContainer);
    PyCallable_DECL_CALL(Jettison);
    PyCallable_DECL_CALL(GetShipConfiguration);
    PyCallable_DECL_CALL(SelfDestruct);

    PyCallable_DECL_CALL(BoardStoredShip);
    PyCallable_DECL_CALL(StoreVessel);

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

PyBoundObject *ShipService::CreateBoundObject(Client *pClient, const PyRep *bind_args) {
    _log(CLIENT__MESSAGE, "ShipService bind request");
    bind_args->Dump(CLIENT__MESSAGE, "    ");
    return new ShipBound(m_manager, m_db);
}

/* only called in space */
PyResult ShipBound::Handle_Board(PyCallArgs &call) {
    if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change already active.");
        return nullptr;
    }

    Call_BoardShip args;
    //     .arg1 (newShipID) -  itemID of the ship to be boarded
    //     .arg2 (oldShipID) -  itemID of the current ship
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    Client* pClient = call.client;

    Ship* pShipSE =  pClient->GetShipSE();
    if (pShipSE == nullptr)
        throw PyException(MakeCustomError("Invalid Ship.  Ref: ServerError xxxxx"));
    /** @todo  check for active cyno (when we implement it...) and other things that affect eject */
    if (pShipSE->isGlobal()) { /* close enough.  cyno (isGlobal() = true), so this will work */
        /* find proper error msg for this...im sure there is one  */
        throw PyException(MakeCustomError("You cannot eject current ship with an active Cyno Field."));
    }

    //  do we need this? yes....this needs more work in destiny to implement correctly
    if (pShipSE->DestinyMgr()->GetSpeed() > 20)
        throw PyException(MakeCustomError("You cannot eject current ship while moving faster than 20m/s. Ref: ServerError 05139."));

    SystemManager* pSystem = pClient->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return nullptr;
    }

    // this will segfault if newShipID is invalid or not in system inventory
    pShipSE = pSystem->GetSE(args.newShipID)->GetShipSE();

    if (pShipSE == nullptr) {
        _log(SHIP__ERROR, "Handle_Board() - Failed to get new ship %u for %s.", args.newShipID, pClient->GetName());
        throw PyException(MakeCustomError("Something bad happened as you prepared to board the ship.  Ref: ServerError 25107."));
    }

    if (pShipSE->GetTypeID() == itemTypeCapsule) {
        codelog(ITEM__ERROR, "Empty Pod %u in space.  SystemID %u.", args.newShipID, pSystem->GetID());
        throw PyException(MakeCustomError("You already have a pod.  These cannot be boarded manally."));
    }

    //CantBoardTargeted

    //  do we need this? yes....this needs more work in destiny to implement correctly
    if (pShipSE->DestinyMgr()->GetSpeed() > 20)
        throw PyException(MakeCustomError("You cannot board the ship while it's moving faster than 20m/s. Ref: ServerError 05139."));

    // should we eject player here and deny boarding new ship, or just leave char in current ship and return?
    if (!pShipSE->GetShipItemRef()->ValidateBoardShip(pClient->GetChar()))
        throw PyException(MakeCustomError("You do not have the skills to fly a %s.", pShipSE->GetName()));

    float distance = pClient->GetShipSE()->GetPosition().distance(pShipSE->GetPosition());
    // fudge for radii ?
    if (distance > sConfig.world.shipBoardDistance)
        throw PyException(MakeCustomError("You are too far from %s to board it.<br>You must be within %u meters to board this ship.",\
                pShipSE->GetName(), sConfig.world.shipBoardDistance));

    pClient->Board(pShipSE);

    /* return error msg from this call, if applicable (not sure how yet), else nodeid and timestamp */
    // returns nodeID and timestamp
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
        tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    return tuple;
}

/* only called in space */
PyResult ShipBound::Handle_Eject(PyCallArgs &call) {
    if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change already active.");
        return nullptr;
    }

    //no arguments.
    Client* pClient = call.client;
    /** @todo create and implement "Weapon Flag"....
     *      Weapon Flag --  the 60-sec timer started upon any offensive weapon activation
     *   this will be in client's criminaltimer object
     *
     * if (pClient->CrimeMgr()->IsWeaponFlagActive())
     *  deny eject
     */
    //{'FullPath': u'UI/Messages', 'messageID': 256625, 'label': u'NoEjectingToSpaceInStationBody'}(u"You can't eject into space while you're docked. Try leaving your ship the usual way.", None, None)

    SystemEntity* pShipSE = pClient->GetShipSE();
    if (pShipSE == nullptr)
        throw PyException(MakeCustomError("Invalid Ship.  Ref: ServerError xxxxx"));
    /** @todo  check for active cyno (when we implement it...) and other things that affect eject */
    if (pShipSE->isGlobal()) { /* close enough.  cyno (isGlobal() = true), so this will work */
        /* find proper error msg for this...im sure there is one  */
        throw PyException(MakeCustomError("You cannot eject with an active Cyno Field."));
    }

    //  do we need this? yes....this needs more work in destiny to implement correctly
    if (pShipSE->DestinyMgr()->GetSpeed() > 20)
        throw PyException(MakeCustomError("You cannot eject current ship while moving faster than 20m/s. Ref: ServerError 05139."));

    pClient->Eject();

    /* return error msg from this call, if applicable (not sure how yet), else nodeid and timestamp */
    // returns nodeID and timestamp
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
        tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    return tuple;
}

// NOTE  LeaveShip and ActivateShip are working.  dont fuck with them
/* only called when docked. */
PyResult ShipBound::Handle_LeaveShip(PyCallArgs &call)
{
    if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change already active.");
        return nullptr;
    }

    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    //  sends itemID of ship to leave
    Client* pClient = call.client;
    uint32 podID = pClient->GetPodID();
    ShipItemRef podRef = pClient->SystemMgr()->GetShipFromInventory(podID);
    if (podRef.get() == nullptr)
        podRef = sItemFactory.GetShip(podID);

    //verify owner (not sure why pod doesnt have correct owner...)
    podRef->ChangeOwner(pClient->GetCharacterID(), false);
    //move capsule into the players hangar
    podRef->Move(pClient->GetStationID(), flagHangar, true);

    // capsuleID = shipsvc.LeaveShip(shipid)
    return new PyInt(podID);
}

/* only called when docked. */
PyResult ShipBound::Handle_ActivateShip(PyCallArgs &call) {
    //self.instanceCache, self.instanceFlagQuantityCache, self.wbData = self.remoteShipMgr.ActivateShip(shipID, oldShipID)

    if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change already active.");
        return nullptr;
    }
    Call_BoardShip args;
    //     .arg1 (newShipID) -  itemID of the ship to be boarded
    //     .arg2 (oldShipID) -  itemID of the current ship
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    Client* pClient = call.client;
    ShipItemRef newShipRef = sItemFactory.GetShip(args.newShipID);
    if (newShipRef.get() == nullptr) {
        sLog.Error("ShipBound::Handle_ActivateShip()", "%s: Failed to get new ship %u.", pClient->GetName(), args.newShipID);
        throw PyException(MakeCustomError("Something bad happened as you prepared to board the ship.  Ref: ServerError 15173+1"));
    }
    //ShipMustBeInPersonalHangar

    pClient->BoardShip(newShipRef);

    // response should return ship modules, loaded charges, and linked weapons
    PyTuple* rsp = new PyTuple(3);
        rsp->SetItem(0, newShipRef->GetShipState());    //dict of ship modules
        rsp->SetItem(1, newShipRef->GetChargeState());    //dict of ship charges
        rsp->SetItem(2, newShipRef->GetLinkedWeapons()); // dict of linked modules
    if (is_log_enabled(CLIENT__INFO))
        rsp->Dump(CLIENT__INFO, "    ");
    return rsp;
}

PyResult ShipBound::Handle_Undock(PyCallArgs &call) {

    //ShipIllegalTypeUndock

    /*  we could have some fun with these....
     * (258696, `Undock Delayed`)
     * (258702, `Undock Prohibited`)
     * (258700, `Undock Delayed`)
     * (258703, `Officials have closed the undocking ramps in {system} due to heavy congestion. Please try again later.`)
     * (258697, `{system} Traffic Control is currently offline and unable to process your undocking request. Please try again in a moment.`)
     */

    Call_IntBoolArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        throw PyException(MakeCustomError("Something bad happened as you prepared to board the ship.  Ref: ServerError 15173"));
    }

    Client* pClient = call.client;
    ShipItemRef pShip = pClient->GetShip();
    if (pShip.get() == nullptr) {
        sLog.Error("ShipBound::Handle_ActivateShip()", "%s: Failed to get ship item.", pClient->GetName());
        throw PyException(MakeCustomError("Something bad happened as you prepared to board the ship.  Ref: ServerError 15173"));
        call.client->SendNotifyMsg("Internal Server Error - Ref: ServerError xxxxx   -undock failed.");
        return nullptr;
    }

    // nowhere near implementing this one yet....
    bool ignoreContraband = args.arg2;

    //  get vector of online modules as (k,v) pair,
    //    where key is slotID, value is moduleID
    if (call.byname.find("onlineModules") != call.byname.end()) {
        PyDict* onlineModules = call.byname["onlineModules"]->AsDict();
        if (is_log_enabled(MODULE__INFO)) {
            _log(MODULE__INFO, "Dumping 'onlineModules' List");
            onlineModules->Dump(MODULE__INFO, "   ");
        }
        PyDict::const_iterator cur = onlineModules->begin();
        for (; cur != onlineModules->end(); ++cur)
            pShip->AddModuleToOnlineVec(cur->second->AsInt()->value());
    }

    pClient->UndockFromStation();

    // returns nodeID and timestamp
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
        tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    return tuple;
}

PyResult ShipBound::Handle_Drop(PyCallArgs &call)
{
    _log(SHIP__INFO, "ShipBound::Handle_Drop()");
    call.Dump(SHIP__INFO);

    if (IsStation(call.client->GetLocationID())) {
        _log(SERVICE__ERROR, "%s: Trying to drop items when not in space!", call.client->GetName());
        return nullptr;
    }

    Call_Drop3 args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    PyList* PyToDropList = args.toDrop;
    uint32 ownerID = args.ownerID;  // not sent for LaunchDrone() command  (not needed)
    //used for LaunchUpgradePlatformWarning
    // args.ignoreWarning

    bool dropped = false, shipDrop = false;

    Client* pClient = call.client;
    SystemManager* pSystem = pClient->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return nullptr;
    }

    uint8 qty = 0;
    uint32 itemID = 0;
    double radius = pClient->GetShipSE()->GetRadius();

    InventoryItemRef iRef(nullptr);
    PyDict* dict = new PyDict();
    for (uint32 i = 0; i < PyToDropList->size(); ++i) {
        dropped = false;
        PyList* list = new PyList();
        GPoint location(pClient->GetShipSE()->GetPosition());
        location.MakeRandomPointOnSphereLayer(500,1500);
        qty = PyToDropList->items.at(i)->AsTuple()->items.at(1)->AsInt()->value();
        itemID = PyToDropList->items.at(i)->AsTuple()->items.at(0)->AsInt()->value();
        iRef = sItemFactory.GetItem(itemID);
        if (iRef.get() == nullptr) {
            sLog.Error("ShipBound::Handle_Drop()", "%s: Unable to find item %u to drop.", pClient->GetName(), itemID);
            continue;
        }

        switch (iRef->categoryID()) {
            case EVEDB::invCategories::Drone: {
                if (!sConfig.testing.EnableDrones) {
                    throw PyException(MakeCustomError("Drones are disabled."));
                }
                if (pClient->GetChar()->GetAttribute(AttrMaxActiveDrones).get_uint32() < 1) {
                    std::map<std::string, PyRep *> arg;
                    arg["typeID"] = new PyInt(iRef->typeID());
                    throw PyException(MakeUserError("NoDroneManagementAbilities", arg));
                    //{'FullPath': u'UI/Messages', 'messageID': 259203, 'label': u'NoDroneManagementAbilitiesBody'}(u'You cannot launch {[item]typeID.nameWithArticle} because you do not have the ability to control any drones.', None, {u'{[item]typeID.nameWithArticle}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'nameWithArticle', 'args': 0, 'kwargs': {}, 'variableName': 'typeID'}})
                }
                if (iRef->flag() != flagDroneBay) {
                    std::map<std::string, PyRep *> arg;
                    arg["item"] = new PyInt(iRef->typeID());
                    throw PyException(MakeUserError("DropItemNotInDroneBay", arg));
                    // {'FullPath': u'UI/Messages', 'messageID': 259680, 'label': u'DropItemNotInDroneBayBody'}(u'{[item]item.name} cannot be dropped because it is not in your drone bay.', None, {u'{[item]item.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'item'}})
                }

                /** @todo check char skills to launch this drone */

                // This item is a drone, so launch it into space:
                if (qty > 1) {
                    for (uint8 i = 0; i < qty; ++i) {
                        InventoryItemRef newItem = iRef->Split(1);
                        if (newItem.get() == nullptr) {
                            _log(INV__ERROR, "ShipBound::Handle_Drop() - Error splitting item %u. Skipping.", iRef->itemID());
                            continue;
                        }
                        if (newItem->quantity() > 1)
                            _log(INV__ERROR, "ShipBound::Handle_Drop() - Split item %u qty > 1 (%u).  Continuing.", newItem->itemID(), newItem->quantity());

                        if (pClient->GetShipSE()->LaunchDrone(newItem)) {
                            dropped = true;
                            shipDrop = true;
                            list->AddItem(new PyInt(newItem->itemID()));
                        }
                    }
                } else {
                    if (pClient->GetShipSE()->LaunchDrone(iRef)) {
                        dropped = true;
                        shipDrop = true;
                        list->AddItem(new PyInt(iRef->itemID()));
                    }
                }
            } break;
            case EVEDB::invCategories::Structure: {
                if (pClient->SystemMgr()->GetClosestMoonSE(location)->GetMoonSE()->HasTower()) {
                    pClient->SendErrorMsg("This Moon already has a Control Tower in orbit.  Aborting Drop.");
                    return nullptr;
                }

                //{'FullPath': u'UI/Messages', 'messageID': 259679, 'label': u'DropNeedsPlayerCorpBody'}(u'In order to launch {[item]item.name} you need to be a member of a independent corporation.', None, {u'{[item]item.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'item'}})
                //{'FullPath': u'UI/Messages', 'messageID': 256411, 'label': u'CantInHighSecSpaceBody'}(u'You cannot do that as CONCORD currently restricts the launching, anchoring and control of that type of structure within CONCORD-protected space to authorized agents of the empires.', None, None)

                DBSystemDynamicEntity entity = DBSystemDynamicEntity();
                entity.ownerID = ownerID;
                entity.factionID = pClient->GetWarFactionID();
                entity.allianceID = pClient->GetAllianceID();
                entity.corporationID = pClient->GetCorporationID();
                entity.itemID = iRef->itemID();
                entity.itemName = iRef->itemName();
                entity.typeID = iRef->typeID();
                entity.groupID = iRef->groupID();
                entity.categoryID = iRef->categoryID();

                // Move item from cargo bay to space: (and send OnItemChange packet)
                iRef->Move(pClient->GetLocationID(), flagAutoFit, true);
                iRef->SetPosition(location + iRef->radius() + radius);
                iRef->ChangeOwner(entity.ownerID);

                entity.x = iRef->position().x;
                entity.y = iRef->position().y;
                entity.z = iRef->position().z;

                if (entity.groupID == EVEDB::invGroups::Orbital_Infrastructure)
                    entity.planetID = pSystem->GetClosestPlanetID(location);
                SystemEntity* pSE = DynamicEntityFactory::BuildEntity(*pSystem, entity);
                if (pSE == nullptr) {
                    //couldnt create entity.  move item back to orig location and continue
                    iRef->Donate(pClient->GetCharacterID(), pClient->GetShipID(), flagCargoHold);
                    continue;
                }

                // item was successfully created.  set singleton
                iRef->ChangeSingleton(true);

                dropped = true;
                shipDrop = true;
                pSE->GetPOSSE()->Drop(pClient->GetShipSE()->SysBubble());
                pSystem->AddEntity(pSE);
                list->AddItem(new PyInt(entity.itemID));
                // may need separate test for infrastructure hubs
            } break;
            default: {
                _log(INV__ERROR, "ShipBound::Handle_Drop() - Item %s (cat %u) is neither drone nor structure.", iRef->name(), iRef->categoryID());
            }
        }

    // returns dict of dropped items as {itemID, data} where data is list of itemIDs dropped (split stack if applicable)
    // however, on non-throw error, data is tuple of errID, errDetailsType, errDetails (unknown where these are defined)

        if (dropped) {
            dict->SetItem(new PyInt(iRef->itemID()), list);
        } else {
            PyTuple* err = new PyTuple(3);
            err->SetItem(0, new PyInt(1));
            err->SetItem(1, new PyString("unsure"));
            err->SetItem(2, new PyString("misc error"));
            dict->SetItem(new PyInt(iRef->itemID()), err);
        }
    }

    //  missing launch error throw msgs.... LaunchCPWarning, LaunchUpgradePlatformWarning
    //   - these are thrown before item is launched and will negate entire launch group until warning is approved, then entire group is reprocessed

    if (shipDrop)
        pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket();

    return dict;
}

PyResult ShipBound::Handle_Scoop(PyCallArgs &call) {
    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        //TODO: throw exception
        return nullptr;
    }

    Client* pClient(call.client);
    SystemManager* pSysMgr(pClient->SystemMgr());
    if ( pSysMgr == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager.", pClient->GetName());
        return PyStatic.NewNone();
    }
    SystemEntity* pSE = pSysMgr->GetSE(arg.arg);
    if (pSE == nullptr) {
        _log(SERVICE__ERROR, "%s: Unable to find object %u to scoop.", pClient->GetName(), arg.arg);
        return PyStatic.NewNone();
        //{'FullPath': u'UI/Messages', 'messageID': 258825, 'label': u'ScoopObjectGoneBody'}(u'{target} is no longer there.', None, {u'{target}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'target'}})
    }

    // check to see if this object is anchored and if so, refuse to scoop it
    if (pSE->IsContainerSE())
        if (pSE->GetContSE()->IsAnchored())
            throw PyException(MakeCustomError("%s is anchored.  Cannot scoop.", pSE->GetName()));

    /** @todo check ownership of this object, ie does this character/corporation own this object? */
    // check drones for other pilots control
    if (pSE->IsDroneSE())
        if (pSE->GetDroneSE()->IsEnabled())
            if (pSE->GetDroneSE()->GetOwner() != pClient)
                throw PyException(MakeCustomError("%s is under another pilot's control.  Cannot scoop.", pSE->GetName()));

    InventoryItemRef iRef = pSE->GetSelf();
    if (iRef.get() == nullptr) {
        codelog(CLIENT__ERROR, "ItemRef for %s not found.", arg.arg);
        return PyStatic.NewNone();
    }

    //AttrDroneBaySlotsLeft
    // Check cargo bay capacity:
    double capacity = pClient->GetShip()->GetMyInventory()->GetCapacity(flagCargoHold);
    double volume = iRef->GetAttribute(AttrVolume).get_float();
    if (capacity < volume)
        throw PyException(MakeCustomError("%s is too large to fit in remaining Cargo bay capacity.", iRef->name()));
    else {
        // We have enough Cargo bay capacity to hold the item being scooped,
        // so take ownership of it and move it into the cargo bay:
        iRef->ChangeOwner(pClient->GetCharacterID(), true);
        pClient->MoveItem(iRef->itemID(), pClient->GetShipID(), flagCargoHold);
        pSysMgr->RemoveEntity(pSE);
        // perform data cleanup for structures
        if (pSE->IsPOSSE())
            pSE->GetPOSSE()->Scoop();
        // perform data cleanup for drones
        if (pSE->IsDroneSE())
            pClient->GetShipSE()->ScoopDrone(pSE);
        // delete SE since item is no longer in space
        SafeDelete(pSE);
    }

    return PyStatic.NewNone();
}

PyResult ShipBound::Handle_ScoopDrone(PyCallArgs &call) {
    Call_SingleIntList args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }
    // per patch notes, if ship is too far to scoop, it will automagically travel closer till drone is within range, then scoop and stop

    Client* pClient(call.client);
    SystemManager* pSysMgr(pClient->SystemMgr());
    std::vector<int32>::const_iterator cur = args.ints.begin();
    for(; cur != args.ints.end(); cur++) {
        SystemEntity* pDroneSE = pSysMgr->GetSE(*cur);
        if (pDroneSE == nullptr) {
            _log(SERVICE__ERROR, "%s: Unable to find drone %u to scoop.", pClient->GetName(), *cur);
            continue;
        }

        InventoryItemRef iRef(pDroneSE->GetSelf());

        // Check to see that this is really a drone:
        pClient->GetShip()->ValidateAddItem(flagDroneBay, iRef);

        /** @todo check ownership/control. */

        // Check drone bay capacity:
        double capacity = pClient->GetShip()->GetMyInventory()->GetCapacity(flagDroneBay);
        double volume = iRef->GetAttribute(AttrVolume).get_float();
        if (capacity < volume)
            throw PyException(MakeCustomError("%s is too large to fit in Drone bay remaining space.", iRef->name()));
        //{'FullPath': u'UI/Messages', 'messageID': 259191, 'label': u'NotEnoughDroneBaySpaceBody'}(u'{[numeric]volume} units would be required to complete this operation. Destination container only has {[numeric]available} units available.', None, {u'{[numeric]volume}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'volume'}, u'{[numeric]available}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'available'}})
        //{'FullPath': u'UI/Messages', 'messageID': 259210, 'label': u'NotEnoughDroneBaySpaceOverloadBody'}(u'The drone bay is overloaded and cannot be made to fit {item}. It is currently only capable of fitting {maximum} units and it is currently jammed full with {used} units.', None, {u'{maximum}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'maximum'}, u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{used}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'used'}})
        //{'FullPath': u'UI/Messages', 'messageID': 258737, 'label': u'ShipDronesOverloadedBody'}(u'You cannot do that because you have somehow overloaded your drone bay.', None, None)
        //{'FullPath': u'UI/Messages', 'messageID': 258904, 'label': u'ShipDronesOverloaded2Body'}(u'You cannot do that because you have somehow overloaded your drone bay. It fits {[numeric]canHave, decimalPlaces=2} units but somehow {[numeric]have, decimalPlaces=2} has been squeezed in.', None, {u'{[numeric]have, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'have'}, u'{[numeric]canHave, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'canHave'}})
        //{'FullPath': u'UI/Messages', 'messageID': 258499, 'label': u'CannotMixDronesBody'}(u'You can not mix different types of drones in the same group', None, None)

        else {
            // We have enough Drone bay capacity to hold the drone,
            // so take ownership of it and move it into the Drone bay:
            iRef->ChangeOwner(pClient->GetCharacterID(), true);
            pClient->MoveItem(iRef->itemID(), pClient->GetShipID(), flagDroneBay);
            pClient->GetShipSE()->ScoopDrone(pDroneSE);
            pSysMgr->RemoveEntity(pDroneSE);
            SafeDelete(pDroneSE);
        }
    }

    // returns error on error else none
    return PyStatic.NewNone();
}

PyResult ShipBound::Handle_Jettison(PyCallArgs &call) {
    Call_SingleIntList args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
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

    InventoryItemRef cRef, iRef;
    CargoContainerRef jcRef, ccRef;
    StructureItemRef sRef;
    uint32 groupID = 0, categoryID = 0;

    /** @todo  deal with launching items for corp... they will use flagProperty */

    FactionData data = FactionData();
        data.allianceID = pClient->GetAllianceID();
        data.corporationID = pClient->GetCorporationID();
        data.factionID = pClient->GetWarFactionID();
        /** @todo  determine if this is char or corp here */
        data.ownerID = pClient->GetCharacterID();

    // returns nodeID and timestamp
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
        tuple->SetItem(1, new PyLong(GetFileTimeNow()));

    //args contains id's of items to jettison
    std::vector<int32>::iterator cur = args.ints.begin();
    // loop thru items to see if there is a container in this list.
    for (; cur != args.ints.end(); ++cur) {
        // running this list twice is fuckedup, but not sure of another way to determine if container is in jettison list.
        iRef = sItemFactory.GetItem(*cur);
        if (iRef.get() == nullptr)
            continue;
        groupID = iRef->groupID();

        if ((groupID == EVEDB::invGroups::Audit_Log_Secure_Container)
        or  (groupID == EVEDB::invGroups::Secure_Cargo_Container)
        or  (groupID == EVEDB::invGroups::Freight_Container)) {
            /** @todo (allan)  check these for accuracy  */
            /** @todo (allan)  *****  there are stipulations on placement of these items.  *****  */
            ccRef = sItemFactory.GetCargoContainer(*cur);
            if (ccRef.get() == nullptr)
                throw PyException(MakeCustomError("Unable to spawn item of type %u.", ccRef->typeID()));

            ccRef->Move(pClient->GetLocationID(), flagAutoFit, true);
            ContainerSE* cSE = new ContainerSE(ccRef, *m_manager, pSysMgr, data);
            location.MakeRandomPointOnSphere(500.0);
            cSE->SetPosition(location);
            ccRef->SaveItem();
            pSysMgr->AddEntity(cSE);
            pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket();

            // container found.  remove this item from list, then break out of here and use to contain all other non-pos items
            args.ints.erase(cur);
            cur = args.ints.end();
        }
    }

    // container check complete, loop thru list for other items
    for (auto cur : args.ints) {
        iRef = sItemFactory.GetItem(cur);
        if (iRef.get() == nullptr)
            continue;
        categoryID = iRef->categoryID();

        if ((categoryID == EVEDB::invCategories::Structure)
        or  (categoryID == EVEDB::invCategories::SovereigntyStructure)
        or  (categoryID == EVEDB::invCategories::StructureUpgrade)) {
            sRef = sItemFactory.GetStructure(cur);
            if (sRef.get() == nullptr)
                throw PyException(MakeCustomError("Unable to spawn Structure item of type %u.", sRef->typeID()));

            sRef->Move(pClient->GetLocationID(), flagAutoFit, true);
            StructureSE* sSE = new StructureSE(sRef, *m_manager, pSysMgr, data);
            location.MakeRandomPointOnSphere(1500.0 + sRef->type().radius());
            sSE->SetPosition(location);
            sRef->SaveItem();
            pSysMgr->AddEntity(sSE);
            pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket();
            continue;
        } else if (categoryID == EVEDB::invCategories::Orbitals) {
            sRef = sItemFactory.GetStructure(cur);
            if (sRef.get() == nullptr)
                throw PyException(MakeCustomError("Unable to spawn Structure item of type %u.", sRef->typeID()));

            sRef->Move(pClient->GetLocationID(), flagAutoFit, true);
            CustomsSE* sSE = new CustomsSE(sRef, *m_manager, pSysMgr, data);
            location.MakeRandomPointOnSphere(1500.0 + sRef->type().radius());
            sSE->SetPosition(location);
            sRef->SaveItem();
            pSysMgr->AddEntity(sSE);
            pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket();
            continue;
        } else if (categoryID == EVEDB::invCategories::Deployable) {
            cRef = sItemFactory.GetItem(cur);
            if (cRef.get() == nullptr)
                throw PyException(MakeCustomError("Unable to spawn Deployable item of type %u.", cRef->typeID()));

            cRef->Move(pClient->GetLocationID(), flagAutoFit, true);
            //flagUnanchored: for some DUMB reason, this flag, 1023 yields a PyNone when notifications
            // are created inside InventoryItem::Move() from passing it into a PyInt() constructor...WTF?
            DeployableSE* dSE = new DeployableSE(cRef, *m_manager, pSysMgr, data);
            location.MakeRandomPointOnSphere(1500.0 + cRef->type().radius());
            dSE->SetPosition(location);
            cRef->SaveItem();
            pSysMgr->AddEntity(dSE);
            pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket();
            continue;
        } //else if ()
        /** @todo  Handle other cargo */

        // item isnt structure or deployable and can be jettisoned.  check if container was already created
        if ((ccRef.get() == nullptr) or (jcRef.get() == nullptr)) {
            if (!pClient->IsJetcanAvalible()) {
                std::string msg = "A Jettison Container is currently being prepped in your cargo hold. \n";
                msg += "Your estimated wait time is ";
                msg += itoa(pClient->JetcanTime());
                msg += " seconds.";
                pClient->SendNotifyMsg(msg.c_str());
                return tuple;
            }
            // Spawn jetcan then continue loop
            location.MakeRandomPointOnSphere(500.0);
            ItemData p_idata(
                            23,                         // 23 = cargo container
                            pClient->GetCharacterID(),  //owner is Character?  figure out how to test for corp owner
                            pClient->GetLocationID(),
                            flagAutoFit,
                            "Jettisoned Cargo Container",
                            location);

            jcRef = sItemFactory.SpawnCargoContainer(p_idata);
            if (jcRef.get() == nullptr)
                throw PyException(MakeCustomError("Unable to spawn jetcan."));
            // create new container
            ContainerSE* cSE = new ContainerSE(jcRef, *m_manager, pSysMgr, data);

            jcRef->SetMySE(cSE);
            // set anchored to avoid deletion when empty
            jcRef->SetAnchor(true);
            pSysMgr->AddEntity(cSE);
            pClient->GetShipSE()->DestinyMgr()->SendJettisonPacket();
            pClient->StartJetcanTimer();
        }
        /** @todo  check current can for capacity limits. */
        //if over limit create new can?  reject remainging cargo?  delete?  crash?  run thru station naked?
        pClient->MoveItem(cur, (ccRef ? ccRef->itemID() : jcRef->itemID()), flagAutoFit);
        continue;
    }

    return tuple;
}

/**     ***********************************************************************
 * @note   these below are partially coded
 */

PyResult ShipBound::Handle_AssembleShip(PyCallArgs &call) {

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
     *              [PyTuple 2 items]     << response to AssembleShip call
     *                [PyList 1 items]
     *                  [PyPackedRow 33 bytes]
     *                    ["itemID" => <1002333477860> [I8]]
     *                    ["typeID" => <24700> [I4]]
     *                    ["ownerID" => <1661059544> [I4]]
     *                    ["locationID" => <61000012> [I8]]
     *                    ["flagID" => <4> [I2]]
     *                    ["quantity" => <-1> [I4]]
     *                    ["groupID" => <419> [I2]]
     *                    ["categoryID" => <6> [I2]]
     *                    ["customInfo" => <empty string> [Str]]
     *                [PyDict 1 kvp]
     *                  [PyInt 10]        << flagdisconnect??
     *                  [PyInt 0]
     */

    call.Dump(COLLECT__CALL_DUMP);
    if (call.tuple->empty())
        return nullptr;

    Call_AssembleShip args;
    //Call_AssembleShipWithName argsNamed;

    std::vector<int32> itemIDList;
    bool completeTech3Assembly = false;
    if (call.tuple->GetItem(0)->IsList()) {
        if (!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
            return nullptr;
        }
        itemIDList = args.items;
    } else if (call.tuple->GetItem(0)->IsInt() &&
        call.tuple->GetItem(1)->IsString()) {
        // This block is for how DNA calls AssembleShip
        // @TODO Ignoring name
        // Can't get xmlpktgen to pickup the change so.. lol
        //if (!argsNamed.Decode(&call.tuple)) {
        //    codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        //    return nullptr;
        //}
        itemIDList.push_back(call.tuple->GetItem(0)->AsInt()->value());
        } else { // Because we check for the second item in the list being string we get here for t3 ship assembly
            sLog.Error( "Handle_AssembleShip", "Modular ships are not implemented yet" );
            throw PyException(MakeCustomError("Modular ships are not implemented yet." ) );
            return nullptr;
            //Call_AssembleShipTech3 argsT3;
            //argsT3.item;
        }

        ShipItemRef ship(nullptr);
        for (auto cur : itemIDList) {
            ship = sItemFactory.GetShip(cur);

            if (ship.get() == nullptr) {
                _log(ITEM__ERROR, "Failed to load ship %u to assemble.", cur);
                continue;
            }

            //check if the ship is a stack
            if (ship->quantity() > 1) {
                // Split the stack into a new inventory item with quantity of one, cast to ShipItemRef then assembled:
                // original item stack will be left with qty-1 at original location
                ship = ShipItemRef::StaticCast(ship->Split(1, true));
                if (ship.get() == nullptr) {
                    _log(ITEM__ERROR, "Failed to split stack to assemble ship %u.", cur);
                    continue;
                }
            }

            ship->ChangeSingleton(true, true);

            if (completeTech3Assembly) {
                std::vector<uint32> subSystemList;
                // Move the five specified subsystems to the newly assembled Tech 3 ship
                InventoryItemRef subSystemItem(nullptr);
                for (auto cur : subSystemList) {
                    subSystemItem = sItemFactory.GetItem(cur);
                    subSystemItem->Move(ship->itemID(), (EVEItemFlags)(subSystemItem->GetAttribute(AttrSubSystemSlot).get_int()), true);
                }
            }
        }
        return nullptr;
}


/**     ***********************************************************************
 * @note   these do absolutely nothing at this time....
 */



PyResult ShipBound::Handle_LaunchFromContainer(PyCallArgs &call) {
    /*
     * def LaunchSMAContents(self, invItems):
     *        ids = []
     *        for invItem in invItems:
     *            structureID = invItem.locationID
     *            ids += [invItem.itemID] * invItem.stacksize
     *
     *  LaunchFromContainer(structureID, ids)
     */

    _log(SERVICE__CALL_DUMP, "ShipBound::Handle_LaunchFromContainer()");
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
}


// ShipMaintenanceArray
PyResult ShipBound::Handle_ScoopToSMA(PyCallArgs &call) {
    /*      ******* no packet data ***********     */

    _log(SERVICE__CALL_DUMP, "ShipBound::Handle_ScoopToSMA()");
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
}

PyResult ShipBound::Handle_GetShipConfiguration(PyCallArgs &call) {
    /*      ******* no packet data ***********
     *
     *     13:15:58 L ShipBound::Handle_GetShipConfiguration(): size=0
     *     13:15:58 [SvcCall]   Call Arguments:
     *     13:15:58 [SvcCall]       Tuple: Empty
     *     _log(SERVICE__CALL_DUMP, "ShipBound::Handle_GetShipConfiguration()", "size=%u", call.tuple->size());
     *     call.Dump(SERVICE__CALL_DUMP);
     */

    /*
     *        self.conf = self.ship.GetShipConfiguration()
     *        self.sr.smballowfleet.SetChecked(self.conf['allowFleetSMBUsage']) (SMB = ShipMaintenanceBay)
     */

    return nullptr;
}


PyResult ShipBound::Handle_BoardStoredShip(PyCallArgs &call) {
    /*      ******* no packet data ***********
     */
    //sm.StartService('sessionMgr').PerformSessionChange('board', ship.BoardStoredShip, structureID, shipID)
    _log(SERVICE__CALL_DUMP, "ShipBound::Handle_BoardStoredShip()");
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
}

PyResult ShipBound::Handle_StoreVessel(PyCallArgs &call) {
    /*      ******* no packet data ***********
     */
    //sm.StartService('sessionMgr').PerformSessionChange('storeVessel', ship.StoreVessel, destID)
    _log(SERVICE__CALL_DUMP, "ShipBound::Handle_StoreVessel()");
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
}

PyResult ShipBound::Handle_SelfDestruct(PyCallArgs &call) {
    /** @todo finish this later
     * 22:13:29 L ShipBound::Handle_SelfDestruct(): size=1
     * 22:13:29 [SvcCall]   Call Arguments:
     * 22:13:29 [SvcCall]       Tuple: 1 elements
     * 22:13:29 [SvcCall]         [ 0] Integer field: 140000378     <- ship id
     *
  _log(SERVICE__CALL_DUMP, "ShipBound::Handle_SelfDestruct()");
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

    /*{'messageKey': 'SelfDestructAborted2', 'dataID': 17879480, 'suppressable': False, 'bodyID': 258024, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2405}
     * {'messageKey': 'SelfDestructAbortedOther2', 'dataID': 17879483, 'suppressable': False, 'bodyID': 258025, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2406}
     * {'messageKey': 'SelfDestructCancelledExternal', 'dataID': 17876999, 'suppressable': False, 'bodyID': 257085, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3182}
     * {'messageKey': 'SelfDestructCancelledWarp', 'dataID': 17878652, 'suppressable': False, 'bodyID': 257707, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2564}
     * {'messageKey': 'SelfDestructImmediate', 'dataID': 17881593, 'suppressable': False, 'bodyID': 258829, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1533}
     * {'messageKey': 'SelfDestructImmediateOther', 'dataID': 17881596, 'suppressable': False, 'bodyID': 258830, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1534}
     * {'messageKey': 'SelfDestructInitiated', 'dataID': 17881805, 'suppressable': False, 'bodyID': 258902, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1535}
     * {'messageKey': 'SelfDestructInitiatedOther', 'dataID': 17881599, 'suppressable': False, 'bodyID': 258831, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1536}
     * {'messageKey': 'SelfDestructTimer', 'dataID': 17878655, 'suppressable': False, 'bodyID': 257708, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2563}
     * {'messageKey': 'SelfDestructTooEarly', 'dataID': 17881605, 'suppressable': False, 'bodyID': 258833, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1537}
     */
    /* return error msg from this call, if applicable, else nodeid and timestamp */
    // returns nodeID and timestamp
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
        tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    return tuple;
}
