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
    Author:        Reve, AknorJaden
    Updates:    Allan
*/

#include "ship/modules/weapon_modules/EnergyTurret.h"
#include "system/Damage.h"


EnergyTurret::EnergyTurret( InventoryItemRef item, ShipItemRef ship )
: TurrentModule(item, ship)
{
    Character* pChar = m_Ship->GetPilot()->GetChar().get();

    switch (GetAttribute(AttrChargeSize).get_int()) {
        case 1:
            m_damageModifier *= (1 + ( 0.05 * (pChar->GetSkillLevel(skillSmallEnergyTurret, true)))); // 5% increase in damage
            break;
        case 2:
            m_damageModifier *= (1 + ( 0.05 * (pChar->GetSkillLevel(skillMediumEnergyTurret, true)))); // 5% increase in damage
            break;
        case 3:
            m_damageModifier *= (1 + ( 0.05 * (pChar->GetSkillLevel(skillLargeEnergyTurret, true)))); // 5% increase in damage
            break;
        case 4:
            m_damageModifier *= (1 + ( 0.05 * (pChar->GetSkillLevel(skillCapitalEnergyTurret, true)))); // 5% increase in damage
            break;
    }
    //specialization bonuses for tech 2
    switch (m_Item->typeID()) {
        case 2993:    //  Dual Light Beam Laser II
        case 3033:    //  Medium Beam Laser II
            m_damageModifier *= (1 + ( 0.02 * (pChar->GetSkillLevel(skillSmallPulseLaserSpecialization, true)))); // 2% increase in damage
            break;
        case 3001:    //  Dual Light Pulse Laser II
        case 3041:    //  Medium Pulse Laser II
        case 3017:    //  Gatling Pulse Laser II
            m_damageModifier *= (1 + ( 0.02 * (pChar->GetSkillLevel(skillSmallBeamLaserSpecialization, true)))); // 2% increase in damage
            break;
        case 3520:    //  Heavy Pulse Laser II
        case 3512:    //  Focused Medium Pulse Laser II
            m_damageModifier *= (1 + ( 0.02 * (pChar->GetSkillLevel(skillMediumPulseLaserSpecialization, true)))); // 2% increase in damage
            break;
        case 3025:    //  Heavy Beam Laser II
        case 3009:    //  Focused Medium Beam Laser II
        case 3285:    //  Quad Light Beam Laser II
            m_damageModifier *= (1 + ( 0.02 * (pChar->GetSkillLevel(skillMediumBeamLaserSpecialization, true))));  // 2% increase in damage
            break;
        case 3057:    //  Mega Pulse Laser II
        case 4147:    //  Dual Heavy Pulse Laser II
            m_damageModifier *= (1 + ( 0.02 * (pChar->GetSkillLevel(skillLargePulseLaserSpecialization, true)))); // 2% increase in damage
            break;
        case 3049:    //  Mega Beam Laser II
        case 3065:    //  Tachyon Beam Laser II
        case 2985:    //  Dual Heavy Beam Laser II
            m_damageModifier *= (1 + ( 0.02 * (pChar->GetSkillLevel(skillLargeBeamLaserSpecialization, true)))); // 2% increase in damage
            break;
    }
}

void EnergyTurret::Activate(SystemEntity* targetEntity)
{
	if (this->m_chargeRef) {
		m_targetEntity = targetEntity;
		m_targetID = targetEntity->GetID();
		m_AMPC->ActivateCycle();
	} else {
		sLog.Error( "EnergyTurret::Activate()", "ERROR: Cannot find charge that is supposed to be loaded into this module!" );
		throw PyException( MakeCustomError( "ERROR!  Cannot find charge that is supposed to be loaded into this module!" ) );
    }
}

void EnergyTurret::StopCycle()
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
        ge.effectID = effectTargetAttack;
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

double EnergyTurret::DoCycle() {
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
                 effectTargetAttack       // from EVEEffectID::
                );

        d *= m_damageModifier;
        if (sConfig.rates.turrentRate != 1.0)
            d *= sConfig.rates.turrentRate;
		m_targetEntity->ApplyDamage(d);
        // doesnt the crystals have heat damage?

        return _GetROF();
}

void EnergyTurret::_ShowCycle()
{
    // Create Special Effect:
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
    (
        m_Ship,
        m_Item->itemID(),
        m_Item->typeID(),
        m_targetID,
        m_chargeRef->typeID(),
        "effects.Laser",
        1,
        1,
        1,
        _GetROF(),
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
        ge.area = new PyList;
        ge.effectID = effectTargetAttack;
    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = 1;
        shipEff.active = 1;
        shipEff.environment = ge.Encode();
        shipEff.startTime = shipEff.timeNow;
        shipEff.duration = _GetROF();
        shipEff.repeat = 1000;
        shipEff.error = new PyNone;
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

double EnergyTurret::_GetROF() {
    return m_ROF;
}

void EnergyTurret::_SetCapNeed()
{
    // this will be needed for modules and rigs that affect cap need for mining modules
    //double need = GetAttribute(AttrCapacitorNeed);

}
