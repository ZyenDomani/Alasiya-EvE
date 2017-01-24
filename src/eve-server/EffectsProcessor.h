/**
 * @name EffectsProcessor.h
 *   This file is for decoding and processing the proprietary effect data format for Alasiya-EvE
 *   Copyright 2017  Alasiya-EVEmu Team
 *
 * @Author:    Allan
 * @date:      24 January 2017
 *
 */


#ifndef _EVE_FX_PROC_H__
#define _EVE_FX_PROC_H__

#include "EffectsData.h"
#include "eve-server.h"


//////////////////////////////////////////////////////////////////////////////////
// Modifier classes containing all data to modify an attribute
#pragma region Modifier

class Modifier
: public RefObject
{
public:
    Modifier(uint32 originatorID, uint32 targetAttributeID, uint32 targetID, bool penaltiesApply, double modifierValue, uint32 calcTypeID, uint32 revCalcTypeID)
    : RefObject( 0 )
    {
        m_OriginatorID = originatorID;
        m_TargetAttributeID = targetAttributeID;
        m_TargetID = targetID;
        m_bPenaltiesApply = penaltiesApply;
        m_ModifierValue = modifierValue;
        m_CalculationTypeID = calcTypeID;
        m_ReverseCalculationTypeID = revCalcTypeID;
    }

    ~Modifier();

    double GetModifierValue() { return m_ModifierValue; }
    void SetModifierValue(double newModifierValue) { m_ModifierValue = newModifierValue; }
    uint32 GetOriginatorID() { return m_OriginatorID; }

protected:
    uint32 m_OriginatorID;
    uint32 m_TargetAttributeID;
    uint32 m_TargetID;
    bool m_bPenaltiesApply;
    double m_ModifierValue;
    uint32 m_CalculationTypeID;
    uint32 m_ReverseCalculationTypeID;
};

typedef RefPtr<Modifier> ModifierRef;

typedef std::multimap<double, ModifierRef> ModifierMapType;     // The ModifierRef is NOT owned by the owner of instances of this type

class ModifierMap
{
public:
    ModifierMap() { m_MapIsDirty = false; }
    ~ModifierMap();

    bool m_MapIsDirty;
    ModifierMapType m_ModifierMap;   // Key= modifier value, Value= Modifier class object containing all data describing this modifier for this attribute
};

typedef std::map<uint32, ModifierMap *> ModifierMaps;   // Key= attributeID, Value= ModifierMap class object containing a map of all modifiers for this attribute

#pragma endregion
/////////////////////////////// END MODIFIER /////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
// Classes for passing effects around to targets
#pragma region Effect Passing

static const uint8 MAX_EFFECT_COUNT = 5;  //arbitrary, lazy etc.  The bigger this number, the larger these message classes will be

class SubEffect
{
public:

    SubEffect(uint32 attrID, EVECalculationType type, EvilNumber val, uint32 targetItemID = 0)
    : m_AttrID( attrID ), m_TargetItemID( targetItemID ), m_CalcType( type ), m_Val( val )
    {

    }

    virtual ~SubEffect() { }

    //gets
    uint32 AttributeID()                    { return m_AttrID; }
    uint32 TargetItemID()                   { return m_TargetItemID; }
    EVECalculationType CalculationType()    { return m_CalcType; }
    EvilNumber AppliedValue()               { return m_Val; }

private:
    uint32 m_AttrID;
    uint32 m_TargetItemID;
    EVECalculationType m_CalcType;
    EvilNumber m_Val;

};



class Effect
{
public:
    Effect()
    : m_Count( 0 )
    {

    }

    ~Effect()
    {
        for (int i = 0; i <= m_Count; i++)
        {
            delete m_SubEffects[i];
        }
    }

    void AddEffect(uint32 attributeID, EVECalculationType type, EvilNumber val, uint32 targetItemID = 0)
    {
        SubEffect * s = new SubEffect(attributeID, type, val, targetItemID);
        if( m_Count + 1 < MAX_EFFECT_COUNT )
        {
            m_SubEffects[m_Count] = s;
            ++m_Count;
        }
    }

    bool hasEffect() { return (m_Count > 0);  }

    SubEffect * next()
    {
        --m_Count;
        return m_SubEffects[m_Count];

    }

private:
    SubEffect * m_SubEffects[MAX_EFFECT_COUNT];
    int m_Count;

};

#pragma endregion
/////////////////////////// END MODULE EFFECTS //////////////////////////////////



class SystemEntity;

class FxProc
: public Singleton< FxProc >
{
public:
    FxProc();
    virtual ~FxProc();

    EvilNumber CalculateAttributeValue(EvilNumber val1, EvilNumber val2, EVECalculationType type);

protected:
    void ProcessExternalEffect(Effect* e);

    // External Methods For use by hostile entities directing effects to this entity:
    int32 ApplyRemoteEffect(uint32 attributeID, uint32 originatorID, SystemEntity * systemEntity, ModifierRef modifierRef);
    int32 RemoveRemoteEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef);

    int32 ApplySubsystemEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef);
    int32 RemoveSubsystemEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef);

    int32 ApplyShipSkillEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef);
    int32 RemoveShipSkillEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef);

    int32 ApplyModuleRigEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef);
    int32 RemoveModuleRigEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef);

    int32 ApplyImplantEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef);
    int32 RemoveImplantEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef);


private:

    void _processExternalEffect(SubEffect * e);

    // not used....may not be needed.
    //ModuleCommand _translateEffectName(std::string s);

    //modifier maps, we own these
    ModifierMaps* m_LocalSubsystemModifierMaps;    // Holds std::map<> maps of Modifiers for attributes applied by SUBSYSTEMS
    ModifierMaps* m_LocalShipSkillModifierMaps;    // Holds std::map<> maps of Modifiers for attributes applied by SHIPS and SKILLS
    ModifierMaps* m_LocalModuleRigModifierMaps;    // Holds std::map<> maps of Modifiers for attributes applied by MODULES and RIGS
    ModifierMaps* m_LocalImplantModifierMaps;      // Holds std::map<> maps of Modifiers for attributes applied by IMPLANTS
    ModifierMaps* m_RemoteModifierMaps;            // Holds std::map<> maps of Modifiers for attributes applied by EXTERNAL ENTITY MODULES


};

#define sFxProc \
( FxProc::get() )

#endif  // _EVE_FX_PROC_H__