/*
 * POD_continers.h
 *
 *    -allan  1Aug16
 */

#ifndef _EVEMU_POD_CONTAINERS_H_
#define _EVEMU_POD_CONTAINERS_H_

#include "eve-server.h"


/* POD structure for saving attribute data */
struct AttrData {
    uint32 itemID;
    uint16 attrID;
    uint32 valueInt;
    double valueFloat;
};

/* POD structure for account data */
struct AccountData {
    int32 id;
    uint64 role;
    int32 visits;
    int32 clientID;
    std::string name;
    std::string hash;
    std::string password;
    std::string last_login;
    bool online;
    bool banned;
};

/* POD structure for fleet data    -allan 31Jul14 */
struct FleetData {
    uint8 fleetRole;
    uint8 fleetBooster;
    uint8 fleetJob;
    uint32 fleetID;
    uint32 wingID;
    uint32 squadID;
};

/* POD structure for blueprint data */
struct BlueprintData {
    bool copy;
    int32 mLevel;
    int32 pLevel;
    int32 runs;
};

/* POD structure for blueprint type data */
struct BlueprintTypeData {
    uint32 parentBlueprintTypeID;
    uint32 productTypeID;
    uint32 productionTime;
    uint32 techLevel;
    uint32 researchProductivityTime;
    uint32 researchMaterialTime;
    uint32 researchCopyTime;
    uint32 researchTechTime;
    uint32 productivityModifier;
    uint32 materialModifier;
    uint32 maxProductionLimit;
    double wasteFactor;
    double chanceOfReverseEngineering;
};

/* POD structure for blueprint ram requirements */
struct ramRequirements {
    bool recycle;
    uint8 activityID;
    uint16 requiredTypeID;
    uint32 quantity;
    float damagePerJob;
};

/* POD structure for blueprint item materials  */
struct ramMaterials {
    uint16 materialTypeID;
    uint32 quantity;
};

/* POD structure for character kill data  -allan 01May16 */
struct CharKillData {
    uint32 solarSystemID;
    uint32 victimCharacterID;
    uint32 victimCorporationID;
    uint32 victimAllianceID;
    uint32 victimFactionID;
    uint16 victimShipTypeID;
    uint32 victimDamageTaken;
    uint32 finalCharacterID;
    uint32 finalCorporationID;
    uint32 finalAllianceID;
    uint32 finalFactionID;
    uint16 finalShipTypeID;
    uint16 finalWeaponTypeID;
    double finalSecurityStatus;
    uint32 finalDamageDone;
    std::string killBlob;
    uint64 killTime;
    uint32 moonID;
};

/* POD structure for asteroid */
struct AsteroidData {
    uint32 itemID;
    std::string itemName;
    uint32 typeID;
    uint32 systemID;
    uint32 beltID;
    double quantity;
    double radius;
    double x;
    double y;
    double z;
};

/* POD structure for asteroid distrubtion methods by group */
struct OreTypeChance {
    uint16 typeID;
    float chance;
};

/* POD structure for active dungeon */
struct ActiveDungeon {
    uint32 systemID;
    uint32 dunItemID;
    uint16 dunTemplateID;
    uint64 dunExpiryTime;
    uint8 state;
    double x;
    double y;
    double z;
};

/* POD structure for cosmic signatures/anomalies */
struct CosmicSignature {
    std::string sigID;  // this is unique xxx-nnn id displayed in scanner
    std::string sigName;
    uint32 ownerID;
    uint32 systemID;
    uint32 sigItemID;   // itemID of this entry
    uint8 dungeonType;
    uint16 sigTypeID;
    uint16 sigGroupID;
    uint16 scanGroupID;
    uint16 scanAttributeID;
    double x;
    double y;
    double z;
};

/* POD structure for spawn groups */
struct SystemSpawnGroup { //reference to this bubble's data for spawn groups.  may need later.
    //SystemBubble* pSysBubble;   //cant use reference or pointer here...
    uint32 bubbleID;
    uint32 systemID;
    uint32 regionID;
    double secRating;
};

/* POD structure for spawn groups */
struct SpawnGroup {
    uint32 typeID;  //typeID to spawn
    uint8 quantity; //quantity to spawn for this typeID
};

/* POD structure for spawn entries */
struct SpawnEntry {     // notes for me while creating/writing/testing
    bool enabled;       // is group timer enabled for this entry?
    uint8 spawnType;    // spawn type.  1 = roaming, 2 = static
    uint8 total;        // total number of this group spawned
    uint8 number;       // this rats number in group (to match up with above total)
    uint8 sub;          // spawn data subtype
    uint8 type;         // spawn data class id (in case we have to look it up again)
    uint16 typeID;      // rat type id
    uint32 itemID;      // rat entity id
    uint32 groupID;     // rat group id (may look into changing typeID within group later on respawn (for chaining))
    uint32 corpID;      // rat corp id
    uint32 factionID;   // rat faction id
    uint32 spawnID;     // spawn id (if needed to match up with other spawns of this group (multiple spawn types in this group))
    uint32 time;        // spawn group timer start time
};

/* POD structure for spawn faction groups */
struct RatFactionGroups {  // notes for me while creating/writing/testing
    uint8 shipClass;      // shipclass - arbitrary
    uint32 groupID;     // item groupID
};

/* POD structure for spawn classes */
struct RatSpawnClass { // notes for me while creating/writing/testing
    uint8 type;     // this is spawn type.  see notes in SpawnDB.h
    uint8 sub;      // this is spawn class id.  see notes in SpawnDB.h
    uint8 f;        // frigate
    uint8 d;        // destroyer
    uint8 c;        // cruiser
    uint8 bc;       // battlecruiser
    uint8 bs;       // battleship
    uint8 h;        // hauler
    uint8 o;        // officer - swarm for rogue drones
    uint8 cf;       // commander frigate
    uint8 cd;       // commander destroyer
    uint8 cc;       // commander cruiser
    uint8 cbc;      // commander battlecruiser
    uint8 cbs;      // commander battleship
};


/* POD structure for systems. */
struct SystemData {
    uint32 systemID;
    std::string name;
    uint32 constellationID;
    uint32 regionID;
    float securityRating;
    std::string securityClass;
};

/* POD structure for static items. */
struct StaticData {
    uint32 itemID;
    uint32 systemID;
    uint32 constellationID;
    uint32 regionID;
    GPoint position;
};

/* POD structure for stations. */
struct StationData {
    uint32 stationID;
    GPoint position;
    GPoint dockPosition;
    GVector dockOrientation;
};

/* POD structure for saving items */
struct SaveData {
    uint32          itemID;
    uint32          typeID;
    uint32          ownerID;
    uint32          locationID;
    EVEItemFlags    flag;
    bool            contraband;
    bool            singleton;
    uint32          quantity;
    GPoint          position;
    std::string     customInfo;
};

/* POD structure for loading dynamic items */
struct OwnerData {
    uint32          ownerID;
    uint32          corpID;
    uint32          locID;
};

/* POD structure for container faction data */
struct FactionData {
    uint32 ownerID;
    uint32 corporationID;
    uint32 allianceID;
    uint32 factionID;
};

/* structure for type attributes */
struct DmgTypeAttribute {
    uint16 attributeID;
    EvilNumber value;
};

/* structure for loading static system items */
struct DBSystemEntity {
    uint32 itemID;
    uint32 typeID;
    uint16 groupID;
    double radius;
};

struct DBSystemDynamicEntity {
    uint32 itemID;
    std::string itemName;
    uint32 typeID;
    uint16 groupID;
    EVEItemCategories categoryID;
    uint32 ownerID;
    uint32 corporationID;
    uint32 allianceID;
    uint32 factionID;
    uint32 planetID;
    double x;
    double y;
    double z;
};

struct DBGPointEntity {
    uint8 idx;
    uint32 itemID;
    double radius;
    GPoint position;
    double x;
    double y;
    double z;
};

/* POD structure entries for dungeon data */
struct DunTemplate {
    std::string dunName;
    uint16 dunRoomID;
    uint16 dunEntryID;
    uint8 dunTypeID;
    uint8 dunSpawnType;
    uint8 dunRooms;
    uint8 dunRoomTypeID;
    uint8 dunRoomCategoryID;
};

struct DunRoomInfo {
    uint16 dunRoomID;
    uint8 dunRoomType;
    uint8 dunRoomCategory;
    uint8 dunRoomSpawnID;
    uint8 dunRoomSpawnType;
};

struct DunRoomData {
    uint32 dunGroupID;
    uint16 x;
    uint16 y;
    uint16 z;
};

struct DunGroupData {
    uint32 typeID;
    std::string typeName;
    uint32 typeGrpID;   // this is groupID of the itemType, and needed to simplify create/spawn code
    uint8 typeCatID;    // this is categoryID of the itemType, and needed to simplify create/spawn code
    uint16 x;
    uint16 y;
    uint16 z;
};

struct DunRoomSpawnInfo {
    uint16 dunRoomSpawnID;
    uint16 dunRoomSpawnType;
    uint16 x;
    uint16 y;
    uint16 z;
};

/* POD structure entries for PI data */
struct PlanetResourceData {
    float dist_1;
    float dist_2;
    float dist_3;
    float dist_4;
    float dist_5;
    int32 type_1;
    int32 type_2;
    int32 type_3;
    int32 type_4;
    int32 type_5;
    std::string buffer_1;
    std::string buffer_2;
    std::string buffer_3;
    std::string buffer_4;
    std::string buffer_5;
};

struct PI_Link {
    int8 state;
    uint16 level;
    uint16 typeID;
    uint32 endpoint1;
    uint32 endpoint2;
};

struct PI_Route {
    int8 state;
    int8 priority;
    uint32 srcPinID;
    uint32 destPinID;
    uint16 commodityTypeID;
    uint16 commodityQuantity;
    std::list<uint32> path;
};

struct PI_Heads {
    uint16 typeID;
    uint32 ecuPinID;
    double latitude;
    double longitude;
};

struct PI_Schematic {
    uint8 outputQty;
    uint16 outputType;
    uint32 cycleTime;

    // typeID, qty
    std::map<uint16, uint16> inputs;
};

struct PI_Plant {
    // specifically for processing plants. this is not saved in db
    PI_Schematic data;
    int8 state;
    uint8 order;
    uint16 schematicID;
    uint16 qtyPerCycle;
    uint64 cycleTime;
    uint64 expiryTime;
    uint64 installTime;
    uint64 lastRunTime;

    bool hasReceivedInputs;
    bool receivedInputsLastCycle;
};

/* optimize this after everything is working!!  */
class PI_Pin {
public:
    bool isCommandCenter : 1;
    bool isStorage : 1;
    bool isConsumer : 1;
    bool isLaunchable : 1;
    bool isProcess : 1;
    bool isBase : 1;
    bool isECU : 1;

    // common for all pins
    int8 state;
    uint16 level;
    uint16 typeID;
    uint32 ownerID;
    uint64 lastRunTime;

    double latitude;
    double longitude;

    // Command/Spaceport
    uint64 lastLaunchTime;

    //ExtractorControlUnit
    std::map<uint16, PI_Heads> heads;
    float headRadius;

    // Process and ECU
    bool hasReceivedInputs : 1;
    bool receivedInputsLastCycle : 1;
    uint16 schematicID;   // used in ecu as extractor head typeID
    uint16 programType;      // used in extractors as extracted resource typeID
    uint16 qtyPerCycle;
    uint64 cycleTime;
    uint64 expiryTime;
    uint64 installTime;

    // Storage    typeID, qty
    std::map<uint16, uint32> contents;

    // specifically for updating contents. this is not saved in db
    bool update : 1;
    float capacity;  // this is not implemented yet
};

class PI_CCPin {
public:
    uint8 level;
    uint32 ccPinID;
    uint64 currentSimTime;

    // pinID, pinData
    std::map<uint32, PI_Pin> pins;
    // linkID, linkData
    std::map<uint32, PI_Link> links;
    // routeID, routeData
    std::map<uint16, PI_Route> routes;
    // plantPinID, plantData   - this dynamic data is not saved
    std::map<uint32, PI_Plant> plants;
};

#endif  // _EVEMU_POD_CONTAINERS_H_