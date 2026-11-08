/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2016 The EVEmu Team
 *    For the latest information visit http://evemu.org
 *    ------------------------------------------------------------------------------------
 *    This program is free software; you can redistribute it and/or modify it under
 *    the terms of the GNU Lesser General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option) any later
 *    version.
 *
 *    This program is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License along with
 *    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 *    http://www.gnu.org/copyleft/lesser.txt.
 *    ------------------------------------------------------------------------------------
 *    Author:     Zhur
 */

#include "eve-core.h"

#include <string>
#include <cstring>
#include <cstdarg>
#include <vector>

#include "log/logsys.h"
#include "utils/utils_hex.h"
#include "threading/Mutex.h"


Mutex mLogSys;

FILE *logsys_log_file(nullptr);

#define LOG_CATEGORY(category) #category ,
const char *log_category_names[NUMBER_OF_LOG_CATEGORIES] = {
    #include "log/logtypes.h"
};

#define LOG_TYPE(category, type, enabled, str) { enabled, LOG_ ##category, #category "__" #type, str },
static LogTypeStatus real_log_type_info[NUMBER_OF_LOG_TYPES+1] ={
    #include "log/logtypes.h"
    #include "utils/Lock.h"
    { false, NUMBER_OF_LOG_CATEGORIES, "BAD TYPE", "Bad Name" } /* dummy trailing record */
};

const LogTypeStatus *log_type_info = real_log_type_info;

// FIX: Completely synchronized with our safe build_hex_line rewrite, wiping out the VLA hazards!
void log_hex(LogType type, const void *data, unsigned long length, unsigned char padding)
{
    if (data == nullptr || length == 0) {
        return;
    }

    for (uint32 offset = 0; offset < length; offset += 16) {
        std::string hex_line = build_hex_line(reinterpret_cast<const uint8*>(data), length, offset, padding);
        if (!hex_line.empty()) {
            log_message(type, "%s", hex_line.c_str()); // %s protects against % escape tokens in string streams
        }
    }
}

void log_phex(LogType type, const void *data, unsigned long length, unsigned char padding)
{
    if (data == nullptr || length == 0) {
        return;
    }

    if (length <= 1024) {
        log_hex(type, data, length, padding);
    } else {
        log_hex(type, data, length, padding);
        log_message(type, " ... truncated ...");

        std::string trailing_line = build_hex_line(reinterpret_cast<const uint8*>(data), length, length - 16, padding);
        if (!trailing_line.empty()) {
            log_message(type, "%s", trailing_line.c_str());
        }
    }
}

void log_message(LogType type, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    log_messageVA(type, 0, fmt, args);
    va_end(args);
}

void log_messageVA(LogType type, const char *fmt, va_list args)
{
    log_messageVA(type, 0, fmt, args);
}

extern void log_messageVA( LogType type, uint32 iden, const char *fmt, va_list args)
{
    /* Use local stack array to prevent heap allocation chokes on frequent text writes */
    char formatted_msg[4096];
    std::memset(formatted_msg, 0, sizeof(formatted_msg));

    tm t;
    std::memset(&t, 0, sizeof(t));
    time_t tTime;
    time(&tTime);
    ::localtime_r(&tTime, &t);

    size_t current_index = 0;
    size_t remaining_size = sizeof(formatted_msg);

    int written = std::snprintf(&formatted_msg[current_index], remaining_size, "%02u:%02u:%02u [%s] ",
                                t.tm_hour, t.tm_min, t.tm_sec, log_type_info[type].display_name);

    current_index += written;
    remaining_size -= written;

    /* Securely insert leading indentation spacing thresholds */
    for (uint32 i = 0; i < iden && remaining_size > 2; i++) {
        formatted_msg[current_index++] = ' ';
        remaining_size--;
    }

    written = std::vsnprintf(&formatted_msg[current_index], remaining_size - 2, fmt, args);
    current_index += written;

    /* Enforce strict terminal boundaries */
    formatted_msg[current_index++] = '\n';
    formatted_msg[current_index] = '\0';

    MutexLock lock(mLogSys);

    std::fputs(formatted_msg, stdout);

    if (logsys_log_file != nullptr) {
        std::fputs(formatted_msg, logsys_log_file);
        std::fflush(logsys_log_file);
    }

    lock.Unlock();
}

void log_enable( LogType t ) {
    real_log_type_info[t].enabled = true;
}

void log_disable( LogType t ) {
    real_log_type_info[t].enabled = false;
}

void log_toggle( LogType t ) {
    real_log_type_info[t].enabled = !real_log_type_info[t].enabled;
}

bool log_open_logfile( const char* filename )
{
    MutexLock lock(mLogSys);
    if (logsys_log_file != nullptr) {
        if (!log_close_logfile()) {
            return false;
        }
    }

    logsys_log_file = std::fopen(filename, "w");
    return (nullptr != logsys_log_file);
}

bool log_close_logfile()
{
    MutexLock lock(mLogSys);
    if (!logsys_log_file) {
        return true;
    }
    return (0 == std::fclose(logsys_log_file));
}

bool load_log_settings(const char *filename)
{
    FILE *f = std::fopen(filename, "r");
    if (f == nullptr) {
        return false;
    }

    char linebuf[512], type_name[256], value[256];
    uint16 line_count = 0;
    bool enabled = false;

    while (std::fgets(linebuf, sizeof(linebuf), f) != nullptr) {
        ++line_count;
        if (std::sscanf(linebuf, "%[^=]=%[^\r\n]\n", type_name, value) != 2) {
            continue;
        }
        if (type_name[0] == '\0' || type_name[0] == '#') {
            continue;
        }

        if (!strcasecmp(value, "on") || !strcasecmp(value, "yes") || !strcasecmp(value, "enabled") || !std::strcmp(value, "1")) {
            enabled = true;
        } else if (!strcasecmp(value, "off") || !strcasecmp(value, "no") || !strcasecmp(value, "disabled") || !std::strcmp(value, "0")) {
            enabled = false;
        } else {
            std::printf("Unable to parse value '%s' from %s around line %u. Skipping.\n", value, filename, line_count);
            continue;
        }

        uint16 category_match = NUMBER_OF_LOG_CATEGORIES;
        for (uint16 r = 0; r < NUMBER_OF_LOG_CATEGORIES; r++) {
            if (!strcasecmp(log_category_names[r], type_name)) {
                category_match = r;
                break;
            }
        }

        if (category_match != NUMBER_OF_LOG_CATEGORIES) {
            for (uint16 k = 0; k < NUMBER_OF_LOG_TYPES; k++) {
                if (log_type_info[k].category == category_match) {
                    if (enabled) {
                        log_enable(LogType(k));
                    } else {
                        log_disable(LogType(k));
                    }
                }
            }
            continue;
        }

        uint16 type_match = NUMBER_OF_LOG_TYPES;
        for (uint16 r = 0; r < NUMBER_OF_LOG_TYPES; r++) {
            if (!strcasecmp(log_type_info[r].name, type_name)) {
                type_match = r;
                break;
            }
        }

        if (type_match == NUMBER_OF_LOG_TYPES) {
            std::printf("Unable to locate log type %s from file %s around line %u. Skipping.\n", type_name, filename, line_count);
            continue;
        }

        if (enabled) {
            log_enable(LogType(type_match));
        } else {
            log_disable(LogType(type_match));
        }
    }

    std::fclose(f);
    return true;
}
