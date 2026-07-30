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


namespace NPCAI {
    //NOTE:  state and action are reversed from drones
    namespace State {
        enum {
            Invalid     = -1,
            Passive     = 0,  // non-combat npcs    no targeting, no weapons
            Delay       = 1,
            Wandering   = 2,
            Idle        = 3,  // not doing anything....idle.
            Chasing     = 4,  // target out of range to attack/follow, but within npc sight range....use mwd/ab if equiped
            Following   = 5,  // too close to chase, but to far to engage
            Engaged     = 6,  // actively fighting
            Assisting   = 7,  // another npc was signaling; this npc is boosting
            Fleeing     = 8,  // running away
            Signaling   = 9,  // calling for help
            WarpOut     = 10,  // leaving bubble
            WarpFollow  = 11   // will follow warping ship to their destination (adv)
        };
    }

    namespace Action {
        enum {
            // these act on m_*target. attack is missile/lazor; action is all others, including self
            Invalid     = -1,
            Passive     = 0,  // non-combat npcs    no targeting, no weapons
            Idle        = 1,  // not doing anything....idle.
            Wandering   = 2,
            Attack      = 3,  // only acts on m_attackTarget
            // the following act on m_actionTarget
            ShieldRep   = 4,  //
            ArmorRep    = 5,  //
            Web         = 6,  //
            Scram       = 7,  //
            EWar        = 8   //
        };
    }

    namespace Size {
        enum {
            None        = 0,
            Swarm       = 35,
            Frigate     = 50,
            Destroyer   = 100,
            Cruiser     = 200,
            BCruiser    = 300,
            BShip       = 350,
            Indy        = 450
        };
    }

    enum Rank {
        None    = 0,
        Frigate = 1,
        Cruiser = 2,
        Elite   = 3,
        BShip   = 4,
        Commander = 5 // Specialized Overseer/Boss hulls
    };
}


#include "ship/modules/TurretFormulas.h"
#include "system/SystemEntity.h"

class Client;
class Missile;
class NPC;
class Timer;

class NPCAIMgr {
public:
    NPCAIMgr(NPC *who);
    ~NPCAIMgr()                                         { /* do nothing here */ }

    void                Init();  // initialize multiple variables after npc/se created

    // this is called from NPC::Process() which is called from SystemManager::Process()
    void                Process();

    void                Target(SystemEntity* pTargetSE);
    void                Targeted(SystemEntity* pSE);
    void                TargetLost(SystemEntity* pSE);
    void                TargetWarping(SystemEntity* pSE);
    void                ReportDamage(uint8 type=0, SystemEntity* pSourceSE=nullptr);

    void                DisableRepTimers(bool shield=true, bool armor=true);

    // public methods to enable calls from other classes (namely, TurretFormulas.cpp)
    bool                IsIdle()                        { return (m_state == NPCAI::State::Idle); }
    bool                IsFighting();
    uint16              GetOptimalRange()               { return m_optimalRange; }
    int32               GetFalloff()                    { return m_falloffDistance; }
    double              GetTrackingSpeed()              { return m_trackingSpeed; }

    uint16              GetSize()                       { return m_size; }

    uint16              GetMaxSpeed()                   { return m_maxSpeed; }
    uint16              GetCruiseSpeed()                { return m_cruiseSpeed; }

    // npcAI methods
    void                SendGFX(Client* pClient=nullptr);
    void                DisableWarpOutTimer()           { m_warpOutTimer.Disable(); }
    void                WarpOutComplete();

    void                LaunchMissile(uint16 typeID, SystemEntity* pTargetSE);   // us to them
    void                MissileLaunched(Missile* pMissile); // them to us

    void                ShipArrived(Client* pClient);

protected:
    // idle.  not doing anything
    void                SetIdle();
    void                WarpOut();
    // this will check for all action/attack types (lazor, ewar, tackle)
    void                DoAction();
    // no targets in sight.  wandering around until warp out or targets appear
    void                SetWander();
    // running away....use m_maxSpeed then warp away when out of range (does this make sense??)
    void                SetFleeing(SystemEntity* pTargetSE);
    void                ClearTarget(SystemEntity* pTargetSE);
    // calling for help..use m_orbitSpeed *2 to speed tank while calling for reinforcements
    void                SetSignaling(SystemEntity* pTargetSE);
    // updates target distance to determine action
    void                CheckDistance(SystemEntity* pTargetSE);
    // advanced method to pick preferred target
    void                PickTarget();
    // NOTE:  must call destiny->follow (or another movement call) after changing speed
    void                ChangeSpeed();
    // Method to broadcast misc tidbits to local channel of all players in bubble
    void                BcastLocal(uint8 state);

    //actual attacking methods
    // target within npc sight range. range 4+  use m_maxSpeed to get within falloff
    void                SetChasing(SystemEntity* pTargetSE);
    // between optimal and falloff.  range 3   try to get closer, but still orbiting and attacking
    void                SetFollowing(SystemEntity* pTargetSE);
    // actively fighting (in orbit).  range 2  use m_orbitSpeed.
    void                SetEngaged(SystemEntity* pTargetSE);
    // only for using lasers
    void                ShootTarget();
    // only for modules (not pewpew or missiles)
    void                EffectTarget();

    // for npcs that can rep
    void                Heal();
    // for npcs that have modules
    void                UseModule();

    bool                InFlyRange(SystemEntity* pTargetSE);            // near    - range 1
    bool                InOptimalRange(SystemEntity* pTargetSE);        // close   - range 2
    bool                InFalloffDistance(SystemEntity* pTargetSE);     // mid     - range 3
    bool                InAttackRange(SystemEntity* pTargetSE);         // far     - range 4
    bool                InChaseRange(SystemEntity* pTargetSE);          // distant - range 5
    bool                InSightRange(SystemEntity* pTargetSE);          // sight   - range 6

    // checks attack target only
    bool                VerifyTarget();

    void                ClearAllTimers();
    void                SetActionTimers();
    void                SetAttackTimers();
    void                ClearAttackTimers();

    uint16              GetTargetingTime();

    const char*         GetStateName(int8 stateID);
    const char*         GetSizeName();

    // advanced AI methods
    void                SwitchTarget();
    void                Guard(SystemEntity* pTargetSE); // for now, orbit? target ship at 1/2 orbit range
    void                Assist(SystemEntity* pTargetSE);// use reppers on target
    SystemEntity*       FindSecondaryTarget();
    float               AggroModifiers(SystemEntity* pTargetSE);
    SystemEntity*       EvaluateThreats();
    void                ExecuteCombatMovement(SystemEntity* pPlayerTarget);

private:
    NPC*                myNPC;
    SystemEntity*       m_actionTarget;
    SystemEntity*       m_attackTarget;
    DestinyManager*     m_destiny;
    InventoryItemRef    m_self;

    TurretFormulas      m_formula;

    bool                m_useSigRadius     :1;
    bool                m_useSecondTarget  :1;
    bool                m_useTargSwitching :1;

    int8                m_state;
    int8                m_action;
    uint8               m_maxLockedTargets;
    uint16              m_size;
    // do i need to separate fxID and gunID for action/attack?  probably so
    uint16              m_effectID;
    uint16              m_turretID;

    //these attributes are cached to reduce access times. (faster but uses more memory)

    uint16              m_maxSpeed;                     // mwd speed
    uint16              m_cruiseSpeed;                  // normal and orbit speed

    int32               m_actionSpeed;
    int32               m_attackSpeed;
    int32               m_armorRepairDuration;
    int32               m_shieldBoosterDuration;

    float               m_sigRadModifier;

    //in order of distance  far to close
    double              m_sightRange;                   //[6] npc sight range
    double              m_sightRangeSq;
    double              m_attackRange;                  //[5] maximum engagement distance
    double              m_attackRangeSq;
    double              m_chaseRange;                   //[4] min distance to activate mwd, if equipped
    double              m_chaseRangeSq;
    double              m_falloffDistance;              //[3] distance past optimal where accuracy has fallen by half
    double              m_falloffDistanceSq;
    double              m_optimalRange;                 //[2] max distance range does not affect the to-hit equation.
    double              m_optimalRangeSq;
    double              m_flyRange;                     //[1] distance the npc orbits
    double              m_flyRangeSq;

    int64               m_actionTime;
    int64               m_attackTime;                   // timestamp when attack started
    int64               m_chaseTimeEnd;                 // timestamp when npc chasing will end (maxChaseDuration)

    float               m_switchTargChance;             //fuzzy logic
    float               m_trackingSpeed;
    float               m_damageMultiplier;
    float               m_armorRepairDelayChance;
    float               m_shieldBoosterDelayChance;

    Timer               m_actionTimer;                  // action timer - module use, not lazors
    Timer               m_attackTimer;                  // main attack (lazer, tackle, ewar, etc)
    Timer               m_missileTimer;                 // missile attack
    Timer               m_shieldBoosterTimer;           // sebo
    Timer               m_armorRepairTimer;             // repper
    Timer               m_beginFindTarget;              // main targeting timer (used as delay after warp-in)
    Timer               m_warpOutTimer;                 // as stated
    Timer               m_retargetTimer;                // comfort breaker (allow npcs to change targets)

    // not sure how im gonna do this yet...160 fx types
    std::vector<TypeEffects>   m_effectMap;             //  all 'modules' this npc has (using effect data)

    std::map<int8, uint16>     m_attackFxMap;           // (NPCAI::State, fxID)
    std::map<int8, uint16>     m_defendFXMap;           // (NPCAI::State, fxID)
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