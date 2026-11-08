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
    Author:     Bloody.Rabbit
    Update:     Allan
*/

#include <cstring>
#include <cstdlib>
#include <cctype>

#include "eve-core.h"
#include "utils/str2conv.h"


template<>
bool str2< bool >( const char* str )
{
    // 1. Strict boundary guard: Protect against missing or null XML attribute nodes
    if ( str == nullptr || *str == '\0' ) {
        return false;
    }

    if ( ::strcasecmp( str, "true" ) == 0 ) {
        return true;
    } else if ( ::strcasecmp( str, "false" ) == 0 ) {
        return false;
    } else if ( ::strcasecmp( str, "yes" ) == 0 ) {
        return true;
    } else if ( ::strcasecmp( str, "no" ) == 0 ) {
        return false;
    } else if ( ::strcasecmp( str, "y" ) == 0 ) {
        return true;
    } else if ( ::strcasecmp( str, "n" ) == 0 ) {
        return false;
    } else if ( ::strcasecmp( str, "on" ) == 0 ) {
        return true;
    } else if ( ::strcasecmp( str, "off" ) == 0 ) {
        return false;
    } else if ( ::strcasecmp( str, "enable" ) == 0 ) {
        return true;
    } else if ( ::strcasecmp( str, "disable" ) == 0 ) {
        return false;
    } else if ( ::strcasecmp( str, "enabled" ) == 0 ) {
        return true;
    } else if ( ::strcasecmp( str, "disabled" ) == 0 ) {
        return false;
    }

    // FIX: Safely parse standard integers (like "1" or "0") and verify the actual non-zero value
    char* endptr = nullptr;
    long val = std::strtol( str, &endptr, 10 );
    if ( endptr != str ) {
        return ( val != 0 );
    }

    return false;
}

// --- Streamlined C++11 Specializations (Replacing sscanf with high-performance C++ standard parsers) ---

template<>
int8 str2< int8 >( const char* str ) {
    if ( str == nullptr ) { return 0; }
    return static_cast<int8>( std::strtol( str, nullptr, 10 ) );
}

template<>
int16 str2< int16 >( const char* str ) {
    if ( str == nullptr ) { return 0; }
    return static_cast<int16>( std::strtol( str, nullptr, 10 ) );
}

template<>
int32 str2< int32 >( const char* str ) {
    if ( str == nullptr ) { return 0; }
    return static_cast<int32>( std::strtol( str, nullptr, 10 ) );
}

template<>
int64 str2< int64 >( const char* str ) {
    if ( str == nullptr ) { return 0; }
    return static_cast<int64>( std::strtoll( str, nullptr, 10 ) );
}

template<>
uint8 str2< uint8 >( const char* str ) {
    if ( str == nullptr ) { return 0; }
    return static_cast<uint8>( std::strtoul( str, nullptr, 10 ) );
}

template<>
uint16 str2< uint16 >( const char* str ) {
    if ( str == nullptr ) { return 0; }
    return static_cast<uint16>( std::strtoul( str, nullptr, 10 ) );
}

template<>
uint32 str2< uint32 >( const char* str ) {
    if ( str == nullptr ) { return 0; }
    return static_cast<uint32>( std::strtoul( str, nullptr, 10 ) );
}

template<>
float str2< float >( const char* str ) {
    if ( str == nullptr ) { return 0.0f; }
    return std::strtof( str, nullptr );
}

template<>
double str2< double >( const char* str ) {
    if ( str == nullptr ) { return 0.0; }
    return std::strtod( str, nullptr );
}

template<>
long double str2< long double >( const char* str ) {
    if ( str == nullptr ) { return 0.0L; }
    return std::strtold( str, nullptr );
}
