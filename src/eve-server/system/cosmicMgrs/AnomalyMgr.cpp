
 /**
  * @name AnomalyMgr.h
  *     Anomaly managment system for Alasiya EvEmu
  *
  * @Author:        Allan
  * @date:          12 December 2015
  *
  */
 

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "PyServiceMgr.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/AnomalyMgr.h"
#include "system/cosmicMgrs/BeltMgr.h"
#include "system/cosmicMgrs/DungeonMgr.h"
#include "system/cosmicMgrs/SpawnMgr.h"
#include "system/cosmicMgrs/WormholeMgr.h"

/*
 * # Cosmic Mgr Logging:
 * COSMIC_MGR=1
 * COSMIC_MGR__ERROR=1
 * COSMIC_MGR__WARNING=1
 * COSMIC_MGR__MESSAGE=0
 * COSMIC_MGR__DEBUG=1
 * COSMIC_MGR__TRACE=0
 */
AnomalyMgr::AnomalyMgr(SystemManager* mgr, PyServiceMgr& svc)
: m_system(mgr),
  m_services(svc)
{
    m_initalized = false;
}

void AnomalyMgr::Init(AsteroidBeltMgr* beltMgr, DungeonMgr* dungMgr, SpawnMgr* spawnMgr)
{
    m_beltMgr = beltMgr;
    m_dungMgr = dungMgr;
    m_spawnMgr = spawnMgr;

    if (!sConfig.cosmic.AnomalyEnabled) {
        _log(COSMIC_MGR__MESSAGE, "Anomaly System Disabled.  Not Initalizing Anomaly Manager for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
        return;
    }

    m_initalized = true;

    _log(COSMIC_MGR__MESSAGE, "AnomalyMgr Initialized for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
}

void AnomalyMgr::Process() {
    if (!m_initalized)
        return;

}

/* eventually,this will be the ONLY save routine for Anomalies/Signatures.
 * for now, dungeon anoms are saved in DungeonMgr
 */
void AnomalyMgr::SaveAnomaly()
{
    uint8 scanGroupID = ScanGroupAnomaly;
    uint16 groupID = EVEDB::invGroups::Cosmic_Anomaly; //885
    uint16 groupID2 = EVEDB::invGroups::Cosmic_Signature; //502
    uint16 typeID = 28356; // Cosmic_Anomaly - dont need probes or sklls
    uint16 typeID2 = 25880; // Cosmic_Signature - need probes and skills (exploring)

    uint16 strengthAttributeID = AttrScanAllStrength;

    CosmicSignature sig;
    /* will need a bit of code here to check and set all items correctly for saving -- see below*/
    m_db.SaveAnomaly(sig);

}
