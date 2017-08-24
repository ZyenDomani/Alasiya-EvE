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

}

//http://wiki.eve-inspiracy.com/index.php?title=Complete_Signature_Strength_List
/*
signatures get allocated a new ID.
The numeric portion is random, but the alphabet portion works with a counter.
 Multiple signatures in the same solar system will have the left most letter increased until it reaches Z,
 after which it will reset to A and the second letter will be increased, which upon reaching Z will increase the third letter.
When it skips letters on your scan results, remember that POS and abandoned ships in the system receive sig IDs too!
The counter seems to be persistent between regions as well.
*/
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
    typeMission             = 1, npc mission
    typeGravimetric         = 2, roids
    typeMagnetometric       = 3, salvage/archeology
    typeRadar               = 4, hacking
    typeLadar               = 5, gas mining
    typeWormholes           = 6,
    typeAnomaly             = 7,
    typeUnrated             = 8,
    typeEscalation          = 9, extra rooms from previous sute
    typeDED_Complex         = 10
} dunTypes;
	 */

//Salvaging sites contain wrecked ships that appear as containers in space and can be opened with a Salvager module

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

/*
   Besieged Guristas Covert Research Facility is an unique cosmic anomaly found in all low sec regions that spawns two waves
 of Mordu's Legion cruisers and battleships equipped with webifiers and warp disruptors.
Unlike other enemies, these ships are able to switch damage types to find what is most effective against
a given target (they do not deal omni damage).
 Notable loot includes higher-end implants and ship skin BPCs,
as well as special module BPCs.
*/

/*
Data sites use the following naming convention according to their location:
"Local (Faction) (site)" in high-security systems;
"Regional (Faction) (site)" in low-security systems;
"Central (Faction) (site)" in null-security and wormhole systems.

 Relic sites follow a similar convention:
"Crumbling (Faction) (site)" in high-security systems;
"Decayed (Faction) (site)" in low-security systems;
"Ruined (Faction) (site)" in null-security and wormhole systems.
Each region spawns sites for the local pirate group.

Data site containers have a physical appearance that resembles a gyroscope,
and are usually named "(Faction) Info Shard", "(Faction) Com Tower", "(Faction) Mainframe", "(Faction) Data Processing Center", "(Faction) Shattered Life-Support Unit", "(Faction) Virus Test Site", "(Faction) Minor Shipyard", "(Faction) Production Installer" or "(Faction) Backup Server".
Relic Site containers resemble ancient shipwrecks and are named "(Faction) Debris", "(Faction) Rubble", "(Faction) Crystal Quarry", "(Faction) Antiquated Outpost" or "(Faction) Stone Formation"
*/

/* High security space
 *    Omber (small • average • large)
 *    Kernite and Omber (small • average • large)
 *    Jaspet, Kernite, and Omber (small • average • large)
 *    Hemorphite, Jaspet, and Kernite (small • average • large)
 *    Hedbergite, Hemorphite, and Jaspet (small • average • large)
 *
 * Low security space
 *    Gneiss (small • average • large)
 *    Dark Ochre and Gneiss (small • average • large)
 *    Crokite, Dark Ochre and Gneiss (small • average • large)
 *    Spodumain, Crokite and Dark Ochre (small • average • large)
 *
 * Null security space
 *    Bistot (small • average • large)
 *    Arkonor and Bistot (small • average • large)
 *    Mercoxit, Arkonor and Bistot (small • average • large)
 */
 // non-drone:  hideaway, burrow, refuge, den, yard, rally point, port, hub, haven, sanctum
 // drone:  cluster, collection, assembly, gathering, Surveillance, Menagerie, herd, squad, patrol, horde
 /*
 Difficulty	Found In	Combat anomalies
Class	Level	High	Low	Null	Angel Cartel	Blood Raiders	Guristas Pirates	Sansha's Nation	Serpentis Corporation	Rogue Drones
1	      1.  	x			       Angel Hideaway	Blood Hideaway	Guristas Hideaway	Sansha Hideaway	Serpentis Hideaway	Drone Cluster
           2.  	x			       Angel Hidden Hideaway	Blood Hidden Hideaway	Guristas Hidden Hideaway	Sansha Hidden Hideaway	Serpentis Hidden Hideaway
           3.  	x		       	Angel Forsaken Hideaway	Blood Forsaken Hideaway	Guristas Forsaken Hideaway	Sansha Forsaken Hideaway	Serpentis Forsaken Hideaway
           4.  	x		       	Angel Forlorn Hideaway	Blood Forlorn Hideaway	Guristas Forlorn Hideaway	Sansha Forlorn Hideaway	Serpentis Forlorn Hideaway
2	          	x		       	Angel Burrow	Blood Burrow	Guristas Burrow	Sansha Burrow	Serpentis Burrow	Drone Collection
3	          	x	  x	   	Angel Refuge	Blood Refuge	Guristas Refuge	Sansha Refuge	Serpentis Refuge	Drone Assembly
4.     	1.  	x. 	x	   	Angel Den	Blood Den	Guristas Den	Sansha Den	Serpentis Den	Drone Gathering
           2.  	        x. 	x	Angel Hidden Den	Blood Hidden Den	Guristas Hidden Den	Sansha Hidden Den	Serpentis Hidden Den
           3.  	        x. 	x	Angel Forsaken Den	Blood Forsaken Den	Guristas Forsaken Den	Sansha Forsaken Den	Serpentis Forsaken Den
           4.          	x. 	x	Angel Forlorn Den	Blood Forlorn Den	Guristas Forlorn Den	Sansha Forlorn Den	Serpentis Forlorn Den
5		              	x. 	x	Angel Yard	Blood Yard	Guristas Yard	Sansha Yard	Serpentis Yard	Drone Surveillance
6.     	1.          	x. 	x	Angel Rally Point	Blood Rally Point	Guristas Rally Point	Sansha Rally Point	Serpentis Rally Point	Drone Menagerie
           2	       	x  	x	Angel Hidden Rally Point	Blood Hidden Rally Point	Guristas Rally Rally Point	Sansha Hidden Rally Point	Serpentis Hidden Rally Point
           3	       	x. 	x	Angel Forsaken Rally Point	Blood Forsaken Rally Point	Guristas Forsaken Rally Point	Sansha Forsaken Rally Point	Serpentis Forsaken Rally Point
           4	       	x. 	x	Angel Forlorn Rally Point	Blood Forlorn Rally Point	Guristas Forlorn Rally Point	Sansha Forlorn Rally Point	Serpentis Forlorn Rally Point
7		              	x  	x	Angel Port	Blood Port	Guristas Port	Sansha Port	Serpentis Port	Drone Herd
8.     	1	       	x	  x	Angel Hub	Blood Hub	Guristas Hub	Sansha Hub	Serpentis Hub	Drone Squad
           2	       	x	  x	Angel Hidden Hub	Blood Hidden Hub	Guristas Hidden Hub	Sansha Hidden Hub	Serpentis Hidden Hub
           3	       	x	  x	Angel Forsaken Hub	Blood Forsaken Hub	Guristas Forsaken Hub	Sansha Forsaken Hub	Serpentis Forsaken Hub
           4	       	x	  x	Angel Forlorn Hub	Blood Forlorn Hub	Guristas Forlorn Hub	Sansha Forlorn Hub	Serpentis Forlorn Hub
9			                  	x	Angel Haven	Blood Haven	Guristas Haven	Sansha Haven	Serpentis Haven	Drone Patrol
10			                 	x	Angel Sanctum	Blood Sanctum	Guristas Sanctum	Sansha Sanctum	Serpentis Sanctum	Drone Horde

In addition to their basic version, Hideaway, Den, Rally Point and Hub sites also feature three variants of progressive difficulty featuring elite enemies.
These variants follow an extended naming convention as follows:
<Faction> Hidden <anomaly> - Level 2; higher amount of triggered spawns.
<Faction> Forsaken <anomaly> - Level 3; moderate difficulty, featuring elite enemy ships.
<Faction> Forlorn <anomaly> - Level 4; featuring tougher enemies and elite ships.


 Cosmic signatures

Combat sites that appear as Cosmic Signatures must be scanned down using Core Scanner Probes.
 These combat sites can be further divided into three groups: unrated complexes, DED rated complexes, and Chemical Labs.
The differences between the three are relatively minor, the most notable being that Chemical Labs require the use of Data Analyzer modules to obtain all of their rewards.

Unrated complexes have a good chance to contain a commander spawn that can drop Faction items.
They may also escalate into an expedition.
The sites contain multiple deadspace rooms separated by acceleration gates.
The acceleration gates separating the rooms are usually locked, in which case some condition must be fulfilled, or a key must be found to proceed.
The sites also have triggers for additional defender spawns and/ or an escalation.
 The size of ship allowed in Unrated Complexes does not follow a predictable pattern; each site, even of the same difficulty but from a different faction, may have a different ship size limit.
Found In	Unrated complexes
High	Low	Null	Angel Cartel	Blood Raiders	Guristas Pirates	Sansha's Nation	Serpentis Corporation	Rogue Drones
 x			        Angel Hideout	Blood Hideout	Gurista Hideout	Sansha Hideout	Serpentis Hideout	Haunted Yard
 x		        	Angel Lookout	Blood Lookout	Gurista Lookout	Sansha Lookout	Serpentis Lookout	Desolate Site
 x		        	Angel Watch	Blood Watch	Gurista Watch	Sansha Watch	Serpentis Watch	Chemical Yard
 x		        	Angel Vigil	Blood Vigil	Gurista Vigil	Sansha Vigil	Serpentis Vigil	-
         x	    	Provisional Angel Outpost	Provisional Blood Outpost	Provisional Gurista Outpost	Provisional Sansha Outpost	Provisional Serpentis Outpost	Rogue Trial Yard
         x	    	Angel Outpost	Blood Raider Outpost	Gurista Outpost	Sansha Outpost	Serpentis Outpost	Dirty Site
         x	    	Minor Angel Annex	Minor Blood Annex	Minor Guristas Annex	Minor Sansha Annex	Minor Serpentis Annex	Ruins
         x		    Angel Annex	Blood Annex	Guristas Annex	Sansha Annex	Serpentis Annex	-
                x	 Angel Base	Blood Raider Base	Gurista Base	Sansha Base	Serpentis Base	Independence
                x	 Angel Fortress	Blood Raider Fortress	Gurista Fortress	Sansha Fortress	Serpentis Fortress	Radiance
                x	 Angel Military Complex	Blood Military Complex	Gurista Military Complex	Sansha Military Complex	Serpentis Military Complex	Hierarchy
                x 	Angel Provincial HQ	Blood Provincial HQ	Gurista Provincial HQ	Sansha Provincial HQ	Serpentis Provincial HQ	-
                x	 Angel Domination Fleet Staging Point	Dark Blood Fleet Staging Point	Dread Guristas Fleet Staging Point	True Sansha Fleet Staging Point	Shadow Serpentis Fleet Staging Point	-

DED complexes are a type of cosmic signature that has been rated on a difficulty scale of 1/10 to 10/10 by CONCORD's Directive Enforcement Department.
 Lower rated sites contain weaker enemies, and limited ship class access.
DED rated complexes are found using Core Scanner Probes, or are received as an escalation from a combat anomaly.
There are no 9/10-rated DED complexes, however some consider the <Faction> Fleet Staging Point to be their equivalent.
The structure of a DED rated complex is very similar an Unrated Complex, and usually involve gated deadspace pockets with multiple groups of enemies.
Unlike other combat sites, DED rated complexes do not have additional enemy spawns; all enemies begin on-grid in each pocket.
 Many DED rated complexes also contain locked gates that may be unlocked by a certain trigger.
This makes blitzing possible, as not all enemies need to be cleared to proceed.
The sites also always contain one or more structures or overseer ships that can drop Faction items or Deadspace modules.
 Some DED complexes contain additional targets that have a low probability to drop a Faction module.
DED rated complexes do not normally escalate, but DED rated complexes that are received as an escalation from an anomaly may receive another escalation to the same site or a second part.
The difficulty rating of a complex determines maximum size of ship class that may be used to engage.
Starting from frigates at level 1/10, the ship size that can enter a site goes up by one size category with each increase in rating, maxing out at battleships at level 5/10.
DED Rating	Maximum Ship Class Size Allowed
1/10.      	Frigate class
2/10.     	Destroyer class (including Tactical Destroyer)
3/10.      	Cruiser class (except for T3)
4/10	      Battlecruiser class (except for T3 Cruiser)
5+/10.     	Battleship class and T3 Cruiser

DED Rating	Found In	DED complexes
       High	Low	Null	Angel Cartel	Blood Raiders	Guristas Pirates	Sansha's Nation	Serpentis Corporation	Rogue Drones
1/10	x 	x	      	Minmatar Contracted Bio-Farm	Old Meanie - Cultivation Center	Pith Robux Asteroid Mining & Co.	Sansha Military Outpost	Serpentis Drug Outlet	-
2/10	x	 x	      	Angel Creo-Corp Mining	Blood Raider Human Farm	Pith Deadspace Depot	Sansha Acclimatization Facility	Serpentis Live Cargo Distribution Facilities	Rogue Drone Infestation Sprout
3/10	x 	x	      	Angel Repurposed Outpost	Blood Raider Intelligence Collection Point	Guristas Guerilla Grounds	Sansha's Command Relay Outpost	Serpentis Narcotic Warehouses	Rogue Drone Asteroid Infestation
4/10	x	 x	      	Angel Cartel Occupied Mining Colony	Mul-Zatah Monastery	Guristas Scout Outpost	Sansha's Nation Occupied Mining Colony	Serpentis Phi-Outpost	-
5/10	x	 x	      	Angel's Red Light District	Blood Raider Psychotropics Depot	Guristas Hallucinogen Supply Waypoint	Sansha's Nation Neural Paralytic Facility	Serpentis Corporation Hydroponics Site	Outgrowth Rogue Drone Hive
6/10		   x. 	x. 	Angel Mineral Acquisition Outpost [1]	Crimson Hand Supply Depot	Guristas Troop Reinvigoration Camp	Sansha War Supply Complex	Serpentis Logistical Outpost [1]	-
7/10		       	x	  Angel Military Operations Complex	Blood Raider Coordination Center	Gurista Military Operations Complex	Sansha Military Operations Complex	Serpentis Paramilitary Complex	-
8/10		       	x. 	Cartel Prisoner Retention	Blood Raider Prison Camp	Pith's Penal Complex	Sansha Prison Camp	Serpentis Prison Camp	-
9/10	-	-	-	-	-	-
10/10			      x	  Angel Cartel Naval Shipyard	Blood Raider Naval Shipyard	The Maze	Centus Assembly T.P. Co.	Serpentis Fleet Shipyard	-
^ a b There is no rating displayed in warp in popup text.

Expeditions (also known as escalations) cannot be found by scanning, instead they "escalate" from Unrated Complexes.
When a certain trigger condition is met, there is a small chance that a pop up window will open explaining that the details of the location of another enemy site has been found.
 This site's information is then added to the Journal under the Expeditions tab.
When the site listed in the Journal is completed, there is a chance that it will escalate further, leading to yet another site.
Most expeditions have four such parts.
The locations of the sites provided by an expedition are random, often appearing several jumps away from where the expedition was obtained.
 Many expeditions may spawn their fourth site in lower-security space than where they began.
Unrated Complex	Expeditions
Angel Cartel	Blood Raiders	Guristas Pirates	Sansha's Nation	Serpentis Corporation	Rogue Drones
High Sec	Expeditions may appear in lower security systems.
<Faction> Hideout
Haunted Yard	Blue Pill	Frentix	Sooth Sayer	Drop	Mindflood	Pulverize The Pioneers
<Faction> Lookout
Desolate Site	Chasing the Dragon	Following the Blood	Trap?	Slave Breeding Plants	Angel Kickbacks	Mare Sargassum
<Faction> Watch
Chemical Yard	The Nuclear Small Arms Project	Medical Twilight	Terrorist Plot!	Nation on the Rise	Jet-Set Hooligans	Hunting the Drudge Factory
<Faction> Vigil	The Big Blue	The Rewards of Devotion	Kidnapped!	True Power Shipyards	Booster R&D	-
Low Sec	Expeditions may appear in lower security systems.
Provisional <Faction> Outpost
Rogue Trial Yard	Domination Surveillance Squad	Blood Surveillance Squad	Gurista Surveillance Squad	Sansha Surveillance Squad	Guardian Angels Surveillance Squad	Moving Day
<Faction> Outpost
Dirty Site	Salvation Angel's Shipment	Save The Slaves	Gurista Productions Shipment	Hidden Riches	Elite Playground	Loose Ends
Minor <Faction> Annex
Ruins	Angel Owned Station	Blood Owned Station	Guristas Owned Station	Sansha Owned Station	Serpentis Owned Station	Menacing Mechanics
<Faction> Annex	Angel Powergrid	Blood Raider Powergrid	Guristas Powergrid	Sansha Powergrid	Serpentis Powergrid	-
Null Sec
<Faction> Base
Independence	Toxic Waste Scandal!	Religious Fury	Consequences Smonsequences	True Power HQ	Contract Killers	The Drone Roulette
<Faction> Fortress
Radiance	Operation Spring Cleaning	Dubious Assignment	Hired Gun	Shady Operation	Suspicious Job	Molting Season
<Faction> Military Complex
Hierarchy	Pioneers Peril	Frontier in Flames	Pirate's Path	David V Goliath	Colony Under Fire	Trouble in Paradise
<Faction> Provincial HQ	Special Forces	Fountain of Youth	No Quarter	The Ancient City	Serpentis Secrets	-
<Faction> Fleet Staging Point	Angel Domination Fleet Staging Point 2	-	-	-	Shadow Serpentis Fleet Staging Point 2	-

Chemical Labs are Cosmic Signatures, and may be found by scanning with Core Scanner Probes.
Even though the probe scan window categorizes these as "gas sites", they are actually combat sites.
 To access all possible rewards, the site needs to be cleared of all hostiles and the containers hacked using a Data Analyzer module.
The Production Facility spawns in null security space only, while all other sites spawn in low security space.
Details of these sites are not well documented.
Most contain two waves of enemies and containers that need to be hacked, and certain sites are restricted to specific regions, though the region in which each site spawns is not entirely known.
 The regions listed here may not be the only region containing the particular site.
Faction	Site	Location
Angel Cartel	Angel Chemical Lab	Heimatar
Angel Gas Processing Site	Heimatar
Elohim Sooth Sayer Distribution Base	Molden Heath
Elohim X-Instinct Distribution Base	Heimatar
Elohim Sooth Sayer Production Facility	Wicked Creek
Elohim X-Instinct Production Facility	Feythabolis
Blood Raiders	Blood Raider Chemical Lab	Aridia
Blood Raider Gas Processing Site	?
CHAIN Mindflood Distribution Base	Aridia
CHAIN Mindflood Production Facility	Delve
Guristas Pirates	Guristas Chemical Lab	The Forge
Guristas Gas Processing Site	The Forge
H-PA Crew Blue Pill Distribution Base	The Forge
H-PA Crew Crash Distribution Base	Lonetrek
H-PA Crew Blue Pill Production Facility	Vale of the Silent
H-PA Crew Crash Production Facility	Tenal
Sansha's Nation	Sansha Chemical Lab	?
Sansha Gas Processing Site	Derelik
PDW-09FX Frentix Distribution Base	Derelik
PDW-09FX Frentix Production Facility	Catch
Serpentis Corporation	Serpentis Chemical Lab	Solitude
Serpentis Gas Processing Site	Placid
Core Runner Drop Distribution	Placid
Core Runner Exile Distribution Base	Solitude
Core Runner Drop Production Facility	Cloud Ring
Core Runner Exile Production Facility	Fountain

 */

