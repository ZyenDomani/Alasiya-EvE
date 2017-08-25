
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

/*  this class will keep track of all Anomalies for its system
 *.   the scan system will query this class for current anomaly data
 *
 *  it will need access to its system manager (thru m_system), the wh mgr (thru sWHMgr), dungeon mgr (thru m_dungeon), and ???
 *.  we should not need spawn mgr here, as it is called by dunmgr and processed thru sysmgr.
 *
 *  when one anomaly despawns, this class is in charge of creating another as needed.
 *
 *  this class is also in charge of all dynamic anomaly data in the db
 *    pos items, wrecks and abandoned ships will have to process thru here also, as they get sigIDs and are listed on scan results.
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
AnomalyMgr::AnomalyMgr(SystemManager* mgr, PyServiceMgr& svc)
:m_services(svc),
m_system(mgr),
m_spawnTimer(10000),
m_anomTimer(10000)
{
    m_initalized = false;

    m_anomTimer.Disable();
    m_spawnTimer.Disable();. // is this needed?
}

bool AnomalyMgr::Init(BeltMgr* beltMgr, DungeonMgr* dungMgr, SpawnMgr* spawnMgr) {
    if (!sConfig.cosmic.AnomalyEnabled) {
         _log(COSMIC_MGR__MESSAGE, "Anomaly System Disabled.  Not Initalizing Anomaly Manager for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
        return m_initalized;
    }
    m_beltMgr = beltMgr;
    m_dungMgr = dungMgr;
    m_spawnMgr = spawnMgr;

    if (m_beltMgr == nullptr) {
        _log(COSMIC_MGR__ERROR, "System Init Fault. beltMgr == nullptr.  Not Initalizing Anomaly Manager for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
        return m_initalized;
    }

    if (m_dungMgr == nullptr) {
        _log(COSMIC_MGR__ERROR, "System Init Fault. dungMgr == nullptr.  Not Initalizing Anomaly Manager for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
        return m_initalized;
    }

    if (m_spawnMgr == nullptr) {
        _log(COSMIC_MGR__ERROR, "System Init Fault. spawnMgr == nullptr.  Not Initalizing Anomaly Manager for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
        return m_initalized;
    }

    // set internal check data
    //. will these be static, var by system, var by trusec, config options, other???
    m_Sigs = 0;
    m_Anoms = 0;
    m_WH = 0;
    m_Grav = 0;
    m_Mag = 0;
    m_Ladar = 0;
    m_Radar = 0;
    m_Unrated = 0;
    m_Complex = 0;

    LoadAnomalies();

    m_anomTimer.Start(120000);
    //  system tests to determine amounts and types

    /* load current data?, start timers, process current data, and create new items, if needed */

    Process();


    _log(COSMIC_MGR__MESSAGE, "AnomalyMgr Initialized for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
    return (m_initalized = true);
}

void AnomalyMgr::Process() {
    if (!m_initalized)
        return;
    if (m_anomTimer.Check(false)) {
        /* do something useful here */
        // check for current sys anoms vs max, and create new if needed.
    }

    if (m_spawnTimer.Check(false)) {
        /* do something useful here */
    }

    m_dungMgr->Process();
}

void AnomalyMgr::LoadAnomalies() {
	//. is this needed?  probably not.  make em all dynamic
	// check for existing data and load accordingly.
	// this will only hit on system load

	// get loaded type data and save in memobj for later use
}

void AnomalyMgr::SaveAnomaly()
{
	//. same as above...not needed
    for (auto sig : m_sigs)
        m_mdb.SaveAnomaly(sig.second);

}

void AnomalyMgr::CreateAnomaly() {
    using namespace EVEDUNG;
	/* determine anomaly type, set variables,
	spawn dungeon, spawn NPCs, save data.
	*/
 /* will need more here to check and set all items correctly
    uint8 scanGroupID = EVESCAN::ScanGroup::ScanGroupAnomaly;
    uint16 groupID = EVEDB::invGroups::Cosmic_Anomaly; //885
    uint16 groupID2 = EVEDB::invGroups::Cosmic_Signature; //502
    uint16 typeID = EVEDB::invTypes::typeCosmicAnomaly; // 28356 - dont need probes or sklls
    uint16 typeID2 = EVEDB::invTypes::typeCosmicSignature; // 25880 - need probes and skills (exploring)
    uint16 strengthAttributeID = AttrScanAllStrength;
    */
    bool isDungeon = false, isBelt = false;
	uint8 anomalyType = GetAnomalyType();
    // compile data for new system signature.
    CosmicSignature sig;
    sig.sigName = ""; // will have to determine name and insert here.
    sig.sigID = sEntityList.GetAnomalyID();
    sig.sigItemID = 0;
    sig.systemID = m_system->GetID();
    sig.typeID = anomalyType;
    switch(anomalyType) {
        case dunTypes::typeGravimetric: { // 2
            sig.typeID = EVEDB::invTypes::typeCosmicAnomaly;
            sig.groupID = EVEDB::invGroups::Cosmic_Anomaly;
            sig.scanGroupID = EVESCAN::ScanGroup::ScanGroupAnomaly;         // this will change based on the actual ITEM being scanned...ship, tower, drone, etc.  see below
            sig.strengthAttributeID = AttrScanGravimetricStrength;
            isBelt = true;
        } break;
        case dunTypes::typeMagnetometric: { // 3,
            sig.typeID = 25880; // Cosmic_Signature
            sig.groupID = EVEDB::invGroups::Cosmic_Signature;
            sig.strengthAttributeID = AttrScanMagnetometricStrength;
        } break;
        case dunTypes::typeRadar: {       // 4,
            sig.typeID = 25880; // Cosmic_Signature
            sig.groupID = EVEDB::invGroups::Cosmic_Signature;
            sig.strengthAttributeID = AttrScanRadarStrength;
        } break;
        case dunTypes::typeLadar: {       // 5,
            sig.typeID = 25880; // Cosmic_Signature
            sig.groupID = EVEDB::invGroups::Cosmic_Signature;
            sig.strengthAttributeID = AttrScanLadarStrength;
        } break;
        // these will use default for now.  maybe change them later...wait till system matures more and i better understand hoe to implement them.
        case dunTypes::typeMission:       // 1
        case dunTypes::typeWormhole:     // 6
        case dunTypes::typeAnomaly:       // 7
        case dunTypes::typeUnrated:       // 8
        case dunTypes::typeEscalation:    // 9
        case dunTypes::typeDED_Complex: { // 10
            sig.typeID = EVEDB::invTypes::typeCosmicAnomaly;
            sig.groupID = EVEDB::invGroups::Cosmic_Anomaly;
            sig.strengthAttributeID = AttrScanAllStrength;  // Unknown
        } break;
    }

 if (isDungeon)
     m_dungMgr->MakeDungeon(sig); // pass by ref here, so other vars can be set.
 if (isBelt)
     m_beltMgr->Create(sig); // pass by ref here, so other vars can be set.

    // add new sig to sysSigMap
    // we will set location and faction here, dung will set up and call spawn for area, and deal with triggers using process called from sysmgr
     m_sigs.insert(std::pair<int32, CosmicSignature>(sig.sigItemID, sig)); //key is itemID for ease of removal later


    m_mdb.SaveAnomaly(sig);
}

int8 AnomalyMgr::GetAnomalyType()
{
    using namespace EVEDUNG;

    uint8 typeID = MakeRandomInt(1,10);
    switch(typeID) {
        case dunTypes::typeMission: {   // 1
            // cannot create this type.  try again.
            return GetAnomalyType();
        } break;
        case dunTypes::typeGravimetric: {   // 2
            m_Grav = 0;
            m_Sigs = 0;
        } break;
        case dunTypes::typeMagnetometric: {   // 3

            m_Mag = 0;
            m_Sigs = 0;
        } break;
        case dunTypes::typeRadar: {   // 4

            m_Radar = 0;
            m_Sigs = 0;
        } break;
        case dunTypes::typeLadar: {   // 5

            m_Ladar = 0;
            m_Sigs = 0;
        } break;
        case dunTypes::typeWormhole: {   // 6

            m_WH = 0;
            m_Sigs = 0;
        } break;
        case dunTypes::typeAnomaly: {   // 7

            m_Anoms = 0;
        } break;
        case dunTypes::typeUnrated: {   // 8

            m_Unrated = 0;
            m_Anoms = 0;
        } break;
        case dunTypes::typeEscalation: {   // 9
            // cannot create this type.  try again.
            return GetAnomalyType();
        } break;
        case dunTypes::typeDED_Complex: {  // 10

            m_Complex = 0;
            m_Sigs = 0;
        } break;
    }
}


void AnomalyMgr::AddAnomaly(InventoryItemRef iRef) {
    // add method for pos items, wrecks and abandoned ships
    // do these need probes to scan down?

}

void AnomalyMgr::GetAnomalyList(CosmicSignature& sig) {
    // retrieval method for scan queries
    //. NOTE. cannot scan pos, wrecks or ships.  they DO have sigIDs, and can get to type (25%), but no farther
}



/* more data for signatures...
 * this will have to be checked and set in the code.
 * this is def for scanGroupID:
 *
 * enum ScanGroup {
 *    ScanGroupScrap                = 1,. make a sig for wrecks in system?
 *    ScanGroupSignature            = 4,
 *    ScanGroupShip                 = 8,. abandoned ships in space
 *    ScanGroupStructure            = 16,. all pos structures
 *    ScanGroupDroneOrProbe         = 32,. dromes or probes in system
 *    ScanGroupCelestial            = 64,. not sure here
 *    ScanGroupAnomaly              = 128
 * };
 *
 *  for strengthAttributeID, use these attributes to indicate site type:
 *
 * AttrScanRadarStrength = 208,
 * AttrScanLadarStrength = 209,
 * AttrScanMagnetometricStrength = 210,
 * AttrScanGravimetricStrength = 211,
 * AttrScanAllStrength = 1136     - this is for the "unknown" anomaly type
 *
 */
