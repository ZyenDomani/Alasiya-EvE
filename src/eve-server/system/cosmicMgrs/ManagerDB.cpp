/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Allan
*/

#include "eve-server.h"

#include "system/Asteroid.h"
#include "system/cosmicMgrs/ManagerDB.h"

MgrData::MgrData()
{
}

int MgrData::Initialize()
{
    _Populate();
    return 1;
}

void MgrData::_Populate()
{
    double start = GetTimeUSeconds();
    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;

    ManagerDB m_db;
    m_db.GetOreBySSC(*res);
    OreBySSC oreBySSC;
    while (res->GetRow(row)) {
    // SELECT systemSecurityClass, V, S, Py, Pl, O, K, J, Hem, Hed, G, DO, Sp, C, B, A, M FROM mapOreBySystemSecurityClass
        oreBySSC.secClass = row.GetText(0);
        oreBySSC.V = row.GetInt(1);
        oreBySSC.S = row.GetInt(2);
        oreBySSC.Py = row.GetInt(3);
        oreBySSC.Pl = row.GetInt(4);
        oreBySSC.O = row.GetInt(5);
        oreBySSC.K = row.GetInt(6);
        oreBySSC.J = row.GetInt(7);
        oreBySSC.Hem = row.GetInt(8);
        oreBySSC.Hed = row.GetInt(9);
        oreBySSC.G = row.GetInt(10);
        oreBySSC.DO = row.GetInt(11);
        oreBySSC.Sp = row.GetInt(12);
        oreBySSC.C = row.GetInt(13);
        oreBySSC.B = row.GetInt(14);
        oreBySSC.A = row.GetInt(15);
        oreBySSC.M = row.GetInt(16);
        m_oreBySSC.emplace(row.GetText(0), oreBySSC);
    }

    res->Reset();
    m_db.GetRegionFaction(*res);
    while (res->GetRow(row)) {
        //SELECT regionID, factionID FROM mapRegions
        m_regions.insert(std::pair<uint32, uint32>(row.GetInt(0), row.GetInt(1)));
    }

    //cleanup
    SafeDelete(res);

    sLog.Log("          MgrData", "%u ore data sets and %u region factions loaded in %.3fms.", m_oreBySSC.size(), m_regions.size(), (GetTimeUSeconds() - start));
}


uint8 MgrData::GetRegionQuarter(uint32 regionID)
{
    uint32 factionID = 0;
    std::map<uint32, uint32>::iterator itr = m_regions.find(regionID);
    if (itr != m_regions.end())
        factionID = (*itr).second;

    // caldari=1, minmatar=2, amarr=3, gallente=4, none=5
    switch (factionID) {
        case 500001:    //Caldari State
        case 500010:    //Guristas Pirates
            return 1; break;
        case 500002:    //Minmatar Republic
        case 500011:    //Angel Cartel
            return 2; break;
        case 500003:    //Amarr Empire
        case 500007:    //Ammatar Mandate
        case 500008:    //Khanid Kingdom
        case 500012:    //Blood Raider Covenant
        case 500019:    //Sansha's Nation
            return 3; break;
        case 500004:    //Gallente Federation
        case 500020:    //Serpentis
            return 4; break;
        case 500005:    //Jove Empire
        case 500006:    //CONCORD Assembly
        case 500009:    //The Syndicate
        case 500013:    //The InterBus
        case 500014:    //ORE
        case 500015:    //Thukker Tribe
        case 500016:    //Servant Sisters of EVE
        case 500017:    //The Society of Conscious Thought
        case 500018:    //Mordu's Legion Command
            return 5; break;
    }
}

bool MgrData::GetRoidDist(uint8& quarter, const char* sec, std::map< float, uint32 >& roids) {
    DBQueryResult res;

    if (!sDatabase.RunQuery(res,
        " SELECT roidID, percent FROM roidDistributionCmb WHERE systemSec = '%s' ORDER BY roidID", sec))
    {
        codelog(DATABASE__ERROR, "Error in GetRoidDist query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    float tot = 0.0;
    while (res.GetRow(row)) {
        tot += row.GetFloat(1);
        roids[tot] = row.GetUInt(0);
    }

    return !roids.empty();
}


void ManagerDB::GetOreBySSC(DBQueryResult& res)
{
    if (!sDatabase.RunQuery(res,
        "SELECT systemSecurityClass, Veldspar, Scordite, Pyroxeres, Plagioclase, Omber, Kernite, Jaspet, "
        " Hemorphite, Hedbergite, Gneiss, DarkOchre, Spodumain, Crokite, Bistot, Arkanor, Mercoxit"
        " FROM mapOreBySystemSecurityClass")) {
        _log(DATABASE__ERROR, "Error in GetOreBySSC query: %s", res.error.c_str());
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
    GPoint pos(row.GetFloat(0), row.GetFloat(1), row.GetFloat(2));
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
    if (!sDatabase.RunQuery(res, "SELECT typeID FROM invTypes WHERE groupID = %u", groupID)) {
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

    return !into.empty();
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

