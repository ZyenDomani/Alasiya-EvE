
 /**
  * @name TurretModule.cpp
  *   turret module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */


#include "eve-server.h"

#include "ship/modules/TurretModule.h"
#include "system/Damage.h"

TurretModule::TurretModule(InventoryItemRef item, ShipItemRef shipRef)
: ActiveModule(item, shipRef)
{
    m_crystalDmg = 0;
    m_crystalDmgAmount = 0;
    m_crystalDmgChance = 0;
}

void TurretModule::LoadCharge(InventoryItemRef charge)
{
    ActiveModule::LoadCharge(charge);
    m_crystalDmg        = m_chargeRef->GetAttribute(AttrDamage).get_float();
    m_crystalDmgAmount  = m_chargeRef->GetAttribute(AttrCrystalVolatilityDamage).get_float();
    m_crystalDmgChance  = m_chargeRef->GetAttribute(AttrCrystalVolatilityChance).get_float();
}

void TurretModule::UnloadCharge()
{
    ActiveModule::UnloadCharge();
    m_crystalDmg        = 0;
    m_crystalDmgAmount  = 0;
    m_crystalDmgChance  = 0;
}

void TurretModule::ApplyDamage()
{
    Damage d(m_shipRef->GetPilot()->GetShipSE(),
             m_modRef,
             m_chargeRef->GetAttribute(AttrKineticDamage).get_float(),
             m_chargeRef->GetAttribute(AttrThermalDamage).get_float(),
             m_chargeRef->GetAttribute(AttrEmDamage).get_float(),
             m_chargeRef->GetAttribute(AttrExplosiveDamage).get_float(),
             m_formula.GetToHit(m_shipRef, this, m_targetSE),
             m_effectID
    );

    d *= GetAttribute(AttrDamageMultiplier).get_float();
    d *= sConfig.rates.turretDamage;
    m_targetSE->ApplyDamage(d);

    switch (m_modRef->groupID()) {
        case EVEDB::invGroups::Projectile_Weapon:
        case EVEDB::invGroups::Hybrid_Weapon: {
            if (m_chargeLoaded)
                m_chargeRef->SetQuantity(m_chargeRef->quantity() - 1, true);
        } break;
        case EVEDB::invGroups::Energy_Weapon: {
            if (m_chargeRef->HasAttribute(AttrCrystalsGetDamaged))
                if (MakeRandomFloat(0,1) < m_crystalDmgChance) {
                    m_crystalDmg += m_crystalDmgAmount;
                    if (m_crystalDmg > 1.0f) {
                        m_shipRef->GetPilot()->SendNotifyMsg("Your %s loaded in %s has been destroyed.", m_chargeRef->itemName().c_str(), m_modRef->itemName().c_str());
                        InventoryItemRef chargeRef = m_chargeRef;   // make a copy of item ref, as m_chargeRef = NULL after next call returns
                        m_shipRef->RemoveItem(m_chargeRef);
                        chargeRef->Delete();
                    } else {
                        m_chargeRef->SetAttribute(AttrDamage, m_crystalDmg);
                    }
                }

        } break;
    }
}
