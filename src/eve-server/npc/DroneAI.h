/**
 * DroneAI.h
 *      this class is for drone AI
 *
 * @Author:     Allan
 * @Version:    0.15
 * @Date:       27Nov19
 */

#ifndef __EVEMU_SHIP_DRONEAI_H__
#define __EVEMU_SHIP_DRONEAI_H__

#include "ship/modules/TurretFormulas.h"

// only for drones
namespace DroneAI {
    namespace State {
        enum {
            Invalid           = -1,
            // defined in client
            Idle              = 0,  // not doing anything....idle.
            Combat            = 1,  // fighting - needs target
            Mining            = 2,  // unsure - may need target
            Approaching       = 3,  // too close to chase, but to far to engage
            Departing         = 4,  // return to ship
            Departing2        = 5,  // leaving.  different from Departing
            Pursuit           = 6,  // target out of range to attack/follow, but within npc sight range....use mwd/ab if equiped
            Fleeing           = 7,  // running away
            Operating         = 9,  // whats diff from engaged here? unanchoring?
            Engaged           = 10, // non-combat? - needs target
            // internal only
            Unknown           = 8,  // as stated
            Guarding          = 11,
            Assisting         = 12, //  this will be remote reppers/boosters
            Incapacitated     = 13  //
        };
    }
}

class DroneSE;
class SystemEntity;
class Timer;

class DroneAIMgr {
public:
    DroneAIMgr(DroneSE* pdSE);

    void Init();

    void Process();

    void Abandon();

    void Target(SystemEntity* pTarget);
    void Targeted(SystemEntity *by_who);
    void TargetLost(SystemEntity *by_who);

    void ClearTargets();
    void ClearAllTargets();

    int8 GetState();

    void SetIdle();
    void Return();
    void AssignShip(ShipSE* pSE);
    SystemEntity* GetTarget()                           { return m_targetSE; }

protected:
    void Mine();
    void DoCycle();
    void SetEngaged();
    void ClearTarget();
    void AttackTarget();
    void CheckDistance();

    int8 m_state;
    std::string GetStateName(int8 stateID);

private:
    SystemEntity* m_targetSE;
    DroneSE* m_droneSE;
    ShipSE* m_assignedShip;

    TurretFormulas m_formula;

    EVEItemFlags m_holdFlag;

    Timer m_processTimer;
    Timer m_mainAttackTimer;
    Timer m_beginFindTarget;
    Timer m_warpScramblerTimer;
    Timer m_webifierTimer;

    //uint16 m_targetRange;   // max targeting range  default: m_maxAttackRange (unused)
    uint32 m_maxSpeed;
    uint32 m_cruiseSpeed;
    uint32 m_armorRepairDuration;
    uint32 m_shieldBoosterDuration;

    //cached to reduce access times. (faster but uses more memory)
    float m_maxRange;
    float m_cycleTime;
    float m_sigRadius;
    float m_proximityRange;
    float m_entityFlyRange;
    float m_entityChaseRange;
    float m_entityOrbitRange;
    float m_entityAttackRange;
};

#endif  // __EVEMU_SHIP_DRONEAI_H__