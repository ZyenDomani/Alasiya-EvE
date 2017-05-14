
 /**
  * @name AnomalyMgr.h
  *     Anomaly managment system for Alasiya EvEmu
  *
  * @Author:        Allan
  * @date:          12 December 2015
  *
  */
 
#ifndef EVEMU_SYSTEM_ANOMALYMGR_H_
#define EVEMU_SYSTEM_ANOMALYMGR_H_

/*  this class is in charge of creating/destroying and maintaining
 * anomalies in a system.
 *
 *  a new iteration of this class is created for each system as that system is booted.
 */

#include "system/cosmicMgrs/ManagerDB.h"


class DungeonMgr;
class AsteroidBeltMgr;
class PyServiceMgr;
class SpawnMgr;
class SystemManager;

class AnomalyMgr
{
  public:
      AnomalyMgr(SystemManager* mgr, PyServiceMgr& svc);
      virtual ~AnomalyMgr()                             { /* do nothing here */ }

      void Init(AsteroidBeltMgr* beltMgr, DungeonMgr* dungMgr, SpawnMgr* spawnMgr);
      void Process();

      void SaveAnomaly();

protected:
    /* we do not own any of these */
    ManagerDB m_db;

private:
    /* we do not own any of these */
    AsteroidBeltMgr* m_beltMgr;
    DungeonMgr* m_dungMgr;
    SpawnMgr* m_spawnMgr;
    SystemManager* m_system;
    PyServiceMgr& m_services;

    bool m_initalized;

};

#endif  // EVEMU_SYSTEM_ANOMALYMGR_H_

