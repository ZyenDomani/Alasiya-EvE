/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Authors:    BloodyRabbit, Captnoord, Zhur
    Update:     Allan
*/

#include "eve-common.h"
#include "marshal/EVEMarshal.h"
#include "marshal/EVEMarshalOpcodes.h"
#include "marshal/EVEMarshalStringTable.h"
#include "python/classes/PyDatabase.h"
#include "python/PyRep.h"
#include "python/PyVisitor.h"
#include "utils/EVEUtils.h"
#include "utils/Buffer.h"

bool Marshal(const PyRep* rep, Buffer& into)
{
    // FIX: Stack allocation completely bypasses heap management overhead inside hot loops
    MarshalStream stream;
    return stream.Save(rep, into);
}

bool MarshalDeflate(const PyRep* rep, Buffer& into, const uint32 deflationLimit)
{
    // FIX: Stack allocated scratch buffer prevents dynamic heap allocations
    Buffer scratch_data;
    if (Marshal(rep, scratch_data)) {
        if (scratch_data.size() >= deflationLimit) {
            return DeflateData(scratch_data, into);
        } else {
            into.AppendSeq(scratch_data.get_raw_ptr(), scratch_data.get_raw_ptr() + scratch_data.size());
            return true;
        }
    }
    return false;
}

/********************************************************************/
/* MarshalStream Core Implementations                               */
/********************************************************************/

MarshalStream::MarshalStream()
: mBuffer(nullptr)
{
}

bool MarshalStream::Save(const PyRep* rep, Buffer& into)
{
    mBuffer = &into;
    bool res = SaveStream(rep);
    mBuffer = nullptr;
    return res;
}

bool MarshalStream::SaveStream(const PyRep* rep)
{
    Put<uint8>(MarshalHeaderByte);

    // Mapcount (EVE protocol object referencing; tracking placeholder left as zero)
    Put<uint32>(0);

    if (rep == nullptr) {
        return false;
    }
    return rep->visit(*this);
}

bool MarshalStream::VisitInteger(const PyInt* rep)
{
    if (rep == nullptr) { return false; }
    const int32 val = rep->value();

    if (val == -1) {
        Put<uint8>(Op_PyMinusOne);
    } else if (val == 0) {
        Put<uint8>(Op_PyZeroInteger);
    } else if (val == 1) {
        Put<uint8>(Op_PyOneInteger);
    } else if (val + 0x8000u > 0xFFFF) {
        Put<uint8>(Op_PyLong);
        Put<int32>(val);
    } else if (val + 0x80u > 0xFF) {
        Put<uint8>(Op_PySignedShort);
        Put<int16>(val);
    } else {
        Put<uint8>(Op_PyByte);
        Put<int8>(static_cast<int8>(val));
    }

    return true;
}

bool MarshalStream::VisitLong(const PyLong* rep)
{
    if (rep == nullptr) { return false; }
    SaveVarInteger(rep);
    return true;
}

bool MarshalStream::VisitBoolean(const PyBool* rep)
{
    if (rep == nullptr) { return false; }
    if (rep->value()) {
        Put<uint8>(Op_PyTrue);
    } else {
        Put<uint8>(Op_PyFalse);
    }
    return true;
}

bool MarshalStream::VisitReal(const PyFloat* rep)
{
    if (rep == nullptr) { return false; }
    if (rep->value() == 0.0) {
        Put<uint8>(Op_PyZeroReal);
    } else {
        Put<uint8>(Op_PyReal);
        Put<double>(rep->value());
    }
    return true;
}

bool MarshalStream::VisitNone(const PyNone* rep)
{
    Put<uint8>(Op_PyNone);
    return true;
}

bool MarshalStream::VisitBuffer(const PyBuffer* rep)
{
    if (rep == nullptr) { return false; }
    Put<uint8>(Op_PyBuffer);

    const Buffer& buf = rep->content();
    const uint32 buf_len = static_cast<uint32>(buf.size());

    PutSizeEx(buf_len);
    if (buf_len > 0) {
        // FIX: Directly leverages raw byte stream pointers, cutting out old template iterator lookups
        Put(buf.get_raw_ptr(), buf.get_raw_ptr() + buf_len);
    }

    return true;
}

bool MarshalStream::VisitString(const PyString* rep)
{
    if (rep == nullptr) { return false; }
    const std::string& str_content = rep->content();
    const size_t len = str_content.size();

    if (len == 0) {
        Put<uint8>(Op_PyEmptyString);
    } else if (len == 1) {
        Put<uint8>(Op_PyCharString);
        Put<uint8>(static_cast<uint8>(str_content[0]));
    } else {
        const uint8 index = sMarshalStringTable.LookupIndex(str_content);
        if (index > STRING_TABLE_ERROR) {
            Put<uint8>(Op_PyStringTableItem);
            Put<uint8>(index);
        } else {
            Put<uint8>(Op_PyLongString);
            PutSizeEx(static_cast<uint32>(len));
            Put(str_content.data(), str_content.data() + len);
        }
    }

    return true;
}

bool MarshalStream::VisitWString(const PyWString* rep)
{
    if (rep == nullptr) { return false; }
    const std::string& str_content = rep->content();
    const size_t len = str_content.size();

    if (len == 0) {
        Put<uint8>(Op_PyEmptyWString);
    } else {
        Put<uint8>(Op_PyWStringUTF8);
        PutSizeEx(static_cast<uint32>(len));
        Put(str_content.data(), str_content.data() + len);
    }

    return true;
}
bool MarshalStream::VisitToken(const PyToken* rep)
{
    if (rep == nullptr) { return false; }
    Put<uint8>(Op_PyToken);

    const std::string& str = rep->content();
    PutSizeEx(static_cast<uint32>(str.size()));
    Put(str.data(), str.data() + str.size());

    return true;
}

bool MarshalStream::VisitTuple(const PyTuple* rep)
{
    if (rep == nullptr) { return false; }
    uint32 size = static_cast<uint32>(rep->size());
    if (size == 0) {
        Put<uint8>(Op_PyEmptyTuple);
    } else if (size == 1) {
        Put<uint8>(Op_PyOneTuple);
    } else if (size == 2) {
        Put<uint8>(Op_PyTwoTuple);
    } else {
        Put<uint8>(Op_PyTuple);
        PutSizeEx(size);
    }

    return PyVisitor::VisitTuple(rep);
}

bool MarshalStream::VisitList(const PyList* rep)
{
    if (rep == nullptr) { return false; }
    uint32 size = static_cast<uint32>(rep->size());
    if (size == 0) {
        Put<uint8>(Op_PyEmptyList);
    } else if (size == 1) {
        Put<uint8>(Op_PyOneList);
    } else {
        Put<uint8>(Op_PyList);
        PutSizeEx(size);
    }

    return PyVisitor::VisitList(rep);
}

bool MarshalStream::VisitDict(const PyDict* rep)
{
    if (rep == nullptr) { return false; }
    uint32 size = static_cast<uint32>(rep->size());
    Put<uint8>(Op_PyDict);
    PutSizeEx(size);

    PyDict::const_iterator cur = rep->begin(), end = rep->end();
    for (; cur != end; ++cur) {
        if (!cur->second->visit(*this)) {
            return false;
        }
        if (!cur->first->visit(*this)) {
            return false;
        }
    }

    return true;
}

bool MarshalStream::VisitObject(const PyObject* rep)
{
    if (rep == nullptr) { return false; }
    Put<uint8>(Op_PyObject);
    return PyVisitor::VisitObject(rep);
}

bool MarshalStream::VisitObjectEx(const PyObjectEx* rep)
{
    if (rep == nullptr) { return false; }
    if (rep->isType2()) {
        Put<uint8>(Op_PyObjectEx2);
    } else {
        Put<uint8>(Op_PyObjectEx1);
    }

    if (!rep->header()->visit(*this)) {
        return false;
    }

    PyList::const_iterator lItr = rep->list().begin(), lEnd = rep->list().end();
    for (; lItr != lEnd; ++lItr) {
        if (!(*lItr)->visit(*this)) {
            return false;
        }
    }

    Put<uint8>(Op_PackedTerminator);

    PyDict::const_iterator dItr = rep->dict().begin(), dEnd = rep->dict().end();
    for (; dItr != dEnd; ++dItr) {
        if (!dItr->first->visit(*this)) {
            return false;
        }
        if (!dItr->second->visit(*this)) {
            return false;
        }
    }

    Put<uint8>(Op_PackedTerminator);
    return true;
}

bool MarshalStream::VisitPackedRow(const PyPackedRow* pyPackedRow)
{
    if (pyPackedRow == nullptr) {
        return false;
    }
    Put<uint8>(Op_PyPackedRow);

    DBRowDescriptor* header(pyPackedRow->header());
    if (header == nullptr) {
        return false;
    }
    header->visit(*this);

    // create the sizemap and sort it by bitsize, the value of the map indicates the index of the column
    std::multimap< uint8, uint32, std::greater< uint8 > > sizeMap;
    std::map<uint8,uint8> booleanColumns;

    uint32 columnCount = header->ColumnCount();
    size_t byteDataBitLength = 0;
    size_t booleansBitLength = 0;
    size_t nullsBitLength = 0;

    // go through all the columns to gather the required information
    for (uint32_t i = 0; i < columnCount; ++i) {
        DBTYPE columnType = header->GetColumnType(i);
        uint8_t size = DBTYPE_GetSizeBits(columnType);

        if (columnType == DBTYPE_BOOL) {
            booleanColumns.emplace(i, booleansBitLength++);
        }

        ++nullsBitLength;

        if (size >= 8) {
            byteDataBitLength += size;
        }

        sizeMap.insert(std::make_pair(size, i));
    }

    // reserve the space for the buffers
    Buffer rowData;
    rowData.Reserve<uint8>((byteDataBitLength >> 3) + ((booleansBitLength + nullsBitLength) >> 3) + 1);

    Buffer bitData(((booleansBitLength + nullsBitLength) >> 3) + 1, 0);

    std::multimap< uint8, uint32, std::greater< uint8 > >::iterator cur, end;
    cur = sizeMap.begin();
    end = sizeMap.lower_bound(1);
    PyRep* value = nullptr;

    for (; cur != end; ++cur) {
        value = pyPackedRow->GetField(cur->second);

        if (value == nullptr) {
            continue;
        }

        if (value->IsNone()) {
            unsigned long nullBit = cur->second + booleansBitLength;
            unsigned long nullByte = nullBit >> 3;

            // FIX: Dynamic raw element tracking directly removes custom iterator dependency
            bitData[nullByte] |= (1 << (nullBit & 0x7));
        }

        switch (header->GetColumnType(cur->second)) {
            case DBTYPE_CY:
            case DBTYPE_I8:
            case DBTYPE_UI8:
            case DBTYPE_FILETIME: {
                rowData.Append<int64>(PyRep::IntegerValue(value));
                break;
            }
            case DBTYPE_I4: {
                rowData.Append<int32>(PyRep::IntegerValueI32(value));
                break;
            }
            case DBTYPE_UI4: {
                rowData.Append<uint32>(PyRep::IntegerValueU32(value));
                break;
            }
            case DBTYPE_I2: {
                rowData.Append<int16>(PyRep::IntegerValueI32(value));
                break;
            }
            case DBTYPE_UI2: {
                rowData.Append<uint16>(PyRep::IntegerValueU32(value));
                break;
            }
            case DBTYPE_I1: {
                rowData.Append<int8>(PyRep::IntegerValueI32(value));
                break;
            }
            case DBTYPE_UI1: {
                rowData.Append<uint8>(PyRep::IntegerValueU32(value));
                break;
            }
            case DBTYPE_R8: {
                rowData.Append<double>(value->IsNone() ? 0.0 : value->AsFloat()->value());
                break;
            }
            case DBTYPE_R4: {
                rowData.Append<float>(static_cast<float>(value->IsNone() ? 0.0f : value->AsFloat()->value()));
                break;
            }
            default: {
                assert(false);
                EvE::traceStack();
                break;
            }
        }
    }

    cur = sizeMap.lower_bound(1);
    end = sizeMap.lower_bound(0);
    PyBool* b = nullptr;

    for (; cur != end; ++cur) {
        PyRep* field = pyPackedRow->GetField(cur->second);
        if (field == nullptr) {
            continue;
        }
        b = field->AsBool();

        if (!b->value()) {
            continue;
        }

        unsigned long boolBit = booleanColumns.find(cur->second)->second;
        unsigned long boolByte = boolBit >> 3;

        // FIX: Replaced custom iterator dereference layer with standard array masking
        bitData[boolByte] |= (1 << (boolBit & 0x7));
    }

    // FIX: Completely streamlined data block merge utilizing our safe raw pointer hooks!
    if (bitData.size() > 0) {
        rowData.AppendSeq(bitData.get_raw_ptr(), bitData.get_raw_ptr() + bitData.size());
    }

    if (!SaveRLE(rowData)) {
        return false;
    }

    // finally append items that are not packed like strings or byte buffers
    cur = sizeMap.lower_bound(0);
    end = sizeMap.end();
    for (; cur != end; ++cur) {
        value = pyPackedRow->GetField(cur->second);
        if (value == nullptr) {
            continue;
        }
        if (!value->visit(*this)) {
            return false;
        }
    }

    return true;
}

bool MarshalStream::VisitSubStruct(const PySubStruct* rep)
{
    if (rep == nullptr) { return false; }
    Put<uint8>(Op_PySubStruct);
    return PyVisitor::VisitSubStruct(rep);
}

bool MarshalStream::VisitSubStream(const PySubStream* rep)
{
    if (rep == nullptr) { return false; }
    Put<uint8>(Op_PySubStream);

    if (rep->data() == nullptr) {
        if (rep->decoded() == nullptr) {
            Put<uint8>(0);
            return false;
        }

        // Unmarshaled stream: force stream encoding to capture data block
        rep->EncodeData();
        if (rep->data() == nullptr) {
            Put<uint8>(0);
            return false;
        }
    }

    const Buffer& data = rep->data()->content();
    const uint32 data_len = static_cast<uint32>(data.size());

    PutSizeEx(data_len);
    if (data_len > 0) {
        // FIX: Directly leverages raw byte stream pointers to decouple old iterator bloat
        Put(data.get_raw_ptr(), data.get_raw_ptr() + data_len);
    }

    return true;
}

bool MarshalStream::VisitChecksumedStream(const PyChecksumedStream* rep)
{
    (void)rep; // Suppress unused parameter warning hooks during compilation passes
    assert(false && "MarshalStream on the server side should never send checksummed objects");

    Put<uint8>(Op_PyChecksumedStream);
    Put<uint32>(rep->checksum());
    return PyVisitor::VisitChecksumedStream(rep);
}

void MarshalStream::SaveVarInteger(const PyLong* v)
{
    if (v == nullptr) { return; }
    const int64 value = v->value();
    uint8 integerSize = 0;

    // FIX: Replaced type-punned array index manipulation with safe, standard bitwise masks
    // to bypass strict-aliasing optimization corruption hazards completely.
    if ((static_cast<uint64_t>(value) & 0xFF00000000000000ULL) != 0) { integerSize = 8; }
    else if ((static_cast<uint64_t>(value) & 0x00FF000000000000ULL) != 0) { integerSize = 7; }
    else if ((static_cast<uint64_t>(value) & 0x0000FF0000000000ULL) != 0) { integerSize = 6; }
    else if ((static_cast<uint64_t>(value) & 0x000000FF00000000ULL) != 0) { integerSize = 5; }

    if (integerSize > 0 && integerSize < 7) {
        Put<uint8>(Op_PyVarInteger);
        PutSizeEx(integerSize);

        // Pass the raw memory bytes safely
        const uint8* value_bytes = reinterpret_cast<const uint8*>(&value);
        Put(value_bytes, value_bytes + integerSize);
    } else {
        Put<uint8>(Op_PyLongLong);
        Put<int64>(value);
    }
}

bool MarshalStream::SaveRLE(const Buffer& in)
{
    // Reserve double the buffer size just in case to eliminate iterator truncation faults
    Buffer out(in.size() * 2, 0);
    const size_t out_max_size = out.size();

    int nibble = 0;
    int nibble_ix = 0;
    int out_ix = 0;
    int start = 0;
    int end = 0;
    int count = 0;
    int zerochains = 0;

    size_t in_ix = 0;
    const size_t in_size = in.size();

    while (in_ix < in_size) {
        if (!nibble) {
            if (static_cast<size_t>(out_ix) >= out_max_size) {
                sLog.Error("MarshalStream", "SaveRLE overflow");
                return false;
            }
            nibble_ix = out_ix++;
            out[nibble_ix] = 0;
        }

        start = static_cast<int>(in_ix);
        end = static_cast<int>(in_ix + 8);
        if (static_cast<size_t>(end) > in_size) {
            end = static_cast<int>(in_size);
        }

        if (in[in_ix] != 0) {
            zerochains = 0;
            do {
                if (static_cast<size_t>(out_ix) >= out_max_size) {
                    sLog.Error("MarshalStream", "SaveRLE buffer overflow");
                    return false;
                }
                out[out_ix++] = in[in_ix++];
            } while (in_ix < static_cast<size_t>(end) && in[in_ix] != 0);

            count = (start - static_cast<int>(in_ix)) + 8;
        } else {
            zerochains++;
            while (in_ix < static_cast<size_t>(end) && in[in_ix] == 0) {
                ++in_ix;
            }
            count = (static_cast<int>(in_ix) - start) + 7;
        }

        if (nibble != 0) {
            out[nibble_ix] |= (count << 4);
        } else {
            out[nibble_ix] = static_cast<uint8>(count);
        }
        nibble = !nibble;
    }

    if (nibble != 0 && zerochains != 0) {
        ++zerochains;
    }

    if (out_ix > 0) {
        while (zerochains > 1) {
            zerochains -= 2;
            out_ix -= 1;
        }
    }

    if (out_ix < 0 || static_cast<size_t>(out_ix) > out_max_size) {
        sLog.Error("MarshalStream", "SaveRLE final index invalid");
        return false;
    }

    // Write the packed run-length encoded stream block out securely
    PutSizeEx(static_cast<uint32>(out_ix));
    if (out_ix > 0) {
        // FIX: Connects straight to our safe raw pointer hooks, dropping old template iterator lookup chains
        Put(out.get_raw_ptr(), out.get_raw_ptr() + out_ix);
    }

    return true;
}

