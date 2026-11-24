/**
 * @name Profile.cpp
 *   lightweight profiling code specifically for timing sections of running EvEmu application
 *   this code is very basic, and very specific.
 * @Author:         Allan
 * @date:   13 April 2015
 */

#include "Profiler.h"
#include "EVEServerConfig.h"
#include "../eve-core/utils/misc.h"


void Profile::Initialize() {
    sLog.Blue("  Profile Manager", "Profiling initialized.");
}

void Profile::ClearAll() {
    for (uint8 i = 0; i < Profile::Type::Max; ++i)
        m_stats[i].Reset();
}


void Profile::PrintProfile() {
    double startTime = GetTimeUSeconds();
    sLog.Green("   Server Profile", " Current Process Profile times for this run:");

    std::printf("\t\tProcess Loop Calls\n");
    PrintLine("EntMgr Tic",     Profile::Type::entityS);
    PrintLine("Client Network", Profile::Type::client);
    PrintLine("SystemMgr",      Profile::Type::system);
    PrintLine("Bubbles",        Profile::Type::bubbles);
    PrintLine("BallPark",       Profile::Type::destiny);
    PrintLine("NPC",            Profile::Type::npc);
    PrintLine("Modules",        Profile::Type::modules);
    PrintLine("Ship",           Profile::Type::ship);
    PrintLine("TargetProc",     Profile::Type::targets);
    PrintLine("Missile",        Profile::Type::missile);
    PrintLine("Damage",         Profile::Type::damage);

    if (sConfig.npc.RoamingSpawns or sConfig.npc.StaticSpawns) {
        PrintLine("Spawns", Profile::Type::spawn);
    } else {
        std::printf("        Spawns   Disabled.\n");
    }

    if (sConfig.cosmic.BumpEnabled) {
        PrintLine("Collisions", Profile::Type::collision);
    } else {
        std::printf("    Collisions   Disabled.\n");
    }

    if (sConfig.testing.EnableDrones) {
        PrintLine("Drones", Profile::Type::drone);
    } else {
        std::printf("        Drones   Disabled.\n");
    }

    std::printf("\t\tPeriodic Calls\n");
    PrintLine("Item Loading",   Profile::Type::itemload);
    PrintLine("Client Calls",   Profile::Type::clientCall);
    PrintLine("DB",             Profile::Type::db);
    PrintLine("Parse Effects",  Profile::Type::parseFX);
    PrintLine("Apply Effects",  Profile::Type::applyFX);
    PrintLine("Loot",           Profile::Type::loot);
    PrintLine("Salvage",        Profile::Type::salvage);

    if (sConfig.cosmic.PIEnabled) {
        PrintLine("Colony", Profile::Type::colony);
    } else {
        std::printf("        Colony   Disabled.\n");
    }

    if (sConfig.crime.Enabled) {
        PrintLine("Concord", Profile::Type::concord);
    } else {
        std::printf("       Concord   Disabled.\n");
    }

    if (sConfig.cosmic.CiviliansEnabled) {
        PrintLine("Civilian Ships", Profile::Type::civilians);
    } else {
        std::printf("Civilian Ships   Disabled.\n");
    }

    std::printf(" Profile Times Compiled in %.2fus\n", (GetTimeUSeconds() - startTime));
}

void Profile::PrintStartUpData() {
    double startTime = GetTimeUSeconds();
    sLog.Green("   Server Profile", " Current Process Profile times for this run:");

    PrintLine("DB",           Profile::Type::db);
    PrintLine("Item Loading", Profile::Type::itemload);

    std::printf(" Profile Times Compiled in %.2fus\n", (GetTimeUSeconds() - startTime));
}

void Profile::PrintLine(const char* label, uint8 key) {
    if (key >= Profile::Type::Max)
        return;
    const Stat& s = m_stats[key];
    std::string fSize;
    GetSize(static_cast<size_t>(s.count), fSize);

    double a = (s.count > 0) ? (s.total / static_cast<double>(s.count)) : 0.0;
    std::printf("%14s   %4s times.   \tHi: %10.2fus   \tLo: %8.3fus   \tAvg: %8.2fus\n",
                label, fSize.c_str(), s.hi, s.lo, a);
    if (s.over > 0)
        std::printf("%14s: %u over 1s.\n", label, s.over);
}

void Profile::GetSize(size_t cSize, std::string& ret)
{
    if (cSize > 999999) {
        ret = std::to_string(cSize / 1000000);
        ret += "m";
    } else if (cSize > 9999) {
        ret = std::to_string(cSize / 1000);
        ret += "k";
    } else {
        ret = std::to_string(cSize);
    }
}

const char* Profile::GetKeyName(uint8 key) const {
    switch (key) {
        case Profile::Type::destiny:       return "Destiny";      //  1,
        case Profile::Type::map:           return "Map";          //  2,
        case Profile::Type::client:        return "Client";       //  3,
        case Profile::Type::npc:           return "NPC";          //  4,
        case Profile::Type::bubbles:       return "Bubble";       //  5,
        case Profile::Type::items:         return "Item";         //  6,
        case Profile::Type::modules:       return "Module";       //  7,
        case Profile::Type::functions:     return "Function";     //  8,
        case Profile::Type::db:            return "DB";           //  9,
        case Profile::Type::ship:          return "Ship";         //  10,
        case Profile::Type::targets:       return "Target";       //  11,
        case Profile::Type::server:        return "Server";       //  12,
        case Profile::Type::missile:       return "Missile";      //  13,
        case Profile::Type::system:        return "System";       //  14,
        case Profile::Type::entityS:       return "EntityTic";    //  15,
        case Profile::Type::loot:          return "Loot";         //  16,
        case Profile::Type::salvage:       return "Salvage";      //  17,
        case Profile::Type::spawn:         return "Spawn";        //  18,
        case Profile::Type::collision:     return "Collision";    //  19,
        case Profile::Type::drone:         return "Drone";        //  20,
        case Profile::Type::itemload:      return "ItemLoad";     //  21,
        case Profile::Type::concord:       return "Concord";      //  22,
        case Profile::Type::colony:        return "Colony";       //  23,
        case Profile::Type::damage:        return "Damage";       //  24,
        case Profile::Type::parseFX:       return "ParseFX";      //  25,
        case Profile::Type::applyFX:       return "ApplyFX";      //  26,
        case Profile::Type::onTarg:        return "OnTarget";     //  27,
        case Profile::Type::clientCall:    return "Client Calls"; //  28,
        case Profile::Type::civilians:     return "Civilian";     //  29,
        case Profile::Type::entityP:       return "EntitySubTic"; //  30
        default:                           return "Invalid Key";
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

