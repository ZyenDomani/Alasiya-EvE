
 /**
  * @name PlanetDataMgr.cpp
  *   Specific Class for managing planet and pi data
  * @Author:         Allan
  * @date:   30 November 2016
  * @update: 02 August 2026
  */


 /*  log types
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

#include "../eve-server.h"
#include "math/Trig.h"

#include "EVEServerConfig.h"
#include "inventory/ItemFactory.h"
#include "inventory/InventoryItem.h"
#include "planet/PlanetDataMgr.h"
#include "planet/Colony.h"
#include "Planet.h"

PlanetDataMgr::PlanetDataMgr()
{
}

int PlanetDataMgr::Initialize()
{
    sLog.Blue("    PlanetDataMgr", "Planet Data Manager Initialized.");
    Populate();
    return 1;
}

void PlanetDataMgr::Populate()
{
    double start = GetTimeMSeconds();
    DBQueryResult* res = new DBQueryResult();
    DBResultRow row;

    m_db.GetPlanetData(*res);
    while (res->GetRow(row)) {
        // SELECT planet.typeID, resource.typeID
        m_planetData.emplace(row.GetInt(0), row.GetInt(1));
    }

    //cleanup
    SafeDelete(res);
    sLog.Cyan("    PlanetDataMgr", "%zu planet data groups in %zu buckets loaded in %.3fms.",\
            m_planetData.size(), m_planetData.bucket_count(), (GetTimeMSeconds() - start));
}

void PlanetDataMgr::GetPlanetData(uint32 planetID, std::vector<uint16> &typeIDs)
{
    auto itr = m_planetData.equal_range(planetID);
    for (auto it = itr.first; it != itr.second; ++it)
        typeIDs.push_back(it->second);
}

// Call this right before saving m_data.buffer_1 to your database text column
std::string PlanetDataMgr::BinaryToHex(const std::string& binaryInput) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned char byte : binaryInput) {
        oss << std::setw(2) << static_cast<int>(byte);
    }
    return oss.str(); // Returns a safe, 2592-character alphanumeric text string
}

// Call this right after loading the text string from the database, before assigning to m_data.buffer_1
std::string PlanetDataMgr::HexToBinary(const std::string& hexInput) {
    std::string binaryOutput;
    binaryOutput.reserve(hexInput.length() / 2);
    for (size_t i = 0; i < hexInput.length(); i += 2) {
        std::string byteString = hexInput.substr(i, 2);
        char byte = static_cast<char>(std::strtol(byteString.c_str(), nullptr, 16));
        binaryOutput.push_back(byte);
    }
    return binaryOutput; // Returns the exact 1296-byte raw data chunk
}

// Evaluates the raw SH density at a specific head location
float PlanetDataMgr::EvaluateSingleNodeSH(const std::string& binaryBuffer, float latitude, float longitude) {
    if (binaryBuffer.size() != 1296)
        return 0.0f;

    // 1. Direct type-punned pointer access to avoid memory overhead
    const float* C = reinterpret_cast<const float*>(binaryBuffer.data());

    // 2. Map EVE latitude/longitude straight into Spherical Coordinates (Theta, Phi)
    float theta = longitude;               // Azimuthal angle [0 to 2*PI]
    float phi   = EvE::Trig::halfPi - latitude; // Polar angle [0 to PI] (0 at North Pole)

    float totalDensity = C[0]; // Start directly with the global baseline floor (l=0, m=0)
    size_t idx = 0;

    // 3. Evaluate the 18 bands (Degrees l=1 up to l=17) sequentially
    for (int l = 1; l < 18; ++l) {
        // Precompute standard Legendre polynomial values for this band
        float cosPhi = cos(phi);
        float sinPhi = sin(phi);

        for (int m = -l; m <= l; ++m) {
            float waveShape = 1.0f;
            int absM = abs(m);

            // Compute basic sectorial and zonal harmonics shapes natively
            // (Standard Associated Legendre shortcut suitable for real-time emulator updates)
            float legendreP = pow(sinPhi, absM) * pow(cosPhi, l - absM);

            if (m < 0) {
                // Negative orders represent the longitudinal sine component
                waveShape = legendreP * sin(absM * theta);
            } else if (m > 0) {
                // Positive orders represent the longitudinal cosine component
                waveShape = legendreP * cos(absM * theta);
            } else {
                // Order m = 0 represents the zonal latitude stripe
                waveShape = legendreP;
            }

            // Accumulate the scaled wave amplitude weight into the coordinate density
            totalDensity += C[++idx] * waveShape;
        }
    }

    // Explicit clamp to prevent any extreme negative mathematical dips
    return (totalDensity < 0.0f) ? 0.0f : totalDensity;
}

// Evaluates the true, localized resource density at a specific head position,
// factoring in all nearby player depletion sinkholes dynamically.
float PlanetDataMgr::EvaluatePointDensityWithDepletion(
    const std::string& binaryBuffer,
    double latitude,
    double longitude,
    const std::vector<PI::ActiveEcuHead>& activePlanetPins
) {
    // 1. Calculate the raw, un-depleted natural maximum density from your master SH array
    // This runs your 18-band Associated Legendre Real-Valued Spherical Harmonics combination
    float naturalMaxDensity = EvaluateSingleNodeSH(binaryBuffer, latitude, longitude);

    float totalLocalDepletion = 0.0f;

    // 2. Sweep across all active extraction pins on the planet to see if their
    // extraction radii overlap our target evaluation coordinate
    for (const auto& pin : activePlanetPins) {
        if (pin.depletionAmount <= 0.0f)
            continue;

        // Calculate angular distance on a 3D unit sphere using the Haversine formula
        double dLat = latitude - pin.latitude;
        double dLon = longitude - pin.longitude;

        double a = std::sin(dLat / 2.0) * std::sin(dLat / 2.0) +
        std::cos(pin.latitude) * std::cos(latitude) *
        std::sin(dLon / 2.0) * std::sin(dLon / 2.0);

        double angularDistance = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));

        // If the coordinate falls within the physical extraction radius of this pin,
        // calculate a smooth, sloped mathematical sinkhole (Gaussian curve)
        if (angularDistance <= pin.headRadius) {
            // Factor determines how close to the dead center of the pin the coordinate is
            double normalizedDistance = angularDistance / pin.headRadius;

            // Smooth bell-curve drop: max depletion at center, tapering out to 0 at the edge
            float falloff = std::exp(-3.0f * static_cast<float>(normalizedDistance * normalizedDistance));

            totalLocalDepletion += pin.depletionAmount * falloff;
        }
    }

    // 3. Subtract the localized depletion from the natural base
    float finalDensity = naturalMaxDensity - totalLocalDepletion;

    // Explicit clamp to prevent any extreme negative mathematical dips from hitting the shader or cargo
    return (finalDensity < 0.0f) ? 0.0f : finalDensity;
}

int32 PlanetDataMgr::CalculateEcuYield(PlanetSE* pSE, uint16 cycles, std::string& resourceBuffer,
                                       std::unordered_map<uint16, PI::Heads>& heads,
                                       std::vector<PI::ActiveEcuHead>& progressiveMasks) {

    double amount = 0.0;
    float scarcity = pSE->GetScarcity();
    float abundance = pSE->GetAbundance();

    // Loop through your 15-minute segments progressively to simulate depletion curves
    for (uint16 cycle = 0; cycle < cycles; ++cycle) {
        // Run through the active heads for this ECU pin and execute the math
        for (auto &head : heads) {
            // 1. Evaluate the dynamic coordinate density at this step
            float currentLocalDensity = sPlanetDataMgr.EvaluatePointDensityWithDepletion(
                resourceBuffer,
                head.second.latitude,
                head.second.longitude,
                progressiveMasks
            );

            // 2. Calculate the yield for this 15-minute interval segment (0.25h)
            float cycleYield = currentLocalDensity * 0.25f * scarcity * abundance;
            if (cycleYield < 0.0f)
                cycleYield = 0.0f;

            amount += cycleYield;

            // 3. Drive up the ephemeral local depletion member natively in RAM
            // This ensures the hotspot smoothly degrades hour-by-hour over the time skip!
            float stepDecay = cycleYield * 0.0015f;
            head.second.currentDepletion += stepDecay;
        }
    }

    return static_cast<int32>(EvE::max(floor(amount)));
}

const char* PlanetDataMgr::GetCommandName(int8 commandID)
{
    using namespace PI::Command;
    switch (commandID) {
        case Invalid:                       return "Invalid";
        case CreatePin:                     return "CreatePin";
        case RemovePin:                     return "RemovePin";
        case CreateLink:                    return "CreateLink";
        case RemoveLink:                    return "RemoveLink";
        case CreateRoute:                   return "CreateRoute";
        case SetLinkLevel:                  return "SetLinkLevel";
        case UpgradeCommandCenter:          return "UpgradeCommandCenter";
        case SetSchematic:                  return "SetSchematic";
        case RemoveRoute:                   return "RemoveRoute";
        case AddExtractorHead:              return "AddExtractorHead";
        case MoveExtractorHead:             return "MoveExtractorHead";
        case InstallProgram:                return "InstallProgram";
        case KillExtractorHead:             return "KillExtractorHead";
        case PrioritizeRoute:               return "PrioritizeRoute";
        default:                            return "UnknownCommandID";
    }
}

const char* PlanetDataMgr::GetProximity(uint8 level) {
    switch (level) {
        case 0:         return "Planet";
        case 1:         return "Planet";
        case 2:         return "System";
        case 3:         return "Constellation";
        case 4:         return "Region";
        case 5:         return "Distant";
        default:        return "Invalid";
    }
}

void PlanetDataMgr::GetProximityLimits(uint8 level, uint8& minBand, uint8& maxBand) {
    switch (level) {
        case 0: {
            minBand = 2;
            maxBand = 6;
        } break;
        case 1: {
            minBand = 4;
            maxBand = 10;
        } break;
        case 2: {
            minBand = 6;
            maxBand = 15;
        } break;
        case 3: {
            minBand = 10;
            maxBand = 20;
        } break;
        case 4: {
            minBand = 15;
            maxBand = 30;
        } break;
        default: {
            minBand = 0;
            maxBand = 0;
        } break;
    }
}

float PlanetDataMgr::GetScanningRange(uint8 level) {
    switch (level) {
        case 0:         return 0.0f;
        case 1:         return 1.0f;
        case 2:         return 3.0f;
        case 3:         return 5.0f;
        case 4:         return 7.0f;
        case 5:         return 9.0f;
        default:        return 0.0f;
    }
}

float PlanetDataMgr::GetAbundanceMod(uint16 typeID) {
    switch (typeID) {
        case 12:      //Ice
            return 1.35f;
        case 13:      //Gas
            return 1.5f;
        case 2014:    //Oceanic
            return 1.15f;
        case 2015:    //Lava
            return 0.85f;
        case 2016:    //Barren
            return 0.9f;
        case 2017:    //Storm
            return 1.2f;
        case 2063:    //Plasma
            return 1.1f;
        case 11:      //Temperate
        case 30889:   //Shattered
            return 1.0f;
    }

    // catchall
    return 1.0f;
}



PIDataMgr::PIDataMgr()
{
}

int PIDataMgr::Initialize()
{
    sLog.Blue("        PIDataMgr", "Planet Interaction Data Manager Initialized.");
    Populate();
    return 1;
}

void PIDataMgr::Populate()
{
    double start = GetTimeMSeconds();
    DBQueryResult* res = new DBQueryResult();

    m_db.GetSchematicData(*res);
    DBResultRow row;
    std::map<uint8, PI::Schematic>::iterator itr;
    while (res->GetRow(row)) {
        // SELECT `schematicID`, `typeID`, `quantity`, `isInput`
        itr = m_schematicData.find(row.GetInt(0));
        if (itr != m_schematicData.end()) {
            if (row.GetBool(3)) {
                itr->second.inputs[row.GetInt(1)] = row.GetInt(2);
            } else {
                itr->second.outputType = row.GetInt(1);
                itr->second.outputQty = row.GetInt(2);
            }
        } else {
            PI::Schematic data = PI::Schematic();
            if (row.GetBool(3)) {
                data.inputs[row.GetInt(1)] = row.GetInt(2);
            } else {
                data.outputType = row.GetInt(1);
                data.outputQty = row.GetInt(2);
            }
            m_schematicData[row.GetInt(0)] = data;
        }
    }

    m_db.GetSchematicTimes(*res);
    while (res->GetRow(row)) {
        // SELECT `schematicID`, `cycleTime`
        itr = m_schematicData.find(row.GetInt(0));
        if (itr != m_schematicData.end())
            itr->second.cycleTime = row.GetInt(1);
    }

    //cleanup
    SafeDelete(res);
    sLog.Cyan("        PIDataMgr", "%zu PI Schematic data groups loaded in %.3fms.", m_schematicData.size(), (GetTimeMSeconds() - start));
}

void PIDataMgr::GetSchematicData(uint8 schematicID, PI::Schematic& data)
{
    std::map<uint8, PI::Schematic>::iterator itr = m_schematicData.find(schematicID);
    if (itr != m_schematicData.end()) {
        data = itr->second;
        return;
    }
    _log(PLANET__ERROR, "PIDataMgr::GetSchematicData() - Data not found for schematic %u", schematicID);
}

/* these are for PI */
/*
 *    AttrHarvesterType = 709,
 *    AttrHarvesterQuality = 710,
 *    AttrLogisticalCapacity = 1631,
 *    AttrPlanetRestriction = 1632,
 *    AttrPowerLoadPerKm = 1633,
 *    AttrCPULoadPerKm = 1634,
 *    AttrCPULoadLevelModifier = 1635,
 *    AttrPowerLoadLevelModifier = 1636,
 *    AttrImportTax = 1638,
 *    AttrExportTax = 1639,
 *    AttrImportTaxMultiplier = 1640,
 *    AttrExportTaxMultiplier = 1641,
 *    AttrPinExtractionQuantity = 1642,         // 1000
 *    AttrPinCycleTime = 1643,
 *    AttrExtractorDepletionRange = 1644,
 *    AttrExtractorDepletionRate = 1645,        // 1
 *    AttrCommandCenterHoldCapacity = 1646,
 *    AttrECUDecayFactor = 1683,            // this attr is empty (0.012f)
 *    AttrECUMaxVolume = 1684,
 *    AttrECUOverlapFactor = 1685,
 *    AttrECUNoiseFactor = 1687,            // this attr is empty (0.8f)
 *    AttrECUAreaOfInfluence = 1689,
 *    AttrECUExtractorHeadCPU = 1690,
 *    AttrECUExtractorHeadPower = 1691,
 *
 * decayFactor = 0.012
 * noiseFactor = 0.8
 * baseValue = 1998.0
 */
PyRep* PIDataMgr::GetProgramResultInfo(Colony* pColony, uint32 pinID, uint16 typeID, double headRadius, PyList* heads) {
    if (pColony == nullptr || heads == nullptr || heads->size() == 0) {
        _log(COLONY__ERROR, "PIDataMgr::GetProgramResultInfo() - missing data. Returning failRes.");
        PyTuple* failRes = new PyTuple(3);
        failRes->SetItem(0, new PyInt(0));
        failRes->SetItem(1, new PyLong(9000000000LL));
        failRes->SetItem(2, new PyInt(0));
        return failRes;
    }

    // 1. Calculate length, cycle time, and number of cycles using authentic client-mapped math
    double one = (headRadius - 0.01) / 0.04;
    float length = one * 335.0f + 1.0f; // Total runtime length in hours (1 to 336)
    double two = std::log(length / 25.0);
    uint8 three = static_cast<uint8>(EvE::max(std::floor(two) + 1.0));
    // Simulates the client python calculation: 0.25 * 2^max(0, floor(log(L/25, 2)) + 1)
    float cycleTime = 0.25f * static_cast<float>(1 << three);

    if (cycleTime < 0.01f) {
        _log(COLONY__ERROR, "PIDataMgr::GetProgramResultInfo() - cycleTime invalid. Forcing to 0.25h");
        cycleTime = 0.25f;
    }

    uint16 numCycles = static_cast<uint16>(std::floor(length / cycleTime));
    int64 iCycleTime = static_cast<int64>(std::ceil(cycleTime * EvE::Time::Hour));

    // 3. Assemble your type-isolated dynamic depletion profile from active pins on the planet
    std::vector<PI::ActiveEcuHead> progressiveMasks;
    std::unordered_map<uint32, PI::ECU> ecuMap = pColony->GetECUMap();
    for (const auto& activeEcu : ecuMap) {
        if (activeEcu.second.programType != typeID)
            continue;
        for (const auto& activeHead : activeEcu.second.heads) {
            PI::ActiveEcuHead mask;
            mask.pinID = activeHead.second.ecuPinID;
            mask.latitude = activeHead.second.latitude;
            mask.longitude = activeHead.second.longitude;
            mask.headRadius = activeEcu.second.headRadius;
            mask.depletionAmount = activeHead.second.currentDepletion;
            progressiveMasks.push_back(mask);
        }
    }

    std::unordered_map<uint16, PI::Heads> tempHeads;
    for (int i = 0; i < heads->size(); ++i) {
        PyTuple* coord = reinterpret_cast<PyTuple*>(heads->GetItem(i));
        uint32 headID = PyRep::IntegerValueU32(coord->GetItem(0));
        PI::Heads tmpHead;
            tmpHead.latitude = PyRep::FloatValue(coord->GetItem(1));
            tmpHead.longitude = PyRep::FloatValue(coord->GetItem(2));
            tmpHead.currentDepletion = 0.0f;
        tempHeads.emplace(headID, tmpHead);
    }

    uint16 subCycles = static_cast<uint16>(round(cycleTime / 0.25f));
    std::string& resourceBuffer = pColony->GetPlanet()->GetResourceBuffer(typeID);
    int32 qtyPerCycle = sPlanetDataMgr.CalculateEcuYield(pColony->GetPlanet(), subCycles, resourceBuffer, tempHeads, progressiveMasks);

    _log(PLANET__TRACE, "PIDataMgr::GetProgramResultInfo() UNIFIED - cycleTime:%.2fh, numCycles:%u, qtyPerCycle:%i",
         cycleTime, numCycles, qtyPerCycle);

    // Save state changes directly to the colony layout in RAM before confirming the network frame
    pColony->SetProgramResults(pinID, typeID, numCycles, headRadius, cycleTime, qtyPerCycle);

    // 5. Pack responses into clean Python tuples for standard client synchronization
    PyTuple* res = new PyTuple(3);
        res->SetItem(0, new PyInt(qtyPerCycle));   // Item 0: qtyToDistribute
        res->SetItem(1, new PyLong(iCycleTime));   // Item 1: cycleTime in microseconds
        res->SetItem(2, new PyInt(numCycles));     // Item 2: total program cycles

    if (is_log_enabled(PLANET__RES_DUMP))
        res->Dump(PLANET__RES_DUMP, "    ");

    return res;
}

uint16 PIDataMgr::GetHeadType(uint16 ecuTypeID, uint16 programType) {
    if (programType == 0)
        return 0;

    switch (ecuTypeID) {
        case 2848: {  //Barren ECU
            switch (programType) {
                case 2268: return 2409; //Barren Aqueous Liquid Extractor
                case 2267: return 2430; //Barren Base Metals Extractor
                case 2270: return 2435; //Barren Noble Metals Extractor
                case 2073: return 2449; //Barren Microorganisms Extractor
                case 2288: return 2459; //Barren Carbon Compounds Extractor
            }
        } break;
        case 3060: {  //Gas ECU
            switch (programType) {
                case 2268: return 2416; //Gas Aqueous Liquid Extractor
                case 2309: return 2424; //Gas Ionic Solutions Extractor
                case 2310: return 2426; //Gas Noble Gas Extractor
                case 2311: return 2427; //Gas Reactive Gas Extractor
                case 2267: return 2433; //Gas Base Metals Extractor
            }
        } break;
        case 3061: {  //Ice ECU
            switch (programType) {
                case 2268: return 2415; //Ice Aqueous Liquid Extractor
                case 2310: return 2423; //Ice Noble Gas Extractor
                case 2073: return 2432; //Ice Microorganisms Extractor
                case 2286: return 2438; //Ice Planktic Colonies Extractor
                case 2272: return 2441; //Ice Heavy Metals Extractor
            }
        } break;
        case 3062: {  //Lava ECU
            switch (programType) {
                case 2308: return 2418; //Lava Suspended Plasma Extractor
                case 2267: return 2428; //Lava Base Metals Extractor
                case 2272: return 2439; //Lava Heavy Metals Extractor
                case 2306: return 2442; //Lava Non-CS Crystals Extractor
                case 2307: return 2448; //Lava Felsic Magma Extractor
            }
        } break;
        case 3063: {  //Oceanic ECU
            switch (programType) {
                case 2268: return 2414; //Oceanic Aqueous Liquid Extractor
                case 2287: return 2458; //Oceanic Complex Organisms Extractor
                case 2286: return 2452; //Oceanic Planktic Colonies Extractor
                case 2288: return 2461; //Oceanic Carbon Compounds Extractor
                case 2073: return 2451; //Oceanic Microorganisms Extractor
            }
        } break;
        case 3064: {  //Plasma ECU
            switch (programType) {
                case 2308: return 2417; //Plasma Suspended Plasma Extractor
                case 2267: return 2429; //Plasma Base Metals Extractor
                case 2270: return 2434; //Plasma Noble Metals Extractor
                case 2272: return 2440; //Plasma Heavy Metals Extractor
                case 2306: return 2443; //Plasma Non-CS Crystals Extractor
            }
        } break;
        case 3067: {  //Storm ECU
            switch (programType) {
                case 2268: return 2413; //Storm Aqueous Liquid Extractor
                case 2308: return 2419; //Storm Suspended Plasma Extractor
                case 2309: return 2422; //Storm Ionic Solutions Extractor
                case 2310: return 2425; //Storm Noble Gas Extractor
                case 2267: return 2431; //Storm Base Metals Extractor
            }
        } break;
        case 3068: {  //Temperate ECU
            switch (programType) {
                case 2268: return 2412; //Temperate Aqueous Liquid Extractor
                case 2073: return 2450; //Temperate Microorganisms Extractor
                case 2287: return 2453; //Temperate Complex Organisms Extractor
                case 2288: return 2460; //Temperate Carbon Compounds Extractor
                case 2305: return 2462; //Temperate Autotrophs Extractor
            }
        } break;
    }
    _log(PLANET__ERROR, "PIDataMgr::GetHeadType() - Extractor typeID not found using ECU typeID: %u and Resource typeID: %u", ecuTypeID, programType);
    return 0;
}

uint8 PIDataMgr::GetProductLevel(uint16 typeID)
{
    switch (typeID) {
    // P0 - Raw Materials
        case  2267: //Base Metals
        case  2270: //Noble Metals
        case  2272: //Heavy Metals
        case  2306: //Non-CS Crystals
        case  2307: //Felsic Magma
        case  2268: //Aqueous Liquids
        case  2308: //Suspended Plasma
        case  2309: //Ionic Solutions
        case  2310: //Noble Gas
        case  2311: //Reactive Gas
        case  2073: //Microorganisms
        case  2286: //Planktic Colonies
        case  2287: //Complex Organisms
        case  2288: //Carbon Compounds
        case  2305: //Autotrophs
            return 0;

    // P1 - Basic Commodities
        case  2389: //Plasmoids
        case  2390: //Electrolytes
        case  2392: //Oxidizing Compound
        case  2393: //Bacteria
        case  2395: //Proteins
        case  2396: //Biofuels
        case  2397: //Industrial Fibers
        case  2398: //Reactive Metals
        case  2399: //Precious Metals
        case  2400: //Toxic Metals
        case  2401: //Chiral Structures
        case  3779: //Biomass
        case  9828: //Silicon
        case  3683: //Oxygen
        case  3645: //Water
            return 1;

    // P2 - Refined Commodities
        case    44: //Enriched Uranium
        case  2312: //Supertensile Plastics
        case  2317: //Oxides
        case  2319: //Test Cultures
        case  2321: //Polyaramids
        case  2327: //Microfiber Shielding
        case  2328: //Water-Cooled CPU
        case  2329: //Biocells
        case  2463: //Nanites
        case  3689: //Mechanical Parts
        case  3691: //Synthetic Oil
        case  3693: //Fertilizer
        case  3695: //Polytextiles
        case  3697: //Silicate Glass
        case  3725: //Livestock
        case  3775: //Viral Agent
        case  3828: //Construction Blocks
        case  9830: //Rocket Fuel
        case  9832: //Coolant
        case  9836: //Consumer Electronics
        case  9838: //Superconductors
        case  9840: //Transmitter
        case  9842: //Miniature Electronics
        case 15317: //Genetically Enhanced Livestock
            return 2;

    // P3 - Specialized Commodities
        case  2344: //Condensates
        case  2345: //Camera Drones
        case  2346: //Synthetic Synapses
        case  2348: //Gel-Matrix Biopaste
        case  2349: //Supercomputers
        case  2351: //Smartfab Units
        case  2352: //Nuclear Reactors
        case  2354: //Neocoms
        case  2358: //Biotech Research Reports
        case  2360: //Industrial Explosives
        case  2361: //Hermetic Membranes
        case  2366: //Hazmat Detection Systems
        case  2367: //Cryoprotectant Solution
        case  9834: //Guidance Systems
        case  9846: //Planetary Vehicles
        case  9848: //Robotics
        case 12836: //Transcranial Microcontrollers
        case 17136: //Ukomi Superconductors
        case 17392: //Data Chips
        case 17898: //High-Tech Transmitters
        case 28974: //Vaccines
            return 3;

    // P4 - Advanced Commodities
        case  2867: //Broadcast Node
        case  2868: //Integrity Response Drones
        case  2869: //Nano-Factory
        case  2870: //Organic Mortar Applicators
        case  2871: //Recursive Computing Module
        case  2872: //Self-Harmonizing Power Core
        case  2875: //Sterile Conduits
        case  2876: //Wetware Mainframe
            return 4;
    }
    _log(PLANET__ERROR, "PIDataMgr::GetProductLevel() - Commodity product level not found for typeID: %u", typeID);
    EvE::traceStack();
    return 0;
}

const char* PIDataMgr::GetProductName(uint16 typeID)
{
    switch (typeID) {
        // P0 - Raw Materials
        case  2267:        return "Base Metals";
        case  2270:        return "Noble Metals";
        case  2272:        return "Heavy Metals";
        case  2306:        return "Non-CS Crystals";
        case  2307:        return "Felsic Magma";
        case  2268:        return "Aqueous Liquids";
        case  2308:        return "Suspended Plasma";
        case  2309:        return "Ionic Solutions";
        case  2310:        return "Noble Gas";
        case  2311:        return "Reactive Gas";
        case  2073:        return "Microorganisms";
        case  2286:        return "Planktic Colonies";
        case  2287:        return "Complex Organisms";
        case  2288:        return "Carbon Compounds";
        case  2305:        return "Autotrophs";
            // P1 - Basic Commodities
        case  2389:        return "Plasmoids";
        case  2390:        return "Electrolytes";
        case  2392:        return "Oxidizing Compound";
        case  2393:        return "Bacteria";
        case  2395:        return "Proteins";
        case  2396:        return "Biofuels";
        case  2397:        return "Industrial Fibers";
        case  2398:        return "Reactive Metals";
        case  2399:        return "Precious Metals";
        case  2400:        return "Toxic Metals";
        case  2401:        return "Chiral Structures";
        case  3779:        return "Biomass";
        case  9828:        return "Silicon";
        case  3683:        return "Oxygen";
        case  3645:        return "Water";
            // P2 - Refined Commodities
        case    44:        return "Enriched Uranium";
        case  2312:        return "Supertensile Plastics";
        case  2317:        return "Oxides";
        case  2319:        return "Test Cultures";
        case  2321:        return "Polyaramids";
        case  2327:        return "Microfiber Shielding";
        case  2328:        return "Water-Cooled CPU";
        case  2329:        return "Biocells";
        case  2463:        return "Nanites";
        case  3689:        return "Mechanical Parts";
        case  3691:        return "Synthetic Oil";
        case  3693:        return "Fertilizer";
        case  3695:        return "Polytextiles";
        case  3697:        return "Silicate Glass";
        case  3725:        return "Livestock";
        case  3775:        return "Viral Agent";
        case  3828:        return "Construction Blocks";
        case  9830:        return "Rocket Fuel";
        case  9832:        return "Coolant";
        case  9836:        return "Consumer Electronics";
        case  9838:        return "Superconductors";
        case  9840:        return "Transmitter";
        case  9842:        return "Miniature Electronics";
        case 15317:        return "Genetically Enhanced Livestock";
            // P3 - Specialized Commodities
        case  2344:        return "Condensates";
        case  2345:        return "Camera Drones";
        case  2346:        return "Synthetic Synapses";
        case  2348:        return "Gel-Matrix Biopaste";
        case  2349:        return "Supercomputers";
        case  2351:        return "Smartfab Units";
        case  2352:        return "Nuclear Reactors";
        case  2354:        return "Neocoms";
        case  2358:        return "Biotech Research Reports";
        case  2360:        return "Industrial Explosives";
        case  2361:        return "Hermetic Membranes";
        case  2366:        return "Hazmat Detection Systems";
        case  2367:        return "Cryoprotectant Solution";
        case  9834:        return "Guidance Systems";
        case  9846:        return "Planetary Vehicles";
        case  9848:        return "Robotics";
        case 12836:        return "Transcranial Microcontrollers";
        case 17136:        return "Ukomi Superconductors";
        case 17392:        return "Data Chips";
        case 17898:        return "High-Tech Transmitters";
        case 28974:        return "Vaccines";
            // P4 - Advanced Commodities
        case  2867:        return "Broadcast Node";
        case  2868:        return "Integrity Response Drones";
        case  2869:        return "Nano-Factory";
        case  2870:        return "Organic Mortar Applicators";
        case  2871:        return "Recursive Computing Module";
        case  2872:        return "Self-Harmonizing Power Core";
        case  2875:        return "Sterile Conduits";
        case  2876:        return "Wetware Mainframe";
    }
    _log(PLANET__ERROR, "PIDataMgr::GetProductName() - Commodity product not found for typeID: %u", typeID);
    return "NULL";
}

const ItemType* PIDataMgr::GetPinType(uint32 pinID) {
    if (pinID < 65535) // max uint16
        return sItemFactory.GetType(pinID);
    InventoryItemRef iRef = sItemFactory.GetItemRef(pinID);
    if (iRef.get() == nullptr)
        return nullptr;
    return &(iRef->type());
}

const ItemType* PIDataMgr::GetPinType(uint16 typeID) {
    return sItemFactory.GetType(typeID);
}

InventoryItemRef PIDataMgr::GetPinRef(uint32 pinID) {
    return sItemFactory.GetItemRef(pinID);
}

const char* PIDataMgr::GetPinTypeName(uint16 typeID) {
    const ItemType* iType = sItemFactory.GetType(typeID);
    if (iType == nullptr)
        return "NULL";
    return iType->name().c_str();
}

const char* PIDataMgr::GetPinName(uint32 pinID) {
    if (pinID < 65535) { // max uint16
        const ItemType* iType = sItemFactory.GetType(pinID);
        if (iType == nullptr)
            return "NULL";
        return iType->name().c_str();
    }
    InventoryItemRef iRef = sItemFactory.GetItemRef(pinID);
    if (iRef.get() == nullptr)
        return "NULL";
    return iRef->name();
}
