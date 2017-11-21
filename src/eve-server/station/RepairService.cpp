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
    Author:        Reve
    Updates:    Allan
*/

#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "Client.h"
#include "packets/Repair.h"
#include "station/RepairService.h"
#include "station/Station.h"
#include "system/SystemManager.h"

class RepairSvcBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(RepairSvcBound)

    RepairSvcBound(PyServiceMgr* mgr, uint32 locationID)
    : PyBoundObject(mgr),
    m_dispatch(new Dispatcher(this)),
    m_locationID(locationID)    // this is stationID
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "RepairSvcBound";

        PyCallable_REG_CALL(RepairSvcBound, GetDamageReports);
        PyCallable_REG_CALL(RepairSvcBound, RepairItems);
        PyCallable_REG_CALL(RepairSvcBound, DamageModules);
    }
    virtual ~RepairSvcBound() {
        delete m_dispatch;
    }
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(GetDamageReports);
    PyCallable_DECL_CALL(RepairItems);
    PyCallable_DECL_CALL(DamageModules);

protected:
    Dispatcher* const m_dispatch;
    ItemFactory m_ifac;

    uint32 m_locationID;
};

PyCallable_Make_InnerDispatcher(RepairService)

RepairService::RepairService(PyServiceMgr *mgr)
: PyService(mgr, "repairSvc"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(RepairService, UnasembleItems);
}

RepairService::~RepairService() {
    delete m_dispatch;
}

PyBoundObject* RepairService::_CreateBoundObject(Client* c, const PyRep* bind_args) {
    _log(CLIENT__MESSAGE, "RepairService bind request for:");
    bind_args->Dump(CLIENT__MESSAGE, "    ");

    return new RepairSvcBound(m_manager, bind_args->AsInt()->value());
}

PyResult RepairSvcBound::Handle_DamageModules(PyCallArgs &call) {
    /*    itemIDAndAmountOfDamageList.append((item.itemID, amount))
     *    self.repairSvc.DamageModules(itemIDAndAmountOfDamageList)
     */
    sLog.White( "RepairSvcBound::Handle_DamageModules()", "size= %u", call.tuple->size() );
    call.Dump(PHYSICS__INFO);

    Call_SingleIntList args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode bind args from '%s'", call.client->GetName());
        return nullptr;
    }

    return new PyNone();
}

PyResult RepairSvcBound::Handle_RepairItems(PyCallArgs &call) {
    //  self.repairSvc.RepairItems(itemIDs, amount['qty'])
    /*
     * 00:18:28 W RepairSvcBound::Handle_RepairItems(): size= 2
     * 00:18:28 [PhysicsInfo]   Call Arguments:
     * 00:18:28 [PhysicsInfo]       Tuple: 2 elements
     * 00:18:28 [PhysicsInfo]         [ 0] List: 1 elements                 <-- list of itemIDs to repair
     * 00:18:28 [PhysicsInfo]         [ 0]   [ 0] Integer field: 140005905  <-- itemID
     * 00:18:28 [PhysicsInfo]         [ 1] Real field: 112500.000000        <-- isk amount to spend on repairs.
     */

    Call_RepairItems args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode bind args from '%s'", call.client->GetName());
        return nullptr;
    }
    if (args.iskAmount < 0.01)
        return nullptr;

    /* itemIDs is list of itemIDs to repair.
     * iskAmount is how much to spend on repairs.
     *  - this needs to be checked against total repair amount, and use fraction to reduce damage for all items in list
     */

    std::vector<InventoryItemRef> itemRefVec;
    Inventory* pInv = call.client->SystemMgr()->GetStationFromInventory(m_locationID)->GetMyInventory();
    ShipItem* pShip(nullptr);
    InventoryItemRef iRef;
    float fraction = 1.0;
    double cost = 0, total = 0;
    uint32 damage = 0, hp = 0, delta = 0;
    PyList::const_iterator itr = args.itemIDs->begin();
    for (; itr != args.itemIDs->end(); ++itr) {
        hp = 0;
        cost = 0;
        delta = 0;
        damage = 0;
        iRef = pInv->GetByID((*itr)->AsInt()->value());
        if (iRef.get() == nullptr) {
            iRef = m_ifac.GetItem((*itr)->AsInt()->value());
            if (iRef.get() == nullptr)
                continue;
        }
        hp         = iRef->GetAttribute(AttrHP).get_int();
        damage     = iRef->GetAttribute(AttrDamage).get_int();
        if (iRef->IsShipItem()) {
            if ((pShip != nullptr) and (pShip != iRef->GetShipItem())) {
                codelog(ITEM__ERROR, "Got a new ship item here.  Rework this code!");
                return new PyNone();
            }
            pShip = iRef->GetShipItem();
            hp     += iRef->GetAttribute(AttrArmorHP).get_int();
            damage += iRef->GetAttribute(AttrArmorDamage).get_int();
            // ship is (basePrice)*7.5e-10
            cost   = (iRef->type().basePrice() * 0.00000000075);
        } else {
            itemRefVec.push_back(iRef);
            // modules are (basePrice)*1.25e-6
            cost   = (iRef->type().basePrice() * 0.00000125);
        }
        delta = hp - damage;
        total += delta * cost;
    }

    if (args.iskAmount < total)
        fraction = total / args.iskAmount;

    pShip->RepairShip(fraction);
    pShip->RepairModules(itemRefVec, fraction);

    return new PyNone();
}

PyResult RepairSvcBound::Handle_GetDamageReports(PyCallArgs &call) {
    /*
     * 20:39:30 W RepairSvcBound::Handle_GetDamageReports(): size= 1
     * 20:39:30 [SvcCallDump]   Call Arguments:
     * 20:39:30 [SvcCallDump]       Tuple: 1 elements
     * 20:39:30 [SvcCallDump]         [ 0] List: 1 elements
     * 20:39:30 [SvcCallDump]         [ 0]   [ 0] Integer field: 140012041
     */
    Call_SingleIntList args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode bind args from '%s'", call.client->GetName());
        return nullptr;
    }

    PyDict* dict = new PyDict();
    Client* pClient = call.client;
    StationItemRef sRef = pClient->SystemMgr()->GetStationFromInventory(m_locationID);
    Inventory* pInv = sRef->GetMyInventory();
    float standing = 0;
    // standing system isnt complete, but this is the correct data methods for station standing checks
    if (IsNPCCorp(sRef->ownerID()))
        standing = pClient->GetChar()->GetNPCCorpStanding(pClient->GetCharacterID(), sRef->ownerID());
    else
        standing = pClient->GetChar()->GetCorpStanding(pClient->GetCharacterID(), sRef->ownerID());

    for (auto cur : args.ints) {
        RepairListRsp rlr;
            rlr.discount       = "0%";  // not sure....seen 0% and 100% in packets
            rlr.serviceCharge  = "0%";  // not sure....seen 0% in packets
            rlr.playerStanding = standing;
            rlr.lines = new PyList();
            RepairService::GetDamageReports(cur, pInv, rlr.lines);
        dict->SetItem(new PyInt(cur), rlr.Encode());
    }

    return dict;
}

void RepairService::GetDamageReports(uint32 itemID, Inventory* pInv, PyList* list) {
    ItemFactory m_ifac;
    std::vector<InventoryItemRef> itemRefVec;
    InventoryItemRef iRef = pInv->GetByID(itemID);
    if (iRef.get() == nullptr) {
        iRef = m_ifac.GetItem(itemID);
        if (iRef.get() == nullptr)
            return;
    }
    itemRefVec.push_back(iRef);
    if (iRef->IsShipItem())
        iRef->GetShipItem()->GetModuleRefVec(itemRefVec);

    for (auto cur : itemRefVec) {
        RepairItemData rid;
        rid.itemID                     = cur->itemID();
        rid.typeID                     = cur->typeID();
        rid.groupID                    = cur->groupID();
        rid.damage                     = cur->GetAttribute(AttrDamage).get_int();
        rid.maxHealth                  = cur->GetAttribute(AttrHP).get_int();
        // not sure how to find this data
        rid.repairable                 = 1;
        if (cur->IsShipItem()) {
            rid.damage                 += cur->GetAttribute(AttrArmorDamage).get_int();
            rid.maxHealth              += cur->GetAttribute(AttrArmorHP).get_int();
            // ship is (basePrice)*7.5e-10
            rid.costToRepairOneUnitOfDamage = (cur->type().basePrice() * 0.00000000075);
        } else {
            // modules are (basePrice)*1.25e-6
            rid.costToRepairOneUnitOfDamage = (cur->type().basePrice() * 0.00000125);
        }

        list->AddItem(rid.Encode());
    }
}

PyResult RepairService::Handle_UnasembleItems(PyCallArgs &call) {
    /**
     *                sm.RemoteSvc('repairSvc').UnasembleItems(dict(validIDsByStationID), skipChecks)
     *
     * repackableCategorys = (categoryStructure,
     * categoryShip,
     * categoryDrone,
     * categoryModule,
     * categorySubSystem,
     * categorySovereigntyStructure)
     * repackableGroups = (groupCargoContainer,
     * groupSecureCargoContainer,
     * groupAuditLogSecureContainer,
     * groupFreightContainer,
     * groupTool,
     * groupMobileWarpDisruptor)
     */

    /*
     * 19:49:29 L RepairService::Handle_UnasembleItems: Called UnasembleItems stub.
     * 19:49:29 [SvcCall]   Call Arguments:
     * 19:49:29 [SvcCall]       Tuple: 2 elements
     * 19:49:29 [SvcCall]         [ 0] Dictionary: 1 entries
     * 19:49:29 [SvcCall]         [ 0]   [ 0] Key: Integer field: 60004450
     * 19:49:29 [SvcCall]         [ 0]   [ 0] Value: List: 1 elements
     * 19:49:29 [SvcCall]         [ 0]   [ 0] Value:   [ 0] Tuple: 2 elements
     * 19:49:29 [SvcCall]         [ 0]   [ 0] Value:   [ 0]   [ 0] Integer field: 140001999
     * 19:49:29 [SvcCall]         [ 0]   [ 0] Value:   [ 0]   [ 1] Integer field: 60004450
     * 19:49:29 [SvcCall]         [ 1] List: Empty
     */

    /*
    [PyTuple 1 items]
      [PyTuple 2 items]
        [PyInt 0]
        [PySubStream 55 bytes]
          [PyTuple 4 items]
            [PyInt 1]
            [PyString "UnasembleItems"]
            [PyTuple 2 items]
              [PyDict 1 kvp]
                [PyIntegerVar 61000533]
                [PyList 1 items]
                  [PyTuple 2 items]
                    [PyIntegerVar 1005888156061]
                    [PyIntegerVar 61000533]
              [PyList 0 items]
    */
    /** @todo verify and update this... */
    /** @todo  check if this is container, and remove items BEFORE repacking!!  */
    if (call.tuple->size() == 2)
    {
        bool repackDamaged = false;
        ItemFactory* factory = call.client->services().item_factory;
        // Call contains dictionary and empty list, get the dictionary.
        PyDict *dict = call.tuple->GetItem(0)->AsDict();
        PyDict::const_iterator cur = dict->begin();
        for (; cur != dict->end(); cur++) {
            // Dictionary is of Int as a locationID and list of item entries.
            //PyInt *pInt = cur->first->AsInt();
            // Location is irrelevant so get list.
            PyList *pList = cur->second->AsList();
            if (pList != nullptr) {
                //uint32 locationID = pInt->value();
                // Iterate through list.
                PyList::const_iterator itemItr = pList->begin();
                for (; itemItr != pList->end(); itemItr++) {
                    // List is tuples of itemID, LocationID pairs.
                    PyTuple *tuple = (*itemItr)->AsTuple();
                    if (tuple != nullptr) {
                        // Get the itemID.
                        PyInt *itemInt = tuple->GetItem(0)->AsInt();
                        //PyInt *itemLocation = tuple->GetItem(1)->AsInt();
                        if (itemInt != nullptr) {
                            // Get the item itself.
                            InventoryItemRef item = factory->GetItem(itemInt->value());
                            if (item.get() != nullptr) {
                                // Add type exceptions here.
                                if (item->categoryID() == EVEDB::invCategories::Blueprint
                                    or item->categoryID() == EVEDB::invCategories::Skill
                                    or item->categoryID() == EVEDB::invCategories::Material) {
                                    // Item cannot be repackaged once used!
                                    continue;
                                    }
                                    if (item->GetAttribute(AttrDamage) == 0)
                                        item->ChangeSingleton(false);
                                    else
                                        repackDamaged = true;
                            }
                        }
                    }
                }
            }
        }
        if (repackDamaged)
            throw PyException(MakeCustomError("Cannot repackage damaged items."));
    }

    return new PyNone();
}
