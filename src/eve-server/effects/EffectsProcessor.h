/**
 * @name EffectsProcessor.h
 *   This file is for decoding and processing the effect data
 *   Copyright 2017  Alasiya-EVEmu Team
 *
 * @Author:    Allan
 * @date:      24 January 2017
 *
 */


#ifndef _EVE_FX_PROC_H__
#define _EVE_FX_PROC_H__

#include "effects/EffectsDataMgr.h"



class FxProc
//: public Singleton< FxProc >
{
public:
    FxProc();
    virtual ~FxProc();

    EvilNumber CalculateAttributeValue(EvilNumber val1, EvilNumber val2, EVECalculationType type);

    // new effects system
    void EvaluateExpression(const uint16 expID, EffectData& fxData);
    int8 GetEnvironmentEnum(const std::string& domain);
    int8 GetAssociationEnum(const std::string& association);

    std::string ParseExpression(Expression expression, bool restricted = false, bool topLevel = false);

protected:

private:


};

/*
#define sFxProc \
( FxProc::get() )
*/

#endif  // _EVE_FX_PROC_H__
