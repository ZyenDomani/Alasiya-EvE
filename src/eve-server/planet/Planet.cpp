
 /**
  * @name Planet.cpp
  *   Specific Class for individual planets.
  * this class will hold all planet data and relative info for each planet.
  *
  * @Author:         Allan
  * @date:   30 April 2016
  */

 /*
  * PLANET__ERROR
  * PLANET__WARNING
  * PLANET__MESSAGE
  * PLANET__DEBUG
  * PLANET__INFO
  * PLANET__TRACE
  * PLANET__DUMP
  * PLANET__RES_DUMP
  * PLANET__GC_DUMP
  * PLANET__PKT_TRACE
  * PLANET__DB_ERROR
  * PLANET__DB_WARNING
  */

#include <iomanip>

#include "Client.h"
#include "EVEServerConfig.h"
#include "math/Trig.h"
#include "planet/Colony.h"
#include "planet/Planet.h"
#include "planet/PlanetDB.h"
#include "planet/PlanetMgr.h"
#include "planet/CustomsOffice.h"
#include "packets/Planet.h"
#include "pos/Structure.h"
#include "system/Celestial.h"
#include "system/SystemManager.h"


Planet::Planet()
{
    /** @todo  will need to make an InventoryItem class specific for planets here eventually */
}


PlanetSE::PlanetSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system)
: StaticSystemEntity(self, services, system),
pCO(nullptr),
m_data(PlanetResourceData())
{
}

PlanetSE::~PlanetSE()
{
    for (auto &cur : m_colonies) {
        cur.second->Shutdown();
        SafeDelete(cur.second);
    }
}

//TODO:  update this to change data on *some yet-unknown* timeframe
bool PlanetSE::LoadExtras() {
    // this is called after SE is created.
    if (!StaticSystemEntity::LoadExtras())
        return false;

    // has this planet been created/populated already?
    if (!PlanetDB::LoadPlanetResourceData(m_self->itemID(), m_data)) {
        std::vector<uint16> typeIDs;
        sPlanetDataMgr.GetPlanetData(m_self->typeID(), typeIDs);
        m_data.type_1 = typeIDs.at(0);
        m_data.type_2 = typeIDs.at(1);
        m_data.type_3 = typeIDs.at(2);
        m_data.type_4 = typeIDs.at(3);
        m_data.type_5 = typeIDs.at(4);

        float sysSec(m_system->GetSecValue()); // 0.1 - 2.0
        float baseScarcityMultiplier = (1.1f - sysSec);
        float baseMin = sysSec * 10.0f;
        float abundanceMod = sPlanetDataMgr.GetAbundanceMod(m_self->typeID());

        // Set overall quantity tracking floats
        m_data.dist_1 = (MakeRandomFloat(baseMin, 75.0f) * sysSec + MakeRandomFloat(0.0f, 4.0f)) * abundanceMod;
        m_data.dist_2 = (MakeRandomFloat(baseMin, 75.0f) * sysSec + MakeRandomFloat(0.0f, 4.0f)) * abundanceMod;
        m_data.dist_3 = (MakeRandomFloat(baseMin, 75.0f) * sysSec + MakeRandomFloat(0.0f, 4.0f)) * abundanceMod;
        m_data.dist_4 = (MakeRandomFloat(baseMin, 75.0f) * sysSec + MakeRandomFloat(0.0f, 4.0f)) * abundanceMod;
        m_data.dist_5 = (MakeRandomFloat(baseMin, 75.0f) * sysSec + MakeRandomFloat(0.0f, 4.0f)) * abundanceMod;

        // Populate every database buffer string with up to 25 layered hotspots
        m_data.buffer_1 = GenerateResourceBuffer(baseScarcityMultiplier, abundanceMod);
        m_data.buffer_2 = GenerateResourceBuffer(baseScarcityMultiplier, abundanceMod);
        m_data.buffer_3 = GenerateResourceBuffer(baseScarcityMultiplier, abundanceMod);
        m_data.buffer_4 = GenerateResourceBuffer(baseScarcityMultiplier, abundanceMod);
        m_data.buffer_5 = GenerateResourceBuffer(baseScarcityMultiplier, abundanceMod);

        PlanetDB::SavePlanetResourceData(m_self->itemID(), m_data);
    }

    // load resource data into local container for faster/easier lookups
    m_typeBuffers.reserve(5);
    m_typeBuffers.emplace(m_data.type_1, m_data.buffer_1);
    m_typeBuffers.emplace(m_data.type_2, m_data.buffer_2);
    m_typeBuffers.emplace(m_data.type_3, m_data.buffer_3);
    m_typeBuffers.emplace(m_data.type_4, m_data.buffer_4);
    m_typeBuffers.emplace(m_data.type_5, m_data.buffer_5);

    return true;
}

// called from SystemManager::Process() @ 1m
void PlanetSE::Process() {
    for (auto &cur : m_colonies)
        cur.second->Process();
}

PyRep* PlanetSE::GetResourceData(Call_ResourceDataDict& dict) {
    // will update this to use PI skills (sent in dict) as system grows..not sure how yet.
    /** @todo  this needs a minor rewrite....bands are dictated by client request.
     * bufferData is random fill based on bands, but kept per planet
     * will have to create a method to fill buffer with random values, rather than fill with single value
     *  the full 30 band data buffer will be created on planet creation for each resource, and the "bands"
     * are the "layers" of the resource, per se, with more layers giving higher degree of accuracy.
     *  the client sends depth request, and that will determine the bands and buffer size to return.
     * the requested bands will have to be taken from the full 30-band data buffer, as needed.
     * this resource data *MAY* change over the course of the running server, but not decided how/when/why yet.
     */

    /*
    dict.resourceTypeID;
    dict.planetology;
    dict.remoteSensing;
    dict.advancedPlanetology;
    dict.newBand;               <- min(maxBand, minBand + info.planetology + info.advancedPlanetology * 2)
    dict.oldBand;               <- is this used?  how?
    dict.updateTime;
    */
    std::unordered_map<uint16, std::string>::iterator itr = m_typeBuffers.find(dict.resourceTypeID);
    if (itr == m_typeBuffers.end())
        return nullptr;

    int size = dict.newBand * dict.newBand * 4;         // 18 band SH (18*18*4 = 1296)
    std::string data = itr->second.substr(0, size);
    // adjust data for system security.  not sure how to make it 'less' yet
    if (is_log_enabled(PLANET__DEBUG)) {
        _log(PLANET__DEBUG, "PlanetSE::GetResourceData() for %s (%u) using remoteSense: %u, planetology: %u, advPlanetology: %u - updateTime: %lu, proximity: %s, newBand: %u, oldBand: %u, bufferSize: %u", \
                sPIDataMgr.GetProductName(dict.resourceTypeID), dict.resourceTypeID, dict.remoteSensing, dict.planetology, dict.advancedPlanetology, \
                dict.updateTime, sPlanetDataMgr.GetProximity(dict.proximity), dict.newBand, dict.oldBand, size);
        _log(PLANET__DEBUG, "PlanetSE::GetResourceData() for %s:  %s", sPIDataMgr.GetProductName(dict.resourceTypeID), data.c_str());
    }
    PyDict* args = new PyDict();
        args->SetItemString("data", new PyString(data));
        args->SetItemString("numBands", new PyInt(dict.newBand));
        args->SetItemString("proximity", new PyInt(dict.proximity));
    //PyIncRef(args);
    PyObject* rtn = new PyObject("util.KeyVal", args);
    if (is_log_enabled(PLANET__RES_DUMP))
        rtn->Dump(PLANET__RES_DUMP, "   ");
    return rtn;
}

PyRep* PlanetSE::GetPlanetResourceInfo() {
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

    if (pColony->HasColony()/* and sConfig.cosmic.PIEnabled*/) {
        //pColony->Update();
        args->SetItem("level", new PyInt(pColony->GetLevel()));
        args->SetItem("pins", pColony->GetPins());
        args->SetItem("links", pColony->GetLinks());
        args->SetItem("routes", pColony->GetRoutes());
        args->SetItem("currentSimTime", new PyLong(pColony->GetSimTime()));
    }

    PyObject *rtn = new PyObject("util.KeyVal", args);
    if (is_log_enabled(PLANET__RES_DUMP))
        rtn->Dump(PLANET__RES_DUMP, "   ");
    return rtn;
}

PyRep* PlanetSE::GetExtractorsForPlanet(int32 planetID) {
    // NOTE this gets ALL extractors on this planet
    // returns typeID, ownerID, latitude?, longitude?

    DBQueryResult res;
    // {for ecu in planetID}  SELECT `headID`, `typeID`, `ownerID`, `latitude`, `longitude` FROM `piECUHeads`
    PlanetDB::GetExtractorsForPlanet(planetID, res);

    PyList* list = new PyList();
    DBResultRow row;
    while (res.GetRow(row)) {
        PyDict* dict(new PyDict());
            dict->SetItem("pinID", new PyInt(row.GetInt(0)));
            dict->SetItem("typeID", new PyInt(row.GetInt(1)));
            dict->SetItem("ownerID", new PyInt(row.GetInt(2)));
            dict->SetItem("latitude", new PyFloat(row.GetDouble(3)));
            dict->SetItem("longitude", new PyFloat(row.GetDouble(4)));
        list->AddItem(new PyObject("util.KeyVal", dict));
    }

    if (is_log_enabled(PLANET__RES_DUMP))
        list->Dump(PLANET__RES_DUMP, "   ");
    return list;
}

Colony* PlanetSE::GetColony(Client* pClient)
{
    std::map<uint32, Colony*>::const_iterator itr = m_colonies.find(pClient->GetCharacterID());
    if (itr != m_colonies.end())
        return itr->second;
    Colony* pColony = new Colony(&m_services, pClient, this);
    m_colonies[pClient->GetCharacterID()] = pColony;

    return pColony;
}

void PlanetSE::AbandonColony(Colony* pColony) {
    pColony->AbandonColony();
    m_colonies.erase(pColony->GetOwner());
}

void PlanetSE::CreateCustomsOffice() {
    /** @todo  will need to write this code and make it play nice with everything else.
     * a CO will be a special container as a CustomsSE, linked to the planet it orbits, and any colony on that planet.
     * there is only one CO per planet, but ALL chars with a colony on that planet can access their items on the same CO
     * the CO will load all items when it loads, but will need checks on "view items" or "open container" to ONLY send
     * items owned by calling char, or NONE for chars that dont have a colony on that planet.
     * i dont know where/how to do that yet...will need testing
     * CO can go into reinforced mode, like towers (hint: use same code)
     */

    //ItemData( uint32 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, uint32 _quantity, const char *_customInfo = "", bool _contraband = false);
    uint16 typeID(EVEDB::invTypes::InterbusCustomsOffice);
    FactionData data = FactionData();
        data.ownerID = corpInterbus;
        data.factionID = 0; //factionInterBus;
        data.allianceID = 0;
        data.corporationID = corpInterbus;

    if (m_system->GetSecurityRating() > 0.49) {
        typeID = EVEDB::invTypes::PlanetaryCustomsOffice;
        data.ownerID = corpCONCORD;
        data.factionID = 0; //sDataMgr.GetRegionFaction(m_system->GetRegionID());
        data.allianceID = 0;
        data.corporationID = corpCONCORD;
    }

    ItemData idata(typeID, data.ownerID, m_system->GetID(), flagAutoFit, 1, itoa(m_self->itemID()), false);
    StructureItemRef iRef = sItemFactory.SpawnStructure(idata);
    // get warpInPoint for planet
    int32 radius = GetRadius();
    Vector3d warpInPoint = GetPosition();
    srandom(GetID());  //this is the only place random() is used....other random functions use rand() as it's non-repeatable.
    int rand = random();
    double j = (((rand / RAND_MAX) - 1.0f) / 3.0f);
    double s = 20 * std::pow(0.025f * (10 * std::log10(radius / 1000000) - 39), 20) + 0.5f;
    s = EvE::max(0.5f, EvE::min(s, 10.5f));
    double t = std::asin((warpInPoint.x / std::fabs(warpInPoint.x)) * (warpInPoint.z / std::sqrt(std::pow(warpInPoint.x, 2) + std::pow(warpInPoint.z, 2)))) + j;
    uint32 d = radius * (s + 1) + 1000000;
    warpInPoint.x += (d * EvE::Trig::FastSin(t));
    warpInPoint.y += (0.5f * radius * EvE::Trig::FastSin(j));
    warpInPoint.z -= (d * EvE::Trig::FastCos(t));

    // set new position in middle of grid
    int64 bubbleDia = (BUBBLE_RADIUS_METERS * 2);
    int64 xGrid = (floor(warpInPoint.x / bubbleDia));
    int64 yGrid = (floor(warpInPoint.y / bubbleDia));
    int64 zGrid = (floor(warpInPoint.z / bubbleDia));
    warpInPoint.x = (xGrid * bubbleDia + BUBBLE_RADIUS_METERS);
    warpInPoint.y = (yGrid * bubbleDia + BUBBLE_RADIUS_METERS);
    warpInPoint.z = (zGrid * bubbleDia + BUBBLE_RADIUS_METERS);

    iRef->Move(GetLocationID(), flagAutoFit, true);
    iRef->ChangeSingleton(true, false);
    iRef->SetPosition(warpInPoint);
    iRef->SaveItem();
    pCO = new CustomsSE(iRef, GetServices(), m_system, data);
    pCO->Init();
    m_system->AddEntity(pCO);
}


// Procedural array builder generating up to 25 layered SH hotspots per material type
std::string PlanetSE::GenerateResourceBuffer(float baseScarcityMultiplier, float abundanceMod) {
    // 225 continuous floats storing our 25 discrete nodes
    std::vector<float> resourceFloatArray(225, 0.0f);

    // Determine how many active hot spots this planet has (e.g., 8 to 15 nodes for rich distribution)
    int activeHotspots = MakeRandomInt(8, 15);

    for (int nodeIdx = 0; nodeIdx < activeHotspots; ++nodeIdx) {
        // Find a distinct random coordinate direction vector over the spherical planet surface
        float theta = MakeRandomFloat(0.0f, 2.0f * M_PI);
        float phi = acos(MakeRandomFloat(-1.0f, 1.0f));

        // Project onto Cartesian space matching EVE Online's Y-Up transformation matrix rules
        float x = sin(phi) * cos(theta);
        float y = cos(phi);
        float z = sin(phi) * sin(theta);

        // Balance intensity using security status and localized type abundance
        float intensity = MakeRandomFloat(10.0f, 95.0f) * baseScarcityMultiplier * abundanceMod;

        // Calculate the starting array pointer offset for this specific node block
        size_t offset = nodeIdx * 9;

        // Generate and assign the unique 9 basis functions for this hotspot position
        resourceFloatArray[offset + 0] = intensity * 0.2820948f;               // l=0, m=0
        resourceFloatArray[offset + 1] = intensity * 0.4886025f * y;           // l=1, m=-1
        resourceFloatArray[offset + 2] = intensity * 0.4886025f * z;           // l=1, m=0
        resourceFloatArray[offset + 3] = intensity * 0.4886025f * x;           // l=1, m=1
        resourceFloatArray[offset + 4] = intensity * 1.0925484f * x * y;       // l=2, m=-2
        resourceFloatArray[offset + 5] = intensity * 1.0925484f * y * z;       // l=2, m=-1
        resourceFloatArray[offset + 6] = intensity * 0.3153916f * (3.0f * z * z - 1.0f); // l=2, m=0
        resourceFloatArray[offset + 7] = intensity * 1.0925484f * x * z;       // l=2, m=1
        resourceFloatArray[offset + 8] = intensity * 0.5462742f * (x * x - y * y);       // l=2, m=2
    }

    // Remaining node blocks default to 0.0f, acting as clean padding
    return sPlanetDataMgr.EncodeMultiNodeHexBuffer(resourceFloatArray);
}
