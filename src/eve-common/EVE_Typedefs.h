
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
    itemTypeCredits = 29,
    itemTypeJetCan  = 23,
    itemCivilianMiner = 3651,
    AllianceTypeID  = 16159,
    itemTypeBillboard = 11136,
    itemCloneAlpha  = 164
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
    raceAmmatar     = 6,        // not an actual race, but combo of Minmatar and Amarr
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

/*
locationAbstract = 0
locationSystem = 1
locationBank = 2
locationTemp = 5
locationTrading = 7
locationGraveyard = 8
locationUniverse = 9
locationHiddenSpace = 9000001
locationJunkyard = 10
locationCorporation = 13
locationSingletonJunkyard = 25
locationTradeSessionJunkyard = 1008
locationCharacterGraveyard = 1501
locationCorporationGraveyard = 1502
locationRAMInstalledItems = 2003
locationAlliance = 3007
locationMinJunkyardID = 1000
locationMaxJunkyardID = 1999
*/
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
    salvageWreck    = 1,
    salvageData     = 2,
    salvageArch     = 3,
    salvageAncient  = 4,
    salvageOther    = 5
} SalvageTypes;

// mostly pirate corps for now...
typedef enum {
    corpRogueDrones         = 1000001,
     corpCBD    = 1000002,
     corpPromptDelivery     = 1000003,
     corpYtiri   = 1000004,
     corpHyasyoda    = 1000005,
     corpDeepCoreMining   = 1000006,
     corpPoksuMineralGroup     = 1000007,
     corpMinedrill   = 1000008,
     corpCaldariProvisions  = 1000009,
     corpKaalakiota  = 1000010,
     corpWiyrkomi    = 1000011,
     corpTopDown    = 1000012,
     corpRapidAssembly  = 1000013,
     corpPerkone     = 1000014,
     corpCaldariSteel   = 1000015,
     corpZainou  = 1000016,
     corpNugoeihuvi  = 1000017,
     corpEchelonEntertainment   = 1000018,
     corpIshukone    = 1000019,
     corpLaiDai     = 1000020,
     corpZeroGResearch    = 1000021,
     corpPropelDynamics     = 1000022,
     corpExpertDistribution     = 1000023,
     corpCBDSell   = 1000024,
     corpSukuuvestaa     = 1000025,
     corpCaldariConstructions   = 1000026,
     corpExpertHousing  = 1000027,
     corpCaldariFundsUnlimited     = 1000028,
     corpStateRegionBank   = 1000029,
     corpModernFinances     = 1000030,
     corpChiefExecutivePanel   = 1000031,
     corpMercantileClub     = 1000032,
     corpCaldariBusinessTribunal   = 1000033,
     corpHouseofRecords    = 1000034,
     corpCaldariNavy    = 1000035,
     corpInternalSecurity   = 1000036,
     corpLaiDaiProtection  = 1000037,
     corpIshukoneWatch  = 1000038,
     corpHomeGuard  = 1000039,
     corpPeaceandOrderUnit    = 1000040,
     corpSpacelanePatrol    = 1000041,
     corpWiyrkomiPeaceCorps    = 1000042,
     corpCorporatePoliceForce  = 1000043,
     corpSchoolofAppliedKnowledge     = 1000044,
     corpScienceandTradeInstitute     = 1000045,
     corpSebiestortribe     = 1000046,
     corpKrusualTribe   = 1000047,
     corpVherokiorTribe     = 1000048,
     corpBrutorTribe    = 1000049,
     corpRepublicParliament     = 1000050,
     corpRepublicFleet  = 1000051,
     corpRepublicJustice     = 1000052,
     corpUrbanManagement    = 1000053,
     corpRepublicSecurityServices  = 1000054,
     corpMinmatarMining     = 1000055,
     corpCoreComplexion    = 1000056,
     corpBoundlessCreation  = 1000057,
     corpEifyrandCo   = 1000058,
     corpSixKinDevelopment     = 1000059,
     corpNativeFreshfood    = 1000060,
     corpFreedomExtension   = 1000061,
     corpLeisureGroup   = 1000062,
     corpAmarrConstructions     = 1000063,
     corpCarthumConglomerate    = 1000064,
     corpImperialArmaments  = 1000065,
     corpViziam  = 1000066,
     corpZoarandSons   = 1000067,
     corpNobleAppliances    = 1000068,
     corpDuciaFoundry   = 1000069,
     corpHZORefinery    = 1000070,
     corpInherentImplants   = 1000071,
     corpImperialShipment   = 1000072,
     corpAmarrCertifiedNews    = 1000073,
     corpJointHarvesting    = 1000074,
     corpNurtura     = 1000075,
     corpFurtherFoodstuffs  = 1000076,
     corpRoyalAmarrInstitute   = 1000077,
     corpImperialChancellor     = 1000078,
     corpAmarrCivilService     = 1000079,
     corpMinistryofWar     = 1000080,
     corpMinistryofAssessment  = 1000081,
     corpMinistryofInternalOrder  = 1000082,
     corpAmarrTradeRegistry    = 1000083,
     corpAmarrNavy  = 1000084,
     corpCourtChamberlain   = 1000085,
     corpEmperorFamily  = 1000086,
     corpKadorFamily    = 1000087,
     corpSarumFamily    = 1000088,
     corpKorAzorFamily     = 1000089,
     corpArdishapurFamily   = 1000090,
     corpTashMurkonFamily  = 1000091,
     corpCivicCourt     = 1000092,
     corpTheologyCouncil    = 1000093,
     corpTransStellarShipping   = 1000094,
     corpFederalFreight     = 1000095,
     corpInnerZoneShipping     = 1000096,
     corpMaterialAcquisition    = 1000097,
     corpAstralMining  = 1000098,
     corpCombinedHarvest    = 1000099,
     corpQuafeCompany   = 1000100,
     corpCreoDron    = 1000101,
     corpRodenShipyards     = 1000102,
     corpAllotekIndustries  = 1000103,
     corpPotequePharmaceuticals     = 1000104,
     corpImpetus     = 1000105,
     corpEgonics    = 1000106,
     corpTheScope   = 1000107,
     corpChemalTech     = 1000108,
     corpDuvolleLaboratories    = 1000109,
     corpFedMart     = 1000110,
     corpAliastra    = 1000111,
     corpBankofLuminaire   = 1000112,
     corpPendInsurance  = 1000113,
     corpGarounInvestmentBank  = 1000114,
     corpUniversityofCaille    = 1000115,
     corpPresident   = 1000116,
     corpSenate  = 1000117,
     corpSupremeCourt   = 1000118,
     corpFederalAdministration  = 1000119,
     corpFederationNavy     = 1000120,
     corpFederalIntelligenceOffice     = 1000121,
     corpFederationCustoms  = 1000122,
     corpAmmatarFleet   = 1000123,
     corpArchangels               = 1000124,
     corpCONCORD             = 1000125,
     corpAmmatarConsulate   = 1000126,
     corpGuristas            = 1000127,
     corpMordusLegion        = 1000128,
     corpORE                 = 1000129,
     corpSoE                 = 1000130,
     corpSocietyofCT             = 1000131,
     corpSCC                 = 1000132,
     corpSalvationAngels    = 1000133,
     corpBloodRaider         = 1000134,
     corpSerpentis           = 1000135,
     corpGuardianAngels     = 1000136,
     corpDED                 = 1000137,
     corpDominations     = 1000138,
     corpFoodRelief     = 1000139,
     corpGenolution  = 1000140,
     corpGuristasProduction     = 1000141,
     corpImpro   = 1000142,
     corpInnerCircle    = 1000143,
     corpIntakiBank     = 1000144,
     corpIntakiCommerce     = 1000145,
     corpIntakiSpacePolice     = 1000146,
     corpIntakiSyndicate    = 1000147,
     corpInterbus            = 1000148,
     corpJoveNavy   = 1000149,
     corpJovianDirectorate  = 1000150,
     corpKhanidInnovation    = 1000151,
     corpKhanidTransport     = 1000152,
     corpKhanidWorks         = 1000153,
     corpNefantarMiners  = 1000154,
     corpProsper     = 1000155,
     corpKhanidNavy          = 1000156,
     corpSerpentisInquest   = 1000157,
     corpShapeset    = 1000158,
     corpTheSanctuary   = 1000159,
     corpThukkerMix          = 1000160,
     //corpSanshas             = 1000161,
     corpTrueCreations  = 1000161,
     corpTruePower  = 1000162,
     corpTrustPartners  = 1000163,
     corpXSense     = 1000164,
     corpHedionUniversity   = 1000165,
     corpImperialAcademy    = 1000166,
     corpStateWarAcademy   = 1000167,
     corpFederalNavyAcademy    = 1000168,
     corpCenterforAdvancedStudies     = 1000169,
     corpRepublicMilitarySchool    = 1000170,
     corpRepublicUniversity     = 1000171,
     corpPatorTechSchool   = 1000172,
     corpMaterialInstitute  = 1000177,
     corpAcademyofAggressiveBehaviour     = 1000178,
     corp24ImperialCrusade   = 1000179,
     corpStateProtectorate  = 1000180,
     corpFederalDefenseUnion   = 1000181,
     corpTribalLiberationForce     = 1000182
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


#endif  // EVE_TYPEDEFS_H