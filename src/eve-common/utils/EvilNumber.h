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
    Author:        Captnoord, Aknor Jaden
    Updates:    Allan
    Major Rewrite to remove all sketchy/stupid code (finally)
        -Allan  6Nov26
*/

#pragma once

#define MAX_EVIL_INTEGER    0x7FFFFFFFFFFFFFFFLL      // 64-bit signed integer

#include <string>
#include <cmath>

#include "eve-compat.h"


extern EvilNumber EvilZero;
extern EvilNumber EvilZerof;
extern EvilNumber EvilOne;
extern EvilNumber EvilNegOne;

enum EVIL_NUMBER_TYPE
{
    evil_number_nan = 0,
    evil_number_int,
    evil_number_float,
};


class PyRep;

/**
 * @class EvilNumber
 *
 * @brief this is a class that kinda mimics how python polymorph's numbers.
 *
 * This is a class that kinda mimics how python polymorph's numbers.
 * VARIANT design style number class.
 *
 * @author Captnoord.
 * @date Juni 2010
 * @todo try to create a performance test for this.
 */

class EvilNumber
{
private:
    double fVal;
    int64 iVal;
    EVIL_NUMBER_TYPE mType;

public:
    // default c'tor
    EvilNumber();
    // main c'tors
    EvilNumber(int8 val);
    EvilNumber(uint8 val);
    EvilNumber(int16 val);
    EvilNumber(uint16 val);
    EvilNumber(int32 val);
    EvilNumber(uint32 val);
    EvilNumber(int64 val);
    EvilNumber(float val);
    EvilNumber(double val);
    // copy c'tor
    EvilNumber(const EvilNumber& oth) =default;
    // move c'tor
    EvilNumber(EvilNumber&& oth) =default;
    // copy assignment
    EvilNumber& operator=(const EvilNumber& oth) =default;
    // move assignment
    EvilNumber& operator=(EvilNumber&& oth) =default;
    // d'tor
    ~EvilNumber() =default;

    // Pre-increment (++x)
    EvilNumber& operator++() {
        if (mType == evil_number_int) ++iVal;
        else if (mType == evil_number_float) ++fVal;
        return *this;
    }

    // Pre-decrement (--x)
    EvilNumber& operator--() {
        if (mType == evil_number_int) --iVal;
        else if (mType == evil_number_float) --fVal;
        return *this;
    }
    // Post-increment (x++)
    EvilNumber operator++(int) {
        EvilNumber temp = *this;
        ++(*this);
        return temp;
    }

    // Post-decrement (x--)
    EvilNumber operator--(int) {
        EvilNumber temp = *this;
        --(*this);
        return temp;
    }
    
    EvilNumber& operator+=(const EvilNumber& val) {
        if (mType == evil_number_float) {
            fVal += (val.mType == evil_number_float) ? val.fVal : static_cast<double>(val.iVal);
        } else if (mType == evil_number_int && val.mType == evil_number_int) {
            iVal += val.iVal;
        } else {
            fVal = static_cast<double>(iVal) + val.fVal;
            mType = evil_number_float;
        }
        return *this;
    }

    EvilNumber& operator-=(const EvilNumber& val) {
        if (mType == evil_number_float) {
            fVal -= (val.mType == evil_number_float) ? val.fVal : static_cast<double>(val.iVal);
        } else if (mType == evil_number_int && val.mType == evil_number_int) {
            iVal -= val.iVal;
        } else {
            fVal = static_cast<double>(iVal) - val.fVal;
            mType = evil_number_float;
        }
        return *this;
    }

    EvilNumber& operator*=(const EvilNumber& val) {
        if (mType == evil_number_float) {
            fVal *= (val.mType == evil_number_float) ? val.fVal : static_cast<double>(val.iVal);
        } else if (mType == evil_number_int && val.mType == evil_number_int) {
            iVal *= val.iVal;
        } else {
            fVal = static_cast<double>(iVal) * val.fVal;
            mType = evil_number_float;
        }
        return *this;
    }

    EvilNumber& operator/=(const EvilNumber& val) {
        double denominator = (val.mType == evil_number_float) ? val.fVal : static_cast<double>(val.iVal);
        if (mType == evil_number_float) {
            fVal /= denominator;
        } else {
            fVal = static_cast<double>(iVal) / denominator;
            mType = evil_number_float;
        }
        return *this;
    }

    EvilNumber& operator%=(const EvilNumber& val) {
        int64 left = (mType == evil_number_float) ? static_cast<int64>(fVal) : iVal;
        int64 right = (val.mType == evil_number_float) ? static_cast<int64>(val.fVal) : val.iVal;
        iVal = left % right;
        mType = evil_number_int;
        return *this;
    }

    // convert the EvilNumber into a python object.
    PyRep* GetPyObject();

    EVIL_NUMBER_TYPE get_type() const                   { return mType; }

    bool isInt() const                                  { return ( mType == evil_number_int ); }
    bool isFloat() const                                { return ( mType == evil_number_float ); }
    bool isNaN() const;
    bool isInf() const;

    bool get_bool() const;
    int8 get_int8() const;
    int16 get_int16() const;
    int32 get_int() const;
    int64 get_long() const;
    uint32 get_uint32() const;
    float get_float() const;
    double get_double() const;

    // Unary minus remains a normal member function
    EvilNumber operator-() const {
        EvilNumber result = *this;
        if (result.mType == evil_number_float) result.fVal = -result.fVal;
        else if (result.mType == evil_number_int) result.iVal = -result.iVal;
        return result;
    }

    friend inline bool operator<(const EvilNumber& lhs, const EvilNumber& rhs) {
        if (lhs.mType == evil_number_int && rhs.mType == evil_number_int) return lhs.iVal < rhs.iVal;
        double lVal = (lhs.mType == evil_number_float) ? lhs.fVal : static_cast<double>(lhs.iVal);
        double rVal = (rhs.mType == evil_number_float) ? rhs.fVal : static_cast<double>(rhs.iVal);
        return lVal < rVal;
    }

    friend inline bool operator<=(const EvilNumber& lhs, const EvilNumber& rhs) {
        if (lhs.mType == evil_number_int && rhs.mType == evil_number_int) return lhs.iVal <= rhs.iVal;
        double lVal = (lhs.mType == evil_number_float) ? lhs.fVal : static_cast<double>(lhs.iVal);
        double rVal = (rhs.mType == evil_number_float) ? rhs.fVal : static_cast<double>(rhs.iVal);
        return lVal <= rVal;
    }

    friend inline bool operator>(const EvilNumber& lhs, const EvilNumber& rhs) {
        if (lhs.mType == evil_number_int && rhs.mType == evil_number_int) return lhs.iVal > rhs.iVal;
        double lVal = (lhs.mType == evil_number_float) ? lhs.fVal : static_cast<double>(lhs.iVal);
        double rVal = (rhs.mType == evil_number_float) ? rhs.fVal : static_cast<double>(rhs.iVal);
        return lVal > rVal;
    }

    friend inline bool operator>=(const EvilNumber& lhs, const EvilNumber& rhs) {
        if (lhs.mType == evil_number_int && rhs.mType == evil_number_int) return lhs.iVal >= rhs.iVal;
        double lVal = (lhs.mType == evil_number_float) ? lhs.fVal : static_cast<double>(lhs.iVal);
        double rVal = (rhs.mType == evil_number_float) ? rhs.fVal : static_cast<double>(rhs.iVal);
        return lVal >= rVal;
    }

    friend inline bool operator==(const EvilNumber& lhs, const EvilNumber& rhs) {
        if (lhs.mType == evil_number_int && rhs.mType == evil_number_int) return lhs.iVal == rhs.iVal;
        double lVal = (lhs.mType == evil_number_float) ? lhs.fVal : static_cast<double>(lhs.iVal);
        double rVal = (rhs.mType == evil_number_float) ? rhs.fVal : static_cast<double>(rhs.iVal);
        return lVal == rVal;
    }

    friend inline bool operator!=(const EvilNumber& lhs, const EvilNumber& rhs) {
        return !(lhs == rhs);
    }

    // CHANGE: Make these friend non-member functions inside the class block
    friend inline EvilNumber operator+(EvilNumber lhs, const EvilNumber& rhs) {
        lhs += rhs;
        return lhs;
    }

    friend inline EvilNumber operator-(EvilNumber lhs, const EvilNumber& rhs) {
        lhs -= rhs;
        return lhs;
    }

    friend inline EvilNumber operator*(EvilNumber lhs, const EvilNumber& rhs) {
        lhs *= rhs;
        return lhs;
    }

    friend inline EvilNumber operator/(EvilNumber lhs, const EvilNumber& rhs) {
        lhs /= rhs;
        return lhs;
    }

    friend inline EvilNumber operator%(EvilNumber lhs, const EvilNumber& rhs) {
        lhs %= rhs;
        return lhs;
    }

    std::string to_str() {
        if (mType == evil_number_int) {
            return std::to_string(iVal);
        } else if (mType == evil_number_float) {
            return std::to_string(fVal);
        }
        assert(false);
        return "0"; // Safe fallback if NDEBUG is defined in release builds
    }

};

inline bool EvilNumber::isNaN() const {
    if ( mType == evil_number_nan )
        return true;
    if ( mType == evil_number_int )
        return false;
    return std::isnan(fVal);
}

inline bool EvilNumber::isInf() const {
    if ( mType == evil_number_nan )
        return true;
    if ( mType == evil_number_int )
        return false;
    return std::isinf(fVal);
}

inline bool EvilNumber::get_bool() const {
    if (mType == evil_number_float)
        return (fVal != 0.0);
    if (mType == evil_number_int)
        return (iVal != 0);
    return false;
}

inline int64 EvilNumber::get_long() const {
    if (mType == evil_number_float)
        return static_cast<int64>(std::floor(fVal));
    return iVal;
}

inline int8 EvilNumber::get_int8() const {
    if (mType == evil_number_float)
        return static_cast<int8>(std::floor(fVal));
    return static_cast<int8>(iVal);
}

inline int16 EvilNumber::get_int16() const {
    if (mType == evil_number_float)
        return static_cast<int16>(std::floor(fVal));
    return static_cast<int16>(iVal);
}

inline int32 EvilNumber::get_int() const {
    if (mType == evil_number_float)
        return static_cast<int32>(std::floor(fVal));
    return static_cast<int32>(iVal);
}

inline uint32 EvilNumber::get_uint32() const {
    if (mType == evil_number_float)
        return static_cast<uint32>(std::floor(fVal));
    return static_cast<uint32>(iVal);
}

inline float EvilNumber::get_float() const {
    if (mType == evil_number_int)
        return static_cast<float>(iVal);
    return static_cast<float>(fVal);
}

inline double EvilNumber::get_double() const {
    if (mType == evil_number_int)
        return static_cast<double>(iVal);
    return fVal;
}
