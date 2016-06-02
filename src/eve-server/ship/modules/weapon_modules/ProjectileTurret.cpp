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

#include "ship/Ship.h"
#include "ship/modules/weapon_modules/ProjectileTurret.h"
#include "system/Damage.h"


ProjectileTurret::ProjectileTurret( InventoryItemRef item, ShipItemRef shipRef )
: TurrentModule(item, shipRef)
{
    Character* pChar = m_Ship->GetPilot()->GetChar().get();

    switch (GetAttribute(AttrChargeSize).get_int()) {
        case 1:
            m_damageModifier *= (1 + ( 0.05 * (pChar->GetSkillLevel(skillSmallProjectileTurret, true)))); // 5% increase in damage
            break;
        case 2:
            m_damageModifier *= (1 + ( 0.05 * (pChar->GetSkillLevel(skillMediumProjectileTurret, true)))); // 5% increase in damage
            break;
        case 3:
            m_damageModifier *= (1 + ( 0.05 * (pChar->GetSkillLevel(skillLargeProjectileTurret, true)))); // 5% increase in damage
            break;
        case 4:
            m_damageModifier *= (1 + ( 0.05 * (pChar->GetSkillLevel(skillCapitalProjectileTurret, true)))); // 5% increase in damage
            break;
    }
    //specialization bonuses for tech 2
    switch (m_Item->typeID()) {
        case 2873:   //  125mm Gatling AutoCannon II
        case 2881:   //  150mm Light AutoCannon II
        case 2889:   //  200mm AutoCannon II
            m_damageModifier *= (1 + ( 0.03 * (pChar->GetSkillLevel(skillSmallAutocannonSpecialization, true)))); // 3% increase in damage
            break;
        case 2905:   //  250mm Light Artillery Cannon II
        case 2977:   //  280mm Howitzer Artillery II
            m_damageModifier *= (1 + ( 0.02 * (pChar->GetSkillLevel(skillSmallArtillerySpecialization, true))));  // 2% increase in damage
            break;
        case 2937:   //  Dual 180mm AutoCannon II
        case 2897:   //  220mm Vulcan AutoCannon II
        case 2913:   //  425mm AutoCannon II
            m_damageModifier *= (1 + ( 0.03 * (pChar->GetSkillLevel(skillMediumAutocannonSpecialization, true)))); // 3% increase in damage
            break;
        case 2921:   //  650mm Artillery Cannon II
        case 2969:   //  720mm Howitzer Artillery II
            m_damageModifier *= (1 + ( 0.02 * (pChar->GetSkillLevel(skillMediumArtillerySpecialization, true)))); // 2% increase in damage
            break;
        case 2945:   //  Dual 425mm AutoCannon II
        case 2953:   //  Dual 650mm Repeating Artillery II
        case 2929:   //  800mm Repeating Artillery II
            m_damageModifier *= (1 + ( 0.03 * (pChar->GetSkillLevel(skillLargeAutocannonSpecialization, true)))); // 3% increase in damage
            break;
        case 2865:   //  1200mm Artillery Cannon II
        case 2961:   //  1400mm Howitzer Artillery II
            m_damageModifier *= (1 + ( 0.02 * (pChar->GetSkillLevel(skillLargeArtillerySpecialization, true)))); // 2% increase in damage
            break;
    }
}

void ProjectileTurret::Activate(SystemEntity * targetEntity)
{
	if (m_chargeRef) {
		m_targetEntity = targetEntity;
		m_targetID = targetEntity->GetID();
		m_AMPC->ActivateCycle();
	} else {
		sLog.Error( "ProjectileTurret::Activate()", "ERROR: Cannot find charge that is supposed to be loaded into this module!" );
		throw PyException( MakeCustomError( "ERROR!  Cannot find charge that is supposed to be loaded into this module!" ) );
    }
}

void ProjectileTurret::StopCycle(bool abort)
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
        ge.area = new PyList;
        ge.effectID = effectProjectileFired;
    uint32 timeLeft = m_AMPC->GetRemainingCycleTimeMS();
    timeLeft /= 100;
    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = 0;
        shipEff.active = 0;
        shipEff.environment = ge.Encode();
        shipEff.startTime = (shipEff.timeNow - (timeLeft * Win32Time_Second));
        shipEff.duration = timeLeft;
        shipEff.repeat = 0;
        shipEff.error = new PyNone;
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

double ProjectileTurret::DoCycle() {
        if ((!m_Ship->GetPilot()->GetShipSE()->SysBubble())
            || (!m_Ship->GetPilot()->GetShipSE()->SysBubble()->GetEntity(m_targetID))
            || (!m_chargeLoaded) || (!m_chargeRef) )
        {
            Deactivate();
            return 0;
        }
        if (!m_chargeRef->quantity()) {
            Unload();
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
                 m_formula.GetToHit(m_Ship, m_Item, m_targetEntity),
                 effectProjectileFired       // from EVEEffectID::
                 );

        d *= m_damageModifier;
        if (sConfig.rates.turrentRate != 1.0)
            d *= sConfig.rates.turrentRate;
        m_targetEntity->ApplyDamage(d);

        // Reduce ammo charge by 1 unit:
        m_chargeRef->SetQuantity(m_chargeRef->quantity() - 1);

        return _GetROF();
}

void ProjectileTurret::_ShowCycle()
{
    // Create Special Effect:
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
    (
        m_Ship,
        m_Item->itemID(),
        m_Item->typeID(),
        m_targetID,
        m_chargeRef->typeID(),
        "effects.ProjectileFired",
        true,
        true,
        true,
        _GetROF(),
        1
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
        ge.area = new PyList;
        ge.effectID = effectProjectileFired;
    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = 1;
        shipEff.active = 1;
        shipEff.environment = ge.Encode();
        shipEff.startTime = shipEff.timeNow;
        shipEff.duration = _GetROF();
        shipEff.repeat = m_chargeRef->quantity();
        shipEff.error = new PyNone;
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

double ProjectileTurret::_GetROF() {
    return m_ROF;
}

void ProjectileTurret::_SetCapNeed()
{
    // this will be needed for modules and rigs that affect cap need for mining modules
    //double need = GetAttribute(AttrCapacitorNeed);

}
