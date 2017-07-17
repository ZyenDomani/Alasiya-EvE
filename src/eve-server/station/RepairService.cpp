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
#include "Station.h"
#include <system/SystemManager.h>

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
    RepairService* m_rs;

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

PyResult RepairSvcBound::Handle_RepairItems(PyCallArgs &call) {
    //  self.repairSvc.RepairItems(itemIDs, amount['qty'])

    sLog.White( "RepairSvcBound::Handle_RepairItems()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    Call_SingleIntList args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode bind args from '%s'", call.client->GetName());
        return nullptr;
    }

    /** @todo loop here for each item in repair list */
    PyDict* dict = new PyDict();
    return dict;
}

PyResult RepairSvcBound::Handle_DamageModules(PyCallArgs &call) {
    /*    itemIDAndAmountOfDamageList.append((item.itemID, amount))
     *    self.repairSvc.DamageModules(itemIDAndAmountOfDamageList)
     */

    sLog.White( "RepairSvcBound::Handle_DamageModules()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    Call_SingleIntList args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode bind args from '%s'", call.client->GetName());
        return nullptr;
    }

    /** @todo loop here for each item in repair list */
    PyDict* dict = new PyDict();
    return dict;
}

PyResult RepairSvcBound::Handle_GetDamageReports(PyCallArgs &call) {
    /*
     * 20:39:30 W RepairSvcBound::Handle_GetDamageReports(): size= 1
     * 20:39:30 [SvcCallDump]   Call Arguments:
     * 20:39:30 [SvcCallDump]       Tuple: 1 elements
     * 20:39:30 [SvcCallDump]         [ 0] List: 1 elements
     * 20:39:30 [SvcCallDump]         [ 0]   [ 0] Integer field: 140012041
     */
    sLog.White( "RepairSvcBound::Handle_GetDamageReports()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    Call_SingleIntList args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode bind args from '%s'", call.client->GetName());
        return nullptr;
    }

    PyDict* dict = new PyDict();
    for (auto cur : args.ints)
        dict->SetItem(new PyInt(cur), m_rs->GetDamageReports(call.client, m_locationID, cur));

    if (is_log_enabled(CLIENT__CALL_REP))
        dict->Dump(CLIENT__CALL_REP, "   ");

    return dict;
}

PyObject* RepairService::GetDamageReports(Client* pClient, uint32 stationID, uint32 itemID) {
    PyDict* data = new PyDict();
        //data->SetItemString("RowClass", new PyToken("util.Row"));
    PyList* headerList = new PyList(7);
        headerList->SetItem(0, new PyString("itemID"));
        headerList->SetItem(1, new PyString("typeID"));
        headerList->SetItem(2, new PyString("groupID"));
        headerList->SetItem(3, new PyString("damage"));
        headerList->SetItem(4, new PyString("maxHealth"));
        headerList->SetItem(5, new PyString("repairable"));
        headerList->SetItem(6, new PyString("costToRepairOneUnitOfDamage"));
    data->SetItemString("header", headerList);

    InventoryItemRef itemRef = pClient->SystemMgr()->GetStationFromInventory(stationID)->GetMyInventory()->GetByID(itemID);
    RepairListItemData rlid;
        rlid.itemID                      = itemID;
        rlid.typeID                      = itemRef->typeID();
        rlid.groupID                     = itemRef->groupID();
        // these should be total damage for ships
        if (itemRef->IsShipItem()) {
            rlid.damage                  = itemRef->GetAttribute(AttrDamage).get_int();
            rlid.maxHealth               = itemRef->GetAttribute(AttrHP).get_int();
            rlid.damage                 += itemRef->GetAttribute(AttrArmorDamage).get_int();
            rlid.maxHealth              += itemRef->GetAttribute(AttrArmorHP).get_int();
        } else {
            rlid.damage                  = itemRef->GetAttribute(AttrDamage).get_int();
            rlid.maxHealth               = itemRef->GetAttribute(AttrHP).get_int();
        }
        // not sure how to find this data
        rlid.repairable                  = 1;
        // not sure how to do this one yet
        rlid.costToRepairOneUnitOfDamage = 450;
    data->SetItemString("line", rlid.Encode());

    if (itemRef->IsShipItem()) {
        ShipItem* shipItem = itemRef->GetShipItem();

    }

    RepairListData rld;
        rld.playerStanding = pClient->GetSecurityRating();  // testing...fix later
        rld.serviceCharge  = "0%";
        rld.discount       = "0%";
        rld.quote          = new PyObject("util.Row", data);
    PyList* itemNames = new PyList(4);
        itemNames->SetItem(0, new PyString("playerStanding"));
        itemNames->SetItem(1, new PyString("serviceCharge"));
        itemNames->SetItem(2, new PyString("discount"));
        itemNames->SetItem(3, new PyString("quote"));
    RepairListRsp rlr;
        rlr.header  = itemNames;
        rlr.line    = rld.Encode();
    return rlr.Encode();
}

DBRowDescriptor* RepairService::CreateHeader() {
    DBRowDescriptor* header = new DBRowDescriptor;
    header->AddColumn( "playerStanding", DBTYPE_R4 );
    header->AddColumn( "serviceCharge",  DBTYPE_STR );
    header->AddColumn( "discount",       DBTYPE_STR );
    header->AddColumn( "quote",          DBTYPE_I8 );
    return header;
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
