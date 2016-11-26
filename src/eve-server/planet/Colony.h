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
    Colony(PyServiceMgr* mgr, Client* pclient, SystemEntity* pSE);
    ~Colony();

    void Init();
    void Load();
    void Save();
    void Update();
    void AbandonColony();

    void Process();
    void ProcessECUs(bool& save);
    void ProcessSilos(bool& save);
    void ProcessPlants(bool& save);

    void RemovePin(uint32 pinID);
    void RemoveLink(uint32 linkID);
    void RemoveRoute(uint32 routeID);

    void UpgradeLink(uint32 linkID, uint8 level);
    void UpgradeCommandCenter(uint32 pinID, uint8 level);

    void CreatePin(uint32 groupID, uint32 pinID, uint32 typeID, double latitude, double longitude);
    void CreateLink(uint32 src, uint32 dest, uint32 level);
    void CreateRoute(uint8 routeID, uint32 typeID, uint32 qty, PyList* path);
    void CreateCommandPin(uint32 itemID, uint32 typeID, double latitude, double longitude);
    void CreateExtractorHead();

    void AddExtractorHead(uint32 ecuID, uint32 pinID, double latitude, double longitude);
    void MoveExtractorHead(uint32 ecuID, uint32 pinID, double latitude, double longitude);

    void InstallProgram(uint32 ecuID, uint16 typeID, float headRadius);
    void SetSchematic(uint32 pinID, uint8 schematicID);
    void SetProgramResults(uint32 ecuID, uint16 typeID, uint16 numCycles, float headRadius, float cycleTime);

    void LaunchCommodities(uint32 pinID, std::map<uint16, uint16>& items);

    uint32 GetHeadType(uint16 ecuTypeID, uint16 resTypeID);

    PyRep* GetColony();
    PyTuple* GetPins();
    PyTuple* GetLinks();
    PyTuple* GetRoutes();

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

    uint16 m_pg = 0;
    uint16 m_cpu = 0;
    uint32 m_colonyID = 0;

    std::map<uint8, uint32> tempPinIDs;
    std::map<uint8, PI_Heads> tempHeadIDs;
};


#endif

