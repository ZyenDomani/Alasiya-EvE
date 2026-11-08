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


#include "utils/utils_time.h"
#include <ctime>
#include <cmath>
#include <sstream>
#include <iomanip>

// Number of days in month in normal year
static const int daysOfMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static const int64 SECS_BETWEEN_EPOCHS = 11644473600LL;
static const int64 SECS_TO_100NS = 10000000L; // 10^7

const int64 Win32Time_Second = SECS_TO_100NS;
const int64 Win32Time_Minute = Win32Time_Second * 60;
const int64 Win32Time_Hour = Win32Time_Minute * 60;

std::string Win32TimeToString(int64 win32t)
{
    std::time_t unix_time;
    uint32 nsec = 0;
    Win32TimeToUnixTime(win32t, unix_time, nsec);

    struct tm local_tm;
    std::memset(&local_tm, 0, sizeof(local_tm));
    // FIX: Using thread-safe localtime_r to prevent cross-thread logging corruption
    ::localtime_r(&unix_time, &local_tm);

    char buf[256];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &local_tm);

    return buf;
}

void Win32TimeToUnixTime( int64 win32t, time_t &unix_time, uint32 &nsec )
{
    win32t -= (FILETIME_EPOCH_OFFSET);
    nsec = (win32t % SECS_TO_100NS) * 100;
    win32t /= SECS_TO_100NS;
    unix_time = static_cast<time_t>(win32t);
}

int32 GetElapsedHours(int64 time)
{
    double hours = static_cast<double>(GetFileTimeNow() - time);
    hours /= SECS_TO_100NS;
    hours -= SECS_BETWEEN_EPOCHS;
    hours /= 3600.0;
    return static_cast<int32>(hours);
}

const std::string currentDateTime()
{
    time_t now = std::time(nullptr);
    struct tm tstruct;
    std::memset(&tstruct, 0, sizeof(tstruct));

    // FIX: Thread-safe runtime conversion
    ::localtime_r(&now, &tstruct);

    char buf[80];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

// FIX: Completely reconstructed string format handlers to eliminate the invalid sprintf returns
std::string GetUTimeTillNow(double fromTime)
{
    double elapsed = GetTimeUSeconds() - fromTime;
    char buf[64];
    std::memset(buf, 0, sizeof(buf));

    if (elapsed > 999999.0) {
        std::snprintf(buf, sizeof(buf), "%0.4fs", elapsed / 1000000.0);
    } else if (elapsed > 999.0) {
        std::snprintf(buf, sizeof(buf), "%0.4fms", elapsed / 1000.0);
    } else {
        std::snprintf(buf, sizeof(buf), "%0.4fus", elapsed);
    }
    return buf;
}

std::string GetMTimeTillNow(double fromTime)
{
    double elapsed = GetTimeMSeconds() - fromTime;
    char buf[64];
    std::memset(buf, 0, sizeof(buf));

    if (elapsed > 999.0) {
        std::snprintf(buf, sizeof(buf), "%0.4fs", elapsed / 1000.0);
    } else {
        std::snprintf(buf, sizeof(buf), "%0.4fms", elapsed);
    }
    return buf;
}

EvE::TimeParts GetTimeParts(int64 filetime)
{
    // time sent as (windows)FILETIME; convert to unix time
    int64 time = (filetime / EvE::Time::Second); // Upgrade to explicit int64 to preserve bit depths
    time -= SECS_BETWEEN_EPOCHS;

    // FIX: Using fast integer math (%) instead of double-precision fmod call overhead
    uint16 day = static_cast<uint16>((time / 86400) + 1);
    uint32 seconds = static_cast<uint32>(time % 86400);

    uint16 year = 1970;
    while (day >= 365) {
        if ((year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0))) {
            if (day >= 366) {
                day -= 366;
                ++year;
            } else {
                break;
            }
        } else {
            day -= 365;
            ++year;
        }
    }

    bool isLeap = ((year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0)));

    uint8 month = 0;
    uint8 index = 0;

    if (isLeap) {
        while (index < 12) {
            int daysInMonth = (index == 1) ? 29 : daysOfMonth[index];
            if (day - daysInMonth < 0) {
                break;
            }
            ++month;
            day -= daysInMonth;
            ++index;
        }
    } else {
        while (index < 12) {
            if (day - daysOfMonth[index] < 0) {
                break;
            }
            ++month;
            day -= daysOfMonth[index];
            ++index;
        }
    }

    if (day > 0) {
        ++month;
    } else {
        if (month == 2 && isLeap) {
            day = 29;
        } else {
            day = static_cast<uint16>(daysOfMonth[month - 1]);
        }
    }

    boost::gregorian::date d(year, month, day);

    EvE::TimeParts data = EvE::TimeParts();
    data.year   = year;
    data.month  = month;
    data.day    = day;
    data.wn     = d.week_number();
    data.wd     = d.day_of_week();
    data.dy     = d.day_of_year();
    data.hour   = seconds / 3600;
    data.min    = (seconds % 3600) / 60;
    data.sec    = (seconds % 3600) % 60;
    data.ms     = static_cast<uint16>(time % 1000);

    return data;
}
