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
    Author:     Allan
*/

#include "ship/modules/hull_modules/HullRepairer.h"
#include "character/Character.h"


HullRepairer::HullRepairer( InventoryItemRef item, ShipItemRef ship )
: ActiveModule(item, ship)
{
    Character* pChar = m_shipRef->GetPilot()->GetChar().get();
    m_cycleTime *= (1 - ( 0.05 * (pChar->GetSkillLevel(skillRepairSystems, true))));      //  5% decrease in cycle time
    m_modRef->SetAttribute(AttrDuration, m_cycleTime);

}

void HullRepairer::StopCycle(bool abort)
{
    uint32 timeLeft = GetRemainingCycleTimeMS();
    timeLeft /= 1000;
    m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
    (
        m_shipRef,
     m_modRef->itemID(),
     m_modRef->typeID(),
     0,
     0,
     "effects.StructureRepair",
     0,
     0,
     0,
     timeLeft,
     0
    );

    // Create Destiny Updates:
    GodmaOther go;
        go.shipID = m_shipRef->itemID();
        go.slotID = m_modRef->flag();
        go.chargeTypeID = 0;
    GodmaEnvironment ge;
        ge.selfID = m_modRef->itemID();
        ge.charID = m_shipRef->ownerID();
        ge.shipID = go.shipID;
        ge.targetID = m_targetID;
        ge.other = go.Encode();
        ge.area = new PyList;
        ge.effectID = effectStructureRepair;
    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = 0;
        shipEff.active = 0;
        shipEff.environment = ge.Encode();
        shipEff.startTime = (shipEff.timeNow + (timeLeft * Win32Time_Second));
        shipEff.duration = timeLeft;
        shipEff.repeat = 0;
        shipEff.error = new PyNone();
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

double HullRepairer::DoCycle()
{
        if (!m_shipRef->GetPilot()->GetShipSE()->SysBubble())
        {
            Deactivate();
            return 0;
        }
        _ShowCycle();

		// Apply repair amount:
        EvilNumber newDamageAmount = m_shipRef->GetAttribute(AttrDamage);
        newDamageAmount -= GetAttribute(AttrStructureDamageAmount);
        if (newDamageAmount < 0) {
            m_shipRef->SetAttribute(AttrDamage, 0);
            Deactivate();
        } else
            m_shipRef->SetAttribute(AttrDamage, newDamageAmount);

        return m_cycleTime;
}

void HullRepairer::_ShowCycle()
{
    // Create Special Effect:
    m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
    (
        m_shipRef,
     m_modRef->itemID(),
     m_modRef->typeID(),
     0,
     0,
     "effects.StructureRepair",
     0,
     1,
     1,
     m_cycleTime,
     1
    );

    // Create Destiny Updates:
    GodmaOther go;
        go.shipID = m_shipRef->itemID();
        go.slotID = m_modRef->flag();
        go.chargeTypeID = 0;
    GodmaEnvironment ge;
        ge.selfID = m_modRef->itemID();
        ge.charID = m_shipRef->ownerID();
        ge.shipID = go.shipID;
        ge.targetID = m_targetID;
        ge.other = go.Encode();
        ge.area = new PyList;
        ge.effectID = effectStructureRepair;
    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = 1;
        shipEff.active = 1;
        shipEff.environment = ge.Encode();
        shipEff.startTime = shipEff.timeNow;
        shipEff.duration = m_cycleTime;
        shipEff.repeat = m_repeat;
        shipEff.error = new PyNone();
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

void HullRepairer::_SetCapNeed()
{
    // this will be needed for modules and rigs that affect cap need for mining modules
    //double need = GetAttribute(AttrCapacitorNeed);

}
