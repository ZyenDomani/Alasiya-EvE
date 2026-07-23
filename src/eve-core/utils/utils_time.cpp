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

// #include "date.h"

#include "utils/utils_time.h"

// Number of days in month in normal year
static const int daysOfMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static const int64 SECS_BETWEEN_EPOCHS = 11644473600LL;
static const int64 SECS_TO_100NS = 10000000L; // 10^7

const int64 Win32Time_Second = SECS_TO_100NS;
const int64 Win32Time_Minute = Win32Time_Second*60;
const int64 Win32Time_Hour = Win32Time_Minute*60;

std::string Win32TimeToString(int64 win32t) {
    std::time_t unix_time;
    uint32 nsec = 0;
    Win32TimeToUnixTime(win32t, unix_time, nsec);

    char buf[256];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&unix_time));

    return buf;
}

void Win32TimeToUnixTime( int64 win32t, time_t &unix_time, uint32 &nsec ) {
    win32t -= (FILETIME_EPOCH_OFFSET);
    nsec = (win32t % SECS_TO_100NS) * 100;
    win32t /= SECS_TO_100NS;
    unix_time = win32t;
}

int32 GetElapsedHours(int64 time)  // -allan
{
    double hours = GetFileTimeNow() -time;
    hours /= SECS_TO_100NS;
    hours -= SECS_BETWEEN_EPOCHS;
    hours /= 3600;
    return (int32)hours;
}

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string currentDateTime() {
    time_t     now = std::time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *std::localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    std::strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

std::string GetUTimeTillNow(double fromTime)
{
    double elapsed = GetTimeUSeconds() - fromTime;
    if (elapsed > 999999)
        return sprintf("%0.4fs", elapsed / 1000000);
    else if (elapsed > 999)
        return sprintf("%0.4fms", elapsed / 1000);
    else
        return sprintf("%0.4fus", elapsed);
}

std::string GetMTimeTillNow(double fromTime)
{
    double elapsed = GetTimeMSeconds() - fromTime;
    if (elapsed > 999)
        return sprintf("%0.4fs", elapsed / 1000);
    else
        return sprintf("%0.4fms", elapsed);
}

EvE::TimeParts GetTimeParts(int64 filetime/*0*/)
{
    // time sent as (windows)FILETIME; convert to unix time
    long time = (filetime / EvE::Time::Second);// to Seconds
    time -= SECS_BETWEEN_EPOCHS;    // epoc offset

    // Calculate total days
    uint16 day = (time / 86400) +1;
    uint16 seconds = std::fmod(time, 86400);

    // year loop
    uint16 year = 1970;
    while (day >= 365) {
        if ((year % 400 == 0)
        or ((year % 4 == 0) and (year % 100 != 0))) {
            day -= 366;
        } else {
            day -= 365;
        }
        ++year;
    }

    bool flag = false;
    if ((year % 400 == 0)
    or ((year % 4 == 0) and (year % 100 != 0)))
        flag = true;

    // Calculating MONTH and DATE
    uint8 month(0), index = 0;
    if (flag) {
        while (true) {
            if (index == 1) {
                if (day - 29 < 0)
                    break;
                ++month;
                day -= 29;
            } else {
                if (day - daysOfMonth[index] < 0)
                    break;
                ++month;
                day -= daysOfMonth[index];
            }
            ++index;
        }
    } else {
        while (true) {
            if (day - daysOfMonth[index] < 0)
                break;
            day -= daysOfMonth[index];
            ++month;
            ++index;
        }
    }

    // Current Month
    if (day > 0) {
        ++month;
    } else {
        if (month == 2 && flag)
            day = 29;
        else {
            day = daysOfMonth[month - 1];
        }
    }

    // use boost to get day of week and week of year
    boost::gregorian::date d(year, month, day);

    EvE::TimeParts data = EvE::TimeParts();
    data.year   = year;
    data.month  = month;
    data.day    = day;
    data.wn     = d.week_number();
    data.wd     = d.day_of_week();
    data.dy     = d.day_of_year();
    data.hour   = seconds / 3600;
    data.min    = std::fmod(seconds, 3600) / 60;
    data.sec    = std::fmod(std::fmod(seconds, 3600), 60);
    data.ms     = std::fmod(time, 1000);

    return data;
}
