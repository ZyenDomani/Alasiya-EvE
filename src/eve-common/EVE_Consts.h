/*
 *  misc static consts
 */

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

static const GPoint NULL_ORIGIN(0,0,0);  // common place for a zero-value gpoint
static const GVector NULL_ORIGIN_V(0,0,0);

const EvilNumber EVIL_SKILL_BASE_POINTS(250);

const int32 ITEM_DB_SAVE_TIMER_EXPIRY(10);

static const float TIC_DURATION_IN_SECONDS(1000);

static const uint32 minWarpDistance(100000);    // 100km

//   based on client code...
static const uint64 ONE_LIGHTYEAR = 9460000000000000UL;  // in meters
static const uint64 ONE_AU_IN_METERS = 149597870700L;     // 1 astronomical unit in meters, per EVElopedia: http://wiki.eveonline.com/en/wiki/Astronomical_Unit


// Cosmic Managers constants here
static const uint32 ASTEROID_GROWTH_INTERVAL_MS = 3600000;

