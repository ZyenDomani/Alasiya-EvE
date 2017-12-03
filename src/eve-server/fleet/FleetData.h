
 /**
  * @name FleetData.h
  *     Fleet enums and Data containers for Alasiya EvEmu
  *
  * @Author:        Allan
  * @date:          05 August 2014 (original skeleton outline)
  * @update:        21 November 2017 (begin actual implementation)
  *
  */

#ifndef EVEMU_SRC_FLEET_DATA_H_
#define EVEMU_SRC_FLEET_DATA_H_

namespace Fleet {
    enum Job {
        None        = 0,
        Scout       = 1,
        Creator     = 2
    };

    enum Role {
        FleetLeader = 1,
        WingLeader  = 2,
        SquadLeader = 3,
        Member      = 4
    };

    enum Booster {
        No      = 0,
        Fleet   = 1,
        Wing    = 2,
        Squad   = 3
    };

    enum Invite {
        Closed = 0,
        Corp = 1,
        Alliance = 2,
        Militia = 4,
        Public = 8,
        Any = 15
    };

    enum BCastScope {
        Universe = 0,
        System = 1,
        Bubble = 2
    };

    enum BCastGroup {
        //None = 0, already defined...
        Down = 1,   // subordinates (or squad if member)
        Up = 2,     // superiors
        All = 3
    };
}

// all bonuses are 2%/lvl
struct BoostData {
    int8 leader;   // targeting speed
    int8 armored;  // armor hit points
    int8 info;     // targeting range
    int8 siege;    // shield capacity
    int8 skirmish; // agility
    int8 mining;   // mining yield
};

class Client;

struct FleetAdvert {
    bool hideInfo;
    bool joinNeedsApproval;
    uint8 inviteScope;
    Client* leader;
    uint32 solarSystemID;
    int32 fleetID;
    int64 advertTime;
    int64 dateCreated;
    float local_minSecurity;
    float public_minStanding;
    float local_minStanding;
    float public_minSecurity;
    std::string fleetName;
    std::string description;
    std::vector<uint32> public_allowedEntities;
    std::vector<uint32> local_allowedEntities;
};

struct FleetData {
    bool isFreeMove;
    bool isRegistered;
    bool isVoiceEnabled;
    bool isLootLogging;
    int8 squads;
    int64 dateCreated;
    Client* creator;
    Client* leader;
    Client* booster;
    std::string name;
    std::string motd;
    std::multimap<uint32, uint32> isMutedByLeader;
    std::multimap<uint32, uint32> isExcludedFromMuting;
};

// fleetID, wing name and squad count (5 per fleet)
struct WingData {
    Client* leader;
    Client* booster;
    uint32 fleetID;
    BoostData boost;
    std::string name;
};

// wingID, squad name and member count (5 per wing)
struct SquadData {
    Client* leader;
    Client* booster;
    uint32 fleetID;
    uint32 wingID;
    BoostData boost;
    std::string name;
    std::map<uint32, Client*> members;
};

struct InviteData {
    Client* inviteBy;
    Client* invited;
    int32 wingID;
    int32 squadID;
    int8 role;
};

#endif  // EVEMU_SRC_FLEET_DATA_H_


//Pilots who are disconnected and reconnect within 2 minutes will automatically rejoin their fleet.

/*Note that the maximum size of a fleet is 256 pilots:
 * 1 Fleet Commander,
 * 5 Wing Commanders,
 * 25 10-man squadrons (5 Squads of 9 Pilots + Squad Commander per Wing Commander).
 *
 * To put it another way the maximum size of:
 *   each squadron is 9 members plus the Squad Commander, (10 total)
 *   each wing is five squadrons plus the Wing Commander, (51 total)
 *   each fleet is 5 wings plus the Fleet Commander.      (256 total)
 */
