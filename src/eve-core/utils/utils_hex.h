
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
#pragma once

#include <string>
#include <cstdio> // Kept strictly for the legacy FILE* compatibility signatures

// Forward declaration of LogType, 
// ensuring this file stays incredibly light on the preprocessor graph!
enum LogType;

/** The maximal number of bytes printed by pfxHexDumpPreview. */
extern const uint32 HEX_DUMP_PREVIEW_LIMIT;

/**
 * @brief Build a printable line suitable for a hex dump.
 *        FIX: Returns a safe std::string to permanently eliminate buffer overflows.
 *
 * @param[in]  buffer  Buffer which is being dumped.
 * @param[in]  length  Length of buffer data available.
 * @param[in]  offset  Physical offset at which the dump should start.
 * @param[in]  padding Padding to be used when printing the address offset.
 *
 * @return Formatted printable hex string line.
 */
std::string build_hex_line( const uint8* buffer, size_t length, size_t offset, unsigned int padding = 4 );

/********************************************************************/
/* Packet Packet Logging & Hex Dump Outlets                        */
/********************************************************************/

void pfxHexDump( const char* pfx, FILE* into, const uint8* data, size_t length );
void pfxHexDump( const char* pfx, LogType type, const uint8* data, size_t length );

void pfxHexDumpPreview( const char* pfx, FILE* into, const uint8* data, size_t length );
void pfxHexDumpPreview( const char* pfx, LogType type, const uint8* data, size_t length );

