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

#include "utils/Singleton.h"
#include "system/cosmicMgrs/ManagerDB.h"


class PyServiceMgr;

class AnomalyMgr
: public Singleton<AnomalyMgr>
{
  public:
      AnomalyMgr();
      virtual ~AnomalyMgr()                             { /* do nothing here */ }

      void Initialize(PyServiceMgr* svc);
      void Process();

      void LoadAnomalies();
      void SaveAnomaly();
      void CreateAnomaly();

protected:
    ManagerDB* m_mdb;
    ServiceDB* m_sdb;

private:
    /* we do not own any of these */
    PyServiceMgr* m_services;

    Timer m_spawnTimer;
    Timer m_anomTimer;

    bool m_initalized;

};

//Singleton
#define sAnomalyMgr \
( AnomalyMgr::get() )

#endif  // EVEMU_SYSTEM_ANOMALYMGR_H_
