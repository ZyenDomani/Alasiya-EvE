
 /**
  * @name RigModule.cpp
  *   rig module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */

#include "ship/modules/RigModule.h"


RigModule::RigModule(ModuleItemRef mRef, ShipItemRef sRef)
: PassiveModule(mRef, sRef)
{

}


//not much to do here... hopefully there won't be
int8 RigModule::GetModulePowerLevel()
{
    return Module::Bank::Rig;
}

void RigModule::DestroyRig()
{
    //delete the item
    m_modRef->Delete();
}
