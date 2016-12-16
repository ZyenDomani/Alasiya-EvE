/* Alasiya EvE Constants
 *   this file is a common location for all static-type defined data
 */


/*
 *  misc static consts
 */

static const char alphaList[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char numList[] = "0123456789";

/*  these are based on client settings of damage notification.
 * msg packets are
 *   " "  - to others
 *   "R"  - received
 *   "RD  - received with details
 */
static const char* DamageMessageIDs_Self[7] = {
    "AttackMiss1R",   //miss
    //AttackMiss1Banked  ?not sure here....'banked' means 'group weapons'  also in ship.modules.GetTurretSets in MakeSlimItem()
    "AttackHit1R",    //barely scratches
    "AttackHit2R",    //lightly hits
    "AttackHit3R",    //hits
    "AttackHit4R",    //aims well at you
    "AttackHit5R",    //places an excellent hit
    "AttackHit6R"     //strikes you perfectly, wrecking
};

static const char* DamageMessageIDs_SelfNamed[7] = {
    "AttackMiss1RD",   //miss
    "AttackHit1RD",    //barely scratches
    "AttackHit2RD",    //lightly hits
    "AttackHit3RD",    //hits
    "AttackHit4RD",    //aims well at you
    "AttackHit5RD",    //places an excellent hit
    "AttackHit6RD"     //strikes you perfectly, wrecking
};

static const char* DamageMessageIDs_Other[7] = {
    "AttackMiss1",   //miss
    "AttackHit1",    //barely scratches
    "AttackHit2",    //lightly hits
    "AttackHit3",    //hits
    "AttackHit4",    //aims well
    "AttackHit5",    //places an excellent hit
    "AttackHit6"     //strikes perfectly, wrecking
};

static const uint16 SHIP_PROCESS_TICK_MS = 5000;    // 5s

static const GPoint NULL_ORIGIN(0,0,0);  // common place for a zero-value gpoint
static const GVector NULL_ORIGIN_V(0,0,0);

const EvilNumber EVIL_SKILL_BASE_POINTS(250);

const int32 ITEM_DB_SAVE_TIMER_EXPIRY(10);

static const float TIC_DURATION_IN_SECONDS(1000);

static const uint32 minWarpDistance(100000);    // 100km

static const float onlineModInSpace(0.75);     // onling modules while NOT docked or using fitting services will take 75% of current capacitor.

//   based on client code...
static const uint64 ONE_LIGHTYEAR(9460000000000000UL);  // in meters
static const uint64 ONE_AU_IN_METERS(149597870700L);     // 1 astronomical unit in meters, per EVElopedia: http://wiki.eveonline.com/en/wiki/Astronomical_Unit
static const uint64 STATION_HANGAR_MAX_CAPACITY(9000000000000000);  //per client
static const double MAX_MARKET_PRICE(9223372036854);  //max int64/1000000  (9223372036854775807/1000000)
static const uint32 INCAPACITATION_DISTANCE(250000);

// Cosmic Managers constants here  *not used yet*
static const uint32 ASTEROID_GROWTH_INTERVAL_MS(3600000);  /* this is grow check in ms (1d) */

// gravitational constant
static const double Gc(6.6725985e-11);     //per client (changed from original 6.673e-11)

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
