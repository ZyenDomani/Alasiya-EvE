/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    Copyright 2016 - 2026 Alasiya-EvE by Allan
    For the latest implementation status visit http://eve.alasiya.net/?p=op_status
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
    Author:     Captnoord
*/

#ifndef __LOG__LOG_NEW_H__INCL__
#define __LOG__LOG_NEW_H__INCL__

#include "utils/Singleton.h"
#include "threading/Mutex.h"

/**
 * @brief a small and simple logging system.
 *
 * This class is designed to be a simple logging system that both logs to file
 * and console regarding the settings.
 *
 * @author Captnoord.
 * @date August 2009
 */
class NewLog
: public Singleton< NewLog >
{
public:
    /// Primary constructor, initializes logging.
    NewLog();
    NewLog(std::string logPath);
    NewLog(NewLog&&) =delete;
    NewLog(const NewLog&) =delete;
    NewLog& operator=(NewLog&&) =delete;
    NewLog& operator=(const NewLog&) =delete;
    virtual ~NewLog();

    void Initialize();

    void InitializeLogging( std::string logPath );
    void Log( const char* source, const char* fmt, ... );
    void Error( const char* source, const char* fmt, ... );
    void Warning( const char* source, const char* fmt, ... );
    void White( const char* source, const char* fmt, ... );
    void Green( const char* source, const char* fmt, ... );
    void Blue( const char* source, const char* fmt, ... );
    void Magenta( const char* source, const char* fmt, ... );
    void Yellow( const char* source, const char* fmt, ... );
    void Cyan( const char* source, const char* fmt, ... );
    void Debug( const char* source, const char* fmt, ... );
    bool SetLogfile( const char* filename );
    bool SetLogfile( FILE* file );
    void SetTime( time_t time ) { mTime = time; }

protected:
    /// A convenience color enum.
    enum Color
    {
        COLOR_DEFAULT, ///< A default color.
        COLOR_BLACK,   ///< Black color.
        COLOR_RED,     ///< Red color.
        COLOR_GREEN,   ///< Green color.
        COLOR_YELLOW,  ///< Yellow color.
        COLOR_BLUE,    ///< Blue color.
        COLOR_MAGENTA, ///< Magenta color.
        COLOR_CYAN,    ///< Cyan color.
        COLOR_WHITE,   ///< White color.

        COLOR_COUNT    ///< Number of colors.
    };

    /**
     * @brief Prints a message.
     *
     * This prints a generic message.
     *
     * @param[in] color  Color of the message.
     * @param[in] pfx    Single-character prefix/identificator.
     * @param[in] source Origin of message.
     * @param[in] fmt    The format string.
     * @param[in] ap     The arguments.
     */
    void PrintMsg( Color color, char pfx, const char* source, const char* fmt, va_list ap );
    /**
     * @brief Prints current time.
     */
    void PrintTime();

    /**
     * @brief Prints a raw message.
     *
     * This method only handles printing to all desired
     * destinations (standard output and logfile at the moment).
     *
     * @param[in] fmt The format string.
     * @param[in] ... The arguments.
     */
    void Print( const char* fmt, ... );
    /**
     * @brief Prints a raw message.
     *
     * This method only handles printing to all desired
     * destinations (standard output and logfile at the moment).
     *
     * @param[in] fmt The format string.
     * @param[in] ap  The arguments.
     */
    void PrintVa( const char* fmt, va_list ap );

    /**
     * @brief Sets the color of the output text.
     *
     * @param[in] color The new color of output text.
     */
    void SetColor( Color color );
    /**
     * @brief Sets the default logfile.
     */
    void SetLogfileDefault(std::string logPath);

    /// The active logfile.
    FILE* mLogfile;
    /// Current timestamp.
    time_t mTime; // crap there should be 1 generic easy to understand time manager.
    /// Protection against concurrent log messages
    Mutex mMutex;

    bool m_initialized;

    /// Color translation table.
    static const char* const COLOR_TABLE[ COLOR_COUNT ];
};

/// Evaluates to a NewLog instance.
#define sLog \
    ( NewLog::get() )

#endif /* !__LOG__LOG_NEW_H__INCL__ */
