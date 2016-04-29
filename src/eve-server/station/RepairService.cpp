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

#include "PyServiceCD.h"
#include "station/RepairService.h"

PyCallable_Make_InnerDispatcher(RepairService)

RepairService::RepairService(PyServiceMgr *mgr)
: PyService(mgr, "repairSvc"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(RepairService, UnasembleItems);
    PyCallable_REG_CALL(RepairService, GetDamageReports);
}

RepairService::~RepairService() {
    delete m_dispatch;
}

PyResult RepairService::Handle_GetDamageReports(PyCallArgs &call) {
    /**
      sm.RemoteSvc('repairSvc').UnasembleItems(dict(validIDsByStationID), skipChecks)

            damageReports = self.repairSvc.GetDamageReports(itemIDs)

                    self.repairSvc.RepairItems(itemIDs, amount['qty'])


                itemIDAndAmountOfDamageList.append((item.itemID, amount))
                self.repairSvc.DamageModules(itemIDAndAmountOfDamageList)
                */

    sLog.Warning("RepairService::Handle_GetDamageReports", "Called GetDamageReports stub.");
     call.Dump(SERVICE__CALL_DUMP);
return nullptr;
}

/*
22:44:49 [SvcError] repairSvc Service: emily: Unable to create bound object for:
22:44:49 [SvcError]     Integer field: 60014749
*/
PyResult RepairService::Handle_UnasembleItems(PyCallArgs &call) {
  /**
                sm.RemoteSvc('repairSvc').UnasembleItems(dict(validIDsByStationID), skipChecks)
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
