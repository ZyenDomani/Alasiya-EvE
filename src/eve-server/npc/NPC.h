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
    virtual bool        Load();  // sets orbit range and initalizes the AIMgr

    /* virtual functions for npc/drone AI and player reporting */
    virtual void        ReportDamage(uint8 type=0)      { /* do nothing here */ }  // not used yet
    // tells AI a missile has been launched at us.  allows defender missile code
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
    void                UseHullRepairer();
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

protected:

private:
    NPCAIMgr*           m_AI;
    SpawnMgr*           m_spawnMgr;
    uint32              m_orbitingID;
};

#endif
