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

#pragma once

#include "system/SpaceEntity.h"

class PyServiceMgr;
class InventoryItem;
class SystemManager;
class ShipItem;
class GenericModule;

class Missile
: public DynamicSpaceEntity {
public:
    // default c'tor
    Missile() =delete;
    // main c'tor
    Missile(InventoryItemRef self, PyServiceMgr& services, SystemManager* pSystem, InventoryItemRef modRef, SpaceEntity* tSE, SpaceEntity* pSE, GenericModule* pMod=nullptr);
    // copy c'tor
    Missile(const Missile& oth) =delete;
    // move c'tor
    Missile(Missile&& oth) noexcept =default;
    // copy assignment
    Missile& operator=(const Missile& oth) =delete;
    // move assignment
    Missile& operator=(Missile&& oth) =default;
    // d'tor
    virtual ~Missile()                                  { /* do nothing here */ }

    /* class type pointer querys. */
    virtual const char*         GetSEType()             { return "Missle SE"; }
    virtual Missile*            GetMissileSE()          { return this; }
    /* Dynamic */
    virtual bool                IsMissileSE()           { return true; }

    /* SpaceEntity interface */
    virtual void                Delete();
    virtual void                Process();
    virtual void                EncodeStateData(Buffer& into);
    virtual void                MakeDamageState(DoDestinyDamageState &into);
    virtual PyDict*             MakeSlimItem();

    /* specific functions handled here. */
    SpaceEntity*                GetLauncherSE()         { return m_fromSE; }
    uint32                      GetLauncherID()         { return m_fromSE->GetID(); }

    void                        SetHitTimer(uint32 setTime) { m_hitTimer.Start(setTime); }

    bool                        IsAlive()               { return m_alive; }
    bool                        IsOverloaded()          { return false; }


protected:
    void                        HitTarget();
    void                        EndOfLife();
    PyRep*                      PyGetNameID();

    SpaceEntity*                m_fromSE;
    InventoryItemRef            m_modRef;

    uint16                      m_hullHP;
    float                       m_damageMod;

    Timer                       m_hitTimer;
    Timer                       m_lifeTimer;

    bool                        m_alive;

};
