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
    Author:        Aknor Jaden
    Updates:    Allan
*/

#ifndef __DRONE__H__INCL__
#define __DRONE__H__INCL__

#include "Client.h"
#include "system/SystemEntity.h"

/**
 * DynamicSystemEntity which represents drone object in space
 */
class DroneAIMgr;
class PyServiceMgr;
class InventoryItem;
class DestinyManager;
class SystemManager;
class ServiceDB;
class NPCAIMgr;

class Drone
: public DynamicSystemEntity
{
public:
    Drone(
        InventoryItemRef drone,
        SystemManager *system,
        PyServiceMgr &services,
        const GPoint &position);
    ~Drone();

    /*
     * Primary public interface:
     */
    InventoryItemRef GetDroneObject() { return _droneRef; }
    DestinyManager * GetDestiny() { return m_destiny; }
    SystemManager * GetSystem() { return m_system; }

    /*
     * Public fields:
     */

    inline double x() const { return(GetPosition().x); }
    inline double y() const { return(GetPosition().y); }
    inline double z() const { return(GetPosition().z); }

    //SystemEntity interface:
    virtual EntityClass GetClass() const { return(ecDroneEntity); }
    virtual bool IsDrone() const { return true; }
    virtual Drone *CastToDroneEntity() { return(this); }
    virtual const Drone *CastToDroneEntity() const { return(this); }
    virtual void Process();
    virtual void EncodeDestiny( Buffer& into ) const;
    virtual void TargetAdded(SystemEntity *who) { /* will need code once drones are implemented */ }
    virtual void TargetLost(SystemEntity *who);
    virtual void TargetedAdd(SystemEntity *who);
    virtual void TargetedLost(SystemEntity *who) { /* will need code once drones are implemented */ }
    virtual void TargetsCleared() {}
    virtual void QueueDestinyUpdate(PyTuple **du) {/* not required to consume */}
    virtual void QueueDestinyEvent(PyTuple **multiEvent) {/* not required to consume */}
    virtual void Killed(Damage &fatal_blow);
    virtual SystemManager* System() const { return m_system; }

    uint32 GetCorporationID() const     { return (m_owner ? m_owner->GetCorporationID() : 0); }
    uint32 GetAllianceID() const        { return (m_owner ? m_owner->GetAllianceID() : 0); }
    uint32 GetWarFactionID() const      { return (m_owner ? m_owner->GetWarFactionID() : 0); }
    uint32 GetBounty() const            { return 0; }
    uint32 GetOwnerID() const           { return (m_owner ? m_owner->GetCharacterID() : 1); }
    float GetSecurityRating() const     { return (m_owner ? m_owner->GetChar()->GetSecurityRating() : 1.0); }

    Client* GetOwner()                  { return (m_owner ? m_owner : nullptr); }
    void SetOwner(Client* pClient)      { m_owner = pClient; }

    void ForcedSetPosition(const GPoint &pt);

    virtual bool ApplyDamage(Damage &d);
    virtual void MakeDamageState(DoDestinyDamageState &into) const;
    virtual PyDict* MakeSlimItem() const;

    void SendNotification(const PyAddress &dest, EVENotificationStream &noti, bool seq=true);
    void SendNotification(const char *notifyType, const char *idType, PyTuple **payload, bool seq=true);

    void UseShieldRecharge();
    void UseArmorRepairer();

    float GetThermal() { return m_therDamage; }
    float GetEM() { return m_emDamage; }
    float GetKinetic() { return m_kinDamage; }
    float GetExplosive() { return m_expDamage; }

    void Orbit(SystemEntity *who);
    double GetOrbitRange();

    void SaveDrone();
    void RemoveDrone();

    DroneAIMgr* AI() const { return m_AI; }

protected:
    /*
     * Member fields:
     */
    Client* m_owner;    // we dont own this
    PyServiceMgr &m_services;    //we do not own this
    SystemManager* const m_system;    //we do not own this
    InventoryItemRef _droneRef;   // We don't own this
    DroneAIMgr* m_AI;

private:
    void _UpdateDamage();

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

#endif /* !__DRONE__H__INCL__ */


