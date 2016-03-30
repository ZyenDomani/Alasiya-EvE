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
 *    Author:   Allan
 */

#ifndef _EVE_NPC_SPAWNDB_H__
#define _EVE_NPC_SPAWNDB_H__

#include "ServiceDB.h"

//  new spawn manager   -allan 12July15

class SpawnDB
{
public:
    //  following is for new spawn manager   -allan 12July15
    void GetRegionFactionInfo(DBQueryResult& res);
    void GetFactionGroups(DBQueryResult& res);
    void GetSpawnClasses(DBQueryResult& res);
    void GetGroupTypeIDs(uint32 groupID, DBQueryResult& res);

    void DeleteSpawnedRats();
};

/*spawn class is type of spawn based on system security rating
 * 1-7 are 'normal' roid rat spawns
 * 8 is hauler spawns (convoy, carrier, trailer, transporter, bulker, trucker, loader)
 * 9 is commander spawns
 * 10 is officer spawns
 *sub is the type subgroup number.  nothing special here.
 */

#endif  // _EVE_NPC_SPAWNDB_H__