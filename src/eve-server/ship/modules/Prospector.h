
 /**
  * @name Prospector.h
  *   prospector module class (salvage, hacking, data mining)
  * @Author:         Allan
  * @date:   11 August 2016   -UD/RW 12 April 2017  -UD/RN 10 Feburary 2018
  */

#ifndef _EVE_SHIP_MODULES_PROSPECTOR_MODULE_H_
#define _EVE_SHIP_MODULES_PROSPECTOR_MODULE_H_

#include "ship/modules/ActiveModule.h"


class Prospector: public ActiveModule
{
public:
    Prospector( InventoryItemRef item, ShipItemRef ship );
    virtual ~Prospector()                                 { /* do nothing here */ }

    /* ActiveModule overrides */
    virtual void Activate(uint16 effectID, uint32 targetID=0, int16 repeat=0);
    uint32 DoCycle();
    // this is a check for those active modules that need it (mining, weapons) and overridden as needed
    virtual bool CanActivate();

protected:
    void SendFailure();
    void CheckSuccess();
    void DropSalvage();
    void DropItems();

    bool m_success :1;
    bool m_firstRun :1;
    bool m_salvager :1;
    bool m_dataMiner :1;

    int8 m_accessChance;    // target chance (base chance)

    Character* pChar;
};

#endif  //_EVE_SHIP_MODULES_PROSPECTOR_MODULE_H_
