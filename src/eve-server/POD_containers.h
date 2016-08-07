/*
 * POD_continers.h
 *
 *    -allan  1Aug16
 */

#ifndef _EVEMU_POD_CONTAINERS_H_
#define _EVEMU_POD_CONTAINERS_H_

#include "eve-server.h"

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

/**
 * POD for fleet data    -allan 31Jul14
 */
struct FleetData {
    uint32 fleetID;
    uint32 wingID;
    uint32 squadID;
    uint8 fleetRole;
    uint8 fleetBooster;
    uint8 fleetJob;
};

/**
 * POD for character kill data  -allan 01May16
 */
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

/* POD entry for asteroid */
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

/* POD entry for asteroid distrubtion methods by group */
struct OreTypeChance {
    uint16 typeID;
    float chance;
};

/* POD entry for active dungeon */
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

/* POD entry for cosmic signatures/anomalies */
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

/* POD entry for spawn groups */
struct SystemSpawnGroup { //reference to this bubble's data for spawn groups.  may need later.
    //SystemBubble* pSysBubble;   //cant use reference or pointer here...
    uint32 bubbleID;
    uint32 systemID;
    uint32 regionID;
    double secRating;
};

/* POD entry for spawn groups */
struct SpawnGroup {
    uint32 typeID;  //typeID to spawn
    uint8 quantity; //quantity to spawn for this typeID
};

/* POD entry for spawn entries */
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

/* POD entry for spawn faction groups */
struct RatFactionGroups {  // notes for me while creating/writing/testing
    uint8 shipClass;      // shipclass - arbitrary
    uint32 groupID;     // item groupID
};

/* POD entry for spawn classes */
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


/* POD entry for systems. */
struct SystemData {
    uint32 systemID;
    std::string name;
    uint32 constellationID;
    uint32 regionID;
    float securityRating;
    std::string securityClass;
};

/* POD entry for static items. */
struct StaticData {
    uint32 itemID;
    uint32 systemID;
    uint32 constellationID;
    uint32 regionID;
    GPoint position;
};

/* POD entry for stations. */
struct StationData {
    uint32 stationID;
    GPoint position;
    GPoint dockPosition;
    GVector dockOrientation;
};


#endif  // _EVEMU_POD_CONTAINERS_H_