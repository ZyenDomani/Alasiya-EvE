
 /**
  * @name TurrentModule.h
  *   turrent module helper class
  * @Author:         Allan
  * @date:   10 June 2015
  */


#ifndef EVE_SHIP_MODULES_TURRENTMODULE_H
#define EVE_SHIP_MODULES_TURRENTMODULE_H

#include "ship/modules/ActiveModule.h"


class TurrentModule : public ActiveModule
{
public:
    TurrentModule(InventoryItemRef item, ShipItemRef shipRef);
    virtual ~TurrentModule()                                { /* do nothing here */ }

    //  class type helpers.  public for anyone to access.
    virtual bool IsTurrentModule()                          { return true; }

    //  functions to be handled in derived classes as needed
    virtual void LoadCharge(InventoryItemRef charge);
    virtual void UnloadCharge();

protected:
    //  these are  pre-calculated values to eliminate code calculating on every call
    float m_timerTime;

    double m_kinetic;
    double m_thermal;
    double m_em;
    double m_explosive;
};


#endif  // EVE_SHIP_MODULES_TURRENTMODULE_H