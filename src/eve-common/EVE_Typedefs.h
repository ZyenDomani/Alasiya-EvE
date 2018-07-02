
/*
 *
 *
 *
 */

#ifndef EVE_TYPEDEFS_H
#define EVE_TYPEDEFS_H

#include "tables/invCategories.h"
typedef EVEDB::invCategories::invCategories EVEItemCategories;
#include "tables/invGroups.h"
typedef EVEDB::invGroups::invGroups EVEItemGroups;
#include "tables/invTypes.h"
typedef EVEDB::invTypes::invTypes EVEItemTypes;


//List of eve item types which have special purposes in the game.
//try to keep this list as short as possible, most things should be accomplish able
//by looking at the attributes of an item, not its type.
typedef enum {
    itemTypeCapsule = 670,
    itemTypeTrit    = 34,
    itemTypeJetCan  = 23,
    AllianceTypeID  = 16159,
    itemTypeBillboard = 11136
} EVEItemTypeID;


//paperdoll state.  unused, but may be used later
typedef enum {
    paperdollStateNoRecustomization = 0,
    paperdollStateResculpting = 1,
    paperdollStateNoExistingCustomization = 2,
    paperdollStateFullRecustomizing = 3,
    paperdollStateForceRecustomize = 4
} paperDollState;

//userType id's used for accounts
typedef enum {
    userTypePlayer              = 1,        // i added this....not sure if it's right.
    userTypeCCP                 = 13,
    userTypeUK                  = 17,
    userTypePBC                 = 20,
    userTypeETC                 = 21,
    userTypeTrial               = 23,
    userTypeMammon              = 30,       // this is default player account type
    userTypeMedia               = 31,
    userTypeCDKey               = 33,
    userTypeIA                  = 34,

    userTypeDustPlayer          = 101,
    userTypeDustCCP             = 102,
    userTypeDustBattleServer    = 103
} EVEUserType;


//raceID as in table 'entity'
enum EVERace {
    raceCaldari     = 1,
    raceMinmatar    = 2,
    raceAmarr       = 4,
    raceGallente    = 8,
    raceJove        = 16,
    racePirate      = 32,
    raceSleepers    = 64,
    raceORE         = 128
};

typedef enum {
    CharAmarr       = 1373,
    CharNiKunni     = 1374,
    CharCivire      = 1375,
    CharDeteis      = 1376,
    CharGallente    = 1377,
    CharIntaki      = 1378,
    CharSebiestor   = 1379,
    CharBrutor      = 1380,
    CharStatic      = 1381,
    CharModifier    = 1382,
    CharAchura      = 1383,
    CharJinMei      = 1384,
    CharKhanid      = 1385,
    CharVherokior   = 1386
} EVECharType;

//eve standing change messages in db.repStandingChanges.eventTypeID
//  these come from /eve/common/script/mgt/appLogConst.py
typedef enum {
    standingPodKill                         = 9,
    standingPodKilled                       = 10,
    standingStandingReset                   = 25,   //Reset by a GM.
    standingShipKill                        = 26,
    standingShipKilled                      = 27,
    standingUpdateStanding                  = 45,
    standingDecay                           = 49,   //All standing decays except when user isn't logged in
    standingPlayerSet                       = 65,   //Set by player him/herself. Reason: _msg
    standingCorpSet                         = 68,   //Corp stand set by _int1. Reason: _msg
    standingMissionCompleted                = 73,   //_msg: name of mission
    standingMissionFailure                  = 74,   //_msg: name of mission
    standingMissionDeclined                 = 75,   //_msg: name of mission
    standingCombatAggression                = 76,   //Combat - Aggression
    standingCombatShipKill                  = 77,   //Combat - Ship Kill
    standingCombatPodKill                   = 78,   //Combat - Pod Kill
    standingCombatOther                     = 79,
    standingAgentMissionBonus               = 80,
    standingDerivedModificationPleased      = 82,   //fromID was pleased
    standingDerivedModificationDispleased   = 83,   //fromID was displeased
    standingGMInterventionDirect            = 84,   //Mod directly by _int1. Reason: _msg
    standingLawEnforcement                  = 89,   //Granted by Concord for actions against _int1
    standingMissionOfferExpired             = 90,   //Mission Offer Expired - _msg
    standingStandingCorrection              = 96,
    standingAgentMissionFailedRollback      = 97,
    standingStandingRollback                = 98,
    standingCombatAssistance                = 112,  //Combat - Assistance
    standingPropertyDamage                  = 154,  //Property Damage
    standingCombatShipKillOwnFaction        = 223,
    standingCombatPodKillOwnFaction         = 224,
    standingCombatAggressionOwnFaction      = 225,
    standingCombatAssistanceOwnFaction      = 226,
    standingCombatOtherOwnFaction           = 228
    //anything up until 500 is 'Standing Change'
} EVEStandingEventTypeID;

//  -allan 7Jul14
typedef enum {
    DungeonStarted      = 0,
    DungeonCompleted    = 1,
    DungeonFailed       = 2
} DungeonState;

enum EVERookieShipTypes {
    amarrRookie                = 596,
    caldariRookie              = 601,
    gallenteRookie             = 606,
    minmatarRookie             = 588
};

enum EVERookieWeaponTypes {
    amarrWeapon                 = 3634,
    caldariWeapon               = 3638,
    gallenteWeapon              = 3640,
    minmatarWeapon              = 3636
};

//  -allan 5Aug14
enum SearchTypes {
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
};

enum EVETutorialTypes {
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
};


//  -updated 18Dec16
enum ClientTimers {
    DefaultTimer     = 1000,
    BoardTimer       = 900,
    JumpTimer        = 300,
    UndockTimer      = 500,     // used to delay sending Destiny::State (client error fix)
    DockingTimer     = 1000,    // Timer to delay docking (as on live)
    JumpingTimer     = 4000,    // Timer to delay jumping
    MovingTimer      = 1000,
    ScanningTimer    = 10000,   // used to delay scan results based on skills, items, and other shit
    KilledTimer      = 1500,    // used to reset destiny set state after killed or otherwise changing ships
    ProcTimer        = 1000,    // used to give process ticks to docked players (for skill updates...tick cycle consumption negligible)
    JetcanTimer      = 180000,  // used to delay jetcan creation.  3min default
    LogoutTimer      = 10000,    // used to hold client object until WarpOut finishes
    LoginTimer       = 2000,
    SessionTimer     = 10000,   // used to prevent multiple session changes from occuring too fast
    DockInvul        = 3000,
    FleetTimer       = 1500,
    JumpInvul        = 5000,
    WarpOutInvul     = 5000,
    WarpInInvul      = 18000,   // increased from 10s
    UndockInvul      = 20000,
    RestoringInvul   = 60000,
    JumpCloak        = 30000,
    LoginCloak       = 20000
};

enum ClientState {
    csIdle = 1,
    csJump = 2,
    csDock = 3,
    csUndock = 4,
    csKilled = 5,
    csLogout = 6,
    csBoard  = 7,
    csLogin  = 8
};

//  -allan 7Jul14
typedef enum {
    calendarMonday = 0,
    calendarTuesday = 1,
    calendarWednesday = 2,
    calendarThursday = 3,
    calendarFriday = 4,
    calendarSaturday = 5,
    calendarSunday = 6,
    calendarJanuary = 1,
    calendarFebruary = 2,
    calendarMarch = 3,
    calendarApril = 4,
    calendarMay = 5,
    calendarJune = 6,
    calendarJuly = 7,
    calendarAugust = 8,
    calendarSeptember = 9,
    calendarOctober = 10,
    calendarNovember = 11,
    calendarDecember = 12,
    calendarNumDaysInWeek = 7,
    calendarTagPersonal = 1,
    calendarTagCorp = 2,
    calendarTagAlliance = 4,
    calendarTagCCP = 8,
    calendarTagAutomated = 16,
    calendarViewRangeInMonths = 12,
    calendarMaxTitleSize = 40,
    calendarMaxDescrSize = 500,
    calendarMaxInvitees = 50,
    calendarMaxInviteeDisplayed = 100,
    calendarAutoEventPosFuel = 1,
    calendarStartYear = 2003,
    calendarResponseUninvited = 0,
    calendarResponseDeleted = 1,
    calendarResponseDeclined = 2,
    calendarResponseUndecided = 3,
    calendarResponseAccepted = 4,
    calendarResponseMaybe = 5
} CalendarDef;

//message format argument types:
typedef enum {
    fmtMapping_OWNERID2 = 1,    //not used? owner name
    fmtMapping_OWNERID = 2,    //owner name
    fmtMapping_LOCID = 3,    //locations
    fmtMapping_itemTypeName = 4,    //TYPEID: takes the item ID
    fmtMapping_itemTypeDescription = 5,    //TYPEID2: takes the item ID
    fmtMapping_blueprintTypeName = 6,    //from invBlueprints
    fmtMapping_itemGroupName = 7,    //GROUPID: takes the item group ID
    fmtMapping_itemGroupDescription = 8,    //GROUPID2: takes the item group ID
    fmtMapping_itemCategoryName = 9,    //CATID: takes the item category ID
    fmtMapping_itemCategoryDescription = 10,    //CATID2: takes the item category ID
    fmtMapping_DGMATTR = 11,    //not used...
    fmtMapping_DGMFX = 12,        //not used...
    fmtMapping_DGMTYPEFX = 13,    //not used...
    fmtMapping_dateTime = 14,    //DATETIME: formatted date and time
    fmtMapping_date = 15,        //DATE: formatted date
    fmtMapping_time = 16,        //TIME: formatted time
    fmtMapping_shortTime = 17,    //TIMESHRT: formatted time, short format
    fmtMapping_long = 18,        //AMT: regular number format
    fmtMapping_ISK2 = 19,        //AMT2: ISK format
    fmtMapping_ISK3 = 20,        //AMT3: ISK format
    fmtMapping_distance = 21,        //DIST: distance format
    fmtMapping_message = 22,    //MSGARGS: nested message
    fmtMapping_ADD_THE = 22,    //ADD_THE: prefix argument with 'the '
    fmtMapping_ADD_A = 23,        //ADD_A: prefix argument with 'a ' or 'an ' as appropriate
    fmtMapping_typeQuantity = 24,    //TYPEIDANDQUANTITY: human readable representation of the two arguments: typeID and quantity
    fmtMapping_ownerNickname = 25,    //OWNERIDNICK: first part of owner name (before space)
    fmtMapping_station = 26,    //SESSIONSENSITIVESTATIONID: human readable, fully qualified station name (includes system, constellation and region)
    fmtMapping_system = 27,    //SESSIONSENSITIVELOCID: human readable, fully qualified system name (includes constellation and region)
    fmtMapping_ISK = 28,        //ISK: ISK format
    fmtMapping_TYPEIDL = 29
} fmtMappingType;

typedef enum {
    ownerNone               = 0,
    ownerSystem             = 1,
    ownerBank               = 2,
    ownerStation            = 4,
    ownerCombatSimulator    = 5,
    ownerUnknown            = 3006,
    ownerCONCORD            = 1000125,
    ownerSCC                = 1000132,
    ownerDED                = 1000137   //division of concord
} ownerDefs;

typedef enum {
    factionNoFaction     = 0,
    factionCaldari       = 500001,
    factionMinmatar      = 500002,
    factionAmarr         = 500003,
    factionGallente      = 500004,
    factionJove          = 500005,
    factionCONCORD       = 500006,
    factionAmmatar       = 500007,
    factionKhanid        = 500008,
    factionSyndicate     = 500009,
    factionGuristas      = 500010,
    factionAngel         = 500011,
    factionBloodRaider   = 500012,
    factionInterBus      = 500013,
    factionORE           = 500014,
    factionThukker       = 500015,
    factionSistersOfEVE  = 500016,
    factionSociety       = 500017,
    factionMordusLegion  = 500018,
    factionSanshas       = 500019,
    factionSerpentis     = 500020,
    factionUnknown       = 500021,
    factionRogueDrones   = 500022,
    factionSleepers      = 500023
} FactionDef;

typedef enum {
    /* this is a numeric system to organize salvage data types and is arranged as follows:
     * ABCDE
     * A  = tech level of salvage  - 1,2,3
     * B  = type of salvage: 0-, 1-data, 2-archeology, 3-ancient, 4-wrecks, 5-loot, 6-, 7-, 8-, 9-
     * C  = size of salvage: 0-tiny, 1-small, 2-med, 3-large, 4-huge, 5-, 6-, 7-, 8-, 9-
     * DE = last 2 actual faction numbers as defined in FactionDef
     */
    salvageSleepers     = 0

} FactionSalvageDef;

typedef enum {
    salvageWreck = 1,
    salvageData = 2,
    salvageArch = 3,
    salvageAncient = 4,
    salvageOther = 5
} SalvageTypes;

// mostly pirate corps for now...
typedef enum {
    corpRogueDrones         = 1000001,
    corpCONCORD             = 1000125,
    corpSCC                 = 1000132,
    corpDED                 = 1000137,
    corpGuristas            = 1000127,
    corpAngel               = 1000124,
    corpBloodRaider         = 1000134,
    corpSociety             = 1000131,
    corpMordusLegion        = 1000128,
    corpSanshas             = 1000161,
    corpSerpentis           = 1000135,
    corpInterbus            = 1000148

    /*
     CBD Corporation     1000002
     Prompt Delivery     1000003
     Ytiri   1000004
     Hyasyoda Corporation    1000005
     Deep Core Mining Inc.   1000006
     Poksu Mineral Group     1000007
     Minedrill   1000008
     Caldari Provisions  1000009
     Kaalakiota Corporation  1000010
     Wiyrkomi Corporation    1000011
     Top Down    1000012
     Rapid Assembly  1000013
     Perkone     1000014
     Caldari Steel   1000015
     Zainou  1000016
     Nugoeihuvi Corporation  1000017
     Echelon Entertainment   1000018
     Ishukone Corporation    1000019
     Lai Dai Corporation     1000020
     Zero-G Research Firm    1000021
     Propel Dynamics     1000022
     Expert Distribution     1000023
     CBD Sell Division   1000024
     Sukuuvestaa Corporation     1000025
     Caldari Constructions   1000026
     Expert Housing  1000027
     Caldari Funds Unlimited     1000028
     State and Region Bank   1000029
     Modern Finances     1000030
     Chief Executive Panel   1000031
     Mercantile Club     1000032
     Caldari Business Tribunal   1000033
     House of Records    1000034
     Caldari Navy    1000035
     Internal Security   1000036
     Lai Dai Protection Service  1000037
     Ishukone Watch  1000038
     Home Guard  1000039
     Peace and Order Unit    1000040
     Spacelane Patrol    1000041
     Wiyrkomi Peace Corps    1000042
     Corporate Police Force  1000043
     School of Applied Knowledge     1000044
     Science and Trade Institute     1000045
     Sebiestor tribe     1000046
     Krusual tribe   1000047
     Vherokior tribe     1000048
     Brutor tribe    1000049
     Republic Parliament     1000050
     Republic Fleet  1000051
     Republic Justice Department     1000052
     Urban Management    1000053
     Republic Security Services  1000054
     Minmatar Mining Corporation     1000055
     Core Complexion Inc.    1000056
     Boundless Creation  1000057
     Eifyr and Co.   1000058
     Six Kin Development     1000059
     Native Freshfood    1000060
     Freedom Extension   1000061
     The Leisure Group   1000062
     Amarr Constructions     1000063
     Carthum Conglomerate    1000064
     Imperial Armaments  1000065
     Viziam  1000066
     Zoar and Sons   1000067
     Noble Appliances    1000068
     Ducia Foundry   1000069
     HZO Refinery    1000070
     Inherent Implants   1000071
     Imperial Shipment   1000072
     Amarr Certified News    1000073
     Joint Harvesting    1000074
     Nurtura     1000075
     Further Foodstuffs  1000076
     Royal Amarr Institute   1000077
     Imperial Chancellor     1000078
     Amarr Civil Service     1000079
     Ministry of War     1000080
     Ministry of Assessment  1000081
     Ministry of Internal Order  1000082
     Amarr Trade Registry    1000083
     Amarr Navy  1000084
     Court Chamberlain   1000085
     Emperor Family  1000086
     Kador Family    1000087
     Sarum Family    1000088
     Kor-Azor Family     1000089
     Ardishapur Family   1000090
     Tash-Murkon Family  1000091
     Civic Court     1000092
     Theology Council    1000093
     TransStellar Shipping   1000094
     Federal Freight     1000095
     Inner Zone Shipping     1000096
     Material Acquisition    1000097
     Astral Mining Inc.  1000098
     Combined Harvest    1000099
     Quafe Company   1000100
     corporationName     corporationID
     CreoDron    1000101
     Roden Shipyards     1000102
     Allotek Industries  1000103
     Poteque Pharmaceuticals     1000104
     Impetus     1000105
     Egonics Inc.    1000106
     The Scope   1000107
     Chemal Tech     1000108
     Duvolle Laboratories    1000109
     FedMart     1000110
     Aliastra    1000111
     Bank of Luminaire   1000112
     Pend Insurance  1000113
     Garoun Investment Bank  1000114
     University of Caille    1000115
     President   1000116
     Senate  1000117
     Supreme Court   1000118
     Federal Administration  1000119
     Federation Navy     1000120
     Federal Intelligence Office     1000121
     Federation Customs  1000122
     Ammatar Fleet   1000123
     Archangels  1000124
     CONCORD     1000125
     Ammatar Consulate   1000126
     Guristas    1000127
     Mordu's Legion  1000128
     Outer Ring Excavations  1000129
     Sisters of EVE  1000130
     Society of Conscious Thought    1000131
     Secure Commerce Commission  1000132
     Salvation Angels    1000133
     Blood Raiders   1000134
     Serpentis Corporation   1000135
     Guardian Angels     1000136
     DED     1000137
     Dominations     1000138
     Food Relief     1000139
     Genolution  1000140
     Guristas Production     1000141
     Impro   1000142
     Inner Circle    1000143
     Intaki Bank     1000144
     Intaki Commerce     1000145
     Intaki Space Police     1000146
     Intaki Syndicate    1000147
     InterBus    1000148
     Jove Navy   1000149
     Jovian directorate  1000150
     Khanid Innovation   1000151
     Khanid Transport    1000152
     Khanid Works    1000153
     Nefantar Miner Association  1000154
     Prosper     1000155
     Royal Khanid Navy   1000156
     Serpentis Inquest   1000157
     Shapeset    1000158
     The Sanctuary   1000159
     Thukker Mix     1000160
     True Creations  1000161
     True Power  1000162
     Trust Partners  1000163
     X-Sense     1000164
     Hedion University   1000165
     Imperial Academy    1000166
     State War Academy   1000167
     Federal Navy Academy    1000168
     Center for Advanced Studies     1000169
     Republic Military School    1000170
     Republic University     1000171
     Pator Tech School   1000172
     Material Institute  1000177
     Academy of Aggressive Behaviour     1000178
     24th Imperial Crusade   1000179
     State Protectorate  1000180
     Federal Defense Union   1000181
     Tribal Liberation Force     1000182
     */
} corpDef;

// only for drones
typedef enum {
    droneIdle              = 0,
    droneCombat            = 1,
    droneMining            = 2,
    droneApproaching       = 3,
    droneDeparting         = 4,
    droneDeparting2        = 5,
    dronePursuit           = 6,
    droneFleeing           = 7,
    droneUnknown           = 8,
    droneOperating         = 9,
    droneEngage            = 10
} aiState;

// not sure what these are used for...stateFlags??
typedef enum {
    mouseOver = 1,
    selected = 2,
    activeTarget = 3,
    targeting = 4,
    targeted = 5,
    lookingAt = 6,
    threatTargetsMe = 7,
    threatAttackingMe = 8,
    flagCriminal = 9,
    flagDangerous = 10,
    flagSameFleet = 11,
    flagSameCorp = 12,
    flagAtWarCanFight = 13,
    flagSameAlliance = 14,
    flagStandingHigh = 15,
    flagStandingGood = 16,
    flagStandingNeutral = 17,
    flagStandingBad = 18,
    flagStandingHorrible = 19,
    flagIsWanted = 20,
    flagAgentInteractable = 21,
    gbEnemySpotted = 22,
    gbTarget = 23,
    gbHealShield = 24,
    gbHealArmor = 25,
    gbHealCapacitor = 26,
    gbWarpTo = 27,
    gbNeedBackup = 28,
    gbAlignTo = 29,
    gbJumpTo = 30,
    gbInPosition = 31,
    gbHoldPosition = 32,
    gbTravelTo = 33,
    gbJumpBeacon = 34,
    gbLocation = 35,
    flagWreckAlreadyOpened = 36,
    flagWreckEmpty = 37,
    flagWarpScrambled = 38,
    flagWebified = 39,
    flagECMd = 40,
    flagSensorDampened = 41,
    flagTrackingDisrupted = 42,
    flagTargetPainted = 43,
    flagAtWarMilitia = 44,
    flagSameMilitia = 45,
    flagEnergyLeeched = 46,
    flagEnergyNeut = 47,
    flagNoStanding = 48
} stateFlags;

typedef enum {
    cacheEspCorporations = 1,
    cacheEspAlliances = 2,
    cacheEspSolarSystems = 3,
    cacheSolarSystemObjects = 4,
    cacheCargoContainers = 5,
    cachePriceHistory = 6,
    cacheTutorialVersions = 7,
    cacheSolarSystemOffices = 8,
    cacheEosNpcToNpcStandings = 109998,
    cacheAutAffiliates = 109997,
    cacheAutCdkeyTypes = 109996,
    cacheTutCategories = 200006,
    cacheTutCriterias = 200003,
    cacheTutTutorials = 200001,
    cacheTutActions = 200009,
    cacheDungeonArchetypes = 300001,
    cacheDungeonDungeons = 300005,
    cacheDungeonEntityGroupTypes = 300004,
    cacheDungeonEventMessageTypes = 300017,
    cacheDungeonEventTypes = 300015,
    cacheDungeonSpawnpoints = 300012,
    cacheDungeonTriggerTypes = 300013,
    cacheInvCategories = 600001,
    cacheInvContrabandTypes = 600008,
    cacheInvGroups = 600002,
    cacheInvTypes = 600004,
    cacheInvTypeMaterials = 600005,
    cacheInvTypeReactions = 600010,
    cacheInvWreckUsage = 600009,
    cacheInvMetaGroups = 600006,
    cacheInvMetaTypes = 600007,
    cacheDogmaAttributes = 800004,
    cacheDogmaEffects = 800005,
    cacheDogmaExpressionCategories = 800001,
    cacheDogmaExpressions = 800003,
    cacheDogmaOperands = 800002,
    cacheDogmaTypeAttributes = 800006,
    cacheDogmaTypeEffects = 800007,
    cacheDogmaUnits = 800009,
    cacheEveMessages = 1000001,
    cacheInvBlueprintTypes = 1200001,
    cacheMapRegions = 1409999,
    cacheMapConstellations = 1409998,
    cacheMapSolarSystems = 1409997,
    cacheMapSolarSystemLoadRatios = 1409996,
    cacheLocationWormholeClasses = 1409994,
    cacheMapPlanets = 1409993,
    cacheMapSolarSystemJumpIDs = 1409992,
    cacheMapTypeBalls = 1400001,
    cacheMapWormholeClasses = 1400003,
    cacheMapLocationTypes = 1400004,
    cacheMapCelestialDescriptions = 1400008,
    cacheMapNebulas = 1400016,
    cacheMapLocationWormholeClasses = 1400002,
    cacheMapRegionsTable = 1400009,
    cacheMapConstellationsTable = 1400010,
    cacheMapSolarSystemsTable = 1400011,
    cacheNpcCommandLocations = 1600009,
    cacheNpcCommands = 1600005,
    cacheNpcDirectorCommandParameters = 1600007,
    cacheNpcDirectorCommands = 1600006,
    cacheNpcLootTableFrequencies = 1600004,
    cacheNpcCommandParameters = 1600008,
    cacheNpcTypeGroupingClassSettings = 1600016,
    cacheNpcTypeGroupingClasses = 1600015,
    cacheNpcTypeGroupingTypes = 1600017,
    cacheNpcTypeGroupings = 1600014,
    cacheNpcTypeLoots = 1600001,
    cacheRamSkillInfo = 1809999,
    cacheRamActivities = 1800003,
    cacheRamAssemblyLineTypes = 1800006,
    cacheRamAssemblyLineTypesCategory = 1800004,
    cacheRamAssemblyLineTypesGroup = 1800005,
    cacheRamCompletedStatuses = 1800007,
    cacheRamInstallationTypes = 1800002,
    cacheRamTypeRequirements = 1800001,
    cacheReverseEngineeringTableTypes = 1800009,
    cacheReverseEngineeringTables = 1800008,
    cacheShipInsurancePrices = 2000007,
    cacheShipTypes = 2000001,
    cacheStaOperations = 2209999,
    cacheStaServices = 2209998,
    cacheStaOperationServices = 2209997,
    cacheStaSIDAssemblyLineQuantity = 2209996,
    cacheStaSIDAssemblyLineType = 2209995,
    cacheStaSIDAssemblyLineTypeQuantity = 2209994,
    cacheStaSIDOfficeSlots = 2209993,
    cacheStaSIDReprocessingEfficiency = 2209992,
    cacheStaSIDServiceMask = 2209991,
    cacheStaStationImprovementTypes = 2209990,
    cacheStaStationUpgradeTypes = 2209989,
    cacheStaStations = 2209988,
    cacheStaStationsStatic = 2209987,
    cacheMktOrderStates = 2409999,
    cacheMktNpcMarketData = 2400001,
    cacheCrpRoles = 2809999,
    cacheCrpActivities = 2809998,
    cacheCrpNpcDivisions = 2809997,
    cacheCrpCorporations = 2809996,
    cacheCrpNpcMembers = 2809994,
    cacheCrpPlayerCorporationIDs = 2809993,
    cacheCrpTickerNamesStatic = 2809992,
    cacheNpcSupplyDemand = 2800001,
    cacheCrpRegistryGroups = 2800002,
    cacheCrpRegistryTypes = 2800003,
    cacheCrpNpcCorporations = 2800006,
    cacheAgentAgents = 3009999,
    cacheAgentCorporationActivities = 3009998,
    cacheAgentCorporations = 3009997,
    cacheAgentEpicMissionMessages = 3009996,
    cacheAgentEpicMissionsBranching = 3009995,
    cacheAgentEpicMissionsNonEnd = 3009994,
    cacheAgtContentAgentInteractionMissions = 3009992,
    cacheAgtContentFlowControl = 3009991,
    cacheAgtContentTalkToAgentMissions = 3009990,
    cacheAgtPrices = 3009989,
    cacheAgtResearchStartupData = 3009988,
    cacheAgtContentTemplates = 3000001,
    cacheAgentMissionsKill = 3000006,
    cacheAgtStorylineMissions = 3000008,
    cacheAgtContentCourierMissions = 3000003,
    cacheAgtContentExchangeOffers = 3000005,
    cacheAgentEpicArcConnections = 3000013,
    cacheAgentEpicArcMissions = 3000015,
    cacheAgentEpicArcs = 3000012,
    cacheAgtContentMissionTutorials = 3000018,
    cacheAgtContentMissionExtraStandings = 3000020,
    cacheAgtContentMissionLocationFilters = 3000021,
    cacheAgtOfferDetails = 3000004,
    cacheAgtOfferTableContents = 3000010,
    cacheChrSchools = 3209997,
    cacheChrRaces = 3200001,
    cacheChrBloodlines = 3200002,
    cacheChrAncestries = 3200003,
    cacheChrCareers = 3200004,
    cacheChrSpecialities = 3200005,
    cacheChrBloodlineNames = 3200010,
    cacheChrAttributes = 3200014,
    cacheChrFactions = 3200015,
    cacheChrDefaultOverviews = 3200011,
    cacheChrDefaultOverviewGroups = 3200012,
    cacheChrNpcCharacters = 3200016,
    cacheUserEspTagTypes = 4309999,
    cacheFacWarCombatZoneSystems = 4500006,
    cacheFacWarCombatZones = 4500005,
    cacheCertificates = 5100001,
    cacheCertificateRelationships = 5100004,
    cacheActBillTypes = 6400004,
    cachePlanetSchematics = 7300004,
    cachePlanetSchematicsTypeMap = 7300005,
    cachePlanetSchematicsPinMap = 7300003,
    cachePetCategories = 8109999,
    cachePetQueues = 8109998,
    cachePetCategoriesVisible = 8109997,
    cacheGMQueueOrder = 8109996,
    cacheBattleStatuses = 100509999,
    cacheBattleResults = 100509998,
    cacheBattleServerStatuses = 100509997,
    cacheBattleMachines = 100509996,
    cacheBattleClusters = 100509995,
    cacheMapDistrictCelestials = 100309999,
    cacheMapDistricts = 100300014,
    cacheMapBattlefields = 100300015,
    cacheMapLevels = 100300020,
    cacheMapOutposts = 100300022,
    cacheMapLandmarks = 100300023,
    cacheSystemIntervals = 2000109999,
    cacheSystemSettings = 2000100001,
    cacheSystemSchemas = 2000100003,
    cacheSystemTables = 2000100004,
    cacheSystemProcedures = 2000100006,
    cacheSystemEventTypes = 2000100013,
    cacheUserEventTypes = 2000209999,
    cacheUserColumns = 2000209998,
    cacheUserRegions = 2000209997,
    cacheUserTimeZones = 2000209996,
    cacheUserCountries = 2000209995,
    cacheUserTypes = 2000209994,
    cacheUserStatuses = 2000209993,
    cacheUserRoles = 2000209992,
    cacheUserConnectTypes = 2000209991,
    cacheUserOperatingSystems = 2000209990,
    cacheStaticSettings = 2000309999,
    cacheStaticBranches = 2000300001,
    cacheStaticReleases = 2000300006,
    cacheStaticIntegrateOptions = 2000300008,
    cacheMlsLanguages = 2000409999,
    cacheMlsTranslationStatuses = 2000409998,
    cacheMlsTextGroupTypes = 2000409997,
    cacheMlsTextStatuses = 2000409996,
    cacheMlsTaskStatuses = 2000409995,
    cacheClusterServices = 2000909999,
    cacheClusterMachines = 2000909998,
    cacheClusterProxies = 2000909997,
    cacheClientBrowserSiteFlags = 2003009999,
    cacheAccountingKeys = 2001100001,
    cacheAccountingEntryTypes = 2001100002,
    cacheInventoryCategories = 2001300001,
    cacheInventoryGroups = 2001300002,
    cacheInventoryTypes = 2001300003,
    cacheInventoryFlags = 2001300012,
    cacheEventGroups = 2001500002,
    cacheEventTypes = 2001500003,
    cacheWorldSpaces = 2001700035,
    cacheWorldSpaceDistricts = 2001700001,
    cacheResGraphics = 2001800001,
    cacheResSounds = 2001800002,
    cacheResDirectories = 2001800003,
    cacheResIcons = 2001800004,
    cacheResDetailMeshes = 2001800005,
    cacheActionTreeSteps = 2001900002,
    cacheActionTreeProcs = 2001900003,
    cacheEntityIngredients = 2002200001,
    cacheEntityIngredientInitialValues = 2002200002,
    cacheEntitySpawns = 2002200006,
    cacheEntityRecipes = 2002200009,
    cacheEntitySpawnGroups = 2002200010,
    cacheEntitySpawnGroupLinks = 2002200011,
    cacheActionObjects = 2002400001,
    cacheActionStations = 2002400002,
    cacheActionStationActions = 2002400003,
    cacheActionObjectStations = 2002400004,
    cacheActionObjectExits = 2002400005,
    cacheTreeNodes = 2002500001,
    cacheTreeLinks = 2002500002,
    cacheTreeProperties = 2002500005,
    cachePerceptionSenses = 2002600001,
    cachePerceptionStimTypes = 2002600002,
    cachePerceptionSubjects = 2002600004,
    cachePerceptionTargets = 2002600005,
    cachePerceptionBehaviorSenses = 2002600010,
    cachePerceptionBehaviorFilters = 2002600011,
    cachePerceptionBehaviorDecays = 2002600012,
    cachePaperdollModifierLocations = 2001600002,
    cachePaperdollResources = 2001600003,
    cachePaperdollSculptingLocations = 2001600004,
    cachePaperdollColors = 2001600005,
    cachePaperdollColorNames = 2001600006,
    cachePaperdollColorRestrictions = 2001600007,
    cacheEncounterEncounters = 2003100001,
    cacheEncounterCoordinates = 2003100002,
    cacheEncounterCoordinateSets = 2003100003,
    cacheStaticUsers = 2000000001,
    cacheUsersDataset = 2000000002,
    cacheCharactersDataset = 2000000003,
    cacheNameNames = 2000000004
} cacheFlags;


/*
eventSlashHeal = 100
eventSlashSetQty = 30
eventSlashSpawn = 28
eventSlashTransfer = 99
eventSlashUnspawn = 29
eventAddCorporation = 12
eventAddSuper = 18
eventAdd = 1
eventAborted = 158
eventAcceptApplication = 138
eventAcceptedMission = 88
eventAcceptedOffer = 326
eventActivateGate = 147
eventAddAlliance = 131
eventAddFacwar = 217
eventAddOffice = 46
eventAgentBuyOff = 71
eventAgentDonation = 72
eventAgentMissionBonus = 80
eventAgentMissionCompleted = 73
eventAgentMissionDeclined = 75
eventAgentMissionFailedRollback = 97
eventAgentMissionFailed = 74
eventAgentMissionOfferExpired = 90
eventAllocationFailure_ItemDeclarationError = 124
eventAllocationFailure_ItemResolutionFailure = 123
eventAllocationFailure_SanityCheckFailure = 122
eventAllocationFailure_UnexpectedException = 125
eventApplicationClosed = 157
eventApplyAlliance = 133
eventApplyForCorporationMembership = 15
eventAttributeRespecFree = 51
eventAttributeRespecScheduled = 50
eventAttributeRespecPlexTrade = 311
eventBatchLpModification = 66
eventBatchUpdateLP = 263
eventBecomeExecutor = 136
eventBecomeACeoInACorporation = 16
eventBlueprintAccepted = 106
eventBlueprintOfferExpired = 105
eventBlueprintOfferInvalid = 111
eventBlueprintOfferRejectedIncompatibleAgent = 110
eventBlueprintOfferRejectedInvalidBlueprint = 109
eventBlueprintOfferRejectedRecently = 108
eventBlueprintOfferRejectedTooLowStandings = 107
eventBlueprintOffered = 101
eventBlueprintRejected = 102
eventBoardedShipFrom = 184
eventBoosterAdded = 172
eventBoosterRemoved = 173
eventBountySources = 325
eventBunkerConquered = 221
eventBunkerLost = 222
eventCancelRemovePrepare = 41
eventChangeAppearance = 117
eventChangeEmploymentRecord = 149
eventChangeName = 31
eventChangeShortName = 127
eventChangeTicker = 115
eventChangeUser = 114
eventCapitalStationDeclared = 202
eventCapitalStationLost = 195
eventCertificateGranted = 231
eventCharacterCreationStarted = 246
eventCharacterPaused = 305
eventCharacterRescued = 191
eventCharacterResumed = 306
eventCloneDestroyedWithLocation = 190
eventCloneDestruction = 166
eventCloneImplantInstallation = 168
eventCloneInstallation = 167
eventCloneJumpTimeReset = 169
eventCloneJump = 165
eventClosed = 156
eventCombatAggressionOwnFaction = 225
eventCombatAggression = 76
eventCombatAssistanceOwnFaction = 226
eventCombatAssistance = 112
eventCombatOtherOwnFaction = 228
eventCombatOther = 79
eventCombatPodKillOwnFaction = 224
eventCombatPodKill = 78
eventCombatShipKillOwnFaction = 223
eventCombatShipKill = 77
eventCommandUnspecified = 153
eventCommodityExported = 298
eventCommodityImported = 297
eventCompleteSlashAgent = 146
eventCompleteSlashDistribution = 176
eventCompleteSlashPathDungeon = 179
eventCompleteDungeon = 145
eventCompletedTutorial = 155
eventConstellationCapitalSystemLost = 201
eventConstellationSovereigntyContested = 199
eventConstellationSovereigntyGained = 196
eventConstellationSovereigntyLost = 200
eventConstellationSovereigntyRecoveredInGrace = 198
eventContrabandTrafficking = 126
eventContractMarkedAsFinished = 212
eventContractDelete = 187
eventControlTowerSovereignityClaimStatusChanged = 255
eventCreatedBySlashCommand = 358
eventCreatedItem = 91
eventCreditGift = 22
eventCynosuralGeneratorArrayJump = 208
eventDeleteBookmark = 116
eventDeployment = 19
eventDecay = 49
eventDeclineApplication = 139
eventDeclinedMission = 120
eventDeployPermissions = 163
eventDepositSelected = 295
eventDerivedModificationNegative = 83
eventDerivedModificationPositive = 82
eventDividendsPayed = 193
eventDock = 4
eventEditItemFrom = 141
eventEditItemTo = 142
eventEditItem = 140
eventErrorControlTower = 151
eventEditShipName = 213
eventEditAssemblyLine = 170
eventEnterSlashAgent = 144
eventEnterSlashDistribution = 175
eventEnterSlashPathDungeon = 178
eventEnterDungeon = 143
eventEpicArcCompleted = 244
eventEpicArcStarted = 243
eventEpicArcTerminated = 261
eventExitTimeSetTo = 268
eventExitTimeChanged = 262
eventExitTimeChangedBy = 269
eventExpireSlashDistribution = 186
eventExpireSlashPathDungeon = 180
eventFailSlashAgent = 310
eventFailedMission = 87
eventFittedItemExpired = 174
eventForcefieldSettings = 161
eventFreeSkillPointsSet = 308
eventFreeSkillPointsUsed = 307
eventGag = 20
eventGaveCredits = 327
eventGaveDogmaTypeID = 329
eventGivenSlashPathDungeon = 181
eventGMCalendarCcpEvent = 302
eventGMCalendarDelete = 300
eventGMCalendarEdit = 299
eventGMCalendarRecover = 301
eventGMCertificateGranted = 232
eventGMCertificateRevoked = 233
eventGMCommodityGift = 284
eventGMCommodityTake = 285
eventGMDeletionSlashPathDungeon = 182
eventGMDepositInstall = 296
eventGMGiveSkill = 39
eventGMLPChanged = 205
eventGMMailDeleteBy = 275
eventGMMailUndeleteBy = 276
eventGMMassMedalRemoval = 241
eventGMMedalEdit = 242
eventGMMedalRemoved = 240
eventGMPinCreated = 292
eventGMPinRemoved = 294
eventGMResearchEdit = 189
eventGMReverseFreeSkillPointsUsed = 309
eventGMUnrentOffice = 211
eventHubExploded = 281
eventHadShipBoardedBy = 185
eventHubInvulnerable = 267
eventHubVulnerable = 266
eventImplantAdded = 94
eventImplantRemoved = 95
eventInitialCorpAgent = 52
eventInitialFactionAlly = 70
eventInitialFactionCorp = 54
eventInitialFactionEnemy = 69
eventInsuranceNoPayoutGCC = 343
eventItemOwnerChanged = 93
eventItemReverseRedeemed = 62
eventJetcanStolenFrom = 183
eventJoinMinigame = 235
eventJoinAlliance = 134
eventJoinCorporation = 44
eventJump = 6
eventLPExchange = 322
eventLPRewardPoolLost = 314
eventLPRewardPoolPayedOut = 313
eventLPRewardStoredInPool = 312
eventLPGainedFromMission = 203
eventLPPaidForOffer = 204
eventLeaveAlliance = 135
eventLeaveCorporation = 14
eventLeaveMinigame = 237
eventLootGift = 23
eventLootTrackingTookFromContainer = 214
eventLootTrackingTookFromContainerNotTheirs = 215
eventLose = 236
eventMakeSuper = 128
eventMailingListChangeOwner = 278
eventMailingListDelete = 277
eventMassRedeemTokenAdded = 247
eventMassRedeemTokenExpired = 248
eventMassTokenClaimed = 254
eventNpcAttackPoliceArrivedWithLowFactionStanding = 48
eventNpcAttackPoliceArrivedWithLowSecurityStatus = 47
eventNoLongerExecutor = 137
eventOfferClosed = 334
eventOfferDeleted = 335
eventOfferEdit = 332
eventOfferError = 331
eventOfferExpired = 121
eventOfferPublished = 333
eventOfferedMission = 118
eventOutpostInvulnerable = 265
eventOutpostMadeInvulnerable = 271
eventOutpostMadeVulnerable = 270
eventOutpostVulnerable = 264
eventOwnerChangedBy = 272
eventOwnerSetRelationshipLevelToAContactToANewLevel = 303
eventPinCreated = 291
eventPinRemoved = 293
eventPassword = 160
eventPirateKillSecurityStatus = 89
eventPlayerCorpSetStanding = 68
eventPlayerSetStanding = 65
eventPlayerChange = 152
eventPodKill = 9
eventPodKilled = 10
eventPortalArrayJumpMovement = 209
eventPortalArrayJumpStarbase = 210
eventPortedStanding = 85
eventPrepareRemove = 40
eventPromotionFactionStandingIncrease = 216
eventPropertyDamageOwnFaction = 227
eventPropertyDamage = 154
eventQueuedSkillTrainingCompleted = 53
eventQuitCeoPosition = 17
eventQuit = 119
eventReimburse = 113
eventRemoveCorporation = 13
eventRemoveEmploymentRecord = 150
eventRemove = 3
eventRestore = 43
eventRecalcEntityKills = 58
eventRecalcMissionFailure = 61
eventRecalcMissionSuccess = 55
eventRecalcPirateKills = 57
eventRecalcPlayerSetStanding = 67
eventRecommendationLetterUsed = 60
eventRedeemTokenAddedToUser = 249
eventRedeemTokenRemovedFromUser = 250
eventRemoveAlliance = 132
eventRemoveFacwar = 218
eventRemovingAContact = 304
eventRewardDisqualified = 315
eventRigDestroyedManuallyByPilot = 192
eventRocketCanBurnUp = 288
eventRocketCanClaimed = 290
eventRocketCanLaunch = 286
eventRocketCanSpawn = 287
eventRocketCanUnburnedup = 289
eventSBUExploded = 279
eventSBUOffline = 257
eventSBUOnline = 256
eventSelect = 2
eventSceneAdded = 320
eventSceneAddedToOther = 321
eventSelfDestruct = 42
eventSentrySettings = 162
eventServerPythonConsole = 188
eventSetStatusActive = 219
eventSetStatusLeaving = 220
eventShipHideCompleted = 342
eventShipHideExtendTimer = 341
eventShipHideStartTimer = 337
eventShipHideStartTimerModule = 340
eventShipHideStartTimerPve = 338
eventShipHideStartTimerPvp = 339
eventShipKill = 26
eventShipKilled = 27
eventSingletonItemMadecore = 92
eventSkillInjected = 56
eventSkillQueueHaltedLapsed = 260
eventSkillClonePenalty = 34
eventSkillGift = 24
eventSkillReceivedInCharacterCreation = 33
eventSkillRemoval = 177
eventSkillTaskMaster = 35
eventSkillTrainingCancelled = 38
eventSkillTrainingComplete = 37
eventSkillTrainingStarted = 36
eventSlashBlueprint = 357
eventSlashCreateItem = 354
eventSlashFit = 355
eventSlashLoad = 356
eventSlashSetStanding = 84
eventSovereigntyClaimed = 197
eventSovereigntyLost = 194
eventSpawnBlockedByOtherPlayer = 59
eventStandingCorrection = 96
eventStandingRollback = 98
eventStandingReset = 25
eventStarbaseStructureControlLost = 207
eventStarbaseStructureControlTaken = 206
eventStartMiniGame = 239
eventStartedResearch = 103
eventStartedTutorial = 245
eventStateChange = 159
eventStationMoveSystemFull = 234
eventStationMove = 7
eventStoleFromJetcan = 171
eventStoppedResearch = 104
eventSucceededMission = 86
eventSuicide = 11
eventSystemInfluenceChanged = 319
eventSystemMove = 8
eventTacticalSiteConquered = 230
eventTacticalSiteDefended = 229
eventTCUExploded = 280
eventTCUInvulnerable = 283
eventTCUOffline = 259
eventTCUOnline = 258
eventTCUVulnerable = 282
eventTaleEndedGmPlayersLose = 318
eventTaleEndedGmPlayersWin = 324
eventTaleEndedPlayerWins = 317
eventTaleExpiredNoRewards = 323
eventTaleStarted = 316
eventTokenClaimed = 253
eventTokenOwnershipChanged = 63
eventTokenQuantityDecreased = 64
eventTookCredits = 328
eventTookDogmaTypeID = 330
eventTransaction = 129
eventTutorialAgentInitial = 81
eventTypeAddedToWhitelist = 251
eventTypeRemovedFromWhitelist = 252
eventUndock = 5
eventUngag = 21
eventUnrentOffice = 344
eventUpdateMember = 148
eventUpdateSkill = 32
eventUpdateStanding = 45
eventUpgradeInstalledBy = 273
eventUpgradeInstalled = 274
eventUsagePermissions = 164
eventWinMinigame = 238
eventWormholeJump = 130
eventEnableBillTypeAutopay = 347
eventDisableBillTypeAutopay = 348
eventAutopaySuccess = 349
eventAutopayFailure = 350
eventControlTowerAnchored = 364
eventControlTowerUnanchored = 365
eventControlTowerDestroyed = 366
eventAddSkill = 10301
eventDecreaseExtraSkillPoints = 10304
eventIncreaseExtraSkillPoints = 10303
eventInsertAutoRecreate = 10006
eventInsert = 10001
eventPrimaryMarketPurchase = 10203
eventRemoveAllSkills = 10305
eventRemoveSkill = 10302
eventUpdateMachine = 10003
eventUpdatePortSlashProcess = 10005
eventUpdateServerStatus = 10004
eventUpdateStatus = 10002
eventUpdateQuantity = 10201
eventUseQuantity = 10202
*/

#endif  // EVE_TYPEDEFS_H