/**
 * SwarmSE.h
 *      this class is for rogue drones
 *
 * @Author:     Allan
 * @Version:    1.03
 * @AI Version: 0.95
 * @Date:       14Jul26  (copied from NPC.h)
 */


#pragma once

#include "npc/NPC.h"
#include "npc/SwarmAI.h"
#include "system/cosmicMgrs/SpawnMgr.h"
#include "system/SystemEntity.h"

class PyServiceMgr;
class DestinyManager;
class InventoryItem;
class Missile;
class SystemManager;

class SwarmSE
: public NPC
{
public:
    SwarmSE(InventoryItemRef self, PyServiceMgr& services, SystemManager* system, const FactionData& data, SpawnMgr* spawnMgr = nullptr);
    virtual ~SwarmSE();

    /* class type pointer querys. */
    virtual const char*         GetSEType()             { return "SwarmSE"; }
    virtual SwarmSE*        GetNPCSE()                      { return this; }
    virtual SwarmSE*        GetSwarmSE()                      { return this; }
    /* class type tests. */
    virtual bool        IsSwarmSE()                       { return true; }

    /* SystemEntity interface */
    virtual void        Process();
    virtual void        EncodeDestiny(Buffer& into);

    /* virtual functions default to base class and overridden as needed */
    virtual void        Killed(Damage &fatal_blow);
    virtual bool        Load();  // sets resists and inits destiny vars

    /* virtual functions for npc/drone AI and player reporting */
    virtual void        ReportDamage(uint8 type=0, SystemEntity* pSourceSE=nullptr)
                                                        { m_AI->ReportDamage(type, pSourceSE); }
    // tell AI a missile has been launched at us.  allows defender missile code
    virtual void     MissileLaunched(Missile* pMissile) { m_AI->MissileLaunched(pMissile); }
    virtual void    TargetLost(SystemEntity* pTargetSE) { m_AI->TargetLost(pTargetSE); }
    virtual void   TargetAdded(SystemEntity* pTargetSE) { /* do nothing here */ }
    // this is call to inform us of yellowbox
    virtual void   TargetedAdd(SystemEntity* pTargetSE) { m_AI->Targeted(pTargetSE); }
    virtual void  TargetedLost(SystemEntity* pTargetSE) { /* do nothing here */ }


    /* specific functions handled here. */
    void                SaveNPC()                       { m_self->SaveItem(); }
    //this is called from SystemManager::RemoveNPC() which calls other SE* methods as needed
    void                RemoveNPC()                     { m_self->Delete(); }
    void                SetResists();
    void                UseArmorRepairer();
    void                UseShieldRecharge();
    void                Orbit(SystemEntity* pTargetSE);
    void            ForceSetSpawner(SpawnMgr* spawnMgr) { m_spawnMgr = spawnMgr; }

    float               GetThermal()                    { return m_self->GetAttribute(AttrThermalDamage).get_float(); }
    float               GetEM()                         { return m_self->GetAttribute(AttrEmDamage).get_float(); }
    float               GetKinetic()                    { return m_self->GetAttribute(AttrKineticDamage).get_float(); }
    float               GetExplosive()                  { return m_self->GetAttribute(AttrExplosiveDamage).get_float(); }

    NPCAI*              GetAI()                         { return m_AI; }
    SpawnMgr*           GetSpawnMgr()                   { return m_spawnMgr; }

    /* for command dropLoot - commands all npcs in bubble to jettison loot */
    void                CmdDropLoot();

    /* advanced ai methods */
    void                ApplyTrackingBoost(float mod=1.0f);


protected:
    void                SetHauler()                     { m_hauler = true; }

private:
    SwarmAI*            m_AI;
    SpawnMgr*           m_spawnMgr;

    bool                m_hauler;
    uint8               m_moduleCount;
    uint32              m_orbitingID;
};
