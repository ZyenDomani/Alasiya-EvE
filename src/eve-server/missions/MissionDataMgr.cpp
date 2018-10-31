
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
    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;

    MissionDB::LoadCourierData(*res);
    while (res->GetRow(row)) {
        //SELECT id, contentID, name, level, typeID, important, storyline, itemTypeID, itemQty, rewardISK, rewardItemID, rewardItemQty, bonusISK, bonusTime FROM qstCourier
        CourierData cData;
        cData.missionID     = row.GetInt(0);
        cData.name          = row.GetText(1);
        cData.contentID     = row.GetInt(2);
        cData.level         = row.GetInt(3);
        cData.typeID        = row.GetInt(4);
        cData.important     = row.GetBool(5);
        cData.storyline     = row.GetBool(6);
        cData.itemTypeID    = row.GetInt(7);
        cData.itemQty       = row.GetInt(8);
        cData.rewardISK     = row.GetInt(9);
        cData.rewardItemID  = row.GetInt(10);
        cData.rewardItemQty = row.GetInt(11);
        cData.bonusISK      = row.GetInt(12);
        cData.bonusTime     = row.GetUInt(13);
        m_courier.emplace(row.GetInt(3), cData);
    }
    sLog.Cyan("   MissionDataMgr", "%u Courier Mission Data Sets loaded in %.3fms.", m_courier.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    MissionDB::LoadMiningData(*res);
    while (res->GetRow(row)) {
        //SELECT id, contentID, name, level, typeID, important, storyline, itemTypeID, itemQty, rewardISK, rewardItemID, rewardItemQty, bonusISK, bonusTime FROM qstMining
        CourierData cData;
        cData.missionID     = row.GetInt(0);
        cData.name          = row.GetText(1);
        cData.contentID     = row.GetInt(2);
        cData.level         = row.GetInt(3);
        cData.typeID        = row.GetInt(4);
        cData.important     = row.GetBool(5);
        cData.storyline     = row.GetBool(6);
        cData.itemTypeID    = row.GetInt(7);
        cData.itemQty       = row.GetInt(8);
        cData.rewardISK     = row.GetInt(9);
        cData.rewardItemID  = row.GetInt(10);
        cData.rewardItemQty = row.GetInt(11);
        cData.bonusISK      = row.GetInt(12);
        cData.bonusTime     = row.GetUInt(13);
        m_mining.emplace(row.GetInt(3), cData);
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
        MissionData mData;
        mData.missionID = row.GetInt(0);
        mData.contentID = row.GetInt(1);
        mData.name = row.GetText(2);
        mData.level = row.GetInt(3);
        mData.typeID = row.GetInt(4);
        mData.important = row.GetBool(5);
        mData.constellationID = row.GetInt(8);
        mData.corporationID = row.GetInt(9);
        mData.dungeonID = row.GetInt(10);
        m_missions.emplace(row.GetInt(3), mData);
    }
    sLog.Cyan("   MissionDataMgr", "%u Unsorted Mission Data Sets loaded in %.3fms.", m_missions.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    MissionDB::LoadOpenOffers(*res);
    while (res->GetRow(row)) {
        //SELECT acceptFee, agentID, characterID, courierAmount, courierItemID, dateAccepted, dateCompleted, dateIssued, destinationID, expiryTime, important, storyline,
        // missionID, contentID, name, offerID, originID, remoteCompletable, remoteOfferable, rewardISK, rewardItemID, rewardItemQty, rewardLP, bonusISK, bonusTime,
        // stateID, typeID FROM agtOffers
        MissionOffer oData;
        oData.acceptFee = row.GetInt(0);
        oData.agentID = row.GetInt(1);
        // will need to determine how to store/retrieve bookmarks as a list of dicts here
        oData.bookmarks = new PyList();
        oData.characterID = row.GetInt(2);
        oData.courierAmount = row.GetInt(3);
        oData.courierItemID = row.GetInt(4);
        oData.dateAccepted = row.GetInt64(5);
        oData.dateCompleted = row.GetInt64(6);
        oData.dateIssued = row.GetInt64(7);
        oData.destinationID = row.GetInt(8);
        oData.expiryTime = row.GetInt64(9);
        oData.important = row.GetInt(10);
        oData.storyline = row.GetInt(11);
        oData.missionID = row.GetInt(12);
        oData.contentID = row.GetInt(13);
        oData.name = row.GetInt(14);
        oData.offerID = row.GetInt(15);
        oData.originID = row.GetInt(16);
        oData.remoteCompletable = row.GetInt(17);
        oData.remoteOfferable = row.GetInt(18);
        oData.rewardISK = row.GetInt(19);
        oData.rewardItemID = row.GetInt(20);
        oData.rewardItemQty = row.GetInt(21);
        oData.rewardLP = row.GetInt(22);
        oData.bonusISK = row.GetInt(23);
        oData.bonusTime = row.GetInt(24);
        oData.stateID = row.GetInt(25);
        oData.typeID = row.GetInt(26);
        m_offers.emplace(row.GetInt(2), oData);
        m_aoffers.emplace(row.GetInt(1), oData);    // do we really want dupe data here?
    }
    sLog.Cyan("   MissionDataMgr", "%u Open Mission Offers loaded in %.3fms.", m_offers.size(), (GetTimeMSeconds() - start));

    res->Reset();
    start = GetTimeMSeconds();
    // config switch to allow loading/displaying of expired/completed mission offers
    if (sConfig.server.LoadOldMissions)
        MissionDB::LoadClosedOffers(*res);
    while (res->GetRow(row)) {
        //SELECT acceptFee, agentID, characterID, courierAmount, courierItemID, dateAccepted, dateCompleted, dateIssued, destinationID, expiryTime, important, storyline,
        // missionID, contentID, name, offerID, originID, remoteCompletable, remoteOfferable, rewardISK, rewardItemID, rewardItemQty, rewardLP, bonusISK, bonusTime,
        // stateID, typeID FROM agtOffers
        MissionOffer oData;
        oData.acceptFee = row.GetInt(0);
        oData.agentID = row.GetInt(1);
        oData.bookmarks = new PyList(); //invalid offers will not have bms
        oData.characterID = row.GetInt(2);
        oData.courierAmount = row.GetInt(3);
        oData.courierItemID = row.GetInt(4);
        oData.dateAccepted = row.GetInt64(5);
        oData.dateCompleted = row.GetInt64(6);
        oData.dateIssued = row.GetInt64(7);
        oData.destinationID = row.GetInt(8);
        oData.expiryTime = row.GetInt64(9);
        oData.important = row.GetInt(10);
        oData.storyline = row.GetInt(11);
        oData.missionID = row.GetInt(12);
        oData.contentID = row.GetInt(13);
        oData.name = row.GetInt(14);
        oData.offerID = row.GetInt(15);
        oData.originID = row.GetInt(16);
        oData.remoteCompletable = row.GetInt(17);
        oData.remoteOfferable = row.GetInt(18);
        oData.rewardISK = row.GetInt(19);
        oData.rewardItemID = row.GetInt(20);
        oData.rewardItemQty = row.GetInt(21);
        oData.rewardLP = row.GetInt(22);
        oData.bonusISK = row.GetInt(23);
        oData.bonusTime = row.GetInt(24);
        oData.stateID = row.GetInt(25);
        oData.typeID = row.GetInt(26);
        m_xoffers.emplace(row.GetInt(2), oData);
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


void MissionDataMgr::UpdateMissionData()
{
    double start = GetTimeMSeconds();

    DBerror err;
    int16 count = 0;
/*
    std::map<std::string, uint32>::iterator itr = m_names.begin(), end = m_names.end();
    for (; itr != end; ++itr) {
        if (sDatabase.RunQuery(err,"UPDATE qstMining SET itemQty = %u WHERE name LIKE '%s'", itr->second, itr->first.c_str() ))
            ++count;
        else
            sLog.Error("   MissionDataMgr", "%s not found", itr->first.c_str());
    }
*/
    sLog.Cyan("   MissionDataMgr", "UpdateMissionData - %u missions udpated in %.3f ms.", count, (GetTimeMSeconds() - start));
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
    data.stateID            = Mission::State::Allocated;
    data.dateIssued         = GetFileTimeNow();
    data.remoteOfferable    = false;
    data.remoteCompletable  = false;
    data.offerID            = 0;
    data.agentID            = 0;
    data.rewardLP           = 0;
    data.originID           = 0;
    data.acceptFee          = 0;
    data.expiryTime         = 0;
    data.characterID        = 0;
    data.dateAccepted       = 0;
    data.dateCompleted      = 0;
    data.destinationID      = 0;
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

