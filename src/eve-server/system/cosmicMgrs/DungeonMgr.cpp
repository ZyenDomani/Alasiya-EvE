
 /**
  * @name DungeonMgr.cpp
  *     Dungeon managment system for Alasiya EvEmu
  *
  * @Author:        Allan
  * @date:          12 December 2015
  * @updated:       27 August 2017
  */


#include "eve-server.h"

#include "EVEServerConfig.h"
#include "PyServiceMgr.h"
#include "StaticDataMgr.h"
#include "system/cosmicMgrs/AnomalyMgr.h"
#include "system/cosmicMgrs/DungeonMgr.h"
#include "BeltMgr.h"

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
    double start = GetTimeMSeconds();
    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;

    m_db.GetDunTemplates(*res);
    DunTemplate dtemplates;
    while (res->GetRow(row)) {
        // SELECT dunTemplateID, dunTemplateName, dunEntryID, dunSpawnID, dunRoomID
        dtemplates.dunName = row.GetText(1);
        dtemplates.dunRoomID = row.GetInt(4);
        dtemplates.dunEntryID = row.GetInt(2);
        dtemplates.dunSpawnType = row.GetInt(3);
        templates.emplace(row.GetInt(0), dtemplates);
    }

    res->Reset();
    m_db.GetDunRoomData(*res);
    DunRoomData drooms;
    while (res->GetRow(row)) {
        // SELECT dunRoomID, dunGroupID, xpos, ypos, zpos
        drooms.dunGroupID = row.GetInt(1);
        drooms.x = row.GetInt(2);
        drooms.y = row.GetInt(3);
        drooms.z = row.GetInt(4);
       rooms.emplace(row.GetInt(0), drooms);
    }

    res->Reset();
    m_db.GetDunGroupData(*res);
    DunGroupData dgroups;
    while (res->GetRow(row)) {
        // SELECT d.dunGroupID, d.itemTypeID, d.itemGroupID, t.typeName, t.groupID, g.categoryID, d.xpos, d.ypos, d.zpos
        dgroups.typeID = row.GetInt(1);
        dgroups.typeName = row.GetText(3);
        dgroups.typeGrpID = row.GetInt(4);
        dgroups.typeCatID = row.GetInt(5);
        dgroups.x = row.GetInt(6);
        dgroups.y = row.GetInt(7);
        dgroups.z = row.GetInt(8);
        groups.emplace(row.GetInt(0), dgroups);
    }

    res->Reset();
    m_db.GetDunEntryData(*res);
    DunEntryData dentry;
    while (res->GetRow(row)) {
        //SELECT dunEntryID, xpos, ypos, zpos FROM dunEntryData
        dentry.x = row.GetInt(1);
        dentry.y = row.GetInt(2);
        dentry.z = row.GetInt(3);
        entrys.emplace(row.GetInt(0), dentry);
    }

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
        groups.emplace(spawn.dunRoomSpawnID, spawn);
    } */

    //cleanup
    SafeDelete(res);

    sLog.Cyan("   DungeonDataMgr", "%u rooms in %u buckets and %u groups in %u buckets for %u dungeon templates loaded in %.3fms.",
              rooms.size(), rooms.bucket_count(), groups.size(), groups.bucket_count(), templates.size(), (GetTimeMSeconds() - start));
}

void DungeonDataMgr::AddDungeon(ActiveDungeon& dungeon)
{
    activeDungeons.emplace(dungeon.systemID, dungeon);
    _log(COSMIC_MGR__MESSAGE, "Added Dungeon %u (%u) in systemID %u to active dungeon list.", dungeon.dunItemID, dungeon.dunTemplateID, dungeon.systemID);
    //m_db.SaveActiveDungeon(dungeon);
}

void DungeonDataMgr::GetDungeons(std::vector< ActiveDungeon >& dunList)
{
    for (auto cur : activeDungeons)
        dunList.push_back(cur.second);
}



DungeonMgr::DungeonMgr(SystemManager* mgr, PyServiceMgr& svc)
: m_system(mgr),
m_services(svc),
m_anomMgr(nullptr),
m_spawnMgr(nullptr)
{
    m_initalized = false;
    m_anomalyItems.clear();
}

DungeonMgr::~DungeonMgr()
{
    /*  this may be hanging system unloading....
    //for now we're deleting everything till i can write proper item handling code
    std::map<uint32, std::vector<uint32>>::iterator itr = m_dungeonList.begin();
    while (itr != m_dungeonList.end()) {
        std::vector<uint32>::iterator itr2 = itr->second.begin();
        while (itr2 != itr->second.end())
            InventoryDB::DeleteItem(*itr2);
    }
    */
}

bool DungeonMgr::Init(AnomalyMgr* anomMgr, SpawnMgr* spawnMgr)
{
    if (!sConfig.cosmic.DungeonEnabled){
        _log(COSMIC_MGR__MESSAGE, "Dungeon System Disabled.  Not Initializing Dungeon Manager for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
        return m_initalized;
    }

    m_anomMgr = anomMgr;
    m_spawnMgr = spawnMgr;

    if (m_anomMgr == nullptr) {
        _log(COSMIC_MGR__ERROR, "System Init Fault. anomMgr == nullptr.  Not Initalizing Dungeon Manager for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
        return m_initalized;
    }

    if (m_spawnMgr == nullptr) {
        _log(COSMIC_MGR__ERROR, "System Init Fault. spawnMgr == nullptr.  Not Initalizing Dungeon Manager for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
        return m_initalized;
    }

    Load();

    _log(COSMIC_MGR__MESSAGE, "DungeonMgr Initialized for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
    return (m_initalized = true);
}

// called from AnomalyMgr
void DungeonMgr::Process() {
    if (!m_initalized)
        return;

    // this is used to remove empty/completed/timed-out dungons.
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

bool DungeonMgr::Create(uint32 templateID, CosmicSignature& sig)
{
    // get dungeon template
    std::unordered_multimap<uint32, DunTemplate>::iterator itr = sDunDataMgr.templates.find(templateID);
    if (itr == sDunDataMgr.templates.end()) {
        _log(COSMIC_MGR__ERROR, "DungeonMgr::Create() - template %u not found.", templateID);
        return false;
    }

    int32 roomID = itr->second.dunRoomID;
    if (roomID == 0) {
        _log(COSMIC_MGR__ERROR, "DungeonMgr::Create() - roomID is 0 for template %u.", templateID);
        return false;
    }

    //uint32 typeID = itr->second.dunTypeID;  **removed**

    if ((sig.dungeonType == EVEDUNG::dunTypes::typeGravimetric)      //2
        or (sig.dungeonType == EVEDUNG::dunTypes::typeMagnetometric) //3
        or (sig.dungeonType == EVEDUNG::dunTypes::typeRadar)         //4
        or (sig.dungeonType == EVEDUNG::dunTypes::typeLadar)         //5
        or (sig.ownerID == factionRogueDrones)) {
            sig.sigName = itr->second.dunName;
        } else {
            sig.sigName = sDataMgr.GetFactionName(sig.ownerID);
            sig.sigName += itr->second.dunName;
        }

    GPoint pos(sig.x, sig.y, sig.z);

    // create and spawn and save actual anomaly item  // typeID, ownerID, locationID, flag, name, &_position
    ItemData iData(sig.sigTypeID, sig.ownerID, sig.systemID, flagAutoFit, sig.sigName.c_str(), pos);

    /** @todo update this to use temp items */
    InventoryItemRef iRef = m_services.item_factory->SpawnItem(iData);  /* not sure how well generic spawn will work here. */
    if (iRef.get() == nullptr) // make error and exit
        return false;
    // do this or create/add generic se here?
    DBSystemDynamicEntity entity;
        entity.categoryID = iRef->categoryID();
        entity.groupID = iRef->groupID();
        entity.itemID = iRef->itemID();
        entity.itemName = sig.sigName;
        entity.typeID = sig.sigTypeID;
        entity.x = pos.x;
        entity.y = pos.y;
        entity.z = pos.z;
        /** @todo  fix these... */
        entity.ownerID = sig.ownerID;
        entity.allianceID = 0;  /** @todo  may have to write a method to check and set this */
        entity.corporationID = sDataMgr.GetCorpID(entity.ownerID);
        // do the spawn using SystemManager's BuildEntity:
    /** @todo this is more shit that should NOT be in db */
    m_system->BuildDynamicEntity(entity);
    sig.sigItemID = entity.itemID;

    _log(COSMIC_MGR__TRACE, "DungeonMgr::Create() - templateID %u, roomID %i for %s", templateID, roomID, sig.sigName.c_str());

    /* do we need this?  persistant dungeons? */
   // if ((typeID == 1) or (typeID == 8) or (typeID == 9) or (typeID == 10)) {
        // setup data to save active dungeon
        ActiveDungeon dungeon;
            dungeon.dunExpiryTime = Win32TimeNow() + (Win32Time_Day * 3);       // 3 days - i know this isnt right. just for testing.
            dungeon.dunTemplateID = templateID;
            dungeon.dunItemID = sig.sigItemID;
            dungeon.state = 0;  //dunType here.
            dungeon.systemID = sig.systemID;
            dungeon.x = sig.x;
            dungeon.y = sig.y;
            dungeon.z = sig.z;
        sDunDataMgr.AddDungeon(dungeon);
    //}

    // get room and group data and put in spawn vector
    int16 x=0, y=0, z=0;
    int32 groupID=0;
    DunGroupData grp;
    auto roomRange = sDunDataMgr.rooms.equal_range(roomID);
    for (auto it = roomRange.first; it != roomRange.second; ++it) {
        x = it->second.x;
        y = it->second.y;
        z = it->second.z;
        groupID = it->second.dunGroupID;
        auto groupRange = sDunDataMgr.groups.equal_range(groupID);
        for (auto it2 = groupRange.first; it2 != groupRange.second; ++it2) {
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

    /* at this point, we have all room/group data.
     * now, we need to separate grav sites and send to beltMgr for processing.
     *
     */

    if (sig.dungeonType == EVEDUNG::dunTypes::typeGravimetric) {
        // dungeon template for grav sites just give 'extra' roid data
        std::vector<uint16> roidTypes;
        return m_system->GetBeltMgr()->Create(sig, roidTypes);
    } else {
        /* item spawning method - just set up data and let SystemManager create and place the object */
        uint32 systemID = m_system->GetID();
        std::vector<uint32> items;
        GPoint pos2(NULL_ORIGIN);
        auto cur = m_anomalyItems.begin();
        while (cur != m_anomalyItems.end()) {
            pos2.x = sig.x + cur->x;
            pos2.y = sig.y + cur->y;
            pos2.z = sig.z + cur->z;
            // typeID, ownerID, locationID, flag, name, &_position
            ItemData iData(cur->typeID, sig.ownerID, systemID, flagAutoFit, cur->typeName.c_str(), pos2);

            /** @todo update this to use temp items */
            InventoryItemRef item = m_services.item_factory->SpawnItem(iData);  /* not sure how well generic spawn will work here. */
            if (item.get() == nullptr) // we'll survive...
                continue;

            DBSystemDynamicEntity entity;
                entity.categoryID = (EVEItemCategories)cur->typeCatID;
                entity.groupID = cur->typeGrpID;
                entity.itemID = item->itemID();
                entity.itemName = cur->typeName;
                entity.typeID = cur->typeID;
                entity.x = pos2.x;
                entity.y = pos2.y;
                entity.z = pos2.z;
                /** @todo  fix these... */
                entity.ownerID = sig.ownerID;
                entity.allianceID = -1;
                entity.corporationID = sDataMgr.GetCorpID(sig.ownerID);
            // do the spawn using SystemManager's BuildEntity:
                /** @todo this is more shit that should NOT be in db */
            m_system->BuildDynamicEntity(entity);
            items.push_back(item->itemID());
            ++cur;
        }
        _log(COSMIC_MGR__TRACE, "DungeonMgr::Create() - dungeonID %u created with %u items in system %u using template %u.", \
              sig.sigItemID, m_anomalyItems.size(),sig.systemID, templateID);

        m_anomalyItems.clear();
        if (!items.empty())
            m_dungeonList.insert(std::make_pair(sig.sigItemID, items));
    }
    return true;
}

bool DungeonMgr::MakeDungeon(CosmicSignature& sig)
{
    using namespace EVEDUNG;

    float secRating = m_system->GetSystemSecurityRating();

    int8 type = 1; // > 0.6
    if (secRating < 0.1)
        type = 3;
    else if (secRating < 0.6)
        type = 2;

    float level = 1;
    int8 subType = 1;
    int8 factionID = GetFactionID(sig.ownerID);

    switch (sig.dungeonType) {
        case dunTypes::typeGravimetric: {       // 2
            factionID = 0;
            if (type == 1) {
                subType = MakeRandomInt(0,5);
            } else if (type == 2) {
                subType = MakeRandomInt(0,3);
            } else if (type == 3) {
                subType = MakeRandomInt(0,2);
            }

            // this cannot be random....need to verify these roid types CAN spawn in this (m_system) region.
            level = MakeRandomFloat();
            if (level < 0.1) {
                level = 3;
            } else if (level < 0.3) {
                level = 2;
            } else {
                level = 1;
            }
        } break;
        case dunTypes::typeMagnetometric: {     // 3
            subType = MakeRandomInt(1,8);
            if (type == 3) {
                level = MakeRandomFloat();
                if (level < 0.1) {
                    level = 3;
                    subType = MakeRandomInt(1,7);
                    factionID = 0;
                } else if (level < 0.3) {
                    level = 2;
                    subType = MakeRandomInt(1,6);
                } else
                    level = 1;
            } else {
                if (IsEven(MakeRandomInt(0,10)))
                    level = 2;
                else
                    level = 1;
            }
        } break;
        case dunTypes::typeRadar: {             // 4
            if (factionID == 6) {
                level = 1;
                subType = 1;
                factionID = 0;
            } else {
                subType = MakeRandomInt(1,8);
                if (type == 3)
                    if (IsEven(MakeRandomInt(0,10))) {
                        level = 2;
                        factionID = 0;
                    }
            }
        } break;
        case dunTypes::typeLadar: {             // 5
            factionID = 0;
            subType = MakeRandomInt(1,8);
        } break;
        case dunTypes::typeAnomaly: {           // 7
            if (factionID < 6)
                factionID = 0;
            subType = MakeRandomInt(1,5);
            if (type == 1) {
                if (subType == 1) {
                    level = GetRandLevel();
                }
            } else if (type ==2) {
                if (subType == 2) {
                    level = GetRandLevel();
                } else if (subType == 4) {
                    level = GetRandLevel();
                }
            } else if (type == 3) {
                if (subType == 1) {
                    level = GetRandLevel();
                } else if (subType == 3) {
                    level = GetRandLevel();
                }
            }
        } break;
        case dunTypes::typeMission: {   // 1
            // not sure how im gonna do this one yet...make it unrated for now
            sig.dungeonType = 8;
        };
        case dunTypes::typeUnrated: {           // 8
            if (factionID == 6)
                subType = MakeRandomInt(1,3);
            else {
                factionID = 0;
                subType = MakeRandomInt(1,5);
            }
        } break;
        case dunTypes::typeEscalation:  // 9
        case dunTypes::typeDED_Complex: {  // 10
            sig.dungeonType = 9;
            if (factionID == 6)
                subType = MakeRandomInt(1,3);
            else {
                factionID = 0;
                subType = MakeRandomInt(1,5);
            }
        };
        case 0: {
            sig.dungeonType = 7;
            subType = MakeRandomInt(1,5);
            if (type == 1) {
                if (subType == 1) {
                    level = GetRandLevel();
                }
            } else if (type ==2) {
                if (subType == 2) {
                    level = GetRandLevel();
                } else if (subType == 4) {
                    level = GetRandLevel();
                }
            } else if (type == 3) {
                if (subType == 1) {
                    level = GetRandLevel();
                } else if (subType == 3) {
                    level = GetRandLevel();
                }
            }
        } break;
    }

    /* templateID format.  ABCDE
     *       A = sitetype - 1:mission, 2:grav, 3:mag, 4:radar, 5:ladar, 7:anomaly, 8:unrated, 9:ded/escalation
     *       B = type - anomaly security: 1=hi, 2=lo, 3=null, 4=mid, mission misc: 1 to 9
     *       C = subtype  - 2: 0 to 5, 7: 1 to 5, 1: 1 to 9, 3: 1 to 8, 5: 1 to 8
     *       D = level - 2: 1 to 3, 4: type 3, 2 levels, 7: 1 to 5, 1: 1 to 9
     *       E = faction - 0=*use region faction*, 1=Serpentis, 2=Angel, 3=Blood, 4=Guristas, 5=Sansha, 6=Drones, 7= , 8= , 9=
     */

    _log(COSMIC_MGR__MESSAGE, "DungeonMgr::MakeDungeon() - Calling Create on type %u", sig.dungeonType);

    uint32 templateID = (sig.dungeonType *10000) + (type *1000) + (subType *100) + (level *10) + factionID;

    return Create(templateID, sig);
}

int8 DungeonMgr::GetFactionID(uint32 factionID)
{
    switch (factionID) {
        case factionAngel:          return 2;
        case factionSanshas:        return 5;
        case factionBloodRaider:    return 3;
        case factionGuristas:       return 4;
        case factionSerpentis:      return 1;
        case factionRogueDrones:    return 6;
        // these arent gonna fit...
        case factionAmarr:          return 0;
        case factionAmmatar:        return 0;
        case factionCaldari:        return 0;
        case factionGallente:       return 0;
        case factionMinmatar:       return 0;
    }
}

int8 DungeonMgr::GetRandLevel()
{
    float level = MakeRandomFloat();
    if (level < 0.15)
        return 4;
    else if (level < 0.20)
        return 3;
    else if (level < 0.40)
        return 2;
    else
        return 1;
}


    /*
    In player-owned sovereign nullsec, using Ore Prospecting Arrays,
    (23510,'Small Asteroid Cluster',0,2,0,1,0,0,0),
    (23520,'Moderate Asteroid Cluster',0,2,0,15,0,0,0),
    (23530,'Large Asteroid Cluster',0,2,0,29,0,0,0),
    (23540,' Enormous Asteroid Cluster ',0,2,0,29,0,0,0),
    (23550,'Colossal Asteroid Cluster',0,2,0,29,0,0,0),
    */