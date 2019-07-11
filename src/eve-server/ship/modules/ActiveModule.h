
 /**
  * @name ActiveModule.h
  *   active module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */


#ifndef _EVE_SHIP_MODULES_ACTIVE_MODULE_H_
#define _EVE_SHIP_MODULES_ACTIVE_MODULE_H_

#include "Client.h"
#include "ship/modules/GenericModule.h"
#include "system/SystemBubble.h"


class ActiveModule : public GenericModule
{
public:
    ActiveModule(ModuleItemRef mRef, ShipItemRef sRef);
    virtual ~ActiveModule()                             { /* Do nothing here */ }

    /* class type helpers.  public for anyone to access. */
    virtual bool IsActiveModule() const                 { return true; }

    /* GenericModule overrides */
	virtual void Process();
    virtual void LoadCharge(InventoryItemRef charge);
    virtual void UnloadCharge();
    virtual void Overload();
    virtual void AbortCycle();
    virtual void DeOverload();
    virtual void Deactivate(std::string effect="");
    virtual void DeactivateCycle(bool abort=false);
    virtual void Activate(uint16 effectID, uint32 targetID=0, int16 repeat=0);

    /* generic DoCycle() for active modules that only affect ship on Activate/Deactivate (not recurring on each cycle)
     *  for modules that perform action on each DoCycle(), they will override this call in their class implementation
     */
    virtual uint32 DoCycle();

    /* functions to be handled in derived classes as needed */
    virtual void ApplyDamage()                          { /* do nothing here */ }
    virtual uint16 GetReloadTime()                      { return m_reloadTime; }
    // this is a check for those active modules that need it (mining, weapons) and overridden as needed
    virtual bool CanActivate();
    /* apply charge effects when undocking, as they are reset and not called anywhere else */
    virtual void ReprocessCharge();

    /* ActiveModule methods */
    virtual uint32 GetTargetID()                        { return m_targetID; }
    SystemEntity* GetTarget()                           { return m_targetSE; }

    void LaunchProbe();
    void LaunchMissile();
    void LaunchSnowBall();

    /* new effects processing code and updates */
    void ApplyEffect(int8 state, bool active=false);
	/* common method for all modules that have a visual effect when active */
    void ShowEffect(bool active=false, bool abort=false);

protected:
    SystemBubble* m_bubble;                             // we do not own this
    SystemEntity* m_targetSE;                           // we do not own this
    DestinyManager* m_destinyMgr;                       // we do not own this
    TargetManager* m_targMgr;                           // we do not own this

    void Clear();
    void ProcessActiveCycle();                          // checks and sets cap use
    void UpdateCharge(uint16 attrID, uint16 testAttrID, uint16 srcAttrID, InventoryItemRef iRef);
    void UpdateDamage(uint16 attrID, uint16 srcAttrID, InventoryItemRef iRef);

    uint32 GetRemainingCycleTimeMS()                    { return m_timer.GetRemainingTime(); }

    void SetTimer(uint32 time);
    void StopTimer()                                    { m_timer.Disable(); }

    uint16 m_reloadTime;
    uint16 m_effectID;                                  //passed to us by activate
    uint32 m_targetID;                                  //passed to us by activate

    // protected to allow derived usage
    bool m_needsCharge :1;
    bool m_needsTarget :1;

private:
    Timer m_timer;
    Timer m_reloadTimer;

    bool m_Stop :1;

    std::string m_guidStr;

};


#endif  // _EVE_SHIP_MODULES_ACTIVE_MODULE_H_
