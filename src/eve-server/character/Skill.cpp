/*
------------------------------------------------------------------------------------
LICENSE:
------------------------------------------------------------------------------------
This file is part of EVEmu: EVE Online Server Emulator
Copyright 2006 - 2011 The EVEmu Team
For the latest information visit http://evemu.org
------------------------------------------------------------------------------------
This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
------------------------------------------------------------------------------------
Author: Bloody.Rabbit
Rewrite:    Allan
*/

#include "eve-server.h"

#include "character/Character.h"
#include "character/Skill.h"
#include "inventory/AttributeEnum.h"

/*
 * SKILL__ERROR
 * SKILL__WARNING
 * SKILL__MESSAGE
 * SKILL__INFO
 * SKILL__DEBUG
 * SKILL__TRACE
 */

Skill::Skill(uint32 _skillID, const ItemType& _type, const ItemData& _data)
: InventoryItem(_skillID, _type, _data)
{
}

SkillRef Skill::Load( uint32 skillID)
{
    return InventoryItem::Load<Skill>(skillID );
}

SkillRef Skill::Spawn( ItemData &data)
{
    uint32 skillID = InventoryItem::CreateItemID(data );
    if ( skillID == 0 )
        return SkillRef(nullptr);

    SkillRef skillRef = Skill::Load(skillID );
    if (skillRef.get() == nullptr) {
        // make error msg here for failure to load skill?
        return SkillRef(nullptr);
    }

    skillRef->SaveItem();
    return skillRef;
}

uint32 Skill::GetSPForLevel(uint8 level) {
    return EvEMath::Skill::PointsAtLevel(level, GetAttribute(AttrSkillTimeConstant).get_uint32());
}

uint32 Skill::GetCurrentSP(Character* ch)
{
    int64 expiryTime = GetAttribute(AttrExpiryTime).get_int();
    if (expiryTime == 0)
        return GetAttribute(AttrSkillPoints).get_uint32();

    uint8 level = GetAttribute(AttrSkillLevel).get_uint32() +1;
    uint32 spNextLevel = GetSPForLevel(level);
    uint32 currentSP = 0;
    float timeLeft = (GetAttribute(AttrExpiryTime).get_int() - GetFileTimeNow()) / EvE::Time::Second;
    if (timeLeft > 0) {
        timeLeft /= 60;     // more accurate to get minutes here...get fraction of minutes also, where x/minute (above) didnt
        currentSP = spNextLevel - (timeLeft * ch->GetSPPerMin(this));
    } else {
        // training is complete, so set points for next level
        currentSP = spNextLevel;
    }
    _log(SKILL__TRACE, "Skill::GetCurrentSP() for %s is %u - remaining time for level %u: %0.2fm", itemName().c_str(), currentSP, level, timeLeft);

    return currentSP;
}


void Skill::VerifySP()
{
    if (is_log_enabled(SKILL__INFO))
        _log(SKILL__INFO, "Begin SP check for %s. level %u: CurrentSP: %u", \
                itemName().c_str(), GetAttribute(AttrSkillLevel).get_uint32(), GetAttribute(AttrSkillPoints).get_uint32());

    if (GetAttribute(AttrSkillPoints) == EvilZero)
        return;
    uint8 level = GetAttribute(AttrSkillLevel).get_uint32() +1;
    if (level > 5)
        return;

    uint32 spThisLevel = GetSPForLevel(level -1);
    uint32 spNextLevel = GetSPForLevel(level);
    uint32 spCurrent = GetAttribute(AttrSkillPoints).get_uint32();
    if (spCurrent < spThisLevel) {
        _log(SKILL__WARNING, "Skill %s points low.  Updating from %u to %u (next: %u)", itemName().c_str(), spCurrent, spThisLevel, spNextLevel);
        SetAttribute(AttrSkillPoints, spThisLevel);
        SetAttribute(AttrExpiryTime, EvilZero);
        SetAttribute(AttrSkillStartTime, EvilZero);
    }
    if (spCurrent > spNextLevel) {
        SetAttribute(AttrSkillLevel, level);
        if (level > 4) {
            _log(SKILL__WARNING, " %s - Skillpoints high for L5. Updating SP from %u to %u.", \
                itemName().c_str(), spCurrent, spNextLevel);
            SetAttribute(AttrSkillPoints, spNextLevel);
            DeleteAttribute(AttrExpiryTime);
            DeleteAttribute(AttrSkillStartTime);
            return;
        } else
            _log(SKILL__WARNING, " %s - Skillpoints high. Updating level from %u to %u.", \
                itemName().c_str(), level -1, level);
        //SetAttribute(AttrSkillPoints, spThisLevel, false);
        SetAttribute(AttrExpiryTime, EvilZero);
        SetAttribute(AttrSkillStartTime, EvilZero);
        VerifySP();
    }
}

void Skill::VerifyAttribs()
{
    if (!m_singleton)
        ChangeSingleton(true, true);
    if (GetAttribute(AttrSkillLevel).get_type() != evil_number_int) {
        _log(SKILL__INFO, "Skill %s level type != int.  Fixing...", itemName().c_str());
        SetAttribute(AttrSkillLevel, GetAttribute(AttrSkillLevel).get_uint32(), false);
    }
    if (GetAttribute(AttrSkillPoints).get_type() != evil_number_int) {
        _log(SKILL__INFO, "Skill %s sp type != int.  Fixing...", itemName().c_str());
        SetAttribute(AttrSkillPoints, GetAttribute(AttrSkillPoints).get_uint32());
    }
    // is this needed?
    //if (m_flag != flagSkillInTraining)
    //    SetAttribute(AttrExpiryTime, EvilZerof);
}

bool Skill::SkillPrereqsComplete(Character &ch) {
    bool test = true;
    EvilNumber skillID = 0;
    if (HasAttribute(AttrRequiredSkill1, skillID)) {
        if (GetAttribute(AttrRequiredSkill1Level) > ch.GetSkillLevel(skillID.get_uint32()))
            test = false;
        if (HasAttribute(AttrRequiredSkill2, skillID)) {
            if (GetAttribute(AttrRequiredSkill2Level) > ch.GetSkillLevel(skillID.get_uint32()))
                test = false;
            if (HasAttribute(AttrRequiredSkill3, skillID)) {
                if (GetAttribute(AttrRequiredSkill3Level) > ch.GetSkillLevel(skillID.get_uint32()))
                    test = false;
                if (HasAttribute(AttrRequiredSkill4, skillID)) {
                    if (GetAttribute(AttrRequiredSkill4Level) > ch.GetSkillLevel(skillID.get_uint32()))
                        test = false;
                }
            }
        }
    }

    return test;
}

bool Skill::FitModuleSkillCheck(InventoryItemRef iRef, CharacterRef cRef) {
    bool test = true;
    EvilNumber skillID = 0;
    if (iRef->HasAttribute(AttrRequiredSkill1, skillID)) {//Primary Skill
        if ( iRef->GetAttribute(AttrRequiredSkill1Level) > cRef->GetSkillLevel(skillID.get_uint32()))
            test = false;
        if (iRef->HasAttribute(AttrRequiredSkill2, skillID)) {//Secondary Skill
            if ( iRef->GetAttribute(AttrRequiredSkill2Level) > cRef->GetSkillLevel(skillID.get_uint32()))
                test = false;
            if (iRef->HasAttribute(AttrRequiredSkill3, skillID)) {//Tertiary Skill
                if ( iRef->GetAttribute(AttrRequiredSkill3Level) > cRef->GetSkillLevel(skillID.get_uint32()))
                    test = false;
                if (iRef->HasAttribute(AttrRequiredSkill4, skillID)) {//Quarternary Skill
                    if ( iRef->GetAttribute(AttrRequiredSkill4Level) > cRef->GetSkillLevel(skillID.get_uint32()))
                        test = false;
                    if (iRef->HasAttribute(AttrRequiredSkill5, skillID)) {//Quinary Skill
                        if ( iRef->GetAttribute(AttrRequiredSkill5Level) > cRef->GetSkillLevel(skillID.get_uint32()))
                            test = false;
                        if (iRef->HasAttribute(AttrRequiredSkill6, skillID)) {//Senary Skill
                            if ( iRef->GetAttribute(AttrRequiredSkill6Level) > cRef->GetSkillLevel(skillID.get_uint32()))
                                test = false;
                        }
                    }
                }
            }
        }
    }

    return test;
}