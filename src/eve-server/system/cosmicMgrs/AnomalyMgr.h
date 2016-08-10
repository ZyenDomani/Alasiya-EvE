/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Allan
*/

#ifndef EVEMU_SYSTEM_ANOMALYMGR_H_
#define EVEMU_SYSTEM_ANOMALYMGR_H_

/*  this class is in charge of creating/destroying and maintaining
 * anomalies in a system.
 *
 *  a new iteration of this class is created for each system as that system is booted.
 */

#include "system/cosmicMgrs/ManagerDB.h"


class DungeonMgr;
class AsteroidBeltMgr;
class PyServiceMgr;
class SpawnMgr;
class SystemManager;

class AnomalyMgr
{
  public:
      AnomalyMgr(SystemManager* mgr, PyServiceMgr& svc);
      virtual ~AnomalyMgr()                             { /* do nothing here */ }

      void Init(AsteroidBeltMgr* beltMgr, DungeonMgr* dungMgr, SpawnMgr* spawnMgr);
      void Process();

      void SaveAnomaly();

protected:
    /* we do not own any of these */
    ManagerDB m_db;

private:
    /* we do not own any of these */
    AsteroidBeltMgr* m_beltMgr;
    DungeonMgr* m_dungMgr;
    SpawnMgr* m_spawnMgr;
    SystemManager* m_system;
    PyServiceMgr& m_services;

    bool m_initalized;

};

#endif  // EVEMU_SYSTEM_ANOMALYMGR_H_

