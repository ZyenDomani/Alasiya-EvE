/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2011 The EVEmu Team
 *    For the latest information visit http://evemu.org
 *    ------------------------------------------------------------------------------------
 *    This program is free software; you can redistribute it and/or modify it under
 *    the terms of the GNU Lesser General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option) any later
 *    version.
 *
 *    This program is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License along with
 *    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 *    http://www.gnu.org/copyleft/lesser.txt.
 *    ------------------------------------------------------------------------------------
 *    Author:     Allan
 */


#include "eve-server.h"

#include "character/Character.h"
#include "ship/modules/TurrentModule.h"

TurrentModule::TurrentModule(InventoryItemRef item, ShipItemRef shipRef)
: ActiveModule(item, shipRef)
{
    m_cycleTime = GetAttribute(AttrSpeed).get_float();
    m_falloff = GetAttribute(AttrFalloff).get_int();
    m_maxRange = GetAttribute(AttrMaxRange).get_int();
    m_capNeed = GetAttribute(AttrCapacitorNeed).get_float();
    m_trackingSpeed = GetAttribute(AttrTrackingSpeed).get_float();
    m_damageModifier = GetAttribute(AttrDamageMultiplier).get_float();
    m_optimalSigRadius = GetAttribute(AttrOptimalSigRadius).get_int();

    Character* pChar = m_Ship->GetPilot()->GetChar().get();
    m_cycleTime *= (1 - ( 0.02 * (pChar->GetSkillLevel(skillGunnery, true))));      //  2% increase in rof (lower cycle times)
    m_cycleTime *= (1 - ( 0.04 * (pChar->GetSkillLevel(skillRapidFiring, true))));  //  4% increase in rof
    m_capNeed *= (1 - ( 0.05 * (pChar->GetSkillLevel(skillControlledBursts, true))));  //  5% decrease in cap need
    m_damageModifier *= (1 + ( 0.05 * (pChar->GetSkillLevel(skillSurgicalStrike, true)))); // 5% increase in damage (upped from 3%)
    m_damageModifier *= (1 + ( 0.03 * (pChar->GetSkillLevel(skillWeaponUpgrades, true)))); // 3% increase in damage

    // Turrent Tracking data   - these may/may not be modified by loaded charge
    m_falloff *= (1 + ( 0.05 * (pChar->GetSkillLevel(skillTrajectoryAnalysis, true))));  //  5% increase in falloff
    m_maxRange *= (1 + ( 0.05 * (pChar->GetSkillLevel(skillSharpshooter, true))));      //  5% increase in optimal range
    m_trackingSpeed *= (1 + ( 0.05 * (pChar->GetSkillLevel(skillMotionPrediction, true)))); // 5% increase in tracking speed

    // add ship bonuses here

    // save adjusted attributes
    m_Item->SetAttribute(AttrSpeed, m_cycleTime);
    m_Item->SetAttribute(AttrCapacitorNeed, m_capNeed);
    m_Item->SetAttribute(AttrFalloff, m_falloff);
    m_Item->SetAttribute(AttrMaxRange, m_maxRange);
    m_Item->SetAttribute(AttrTrackingSpeed, m_trackingSpeed);
    m_Item->SetAttribute(AttrOptimalSigRadius, m_optimalSigRadius);
    //m_Item->SetAttribute(AttrDamageMultiplier, m_damageModifier);  set in individual module code
}

void TurrentModule::Overload()
{
    GenericModule::Overload();
    m_damageModifier *= (1 + (GetAttribute(AttrOverloadDamageModifier).get_float() /100));
    m_cycleTime *= (1 + GetAttribute(AttrOverloadRofBonus).get_float());
    m_Item->SetAttribute(AttrSpeed, m_cycleTime);
    m_Item->SetAttribute(AttrDamageMultiplier, m_damageModifier);
}

void TurrentModule::DeOverload()
{
    m_damageModifier /= (1 + (GetAttribute(AttrOverloadDamageModifier).get_float() /100));
    m_cycleTime /= (1 + GetAttribute(AttrOverloadRofBonus).get_float());
    GenericModule::DeOverload();
    m_Item->SetAttribute(AttrSpeed, m_cycleTime);
    m_Item->SetAttribute(AttrDamageMultiplier, m_damageModifier);
}

void TurrentModule::LoadCharge(InventoryItemRef charge)
{
    ActiveModule::LoadCharge(charge);
    m_kinetic       = m_chargeRef->GetAttribute(AttrKineticDamage).get_float();
    m_thermal       = m_chargeRef->GetAttribute(AttrThermalDamage).get_float();
    m_em            = m_chargeRef->GetAttribute(AttrEmDamage).get_float();
    m_explosive     = m_chargeRef->GetAttribute(AttrExplosiveDamage).get_float();

    if (m_chargeRef->HasAttribute(AttrWeaponRangeMultiplier)) {
        m_falloff *= m_chargeRef->GetAttribute(AttrWeaponRangeMultiplier).get_float();
        m_maxRange *= m_chargeRef->GetAttribute(AttrWeaponRangeMultiplier).get_float();
    }
    if (m_chargeRef->HasAttribute(AttrDamageMultiplier))
        m_damageModifier = m_chargeRef->GetAttribute(AttrDamageMultiplier).get_float();
    if (m_chargeRef->HasAttribute(AttrMaxRangeBonus))
        m_maxRange *= m_chargeRef->GetAttribute(AttrMaxRangeBonus).get_float();
    if (m_chargeRef->HasAttribute(AttrTrackingSpeedBonus))
        m_trackingSpeed *= m_chargeRef->GetAttribute(AttrTrackingSpeedBonus).get_float();
    if (m_chargeRef->HasAttribute(AttrFalloffBonus))
        m_falloff *= m_chargeRef->GetAttribute(AttrFalloffBonus).get_float();

    /*  these are tracking scripts used with weapon upgrades modules 209,213, etc
    if (m_chargeRef->HasAttribute(AttrMaxRangeBonusBonus))
        m_maxRange *= m_chargeRef->GetAttribute(AttrMaxRangeBonusBonus).get_float();
    if (m_chargeRef->HasAttribute(AttrTrackingSpeedBonusBonus))
        m_trackingSpeed *= m_chargeRef->GetAttribute(AttrTrackingSpeedBonusBonus).get_float();
    if (m_chargeRef->HasAttribute(AttrFalloffBonusBonus))
        m_falloff *= m_chargeRef->GetAttribute(AttrFalloffBonusBonus).get_float();
    */
}

void TurrentModule::UnloadCharge()
{
    ActiveModule::UnloadCharge();
    m_kinetic       = 0;
    m_thermal       = 0;
    m_em            = 0;
    m_explosive     = 0;
}

