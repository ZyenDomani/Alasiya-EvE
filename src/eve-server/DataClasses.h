
/**
 * @name DataClasses.h
 *  data container classes that cannot be trivally constructed/destructed
 *
 * @author: allan
 * @date 4 January 2018
 */


#ifndef EVE_DATA_CLASSES_H
#define EVE_DATA_CLASSES_H

#include "eve-server.h"
#include "POD_containers.h"
#include "../eve-common/EVE_POS.h"


// POS class container for processing-type items
class ReactorData {
public:
    ReactorData();
    ~ReactorData();

    void Init();
    void Clear();

private:
    bool active;
    int32 itemID;
    int16 reaction;     // bp typeID?
    std::map<uint32, EVEPOS::POS_Connections> connections;  // itemID, data
    std::map<uint32, EVEPOS::POS_Resource> demands;         // itemID, resourceData(typeID/quantity)
    std::map<uint32, EVEPOS::POS_Resource> supplies;        // itemID, resourceData(typeID/quantity)
};


// PI classes for pins and processing
/** @todo  optimize this after everything is working!!  */
/** @todo  update this to use namespace for PI */
/*
class PI_Pin {
public:
    PI_Pin();
    ~PI_Pin();

    void Init();

private:
    bool isCommandCenter :1;
    bool isStorage :1;
    bool isConsumer :1;
    bool isLaunchable :1;
    bool isProcess :1;
    bool isBase :1;
    bool isECU :1;

    // common for all pins
    int8 state;
    uint16 level;
    uint16 typeID;
    uint32 ownerID;
    int64 lastRunTime;

    double latitude;
    double longitude;

    // Command/Spaceport
    int64 lastLaunchTime;

    //ExtractorControlUnit
    std::map<uint16, PI_Heads> heads;
    float headRadius;

    // Process and ECU
    bool hasReceivedInputs :1;
    bool receivedInputsLastCycle :1;
    uint16 schematicID;   // used in ecu as extractor head typeID
    uint16 programType;      // used in extractors as extracted resource typeID
    uint16 qtyPerCycle;
    int64 cycleTime;
    int64 expiryTime;
    int64 installTime;

    // Storage    typeID, qty
    std::map<uint16, uint32> contents;

    // specifically for updating contents. this is not saved in db
    bool update :1;
    float capacity;  // this is not implemented yet
};

class PI_CCPin {
public:
    PI_CCPin();
    ~PI_CCPin();

    void Init();

private:
    uint8 level;
    uint32 ccPinID;
    int64 currentSimTime;

    std::map<uint32, PI_Pin> pins;// pinID, pinObject
    std::map<uint32, PI_Link> links;// linkID, linkData
    std::map<uint16, PI_Route> routes;// routeID, routeData
    std::map<uint32, PI_Plant> plants;// plantPinID, plantData   - this dynamic data is not saved
};
*/

#endif  // EVE_DATA_CLASSES_H