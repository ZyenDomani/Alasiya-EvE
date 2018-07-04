
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
    PyTuple* agentSays = new PyTuple(2);
    PyList* dialog = new PyList();

    if (actionID == 0) {
        // response as string for custom data.  response as pyint to use client data (using getlocale shit)
        // default initial agent response based on agent location, level, bloodline, quality, and char/agent standings
        //  this will be modeled after UO speech data, in tiers and levels.
        // if RequestMission is only option, this is ignored.  see note under 'dialog data'
        response = "Why the fuck am I looking at you again, ";
        response += call.client->GetCharacterName().c_str();
        response += "?";
        agentSays->SetItem(0, new PyString(response));  //msgInfo  -- if tuple[0].string then return msgInfo
        agentSays->SetItem(1, new PyNone());      // ContentID  -- PyNone used when msgInfo is string (mostly for initial greetings)

        m_agent->SetMission(false);
        // dialogue data.  if RequestMission is only option, client auto-responds with DoAction(RequestMission optionID)
        //  if char has current mission wiht this agent, add this one.
        /*
        PyTuple* button1 = new PyTuple(2);
            button1->SetItem(0, new PyInt(12)); // this are buttonIDs which are unique and sequential to each agent, regardless of chars
            button1->SetItem(1, new PyInt(Dialog::Button::ViewMission));
        dialog->AddItem(button1);
            */
        PyTuple* button2 = new PyTuple(2);
            button2->SetItem(0, new PyInt(13));
            button2->SetItem(1, new PyInt(Dialog::Button::RequestMission));
        dialog->AddItem(button2);
        /*
        // if agent does location, add this one...
        PyTuple* button3 = new PyTuple(2);
            button3->SetItem(0, new PyInt(34));
            button3->SetItem(1, new PyInt(Dialog::Button::LocateCharacter));
        dialog->AddItem(button3);
        // if agent also does research, add this one...
        PyTuple* button4 = new PyTuple(2);
            button4->SetItem(0, new PyInt(56));
            button4->SetItem(1, new PyInt(Dialog::Button::StartResearch));
        dialog->AddItem(button4);
        */
    } else {
        // agentSays data.....this is for custom shit here...
        /*
        response = "This just came up, ";
        response += call.client->GetCharacterName().c_str();
        response += ".";
        response += "<div id=basetext>Our cargo area and lower-tier decks are overflowing with vagabonds and tramps.  It has almost reached epidemic proportions. ";
        response += "These people simply refuse to leave, some are laid off workers of ours while others are simply outsiders who think they can come and live off our hospitality indefinitely. ";
        response += "Since there are strict regulations against sending them out the airlock for no reason, and since we can't simply frame every single one of them for a crime worthy of such a fate, we've no choice but to forcibly move them somewhere else. ";
        response += "Another station will do.  In fact I'd like you to move a heap-load of them to Isutaka I - Caldari Steel Warehouse.  I never liked those bastards over there anyway, let them deal with this 'human problem'.</div><br>";
        */
        m_agent->SetMission(true);

        /*
        agentSays, dialogue, extraInfo = self.__GetConversation(agentDialogueWindow, actionID)

        tmp = wnd.sr.agentMoniker.DoAction(actionID)
        ret, wnd.sr.oob = tmp
        agentSays, wnd.sr.dialogue = ret

        wnd.sr.agentSays = self.ProcessMessage(agentSays, wnd.sr.agentID)
          */

    //  this will get complicated and is based on agent/char interaction
    /*
     *    def ProcessMessage(self, msg, agentID = None):
     *        if isinstance(msg, types.TupleType):
     *            msgInfo, contentID = msg
     *            if isinstance(msgInfo, basestring):
     *                return msgInfo
     *            if contentID is not None:
     *                if contentID not in self.missionArgs:
     *                    if agentID is None:
     *                        raise RuntimeError('Agent message received a content ID without an agent ID. Something is wrong!')
     *                    self.missionArgs[contentID] = self.GetAgentMoniker(agentID).GetMissionKeywords(contentID)
     *                msgArgs = self.missionArgs[contentID]
     *            else:
     *                msgArgs = {}
     *            if agentID is not None:
     *                if agentID not in self.agentArgs:
     *                    self.agentArgs[agentID] = self.GetAgentArgs(agentID)
     *                msgArgs.update(self.agentArgs[agentID])
     *            if isinstance(msgInfo, tuple):
     *                for k in msgInfo[1]:
     *                    if k in ('missionCompletionText', 'missionOfferText', 'missionBriefingText', 'locationString'):
     *                        msgInfo[1][k] = self.ProcessMessage((msgInfo[1][k], contentID), agentID)
     *
     *                msgArgs.update(msgInfo[1])
     *                try:
     *                    return localization.GetByLabel(msgInfo[0], **msgArgs)
     *                except:
     *                    log.LogException('Error parsing message with label %s' % msgInfo[0])
     *                    return localization.GetByLabel('UI/Agents/Dialogue/StandardMission/CorruptBriefing')
     *
     *            else:
     *                try:
     *                    return localization.GetByMessageID(msgInfo, **msgArgs)
     *                except:
     *                    log.LogException('Error parsing agent message with ID %s' % msgInfo)
     *                    return localization.GetByLabel('UI/Agents/Dialogue/StandardMission/CorruptBriefing') + '<br>----------------------<br>' + localization._GetRawByMessageID(msgInfo)
     *
     *        else:
     *            return msg
     */
    //   detail in /eve/client/script/ui/station/agents/agents.py

        // offer and msgInfo are only used when agentSays[0] != string.
        PyList* msgInfoList = new PyList();
        /*
         *    PyDict* offer = new PyDict();
         *        offer->SetItemString("missionOfferText", new PyInt(235567));
         *        msgInfoList->AddItem(new PyObject("util.KeyVal", offer));
         */
        PyDict* briefingInfo = new PyDict();
        briefingInfo->SetItemString("missionBriefingText", new PyInt(235548));
        msgInfoList->AddItem(new PyObject("util.KeyVal", briefingInfo));
        /*
         *    PyDict* completion = new PyDict();
         *        completion->SetItemString("missionCompletionText", new PyInt(235571));
         *        msgInfoList->AddItem(new PyObject("util.KeyVal", completion));
         */
        /*
         *    PyDict* location = new PyDict();
         *        location->SetItemString("locationString", new PyInt(235546));
         *        msgInfoList->AddItem(new PyObject("util.KeyVal", location));
         */

        PyTuple* msgInfo = new PyTuple(2);
        msgInfo->SetItem(0, new PyInt(236729));     // agent greeting?
        msgInfo->SetItem(1, msgInfoList);
        //TypeError: cannot convert dictionary update sequence element #0 to a sequence     ** no clue what this means....

        agentSays->SetItem(0, /*msgInfo*/new PyInt(236729));       // PyInt of agent greeting (under mission name) unless offer is PyDict..  cant get msgInfo working yet.
        agentSays->SetItem(1, new PyInt(130997));    //contentID    - mission descriptionID


        PyTuple* button1 = new PyTuple(2);
            button1->SetItem(0, new PyInt(23));
            button1->SetItem(1, new PyInt(Dialog::Button::Accept));
        PyTuple* button2 = new PyTuple(2);
            button2->SetItem(0, new PyInt(45));
            button2->SetItem(1, new PyInt(Dialog::Button::Decline));
        PyTuple* button3 = new PyTuple(2);
            button3->SetItem(0, new PyInt(67));
            button3->SetItem(1, new PyInt(Dialog::Button::Defer));

        dialog->AddItem(button1);
        dialog->AddItem(button2);
        dialog->AddItem(button3);
    }

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

    if (is_log_enabled(AGENT__RSPDUMP)) {
        _log(AGENT__RSPDUMP, "AgentBound::Handle_DoAction RSP:" );
        outer->Dump(AGENT__RSPDUMP, "    ");
    }

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
        dialog->SetItemString("ContentID", new PyInt(130997));  // not sure when/if this is filled
        dialog->SetItemString("Mission Keywords", keywords);  // only used when "ContentID" is filled?
        dialog->SetItemString("Decline Time", new PyInt(-1));   // -1 is generic decline msg
        // will have to find and store mission images *somewhere*  EVE_Mission.h probably.
        dialog->SetItemString("Mission Image", new PyString("<img src='res:/UI/netres/mission_content/couriermission.png' align=center hspace=4 vspace=4>") );
        //dialog->SetItemString("Mission Title", new PyString("Human Problem") );
        //dialog->SetItemString("Mission Briefing", new PyString("<div id=basetext>Our cargo area and lower-tier decks are overflowing with vagabonds and tramps.  It has almost reached epidemic proportions.  These people simply refuse to leave, some are laid off workers of ours while others are simply outsiders who think they can come and live off our hospitality indefinitely.  Since there are strict regulations against sending them out the airlock for no reason, and since we can't simply frame every single one of them for a crime worthy of such a fate, we've no choice but to forcibly move them somewhere else.  Another station will do.  In fact I'd like you to move a heap-load of them to Isutaka I - Caldari Steel Warehouse.  I never liked those bastards over there anyway, let them deal with this 'human problem'.</div>    <br>") );
        //dialog->SetItemString("Expiration Message", new PyString("<span id=subheader>Mission Expiration</span><br><div id=basetext>This mission expires at 2011.05.10 01:29:33</div>"));
        dialog->SetItemString("Expiration Time", new PyLong( GetFileTimeNow()+Win32Time_Day ) );
        dialog->SetItemString("Mission Title ID", new PyInt(55205) );  // this 'value' will display on initial convo window, under agent picture, if sent with DoAction = 0
        dialog->SetItemString("Mission Briefing ID", new PyInt(130997) );

    if (is_log_enabled(AGENT__RSPDUMP)) {
        _log(AGENT__RSPDUMP, "AgentBound::Handle_GetMissionBriefingInfo() RSP:" );
        dialog->Dump(AGENT__RSPDUMP, "    ");
    }

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
    // sends charID, contentID
    // returns PyDict loaded with mission info  or PyNone
    // this one is where the "double pane" agent window is set.

    /*  ret = self.GetAgentMoniker(agentID).GetMissionObjectiveInfo(charID, contentID)
     *
     *  if ret:
            objectiveHtml = '\n                        <html>\n                        <head>\n                            <link rel="stylesheet" type="text/css" href="res:/ui/css/missionobjectives.css">\n                        </head>\n                        <body>\n                    '
            objectiveHtml += agentDialogueUtil.BuildObjectiveHTML(agentDialogueWindow.sr.agentID, ret)
            objectiveHtml += '</body></html>'
            agentDialogueWindow.SetDoublePaneView(objectiveHtml=objectiveHtml)
        else:
            agentDialogueWindow.SetSinglePaneView()



def BuildObjectiveHTML(agentID, objectiveData):
    html = ''
    if objectiveData.get('importantStandings', 0):
        html += '<span id=ip>%s</span><br><br>' % localization.GetByLabel('UI/Agents/StandardMission/ImportantStandingsWarning')
    cmpStatus = objectiveData['completionStatus']
    if isinstance(objectiveData['missionTitleID'], basestring):
        missionName = objectiveData['missionTitleID']
    else:
        missionName = localization.GetByMessageID(objectiveData['missionTitleID'])
    if cmpStatus > 0:
        missionHeaderColor = '<font color=#5ABA56>'
        missionHeader = localization.GetByLabel('UI/Agents/StandardMission/MissionObjectivesComplete', missionName=missionName)
    else:
        missionHeaderColor = '<font>'
        missionHeader = localization.GetByLabel('UI/Agents/StandardMission/MissionObjectives', missionName=missionName)
    if cmpStatus == 2:
        gmStatusHeader = '<font color=#00FF00>Debug Mode: Cheat Complete Enabled</div></font>'
    else:
        gmStatusHeader = ''
    objectives = ''
    for objType, objData in objectiveData['objectives']:
        objectives += ProcessObjectiveEntry(objType, objData)

    for dunData in objectiveData['dungeons']:
        objectives += ProcessDungeonData(dunData, agentID)

    html += '\n        %(GMStatusHeader)s\n        <span id=subheader>%(missionHeaderColor)s%(missionHeader)s</font></span><br>\n        <div id=basetext>%(objectivesHeader)s<br>\n        <br>\n        <span id=basetext>\n        %(objectives)s\n        </span>\n        <br>\n    ' % {'GMStatusHeader': gmStatusHeader,
     'missionHeader': missionHeader,
     'missionHeaderColor': missionHeaderColor,
     'objectivesHeader': localization.GetByLabel('UI/Agents/StandardMission/OverviewAndObjectivesBlurb'),
     'objectives': objectives}
    secWarning = sm.GetService('agents').GetSecurityWarning(objectiveData['locations'])
    if secWarning:
        html += '<font color=red>%s</font><br><br>' % secWarning

    def ProcessEntry(typeID, quantity, extra):
        if util.IsCharacter(typeID):
            iconWrap = OwnerWrap(typeID)
            description = localization.GetByLabel('UI/Agents/StandardMission/MissionReferral', agentID=typeID)
        else:
            iconWrap = IconWrap(typeID, extra)
            description = ProcessTypeAndQuantity(typeID, quantity, extra)
        return (iconWrap, description)

    if len(objectiveData['agentGift']) > 0:
        if objectiveData['missionState'] in (const.agentMissionStateAccepted, const.agentMissionStateFailed):
            grantedItemsDetail = localization.GetByLabel('UI/Agents/StandardMission/AcceptedGrantedItemDetail')
        else:
            grantedItemsDetail = localization.GetByLabel('UI/Agents/StandardMission/GrantedItemDetail')
        html += '<br>\n            <span id=subheader>%s</span>\n            <div id=basetext>%s</div>\n            <div><table>\n        ' % (localization.GetByLabel('UI/Agents/StandardMission/GrantedItems'), grantedItemsDetail)
        for typeID, quantity, extra in objectiveData['agentGift']:
            icon, description = ProcessEntry(typeID, quantity, extra)
            html += '\n                <tr valign=middle>\n                    <td width=36>%s</td>\n                    <td width=352>%s</td>\n                </tr>\n                ' % (icon, description)

        html += '</table></div><br>'
    if len(objectiveData['normalRewards']) or objectiveData['loyaltyPoints'] > 0 or objectiveData['researchPoints'] > 0:
        html += '\n            <span id=subheader>%s</span>\n            <div id=basetext>%s</div>\n            <div><table>\n        ' % (localization.GetByLabel('UI/Agents/StandardMission/RewardsTitle'), localization.GetByLabel('UI/Agents/StandardMission/RewardsHeader'))
        for typeID, quantity, extra in objectiveData['normalRewards']:
            icon, description = ProcessEntry(typeID, quantity, extra)
            html += '\n                <tr valign=middle>\n                    <td width=36>%s</td>\n                    <td width=352>%s</td>\n                </tr>\n            ' % (icon, description)

        if objectiveData['loyaltyPoints'] > 0:
            loyaltyPointsIcon = IconWrap(const.typeLoyaltyPoints)
            loyaltyPoints = objectiveData['loyaltyPoints']
            html += '\n                <tr valign=middle>\n                    <td width=36>%s</td>\n                    <td width=352>%s</td>\n                </tr>\n            ' % (loyaltyPointsIcon, localization.GetByLabel('UI/Agents/StandardMission/NumLoyaltyPoints', lpAmount=loyaltyPoints))
        if objectiveData['researchPoints'] > 0:
            researchPointsIcon = IconWrap(const.typeResearch)
            researchPoints = round(objectiveData['researchPoints'], 0)
            html += '\n                <tr valign=middle>\n                    <td width=36>%s</td>\n                    <td width=352>%s</td>\n                </tr>\n            ' % (researchPointsIcon, localization.GetByLabel('UI/Agents/StandardMission/NumResearchPoints', rpAmount=researchPoints))
        html += '</table></div><br>'
    if len(objectiveData['bonusRewards']) > 0:
        html += '<span id=subheader>%s</span><br>' % localization.GetByLabel('UI/Agents/StandardMission/BonusRewardsTitle')
        for timeRemaining, typeID, quantity, extra in objectiveData['bonusRewards']:
            if timeRemaining > 0:
                header = localization.GetByLabel('UI/Agents/StandardMission/BonusRewardsHeader', timeRemaining=timeRemaining)
            else:
                header = localization.GetByLabel('UI/Agents/StandardMission/BonusTimePassed')
            icon, description = ProcessEntry(typeID, quantity, extra)
            html += '\n                <div id=basetext>%s<br>\n                <div><table>\n                    <tr valign=middle>\n                        <td width=36>%s</TD>\n                        <td width=352>%s</TD>\n                    </tr>\n                </table></div>\n            ' % (header, icon, description)

        html += '<br>'
    if len(objectiveData['collateral']) > 0:
        html += '\n            <span id=subheader>%s</span>\n            <div id=basetext>%s</div><br>\n            <div><table>\n        ' % (localization.GetByLabel('UI/Agents/StandardMission/CollateralTitle'), localization.GetByLabel('UI/Agents/StandardMission/CollateralHeader'))
        for typeID, quantity, extra in objectiveData['collateral']:
            collateralIcon = IconWrap(typeID, extra)
            collateralDescription = ProcessTypeAndQuantity(typeID, quantity, extra)
            html += '\n                <tr valign=middle>\n                    <td width=36>%s</td>\n                    <td width=352>%s</td>\n                </tr>\n            ' % (collateralIcon, collateralDescription)

        html += '</table></div><br>'
    if 'missionExtra' in objectiveData:
        headerID, bodyID = objectiveData['missionExtra']
        html += '\n            <span id=subheader>%s</span>\n            <div id=basetext>%s</div>\n        ' % (sm.GetService('agents').ProcessMessage((headerID, objectiveData['contentID']), agentID), sm.GetService('agents').ProcessMessage((bodyID, objectiveData['contentID']), agentID))
    return html


     */

    _log(AGENT__DUMP,  "AgentBound::Handle_GetMissionObjectiveInfo() - size= %u", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    if (call.tuple->size() == 0)
        if (!m_agent->HasMission())
            return PyStatic.NewNone();

    PyDict* info = new PyDict();
    info->SetItemString("contentID", new PyInt());
    info->SetItemString("importantStandings", new PyInt());
    info->SetItemString("completionStatus", new PyInt());
    info->SetItemString("missionTitleID", new PyInt());
    info->SetItemString("locations", new PyInt());
    info->SetItemString("agentGift", new PyInt());
    info->SetItemString("missionState", new PyInt());
    info->SetItemString("normalRewards", new PyInt());
    info->SetItemString("loyaltyPoints", new PyInt(0));
    info->SetItemString("researchPoints", new PyInt(0));
    info->SetItemString("bonusRewards", new PyInt());   // this is tuple(4)  timeRemaining, typeID, quantity, extra
    info->SetItemString("collateral", new PyTuple(0));   // this is tuple(3)  typeID, quantity, extra
    info->SetItemString("missionExtra", new PyTuple(0));       // this is tuple(2)  headerID, bodyID
    info->SetItemString("objectives", new PyTuple(0));     // this is tuple(2)    objType, objData
    info->SetItemString("dungeons", new PyTuple(0)); // this is tuple(1)   dunData

    if (is_log_enabled(AGENT__RSPDUMP)) {
        _log(AGENT__RSPDUMP, "AgentBound::Handle_GetMissionObjectiveInfo() RSP:" );
        info->Dump(AGENT__RSPDUMP, "    ");
    }

    return info;
}

PyResult AgentBound::Handle_GetMissionKeywords(PyCallArgs &call) {
    //self.missionArgs[contentID] = self.GetAgentMoniker(agentID).GetMissionKeywords(contentID)
    _log(AGENT__DUMP,  "AgentBound::Handle_GetMissionKeywords() - size= %u", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    /** @todo  load/save mission data for keywords, to be indexed by contentID
     *      -- this data currently saved in db by missionID (titleID)  may move to separate table to allow (easier) indexing
     *
     *  location/destination may be random.  other info found in mission data.
     */

    PyDict* keywords = new PyDict();
        keywords->SetItemString("objectiveLocationID", new PyInt(m_agent->GetStationID()));
        keywords->SetItemString("objectiveLocationSystemID", new PyInt(m_agent->GetSystemID()));
        keywords->SetItemString("objectiveTypeID", new PyInt(2631));
        keywords->SetItemString("objectiveQuantity", new PyInt(7));
        keywords->SetItemString("objectiveDestinationID", new PyInt(60001690));
        keywords->SetItemString("objectiveDestinationSystemID", new PyInt(30002777));
        keywords->SetItemString("rewardTypeID", new PyInt(29));
        keywords->SetItemString("rewardQuantity", new PyInt(28000));


    if (is_log_enabled(AGENT__RSPDUMP)) {
        _log(AGENT__RSPDUMP, "AgentBound::Handle_GetMissionKeywords() RSP:" );
        keywords->Dump(AGENT__RSPDUMP, "    ");
    }

    return keywords;
}

// new...not handled
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
