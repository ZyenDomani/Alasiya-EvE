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
    Updates:        Allan
*/


#ifndef __NPC_H_INCL__
#define __NPC_H_INCL__

#include "system/cosmicMgrs/SpawnMgr.h"
#include "system/SystemEntity.h"

class PyServiceMgr;
class DestinyManager;
class InventoryItem;
class NPCAIMgr;
class SystemManager;
class ServiceDB;

class NPC
: public DynamicSystemEntity
{
public:
    NPC(InventoryItemRef self, PyServiceMgr &services, SystemManager* system, uint32 corpID, uint32 factionID, SpawnMgr* spawnMgr = nullptr);
    virtual ~NPC();

    /* class type pointer querys. */
    virtual NPC* GetNPCSE()                             { return this; }
    /* class type tests. */
    virtual bool IsNPCSE()                              { return true; }

    /* SystemEntity interface */
    virtual void Process();
    virtual void TargetLost(SystemEntity* who);
    virtual void TargetedAdd(SystemEntity* who);
    virtual void EncodeDestiny(Buffer& into);

    /* virtual functions default to base class and overridden as needed */
    virtual void Killed(Damage &fatal_blow);    /* This method is defined in Damage.cpp */

    /* specific functions handled here. */
    void SaveNPC();
    void RemoveNPC();
    void SetResists();
    void UseHullRepairer();
    void UseArmorRepairer();
    void UseShieldRecharge();
    void Orbit(SystemEntity* who);
    void ForcedSetSpawner(SpawnMgr* spawnMgr)           { m_spawnMgr = spawnMgr; }

    float GetThermal()                                  { return m_therDamage; }
    float GetEM()                                       { return m_emDamage; }
    float GetKinetic()                                  { return m_kinDamage; }
    float GetExplosive()                                { return m_expDamage; }

    float GetOrbitRange()                               { return m_orbitRange; }

    NPCAIMgr* GetAIMgr()                                { return m_AI; }

protected:
    NPCAIMgr* m_AI;
    SpawnMgr* m_spawnMgr;

private:
    uint32 m_orbitingID = 0;

    float m_orbitRange = 0;
    float m_emDamage = 0;
    float m_expDamage = 0;
    float m_kinDamage = 0;
    float m_therDamage = 0;
    float m_hullDamage = 0;
    float m_armorDamage = 0;
    float m_shieldCharge = 0;
    float m_shieldCapacity = 0;
};

#endif
