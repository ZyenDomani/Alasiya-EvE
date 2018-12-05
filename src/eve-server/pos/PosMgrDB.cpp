
/**
 * @name PosMgrDB.cpp
 *   DataBase methods for all POS items
 *
 * @Author:         Allan
 * @date:   8 December 17
 */

#include "eve-server.h"

#include "pos/PosMgrDB.h"
#include "pos/Structure.h"
#include "../DataClasses.h"


PyRep* PosMgrDB::GetSiloCapacityForType(uint16 typeID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
            "SELECT attributeID, valueFloat FROM dgmTypeAttributes"
            " WHERE typeID = %u AND attributeID = %u", typeID, AttrCapacity))
    {
        codelog(DATABASE__ERROR, "Error in GetSiloCapacityForType query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

PyRep* PosMgrDB::GetCorpControlTowers(uint32 corpID)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
            "SELECT typeID, itemID, locationID"
            " FROM entity"
            " LEFT JOIN invTypes USING (typeID)"    // do we need ONLY anchored+ towers.
            " WHERE ownerID = %u AND groupID = %u", corpID, EVEDB::invGroups::Control_Tower))
    {
        codelog(DATABASE__ERROR, "Error in GetCorpControlTowers query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

void PosMgrDB::GetControlTowerFuelRequirements(DBQueryResult& res) {

    if (!sDatabase.RunQuery(res,
            "SELECT controlTowerTypeID, resourceTypeID, purpose, quantity, minSecurityLevel, factionID, wormholeClassID"
            " FROM invControlTowerResources"
            " WHERE purpose = 1 OR purpose = 4"))
    {
        codelog(DATABASE__ERROR, "Error in GetControlTowerFuelRequirements query: %s", res.error.c_str());
    }
}

void PosMgrDB::GetLinkableJumpArrays(uint32 corpID, DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
            "SELECT systemID, itemID"
            " FROM posJumpBridgeData WHERE corpID = %u AND toItemID = 0", corpID))
    {
        codelog(DATABASE__ERROR, "Error in GetLinkableJumpArrays query: %s", res.error.c_str());
    }
}

void PosMgrDB::GetCorpJumpArrays(uint32 corpID, DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
            "SELECT itemID, systemID, toItemID, toTypeID, toSystemID"
            " FROM posJumpBridgeData WHERE corpID = %u", corpID))
    {
        codelog(DATABASE__ERROR, "Error in GetCorpJumpArrays query: %s", res.error.c_str());
    }
}


bool PosMgrDB::GetBaseData(EVEPOS::StructureData& data)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
            "SELECT towerID, moonID, state, status, timestamp, canUse, canView, canTake FROM posStructureData"
            " WHERE itemID = %u", data.itemID))
    {
        codelog(DATABASE__ERROR, "Error in GetBaseData query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row))
        return false;
    data.towerID = row.GetInt(0);
    data.moonID = row.GetInt(1);
    data.state = row.GetInt(2);
    data.status = row.GetFloat(3);
    data.timestamp = row.GetInt64(4);
    data.use = row.GetInt(5);
    data.view = row.GetInt(6);
    data.take = row.GetInt(7);
    return true;
}

void PosMgrDB::SaveBaseData(EVEPOS::StructureData& data)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "INSERT INTO posStructureData "
        "(itemID, towerID, moonID, state, status, timestamp, canUse, canView, canTake)"
        " VALUES ( %i, %i, %i, %u, %f, %lli, %i, %i, %i)",
        data.itemID, data.towerID, data.moonID, data.state, data.status, data.timestamp, data.use, data.view, data.take);
}

void PosMgrDB::UpdateBaseData(EVEPOS::StructureData& data)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "UPDATE posStructureData SET state=%u, status=%f, timestamp=%lli WHERE itemID = %i",
        data.state, data.status, data.timestamp, data.itemID);
}

bool PosMgrDB::GetTowerData(EVEPOS::TowerData& tData, EVEPOS::StructureData& sData)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
            "SELECT "
            " harmonic, standing, standingOwnerID, statusDrop, corpWar, allyStandings, showInCalendar, "
            " sendFuelNotifications, allowCorp, allowAlliance, password, anchor, unanchor, online, offline"
            " FROM posTowerData"
            " WHERE itemID = %u", sData.itemID))
    {
        codelog(DATABASE__ERROR, "Error in GetTowerData query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    if (!res.GetRow(row))
        return false;
    tData.harmonic = row.GetInt(0);
    tData.standing = row.GetFloat(1);
    tData.standingOwnerID = row.GetInt(2);
    tData.statusDrop = row.GetInt(3);
    tData.corpWar = row.GetInt(4);
    tData.allyStandings = row.GetInt(5);
    tData.showInCalendar = row.GetInt(6);
    tData.sendFuelNotifications = row.GetInt(7);
    tData.allowCorp = row.GetInt(8);
    tData.allowAlliance = row.GetInt(9);
    tData.password = row.GetText(10);
    tData.anchor = row.GetInt(11);
    tData.unanchor = row.GetInt(12);
    tData.online = row.GetInt(13);
    tData.offline = row.GetInt(14);
    return true;
}

void PosMgrDB::SaveTowerData(EVEPOS::TowerData& tData, EVEPOS::StructureData& sData)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "INSERT INTO posTowerData "
        "(itemID, harmonic, standing, standingOwnerID, statusDrop, corpWar, allyStandings, showInCalendar, "
        " sendFuelNotifications, allowCorp, allowAlliance, anchor, unanchor, online, offline)"
        " VALUES ( %i, %i, %f, %i, %u, %u, %i, %i, %i, %i, %i, %i, %i, %i, %i)",
        sData.itemID, tData.harmonic, tData.standing, tData.standingOwnerID, (tData.statusDrop ? 1 : 0), (tData.corpWar ? 1 : 0), (tData.allyStandings ? 1 : 0),
        (tData.showInCalendar ? 1 : 0), (tData.sendFuelNotifications ? 1 : 0), (tData.allowCorp ? 1 : 0), (tData.allowAlliance ? 1 : 0),
        tData.anchor, tData.unanchor, tData.online, tData.offline);
}


bool PosMgrDB::GetBridgeData(EVEPOS::JumpBridgeData& data)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT towerID, corpID, allyID, systemID, toItemID, toSystemID, toTypeID, password, allowCorp, allowAlliance"
            " FROM posJumpBridgeData WHERE itemID = %u", data.itemID))
        {
            codelog(DATABASE__ERROR, "Error in GetBridgeData query: %s", res.error.c_str());
            return false;
        }

    DBResultRow row;
    if (!res.GetRow(row))
        return false;

    data.towerID = row.GetInt(0);
    data.corpID = row.GetInt(1);
    data.allyID = row.GetInt(2);
    data.systemID = row.GetInt(3);
    data.toItemID = row.GetInt(4);
    data.toSystemID = row.GetInt(5);
    data.toTypeID = row.GetInt(6);
    data.password = row.GetText(7);
    data.allowCorp = row.GetInt(8);
    data.allowAlliance = row.GetInt(9);

    return true;
}

void PosMgrDB::SaveBridgeData(EVEPOS::JumpBridgeData& data)
{
    std::string escPass;
    sDatabase.DoEscapeString(escPass, data.password);
    DBerror err;
    sDatabase.RunQuery(err,
        "INSERT INTO posJumpBridgeData(itemID, towerID, corpID, allyID, systemID, toItemID, toTypeID, toSystemID, password, allowCorp, allowAlliance)"
        " VALUES (%i,%i,%i,%i,%i,%i,%i,%i,'%s',%i,%i)",
        data.itemID, data.towerID, data.corpID, data.allyID, data.systemID, data.toItemID, data.toTypeID,
        data.toSystemID, escPass.c_str(), data.allowCorp, data.allowAlliance);
}

void PosMgrDB::UpdateBridgeData(EVEPOS::JumpBridgeData& data)
{
    std::string escPass;
    sDatabase.DoEscapeString(escPass, data.password);
    DBerror err;
    sDatabase.RunQuery(err,
        "UPDATE posJumpBridgeData"
        " SET allyID=%i, toItemID=%i, toTypeID=%i, toSystemID=%i, password='%s', allowCorp=%i, allowAlliance=%i"
        " WHERE itemID=%i",
        data.allyID, data.toItemID, data.toTypeID, data.toSystemID, escPass.c_str(), data.allowCorp, data.allowAlliance, data.itemID);
}

bool PosMgrDB::GetReactorData(ReactorData* pData, EVEPOS::StructureData& sData)
{
    return true;
}

void PosMgrDB::SaveReactorData(ReactorData* pData, EVEPOS::StructureData& sData)
{
    /*
    bool active;
    int32 itemID;
    int16 reaction;     // bp typeID?
    std::map<uint32, EVEPOS::POS_Connections> connections;  // itemID, data
    std::map<uint32, EVEPOS::POS_Resource> demands;         // itemID, resourceData(typeID/quantity)
    std::map<uint32, EVEPOS::POS_Resource> supplies;        // itemID, resourceData(typeID/quantity)
*/

}

void PosMgrDB::UpdateReactorData(ReactorData* pData, EVEPOS::StructureData& sData)
{

}


/*
 * CREATE TABLE posJumpBridgeData (
 *  itemID int(10) NOT NULL DEFAULT '0',
 *  corpID int(10) NOT NULL DEFAULT '0',
 *  allyID int(10) NOT NULL DEFAULT '0',
 *  towerID int(10) NOT NULL DEFAULT '0',
 *  systemID int(10) NOT NULL DEFAULT '0',
 *  toItemID int(10) NOT NULL DEFAULT '0',
 *  toTypeID int(10) NOT NULL DEFAULT '0',
 *  toSystemID int(10) NOT NULL DEFAULT '0',
 *  password varchar(50) NOT NULL,
 *  allowCorp tinyint(1) NOT NULL DEFAULT '0',
 *  allowAlliance tinyint(1) NOT NULL DEFAULT '0',
 *  PRIMARY KEY (itemID)
 * ) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='POS Jump Bridge Data';
 *
 *
 * CREATE TABLE posCustomOfficeData (
 *  itemID int(10) NOT NULL DEFAULT '0',
 *  planetID int(10) NOT NULL DEFAULT '0',
 *  orbitalHackerProgress int(10) NOT NULL DEFAULT '0',
 *  orbitalHackerID int(10) NOT NULL DEFAULT '0',
 *  level tinyint(2) NOT NULL DEFAULT '0',
 *  PRIMARY KEY (itemID)
 * ) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='POS Customs Office Data';
 *
 */

void PosMgrDB::UpdateTimeStamp(int32 itemID, EVEPOS::StructureData& data)
{
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE posStructureData SET timestamp=%lli WHERE itemID = %u", data.timestamp, itemID);
}


void PosMgrDB::UpdateAccess(int32 itemID, EVEPOS::TowerData& data)
{
    std::string nameEsc;
    sDatabase.DoEscapeString(nameEsc, data.password);
    DBerror err;
    sDatabase.RunQuery(err,
        "UPDATE posTowerData"
        " SET allowAlliance=%i, allowCorp=%i, password = '%s'"
        " WHERE itemID = %u", data.allowAlliance, data.allowCorp, nameEsc.c_str(), itemID);
}

void PosMgrDB::UpdatePassword(int32 itemID, EVEPOS::TowerData& data)
{
    std::string nameEsc;
    sDatabase.DoEscapeString(nameEsc, data.password);
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE posTowerData SET password = '%s' WHERE itemID = %u", nameEsc.c_str(), itemID);
}

void PosMgrDB::UpdateNotify(int32 itemID, EVEPOS::TowerData& data)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "UPDATE posTowerData"
        " SET showInCalendar=%i, sendFuelNotifications=%i"
        " WHERE itemID = %u", data.showInCalendar, data.sendFuelNotifications, itemID);
}

void PosMgrDB::UpdateSentry(int32 itemID, EVEPOS::TowerData& data)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "UPDATE posTowerData"
        " SET standing=%f, statusDrop=%i, corpWar=%i, allyStandings=%i WHERE itemID = %u",
        data.standing, data.statusDrop, data.corpWar, data.allyStandings, itemID);
}

void PosMgrDB::UpdateHarmonicAndPassword(int32 itemID, EVEPOS::TowerData& data)
{
    std::string nameEsc;
    sDatabase.DoEscapeString(nameEsc, data.password);
    DBerror err;
    sDatabase.RunQuery(err, "UPDATE posTowerData SET harmonic=%i, password = '%s' WHERE itemID = %i", data.harmonic, nameEsc.c_str(), itemID);
}

void PosMgrDB::UpdatePermission(int32 itemID, EVEPOS::TowerData& data)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "UPDATE posTowerData"
        " SET allowCorp=%i, allowAlliance=%i"
        " WHERE itemID = %u", data.allowCorp, data.allowAlliance, itemID);
}

void PosMgrDB::UpdateUsageFlags(int32 itemID, EVEPOS::StructureData& data)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "UPDATE posStructureData"
        " SET canUse=%i, canView=%i, canTake=%i"
        " WHERE itemID = %u", data.use, data.view, data.take, itemID);
}

void PosMgrDB::UpdateDeployFlags(int32 itemID, EVEPOS::TowerData& data)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "UPDATE posTowerData"
        " SET anchor=%f, unanchor=%i, online=%i, offline=%i WHERE itemID = %u",
        data.anchor, data.unanchor, data.online, data.offline, itemID);
}
