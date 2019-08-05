/**
 * @name Profile.cpp
 *   lightweight profiling code specifically for timing sections of running EvEmu application
 *   this code is very basic, and very specific.
 * @Author:         Allan
 * @date:   13 April 2015
 */

#include "Profile.h"
#include "EVEServerConfig.h"
#include "../eve-core/utils/misc.h"

Profile::Profile() { }

Profile::~Profile() {
    ClearAll();
}

int Profile::Initialize() {
    ClearAll();
    sLog.Blue("  Profile Manager", "Profiling initialized.");
    return 1;
}

void Profile::AddTime(uint8 key, double value) {
    if (value < 0.0001)
        return;
    if ((sConfig.debug.ProfileTraceTime > 0) and (value > sConfig.debug.ProfileTraceTime *1000)) {
        sLog.Warning("  Profile Manager", "Long Profile Time on key %s, time %.3f.", GetKeyName(key).c_str(), value);
        EvE::traceStack();
    }
    /*
    _destinyProfile     = 1,    *
    _mapProfile         = 2,
    _clientProfile      = 3,    *
    _npcProfile         = 4,    *
    _bubblesProfile     = 5,    *
    _itemsProfile       = 6,
    _modulesProfile     = 7,    *
    _functionsProfile   = 8,
    _dbProfile          = 9,    *
    _shipProfile        = 10,   *
    _targetsProfile     = 11,
    _serverProfile      = 12,
    _missileProfile     = 13,
    _systemProfile      = 14,
    _entitySProfile     = 15,   *
    _lootProfile        = 16,   *
    _salvageProfile     = 17,
    _spawnProfile       = 18,   *
    _collisionProfile   = 19,   *
    _droneProfile       = 20,   *
    _itemloadProfile    = 21,   *
    _concordProfile     = 22,   *
    _colonyProfile      = 23,   *
    _damageProfile      = 24
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
            m_system.push_back(value);
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
        case 24:
            m_damage.push_back(value);
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
    m_system.clear();
    m_entityS.clear();
    m_loot.clear();
    m_salvage.clear();
    m_spawn.clear();
    m_collision.clear();
    m_drone.clear();
    m_itemload.clear();
    m_concord.clear();
    m_colony.clear();
    m_damage.clear();
}

void Profile::PrintProfile()
{
    /** @todo figure out how to color this based on times....R,Y,G,M,B,W  */

    double startTime = GetTimeUSeconds();
    double h = 0, l = 0, a = 0;
    sLog.Green("   Server Profile", " Current Process Profile times for this run:");
    /* not used yet.
    GetRunTimes(m_server, h, l, a);
    std::printf("        *Main()  %u times.   \tHi: %.2f   \tLo: %.2fus   \tAvg: %.4fus\n", m_server.size(), h, l, a );
    GetRunTimes(m_map, h, l, a);
    std::printf("          *Map   %u times.   \tHi: %.2f   \tLo: %.2fus   \tAvg: %.4fus\n", m_map.size(), h, l, a );
    GetRunTimes(m_items, h, l, a);
    std::printf("        *Items   %u times.   \tHi: %.2f   \tLo: %.2fus   \tAvg: %.4fus\n", m_items.size(),  h, l, a );
    GetRunTimes(m_functions, h, l, a);
    std::printf("    *Functions   %u times.   \tHi: %.2f   \tLo: %.2fus   \tAvg: %.4fus\n", m_functions.size(), h, l, a );
    GetRunTimes(m_concord, h, l, a);
    std::printf("       Concord   %u times.   \tHi: %.2f   \tLo: %.2fus   \tAvg: %.4fus\n", m_concord.size(), h, l, a );
    */

    GetRunTimes(m_db, h, l, a);
    std::printf("            DB   %s times.   \tHi: %.2fus   \tLo: %.2fus   \tAvg: %.4fus\n", GetSize(m_db.size()),  h, l, a );
    GetRunTimes(m_entityS, h, l, a);
    std::printf("    EntityList   %s times.   \tHi: %.2fus   \tLo: %.2fus   \tAvg: %.4fus\n", GetSize(m_entityS.size()),  h, l, a );
    GetRunTimes(m_client, h, l, a);
    std::printf("        Client   %s times.   \tHi: %.2fus   \tLo: %.2fus   \tAvg: %.4fus\n", GetSize(m_client.size()),  h, l, a );
    GetRunTimes(m_system, h, l, a);
    std::printf("     SystemMgr   %s times.   \tHi: %.2fus   \tLo: %.2fus   \tAvg: %.4fus\n", GetSize(m_system.size()),  h, l, a );
    GetRunTimes(m_bubbles, h, l, a);
    std::printf("       Bubbles   %s times.   \tHi: %.2fus   \tLo: %.2fus   \tAvg: %.4fus\n", GetSize(m_bubbles.size()),  h, l, a );
    GetRunTimes(m_destiny, h, l, a);
    std::printf("       Destiny   %s times.   \tHi: %.2fus   \tLo: %.2fus   \tAvg: %.4fus\n", GetSize(m_destiny.size()),  h, l, a );
    GetRunTimes(m_npc, h, l, a);
    std::printf("           NPC   %s times.   \tHi: %.2fus   \tLo: %.2fus   \tAvg: %.4fus\n", GetSize(m_npc.size()),  h, l, a );
    GetRunTimes(m_itemload, h, l, a);
    std::printf("  Item Loading   %s times.   \tHi: %.2fus   \tLo: %.2fus   \tAvg: %.4fus\n", GetSize(m_itemload.size()),  h, l, a );
    GetRunTimes(m_modules, h, l, a);
    std::printf("       Modules   %s times.   \tHi: %.2fus   \tLo: %.2fus   \tAvg: %.4fus\n", GetSize(m_modules.size()),  h, l, a );
    GetRunTimes(m_ship, h, l, a);
    std::printf("          Ship   %s times.   \tHi: %.2fus   \tLo: %.2fus   \tAvg: %.4fus\n", GetSize(m_ship.size()),  h, l, a );
    GetRunTimes(m_missile, h, l, a);
    std::printf("       Missile   %s times.   \tHi: %.2fus   \tLo: %.2fus   \tAvg: %.4fus\n", GetSize(m_missile.size()), h, l, a );
    GetRunTimes(m_damage, h, l, a);
    std::printf("        Damage   %s times.   \tHi: %.2fus   \tLo: %.2fus   \tAvg: %.4fus\n", GetSize(m_damage.size()), h, l, a );
    GetRunTimes(m_loot, h, l, a);
    std::printf("          Loot   %s times.   \tHi: %.2fus   \tLo: %.2fus   \tAvg: %.4fus\n", GetSize(m_loot.size()), h, l, a );
    GetRunTimes(m_salvage, h, l, a);
    std::printf("       Salvage   %s times.   \tHi: %.2fus   \tLo: %.2fus   \tAvg: %.4fus\n", GetSize(m_salvage.size()), h, l, a );
    if (sConfig.npc.RoamingSpawns or sConfig.npc.StaticSpawns) {
        GetRunTimes(m_spawn, h, l, a);
        std::printf("        Spawns   %s times.   \tHi: %.2fus   \tLo: %.2fus   \tAvg: %.4fus\n", GetSize(m_spawn.size()), h, l, a );
    } else
        std::printf("        Spawns   Disabled.\n");
    if (sConfig.cosmic.BumpEnabled) {
        GetRunTimes(m_collision, h, l, a);
        std::printf("    Collisions   %u times.   \tHi: %.2fus   \tLo: %.2fus   \tAvg: %.4fus\n", GetSize(m_collision.size()), h, l, a );
    } else
        std::printf("    Collisions   Disabled.\n");

    if (sConfig.testing.EnableDrones) {
        GetRunTimes(m_drone, h, l, a);
        std::printf("        Drones   %s times.   \tHi: %.2fus   \tLo: %.2fus   \tAvg: %.4fus\n", GetSize(m_drone.size()), h, l, a );
    } else
        std::printf("        Drones   Disabled.\n");

    if (sConfig.cosmic.PIEnabled) {
        GetRunTimes(m_colony, h, l, a);
        std::printf("        Colony   %s times.   \tHi: %.2fus   \tLo: %.2fus   \tAvg: %.4fus\n", GetSize(m_colony.size()), h, l, a );
    } else
        std::printf("        Colony   Disabled.\n");

    std::printf(" Profile Times Compiled in %s\n", (GetTimeUSeconds() -startTime) );
}

void Profile::GetRunTimes(std::vector< double >& container, double& h, double& l, double& a)
{
    uint32 size = container.size();
    if (!size) {
        h = 0;
        l = 0;
        a = 0;
        return;
    }

    double total = 0.0, lo = 0.0, hi = 0.0;
    for (uint32 i = 0; i < size; ++i) {
        total += container.at(i);
        if ((lo > container.at(i)) or (lo < 0.000001))
            lo = container.at(i);
        if (hi < container.at(i))
            hi = container.at(i);
    }

    h = hi;
    l = lo;
    a = total /size;
    /*
    if (hi > 1000000) {
        h = itoa(hi /1000000);
        h += "s";
    } else if (hi > 1000) {
        h = itoa(hi /1000);
        h = "ms";
    } else {
        h = hi;
        h += "us";
    }
    */
}

const char* Profile::GetSize(size_t cSize)
{
    if (cSize > 999999) {
        std::string ret = itoa(cSize /1000000);
        ret += "m";
        return ret.c_str();
    } else if (cSize > 9999) {
        std::string ret = itoa(cSize /1000);
        ret += "k";
        return ret.c_str();
    }
    return itoa(cSize);
}

std::string Profile::GetKeyName(uint8 key)
{
    switch (key) {
        case _destinyProfile:       return "Destiny";   //  1,
        case _mapProfile:           return "Map";       //  2,
        case _clientProfile:        return "Client";    //  3,
        case _npcProfile:           return "NPC";       //  4,
        case _bubblesProfile:       return "Bubble";    //  5,
        case _itemsProfile:         return "Item";      //  6,
        case _modulesProfile:       return "Module";    //  7,
        case _functionsProfile:     return "Function";  //  8,
        case _dbProfile:            return "DB";        //  9,
        case _shipProfile:          return "Ship";      //  10,
        case _targetsProfile:       return "Target";    //  11,
        case _serverProfile:        return "Server";    //  12,
        case _missileProfile:       return "Missile";   //  13,
        case _systemProfile:        return "System";    //  14,
        case _entitySProfile:       return "EntityTic"; //  15,
        case _lootProfile:          return "Loot";      //  16,
        case _salvageProfile:       return "Salvage";   //  17,
        case _spawnProfile:         return "Spawn";     //  18,
        case _collisionProfile:     return "Collision"; //  19,
        case _droneProfile:         return "Drone";     //  20,
        case _itemloadProfile:      return "ItemLoad";  //  21,
        case _concordProfile:       return "Concord";   //  22,
        case _colonyProfile:        return "Colony";    //  23,
        case _damageProfile:        return "Damage";    //  24
    }
}

/*  color shit....
 *
    ::fputs( COLOR_TABLE[ color ], stdout );
    std::printf(*message*);
    *


    const char* const NewLog::COLOR_TABLE[ COLOR_COUNT ] =
    {
    "\033[" "00"    "m", // COLOR_DEFAULT
    "\033[" "30;22" "m", // COLOR_BLACK
    "\033[" "31;22" "m", // COLOR_RED
    "\033[" "32;22" "m", // COLOR_GREEN
    "\033[" "33;01" "m", // COLOR_YELLOW
    "\033[" "34;01" "m", // COLOR_BLUE
    "\033[" "35;22" "m", // COLOR_MAGENTA
    "\033[" "36;01" "m", // COLOR_CYAN
    "\033[" "37;01" "m"  // COLOR_WHITE
    };

    */

