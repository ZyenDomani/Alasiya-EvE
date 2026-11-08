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
    Rewrite:    Allan
*/

#pragma once

#include <cstring>

#include "utils/misc.h"
#include "memory/SafeMem.h"


// NOTE:  this class had some questionable calls and interactions.  it is now fixed.  -allan

class Buffer
{
public:
    template< typename T >
    class const_iterator : public std::iterator< std::random_access_iterator_tag, T >
    {
        typedef std::iterator< std::random_access_iterator_tag, T > _Base;

    public:
        typedef typename _Base::iterator_category iterator_category;
        typedef typename _Base::value_type        value_type;
        typedef typename _Base::difference_type   difference_type;
        typedef typename _Base::pointer           pointer;
        typedef typename _Base::reference         reference;
        typedef const T*                           const_pointer;
        typedef const T&                           const_reference;

        const_iterator( const Buffer* buffer = nullptr, size_t index = 0 )
        : mIndex( index ), mBuffer( buffer ) {}

        const_iterator( const const_iterator& oth ) = default;
        const_iterator& operator=( const const_iterator& oth ) = default;
        const_iterator( const_iterator&& oth ) noexcept = default;
        const_iterator& operator=( const_iterator&& oth ) noexcept = default;

        template< typename T2 >
        const_iterator< T2 > As() const { return const_iterator< T2 >( mBuffer, mIndex ); }

        // EXPLICIT FIXED GAUNTLET: Stripped out the recursive 'mBuffer->end()' dependency
        const_reference operator*() const {
            assert( mBuffer != nullptr );
            return *reinterpret_cast<const_pointer>(&mBuffer->get_raw_vector()[ mIndex ]);
        }

        const_pointer operator->() const { return &**this; }
        const_reference operator[]( difference_type diff ) const { return *( *this + diff ); }

        const_iterator operator+( difference_type diff ) const { const_iterator res( *this ); return ( res += diff ); }

        const_iterator& operator+=( difference_type diff ) {
            const difference_type byte_diff = ( diff * sizeof( value_type ) );
            assert( mBuffer != nullptr );
            mIndex += byte_diff;
            return *this;
        }

        const_iterator& operator++() { return ( *this += 1 );  }
        const_iterator operator++( int ) { const_iterator res( *this ); ++*this; return res; }

        const_iterator operator-( difference_type diff ) const { const_iterator res( *this ); return ( res -= diff ); }
        const_iterator& operator-=( difference_type diff ) { return ( *this += ( -diff ) ); }
        const_iterator& operator--() { return ( *this -= 1 ); }
        const_iterator operator--( int ) { const_iterator res( *this ); --*this; return res; }

        difference_type operator-( const const_iterator& oth ) const {
            assert( oth.mBuffer == mBuffer );
            return ( static_cast<difference_type>(mIndex - oth.mIndex) / sizeof( value_type ) );
        }

        bool operator==( const const_iterator& oth ) const { assert( oth.mBuffer == mBuffer ); return ( mIndex == oth.mIndex ); }
        bool operator!=( const const_iterator& oth ) const { return !( *this == oth ); }
        bool operator<( const const_iterator& oth ) const { assert( oth.mBuffer == mBuffer ); return ( mIndex < oth.mIndex ); }
        bool operator>( const const_iterator& oth ) const { assert( oth.mBuffer == mBuffer ); return ( mIndex > oth.mIndex ); }
        bool operator<=( const const_iterator& oth ) const { return !( *this > oth ); }
        bool operator>=( const const_iterator& oth ) const { return !( *this < oth ); }

        // Getter helper we will hook up in the main class body to keep mBuffer private/safe
        size_t get_internal_index() const { return mIndex; }

    protected:
        size_t mIndex;
        const Buffer* mBuffer;
    };

    template< typename T >
    class iterator : public const_iterator< T >
    {
        typedef const_iterator< T > _Base;

    public:
        typedef typename _Base::iterator_category iterator_category;
        typedef typename _Base::value_type        value_type;
        typedef typename _Base::difference_type   difference_type;
        typedef typename _Base::pointer           pointer;
        typedef typename _Base::const_pointer     const_pointer;
        typedef typename _Base::reference         reference;
        typedef typename _Base::const_reference   const_reference;

        iterator( Buffer* buffer = nullptr, size_t index = 0 )
        : _Base( buffer, index ) {}

        iterator( const iterator& oth ) = default;
        iterator( iterator&& oth ) noexcept = default;

        // Clean C++11 assignment utilizing explicit base member access instead of unsafe casting
        iterator& operator=( const iterator& oth ) {
            _Base::mIndex = oth.mIndex;
            _Base::mBuffer = oth.mBuffer;
            return *this;
        }

        iterator& operator=( iterator&& oth ) noexcept {
            _Base::mIndex = oth.mIndex;
            _Base::mBuffer = oth.mBuffer;
            return *this;
        }

        template< typename T2 >
        iterator< T2 > As() const { return iterator< T2 >( const_cast<Buffer*>(_Base::mBuffer), _Base::mIndex ); }

        // Clean const_cast boundary resolution
        reference operator*() const {
            return const_cast< reference >( _Base::operator*() );
        }

        pointer operator->() const { return &**this; }
        reference operator[]( difference_type diff ) const { return *( *this + diff ); }

        iterator operator+( difference_type diff ) const { iterator res( *this ); return ( res += diff ); }

        iterator& operator+=( difference_type diff ) {
            _Base::operator+=(diff);
            return *this;
        }

        iterator& operator++() { _Base::operator+=(1); return *this; }
        iterator operator++( int ) { iterator res( *this ); ++*this; return res; }

        iterator operator-( difference_type diff ) const { iterator res( *this ); return ( res -= diff ); }
        iterator& operator-=( difference_type diff ) { _Base::operator+=(-diff); return *this; }
        iterator& operator--() { _Base::operator+=(-1); return *this; }
        iterator operator--( int ) { iterator res( *this ); --*this; return res; }

        difference_type operator-( const _Base& oth ) const {
            return _Base::operator-(oth);
        }
    };

    // --- Main Buffer Class Enforcements ---

    Buffer( size_t len = 0, const uint8& fill = 0 )
    : mBuffer( nullptr ),
    mSize( 0 ),
    mCapacity( 0 )
    {
        Resize< uint8 >( len, fill );
    }

    template< typename Iter >
    Buffer( Iter first, Iter last )
    : mBuffer( nullptr ),
    mSize( 0 ),
    mCapacity( 0 )
    {
        AssignSeq( first, last );
    }

    // Modern C++11 Copy / Move Lifecycles (Rule of 5 Rule-set alignment)
    Buffer( const Buffer& oth )
    : mBuffer( nullptr ),
    mSize( 0 ),
    mCapacity( 0 )
    {
        *this = oth;
    }

    Buffer( Buffer&& oth ) noexcept
    : mBuffer( oth.mBuffer ),
    mSize( oth.mSize ),
    mCapacity( oth.mCapacity )
    {
        oth.mBuffer = nullptr;
        oth.mSize = 0;
        oth.mCapacity = 0;
    }

    Buffer& operator=( Buffer&& oth ) noexcept
    {
        if (this != &oth) {
            SafeFree(mBuffer);
            mBuffer = oth.mBuffer;
            mSize = oth.mSize;
            mCapacity = oth.mCapacity;

            oth.mBuffer = nullptr;
            oth.mSize = 0;
            oth.mCapacity = 0;
        }
        return *this;
    }

    ~Buffer()
    {
        SafeFree( mBuffer );
    }

    /********************************************************************/
    /* Read methods                                                     */
    /********************************************************************/
    template< typename T >
    iterator< T > begin() { return iterator< T >( this, 0 ); }

    template< typename T >
    const_iterator< T > begin() const { return const_iterator< T >( this, 0 ); }

    template< typename T >
    iterator< T > end() { return iterator< T >( this, size() ); }

    template< typename T >
    const_iterator< T > end() const { return const_iterator< T >( this, size() ); }

    template< typename T >
    T& Get( size_t index )  {
        const size_t byte_offset = index * sizeof(T);
        assert( byte_offset + sizeof(T) <= size() );
        return *reinterpret_cast<T*>( &mBuffer[ byte_offset ] );
    }

    template< typename T >
    const T& Get( size_t index ) const  {
        const size_t byte_offset = index * sizeof(T);
        assert( byte_offset + sizeof(T) <= size() );
        return *reinterpret_cast<const T*>( &mBuffer[ byte_offset ] );
    }

    uint8& operator[]( size_t index ) { assert( index < size() ); return mBuffer[ index ]; }
    const uint8& operator[]( size_t index ) const { assert( index < size() ); return mBuffer[ index ]; }

    /********************************************************************/
    /* Write methods                                                    */
    /********************************************************************/

    template< typename T >
    void Append( const T& value ) {
        const size_t old_size = size();
        // Modern approach: Explicitly scale the internal byte store directly
        _ResizeBytes( old_size + sizeof(T) );
        // Safe casting via reinterpret_cast
        *reinterpret_cast<T*>( &mBuffer[ old_size ] ) = value;
    }

    template< typename Iter >
    void AppendSeq( Iter first, Iter last ) {
        typedef typename std::iterator_traits< Iter >::value_type ValueType;
        const size_t count = std::distance(first, last);
        if (count == 0)
            return;

        const size_t old_size = size();
        const size_t total_bytes = count * sizeof(ValueType);

        _ResizeBytes( old_size + total_bytes );
        std::memcpy( &mBuffer[ old_size ], &*first, total_bytes );
    }

    template< typename T >
    void Assign( const T& value ) {
        _ResizeBytes( sizeof(T) );
        *reinterpret_cast<T*>( &mBuffer[ 0 ] ) = value;
    }

    template< typename Iter >
    void AssignSeq( Iter first, Iter last ) {
        typedef typename std::iterator_traits< Iter >::value_type ValueType;
        const size_t count = std::distance(first, last);
        const size_t total_bytes = count * sizeof(ValueType);

        _ResizeBytes( total_bytes );
        if (total_bytes > 0)
            std::memcpy( &mBuffer[ 0 ], &*first, total_bytes );
    }

    template< typename T >
    void AssignAt( const_iterator< T > index, const T& value ) {
        const size_t _index = index.get_internal_index();
        // Safe, flat numeric assert boundaries
        assert( _index + sizeof(T) <= size() );
        *reinterpret_cast<T*>( &mBuffer[ _index ] ) = value;
    }

    template< typename Iter >
    void AssignSeqAt( const_iterator< typename std::iterator_traits< Iter >::value_type > index, Iter first, Iter last ) {
        typedef typename std::iterator_traits< Iter >::value_type ValueType;
        const size_t count = std::distance(first, last);
        if (count == 0)
            return;

        const size_t _index = index.get_internal_index();
        const size_t total_bytes = count * sizeof(ValueType);
        assert( _index + total_bytes <= size() );

        std::memmove( &mBuffer[ _index ], &*first, total_bytes );
    }

    template< typename T >
    Buffer& operator<<( const T& value ) { Append< T >( value ); return *this; }

    template< typename T >
    Buffer& operator=( const T& value ) { Assign< T >( value ); return *this; }

    Buffer& operator=( const Buffer& value ) {
        if (this != &value) {
            _ResizeBytes( value.size() );
            if (value.size() > 0)
                std::memcpy( &mBuffer[0], value.get_raw_ptr(), value.size() );
        }
        return *this;
    }

    /********************************************************************/
    /* Size methods                                                     */
    /********************************************************************/
    /// @return Current size of buffer, in bytes.
    size_t size() const { return mSize; }
    /// @return Current capacity of buffer, in bytes.
    size_t capacity() const { return mCapacity; }

    template< typename T >
    void Reserve( size_t requiredCount ) {
        _ReserveBytes( requiredCount * sizeof(T) );
    }

    template< typename T >
    void ReserveAt( const_iterator< T > index, size_t requiredCount ) {
        const size_t _index = index.get_internal_index();
        _ReserveBytes( _index + (requiredCount * sizeof(T)) );
    }

    template< typename T >
    void Resize( size_t requiredCount, const uint8& fill = 0 ) {
        const size_t old_size = mSize;
        const size_t new_bytes = requiredCount * sizeof(T);

        _ResizeBytes( new_bytes );

        if ( mSize > old_size )
            std::memset( &mBuffer[ old_size ], fill, mSize - old_size );
    }

    template< typename T >
    void ResizeAt( const_iterator< T > index, size_t requiredCount, const uint8& fill = 0 ) {
        const size_t _index = index.get_internal_index();
        const size_t old_size = mSize;
        const size_t new_bytes = _index + (requiredCount * sizeof(T));

        _ResizeBytes( new_bytes );

        if ( mSize > old_size )
            std::memset( &mBuffer[ old_size ], fill, mSize - old_size );
    }

    // High-performance public hooks for optimized iterators and serializers
    uint8* get_raw_ptr() { return mBuffer; }
    const uint8* get_raw_ptr() const { return mBuffer; }

    // Explicit array reference link for the custom const_iterator we built in Post 1
    const uint8* get_raw_vector() const { return mBuffer; }

protected:
    /// Pointer to start of buffer.
    uint8* mBuffer;
    /// Current size of buffer, in bytes.
    size_t mSize;
    /// Current capacity of buffer, in bytes.
    size_t mCapacity;

    // --- Streamlined C++11 Internal Sizing Core (No template clutter / No recursion loops) ---
    void _ResizeBytes( size_t requiredSizeInBytes ) {
        if ( requiredSizeInBytes > mCapacity )
            _Reallocate( requiredSizeInBytes );

        mSize = requiredSizeInBytes;
    }

    void _ReserveBytes( size_t requiredSizeInBytes ) {
        if ( requiredSizeInBytes > mCapacity )
            _Reallocate( requiredSizeInBytes );
    }

    void _Reallocate( size_t requiredSize ) {
        size_t newCapacity = _CalcBufferCapacity( mCapacity, requiredSize );
        assert( requiredSize <= newCapacity );

        if ( newCapacity != mCapacity ) {
            uint8* newBuffer = static_cast<uint8*>(std::realloc( mBuffer, newCapacity ));
            assert( newBuffer != nullptr ); // Halt server early if bare metal runs completely out of memory

            mBuffer = newBuffer;
            mCapacity = newCapacity;
        }
    }

    static size_t _CalcBufferCapacity( size_t currentCapacity, size_t requiredSize ) {
        size_t newCapacity = 0;

        if ( 0x100 < requiredSize ) {
            newCapacity = static_cast<size_t>( npowof2( requiredSize ) );
        } else if ( 0 < requiredSize ) {
            newCapacity = 0x100;
        }

        if ( requiredSize <= currentCapacity && currentCapacity < newCapacity ) {
            return currentCapacity;
        } else {
            return newCapacity;
        }
    }
};

