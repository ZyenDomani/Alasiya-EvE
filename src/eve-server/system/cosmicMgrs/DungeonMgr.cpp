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

#include "EVEServerConfig.h"
#include "PyServiceMgr.h"
#include "system/cosmicMgrs/DungeonMgr.h"

DungeonDataMgr::DungeonDataMgr()
{
}

int DungeonDataMgr::Initialize()
{
    _Populate();
    return 1;
}

void DungeonDataMgr::_Populate()
{
    double start = GetTimeUSeconds();
    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;

    m_db.GetDunTemplates(*res);
    DunTemplate templates;
    while (res->GetRow(row)) {
        // SELECT dunTemplateID, dunRoomID, dunEntryID, dunTypeID, dunSpawnType, dunRooms, dunRoomTypeID, dunRoomCategoryID
        templates.dunRoomID = row.GetInt(1);
        templates.dunEntryID = row.GetInt(2);
        templates.dunTypeID = row.GetInt(3);
        templates.dunSpawnType = row.GetInt(4);
        templates.dunRooms = row.GetInt(5);
        templates.dunRoomTypeID = row.GetInt(6);
        templates.dunRoomCategoryID = row.GetInt(7);
        m_templates.emplace(row.GetInt(0), templates);
    }

    res->Reset();
    m_db.GetDunRoomData(*res);
    DunRoomData rooms;
    while (res->GetRow(row)) {
        // SELECT dunRoomID, dunGroupID, xpos, ypos, zpos
        rooms.dunGroupID = row.GetInt(1);
        rooms.x = row.GetInt(2);
        rooms.y = row.GetInt(3);
        rooms.z = row.GetInt(4);
        m_rooms.emplace(row.GetInt(0), rooms);
    }

    res->Reset();
    m_db.GetDunGroupData(*res);
    DunGroupData groups;
    while (res->GetRow(row)) {
        // SELECT d.typeID, t.groupID, g.categoryID, d.xpos, d.ypos, d.zpos
        groups.typeID = row.GetInt(0);
        groups.typeGrpID = row.GetInt(1);
        groups.typeCatID = row.GetInt(2);
        groups.x = row.GetInt(3);
        groups.y = row.GetInt(4);
        groups.z = row.GetInt(5);
        m_groups.emplace(groups.typeID, groups);
    }

    /* not ready yet
    res->Reset();
    m_db.GetDunRoomInfo(*res);
    DunRoomInfo info;
    while (res->GetRow(row)) {
        // SELECT dunRoomID, dunRoomType, dunRoomCategory, dunRoomSpawnID, dunRoomSpawnType
        info.dunRoomID = row.GetInt(0);
        info.dunRoomType = row.GetInt(1);
        info.dunRoomCategory = row.GetInt(2);
        info.dunRoomSpawnID = row.GetInt(3);
        info.dunRoomSpawnType = row.GetInt(4);
        m_groups.emplace(info.dunRoomID, info);
    } */

    /* not ready yet
    res->Reset();
    m_db.GetDunSpawnInfo(*res);
    DunRoomSpawnInfo spawn;
    while (res->GetRow(row)) {
        //SELECT dunRoomSpawnID, dunRoomSpawnType, xpos, ypos, zpos
        spawn.dunRoomSpawnID = row.GetInt(0);
        spawn.dunRoomSpawnType = row.GetInt(1);
        spawn.x = row.GetInt(2);
        spawn.y = row.GetInt(3);
        spawn.z = row.GetInt(4);
        m_groups.emplace(spawn.dunRoomSpawnID, spawn);
    } */


    //cleanup
    SafeDelete(res);

    sLog.Log("   DungeonDataMgr", "%u rooms in %u buckets and %u groups in %u buckets for %u dungeon templates loaded in %.3fms.",
             m_rooms.size(), m_rooms.bucket_count(), m_groups.size(), m_groups.bucket_count(), m_templates.size(), (GetTimeUSeconds() - start));
}



DungeonMgr::DungeonMgr(SystemManager* mgr, PyServiceMgr& svc)
: m_system(mgr),
m_services(svc)
{
    m_initalized = false;
}

void DungeonMgr::Init()
{
    if (!sConfig.cosmic.DungeonEnabled) return;

    m_initalized = true;

    _log(COSMIC_MGR__MESSAGE, "DungeonMgr Initialized for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
}

void DungeonMgr::Process() {
    if (!m_initalized) return;

}

void DungeonMgr::Load()
{
    std::vector<DBActiveDungeon> dungeons;
    m_db.GetActiveDungeons(m_system->GetID(), dungeons);
    /** @todo this will need more work as the system matures...
    for(auto dungeon : dungeons) {
        InventoryItemRef dungeonRef = m_system->itemFactory()->GetItem( dungeon.dungeonID );
        if( !dungeonRef ) {
            _log(COSMIC_MGR__WARNING, "DungeonMgr::Load() -  Unable to spawn dungeon item #%u:'%s' of type %u.", dungeon.dungeonID, dungeon.typeID);
            continue;
        }
        AsteroidSE* asteroidObj = new AsteroidSE( dungeonRef, *(m_system->GetServiceMgr()), m_system );
        if( !asteroidObj ) {
            _log(COSMIC_MGR__WARNING, "DungeonMgr::Load() -  Unable to spawn dungeon entity #%u:'%s' of type %u.", dungeon.dungeonID, dungeon.typeID);
            continue;
        }
        _log(COSMIC_MGR__TRACE, "DungeonMgr::Load() - Loaded dungeon %u, type %u for %s(%u)", dungeon.dungeonID, dungeon.typeID, m_system->GetName().c_str(), m_systemID );
        sBubbleMgr.Add( asteroidObj );
        sDunDataMgr.AddDungeon(std::pair<uint32, DBActiveDungeon*>(m_system->GetID(), dungeon));
    } */
}


// EVEMU_DUNGEON_ID  1200000000
