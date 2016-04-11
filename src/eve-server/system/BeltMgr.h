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

/* this class will control all aspects of
 * creating, monitoring, removing, logging
 * and saving of all asteroid belts in a single system
 */

class PyServiceMgr;
class SystemManager;

class BeltMgr
{
public:
    BeltMgr(SystemManager* mgr, PyServiceMgr& svc);
    virtual ~BeltMgr();

    void Process();
    void ForceGrowth();

    bool LoadState();
    bool SaveState();

protected:
    void _TriggerGrowth();
    void _Clear();

    Timer m_growthTimer;

    /*  map contains beltID, asteroidSE for entire system */
    std::unordered_multimap<uint32, AsteroidEntity*> m_asteroids;

private:
    SystemManager* m_system;    //we do not own this
    PyServiceMgr& m_services;    //we do not own this

};

#endif  // EVEMU_SYSTEM_BELTMGR_H_


