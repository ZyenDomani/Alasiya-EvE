/**
 * @name EffectsData.h
 *   This file is data containers used with EffectsProcessor (FxProc)
 *   Copyright 2017  Alasiya-EVEmu Team
 *
 * @Author:    Allan
 * @date:      24 January 2017
 *
 */



#ifndef _EVE_FX_PROC_DATA_H__
#define _EVE_FX_PROC_DATA_H__



/** @todo  there is much more to be done here.  this is just the beginning.
 * many, many effects missing from dgmModuleEffects table (aknor was hand-writing them)
 *
 * this file is to decode the fields in the 'dgmModuleEffects' table.
 *
 * field defs...
 * effectID             - id of this effect
 * description          - effect description
 * sourceAttribute      - modifier value location
 * targetAttribute      - location of value to be modified
 * calculationType      - calculation to be used when applying this effect.     defined in EVECalculationType
 * rCalculationType     - calculation to be used when removing this effect.     defined in EVECalculationType
 * affectedGrps         - this is a list of groupIDs affected by this effect
 * stacked              - boolean requesting the application of stacking penality for this effect
 * state                - item state needed to apply this effect.               defined in EffectStates
 * targetType           - this is the "type" of item affected by this effect.   defined in EffectTargetTypes
 * effectGrp            - this is the module groupID this effect is applied to
 *
 *
 * Distinguish the difference between targetGroupIDs, targetGroup, affectedGrps, targetType
 *
 * targetGroup      = effectGrp
 * targetGroupIDs   = affectedGrps
 *
 */

// These are used in ModuleEffects.cpp to seperate effects into state containers
// *** these values are the 'state' bitfield (as integer) and tested during module object creation for effect states
enum EffectStates
{
    EFFECT_UNFITTED             = 0,
    EFFECT_OFFLINE              = 1,
    EFFECT_ONLINE               = 2,
    EFFECT_ACTIVATED            = 4,
    EFFECT_OVERLOADED           = 8,
    EFFECT_GANG                 = 16,
    EFFECT_FLEET                = 32,
    EFFECT_PASSIVE              = 64   // for character and system effects
};

// Target types to which module effects are applied when activated:
// *** these values are the 'targetType' field
enum EffectTargetTypes
{   // 0: zero value.  undefined effect.  this effect is not loaded (currently in testing or incomplete)
    EFFECT_UNDEFINED            = 0,
    // 1: the target is the ship to which the module is fitted
    EFFECT_SHIP                 = 1,
    // 2: the target is a module fit to same ship. use 'affectedGrps' to decode affected groups
    EFFECT_MODULE               = 2,
    // 3: the target is a charge.  use 'affectedGrps' to decode affected groups of charges
    EFFECT_CHARGE               = 3,
    // 4: the affected target is the current target of the ship the module is fitted to  (remotes)
    EFFECT_TARGET               = 4,
    // 7: passive effect which acts upon the character's attribute specific to the effect  (boosters and implants)
    EFFECT_CHARACTER            = 5,
    // 6: passive effect which acts upon all ships in the system with this beacon.  (wormholes and incursions)
    EFFECT_SYSTEM               = 6
};

//calculation types    rewrite 27Dec16    -allan
// *** these values are the 'calculation' and 'rCalculation' fields
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


#endif  // _EVE_FX_PROC_DATA_H__
