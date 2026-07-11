
 /**
  * @name AgentBound.cpp
  *   agent specific code
  *    removed from AgentMgrService.cpp
  *
  * @Author:        Allan
  * @date:      26 June 2018
  * @update:    02 March 2025
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
 * AGENT__RSP_DUMP
 */

#include "../eve-server.h"
#include "../../eve-common/EVE_Missions.h"
#include "../../eve-common/EVE_Standings.h"

#include "StaticDataMgr.h"
#include "account/AccountService.h"
#include "agents/AgentBound.h"
#include "standing/StandingMgr.h"
#include "station/Station.h"

AgentBound::AgentBound(PyServiceMgr *mgr, Agent *agt)
: PyBoundObject(mgr),
m_dispatch(new Dispatcher(this)),
m_agent(agt),
m_showObjectives(false)
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

PyResult AgentBound::Handle_GetInfoServiceDetails(PyCallArgs& call ) {
    // this is agents personal info... level, division, station, etc. but based on calling character
    return m_agent->GetInfoServiceDetails(call.client);
}


/**     ***********************************************************************
 * @note   these below are partially coded
 */

PyResult AgentBound::Handle_DoAction(PyCallArgs &call) {
    // this is first call when initiating agent convo
    _log(AGENT__DUMP,  "AgentBound::Handle_DoAction() - size= %lu", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    // sends PyNone or actionID
    Call_SingleArg args;
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }
    /* this actually returns a complicated tuple depending on other variables involving this agent and char.
     * data shows this can be a full dialog with multiple options during conversation
     * buttonIDs are coded to set the next set of dialog options, but i dont quite know how to implement them yet
     * nor do i know how to show an actual dialog besides mission information at this time.
     */

    //   detail in /eve/client/script/ui/station/agents/agents.py

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

    m_showObjectives = false;

    std::string response = "";
    Character* pChar(call.client->GetChar().get());
    uint8 actionID(PyRep::IntegerValueU32(args.arg));
    bool missionQuit(false);
    bool missionCompleted(false);
    bool missionDeclined(false);

    PyTuple* agentSays(new PyTuple(2));
    // dialog is button info
    PyList* dialog(new PyList());
    PyDict* xtraInfo(new PyDict());

    using namespace Dialog::Button;

    // to set 'admin dialog options' (which i dont know wtf they are yet), i *think* you add a tuple of *something* that is NOT dict or int.
    //NOTE:  segfaults when enabled...
    if (sConfig.agent.AllowAdminMenu) {
        PyTuple* adminButton(new PyTuple(2));
        adminButton->SetItem(0, new PyInt(Admin));
        adminButton->SetItem(1, new PyString("Admin Options"));
        dialog->AddItem(adminButton);
        /*
            if adminOptions:
                if len(adminOptions) == 1:
                    adminBlock = '<tr><td align=center><br>'
                    adminBlock += adminOptions[0]
                else:
                    adminBlock = '<tr><td>'
                    adminBlock += '<ol>'
                    adminBlock += ''.join([ '<br><li>%s</li>' % x for x in adminOptions ])
                    adminBlock += '</ol>'
                adminBlock += '</td></tr>'
         */
    }

    if (!m_agent->CanUseAgent(call.client)) {
        agentSays->SetItem(0, new PyInt(m_agent->GetResponse(pChar, Agents::Response::NoStanding)));
        agentSays->SetItem(1, PyStatic.NewNone());
        PyTuple* inner = new PyTuple(2);
            inner->SetItem(0, agentSays);
            inner->SetItem(1, dialog);
        PyTuple* outer = new PyTuple(2);
            outer->SetItem(0, inner);
            outer->SetItem(1, xtraInfo);
        return outer;
    }

    switch (actionID) {
        case 0: {   //initial dialog (start convo with...)
            //  initial greetings - offered mission...these need more data to properly implement

            //  if char has current mission with this agent, add this one.
            MissionOffer offer = MissionOffer();
            if (m_agent->HasMission(pChar->itemID(), offer)) {
                if (call.client->IsMissionComplete(offer))  {
                    agentSays->SetItem(0, new PyInt(m_agent->GetResponse(pChar, Agents::Response::Greeting)));
                    agentSays->SetItem(1, PyStatic.NewNone());
                    PyTuple* button2 = new PyTuple(2);
                        button2->SetItem(0, new PyInt(Complete));
                        button2->SetItem(1, new PyInt(Complete));
                    dialog->AddItem(button2);
                    m_showObjectives = true;
                } else if (offer.stateID == Mission::State::Accepted) {
                    agentSays->SetItem(0, new PyInt(m_agent->GetResponse(pChar, Agents::Response::InProgress)));
                    agentSays->SetItem(1, PyStatic.NewNone());
                    PyTuple* button1 = new PyTuple(2);
                        button1->SetItem(0, new PyInt(Quit));
                        button1->SetItem(1, new PyInt(Quit));
                    dialog->AddItem(button1);
                    PyTuple* button2 = new PyTuple(2);
                        button2->SetItem(0, new PyInt(ViewMission));
                        button2->SetItem(1, new PyInt(ViewMission));
                    dialog->AddItem(button2);
                } else if (offer.stateID == Mission::State::Offered) {
                    PyDict* offerDict = new PyDict();
                        offerDict->SetItemString("missionOfferText", new PyInt(offer.briefingID));
                    PyTuple* wrapper = new PyTuple(2);
                    wrapper->SetItem(0, new PyString("UI/Agents/DefaultMessages/OfferStillOpen"));  // `Yes my offer is still open.<br><br>{missionOfferText}`)
                        wrapper->SetItem(1, offerDict);
                    agentSays->SetItem(0, wrapper);
                    agentSays->SetItem(1, new PyInt(offer.characterID));       //contentID
                    PyTuple* button2 = new PyTuple(2);
                        button2->SetItem(0, new PyInt(ViewMission));
                        button2->SetItem(1, new PyInt(ViewMission));
                    dialog->AddItem(button2);
                } else {
                    PyDict* offerDict = new PyDict();
                        offerDict->SetItemString("missionBriefingText", new PyInt(offer.briefingID));
                    PyTuple* wrapper = new PyTuple(2);
                        wrapper->SetItem(0, new PyString(m_agent->GetTextResponse(pChar, Agents::Response::Available)));
                        wrapper->SetItem(1, offerDict);
                    agentSays->SetItem(0, wrapper);
                    agentSays->SetItem(1, new PyInt(offer.characterID));       //contentID
                    PyTuple* button2 = new PyTuple(2);
                        button2->SetItem(0, new PyInt(ViewMission));
                        button2->SetItem(1, new PyInt(ViewMission));
                    dialog->AddItem(button2);
                }
            } else {
                if (m_agent->IsStoryline()) {
                    agentSays->SetItem(0, new PyInt(m_agent->GetResponse(pChar, Agents::Response::Greeting)));
                    agentSays->SetItem(1, PyStatic.NewNone());
                } else {
                    agentSays->SetItem(0, new PyInt(m_agent->GetResponse(pChar, Agents::Response::Greeting)));
                    agentSays->SetItem(1, PyStatic.NewNone());
                    // dialogue data.  if RequestMission is only option, client auto-responds with DoAction(RequestMission optionID)
                    PyTuple* button2 = new PyTuple(2);
                        button2->SetItem(0, new PyInt(RequestMission));
                        button2->SetItem(1, new PyInt(RequestMission));
                        dialog->AddItem(button2);
                    // response as string for custom data.  response as pyint to use client data (using getlocale shit)
                    // default initial agent response based on agent location, level, bloodline, quality, and char/agent standings
                    //  this will be modeled after UO speech data, in tiers and levels.
                    // if RequestMission is only option, this is ignored.  see note under 'dialog data'
                /*
                    response = "Why the fuck am I looking at you again, ";
                    response += call.client->GetName();
                    response += "?";
                    agentSays->SetItem(0, new PyString(response));  //msgInfo  -- raw string for custom or messageID
                    agentSays->SetItem(1, PyStatic.NewNone());      // ContentID  -- PyNone used when msgInfo is string or greetingID
                */
                }
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
        case RequestMission: {  //2
            // if mission not available or delayed, send greetingID as above
            if (m_agent->IsDelayed(pChar)) {
                // new mission offer delayed.  send no work response and exit
                agentSays->SetItem(0, new PyInt(m_agent->GetResponse(pChar, Agents::Response::NoWork)));
                agentSays->SetItem(1, PyStatic.NewNone());
            } else {
                MissionOffer offer = MissionOffer();
                m_agent->MakeOffer(pChar, offer);
                m_agent->SendMissionUpdate(call.client, "offered");
                /*  agentSays is a tuple of msgData and contentID
                 *   msgData can be single integer of briefingID, a string literal, or a tuple as defined above.
                 *   contentID is used for specific char's mission keywords. calls GetMissionKeywords()
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
                m_showObjectives = true;
            }
        } break;
        case ViewMission: { //1
            m_showObjectives = true;
            MissionOffer offer = MissionOffer();
            m_agent->GetOffer(pChar->itemID(), offer);
            agentSays->SetItem(0, new PyInt(offer.briefingID));
            agentSays->SetItem(1, new PyInt(offer.characterID));
            if (offer.stateID < Mission::State::Accepted) {
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
            } else if (offer.stateID == Mission::State::Accepted) {
                PyTuple* button1 = new PyTuple(2);
                    button1->SetItem(0, new PyInt(Quit));
                    button1->SetItem(1, new PyInt(Quit));
                dialog->AddItem(button1);
                if (call.client->IsMissionComplete(offer))  {
                    PyTuple* button2 = new PyTuple(2);
                        button2->SetItem(0, new PyInt(Complete));
                        button2->SetItem(1, new PyInt(Complete));
                    dialog->AddItem(button2);
                }
            }
        } break;
        case Accept:            //3
        case AcceptRemotely: {  //5
            MissionOffer offer = MissionOffer();
            m_agent->GetOffer(pChar->itemID(), offer);
            offer.stateID = Mission::State::Accepted;
            offer.dateAccepted = GetFileTimeNow();

            // expiry time depends on agent level and mission type
            switch (offer.typeID) {
                case Mission::Type::Courier: {
                    offer.expiryTime = GetFileTimeNow() + (sConfig.mission.AcceptExpiryTime * m_agent->GetLevel() * EvE::Time::Minute);
                } break;
                case Mission::Type::Trade:
                case Mission::Type::Mining:
                case Mission::Type::Encounter: {
                    offer.expiryTime = GetFileTimeNow() + (2 * sConfig.mission.AcceptExpiryTime * m_agent->GetLevel() * EvE::Time::Minute);
                } break;
                case Mission::Type::Arc:
                case Mission::Type::Storyline: {
                    offer.expiryTime = GetFileTimeNow() + (3 * sConfig.mission.AcceptExpiryTime * m_agent->GetLevel() * EvE::Time::Minute);
                } break;
                case Mission::Type::Research:
                case Mission::Type::Data:
                case Mission::Type::Cosmos:
                case Mission::Type::Anomic:
                case Mission::Type::Burner:
                case Mission::Type::Tutorial: {
                    offer.expiryTime = GetFileTimeNow() + (sConfig.mission.AcceptExpiryTime * m_agent->GetLevel() * EvE::Time::Minute);
                } break;
            }

            if (offer.courierTypeID) {
                // add item to players hangar
                sItemFactory.SetUsingClient(call.client);
                ItemData data(offer.courierTypeID, pChar->itemID(), locTemp, flagAutoFit, offer.courierAmount);
                InventoryItemRef iRef = sItemFactory.SpawnItem(data);
                iRef->Move(offer.originID, flagHangar, true);
                sItemFactory.UnsetUsingClient();
            }
            m_agent->UpdateOffer(pChar->itemID(), offer);
            m_agent->SendMissionUpdate(call.client, "offer_accepted");
            agentSays->SetItem(0, new PyInt(m_agent->GetResponse(pChar, Agents::Response::Accept)));
            agentSays->SetItem(1, new PyInt(pChar->itemID()));

            // TODO:  this should notify a storyline agent to offer this work.
            if (pChar->RdyForImportantMission(m_agent->GetCorpID(), m_agent->GetLevel()))
                pChar->ResetMissionCount(m_agent->GetCorpID(), m_agent->GetLevel());
        } break;
        case Complete:              //6
        case CompleteRemotely: {    //7
            //  need to verify all requirements have been met.
            MissionOffer offer = MissionOffer();
            m_agent->GetOffer(pChar->itemID(), offer);
            if (pChar->locationID() == offer.destinationID) {
                offer.stateID = Mission::State::Completed;
                offer.dateCompleted = GetFileTimeNow();
                m_agent->UpdateOffer(pChar->itemID(), offer);
                m_agent->SendMissionUpdate(call.client, "completed");
                agentSays->SetItem(0, new PyInt(m_agent->GetResponse(pChar, Agents::Response::Complete)));
                agentSays->SetItem(1, new PyInt(pChar->itemID()));
                if (offer.courierTypeID) {
                    // remove item from player possession
                    call.client->RemoveMissionItem(offer.courierTypeID, offer.courierAmount);
                }
                if (offer.rewardItemID) {
                    // add reward item to players hangar
                    sItemFactory.SetUsingClient(call.client);
                    ItemData data(offer.rewardItemID, pChar->itemID(), locTemp, flagAutoFit, offer.rewardItemQty);
                    InventoryItemRef iRef = sItemFactory.SpawnItem(data);
                    iRef->Move(m_agent->GetStationID(), flagHangar, true);
                    sItemFactory.UnsetUsingClient();
                }

                m_agent->DistributeRewards(pChar, offer);

                sStandingMgr.UpdateStandings(call.client->GetChar().get(), m_agent, Standings::MissionCompleted, offer.name, offer.important);
                m_agent->RemoveOffer(pChar->itemID());
                pChar->AddMissionTracking(m_agent->GetCorpID(), m_agent->GetLevel());
            } else {
                agentSays->SetItem(0, new PyInt(235492));  //You have to be at the drop off location to deliver the items in person'
                agentSays->SetItem(1, PyStatic.NewNone());
            }
        } break;
        case Defer: {   //10
            // extend expiry time and close
            MissionOffer offer = MissionOffer();
            if (m_agent->HasMission(pChar->itemID(), offer)) {
                offer.stateID = Mission::State::Allocated; //Defered
                offer.expiryTime += EvE::Time::Day;
                m_agent->UpdateOffer(pChar->itemID(), offer);
                m_agent->SendMissionUpdate(call.client, "prolong");
                //this isnt shown in client.  defer button calls closeWindow on click.
                agentSays->SetItem(0, new PyString("I can give you 24 hours to think about it."));    //msgInfo  -- if tuple[0].string then return msgInfo
                agentSays->SetItem(1, PyStatic.NewNone());    // ContentID  -- PyNone used when msgInfo is string to return without processing
            }
        } break;
        case Decline: { //9
            missionDeclined = true;
            MissionOffer offer = MissionOffer();
            m_agent->GetOffer(pChar->itemID(), offer);
            int64 timeleft(0);  // must send, but not used in this instance
            if (m_agent->IsDeclineCooldown(pChar, timeleft))
                sStandingMgr.UpdateStandings(call.client->GetChar().get(), m_agent, Standings::MissionDeclined, offer.name, offer.important);
            m_agent->DeclineOffer(pChar);
            m_agent->DeleteOffer(pChar->itemID());
            m_agent->SendMissionUpdate(call.client, "offer_declined");
            // mission delay timer set in GetResponse() method
            agentSays->SetItem(0, new PyInt(m_agent->GetResponse(pChar, Agents::Response::Decline)));
            agentSays->SetItem(1, new PyInt(pChar->itemID()));
            /** @todo  add lp, etc, etc  */
        } break;
        case Quit: {    //11
            missionQuit = true;
            MissionOffer offer = MissionOffer();
            m_agent->GetOffer(pChar->itemID(), offer);
            if (offer.courierTypeID) {
                // remove item from player possession
                call.client->RemoveMissionItem(offer.courierTypeID, offer.courierAmount);
            }
            // remove mission offer and set standings accordingly
            m_agent->DeleteOffer(pChar->itemID());
            m_agent->SendMissionUpdate(call.client, "quit");
            agentSays->SetItem(0, new PyInt(m_agent->GetResponse(pChar, Agents::Response::Quit)));
            agentSays->SetItem(1, new PyInt(pChar->itemID()));
            /** @todo  add lp, etc, etc  */
            sStandingMgr.UpdateStandings(call.client->GetChar().get(), m_agent, Standings::MissionFailure, offer.name, offer.important);
        } break;
        // all of these are being put into a PyTuple(2)
        // segfault if the tuple is missing
        //  NOTE:  all of these `new PyString` need objects call to PyDecRef on
        case Continue: {    //8
            // not sure what to do here.
            response = "Continue has been chosen.";
            agentSays->SetItem(0, new PyString(response));  //msgInfo  -- if tuple[0].string then return msgInfo
            agentSays->SetItem(1, PyStatic.NewNone());      // ContentID  -- PyNone used when msgInfo is string (mostly for initial greetings)
        } break;
        case StartResearch: {   //12
            // not sure what to do here.
            response = "Start Research has been chosen.";
            agentSays->SetItem(0, new PyString(response));
            agentSays->SetItem(1, PyStatic.NewNone());
        } break;
        case CancelResearch: {  //13
            // not sure what to do here.
            response = "Cancel Research has been chosen.";
            agentSays->SetItem(0, new PyString(response));
            agentSays->SetItem(1, PyStatic.NewNone());
        } break;
        case BuyDatacores: {    //14
            // not sure what to do here.
            response = "Buy Datacores has been chosen.";
            agentSays->SetItem(0, new PyString(response));
            agentSays->SetItem(1, PyStatic.NewNone());
        } break;
        case LocateCharacter:  {  //15
            Character* pChar(call.client->GetChar().get());
            float charStanding = sStandingMgr.GetEffectiveStanding(m_agent->GetID(), pChar);
            float quality = EvEMath::Agent::EffectiveQuality(m_agent->GetQuality(), pChar->GetSkillLevel(EvESkill::Negotiation), charStanding);

            // this needs an entry box for character to search
            response = "Locate Character has been chosen.";
            agentSays->SetItem(0, new PyString(response));
            agentSays->SetItem(1, PyStatic.NewNone());
            /*  cost, time, distance for search
            * lvl   fac standing    max frequency   solSystem       Constellation   Region     Other Region
            * 1            Any             5m      1k, instant           5k, 1m      n/a           n/a
            * 2            1.0             5m      5k, instant          10k, 20s   25k, 2m         n/a
            * 3            3.0             15m     10k, instant         25k, 30s   50k, 4m         100k, 8m
            * 4            5.0             30m     25k, instant         50k, 20s   100k, 2m        250k, 4m
            * 5            7.0             60m     50k, instant        100k, 10s   250k, 1m        500k, 2m
            *
            * results sent in evemail: char initiating search, initated from, target name, location.
            * no station if target in space.    if offline, last known location (not online/offline)
            * wormhole or out of area CharacterOutOfRange
            * FoundCharacterImmediately
            * TooPoorToLocate
            * BusyCantLocate
            * CharacterDoesntExist  - this will be after fee/wait
            */
            //NOTE:  when used, update service details in Agent::GetInfoServiceDetails()
        } break;
        case LocateAccept: {    //16
            // not sure what to do here.
            response = "Locate Accept has been chosen.";
            agentSays->SetItem(0, new PyString(response));
            agentSays->SetItem(1, PyStatic.NewNone());
        } break;
        case LocateReject: {    //17
            // not sure what to do here.
            response = "Locate Reject has been chosen.";
            agentSays->SetItem(0, new PyString(response));
            agentSays->SetItem(1, PyStatic.NewNone());
        } break;
        case Yes: {             //18
            // not sure what to do here.
            response = "Yes has been chosen.";
            agentSays->SetItem(0, new PyString(response));
            agentSays->SetItem(1, PyStatic.NewNone());
        } break;
        case No: {              //19
            // not sure what to do here.
            response = "No has been chosen.";
            agentSays->SetItem(0, new PyString(response));
            agentSays->SetItem(1, PyStatic.NewNone());
        } break;
        case AcceptChoice:{     //4
            // i think this is for options
            response = "Accept Choice has been chosen.";
            agentSays->SetItem(0, new PyString(response));
            agentSays->SetItem(1, PyStatic.NewNone());
        } break;
        case Admin: {           //20
            // not sure what to do here.
            response = "Admin has been chosen.";
            agentSays->SetItem(0, new PyString(response));
            agentSays->SetItem(1, PyStatic.NewNone());
        } break;
        default: {
            // error
            _log(AGENT__ERROR, "AgentBound::Handle_DoAction() - unhandled buttonID %u", actionID );
            call.client->SendErrorMsg("Unhandled Button ID.");
            PyDecRef(agentSays);
            PyDecRef(dialog);
            return nullptr;
            response = "Unknown Button ";
            agentSays->SetItem(0, new PyString(response));
            agentSays->SetItem(1, PyStatic.NewNone());
        }
    }

    if (agentSays->empty()) {
        agentSays->SetItem(0, PyStatic.NewNone());  // briefingID
        agentSays->SetItem(1, PyStatic.NewNone());  // ContentID
    }

    // extraInfo data....
    xtraInfo->SetItemString("loyaltyPoints",    new PyInt(call.client->GetLoyaltyPoints(m_agent->GetCorpID())));  // this is char current LP
    xtraInfo->SetItemString("missionCompleted", new PyBool(missionCompleted));
    xtraInfo->SetItemString("missionQuit",      new PyBool(missionQuit));
    xtraInfo->SetItemString("missionDeclined",  new PyBool(missionDeclined));

    PyTuple* inner = new PyTuple(2);
        inner->SetItem(0, agentSays);
        inner->SetItem(1, dialog);
    PyTuple* outer = new PyTuple(2);
        outer->SetItem(0, inner);
        outer->SetItem(1, xtraInfo);

    if (is_log_enabled(AGENT__RSP_DUMP)) {
        _log(AGENT__RSP_DUMP, "AgentBound::Handle_DoAction RSP:" );
        outer->Dump(AGENT__RSP_DUMP, "    ");
    }

    return outer;
}

PyResult AgentBound::Handle_GetMissionBriefingInfo(PyCallArgs &call) {
    MissionOffer offer = MissionOffer();
    if (!m_agent->HasMission(call.client->GetCharacterID(), offer))
        return PyStatic.NewNone();

    // these are found in the client data by MessageIDs ....  i.e.  {[location]objectiveDestinationID.name}
    // contentID is the key for the keywords data  (we are using charID here.  live seems to use incrementing integers)
    //  NOTE:  all of these values sent to SetItemString will need objects to call PyDecRef on.
    PyDict* keywords(new PyDict());
        keywords->SetItemString("objectiveLocationID", new PyInt(offer.originID));
        keywords->SetItemString("objectiveLocationSystemID", new PyInt(offer.originSystemID));
        keywords->SetItemString("objectiveTypeID", new PyInt(offer.courierTypeID));
        keywords->SetItemString("objectiveQuantity", new PyInt(offer.courierAmount));
        keywords->SetItemString("objectiveDestinationID", new PyInt(offer.destinationID));
        keywords->SetItemString("objectiveDestinationSystemID", new PyInt(offer.destinationSystemID));
        if (offer.rewardISK) {
            keywords->SetItemString("rewardTypeID", new PyInt(Item::Type::Credit));
            keywords->SetItemString("rewardQuantity", new PyInt(offer.rewardISK));
        } else {
            // wouldnt these be in 'extra' or ?
            keywords->SetItemString("rewardTypeID", new PyInt(offer.rewardItemID));
            keywords->SetItemString("rewardQuantity", new PyInt(offer.rewardItemQty));
        }
        keywords->SetItemString("dungeonLocationID", new PyInt(offer.dungeonLocationID));
        keywords->SetItemString("dungeonSolarSystemID", new PyInt(offer.dungeonSolarSystemID));
    PyDict *briefingInfo(new PyDict());
        briefingInfo->SetItemString("Expiration Time", new PyLong(offer.expiryTime) );
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
        //TODO:  once char decline data is complete, check it here.  if char declined from this agent in last 4 hours,
        //      time goes into "decline time".  if not, send -1 which shows the default msg about 4h decline standings hit
        int64 timeLeft(0);
        if (m_agent->IsDeclineCooldown(call.client->GetChar().get(), timeLeft)) {
            // declined a mission <4h ago.  send time left to player
            briefingInfo->SetItemString("Decline Time", new PyLong(timeLeft));
        } else {
            // all timeInterval should be sent as int64:  cant use NewNegOne() here (int)  (well, we can but client complains)
            briefingInfo->SetItemString("Decline Time", new PyLong(-1));
        }

        /*
        if (offer.stateID == Mission::State::Expired) {
            briefingInfo->SetItemString("Expiration Time", new PyLong(offer.expiryTime));
            briefingInfo->SetItemString("Decline Time", new PyLong(offer.dateCompleted));
        } else if ((offer.stateID == Mission::State::Rejected)
            or (offer.stateID == Mission::State::Completed)
            or (offer.stateID == Mission::State::Failed)) {
            briefingInfo->SetItemString("Decline Time", new PyLong(offer.dateCompleted));   // -1 is generic decline msg
        }*/

    if (is_log_enabled(AGENT__RSP_DUMP)) {
        _log(AGENT__RSP_DUMP, "AgentBound::Handle_GetMissionBriefingInfo() RSP:" );
        briefingInfo->Dump(AGENT__RSP_DUMP, "    ");
    }

    return briefingInfo;
}

PyResult AgentBound::Handle_GetMissionKeywords(PyCallArgs &call) {
    // these are the variables embedded in the messageIDs
    //self.missionArgs[contentID] = self.GetAgentMoniker(agentID).GetMissionKeywords(contentID)
    _log(AGENT__DUMP,  "AgentBound::Handle_GetMissionKeywords() - size= %lu", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    /*   none of this really matters as we're not using 'contentID' like live does
    Call_SingleArg args;
    if (!args.Decode(&call.tuple)) {
        _log(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    uint32 contentID = PyRep::IntegerValueU32(args.arg);
    if (contentID == 0)
        return PyStatic.NewNone();
    */
    MissionOffer offer = MissionOffer();
    if (!m_agent->HasMission(call.client->GetCharacterID(), offer))
        return PyStatic.NewNone();

    PyDict* keywords = new PyDict();
    keywords->SetItemString("objectiveLocationID", new PyInt(offer.originID));
    keywords->SetItemString("objectiveLocationSystemID", new PyInt(offer.originSystemID));
    keywords->SetItemString("objectiveTypeID", new PyInt(offer.courierTypeID));
    keywords->SetItemString("objectiveQuantity", new PyInt(offer.courierAmount));
    keywords->SetItemString("objectiveDestinationID", new PyInt(offer.destinationID));
    keywords->SetItemString("objectiveDestinationSystemID", new PyInt(offer.destinationSystemID));
    if (offer.rewardISK) {
        keywords->SetItemString("rewardTypeID", new PyInt(Item::Type::Credit));
        keywords->SetItemString("rewardQuantity", new PyInt(offer.rewardISK));
    } else {
        // wouldnt these be in 'extra' or ?
        keywords->SetItemString("rewardTypeID", new PyInt(offer.rewardItemID));
        keywords->SetItemString("rewardQuantity", new PyInt(offer.rewardItemQty));
    }
    keywords->SetItemString("dungeonLocationID", new PyInt(offer.dungeonLocationID));
    keywords->SetItemString("dungeonSolarSystemID", new PyInt(offer.dungeonSolarSystemID));

    if (is_log_enabled(AGENT__RSP_DUMP)) {
        _log(AGENT__RSP_DUMP, "AgentBound::Handle_GetMissionKeywords() RSP:" );
        keywords->Dump(AGENT__RSP_DUMP, "    ");
    }

    return keywords;
}

PyResult AgentBound::Handle_GetMissionObjectiveInfo(PyCallArgs &call) {
    /*  sends charID, contentID to request full objectives  (these values are the same here)
     * this is the data on the right pane of agent convo (mission objectives)
     * if offering mission,  send PyNone to only show single-pane with briefing info
     * if viewing mission, send PyDict loaded with mission info, to be displayed on right-hand side of double-pane view
     */

    _log(AGENT__DUMP,  "AgentBound::Handle_GetMissionObjectiveInfo() - size= %lu", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    // if charID, contentID are not sent, this is just to view briefing
    if (call.tuple->empty() and !m_showObjectives)
        return PyStatic.NewNone();

    MissionOffer offer = MissionOffer();
    if (m_agent->HasMission(call.client->GetCharacterID(), offer))
        return GetMissionObjectiveInfo(call.client, offer);

    m_showObjectives = false;
    return PyStatic.NewNone();
}

PyResult AgentBound::Handle_GetMyJournalDetails(PyCallArgs &call) {
    //parallelCalls.append((sm.GetService('agents').GetAgentMoniker(agentID).GetMyJournalDetails, ()))
    //missionState, importantMission, missionType, missionName, agentID, expirationTime, bookmarks, remoteOfferable, remoteCompletable = each
    // this is to update ONLY info with this agent....
    _log(AGENT__DUMP,  "AgentBound::Handle_GetMyJournalDetails() - size= %lu", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    PyTuple *tuple = new PyTuple(2);
    //missions:
    PyList* missions = new PyList();
    MissionOffer offer = MissionOffer();
    if (m_agent->HasMission(call.client->GetCharacterID(), offer)) {
        if (offer.stateID < Mission::State::Completed) {
            PyTuple* mData = new PyTuple(9);
                mData->SetItem(0, new PyInt(offer.stateID)); //missionState  .. these may be wrong also.
                mData->SetItem(1, new PyInt(offer.important?1:0)); //importantMission  -- integer boolean
                mData->SetItem(2, new PyString(sMissionDataMgr.GetTypeLabel(offer.typeID))); //missionTypeLabel
                mData->SetItem(3, new PyString(offer.name)); //missionName
                mData->SetItem(4, new PyInt(offer.agentID)); //agentID
                mData->SetItem(5, new PyLong(offer.expiryTime)); //expirationTime
                mData->SetItem(6, offer.bookmarks->Clone()); //bookmarks -- if populated, this is PyList of PyDicts as defined below...
                mData->SetItem(7, new PyBool(offer.remoteOfferable)); //remoteOfferable
                mData->SetItem(8, new PyBool(offer.remoteCompletable)); //remoteCompletable
            missions->AddItem(mData);
        }
    }
    tuple->SetItem(0, missions);

    //research:
    PyList* research = new PyList();
    tuple->SetItem(1, research);

    if (is_log_enabled(AGENT__RSP_DUMP))
        tuple->Dump(AGENT__RSP_DUMP, "   ");
    return tuple;
}

PyResult AgentBound::Handle_GetMissionJournalInfo(PyCallArgs &call) {
    //called on rclick in journal to "read details"
    //ret = self.GetAgentMoniker(agentID).GetMissionJournalInfo(charID, contentID)
    _log(AGENT__DUMP,  "AgentBound::Handle_GetMissionJournalInfo() - size= %lu", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    MissionOffer offer = MissionOffer();
    if (!m_agent->HasMission(call.client->GetCharacterID(), offer))
        return PyStatic.NewNone();

    PyDict* journalInfo = new PyDict();
    journalInfo->SetItemString("contentID", new PyInt(offer.characterID));
    journalInfo->SetItemString("missionNameID", new PyInt(offer.missionID));

    // if briefingID = 135290, send 135283 for 'read details'
    if (offer.briefingID == 135290) {
        journalInfo->SetItemString("briefingTextID", new PyInt(135283));
    } else {
        journalInfo->SetItemString("briefingTextID", new PyInt(offer.briefingID));
    }
    journalInfo->SetItemString("missionState", new PyInt(offer.stateID));
    journalInfo->SetItemString("expirationTime", new PyLong(offer.expiryTime) );
    journalInfo->SetItemString("objectives", GetMissionObjectiveInfo(call.client, offer));
    switch(offer.typeID) {
        case Mission::Type::Courier:
            journalInfo->SetItemString("missionImage", sMissionDataMgr.GetCourierRes()); break;
        case Mission::Type::Mining:
            journalInfo->SetItemString("missionImage", sMissionDataMgr.GetMiningRes()); break;
        case Mission::Type::Encounter:
            journalInfo->SetItemString("missionImage", sMissionDataMgr.GetKillRes()); break;
    }

    if (is_log_enabled(AGENT__RSP_DUMP)) {
        _log(AGENT__RSP_DUMP, "AgentBound::Handle_GetMissionJournalInfo() RSP:" );
        journalInfo->Dump(AGENT__RSP_DUMP, "    ");
    }

    return journalInfo;
}

PyDict* AgentBound::GetMissionObjectiveInfo(Client* pClient, MissionOffer& offer)
{
    PyDict* objectiveData = new PyDict();
    objectiveData->SetItemString("missionTitleID", new PyInt(offer.missionID));
    objectiveData->SetItemString("contentID", new PyInt(offer.characterID));
    objectiveData->SetItemString("importantStandings", new PyInt(offer.important));     // boolean integer
    // will need to test for this to set correctly.....
    if (pClient->IsMissionComplete(offer)) {     // Mission::Status:: data here 0=no, 1=yes, 2=cheat
        objectiveData->SetItemString("completionStatus", new PyInt(Mission::Status::Complete));
    } else {
        objectiveData->SetItemString("completionStatus", new PyInt(Mission::Status::Incomplete));
    }
    objectiveData->SetItemString("missionState", new PyInt(offer.stateID /*Mission::State::Offered*/));   // Mission::State:: data here for agentGift populating.  Accepted/failed to display gift items as accepted
    objectiveData->SetItemString("loyaltyPoints", new PyInt(0));
    objectiveData->SetItemString("researchPoints", new PyInt(0));

    //  this puts title/msg at bottom of right pane...incomplete
    /*
    switch (offer.stateID) {
        case Mission::State::Accepted: {
            if (offer.typeID == Mission::Type::Courier) {
                PyTuple* missionExtra = new PyTuple(2);  // this is tuple(2)  headerID, bodyID    -- std locale msgIDs
                    missionExtra->SetItem(0, new PyString("Reminder...."));   // this should be separate title from mission name
                    missionExtra->SetItem(1, new PyString("Remember to get the %s from your hangar before you leave.", pClient->GetCourierItemRef(m_agent->GetID())->name()));   // this is additional info about mission, etc.
                objectiveData->SetItemString("missionExtra", missionExtra);
            }
        } break;
    } */

    PyList* locList = new PyList();    // tuple of list of locationIDs (pickup and dropoff)
        locList->AddItem(new PyInt(offer.originSystemID));
        locList->AddItem(new PyInt(offer.destinationSystemID));
    objectiveData->SetItemString("locations", locList);

    PyList* giftList = new PyList();    // this is list of tuple(3)  typeID, quantity, extra
    /*
    PyDict* extra = new PyDict();    // 'extra' is either specificItemID or blueprint data.
        extra->SetItemString("specificItemID", PyStatic.NewNone());
        extra->SetItemString("blueprintInfo", PyStatic.NewNone());
    PyTuple* agentGift = new PyTuple(3);
        agentGift->SetItem(0, PyStatic.NewNone());
        agentGift->SetItem(1, PyStatic.NewNone());
        agentGift->SetItem(2, extra);
        giftList->AddItem(agentGift);
    */
    objectiveData->SetItemString("agentGift", giftList);

    PyList* normList = new PyList();    // this is list of tuple(3)  typeID, quantity, extra
    if (offer.rewardISK) {
        PyDict* extra = new PyDict();    // 'extra' is either specificItemID or blueprint data.
            //extra->SetItemString("specificItemID", PyStatic.NewNone());
            //extra->SetItemString("blueprintInfo", PyStatic.NewNone());
        PyTuple* normalRewards = new PyTuple(3);
            normalRewards->SetItem(0, new PyInt(Item::Type::Credit));
            normalRewards->SetItem(1, new PyInt(offer.rewardISK));
            normalRewards->SetItem(2, extra);
        normList->AddItem(normalRewards);
    }
    if (offer.rewardItemID) {
        PyDict* extra = new PyDict();    // 'extra' is either specificItemID or blueprint data.
            //extra->SetItemString("specificItemID", PyStatic.NewNone());
            //extra->SetItemString("blueprintInfo", PyStatic.NewNone());
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
        extra->SetItemString("specificItemID", PyStatic.NewNone());
        extra->SetItemString("blueprintInfo", PyStatic.NewNone());
    PyTuple* collateral = new PyTuple(3);
        collateral->SetItem(0, PyStatic.NewNone());
        collateral->SetItem(1, PyStatic.NewNone());
        collateral->SetItem(2, extra);
        */
    objectiveData->SetItemString("collateral", collateralList);

    PyList* bonusList = new PyList();   // this is list of tuple(4)  timeRemaining, typeID, quantity, extra
    if (offer.bonusTime > 0) {
        PyDict* extra = new PyDict();    // 'extra' is either specificItemID or blueprint data.
            //extra->SetItemString("specificItemID", PyStatic.NewNone());
            //extra->SetItemString("blueprintInfo", PyStatic.NewNone());
        PyTuple* bonusRewards = new PyTuple(4);
        if (offer.dateAccepted > 0) {
            bonusRewards->SetItem(0, new PyLong((offer.bonusTime * EvE::Time::Minute) - (GetFileTimeNow() - offer.dateAccepted)));  // bonus time * minutes - (filetime)elapsed time
        } else {
            bonusRewards->SetItem(0, new PyLong(offer.bonusTime * EvE::Time::Minute));  // bonus time * minutes
        }
            bonusRewards->SetItem(1, new PyInt(Item::Type::Credit));   // bonus is *usually* isk.  for now, we'll keep it as isk (easier)
            bonusRewards->SetItem(2, new PyInt(offer.bonusISK));
            bonusRewards->SetItem(3, extra);
        bonusList->AddItem(bonusRewards);
    }
    // bonusList can be multiple items, usually only item or isk for time bonus
    if (false/*bonus2*/) {
        PyDict* extra = new PyDict();    // 'extra' is either specificItemID or blueprint data.
            //extra->SetItemString("specificItemID", PyStatic.NewNone());
            //extra->SetItemString("blueprintInfo", PyStatic.NewNone());
        PyTuple* bonusRewards2 = new PyTuple(4);
            bonusRewards2->SetItem(0, new PyLong(2 * EvE::Time::Minute)); //20m
            bonusRewards2->SetItem(1, new PyInt(Item::Type::Tritanium));
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

    objectiveData->SetItemString("objectives", GetMissionObjectives(pClient, offer));
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

    if (is_log_enabled(AGENT__RSP_DUMP)) {
        _log(AGENT__RSP_DUMP, "AgentBound::Handle_GetMissionObjectiveInfo() RSP:" );
        objectiveData->Dump(AGENT__RSP_DUMP, "    ");
    }

    return objectiveData;
}

PyTuple* AgentBound::GetMissionObjectives(Client* pClient, MissionOffer& offer)
{
    // set mission objectiveData based on mission type.
    PyDict* dropoffLocation = new PyDict();
    if (sDataMgr.IsStation(offer.destinationID)) {
        dropoffLocation->SetItemString("typeID", new PyInt(offer.destinationTypeID) );
        dropoffLocation->SetItemString("locationID", new PyInt(offer.destinationID) );
        dropoffLocation->SetItemString("solarsystemID", new PyInt(offer.destinationSystemID) );
    } else {
        dropoffLocation->SetItemString("shipTypeID", new PyInt(offer.destinationTypeID) );
        dropoffLocation->SetItemString("agentID", new PyInt(offer.destinationOwnerID) );
        // get agent in space location and set here
        PyTuple* coords = new PyTuple(3);
            coords->SetItem(0, new PyFloat(0)); //x
            coords->SetItem(1, new PyFloat(0)); //y
            coords->SetItem(2, new PyFloat(0)); //z
        dropoffLocation->SetItemString("coords", coords);
        dropoffLocation->SetItemString("referringAgentID", new PyInt(offer.agentID) );
    }

    PyTuple* objectives = new PyTuple(1);
    switch (offer.typeID) {
        case Mission::Type::Trade:
        case Mission::Type::Courier: {
            PyDict* pickupLocation = new PyDict();
                pickupLocation->SetItemString("typeID", new PyInt(m_agent->GetLocTypeID()) );
                pickupLocation->SetItemString("locationID", new PyInt(offer.originID) );
                pickupLocation->SetItemString("solarsystemID", new PyInt(offer.originSystemID) );
            PyDict* cargo = new PyDict();
                cargo->SetItemString("hasCargo", new PyBool(pClient->ContainsTypeQty(offer.courierTypeID, offer.courierAmount)));
                cargo->SetItemString("typeID", new PyInt(offer.courierTypeID));
                cargo->SetItemString("quantity", new PyInt(offer.courierAmount));
                cargo->SetItemString("volume", new PyFloat(offer.courierItemVolume * offer.courierAmount));    // calculated shipment volume.  *this is direct to window*
            PyTuple* objData = new PyTuple(5);
                objData->SetItem(0, new PyInt(offer.originOwnerID));
                objData->SetItem(1, pickupLocation/*m_agent->GetLocationWrap()*/);
                objData->SetItem(2, new PyInt(offer.destinationOwnerID));
                objData->SetItem(3, dropoffLocation);
                objData->SetItem(4, cargo);
            PyTuple* objType = new PyTuple(2);   // this is list of tuple(2)    objType, objData
                objType->SetItem(0, new PyString("transport"));
                objType->SetItem(1, objData);
            objectives->SetItem(0, objType);
        } break;
        case Mission::Type::Encounter:
        case Mission::Type::Mining: {
            PyDict* cargo = new PyDict();
                cargo->SetItemString("hasCargo", new PyBool(pClient->ContainsTypeQty(offer.courierTypeID, offer.courierAmount)));
                cargo->SetItemString("typeID", new PyInt(offer.courierTypeID));
                cargo->SetItemString("quantity", new PyInt(offer.courierAmount));
                cargo->SetItemString("volume", new PyFloat(offer.courierItemVolume * offer.courierAmount));    // calculated shipment volume.  *this is direct to window*
            PyTuple* objData = new PyTuple(3);
                objData->SetItem(0, new PyInt(offer.destinationOwnerID));
                objData->SetItem(1, dropoffLocation/*m_agent->GetLocationWrap()*/);
                objData->SetItem(2, cargo);
            PyTuple* objType = new PyTuple(2);   // this is list of tuple(2)    objType, objData
                objType->SetItem(0, new PyString("fetch"));
                objType->SetItem(1, objData);
            objectives->SetItem(0, objType);
        } break;
        case Mission::Type::Anomic:
        case Mission::Type::Arc:
        case Mission::Type::Burner:
        case Mission::Type::Cosmos:
        case Mission::Type::Data:
        case Mission::Type::Research:
        case Mission::Type::Storyline:
        case Mission::Type::Tutorial: {
            objectives->SetItem(0, PyStatic.NewNone());
        } break;
    }

    PySafeDecRef(dropoffLocation);

    return objectives;

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
}


/**     ***********************************************************************
 * @note   these do absolutely nothing at this time....
 */

PyResult AgentBound::Handle_GetDungeonShipRestrictions(PyCallArgs &call) {
    //restrictions = self.GetAgentMoniker(agentID).GetDungeonShipRestrictions(dungeonID)
    _log(AGENT__DUMP,  "AgentBound::Handle_GetDungeonShipRestrictions() - size= %lu", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    return nullptr;
}

PyResult AgentBound::Handle_RemoveOfferFromJournal(PyCallArgs &call) {
    //called on rclick in journal to "remove offer"
    //self.GetAgentMoniker(agentID).RemoveOfferFromJournal()
    _log(AGENT__DUMP,  "AgentBound::Handle_RemoveOfferFromJournal() - size= %lu", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    return nullptr;
}

PyResult AgentBound::Handle_GetOfferJournalInfo(PyCallArgs &call) {
    //html = self.GetAgentMoniker(agentID).GetOfferJournalInfo()
    _log(AGENT__DUMP,  "AgentBound::Handle_GetOfferJournalInfo() - size= %lu", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    return nullptr;
}

PyResult AgentBound::Handle_GetEntryPoint(PyCallArgs &call) {
    //entryPoint = sm.StartService('agents').GetAgentMoniker(bookmark.agentID).GetEntryPoint()
    _log(AGENT__DUMP,  "AgentBound::Handle_GetEntryPoint() - size= %lu", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    return nullptr;
}

PyResult AgentBound::Handle_GotoLocation(PyCallArgs &call) {
    //sm.StartService('agents').GetAgentMoniker(bookmark.agentID).GotoLocation(bookmark.locationType, bookmark.locationNumber, referringAgentID)
    _log(AGENT__DUMP,  "AgentBound::Handle_GotoLocation() - size= %lu", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    return nullptr;
}

PyResult AgentBound::Handle_WarpToLocation(PyCallArgs &call) {
    //sm.StartService('agents').GetAgentMoniker(bookmark.agentID).WarpToLocation(bookmark.locationType, bookmark.locationNumber, warpRange, fleet, referringAgentID)
    _log(AGENT__DUMP,  "AgentBound::Handle_WarpToLocation() - size= %lu", call.tuple->size() );
    call.Dump(AGENT__DUMP);

    return nullptr;
}
