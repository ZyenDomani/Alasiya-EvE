
 /**
  * @name StandingMgr.cpp
  *   methods and functions relating to managing, manipulation of standings and saving standings data
  *
  * TODO;  this class should handle all standings changes.  rewrite to do this!
  *   NO standings should be calc'd, modified or set elsewhere
  *
  * @Author:        Allan
  * @date:      14 November 2018
  * @update:    02 March 2025
  *
  */


#include "fleet/FleetManager.h"
#include "system/SystemManager.h"
#include "StandingMgr.h"
#include "agents/Agent.h"
#include "character/Character.h"

/*
 * STANDING__ERROR
 * STANDING__WARNING
 * STANDING__MESSAGE
 * STANDING__DEBUG
 * STANDING__INFO
 * STANDING__TRACE
 * STANDING__DUMP
 * STANDING__RSPDUMP
 */


StandingMgr::StandingMgr()
: m_factionStandings(nullptr)
{
}

void StandingMgr::Clear() {
    PySafeDecRef(m_factionStandings);
}

int StandingMgr::Initialize() {
    Populate();
    sLog.Blue("      StandingMgr", "Standings Manager Initialized.");
    return 1;
}

void StandingMgr::GetInfo() {
    // not sure what im doing here...
    // loaded standing counts?  faction->corp/char, corp->corp/char, agent->corp/char
}

void StandingMgr::Populate() {
    double start = GetTimeMSeconds();

    m_factionStandings = StandingDB::GetFactionStandings();
    if (m_factionStandings == nullptr)
        sLog.Error("      StandingMgr", "m_factionStandings is null");
    PySafeIncRef(m_factionStandings);

    DBResultRow row;
    DBQueryResult res;
    // this covers all faction, corp and character standings
    StandingDB::GetAllStandings(res);
    while (res.GetRow(row)) {
        // fromID,toID,standing
        SetStanding(row.GetUInt(0), row.GetUInt(1), row.GetFloat(2));
    }

    sLog.Cyan("      StandingMgr", "%lu Standing Data Sets loaded in %.3fms.", m_standings.size(), (GetTimeMSeconds() - start));
}

void StandingMgr::SetStanding(uint32 fromID, uint32 toID, float value) {
    std::map<uint32, std::map<uint32, float>>::iterator itr = m_standings.find(fromID);
    if (itr == m_standings.end()) {
        standingData data;
        data[toID] = value;
        m_standings[fromID] = data;
    } else {
        standingData::iterator itr2 = itr->second.find(toID);
        if (itr2 == itr->second.end()) {
            itr->second[toID] = value;
        } else {
            itr2->second = value;
        }
    }

    itr = m_standingsRevered.find(toID);
    if (itr == m_standingsRevered.end()) {
        standingData data;
        data[fromID] = value;
        m_standingsRevered[toID] = data;
    } else {
        standingData::iterator itr2 = itr->second.find(fromID);
        if (itr2 == itr->second.end()) {
            itr->second[fromID] = value;
        } else {
            itr2->second = value;
        }
    }
}

float StandingMgr::GetRawStanding(uint32 fromID, uint32 toID) {
    std::map<uint32, std::map<uint32, float>>::iterator itr = m_standings.find(fromID);
    if (itr != m_standings.end()) {
        standingData::iterator itr2 = itr->second.find(toID);
        if (itr2 != itr->second.end())
            return itr2->second;
    }

    return 0.0f;
}

float StandingMgr::GetEffectiveStanding(uint32 fromID, Character* pChar) {
    float standing(0.0f);
    uint8 sConn = pChar->GetSkillLevel(EvESkill::Connections);
    uint8 sDiplo = pChar->GetSkillLevel(EvESkill::Diplomacy);
    uint8 sCrim = pChar->GetSkillLevel(EvESkill::CriminalConnections);

    if (IsFactionID(fromID)) {
        float facChr = GetRawStanding(fromID, pChar->itemID());
        float facBonus = EvEMath::Agent::GetStandingBonus(facChr, fromID, sConn, sDiplo, sCrim);
        standing = EvEMath::Agent::EffectiveStanding(facChr, facBonus);
    } else if (IsNPCCorp(fromID)) {
        float corpChr = GetRawStanding(fromID, pChar->itemID());
        float corpBonus = EvEMath::Agent::GetStandingBonus(corpChr, sDataMgr.GetCorpFaction(fromID), sConn, sDiplo, sCrim);
        standing = EvEMath::Agent::EffectiveStanding(corpChr, corpBonus);
    } else if (IsAgent(fromID)) {
        float charChr = GetRawStanding(fromID, pChar->itemID());
        uint32 corpID = sDataMgr.GetAgentCorpID(fromID);
        float charBonus = EvEMath::Agent::GetStandingBonus(charChr, sDataMgr.GetCorpFaction(corpID), sConn, sDiplo, sCrim);
        standing = EvEMath::Agent::EffectiveStanding(charChr, charBonus);
    } else {
        // error
        sLog.Error("StandingMgr::GetEffectiveStanding", "Something sent %u, not agent, not corp, not faction", fromID);
    }

    return standing;
}

void StandingMgr::UpdateStandings(uint32 fromID, uint32 toID, uint16 eventType, float pctChange, std::string msg) {
    float currentStanding(GetRawStanding(fromID, toID));
    float newStanding(0.0f);
    if (pctChange > 0.0f) {
        newStanding = ((10.0f - currentStanding) * pctChange);
        newStanding += currentStanding;
    } else {
        newStanding = ((-10.0f - currentStanding) * pctChange);
        newStanding += currentStanding;
    }

    SetStanding(fromID, toID, newStanding);

    StandingDB::UpdateStanding(fromID, toID, newStanding);
    StandingDB::SaveStandingChanges(fromID, toID, eventType, pctChange, msg);

    // do derived standings
    UpdateDerivedStandings(fromID, toID, eventType, pctChange, msg);
}

void StandingMgr::UpdateDerivedStandings(uint32 fromID, uint32 toID, uint16 eventType, float pctChange, std::string msg) {
    float newStanding(0.0f);
    //figure faction/corp relations and set derived accordingly

    // update derived standings based on views to original entity and toID
    float currentStanding(GetRawStanding(fromID, toID));
/*
    // save data
    SetStanding(fromID, toID, newStanding);

    StandingDB::UpdateStanding(fromID, toID, newStanding);
    StandingDB::SaveStandingChanges(fromID, toID, eventType, pctChange, msg);
*/
}

void StandingMgr::UpdateStandings(Character* pChar, Agent* pAgent, uint8 eventID, std::string missionName, bool important/*false*/) {
    uint32 charID(pChar->itemID());
    float charStanding = GetEffectiveStanding(pAgent->GetID(), pChar);
    float quality = EvEMath::Agent::EffectiveQuality(pAgent->GetQuality(), pChar->GetSkillLevel(EvESkill::Negotiation), charStanding);
    float pctChange = EvEMath::Agent::StandingChange(pAgent->GetLevel(), quality);

    SystemManager* pSysMgr = sEntityMgr.FindOrBootSystem(pAgent->GetSystemID());

    // set bonuses/penalties based on type of standing change
    switch (eventID) {
        case Standings::MissionFailedRollback: {
            pctChange *= sConfig.standings.MissionFailedRollback;
        } break;
        case Standings::MissionOfferExpired: {
            pctChange *= sConfig.standings.MissionOfferExpired;
        } break;
        case Standings::MissionBonus: {
            pctChange *= sConfig.standings.MissionBonus;
            if (pSysMgr != nullptr)
                pctChange *= (1.0f + pSysMgr->GetSecValue());
        } break;
        case Standings::MissionCompleted: {
            pctChange *= sConfig.standings.BaseMissionMultiplier;
            pctChange *= sConfig.standings.MissionCompleted;
            if (pSysMgr != nullptr)
                pctChange *= (1.0f + pSysMgr->GetSecValue());
            if (important)
                pctChange *= sConfig.standings.ImportantMissionBonus;

            pctChange *= (1.0f + (0.04f * pChar->GetSkillLevel(EvESkill::Social)));
        } break;
        case Standings::MissionDeclined: {
            // this will come from agent after finding another mission declined in previous 4h
            pctChange *= sConfig.standings.MissionDeclined;
        } break;
        case Standings::MissionFailure: {
            pctChange *= sConfig.standings.MissionFailure;
        } break;
    }


    std::string msg = missionName;
    msg += " from ";
    msg += pAgent->GetName();

    if (pSysMgr != nullptr) {
        msg += " in ";
        msg += pSysMgr->GetNameStr();
    }

    if (IsFleetID(pChar->fleetID()) and (pctChange > 0)) {
        pctChange *= sConfig.standings.FleetMissionMultiplier;  // live does half, we do 75%.
        // shared mission standings are from agent & agent's corp to character only, no char corp or derived (yet)
        // no faction, basic only
        std::vector<Client*> clientVec;
        sFltSvc.GetFleetClientsInSystem(pChar->GetClient(), clientVec);
        for (auto &cur : clientVec) {
            UpdateStandings(pAgent->GetID(), cur->GetCharacterID(), eventID, pctChange, msg);
            PyTuple* agent = new PyTuple(5);
                agent->SetItem(0, new PyInt(pAgent->GetID()));
                agent->SetItem(1, new PyInt(cur->GetCharacterID()));
                agent->SetItem(2, new PyFloat(pctChange));
                agent->SetItem(3, PyStatic.NewNegOne());
                agent->SetItem(4, PyStatic.NewOne());
            PyTuple* corp = new PyTuple(5);
                corp->SetItem(0, new PyInt(pAgent->GetCorpID()));
                corp->SetItem(1, new PyInt(charID));
                float change = pctChange * sConfig.standings.ACorp2CharMissionMultiplier;
                corp->SetItem(2, new PyFloat(change));
                corp->SetItem(3, PyStatic.NewNegOne());
                corp->SetItem(4, PyStatic.NewOne());
            PyList* list = new PyList();
                list->AddItem(agent);
                list->AddItem(corp);
            PyTuple* payload = new PyTuple(1);
                payload->SetItem(0, list);
            cur->SendNotification("OnStandingsModified", "charid", payload, false);
            // fleet will share corp standings on some missions.  fix later.
        }
    }

    UpdateStandings(pAgent->GetID(), charID, eventID, pctChange, msg);
    float change = pctChange * sConfig.standings.ACorp2CharMissionMultiplier;
    UpdateStandings(pAgent->GetCorpID(), charID, eventID, change, msg);
    if (important) {
        change = pctChange * sConfig.standings.AFaction2CharMissionMultiplier;
        UpdateStandings(pAgent->GetFactionID(), charID, eventID, change, msg);
    }

    if (IsPlayerCorp(pChar->GetClient()->GetCorporationID())) {
        change = pctChange * sConfig.standings.Agent2PCorpMissionMultiplier;
        UpdateStandings(pAgent->GetID(), pChar->GetClient()->GetCorporationID(), eventID, change, msg);
        change = pctChange * sConfig.standings.ACorp2PCorpMissionMultiplier;
        UpdateStandings(pAgent->GetCorpID(), pChar->GetClient()->GetCorporationID(), eventID, change, msg);
        if (important) {
            change = pctChange * sConfig.standings.AFaction2PCorpMissionMultiplier;
            UpdateStandings(pAgent->GetFactionID(), pChar->GetClient()->GetCorporationID(), eventID, change, std::move(msg));
        }
    }

    PyTuple* agent = new PyTuple(5);
        agent->SetItem(0, new PyInt(pAgent->GetID()));
        agent->SetItem(1, new PyInt(charID));
        agent->SetItem(2, new PyFloat(pctChange));
        agent->SetItem(3, PyStatic.NewNegOne());
        agent->SetItem(4, PyStatic.NewOne());
    PyTuple* corp = new PyTuple(5);
        corp->SetItem(0, new PyInt(pAgent->GetCorpID()));
        corp->SetItem(1, new PyInt(charID));
        change = pctChange * sConfig.standings.ACorp2CharMissionMultiplier;
        corp->SetItem(2, new PyFloat(change));
        corp->SetItem(3, PyStatic.NewNegOne());
        corp->SetItem(4, PyStatic.NewOne());
    PyList* list = new PyList();
        list->AddItem(agent);
        list->AddItem(corp);
    if (important) {
        PyTuple* faction = new PyTuple(5);
            faction->SetItem(0, new PyInt(pAgent->GetFactionID()));
            faction->SetItem(1, new PyInt(charID));
            change = pctChange * sConfig.standings.AFaction2CharMissionMultiplier;
            faction->SetItem(2, new PyFloat(change));
            faction->SetItem(3, PyStatic.NewNegOne());
            faction->SetItem(4, PyStatic.NewOne());
        list->AddItem(faction);
    }
    PyTuple* payload = new PyTuple(1);
    payload->SetItem(0, list);

    if (is_log_enabled(STANDING__RSPDUMP)) {
        _log(STANDING__RSPDUMP, "Agent::UpdateStandings RSP:" );
        payload->Dump(STANDING__RSPDUMP, "    ");
    }

    pChar->GetClient()->SendNotification("OnStandingsModified", "charid", payload, false);
}
