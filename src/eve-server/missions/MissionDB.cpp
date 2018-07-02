
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


void MissionDB::GetInfo()
{

}

void MissionDB::GetMissionData(uint16 missionID)
{

}

void MissionDB::LoadMissionData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT id, descID, name, level, typeID, important, storyline, raceID, constellationID, corporationID, dungeonID, rewardISK, rewardItemID, rewardISKQty, rewardItemQty, bonusISK, bonusTime FROM agtMissions"))
        codelog(DATABASE__ERROR, "Error in LoadMissionData query: %s", res.error.c_str());
}

void MissionDB::LoadCourierData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT id, descID, name, level, typeID, important, storyline, itemTypeID, itemQty, rewardISK, rewardItemID, rewardItemQty, bonusISK, bonusTime FROM qstCourier"))
        codelog(DATABASE__ERROR, "Error in LoadCourierData query: %s", res.error.c_str());
}

void MissionDB::CreateOfferID(MissionOffer& data)
{
    DBerror err;
    uint32 uid = 0;
    if (!sDatabase.RunQueryLID(err, uid,
        "INSERT INTO agtOffers(agentID, characterID, missionID, stateID, expiryTime, rewardLP, rewardISK, rewardItemID, rewardItemAmount, originID, destinationID, acceptFee, courierItemID, courierAmount, dateIssued, dateAccepted, dateCompleted) VALUES"
        " VALUES ('%s', %u, %u, %u, %f, %f, %f, %f, %f)",
        /*data.typeID, data.systemID, data.beltID, data.quantity, data.radius, data.x, data.y, data.z*/0 ))
    {
        codelog(DATABASE__ERROR, "Failed to insert new MissionOffer: %s", err.c_str());
        return;
    }

    data.offerID = uid;
}

void MissionDB::LoadMissionOffers(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT offerID, agentID, characterID, missionID, stateID, expiryTime, rewardLP, rewardISK, rewardItemID, rewardItemAmount, originID, destinationID, acceptFee, courierItemID, courierAmount, dateIssued FROM agtOffers WHERE dateCompleted = 0"))
        codelog(DATABASE__ERROR, "Error in LoadMissionOffers query: %s", res.error.c_str());
}


// SELECT offerID, agentID, characterID, missionID, stateID, expiryTime, rewardLP, rewardISK, rewardItemID, rewardItemAmount, originID, destinationID, acceptFee, courierItemID, courierAmount, dateIssued, dateAccepted, dateCompleted FROM agtOffers WHERE
