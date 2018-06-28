
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
    std::string response = "";
    PyList* dialog = new PyList();

    if (actionID == 0) {
        response = "Why the fuck am I looking at you again, ";
        response += call.client->GetCharacterName().c_str();
        response += "?";
        m_agent->SetMission(false);
        // dialogue data....
        PyTuple* button1 = new PyTuple(2);
            button1->SetItem(0, new PyInt(12));
            button1->SetItem(1, new PyInt(Dialog::Button::ViewMission));
        PyTuple* button2 = new PyTuple(2);
            button2->SetItem(0, new PyInt(13));
            button2->SetItem(1, new PyInt(Dialog::Button::RequestMission));
        // if agent does location, add this one...
        PyTuple* button3 = new PyTuple(2);
            button3->SetItem(0, new PyInt(34));
            button3->SetItem(1, new PyInt(Dialog::Button::LocateCharacter));
        // if agent also does research, add this one...
        PyTuple* button4 = new PyTuple(2);
            button4->SetItem(0, new PyInt(56));
            button4->SetItem(1, new PyInt(Dialog::Button::StartResearch));

        dialog->AddItem(button1);
        dialog->AddItem(button2);
        dialog->AddItem(button3);
        dialog->AddItem(button4);
    } else {
        // agentSays data.....
        response = "This just came up, ";
        response += call.client->GetCharacterName().c_str();
        response += ".";
        response += "<div id=basetext>Our cargo area and lower-tier decks are overflowing with vagabonds and tramps.  It has almost reached epidemic proportions. ";
        response += "These people simply refuse to leave, some are laid off workers of ours while others are simply outsiders who think they can come and live off our hospitality indefinitely. ";
        response += "Since there are strict regulations against sending them out the airlock for no reason, and since we can't simply frame every single one of them for a crime worthy of such a fate, we've no choice but to forcibly move them somewhere else. ";
        response += "Another station will do.  In fact I'd like you to move a heap-load of them to Isutaka I - Caldari Steel Warehouse.  I never liked those bastards over there anyway, let them deal with this 'human problem'.</div><br>";
        m_agent->SetMission(true);
        PyTuple* button1 = new PyTuple(2);
        button1->SetItem(0, new PyInt(23));
        button1->SetItem(1, new PyInt(Dialog::Button::Accept));
        PyTuple* button2 = new PyTuple(2);
        button2->SetItem(0, new PyInt(45));
        button2->SetItem(1, new PyInt(Dialog::Button::Decline));
        // if agent also does research, add this one...
        PyTuple* button3 = new PyTuple(2);
        button3->SetItem(0, new PyInt(67));
        button3->SetItem(1, new PyInt(Dialog::Button::Defer));

        dialog->AddItem(button1);
        dialog->AddItem(button2);
        dialog->AddItem(button3);
    }

        /*
        agentSays, dialogue, extraInfo = self.__GetConversation(agentDialogueWindow, actionID)

        tmp = wnd.sr.agentMoniker.DoAction(actionID)
        ret, wnd.sr.oob = tmp
        agentSays, wnd.sr.dialogue = ret
          */

    //  this will get complicated and is based on agent/char interaction
    //   detail in /eve/client/script/ui/station/agents/agents.py

        PyTuple* agentSays = new PyTuple(2);
            agentSays->SetItem(0, new PyString(response));  //msgInfo  -- if tuple[0].string then return msgInfo
            agentSays->SetItem(1, new PyNone());    //contentID

        // extraInfo data....
        PyDict* xtraInfo = new PyDict();
            xtraInfo->SetItemString("loyaltyPoints", new PyInt(10));  // this is char current LP
            xtraInfo->SetItemString("missionCompleted", new PyBool(false));
            xtraInfo->SetItemString("missionQuit", new PyBool(false));
            xtraInfo->SetItemString("missionDeclined", new PyBool(false));

        PyTuple* inner = new PyTuple(2);
            inner->SetItem(0, agentSays);
            inner->SetItem(1, dialog);
        PyTuple* outer = new PyTuple(2);
            outer->SetItem(0, inner);
            outer->SetItem(1, new PyObject("util.KeyVal", xtraInfo));

        if (is_log_enabled(AGENT__RSPDUMP))
            outer->Dump(AGENT__RSPDUMP, "    ");
        return outer;

        /*  above code creates the following packet structure when DoAction = 0
         * 07:50:26 [AgentRspDump]      Tuple: 2 elements
         * 07:50:26 [AgentRspDump]       [ 0]  Tuple: 2 elements
         * 07:50:26 [AgentRspDump]       [ 0]   [ 0]  Tuple: 2 elements
         * 07:50:26 [AgentRspDump]       [ 0]   [ 0]   [ 0]     String: 'Why the fuck am I looking at you again, allan?'
         * 07:50:26 [AgentRspDump]       [ 0]   [ 0]   [ 1]       None
         * 07:50:26 [AgentRspDump]       [ 0]   [ 1]   List: 4 elements
         * 07:50:26 [AgentRspDump]       [ 0]   [ 1]   [ 0]  Tuple: 2 elements
         * 07:50:26 [AgentRspDump]       [ 0]   [ 1]   [ 0]   [ 0]    Integer: 12
         * 07:50:26 [AgentRspDump]       [ 0]   [ 1]   [ 0]   [ 1]    Integer: 1
         * 07:50:26 [AgentRspDump]       [ 0]   [ 1]   [ 1]  Tuple: 2 elements
         * 07:50:26 [AgentRspDump]       [ 0]   [ 1]   [ 1]   [ 0]    Integer: 13
         * 07:50:26 [AgentRspDump]       [ 0]   [ 1]   [ 1]   [ 1]    Integer: 2
         * 07:50:26 [AgentRspDump]       [ 0]   [ 1]   [ 2]  Tuple: 2 elements
         * 07:50:26 [AgentRspDump]       [ 0]   [ 1]   [ 2]   [ 0]    Integer: 34
         * 07:50:26 [AgentRspDump]       [ 0]   [ 1]   [ 2]   [ 1]    Integer: 15
         * 07:50:26 [AgentRspDump]       [ 0]   [ 1]   [ 3]  Tuple: 2 elements
         * 07:50:26 [AgentRspDump]       [ 0]   [ 1]   [ 3]   [ 0]    Integer: 56
         * 07:50:26 [AgentRspDump]       [ 0]   [ 1]   [ 3]   [ 1]    Integer: 12
         * 07:50:26 [AgentRspDump]       [ 1] Object:
         * 07:50:26 [AgentRspDump]       [ 1]   Type:     String: 'util.KeyVal'
         * 07:50:26 [AgentRspDump]       [ 1]   Args:  Dictionary: 4 entries
         * 07:50:26 [AgentRspDump]       [ 1]   Args:   [ 0]   Key:     String: 'missionQuit'
         * 07:50:26 [AgentRspDump]       [ 1]   Args:   [ 0] Value:    Boolean: false
         * 07:50:26 [AgentRspDump]       [ 1]   Args:   [ 1]   Key:     String: 'missionCompleted'
         * 07:50:26 [AgentRspDump]       [ 1]   Args:   [ 1] Value:    Boolean: false
         * 07:50:26 [AgentRspDump]       [ 1]   Args:   [ 2]   Key:     String: 'missionDeclined'
         * 07:50:26 [AgentRspDump]       [ 1]   Args:   [ 2] Value:    Boolean: false
         * 07:50:26 [AgentRspDump]       [ 1]   Args:   [ 3]   Key:     String: 'loyaltyPoints'
         * 07:50:26 [AgentRspDump]       [ 1]   Args:   [ 3] Value:    Integer: 10
         *
         */
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
 *
 * --- response to job complete
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

PyResult AgentBound::Handle_GetMissionBriefingInfo(PyCallArgs &call) {
    // called from iniate agent convo... should be populated when mission available
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

    if (!m_agent->HasMission())
        return nullptr;

    PyDict* keywords = new PyDict();
        keywords->SetItemString("objectiveLocationID", new PyInt(m_agent->GetStationID()));
        keywords->SetItemString("objectiveLocationSystemID", new PyInt(m_agent->GetSystemID()));
        keywords->SetItemString("objectiveTypeID", new PyInt(2631));
        keywords->SetItemString("objectiveQuantity", new PyInt(7));
        keywords->SetItemString("objectiveDestinationID", new PyInt(60001690));
        keywords->SetItemString("objectiveDestinationSystemID", new PyInt(30002777));
        keywords->SetItemString("rewardTypeID", new PyInt(29));
        keywords->SetItemString("rewardQuantity", new PyInt(28000));
    PyDict *dialog = new PyDict();
        dialog->SetItemString("ContentID", new PyInt(57858));  // not sure when/if this is filled
        dialog->SetItemString("Mission Keywords", keywords);  // only used when "ContentID" is filled?
        dialog->SetItemString("Decline Time", new PyInt(-1));   // -1 is generic decline msg
        dialog->SetItemString("Mission Image", new PyString("<img src='res:/UI/netres/mission_content/couriermission.png' align=center hspace=4 vspace=4>") );
        //dialog->SetItemString("Mission Title", new PyString("Human Problem") );
        //dialog->SetItemString("Mission Briefing", new PyString("<div id=basetext>Our cargo area and lower-tier decks are overflowing with vagabonds and tramps.  It has almost reached epidemic proportions.  These people simply refuse to leave, some are laid off workers of ours while others are simply outsiders who think they can come and live off our hospitality indefinitely.  Since there are strict regulations against sending them out the airlock for no reason, and since we can't simply frame every single one of them for a crime worthy of such a fate, we've no choice but to forcibly move them somewhere else.  Another station will do.  In fact I'd like you to move a heap-load of them to Isutaka I - Caldari Steel Warehouse.  I never liked those bastards over there anyway, let them deal with this 'human problem'.</div>    <br>") );
        //dialog->SetItemString("Expiration Message", new PyString("<span id=subheader>Mission Expiration</span><br><div id=basetext>This mission expires at 2011.05.10 01:29:33</div>"));
        dialog->SetItemString("Expiration Time", new PyLong( GetFileTimeNow()+Win32Time_Day ) );
        dialog->SetItemString("Mission Title ID", new PyInt(57858) );  // this 'value' will display on initial convo window, under agent picture, if sent with DoAction = 0
        dialog->SetItemString("Mission Briefing ID", new PyInt(130987) ); // or 130990  same data

    if (is_log_enabled(AGENT__RSPDUMP))
        dialog->Dump(AGENT__RSPDUMP, "    ");

    return dialog;
    /*
        [CallRsp PySubStream]
            [PyDict 7 kvp]
                Key:[PyString "Mission Keywords"]
                ==Value:[PyDict 8 kvp]
                            Key:[PyString "objectiveLocationID"]
                            ==Value:[PyInt 60007390]
                            Key:[PyString "objectiveDestinationID"]
                            ==Value:[PyInt 60007531]
                            Key:[PyString "objectiveQuantity"]
                            ==Value:[PyInt 7]
                            Key:[PyString "objectiveDestinationSystemID"]
                            ==Value:[PyInt 30003555]
                            Key:[PyString "objectiveTypeID"]
                            ==Value:[PyInt 2631]
                            Key:[PyString "objectiveLocationSystemID"]
                            ==Value:[PyInt 30003558]
                            Key:[PyString "rewardTypeID"]
                            ==Value:[PyInt 29]
                            Key:[PyString "rewardQuantity"]
                            ==Value:[PyIntegerVar 28000]
                Key:[PyString "Mission Title ID"]
                ==Value:[PyInt 57117]
                Key:[PyString "Decline Time"]
                ==Value:[PyInt -1]
                Key:[PyString "Expiration Time"]
                ==Value:[PyIntegerVar 131270978560454574]
                Key:[PyString "Mission Briefing ID"]
                ==Value:[PyInt 145160]
                Key:[PyString "ContentID"]
                ==Value:[PyNone]
                Key:[PyString "Mission Image"]
                ==Value:[PyString "<img src="res:/UI/netres/mission_content/couriermission.png" align=center hspace=4 vspace=4>"]
    Named Payload:
        [PyNone]
     */
}

PyResult AgentBound::Handle_GetMissionObjectiveInfo(PyCallArgs &call)
{
    //ret = self.GetAgentMoniker(agentID).GetMissionObjectiveInfo(charID, contentID)

    _log(AGENT__DUMP,  "AgentBound::Handle_GetMissionObjectiveInfo() - size= %u", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    if (call.tuple->size() == 0)
        if (!m_agent->HasMission())
            return nullptr;

        /*  not right....
    Call_SingleArg args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: failed to decode arguments", call.client->GetName());
        return nullptr;
    }

    uint32 missionID = PyRep::IntegerValue(args.arg);
    if (missionID == 0)
        return PyStatic.NewNone();
    */

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
    return nullptr;
}
/*)
 * 07:50:33 [AgentDump] AgentBound::Handle_DoAction() - size= 1
 * 07:50:33 [AgentDump]   Call Arguments:
 * 07:50:33 [AgentDump]      Tuple: 1 elements
 * 07:50:33 [AgentDump]       [ 0]    Integer: 13
 * 07:50:33 [AgentDump]  Named Arguments:
 * 07:50:33 [AgentDump]   machoVersion
 * 07:50:33 [AgentDump]        Integer: 1
 * 07:50:33 [AgentRspDump]      Tuple: 2 elements
 * 07:50:33 [AgentRspDump]       [ 0]  Tuple: 2 elements
 * 07:50:33 [AgentRspDump]       [ 0]   [ 0]  Tuple: 2 elements
 * 07:50:33 [AgentRspDump]       [ 0]   [ 0]   [ 0]     String: 'This just came up, allan.'
 * 07:50:33 [AgentRspDump]       [ 0]   [ 0]   [ 1]       None
 * 07:50:33 [AgentRspDump]       [ 0]   [ 1]   List: 3 elements
 * 07:50:33 [AgentRspDump]       [ 0]   [ 1]   [ 0]  Tuple: 2 elements
 * 07:50:33 [AgentRspDump]       [ 0]   [ 1]   [ 0]   [ 0]    Integer: 23
 * 07:50:33 [AgentRspDump]       [ 0]   [ 1]   [ 0]   [ 1]    Integer: 3
 * 07:50:33 [AgentRspDump]       [ 0]   [ 1]   [ 1]  Tuple: 2 elements
 * 07:50:33 [AgentRspDump]       [ 0]   [ 1]   [ 1]   [ 0]    Integer: 45
 * 07:50:33 [AgentRspDump]       [ 0]   [ 1]   [ 1]   [ 1]    Integer: 9
 * 07:50:33 [AgentRspDump]       [ 0]   [ 1]   [ 2]  Tuple: 2 elements
 * 07:50:33 [AgentRspDump]       [ 0]   [ 1]   [ 2]   [ 0]    Integer: 67
 * 07:50:33 [AgentRspDump]       [ 0]   [ 1]   [ 2]   [ 1]    Integer: 10
 * 07:50:33 [AgentRspDump]       [ 1] Object:
 * 07:50:33 [AgentRspDump]       [ 1]   Type:     String: 'util.KeyVal'
 * 07:50:33 [AgentRspDump]       [ 1]   Args:  Dictionary: 4 entries
 * 07:50:33 [AgentRspDump]       [ 1]   Args:   [ 0]   Key:     String: 'missionQuit'
 * 07:50:33 [AgentRspDump]       [ 1]   Args:   [ 0] Value:    Boolean: false
 * 07:50:33 [AgentRspDump]       [ 1]   Args:   [ 1]   Key:     String: 'missionCompleted'
 * 07:50:33 [AgentRspDump]       [ 1]   Args:   [ 1] Value:    Boolean: false
 * 07:50:33 [AgentRspDump]       [ 1]   Args:   [ 2]   Key:     String: 'missionDeclined'
 * 07:50:33 [AgentRspDump]       [ 1]   Args:   [ 2] Value:    Boolean: false
 * 07:50:33 [AgentRspDump]       [ 1]   Args:   [ 3]   Key:     String: 'loyaltyPoints'
 * 07:50:33 [AgentRspDump]       [ 1]   Args:   [ 3] Value:    Integer: 10
 * 07:50:33 [SvcCall] Service AgentBound::GetMissionBriefingInfo()
 * 07:50:33 [AgentDump] AgentBound::Handle_GetMissionBriefingInfo() - size= 0
 * 07:50:33 [AgentDump]   Call Arguments:
 * 07:50:33 [AgentDump]      Tuple: Empty
 * 07:50:33 [AgentDump]  Named Arguments:
 * 07:50:33 [AgentDump]   machoVersion
 * 07:50:33 [AgentDump]        Integer: 1
 * 07:50:33 [AgentRspDump]      Dictionary: 7 entries
 * 07:50:33 [AgentRspDump]       [ 0]   Key:     String: 'Mission Briefing ID'
 * 07:50:33 [AgentRspDump]       [ 0] Value:    Integer: 130987
 * 07:50:33 [AgentRspDump]       [ 1]   Key:     String: 'Mission Title ID'
 * 07:50:33 [AgentRspDump]       [ 1] Value:    Integer: 57858
 * 07:50:33 [AgentRspDump]       [ 2]   Key:     String: 'Expiration Time'
 * 07:50:33 [AgentRspDump]       [ 2] Value:       Long: 131746638334523632
 * 07:50:33 [AgentRspDump]       [ 3]   Key:     String: 'Mission Image'
 * 07:50:33 [AgentRspDump]       [ 3] Value:     String: '<img src='res:/UI/netres/mission_content/couriermission.png' align=center hspace=4 vspace=4>'
 * 07:50:33 [AgentRspDump]       [ 4]   Key:     String: 'Decline Time'
 * 07:50:33 [AgentRspDump]       [ 4] Value:    Integer: -1
 * 07:50:33 [AgentRspDump]       [ 5]   Key:     String: 'Mission Keywords'
 * 07:50:33 [AgentRspDump]       [ 5] Value:  Dictionary: 8 entries
 * 07:50:33 [AgentRspDump]       [ 5] Value:   [ 0]   Key:     String: 'rewardQuantity'
 * 07:50:33 [AgentRspDump]       [ 5] Value:   [ 0] Value:    Integer: 28000
 * 07:50:33 [AgentRspDump]       [ 5] Value:   [ 1]   Key:     String: 'objectiveDestinationSystemID'
 * 07:50:33 [AgentRspDump]       [ 5] Value:   [ 1] Value:    Integer: 30002777
 * 07:50:33 [AgentRspDump]       [ 5] Value:   [ 2]   Key:     String: 'objectiveDestinationID'
 * 07:50:33 [AgentRspDump]       [ 5] Value:   [ 2] Value:    Integer: 60001690
 * 07:50:33 [AgentRspDump]       [ 5] Value:   [ 3]   Key:     String: 'rewardTypeID'
 * 07:50:33 [AgentRspDump]       [ 5] Value:   [ 3] Value:    Integer: 29
 * 07:50:33 [AgentRspDump]       [ 5] Value:   [ 4]   Key:     String: 'objectiveQuantity'
 * 07:50:33 [AgentRspDump]       [ 5] Value:   [ 4] Value:    Integer: 7
 * 07:50:33 [AgentRspDump]       [ 5] Value:   [ 5]   Key:     String: 'objectiveTypeID'
 * 07:50:33 [AgentRspDump]       [ 5] Value:   [ 5] Value:    Integer: 2631
 * 07:50:33 [AgentRspDump]       [ 5] Value:   [ 6]   Key:     String: 'objectiveLocationSystemID'
 * 07:50:33 [AgentRspDump]       [ 5] Value:   [ 6] Value:    Integer: 30002507
 * 07:50:33 [AgentRspDump]       [ 5] Value:   [ 7]   Key:     String: 'objectiveLocationID'
 * 07:50:33 [AgentRspDump]       [ 5] Value:   [ 7] Value:    Integer: 60005728
 * 07:50:33 [AgentRspDump]       [ 6]   Key:     String: 'ContentID'
 * 07:50:33 [AgentRspDump]       [ 6] Value:    Integer: 57858
 * 07:50:33 [SvcCall] Service AgentBound::GetAgentLocationWrap()
 * 07:50:33 [SvcCall] Service AgentBound::GetMissionObjectiveInfo()
 * 07:50:33 [AgentDump] AgentBound::Handle_GetMissionObjectiveInfo() - size= 0
 * 07:50:33 [AgentDump]   Call Arguments:
 * 07:50:33 [AgentDump]      Tuple: Empty
 * 07:50:33 [AgentDump]  Named Arguments:
 * 07:50:33 [AgentDump]   machoVersion
 * 07:50:33 [AgentDump]        Integer: 1
 * 07:50:33 [SvcCall] Service alert::BeanCount()
 * 07:50:33 [SvcCall] Service alert::SendClientStackTraceAlert()
 * EXCEPTION #5 logged at  06/27/2018 7:50:33 Unhandled exception in <TaskletExt object at 45e665b0, abps=1001, ctxt=None>
 * Caught at:
 * /common/lib/bluepy.py(98) CallWrapper
 * Thrown at:
 * /common/lib/bluepy.py(86) CallWrapper
 * /../carbon/client/script/ui/control/buttons.py(245) OnClick
 * /client/script/ui/station/agents/agents.py(580) DoAction
 * /client/script/ui/station/agents/agents.py(1157) __Interact
 *        agentDialogueWindow = form.AgentDialogueWindow object at 0x49c68710, name=agentinteraction_3012674, destroyed=False>
 *        briefingInformation = {'ContentID': 57858,
 *                        'Decline Time': -1,
 *                        'Expiration Time': 131746638334523632L,
 *                        'Mission Briefing ID': 130987,
 *                       ...
 *        charSays = ''
 *        disabledButtons = []
 *        actionID = 13
 *        appendCloseButton = False
 *        self = <svc.Agents instance at 0x48A75E18>
 *        agentLocationWrap = {'locationID': 60005728, 'solarsystemID': 30002507, 'typeID': 2497}
 *        ret = "<div id=basetext>Our cargo area and lower-tier decks are overflowing with vagabonds and tramps.  It has almost reached epidemic proportions.  These people simply refuse to leave, some are laid off workers of ours while others are simply outsiders who think they can come and live off our hospitality indefinitely.  Since there are strict regulations against sending them out the airlock for no reason, and since we can't simply frame every single one of them for a crime worthy of such a fate, we've no choice but to forcibly move them somewhere else.  Another station will do.  In fact I'd like you to move a heap-load of them to Isutaka I - Caldari Steel Warehouse.  I never liked those bastards over there anyway, let them deal with this 'human problem'.</div>    <br>    "
 *        label = u'Delay'
 *        html = u'\n            <html>\n            <head>\n                <link rel="stylesheet" type="text/css" href="res:/ui/css/agentconvo.css">\n            </head>\n                <body background-color=#00000000 link=#ffa800>\n                    \n        <table border=0 cellpadding=1 cellspacing=1>\n            <tr>\n                <td valign=top >\n                    <table border=0 cellpadding=1 cellspacing=1>\n                        <tr>\n                        </tr>\n                        <tr>\n                        </tr>\n                        <tr>\n                        </tr>\n                        <tr>\n                            <td valign=top><img src="portrait:3012674" width=120 height=120 size=256 align=left style=margin-right:10></td>\n                        </tr>\n                    </table>\n                </td>\n                <td valign=top>\n                    <table border=0 width=290 cellpadding=1 cellspacing=1>\n                        <tr>\n                            <td w...
 *        lp = 10
 *        missionTitle = u'<BR><span id=subheader>Human Problem</span><BR>'
 *        blurbDivision = u'Division: Mining'
 *        customAgentButtons = {'args': [...], 'okFunc': [...], 'okLabel': [...]}
 *        dialogue = [(...), (...), (...)]
 *        agentCorpID = 1000059
 *        numButtons = 3
 *        agentDivisionID = 23
 *        extraMissionInfo = u"<BR>Declining a mission from a particular agent more than once every 4 hours will result in a loss of standing with that agent.<br><center><img src='res:/UI/netres/mission_content/couriermission.png' align=center hspace=4 vspace=4></center>"
 *        adminOptions = []
 *        a = <Row agentID:3012674,agentTypeID:2,divisionID:23,level:2,quality:20,corporationID:1000059,stationID:60005728,gender:0,bloodlineID:2,factionID:500002,solarsystemID:30002507>
 *        divisions = <util.IndexRowset instance at 0x49A78238>
 *        objectiveHtml = None
 *        extraInfo = <Anonymous KeyVal: {'missionCompleted': False, 'missionQuit': False, 'loyaltyPoints': 10, 'missionDeclined': False}>
 *        agentSays = 'This just came up, allan.'
 *        agentInfoIcon = u'<a href=showinfo:1375//3012674><img src=icon:38_208 size=16 alt="Show Info"></a>'
 *        adminBlock = ''
 *        isAgentInteractionMission = False
 *        each = (67, 10)
 *        numDialogChoices = 0
 *        closeWindowOnClick = True
 *        initialContentID = None
 *        closeWindowAfterInteraction = False
 *        labelPath = 'UI/Agents/Dialogue/Buttons/DeferMission'
 * AttributeError: KeyVal instance has no attribute '__getitem__'
 */

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

    /*
        [CallRsp PySubStream]
            [PyTuple 2 items]
                [PyList 1 items]
                    [PyTuple 10 items]
                        [PyInt 1]
                        [PyInt 0]
                        [PyString "UI/Agents/MissionTypes/Courier"]
                        [PyInt 57117]
                        [PyInt 3013251]
                        [PyIntegerVar 131276594538586776]
                        [PyList 0 items]
                        [PyBool False]
                        [PyBool False]
                        [PyInt 13773]
                [PyList 0 items]
        */

    return nullptr;
}
