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
    Author:     Bloody.Rabbit, Captnoord, Zhur
    Update:     Allan
*/

#include "eve-common.h"
#include "utils/utfUtils.h"
#include "python/classes/PyDatabase.h"
#include "python/PyVisitor.h"
#include "python/PyRep.h"
#include "marshal/EVEUnmarshal.h"
#include "marshal/EVEMarshalOpcodes.h"
#include "marshal/EVEMarshalStringTable.h"
#include "utils/EVEUtils.h"
#include "utils/Buffer.h"

PyRep* Unmarshal( const Buffer& data )
{
    UnmarshalStream stream;
    return stream.Load( data );
}

PyRep* InflateUnmarshal( const Buffer& data )
{
    if (IsDeflated(data)) {
        Buffer inflatedData;
        if (!InflateData(data, inflatedData)) {
            return nullptr;
        }
        return Unmarshal(inflatedData);
    }
    return Unmarshal(data);
}

UnmarshalStream::~UnmarshalStream()
{
    // Object store lists are cleanly decref'd inside DestroyObjectStore
}

/********************************************************************/
/* UnmarshalStream Static Jump Table Mapping                        */
/********************************************************************/
PyRep* ( UnmarshalStream::* const UnmarshalStream::s_mLoadMap[ PyRepOpcodeMask + 1 ] )() =
{
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadNone,                 //Op_PyNone
    &UnmarshalStream::LoadToken,                //Op_PyToken
    &UnmarshalStream::LoadIntegerLongLong,      //Op_PyLongLong
    &UnmarshalStream::LoadIntegerLong,          //Op_PyLong
    &UnmarshalStream::LoadIntegerSignedShort,   //Op_PySignedShort
    &UnmarshalStream::LoadIntegerByte,          //Op_PyByte
    &UnmarshalStream::LoadIntegerMinusOne,      //Op_PyMinusOne
    &UnmarshalStream::LoadIntegerZero,          //Op_PyZeroInteger
    &UnmarshalStream::LoadIntegerOne,           //Op_PyOneInteger
    &UnmarshalStream::LoadReal,                 //Op_PyReal
    &UnmarshalStream::LoadRealZero,             //Op_PyZeroReal
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadBuffer,               //Op_PyBuffer
    &UnmarshalStream::LoadStringEmpty,          //Op_PyEmptyString
    &UnmarshalStream::LoadStringChar,           //Op_PyCharString
    &UnmarshalStream::LoadStringShort,          //Op_PyShortString
    &UnmarshalStream::LoadStringTable,          //Op_PyStringTableItem
    &UnmarshalStream::LoadWStringUCS2,          //Op_PyWStringUCS2
    &UnmarshalStream::LoadStringLong,           //Op_PyLongString
    &UnmarshalStream::LoadTuple,                //Op_PyTuple
    &UnmarshalStream::LoadList,                 //Op_PyList
    &UnmarshalStream::LoadDict,                 //Op_PyDict
    &UnmarshalStream::LoadObject,               //Op_PyObject
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadSubStruct,            //Op_PySubStruct
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadSavedStreamElement,   //Op_PySavedStreamElement
    &UnmarshalStream::LoadChecksumedStream,     //Op_PyChecksumedStream
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadBoolTrue,             //Op_PyTrue
    &UnmarshalStream::LoadBoolFalse,            //Op_PyFalse
    &UnmarshalStream::LoadError,                //Op_cPicked
    &UnmarshalStream::LoadObjectEx1,            //Op_PyObjectEx1
    &UnmarshalStream::LoadObjectEx2,            //Op_PyObjectEx2
    &UnmarshalStream::LoadTupleEmpty,           //Op_PyEmptyTuple
    &UnmarshalStream::LoadTupleOne,             //Op_PyOneTuple
    &UnmarshalStream::LoadListEmpty,            //Op_PyEmptyList
    &UnmarshalStream::LoadListOne,              //Op_PyOneList
    &UnmarshalStream::LoadWStringEmpty,         //Op_PyEmptyWString
    &UnmarshalStream::LoadWStringUCS2Char,      //Op_PyWStringUCS2Char
    &UnmarshalStream::LoadPackedRow,            //Op_PyPackedRow
    &UnmarshalStream::LoadSubStream,            //Op_PySubStream
    &UnmarshalStream::LoadTupleTwo,             //Op_PyTwoTuple
    &UnmarshalStream::LoadError,                //Op_PackedTerminator
    &UnmarshalStream::LoadWStringUTF8,          //Op_PyWStringUTF8
    &UnmarshalStream::LoadIntegerVar,           //Op_PyVarInteger
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError,
    &UnmarshalStream::LoadError
};

PyRep* UnmarshalStream::Load( const Buffer& data )
{
    if (data.size() == 0) {
        return nullptr;
    }

    // Direct registration of high-speed memory buffers
    mInPtr = data.get_raw_ptr();
    mStartPtr = mInPtr;
    mEndPtr = mInPtr + data.size();

    PyRep* res = LoadStream( data.size() );

    mInPtr = nullptr;
    mStartPtr = nullptr;
    mEndPtr = nullptr;

    return res;
}

PyRep* UnmarshalStream::LoadStream( size_t streamLength )
{
    const uint8 header = Read<uint8>();
    if (MarshalHeaderByte != header) {
        sLog.Error( "Unmarshal", "Invalid stream received (header byte 0x%X).", header );
        return nullptr;
    }

    const uint32 saveCount = Read<uint32>();
    CreateObjectStore( streamLength - sizeof( uint8 ) - sizeof( uint32 ), saveCount );

    PyRep* rep = LoadRep();

    DestroyObjectStore();
    return rep;
}

PyRep* UnmarshalStream::LoadRep()
{
    if (mInPtr >= mEndPtr)
        return nullptr;

    const uint8 header = Read<uint8>();

    const bool flagExtended = ( header & PyRepExtendedMask ) != 0;
    const bool flagSave = ( header & PyRepSaveMask ) != 0;
    const uint8 opcode = ( header & PyRepOpcodeMask );

    if ( flagExtended )
        sLog.Warning( "Unmarshal", "Encountered Extended flag in header 0x%X.", header );

    const uint32 storageIndex = ( flagSave ? GetStorageIndex() : 0 );

    PyRep* rep = ( this->*s_mLoadMap[ opcode ] )();
    
    if (storageIndex != 0)
        StoreObject(storageIndex, rep);

    return rep;
}

void UnmarshalStream::CreateObjectStore( size_t streamLength, uint32 saveCount )
{
    DestroyObjectStore();

    if (saveCount > 0) {
        const uint8* index_table_ptr = mStartPtr + sizeof(uint8_t) + streamLength;
        mStoreIndexPtr = reinterpret_cast<const uint32_t*>(index_table_ptr) - saveCount;
        mStoredObjects = new PyList( saveCount );
    }
}

void UnmarshalStream::DestroyObjectStore()
{
    mStoreIndexPtr = nullptr;
    PySafeDecRef( mStoredObjects );
}

PyRep* UnmarshalStream::GetStoredObject( uint32 index )
{
    if ( index > 0 && mStoredObjects != nullptr ) {
        return mStoredObjects->GetItem( --index );
    }
    return nullptr;
}

void UnmarshalStream::StoreObject( uint32 index, PyRep* object )
{
    if (index > 0 && mStoredObjects != nullptr) {
        PyIncRef( object );
        mStoredObjects->SetItem( --index, object );
    }
}

PyRep* UnmarshalStream::LoadIntegerVar()
{
    const uint32 len = ReadSizeEx();
    const uint8* data_ptr = Read<uint8>( len );

    if ( sizeof( int32 ) >= len ) {
        int32 intval = 0;
        std::memcpy( &intval, data_ptr, len );
        return new PyInt( intval );
    } else if ( sizeof( int64 ) >= len ) {
        int64 intval = 0;
        std::memcpy( &intval, data_ptr, len );
        return new PyLong( intval );
    } else {
        // FIX: Construct standard sequential containers using safe raw pointer boundaries
        Buffer scratch_buf;
        scratch_buf.AppendSeq(data_ptr, data_ptr + len);
        return new PyBuffer( scratch_buf );
    }
}

PyRep* UnmarshalStream::LoadStringChar()
{
    const char* str_ptr = Read<char>( 1 );
    return new PyString( str_ptr, 1 );
}

PyRep* UnmarshalStream::LoadStringShort()
{
    const uint8 len = Read<uint8>();
    const char* str_ptr = Read<char>( len );
    return new PyString( str_ptr, len );
}

PyRep* UnmarshalStream::LoadStringLong()
{
    const uint32 len = ReadSizeEx();
    const char* str_ptr = Read<char>( len );
    return new PyString( str_ptr, len );
}

PyRep* UnmarshalStream::LoadStringTable()
{
    const uint8 index = Read<uint8>();

    const char* str = sMarshalStringTable.LookupString( index );
    if (str == nullptr) {
        assert( false );
        sLog.Error( "Unmarshal", "String Table Item %u is out of range!", index );

        char ebuf[64];
        std::snprintf( ebuf, sizeof(ebuf), "Invalid String Table Item %u", index );
        return new PyString( ebuf );
    }

    return new PyString( str );
}

PyRep* UnmarshalStream::LoadWStringUCS2Char()
{
    const uint16* wstr_ptr = Read<uint16>( 1 );

    // FIX: Using our updated, high-performance wide string converter
    std::u16string u16_str(reinterpret_cast<const char16_t*>(wstr_ptr), 1);
    std::string utf8_str = utf16to8(u16_str);

    return new PyWString( utf8_str );
}

PyRep* UnmarshalStream::LoadWStringUCS2()
{
    const uint32 len = ReadSizeEx();
    const uint16* wstr_ptr = Read<uint16>( len );

    // FIX: Decoupled old third-party iterator wrappers for standard u16string primitives
    std::u16string u16_str(reinterpret_cast<const char16_t*>(wstr_ptr), len);
    std::string utf8_str = utf16to8(u16_str);

    return new PyWString( utf8_str );
}
PyRep* UnmarshalStream::LoadWStringUTF8()
{
    const uint32 len = ReadSizeEx();
    const char* wstr_ptr = Read<char>( len );

    // FIX: Using explicit data lengths instead of pointer-addition iterator ranges
    return new PyWString( wstr_ptr, len );
}

PyRep* UnmarshalStream::LoadToken()
{
    const uint8 len = Read<uint8>();
    const char* str_ptr = Read<char>( len );

    return new PyToken( str_ptr, len );
}

PyRep* UnmarshalStream::LoadBuffer()
{
    const uint32 len = ReadSizeEx();
    const uint8* data_ptr = Read<uint8>( len );

    Buffer scratch_buf;
    if (len > 0) {
        scratch_buf.AppendSeq(data_ptr, data_ptr + len);
    }
    return new PyBuffer( scratch_buf );
}

PyRep* UnmarshalStream::LoadTuple()
{
    const uint32 count = ReadSizeEx();
    PyTuple* tuple = new PyTuple( count );

    for ( uint32 i = 0; i < count; i++ ) {
        PyRep* rep = LoadRep();
        if (rep == nullptr) {
            PyDecRef( tuple );
            return nullptr;
        }

        tuple->SetItem( i, rep );
    }

    return tuple;
}

PyRep* UnmarshalStream::LoadTupleOne()
{
    PyRep* i = LoadRep();
    if (i == nullptr) {
        return nullptr;
    }

    PyTuple* tuple = new PyTuple( 1 );
    tuple->SetItem( 0, i );

    return tuple;
}

PyRep* UnmarshalStream::LoadTupleTwo()
{
    PyRep* i = LoadRep();
    if (i == nullptr) {
        return nullptr;
    }

    PyRep* j = LoadRep();
    if (j == nullptr) {
        PyDecRef(i);
        return nullptr;
    }

    PyTuple *tuple = new PyTuple( 2 );
    tuple->SetItem( 0, i );
    tuple->SetItem( 1, j );

    return tuple;
}

PyRep* UnmarshalStream::LoadList()
{
    const uint32 count = ReadSizeEx();
    PyList* list = new PyList( count );

    for ( uint32 i = 0; i < count; i++ ) {
        PyRep* rep = LoadRep();
        if (rep == nullptr) {
            PyDecRef( list );
            return nullptr;
        }

        list->SetItem( i, rep );
    }

    return list;
}

PyRep* UnmarshalStream::LoadListOne()
{
    PyRep* i = LoadRep();
    if (i == nullptr) {
        return nullptr;
    }

    PyList* list = new PyList();
    list->AddItem( i );

    return list;
}

PyRep* UnmarshalStream::LoadDict()
{
    const uint32 count = ReadSizeEx();
    PyDict* dict = new PyDict();

    for ( uint32 i = 0; i < count; i++ ) {
        PyRep* value = LoadRep();
        if (value == nullptr) {
            PyDecRef(dict);
            return nullptr;
        }

        PyRep* key = LoadRep();
        if (key == nullptr) {
            PyDecRef(dict);
            PyDecRef(value);
            return nullptr;
        }

        dict->SetItem(key, value);
    }

    return dict;
}

PyRep* UnmarshalStream::LoadObject()
{
    PyRep* type = LoadRep();
    if (type == nullptr) {
        return nullptr;
    }

    if ( !type->IsString() ) {
        sLog.Error( "Unmarshal", "Object: Expected 'String' as type, got '%s'.", type->TypeString() );
        PyDecRef( type );
        return nullptr;
    }

    PyRep* arguments = LoadRep();
    if (arguments == nullptr) {
        PyDecRef( type );
        return nullptr;
    }

    return new PyObject( type->AsString(), arguments );
}

PyRep* UnmarshalStream::LoadObjectEx1()
{
    return LoadObjectEx( false );
}

PyRep* UnmarshalStream::LoadObjectEx2()
{
    return LoadObjectEx( true );
}

PyRep* UnmarshalStream::LoadSubStream()
{
    const uint32 len = ReadSizeEx();
    const uint8* data_ptr = Read<uint8>( len );

    Buffer scratch_buf;
    if (len > 0) {
        scratch_buf.AppendSeq(data_ptr, data_ptr + len);
    }
    return new PySubStream( new PyBuffer( scratch_buf ) );
}

PyRep* UnmarshalStream::LoadSubStruct()
{
    PyRep* ss = LoadRep();
    if (ss == nullptr) {
        return nullptr;
    }

    return new PySubStruct( ss );
}

PyRep* UnmarshalStream::LoadChecksumedStream() {
    const uint32 sum = Read<uint32>();

    PyRep* ss = LoadRep();
    if (ss == nullptr)
        return nullptr;

    return new PyChecksumedStream(ss, sum);
}

PyRep* UnmarshalStream::LoadPackedRow()
{
    PyRep* header_element = LoadRep();
    if (header_element == nullptr) {
        return nullptr;
    }

    Buffer unpacked;
    if ( !LoadZeroCompressed( unpacked ) ) {
        PyDecRef( header_element );
        return nullptr;
    }

    PyPackedRow* row = new PyPackedRow( reinterpret_cast<DBRowDescriptor*>(header_element) );

    // Create size map, sorted from the greatest to the smallest value
    std::multimap< uint8, uint32, std::greater< uint8 > > sizeMap;
    uint32 cc = row->header()->ColumnCount();
    size_t sum = 0;

    for ( uint32 i = 0; i < cc; i++ ) {
        uint8 size = DBTYPE_GetSizeBits( row->header()->GetColumnType( i ) );
        sizeMap.insert( std::make_pair( size, i ) );
        sum += size;
    }

    sum = ( ( sum + 7 ) >> 3 );
    unpacked.Resize<uint8>( sum );

    // FIX: Swapped out custom iterators for high-performance raw reading cursors
    const uint8* cursor = unpacked.get_raw_ptr();
    const uint8* const unpacked_end = cursor + unpacked.size();
    uint8 bitOffset = 0;

    std::multimap< uint8, uint32, std::greater< uint8 > >::iterator cur = sizeMap.begin();
    std::multimap< uint8, uint32, std::greater< uint8 > >::iterator end = sizeMap.end();

    for (; cur != end; ++cur) {
        const uint32 index = cur->second;

        switch( row->header()->GetColumnType( index ) ) {
            case DBTYPE_I8:
            case DBTYPE_CY:
            case DBTYPE_UI8:
            case DBTYPE_FILETIME: {
                int64 val = 0;
                std::memcpy(&val, cursor, sizeof(int64));
                row->SetField( index, new PyLong( val ) );
                cursor += sizeof(int64);
                break;
            }
            case DBTYPE_I4: {
                int32 val = 0;
                std::memcpy(&val, cursor, sizeof(int32));
                row->SetField( index, new PyInt( val ) );
                cursor += sizeof(int32);
                break;
            }
            case DBTYPE_UI4: {
                uint32 val = 0;
                std::memcpy(&val, cursor, sizeof(uint32));
                row->SetField( index, new PyInt( static_cast<int32>(val) ) );
                cursor += sizeof(uint32);
                break;
            }
            case DBTYPE_I2: {
                int16 val = 0;
                std::memcpy(&val, cursor, sizeof(int16));
                row->SetField( index, new PyInt( val ) );
                cursor += sizeof(int16);
                break;
            }
            case DBTYPE_UI2: {
                uint16 val = 0;
                std::memcpy(&val, cursor, sizeof(uint16));
                row->SetField( index, new PyInt( val ) );
                cursor += sizeof(uint16);
                break;
            }
            case DBTYPE_I1: {
                int8 val = 0;
                std::memcpy(&val, cursor, sizeof(int8));
                row->SetField( index, new PyInt( val ) );
                cursor += sizeof(int8);
                break;
            }
            case DBTYPE_UI1: {
                uint8 val = *cursor;
                row->SetField( index, new PyInt( val ) );
                cursor += sizeof(uint8);
                break;
            }
            case DBTYPE_R8: {
                double val = 0.0;
                std::memcpy(&val, cursor, sizeof(double));
                row->SetField( index, new PyFloat( val ) );
                cursor += sizeof(double);
                break;
            }
            case DBTYPE_R4: {
                float val = 0.0f;
                std::memcpy(&val, cursor, sizeof(float));
                row->SetField( index, new PyFloat( val ) );
                cursor += sizeof(float);
                break;
            }
            case DBTYPE_BOOL: {
                if ( bitOffset > 7 ) {
                    bitOffset = 0;
                    ++cursor;
                }
                if (cursor < unpacked_end) {
                    row->SetField( index, new PyBool( ( *cursor >> bitOffset++ ) & 0x01 ) );
                }
                break;
            }
            case DBTYPE_BYTES:
            case DBTYPE_STR:
            case DBTYPE_WSTR: {
                PyRep* el = LoadRep();
                if (el == nullptr) {
                    PyDecRef( row );
                    return nullptr;
                }
                row->SetField( index, el );
                break;
            }
            case DBTYPE_EMPTY:
            case DBTYPE_ERROR: {
                PySafeDecRef(row);
                return nullptr;
            }
        }
    }

    return row;
}

PyRep* UnmarshalStream::LoadError()
{
    sLog.Error( "Unmarshal", "Invalid opcode encountered." );
    return nullptr;
}

PyRep* UnmarshalStream::LoadSavedStreamElement()
{
    const uint32 index = ReadSizeEx();
    PyRep* obj = GetStoredObject( index );
    if (obj == nullptr) {
        sLog.Error( "Unmarshal", "SavedStreamElement: Got invalid stored object." );
        return nullptr;
    }
    return obj;
}

PyObjectEx* UnmarshalStream::LoadObjectEx( bool is_type_2 )
{
    PyRep* header = LoadRep();
    if (header == nullptr) {
        return nullptr;
    }

    PyObjectEx* obj = new PyObjectEx( is_type_2, header );

    while ( Op_PackedTerminator != Peek<uint8>() ) {
        PyRep* el = LoadRep();
        if (el == nullptr) {
            PyDecRef( obj );
            return nullptr;
        }
        obj->list().AddItem( el );
    }
    Read<uint8>(); // skip Op_PackedTerminator

    while ( Op_PackedTerminator != Peek<uint8>() ) {
        PyRep* key = LoadRep();
        if (key == nullptr) {
            PyDecRef( obj );
            return nullptr;
        }

        PyRep* value = LoadRep();
        if (value == nullptr) {
            PyDecRef( key );
            PyDecRef( obj );
            return nullptr;
        }
        obj->dict().SetItem( key, value );
    }
    Read<uint8>(); // skip Op_PackedTerminator

    return obj;
}

bool UnmarshalStream::LoadZeroCompressed( Buffer& into )
{
    const uint32 packedLen = ReadSizeEx();
    if (packedLen == 0) {
        return true;
    }

    // FIX: Using raw input tracking bounds to decouple ancient iterator shifts
    const uint8* cur = Read<uint8>( packedLen );
    const uint8* const packet_end = cur + packedLen;

    while ( cur < packet_end ) {
        // Load the single byte control opcode safely
        const uint8 op_byte = *cur++;

        // FIX: Extract bit masks directly from raw byte variables to prevent strict-aliasing bugs
        const bool firstIsZero  = (op_byte & 0x01) != 0;
        const uint8 firstLen    = (op_byte >> 1) & 0x07;
        const bool secondIsZero = (op_byte & 0x10) != 0;
        const uint8 secondLen   = (op_byte >> 5) & 0x07;

        // --- Streamlined Part 1 Decode Pass ---
        if (firstIsZero) {
            uint8 len = firstLen + 1;
            while (len > 0) {
                into.Append<uint8>( 0 );
                --len;
            }
        } else {
            const size_t bytes_to_read = 8 - firstLen;
            const uint8* dataEnd = (cur + bytes_to_read < packet_end) ? (cur + bytes_to_read) : packet_end;

            if (dataEnd > cur) {
                into.AppendSeq( cur, dataEnd );
                cur = dataEnd;
            }
        }

        // --- Streamlined Part 2 Decode Pass ---
        if (cur < packet_end) {
            if (secondIsZero) {
                uint8 len = secondLen + 1;
                while (len > 0) {
                    into.Append<uint8>( 0 );
                    --len;
                }
            } else {
                const size_t bytes_to_read = 8 - secondLen;
                const uint8* dataEnd = (cur + bytes_to_read < packet_end) ? (cur + bytes_to_read) : packet_end;

                if (dataEnd > cur) {
                    into.AppendSeq( cur, dataEnd );
                    cur = dataEnd;
                }
            }
        }
    }

    return true;
}
