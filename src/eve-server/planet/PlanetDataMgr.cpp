
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
    sLog.Cyan("    PlanetDataMgr", "%lu planet data groups in %lu buckets loaded in %.3fms.",\
            m_planetData.size(), m_planetData.bucket_count(), (GetTimeMSeconds() - start));
}

void PlanetDataMgr::GetPlanetData(uint32 planetID, std::vector<uint16> &typeIDs)
{
    auto itr = m_planetData.equal_range(planetID);
    for (auto it = itr.first; it != itr.second; ++it)
        typeIDs.push_back(it->second);
}

// Highly optimized, zero-copy 1800-byte hex encoder for C++11
std::string PlanetDataMgr::EncodeMultiNodeHexBuffer(const std::vector<float>& fullFloatArray) {
    // 225 floats * 4 bytes per float * 2 hex characters per byte = exactly 1800 characters
    const size_t TARGET_CHAR_COUNT = 1800;
    const size_t TARGET_FLOAT_COUNT = 225;

    // Allocate the exact destination string memory upfront (This acts like reserve for strings!)
    std::string hexResult;
    hexResult.resize(TARGET_CHAR_COUNT, '0'); // Pre-fill with hex zeros

    // Static hex lookup table to bypass slow printf/stream formatting loops entirely
    static const char hexChars[] = "0123456789ABCDEF";

    // Stop at 225 or the actual size of the incoming array, whichever is smaller
    size_t activeCount = std::min(fullFloatArray.size(), TARGET_FLOAT_COUNT);

    size_t charIndex = 0;
    for (size_t i = 0; i < activeCount; ++i) {
        float val = fullFloatArray[i];

        // Direct, safe aliasing of the float's IEEE-754 raw binary memory footprint
        uint32_t binaryPattern;
        std::memcpy(&binaryPattern, &val, sizeof(uint32_t)); // Cleaner/safer than reinterpret_cast on C++11

        // Encode little-endian sequence directly into the string buffer characters
        for (int byte = 0; byte < 4; ++byte) {
            uint8_t currentByte = (binaryPattern >> (byte * 8)) & 0xFF;
            hexResult[charIndex++] = hexChars[(currentByte >> 4) & 0x0F]; // High nibble
            hexResult[charIndex++] = hexChars[currentByte & 0x0F];        // Low nibble
        }
    }

    // If fullFloatArray was shorter than 225, the remaining positions in hexResult
    // are already perfectly padded with '0' because we pre-allocated with '0' characters!
    return hexResult;
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
        case 0:         return "Distant";
        case 1:         return "Region";
        case 2:         return "Constellation";
        case 3:         return "System";
        case 4:         return "Planet";
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
        case 0:         return 9.0f;
        case 1:         return 7.0f;
        case 2:         return 5.0f;
        case 3:         return 3.0f;
        case 4:         return 1.0f;
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

// do we need anything from piPinMap?  - maps SchematicID to manuf facility's typeID
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
    sLog.Cyan("        PIDataMgr", "%lu PI Schematic data groups loaded in %.3fms.", m_schematicData.size(), (GetTimeMSeconds() - start));
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

PyRep* PIDataMgr::GetProgramResultInfo(Colony* pColony, uint32 pinID, uint16 typeID, double headRadius, PyList* heads)
{
    //  ECU pinID, resource typeID, list of {headID, lat, long}, radius of head (small number...rad maybe?)
    // def InstallProgram(self, pinID, typeID, headRadius):
    // qtyToDistribute, cycleTime, numCycles = self.remoteHandler.GetProgramResultInfo(pinID, typeID, pin.heads, headRadius)

    /** @todo get head interference and calculate decayFactor, noiseFactor and overlapFactor for heads.  */

    /*
     *    SEC = 10000000L
     *    MIN = SEC * 60L
     *    HOUR = MIN * 60L
     * RADIUS_DRILLAREAMAX = 0.05
     * RADIUS_DRILLAREAMIN = 0.01
     * RADIUS_DRILLAREADIFF = RADIUS_DRILLAREAMAX - RADIUS_DRILLAREAMIN
     *
     * def GetProgramLengthFromHeadRadius(headRadius):
     *    return ((headRadius - RADIUS_DRILLAREAMIN) / RADIUS_DRILLAREADIFF) * 335 + 1   << length in hours between 1 and 336  (336h = 14d)
     * def GetCycleTimeFromProgramLength(programLength):
     *    return 0.25 * 2 ^ max(0, math.floor(math.log(programLength / 25.0, 2)) + 1)
     *
     *        programLength = planetCommon.GetProgramLengthFromHeadRadius(headRadius)
     *        cycleTime = planetCommon.GetCycleTimeFromProgramLength(programLength)
     *        numCycles = int(programLength / cycleTime)
     *        cycleTime = int(cycleTime * HOUR)
     */
    InventoryItemRef iRef = sItemFactory.GetItemRef(pinID);
    //float cycleTime = iRef->GetAttribute(AttrPinCycleTime).get_float()/*300*/;
    double one = (headRadius - 0.01) / 0.04;
    float length = one * 335.0f + 1.0f;  //length in hours between 1 and 336  (336h = 14d)
    double two = std::log(length / 25.0);  //3.584962501
    uint8 three = static_cast<uint8>(EvE::max(std::floor(two) + 1));    //4
    float cycleTime = 0.25f * (2 xor three);  // this is (float) in hours (0.25, 0.5, etc)

    if (cycleTime < 0.01f) {
        _log(COLONY__ERROR, "PlanetMgr::GetProgramResultInfo() - ecuPinID %u cycleTime < 0.01f.  setting to 1.0", pinID);
        cycleTime = 0.25f;
    }
    // modify time
    cycleTime *= sConfig.rates.DrillCycleMod;
    _log(PLANET__TRACE, "PlanetMgr::GetProgramResultInfo(test) - mod:%0.2f  cycleTime:%.2fh", sConfig.rates.DrillCycleMod, cycleTime);

    uint16 numCycles = static_cast<uint16>(std::floor(length / cycleTime));   //73
    int64 iCycleTime = (cycleTime * EvE::Time::Hour); // should be around 9000000000

    uint32 qtyPerCycle = GetProgramOutput(iRef, iCycleTime);
    qtyPerCycle *= heads->size();

    pColony->SetProgramResults(pinID, typeID, numCycles, headRadius, cycleTime, qtyPerCycle);

    _log(PLANET__TRACE, "PlanetMgr::GetProgramResultInfo() - cycleTime:%.2fh, iCycleTime:%llius, length:%.2fh, numCycles:%u, qtyPerCycle:%u, headRadius:%.4f", \
                cycleTime, iCycleTime, length, numCycles, qtyPerCycle, headRadius);

    PyTuple* res = new PyTuple(3);
        res->SetItem(0, new PyInt(qtyPerCycle));    //qtyToDistribute  (2843)
        res->SetItem(1, new PyLong(iCycleTime));    //cycleTime - in usec  (9000000000)
        res->SetItem(2, new PyInt(numCycles));      //numCycles   (12)

    if (is_log_enabled(PLANET__RES_DUMP))
        res->Dump(PLANET__RES_DUMP, "    ");
    return res;
}

// ecu program methods from client
uint32 PIDataMgr::GetProgramOutput(InventoryItemRef iRef, int64 cycleTime, int64 startTime/*0*/, int64 currentTime/*0*/)
{
    if (startTime == 0)
        startTime = GetFileTimeNow() - (2 * EvE::Time::Second);
    if (currentTime == 0)
        currentTime = GetFileTimeNow();

    float cycleNum = static_cast<float>(EvE::max((currentTime - startTime + EvE::Time::Second) / (cycleTime - 1), 1));
    float barWidth = cycleTime / EvE::Time::Second / 900.0f; //0.13888
    float t = (cycleNum + 0.5f) * barWidth; // 0.20833
    float qtyPerCycle = iRef->GetDefaultAttribute(AttrPinExtractionQuantity).get_float();
    float decayValue = qtyPerCycle / (1.0f + t * 0.012f/*iRef->GetAttribute(AttrECUDecayFactor).get_float()*/);     // 1000
    float phaseShift = std::pow(qtyPerCycle, 0.7f);   // 125.89254
    float sinA = EvE::Trig::FastCos(phaseShift + t * 0.08333333f);      // 0.96985
    float sinB = EvE::Trig::FastCos(phaseShift / 2 + t * 0.2f);  // 0.98784
    float sinC = EvE::Trig::FastCos(t * 0.5f);                   // 0.99457
    float sinStuff = (sinA + sinB + sinC) / 3.0f;  // 0.98408
    sinStuff = EvE::max(sinStuff);
    float barHeight = decayValue * (1.0f + (0.8f/*iRef->GetAttribute(AttrECUNoiseFactor).get_float()*/ * sinStuff));     //0.8

    return static_cast<uint32>(std::floor(barHeight * barWidth));     // 0.13888 * 1000          16
}

uint32 PIDataMgr::GetProgramOutputPrediction(InventoryItemRef iRef, int64 cycleTime, uint32 numCycles/*0*/)
{
    if (numCycles > 120)    // hardcoded in client
        numCycles = 120;

    uint32 val = 0;
    int64 startTime = GetFileTimeNow();

    for (int i(1); i <= numCycles; ++i) {
        int64 curTime = startTime + (i * cycleTime);
        val += GetProgramOutput(iRef, cycleTime, startTime, curTime);
    }
    return val;
}

float PIDataMgr::GetMaxOutput(InventoryItemRef iRef, uint32 qtyPerCycle/*0*/, int64 cycleTime/*0*/)
{
    if (qtyPerCycle == 0)
        qtyPerCycle = iRef->GetAttribute(AttrPinExtractionQuantity).get_uint32();
    if (cycleTime == 0)
        cycleTime = iRef->GetAttribute(AttrPinCycleTime).get_long() * EvE::Time::Second; // base time is 300s
    //float scalar = iRef->GetAttribute(AttrECUNoiseFactor).get_float() + 1;
    return (1.8f * qtyPerCycle) * cycleTime / EvE::Time::Second / 900.0f;
}

// Evaluates a single 9-float Order-2 Real SH block at a target vector location (by Gemini)
float PIDataMgr::EvaluateSingleNodeSH(const float* c, float x, float y, float z) {
    // Basis functions matching the generator
    float Y_0_0  = 0.2820948f;
    float Y_1_m1 = 0.4886025f * y;
    float Y_1_0  = 0.4886025f * z;
    float Y_1_1  = 0.4886025f * x;
    float Y_2_m2 = 1.0925484f * x * y;
    float Y_2_m1 = 1.0925484f * y * z;
    float Y_2_0  = 0.3153916f * (3.0f * z * z - 1.0f);
    float Y_2_1  = 1.0925484f * x * z;
    float Y_2_2  = 0.5462742f * (x * x - y * y);

    float density = (c[0]*Y_0_0) + (c[1]*Y_1_m1) + (c[2]*Y_1_0) + (c[3]*Y_1_1) +
                    (c[4]*Y_2_m2) + (c[5]*Y_2_m1) + (c[6]*Y_2_0) + (c[7]*Y_2_1) + (c[8]*Y_2_2);

    // This stops negative interference from breaking your extraction totals.
    if (density < 0.0f)
        return 0.0f;

    return density;
}

// Fast float-to-hex stream encoder (by Gemini)
std::string PIDataMgr::EncodeFloatsToHexBuffer(std::vector<float>& data) {
    std::string hexResult = "";
    if (data.empty())
        return hexResult;

    size_t length = data.size() * 8;
    hexResult.resize(length);

    // cast to raw byte stream
    const uint8* byteStream = reinterpret_cast<const uint8*>(data.data());
    size_t totalBytes = data.size() * sizeof(float);

    size_t writeIdx = 0;
    for (size_t i = 0; i < totalBytes; ++i) {
        uint8 curByte = byteStream[i];
        // extract high and low nibbles from the byte
        hexResult[writeIdx++] = hexList[(curByte >> 4)] & 0x0F;
        hexResult[writeIdx++] = hexList[curByte & 0x0F];
    }
    return hexResult;
}

// Converts your database string back into raw float data for evaluation (by Gemini)
std::vector<float> PIDataMgr::DecodeHexBufferToFloats(const std::string& hexBuffer) {
    std::vector<float> floats(225, 0.0f);
    if (hexBuffer.length() < 1800)
        return floats;

    for (size_t i = 0; i < 225; ++i) {
        uint32_t pattern = 0;
        for (int b = 0; b < 4; ++b) {
            std::string byteString = hexBuffer.substr((i * 8) + (b * 2), 2);
            uint8_t byteVal = (uint8_t)strtol(byteString.c_str(), nullptr, 16);
            pattern |= ((uint32_t)byteVal << (b * 8));
        }
        std::memcpy(&floats[i], &pattern, 4);
    }
    return floats;
}

// Core Execution: Calculates raw output yield and reduces the local heatmap intensity (by Gemini)
float PIDataMgr::ExtractAndDepletePlanetResource(std::string& io_dbBuffer, const PI::Heads& headPin,
                                                 float duration/*1.0f*/, float headRadius/*1.0f*/) {
    std::vector<float> floatArray = DecodeHexBufferToFloats(io_dbBuffer);
    float totalExtractedYield = 0.0f;

    float pinX = EvE::Trig::FastCos(headPin.latitude) * EvE::Trig::FastCos(headPin.longitude);
    float pinY = EvE::Trig::FastCos(headPin.latitude) * EvE::Trig::FastSin(headPin.longitude);
    float pinZ = EvE::Trig::FastSin(headPin.latitude);

    // 1. Loop through all 25 structural nodes to compile total local density
    for (int nodeIdx = 0; nodeIdx < 25; ++nodeIdx) {
        float* nodeCoeffs = &floatArray[nodeIdx * 9];
        // Skip uninitialized or dead structural cells
        if (nodeCoeffs[0] <= 0.001f)
            continue;
        // Evaluate baseline yield contributed specifically by this node mesh
        float localNodeDensity = EvaluateSingleNodeSH(nodeCoeffs, pinX, pinY, pinZ);
        // Clean out negative valleys so they don't break the calculator
        if (localNodeDensity <= 0.0f)
            continue;

        totalExtractedYield += localNodeDensity * headRadius;
        // 2. Dynamic Depletion Rule: Reduce the local node amplitude (c0) based on extraction
        // Nodes directly under or close to the pin deplete significantly faster
        float depletionAmount = localNodeDensity * 0.02f * duration;
        nodeCoeffs[0] -= depletionAmount; // Reduce base amplitude height
        if (nodeCoeffs[0] < 0.0f)
            nodeCoeffs[0] = 0.0f; // Prevent flipping to negative resources
    }

    // 3. Re-encode the newly depleted map back into hex code for your database update
    io_dbBuffer = sPlanetDataMgr.EncodeMultiNodeHexBuffer(floatArray);

    return totalExtractedYield;
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
