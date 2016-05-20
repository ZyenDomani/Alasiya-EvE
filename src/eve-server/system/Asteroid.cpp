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
    Updates:    Allan
*/

#include "eve-server.h"

#include "system/Asteroid.h"
#include "ship/DestinyManager.h"


AsteroidSE::AsteroidSE(InventoryItemRef self, PyServiceMgr& services, SystemManager* system)
: ObjectSystemEntity(self, services, system),
m_growTimer(360000) /* arbitrary for 1 hour */
{
    m_growTimer.Disable();
}

void AsteroidSE::Process() {
    /* called by EntityList::Process on every loop */
    /* this is empty default call */
    SystemEntity::Process();
}

void AsteroidSE::ProcessObject() {
    /* called by EntityList::Process on each tic */
    /* this is empty default call */
    ObjectSystemEntity::ProcessObject();

    /*  set/check timers for grow/respawn, etc */
    if (m_growTimer.Check())
        Grow();

}

void AsteroidSE::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;

    BallHeader head;
        head.entityID = GetID();
        head.mode = DSTBALL_RIGID;
        head.radius = GetRadius();
        head.x = x();
        head.y = y();
        head.z = z();
        head.flags = IsMassive;
    into.Append( head );

    DSTBALL_RIGID_Struct main;
        main.formationID = 0xFF;
    into.Append( main );

    _log(COMMON__WARNING, "AsteroidSE::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void AsteroidSE::MakeDamageState(DoDestinyDamageState &into) {
    into.shield = 1.0;
    into.recharge = 30000;
    into.timestamp = Win32TimeNow();
    into.armor = 1.0;
    into.structure = 1.0;
}

void AsteroidSE::Grow() {
    /*  not real sure how to implement this
     * maybe use internal data structure to hold sizes (current, possible) and time interval
     * use this to check/update current sizes (radius and mass)
     */
}
