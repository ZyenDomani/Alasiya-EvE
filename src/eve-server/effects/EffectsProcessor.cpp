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

FxProc::FxProc()
{
}

FxProc::~FxProc()
{
}

EvilNumber FxProc::CalculateAttributeValue(EvilNumber val1, EvilNumber val2, EVECalculationType type)
{
    switch (type) {
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

int8 FxProc::GetAssociationEnum(const std::string& association)
{   // opID 21
    using namespace Effects;
    if (association == "PreAssignment")
        return ASSOC_PRE_ASSIGNMENT;
    else if (association == "PreDiv")
        return ASSOC_PRE_DIV;
    else if (association == "PreMul")
        return ASSOC_PRE_MUL;
    else if (association == "ModAdd")
        return ASSOC_MOD_ADD;
    else if (association == "ModSub")
        return ASSOC_MOD_SUB;
    else if (association == "PostPercent")
        return ASSOC_POST_PERCENT;
    else if (association == "PostMul")
        return ASSOC_POST_MUL;
    else if (association == "PostDiv")
        return ASSOC_POST_DIV;
    else if (association == "PostAssignment")
        return ASSOC_POST_ASSIGNMENT;
    else if (association == "9")    // wtf is this?
        return ASSOC_SKILL_TIME;
    else if (association == "AddRate")
        return ASSOC_ADD_RATE;
    else if (association == "SubRate")
        return ASSOC_SUB_RATE;
    else
        return ASSOC_INVALID;  //throw std::bad_typeid();
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
    else if (domain == "Other")
        return dgmEnvOther;
    else if (domain == "Area")
        return dgmEnvArea;
    else
        return dgmEnvInvalid;  //throw std::bad_typeid();
}

void FxProc::EvaluateExpression(const uint16 expID, EffectData& fxData)
{
    using namespace Effects;

    fxData.sourceName = "";
    fxData.targetName = "";
    fxData.arg1Description = "";
    fxData.arg2Description = "";

    fxData.srcEnv = 0;
    fxData.targEnv = 0;
    fxData.srcAttrib = 0;
    fxData.targAttrib = 0;
    fxData.association = 0;

    fxData.typeID = 0;
    fxData.groupID = 0;
    fxData.attribID = 0;
    fxData.value = "";

    Expression Exp = sFxDataMgr.GetExpression(expID);
    fxData.id = expID;
    fxData.effectName = Exp.description;
    fxData.preExpression = Exp.arg1;
    fxData.postExpression = Exp.arg2;

    std::string res = "\n";
    res += ParseExpression(Exp, false, true);
    sLog.Green("EvaluateExpression", "returned %s for expID %u", res.c_str(), expID);
}

std::string FxProc::ParseExpression(Expression expression, bool restricted/*false*/, bool topLevel/*false*/)
{
    using namespace Effects;

    if (((expression.operandID == operandDEFBOOL) or (expression.operandID == operandDEFINT)) and (topLevel))
        return expression.expressionValue;

    std::ostringstream ret;
    if (expression.operandID == operandUE) {
        std::string a1 = "", a2 = "";
        if (expression.arg1)
            a1 = ParseExpression(sFxDataMgr.GetExpression(expression.arg1));
        if (expression.arg2)
            a2 = ParseExpression(sFxDataMgr.GetExpression(expression.arg2));
        ret << "UserError(env, " << a1 << ", " << a2 << ")";
        return ret.str().c_str();
    }

    if (expression.operandID == operandCOMBINE) {
        // executes two statements  '%(arg1)s); (%(arg2)s'
        if (expression.arg1)
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg1), restricted) << "\n  ";
        if (expression.arg2)
            ret << ParseExpression(sFxDataMgr.GetExpression(expression.arg2), restricted) << "\n  ";
        return ret.str().c_str();
    }
    if (expression.operandID == operandEFF) {
        //define association type  '(%(arg2)s).(%(arg1)s)'
        std::string arg1= "", arg2 = "";
        if (expression.arg1)
            arg1 = ParseExpression(sFxDataMgr.GetExpression(expression.arg1), restricted);
        if (expression.arg2)
            arg2 = ParseExpression(sFxDataMgr.GetExpression(expression.arg2), restricted);
        ret << "(" << arg1 << "." << arg2 << ")";
        return ret.str().c_str();
    }
    if (expression.operandID == operandATT) {
        //'%(arg1)s->%(arg2)s'      (domain:attribID)
        std::string arg1= "", arg2 = "";
        if (expression.arg1)
            arg1 = ParseExpression(sFxDataMgr.GetExpression(expression.arg1), restricted);
        if (expression.arg2)
            arg2 = ParseExpression(sFxDataMgr.GetExpression(expression.arg2), restricted);
        ret << "(" << arg1 << "->" << arg2 << ")";
        return ret.str().c_str();
    }
    if (expression.operandID == operandADD) {
        //'(%(arg1)s)+(%(arg2)s)'
        std::string arg1= "", arg2 = "";
        if (expression.arg1)
            arg1 = ParseExpression(sFxDataMgr.GetExpression(expression.arg1), restricted);
        if (expression.arg2)
            arg2 = ParseExpression(sFxDataMgr.GetExpression(expression.arg2), restricted);
        ret << "(" << arg1 << "+" << arg2 << ")";
        return ret.str().c_str();
    }
    if (expression.operandID == operandSUB) {
        //'(%(arg1)s)-(%(arg2)s)'
        std::string arg1= "", arg2 = "";
        if (expression.arg1)
            arg1 = ParseExpression(sFxDataMgr.GetExpression(expression.arg1), restricted);
        if (expression.arg2)
            arg2 = ParseExpression(sFxDataMgr.GetExpression(expression.arg2), restricted);
        ret << "(" << arg1 << "-" << arg2 << ")";
        return ret.str().c_str();
    }



    if (expression.operandID == operandDEFASSOCIATION) {
        return expression.expressionValue;
        /*
        if (expression.expressionValue == "PreMul")
            return "value";
        if (expression.expressionValue == "PreDiv")
            return "value";
        if (expression.expressionValue == "ModAdd")
            return "value";
        if (expression.expressionValue == "ModSub")
            return "9";
        if (expression.expressionValue == "PostMul")
            return "value";
        if (expression.expressionValue == "PostDiv")
            return "value";
        if (expression.expressionValue == "9")
            return "9";
        if (expression.expressionValue == "PreAssignment")
            return "value";
        if (expression.expressionValue == "PostAssignment")
            return "value";
        */
    } else if (expression.operandID == operandDEFENVIDX) {
        return expression.expressionValue;
        /*
        if (expression.expressionValue == "Self")
            return "itemID";
        if (expression.expressionValue == "Ship")
            return "shipID";
        if (expression.expressionValue == "Target")
            return "targetID";
        if (expression.expressionValue == "Char")
            return "charID";
        if (expression.expressionValue == "Other")
            return "otherID";
        */
    } else {
        if (expression.operandID == operandAIM) {
            //'dogma.AddItemModifier(env,%(arg1)s, %(arg2)s)'
            Expression arg1Expression = sFxDataMgr.GetExpression(expression.arg1);
            std::string affectingAttribute = ParseExpression(sFxDataMgr.GetExpression(expression.arg2));
            Expression affectedStuffExpression = sFxDataMgr.GetExpression(arg1Expression.arg2);
            std::string affectedItem = ParseExpression(sFxDataMgr.GetExpression(affectedStuffExpression.arg1));
            std::string affectedAttribute = ParseExpression(sFxDataMgr.GetExpression(affectedStuffExpression.arg2));
            std::string affectedType = ParseExpression(sFxDataMgr.GetExpression(arg1Expression.arg1));
            ret << "AddModifier(targType:" << affectedType << ", targ:" << affectedItem << ", targAttrib:" << affectedAttribute<< ", src:";
            ret << "Self, srcAttrib:" << affectingAttribute << ")";
            return ret.str().c_str();
        }
        if (expression.operandID == operandRIM) {
            //'dogma.RemoveItemModifier(env,%(arg1)s, %(arg2)s)'
            Expression arg1Expression = sFxDataMgr.GetExpression(expression.arg1);
            std::string affectingAttribute = ParseExpression(sFxDataMgr.GetExpression(expression.arg2));
            Expression affectedStuffExpression = sFxDataMgr.GetExpression(arg1Expression.arg2);
            std::string affectedItem = ParseExpression(sFxDataMgr.GetExpression(affectedStuffExpression.arg1));
            std::string affectedAttribute = ParseExpression(sFxDataMgr.GetExpression(affectedStuffExpression.arg2));
            std::string affectedType = ParseExpression(sFxDataMgr.GetExpression(arg1Expression.arg1));
            ret << "RemoveModifier(targType:" << affectedType << ", targ:" << affectedItem << ", targAttrib:" << affectedAttribute<< ", src:";
            ret << "Self, srcAttrib:" << affectingAttribute << ")";
            return ret.str().c_str();
        }
        if (expression.operandID == operandDEFATTRIBUTE) {
            if (expression.expressionAttributeID) {
                ret << expression.expressionAttributeID;
                return ret.str().c_str();
            }
        } else if (expression.operandID == operandDEFGROUP) {
            if (expression.expressionGroupID){
                ret << expression.expressionGroupID;
                return ret.str().c_str();
            }
            if (expression.expressionValue != "") {
                ret << "*GroupName* " << expression.expressionValue;
                return ret.str().c_str();
                /*
                groupName = expression.expressionValue.replace(' ', '')
                groupName = 'group' + groupName[0].upper() + groupName[1:]
                if hasattr(const, groupName)
                    return str(getattr(const, groupName));
            */}
        } else if (expression.operandID == operandDEFTYPEID) {
            if (expression.expressionTypeID) {
                ret << expression.expressionTypeID;
                return ret.str().c_str();
            }
            if (expression.expressionValue != "") {
                ret << "*TypeName* " << expression.expressionValue;
                return ret.str().c_str();
                /*
                typeName = expression.expressionValue.replace(' ', '')
                typeName = 'type' + typeName[0].upper() + typeName[1:]
                if hasattr(const, typeName)
                    return str(getattr(const, typeName));
            */}
        } else if (expression.operandID == operandDEFFLOAT) {
            if (expression.expressionValue != "")
                return expression.expressionValue;
            else
                ret << "defFloat.value==NULL";

        } else if (expression.operandID == operandDEFSTRING) {
            if (expression.expressionValue != "")
                return expression.expressionValue;
            else
                ret << "defString.value==NULL";
        }
        if (expression.operandID == operandIF) {
            //'If(%(arg1)s), Then (%(arg2)s)'
            Expression arg1Expression = sFxDataMgr.GetExpression(expression.arg1);
            Expression arg2expression = sFxDataMgr.GetExpression(expression.arg2);
            if ((arg1Expression.operandID == operandRS) and restricted)
                ret << ParseExpression(arg2expression, restricted);
            else {
                if (ParseExpression(arg1Expression, restricted) == "true")
                    ret << ParseExpression(arg2expression, restricted);
            }
        } else if (expression.operandID == operandINC) {
            //'%(arg1)s+=self.%(arg2)s'
            Expression arg1Expression = sFxDataMgr.GetExpression(expression.arg1);
            Expression arg2expression = sFxDataMgr.GetExpression(expression.arg2);
            ret << "IncreaseItemAttribute(" << ParseExpression(arg1Expression, restricted) << ", (Self->" << ParseExpression(arg2expression, restricted) << "))";
        } else if (expression.operandID == operandINCN) {
            //'%(arg1)s+=%(arg2)s'
            Expression arg1Expression = sFxDataMgr.GetExpression(expression.arg1);
            Expression arg2expression = sFxDataMgr.GetExpression(expression.arg2);
            ret << "IncreaseItemAttributeEx(" << ParseExpression(arg1Expression, restricted) << ", " << ParseExpression(arg2expression, restricted) << ")";
        } else if (expression.operandID == operandDEC) {
            //'%(arg1)s-=self.%(arg2)s'
            Expression arg1Expression = sFxDataMgr.GetExpression(expression.arg1);
            Expression arg2expression = sFxDataMgr.GetExpression(expression.arg2);
            ret << "DecreaseItemAttribute(" << ParseExpression(arg1Expression, restricted) << ", (Self->" << ParseExpression(arg2expression, restricted) << "))";
        } else if (expression.operandID == operandDECN) {
            //'%(arg1)s-=%(arg2)s'
            Expression arg1Expression = sFxDataMgr.GetExpression(expression.arg1);
            Expression arg2expression = sFxDataMgr.GetExpression(expression.arg2);
            ret << "DecreaseItemAttributeEx(" << ParseExpression(arg1Expression, restricted) << ", " << ParseExpression(arg2expression, restricted) << ")";
        } else if (expression.operandID == operandSET) {
            //'%(arg1)s := %(arg2)s'
            Expression arg1Expression = sFxDataMgr.GetExpression(expression.arg1);
            Expression arg2expression = sFxDataMgr.GetExpression(expression.arg2);
            ret << "SetAttributeValue(" << ParseExpression(arg1Expression, restricted) << ", " << ParseExpression(arg2expression, restricted) << ")";
        } else if (expression.operandID == operandOR) {
            //'%(arg1)s OR %(arg2)s'
            Expression arg1Expression = sFxDataMgr.GetExpression(expression.arg1);
            Expression arg2expression = sFxDataMgr.GetExpression(expression.arg2);
            if (arg1Expression.operandID == operandIF) {
                //'If(%(arg1)s), Then (%(arg2)s)'
                ret << ParseExpression(arg1Expression, restricted) << "\nelse:\n" << ParseExpression(arg2expression,  restricted);
            }
        } else if (expression.operandID == operandAND) {
            //'(%(arg1)s) AND (%(arg2)s)'
            Expression arg1Expression = sFxDataMgr.GetExpression(expression.arg1);
            Expression arg2expression = sFxDataMgr.GetExpression(expression.arg2);
            if (arg1Expression.operandID == operandSKILLCHECK)
                ret << ParseExpression(arg1Expression, restricted) << "\n" << ParseExpression(arg2expression, restricted);
        }
        if (ret == "") {
            if (expression.expressionAttributeID) {
                ret << expression.expressionAttributeID;
                return ret.str().c_str();
            }
            if (expression.expressionTypeID) {
                ret << expression.expressionTypeID;
                return ret.str().c_str();
            }
            if (expression.expressionGroupID) {
                ret << expression.expressionGroupID;
                return ret.str().c_str();
            }
            if (expression.operandID == operandDEFGROUP) {
                ret << "*groupNameByID*" << expression.expressionValue; // get group name by id here
                return ret.str().c_str();
            }
            if (expression.operandID == operandDEFTYPEID) {
                ret << "*typeNameByID*" << expression.expressionValue; // get type name by id here
                return ret.str().c_str();
            }
            std::string arg1= "", arg2 = "";
            if (expression.arg1)
                arg1 = ParseExpression(sFxDataMgr.GetExpression(expression.arg1), restricted);
            if (expression.arg2)
                arg2 = ParseExpression(sFxDataMgr.GetExpression(expression.arg2), restricted);
            if ((arg1 != "") and (arg1.find_first_of('\n') < 1))
                ret << "*arg1Newline*" ;   //arg1 = arg1.strip();
            if ((arg2 != "") and (arg2.find_first_of('\n') < 1))
                ret << "*arg2Newline*" ;   //arg2 = arg2.strip();
        }
        Operand operand = sFxDataMgr.GetOperand(expression.operandID);
        ret << "Operand id:" << expression.operandID << " key: " << operand.operandKey;
        if (operand.format == "")
            ret << " - has not been defined";
        else
            ret << " format: " << operand.format << " *needsWork*"; // % {'arg1': arg1, 'arg2': arg2, 'value': expression.expressionValue}
        return ret.str().c_str();
    }
}

