/**
 * @name EffectsDataMgr.h
 *   This file is for retrieving, manipulating and managing effect data
 *   Copyright 2017  Alasiya-EVEmu Team
 *
 * @Author:    Allan
 * @date:      29 January 2017
 *
 */


#include "effects/EffectsDataMgr.h"
#include "effects/EffectsProcessor.h"


FxDataMgr::FxDataMgr()
{
    m_loaded = false;

    m_fxMap.clear();
    m_opMap.clear();
    m_expMap.clear();
    m_effectMap.clear();
    m_typeFxMap.clear();
}

FxDataMgr::~FxDataMgr()
{

}

void FxDataMgr::Initialize()
{
    if (m_loaded)
        return;

    double begin = GetTimeMSeconds();
    double start = GetTimeMSeconds();
    DBResultRow row;
    DBQueryResult* res = new DBQueryResult();

    GetDgmTypeEffects(*res);
    while (res->GetRow(row)) {
        //SELECT typeID, effectID, isDefault
        TypeEffects mTfx;
        mTfx.effectID = row.GetInt(1);
        mTfx.isDefault = row.GetBool(2);
        m_typeFxMap.insert(std::pair<uint16, TypeEffects>(row.GetInt(0), mTfx));
    }
    sLog.Cyan("        FxDataMgr", "%u Type Effects loaded in %.3fms.", m_typeFxMap.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    GetOperands(*res);
    while (res->GetRow(row)) {
        //SELECT operandID, operandKey, format, arg1categoryID, arg2categoryID, resultCategoryID
        Operand mOpr;
            mOpr.arg1categoryID = row.GetInt(3);
            mOpr.arg2categoryID = row.GetInt(4);
            mOpr.format = row.GetText(2);
            mOpr.operandKey = row.GetText(1);
            mOpr.resultCategoryID = row.GetInt(5);
        m_opMap.insert(std::pair<uint16, Operand>(row.GetInt(0), mOpr));
    }
    // insert a zero-value data set
    Operand mOpr;
        mOpr.arg1categoryID = 0;
        mOpr.arg2categoryID = 0;
        mOpr.format = "None";
        mOpr.operandKey = "NULL";
        mOpr.resultCategoryID = 0;
    m_opMap.insert(std::pair<uint16, Operand>(0, mOpr));
    sLog.Cyan("        FxDataMgr", "%u Operands loaded in %.3fms.", m_opMap.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    GetExpressions(*res);
    while (res->GetRow(row)) {
        //SELECT expressionID, operandID, arg1, arg2, expressionValue, description, expressionName, expressionTypeID, expressionGroupID, expressionAttributeID
        Expression mExp;
            mExp.id = row.GetInt(0);
            mExp.arg1 = (row.IsNull(2) ? 0 : row.GetInt(2));
            mExp.arg2 = (row.IsNull(3) ? 0 : row.GetInt(3));
            mExp.expressionAttributeID = (row.IsNull(9) ? 0 : row.GetInt(9));
            mExp.description = row.GetText(5);
            mExp.expressionGroupID = (row.IsNull(8) ? 0 : row.GetInt(8));
            mExp.expressionName = row.GetText(6);
            mExp.operandID = row.GetInt(1);
            mExp.expressionTypeID = (row.IsNull(7) ? 0 : row.GetInt(7));
            mExp.expressionValue = row.GetText(4);
        m_expMap.insert(std::pair<uint16, Expression>(row.GetInt(0), mExp));
    }
    // insert a zero-value data set
    Expression mExp;
        mExp.id = 0;
        mExp.arg1 = 0;
        mExp.arg2 = 0;
        mExp.expressionAttributeID = 0;
        mExp.description = "NULL";
        mExp.expressionGroupID = 0;
        mExp.expressionName = "NULL";
        mExp.operandID = 0;
        mExp.expressionTypeID = 0;
        mExp.expressionValue = "";
    m_expMap.insert(std::pair<uint16, Expression>(0, mExp));
    sLog.Cyan("        FxDataMgr", "%u Expressions loaded in %.3fms.", m_expMap.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    GetDgmEffects(*res);
    while (res->GetRow(row)) {
        //SELECT effectID, effectName, effectState, preExpression, postExpression, isOffensive, isAssistance, disallowAutoRepeat, isWarpSafe \
        //      npcUsageChanceAttributeID, npcActivationChanceAttributeID, fittingUsageChanceAttributeID,\
        //      durationAttributeID, trackingSpeedAttributeID, dischargeAttributeID, rangeAttributeID, falloffAttributeID, rangeChance, electronicChance, propulsionChance, guid
        Effect mEffect;
            mEffect.effectID = row.GetInt(0);
            mEffect.effectName = row.GetText(1);
            mEffect.effectState = row.GetInt(2);
            mEffect.preExpression = row.GetInt(3);
            mEffect.postExpression = row.GetInt(4);
            mEffect.isOffensive = row.GetBool(5);
            mEffect.isAssistance = row.GetBool(6);
            mEffect.disallowAutoRepeat = row.GetBool(7);
            mEffect.isWarpSafe = row.GetBool(8);
            mEffect.npcUsageChanceAttributeID = row.GetInt(9);
            mEffect.npcActivationChanceAttributeID = row.GetInt(10);
            mEffect.fittingUsageChanceAttributeID = row.GetInt(11);
            mEffect.durationAttributeID = row.GetInt(12);
            mEffect.trackingSpeedAttributeID = row.GetInt(13);
            mEffect.dischargeAttributeID = row.GetInt(14);
            mEffect.rangeAttributeID = row.GetInt(15);
            mEffect.falloffAttributeID = row.GetInt(16);
            mEffect.rangeChance = row.GetFloat(17);
            mEffect.electronicChance = row.GetFloat(18);
            mEffect.propulsionChance = row.GetFloat(19);
            mEffect.guid = row.GetText(20);
        m_effectMap.insert(std::pair<uint16, Effect>(row.GetInt(0), mEffect));
        m_effectName.insert(std::pair<std::string, uint16>(mEffect.effectName, row.GetInt(0)));
    }
    // insert a zero-value data set
    Effect mEffect;
        mEffect.effectName = "NULL";
        mEffect.effectState = 0;
        mEffect.preExpression = 0;
        mEffect.postExpression = 0;
        mEffect.isOffensive = false;
        mEffect.isAssistance = false;
        mEffect.disallowAutoRepeat = false;
        mEffect.isWarpSafe = false;
        mEffect.npcUsageChanceAttributeID = 0;
        mEffect.npcActivationChanceAttributeID = 0;
        mEffect.fittingUsageChanceAttributeID = 0;
        mEffect.durationAttributeID = 0;
        mEffect.trackingSpeedAttributeID = 0;
        mEffect.dischargeAttributeID = 0;
        mEffect.rangeAttributeID = 0;
        mEffect.falloffAttributeID = 0;
        mEffect.rangeChance = 0;
        mEffect.electronicChance = 0;
        mEffect.propulsionChance = 0;
        mEffect.guid = "";
    m_effectMap.insert(std::pair<uint16, Effect>(0, mEffect));
    sLog.Cyan("        FxDataMgr", "%u Effects loaded in %.3fms.", m_effectMap.size(), (GetTimeMSeconds() - start));

    //cleanup
    SafeDelete(res);

    m_loaded = true;
    sLog.Cyan("        FxDataMgr", "Effects Data loaded in %.3fms.", (GetTimeMSeconds() - begin));
}

void FxDataMgr::ConfigureEffects()
{
    sLog.Yellow("     Effects Test", "Test Begin - Process Prop Mod Effects.");
    double start = GetTimeMSeconds();
    Initialize();
    std::vector<uint16> types;
    std::vector<TypeEffects> typeFx;
    DBQueryResult res;
    DBResultRow row;
    sDatabase.RunQuery(res, "SELECT typeID, typeName FROM `invTypes` WHERE `groupID` IN (46.475)");
    FxProc fxProc;
    effectMapType::const_iterator itr;
    // begin the task of compiling effect data
    while (res.GetRow(row)) {
        typeFx.clear();
        sLog.Blue("ConfigureEffects", "getting data for skill %u (%s)", row.GetInt(0), row.GetText(1));
        GetTypeEffect(row.GetInt(0), typeFx);
        for (auto cur : typeFx) {
            itr = m_effectMap.find(cur.effectID);
            if (itr == m_effectMap.end()) {
                sLog.Blue("ConfigureEffects", "No effect found for effectID %u", cur.effectID);
                continue;
            }
            // we only want ONE copy of the effect
           // if (m_fxMap.find(itr->first) != m_fxMap.end())
             //   continue;
            sLog.Yellow("ConfigureEffects", "starting eval for %u:%u (%s)", itr->first, itr->second.effectState, itr->second.effectName.c_str());
            //fxProc.EvaluateExpression(itr->second.preExpression);
            //fxProc.EvaluateExpression(itr->second.postExpression);
            m_fxMap.insert(std::pair<uint16, Effect>(itr->first, itr->second));
        }
    }

    // save compiled effect data to avoid compilation on every startup?  -check for execution time  ...nah, it's fast enough
    //SaveFXData();
    m_time = (GetTimeMSeconds() - start);
}

Effect FxDataMgr::GetEffect(uint16 eID)
{
    effectMapType::const_iterator itr = m_effectMap.find(eID);
    if (itr != m_effectMap.end())
        return itr->second;
    return m_effectMap.at(0);
}

void FxDataMgr::GetTypeEffect(uint16 typeID, std::vector< TypeEffects >& typeEffMap)
{
    auto itr = m_typeFxMap.equal_range(typeID);
    for (auto it = itr.first; it != itr.second; ++it)
        typeEffMap.push_back(it->second);
}

Expression FxDataMgr::GetExpression(uint16 eID)
{
    std::map<uint16, Expression>::const_iterator itr = m_expMap.find(eID);
    if (itr != m_expMap.end())
        return itr->second;
    return m_expMap.at(0);
}

Operand FxDataMgr::GetOperand(uint16 oID)
{
    std::map<uint16, Operand>::const_iterator itr = m_opMap.find(oID);
    if (itr != m_opMap.end())
        return itr->second;
    return m_opMap.at(0);

}

bool FxDataMgr::isWarpSafe(uint16 eID)
{
    effectMapType::const_iterator itr = m_effectMap.find(eID);
    if (itr != m_effectMap.end())
        return itr->second.isWarpSafe;
    return false;   // default to false if effectID not found
}

bool FxDataMgr::isOffensive(uint16 eID)
{
    effectMapType::const_iterator itr = m_effectMap.find(eID);
    if (itr != m_effectMap.end())
        return itr->second.isOffensive;
    return false;   // default to false if effectID not found
}

bool FxDataMgr::isAssistance(uint16 eID)
{
    effectMapType::const_iterator itr = m_effectMap.find(eID);
    if (itr != m_effectMap.end())
        return itr->second.isAssistance;
    return false;   // default to false if effectID not found
}

uint16 FxDataMgr::GetEffectID(std::string effectName)
{
    std::map<std::string, uint16>::const_iterator itr = m_effectName.find(effectName);
    if (itr != m_effectName.end())
        return itr->second;
    return 0;
}

std::string FxDataMgr::GetEffectGuid(uint16 eID)
{
    effectMapType::const_iterator itr = m_effectMap.find(eID);
    if (itr != m_effectMap.end())
        return itr->second.guid;
    return "";   // default to 'nothing' if effectID not found
}

std::string FxDataMgr::GetEffectName(uint16 eID)
{
    effectMapType::const_iterator itr = m_effectMap.find(eID);
    if (itr != m_effectMap.end())
        return itr->second.effectName;
    return "";   // default to 'nothing' if effectID not found
}


void FxDataMgr::GetOperands(DBQueryResult& res)
{
    if( !sDatabase.RunQuery(res,
        " SELECT"
        " operandID,"
        " operandKey,"
        " format,"
        " arg1categoryID,"
        " arg2categoryID,"
        " resultCategoryID"
        " FROM dgmOperands"))
    {
        codelog(DATABASE__ERROR, "Error in GetOperands: %s", res.error.c_str());
    }
}

void FxDataMgr::GetDgmEffects(DBQueryResult& res)
{
    if( !sDatabase.RunQuery(res,
        " SELECT"
        "   effectID,"
        "   effectName,"
        "   effectCategory,"
        "   preExpression,"
        "   postExpression,"
        "   isOffensive,"
        "   isAssistance,"
        "   disallowAutoRepeat,"
        "   isWarpSafe,"
        "   npcUsageChanceAttributeID,"
        "   npcActivationChanceAttributeID,"
        "   fittingUsageChanceAttributeID,"
        "   durationAttributeID,"
        "   trackingSpeedAttributeID,"
        "   dischargeAttributeID,"
        "   rangeAttributeID,"
        "   falloffAttributeID,"
        "   rangeChance,"    // dunno what this is ...bool?
        "   electronicChance,"   // dunno what this is...bool?
        "   propulsionChance,"   // not used (all 0)...bool?
        "   guid"
        " FROM dgmEffects"))
    {
        codelog(DATABASE__ERROR, "Error in GetDgmEffects: %s", res.error.c_str());
    }
}

void FxDataMgr::GetExpressions(DBQueryResult& res)
{
    if( !sDatabase.RunQuery(res,
        " SELECT"
        " expressionID,"
        " operandID,"
        " arg1,"
        " arg2,"
        " expressionValue,"
        " description,"
        " expressionName,"
        " expressionTypeID,"
        " expressionGroupID,"
        " expressionAttributeID"
        " FROM dgmExpressions"))
    {
        codelog(DATABASE__ERROR, "Error in GetExpressions: %s", res.error.c_str());
    }
}

void FxDataMgr::GetDgmTypeEffects(DBQueryResult &res)
{
    if( !sDatabase.RunQuery(res,
        " SELECT"
        "  typeID,"
        "  effectID,"
        "  isDefault"
        " FROM dgmTypeEffects "
        " WHERE effectID != 132"))  // this maps skill level onto skill data.  we are tracking skill level, so dont need to process this effect
    {
        codelog(DATABASE__ERROR, "Error in GetDgmTypeEffects: %s", res.error.c_str());
    }
}
