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
#include "effects/EffectsProcessor.h"
#include "inventory/InventoryItem.h"
#include "ship/Ship.h"
#include "ship/modules/ModuleDefs.h"

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
    void Online();
    void Offline();

    void ProcessEffects(Effects::State state, bool online = false);

    void Repair()                                       { m_modRef->ResetAttribute(AttrHP, true); }
    void Repair(EvilNumber amount)                      { m_modRef->SetAttribute(AttrHP, m_modRef->GetAttribute(AttrHP) + amount); }

    bool HasAttribute(uint32 attrID)                    { return m_modRef->HasAttribute(attrID); }
    void SetAttribute(uint32 attrID, EvilNumber val)    { m_modRef->SetAttribute(attrID, val); }
    void ResetAttribute(uint32 attrID)                  { m_modRef->ResetAttribute(attrID); }
    EvilNumber GetAttribute(uint32 attrID)              { return m_modRef->GetAttribute(attrID); }

    void SetRepeat(int32 repeat)                        { m_repeat = repeat; }

    /* class type helpers.  public for anyone to access. */                 /** @todo  update these below as noted */
    virtual bool IsLoaded()                             { return false; }
    virtual bool IsGenericModule() const                { return true; }
    virtual bool IsPassiveModule() const                { return false; }
    virtual bool IsActiveModule() const                 { return false; }
    virtual bool IsRigModule() const                    { return false; }   // check this in m_rigSlot?
    virtual bool IsSubSystemModule() const              { return false; }   // check this in m_subSystem?
    virtual bool IsTurrentModule()                      { return false; }   // check this in module effect data.
    virtual bool IsLauncherModule()                     { return false; }   // check this in module effect data.

    /* generic access functions handled here, but set elsewhere.  only slightly slower than above */
    bool isOnline()                                     { return m_modRef->IsOnline(); }
    bool isLowPower()                                   { return m_loPower; }
    bool isHighPower()                                  { return m_hiPower; }
    bool isMediumPower()                                { return m_medPower; }
    bool isRig()                                        { return m_rigSlot; }
    bool isSubSystem()                                  { return m_subSystem; }

    uint32 itemID()                                     { return m_modRef->itemID(); }
    uint32 typeID()                                     { return m_modRef->typeID(); }
    uint32 groupID()                                    { return m_modRef->groupID(); }
    EVEItemFlags flag()                                 { return m_modRef->flag(); }
    InventoryItemRef getItem()                          { return m_modRef; }

    ShipItemRef GetShipRef()                            { return m_shipRef; }

    void SetChargeRef(InventoryItemRef iRef)            { m_chargeRef = iRef; }
    void SetModuleState(ModStates::ModuleStates state)  { m_ModuleState = state; }
    void SetChargeState(ModStates::ChargeStates state)  { m_ChargeState = state; }
    ModStates::ModuleStates GetModuleState()            { return m_ModuleState; }
    ModStates::ChargeStates GetChargeState()            { return m_ChargeState; }

	virtual bool isTurretFitted()                       { return (m_modRef->type().HasEffect(effectTurretFitted) ? true : false); false; }
	virtual bool isLauncherFitted()                     { return (m_modRef->type().HasEffect(effectLauncherFitted) ? true : false); false; }
	virtual bool isMaxGroupFitLimited()                 { return (m_modRef->type().HasEffect(AttrMaxGroupFitted) ? true : false); false; }

	/* generic access functions to be handled in derived classes (must override) */
    virtual void Process()                              { /* Do nothing here */ }
    virtual void Activate(SystemEntity* pSE, std::string effect="") { /* Do nothing here */ }
    virtual void Deactivate(std::string effect="")      { /* Do nothing here */ }
    virtual void AbortCycle()                           { /* Do nothing here */ }
    virtual void LoadCharge(InventoryItemRef charge)    { /* Do nothing here */ }
    virtual void UnloadCharge()                         { /* Do nothing here */ }
    virtual void DestroyRig()                           { /* Do nothing here */ }

    /* generic access functions to be overridden in derived classes as needed */
    virtual void Overload();
    virtual void DeOverload();
    virtual InventoryItemRef GetLoadedChargeRef()       { return InventoryItemRef(); }

    /* override for rigs and subsystems in approprate derived class */
    virtual ModStates::ModulePowerLevel GetModulePowerLevel() {
        return m_hiPower ? ModStates::MODULE_BANK_HIGH_POWER
                : ( m_medPower ? ModStates::MODULE_BANK_MEDIUM_POWER
                    : (m_loPower ? ModStates::MODULE_BANK_LOW_POWER
                        : (m_rigSlot ? ModStates::MODULE_BANK_RIG
                            : (m_subSystem ? ModStates::MODULE_BANK_SUBSYSTEM
                                : ModStates::MODULE_BANK_UNDEFINED ))));
    }

protected:
    InventoryItemRef m_modRef;
    ShipItemRef      m_shipRef;
    InventoryItemRef m_chargeRef;

    ModStates::ModuleStates     m_ModuleState;
    ModStates::ChargeStates     m_ChargeState;

    bool             m_hiPower;
    bool             m_medPower;
    bool             m_loPower;
    bool             m_rigSlot;
    bool             m_subSystem;
    bool             m_warpSafe;
    bool             m_targReq;

    int32            m_repeat;

    std::string GetModuleStateName(ModStates::ModuleStates state);
    std::string GetChargeStateName(ModStates::ChargeStates state);

};

#endif /* __EVESERVER_SHIPMODULES_GENERICMODULE_H_ */
