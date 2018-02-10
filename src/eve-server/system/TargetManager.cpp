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
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "pos/Structure.h"
#include "pos/Tower.h"
#include "ship/Ship.h"
#include "ship/modules/ActiveModule.h"
#include "system/TargetManager.h"
#include "system/SystemEntity.h"
#include "system/SystemBubble.h"

TargetManager::TargetManager(SystemEntity *self)
: mySE(self)
{
    m_canAttack = false;

    m_modules.clear();
    m_targets.clear();
    m_targetedBy.clear();
}

void TargetManager::Process() {
     double profileStartTime = 0.0;
     if (sConfig.debug.UseProfiling)
         profileStartTime = GetTimeUSeconds();

    //process outgoing targeting (outgoing will call incomming as needed)
    std::map<SystemEntity*, TargetEntry*>::iterator itr = m_targets.begin();
    while (itr != m_targets.end()) {
        if ((itr->first == nullptr) or (itr->second == nullptr)) {
            itr = m_targets.erase(itr);
            continue;
        }
        switch (itr->second->state) {
            case TargetEntry::Idle:
            case TargetEntry::Locked:{          //do nothing
            } break;
            case TargetEntry::PassiveLocking:   // this will be used with stealth modules (which, ofc, are not written yet)
            case TargetEntry::Locking: {
                if (itr->second->timer.Check(false)) {
                    itr->second->timer.Disable();
                    itr->second->state = TargetEntry::Locked;
                    _log(TARGET__TRACE, "%s(%u) has finished locking %s(%u)", \
                                mySE->GetName(), mySE->GetID(), itr->first->GetName(), itr->first->GetID());
                    TargetAdded(itr->first);
                    itr->first->TargetMgr()->TargetedByLocked(mySE);
                    m_canAttack = true;
                }
            } break;
        }
        ++itr;
    }

    if (sConfig.debug.UseProfiling)
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

void TargetManager::ClearAllTargets(bool notify_self/*true*/) {
    ClearTargets(notify_self);
    ClearFromTargets();
    _log(TARGET__TRACE, "ClearAllTargets:  %s(%u) has cleared all targeting information.", mySE->GetName(), mySE->GetID());
}

void TargetManager::ClearTargets(bool notify_self/*true*/) {
    if (m_targets.empty()) {
        m_canAttack = false;
        return;
    }
    std::map<SystemEntity*, TargetEntry*>::iterator cur = m_targets.begin();
    for(; cur != m_targets.end(); ++cur) {
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
    if (m_targetedBy.empty())
        return;

    std::vector<SystemEntity *> ToNotify;
    std::map<SystemEntity*, TargetedByEntry*>::iterator cur = m_targetedBy.begin();
    for (; cur != m_targetedBy.end(); ++cur) {
        //do not notify until we clear our target list! otherwise bad things happen.
        ToNotify.push_back(cur->first);
        _log(TARGET__TRACE, "ClearFromTargets:  Added %s(%u) to delete list for %s(%u).", \
                            cur->first->GetName(), cur->first->GetID(), mySE->GetName(), mySE->GetID());
        SafeDelete(cur->second);
    }
    m_targetedBy.clear();

    for (auto cur : ToNotify)
        if (cur->TargetMgr() != nullptr)
            cur->TargetMgr()->TargetLost(mySE);
}
/*{'messageKey': 'DeniedTargetAfterCloak', 'dataID': 17883412, 'suppressable': False, 'bodyID': 259495, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 781}
 * {'messageKey': 'DeniedTargetEvadesSensors', 'dataID': 17883870, 'suppressable': False, 'bodyID': 259657, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 782}
 * {'messageKey': 'DeniedTargetForceField', 'dataID': 17883882, 'suppressable': False, 'bodyID': 259661, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 783}
 * {'messageKey': 'DeniedTargetInvulnerable', 'dataID': 17883415, 'suppressable': False, 'bodyID': 259496, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 784}
 * {'messageKey': 'DeniedTargetOtherFrozen', 'dataID': 17883876, 'suppressable': False, 'bodyID': 259659, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 785}
 * {'messageKey': 'DeniedTargetOtherWarping', 'dataID': 17883809, 'suppressable': False, 'bodyID': 259635, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 786}
 * {'messageKey': 'DeniedTargetReinforcedStructure', 'dataID': 17883888, 'suppressable': False, 'bodyID': 259663, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 787}
 * {'messageKey': 'DeniedTargetSelf', 'dataID': 17883418, 'suppressable': False, 'bodyID': 259497, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 788}
 * {'messageKey': 'DeniedTargetSelfFrozen', 'dataID': 17883873, 'suppressable': False, 'bodyID': 259658, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 789}
 * {'messageKey': 'DeniedTargetSelfWarping', 'dataID': 17883879, 'suppressable': False, 'bodyID': 259660, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 790}
 * {'messageKey': 'DeniedTargetUntargetable', 'dataID': 17880323, 'suppressable': False, 'bodyID': 258348, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2170}
 * {'messageKey': 'DeniedTargetingAttemptFailed', 'dataID': 17883942, 'suppressable': False, 'bodyID': 259683, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 791}
 * {'messageKey': 'DeniedTargetingCloaked', 'dataID': 17883664, 'suppressable': False, 'bodyID': 259583, 'messageType': 'notify', 'urlAudio': 'wise:/msg_DeniedTargetingCloaked_play', 'urlIcon': '', 'titleID': None, 'messageID': 792}
 * {'messageKey': 'DeniedTargetingInsideField', 'dataID': 17883885, 'suppressable': False, 'bodyID': 259662, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 793}
 * {'messageKey': 'DeniedTargetingTargetCloaked', 'dataID': 17883421, 'suppressable': False, 'bodyID': 259498, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 794}
 */

/*{'FullPath': u'UI/Messages', 'messageID': 259683, 'label': u'DeniedTargetingAttemptFailedBody'}(u'Your attempt to target {[item]target.name} failed.', None, {u'{[item]target.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'target'}})
 */
bool TargetManager::StartTargeting(SystemEntity *who, ShipItemRef sRef)
{       // NOTE this is for players and CAN throw (client calls this inside try/catch block)

    if (!mySE->HasPilot()) {
        codelog(TARGET__ERROR, "StartTargeting() called by pilot-less ship %s(%u) to target %s", mySE->GetName(), mySE->GetID(), who->GetName());
        return false;
    }

    if (who == mySE)
        throw PyException( MakeUserError("DeniedTargetSelf"));
    if (who->IsInvul())
        throw PyException( MakeUserError("DeniedTargetInvulnerable"));
    if ((who->TargetMgr() == nullptr) or (who->GetSelf()->HasAttribute(AttrUntargetable))) { //only for 21094, 28650 (cyno fields)
        std::map<std::string, PyRep *> args;
        args["targetName"] = new PyString(who->GetName());
        throw PyException( MakeUserError("DeniedTargetEvadesSensors", args));
    }
    if (who->DestinyMgr() != nullptr) {
        if (who->DestinyMgr()->IsCloaked())
            throw PyException( MakeUserError("DeniedTargetingTargetCloaked"));
        if (who->DestinyMgr()->IsWarping()) {
            std::map<std::string, PyRep *> args;
            args["targetName"] = new PyString(who->GetName());
            throw PyException( MakeUserError("DeniedTargetOtherWarping", args));
        }
    }
    if (who->IsPOSSE())
        if (who->GetPOSSE()->IsReinforced()) {
            std::map<std::string, PyRep *> args;
            args["target"] = new PyInt(who->GetID());
            throw PyException( MakeUserError("DeniedTargetReinforcedStructure", args));
        }
    /** @todo figure out how to determine being inside forcefield...
    if (who->InsideForceField()){
        std::map<std::string, PyRep *> args;
        args["target"] = new PyInt(who->GetID());
        args["range"] = new PyInt(who->GetTowerSE()->GetSOI());
        args["item"] = new PyInt(who->GetTowerSE()->GetID());
        throw PyException( MakeUserError("DeniedTargetForceField", args));
    }
    if (mySE->InsideForceField()) {
        std::map<std::string, PyRep *> args;
        args["target"] = new PyInt(who->GetID());
        throw PyException( MakeUserError("DeniedTargetingInsideField", args));
    } */

    if (mySE->DestinyMgr() != nullptr) {
        if (mySE->DestinyMgr()->IsWarping())
            throw PyException( MakeUserError("DeniedTargetSelfWarping"));
        if (mySE->DestinyMgr()->IsCloaked())
            throw PyException( MakeUserError("DeniedTargetingCloaked"));
    }

    TargetTry(who);
    //first make sure they are not already in the list
    std::map<SystemEntity *, TargetEntry *>::iterator res = m_targets.find(who);
    if (res != m_targets.end()) {
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but we are already targeting them. Ignoring request.", \
             mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID());
        return TargetFail(who);
    }
    // Check login for client just logging into game.
    if (who->IsLogin()) {
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but they are just Logging In.  Ignoring request.", \
             mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID());
        return TargetFail(who);
    }

	uint8 maxLockedTargets = (uint8)sRef->GetAttribute(AttrMaxLockedTargets).get_int();
    if (maxLockedTargets < 1)
        maxLockedTargets = 1;
    if (GetTotalTargets() >= maxLockedTargets) {
        mySE->GetPilot()->SendInfoModalMsg("Your ship and skills combination can only handle %u targets at a time.", maxLockedTargets);
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but we already have max targets.  Ignoring request.", \
             mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID());
        return TargetFail(who);
    }
    // Check against max target range
    double maxTargetRange = sRef->GetAttribute(AttrMaxTargetRange).get_double();
    GVector rangeToTarget( mySE->GetPosition(), who->GetPosition() );
    // adjust for target radius, in case of ice or other large objects..
    double targetDistance = rangeToTarget.length();
    if (who->IsAsteroidSE())
        targetDistance -= who->GetRadius();
    if (targetDistance > maxTargetRange) {
        mySE->GetPilot()->SendInfoModalMsg("Your ship and skills combination can only target to %.0f meters.  %s is %.0f meters away.", \
                        maxTargetRange, who->GetName(), targetDistance);
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but they are too far away.  Ignoring request.", \
                    mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID());
        return TargetFail(who);
    }

    // Calculate Time to Lock target:
    float lockTime = TimeToLock( sRef, who );

    TargetEntry *te = new TargetEntry(who);
        te->state = TargetEntry::Locking;
        te->timer.Start(lockTime *1000);      //timer has ms resolution
	m_targets[who] = te;
    who->TargetMgr()->TargetedAdd(mySE);

    _log(TARGET__INFO, "Pilot %s(%u) started targeting %s(%u) (%.2fs lock time)", \
                mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID(), lockTime);

    if (is_log_enabled(TARGET__DUMP))
        Dump();

    return true;
}

bool TargetManager::StartTargeting(SystemEntity *who, float lockTime, uint8 maxLockedTargets, double maxTargetLockRange, bool &chase)
{       // NOTE  this is for npcs
    //first make sure they are not already in the list
    std::map<SystemEntity *, TargetEntry *>::iterator res = m_targets.find(who);
    if (res != m_targets.end()) {
        //what to do?
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but we are already targeting them. Ignoring request.", \
             mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID());
        return true;
    }
    TargetTry(who);
    // Check against max locked target count
    if (m_targets.size() >= maxLockedTargets){
        _log(TARGET__DEBUG, " %s(%u): Told to target %s(%u), but we already have max targets.  Ignoring request.", \
             mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID());
        return TargetFail(who);
    }
    // Check against max target range
    if (mySE->GetPosition().distance(who->GetPosition()) > maxTargetLockRange){
        _log(TARGET__TRACE, " %s(%u): Told to target %s(%u), but they are too far away.  Begin Approaching.", \
             mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID());
        chase = true;
        return TargetFail(who);
    }

    TargetEntry *te = new TargetEntry(who);
        te->state = TargetEntry::Locking;
        te->timer.Start(lockTime);
    m_targets[who] = te;
    who->TargetMgr()->TargetedAdd(mySE);

    _log(TARGET__INFO, "NPC %s(%u) started targeting %s(%u) (%.2fs lock time)", \
         mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID(), (lockTime /1000));

    if (is_log_enabled(TARGET__DUMP))
        Dump();

    return true;
}

void TargetManager::TargetLost(SystemEntity *who) {
    std::map<SystemEntity *, TargetEntry *>::iterator itr = m_targets.find(who);
    if (itr == m_targets.end())
        return;

    SafeDelete(itr->second);
    m_targets.erase(itr);

    _log(TARGET__INFO, "%s(%u) has lost lock on %s(%u)", mySE->GetName(), mySE->GetID(), who->GetName(), who->GetID());

    if (mySE->IsSentrySE())
        return;

    mySE->DestinyMgr()->EntityRemoved(who);
    if (mySE->IsNPCSE())
        mySE->GetNPCSE()->TargetLost(who);
    if (!mySE->HasPilot())
        return;
    Notify_OnTarget te;
        te.mode = "lost";
        te.targetID = who->GetID();
        //te.reason = "Docking";
    Notify_OnMultiEvent multi;
        multi.events = new PyList();
        multi.events->AddItem(te.Encode());
    PyTuple* tmp = multi.Encode();   //this is consumed below
    mySE->GetPilot()->SendNotification("OnMultiEvent", "clientID", &tmp);
}

void TargetManager::TargetedByLocked(SystemEntity *from_who) {
    //first make sure they are not already in the list
    std::map<SystemEntity *, TargetedByEntry *>::iterator itr = m_targetedBy.find(from_who);
    if (itr != m_targetedBy.end()) {
        //just re-use the old entry...
        itr->second->state = TargetedByEntry::Locked;
        return;
    } else {
        //new entry.
        TargetedByEntry *te = new TargetedByEntry(from_who);
        te->state = TargetedByEntry::Locked;
        m_targetedBy[from_who] = te;
    }
    _log(TARGET__TRACE, "%s(%u) has been locked by %s(%u)", \
         mySE->GetName(), mySE->GetID(), from_who->GetName(), from_who->GetID());
    mySE->TargetMgr()->TargetedAdd(from_who);
}

void TargetManager::TargetedByLost(SystemEntity *from_who) {
    std::map<SystemEntity *, TargetedByEntry *>::iterator itr = m_targetedBy.find(from_who);
    if (itr != m_targetedBy.end()) {
        SafeDelete(itr->second);
        m_targetedBy.erase(itr);
        TargetedLost(from_who);
        _log(TARGET__INFO, "%s(%u) is no longer locked by %s(%u)", \
             mySE->GetName(), mySE->GetID(), from_who->GetName(), from_who->GetID());
    } else {
        _log(TARGET__DEBUG, "%s(%u) was notified of targeted lost by %s(%u), but they did not have us targeted.", \
             mySE->GetName(), mySE->GetID(), from_who->GetName(), from_who->GetID());
    }
}

bool TargetManager::IsTargetedBy(SystemEntity* pSE)
{
    std::map<SystemEntity *, TargetedByEntry *>::iterator itr = m_targetedBy.find(pSE);
    if (itr != m_targetedBy.end())
        return true;
    return false;
}

SystemEntity* TargetManager::GetFirstTarget(bool need_locked/*false*/) {
    if (m_targets.empty())
        return nullptr;

    if (!need_locked)
        return m_targets.begin()->first;

    std::map<SystemEntity *, TargetEntry *>::iterator itr = m_targets.begin();
    for (; itr != m_targets.end(); ++itr)
        if (itr->second->state == TargetEntry::Locked)
            return itr->first;

    return nullptr;
}

PyList* TargetManager::GetTargets() const {
    PyList* result = new PyList();
    if (m_targets.empty())
        return result;

    std::map<SystemEntity *, TargetEntry *>::const_iterator itr = m_targets.begin();
    for (; itr != m_targets.end(); ++itr)
        result->AddItemInt( itr->first->GetID() );

    return result;
}

// no longer used.  1Feb18
SystemEntity* TargetManager::GetTarget(uint32 targetID, bool need_locked/*true*/) const {
    if (m_targets.empty())
        return nullptr;

    std::map<SystemEntity*, TargetEntry*>::const_iterator itr = m_targets.begin();
    for (; itr != m_targets.end(); ++itr) {
        if (itr->first->GetID() != targetID)
            continue;
        //found it...
        if (need_locked and (itr->second->state != TargetEntry::Locked)) {
            _log(TARGET__WARNING, "Found target %u, but it is not locked.", targetID);
            continue;
        }
        _log(TARGET__INFO, "Found target %u: %s (nl? %s)", targetID, itr->first->GetName(), need_locked?"yes":"no");
        return itr->first;
    }
    _log(TARGET__WARNING, "Unable to find target %u (nl? %s)", targetID, need_locked?"yes":"no");
    return nullptr;    //not found.
}

PyList* TargetManager::GetTargeters() const {
    PyList* result = new PyList();
    if (m_targetedBy.empty())
        return result;

    std::map<SystemEntity*, TargetedByEntry*>::const_iterator itr = m_targetedBy.begin();
    for(; itr != m_targetedBy.end(); ++itr)
        result->AddItemInt( itr->first->GetID() );

    return result;
}

float TargetManager::TimeToLock(ShipItemRef ship, SystemEntity *target) const {
    if ((target->IsAsteroidSE()) or (target->IsDeployableSE()) or (target->IsWreckSE())
    or  (target->IsContainerSE()) or (target->IsInanimateSE()) or (target->IsStaticEntity()))
        return 2.0;

    //  fixed lock time  -allan 24Dec14  -updated 26May15   -revisited after new effects system implementation 25Mar17
    uint32 scanRes = ship->GetAttribute(AttrScanResolution).get_int();
    uint32 sigRad = 25; // set base as capsule with 25m signature radius

	if ( target->GetSelf().get() != nullptr )
        if ( target->GetSelf()->HasAttribute(AttrSignatureRadius) )
            sigRad = target->GetSelf()->GetAttribute(AttrSignatureRadius).get_int();

    //https://wiki.eveonline.com/en/wiki/Targeting_speed
    //locktime = 40000/(scanres * asinh(sigrad)^2)
    float time = ( 40000 /(scanRes * std::pow(std::asinh(sigRad), 2)));   // higher scan res means faster lock time.

    /*  distance-based modifier to targeting speed?         sure, why the hell not?   -allan 27.6.15
     *  +0.1s for each 10k distance
     *     distance = pos - targ.pos
     *     disMod = distance /10k (for 10k increments)
     *     time += disMod * 0.1
     */
    double distance = ship->position().distance(target->GetPosition());
    // check for snipers... >85k distance do NOT need additional 7.5+s to targettime
    // should we check LDT skill for pilots to modify this?  yes....not sure how
    //if (mySE->IsNPCSE())      // not all snipers are npc
        if (distance > 85000)
            distance -= 75000;

    float disMod = distance /10000;
    if (disMod < 1) disMod = 0;
    time += (disMod *0.1);

	return time;
}

/*
    OnTarget.mode (* means not defined in client - that i've found.)
        *try - starting to target?
        add - targeting successful
        *fail - targeting unsuccessful
        clear - clear all targets
        lost - target lost
            - Docking
        otheradd - somebody else has targeted you
        otherlost - somebody else has stopped targeting you
            - WarpingOut
            - StoppedTargeting
        *otherfail - problem with somebody else targeting you
            - StoppedTargeting
*/
void TargetManager::TargetTry(SystemEntity *who) {
    if (!mySE->HasPilot())
        return;
    Notify_OnTarget te;
        te.mode = "try";
        te.targetID = who->GetID();
    Notify_OnMultiEvent multi;
        multi.events = new PyList();
        multi.events->AddItem(te.Encode());
    PyTuple* tmp = multi.Encode();
    mySE->SysBubble()->BubblecastSendNotification("OnMultiEvent", "clientID", &tmp, false);
}

bool TargetManager::TargetFail(SystemEntity* who) {
    if (!mySE->HasPilot())
        return false;
    Notify_OnTarget te;
        te.mode = "fail";
        te.targetID = who->GetID();
    Notify_OnMultiEvent multi;
        multi.events = new PyList();
        multi.events->AddItem(te.Encode());
    PyTuple* tmp = multi.Encode();
    mySE->SysBubble()->BubblecastSendNotification("OnMultiEvent", "clientID", &tmp, false);
    return false;
}

void TargetManager::TargetAdded(SystemEntity* who) {
    if (!mySE->HasPilot())
        return;
    PyTuple* up(nullptr);
    Notify_OnTarget te;
        te.mode = "add";
        te.targetID = who->GetID();
    up = te.Encode();
    mySE->GetPilot()->QueueDestinyEvent(&up);
    OnDamageStateChange odsc;
        odsc.entityID = who->GetID();
        odsc.state = who->MakeDamageState();
    up = odsc.Encode();
    mySE->GetPilot()->QueueDestinyUpdate(&up);
}

void TargetManager::TargetedAdd(SystemEntity *who) {
    if (mySE->IsNPCSE())
        mySE->GetNPCSE()->TargetedAdd(who);
    if (!mySE->HasPilot())
        return;
    Notify_OnTarget te;
        te.mode = "otheradd";
        te.targetID = who->GetID();
    Notify_OnMultiEvent multi;
        multi.events = new PyList();
        multi.events->AddItem(te.Encode());
    PyTuple* tmp = multi.Encode();
    mySE->GetPilot()->SendNotification("OnMultiEvent", "clientID", &tmp);
}

void TargetManager::TargetedLost(SystemEntity *who) {
    if (!mySE->HasPilot())
        return;
    Notify_OnTarget te;
        te.mode = "otherlost";
        te.targetID = who->GetID();
       // te.reason = "WarpingOut";
       // te.reason = "StoppedTargeting";
    Notify_OnMultiEvent multi;
        multi.events = new PyList();
        multi.events->AddItem(te.Encode());
    PyTuple* tmp = multi.Encode();
    mySE->GetPilot()->SendNotification("OnMultiEvent", "clientID", &tmp);
}

void TargetManager::TargetsCleared() {
    if (!mySE->HasPilot())
        return;
    Notify_OnTarget te;
        te.mode = "clear";
        te.targetID = 0;
    Notify_OnMultiEvent multi;
        multi.events = new PyList();
        multi.events->AddItem(te.Encode());
    PyTuple* tmp = multi.Encode();
    mySE->GetPilot()->SendNotification("OnMultiEvent", "clientID", &tmp);
}

void TargetManager::AddTargetModule(ActiveModule* pMod)
{
    m_modules.emplace(pMod->itemID(), pMod);
}

void TargetManager::RemoveTargetModule(ActiveModule* pMod)
{
    m_modules.erase(pMod->itemID());
}

void TargetManager::Destroyed()
{
    std::string effect = "TargetDestroyed";
    // iterate thru the map of modules targeting this object, and call Deactivate on each.
    for (auto cur : m_modules)
        cur.second->Deactivate(effect);
}

/* unused at this time */
void TargetManager::QueueTBDestinyEvent( PyTuple** event ) const
{
    for (auto cur : m_targetedBy)
        if (cur.first->HasPilot())
            cur.first->GetPilot()->QueueDestinyEvent(event);
}

void TargetManager::QueueTBDestinyUpdate( PyTuple** update ) const
{
    for (auto cur : m_targetedBy)
        if (cur.first->HasPilot()) {
            PyIncRef(*update);
            cur.first->GetPilot()->QueueDestinyUpdate(update);
        }
}

/* debugging methods */
std::string TargetManager::TargetList(uint16 &length, uint16 &count) {
    std::ostringstream str;
    if (!m_targets.empty()) {
        str << "Targets: <br>";
        length += 11;
        for (auto cur : m_targets) {
            str << "  " << cur.second->who->GetSelf()->itemName();
            str << " (" << cur.second->who->GetID() << ") <br>";
            length += 35;
            ++count;
        }
    }
    if (!m_targetedBy.empty()) {
        str << "Targeted by: <br>";
        length += 15;
        for (auto cur : m_targetedBy) {
            str << "  " << cur.second->who->GetSelf()->itemName();
            str << " (" << cur.second->who->GetID() << ") <br>";
            length += 35;
            ++count;
        }
    }
    return str.str();
}

void TargetManager::Dump() const {
    _log(TARGET__DUMP, "Target Dump for %s(%u):", mySE->GetName(), mySE->GetID());
    for (auto cur : m_targets)
        cur.second->Dump();
    for (auto cur : m_targetedBy)
        cur.second->Dump();

    _log(TARGET__DUMP, "    Targeted By Modules: (ship:module)");
    for (auto cur : m_modules) {
        _log(TARGET__DUMP, "\t\t %s: %s", cur.second->GetShipRef()->itemName().c_str(), cur.second->GetSelf()->itemName().c_str());
    }
}

void TargetManager::TargetEntry::Dump() const {
    const char *sname = "Invalid";
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
    const char *sname = "Invalid";
    switch(state) {
        case Idle:      sname = "Idle";     break;
        case Locking:   sname = "Locking";  break;
        case Locked:    sname = "Locked";   break;
    }
    _log(TARGET__DUMP, "    Targeted By %s(%u): %s", who->GetName(), who->GetID(), sname);
}

