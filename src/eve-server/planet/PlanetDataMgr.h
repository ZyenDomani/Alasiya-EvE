
 /**
  * @name PlanetDataMgr.h
  *   Specific Class for managing planet and pi data
  * @Author:         Allan
  * @date:   30 November 2016
  */


#ifndef EVEMU_PLANET_PLANETDATAMGR_H_
#define EVEMU_PLANET_PLANETDATAMGR_H_

#include <unordered_map>
#include "planet/PlanetDB.h"


class PlanetSE;

// this class is a singleton object to have a common place for all (cached) planet data
class PlanetDataMgr
: public Singleton< PlanetDataMgr >
{
public:
    PlanetDataMgr();
    PlanetDataMgr(PlanetDataMgr&&) =delete;
    PlanetDataMgr(const PlanetDataMgr&) =delete;
    PlanetDataMgr& operator=(PlanetDataMgr&&) =delete;
    PlanetDataMgr& operator=(const PlanetDataMgr&) =delete;
    ~PlanetDataMgr()                                    { /* nothing do to yet */ }

    // Initializes the Table:
    int Initialize();

    void GetPlanetData(uint32 planetID, std::vector<uint16> &typeIDs);

    const char* GetCommandName(int8 commandID);
    const char* GetProximity(uint8 level);
    // scan range in Lightyears
    float GetScanningRange(uint8 level);
    // gets min/max bands based on proximity
    void GetProximityLimits(uint8 level, uint8 &minBand, uint8 &maxBand);
    // multipliers from baseline scanner dist rules
    float GetAbundanceMod(uint16 typeID);

    std::string HexToBinary(const std::string& hexInput);
    // Converts your database string back into raw float data for evaluation
    std::string BinaryToHex(const std::string& binaryInput);
    // Evaluates a single SH block at a target vector location
    float EvaluateSingleNodeSH(const std::string& binaryBuffer, float latitude, float longitude);
    // Calculates raw output yield and reduces the local heatmap intensity
    float EvaluatePointDensityWithDepletion(const std::string& binaryBuffer,
                                            double latitude, double longitude,
                                            const std::vector<PI::ActiveEcuHead>& activePlanetPins);

    int32 CalculateEcuYield(PlanetSE* pSE, uint16 cycles, std::string& resourceBuffer,
                            std::unordered_map<uint16, PI::Heads>& heads,
                            std::vector<PI::ActiveEcuHead>& progressiveMasks);

protected:
    void Populate();


private:
    PlanetDB m_db;

    std::unordered_multimap<uint32, uint32> m_planetData;
};

#define sPlanetDataMgr \
( PlanetDataMgr::get() )


class Colony;
class ItemType;

// this class is a singleton object to have a common place for all (cached) PI schematic and program data
class PIDataMgr
: public Singleton< PIDataMgr >
{
public:
    PIDataMgr();
    PIDataMgr(PIDataMgr&&) =delete;
    PIDataMgr(const PIDataMgr&) =delete;
    PIDataMgr& operator=(PIDataMgr&&) =delete;
    PIDataMgr& operator=(const PIDataMgr&) =delete;
    ~PIDataMgr()                                        { /* do nothing here */ }

    // Initializes the Table:
    int Initialize();

    PyRep* GetProgramResultInfo(Colony* pColony, uint32 pinID, uint16 typeID, double headRadius, PyList* heads);

    void GetSchematicData(uint8 schematicID, PI::Schematic& data);

    uint8 GetProductLevel(uint16 typeID);
    uint16 GetHeadType(uint16 ecuTypeID, uint16 programType);

    // test your return; this can return null
    const ItemType* GetPinType(uint32 pinID);
    // test your return; this can return null
    const ItemType* GetPinType(uint16 typeID);
    InventoryItemRef GetPinRef(uint32 pinID);
    const char* GetPinName(uint32 pinID);
    const char* GetProductName(uint16 typeID);
    const char* GetPinTypeName(uint16 typeID);

protected:
    void Populate();

private:
    PlanetDB m_db;

    std::map<uint8, PI::Schematic> m_schematicData;
};

#define sPIDataMgr \
( PIDataMgr::get() )


#endif  // EVEMU_PLANET_PLANETDATAMGR_H_