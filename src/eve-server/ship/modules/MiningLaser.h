
 /**
  * @name MiningModule.h
  *   mining module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */


#ifndef __EVESERVER_SHIPMODULES_ACTIVE_MODULES_MININGLASER_H__
#define __EVESERVER_SHIPMODULES_ACTIVE_MODULES_MININGLASER_H__

#include "ship/modules/ActiveModule.h"


class MiningLaser: public ActiveModule
{
public:
    MiningLaser(ModuleItemRef mRef, ShipItemRef sRef);
    virtual ~MiningLaser()                              { /* do nothing here */ }

    /* GenericModule overrides */
    virtual void DeactivateCycle(bool abort=false);

    /* ActiveModule overrides */
    virtual uint32 DoCycle();
    //virtual void Activate(uint16 effectID, uint32 targetID=0, int16 repeat=0);

    //  functions to be handled in derived classes as needed
    virtual void LoadCharge(InventoryItemRef charge);
    virtual void UnloadCharge();
    virtual bool CanActivate();

protected:
	void ProcessCycle(bool abort=false);


private:
    //cached item-type stuff
    bool m_rMiner, m_dcMiner, m_iMiner, m_gMiner;
    bool m_IsInitialCycle;

    float m_crystalDmg;
    float m_crystalDmgAmount;
    float m_crystalDmgChance;

    uint16 m_crystalRoidGrp;

    EVEItemFlags m_holdFlag;
};

#endif  // __EVESERVER_SHIPMODULES_ACTIVE_MODULES_MININGLASER_H__
