
 /**
  * @name MissionDB.cpp
  *   memory object caching system for managing and saving ingame data specific to missions
  *
  * @Author:        Allan
  * @date:      24 June 2018
  *
  */


#include "missions/MissionDB.h"
#include "database/EVEDBUtils.h"


MissionDB::MissionDB() { }


void MissionDB::LoadMissionData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT id, contentID, name, level, typeID, important, storyline, raceID, constellationID, corporationID, dungeonID,"
        " rewardISK, rewardItemID, rewardISKQty, rewardItemQty, bonusISK, bonusTime FROM agtMissions WHERE contentID > 0"))
        codelog(DATABASE__ERROR, "Error in LoadMissionData query: %s", res.error.c_str());
}

void MissionDB::LoadCourierData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT id, contentID, name, level, typeID, important, storyline, itemTypeID, itemQty, rewardISK, rewardItemID, rewardItemQty, bonusISK, bonusTime FROM qstCourier WHERE contentID > 0"))
        codelog(DATABASE__ERROR, "Error in LoadCourierData query: %s", res.error.c_str());
}

void MissionDB::LoadMiningData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT id, contentID, name, level, typeID, important, storyline, itemTypeID, itemQty, rewardISK, rewardItemID, rewardItemQty, bonusISK, bonusTime FROM qstMining WHERE contentID > 0"))
        codelog(DATABASE__ERROR, "Error in LoadMiningData query: %s", res.error.c_str());
}

void MissionDB::CreateOfferID(MissionOffer& data)
{
    DBerror err;
    uint32 uid = 0;
    if (!sDatabase.RunQueryLID(err, uid,
        "INSERT INTO agtOffers(agentID, characterID, missionID, stateID, expiryTime, rewardLP, rewardISK, rewardItemID, rewardItemQty,"
        " originID, destinationID, acceptFee, courierItemID, courierAmount, dateIssued)"
        " VALUES (%u, %u, %u, %u %f, %u, %u, %u, %u, %u, %u, %f, %u, %u, %f)",
        data.agentID, data.characterID, data.missionID, data.stateID, data.expiryTime, data.rewardLP, data.rewardISK, data.rewardItemID, data.rewardItemQty,
        data.originID, data.destinationID, data.acceptFee, data.courierItemID, data.courierAmount, data.dateIssued))
    {
        codelog(DATABASE__ERROR, "Failed to insert new MissionOffer: %s", err.c_str());
        return;
    }

    data.offerID = uid;
}

void MissionDB::UpdateMissionOffer(MissionOffer& data)
{
    DBerror err;
    if (!sDatabase.RunQuery(err, "UPDATE agtOffers SET stateID = %u, dateAccepted = %f, dateCompleted = %f WHERE offerID = %u",
        data.stateID, data.dateAccepted, data.dateCompleted, data.offerID))
    {
        codelog(DATABASE__ERROR, "Failed to insert new MissionOffer: %s", err.c_str());
        return;
    }
}


void MissionDB::LoadMissionOffers(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT acceptFee, agentID, characterID, courierAmount, courierItemID, dateAccepted, dateCompleted, dateIssued, destinationID, expiryTime, missionID, offerID, originID, "
        " rewardISK, rewardItemID, rewardItemAmount, rewardLP, stateID FROM agtOffers WHERE dateCompleted = 0"))
        codelog(DATABASE__ERROR, "Error in LoadMissionOffers query: %s", res.error.c_str());
}


// SELECT offerID, agentID, characterID, missionID, stateID, expiryTime, rewardLP, rewardISK, rewardItemID, rewardItemAmount, originID, destinationID, acceptFee, courierItemID, courierAmount, dateIssued, dateAccepted, dateCompleted FROM agtOffers WHERE
