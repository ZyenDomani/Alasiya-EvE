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

FxProc::FxProc()
{
}

FxProc::~FxProc()
{
}

EvilNumber FxProc::CalculateAttributeValue(EvilNumber val1, EvilNumber val2, Effects::Association type)
{
    using namespace Effects;
    switch (type) {
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
    _log(SHIP__MODULE_ERROR, "CalculateNewAttributeValue() - Unknown Association used: %i", (int)type);
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
    res += ParseExpression(nullptr, sFxDataMgr.GetExpression(expID), false, true);
    sLog.Green("EvaluateExpression", "expID %u: %s", expID, res.c_str());
}

std::string FxProc::ParseExpression(InventoryItem* pItem, Expression expression, bool restricted/*false*/, bool topLevel/*false*/)
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
                 a1 = ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1));
                 ret << ", " << a1;
                 if (expression.arg2) {
                     a2 = ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2));
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
        case operandDEFBOOL:
        case operandDEFINT: {
            //if (topLevel)
                ret << expression.expressionValue;
        } break;
        case operandDEFFLOAT: {    //
            if (expression.expressionValue != "")
                ret << expression.expressionValue;
            else
                ret << "(defFloat.value==NULL)";
        } break;
        case operandDEFSTRING: { //
            if (expression.expressionValue != "")
                ret << expression.expressionValue;
            else
                ret << "(defString.value==NULL)";
        } break;
        // these provide the given expression*ID
        case operandDEFATTRIBUTE: {    //
            if (expression.expressionAttributeID) {
                ret << expression.expressionAttributeID;
            }
        } break;
        case operandDEFGROUP: {    //
            if (expression.expressionGroupID){
                ret << expression.expressionGroupID;
            } else if (expression.expressionValue != "") {
                ret << "(*GroupName* " << expression.expressionValue << ")";
                /*
                 *        groupName = expression.expressionValue.replace(' ', '')
                 *        groupName = 'group' + groupName[0].upper() + groupName[1:]
                 *        if hasattr(const, groupName)
                 *            return str(getattr(const, groupName));
                 */}
        } break;
        case operandDEFTYPEID: {    //
            if (expression.expressionTypeID) {
                ret << expression.expressionTypeID;
            } else if (expression.expressionValue != "") {
                ret << "(*TypeName* " << expression.expressionValue << ")";
                /*
                 *        typeName = expression.expressionValue.replace(' ', '')
                 *        typeName = 'type' + typeName[0].upper() + typeName[1:]
                 *        if hasattr(const, typeName)
                 *            return str(getattr(const, typeName));
                 */}
        } break;
        // do as stated
        case operandCOMBINE: { // executes two statements  '%(arg1)s); (%(arg2)s'
            if (expression.arg1)
                ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), restricted) << "\n";
            if (expression.arg2)
                ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), restricted);
        } break;
        case operandEFF: {      //define association type  '(%(arg2)s).(%(arg1)s)'
            std::string arg1= "nil", arg2 = "nil";
            if (expression.arg1)
                arg1 = ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), restricted);
            if (expression.arg2)
                arg2 = ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), restricted);
            ret  << arg1 << ", " << arg2;
        } break;

        // these are trivial attribute operations
        case operandATT: {      //'%(arg1)s->%(arg2)s'      (domain:attribID)
            std::string arg1= "", arg2 = "";
            if (expression.arg1)
                arg1 = ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), restricted);
            if (expression.arg2)
                arg2 = ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), restricted);
            ret << "(" << arg1 << ":" << arg2 << ")";
        } break;
        case operandADD: {      //'(%(arg1)s)+(%(arg2)s)'
            std::string arg1= "", arg2 = "";
            if (expression.arg1)
                arg1 = ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), restricted);
            if (expression.arg2)
                arg2 = ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), restricted);
            ret << "(" << arg1 << "+" << arg2 << ")";
        } break;
        case operandSUB: {      //'(%(arg1)s)-(%(arg2)s)'
            std::string arg1= "", arg2 = "";
            if (expression.arg1)
                arg1 = ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), restricted);
            if (expression.arg2)
                arg2 = ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), restricted);
            ret << "(" << arg1 << "-" << arg2 << ")";
        } break;
        case operandMUL: {    //'(%(arg1)s)*(%(arg2)s)'
            std::string arg1= "", arg2 = "";
            if (expression.arg1)
                arg1 = ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), restricted);
            if (expression.arg2)
                arg2 = ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), restricted);
            ret << "(" << arg1 << "*" << arg2 << ")";
        } break;
        case operandEQ: {    //'%(arg1)s == %(arg2)s'
            ret << "(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << "==";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandGT: {    //'%(arg1)s> %(arg2)s'
            ret << "(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ">";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandGTE: {   //'%(arg1)s>=%(arg2)s'
            ret << "(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ">=";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandINC: {      //'%(arg1)s+=self.%(arg2)s'
            ret << "(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), restricted) << " += (Self:";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), restricted) << "))";
        } break;
        case operandINCN: {     //'%(arg1)s+=%(arg2)s'
            ret << "(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), restricted) << " += ";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), restricted) << ")";
        } break;
        case operandDEC: {      //'%(arg1)s-=self.%(arg2)s'
            ret << "(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), restricted) << " -= (Self:";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), restricted) << "))";
        } break;
        case operandDECN: {     //'%(arg1)s-=%(arg2)s'
            ret << "(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), restricted) << " -= ";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), restricted) << ")";
        } break;

        // effect function calls.   handled in ShipItem class in Ship.cpp
        case operandSKILLCHECK: { //'dogma.SkillCheck(env, %(arg1)s, %(arg2)s)'
            ret << "SkillCheck(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), restricted);
            if (expression.arg2) {
                ret << ", " << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), restricted);
            }
            ret << ")";
        } break;
        case operandAIM: {    //
            //'dogma.AddItemModifier(env,%(arg1)s, %(arg2)s)'
            Expression arg1Expression = sFxDataMgr.GetExpression(expression.arg1);
            ret << "AddModifier(" << ParseExpression(pItem, sFxDataMgr.GetExpression(arg1Expression.arg1));
            ret << ", " << ParseExpression(pItem, sFxDataMgr.GetExpression(arg1Expression.arg2)) << ", (Self:";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << "))";
        } break;
        case operandRIM: {    //
            //'dogma.RemoveItemModifier(env,%(arg1)s, %(arg2)s)'
            Expression arg1Expression = sFxDataMgr.GetExpression(expression.arg1);
            ret << "RemoveModifier(" << ParseExpression(pItem, sFxDataMgr.GetExpression(arg1Expression.arg1));
            ret << ", " << ParseExpression(pItem, sFxDataMgr.GetExpression(arg1Expression.arg2)) << ", (Self:";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << "))";
        } break;
        case operandAGGM: {    //2,[%(arg1)s].AGGM(%(arg2)s)
            ret << "AGGM(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ", ";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandAGIM: {    //3,[%(arg1)s].AGIM(%(arg2)s)
            ret << "AGIM(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ", ";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandAGORSM: {    //4, [%(arg1)s].AGORSM(%(arg2)s)
            ret << "AGORSM(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ", ";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandAGRSM: {    //5,  [%(arg1)s].AGRSM(%(arg2)s)
            ret << "AGRSM(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ", ";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandALGM: {    //7,(%(arg1)s).AddLocationGroupModifier (%(arg2)s)
            ret << "AddLocationGroupModifier(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ", ";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandALM: {    //8,(%(arg1)s).AddLocationModifier (%(arg2)s)
            ret << "AddLocationModifier(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ", ";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandALRSM: {    //9,(%(arg1)s).ALRSM(%(arg2)s)
            ret << "ALRSM(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ", ";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandAORSM: {    //11,(%(arg1)s).AORSM(%(arg2)s)
            ret << "AORSM(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ", ";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRGGM: {    //54, [%(arg1)s].RGGM(%(arg2)s)
            ret << "RGGM(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ", ";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRGIM: {    //55,[%(arg1)s].RGIM(%(arg2)s)
            ret << "RGIM(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ", ";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRGORSM: {    //56,[%(arg1)s].RGORSM(%(arg2)s)
            ret << "RGORSM(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ", ";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRGRSM: {    //57,[%(arg1)s].RGRSM(%(arg2)s)
            ret << "RGRSM(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ", ";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRLGM: {    //59,(%(arg1)s).RemoveLocationGroupModifier (%(arg2)s)
            ret << "RemoveLocationGroupModifier(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ", ";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRLM: {    //60, (%(arg1)s).RemoveLocationModifier (%(arg2)s)
            ret << "RemoveLocationModifier(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ", ";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRLRSM: {    //61,(%(arg1)s).RLRSM(%(arg2)s)
            ret << "RLRSM(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ", ";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRORSM: {    //62, (%(arg1)s).RORSM(%(arg2)s)
            ret << "RORSM(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ", ";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRS: {    //63, %(arg1)s.Requires(%(arg2)s)
            ret << "(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ".Requires(";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandRSA: {    //64, %(arg1)s.%(arg2)s
            ret << "RSA(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ", ";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;

        // these function calls are a bit more complicated.  handled in ShipItem class in Ship.cpp
        case operandGA: {    //'%(arg1)s.%(arg2)s'
            ret << "ModuleGroupAttrib(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ".";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandGM: {    //'%(arg1)s.%(arg2)s'
            ret << "GetModule(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ".";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandGET: {    //'%(arg1)s.%(arg2)s()'
            ret << "GetAttribute(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ".";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << ")";
        } break;
        case operandGETTYPE: {    //'%(arg1)s.GetTypeID()'
            ret << "GetTypeID(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ")";
        } break;
        case operandIA: {    //'%(arg1)s'
            std::string arg1 = ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1));
            ret << "attribute(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << ")";
        } break;
        case operandLG: {    //'%(arg1)s..%(arg2)s'  -- specify a group in a location'
            ret << "LG(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << "[";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << "])";
        } break;
        case operandLS: {    //'%(arg1)s[%(arg2)s]'  -- skill required item group in location
            ret << "SRLG(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1)) << "[";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2)) << "])";
        } break;
        case operandSET: {      //'%(arg1)s := %(arg2)s'
            ret << "SetAttribute(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), restricted) << " = ";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), restricted) << ")";
        } break;
        case operandOR: {       //'%(arg1)s OR %(arg2)s'
            ret << "(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), restricted) << ")\nOR (";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), restricted) << ")";
        } break;
        case operandAND: {      //'(%(arg1)s) AND (%(arg2)s)'
            ret << "(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), restricted) << ")\nAND (";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), restricted) << ")";
        } break;
        case operandIF: { //'If(%(arg1)s), Then (%(arg2)s)'
            ret << "IF(" << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), restricted) << ")\nTHEN (";
            ret << ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), restricted) << ")";
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
        if (expression.expressionAttributeID) {
            ret << expression.expressionAttributeID;
        } else if (expression.expressionTypeID) {
            ret << expression.expressionTypeID;
        } else if (expression.expressionGroupID) {
            ret << expression.expressionGroupID;
        } else if (expression.operandID == operandDEFGROUP) {
            ret << "*groupNameByID*" << expression.expressionValue; // get group name by id here
        } else if (expression.operandID == operandDEFTYPEID) {
            ret << "*typeNameByID*" << expression.expressionValue; // get type name by id here
        } else {
            std::string arg1= "", arg2 = "";
            if (expression.arg1)
                arg1 = ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg1), restricted);
            if (expression.arg2)
                arg2 = ParseExpression(pItem, sFxDataMgr.GetExpression(expression.arg2), restricted);
            if ((arg1 != "") and (arg1.find_first_of('\n') > 1))
                ret << "*arg1Newline*" ;   //arg1 = arg1.strip();
            if ((arg2 != "") and (arg2.find_first_of('\n') > 1))
                ret << "*arg2Newline*" ;   //arg2 = arg2.strip();
        }
    }

    return ret.str().c_str();
}
