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
        _log(DATABASE__ERROR, "Error in GetControlTowerFuelRequirements query: %s", res.error.c_str());
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
        _log(DATABASE__ERROR, "Error in GetSiloCapacityByItemID query: %s", res.error.c_str());
        return nullptr;
    }

    return DBResultToCRowset(res);
}

bool PosMgrDB::GetPOSData(EVEPOS::SaveData& data)
{
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        "SELECT towerID, planetID, harmonic, standingOwnerID, state, level, standing,"
        " status, timestamp, rotationX, rotationY, rotationZ, statusDrop, corpWar, showInCalendar, sendFuelNotifications"
        " FROM posStructureData"
        " WHERE itemID = %u", data.itemID))
    {
        _log(DATABASE__ERROR, "Error in GetPOSData query: %s", res.error.c_str());
        return false;
    }

    DBResultRow row;
    res.GetRow(row);
    data.timestamp = 0;
    data.harmonic = 0;
    data.state = 0;
    data.towerID = 0;
    data.rotation = NULL_ORIGIN;
    data.planetID = 0;
    data.status = 0.0f;
    data.standing = 0.0f;
    data.standingOwnerID = 0;
    data.corpWar = row.GetBool(0);
    data.statusDrop = row.GetBool(0);
    data.showInCalendar = row.GetBool(0);
    data.sendFuelNotifications = row.GetBool(0);
    return true;
}

void PosMgrDB::SavePOSData(EVEPOS::SaveData& data)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "INSERT INTO posStructureData "
        "(itemID, towerID, planetID, harmonic, standingOwnerID, state, level, standing,"
        " status, timestamp, rotationX, rotationY, rotationZ, statusDrop, corpWar, showInCalendar, sendFuelNotifications)"
        " VALUES ");

}

void PosMgrDB::UpdatePOSData(EVEPOS::SaveData& data)
{
    DBerror err;
    sDatabase.RunQuery(err,
        "UPDATE posStructureData"
        " SET "
        "  towerID=[value-2],"
        "  planetID=[value-3],"
        "  harmonic=[value-4],"
        "  standingOwnerID=[value-5],"
        "  state=[value-6],"
        "  level=[value-7],"
        "  standing=[value-8],"
        "  status=[value-9],"
        "  timestamp=[value-10],"
        "  rotationX=[value-11],"
        "  rotationY=[value-12],"
        "  rotationZ=[value-13],"
        "  statusDrop=[value-14],"
        "  corpWar=[value-15],"
        "  showInCalendar=[value-16],"
        "  sendFuelNotifications=[value-17]"
        " WHERE itemID = %u");

}







