/*
 * POD_continers.h
 *
 *    -allan  1Aug16
 */

#ifndef _EVEMU_POD_CONTAINERS_H_
#define _EVEMU_POD_CONTAINERS_H_

#include "eve-server.h"

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
    uint32 fleetID;
    uint32 wingID;
    uint32 squadID;
    uint8 fleetRole;
    uint8 fleetBooster;
    uint8 fleetJob;
};

/* POD structure for character kill data  -allan 01May16 */
struct CharKillData {
    uint32 killID;
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
    std::string dungeonName;
    uint32 systemID;
    uint32 sigItemID;   // itemID of this entry
    uint16 typeID;
    uint16 groupID;
    uint16 scanGroupID; // see below
    uint16 strengthAttributeID; // see below
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
    uint32          ownerID = 0;
    uint32          corpID = 0;
    uint32          locID = 0;
};

/* POD structure for container faction data */
struct FactionData {
    uint32 ownerID = 0;
    uint32 corporationID = 0;
    uint32 allianceID = 0;
    uint32 factionID = 0;
};

class DBSystemEntity {
public:
    uint32 itemID = 0;
    uint32 typeID = 0;
    uint32 groupID = 0;
    uint32 categoryID = 0;  //TODO populate this for simple system entities. (recently added - 1Dec15)
    uint32 orbitID = 0;
    GPoint position = NULL_ORIGIN;
    double radius = 0;
    double security = 0;
    std::string itemName = "";
};

class DBSystemDynamicEntity {
public:
    uint32 itemID = 0;
    std::string itemName = "";
    uint32 typeID = 0;
    uint32 groupID = 0;
    uint32 categoryID = 0;
    uint32 ownerID = 0;
    uint32 corporationID = 0;
    uint32 allianceID = 0;
    uint32 factionID = 0;
    uint32 planetID = 0;
    double x = 0;
    double y = 0;
    double z = 0;
};

struct DBGPointEntity {
    uint8 idx = 0;
    uint32 itemID = 0;
    double radius = 0;
    GPoint position = NULL_ORIGIN;
    double x = 0;
    double y = 0;
    double z = 0;
};

/* POD structure entries for dungeon data */
struct DunTemplate {
    std::string dunName = "";
    uint16 dunRoomID = 0;
    uint16 dunEntryID = 0;
    uint8 dunTypeID = 0;
    uint8 dunSpawnType = 0;
    uint8 dunRooms = 0;
    uint8 dunRoomTypeID = 0;
    uint8 dunRoomCategoryID = 0;
};

struct DunRoomInfo {
    uint16 dunRoomID = 0;
    uint8 dunRoomType = 0;
    uint8 dunRoomCategory = 0;
    uint8 dunRoomSpawnID = 0;
    uint8 dunRoomSpawnType = 0;
};

struct DunRoomData {
    uint32 dunGroupID = 0;
    uint16 x = 0;
    uint16 y = 0;
    uint16 z = 0;
};

struct DunGroupData {
    uint32 typeID = 0;
    std::string typeName = "";
    uint32 typeGrpID = 0;   // this is groupID of the itemType, and needed to simplify create/spawn code
    uint8 typeCatID = 0;    // this is categoryID of the itemType, and needed to simplify create/spawn code
    uint16 x = 0;
    uint16 y = 0;
    uint16 z = 0;
};

struct DunRoomSpawnInfo {
    uint16 dunRoomSpawnID = 0;
    uint16 dunRoomSpawnType = 0;
    uint16 x = 0;
    uint16 y = 0;
    uint16 z = 0;
};

/* POD structure entries for PI data */
struct PlanetResourceData {
    float dist_1 = 0;
    float dist_2 = 0;
    float dist_3 = 0;
    float dist_4 = 0;
    float dist_5 = 0;
    int32 type_1 = 0;
    int32 type_2 = 0;
    int32 type_3 = 0;
    int32 type_4 = 0;
    int32 type_5 = 0;
    std::string buffer_1 = "";
    std::string buffer_2 = "";
    std::string buffer_3 = "";
    std::string buffer_4 = "";
    std::string buffer_5 = "";
};

struct PI_Link {
    int8 state = 0;
    uint16 level = 0;
    uint16 typeID = 0;
    uint32 endpoint1 = 0;
    uint32 endpoint2 = 0;
};

struct PI_Route {
    int8 state = 0;
    int8 priority = 0;
    uint32 srcPinID = 0;
    uint32 destPinID = 0;
    uint16 commodityTypeID = 0;
    uint16 commodityQuantity = 0;
    std::list<uint32> path;
};

struct PI_Heads {
    uint16 typeID = 0;
    uint32 ecuPinID = 0;
    double latitude = 0.0f;
    double longitude = 0.0f;
};

struct PI_Schematic {
    uint8 outputQty = 0;
    uint16 outputType = 0;
    uint32 cycleTime = 0;

    // typeID, qty
    std::map<uint16, uint16> inputs;
};

struct PI_Plant {
    // specifically for processing plants. this is not saved in db
    PI_Schematic data;
    int8 state = 0;
    uint8 order = 0;
    uint16 schematicID = 0;
    uint16 qtyPerCycle = 0;
    uint64 cycleTime = 0;
    uint64 expiryTime = 0;
    uint64 installTime = 0;
    uint64 lastRunTime = 0;

    bool hasReceivedInputs = false;
    bool receivedInputsLastCycle = false;
};

/* optimize this after everything is working!!  */
class PI_Pin {
public:
    bool isCommandCenter = false;
    bool isStorage = false;
    bool isConsumer = false;
    bool isLaunchable = false;
    bool isProcess = false;
    bool isBase = false;
    bool isECU = false;

    // common for all pins
    int8 state = 0;
    uint16 level = 0;
    uint16 typeID = 0;
    uint32 ownerID = 0;
    uint64 lastRunTime = 0;

    double latitude = 0.0f;
    double longitude = 0.0f;

    // Command/Spaceport
    uint64 lastLaunchTime = 0;

    //ExtractorControlUnit
    std::map<uint16, PI_Heads> heads;
    float headRadius = 0.0f;

    // Process and ECU
    bool hasReceivedInputs = false;
    bool receivedInputsLastCycle = false;
    uint16 schematicID = 0;   // used in ecu as extractor head typeID
    uint16 programType = 0;      // used in extractors as extracted resource typeID
    uint16 qtyPerCycle = 0;
    uint64 cycleTime = 0;
    uint64 expiryTime = 0;
    uint64 installTime = 0;

    // Storage    typeID, qty
    std::map<uint16, uint32> contents;

    // specifically for updating contents. this is not saved in db
    bool update = false;
    float capacity = 0;  // this is not implemented yet
};

class PI_CCPin {
public:
    uint8 level = 0;
    uint32 ccPinID = 0;
    uint64 currentSimTime = 0;

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