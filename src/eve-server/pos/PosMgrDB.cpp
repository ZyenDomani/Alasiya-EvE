/*
    Author:        Allan
*/

#include "eve-server.h"

#include "pos/PosMgrDB.h"

PyRep* PosMgrDB::GetControlTowerFuelRequirements() {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT "
        "   controlTowerTypeID, resourceTypeID, purpose, quantity, minSecurityLevel, factionID, wormholeClassID"
        " FROM invControlTowerResources "))
    {
        codelog(DATABASE__ERROR, "Error in GetControlTowerFuelRequirements query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
/*
    PyList* list = new PyList();
    DBResultRow row;

    while (res.GetRow(row)) {
        PyDict* dict = new PyDict();
            dict->SetItemString( "controlTowerTypeID",  new PyInt(row.GetInt(0)));
            dict->SetItemString( "resourceTypeID",      new PyInt(row.GetInt(1)));
            dict->SetItemString( "purpose",             new PyInt(row.GetInt(2)));
            dict->SetItemString( "quantity",            new PyInt(row.GetInt(3)));
            dict->SetItemString( "minSecurityLevel",    new PyFloat(row.IsNull(5) ? 0 : row.GetFloat(5)));
            dict->SetItemString( "factionID",           new PyInt(row.IsNull(4) ? 0 : row.GetInt(4)));
        list->AddItem(dict);
    }

    return new PyObject("util.KeyVal", list);
    */
}


PyRep* PosMgrDB::GetSiloCapacityByItemID(uint16 typeID) {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT attributeID, valueFloat"
        " FROM dgmTypeAttributes "
        " WHERE typeID = %u AND attributeID = %u", typeID, AttrCapacity))
    {
        codelog(DATABASE__ERROR, "Error in GetSiloCapacityByItemID query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

bool PosMgrDB::GetPOSData(EVEPOS::SaveData& data)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT towerID, planetID, harmonic, standingOwnerID, state, standing,"
        " status, timestamp, rotationX, rotationY, rotationZ, statusDrop, corpWar, showInCalendar, sendFuelNotifications"
        " FROM posStructureData"
        " WHERE itemID = %u", data.itemID))
    {
        codelog(DATABASE__ERROR, "Error in GetPOSData query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    res.GetRow(row);
    data.towerID = row.GetInt(0);
    data.planetID = row.GetInt(1);
    data.harmonic = row.GetInt(2);
    data.standingOwnerID = row.GetInt(3);
    data.state = row.GetInt(4);
    data.standing = row.GetFloat(5);
    data.status = row.GetFloat(6);
    data.timestamp = row.GetInt64(7);
    data.rotation = GPoint (
                row.GetDouble(8),
                row.GetDouble(9),
                row.GetDouble(10)
                );
    data.statusDrop = row.GetBool(11);
    data.corpWar = row.GetBool(12);
    data.showInCalendar = row.GetBool(13);
    data.sendFuelNotifications = row.GetBool(14);
    return true;
}

void PosMgrDB::SavePOSData(EVEPOS::SaveData& data)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "INSERT INTO posStructureData "
        "(itemID, towerID, planetID, harmonic, standingOwnerID, state, standing,"
        " status, timestamp, rotationX, rotationY, rotationZ, statusDrop, corpWar, showInCalendar, sendFuelNotifications)"
        " VALUES ",
        (data.itemID, data.towerID, data.planetID, data.harmonic, data.standingOwnerID, data.state, data.standing,
        data.status, data.timestamp, data.rotation.x, data.rotation.y, data.rotation.z, data.statusDrop, data.corpWar, data.showInCalendar,
        data.sendFuelNotifications));
}

void PosMgrDB::UpdatePOSData(EVEPOS::SaveData& data)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "UPDATE posStructureData"
        " SET "
        "  towerID=%u,"
        "  planetID=%u,"
        "  harmonic=%i,"
        "  standingOwnerID=%u,"
        "  state=%u,"
        "  standing=%f,"
        "  status=%f,"
        "  timestamp=" PRIu64 ","
        "  rotationX=%f,"
        "  rotationY=%f,"
        "  rotationZ=%f,"
        "  statusDrop=%i,"
        "  corpWar=%i,"
        "  showInCalendar=%i,"
        "  sendFuelNotifications=%i"
        " WHERE itemID = %u", data.towerID, data.planetID, data.harmonic, data.standingOwnerID, data.state, data.standing,
        data.status, data.timestamp, data.rotation.x, data.rotation.y, data.rotation.z, data.statusDrop, data.corpWar, data.showInCalendar,
        data.sendFuelNotifications, data.itemID);
}

void PosMgrDB::UpdatePOSNotify(uint32 towerID, EVEPOS::TowerData& data)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "UPDATE posStructureData"
        " SET "
        "  showInCalendar=%i,"
        "  sendFuelNotifications=%i"
        " WHERE itemID = %u", data.showInCalendar, data.sendFuelNotifications, towerID);
}

void PosMgrDB::UpdatePOSPermission(uint32 towerID, EVEPOS::TowerData& data)
{
    //DBerror err;
    //sDatabase.RunQuery(err, "UPDATE posStructureData");
}

void PosMgrDB::UpdatePOSPassword(uint32 towerID, EVEPOS::TowerData& data)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "UPDATE posStructureData"
        " SET "
        "  password='%s'"
        " WHERE itemID = %u", data.password.c_str(), towerID);
}

void PosMgrDB::UpdatePOSSentry(uint32 towerID, EVEPOS::TowerData& data)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "UPDATE posStructureData"
        " SET "
        "  standingOwnerID=%u,"
        "  standing=%f,"
        "  status=%f,"
        "  statusDrop=%i,"
        "  corpWar=%i"
        " WHERE itemID = %u", data.standingOwnerID, data.standing, data.status, data.statusDrop, data.corpWar, data.showInCalendar,
            data.sendFuelNotifications, towerID);
}

void PosMgrDB::UpdatePOSTimeStamp(uint32 towerID, uint64 timeStamp)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "UPDATE posStructureData"
        " SET "
        "  timestamp=" PRIu64
        " WHERE itemID = %u", timeStamp, towerID);
}

