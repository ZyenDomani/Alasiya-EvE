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
    Updates:    Allan   (rewrite)
*/

#ifndef __EVESERVER_SHIPMODULES_GENERICMODULE_H_
#define __EVESERVER_SHIPMODULES_GENERICMODULE_H_

#include "EVEServerConfig.h"
#include "inventory/InventoryItem.h"
#include "ship/Ship.h"
#include "ship/modules/ModuleDefs.h"
#include "ship/modules/ModuleEffects.h"

class ModuleEffects;
class ModifyModuleAttributesComponent;
class ModifyShipAttributesComponent;

/* generic module base class */
class GenericModule
{
public:
    GenericModule( InventoryItemRef item, ShipItemRef ship );
    virtual ~GenericModule();

    /* generic functions handled in base class */
    void Offline();
    void Online();

    void Repair()                                       { m_Item->ResetAttribute(AttrHP, true); }
    void Repair(EvilNumber amount)                      { m_Item->SetAttribute(AttrHP, m_Item->GetAttribute(AttrHP) + amount); }

    bool HasAttribute(uint32 attrID)                    { return m_Item->HasAttribute(attrID); }
    void SetAttribute(uint32 attrID, EvilNumber val)    { m_Item->SetAttribute(attrID, val); }
    EvilNumber GetAttribute(uint32 attrID)              { return m_Item->GetAttribute(attrID); }

    void SetRepeat(int32 repeat)                        { m_repeat = repeat; }

    ShipItemRef GetShipRef()                            { return m_Ship; }

    /* class type helpers.  public for anyone to access. */
    virtual bool IsLoaded()                             { return false; }
    virtual bool IsGenericModule() const                { return true; }
    virtual bool IsPassiveModule() const                { return false; }
    virtual bool IsActiveModule() const                 { return false; }
    virtual bool IsRigModule() const                    { return false; }
    virtual bool IsSubSystemModule() const              { return false; }
    virtual bool IsTurrentModule()                      { return false; }

    /* generic access functions handled here, but set elsewhere.  slower than above */
    bool isOnline()                                     { return (m_Item->GetAttribute(AttrIsOnline) == 1); }
    bool isLowPower()                                   { return m_Effects->isLowSlot(); }
    bool isHighPower()                                  { return m_Effects->isHighSlot(); }
    bool isMediumPower()                                { return m_Effects->isMediumSlot(); }
    bool isRig()                                        { return m_Effects->isRig(); }
    bool isSubSystem()                                  { return m_Effects->isSubSystem(); }

    uint32 itemID()                                     { return m_Item->itemID(); }
    uint32 typeID()                                     { return m_Item->typeID(); }
    uint32 groupID()                                    { return m_Item->groupID(); }
    EVEItemFlags flag()                                 { return m_Item->flag(); }
    InventoryItemRef getItem()                          { return m_Item; }

	void SetModuleState(ModuleStates state)             { m_ModuleState = state; }
	ModuleStates GetModuleState()                       { return m_ModuleState; }
	ChargeStates GetChargeState()                       { return m_ChargeState; }

    /* functions to be handled in derived classes (must override) */
    virtual void Process()                              { /* Do nothing here */ }
    virtual void Activate(SystemEntity* targetEntity)   { /* Do nothing here */ }
    virtual void Deactivate()                           { /* Do nothing here */ }
    virtual void AbortCycle()                           { /* Do nothing here */ }
    virtual void Load(InventoryItemRef charge)          { /* Do nothing here */ }
    virtual void Unload()                               { /* Do nothing here */ }
    virtual void Overload()                             { /* Do nothing here */ }
    virtual void DeOverload()                           { /* Do nothing here */ }
    virtual void DestroyRig()                           { /* Do nothing here */ }

    /* functions to be overridden in derived classes as needed */
    virtual InventoryItemRef GetLoadedChargeRef()       { return InventoryItemRef(); }
    virtual bool isTurretFitted() {
        if( m_Effects->HasEffect(effectTurretFitted) )
            return true;
        return false;
    }

    virtual bool isLauncherFitted() {
        if( m_Effects->HasEffect(effectLauncherFitted) )
            return true;
        return false;
    }

    virtual bool isMaxGroupFitLimited()  {
        if( m_Item->HasAttribute(AttrMaxGroupFitted) )
            return true;
        return false;
    }

    /* override for rigs and subsystems in approprate derived class */
    virtual ModulePowerLevel GetModulePowerLevel() {
        return isHighPower() ? MODULE_BANK_HIGH_POWER
                : ( isMediumPower() ? MODULE_BANK_MEDIUM_POWER
                        : (isLowPower() ? MODULE_BANK_LOW_POWER
                            : (isRig() ? MODULE_BANK_RIG
                                : (isSubSystem() ? MODULE_BANK_SUBSYSTEM
                                    : MODULE_BANK_UNDEFINED ))));
    }

	/*  these have to be public for ampc/msac/mmac to access it's methods */
    ModuleEffects*                  m_Effects;          /* we own this */
    ModifyModuleAttributesComponent*  m_MMAC;           /* we own this */
    ModifyShipAttributesComponent*  m_MSAC;             /* we own this */

protected:
    InventoryItemRef                m_Item;
    ShipItemRef                     m_Ship;

    ModuleStates                    m_ModuleState;
    ChargeStates                    m_ChargeState;

    int32                           m_repeat;

    /*  this is for pre-calculated values, to eliminate previous code calculating on EVERY CALL.
     * defined in WeaponModule code.
     * not used in ActiveModule or PassiveModule.
     * put here to access using GenericModule.
     */
    virtual void _UpdateModifiers(InventoryItemRef item){ /* Do nothing here */ }
    virtual void _RemoveModifier(InventoryItemRef item) { /* Do nothing here */ }


};

#endif /* __EVESERVER_SHIPMODULES_GENERICMODULE_H_ */
