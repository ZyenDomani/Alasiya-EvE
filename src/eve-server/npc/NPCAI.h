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

#include "ship/modules/TurretFormulas.h"

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

    // public methods to enable calls from other classes (namely, TurretFormulas.cpp)
    bool IsIdle()                                       { return (m_state == Idle); }
    bool IsFighting()                                   { return (m_state != Idle); }
    uint16 GetMaxRange()                                { return m_optimalRange; }
    uint32 GetFalloff()                                 { return m_falloff; }
    uint32 GetAttackRange()                             { return m_maxAttackRange; }
    double GetTrackingSpeed()                           { return m_trackingSpeed; }

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

    enum State {
        Idle        = 1,  // not doing anything....idle.
        Chasing     = 2,  // target out of range to attack/follow, but within npc sight range....use mwd/ab if equiped
        Following   = 3,  // too close to chase, but to far to engage
        Engaged     = 4,  // actively fighting
        Fleeing     = 5,  // running away
        Signaling   = 6   // calling for help
    };
    State m_state;

    std::string GetStateName(State name);

private:
    bool m_webber : 1;
    bool m_warpScram : 1;
    bool m_isWandering : 1;
    bool m_useSigRadius : 1;
    bool m_useTargSwitching : 1;
    bool m_useSecondTarget : 1;
    float m_switchTargChance;   //fuzzy logic
    uint16 m_preferedSigRadius;
    //these attributes are cached to reduce access times. (much faster but uses more memory)
    uint16 m_maxSpeed;
    uint16 m_ROF;
    uint16 m_orbitSpeed;
    uint16 m_targetRange;   // max targeting range  default: m_maxAttackRange (unused)
    uint16 m_damageMultiplier;
    uint16 m_optimalRange;
    uint16 m_boostRange;    // distance for Speed Boost activation   default:2500
    uint16 m_armorRepairDuration;
    uint16 m_shieldBoosterDuration;

    uint32 m_radius;
    uint32 m_falloff;// distance past maximum range at which accuracy has fallen by half
    uint32 m_flyRange;  // npc tries to stay at this distance from active target    default:500
    uint32 m_sightRange;
    uint32 m_maxAttackRange;// max firing range   default:15000

    float m_armorRepairChance;
    float m_shieldBoosterChance;

    double m_trackingSpeed;

    NPC* m_npc;

    TurretFormulas m_formula;

    Timer m_processTimer;
    Timer m_mainAttackTimer;
    Timer m_shieldBoosterTimer;
    Timer m_armorRepairTimer;
	Timer m_beginFindTarget;
    Timer m_warpScramblerTimer;
    Timer m_webifierTimer;
};

#endif