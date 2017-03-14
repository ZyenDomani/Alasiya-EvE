/**
 * @name EffectsProcessor.cpp
 *   This file is for decoding and processing the effect data
 *   Copyright 2017  Alasiya-EVEmu Team
 *
 * @Author:    Allan
 * @date:      24 January 2017
 *
 */

#include "Client.h"
#include "effects/EffectsProcessor.h"
#include "inventory/InventoryItem.h"
#include "character/Character.h"
#include "ship/Ship.h"

/*
 * # Effects Logging:
 * EFFECTS=0
 * EFFECTS__ERROR=1
 * EFFECTS__WARNING=0
 * EFFECTS__MESSAGE=0
 * EFFECTS__DEBUG=0
 * EFFECTS__TRACE=0
 */

EvilNumber FxProc::CalculateAttributeValue(EvilNumber val1, EvilNumber val2, int8 method)
{
    if (val2 == 0)
        return val1;
    using namespace Effects;
    switch (method) {
        case dgmMathInvalid:
            _log(EFFECTS__WARNING, "FxProc::CalculateNewAttributeValue() - Invalid Association used");
        case dgmMathSkillCheck:
        case dgmMathPreAssignment:
        case dgmMathPostAssignment:
            return val1;
        case dgmMathPreMul:
        case dgmMathPostMul:
            return val1 * val2;
        case dgmMathPreDiv:
        case dgmMathPostDiv:
            return val1 / val2;
        case dgmMathModAdd:
            return val1 + val2;
        case dgmMathModSub:
            return val1 - val2;
        case dgmMathPostPercent:
            return val1 * ((100 + val2) / 100);
    }
    _log(EFFECTS__ERROR, "FxProc::CalculateNewAttributeValue() - Unknown Association used: %i", (int8)method);
    return 0;
}

int8 FxProc::GetAssociationEnum(const std::string& association)
{   // opID 21
    using namespace Effects;
    if (association == "PreAssignment")
        return dgmMathPreAssignment;
    else if (association == "PreDiv")
        return dgmMathPreDiv;
    else if (association == "PreMul")
        return dgmMathPreMul;
    else if (association == "ModAdd")
        return dgmMathModAdd;
    else if (association == "ModSub")
        return dgmMathModSub;
    else if (association == "PostPercent")
        return dgmMathPostPercent;
    else if (association == "PostMul")
        return dgmMathPostMul;
    else if (association == "PostDiv")
        return dgmMathPostDiv;
    else if (association == "PostAssignment")
        return dgmMathPostAssignment;
    else if (association == "SkillCheck")
        return dgmMathSkillCheck;
    else if (association == "AddRate")
        return dgmMathAddRate;
    else if (association == "SubRate")
        return dgmMathSubRate;
    else
        return dgmMathInvalid;  //throw std::bad_typeid();
}

int8 FxProc::GetEnvironmentEnum(const std::string& env)
{   // opID 24
    using namespace Effects;
    if (env == "Self")
        return dgmTargLocSelf;
    else if (env == "Char")
        return dgmTargLocChar;
    else if (env == "Ship")
        return dgmTargLocShip;
    else if (env == "Target")
        return dgmTargLocTarget;
    else if (env == "Area")
        return dgmTargLocArea;
    else if (env == "Other")
        return dgmTargLocOther;
    else if (env == "Charge")
        return dgmTargLocCharge;
    else
        return dgmTargLocInvalid;  //throw std::bad_typeid();
}

std::string FxProc::GetMathMethodName(int8 id)
{
    using namespace Effects;
    switch (id) {
        case dgmMathPreAssignment:   return "PreAssignment";
        case dgmMathPreDiv:          return "PreDiv";
        case dgmMathPreMul:          return "PreMul";
        case dgmMathModAdd:          return "ModAdd";
        case dgmMathModSub:          return "ModSub";
        case dgmMathPostPercent:     return "PostPercent";
        case dgmMathPostMul:         return "PostMul";
        case dgmMathPostDiv:         return "PostDiv";
        case dgmMathPostAssignment:  return "PostAssignment";
        case dgmMathSkillCheck:      return "SkillCheck";
        case dgmMathAddRate:         return "AddRate";
        case dgmMathSubRate:         return "SubRate";
        case dgmMathInvalid:
        default:                    return "Invalid";
    }
}

std::string FxProc::GetSourceName(int8 id)
{
    using namespace Effects;
    switch (id) {
        case dgmSrcSelf:         return "Self";
        case dgmSrcSkill:        return "Skill";
        case dgmSrcShip:         return "Ship";
        case dgmSrcOwner:        return "Owner";
        case dgmSrcGang:         return "Gang";
        case dgmSrcGroup:        return "Group";
        case dgmSrcTarget:       return "Target";
        case dgmSrcInvalid:
        default:                    return "Invalid";
    }
}

std::string FxProc::GetTargLocName(int8 id)
{
    using namespace Effects;
    switch (id) {
        case dgmTargLocSelf:            return "Self";
        case dgmTargLocChar:            return "Char";
        case dgmTargLocShip:            return "Ship";
        case dgmTargLocTarget:          return "Target";
        case dgmTargLocArea:            return "Area";
        case dgmTargLocOther:           return "Other";
        case dgmTargLocCharge:          return "Charge";
        case dgmTargLocInvalid:
        default:                    return "Invalid";
    }
}

void FxProc::EvaluateExpression(const uint16 expID)
{
    std::string res = "\n";
    res += DecodeExpression(sFxDataMgr.GetExpression(expID), false, true);
    sLog.Green("EvaluateExpression", "expID %u: %s", expID, res.c_str());
}

std::string FxProc::DecodeExpression(Expression expression, bool restricted/*false*/, bool topLevel/*false*/)
{
    using namespace Effects;
    std::ostringstream ret;
    switch(expression.operandID) {
        case operandUE: {   //UserError
            ret << expression.expressionName;
            /*
             ret << "UserError(env";
             std::string a1 = "", a2 = "";
             if (expression.arg1) {
                 a1 = DecodeExpression(sFxDataMgr.GetExpression(expression.arg1));
                 ret << ", " << a1;
                 if (expression.arg2) {
                     a2 = DecodeExpression(sFxDataMgr.GetExpression(expression.arg2));
                     ret << ", " << a2;
                }
            } else
                ret << ", NULL";
            ret << ")";
        */
        } break;
        // these return the provided expressionValue
        case operandDEFASSOCIATION:
        case operandDEFENVIDX:
        case operandDEFBOOL:    // this evaulates to 'true' (Bool(1))
        case operandDEFINT: {   // this is used as  0,1,2,{raceID}
            //if (topLevel)
                ret << expression.expressionValue;
        } break;
        case operandDEFFLOAT:   // not used
        case operandDEFSTRING: { // errors and SkillCheck()
            if (expression.expressionValue != "")
                ret << expression.expressionValue;
        } break;
        // these provide the given expression*ID
        case operandDEFATTRIBUTE: {    //
            if (expression.expressionAttributeID)
                ret << expression.expressionAttributeID;
        } break;
        case operandDEFGROUP: {    //
            if (expression.expressionGroupID)
                ret << expression.expressionGroupID;
        } break;
        case operandDEFTYPEID: {    //
            if (expression.expressionTypeID)
                ret << expression.expressionTypeID;
        } break;
        // do as stated
        case operandCOMBINE: { // executes two statements  '%(arg1)s); (%(arg2)s'
            if (expression.arg1)
                ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1), restricted) << "\n";
            if (expression.arg2)
                ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2), restricted);
        } break;
        case operandEFF: {      //define association type  '(%(arg2)s).(%(arg1)s)'
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1), restricted);
            ret << ", " << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2), restricted);
        } break;
        // these are trivial attribute operations
        case operandATT: {      //'%(arg1)s->%(arg2)s'      (domain:attribID)
            std::string arg1 = "nil", arg2 = "nil";
            if (expression.arg1)
                arg1 = DecodeExpression(sFxDataMgr.GetExpression(expression.arg1), restricted);
            if (expression.arg2)
                arg2 = DecodeExpression(sFxDataMgr.GetExpression(expression.arg2), restricted);
            ret << "(" << arg1 << ":" << arg2 << ")";
        } break;
        case operandADD: {      //'(%(arg1)s)+(%(arg2)s)'
            std::string arg1 = "nil", arg2 = "nil";
            if (expression.arg1)
                arg1 = DecodeExpression(sFxDataMgr.GetExpression(expression.arg1), restricted);
            if (expression.arg2)
                arg2 = DecodeExpression(sFxDataMgr.GetExpression(expression.arg2), restricted);
            ret << "(" << arg1 << "+" << arg2 << ")";
        } break;
        case operandSUB: {      //'(%(arg1)s)-(%(arg2)s)'
            std::string arg1 = "nil", arg2 = "nil";
            if (expression.arg1)
                arg1 = DecodeExpression(sFxDataMgr.GetExpression(expression.arg1), restricted);
            if (expression.arg2)
                arg2 = DecodeExpression(sFxDataMgr.GetExpression(expression.arg2), restricted);
            ret << "(" << arg1 << "-" << arg2 << ")";
        } break;
        case operandMUL: {    //'(%(arg1)s)*(%(arg2)s)'
            std::string arg1 = "nil", arg2 = "nil";
            if (expression.arg1)
                arg1 = DecodeExpression(sFxDataMgr.GetExpression(expression.arg1), restricted);
            if (expression.arg2)
                arg2 = DecodeExpression(sFxDataMgr.GetExpression(expression.arg2), restricted);
            ret << "(" << arg1 << "*" << arg2 << ")";
        } break;
        case operandEQ: {    //'%(arg1)s == %(arg2)s'
            ret << "(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << "==";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandGT: {    //'%(arg1)s> %(arg2)s'
            ret << "(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ">";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandGTE: {   //'%(arg1)s>=%(arg2)s'
            ret << "(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ">=";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandINC: {      //'%(arg1)s+=self.%(arg2)s'
            ret << "(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1), restricted) << " += (Self:";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2), restricted) << "))";
        } break;
        case operandINCN: {     //'%(arg1)s+=%(arg2)s'
            ret << "(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1), restricted) << " += (Self:";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2), restricted) << ")";
        } break;
        case operandDEC: {      //'%(arg1)s-=self.%(arg2)s'
            ret << "(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1), restricted) << " -= (Self:";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2), restricted) << "))";
        } break;
        case operandDECN: {     //'%(arg1)s-=%(arg2)s'
            ret << "(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1), restricted) << " -= (Self:";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2), restricted) << ")";
        } break;

        // effect function calls.   handled in ShipItem class in Ship.cpp
        case operandSKILLCHECK: { //'dogma.SkillCheck(env, %(arg1)s, %(arg2)s)'
            ret << "SkillCheck(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1), restricted);
            if (expression.arg2) {
                ret << ", " << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2), restricted);
            }
            ret << ")";
        } break;
        case operandAIM: {    //
            //'dogma.AddItemModifier(env, %(arg1)s, %(arg2)s)'
            Expression arg1Expression = sFxDataMgr.GetExpression(expression.arg1);
            ret << "AddItemModifier(" << DecodeExpression(sFxDataMgr.GetExpression(arg1Expression.arg1));
            ret << ", " << DecodeExpression(sFxDataMgr.GetExpression(arg1Expression.arg2)) << ", (Self:";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << "))";
        } break;
        case operandRIM: {    //
            //'dogma.RemoveItemModifier(env, %(arg1)s, %(arg2)s)'
            Expression arg1Expression = sFxDataMgr.GetExpression(expression.arg1);
            ret << "RemoveItemModifier(" << DecodeExpression(sFxDataMgr.GetExpression(arg1Expression.arg1));
            ret << ", " << DecodeExpression(sFxDataMgr.GetExpression(arg1Expression.arg2)) << ", (Self:";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << "))";
        } break;
        case operandAGGM: {    //2,[%(arg1)s].AGGM(%(arg2)s)    --not used
            ret << "AddGangGroupModifier(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandAGIM: {    //3,[%(arg1)s].AGIM(%(arg2)s)
            ret << "AddGangShipModifier(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandAGORSM: {    //4, [%(arg1)s].AGORSM(%(arg2)s)   --not used
            ret << "AddGangOwnerRequiredSkillModifier(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandAGRSM: {    //5,  [%(arg1)s].AGRSM(%(arg2)s)
            ret << "AddGangRequiredSkillModifier(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandALGM: {    //7,(%(arg1)s).AddLocationGroupModifier (%(arg2)s)
            ret << "AddLocationGroupModifier(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandALM: {    //8,(%(arg1)s).AddLocationModifier (%(arg2)s)
            ret << "AddLocationModifier(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandALRSM: {    //9,(%(arg1)s).ALRSM(%(arg2)s)
            ret << "AddLocationRequiredSkillModifier(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandAORSM: {    //11,(%(arg1)s).AORSM(%(arg2)s)
            ret << "AddOwnerRequiredSkillModifier(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRGGM: {    //54, [%(arg1)s].RGGM(%(arg2)s)
            ret << "RemoveGangGroupModifier(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRGIM: {    //55,[%(arg1)s].RGIM(%(arg2)s)
            ret << "RemoveGangShipModifier(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRGORSM: {    //56,[%(arg1)s].RGORSM(%(arg2)s)
            ret << "RemoveGangOwnerRequiredSkillModifier(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRGRSM: {    //57,[%(arg1)s].RGRSM(%(arg2)s)
            ret << "RemoveGangRequiredSkillModifier(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRLGM: {    //59,(%(arg1)s).RemoveLocationGroupModifier (%(arg2)s)
            ret << "RemoveLocationGroupModifier(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRLM: {    //60, (%(arg1)s).RemoveLocationModifier (%(arg2)s)
            ret << "RemoveLocationModifier(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRLRSM: {    //61,(%(arg1)s).RLRSM(%(arg2)s)
            ret << "RemoveLocationRequiredSkillModifier(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRORSM: {    //62, (%(arg1)s).RORSM(%(arg2)s)
            ret << "RemoveOwnerRequiredSkillModifier(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRS: {    //63, %(arg1)s.Requires(%(arg2)s)  --not used
            ret << "(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ".Requires(";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRSA: {    //64, %(arg1)s.%(arg2)s      -- used by AGRSM/RGRSM  ** NEEDS WORK **
            ret << "RequiredSkillAttribute(typeID(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << "):";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        // these function calls are a bit more complicated...will need more work and better understanding
        case operandGA: {    //'%(arg1)s.%(arg2)s'      --not used
            ret << "ModuleGroupAttrib(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ".";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandGM: {    //'%(arg1)s.%(arg2)s'      --used by subsystems as GetModule(Ship:201)
            ret << "GetModule(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ":";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandGET: {    //'%(arg1)s.%(arg2)s()'   --used a lot.  eg. GetAttribute(Ship:101)
            ret << "GetAttribute(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ":";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandGETTYPE: {    //'%(arg1)s.GetTypeID()'  --used by SRLG in AORSM/RORSM
            ret << "GetTypeID(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ")";
        } break;
        case operandIA: {    //'%(arg1)s'   -used by AGIM
            ret << "attribute(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << ")";
        } break;
        case operandLG: {    //48, %(arg1)s..%(arg2)s  -- specify a group by grpID in a location'
            ret << "LocationGroup(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << "[";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << "])";
        } break;
        case operandLS: {    //49, %(arg1)s[%(arg2)s]  -- specify a group by skillID in a location
            ret << "SkillRequiredLocationGroup(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1)) << "[";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2)) << "])";
        } break;
        case operandSET: {      //'%(arg1)s := %(arg2)s'        --used by online/offline for all moodules
            ret << "SetAttribute(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1), restricted) << " = ";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2), restricted) << ")";
        } break;
        // these will need to work to properly code conditionals here
        case operandOR: {       //'%(arg1)s OR %(arg2)s'    -- usually used with 'if'  (if x ... OR y)  (used as "else" or elif)
            ret << "(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1), restricted) << ")\nOR (";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2), restricted) << ")";
        } break;
        case operandAND: {      //'(%(arg1)s) AND (%(arg2)s)'   -- usually used with 'if'  (if x AND y then ....)
            ret << "(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1), restricted) << ")\nAND (";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2), restricted) << ")";
        } break;
        case operandIF: { //'If(%(arg1)s), Then (%(arg2)s)'     --std conditional.  if x then y
            ret << "IF(" << DecodeExpression(sFxDataMgr.GetExpression(expression.arg1), restricted) << ")\nTHEN (";
            ret << DecodeExpression(sFxDataMgr.GetExpression(expression.arg2), restricted) << ")";
        } break;
        // effect action calls.  handled in dogmaLM
        case operandATTACK:     //13,
        case operandCARGOSCAN:     //14,
        case operandCHEATTELEDOCK:     //15,
        case operandCHEATTELEGATE:     //16,
        case operandDECLOAKWAVE:     //19,
        case operandECMBURST:     //30,
        case operandEMPWAVE:     //32,
        case operandLAUNCH:     //44,
        case operandLAUNCHDEFENDERMISSILE:     //45,
        case operandLAUNCHDRONE:     //46,
        case operandLAUNCHFOFMISSILE:     //47,
        case operandMINE:     //50,
        case operandPOWERBOOST:     //53,
        case operandSHIPSCAN:     //66,
        case operandSURVEYSCAN:     //69,
        case operandTARGETHOSTILES:     //70,
        case operandTARGETSILENTLY:     //71,
        case operandTOOLTARGETSKILLS:     //72,
        case operandVERIFYTARGETGROUP: {    //74
            ret << sFxDataMgr.GetOperand(expression.operandID).format;
        } break;
        default: {              // in case the op hasnt been defined, make a note here (should not hit)
            Operand operand = sFxDataMgr.GetOperand(expression.operandID);
            ret << "Operand id:" << expression.operandID << " key: " << operand.operandKey;
            if (operand.format == "")
                ret << " - has not been defined";
            else                // % {'arg1': arg1, 'arg2': arg2, 'value': expression.expressionValue}
                ret << " *needsWork*";
        } break;
    }
    if (ret == "") {            // check for empty returns
        if (expression.operandID == operandDEFGROUP) {
            ret << "*groupNameByID*" << expression.expressionValue; // get group name by id here
        } else if (expression.operandID == operandDEFTYPEID) {
            ret << "*typeNameByID*" << expression.expressionValue; // get type name by id here
        } else {    //not used
            std::string arg1 = "nil", arg2 = "nil";
            if (expression.arg1)
                arg1 = DecodeExpression(sFxDataMgr.GetExpression(expression.arg1), restricted);
            if (expression.arg2)
                arg2 = DecodeExpression(sFxDataMgr.GetExpression(expression.arg2), restricted);
            if ((arg1 != "") and (arg1.find_first_of('\n') > 1))
                ret << "*arg1Newline*" ;   //arg1 = arg1.strip();
            if ((arg2 != "") and (arg2.find_first_of('\n') > 1))
                ret << "*arg2Newline*" ;   //arg2 = arg2.strip();
        }
    }

    return ret.str().c_str();
}

void FxProc::ParseExpression(InventoryItem* pItem, Expression expression, fxData& data)
{
    using namespace Effects;
    switch(expression.operandID) {
        // trivial attribute operations
        case operandATT: {      //12,'%(arg1)s->%(arg2)s'      (item:attribID)
            if (expression.arg1)
                ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data);
            if (expression.arg2)
                ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data);
        } break;
        // these return the given expressionValue
        case operandDEFBOOL:    //23 this evaulates to 'true' (Bool(1))
        case operandDEFINT: {   //27 this is used as  0,1,2,{raceID}
            // not sure what to do here
            //expression.expressionValue;
        } break;
        case operandDEFASSOCIATION: {   //21
            data.math = sFxProc.GetAssociationEnum(expression.expressionValue);
            if (data.math > MaxMathMethod) {
                Operand operand = sFxDataMgr.GetOperand(expression.operandID);
                _log(EFFECTS__ERROR, "FxProc::ParseExpression(): out of range assoc: %i for operand %u (%s).", \
                data.targAttr, expression.operandID, operand.operandKey.c_str());
            }
        } break;
        case operandDEFENVIDX: {    //24
            data.targLoc = sFxProc.GetEnvironmentEnum(expression.expressionValue);
            if (data.targLoc > MaxTargLocation) {
                Operand operand = sFxDataMgr.GetOperand(expression.operandID);
                _log(EFFECTS__ERROR, "FxProc::ParseExpression(): out of range env: %i for operand %u (%s).", \
                data.targAttr, expression.operandID, operand.operandKey.c_str());
            }
        } break;
        // these provide the given expression*ID
        case operandDEFATTRIBUTE: {    //22
            if (expression.expressionAttributeID) {
                /*
                if (expression.expressionAttributeID > 1817) {  //2003 -max Rhea value;  1817 -max Crucible value
                    std::string type = "targ";
                    if (data.targAttr)
                        type = "src";
                    Operand operand = sFxDataMgr.GetOperand(expression.operandID);
                    _log(EFFECTS__ERROR, "FxProc::ParseExpression(): out of range %sAttr: %u > 1817 for operand %u (%s).", \
                            type.c_str(), data.targAttr, expression.operandID, operand.operandKey.c_str());
                }*/
                if (data.targAttr) { // always processed first
                    data.srcAttr = expression.expressionAttributeID;
                    if (expression.expressionName == "skillLevel")
                        data.fxSrc = dgmSrcSkill;
                } else
                    data.targAttr = expression.expressionAttributeID;
            }
        } break;
        case operandDEFGROUP: {    //26
            if (expression.expressionGroupID) {
                data.grpID = expression.expressionGroupID;
            } else if (expression.expressionValue != "") {
                ;   // will have to figure out how to do this one.
                _log(EFFECTS__WARNING, "FxProc::ParseExpression(): operandDEFGROUP called using expressionValue");
            } else {
                _log(EFFECTS__ERROR, "FxProc::ParseExpression(): operandDEFGROUP called with no expressionGroupID or expressionValue defined");
            }
        } break;
        case operandDEFTYPEID: {    //29
            if (expression.expressionTypeID) {
                data.typeID = expression.expressionTypeID;
            } else if (expression.expressionValue != "") {
                ;   // will have to figure out how to do this one.
                _log(EFFECTS__WARNING, "FxProc::ParseExpression(): operandDEFTYPEID called using expressionValue");
            } else {
                _log(EFFECTS__ERROR, "FxProc::ParseExpression(): operandDEFTYPEID called with no expressionTypeID or expressionValue defined");
            }
        } break;
        // do as stated
        case operandCOMBINE: { //17,'%(arg1)s); (%(arg2)s'      --executes two statements
            if (expression.arg1)
                ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data);
            fxData data1;
            data1.srcRef = data.srcRef;
            data1.math = data1.targLoc = data1.fxSrc = data1.targAttr = data1.srcAttr = data1.grpID = data1.typeID = 0;
            if (expression.arg2)
                ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data1);
        } break;
        case operandEFF: {      //31, '(%(arg2)s).(%(arg1)s)'       --define association type
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data);
        } break;
        case operandGETTYPE: {    //36,'%(arg1)s.GetTypeID()'  --used by SRLG in AORSM
            if (!data.typeID)
                data.typeID = data.srcRef->typeID();    // get modules and charges on ship that require SkillItem in srcRef
            else
                _log(EFFECTS__ERROR, "FxProc::ParseExpression(): GetType called when typeID previously defined.");
        } break;
        case operandLG: {    //48, '%(arg1)s.LocationGroup.%(arg2)s'  -- specify a group by grpID for a location'  used by ALGM
            if (data.fxSrc)
                _log(EFFECTS__ERROR, "FxProc::ParseExpression(): LocationGroup called when fxSrc previously defined.");
            data.fxSrc = dgmSrcGroup;
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data);   //source
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data);   //groupID
        } break;
        case operandLS: {    //49, '%(arg1)s.SkillRequiredLocationGroup[%(arg2)s]'  --  specify a group by skillID for a location   used by ALRSM and AORSM
            if (data.fxSrc)
                _log(EFFECTS__ERROR, "FxProc::ParseExpression(): SkillRequiredLocationGroup called when fxSrc previously defined.");
            data.fxSrc = dgmSrcSkill;
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data);   //source
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data);   //skillID
        } break;
        case operandGA: {    //34,'%(arg1)s.%(arg2)s'       --GetAttribute      (no known uses)
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data);
        } break;
        case operandGET: {    //35,'%(arg1)s.%(arg2)s()'   --used a lot.  eg. Get(Ship:101) means 'get attribute 101 on ShipItem'
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data);
        } break;
        case operandIA: {    //40,'%(arg1)s'   -used by AGIM
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data);
        } break;
        case operandGM: {    //37,'%(arg1)s.GetModule(%(arg2)s)'      --used by subsystems as (GetModule(Ship:201):55)
            if (data.fxSrc)
                _log(EFFECTS__ERROR, "FxProc::ParseExpression(): GetModule called when fxSrc previously defined.");
            data.fxSrc = dgmSrcShip;
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data);
        } break;
        // effect function calls.
        // here is where we'll actually add the modifier data to the item's map
        case operandAIM: {    //6,'AddItemModifier(env,%(arg1)s, %(arg2)s)'
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data);
            pItem->AddModifier(data);
        } break;
        case operandALGM: {    //7,(%(arg1)s).AddLocationGroupModifier (%(arg2)s)
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data);
            pItem->AddModifier(data);
        } break;
        case operandALM: {    //8,(%(arg1)s).AddLocationModifier (%(arg2)s)
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data);
            pItem->AddModifier(data);
        } break;
        case operandALRSM: {    //9,(%(arg1)s).AddLocationRequiredSkillModifier(%(arg2)s)
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data);
            pItem->AddModifier(data);
        } break;
        case operandAORSM: {    //11,(%(arg1)s).AddOwnerRequiredSkillModifier(%(arg2)s)
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data);
            pItem->AddModifier(data);
        } break;
        case operandAGRSM: {    //5,  [%(arg1)s].AGRSM(%(arg2)s)    --AddGangRequiredSkillModifier
            if (data.fxSrc)
                _log(EFFECTS__ERROR, "FxProc::ParseExpression(): AddGangRequiredSkillModifier called when fxSrc previously defined.");
            data.fxSrc = dgmSrcGang;
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data);
            pItem->AddModifier(data);
        } break;
        case operandAGIM: {    //3,[%(arg1)s].AGIM(%(arg2)s)        --AddGangShipModifier
            if (data.fxSrc)
                _log(EFFECTS__ERROR, "FxProc::ParseExpression(): AddGangShipModifier called when fxSrc previously defined.");
            data.fxSrc = dgmSrcGang;
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data);
            pItem->AddModifier(data);
        } break;
        case operandRSA: {    //64, %(arg1)s.%(arg2)s      -- used by AGRSM
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), data);
            ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), data);
            pItem->AddModifier(data);
        } break;
        // these will need to work to properly code conditionals here
        case operandOR: {       //'%(arg1)s OR %(arg2)s'    -- usually used with 'if'  (if x ... OR y)  (used as "else" or elif)
            //ParseExpression(sFxDataMgr.GetExpression(expression.arg1), data) << ")\nOR (";
            //ParseExpression(sFxDataMgr.GetExpression(expression.arg2), data) << ")";
        } break;
        case operandAND: {      //'(%(arg1)s) AND (%(arg2)s)'   -- usually used with 'if'  (if x AND y then ....)
            //ParseExpression(sFxDataMgr.GetExpression(expression.arg1), data);
            //ParseExpression(sFxDataMgr.GetExpression(expression.arg2), data);
        } break;
        case operandIF: { //'If(%(arg1)s), Then (%(arg2)s)'     --std conditional.  if x then y
            //ParseExpression(sFxDataMgr.GetExpression(expression.arg1), data);
            //ParseExpression(sFxDataMgr.GetExpression(expression.arg2), data);
        } break;
        /** @todo  will have to figure out how to remove modifiers and delete from the map(s) */
        // why?  just reset attribs using attribMap::Load(true)  (this is 'reset')
        // keep these here to avoid 'default' error below
        //  UPDATE...modules will need effect removal to work properly
        case operandRIM:     //58, RemoveItemModifier(env,%(arg1)s, %(arg2)s)
        case operandRLGM:    //59, (%(arg1)s).RemoveLocationGroupModifier (%(arg2)s)
        case operandRLM:     //60, (%(arg1)s).RemoveLocationModifier (%(arg2)s)
        case operandRLRSM:   //61, (%(arg1)s).RemoveLocationRequiredSkillModifier(%(arg2)s)
        case operandRORSM: { //62, (%(arg1)s).RemoveOwnerRequiredSkillModifier(%(arg2)s)
        } break;
        default: {              // in case the op hasnt been defined, make a note here (should not hit)
            std::ostringstream ret;
            Operand operand = sFxDataMgr.GetOperand(expression.operandID);
            ret << "*** ERROR ***  Operand id:" << expression.operandID << " key:" << operand.operandKey;
            if (operand.format == "")
                ret << " - has not been defined.";
            else                // % {'arg1': arg1, 'arg2': arg2, 'value': expression.expressionValue}
                ret << " - should be added as " << operand.format.c_str();
                sLog.Error("FxProc::ParseExpression", "%s", ret.str().c_str());
        } break;
    }
}

void FxProc::ApplyEffects(InventoryItem* pItem, Character* pChar, ShipItem* pShip)
{
    using namespace Effects;
    for (auto cur : pItem->m_modifiers) {  // k,v of assoc, data<math, src, targLoc, targAttr, srcAttr, grpID, typeID>
        _log(EFFECTS__MESSAGE, "FxProc::ApplyEffects(%i): method: %s, fxSrc: %s(%s), targLoc: %s, targAttr: %u, srcAttr: %u, grpID: %u, typeID: %u", cur.first,\
                sFxProc.GetMathMethodName(cur.second.math).c_str(), sFxProc.GetSourceName(cur.second.fxSrc).c_str(), cur.second.srcRef->itemName().c_str(), \
                sFxProc.GetTargLocName(cur.second.targLoc).c_str(), cur.second.targAttr, cur.second.srcAttr, cur.second.grpID, cur.second.typeID );

        InventoryItemRef srcItemRef = cur.second.srcRef;
        std::vector<InventoryItemRef> itemRefVec;
        // affected target depends on source.  get source and target(s) here.
        switch (cur.second.fxSrc) {
            case dgmSrcGroup: {     // not a source per se, but defines effect's target selection requirements and IS nerfed
                // this is to apply modifiers to ship's modules of groupID defined in 'grpID'
                std::vector<InventoryItemRef> moduleList;
                pShip->GetModuleManager()->GetModuleListOfRefs(&moduleList);
                for (auto mod : moduleList)
                    if (mod->groupID() == cur.second.grpID)
                        itemRefVec.push_back(mod);
            } break;
            case dgmSrcSkill: {    // source of this effect is skill, implant, or booster and IS NOT nerfed
                if (cur.second.typeID == EVEDB::invTypes::typeInvalid) {    //invalid
                    _log(EFFECTS__WARNING, "FxProc::ApplyEffects(): fxSrc is skill.  typeID is invalid");
                    continue;  // make error here
                }
                switch (cur.second.targLoc) {
                    // this is to apply modifier to....
                    case dgmTargLocShip:  {
                        // .... ship's modules that require skill defined in 'typeID'
                        pShip->GetModuleManager()->GetModuleListByReqSkill(cur.second.typeID, &itemRefVec);
                        // ....ship that requires skill defined in 'typeID'
                        if (itemRefVec.empty())
                            if (pShip->HasReqSkill(cur.second.srcRef->typeID()))
                                itemRefVec.push_back(static_cast<InventoryItemRef>(pShip));
                    } break;
                    case dgmTargLocSelf: {
                        // ....item itself
                        itemRefVec.push_back(cur.second.srcRef);
                    } break;
                    case dgmTargLocChar: {
                        // ....char skills that require skill in 'srcRef' or defined in 'typeID'
                        uint16 skillID = cur.second.srcRef->typeID();
                        if (cur.second.typeID)
                            skillID = cur.second.typeID;
                        std::vector<InventoryItemRef> allSkills;
                        pChar->GetSkillsList(allSkills);
                        for (auto curSkill : allSkills)
                            if (curSkill->HasReqSkill(skillID))
                                itemRefVec.push_back(curSkill);
                    } break;
                    case dgmTargLocCharge: {
                        // ....charges
                        // will need more testing to verify this.
                        std::map<EVEItemFlags, InventoryItemRef> charges;
                        pShip->GetModuleManager()->GetLoadedCharges(charges);
                        for (auto mod : charges)
                            if (mod.second->HasReqSkill(cur.second.typeID))
                                itemRefVec.push_back(mod.second);
                    } break;
                    case dgmTargLocTarget: {
                        // ....unknown at this time
                        // will need more testing to verify this.
                        _log(EFFECTS__DEBUG, "FxProc::ApplyEffects(): targLoc is target.  is this right?");
                        itemRefVec.push_back(pShip->GetPilot()->GetShipSE()->TargetMgr()->GetFirstTarget(true)->GetSelf());
                    } break;
                    case dgmTargLocArea: {
                        // ....unknown at this time
                        _log(EFFECTS__DEBUG, "FxProc::ApplyEffects(): called Area() target location.");
                        continue;
                    } break;
                    case dgmTargLocInvalid: {   // null
                        _log(EFFECTS__WARNING, "FxProc::ApplyEffects(): target location invalid.");
                        continue;
                    } break;
                    default: {
                        _log(EFFECTS__ERROR, "FxProc::ApplyEffects(): src is skill.  target is not defined yet.");
                    } break;
                }
            } break;
            case dgmSrcSelf: {  // source is various items (module, charge, skill) and IS nerfed
                switch (cur.second.targLoc) {
                    case dgmTargLocShip:  {
                        // apply effect to....
                        if (cur.second.typeID) {
                            // .....ship's modules that require skillID defined in "typeID"
                            pShip->GetModuleManager()->GetModuleListByReqSkill(cur.second.typeID, &itemRefVec);
                        } else {
                            // ..... ship that require skill in 'srcRef'
                            if (pShip->HasReqSkill(cur.second.srcRef->typeID()))
                                itemRefVec.push_back(static_cast<InventoryItemRef>(pShip));
                        }
                        // if neither above are right, the source could be a module, and target is ship itself
                        if (itemRefVec.empty())
                            itemRefVec.push_back(static_cast<InventoryItemRef>(pShip));
                    } break;
                    case dgmTargLocSelf: {
                        // apply effect to calling item
                        itemRefVec.push_back(cur.second.srcRef);
                    } break;
                    case dgmTargLocCharge: {
                        // apply effect to calling item's charge
                        // will need more testing to verify this.
                        itemRefVec.push_back(pShip->GetModuleManager()->GetLoadedChargeOnModule(cur.second.srcRef));
                    } break;
                    case dgmTargLocChar: {
                        // apply effect to char skills that require skill in 'srcRef' or 'typeID'
                        uint16 skillID = cur.second.srcRef->typeID();
                        if (cur.second.typeID)
                            skillID = cur.second.typeID;
                        std::vector<InventoryItemRef> allSkills;
                        pChar->GetSkillsList(allSkills);
                        for (auto curSkill : allSkills)
                            if (curSkill->HasReqSkill(skillID))
                                itemRefVec.push_back(curSkill);
                    } break;
                    default: {
                        _log(EFFECTS__DEBUG, "FxProc::ApplyEffects(): src is self.  target is default (ship modules by typeID?).");
                        // apply effect to ship's modules of type defined in 'typeID'
                        std::vector<InventoryItemRef> moduleList;
                        pShip->GetModuleManager()->GetModuleListOfRefs(&moduleList);
                        for (auto mod : moduleList)
                            if (mod->typeID() == cur.second.typeID)
                                itemRefVec.push_back(mod);
                    } break;
                }
            } break;
            case dgmSrcShip: {      // source is a subsystem and IS NOT nerfed
                ;   // not sure how to do this on yet.  t3 ships arent implemented (actually blocked)
            } break;
            case dgmSrcGang: {      // source is a gang leader skill and IS nerfed
                ;   //dgmTargLocSelf is ship of gang member to apply leader's skill bonuses to
            } break;
            case dgmSrcInvalid: {
                _log(EFFECTS__ERROR, "FxProc::ApplyEffects(): source location invalid.");
                continue;
            } break;
            // these are not used (not coded)
            case dgmSrcTarget:
            case dgmSrcOwner: {
                _log(EFFECTS__DEBUG, "FxProc::ApplyEffects(): called owner, target or gang as source.");
                continue;
            } break;
        }
        // get srcAttr
        EvilNumber srcValue = srcItemRef->GetAttribute(cur.second.srcAttr);
        /*
        dgmUnnerfedCategories = [
        categorySkill,
        categoryImplant,
        categoryShip,
        categoryCharge,
        categorySubSystem]
        */
        // check for nerf, modify value as needed
        bool nerfed = false;
        switch (cur.second.fxSrc) {
            case dgmSrcSelf:
            case dgmSrcGang:
            case dgmSrcTarget:
            case dgmSrcOwner: {
                switch (cur.second.targLoc) {
                    case dgmTargLocShip:
                    case dgmTargLocTarget: {
                        switch (cur.second.math) {
                            case dgmMathPreDiv:
                            case dgmMathPreMul:
                            case dgmMathPostMul:
                            case dgmMathPostDiv:
                            case dgmMathPostPercent: {
                                nerfed = true; // not sure how to do this yet....probably map these on ship for easier access/checking/etc
                            } break;
                        }
                    } break;
                }
            } break;
        }
        // set target attr to modified value
        EvilNumber targValue = 0;
        if (itemRefVec.empty()) {
            _log(EFFECTS__TRACE, "FxProc::ApplyEffects(): target item vector empty.");
        } else {
            for (auto item : itemRefVec) {
                // get targAttr
                targValue = item->GetAttribute(cur.second.targAttr);
                // send data to calculator
                EvilNumber newValue = sFxProc.CalculateAttributeValue(targValue, srcValue, cur.first);
                // set new calculated value for target attribute
                _log(EFFECTS__MESSAGE, "FxProc::ApplyEffects(): setting attribute %u for %s from %.3f to %.3f.  Nerfed: %s", \
                        cur.second.targAttr, item->itemName().c_str(), targValue.get_float(), newValue.get_float(), (nerfed ? "true" : "false"));
                item->SetAttribute(cur.second.targAttr, newValue, false);
            }
        }
    }
}
