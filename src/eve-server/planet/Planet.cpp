
 /**
  * @name Planet.cpp
  *   Specific Class for individual planets.
  * this class will hold all planet data and relative info for each planet.
  *
  * @Author:         Allan
  * @date:   30 April 2016
  */


#include "Client.h"
#include "planet/Colony.h"
#include "planet/Planet.h"
#include "planet/PlanetMgr.h"
#include "packets/Planet.h"
#include "system/SystemManager.h"


/** @note  general design notes
 * planetse will have a Planet class to hold data and call other functions/methods as needed
 * the PlanetMgr class will manage all aspects of planet data, init'd as a single instance (no reason for multiples)
 *
 */

Planet::Planet()
{
    /** @todo  will need to make an InventoryItem class specific for planets here eventually */
}


PlanetSE::PlanetSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system)
: StaticSystemEntity(self, services, system),
m_colonyTimer(100000) //arbitrary default
{
    m_colonyTimer.Disable();
    self->SetAttribute(AttrMass,   self->type().mass());
    self->SetAttribute(AttrRadius, self->type().radius());
    self->SetAttribute(AttrVolume, self->type().volume());
}

PlanetSE::~PlanetSE()
{
    for (auto cur : m_colonies) {
        cur.second->Shutdown();
        SafeDelete(cur.second);
    }
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

    /** @todo  make these more realistic, based on system trusec and planet size
     *   quality: (min=1.0, max=154.275)
     */
    double sysSec = (1.1 - m_system->GetSystemSecurityRating());
    m_data.dist_1 = MakeRandomInt(1, 75) * sysSec + MakeRandomFloat(0, 1);
    m_data.dist_2 = MakeRandomInt(1, 75) * sysSec + MakeRandomFloat(0, 1);
    m_data.dist_3 = MakeRandomInt(1, 75) * sysSec + MakeRandomFloat(0, 1);
    m_data.dist_4 = MakeRandomInt(1, 75) * sysSec + MakeRandomFloat(0, 1);
    m_data.dist_5 = MakeRandomInt(1, 75) * sysSec + MakeRandomFloat(0, 1);
    // this sets the vein "hot spots" on planet, should be 2 - 30?
    m_data.numBands_1 = MakeRandomInt(15, 30);
    m_data.numBands_2 = MakeRandomInt(15, 30);
    m_data.numBands_3 = MakeRandomInt(15, 30);
    m_data.numBands_4 = MakeRandomInt(15, 30);
    m_data.numBands_5 = MakeRandomInt(15, 30);

    return true;
}

void PlanetSE::Process()
{
    if (m_colonyTimer.Check()) {
        if (m_colonies.empty()) {
            m_colonyTimer.Disable();
            return;
        }
        for (auto cur : m_colonies)
            cur.second->Process();
    }
}

void PlanetSE::CreateCustomsOffice()
{
    /** @todo  will need to write this code and make it play nice with everything else.  */
}

PyRep* PlanetSE::GetResourceData(Call_ResourceDataDict& dict)
{
    // will update this to use PI skills (sent in dict) as system grows
    uint8 numBands = 2, bufferData = /*62*/63/*64*/;  // this seems to give the best results
         if (dict.resourceTypeID == m_data.type_1) { numBands = m_data.numBands_1; }
    else if (dict.resourceTypeID == m_data.type_2) { numBands = m_data.numBands_2; }
    else if (dict.resourceTypeID == m_data.type_3) { numBands = m_data.numBands_3; }
    else if (dict.resourceTypeID == m_data.type_4) { numBands = m_data.numBands_4; }
    else if (dict.resourceTypeID == m_data.type_5) { numBands = m_data.numBands_5; }
    else
        _log(PLANET__ERROR, "PlanetSE::GetResourceData() - Resource TypeID %u not found in list.", dict.resourceTypeID);

    size_t buffer = (uint16)pow(numBands, 2)*4;
    _log(PLANET__DEBUG, "PlanetSE::GetResourceData() - proximity: %u, newBand: %u, bands: %u, data: %u, bufferSize: %u", dict.proximity, dict.newBand, numBands, bufferData, (uint32)buffer);
    PyDict* args = new PyDict();
        args->SetItemString("data", new PyBuffer(buffer, bufferData));
        args->SetItemString("numBands", new PyInt(numBands));
        args->SetItemString("proximity", new PyInt(dict.proximity));
    PyIncRef(args);
    PyObject* rtn = new PyObject("util.KeyVal", args);
    if (is_log_enabled(PLANET__RES_DUMP))
        rtn->Dump(PLANET__RES_DUMP, "   ");
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
    if (is_log_enabled(PLANET__RES_DUMP))
        res->Dump(PLANET__RES_DUMP, "   ");
    return res;
}

PyRep* PlanetSE::GetPlanetInfo(Colony* pColony) {
    PyDict *args = new PyDict();
    args->SetItem("planetTypeID", new PyInt(m_self->typeID()));
    args->SetItem("solarSystemID", new PyInt(m_system->GetID()));
    args->SetItem("radius", new PyInt(GetRadius()));
    args->SetItem("planetID", new PyInt(m_self->itemID()));
    if (pColony->HasColony()) {
        pColony->Update();
        args->SetItem("level", new PyInt(pColony->GetLevel()));
        args->SetItem("pins", pColony->GetPins());
        args->SetItem("links", pColony->GetLinks());
        args->SetItem("routes", pColony->GetRoutes());
        args->SetItem("currentSimTime", new PyULong(pColony->GetSimTime()));
    }
    PyIncRef(args);
    PyObject *rtn = new PyObject("util.KeyVal", args);
    if (is_log_enabled(PLANET__RES_DUMP))
        rtn->Dump(PLANET__RES_DUMP, "   ");
    return rtn;
}

PyRep* PlanetSE::GetExtractorsForPlanet(int32 planetID) {
    // NOTE this gets ALL extractors on this planet
    // returns typeID, ownerID, latitude?, longitude?

    DBQueryResult res;
    PlanetDB m_db;
    // {for ecu in planetID}  SELECT `headID`, `typeID`, `ownerID`, `latitude`, `longitude` FROM `chrPlanetECUHeads`
    m_db.GetExtractorsForPlanet(planetID, res);

    PyList* list(new PyList());
    DBResultRow row;
    while (res.GetRow(row)) {
        PyDict* dict(new PyDict());
            dict->SetItem("pinID", new PyInt(row.GetInt(0)));
            dict->SetItem("typeID", new PyInt(row.GetInt(1)));
            dict->SetItem("ownerID", new PyInt(row.GetInt(2)));
            dict->SetItem("latitude", new PyFloat(row.GetFloat(3)));
            dict->SetItem("longitude", new PyFloat(row.GetFloat(4)));
        list->AddItem(dict);
    }

    return list;
}

Colony* PlanetSE::GetColony(Client* pClient)
{
    std::map<uint32, Colony*>::iterator itr = m_colonies.find(pClient->GetCharacterID());
    if (itr != m_colonies.end())
        return itr->second;
    Colony* pColony = new Colony(&m_services, pClient, this);
    m_colonies[pClient->GetCharacterID()] = pColony;
    if (!m_colonyTimer.Enabled()) {
        // start colony timer.  this will process colony data every 30 mins.
        m_colonyTimer.Start(30*60*1000);   //30m
    }
    return pColony;
}

void PlanetSE::AbandonColony(Colony* pColony)
{
    std::map<uint32, Colony*>::iterator itr = m_colonies.find(pColony->GetOwner());
    if (itr != m_colonies.end())
        m_colonies.erase(itr);
    pColony->AbandonColony();
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
        minBand, maxBand = const.planetResourceProximityLimits[info.proximity]
        info.newBand = min(maxBand, minBand + info.planetology + info.advancedPlanetology * 2)
        requiredSkill = 5 - info.proximity
        if info.remoteSensing < requiredSkill:
            info.requiredSkill = requiredSkill


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
MAX_DISPLAY_QUALTY = const.planetResourceMaxValue * 255 * 0.5
qualityRemapped = quality / MAX_DISPLAY_QUALTY
self.resourceList.AddItem(typeID, quality=max(0, min(1.0, qualityRemapped)))

*/