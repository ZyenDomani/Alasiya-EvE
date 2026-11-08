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
*/

#include "eve-core.h"

#include <string>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <bitset>

#include "utils_string.h"
#include "utils/Buffer.h"

const std::string NULL_STRING = "NULL";

std::string GenerateKey( size_t length )
{
    static const char CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static const size_t CHARS_COUNT = (sizeof( CHARS ) / sizeof( char )) - 1; // Correct null-terminator sizing

    std::string key;
    key.reserve(length); // Optimize memory allocations upfront

    for (; length > 0; --length) {
        key += CHARS[ MakeRandomInt( 0, CHARS_COUNT - 1 ) ];
    }

    return key;
}

bool IsNumber( char c )
{
    return (std::isdigit( static_cast<unsigned char>(c) ) != 0);
}

bool IsNumber( const char* str, size_t len )
{
    if ( len == 0 ) {
        return false;
    }

    // Explicitly fix sign evaluations for strings of ANY length safely
    if ( str[0] == '-' || str[0] == '+' ) {
        str += 1;
        len -= 1;
    }

    if ( len == 0 ) {
        return false;
    }

    bool seenDec = false;
    for (; len > 0; str++, len--) {
        const char c = *str;
        if ( !IsNumber( c ) ) {
            if ( !seenDec && c == '.' ) {
                seenDec = true;
            } else {
                return false;
            }
        }
    }

    return true;
}

bool IsNumber( const std::string& str )
{
    return IsNumber( str.c_str(), str.length() );
}

bool IsHexNumber( char c )
{
    return (std::isxdigit( static_cast<unsigned char>(c) ) != 0);
}

bool IsHexNumber( const char* str, size_t len )
{
    if ( len == 0 ) {
        return false;
    }

    // FIX: Flipped inequalities from structural bug layout to correctly handle real lengths
    if ( len >= 1 ) {
        if ( str[0] == '-' || str[0] == '+' ) {
            str += 1;
            len -= 1;
        }
    }

    // FIX: Safely peel the 0x/0X hex prefix indicator off strings of any arbitrary length
    if ( len >= 2 ) {
        if ( str[0] == '0' && (str[1] == 'x' || str[1] == 'X') ) {
            str += 2;
            len -= 2;
        }
    }

    if ( len == 0 ) {
        return false;
    }

    for (; len > 0; str++, len--) {
        if ( !IsHexNumber( *str ) ) {
            return false;
        }
    }

    return true;
}

bool IsHexNumber( const std::string& str )
{
    return IsHexNumber( str.c_str(), str.length() );
}

bool IsPrintable( char c )
{
    const unsigned char _c = static_cast<unsigned char>(c);
    return ( std::isgraph( _c ) || std::isspace( _c ) );
}

bool IsPrintable( const char* str, size_t len )
{
    for (; len > 0; str++, len--) {
        if ( !IsPrintable( *str ) ) {
            return false;
        }
    }

    return true;
}

bool IsPrintable( const std::string& str )
{
    return IsPrintable( str.c_str(), str.size() );
}

#define _ITOA_BUFLEN 21

const char* itoa( int64 num )
{
    // FIX: Using thread_local ensures each thread gets its own isolated buffer ring slot,
    // making the method completely safe across simultaneous logging/DB operations!
    thread_local char buf[ _ITOA_BUFLEN ];
    std::memset( buf, 0, _ITOA_BUFLEN );

    std::snprintf( buf, _ITOA_BUFLEN, "%lli", num );

    return buf;
}

void ListToINString( const std::vector<int32>& ints, std::string& into, const char* if_empty )
{
    if ( ints.empty() ) {
        into = if_empty;
        return;
    }

    // Modern C++11 High-Speed Stream formulation: Completely safe from snprintf buffer overruns!
    std::ostringstream ss;
    for ( size_t i = 0; i < ints.size(); ++i ) {
        ss << ints[i];
        if ( i + 1 < ints.size() ) {
            ss << ",";
        }
    }
    into += ss.str();
}

// Clean safety replacements: Uses std::string signatures to permanently prevent target buffer overflows
std::string MakeUpperString( const std::string& source )
{
    std::string target = source;
    std::transform(target.begin(), target.end(), target.begin(), [](unsigned char c) {
        return std::toupper(c);
    });
    return target;
}

std::string MakeLowerString( const std::string& source )
{
    std::string target = source;
    std::transform(target.begin(), target.end(), target.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return target;
}

bool PyDecodeEscape( const char* str, Buffer& into )
{
    if ( str == nullptr ) {
        return false;
    }

    const size_t len = std::strlen( str );
    const char* const end = str + len;

    while ( str < end ) {
        if ( *str != '\\' ) {
            into.Append< char >( *str++ );
            continue;
        }

        if ( ++str == end ) {
            return false; // Ended with an unescaped trailing slash
        }

        int c;
        switch ( *str++ ) {
            case '\n': break;
            case '\\': into.Append< char >( '\\' );   break;
            case '\'': into.Append< char >( '\'' );   break;
            case '\"': into.Append< char >( '\"' );   break;
            case 'b':  into.Append< char >( '\b' );   break;
            case 'f':  into.Append< char >( '\014' ); break; // FF
            case 't':  into.Append< char >( '\t' );   break;
            case 'n':  into.Append< char >( '\n' );   break;
            case 'r':  into.Append< char >( '\r' );   break;
            case 'v':  into.Append< char >( '\013' ); break; // VT
            case 'a':  into.Append< char >( '\007' ); break; // BEL

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7': {
                c = str[-1] - '0';
                if ( str < end && '0' <= *str && *str <= '7' ) {
                    c = ( c << 3 ) + *str++ - '0';
                    if ( str < end && '0' <= *str && *str <= '7' ) {
                        c = ( c << 3 ) + *str++ - '0';
                    }
                }
                into.Append< uint8 >( static_cast<uint8>(c) );
                break;
            }

            case 'x': {
                // Safeguard length boundaries before sampling hex offsets
                if ( (end - str) >= 2 && std::isxdigit(static_cast<unsigned char>(str[0])) && std::isxdigit(static_cast<unsigned char>(str[1])) ) {
                    unsigned int x = 0;
                    c = *str++;

                    if ( std::isdigit(static_cast<unsigned char>(c)) ) {
                        x = c - '0';
                    } else if ( std::islower(static_cast<unsigned char>(c)) ) {
                        x = 10 + c - 'a';
                    } else {
                        x = 10 + c - 'A';
                    }

                    x = x << 4;
                    c = *str++;

                    if ( std::isdigit(static_cast<unsigned char>(c)) ) {
                        x += c - '0';
                    } else if ( std::islower(static_cast<unsigned char>(c)) ) {
                        x += 10 + c - 'a';
                    } else {
                        x += 10 + c - 'A';
                    }

                    into.Append< uint8 >( static_cast<uint8>(x) );
                    break;
                }
                return false;
            }

            default:
                return false;
        }
    }

    return true;
}

void SearchReplace( std::string& subject, const std::string& search, const std::string& replace )
{
    if ( search.empty() ) {
        return;
    }
    std::string::size_type index = 0;
    while ( ( index = subject.find( search, index ) ) != std::string::npos ) {
        subject.replace( index, search.length(), replace );
        index += replace.length();
    }
}

void SplitPath( const std::string& path, std::vector<std::string>& into )
{
    const char* p = path.c_str();
    const char* begin_ptr = p;
    size_t segment_len = 0;

    for (; *p != '\0'; ++p) {
        if ( *p == '/' || *p == '\\' ) {
            into.push_back( std::string( begin_ptr, segment_len ) );
            segment_len = 0;
            begin_ptr = p + 1;
        } else {
            ++segment_len;
        }
    }

    if ( begin_ptr < p ) {
        into.push_back( std::string( begin_ptr, segment_len ) );
    }
}

std::string BinString( std::string words )
{
    std::string binaryString;
    binaryString.reserve(words.size() * 8); // Allocation optimization for string construction loops
    for ( char& _char : words ) {
        binaryString += std::bitset<8>(_char).to_string();
    }
    return binaryString;
}
