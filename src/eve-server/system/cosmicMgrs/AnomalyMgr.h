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
 * anomaly types in it's system.
 *
 *  a new iteration of this class is created for each system as that system is booted.
 */

#include "system/SystemGPoint.h"
#include "system/cosmicMgrs/ManagerDB.h"


class DungeonMgr;
class BeltMgr;
class PyServiceMgr;
class SpawnMgr;
class SystemManager;

class AnomalyMgr
{
  public:
      AnomalyMgr(SystemManager* mgr, PyServiceMgr& svc);
      virtual ~AnomalyMgr()                             { /* do nothing here */ }

      bool Init(BeltMgr* beltMgr, DungeonMgr* dungMgr, SpawnMgr* spawnMgr);
      void Process();

      void SaveAnomaly();
      void CreateAnomaly();
      void LoadAnomalies();

      void AddAnomaly(InventoryItemRef iRef);
      void GetAnomalyList(CosmicSignature& sig);

protected:
    ManagerDB m_mdb;
    ServiceDB m_sdb;
    SystemGPoint m_gp;

    int8 GetAnomalyType();

private:
    /* we do not own any of these */
    BeltMgr* m_beltMgr;
    DungeonMgr* m_dungMgr;
    SpawnMgr* m_spawnMgr;
    SystemManager* m_system;
    PyServiceMgr& m_services;

    Timer m_spawnTimer;
    Timer m_anomTimer;

    bool m_initalized;

    // internal data counters
    int8 m_Sigs;
    int8 m_Anoms;
    int8 m_WH;
    int8 m_Grav;
    int8 m_Mag;
    int8 m_Ladar;
    int8 m_Radar;
    int8 m_Unrated;
    int8 m_Complex;


    std::map<uint32, CosmicSignature> m_sigs;

};

#endif  // EVEMU_SYSTEM_ANOMALYMGR_H_
