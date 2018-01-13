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
class Item;
class DestinyManager;
class SystemManager;
class ServiceDB;
class NPCAIMgr;

class Drone
: public DynamicSystemEntity
{
public:
    Drone(InventoryItemRef drone, PyServiceMgr& services, SystemManager* pSystem, const GPoint& position, const FactionData& data);
    virtual ~Drone();

    /* class type pointer querys. */
    virtual Drone* GetDroneSE()                         { return this; }
    /* class type tests. */
    virtual bool IsDroneSE()                            { return true; }

    /* SystemEntity interface */
    virtual void Process();
    virtual void EncodeDestiny( Buffer& into );
    virtual void MakeDamageState(DoDestinyDamageState &into);
    virtual PyDict* MakeSlimItem();

    virtual void TargetAdded(SystemEntity *who);
    virtual void TargetLost(SystemEntity *who);
    virtual void TargetedAdd(SystemEntity *who);
    virtual void TargetedLost(SystemEntity *who);

    /* specific functions handled here. */
    Client* GetOwner()                                  { return ((m_pClient == nullptr) ? m_pClient : nullptr); }
    DroneAIMgr* GetAI()                                 { return m_AI; }

    void SaveDrone();
    void RemoveDrone();
    void SetResists();
    void UseArmorRepairer();
    void UseShieldRecharge();
    void Orbit(SystemEntity *who);
    void SetOwner(Client* pClient);

    uint32 GetBounty() const                            { return ((m_pClient == nullptr) ? m_pClient->GetChar()->bounty() : 0); }
    uint32 GetOwnerID() const                           { return ((m_pClient == nullptr) ? m_pClient->GetCharacterID() : 1); }

    float GetThermal()                                  { return m_therDamage; }
    float GetEM()                                       { return m_emDamage; }
    float GetKinetic()                                  { return m_kinDamage; }
    float GetExplosive()                                { return m_expDamage; }
    float GetSecurityRating() const                     { return ((m_pClient == nullptr) ? m_pClient->GetChar()->GetSecurityRating() : 1.0); }

    double GetOrbitRange()                              { return m_orbitRange; }

protected:
    Client* m_pClient;
    DroneAIMgr* m_AI;

private:
    uint32 m_orbitingID = 0;

    double m_orbitRange = 0;
    double m_emDamage = 0;
    double m_expDamage = 0;
    double m_kinDamage = 0;
    double m_therDamage = 0;
    double m_hullDamage = 0;
    double m_armorDamage = 0;
    double m_shieldCharge = 0;
    double m_shieldCapacity = 0;
};

#endif /* !__DRONE__H__INCL__ */


