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
    Author:        Aknor Jaden, Luck    (original code)
    Updates:    Allan   (reworked and implemented)
*/

/** @todo  there is much more to be done here.  this is just the beginning.
 * many, many effects missing from dgmEffectsInfo table (aknor was hand-writing them)
 *
 *      this file is to decode the fields in the 'dgmEffectsInfo' table.
 */

#ifndef MODULE_DEFS_H
#define MODULE_DEFS_H

#include "utils/EvilNumber.h"


//this is to avoid include complications and multiple dependancies etc.
enum ModuleCommand
{
    CMD_ERROR                   = 0,
    ONLINE                      = 1,
    OFFLINE                     = 2,
    ACTIVATE                    = 3,
    DEACTIVATE                  = 4,
    OVERLOAD                    = 5,
    DEOVERLOAD                  = 6,
    LOAD_CHARGE                 = 7,
    RELOAD_CHARGE               = 8,
    UNLOAD_CHARGE               = 9
};

enum ChargeStates
{
    MOD_UNLOADED                = 0,
    MOD_LOADING                 = 1,
    MOD_RELOADING               = 2,
    MOD_LOADED                  = 3
};

// these are used for internal module status checking
enum ModuleStates
{
    MOD_UNFITTED                = 0,
    MOD_OFFLINE                 = 1,   // module fitted, but NOT put online yet - NOT used for rigs
    /* 'Online' is used for:
     * ACTIVE modules fitted and online, but not activated (using the PASSIVE effects only)
     * PASSSIVE modules fitted and online
     * RIG modules fitted (either online or offline)
     */
    MOD_ONLINE                  = 2,    // module online  - rigs are either online or offline.
    MOD_ACTIVATED               = 3,    // used only for activated ACTIVE modules (Overloaded mode is calculated separately now)
    MOD_DEACTIVATING            = 4     // module transistioning from MOD_ACTIVATED to MOD_OFFLINE
};

// These are used in ModuleEffects.cpp to seperate effects into state containers
// *** these values are the 'effectAppliedInState' bitfield (as integer)
enum EffectStates
{
    EFFECT_UNFITTED             = 0,
    EFFECT_OFFLINE              = 1,
    EFFECT_ONLINE               = 2,
    EFFECT_ACTIVATED            = 4,
    EFFECT_OVERLOADED           = 8,
    EFFECT_GANG                 = 16,
    EFFECT_FLEET                = 32,
    EFFECT_DEACTIVATING         = 64
};

/** @todo  this needs updating and implementation....eventually  */
// Target types to which module effects are applied when activated:
// *** these values are the 'targetType' field
enum EffectTargetTypes
{   // 0: zero value.  undefined
    EFFECT_UNDEFINED            = 0,
    // 1: the target is the ship to which the module is fitted
    EFFECT_SHIP                 = 1,
    // 2: the target is a module fit to same ship. use 'targetGroupIDs' to decode affected groups
    EFFECT_MODULE               = 2,
    // 3: the target is a loaded charge.  use 'targetGroupIDs' to decode affected groups of loaded charges
    EFFECT_LOADED_CHARGE        = 3,
    // 4: the target is the current target of the ship to which the module is fitted
    EFFECT_TARGET               = 4,
    // 5: the target is a charge of a loaded module
    EFFECT_CHARGE               = 5,
    // 6: the target of the effect is the module's own attribute(s)
    EFFECT_TARGET_SELF          = 6,
    // 7: the effect acts upon the character's attribute specific to the effect
    EFFECT_CHARACTER            = 7
};

// this is used in generic module class
enum ModulePowerLevel
{
    MODULE_BANK_UNDEFINED       = 0,
    MODULE_BANK_LOW_POWER       = 1,
    MODULE_BANK_MEDIUM_POWER    = 2,
    MODULE_BANK_HIGH_POWER      = 3,
    MODULE_BANK_RIG             = 4,
    MODULE_BANK_SUBSYSTEM       = 5
};

//calculation types    rewrite 27Dec16    -allan
// *** these values are the 'calculationTypeID' and 'reverseCalculationTypeID' fields
enum EVECalculationType
{
    CALC_NONE                   = 0,
    CALC_ADD                    = 1,
    CALC_SUBTRACT               = 2,
    CALC_MULTIPLY               = 3,
    CALC_DIVIDE                 = 4,
    CALC_PERCENTAGE             = 5,
    CALC_REV_PERCENTAGE         = 6,
    CALC_ADD_PERCENT            = 7,
    CALC_SUBTRACT_PERCENT       = 8,
    CALC_ADD_RESIST             = 9,
    CALC_SUBTRACT_RESIST        = 10,
};

static EvilNumber CalculateNewAttributeValue(EvilNumber val1, EvilNumber val2, EVECalculationType type)
{
    switch(type) {
        case CALC_NONE:                            return val1;
        case CALC_ADD:                             return val1 + val2;
        case CALC_SUBTRACT:                        return val1 - val2;
        case CALC_MULTIPLY:                        return val1 * val2;
        case CALC_DIVIDE:                          return ((val2 != 0) ? val1 / val2 : val1);
        case CALC_PERCENTAGE:                      return val1 * (1 + (val2 / 100));
        case CALC_REV_PERCENTAGE:                  return val1 / (1 + (val2 / 100));
        case CALC_ADD_PERCENT:                     return val1 + (val2 /100);
        case CALC_SUBTRACT_PERCENT:                return val1 - (val2 /100);
        case CALC_ADD_RESIST:                      return val1 - (1 - val2);
        case CALC_SUBTRACT_RESIST:                 return val1 + (1 - val2);
    }

    _log(SHIP__MODULE_ERROR, "CalculateNewAttributeValue() - Unknown EveCalculationType used: %i", (int)type);
    return 0;
}


static EvilNumber Divide(EvilNumber& val1, EvilNumber& val2)
{// 4
    if (val2 != 0)
        return val1 / val2;
    return val1;
}

// these are used for DCU's (full resist, no penality)
static EvilNumber AddResist(EvilNumber& val1, EvilNumber& val2)
{// 30
    // name/operation is confusing...this ADDS RESISTANCE to ship (lowers attribute)
    return val1 - ( 1 - val2 );
}
static EvilNumber SubtractResist(EvilNumber& val1, EvilNumber& val2)
{// 31
    // name/operation is confusing...this SUBTRACTS RESISTANCE to ship (raises attribute)
    return val1 + ( 1 - val2 );
}

// used for passive resists (1%)
static EvilNumber AddPercent(EvilNumber& val1, EvilNumber& val2)
{// 50
    return val1 + ( val2 /100 );
}

static EvilNumber ReverseAddPercent(EvilNumber& val1, EvilNumber& val2)
{// 51
    return val1 - ( val2 /100 );
}

static EvilNumber AddAsPercent(EvilNumber& val1, EvilNumber& val2)
{// 54
    return val1 + ( val1 * (val2 / 100) );
}

static EvilNumber SubtractAsPercent(EvilNumber& val1, EvilNumber& val2)
{// 55
    return val1 / ( 1 + (val2 / 100) );
}

// the following arent currently used:
static EvilNumber SubtractByPercent(EvilNumber& val1, EvilNumber& val2)
{
    return val1 - ( val1 * val2 );
}

static EvilNumber ReverseSubtractByPercent(EvilNumber& val1, EvilNumber& val2)
{
    return val1 / ( 1 - val2 );
}

static EvilNumber MultiplyByPercent(EvilNumber& val1, EvilNumber& val2)
{
	return val1 * ( 1 - (val2 / 100) );
}

static EvilNumber ReverseMultiplyByPercent(EvilNumber& val1, EvilNumber& val2)
{
	return val1 / ( 1 - (val2 / 100) );
}

#endif
