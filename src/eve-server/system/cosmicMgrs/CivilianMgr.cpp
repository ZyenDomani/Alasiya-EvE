
/**
 * @name CivilianMgr.cpp
 *     Civilian (non-combatant NPC) managment system for Alasiya EvEmu
 *
 * @Author:        Allan
 * @date:          12 Feb 2017
 *
 */


#include "eve-server.h"

#include "EVEServerConfig.h"
#include "PyServiceMgr.h"
#include "system/cosmicMgrs/CivilianMgr.h"

/*  this class will be in charge of all non-combatant npcs ingame.
 * the purpose here is to simulate civilian actions by having ships travel from station to station.
 * this will also include jumping systems
 *
 *  mostly, it's simple AI to run a ship from point A to point B to simulate 'traffic' in the system
 *  as the system matures, it may be possible for 'shadier' npcs to travel to unknown areas
 *     (the astute capsuleer will notice the ship NOT traveling to a planet, gate, or station, and will then know the general area to scan)
 *
 *  this will be a singleton class, in order to span multiple systems.
 */


CivilianMgr::CivilianMgr()
:  m_updateTimer(120000)    // arbitrary 2m default
{
    m_services = nullptr;
    m_updateTimer.Disable();
    m_initalized = false;
}

void CivilianMgr::Initialize(PyServiceMgr* svc) {
    m_services = svc;
    if (!sConfig.cosmic.CiviliansEnabled) {
        sLog.Warning(" Civilian Manager", "Civilian Manager Disabled.");
        return;
    }

    m_initalized = true;
    sLog.Blue(" Civilian Manager", "Civilian Manager Initialized.");
    /* load current data, start timers, process current data, and create new items, if needed */
}

void CivilianMgr::Process() {
    if (!m_initalized) return;
    if (m_updateTimer.Check(false)) {
        /* do something useful here */
    }
}

