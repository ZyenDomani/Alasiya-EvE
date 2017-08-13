/**
 * @name AnomalyMgr.cpp
 *     Anomaly managment system for Alasiya EvEmu
 *
 * @Author:        Allan
 * @date:          12 December 2015 (original idea)
 * @update:        3 August 2017 (implementation)
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
m_services(svc),
m_beltMgr(nullptr),
m_dungMgr(nullptr),
m_spawnMgr(nullptr)
{
    m_initalized = false;
}

void AnomalyMgr::Init(BeltMgr* beltMgr, DungeonMgr* dungMgr, SpawnMgr* spawnMgr)
{
    m_beltMgr = beltMgr;
    m_dungMgr = dungMgr;
    m_spawnMgr = spawnMgr;

    // test for null mgrs here.  probably dont need, but just to be thorough

    if (!sConfig.cosmic.AnomalyEnabled) {
        _log(COSMIC_MGR__MESSAGE, "Anomaly System Disabled.  Not Initalizing Anomaly Manager for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
        return;
    }

    //  system tests to determine amounts and types

    m_initalized = true;

    _log(COSMIC_MGR__MESSAGE, "AnomalyMgr Initialized for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
}

void AnomalyMgr::Process() {
    if (!m_initalized)
        return;

}

void LoadAnomaly() {
	// check for existing data and load accordingly.
	// this will only hit on system load

	// get loaded type data and save in memobj for later comparison
}

/* eventually,this will be the ONLY save routine for Anomalies/Signatures.
 * for now, dungeon anoms are saved in DungeonMgr
 */
void AnomalyMgr::SaveAnomaly()
{
    uint8 scanGroupID = EVESCAN::ScanGroup::ScanGroupAnomaly;
    uint16 groupID = EVEDB::invGroups::Cosmic_Anomaly; //885
    uint16 groupID2 = EVEDB::invGroups::Cosmic_Signature; //502
    uint16 typeID = 28356; // Cosmic_Anomaly - dont need probes or sklls
    uint16 typeID2 = 25880; // Cosmic_Signature - need probes and skills (exploring)

    uint16 strengthAttributeID = AttrScanAllStrength;

    CosmicSignature sig;
    /* will need a bit of code here to check and set all items correctly for saving -- see below*/
    m_db.SaveAnomaly(sig);

}

void CreateAnomaly() {
	/*. this needs more research to get groups and types right.
enum ScanGroup {
  ScanGroupScrap         = 1,
  ScanGroupSignature     = 4,
  ScanGroupShip          = 8,
  ScanGroupStructure     = 16,
  ScanGroupDroneOrProbe  = 32,
  ScanGroupCelestial     = 64,
  ScanGroupAnomaly       = 128
} ;

    uint8 scanGroupID = ScanGroupAnomaly;
    uint16 groupID = EVEDB::invGroups::Cosmic_Anomaly; //885
    uint16 groupID2 = EVEDB::invGroups::Cosmic_Signature; //502
    uint16 typeID = 28356; // Cosmic_Anomaly - dont need probes or sklls
    uint16 typeID2 = 25880; // Cosmic_Signature - need probes and skills (exploring)

    uint16 strengthAttributeID = AttrScanAllStrength;

    10 types of signatures the AMS can define.
typedef enum {
    typeMission             = 1,
    typeGravimetric         = 2,
    typeMagnetometric       = 3,
    typeRadar               = 4,
    typeLadar               = 5,
    typeWormholes           = 6,
    typeAnomaly             = 7,
    typeUnrated             = 8,
    typeEscalation          = 9,
    typeDED_Complex         = 10
} dunTypes;
	 */
}