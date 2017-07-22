/*
 *
 *
 *
 */


#ifndef EVE_POS_ENUMS_H
#define EVE_POS_ENUMS_H

enum StructureState {
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
    ANCHORED_STRUCTURE_STATES = (STRUCTURE_ANCHORED,
                                 STRUCTURE_ONLINING,
                                 STRUCTURE_REINFORCED,
                                 STRUCTURE_ONLINE,
                                 STRUCTURE_OPERATING,
                                 STRUCTURE_VULNERABLE,
                                 STRUCTURE_SHIELD_REINFORCE,
                                 STRUCTURE_ARMOR_REINFORCE,
                                 STRUCTURE_INVULNERABLE)
    db2Entity = {STRUCTURE_UNANCHORED: [STATE_UNANCHORING, STATE_UNANCHORED],
        STRUCTURE_ANCHORED: [STATE_ANCHORING, STATE_ANCHORED],
        STRUCTURE_ONLINING: [STATE_ONLINING, STATE_ONLINING],
        STRUCTURE_REINFORCED: [STATE_REINFORCED],
        STRUCTURE_ONLINE: [STATE_IDLE],
        STRUCTURE_OPERATING: [STATE_OPERATING, STATE_IDLE],
        STRUCTURE_VULNERABLE: [STATE_VULNERABLE],
        STRUCTURE_SHIELD_REINFORCE: [STATE_SHIELD_REINFORCE],
        STRUCTURE_ARMOR_REINFORCE: [STATE_ARMOR_REINFORCE]}
     */
    /*
ONLINE_STABLE_STATES = (STRUCTURE_REINFORCED,
 STRUCTURE_ONLINE,
 STRUCTURE_OPERATING,
 STRUCTURE_VULNERABLE,
 STRUCTURE_INVULNERABLE,
 STRUCTURE_SHIELD_REINFORCE,
 STRUCTURE_ARMOR_REINFORCE)

def Entity2DB(activityState):
    if activityState in (STATE_UNANCHORING, STATE_UNANCHORED):
        return STRUCTURE_UNANCHORED
    if activityState in (STATE_ANCHORING, STATE_ANCHORED):
        return STRUCTURE_ANCHORED
    if activityState == STATE_ONLINING:
        return STRUCTURE_ONLINING
    if activityState == STATE_REINFORCED:
        return STRUCTURE_REINFORCED
    if activityState == STATE_VULNERABLE:
        return STRUCTURE_VULNERABLE
    if activityState == STATE_INVULNERABLE:
        return STRUCTURE_INVULNERABLE
    if activityState == STATE_OPERATING:
        return STRUCTURE_OPERATING
    if activityState == STATE_SHIELD_REINFORCE:
        return STRUCTURE_SHIELD_REINFORCE
    if activityState == STATE_ARMOR_REINFORCE:
        return STRUCTURE_ARMOR_REINFORCE
    if activityState >= STATE_IDLE:
        return STRUCTURE_ONLINE
*/
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
} ;

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