/*
 *
 *
 *
 */


#ifndef EVE_POS_ENUMS_H
#define EVE_POS_ENUMS_H

namespace EVEPOS {
    struct TowerData {
        // tower management
        int32 harmonic;       /* this is POS ForceField status */
        std::string password;
        bool allowCorp :1;
        bool allowAlliance :1;
        float standing;
        float status;
        bool statusDrop :1;
        bool corpWar :1;
        uint32 standingOwnerID; // corp/ally
        bool showInCalendar :1;
        bool sendFuelNotifications :1;
    };

    struct SaveData {
        uint32 itemID;
        int32 harmonic;       /* this is POS ForceField status */

        uint8 state;          /* used to hold POS state (online, reinforced, operating, etc) */
        uint32 towerID;       /* this is the controlling towerID for POS modules */
        uint64 timestamp;     /* used to track start time on POS states (onlining, reinforced, etc) */

        // for orbital infrastructure (customs office)
        GPoint rotation;      /* direction to planet (for correct orientation) */
        uint32 planetID;

        // tower management
        float standing;
        float status;
        bool allowCorp :1;
        bool allowAlliance :1;
        bool statusDrop :1;
        bool corpWar :1;
        uint32 standingOwnerID; // corp/ally
        bool showInCalendar :1;
        bool sendFuelNotifications :1;
    };

    enum ForceField {
        inactive = -1,
        offline = 0,
        online = 1
    };

    enum StructureState {
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
    } ;

    // not totally sure what these are for....customs offices for one...
    enum OrbitalState {
        STATE_OFFLINING             = -7,
        STATE_ANCHORING             = -6,
        STATE_ONLINING              = -5,
        STATE_ANCHORED              = -4,
        STATE_UNANCHORING           = -3,
        STATE_UNANCHORED            = -2,
        STATE_INCAPACITATED         = -1,
        STATE_IDLE                  = 0,
        STATE_COMBAT                = 1,
        STATE_MINING                = 2,
        STATE_APPROACHING           = 3,
        STATE_DEPARTING             = 4,
        STATE_DEPARTING_2           = 5,
        STATE_PURSUIT               = 6,
        STATE_FLEEING               = 7,
        STATE_REINFORCED            = 8,
        STATE_OPERATING             = 9,
        STATE_ENGAGE                = 10,
        STATE_VULNERABLE            = 11,
        STATE_SHIELD_REINFORCE      = 12,
        STATE_ARMOR_REINFORCE       = 13,
        STATE_INVULNERABLE          = 14,
        STATE_WARPAWAYANDDIE        = 15,
        STATE_WARPAWAYANDCOMEBACK   = 16,
        STATE_WARPTOPOSITION        = 17
    } ;

/*
 * typedef enum {
 *    posShieldStartLevel = 0.505f,
 *    posMaxShieldPercentageForWatch = 0.95f,
 *    posMinDamageDiffToPersist = 0.05f
 * };
 */
}


/*  for indy */
//from table 'ramActivities'
enum EVERamActivity {
    ramActivityManufacturing = 1,
    ramActivityResearchingTechnology = 2,
    ramActivityResearchingTimeProductivity = 3,
    ramActivityResearchingMaterialProductivity = 4,
    ramActivityCopying = 5,
    ramActivityDuplicating = 6,
    ramActivityReverseEngineering = 7,
    ramActivityInvention = 8
};

//from table 'ramCompletedStatuses'
enum EVERamCompletedStatus {
    ramCompletedStatusInProgress = 0,
    ramCompletedStatusDelivered = 1,
    ramCompletedStatusAbort = 2,
    ramCompletedStatusGMAbort = 3,
    ramCompletedStatusUnanchor = 4,
    ramCompletedStatusDestruction = 5
};

//restrictionMask from table 'ramAssemblyLines'
enum EVERamRestrictionMask {
    ramRestrictNone = 0,
    ramRestrictBySecurity = 1,
    ramRestrictByStanding = 2,
    ramRestrictByCorp = 4,
    ramRestrictByAlliance = 8
};

#endif  //EVE_POS_ENUMS_H