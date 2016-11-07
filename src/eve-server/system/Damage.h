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
    Damage( SystemEntity *_source,
            InventoryItemRef _weapon,
            double _kinetic,
            double _thermal,
            double _em,
            double _explosive,
            double modifier,
            EVEEffectID _effect);
    Damage( SystemEntity *_source,
            bool fatal_blow );          // weapon-less and charge-less constructor RESERVED for Killed() methods of derived SystemEntity objects
    Damage( SystemEntity *_source,
            InventoryItemRef _weapon,  // damage derrived directly from weapon.
            EVEEffectID _effect );
    Damage( SystemEntity* _source, InventoryItemRef _weapon, InventoryItemRef _charge, EVEEffectID _effect );

    virtual ~Damage() { }

    double GetTotal() const { return (kinetic + thermal + em + explosive); }

    Damage MultiplyDup( double _kinetic_multiplier,
                        double _thermal_multiplier,
                        double _em_multiplier,
                        double _explosive_multiplier ) const
                        {       // NOTE:  remember, these come in BACKWARD from 'normal' fuzzy logic..  0=full and 1=none
                                // added checks here for > 95% resists, and < 1% to avoid crazy damage shit.
                                // also added checks for missing resists (some npcs have no hull resist in db which = 100% resist)
                            if (_kinetic_multiplier > 1.0) _kinetic_multiplier = 1.0;
                            if (_kinetic_multiplier < 0.05) _kinetic_multiplier = 0.05;
                            if (_thermal_multiplier > 1.0) _thermal_multiplier = 1.0;
                            if (_thermal_multiplier < 0.05) _thermal_multiplier = 0.05;
                            if (_em_multiplier > 1.0) _em_multiplier = 1.0;
                            if (_em_multiplier < 0.05) _em_multiplier = 0.05;
                            if (_explosive_multiplier > 1.0) _explosive_multiplier = 1.0;
                            if (_explosive_multiplier < 0.05) _explosive_multiplier = 0.05;
                            return Damage( source, weapon,
                                            kinetic      * _kinetic_multiplier,
                                            thermal      * _thermal_multiplier,
                                            em           * _em_multiplier,
                                            explosive    * _explosive_multiplier,
                                            modifier,
                                            effect );
    }

    void ReduceTo(double total_amount)
    {
        *this *= ( total_amount / GetTotal() );
    }

    Damage &operator *=(double factor)
    {
        if (!factor) factor = 1;
        kinetic     *= factor;
        thermal     *= factor;
        em          *= factor;
        explosive   *= factor;

        return *this;
    }

    void SumWithMultFactor( double factor )
    {
        kinetic   += kinetic * factor;
        thermal   += thermal * factor;
        em        += em * factor;
        explosive += explosive * factor;
    }

    float GetThermal()      { return thermal; }
    float GetEM()           { return em; }
    float GetKinetic()      { return kinetic; }
    float GetExplosive()    { return explosive; }
    double GetModifier()    { return modifier; }

    SystemEntity *const     source;    //we do not own this.
    const EVEEffectID       effect;
    InventoryItemRef        weapon;    //we own a ref to this.
    InventoryItemRef        charge;    //we own a ref to this. May be null.

private:
    double kinetic;
    double thermal;
    double em;
    double explosive;
    double modifier;
};

#endif
