
 /**
  * @name TurrentModule.h
  *   turrent module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */


#ifndef __EVESERVER_SHIPMODULES_ACTIVE_MODULES_TURRENTMODULE_H
#define __EVESERVER_SHIPMODULES_ACTIVE_MODULES_TURRENTMODULE_H

#include "ship/modules/ActiveModule.h"
#include "ship/modules/TurrentFormulas.h"


class TurrentModule : public ActiveModule
{
public:
    TurrentModule(InventoryItemRef item, ShipItemRef shipRef);
    virtual ~TurrentModule()                            { /* do nothing here */ }

    //  class type helpers.  public for anyone to access.
    virtual bool IsTurrentModule()                      { return true; }

    /* ActiveModule overrides */
    virtual void LoadCharge(InventoryItemRef charge);
    virtual void UnloadCharge();

    //  functions to be handled in derived classes as needed
    virtual void ApplyDamage();

protected:
    TurrentFormulas m_formula;

    float m_crystalDmg;
    float m_crystalDmgAmount;
    float m_crystalDmgChance;

};


#endif  // __EVESERVER_SHIPMODULES_ACTIVE_MODULES_TURRENTMODULE_H