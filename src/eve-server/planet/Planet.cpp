
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
#include "pos/Structure.h"
#include "system/SystemManager.h"


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

    /*  quality: (min=1.0, max=154.275)  */
    double sysSec = (1.1 - m_system->GetSystemSecurityRating());
    m_data.dist_1 = MakeRandomInt(2, 75) * sysSec + MakeRandomFloat(0, 1);
    m_data.dist_2 = MakeRandomInt(2, 75) * sysSec + MakeRandomFloat(0, 1);
    m_data.dist_3 = MakeRandomInt(2, 75) * sysSec + MakeRandomFloat(0, 1);
    m_data.dist_4 = MakeRandomInt(2, 75) * sysSec + MakeRandomFloat(0, 1);
    m_data.dist_5 = MakeRandomInt(2, 75) * sysSec + MakeRandomFloat(0, 1);

    for (uint16 i=0; i<3600; i++) {
        m_data.buffer_1 += hexList[MakeRandomInt(0,15)];   // random fill buffer to capacity, 3k6 bytes.
    }
    for (uint16 i=0; i<3600; i++) {
        m_data.buffer_2 += hexList[MakeRandomInt(0,15)];   // random fill buffer to capacity, 3k6 bytes.
    }
    for (uint16 i=0; i<3600; i++) {
        m_data.buffer_3 += hexList[MakeRandomInt(0,15)];   // random fill buffer to capacity, 3k6 bytes.
    }
    for (uint16 i=0; i<3600; i++) {
        m_data.buffer_4 += hexList[MakeRandomInt(0,15)];   // random fill buffer to capacity, 3k6 bytes.
    }
    for (uint16 i=0; i<3600; i++) {
        m_data.buffer_5 += hexList[MakeRandomInt(0,15)];   // random fill buffer to capacity, 3k6 bytes.
    }

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
    /** @todo  will need to write this code and make it play nice with everything else.
     * a CO will be a special container as a StructureSE, linked to the planet it orbits, and any colony on that planet.
     * there is only one CO per planet, but ALL chars with a colony on that planet can access their items on the same CO
     * the CO will load all items when it loads, but will need checks on "view items" or "open container" to ONLY send
     * items owned by calling char, or NONE for chars that dont have a colony on that planet.
     * i dont know where/how to do that yet...will need testing
     */

    //ItemData( uint32 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, uint32 _quantity, const char *_customInfo = "", bool _contraband = false);
    FactionData data;
        data.ownerID = corpInterbus;
        data.factionID = factionInterBus;
        data.allianceID = 0;
        data.corporationID = corpInterbus;
    uint16 typeID = EVEDB::invTypes::typeInterbusCustomsOffice;

    if (m_system->GetSystemSecurityRating() > 0.49) {
        typeID = EVEDB::invTypes::typePlanetaryCustomsOffice;
        // hisec...reset data for system sov holder...not sure how im gonna do this one.
        data.ownerID = 1;
        data.factionID = sDataMgr.GetRegionFaction(m_system->GetRegionID());
        data.allianceID = 0;
        data.corporationID = 0;
    }

    GPoint pos = GetPosition();
    uint32 radius = m_self->radius();
    srandom(m_self->itemID());
    int64 rand = random();
    double j = (((rand / RAND_MAX) -1.0) / 3.0);
    double s = 20 * pow(0.025 * (10 * log10(radius/1000000) -39), 20) +0.5;
    s = EvE::max(0.5, EvE::min(s, 10.5));
    double t = asin((pos.x/fabs(pos.x)) * (pos.z / sqrt(pow(pos.x, 2) + pow(pos.z, 2)))) +j;
    uint32 d = radius * (s +1) +10000;
    pos.x += d * sin(t);
    pos.y += 0.5 * radius * sin(j);
    pos.z -= d * cos(t);
/*
    GVector dir(pos, m_self->position());
    dir.normalize();
    pos -= (dir * 25000);   // put CO 25km closer to planet than warpIn point.
*/
    ItemData idata(typeID, data.ownerID, m_system->GetID(), flagAutoFit, 1, itoa(m_self->itemID()), false);
    StructureItemRef iRef = m_services.item_factory->SpawnStructure(idata);
    iRef->Relocate(pos);
    iRef->ChangeSingleton(true, false);
    iRef->SetAttribute(AttrIsGlobal, 1, false);
    iRef->SaveItem();
    pCO = new StructureSE(iRef, m_services, m_system, data);
    m_system->AddEntity(pCO);
}

PyRep* PlanetSE::GetResourceData(Call_ResourceDataDict& dict)
{
    // will update this to use PI skills (sent in dict) as system grows
    /** @todo  this needs a minor rewrite....bands are dictated by client request.
     * bufferData is random fill based on bands, but kept per planet
     * will have to create a method to fill buffer with random values, rather than fill with single value
     *  the full 30 band data buffer will be created on planet creation for each resource, and the "bands"
     * are the "layers" of the resource, per se, with more layers giving higher degree of accuracy.
     *  the client sends depth request, and that will determine the bands and buffer size to return.
     * the requested bands will have to be taken from the full 30-band data buffer, as needed.
     * this resource data *MAY* change over the course of the running server, but not decided how/when/why yet.
     */
    //Buffer* dataBuffer = new Buffer;
    //uint16 size = (uint16)pow(bands, 2)*4;
    uint8 bufferData = /*62*/63/*64*/;  // this seems to give the best results for static data (need to change/update)
    size_t buffer = (uint16)pow(dict.newBand, 2)*4;
    _log(PLANET__DEBUG, "PlanetSE::GetResourceData() - proximity: %u, newBand: %u, oldBand: %u, data: %u, bufferSize: %u", dict.proximity, dict.newBand, dict.oldBand, bufferData, (uint32)buffer);
    PyDict* args = new PyDict();
        args->SetItemString("data", new PyBuffer(buffer, bufferData));
        args->SetItemString("numBands", new PyInt(dict.newBand));
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