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
    _systemProfile      = 14,   //*
    _entitySProfile     = 15,   //*
    _lootProfile        = 16,   //*
    _salvageProfile     = 17,   //
    _spawnProfile       = 18,   //*
    _collisionProfile   = 19,   //*
    _droneProfile       = 20,   //*
    _itemloadProfile    = 21,   //*
    _concordProfile     = 22,   //*
    _colonyProfile      = 23    //*
} profile;

class Profile
: public Singleton<Profile>
{
  public:
      Profile();
      virtual ~Profile();

      void Init();

      void AddTime(uint8 key, double value);
      void PrintProfile();
      void ClearAll();

      double GetAverage(std::vector<double> container);
      double GetTime() { return profile.time;}

      std::string GetSize(size_t cSize);

      void GetRunTimes(std::vector< double > container, double* h, double* l, double* a);

  private:
      struct {
          double time;
          std::string name;
      } profile;

      std::vector<double> m_server;
      std::vector<double> m_functions;
      std::vector<double> m_db;
      std::vector<double> m_client;
      std::vector<double> m_map;
      std::vector<double> m_destiny;
      std::vector<double> m_system;
      std::vector<double> m_entityS;
      std::vector<double> m_npc;
      std::vector<double> m_bubbles;
      std::vector<double> m_items;
      std::vector<double> m_itemload;
      std::vector<double> m_modules;
      std::vector<double> m_ship;
      std::vector<double> m_targets;
      std::vector<double> m_missile;
      std::vector<double> m_loot;
      std::vector<double> m_salvage;
      std::vector<double> m_spawn;
      std::vector<double> m_collision;
      std::vector<double> m_drone;
      std::vector<double> m_concord;
      std::vector<double> m_colony;
};

#define sProfile \
    ( Profile::get() )

#endif  // EVEMU_EVESERVER_PROFILER_H_