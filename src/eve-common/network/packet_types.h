/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2011 The EVEmu Team
 *    For the latest information visit http://evemu.org
 *    ------------------------------------------------------------------------------------
 *    This program is free software; you can redistribute it and/or modify it under
 *    the terms of the GNU Lesser General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option) any later
 *    version.
 *
 *    This program is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License along with
 *    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 *    http://www.gnu.org/copyleft/lesser.txt.
 *    ------------------------------------------------------------------------------------
 *    Author:        Zhur
 *    Updates:       Allan
 */

/** @todo  this file really isnt named correctly.  move like items into new files named accordingly. */
//  wip  -allan 29May15

#ifndef EVE_PACKET_TYPES_H
#define EVE_PACKET_TYPES_H

#include "../EVE_Consts.h"
#include "../EVE_Defines.h"
#include "../EVE_Effects.h"
#include "../EVE_Flags.h"
#include "../EVE_POS_enums.h"
#include "../EVE_Roles.h"
#include "../EVE_Scanning.h"
#include "../EVE_Skills.h"
#include "../EVE_Typedefs.h"


/*
 *
 * SERVICE_STOPPED = 1
 * SERVICE_START_PENDING = 2
 * SERVICE_STOP_PENDING = 3
 * SERVICE_RUNNING = 4
 * SERVICE_CONTINUE_PENDING = 5
 * SERVICE_PAUSE_PENDING = 6
 * SERVICE_PAUSED = 7
 * SERVICETYPE_NORMAL = 1
 * SERVICETYPE_BUILTIN = 2
 * SERVICETYPE_EXPORT_CONSTANTS = 4
 * SERVICE_CONTROL_STOP = 1
 * SERVICE_CONTROL_PAUSE = 2
 * SERVICE_CONTROL_CONTINUE = 3
 * SERVICE_CONTROL_INTERROGATE = 4
 * SERVICE_CONTROL_SHUTDOWN = 5
 * SERVICE_CHECK_NONE = 0
 * SERVICE_CHECK_CALL = 1
 * SERVICE_CHECK_INIT = 2
 * SERVICE_WANT_SESSIONS = 1
 * PRE_NONE = 0
 * PRE_AUTH = 1
 * PRE_HASCHAR = 2
 * PRE_HASSHIP = 4
 * PRE_INSTATION = 8
 * PRE_INFLIGHT = 16
 */

enum MACHONETMSG_TYPE
{
    AUTHENTICATION_REQ              = 0,
    AUTHENTICATION_RSP              = 1,
    IDENTIFICATION_REQ              = 2,
    IDENTIFICATION_RSP              = 3,
    __Fake_Invalid_Type             = 4,
    CALL_REQ                        = 6,
    CALL_RSP                        = 7,
    TRANSPORTCLOSED                 = 8,
    RESOLVE_REQ                     = 10,
    RESOLVE_RSP                     = 11,
    NOTIFICATION                    = 12,
    ERRORRESPONSE                   = 15,
    SESSIONCHANGENOTIFICATION       = 16,
    SESSIONINITIALSTATENOTIFICATION = 18,
    PING_REQ                        = 20,
    PING_RSP                        = 21,
    MOVEMENTNOTIFICATION            = 100,
    MACHONETMSG_TYPE_COUNT
};

enum MACHONETERR_TYPE
{
    UNMACHODESTINATION  = 0,
    UNMACHOCHANNEL      = 1,
    WRAPPEDEXCEPTION    = 2
};
//see PyPacket.cpp
extern const char* MACHONETMSG_TYPE_NAMES[MACHONETMSG_TYPE_COUNT];


enum SESSION_TYPE
{
    SESSION_TYPE_INVALID = 0,
    SESSION_TYPE_EXECUTIONCONTEXT = 1,
    SESSION_TYPE_SERVICE = 2,
    SESSION_TYPE_CREST = 3,
    SESSION_TYPE_ESP = 4,
    SESSION_TYPE_GAME = 5
};

//these came from the 'constants' object:
enum EVEItemChangeType {
    ixItemID        = 0,    //also ixLauncherCapacity?
    ixTypeID        = 1,    //also ixLauncherUsed = 1,
    ixOwnerID       = 2,    //also ixLauncherChargeItem?
    ixLocationID    = 3,
    ixFlag          = 4,
    ixContraband    = 5,
    ixSingleton     = 6,
    ixGroupID       = 8,
    ixQuantity      = 7,
    ixCategoryID    = 9,
    ixCustomInfo    = 10,
    ixSubitems      = 11
};

enum EVEContainerTypes {
    containerWallet            = 10001,
    containerGlobal            = 10002,
    containerSolarSystem       = 10003,
    containerHangar            = 10004,
    containerScrapHeap         = 10005,
    containerFactory           = 10006,
    containerBank              = 10007,
    containerRecycler          = 10008,
    containerOffices           = 10009,
    containerStationCharacters = 10010,
    containerCharacter         = 10011,
    containerCorpMarket        = 10012
};

enum EVERookieShipTypes {
    amarrRookie                = 596,
    caldariRookie              = 601,
    gallenteRookie             = 606,
    minmatarRookie             = 588,
};


typedef enum {
    dgmEffPassive       = 0,
    dgmEffActivation    = 1,
    dgmEffTarget        = 2,
    dgmEffArea          = 3,
    dgmEffOnline        = 4,
} EffectCategories;

//  -allan 5Aug14
typedef enum {
    searchResultAgent           = 1,
    searchResultCharacter       = 2,
    searchResultCorporation     = 3,
    searchResultAlliance        = 4,
    searchResultFaction         = 5,
    searchResultConstellation   = 6,
    searchResultSolarSystem     = 7,
    searchResultRegion          = 8,
    searchResultStation         = 9,
    searchResultInventoryType   = 10,
    //searchResultAllOwners = [1, 2, 3, 4, 5],
    //searchResultAllLocations = [6, 7, 8, 9],
    searchMaxResults            = 500,
    searchMinWildcardLength     = 3
} SearchTypes;

typedef enum EVETutorialTypes {
    advchannelsTutorial = 50,
    cloningTutorial = 42,
    cloningWhenPoddedTutorial = 27,
    podriskTutorial = 43,
    skillfittingTutorial = 51,
    insuranceTutorial = 41,
    tutorial = 109,
    tutorialAuraIntroduction = 5,
    tutorialBoarding = 6,
    tutorialCharacterSheet = 7,
    tutorialControlConsole = 18,
    tutorialItems = 8,
    tutorialNavigation = 19,
    tutorialOverview = 21,
    tutorialSelling = 10,
    tutorialShips = 6,
    tutorialSpace = 17,
    tutorialTargeting = 20,
    tutorialUndock = 16,
    tutorialWallet = 11,
    tutorialWarpingDC = 23,
    tutorialCombatChooseTheVenue = 103,
    tutorialCombatConcepts = 105,
    tutorialCombatKnowYourEquipment = 104,
    tutorialCombatStudyTheOpponent = 102,
    tutorialInformativeCareerPlanning = 99,
    tutorialInformativeCharacterSheetAdvancedInformation = 100,
    tutorialInformativeContracts = 54,
    tutorialInformativeCorporations = 33,
    tutorialInformativeCosmosComplexes = 101,
    tutorialInformativeCrimeAndPunishment = 97,
    tutorialInformativeDrones = 65,
    tutorialInformativeExploration = 124,
    tutorialInformativeFittingStationService = 13,
    tutorialInformativeHeat = 123,
    tutorialInformativeMap = 14,
    tutorialInformativeMarket = 12,
    tutorialInformativePeopleAndPlaces = 15,
    tutorialInformativePoliticsAndmanagement = 98,
    tutorialInformativeRepairshop = 46,
    tutorialInformativeReprocessingPlant = 9,
    tutorialInformativeSalvaging = 122,
    tutorialInformativeScanning = 63,
    tutorialInformativeScienceIndustry = 52,
    tutorialWorldspaceNavigation = 235,
    tutorialTutorials = 215,
    tutorialCertificates = 134
} TutorialTypes;


/*
 * planetResourceScanDistance = 1000000000
 * planetResourceProximityDistant = 0
 * planetResourceProximityRegion = 1
 * planetResourceProximityConstellation = 2
 * planetResourceProximitySystem = 3
 * planetResourceProximityPlanet = 4
 * planetResourceProximityLimits = [(2, 6),
 * (4, 10),
 * (6, 15),
 * (10, 20),
 * (15, 30)]
 * planetResourceScanningRanges = [9.0,
 * 7.0,
 * 5.0,
 * 3.0,
 * 1.0]
 * planetResourceUpdateTime = 1 * HOUR
 * planetResourceMaxValue = 1.21
 */

/*
 * mailingListBlocked = 0
 * mailingListAllowed = 1
 * mailingListMemberMuted = 0
 * mailingListMemberDefault = 1
 * mailingListMemberOperator = 2
 * mailingListMemberOwner = 3
 * ALLIANCE_SERVICE_MOD = 200
 * CHARNODE_MOD = 64
 * PLANETARYMGR_MOD = 128
 * mailTypeMail = 1
 * mailTypeNotifications = 2
 * mailStatusMaskRead = 1
 * mailStatusMaskReplied = 2
 * mailStatusMaskForwarded = 4
 * mailStatusMaskTrashed = 8
 * mailStatusMaskDraft = 16
 * mailStatusMaskAutomated = 32
 * mailLabelInbox = 1
 * mailLabelSent = 2
 * mailLabelCorporation = 4
 * mailLabelAlliance = 8
 * mailLabelsSystem = mailLabelInbox + mailLabelSent + mailLabelCorporation + mailLabelAlliance
 * mailMaxRecipients = 50
 * mailMaxGroups = 1
 * mailMaxSubjectSize = 150
 * mailMaxBodySize = 8000
 * mailMaxTaggedBodySize = 10000
 * mailMaxLabelSize = 40
 * mailMaxNumLabels = 25
 * mailMaxPerPage = 100
 * mailTrialAccountTimer = 1
 * mailMaxMessagePerMinute = 5
 * mailinglistMaxMembers = 3000
 * mailinglistMaxMembersUpdated = 1000
 * mailingListMaxNameSize = 60
 */


/*
 * typedef enum {
 * 3018681,
 * 3018821,
 * 3018822,
 * 3018823,
 * 3018824,
 * 3018680,
 * 3018817,
 * 3018818,
 * 3018819,
 * 3018820,
 * 3018682,
 * 3018809,
 * 3018810,
 * 3018811,
 * 3018812,
 * 3018678,
 * 3018837,
 * 3018838,
 * 3018839,
 * 3018840,
 * 3018679,
 * 3018841,
 * 3018842,
 * 3018843,
 * 3018844,
 * 3018677,
 * 3018845,
 * 3018846,
 * 3018847,
 * 3018848,
 * 3018676,
 * 3018825,
 * 3018826,
 * 3018827,
 * 3018828,
 * 3018675,
 * 3018805,
 * 3018806,
 * 3018807,
 * 3018808,
 * 3018672,
 * 3018801,
 * 3018802,
 * 3018803,
 * 3018804,
 * 3018684,
 * 3018829,
 * 3018830,
 * 3018831,
 * 3018832,
 * 3018685,
 * 3018813,
 * 3018814,
 * 3018815,
 * 3018816,
 * 3018683,
 * 3018833,
 * 3018834,
 * 3018835,
 * 3018836]
 * }rookieAgentList;
 */
/*
auraAgentIDs = [
 3019499,
 3019493,
 3019495,
 3019490,
 3019497,
 3019496,
 3019486,
 3019498,
 3019492,
 3019500,
 3019489,
 3019494]
 */

//  -allan 20Dec14
typedef enum {
    agentTypeNonAgent = 1,
    agentTypeBasicAgent = 2,
    agentTypeTutorialAgent = 3,
    agentTypeResearchAgent = 4,
    agentTypeGenericStorylineMissionAgent = 6,
    agentTypeStorylineMissionAgent = 7,
    agentTypeEventMissionAgent = 8,
    agentTypeFactionalWarfareAgent = 9,
    agentTypeEpicArcAgent = 10,
    agentTypeAura = 11
} agentTypes;

/*
agentRangeSameSystem = 1
agentRangeSameOrNeighboringSystemSameConstellation = 2
agentRangeSameOrNeighboringSystem = 3
agentRangeNeighboringSystemSameConstellation = 4
agentRangeNeighboringSystem = 5
agentRangeSameConstellation = 6
agentRangeSameOrNeighboringConstellationSameRegion = 7
agentRangeSameOrNeighboringConstellation = 8
agentRangeNeighboringConstellationSameRegion = 9
agentRangeNeighboringConstellation = 10
agentRangeNearestEnemyCombatZone = 11
agentRangeNearestCareerHub = 12
agentIskMultiplierLevel1 = 1
agentIskMultiplierLevel2 = 2
agentIskMultiplierLevel3 = 4
agentIskMultiplierLevel4 = 8
agentIskMultiplierLevel5 = 16
agentIskMultipliers = (agentIskMultiplierLevel1,
 agentIskMultiplierLevel2,
 agentIskMultiplierLevel3,
 agentIskMultiplierLevel4,
 agentIskMultiplierLevel5)
agentLpMultiplierLevel1 = 20
agentLpMultiplierLevel2 = 60
agentLpMultiplierLevel3 = 180
agentLpMultiplierLevel4 = 540
agentLpMultiplierLevel5 = 4860
agentLpMultipliers = (agentLpMultiplierLevel1,
 agentLpMultiplierLevel2,
 agentLpMultiplierLevel3,
 agentLpMultiplierLevel4,
 agentLpMultiplierLevel5)
agentIskRandomLowValue = 11000
agentIskRandomHighValue = 16500
agentCareerTypeIndustry = 1
agentCareerTypeBusiness = 2
agentCareerTypeMilitary = 3
agentCareerTypeExploration = 4
agentCareerTypeAdvMilitary = 5
agentDialogueButtonViewMission = 1
agentDialogueButtonRequestMission = 2
agentDialogueButtonAccept = 3
agentDialogueButtonAcceptChoice = 4
agentDialogueButtonAcceptRemotely = 5
agentDialogueButtonComplete = 6
agentDialogueButtonCompleteRemotely = 7
agentDialogueButtonContinue = 8
agentDialogueButtonDecline = 9
agentDialogueButtonDefer = 10
agentDialogueButtonQuit = 11
agentDialogueButtonStartResearch = 12
agentDialogueButtonCancelResearch = 13
agentDialogueButtonBuyDatacores = 14
agentDialogueButtonLocateCharacter = 15
agentDialogueButtonLocateAccept = 16
agentDialogueButtonLocateReject = 17
agentDialogueButtonYes = 18
agentDialogueButtonNo = 19
*/

/*
allianceApplicationAccepted = 2
allianceApplicationEffective = 3
allianceApplicationNew = 1
allianceApplicationRejected = 4
allianceCreationCost = 1000000000
allianceMembershipCost = 2000000
allianceRelationshipCompetitor = 3
allianceRelationshipEnemy = 4
allianceRelationshipFriend = 2
allianceRelationshipNAP = 1
*/

/*
facwarCorporationJoining = 0
facwarCorporationActive = 1
facwarCorporationLeaving = 2
facwarStandingPerVictoryPoint = 0.0015
facwarWarningStandingCharacter = 0
facwarWarningStandingCorporation = 1
facwarOccupierVictoryPointBonus = 0.1
facwarMinStandingsToJoin = 0.5
facwarStatTypeKill = 0
facwarStatTypeLoss = 1
*/

/** misc costs
 * costCloneContract = 5600
 * costJumpClone = 100000
 */

//the constants are made up of:
//  prefix     -> cachedObject
//                config.BulkData.constants
//     category   -> config.BulkData.categories
//     group      -> config.BulkData.groups
//     metaGreoup -> config.BulkData.metagroups
//     attribute  -> config.BulkData.dgmattribs
//     effect     -> config.BulkData.dgmeffects
//    billType   -> config.BulkData.billtypes
//     role       -> config.Roles
//     flag       -> config.Flags
//     race       -> config.Races
//     bloodline  -> config.Bloodlines
//     statistic  -> config.Statistics
//     unit       -> config.Units
//     channelType -> config.ChannelTypes
//     encyclopediaType -> config.EncyclopediaTypes
//     activity   -> config.BulkData.ramactivities
//     completedStatus -> config.BulkData.ramcompletedstatuses
//
// First letter of `Name` field if capitalized when prefixed.
// see InsertConstantsFromRowset

/*
 * typedef enum {
 *    posShieldStartLevel = 0.505f,
 *    posMaxShieldPercentageForWatch = 0.95f,
 *    posMinDamageDiffToPersist = 0.05f
 * };
 */

/*
 * cacheSystemIntervals = 2000109999
 * cacheSystemSettings = 2000100001
 * cacheSystemSchemas = 2000100003
 * cacheSystemTables = 2000100004
 * cacheSystemProcedures = 2000100006
 * cacheSystemEventTypes = 2000100013
 * cacheUserEventTypes = 2000209999
 * cacheUserColumns = 2000209998
 * cacheUserRegions = 2000209997
 * cacheUserTimeZones = 2000209996
 * cacheUserCountries = 2000209995
 * cacheUserTypes = 2000209994
 * cacheUserStatuses = 2000209993
 * cacheUserRoles = 2000209992
 * cacheUserConnectTypes = 2000209991
 * cacheUserOperatingSystems = 2000209990
 * cacheStaticSettings = 2000309999
 * cacheStaticBranches = 2000300001
 * cacheStaticReleases = 2000300006
 * cacheStaticIntegrateOptions = 2000300008
 * cacheMlsLanguages = 2000409999
 * cacheMlsTranslationStatuses = 2000409998
 * cacheMlsTextGroupTypes = 2000409997
 * cacheMlsTextStatuses = 2000409996
 * cacheMlsTaskStatuses = 2000409995
 * cacheClusterServices = 2000909999
 * cacheClusterMachines = 2000909998
 * cacheClusterProxies = 2000909997
 * cacheClientBrowserSiteFlags = 2003009999
 * cacheAccountingKeys = 2001100001
 * cacheAccountingEntryTypes = 2001100002
 * cacheInventoryCategories = 2001300001
 * cacheInventoryGroups = 2001300002
 * cacheInventoryTypes = 2001300003
 * cacheInventoryFlags = 2001300012
 * cacheEventGroups = 2001500002
 * cacheEventTypes = 2001500003
 * cacheWorldSpaces = 2001700035
 * cacheWorldSpaceDistricts = 2001700001
 * cacheResGraphics = 2001800001
 * cacheResSounds = 2001800002
 * cacheResDirectories = 2001800003
 * cacheResIcons = 2001800004
 * cacheResDetailMeshes = 2001800005
 * cacheActionTreeSteps = 2001900002
 * cacheActionTreeProcs = 2001900003
 * cacheEntityIngredients = 2002200001
 * cacheEntityIngredientInitialValues = 2002200002
 * cacheEntitySpawns = 2002200006
 * cacheEntityRecipes = 2002200009
 * cacheEntitySpawnGroups = 2002200010
 * cacheEntitySpawnGroupLinks = 2002200011
 * cacheActionObjects = 2002400001
 * cacheActionStations = 2002400002
 * cacheActionStationActions = 2002400003
 * cacheActionObjectStations = 2002400004
 * cacheActionObjectExits = 2002400005
 * cacheTreeNodes = 2002500001
 * cacheTreeLinks = 2002500002
 * cacheTreeProperties = 2002500005
 * cachePerceptionSenses = 2002600001
 * cachePerceptionStimTypes = 2002600002
 * cachePerceptionSubjects = 2002600004
 * cachePerceptionTargets = 2002600005
 * cachePerceptionBehaviorSenses = 2002600010
 * cachePerceptionBehaviorFilters = 2002600011
 * cachePerceptionBehaviorDecays = 2002600012
 * cachePaperdollModifierLocations = 2001600002
 * cachePaperdollResources = 2001600003
 * cachePaperdollSculptingLocations = 2001600004
 * cachePaperdollColors = 2001600005
 * cachePaperdollColorNames = 2001600006
 * cachePaperdollColorRestrictions = 2001600007
 * cacheEncounterEncounters = 2003100001
 * cacheEncounterCoordinates = 2003100002
 * cacheEncounterCoordinateSets = 2003100003
 * cacheStaticUsers = 2000000001
 * cacheUsersDataset = 2000000002
 * cacheCharactersDataset = 2000000003
 * cacheNameNames = 2000000004
 */


/*
 * service.ROLE_CHTADMINISTRATOR | service.ROLE_GMH
 * CHTMODE_CREATOR = (((8 + 4) + 2) + 1)
 * CHTMODE_OPERATOR = ((4 + 2) + 1)
 * CHTMODE_CONVERSATIONALIST = (2 + 1)
 * CHTMODE_SPEAKER = 2
 * CHTMODE_LISTENER = 1
 * CHTMODE_NOTSPECIFIED = -1
 * CHTMODE_DISALLOWED = -2
 * CHTERR_NOSUCHCHANNEL = -3
 * CHTERR_ACCESSDENIED = -4
 * CHTERR_INCORRECTPASSWORD = -5
 * CHTERR_ALREADYEXISTS = -6
 * CHTERR_TOOMANYCHANNELS = -7
 * CHT_MAX_USERS_PER_IMMEDIATE_CHANNEL = 50
 *
 * CHANNEL_CUSTOM = 0
 * CHANNEL_GANG = 3
 *
 *
 */
#endif  //EVE_PACKET_TYPES_H