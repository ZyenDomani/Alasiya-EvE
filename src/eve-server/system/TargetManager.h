/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
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


#ifndef __TARGETMANAGER_H_INCL__
#define __TARGETMANAGER_H_INCL__

#include "inventory/ItemRef.h"

namespace TargMgr {
    namespace State {
        enum {
            Idle                = 0,
            Locking             = 1,
            Passive             = 2,
            Locked              = 3
        };
    }
    namespace Mode {
        enum {
            None                = 0,
            Add                 = 1,
            Lost                = 2,
            Clear               = 3,
            OtherAdd            = 4,
            OtherLost           = 5,
            LockedBy            = 6
        };
    }
    namespace Msg {
        enum {
            NoMsg               = 0,
            Locked              = 1,
            Docking             = 2,
            Jumping             = 3,
            Cloaked             = 4,
            WarpingOut          = 5,
            StoppedTargeting    = 6,
            Destroyed           = 7,
            ClientReq           = 8,
            InternalCall        = 9,
            Deleted             = 10,
            Shutdown            = 11
        };
    }
}


class MiningLaser;
class ActiveModule;
class SystemEntity;
class PyRep;
class PyTuple;
class PyList;

class TargetEntry {
public:
    TargetEntry()
    : state(TargMgr::State::Idle), timer(0) {}

    void Dump(SystemEntity* pSE) const;

    uint8 state;

    Timer timer;
};

class TargetedByEntry {
public:
    TargetedByEntry()
    : state(TargMgr::State::Idle) {}

    void Dump(SystemEntity* pSE) const;

    uint8 state;
};

class TargetManager {
public:
    TargetManager(SystemEntity* self);
    ~TargetManager()                            { /* do nothing here */ }
    TargetManager(const TargetManager&) =delete;
    TargetManager& operator=(const TargetManager&) =delete;

    static const char*  GetModeName(uint8 mode);
    static const char*  GetStateName(uint8 state);

    /* Common Methods for all objects */
    bool                Process();
    void                Unload();       // called on npcs from sysMgr when unloading system.

    // iterate thru the map of modules targeting this object and call AbortCycle on each.
    void                ClearModules();
    
    void                TargetsCleared();
    void                ClearFromTargets();
    void                TargetLost(SystemEntity *tSE);
    void                ClearTarget(SystemEntity *tSE);
    void                TargetAdded(SystemEntity *tSE);
    void                ClearTargets(bool notify=true);
    void                ClearAllTargets(bool notify=true);

    /* method to remove target without triggering anything else (target destroyed) */
    void                RemoveTarget(SystemEntity* tSE);

    /*
     *    OnTarget.mode
     *        add - targeting successful
     *        clear - clear all targets
     *        lost - target lost (reason not used)
     *            - Docking
     *            - Cloaked
     *        otheradd - somebody else has targeted you
     *        otherlost - somebody else has stopped targeting you (reason not used)
     *            - WarpingOut
     *            - StoppedTargeting
     *            - Destroyed
     *
     *    OnTargetClear - immediately removes all target info from ship, including pending targets
     *        - this is done automagically when client jump, dock, or warp.  we just clean up our side
     */
    //void                OnTarget(SystemEntity* tSE, uint8 mode=TargMgr::Mode::None, uint8 msg=TargMgr::Msg::NoMsg);
    //void                ClearTargets(uint8 msg=TargMgr::Msg::NoMsg);
    // notify targeters this entity is gone
    //void                ClearFromTargets(bool update=true, uint8 msg=TargMgr::Msg::NoMsg);


    bool                StartTargeting(SystemEntity* tSE, ShipItemRef sRef);

    /* NPC AI Methods */
    bool                IsTargetedBy(SystemEntity *pSE);
    SystemEntity*       GetFirstTarget(bool need_locked=false);
    SystemEntity*       GetTarget(uint32 targetID, bool need_locked=true) const;

    bool                StartTargeting(SystemEntity* who, float lockTime, uint8 maxLockedTargets, double maxTargetLockRange, bool& chase);

    bool                CanAttack()                     { return m_canAttack; }
    bool                HasNoTargets() const            { return m_targets.empty(); }

    /* PC Module Methods (for module deactivation on target removed) */
    void                Destroyed();    // this does NOT remove target from targeters map
    void                AddTargetModule(ActiveModule* pMod);
    void                RemoveTargetModule(ActiveModule* pMod);
    // only called by MiningLaser
    void                Depleted(MiningLaser* pMod);
    // only called by non-lasers
    void                Depleted(InventoryItemRef iRef);

    /* Packet builders: */
    PyList*             GetTargets() const;
    PyList*             GetTargeters() const;

    void                QueueEvent(PyTuple **up) const;    //queue an event to all SEs targeting me.
    void                QueueUpdate(PyTuple **up) const;   //queue an update to all SEs targeting me.

    // querying
    bool                IsTargeting(SystemEntity* tSE);

    /* debugging methods */
    void                Dump() const;
    // called by .targlist (player command)
    std::string         TargetList(uint16 &length, uint16 &count);
    // for querying targ count outside targMgr
    uint8               GetTargetCount()                { return m_targets.size(); }

    // get full target list for advanced AI
    void                GetAllTargets(std::map<SystemEntity*, TargetEntry*> &targets) { targets = m_targets; }
    // get full targeter list for advanced AI
    void                GetAllTargeters(std::map<SystemEntity*, TargetedByEntry*> &targetby) { targetby = m_targetedBy; }


protected:
    float               TimeToLock(ShipItemRef sRef, SystemEntity* tSE) const;

    //  add this object to our target list and informing target of yellowbox
    void                TargetedAdd(SystemEntity *tSE); // this is initial targeting call.
    //called from other targMgr once locking has been completed on this object
    void                TargetedByLocked(SystemEntity *tSE);

    void                TargetedLost(SystemEntity *tSE);
    void                TargetedByLost(SystemEntity *tSE);

private:
    SystemEntity* mySE;    //we do not own this.

    std::map<uint32, ActiveModule*> m_modules;               // map of modID/Mod* targeting this object
    std::map<SystemEntity*, TargetEntry*> m_targets;         //we own these values, not the keys
    std::map<SystemEntity*, TargetedByEntry*> m_targetedBy;  //we own these values, not the keys

    bool m_canAttack;   // true if npcs can begin attack (to correct attacking before targetlock)
};

#endif



/*
u'TargetNoLongerPresentGenericBody'}(u'{[item]moduleID.name} deactivates as the item it was targeted at is no longer present.', None, {u'{[item]moduleID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleID'}})
258753, 'label': u'TargetCantValidateBody'}(u'Targeting attempt failed as the designated object is no longer present.', None, None)
258754, 'label': u'TargetNotShipBody'}(u'The target is not a ship.', None, None)
258755, 'label': u'TargetingSystemsNotInstalledBody'}(u'The ship you are piloting does not have targeting systems installed.', None, None)
258794, 'label': u'TargetElectronicsScanStrengthsIncompatibleBody'}(u'Unable to modify the scan strengths of the target because one or more is higher than your own preventing the process.', None, None)
258795, 'label': u'TargetLostBody'}(u'Target is lost', None, None)
258850, 'label': u'TargetJammingFullSuccessBody'}(u'Your {[item]moduleID.name} managed to jam all of the objects targeting you: {jammedNames}.', None, {u'{[item]moduleID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleID'}, u'{jammedNames}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'jammedNames'}})
258853, 'label': u'TargetJammingWithoutTargetersBody'}(u'As nothing is targeting you at this time, your {[item]moduleID.name} failed to jam anything.', None, {u'{[item]moduleID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleID'}})
258854, 'label': u'TargetLostNotWithinRangeBody'}(u'The {[item]targetGroupID.name} is too far away, you need to be within {[numeric]desiredRange} meters of it but are actually {[numeric]actualRange} meters away.', None, {u'{[numeric]desiredRange}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'desiredRange'}, u'{[numeric]actualRange}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'actualRange'}, u'{[item]targetGroupID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'targetGroupID'}})
258855, 'label': u'TargetNoLongerPresentBody'}(u'{[item]moduleID.name} deactivates as the {[item]targetID.name} it was targeted at is no longer present.', None, {u'{[item]moduleID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleID'}, u'{[item]targetID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'targetID'}})
258858, 'label': u'TargetNotLockedBody'}(u'{[item]moduleID.name} deactivates because its target, {[item]targetID}, is not locked.', None, {u'{[item]targetID}': {'conditionalValues': [], 'variableType': 2, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetID'}, u'{[item]moduleID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleID'}})
258859, 'label': u'TargetObscuredBody'}(u'No line of sight to {name}.', None, {u'{name}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'name'}})
258860, 'label': u'TargetOutOfRangeFarBody'}(u'{targetname} is too far away to target.', None, {u'{targetname}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetname'}})
258864, 'label': u'TargetingAptitudeAlreadyFullyUtilizedBody'}(u'You are already managing {[numeric]limit, decimalPlaces=0} targets, as many as you have skill to.', None, {u'{[numeric]limit, decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'limit'}})
258865, 'label': u'TargetingMissileToSelfBody'}(u'It is not a good idea to shoot a missile at yourself. Code {shipID} {targetID}.', None, {u'{shipID}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'shipID'}, u'{targetID}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetID'}})
258868, 'label': u'TargetingSystemsAlreadyFullyUtilizedBody'}(u"You are already managing {[numeric]limit, decimalPlaces=0} targets, as many as your ship's electronics are capable of.", None, {u'{[numeric]limit, decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'limit'}})
258883, 'label': u'TargetJammedByBody'}(u'{jammerShipName} has started trying to target jam you.', None, {u'{jammerShipName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'jammerShipName'}})
258884, 'label': u'TargetJammedSuccessBody'}(u'You have started trying to target jam {jammedShipName}.', None, {u'{jammedShipName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'jammedShipName'}})
258885, 'label': u'TargetJammedOtherByBody'}(u'{jammerShipName} has started trying to target jam {jammedShipName}.', None, {u'{jammedShipName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'jammedShipName'}, u'{jammerShipName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'jammerShipName'}})
258886, 'label': u'TargetingSystemsBeingJammedBody'}(u'Unable to target {targetname} as your targeting systems are currently being jammed.', None, {u'{targetname}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetname'}})
258896, 'label': u'TargetNotWithinRangeBody'}(u'The {targetGroupName} is too far away, you need to be within {[numeric]desiredRange, decimalPlaces=0} meters of it but are actually {[numeric]actualDistance, decimalPlaces=0} meters away.', None, {u'{targetGroupName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetGroupName'}, u'{[numeric]desiredRange, decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'desiredRange'}, u'{[numeric]actualDistance, decimalPlaces=0}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 0}, 'variableName': 'actualDistance'}})
258942, 'label': u'TargetingBody'}(u'{name} targeted', None, {u'{name}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'name'}})
264194, 'label': u'TargetExploding2Body'}(u'{[item]moduleTypeID.name} deactivates as {targetName} begins to explode.', None, {u'{[item]moduleTypeID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleTypeID'}, u'{targetName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetName'}})
264195, 'label': u'TargetNotLocked2Body'}(u'{[item]moduleID.name} deactivates because its target, {targetName}, is not locked.', None, {u'{[item]moduleID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleID'}, u'{targetName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetName'}})
258842, 'label': u'TargetExplodingBody'}(u'{[item]moduleTypeID.name} deactivates as {[item]targetID.name} begins to explode.', None, {u'{[item]targetID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'targetID'}, u'{[item]moduleTypeID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleTypeID'}})

*/

/* 259494, 'label': u'DeniedInvulnerableBody'}(u'Your ship is realigning its magnetic field, please wait a moment.', None, None)
 * 259495, 'label': u'DeniedTargetAfterCloakBody'}(u'You cannot perform that action at this time as your systems are still recalibrating after the use of a cloaking device.', None, None)
 * 259496, 'label': u'DeniedTargetInvulnerableBody'}(u'Target is invulnerable.', None, None)
 * 259497, 'label': u'DeniedTargetSelfBody'}(u'You cannot target your own ship.', None, None)
 * 259498, 'label': u'DeniedTargetingTargetCloakedBody'}(u'You failed to target nothing.', None, None)
 * 259600, 'label': u'DeniedStationInvulnerableSovereignBody'}(u'{[item]module.name} deactivates as the target {targetname} is invulnerable because its owning alliance holds sovereignty in this solar system.', None, {u'{targetname}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetname'}, u'{[item]module.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'module'}})
 * 258348, 'label': u'DeniedTargetUntargetableBody'}(u'You are unable to target {targetName} as it has been made untargetable by a GM.', None, {u'{targetName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetName'}})
 * 259583, 'label': u'DeniedTargetingCloakedBody'}(u'You cannot target anything while you are cloaked.', None, None)
 * 259635, 'label': u'DeniedTargetOtherWarpingBody'}(u"Interference from {targetName}'s warp prevents your sensors from locking the target.", None, {u'{targetName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetName'}})
 * 259657, 'label': u'DeniedTargetEvadesSensorsBody'}(u'You are unable to target the {targetName} as your sensors are unable to lock onto it.', None, {u'{targetName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetName'}})
 * 259658, 'label': u'DeniedTargetSelfFrozenBody'}(u'You are unable to target {targetName} because you have been frozen by a GM.', None, {u'{targetName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetName'}})
 * 259659, 'label': u'DeniedTargetOtherFrozenBody'}(u'You are unable to target {targetName} because they are currently frozen by a GM.', None, {u'{targetName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetName'}})
 * 259660, 'label': u'DeniedTargetSelfWarpingBody'}(u'Interference from the warp you are doing is preventing your sensors from getting a target lock on {targetName}.', None, {u'{targetName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'targetName'}})
 * 259661, 'label': u'DeniedTargetForceFieldBody'}(u'You failed to target {[item]target.name}, they are within range {[numeric]range.distance} of a {[item]item.name} and with you being outside of it, it is preventing you from holding a lock on them.', None, {u'{[item]target.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'target'}, u'{[item]item.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'item'}, u'{[numeric]range.distance}': {'conditionalValues': [], 'variableType': 9, 'propertyName': 'distance', 'args': 256, 'kwargs': {}, 'variableName': 'range'}})
 * 259662, 'label': u'DeniedTargetingInsideFieldBody'}(u'You cannot target the {[item]target.name} while you are inside a force field.', None, {u'{[item]target.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'target'}})
 * 259663, 'label': u'DeniedTargetReinforcedStructureBody'}(u'You failed to target {[item]target.name} as it is locked down in reinforced mode.', None, {u'{[item]target.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'target'}})
 * 259683, 'label': u'DeniedTargetingAttemptFailedBody'}(u'Your attempt to target {[item]target.name} failed.', None, {u'{[item]target.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'target'}})
 *
 */
