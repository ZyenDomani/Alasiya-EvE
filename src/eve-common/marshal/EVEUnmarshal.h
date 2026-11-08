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
*/

#pragma once

#include <cstdint>
#include <cstring>
#include <cassert>

#include "python/PyRep.h"

class Buffer;

/**
 * @brief Turns marshal stream into Python object.
 */
extern PyRep* Unmarshal( const Buffer& data );

/**
 * @brief Turns possibly inflated marshal stream into Python object.
 */
extern PyRep* InflateUnmarshal( const Buffer& data );

/**
 * @brief Class which turns marshal bytecode into Python object.
 */
class UnmarshalStream
{
public:
    UnmarshalStream()
    : mStoredObjects( nullptr ),
    mInPtr( nullptr ),
    mStartPtr( nullptr ),
    mEndPtr( nullptr ),
    mStoreIndexPtr( nullptr )
    {
    }

    ~UnmarshalStream();

    PyRep* Load( const Buffer& data );

protected:
    // --- Optimized High-Performance Pointer Core (No iterators, zero allocation bloat) ---

    /**
     * @brief Reads a single element from the raw stream memory.
     *        FIX: Copies the type by value securely to protect against unaligned memory access crashes.
     */
    template<typename T>
    T Read()
    {
        assert( mInPtr != nullptr );
        assert( mInPtr + sizeof(T) <= mEndPtr ); // Hard protection against malformed packet overruns

        T value;
        std::memcpy(&value, mInPtr, sizeof(T));
        mInPtr += sizeof(T); // Linear pointer register shift
        return value;
    }

    /**
     * @brief Returns a direct pointer to an array block in the stream and advances the cursor.
     */
    template<typename T>
    const T* Read( size_t count )
    {
        const size_t total_bytes = count * sizeof(T);
        assert( mInPtr != nullptr );
        assert( mInPtr + total_bytes <= mEndPtr );

        const T* res = reinterpret_cast<const T*>(mInPtr);
        mInPtr += total_bytes;
        return res;
    }

    /** @brief Peeks a single element from the stream without advancing the cursor. */
    template<typename T>
    T Peek() const
    {
        assert( mInPtr != nullptr );
        assert( mInPtr + sizeof(T) <= mEndPtr );

        T value;
        std::memcpy(&value, mInPtr, sizeof(T));
        return value;
    }

    /** @brief Peeks an array block from the stream without advancing the cursor. */
    template<typename T>
    const T* Peek( size_t count ) const
    {
        assert( mInPtr != nullptr );
        assert( mInPtr + (count * sizeof(T)) <= mEndPtr );
        return reinterpret_cast<const T*>(mInPtr);
    }

    uint32 ReadSizeEx()
    {
        uint32 size = Read<uint8>();
        if ( 0xFF == size ) {
            size = Read<uint32>();
        }
        return size;
    }

    PyRep* LoadStream( size_t streamLength );
    PyRep* LoadRep();

    void CreateObjectStore( size_t streamLength, uint32 saveCount );
    void DestroyObjectStore();
    /**
     * @brief Obtains storage index for StoreObject.
     *        FIX: Replaced custom iterator increments with a lightning-fast raw pointer dereference loop!
     *
     * @return Storage index.
     */
    uint32 GetStorageIndex()
    {
        assert( mStoreIndexPtr != nullptr );
        return *mStoreIndexPtr++;
    }

    PyRep* GetStoredObject( uint32 index );
    void StoreObject( uint32 index, PyRep* object );

private:
    PyRep* LoadNone() { return PyStatic.NewNone(); }

    PyRep* LoadBoolTrue() { return PyStatic.NewTrue(); }
    PyRep* LoadBoolFalse() { return PyStatic.NewFalse(); }

    PyRep* LoadIntegerLongLong() { return new PyLong( Read<int64>() ); }
    PyRep* LoadIntegerLong() { return new PyInt( Read<int32>() ); }
    PyRep* LoadIntegerSignedShort() { return new PyInt( Read<int16>() ); }
    PyRep* LoadIntegerByte() { return new PyInt( Read<int8>() ); }
    PyRep* LoadIntegerVar();
    PyRep* LoadIntegerMinusOne() { return PyStatic.NewNegOne(); }
    PyRep* LoadIntegerZero() { return PyStatic.NewZero(); }
    PyRep* LoadIntegerOne() { return PyStatic.NewOne(); }

    PyRep* LoadReal() { return new PyFloat( Read<double>() ); }
    PyRep* LoadRealZero() { return new PyFloat( 0.0 ); }

    PyRep* LoadStringEmpty() { return new PyString( "" ); }
    PyRep* LoadStringChar();
    PyRep* LoadStringShort();
    PyRep* LoadStringLong();
    PyRep* LoadStringTable();

    PyRep* LoadWStringEmpty() { return new PyWString( "", 0 ); }
    PyRep* LoadWStringUCS2Char();
    PyRep* LoadWStringUCS2();
    PyRep* LoadWStringUTF8();

    PyRep* LoadToken();
    PyRep* LoadBuffer();

    PyRep* LoadTupleEmpty() { return PyStatic.mtTuple(); }
    PyRep* LoadTuple();
    PyRep* LoadTupleOne();
    PyRep* LoadTupleTwo();

    PyRep* LoadListEmpty() { return PyStatic.mtList(); }
    PyRep* LoadList();
    PyRep* LoadListOne();

    PyRep* LoadDict();

    PyRep* LoadObject();
    PyRep* LoadObjectEx1();
    PyRep* LoadObjectEx2();

    PyRep* LoadSubStream();
    PyRep* LoadSubStruct();
    PyRep* LoadChecksumedStream();

    PyRep* LoadPackedRow();

    PyRep* LoadError();
    PyRep* LoadSavedStreamElement();

    PyObjectEx* LoadObjectEx( bool is_type_2 );
    bool LoadZeroCompressed( Buffer& into );

    // --- Modernized Raw Data Stream Cursors (Replaced the volatile C++98 custom iterators) ---

    /** @brief Active reading extraction pointer cursor on the stream block. */
    const uint8* mInPtr;

    /** @brief Pin marker tracking the absolute memory start boundary of the incoming data package. */
    const uint8* mStartPtr;

    /** @brief Hard cap safety fence defining the absolute memory end boundary of the packet storage array. */
    const uint8* mEndPtr;

    /** @brief High-speed pointer targeting the serialized Python back-reference storage index array. */
    const uint32* mStoreIndexPtr;

    /** @brief Storage collection tracking active referenced Python variables. */
    PyList* mStoredObjects;

    static PyRep* ( UnmarshalStream::* const s_mLoadMap[] )();
};
