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

/*
* Skill
*/
Skill::Skill(
    ItemFactory &_factory,
    uint32 _skillID,
    // InventoryItem stuff:
    const ItemType &_type,
    const ItemData &_data )
: InventoryItem(_factory, _skillID, _type, _data)
{
}

SkillRef Skill::Load(ItemFactory &factory, uint32 skillID)
{
    return InventoryItem::Load<Skill>( factory, skillID );
}

SkillRef Skill::Spawn(ItemFactory &factory, ItemData &data)
{
    uint32 skillID = CreateItemID( factory, data );
    if( skillID == 0 )
        return SkillRef();

    SkillRef skillRef = Skill::Load( factory, skillID );
    skillRef->SaveItem();
    return skillRef;
}

uint32 Skill::CreateItemID(ItemFactory &factory, ItemData &data)
{
    return InventoryItem::CreateItemID( factory, data );
}

EvilNumber Skill::GetSPForLevel( EvilNumber level ) {
    return (EVIL_SKILL_BASE_POINTS * GetAttribute(AttrSkillTimeConstant) * EvilNumber::pow(2, (2.5*(level - 1))));
}

bool Skill::SkillPrereqsComplete(Character &ch) {
    EvilNumber skillID;
    if (HasAttribute(AttrRequiredSkill1, skillID))
        if ( GetAttribute(AttrRequiredSkill1Level) > ch.GetSkillLevel(skillID.get_int()))
            return false;
    if (HasAttribute(AttrRequiredSkill2, skillID))
        if ( GetAttribute(AttrRequiredSkill2Level) > ch.GetSkillLevel(skillID.get_int()))
            return false;
    if (HasAttribute(AttrRequiredSkill3, skillID))
        if ( GetAttribute(AttrRequiredSkill3Level) > ch.GetSkillLevel(skillID.get_int()))
            return false;
    if (HasAttribute(AttrRequiredSkill4, skillID))
        if ( GetAttribute(AttrRequiredSkill4Level) > ch.GetSkillLevel(skillID.get_int()))
            return false;
    return true;
}

bool Skill::FitModuleSkillCheck(InventoryItemRef item, CharacterRef cRef) {
    EvilNumber skillID;
    if (item->HasAttribute(AttrRequiredSkill1, skillID)) //Primary Skill
        if ( item->GetAttribute(AttrRequiredSkill1Level) > cRef->GetSkillLevel(skillID.get_int()))
            return false;
    if (item->HasAttribute(AttrRequiredSkill2, skillID)) //Secondary Skill
        if ( item->GetAttribute(AttrRequiredSkill2Level) > cRef->GetSkillLevel(skillID.get_int()))
            return false;
    if (item->HasAttribute(AttrRequiredSkill3, skillID)) //Tertiary Skill
        if ( item->GetAttribute(AttrRequiredSkill3Level) > cRef->GetSkillLevel(skillID.get_int()))
            return false;
    if (item->HasAttribute(AttrRequiredSkill4, skillID)) //Quarternary Skill
        if ( item->GetAttribute(AttrRequiredSkill4Level) > cRef->GetSkillLevel(skillID.get_int()))
            return false;
    if (item->HasAttribute(AttrRequiredSkill5, skillID)) //Quinary Skill
        if ( item->GetAttribute(AttrRequiredSkill5Level) > cRef->GetSkillLevel(skillID.get_int()))
            return false;
    if (item->HasAttribute(AttrRequiredSkill6, skillID)) //Senary Skill
        if ( item->GetAttribute(AttrRequiredSkill6Level) > cRef->GetSkillLevel(skillID.get_int()))
            return false;
    return true;
}