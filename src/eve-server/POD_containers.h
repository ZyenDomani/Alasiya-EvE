/*
 * POD_continers.h
 *
 *    -allan  1Aug16
 */

#ifndef _EVEMU_POD_CONTAINERS_H_
#define _EVEMU_POD_CONTAINERS_H_

#include "eve-server.h"

/** @todo most of these need their own namespace
 *     currently moving to specific eve-common/EVE_xxxx.h files
 */

/* POD structure for certificate data */
struct CharCerts {
    uint8 visibilityFlags;
    uint32 certificateID;
    int64 grantDate;
};
typedef std::map<uint16, CharCerts> CertMap;

/* POD structure for account data */
struct AccountData {
    bool online:1;
    bool banned:1;
    uint8 type=0;
    int32 id=0;
    int32 visits=0;
    uint32 clientID=0;
    int64 role=0;
    std::string name = "none";
    std::string hash = "none";
    std::string password = "none";
    std::string last_login = "none";
};

/* POD structure for character data */
struct CharacterData {
    bool gender:1;
    uint8 flag=0;
    uint8 bloodlineID=0;
    uint8 raceID=0;
    uint16 typeID=0;
    uint32 accountID=0;
    uint32 shipID=0;
    uint32 capsuleID=0;
    uint32 logonMinutes=0;
    uint32 locationID=0;
    uint32 stationID=0;
    uint32 solarSystemID=0;
    uint32 constellationID=0;
    uint32 regionID=0;
    uint32 ancestryID=0;
    uint32 careerID=0;
    uint32 schoolID=0;
    uint32 careerSpecialityID=0;
    uint32 skillPoints=0;
    int64 loginTime=0;
    int64 createDateTime=0;
    float bounty=0.0f;
    float balance=0.0f;
    float aurBalance=0.0f;
    float securityRating=0.0f;
    std::string name = "none";
    std::string title = "none";
    std::string description = "none";
};

/* POD structure for corp data */
struct CorpData {
    int16 corpAccountKey=0;
    int32 allianceID=0;
    int32 warFactionID=0;
    uint32 corporationID=0;
    uint32 corpHQ=0;
    uint32 baseID=0;
    int64 startDateTime=0;
    int64 corpRole=0;
    int64 rolesAtAll=0;
    int64 rolesAtBase=0;
    int64 rolesAtHQ=0;
    int64 rolesAtOther=0;
    int64 grantableRoles=0;
    int64 grantableRolesAtBase=0;
    int64 grantableRolesAtHQ=0;
    int64 grantableRolesAtOther=0;
    float taxRate=0.0f;
    float secRating=0.0f;
    std::string name = "none";
    std::string ticker = "none";
};

/* POD structure for fleet data    -allan 31Jul14 */
struct CharFleetData {
    int8 job=0;
    int8 role=0;
    int8 booster=0;
    int32 fleetID=0;
    int32 wingID=0;
    int32 squadID=0;
    int64 joinTime=0;
};

/* POD structure for bounty timer data  */
struct BountyData {
    uint8 refTypeID=0;
    uint16 fromKey=0;
    uint16 toKey=0;
    uint32 fromID=0;
    uint32 toID=0;
    float amount=0.0f;
    std::string reason = "none";
};


/* POD structure for kill data  -allan 01May16 */
struct KillData {
    uint16 victimShipTypeID=0;
    uint16 finalShipTypeID=0;
    uint16 finalWeaponTypeID=0;
    int32 finalAllianceID=0;
    int32 victimAllianceID=0;
    int32 victimFactionID=0;
    int32 finalFactionID=0;
    uint32 solarSystemID=0;
    uint32 victimCharacterID=0;
    uint32 victimCorporationID=0;
    uint32 victimDamageTaken=0;
    uint32 finalCharacterID=0;
    uint32 finalCorporationID=0;
    uint32 finalDamageDone=0;
    uint32 moonID=0;
    int64 killTime=0;
    double finalSecurityStatus=0.0;
    std::string killBlob = "none";
};

/* POD structure for asteroid */
struct AsteroidData {
    uint16 typeID=0;
    uint32 itemID=0;
    uint32 systemID=0;
    uint32 beltID=0;
    double quantity=0.0;
    double radius=0.0;
    GPoint position = NULL_ORIGIN;
    std::string itemName = "none";
};

/* POD structure for asteroid distribution methods by group */
struct OreTypeChance {
    uint16 typeID=0;
    float chance=0.0f;
};

/* POD structure for cosmic signatures/anomalies */
struct CosmicSignature {
    uint8 dungeonType=0;          // internal for creation checks
    uint16 bubbleID=0;            // internal for .siglist command
    // typeID of signal
    uint16 sigTypeID=0;           // type name if scanGroupID is not sig or anom and certainty > 0.75
    // groupID of signal
    uint16 sigGroupID=0;          // group name if scanGroupID is not sig or anom and certainty > 0.25
    // groupID of signature...must be one of sig, anom, ship, drone, structure
    uint16 scanGroupID=0;         // ship,drone and structure uses sigGroupID for group name
    uint16 scanAttributeID=0;     // group naming data if scanGroupID is anom or sig and certainty > 0.25
    uint32 ownerID=0;
    uint32 systemID=0;
    uint32 sigItemID=0;           // itemID of this entry
    float sigStrength=0.0f;
    GPoint position=NULL_ORIGIN;
    std::string sigID="none";          // this is unique xxx-nnn id displayed in scanner.  can be other values
    std::string sigName="none";        // site name if scanGroupID is sig or anom and certainty > 0.75
};

/* POD structure for spawn groups */
struct SystemSpawnGroup { //reference to this bubble's data for spawn groups.  may need later.
    //SystemBubble* pSysBubble;   //cant use reference or pointer here...
    uint32 bubbleID=0;
    uint32 systemID=0;
    uint32 regionID=0;
    double secRating=0;
};

/* POD structure for spawn groups */
struct SpawnGroup {
    uint8 quantity=0; //quantity to spawn for this typeID
    uint16 typeID=0;  //typeID to spawn
};

/* POD structure for spawn entries */
struct SpawnEntry {     // this is a single entry for a particular spawn.  it is probably one of many
    bool enabled=false;   // is respawn enabled for this entry?  also provides conditional test for SpawnMgr::IsChaining() method
    uint8 spawnClass=0;   // spawn class.  0 = none, 1-7 = easy to insane based on sysSec, 8 = hauler, 9 = commander, 10 = officer  - 20+ are anomalies
    uint8 spawnGroup=0;   // spawn group.   1 = roaming, 2 = static, 3 = anomaly, 4 = combat, 5 = deadspace, 6 = mission, 7 = incursion, 8 = sleeper, 9 = escalation
    uint8 total=0;        // total number of this group spawned
    uint8 number=0;       // this rat's number in group (to match up with above total)
    uint8 level=0;        // spawn data subtype/wave
    uint8 classID=0;      // spawn data class id (in case we have to look it up again)
    uint16 typeID=0;      // rat type id
    uint16 groupID=0;     // rat group id (may look into changing typeID within group later on respawn (for chaining))
    uint16 spawnID=0;     // spawn id (if needed to match up with other spawns of this group (multiple spawn types in this group))
    uint16 stamp=0;       // entry stamp time to respawn (process conditional to allow for common timer and multiple respawn times)
    uint32 itemID=0;      // rat entity id
    uint32 corpID=0;      // rat corp id
    uint32 factionID=0;   // rat faction id
};

/* POD structure for spawn faction groups */
struct RatFactionGroups {  // notes for me while creating/writing/testing
    uint8 shipClass=0;      // shipClass as defined in Spawn::Class
    uint16 groupID=0;     // item groupID
};

/* POD structure for spawn classes */
struct RatSpawnClass { // notes for me while creating/writing/testing
    uint8 type=0;     // this is spawn type.  Spawn::Type 1 - 10
    uint8 sub=0;      // this is spawn subtype.  ship grouping.  varies.  enables loop for random pick. no notes
    uint8 f=0;        // frigate
    uint8 af=0;       // advanced frigate
    uint8 d=0;        // destroyer
    uint8 c=0;        // cruiser
    uint8 ac=0;       // advanced cruiser
    uint8 bc=0;       // battlecruiser
    uint8 bs=0;       // battleship
    uint8 h=0;        // hauler
    uint8 o=0;        // officer - swarm for rogue drones
    uint8 cf=0;       // commander frigate
    //uint8 acf;      // advanced commander frigate
    uint8 cd=0;       // commander destroyer
    uint8 cc=0;       // commander cruiser
    uint8 cbc=0;      // commander battlecruiser
    uint8 cbs=0;      // commander battleship
    std::string desc; // defined ship counts
};

/* POD structure for loot groups */
struct LootGroup {
    //uint32 groupID;
    uint16 lootGroupID=0;
    float dropChance=0.0f;
};

/* POD structure for loot types */
struct LootGroupType {
    uint8 metaLevel=0;
    uint16 lootGroupID=0;
    uint16 typeID=0;
    uint32 minQuantity=0;
    uint32 maxQuantity=0;
};

/* POD structure for possible loot drops */
struct LootList {
    uint8 minDrop=0;
    uint8 maxDrop=0;
    uint16 typeID=0;
};
/* POD structure for statistic data */
struct StatisticData {
    bool changed=false;
    uint16 span=0;        // 45.5d in minutes (max)
    uint16 shipsSalvaged=0;
    uint16 probesLaunched=0;
    uint16 sitesScanned=0;
    uint16 ramJobs=0;
    uint32 pcShots=0;
    uint32 pcMissiles=0;
    double pcBounties=0;
    double npcBounties=0;
    double oreMined=0;
    double iskMarket=0;
};


/* POD structure for systems. */
struct SystemData {
    uint32 systemID=0;
    uint32 constellationID=0;
    uint32 regionID=0;
    uint32 factionID=0;
    int64 radius=0;
    float securityRating=0.0f;
    std::string name="none";
    std::string securityClass="none";
};

/* POD structure for solarsystem item. */
struct SolarSystemData {
    bool border=false;
    bool fringe=false;
    bool corridor=false;
    bool hub=false;
    bool international=false;
    bool regional=false;
    bool constellation=false;
    uint32 factionID=0;
    uint32 sunTypeID=0;
    double security=0.0;
    double radius=0.0;
    double luminosity=0.0;
    GPoint minPosition = NULL_ORIGIN;
    GPoint maxPosition = NULL_ORIGIN;
    std::string securityClass="none";
};
struct SystemKillData {
    uint16 killsHour=0;
    uint16 kills24Hour=0;
    uint16 factionKills=0;
    uint16 factionKills24Hour=0;
    uint16 podKillsHour=0;
    uint16 podKills24Hour=0;

    int64 killsDateTime=0;
    int64 kills24DateTime=0;
    int64 factionDateTime=0;
    int64 faction24DateTime=0;
    int64 podDateTime=0;
    int64 pod24DateTime=0;
};

/* POD structure for static items. */
struct StaticData {
    uint16 typeID=0;
    uint32 itemID=0;
    uint32 systemID=0;
    uint32 constellationID=0;
    uint32 regionID=0;
    float radius=0.0f;
    GPoint position = NULL_ORIGIN;
};


/* POD structure for sovereignty */
struct SovereigntyData {
    uint8 contested=0;
    uint8 stationCount=0;
    uint8 militaryPoints=0;
    uint8 industrialPoints=0;
    uint32 claimID=0;
    uint32 solarSystemID=0;
    uint32 constellationID=0;
    uint32 regionID=0;
    uint32 corporationID=0;
    uint32 allianceID=0;
    uint32 claimStructureID=0;
    uint32 hubID=0;
    uint32 beaconID=0;
    uint32 jammerID=0;
    int64 claimTime=0;
};

/* Tags for sov multi-index container */
struct SovDataBySolarSystem {};
struct SovDataByConstellation {};
struct SovDataByRegion {};
struct SovDataByAlliance {};
struct SovDataByClaim {};

/* POD structure for stations. */
struct StationData {
    bool conquerable :1;
    uint8 officeSlots=0;
    uint8 operationID=0;
    uint16 typeID=0;
    uint16 graphicID=0;
    uint16 descriptionID=0;
    uint16 hangarGraphicID=0;
    uint16 dockingBayGraphicID=0;
    uint16 reprocessingHangarFlag=0;
    int32 officeRentalFee=0;
    uint32 stationID=0;
    uint32 corporationID=0;
    uint32 maxShipVolumeDockable=0;
    uint32 systemID=0;
    uint32 constellationID=0;
    uint32 regionID=0;
    uint32 orbitID=0;
    int64 serviceMask=0;
    float radius=0.0f;
    float security=0.0f;
    float dockingCostPerVolume=0.0f;
    float reprocessingEfficiency=0.0f;
    float reprocessingStationsTake=0.0f;
    GPoint position = NULL_ORIGIN;
    GPoint dockEntry = NULL_ORIGIN;
    GPoint dockPosition = NULL_ORIGIN;
    GVector dockOrientation = NULL_ORIGIN_V;
    std::string name="none";
    std::string description="none";
};

/* POD structure for corp office data */
// this is used by multiple systems.  keep here instead of in corpData.h
struct OfficeData {
    bool lockDown :1;
    uint32 officeID=0;
    uint32 folderID=0;
    uint16 typeID=0;
    uint32 stationID=0;
    uint32 corporationID=0;
    int64 rentalFee=0;
    int64 expiryTime=0;
    std::string ticker="none";
    std::string name="none";
};

/* POD structure for loading dynamic items */
struct OwnerData {
    uint32 ownerID=0;
    uint32 corpID=0;
    uint32 locID=0;
};

/* POD structure for container faction data */
struct FactionData {
    int32 allianceID=0;
    int32 factionID=0;
    uint32 ownerID=0;
    uint32 corporationID=0;
};

/* structure for loading static system items */
struct DBSystemEntity {
    uint16 typeID=0;
    uint16 groupID=0;
    uint32 itemID=0;
    double radius=0.0;
};

struct DBSystemDynamicEntity {
    uint8 categoryID=0;
    uint16 typeID=0;
    uint16 groupID=0;
    int32 allianceID=0;
    int32 factionID=0;
    uint32 itemID=0;
    uint32 ownerID=0;
    uint32 corporationID=0;
    uint32 planetID=0;
    GPoint position = NULL_ORIGIN;
    std::string itemName = "none";
};

struct DBGPointEntity {
    uint8 idx=0;
    uint32 itemID=0;
    double radius=0.0;
    GPoint position = NULL_ORIGIN;
};

/* POD structure for decoded probe data */
struct ProbeData {
    uint8 state=0;
    uint8 rangeStep=0;
    int64 expiry=0;
    float scanRange=0.0f;
    GPoint dest = NULL_ORIGIN;
};


#endif  // _EVEMU_POD_CONTAINERS_H_