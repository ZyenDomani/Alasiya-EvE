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

#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "StaticDataMgr.h"
#include "cache/ObjCacheService.h"
#include "agents/Agent.h"
#include "agents/AgentMgrService.h"

class AgentMgrBound
: public PyBoundObject {
public:

    PyCallable_Make_Dispatcher(AgentMgrBound)

    AgentMgrBound(PyServiceMgr *mgr, AgentDB *db, Agent *agt)
    : PyBoundObject(mgr),
      m_db(db),
      m_dispatch(new Dispatcher(this)),
      m_agent(agt)
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "AgentMgrBound";

        PyCallable_REG_CALL(AgentMgrBound, DoAction);
        PyCallable_REG_CALL(AgentMgrBound, GetAgentLocationWrap);
        PyCallable_REG_CALL(AgentMgrBound, GetInfoServiceDetails);
        PyCallable_REG_CALL(AgentMgrBound, GetMissionBriefingInfo);
        PyCallable_REG_CALL(AgentMgrBound, GetMissionObjectiveInfo);

    }
    virtual ~AgentMgrBound() { delete m_dispatch; }
    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(DoAction);
    PyCallable_DECL_CALL(GetAgentLocationWrap);
    PyCallable_DECL_CALL(GetInfoServiceDetails);
    PyCallable_DECL_CALL(GetMissionBriefingInfo);
    PyCallable_DECL_CALL(GetMissionObjectiveInfo);

protected:
    Agent *const m_agent;    //we do not own this.
    AgentDB *const m_db;        //we do not own this
    Dispatcher *const m_dispatch;    //we own this
};

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
        return NULL;
    }
    m_agents[agentID] = a;
    return(a);
}

PyBoundObject *AgentMgrService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
    if(!bind_args->IsInt()) {
        codelog(CLIENT__ERROR, "%s: Non-integer bind argument '%s'", c->GetName(), bind_args->TypeString());
        return NULL;
    }

    uint32 agentID = bind_args->AsInt()->value();

    Agent *agent = _GetAgent(agentID);
    if(agent == NULL) {
        codelog(CLIENT__ERROR, "%s: Unable to obtain agent %u", c->GetName(), agentID);
        return NULL;
    }

    return(new AgentMgrBound(m_manager, &m_db, agent));
}

PyResult AgentMgrService::Handle_GetAgents(PyCallArgs &call) {
    return sDataMgr.GetAgents();
}

///  this really needs to be a cached object....load/save in an agent data mgr singleton?
PyResult AgentMgrService::Handle_GetSolarSystemOfAgent(PyCallArgs &call)
{
    Call_SingleIntegerArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: failed to decode arguments", call.client->GetName());
        return nullptr;
    }

    return sDataMgr.GetAgentSystemID(args.arg);
}

//15:39:45 L AgentMgrBound::Handle_DoAction(): size= 1, 0=None
PyResult AgentMgrBound::Handle_DoAction(PyCallArgs &call) {
  /*
17:00:59 L AgentMgrBound::Handle_DoAction(): size= 1, 0=None
17:00:59 [SvcCall]   Call Arguments:
17:00:59 [SvcCall]       Tuple: 1 elements
17:00:59 [SvcCall]         [ 0] (None)
17:00:59 [SvcCall]   Call Arguments:
17:00:59 [SvcCall]       Tuple: Empty
17:00:59 [SvcCall]   Call Named Arguments:
17:00:59 [SvcCall]     Argument 'machoVersion':
17:00:59 [SvcCall]         Integer field: 1
  sLog.White( "AgentMgrBound::Handle_DoAction()", "size= %u, 0=%s", call.tuple->size(), call.tuple->GetItem( 0 )->TypeString() );
    call.Dump(SERVICE__CALL_DUMP);
    */
  /**
16:53:45 [SvcMessage] agentMgr Service: MachoBindObject also contains call to DoAction
16:53:45 [SvcCallTrace]   Call Arguments:
16:53:45 [SvcCallTrace]       Tuple: 1 elements
16:53:45 [SvcCallTrace]         [ 0] (None)
16:53:45 [SvcCallTrace] Call DoAction returned:
16:53:45 [SvcCallTrace]       Tuple: 2 elements
16:53:45 [SvcCallTrace]         [ 0] Tuple: 2 elements
16:53:45 [SvcCallTrace]         [ 0]   [ 0] String: 'Result of DoAction(0)'
16:53:45 [SvcCallTrace]         [ 0]   [ 1] List: 2 elements
16:53:45 [SvcCallTrace]         [ 0]   [ 1]   [ 0] Tuple: 2 elements
16:53:45 [SvcCallTrace]         [ 0]   [ 1]   [ 0]   [ 0] Integer field: 2
16:53:45 [SvcCallTrace]         [ 0]   [ 1]   [ 0]   [ 1] String: 'I want work, do you have anything?'
16:53:45 [SvcCallTrace]         [ 0]   [ 1]   [ 1] Tuple: 2 elements
16:53:45 [SvcCallTrace]         [ 0]   [ 1]   [ 1]   [ 0] Integer field: 15
16:53:45 [SvcCallTrace]         [ 0]   [ 1]   [ 1]   [ 1] String: 'I need to find somebody.  Can you help me?'
16:53:45 [SvcCallTrace]         [ 1] Dictionary: 1 entries
16:53:45 [SvcCallTrace]         [ 1]   [ 0] Key: String: 'loyaltyPoints'
16:53:45 [SvcCallTrace]         [ 1]   [ 0] Value: Integer field: 0
*/
    //takes a single argument, which may be None, or may be an integer actionID
    Call_SingleArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: failed to decode arguments", call.client->GetName());
        return NULL;
    }

    /** @todo: send loyaltyPoints in the keywords return. */
    //uint32 loyaltyPoints = m_agent->GetLoyaltyPoints(call.client);

    DoAction_Result res;
    res.dialogue = new PyList;

    std::map<uint32, std::string> choices;
//00:20:34 E AgentMgrBound::Handle_DoAction(): args.arg->IsInt() failed.  Expected type Int, got type None
    if (args.arg->IsInt())
        m_agent->DoAction( call.client, args.arg->AsInt()->value(), res.agentSays, choices );
    else   //* EVE client calls DoAction without parameters to initiate convo
        m_agent->DoAction( call.client, 0, res.agentSays, choices );
    /* server reply to initiate convo
          [PyTuple 2 items]
            [PyTuple 2 items]
              [PyObjectEx Normal]
                [PyTuple 2 items]
                  [PyToken __builtin__.unicode]
                  [PyTuple 1 items]
                    [PyString "Welcome, Aknor Jaden."]
              [PyList 2 items]
                [PyTuple 2 items]
                  [PyInt 606]
                  [PyString "[Button]Request Mission"]
                [PyTuple 2 items]
                  [PyInt 607]
                  [PyString "[Button]Locate Character"]
            [PyDict 4 kvp]
              [PyString "missionCompleted"]
              [PyNone]
              [PyString "missionQuit"]
              [PyNone]
              [PyString "loyaltyPoints"]
              [PyInt 0]
              [PyString "missionDeclined"]
              [PyNone]
    **************************************
      [PySubStream 230 bytes]
        [PyTuple 2 items]
          [PyTuple 2 items]
            [PyObjectEx Normal]
              [PyTuple 2 items]
                [PyToken __builtin__.unicode]
                [PyTuple 1 items]
                  [PyString "You do know you haven't finished your current mission for me, right?"]
            [PyList 2 items]
              [PyTuple 2 items]
                [PyInt 592]
                [PyString "[Button]View Mission"]
              [PyTuple 2 items]
                [PyInt 593]
                [PyString "[Button]Locate Character"]
          [PyDict 4 kvp]
            [PyString "missionCompleted"]
            [PyNone]
            [PyString "missionQuit"]
            [PyNone]
            [PyString "loyaltyPoints"]
            [PyInt 0]
            [PyString "missionDeclined"]
            [PyNone]
    ***************************************
            [PyString "DoAction"]
            [PyTuple 1 items]
              [PyInt 592]
    *************************************
      [PySubStream 185 bytes]
        [PyTuple 2 items]
          [PyTuple 2 items]
            [PyObjectEx Normal]
              [PyTuple 2 items]
                [PyToken __builtin__.unicode]
                [PyTuple 1 items]
                  [PyString "Good to hear, carry on."]
            [PyList 2 items]
              [PyTuple 2 items]
                [PyInt 594]
                [PyString "[Button]Complete Mission"]
              [PyTuple 2 items]
                [PyInt 595]
                [PyString "[Button]Quit Mission"]
          [PyDict 4 kvp]
            [PyString "missionCompleted"]
            [PyBool False]
            [PyString "missionQuit"]
            [PyBool False]
            [PyString "loyaltyPoints"]
            [PyInt 0]
            [PyString "missionDeclined"]
            [PyBool False]
              */

    DoAction_Dialogue_Item choice;

    std::map<uint32, std::string>::iterator cur = choices.begin();
    for (; cur != choices.end(); cur++)
    {
        choice.actionID = cur->first;
        choice.actionText = cur->second;

        res.dialogue->AddItem( choice.Encode() );
    }

    return res.Encode();
}
/*
    [PyTuple 1 items]
      [PyTuple 2 items]
        [PyInt 1]
        [PySubStream 41 bytes]
          [PyTuple 4 items]
            [PyString "N=696805:4035"]
            [PyString "DoAction"]
            [PyTuple 1 items]
              [PyInt 594]
            [PyDict 1 kvp]
              [PyString "machoVersion"]
              [PyInt 1]

    [PyTuple 1 items]
      [PySubStream 206 bytes]
        [PyTuple 2 items]
          [PyTuple 2 items]
            [PyObjectEx Normal]
              [PyTuple 2 items]
                [PyToken __builtin__.unicode]
                [PyTuple 1 items]
                  [PyString "I knew I could count on you, Aknor Jaden."]
            [PyList 2 items]
              [PyTuple 2 items]
                [PyInt 596]
                [PyString "[Button]Request Mission"]
              [PyTuple 2 items]
                [PyInt 597]
                [PyString "[Button]Locate Character"]
          [PyDict 4 kvp]
            [PyString "missionCompleted"]
            [PyBool True]
            [PyString "missionQuit"]
            [PyBool False]
            [PyString "loyaltyPoints"]
            [PyInt 0]
            [PyString "missionDeclined"]
            [PyBool False]

    [PyTuple 1 items]
      [PyTuple 2 items]
        [PyInt 1]
        [PySubStream 41 bytes]
          [PyTuple 4 items]
            [PyString "N=696805:4035"]
            [PyString "DoAction"]
            [PyTuple 1 items]
              [PyInt 596]
            [PyDict 1 kvp]
              [PyString "machoVersion"]
              [PyInt 1]

      [PySubStream 713 bytes]
        [PyTuple 2 items]
          [PyTuple 2 items]
            [PyObjectEx Normal]
              [PyTuple 2 items]
                [PyToken __builtin__.unicode]
                [PyTuple 1 items]
                  [PyString "Something just came up that's right up your alley.  Bio-engineers and farmers in nearby settlements, which have been supplying us with foodstuffs, have had good fortune recently.  Their supplies of frozen plant seeds are overflowing, and they would like to sell some of their excess products.  Obviously we took advantage of a good opportunity and bought the goods for silly prices, those people are so gullible.  I'd like you to deliver the goods to Tasabeshi VIII - Moon 13 - CBD Corporation Storage, where we've found a buyer."]
            [PyList 3 items]
              [PyTuple 2 items]
                [PyInt 598]
                [PyString "[Button]Accept"]
              [PyTuple 2 items]
                [PyInt 599]
                [PyString "[Button]Decline"]
              [PyTuple 2 items]
                [PyInt 600]
                [PyString "[Button][CloseOnClick]Delay"]
          [PyDict 4 kvp]
            [PyString "missionCompleted"]
            [PyBool False]
            [PyString "missionQuit"]
            [PyBool False]
            [PyString "loyaltyPoints"]
            [PyInt 0]
            [PyString "missionDeclined"]
            [PyBool False]


            [PyString "DoAction"]
            [PyTuple 1 items]
              [PyInt 598]
    [PyTuple 1 items]
      [PySubStream 185 bytes]
        [PyTuple 2 items]
          [PyTuple 2 items]
            [PyObjectEx Normal]
              [PyTuple 2 items]
                [PyToken __builtin__.unicode]
                [PyTuple 1 items]
                  [PyString "Good to hear, carry on."]
            [PyList 2 items]
              [PyTuple 2 items]
                [PyInt 601]
                [PyString "[Button]Complete Mission"]
              [PyTuple 2 items]
                [PyInt 602]
                [PyString "[Button]Quit Mission"]
          [PyDict 4 kvp]
            [PyString "missionCompleted"]
            [PyBool False]
            [PyString "missionQuit"]
            [PyBool False]
            [PyString "loyaltyPoints"]
            [PyInt 0]
            [PyString "missionDeclined"]
            [PyBool False]
            */
//21:13:12 L AgentMgrBound::Handle_GetAgentLocationWrap(): size= 0
PyResult AgentMgrBound::Handle_GetAgentLocationWrap(PyCallArgs &call)
{
    /*
     */
    sLog.White( "AgentMgrBound::Handle_GetAgentLocationWrap()", "size= %u, AgentID = %u", call.tuple->size(), m_agent->GetID() );
    call.Dump(SERVICE__CALL_DUMP);

    return m_agent->GetLocation();
}

//21:13:12 L AgentMgrBound::Handle_GetMissionBriefingInfo(): size= 0
PyResult AgentMgrBound::Handle_GetMissionBriefingInfo(PyCallArgs &call) {
  /**
    [PyTuple 1 items]
      [PySubStream 845 bytes]
        [PyDict 6 kvp]
          [PyString "ContentID"]
          [PyNone]
          [PyString "Expiration Message"]
          [PyString "
<span id=subheader>Mission Expiration</span><br>
<div id=basetext>This mission expires at 2011.05.11 02:43:00</div>
"]
          [PyString "Decline Warning"]
          [PyString ""]
          [PyString "Mission Image"]
          [PyString "<img src="res:/UI/netres/mission_content/couriermission.png" align=center hspace=4 vspace=4>"]
          [PyString "Mission Title"]
          [PyString "Good Harvest"]
          [PyString "Mission Briefing"]
          [PyString "
<div id=basetext>Bio-engineers and farmers in nearby settlements, which have been supplying us with foodstuffs, have had good fortune recently.  Their supplies of frozen plant seeds are overflowing, and they would like to sell some of their excess products.  Obviously we took advantage of a good opportunity and bought the goods for silly prices, those people are so gullible.  I'd like you to deliver the goods to Tasabeshi VIII - Moon 13 - CBD Corporation Storage, where we've found a buyer.</div>
<br>
"]
*/
  sLog.White( "AgentMgrBound::Handle_GetMissionBriefingInfo()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
    PyDict *res = new PyDict();

    res->SetItemString("ContentID", new PyInt(123) ) ;
    res->SetItemString("Mission Keywords", new PyString("Mission Keywords"));
    res->SetItemString("Mission Title ID", new PyString("Mission Title ID") );
    res->SetItemString("Mission Briefing ID", new PyString("Mission Briefing ID") );
    res->SetItemString("Decline Time", new PyLong( GetFileTimeNow() + Win32Time_Hour ) );
    res->SetItemString("Expiration Time", new PyLong( GetFileTimeNow()+Win32Time_Day ) );
    res->SetItemString("Mission Image", new PyString("MissionImage") );

    return res;
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
                    [PyString "ownerID"]
                    [PyInt 1661059544]
                    [PyString "y"]
                    [PyInt 0]
                    [PyString "x"]
                    [PyInt 0]
                    [PyString "solarsystemID"]
                    [PyInt 30002778]
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
    tuple->items[0] = new PyList();
    //research:
    tuple->items[1] = new PyList();

    if (is_log_enabled(AGENT__DUMP))
        tuple->Dump(AGENT__DUMP, "   ");
    return tuple;
}

PyResult AgentMgrService::Handle_GetMyEpicJournalDetails( PyCallArgs& call )
{
    //no args
  sLog.White( "AgentMgrBound::Handle_GetMyEpicJournalDetails()", "size= %u", call.tuple->size() );

    return new PyList;
}

PyResult AgentMgrService::Handle_GetCareerAgents(PyCallArgs &call)
{
  sLog.White( "AgentMgrBound::Handle_GetCareerAgents()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return new PyInt( 0 );
}

//17:09:07 L AgentMgrBound::Handle_GetInfoServiceDetails(): size= 0
PyResult AgentMgrBound::Handle_GetInfoServiceDetails( PyCallArgs& call ) {
  sLog.White( "AgentMgrBound::Handle_GetInfoServiceDetails()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
    //takes no arguments
    return new PyNone();
}

//15:46:37 L AgentMgrBound::Handle_GetMissionObjectiveInfo(): size= 0
PyResult AgentMgrBound::Handle_GetMissionObjectiveInfo(PyCallArgs &call)
{/*     called when clicking on line item (actionText)
      [PySubStream 2792 bytes]
        [PyObjectData Name: util.KeyVal]
          [PyDict 2 kvp]
            [PyString "html"]
            [PyString "
<html>
<head>
<LINK REL="stylesheet" TYPE="text/css" HREF="res:/ui/css/missionobjectives.css">
</head>
<body>


<span id=subheader><font>Good Harvest Objectives</font></span><br>
<div id=basetext>The following objectives must be completed to finish the mission:<br>
<br>
<span id=basetext>

                    <span id=caption>Transport Objective</span><br>
                    <div id=basetext>Transport these goods:<br>
                    <TABLE>
                    <TR VALIGN=middle>
                        <TD><img src=icon:38_193 size=16></TD>
                        <TD width=32><a href=showinfo:2//1000167><img src="corplogo:1000167" width=32 height=32 hspace=2 vspace=2></a></TD>
                        <TD>Pickup Location</TD>
                        <TD> <font color=#00FF7F>0.8</font> <a href=showinfo:1529//60014683>Annaro VIII - Moon 4 - State War Academy School</a></TD>
                    </TR>
                    <TR VALIGN=middle>
                        <TD><img src=icon:38_195 size=16></TD>
                        <TD width=32><a href=showinfo:2//1000002><img src="corplogo:1000002" width=32 height=32 hspace=2 vspace=2></a></TD>
                        <TD>Drop-off Location</TD>
                        <TD> <font color=#00FF7F>0.8</font> <a href=showinfo:1531//60000016>Tasabeshi VIII - Moon 13 - CBD Corporation Storage</a></TD>
                    </TR>
                    <TR VALIGN=middle>
                        <TD><img src=icon:38_195 size=16></TD>
                        <TD width=32><a href=showinfo:26785><img src="typeicon:26785" width=32 height=32 align=left></a></TD>
                        <TD>Cargo</TD>
                        <TD>seven units of Crates of Frozen Plant Seeds (350.0 m&sup3;)</TD>
                    </TR>
                    </TABLE></div>

</span><br>
LOWSECREPLACE <br><br>
<span id=subheader>Rewards</span>
<div id=basetext>The following rewards will be yours if you complete this mission:<br>
<div><TABLE>

<TR VALIGN=middle>
    <TD width=36><img style:vertical-align:bottom src="icon:06_03" size="32"></TD>
    <TD width=352>25000 credits</TD>
</TR>

<TR VALIGN=middle>
    <TD width=36><a href=showinfo:29247><img src="typeicon:29247" width=32 height=32 align=left></a></TD>
    <TD width=352>14 Loyalty Points.</TD>
</TR>

</TABLE></div><br>
<span id=subheader>Bonus Rewards<BR></span>

<div id=basetext>The following rewards will be awarded to you as a bonus if you complete the mission within 34 minutes.<br>
<div><TABLE>
<TR VALIGN=middle>
    <TD width=36><img style:vertical-align:bottom src="icon:06_03" size="32"></TD>
    <TD width=352>22000 credits</TD>
</TR>
</TABLE></div><br>
</body></html>"]
            [PyString "locations"]
            [PyList 2 items]
              [PyInt 30002776]
              [PyInt 30002778]
*/
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
    //takes no arguments
    return new PyNone();

}
