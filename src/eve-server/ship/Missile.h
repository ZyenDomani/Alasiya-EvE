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
class ShipItem;

class Missile
: public DynamicSystemEntity {
public:
    Missile(InventoryItemRef self, PyServiceMgr &services, SystemManager* system, InventoryItemRef module, SystemEntity* target, ShipItem* ship);
    virtual ~Missile();

    /* class type pointer querys. */
    virtual Missile* GetMissileSE()                     { return this; }
    /* Dynamic */
    virtual bool IsMissileSE()                          { return true; }

    /* SystemEntity interface */
    virtual void Delete();
    virtual void Process();
    virtual void EncodeDestiny( Buffer& into );
    virtual void MakeDamageState(DoDestinyDamageState &into);
    virtual PyDict *MakeSlimItem();

    /* specific functions handled here. */
    ShipItem* GetShip()                                 { return m_ship; }
    SystemEntity* GetTarget()                           { return m_targetSE; }

    void SetHitTimer(uint32 setTime)                    { m_hitTimer.Start(setTime); }
    void SetSpeed(double speed)                         { m_speed = speed; }

    bool IsAlive()                                      { return m_alive; }
    bool IsOverloaded()                                 { return false; }

    //uint32 GetOwnerID()                                 { return m_ownerID; }
    //uint32 GetWarFactionID()                            { return m_warID; }

    float GetThermal()                                  { return m_therDamage; }
    float GetEM()                                       { return m_emDamage; }
    float GetKinetic()                                  { return m_kinDamage; }
    float GetExplosive()                                { return m_expDamage; }

    double GetSpeed()                                   { return m_speed; }

protected:
    SystemEntity* m_targetSE;
    InventoryItemRef m_module;
    ShipItem* m_ship;

    void HitTarget();
    void EndOfLife();

    Timer m_hitTimer;
    Timer m_lifeTimer;

    bool m_alive;

    //uint32 m_ownerID;
    //uint32 m_warFactionID;
    uint32 m_orbitingID;

    double m_speed;
    double m_hullHP;
    double m_emDamage;
    double m_therDamage;
    double m_kinDamage;
    double m_expDamage;
};

#endif  //EVE_SHIP_MISSILE_H


