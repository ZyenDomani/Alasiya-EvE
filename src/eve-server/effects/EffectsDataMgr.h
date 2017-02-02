/**
 * @name EffectsDataMgr.h
 *   This file is for retrieving, manipulating and managing effect data
 *   Copyright 2017  Alasiya-EVEmu Team
 *
 * @Author:    Allan
 * @date:      29 January 2017
 *
 */


#ifndef _EVE_FX_PROC_DATAMGR_H__
#define _EVE_FX_PROC_DATAMGR_H__

#include <unordered_map>
#include "effects/EffectsData.h"

class FxDataMgr
: public Singleton< FxDataMgr >
{
public:
    FxDataMgr();
    ~FxDataMgr();

    void Initialize();
    void ConfigureEffects(std::vector<Effect>& effectMap);

    void ApplyEffect();

    Effect GetEffect(uint16 eID);
    Operand GetOperand(uint16 oID);
    Expression GetExpression(uint16 eID);

    void GetTypeEffect(uint16 typeID, std::vector< TypeEffects >& typeEffMap);

    float GetFxTime()                                   { return m_time; }
    uint16 GetFxSize()                                  { return m_fxMap.size(); }

protected:
    void SaveFXData();

    void GetOperands(DBQueryResult& res);
    void GetDgmEffects(DBQueryResult& res);
    void GetExpressions(DBQueryResult& res);
    void GetDgmTypeEffects(DBQueryResult &res);

private:
    bool m_loaded;
    float m_time;

    // data maps
    std::map<uint16, EffectsData> m_fxMap;   // k,v of effID, data   -to search by effect
    std::unordered_multimap<uint16, EffectsData> m_stateFxMap;  // k,v of state, data   -to search by state

    // these are temp to build effect data table
    effectMapType m_effectMap;  //std::map<uint16, Effect>
    std::map<uint16, Expression> m_expMap;
    std::map<uint16, Operand> m_opMap;
    std::unordered_multimap<uint16, TypeEffects> m_typeFxMap;  // k,v of typeID, data<effectID, isDefault>
};

#define sFxDataMgr \
( FxDataMgr::get() )

#endif  // _EVE_FX_PROC_DATAMGR_H__