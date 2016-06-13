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
 * module states incomplete.  only coded for online, deactivating, offline and unfitted right now.
 *
 *      this file is to decode the fields in the 'dgmEffectsInfo' table.
 */

#ifndef MODULE_DEFS_H
#define MODULE_DEFS_H

#include "utils/EvilNumber.h"


//this is to avoid include complications and multiple dependancies etc..
enum ModuleCommand
{
    CMD_ERROR                   = 1000,
    ONLINE                      = 1001,
    OFFLINE                     = 1002,
    ACTIVATE                    = 1003,
    DEACTIVATE                  = 1004,
    OVERLOAD                    = 1005,  //idk if this is used
    DEOVERLOAD                  = 1006,  //idk if this is used
    LOAD_CHARGE                 = 1007,
    RELOAD_CHARGE               = 1008,
    UNLOAD_CHARGE               = 1009
};

enum ChargeStates
{
    MOD_UNLOADED                = 0,
    MOD_LOADING                 = 1,
    MOD_RELOADING               = 2,
    MOD_LOADED                  = 3
};


enum EffectCategories   // not sure what this is, or if it's used.
{
    dgmEffPassive               = 0,
    dgmEffActivation            = 1,
    dgmEffTarget                = 2,
    dgmEffArea                  = 3,
    dgmEffOnline                = 4,
    dgmEffOverload              = 5
};

// These are the module states when an effect will take affect:  still needs a bit of work and thought.
// *** these values are the 'effectAppliedInState' bitfield (as integer)
/* these are used in ModuleEffects.cpp to seperate effects into state containers  */
    // also used for internal module status checking
enum ModuleStates
{
    MOD_UNFITTED                = 0,
    // means the effect is active AT ALL TIMES; used ONLY for skill, ship, rig, subsystem, and beacon effects
    MOD_OFFLINE                  = 1,   // module fitted, but NOT put online yet - NOT used for rigs    -- not used yet (needs code rewrite)
    /* 'Online' is used for:
     * ACTIVE modules fitted and online, but not activated (PASSIVE effects only)
     * PASSSIVE modules fitted and online
     * RIG modules fitted (always online)
     */
    MOD_ONLINE                  = 2,    // module online  - rigs are either online or offline.
    MOD_ACTIVATED               = 4,    // used only for ACTIVE modules operating in non-Overloaded mode
    MOD_OVERLOADED              = 8,    // used only for ACTIVE modules operating in Overloaded mode
    MOD_GANG                    = 16,   // not used yet
    MOD_FLEET                   = 32,   // not used yet
    MOD_DEACTIVATING            = 64,   // module transistioning from MOD_ACTIVATED to MOD_OFFLINE
};

/** @todo  this needs updating and implementation....eventually  */
// These are the target types to which module effects are applied when activated:
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
    // 5: the target is a loaded module  - this could use EFFECT_MODULE
    EFFECT_CHARGE               = 5,
    // 6: the target of the effect is the module's own attribute(s)  - maybe unused
    EFFECT_TARGET_SELF          = 6,
    // 7: the effect acts upon the character's attribute specific to the effect  - maybe unused.
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

//calculation types    updated Dec2015    -allan
// *** these values are the 'calculationTypeID' and the 'reverseCalculationTypeID' fields
enum EVECalculationType
{
    CALC_NONE                   = -1,
    CALC_PERCENTAGE             = 0,
    CALC_ADDITION               = 1,
    CALC_DIFFERENCE             = 2,
	CALC_VELOCITY               = 3,
    CALC_ABSOLUTE               = 4,
	CALC_MULTIPLIER             = 5,
    CALC_ADD_POSITIVE           = 6,
    CALC_ADD_NEGATIVE           = 7,
    CALC_SUBTRACTION            = 8,
    CALC_CLOAKED_VELOCITY       = 9,
    CALC_SKILL_LEVEL            = 10,
    CALC_SKILL_LEVEL_x_ATT      = 11,
    CALC_ABSOLUTE_MAX           = 12,
    CALC_ABSOLUTE_MIN           = 13,
    CALC_CAP_BOOSTERS           = 14,

    CALC_REV_ABSOLUTE           = 24,
    CALC_DIVIDER                = 25,
    CALC_SUBTRACT_POSITIVE      = 26,
    CALC_SUBTRACT_NEGATIVE      = 27,

    CALC_ADD_RESIST             = 30,
    CALC_SUBTRACT_RESIST        = 31,

    CALC_REVERSE_PERCENTAGE     = 40,

    // for resists
    CALC_ADD_PERCENT            = 50,
    CALC_REV_ADD_PERCENT        = 51,

    CALC_ADD_AS_PERCENT         = 54,
    CALC_SUBTRACT_AS_PERCENT    = 55,

    //  added these but not sure if we'll use them....yes, in incursion effect beacons (ie 3069)
    CALC_SUBTRACT_PERCENT       = 52,
    CALC_REV_SUBTRACT_PERCENT   = 53,
    CALC_MODIFY_PERCENT_W_PERCENT       = 56,
    CALC_REV_MODIFY_PERCENT_W_PERCENT   = 57,
    CALC_REDUCE_BY_PERCENT      = 58,
    CALC_REV_REDUCE_BY_PERCENT  = 59
};


static EvilNumber Percentage(EvilNumber &attrVal, EvilNumber &modVal)
{
    return (attrVal * (EvilNumber(1.0) + (modVal / EvilNumber(100.0))));
}

static EvilNumber ReversePercentage(EvilNumber &attrVal, EvilNumber &modVal)
{
    return (attrVal / (EvilNumber(1.0) + (modVal / EvilNumber(100.0))));
}

static EvilNumber Addition(EvilNumber &attrVal, EvilNumber &modVal)
{// 1
    return (attrVal + modVal);
}

static EvilNumber Subtraction(EvilNumber &attrVal, EvilNumber &modVal)
{// 8
    return (attrVal - modVal);
}

static EvilNumber Difference(EvilNumber &attrVal, EvilNumber &modVal)
{
	if( modVal <= 0 )
		return (((EvilNumber(100.0) - attrVal) * (-modVal / EvilNumber(100))) + attrVal);
	else
		return ((attrVal * (-modVal / EvilNumber(100.0))) + attrVal);
}

static EvilNumber Velocity(EvilNumber &attrVal, EvilNumber &modVal)
{
	// In this special case, it is expected that modVal is actually the thrust/mass ratio multiplied by the module effect source attribute:
	return (attrVal + (attrVal * modVal / EvilNumber(100.0)));
}

static EvilNumber Multiplier(EvilNumber &attrVal, EvilNumber &modVal)
{// 5
    return (attrVal * modVal);
}

static EvilNumber Divider(EvilNumber &val1, EvilNumber &val2)
{// 25
    if (val2 != 0)
        return ( val1 / val2 );
    return val1;
}

static EvilNumber AddPositive(EvilNumber &attrVal, EvilNumber &modVal)
{
	if( modVal > 0 )
		return (attrVal + modVal);
	else
		return (attrVal);
}

static EvilNumber AddNegative(EvilNumber &attrVal, EvilNumber &modVal)
{
	if( modVal < 0 )
		return (attrVal + modVal);
	else
		return (attrVal);
}

static EvilNumber SubtractPositive(EvilNumber &attrVal, EvilNumber &modVal)
{
    if( modVal > 0 )
        return (attrVal - modVal);
    else
        return (attrVal);
}

static EvilNumber SubtractNegative(EvilNumber &attrVal, EvilNumber &modVal)
{
    if( modVal < 0 )
        return (attrVal - modVal);
    else
        return (attrVal);
}

static EvilNumber CloakedVelocity(EvilNumber &attrVal, EvilNumber &modVal)
{
	return (EvilNumber(-100.0) + ((EvilNumber(100.0) + attrVal * (modVal / EvilNumber(100.0)))));
}

static EvilNumber AbsoluteMax(EvilNumber &attrVal, EvilNumber &modVal)
{
	if( attrVal > modVal )
		return attrVal;
	else
		return modVal;
}

static EvilNumber AbsoluteMin(EvilNumber &attrVal, EvilNumber &modVal)
{
	if( attrVal < modVal )
		return attrVal;
	else
		return modVal;
}

static EvilNumber CapBoosters(EvilNumber &attrVal, EvilNumber &modVal)
{
	if( (attrVal - modVal) < 0 )
		return (attrVal - modVal);
	else
		return EvilNumber(0.0);
}

static EvilNumber AddResist(EvilNumber &val1, EvilNumber &val2)
{   // name/operation is confusing...this ADDS RESISTANCE to ship (lowers attribute)
    EvilNumber res = val1 - ( 1 - val2 );
    if (res < 0) res = 0;
    if (res > 1) res = 1;
    return res;
}

static EvilNumber SubtractResist(EvilNumber &val1, EvilNumber &val2)
{   // name/operation is confusing...this SUBTRACTS RESISTANCE to ship (raises attribute)
    EvilNumber res = val1 + ( 1 - val2 );
    if (res < 0) res = 0;
    if (res > 1) res = 1;
    return res;
}

// used for shields
static EvilNumber AddPercent(EvilNumber &val1, EvilNumber &val2)
{// 50
    return val1 + ( val2 /100 );
}

static EvilNumber ReverseAddPercent(EvilNumber &val1, EvilNumber &val2)
{// 51
    return val1 - ( val2 /100 );
}

static EvilNumber SubtractPercent(EvilNumber &val1, EvilNumber &val2)
{
    return val1 - ( val1 * val2 );
}

static EvilNumber ReverseSubtractPercent(EvilNumber &val1, EvilNumber &val2)
{
    EvilNumber val3 = 1;
    return val1 / ( val3 - val2 );
}

static EvilNumber AddAsPercent(EvilNumber &val1, EvilNumber &val2)
{// 54
    EvilNumber val3 = 100;
    return val1 + ( val1 * (val2 / val3) );
}

static EvilNumber SubtractAsPercent(EvilNumber &val1, EvilNumber &val2)
{//55
    EvilNumber val3 = 1;
    EvilNumber val4 = 100;

    return val1 / ( val3 + (val2 / val4) );
}

static EvilNumber ModifyPercentWithPercent(EvilNumber &val1, EvilNumber &val2)
{//56
    EvilNumber val3 = 1;
    EvilNumber val4 = 100;

    return val1 * (val3 + (val2 / val4) );
}

static EvilNumber ReverseModifyPercentWithPercent(EvilNumber &val1, EvilNumber &val2)
{//57
    EvilNumber val3 = 1;
    EvilNumber val4 = 100;

    return val4 * ( (val1 / val2) - 1 );
}

static EvilNumber ReduceByPercent(EvilNumber &val1, EvilNumber &val2)
{
	EvilNumber val3 = 1;
	EvilNumber val4 = 100;

	return val1 * ( val3 - (val2 / val4) );
}

static EvilNumber ReverseReduceByPercent(EvilNumber &val1, EvilNumber &val2)
{
	EvilNumber val3 = 1;
	EvilNumber val4 = 100;

	return val1 / ( val3 - (val2 / val4) );
}

static EvilNumber CalculateNewAttributeValue(EvilNumber attrVal, EvilNumber attrMod, EVECalculationType type)
{
    switch(type)
    {
        case CALC_NONE :                            return attrVal;
		case CALC_PERCENTAGE :						return Percentage(attrVal, attrMod); break;
        case CALC_REVERSE_PERCENTAGE :              return ReversePercentage(attrVal, attrMod); break;
		case CALC_ADDITION :						return Addition(attrVal, attrMod); break;
		case CALC_DIFFERENCE :						return Difference(attrVal, attrMod); break;
		case CALC_VELOCITY :						return Velocity(attrVal, attrMod); break;
        case CALC_ABSOLUTE :						return attrVal - attrMod; break;
        case CALC_REV_ABSOLUTE :                    return attrVal + attrMod; break;
        case CALC_MULTIPLIER :						return Multiplier(attrVal, attrMod); break;
        case CALC_DIVIDER :                         return Divider(attrVal, attrMod); break;
		case CALC_ADD_POSITIVE :					return AddPositive(attrVal, attrMod); break;
        case CALC_SUBTRACT_POSITIVE :               return SubtractPositive(attrVal, attrMod); break;
		case CALC_ADD_NEGATIVE :					return AddNegative(attrVal, attrMod); break;
        case CALC_SUBTRACT_NEGATIVE :               return SubtractNegative(attrVal, attrMod); break;
		case CALC_SUBTRACTION :						return Subtraction(attrVal, attrMod); break;
		case CALC_CLOAKED_VELOCITY :				return CloakedVelocity(attrVal, attrMod); break;
		case CALC_SKILL_LEVEL :						return attrVal; break;	// is this really right for attribute effect per skill level?
		case CALC_SKILL_LEVEL_x_ATT :				return attrVal; break;	// is this really right for attribute effect per skill level?
		case CALC_ABSOLUTE_MAX :					return AbsoluteMax(attrVal, attrMod); break;
		case CALC_ABSOLUTE_MIN :					return AbsoluteMin(attrVal, attrMod); break;
		case CALC_CAP_BOOSTERS :					return CapBoosters(attrVal, attrMod); break;
        case CALC_ADD_RESIST :                      return AddResist(attrVal, attrMod); break;
        case CALC_SUBTRACT_RESIST :                 return SubtractResist(attrVal, attrMod); break;
        //case CALC_AUTO :                            return attrVal; break;                             // AUTO NOT SUPPORTED AT THIS TIME !!!
        //case CALC_ADD :                             return Add(attrVal, attrMod); break;
        //case CALC_SUBTRACT :                        return Subtract(attrVal, attrMod); break;
        //case CALC_DIVIDE :                          return Divide(attrVal, attrMod); break;
        //case CALC_MULTIPLY :                        return Multiply(attrVal, attrMod); break;
        case CALC_ADD_PERCENT :                     return AddPercent(attrVal, attrMod); break;
        case CALC_REV_ADD_PERCENT :                 return ReverseAddPercent(attrVal, attrMod); break;
        case CALC_SUBTRACT_PERCENT :                return SubtractPercent(attrVal, attrMod); break;
        case CALC_REV_SUBTRACT_PERCENT :            return ReverseSubtractPercent(attrVal, attrMod); break;
        case CALC_ADD_AS_PERCENT :                  return AddAsPercent(attrVal, attrMod); break;
        case CALC_SUBTRACT_AS_PERCENT :             return SubtractAsPercent(attrVal, attrMod); break;
        case CALC_MODIFY_PERCENT_W_PERCENT :        return ModifyPercentWithPercent(attrVal, attrMod); break;
        case CALC_REV_MODIFY_PERCENT_W_PERCENT :    return ReverseModifyPercentWithPercent(attrVal, attrMod); break;
		case CALC_REDUCE_BY_PERCENT:				return ReduceByPercent(attrVal, attrMod); break;
		case CALC_REV_REDUCE_BY_PERCENT :			return ReverseReduceByPercent(attrVal, attrMod); break;
		//default:									return 0; break;
    }

    _log(SHIP__MODULE_ERROR, "CalculateNewAttributeValue() - Unknown EveCalculationType used: %i", (int)type);
    //assert(false);
    return 0;
}

#endif
