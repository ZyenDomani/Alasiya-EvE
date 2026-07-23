/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    For the latest information visit http://evemu.org
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:     Zhur
    Updates:    Allan
*/

#pragma once
#ifndef __UTILS_TIME_H__INCL__
#define __UTILS_TIME_H__INCL__

#include <chrono>
#include <cstdint>

#include "../eve-core.h"

/*
uSEC = 10L
MSEC = 10000L
SEC = 10000000L
MIN = (SEC * 60L)
HOUR = (MIN * 60L)
DAY = (HOUR * 24L)
MONTH = (30 * DAY)
YEAR = (12 * MONTH)
*/
namespace EvE {
    namespace Time {
        enum:int64_t {
            uSecond     = 10L,
            mSecond     = 10000L,        //1000
            Second      = 10000000L,
            Minute      = (Second * 60L),
            Hour        = (Minute * 60L),
            Day         = (Hour * 24L),
            Week        = (Day * 7L),
            Month       = (Day * 30L),
            Year        = (Day * 365L)
        };
    }

    struct TimeParts {
        uint16 year;
        uint8 month;
        uint8 wd;       // day of week
        uint8 wn;       // week of year
        uint8 day;
        uint8 hour;
        uint8 min;
        uint8 sec;
        uint16 dy;      // day of year
        uint16 ms;
    };
}

extern const int64 Win32Time_Minute;
extern const int64 Win32Time_Hour;

typedef std::chrono::duration<int64, std::ratio<1, 10000000>> duration_100ns;
constexpr uint64_t FILETIME_EPOCH_OFFSET = 116444736000000000ULL;


extern std::string Win32TimeToString(int64 win32t);
void Win32TimeToUnixTime( int64 win32t, time_t &unix_time, uint32 &nsec );

// returns delta between time and now, in hours
int32 GetElapsedHours(int64 time);

// return elapsed time formatted in units
std::string GetUTimeTillNow(double fromTime);
// return elapsed time formatted in units
std::string GetMTimeTillNow(double fromTime);

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string currentDateTime();

// break down given filetime into year, month, day, hour, min, sec, ms
EvE::TimeParts GetTimeParts(int64 filetime=0);  // also gives day of week, day of year, and week of year



// ============================================================================
// PART 1: Monotonic Performance Timers (Double & Int64 Hybrid)
// ============================================================================

//  this returns milliseconds
inline int64 GetSteadyTime() noexcept {  // -allan
    // Simulation of Windows GetTickCount() returning integer milliseconds.
    using namespace std::chrono;

    // Crucial: Swapped to steady_clock to prevent NTP time jumps
    // and fixed the explicit type to assist compiler optimization.
    static const steady_clock::time_point bootTime = steady_clock::now();

    auto current = steady_clock::now();
    return duration_cast<milliseconds>(current - bootTime).count();
}

inline double GetTimeSeconds() noexcept {
    // returns <s>.<ms><us><ns>
    using namespace std::chrono;
    static const steady_clock::time_point bootTime = steady_clock::now();

    return duration<double>(steady_clock::now() - bootTime).count();
}

inline double GetTimeMSeconds() noexcept {
    // returns <s><ms>.<us><ns>
    using namespace std::chrono;
    static const steady_clock::time_point bootTime = steady_clock::now();

    return duration<double, std::milli>(steady_clock::now() - bootTime).count();
}

inline int64 GetTimeUSeconds() noexcept {
    // returns <s><ms><us>
    using namespace std::chrono;
    static const steady_clock::time_point bootTime = steady_clock::now();

    auto delta = steady_clock::now() - bootTime;
    // On GCC 4.9.2 / Linux, steady_clock tracks raw nanoseconds natively.
    // Dividing by 1000 yields microseconds with near-zero assembly instructions.
    return delta.count() / 1000LL;
}

inline int64 GetTimeNSeconds() noexcept {
    // returns <s><ms><us><ns>
    using namespace std::chrono;
    static const steady_clock::time_point bootTime = steady_clock::now();

    auto delta = steady_clock::now() - bootTime;
    // Returns native nanosecond register values instantly.
    return delta.count();
}

// ============================================================================
// PART 2: Wall-Clock File Timers (Unix Epoch to FILETIME format)
// ============================================================================

// 100-nanosecond block representation required for custom FILETIME calculations
using duration_100ns = std::chrono::duration<int64, std::ratio<1, 10000000>>;

// this returns 100 nanosecond resolution in filetime format
inline int64 GetFileTimeNow() noexcept {
     // replacement for Win32TimeNow()
    using namespace std::chrono;

    auto now = system_clock::now();
    auto duration = now.time_since_epoch();

    int64 epoch_interval = duration_cast<duration_100ns>(duration).count();
    return epoch_interval + FILETIME_EPOCH_OFFSET;
}

// returns second resolution in filetime format
inline int64 GetFileTimeNowSeconds() noexcept {
    using namespace std::chrono;

    auto now = system_clock::now();
    auto duration = now.time_since_epoch();

    // Server Optimization: Extracted seconds early to bypass 18-digit division loops
    return duration_cast<seconds>(duration).count() + (FILETIME_EPOCH_OFFSET / 10000000LL);
}

#endif /* !__UTILS_TIME_H__INCL__ */
