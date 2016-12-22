/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2011 The EVEmu Team
 *    For the latest information visit http://evemu.org
 *    ------------------------------------------------------------------------------------
 *    This program is free software; you can redistribute it and/or modify it under
 *    the terms of the GNU Lesser General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option) any later
 *    version.
 *
 *    This program is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License along with
 *    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 *    http://www.gnu.org/copyleft/lesser.txt.
 *    ------------------------------------------------------------------------------------
 *    Author:        Cometo
 *    Updates:  Allan
 */

#ifndef __COLONY_H_INCL__
#define __COLONY_H_INCL__

#include "PyCallable.h"

#include "planet/PlanetDB.h"

class PlanetSE;
class SystemEntity;
class Colony {
public:
    Colony(PyServiceMgr* mgr, Client* pClient, SystemEntity* pSE);
    ~Colony();

    void Init();
    void Load();
    void LoadPlants();    // for loading current data to pins
    void Save();
    void Update(bool updateTimes=false);
    void Shutdown();
    void UpdatePlants();  // for saving current data to pins
    void AbandonColony();

    void Process();
    void ProcessECUs(bool& save);
    void ProcessPlants(bool& save);

    void RemovePin(uint32 pinID);
    void RemoveLink(uint32 src, uint32 dest);
    void RemoveRoute(uint16 routeID);

    void UpgradeLink(uint32 src, uint32 dest, uint8 level);
    void UpgradeCommandCenter(uint32 pinID, int8 level);

    void CreatePin(uint32 groupID, uint32 pinID, uint32 typeID, double latitude, double longitude);
    void CreateLink(uint32 src, uint32 dest, uint16 level);
    void CreateRoute(uint16 routeID, uint32 typeID, uint32 qty, PyList* path);
    void CreateCommandPin(uint32 itemID, uint32 typeID, double latitude, double longitude);

    void AddExtractorHead(uint32 ecuID, uint16 headID, double latitude, double longitude);
    void MoveExtractorHead(uint32 ecuID, uint16 headID, double latitude, double longitude);
    void KillExtractorHead(uint32 ecuID, uint16 headID);

    void InstallProgram(uint32 ecuID, uint16 typeID, float headRadius);
    void SetSchematic(uint32 pinID, uint16 schematicID);
    void SetProgramResults(uint32 ecuID, uint16 typeID, uint16 numCycles, float headRadius, float cycleTime);

    void LaunchCommodities(uint32 pinID, std::map<uint16, uint32>& items);

    void PrioritizeRoute();

    uint8 GetPlantOrder(uint16 resTypeID);
    uint32 GetHeadType(uint16 ecuTypeID, uint16 resTypeID);

    uint32 GetOwner();

    PyRep* GetColony();
    PyTuple* GetPins();
    PyTuple* GetLinks();
    PyTuple* GetRoutes();
    PyDict* TransferCommodities(uint32 srcID, uint32 destID, std::map< uint16, uint32 > items);

    bool HasColony()                                    { return (ccPin->ccPinID ? true : false); }

    int8 GetLevel()                                     { return ccPin->level; }
    uint64 GetSimTime()                                 { return ccPin->currentSimTime; }

private:
    PyServiceMgr* m_svcMgr;
    PlanetSE* m_pSE;
    PI_CCPin* ccPin;
    Client* m_client;

    PlanetDB m_db;

    bool m_active = false;
    bool m_loaded = false;
    bool m_newHead = false;

    uint8 m_pLevel = 0;
    uint16 m_pg = 0;
    uint16 m_cpu = 0;
    uint32 m_colonyID = 0;

    uint64 m_procTime = 0;

    std::vector<uint32> tempECUs;
    std::map<uint8, uint32> tempPinIDs;
};

/* P0 - Raw Materials
 *  2267    Base Metals
 *  2270    Noble Metals
 *  2272    Heavy Metals
 *  2306    Non-CS Crystals
 *  2307    Felsic Magma
 *  2268    Aqueous Liquids
 *  2308    Suspended Plasma
 *  2309    Ionic Solutions
 *  2310    Noble Gas
 *  2311    Reactive Gas
 *  2073    Microorganisms
 *  2286    Planktic Colonies
 *  2287    Complex Organisms
 *  2288    Carbon Compounds
 *  2305    Autotrophs
 *
 * P1 - Basic Commodities
 *  2389    Plasmoids
 *  2390    Electrolytes
 *  2392    Oxidizing Compound
 *  2393    Bacteria
 *  2395    Proteins
 *  2396    Biofuels
 *  2397    Industrial Fibers
 *  2398    Reactive Metals
 *  2399    Precious Metals
 *  2400    Toxic Metals
 *  2401    Chiral Structures
 *  3779    Biomass
 *  9828    Silicon
 *  3683    Oxygen
 *  3645    Water
 *
 * P2 - Refined Commodities
 *    44    Enriched Uranium
 *  2312    Supertensile Plastics
 *  2317    Oxides
 *  2319    Test Cultures
 *  2321    Polyaramids
 *  2327    Microfiber Shielding
 *  2328    Water-Cooled CPU
 *  2329    Biocells
 *  2463    Nanites
 *  3689    Mechanical Parts
 *  3691    Synthetic Oil
 *  3693    Fertilizer
 *  3695    Polytextiles
 *  3697    Silicate Glass
 *  3725    Livestock
 *  3775    Viral Agent
 *  3828    Construction Blocks
 *  9830    Rocket Fuel
 *  9832    Coolant
 *  9836    Consumer Electronics
 *  9838    Superconductors
 *  9840    Transmitter
 *  9842    Miniature Electronics
 * 15317    Genetically Enhanced Livestock
 *
 * P3 - Specialized Commodities
 *  2344    Condensates
 *  2345    Camera Drones
 *  2346    Synthetic Synapses
 *  2348    Gel-Matrix Biopaste
 *  2349    Supercomputers
 *  2351    Smartfab Units
 *  2352    Nuclear Reactors
 *  2354    Neocoms
 *  2358    Biotech Research Reports
 *  2360    Industrial Explosives
 *  2361    Hermetic Membranes
 *  2366    Hazmat Detection Systems
 *  2367    Cryoprotectant Solution
 *  9834    Guidance Systems
 *  9846    Planetary Vehicles
 *  9848    Robotics
 * 12836    Transcranial Microcontrollers
 * 17136    Ukomi Superconductors
 * 17392    Data Chips
 * 17898    High-Tech Transmitters
 * 28974    Vaccines
 *
 *
 * P4 - Advanced Commodities
 *  2867    Broadcast Node
 *  2868    Integrity Response Drones
 *  2869    Nano-Factory
 *  2870    Organic Mortar Applicators
 *  2871    Recursive Computing Module
 *  2872    Self-Harmonizing Power Core
 *  2875    Sterile Conduits
 *  2876    Wetware Mainframe
 *
 */

#endif

