
 /**
  * @name Agent.h
  *   agent specific code
  *    original agent code by zhur, this was completely rewritten based on new data.
  *
  * @Author:        Allan
  * @date:      19 June 2018
  *
  */

#ifndef _EVE_SERVER_AGENT_H
#define _EVE_SERVER_AGENT_H


#include "agents/AgentDB.h"
#include "missions/MissionDataMgr.h"

class Client;

class Agent {
public:
    Agent(uint32 id);
    ~Agent()                                            { /* do nothing here */ }

    bool Load();

    PyDict* GetLocationWrap();
    PyObject* GetInfoServiceDetails(Client* pClient);

    bool IsLocator()                                    { return m_agentData.locator; }
    bool IsResearch()                                   { return m_agentData.research; }
    bool IsStoryline()                                  { return m_agentData.storyline; }

    uint8 GetLevel()                                    { return m_agentData.level; }
    int8 GetQuality()                                   { return m_agentData.quality; }

    uint32 GetID()                                      { return m_agentID; }
    uint8  GetTypeID()                                  { return m_agentData.typeID; }
    uint32 GetCorpID()                                  { return m_agentData.corporationID; }
    uint32 GetSystemID()                                { return m_agentData.solarSystemID; }
    uint32 GetStationID()                               { return m_agentData.stationID; }
    uint32 GetLocTypeID()                               { return m_agentData.locationTypeID; }
    uint32 GetFactionID()                               { return m_agentData.factionID; }
    uint32 MakeButtonID()                               { return ++m_buttonID; }

    std::string GetName()                               { return m_agentData.name; }

    bool HasMission(uint32 charID);
    bool HasMission(uint32 charID, MissionOffer& offer);
    bool HasCurrentMission(uint32 charID);

    void MakeOffer(Character* pChar, MissionOffer& offer );
    void GetOffer(uint32 charID, MissionOffer& offer);
    void UpdateOffer(uint32 charID, MissionOffer& offer);
    void DeleteOffer(uint32 charID);    // completely deletes offer from agent data, missionMgr data, and db
    void RemoveOffer(uint32 charID);    // removes offer from agent data
    void DeclineOffer(Character* pChar);    // makes note and removes offer from agent data

    void SendMissionUpdate(Client* pClient, std::string action);

    bool CanUseAgent(Client* pClient);

    // divys out isk and lp rewards among mission acceptor and fleet members, if any
    void DistributeRewards(Character* pChar, MissionOffer& offer);

    // all numbered agent responses here.  rspID found in EVE_Agent.h Agents::Response::xxx
    uint32 GetResponse(Character* pChar, uint8 rspID);
    std::string GetTextResponse(Character* pChar, uint8 rspID);

    // if char as declined mission from this agent in last 4 hours, return true.
    // this is used for sending decline time info on mission data page
    bool IsDeclineCooldown(Character* pChar, int64& timeLeft);  // if no decline <4h, timeLeft = 0, else time is sent in winFileTime

    // if char has declined mission recently, delay next offer
    bool IsDelayed(Character* pChar);

protected:
    std::map<uint16, uint8>             m_skills;       // skillID/level
    std::map<uint32, MissionOffer>      m_offers;       // charID/data      -- shouldnt this be in mission data??
    std::map<uint16, AgentActions>      m_actions;      // buttonID/data

private:
    bool                m_important;
    uint16              m_buttonID;
    const uint32        m_agentID;
    AgentData           m_agentData;

    // map of declined missions by char.  this is saved and loaded with agent
    std::map<uint32, int64>             m_declineMap;   // charID/declineTimer (filetime)
    // used to delay next mission after decline based on standings
    std::map<uint32, int64>             m_delayMap;     // charID/delaytime (filetime)

};

#endif  // _EVE_SERVER_AGENT_H
/*
        tmp = wnd.sr.agentMoniker.DoAction(actionID)
        if wnd is None or wnd.destroyed or wnd.sr is None:
            return (None, None, None)
        ret, wnd.sr.oob = tmp
        agentSays, wnd.sr.dialogue = ret
        if actionID is None and len(wnd.sr.dialogue):
            self.LogInfo('Agent Service: Started a new conversation with an agent and successfully retrieved dialogue options.')
            firstActionID = wnd.sr.dialogue[0][0]
            firstActionDialogue = wnd.sr.dialogue[0][1]

            if firstActionDialogue == const.agentDialogueButtonRequestMission and not agentHasLocatorService and not isResearchAgent:
                self.LogInfo("Agent Service: Automatically executing the 'Ask' dialogue action for the player.")
                tmp = wnd.sr.agentMoniker.DoAction(firstActionID)
                if wnd is None or wnd.destroyed or wnd.sr is None:
                    return (None, None, None)
                ret, wnd.sr.oob = tmp
                agentSays, wnd.sr.dialogue = ret
        wnd.sr.agentSays = self.ProcessMessage(agentSays, wnd.sr.agentID)
        return (wnd.sr.agentSays, wnd.sr.dialogue, wnd.sr.oob)
            */
/*
 *                    for data in details.services:
 *                        serviceInfo = sm.GetService('agents').ProcessAgentInfoKeyVal(data)
 *                        for entry in serviceInfo:
 *                            wnd.sr.data[C_AGENTINFOTAB]['items'].append(listentry.Get('Header', {'label': entry[0]}))
 *                            for entryDetails in entry[1]:
 *                                wnd.sr.data[C_AGENTINFOTAB]['items'].append(listentry.Get('LabelTextTop', {'line': 1,
 *                                 'label': entryDetails[0],
 *                                 'text': entryDetails[1]}))
 *
 *    def _ProcessResearchServiceInfo(self, data):
 *        header = localization.GetByLabel('UI/Agents/Research/ResearchServices', session.languageID)
 *        skillList = []
 *        for skillTypeID, skillLevel in data.skills:
 *            skillList.append(localization.GetByLabel('UI/Agents/Research/SkillListing', session.languageID, skillID=skillTypeID, skillLevel=skillLevel))
 *
 *        if not skillList:
 *            skills = localization.GetByLabel('UI/Agents/Research/ErrorNoRelevantResearchSkills', session.languageID)
 *        else:
 *            skillList = localizationUtil.Sort(skillList)
 *            skills = localizationUtil.FormatGenericList(skillList)
 *        details = [(localization.GetByLabel('UI/Agents/Research/RelevantSkills', session.languageID), skills)]
 *        status = []
 *        if data.researchData:
 *            researchData = data.researchData
 *            researchStuff = [(localization.GetByLabel('UI/Agents/Research/ResearchField', session.languageID), cfg.invtypes.Get(researchData['skillTypeID']).name), (localization.GetByLabel('UI/Agents/Research/CurrentStatus', session.languageID), localization.GetByLabel('UI/Agents/Research/CurrentStatusRP', session.languageID, rpAmount=researchData['points'])), (localization.GetByLabel('UI/Agents/Research/ResearchRate', session.languageID), localization.GetByLabel('UI/Agents/Research/ResearchRateRPDay', session.languageID, rpAmount=researchData['pointsPerDay']))]
 *            rpMultiplier = researchData['rpMultiplier']
 *            if rpMultiplier > 1:
 *                researchStuff.append((localization.GetByLabel('UI/Agents/Research/ResearchFieldBonus', session.languageID), localization.GetByLabel('UI/Agents/Research/ResearchFieldBonusRPMultiplier', session.languageID, rpMultiplier=rpMultiplier)))
 *            status = [(localization.GetByLabel('UI/Agents/Research/YourResearch', session.languageID), researchStuff)]
 *        research = []
 *        for skillTypeID, pattentIDs in data.researchSummary:
 *            predictablePatentNames = []
 *            for blueprintTypeID in pattentIDs:
 *                predictablePatentNames.append(cfg.invtypes.Get(blueprintTypeID).name)
 *
 *            if predictablePatentNames:
 *                predictablePatentNames = localizationUtil.Sort(predictablePatentNames)
 *                predictablePatents = localizationUtil.FormatGenericList(predictablePatentNames)
 *            else:
 *                predictablePatents = localization.GetByLabel('UI/Agents/Research/NoPredictablePatents', session.languageID)
 *            research.append((localization.GetByLabel('UI/Agents/Research/ResearchSummary', session.languageID, skillTypeID=skillTypeID), [(localization.GetByLabel('UI/Agents/Research/PredictablePatents', session.languageID), predictablePatents)]))
 *
 *        return [(header, details)] + status + research
 *
 *
 *    def _ProcessLocateServiceInfo(self, data):
 *        header = localization.GetByLabel('UI/Agents/Locator/LocationServices', session.languageID)
 *        if data.frequency:
 *            details = [(localization.GetByLabel('UI/Agents/Locator/MaxFrequency', session.languageID), localization.GetByLabel('UI/Agents/Locator/EveryInterval', session.languageID, interval=data.frequency))]
 *        else:
 *            details = [(localization.GetByLabel('UI/Agents/Locator/MaxFrequency', session.languageID), localization.GetByLabel('UI/Generic/NotAvailableShort', session.languageID))]
 *        for delayRange, delay, cost in data.delays:
 *            rangeText = [localization.GetByLabel('UI/Agents/Locator/SameSolarSystem', session.languageID),
 *             localization.GetByLabel('UI/Agents/Locator/SameConstellation', session.languageID),
 *             localization.GetByLabel('UI/Agents/Locator/SameRegion', session.languageID),
 *             localization.GetByLabel('UI/Agents/Locator/DifferentRegion', session.languageID)][delayRange]
 *            if not delay:
 *                delay = localization.GetByLabel('UI/Agents/Locator/ResultsInstantaneous', session.languageID)
 *            else:
 *                delay = util.FmtTimeInterval(delay * SEC)
 *            details.append((rangeText, localizationUtil.FormatGenericList((util.FmtISK(cost), delay))))
 *
 *        if data.callbackID:
 *            details.append((localization.GetByLabel('UI/Agents/Locator/Availability', session.languageID), localization.GetByLabel('UI/Agents/Locator/NotAvailableInProgress', session.languageID)))
 *        elif data.lastUsed and blue.os.GetWallclockTime() - data.lastUsed < data.frequency:
 *            details.append((localization.GetByLabel('UI/Agents/Locator/AvailableAgain', session.languageID), util.FmtDate(data.lastUsed + data.frequency)))
 *        return [(header, details)]
 */

/*
switch (m_agentData.divisionID) {
    case Corp::Division::Accounting:
    case Corp::Division::Administration:
    case Corp::Division::Advisory:
    case Corp::Division::Archives:
    case Corp::Division::Astrosurveying:
    case Corp::Division::Command:
    case Corp::Division::Distribution:
    case Corp::Division::Financial:
    case Corp::Division::Intelligence:
    case Corp::Division::InternalSecurity:
    case Corp::Division::Legal:
    case Corp::Division::Manufacturing:
    case Corp::Division::Marketing:
    case Corp::Division::Mining:
    case Corp::Division::Personnel:
    case Corp::Division::Production:
    case Corp::Division::PublicRelations:
    case Corp::Division::RnD:
    case Corp::Division::Security:
    case Corp::Division::Storage:
    case Corp::Division::Surveillance:
    case Corp::Division::DistributionNew:
    case Corp::Division::MiningNew:
    case Corp::Division::SecurityNew: {
    } break;
}
*/