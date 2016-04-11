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
    Author:        Zhur
*/

#include "eve-server.h"

#include "system/SystemEntities.h"
#include "SystemBubble.h"

using namespace Destiny;

SimpleSystemEntity* SimpleSystemEntity::MakeEntity( SystemManager* system, const DBSystemEntity& entity )
{
    switch( entity.groupID )
    {
        case EVEDB::invGroups::Sun:
        case EVEDB::invGroups::Planet:
        case EVEDB::invGroups::Moon:
            return new SystemPlanetEntity( system, entity );

        //case EVEDB::invGroups::Asteroid_Belt:
        //    return new SystemBeltEntity( system, entity );

        case EVEDB::invGroups::Stargate:    //Stargate
            return new SystemStargateEntity( system, entity );

        case EVEDB::invGroups::Station:        //Station
            return new SystemStationEntity( system, entity );

        default:
            sLog.Error( "Simple sys Entity", "Unrecognized entity type '%u' on '%s' (%u), falling back to simple space item.",
                        entity.typeID, entity.itemName.c_str(), entity.itemID );
            return new SystemSimpleEntity( system, entity );
    }
}

SimpleSystemEntity::SimpleSystemEntity( SystemManager* system, const DBSystemEntity& entity )
: InanimateSystemEntity(system), data(entity)
{
}

bool SimpleSystemEntity::LoadExtras(SystemDB *db) {
    return true;
}

PyDict *SimpleSystemEntity::MakeSlimItem() const {
    _log(COMMON__WARNING, "MakeSlimItem for SimpleSystemEntity %s(%u)", data.itemName.c_str(), data.itemID);
    PyDict *slim = new PyDict();
    slim->SetItemString("typeID",       new PyInt(data.typeID));
    slim->SetItemString("ownerID",      new PyInt(1));
    slim->SetItemString("itemID",       new PyLong(data.itemID));
    //slim->SetItemString("categoryID",   new PyInt(data.categoryID()));
    return slim;
}

void SimpleSystemEntity::MakeDamageState(DoDestinyDamageState &into) const {
    into.shield = 1;
    into.recharge = 110000;
    into.armor = 1;
    into.structure = 1;
    into.timestamp = Win32TimeNow();
}

const GVector &SimpleSystemEntity::GetVelocity() const {
    static const GVector err(0.0, 0.0, 0.0);
    return(err);
}

void InanimateSystemEntity::MakeDamageState(DoDestinyDamageState &into) const {
    into.shield = 1;
    into.recharge = 120000;
    into.armor = 1;
    into.structure = 1;
    into.timestamp = Win32TimeNow();
}

SystemPlanetEntity::SystemPlanetEntity(SystemManager *system, const DBSystemEntity &entity) : SimpleSystemEntity(system, entity) {}

void SystemPlanetEntity::EncodeDestiny( Buffer& into ) const
{
    BallHeader head;
    head.entityID = data.itemID;
    head.mode = DSTBALL_RIGID;
    head.radius = data.radius;
    head.x = data.position.x;
    head.y = data.position.y;
    head.z = data.position.z;
    head.flags = IsGlobal;
    into.Append( head );

    DSTBALL_RIGID_Struct main;
    main.formationID = 0xFF;
    into.Append( main );
    _log(COMMON__WARNING, "SystemPlanetEntity::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

SystemStationEntity::SystemStationEntity(SystemManager *system, const DBSystemEntity &entity)
: SimpleSystemEntity(system, entity) {
}

void SystemStationEntity::EncodeDestiny( Buffer& into ) const
{
    BallHeader head;
    head.entityID = data.itemID;
    head.mode = DSTBALL_RIGID;
    head.radius = data.radius;
    head.x = data.position.x;
    head.y = data.position.y;
    head.z = data.position.z;
    head.flags = HasMiniBalls | IsGlobal;
    into.Append( head );

    DSTBALL_RIGID_Struct main;
    main.formationID = 0xFF;
    into.Append( main );

    const uint16 miniballsCount = 1;
    into.Append( miniballsCount );

    MiniBall miniball;
    miniball.x = -7701.181;
    miniball.y = 8060.06;
    miniball.z = 27878.900;
    miniball.radius = 1639.241;
    into.Append( miniball );
    _log(COMMON__WARNING, "SystemStationEntity::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

PyDict *SystemStationEntity::MakeSlimItem() const {
    _log(COMMON__WARNING, "MakeSlimItem for SystemStationEntity %s(%u)", data.itemName.c_str(), data.itemID);
    PyDict *slim = new PyDict();
    slim->SetItemString("typeID",       new PyInt(data.typeID));
    slim->SetItemString("ownerID",      new PyInt(1));
    slim->SetItemString("itemID",       new PyLong(data.itemID));
    //slim->SetItemString("categoryID",   new PyInt(data.categoryID()));
    return(slim);
}


SystemStargateEntity::SystemStargateEntity(SystemManager *system, const DBSystemEntity &entity)
: SystemStationEntity(system, entity),
  m_jumps(NULL)
{
}

SystemStargateEntity::~SystemStargateEntity() {
    TargMgr.DoDestruction();
    PySafeDecRef( m_jumps );
}

bool SystemStargateEntity::LoadExtras(SystemDB *db) {
    if(!SystemStationEntity::LoadExtras(db))
        return false;

    Bubble()->SetGate(true);
    _log(DESTINY__BUBBLE_DEBUG, "SystemStargateEntity::LoadExtras() - IsGate set to true for bubble %u.", Bubble()->GetID() );
    m_jumps = db->ListJumps(GetID());
    if (m_jumps)
        return true;

    return false;
}

PyDict *SystemStargateEntity::MakeSlimItem() const {
    _log(COMMON__WARNING, "MakeSlimItem for SystemStargateEntity %s(%u)", data.itemName.c_str(), data.itemID);
    PyDict *slim = new PyDict();
    slim->SetItemString("typeID",   new PyInt(data.typeID));
    slim->SetItemString("ownerID",  new PyInt(1));       // TODO make function to lookup controlling faction id for this
    slim->SetItemString("itemID",   new PyLong(data.itemID));
    if(m_jumps != NULL)
        slim->SetItemString("jumps", m_jumps->Clone());
    return(slim);
}

