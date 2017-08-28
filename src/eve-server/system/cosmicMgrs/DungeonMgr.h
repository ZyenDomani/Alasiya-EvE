
 /**
  * @name DungeonMgr.h
  *     Dungeon managment system for Alasiya EvEmu
  *
  * @Author:        Allan
  * @date:          12 December 2015
  *
  */



#ifndef _EVEMU_SYSTEM_DUNGEONMGR_H
#define _EVEMU_SYSTEM_DUNGEONMGR_H

#include <unordered_map>
#include "POD_containers.h"
#include "system/SystemGPoint.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/ManagerDB.h"


/*
 * struct ActiveDungeon {
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

    uint32 GetDungeonID()                               { return ++m_dungeonID; }

protected:
    void _Populate();

    typedef std::unordered_multimap<uint32, ActiveDungeon> ActiveDungeonDef;    //systemID is key (defined in ManagerDB)
    typedef std::unordered_multimap<uint16, DunTemplate> DunTemplateDef;    //templateID is key
    typedef std::unordered_multimap<uint16, DunRoomInfo> DunRoomInfoDef;       //roomID is key
    typedef std::unordered_multimap<uint16, DunRoomData> DunRoomsDef;       //roomID is key
    typedef std::unordered_multimap<uint16, DunGroupData> DunGroupsDef;     //groupID is key

public:
    ActiveDungeonDef activeDungeons;  // systemID is key.  holds active dungeon data
    DunTemplateDef templates;         // templateID is key.  holds all template data
    DunRoomInfoDef roomInfo;          // roomID is key.  holds all room info
    DunRoomsDef rooms;                // roomID is key.  holds all room data
    DunGroupsDef groups;              // groupID is key. holds all group data

private:
    ManagerDB m_db;

    uint32 m_dungeonID;
};

#define sDunDataMgr \
( DungeonDataMgr::get() )

/*  this class is in charge of creating/destroying and maintaining
 * dungeons in a system.
 *
 *  a new iteration of this class is created for each system as that system is booted.
 */

class AnomalyMgr;
class SpawnMgr;
class PyServiceMgr;

class DungeonMgr {
public:
    DungeonMgr(SystemManager *system, PyServiceMgr& svc);
    ~DungeonMgr();


    bool Init(AnomalyMgr* anomMgr, SpawnMgr* spawnMgr);
    void Process();
    void Load();

    bool MakeDungeon(CosmicSignature& sig);

    bool Create(uint16 templateID, CosmicSignature& sig);

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

    int8 GetFactionID(uint32 factionID);
    int8 GetRandLevel();

    bool m_initalized;

    std::vector<DunGroupData> m_anomalyItems;

    std::map<uint32, std::vector<uint32>> m_dungeonList;  // this holds all items associated with the key 'dungeonID' in this system

};

#endif  // _EVEMU_SYSTEM_DUNGEONMGR_H