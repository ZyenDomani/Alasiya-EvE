
 /**
  * @name Planet.cpp
  *   Specific Class for individual planets.
  * this class will hold all planet data and relative info for each planet.
  *
  * @Author:         Allan
  * @date:   30 April 2016
  * @update: 02 August 2026
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
m_abundance(sPlanetDataMgr.GetAbundanceMod(self->typeID())),
m_scarcity(system->GetSecValue()),
m_data(PlanetResourceData())
{
}

PlanetSE::~PlanetSE()
{
    // update depleted resources as needed before close
    if (m_colonies.size() > 0) {
        // assume there are ecus on any colony.

        m_data.buffer_1 = m_typeBuffers[m_data.type_1].current;
        m_data.buffer_2 = m_typeBuffers[m_data.type_2].current;
        m_data.buffer_3 = m_typeBuffers[m_data.type_3].current;
        m_data.buffer_4 = m_typeBuffers[m_data.type_4].current;
        m_data.buffer_5 = m_typeBuffers[m_data.type_5].current;
        m_data.origBuf_1 = m_typeBuffers[m_data.type_1].spawned;
        m_data.origBuf_2 = m_typeBuffers[m_data.type_2].spawned;
        m_data.origBuf_3 = m_typeBuffers[m_data.type_3].spawned;
        m_data.origBuf_4 = m_typeBuffers[m_data.type_4].spawned;
        m_data.origBuf_5 = m_typeBuffers[m_data.type_5].spawned;

        PlanetDB::SavePlanetResourceData(m_self->itemID(), m_data);
    }

    // now we can clear the colony map
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

        /* distribution matrix
         *  1.0 = min:  0.0  max:  0.0
         *  0.7 = min:  7.6  max: 66.7
         *  0.3 = min: 22.6  max: 69.1
         * -0.2 = min: 49.2  max:121.1
         * -1.0 = min:110.0  max:150.9
         */
        float baseMin =  0.0f;
        float baseMax =  0.0f;
        float secDelta = m_system->GetSecValue() - 0.1f;
        if (secDelta > 0.01f) {
            baseMin = (20.0f * secDelta) + (17.5f * (secDelta * secDelta));
            baseMax = 112.0f * pow(secDelta, 0.43f);
        }

        if (baseMin < 0.0f)
            baseMin = 0.0f;
        if (baseMax > 150.0f)
            baseMax = 150.0f;

        // abundance = 0.85 - 1.5
        // Set overall quantity [11.4, 154.275]
        m_data.dist_1 = (MakeRandomFloat(baseMin, baseMax) * m_abundance);
        m_data.dist_2 = (MakeRandomFloat(baseMin, baseMax) * m_abundance); // + MakeRandomFloat(1.0f, 5.0f)) * sysSec;
        m_data.dist_3 = (MakeRandomFloat(baseMin, baseMax) * m_abundance); // + MakeRandomFloat(1.0f, 5.0f)) * sysSec;
        m_data.dist_4 = (MakeRandomFloat(baseMin, baseMax) * m_abundance); // + MakeRandomFloat(1.0f, 5.0f)) * sysSec;
        m_data.dist_5 = (MakeRandomFloat(baseMin, baseMax) * m_abundance); // + MakeRandomFloat(1.0f, 5.0f)) * sysSec;

        // Populate planet buffer string with 18 bands
        sLog.Warning("PlanetLoad", "%s - generating buffer for %s (%u)", \
                m_self->name(), sDataMgr.GetTypeName(m_data.type_1), m_data.type_1);
        m_data.buffer_1 = GenerateResourceBuffer(m_data.dist_1);
        sLog.Warning("PlanetLoad", "%s - generating buffer for %s (%u)", \
                m_self->name(), sDataMgr.GetTypeName(m_data.type_2), m_data.type_2);
        m_data.buffer_2 = GenerateResourceBuffer(m_data.dist_2);
        sLog.Warning("PlanetLoad", "%s - generating buffer for %s (%u)", \
                m_self->name(), sDataMgr.GetTypeName(m_data.type_3), m_data.type_3);
        m_data.buffer_3 = GenerateResourceBuffer(m_data.dist_3);
        sLog.Warning("PlanetLoad", "%s - generating buffer for %s (%u)", \
                m_self->name(), sDataMgr.GetTypeName(m_data.type_4), m_data.type_4);
        m_data.buffer_4 = GenerateResourceBuffer(m_data.dist_4);
        sLog.Warning("PlanetLoad", "%s - generating buffer for %s (%u)", \
                m_self->name(), sDataMgr.GetTypeName(m_data.type_5), m_data.type_5);
        m_data.buffer_5 = GenerateResourceBuffer(m_data.dist_5);

        m_data.origBuf_1 = m_data.buffer_1;
        m_data.origBuf_2 = m_data.buffer_2;
        m_data.origBuf_3 = m_data.buffer_3;
        m_data.origBuf_4 = m_data.buffer_4;
        m_data.origBuf_5 = m_data.buffer_5;

        m_data.replenishTime = GetFileTimeNow();

        PlanetDB::SavePlanetResourceData(m_self->itemID(), m_data);
    }

    // load resource data into local container for faster/easier lookups
    m_typeBuffers.reserve(5);
    m_typeBuffers.emplace(m_data.type_1, PlanetResourceBuffer(m_data.buffer_1, m_data.origBuf_1));
    m_typeBuffers.emplace(m_data.type_2, PlanetResourceBuffer(m_data.buffer_2, m_data.origBuf_2));
    m_typeBuffers.emplace(m_data.type_3, PlanetResourceBuffer(m_data.buffer_3, m_data.origBuf_3));
    m_typeBuffers.emplace(m_data.type_4, PlanetResourceBuffer(m_data.buffer_4, m_data.origBuf_4));
    m_typeBuffers.emplace(m_data.type_5, PlanetResourceBuffer(m_data.buffer_5, m_data.origBuf_5));

    // just in case...
    if (m_data.replenishTime == 0)
        m_data.replenishTime = GetFileTimeNow();

    return true;
}

// called from SystemManager::Process() @ 1m
void PlanetSE::Process() {
    for (auto &cur : m_colonies)
        cur.second->Process();
}

PyRep* PlanetSE::GetResourceData(Call_ResourceDataDict& dict) {
    // will update this to use PI skills (sent in dict) as system grows..not sure how yet.

    /*
    dict.resourceTypeID;
    dict.planetology;
    dict.remoteSensing;
    dict.advancedPlanetology;
    dict.newBand;               <- min(maxBand, minBand + info.planetology + info.advancedPlanetology * 2)
    dict.oldBand;               <- is this used?  how?
    dict.updateTime;
    */
    std::unordered_map<uint16, PlanetResourceBuffer>::iterator itr = m_typeBuffers.find(dict.resourceTypeID);
    if (itr == m_typeBuffers.end())
        return nullptr;

    if (dict.newBand > 18) {
        sLog.Error("Planet::GetResourceData", "Band > 18: %i", dict.newBand);
        return nullptr;
    }

    int size = dict.newBand * dict.newBand * 4;         // 18 band SH (18*18*4 = 1296)
    std::string data = itr->second.current.substr(0, size);
    // adjust data for system security.  not sure how to make it 'less' yet
    if (is_log_enabled(PLANET__DEBUG)) {
        float dist = 0.0f;
        if (dict.resourceTypeID == m_data.type_1) {
            dist = m_data.dist_1;
        } else if (dict.resourceTypeID == m_data.type_2) {
            dist = m_data.dist_2;
        } else if (dict.resourceTypeID == m_data.type_3) {
            dist = m_data.dist_3;
        } else if (dict.resourceTypeID == m_data.type_4) {
            dist = m_data.dist_4;
        } else {
            dist = m_data.dist_5;
        }

        _log(PLANET__DEBUG, "PlanetSE::GetResourceData() for %s (%u) using remoteSense: %u, planetology: %u, advPlanetology: %u - updateTime: %lu, proximity: %s, newBand: %u, oldBand: %u, dist:%0.5f,  bufferSize: %i", \
                sPIDataMgr.GetProductName(dict.resourceTypeID), dict.resourceTypeID, dict.remoteSensing, dict.planetology, dict.advancedPlanetology, \
                dict.updateTime, sPlanetDataMgr.GetProximity(dict.proximity), dict.newBand, dict.oldBand, dist, size);

        const float* floatArray = reinterpret_cast<const float*>(data.data());
        std::cout << "\n=== SH Dump ===" << std::endl;
        for (int i = 0; i < 18; ++i)
            std::cout << "Index [" << i << "]: " << floatArray[i] << std::endl;
    }

    PyDict* args = new PyDict();
        args->SetItemString("data", new PyString(data));
        args->SetItemString("numBands", new PyInt(dict.newBand));
        args->SetItemString("proximity", new PyInt(dict.proximity));
    PyObject* res = new PyObject("util.KeyVal", args);
    if (is_log_enabled(PLANET__RES_DUMP))
        res->Dump(PLANET__RES_DUMP, "   ");
    return res;
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
        args->SetItem("pins", pColony->GetPins(false));
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
    //SELECT pinID, typeID, ownerID, latitude, longitude FROM piPins WHERE isECU = 1
    DBQueryResult res;
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
    double s = 20 * pow(0.025f * (10 * log10(radius / 1000000) - 39), 20) + 0.5f;
    s = EvE::max(0.5f, EvE::min(s, 10.5f));
    double t = asin((warpInPoint.x / fabs(warpInPoint.x)) * (warpInPoint.z / sqrt(pow(warpInPoint.x, 2) + pow(warpInPoint.z, 2)))) + j;
    uint32 d = radius * (s + 1) + 1000000;
    warpInPoint.x += (d * sin(t));
    warpInPoint.y += (0.5f * radius * sin(j));
    warpInPoint.z -= (d * cos(t));

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

// resource node array builder for 18 order SH  (by Gemini)
/*  testing values to figure out SH data
 * idx0 = base? (l=0)  1.0f = 0.28 & 5u/h base rate
 * [idx0: 5.0f] & [idx1: 2.0f] = 0.98 & 18u/h base rate
 * idx0: 1.0 & idx1:50.0 (l=1, m=0)  = nothing
 *
 */
std::string PlanetSE::GenerateResourceBuffer(float baseDistQuantity) {
    const size_t TOTAL_FLOATS_REQUIRED = 324;
    std::vector<float> resourceFloatArray(TOTAL_FLOATS_REQUIRED, 0.0f);

    float minWaveScale  = sConfig.rates.MinWaveMultiplier;
    float maxWaveScale  = sConfig.rates.MaxWaveMultiplier;
    float waveDensity   = sConfig.rates.WavePopulationDensity;

    // 2. Assign the Master Baseline Floor (Index 0)
    float globalFloor = baseDistQuantity * sConfig.rates.DistributionScalar;
    resourceFloatArray[0] = globalFloor; //idx[0] = l0 - global
    resourceFloatArray[1] = globalFloor *  0.16f; //idx[1] = l1.x
    // skip these for 'zebra stripes'
    resourceFloatArray[2] = globalFloor *  0.11f; //idx[2] = l1.y
    resourceFloatArray[3] = globalFloor * -0.14f; //idx[3] = l1.z

    // 3. Continuous Loop to Populate ALL 323 Trailing Wave Indices Natively
    // loop thru the band (degree)
    for (int l = 2; l < 18; ++l) {
        size_t startIdx = l * l;
        size_t endIdx   = ((l + 1) * (l + 1)) - 1;

        // Frequency Decay Factor: Higher bands get progressively smaller weights
        float frequencyDecay = 1.0f / static_cast<float>(l);

        bool alternate = true;
        for (size_t i = startIdx; i <= endIdx; ++i) {
            if (i >= TOTAL_FLOATS_REQUIRED)
                break;

            if (MakeRandomFloat() < waveDensity) {
                // Base the wave amplitude directly as a configured percentage fraction of Index 0
                float baseWaveWeight = MakeRandomFloat(minWaveScale, maxWaveScale) * globalFloor * frequencyDecay;

                // Alternating signs to provide full phase variations (negatives/positives)
                if (alternate) {
                    baseWaveWeight *= -1.0f;
                }

                alternate = !alternate;

                // Apply a minor, sub-index orientation noise so shapes remain completely organic
                float orientationNoise = MakeRandomFloat(0.85f, 1.15f);
                resourceFloatArray[i] = baseWaveWeight * orientationNoise;
            } else {
                // if density fails, apply slight 'background noise'
                resourceFloatArray[i] = MakeRandomFloat(0.01f, 0.03f) * globalFloor * frequencyDecay;
            }
        }
    }

    // 4. Post-Processing Safety Sweep (Prevents any localized clipping anomalies)
    for (size_t i = 1; i < TOTAL_FLOATS_REQUIRED; ++i) {
        float maxNegativeAllowed = -0.35f * resourceFloatArray[0];
        if (resourceFloatArray[i] < maxNegativeAllowed) {
            resourceFloatArray[i] = maxNegativeAllowed * MakeRandomFloat(0.8f, 0.9f);
        }
        float maxPositiveAllowed = 1.15f * resourceFloatArray[0];
        if (resourceFloatArray[i] > maxPositiveAllowed) {
            resourceFloatArray[i] = maxPositiveAllowed * MakeRandomFloat(0.8f, 0.9f);
        }
    }

/*
    // 3. CONSOLE DEBUG DUMP (Safe Float Unpacking for GCC 4.9.2)
    std::cout << "\n=== [ALASIYA-EVE SH GENERATOR] ===" << std::endl;
    std::cout << "Input Dist Quantity: " << baseDistQuantity << std::endl;
    std::cout << "Index [0] (Base Floor): " << resourceFloatArray[0] << std::endl;
    std::cout << "Index [1] (Macro Wave): " << resourceFloatArray[1] << std::endl;
    std::cout << "Index [4] (Micro Wave): " << resourceFloatArray[4] << std::endl;
    std::cout << "Index [8] (Micro Wave): " << resourceFloatArray[8] << std::endl;
    std::cout << "Index [12] (Micro Wave): " << resourceFloatArray[12] << std::endl;
    std::cout << "===================================\n" << std::endl;
*/
    // 5. Atomic Direct Cast Serialization to safe binary container
    std::string binaryContainer(reinterpret_cast<const char*>(resourceFloatArray.data()),
                                TOTAL_FLOATS_REQUIRED * sizeof(float));

    return binaryContainer;
}

// Dynamic-Length Resource Replenishment Routine (by Gemini)
void PlanetSE::ReplenishResources() {
    if (m_typeBuffers.empty())
        return;

    float regenRate = sConfig.rates.PIRegenRate;
    // this will need to be in hours.
    float elapsedTime = GetElapsedHours(m_data.replenishTime);
    if (elapsedTime <= 0.1f)
        return;
/*
    bool change = false;
    for (auto &buffer : m_typeBuffers) {
        change = false;
        PlanetResourceBuffer &resource = buffer.second;
        std::vector<float> currentHeat = sPlanetDataMgr.DecodeHexBufferToFloats(resource.current);
        std::vector<float> spawnedHeat = sPlanetDataMgr.DecodeHexBufferToFloats(resource.spawned);

        if ((currentHeat.size() < 324) or (spawnedHeat.size() < 324))
            continue;

        for (size_t i = 0; i < 324; ++i) {
            float& maxAmplitude = spawnedHeat[i];
            float& curAmplitude = currentHeat[i];
            if (curAmplitude < maxAmplitude) {
                float growth = maxAmplitude * regenRate * elapsedTime;
                curAmplitude += growth;

                if (curAmplitude > maxAmplitude)
                    curAmplitude = maxAmplitude;
                change = true;
            }
        }

        if (change) {
            resource.current = sPlanetDataMgr.EncodeFloatsToHexBuffer(currentHeat);
        }
    }
    */
}
