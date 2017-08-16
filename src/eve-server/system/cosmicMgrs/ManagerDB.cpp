
 /**
  * @name ManagerDB.cpp
  *   cosmic manager database methods
  * @Author:         Allan
  * @date:   17 April 2016
  */


#include "eve-server.h"

#include "system/Asteroid.h"
#include "system/cosmicMgrs/ManagerDB.h"


void ManagerDB::GetSkillList(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT typeID, typeName FROM invTypes WHERE groupID IN (SELECT groupID FROM invGroups WHERE categoryID = 16)")) {
        codelog(DATABASE__ERROR, "Error in GetSkillList query: %s", res.error.c_str());
    }
}

void ManagerDB::GetTypeAttributes(DBQueryResult& res)
{
    if( !sDatabase.RunQuery(res, "SELECT typeID, attributeID, valueInt, valueFloat FROM dgmTypeAttributes")) {
        codelog(DATABASE__ERROR, "Error in GetTypeAttributes query: %s", res.error.c_str());
    }
}


void ManagerDB::GetBlueprintType(DBQueryResult& res) {
    if(!sDatabase.RunQuery(res,
        "SELECT"
        "  blueprintTypeID,"
        "  parentBlueprintTypeID,"
        "  productTypeID,"
        "  productionTime,"
        "  techLevel,"
        "  researchProductivityTime,"
        "  researchMaterialTime,"
        "  researchCopyTime,"
        "  researchTechTime,"
        "  productivityModifier,"
        "  materialModifier,"
        "  wasteFactor,"
        "  maxProductionLimit, "
        "  chanceOfRE"
        " FROM invBlueprintTypes "))
    {
        codelog(DATABASE__ERROR, "Error in GetBlueprintType query: %s.", res.error.c_str());
    }
}

void ManagerDB::GetRAMMaterials(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT typeID, materialTypeID, quantity FROM invTypeMaterials")) {
        codelog(DATABASE__ERROR, "Error in GetRAMMaterials query: %s", res.error.c_str());
    }
}

void ManagerDB::GetRAMRequirements(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT typeID, activityID, requiredTypeID, quantity, damagePerJob, recycle FROM ramTypeRequirements")) {
        codelog(DATABASE__ERROR, "Error in GetRAMRequirements query: %s", res.error.c_str());
    }
}

void ManagerDB::GetOreBySSC(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT systemSec, roidID, percent FROM roidDistribution")) {
        codelog(DATABASE__ERROR, "Error in GetRoidDist query: %s", res.error.c_str());
    }
}

void ManagerDB::GetSystemData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT solarSystemID, solarSystemName, constellationID, regionID, securityClass, security FROM mapSolarSystems")) {
        codelog(DATABASE__ERROR, "Error in GetSystemInfo query: %s", res.error.c_str());
    }
}

void ManagerDB::GetStaticData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT itemID, regionID, constellationID, solarSystemID, x, y, z FROM mapDenormalize WHERE solarSystemID IS NOT NULL")) {
        codelog(DATABASE__ERROR, "Error in GetStaticInfo query: %s", res.error.c_str());
    }
}

void ManagerDB::GetStationInfo(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT s.stationID, s.x, s.y, s.z, st.dockEntryX, st.dockEntryY, st.dockEntryZ, st.dockOrientationX, st.dockOrientationY, st.dockOrientationZ FROM staStations AS s"
        " LEFT JOIN staStationTypes AS st USING (stationTypeID)")) {
        codelog(DATABASE__ERROR, "Error in GetStationInfo query: %s", res.error.c_str());
    }
}

void ManagerDB::GetStationSystem(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT stationID, solarSystemID FROM staStations")) {
        codelog(DATABASE__ERROR, "Error in GetStationSystem query: %s", res.error.c_str());
    }
}

void ManagerDB::GetStationRegion(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT stationID, regionID FROM staStations")) {
        codelog(DATABASE__ERROR, "Error in GetStationRegion query: %s", res.error.c_str());
    }
}

void ManagerDB::GetMoonResouces(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT typeID,volume FROM invTypes WHERE groupID = %u", EVEDB::invGroups::Moon_Materials)) {
        codelog(DATABASE__ERROR, "Error in GetMoonResouces query: %s", res.error.c_str());
    }
}

void ManagerDB::SaveAnomaly(CosmicSignature& sig)
{// sysSignatures (sigID,sigItemID,dungeonName,systemID,typeID,groupID,scanGroupID,strengthAttributeID,x,y,z)
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "INSERT INTO sysSignatures"
        " (sigID,sigItemID,dungeonName,systemID,typeID,groupID,scanGroupID,strengthAttributeID,x,y,z)"
        " VALUES "
        "('%s', %u, '%s', %u, %u, %u, %u, %u, %f, %f, %f)", \
            sig.sigID.c_str(), sig.sigItemID, sig.dungeonName.c_str(), sig.systemID, sig.typeID, sig.groupID, \
            sig.scanGroupID, sig.strengthAttributeID, sig.x, sig.y, sig.z )) {
        _log(DATABASE__ERROR, "SaveActiveDungeon - unable to save dungeon");
        }
}

void ManagerDB::GetAnomalyList(DBQueryResult& res)
{// sysSignatures (sigID,sigItemID,dungeonName,systemID,typeID,groupID,scanGroupID,strengthAttributeID,x,y,z)
    if(!sDatabase.RunQuery(res,
        "SELECT sigID,sigItemID,dungeonName,systemID,typeID,groupID,scanGroupID,strengthAttributeID,x,y,z"
        " FROM sysSignatures"
        " ORDER BY systemid")) {
        _log(DATABASE__ERROR, "Error in GetAnomalyList query: %s", res.error.c_str());
        }
}

GPoint ManagerDB::GetAnomalyPos(std::string& string)
{
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT x,y,z FROM sysSignatures WHERE sigID = '%s'", string.c_str())) {
        _log(DATABASE__ERROR, "Error in GetAnomalyPos query: %s", res.error.c_str());
    }
    DBResultRow row;
    if (!res.GetRow(row)) {
        _log(DATABASE__ERROR, "Error in GetAnomalyPos query: %s", res.error.c_str());
        return NULL_ORIGIN;
    }

    GPoint pos(row.GetDouble(0), row.GetDouble(1), row.GetDouble(2));
    return pos;
}

void ManagerDB::GetSystemAnomalies(uint32 systemID, DBQueryResult& res)
{// sysSignatures (typeID,scanGroupID,groupID,strengthAttributeID,dungeonName,sigID,x,y,z)
    if(!sDatabase.RunQuery(res,
        "SELECT typeID, scanGroupID, groupID, strengthAttributeID, dungeonName, sigID, x, y, z"
        " FROM sysSignatures"
        " WHERE systemID = %u", systemID)) {
        _log(DATABASE__ERROR, "Error in GetSystemAnomalies query: %s", res.error.c_str());
        }
}

void ManagerDB::GetSystemAnomalies(uint32 systemID, std::vector< CosmicSignature >& sigs)
{// sysSignatures (sigID,sigItemID,dungeonName,systemID,typeID,groupID,scanGroupID,strengthAttributeID,x,y,z)

}

void ManagerDB::GetRegionFaction(DBQueryResult& res) {
    if (!sDatabase.RunQuery(res, "SELECT regionID, factionID FROM mapRegions WHERE factionID != 0")) {
        _log(DATABASE__ERROR, "Error in GetRegionFactionInfo query: %s", res.error.c_str());
    }
}

void ManagerDB::GetRegionRatFaction(DBQueryResult& res) {
    if (!sDatabase.RunQuery(res, "SELECT regionID, ratFactionID FROM mapRegions WHERE ratFactionID != 0")) {
        _log(DATABASE__ERROR, "Error in GetRegionFactionInfo query: %s", res.error.c_str());
    }
}

void ManagerDB::GetFactionGroups(DBQueryResult& res) {
    if (!sDatabase.RunQuery(res, "SELECT shipClass, groupID, factionID FROM roidRatClassGroup")) {
        _log(DATABASE__ERROR, "Error in GetFactionGroups query: %s", res.error.c_str());
    }
}

void ManagerDB::GetSpawnClasses(DBQueryResult& res) {
    if (!sDatabase.RunQuery(res, "SELECT type, sub, f, d, c, bc, bs, h, o, cf, cd, cc, cbc, cbs FROM roidRatSpawnClass")) {
        _log(DATABASE__ERROR, "Error in GetSpawnClasses query: %s", res.error.c_str());
    }
}

void ManagerDB::GetGroupTypeIDs(uint32 groupID, DBQueryResult& res) {
    if (!sDatabase.RunQuery(res, "SELECT typeID FROM invTypes WHERE groupID = %u ORDER BY typeID LIMIT 10", groupID)) {
        _log(DATABASE__ERROR, "Error in GetGroupTypeIDs query: %s", res.error.c_str());
    }
}

void ManagerDB::DeleteSpawnedRats()
{
    DBerror err;
    std::string query = "'beltrat'";
    sDatabase.RunQuery(err, "DELETE FROM entity WHERE customInfo LIKE %s", query.c_str());
}

bool ManagerDB::LoadSystemRoids(uint32 systemID, uint32& beltID, std::vector< AsteroidData >& into)
{
    DBQueryResult res;
    if(!sDatabase.RunQuery(res,
        "SELECT itemID, itemName, typeID, systemID, beltID, quantity, radius, x, y, z"
        " FROM sysAsteroids"
        " WHERE systemID = %u"
        "  AND beltID = %u", systemID, beltID)) {
        _log(DATABASE__ERROR, "Error in LoadSystemRoids query: %s", res.error.c_str());
        return false;
    }

    _log(DATABASE__RESULTS, "LoadSystemRoids returned %u items", res.GetRowCount());
    DBResultRow row;
    AsteroidData entry;
    while(res.GetRow(row)) {
        entry.itemID = row.GetInt(0);
        entry.itemName = row.GetText(1);
        entry.typeID = row.GetInt(2);
        entry.systemID = row.GetInt(3);
        entry.beltID = row.GetInt(4);
        entry.quantity = row.GetDouble(5);
        entry.radius = row.GetDouble(6);
        entry.x = row.GetDouble(7);
        entry.y = row.GetDouble(8);
        entry.z = row.GetDouble(9);
        into.push_back(entry);
    }

    return into.empty();
}

void ManagerDB::SaveRoid(AsteroidData& data)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "INSERT INTO sysAsteroids"
        " (itemID,itemName,typeID,systemID,beltID,quantity,radius,x, y, z)"
        " VALUES "
        "(%u, '%s', %u, %u, %u, %f, %f, %f, %f, %f)",
        data.itemID, data.itemName.c_str(), data.typeID, data.systemID, data.beltID, data.quantity, data.radius, data.x, data.y, data.z))
    {
        _log(DATABASE__ERROR, "SaveSystemRoids - unable to save roids - %s", err.c_str());
    }
}

void ManagerDB::SaveSystemRoids(uint32 systemID, std::vector< AsteroidData >& roids)
{
    std::ostringstream Inserts;
    // start the insert into command.
    Inserts << "INSERT INTO sysAsteroids";
    Inserts << " (itemID,itemName,typeID,systemID,beltID,quantity,radius,x, y, z)";
    bool first = true;
    for (auto cur : roids) {
        if (first) {
            Inserts << " VALUES ";
            first = false;
        } else
            Inserts << ", ";
        // itemID and attributeID keys.
        Inserts << "(" << cur.itemID << ", '" << cur.itemName << "', " << cur.typeID << ", " << systemID << ", " << cur.beltID << ", ";
        Inserts << cur.quantity << ", " << cur.radius << ", " << cur.x << ", " << cur.y << ", " << cur.z << ")";
    }
    // did we get at least 1 insert?
    if (!first) {
        // finish creating the command.
        Inserts << "ON DUPLICATE KEY UPDATE ";
        Inserts << "quantity=VALUES(quantity), ";
        Inserts << "radius=VALUES(radius)";
        // execute the command.
        DBerror err;
        if (!sDatabase.RunQuery(err, Inserts.str().c_str()))
            _log(DATABASE__ERROR, "SaveSystemRoids - unable to save roids - %s", err.c_str());
    }
}


void ManagerDB::GetDunGroupData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT d.dunGroupID, d.itemTypeID, t.typeName, t.groupID, g.categoryID, d.xpos, d.ypos, d.zpos"
        " FROM dunGroupData AS d"
        "  LEFT JOIN invTypes AS t ON d.itemTypeID = t.typeID"
        "  LEFT JOIN invGroups AS g ON g.groupID = t.groupID" )) {
        _log(DATABASE__ERROR, "Error in GetDunGroupData query: %s", res.error.c_str());
    }
}

void ManagerDB::GetDunRoomData(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT dunRoomID, dunGroupID, xpos, ypos, zpos FROM dunRoomData"))
        _log(DATABASE__ERROR, "Error in GetDunRoomData query: %s", res.error.c_str());
}

void ManagerDB::GetDunRoomInfo(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT dunRoomID, dunRoomType, dunRoomCategory, dunRoomSpawnID, dunRoomSpawnType FROM dunRoomInfo"))
        _log(DATABASE__ERROR, "Error in GetDunRoomInfo query: %s", res.error.c_str());
}

void ManagerDB::GetDunSpawnInfo(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res, "SELECT dunRoomSpawnID, dunRoomSpawnType, xpos, ypos, zpos FROM dunRoomSpawnInfo"))
        _log(DATABASE__ERROR, "Error in GetDunSpawnInfo query: %s", res.error.c_str());
}

void ManagerDB::GetDunTemplates(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT dunTemplateID, dunTemplateName, dunRoomID, dunEntryID, dunTypeID, dunSpawnType, dunRooms, dunRoomTypeID, dunRoomCategoryID FROM dunTemplates"))
        _log(DATABASE__ERROR, "Error in GetDunTemplates query: %s", res.error.c_str());
}

bool ManagerDB::GetSavedDungeons(uint32 systemID, std::vector< ActiveDungeon >& into)
{
    DBQueryResult res;

    if(!sDatabase.RunQuery(res,
        "SELECT systemID, state, dunTemplateID, dunExpiryTime, xpos, ypos, zpos"
        " FROM dunActive"   //Active Dungeons
        " WHERE systemID = %u", systemID)) {
        _log(DATABASE__ERROR, "Error in GetSavedDungeons query: %s", res.error.c_str());
        return false;
    }

    _log(DATABASE__RESULTS, "GetSavedDungeons returned %u items", res.GetRowCount());
    DBResultRow row;
    ActiveDungeon entry;
    while(res.GetRow(row)) {
        entry.systemID = row.GetInt(0);
        entry.state = row.GetInt(1);
        entry.dunItemID = 0;
        entry.dunTemplateID = row.GetInt(2);
        entry.dunExpiryTime = row.GetInt64(3);
        entry.x = row.GetInt(4);
        entry.y = row.GetInt(5);
        entry.z = row.GetInt(6);
        into.push_back(entry);
    }

    return !into.empty();
}

void ManagerDB::SaveActiveDungeon(ActiveDungeon& dun)
{
    DBerror err;
    if (!sDatabase.RunQuery(err,
        "INSERT INTO dunActive" //Active Dungeons
        " (systemID, dungeonID, state, dunTemplateID, dunExpiryTime, xpos, ypos, zpos)"
        " VALUES "
        "(%u, %u, %u, %u, %" PRIu64 ", %f, %f, %f)",
        dun.systemID, dun.dunItemID, dun.state, dun.dunTemplateID, dun.dunExpiryTime, dun.x, dun.y, dun.z )) {
        _log(DATABASE__ERROR, "SaveActiveDungeon - unable to save dungeon");
    }
}

void ManagerDB::ClearDungeons()
{
    DBerror err;
    sDatabase.RunQuery(err, "DELETE FROM dunActive WHERE 1");
    sDatabase.RunQuery(err, "DELETE FROM sysSignatures WHERE 1");
}
