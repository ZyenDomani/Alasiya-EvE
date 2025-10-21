
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

bool PlanetSE::LoadExtras() {
    // this is called after SE is created.
    if (!StaticSystemEntity::LoadExtras())
        return false;

    if (!PlanetDB::LoadPlanetResourceData(m_self->itemID(), m_data)) {
        std::vector<uint16> typeIDs;
        sPlanetDataMgr.GetPlanetData(m_self->typeID(), typeIDs);
        m_data.type_1 = typeIDs.at(0);
        m_data.type_2 = typeIDs.at(1);
        m_data.type_3 = typeIDs.at(2);
        m_data.type_4 = typeIDs.at(3);
        m_data.type_5 = typeIDs.at(4);

    /*  quality: (min=1.0, max=154.275)  */
    /*
     * [PyDict 5 kvp]
     *  [PyInt 2288]
     *  [PyFloat 100.11311298535]
     *  [PyInt 2073]
     *  [PyFloat 93.8896871755585]
     *  [PyInt 2267]
     *  [PyFloat 88.2820365560074]
     *  [PyInt 2268]
     *  [PyFloat 50.6448014279542]
     *  [PyInt 2270]
     *  [PyFloat 66.0059005883846]
     */

        // these are relative indicators of material quantity.  makes no difference to ecu or extraction amount
        // as system matures, we will begin adjusting these (from extractor data) and saving per planet
        float sysSec(m_system->GetSecValue());    // 0.1 - 2.0
        float min = sysSec * 10; //1.0 - 20.0
        m_data.dist_1 = MakeRandomFloat(min, 75.0f) * sysSec + MakeRandomFloat(0.0f, 4.0f);
        m_data.dist_2 = MakeRandomFloat(min, 75.0f) * sysSec + MakeRandomFloat(0.0f, 4.0f);
        m_data.dist_3 = MakeRandomFloat(min, 75.0f) * sysSec + MakeRandomFloat(0.0f, 4.0f);
        m_data.dist_4 = MakeRandomFloat(min, 75.0f) * sysSec + MakeRandomFloat(0.0f, 4.0f);
        m_data.dist_5 = MakeRandomFloat(min, 75.0f) * sysSec + MakeRandomFloat(0.0f, 4.0f);

        // these work rather well...graphing the results compare similarly to data from live.
        uint16 i(0);
        for (i=0; i<3600; ++i)  //this cannot use numList
            m_data.buffer_1 += hexList[MakeRandomInt(0,15)];   // random fill buffer to capacity, 1k8 bytes.
        for (i=0; i<3600; ++i)
            m_data.buffer_2 += hexList[MakeRandomInt(0,15)];   // random fill buffer to capacity, 1k8 bytes.
        for (i=0; i<3600; ++i)
            m_data.buffer_3 += hexList[MakeRandomInt(0,15)];   // random fill buffer to capacity, 1k8 bytes.
        for (i=0; i<3600; ++i)
            m_data.buffer_4 += hexList[MakeRandomInt(0,15)];   // random fill buffer to capacity, 1k8 bytes.
        for (i=0; i<3600; ++i)
            m_data.buffer_5 += hexList[MakeRandomInt(0,15)];   // random fill buffer to capacity, 1k8 bytes.

        PlanetDB::SavePlanetResourceData(m_self->itemID(), m_data);
    }

    m_typeBuffers[m_data.type_1] = m_data.buffer_1;
    m_typeBuffers[m_data.type_2] = m_data.buffer_2;
    m_typeBuffers[m_data.type_3] = m_data.buffer_3;
    m_typeBuffers[m_data.type_4] = m_data.buffer_4;
    m_typeBuffers[m_data.type_5] = m_data.buffer_5;

    return true;
}

// called from SystemManager::Process() @ 1m
void PlanetSE::Process()
{
    for (auto &cur : m_colonies)
        cur.second->Process();
}

PyRep* PlanetSE::GetResourceData(Call_ResourceDataDict& dict)
{
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
    std::map<uint16, std::string>::iterator itr = m_typeBuffers.find(dict.resourceTypeID);
    if (itr == m_typeBuffers.end())
        return nullptr;

    uint16 size = static_cast<uint16>(pow(dict.newBand, 2) * 4);
    std::string data = itr->second.substr(0, size);
    // adjust data for system security.  not sure how to make it 'less' yet
    if (is_log_enabled(PLANET__DEBUG)) {
        _log(PLANET__DEBUG, "PlanetSE::GetResourceData() for %s (%u) using remoteSense: %u, planetology: %u, advPlanetology: %u - updateTime: %lu, proximity: %s, newBand: %u, oldBand: %u, bufferSize: %u", \
                sPIDataMgr.GetProductName(dict.resourceTypeID), dict.resourceTypeID, dict.remoteSensing, dict.planetology, dict.advancedPlanetology, \
                dict.updateTime, sPlanetDataMgr.GetProximity(dict.proximity), dict.newBand, dict.oldBand, size);
        _log(PLANET__DUMP, "PlanetSE::GetResourceData() for %s:  %s", sPIDataMgr.GetProductName(dict.resourceTypeID), data.c_str());
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
    if (pColony->HasColony()) {
        //pColony->Update();
        args->SetItem("level", new PyInt(pColony->GetLevel()));
        args->SetItem("pins", pColony->GetPins());
        args->SetItem("links", pColony->GetLinks());
        args->SetItem("routes", pColony->GetRoutes());
        args->SetItem("currentSimTime", new PyLong(pColony->GetSimTime()));
    }
    //PyIncRef(args);
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
            dict->SetItem("latitude", new PyFloat(row.GetFloat(3)));
            dict->SetItem("longitude", new PyFloat(row.GetFloat(4)));
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

void PlanetSE::AbandonColony(Colony* pColony)
{
    pColony->AbandonColony();
    m_colonies.erase(pColony->GetOwner());
}

void PlanetSE::CreateCustomsOffice()
{
    /** @todo  will need to write this code and make it play nice with everything else.
     * a CO will be a special container as a CustomsSE, linked to the planet it orbits, and any colony on that planet.
     * there is only one CO per planet, but ALL chars with a colony on that planet can access their items on the same CO
     * the CO will load all items when it loads, but will need checks on "view items" or "open container" to ONLY send
     * items owned by calling char, or NONE for chars that dont have a colony on that planet.
     * i dont know where/how to do that yet...will need testing
     * CO can go into reinforced mode, like towers (hint: use same code)
     */

    //ItemData( uint32 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, uint32 _quantity, const char *_customInfo = "", bool _contraband = false);
    FactionData data = FactionData();
        data.ownerID = corpInterbus;
        data.factionID = factionInterBus;
        data.allianceID = 0;
        data.corporationID = corpInterbus;
    uint16 typeID = EVEDB::invTypes::InterbusCustomsOffice;

    if (m_system->GetSecurityRating() > 0.49) {
        typeID = EVEDB::invTypes::PlanetaryCustomsOffice;
        data.ownerID = corpCONCORD;
        data.factionID = factionCONCORD; //sDataMgr.GetRegionFaction(m_system->GetRegionID());
        data.allianceID = 0;
        data.corporationID = corpCONCORD;
    }

    /*  this puts CO in warp-in bubble
    // calculate warp-in point
    GPoint pos = GetPosition();
    uint32 radius = m_self->radius();
    srandom(m_self->itemID());
    int64 rand = random();
    double j = (((rand / RAND_MAX) - 1.0) / 3.0);
    double s = 20 * pow(0.025 * (10 * log10(radius/1000000) -39), 20) +0.5;
    s = EvE::max(0.5, EvE::min(s, 10.5));
    double t = asin((pos.x/fabs(pos.x)) * (pos.z / sqrt(pow(pos.x, 2) + pow(pos.z, 2)))) +j;
    uint32 d = radius * (s +1) +10000;
    pos.x += d * sin(t);
    pos.y += 0.5 * radius * sin(j);
    pos.z -= d * cos(t);
    // put CO 50km closer to planet than warpIn point.
    GVector dir(pos, m_self->position());
    dir.normalize();
    pos -= (dir * 50000);
    */
    // this puts CO in random 700km orbit around planet
    GPoint pos(GetPosition());
    //pos.MakeRandomPointOnSphere(GetRadius() + 700000);

    //uint32 dist = BUBBLE_RADIUS_METERS + 10000/*m_self->GetAttribute(AttrMoonAnchorDistance).get_long()*/;
    float radius = GetRadius();
    float rad = EvE::Trig::Deg2Rad(25);

    pos.x += radius + 700000.0f * std::sin(rad);
    pos.z += radius + 700000.0f * std::cos(rad);
    pos.y += MakeRandomInt(-1000, 1000);

    ItemData idata(typeID, data.ownerID, m_system->GetID(), flagAutoFit, 1, itoa(m_self->itemID()), false);
    StructureItemRef iRef = sItemFactory.SpawnStructure(idata);
    iRef->SetPosition(pos);
    iRef->ChangeSingleton(true, false);
    iRef->SaveItem();
    pCO = new CustomsSE(iRef, m_services, m_system, data);
    pCO->Init();
    m_system->AddEntity(pCO);
}


void PlanetSE::CreateSHData() {
    // Parameters (edit as needed)
    const int num_samples = 450;
    const double amplitude = 175.0;
    const double frequency = 5.0; // Hz
    const double sampling_rate = 225.0; // samples per second
    const double phase = 0.0;

    // Generate samples (float32)
    std::vector<float> samples(num_samples);
    for (int i = 0; i < num_samples; ++i) {
        double t = static_cast<double>(i) / sampling_rate;
        samples[i] = static_cast<float>(amplitude * std::sin(2.0 * M_PI * frequency * t + phase));
    }
/*
    std::stringstream hex_stream;
    hex_stream << std::hex << std::setfill('0');
    const unsigned char* byte_data = reinterpret_cast<const unsigned char*>(samples);
    size_t total_bytes = samples.size() * sizeof(float);

    for (uint16 i=0; i < total_bytes; ++i)
        hex_stream << std::setw(2) << static_cast<unsigned int>(byte_data[i]);
*/

}
