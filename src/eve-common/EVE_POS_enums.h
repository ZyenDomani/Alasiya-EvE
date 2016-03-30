/*
 *
 *
 *
 */



#ifndef EVE_POS_ENUMS_H
#define EVE_POS_ENUMS_H

typedef enum {
    STRUCTURE_UNANCHORED        = 0,
    STRUCTURE_ANCHORED          = 1,
    STRUCTURE_ONLINING          = 2,
    STRUCTURE_REINFORCED        = 3,
    STRUCTURE_ONLINE            = 4,
    STRUCTURE_OPERATING         = 5,
    STRUCTURE_VULNERABLE        = 6,
    STRUCTURE_SHIELD_REINFORCE  = 7,
    STRUCTURE_ARMOR_REINFORCE   = 8,
    STRUCTURE_INVULNERABLE      = 9
    /*
     *    pwnStructureStateAnchored = 'anchored',
     *    pwnStructureStateAnchoring = 'anchoring',
     *    pwnStructureStateOnline = 'online',
     *    pwnStructureStateOnlining = 'onlining',
     *    pwnStructureStateUnanchored = 'unanchored',
     *    pwnStructureStateUnanchoring = 'unanchoring',
     *    pwnStructureStateVulnerable = 'vulnerable',
     *    pwnStructureStateInvulnerable = 'invulnerable',
     *    pwnStructureStateReinforced = 'reinforced',
     *    pwnStructureStateOperating = 'operating',
     *    pwnStructureStateIncapacitated = 'incapacitated',
     *    pwnStructureStateAnchor = 'anchor',
     *    pwnStructureStateUnanchor = 'unanchor',
     *    pwnStructureStateOffline = 'offline',
     *    pwnStructureStateOnlineActive = 'online - active',
     *    pwnStructureStateOnlineStartingUp = 'online - starting up'
     */
} POSState;

typedef enum {
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
} StructureState;

#endif  //EVE_POS_ENUMS_H