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

    void Repair()                                       { m_modRef->ResetAttribute(AttrHP, true); }
    void Repair(EvilNumber amount)                      { m_modRef->SetAttribute(AttrHP, m_modRef->GetAttribute(AttrHP) + amount); }

    bool HasAttribute(uint32 attrID)                    { return m_modRef->HasAttribute(attrID); }
    void SetAttribute(uint32 attrID, EvilNumber val)    { m_modRef->SetAttribute(attrID, val); }
    void ResetAttribute(uint32 attrID)                  { m_modRef->ResetAttribute(attrID); }
    EvilNumber GetAttribute(uint32 attrID)              { return m_modRef->GetAttribute(attrID); }

    void SetRepeat(int32 repeat)                        { m_repeat = repeat; }

    /* class type helpers.  public for anyone to access. */                 /** @todo  update these below as noted */
    virtual bool IsWarpSafe() const                     { return true; }    // check this in module effect data.
    virtual bool IsLoaded()                             { return false; }
    virtual bool IsGenericModule() const                { return true; }
    virtual bool IsPassiveModule() const                { return false; }
    virtual bool IsActiveModule() const                 { return false; }
    virtual bool IsRigModule() const                    { return false; }   // check this in m_rigSlot?
    virtual bool IsSubSystemModule() const              { return false; }   // check this in m_subSystem?
    virtual bool IsTurrentModule()                      { return false; }   // check this in module effect data.
    virtual bool IsLauncherModule()                     { return false; }   // check this in module effect data.

    /* generic access functions handled here, but set elsewhere.  only slightly slower than above */
    bool isOnline()                                     { return (m_modRef->GetAttribute(AttrIsOnline) == 1); }
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

	void SetModuleState(ModuleStates state)             { m_ModuleState = state; }
	ModuleStates GetModuleState()                       { return m_ModuleState; }
	ChargeStates GetChargeState()                       { return m_ChargeState; }

    /* functions to be handled in derived classes (must override) */
    virtual void Process()                              { /* Do nothing here */ }
    virtual void Activate(SystemEntity* pSE)            { /* Do nothing here */ }
    virtual void Deactivate()                           { /* Do nothing here */ }
    virtual void AbortCycle()                           { /* Do nothing here */ }
    virtual void LoadCharge(InventoryItemRef charge)    { /* Do nothing here */ }
    virtual void UnloadCharge()                         { /* Do nothing here */ }
    virtual void Overload()                             { /* Do nothing here */ }
    virtual void DeOverload()                           { /* Do nothing here */ }
    virtual void DestroyRig()                           { /* Do nothing here */ }

    /* functions to be overridden in derived classes as needed */
    virtual bool isTurretFitted()                       { return (m_modRef->type().HasEffect(effectTurretFitted) ? true : false); false; }
    virtual bool isLauncherFitted()                     { return (m_modRef->type().HasEffect(effectLauncherFitted) ? true : false); false; }
    virtual bool isMaxGroupFitLimited()                 { return (m_modRef->type().HasEffect(AttrMaxGroupFitted) ? true : false); false; }

    virtual InventoryItemRef GetLoadedChargeRef()       { return InventoryItemRef(); }

    /* override for rigs and subsystems in approprate derived class */
    virtual ModulePowerLevel GetModulePowerLevel() {
        return m_hiPower ? MODULE_BANK_HIGH_POWER
                : ( m_medPower ? MODULE_BANK_MEDIUM_POWER
                    : (m_loPower ? MODULE_BANK_LOW_POWER
                        : (m_rigSlot ? MODULE_BANK_RIG
                            : (m_subSystem ? MODULE_BANK_SUBSYSTEM
                                : MODULE_BANK_UNDEFINED ))));
    }

protected:
    InventoryItemRef                m_modRef;
    ShipItemRef                     m_shipRef;

    ModuleStates                    m_ModuleState;
    ChargeStates                    m_ChargeState;

    bool                            m_hiPower;
    bool                            m_medPower;
    bool                            m_loPower;
    bool                            m_rigSlot;
    bool                            m_subSystem;
    bool                            m_warpSafe;
    bool                            m_targReq;

    int32                           m_repeat;

    void ModifyShipAttribute(uint16 targetAttrID, uint16 sourceAttrID, Effects::Math type, bool stacking);
    void ModifyModuleAttribute(GenericModule* targetMod, uint32 targetAttrID, uint32 sourceAttrID, Effects::Math type);
    void ModifyTargetAttribute(uint32 targetItemID, uint16 targetAttrID, uint16 sourceAttrID, Effects::Math type, bool stacking);

};

#endif /* __EVESERVER_SHIPMODULES_GENERICMODULE_H_ */
