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
    Updates:    Allan
*/
/** @todo (Allan)  add target lost and target fail reasons.
 * maybe make common function, and pass "add", "clear", "otheradd", reason, etc ??
 */

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "Profile.h"
#include "Client.h"
#include "inventory/AttributeEnum.h"
#include "ship/Ship.h"
#include "system/TargetManager.h"
#include "system/SystemEntity.h"
#include <system/SystemBubble.h>
#include <npc/NPC.h>
#include <npc/NPCAI.h>

TargetManager::TargetManager(SystemEntity *self)
: m_destroyed(false),
  mySE(self)
{
    m_canAttack = false;
    _log(TARGET__INFO, "Created TargMgr %p for %s(%u)", this, self->GetName(), self->GetID());
}

TargetManager::~TargetManager() {
    //DO NOT call DoDestruction here! it calls virtuals!
}

//I am not happy with this:
//this function exists to deal with a specific problem with the
// destruction chain where we reference a SystemEntity (mySE), which
// also contains their TargetManager. The TargetManager object is
// not destroyed until the base SystemEntity is destroyed, but
// the SystemEntity pointer itself becomes invalid as soon as the
// first child class in its hierarchy (such as Client or NPC) are
// destroyed. Thus, all terminal children of SystemEntity must call
// this from their destructor.
void TargetManager::DoDestruction() {
    if (!m_destroyed && mySE) {
        ClearAllTargets();
    }
}

void TargetManager::Process() {
     double profileStartTime = 0.0;
     if (sConfig.server.UseProfiling)
         profileStartTime = GetTimeUSeconds();

    //process outgoing targeting
    std::map<SystemEntity*, TargetEntry*>::iterator cur = m_targets.begin();
    while (cur != m_targets.end()) {
        if (m_targets.empty() || (!cur->first)) return;
        switch (cur->second->state) {
            case TargetEntry::Idle:
            case TargetEntry::Locked:{          //do nothing
                } break;
            case TargetEntry::PassiveLocking:   // this will be used with stealth modules (which, ofc, are not written yet)
            case TargetEntry::Locking: {
                    if (cur->second->timer.Check(false)) {
                        cur->second->timer.Disable();
                        cur->second->state = TargetEntry::Locked;
                        _log(TARGET__TRACE, "%s(%u) has finished locking %s(%u)", \
                                    mySE->GetName(), mySE->GetID(), cur->first->GetName(), cur->first->GetID());
                        TargetAdded(cur->first);
                        cur->first->TargetMgr()->TargetedByLocked(mySE);
                        m_canAttack = true;
                    }
                } break;
        }
        ++cur;
    }

    //nothing else to do right now...check target distances maybe?

    if (sConfig.server.UseProfiling)
        sProfile.AddTime(_targetsProfile, GetTimeUSeconds() - profileStartTime);
}

void TargetManager::ClearTarget(SystemEntity *who) {
    //let the other entity know they are no longer targeted.
    who->TargetMgr()->TargetedByLost(mySE);
    //clear it from our own state
    TargetLost(who);
    if (m_targets.empty())
        m_canAttack = false;
    _log(TARGET__TRACE, "ClearTarget:  %s(%u) has cleared target information for %s(%u).", \
                    mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID());
}

void TargetManager::ClearAllTargets(bool notify_self) {
    ClearTargets(notify_self);
    ClearFromTargets();
    _log(TARGET__TRACE, "ClearAllTargets:  %s(%u) has cleared all targeting information.", mySE->GetName(), mySE->GetID());
}

void TargetManager::ClearTargets(bool notify_self) {
    if (m_targets.empty()) {
        m_canAttack = false;
        return;
    }
    std::map<SystemEntity*, TargetEntry*>::iterator cur = m_targets.begin();
    for(; cur != m_targets.end(); cur++) {
        _log(TARGET__INFO, "%s(%u) has cleared target %s(%u) during clear all.",
                mySE->GetName(), mySE->GetID(), cur->first->GetName(), cur->first->GetID());
        cur->first->TargetMgr()->TargetedByLost(mySE);
        SafeDelete(cur->second);
    }
    m_targets.clear();

    if (notify_self)
        TargetsCleared();

    m_canAttack = false;
}

void TargetManager::ClearFromTargets() {
    if (m_targetedBy.empty()) return;

    std::vector<SystemEntity *> ToNotify;

    //first, clean up our internal structure.
    std::map<SystemEntity*, TargetedByEntry*>::iterator cur = m_targetedBy.begin();
    for (; cur != m_targetedBy.end(); cur++) {
        //do not notify until we clear our target list! otherwise bad things happen.
        ToNotify.push_back(cur->first);
        _log(TARGET__TRACE, "ClearFromTargets:  Added %s(%u) to delete list for %s(%u).", \
                            cur->first->GetName(), cur->first->GetID(), mySE->GetName(), mySE->GetID());
        SafeDelete(cur->second);
    }
    m_targetedBy.clear();

    for (auto cur : ToNotify)
        if (cur->TargetMgr())
            cur->TargetMgr()->TargetLost(mySE);
}

bool TargetManager::StartTargeting(SystemEntity *who, ShipItemRef ship)
{       // NOTE this is for players
    TargetTry(who);
    if (!mySE->HasPilot()) {
        codelog(TARGET__ERROR, "StartTargeting() called by pilot-less ship %s(%u) to target %s", mySE->GetName(), mySE->GetID(), who->GetName());
        return TargetFail(who);
    }

    //first make sure they are not already in the list
    std::map<SystemEntity *, TargetEntry *>::iterator res = m_targets.find(who);
    if (res != m_targets.end()) {
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but we are already targeting them. Ignoring request.", \
             mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID());
        return TargetFail(who);
    }
    //Check that they aren't targeting themselves (which may not be possible)
    if (who == mySE)
        return TargetFail(who);
    // Check invulnerability (undock and jump invul states)
    if (who->IsInvul()) {
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but they are Invul.  Ignoring request.", \
             mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID());
        return TargetFail(who);
    }
    // Check login for client just logging into game.
    if (who->IsLogin()) {
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but they are just Logging In.  Ignoring request.", \
             mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID());
        return TargetFail(who);
    }

    uint8 targetSkills = 1; //AttrMaxLockedTargets is for characters too!!
    Character* pChar = mySE->GetPilot()->GetChar().get();
    targetSkills += pChar->GetSkillLevel(skillTargeting);    // +1 target/level
    targetSkills += pChar->GetSkillLevel(skillMultitasking);    // +1 target/level
	uint8 maxLockedTargets = (uint8)ship->GetAttribute(AttrMaxLockedTargets).get_int();
    if (!maxLockedTargets) maxLockedTargets = 1;
    // add module updates to target capacity of ship here.
    if (targetSkills < maxLockedTargets)
        maxLockedTargets = targetSkills;
    if (GetTotalTargets() >= maxLockedTargets) {
        mySE->GetPilot()->SendInfoModalMsg("Your ship and skills combination can only handle %u targets at a time.", \
            maxLockedTargets);
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but we already have max targets.  Ignoring request.", \
             mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID());
        return TargetFail(who);
    }
    // Check against max locked target range
	double maxTargetLockRange = ship->GetAttribute(AttrMaxTargetRange).get_float();
    float targetRangeModifier = 1.0f;
    targetRangeModifier += (0.05 * pChar->GetSkillLevel(skillLongRangeTargeting)); // +5% level
    maxTargetLockRange *= targetRangeModifier;
    GVector rangeToTarget( mySE->GetPosition(), who->GetPosition() );
    // adjust for target radius, in case of ice or other large objects..
    double targetDistance = rangeToTarget.length();
    if (who->IsAsteroidSE())
        targetDistance -= who->GetRadius();
    if (targetDistance > maxTargetLockRange) {
        mySE->GetPilot()->SendInfoModalMsg("Your ship and skills combination can only target to %f meters.  %s is %f meters away.", \
            maxTargetLockRange, who->GetName(), targetDistance);
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but they are too far away.  Ignoring request.", \
             mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID());
        return TargetFail(who);
    }

    // Calculate Time to Lock target:
    float lockTime = TimeToLock( ship, who );

    TargetEntry *te = new TargetEntry(who);
        te->state = TargetEntry::Locking;
        te->timer.Start(lockTime *1000);      //timer has ms resolution
	m_targets[who] = te;
    who->TargetMgr()->TargetedAdd(mySE);

    _log(TARGET__INFO, "Pilot %s(%u) started targeting %s(%u) (%.2fs lock time)", \
                mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID(), lockTime);

    if (sConfig.server.IsTestServer or is_log_enabled(TARGET__DUMP))
        Dump();

    return true;
}

bool TargetManager::StartTargeting(SystemEntity *who, float lockTime, uint8 maxLockedTargets, double maxTargetLockRange, bool &chase)
{       // NOTE  this is for npcs
    //first make sure they are not already in the list
    std::map<SystemEntity *, TargetEntry *>::iterator res = m_targets.find(who);
    if (res != m_targets.end()) {
        //what to do?
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but we are already targeting them. Ignoring request.",
             mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID());
        return true;
    }
    TargetTry(who);
    //Check that they aren't targeting themselves (which may not be possible)
    if (who == mySE)
        return TargetFail(who);
    // Check against max locked target count
    if (m_targets.size() >= maxLockedTargets){
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but we already have max targets.  Ignoring request.",
             mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID());
        return TargetFail(who);
    }
    // Check against max locked target range
    if (mySE->GetPosition().distance(who->GetPosition()) > maxTargetLockRange){
        _log(TARGET__TRACE, " %s(%u): Told to target %s(%u), but they are too far away.  Begin Approaching.",
             mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID());
        chase = true;
        return TargetFail(who);
    }
    // Check invulnerability (undock and jump invul states)
    if (who->IsInvul()) {
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but they are Invul.  Ignoring request.",
             mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID());
        return TargetFail(who);
    }
    // Check login for client just logging into game.
    if (who->IsLogin()) {
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but they are just Logging In.  Ignoring request.",
             mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID());
        return TargetFail(who);
    }

    TargetEntry *te = new TargetEntry(who);
        te->state = TargetEntry::Locking;
        te->timer.Start(lockTime);
    m_targets[who] = te;
    who->TargetMgr()->TargetedAdd(mySE);

    _log(TARGET__INFO, "NPC %s(%u) started targeting %s(%u) (%.2fs lock time)",
         mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID(), (lockTime /1000));

    if (sConfig.server.IsTestServer or is_log_enabled(TARGET__DUMP))
        Dump();

    return true;
}

void TargetManager::TargetLost(SystemEntity *who) {
    std::map<SystemEntity *, TargetEntry *>::iterator res = m_targets.find(who);
    if (res == m_targets.end())
        return;

    SafeDelete(res->second);
    m_targets.erase(res);

    _log(TARGET__INFO, "%s(%u) has lost lock on %s(%u)",
         mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID());

    if (mySE->IsNPCSE())
        mySE->GetNPCSE()->TargetLost(who);

    if (!mySE->HasPilot()) return;
    Notify_OnTarget te;
        te.mode = "lost";
        te.targetID = who->GetID();
        //te.reason = "Docking";
    Notify_OnMultiEvent multi;
        multi.events = new PyList;
        multi.events->AddItem(te.Encode());
    PyTuple* tmp = multi.Encode();   //this is consumed below
    mySE->GetPilot()->SendNotification("OnMultiEvent", "clientID", &tmp);
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
         mySE->GetName(), mySE->GetID(), from_who->GetName(), from_who->GetID());
    mySE->TargetMgr()->TargetedAdd(from_who);
}

void TargetManager::TargetedByLost(SystemEntity *from_who) {
    std::map<SystemEntity *, TargetedByEntry *>::iterator res = m_targetedBy.find(from_who);
    if (res != m_targetedBy.end()) {
        SafeDelete(res->second);
        m_targetedBy.erase(res);
        TargetedLost(from_who);
        _log(TARGET__INFO, "%s(%u) is no longer locked by %s(%u)",
             mySE->GetName(), mySE->GetID(), from_who->GetName(), from_who->GetID());
    } else {
        _log(TARGET__DEBUG, "%s(%u) was notified of targeted lost by %s(%u), but they did not have us targeted in the first place.",
             mySE->GetName(), mySE->GetID(), from_who->GetName(), from_who->GetID());
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
            _log(TARGET__WARNING, "Found target %u, but it is not locked.", targetID);
            continue;
        }
        _log(TARGET__INFO, "Found target %u: %s (nl? %s)", targetID, cur->first->GetName(), need_locked?"yes":"no");
        return(cur->first);
    }
    _log(TARGET__WARNING, "Unable to find target %u (nl? %s)", targetID, need_locked?"yes":"no");
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

float TargetManager::TimeToLock(ShipItemRef ship, SystemEntity *target) const {
    if ( (target->IsAsteroidSE()) || (target->IsDeployableSE()) || (target->IsWreckSE())
        || (target->IsContainerSE()) || (target->IsInanimateSE()) || (target->IsStaticEntity()) )
        return 2.0;

    //  fixed lock time  -allan 24Dec14  -updated 26May15
    /** @todo add ship bonuses in here */
    uint32 scanRes = ship->GetAttribute(AttrScanResolution).get_int();
    uint32 sigRad = 25; // set base as capsule with 25m signature radius

	if ( target->GetSelf() )
        if ( target->GetSelf()->HasAttribute(AttrSignatureRadius) )
            sigRad = target->GetSelf()->GetAttribute(AttrSignatureRadius).get_int();

    /*
     * fleet invlovement enhances targeting speed using leadership of highest member (2%/lvl)
     * modules - sensor boosters
     */

    //https://wiki.eveonline.com/en/wiki/Targeting_speed
    //locktime = 40000/(scanres * asinh(sigrad)^2)
    float time = ( 40000 /(scanRes * pow(asinh(sigRad), 2)));

    if (mySE->HasPilot()) {
        Character* pChar = mySE->GetPilot()->GetChar().get();
        time *= (1 - (0.05 * pChar->GetSkillLevel(skillSignatureAnalysis))); // 5% decrease/level
        if (pChar->fleetID()) { /** @todo  always returns 0 until fleets are implemented */
            //Character* pLeader = pChar->GetFleetLeader;   /** @todo this needs to be written */
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
    if (mySE->IsNPCSE())
        if (distance > 85000)
            distance -= 75000;

    float disMod = distance /10000;
    if (disMod < 0) disMod = 0;
    time += (disMod *0.1);

	return time;
}

/*
    OnTarget.mode
        try - starting to target?
        add - targeting successful
        fail - targeting unsuccessful
        clear - clear all targets
        lost - target lost
            - Docking
        otheradd - somebody else has targeted you
        otherlost - somebody else has stopped targeting you
            - WarpingOut
            - StoppedTargeting
        otherfail - problem with somebody else targeting you
            - StoppedTargeting
*/
void TargetManager::TargetTry(SystemEntity *who) {
    if (!mySE->HasPilot()) return;
    Notify_OnTarget te;
        te.mode = "try";
        te.targetID = who->GetID();
    Notify_OnMultiEvent multi;
        multi.events = new PyList;
        multi.events->AddItem(te.Encode());
    PyTuple* tmp = multi.Encode();   //this is consumed below
    mySE->SysBubble()->BubblecastSendNotification("OnMultiEvent", "clientID", &tmp, false);
}

bool TargetManager::TargetFail(SystemEntity* who) {
    if (!mySE->HasPilot()) return false;
    Notify_OnTarget te;
        te.mode = "fail";
        te.targetID = who->GetID();
    Notify_OnMultiEvent multi;
        multi.events = new PyList;
        multi.events->AddItem(te.Encode());
    PyTuple* tmp = multi.Encode();   //this is consumed below
    mySE->SysBubble()->BubblecastSendNotification("OnMultiEvent", "clientID", &tmp, false);
    return false;
}

void TargetManager::TargetAdded(SystemEntity* who) {
    if (!mySE->HasPilot()) return;
    PyTuple* up(nullptr);
    DoDestiny_OnDamageStateChange odsc;
        odsc.entityID = who->GetID();
        odsc.state = who->MakeDamageState();
    up = odsc.Encode();
    mySE->GetPilot()->QueueDestinyUpdate(&up);
    Notify_OnTarget te;
        te.mode = "add";
        te.targetID = who->GetID();
    up = te.Encode();
    mySE->GetPilot()->QueueDestinyEvent(&up);
    PySafeDecRef(up);
}

void TargetManager::TargetedAdd(SystemEntity *who) {
    if (mySE->IsNPCSE())
        mySE->GetNPCSE()->TargetedAdd(who);
    if (!mySE->HasPilot()) return;
    Notify_OnTarget te;
        te.mode = "otheradd";
        te.targetID = who->GetID();
    Notify_OnMultiEvent multi;
        multi.events = new PyList;
        multi.events->AddItem(te.Encode());
    PyTuple* tmp = multi.Encode();   //this is consumed below
    mySE->GetPilot()->SendNotification("OnMultiEvent", "clientID", &tmp);
}

void TargetManager::TargetedLost(SystemEntity *who) {
    if (!mySE->HasPilot()) return;
    Notify_OnTarget te;
        te.mode = "otherlost";
        te.targetID = who->GetID();
       // te.reason = "WarpingOut";
       // te.reason = "StoppedTargeting";
    Notify_OnMultiEvent multi;
        multi.events = new PyList;
        multi.events->AddItem(te.Encode());
    PyTuple* tmp = multi.Encode();   //this is consumed below
    mySE->GetPilot()->SendNotification("OnMultiEvent", "clientID", &tmp);
}

void TargetManager::TargetsCleared() {
    if (!mySE->HasPilot()) return;
    Notify_OnTarget te;
        te.mode = "clear";
        te.targetID = 0;
    Notify_OnMultiEvent multi;
        multi.events = new PyList;
        multi.events->AddItem(te.Encode());
    PyTuple* tmp = multi.Encode();   //this is consumed below
    mySE->GetPilot()->SendNotification("OnMultiEvent", "clientID", &tmp);
}

void TargetManager::QueueTBDestinyEvent( PyTuple** up_in ) const
{
    PyTuple* up = *up_in;
    *up_in = nullptr;    //could optimize out one of the Clones in here...

    PyTuple* up_dup(nullptr);

    for (auto cur : m_targetedBy) {
        if (cur.first->HasPilot()) {
            if (!up_dup)
                up_dup = new PyTuple( *up );

            cur.first->GetPilot()->QueueDestinyEvent( &up_dup );
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
        if (cur.first->HasPilot()) {
            if (!up_dup)
                up_dup = new PyTuple( *up );

            cur.first->GetPilot()->QueueDestinyUpdate( &up_dup );
        }
    }

    PySafeDecRef( up_dup );
    PyDecRef( up );
}

/* debugging methods */
void TargetManager::TargetList(std::string* into, uint16* length, uint16* count) {
    for (auto cur : m_targets)
        ++count;
    for (auto cur : m_targetedBy)
        ++count;
}

void TargetManager::Dump() const {
    _log(TARGET__DUMP, "Target Dump for %s(%u):", mySE->GetName(), mySE->GetID());
    for (auto cur : m_targets)
        cur.second->Dump();
    for (auto cur : m_targetedBy)
        cur.second->Dump();
}

void TargetManager::TargetEntry::Dump() const {
    const char *sname = "Unknown State";
    switch(state) {
        case Idle:              sname = "Idle";    break;
        case PassiveLocking:    sname = "Passive"; break;
        case Locking:           sname = "Locking"; break;
        case Locked:            sname = "Locked";  break;
    }
    _log(TARGET__DUMP, "    Targeted %s(%u): %s (Timer %s with %ums remaining)", \
                who->GetName(), who->GetID(), sname, timer.Enabled() ? "Running" : "Disabled", timer.GetRemainingTime());
}

void TargetManager::TargetedByEntry::Dump() const {
    const char *sname = "Unknown State";
    switch(state) {
        case Idle:      sname = "Idle";     break;
        case Locking:   sname = "Locking";  break;
        case Locked:    sname = "Locked";   break;
    }
    _log(TARGET__DUMP, "    Targeted By %s(%u): %s", who->GetName(), who->GetID(), sname);
}

