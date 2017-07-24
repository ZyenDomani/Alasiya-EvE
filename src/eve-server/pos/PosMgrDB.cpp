/*
    Author:        Allan
*/

#include "eve-server.h"

#include "pos/PosMgrDB.h"

PyRep* PosMgrDB::GetControlTowerFuelRequirements() {
    DBQueryResult res;
    if (!sDatabase.RunQuery(res,
        " SELECT "
        "   controlTowerTypeID, resourceTypeID, purpose, quantity, factionID, minSecurityLevel"
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
            dict->SetItemString( "factionID",           new PyInt(row.IsNull(4) ? 0 : row.GetInt(4)));
            dict->SetItemString( "minSecurityLevel",    new PyFloat(row.IsNull(5) ? 0 : row.GetFloat(5)));
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