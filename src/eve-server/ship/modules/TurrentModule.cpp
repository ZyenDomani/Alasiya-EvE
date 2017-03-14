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

}

void TurrentModule::Overload()
{
    GenericModule::Overload();
}

void TurrentModule::DeOverload()
{
    GenericModule::DeOverload();
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

