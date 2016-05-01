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
#include "packets/Repair.h"
#include "station/RepairService.h"

class RepairSvcBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(RepairSvcBound)

    RepairSvcBound(PyServiceMgr* mgr, uint32 locationID)
    : PyBoundObject(mgr),
    m_dispatch(new Dispatcher(this)),
    m_locationID(locationID)
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "RepairSvcBound";

        PyCallable_REG_CALL(RepairSvcBound, GetDamageReports);

    }
    virtual ~RepairSvcBound() {
        delete m_dispatch;
    }
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(GetDamageReports);

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

    Call_SingleIntegerArg args;
    PyRep *t = bind_args->Clone();
    if(!args.Decode(&t)) {
        codelog(SERVICE__ERROR, "Failed to decode bind args from '%s'", c->GetName());
        return nullptr;
    }

    return new RepairSvcBound(m_manager, args.arg);
}

PyResult RepairSvcBound::Handle_GetDamageReports(PyCallArgs &call) {
    /**
            damageReports = self.repairSvc.GetDamageReports(itemIDs)
        currIndex = 1
        for item in items:
            for each in damageReports[item.itemID].quote:
                */

    sLog.Warning("RepairSvcBound::Handle_GetDamageReports", "Called GetDamageReports stub.");
    call.Dump(SERVICE__CALL_DUMP);

    Call_SingleIntegerArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "Failed to decode bind args from '%s'", call.client->GetName());
        return nullptr;
    }

    /** @todo loop here for each item in repair list */
    PyDict* dict = new PyDict;
        dict->SetItem(new PyInt(0), m_rs->GetDamageReports());
    return dict;
}

PyResult RepairService::Handle_UnasembleItems(PyCallArgs &call) {
/**
                sm.RemoteSvc('repairSvc').UnasembleItems(dict(validIDsByStationID), skipChecks)

repackableCategorys = (categoryStructure,
 categoryShip,
 categoryDrone,
 categoryModule,
 categorySubSystem,
 categorySovereigntyStructure)
repackableGroups = (groupCargoContainer,
 groupSecureCargoContainer,
 groupAuditLogSecureContainer,
 groupFreightContainer,
 groupTool,
 groupMobileWarpDisruptor)
 */

  /*
19:49:29 L RepairService::Handle_UnasembleItems: Called UnasembleItems stub.
19:49:29 [SvcCall]   Call Arguments:
19:49:29 [SvcCall]       Tuple: 2 elements
19:49:29 [SvcCall]         [ 0] Dictionary: 1 entries
19:49:29 [SvcCall]         [ 0]   [ 0] Key: Integer field: 60004450
19:49:29 [SvcCall]         [ 0]   [ 0] Value: List: 1 elements
19:49:29 [SvcCall]         [ 0]   [ 0] Value:   [ 0] Tuple: 2 elements
19:49:29 [SvcCall]         [ 0]   [ 0] Value:   [ 0]   [ 0] Integer field: 140001999
19:49:29 [SvcCall]         [ 0]   [ 0] Value:   [ 0]   [ 1] Integer field: 60004450
19:49:29 [SvcCall]         [ 1] List: Empty
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
          if (pList) {
              //uint32 locationID = pInt->value();
              // Iterate through list.
              PyList::const_iterator itemItr = pList->begin();
              for (; itemItr != pList->end(); itemItr++) {
                  // List is tuples of itemID, LocationID pairs.
                  PyTuple *tuple = (*itemItr)->AsTuple();
                  if (tuple) {
                      // Get the itemID.
                      PyInt *itemInt = tuple->GetItem(0)->AsInt();
                      //PyInt *itemLocation = tuple->GetItem(1)->AsInt();
                      if (itemInt) {
                          // Get the item itself.
                          uint32 itemID = itemInt->value();
                          InventoryItemRef item = factory->GetItem(itemID);
                          if (item) {
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
  return nullptr;
}

PyObject* RepairService::GetDamageReports() {
    /*
        [PyDict 8 kvp]
          [PyIntegerVar 1002331329817]
          [PyObjectData Name: util.Row]
            [PyDict 2 kvp]
              [PyString "header"]
              [PyList 4 items]
                [PyString "playerStanding"]
                [PyString "serviceCharge"]
                [PyString "discount"]
                [PyString "quote"]
              [PyString "line"]
              [PyList 4 items]
                [PyInt 0]
                [PyString "0%"]
                [PyString "0%"]
                [PyObjectData Name: util.Rowset]
                  [PyDict 3 kvp]
                    [PyString "header"]
                    [PyList 7 items]
                      [PyString "itemID"]
                      [PyString "typeID"]
                      [PyString "groupID"]
                      [PyString "damage"]
                      [PyString "maxHealth"]
                      [PyString "repairable"]
                      [PyString "costToRepairOneUnitOfDamage"]
                    [PyString "RowClass"]
                    [PyToken util.Row]
                    [PyString "lines"]
                    [PyList 1 items]
                      [PyList 7 items]
                        [PyIntegerVar 1002331329817]
                        [PyInt 20138]
                        [PyInt 771]
                        [PyInt 0]
                        [PyInt 40]
                        [PyInt 1]
                        [PyFloat 430]
            */

    /** @todo  need to make *something in a stl container* with all items in shipID requested */

    RepairListData rld;
        rld.itemID                      = 0;
        rld.typeID                      = 0;
        rld.groupID                     = 0;
        rld.damage                      = 0;
        rld.maxHealth                   = 0;
        rld.repairable                  = 0;
        rld.costToRepairOneUnitOfDamage = 0;

    DBRowDescriptor* header = CreateHeader();
    PyList* list = new PyList();

        PyPackedRow* row = new PyPackedRow( header );
            row->SetField( "playerStanding", new PyLong(0));
            row->SetField( "serviceCharge",  new PyInt(0));
            row->SetField( "discount",       new PyInt(0));
            row->SetField( "quote",          new PyInt(0));
        list->AddItem(row);


    PyTuple* tuple2 = new PyTuple(1);
        tuple2->SetItem(0, list);
    PyToken* token = new PyToken("util.Row");
    PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, token);
        tuple->SetItem(1, tuple2);

    PyList* itemNames = new PyList(4);
        itemNames->SetItem(0, new PyString("playerStanding"));
        itemNames->SetItem(1, new PyString("serviceCharge"));
        itemNames->SetItem(2, new PyString("discount"));
        itemNames->SetItem(3, new PyString("quote"));
    RepairListRsp rlr;
        rlr.header  = itemNames;
        rlr.line    = list;
    rlr.Dump(CLIENT__CALL_DUMP, "   ");
    return rlr.Encode();

}

DBRowDescriptor* RepairService::CreateHeader() {
    DBRowDescriptor* header = new DBRowDescriptor;
    header->AddColumn( "playerStanding", DBTYPE_I4 );
    header->AddColumn( "serviceCharge",  DBTYPE_STR );
    header->AddColumn( "discount",       DBTYPE_STR );
    header->AddColumn( "quote",          DBTYPE_I8 );
    return header;
}

DBRowDescriptor* RepairService::CreateQuoteHeader() {
    DBRowDescriptor* header = new DBRowDescriptor;
    header->AddColumn( "itemID",                      DBTYPE_I8 );
    header->AddColumn( "typeID",                      DBTYPE_I4 );
    header->AddColumn( "groupID",                     DBTYPE_I4 );
    header->AddColumn( "damage",                      DBTYPE_I4 );
    header->AddColumn( "maxHealth",                   DBTYPE_I4 );
    header->AddColumn( "repairable",                  DBTYPE_I2 );
    header->AddColumn( "costToRepairOneUnitOfDamage", DBTYPE_R8 );
    return header;
}
