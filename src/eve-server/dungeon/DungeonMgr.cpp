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
    Author:        Allan
*/

#include "eve-server.h"

#include "dungeon/DungeonMgr.h"

SystemDungeonEntity::SystemDungeonEntity(SystemManager *system, InventoryItemRef self)
: ItemSystemEntity(self),
  m_system(system)
{
    //will use effects.WarpGateEffect
    /*TODO  CODERS: upon entering the location of the dungeon for the first time (the one being entered)
     * the DB should be called to spawn the following in this table.
     * Just at the location being entered. not for all and only for the first time (but would reset after server restart/shutdown).
        script is a place-holder in the DB to refer to any particular activity the spawn may do on initial player entry
            (move to x,y,z RELATIVE to the location or attack target etc etc)
        location is relative to where the player warps in (which would be 0,0,0 unless scripted elsewhere)
     there needs to be something linking spawns to their location (dungeonspawnedID) once they have been spawned
        so they can be removed later. I thought this may have been an entry in entityattributes but I don't think there is a value for that..
     'spawn' is there because there are multiple typeids that are the same in many parts of a complex.
    */
}

//this is a big hack just to document the kind of stuff a dungeon conveys.
PyDict *SystemDungeonEntity::MakeSlimItem() const {
    _log(COMMON__WARNING, "MakeSlimItem for SystemDungeonEntity %u", Item()->itemID());

    PyDict *slim = new PyDict();

    slim->SetItemString("itemID", new PyLong(Item()->itemID()));
    slim->SetItemString("typeID", new PyInt(12273));
    slim->SetItemString("ownerID", new PyInt(1));

    slim->SetItemString("dunSkillLevel", new PyInt(0));
    slim->SetItemString("dunSkillTypeID", new PyNone);
    slim->SetItemString("dunObjectID", new PyInt(160449));
    slim->SetItemString("dunWipeNPC", new PyInt(1));
    slim->SetItemString("dunToGateID", new PyInt(160484));
    slim->SetItemString("dunCloaked", new PyInt(0));
    slim->SetItemString("dunScenarioID", new PyInt(23));
    slim->SetItemString("dunSpawnID", new PyInt(4));
    slim->SetItemString("dunAmount", new PyFloat(0.0));
    slim->SetItemString("dunShipClasses", new PyList(/*237, 31*/));
    slim->SetItemString("dunDirection", new PyList(/*235, 0, 1*/));
    slim->SetItemString("dunKeyLock", new PyInt(0));
    //slim->SetItemString("dunKeyQuantity", new PyInt(1));
    //slim->SetItemString("dunKeyTypeID", new PyInt(21839));
    //slim->SetItemString("dunOpenUntil", new PyInt(Win32TimeNow()+Win32Time_Hour));
    slim->SetItemString("dunMusicUrl", new PyString("res:/Sound/Music/Ambient031combat.ogg"));

    return(slim);
}

void SystemDungeonEntity::EncodeDestiny( Buffer& into ) const
{
    using namespace Destiny;

    const GPoint& position = m_self->position();
    const std::string itemName( GetName() );

    BallHeader head;
    head.entityID = m_self->itemID();
    head.mode = DSTBALL_RIGID;
    head.radius = GetRadius();
    head.x = position.x;
    head.y = position.y;
    head.z = position.z;
    head.flags = IsInteractive;
    into.Append( head );

    MassSector mass;
    mass.mass = 10000000000.0;
    mass.cloak = 0;
    mass.Harmonic = 0.0f;
    mass.corporationID = m_self->ownerID();    //a little hacky...
    mass.allianceID = 0;
    into.Append( mass );

    DSTBALL_STOP_Struct main;
    main.formationID = 0xFF;
    into.Append( main );

    const uint16 miniballCount = 1;
    into.Append( miniballCount );

    MiniBall miniball;
    miniball.x = -7701.181;
    miniball.y = 8060.06;
    miniball.z = 27878.900;
    miniball.radius = 1639.241;
    into.Append( miniball );
    _log(COMMON__WARNING, "SystemDungeonEntity::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}
