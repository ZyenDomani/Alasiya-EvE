
 /**
  * @name DungeonMgr.h
  *     Dungeon management system for Alasiya EvEmu
  *
  * @Author:        Allan
  * @date:          12 December 2015
  *
  */



#ifndef _EVEMU_SYSTEM_DUNGEONMGR_H
#define _EVEMU_SYSTEM_DUNGEONMGR_H

#include <unordered_map>
#include "EVE_Dungeon.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/ManagerDB.h"


/*
 * struct Dungeon::ActiveData {
 *    uint32 systemID;
 *    uint32 dungeonID;
 *    uint8 dunTemplateID;
 *    int64 dunExpiryTime;
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
    ~DungeonDataMgr() { /* nothing do to yet */ }

    // Initializes the Table:
    int Initialize();

    void AddDungeon(Dungeon::ActiveData& dungeon);
    void GetDungeons(std::vector<Dungeon::ActiveData>& dunList);

    bool GetTemplate(uint32 templateID, Dungeon::Template& dTemplate);

    uint32 GetDungeonID()                               { return ++m_dungeonID; }

    const char* GetDungeonType(int8 typeID);

    PyDict* GetPaletteGroups()                          { return m_paletteGroups; }

protected:
    typedef std::map<uint32, Dungeon::Template> DunTemplateDef;                       //templateID/data
    typedef std::unordered_multimap<uint32, Dungeon::ActiveData> ActiveDungeonDef;    //systemID/data
    typedef std::unordered_multimap<uint32, Dungeon::RoomData> DunRoomsDef;           //roomID/data
    typedef std::unordered_multimap<uint32, Dungeon::EntryData> DunEntryDef;          //entryID/data
    typedef std::unordered_multimap<uint32, Dungeon::GroupData> DunGroupsDef;         //groupID/data

    DunTemplateDef templates;         // templateID/data

public:
    ActiveDungeonDef activeDungeons;  // systemID/data
    DunEntryDef entrys;               // entryID/data
    DunRoomsDef rooms;                // roomID/data
    DunGroupsDef groups;              // groupID/data

private:
    void Populate();

    uint32 m_dungeonID;

    PyDict* m_paletteGroups;

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

    bool Create(uint32 templateID, CosmicSignature& sig);

protected:
    ManagerDB m_db;

    void CreateDeco(uint32 templateID, CosmicSignature& sig);

    /* we do not own any of these */
private:
    bool m_initalized;

    AnomalyMgr* m_anomMgr;
    SpawnMgr* m_spawnMgr;
    SystemManager* m_system;
    PyServiceMgr& m_services;

    int8 GetFaction(uint32 factionID);
    int8 GetRandLevel();

    void AddDecoToVector(uint8 dunType, uint32 templateID, std::vector<uint16>& groupVec);


    std::vector<Dungeon::GroupData> m_anomalyItems;

    std::map<uint32, std::vector<uint32>> m_dungeonList;  // this holds all items associated with the key 'dungeonID' in this system

};

#endif  // _EVEMU_SYSTEM_DUNGEONMGR_H


/* templateID format.  ABCDE
 *       A = site - 1:mission, 2:grav, 3:mag, 4:radar, 5:ladar, 6:ded, 7:anomaly, 8:unrated, 9:escalation
 *       B = sec - mission: 1-9 (incomplete); others - sysSec: 1=hi, 2=lo, 3=null, 4=mid;
 *       C = type - grav: ore 0-5, ice 6-9; anomaly: 1-5; mission: 1-9; mag: *see below*; ded: 1-8; ladar/radar: 1-8
 *       D = level - mission: 1-9; grav: ore 1-3, ice 0; mag: *see below*; radar: 1-norm; 2-digital(nullsec); ladar: 1; anomaly: 1-5
 *       E = faction - 0=code defined, 1=Serpentis, 2=Angel, 3=Blood, 4=Guristas, 5=Sansha, 6=Drones, 7=region sov, 8=region rat, 9=other
 *
 * NOTE:  mag sites have multiple types and levels based on other variables.
 *      types are defined as relic(1), salvage(2), and drone(3), with salvage being dominant.
 *      for hisec and losec, levels are 1-8 for relic and salvage.  there are no drone mag sites here
 *      for nullsec, relic site levels are 1-8, salvage site levels are 1-4, and drone site levels are 1-7
 *
 * NOTE:  faction can only be 8 for grav and anomaly sites, unless drones (6). all others MUST use 1-6
 */

/* groupID format
 *      ABB - item def groups
 *   A = group type - 1:deco, 2:system effect beacon, 3:mining, 4:lco, 5:ships, 6:base, 7:station gun, 8:station wrecks, 9:misc
 *   B =  1:space objects, 2:effect beacons, 3:roid types, 4:ice types, 5:, 6:asteroid colony, 7:, 8:, 9:misc
 *   B = ship/gun faction:  1:Amarr, 2:Caldari, 3:Gallente, 4:Minmatar, 5:Sentinel, 6:Guardian
 *   B =  faction:  00:none, 01:Serpentis, 02:Angel, 03:Blood, 04:Guristas, 05:Sansha, 06:Drones, 07:Amarr, 08:Caldari, 09:Gallente,
 *                  10:Minmatar, 11:Sleeper, 12:Talocan, 13:Ammatar
 */

/* roomID format.  ABCD
 *      A = roomtype - 1:combat, 2:rescue, 3:capture, 4:salvage, 5:relic, 6:hacking, 7:roids, 8:clouds, 9:recon
 *      B = level -  0:none, 1:f, 2:d, 3:c, 4:af/ad, 5:bc, 6:ac, 7:bs, 8:abs, 9:hard
 *      C = amount/size - 0:code defined 1:small(1-5), 2:medium(2-10), 3:large(5-25), 4:enormous(10-50), 5:colossal(20-100), 6-9:ice
 *      D = faction - 0=drone, 1:Serpentis, 2:Angel, 3:Blood, 4:Guristas, 5:Sansha, 6:Amarr, 7:Caldari, 8:Gallente, 9:Minmatar
 *      D = sublevel - 0:gas, 1-5:ore, 6-9:ice
 */
