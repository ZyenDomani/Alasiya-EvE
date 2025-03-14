/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
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
    Rewrite:    Allan
*/

#ifndef __NPCAI_H_INCL__
#define __NPCAI_H_INCL__

#include "ship/Missile.h"
#include "ship/modules/TurretFormulas.h"

namespace NPCAI {
    namespace State {
        enum {
            Invalid     = -1,
            Idle        = 1,  // not doing anything....idle.
            Chasing     = 2,  // target out of range to attack/follow, but within npc sight range....use mwd/ab if equiped
            Following   = 3,  // too close to chase, but to far to engage
            Engaged     = 4,  // actively fighting
            Fleeing     = 5,  // running away
            Signaling   = 6,  // calling for help
            WarpOut     = 7,  // leaving bubble
            WarpFollow  = 8   // will follow warping ship to their destination (adv)
        };
    }
}

class NPC;
class SystemEntity;
class Timer;

class NPCAIMgr {
public:
    NPCAIMgr(NPC *who);
    ~NPCAIMgr()                                         { /* do nothing here */ }

    // this is called from NPC::Process() which is called from SystemManager::Process()
    void Process();

    void Target(SystemEntity* pTargetSE);
    void Targeted(SystemEntity* pSE);
    void TargetLost(SystemEntity* pSE);

    void DisableRepTimers(bool shield=true, bool armor=true);

    // public methods to enable calls from other classes (namely, TurretFormulas.cpp)
    bool IsIdle()                                       { return (m_state == NPCAI::State::Idle); }
    bool IsFighting();
    uint16 GetOptimalRange()                            { return m_optimalRange; }
    uint16 GetSigRes()                                  { return m_sigResolution; }
    uint32 GetFalloff()                                 { return m_falloff; }
    uint32 GetAttackRange()                             { return m_maxAttackRange; }
    double GetTrackingSpeed()                           { return m_trackingSpeed; }

    // npcAI methods
    void DisableWarpOutTimer()                          { m_warpOutTimer.Disable(); }
    void WarpOutComplete()                              { m_warpOutTimer.Disable(); m_state = NPCAI::State::Idle; }

    void LaunchMissile(uint16 typeID, SystemEntity* pTargetSE);   // us to them
    void MissileLaunched(Missile* pMissile); // them to us

protected:
    void Attack(SystemEntity* pTargetSE);
    void SetIdle();
    void WarpOut();
    void SetWander();
    void SetChasing(SystemEntity* pTargetSE);
    void SetEngaged(SystemEntity* pTargetSE);
    void SetFleeing(SystemEntity* pTargetSE);
    void ClearTarget(SystemEntity* pTargetSE);
    void SetFollowing(SystemEntity* pTargetSE);
    void SetSignaling(SystemEntity* pTargetSE);
    void AttackTarget(SystemEntity* pTargetSE);
    void CheckDistance(SystemEntity* pTargetSE);

    float GetTargetTime();

    int8 m_state;
    int8 m_action;

    std::string GetStateName(int8 stateID);

    // checks if target is within m_orbitRange to orbit target
    bool                InOrbitDistance(SystemEntity* pTargetSE);         // near - range 1
    // checks if target is within m_falloffRange to move closer to target
    bool                InFalloffDistance(SystemEntity* pTargetSE);       // close - range 2
    // checks if target is within m_engageDistance to engage with target
    bool                InEngageDistance(SystemEntity* pTargetSE);        // mid - range 3
    // checks if target is within m_chaseRange to chase target
    bool                InChaseDistance(SystemEntity* pTargetSE);         // far - range 4
    // checks if target is within m_maxRange of target
    bool                InMaxDistance(SystemEntity* pTargetSE);           // distant - range 5


private:
    NPC* myNPC;
    DestinyManager* m_destiny;
    InventoryItemRef m_self;

    TurretFormulas m_formula;

    bool m_webber           :1;
    bool m_warpScram        :1;
    bool m_isWandering      :1;
    bool m_useSigRadius     :1;
    bool m_useTargSwitching :1;
    bool m_useSecondTarget  :1;

    float m_switchTargChance;   //fuzzy logic
    uint16 m_preferedSigRadius;

    //these attributes are cached to reduce access times. (faster but uses more memory)
    uint8 m_maxAttackTargets;
    uint8 m_maxLockedTargets;
    uint16 m_attackSpeed;
    uint16 m_missileTypeID;
    uint16 m_launcherCycleTime;
    uint16 m_sigResolution;
    uint16 m_orbitSpeed;
    uint16 m_targetRange;   // max targeting range  default: m_maxAttackRange (unused)
    uint16 m_optimalRange;
    uint16 m_boostRange;    // distance for Speed Boost activation   default:2500
    uint16 m_armorRepairDuration;
    uint16 m_shieldBoosterDuration;

    uint32 m_sigRadius;
    uint32 m_falloff;// distance past maximum range at which accuracy has fallen by half
    uint32 m_flyRange;  // npc tries to stay at this distance from active target    default:500
    uint32 m_sightRange;
    uint32 m_maxAttackRange;// max firing range   default:15000
    uint32 m_warpScramRange;

    //in order of distance  far to close  (from droneAI code)
    int32               m_maxDistance;                  //[5] maximum engagement distance
    int32               m_chaseDistance;                //[4] min distance to activate mwd, if equipped
    int32               m_engageDistance;               //[3] max distance drone will engage a target
    int32               m_falloffDistance;              //[2] distance where accuracy has fallen by half
    int32               m_orbitDistance;                //[1] distance the drone orbits

    uint16              m_maxSpeed;                     // mwd speed
    uint32              m_cruiseSpeed;                  // normal speed

    int64               m_startTime;                    // timestamp when effect started


    float m_warpScramChance;
    float m_armorRepairDelayChance;
    float m_shieldBoosterDelayChance;

    double m_trackingSpeed;
    double m_damageMultiplier;

    Timer m_processTimer;
    Timer m_mainAttackTimer;
    Timer m_missileTimer;
    Timer m_shieldBoosterTimer;
    Timer m_armorRepairTimer;
    Timer m_beginFindTarget;
    Timer m_warpOutTimer;
    Timer m_warpScramblerTimer;
    Timer m_webifierTimer;
};

#endif

/*  these are only for sleepers (hence the high attr #'s)
    AttrAI_ShouldUseTargetSwitching = 1648,
    AttrAI_ShouldUseSecondaryTarget = 1649,
    AttrAI_ShouldUseSignatureRadius = 1650,
    AttrAI_ChanceToNotTargetSwitch = 1651,
    AttrAI_ShouldUseEffectMultiplier = 1652,
    AttrAI_ImmuneToSuperWeapon = 1654,
    AttrAI_PreferredSignatureRadius = 1655,
    AttrAI_TankingModifierDrone = 1656,
    AttrAI_TankingModifier = 1657,
    */