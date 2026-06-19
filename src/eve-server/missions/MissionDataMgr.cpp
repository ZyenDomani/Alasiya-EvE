
 /**
  * @name MissionDataMgr.cpp
  *   memory object caching system for managing and saving ingame data specific to missions
  *
  * @Author:        Allan
  * @date:      24 June 2018
  * @update:    02 March 2025
  *
  * @note:  MemMgmt data:
  *     no leaks with 0 clients - 12Mar23
  */

#include "../EVEServerConfig.h"

#include "Client.h"
#include "EntityMgr.h"
#include "agents/Agent.h"
#include "agents/AgentMgrService.h"
#include "database/EVEDBUtils.h"
#include "missions/MissionDataMgr.h"
#include "inventory/ItemFactory.h"

MissionDataMgr::MissionDataMgr()
 : KillPNG(nullptr),
 MiningPNG(nullptr),
 CourierPNG(nullptr)
{
}

MissionDataMgr::~MissionDataMgr() {
    PyDecRef(KillPNG);
    PyDecRef(TalkPNG);
    PyDecRef(SmashPNG);
    PyDecRef(MiningPNG);
    PyDecRef(CourierPNG);
    PyDecRef(InteractPNG);
}

void MissionDataMgr::Clear() {
    for (auto &cur : m_offers) {
        PyRep* pbmk = cur.second.bookmarks;
        cur.second.bookmarks = nullptr;
        PySafeDecRef(pbmk);
    }
    for (auto &cur : m_xoffers) {
        PyRep* pbmk = cur.second.bookmarks;
        cur.second.bookmarks = nullptr;
        PySafeDecRef(pbmk);
    }
    
    m_names.clear();
    m_offers.clear();
    m_mining.clear();
    m_courier.clear();
    m_xoffers.clear();
    m_missions.clear();
}

int MissionDataMgr::Initialize() {
    sLog.Blue("   MissionDataMgr", "Mission Data Manager Initialized.");
    Populate();
    return 1;
}

void MissionDataMgr::GetInfo() {
    // nothing to do here
}

// called every 15m from EntityMgr::Process()
void MissionDataMgr::Process() {
    // process open offers every 15m
    Agent* pAgent(nullptr);
    Client* pClient(nullptr);
    std::multimap<uint32, MissionOffer>::iterator itr = m_offers.begin();
    while (itr != m_offers.end()) {
        if (itr->second.stateID < Mission::State::Failed) {
            if (itr->second.expiryTime < GetFileTimeNow()) {
                pAgent = sEntityMgr.GetAgent(itr->second.agentID);
                pClient = sEntityMgr.FindClientByCharID(itr->first);
                // notify client if they are online.  eventually we'll send mail also
                if (itr->second.stateID == Mission::State::Accepted) {
                    pAgent->SendMissionUpdate(pClient, "failed");
                    itr->second.stateID = Mission::State::Failed;
                    if (itr->second.courierTypeID) {
                        // remove item from player's possession
                        if (pClient != nullptr) {
                            pClient->RemoveMissionItem(itr->second.courierTypeID, itr->second.courierAmount);
                        } else {
                            MissionDB::RemoveMissionItem(itr->first, itr->second.courierTypeID, itr->second.courierAmount);
                        }
                    }
                } else if (itr->second.stateID == Mission::State::Offered) {
                    pAgent->SendMissionUpdate(pClient, "offer_expired");
                    itr->second.stateID = Mission::State::Expired;
                }
                std::multimap<uint32, MissionOffer>::iterator itr2 = m_aoffers.find(itr->second.agentID);
                if (itr2 != m_aoffers.end())
                    m_aoffers.erase(itr2);
                m_xoffers.emplace(itr->first, itr->second);
                pAgent->RemoveOffer(itr->first);
                MissionDB::UpdateMissionOffer(itr->second);
                itr = m_offers.erase(itr);
            } else {
                ++itr;
            }
        } else {
            ++itr;
        }
    }
}

void MissionDataMgr::Populate() {
    double start = GetTimeMSeconds();
    double begin = GetTimeMSeconds();

    InteractPNG = new PyString("<img src='res:/UI/netres/mission_content/agent_interaction.png' align=center hspace=4 vspace=4>");
    CourierPNG = new PyString("<img src='res:/UI/netres/mission_content/couriermission.png' align=center hspace=4 vspace=4>");
    MiningPNG = new PyString("<img src='res:/UI/netres/mission_content/miningmission.png' align=center hspace=4 vspace=4>");
    KillPNG = new PyString("<img src='res:/UI/netres/mission_content/killmission.png' align=center hspace=4 vspace=4>");
    TalkPNG = new PyString("<img src='res:/UI/netres/mission_content/agent_talkto.png' align=center hspace=4 vspace=4>");
    SmashPNG = new PyString("<img src='res:/UI/netres/mission_content/smash_and_grab.png' align=center hspace=4 vspace=4>");
    /*  not sure if these are used/needed....
    PNG = new PyString("<img src='res:/UI/netres/mission_content/arc_amarr.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/arc_caldari.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/arc_gallente.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/arc_minmatar.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/arc_npe.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/blood_stained.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/angels_and_artifacts.png' align=center hspace=4 vspace=4>");
    */

    sLog.Cyan("   MissionDataMgr", "Loading Mission Data.  Loaded counts are '#normal(#important)'.");

    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;

    MissionDB::LoadCourierData(*res);
    while (res->GetRow(row)) {
        //SELECT q.id, q.briefingID, q.name, q.level, q.typeID, q.important, q.storyline, q.itemTypeID, q.itemQty, it.volume,
        //q.rewardItemID, q.rewardItemQty, q.bonusTime, q.raceID" FROM qstCourier
        CourierData data        = CourierData();
        data.missionID          = row.GetUInt(0);
        data.briefingID         = row.GetUInt(1);
        data.name               = row.GetText(2);
        data.level              = row.GetUInt8(3);
        data.typeID             = row.GetUInt8(4);
        data.important          = row.GetBool(5);
        data.storyline          = row.GetBool(6);
        data.itemTypeID         = row.GetUInt(7);
        data.itemQty            = row.GetUInt(8);
        data.itemVolume         = row.GetFloat(9);
        data.rewardItemID       = row.GetUInt(10);
        data.rewardItemQty      = row.GetUInt(11);
        data.bonusTime          = row.GetUInt(12);
        data.raceID             = row.GetUInt8(13);
        if (data.important) {
            m_courierImp.emplace(row.GetUInt8(3), data);
        } else {
            m_courier.emplace(row.GetUInt8(3), data);
        }
    }
    sLog.Cyan("   MissionDataMgr", "%lu(%lu) Courier Mission Data Sets loaded in %.3fms.", m_courier.size(), m_courierImp.size(),(GetTimeMSeconds() - start));

    //res->Reset();
    start = GetTimeMSeconds();
    MissionDB::LoadMiningData(*res);
    while (res->GetRow(row)) {
        //SELECT q.id, q.briefingID, q.name, q.level, q.typeID, q.itemTypeID, q.itemQty, it.volume,
        //q.rewardItemID, q.rewardItemQty, q.bonusTime FROM qstMining
        CourierData data        = CourierData();
        data.missionID          = row.GetUInt(0);
        data.briefingID         = row.GetUInt(1);
        data.name               = row.GetText(2);
        data.level              = row.GetUInt8(3);
        data.typeID             = row.GetUInt8(4);
        data.itemTypeID         = row.GetUInt(5);
        data.itemQty            = row.GetUInt(6);
        data.itemVolume         = row.GetFloat(7);
        data.rewardItemID       = row.GetUInt(8);
        data.rewardItemQty      = row.GetUInt(9);
        data.bonusTime          = row.GetUInt(10);
        m_mining.emplace(row.GetUInt8(3), data);
    }
    sLog.Cyan("   MissionDataMgr", "%lu Mining Mission Data Sets loaded in %.3fms.", m_mining.size(), (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0 Encounter Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0 Career Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0 Research Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0 Anomic Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0 Data Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0 Trade Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0 Burner Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0(0) Storyline Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0(0) MultiPart Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    //start = GetTimeMSeconds();
    //sLog.Cyan("   MissionDataMgr", "0(0) Cosmos Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    //start = GetTimeMSeconds();
    //sLog.Cyan("   MissionDataMgr", "0(0) Arc Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    //res->Reset();
    start = GetTimeMSeconds();
    MissionDB::LoadMissionData(*res);
    while (res->GetRow(row)) {
        //SELECT id, briefingID, name, level, typeID, important, storyline, raceID, constellationID, corporationID,
        // dungeonID, rewardItemID, rewardItemQty, bonusTime FROM agtMission
        MissionData data        = MissionData();
        data.missionID          = row.GetUInt(0);
        data.briefingID         = row.GetUInt(1);
        data.name               = row.GetText(2);
        data.level              = row.GetUInt8(3);
        data.typeID             = row.GetUInt8(4);
        data.important          = row.GetBool(5);
        data.storyLine          = row.GetBool(6);
        data.raceID             = row.GetUInt8(7);
        data.constellationID    = row.GetUInt(8);
        data.corporationID      = row.GetUInt(9);
        data.dungeonID          = row.GetUInt(10);
        data.rewardItemID       = row.GetUInt(11);
        data.rewardItemQty      = row.GetUInt(12);
        data.bonusTime          = row.GetUInt(13);
        if (data.important) {
            m_missionsImp.emplace(row.GetUInt8(3), data);
        } else {
            m_missions.emplace(row.GetUInt8(3), data);
        }
    }
    sLog.Cyan("   MissionDataMgr", "%lu(%lu) Unsorted Mission Data Sets loaded in %.3fms.", m_missions.size(), m_missionsImp.size(), (GetTimeMSeconds() - start));

    //res->Reset();
    start = GetTimeMSeconds();
    MissionDB::LoadOpenOffers(*res);
    while (res->GetRow(row)) {
        //SELECT acceptFee, agentID, characterID, courierAmount, courierTypeID, courierItemVolume, dateAccepted, dateIssued, destinationID, destinationTypeID, destinationOwnerID, destinationSystemID,
        // expiryTime, important, storyline, missionID, briefingID, name, offerID, originID, originOwnerID, originSystemID, remoteCompletable, remoteOfferable,
        // rewardISK, rewardItemID, rewardItemQty, rewardLP, bonusISK, bonusTime, stateID, typeID, dungeonLocationID, dungeonSolarSystemID FROM agtOffers
        MissionOffer offer              = MissionOffer();
        offer.acceptFee                 = row.GetUInt(0);
        offer.agentID                   = row.GetUInt(1);
        offer.characterID               = row.GetUInt(2);
        offer.courierAmount             = row.GetUInt(3);
        offer.courierTypeID             = row.GetUInt(4);
        offer.courierItemVolume         = row.GetFloat(5);
        offer.dateAccepted              = row.GetInt64(6);
        offer.dateIssued                = row.GetInt64(7);
        offer.destinationID             = row.GetUInt(8);
        offer.destinationTypeID         = row.GetUInt(9);
        offer.destinationOwnerID        = row.GetUInt(10);
        offer.destinationSystemID       = row.GetUInt(11);
        offer.expiryTime                = row.GetInt64(12);
        offer.important                 = row.GetBool(13);
        offer.storyline                 = row.GetBool(14);
        offer.missionID                 = row.GetUInt(15);
        offer.briefingID                = row.GetUInt(16);
        offer.name                      = row.GetText(17);
        offer.offerID                   = row.GetUInt(18);
        offer.originID                  = row.GetUInt(19);
        offer.originOwnerID             = row.GetUInt(20);
        offer.originSystemID            = row.GetUInt(21);
        offer.remoteCompletable         = row.GetBool(22);
        offer.remoteOfferable           = row.GetBool(23);
        offer.rewardISK                 = row.GetUInt(24);
        offer.rewardItemID              = row.GetUInt(25);
        offer.rewardItemQty             = row.GetUInt(26);
        offer.rewardLP                  = row.GetUInt(27);
        offer.bonusISK                  = row.GetUInt(28);
        offer.bonusTime                 = row.GetUInt(29);
        offer.stateID                   = row.GetUInt8(30);
        offer.typeID                    = row.GetUInt8(31);
        offer.dungeonLocationID         = row.GetUInt(32);
        offer.dungeonSolarSystemID      = row.GetUInt(33);
        offer.dateCompleted             = 0;
        // will need to determine how to store/retrieve bookmarks as a list of dicts here
        offer.bookmarks                 = new PyList();
        m_offers.emplace(row.GetUInt(2), offer);
        m_aoffers.emplace(row.GetUInt(1), offer);    // do we really want dupe data here?  yes.  need offer by char and by agent
    }
    sLog.Cyan("   MissionDataMgr", "%lu Open Mission Offers loaded in %.3fms.", m_offers.size(), (GetTimeMSeconds() - start));

    //res->Reset();
    start = GetTimeMSeconds();
    // config switch to allow loading/displaying of expired/completed mission offers
    if (sConfig.server.LoadOldMissions)
        MissionDB::LoadClosedOffers(*res);
    while (res->GetRow(row)) {
        //TODO: determine if these are used.  if so, complete data population
        //SELECT agentID, characterID, courierAmount, courierTypeID, dateAccepted, dateCompleted, dateIssued, destinationID, expiryTime, important, storyline, missionID, name,
        // offerID, originID, rewardISK, rewardItemID, rewardItemQty, rewardLP, stateID, typeID FROM agtOffers
        MissionOffer offer              = MissionOffer();
        offer.agentID                   = row.GetUInt(0);
        offer.characterID               = row.GetUInt(1);
        offer.courierAmount             = row.GetUInt(2);
        offer.courierTypeID             = row.GetUInt(3);
        offer.dateAccepted              = row.GetInt64(4);
        offer.dateCompleted             = row.GetInt64(5);
        offer.dateIssued                = row.GetInt64(6);
        offer.destinationID             = row.GetUInt(7);
        offer.expiryTime                = row.GetInt64(8);
        offer.important                 = row.GetBool(9);
        offer.storyline                 = row.GetBool(10);
        offer.missionID                 = row.GetUInt(11);
        offer.name                      = row.GetText(12);
        offer.offerID                   = row.GetUInt(13);
        offer.originID                  = row.GetUInt(14);
        offer.rewardISK                 = row.GetUInt(15);
        offer.rewardItemID              = row.GetUInt(16);
        offer.rewardItemQty             = row.GetUInt(17);
        offer.rewardLP                  = row.GetUInt(18);
        offer.stateID                   = row.GetUInt8(19);
        offer.typeID                    = row.GetUInt8(20);
        offer.briefingID                = 0;
        offer.acceptFee                 = 0;
        offer.bonusISK                  = 0;
        offer.bonusTime                 = 0;
        offer.remoteCompletable         = 0;
        offer.remoteOfferable           = 0;
        offer.originOwnerID             = 0;
        offer.originSystemID            = 0;
        offer.destinationTypeID         = 0;
        offer.destinationOwnerID        = 0;
        offer.destinationSystemID       = 0;
        offer.dungeonLocationID         = 0;
        offer.dungeonSolarSystemID      = 0;
        offer.bookmarks                 = new PyList(); //invalid offers will not have bms
        m_xoffers.emplace(row.GetUInt(2), offer);
    }
    sLog.Cyan("   MissionDataMgr", "%lu Closed Mission Offers loaded in %.3fms.", m_xoffers.size(), (GetTimeMSeconds() - start));

    // cleanup
    SafeDelete(res);
    sLog.Cyan("   MissionDataMgr", "Mission Data loaded in %.3fms.", (GetTimeMSeconds() - begin));
}

void MissionDataMgr::AddMissionOffer(uint32 charID, MissionOffer& data)
{
    m_offers.emplace(charID, data);
    m_aoffers.emplace(data.agentID, data);
}

void MissionDataMgr::RemoveMissionOffer(uint32 charID, MissionOffer& data)
{
    auto itr = m_offers.equal_range(charID);
    for (auto it = itr.first; it != itr.second; ++it)
        if (it->second.agentID == data.agentID) {
            m_offers.erase(it);
            break;
        }

    itr = m_aoffers.equal_range(data.agentID);
    for (auto it = itr.first; it != itr.second; ++it)
        if (it->second.characterID == charID) {
            m_aoffers.erase(it);
            break;
        }
}

void MissionDataMgr::LoadAgentOffers(const uint32 agentID, std::map< uint32, MissionOffer >& data)
{
    auto itr = m_aoffers.equal_range(agentID);
    for (auto it = itr.first; it != itr.second; ++it)
        data[it->second.characterID] = (it->second);
}

void MissionDataMgr::LoadMissionOffers(uint32 charID, std::vector<MissionOffer>& data)
{
    auto itr = m_offers.equal_range(charID);
    for (auto it = itr.first; it != itr.second; ++it)
        data.push_back(it->second);

    // config switch to allow loading/displaying of expired/completed mission offers
    // not completely working yet.....AgentMgrService::Handle_GetMyJournalDetails() will need work to implement this.
    if (sConfig.server.LoadOldMissions) {
        auto itr = m_xoffers.equal_range(charID);
        for (auto it = itr.first; it != itr.second; ++it)
            data.push_back(it->second);
    }
}

void MissionDataMgr::CreateMissionOffer(uint8 typeID, uint8 level, uint8 raceID, bool important, MissionOffer& data)
{
    // variable mission data based on agent, init to 0 here.
    data = MissionOffer();
    data.stateID                = Mission::State::Allocated;
    data.dateIssued             = GetFileTimeNow();

    switch (typeID) {
        case Mission::Type::Courier: {
            CourierData cData = CourierData();
            std::vector<CourierData> cVec;
            if (important) {
                auto itr = m_courierImp.equal_range(level);
                for (auto it = itr.first; it != itr.second; ++it) {
                    if (it->second.raceID == 0) {
                        cVec.push_back(it->second);
                    } else if (it->second.raceID == raceID) {
                        cVec.push_back(it->second);
                    }
                }
            } else {
                auto itr = m_courier.equal_range(level);
                for (auto it = itr.first; it != itr.second; ++it) {
                    if (it->second.raceID == 0) {
                        cVec.push_back(it->second);
                    } else if (it->second.raceID == raceID) {
                        cVec.push_back(it->second);
                    }
                }
            }

            // pick random mission from group
            cData = cVec[MakeRandomUInt(0, (cVec.size() - 1))];
            data.name               = cData.name;
            data.typeID             = cData.typeID;
            data.bonusTime          = cData.bonusTime;
            data.important          = cData.important;
            data.storyline          = cData.storyline;
            data.missionID          = cData.missionID;
            data.briefingID         = cData.briefingID;
            data.rewardItemID       = cData.rewardItemID;
            data.rewardItemQty      = cData.rewardItemQty;
            data.courierTypeID      = cData.itemTypeID;
            data.courierAmount      = cData.itemQty;
            data.courierItemVolume  = cData.itemVolume;
        } break;
        case Mission::Type::Mining: {
            CourierData cData = CourierData();
            std::vector<CourierData> cVec;
            auto itr = m_mining.equal_range(level);
            for (auto it = itr.first; it != itr.second; ++it) {
                if (it->second.raceID == 0) {
                    cVec.push_back(it->second);
                } else if (it->second.raceID == raceID) {
                    cVec.push_back(it->second);
                }
            }

            // pick random mission from group
            cData = cVec[MakeRandomUInt(0, (cVec.size() - 1))];
            data.name               = cData.name;
            data.typeID             = cData.typeID;
            data.bonusTime          = cData.bonusTime;
            data.important          = cData.important;
            data.storyline          = cData.storyline;
            data.missionID          = cData.missionID;
            data.briefingID         = cData.briefingID;
            data.rewardItemID       = cData.rewardItemID;
            data.rewardItemQty      = cData.rewardItemQty;
            data.courierTypeID      = cData.itemTypeID;
            data.courierAmount      = cData.itemQty;
            data.courierItemVolume  = cData.itemVolume;
        } break;
        case Mission::Type::Tutorial: {
            // maybe use these for career agents
        } break;
        case Mission::Type::Encounter: {
        } break;
        case Mission::Type::Trade: {
        } break;
        case Mission::Type::Research: {
        } break;
        case Mission::Type::Data: {
        } break;
        case Mission::Type::Storyline: {
        } break;
        case Mission::Type::Cosmos: {
        } break;
        case Mission::Type::Arc: {
        } break;
        case Mission::Type::Anomic: {
        } break;
    }

    // create and save bookmarks for this offer.... not sure how yet.
    data.bookmarks              = new PyList();

    _log(AGENT__DEBUG, "Created %s level %u %s offer - '%s'", (important?"an important":"a"), level, GetTypeName(data.typeID).c_str(), data.name.c_str());
}


std::string MissionDataMgr::GetTypeName(uint8 typeID) {
    using namespace Mission::Type;
    switch (typeID) {
        case Tutorial:          return "Tutorial";
        case Encounter:         return "Encounter";
        case Courier:           return "Courier";
        case Trade:             return "Trade";
        case Mining:            return "Mining";
        case Research:          return "Research";
        case Data:              return "Data";
        case Storyline:         return "Storyline";
        case Cosmos:            return "Cosmos";
        case Arc:               return "Arc";
        case Anomic:            return "Anomic";
        case Burner:            return "Burner";
        default:                return "Invalid";
    }
}

std::string MissionDataMgr::GetTypeLabel(uint8 typeID) {
    using namespace Mission::Type;
    switch (typeID) {
        case Tutorial:          return "UI/Agents/MissionTypes/Tutorial";
        case Encounter:         return "UI/Agents/MissionTypes/Encounter";
        case Courier:           return "UI/Agents/MissionTypes/Courier";
        case Trade:             return "UI/Agents/MissionTypes/Trade";
        case Mining:            return "UI/Agents/MissionTypes/Mining";
        case Research:          return "UI/Agents/MissionTypes/Research";
        case Data:              return "UI/Agents/MissionTypes/Data";
        case Storyline:         return "UI/Agents/MissionTypes/Storyline";
        case Cosmos:            return "UI/Agents/MissionTypes/Cosmos";
        case Arc:               return "UI/Agents/MissionTypes/EpicArc";
        case Anomic:            return "UI/Agents/MissionTypes/Anomic";
        case Burner:            return "UI/Agents/MissionTypes/Burner";
        default:                return "Invalid";
    }
}

void MissionDataMgr::UpdateMissionData(uint32 charID, MissionOffer& data) {
    auto itr = m_offers.equal_range(charID);
    for (auto it = itr.first; it != itr.second; ++it)
        if (it->second.agentID == data.agentID) {
            it->second = data;
            break;
        }

    itr = m_aoffers.equal_range(data.agentID);
    for (auto it = itr.first; it != itr.second; ++it)
        if (it->second.characterID == charID) {
            it->second = data;
            break;
        }
}
