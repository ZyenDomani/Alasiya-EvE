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


#pragma once

#include "npc/NPCAI.h"
#include "system/cosmicMgrs/SpawnMgr.h"
#include "system/SpaceEntity.h"

class PyServiceMgr;
class InventoryItem;
class Missile;
class Squad;
class SystemManager;

class NPC
: public DynamicSpaceEntity
{
public:
    struct Size {
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
    };

    struct Rank {
        enum  {
            None    = 0,
            Frigate = 1,
            Cruiser = 2,
            Elite   = 3,
            BShip   = 4,
            Commander = 5 // Specialized Overseer/Boss hulls
        };
    };

    // default c'tor
    NPC() =delete;
    // main c'tor
    NPC(InventoryItemRef self, PyServiceMgr& services, SystemManager* system, const FactionData& data, SpawnMgr* spawnMgr = nullptr);
    // copy c'tor
    NPC(const NPC* oth) =delete;
    // move c'tor
    NPC(NPC&& oth) noexcept =default;
    // copy assignment
    NPC& operator=(NPC& oth) =delete;
    // move assignment
    NPC& operator=(NPC&& oth) =default;
    // d'tor
    ~NPC();

    /* class type pointer querys. */
    const char*         GetSEType()             { return "NPC SE"; }
    NPC*        GetNPCSE()                      { return this; }
    /* class type tests. */
    bool        IsNPCSE()                       { return true; }

    /* SpaceEntity interface */
    void        Process();
    void        EncodeDestiny(Buffer& into);

    /* functions default to base class and overridden as needed */
    void        Killed(Damage &fatal_blow);
    bool        Load();  // sets npc resists and inits destiny vars

    /* functions for npc/drone AI and player reporting */
    void        ReportDamage(uint8 type=0, SpaceEntity* pSourceSE=nullptr)
                                                        { m_AI->ReportDamage(type, pSourceSE); }
    // tell AI a missile has been launched at us.  allows defender missile code
    void     MissileLaunched(Missile* pMissile) { m_AI->MissileLaunched(pMissile); }
    void    TargetLost(SpaceEntity* pTargetSE) { m_AI->TargetLost(pTargetSE); }
    void   TargetAdded(SpaceEntity* pTargetSE) { /* do nothing here */ }
    // this is call to inform us of yellowbox
    void   TargetedAdd(SpaceEntity* pTargetSE) { m_AI->Targeted(pTargetSE); }
    void  TargetedLost(SpaceEntity* pTargetSE) { /* do nothing here */ }


    /* specific functions handled here. */
    void                SaveNPC()                       { m_self->SaveItem(); }
        //this is called from SystemManager::RemoveNPC() which calls other SE* methods as needed
    void                RemoveNPC()                     { m_self->Delete(); }
    void                SetResists();
    void                UseArmorRepairer();
    void                UseShieldRecharge();
    void                Orbit(SpaceEntity* pTargetSE);
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

    /* for new squad class */
    void                SetSquad(Squad* squad)          { m_squad = squad; }
    Squad*              GetSquad()                      { return m_squad; }
    uint8               GetRank()                       { return m_rank; }

protected:
    void                SetHauler()                     { m_hauler = true; }

private:
    NPCAI*              m_AI;
    SpawnMgr*           m_spawnMgr;
    Squad*              m_squad;

    uint32              m_orbitingID;
    uint8               m_moduleCount;
    uint8               m_rank;
    bool                m_hauler;
};
