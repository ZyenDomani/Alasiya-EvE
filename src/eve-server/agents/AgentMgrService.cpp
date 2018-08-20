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

/*
 * AgentMgr bound(agentID):
 *  -> DoAction(actionID or None)
 *  -> WarpToLocation(locationType, locationNumber, warpRange, is_gang)
 *
 *   Also sent an OnRemoteMessage(AgtMissionOfferWarning)
 *
 *   and various OnAgentMissionChange()
 *
*/

/*
 * # Agent Logging:
 * AGENT__ERROR
 * AGENT__WARNING
 * AGENT__MESSAGE
 * AGENT__DEBUG
 * AGENT__INFO
 * AGENT__TRACE
 * AGENT__DUMP
 * AGENT__RSPDUMP
 */

#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "StaticDataMgr.h"
#include "cache/ObjCacheService.h"
#include "agents/Agent.h"
#include "agents/AgentBound.h"
#include "agents/AgentMgrService.h"

PyCallable_Make_InnerDispatcher(AgentMgrService)

AgentMgrService::AgentMgrService(PyServiceMgr *mgr)
: PyService(mgr, "agentMgr"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(AgentMgrService, GetAgents);
    PyCallable_REG_CALL(AgentMgrService, GetCareerAgents);
    PyCallable_REG_CALL(AgentMgrService, GetMyJournalDetails);
    PyCallable_REG_CALL(AgentMgrService, GetSolarSystemOfAgent);
    PyCallable_REG_CALL(AgentMgrService, GetMyEpicJournalDetails);
}

AgentMgrService::~AgentMgrService() {
    delete m_dispatch;
    std::map<uint32, Agent *>::iterator cur = m_agents.begin();
    for(; cur != m_agents.end(); cur++) {
        delete cur->second;
    }
}

Agent *AgentMgrService::_GetAgent(uint32 agentID) {
    std::map<uint32, Agent *>::iterator res;
    res = m_agents.find(agentID);
    if(res != m_agents.end())
        return(res->second);
    Agent *a = new Agent(agentID);
    if(!a->Load(&m_db)) {
        delete a;
        return nullptr;
    }
    m_agents[agentID] = a;
    return(a);
}

// need a way to check created objects for client/agent combinations to avoid duplicates.
//  also need a way to check/delete released objects/agents
PyBoundObject *AgentMgrService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
    if(!bind_args->IsInt()) {
        codelog(CLIENT__ERROR, "%s: Non-integer bind argument '%s'", c->GetName(), bind_args->TypeString());
        return nullptr;
    }

    uint32 agentID = bind_args->AsInt()->value();

    Agent *agent = _GetAgent(agentID);
    if(agent == NULL) {
        codelog(CLIENT__ERROR, "%s: Unable to obtain agent %u", c->GetName(), agentID);
        return nullptr;
    }

    return(new AgentBound(m_manager, agent));
}

PyResult AgentMgrService::Handle_GetAgents(PyCallArgs &call) {
    // this is cached on client side...
    return sDataMgr.GetAgents();
}

PyResult AgentMgrService::Handle_GetSolarSystemOfAgent(PyCallArgs &call)
{
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: failed to decode arguments", call.client->GetName());
        return nullptr;
    }

    return sDataMgr.GetAgentSystemID(args.arg);
}


/** not handled */
PyResult AgentMgrService::Handle_GetMyJournalDetails(PyCallArgs &call) {

    /** @todo  journal details
     * found in eve/client/script/ui/shared/neocom/journal.py
     *
     *  missions = self.GetMyAgentJournalDetails()[0]
     *    missionState, importantMission, missionType, missionName, agentID, expirationTime, bookmarks, remoteOfferable, remoteCompletable = missions[i]
     *
     *    research = sm.GetService('journal').GetMyAgentJournalDetails()[1]
     *    agentID, typeID, ppd, points, level, quality, stationID = research[i]
     *
     *            self.agentjournal = sm.RemoteSvc('agentMgr').GetMyJournalDetails()
     *            for mission in self.agentjournal[0]:
     *                if mission[4] == agentID:
     *            for research in self.agentjournal[1]:
     *                if research[0] == agentID:
     */

  /*
      [PySubStream 59 bytes]
        [PyTuple 2 items]
          [PyList 1 items]
            [PyTuple 9 items]
              [PyInt 1]
              [PyInt 0]
              [PyString "Encounter"]
              [PyString "Seek and Destroy"]
              [PyInt 3010819]
              [PyIntegerVar 129495559223373466]
              [PyList 0 items]
              [PyBool False]
              [PyBool False]
          [PyList 0 items]

    [PyTuple 1 items]
      [PySubStream 53 bytes]
        [PyTuple 2 items]
          [PyList 1 items]
            [PyTuple 9 items]
              [PyInt 1]
              [PyInt 0]
              [PyString "Courier"]
              [PyString "Good Harvest"]
              [PyInt 3010819]
              [PyIntegerVar 129495553039999957]
              [PyList 0 items]
              [PyBool False]
              [PyBool False]
          [PyList 0 items]


      [PySubStream 513 bytes]
        [PyTuple 2 items]
          [PyList 1 items]
            [PyTuple 9 items]
              [PyInt 2]
              [PyInt 0]
              [PyString "Courier"]
              [PyString "Good Harvest"]
              [PyInt 3010819]
              [PyIntegerVar 129495553802043094]
              [PyList 2 items]
                [PyObjectData Name: util.KeyVal]
                  [PyDict 15 kvp]
                    [PyString "itemID"]
                    [PyInt 60014683]
                    [PyString "typeID"]
                    [PyInt 1529]
                    [PyString "agentID"]
                    [PyInt 3010819]
                    [PyString "hint"]
                    [PyString "Objective (Pick Up) & Agent Base - Annaro VIII - Moon 4 - State War Academy School"]
                    [PyString "locationType"]
                    [PyString "objective.source"]
                    [PyString "memo"]
                    [PyString ""]
                    [PyString "created"]
                    [PyIntegerVar 129489505802043094]
                    [PyString "locationNumber"]
                    [PyInt 0]
                    [PyString "flag"]
                    [PyNone]
                    [PyString "locationID"]
                    [PyInt 30002776]
                    [PyString "ownerID"]
                    [PyInt 1661059544]
                    [PyString "y"]
                    [PyInt 0]
                    [PyString "x"]
                    [PyInt 0]
                    [PyString "solarsystemID"]
                    [PyInt 30002776]
                    [PyString "z"]
                    [PyInt 0]
                [PyObjectData Name: util.KeyVal]
                  [PyDict 15 kvp]
                    [PyString "itemID"]
                    [PyInt 60000016]
                    [PyString "typeID"]
                    [PyInt 1531]
                    [PyString "agentID"]
                    [PyInt 3010819]
                    [PyString "hint"]
                    [PyString "Objective (Drop Off) - Tasabeshi VIII - Moon 13 - CBD Corporation Storage"]
                    [PyString "locationType"]
                    [PyString "objective.destination"]
                    [PyString "memo"]
                    [PyString ""]
                    [PyString "created"]
                    [PyIntegerVar 129489505802043094]
                    [PyString "locationNumber"]
                    [PyInt 0]
                    [PyString "flag"]
                    [PyNone]
                    [PyString "locationID"]
                    [PyInt 30002778]
                    [PyString "solarsystemID"]
                    [PyInt 30002778]
                    [PyString "ownerID"]
                    [PyInt 1661059544]
                    [PyString "y"]
                    [PyInt 0]
                    [PyString "x"]
                    [PyInt 0]
                    [PyString "z"]
                    [PyInt 0]
              [PyBool False]
              [PyBool False]
          [PyList 0 items]
  sLog.White( "AgentMgrService::Handle_GetMyJournalDetails()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
    */

    PyTuple *tuple = new PyTuple(2);
    //missions:
    tuple->SetItem(0, new PyList());
    //research:
    tuple->SetItem(1, new PyList());

    if (is_log_enabled(AGENT__RSPDUMP))
        tuple->Dump(AGENT__RSPDUMP, "   ");
    return tuple;
}

PyResult AgentMgrService::Handle_GetMyEpicJournalDetails( PyCallArgs& call )
{
    //no args
  sLog.White( "AgentMgrBound::Handle_GetMyEpicJournalDetails()", "size= %u", call.tuple->size() );

    return new PyList();
}

PyResult AgentMgrService::Handle_GetCareerAgents(PyCallArgs &call)
{
  sLog.White( "AgentMgrBound::Handle_GetCareerAgents()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return new PyInt( 0 );
}


PyCallable_Make_InnerDispatcher(EpicArcService)

EpicArcService::EpicArcService(PyServiceMgr *mgr)
: PyService(mgr, "epicArcStatus"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(EpicArcService, AgentHasEpicMissionsForCharacter);
}

EpicArcService::~EpicArcService() {
    delete m_dispatch;
}

PyResult EpicArcService::Handle_AgentHasEpicMissionsForCharacter(PyCallArgs &call) {
  /**
     epicArcStatusSvc = sm.RemoteSvc('epicArcStatus').AgentHasEpicMissionsForCharacter(agent.agentID):
     */
    sLog.White( "EpicArcService::Handle_AgentHasEpicMissionsForCharacter()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    // return boolean
    return PyStatic.NewFalse();

}
