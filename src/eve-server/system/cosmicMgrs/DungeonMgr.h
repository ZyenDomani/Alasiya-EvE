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

#ifndef _EVEMU_SYSTEM_DUNGEONMGR_H
#define _EVEMU_SYSTEM_DUNGEONMGR_H

#include <unordered_map>
#include "system/SystemGPoint.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/ManagerDB.h"


struct DunTemplate {
    std::string dunName = "";
    uint16 dunRoomID = 0;
    uint16 dunEntryID = 0;
    uint8 dunTypeID = 0;
    uint8 dunSpawnType = 0;
    uint8 dunRooms = 0;
    uint8 dunRoomTypeID = 0;
    uint8 dunRoomCategoryID = 0;
};

struct DunRoomInfo {
    uint16 dunRoomID = 0;
    uint8 dunRoomType = 0;
    uint8 dunRoomCategory = 0;
    uint8 dunRoomSpawnID = 0;
    uint8 dunRoomSpawnType = 0;
};

struct DunRoomData {
    uint32 dunGroupID = 0;
    uint16 x = 0;
    uint16 y = 0;
    uint16 z = 0;
};

struct DunGroupData {
    uint32 typeID = 0;
    std::string typeName = "";
    uint32 typeGrpID = 0;   // this is groupID of the itemType, and needed to simplify create/spawn code
    uint8 typeCatID = 0;    // this is categoryID of the itemType, and needed to simplify create/spawn code
    uint16 x = 0;
    uint16 y = 0;
    uint16 z = 0;
};

struct DunRoomSpawnInfo {
    uint16 dunRoomSpawnID = 0;
    uint16 dunRoomSpawnType = 0;
    uint16 x = 0;
    uint16 y = 0;
    uint16 z = 0;
};

/*
 * class ActiveDungeon {
 * public:
 *    uint32 systemID;
 *    uint32 dungeonID;
 *    uint8 dunTemplateID;
 *    uint64 dunExpiryTime;
 *    uint8 state;
 *    double x;
 *    double y;
 *    double z;
 * };
 */

// this class is a singleton object to have a common place for all dungeon template data
class DungeonDataMgr
: public Singleton< DungeonDataMgr >
{
public:
    DungeonDataMgr();
    virtual ~DungeonDataMgr() { /* nothing do to yet */ }

    // Initializes the Table:
    int Initialize();

    void AddDungeon(ActiveDungeon& dungeon);

    void GetDungeons(std::vector<ActiveDungeon>& dunList);

    uint32 GetDungeonID();

protected:
    void _Populate();

    typedef std::unordered_multimap<uint32, ActiveDungeon> ActiveDungeonDef;    //systemID is key (defined in ManagerDB)
    typedef std::unordered_multimap<uint16, DunTemplate> DunTemplateDef;    //templateID is key
    typedef std::unordered_multimap<uint16, DunRoomInfo> DunRoomInfoDef;       //roomID is key
    typedef std::unordered_multimap<uint16, DunRoomData> DunRoomsDef;       //roomID is key
    typedef std::unordered_multimap<uint16, DunGroupData> DunGroupsDef;     //groupID is key

public:
    ActiveDungeonDef m_activeDungeons;  // systemID is key.  holds active dungeon data
    DunTemplateDef m_templates;         // templateID is key.  holds all template data
    DunRoomInfoDef m_roomInfo;          // roomID is key.  holds all room info
    DunRoomsDef m_rooms;                // roomID is key.  holds all room data
    DunGroupsDef m_groups;              // groupID is key. holds all group data

private:
    ManagerDB m_db;

    uint32 m_dungeonID;
};

#define sDunDataMgr \
( DungeonDataMgr::get() )

/*  this class is in charge of creating/destroying and maintaining
 * dungeons in a system.
 *
 *  a new iteration of this class is created for each system as that system
 * is booted.
 */

class AnomalyMgr;
class SpawnMgr;
class PyServiceMgr;

class DungeonMgr {
public:
    DungeonMgr(SystemManager *system, PyServiceMgr& svc);
    ~DungeonMgr()     { /* do nothing here */ }


    void Init(AnomalyMgr* anomMgr, SpawnMgr* spawnMgr);
    void Process();
    void Load();

    void Create(uint16 templateID);

    /* we do not own any of these */
protected:
    ManagerDB m_db;
    SystemGPoint m_gp;

    /* we do not own any of these */
private:
    AnomalyMgr* m_anomMgr;
    SpawnMgr* m_spawnMgr;
    SystemManager* m_system;
    PyServiceMgr& m_services;

    bool m_initalized;

    std::vector<DunGroupData> m_anomalyItems;

    std::map<uint32, std::vector<uint32>> m_dungeonList;  // this holds all items associated with the key 'dungeonID' in this system

};

#endif  // _EVEMU_SYSTEM_DUNGEONMGR_H