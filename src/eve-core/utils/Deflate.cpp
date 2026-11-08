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
    Author:        Zhur
    Update:     Allan
*/

#include <zlib.h>

#include "eve-core.h"
#include "utils/Deflate.h"
#include "utils/Buffer.h"


const uint8 DeflateHeaderByte = 0x78; // 'x'

bool IsDeflated( const Buffer& data )
{
    if ( data.size() == 0 ) {
        return false;
    }
    return ( DeflateHeaderByte == data[0] );
}

bool DeflateData( Buffer& data )
{
    Buffer dataDeflated;
    if ( !DeflateData( data, dataDeflated ) ) {
        return false;
    }

    data = std::move(dataDeflated); // Use C++11 move semantics to skip internal array copies!
    return true;
}

bool DeflateData( const Buffer& input, Buffer& output )
{
    if ( input.size() == 0 ) {
        output.Resize<uint8>(0);
        return false;
    }

    const size_t old_output_offset = output.size();
    uLongf outputSize = compressBound( input.size() );

    // Scale the destination raw byte footprint directly without iterator indirection
    output.Resize<uint8>( old_output_offset + outputSize );

    int res = compress(
    output.get_raw_ptr() + old_output_offset,
                       &outputSize,
                        input.get_raw_ptr(),
                       input.size()
    );

    if ( Z_OK == res ) {
        output.Resize<uint8>( old_output_offset + outputSize );
        return true;
    } else {
        output.Resize<uint8>( old_output_offset );
        return false;
    }
}

bool InflateData( Buffer& data )
{
    Buffer dataInflated;
    if ( !InflateData( data, dataInflated ) ) {
        return false;
    }

    data = std::move(dataInflated); // Modern fast reference swap
    return true;
}

bool InflateData( const Buffer& input, Buffer& output )
{
    if ( input.size() == 0 ) {
        output.Resize<uint8>(0);
        return false;
    }

    const size_t old_output_offset = output.size();

    // Start with a safe estimate: typical zlib compression ratio averages 4x expand
    uLongf outputSize = input.size() * 4;
    int res = Z_BUF_ERROR;

    // Streamlined allocation strategy preventing memory fragmentation overruns
    for ( size_t attempts = 1; attempts <= 5 && res == Z_BUF_ERROR; ++attempts ) {
        output.Resize<uint8>( old_output_offset + outputSize );

        res = uncompress(
        output.get_raw_ptr() + old_output_offset,
                         &outputSize,
                          input.get_raw_ptr(),
                         input.size()
        );

        if ( res == Z_BUF_ERROR ) {
            outputSize *= 2; // Step up allocation linearly if destination room was insufficient
        }
    }

    if ( Z_OK == res ) {
        output.Resize<uint8>( old_output_offset + outputSize );
        return true;
    } else {
        output.Resize<uint8>( old_output_offset );
        return false;
    }
}
