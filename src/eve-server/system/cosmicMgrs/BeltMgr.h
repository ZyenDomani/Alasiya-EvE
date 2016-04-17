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
 *    Author:        Allan
 */

#ifndef EVEMU_SYSTEM_BELTMGR_H_
#define EVEMU_SYSTEM_BELTMGR_H_

#include <unordered_map>
#include "system/Asteroid.h"
#include "system/SystemEntity.h"
#include "system/cosmicMgrs/ManagerDB.h"

/*  this class will keep track of all asteroid belts in a system
 * it is created on a per-system basis, and will also deal with
 * calling spawn/delete/grow functions for each belt.
 *
 * this class will also be in charge of belts in anomalies
 *
 *  a new iteration of this class is created for each system as that system
 * is booted.
 */


class PyServiceMgr;
class SystemManager;

class BeltMgr
{
public:
    BeltMgr(SystemManager* mgr, PyServiceMgr& svc);
    virtual ~BeltMgr();

    void Init();
    void Save();
    void Process();
    void ForceGrowth();
    void RegisterBelt(InventoryItemRef itemRef);
    void ClearBelt();

    bool Load(uint32 beltID);
    bool IsSpawned(uint32 beltID);
    bool IsSpawned(InventoryItemRef itemRef);
    bool CheckSpawn(InventoryItemRef itemRef);

    void GetList(uint32 beltID, std::vector< AsteroidEntity* >& list);

protected:
    ManagerDB m_db;
    Timer m_growthTimer;

    void _TriggerGrowth();
    void Clear();
    void ClearAll();
    void SpawnBelt(InventoryItemRef itemRef);
    void SpawnAsteroid(uint32 beltID, uint32 typeID, double radius, const GPoint& position);

    uint32 GetAsteroidType(double p, const std::map<float, uint32>& roids);

private:
    SystemManager* m_system;    //we do not own this
    PyServiceMgr& m_services;    //we do not own this

    bool m_initialized;
    uint32 m_systemID;

    /* map contains beltID, boolean for IsSpawned() */
    std::map<uint32, bool> m_spawned;
    /* vector contains belt's itemID, itemRef */
    std::map<uint32, InventoryItemRef> m_belts;
    /*  this map contains beltID, asteroidSE for entire system */
    std::unordered_multimap<uint32, AsteroidEntity*> m_asteroids;


};

#endif  // EVEMU_SYSTEM_BELTMGR_H_


