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
m_dungeonTimer(10000),  // arbitrary 10s default
m_beltTimer(10000),
m_spawnTimer(10000),
m_anomTimer(10000),
m_beltMgr(nullptr),
m_dungMgr(nullptr),
m_spawnMgr(nullptr)
{
    m_initalized = false;

    m_dungeonTimer.Disable();
    m_beltTimer.Disable();
    m_spawnTimer.Disable();

    m_anomTimer.Start();

}

bool AnomalyMgr::Init(BeltMgr* beltMgr, DungeonMgr* dungMgr, SpawnMgr* spawnMgr)
{
    if (!sConfig.cosmic.AnomalyEnabled) {
        _log(COSMIC_MGR__MESSAGE, "Anomaly System Disabled.  Not Initalizing Anomaly Manager for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
        return;
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

    LoadAnomaly();

    //  system tests to determine amounts and types


    Process();

    _log(COSMIC_MGR__MESSAGE, "AnomalyMgr Initialized for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
    return (m_initalized = true);
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

    10 types of anomalies.
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

/* Cosmic anomalies are PvE sites that can be found throughout EVE. They are found using the system scanner, and require no skills or equipment to locate. To locate cosmic anomalies, simply open the scan window where they are visible as warpable sites.
 *
 *    Combat Sites are ungated pockets with multiple waves of rats to kill. They can drop faction items or escalate to new sites.
 *        Besieged Covert Research Facilities are special combat sites which are found only in low-sec. They containing multiple cruiser- and battleship-class NPC rats to be killed with a combat ship. In addition, they may contain destructible structures with loot.
 *    Ore sites contain asteroids to be mined for minerals or ice products; the ore often includes types not normally found in systems of that security rating, i.e. anomalies in high sec may contain low sec ore, etc.
 *
 * Cosmic signatures are scannable locations in space. To locate a cosmic signature it must be scanned with scan probes. You can see in the scan window whether a system contains cosmic signatures, but identifying them and pinpointing them requires scan probes. The type is identified at 25% scan, the name is revealed at 75% scan and the site is warpable at 100% scan.
 *
 *    Combat Sites are gated deadspace complexes with rats to kill. They can drop valuable faction modules and deadspace modules, and can escalate into expeditions.
 *    Gas Sites contain gas clouds that can be harvested. For more details see Gas cloud harvesting.
 *    Relic Sites contain containers that need to be hacked with a relic analyzer. In normal space they do not contain any dangerous elements. For more details see relic and data sites.
 *    Data Sites contain containers that need to be hacked with a data analyzer. They range from completely safe to deadly dangerous. For more details see relic and data sites.
 *    Wormholes are temporary unstable connections between two systems.
 *
 * Site Respawn Mechanics
 *    Much of the information surrounding the respawn mechanics of cosmic signatures is based on theory, with little hard data offered by CCP. Nevertheless, the sites will respawn immediately after being fully cleared by a player. It is unknown what range the new site will spawn in (if it's in the same region or not) or whether it will be of the same or similar type as the one cleared. Note that, based on this information, we know that sites will not respawn after daily downtime. They will only spawn when sites are cleared somewhere else within the game world.
 */

/* High security space
 *
 *    See also: High-sec
 *
 *    Omber (small • average • large)
 *    Kernite and Omber (small • average • large)
 *    Jaspet, Kernite, and Omber (small • average • large)
 *    Hemorphite, Jaspet, and Kernite (small • average • large)
 *    Hedbergite, Hemorphite, and Jaspet (small • average • large)
 *
 * Low security space
 *
 *    See also: Low-sec
 *
 *    Gneiss (small • average • large)
 *    Dark Ochre and Gneiss (small • average • large)
 *    Crokite, Dark Ochre and Gneiss (small • average • large)
 *    Spodumain, Crokite and Dark Ochre (small • average • large)
 *
 * Null security space
 *
 *    See also: Null-sec
 *
 *    Bistot (small • average • large)
 *    Arkonor and Bistot (small • average • large)
 *    Mercoxit, Arkonor and Bistot (small • average • large)
 *
 */