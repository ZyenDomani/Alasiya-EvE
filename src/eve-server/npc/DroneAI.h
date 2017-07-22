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
    Author:     Allan (copied from NPC AI)
*/

#ifndef __EVEMU_SHIP_DRONEAI_H__
#define __EVEMU_SHIP_DRONEAI_H__

#include "ship/modules/TurretFormulas.h"

class Drone;
class SystemEntity;
class Timer;
class EvilNumber;

class DroneAIMgr {
public:
    DroneAIMgr(Drone *who);

    void Process();

    void Target(SystemEntity *by_who);
    void Targeted(SystemEntity *by_who);
    void TargetLost(SystemEntity *by_who);

    void ClearTargets();
	void ClearAllTargets();

    void DisableRepTimers();

protected:
    void Attack(SystemEntity* pTarget);
    void AttackTarget(SystemEntity* pTarget);
    void SetIdle();
    void SetChasing(SystemEntity* pTarget);
    void SetFollowing(SystemEntity* pTarget);
    void SetEngaged(SystemEntity* pTarget);
    void SetFleeing(SystemEntity* pTarget);
    void SetSignaling(SystemEntity* pTarget);
    void CheckDistance(SystemEntity* pTarget);

    double GetTargetTime();

    typedef enum {
        Idle,       // not doing anything....idle.
        Chasing,    // target out of range to attack/follow, but within npc sight range....use mwd/ab if equiped
        Following,  // too close to chase, but to far to engage
        Engaged,    // actively fighting
        Fleeing,    // running away
        Signaling   // calling for help
    } State;
    State m_state;

private:
    //cached to reduce access times. (faster but uses more memory)
    double m_sigRadius;
    double m_attackSpeed;
    double m_entityFlyRange;
    double m_entityOrbitRange;
    double m_entityChaseRange;
    double m_entityAttackRange;
    double m_armorRepairChance;
    double m_shieldBoosterChance;

    uint32 m_chaseSpeed;
    uint32 m_cruiseSpeed;
    uint32 m_targetRange;
    uint32 m_armorRepairDuration;
    uint32 m_shieldBoosterDuration;

    Drone* m_drone;

    TurretFormulas m_formula;

    Timer m_processTimer;
    Timer m_mainAttackTimer;
    Timer m_shieldBoosterTimer;
    Timer m_armorRepairTimer;
	Timer m_beginFindTarget;
    Timer m_warpScramblerTimer;
    Timer m_webifierTimer;
};

#endif  // __EVEMU_SHIP_DRONEAI_H__