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
*/


#include "eve-core.h"

#include <sstream>
#include <iomanip>
#include <cstring>

#include "log/logsys.h"
#include "utils/utils_string.h"
#include "utils_hex.h"


const uint32 HEX_DUMP_PREVIEW_LIMIT = 1024;

std::string build_hex_line( const uint8* buffer, size_t length, size_t offset, unsigned int padding )
{
    if ( buffer == nullptr || offset >= length ) {
        return "";
    }

    std::ostringstream ss;

    // 1. Format the address offset (e.g., 00A0:)
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(padding) << offset << ":";

    char printable[17];
    std::memset(printable, 0, sizeof(printable));

    // 2. Process the 16-byte hex window chunk
    for ( size_t i = 0; i < 16; ++i ) {
        if ( i == 8 ) {
            ss << " -";
        }

        const size_t current_offset = offset + i;
        if ( current_offset < length ) {
            const uint8 c = buffer[current_offset];
            ss << " " << std::setfill('0') << std::setw(2) << static_cast<int>(c);
            printable[i] = ( IsPrintable( static_cast<char>(c) ) ? static_cast<char>(c) : '.' );
        } else {
            ss << "   ";
            printable[i] = ' '; // Pad out empty space visual grids cleanly
        }
    }

    // 3. Format the final ASCII visualization bracket
    ss << "  | " << printable;
    return ss.str();
}

void pfxHexDump( const char* pfx, FILE* into, const uint8* data, size_t length )
{
    if ( into == nullptr || data == nullptr ) {
        return;
    }

    for ( size_t offset = 0; offset < length; offset += 16 ) {
        std::string line = build_hex_line( data, length, offset, 4 );
        std::fprintf( into, "%s%s\n", pfx, line.c_str() );
    }
}

void pfxHexDump( const char* pfx, LogType type, const uint8* data, size_t length )
{
    if ( data == nullptr ) {
        return;
    }

    for ( size_t offset = 0; offset < length; offset += 16 ) {
        std::string line = build_hex_line( data, length, offset, 4 );
        _log( type, "%s%s", pfx, line.c_str() );
    }
}

void pfxHexDumpPreview( const char* pfx, FILE* into, const uint8* data, size_t length )
{
    if ( into == nullptr || data == nullptr ) {
        return;
    }

    if ( length > HEX_DUMP_PREVIEW_LIMIT ) {
        pfxHexDump( pfx, into, data, HEX_DUMP_PREVIEW_LIMIT - 32 );
        std::fprintf( into, "%s ... truncated ...\n", pfx );

        std::string trailing_line = build_hex_line( data, length, length - 16, 4 );
        std::fprintf( into, "%s%s\n", pfx, trailing_line.c_str() );
    } else {
        pfxHexDump( pfx, into, data, length );
    }
}

void pfxHexDumpPreview( const char* pfx, LogType type, const uint8* data, size_t length )
{
    if ( data == nullptr ) {
        return;
    }

    if ( length > HEX_DUMP_PREVIEW_LIMIT ) {
        pfxHexDump( pfx, type, data, HEX_DUMP_PREVIEW_LIMIT - 32 );
        _log( type, "%s ... truncated ...", pfx );

        std::string trailing_line = build_hex_line( data, length, length - 16, 4 );
        _log( type, "%s%s", pfx, trailing_line.c_str() );
    } else {
        pfxHexDump( pfx, type, data, length );
    }
}
