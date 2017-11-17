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


#ifndef __TARGETMANAGER_H_INCL__
#define __TARGETMANAGER_H_INCL__

#include "inventory/ItemRef.h"

class SystemEntity;
class PyRep;
class PyTuple;

class TargetManager {
public:
    TargetManager(SystemEntity* self);
    virtual ~TargetManager()                            { /* do nothing here */ }

    void                Process();

    /* Common Methods for all objects */
    void                TargetsCleared();
    void                ClearFromTargets();
    void                TargetTry(SystemEntity *who);
    void                TargetLost(SystemEntity *who);
    void                ClearTarget(SystemEntity *who);
    void                TargetAdded(SystemEntity *who);
    void                TargetedAdd(SystemEntity *who);
    void                TargetedLost(SystemEntity *who);
    void                ClearTargets(bool notify_self=true);
    void                ClearAllTargets(bool notify_self=true);

    bool                TargetFail(SystemEntity* who);
    bool                StartTargeting(SystemEntity *who, ShipItemRef ship);

    bool                IsTargetedBySomething() const   { return (!m_targetedBy.empty()); }

    uint8               GetTotalTargets() const         { return (uint8)m_targets.size(); }

    float               TimeToLock(ShipItemRef ship, SystemEntity *target) const;

    /* NPC AI Methods */
    bool                IsTargetedBy(SystemEntity *pSE);
    SystemEntity*       GetFirstTarget(bool need_locked=false);
    SystemEntity*       GetTarget(uint32 targetID, bool need_locked=true) const;

    bool                StartTargeting(SystemEntity* who, float lockTime, uint8 maxLockedTargets, double maxTargetLockRange, bool& chase);

    bool                CanAttack()                     { return m_canAttack; }
    bool                HasNoTargets() const            { return m_targets.empty(); }

    /* debugging methods */
    void                Dump() const;
    std::string         TargetList(uint16 &length, uint16 &count);

    /* Packet builders: */
    PyList*             GetTargets() const;
    PyList*             GetTargeters() const;

    /* currently unused methods */
    void                QueueTBDestinyEvent(PyTuple **up) const;    //queue a destiny event to all people targeting me.
    void                QueueTBDestinyUpdate(PyTuple **up) const;    //queue a destiny update to all people targeting me.

protected:

    //called in reaction to outgoing targeting events in other target managers.
    //void TargetedByLocking(SystemEntity *from_who);
    void                TargetedByLocked(SystemEntity *from_who);
    void                TargetedByLost(SystemEntity *from_who);


    class TargetEntry {
    public:
        TargetEntry(SystemEntity *_who)
            : state(Idle), who(_who), timer(0) {}

        void Dump() const;

        enum {
            Idle,
            PassiveLocking,
            Locking,
            Locked
        } state;
        SystemEntity *const who;
        Timer timer;
    };

    class TargetedByEntry {
    public:
        TargetedByEntry(SystemEntity *_who)
            : state(Idle), who(_who) {}

        void Dump() const;

        enum {
            Idle,
            Locking,
            Locked
        } state;
        SystemEntity *const who;
    };

private:
    SystemEntity* mySE;    //we do not own this.

    bool m_canAttack;   // true if npcs can begin attack (to correct attacking before targetlock)

    std::map<SystemEntity*, TargetEntry*> m_targets;    //we own these values, not the keys
    std::map<SystemEntity*, TargetedByEntry*> m_targetedBy;    //we own these values, not the keys
};

#endif


