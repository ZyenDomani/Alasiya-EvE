/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2008 The EVEmu Team
    For the latest information visit http://evemu.mmoforge.org
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
    Author:     Allan
*/

#ifndef EVE_SHIP_MISSILE_H
#define EVE_SHIP_MISSILE_H

#include "system/SystemEntity.h"

class PyServiceMgr;
class InventoryItem;
class DestinyManager;
class SystemManager;
class Ship;

class Missile
: public DynamicSystemEntity {
public:
    Missile( SystemManager* system,
             PyServiceMgr &services,
             InventoryItemRef self,
             InventoryItemRef module,
             SystemEntity* target,
             Ship* ship,
             const GPoint &position );

    virtual ~Missile();

    bool Load(ServiceDB &from);

    inline double x() const { return(GetPosition().x); }
    inline double y() const { return(GetPosition().y); }
    inline double z() const { return(GetPosition().z); }

    //SystemEntity interface:
    bool IsMissile() const { return true; }
    bool IsAlive() { return m_alive; }
    EntityClass GetClass() const { return(ecMissileEntity); }
    Missile* CastToMissile() { return(this); }
    const Missile* CastToMissile() const { return(this); }
    SystemEntity* GetTarget() { return m_target; }
    SystemManager* System() const { return m_system; }
    Ship* GetShip() { return m_ship; }

    bool IsOverloaded() { return false; }

    void Process();
    void EncodeDestiny( Buffer& into ) const;
    void QueueDestinyUpdate(PyTuple **du) {/* not required to consume */}
    void QueueDestinyEvent(PyTuple **multiEvent) {/* not required to consume */}

    void TargetAdded(SystemEntity *who) { /* not used for missiles */ }
    void TargetLost(SystemEntity *who)  { /* not used for missiles */ }
    void TargetedAdd(SystemEntity *who)  { /* not used for missiles */ }
    void TargetedLost(SystemEntity *who)  { /* not used for missiles */ }
    void TargetsCleared()  { /* not used for missiles */ }

    uint32 GetCorporationID() const { return m_corporationID; }
    uint32 GetAllianceID() const { return m_allianceID; }
    uint32 GetOwnerID() const { return m_ownerID; }
    uint32 GetWarFactionID() const { return m_warFactionID; }

    void ForcedSetPosition(const GPoint &pt);

   // bool ApplyDamage(Damage &d);
    void MakeDamageState(DoDestinyDamageState &into) const {}
    PyDict *MakeSlimItem() const;

    float GetThermal() { return m_therDamage; }
    float GetEM() { return m_emDamage; }
    float GetKinetic() { return m_kinDamage; }
    float GetExplosive() { return m_expDamage; }
    float GetRadius() { return 5.0f; }

    void Delete();

    void SetHitTimer(uint32 setTime) { m_hitTimer.Start(setTime); }

    void SetSpeed(double speed) { m_speed = speed; }
    double GetSpeed() { return m_speed; }

    Timer m_hitTimer;
    Timer m_lifeTimer;

    double Min(double a, double b);

protected:
    void _HitTarget();
    void _EndOfLife();

    SystemManager* const m_system;    //we do not own this
    PyServiceMgr& m_services;    //we do not own this
    SystemEntity* m_target;
    InventoryItemRef m_self;
    InventoryItemRef m_module;
    Ship* m_ship;

    bool m_alive;
    double m_speed;

    uint32 m_corporationID;
    uint32 m_allianceID;
    uint32 m_ownerID;
    uint32 m_warFactionID;

    uint32 m_orbitingID;

    double m_hullHP;
    double m_emDamage;
    double m_therDamage;
    double m_kinDamage;
    double m_expDamage;
};

#endif  //EVE_SHIP_MISSILE_H


