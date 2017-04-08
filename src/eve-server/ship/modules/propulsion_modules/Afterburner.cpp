
 /**
  * @name Afterburner.cpp
  *   propulsion module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */



#include "ship/modules/propulsion_modules/Afterburner.h"


Afterburner::Afterburner( InventoryItemRef item, ShipItemRef ship )
: ActiveModule(item, ship)
{
}

void Afterburner::Activate(uint16 effectID, uint32 targetID, int16 repeat)
{
    ActiveModule::Activate(effectID, targetID, repeat);
    m_destinyMgr->SpeedBoost();
}

void Afterburner::DeactivateCycle(bool abort)
{
    ActiveModule::DeactivateCycle(abort);
    m_destinyMgr->SpeedBoost(true);
}
