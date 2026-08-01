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


#ifndef __NPC_H_INCL__
#define __NPC_H_INCL__

#include "npc/NPCAI.h"
#include "system/cosmicMgrs/SpawnMgr.h"
#include "system/SystemEntity.h"

class PyServiceMgr;
class DestinyManager;
class InventoryItem;
class Missile;
class NPCSquad;
class SystemManager;

class NPC
: public DynamicSystemEntity
{
public:
    NPC(InventoryItemRef self, PyServiceMgr& services, SystemManager* system, const FactionData& data, SpawnMgr* spawnMgr = nullptr);
    virtual ~NPC();

    /* class type pointer querys. */
    virtual const char*         GetSEType()             { return "NPC SE"; }
    virtual NPC*        GetNPCSE()                      { return this; }
    /* class type tests. */
    virtual bool        IsNPCSE()                       { return true; }

    /* SystemEntity interface */
    virtual void        Process();
    virtual void        EncodeDestiny(Buffer& into);

    /* virtual functions default to base class and overridden as needed */
    virtual void        Killed(Damage &fatal_blow);
    virtual bool        Load();  // sets npc resists and inits destiny vars

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

    NPCAIMgr*           GetAI()                         { return m_AI; }
    SpawnMgr*           GetSpawnMgr()                   { return m_spawnMgr; }

    /* for command dropLoot - commands all npcs in bubble to jettison loot */
    void                CmdDropLoot();

    /* advanced ai methods */
    void                ApplyTrackingBoost(float mod=1.0f);

    /* for new squad class */
    void                SetSquad(NPCSquad* squad)       { m_squad = squad; }
    void                SetSquadLeader(bool set=false)  { m_squadLeader = set; }
    bool                IsSquadLeader()                 { return m_squadLeader; }
    void                SetCommandRank(uint8 set=0)     { m_rank = set; }
    uint8               GetCommandRank()                { return m_rank; }
    NPCSquad*           GetSquad()                      { return m_squad; }

protected:
    void                SetHauler()                     { m_hauler = true; }

private:
    NPCAIMgr*           m_AI;
    SpawnMgr*           m_spawnMgr;
    NPCSquad*           m_squad;

    bool                m_hauler;
    bool                m_squadLeader;
    uint8               m_moduleCount;
    uint8               m_rank;
    uint32              m_orbitingID;
};


namespace Squad {
    namespace Tier {
        enum {
            Rookie      = 1,
            Soldier     = 2,
            Veteran     = 3,
            Elite       = 4,
            Apex        = 5
        };
    }
}

// A lightweight, transient group coordinator
class NPCSquad {
public:
    NPCSquad(uint32 squadID) : m_formationBreakTimer(0), m_tacticalTier(0), m_squadID(squadID),
                               m_squadLeader(nullptr), m_squadTarget(nullptr)  {}
    ~NPCSquad()                                         { m_members.clear(); }

    void                RegisterMember(NPC* pNPC);
    void                UnregisterMember(NPC* pNPC);
    void                OnAllMembersArrived();

    // The Master Focus-Fire Hook
    SystemEntity*       GetSquadTarget()                { return m_squadTarget; }
    void                SetSquadTarget(SystemEntity* pTarget)
                                                        { m_squadTarget = pTarget; }

    uint16              GetID()                         { return m_squadID; }
    uint8               GetFormID()                     { return m_formationID; }
    float               GetSpacing()                    { return m_spacing; }
    uint8               GetTier()                       { return m_tacticalTier; }

    // Formation Handles
    void                AssignLeader(NPC* pNPC)         { m_squadLeader = pNPC; }
    NPC*                GetLeader()                     { return m_squadLeader; }
    std::vector<NPC*>&  GetMembers()                    { return m_members; }

private:
    Timer               m_formationBreakTimer;
    uint8               m_tacticalTier;
    uint32              m_squadID;
    uint8               m_formationID;
    float               m_spacing;
    NPC*                m_squadLeader;
    SystemEntity*       m_squadTarget;
    std::vector<NPC*>   m_members;     // Safe transient references to active grid rats
};

#endif
