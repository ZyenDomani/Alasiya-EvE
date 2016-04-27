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

/** @todo (Allan)  update this class */

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "Profile.h"
#include "Client.h"
#include "inventory/AttributeEnum.h"
#include "ship/Ship.h"
#include "ship/TargetManager.h"
#include "system/SystemEntity.h"

TargetManager::TargetManager(SystemEntity *self)
: m_destroyed(false),
  m_self(self)
{
    m_canAttack = false;
    _log(TARGET__INFO, "Created TargMgr %p for %s(%u)", this, self->GetID(), self->GetName());
}

TargetManager::~TargetManager() {
    //DO NOT call DoDestruction here! it calls virtuals!
}

//I am not happy with this:
//this function exists to deal with a specific problem with the
// destruction chain where we reference a SystemEntity (m_self), which
// also contains their TargetManager. The TargetManager object is
// not destroyed until the base SystemEntity is destroyed, but
// the SystemEntity pointer itself becomes invalid as soon as the
// first child class in its hierarchy (such as Client or NPC) are
// destroyed. Thus, all terminal children of SystemEntity must call
// this from their destructor.
void TargetManager::DoDestruction() {
    if (!m_destroyed && m_self) {
        ClearTargets(false);
    }
}

void TargetManager::SetSelf(SystemEntity* self) {
    m_self = self;
    _log(TARGET__INFO, "TargMgr set self to %s(%u)", self->GetID(), self->GetName());
}

void TargetManager::Process() {
//     double profileStartTime = 0.0;
//     if (sConfig.server.UseProfiling)
//         profileStartTime = GetTimeUSeconds();
    //process outgoing targeting
    if (!GetTotalTargets()) return;

    std::map<SystemEntity*, TargetEntry*>::iterator cur = m_targets.begin();
    while (cur != m_targets.end()) {
        if (m_targets.empty() || (!cur->first)) return;
        switch (cur->second->state) {
            case TargetEntry::Idle:
            case TargetEntry::Locked:{
                //nothing to do right now...
            } break;
            case TargetEntry::PassiveLocking:   // this will be used with stealth modules (which, ofc, are not written yet)
            case TargetEntry::Locking: {
                //see if we are finished locking...
                if (cur->second->timer.Check(false)) {
                    cur->second->timer.Disable();
                    //yay, they are locked..
                    cur->second->state = TargetEntry::Locked;
                    _log(TARGET__TRACE, "%s(%u) has finished locking %s(%u)", \
                            m_self->GetName(), m_self->GetID(), cur->first->GetName(), cur->first->GetID());
                    m_self->TargetAdded(cur->first);
                    cur->first->TargMgr.TargetedByLocked(m_self);
                    m_canAttack = true;
                }
            } break;
        }
        ++cur;
    }
    //now incoming...?
    //nothing to do right now...
    /*{
        std::map<SystemEntity *, TargetedByEntry *>::iterator cur, end;
        cur = m_targetedBy.begin();
        end = m_targetedBy.end();
        for(; cur != end; cur++) {
            cur->first->TargMgr.TargetLost(m_self);
        }
}*/
    //if (sConfig.server.UseProfiling)
    //    sProfile.AddTime(targets, GetTimeUSeconds() - profileStartTime);
}

void TargetManager::ClearTarget(SystemEntity *who) {
    //let the other entity know they are no longer targeted.
    who->TargMgr.TargetedByLost(m_self);
    //clear it from our own state
    TargetLost(who);
    if (m_self->IsNPC() && HasNoTargets())
        m_canAttack = false;
    _log(TARGET__TRACE, "ClearTarget:  %s(%u) has cleared target information for %s(%u).",
         m_self->GetName(), m_self->GetID(), who->GetName(), who->GetID());
}

void TargetManager::ClearAllTargets(bool notify_self) {
    //if ((!m_self->IsNPC()) || (!m_self->IsClient())) return;
    ClearTargets(notify_self);
    ClearFromTargets();
    _log(TARGET__TRACE, "ClearAllTargets:  %s(%u) has cleared all targeting information.", m_self->GetName(), m_self->GetID());
}

void TargetManager::ClearTargets(bool notify_self) {
    if (HasNoTargets()) {
        m_canAttack = false;
        return;
    }
    _log(TARGET__TRACE, "ClearTargets:  %s(%u) is clearing all targeting information.", m_self->GetName(), m_self->GetID());
    std::map<SystemEntity*, TargetEntry*>::iterator cur = m_targets.begin();
    for(; cur != m_targets.end(); cur++) {
        _log(TARGET__TRACE, "%s(%u) has cleared target %s(%u) during clear all.",
                m_self->GetName(), m_self->GetID(), cur->first->GetName(), cur->first->GetID());
        cur->first->TargMgr.TargetedByLost(m_self);
        SafeDelete(cur->second);
    }
    m_targets.clear();

    if (notify_self)
        m_self->TargetsCleared();

    m_canAttack = false;
}

void TargetManager::ClearFromTargets() {
    if (!IsTargetedBySomething()) return;
    std::vector<SystemEntity *> ToNotify;

    //first, clean up our internal structure.
    std::map<SystemEntity*, TargetedByEntry*>::iterator cur = m_targetedBy.begin();
    for (; cur != m_targetedBy.end(); cur++) {
        //do not notify until we clear our target list! otherwise bad things happen.
        ToNotify.push_back(cur->first);
        _log(TARGET__TRACE, "ClearFromTargets:  Added %s(%u) to delete list for %s(%u).",
             cur->first->GetName(), cur->first->GetID(), m_self->GetName(), m_self->GetID());
        SafeDelete(cur->second);
    }
    m_targetedBy.clear();

    std::vector<SystemEntity *>::iterator curn = ToNotify.begin();
    for (; curn != ToNotify.end(); curn++)
        (*curn)->TargMgr.TargetLost(m_self);
}

bool TargetManager::StartTargeting(SystemEntity *who, ShipRef ship)
{       // NOTE this is for clients
    if (!m_self->IsClient()) {
        codelog(TARGET__ERROR, "StartTargeting() called by %s to target %s", m_self->GetName(), who->GetName());
        return false;
    }

    //first make sure they are not already in the list
    std::map<SystemEntity *, TargetEntry *>::iterator res = m_targets.find(who);
    if (res != m_targets.end()) {
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but we are already targeting them. Ignoring request.", \
             m_self->GetName(), m_self->GetID(), who->GetName(), who->GetID());
        return true;
    }
    //Check that they aren't targeting themselves (which may not be possible)
    if (who == m_self)
        return false;
    // Check invulnerability (undock and jump invul states)
    if (who->IsInvul()) {
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but they are Invul.  Ignoring request.", \
             m_self->GetName(), m_self->GetID(), who->GetName(), who->GetID());
        return false;
    }
    // Check login for client just logging into game.
    if (who->IsLogin()) {
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but they are just Logging In.  Ignoring request.", \
             m_self->GetName(), m_self->GetID(), who->GetName(), who->GetID());
        return false;
    }

    uint8 targetSkills = 1; //AttrMaxLockedTargets is for characters too!!
    float targetRangeModifier = 1.0f;
    Character* pChar = m_self->CastToClient()->GetChar().get();
    targetSkills += pChar->GetSkillLevel(skillTargeting);    // +1 target/level
    targetSkills += pChar->GetSkillLevel(skillMultitasking);    // +1 target/level
    targetRangeModifier += (0.05 * pChar->GetSkillLevel(skillLongRangeTargeting)); // +5% level

	uint8 maxLockedTargets = (uint8)ship->GetAttribute(AttrMaxLockedTargets).get_int();
    if (maxLockedTargets < 1) maxLockedTargets = 1;
    // add module updates to target capacity of ship here.
    if (targetSkills < maxLockedTargets)
        maxLockedTargets = targetSkills;
    if (GetTotalTargets() >= maxLockedTargets) {
        m_self->CastToClient()->SendInfoModalMsg("Your ship and skills combination can only handle %u target at a time.", maxLockedTargets);
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but we already have max targets.  Ignoring request.", \
             m_self->GetName(), m_self->GetID(), who->GetName(), who->GetID());
        return false;
    }
    // Check against max locked target range
	double maxTargetLockRange = ship->GetAttribute(AttrMaxTargetRange).get_float();
    maxTargetLockRange *= targetRangeModifier;
    GVector rangeToTarget( m_self->GetPosition(), who->GetPosition() );
    if (rangeToTarget.length() > maxTargetLockRange) {
        m_self->CastToClient()->SendInfoModalMsg("Your ship and skills combination can only target to %f meters.  %s is %f meters away.", \
            maxTargetLockRange, who->GetName(), rangeToTarget.length());
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but they are too far away.  Ignoring request.", \
             m_self->GetName(), m_self->GetID(), who->GetName(), who->GetID());
        return false;
    }

    // Calculate Time to Lock target:
    float lockTime = TimeToLock( ship, who );

    TargetEntry *te = new TargetEntry(who);
    te->state = TargetEntry::Locking;
    te->timer.Start(lockTime *1000);      //timer has ms resolution
	m_targets[who] = te;

    _log(TARGET__TRACE, "Target 1: %s(%u) started targeting %s(%u) (%.2fs lock time)",
         m_self->GetName(), m_self->GetID(), who->GetName(), who->GetID(), lockTime);
    who->TargetedAdd(m_self);

    if (sConfig.server.testServer)
        Dump();

    return true;
}

bool TargetManager::StartTargeting(SystemEntity *who, float lockTime, uint32 maxLockedTargets, double maxTargetLockRange)
{       // NOTE  this is for npcs
    //first make sure they are not already in the list
    std::map<SystemEntity *, TargetEntry *>::iterator res = m_targets.find(who);
    if (res != m_targets.end()) {
        //what to do?
        _log(TARGET__TRACE, " %s(%u): Told to target %s(%u), but we are already targeting them. Ignoring request.",
             m_self->GetName(), m_self->GetID(), who->GetName(), who->GetID());
        return true;
    }
    //Check that they aren't targeting themselves (which may not be possible)
    if (who == m_self)
        return false;
    // Check against max locked target count
    if (GetTotalTargets() >= maxLockedTargets){
        _log(TARGET__TRACE, " %s(%u): Told to target %s(%u), but we already have max targets.  Ignoring request.",
             m_self->GetName(), m_self->GetID(), who->GetName(), who->GetID());
        return false;
    }
    // Check against max locked target range
    if (m_self->GetPosition().distance(who->GetPosition()) > maxTargetLockRange){
        _log(TARGET__TRACE, " %s(%u): Told to target %s(%u), but they are too far away.  Begin Approaching.",
             m_self->GetName(), m_self->GetID(), who->GetName(), who->GetID());
        return false;
    }
    // Check invulnerability (undock and jump invul states)
    if (who->IsInvul()) {
        _log(TARGET__TRACE, " %s(%u): Told to target %s(%u), but they are Invul.  Ignoring request.",
             m_self->GetName(), m_self->GetID(), who->GetName(), who->GetID());
        return false;
    }
    // Check login for client just logging into game.
    if (who->IsLogin()) {
        _log(TARGET__TRACE, " %s(%u): Told to target %s(%u), but they are just Logging In.  Ignoring request.",
             m_self->GetName(), m_self->GetID(), who->GetName(), who->GetID());
        return false;
    }

    TargetEntry *te = new TargetEntry(who);
    te->state = TargetEntry::Locking;
	te->timer.Start(lockTime);
	m_targets[who] = te;

    _log(TARGET__TRACE, "Target 2: %s(%u) started targeting %s(%u) (%.2fs lock time)",
         m_self->GetName(), m_self->GetID(), who->GetName(), who->GetID(), (lockTime /1000));
    who->TargetedAdd(m_self);
    return true;
}

void TargetManager::TargetLost(SystemEntity *who) {
    std::map<SystemEntity *, TargetEntry *>::iterator res = m_targets.find(who);
    if (res == m_targets.end())
        return;

    //clear our internal state for this target (BEFORE the callback!)
    SafeDelete(res->second);
    m_targets.erase(res);

    _log(TARGET__TRACE, "%s(%u) has lost target %s(%u)",
         m_self->GetName(), m_self->GetID(), who->GetName(), who->GetID());

    m_self->TargetLost(who);
}

void TargetManager::TargetedByLocked(SystemEntity *from_who) {
    //first make sure they are not already in the list
    std::map<SystemEntity *, TargetedByEntry *>::iterator res = m_targetedBy.find(from_who);
    if (res != m_targetedBy.end()) {
        //just re-use the old entry...
        res->second->state = TargetedByEntry::Locked;
        return;
    } else {
        //new entry.
        TargetedByEntry *te = new TargetedByEntry(from_who);
        te->state = TargetedByEntry::Locked;
        m_targetedBy[from_who] = te;
    }
    _log(TARGET__TRACE, "%s(%u) has been locked by %s(%u)",
         m_self->GetName(), m_self->GetID(), from_who->GetName(), from_who->GetID());
    m_self->TargetedAdd(from_who);
}

void TargetManager::TargetedByLost(SystemEntity *from_who) {
    //first make sure they are not already in the list
    std::map<SystemEntity *, TargetedByEntry *>::iterator res = m_targetedBy.find(from_who);
    if (res != m_targetedBy.end()) {
        SafeDelete(res->second);
        m_targetedBy.erase(res);
        m_self->TargetedLost(from_who);
        _log(TARGET__TRACE, "%s(%u) is no longer locked by %s(%u)",
             m_self->GetName(), m_self->GetID(), from_who->GetName(), from_who->GetID());
    } else {
        //not found.. do nothing to our state, no notification?
        _log(TARGET__TRACE, "%s(%u) was notified of targeted lost by %s(%u), but they did not have us targeted in the first place.",
             m_self->GetName(), m_self->GetID(), from_who->GetName(), from_who->GetID());
    }
}

SystemEntity* TargetManager::GetFirstTarget(bool need_locked) {
    if (m_targets.empty())
        return nullptr;

    if (!need_locked) {
        //we know there is at least one entry here...
        return (m_targets.begin()->first);
    }

    std::map<SystemEntity *, TargetEntry *>::const_iterator cur = m_targets.begin();
    for (; cur != m_targets.end(); cur++)
        if (cur->second->state == TargetEntry::Locked)
            return(cur->first);

    return nullptr;
}

PyList* TargetManager::GetTargets() const {
    PyList* result = new PyList();
    if (m_targets.empty())
        return result;

    std::map<SystemEntity *, TargetEntry *>::const_iterator cur = m_targets.begin();
    for (; cur != m_targets.end(); cur++)
        result->AddItemInt( cur->first->GetID() );

    return result;
}

SystemEntity* TargetManager::GetTarget(uint32 targetID, bool need_locked) const {
    if (m_targets.empty())
        return nullptr;

    std::map<SystemEntity*, TargetEntry*>::const_iterator cur = m_targets.begin();
    for (; cur != m_targets.end(); cur++) {
        if (cur->first->GetID() != targetID)
            continue;
        //found it...
        if (need_locked && cur->second->state != TargetEntry::Locked) {
            _log(TARGET__TRACE, "Found target %u, but it is not locked.", targetID);
            continue;
        }
        _log(TARGET__TRACE, "Found target %u: %s (nl? %s)", targetID, cur->first->GetName(), need_locked?"yes":"no");
        return(cur->first);
    }
    _log(TARGET__TRACE, "Unable to find target %u (nl? %s)", targetID, need_locked?"yes":"no");
    return nullptr;    //not found.
}

PyList* TargetManager::GetTargeters() const {
    PyList* result = new PyList();
    if (m_targetedBy.empty())
        return result;

    std::map<SystemEntity*, TargetedByEntry*>::const_iterator cur = m_targetedBy.begin();
    for(; cur != m_targetedBy.end(); cur++)
        result->AddItemInt( cur->first->GetID() );

    return result;
}

float TargetManager::TimeToLock(ShipRef ship, SystemEntity *target) const {
    if ( (target->IsAsteroid()) || (target->IsDeployable()) || (target->IsWreck())
        || (target->IsContainer()) || (target->IsInanimate()) || (target->IsStaticEntity()) )
        return 2.0;

    //  fixed lock time  -allan 24Dec14  -updated 26May15
    //TODO add ship bonuses in here
    uint32 scanRes = ship->GetAttribute(AttrScanResolution).get_int();
    uint32 sigRad = 25; // set base as capsule with 25m signature radius

	if ( target->Item().get() )
		if ( target->Item()->HasAttribute(AttrSignatureRadius) )
			sigRad = target->Item()->GetAttribute(AttrSignatureRadius).get_int();

    /*
     * fleet invlovement enhances targeting speed using leadership of highest member (2%/lvl)
     */

    //https://wiki.eveonline.com/en/wiki/Targeting_speed
    //locktime = 40000/(scanres * asinh(sigrad)^2)
    float time = ( 40000 /(scanRes * pow(asinh(sigRad), 2)));

    if (m_self->IsClient()) {
        Character* pChar = m_self->CastToClient()->GetChar().get();
        time *= (1 - (0.05 * pChar->GetSkillLevel(skillSignatureAnalysis))); // 5% decrease/level
        if (pChar->fleetID()) { //FIXME always returns 0 for now
            //Character* pLeader = pChar->GetFleetLeader;   //TODO this needs to be written
            time *= (1 - (0.02 * pChar->GetSkillLevel(skillLeadership))); // 2% decrease/level
        }
    }

    /*  distance-based modifier to targeting speed?         sure, why the hell not?   -allan 27.6.15
     *  +0.1s for each 10k distance
     *     distance = pos - targ.pos
     *     disMod = distance /10k (for 10k increments)
     *     time += disMod * 0.1
     */
    double distance = ship->position().distance(target->GetPosition());
    // check for snipers... >85k distance do NOT need additional 7.5+s to targettime
    if (m_self->IsNPC())
        if (distance > 85000)
            distance -= 75000;

    float disMod = distance /10000;
    if (disMod < 0) disMod = 0;
    time += (disMod *0.1);

	return time;
}

/*
 * NOTE  the functions below are no longer used.
 *  i may find a use for them again, so keep them here.
 *
 */

void TargetManager::QueueTBDestinyEvent( PyTuple** up_in ) const
{
    PyTuple* up = *up_in;
    *up_in = nullptr;    //could optimize out one of the Clones in here...

    PyTuple* up_dup(nullptr);

    for (auto cur : m_targetedBy) {
        if (cur.first->IsClient()) {
            if (!up_dup)
                up_dup = new PyTuple( *up );

            cur.first->QueueDestinyEvent( &up_dup );
        }
    }

    PySafeDecRef( up_dup );
    PyDecRef( up );
}

void TargetManager::QueueTBDestinyUpdate( PyTuple** up_in ) const
{
    PyTuple* up = *up_in;
    *up_in = nullptr;    //could optimize out one of the Clones in here...

    PyTuple* up_dup(nullptr);

    for (auto cur : m_targetedBy) {
        if (cur.first->IsClient()) {
            if (!up_dup)
                up_dup = new PyTuple( *up );

            cur.first->QueueDestinyUpdate( &up_dup );
        }
    }

    PySafeDecRef( up_dup );
    PyDecRef( up );
}

void TargetManager::TargetEntry::Dump() const {
    const char *sname = "Unknown State";
    switch(state) {
        case Idle:
            sname = "Idle Entry";
            break;
        case PassiveLocking:
            sname = "Passive Locking";
            break;
        case Locking:
            sname = "Locking";
            break;
        case Locked:
            sname = "Locked";
            break;
            //no default on purpose.
    }
    _log(TARGET__DUMP, "    Targeted %s(%u): %s (Timer %s at %dms remaining)",
         who->GetName(),
         who->GetID(),
         sname,
         timer.Enabled() ? "Running" : "Disabled",
         timer.GetRemainingTime()
    );
}

void TargetManager::TargetedByEntry::Dump() const {
    const char *sname = "Unknown State";
    switch(state) {
        case Idle:
            sname = "Idle Entry";
            break;
        case Locking:
            sname = "Locking";
            break;
        case Locked:
            sname = "Locked";
            break;
            //no default on purpose.
    }
    _log(TARGET__DUMP, "    Targeted By %s (%u): %s",
         who->GetName(),
         who->GetID(),
         sname
    );
}

void TargetManager::Dump() const {
    _log(TARGET__DUMP, "Target Dump for %u:", m_self->GetID());
    for (auto cur : m_targets)
        cur.second->Dump();
    for (auto cur : m_targetedBy)
        cur.second->Dump();
}
