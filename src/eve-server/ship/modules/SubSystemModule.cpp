
 /**
  * @name SubSystemModule.cpp
  *   SubSystem module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */

#include "ship/modules/SubSystemModule.h"


SubSystemModule::SubSystemModule(InventoryItemRef item, ShipItemRef ship)
: PassiveModule(item, ship)
{

}

//not much to do here... hopefully there won't be
int8 SubSystemModule::GetModulePowerLevel()
{
    return Module::Bank::Subsystem;
}
