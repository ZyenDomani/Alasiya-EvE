/*
 * POD_continers.h
 *
 *    -allan  1Aug16
 */

#ifndef _EVEMU_POD_CONTAINERS_H_
#define _EVEMU_POD_CONTAINERS_H_

#include "eve-server.h"


/* POD structure for certificate data */
struct CharCerts {
    uint8 visibilityFlags;
    uint32 certificateID;
    int64 grantDate;
};
typedef std::vector<CharCerts> CertVector;

/* POD structure for skills in queue */
struct QueuedSkill {
    uint8 level;
    uint16 typeID;
};
typedef std::vector<QueuedSkill> SkillQueue;

/* POD structure for saving attribute data */
struct AttrData {
    uint16 attrID;
    uint32 itemID;
    uint32 valueInt;
    double valueFloat;
};

/* POD structure for account data */
struct AccountData {
    bool online:1;
    bool banned:1;
    int32 id;
    int64 role;
    int32 visits;
    int64 clientID;
    std::string name;
    std::string hash;
    std::string password;
    std::string last_login;
};

/* POD structure for character data */
struct CharacterData {
    bool gender:1;
    uint8 flag;
    uint8 bloodlineID;
    uint8 raceID;
    uint16 typeID;
    uint32 accountID;
    uint32 shipID;
    uint32 capsuleID;
    uint32 logonMinutes;
    uint32 locationID;
    uint32 stationID;
    uint32 solarSystemID;
    uint32 constellationID;
    uint32 regionID;
    uint32 ancestryID;
    uint32 careerID;
    uint32 schoolID;
    uint32 careerSpecialityID;
    int64 loginTime;
    int64 createDateTime;
    double bounty;
    double balance;
    double aurBalance;
    double securityRating;
    double skillPoints;
    std::string name;
    std::string title;
    std::string description;
};

/* POD structure for corp data */
struct CorpData {
    int16 corpAccountKey;
    uint32 corporationID;
    uint32 corpHQ;
    uint32 baseID;
    int32 allianceID;
    int32 warFactionID;
    int64 startDateTime;
    int64 corpRole;
    int64 rolesAtAll;
    int64 rolesAtBase;
    int64 rolesAtHQ;
    int64 rolesAtOther;
    int64 grantableRoles;
    int64 grantableRolesAtBase;
    int64 grantableRolesAtHQ;
    int64 grantableRolesAtOther;
    double taxRate;
    std::string name;
    std::string ticker;
};

/* POD structure for corp office data */
struct OfficeData {
    bool lockDown;
    uint32 officeID;
    uint32 folderID;
    uint16 typeID;
    uint32 stationID;
    uint32 corporationID;
    int64 rentalFee;
    int64 expiryTime;
    std::string ticker;
};

/* POD structure for corp app data */
struct ApplicationInfo {
    bool valid;
    uint32 appID;
    uint32 corpID;
    uint32 charID;
    uint32 status;
    uint32 deleted;
    uint32 lastCID;
    int64 appTime;
    int64 role;
    int64 grantRole;
    std::string appText;
};

/* POD structure for fleet data    -allan 31Jul14 */
struct CharFleetData {
    int8 job;
    int8 role;
    int8 booster;
    int32 fleetID;
    int32 wingID;
    int32 squadID;
    int64 joinTime;
};

/* POD structure for bounty timer data  */
struct BountyData {
    uint8 refTypeID;
    uint16 fromKey;
    uint16 toKey;
    uint32 fromID;
    uint32 toID;
    double amount;
    std::string reason;
};

/* POD structure for blueprint data */
struct BlueprintData {
    bool copy:1;
    int32 mLevel;
    int32 pLevel;
    int32 runs;
};

/* POD structure for blueprint type data */
struct BlueprintTypeData {
    int8 catID;
    int8 techLevel;
    uint16 wasteFactor;
    uint16 productTypeID;
    uint16 parentBlueprintTypeID;
    uint32 productionTime;
    uint32 researchProductivityTime;
    uint32 researchMaterialTime;
    uint32 researchCopyTime;
    uint32 researchTechTime;
    uint32 productivityModifier;
    uint32 materialModifier;
    uint32 maxProductionLimit;
    double chanceOfReverseEngineering;
};


/* POD structure for character kill data  -allan 01May16 */
struct CharKillData {
    uint16 victimShipTypeID;
    uint16 finalShipTypeID;
    uint16 finalWeaponTypeID;
    int32 finalAllianceID;
    int32 victimAllianceID;
    int32 victimFactionID;
    int32 finalFactionID;
    uint32 solarSystemID;
    uint32 victimCharacterID;
    uint32 victimCorporationID;
    uint32 victimDamageTaken;
    uint32 finalCharacterID;
    uint32 finalCorporationID;
    uint32 finalDamageDone;
    uint32 moonID;
    int64 killTime;
    double finalSecurityStatus;
    std::string killBlob;
};

/* POD structure for asteroid */
struct AsteroidData {
    uint16 typeID;
    uint32 itemID;
    uint32 systemID;
    uint32 beltID;
    double quantity;
    double radius;
    double x;
    double y;
    double z;
    std::string itemName;
};

/* POD structure for asteroid distrubtion methods by group */
struct OreTypeChance {
    uint16 typeID;
    float chance;
};

/* POD structure for active dungeon */
struct ActiveDungeon {
    uint8 state;
    uint32 systemID;
    uint32 dunItemID;
    uint32 dunTemplateID;
    int64 dunExpiryTime;
    double x;
    double y;
    double z;
};

/* POD structure for cosmic signatures/anomalies */
struct CosmicSignature {
    uint8 dungeonType;
    uint16 sigTypeID;
    uint16 sigGroupID;
    uint16 scanGroupID;
    uint16 scanAttributeID;
    uint32 ownerID;
    uint32 systemID;
    uint32 sigItemID;   // itemID of this entry
    float sigStrength;
    double x;
    double y;
    double z;
    std::string sigID;  // this is unique xxx-nnn id displayed in scanner
    std::string sigName;
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
    uint8 quantity; //quantity to spawn for this typeID
    uint16 typeID;  //typeID to spawn
};

/* POD structure for spawn entries */
struct SpawnEntry {     // notes for me while creating/writing/testing
    bool enabled;       // is respawn enabled for this entry?  also provides conditional test for SpawnMgr::IsChaining() method
    uint8 spawnClass;   // spawn class.  0 = none, 1-7 = easy to insance based on sysSec, 8 = hauler, 9 = commander, 10 = officer  - 20+ are anomalies
    uint8 spawnGroup;   // spawn group.   1 = roaming, 2 = static, 3 = anomaly, 4 = combat, 5 = deadspace, 6 = mission, 7 = incursion, 8 = sleeper, 9 = escalation
    uint8 total;        // total number of this group spawned
    uint8 number;       // this rat's number in group (to match up with above total)
    uint8 level;        // spawn data subtype/wave
    uint8 classID;      // spawn data class id (in case we have to look it up again)
    uint16 typeID;      // rat type id
    uint16 groupID;     // rat group id (may look into changing typeID within group later on respawn (for chaining))
    uint16 spawnID;     // spawn id (if needed to match up with other spawns of this group (multiple spawn types in this group))
    uint16 stamp;       // entry stamp time to respawn (process conditional to allow for common timer and multiple respawn times)
    uint32 itemID;      // rat entity id
    uint32 corpID;      // rat corp id
    uint32 factionID;   // rat faction id
};

/* POD structure for spawn faction groups */
struct RatFactionGroups {  // notes for me while creating/writing/testing
    uint8 shipClass;      // shipClass as defined in Spawn::Class
    uint16 groupID;     // item groupID
};

/* POD structure for spawn classes */
struct RatSpawnClass { // notes for me while creating/writing/testing
    uint8 type;     // this is spawn type.  Spawn::Type 1 - 10
    uint8 sub;      // this is spawn subtype.  ship grouping.  varies.  enables loop for random pick. no notes
    uint8 f;        // frigate
    uint8 af;       // advanced frigate
    uint8 d;        // destroyer
    uint8 c;        // cruiser
    uint8 ac;       // advanced cruiser
    uint8 bc;       // battlecruiser
    uint8 bs;       // battleship
    uint8 h;        // hauler
    uint8 o;        // officer - swarm for rogue drones
    uint8 cf;       // commander frigate
    //uint8 acf;      // advanced commander frigate
    uint8 cd;       // commander destroyer
    uint8 cc;       // commander cruiser
    uint8 cbc;      // commander battlecruiser
    uint8 cbs;      // commander battleship
};

/* POD structure for loot groups */
struct LootGroup {
    //uint32 groupID;
    uint32 lootGroupID;
    double dropChance;
};

/* POD structure for loot types */
struct LootGroupType {
    uint8 metaLevel;
    uint32 lootGroupID;
    uint32 typeID;
    uint32 minQuantity;
    uint32 maxQuantity;
};

/* POD structure for possible loot drops */
struct LootList {
    uint8 minDrop;
    uint8 maxDrop;
    uint32 itemID;
};

/* POD structure for statistic data */
struct StatisticData {
    uint16 span;        // 45.5d in minutes (max)
    uint16 shipsSalvaged;
    uint16 probesLaunched;
    uint16 sitesScanned;
    uint16 ramJobs;
    uint32 pcShots;
    uint32 pcMissiles;
    double pcBounties;
    double npcBounties;
    double oreMined;
    double iskMarket;
};


/* POD structure for systems. */
struct SystemData {
    uint32 systemID;
    uint32 constellationID;
    uint32 regionID;
    float securityRating;
    std::string name;
    std::string securityClass;
};

/* POD structure for static items. */
struct StaticData {
    uint16 typeID;
    uint32 itemID;
    uint32 systemID;
    uint32 constellationID;
    uint32 regionID;
    GPoint position;
};

/* POD structure for stations. */
struct StationData {
    bool conquerable;
    uint8 officeSlots;
    uint8 operationID;
    uint16 typeID;
    uint16 graphicID;
    uint16 descriptionID;
    uint16 hangarGraphicID;
    uint16 dockingBayGraphicID;
    uint16 reprocessingHangarFlag;
    uint32 stationID;
    uint32 corporationID;
    uint32 maxShipVolumeDockable;
    uint32 officeRentalFee;
    uint32 systemID;
    uint32 constellationID;
    uint32 regionID;
    uint32 orbitID;
    int64 serviceMask;
    float radius;
    float security;
    float dockingCostPerVolume;
    float reprocessingEfficiency;
    float reprocessingStationsTake;
    GPoint position;
    GPoint dockEntry;
    GPoint dockPosition;
    GVector dockOrientation;
    std::string name;
    std::string description;
};

/* POD structure for saving items */
struct SaveData {
    EVEItemFlags    flag;
    bool            contraband;
    bool            singleton;
    uint16          typeID;
    uint32          itemID;
    uint32          ownerID;
    uint32          locationID;
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
    int32 allianceID;
    int32 factionID;
    uint32 ownerID;
    uint32 corporationID;
};

/* structure for type attributes */
struct DmgTypeAttribute {
    uint16 attributeID;
    EvilNumber value;
};

/* structure for loading static system items */
struct DBSystemEntity {
    uint16 typeID;
    uint16 groupID;
    uint32 itemID;
    double radius;
};

struct DBSystemDynamicEntity {
    EVEItemCategories categoryID;
    uint16 typeID;
    uint16 groupID;
    int32 allianceID;
    int32 factionID;
    uint32 itemID;
    uint32 ownerID;
    uint32 corporationID;
    uint32 planetID;
    double x;
    double y;
    double z;
    std::string itemName;
};

struct DBGPointEntity {
    uint8 idx;
    uint32 itemID;
    double radius;
    double x;
    double y;
    double z;
    GPoint position;
};

/* POD structure entries for dungeon data */
struct DunTemplate {
    uint8 dunTypeID;
    uint8 dunSpawnClass;
    uint16 dunEntryID;
    int32 dunRoomID;
    std::string dunName;
};

struct DunRoomInfo {
    uint8 dunRoomType;
    uint8 dunRoomCategory;
    uint8 dunRoomSpawnID;
    uint8 dunRoomSpawnType;
    uint16 dunRoomID;
};

struct DunRoomData {
    int16 x;
    int16 y;
    int16 z;
    int32 dunGroupID;
};

struct DunGroupData {
    uint8 typeCatID;    // this is categoryID of the itemType, and needed to simplify create/spawn code
    int16 typeID;
    int16 typeGrpID;   // this is groupID of the itemType, and needed to simplify create/spawn code
    int16 x;
    int16 y;
    int16 z;
    uint16 radius;
    std::string typeName;
};

struct DunRoomSpawnInfo {
    int16 x;
    int16 y;
    int16 z;
    uint16 dunRoomSpawnID;
    uint16 dunRoomSpawnType;
};

struct DunEntryData {
    int16 x;
    int16 y;
    int16 z;
    uint16 dunEntryID;
};

/* POD structure for decoded probe data */
struct ProbeData {
    uint8 state;
    uint8 rangeStep;
    int64 expiry;
    float scanRange;
    GPoint dest;
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
    int64 cycleTime;
    int64 expiryTime;
    int64 installTime;
    int64 lastRunTime;

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
    int64 lastRunTime;

    double latitude;
    double longitude;

    // Command/Spaceport
    int64 lastLaunchTime;

    //ExtractorControlUnit
    std::map<uint16, PI_Heads> heads;
    float headRadius;

    // Process and ECU
    bool hasReceivedInputs : 1;
    bool receivedInputsLastCycle : 1;
    uint16 schematicID;   // used in ecu as extractor head typeID
    uint16 programType;      // used in extractors as extracted resource typeID
    uint16 qtyPerCycle;
    int64 cycleTime;
    int64 expiryTime;
    int64 installTime;

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
    int64 currentSimTime;

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