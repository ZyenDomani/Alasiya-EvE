/**
 * @name EffectsProcessor.cpp
 *   This file is for decoding and processing the effect data
 *   Copyright 2017  Alasiya-EVEmu Team
 *
 * @Author:    Allan
 * @date:      24 January 2017
 *
 */

#include "effects/EffectsProcessor.h"
#include "inventory/InventoryItem.h"


EvilNumber FxProc::CalculateAttributeValue(EvilNumber val1, EvilNumber val2, int8 assoc)
{
    using namespace Effects;
    switch (assoc) {
        case dgmAssInvalid:
            _log(SHIP__MODULE_ERROR, "CalculateNewAttributeValue() - Invalid Association used");
        case dgmAssSkillCheck:
        case dgmAssPreAssignment:
        case dgmAssPostAssignment:
            return val1;
        case dgmAssPreMul:
        case dgmAssPostMul:
            return val1 * val2;
        case dgmAssPreDiv:
        case dgmAssPostDiv:
            return ((val2 != 0) ? val1 / val2 : val1);
        case dgmAssModAdd:
            return val1 + val2;
        case dgmAssModSub:
            return val1 - val2;
        case dgmAssPostPercent:
            return val1 * ((100 + val2) / 100);
    }
    _log(SHIP__MODULE_ERROR, "CalculateNewAttributeValue() - Unknown Association used: %i", (int8)assoc);
    return 0;
}

int8 FxProc::GetAssociationEnum(const std::string& association)
{   // opID 21
    using namespace Effects;
    if (association == "PreAssignment")
        return dgmAssPreAssignment;
    else if (association == "PreDiv")
        return dgmAssPreDiv;
    else if (association == "PreMul")
        return dgmAssPreMul;
    else if (association == "ModAdd")
        return dgmAssModAdd;
    else if (association == "ModSub")
        return dgmAssModSub;
    else if (association == "PostPercent")
        return dgmAssPostPercent;
    else if (association == "PostMul")
        return dgmAssPostMul;
    else if (association == "PostDiv")
        return dgmAssPostDiv;
    else if (association == "PostAssignment")
        return dgmAssPostAssignment;
    else if (association == "SkillCheck")
        return dgmAssSkillCheck;
    else if (association == "AddRate")
        return dgmAssAddRate;
    else if (association == "SubRate")
        return dgmAssSubRate;
    else
        return dgmAssInvalid;  //throw std::bad_typeid();
}

int8 FxProc::GetEnvironmentEnum(const std::string& domain)
{   // opID 24
    using namespace Effects;
    if (domain == "Self")
        return dgmEnvSelf;
    else if (domain == "Char")
        return dgmEnvChar;
    else if (domain == "Ship")
        return dgmEnvShip;
    else if (domain == "Target")
        return dgmEnvTarget;
    else if (domain == "Area")
        return dgmEnvArea;
    else if (domain == "Other")
        return dgmEnvOther;
    else
        return dgmEnvInvalid;  //throw std::bad_typeid();
}

void FxProc::EvaluateExpression(const uint16 expID)
{
    std::string res = "\n";
    res += ParseExpression(sFxDataMgr.GetExpression(expID), false, true);
    sLog.Green("EvaluateExpression", "expID %u: %s", expID, res.c_str());
}

std::string FxProc::ParseExpression(Expression expression, bool restricted/*false*/, bool topLevel/*false*/)
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
                 a1 = ParseExpression(sFxDataMgr.GetExpression(expression.arg1));
                 ret << ", " << a1;
                 if (expression.arg2) {
                     a2 = ParseExpression(sFxDataMgr.GetExpression(expression.arg2));
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
                ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg1), restricted) << "\n";
            if (expression.arg2)
                ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2), restricted);
        } break;
        case operandEFF: {      //define association type  '(%(arg2)s).(%(arg1)s)'
            std::string arg1 = "nil", arg2 = "nil";
            if (expression.arg1)
                arg1 = ParseExpression(sFxDataMgr.GetExpression(expression.arg1), restricted);
            if (expression.arg2)
                arg2 = ParseExpression(sFxDataMgr.GetExpression(expression.arg2), restricted);
            ret  << arg1 << ", " << arg2;
        } break;

        // these are trivial attribute operations
        case operandATT: {      //'%(arg1)s->%(arg2)s'      (domain:attribID)
            std::string arg1 = "nil", arg2 = "nil";
            if (expression.arg1)
                arg1 = ParseExpression(sFxDataMgr.GetExpression(expression.arg1), restricted);
            if (expression.arg2)
                arg2 = ParseExpression(sFxDataMgr.GetExpression(expression.arg2), restricted);
            ret << "(" << arg1 << ":" << arg2 << ")";
        } break;
        case operandADD: {      //'(%(arg1)s)+(%(arg2)s)'
            std::string arg1 = "nil", arg2 = "nil";
            if (expression.arg1)
                arg1 = ParseExpression(sFxDataMgr.GetExpression(expression.arg1), restricted);
            if (expression.arg2)
                arg2 = ParseExpression(sFxDataMgr.GetExpression(expression.arg2), restricted);
            ret << "(" << arg1 << "+" << arg2 << ")";
        } break;
        case operandSUB: {      //'(%(arg1)s)-(%(arg2)s)'
            std::string arg1 = "nil", arg2 = "nil";
            if (expression.arg1)
                arg1 = ParseExpression(sFxDataMgr.GetExpression(expression.arg1), restricted);
            if (expression.arg2)
                arg2 = ParseExpression(sFxDataMgr.GetExpression(expression.arg2), restricted);
            ret << "(" << arg1 << "-" << arg2 << ")";
        } break;
        case operandMUL: {    //'(%(arg1)s)*(%(arg2)s)'
            std::string arg1 = "nil", arg2 = "nil";
            if (expression.arg1)
                arg1 = ParseExpression(sFxDataMgr.GetExpression(expression.arg1), restricted);
            if (expression.arg2)
                arg2 = ParseExpression(sFxDataMgr.GetExpression(expression.arg2), restricted);
            ret << "(" << arg1 << "*" << arg2 << ")";
        } break;
        case operandEQ: {    //'%(arg1)s == %(arg2)s'
            ret << "(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << "==";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandGT: {    //'%(arg1)s> %(arg2)s'
            ret << "(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ">";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandGTE: {   //'%(arg1)s>=%(arg2)s'
            ret << "(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ">=";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandINC: {      //'%(arg1)s+=self.%(arg2)s'
            ret << "(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1), restricted) << " += (Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2), restricted) << "))";
        } break;
        case operandINCN: {     //'%(arg1)s+=%(arg2)s'
            ret << "(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1), restricted) << " += (Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2), restricted) << ")";
        } break;
        case operandDEC: {      //'%(arg1)s-=self.%(arg2)s'
            ret << "(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1), restricted) << " -= (Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2), restricted) << "))";
        } break;
        case operandDECN: {     //'%(arg1)s-=%(arg2)s'
            ret << "(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1), restricted) << " -= (Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2), restricted) << ")";
        } break;

        // effect function calls.   handled in ShipItem class in Ship.cpp
        case operandSKILLCHECK: { //'dogma.SkillCheck(env, %(arg1)s, %(arg2)s)'
            ret << "SkillCheck(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1), restricted);
            if (expression.arg2) {
                ret << ", " << ParseExpression(sFxDataMgr.GetExpression(expression.arg2), restricted);
            }
            ret << ")";
        } break;
        case operandAIM: {    //
            //'dogma.AddItemModifier(env,%(arg1)s, %(arg2)s)'
            Expression arg1Expression = sFxDataMgr.GetExpression(expression.arg1);
            ret << "AddItemModifier(" << ParseExpression(sFxDataMgr.GetExpression(arg1Expression.arg1));
            ret << ", " << ParseExpression(sFxDataMgr.GetExpression(arg1Expression.arg2)) << ", (Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << "))";
        } break;
        case operandRIM: {    //
            //'dogma.RemoveItemModifier(env,%(arg1)s, %(arg2)s)'
            Expression arg1Expression = sFxDataMgr.GetExpression(expression.arg1);
            ret << "RemoveItemModifier(" << ParseExpression(sFxDataMgr.GetExpression(arg1Expression.arg1));
            ret << ", " << ParseExpression(sFxDataMgr.GetExpression(arg1Expression.arg2)) << ", (Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << "))";
        } break;
        case operandAGGM: {    //2,[%(arg1)s].AGGM(%(arg2)s)    --not used
            ret << "AddGangGroupModifier(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandAGIM: {    //3,[%(arg1)s].AGIM(%(arg2)s)
            ret << "AddGangShipModifier(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandAGORSM: {    //4, [%(arg1)s].AGORSM(%(arg2)s)   --not used
            ret << "AddGangOwnerRequiredSkillModifier(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandAGRSM: {    //5,  [%(arg1)s].AGRSM(%(arg2)s)
            ret << "AddGangRequiredSkillModifier(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandALGM: {    //7,(%(arg1)s).AddLocationGroupModifier (%(arg2)s)
            ret << "AddLocationGroupModifier(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandALM: {    //8,(%(arg1)s).AddLocationModifier (%(arg2)s)
            ret << "AddLocationModifier(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandALRSM: {    //9,(%(arg1)s).ALRSM(%(arg2)s)
            ret << "AddLocationRequiredSkillModifier(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandAORSM: {    //11,(%(arg1)s).AORSM(%(arg2)s)
            ret << "AddOwnerRequiredSkillModifier(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRGGM: {    //54, [%(arg1)s].RGGM(%(arg2)s)
            ret << "RemoveGangGroupModifier(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRGIM: {    //55,[%(arg1)s].RGIM(%(arg2)s)
            ret << "RemoveGangShipModifier(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRGORSM: {    //56,[%(arg1)s].RGORSM(%(arg2)s)
            ret << "RemoveGangOwnerRequiredSkillModifier(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRGRSM: {    //57,[%(arg1)s].RGRSM(%(arg2)s)
            ret << "RemoveGangRequiredSModifier(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRLGM: {    //59,(%(arg1)s).RemoveLocationGroupModifier (%(arg2)s)
            ret << "RemoveLocationGroupModifier(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRLM: {    //60, (%(arg1)s).RemoveLocationModifier (%(arg2)s)
            ret << "RemoveLocationModifier(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRLRSM: {    //61,(%(arg1)s).RLRSM(%(arg2)s)
            ret << "RemoveLocationRequiredSkillModifier(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRORSM: {    //62, (%(arg1)s).RORSM(%(arg2)s)
            ret << "RemoveOwnerRequiredSkillModifier(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", (Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRS: {    //63, %(arg1)s.Requires(%(arg2)s)  --not used
            ret << "(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ".Requires(";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRSA: {    //64, %(arg1)s.%(arg2)s      -- used by AGRSM/RGRSM  ** NEEDS WORK **
            ret << "RequiredSkillAttribute(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ", Self:";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;


        // these function calls are a bit more complicated...will need more work and better understanding
        //      handled in ShipItem class in Ship.cpp
        case operandGA: {    //'%(arg1)s.%(arg2)s'      --not used
            ret << "ModuleGroupAttrib(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ".";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandGM: {    //'%(arg1)s.%(arg2)s'      --used by subsystems as GetModule(Ship:201)
            ret << "GetModule(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ":";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandGET: {    //'%(arg1)s.%(arg2)s()'   --used a lot.  eg. GetAttribute(Ship:101)
            ret << "GetAttribute(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ":";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandGETTYPE: {    //'%(arg1)s.GetTypeID()'  --used by SRLG in AORSM/RORSM
            ret << "GetTypeID(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << "[";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << "])";
        } break;
        case operandIA: {    //'%(arg1)s'   -used by AGIM
            std::string arg1 = ParseExpression(sFxDataMgr.GetExpression(expression.arg1));
            ret << "attribute(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << ")";
        } break;
        case operandLG: {    //'%(arg1)s..%(arg2)s'  -- specify a group in a location'
            ret << "LocationGroup(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << "[";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << "])";
        } break;
        case operandLS: {    //'%(arg1)s[%(arg2)s]'  -- skill required item group in location
            ret << "SkillRequiredLocationGroup(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1)) << "[";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2)) << "])";
        } break;
        case operandSET: {      //'%(arg1)s := %(arg2)s'        --used by online/offline for all moodules
            ret << "SetAttribute(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1), restricted) << " = ";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2), restricted) << ")";
        } break;
        // these will need to work to properly code conditionals here
        case operandOR: {       //'%(arg1)s OR %(arg2)s'    -- this is the 'else' of the 'if'
            ret << "(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1), restricted) << ")\nOR (";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2), restricted) << ")";
        } break;
        case operandAND: {      //'(%(arg1)s) AND (%(arg2)s)'   -- means to run both arg1 and arg2
            ret << "(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1), restricted) << ")\nAND (";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2), restricted) << ")";
        } break;
        case operandIF: { //'If(%(arg1)s), Then (%(arg2)s)'     --std conditional.  if (arg1 == true) then (arg2)
            ret << "IF(" << ParseExpression(sFxDataMgr.GetExpression(expression.arg1), restricted) << ")\nTHEN (";
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2), restricted) << ")";
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
                arg1 = ParseExpression(sFxDataMgr.GetExpression(expression.arg1), restricted);
            if (expression.arg2)
                arg2 = ParseExpression(sFxDataMgr.GetExpression(expression.arg2), restricted);
            if ((arg1 != "") and (arg1.find_first_of('\n') > 1))
                ret << "*arg1Newline*" ;   //arg1 = arg1.strip();
            if ((arg2 != "") and (arg2.find_first_of('\n') > 1))
                ret << "*arg2Newline*" ;   //arg2 = arg2.strip();
        }
    }

    return ret.str().c_str();
}
