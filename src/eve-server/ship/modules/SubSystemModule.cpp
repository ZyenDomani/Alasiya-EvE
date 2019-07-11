
 /**
  * @name SubSystemModule.cpp
  *   SubSystem module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */

#include "ship/modules/SubSystemModule.h"


SubSystemModule::SubSystemModule(ModuleItemRef mRef, ShipItemRef sRef)
: PassiveModule(mRef, sRef)
{

}

int8 SubSystemModule::GetModulePowerLevel()
{
    return Module::Bank::Subsystem;
}

//not much to do here... this will be for t3 ships, which arent implented yet

