
 /**
  * @name Salvager.h
  *   salvage module class
  * @Author:         Allan
  * @date:   11 August 2016   -UD/RW 12 April 2017
  */

#ifndef _EVE_SHIP_MOD_SALVAGER_H_
#define _EVE_SHIP_MOD_SALVAGER_H_

#include "ship/modules/ActiveModule.h"


class Salvager: public ActiveModule
{
public:
    Salvager( InventoryItemRef item, ShipItemRef ship );
    virtual ~Salvager()                                 { /* do nothing here */ }

    /* ActiveModule overrides */
    virtual void Activate(uint16 effectID, uint32 targetID=0, int16 repeat=0);
    uint32 DoCycle();
    // this is a check for those active modules that need it (mining, weapons) and overridden as needed
    virtual bool CanActivate();

protected:
    void SendFailure();
    void CheckSuccess();
    void DropSalvage();

    bool m_success;
    bool m_firstRun;

    int8 m_accessChance;

    Character* pChar;
};

#endif  //_EVE_SHIP_MOD_SALVAGER_H_
