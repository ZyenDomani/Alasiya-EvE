/**
 * @name Profile.h
 *   lightweight profiling code specifically for timing sections of running EvEmu application
 *   this code is very basic, and very specific.
 * @Author:         Allan
 * @date:   13 April 2015
 */

/**   Allan's EvEmu Profiler
 * simple singleton profiler using std::vector<double> being simple mem object storage.
 * vector object denotes call type, (db, client, map, etc.)
 * double precision float holds elapsed time for that particular call
 * output functions use size() to count calls, and give readouts as
 *     CALL_TYPE: called N times, hi: Xus, lo: Yus, avg: Zus
 *  Times are measured in microseconds via GetTimeUSeconds() from core/utils/utils_time.cpp
 *
 */


#ifndef EVEMU_EVESERVER_PROFILER_H_
#define EVEMU_EVESERVER_PROFILER_H_


#include "eve-common.h"

typedef enum {          // implemented?  (* = yes)
    destinyProfile     = 1,    //*
    mapProfile         = 2,    //
    clientProfile      = 3,    //*
    npcProfile         = 4,    //*
    bubblesProfile     = 5,    //*
    itemsProfile       = 6,    //
    modulesProfile     = 7,    //*
    functionsProfile   = 8,    //
    dbProfile          = 9,    //
    shipProfile        = 10,   //*
    targetsProfile     = 11,   //
    serverProfile      = 12,   //
    missileProfile     = 13,   //
    systemProfile      = 14,   //*
    entitySProfile     = 15,   //*
    lootProfile        = 16,   //*
    salvageProfile     = 17,   //
    spawnProfile       = 18,   //*
    collisionProfile   = 19,   //*
    droneProfile       = 20,   //*
    itemloadProfile    = 21,   //*
    concordProfile     = 22,   //*
    colonyProfile      = 23,   //*
    damageProfile      = 24,
    parseFXProfile     = 25,
    applyFXProfile     = 26,
    onTargProfile      = 27
} profile;

class Profile
: public Singleton<Profile>
{
public:
    Profile();
    ~Profile();

    int Initialize();

    void AddTime(uint8 key, double value);
    void PrintProfile();
    void ClearAll();

    std::string GetSize(size_t cSize);

    void GetRunTimes(std::vector< double >& container, double& h, double& l, double& a);

protected:
    std::string GetKeyName(uint8 key);

private:
    std::vector<double> m_server;
    std::vector<double> m_functions;
    std::vector<double> m_db;
    std::vector<double> m_client;
    std::vector<double> m_map;
    std::vector<double> m_destiny;
    std::vector<double> m_system;
    std::vector<double> m_entityS;
    std::vector<double> m_npc;
    std::vector<double> m_drone;
    std::vector<double> m_bubbles;
    std::vector<double> m_items;
    std::vector<double> m_itemload;
    std::vector<double> m_modules;
    std::vector<double> m_ship;
    std::vector<double> m_targets;
    std::vector<double> m_ontarget;
    std::vector<double> m_missile;
    std::vector<double> m_loot;
    std::vector<double> m_salvage;
    std::vector<double> m_spawn;
    std::vector<double> m_collision;
    std::vector<double> m_concord;
    std::vector<double> m_colony;
    std::vector<double> m_damage;
    std::vector<double> m_effects1;
    std::vector<double> m_effects2;
};

#define sProfile \
    ( Profile::get() )

#endif  // EVEMU_EVESERVER_PROFILER_H_