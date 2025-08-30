
 /**
  * @name Agent.cpp
  *   agent specific code
  *    original agent code by zhur, this was completely rewritten based on new data.
  *
  * @Author:        Allan
  * @date:      19 June 2018
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

#include <iomanip>

#include "../eve-server.h"
#include "../StaticDataMgr.h"
#include "../station/StationDataMgr.h"
#include "../map/MapData.h"

#include "account/AccountService.h"
#include "agents/Agent.h"
#include "agents/AgentDB.h"
#include "../Client.h"
#include "../fleet/FleetService.h"
#include "../system/SystemManager.h"
#include "../standing/StandingMgr.h"

//NOTE:  SoE has agents.  dont require/modifiy 4 main faction standings (amarr, galente, minmatar, caldari)
Agent::Agent(uint32 id)
: m_important(false),
m_buttonID(1),
m_agentID(id),
m_agentData(AgentData())
{
    _log(AGENT__TRACE, "Agent created for AgentID %u", id);
    if (sConfig.server.StackTrace)
        EvE::traceStack();
}

bool Agent::Load() {
    // agent load isnt called until convo started...missions arent updated until this is called
    AgentDB::LoadAgentData(m_agentID, m_agentData);
    sMissionDataMgr.LoadAgentOffers(m_agentID, m_offers);

    if (m_agentData.research)
        AgentDB::LoadAgentSkills(m_agentID, m_skills);

    if ((m_agentData.typeID == Agents::Type::StoryLine)
    or (m_agentData.typeID == Agents::Type::GenericStoryLine))
        m_agentData.storyline = true;

    if ((m_agentData.factionID == factionSistersOfEVE)
    or  (m_agentData.corporationID == corpSoE))
        sLog.Warning("Agent::Load()", "Agent Created for SoE");

    _log(AGENT__TRACE, "%s(%u) - Data Loaded for level %u %s %s Agent at %s in %s.", \
            m_agentData.name.c_str(), m_agentID, m_agentData.level, sDataMgr.GetAgentTypeName(m_agentData.typeID), \
            sDataMgr.GetCorpDivisionName(m_agentData.divisionID), stDataMgr.GetStationName(m_agentData.locationID).c_str(), \
            sDataMgr.GetSystemName(m_agentData.solarSystemID));

    return true;
}

void Agent::MakeOffer(Character* pChar, MissionOffer& offer) {
    // all missions default to courier
    uint8 misionType = Mission::Type::Courier;
    uint16 connectionSkillType(EvESkill::None);
	uint8 roll(MakeRandomInt(0, 100));

	// random determination here is based on uniform distribution
    switch (m_agentData.divisionID) {
            //  Kill   Courier Trade   Mining
        case Corp::Division::Accounting: {
            //    0%   88%     12%      0%
			if (roll < 13)
				misionType = Mission::Type::Trade;
        } break;
        case Corp::Division::Administration: {
            //   47%   47%      6%      0%
			if (roll < 6) {
				misionType = Mission::Type::Trade;
			} else if (roll < 54) {
				misionType = Mission::Type::Encounter;
			} else {
				misionType = Mission::Type::Courier;
			}
        } break;
        case Corp::Division::Advisory: {
            //   14%   58%     14%     14%
			if (roll < 15) {
				misionType = Mission::Type::Encounter;
			} else if (roll < 72) {
				misionType = Mission::Type::Courier;
			} else if (roll > 85) {
				misionType = Mission::Type::Mining;
			} else {
				misionType = Mission::Type::Trade;
			}
        } break;
        case Corp::Division::Archives: {
            //    0%   92%      8%      0%
			if (roll > 92)
				misionType = Mission::Type::Trade;
        } break;
        case Corp::Division::Astrosurveying: {
            //   13%   25%     13%     50%
            connectionSkillType = EvESkill::MiningConnections;
			if (roll < 13) {
				misionType = Mission::Type::Encounter;
			} else if (roll < 39) {
				misionType = Mission::Type::Courier;
			} else if (roll > 51) {
				misionType = Mission::Type::Mining;
			} else {
				misionType = Mission::Type::Trade;
			}
        } break;
        case Corp::Division::Command: {
            //   88%    6%      6%      0%
            connectionSkillType = EvESkill::DEDConnections;
			if (roll < 89)
				misionType = Mission::Type::Encounter;
			if (roll > 93)
				misionType = Mission::Type::Trade;
        } break;
        case Corp::Division::Distribution: {
            //    5%   85%      5%      5%
            connectionSkillType = EvESkill::DistributionConnections;
			if (roll < 6) {
				misionType = Mission::Type::Encounter;
			} else if (roll < 11) {
				misionType = Mission::Type::Trade;
			} else if (roll < 16) {
				misionType = Mission::Type::Mining;
			} else {
				misionType = Mission::Type::Courier;
			}
        } break;
        case Corp::Division::Financial: {
            //   12%   70%     18%      0%
			if (roll < 13)
				misionType = Mission::Type::Encounter;
			if (roll > 82)
				misionType = Mission::Type::Trade;
        } break;
        case Corp::Division::Intelligence: {
            //   74%   21%      5%      0%
            connectionSkillType = EvESkill::DEDConnections;
			if (roll < 75)
				misionType = Mission::Type::Encounter;
			if (roll > 94)
				misionType = Mission::Type::Trade;
        } break;
        case Corp::Division::InternalSecurity: {
            //  98%    2%      0%      0%
            connectionSkillType = EvESkill::SecurityConnections;
			if (roll < 98)
				misionType = Mission::Type::Encounter;
        } break;
        case Corp::Division::Legal: {
            //  67%   27%      6%      0%
            connectionSkillType = EvESkill::DEDConnections;
			if (roll < 68)
				misionType = Mission::Type::Encounter;
			if (roll > 94)
				misionType = Mission::Type::Trade;
        } break;
        case Corp::Division::Manufacturing: {
            //   0%   48%      4%     48%
            connectionSkillType = EvESkill::MiningConnections;
			if (roll < 6)
				misionType = Mission::Type::Trade;
			if (roll > 52)
				misionType = Mission::Type::Mining;
        } break;
        case Corp::Division::Marketing: {
            //  17%   77%      6%      0%
            connectionSkillType = EvESkill::DistributionConnections;
			if (roll < 18)
				misionType = Mission::Type::Encounter;
			if (roll > 93)
				misionType = Mission::Type::Trade;
        } break;
        case Corp::Division::Mining: {
            //   0%   10%      5%     85%
            connectionSkillType = EvESkill::MiningConnections;
			if (roll < 85)
				misionType = Mission::Type::Mining;
			if (roll > 95)
				misionType = Mission::Type::Trade;
        } break;
        case Corp::Division::Personnel: {
            //  28%   66%      6%      0%
			if (roll < 29)
				misionType = Mission::Type::Encounter;
			if (roll > 93)
				misionType = Mission::Type::Trade;
        } break;
        case Corp::Division::Production: {
            //   0%   52%     13%     35%
            connectionSkillType = EvESkill::DistributionConnections;
			if (roll > 65) {
				misionType = Mission::Type::Mining;
			} else if (roll > 52) {
				misionType = Mission::Type::Trade;
			} else {
				misionType = Mission::Type::Courier;
			}
        } break;
        case Corp::Division::PublicRelations: {
            //  28%   66%      6%      0%
			if (roll < 29)
				misionType = Mission::Type::Encounter;
			if (roll > 93)
				misionType = Mission::Type::Trade;
        } break;
        case Corp::Division::RnD: {
            //   0%   50%     50%      0%
			if (IsEven(roll))
				misionType = Mission::Type::Trade;
        } break;
        case Corp::Division::Security: {
            //  94%    6%      0%      0%
            connectionSkillType = EvESkill::SecurityConnections;
			if (roll < 94)
				misionType = Mission::Type::Encounter;
        } break;
        case Corp::Division::Storage: {
            //   6%   71%      6%     17%
            connectionSkillType = EvESkill::DistributionConnections;
			if (roll < 7) {
				misionType = Mission::Type::Encounter;
			} else if (roll < 78) {
				misionType = Mission::Type::Trade;
			} else if (roll > 82) {
				misionType = Mission::Type::Mining;
			} else {
				misionType = Mission::Type::Courier;
			}
        } break;
        case Corp::Division::Surveillance: {
            //  84%   11%      5%      0%
            connectionSkillType = EvESkill::SecurityConnections;
			if (roll < 47)
				misionType = Mission::Type::Encounter;
			if (roll > 92)
				misionType = Mission::Type::Trade;
        } break;
        case Corp::Division::DistributionNew: {
            //   5%   85%      5%      5%
            connectionSkillType = EvESkill::DistributionConnections;
			if (roll < 7) {
				misionType = Mission::Type::Encounter;
			} else if (roll < 78) {
				misionType = Mission::Type::Trade;
			} else if (roll > 82) {
				misionType = Mission::Type::Mining;
			} else {
				misionType = Mission::Type::Courier;
			}
        } break;
        case Corp::Division::MiningNew: {
            //   0%   10%      5%     85%
            connectionSkillType = EvESkill::MiningConnections;
			if (roll < 11)
				misionType = Mission::Type::Trade;
			if (roll > 92)
				misionType = Mission::Type::Mining;
        } break;
        case Corp::Division::SecurityNew: {
            //  94%    6%      0%      0%
            connectionSkillType = EvESkill::SecurityConnections;
			if (roll < 95)
				misionType = Mission::Type::Encounter;
        } break;
    }
    // if char has 15 completed missions of same level with same corp, make offer for important or storyline mission
    if (pChar->RdyForImportantMission(m_agentData.corporationID, m_agentData.level))
        m_important = true;

    sMissionDataMgr.CreateMissionOffer(misionType, m_agentData.level, m_agentData.raceID, m_important, offer);

    /*  static mission data from db
    offer.name               = cData.name;
    offer.typeID             = cData.typeID;
    offer.bonusTime          = cData.bonusTime;
    offer.important          = cData.important;
    offer.storyline          = cData.storyline;
    offer.missionID          = cData.missionID;
    offer.briefingID         = cData.briefingID;
    offer.rewardItemID       = cData.rewardItemID;
    offer.rewardItemQty      = cData.rewardItemQty;
    offer.courierTypeID      = cData.itemTypeID;
    offer.courierAmount      = cData.itemQty;
    */

    // this is collateral - set in db
    offer.acceptFee          = 0;
    // possibly set in db (not yet)
    offer.remoteOfferable    = false;
    offer.remoteCompletable  = false;


    /* set in mission data mgr
    offer.bookmarks          = new PyList();
    offer.dateIssued         = GetFileTimeNow();
    */

    //offer.dateAccepted       = 0;     //not set here
    //offer.dateCompleted      = 0;     //not set here
    //offer.offerID            = 0;     //created when saving offer in db

    // data set here
    offer.characterID        = pChar->itemID();
    // variable mission data based on agent
    offer.agentID            = m_agentID;
    offer.originID           = m_agentData.stationID;
    offer.originOwnerID      = m_agentData.corporationID;
    offer.originSystemID     = m_agentData.solarSystemID;

    offer.expiryTime         = GetFileTimeNow() + (sConfig.mission.OfferExpiryTime * EvE::Time::Hour);

    // determine destination based on mission type, agent level, agent location, and some other shit.
    sMapData.GetMissionDestination(this, offer);
    /*  data set in mission destination
    offer.destinationID      = 0;
    offer.destinationOwnerID      = 0;
    offer.destinationSystemID = 0;
    offer.destinationTypeID = 0;
    offer.dungeonLocationID      = 0;
    offer.dungeonSolarSystemID   = 0;
    */
    if (offer.destinationID == 0) {
        // make error here and reset...client should never be null
        pChar->GetClient()->SendErrorMsg("Internal Server Error.");
        //return;
    }

    offer.stateID            = Mission::State::Offered;

    // update mission data as needed....
    if ((offer.briefingID == 145156) or (offer.briefingID == 145157)) {
        // the destination and origin need to be reversed on this mission. (Tourists: get from there and bring here)
        offer.originID            = offer.destinationID;
        offer.originOwnerID       = offer.destinationOwnerID;
        offer.originSystemID      = offer.destinationSystemID;

        offer.destinationID       = m_agentData.stationID;
        offer.destinationOwnerID  = m_agentData.corporationID;
        offer.destinationSystemID = m_agentData.solarSystemID;
    }

    // set reward isk based on agent standing, quality, level, system truSec, and char negotiation
    uint32 isk(MakeRandomInt(sConfig.mission.IskRewardLo, sConfig.mission.IskRewardHi));  //11000-16500
    //float charStanding = sStandingMgr.GetRawStanding(m_agentID, pChar);
    float charStanding = sStandingMgr.GetEffectiveStanding(m_agentID, pChar);
    float quality = EvEMath::Agent::EffectiveQuality(m_agentData.quality, pChar->GetSkillLevel(EvESkill::Negotiation), charStanding);

    isk *= (1.0f + (quality / 100.0f));
    isk *= pow(m_agentData.level, 2);

    //  LP reward = (1.6288 - System security) × Base LP
    float lp(1.6288f);
    SystemManager* pSysMgr = sEntityMgr.FindOrBootSystem(offer.destinationSystemID);
    // bonus based on destination system trusec
    if (pSysMgr != nullptr) {
        isk *= (1.0f + pSysMgr->GetSecValue());   // 1.1 to 3.0
        lp -= pSysMgr->GetSystemSecurityRating(); // 0.6288 to 2.5288
    }

    //5% payout bonus per skill level
    isk *= (1.0f + (0.05f * pChar->GetSkillLevel(EvESkill::Negotiation)));
    offer.rewardISK = isk;

    switch (m_agentData.level) {
        case 1:  lp *= Agents::LpMult::Level1;  break;
        case 2:  lp *= Agents::LpMult::Level2;  break;
        case 3:  lp *= Agents::LpMult::Level3;  break;
        case 4:  lp *= Agents::LpMult::Level4;  break;
        case 5:  lp *= Agents::LpMult::Level5;  break;
    }
    lp *= (1.0f + (quality / 100.0f));
    lp *= pow(m_agentData.level, 2);

    //10% payout bonus per <division> connection skills
    lp *= (1.0f + (0.1f * pChar->GetSkillLevel(connectionSkillType)));
    offer.rewardLP = lp;

    // bonus' modified by agent quality and negotiation
    if (offer.bonusTime > 0) {
        offer.bonusISK = offer.rewardISK * (1.0f + (quality / 100.0f));
        offer.bonusISK *= (1.0f + (0.05f * pChar->GetSkillLevel(EvESkill::Negotiation)));
        offer.bonusLP = offer.rewardLP * (1.0f + (quality / 100.0f));
        offer.bonusLP *= (1.0f + (0.05f * pChar->GetSkillLevel(EvESkill::Negotiation)));
    }

    MissionDB::CreateOfferID(offer);

    // keep local copy and add to mission data mgr
    m_offers.emplace(pChar->itemID(), offer);
    sMissionDataMgr.AddMissionOffer(pChar->itemID(), offer);
}

bool Agent::HasMission(uint32 charID) {
    std::map<uint32, MissionOffer>::iterator itr = m_offers.find(charID);
    if (itr != m_offers.end())
        return true;
    return false;
}

bool Agent::HasCurrentMission(uint32 charID) {
    std::map<uint32, MissionOffer>::iterator itr = m_offers.find(charID);
    if (itr != m_offers.end())
        if (itr->second.stateID == Mission::State::Accepted)
            return true;
    return false;
}

bool Agent::HasMission(uint32 charID, MissionOffer& offer) {
    std::map<uint32, MissionOffer>::iterator itr = m_offers.find(charID);
    if (itr != m_offers.end()) {
        offer = itr->second;
        return true;
    }
    return false;
}

void Agent::GetOffer(uint32 charID, MissionOffer& offer)
{
    std::map<uint32, MissionOffer>::iterator itr = m_offers.find(charID);
    if (itr != m_offers.end()) {
        offer = itr->second;
    } else {
        _log(AGENT__WARNING, "Agent::GetOffer() - offer not found for characterID %u", charID);
    }
}

void Agent::UpdateOffer(uint32 charID, MissionOffer& offer) {
    std::map<uint32, MissionOffer>::iterator itr = m_offers.find(charID);
    if (itr != m_offers.end()) {
        itr->second = offer;
        MissionDB::UpdateMissionOffer(itr->second);
        sMissionDataMgr.UpdateMissionData(charID, itr->second);
    } else {
        _log(AGENT__WARNING, "Agent::UpdateOffer() - offer not found for character %u", charID);
    }
}

// need to figure out how/where to save these in db and load with agent for persistence
void Agent::DeclineOffer(Character* pChar) {
    std::map<uint32, MissionOffer>::iterator itr = m_offers.find(pChar->itemID());
    if (itr != m_offers.end()) {
        std::map<uint32, int64>::iterator itr2 = m_declineMap.find(pChar->itemID());
        if (itr2 != m_declineMap.end()) {
            //  hmmm...char is in decline map...check time
            if ((GetFileTimeNow() - itr2->second) < EvE::Time::Hour * 4) {
            // ok, so we have a Decline within last 4h.  hit standings and reset timer
                sStandingMgr.UpdateStandings(pChar, this, Standings::MissionDeclined, itr->second.name, itr->second.important);
            }
            // erase this and set new time (faster than update)
            m_declineMap.erase(pChar->itemID());
        }

        m_declineMap[pChar->itemID()] = GetFileTimeNow();
        m_offers.erase(itr);
    }
}

// adjust this based on system activity (busy systems will have missions in agent's queue)
bool Agent::IsDelayed(Character* pChar) {
    std::map<uint32, int64>::iterator itr = m_delayMap.find(pChar->itemID());
    if (itr != m_delayMap.end()) {
        if (itr->second < GetFileTimeNow()) {
            return true;
        } else {
            // delay time has passed.  reset and return false
            m_delayMap.erase(pChar->itemID());
            return false;
        }
    }
    return false;
}

bool Agent::IsDeclineCooldown(Character* pChar, int64& timeLeft) {
    std::map<uint32, int64>::iterator itr = m_declineMap.find(pChar->itemID());
    if (itr != m_declineMap.end()) {
        //  hmmm...char is in decline map...  set timeLeft and check
        timeLeft = (GetFileTimeNow() - itr->second);
        if (timeLeft < 4 * EvE::Time::Hour) {
            // yep, player has declined a mission from this agent within the last 4h.
            return true;
        } else {
            // ok, so it's been over 4h.  delete this record and return false
            m_declineMap.erase(pChar->itemID());
        }
    }

    return false;
}

void Agent::RemoveOffer(uint32 charID) {
    std::map<uint32, MissionOffer>::iterator itr = m_offers.find(charID);
    if (itr != m_offers.end()) {
        m_offers.erase(itr);
    } else {
        _log(AGENT__WARNING, "Agent::RemoveOffer() - offer not found for character %u", charID);
    }
}

void Agent::DeleteOffer(uint32 charID) {
    std::map<uint32, MissionOffer>::iterator itr = m_offers.find(charID);
    if (itr != m_offers.end()) {
        itr->second.stateID = Mission::State::Rejected;
        MissionDB::UpdateMissionOffer(itr->second);
        sMissionDataMgr.RemoveMissionOffer(charID, itr->second);
        m_offers.erase(itr);
    } else {
        _log(AGENT__WARNING, "Agent::DeleteOffer() - offer not found for character %u", charID);
    }
}


PyDict* Agent::GetLocationWrap() {
    PyDict *res = new PyDict();
        res->SetItemString("typeID", new PyInt(m_agentData.locationTypeID) );
        res->SetItemString("locationID", new PyInt(m_agentData.locationID) );
        res->SetItemString("solarsystemID", new PyInt(m_agentData.solarSystemID) );
    return res;

    /* other location data types to put in dict for agents in space
     * locationType
     * coords
     * referringAgentID
     * shipTypeID
     *
     */

    /*
     * def LocationWrapper(location, locationType = None):
     *    if locationType is None and 'locationType' in location:
     *        locationType = location['locationType']
     *    pseudoSecurityRating = cfg.solarsystems.Get(location['solarsystemID']).pseudoSecurity
     *    if pseudoSecurityRating <= 0:
     *        securityKey = '0.0'
     *    else:
     *        securityKey = str(round(pseudoSecurityRating, 1))
     *    secColor = SECURITY_COLORS[securityKey]
     *    secColorAsHtml = '#%02X%02X%02X' % (secColor[0], secColor[1], secColor[2])
     *    secWarning = '<font color=#E3170D>'
     *    secClass = util.SecurityClassFromLevel(pseudoSecurityRating)
     *    standingSvc = sm.GetService('standing')
     *    if secClass <= const.securityClassLowSec:
     *        secWarning += localization.GetByLabel('UI/Agents/LowSecWarning')
     *    elif standingSvc.GetMySecurityRating() <= -5:
     *        secWarning += localization.GetByLabel('UI/Agents/HighSecWarning')
     *    secWarning += '</font>'
     *    if 'coords' in location:
     *        x, y, z = location['coords']
     *        refAgentString = str(location['agentID'])
     *        if 'referringAgentID' in location:
     *            refAgentString += ',' + str(location['referringAgentID'])
     *        infoLinkData = ['showinfo',
     *         location['typeID'],
     *         location['locationID'],
     *         x,
     *         y,
     *         z,
     *         refAgentString,
     *         0,
     *         locationType]
     *    else:
     *        infoLinkData = ['showinfo', location['typeID'], location['locationID']]
     *    spacePigShipType = location.get('shipTypeID', None)
     *    if spacePigShipType is not None:
     *        locationName = localization.GetByLabel('UI/Agents/Items/ItemLocation', typeID=spacePigShipType, locationID=location['locationID'])
     *    else:
     *        locationName = cfg.evelocations.Get(location['locationID']).locationName
     *    return localization.GetByLabel('UI/Agents/LocationWrapper', startFontTag='<font color=%s>' % secColorAsHtml, endFontTag='</font>', securityRating=pseudoSecurityRating, locationName=locationName, linkdata=infoLinkData, securityWarning=secWarning)
     */
}

PyObject* Agent::GetInfoServiceDetails(Client* pClient)
{
    // can this be static data created when agent is loaded? no, this is called by character and set based on char standings
    PyDict* res = new PyDict();
        res->SetItemString("stationID", new PyInt(m_agentData.stationID) );
        res->SetItemString("level", new PyInt(m_agentData.level) );
        res->SetItemString("quality", new PyInt(m_agentData.quality) );

    // 'services' is a tuple of dicts containing data for [research], [locate], and [mission] services this agent offers

    /*  for research agents....
     *  skillTypeID, skillLevel in data.skills:
     * researchData = data.researchData
     * researchData['rpMultiplier']
     * researchData['skillTypeID']
     * researchData['points']  -- current points
     * researchData['pointsPerDay']
     * skillTypeID, blueprintTypeID in data.researchSummary:  -- for predictablePatentNames
     */

    /**  @todo  finish this..... */
    PyDict* research = new PyDict();
    if (m_agentData.research) {
        PyTuple* skill1 = new PyTuple(2);
            skill1->SetItem(0, new PyInt(11452)); // Mechanical Engineering
            skill1->SetItem(1, new PyInt(4));
        PyTuple* skill2 = new PyTuple(2);
            skill2->SetItem(0, new PyInt(11453));  //Electronic Engineering
            skill2->SetItem(1, new PyInt(3));
        PyList* skillList = new PyList();
            skillList->AddItem(skill1);
            skillList->AddItem(skill2);
        PyDict* researchData = new PyDict();
            researchData->SetItemString("rpMultiplier", new PyInt(2));
            researchData->SetItemString("skillTypeID", new PyInt(11452));   // this is player research field with this agent.  not sure how to make "none" yet
            researchData->SetItemString("points", new PyInt(150));
            researchData->SetItemString("pointsPerDay", new PyInt(30));
        PyTuple* patent1 = new PyTuple(2);
            patent1->SetItem(0, new PyInt(11452));
        PyList* patentlist1 = new PyList();
            patentlist1->AddItem(new PyInt(692));
            patent1->SetItem(1, patentlist1);
        PyTuple* patent2 = new PyTuple(2);
            patent2->SetItem(0, new PyInt(11453));
        PyList* patentlist2 = new PyList();
            patentlist2->AddItem(new PyInt(1196));
            patent2->SetItem(1, patentlist2);
        PyList* patentList = new PyList();
            patentList->AddItem(patent1);
            patentList->AddItem(patent2);

        research->SetItemString("agentServiceType", new PyString("research"));
        research->SetItemString("skills", skillList);
        research->SetItemString("researchSummary", patentList);
        research->SetItemString("researchData", researchData);
    } else {
        research->SetItemString("agentServiceType", PyStatic.NewNone());
    }

    /* for location agents....
     *
 level  Standings  Time to Run                             Cooldown    Cost                Range
    1     Any      Instant/1 minute                        5 minutes   1k/5k               Constellation
    2     1.0      Instant/1 minute/8 minutes              5 minutes   5k/10k/25k          Region
    3     3.0      Instant/30 seconds/4 minutes/8 minutes  15 minutes  10k/25k/50k/100k    Unlimited
    4     5.0      Instant/20 seconds/2 minutes/4 minutes  30 minutes  25k/50k/100k/250k   Unlimited

     * data.frequency
     *delayRange, delay, cost in data.delays:       -- (tuple) range (system, const, region, other region), responseTime (in sec), cost
     *   data.callbackID  -- bool for agent locator services being used (locator unavailable)
     *      OR
     *   data.lastUsed  -- blue time?

     Once done the locator agent will send you a Notification with the location of the target when you started the search which will include the system, constellation and region as well as the station the player might be docked at. If the target is in space, no station will be listed. E.g. "The sleazebag is currently in the Bukah system, Nohshayess constellation of the Khanid region."
     If the target is logged off, the locator agent will tell you the last known position. If the target is in Anoikis (wormhole space), the locator agent will tell you "I'm sorry, but I just can't help you with that one. I'm pretty sure O'b Haru Sen is well out of my zone of influence." - even if the agent can locate anyone in known space.

     (235843, `{[character]charID.gender -> "He", "She"} is in the {systemName} system of the {constellationName} constellation.
     (235844, `{[character]charID.gender -> "He", "She"} is in the {systemName} system, {constellationName} constellation of the {regionName} region.
     (235845, `{[character]charID.gender -> "He", "She"} is in your solar system.
     (235846, `{[character]charID.gender -> "He", "She"} is at your station.
     (235847, `{[character]charID.gender -> "He", "She"} is at {stationName} station in your solar system.
     (235848, `{[character]charID.gender -> "He", "She"} is at {stationName} station in the {systemName} system.
     (235849, `{[character]charID.gender -> "He", "She"} is at{stationName} station in the {systemName} system of the {constellationName} constellation.
     (235850, `{[character]charID.gender -> "He", "She"} is at {stationName} station in the {systemName} system, {constellationName} constellation of {regionName} region.
     (235851, `{[character]charID.gender -> "He", "She"} is in the {systemName} system.
     */
    PyDict* locate = new PyDict();
    if (m_agentData.locator) {
        PyTuple* sameSystem = new PyTuple(3);
            sameSystem->SetItem(0, new PyInt(0));
            sameSystem->SetItem(1, new PyInt(10));
            sameSystem->SetItem(2, new PyInt(20000));
        PyTuple* sameConst = new PyTuple(3);
            sameConst->SetItem(0, PyStatic.NewOne());
            sameConst->SetItem(1, new PyInt(30));
            sameConst->SetItem(2, new PyInt(200000));
        PyTuple* sameRegion = new PyTuple(3);
            sameRegion->SetItem(0, new PyInt(2));
            sameRegion->SetItem(1, new PyInt(60));
            sameRegion->SetItem(2, new PyInt(2000000));
        PyTuple* otherRegion = new PyTuple(3);
            otherRegion->SetItem(0, new PyInt(3));
            otherRegion->SetItem(1, new PyInt(120));
            otherRegion->SetItem(2, new PyInt(20000000));
        PyTuple* delays = new PyTuple(4);
            delays->SetItem(0, sameSystem);
            delays->SetItem(1, sameConst);
            delays->SetItem(2, sameRegion);
            delays->SetItem(3, otherRegion);

        locate->SetItemString("agentServiceType", new PyString("locate"));
        locate->SetItemString("frequency", new PyInt(1200));  // if this is PyNone (or 0?) agent location isnt avalible (client parsed msg)
        locate->SetItemString("delays", delays);
        locate->SetItemString("callbackID", new PyInt(2));
        locate->SetItemString("lastUsed", new PyInt(0));
    } else {
        locate->SetItemString("agentServiceType", PyStatic.NewNone());
    }

    // for mission agents....
    PyDict* mission = new PyDict();
    if (m_agentData.typeID > Agents::Type::None) {
        mission->SetItemString("agentServiceType", new PyString("mission"));
        mission->SetItemString("available", new PyBool(CanUseAgent(pClient)));
    }

    PyTuple* services = new PyTuple(3);
        services->SetItem(0, new PyObject("util.KeyVal", research));
        services->SetItem(1, new PyObject("util.KeyVal", locate));
        services->SetItem(2, new PyObject("util.KeyVal", mission));
    res->SetItemString("services", services);

    // standings info for this agent.
    if ((m_agentData.typeID == Agents::Type::StoryLine)
    or (m_agentData.typeID == Agents::Type::GenericStoryLine)) {
        PyTuple* details = new PyTuple(2);
            details->SetItem(0, new PyString("UI/Agents/Incompatible/NeedsReferral"));  //235467: This agent can only be used through a direct referral.
            PyDict* dict = new PyDict();
            dict->SetItemString("player", new PyInt(pClient->GetCharacterID()));
            details->SetItem(1, dict);
        res->SetItemString("incompatible", details);
    } else if (0) {
    /* can also use locale labelIDs for this using a tuple to define minStandings, minEffective, corpMinStandings, mainEffective, effectiveMinStandings in other msgIDs
     * this will take char, corp, faction, agent, and some other shit into account to determine msg and data sent using the tuple system
     * UI/Agents/Incompatible/CantUseMinStandings        235462:   Your personal standings must be -1.9 or higher toward this agent, its faction, or its corporation in order to use this agent's services.", None, None)
     * UI/Agents/Incompatible/ResearchAgent              235463:   Your personal standings must be -1.9 or higher toward this agent, its faction, and its corporation in order to use this agent's services. Additionally, you need a minimum effective standing to this agent's corp of at least {[numeric]corpMinStandings, decimalPlaces=1} , as well as personal standing of at least {[numeric]effectiveMinStandings, decimalPlaces=1} to this agent's faction, corp, or to the agent in order to use this agent's services.", None, {u'{[numeric]effectiveMinStandings, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'effectiveMinStandings'}, u'{[numeric]corpMinStandings, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'corpMinStandings'}})
     * UI/Agents/Incompatible/ResearchStandings          235464:   Your personal standings must be -1.9 or higher toward this agent, its faction, and its corporation in order to use this agent's services. Additionally, you need a minimum effective standing of at least {[numeric]minStandings, decimalPlaces=1} to this agent's faction, corp, or to the agent in order to use this agent's services.", None, {u'{[numeric]minStandings, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'minStandings'}})
     * UI/Agents/Incompatible/CantUseRequiredStandings   235465:   Your effective personal standings must be {[numeric]minStandings, decimalPlaces=1} or higher toward this agent, its faction, or its corporation in order to use this agent's services", None, {u'{[numeric]minStandings, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'minStandings'}})
     * UI/Agents/Incompatible/CantUseResearchStandings   235466:   Your effective personal standings must be {[numeric]minEffective, decimalPlaces=1} or higher toward this agent's corporation in order to use this agent, as well as an effective personal standing of {[numeric]mainEffective, decimalPlaces=1} or higher toward this agent, its faction, or its corporation in order to use this agent's services.", None, {u'{[numeric]mainEffective, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'mainEffective'}, u'{[numeric]minEffective, decimalPlaces=1}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 1}, 'variableName': 'minEffective'}})
     * UI/Agents/Incompatible/NeedsReferral              235467:   This agent can only be used through a direct referral.', None, None)
     */
        /*
        PyDict* dict = new PyDict();
            dict->SetItemString("minStandings", new PyFloat(GetMinReqStanding(m_agentData.level)));
            dict->SetItemString("mainEffective", new PyFloat(GetMinReqStanding(m_agentData.level +10)));
        PyTuple* tuple = new PyTuple(2);
            tuple->SetItem(0, new PyString("UI/Agents/Incompatible/CantUseRequiredStandings"));
            tuple->SetItem(1, dict);
        res->SetItemString("incompatible", tuple);
        */
    } else {
        std::stringstream msg;
        msg << "Your effective standing must be ";
        msg << std::fixed << std::setprecision(2) << EvEMath::Agent::RequiredStanding(m_agentData.level, m_agentData.quality);
        msg << " or higher in order to use their services.";
        res->SetItemString("incompatible", new PyString(msg.str()));
    }


    if (is_log_enabled(AGENT__RSP_DUMP)) {
        _log(AGENT__RSP_DUMP, "Agent::GetInfoServiceDetails() Dump:" );
        res->Dump(AGENT__RSP_DUMP, "    ");
    }

    return new PyObject("util.KeyVal", res);
}

void Agent::SendMissionUpdate(Client* pClient, std::string action)
{
    if (pClient == nullptr)
        return;

    //specific to the calling action
    //OnInteractWith(agentID)       (force agent convo)

    //OnAgentMissionChange(action, agentID, tutorialID)
    /*
    agentMissionOffered = 'offered'
    agentMissionOfferAccepted = 'offer_accepted'
    agentMissionOfferDeclined = 'offer_declined'
    agentMissionOfferExpired = 'offer_expired'
    agentMissionOfferRemoved = 'offer_removed'
    agentMissionAccepted = 'accepted'
    agentMissionDeclined = 'declined'
    agentMissionCompleted = 'completed'
    agentTalkToMissionCompleted = 'talk_to_completed'
    agentMissionQuit = 'quit'
    agentMissionResearchUpdatePPD = 'research_update_ppd'
    agentMissionResearchStarted = 'research_started'
    agentMissionProlonged = 'prolong'
    agentMissionReset = 'reset'
    agentMissionModified = 'modified'       - force agent convo
    agentMissionFailed = 'failed'
    */

    PyTuple* payload = new PyTuple(3);
        payload->SetItem(0, new PyString(action));
        payload->SetItem(1, new PyInt(m_agentID));
        payload->SetItem(2, PyStatic.NewNone());    //tutorialID NOTE if we ever get tutorials working, this will need to be fixed.
    pClient->SendNotification("OnAgentMissionChange", "charid", payload, false);    // this is unsequenced
}

bool Agent::CanUseAgent(Client* pClient) {
    if (m_agentData.typeID == Agents::Type::Aura)
        return true;
    if (m_agentData.level == 1) {
        switch (m_agentData.typeID) {
            case Agents::Type::Basic:
            case Agents::Type::Career: {
                return true;
            } break;
            // remaining types need checks
        }
    }
    //copied from client code (modified for our variables)

    Character* pChar = pClient->GetChar().get();
    uint32 charID = pChar->itemID();
    uint8 sConn = pChar->GetSkillLevel(EvESkill::Connections);
    uint8 sDiplo = pChar->GetSkillLevel(EvESkill::Diplomacy);
    uint8 sCrim = pChar->GetSkillLevel(EvESkill::CriminalConnections);

    float facChr = sStandingMgr.GetRawStanding(m_agentData.factionID, charID);
    float corpChr = sStandingMgr.GetRawStanding(m_agentData.corporationID, charID);
    float charChr = sStandingMgr.GetRawStanding(m_agentID, charID);
    float facBonus = EvEMath::Agent::GetStandingBonus(facChr, m_agentData.factionID, sConn, sDiplo, sCrim);
    float corpBonus = EvEMath::Agent::GetStandingBonus(corpChr, m_agentData.factionID, sConn, sDiplo, sCrim);
    float charBonus = EvEMath::Agent::GetStandingBonus(charChr, m_agentData.factionID, sConn, sDiplo, sCrim);

    if (facBonus > 0.0f)
        facChr = EvEMath::Agent::EffectiveStanding(facChr, facBonus);
    if (corpBonus > 0.0f)
        corpChr = EvEMath::Agent::EffectiveStanding(corpChr, corpBonus);
    if (charBonus > 0.0f)
        charChr = EvEMath::Agent::EffectiveStanding(charChr, charBonus);

    bool rsp(true);
    float required(EvEMath::Agent::RequiredStanding(m_agentData.level, m_agentData.quality));

    // for agents > l1, ANY standing < -2, denies availability
    if (m_agentData.level > 1)
        if (EvE::min(facChr, corpChr, charChr) < -2.0f)
            rsp = false;

    if (EvE::max(facChr, corpChr, charChr) < required)
        rsp = false;

    _log(AGENT__DEBUG, "%s(%u) CanUseAgent() - charSkills(con:%u,dip:%u,cri:%u), required %f",\
                m_agentData.name.c_str(), m_agentID, sConn, sDiplo, sCrim, required);
    _log(AGENT__DEBUG, "%s(%u) CanUseAgent() result: %s - standings(fac:%.2f,crp:%.2f,chr:%.2f), bonus(%.2f, %.2f, %.2f)", \
                   m_agentData.name.c_str(), m_agentID, rsp?"true":"false", facChr, corpChr, charChr, facBonus, corpBonus, charBonus);

    return rsp;
}

void Agent::DistributeRewards(Character* pChar, MissionOffer& offer) {
    //TODO:  finish this for fleet and LP
    if (offer.rewardISK)
        AccountService::TransferFunds(m_agentID, pChar->itemID(), offer.rewardISK, "Mission Reward", Journal::EntryType::AgentMissionReward, m_agentID);
    if (offer.bonusISK)
        if (offer.bonusTime > ((offer.dateAccepted - GetFileTimeNow()) / EvE::Time::Minute))
            AccountService::TransferFunds(m_agentID, pChar->itemID(), offer.bonusISK, "Mission Bonus Reward", Journal::EntryType::AgentMissionTimeBonusReward, m_agentID);
        // test for bonus items?
        /** @todo  add lp, etc, etc  */
}

/*******************************************************
 *  ALL AGENT RESPONSES HERE.
 *  ALL USE AVAILABILITY, STANDINGS, STATUS, CURRENT MISSION
 *  agents use standings in order of faction, corp, personal
 */
uint32 Agent::GetResponse(Character* pChar, uint8 rspID) {
    // not sure if i wanna use this...no subscription to purchase here...but no trial accts, either.  :/
    if (pChar->GetClient()->GetAccountType() == UserType::Trial) {
        if (m_agentData.level > 2)
            return 236708; // `Trial account users cannot access agents of level 3 or higher. Either use your map settings to find a level 1 or level 2 agent, or consider purchasing an EVE subscription if you wish to access higher-level content.
    }

    // get the fully-modified char standing here, from faction, corp, agent -> char
    float agentStanding = sStandingMgr.GetEffectiveStanding(m_agentID, pChar);
    float corpStanding = sStandingMgr.GetEffectiveStanding(m_agentData.corporationID, pChar);
    float factionStanding = sStandingMgr.GetEffectiveStanding(m_agentData.factionID, pChar);

    /*  Standings::
     *      //these are agent/corp/faction -> char
     *      Bad         = -1.0f,
     *      Lo          = 1.5f,
     *      LoMid       = 3.5f,
     *      MidHi       = 5.5f,
     *      Hi          = 7.5f
     */
    switch (rspID) {
        case Agents::Response::NoStanding: {
            if (agentStanding > Standings::Hi) {
                return 236775; // `Hello, {[character]player.name}.  I was given the assignment of handing out a valuable item to loyal supporters of {[npcOrganization]agentFactionID.name}.  If you know of someone who has proven him or herself time and time again for {[npcOrganization]agentFactionID.nameWithArticle} then point that person to me and perhaps we can come to an 'arrangement' ...<br><br>You on the other hand do not meet my requirements or have already received my offer.
            } else if (agentStanding > Standings::LoMid) {
                return 236682; // `I’m sorry pilot, but you do not have the required standings to receive any missions from me. You will need to raise your standings by doing missions for lower-ranked agents. Use the Agent Finder to locate agents that are available to you.
            } else if (agentStanding > Standings::Lo) {
                if (corpStanding > Standings::Lo) {
                    return 236787; // `Have you even bothered to check your standings.
                } else {
                    return 236785; // `What planet were you born on?  Check your standings next time or I'll drill a hole in you.
                }
            } else if (agentStanding > Standings::Bad) {
                if (corpStanding > Standings::Lo) {
                    return 236787; // `Have you even bothered to check your standings.
                } else {
                    return 236785; // `What planet were you born on?  Check your standings next time or I'll drill a hole in you.
                }
            } else {
                if (corpStanding > Standings::Lo) {
                    return 236786;  //StandingsTooLow1:  Begone, scum.
                } else {
                    return 236788;  //StandingsTooLow2:  You've got a lot of nerve, showing your face around here.
                }
            }

            //236691; // `I’m sorry pilot, but you do not have the required standings to receive any missions from me. You will need to raise your standings by doing missions for lower-ranked agents. Use the Agent Finder to locate agents that are available to you.
            //236853; // `I’m sorry pilot, but you do not have the required standings to receive any missions from me. You will need to raise your standings by doing missions for lower-ranked agents. Use the Agent Finder to locate agents that are available to you.
            //236853; // `I’m sorry pilot, but you do not have the required standings to receive any missions from me. You will need to raise your standings by doing missions for lower-ranked agents. Use the Agent Finder to locate agents that are available to you.
        }

        case Agents::Response::Greeting: {
            if (HasCurrentMission(pChar->itemID())) {
                // initial greetings - unfinished mission
                if (agentStanding > Standings::Hi) {
                    if (IsEven(MakeRandomInt(0, 20))) {
                        return 236733;  //High1:  {[character]player.name}! You're back! If there is anything you need, just ask. But you are going to finish your current mission, right?
                    } else {
                        return 236734;  //High2:  Great to see you, {[character]player.name}, as always. I have to assume there's a good reason that you've come back before finishing your current mission??
                    }
                } else if (agentStanding > Standings::LoMid) {
                    switch (MakeRandomInt(1, 3)) {
                        case 1:  return 236730;  //Medium1:  Nice to see you, {[character]player.name}, but you know you still have an unfinished mission from me, right?', None, {u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}})
                        case 2:  return 236731;  //Medium2:  Always a pleasure, of course. But the last mission I gave you isn't going to complete itself.
                        case 3:  return 236732;  //Medium3:  Hey, {[character]player.name}. I really like your style... but I do need you to finish up your current mission for me.', None, {u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}})
                    }
                } else if (agentStanding > Standings::Bad) {
                    if (IsEven(MakeRandomInt(0, 20))) {
                        return 236735;  //Low1:  You do know you haven't finished your current mission for me, right?
                    } else {
                        return 236736;  //Low3:  {[character]player.name}, you need to finish your current mission for me.', None, {u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}})
                    }
                } else {
                    return 236737;  //Low2:  You need to finish your current mission. Don't talk to me until then.
                }
            } else if (m_agentData.storyline) {
                //***  initial greetings - no referral
                if (agentStanding > Standings::Hi) {
                    return 236774;  //{agentFactionName} does not provide work to the general public.
                } else if (agentStanding > Standings::MidHi) {
                    return 236713;  //Sorry, I have no jobs for the general public.
                } else if (agentStanding > Standings::LoMid) {
                    //return 236770;  //Sorry, I have no jobs for the general public.
                    return 236696;  //Sorry, but I only work with people I trust.  Come back once you have a referral.
                } else if (agentStanding > Standings::Lo) {
                    return 236689;  //I'm sorry, but I have no business with you at the moment.  Come back once you have a referral.
                } else if (agentStanding < Standings::Bad) {
                    return 236692;  //Get out of my sight, mortal.
                } else {
                    return 236686;  //We're closed to the public.
                }
                // (236681;  //I will only give out missions to those who have joined the war!
            } else {
                /* //***  initial greetings - remote denied
                 * pChar->locationID();
                return 236687, `I'm sorry, what did you say? There's a bit of interference on the channel at the moment. Perhaps you could call back using a station line?
                return 236707, `I'm sorry, what did you say? There's a bit of interference on the channel at the moment. Perhaps you could call back using a station line?
                return 236719, `My offices are located at {[location]agentStationID.name}.  If you want to talk, please meet me in person.
                return 236717, `I am currently off in {[location]agentSolarSystemID.name}.  If you want to talk, please meet me in person.
                */

                if (agentStanding > Standings::Hi) {
                    switch (MakeRandomInt(1, 3)) {
                        case 1: return 236723;  //High4:  I am exhilarated at seeing you, {[character]player.name}. I hope I can be of assistance.
                        case 2: return 236724;  //High3:  Nice to see you {[character]player.name}!
                        case 3: return 236726;  //High2:  [character]player.name}! You're back! Please have a seat and make yourself feel comfortable. If there is anything you need, just ask.
                    }
                } else if (agentStanding > Standings::MidHi) {
                    if (IsEven(MakeRandomInt(0, 20))) {
                        return 236729;  //Greetings, {[character]player.name}.
                    } else {
                        return 135808;  //How can I assist you? ')
                    }
                } else if (agentStanding > Standings::LoMid) {
                    if (IsEven(MakeRandomInt(0, 20))) {
                        return 236722;  //Medium2: Nice to see you, {[character]player.name}.
                    } else {
                        return 236778;  //Why hello there.  What do you want?
                    }
                } else if (agentStanding > Standings::Lo) {
                    //***  initial greetings - neutral
                    if (IsEven(MakeRandomInt(0, 20))) {
                        return 236725;  //Low1:  Why hello there. What do you want?
                    } else {
                        return 236720;  //Medium1:  So you think you're tough? If that's the case, I might have some use for you.
                    }
                } else if (agentStanding > Standings::Bad) {
                    if (IsEven(MakeRandomInt(0, 20))) {
                        return 236727;  //Low3:  I hope you're not here to ask me for a favor. I hate beggars.
                    } else {
                        return 236728;  //Low2:  What do you want? Spit it out, stooge.
                    }
                } else {
                    //***  initial greetings - negative
                    if (IsEven(MakeRandomInt(0, 20))) {
                        return 236788;  //You've got a lot of nerve, showing your face around here.
                    } else {
                        return 236786;  //Begone, scum.
                    }
                }
            }
        } break;

        case Agents::Response::NoWork: {
            // initial greetings - no work
            if (agentStanding > Standings::MidHi) {
                if (IsEven(MakeRandomInt(0, 20))) {
                    return 236798;  //Sorry, kid, nothing at the moment. Could you come back later?
                } else {
                    return 236799;  //I'm sorry, but what I've got is promised to another pilot already. Could you come back later?
                }
            } else if (agentStanding > Standings::LoMid) {
                if (IsEven(MakeRandomInt(0, 20))) {
                    return 236703;  //Sorry, I have no jobs available for you.
                } else {
                    return 236779;  //If you're here to find an excuse to show off those big, shiny weapons of yours, then I suggest you go find some nincompoop working for the {[npcOrganization]agentFactionID.name} military and leave me alone.  If not, then I might lend you an ear, depending on your worth to me.
                }
            } else if (agentStanding > Standings::Lo) {
                if (IsEven(MakeRandomInt(0, 20))) {
                    return 236780;  //You just interrupted me from my studies.  This better be good ...
                } else {
                    return 129422;  //Who are you?  I've got important business to attend to, so unless you're somebody important get lost.')
                }
            } else if (agentStanding > Standings::Bad) {
                if (IsEven(MakeRandomInt(0, 20))) {
                    return 129421;  //Who are you?  I've got important business to attend to, so unless you're somebody important get lost.')
                } else {
                    return 236699;  //I'm busy, go away.
                }
            } else {
                return 236697;  //I'm busy.  Go away.
            }
        } break;

        case Agents::Response::InProgress: {
            if (agentStanding > Standings::Hi) {
                if (IsEven(MakeRandomInt(0, 20))) {
                    return 236733;  //High1:  {[character]player.name}! You're back! If there is anything you need, just ask. But you are going to finish your current mission, right?
                } else {
                    return 236734;  //High2:  Great to see you, {[character]player.name}, as always. I have to assume there's a good reason that you've come back before finishing your current mission??
                }
            } else if (agentStanding > Standings::LoMid) {
                switch (MakeRandomInt(1, 3)) {
                    case 1:  return 236730;  //Medium1:  Nice to see you, {[character]player.name}, but you know you still have an unfinished mission from me, right?', None, {u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}})
                    case 2:  return 236731;  //Medium2:  Always a pleasure, of course. But the last mission I gave you isn't going to complete itself.
                    case 3:  return 236732;  //Medium3:  Hey, {[character]player.name}. I really like your style... but I do need you to finish up your current mission for me.', None, {u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}})
                }
            } else if (agentStanding > Standings::Bad) {
                if (IsEven(MakeRandomInt(0, 20))) {
                    return 236735;  //Low1:  You do know you haven't finished your current mission for me, right?
                } else {
                    return 236736;  //Low3:  {[character]player.name}, you need to finish your current mission for me.', None, {u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}})
                }
            } else {
                return 236737;  //Low2:  You need to finish your current mission. Don't talk to me until then.
            }
        } break;
        case Agents::Response::Accept: {
            /*  not sure if we wanna run thru the tests for these or put it in mission specific code
             * //(130431, 'Thank you.  Be careful out there, those logs must <b>not</b> fall into the wrong hands.')
             * //(136324, 'Excellent. I've just uploaded the coordinates of the pickup station to your NeoCom. Do be quick, okay? ')
             */
            if (agentStanding > Standings::Hi) {
                switch (MakeRandomInt(1, 3)) {
                    case 1:  return 136295;  //Excellent. You're doing us a great service, {[character]player.name}. ')
                    case 2:  return 135877;  //I knew we could count on you, {[character]player.name}. Please hurry. ')
                    case 3:  return 236721;  //You have a long and prosperous future within {[npcOrganization]agentFactionID.name}, {[character]player.name}.
                }
            } else if (agentStanding > Standings::MidHi) {
                switch (MakeRandomInt(1, 3)) {
                    case 1:  return 236747;  //MedHigh3:Great.  I know I can trust you with this, mate.
                    case 2:  return 236749;  //Wonderful.  I expect a quick result with you on the job.
                    case 3:  return 139327;  //Your assistance is appreciated, {[character]player.name}. ')
                }
            } else if (agentStanding > Standings::LoMid) {
                switch (MakeRandomInt(1, 3)) {
                    case 1:  return 135811;  //Move quickly, {[character]player.name}. ')
                    case 2:  return 236746;  //MedHigh2: I knew I could count on you.
                    case 3:  return 139398;  //I knew you were the right pilot for the job.')
                }
            } else if (agentStanding > Standings::Lo) {
                switch (MakeRandomInt(1, 4)) {
                    case 1:   return 236744;  //LowMed4: Now be careful out there, you hear me?
                    case 2:   return 236745;  //MedHigh1:Stay alive, friend.
                    case 3:   return 236741;  //LowMed1:Thank you, and good luck.
                    case 4:   return 236742;  //LowMed2: Thanks, I really appreciate it.
                }
            } else if (agentStanding > Standings::Bad) {
                switch (MakeRandomInt(1, 3)) {
                    case 1:   return 236745; // Stay alive, friend.
                    case 2:   return 236740;  //Low3:Good good, now get out there and give me some results.
                    case 3:   return 236743;  //LowMed3:Have fun!
                }
            } else {
                switch (MakeRandomInt(1, 3)) {
                    case 1:   return 236738;  //Low1:Very well then, get going.
                    case 2:   return 135138;  //Don't waste any time. Our people are waiting.
                    case 3:   return 236739;  //Low2:Ok, hurry up will you.
                }
             }
        } break;

        case Agents::Response::Complete: {
            // set a mission delay here based on standings and system
            float delayMins(3.0f - (agentStanding / 5.0f));  // delay minutes; 1 - 5
            if (sDataMgr.IsFringeSystem(m_agentData.solarSystemID)) {
                // light traffic...
                delayMins += 3.5f;
            } else if (sDataMgr.IsCorridorSystem(m_agentData.solarSystemID)) {
                // med traffic...
                delayMins += 1.5f;
            } else if (sDataMgr.IsHubSystem(m_agentData.solarSystemID)) {
                // higher traffic...
                delayMins += 0.5f;
            }
            if (sDataMgr.IsConSystem(m_agentData.solarSystemID)) {
                // a bit more traffic...
                delayMins -= 0.5f;
            } else if (sDataMgr.IsRegionSystem(m_agentData.solarSystemID)) {
                // possibly higher traffic
                delayMins -= 0.9f;
            }
            SystemManager* pSysMgr = sEntityMgr.FindOrBootSystem(m_agentData.solarSystemID);
            // modify delay based on system trusec
            if (pSysMgr != nullptr)
                delayMins -= pSysMgr->GetSecValue();    // 0.9 to 3.0

            // now, modify delay time by agent quality  (lower quality = longer delay)
            float charStanding = sStandingMgr.GetEffectiveStanding(m_agentID, pChar);
            float quality = EvEMath::Agent::EffectiveQuality(m_agentData.quality, pChar->GetSkillLevel(EvESkill::Negotiation), charStanding);
            delayMins *= (1.0f - (quality / 100.0f));   // 0.72 to 3.6

            // sanity check
            if (delayMins < 0.0f)
                delayMins = 0.1f;

            m_delayMap[pChar->itemID()] = (GetFileTimeNow() + (delayMins * EvE::Time::Minute));
            sLog.Cyan("Agent::GetResponse():Complete", "Mission delay set for %.2f minutes", delayMins);

            if (agentStanding > Standings::Hi) {
                switch (MakeRandomInt(1, 3)) {
                    case 1:   return 236721;  //High1:  You have a long and prosperous future within {[npcOrganization]agentFactionID.name}, {[character]player.name}.
                    case 2:   return 236767;  //High2:  If only I had more excellent pilots like you, {[character]player.name} ...
                    case 3:   return 236768;  //High3:  Your talents as a pilot never cease to amaze me.  Keep up the good work!
                    case 4:   return 236769;  //High4:  I am in your debt {[character]player.name}.  If you ever need anything, just look me up.
                }
            } else if (agentStanding > Standings::MidHi) {
                switch (MakeRandomInt(1, 4)) {
                    case 1:   return 236766;  //High1:  I thank you from the bottom of my heart.
                    case 2:   return 135818;  //You're a hell of a pilot, {[character]player.name}. Come back in a bit and I'll let you know if there's more work for you.')
                    case 3:   return 236750;  //Fabulous.  I couldn't have asked for a better man for the job.
                    case 4:   return 236764;  //MedHigh5:  I look forward to your next visit.
                }
            } else if (agentStanding > Standings::LoMid) {
                switch (MakeRandomInt(1, 6)) {
                    case 1:   return 236760;  //MedHigh1:  Thank you very much, I really appreciate it.
                    case 2:   return 236761;  //MedHigh2:  Well done!  Take this reward and my gratitude as well.
                    case 3:   return 236762;  //MedHigh3:  It's a pleasure doing business with you.
                    case 4:   return 236763;  //MedHigh4:  Excellent work!  Care for another assignment?
                    case 5:   return 236765;  //MedHigh6:  Again you finish the job right on time.  Keep this up and I'll probably get a promotion.
                    case 6:   return 236755;  //Excellent job!
                }
            } else if (agentStanding > Standings::Lo) {
                switch (MakeRandomInt(1, 5)) {
                    case 1:   return 236754;  //LowMed1:  Thank you.  Your accomplishment has been noted and saved into our database.
                    case 2:   return 236756;  //LowMed3:  Thanks, I really appreciate your help.
                    case 3:   return 236757;  //LowMed4:  I won't forget this.
                    case 4:   return 236758;  //LowMed5:  I'm grateful for your assistance.
                    case 5:   return 236759;  //LowMed6:  You have my gratitude.
                }
            } else if (agentStanding > Standings::Bad) {
                switch (MakeRandomInt(1, 4)) {
                    case 1:   return 236751;  //Low1:  Not bad.  Get back to me later for another assignment will you?
                    case 2:   return 236752;  //Low2:  Nice work.  I'm starting to like your style.
                    case 3:   return 236753;  //Low3:  Thanks, your services to {[npcOrganization]agentCorpID.name} is duly appreciated.
                    case 4:   return 133764;  //Thanks and good luck, {[character]player.name}. ')
                }
            } else {
                switch (MakeRandomInt(1, 4)) {
                    case 1:   return 236758;  //I'm grateful for your assistance.
                    case 2:   return 139398;  //I knew you were the right pilot for the job.')
                    case 3:   return 133765;  //Good work, {[character]player.name}. ')
                    case 4:   return 135141;  //Nice work. I'll keep you in mind when something else comes up.')
                }
            }

            // this one will need to be sent as GetTextResponse
            //  UI/Agents/DefaultMessages/MissionCompletedNextMission  236789, '{missionCompletionText}\n<br><br>\nBy the way, I have another mission prepared for you already...'
        } break;

        case Agents::Response::Decline: {
            // set a mission delay here based on standings and system
            float delayMins(6.0f - (agentStanding / 5.0f));  // delay minutes; 4 - 8
            if (sDataMgr.IsFringeSystem(m_agentData.solarSystemID)) {
                // light traffic...
                delayMins += 3.5f;
            } else if (sDataMgr.IsCorridorSystem(m_agentData.solarSystemID)) {
                // med traffic...
                delayMins += 2.5f;
            } else if (sDataMgr.IsHubSystem(m_agentData.solarSystemID)) {
                // higher traffic...
                delayMins += 1.5f;
            }
            if (sDataMgr.IsConSystem(m_agentData.solarSystemID)) {
                // a bit more traffic...
                delayMins -= 0.5f;
            } else if (sDataMgr.IsRegionSystem(m_agentData.solarSystemID)) {
                // possibly higher traffic
                delayMins -= 0.9f;
            }
            SystemManager* pSysMgr = sEntityMgr.FindOrBootSystem(m_agentData.solarSystemID);
            // bonus based on system trusec
            if (pSysMgr != nullptr)
                delayMins -= pSysMgr->GetSecValue(); // 3.9 to 6.0

            // now, modify delay time by agent quality  (lower quality = longer delay)
            float charStanding = sStandingMgr.GetEffectiveStanding(m_agentID, pChar);
            float quality = EvEMath::Agent::EffectiveQuality(m_agentData.quality, pChar->GetSkillLevel(EvESkill::Negotiation), charStanding);
            delayMins *= (1.0f - (quality / 100.0f));   // 3.32 to 5.0

            // sanity check
            if (delayMins < 0.0f)
                delayMins = 0.1f;

            m_delayMap[pChar->itemID()] = (GetFileTimeNow() + (delayMins * EvE::Time::Minute));
            sLog.Cyan("Agent::GetResponse():Decline", "Mission delay set for %.2f minutes", delayMins);

            if (agentStanding > Standings::Hi) {
                switch (MakeRandomInt(1, 3)) {
                    case 1:   return 135879;  //Damnation, {[character]player.name}! You were our best hope. I just pray I can find someone else.
                    case 2:   return 135144;  //Maybe next time, huh? In any case, {[npcOrganization]agentCorpID.name} will manage. It's your loss.
                    case 3:   return 236794;  //Declined2 `Bah, that mission wasn't that bad. Oh well, wait a bit and I'll come up with something else.
                }
            } else if (agentStanding > Standings::MidHi) {
                switch (MakeRandomInt(1, 3)) {
                    case 1:   return 139326;  //That’s too bad, {[character]player.name}. Nothing else available right now, I’m afraid.
                    case 2:   return 135858;  //I'm sorry to hear that, {[character]player.name}. I'll find someone else, I suppose.
                    case 3:   return 135142;  //I hope I can find someone else to handle this.
                }
            } else if (agentStanding > Standings::LoMid) {
                switch (MakeRandomInt(1, 4)) {
                    case 1:   return 133432;  //Too bad, {[character]player.name}. You could have made some serious points with the powers-that-be on this one.
                    case 2:   return 236695;  //Son, I am disappoint.
                    case 3:   return 236793;  //Declined1:  Too bad, I'll try to find someone else then for that job.
                    case 4:   return 236693;  //It's your loss.
                }
            } else if (agentStanding > Standings::Lo) {
                switch (MakeRandomInt(1, 5)) {
                    case 1:   return 139287;  //I see.  Well, perhaps you will be of some use next time.
                    case 2:   return 236795;  //Declined3:  It's your loss.
                    case 3:   return 236796;  //Declined4 `Well, don't expect me to come up with something as good later on.
                    case 4:   return 236797;  //Declined5 `Your wayward ways displease me.
                    case 5:   return 136322;  //I see. Easy money's not good enough, eh? Huh.
                }
            } else if (agentStanding > Standings::Bad) {
                switch (MakeRandomInt(1, 5)) {
                    case 1:   return 139286;  //I see.  Well, perhaps you will be of some use next time.
                    // this should probably be minmater only...
                    case 2:   return 137499;  //Fine, I’ll get someone more capable. A word of advice: don’t repeat this display of cowardice and uselessness. We don’t tolerate such things in the Republic,
                    case 3:   return 236800;  //Quitted1: I had a hunch you wouldn't pull through.
                    case 4:   return 130896;  //You suck.
                    case 5:   return 236801;  //Quitted2 `It's your loss.
                }
            } else {
                switch (MakeRandomInt(1, 5)) {
                    case 1:   return 135855;  //I guess you can't take the heat. There are others who will.
                    case 2:   return 130895;  //Fine.  Be that way.  Asshole.
                    case 3:   return 236802;  //Quitted3 `Your wayward ways displease me, young one.
                    case 4:   return 236843;  //See if I offer this to you again... seriously try it.
                    case 5:   return 135895;  //Lollygagger
                }
            }
        } break;

        case Agents::Response::Defer: {
            if (agentStanding > Standings::Hi) {
            } else if (agentStanding > Standings::MidHi) {
            } else if (agentStanding > Standings::LoMid) {
            } else if (agentStanding > Standings::Lo) {
            } else if (agentStanding > Standings::Bad) {
            } else {
            }

        } break;
        case Agents::Response::Quit: {
            // set a mission delay here based on standings and system
            float delayMins(6.0f - (agentStanding / 5.0f));  // delay minutes; 4 - 8
            SystemManager* pSysMgr = sEntityMgr.FindOrBootSystem(m_agentData.solarSystemID);
            // bonus based on system trusec
            if (pSysMgr != nullptr)
                delayMins -= pSysMgr->GetSecValue(); // 3.9 to 6.0

                // now, modify delay time by agent quality  (lower quality = longer delay)
                float charStanding = sStandingMgr.GetEffectiveStanding(m_agentID, pChar);
            float quality = EvEMath::Agent::EffectiveQuality(m_agentData.quality, pChar->GetSkillLevel(EvESkill::Negotiation), charStanding);
            delayMins *= (1.0f - (quality / 100.0f));   // 3.32 to 5.0

            m_delayMap[pChar->itemID()] = (GetFileTimeNow() + (delayMins * EvE::Time::Minute));
            sLog.Cyan("Agent::GetResponse():Quit", "Mission delay set for %.2f minutes", delayMins);

             if (IsEven(MakeRandomInt(0, 20))) {
                 // should this one be sent only on standings change?
                return 139400;  //You are not the pilot I thought you were. I'll be sure to spread the word of your laziness and ingratitude.')
             } else {
                return 236846;  //Quitters never win.
             }
        } break;

        case Agents::Response::Fail: {
            // set a mission delay here based on standings and system
            float delayMins(6.0f - (agentStanding / 5.0f));  // delay minutes; 4 - 8
            SystemManager* pSysMgr = sEntityMgr.FindOrBootSystem(m_agentData.solarSystemID);
            // bonus based on system trusec
            if (pSysMgr != nullptr)
                delayMins -= pSysMgr->GetSecValue(); // 3.9 to 6.0

                // now, modify delay time by agent quality  (lower quality = longer delay)
                float charStanding = sStandingMgr.GetEffectiveStanding(m_agentID, pChar);
            float quality = EvEMath::Agent::EffectiveQuality(m_agentData.quality, pChar->GetSkillLevel(EvESkill::Negotiation), charStanding);
            delayMins *= (1.0f - (quality / 100.0f));   // 3.32 to 5.0

            m_delayMap[pChar->itemID()] = (GetFileTimeNow() + (delayMins * EvE::Time::Minute));
            sLog.Cyan("Agent::GetResponse():Fail", "Mission delay set for %.2f minutes", delayMins);

            if (agentStanding > Standings::Hi) {
            } else if (agentStanding > Standings::MidHi) {
                return 235999;  //You have failed the mission I gave you. I am disappointed in you. I was hoping for a little more competence.
            } else if (agentStanding > Standings::LoMid) {
                return 139400;  //You are not the pilot I thought you were. I'll be sure to spread the word of your laziness and ingratitude.
            } else if (agentStanding > Standings::Lo) {
                return 236800;  //I had a hunch you wouldn't pull through.
            } else if (agentStanding > Standings::Bad) {
                return 236845;  //You have failed me for the last time.
            } else {
                return 130896;  //You suck.
            }
        } break;

        case Agents::Response::StartResearch: {
            /*UI/Agents/DefaultMessages/RootAgentSays  : 236777;  //ResearchHigh2:  I take it you are back for some more research?  If so then I'll see if I can assist you.
             return 236778;  //ResearchLow1:  Why hello there.  What do you want?
             return 236779;  //ResearchLow5:  If you're here to find an excuse to show off those big, shiny weapons of yours, then I suggest you go find some nincompoop working for the {[npcOrganization]agentFactionID.name} military and leave me alone.  If not, then I might lend you an ear, depending on your worth to me.", None, {u'{[npcOrganization]agentFactionID.name}': {'conditionalValues': [], 'variableType': 1, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'agentFactionID'}})
             return 236780;  //ResearchLow4:  You just interrupted me from my studies.  This better be good ...
             return 236781;  //ResearchLow3:  I take it you are interested in scientific studies.  Perhaps I can be of assistance.
             return 236782;  //ResearchHigh1:  Ah, how lucky I am to see you again!  Then again, luck is an expression for the simple minded, as all occurances can easily be explained with mathematical calculations. Oh what am I rambling about, please have a seat and tell me what is on your mind.
             return 236783;  //ResearchHigh3:  Welcome, fellow colleague.  I guess you came to discuss my new theory on quantum mechanics, or are you here on other business?
             return 236784;  //ResearchHigh4:  It's always a pleasure meeting you, {[character]player.name}.  I suppose you are here for some research, or some work perhaps?", None, {u'{[character]player.name}': {'conditionalValues': [], 'variableType': 0, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'player'}})
             return 236810;  //ResearchLow2:  Come to do a little research eh?  I\'m a busy {[character]agentID.gender -> "man", "girl"}, but I guess I can spare a few moments of my time.', None, {u'{[character]agentID.gender -> "man", "girl"}': {'conditionalValues': [u'man', u'girl'], 'variableType': 0, 'propertyName': 'gender', 'args': 64, 'kwargs': {}, 'variableName': 'agentID'}})
             *
             return 236781;  //I take it you are interested in scientific studies.  Perhaps I can be of assistance.
             return 236810;  //Come to do a little research eh?  I'm a busy {[character]agentID.gender -> "man", "girl"}, but I guess I can spare a few moments of my time.
             return 236811;  //Good job!  I don't know how we'd be able to complete this research project without you!
             return 236812;  //Nice work.  Our research will definitely benefit from this.
             */
        } break;
        case Agents::Response::CancelResearch: {
            /*
             return 236827;  //What was that? I did not get your last transmission
             return 236828;  //I don't have any more datacores to sell for now. You should try again tomorrow.
             return 236831;  //I hope those datacores will come in handy for you
             return 236841;  //I was sure that would break somewhere along the way.
             return 236842;  //I'm willing to give you another chance to <b>NOT</b> screw things up.
             return 236874;  //With you and me working together, I'm sure there will be marvelous breakthroughs.
             return 236875;  //You don't have sufficient skill as a Research Project Manager to handle any more research projects.
             return 236876;  //You don't have the skills to match mine.
             return 236877;  //I'm sorry you feel that way, but alas, our research hadn't gotten anywhere yet so no harm done.
             return 236924;  //Are you sure you want to cancel your current research project?  All your points will be lost and you will quit the current mission you have with this agent.
             return 236925;  //Are you sure you want to cancel your current research project?  All your points will be lost.
             return 263558;  //I can no longer await your response to my offer. I am displeased by your indecisiveness.
             */
        } break;
        case Agents::Response::BuyDatacores:
        case Agents::Response::LocateCharacter:
        case Agents::Response::LocateAccept:
        case Agents::Response::LocateReject:  {
            if (agentStanding > Standings::Hi) {
            } else if (agentStanding > Standings::MidHi) {
            } else if (agentStanding > Standings::LoMid) {
            } else if (agentStanding > Standings::Lo) {
            } else if (agentStanding > Standings::Bad) {
            } else {
            }

        } break;

        case Agents::Response::Expired:  {
            /*
             return 263558;  //I can no longer await your response to my offer. I am displeased by your indecisiveness.
             return 263559;  //Mission Offer Expired
             *
             */

        } break;

    }
    return 236701;
    /*`Oh my! My computer just crashed horribly when I was entering your mission acceptance, and I lost the file on your mission.
    I knew I should have installed a backup file system... Anyhow, the cause of this has been logged on the server along with a nice stack trace explaining what went wrong,
     so the proper authorities will undoubtedly repair this mission as soon as possible. Perhaps you would like another mission in the meantime?
     */

}

// this response is required to be sent as string data to properly display message in agent convo window
std::string Agent::GetTextResponse(Character* pChar, uint8 rspID) {
    float agentStanding = sStandingMgr.GetEffectiveStanding(m_agentID, pChar);
    if (agentStanding > Standings::Hi) {
        return "UI/Agents/DefaultMessages/MissionOffer5"; //236808;  //For you, my friend, there's always something.<br><br>{missionBriefingText}"
    } else if (agentStanding > Standings::MidHi) {
        return "UI/Agents/DefaultMessages/MissionOffer1"; //236804;  //Yes, I have something for you.<br><br>{missionBriefingText}',
    } else if (agentStanding > Standings::LoMid) {
        return "UI/Agents/DefaultMessages/MissionOffer3"; //236806;  //I have just the thing you want.<br><br>{missionBriefingText}',
    } else if (agentStanding > Standings::Lo) {
        return "UI/Agents/DefaultMessages/MissionOffer4"; //236807;  //Something just came up that's right up your alley.<br><br>{missionBriefingText}",
    } else if (agentStanding > Standings::Bad) {
        return "UI/Agents/DefaultMessages/MissionOffer6"; //236809;  //Here is a mission suited for someone of your caliber.<br><br>{missionBriefingText}',
    } else {
        return "UI/Agents/DefaultMessages/MissionOffer2"; //236805;  //Sure I've got something.<br><br>{missionBriefingText}",
    }
}


/*  mission errata....
 *
 * courier missions:
 * L1 missions will keep you within the agent's constellation, up to 450 m3
 * L2/L3 will possibly send you to a neighboring constellation,4-6km3
 * L4 missions will always send you to a neighboring constellation. 8km3
 * If a Distribution mission has an item as a reward instead of ISK, then the item will appear in your personal hangar at the agent's station
 *
 * mining missions:
 * Mining missions require you to mine an asteroid or set of asteroids in a mission space, usually until the asteroids are depleted, and bring the ore back to the agent's station.
 * There is a risk of combat in mining missions, though the hostiles that show up tend to be much weaker than hostiles found in security missions.
 * It is advisable to have some offensive capability (like a set of combat drones) or have a strong enough tank that you can ignore any hostiles that show up and start shooting at you.
 * The mission may require you to mine more ore than can fit in your cargohold; this is typical of mining missions.
 * L1 missions will require mining up to 2km3 of ore,
 * L2 up to 6km3 of ore,
 * L3 up to 9km3 of ore or 10km3 of ice,
 * L4 up to 45km3 of ore, 20km3 of ice or 5km3 gas.
 *
 * encounter missions:
 * Level 1 is where most new players start. Most, if not all, level 1 missions can be done in a basic frigate, Only the most basic piloting skills are required.
 * Level 2 mining missions can be done in a cruiser or in a destroyer piloted by a more skilled pilot. These missions generally expect that you are continually improving your piloting skills and learning how to fit out new ships.
 * Level 3 missions require a battlecruiser. These missions go faster if you have trained for better ships and at least some Tech 2 fittings.
 * Level 4 missions require a Battleship with full T2 tank fitted. These missions can be time-consuming, but they offer large rewards.
 * Level 5 missions are designed for groups of players or capital ship and are exclusively located in Low Security space.
 */


/* agent errata...
 *
 * char mission history and current offers in agtOffers
 * notify:OnIncomingAgentMessage(agentID, message)
 * OnIncomingTransmission(self, transmission, isAgentMission = 0, *args)
 *  transmission.header, transmission.text, transmission.icon
 *        if transmission.header.startswith('[d]'):
 *            transmission.header = transmission.header[3:]
 *            self.delayedTransmission = transmission
 *
 *            (237604, `You can only do this on MASTER or LOCAL server
 *            (235975, `I am in need of your services, {[character]notification_receiverID.name, linkify}, for a very special mission.
 *            (235976, `I found {[character]charID.name, linkify} for you
 *
 ****  see also shit in common/utils/evemath.*  *******
 * Research_Points_Per_Day = Multiplier return (1 + (Agent_Effective_Quality / 100)) return (Core_Skill + Agent_Skill) ^ 2))
 *
 * RP/Day = ((Agent Level + Your Skill)^2 return 1 + (20 + 5 * Negotiation Skill + Agent Effective Standing) / 100)) * Multiplier
 return (Agent Skill + Your Skill)^2 return 1 + Effective Quality / 100)) * Area Bonus
 */
