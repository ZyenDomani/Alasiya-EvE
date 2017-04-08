
 /**
  * @name Afterburner.h
  *   propulsion module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */


#ifndef __EVESERVER_SHIPMODULES_ACTIVE_MODULES_AFTERBURNER_H_
#define __EVESERVER_SHIPMODULES_ACTIVE_MODULES_AFTERBURNER_H_

#include "ship/modules/ActiveModule.h"

class Afterburner: public ActiveModule
{
public:
    Afterburner( InventoryItemRef item, ShipItemRef ship );
    virtual ~Afterburner()                              { /* do nothing here */ }

    /* GenericModule overrides */
    virtual void Activate(uint16 effectID, uint32 targetID=0, int16 repeat=0);
    virtual void DeactivateCycle(bool abort=false);

};

#endif  // __EVESERVER_SHIPMODULES_ACTIVE_MODULES_AFTERBURNER_H_
