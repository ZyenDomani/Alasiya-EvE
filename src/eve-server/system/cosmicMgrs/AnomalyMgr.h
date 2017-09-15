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
      virtual ~AnomalyMgr();

      bool Init(BeltMgr* beltMgr, DungeonMgr* dungMgr, SpawnMgr* spawnMgr);
      void Process();

      void SaveAnomaly();
      void CreateAnomaly(int8 typeID=0);
      void LoadAnomalies();

      void AddAnomaly(InventoryItemRef iRef);
      void RemoveAnomaly(uint32 itemID);
      void GetAnomalyList(std::vector< CosmicSignature >& sig);
      void GetSignatureList(std::vector< CosmicSignature >& sig);

      uint32 GetAnomalyID(std::string& sigID);
      GPoint GetAnomalyPos(std::string& sigID);

protected:
    ManagerDB m_mdb;
    ServiceDB m_sdb;
    SystemGPoint m_gp;

    uint8 GetAnomalyType();

private:
    /* we do not own any of these (our sysmgr does) */
    BeltMgr* m_beltMgr;
    DungeonMgr* m_dungMgr;
    SpawnMgr* m_spawnMgr;
    SystemManager* m_system;
    PyServiceMgr& m_services;

    Timer m_spawnTimer;
    Timer m_anomTimer;

    bool m_initalized;

    // internal data counters
    // < 0 (where possible) means "not allowed"
    uint16 m_Anoms; // system total, including pos, wrecks, ships.  65535 *should* be large enough
    uint8 m_Sigs; // total probe-needed items, hard-capped at 256
    // these should be fine soft-capped at 128
    int8 m_WH;
    int8 m_Grav;
    int8 m_Mag;
    int8 m_Ladar;
    int8 m_Radar;
    int8 m_Unrated; // simple combat sites, no probe needed
    int8 m_Complex; // DED sites


    std::map<uint32, CosmicSignature> m_sigByItemID;
    std::map<uint32, CosmicSignature> m_anomByItemID;
    std::map<std::string, CosmicSignature> m_sigBySigID;

};

#endif  // EVEMU_SYSTEM_ANOMALYMGR_H_
