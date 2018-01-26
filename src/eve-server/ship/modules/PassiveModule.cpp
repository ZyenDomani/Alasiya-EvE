
 /**
  * @name PassiveModule.cpp
  *   passive module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */
 
#include "eve-server.h"

#include "ship/modules/PassiveModule.h"

PassiveModule::PassiveModule(InventoryItemRef item, ShipItemRef ship)
: GenericModule(item, ship)
{
}

//  we may not have anything to do here.