/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Author:        Zhur
    Updates:    Allan
*/
#ifndef __DAMAGE_H_INCL__
#define __DAMAGE_H_INCL__

#include "PyServiceMgr.h"
#include "inventory/InventoryItem.h"
#include "system/SystemManager.h"

class Damage {
public:
    Damage(SystemEntity* pSE, InventoryItemRef wRef, float mod);
    Damage(SystemEntity* pSE, InventoryItemRef wRef, InventoryItemRef cRef);
    Damage(SystemEntity* pSE, InventoryItemRef wRef, float kin, float ther, float emp, float exp, float mod);
    // constructor for Killed() methods of derived SystemEntity objects with no weapon
    Damage(SystemEntity *pSE, bool fatal_blow=false);

    ~Damage()                                           { /* do nothing here */ }

    float GetThermal()                                  { return thermal; }
    float GetEM()                                       { return em; }
    float GetKinetic()                                  { return kinetic; }
    float GetExplosive()                                { return explosive; }
    float GetModifier()                                 { return modifier; }
    float GetTotal() const                              { return (kinetic + thermal + em + explosive); }

    Damage MultiplyDup( float kinetic_multiplier,
                        float thermal_multiplier,
                        float em_multiplier,
                        float explosive_multiplier ) const
                        {   // NOTE:  remember, these come in BACKWARD from 'normal' fuzzy logic..  0=full and 1=none
                            // added checks here for > 95% resists, and < 1% to avoid crazy damage shit.
                            // also added checks for missing resists (some npcs have no hull resist in db which = 100% resist)
                            if (kinetic_multiplier > 1.0f) kinetic_multiplier = 1.0f;
                            if (kinetic_multiplier < 0.01f) kinetic_multiplier = 0.01f;
                            if (thermal_multiplier > 1.0f) thermal_multiplier = 1.0f;
                            if (thermal_multiplier < 0.01f) thermal_multiplier = 0.01f;
                            if (em_multiplier > 1.0f) em_multiplier = 1.0f;
                            if (em_multiplier < 0.01f) em_multiplier = 0.01f;
                            if (explosive_multiplier > 1.0f) explosive_multiplier = 1.0f;
                            if (explosive_multiplier < 0.01f) explosive_multiplier = 0.01f;
                            return Damage( srcSE, weaponRef,
                                           kinetic   * kinetic_multiplier,
                                           thermal   * thermal_multiplier,
                                           em        * em_multiplier,
                                           explosive * explosive_multiplier,
                                           modifier);
    }

    Damage &operator *=(float factor)
    {
        kinetic     *= factor;
        thermal     *= factor;
        em          *= factor;
        explosive   *= factor;

        return *this;
    }

    SystemEntity*           srcSE;     //we do not own this.
    InventoryItemRef        weaponRef;
    InventoryItemRef        chargeRef;    // May be null.

private:
    float kinetic;
    float thermal;
    float em;
    float explosive;
    float modifier;
};

#endif

/*
  Calculate how much raw damage from the package gets through the shield           *
Damage DamageToShield = damage.MultiplyDup(
    m_self->GetAttribute(AttrShieldKineticDamageResonance).get_float(),
    m_self->GetAttribute(AttrShieldThermalDamageResonance).get_float(),
    m_self->GetAttribute(AttrShieldEmDamageResonance).get_float(),
    m_self->GetAttribute(AttrShieldExplosiveDamageResonance).get_float()
    );

    float shield_damage = DamageToShield.GetTotal();
    float available_shield = m_self->GetAttribute(AttrShieldCharge).get_float();

    if (shield_damage <= available_shield) {
        // Shield tanks everything normally
        total_damage += shield_damage;
        m_self->SetAttribute(AttrShieldCharge, available_shield - shield_damage);
    } else {
        // Shields collapse!
        // Calculate the percentage of the incoming resisted package that was soaked
        float absorb_ratio = available_shield / shield_damage;
        total_damage += available_shield;
        m_self->SetAttribute(AttrShieldCharge, 0.0f);
        // Turn it into a direct remainder multiplier
        float remainder_factor = 1.0f - absorb_ratio;
        // Fast, direct floating-point scaling
        damage.kinetic   *= remainder_factor;
        damage.thermal   *= remainder_factor;
        damage.em        *= remainder_factor;
        damage.explosive *= remainder_factor;
    }
    // Now 'damage' contains the exact raw profile remainder to pass directly to armor!
*/
