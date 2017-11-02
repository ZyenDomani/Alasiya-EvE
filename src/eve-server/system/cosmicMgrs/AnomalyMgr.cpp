
/**
 * @name AnomalyMgr.cpp
 *     Anomaly managment system for Alasiya EvEmu
 *
 * @Author:        Allan
 * @date:          12 December 2015 (original idea)
 * @update:        3 August 2017 (begin implementation)
 *
 */


#include "eve-server.h"

#include "EVEServerConfig.h"
#include "PyServiceMgr.h"
#include "StaticDataMgr.h"
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
 *  when one anomaly despawns, this class is in charge of calling cleanup and creating another as needed.
 *
 *  this class is also in charge of all dynamic anomaly data in the db
 *    pos items, wrecks and abandoned ships will have to process thru here also,
 *    as they get sigIDs and are listed on scan results, but cannot be totally scanned down
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
m_beltMgr(nullptr),
m_dungMgr(nullptr),
m_spawnMgr(nullptr),
m_spawnTimer(10000),
m_anomTimer(10000)
{
    m_initalized = false;

    m_sigBySigID.clear();
    m_sigByItemID.clear();

    m_anomTimer.Disable();
    m_spawnTimer.Disable(); // is this needed?
}

AnomalyMgr::~AnomalyMgr()
{
    InventoryItemRef iRef;
    for (auto sig : m_sigByItemID) {
        iRef = m_services.item_factory->GetItem(sig.first);
        m_system->RemoveItemFromInventory(iRef);
        iRef->Delete();
    }
}

bool AnomalyMgr::Init(BeltMgr* beltMgr, DungeonMgr* dungMgr, SpawnMgr* spawnMgr) {
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

    if (!sConfig.cosmic.AnomalyEnabled) {
         _log(COSMIC_MGR__MESSAGE, "Anomaly System Disabled.  Not Initalizing Anomaly Manager for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
        return true;
    }
    if (!sConfig.cosmic.DungeonEnabled){
        _log(COSMIC_MGR__MESSAGE, "Dungeon System Disabled.  Not Initializing Anomaly Manager for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
        return true;
    }

    if (!sConfig.npc.RoamingSpawns and !sConfig.npc.StaticSpawns) {
        _log(COSMIC_MGR__MESSAGE, "Spawn System Disabled.  Not Initalizing Anomaly Manager for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
        return true;
    }

    if (!sConfig.cosmic.BeltEnabled) {
        _log(COSMIC_MGR__MESSAGE, "BeltMgr System Disabled.  Not Initalizing Anomaly Manager for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
        return true;
    }

    // set internal check data
    // range is 0.1 for 1.0 system to 2.0 for -0.9 system
    float security = 1.1 - m_system->GetSystemSecurityRating();
         if (security == 2.0)  m_maxSigs = 30;
    else if (security > 1.501) m_maxSigs = 25;
    else if (security > 1.001) m_maxSigs = 20;
    else if (security > 0.751) m_maxSigs = 15;
    else if (security > 0.451) m_maxSigs = 12;
    else if (security > 0.251) m_maxSigs = 8;
    else                       m_maxSigs = 5;

    // will these be static, var by system, var by trusec, config options, other???
    m_Sigs = 0;
    m_Anoms = 0;
    // these can use config option to (en/dis)able individual types
    m_WH = 0;
    m_Grav = 0;
    m_Mag = 0;
    m_Ladar = 0;
    m_Radar = 0;
    m_Unrated = 0;
    m_Complex = 0;

    /* load current data?, start timers, process current data, and create new items, if needed */
    if (sConfig.server.IsTestServer)
        m_anomTimer.Start(1000);  // 1s
    else
        m_anomTimer.Start(120000);  // 120s

    _log(COSMIC_MGR__MESSAGE, "AnomalyMgr Initialized for %s(%u) with %u Max Signals", m_system->GetName().c_str(), m_system->GetID(), m_maxSigs);
    return (m_initalized = true);
}

void AnomalyMgr::Process() {
    if (!m_initalized)
        return;
    if (m_anomTimer.Check(!sConfig.server.IsTestServer)) {
        /* do something useful here */
        if (m_Sigs < m_maxSigs)
            CreateAnomaly();
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
	//. same as above...not needed but used for testing for now.
    //will have to rewrite scan system to use data from here
    for (auto sig : m_sigByItemID)
        m_mdb.SaveAnomaly(sig.second);

}

void AnomalyMgr::RemoveAnomaly(uint32 itemID)
{
    std::map<uint32, CosmicSignature>::iterator itr = m_sigByItemID.find(itemID);
    if (itr != m_sigByItemID.end()) {
        std::map<std::string, CosmicSignature>::iterator itr2 = m_sigBySigID.find(itr->second.sigID);
        if (itr2 != m_sigBySigID.end())
            m_sigBySigID.erase(itr2);
        m_sigByItemID.erase(itr);
    }
}


void AnomalyMgr::CreateAnomaly(int8 typeID/*0*/) {
    using namespace EVEDUNG;

    // compile data for new system anomaly.
    CosmicSignature sig;
        sig.systemID = m_system->GetID();
        sig.sigID = sEntityList.GetAnomalyID();
        // *Mgr will determine name and itemID.
        sig.sigItemID = 0;
        sig.sigName = "Test Name Here";
        sig.ownerID = 500022;
        if (sConfig.npc.AnomalyFaction)
            sig.ownerID = sConfig.npc.AnomalyFaction;
        else if (MakeRandomFloat() > 0.15) // chance to be rogue drones
            sig.ownerID =  sDataMgr.GetRegionRatFaction(m_system->GetRegionID());

        if (typeID == 0)
            sig.dungeonType = GetAnomalyType();
        else  // mission or escalation being called.
            sig.dungeonType = typeID;

        if (sig.dungeonType == 0)
            return;     // make error here?

    GPoint pos = m_gp.GetAnomalyPoint(m_system);
        sig.x = pos.x;
        sig.y = pos.y;
        sig.z = pos.z;

    switch(sig.dungeonType) {
        case dunTypes::typeGravimetric: { // 2
            sig.sigTypeID = EVEDB::invTypes::typeCosmicAnomaly; //dont need probes or skills for anomalies
            sig.sigGroupID = EVEDB::invGroups::Cosmic_Anomaly;
            sig.scanGroupID = EVESCAN::ScanGroup::ScanGroupAnomaly;
            sig.scanAttributeID = AttrScanGravimetricStrength;
        } break;
        case dunTypes::typeMagnetometric: { // 3,
            sig.sigTypeID = EVEDB::invTypes::typeCosmicSignature;// need probes and exploring skills
            sig.sigGroupID = EVEDB::invGroups::Cosmic_Signature;
            sig.scanGroupID = EVESCAN::ScanGroup::ScanGroupSignature;
            sig.scanAttributeID = AttrScanMagnetometricStrength;
        } break;
        case dunTypes::typeRadar: {       // 4,
            sig.sigTypeID = EVEDB::invTypes::typeCosmicSignature;
            sig.sigGroupID = EVEDB::invGroups::Cosmic_Signature;
            sig.scanGroupID = EVESCAN::ScanGroup::ScanGroupSignature;
            sig.scanAttributeID = AttrScanRadarStrength;
        } break;
        case dunTypes::typeLadar: {       // 5,
            sig.sigTypeID = EVEDB::invTypes::typeCosmicSignature;
            sig.sigGroupID = EVEDB::invGroups::Cosmic_Signature;
            sig.scanGroupID = EVESCAN::ScanGroup::ScanGroupSignature;
            sig.scanAttributeID = AttrScanLadarStrength;
        } break;
        case dunTypes::typeAnomaly: {      // 7
            sig.sigTypeID = EVEDB::invTypes::typeCosmicAnomaly;
            sig.sigGroupID = EVEDB::invGroups::Cosmic_Anomaly;
            sig.scanGroupID = EVESCAN::ScanGroup::ScanGroupAnomaly;
            sig.scanAttributeID = AttrScanAllStrength;
        } break;
        // create and register here
        case dunTypes::typeMission:       // 1
        case dunTypes::typeEscalation:   // 9
        // these will use default for now.  revisit later when system matures more and i better understand how to implement them.
        case dunTypes::typeUnrated:       // 8
        case dunTypes::typeDED_Complex: { // 10
            sig.sigTypeID = EVEDB::invTypes::typeCosmicSignature;
            sig.sigGroupID = EVEDB::invGroups::Cosmic_Signature;
            sig.scanGroupID = EVESCAN::ScanGroup::ScanGroupSignature;
            sig.scanAttributeID = AttrScanAllStrength;  // Unknown
        } break;
        case dunTypes::typeWormhole: {    // 6
            // enable WH to be warped to...they are deco only at this time.
            sig.sigTypeID = EVEDB::invTypes::typeCosmicAnomaly;
            sig.sigGroupID = EVEDB::invGroups::Cosmic_Anomaly;
            sig.scanGroupID = EVESCAN::ScanGroup::ScanGroupAnomaly;
            sig.scanAttributeID = AttrScanAllStrength;  // Unknown
            // hand off to WHMgr and exit after return
            sWHMgr.Create(sig);
            m_sigBySigID.insert(std::pair<std::string, CosmicSignature>(sig.sigID, sig));
            m_sigByItemID.insert(std::pair<uint32, CosmicSignature>(sig.sigItemID, sig));
            //m_mdb.SaveAnomaly(sig);
            return;
        } break;
        case 0:     // error or denied
            return;
    }

    // all anomalies will be created/populated by dungmgr, except WH (handed off to WHMgr above)
    if (!m_dungMgr->MakeDungeon(sig)) // pass by ref here, so other vars can be set.
        return;
    // add new sig to sysSigMaps
    //key is itemID for ease of removal later
    m_sigBySigID.insert(std::pair<std::string, CosmicSignature>(sig.sigID, sig));
    //if (sig.sigTypeID == EVEDB::invTypes::typeCosmicAnomaly)
        m_sigByItemID.insert(std::pair<uint32, CosmicSignature>(sig.sigItemID, sig));
   // else
   //     m_anomByItemID.insert(std::pair<uint32, CosmicSignature>(sig.sigItemID, sig));

    //m_mdb.SaveAnomaly(sig);

    _log(COSMIC_MGR__MESSAGE, "AnomalyMgr::Create() - Creating Signal %s of type %u in system %u", sig.sigName.c_str(), sig.dungeonType, sig.systemID);
}

uint8 AnomalyMgr::GetAnomalyType()
{
    using namespace EVEDUNG;

    uint8 typeID = MakeRandomInt(2,10); // skip typeMission
    switch(typeID) {
        case dunTypes::typeEscalation:  // 9
        case dunTypes::typeMission: {   // 1
            // cannot create this type here.  try again.
            return GetAnomalyType();
        } break;
        case dunTypes::typeGravimetric: {   // 2
            if (m_Grav < 0)
                return GetAnomalyType();

            ++m_Grav;
            ++m_Sigs;
        } break;
        case dunTypes::typeMagnetometric: {   // 3
            if (m_Mag < 0)
                return GetAnomalyType();

            ++m_Mag;
            ++m_Sigs;
        } break;
        case dunTypes::typeRadar: {   // 4
            if (m_Radar < 0)
                return GetAnomalyType();

            ++m_Radar;
            ++m_Sigs;
        } break;
        case dunTypes::typeLadar: {   // 5
            if (m_Ladar < 0)
                return GetAnomalyType();

            ++m_Ladar;
            ++m_Sigs;
        } break;
        case dunTypes::typeWormhole: {   // 6
            if (m_WH != 0) // cap at 1 per system, except k162...which ISNT created in this system (it's an exit, from WMS)
                return GetAnomalyType();

            ++m_WH;
            ++m_Sigs;
        } break;
        case dunTypes::typeAnomaly: {   // 7. this is noob dungeon, no probe required
            ++m_Sigs;
        } break;
        case dunTypes::typeUnrated: {   // 8
            if ((m_Unrated < 0) or (m_Unrated > 2)) // cap at 3
                return GetAnomalyType();

            ++m_Unrated;
        } break;
        case dunTypes::typeDED_Complex: {  // 10
            if ((m_Complex < 0) or (m_Complex > 1)) // cap at 2
                return GetAnomalyType();

            ++m_Complex;
            ++m_Sigs;
        } break;
    }
    ++m_Anoms; // still not sure how im gonna use these

    _log(COSMIC_MGR__MESSAGE, "AnomalyMgr::GetAnomalyType() - Returning type %u", typeID);
    return typeID;
}

uint32 AnomalyMgr::GetAnomalyID(std::string& sigID)
{   // <std::string, CosmicSignature>
    std::map<std::string, CosmicSignature>::iterator itr = m_sigBySigID.find(sigID);
    if (itr != m_sigBySigID.end())
        return itr->second.sigItemID;
    return 0;
}

GPoint AnomalyMgr::GetAnomalyPos(std::string& sigID)
{
    // <std::string, CosmicSignature>
    std::map<std::string, CosmicSignature>::iterator itr = m_sigBySigID.find(sigID);
    GPoint pos(NULL_ORIGIN);
    if (itr != m_sigBySigID.end()) {
        pos.x = itr->second.x;
        pos.y = itr->second.y;
        pos.z = itr->second.z;
    }
    return pos;
}


void AnomalyMgr::AddAnomaly(InventoryItemRef iRef) {
    // registration method for pos items, wrecks and abandoned ships
    // creation method for missions, escalations and ??
    /*
 * enum ScanGroup {
 *    ScanGroupScrap                = 1,. make a sig for wrecks in system?
 *    ScanGroupSignature            = 4,
 *    ScanGroupShip                 = 8,. abandoned ships in space
 *    ScanGroupStructure            = 16,. all pos structures
 *    ScanGroupDroneOrProbe         = 32,. dromes or probes in system
 *    ScanGroupCelestial            = 64,. not sure here
 *    ScanGroupAnomaly              = 128
 * };
 */

}

void AnomalyMgr::GetSignatureList(std::vector< CosmicSignature >& sig)
{
    // sysSignatures (sigID,sigItemID,dungeonType,sigName,systemID,sigTypeID,sigGroupID,scanGroupID,scanAttributeID,x,y,z)
    // retrieval method for scan queries
    for (auto cur : m_sigByItemID)
        sig.push_back(cur.second);
}

void AnomalyMgr::GetAnomalyList(std::vector<CosmicSignature>& sig) {
    // sysSignatures (sigID,sigItemID,dungeonType,sigName,systemID,sigTypeID,sigGroupID,scanGroupID,scanAttributeID,x,y,z)
    // retrieval method for scan queries
    for (auto cur : m_anomByItemID)
        sig.push_back(cur.second);
}

