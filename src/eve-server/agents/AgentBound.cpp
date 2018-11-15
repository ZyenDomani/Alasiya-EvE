
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
#include <station/Station.h>

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
    uint8 actionID = PyRep::IntegerValue(args.arg);
    bool missionQuit = false, missionCompleted = false, missionDeclined = false;

    PyTuple* agentSays = new PyTuple(2);
    // dialog is button info
    PyList* dialog = new PyList();


    /** @todo  figure out how to implement this....
     * Level 1 = always available
     * Level 2 = +1.00 standing
     * Level 3 = +3.00 standing
     * Level 4 = +5.00 standing
     * Level 5 = +7.00 standing
     */

    using namespace Dialog::Button;

    // to set 'admin dialog options' (which i dont know wtf they are yet), i *think* you add a tuple of *something* that is NOT dict or int.
    if (false /*admin options*/) {
        PyTuple* adminButton = new PyTuple(2);
        adminButton->SetItem(0, new PyInt(Admin));
        adminButton->SetItem(1, new PyString("Admin Options"));
        dialog->AddItem(adminButton);
    }

    /** @todo determine if char can access this agent using standings.... */

    switch (actionID) {
        case 0: {
            //  if char has current mission with this agent, add this one.
            MissionOffer offer;
            if (m_agent->HasMission(call.client->GetCharacterID(), offer)) {
                PyTuple* button1 = new PyTuple(2);
                    button1->SetItem(0, new PyInt(ViewMission)); // this are buttonIDs which are unique and sequential to each agent, regardless of chars
                    button1->SetItem(1, new PyInt(ViewMission));
                dialog->AddItem(button1);
                agentSays->SetItem(0, new PyInt(offer.briefingID));
                agentSays->SetItem(1, new PyInt(offer.characterID));
            } else {
                // dialogue data.  if RequestMission is only option, client auto-responds with DoAction(RequestMission optionID)
                PyTuple* button2 = new PyTuple(2);
                    button2->SetItem(0, new PyInt(RequestMission));
                    button2->SetItem(1, new PyInt(RequestMission));
                dialog->AddItem(button2);
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

            // if agent does location, add this one...
            if (m_agent->IsLocator()) {
                PyTuple* button3 = new PyTuple(2);
                    button3->SetItem(0, new PyInt(LocateCharacter));
                    button3->SetItem(1, new PyInt(LocateCharacter));
                dialog->AddItem(button3);
            }

            // if agent does research, add this one...
            if (m_agent->IsResearch()) {
                PyTuple* button4 = new PyTuple(2);
                    button4->SetItem(0, new PyInt(StartResearch));
                    button4->SetItem(1, new PyInt(StartResearch));
                dialog->AddItem(button4);
            }
        } break;
        case RequestMission: {
            MissionOffer offer;
            m_agent->MakeOffer(call.client->GetCharacterID(), offer);
            m_agent->SendMissionUpdate(call.client, "offered");

            //  this one will get complicated and is based on agent/char interaction
            //   detail in /eve/client/script/ui/station/agents/agents.py

            /*  agentSays is a tuple of msgData and contentID
             *      msgData can be single integer of briefingID, a string literal, or a tuple as defined above.
             *   contentID is used for specific char's mission keywords.  we're not using it like there here....
             */

            agentSays->SetItem(0, new PyInt(offer.briefingID));
            agentSays->SetItem(1, new PyInt(offer.characterID));

            // dialog can also contain mission data.
            //   set a dialog tuple[1] to dict and fill with MissionBriefingInfo
            PyTuple* button1 = new PyTuple(2);
                button1->SetItem(0, new PyInt(Accept));
                button1->SetItem(1, new PyInt(Accept));
            dialog->AddItem(button1);
            PyTuple* button2 = new PyTuple(2);
                button2->SetItem(0, new PyInt(Decline));
                button2->SetItem(1, new PyInt(Decline));
            dialog->AddItem(button2);
            PyTuple* button3 = new PyTuple(2);
                button3->SetItem(0, new PyInt(Defer));
                button3->SetItem(1, new PyInt(Defer));
            dialog->AddItem(button3);
        } break;
        case ViewMission: {
            MissionOffer offer;
            m_agent->GetOffer(call.client->GetCharacterID(), offer);
            agentSays->SetItem(0, new PyInt(offer.briefingID));
            agentSays->SetItem(1, new PyInt(offer.characterID));
            PyTuple* button1 = new PyTuple(2);
                button1->SetItem(0, new PyInt(Accept));
                button1->SetItem(1, new PyInt(Accept));
            dialog->AddItem(button1);
            PyTuple* button2 = new PyTuple(2);
                button2->SetItem(0, new PyInt(Decline));
                button2->SetItem(1, new PyInt(Decline));
            dialog->AddItem(button2);
            PyTuple* button3 = new PyTuple(2);
                button3->SetItem(0, new PyInt(Defer));
                button3->SetItem(1, new PyInt(Defer));
            dialog->AddItem(button3);
        } break;
        case Accept:
        case AcceptRemotely: {
            MissionOffer offer;
            m_agent->GetOffer(call.client->GetCharacterID(), offer);
            offer.stateID = Mission::State::Accepted;
            offer.dateAccepted = GetFileTimeNow();
            m_agent->UpdateOffer(call.client->GetCharacterID(), offer);
            m_agent->SendMissionUpdate(call.client, "offer_accepted");
            agentSays->SetItem(0, new PyInt(m_agent->GetAcceptRsp(call.client->GetCharacterID())));
            agentSays->SetItem(1, new PyInt(call.client->GetCharacterID()));
            if (offer.courierItemID) {
                // add item to players hangar
                sItemFactory.SetUsingClient(call.client);
                ItemData data(offer.courierItemID, call.client->GetCharacterID(), 0, flagAutoFit, offer.courierAmount);
                InventoryItemRef iRef = sItemFactory.SpawnItem(data);
                iRef->Move(m_agent->GetStationID(), flagHangar);
                call.client->AddCourierItemRef(m_agent->GetID(), iRef);
                sItemFactory.UnsetUsingClient();
            }
        } break;
        case Complete:
        case CompleteRemotely: {
            //  need to verify all requirements have been met.
            MissionOffer offer;
            m_agent->GetOffer(call.client->GetCharacterID(), offer);
            offer.stateID = Mission::State::Completed;
            offer.dateCompleted = GetFileTimeNow();
            m_agent->UpdateOffer(call.client->GetCharacterID(), offer);
            m_agent->SendMissionUpdate(call.client, "completed");
            agentSays->SetItem(0, new PyInt(m_agent->GetCompleteRsp(call.client->GetCharacterID())));
            agentSays->SetItem(1, new PyInt(call.client->GetCharacterID()));
            if (offer.courierItemID) {
                // remove item from player possession
                sItemFactory.GetItemContainerInventory(call.client->GetCourierItemRef(m_agent->GetID())->itemID())->RemoveItem(call.client->GetCourierItemRef(m_agent->GetID()));
            }
        } break;
        case Defer: {
            // extend expiry time and close
            MissionOffer offer;
            if (m_agent->HasMission(call.client->GetCharacterID(), offer)) {
                offer.stateID = Mission::State::Defered;
                offer.expiryTime += Win32Time_Day;
                m_agent->UpdateOffer(call.client->GetCharacterID(), offer);
                m_agent->SendMissionUpdate(call.client, "prolong");
                agentSays->SetItem(0, new PyString("I can give you 24 hours to think about it."));    //msgInfo  -- if tuple[0].string then return msgInfo
                agentSays->SetItem(1, new PyNone());    // ContentID  -- PyNone used when msgInfo is string to return without processing
            }
        } break;
        case Decline: {
            missionDeclined = true;
            m_agent->DeleteOffer(call.client->GetCharacterID());
            m_agent->SendMissionUpdate(call.client, "offer_declined");
            agentSays->SetItem(0, new PyInt(m_agent->GetDeclineRsp(call.client->GetCharacterID())));
            agentSays->SetItem(1, new PyInt(call.client->GetCharacterID()));
        } break;
        case Quit: {
            missionQuit = true;
            MissionOffer offer;
            m_agent->GetOffer(call.client->GetCharacterID(), offer);
            if (offer.courierItemID) {
                // remove item from player possession
                sItemFactory.GetItemContainerInventory(call.client->GetCourierItemRef(m_agent->GetID())->itemID())->RemoveItem(call.client->GetCourierItemRef(m_agent->GetID()));
            }
            // remove mission offer and set standings accordingly
            m_agent->DeleteOffer(call.client->GetCharacterID());
            m_agent->SendMissionUpdate(call.client, "quit");
            agentSays->SetItem(0, new PyInt(m_agent->GetDeclineRsp(call.client->GetCharacterID())));
            agentSays->SetItem(1, new PyInt(call.client->GetCharacterID()));
        } break;
        case Continue: {
            // not sure what to do here.
        } break;
        case StartResearch: {
            // not sure what to do here.
        } break;
        case CancelResearch: {
            // not sure what to do here.
        } break;
        case BuyDatacores: {
            // not sure what to do here.
        } break;
        case LocateCharacter:
        case LocateAccept: {
            // not sure what to do here.
        } break;
        case LocateReject: {
            // not sure what to do here.
        } break;
        case Yes: {
            // not sure what to do here.
        } break;
        case No: {
            // not sure what to do here.
        } break;
        case AcceptChoice:{
            // i think this is for options
        } break;
        case Admin: {
            // not sure what to do here.
        } break;
        default: {
            // error
            _log(AGENT__ERROR, "AgentBound::Handle_DoAction() - unhandled buttonID %u", actionID );
            call.client->SendErrorMsg("Internal Server Error. Ref: ServerError xxxxx.");
            return nullptr;
        }
    }

    // extraInfo data....
    PyDict* xtraInfo = new PyDict();
        xtraInfo->SetItemString("loyaltyPoints",    new PyInt(call.client->GetLoyaltyPoints(m_agent->GetCorpID())));  // this is char current LP
        xtraInfo->SetItemString("missionCompleted", new PyBool(missionCompleted));
        xtraInfo->SetItemString("missionQuit",      new PyBool(missionQuit));
        xtraInfo->SetItemString("missionDeclined",  new PyBool(missionDeclined));

    if (agentSays->empty()) {
        agentSays->SetItem(0, new PyNone());  // briefingID
        agentSays->SetItem(1, new PyNone());  // ContentID
    }
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

    MissionOffer offer;
    if (!m_agent->HasMission(call.client->GetCharacterID(), offer))
        return PyStatic.NewNone();

    switch (offer.stateID) {
        case Mission::State::Allocated:
        case Mission::State::Accepted:
        case Mission::State::Failed:
        case Mission::State::Completed:
        case Mission::State::Rejected:
        case Mission::State::Defered: {
            return PyStatic.NewNone();
        }
    }

    // these are found in the client data by MessageIDs ....  i.e.  {[location]objectiveDestinationID.name}
    // contentID is the key for the keywords data on live...not used here
    PyDict* keywords = new PyDict();
        keywords->SetItemString("objectiveLocationID", new PyInt(offer.originID));
        keywords->SetItemString("objectiveLocationSystemID", new PyInt(offer.originSystemID));
        keywords->SetItemString("objectiveTypeID", new PyInt(offer.courierItemID));
        keywords->SetItemString("objectiveQuantity", new PyInt(offer.courierAmount));
        keywords->SetItemString("objectiveDestinationID", new PyInt(offer.destinationID));
        keywords->SetItemString("objectiveDestinationSystemID", new PyInt(offer.destinationSystemID));
        if (offer.rewardISK) {
            keywords->SetItemString("rewardTypeID", new PyInt(itemTypeCredits));
            keywords->SetItemString("rewardQuantity", new PyInt(offer.rewardISK));
        } else {
            // wouldnt these be in 'extra' or ?
            keywords->SetItemString("rewardTypeID", new PyInt(offer.rewardItemID));
            keywords->SetItemString("rewardQuantity", new PyInt(offer.rewardItemQty));
        }
        keywords->SetItemString("dungeonLocationID", new PyInt(offer.dungeonLocationID));
        keywords->SetItemString("dungeonSolarSystemID", new PyInt(offer.dungeonSolarSystemID));
    PyDict *briefingInfo = new PyDict();
        briefingInfo->SetItemString("ContentID", new PyInt(offer.characterID));
        briefingInfo->SetItemString("Mission Keywords", keywords);
        briefingInfo->SetItemString("Mission Title ID", new PyInt(offer.missionID));
        briefingInfo->SetItemString("Mission Briefing ID", new PyInt(offer.briefingID));
        switch(offer.typeID) {
            case Mission::Type::Courier:
                briefingInfo->SetItemString("Mission Image", sMissionDataMgr.GetCourierRes()); break;
            case Mission::Type::Mining:
                briefingInfo->SetItemString("Mission Image", sMissionDataMgr.GetMiningRes()); break;
            case Mission::Type::Encounter:
                briefingInfo->SetItemString("Mission Image", sMissionDataMgr.GetKillRes()); break;
        }
        // decline time OR expiration time.  if not decline then expiration
        briefingInfo->SetItemString("Decline Time", new PyNone());   // -1 is generic decline msg
        briefingInfo->SetItemString("Expiration Time", new PyLong(offer.expiryTime) );

    if (is_log_enabled(AGENT__RSPDUMP)) {
        _log(AGENT__RSPDUMP, "AgentBound::Handle_GetMissionBriefingInfo() RSP:" );
        briefingInfo->Dump(AGENT__RSPDUMP, "    ");
    }

    return briefingInfo;
}

PyResult AgentBound::Handle_GetMissionKeywords(PyCallArgs &call) {
    // thse are the variables embedded in the messageIDs
    //self.missionArgs[contentID] = self.GetAgentMoniker(agentID).GetMissionKeywords(contentID)
    _log(AGENT__DUMP,  "AgentBound::Handle_GetMissionKeywords() - size= %u", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    /*   none of this really matters as we're not using 'contentID' like live does
    Call_SingleArg args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: failed to decode arguments", call.client->GetName());
        return nullptr;
    }

    uint32 contentID = PyRep::IntegerValue(args.arg);
    if (contentID == 0)
        return PyStatic.NewNone();
    */
    MissionOffer offer;
    if (!m_agent->HasMission(call.client->GetCharacterID(), offer))
        return PyStatic.NewNone();

    PyDict* keywords = new PyDict();
    keywords->SetItemString("objectiveLocationID", new PyInt(offer.originID));
    keywords->SetItemString("objectiveLocationSystemID", new PyInt(offer.originSystemID));
    keywords->SetItemString("objectiveTypeID", new PyInt(offer.courierItemID));
    keywords->SetItemString("objectiveQuantity", new PyInt(offer.courierAmount));
    keywords->SetItemString("objectiveDestinationID", new PyInt(offer.destinationID));
    keywords->SetItemString("objectiveDestinationSystemID", new PyInt(offer.destinationSystemID));
    if (offer.rewardISK) {
        keywords->SetItemString("rewardTypeID", new PyInt(itemTypeCredits));
        keywords->SetItemString("rewardQuantity", new PyInt(offer.rewardISK));
    } else {
        // wouldnt these be in 'extra' or ?
        keywords->SetItemString("rewardTypeID", new PyInt(offer.rewardItemID));
        keywords->SetItemString("rewardQuantity", new PyInt(offer.rewardItemQty));
    }
    keywords->SetItemString("dungeonLocationID", new PyInt(offer.dungeonLocationID));
    keywords->SetItemString("dungeonSolarSystemID", new PyInt(offer.dungeonSolarSystemID));

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
    //  returning mission info sets double-pane view, where PyNone sets single-pane view
    _log(AGENT__DUMP,  "AgentBound::Handle_GetMissionObjectiveInfo() - size= %u", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    MissionOffer offer;
    if (call.tuple->size() == 0)
        if (!m_agent->HasMission(call.client->GetCharacterID(), offer))
            return PyStatic.NewNone();

    if (offer.stateID != Mission::State::Offered)
        return PyStatic.NewNone();

    PyDict* objectiveData = new PyDict();
    objectiveData->SetItemString("missionTitleID", new PyInt(offer.missionID));
    objectiveData->SetItemString("contentID", new PyInt(offer.characterID));
    objectiveData->SetItemString("importantStandings", new PyInt(offer.important));     // boolean integer
    objectiveData->SetItemString("completionStatus", new PyInt(Mission::Status::Incomplete));       // Mission::Status:: data here 0=no, 1=yes, 2=cheat
    objectiveData->SetItemString("missionState", new PyInt(offer.stateID /*Mission::State::Offered*/));   // Mission::State:: data here for agentGift populating.  Accepted/failed to display gift items as accepted
    objectiveData->SetItemString("loyaltyPoints", new PyInt(0));
    objectiveData->SetItemString("researchPoints", new PyInt(0));

    /*  this puts title/msg at bottom of right pane
    PyTuple* missionExtra = new PyTuple(2);  // this is tuple(2)  headerID, bodyID    -- std locale msgIDs
        missionExtra->SetItem(0, new PyInt(offer.missionID));   // this should be separate title from mission name
        missionExtra->SetItem(1, new PyInt(offer.briefingID));   // this is additional info about mission, etc.
    objectiveData->SetItemString("missionExtra", missionExtra);
    */

    PyList* locList = new PyList();    // tuple of list of locationIDs (pickup and dropoff)
        locList->AddItem(new PyInt(offer.originSystemID));
        locList->AddItem(new PyInt(offer.destinationSystemID));
    objectiveData->SetItemString("locations", locList);

    PyList* giftList = new PyList();    // this is list of tuple(3)  typeID, quantity, extra
    /*
    PyDict* extra = new PyDict();    // 'extra' is either specificItemID or blueprint data.
        extra->SetItemString("specificItemID", new PyNone());
        extra->SetItemString("blueprintInfo", new PyNone());
    PyTuple* agentGift = new PyTuple(3);
        agentGift->SetItem(0, new PyNone());
        agentGift->SetItem(1, new PyNone());
        agentGift->SetItem(2, extra);
        giftList->AddItem(agentGift);
    */
    objectiveData->SetItemString("agentGift", giftList);

    PyList* normList = new PyList();    // this is list of tuple(3)  typeID, quantity, extra
    if (offer.rewardISK) {
        PyDict* extra = new PyDict();    // 'extra' is either specificItemID or blueprint data.
            //extra->SetItemString("specificItemID", new PyNone());
            //extra->SetItemString("blueprintInfo", new PyNone());
        PyTuple* normalRewards = new PyTuple(3);
            normalRewards->SetItem(0, new PyInt(itemTypeCredits));
            normalRewards->SetItem(1, new PyInt(offer.rewardISK));
            normalRewards->SetItem(2, extra);
        normList->AddItem(normalRewards);
    }
    if (offer.rewardItemID) {
        PyDict* extra = new PyDict();    // 'extra' is either specificItemID or blueprint data.
            //extra->SetItemString("specificItemID", new PyNone());
            //extra->SetItemString("blueprintInfo", new PyNone());
        PyTuple* normalRewards = new PyTuple(3);
            normalRewards->SetItem(0, new PyInt(offer.rewardItemID));
            normalRewards->SetItem(1, new PyInt(offer.rewardItemQty));
            normalRewards->SetItem(2, extra);
        normList->AddItem(normalRewards);
    }
    objectiveData->SetItemString("normalRewards", normList);

    PyList* collateralList = new PyList(); // this is list of tuple(3)  typeID, quantity, extra
    /*
    PyDict* extra = new PyDict();    // 'extra' is either specificItemID or blueprint data.
        extra->SetItemString("specificItemID", new PyNone());
        extra->SetItemString("blueprintInfo", new PyNone());
    PyTuple* collateral = new PyTuple(3);
        collateral->SetItem(0, new PyNone());
        collateral->SetItem(1, new PyNone());
        collateral->SetItem(2, extra);
        */
    objectiveData->SetItemString("collateral", collateralList);

    PyList* bonusList = new PyList();   // this is list of tuple(4)  timeRemaining, typeID, quantity, extra
    if (true /*bonus1*/) {
        PyDict* extra = new PyDict();    // 'extra' is either specificItemID or blueprint data.
            //extra->SetItemString("specificItemID", new PyNone());
            //extra->SetItemString("blueprintInfo", new PyNone());
        PyTuple* bonusRewards = new PyTuple(4);
            bonusRewards->SetItem(0, new PyLong(18000000000));  //30m
            bonusRewards->SetItem(1, new PyInt(itemTypeCredits));
            bonusRewards->SetItem(2, new PyInt(offer.rewardISK *2));
            bonusRewards->SetItem(3, extra);
        bonusList->AddItem(bonusRewards);
    }
    if (true /*bonus2*/) {
        PyDict* extra = new PyDict();    // 'extra' is either specificItemID or blueprint data.
            //extra->SetItemString("specificItemID", new PyNone());
            //extra->SetItemString("blueprintInfo", new PyNone());
        PyTuple* bonusRewards2 = new PyTuple(4);
            bonusRewards2->SetItem(0, new PyLong(12000000000)); //20m
            bonusRewards2->SetItem(1, new PyInt(itemTypeTrit));
            bonusRewards2->SetItem(2, new PyInt(offer.rewardISK));
            bonusRewards2->SetItem(3, extra);
        bonusList->AddItem(bonusRewards2);
    }
    objectiveData->SetItemString("bonusRewards", bonusList);
    /*  for collateral and rewards, as follows...
    typeID, quantity, extra in objectiveData['normalRewards']
    typeID, quantity, extra in objectiveData['collateral']
    typeID, quantity, extra in objectiveData['agentGift']
    or
    timeRemaining, typeID, quantity, extra in objectiveData['bonusRewards']

        specificItemID = extra.get('specificItemID', 0)
        blueprintInfo = extra.get('blueprintInfo', None)
        */

    PyDict* pickupLocation = new PyDict();
        pickupLocation->SetItemString("typeID", new PyInt(m_agent->GetLocTypeID()) );
        pickupLocation->SetItemString("locationID", new PyInt(offer.originID) );
        pickupLocation->SetItemString("solarsystemID", new PyInt(offer.originSystemID) );

    PyDict* dropoffLocation = new PyDict();
        dropoffLocation->SetItemString("typeID", new PyInt(offer.destinationTypeID) );
        dropoffLocation->SetItemString("locationID", new PyInt(offer.destinationID) );
        dropoffLocation->SetItemString("solarsystemID", new PyInt(offer.destinationSystemID) );

    PyDict* cargo = new PyDict();
    /** @todo  check to see if player has correct cargo and set accordingly  */
    if (call.client->GetShip()->GetMyInventory()->Contains(offer.courierItemID, offer.courierAmount))
        cargo->SetItemString("hasCargo", new PyBool(true));
    else
        cargo->SetItemString("hasCargo", new PyBool(false));
        cargo->SetItemString("typeID", new PyInt(offer.courierItemID));
        cargo->SetItemString("quantity", new PyInt(offer.courierAmount));
        cargo->SetItemString("volume", new PyFloat(offer.courierItemVolume * offer.courierAmount));    // calculated shipment volume.  *this is direct to window*
    PyTuple* objData = new PyTuple(5);
        objData->SetItem(0, new PyInt(offer.originOwnerID));
        objData->SetItem(1, pickupLocation/*m_agent->GetLocationWrap()*/);
        objData->SetItem(2, new PyInt(offer.destinationOwnerID));
        objData->SetItem(3, dropoffLocation/*m_agent->GetLocationWrap()*/);
        objData->SetItem(4, cargo);
    PyTuple* objectives = new PyTuple(2);   // this is list of tuple(2)    objType, objData
        objectives->SetItem(0, new PyString("transport"));
        objectives->SetItem(1, objData);
    PyList* objList = new PyList();
        objList->AddItem(objectives);
    objectiveData->SetItemString("objectives", objList);

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
    PyList* dunList = new PyList();  // this is a list of dunData dicts
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
    objectiveData->SetItemString("dungeons", dunList);
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
    //called on rclick in journal to "read details"
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
    //called on rclick in journal to "remove offer"
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
