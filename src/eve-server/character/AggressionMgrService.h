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
    Author:        caytchen
*/

#ifndef __AGGRESSIONMGRSERVICE__H__INCL__
#define __AGGRESSIONMGRSERVICE__H__INCL__

#include "PyService.h"

class AggressionMgrService : public PyService
{
public:
    AggressionMgrService(PyServiceMgr *mgr);
    virtual ~AggressionMgrService();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    //overloaded in order to support bound objects
    virtual PyBoundObject* CreateBoundObject(Client *pClient, const PyRep *bind_args);
};

#endif // __AGGRESSIONMGRSERVICE__H__INCL__

//TODO;  this should be changed to system...doesnt really fit in character

/*
 *    def OnAggressionChanges(self, solarsystemID, messageList):
 *        self.LogInfo('Received OnAggressionChanges for', len(messageList), 'aggressors')
 *        for m in messageList:
 *            self.OnAggressionChange(solarsystemID, m)
 *
 *    def OnAggressionChange(self, solarsystemID, aggressors):
 *        if eve.session.solarsystemid2 == solarsystemID:
 *            self.LogInfo('Received aggression state change dict: ', aggressors)
 *            self.aggressors.update(aggressors)
 *            sm.ScatterEvent('OnAggressionChanged', solarsystemID, self.aggressors)
 *            for aggressorID, aggressor in aggressors.iteritems():
 *                for aggresseeID, lastAggression in aggressor.iteritems():
 *                    lastAggression = int(lastAggression / SEC) * SEC
 *                    when = lastAggression + const.aggressionTime * MIN
 *                    self.clearAggressions[aggressorID, aggresseeID, solarsystemID] = when
 *
 *    def __ClearAggressionThread(self):
 *        while self.state == SERVICE_RUNNING:
 *            try:
 *                clrList = []
 *                broadcast = False
 *                for k, when in self.clearAggressions.iteritems():
 *                    aggressorID, aggresseeID, solarSystemID = k
 *                    if when < blue.os.GetSimTime() - 2 * SEC:
 *                        aggressionState = self.GetAggressionState(aggressorID)
 *                        if aggressionState != 2 and aggresseeID not in (session.charid, session.corpid):
 *                            clrList.append(k)
 *                            if solarSystemID == session.solarsystemid:
 *                                broadcast = True
 *
 *                for k in clrList:
 *                    del self.clearAggressions[k]
 *
 *                if len(clrList) > 0:
 *                    self.LogInfo('__ClearAggressionThread is clearing out', len(clrList), 'aggressions, broadcast =', broadcast)
 *                if broadcast:
 *                    sm.ScatterEvent('OnAggressionChanged', solarSystemID, self.aggressors)
 *            except:
 *                log.LogException()
 *                sys.exc_clear()
 *
 *            blue.pyos.synchro.SleepWallclock(5000)
 *
 *    def GetAggressionState(self, aggressorID):
 *        aggressor = self.aggressors.get(aggressorID, {})
 *        if not aggressor:
 *            return 0
 *        lastGlobalAggression = aggressor.get(None, 0)
 *        if lastGlobalAggression and blue.os.GetSimTime() - lastGlobalAggression < const.aggressionTime * MIN:
 *            return 2
 *        lastAggression = max(aggressor.get(eve.session.charid, 0), aggressor.get(eve.session.corpid, 0))
 *        if lastAggression and blue.os.GetSimTime() - lastAggression < const.aggressionTime * MIN:
 *            return 1
 *        return 0
 *
 *    def GetCriminalFlagCountDown(self):
 *        corpaggressions = {}
 *        charaggressions = {}
 *        for aggressions, id in ((charaggressions, eve.session.charid), (corpaggressions, eve.session.corpid)):
 *            for victimID, timestamp in self.aggressors.get(id, {}).iteritems():
 *                t = timestamp + const.aggressionTime * MIN
 *                if t > blue.os.GetSimTime():
 *                    aggressions[victimID] = t
 *
 *        return (charaggressions, corpaggressions)
 *
 *    def RefreshCriminalFlagCountDown(self):
 *        charID = eve.session.charid
 *        systemID = eve.session.solarsystemid2
 *        aggressionMgr = util.Moniker('aggressionMgr', systemID)
 *        crimes = {charID: aggressionMgr.GetCriminalTimeStamps(charID)}
 *        self.OnAggressionChange(systemID, crimes)
 *
 *    def IsCriminalFlaggedTo(self, recipientID, actorID):
 *        if self.aggressors.has_key(recipientID) and self.aggressors[recipientID].has_key(actorID):
 *            return self.aggressors[recipientID][actorID] + 15 * MIN > blue.os.GetSimTime()
 *        return False
 *
 *    def GetPlayerAggressionsForOwner(self, actorID):
 *        ret = set()
 *        for victim, t in self.aggressors.get(actorID, {}).iteritems():
 *            if not util.IsSystemOrNPC(victim) and t + const.aggressionTime * MIN > blue.os.GetSimTime():
 *                ret.add(victim)
 *
 *        return ret
 */