
/*  EVE_Planet.h
 *    enumerators and other defines for PI system
 *
 *  Base PI system code by Comet0
 * Updates and rewrites by Allan
 */


#ifndef EVE_PLANET_H
#define EVE_PLANET_H
#include <map>

#include "eve-common.h"

namespace PI {
    namespace Command {
        enum  {
            Invalid                 = 0,
            CreatePin               = 1,
            RemovePin               = 2,
            CreateLink              = 3,
            RemoveLink              = 4,
            SetLinkLevel            = 5,
            CreateRoute             = 6,
            RemoveRoute             = 7,
            SetSchematic            = 8,
            UpgradeCommandCenter    = 9,
            AddExtractorHead        = 10,
            KillExtractorHead       = 11,
            MoveExtractorHead       = 12,
            InstallProgram          = 13,
            PrioritizeRoute         = 14
        };
    }

    namespace Pin {
        namespace State {
            enum {
                Edit        = -2,
                Disabled    = -1,
                Idle        =  0,
                Active      =  1
            };
        }

        enum {
            Level0  = 0,
            Level1  = 1,
            Level2  = 2,
            Level3  = 3,
            Level4  = 4,
            Level5  = 5,
            // the following are only used by Planetary_Links
            Level6  = 6,
            Level7  = 7,
            Level8  = 8,
            Level9  = 9,
            Level10 = 10
        };
    }

    namespace Route {
        enum {
            PriorityLow  = -1,
            PriorityNorm =  0,
            PriorityHi   =  1
        };
    }

    //piLaunchOrbitDecayTime = DAY * 5
    namespace Cargo {
        enum {
            InOrbit     = 0,
            Deployed    = 1,
            Claimed     = 2,
            Deleted     = 3
        };
    }

}

namespace Launch {
    struct Data {
        uint8 status=0;
        uint32 launchID=0;
        uint32 itemID=0;
        uint32 solarSystemID=0;
        uint32 planetID=0;
        int64 launchTime=0;
        double x=0.0;
        double y=0.0;
        double z=0.0;
    };
}

/** @todo these need their own namespace */
/* POD structure entries for PI data */
struct PlanetResourceData {
    uint16 type_1=0;
    uint16 type_2=0;
    uint16 type_3=0;
    uint16 type_4=0;
    uint16 type_5=0;
    float dist_1=0.0f;
    float dist_2=0.0f;
    float dist_3=0.0f;
    float dist_4=0.0f;
    float dist_5=0.0f;
    std::string buffer_1="";
    std::string buffer_2="";
    std::string buffer_3="";
    std::string buffer_4="";
    std::string buffer_5="";
};

struct PI_Link {
    int8 state=0;
    uint16 level=0;
    uint16 typeID=0;
    uint32 endpoint1=0;
    uint32 endpoint2=0;
};

struct PI_Route {
    int8 state=0;
    int8 priority=0;
    uint16 commodityTypeID=0;
    uint16 commodityQuantity=0;         // current route qty, updated for diminishing returns on each loop
    uint32 srcPinID=0;
    uint32 destPinID=0;
    std::list<uint32> path;
};

struct PI_Heads {
    uint16 typeID=0;
    uint32 ecuPinID=0;
    double latitude=0.0;
    double longitude=0.0;
};

struct PI_Schematic {
    uint16 outputQty=0;
    uint16 outputType=0;
    uint16 cycleTime=0;                 // in seconds

    // typeID, qty
    std::map<uint16, uint16> inputs;
};

struct PI_Plant {
    // specifically for processing plants. this is not saved in db as a group, but is in pinData
    // these two are checked in client for the pin.CanActivate() method.  it will return true if either are true.
    bool hasReceivedInputs :1;          // Process Only
    bool receivedInputsLastCycle :1;    // Process Only

    uint8 pLevel=0;                     // production level of this plant
    uint8 schematicID=0;
    uint16 qtyPerCycle=0;
    int64 cycleTime=0;                  // in filetime
    int64 lastRunTime=0;                // in filetime

    PI_Schematic data=PI_Schematic();
};

struct PI_Pin {
    bool update :1;                     // specifically for updating contents in db. this is a runtime value.

    bool isECU :1;                      // common for all pins
    bool isBase :1;                     // common for all pins
    bool isStorage :1;                  // common for all pins
    bool isProcess :1;                  // common for all pins
    bool isConsumer :1;                 // common for all pins
    bool isLaunchable :1;               // common for all pins
    bool isCommandCenter :1;            // common for all pins

    int8 state=-1;                      // common for all pins
    uint16 level=0;                     // common for all pins
    uint16 typeID=0;                    // common for all pins
    uint16 schematicID=0;               // Process type, also used in ecu as extractor head typeID
    uint16 programType=0;               // used in extractors as extracted resource typeID
    uint32 qtyPerCycle=0;               // Process and ECU
    uint32 ownerID=0;                   // common for all pins
    int64 lastRunTime=0;                // common for all pins - copy of launchTime for Spaceports   // saved as filetime
    int64 cycleTime=0;                  // Process and ECU      // saved as filetime
    int64 expiryTime=0;                 // ECU Only             // saved as filetime
    int64 installTime=0;                // ECU Only - used by client to calculate data      // saved as filetime
    int64 lastLaunchTime=0;             // Command Center and Spaceports  // saved as filetime

    double headRadius=0.0;              // ECU Only
    double latitude=0.0;                // planetary location common for all pins
    double longitude=0.0;               // planetary location common for all pins

    std::map<uint16, PI_Heads> heads;   // ECU Only
    std::map<uint16, uint32> contents;  // Storage    <typeID, qty>
};


class PI_CCPin {
public:
    PI_CCPin() : level(0), ccPinID(0)                   { /* Init(); */ }
    ~PI_CCPin()                                         { /* do nothing here */ }

    void Clear()
    {
        pins.clear();
        links.clear();
        plants.clear();
        routes.clear();
    }
    void Init()
    {
        //Clear();
        level = 0;
        ccPinID = 0;
    }

    uint8 GetLevel()                                    { return level; }
    uint32 GetPinID()                                   { return ccPinID; }

    uint8 level;
    uint32 ccPinID;

    // pinID, pinData
    std::map<uint32, PI_Pin> pins;
    // linkID, linkData
    std::map<uint32, PI_Link> links;
    // routeID, routeData
    std::map<uint16, PI_Route> routes;
    // plantPinID, plantData   - this dynamic data is not saved
    std::map<uint32, PI_Plant> plants;
};

/*  these are internal client state events
enum PlanetEvents {
    EVENT_NORMAL = 0,
    EVENT_BUILDPIN = 1,
    EVENT_CREATELINKSTART = 2,
    EVENT_CREATELINKEND = 3,
    EVENT_CREATEROUTE = 4,
    EVENT_SURVEY = 5,
    SUBEVENT_NORMAL = 6,
    SUBEVENT_MOVEEXTRACTIONHEAD = 7
}; */

                    /*
            Extractors = 1026,
            Command_Centers = 1027,
            Processors = 1028,
            Storage_Facilities = 1029,
            Spaceports = 1030,
            Planetary_Resources = 1031,
            Planet_Solid = 1032,
            Planet_Liquid_Gas = 1033,
            Refined_Commodities = 1034,
            Planet_Organic = 1035,
            Planetary_Links = 1036,
            Specialized_Commodities = 1040,
            Advanced_Commodities = 1041,
            Basic_Commodities = 1042,
            */
/*
     COMMAND_CREATEPIN: 'CreatePin',
     COMMAND_REMOVEPIN: 'RemovePin',
     COMMAND_CREATELINK: 'CreateLink',
     COMMAND_REMOVELINK: 'RemoveLink',
     COMMAND_SETLINKLEVEL: 'SetLinkLevel',
     COMMAND_CREATEROUTE: 'CreateRoute',
     COMMAND_REMOVEROUTE: 'RemoveRoute',
     COMMAND_SETSCHEMATIC: 'SetSchematic',
     COMMAND_UPGRADECOMMANDCENTER: 'UpgradeCommandCenter',
     COMMAND_ADDEXTRACTORHEAD: 'AddExtractorHead',
     COMMAND_KILLEXTRACTORHEAD: 'KillExtractorHead',
     COMMAND_MOVEEXTRACTORHEAD: 'MoveExtractorHead',
     COMMAND_INSTALLPROGRAM: 'InstallProgram'}
*/
/*
   __identifiers__ = {COMMAND_CREATEPIN: ['pinID'],
     COMMAND_REMOVEPIN: ['pinID'],
     COMMAND_CREATELINK: ['endpoint1', 'endpoint2'],
     COMMAND_REMOVELINK: ['endpoint1', 'endpoint2'],
     COMMAND_SETLINKLEVEL: ['endpoint1', 'endpoint2', 'level'],
     COMMAND_CREATEROUTE: ['routeID'],
     COMMAND_REMOVEROUTE: ['routeID'],
     COMMAND_SETSCHEMATIC: ['pinID'],
     COMMAND_UPGRADECOMMANDCENTER: ['pinID', 'level'],
     COMMAND_ADDEXTRACTORHEAD: ['pinID', 'headID'],
     COMMAND_KILLEXTRACTORHEAD: ['pinID', 'headID'],
     COMMAND_MOVEEXTRACTORHEAD: ['pinID', 'headID'],
     COMMAND_INSTALLPROGRAM: ['pinID']}

   __arguments__ = {COMMAND_CREATEPIN: ['typeID', 'latitude', 'longitude'],
     COMMAND_REMOVEPIN: [],
     COMMAND_CREATELINK: ['level'],
     COMMAND_REMOVELINK: [],
     COMMAND_SETLINKLEVEL: [],
     COMMAND_CREATEROUTE: ['path', 'typeID', 'quantity'],
     COMMAND_REMOVEROUTE: [],
     COMMAND_SETSCHEMATIC: ['schematicID'],
     COMMAND_UPGRADECOMMANDCENTER: [],
     COMMAND_ADDEXTRACTORHEAD: ['latitude', 'longitude'],
     COMMAND_KILLEXTRACTORHEAD: [],
     COMMAND_MOVEEXTRACTORHEAD: ['latitude', 'longitude'],
     COMMAND_INSTALLPROGRAM: ['typeID', 'headRadius']}
*/
/*
            info.proximity = const.planetResourceProximityPlanet
            for i, scanRange in enumerate(const.planetResourceScanningRanges):
                if scanRange >= dist:
                    info.proximity = i
        minBand, maxBand = const.planetResourceProximityLimits[info.proximity]
        info.newBand = min(maxBand, minBand + info.planetology + info.advancedPlanetology * 2)
        requiredSkill = 5 - info.proximity

 * planetResourceScanDistance = 1000000000
 * planetResourceProximityDistant = 0
 * planetResourceProximityRegion = 1
 * planetResourceProximityConstellation = 2
 * planetResourceProximitySystem = 3
 * planetResourceProximityPlanet = 4
 * planetResourceProximityLimits = [(2, 6),
 * (4, 10),
 * (6, 15),
 * (10, 20),
 * (15, 30)]
 * planetResourceScanningRanges = [9.0,
 * 7.0,
 * 5.0,
 * 3.0,
 * 1.0]
 * planetResourceUpdateTime = 1 * HOUR
 * planetResourceMaxValue = 1.21
 */

#endif  // EVE_PLANET_H
