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
    m_dungeonID = EVEMU_DUNGEON_ID;

    // for now, we are deleting all saved dungeons on startup.  will fix this later as system matures.
    m_db.ClearDungeons();
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
        // SELECT dunTemplateID, dunName, dunRoomID, dunEntryID, dunTypeID, dunSpawnType, dunRooms, dunRoomTypeID, dunRoomCategoryID
        templates.dunName = row.GetText(1);
        templates.dunRoomID = row.GetInt(2);
        templates.dunEntryID = row.GetInt(3);
        templates.dunTypeID = row.GetInt(4);
        templates.dunSpawnType = row.GetInt(5);
        templates.dunRooms = row.GetInt(6);
        templates.dunRoomTypeID = row.GetInt(7);
        templates.dunRoomCategoryID = row.GetInt(8);
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
        // SELECT d.dunGroupID, d.itemTypeID, t.typeName, t.groupID, g.categoryID, d.xpos, d.ypos, d.zpos
        groups.typeID = row.GetInt(1);
        groups.typeName = row.GetText(2);
        groups.typeGrpID = row.GetInt(3);
        groups.typeCatID = row.GetInt(4);
        groups.x = row.GetInt(5);
        groups.y = row.GetInt(6);
        groups.z = row.GetInt(7);
        m_groups.emplace(row.GetInt(0), groups);
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
        m_roomInfo.emplace(info.dunRoomID, info);
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

    sLog.Cyan("   DungeonDataMgr", "%u rooms in %u buckets and %u groups in %u buckets for %u dungeon templates loaded in %.3fms.",
             m_rooms.size(), m_rooms.bucket_count(), m_groups.size(), m_groups.bucket_count(), m_templates.size(), (GetTimeUSeconds() - start));
}

void DungeonDataMgr::AddDungeon(ActiveDungeon& dungeon)
{
    m_activeDungeons.emplace(dungeon.systemID, dungeon);
    _log(COSMIC_MGR__MESSAGE, "Added Dungeon %u (%u) in systemID %u to active dungeon list.", dungeon.dunItemID, dungeon.dunTemplateID, dungeon.systemID);
    m_db.SaveActiveDungeon(dungeon);
}

void DungeonDataMgr::GetDungeons(std::vector< ActiveDungeon >& dunList)
{
    for (auto cur : m_activeDungeons)
        dunList.push_back(cur.second);
}



DungeonMgr::DungeonMgr(SystemManager* mgr, PyServiceMgr& svc)
: m_system(mgr),
m_services(svc)
{
    m_initalized = false;
    m_anomalyItems.clear();
}

void DungeonMgr::Init(AnomalyMgr* anomMgr, SpawnMgr* spawnMgr)
{
    m_anomMgr = anomMgr;
    m_spawnMgr = spawnMgr;

    if (!sConfig.cosmic.DungeonEnabled){
        _log(COSMIC_MGR__MESSAGE, "Dungeon System Disabled.  Not Initializing Dungeon Manager for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
        return;
    }

    m_initalized = true;

    _log(COSMIC_MGR__MESSAGE, "DungeonMgr Initialized for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
}

void DungeonMgr::Process() {
    if (!m_initalized) return;
    // im not sure if we need this.  what would be calling Process() on???
}

void DungeonMgr::Load()
{
    std::vector<ActiveDungeon> dungeons;
    m_db.GetSavedDungeons(m_system->GetID(), dungeons);
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
        sDunDataMgr.AddDungeon(std::pair<uint32, ActiveDungeon*>(m_system->GetID(), dungeon));
    } */
}

bool DungeonMgr::Create(uint16 templateID)
{
    uint32 roomID = 0, typeID = 0;

    // get dungeon template
    std::unordered_multimap<uint16, DunTemplate>::iterator itr = sDunDataMgr.m_templates.find(templateID);
    if (itr != sDunDataMgr.m_templates.end()) {
        roomID = itr->second.dunRoomID;
        typeID = itr->second.dunTypeID;
    }
    if (!roomID) {
        _log(COSMIC_MGR__ERROR, "DungeonMgr::Create() - roomID is 0 for template %u.", templateID);
        return false;
    }

    _log(COSMIC_MGR__TRACE, "DungeonMgr::Create() - templateID %u, roomID %u, typeID %u", templateID, roomID, typeID);

    // begin compiling data for saving in system signatures table.
    CosmicSignature sig;
        sig.dungeonName = itr->second.dunName;
        sig.sigID = sEntityList.GetAnomalyID();
        sig.sigItemID = sDunDataMgr.GetDungeonID();
        sig.systemID = m_system->GetID();
        sig.scanGroupID = ScanGroupAnomaly;         // this will change based on the actual ITEM being scanned...ship, tower, drone, etc.
        sig.typeID = 25880; // Cosmic_Signature
        sig.groupID = EVEDB::invGroups::Cosmic_Anomaly;
        sig.strengthAttributeID = AttrScanAllStrength;  // Unknown
    switch(typeID) {
        case typeGravimetric: { // 2
            sig.typeID = 25880; // Cosmic_Signature
            sig.groupID = EVEDB::invGroups::Cosmic_Signature;
            sig.strengthAttributeID = AttrScanGravimetricStrength;
        } break;
        case typeMagnetometric: { // 3,
            sig.typeID = 25880; // Cosmic_Signature
            sig.groupID = EVEDB::invGroups::Cosmic_Signature;
            sig.strengthAttributeID = AttrScanMagnetometricStrength;
        } break;
        case typeRadar: {       // 4,
            sig.typeID = 25880; // Cosmic_Signature
            sig.groupID = EVEDB::invGroups::Cosmic_Signature;
            sig.strengthAttributeID = AttrScanRadarStrength;
        } break;
        case typeLadar: {       // 5,
            sig.typeID = 25880; // Cosmic_Signature
            sig.groupID = EVEDB::invGroups::Cosmic_Signature;
            sig.strengthAttributeID = AttrScanLadarStrength;
        } break;
        // these will use default for now.  maybe change them later...wait till system matures more.
        case typeMission:       // 1
        case typeWormholes:     // 6
        case typeAnomaly:       // 7
        case typeUnrated:       // 8
        case typeEscalation:    // 9
        case typeDED_Complex: { // 10
        } break;
    }

    // get room and group data and put in spawn vector
    uint16 x=0, y=0, z=0, group = 0;
    DunGroupData grp;
    auto roomRange = sDunDataMgr.m_rooms.equal_range(roomID);
    for (auto it = roomRange.first; it != roomRange.second; it++) {
        x = it->second.x;
        y = it->second.y;
        z = it->second.z;
        group = it->second.dunGroupID;
        auto groupRange = sDunDataMgr.m_groups.equal_range(group);
        for (auto it2 = groupRange.first; it2 != groupRange.second; it2++) {
            grp.typeCatID = it2->second.typeCatID;
            grp.typeGrpID = it2->second.typeGrpID;
            grp.typeName = it2->second.typeName;
            grp.typeID = it2->second.typeID;
            grp.x = (x + it2->second.x);
            grp.y = (y + it2->second.y);
            grp.z = (z + it2->second.z);
            m_anomalyItems.push_back(grp);
        }
    }
    _log(COSMIC_MGR__TRACE, "DungeonMgr::Create() - there are %u items to be created for '%s' (%u:%u) .", \
            m_anomalyItems.size(), sig.dungeonName.c_str(), sig.sigItemID, templateID);

    // get rand pos >0.5au but <4au from random planet.
    GPoint pos = m_gp.GetAnomalyPoint(m_system);
    sig.x = pos.x;
    sig.y = pos.y;
    sig.z = pos.z;
    m_db.SaveAnomaly(sig);

    if ((typeID == 1) or (typeID == 8) or (typeID == 9) or (typeID == 10)) {
        // setup data to save active dungeon
        ActiveDungeon dungeon;
            dungeon.dunExpiryTime = Win32TimeNow() + (Win32Time_Day * 3);       // 3 days - i know this isnt right. just for testing.
            dungeon.dunTemplateID = templateID;
            dungeon.dunItemID = sig.sigItemID;
            dungeon.state = 0;  //dunType here.
            dungeon.systemID = sig.systemID;
            dungeon.x = pos.x;
            dungeon.y = pos.y;
            dungeon.z = pos.z;
        sDunDataMgr.AddDungeon(dungeon);
    }

    /* spawning method - just set up data and let SystemManager create and place the object */
    uint32 systemID = m_system->GetID();
    std::vector<uint32> items;
    GPoint pos2(NULL_ORIGIN);
    auto cur = m_anomalyItems.begin();
    while (cur != m_anomalyItems.end()) {
        pos2.x = pos.x + cur->x;
        pos2.y = pos.y + cur->y;
        pos2.z = pos.z + cur->z;
        // typeID, ownerID, locationID, flag, name, &_position
        ItemData iData(cur->typeID, 1/*fix this*/, systemID, flagAutoFit, cur->typeName.c_str(), pos2);

        /** @todo update this to use temp items */
        InventoryItemRef item = m_services.item_factory->SpawnItem(iData);  /* not sure how well generic spawn will work here. */
        if (!item) // we'll survive...
            continue;

        DBSystemDynamicEntity entity;
            entity.allianceID = 0;
            entity.categoryID = cur->typeCatID;
            entity.corporationID = 0;
            entity.groupID = cur->typeGrpID;
            entity.itemID = item->itemID();
            entity.itemName = cur->typeName;
            entity.ownerID = 1;
            entity.typeID = cur->typeID;
            entity.x = pos2.x;
            entity.y = pos2.y;
            entity.z = pos2.z;
        // do the spawn using SystemManager's BuildEntity:
        m_system->BuildDynamicEntity(entity);
        items.push_back(item->itemID());
        ++cur;
    }

    m_anomalyItems.clear();
    m_dungeonList.insert(std::make_pair(sig.sigItemID, items));
    _log(COSMIC_MGR__TRACE, "DungeonMgr::Create() - dungeonID %u created in system %u.", sig.sigItemID, sig.systemID);

    return true;
}


