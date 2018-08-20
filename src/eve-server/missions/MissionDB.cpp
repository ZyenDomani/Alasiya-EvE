
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


void MissionDB::LoadMissionData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT id, contentID, name, level, typeID, important, storyline, raceID, constellationID, corporationID, dungeonID,"
        " rewardISK, rewardItemID, rewardISK, rewardItemQty, bonusISK, bonusTime FROM agtMissions WHERE contentID > 0"))
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
        " originID, destinationID, acceptFee, courierItemID, courierAmount, dateIssued, important, name, remoteCompletable, remoteOfferable, typeID)"
        " VALUES (%u, %u, %u, %u %f, %u, %u, %u, %u, %u, %u, %f, %u, %u, %f, %i, '%s', %i, %i, %u)",
        data.agentID, data.characterID, data.missionID, data.stateID, data.expiryTime, data.rewardLP, data.rewardISK, data.rewardItemID, data.rewardItemQty,
        data.originID, data.destinationID, data.acceptFee, data.courierItemID, data.courierAmount, data.dateIssued, (data.important?1:0), data.name.c_str(),
        (data.remoteCompletable?1:0), (data.remoteOfferable?1:0), data.typeID))
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


//SELECT acceptFee, agentID, characterID, courierAmount, courierItemID, dateAccepted, dateCompleted, dateIssued, destinationID, expiryTime, important, storyline,
// missionID, contentID, name, offerID, originID, remoteCompletable, remoteOfferable, rewardISK, rewardItemID, rewardItemQty, rewardLP, bonusISK, bonusTime,
// stateID, typeID FROM agtOffers
void MissionDB::LoadOpenOffers(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT acceptFee, agentID, characterID, courierAmount, courierItemID, dateAccepted, dateCompleted, dateIssued, destinationID, expiryTime, important, missionID, name,"
        " offerID, originID, remoteCompletable, remoteOfferable, rewardISK, rewardItemID, rewardItemQty, rewardLP, stateID, typeID FROM agtOffers WHERE dateCompleted = 0"))
        codelog(DATABASE__ERROR, "Error in LoadOpenOffers query: %s", res.error.c_str());
}


void MissionDB::LoadClosedOffers(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT acceptFee, agentID, characterID, courierAmount, courierItemID, dateAccepted, dateCompleted, dateIssued, destinationID, expiryTime, important, missionID, name,"
        " offerID, originID, remoteCompletable, remoteOfferable, rewardISK, rewardItemID, rewardItemQty, rewardLP, stateID, typeID"
        " FROM agtOffers WHERE dateCompleted > 0 OR expiryTime > %f", GetFileTimeNow()))
        codelog(DATABASE__ERROR, "Error in LoadClosedOffers query: %s", res.error.c_str());
}

void MissionDB::LoadMissionBookMark(DBQueryResult& res, std::vector<int32>& bmIDs)
{
    std::string ids = "";
    ListToINString(bmIDs, ids);
    if (!sDatabase.RunQuery(res,
        "SELECT bookmarkID, ownerID, itemID, typeID, memo, created, x, y, z, locationID, note, creatorID, folderID"
        " FROM bookmarks WHERE bookmarkID IN (%s)", ids.c_str()))
    {
        codelog(DATABASE__ERROR, "Error in query: %s", res.error.c_str());
    }
}
