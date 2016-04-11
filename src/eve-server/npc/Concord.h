/**
 * @name Concord.h
 *   this file is for Concord AI and other concord-specific code as it is written.
 *
 * @Author:         Allan
 * @date:   09 March 2016
 */


#ifndef EVE_SYSTEM_CONCORD_H
#define EVE_SYSTEM_CONCORD_H

#include "eve-server.h"

#include "ship/modules/weapon_modules/TurrentFormulas.h"
#include "system/SystemEntity.h"

/*  basic notes for concord actions...
 *
 * response is 0.5 - 1.0, heavy in 1.0, light in 0.5, nil < 0.5
 * 0.1 - 0.4 has guns for gates n stations according to sec status (0.1 = 1, 0.4 = 4)
 *
 *
 */

/** @todo this needs to be updated for new SE class..*/

class PyServiceMgr;
class DestinyManager;
class InventoryItem;
class SystemManager;
class ServiceDB;
class ConcordSpawnMgr;
class ConcordAI;

class Concord
: public DynamicSystemEntity
{
public:
    Concord(
        SystemManager* system,
        PyServiceMgr& services,
        InventoryItemRef self,
        const GPoint &position,
        ConcordSpawnMgr* spawnMgr = nullptr);

    virtual ~Concord();

    void Orbit(SystemEntity* who);

    void Process();
    void EncodeDestiny( Buffer& into ) const;

    bool ApplyDamage(Damage& d);
    void MakeDamageState(DoDestinyDamageState& into) const;
    void Killed(Damage& fatal_blow);

    void TargetAdded(SystemEntity* who) {}
    void TargetLost(SystemEntity* who);
    void TargetedAdd(SystemEntity* who);
    void TargetedLost(SystemEntity* who) {}
    void TargetsCleared() {}

    void UseShieldRecharge();
    void UseArmorRepairer();

    float GetThermal() { return m_therDamage; }
    float GetEM() { return m_emDamage; }
    float GetKinetic() { return m_kinDamage; }
    float GetExplosive() { return m_expDamage; }

    double GetOrbitRange();

    SystemManager* SystemMgr() const { return m_system; }
    ConcordAI* AI() const { return m_AI; }

protected:
    void _AwardBounty(SystemEntity* who);
    void _DropLoot(uint32 groupID, uint32 owner, uint32 locationID);
    void _UpdateDamage();

    SystemManager* const m_system;    //we do not own this
    PyServiceMgr& m_services;    //we do not own this
    ConcordSpawnMgr* m_spawnMgr;    //we do not own this, may be NULL
    ConcordAI* m_AI;    //never NULL

private:
    uint32 m_orbitingID;

    double m_hullDamage;
    double m_armorDamage;
    double m_shieldCharge;
    double m_shieldCapacity;

    double m_emDamage;
    double m_expDamage;
    double m_kinDamage;
    double m_therDamage;
};

class ConcordAI
{
public:
    ConcordAI(Concord* who);
    virtual ~ConcordAI()     { /* do nothing here */ }

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
    void _EnterIdle();
    void _EnterChasing(SystemEntity* pTarget);
    void _EnterFollowing(SystemEntity* pTarget);
    void _EnterEngaged(SystemEntity* pTarget);
    void _EnterFleeing(SystemEntity* pTarget);
    void _EnterSignaling(SystemEntity* pTarget);
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
    //cached to reduce access times. (faster but uses more memory)
    double m_radius;
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

    Concord* m_npc;

    TurrentFormulas m_formula;

    Timer m_processTimer;
    Timer m_mainAttackTimer;
    Timer m_shieldBoosterTimer;
    Timer m_armorRepairTimer;
    Timer m_beginFindTarget;
    Timer m_warpScramblerTimer;
    Timer m_webifierTimer;

};

class ConcordSpawnMgr
{
public:
    ConcordSpawnMgr();
    virtual ~ConcordSpawnMgr()     { /* do nothing here */ }

private:

};
#endif  // EVE_SYSTEM_CONCORD_H
