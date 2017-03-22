/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Luck
    Updates:    Allan
*/

#ifndef ACTIVE_MODULES_H
#define ACTIVE_MODULES_H

#include "Client.h"
#include "ship/modules/GenericModule.h"
#include "system/SystemBubble.h"


class ActiveModule : public GenericModule
{
public:
    ActiveModule(InventoryItemRef item, ShipItemRef ship);
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
    virtual void Activate(uint16 effectID, uint32 targetID=0, int16 repeat=0);

    /* GenericModule access function overriders */
    virtual bool IsLoaded()                             { return m_chargeLoaded; }
    virtual bool IsOverloaded()                         { return m_overLoaded; }
    virtual InventoryItemRef GetLoadedChargeRef()       { return m_chargeRef; }

    // generic *Cycle() for active modules that only affect ship on Activate/Deactivate (not recurring on each cycle)
    //  for modules that perform action on each DoCycle(), they will override this call in their class implementation
    uint32 DoCycle();
    virtual void StopCycle(bool abort=false)            { /* Do nothing here */ }

    /* ActiveModule methods */
    bool ShipHasCapCharge();
    uint32 GetTargetID()                                { return m_targetID; }
    SystemEntity* GetTarget()                           { return m_targetEntity; }

    // public methods to enable calls from other classes (namely, TurrentFormulas.cpp)
    uint32 GetFalloff()                                 { return m_falloff; }
    uint32 GetMaxRange()                                { return m_maxRange; }
    uint32 GetSigRadius()                               { return m_optimalSigRadius; }
    double GetTrackingSpeed()                           { return m_trackingSpeed; }

    /* new effects processing code and updates */
    void ApplyEffect(Effects::State state, bool active=false);
	/* common method for all modules that have a visual effect when active (wip) */
    void ShowEffect(bool active=false, bool abort=false);

protected:
	bool m_overLoaded = false;
    bool m_chargeLoaded = false;

    void DeactivateCycle(bool abort=false);
    void ShouldProcessActiveCycle();

    uint32 GetRemainingCycleTimeMS()                    { return m_timer.GetRemainingTime(); }

    void SetTimer(uint32 time);
    void StopTimer()                                    { m_timer.Disable(); }

    /* skill, charge, and module combined modifiers to avoid constant calculations. */
    // may no longer need these....pull current attrib from item, as effects are working now, and modify items attribs directly.
    uint32 m_falloff;                               // distance past maximum range at which accuracy has fallen by half
    uint32 m_optimalRange;
    uint32 m_maxRange;
    uint32 m_optimalSigRadius;
    double m_capNeed;
    double m_cycleTime;
    double m_rangeModifier;
    double m_damageModifier;
    double m_trackingSpeed;

private:
    Timer m_reloadTimer;
    //SystemBubble* m_bubble;                           // we do not own this
    SystemEntity* m_targetEntity;                       // we do not own this
    //DestinyManager* m_destiny;                        // we do not own this

    uint16 m_effectID;                                  //passed to us by activate
    uint32 m_targetID;                                  //passed to us by activate
    uint16 m_reloadTime;
    std::string m_guidStr;

    Timer m_timer;

    bool m_Stop;

};


#endif