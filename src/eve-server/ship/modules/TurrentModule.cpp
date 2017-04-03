
 /**
  * @name TurrentModule.h
  *   turrent module helper class
  * @Author:         Allan
  * @date:   10 June 2015
  */

 

#include "eve-server.h"

#include "ship/modules/TurrentModule.h"

TurrentModule::TurrentModule(InventoryItemRef item, ShipItemRef shipRef)
: ActiveModule(item, shipRef)
{
    m_falloff = GetAttribute(AttrFalloff).get_int();
    m_maxRange = GetAttribute(AttrMaxRange).get_int();
    m_capNeed = GetAttribute(AttrCapacitorNeed).get_float();
    m_trackingSpeed = GetAttribute(AttrTrackingSpeed).get_float();
    m_damageModifier = GetAttribute(AttrDamageMultiplier).get_float();
    m_optimalSigRadius = GetAttribute(AttrOptimalSigRadius).get_int();
}

void TurrentModule::LoadCharge(InventoryItemRef charge)
{
    ActiveModule::LoadCharge(charge);
    m_kinetic       = m_chargeRef->GetAttribute(AttrKineticDamage).get_float();
    m_thermal       = m_chargeRef->GetAttribute(AttrThermalDamage).get_float();
    m_em            = m_chargeRef->GetAttribute(AttrEmDamage).get_float();
    m_explosive     = m_chargeRef->GetAttribute(AttrExplosiveDamage).get_float();
}

void TurrentModule::UnloadCharge()
{
    ActiveModule::UnloadCharge();
    m_kinetic       = 0;
    m_thermal       = 0;
    m_em            = 0;
    m_explosive     = 0;
}

