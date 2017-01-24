/**
 * @name EffectsProcessor.cpp
 *   This file is for decoding and processing the proprietary effect data format for Alasiya-EvE
 *   Copyright 2017  Alasiya-EVEmu Team
 *
 * @Author:    Allan
 * @date:      24 January 2017
 *
 */

#include "EffectsProcessor.h"

FxProc::FxProc()
{

    //modifier maps, we own these
    m_LocalSubsystemModifierMaps = new ModifierMaps;
    m_LocalShipSkillModifierMaps = new ModifierMaps;
    m_LocalModuleRigModifierMaps = new ModifierMaps;
    m_LocalImplantModifierMaps = new ModifierMaps;
    m_RemoteModifierMaps = new ModifierMaps;
}

FxProc::~FxProc()
{

    //modifier map cleanup is handled in the std::map destructor
    delete m_LocalSubsystemModifierMaps;
    delete m_LocalShipSkillModifierMaps;
    delete m_LocalModuleRigModifierMaps;
    delete m_LocalImplantModifierMaps;
    delete m_RemoteModifierMaps;
    m_LocalSubsystemModifierMaps = nullptr;
    m_LocalShipSkillModifierMaps = nullptr;
    m_LocalModuleRigModifierMaps = nullptr;
    m_LocalImplantModifierMaps = nullptr;
    m_RemoteModifierMaps = nullptr;
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



// these below are not used yet.  not sure what they're actually for, or if ill even implement them.
void FxProc::ProcessExternalEffect(Effect* e)
{
    while (e->hasEffect())
        _processExternalEffect(e->next());
}

int32 FxProc::ApplyRemoteEffect(uint32 attributeID, uint32 originatorID, SystemEntity * systemEntity, ModifierRef modifierRef)
{
    sLog.Magenta("FxProc::ApplyRemoteEffect()","Needs to be implemented");
    return 1;
}

int32 FxProc::RemoveRemoteEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    sLog.Magenta("FxProc::RemoveRemoteEffect()","Needs to be implemented");
    return 1;
}

int32 FxProc::ApplySubsystemEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    ModifierMap* modMap = nullptr;

    // Make sure the ModifierRef passed in is not NULL:
    if (!modifierRef.get())
        return -1;

    // Check to see if this attributeID does not have a ModifierMap in the Map of ModifierMaps
    if ( m_LocalSubsystemModifierMaps->find(attributeID) == m_LocalSubsystemModifierMaps->end() )
    {
        // A Modifier Map for this attributeID does not exist, create a new one:
        modMap = new ModifierMap();
        if (!modMap)
            return -1;
    }
    else
    {
        // A Modifier Map for this attributeID already exists, find it and get its pointer:
        modMap = m_LocalSubsystemModifierMaps->find(attributeID)->second;
        if (!modMap)
            return -1;
    }

    // Check to see if the modifier map has any entries corresponding to the passed-in modifier's value:
    if ( modMap->m_ModifierMap.find(modifierRef->GetModifierValue()) != modMap->m_ModifierMap.end() )
    {
        // Modifier entry in this attributeID's Modifier Map already exists (modifierRef->GetModifierValue() found a match),
        // so check its originatorID and if that matches, DO NOT add this Modifier object to the map as the reference
        // already exists, the Module class can modify the contents of the Modifier object without really calling this function,
        // however, to maintain consistent code, the Module classes will always call this function to notify the map class
        // that the contents of the map was changed, or made 'dirty':
        modMap->m_MapIsDirty = true;
        ModifierMapType::iterator cur;
        std::pair<ModifierMapType::iterator,ModifierMapType::iterator> range;
        range = modMap->m_ModifierMap.equal_range(modifierRef->GetModifierValue());   // Get the one or more modifier map entries matching this modifier being added
        for (cur=range.first; cur!=range.second; ++cur)
            if ( cur->second->GetOriginatorID() == originatorID )
                return 1;   // Yep, we found the Modifier owned by this originatorID, so we return "success" because the Module
                            // class object already updated this Modifier through its own ModifierRef, we don't need to do anything
                            // else here except return and prevent ADDING to the ModifierMap

        // For loop searching existing modifiers completed, so this originatorID's Modifier
        // is NOT in the map yet... Let's add it:
        modMap->m_ModifierMap.insert(std::pair<double, ModifierRef>(modifierRef->GetModifierValue(), modifierRef));
    }
    else
    {
        // Modifier entry in this attributeID's Modifier Map does not exist yet, so lets insert it for the first time:
        // Insert the (modifierValue, ModifierRef) pair into the Modifier Map for this attributeID:
        modMap->m_ModifierMap.insert(std::pair<double, ModifierRef>(modifierRef->GetModifierValue(), modifierRef));
        modMap->m_MapIsDirty = true;
        m_LocalSubsystemModifierMaps->insert(std::pair<uint32, ModifierMap *>(attributeID, modMap));
    }

    return 1;
}

int32 FxProc::RemoveSubsystemEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    bool bModifierFound = false;
    ModifierMap * modMap = nullptr;

    if ( m_LocalSubsystemModifierMaps->find(attributeID) != m_LocalSubsystemModifierMaps->end() )
    {
        modMap = m_LocalSubsystemModifierMaps->find(attributeID)->second;
        modMap->m_MapIsDirty = true;

        if ( modMap->m_ModifierMap.find(modifierRef->GetModifierValue()) != modMap->m_ModifierMap.end() )
        {
            modMap->m_MapIsDirty = true;
            ModifierMapType::iterator cur;
            std::pair<ModifierMapType::iterator,ModifierMapType::iterator> range;
            range = modMap->m_ModifierMap.equal_range(modifierRef->GetModifierValue());   // Get the one or more modifier map entries matching this modifier being removed
            for (cur=range.first; cur!=range.second; ++cur)
                if ( cur->second->GetOriginatorID() == originatorID )
                {
                    bModifierFound = true;  // Yep, we found the Modifier owned by this originatorID, so we break out of the for ()
                                            // so we can now remove this exact Modifier object from the multimap
                    break;
                }

            if ( bModifierFound == true )
            {
                // For loop searching existing modifiers completed, so this originatorID's Modifier
                // was found in the map
                modMap->m_ModifierMap.insert(std::pair<double, ModifierRef>(modifierRef->GetModifierValue(), modifierRef));
            }
            else
                return -1;  // This modifier's originatorID was not found in the map, so return error code
        }
        else
            return -1;  // This modifier's modifier value was not even found in the map, so return error code
    }
    else
        return -1;  // Modifier Map for supplied attributeID does not exist, return error value

    return 1;
}

int32 FxProc::ApplyShipSkillEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    ModifierMap * modMap = nullptr;

    // Make sure the ModifierRef passed in is not NULL:
    if (!modifierRef.get())
        return -1;

    // Check to see if this attributeID does not have a ModifierMap in the Map of ModifierMaps
    if ( m_LocalShipSkillModifierMaps->find(attributeID) == m_LocalShipSkillModifierMaps->end() )
    {
        // A Modifier Map for this attributeID does not exist, create a new one:
        modMap = new ModifierMap();
        if (!modMap)
            return -1;
    }
    else
    {
        // A Modifier Map for this attributeID already exists, find it and get its pointer:
        modMap = m_LocalShipSkillModifierMaps->find(attributeID)->second;
        if (!modMap)
            return -1;
    }

    // Check to see if the modifier map has any entries corresponding to the passed-in modifier's value:
    if ( modMap->m_ModifierMap.find(modifierRef->GetModifierValue()) != modMap->m_ModifierMap.end() )
    {
        // Modifier entry in this attributeID's Modifier Map already exists (modifierRef->GetModifierValue() found a match),
        // so check its originatorID and if that matches, DO NOT add this Modifier object to the map as the reference
        // already exists, the Module class can modify the contents of the Modifier object without really calling this function,
        // however, to maintain consistent code, the Module classes will always call this function to notify the map class
        // that the contents of the map was changed, or made 'dirty':
        modMap->m_MapIsDirty = true;
        ModifierMapType::iterator cur;
        std::pair<ModifierMapType::iterator,ModifierMapType::iterator> range;
        range = modMap->m_ModifierMap.equal_range(modifierRef->GetModifierValue());   // Get the one or more modifier map entries matching this modifier being added
        for (cur=range.first; cur!=range.second; ++cur)
            if ( cur->second->GetOriginatorID() == originatorID )
                return 1;   // Yep, we found the Modifier owned by this originatorID, so we return "success" because the Module
                            // class object already updated this Modifier through its own ModifierRef, we don't need to do anything
                            // else here except return and prevent ADDING to the ModifierMap

        // For loop searching existing modifiers completed, so this originatorID's Modifier
        // is NOT in the map yet... Let's add it:
        modMap->m_ModifierMap.insert(std::pair<double, ModifierRef>(modifierRef->GetModifierValue(), modifierRef));
    }
    else
    {
        // Modifier entry in this attributeID's Modifier Map does not exist yet, so lets insert it for the first time:
        // Insert the (modifierValue, ModifierRef) pair into the Modifier Map for this attributeID:
        modMap->m_ModifierMap.insert(std::pair<double, ModifierRef>(modifierRef->GetModifierValue(), modifierRef));
        modMap->m_MapIsDirty = true;
        m_LocalShipSkillModifierMaps->insert(std::pair<uint32, ModifierMap *>(attributeID, modMap));
    }

    return 1;
}

int32 FxProc::RemoveShipSkillEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    bool bModifierFound = false;
    ModifierMap * modMap = nullptr;

    if ( m_LocalShipSkillModifierMaps->find(attributeID) != m_LocalShipSkillModifierMaps->end() )
    {
        modMap = m_LocalShipSkillModifierMaps->find(attributeID)->second;
        modMap->m_MapIsDirty = true;

        if ( modMap->m_ModifierMap.find(modifierRef->GetModifierValue()) != modMap->m_ModifierMap.end() )
        {
            modMap->m_MapIsDirty = true;
            ModifierMapType::iterator cur;
            std::pair<ModifierMapType::iterator,ModifierMapType::iterator> range;
            range = modMap->m_ModifierMap.equal_range(modifierRef->GetModifierValue());   // Get the one or more modifier map entries matching this modifier being removed
            for (cur=range.first; cur!=range.second; ++cur)
                if ( cur->second->GetOriginatorID() == originatorID )
                {
                    bModifierFound = true;  // Yep, we found the Modifier owned by this originatorID, so we break out of the for ()
                                            // so we can now remove this exact Modifier object from the multimap
                    break;
                }

            if ( bModifierFound == true )
            {
                // For loop searching existing modifiers completed, so this originatorID's Modifier
                // was found in the map
                modMap->m_ModifierMap.insert(std::pair<double, ModifierRef>(modifierRef->GetModifierValue(), modifierRef));
            }
            else
                return -1;  // This modifier's originatorID was not found in the map, so return error code
        }
        else
            return -1;  // This modifier's modifier value was not even found in the map, so return error code
    }
    else
        return -1;  // Modifier Map for supplied attributeID does not exist, return error value

    return 1;
}

int32 FxProc::ApplyModuleRigEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    sLog.Magenta("FxProc::ApplyModuleRigEffect()","Needs to be implemented");
    return 1;
}

int32 FxProc::RemoveModuleRigEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    sLog.Magenta("FxProc::RemoveModuleRigEffect()","Needs to be implemented");
    return 1;
}

int32 FxProc::ApplyImplantEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    sLog.Magenta("FxProc::ApplyImplantEffect()","Needs to be implemented");
    return 1;
}

int32 FxProc::RemoveImplantEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    sLog.Magenta("FxProc::RemoveImplantEffect()","Needs to be implemented");
    return 1;
}

void FxProc::_processExternalEffect(SubEffect * s)
{
    /*
    //50-50 it's targeting a specific module ( i'm assuming here )
    GenericModule* mod = m_Modules->GetModule(s->TargetItemID());
    if (mod)
    {
        //calculate new attribute
        mod->SetAttribute(s->AttributeID(),
                          CalculateAttributeValue(mod->GetAttribute(s->AttributeID()),
                                                                       s->AppliedValue(), s->CalculationType()));
    }
    else if ( s->TargetItemID() == m_Ship->itemID() ) //guess it's not, but that means it should be targeting our ship itself
    {
        //calculate new attribute
        m_Ship->SetAttribute(s->AttributeID(),
                             CalculateAttributeValue(m_Ship->GetAttribute(s->AttributeID()),
                                                                             s->AppliedValue(), s->CalculationType()));
    }
    else //i have no idea what their targeting X_X
        sLog.Error("FxProc", "Process external effect inconsistency.  This shouldn't happen");
*/
}

// not used
/*
ModuleCommand FxProc::_translateEffectName(std::string s)
{
    //slow but it's better to do it once then many times as it gets passed around in modules or w/e
    //all modules should expect a ModuleCommand instead of a string

    //slightly faster version for when I know what things are really called
    //might as well use, but will definately not be right

    switch(s[0])
    {
        case 'a': return ACTIVATE;
        case 'd': return (s[2] == 'a' ? DEACTIVATE : DEOVERLOAD);
        case 'o': return (s[1] == 'n' ? ONLINE : (s[1] == 'f' ? OFFLINE : OVERLOAD)); //compound booleans ftw
    }

    return CMD_ERROR;
}
*/