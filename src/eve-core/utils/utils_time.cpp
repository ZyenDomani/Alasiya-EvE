/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
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
*/

#include "eve-core.h"

#include "utils/utils_time.h"

const int64 Win32Time_Second = 10000000L;
const int64 Win32Time_Minute = Win32Time_Second*60;
const int64 Win32Time_Hour = Win32Time_Minute*60;
const int64 Win32Time_Day = Win32Time_Hour*24;
const int64 Win32Time_Month = Win32Time_Day*30;
const int64 Win32Time_Year = Win32Time_Month*12;

static const int64 SECS_BETWEEN_EPOCHS = 11644473600LL;
static const int64 SECS_TO_100NS = 10000000L; // 10^7

int64 UnixTimeToWin32Time( time_t sec, uint32 nsec ) {
    return(
        (((int64) sec) + SECS_BETWEEN_EPOCHS) * SECS_TO_100NS
        + (nsec / 100)
    );
}

void Win32TimeToUnixTime( int64 win32t, time_t &unix_time, uint32 &nsec ) {
    win32t -= (SECS_BETWEEN_EPOCHS * SECS_TO_100NS);
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

    return(buf);
}

int64 Win32TimeNow() {
    //  returns second resolution.
    return UnixTimeToWin32Time(std::time(NULL), 0);
}

int32 GetElapsedHours(int64 time)
{
    double hours = time - GetFileTimeNow();
    hours /= 10000000;
    hours -= 11644473600;
    hours /= 3600;
    return (int32)hours;
}

double GetFileTimeNow()
{
    // convert system time to filetime.
    double time = GetTimeMSeconds();
    time /= 1000;
    time += 11644473600;
    time *= 10000000;
    return time;
}

//  NOTE  auto and std::chrono require C++11
int64 GetSteadyTime() {
    // simulation of Windows GetTickCount()
    //  this returns milliseconds
    using namespace std::chrono;
    auto duration = system_clock::now().time_since_epoch();     // return in nanoseconds
    //int64 time = duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
    return duration_cast<milliseconds>(duration).count();
}

double GetTimeMSeconds() {  // -allan
    //  this returns milliseconds in microsecond resolution
    using namespace std::chrono;
    /*
    auto now = steady_clock::now();
    duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
    */
    auto duration = system_clock::now().time_since_epoch();     // return in nanoseconds
    double time = duration_cast<microseconds>(duration).count();
    return (time /1000);
}

double GetTimeUSeconds() {  // -allan
    //  this returns microseconds in nanosecond resolution
    using namespace std::chrono;
    auto duration = system_clock::now().time_since_epoch();     // return in nanoseconds
    double time = duration_cast<nanoseconds>(duration).count();
    return (time /1000);
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