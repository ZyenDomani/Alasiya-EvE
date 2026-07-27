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
