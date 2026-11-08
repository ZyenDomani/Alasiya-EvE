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
    Author:     Captnoord, Bloody.Rabbit
    Update:     Allan
*/

#pragma once

#include <string>
#include <vector>

#include "../eve-compat.h"

// Forward declaration of Buffer prevents us from including the whole Buffer.h file here,
// smashing the include cascade and speeding up your PCH compilation passes!
class Buffer;

// std::string to use where you would use NULL for const char*.
extern const std::string NULL_STRING;

/**
 * @brief Generates a random cryptographic or session key.
 */
std::string GenerateKey( size_t length );

/********************************************************************/
/* Character & String Format Validators                             */
/********************************************************************/

bool IsNumber( char c );
bool IsNumber( const char* str, size_t len );
bool IsNumber( const std::string& str );

bool IsHexNumber( char c );
bool IsHexNumber( const char* str, size_t len );
bool IsHexNumber( const std::string& str );

bool IsPrintable( char c );
bool IsPrintable( const char* str, size_t len );
bool IsPrintable( const std::string& str );

/********************************************************************/
/* String Formatting & Conversion Utilities                          */
/********************************************************************/

// Keep legacy itoa signature for symbol compatibility, but we make it thread-safe inside the cpp
const char* itoa( int64 num );

/**
 * Converts a vector of numbers into a formatted SQL-ready IN string list.
 */
void ListToINString( const std::vector<int32>& ints, std::string& into, const char* if_empty = "" );

// Enforces modern std::string transformations to eliminate raw char* buffer overflows
std::string MakeUpperString( const std::string& source );
std::string MakeLowerString( const std::string& source );

/**
 * @brief Decodes string escapes into actual characters.
 *        Based on PyString_DecodeEscape from Python core libraries.
 */
bool PyDecodeEscape( const char* str, Buffer& into );

void SearchReplace( std::string& subject, const std::string& search, const std::string& replace );

void SplitPath( const std::string& path, std::vector<std::string>& into );

std::string BinString( std::string words );
