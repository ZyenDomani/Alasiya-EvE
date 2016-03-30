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

#include "npc/SpawnMgr.h"
#include "system/SystemEntity.h"

class PyServiceMgr;
class DestinyManager;
class InventoryItem;
class NPCAIMgr;
class SystemManager;
class ServiceDB;
class SpawnMgr;

//Caution: do not inherit this, see constructor.
// TODO: This class should be inheriting from ShipEntity so as to contain a ShipRef for use with DestinyManager
class NPC
: public DynamicSystemEntity {
public:
    NPC(
        SystemManager* system,
        PyServiceMgr& services,
        InventoryItemRef self,
        uint32 corporationID,
        uint32 allianceID,
        const GPoint &position,
        SpawnMgr* spawnMgr = nullptr);
    virtual ~NPC();

    bool Load(ServiceDB& from);

    void Orbit(SystemEntity* who);

    inline double x() const { return(GetPosition().x); }
    inline double y() const { return(GetPosition().y); }
    inline double z() const { return(GetPosition().z); }

    //SystemEntity interface:
    EntityClass GetClass() const { return(ecNPC); }
    bool IsNPC() const { return true; }
    NPC* CastToNPC() { return(this); }
    const NPC* CastToNPC() const { return(this); }

    void Process();
    void EncodeDestiny( Buffer& into ) const;
    void QueueDestinyUpdate(PyTuple** du) {/* not required to consume */}
    void QueueDestinyEvent(PyTuple** multiEvent) {/* not required to consume */}

    void TargetAdded(SystemEntity* who) {}
    void TargetLost(SystemEntity* who);
    void TargetedAdd(SystemEntity* who);
    void TargetedLost(SystemEntity* who) {}
    void TargetsCleared() {}

    uint32 GetCorporationID() const { return(m_corporationID); }
    uint32 GetAllianceID() const { /* hack for now */ return(m_allianceID); }

    void ForcedSetSpawner(SpawnMgr* spawnMgr) { m_spawnMgr = spawnMgr; }
    void ForcedSetPosition(const GPoint& pt);

    bool ApplyDamage(Damage& d);
    void MakeDamageState(DoDestinyDamageState& into) const;
    void Killed(Damage& fatal_blow);

    void UseShieldRecharge();
    void UseArmorRepairer();

	void SaveNPC();
    void RemoveNPC();

    float GetThermal() { return m_therDamage; }
    float GetEM() { return m_emDamage; }
    float GetKinetic() { return m_kinDamage; }
    float GetExplosive() { return m_expDamage; }

    double GetOrbitRange();
    
    SystemManager* System() const { return(m_system); }
    NPCAIMgr* AI() const { return(m_AI); }

protected:
    void _AwardBounty(SystemEntity* who);
    void _DropLoot(uint32 groupID, uint32 owner, uint32 locationID);
    void _UpdateDamage();

    SystemManager* const m_system;    //we do not own this
    PyServiceMgr& m_services;    //we do not own this
    SpawnMgr* m_spawnMgr;    //we do not own this, may be NULL
    uint32 m_corporationID;
    uint32 m_allianceID;

    uint32 m_orbitingID;

    NPCAIMgr* m_AI;    //never NULL

    double m_hullDamage;
    double m_armorDamage;
    double m_shieldCharge;
    double m_shieldCapacity;

    double m_emDamage;
    double m_expDamage;
    double m_kinDamage;
    double m_therDamage;
};

#endif
