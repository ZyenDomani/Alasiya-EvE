
/*
 * EVE_POS.h
 * data containers for POS items
 *
 */


#ifndef EVE_POS_ENUMS_H
#define EVE_POS_ENUMS_H

class GVector;

namespace EVEPOS {
    struct CustomsData {
        bool allowAlliance;
        bool allowStandings;
        int8 state;          /* used to hold POS state (online, reinforced, operating, etc) */
        int16 selectedHour;
        int32 itemID;
        int32 planetID;
        int64 timestamp;
        float status;
        float taxRate;
        float standingLevel;
        GVector rotation;      /* yaw,pitch,roll - direction to planet (for correct orientation) */
    };

    struct StructureData {
        int8 use;
        int8 view;
        int8 take;
        int8 state;          /* used to hold POS state (online, reinforced, operating, etc) */
        int32 itemID;
        int32 towerID;
        int32 moonID;
        int64 timestamp;
        float status;
    };
    struct TowerData {
        int8 anchor;
        int8 unanchor;
        int8 online;
        int8 offline;
        int32 harmonic;       /* this is POS ForceField status */
        int32 standingOwnerID;
        float standing;
        std::string password;
        bool allowCorp :1;
        bool allowAlliance :1;
        bool allyStandings :1;
        bool statusDrop :1;
        bool corpWar :1;
        bool showInCalendar :1;
        bool sendFuelNotifications :1;
    };

    struct JumpBridgeData {
        int32 itemID;
        int32 corpID;
        int32 allyID;
        int32 towerID;
        int32 systemID;
        int32 toSystemID;
        int32 toItemID;
        int32 toTypeID;
        std::string password;
        bool allowCorp :1;
        bool allowAlliance :1;
    };

    struct OrbitalData {
        uint32 planetID;
        int8 level;
        uint32 orbitalHackerID;
        float orbitalHackerProgress;
        uint32 standingOwnerID; // corp/ally
    };

    struct POS_Connections {
        uint32 toID;
        uint32 sourceID;
    };

    struct POS_Resource {
        uint32 typeID;
        uint32 quantity;
    };

    namespace Harmonic {
        enum  {
            Inactive = -1,
            Offline = 0,
            Online = 1
        };
    }

    namespace ProcState {
        // these are internal only, used to determine method call on timer tic
        enum {
            Invalid            = 0,
            Unanchoring        = 1,
            Anchoring          = 2,
            Offlining          = 3,
            Onlining           = 4,
            Online             = 5,
            Operating          = 6,
            Reinforcing        = 7,
            SheildReinforcing  = 8,
            ArmorReinforcing   = 9
        };
    }

    namespace StructureState {
        enum  {
            Incapacitated     = -1,
            Unanchored        = 0,
            Anchored          = 1,
            Onlining          = 2,
            Reinforced        = 3,
            Online            = 4,
            Operating         = 5,
            Vulnerable        = 6,
            SheildReinforced  = 7,
            ArmorReinforced   = 8,
            Invulnerable      = 9
        };
    }

    namespace OrbitalState {
    // not totally sure what these are for....customs offices for one...no, these are entityState (drones)
        enum  {
            Offlining             = -7,
            Anchoring             = -6,
            Onlining              = -5,
            Anchored              = -4,
            Unanchoring           = -3,
            Unanchored            = -2,
            Incapacitated         = -1,
            Idle                  = 0,
            Combat                = 1,
            Mining                = 2,
            Approaching           = 3,
            Departing             = 4,
            Departing_2           = 5,
            Pursuit               = 6,
            Fleeing               = 7,
            Reinforced            = 8,
            Operating             = 9,
            Engage                = 10,
            Vulnerable            = 11,
            ShieldReinforce       = 12,
            ArmorReinforce        = 13,
            Invulnerable          = 14,
            WarpAwayAndDie        = 15,
            WarpAwayAndComeCack   = 16,
            WarpToPosition        = 17
        };
    }

/*
 * typedef enum {
 *    posShieldStartLevel = 0.505f,
 *    posMaxShieldPercentageForWatch = 0.95f,
 *    posMinDamageDiffToPersist = 0.05f
 * };
 */
}

#endif  //EVE_POS_ENUMS_H


/*{'FullPath': u'UI/Inflight/MoonMining', 'messageID': 238189, 'label': u'TowerLocated'}(u'Location: <b>{moonLocation}</b>', None, {u'{moonLocation}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'moonLocation'}})
 * {'FullPath': u'UI/Inflight/MoonMining', 'messageID': 238190, 'label': u'TowerPowerUsage'}(u'{[numeric]currentValue}/{[numeric]totalPower} MW', None, {u'{[numeric]currentValue}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'currentValue'}, u'{[numeric]totalPower}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'totalPower'}})
 * {'FullPath': u'UI/Inflight/MoonMining', 'messageID': 238191, 'label': u'TowerCPUUsage'}(u'{[numeric]currentValue}/{[numeric]totalCPU} tf', None, {u'{[numeric]currentValue}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'currentValue'}, u'{[numeric]totalCPU}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'totalCPU'}})
 */