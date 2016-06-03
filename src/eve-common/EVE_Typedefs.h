
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
    AllianceTypeID  = 16159
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
    racePirate      = 32
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
    standingPirateKillSecurityStatus        = 89,
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

/*
 * corpactivityEducation = 18
 * corpactivityEntertainment = 8
 * corpactivityMilitary = 5
 * corpactivitySecurity = 16
 * corpactivityTrading = 12
 * corpactivityWarehouse = 10
 * corpDivisionDistribution = 22
 * corpDivisionMining = 23
 * corpDivisionSecurity = 24
 */

//from market_keyMap
typedef enum {
    accountingKeyCash           = 1000,
    accountingKeyCash2          = 1001,     //walletDivision2...
    accountingKeyCash3          = 1002,
    accountingKeyCash4          = 1003,
    accountingKeyCash5          = 1004,
    accountingKeyCash6          = 1005,
    accountingKeyCash7          = 1006,
    accountingKeyProperty       = 1100,
    accountingKeyAUR            = 1200,
    accountingKeyAUR2           = 1201,     //walletDivision2...
    accountingKeyAUR3           = 1202,
    accountingKeyAUR4           = 1203,
    accountingKeyAUR5           = 1204,
    accountingKeyAUR6           = 1205,
    accountingKeyAUR7           = 1206,
    accountingKeyEscrow         = 1500,
    accountingKeyReceivables    = 1800,
    accountingKeyPayables       = 2000,
    accountingKeyGold           = 2010,
    accountingKeyEquity         = 2900,
    accountingKeySales          = 3000,
    accountingKeyPurchases      = 4000
} EVEAccountKeys;

//  -allan 20Dec14
typedef enum {
    refSkipLog = -1,
    refUndefined = 0,
    refPlayerTrading = 1,
    refMarketTransaction = 2,
    refGMCashTransfer = 3,
    refATMWithdraw = 4,
    refATMDeposit = 5,
    refBackwardCompatible = 6,
    refMissionReward = 7,
    refCloneActivation = 8,
    refInheritance = 9,
    refPlayerDonation = 10,
    refCorporationPayment = 11,
    refDockingFee = 12,
    refOfficeRentalFee = 13,
    refFactorySlotRentalFee = 14,
    refRepairBill = 15,
    refBounty = 16,
    refBountyPrize = 17,
    refInsurance = 19,
    refMissionExpiration = 20,
    refMissionCompletion = 21,
    refShares = 22,
    refCourierMissionEscrow = 23,
    refMissionCost = 24,
    refAgentMiscellaneous = 25,
    refPaymentToLPStore = 26,
    refAgentLocationServices = 27,
    refAgentDonation = 28,
    refAgentSecurityServices = 29,
    refAgentMissionCollateralPaid = 30,
    refAgentMissionCollateralRefunded = 31,
    refAgentMissionReward = 33,
    refAgentMissionTimeBonusReward = 34,
    refCSPA = 35,
    refCSPAOfflineRefund = 36,
    refCorporationAccountWithdrawal = 37,
    refCorporationDividendPayment = 38,
    refCorporationRegistrationFee = 39,
    refCorporationLogoChangeCost = 40,
    refReleaseOfImpoundedProperty = 41,
    refMarketEscrow = 42,
    refMarketFinePaid = 44,
    refBrokerfee = 46,
    refAllianceRegistrationFee = 48,
    refWarFee = 49,
    refAllianceMaintainanceFee = 50,
    refContrabandFine = 51,
    refCloneTransfer = 52,
    refAccelerationGateFee = 53,
    refTransactionTax = 54,
    refJumpCloneInstallatio,nFee = 55,
    refManufacturing = 56,
    refResearchingTechnology = 57,
    refResearchingTimeProductivity = 58,
    refResearchingMaterialProductivity = 59,
    refCopying = 60,
    refDuplicating = 61,
    refReverseEngineering = 62,
    refContractAuctionBid = 63,
    refContractAuctionBidRefund = 64,
    refContractCollateral = 65,
    refContractRewardRefund = 66,
    refContractAuctionSold = 67,
    refContractReward = 68,
    refContractCollateralRefund = 69,
    refContractCollateralPayout = 70,
    refContractPrice = 71,
    refContractBrokersFee = 72,
    refContractSalesTax = 73,
    refContractDeposit = 74,
    refContractDepositSalesTax = 75,
    refSecureEVETimeCodeExchange = 76,
    refContractAuctionBidCorp = 77,
    refContractCollateralCorp = 78,
    refContractPriceCorp = 79,
    refContractBrokersFeeCorp = 80,
    refContractDepositCorp = 81,
    refContractDepositRefund = 82,
    refContractRewardAdded = 83,
    refContractRewardAddedCorp = 84,
    refBountyPrizes = 85,
    refCorporationAdvertisementFee = 86,
    refMedalCreation = 87,
    refMedalIssuing = 88,
    refAttributeRespecification = 90,
    refSovereignityRegistrarFee = 91,
    refCorporationTaxNpcBounties = 92,
    refCorporationTaxAgentRewards = 93,
    refCorporationTaxAgentBonusRewards = 94,
    refSovereignityUpkeepAdjustment = 95,
    refPlanetaryImportTax = 96,
    refPlanetaryExportTax = 97,
    refPlanetaryConstruction = 98,
    refRewardManager = 99,
    refBountySurcharge = 101,
    refContractReversal = 102,
    refCorporationTaxRewards = 103,
    refStorePurchase = 106,
    refStoreRefund = 107,
    refPlexConversion = 108,
    refAurumGiveAway = 109,
    refAurumTokenConversion = 111,
    refMaxEve = 10000
} JournalRefType;

//  -allan 7Jul14
typedef enum {
    WH_RegionMin           = 11000000,
    WH_RegionMax           = 11999999,
    WH_ConstellationMin    = 21000000,
    WH_ConstellationMax    = 21999999,
    WH_SystemMin           = 31000000,
    WH_SystemMax           = 31999999
} mapWormholeValues;

//  -allan 7Jul14
typedef enum {
    MissionAllocated    = 0,
    MissionOffered      = 1,
    MissionAccepted     = 2,
    MissionFailed       = 3
} MissionState;

//  -allan 7Jul14
typedef enum {
    DungeonStarted      = 0,
    DungeonCompleted    = 1,
    DungeonFailed       = 2
} DungeonState;

//  -allan 7Jul14
typedef enum {
    DockingInvul     = 3000,
    JumpingInvul     = 5000,
    WarpingOutInvul  = 5000,
    WarpingInInvul   = 10000,
    UndockingInvul   = 15000,
    RestoringInvul   = 60000
} InvulTimer;

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
    npcDivisionAccounting = 1,
    npcDivisionAdministration = 2,
    npcDivisionAdvisory = 3,
    npcDivisionArchives = 4,
    npcDivisionAstrosurveying = 5,
    npcDivisionCommand = 6,
    npcDivisionDistribution = 7,
    npcDivisionFinancial = 8,
    npcDivisionIntelligence = 9,
    npcDivisionInternalSecurity = 10,
    npcDivisionLegal = 11,
    npcDivisionManufacturing = 12,
    npcDivisionMarketing = 13,
    npcDivisionMining = 14,
    npcDivisionPersonnel = 15,
    npcDivisionProduction = 16,
    npcDivisionPublicRelations = 17,
    npcDivisionRD = 18,
    npcDivisionSecurity = 19,
    npcDivisionStorage = 20,
    npcDivisionSurveillance = 21
} npcDivisions;

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
    factionRogueDrones   = 500021,
    factionUnknown       = 500022
} FactionDef;

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
    corpSerpentis           = 1000135
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
    cacheSolarSystemOffices = 8
} cacheFlags;
#endif  // EVE_TYPEDEFS_H