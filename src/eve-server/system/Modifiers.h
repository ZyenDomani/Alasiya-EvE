/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2011 The EVEmu Team
 *    For the latest information visit http://evemu.org
 *    ------------------------------------------------------------------------------------
 *    This program is free software; you can redistribute it and/or modify it under
 *    the terms of the GNU Lesser General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option) any later
 *    version.
 *
 *    This program is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License along with
 *    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 *    http://www.gnu.org/copyleft/lesser.txt.
 *    ------------------------------------------------------------------------------------
 *    Author:        Aknor Jaden, Luck
 */

#ifndef EVEMU_SYSTEM_MODIFIERS_H__
#define EVEMU_SYSTEM_MODIFIERS_H__

#include "ship/modules/ModuleDB.h"
#include "ship/modules/ModuleDefs.h"
#include "ship/modules/ModuleEffects.h"
#include "utils/Singleton.h"



// ///////////////////// Skill Modifiers Class ///////////////////////////
class SkillBonusModifier
{
public:
    SkillBonusModifier(uint32 skillID);
    ~SkillBonusModifier();

    //accessors for the data loaded, if any, from the dgmSkillBonusModifiers table:
    uint32 GetSkillID()                                          { return m_SkillID; }
    uint32 GetSizeOfModifierList()                                { return ((m_SkillID == 0) || (!m_ModifierLoaded)) ? 0 : m_numOfIDs; }
    uint32 GetEffectID(uint32 index)                              { return ((m_SkillID == 0) || (!m_ModifierLoaded)) ? 0 : m_EffectIDs[index]; }
    uint32 GetSourceAttributeID(uint32 index)                    { return ((m_SkillID == 0) || (!m_ModifierLoaded)) ? 0 : m_SourceAttributeIDs[index]; }
    uint32 GetTargetAttributeID(uint32 index)                    { return ((m_SkillID == 0) || (!m_ModifierLoaded)) ? 0 : m_TargetAttributeIDs[index]; }
    EVECalculationType GetCalculationType(uint32 index)            { return ((m_SkillID == 0) || (!m_ModifierLoaded)) ? (EVECalculationType)0 : (EVECalculationType)m_CalculationTypeIDs[index];}
    EVECalculationType GetReverseCalculationType(uint32 index)    { return ((m_SkillID == 0) || (!m_ModifierLoaded)) ? (EVECalculationType)0 : (EVECalculationType)m_ReverseCalculationTypeIDs[index];}
    typeTargetGroupIDlist * GetTargetGroupIDlist(uint32 index)    { return ((m_SkillID == 0) || (!m_ModifierLoaded)) ? 0 : m_TargetGroupIDlists.find(index)->second; }
    uint32 GetTargetChargeSize(uint32 index)                    { return ((m_SkillID == 0) || (!m_ModifierLoaded)) ? 0 : m_TargetChargeSizes[index]; }
    uint32 GetAppliedPerLevel(uint32 index)                    { return ((m_SkillID == 0) || (!m_ModifierLoaded)) ? 0 : m_AppliedPerLevelList[index]; }
    uint32 GetTargetTypeToWhichEffectApplied(uint32 index)        { return ((m_SkillID == 0) || (!m_ModifierLoaded)) ? 0 : m_AffectedTypes[index]; }
    uint32 GetEffectApplicationType(uint32 index)               { return ((m_SkillID == 0) || (!m_ModifierLoaded)) ? 0 : m_targetType[index]; }

    bool IsModifierLoaded() { return m_ModifierLoaded; }

private:
    void _Populate(uint32 skillID);

    bool m_ModifierLoaded;

    uint32 m_SkillID;
    uint32 m_numOfIDs;

    uint32* m_EffectIDs;
    uint32* m_SourceAttributeIDs;
    uint32* m_TargetAttributeIDs;
    uint32* m_CalculationTypeIDs;
    uint32* m_ReverseCalculationTypeIDs;
    uint32* m_TargetChargeSizes;
    uint32* m_AppliedPerLevelList;
    uint32* m_targetType;
    uint32* m_AffectedTypes;

    std::map<uint32, std::string> m_Descriptions;
    std::map<uint32, typeTargetGroupIDlist*> m_TargetGroupIDlists;
};
//////////////////////////////////////////////////////////////////////////


// ///////////////////// Ship Modifiers Class ///////////////////////////
class ShipBonusModifier
{
public:
    ShipBonusModifier(uint32 shipID);
    ~ShipBonusModifier();

    //accessors for the data loaded, if any, from the dgmSkillBonusModifiers table:
    uint32 GetShipID()                                          { return m_ShipID; }
    uint32 GetSizeOfModifierList()                                { return ((m_ShipID == 0) || (!m_ModifierLoaded)) ? 0 : m_numOfIDs; }
    uint32 GetEffectID(uint32 index)                              { return ((m_ShipID == 0) || (!m_ModifierLoaded)) ? 0 : m_EffectIDs[index]; }
    uint32 GetAttributeSkillID(uint32 index)                    { return ((m_ShipID == 0) || (!m_ModifierLoaded)) ? 0 : m_AttributeSkillIDs[index]; }
    uint32 GetSourceAttributeID(uint32 index)                    { return ((m_ShipID == 0) || (!m_ModifierLoaded)) ? 0 : m_SourceAttributeIDs[index]; }
    uint32 GetTargetAttributeID(uint32 index)                    { return ((m_ShipID == 0) || (!m_ModifierLoaded)) ? 0 : m_TargetAttributeIDs[index]; }
    EVECalculationType GetCalculationType(uint32 index)            { return ((m_ShipID == 0) || (!m_ModifierLoaded)) ? (EVECalculationType)0 : (EVECalculationType)m_CalculationTypeIDs[index];}
    EVECalculationType GetReverseCalculationType(uint32 index)    { return ((m_ShipID == 0) || (!m_ModifierLoaded)) ? (EVECalculationType)0 : (EVECalculationType)m_ReverseCalculationTypeIDs[index];}
    typeTargetGroupIDlist * GetTargetGroupIDlist(uint32 index)    { return ((m_ShipID == 0) || (!m_ModifierLoaded)) ? 0 : m_TargetGroupIDlists.find(index)->second; }
    uint32 GetAppliedPerLevel(uint32 index)                    { return ((m_ShipID == 0) || (!m_ModifierLoaded)) ? 0 : m_AppliedPerLevelList[index]; }
    uint32 GetTargetTypeToWhichEffectApplied(uint32 index)        { return ((m_ShipID == 0) || (!m_ModifierLoaded)) ? 0 : m_AffectedTypes[index]; }
    uint32 GetEffectApplicationType(uint32 index)               { return ((m_ShipID == 0) || (!m_ModifierLoaded)) ? 0 : m_targetType[index]; }

    bool IsModifierLoaded() { return m_ModifierLoaded; }

private:
    void _Populate(uint32 shipID);

    bool m_ModifierLoaded;

    uint32 m_ShipID;
    uint32 m_numOfIDs;

    uint32* m_EffectIDs;
    uint32* m_AttributeSkillIDs;
    uint32* m_SourceAttributeIDs;
    uint32* m_TargetAttributeIDs;
    uint32* m_CalculationTypeIDs;
    uint32* m_ReverseCalculationTypeIDs;
    uint32* m_AppliedPerLevelList;
    uint32* m_targetType;
    uint32* m_AffectedTypes;

    std::map<uint32, std::string> m_Descriptions;
    std::map<uint32, typeTargetGroupIDlist*> m_TargetGroupIDlists;
};
//////////////////////////////////////////////////////////////////////////

// ///////////////////// Implant Modifiers Class ///////////////////////////
class ImplantModifier
{
public:
    ImplantModifier(uint32 implantID);
    ~ImplantModifier();

    //accessors for the data loaded, if any, from the dgmImplantModifiers table:
    uint32 GetImplantID()                                          { return m_implantID; }
    uint32 GetSizeOfModifierList()                                { return ((m_implantID == 0) || (!m_ModifierLoaded)) ? 0 : m_numOfIDs; }
    uint32 GetEffectID(uint32 index)                              { return ((m_implantID == 0) || (!m_ModifierLoaded)) ? 0 : m_EffectIDs[index]; }
    uint32 GetAttributeSkillID(uint32 index)                    { return ((m_implantID == 0) || (!m_ModifierLoaded)) ? 0 : m_AttributeSkillIDs[index]; }
    uint32 GetSourceAttributeID(uint32 index)                    { return ((m_implantID == 0) || (!m_ModifierLoaded)) ? 0 : m_SourceAttributeIDs[index]; }
    uint32 GetTargetAttributeID(uint32 index)                    { return ((m_implantID == 0) || (!m_ModifierLoaded)) ? 0 : m_TargetAttributeIDs[index]; }
    EVECalculationType GetCalculationType(uint32 index)            { return ((m_implantID == 0) || (!m_ModifierLoaded)) ? (EVECalculationType)0 : (EVECalculationType)m_CalculationTypeIDs[index];}
    EVECalculationType GetReverseCalculationType(uint32 index)    { return ((m_implantID == 0) || (!m_ModifierLoaded)) ? (EVECalculationType)0 : (EVECalculationType)m_ReverseCalculationTypeIDs[index];}
    typeTargetGroupIDlist * GetTargetGroupIDlist(uint32 index)    { return ((m_implantID == 0) || (!m_ModifierLoaded)) ? 0 : m_TargetGroupIDlists.find(index)->second; }
    uint32 GetAppliedPerLevel(uint32 index)                    { return ((m_implantID == 0) || (!m_ModifierLoaded)) ? 0 : m_AppliedPerLevelList[index]; }
    uint32 GetTargetTypeToWhichEffectApplied(uint32 index)        { return ((m_implantID == 0) || (!m_ModifierLoaded)) ? 0 : m_AffectedTypes[index]; }
    uint32 GetEffectApplicationType(uint32 index)               { return ((m_implantID == 0) || (!m_ModifierLoaded)) ? 0 : m_targetType[index]; }

    bool IsModifierLoaded() { return m_ModifierLoaded; }

private:
    void _Populate(uint32 implantID);

    bool m_ModifierLoaded;

    uint32 m_implantID;
    uint32 m_numOfIDs;

    uint32* m_EffectIDs;
    uint32* m_AttributeSkillIDs;
    uint32* m_SourceAttributeIDs;
    uint32* m_TargetAttributeIDs;
    uint32* m_CalculationTypeIDs;
    uint32* m_ReverseCalculationTypeIDs;
    uint32* m_AppliedPerLevelList;
    uint32* m_targetType;
    uint32* m_AffectedTypes;

    std::map<uint32, std::string> m_Descriptions;
    std::map<uint32, typeTargetGroupIDlist*> m_TargetGroupIDlists;
};
//////////////////////////////////////////////////////////////////////////

// This class is a singleton object, containing all Skill Bonus Modifiers loaded from dgmSkillBonusModifiers table as memory objects of type SkillBonusModifier:
class DGM_Skill_Bonus_Modifiers_Table
: public Singleton< DGM_Skill_Bonus_Modifiers_Table >
{
public:
    DGM_Skill_Bonus_Modifiers_Table();
    ~DGM_Skill_Bonus_Modifiers_Table();

    // Initializes the Table:
    int Initialize();

    // Returns pointer to SkillBonusModifier object corresponding to the skillID supplied:
    SkillBonusModifier* GetSkillModifier(uint32 skillID);

protected:
    void _Populate();

    std::map<uint32, SkillBonusModifier*> m_SkillBonusModifiersMap;
};

#define sDGM_Skill_Bonus_Modifiers_Table \
( DGM_Skill_Bonus_Modifiers_Table::get() )
// -----------------------------------------------------------------------


// This class is a singleton object, containing all Effects loaded from dgmShipBonusModifiers table as memory objects of type MEffect:
class DGM_Ship_Bonus_Modifiers_Table
: public Singleton< DGM_Ship_Bonus_Modifiers_Table >
{
public:
    DGM_Ship_Bonus_Modifiers_Table();
    ~DGM_Ship_Bonus_Modifiers_Table();

    // Initializes the Table:
    int Initialize();

    // Returns pointer to ShipBonusModifier object corresponding to the shipID supplied:
    ShipBonusModifier* GetShipModifier(uint32 shipID);

protected:
    void _Populate();

    std::map<uint32, ShipBonusModifier*> m_ShipBonusModifiersMap;
};

#define sDGM_Ship_Bonus_Modifiers_Table \
( DGM_Ship_Bonus_Modifiers_Table::get() )
// -----------------------------------------------------------------------


// This class is a singleton object, containing all Modifiers loaded from dgmImplantModifiers table as memory objects of type MEffect:
class DGM_Implant_Modifiers_Table
: public Singleton< DGM_Implant_Modifiers_Table >
{
public:
    DGM_Implant_Modifiers_Table();
    ~DGM_Implant_Modifiers_Table();

    // Initializes the Table:
    int Initialize();

    // Returns pointer to ImplantModifier object corresponding to the implantID supplied:
    ImplantModifier* GetImplantModifier(uint32 implantID);

protected:
    void _Populate();

    std::map<uint32, ImplantModifier*> m_ImplantModifiersMap;
};

#define sDGM_Implant_Modifiers_Table \
( DGM_Implant_Modifiers_Table::get() )
// -----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////

#endif  // EVEMU_SYSTEM_MODIFIERS_H__