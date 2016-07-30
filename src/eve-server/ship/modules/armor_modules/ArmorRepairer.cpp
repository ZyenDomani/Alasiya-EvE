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
    Author:        AknorJaden
    Updates:    Allan
*/

#include "character/Character.h"
#include "system/SystemBubble.h"
#include "ship/Ship.h"
#include "ship/modules/armor_modules/ArmorRepairer.h"


ArmorRepairer::ArmorRepairer( InventoryItemRef item, ShipItemRef ship )
: ActiveModule(item, ship)
{

}

void ArmorRepairer::StopCycle(bool abort)
{
    // Create Destiny Updates:
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
        ge.effectID = effectArmorRepair;
    uint32 timeLeft = m_AMPC->GetRemainingCycleTimeMS();
    timeLeft /= 100;
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
        shipEff.error = new PyNone;
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

double ArmorRepairer::DoCycle()
{
        if (!m_Ship->GetPilot()->GetShipSE()->SysBubble())
        {
            Deactivate();
            return 0;
        }
        _ShowCycle();

		// Apply repair amount:
        EvilNumber newDamageAmount = m_Ship->GetAttribute(AttrArmorDamage);
        newDamageAmount -= m_Item->GetAttribute(AttrArmorDamageAmount);
        if (newDamageAmount < 0) {
            m_Ship->SetAttribute(AttrArmorDamage, 0);
            Deactivate();
        } else
            m_Ship->SetAttribute(AttrArmorDamage, newDamageAmount);

        return _GetDuration();
}

void ArmorRepairer::_ShowCycle()
{
    // Create Special Effect:
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
    (
        m_Ship,
     m_Item->itemID(),
     m_Item->typeID(),
     0,
     0,
     "effects.ArmorRepair",
     0,
     1,
     1,
     _GetDuration(),
     1
    );

    // Create Destiny Updates:
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
        ge.effectID = effectArmorRepair;
    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = 1;
        shipEff.active = 1;
        shipEff.environment = ge.Encode();
        shipEff.startTime = shipEff.timeNow;
        shipEff.duration = _GetDuration();
        shipEff.repeat = m_repeat;
        shipEff.error = new PyNone;
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

double ArmorRepairer::_GetDuration()
{
    Character* pChar = m_Ship->GetPilot()->GetChar().get();
    double duration = m_Item->GetAttribute(AttrDuration).get_float();
    duration *= (1 - ( 0.05 * (pChar->GetSkillLevel(skillRepairSystems, true))));      //  5% decrease in cycle time

    if (IsOverloaded())
        duration *= (1 + m_Item->GetAttribute(AttrOverloadDurationBonus).get_float());

    return duration;
}

void ArmorRepairer::_SetCapNeed()
{
    // this will be needed for modules and rigs that affect cap need for mining modules
    //double need = GetAttribute(AttrCapacitorNeed);

}
