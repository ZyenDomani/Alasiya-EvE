
 /**
  * @name SubSystemModule.h
  *   SubSystem module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */

#ifndef _EVE_SHIP_SUBSYSTEM_MODULE_H
#define _EVE_SHIP_SUBSYSTEM_MODULE_H

#include "ship/modules/PassiveModule.h"

class SubSystemModule
: public PassiveModule
{
public:
    SubSystemModule(InventoryItemRef item, ShipItemRef ship);
    virtual ~SubSystemModule() { }

    bool IsSubSystemModule() const                      { return true; }

    ModStates::ModulePowerLevel GetModulePowerLevel();
};

#endif  // _EVE_SHIP_SUBSYSTEM_MODULE_H
