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

#include "eve-server.h"

#include "system/Modifiers.h"


// ////////////////////// SkillBonusModifier Class ////////////////////////////
SkillBonusModifier::SkillBonusModifier(uint32 skillID)
{
    m_SkillID = skillID;
    m_numOfIDs = 0;
    m_EffectIDs = nullptr;
    m_SourceAttributeIDs = nullptr;
    m_TargetAttributeIDs = nullptr;
    m_CalculationTypeIDs = nullptr;
    m_ReverseCalculationTypeIDs = nullptr;
    m_TargetChargeSizes = nullptr;
    m_AppliedPerLevelList = nullptr;
    m_AffectingTypes = nullptr;
    m_AffectedTypes = nullptr;

    m_ModifierLoaded = false;

    _Populate(skillID);
}

SkillBonusModifier::~SkillBonusModifier()
{
    if (m_numOfIDs) {
        SafeDelete(m_SourceAttributeIDs);
        SafeDelete(m_TargetAttributeIDs);
        SafeDelete(m_CalculationTypeIDs);
        SafeDelete(m_ReverseCalculationTypeIDs);
        SafeDelete(m_TargetChargeSizes);
        SafeDelete(m_AppliedPerLevelList);
        SafeDelete(m_AffectingTypes);
        SafeDelete(m_AffectedTypes);
    }
}

void SkillBonusModifier::_Populate(uint32 skillID)
{
    DBQueryResult* res = new DBQueryResult();
    ModuleDB::GetDgmSkillBonusModifiers(skillID,* res);

    DBResultRow row1;
    if ( res->GetRowCount() == 0 )
    {
        sLog.Error("SkillBonusModifier","Could not populate skill bonus modifier information for skillID: %u from the 'dgmSkillBonusModifiers' table", skillID);
        m_ModifierLoaded = false;
    }
    else
    {
        m_EffectIDs = new uint32[res->GetRowCount()];
        m_SourceAttributeIDs = new uint32[res->GetRowCount()];
        m_TargetAttributeIDs = new uint32[res->GetRowCount()];
        m_CalculationTypeIDs = new uint32[res->GetRowCount()];
        m_ReverseCalculationTypeIDs = new uint32[res->GetRowCount()];
        m_TargetChargeSizes = new uint32[res->GetRowCount()];
        m_AppliedPerLevelList = new uint32[res->GetRowCount()];
        m_AffectingTypes = new uint32[res->GetRowCount()];
        m_AffectedTypes = new uint32[res->GetRowCount()];

        int count = 0;
        std::string targetGroupIDs;
        typeTargetGroupIDlist*  TargetGroupIDs;

        while (res->GetRow(row1) )
        {
            m_EffectIDs[count] = row1.GetUInt(0);
            m_SourceAttributeIDs[count] = row1.GetUInt(1);
            m_TargetAttributeIDs[count] = row1.GetUInt(2);
            m_CalculationTypeIDs[count] = row1.GetUInt(3);
            m_Descriptions.insert(std::pair<uint32,std::string>(count,row1.GetText(4)));
            m_ReverseCalculationTypeIDs[count] = row1.GetUInt(5);
            targetGroupIDs = row1.GetText(6);
            m_TargetChargeSizes[count] = row1.GetUInt(7);
            m_AppliedPerLevelList[count] = row1.GetUInt(8);
            m_AffectingTypes[count] = row1.GetUInt(9);
            m_AffectedTypes[count] = row1.GetUInt(10);

            TargetGroupIDs = new typeTargetGroupIDlist;
            if ( !(targetGroupIDs.empty()) )
            {
                // targetGroupIDs string is not empty, so extract one number at a time until it is empty
                int pos = 0;
                std::string tempString = "";

                pos = targetGroupIDs.find_first_of(';');
                if ( pos < 0 )
                    pos = targetGroupIDs.length()-1;    // we did not find any ';' characters, so targetGroupIDs contains only one number
                tempString = targetGroupIDs.substr(0,pos);

                while ((pos = targetGroupIDs.find_first_of(';')) > 0 )
                {
                    tempString = targetGroupIDs.substr(0,pos);
                    TargetGroupIDs->insert(TargetGroupIDs->begin(), (atoi(tempString.c_str())));
                    targetGroupIDs = targetGroupIDs.substr(pos+1,targetGroupIDs.length()-1);
                }

                // Get final number now that there are no more separators to find:
                if ( !(targetGroupIDs.empty()) )
                    TargetGroupIDs->insert(TargetGroupIDs->begin(), (atoi(targetGroupIDs.c_str())));

                m_TargetGroupIDlists.insert(std::pair<uint32, typeTargetGroupIDlist* >(count, TargetGroupIDs));
            }

            ++count;
        }

        if ( count == 0 )
        {
            ;//sLog.Error("SkillBonusModifier","Could not populate bonus modifier information for skillID: %u from the 'dgmSkillBonusModifiers' table as the SQL query returned ZERO rows", skillID);
            m_ModifierLoaded = false;
        }
        else
        {
            m_numOfIDs = count;
            m_ModifierLoaded = true;
        }
    }

    //cleanup
    SafeDelete(res);
}


// ////////////////////// ShipBonusModifier Class ////////////////////////////
ShipBonusModifier::ShipBonusModifier(uint32 shipID)
{
    m_ShipID = shipID;
    m_numOfIDs = 0;
    m_EffectIDs = nullptr;
    m_AffectedTypes = nullptr;
    m_AffectingTypes = nullptr;
    m_AttributeSkillIDs = nullptr;
    m_SourceAttributeIDs = nullptr;
    m_TargetAttributeIDs = nullptr;
    m_CalculationTypeIDs = nullptr;
    m_AppliedPerLevelList = nullptr;
    m_ReverseCalculationTypeIDs = nullptr;

    m_ModifierLoaded = false;

    _Populate(shipID);
}

ShipBonusModifier::~ShipBonusModifier() {
    if (m_numOfIDs) {
        SafeDelete(m_AttributeSkillIDs);
        SafeDelete(m_SourceAttributeIDs);
        SafeDelete(m_TargetAttributeIDs);
        SafeDelete(m_CalculationTypeIDs);
        SafeDelete(m_ReverseCalculationTypeIDs);
        SafeDelete(m_AppliedPerLevelList);
        SafeDelete(m_AffectingTypes);
        SafeDelete(m_AffectedTypes);
    }
}

void ShipBonusModifier::_Populate(uint32 shipID)
{
    DBQueryResult* res = new DBQueryResult();
    ModuleDB::GetDgmShipBonusModifiers(shipID, *res);

    DBResultRow row1;
    if ( res->GetRowCount() == 0 )
    {
        sLog.Error("ShipBonusModifier","Could not populate ship bonus modifier information for shipID: %u from the 'dgmShipBonusModifiers' table", shipID);
        m_ModifierLoaded = false;
    }
    else
    {
        m_EffectIDs = new uint32[res->GetRowCount()];
        m_AttributeSkillIDs = new uint32[res->GetRowCount()];
        m_SourceAttributeIDs = new uint32[res->GetRowCount()];
        m_TargetAttributeIDs = new uint32[res->GetRowCount()];
        m_CalculationTypeIDs = new uint32[res->GetRowCount()];
        m_ReverseCalculationTypeIDs = new uint32[res->GetRowCount()];
        m_AppliedPerLevelList = new uint32[res->GetRowCount()];
        m_AffectingTypes = new uint32[res->GetRowCount()];
        m_AffectedTypes = new uint32[res->GetRowCount()];

        int count = 0;
        std::string targetGroupIDs;
        typeTargetGroupIDlist*  TargetGroupIDs;

        while (res->GetRow(row1) )
        {
            m_EffectIDs[count] = row1.GetUInt(0);
            m_AttributeSkillIDs[count] = row1.GetUInt(1);
            m_SourceAttributeIDs[count] = row1.GetUInt(2);
            m_TargetAttributeIDs[count] = row1.GetUInt(3);
            m_CalculationTypeIDs[count] = row1.GetUInt(4);
            m_Descriptions.insert(std::pair<uint32,std::string>(count,row1.GetText(5)));
            m_ReverseCalculationTypeIDs[count] = row1.GetUInt(6);
            targetGroupIDs = row1.GetText(7);
            m_AppliedPerLevelList[count] = row1.GetUInt(8);
            m_AffectingTypes[count] = row1.GetUInt(9);
            m_AffectedTypes[count] = row1.GetUInt(10);

            TargetGroupIDs = new typeTargetGroupIDlist;
            if ( !(targetGroupIDs.empty()) )
            {
                // targetGroupIDs string is not empty, so extract one number at a time until it is empty
                int pos = 0;
                std::string tempString = "";

                pos = targetGroupIDs.find_first_of(';');
                if ( pos < 0 )
                    pos = targetGroupIDs.length()-1;    // we did not find any ';' characters, so targetGroupIDs contains only one number
                tempString = targetGroupIDs.substr(0,pos);

                while ((pos = targetGroupIDs.find_first_of(';')) > 0 )
                {
                    tempString = targetGroupIDs.substr(0,pos);
                    TargetGroupIDs->insert(TargetGroupIDs->begin(), (atoi(tempString.c_str())));
                    targetGroupIDs = targetGroupIDs.substr(pos+1,targetGroupIDs.length()-1);
                }

                // Get final number now that there are no more separators to find:
                if ( !(targetGroupIDs.empty()) )
                    TargetGroupIDs->insert(TargetGroupIDs->begin(), (atoi(targetGroupIDs.c_str())));

                m_TargetGroupIDlists.insert(std::pair<uint32, typeTargetGroupIDlist* >(count, TargetGroupIDs));
            }

            ++count;
        }

        if (!count) {
            ;//sLog.Error("ShipBonusModifier","Could not populate bonus modifier information for shipID: %u from the 'dgmShipBonusModifiers' table as the SQL query returned ZERO rows", shipID);
            m_ModifierLoaded = false;
        } else {
            m_numOfIDs = count;
            m_ModifierLoaded = true;
        }
    }

    //cleanup
    SafeDelete(res);
}

// ////////////////////// ImplantModifier Class ////////////////////////////
ImplantModifier::ImplantModifier(uint32 implantID)
{
    m_implantID = implantID;
    m_numOfIDs = 0;
    m_EffectIDs = nullptr;
    m_AffectedTypes = nullptr;
    m_AffectingTypes = nullptr;
    m_AttributeSkillIDs = nullptr;
    m_SourceAttributeIDs = nullptr;
    m_TargetAttributeIDs = nullptr;
    m_CalculationTypeIDs = nullptr;
    m_AppliedPerLevelList = nullptr;
    m_ReverseCalculationTypeIDs = nullptr;

    m_ModifierLoaded = false;

    _Populate(implantID);
}

ImplantModifier::~ImplantModifier() {
    if (m_numOfIDs) {
        SafeDelete(m_AttributeSkillIDs);
        SafeDelete(m_SourceAttributeIDs);
        SafeDelete(m_TargetAttributeIDs);
        SafeDelete(m_CalculationTypeIDs);
        SafeDelete(m_ReverseCalculationTypeIDs);
        SafeDelete(m_AppliedPerLevelList);
        SafeDelete(m_AffectingTypes);
        SafeDelete(m_AffectedTypes);
    }
}

void ImplantModifier::_Populate(uint32 implantID)
{
    DBQueryResult* res = new DBQueryResult();
    ModuleDB::GetDgmImplantModifiers(implantID, *res);

    DBResultRow row1;
    if ( res->GetRowCount() == 0 )
    {
        sLog.Error("ImplantModifier","Could not populate ship bonus modifier information for implantID: %u from the 'dgmImplantModifiers' table", implantID);
        m_ModifierLoaded = false;
    }
    else
    {
        m_EffectIDs = new uint32[res->GetRowCount()];
        m_AttributeSkillIDs = new uint32[res->GetRowCount()];
        m_SourceAttributeIDs = new uint32[res->GetRowCount()];
        m_TargetAttributeIDs = new uint32[res->GetRowCount()];
        m_CalculationTypeIDs = new uint32[res->GetRowCount()];
        m_ReverseCalculationTypeIDs = new uint32[res->GetRowCount()];
        m_AppliedPerLevelList = new uint32[res->GetRowCount()];
        m_AffectingTypes = new uint32[res->GetRowCount()];
        m_AffectedTypes = new uint32[res->GetRowCount()];

        int count = 0;
        std::string targetGroupIDs;
        typeTargetGroupIDlist*  TargetGroupIDs;

        while (res->GetRow(row1) )
        {
            m_EffectIDs[count] = row1.GetUInt(0);
            m_AttributeSkillIDs[count] = row1.GetUInt(1);
            m_SourceAttributeIDs[count] = row1.GetUInt(2);
            m_TargetAttributeIDs[count] = row1.GetUInt(3);
            m_CalculationTypeIDs[count] = row1.GetUInt(4);
            m_Descriptions.insert(std::pair<uint32,std::string>(count,row1.GetText(5)));
            m_ReverseCalculationTypeIDs[count] = row1.GetUInt(6);
            targetGroupIDs = row1.GetText(7);
            m_AppliedPerLevelList[count] = row1.GetUInt(8);
            m_AffectingTypes[count] = row1.GetUInt(9);
            m_AffectedTypes[count] = row1.GetUInt(10);

            TargetGroupIDs = new typeTargetGroupIDlist;
            if ( !(targetGroupIDs.empty()) )
            {
                // targetGroupIDs string is not empty, so extract one number at a time until it is empty
                int pos = 0;
                std::string tempString = "";

                pos = targetGroupIDs.find_first_of(';');
                if ( pos < 0 )
                    pos = targetGroupIDs.length()-1;    // we did not find any ';' characters, so targetGroupIDs contains only one number
                    tempString = targetGroupIDs.substr(0,pos);

                while ((pos = targetGroupIDs.find_first_of(';')) > 0 )
                {
                    tempString = targetGroupIDs.substr(0,pos);
                    TargetGroupIDs->insert(TargetGroupIDs->begin(), (atoi(tempString.c_str())));
                    targetGroupIDs = targetGroupIDs.substr(pos+1,targetGroupIDs.length()-1);
                }

                // Get final number now that there are no more separators to find:
                if ( !(targetGroupIDs.empty()) )
                    TargetGroupIDs->insert(TargetGroupIDs->begin(), (atoi(targetGroupIDs.c_str())));

                m_TargetGroupIDlists.insert(std::pair<uint32, typeTargetGroupIDlist* >(count, TargetGroupIDs));
            }

            ++count;
        }

        if (!count) {
            ;//sLog.Error("ImplantModifier","Could not populate bonus modifier information for shipID: %u from the 'dgmImplantModifiers' table as the SQL query returned ZERO rows", shipID);
            m_ModifierLoaded = false;
        } else {
            m_numOfIDs = count;
            m_ModifierLoaded = true;
        }
    }

    //cleanup
    SafeDelete(res);
}

// ////////////////////// DGM_Skill_Bonus_Modifiers_Table Class ////////////////////////////
DGM_Skill_Bonus_Modifiers_Table::DGM_Skill_Bonus_Modifiers_Table()
{
}

DGM_Skill_Bonus_Modifiers_Table::~DGM_Skill_Bonus_Modifiers_Table()
{
    // TODO: loop through entire std::map<> and delete ALL entries, calling ~SkillBonusModifier() on each
    //for (auto cur : m_SkillBonusModifiersMap)
    //    SafeDelete(cur.second);
    m_SkillBonusModifiersMap.clear();
}

int DGM_Skill_Bonus_Modifiers_Table::Initialize()
{
    _Populate();

    return 1;
}

void DGM_Skill_Bonus_Modifiers_Table::_Populate()
{
    double start = GetTimeMSeconds();
    //first get list of all effects from dgmSkillBonusModifiers table
    DBQueryResult* res = new DBQueryResult();
    //ModuleDB::GetAllDgmSkillBonusModifiers(*res);

    //counter
    SkillBonusModifier* mSkillBonusModifierPtr = nullptr;
    uint32 skillID = 0, total_modifier_count = 0, error_count = 0;

    //go through and populate each skill bonus modifier
    DBResultRow row;
    while (res->GetRow(row) )
    {
        skillID = row.GetInt(0);
        mSkillBonusModifierPtr = new SkillBonusModifier(skillID);
        if ( mSkillBonusModifierPtr->IsModifierLoaded() )
            m_SkillBonusModifiersMap.insert(std::pair<uint32, SkillBonusModifier* >(skillID,mSkillBonusModifierPtr));
        else
            ++error_count;

        ++total_modifier_count;
    }

    if ( error_count > 0 )
        sLog.Error("DGM_Skill_Bonus_Modifiers_Table::_Populate()","ERROR Populating the DGM_Skill_Bonus_Modifiers_Table memory object: %u of %u skill bonus modifiers failed to load!", error_count, total_modifier_count);

    sLog.Log("  Skill Modifiers", "%u skill modifier objects loaded in %.3fs", total_modifier_count, (GetTimeMSeconds() - start));

    //cleanup
    SafeDelete(res);
    SafeDelete(mSkillBonusModifierPtr);
}

SkillBonusModifier*  DGM_Skill_Bonus_Modifiers_Table::GetSkillModifier(uint32 skillID)
{
    // return SkillBonusModifier*  corresponding to skillID from m_SkillBonusModifiersMap
    std::map<uint32, SkillBonusModifier* >::iterator skillBonusModifierMapIterator;
    if ( (skillBonusModifierMapIterator = m_SkillBonusModifiersMap.find(skillID)) == m_SkillBonusModifiersMap.end() )
        return nullptr;
    else
        return skillBonusModifierMapIterator->second;
}


// ////////////////////// DGM_Skill_Bonus_Modifiers_Table Class ////////////////////////////
DGM_Ship_Bonus_Modifiers_Table::DGM_Ship_Bonus_Modifiers_Table()
{
}

DGM_Ship_Bonus_Modifiers_Table::~DGM_Ship_Bonus_Modifiers_Table()
{
    // TODO: loop through entire std::map<> and delete ALL entries, calling ~ShipBonusModifier() on each
    for (auto cur : m_ShipBonusModifiersMap)
        SafeDelete(cur.second);
    m_ShipBonusModifiersMap.clear();
}

int DGM_Ship_Bonus_Modifiers_Table::Initialize()
{
    _Populate();

    return 1;
}

void DGM_Ship_Bonus_Modifiers_Table::_Populate()
{
    double start = GetTimeMSeconds();
    //first get list of all effects from dgmShipBonusModifiers table
    DBQueryResult* res = new DBQueryResult();
    //ModuleDB::GetAllDgmShipBonusModifiers(*res);

    //counter
    ShipBonusModifier* mShipBonusModifierPtr = nullptr;
    uint32 shipID = 0, total_modifier_count = 0, error_count = 0;

    //go through and populate each ship bonus modifier
    DBResultRow row;
    while (res->GetRow(row) )
    {
        shipID = row.GetInt(0);
        mShipBonusModifierPtr = new ShipBonusModifier(shipID);
        if ( mShipBonusModifierPtr->IsModifierLoaded() )
            m_ShipBonusModifiersMap.insert(std::pair<uint32, ShipBonusModifier* >(shipID,mShipBonusModifierPtr));
        else
            ++error_count;

        ++total_modifier_count;
    }

    if ( error_count > 0 )
        sLog.Error("DGM_Ship_Bonus_Modifiers_Table::_Populate()","ERROR Populating the DGM_Ship_Bonus_Modifiers_Table memory object: %u of %u ship bonus modifiers failed to load!", error_count, total_modifier_count);

    sLog.Log("   Ship Modifiers", "%u ship modifier objects loaded in %.3fs", total_modifier_count, (GetTimeMSeconds() - start));

    //cleanup
    SafeDelete(res);
    SafeDelete(mShipBonusModifierPtr);
}

ShipBonusModifier* DGM_Ship_Bonus_Modifiers_Table::GetShipModifier(uint32 shipID)
{
    // return ShipBonusModifier*  corresponding to shipID from m_ShipBonusModifiersMap
    std::map<uint32, ShipBonusModifier* >::iterator shipBonusModifierMapIterator;
    if ( (shipBonusModifierMapIterator = m_ShipBonusModifiersMap.find(shipID)) == m_ShipBonusModifiersMap.end() )
        return nullptr;
    else
        return shipBonusModifierMapIterator->second;
}


// ////////////////////// DGM_Implant_Modifiers_Table Class ////////////////////////////
DGM_Implant_Modifiers_Table::DGM_Implant_Modifiers_Table()
{
}

DGM_Implant_Modifiers_Table::~DGM_Implant_Modifiers_Table()
{
    // TODO: loop through entire std::map<> and delete ALL entries, calling ~ImplantModifier() on each
    for (auto cur : m_ImplantModifiersMap)
        SafeDelete(cur.second);
    m_ImplantModifiersMap.clear();
}

int DGM_Implant_Modifiers_Table::Initialize()
{
    _Populate();

    return 1;
}

void DGM_Implant_Modifiers_Table::_Populate()
{
    double start = GetTimeMSeconds();
    //first get list of all effects from dgmImplantModifiers table (which doesnt exist yet)
    DBQueryResult* res = new DBQueryResult();
    //ModuleDB::GetAllDgmImplantModifiers(*res);

    //counter
    ImplantModifier* mImplantModifierPtr = nullptr;
    uint32 implantID = 0, total_modifier_count = 0, error_count = 0;

    //go through and populate each implant bonus modifier
    DBResultRow row;
    while (res->GetRow(row) )
    {
        implantID = row.GetInt(0);
        mImplantModifierPtr = new ImplantModifier(implantID);
        if ( mImplantModifierPtr->IsModifierLoaded() )
            m_ImplantModifiersMap.insert(std::pair<uint32, ImplantModifier* >(implantID,mImplantModifierPtr));
        else
            ++error_count;

        ++total_modifier_count;
    }

    if ( error_count > 0 )
        sLog.Error("DGM_Implant_Modifiers_Table::_Populate()","ERROR Populating the DGM_Implant_Modifiers_Table memory object: %u of %u implant bonus modifiers failed to load!", error_count, total_modifier_count);

    sLog.Log("Implant Modifiers", "%u implant modifier objects loaded in %.3fs", total_modifier_count, (GetTimeMSeconds() - start));

    //cleanup
    SafeDelete(res);
    SafeDelete(mImplantModifierPtr);
}

ImplantModifier* DGM_Implant_Modifiers_Table::GetImplantModifier(uint32 implantID)
{
    // return ImplantModifier*  corresponding to implantID from m_ImplantModifiersMap
    std::map<uint32, ImplantModifier* >::iterator implantBonusModifierMapIterator;
    if ( (implantBonusModifierMapIterator = m_ImplantModifiersMap.find(implantID)) == m_ImplantModifiersMap.end() )
        return nullptr;
    else
        return implantBonusModifierMapIterator->second;
}
