/* Alasiya EvE Constants
 *   this file is a common location for all static-type defined data
 */

#ifndef EVE_CONSTANTS_H
#define EVE_CONSTANTS_H

#include <array>

#include "../eve-core/eve-compat.h"
#include "../eve-core/math/Vector3d.h"

// define default home page for IGB
const std::string HomePageURL = "http://eve.alasiya.net/";

/*
 *  misc static consts
 */

static const char numList[]     = "0123456789";
static const char hexList[]     = "0123456789ABCDEF";
static const char alphaList[]   = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char asciiList[]   = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ~!@#$%^&*()[]{};:'<>?";

/* marshaled Python string "None" */
static const uint8 marshaledNone[] = { 0x74, 0x04, 0x00, 0x00, 0x00, 0x4E, 0x6F, 0x6E, 0x65 };

static const Vector3d NULL_ORIGIN(0.0,0.0,0.0);  // common place for a zero-value vector

constexpr double Destiny_k      = 1000000.0;

//   based on client code...
constexpr float TIC_DURATION_IN_SECONDS = 1000;       // not used yet
constexpr uint32 MIN_WARP_DISTANCE = 150000;    // client and live defined as 150km
constexpr uint16 SHIP_PROCESS_TICK_MS = 5000;    // 5s
constexpr double ORBITAL_PRECESSION = 0.001;
constexpr int64 ONE_LIGHTYEAR = 9460000000000000;  // in meters
constexpr int64 ONE_AU_IN_METERS = 149597870700;     // 1 astronomical unit in meters, per EVElopedia: http://wiki.eveonline.com/en/wiki/Astronomical_Unit
constexpr int64 STATION_HANGAR_MAX_CAPACITY = 9000000000000000;  //per client
constexpr int64 NEXT_DUNGEON_ROOM_DIST = 50000000000; // 50M meters as generic distance between rooms
constexpr int64 MAX_MARKET_PRICE = 9223372036854;  //max int64/1000000  (9223372036854775807/1000000)
constexpr int32 INCAPACITATION_DISTANCE = 250000;    // drone to ship max distance.  after this, drone goes Offline and is considered 'lost'

constexpr float MODULE_ONLINE_IN_SPACE = 0.75;     // onlining modules while NOT docked or using fitting services will take 75% of capacitor capacity.

// Cosmic Managers constants here  *not used yet*
constexpr uint32 ASTEROID_GROWTH_INTERVAL_MS = 3600000;  /* this is grow check in ms (1d) */

// constants for destiny movement checks
constexpr float ASF_CHECK = 0.0011f;

// verify that NO ONE tries to use "ccp" in their name
// also check for mysql commands among other dumb shit
static const std::array<std::string, 28> badWords {
    {"ccp",
    "admin",
    "fucker",
    "cunt",
    "concat",
    "collate",
    "select",
    "drop",
    "truncate",
    "count",
    "char",
    "load",
    "ascii",
    "union",
    "having",
    "group",
    "insert",
    "cast",
    "version",
    "exec ",
    "benchmark",
    "create",
    "md5",
    "sha1",
    "encode",
    "compress",
    "row_",
    "bulk"}
};
// check for common mysql injection hacks
//  special chars are illegal just because
static const std::array<std::string, 18> badChars {
    {";",
    "--",
    "#",
    "/*",
    "/0",
    "0x",
    "|",
    "' ",
    "+",
    "@",
    "!",
    "$",
    "%",
    "^",
    "&",
    "*",
    "(",
    ")"}
};

static const std::array<std::string, 16> badCharsSearch {
    {";",
    "--",
    "#",
    "/*",
    "/0",
    "0x",
    "|",
    "' ",
    "+",
    "@",
    "!",
    "$",
    "^",
    "&",
    "(",
    ")"}
};
#endif  // EVE_CONSTANTS_H

/*  misc data
 * radius constants
 * moon    =  1737km
 * mars    =  3390km
 * earth   =  6371km
 * jupiter = 69911km
 *
 * gravity constants
 * moon    =  1.622 m/s^2
 * mars    =  3.711 m/s^2
 * earth   =  9.807 m/s^2
 * jupiter = 24.790 m/s^2
 */


/*
cacheEosNpcToNpcStandings = 109998
cacheAutAffiliates = 109997
cacheAutCdkeyTypes = 109996
cacheTutCategories = 200006
cacheTutCriterias = 200003
cacheTutTutorials = 200001
cacheTutActions = 200009
cacheDungeonArchetypes = 300001
cacheDungeonDungeons = 300005
cacheDungeonEntityGroupTypes = 300004
cacheDungeonEventMessageTypes = 300017
cacheDungeonEventTypes = 300015
cacheDungeonSpawnpoints = 300012
cacheDungeonTriggerTypes = 300013
cacheInvCategories = 600001
cacheInvContrabandTypes = 600008
cacheInvGroups = 600002
cacheInvTypes = 600004
cacheInvTypeMaterials = 600005
cacheInvTypeReactions = 600010
cacheInvWreckUsage = 600009
cacheInvMetaGroups = 600006
cacheInvMetaTypes = 600007
cacheDogmaAttributes = 800004
cacheDogmaEffects = 800005
cacheDogmaExpressionCategories = 800001
cacheDogmaExpressions = 800003
cacheDogmaOperands = 800002
cacheDogmaTypeAttributes = 800006
cacheDogmaTypeEffects = 800007
cacheDogmaUnits = 800009
cacheEveMessages = 1000001
cacheInvBlueprintTypes = 1200001
cacheMapRegions = 1409999
cacheMapConstellations = 1409998
cacheMapSolarSystems = 1409997
cacheMapSolarSystemLoadRatios = 1409996
cacheLocationWormholeClasses = 1409994
cacheMapPlanets = 1409993
cacheMapSolarSystemJumpIDs = 1409992
cacheMapTypeBalls = 1400001
cacheMapWormholeClasses = 1400003
cacheMapLocationTypes = 1400004
cacheMapCelestialDescriptions = 1400008
cacheMapNebulas = 1400016
cacheMapLocationWormholeClasses = 1400002
cacheMapRegionsTable = 1400009
cacheMapConstellationsTable = 1400010
cacheMapSolarSystemsTable = 1400011
cacheNpcCommandLocations = 1600009
cacheNpcCommands = 1600005
cacheNpcDirectorCommandParameters = 1600007
cacheNpcDirectorCommands = 1600006
cacheNpcLootTableFrequencies = 1600004
cacheNpcCommandParameters = 1600008
cacheNpcTypeGroupingClassSettings = 1600016
cacheNpcTypeGroupingClasses = 1600015
cacheNpcTypeGroupingTypes = 1600017
cacheNpcTypeGroupings = 1600014
cacheNpcTypeLoots = 1600001
cacheRamSkillInfo = 1809999
cacheRamActivities = 1800003
cacheRamAssemblyLineTypes = 1800006
cacheRamAssemblyLineTypesCategory = 1800004
cacheRamAssemblyLineTypesGroup = 1800005
cacheRamCompletedStatuses = 1800007
cacheRamInstallationTypes = 1800002
cacheRamTypeRequirements = 1800001
cacheReverseEngineeringTableTypes = 1800009
cacheReverseEngineeringTables = 1800008
cacheShipInsurancePrices = 2000007
cacheShipTypes = 2000001
cacheStaOperations = 2209999
cacheStaServices = 2209998
cacheStaOperationServices = 2209997
cacheStaSIDAssemblyLineQuantity = 2209996
cacheStaSIDAssemblyLineType = 2209995
cacheStaSIDAssemblyLineTypeQuantity = 2209994
cacheStaSIDOfficeSlots = 2209993
cacheStaSIDReprocessingEfficiency = 2209992
cacheStaSIDServiceMask = 2209991
cacheStaStationImprovementTypes = 2209990
cacheStaStationUpgradeTypes = 2209989
cacheStaStations = 2209988
cacheStaStationsStatic = 2209987
cacheMktOrderStates = 2409999
cacheMktNpcMarketData = 2400001
cacheCrpRoles = 2809999
cacheCrpActivities = 2809998
cacheCrpNpcDivisions = 2809997
cacheCrpCorporations = 2809996
cacheCrpNpcMembers = 2809994
cacheCrpPlayerCorporationIDs = 2809993
cacheCrpTickerNamesStatic = 2809992
cacheNpcSupplyDemand = 2800001
cacheCrpRegistryGroups = 2800002
cacheCrpRegistryTypes = 2800003
cacheCrpNpcCorporations = 2800006
cacheAgentAgents = 3009999
cacheAgentCorporationActivities = 3009998
cacheAgentCorporations = 3009997
cacheAgentEpicMissionMessages = 3009996
cacheAgentEpicMissionsBranching = 3009995
cacheAgentEpicMissionsNonEnd = 3009994
cacheAgtContentAgentInteractionMissions = 3009992
cacheAgtContentFlowControl = 3009991
cacheAgtContentTalkToAgentMissions = 3009990
cacheAgtPrices = 3009989
cacheAgtResearchStartupData = 3009988
cacheAgtContentTemplates = 3000001
cacheAgentMissionsKill = 3000006
cacheAgtStorylineMissions = 3000008
cacheAgtContentCourierMissions = 3000003
cacheAgtContentExchangeOffers = 3000005
cacheAgentEpicArcConnections = 3000013
cacheAgentEpicArcMissions = 3000015
cacheAgentEpicArcs = 3000012
cacheAgtContentMissionExtraStandings = 3000020
cacheAgtContentMissionTutorials = 3000018
cacheAgtContentMissionLocationFilters = 3000021
cacheAgtOfferDetails = 3000004
cacheAgtOfferTableContents = 3000010
cacheChrSchools = 3209997
cacheChrRaces = 3200001
cacheChrBloodlines = 3200002
cacheChrAncestries = 3200003
cacheChrCareers = 3200004
cacheChrSpecialities = 3200005
cacheChrBloodlineNames = 3200010
cacheChrAttributes = 3200014
cacheChrFactions = 3200015
cacheChrDefaultOverviews = 3200011
cacheChrDefaultOverviewGroups = 3200012
cacheChrNpcCharacters = 3200016
cacheUserEspTagTypes = 4309999
cacheFacWarCombatZoneSystems = 4500006
cacheFacWarCombatZones = 4500005
cacheActBillTypes = 6400004
cachePetCategories = 8109999
cachePetQueues = 8109998
cachePetCategoriesVisible = 8109997
cacheGMQueueOrder = 8109996
cacheCertificates = 5100001
cacheCertificateRelationships = 5100004
cachePlanetSchematics = 7300004
cachePlanetSchematicsTypeMap = 7300005
cachePlanetSchematicsPinMap = 7300003
cacheBattleStatuses = 100509999
cacheBattleResults = 100509998
cacheBattleServerStatuses = 100509997
cacheBattleMachines = 100509996
cacheBattleClusters = 100509995
cacheMapDistrictCelestials = 100309999
cacheMapDistricts = 100300014
cacheMapBattlefields = 100300015
cacheMapLevels = 100300020
cacheMapOutposts = 100300022
cacheMapLandmarks = 100300023
*/
