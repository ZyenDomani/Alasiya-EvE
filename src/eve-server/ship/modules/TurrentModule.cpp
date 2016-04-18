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

TurrentModule::TurrentModule(InventoryItemRef item, ShipRef shipRef)
: ActiveModule(item, shipRef)
{
    m_ROF = m_Item->GetAttribute(AttrSpeed).get_float();
    m_capNeed = m_Item->GetAttribute(AttrCapacitorNeed).get_float();
    m_damageModifier = m_Item->GetAttribute(AttrDamageMultiplier).get_float();

    Character* pChar = m_Ship->GetOperator()->GetChar().get();
    m_ROF *= (1 - ( 0.02 * (pChar->GetSkillLevel(skillGunnery, true))));      //  2% decrease in rof
    m_ROF *= (1 - ( 0.04 * (pChar->GetSkillLevel(skillRapidFiring, true))));  //  4% decrease in rof
    m_capNeed *= (1 - ( 0.05 * (pChar->GetSkillLevel(skillControlledBursts, true))));  //  5% decrease in cap need
    m_damageModifier *= (1 + ( 0.05 * (pChar->GetSkillLevel(skillSurgicalStrike, true)))); // 5% increase in damage (upped from 3%)
    m_damageModifier *= (1 + ( 0.03 * (pChar->GetSkillLevel(skillWeaponUpgrades, true)))); // 3% increase in damage
}

void TurrentModule::Overload()
{
    GenericModule::Overload();
    m_damageModifier *= (1 + (m_Item->GetAttribute(AttrOverloadDamageModifier).get_float() /100));
    m_ROF *= (1 + m_Item->GetAttribute(AttrOverloadRofBonus).get_float());
}

void TurrentModule::DeOverload()
{
    m_damageModifier /= (1 + (m_Item->GetAttribute(AttrOverloadDamageModifier).get_float() /100));
    m_ROF /= (1 + m_Item->GetAttribute(AttrOverloadRofBonus).get_float());
    GenericModule::DeOverload();
}

void TurrentModule::Load(InventoryItemRef charge)
{
    //if (m_chargeRef) assert(m_chargeRef != charge);

    ActiveModule::Load(charge);
    _UpdateModifiers(charge);
    m_kinetic       = m_chargeRef->GetAttribute(AttrKineticDamage).get_float();
    m_thermal       = m_chargeRef->GetAttribute(AttrThermalDamage).get_float();
    m_em            = m_chargeRef->GetAttribute(AttrEmDamage).get_float();
    m_explosive     = m_chargeRef->GetAttribute(AttrExplosiveDamage).get_float();
}

void TurrentModule::Unload()
{
    if (m_chargeRef)
        _RemoveModifier(m_chargeRef);
    ActiveModule::Unload();
    m_kinetic       = 0;
    m_thermal       = 0;
    m_em            = 0;
    m_explosive     = 0;
}

void TurrentModule::_UpdateModifiers(InventoryItemRef item)
{
    //  this part will get damage modifier from the module itself.  this is a hack.
    // the method/function to get skill damage modifiers is in the specific weapon constructor.
    if (!m_damageModifier) m_damageModifier = 1.0;
    if (item->HasAttribute(AttrDamageMultiplier))
        m_damageModifier *= item->GetAttribute(AttrDamageMultiplier).get_float();


    //  this will be the place to put ship and skill and implant modifiers
}

void TurrentModule::_RemoveModifier(InventoryItemRef item)
{
    if (!m_damageModifier) m_damageModifier = 1.0;
    if (item->HasAttribute(AttrDamageMultiplier))
        m_damageModifier /= item->GetAttribute(AttrDamageMultiplier).get_float();

}

