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
    Author:        Zhur
    Updates:    Allan
*/

#ifndef __NPCAI_H_INCL__
#define __NPCAI_H_INCL__

#include "ship/modules/weapon_modules/TurrentFormulas.h"

class NPC;
class SystemEntity;
class Timer;
class EvilNumber;

class NPCAIMgr {
public:
    NPCAIMgr(NPC *who);
    ~NPCAIMgr()                 { /* do nothing here */ }

    void Process();

    void Target(SystemEntity *by_who);
    void Targeted(SystemEntity *by_who);
    void TargetLost(SystemEntity *by_who);

    void ClearTargets();
	void ClearAllTargets();

    void DisableRepTimers();

    // public methods to enable calls from other classes

protected:
    void Attack(SystemEntity* pTarget);
    void AttackTarget(SystemEntity* pTarget);
    void Wander();
    void EnterIdle();
    void EnterChasing(SystemEntity* pTarget);
    void EnterFollowing(SystemEntity* pTarget);
    void EnterEngaged(SystemEntity* pTarget);
    void EnterFleeing(SystemEntity* pTarget);
    void EnterSignaling(SystemEntity* pTarget);
    void _CheckDistance(SystemEntity* pTarget);
    void _SendWeaponEffect(const char *effect, SystemEntity* pTarget);

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
    bool m_isWandering;
    //cached to reduce access times. (faster but uses more memory)
    uint16 m_falloff;
    uint16 m_maxSpeed;
    uint16 m_ROF;
    uint16 m_orbitSpeed;
    uint16 m_targetRange;
    uint16 m_damageMultiplier;
    uint16 m_optimalRange;
    uint16 m_boostRange;
    uint16 m_armorRepairDuration;
    uint16 m_shieldBoosterDuration;

    uint32 m_radius;
    uint32 m_flyRange;
    uint32 m_sightRange;
    uint32 m_maxAttackRange;

    float m_armorRepairChance;
    float m_shieldBoosterChance;

    NPC* m_npc;

    TurrentFormulas m_formula;

    Timer m_processTimer;
    Timer m_mainAttackTimer;
    Timer m_shieldBoosterTimer;
    Timer m_armorRepairTimer;
	Timer m_beginFindTarget;
    Timer m_warpScramblerTimer;
    Timer m_webifierTimer;
};

#endif