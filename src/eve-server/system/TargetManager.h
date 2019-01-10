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
//#include "ship/modules/ActiveModule.h"

class ActiveModule;
class SystemEntity;
class PyRep;
class PyTuple;
class PyList;

class TargetManager {
public:
    TargetManager(SystemEntity* self);
    ~TargetManager()                            { /* do nothing here */ }

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

    /* PC Module Methods (for module deactivation on target removed) */
    void                Destroyed();
    void            AddTargetModule(ActiveModule* pMod);
    void         RemoveTargetModule(ActiveModule* pMod);

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

    std::map<uint32, ActiveModule*> m_modules;  // map of modID/Mod* targeting this object
    std::map<SystemEntity*, TargetEntry*> m_targets;    //we own these values, not the keys
    std::map<SystemEntity*, TargetedByEntry*> m_targetedBy;    //we own these values, not the keys
};

#endif

/*{'messageKey': 'TargetByOther', 'dataID': 2040886, 'suppressable': False, 'bodyID': None, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1612}
{'messageKey': 'TargetCantValidate', 'dataID': 17881380, 'suppressable': False, 'bodyID': 258753, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1613}
{'messageKey': 'TargetElectronicsScanStrengthsIncompatible', 'dataID': 17881494, 'suppressable': False, 'bodyID': 258794, 'messageType': 'notify', 'urlAudio': 'wise:/msg_TargetElectronicsScanStrengthsIncompatible_play', 'urlIcon': '', 'titleID': None, 'messageID': 1614}
{'messageKey': 'TargetExploding', 'dataID': 17881630, 'suppressable': False, 'bodyID': 258842, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1615}
{'messageKey': 'TargetExploding2', 'dataID': 38244390, 'suppressable': False, 'bodyID': 264194, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3798}
{'messageKey': 'TargetJammedBy', 'dataID': 17881749, 'suppressable': False, 'bodyID': 258883, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1616}
{'messageKey': 'TargetJammedOtherBy', 'dataID': 17881755, 'suppressable': False, 'bodyID': 258885, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1617}
{'messageKey': 'TargetJammedSuccess', 'dataID': 17881752, 'suppressable': False, 'bodyID': 258884, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1618}
{'messageKey': 'TargetJammingFullSuccess', 'dataID': 17881651, 'suppressable': False, 'bodyID': 258850, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1619}
{'messageKey': 'TargetJammingPartialSuccess', 'dataID': 17881657, 'suppressable': False, 'bodyID': 258852, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1620}
{'messageKey': 'TargetJammingWithoutTargeters', 'dataID': 17881660, 'suppressable': False, 'bodyID': 258853, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1621}
u'TargetJammingWithoutTargetersBody'}(u'As nothing is targeting you at this time, your {[item]moduleID.name} failed to jam anything.', None, {u'{[item]moduleID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleID'}})
{'messageKey': 'TargetLocked', 'dataID': 2987151, 'suppressable': False, 'bodyID': None, 'messageType': 'audio', 'urlAudio': 'wise:/msg_TargetLocked_play', 'urlIcon': '', 'titleID': None, 'messageID': 1622}
{'messageKey': 'TargetLocking', 'dataID': 2987152, 'suppressable': False, 'bodyID': None, 'messageType': 'audio', 'urlAudio': 'wise:/msg_TargetLocking_play', 'urlIcon': '', 'titleID': None, 'messageID': 1623}
{'messageKey': 'TargetLost', 'dataID': 17881497, 'suppressable': False, 'bodyID': 258795, 'messageType': 'notify', 'urlAudio': 'wise:/msg_TargetLost_play', 'urlIcon': '', 'titleID': None, 'messageID': 1624}
{'messageKey': 'TargetLostNotWithinRange', 'dataID': 17881663, 'suppressable': False, 'bodyID': 258854, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1625}
u'TargetLostNotWithinRangeBody'}(u'The {[item]targetGroupID.name} is too far away, you need to be within {[numeric]desiredRange} meters of it but are actually {[numeric]actualRange} meters away.', None, {u'{[numeric]desiredRange}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'desiredRange'}, u'{[numeric]actualRange}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'actualRange'}, u'{[item]targetGroupID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'targetGroupID'}})
{'messageKey': 'TargetNoLongerPresent', 'dataID': 17881666, 'suppressable': False, 'bodyID': 258855, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1626}
u'TargetNoLongerPresentBody'}(u'{[item]moduleID.name} deactivates as the {[item]targetID.name} it was targeted at is no longer present.', None, {u'{[item]moduleID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleID'}, u'{[item]targetID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'targetID'}})
{'messageKey': 'TargetNoLongerPresentGeneric', 'dataID': 17875297, 'suppressable': False, 'bodyID': 256459, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3742}
u'TargetNoLongerPresentGenericBody'}(u'{[item]moduleID.name} deactivates as the item it was targeted at is no longer present.', None, {u'{[item]moduleID.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleID'}})
{'messageKey': 'TargetNotLocked', 'dataID': 17881675, 'suppressable': False, 'bodyID': 258858, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1627}
{'messageKey': 'TargetNotLocked2', 'dataID': 38244393, 'suppressable': False, 'bodyID': 264195, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3799}
{'messageKey': 'TargetNotShip', 'dataID': 17881383, 'suppressable': False, 'bodyID': 258754, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1628}
{'messageKey': 'TargetNotWithinRange', 'dataID': 17881787, 'suppressable': False, 'bodyID': 258896, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1629}
{'messageKey': 'TargetNotWithinRangeGeneric', 'dataID': 17875391, 'suppressable': False, 'bodyID': 256493, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3744}
{'messageKey': 'TargetObscured', 'dataID': 17881678, 'suppressable': False, 'bodyID': 258859, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1630}
{'messageKey': 'TargetOutOfRangeFar', 'dataID': 17881681, 'suppressable': False, 'bodyID': 258860, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1631}
{'messageKey': 'TargetTooFar', 'dataID': 17881689, 'suppressable': False, 'bodyID': 258863, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1632}
{'messageKey': 'Targeting', 'dataID': 17881919, 'suppressable': False, 'bodyID': 258942, 'messageType': 'notify', 'urlAudio': 'wise:/msg_Targeting_play', 'urlIcon': 'res:/ui/icon/misc/targeting.blue', 'titleID': None, 'messageID': 1633}
{'messageKey': 'TargetingAptitudeAlreadyFullyUtilized', 'dataID': 17881692, 'suppressable': False, 'bodyID': 258864, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1634}
{'messageKey': 'TargetingMissileToSelf', 'dataID': 17881695, 'suppressable': False, 'bodyID': 258865, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1635}
{'messageKey': 'TargetingSystemsAlreadyFullyUtilized', 'dataID': 17881704, 'suppressable': False, 'bodyID': 258868, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1636}
{'messageKey': 'TargetingSystemsBeingJammed', 'dataID': 17881758, 'suppressable': False, 'bodyID': 258886, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1637}
{'messageKey': 'TargetingSystemsNotInstalled', 'dataID': 17881386, 'suppressable': False, 'bodyID': 258755, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1638}
*/

