/**
 * @name EffectsDataMgr.h
 *   This file is for retrieving, manipulating and managing effect data
 *   Copyright 2017  Alasiya-EVEmu Team
 *
 * @Author:    Allan
 * @date:      29 January 2017
 *
 */


#include "EffectsDataMgr.h"
#include "effects/EffectsProcessor.h"


FxDataMgr::FxDataMgr()
{
    m_loaded = false;

    m_fxMap.clear();
    m_opMap.clear();
    m_expMap.clear();
    m_effectMap.clear();
    m_typeFxMap.clear();
    m_stateFxMap.clear();
}

FxDataMgr::~FxDataMgr()
{

}

void FxDataMgr::Initialize()
{
    if (m_loaded)
        return;

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
        //SELECT effectID, effectName, effectCategory, preExpression, postExpression, isOffensive, isAssistance, disallowAutoRepeat, isWarpSafe \
        //      npcUsageChanceAttributeID, npcActivationChanceAttributeID, fittingUsageChanceAttributeID,\
        //      durationAttributeID, trackingSpeedAttributeID, dischargeAttributeID, rangeAttributeID, falloffAttributeID, rangeChance, electronicChance, propulsionChance
        Effect mEffect;
            mEffect.effectID = row.GetInt(0);
            mEffect.effectName = row.GetText(1);
            mEffect.effectCategory = row.GetInt(2);
            mEffect.preExpression = row.GetInt(3);
            mEffect.postExpression = row.GetInt(4);
            mEffect.isOffensive = (row.IsNull(5) ? 0 : row.GetBool(5));
            mEffect.isAssistance = (row.IsNull(6) ? 0 : row.GetBool(6));
            mEffect.disallowAutoRepeat = (row.IsNull(7) ? 0 : row.GetBool(7));
            mEffect.isWarpSafe = (row.IsNull(8) ? 0 : row.GetBool(8));
            mEffect.npcUsageChanceAttributeID = (row.IsNull(9) ? 0 : row.GetInt(9));
            mEffect.npcActivationChanceAttributeID = (row.IsNull(10) ? 0 : row.GetInt(10));
            mEffect.fittingUsageChanceAttributeID = (row.IsNull(11) ? 0 : row.GetInt(11));
            mEffect.durationAttributeID = (row.IsNull(12) ? 0 : row.GetInt(12));
            mEffect.trackingSpeedAttributeID = (row.IsNull(13) ? 0 : row.GetInt(13));
            mEffect.dischargeAttributeID = (row.IsNull(14) ? 0 : row.GetInt(14));
            mEffect.rangeAttributeID = (row.IsNull(15) ? 0 : row.GetInt(15));
            mEffect.falloffAttributeID = (row.IsNull(16) ? 0 : row.GetInt(16));
            mEffect.rangeChance = (row.IsNull(17) ? 0 : row.GetFloat(17));
            mEffect.electronicChance = (row.IsNull(18) ? 0 : row.GetFloat(18));
            mEffect.propulsionChance = (row.IsNull(19) ? 0 : row.GetFloat(19));
        m_effectMap.insert(std::pair<uint16, Effect>(row.GetInt(0), mEffect));
    }
    // insert a zero-value data set
    Effect mEffect;
        mEffect.effectName = "NULL";
        mEffect.effectCategory = 0;
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
    m_effectMap.insert(std::pair<uint16, Effect>(0, mEffect));
    sLog.Cyan("        FxDataMgr", "%u Effects loaded in %.3fms.", m_effectMap.size(), (GetTimeMSeconds() - start));

    //cleanup
    SafeDelete(res);

    m_loaded = true;
}

void FxDataMgr::ConfigureEffects(std::vector<Effect>& effectMap)
{
    double start = GetTimeMSeconds();
    Initialize();

    FxProc fxProc;
    // begin the task of compiling effect data
    for (auto curFx : effectMap) {
        // we only want ONE copy of the effect
        if (m_fxMap.find(curFx.effectID) != m_fxMap.end())
            continue;
        EffectsData tFxData;
            tFxData.isOffensive = curFx.isOffensive;
            tFxData.isAssistance = curFx.isAssistance;
            tFxData.disallowAutoRepeat = curFx.disallowAutoRepeat;
            tFxData.isWarpSafe = curFx.isWarpSafe;
            tFxData.effectCategory = curFx.effectCategory;
            tFxData.effectName = curFx.effectName;
            tFxData.preExpression = curFx.preExpression;
            tFxData.postExpression = curFx.postExpression;
            tFxData.npcUsageChanceAttributeID = curFx.npcUsageChanceAttributeID;
            tFxData.npcActivationChanceAttributeID = curFx.npcActivationChanceAttributeID;
            tFxData.fittingUsageChanceAttributeID = curFx.fittingUsageChanceAttributeID;
            tFxData.durationAttributeID = curFx.durationAttributeID;
            tFxData.trackingSpeedAttributeID = curFx.trackingSpeedAttributeID;
            tFxData.dischargeAttributeID = curFx.dischargeAttributeID;
            tFxData.rangeAttributeID = curFx.rangeAttributeID;
            tFxData.falloffAttributeID = curFx.falloffAttributeID;
            tFxData.rangeChance = curFx.rangeChance;
            tFxData.electronicChance = curFx.electronicChance;
            tFxData.propulsionChance = curFx.propulsionChance;

        EffectData fxData1;
        tFxData.pre = fxData1;
        sLog.Blue("ConfigureEffects", "starting pre eval for effect %u", curFx.effectID);
        fxProc.EvaluateExpression(curFx.preExpression, tFxData.pre);

        EffectData fxData2;
        tFxData.post = fxData2;
        sLog.Blue("ConfigureEffects", "starting post eval for effect %u", curFx.effectID);
        fxProc.EvaluateExpression(curFx.postExpression, tFxData.post);

        m_fxMap.insert(std::pair<uint16, EffectsData>(curFx.effectID, tFxData));
        m_stateFxMap.insert(std::pair<uint16, EffectsData>(tFxData.effectCategory, tFxData));
    }

    // save compiled effect data to avoid compilation on every startup?  -check for execution time
    //SaveFXData();
    m_time = (GetTimeMSeconds() - start);
}

Effect FxDataMgr::GetEffect(uint16 eID)
{
    effectMapType::iterator itr = m_effectMap.find(eID);
    if (itr != m_effectMap.end())
        return itr->second;
    return m_effectMap.at(0);
}

void FxDataMgr::GetTypeEffect(uint16 typeID, std::vector< TypeEffects >& typeEffMap)
{
    auto itr = m_typeFxMap.equal_range(typeID);
    for (auto it = itr.first; it != itr.second; it++) {
        typeEffMap.push_back(it->second);
    }
}

Expression FxDataMgr::GetExpression(uint16 eID)
{
    std::map<uint16, Expression>::iterator itr = m_expMap.find(eID);
    if (itr != m_expMap.end())
        return itr->second;
    return m_expMap.at(0);
}

Operand FxDataMgr::GetOperand(uint16 oID)
{
    std::map<uint16, Operand>::iterator itr = m_opMap.find(oID);
    if (itr != m_opMap.end())
        return itr->second;
    return m_opMap.at(0);

}

void FxDataMgr::ApplyEffect()
{

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
        "   propulsionChance"   // not used (all 0)...bool?
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
        " FROM dgmTypeEffects "))
    {
        codelog(DATABASE__ERROR, "Error in GetDgmTypeEffects: %s", res.error.c_str());
    }
}

void FxDataMgr::SaveFXData()
{
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO dgmFxData";
    Inserts << " (effectID, effectName, effectCategory, preExpression, postExpression, isOffensive, isAssistance, disallowAutoRepeat, isWarpSafe)";
    bool first = true;

    for (auto cur : m_fxMap) {
        if (first) {
            Inserts << " VALUES ";
            first = false;
        } else
            Inserts << ", ";
        Inserts << "(" << cur.first << ", " << cur.second.effectName << ", " << cur.second.effectCategory << ", " << cur.second.preExpression << ", " << cur.second.postExpression << ", ";
        Inserts << (cur.second.isOffensive ? 1 : 0) << ", " << (cur.second.isAssistance ? 1 : 0) << ", " << (cur.second.disallowAutoRepeat ? 1 : 0) << ", " << (cur.second.isWarpSafe ? 1 : 0) << ")";
    }

    if (!first) {
        DBerror err;
        sDatabase.RunQuery(err, "TRUNICATE TABLE dgmFxData");
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SaveFXData - unable to save data - %s", err.c_str());
    }
}
