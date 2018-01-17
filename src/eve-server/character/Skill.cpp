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
Updates:    Allan
*/

#include "eve-server.h"

#include "character/Character.h"
#include "character/Skill.h"
#include "inventory/AttributeEnum.h"


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
    uint32 skillID = CreateItemID(data );
    if ( skillID == 0 )
        return SkillRef();

    SkillRef skillRef = Skill::Load(skillID );
    if (skillRef.get() == nullptr) {
        // make error msg here for failure to load skill?
        return SkillRef();
    }

    skillRef->SaveItem();
    return skillRef;
}

uint32 Skill::CreateItemID( ItemData &data)
{
    return InventoryItem::CreateItemID(data );
}

EvilNumber Skill::GetSPForLevel( EvilNumber level ) {
    return EvEMath::Skill::PointsAtLevel(level, GetAttribute(AttrSkillTimeConstant));
}

void Skill::VerifySP()
{
    EvilNumber spThisLevel = EvEMath::Skill::PointsAtLevel(GetAttribute(AttrSkillLevel), GetAttribute(AttrSkillTimeConstant));
    EvilNumber spNextLevel = EvEMath::Skill::PointsAtLevel(GetAttribute(AttrSkillLevel) +1, GetAttribute(AttrSkillTimeConstant));
    if ((GetAttribute(AttrSkillPoints) < spThisLevel) or (GetAttribute(AttrSkillPoints) > spNextLevel)) {
        _log(CHARACTER__SKILL_TRACE, "Updating Skill %s from %.6f to %.6f", itemName().c_str(), GetAttribute(AttrSkillPoints).get_float(), (spThisLevel.get_float() + 0.0001));
        SetAttribute(AttrSkillPoints, spThisLevel);
    }
}

bool Skill::SkillPrereqsComplete(Character &ch) {
    bool test = true;
    EvilNumber skillID = 0;
    if (HasAttribute(AttrRequiredSkill1, skillID)) {
        if (GetAttribute(AttrRequiredSkill1Level) > ch.GetSkillLevel(skillID.get_int()))
            test = false;
        if (HasAttribute(AttrRequiredSkill2, skillID)) {
            if (GetAttribute(AttrRequiredSkill2Level) > ch.GetSkillLevel(skillID.get_int()))
                test = false;
            if (HasAttribute(AttrRequiredSkill3, skillID)) {
                if (GetAttribute(AttrRequiredSkill3Level) > ch.GetSkillLevel(skillID.get_int()))
                    test = false;
                if (HasAttribute(AttrRequiredSkill4, skillID)) {
                    if (GetAttribute(AttrRequiredSkill4Level) > ch.GetSkillLevel(skillID.get_int()))
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
        if ( iRef->GetAttribute(AttrRequiredSkill1Level) > cRef->GetSkillLevel(skillID.get_int()))
            test = false;
        if (iRef->HasAttribute(AttrRequiredSkill2, skillID)) {//Secondary Skill
            if ( iRef->GetAttribute(AttrRequiredSkill2Level) > cRef->GetSkillLevel(skillID.get_int()))
                test = false;
            if (iRef->HasAttribute(AttrRequiredSkill3, skillID)) {//Tertiary Skill
                if ( iRef->GetAttribute(AttrRequiredSkill3Level) > cRef->GetSkillLevel(skillID.get_int()))
                    test = false;
                if (iRef->HasAttribute(AttrRequiredSkill4, skillID)) {//Quarternary Skill
                    if ( iRef->GetAttribute(AttrRequiredSkill4Level) > cRef->GetSkillLevel(skillID.get_int()))
                        test = false;
                    if (iRef->HasAttribute(AttrRequiredSkill5, skillID)) {//Quinary Skill
                        if ( iRef->GetAttribute(AttrRequiredSkill5Level) > cRef->GetSkillLevel(skillID.get_int()))
                            test = false;
                        if (iRef->HasAttribute(AttrRequiredSkill6, skillID)) {//Senary Skill
                            if ( iRef->GetAttribute(AttrRequiredSkill6Level) > cRef->GetSkillLevel(skillID.get_int()))
                                test = false;
                        }
                    }
                }
            }
        }
    }

    return test;
}