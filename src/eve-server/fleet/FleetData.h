
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
    int8 armored;  // armor hit points
    int8 leader;   // targeting speed
    int8 info;     // targeting range
    int8 mining;   // mining yield
    int8 siege;    // shield capacity
    int8 skirmish; // agility
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

// client fleet error msgs
/*{'messageKey': 'FleetAlreadyBooster', 'dataID': 17879147, 'suppressable': False, 'bodyID': 257898, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2260}
 * {'messageKey': 'FleetAlreadyLeader', 'dataID': 17881512, 'suppressable': False, 'bodyID': 258800, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1743}
 * {'messageKey': 'FleetAlreadyMoving', 'dataID': 17880064, 'suppressable': False, 'bodyID': 258248, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2102}
 * {'messageKey': 'FleetAlreadyRequestedAccess', 'dataID': 17876294, 'suppressable': False, 'bodyID': 256820, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 256819, 'messageID': 3069}
 * {'messageKey': 'FleetApplicationReceived', 'dataID': 17876289, 'suppressable': False, 'bodyID': 256818, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3067}
 * {'messageKey': 'FleetBoosterIllegal', 'dataID': 17879152, 'suppressable': False, 'bodyID': 257900, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257899, 'messageID': 2261}
 * {'messageKey': 'FleetBoosterRoleFull', 'dataID': 17879155, 'suppressable': False, 'bodyID': 257901, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2262}
 * {'messageKey': 'FleetBroadcastScopeChange', 'dataID': 17876906, 'suppressable': False, 'bodyID': 257052, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3099}
 * {'messageKey': 'FleetCONCORDSpamPreventionActConfirmation', 'dataID': 17883823, 'suppressable': False, 'bodyID': 259640, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 259639, 'messageID': 898}
 * {'messageKey': 'FleetCandidateDodgy1', 'dataID': 17883738, 'suppressable': True, 'bodyID': 259609, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 259608, 'messageID': 899}
 * {'messageKey': 'FleetCandidateDodgyN', 'dataID': 17883743, 'suppressable': True, 'bodyID': 259611, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 259610, 'messageID': 900}
 * {'messageKey': 'FleetCandidateNotInvited', 'dataID': 17881515, 'suppressable': False, 'bodyID': 258801, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1744}
 * {'messageKey': 'FleetCandidateOffline', 'dataID': 17881015, 'suppressable': False, 'bodyID': 258615, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258614, 'messageID': 1801}
 * {'messageKey': 'FleetCannotDeleteNonEmptySquad', 'dataID': 17880104, 'suppressable': False, 'bodyID': 258264, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258263, 'messageID': 2112}
 * {'messageKey': 'FleetCannotDeleteNonEmptyWing', 'dataID': 17880094, 'suppressable': False, 'bodyID': 258260, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258259, 'messageID': 2109}
 * {'messageKey': 'FleetCannotDoInStation', 'dataID': 17879185, 'suppressable': False, 'bodyID': 257913, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': 257912, 'messageID': 2432}
 * {'messageKey': 'FleetCannotJoinFleet', 'dataID': 17876319, 'suppressable': False, 'bodyID': 256830, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 256829, 'messageID': 3103}
 * {'messageKey': 'FleetCannotSendBroadcastToSelectedGroup', 'dataID': 17880139, 'suppressable': False, 'bodyID': 258278, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258277, 'messageID': 2177}
 * {'messageKey': 'FleetCantKickBoss', 'dataID': 17880134, 'suppressable': False, 'bodyID': 258276, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258275, 'messageID': 2162}
 * {'messageKey': 'FleetConfirmAutoJoinVoice', 'dataID': 17879160, 'suppressable': False, 'bodyID': 257903, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 257902, 'messageID': 2435}
 * {'messageKey': 'FleetConfirmDemoteSelf', 'dataID': 17880114, 'suppressable': False, 'bodyID': 258268, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 258267, 'messageID': 2123}
 * {'messageKey': 'FleetConfirmVoiceEnable', 'dataID': 17879165, 'suppressable': True, 'bodyID': 257905, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 257904, 'messageID': 2437}
 * {'messageKey': 'FleetError', 'dataID': 17880147, 'suppressable': False, 'bodyID': 258281, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2192}
 * {'messageKey': 'FleetExportInfo', 'dataID': 17876911, 'suppressable': True, 'bodyID': 257054, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257053, 'messageID': 3114}
 * {'messageKey': 'FleetInviteAllWithoutStanding', 'dataID': 17876299, 'suppressable': False, 'bodyID': 256822, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 256821, 'messageID': 3071}
 * {'messageKey': 'FleetInviteMultipleErrors', 'dataID': 17877676, 'suppressable': False, 'bodyID': 257342, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257341, 'messageID': 2901}
 * {'messageKey': 'FleetJoinFleetFromLinkError', 'dataID': 17876324, 'suppressable': False, 'bodyID': 256832, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 256831, 'messageID': 3108}
 * {'messageKey': 'FleetJoinRequestRejected', 'dataID': 17876900, 'suppressable': False, 'bodyID': 257050, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3066}
 * {'messageKey': 'FleetMemberAlreadyInGroup', 'dataID': 17881518, 'suppressable': False, 'bodyID': 258802, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1745}
 * {'messageKey': 'FleetMemberAlreadyInvited', 'dataID': 17881521, 'suppressable': False, 'bodyID': 258803, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1746}
 * {'messageKey': 'FleetMemberHasJoined', 'dataID': 17876903, 'suppressable': False, 'bodyID': 257051, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3068}
 * {'messageKey': 'FleetMemberJoinRequest', 'dataID': 17876897, 'suppressable': False, 'bodyID': 257049, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3065}
 * {'messageKey': 'FleetMemberNotFound', 'dataID': 17880084, 'suppressable': False, 'bodyID': 258256, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258255, 'messageID': 2106}
 * {'messageKey': 'FleetMembersDodgy1', 'dataID': 17883748, 'suppressable': True, 'bodyID': 259613, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 259612, 'messageID': 901}
 * {'messageKey': 'FleetMembersDodgyN', 'dataID': 17883753, 'suppressable': True, 'bodyID': 259615, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 259614, 'messageID': 902}
 * {'messageKey': 'FleetMustBeLeader', 'dataID': 17881524, 'suppressable': False, 'bodyID': 258804, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1747}
 * {'messageKey': 'FleetMustBeLeaderAndBoss', 'dataID': 17880124, 'suppressable': False, 'bodyID': 258272, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258271, 'messageID': 2160}
 * {'messageKey': 'FleetMustBeMember', 'dataID': 17881527, 'suppressable': False, 'bodyID': 258805, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1748}
 * {'messageKey': 'FleetMustSpecifyScope', 'dataID': 17876314, 'suppressable': False, 'bodyID': 256828, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 256827, 'messageID': 3097}
 * {'messageKey': 'FleetNoInviteInStation', 'dataID': 17881530, 'suppressable': False, 'bodyID': 258806, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1749}
 * {'messageKey': 'FleetNoPositionFound', 'dataID': 17880144, 'suppressable': False, 'bodyID': 258280, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258279, 'messageID': 2186}
 * {'messageKey': 'FleetNoSuchFleet', 'dataID': 17881533, 'suppressable': False, 'bodyID': 258807, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1750}
 * {'messageKey': 'FleetNobodyHasAccess', 'dataID': 17876304, 'suppressable': False, 'bodyID': 256824, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 256823, 'messageID': 3083}
 * {'messageKey': 'FleetNotAMember', 'dataID': 17880948, 'suppressable': False, 'bodyID': 258589, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1751}
 * {'messageKey': 'FleetNotAllowed', 'dataID': 17880109, 'suppressable': False, 'bodyID': 258266, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258265, 'messageID': 2122}
 * {'messageKey': 'FleetNotCommanderOrBoss', 'dataID': 17876651, 'suppressable': False, 'bodyID': 256957, 'messageType': 'hint', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3140}
 * {'messageKey': 'FleetNotCreator', 'dataID': 17880089, 'suppressable': False, 'bodyID': 258258, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258257, 'messageID': 2107}
 * {'messageKey': 'FleetNotFleet', 'dataID': 17880119, 'suppressable': False, 'bodyID': 258270, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258269, 'messageID': 2124}
 * {'messageKey': 'FleetNotFound', 'dataID': 17876309, 'suppressable': False, 'bodyID': 256826, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 256825, 'messageID': 3093}
 * {'messageKey': 'FleetNotInFleet', 'dataID': 17880069, 'suppressable': False, 'bodyID': 258250, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258249, 'messageID': 2103}
 * {'messageKey': 'FleetNotInvited', 'dataID': 17880074, 'suppressable': False, 'bodyID': 258252, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258251, 'messageID': 2104}
 * {'messageKey': 'FleetNotLeader', 'dataID': 17880079, 'suppressable': False, 'bodyID': 258254, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258253, 'messageID': 2105}
 * {'messageKey': 'FleetNotMemberOfAnyFleet', 'dataID': 17880951, 'suppressable': False, 'bodyID': 258590, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1752}
 * {'messageKey': 'FleetNotNPCCorp', 'dataID': 17879237, 'suppressable': False, 'bodyID': 257933, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257932, 'messageID': 2466}
 * {'messageKey': 'FleetNotOpenToRegister', 'dataID': 17877428, 'suppressable': False, 'bodyID': 257247, 'messageType': 'hint', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2908}
 * {'messageKey': 'FleetNotSelfInvite', 'dataID': 17879170, 'suppressable': False, 'bodyID': 257907, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257906, 'messageID': 2463}
 * {'messageKey': 'FleetNotWingCommander', 'dataID': 17880099, 'suppressable': False, 'bodyID': 258262, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258261, 'messageID': 2110}
 * {'messageKey': 'FleetNotYourAlliance', 'dataID': 17879180, 'suppressable': False, 'bodyID': 257911, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257910, 'messageID': 2465}
 * {'messageKey': 'FleetNotYourCorp', 'dataID': 17879175, 'suppressable': False, 'bodyID': 257909, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257908, 'messageID': 2464}
 * {'messageKey': 'FleetPositionFilled', 'dataID': 17880229, 'suppressable': False, 'bodyID': 258313, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258312, 'messageID': 2113}
 * {'messageKey': 'FleetPositionFilledInvite', 'dataID': 17880249, 'suppressable': False, 'bodyID': 258321, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258320, 'messageID': 2171}
 * {'messageKey': 'FleetPositionFilledMove', 'dataID': 17880254, 'suppressable': False, 'bodyID': 258323, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258322, 'messageID': 2172}
 * {'messageKey': 'FleetRegroup', 'dataID': 17883756, 'suppressable': False, 'bodyID': 259616, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 903}
 * {'messageKey': 'FleetRemoveFleetFinderAd', 'dataID': 17876707, 'suppressable': True, 'bodyID': 256979, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 256978, 'messageID': 3082}
 * {'messageKey': 'FleetTooManyFavorites', 'dataID': 17878295, 'suppressable': False, 'bodyID': 257574, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 257573, 'messageID': 2567}
 * {'messageKey': 'FleetTooManyMembers', 'dataID': 17880234, 'suppressable': False, 'bodyID': 258315, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258314, 'messageID': 2158}
 * {'messageKey': 'FleetTooManyMembersConvert', 'dataID': 17880239, 'suppressable': False, 'bodyID': 258317, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258316, 'messageID': 2159}
 * {'messageKey': 'FleetTooManyMembersInSquad', 'dataID': 17880244, 'suppressable': False, 'bodyID': 258319, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258318, 'messageID': 2165}
 * {'messageKey': 'FleetTooManySquads', 'dataID': 17880259, 'suppressable': False, 'bodyID': 258325, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258324, 'messageID': 2111}
 * {'messageKey': 'FleetTooManyWings', 'dataID': 17880224, 'suppressable': False, 'bodyID': 258311, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258310, 'messageID': 2108}
 * {'messageKey': 'FleetUpdateFleetFinderAd', 'dataID': 17876508, 'suppressable': False, 'bodyID': 256902, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 256901, 'messageID': 3070}
 * {'messageKey': 'FleetUpdateFleetFinderAd_ChangedCorp', 'dataID': 17875813, 'suppressable': False, 'bodyID': 256645, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 256644, 'messageID': 3411}
 * {'messageKey': 'FleetUpdateFleetFinderAd_LastMember', 'dataID': 17876518, 'suppressable': False, 'bodyID': 256906, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 256905, 'messageID': 3094}
 * {'messageKey': 'FleetUpdateFleetFinderAd_NewBoss', 'dataID': 17876503, 'suppressable': False, 'bodyID': 256900, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 256899, 'messageID': 3095}
 * {'messageKey': 'FleetUpdateFleetFinderAd_Standing', 'dataID': 17876513, 'suppressable': False, 'bodyID': 256904, 'messageType': 'question', 'urlAudio': '', 'urlIcon': '', 'titleID': 256903, 'messageID': 3096}
 * {'messageKey': 'FleetWaitForLSC', 'dataID': 17877506, 'suppressable': False, 'bodyID': 257277, 'messageType': 'warning', 'urlAudio': '', 'urlIcon': '', 'titleID': 257276, 'messageID': 2946}
 * {'messageKey': 'FleetWarp', 'dataID': 17883759, 'suppressable': False, 'bodyID': 259617, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 904}
 * {'messageKey': 'FleetYouAreAlreadyInFleet', 'dataID': 17880129, 'suppressable': False, 'bodyID': 258274, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258273, 'messageID': 2161}
 * {'messageKey': 'FleetsterNotInSystem', 'dataID': 17883682, 'suppressable': False, 'bodyID': 259589, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 905}
 */
