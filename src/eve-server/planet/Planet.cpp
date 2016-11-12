
 /**
  * @name Planet.cpp
  *   Specific Class for individual planets.
  * this class will hold all planet data and relative info for each planet.
  *
  * @Author:         Allan
  * @date:   30 April 2016
  */


#include "Colony.h"
#include "Planet.h"
#include "PlanetDB.h"
#include "packets/Planet.h"
#include "system/SystemManager.h"


/** @note  general design notes
 * planetse will have a Planet class to hold data and call other functions/methods as needed
 * the PlanetMgr class will manage all aspects of planet data, init'd as a single instance (no reason for multiples)
 *
 *
 */

PlanetDataMgr::PlanetDataMgr()
{
}

int PlanetDataMgr::Initialize()
{
    _Populate();
    return 1;
}

void PlanetDataMgr::_Populate()
{
    double start = GetTimeUSeconds();
    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;

    m_db->GetPlanetData(*res);
    while (res->GetRow(row)) {
        // SELECT planet.typeID, resource.typeID
        m_planetData.insert(std::pair<uint32, uint32>(row.GetInt(0), row.GetInt(1)));
    }

    //cleanup
    SafeDelete(res);
    sLog.Log("     PlanetDataMgr", "%u planet data groups in %u buckets loaded in %.3fms.",
             m_planetData.size(), m_planetData.bucket_count(), (GetTimeUSeconds() - start));
}

void PlanetDataMgr::GetPlanetData(uint32 planetID, std::vector<uint32> &typeIDs)
{
    auto itr = m_planetData.equal_range(planetID);
    for (auto it = itr.first; it != itr.second; it++)
        typeIDs.push_back(it->second);
}



Planet::Planet()
{

}


PlanetSE::PlanetSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system)
: StaticSystemEntity(self, services, system)
{
}

PlanetSE::~PlanetSE()
{
}


bool PlanetSE::LoadExtras(SystemDB* db) {
    if (!StaticSystemEntity::LoadExtras(db))
        return false;
    /** @todo use this to initialize planet data, create planet manager, or whatever else i decide is needed for planet management
     *  this is called when SE is created.
     */
    std::vector<uint32> typeIDs;
    sPlanetDataMgr.GetPlanetData(m_self->typeID(), typeIDs);
    m_data.type_1 = typeIDs.at(0);
    m_data.type_2 = typeIDs.at(1);
    m_data.type_3 = typeIDs.at(2);
    m_data.type_4 = typeIDs.at(3);
    m_data.type_5 = typeIDs.at(4);
/*
    // no clue wtf these are or how they are populated
    m_data.data_1 = "~";
    m_data.data_2 = "~";
    m_data.data_3 = "~";
    m_data.data_4 = "~";
    m_data.data_5 = "~";
*/
    /** @todo  make these more realistic, based on system trusec and planet size
     *   quality: (min=1.0, max=154.275)
     */
    m_data.dist_1 = MakeRandomInt(1, 150);
    m_data.dist_2 = MakeRandomInt(1, 150);
    m_data.dist_3 = MakeRandomInt(1, 150);
    m_data.dist_4 = MakeRandomInt(1, 150);
    m_data.dist_5 = MakeRandomInt(1, 150);
    // unknown interger here.  seen "0" and "15"
    m_data.numBands_1 = MakeRandomInt(1, 15);
    m_data.numBands_2 = MakeRandomInt(1, 15);
    m_data.numBands_3 = MakeRandomInt(1, 15);
    m_data.numBands_4 = MakeRandomInt(1, 15);
    m_data.numBands_5 = MakeRandomInt(1, 15);

    return true;
}

PyRep* PlanetSE::GetResourceData(Call_ResourceDataDict& dict)
{
    // will update this to use PI skills (sent in dict) as system grows
    int numBands = 0;

    if (dict.resourceTypeID == m_data.type_1)
        numBands = m_data.numBands_1;
    else if (dict.resourceTypeID == m_data.type_2)
        numBands = m_data.numBands_2;
    else if (dict.resourceTypeID == m_data.type_3)
        numBands = m_data.numBands_3;
    else if (dict.resourceTypeID == m_data.type_4)
        numBands = m_data.numBands_4;
    else if (dict.resourceTypeID == m_data.type_5)
        numBands = m_data.numBands_5;

    const char bufferData = *m_data.data;
    PyDict* args = new PyDict();
    args->SetItemString("data", new PyBuffer(numBands*numBands*4, bufferData));
        args->SetItemString("numBands", new PyInt(numBands));
        args->SetItemString("proximity", new PyInt(dict.proximity));
    PyObject* rtn = new PyObject("util.KeyVal", args);

    _log(PLANET__DEBUG, "GetResourceData() Dump  - proximity: %u, bands: %u", dict.proximity, numBands);
    PyIncRef(rtn);
    //rtn->Dump(PLANET__RES_DUMP, "   ");
    return rtn;
}

PyRep* PlanetSE::GetPlanetResourceInfo()
{
    PyDict* res = new PyDict();
        res->SetItem(new PyInt(m_data.type_1), new PyFloat(m_data.dist_1));
        res->SetItem(new PyInt(m_data.type_2), new PyFloat(m_data.dist_2));
        res->SetItem(new PyInt(m_data.type_3), new PyFloat(m_data.dist_3));
        res->SetItem(new PyInt(m_data.type_4), new PyFloat(m_data.dist_4));
        res->SetItem(new PyInt(m_data.type_5), new PyFloat(m_data.dist_5));
    _log(PLANET__MESSAGE, "PlanetSE::GetPlanetResourceInfo()  Dump");
    res->Dump(PLANET__RES_DUMP, "   ");
    return res;
}

PyRep* PlanetSE::GetPlanetInfo(Colony* pColony) {
    /*
          [PyObjectData Name: util.KeyVal]
            [PyDict 4 kvp]
              [PyString "planetTypeID"]
              [PyInt 2016]
              [PyString "solarSystemID"]
              [PyInt 30001984]
              [PyString "radius"]
              [PyFloat 2170000]
              [PyString "planetID"]
              [PyInt 40126699]
     *
     *              'currentSimTime' and 'pins'  are populated for planets that are colonlized
            'pins' = GetColony();
            */
    PyDict *args = new PyDict();
    args->SetItem("planetTypeID", new PyInt(m_self->typeID()));
    args->SetItem("solarSystemID", new PyInt(m_system->GetID()));
    args->SetItem("radius", new PyInt(m_self->radius()));
    args->SetItem("planetID", new PyInt(m_self->itemID()));
    if (pColony->GetSimTime()) {
        args->SetItem("pins", pColony->GetColony());
        args->SetItem("currentSimTime", new PyULong(pColony->GetSimTime()));
    }
    PyObject *rtn = new PyObject("util.KeyVal", args);
}

PyRep* PlanetSE::GetExtractorsForPlanet(int32 planetID) {
    // NOTE this gets ALL extractors on this planet
    // returns typeID, ownerID
    DBQueryResult res;
    if(!sDatabase.RunQuery(res, "SELECT 2130 AS typeID, 0 as ownerID")) {
        _log(DATABASE__ERROR, "Error in GetExtractorsForPlanet Query: %s", res.error.c_str());
        return NULL;
    }

    // does this return a dict?
    return DBResultToRowset(res);
}


    /* these are for PI */
/*
    AttrHarvesterType = 709,
    AttrHarvesterQuality = 710,
    AttrLogisticalCapacity = 1631,
    AttrPlanetRestriction = 1632,
    AttrPowerLoadPerKm = 1633,
    AttrCPULoadPerKm = 1634,
    AttrCPULoadLevelModifier = 1635,
    AttrPowerLoadLevelModifier = 1636,
    AttrImportTax = 1638,
    AttrExportTax = 1639,
    AttrImportTaxMultiplier = 1640,
    AttrExportTaxMultiplier = 1641,
    AttrPinExtractionQuantity = 1642,
    AttrPinCycleTime = 1643,
    AttrExtractorDepletionRange = 1644,
    AttrExtractorDepletionRate = 1645,
    AttrSpecialCommandCenterHoldCapacity = 1646,
    */

/*
piLaunchOrbitDecayTime = DAY * 5
piCargoInOrbit = 0
piCargoDeployed = 1
piCargoClaimed = 2
piCargoDeleted = 3
*/

/*
planetResourceScanDistance = 1000000000
planetResourceProximityDistant = 0
planetResourceProximityRegion = 1
planetResourceProximityConstellation = 2
planetResourceProximitySystem = 3
planetResourceProximityPlanet = 4
planetResourceProximityLimits = [(2, 6),
 (4, 10),
 (6, 15),
 (10, 20),
 (15, 30)]
planetResourceScanningRanges = [9.0,
 7.0,
 5.0,
 3.0,
 1.0]
planetResourceUpdateTime = 1 * HOUR
planetResourceMaxValue = 1.21
*/