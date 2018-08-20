
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
#include "../../eve-common/EVE_Missions.h"

#include "StaticDataMgr.h"
#include "agents/AgentBound.h"

AgentBound::AgentBound(PyServiceMgr *mgr, Agent *agt)
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
    // this is first call when initiating agent convo
    _log(AGENT__DUMP,  "AgentBound::Handle_DoAction() - size= %u", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    // sends PyNone or actionID
    Call_SingleArg args;
    if (!args.Decode(&call.tuple)) {
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

    std::string response = "";
    PyTuple* agentSays = new PyTuple(2);
    // dialog is button info
    PyList* dialog = new PyList();

    if (actionID == 0) {
        m_agent->SetMission(false);

        //  if char has current mission wiht this agent, add this one.
        /*
        PyTuple* button1 = new PyTuple(2);
            button1->SetItem(0, new PyInt(12)); // this are buttonIDs which are unique and sequential to each agent, regardless of chars
            button1->SetItem(1, new PyInt(Dialog::Button::ViewMission));
        dialog->AddItem(button1);
            */

        // dialogue data.  if RequestMission is only option, client auto-responds with DoAction(RequestMission optionID)
        PyTuple* button2 = new PyTuple(2);
            button2->SetItem(0, new PyInt(13));
            button2->SetItem(1, new PyInt(Dialog::Button::RequestMission));
        dialog->AddItem(button2);

        // if agent does location, add this one...
        if (m_agent->IsLocator()) {
            PyTuple* button3 = new PyTuple(2);
                button3->SetItem(0, new PyInt(34));
                button3->SetItem(1, new PyInt(Dialog::Button::LocateCharacter));
            dialog->AddItem(button3);
        }

        // if agent does research, add this one...
        if (m_agent->IsResearch()) {
            PyTuple* button4 = new PyTuple(2);
                button4->SetItem(0, new PyInt(56));
                button4->SetItem(1, new PyInt(Dialog::Button::StartResearch));
            dialog->AddItem(button4);
        }
        if (dialog->size() > 1) {
            // response as string for custom data.  response as pyint to use client data (using getlocale shit)
            // default initial agent response based on agent location, level, bloodline, quality, and char/agent standings
            //  this will be modeled after UO speech data, in tiers and levels.
            // if RequestMission is only option, this is ignored.  see note under 'dialog data'
            response = "Why the fuck am I looking at you again, ";
            response += call.client->GetCharacterName().c_str();
            response += "?";
            agentSays->SetItem(0, new PyString(response));  //msgInfo  -- if tuple[0].string then return msgInfo
            agentSays->SetItem(1, new PyNone());      // ContentID  -- PyNone used when msgInfo is string (mostly for initial greetings)
        }
    } else {
        m_agent->SetMission(true);

    //  this one will get complicated and is based on agent/char interaction
    //   detail in /eve/client/script/ui/station/agents/agents.py

        /*  agentSays is a tuple of msgData and contentID
         *      msgData can be single integer of descriptionID, a string literal, or a tuple as defined above.
         *  it doesnt appear that contentID is used for display.
         */

        agentSays->SetItem(0, new PyInt(236729));    // msgID  -- 236729 = greetings, {char.name}    236722 = hello there, {char.name}
        agentSays->SetItem(1, new PyInt(130997));    //mission contentID (descriptionID) to be displayed in dialog box

        // dialog can also contain mission data.
        //   set a dialog tuple[1] to dict and fill with MissionBriefingInfo
        // dont remember if i got this one working or not....
        /*
        PyDict* keywords = new PyDict();
            keywords->SetItemString("objectiveLocationID", new PyInt(m_agent->GetStationID()));
            keywords->SetItemString("objectiveLocationSystemID", new PyInt(m_agent->GetSystemID()));
            keywords->SetItemString("objectiveTypeID", new PyInt(2631));
            keywords->SetItemString("objectiveQuantity", new PyInt(7));
            keywords->SetItemString("objectiveDestinationID", new PyInt(m_agent->GetStationID()));
            keywords->SetItemString("objectiveDestinationSystemID", new PyInt(m_agent->GetSystemID()));
            keywords->SetItemString("rewardTypeID", new PyInt(29));
            keywords->SetItemString("rewardQuantity", new PyInt(28000));
        PyDict *briefingInfo = new PyDict();
            briefingInfo->SetItemString("ContentID", new PyInt(130997));  // not sure when/if this is filled
            briefingInfo->SetItemString("Mission Keywords", keywords);  // only used when "ContentID" is filled?
            briefingInfo->SetItemString("Mission Title ID", new PyInt(55205) );
            briefingInfo->SetItemString("Mission Briefing ID", new PyInt(130997) );
            // will have to find and store mission images *somewhere*  EVE_Mission.h probably.
            briefingInfo->SetItemString("Mission Image", new PyString("<img src='res:/UI/netres/mission_content/couriermission.png' align=center hspace=4 vspace=4>") );
            briefingInfo->SetItemString("Decline Time", new PyNone());   // -1 is generic decline msg
            // decline time OR expiration time.  if decline is none then expiration
            briefingInfo->SetItemString("Expiration Time", new PyLong( GetFileTimeNow()+Win32Time_Day ) );
        PyTuple* info = new PyTuple(2);
            info->SetItem(0, new PyInt(1)); // this button is a DoAction# for mission title link.  not sure why
            info->SetItem(1, briefingInfo);
        dialog->AddItem(info);
            */

        PyTuple* button1 = new PyTuple(2);
            button1->SetItem(0, new PyInt(23));
            button1->SetItem(1, new PyInt(Dialog::Button::Accept));
        dialog->AddItem(button1);
        PyTuple* button2 = new PyTuple(2);
            button2->SetItem(0, new PyInt(45));
            button2->SetItem(1, new PyInt(Dialog::Button::Decline));
        dialog->AddItem(button2);
        PyTuple* button3 = new PyTuple(2);
            button3->SetItem(0, new PyInt(67));
            button3->SetItem(1, new PyInt(Dialog::Button::Defer));
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
        outer->SetItem(1, xtraInfo);

    if (is_log_enabled(AGENT__RSPDUMP)) {
        _log(AGENT__RSPDUMP, "AgentBound::Handle_DoAction RSP:" );
        outer->Dump(AGENT__RSPDUMP, "    ");
    }

    return outer;
}

PyResult AgentBound::Handle_GetMissionBriefingInfo(PyCallArgs &call) {
    // called from iniate agent convo... should be populated when mission available
    // will return PyNone if no mission avalible
    _log(AGENT__MESSAGE,  "AgentBound::Handle_GetMissionBriefingInfo()");

    if (!m_agent->HasMission(call.client->GetCharacterID()))
        return PyStatic.NewNone();

    // TODO  this data will have to be generated per char based on mission offer data.
    PyDict* keywords = new PyDict();
        keywords->SetItemString("objectiveLocationID", new PyInt(m_agent->GetStationID()));
        keywords->SetItemString("objectiveLocationSystemID", new PyInt(m_agent->GetSystemID()));
        keywords->SetItemString("objectiveTypeID", new PyInt(2631));
        keywords->SetItemString("objectiveQuantity", new PyInt(7));
        keywords->SetItemString("objectiveDestinationID", new PyInt(m_agent->GetStationID()));
        keywords->SetItemString("objectiveDestinationSystemID", new PyInt(m_agent->GetSystemID()));
        keywords->SetItemString("rewardTypeID", new PyInt(29));
        keywords->SetItemString("rewardQuantity", new PyInt(28000));
    PyDict *briefingInfo = new PyDict();
        briefingInfo->SetItemString("ContentID", new PyInt(130997));  // this is mission descriptionID
        briefingInfo->SetItemString("Mission Keywords", keywords);  // only used when "ContentID" is filled?
        briefingInfo->SetItemString("Mission Title ID", new PyInt(55205) );
        briefingInfo->SetItemString("Mission Briefing ID", new PyInt(130997) );
        // will have to find and store mission images *somewhere*  EVE_Mission.h probably.
        briefingInfo->SetItemString("Mission Image", new PyString("<img src='res:/UI/netres/mission_content/couriermission.png' align=center hspace=4 vspace=4>") );
        // decline time OR expiration time.  if not decline then expiration
        briefingInfo->SetItemString("Decline Time", new PyNone());   // -1 is generic decline msg
        briefingInfo->SetItemString("Expiration Time", new PyLong( GetFileTimeNow()+Win32Time_Day ) );

    if (is_log_enabled(AGENT__RSPDUMP)) {
        _log(AGENT__RSPDUMP, "AgentBound::Handle_GetMissionBriefingInfo() RSP:" );
        briefingInfo->Dump(AGENT__RSPDUMP, "    ");
    }

    return briefingInfo;
}

PyResult AgentBound::Handle_GetMissionKeywords(PyCallArgs &call) {
    //self.missionArgs[contentID] = self.GetAgentMoniker(agentID).GetMissionKeywords(contentID)
    _log(AGENT__DUMP,  "AgentBound::Handle_GetMissionKeywords() - size= %u", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    Call_SingleArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: failed to decode arguments", call.client->GetName());
        return nullptr;
    }

    uint32 contentID = PyRep::IntegerValue(args.arg);
    if (contentID == 0)
        return PyStatic.NewNone();

    /** @todo  load/save mission data for keywords to be indexed by contentID
     *      -- this data currently saved in db by missionID (titleID)  may move to separate table to allow (easier) indexing
     *
     *  location/destination may be random.  other info found in mission data.
     */

    PyDict* keywords = new PyDict();
    keywords->SetItemString("objectiveLocationID", new PyInt(m_agent->GetStationID()));
    keywords->SetItemString("objectiveLocationSystemID", new PyInt(m_agent->GetSystemID()));
    keywords->SetItemString("objectiveTypeID", new PyInt(2631));
    keywords->SetItemString("objectiveQuantity", new PyInt(7));
    keywords->SetItemString("objectiveDestinationID", new PyInt(m_agent->GetStationID()));
    keywords->SetItemString("objectiveDestinationSystemID", new PyInt(m_agent->GetSystemID()));
    keywords->SetItemString("rewardTypeID", new PyInt(29));
    keywords->SetItemString("rewardQuantity", new PyInt(28000));

    if (is_log_enabled(AGENT__RSPDUMP)) {
        _log(AGENT__RSPDUMP, "AgentBound::Handle_GetMissionKeywords() RSP:" );
        keywords->Dump(AGENT__RSPDUMP, "    ");
    }

    return keywords;
}

PyResult AgentBound::Handle_GetMissionObjectiveInfo(PyCallArgs &call)
{
    // sends charID, contentID
    // returns PyDict loaded with mission info  or PyNone
    _log(AGENT__DUMP,  "AgentBound::Handle_GetMissionObjectiveInfo() - size= %u", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    if (call.tuple->size() == 0)
        if (!m_agent->HasMission(call.client->GetCharacterID()))
            return PyStatic.NewNone();

    PyDict* objectiveData = new PyDict();
    objectiveData->SetItemString("missionTitleID", new PyInt(55205));
    objectiveData->SetItemString("contentID", new PyInt(130997));
    objectiveData->SetItemString("importantStandings", new PyInt(0));     // boolean integer
    objectiveData->SetItemString("completionStatus", new PyInt(Mission::Status::Incomplete));       // Mission::Status:: data here 0=no, 1=yes, 2=cheat
    objectiveData->SetItemString("missionState", new PyInt(Mission::State::Offered));   // Mission::State:: data here for agentGift populating.  Accepted/failed to display gift items as accepted
    objectiveData->SetItemString("loyaltyPoints", new PyInt(0));
    objectiveData->SetItemString("researchPoints", new PyInt(0));

    PyList* locList = new PyList();
        locList->AddItem(new PyInt(m_agent->GetSystemID()));
    objectiveData->SetItemString("locations", locList);      // tuple of list of locationIDs (pickup and dropoff)

    /*
    PyTuple* agentGift = new PyTuple(3);
        agentGift->SetItem(0, new PyNone());
        agentGift->SetItem(1, new PyNone());
        agentGift->SetItem(2, new PyNone());
        PyList* locList = new PyList();
        */
    objectiveData->SetItemString("agentGift", new PyList());  // this is list of tuple(3)  typeID, quantity, extra

    PyTuple* normalRewards = new PyTuple(3);
        normalRewards->SetItem(0, new PyInt(itemTypeCredits));
        normalRewards->SetItem(1, new PyInt(12000));
        normalRewards->SetItem(2, new PyNone());
    PyList* normList = new PyList();
        normList->AddItem(normalRewards);
    objectiveData->SetItemString("normalRewards", normList);  // this is list of tuple(3)  typeID, quantity, extra

    /*
    PyTuple* collateral = new PyTuple(3);
        collateral->SetItem(0, new PyNone());
        collateral->SetItem(1, new PyNone());
        collateral->SetItem(2, new PyNone());
        */
    objectiveData->SetItemString("collateral", new PyList());   // this is list of tuple(3)  typeID, quantity, extra

    PyTuple* bonusRewards = new PyTuple(4);
        bonusRewards->SetItem(0, new PyLong(18000000000));  //30m
        bonusRewards->SetItem(1, new PyInt(itemTypeCredits));
        bonusRewards->SetItem(2, new PyInt(20000));
        bonusRewards->SetItem(3, new PyNone());
    PyTuple* bonusRewards2 = new PyTuple(4);
        bonusRewards2->SetItem(0, new PyLong(12000000000)); //20m
        bonusRewards2->SetItem(1, new PyInt(itemTypeTrit));
        bonusRewards2->SetItem(2, new PyInt(10000));
        bonusRewards2->SetItem(3, new PyNone());
    PyList* bonusList = new PyList();
        bonusList->AddItem(bonusRewards);
        bonusList->AddItem(bonusRewards2);
    objectiveData->SetItemString("bonusRewards", bonusList);   // this is list of tuple(4)  timeRemaining, typeID, quantity, extra

    /*  for collateral and rewards, as follows...
    typeID, quantity, extra in objectiveData['normalRewards']
    typeID, quantity, extra in objectiveData['collateral']
    typeID, quantity, extra in objectiveData['agentGift']
    or
    timeRemaining, typeID, quantity, extra in objectiveData['bonusRewards']

        specificItemID = extra.get('specificItemID', 0)
        blueprintInfo = extra.get('blueprintInfo', None)
        */

    //PyTuple* missionExtra = new PyTuple(0);
    //objectiveData->SetItemString("missionExtra", missionExtra);       // this is tuple(2)  headerID, bodyID    -- std locale msgIDs

        /*
    PyDict* pickupLocation = new PyDict();
        pickupLocation->SetItemString("typeID", new PyInt(locationTypeID) );
        pickupLocation->SetItemString("locationID", new PyInt(locationID) );
        pickupLocation->SetItemString("solarsystemID", new PyInt(solarSystemID) );
    */
        /*
    PyDict* dropoffLocation = new PyDict(m_agent->GetID());
        dropoffLocation->SetItemString("typeID", new PyInt(locationTypeID) );
        dropoffLocation->SetItemString("locationID", new PyInt(locationID) );
        dropoffLocation->SetItemString("solarsystemID", new PyInt(solarSystemID) );
        */
    PyDict* cargo = new PyDict();
        cargo->SetItemString("hasCargo", new PyBool(false));
        cargo->SetItemString("typeID", new PyInt(3687));
        cargo->SetItemString("volume", new PyFloat(75));    // pre-calculated shipment volume.  *this is direct to window*
        cargo->SetItemString("quantity", new PyInt(75));
    PyTuple* objData = new PyTuple(5);
        objData->SetItem(0, new PyInt(1000059)); // pickupOwnerID  (corpID)
        objData->SetItem(1, /*pickupLocation*/m_agent->GetLocationWrap());
        objData->SetItem(2, new PyInt(1000059));  // dropoffOwnerID
        objData->SetItem(3, /*dropoffLocation*/m_agent->GetLocationWrap());
        objData->SetItem(4, cargo);
    PyTuple* objectives = new PyTuple(2);
        objectives->SetItem(0, new PyString("transport"));
        objectives->SetItem(1, objData);
    PyList* objList = new PyList();
        objList->AddItem(objectives);
    objectiveData->SetItemString("objectives", objList);     // this is list of tuple(2)    objType, objData
    /*  objectives data...
    if objType == 'agent':      -- report to agent
        agentID, agentLocation = objData
        agentLocation['locationID']
        agentLocation['locationType']
        agentLocation['solarsystemID']
        if not in station
            agentLocation['coords']
            agentLocation['agentID']
            ?agentLocation['referringAgentID']
            ?agentLocation['shipTypeID']

    elif objType == 'transport':        -- courier and trade missions
        pickupOwnerID, pickupLocation, dropoffOwnerID, dropoffLocation, cargo = objData
        pickupLocation['locationID']
        pickupLocation['locationType']
        pickupLocation['solarsystemID']
        dropoffLocation['locationID']
        dropoffLocation['locationType']
        dropoffLocation['solarsystemID']
        if not in station
            dropoffLocation['coords']
            dropoffLocation['agentID']
            ?dropoffLocation['referringAgentID']
            ?dropoffLocation['shipTypeID']
        cargo['hasCargo']
        cargo['typeID']
        cargo['volume']
        cargo['quantity']

    elif objType == 'fetch':            -- encounter and mining missions
        dropoffOwnerID, dropoffLocation, cargo = objData
        dropoffLocation['locationID']
        dropoffLocation['locationType']
        dropoffLocation['solarsystemID']
        if not in station
            dropoffLocation['coords']
            dropoffLocation['agentID']
            ?dropoffLocation['referringAgentID']
            ?dropoffLocation['shipTypeID']
        cargo['hasCargo']
        cargo['typeID']
        cargo['volume']
        cargo['quantity']
        */
    /*
    PyDict* dunData = new PyDict();
        dunData->SetItemString("dungeonID", new PyInt(1000));
        dunData->SetItemString("completionStatus", new PyInt(Dungeon::Status::Started));
        dunData->SetItemString("optional", new PyInt());
        dunData->SetItemString("briefingMessage", new PyInt());
        dunData->SetItemString("objectiveCompleted", new PyBool(false));
        dunData->SetItemString("ownerID", new PyInt(m_agent->GetID()));
        dunData->SetItemString("shipRestrictions", new PyInt(0));   // 0=normal 1=special with link to *something else*
        dunData->SetItemString("location", m_agent->GetLocationWrap());
    */
    objectiveData->SetItemString("dungeons", new PyList()); // this is a list of dunData dicts
    /* dunData data....
     * dungeonID
     * completionStatus
     * optional
     * briefingMessage
     * objectiveCompleted
     * ownerID
     * location
        location['locationID']
        location['locationType']
        location['solarsystemID']
        location['coords']
        location['agentID']
        ?location['referringAgentID']
        ?location['shipTypeID']
     * shipRestrictions  0=normal 1=special with link to *something else*
     */

    if (is_log_enabled(AGENT__RSPDUMP)) {
        _log(AGENT__RSPDUMP, "AgentBound::Handle_GetMissionObjectiveInfo() RSP:" );
        objectiveData->Dump(AGENT__RSPDUMP, "    ");
    }

    return objectiveData;
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
