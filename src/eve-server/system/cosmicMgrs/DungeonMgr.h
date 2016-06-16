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
#include "system/SystemManager.h"
#include "system/cosmicMgrs/ManagerDB.h"


struct DunTemplate {
    uint8 dunRoomID = 0;
    uint8 dunEntryID = 0;
    uint8 dunTypeID = 0;
    uint8 dunSpawnType = 0;
    uint8 dunRooms = 0;
    uint8 dunRoomTypeID = 0;
    uint8 dunRoomCategoryID = 0;
};

struct DunRoomInfo {
    uint8 dunRoomID = 0;
    uint8 dunRoomType = 0;
    uint8 dunRoomCategory = 0;
    uint8 dunRoomSpawnID = 0;
    uint8 dunRoomSpawnType = 0;
};

struct DunRoomData {
    uint8 dunGroupID;
    uint16 x;
    uint16 y;
    uint16 z;
};

struct DunGroupData {
    uint32 typeID;
    uint32 typeGrpID;   // this is groupID of the itemType, and needed to simplify create/spawn code
    uint8 typeCatID;    // this is categoryID of the itemType, and needed to simplify create/spawn code
    uint16 x;
    uint16 y;
    uint16 z;
};

struct DunRoomSpawnInfo {
    uint8 dunRoomSpawnID = 0;
    uint8 dunRoomSpawnType = 0;
    uint16 x;
    uint16 y;
    uint16 z;
};

/*
 * class DBActiveDungeon {
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

    void AddDungeon(uint32 systemID, DBActiveDungeon& dungeon);

protected:
    void _Populate();

    typedef std::unordered_multimap<uint32, DBActiveDungeon> ActiveDungeonDef;    //systemID is key (defined in ManagerDB)
    typedef std::unordered_multimap<uint8, DunTemplate> DunTemplateDef;    //templateID is key
    typedef std::unordered_multimap<uint8, DunRoomInfo> DunRoomInfoDef;       //roomID is key
    typedef std::unordered_multimap<uint8, DunRoomData> DunRoomsDef;       //roomID is key
    typedef std::unordered_multimap<uint8, DunGroupData> DunGroupsDef;     //groupID is key

public:
    ActiveDungeonDef m_activeDungeons;  // systemID is key.  holds active dungeon data
    DunTemplateDef m_templates;         // templateID is key.  holds all template data
    DunRoomInfoDef m_roomInfo;          // roomID is key.  holds all room info
    DunRoomsDef m_rooms;             // roomID is key.  holds all room data
    DunGroupsDef m_groups;              // groupID is key. holds all group data

private:
    ManagerDB m_db;
};

#define sDunDataMgr \
( DungeonDataMgr::get() )

/*  this class is in charge of creating/destroying and maintaining
 * dungeons in a system.
 *
 *  a new iteration of this class is created for each system as that system
 * is booted.
 */

class SpawnMgr;
class PyServiceMgr;

class DungeonMgr {
public:
    DungeonMgr(SystemManager *system, PyServiceMgr& svc);
    ~DungeonMgr()     { /* do nothing here */ }


    void Init();
    void Process();
    void Load();

    /* we do not own any of these */
protected:
    ManagerDB m_db;

    /* we do not own any of these */
private:
    SpawnMgr* m_spawnMgr;
    SystemManager* m_system;
    PyServiceMgr& m_services;

    bool m_initalized;

};

#endif  // _EVEMU_SYSTEM_DUNGEONMGR_H