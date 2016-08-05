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
  m_services(svc)
{
    m_initalized = false;
}

void AnomalyMgr::Init(AsteroidBeltMgr* beltMgr, DungeonMgr* dungMgr, SpawnMgr* spawnMgr)
{
    m_beltMgr = beltMgr;
    m_dungMgr = dungMgr;
    m_spawnMgr = spawnMgr;

    if (!sConfig.cosmic.AnomalyEnabled) {
        _log(COSMIC_MGR__MESSAGE, "Anomaly System Disabled.  Not Initalizing Anomaly Manager for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
        return;
    }

    m_initalized = true;

    _log(COSMIC_MGR__MESSAGE, "AnomalyMgr Initialized for %s(%u)", m_system->GetName().c_str(), m_system->GetID());
}

void AnomalyMgr::Process() {
    if (!m_initalized)
        return;

}

/* eventually,this will be the ONLY save routine for Anomalies/Signatures.
 * for now, dungeon anoms are saved in DungeonMgr
 */
void AnomalyMgr::SaveAnomaly()
{
    uint8 scanGroupID = ScanGroupAnomaly;
    uint16 groupID = EVEDB::invGroups::Cosmic_Anomaly; //885
    uint16 groupID2 = EVEDB::invGroups::Cosmic_Signature; //502
    uint16 typeID = 28356; // Cosmic_Anomaly - dont need probes or sklls
    uint16 typeID2 = 25880; // Cosmic_Signature - need probes and skills (exploring)

    uint16 strengthAttributeID = AttrScanAllStrength;

    CosmicSignature sig;
    /* will need a bit of code here to check and set all items correctly for saving -- see below*/
    m_db.SaveAnomaly(sig);

}
