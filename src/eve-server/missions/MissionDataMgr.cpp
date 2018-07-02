
 /**
  * @name MissionDataMgr.cpp
  *   memory object caching system for managing and saving ingame data specific to missions
  *
  * @Author:        Allan
  * @date:      24 June 2018
  *
  */


#include "missions/MissionDataMgr.h"
#include "database/EVEDBUtils.h"

MissionDataMgr::MissionDataMgr()
{
    m_names.clear();
    m_offers.clear();
    m_courier.clear();
}

MissionDataMgr::~MissionDataMgr()
{

}

void MissionDataMgr::Clear()
{
    m_names.clear();
    m_offers.clear();
    m_courier.clear();
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
        //SELECT id, descID, name, level, typeID, important, storyline, itemTypeID, itemQty, rewardISK, rewardItemID, rewardItemQty, bonusISK, bonusTime FROM qstCourier
        CourierData cData;
        cData.id            = row.GetInt(0);
        cData.name          = row.GetText(1);
        cData.descID        = row.GetInt(2);
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
    MissionDB::LoadMissionOffers(*res);
    while (res->GetRow(row)) {
        //SELECT offerID, agentID, characterID, missionID, stateID, expiryTime, rewardLP, rewardISK, rewardItemID, rewardItemAmount,
        //  originID, destinationID, acceptFee, courierItemID, courierAmount, dateIssued FROM agtOffers

        MissionOffer oData;

        m_offers.emplace(row.GetInt(3), oData);
    }
    sLog.Cyan("   MissionDataMgr", "%u Outstanding Mission Offers loaded in %.3fms.", m_offers.size(), (GetTimeMSeconds() - start));

    // cleanup
    SafeDelete(res);
}


void MissionDataMgr::GetMissionNameIDs()
{
    double start = GetTimeMSeconds();

    DBerror err;
    int16 count = 0;
/*
    std::map<std::string, uint32>::iterator itr = m_names.begin(), end = m_names.end();
    for (; itr != end; ++itr) {
        if (sDatabase.RunQuery(err,"UPDATE agtMissions SET typeID=3, storyline = %u WHERE name LIKE '%s'", itr->second, itr->first.c_str() ))
            ++count;
        else
            sLog.Error("   MissionDataMgr", "%s not found", itr->first.c_str());
    }
*/
    sLog.Cyan("   MissionDataMgr", "%u GetMissionNameIDs udpated in %.3f ms.", count, (GetTimeMSeconds() - start));
}


