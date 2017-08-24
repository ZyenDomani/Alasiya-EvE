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

/*  this class will keep track of all Anomalies in universe, what systems they are in, and how long they last.
 *
 *  it will need access to the system manager (thru sEntityList), the wh mgr (thru sWHMgr), spawn mgr (thru sysmgr->SpawnMgr), and ???
 *
 *  when one anomaly despawns, this class is in charge of creating another as needed.
 *
 *  this class is also in charge of all dynamic anomaly data in the db
 */

/*
 * # Cosmic Mgr Logging:
 * COSMIC_MGR=1
 * COSMIC_MGR__ERROR=1
 * COSMIC_MGR__WARNING=1
 * COSMIC_MGR__MESSAGE=0
 * COSMIC_MGR__DEBUG=1
 * COSMIC_MGR__TRACE=0
 */
AnomalyMgr::AnomalyMgr()
: m_spawnTimer(10000),
m_anomTimer(10000)
{
    m_initalized = false;

    m_anomTimer.Disable();
    m_spawnTimer.Disable();
}

void AnomalyMgr::Initialize(PyServiceMgr* svc) {
    if (!sConfig.cosmic.AnomalyEnabled) {
        sLog.Warning("  Anomaly Manager", "Anomaly Manager Disabled.");
        return;
    }
    m_services = svc;

    LoadAnomalies();

    m_anomTimer.Start(120000);
    //  system tests to determine amounts and types


    Process();

    sLog.Blue("  Anomaly Manager", "Anomaly Manager Initialized.");

    /* load current data, start timers, process current data, and create new items, if needed */
}

void AnomalyMgr::Process() {
    if (!m_initalized)
        return;
    if (m_anomTimer.Check(false)) {
        /* do something useful here */
    }

    if (m_spawnTimer.Check(false)) {
        /* do something useful here */
    }
}

void AnomalyMgr::LoadAnomalies() {
	// check for existing data and load accordingly.
	// this will only hit on system load

	// get loaded type data and save in memobj for later comparison
}

/* eventually,this will be the ONLY save routine for Anomalies/Signatures.
 * for now, dungeon anoms are saved in DungeonMgr
 */
void AnomalyMgr::SaveAnomaly()
{
    /* will need a bit of code here to check and set all items correctly for saving -- see below
    uint8 scanGroupID = EVESCAN::ScanGroup::ScanGroupAnomaly;
    uint16 groupID = EVEDB::invGroups::Cosmic_Anomaly; //885
    uint16 groupID2 = EVEDB::invGroups::Cosmic_Signature; //502
    uint16 typeID = EVEDB::invTypes::typeCosmicAnomaly; // 28356 - dont need probes or sklls
    uint16 typeID2 = EVEDB::invTypes::typeCosmicSignature; // 25880 - need probes and skills (exploring)

    uint16 strengthAttributeID = AttrScanAllStrength;

    CosmicSignature sig;
    m_db.SaveAnomaly(sig);
*/
}

void CreateAnomaly() {
/*
    // begin compiling data for saving in system signatures table.
    CosmicSignature sig;
    sig.dungeonName = itr->second.dunName;
    sig.sigID = sEntityList.GetAnomalyID();
    sig.sigItemID = sDunDataMgr.GetDungeonID();
    sig.systemID = m_system->GetID();
    sig.scanGroupID = EVESCAN::ScanGroup::ScanGroupAnomaly;         // this will change based on the actual ITEM being scanned...ship, tower, drone, etc.
    sig.typeID = 25880; // Cosmic_Signature
    sig.groupID = EVEDB::invGroups::Cosmic_Anomaly;
    sig.strengthAttributeID = AttrScanAllStrength;  // Unknown
    switch(typeID) {
        case typeGravimetric: { // 2
            sig.typeID = 25880; // Cosmic_Signature
            sig.groupID = EVEDB::invGroups::Cosmic_Signature;
            sig.strengthAttributeID = AttrScanGravimetricStrength;
        } break;
        case typeMagnetometric: { // 3,
            sig.typeID = 25880; // Cosmic_Signature
            sig.groupID = EVEDB::invGroups::Cosmic_Signature;
            sig.strengthAttributeID = AttrScanMagnetometricStrength;
        } break;
        case typeRadar: {       // 4,
            sig.typeID = 25880; // Cosmic_Signature
            sig.groupID = EVEDB::invGroups::Cosmic_Signature;
            sig.strengthAttributeID = AttrScanRadarStrength;
        } break;
        case typeLadar: {       // 5,
            sig.typeID = 25880; // Cosmic_Signature
            sig.groupID = EVEDB::invGroups::Cosmic_Signature;
            sig.strengthAttributeID = AttrScanLadarStrength;
        } break;
        // these will use default for now.  maybe change them later...wait till system matures more.
        case typeMission:       // 1
        case typeWormholes:     // 6
        case typeAnomaly:       // 7
        case typeUnrated:       // 8
        case typeEscalation:    // 9
        case typeDED_Complex: { // 10
        } break;
    }
*/
}

/* more data for signatures...
 * this will have to be checked and set in the code.
 * this is def for scanGroupID:
 *
 * typedef enum {
 *    ScanGroupScrap                = 1,
 *    ScanGroupSignature            = 4,
 *    ScanGroupShip                 = 8,
 *    ScanGroupStructure            = 16,
 *    ScanGroupDroneOrProbe         = 32,
 *    ScanGroupCelestial            = 64,
 *    ScanGroupAnomaly              = 128
 * } ScanGroup;
 *
 *  for strengthAttributeID, use these attributes to indicate site type:
 *
 * AttrScanRadarStrength = 208,
 * AttrScanLadarStrength = 209,
 * AttrScanMagnetometricStrength = 210,
 * AttrScanGravimetricStrength = 211,
 * AttrScanAllStrength = 1136     - unknown
 *
 */
