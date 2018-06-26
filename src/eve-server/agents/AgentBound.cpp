
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

#include "StaticDataMgr.h"
#include "agents/AgentBound.h"

AgentBound::AgentBound(PyServiceMgr *mgr, CorpAgent *agt)
: PyBoundObject(mgr),
m_dispatch(new Dispatcher(this)),
m_agent(agt)
{
    _SetCallDispatcher(m_dispatch);

    m_strBoundObjectName = "AgentBound";

    PyCallable_REG_CALL(AgentBound, DoAction);
    PyCallable_REG_CALL(AgentBound, GetAgentLocationWrap);
    PyCallable_REG_CALL(AgentBound, GetInfoServiceDetails);
    PyCallable_REG_CALL(AgentBound, GetMissionBriefingInfo);
    PyCallable_REG_CALL(AgentBound, GetMissionObjectiveInfo);
    PyCallable_REG_CALL(AgentBound, GetMissionKeywords);
    PyCallable_REG_CALL(AgentBound, GetMissionJournalInfo);
    PyCallable_REG_CALL(AgentBound, GetDungeonShipRestrictions);
    PyCallable_REG_CALL(AgentBound, RemoveOfferFromJournal);
    PyCallable_REG_CALL(AgentBound, GetOfferJournalInfo);
    PyCallable_REG_CALL(AgentBound, GetEntryPoint);
    PyCallable_REG_CALL(AgentBound, GotoLocation);
    PyCallable_REG_CALL(AgentBound, WarpToLocation);
    PyCallable_REG_CALL(AgentBound, GetMyJournalDetails);
}

PyResult AgentBound::Handle_GetAgentLocationWrap(PyCallArgs &call) {
    // this is detailed info on agent's location
    return m_agent->GetLocationWrap();
}

PyResult AgentBound::Handle_GetInfoServiceDetails( PyCallArgs& call ) {
    // this is agents personal info... level, division, station, etc.
    return m_agent->GetInfoServiceDetails();
}

//15:39:45 L AgentBound::Handle_DoAction(): size= 1, 0=None
PyResult AgentBound::Handle_DoAction(PyCallArgs &call) {
    // sends PyNone or actionID
    _log(AGENT__DUMP,  "AgentBound::Handle_DoAction() - size= %u", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    Call_SingleArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: failed to decode arguments", call.client->GetName());
        return nullptr;
    }

    uint32 actionID = PyRep::IntegerValue(args.arg);

    // this actually returns a complicated tuple depending on other variables involving this agent and char.
    /*
        agentSays, dialogue, extraInfo = self.__GetConversation(agentDialogueWindow, actionID)

    def __GetConversation(self, wnd, actionID):
       tmp = wnd.sr.agentMoniker.DoAction(actionID)
        ret, wnd.sr.oob = tmp
        agentSays, wnd.sr.dialogue = ret
        firstActionID = wnd.sr.dialogue[0][0]
        firstActionDialogue = wnd.sr.dialogue[0][1]
        wnd.sr.agentSays = self.ProcessMessage(agentSays, wnd.sr.agentID)
        return (wnd.sr.agentSays, wnd.sr.dialogue, wnd.sr.oob)
     *
     */

    /* server reply to initiate convo
     *          [PyTuple 2 items]
     *            [PyTuple 2 items]
     *              [PyObjectEx Normal]
     *                [PyTuple 2 items]
     *                  [PyToken __builtin__.unicode]
     *                  [PyTuple 1 items]
     *                    [PyString "Welcome, Aknor Jaden."]
     *              [PyList 2 items]
     *                [PyTuple 2 items]
     *                  [PyInt 606]
     *                  [PyString "[Button]Request Mission"]
     *                [PyTuple 2 items]
     *                  [PyInt 607]
     *                  [PyString "[Button]Locate Character"]
     *            [PyDict 4 kvp]
     *              [PyString "missionCompleted"]
     *              [PyNone]
     *              [PyString "missionQuit"]
     *              [PyNone]
     *              [PyString "loyaltyPoints"]
     *              [PyInt 0]
     *              [PyString "missionDeclined"]
     *              [PyNone]
     **************************************
     *      [PySubStream 230 bytes]
     *        [PyTuple 2 items]
     *          [PyTuple 2 items]
     *            [PyObjectEx Normal]
     *              [PyTuple 2 items]
     *                [PyToken __builtin__.unicode]
     *                [PyTuple 1 items]
     *                  [PyString "You do know you haven't finished your current mission for me, right?"]
     *            [PyList 2 items]
     *              [PyTuple 2 items]
     *                [PyInt 592]
     *                [PyString "[Button]View Mission"]
     *              [PyTuple 2 items]
     *                [PyInt 593]
     *                [PyString "[Button]Locate Character"]
     *          [PyDict 4 kvp]
     *            [PyString "missionCompleted"]
     *            [PyNone]
     *            [PyString "missionQuit"]
     *            [PyNone]
     *            [PyString "loyaltyPoints"]
     *            [PyInt 0]
     *            [PyString "missionDeclined"]
     *            [PyNone]
     ***************************************
     *            [PyString "DoAction"]
     *            [PyTuple 1 items]
     *              [PyInt 592]
     *************************************
     *      [PySubStream 185 bytes]
     *        [PyTuple 2 items]
     *          [PyTuple 2 items]
     *            [PyObjectEx Normal]
     *              [PyTuple 2 items]
     *                [PyToken __builtin__.unicode]
     *                [PyTuple 1 items]
     *                  [PyString "Good to hear, carry on."]
     *            [PyList 2 items]
     *              [PyTuple 2 items]
     *                [PyInt 594]
     *                [PyString "[Button]Complete Mission"]
     *              [PyTuple 2 items]
     *                [PyInt 595]
     *                [PyString "[Button]Quit Mission"]
     *          [PyDict 4 kvp]
     *            [PyString "missionCompleted"]
     *            [PyBool False]
     *            [PyString "missionQuit"]
     *            [PyBool False]
     *            [PyString "loyaltyPoints"]
     *            [PyInt 0]
     *            [PyString "missionDeclined"]
     *            [PyBool False]
     */

    if (actionID == 0) {
        std::string response = "Why the fuck am I looking at you again, ";
        response += call.client->GetCharacterName().c_str();
        response += "?";
        //  this will get complicated and is based on agent/char interaction
        //   detail in /eve/client/script/ui/station/agents/agents.py
        PyTuple* agentSays = new PyTuple(2);
            agentSays->SetItem(0, new PyString(response));  //msgInfo  -- if tuple[0].string then return msgInfo
            agentSays->SetItem(1, new PyNone());    //contentID
            // msgArgs = self.GetAgentMoniker(agentID).GetMissionKeywords(contentID)

        // buttonIDs will change based on char/agent.  will need to store in agent for proper response
        PyTuple* button1 = new PyTuple(2);
            button1->SetItem(0, new PyInt(123));
            button1->SetItem(1, new PyString("Request Mission"));
        // if agent does location, add this one...
        PyTuple* button2 = new PyTuple(2);
            button2->SetItem(0, new PyInt(456));
            button2->SetItem(1, new PyString("Locate Character"));
        // if agent also does research, add this one...
        PyTuple* button3 = new PyTuple(2);
            button3->SetItem(0, new PyInt(789));
            button3->SetItem(1, new PyString("Begin Research"));

        // ItemString("Mission Title ID", *msg*) - *msg* will display under agent picture when returned under 'dialog' as PyDict
        PyList* dialog = new PyList();
            dialog->AddItem(button1);
            dialog->AddItem(button2);
            dialog->AddItem(button3);

        PyTuple* inner = new PyTuple(2);
            inner->SetItem(0, agentSays);
            inner->SetItem(1, dialog);

        PyDict* xtraInfo = new PyDict();
            xtraInfo->SetItemString("loyaltyPoints", new PyInt(10));  // this is char current LP
            xtraInfo->SetItemString("missionCompleted", new PyBool(false));
            xtraInfo->SetItemString("missionQuit", new PyBool(false));
            xtraInfo->SetItemString("missionDeclined", new PyBool(false));

        PyTuple* outer = new PyTuple(2);
            outer->SetItem(0, inner);
            outer->SetItem(1, new PyObject("util.KeyVal", xtraInfo));

        if (is_log_enabled(AGENT__RSPDUMP))
            outer->Dump(AGENT__RSPDUMP, "    ");
        /*  this creates the following packet structure:.
        * 08:19:48 [SvcCall] Service AgentBound::DoAction()
        * 08:19:48 [AgentDump]   Call Arguments:
        * 08:19:48 [AgentDump]      Tuple: 1 elements
        * 08:19:48 [AgentDump]       [ 0]       None
        * 08:19:48 [AgentDump]  Named Arguments:
        * 08:19:48 [AgentDump]   machoVersion
        * 08:19:48 [AgentDump]        Integer: 1
        * 08:19:48 [AgentRspDump]      Tuple: 2 elements
        * 08:19:48 [AgentRspDump]       [ 0]  Tuple: 2 elements
        * 08:19:48 [AgentRspDump]       [ 0]   [ 0]  Tuple: 2 elements
        * 08:19:48 [AgentRspDump]       [ 0]   [ 0]   [ 0]     String: 'Why the fuck am I looking at you again, allan?'
        * 08:19:48 [AgentRspDump]       [ 0]   [ 0]   [ 1]       None
        * 08:19:48 [AgentRspDump]       [ 0]   [ 1]   List: 2 elements
        * 08:19:48 [AgentRspDump]       [ 0]   [ 1]   [ 0]  Tuple: 2 elements
        * 08:19:48 [AgentRspDump]       [ 0]   [ 1]   [ 0]   [ 0]    Integer: 123
        * 08:19:48 [AgentRspDump]       [ 0]   [ 1]   [ 0]   [ 1]     String: '[Button]Request Mission'
        * 08:19:48 [AgentRspDump]       [ 0]   [ 1]   [ 1]  Tuple: 2 elements
        * 08:19:48 [AgentRspDump]       [ 0]   [ 1]   [ 1]   [ 0]    Integer: 456
        * 08:19:48 [AgentRspDump]       [ 0]   [ 1]   [ 1]   [ 1]     String: '[Button]Locate Character'
        * 08:19:48 [AgentRspDump]       [ 1] Object:
        * 08:19:48 [AgentRspDump]       [ 1]   Type:     String: 'util.KeyVal'
        * 08:19:48 [AgentRspDump]       [ 1]   Args:  Dictionary: 4 entries
        * 08:19:48 [AgentRspDump]       [ 1]   Args:   [ 0]   Key:     String: 'missionQuit'
        * 08:19:48 [AgentRspDump]       [ 1]   Args:   [ 0] Value:    Boolean: false
        * 08:19:48 [AgentRspDump]       [ 1]   Args:   [ 1]   Key:     String: 'missionCompleted'
        * 08:19:48 [AgentRspDump]       [ 1]   Args:   [ 1] Value:    Boolean: false
        * 08:19:48 [AgentRspDump]       [ 1]   Args:   [ 2]   Key:     String: 'missionDeclined'
        * 08:19:48 [AgentRspDump]       [ 1]   Args:   [ 2] Value:    Boolean: false
        * 08:19:48 [AgentRspDump]       [ 1]   Args:   [ 3]   Key:     String: 'loyaltyPoints'
        * 08:19:48 [AgentRspDump]       [ 1]   Args:   [ 3] Value:    Integer: 10
        */

        return outer;
    } else if (actionID == 123) {
        PyDict* keywords = new PyDict();
        keywords->SetItemString("objectiveLocationID", new PyInt(m_agent->GetStationID()));
        keywords->SetItemString("objectiveLocationSystemID", new PyInt(m_agent->GetSystemID()));
        keywords->SetItemString("objectiveTypeID", new PyInt(2631));
        keywords->SetItemString("objectiveQuantity", new PyInt(7));
        keywords->SetItemString("objectiveDestinationID", new PyInt(60001690));
        keywords->SetItemString("objectiveDestinationSystemID", new PyInt(30002777));
        keywords->SetItemString("rewardTypeID", new PyInt(29));
        keywords->SetItemString("rewardQuantity", new PyInt(28000));

        PyDict *res = new PyDict();
        //res->SetItemString("ContentID", new PyNone());  // not sure when/if this is filled
        //res->SetItemString("Mission Keywords", keywords);  // only used when "ContentID" is filled?
        res->SetItemString("Decline Time", new PyInt(-1));   // generic decline warning.  else fill with remaining mission time
        res->SetItemString("Mission Image", new PyString("<img src='res:/UI/netres/mission_content/couriermission.png' align=center hspace=4 vspace=4>") );
        res->SetItemString("Mission Title", new PyString("Human Problem") );
        res->SetItemString("Mission Briefing", new PyString("<div id=basetext>Our cargo area and lower-tier decks are overflowing with vagabonds and tramps.  It has almost reached epidemic proportions.  These people simply refuse to leave, some are laid off workers of ours while others are simply outsiders who think they can come and live off our hospitality indefinitely.  Since there are strict regulations against sending them out the airlock for no reason, and since we can't simply frame every single one of them for a crime worthy of such a fate, we've no choice but to forcibly move them somewhere else.  Another station will do.  In fact I'd like you to move a heap-load of them to Isutaka I - Caldari Steel Warehouse.  I never liked those bastards over there anyway, let them deal with this 'human problem'.</div>    <br>") );
        res->SetItemString("Expiration Message", new PyString("<span id=subheader>Mission Expiration</span><br><div id=basetext>This mission expires at 2011.05.10 01:29:33</div>"));

        // client throws error if these are not included....not sure what they're for yet.
        res->SetItemString("Expiration Time", new PyLong( GetFileTimeNow()+Win32Time_Day ) );
        res->SetItemString("Mission Title ID", new PyInt(57117) );  // this 'value' will display on initial convo window, under agent picture, if sent with DoAction = 0
        res->SetItemString("Mission Briefing ID", new PyInt(145160) );

        if (is_log_enabled(AGENT__RSPDUMP))
            res->Dump(AGENT__RSPDUMP, "    ");
        return res;
        /* 1
         * 09:32:03 [SvcCall] Service AgentBound::DoAction()
         * 09:32:03 [AgentDump] AgentBound::Handle_DoAction() - size= 1
         * 09:32:03 [AgentDump]   Call Arguments:
         * 09:32:03 [AgentDump]      Tuple: 1 elements
         * 09:32:03 [AgentDump]       [ 0]    Integer: 123
         * 09:32:03 [AgentDump]  Named Arguments:
         * 09:32:03 [AgentDump]   machoVersion
         * 09:32:03 [AgentDump]        Integer: 1
         * 09:32:03 [AgentRspDump]      Dictionary: 9 entries
         * 09:32:03 [AgentRspDump]       [ 0]   Key:     String: 'Mission Briefing ID'
         * 09:32:03 [AgentRspDump]       [ 0] Value:    Integer: 145160
         * 09:32:03 [AgentRspDump]       [ 1]   Key:     String: 'Mission Title ID'
         * 09:32:03 [AgentRspDump]       [ 1] Value:    Integer: 57117
         * 09:32:03 [AgentRspDump]       [ 2]   Key:     String: 'Expiration Time'
         * 09:32:03 [AgentRspDump]       [ 2] Value:       Long: 131745835230366720
         * 09:32:03 [AgentRspDump]       [ 3]   Key:     String: 'Expiration Message'
         * 09:32:03 [AgentRspDump]       [ 3] Value:     String: '<span id=subheader>Mission Expiration</span><br><div id=basetext>This mission expires at 2011.05.10 01:29:33</div>'
         * 09:32:03 [AgentRspDump]       [ 4]   Key:     String: 'Mission Briefing'
         * 09:32:03 [AgentRspDump]       [ 4] Value:     String: '<div id=basetext>Our cargo area and lower-tier decks are overflowing with vagabonds and tramps.  It has almost reached epidemic proportions.  These people simply refuse to leave, some are laid off workers of ours while others are simply outsiders who think they can come and live off our hospitality indefinitely.  Since there are strict regulations against sending them out the airlock for no reason, and since we can't simply frame every single one of them for a crime worthy of such a fate, we've no choice but to forcibly move them somewhere else.  Another station will do.  In fact I'd like you to move a heap-load of them to Isutaka I - Caldari Steel Warehouse.  I never liked those bastards over there anyway, let them deal with this 'human problem'.</div>    <br>'
         * 09:32:03 [AgentRspDump]       [ 5]   Key:     String: 'Mission Title'
         * 09:32:03 [AgentRspDump]       [ 5] Value:     String: 'Human Problem'
         * 09:32:03 [AgentRspDump]       [ 6]   Key:     String: 'Mission Image'
         * 09:32:03 [AgentRspDump]       [ 6] Value:     String: '<img src='res:/UI/netres/mission_content/couriermission.png' align=center hspace=4 vspace=4>'
         * 09:32:03 [AgentRspDump]       [ 7]   Key:     String: 'Decline Time'
         * 09:32:03 [AgentRspDump]       [ 7] Value:    Integer: -1
         * 09:32:03 [AgentRspDump]       [ 8]   Key:     String: 'Mission Keywords'
         * 09:32:03 [AgentRspDump]       [ 8] Value:  Dictionary: 8 entries
         * 09:32:03 [AgentRspDump]       [ 8] Value:   [ 0]   Key:     String: 'rewardQuantity'
         * 09:32:03 [AgentRspDump]       [ 8] Value:   [ 0] Value:    Integer: 28000
         * 09:32:03 [AgentRspDump]       [ 8] Value:   [ 1]   Key:     String: 'objectiveDestinationSystemID'
         * 09:32:03 [AgentRspDump]       [ 8] Value:   [ 1] Value:    Integer: 30002777
         * 09:32:03 [AgentRspDump]       [ 8] Value:   [ 2]   Key:     String: 'objectiveDestinationID'
         * 09:32:03 [AgentRspDump]       [ 8] Value:   [ 2] Value:    Integer: 60001690
         * 09:32:03 [AgentRspDump]       [ 8] Value:   [ 3]   Key:     String: 'rewardTypeID'
         * 09:32:03 [AgentRspDump]       [ 8] Value:   [ 3] Value:    Integer: 29
         * 09:32:03 [AgentRspDump]       [ 8] Value:   [ 4]   Key:     String: 'objectiveQuantity'
         * 09:32:03 [AgentRspDump]       [ 8] Value:   [ 4] Value:    Integer: 7
         * 09:32:03 [AgentRspDump]       [ 8] Value:   [ 5]   Key:     String: 'objectiveTypeID'
         * 09:32:03 [AgentRspDump]       [ 8] Value:   [ 5] Value:    Integer: 2631
         * 09:32:03 [AgentRspDump]       [ 8] Value:   [ 6]   Key:     String: 'objectiveLocationSystemID'
         * 09:32:03 [AgentRspDump]       [ 8] Value:   [ 6] Value:    Integer: 30002507
         * 09:32:03 [AgentRspDump]       [ 8] Value:   [ 7]   Key:     String: 'objectiveLocationID'
         * 09:32:03 [AgentRspDump]       [ 8] Value:   [ 7] Value:    Integer: 60005728
         * 09:32:03 [SvcCall] Service alert::BeanCount()
         * 09:32:03 [SvcCall] Service alert::SendClientStackTraceAlert()
         * EXCEPTION #6 logged at  06/26/2018 9:32:03 Unhandled exception in <TaskletExt object at 466b7be0, abps=1001, ctxt='<NO CONTEXT>^<bound method BaseLink.OnClick of uicls.BaseLink object at (snip)a94330, name=textlink, destroyed=False>>'>
         * Caught at:
         * /common/lib/bluepy.py(98) CallWrapper
         * Thrown at:
         * /common/lib/bluepy.py(86) CallWrapper
         * /../carbon/client/script/ui/control/baselink.py(42) OnClick
         * /../carbon/client/script/ui/control/baselink.py(52) ClickLink
         * /../carbon/client/script/ui/control/baselink.py(206) LocalSvcCall
         * /client/script/ui/station/agents/agents.py(580) DoAction
         * /client/script/ui/station/agents/agents.py(936) __Interact
         * /client/script/ui/station/agents/agents.py(461) __GetConversation
         *        tmp = {'Decline Time': -1,
         *                        'Expiration Message': '<span id=subheader>Mission Expiration</span><br><div id=basetext>This mission expires at 2011.05.10 01:29:33</div>',
         *                        'Expiration Time': 131745835230366720L,
         *                        'Mission Briefing': "<div id=basetext>Our cargo area and lower-tier decks are overflowing with vagabonds and tramps.  It has almost reached epidemic proportions.  These people simply refuse to leave, some are laid off workers of ours while others are simply outsiders who think they can come and live off our hospitality indefinitely.  Since there are strict regulations against sending them out the airlock for no reason, and since we can't simply frame every single one of them for a crime worthy of such a fate, we've no choice but to forcibly move them somewhere else.  Another station will do.  In fact I'd like you to move a heap-load of them to Isutaka I - Caldari Steel Warehouse.  I never liked those bastards over there anyway, let them deal with this 'human problem'.</div>    <br>",
         *                       ...
         *        self = <svc.Agents instance at 0x48A89CD8>
         *        actionID = 123
         *        wnd = form.AgentDialogueWindow object at 0x489c1b30, name=agentinteraction_3012674, destroyed=False>
         * ValueError: too many values to unpack
         */
    }

    return nullptr;
}
/*
 *    [PyTuple 1 items]
 *      [PyTuple 2 items]
 *        [PyInt 1]
 *        [PySubStream 41 bytes]
 *          [PyTuple 4 items]
 *            [PyString "N=696805:4035"]
 *            [PyString "DoAction"]
 *            [PyTuple 1 items]
 *              [PyInt 594]
 *            [PyDict 1 kvp]
 *              [PyString "machoVersion"]
 *              [PyInt 1]
 *
 *    [PyTuple 1 items]
 *      [PySubStream 206 bytes]
 *        [PyTuple 2 items]
 *          [PyTuple 2 items]
 *            [PyObjectEx Normal]
 *              [PyTuple 2 items]
 *                [PyToken __builtin__.unicode]
 *                [PyTuple 1 items]
 *                  [PyString "I knew I could count on you, Aknor Jaden."]
 *            [PyList 2 items]
 *              [PyTuple 2 items]
 *                [PyInt 596]
 *                [PyString "[Button]Request Mission"]
 *              [PyTuple 2 items]
 *                [PyInt 597]
 *                [PyString "[Button]Locate Character"]
 *          [PyDict 4 kvp]
 *            [PyString "missionCompleted"]
 *            [PyBool True]
 *            [PyString "missionQuit"]
 *            [PyBool False]
 *            [PyString "loyaltyPoints"]
 *            [PyInt 0]
 *            [PyString "missionDeclined"]
 *            [PyBool False]
 *
 *    [PyTuple 1 items]
 *      [PyTuple 2 items]
 *        [PyInt 1]
 *        [PySubStream 41 bytes]
 *          [PyTuple 4 items]
 *            [PyString "N=696805:4035"]
 *            [PyString "DoAction"]
 *            [PyTuple 1 items]
 *              [PyInt 596]
 *            [PyDict 1 kvp]
 *              [PyString "machoVersion"]
 *              [PyInt 1]
 *
 *      [PySubStream 713 bytes]
 *        [PyTuple 2 items]
 *          [PyTuple 2 items]
 *            [PyObjectEx Normal]
 *              [PyTuple 2 items]
 *                [PyToken __builtin__.unicode]
 *                [PyTuple 1 items]
 *                  [PyString "Something just came up that's right up your alley.  Bio-engineers and farmers in nearby settlements, which have been supplying us with foodstuffs, have had good fortune recently.  Their supplies of frozen plant seeds are overflowing, and they would like to sell some of their excess products.  Obviously we took advantage of a good opportunity and bought the goods for silly prices, those people are so gullible.  I'd like you to deliver the goods to Tasabeshi VIII - Moon 13 - CBD Corporation Storage, where we've found a buyer."]
 *            [PyList 3 items]
 *              [PyTuple 2 items]
 *                [PyInt 598]
 *                [PyString "[Button]Accept"]
 *              [PyTuple 2 items]
 *                [PyInt 599]
 *                [PyString "[Button]Decline"]
 *              [PyTuple 2 items]
 *                [PyInt 600]
 *                [PyString "[Button][CloseOnClick]Delay"]
 *          [PyDict 4 kvp]
 *            [PyString "missionCompleted"]
 *            [PyBool False]
 *            [PyString "missionQuit"]
 *            [PyBool False]
 *            [PyString "loyaltyPoints"]
 *            [PyInt 0]
 *            [PyString "missionDeclined"]
 *            [PyBool False]
 *
 *
 *            [PyString "DoAction"]
 *            [PyTuple 1 items]
 *              [PyInt 598]
 *    [PyTuple 1 items]
 *      [PySubStream 185 bytes]
 *        [PyTuple 2 items]
 *          [PyTuple 2 items]
 *            [PyObjectEx Normal]
 *              [PyTuple 2 items]
 *                [PyToken __builtin__.unicode]
 *                [PyTuple 1 items]
 *                  [PyString "Good to hear, carry on."]
 *            [PyList 2 items]
 *              [PyTuple 2 items]
 *                [PyInt 601]
 *                [PyString "[Button]Complete Mission"]
 *              [PyTuple 2 items]
 *                [PyInt 602]
 *                [PyString "[Button]Quit Mission"]
 *          [PyDict 4 kvp]
 *            [PyString "missionCompleted"]
 *            [PyBool False]
 *            [PyString "missionQuit"]
 *            [PyBool False]
 *            [PyString "loyaltyPoints"]
 *            [PyInt 0]
 *            [PyString "missionDeclined"]
 *            [PyBool False]
 */

//21:13:12 L AgentBound::Handle_GetMissionBriefingInfo(): size= 0
PyResult AgentBound::Handle_GetMissionBriefingInfo(PyCallArgs &call) {
    // will return PyNone if no mission avalible
    /**
     *    [PyTuple 1 items]
     *      [PySubStream 845 bytes]
     *        [PyDict 6 kvp]
     *          [PyString "ContentID"]
     *          [PyNone]
     *          [PyString "Expiration Message"]
     *          [PyString "
     * <span id=subheader>Mission Expiration</span><br>
     * <div id=basetext>This mission expires at 2011.05.11 02:43:00</div>
     * "]
     *          [PyString "Decline Warning"]
     *          [PyString ""]
     *          [PyString "Mission Image"]
     *          [PyString "<img src="res:/UI/netres/mission_content/couriermission.png" align=center hspace=4 vspace=4>"]
     *          [PyString "Mission Title"]
     *          [PyString "Good Harvest"]
     *          [PyString "Mission Briefing"]
     *          [PyString "
     * <div id=basetext>Bio-engineers and farmers in nearby settlements, which have been supplying us with foodstuffs, have had good fortune recently.  Their supplies of frozen plant seeds are overflowing, and they would like to sell some of their excess products.  Obviously we took advantage of a good opportunity and bought the goods for silly prices, those people are so gullible.  I'd like you to deliver the goods to Tasabeshi VIII - Moon 13 - CBD Corporation Storage, where we've found a buyer.</div>
     * <br>
     * "]
     *
        [PyDict 6 kvp]
          [PyString "ContentID"]
          [PyNone]
          [PyString "Expiration Message"]
          [PyString "
<span id=subheader>Mission Expiration</span><br>
<div id=basetext>This mission expires at 2011.05.10 01:29:33</div>
"]
          [PyString "Decline Warning"]
          [PyString ""]
          [PyString "Mission Image"]
          [PyString "<img src="res:/UI/netres/mission_content/couriermission.png" align=center hspace=4 vspace=4>"]
          [PyString "Mission Title"]
          [PyString "Human Problem"]
          [PyString "Mission Briefing"]
          [PyString "
<div id=basetext>Our cargo area and lower-tier decks are overflowing with vagabonds and tramps.  It has almost reached epidemic proportions.  These people simply refuse to leave, some are laid off workers of ours while others are simply outsiders who think they can come and live off our hospitality indefinitely.  Since there are strict regulations against sending them out the airlock for no reason, and since we can't simply frame every single one of them for a crime worthy of such a fate, we've no choice but to forcibly move them somewhere else.  Another station will do.  In fact I'd like you to move a heap-load of them to Isutaka I - Caldari Steel Warehouse.  I never liked those bastards over there anyway, let them deal with this 'human problem'.</div>
<br>
"]
     */
    _log(AGENT__DUMP,  "AgentBound::Handle_GetMissionBriefingInfo() - size= %u", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    if (call.tuple->size() == 0)
        return nullptr;

    Call_SingleArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: failed to decode arguments", call.client->GetName());
        return nullptr;
    }

    uint32 missionID = PyRep::IntegerValue(args.arg);
    if (missionID == 0)
        return PyStatic.NewNone();

    PyDict* keywords = new PyDict();
        keywords->SetItemString("objectiveLocationID", new PyInt(m_agent->GetStationID()));
        keywords->SetItemString("objectiveLocationSystemID", new PyInt(m_agent->GetSystemID()));
        keywords->SetItemString("objectiveTypeID", new PyInt(2631));
        keywords->SetItemString("objectiveQuantity", new PyInt(7));
        keywords->SetItemString("objectiveDestinationID", new PyInt(60001690));
        keywords->SetItemString("objectiveDestinationSystemID", new PyInt(30002777));
        keywords->SetItemString("rewardTypeID", new PyInt(29));
        keywords->SetItemString("rewardQuantity", new PyInt(28000));

    PyDict *res = new PyDict();
        //res->SetItemString("ContentID", new PyNone());  // not sure when/if this is filled
        res->SetItemString("Mission Keywords", keywords);  // only used when "ContentID" is filled?
        res->SetItemString("Decline Time", new PyInt(-1));   // generic decline warning.  else fill with remaining mission time
        res->SetItemString("Mission Image", new PyString("<img src='res:/UI/netres/mission_content/couriermission.png' align=center hspace=4 vspace=4>") );
        res->SetItemString("Mission Title", new PyString("Human Problem") );
        res->SetItemString("Mission Briefing", new PyString("<div id=basetext>Our cargo area and lower-tier decks are overflowing with vagabonds and tramps.  It has almost reached epidemic proportions.  These people simply refuse to leave, some are laid off workers of ours while others are simply outsiders who think they can come and live off our hospitality indefinitely.  Since there are strict regulations against sending them out the airlock for no reason, and since we can't simply frame every single one of them for a crime worthy of such a fate, we've no choice but to forcibly move them somewhere else.  Another station will do.  In fact I'd like you to move a heap-load of them to Isutaka I - Caldari Steel Warehouse.  I never liked those bastards over there anyway, let them deal with this 'human problem'.</div>    <br>") );
        res->SetItemString("Expiration Message", new PyString("<span id=subheader>Mission Expiration</span><br><div id=basetext>This mission expires at 2011.05.10 01:29:33</div>"));

        // client throws error if these are not included....not sure what they're for yet.
        res->SetItemString("Expiration Time", new PyLong( GetFileTimeNow()+Win32Time_Day ) );
        res->SetItemString("Mission Title ID", new PyInt(57117) );  // this 'value' will display on initial convo window, under agent picture, if sent with DoAction = 0
        res->SetItemString("Mission Briefing ID", new PyInt(145160) );

    if (is_log_enabled(AGENT__RSPDUMP))
        res->Dump(AGENT__RSPDUMP, "    ");
    return res;
    /*
     * 08:19:48 [SvcCall] Service AgentBound::GetMissionBriefingInfo()
     * 08:19:48 [AgentDump]   Call Arguments:
     * 08:19:48 [AgentDump]      Tuple: Empty
     * 08:19:48 [AgentDump]  Named Arguments:
     * 08:19:48 [AgentDump]   machoVersion
     * 08:19:48 [AgentDump]        Integer: 1
     * 08:19:48 [AgentRspDump]      Dictionary: 9 entries
     * 08:19:48 [AgentRspDump]       [ 0]   Key:     String: 'Mission Briefing ID'
     * 08:19:48 [AgentRspDump]       [ 0] Value:     String: 'Mission Briefing ID'
     * 08:19:48 [AgentRspDump]       [ 1]   Key:     String: 'Mission Title ID'
     * 08:19:48 [AgentRspDump]       [ 1] Value:     String: 'Mission Title ID'
     * 08:19:48 [AgentRspDump]       [ 2]   Key:     String: 'Expiration Time'
     * 08:19:48 [AgentRspDump]       [ 2] Value:       Long: 131745791881122576
     * 08:19:48 [AgentRspDump]       [ 3]   Key:     String: 'Expiration Message'
     * 08:19:48 [AgentRspDump]       [ 3] Value:     String: '<span id=subheader>Mission Expiration</span><br><div id=basetext>This mission expires at 2011.05.10 01:29:33</div>'
     * 08:19:48 [AgentRspDump]       [ 4]   Key:     String: 'Mission Title'
     * 08:19:48 [AgentRspDump]       [ 4] Value:     String: 'Human Problem'
     * 08:19:48 [AgentRspDump]       [ 5]   Key:     String: 'Mission Image'
     * 08:19:48 [AgentRspDump]       [ 5] Value:     String: '<img src='res:/UI/netres/mission_content/couriermission.png' align=center hspace=4 vspace=4>'
     * 08:19:48 [AgentRspDump]       [ 6]   Key:     String: 'Decline Time'
     * 08:19:48 [AgentRspDump]       [ 6] Value:    Integer: -1
     * 08:19:48 [AgentRspDump]       [ 7]   Key:     String: 'Mission Briefing'
     * 08:19:48 [AgentRspDump]       [ 7] Value:     String: '<div id=basetext>Our cargo area and lower-tier decks are overflowing with vagabonds and tramps.  It has almost reached epidemic proportions.  These people simply refuse to leave, some are laid off workers of ours while others are simply outsiders who think they can come and live off our hospitality indefinitely.  Since there are strict regulations against sending them out the airlock for no reason, and since we can't simply frame every single one of them for a crime worthy of such a fate, we've no choice but to forcibly move them somewhere else.  Another station will do.  In fact I'd like you to move a heap-load of them to Isutaka I - Caldari Steel Warehouse.  I never liked those bastards over there anyway, let them deal with this 'human problem'.</div>    <br>'
     * 08:19:48 [AgentRspDump]       [ 8]   Key:     String: 'ContentID'
     * 08:19:48 [AgentRspDump]       [ 8] Value:       None
     */
}

PyResult AgentBound::Handle_GetMissionObjectiveInfo(PyCallArgs &call)
{
    //ret = self.GetAgentMoniker(agentID).GetMissionObjectiveInfo(charID, contentID)

    _log(AGENT__DUMP,  "AgentBound::Handle_GetMissionObjectiveInfo() - size= %u", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    if (call.tuple->size() == 0)
        return nullptr;

    Call_SingleArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: failed to decode arguments", call.client->GetName());
        return nullptr;
    }

    uint32 missionID = PyRep::IntegerValue(args.arg);
    if (missionID == 0)
        return PyStatic.NewNone();

    /*     called when result of DoAction() 'msgInfo' includes a 'contentID' value
    this returns the html style shit for the mission data text....

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


// new...not handled
PyResult AgentBound::Handle_GetMissionKeywords(PyCallArgs &call) {
    //self.missionArgs[contentID] = self.GetAgentMoniker(agentID).GetMissionKeywords(contentID)
    _log(AGENT__DUMP,  "AgentBound::Handle_GetMissionKeywords() - size= %u", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    return nullptr;
}

PyResult AgentBound::Handle_GetMissionJournalInfo(PyCallArgs &call) {
    //ret = self.GetAgentMoniker(agentID).GetMissionJournalInfo(charID, contentID)
    _log(AGENT__DUMP,  "AgentBound::Handle_GetMissionJournalInfo() - size= %u", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    return nullptr;
}

PyResult AgentBound::Handle_GetDungeonShipRestrictions(PyCallArgs &call) {
    //restrictions = self.GetAgentMoniker(agentID).GetDungeonShipRestrictions(dungeonID)
    _log(AGENT__DUMP,  "AgentBound::Handle_GetDungeonShipRestrictions() - size= %u", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    return nullptr;
}

PyResult AgentBound::Handle_RemoveOfferFromJournal(PyCallArgs &call) {
    //self.GetAgentMoniker(agentID).RemoveOfferFromJournal()
    _log(AGENT__DUMP,  "AgentBound::Handle_RemoveOfferFromJournal() - size= %u", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    return nullptr;
}

PyResult AgentBound::Handle_GetOfferJournalInfo(PyCallArgs &call) {
    //html = self.GetAgentMoniker(agentID).GetOfferJournalInfo()
    _log(AGENT__DUMP,  "AgentBound::Handle_GetOfferJournalInfo() - size= %u", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    return nullptr;
}

PyResult AgentBound::Handle_GetEntryPoint(PyCallArgs &call) {
    //entryPoint = sm.StartService('agents').GetAgentMoniker(bookmark.agentID).GetEntryPoint()
    _log(AGENT__DUMP,  "AgentBound::Handle_GetEntryPoint() - size= %u", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    return nullptr;
}

PyResult AgentBound::Handle_GotoLocation(PyCallArgs &call) {
    //sm.StartService('agents').GetAgentMoniker(bookmark.agentID).GotoLocation(bookmark.locationType, bookmark.locationNumber, referringAgentID)
    _log(AGENT__DUMP,  "AgentBound::Handle_GotoLocation() - size= %u", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    return nullptr;
}

PyResult AgentBound::Handle_WarpToLocation(PyCallArgs &call) {
    //sm.StartService('agents').GetAgentMoniker(bookmark.agentID).WarpToLocation(bookmark.locationType, bookmark.locationNumber, warpRange, fleet, referringAgentID)
    _log(AGENT__DUMP,  "AgentBound::Handle_WarpToLocation() - size= %u", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    return nullptr;
}

PyResult AgentBound::Handle_GetMyJournalDetails(PyCallArgs &call) {
    //parallelCalls.append((sm.GetService('agents').GetAgentMoniker(agentID).GetMyJournalDetails, ()))
    _log(AGENT__DUMP,  "AgentBound::Handle_GetMyJournalDetails() - size= %u", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    return nullptr;
}
