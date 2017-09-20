
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
#include "SpawnMgr.h"

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
        // SELECT d.dunGroupID, d.itemTypeID, d.itemGroupID, t.typeName, t.groupID, g.categoryID, t.radius, d.xpos, d.ypos, d.zpos
        dgroups.typeID = row.GetInt(1);
        dgroups.typeName = row.GetText(3);
        dgroups.typeGrpID = row.GetInt(4);
        dgroups.typeCatID = row.GetInt(5);
        dgroups.radius = row.GetInt(6);
        dgroups.x = row.GetInt(7);
        dgroups.y = row.GetInt(8);
        dgroups.z = row.GetInt(9);
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

    // sort/save template room/group data to avoid compilation later?

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
    /*  this may be hanging system unloading...
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

    // not coded yet...placeholder for now
    m_spawnMgr->DoSpawnForAnomaly(itr->second.dunSpawnType);

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
        entity.ownerID = sig.ownerID;
        /** @todo  may have to write a method to check and set this */
        entity.allianceID = 0;
        entity.corporationID = sDataMgr.GetCorpID(entity.ownerID);
        // do the spawn using SystemManager's BuildEntity:
    /** @todo this is more shit that should NOT be in db */
    m_system->BuildDynamicEntity(entity);
    sig.sigItemID = entity.itemID;

    _log(COSMIC_MGR__TRACE, "DungeonMgr::Create() - templateID %u, roomID %i for %s", templateID, roomID, sig.sigName.c_str());

    /* do we need this?  persistant dungeons?
    if ((typeID == 1) or (typeID == 8) or (typeID == 9) or (typeID == 10)) {
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
    } */

        int16 x=0, y=0, z=0;
        DunGroupData grp;
        auto roomRange = sDunDataMgr.rooms.equal_range(roomID);
        for (auto it = roomRange.first; it != roomRange.second; ++it) {
            x = it->second.x;
            y = it->second.y;
            z = it->second.z;
            auto groupRange = sDunDataMgr.groups.equal_range(it->second.dunGroupID);
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
    if (sig.dungeonType == EVEDUNG::dunTypes::typeGravimetric) {
        // dungeon template for grav sites just give 'extra' roid data
        m_system->GetBeltMgr()->Create(sig, m_anomalyItems);
        // clear out extra roids to continue with room deco
        m_anomalyItems.clear();
    }

    // create deco items for this dungeon
    CreateDeco(templateID, sig);

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

    // need to determine region sov, region rat or other here also
    int8 factionID = GetFactionID(sig.ownerID);

    switch (sig.dungeonType) {
        case dunTypes::typeGravimetric: {       // 2
            factionID = 8;  // rats per system
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
            // fix factionID after anomaly templates are finished
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
            // fix factionID after anomaly templates are finished
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
    }

    /* templateID format.  ABCDE
     *       A = sitetype - 1:mission, 2:grav, 3:mag, 4:radar, 5:ladar, 7:anomaly, 8:unrated, 9:ded/escalation
     *       B = type - anomaly security: 1=hi, 2=lo, 3=null, 4=mid, mission misc: 1 to 9
     *       C = subtype  - 2: 0 to 5, 7: 1 to 5, 1: 1 to 9, 3: 1 to 8, 5: 1 to 8
     *       D = level - 2: 1 to 3, 4: type 3, 2 levels, 7: 1 to 5, 1: 1 to 9
     *       E = faction - 0=code defined, 1=Serpentis, 2=Angel, 3=Blood, 4=Guristas, 5=Sansha, 6=Drones, 7=region sov , 8=region pirate , 9=other
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
    double level = MakeRandomFloat();
    _log(COSMIC_MGR__TRACE, "DungeonMgr::GetRandLevel() - level = %.2f", level);

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
struct CosmicSignature {
    std::string sigID;  // this is unique xxx-nnn id displayed in scanner
    std::string sigName;
    uint32 ownerID;
    uint32 systemID;
    uint32 sigItemID;   // itemID of this entry
    uint8 dungeonType;
    uint16 sigTypeID;
    uint16 sigGroupID;
    uint16 scanGroupID;
    uint16 scanAttributeID;
    double x;
    double y;
    double z;
};
*/
void DungeonMgr::CreateDeco(uint32 templateID, CosmicSignature& sig)
{
    /* templateID format.  ABCDE
     *       A = site - 1:mission, 2:grav, 3:mag, 4:radar, 5:ladar, 7:anomaly, 8:unrated, 9:ded/escalation
     *       B = security: 1=hi, 2=lo, 3=null, 4=mid, mission misc: 1 to 9
     *       C = subtype  - 2: 0 to 5, 7: 1 to 5, 1: 1 to 9, 3: 1 to 8, 5: 1 to 8
     *       D = level (size) -  0:code defined 1:small(1-5), 2:medium(2-10), 3:large(5-25), 4:enormous(10-50), 5:colossal(20-100), 6-9:ice
     *       E = faction - 0=code defined, 1=Serpentis, 2=Angel, 3=Blood, 4=Guristas, 5=Sansha, 6=Drones, 7=region sov , 8=region pirate , 9=other
     */

    // templateID = (sig.dungeonType *10000) + (type *1000) + (subType *100) + (level *10) + factionID;

    uint8 factionID = templateID % 10;
    uint8 size = templateID / 10 % 10;
    uint8 subType = templateID / 100 % 10;
    uint8 type = templateID / 1000 % 10;

    uint16 groupID = 0, radius = 0;

    // create groupIDs for this dungeon, and add to vector
    std::vector<uint16> groupVec;
    groupVec.clear();

    if (sig.dungeonType == EVEDUNG::dunTypes::typeMission) {    //1
    }
    if (sig.dungeonType == EVEDUNG::dunTypes::typeGravimetric) {    //2
        groupVec.push_back(130);    //named roids
        groupVec.push_back(131);    //misc roids
        groupVec.push_back(132);    //worthless mining types
        groupVec.push_back(160);    //asteroid colony items
        groupVec.push_back(620);    // Starbase
        groupVec.push_back(630);    // Habitation
        groupVec.push_back(691);    // misc
    }
    if (sig.dungeonType == EVEDUNG::dunTypes::typeMagnetometric) {  //3
        groupVec.push_back(620);    // Starbase
        groupVec.push_back(630);    // Habitation
        groupVec.push_back(640);    // Stationary
        groupVec.push_back(650);    // Indestructible
        groupVec.push_back(660);    // Forcefield
        groupVec.push_back(670);    // Shipyard
        groupVec.push_back(680);    // Construction
        groupVec.push_back(690);    // Storage
        groupVec.push_back(691);    // misc
    }
    if (sig.dungeonType == EVEDUNG::dunTypes::typeRadar) {  //4
        groupVec.push_back(130);    //named roids
        groupVec.push_back(131);    //misc roids
        groupVec.push_back(160);    //asteroid colony items
        groupVec.push_back(630);    // Habitation
        groupVec.push_back(640);    // Stationary
    }
    if (sig.dungeonType == EVEDUNG::dunTypes::typeLadar) {  //5
        groupVec.push_back(132);    //worthless mining types
        groupVec.push_back(160);    //asteroid colony items
        groupVec.push_back(670);    // Shipyard
        groupVec.push_back(680);    // Construction
        groupVec.push_back(690);    // Storage
    }
    if (sig.dungeonType == EVEDUNG::dunTypes::typeAnomaly) {    //7
        groupVec.push_back(430);    //lco misc
        groupVec.push_back(431);    //lco Habitation
        groupVec.push_back(432);    //lco drug labs
        groupVec.push_back(433);    //lco Starbase
        groupVec.push_back(691);    // misc
    }
    if (sig.dungeonType == EVEDUNG::dunTypes::typeUnrated) {    //8
        // this will need faction shit
    }
    if (sig.dungeonType == EVEDUNG::dunTypes::typeEscalation) { //9
        // this will need faction shit
    }
    if (sig.dungeonType == EVEDUNG::dunTypes::typeDED_Complex) {    //10
        // this will need faction shit
    }
    int8 step = 0;
    uint16 count = 0, amount = 0, pos = 10000;
    pos *= size;

    double theta = 0;
    // range is 0.1 for 1.0 system to 2.0 for -0.9 system
    size *= ((1.1 - m_system->GetSystemSecurityRating()) /2 *10);
    uint8 origSize = size;
    DunGroupData grp;
    for (auto cur : groupVec) {
        size = origSize;
        count = sDunDataMgr.groups.count(cur);
        if (count < 1)
            continue;
        else if (count < size)
            size = count;
        _log(COSMIC_MGR__MESSAGE, "DungeonMgr::CreateDeco() - Adding Deco group %u for type %u, subtype %u, size %u, faction %u", cur, sig.dungeonType, subType, size, factionID);

        auto groupRange = sDunDataMgr.groups.equal_range(cur);
        auto it = groupRange.first;

        for (uint8 i=0; i < size; ++i) {
            // site size and item radius determine position
            step = MakeRandomInt(1,count);
            std::advance(it,step);
            radius = it->second.radius;
            grp.typeCatID = it->second.typeCatID;
            grp.typeGrpID = it->second.typeGrpID;
            grp.typeName = it->second.typeName;
            grp.typeID = it->second.typeID;
            if (sig.dungeonType == EVEDUNG::dunTypes::typeGravimetric) {
                theta = MakeRandomFloat(0, (EvE_Pi*2));
                grp.x = (pos + radius * cos(theta)) * (IsEven(MakeRandomInt(0,10)) ? -1 : 1);
                grp.z = (pos + radius * sin(theta)) * -1;
                grp.y = MakeRandomFloat(-radius, radius);
            } else if (IsEven(MakeRandomInt(0,10))) {
                grp.x = (pos + it->second.x + (radius*2)) * (IsEven(MakeRandomInt(0,10)) ? -1 : 1);
                grp.z = pos + it->second.z + radius;
            } else {
                grp.x = (pos + it->second.x + radius) * -1;
                grp.z = (pos + it->second.z + (radius*2)) * -1;
            }
            grp.y = it->second.y + MakeRandomInt(-2500, radius *2);
            m_anomalyItems.push_back(grp);
            it = groupRange.first;
        }
    }
}


/* groupID format
      ABB - item def groups
   A = group type - 1:deco, 2:system effect beacon, 3:mining, 4:lco, 5:ships, 6:base, 7:station gun, 8:station wrecks, 9:misc
   B =  1:space objects, 2:effect beacons, 3:roid types, 4:ice types, 5:, 6:asteroid colony, 7:, 8:, 9:misc
   B = ship/gun faction:  1:Amarr, 2:Caldari, 3:Gallente, 4:Minmatar, 5:Sentinel, 6:Guardian
   B =  faction:  00:none, 01:Serpentis, 02:Angel, 03:Blood, 04:Guristas, 05:Sansha, 06:Drones, 07:Amarr, 08:Caldari, 09:Gallente,
                  10:Minmatar, 11:Sleeper, 12:Talocan, 13:Ammatar

1xx  deco items
110  wormholes
13x  mining types
130  named roids
131  misc roids
132  worthless mining types
140  infested items
160  Asteroid Colony
191  Monument

2xx  effect beacons
211 Electronic
212 omni
22x Incursion
23x Black Hole
24x Magnetar
25x Pulsar
26x Red Giant
27x Wolf Rayet
28x Cataclysmic Variable

3xx  mining objects
30x  ore
34x  ice
36x  clouds

4xx lco
401 Serpentis
402 Angel
403 Blood
404 Guristas
405 Sansha
406 drone
407 Amarr
408 Caldari
409 Gallente
410 Minmatar
42x lco ships
43x lco structures
430 lco misc
431 lco Habitation
432 lco drug labs
433 lco Starbase

5xx ships
51x Amarr
52x Caldari
53x Gallente
54x Minmatar

6xx base
601 Serpentis
602 Angel
603 Blood
604 Guristas
605 Sansha
606 drone
607 Amarr
608 Caldari
609 Gallente
610 Minmatar
611 Sleeper
612 Talocan
613 Ammatar

620 Starbase
630 Habitation
640 Stationary
650 Indestructible
660 Forcefield
670 Shipyard
680 Construction
690 Storage
691 misc

7xx station guns

8xx station and structure ruins
80x  Sansha
81x  Amarr
82x  Caldari
83x  Gallente
84x  Minmatar
85x  misc ruined parts
86x  misc debris

9xx misc
91x comets
92x clouds
93x environment
96x event lco/lcs
*/

    /*
    In player-owned sovereign nullsec, using Ore Prospecting Arrays,
    (23510,'Small Asteroid Cluster',0,2,0,1,0,0,0),
    (23520,'Moderate Asteroid Cluster',0,2,0,15,0,0,0),
    (23530,'Large Asteroid Cluster',0,2,0,29,0,0,0),
    (23540,' Enormous Asteroid Cluster ',0,2,0,29,0,0,0),
    (23550,'Colossal Asteroid Cluster',0,2,0,29,0,0,0),
    */