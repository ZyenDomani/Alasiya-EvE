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

#include "ship/modules/weapon_modules/HybridTurret.h"
#include "system/Damage.h"


HybridTurret::HybridTurret( InventoryItemRef item, ShipItemRef ship )
: TurrentModule(item, ship)
{
    Character* pChar = m_Ship->GetPilot()->GetChar().get();

    switch (GetAttribute(AttrChargeSize).get_int()) {
        case 1:
            m_damageModifier *= (1 + ( 0.05 * (pChar->GetSkillLevel(skillSmallHybridTurret, true)))); // 5% increase in damage
            break;
        case 2:
            m_damageModifier *= (1 + ( 0.05 * (pChar->GetSkillLevel(skillMediumHybridTurret, true)))); // 5% increase in damage
            break;
        case 3:
            m_damageModifier *= (1 + ( 0.05 * (pChar->GetSkillLevel(skillLargeHybridTurret, true)))); // 5% increase in damage
            break;
        case 4:
            m_damageModifier *= (1 + ( 0.05 * (pChar->GetSkillLevel(skillCapitalHybridTurret, true)))); // 5% increase in damage
            break;
    }
    //specialization bonuses for tech 2
    switch (m_Item->typeID()) {
        case 3098:    //  75mm Gatling Rail II
        case 10680:   //  125mm Railgun II
        case 3074:    //  150mm Railgun II
            m_damageModifier *= (1 + ( 0.02 * (pChar->GetSkillLevel(skillSmallRailgunSpecialization, true)))); // 2% increase in damage
            break;
        case 3162:    //  Light Electron Blaster II
        case 3170:    //  Light Ion Blaster II
        case 3178:    //  Light Neutron Blaster II
            m_damageModifier *= (1 + ( 0.02 * (pChar->GetSkillLevel(skillSmallBlasterSpecialization, true)))); // 2% increase in damage
            break;
        case 12346:   //  200mm Railgun II
        case 3082:    //  250mm Railgun II
        case 3106:    //  Dual 150mm Railgun II
            m_damageModifier *= (1 + ( 0.02 * (pChar->GetSkillLevel(skillMediumRailgunSpecialization, true))));  // 2% increase in damage
            break;
        case 3130:    //  Heavy Electron Blaster II
        case 3138:    //  Heavy Ion Blaster II
        case 3146:    //  Heavy Neutron Blaster II
            m_damageModifier *= (1 + ( 0.02 * (pChar->GetSkillLevel(skillMediumBlasterSpecialization, true)))); // 2% increase in damage
            break;
        case 12356:   //  350mm Railgun II
        case 3090:    //  425mm Railgun II
        case 19958:   //  425mm Railgun III
        case 3114:    //  Dual 250mm Railgun II
            m_damageModifier *= (1 + ( 0.02 * (pChar->GetSkillLevel(skillLargeRailgunSpecialization, true)))); // 2% increase in damage
            break;
        case 3122:    //  Electron Blaster Cannon II
        case 3154:    //  Ion Blaster Cannon II
        case 3186:    //  Neutron Blaster Cannon II
            m_damageModifier *= (1 + ( 0.02 * (pChar->GetSkillLevel(skillLargeBlasterSpecialization, true)))); // 2% increase in damage
            break;
    }

    m_Item->SetAttribute(AttrDamageMultiplier, m_damageModifier);
}

void HybridTurret::Activate(SystemEntity* pSE)
{
	if( this->m_chargeRef )	{
        m_targetEntity = pSE;
        m_targetID = pSE->GetID();
		m_AMPC->ActivateCycle();
    } else {
        _log(SHIP__MODULE_WARNING, "HybridTurret::Activate() - Cannot find loaded charge for this module");
        if (m_Ship->HasPilot())
            if (m_Ship->GetPilot()->CanThrow())
                throw PyException( MakeCustomError( "Cannot find loaded charge for this module  - Ref: ServerError 15693"));
    }
}

void HybridTurret::StopCycle(bool abort)
{
    // Create Destiny Updates:
    GodmaOther go;
        go.shipID = m_Ship->itemID();
        go.slotID = m_Item->flag();
        if (m_chargeRef)
            go.chargeTypeID = m_chargeRef->typeID();
        else
            go.chargeTypeID = 0;
    GodmaEnvironment ge;
        ge.selfID = m_Item->itemID();
        ge.charID = m_Ship->ownerID();
        ge.shipID = go.shipID;
        ge.targetID = m_targetID;
        ge.other = go.Encode();
        ge.area = new PyList();
        ge.effectID = effectProjectileFired;
    uint32 timeLeft = m_AMPC->GetRemainingCycleTimeMS();
    timeLeft /= 1000;
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
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

double HybridTurret::DoCycle() {
        if ((!m_Ship->GetPilot()->GetShipSE()->SysBubble())
            || (!m_Ship->GetPilot()->GetShipSE()->SysBubble()->GetEntity(m_targetID))
            || (!m_chargeLoaded) || (!m_chargeRef) )
        {
            Deactivate();
            return 0;
        }
        if (!m_chargeRef->quantity()) {
            UnloadCharge();
            Deactivate();
            return 0;
        }

		_ShowCycle();

        Damage d(m_Ship->GetPilot()->GetShipSE(),
                 m_Item,
                 m_kinetic,
                 m_thermal,
                 m_em,
                 m_explosive,
                 m_formula.GetToHit(m_Ship, this, m_targetEntity),
                 effectProjectileFired       // from EVEEffectID::
                );

        d *= m_damageModifier;
        if (sConfig.rates.turrentRate != 1.0)
            d *= sConfig.rates.turrentRate;
		m_targetEntity->ApplyDamage(d);

		// Reduce ammo charge by 1 unit:
		m_chargeRef->SetQuantity(m_chargeRef->quantity() - 1);

        return m_cycleTime;
}

void HybridTurret::_ShowCycle()
{
    // Create Special Effect:
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
    (
        m_Ship,
        m_Item->itemID(),
        m_Item->typeID(),
        m_targetID,
        m_chargeRef->typeID(),
        "effects.HybridFired",
        true,
        true,
        true,
        m_cycleTime,
        0
    );

    // Create Destiny Updates:
    GodmaOther go;
        go.shipID = m_Ship->itemID();
        go.slotID = m_Item->flag();
        go.chargeTypeID = m_chargeRef->typeID();
    GodmaEnvironment ge;
        ge.selfID = m_Item->itemID();
        ge.charID = m_Ship->ownerID();
        ge.shipID = go.shipID;
        ge.targetID = m_targetID;
        ge.other = go.Encode();
        ge.area = new PyList();
        ge.effectID = effectProjectileFired;
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
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

void HybridTurret::_SetCapNeed()
{
    // this will be needed for modules and rigs that affect cap need for mining modules
    //double need = GetAttribute(AttrCapacitorNeed);

}
