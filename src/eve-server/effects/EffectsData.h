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
    uint8 effectState;
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

struct fxData {
    int8 math;          // math used on data
    int8 fxSrc;        // effect source location
    int8 targLoc;       // effect target location
    uint16 targAttr;
    uint16 srcAttr;
    uint16 grpID;       // used to define items in env grouped by item groupID
    uint16 typeID;      // used to define items in env grouped by skill requirement
    InventoryItemRef srcRef;   // source item ref, if required
};

typedef std::map<uint16, Effect> effectMapType;

// these tables are used to decode fields in Effects table
namespace Effects {
    enum Source {   // formally known as domain
        dgmSrcInvalid          = -1,
        //  these define the location for group-, skill-, gang-, and owner-required effects
        dgmSrcSelf             = 0,
        dgmSrcSkill            = 1,
        dgmSrcShip             = 2,
        dgmSrcOwner            = 3,
        dgmSrcGang             = 4,
        dgmSrcGroup            = 5,
        dgmSrcTarget           = 6,
        MaxSrcLocation            = 6
    };

    enum TargetLocation {   //formally known as environment
        dgmTargLocInvalid      = -1,
        // these define the item containing the attribute to be modified
        //  these are found (as text) in the expressionValue field of dgmExpressions table and may need to merge with Association, or test with it
        dgmTargLocSelf         = 0,
        dgmTargLocChar         = 1,
        dgmTargLocShip         = 2,
        dgmTargLocTarget       = 3,
        dgmTargLocOther        = 4, //charges
        dgmTargLocArea         = 5,
        dgmTargLocPowerCore    = 6,  //defined but not used
        MaxTargLocation        = 6
    };

    enum State  {       // formally known as category
        dgmStateInvalid        = -1,
        // these are the effectState in dgmEffects table to denote when this effect is applied or removed
        dgmStatePassive        = 0, //Applied when item is just present in fit - implants, skills, offlined modules
        dgmStateActive         = 1, //also online effect - Applied when module is onlined
        dgmStateTarget         = 2, //Applied onto selected target
        dgmStateArea           = 3, //defined but not used
        dgmStateOnline         = 4, //Applied when module is activated
        dgmStateOverloaded     = 5, //Applied when module is overloaded
        dgmStateDungeon        = 6, //Dungeon effects, several effects exist in this category, but not assigned to any item
        dgmStateSystem         = 7,  //System-wide effects, like WH and incursion
        MaxState               = 7
    };

    enum Math {     // formally known as association
        dgmMathInvalid        = -1,
        // these define how the data is manipulated according to the format field in dgmOperands table
        dgmMathPreAssignment  = 0,
        dgmMathPreMul         = 1,
        dgmMathPreDiv         = 2,
        dgmMathModAdd         = 3,
        dgmMathModSub         = 4,
        dgmMathPostMul        = 5,
        dgmMathPostDiv        = 6,
        dgmMathPostPercent    = 7,
        dgmMathPostAssignment = 8,
        dgmMathSkillCheck     = 9,
        /* no data or expressions with these */
        dgmMathAddRate        = 10,
        dgmMathSubRate        = 11
    };
    /*  old shit
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
     */
    /*
    dgmUnnerfedCategories = [
        categorySkill,
        categoryImplant,
        categoryShip,
        categoryCharge,
        categorySubSystem]
    dgmPreStackingNerfOperators = {
        dgmMathPreAssignment: lambda ret, value: value,
        dgmMathPreMul: lambda ret, value: ret * value,
        dgmMathPreDiv: lambda ret, value: ret / value,
        dgmMathModAdd: lambda ret, value: ret + value,
        dgmMathModSub: lambda ret, value: ret - value}
    dgmOperators = {
        dgmMathPreAssignment: lambda ret, value: value,
        dgmMathPostAssignment: lambda ret, value: value,
        dgmMathPreMul: lambda ret, value: ret * value,
        dgmMathPostMul: lambda ret, value: ret * value,
        dgmMathPreDiv: lambda ret, value: ret / value,
        dgmMathPostDiv: lambda ret, value: ret / value,
        dgmMathModAdd: lambda ret, value: ret + value,
        dgmMathModSub: lambda ret, value: ret - value,
        dgmMathPostPercent: lambda ret, value: ret * (100 + value) / 100}
    dgmAttributesByIdx = {
        1: attributeIsOnline,
        2: attributeDamage,
        3: attributeCharge,
        4: attributeSkillPoints,
        5: attributeArmorDamage,
        6: attributeShieldCharge,
        7: attributeIsIncapacitated}

    UserErrors
        UE_OWNERID = 2
        UE_LOCID = 3
        UE_TYPEID = 4
        UE_TYPEID2 = 5
        UE_TYPEIDL = 29
        UE_BPTYPEID = 6
        UE_GROUPID = 7
        UE_GROUPID2 = 8
        UE_CATID = 9
        UE_CATID2 = 10
        UE_DGMATTR = 11
        UE_DGMFX = 12
        UE_DGMTYPEFX = 13
        UE_AMT = 18
        UE_AMT2 = 19
        UE_AMT3 = 20
        UE_DIST = 21
        UE_TYPEIDANDQUANTITY = 24
        UE_OWNERIDNICK = 25
        UE_ISK = 28
        UE_AUR = 30
    */
    /*  flags for ???
    dgmExprSkip = 0
    dgmExprOwner = 1
    dgmExprShip = 2
    dgmExprOwnerAndShip = 3
    */

    enum Operands {
        /** @note  '//*' denotes implemented */
        operandADD = 1,             //*
        operandAGGM = 2,            //*
        operandAGIM = 3,            //*
        operandAGORSM = 4,          //*
        operandAGRSM = 5,           //*
        operandAIM = 6,             //*
        operandALGM = 7,            //*
        operandALM = 8,             //*
        operandALRSM = 9,           //*
        operandAND = 10,            //*
        operandAORSM = 11,          //*
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
        operandEQ = 33,             //*
        operandGA = 34,             //*
        operandGET = 35,            //*
        operandGETTYPE = 36,        //*
        operandGM = 37,             //*
        operandGT = 38,             //*
        operandGTE = 39,            //*
        operandIA = 40,             //*
        operandIF = 41,             //*
        operandINC = 42,            //*
        operandINCN = 43,           //*
        operandLAUNCH = 44,
        operandLAUNCHDEFENDERMISSILE = 45,
        operandLAUNCHDRONE = 46,
        operandLAUNCHFOFMISSILE = 47,
        operandLG = 48,             //*
        operandLS = 49,             //*
        operandMINE = 50,
        operandMUL = 51,            //*
        operandOR = 52,             //*
        operandPOWERBOOST = 53,
        operandRGGM = 54,           //*
        operandRGIM = 55,           //*
        operandRGORSM = 56,         //*
        operandRGRSM = 57,          //*
        operandRIM = 58,            //*
        operandRLGM = 59,           //*
        operandRLM = 60,            //*
        operandRLRSM = 61,          //*
        operandRORSM = 62,          //*
        operandRS = 63,             //*
        operandRSA = 64,            //*
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
 * (21, 'DEFASSOCIATION', 'define attribute association type', '%(value)s', 0, 0, 1, 'const.dgmMath%(value)s')
 * (22, 'DEFATTRIBUTE', 'define attribute', '%(value)s', 0, 0, 2, '%(value)s')
 * (23, 'DEFBOOL', 'define bool constant', 'Bool(%(value)s)', 0, 0, 4, '%(value)s')
 * (24, 'DEFENVIDX', 'define environment index', 'Current%(value)s', 0, 0, 6, 'env[const.dgmTargLoc%(value)s]')
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

#endif  // _EVE_FX_PROC_DATA_H__
