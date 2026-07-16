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

#include <chrono>
#include <cstdint>

#include "eve-core.h"
// #include "date.h"

#include "utils/utils_time.h"

// Number of days in month in normal year
static const int daysOfMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static const int64 SECS_BETWEEN_EPOCHS = 11644473600LL;
static const int64 SECS_TO_100NS = 10000000L; // 10^7

const int64 Win32Time_Second = SECS_TO_100NS;
const int64 Win32Time_Minute = Win32Time_Second*60;
const int64 Win32Time_Hour = Win32Time_Minute*60;
const int64 Win32Time_Day = Win32Time_Hour*24;
const int64 Win32Time_Month = Win32Time_Day*30;
const int64 Win32Time_Year = Win32Time_Month*12;

typedef std::chrono::duration<int64, std::ratio<1, 10000000>> duration_100ns;
constexpr uint64_t FILETIME_EPOCH_OFFSET = 116444736000000000ULL;

int64 UnixTimeToWin32Time( time_t sec, uint32 nsec ) {
    return (static_cast<int64>(sec) + FILETIME_EPOCH_OFFSET + (nsec / 100));
}

void Win32TimeToUnixTime( int64 win32t, time_t &unix_time, uint32 &nsec ) {
    win32t -= (FILETIME_EPOCH_OFFSET);
    nsec = (win32t % SECS_TO_100NS) * 100;
    win32t /= SECS_TO_100NS;
    unix_time = win32t;
}

std::string Win32TimeToString(int64 win32t) {
    std::time_t unix_time;
    uint32 nsec = 0;
    Win32TimeToUnixTime(win32t, unix_time, nsec);

    char buf[256];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&unix_time));

    return buf;
}

int64 Win32TimeNow() {
    //  returns second resolution.
    return UnixTimeToWin32Time(std::time(NULL), 0);
}

int32 GetElapsedHours(int64 time)  // -allan
{
    double hours = GetFileTimeNow() -time;
    hours /= SECS_TO_100NS;
    hours -= SECS_BETWEEN_EPOCHS;
    hours /= 3600;
    return (int32)hours;
}

int64 GetFileTimeNow() { // -allan
    // convert (double)system time to (int64)filetime.
    using namespace std::chrono;
    auto now = system_clock::now();
    auto duration = now.time_since_epoch();

    int64 epoch_interval = duration_cast<duration_100ns>(duration).count();
    return epoch_interval + FILETIME_EPOCH_OFFSET;
}

//  NOTE  auto and std::chrono require C++11
int64 GetSteadyTime() {  // -allan
    // simulation of Windows GetTickCount()
    //  this returns milliseconds
    using namespace std::chrono;
    static const auto bootTime = high_resolution_clock::now();

    auto current = high_resolution_clock::now();
    return duration_cast<milliseconds>(current -  bootTime).count();
}

double GetTimeMSeconds() {  // -allan
    //  this returns milliseconds in microsecond resolution
    using namespace std::chrono;
    static const auto bootTime = high_resolution_clock::now();

    auto current = high_resolution_clock::now();
    int64 delta = duration_cast<microseconds>(current -  bootTime).count();
    return static_cast<double>(delta) / 1000.0;
}

double GetTimeUSeconds() {  // -allan
    //  this returns microseconds in nanosecond resolution
    using namespace std::chrono;
    static const auto bootTime = high_resolution_clock::now();

    auto current = high_resolution_clock::now();
    int64 delta = duration_cast<nanoseconds>(current -  bootTime).count();
    return static_cast<double>(delta) / 1000.0;
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
    long time(filetime / EvE::Time::Second);// to Seconds
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

    bool flag(false);
    if ((year % 400 == 0)
    or ((year % 4 == 0) and (year % 100 != 0)))
        flag = true;

    // Calculating MONTH and DATE
    uint8 month(0), index(0);
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

//FILETIME, a struct which stores the 64-bit number of 100-nanosecond intervals since midnight Jan 1, 1601.


/*  this doesnt work....std::get_time() is unavailable
// Converts UTC time string to a time_t value.
std::time_t getEpochTime(const std::wstring& dateTime)
{
    // Let's consider we are getting all the input in
    // this format: '2014-07-25T20:17:22Z' (T denotes
    // start of Time part, Z denotes UTC zone).
    // A better approach would be to pass in the format as well.
    static const std::wstring dateTimeFormat{ L"%Y-%m-%dT%H:%M:%SZ" };

    // Create a stream which we will use to parse the string,
    // which we provide to constructor of stream to fill the buffer.
    std::wstringstream ss{ dateTime };

    // Create a tm object to store the parsed date and time.
    std::tm dt;

    // Now we read from buffer using get_time manipulator
    // and formatting the input appropriately.
    ss >> std::get_time(&dt, dateTimeFormat.c_str());

    // Convert the tm structure to time_t value and return.
    return std::mktime(&dt);
}
*/