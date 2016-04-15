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


#include "eve-server.h"

#include "ship/modules/ActiveModule.h"

ActiveModule::ActiveModule(InventoryItemRef item, ShipRef ship)
: GenericModule(item, ship)
{
    m_AMPC = new ActiveModuleProcessingComponent(item, this, ship);
    m_chargeRef = InventoryItemRef();
    m_overLoaded = false;
    m_chargeLoaded = false;
}

ActiveModule::~ActiveModule()
{
    SafeDelete(m_AMPC);
}

void ActiveModule::Process()
{
    m_AMPC->Process();
}

void ActiveModule::Activate(SystemEntity* targetEntity)
{
    m_targetEntity = targetEntity;
    m_AMPC->ActivateCycle();

}

void ActiveModule::Deactivate()
{
    if ((m_ModuleState != MOD_ACTIVATED) or (m_ModuleState == MOD_PASSIVE) or (m_ModuleState == MOD_UNFITTED)) return;

    m_ModuleState = MOD_DEACTIVATING;
    m_AMPC->StopCycle();
}

    /** @todo  Overload and DeOverload will need to check for running module,
     * and if so, cancel that run, then restart with overloaded settings.
     * if not running, start with overloaded settings.
     */
void ActiveModule::Overload()
{
    GenericModule::Overload();
    m_ModuleState = MOD_OVERLOADED;
}

void ActiveModule::DeOverload()
{
    GenericModule::DeOverload();
    m_ModuleState = MOD_ACTIVATED;
}

void ActiveModule::Load(InventoryItemRef charge)
{
	m_chargeRef = charge;
    m_chargeLoaded = true;
    m_ChargeState = MOD_LOADED;
}

void ActiveModule::Unload()
{
	m_chargeRef = InventoryItemRef();		// Ensure ref is NULL
    m_chargeLoaded = false;
    m_ChargeState = MOD_UNLOADED;
}

double ActiveModule::DoCycle()
{
    if (m_Ship->GetOperator()->GetSystemEntity()->Bubble()) {
        _ShowCycle();
        return _GetDuration();
    }
    Deactivate();
    return 0;
}

bool ActiveModule::RequiresTarget()
{
    if (m_Effects->HasDefaultEffect())
        return (m_Effects->GetDefaultEffect()->GetIsAssistance() or m_Effects->GetDefaultEffect()->GetIsOffensive());
    else
        return false;
}

void ActiveModule::DoEffect(std::string effect, bool active)
{
    // Create Special Effect:
    m_Ship->GetOperator()->GetDestiny()->SendSpecialEffect
    (
        m_Ship,
     m_Item->itemID(),
     m_Item->typeID(),
     0,
     0,
     "effects.ModifyArmorResonance",
     0,
     1,
     1,
     _GetDuration(),
     1
    );

    // Create Destiny Updates:
    //  there are slight variations on this.  look into and fix as required.
    GodmaOther go;
        go.shipID = m_Ship->itemID();
        go.slotID = m_Item->flag();
        go.chargeTypeID = 0;

    GodmaEnvironment ge;
        ge.selfID = m_Item->itemID();
        ge.charID = m_Ship->ownerID();
        ge.shipID = go.shipID;
        ge.targetID = 0;
        ge.other = go.Encode();
        ge.area = new PyList;
        ge.effectID = effectModifyActiveArmorResonanceAndNullifyPassiveResonance;

    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = 1;
        shipEff.active = 1;
        shipEff.environment = ge.Encode();
        shipEff.startTime = shipEff.timeNow;
        shipEff.duration = _GetDuration();
        shipEff.repeat = 1;  //# times to repeat (should be ammo qty?)
        shipEff.error = new PyNone;

    std::vector<PyTuple*> events;
    events.push_back(shipEff.Encode());

    std::vector<PyTuple*> updates;

    m_Ship->GetOperator()->GetDestiny()->SendDestinyUpdate(updates, events, false);
}
