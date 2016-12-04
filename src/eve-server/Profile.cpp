/**
 * @name Profile.cpp
 *   lightweight profiling code specifically for timing sections of running EvEmu application
 *   this code is very basic, and very specific.
 * @Author:         Allan
 * @date:   13 April 2015
 */

#include "Profile.h"


Profile::Profile() { }

Profile::~Profile() {
    ClearAll();
}

void Profile::Init() {
    ClearAll();
    sLog.Success( "       ServerInit", "Profiling initialized." );
}

void Profile::AddTime(uint8 key, double value) {
    if (!value) return;
    /*
    _destinyProfile     = 1,    //*
    _mapProfile         = 2,    //
    _clientProfile      = 3,    //*
    _npcProfile         = 4,    //*
    _bubblesProfile     = 5,    //*
    _itemsProfile       = 6,    //
    _modulesProfile     = 7,    //*
    _functionsProfile   = 8,    //
    _dbProfile          = 9,    //
    _shipProfile        = 10,   //*
    _targetsProfile     = 11,   //
    _serverProfile      = 12,   //
    _missileProfile     = 13,   //
    _entityCProfile     = 14,   //*
    _entitySProfile     = 15,   //*
    _lootProfile        = 16,   //*
    _salvageProfile     = 17,   //
    _spawnProfile       = 18,   //*
    _collisionProfile   = 19,   //*
    _droneProfile       = 20,   //*
    _itemloadProfile    = 21,   //*
    _concordProfile     = 22,   //*
    _colonyProfile      = 23    //*
    */
    switch(key) {
        case 1:
            m_destiny.push_back(value);
            break;
        case 2:
            m_map.push_back(value);
            break;
        case 3:
            m_client.push_back(value);
            break;
        case 4:
            m_npc.push_back(value);
            break;
        case 5:
            m_bubbles.push_back(value);
            break;
        case 6:
            m_items.push_back(value);
            break;
        case 7:
            m_modules.push_back(value);
            break;
        case 8:
            m_functions.push_back(value);
            break;
        case 9:
            m_db.push_back(value);
            break;
        case 10:
            m_ship.push_back(value);
            break;
        case 11:
            m_targets.push_back(value);
            break;
        case 12:
            m_server.push_back(value);
            break;
        case 13:
            m_missile.push_back(value);
            break;
        case 14:
            m_entityC.push_back(value);
            break;
        case 15:
            m_entityS.push_back(value);
            break;
        case 16:
            m_loot.push_back(value);
            break;
        case 17:
            m_salvage.push_back(value);
            break;
        case 18:
            m_spawn.push_back(value);
            break;
        case 19:
            m_collision.push_back(value);
            break;
        case 20:
            m_drone.push_back(value);
            break;
        case 21:
            m_itemload.push_back(value);
            break;
        case 22:
            m_concord.push_back(value);
            break;
        case 23:
            m_colony.push_back(value);
            break;
        default:
            sLog.Error("Profile::AddTime()", "Default reached on key %u.", key );
            break;
    }
}

void Profile::ClearAll()
{
    m_server.clear();
    m_destiny.clear();
    m_map.clear();
    m_client.clear();
    m_npc.clear();
    m_bubbles.clear();
    m_items.clear();
    m_modules.clear();
    m_functions.clear();
    m_db.clear();
    m_ship.clear();
    m_targets.clear();
    m_missile.clear();
    m_entityC.clear();
    m_entityS.clear();
    m_loot.clear();
    m_salvage.clear();
    m_spawn.clear();
    m_collision.clear();
    m_drone.clear();
    m_itemload.clear();
    m_concord.clear();
    m_colony.clear();
}

void Profile::PrintProfile()
{
    double startTime = GetTimeUSeconds();
    double h = 0, l = 0, a = 0;
    GetRunTimes(m_server, &h, &l, &a);
    sLog.Success("   Server Profile", " *Main() called %u times. Hi: %fus, Lo: %fus, Avg: %fus.", m_server.size(), h, l, a );
    GetRunTimes(m_destiny, &h, &l, &a);
    sLog.Success("   Server Profile", " Destiny called %u times. Hi: %fus, Lo: %fus, Avg: %fus.", m_destiny.size(),  h, l, a );
    GetRunTimes(m_entityC, &h, &l, &a);
    sLog.Success("   Server Profile", " EntityClient called %u times. Hi: %fus, Lo: %fus, Avg: %fus.", m_entityC.size(),  h, l, a );
    GetRunTimes(m_entityS, &h, &l, &a);
    sLog.Success("   Server Profile", " EntitySystem called %u times. Hi: %fus, Lo: %fus, Avg: %fus.", m_entityS.size(),  h, l, a );
    GetRunTimes(m_map, &h, &l, &a);
    sLog.Success("   Server Profile", " *Map called %u times. Hi: %fus, Lo: %fus, Avg: %fus.", m_map.size(), h, l, a );
    GetRunTimes(m_client, &h, &l, &a);
    sLog.Success("   Server Profile", " Client called %u times. Hi: %fus, Lo: %fus, Avg: %fus.", m_client.size(),  h, l, a );
    GetRunTimes(m_npc, &h, &l, &a);
    sLog.Success("   Server Profile", " NPC called %u times. Hi: %fus, Lo: %fus, Avg: %fus.", m_npc.size(),  h, l, a );
    GetRunTimes(m_bubbles, &h, &l, &a);
    sLog.Success("   Server Profile", " Bubbles called %u times. Hi: %fus, Lo: %fus, Avg: %fus.", m_bubbles.size(),  h, l, a );
    GetRunTimes(m_items, &h, &l, &a);
    sLog.Success("   Server Profile", " *Items called %u times. Hi: %fus, Lo: %fus, Avg: %fus.", m_items.size(),  h, l, a );
    GetRunTimes(m_itemload, &h, &l, &a);
    sLog.Success("   Server Profile", " Item Loading called %u times. Hi: %fus, Lo: %fus, Avg: %fus.", m_itemload.size(),  h, l, a );
    GetRunTimes(m_modules, &h, &l, &a);
    sLog.Success("   Server Profile", " Modules called %u times. Hi: %fus, Lo: %fus, Avg: %fus.", m_modules.size(),  h, l, a );
    GetRunTimes(m_functions, &h, &l, &a);
    sLog.Success("   Server Profile", " *Functions called %u times. Hi: %fus, Lo: %fus, Avg: %fus.", m_functions.size(), h, l, a );
    GetRunTimes(m_db, &h, &l, &a);
    sLog.Success("   Server Profile", " *DB called %u times. Hi: %fus, Lo: %fus, Avg: %fus.", m_db.size(),  h, l, a );
    GetRunTimes(m_ship, &h, &l, &a);
    sLog.Success("   Server Profile", " Ship called %u times. Hi: %fus, Lo: %fus, Avg: %fus.", m_ship.size(),  h, l, a );
    GetRunTimes(m_missile, &h, &l, &a);
    sLog.Success("   Server Profile", " Missile called %u times. Hi: %fus, Lo: %fus, Avg: %fus.", m_missile.size(), h, l, a );
    GetRunTimes(m_loot, &h, &l, &a);
    sLog.Success("   Server Profile", " Loot called %u times. Hi: %fus, Lo: %fus, Avg: %fus.", m_loot.size(), h, l, a );
    GetRunTimes(m_salvage, &h, &l, &a);
    sLog.Success("   Server Profile", " Salvage called %u times. Hi: %fus, Lo: %fus, Avg: %fus.", m_salvage.size(), h, l, a );
    GetRunTimes(m_spawn, &h, &l, &a);
    sLog.Success("   Server Profile", " Spawn called %u times. Hi: %fus, Lo: %fus, Avg: %fus.", m_spawn.size(), h, l, a );
    GetRunTimes(m_collision, &h, &l, &a);
    sLog.Success("   Server Profile", " Collisions called %u times. Hi: %fus, Lo: %fus, Avg: %fus.", m_collision.size(), h, l, a );
    GetRunTimes(m_drone, &h, &l, &a);
    sLog.Success("   Server Profile", " Drones called %u times. Hi: %fus, Lo: %fus, Avg: %fus.", m_drone.size(), h, l, a );
    GetRunTimes(m_concord, &h, &l, &a);
    sLog.Success("   Server Profile", " Concord called %u times. Hi: %fus, Lo: %fus, Avg: %fus.", m_concord.size(), h, l, a );
    GetRunTimes(m_colony, &h, &l, &a);
    sLog.Success("   Server Profile", " Colony Updates called %u times. Hi: %fus, Lo: %fus, Avg: %fus.", m_colony.size(), h, l, a );
    sLog.Success("   Server Profile", " Profile Times Compiled in %fus.", (GetTimeUSeconds() -startTime) );
}

double Profile::GetAverage(std::vector< double > container)
{
    uint32 size = container.size();
    if (!size) return 0;

    double total = 0.0;
    for (uint32 i = 0; i < size; ++i) {
        total += container.at(i);
    }
    return total /size;
}

void Profile::GetRunTimes(std::vector< double > container, double* h, double* l, double* a)
{
    uint32 size = container.size();
    if (!size) {
        *h = 0;
        *l = 0;
        *a = 0;
        return;
    }

    double total = 0.0, lo = 0.0, hi = 0.0;
    for (uint32 i = 0; i < size; ++i) {
        total += container.at(i);
        if ((lo > container.at(i)) || (lo < 0.000001)) lo = container.at(i);
        if (hi < container.at(i)) hi = container.at(i);
    }

    *h = hi;
    *l = lo;
    *a = total /size;
}

