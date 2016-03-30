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
    Author:     Aknor Jaden
*/

#include "eve-server.h"

#include "inventory/AttributeEnum.h"
#include "ship/DestinyManager.h"
#include "system/Deployable.h"

using namespace Destiny;

DeployableEntity::DeployableEntity(
    InventoryItemRef deployable,
    SystemManager *system,
    PyServiceMgr &services,
    const GPoint &position)
: DynamicSystemEntity(new DestinyManager(this, system), deployable),
  m_system(system),
  m_services(services)
{
    _deployableRef = deployable;
    m_destiny->SetPosition(position, false);
}

void DeployableEntity::Process() {
    //SystemEntity::Process();
}

void DeployableEntity::ForcedSetPosition(const GPoint &pt) {
    m_destiny->SetPosition(pt, false);
}

void DeployableEntity::EncodeDestiny( Buffer& into ) const
{
    const GPoint& position = GetPosition();

    BallHeader head;
    head.entityID = GetID();
    head.mode = DSTBALL_RIGID;
    head.radius = GetRadius();
    head.x = position.x;
    head.y = position.y;
    head.z = position.z;
    head.flags = IsMassive | IsInteractive;
    into.Append( head );

    DSTBALL_RIGID_Struct main;
    main.formationID = 0xFF;
    into.Append( main );

    _log(COMMON__WARNING, "DeployableEntity::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void DeployableEntity::MakeDamageState(DoDestinyDamageState &into) const
{
    into.shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float() +4;
    into.timestamp = Win32TimeNow();
    into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float());
}

