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

#include "../eve-server.h"


// POD for effect data in mem objects
struct Effect {
    bool isOffensive;
    bool isAssistance;
    bool disallowAutoRepeat;
    bool isWarpSafe;
    uint16 effectID;
    uint16 effectCategory;
    uint16 preExpression;
    uint16 postExpression;
    uint16 npcUsageChanceAttributeID;
    uint16 npcActivationChanceAttributeID;
    uint16 fittingUsageChanceAttributeID;
    uint16 durationAttributeID;
    uint16 trackingSpeedAttributeID;
    uint16 dischargeAttributeID;
    uint16 rangeAttributeID;
    uint16 falloffAttributeID;
    float rangeChance;
    float electronicChance;
    float propulsionChance;
    std::string effectName;
    std::string guid;
};

struct Expression {
    uint16 id;
    uint16 operandID;
    uint16 arg1;
    uint16 arg2;
    uint16 expressionTypeID;
    uint16 expressionGroupID;
    uint16 expressionAttributeID;
    std::string expressionValue;
    std::string description;
    std::string expressionName;
};

struct Operand {
    uint8 resultCategoryID;
    uint16 arg1categoryID;
    uint16 arg2categoryID;
    std::string operandKey;
    std::string format;
};

struct TypeEffects {
    bool isDefault;
    uint16 effectID;
};

typedef std::map<uint16, Effect> effectMapType;

namespace Effects {
    // these tables are used to decode fields in Effects table
    enum Environment {
        dgmEnvInvalid     = -1,
        // these define the item containing the attribute [to modify(target)]/[data(source)]
        //  these are found (as text) in the expressionValue field of dgmExpressions table and may need to merge with Association, or test with it
        dgmEnvSelf = 0,
        dgmEnvChar = 1,
        dgmEnvShip = 2,
        dgmEnvTarget = 3,
        dgmEnvOther = 4,
        dgmEnvArea = 5
    };

    enum Category  {
        CAT_INVALID        = -1,
        // these are the effectCategory in dgmEffects table to denote when/where this effect is applied or removed
        CAT_PASSIVE        = 0,    //Applied when item is just present in fit - implants, skills, offlined modules
        CAT_ACTIVE         = 1,    //also online effectApplied only when module is activated
        CAT_TARGET         = 2,    //Applied onto selected target
        CAT_AREA           = 3,    //No effects with this category, so actual impact is unknown
        CAT_ONLINE         = 4,    //Applied when module is onlined
        CAT_OVERLOADED     = 5,    //Applied only when module is overloaded
        CAT_DUNGEON        = 6,    //Dungeon effects, several effects exist in this category, but not assigned to any item
        CAT_SYSTEM         = 7     //System-wide effects, like WH and incursion
    };

    enum Association {
        ASSOC_INVALID         = -1,
        // these define how the data is manipulated according to the format field in dgmOperands table
        ASSOC_PRE_ASSIGNMENT  = 0,
        ASSOC_PRE_MUL         = 1,
        ASSOC_PRE_DIV         = 2,
        ASSOC_MOD_ADD         = 3,
        ASSOC_MOD_SUB         = 4,
        ASSOC_POST_MUL        = 5,
        ASSOC_POST_DIV        = 6,
        ASSOC_POST_PERCENT    = 7,
        ASSOC_POST_ASSIGNMENT = 8,
        ASSOC_SKILL_CHECK     = 9,
        ASSOC_ADD_RATE        = 10,
        ASSOC_SUB_RATE        = 11
        /*
        dgmAssPreAssignment = -1
        dgmAssPreMul = 0
        dgmAssPreDiv = 1
        dgmAssModAdd = 2
        dgmAssModSub = 3
        dgmAssPostMul = 4
        dgmAssPostDiv = 5
        dgmAssPostPercent = 6
        dgmAssPostAssignment = 7
        */
    };

    /*
    dgmUnnerfedCategories = [
        categorySkill,
        categoryImplant,
        categoryShip,
        categoryCharge,
        categorySubSystem]
    dgmPreStackingNerfOperators = {
        dgmAssPreAssignment: lambda ret, value: value,
        dgmAssPreMul: lambda ret, value: ret * value,
        dgmAssPreDiv: lambda ret, value: ret / value,
        dgmAssModAdd: lambda ret, value: ret + value,
        dgmAssModSub: lambda ret, value: ret - value}
    dgmOperators = {
        dgmAssPreAssignment: lambda ret, value: value,
        dgmAssPostAssignment: lambda ret, value: value,
        dgmAssPreMul: lambda ret, value: ret * value,
        dgmAssPostMul: lambda ret, value: ret * value,
        dgmAssPreDiv: lambda ret, value: ret / value,
        dgmAssPostDiv: lambda ret, value: ret / value,
        dgmAssModAdd: lambda ret, value: ret + value,
        dgmAssModSub: lambda ret, value: ret - value,
        dgmAssPostPercent: lambda ret, value: ret * (100 + value) / 100}
    dgmAttributesByIdx = {
        1: attributeIsOnline,
        2: attributeDamage,
        3: attributeCharge,
        4: attributeSkillPoints,
        5: attributeArmorDamage,
        6: attributeShieldCharge,
        7: attributeIsIncapacitated}
    */
    /*  flags for ???
    dgmExprSkip = 0
    dgmExprOwner = 1
    dgmExprShip = 2
    dgmExprOwnerAndShip = 3
    */

    enum Operands {
        operandADD = 1,             //*
        operandAGGM = 2,
        operandAGIM = 3,
        operandAGORSM = 4,
        operandAGRSM = 5,
        operandAIM = 6,             //*
        operandALGM = 7,
        operandALM = 8,
        operandALRSM = 9,
        operandAND = 10,            //*
        operandAORSM = 11,
        operandATT = 12,            //*
        operandATTACK = 13,
        operandCARGOSCAN = 14,
        operandCHEATTELEDOCK = 15,
        operandCHEATTELEGATE = 16,
        operandCOMBINE = 17,        //*
        operandDEC = 18,            //*
        operandDECLOAKWAVE = 19,
        operandDECN = 20,           //*
        operandDEFASSOCIATION = 21, //*
        operandDEFATTRIBUTE = 22,   //*
        operandDEFBOOL = 23,        //*
        operandDEFENVIDX = 24,      //*
        operandDEFFLOAT = 25,       //*
        operandDEFGROUP = 26,       //*
        operandDEFINT = 27,         //*
        operandDEFSTRING = 28,      //*
        operandDEFTYPEID = 29,      //*
        operandECMBURST = 30,
        operandEFF = 31,            //*
        operandEMPWAVE = 32,
        operandEQ = 33,
        operandGA = 34,
        operandGET = 35,
        operandGETTYPE = 36,
        operandGM = 37,
        operandGT = 38,
        operandGTE = 39,
        operandIA = 40,
        operandIF = 41,             //*
        operandINC = 42,            //*
        operandINCN = 43,           //*
        operandLAUNCH = 44,
        operandLAUNCHDEFENDERMISSILE = 45,
        operandLAUNCHDRONE = 46,
        operandLAUNCHFOFMISSILE = 47,
        operandLG = 48,
        operandLS = 49,
        operandMINE = 50,
        operandMUL = 51,
        operandOR = 52,             //*
        operandPOWERBOOST = 53,
        operandRGGM = 54,
        operandRGIM = 55,
        operandRGORSM = 56,
        operandRGRSM = 57,
        operandRIM = 58,            //*
        operandRLGM = 59,
        operandRLM = 60,
        operandRLRSM = 61,
        operandRORSM = 62,
        operandRS = 63,             //*
        operandRSA = 64,
        operandSET = 65,            //*
        operandSHIPSCAN = 66,
        operandSKILLCHECK = 67,     //*
        operandSUB = 68,            //*
        operandSURVEYSCAN = 69,
        operandTARGETHOSTILES = 70,
        operandTARGETSILENTLY = 71,
        operandTOOLTARGETSKILLS = 72,
        operandUE = 73,             //*
        operandVERIFYTARGETGROUP = 74
    };
}


/* these are the operandID field in the dgmExpressions table
 *
 *  will need to hand-write code for these operands.
 * (operandID, operandKey, description, format, arg1categoryID, arg2categoryID, resultCategoryID, pythonFormat)
 *
 * (1, 'ADD', 'add two numbers', '(%(arg1)s)+(%(arg2)s)', 4, 4, 4, '(%(arg1)s)+(%(arg2)s)')
 * (2, 'AGGM', 'add gang group modifier', '[%(arg1)s].AGGM(%(arg2)s)', 5, 2, 4, 'dogma.AddGangGroupModifier(env,%(arg1)s, %(arg2)s)')
 * (3, 'AGIM', 'add gang ship modifier', '[%(arg1)s].AGIM(%(arg2)s)', 5, 2, 4, 'dogma.AddGangShipModifier(env,%(arg1)s, %(arg2)s)')
 * (4, 'AGORSM', 'add gang owner required skill modifier', '[%(arg1)s].AGORSM(%(arg2)s)', 5, 2, 4, 'dogma.AddGangOwnerRequiredSkillModifier(env,%(arg1)s, %(arg2)s)')
 * (5, 'AGRSM', 'add gang required skill modifier', '[%(arg1)s].AGRSM(%(arg2)s)', 5, 2, 4, 'dogma.AddGangRequiredSkillModifier(env,%(arg1)s, %(arg2)s)')
 * (6, 'AIM', 'add item modifier', '(%(arg1)s).AddItemModifier (%(arg2)s)', 5, 2, 4, 'dogma.AddItemModifier(env,%(arg1)s, %(arg2)s)')
 * (7, 'ALGM', 'add location group modifier', '(%(arg1)s).AddLocationGroupModifier (%(arg2)s)', 5, 2, 4, 'dogma.AddLocationGroupModifier(env,%(arg1)s, %(arg2)s)')
 * (8, 'ALM', 'add location modifier', '(%(arg1)s).AddLocationModifier (%(arg2)s)', 5, 2, 4, 'dogma.AddLocationModifier(env,%(arg1)s, %(arg2)s)')
 * (9, 'ALRSM', 'add location required skill modifier', '(%(arg1)s).ALRSM(%(arg2)s)', 5, 2, 4, 'dogma.AddLocationRequiredSkillModifier(env,%(arg1)s, %(arg2)s)')
 * (10, 'AND', 'logical and operation', '(%(arg1)s) AND (%(arg2)s)', 4, 4, 4, '(%(arg1)s and %(arg2)s)')
 * (11, 'AORSM', 'add owner required skill modifier', '(%(arg1)s).AORSM(%(arg2)s)', 5, 2, 4, 'dogma.AddOwnerRequiredSkillModifier(env,%(arg1)s, %(arg2)s)')
 * (12, 'ATT', 'attribute', '%(arg1)s->%(arg2)s', 6, 2, 3, '(%(arg1)s, %(arg2)s)')
 * (13, 'ATTACK', 'attack given ship', 'Attack', 2, 0, 4, 'Attack(env, %(arg1)s, %(arg2)s)')
 * (14, 'CARGOSCAN', 'Scans the cargo of the targeted ship.', 'CargoScan', 0, 0, 4, 'CargoScan(env, None, None)')
 * (15, 'CHEATTELEDOCK', 'Instantly enter a station.', 'CheatTeleDock()', 0, 0, 4, 'CheatTeleDock(env, None, None)')
 * (16, 'CHEATTELEGATE', 'Automatically invoke a stargate destination from remote distances.', 'CheatTeleGate()', 0, 0, 4, 'dogma.CheatTeleGate(env, None, None)')
 * (17, 'COMBINE', 'executes two statements', '%(arg1)s);     (%(arg2)s', 4, 4, 4, '%(arg1)s %(arg2)s')
 * (18, 'DEC', 'decreases an item-attribute by the value of another attribute', '%(arg1)s-=self.%(arg2)s', 3, 2, 4, ' ')
 * (19, 'DECLOAKWAVE', 'broadcasts a decloaking wave', 'DecloakWave', 0, 0, 4, 'DecloakWave(env, None, None)')
 * (20, 'DECN', 'decreases an item-attribute by number', '%(arg1)s-=%(arg2)s', 3, 4, 4, ' ')
 * (21, 'DEFASSOCIATION', 'define attribute association type', '%(value)s', 0, 0, 1, 'const.dgmAss%(value)s')
 * (22, 'DEFATTRIBUTE', 'define attribute', '%(value)s', 0, 0, 2, '%(value)s')
 * (23, 'DEFBOOL', 'define bool constant', 'Bool(%(value)s)', 0, 0, 4, '%(value)s')
 * (24, 'DEFENVIDX', 'define environment index', 'Current%(value)s', 0, 0, 6, 'env[const.dgmEnv%(value)s]')
 * (25, 'DEFFLOAT', 'defines a float constant', 'Float(%(value)s)', 0, 0, 4, ' ')
 * (26, 'DEFGROUP', 'define group', '%(value)s', 0, 0, 8, ' ')
 * (27, 'DEFINT', 'defines an int constant', 'Int(%(value)s)', 0, 0, 4, '%(value)s')
 * (28, 'DEFSTRING', 'defines a string constant', '\"%(value)s\"', 0, 0, 4, '\"%(value)s\"')
 * (29, 'DEFTYPEID', 'define a type ID', 'Type(%(value)s)', 0, 0, 9, ' ')
 * (30, 'ECMBURST', 'Clears all targets on all ships(excluding self) wihin range. ', 'ECMBurst()', 0, 0, 4, 'dogma.ECMBurst(env, None, None)')
 * (31, 'EFF', 'define association type', '(%(arg2)s).(%(arg1)s)', 1, 3, 5, '(%(arg1)s, %(arg2)s)')
 * (32, 'EMPWAVE', 'broadcasts an EMP wave', 'EMPWave', 0, 0, 4, 'EMPWave(env, None, None)')
 * (33, 'EQ', 'checks for equality', '%(arg1)s == %(arg2)s', 4, 4, 4, '(%(arg1)s == %(arg2)s)')
 * (34, 'GA', 'attribute on a module group', '%(arg1)s.%(arg2)s', 8, 2, 3, '(%(arg1)s, %(arg2)s)')
 * (35, 'GET', 'calculate attribute', '%(arg1)s.%(arg2)s()', 6, 2, 4, 'dogmaLM.GetAttributeValue(%(arg1)s, %(arg2)s)')
 * (36, 'GETTYPE', 'gets type of item', '%(arg1)s.GetTypeID()', 6, 0, 9, 'env.itemTypeID')
 * (37, 'GM', 'get a module of a given groupID from a given location (ship or player)', '%(arg1)s.%(arg2)s', 6, 8, 6, 'dogma.GetModule(env,%(arg1)s,%(arg2)s)')
 * (38, 'GT', 'checks whether expression 1  is greater than expression 2', '%(arg1)s> %(arg2)s', 4, 4, 4, '(%(arg1)s > %(arg2)s)')
 * (39, 'GTE', 'checks whether an expression is greater than or equal to another', '%(arg1)s>=%(arg2)s', 4, 4, 4, '(%(arg1)s >= %(arg2)s)')
 * (40, 'IA', 'generic attribute', '%(arg1)s', 2, 0, 3, '%(arg1)s')
 * (41, 'IF', 'if construct', 'If(%(arg1)s), Then (%(arg2)s)', 4, 4, 4, 'if %(arg1)s:%(arg2)s')
 * (42, 'INC', 'increases an item-attribute by the value of another attribute', '%(arg1)s+=self.%(arg2)s', 3, 2, 4, '<handled in code>')
 * (43, 'INCN', 'increases an item-attribute by a number', '%(arg1)s+=%(arg2)s', 3, 4, 4, ' ')
 * (44, 'LAUNCH', 'launches a missile', 'LaunchMissile()', 0, 0, 4, 'dogma.Launch(env, None, None)')
 * (45, 'LAUNCHDEFENDERMISSILE', 'launches a defender missile', 'LaunchDefenderMissile()', 0, 0, 4, 'dogma.LaunchDefenderMissile(env, None, None)')
 * (46, 'LAUNCHDRONE', 'launches a drone.', 'LaunchDrone()', 0, 0, 4, 'dogma.Launch(env, None, None)')
 * (47, 'LAUNCHFOFMISSILE', 'launches an FOF missile', 'LaunchFOFMissile()', 0, 0, 4, 'dogma.LaunchFOFMissile(env, None, None)')
 * (48, 'LG', 'specify a group in a location', '%(arg1)s..%(arg2)s', 6, 8, 6, '(%(arg1)s, %(arg2)s)')
 * (49, 'LS', 'location - skill required item group', '%(arg1)s[%(arg2)s]', 6, 9, 6, '(%(arg1)s, %(arg2)s)')
 * (50, 'MINE', 'mines an asteroid', 'Mine', 0, 0, 4, 'Mine(env, None, None)')
 * (51, 'MUL', 'multiplies two numbers', '(%(arg1)s)*(%(arg2)s)', 4, 4, 4, '(%(arg1)s * %(arg2)s)')
 * (52, 'OR', 'logical or operation', '%(arg1)s OR %(arg2)s', 4, 4, 4, '(%(arg1)s or %(arg2)s)')
 * (53, 'POWERBOOST', '', 'PowerBoost', 0, 0, 4, 'dogma.PowerBoost(env, None, None)')
 * (54, 'RGGM', 'remove gang group modifier', '[%(arg1)s].RGGM(%(arg2)s)', 5, 2, 4, 'dogma.RemoveGangGroupModifier(env,%(arg1)s, %(arg2)s)')
 * (55, 'RGIM', 'remove gang ship modifier', '[%(arg1)s].RGIM(%(arg2)s)', 5, 2, 4, 'dogma.RemoveGangShipModifier(env,%(arg1)s, %(arg2)s)')
 * (56, 'RGORSM', 'remove a gang owner required skill modifier', '[%(arg1)s].RGORSM(%(arg2)s)', 5, 2, 4, 'dogma.RemoveGangOwnerRequiredSkillModifier(env,%(arg1)s, %(arg2)s)')
 * (57, 'RGRSM', 'remove a gang required skill modifier', '[%(arg1)s].RGRSM(%(arg2)s)', 5, 2, 4, 'dogma.RemoveGangRequiredSkillModifier(env,%(arg1)s, %(arg2)s)')
 * (58, 'RIM', 'remove  item modifier', '(%(arg1)s).RemoveItemModifier (%(arg2)s)', 5, 2, 4, 'dogma.RemoveItemModifier(env,%(arg1)s, %(arg2)s)')
 * (59, 'RLGM', 'remove location group modifier', '(%(arg1)s).RemoveLocationGroupModifier (%(arg2)s)', 5, 2, 4, 'dogma.RemoveLocationGroupModifier(env,%(arg1)s, %(arg2)s)')
 * (60, 'RLM', 'remove location modifier', '(%(arg1)s).RemoveLocationModifier (%(arg2)s)', 5, 2, 4, 'dogma.RemoveLocationModifier(env,%(arg1)s, %(arg2)s)')
 * (61, 'RLRSM', 'remove a  required skill modifier', '(%(arg1)s).RLRSM(%(arg2)s)', 5, 2, 4, 'dogma.RemoveLocationRequiredSkillModifier(env,%(arg1)s, %(arg2)s)')
 * (62, 'RORSM', 'remove an owner required skill modifier', '(%(arg1)s).RORSM(%(arg2)s)', 5, 2, 4, 'dogma.RemoveOwnerRequiredSkillModifier(env,%(arg1)s, %(arg2)s)')
 * (63, 'RS', 'true if arg1 requires arg2', '%(arg1)s.Requires(%(arg2)s)', 6, 9, 4, 'dogma.RequireSkill(env, %(arg1)s, %(arg2)s)')
 * (64, 'RSA', 'attribute on modules that have required skill', '%(arg1)s.%(arg2)s', 9, 2, 3, '(%(arg1)s, %(arg2)s)')
 * (65, 'SET', 'sets an item attribute', '%(arg1)s := %(arg2)s', 3, 4, 4, '<handled in code>')
 * (66, 'SHIPSCAN', 'scans a ship', 'ShipScan()', 0, 0, 4, 'ShipScan(env, None, None)')
 * (67, 'SKILLCHECK', '', 'SkillCheck(%(arg1)s)', 4, 0, 4, 'dogma.SkillCheck(env, %(arg1)s, %(arg2)s)')
 * (68, 'SUB', 'subtracts a number from another one', '%(arg1)s-%(arg2)s', 4, 4, 4, ' ')
 * (69, 'SURVEYSCAN', 'scans an asteroid for information', 'SurveyScan()', 0, 0, 4, 'SurveyScan(env, None, None)')
 * (70, 'TARGETHOSTILES', 'Targets any hostile ships within range (assuming electronics have capability).', 'TargetHostiles()', 0, 0, 4, 'dogma.TargetHostiles(env, None, None)')
 * (71, 'TARGETSILENTLY', '', 'TargetSilently()', 0, 0, 4, 'dogmaLM.AddTargetEx(shipID,targetID, silent=1, tasklet=1)')
 * (72, 'TOOLTARGETSKILLS', ' ', 'CheckToolTargetSkills', 0, 0, 4, 'dogma.CheckToolTargetSkills(env,None,None)')
 * (73, 'UE', 'raises an user error', 'UserError(%(arg1)s)', 4, 0, 4, 'raise UserError(%(arg1)s)')
 * (74, 'VERIFYTARGETGROUP', 'raises a user error if incorrect target group', 'VerifyTargetGroup()', 0, 0, 4, 'dogma.VerifyTargetGroup(env, None, None)');
 */

////////////////////////////  OLD MODULE EFFECTS CODE/DATA....PHASING OUT /////
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
    /*      new shit
        // these are the effectCategory in dgmEffects table to denote when/where this effect is applied or removed
        CAT_PASSIVE        = 0,    //Applied when item is just present in fit - implants, skills, offlined modules
        CAT_ACTIVE         = 1,    //also online effectApplied only when module is activated
        CAT_TARGET         = 2,    //Applied onto selected target
        CAT_AREA           = 3,    //No effects with this category, so actual impact is unknown
        CAT_ONLINE         = 4,    //Applied when module at least onlined
        CAT_OVERLOADED     = 5,    //Applied only when module is overloaded
        CAT_DUNGEON        = 6,    //Dungeon effects, several effects exist in this category, but not assigned to any item
        CAT_SYSTEM         = 7     //System-wide effects, like WH and incursion
    */
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
    /*          new shit
        ENV_NONE        = 0,
        ENV_SELF        = 1,
        ENV_CHAR        = 2,
        ENV_SHIP        = 3,
        ENV_TARGET      = 4,
        ENV_AREA        = 5,
        ENV_OTHER       = 6
    */
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
    CALC_SUBTRACT_RESIST        = 10
    /*      new shit
        ASSOC_PRE_ASSIGNMENT  = 0,
        ASSOC_PRE_MUL         = 1,
        ASSOC_PRE_DIV         = 2,
        ASSOC_MOD_ADD         = 3,
        ASSOC_MOD_SUB         = 4,
        ASSOC_POST_MUL        = 5,
        ASSOC_POST_DIV        = 6,
        ASSOC_POST_PERCENT    = 7,
        ASSOC_POST_ASSIGNMENT = 8,
        ASSOC_SKILL_TIME      = 9,
        ASSOC_ADD_RATE        = 10,
        ASSOC_SUB_RATE        = 11
    */
};

#endif  // _EVE_FX_PROC_DATA_H__
