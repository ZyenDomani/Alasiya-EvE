
 /**
  * @name MissionDataMgr.cpp
  *   memory object caching system for managing and saving ingame data specific to missions
  *
  * @Author:        Allan
  * @date:      24 June 2018
  *
  */


#include "../EVEServerConfig.h"
#include "missions/MissionDataMgr.h"
#include "database/EVEDBUtils.h"

MissionDataMgr::MissionDataMgr()
{
    m_names.clear();
    m_offers.clear();
    m_mining.clear();
    m_courier.clear();
    m_xoffers.clear();
    m_missions.clear();
}

MissionDataMgr::~MissionDataMgr()
{
    PyDecRef(KillPNG);
    PyDecRef(MiningPNG);
    PyDecRef(CourierPNG);
}

void MissionDataMgr::Clear()
{
    m_names.clear();
    m_offers.clear();
    m_mining.clear();
    m_courier.clear();
    m_xoffers.clear();
    m_missions.clear();
}

int MissionDataMgr::Initialize()
{
    Populate();
    return 1;

}

void MissionDataMgr::GetInfo()
{

}

void MissionDataMgr::Populate()
{
    double start = GetTimeMSeconds();

    CourierPNG = new PyString("<img src='res:/UI/netres/mission_content/couriermission.png' align=center hspace=4 vspace=4>");
    MiningPNG = new PyString("<img src='res:/UI/netres/mission_content/miningmission.png' align=center hspace=4 vspace=4>");
    KillPNG = new PyString("<img src='res:/UI/netres/mission_content/killmission.png' align=center hspace=4 vspace=4>");
    /*  not sure if these are used/needed....
    PNG = new PyString("<img src='res:/UI/netres/mission_content/agent_interaction.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/agent_talkto.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/arc_amarr.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/arc_caldari.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/arc_gallente.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/arc_minmatar.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/arc_npe.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/blood_stained.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/angels_and_artifacts.png' align=center hspace=4 vspace=4>");
    PNG = new PyString("<img src='res:/UI/netres/mission_content/smash_and_grab.png' align=center hspace=4 vspace=4>");
    */

    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;

    MissionDB::LoadCourierData(*res);
    while (res->GetRow(row)) {
        //SELECT id, contentID, name, level, typeID, important, storyline, itemTypeID, itemQty, rewardISK, rewardItemID, rewardItemQty, bonusISK, bonusTime FROM qstCourier
        CourierData data;
        data.missionID     = row.GetInt(0);
        data.contentID     = row.GetInt(1);
        data.name          = row.GetText(2);
        data.level         = row.GetInt(3);
        data.typeID        = row.GetInt(4);
        data.important     = row.GetBool(5);
        data.storyline     = row.GetBool(6);
        data.itemTypeID    = row.GetInt(7);
        data.itemQty       = row.GetInt(8);
        data.rewardISK     = row.GetInt(9);
        data.rewardItemID  = row.GetInt(10);
        data.rewardItemQty = row.GetInt(11);
        data.bonusISK      = row.GetInt(12);
        data.bonusTime     = row.GetUInt(13);
        m_courier.emplace(row.GetInt(3), data);
    }
    sLog.Cyan("   MissionDataMgr", "%u Courier Mission Data Sets loaded in %.3fms.", m_courier.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    MissionDB::LoadMiningData(*res);
    while (res->GetRow(row)) {
        //SELECT id, contentID, name, level, typeID, important, storyline, itemTypeID, itemQty, rewardISK, rewardItemID, rewardItemQty, bonusISK, bonusTime FROM qstMining
        CourierData data;
        data.missionID     = row.GetInt(0);
        data.contentID     = row.GetInt(1);
        data.name          = row.GetText(2);
        data.level         = row.GetInt(3);
        data.typeID        = row.GetInt(4);
        data.important     = row.GetBool(5);
        data.storyline     = row.GetBool(6);
        data.itemTypeID    = row.GetInt(7);
        data.itemQty       = row.GetInt(8);
        data.rewardISK     = row.GetInt(9);
        data.rewardItemID  = row.GetInt(10);
        data.rewardItemQty = row.GetInt(11);
        data.bonusISK      = row.GetInt(12);
        data.bonusTime     = row.GetUInt(13);
        m_mining.emplace(row.GetInt(3), data);
    }
    sLog.Cyan("   MissionDataMgr", "%u Mining Mission Data Sets loaded in %.3fms.", m_mining.size(), (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0 Encounter Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0 Storyline Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0 Tutorial Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

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
    sLog.Cyan("   MissionDataMgr", "0 Cosmos Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    start = GetTimeMSeconds();
    sLog.Cyan("   MissionDataMgr", "0 Arc Mission Data Sets loaded in %.3fms.", (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    MissionDB::LoadMissionData(*res);
    while (res->GetRow(row)) {
        //SELECT id, contentID, name, level, typeID, important, storyline, raceID, constellationID, corporationID, dungeonID,
        // rewardISK, rewardItemID, rewardISKQty, rewardItemQty, bonusISK, bonusTime FROM agtMissions
        MissionData data;
        data.missionID = row.GetInt(0);
        data.contentID = row.GetInt(1);
        data.name = row.GetText(2);
        data.level = row.GetInt(3);
        data.typeID = row.GetInt(4);
        data.important = row.GetBool(5);
        data.constellationID = row.GetInt(8);
        data.corporationID = row.GetInt(9);
        data.dungeonID = row.GetInt(10);
        m_missions.emplace(row.GetInt(3), data);
    }
    sLog.Cyan("   MissionDataMgr", "%u Unsorted Mission Data Sets loaded in %.3fms.", m_missions.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    MissionDB::LoadOpenOffers(*res);
    while (res->GetRow(row)) {
        //SELECT acceptFee, agentID, characterID, courierAmount, courierItemID, dateAccepted, dateIssued, destinationID, destinationTypeID, destinationOwnerID, destinationSystemID,
        // expiryTime, important, storyline, missionID, contentID, name, offerID, originID, originOwnerID, originSystemID, remoteCompletable, remoteOfferable,
        // rewardISK, rewardItemID, rewardItemQty, rewardLP, bonusISK, bonusTime, stateID, typeID, dungeonLocationID, dungeonSolarSystemID FROM agtOffers
        MissionOffer data;
        data.acceptFee = row.GetInt(0);
        data.agentID = row.GetInt(1);
        data.characterID = row.GetInt(2);
        data.courierAmount = row.GetInt(3);
        data.courierItemID = row.GetInt(4);
        data.dateAccepted = row.GetInt64(5);
        data.dateIssued = row.GetInt64(6);
        data.destinationID = row.GetInt(7);
        data.destinationTypeID = row.GetInt(8);
        data.destinationOwnerID = row.GetInt(9);
        data.destinationSystemID = row.GetInt(10);
        data.expiryTime = row.GetInt64(11);
        data.important = row.GetInt(12);
        data.storyline = row.GetInt(13);
        data.missionID = row.GetInt(14);
        data.contentID = row.GetInt(15);
        data.name = row.GetText(16);
        data.offerID = row.GetInt(17);
        data.originID = row.GetInt(18);
        data.originOwnerID = row.GetInt(19);
        data.originSystemID = row.GetInt(20);
        data.remoteCompletable = row.GetInt(21);
        data.remoteOfferable = row.GetInt(22);
        data.rewardISK = row.GetInt(23);
        data.rewardItemID = row.GetInt(24);
        data.rewardItemQty = row.GetInt(25);
        data.rewardLP = row.GetInt(26);
        data.bonusISK = row.GetInt(27);
        data.bonusTime = row.GetInt(28);
        data.stateID = row.GetInt(29);
        data.typeID = row.GetInt(30);
        data.dateCompleted = 0;
        data.dungeonLocationID = 0;
        data.dungeonSolarSystemID = 0;
        // will need to determine how to store/retrieve bookmarks as a list of dicts here
        data.bookmarks = new PyList();
        m_offers.emplace(row.GetInt(2), data);
        m_aoffers.emplace(row.GetInt(1), data);    // do we really want dupe data here?  yes.  need offer by char and by agent
    }
    sLog.Cyan("   MissionDataMgr", "%u Open Mission Offers loaded in %.3fms.", m_offers.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    // config switch to allow loading/displaying of expired/completed mission offers
    if (sConfig.server.LoadOldMissions)
        MissionDB::LoadClosedOffers(*res);
    while (res->GetRow(row)) {
        //SELECT agentID, characterID, courierAmount, courierItemID, dateAccepted, dateCompleted, dateIssued, destinationID, expiryTime, important, storyline, missionID, name,
        // offerID, originID, rewardISK, rewardItemID, rewardItemQty, rewardLP, stateID, typeID FROM agtOffers
        MissionOffer data;
        data.agentID = row.GetInt(0);
        data.characterID = row.GetInt(1);
        data.courierAmount = row.GetInt(2);
        data.courierItemID = row.GetInt(3);
        data.dateAccepted = row.GetInt64(4);
        data.dateCompleted = row.GetInt64(5);
        data.dateIssued = row.GetInt64(6);
        data.destinationID = row.GetInt(7);
        data.expiryTime = row.GetInt64(8);
        data.important = row.GetInt(9);
        data.storyline = row.GetInt(10);
        data.missionID = row.GetInt(11);
        data.name = row.GetText(12);
        data.offerID = row.GetInt(13);
        data.originID = row.GetInt(14);
        data.rewardISK = row.GetInt(15);
        data.rewardItemID = row.GetInt(16);
        data.rewardItemQty = row.GetInt(17);
        data.rewardLP = row.GetInt(18);
        data.stateID = row.GetInt(19);
        data.typeID = row.GetInt(20);
        data.contentID = 0;
        data.acceptFee = 0;
        data.bonusISK = 0;
        data.bonusTime = 0;
        data.remoteCompletable = 0;
        data.remoteOfferable = 0;
        data.originOwnerID = 0;
        data.originSystemID = 0;
        data.destinationTypeID = 0;
        data.destinationOwnerID = 0;
        data.destinationSystemID = 0;
        data.dungeonLocationID = 0;
        data.dungeonSolarSystemID = 0;
        data.bookmarks = new PyList(); //invalid offers will not have bms
        m_xoffers.emplace(row.GetInt(2), data);
    }
    sLog.Cyan("   MissionDataMgr", "%u Closed Mission Offers loaded in %.3fms.", m_xoffers.size(), (GetTimeMSeconds() - start));

    // cleanup
    SafeDelete(res);
/*
    m_names.emplace("Arisite Envy",   45000 );
    m_names.emplace("Asteroid Catastrophe",    1080 );
    m_names.emplace("Better World", 6000    );
    m_names.emplace("Beware They Live",    9000   );
    m_names.emplace("Bountiful Bandine",   2000         );
    m_names.emplace("Burnt Traces",    1080           );
    m_names.emplace("Cheap Chills",    20000  );
    m_names.emplace("Claimjumpers",    1800       );
    m_names.emplace("Data Mining", 299   );
    m_names.emplace("Down and Dirty",  2250       );
    m_names.emplace("Drone Distribution",  4000   );
    m_names.emplace("Feeding the Giant",   44800 );
    m_names.emplace("Gas Injections",  4250  );
    m_names.emplace("Geodite and Gemology",    44800 );
    m_names.emplace("Ice Installation",    20000  );
    m_names.emplace("Like Drones to a Cloud",  4250 );
    m_names.emplace("Mercium Belt",    6000   );
    m_names.emplace("Mercium Experiments", 1080     );
    m_names.emplace("Mother Lode", 44800  );
    m_names.emplace("Not Gneiss at All",   45000 );
    m_names.emplace("Pile of Pithix",  9000   );
    m_names.emplace("Persistent Pests",    4000   );
    m_names.emplace("Starting Simple", 2000     );
    m_names.emplace("Stay Frosty", 10000   );
    m_names.emplace("Understanding Augmene",   2625  );
    m_names.emplace("Unknown Events",  6000 );
    */
}

void MissionDataMgr::AddMissionOffer(uint32 charID, MissionOffer& data)
{
    m_offers.emplace(charID, data);
    m_aoffers.emplace(data.agentID, data);
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
    if (sConfig.server.LoadOldMissions) {
        auto itr = m_xoffers.equal_range(charID);
        for (auto it = itr.first; it != itr.second; ++it)
            data.push_back(it->second);
    }
}

void MissionDataMgr::CreateMissionOffer(uint8 typeID, uint8 level, MissionOffer& data)
{
    // variable mission data based on agent, init to 0 here.
    data.stateID                = Mission::State::Allocated;
    data.dateIssued             = GetFileTimeNow();
    data.remoteOfferable        = false;
    data.remoteCompletable      = false;
    data.range                  = 0;
    data.offerID                = 0;
    data.agentID                = 0;
    data.rewardLP               = 0;
    data.originID               = 0;
    data.originOwnerID          = 0;
    data.originSystemID         = 0;
    data.acceptFee              = 0;
    data.expiryTime             = 0;
    data.characterID            = 0;
    data.dateAccepted           = 0;
    data.dateCompleted          = 0;
    data.destinationID          = 0;
    data.destinationTypeID      = 0;
    data.destinationOwnerID     = 0;
    data.destinationSystemID    = 0;
    data.dungeonLocationID      = 0;
    data.dungeonSolarSystemID   = 0;
    data.bookmarks          = new PyList();

    switch (typeID) {
        case Mission::Type::Tutorial: {

        } break;
        case Mission::Type::Encounter: {

        } break;
        case Mission::Type::Courier: {
            CourierData cData;
            std::vector<CourierData> cVec;
            auto itr = m_courier.equal_range(level);
            for (auto it = itr.first; it != itr.second; ++it)
                cVec.push_back(it->second);
            cData = cVec[MakeRandomInt(0, (cVec.size() -1))];

            data.name               = cData.name;
            data.typeID             = cData.typeID;
            data.bonusISK           = cData.bonusISK;
            data.rewardISK          = cData.rewardISK;
            data.bonusTime          = cData.bonusTime;
            data.important          = cData.important;
            data.storyline          = cData.storyline;
            data.missionID          = cData.missionID;
            data.contentID          = cData.contentID;
            data.rewardItemID       = cData.rewardItemID;
            data.rewardItemQty      = cData.rewardItemQty;
            data.courierItemID      = cData.itemTypeID;
            data.courierAmount      = cData.itemQty;
        } break;
        case Mission::Type::Trade: {

        } break;
        case Mission::Type::Mining: {
            CourierData cData;
            std::vector<CourierData> cVec;
            auto itr = m_courier.equal_range(level);
            for (auto it = itr.first; it != itr.second; ++it)
                cVec.push_back(it->second);
            cData = cVec[MakeRandomInt(0, (cVec.size() -1))];

            data.name               = cData.name;
            data.typeID             = cData.typeID;
            data.bonusISK           = cData.bonusISK;
            data.rewardISK          = cData.rewardISK;
            data.bonusTime          = cData.bonusTime;
            data.important          = cData.important;
            data.storyline          = cData.storyline;
            data.missionID          = cData.missionID;
            data.contentID          = cData.contentID;
            data.rewardItemID       = cData.rewardItemID;
            data.rewardItemQty      = cData.rewardItemQty;
            data.courierItemID      = cData.itemTypeID;
            data.courierAmount      = cData.itemQty;
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

    _log(AGENT__DEBUG, "Created a level %u %s offer - '%s'", level, GetTypeName(data.typeID).c_str(), data.name.c_str());

}


std::string MissionDataMgr::GetTypeName(uint8 typeID)
{
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
        default:                return "Invalid";
    }
}


void MissionDataMgr::UpdateMissionData()
{
    double start = GetTimeMSeconds();

    DBerror err;
    int16 count = 0;
    /*
     *    std::map<std::string, uint32>::iterator itr = m_names.begin(), end = m_names.end();
     *    for (; itr != end; ++itr) {
     *        if (sDatabase.RunQuery(err,"UPDATE qstMining SET itemQty = %u WHERE name LIKE '%s'", itr->second, itr->first.c_str() ))
     *            ++count;
     *        else
     *            sLog.Error("   MissionDataMgr", "%s not found", itr->first.c_str());
}
*/
    sLog.Cyan("   MissionDataMgr", "UpdateMissionData - %u missions udpated in %.3f ms.", count, (GetTimeMSeconds() - start));
}

