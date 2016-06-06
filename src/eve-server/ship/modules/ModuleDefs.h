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
    Author:        Aknor Jaden, Luck
    Updates:    Allan
*/

/** @todo  there is much more to be done here.  this is just the beginning.
 * many, many effects missing from dgmEffectsInfo table (aknor was hand-writing them)
 * module states incomplete.  only coded for online, deactivating, offline and unfitted right now.
 */

#ifndef MODULE_DEFS_H
#define MODULE_DEFS_H

#include "utils/EvilNumber.h"


// Important constants pertaining to modules and their operation:
// 75% of your capacitor will be drained when you online a module while in space, if you have it
#define ONLINE_MODULE_IN_SPACE_CAP_PENALTY					0.75


//more will go here
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


enum EffectCategories
{
    dgmEffPassive               = 0,
    dgmEffActivation            = 1,
    dgmEffTarget                = 2,
    dgmEffArea                  = 3,
    dgmEffOnline                = 4,
    dgmEffOverload              = 5
};

// These are the module states when an effect will take affect:
// *** use these values to decode the 'effectAppliedInState' field of the 'dgmEffectsInfo' database table
/* these are used in ModuleEffects to seperate effects into state containers  */
enum ModuleStates
{
    MOD_UNFITTED                = 0,
    // means the effect is active AT ALL TIMES; used ONLY for skill, ship, rig, subsystem, and beacon effects
    MOD_PASSIVE                 = 1,
    /* 'Online' is used for:
     * ACTIVE modules fitted and online, but not activated (PASSIVE effects only)
     * PASSSIVE modules fitted and online
     * RIG modules fitted (always online)
     */
    MOD_ONLINE                  = 2, // module fitted and online
    MOD_ACTIVATED               = 4, // used only for ACTIVE modules operating in non-Overloaded mode
    MOD_OVERLOADED              = 8, // used only for ACTIVE modules operating in Overloaded mode
    MOD_GANG                    = 16,
    MOD_FLEET                   = 32,
    // used for internal module status checking
    MOD_DEACTIVATING            = 64,       // module transistioning from online/active state to offline state
    MOD_OFFLINE                 = 128
};

// *** use these values to decode the 'effectID' field of the 'dgmEffectsInfo' database table
/**  depreciated.  use EffectsEnum.h for these
enum ModuleEffectAppliedBehaviors
{
    EFFECT_PERSISTENT           = 1,
    EFFECT_PASSIVE              = 2,
    EFFECT_OVERLOAD             = 8,
    EFFECT_LOPOWER              = 11,
    EFFECT_HIPOWER              = 12,
    EFFECT_MEDPOWER             = 13,
    EFFECT_ONLINE               = 16,
    EFFECT_MINING               = 17,
    EFFECT_RIG                  = 2663,
    EFFECT_SUBSYSTEM            = 3772
};
*/

/** @todo  these next two are way off.  they need updating and implementation....eventually  */
// These are the target types to which module and other types' effects are applied when activated:
// *** use these values to decode the 'effectingType' field of the 'dgmEffectsInfo' database table
enum EffectTargetEquipmentTypes
{
    EQUIP_MODULE                = 0,
    EQUIP_CHARGE                = 1,
    EQUIP_THIS_SHIP             = 2,
    EQUIP_DRONE                 = 3,
    EQUIP_EXTERNAL_SHIP         = 4,
    EQUIP_EXTERNAL_SHIP_MODULE  = 5,
    EQUIP_EXTERNAL_SHIP_CHARGE  = 6
};

// These are the target types to which module effects are applied when activated:
// *** use these values to decode the 'effectedType' field of the 'dgmEffectsInfo' database table
enum ModuleEffectAppliedToTargetTypes
{
    // 0: the target of the effect is the module's own attribute(s)
    EFFECT_TARGET_SELF          = 0,
    // 1: the target of the effect is the attribute(s) of the ship to which the module is fitted
    EFFECT_TARGET_SHIP          = 1,
    // 2: the target of the effect is the attribute(s) of the current target of the ship to which the module is fitted
    EFFECT_TARGET_EXTERNAL      = 2,
    // 3: module or modules that are fitted to the current ship, this special case will indicate when the effect is
    // applied to other modules applied to the same ship - the dgmEffectsActions table fields of targetEquipmentType and
    // targetGroupIDs will have additional information for the Module Manager to make use of this effect
    EFFECT_MODULE_ON_SHIP       = 3,
    // 4: the effect is from a loaded charge, which will affect the weapon module the charge is loaded into
    EFFECT_LOADED_CHARGE        = 4,
    // 5: the effect acts upon a charge loaded into a weapon module.  this will affect loaded charges of the specified groupID
    EFFECT_CHARGE               = 5,
    // 6: the effect acts upon the character's attribute specific to the effect
    EFFECT_CHARACTER            = 6
};

// These are the methods by which module effects are applied to the designated target:
// *** use these values to decode the 'effectApplicationType' field of the 'dgmEffectsInfo' database table
enum ModuleApplicationTypes
{
    // applied by PASSIVE or ACTIVE modules where an effect is maintained; means the effect takes effect on the
    // target (see below) upon entering the ONLINE state, then reversed when going out of ONLINE state
    EFFECT_ONLINE_MAINTAIN      = 1600,
    // applied by ACTIVE modules where an effect is maintained; means the effect takes effect on the
    // target (see below) upon entering the ACTIVATE state, then reversed when going out of ACTIVATE state
    EFFECT_ACTIVE_MAINTAIN      = 1601,
    // applied by ACTIVE modules where an effect is applied cumulatively on each cycle; means the effect takes
    // effect on the target (see below) one extra time when in ACTIVATE state after each CYCLE duration expires
    EFFECT_ACTIVE_ACCUMULATE    = 1602
};

// These are the methods by which module effects are applied to the designated target:
// *** use these values to decode the 'stackingPenaltyApplied' field of the 'dgmEffectsInfo' database table
enum ModuleStackingPenaltyState
{
    NO_STACKING_PENALTY         = 0,
    STACKING_PENALTY_APPLIES    = 1
};

enum ModulePowerLevel
{
    MODULE_BANK_UNDEFINED       = 0,
    MODULE_BANK_LOW_POWER       = 1,
    MODULE_BANK_MEDIUM_POWER    = 2,
    MODULE_BANK_HIGH_POWER      = 3,
    MODULE_BANK_RIG             = 4,
    MODULE_BANK_SUBSYSTEM       = 5
};

//calculation types
// *** use these values to decode the 'calculationTypeID' and the 'reverseCalculationTypeID' fields of the 'dgmEffectsInfo' database table
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

    //  mod'd db for these calculations     -allan Dec2015
    CALC_REV_ABSOLUTE           = 24,
    CALC_DIVIDER                = 25,
    CALC_SUBTRACT_POSITIVE      = 26,
    CALC_SUBTRACT_NEGATIVE      = 27,

    CALC_ADD_RESIST             = 30,
    CALC_SUBTRACT_RESIST        = 31,

    CALC_REVERSE_PERCENTAGE     = 40,

    // for shields
    CALC_ADD_PERCENT            = 50,
    CALC_REV_ADD_PERCENT        = 51,

    CALC_ADD_AS_PERCENT         = 54,
    CALC_SUBTRACT_AS_PERCENT    = 55,

    //  added these but not sure if we'll use them.
    CALC_SUBTRACT_PERCENT       = 52,
    CALC_REV_SUBTRACT_PERCENT   = 53,
    CALC_MODIFY_PERCENT_W_PERCENT       = 56,
    CALC_REV_MODIFY_PERCENT_W_PERCENT   = 57,
    CALC_REDUCE_BY_PERCENT      = 58,
    CALC_REV_REDUCE_BY_PERCENT  = 59

    //more will show up, im sure
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
{
    return (attrVal + modVal);
}

static EvilNumber Subtraction(EvilNumber &attrVal, EvilNumber &modVal)
{
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
{
    return (attrVal * modVal);
}

static EvilNumber Divider(EvilNumber &val1, EvilNumber &val2)
{
    if (val2 > 0)
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

static EvilNumber AddPercent(EvilNumber &val1, EvilNumber &val2)
{
    return val1 + ( val2 /100 );
}

static EvilNumber ReverseAddPercent(EvilNumber &val1, EvilNumber &val2)
{
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
{
    EvilNumber val3 = 100;
    return val1 + ( val1 * (val2 / val3) );
}

static EvilNumber SubtractAsPercent(EvilNumber &val1, EvilNumber &val2)
{
    EvilNumber val3 = 1;
    EvilNumber val4 = 100;

    return val1 / ( val3 + (val2 / val4) );
}

static EvilNumber ModifyPercentWithPercent(EvilNumber &val1, EvilNumber &val2)
{
    EvilNumber val3 = 1;
    EvilNumber val4 = 100;

    return val1 * (val3 + (val2 / val4) );
}

static EvilNumber ReverseModifyPercentWithPercent(EvilNumber &val1, EvilNumber &val2)
{
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
