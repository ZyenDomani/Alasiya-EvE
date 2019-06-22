/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
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
*/

#include "eve-common.h"

#include "python/PyRep.h"
#include "utils/EvilNumber.h"

EvilNumber EvilZero = 0;
EvilNumber EvilOne = 1;
const EvilNumber EvilTime_Second = 10000000;
const EvilNumber EvilTime_Minute = EvilTime_Second * 60;
const EvilNumber EvilTime_Hour = EvilTime_Minute * 60;
const EvilNumber EvilTime_Day = EvilTime_Hour * 24;
const EvilNumber EvilTime_Month = EvilTime_Day * 30;
const EvilNumber EvilTime_Year = EvilTime_Month * 12 + 5;


// CONSTRUCTORS:

EvilNumber::EvilNumber() : mType(evil_number_int)
{
    mValue.iVal = 0;
}

EvilNumber::EvilNumber( int8 val ) : mType(evil_number_int)
{
    mValue.iVal = val;
}

EvilNumber::EvilNumber( uint8 val ) : mType(evil_number_int)
{
    mValue.iVal = val;
}

EvilNumber::EvilNumber( int16 val ) : mType(evil_number_int)
{
    mValue.iVal = val;
}

EvilNumber::EvilNumber( uint16 val ) : mType(evil_number_int)
{
    mValue.iVal = val;
}

EvilNumber::EvilNumber( int32 val ) : mType(evil_number_int)
{
    mValue.iVal = val;
}

EvilNumber::EvilNumber( uint32 val ) : mType(evil_number_int)
{
    mValue.iVal = val;
}

EvilNumber::EvilNumber( int64 val ) : mType(evil_number_int)
{
    mValue.iVal = val;
}

EvilNumber::EvilNumber( float val ) : mType(evil_number_float)
{
    mValue.fVal = val;
}

EvilNumber::EvilNumber( double val ) : mType(evil_number_float)
{
    mValue.fVal = val;
}


// PUBLIC FUNCTIONS:

EvilNumber EvilNumber::sin( const EvilNumber & val )
{
    EvilNumber result;

    if( val.mType == evil_number_float )
        result.mValue.fVal = std::sin( val.mValue.fVal );
    else {
        result.mValue.fVal = std::sin( (double)(val.mValue.iVal) );
    }
    result.mType = evil_number_float;

    return result;
}

EvilNumber EvilNumber::cos( const EvilNumber & val )
{
    EvilNumber result;

    if( val.mType == evil_number_float )
        result.mValue.fVal = std::cos( val.mValue.fVal );
    else {
        result.mValue.fVal = std::cos( (double)(val.mValue.iVal) );
    }
    result.mType = evil_number_float;

    return result;
}

EvilNumber EvilNumber::tan( const EvilNumber & val )
{
    EvilNumber result;

    if( val.mType == evil_number_float )
        result.mValue.fVal = std::tan( val.mValue.fVal );
    else {
        result.mValue.fVal = std::tan( (double)(val.mValue.iVal) );
    }
    result.mType = evil_number_float;

    return result;
}

EvilNumber EvilNumber::asin( const EvilNumber & val )
{
    EvilNumber result;

    if( val.mType == evil_number_float )
        result.mValue.fVal = std::asin( val.mValue.fVal );
    else {
        result.mValue.fVal = std::asin( (double)(val.mValue.iVal) );
    }
    result.mType = evil_number_float;

    return result;
}

EvilNumber EvilNumber::acos( const EvilNumber & val )
{
    EvilNumber result;

    if( val.mType == evil_number_float )
        result.mValue.fVal = std::acos( val.mValue.fVal );
    else {
        result.mValue.fVal = std::acos( (double)(val.mValue.iVal) );
    }
    result.mType = evil_number_float;

    return result;
}

EvilNumber EvilNumber::atan( const EvilNumber & val )
{
    EvilNumber result;

    if( val.mType == evil_number_float )
        result.mValue.fVal = std::atan( val.mValue.fVal );
    else {
        result.mValue.fVal = std::atan( (double)(val.mValue.iVal) );
    }
    result.mType = evil_number_float;

    return result;
}

EvilNumber EvilNumber::sqrt( const EvilNumber & val )
{
    EvilNumber result;

    if( val.mType == evil_number_float )
        result.mValue.fVal = std::sqrt( val.mValue.fVal );
    else {
        result.mValue.fVal = std::sqrt( (double)(val.mValue.iVal) );
    }
    result.mType = evil_number_float;

    return result;
}

EvilNumber EvilNumber::pow( const EvilNumber & val1, const EvilNumber & val2 )
{
    EvilNumber exponent;
    EvilNumber result;

    if( val2.mType == evil_number_float )
        exponent.mValue.fVal = val2.mValue.fVal;
    else
        exponent.mValue.fVal = (double)(val2.mValue.iVal);
    exponent.mType = evil_number_float;

    if( val1.mType == evil_number_float )
        result.mValue.fVal = std::pow( val1.mValue.fVal, exponent.mValue.fVal );
    else {
        result.mValue.fVal = std::pow( (double)(val1.mValue.iVal), exponent.mValue.fVal );
    }
    result.mType = evil_number_float;

    return result;
}

EvilNumber EvilNumber::log( const EvilNumber & val )
{
    EvilNumber result;

    if( val.mType == evil_number_float )
        result.mValue.fVal = std::log( val.mValue.fVal );
    else {
        result.mValue.fVal = std::log( (double)(val.mValue.iVal) );
    }
    result.mType = evil_number_float;

    return result;
}

EvilNumber EvilNumber::log10( const EvilNumber & val )
{
    EvilNumber result;

    if( val.mType == evil_number_float )
        result.mValue.fVal = std::log10( val.mValue.fVal );
    else {
        result.mValue.fVal = std::log10( (double)(val.mValue.iVal) );
    }
    result.mType = evil_number_float;

    return result;
}

EvilNumber EvilNumber::exp( const EvilNumber & val )
{
    EvilNumber result;

    if( val.mType == evil_number_float )
        result.mValue.fVal = std::exp( val.mValue.fVal );
    else {
        result.mValue.fVal = std::exp( (double)(val.mValue.iVal) );
    }
    result.mType = evil_number_float;

    return result;
}

PyRep* EvilNumber::GetPyObject()
{
    if (mType == evil_number_int) {
        if ( mValue.iVal > INT_MAX || mValue.iVal < INT_MIN)
            return (PyRep*)new PyLong(mValue.iVal);
        else
            return (PyRep*)new PyInt((int32)(mValue.iVal));
    } else if (mType == evil_number_float) {
        return (PyRep*)new PyFloat(mValue.fVal);
    } else {
        assert(false);
        return (PyRep*)new PyInt(0);
    }
}

inline void EvilNumber::CheckIntegrity()
{
    // check if we are a integer
    int cmp_val = (int)mValue.fVal;
    if (double(cmp_val) == mValue.fVal) {
        mValue.iVal = cmp_val;
        mType = evil_number_int;
    }
}

bool EvilNumber::isNaN()
{
    if( mType == evil_number_nan )
        return true;

    if ( mType == evil_number_int )
        return false;

    return isnan(mValue.fVal);
}

bool EvilNumber::isInf()
{
    if ( mType == evil_number_nan )
        return true;

    if ( mType == evil_number_int )
        return false;

    return isinf(mValue.fVal);
}

bool EvilNumber::isInt()
{
    if( mType == evil_number_int )
        return true;

    return false;
}

bool EvilNumber::isFloat()
{
    if( mType == evil_number_float )
        return true;

    return false;
}

bool EvilNumber::get_bool()
{
    if (mType == evil_number_float)
        return (mValue.fVal != 0.0);
    if (mType == evil_number_int)
        return (mValue.iVal != 0);
    return false;
}

int64 EvilNumber::get_int()
{
    if (mType == evil_number_float)
        return (int64)floor(mValue.fVal);
    return mValue.iVal;
}

uint32 EvilNumber::get_uint32()
{
    uint32 value = 0;
    if (mType == evil_number_float) {
        value = (uint32)floor(mValue.fVal);
    } else {
        value = (uint32)mValue.iVal;
    }
    /** @todo  this will need testing/checks for overflow... */
    while (value > INT_MAX)
        value -= INT_MAX;
    return value;
}

float EvilNumber::get_float()
{
    if (mType == evil_number_int)
        return (float)mValue.iVal;
    return (float)mValue.fVal;
}

double EvilNumber::get_double()
{
    if (mType == evil_number_int)
        return (double)mValue.iVal;
    return mValue.fVal;
}


// PRIVATE FUNCTIONS:

EvilNumber EvilNumber::_Multiply( const EvilNumber & val1, const EvilNumber & val2 )
{
    EvilNumber result;

    // WARNING!  There should be NO implicit or explicit use of the 'this' pointer here!
    if (val2.mType == val1.mType) {
        if (val1.mType == evil_number_float) {
            result.mValue.fVal = val1.mValue.fVal * val2.mValue.fVal;
            result.mType = evil_number_float;
        } else if (val1.mType == evil_number_int) {
            result.mValue.iVal = val1.mValue.iVal * val2.mValue.iVal;
            result.mType = evil_number_int;
        }
    } else {
        // we assume that the val1 argument type is the opposite of the val2 argument type
        if (val1.mType == evil_number_float) {
            result.mValue.fVal = val1.mValue.fVal * double(val2.mValue.iVal);
            result.mType = evil_number_float;
        } else if (val1.mType == evil_number_int) {
            double tVal = (double)val1.mValue.iVal; // normal integer number
            result.mValue.fVal = tVal * val2.mValue.fVal;
            result.mType = evil_number_float;
        } else {
            assert(false); // crash
        }

        // check if we are an integer
        result.CheckIntegrity();
    }
    return result;
}

EvilNumber EvilNumber::_SelfMultiply( const EvilNumber & val )
{
    if (val.mType == mType) {
        if (mType == evil_number_float) {
            this->mValue.fVal = this->mValue.fVal * val.mValue.fVal;
        } else if (mType == evil_number_int) {
            this->mValue.iVal = this->mValue.iVal * val.mValue.iVal;
        }
    } else {
        // we assume that the val argument is the opposite of the 'this' type
        if (mType == evil_number_float) {
            this->mValue.fVal = this->mValue.fVal * double(val.mValue.iVal);
        } else if (mType == evil_number_int) {
            double tVal = (double)mValue.iVal; // normal integer number
            this->mValue.fVal = tVal * val.mValue.fVal;
            mType = evil_number_float;
        } else {
            assert(false); // crash
        }

        // check if we are an integer
        CheckIntegrity();
    }
    return *this;
}

EvilNumber EvilNumber::_Divide( const EvilNumber & val1, const EvilNumber & val2 )
{
    EvilNumber result;

    // WARNING!  There should be NO implicit or explicit use of the 'this' pointer here!
    if (val2.mType == val1.mType) {
        if (val1.mType == evil_number_float) {
            result.mValue.fVal = val1.mValue.fVal / val2.mValue.fVal;
            result.mType = evil_number_float;
        } else if (val1.mType == evil_number_int) {
            // make sure we can do things like 2 / 4 = 0.5f
            result.mValue.fVal = double(val1.mValue.iVal) / double(val2.mValue.iVal);
            result.mType = evil_number_float;
            // check if its possibly an integer
            result.CheckIntegrity();
        }
    } else {
        // we assume that the val1 argument type is the opposite of the val2 argument type
        if (val1.mType == evil_number_float) {
            result.mValue.fVal = val1.mValue.fVal / double(val2.mValue.iVal);
            result.mType = evil_number_float;
        } else if (val1.mType == evil_number_int) {
            double tVal = (double)val1.mValue.iVal; // normal integer number
            result.mValue.fVal = tVal / val2.mValue.fVal;
            result.mType = evil_number_float;
        } else {
            assert(false); // crash
        }

        // check if we are an integer
        result.CheckIntegrity();
    }
    return result;
}

EvilNumber EvilNumber::_SelfDivide( const EvilNumber & val )
{
    if (val.mType == mType) {
        if (mType == evil_number_float) {
            this->mValue.fVal = this->mValue.fVal / val.mValue.fVal;
        } else if (mType == evil_number_int) {
            // make sure we can do things like 2 / 4 = 0.5f
            this->mValue.fVal = double(this->mValue.iVal) / double(val.mValue.iVal);
            mType = evil_number_float;
            // check if its possibly a integer
            CheckIntegrity();
        }
    } else {
        // we assume that the val argument is the opposite of the 'this' type
        if (mType == evil_number_float) {
            this->mValue.fVal = this->mValue.fVal / double(val.mValue.iVal);
        } else if (mType == evil_number_int) {
            double tVal = (double)mValue.iVal; // normal integer number
            this->mValue.fVal = tVal / val.mValue.fVal;
            mType = evil_number_float;
        } else {
            assert(false); // crash
        }

        // check if we are a integer
        CheckIntegrity();
    }
    return *this;
}

EvilNumber EvilNumber::_Add( const EvilNumber & val1, const EvilNumber & val2 )
{
    EvilNumber result;

    // WARNING!  There should be NO implicit or explicit use of the 'this' pointer here!
    if (val2.mType == val1.mType) {
        if (val1.mType == evil_number_float) {
            result.mValue.fVal = val1.mValue.fVal + val2.mValue.fVal;
            result.mType = evil_number_float;
        } else if (val1.mType == evil_number_int) {
            result.mValue.iVal = val1.mValue.iVal + val2.mValue.iVal;
            result.mType = evil_number_int;
        }
    } else {
        // we assume that the val argument is the opposite of the 'this' type
        if (val1.mType == evil_number_float) {
            result.mValue.fVal = val1.mValue.fVal + double(val2.mValue.iVal);
            result.mType = evil_number_float;
        } else if (val1.mType == evil_number_int) {
            double tVal = (double)val1.mValue.iVal; // normal integer number
            result.mValue.fVal = tVal + val2.mValue.fVal;
            result.mType = evil_number_float;
        } else {
            assert(false); // crash
        }

        // check if we are a integer
        result.CheckIntegrity();
    }
    return result;
}

EvilNumber EvilNumber::_SelfAdd( const EvilNumber & val )
{
    if (val.mType == mType) {
        if (mType == evil_number_float) {
            this->mValue.fVal = this->mValue.fVal + val.mValue.fVal;
        } else if (mType == evil_number_int) {
            this->mValue.iVal = this->mValue.iVal + val.mValue.iVal;
        }
    } else {
        // we assume that the val argument is the opposite of the 'this' type
        if (mType == evil_number_float) {
            this->mValue.fVal = this->mValue.fVal + double(val.mValue.iVal);
        } else if (mType == evil_number_int) {
            double tVal = (double)mValue.iVal; // normal integer number
            this->mValue.fVal = tVal + val.mValue.fVal;
            mType = evil_number_float;
        } else {
            assert(false); // crash
        }

        // check if we are a integer
        CheckIntegrity();
    }
    return *this;
}

EvilNumber EvilNumber::_Subtract( const EvilNumber & val1, const EvilNumber & val2 )
{
    EvilNumber result;

    // WARNING!  There should be NO implicit or explicit use of the 'this' pointer here!
    if (val2.mType == val1.mType) {
        if (val1.mType == evil_number_float) {
            result.mValue.fVal = val1.mValue.fVal - val2.mValue.fVal;
            result.mType = evil_number_float;
        } else if (val1.mType == evil_number_int) {
            result.mValue.iVal = val1.mValue.iVal - val2.mValue.iVal;
            result.mType = evil_number_int;
        }
    } else {
        // we assume that the val argument is the opposite of the 'this' type
        if (val1.mType == evil_number_float) {
            result.mValue.fVal = val1.mValue.fVal - double(val2.mValue.iVal);
            result.mType = evil_number_float;
        } else if (val1.mType == evil_number_int) {
            double tVal = (double)val1.mValue.iVal; // normal integer number
            result.mValue.fVal = tVal - val2.mValue.fVal;
            result.mType = evil_number_float;
        } else {
            assert(false); // crash
        }

        // check if we are a integer
        result.CheckIntegrity();
    }
    return result;
}

EvilNumber EvilNumber::_SelfSubtract( const EvilNumber & val )
{
    if (val.mType == mType) {
        if (mType == evil_number_float) {
            this->mValue.fVal = this->mValue.fVal - val.mValue.fVal;
        } else if (mType == evil_number_int) {
            this->mValue.iVal = this->mValue.iVal - val.mValue.iVal;
        }
    } else {
        // we assume that the val argument is the opposite of the 'this' type
        if (mType == evil_number_float) {
            this->mValue.fVal = this->mValue.fVal - double(val.mValue.iVal);
        } else if (mType == evil_number_int) {
            double tVal = (double)mValue.iVal; // normal integer number
            this->mValue.fVal = tVal - val.mValue.fVal;
            mType = evil_number_float;
        } else {
            assert(false); // crash
        }

        // check if we are a integer
        CheckIntegrity();
    }
    return *this;
}

EvilNumber EvilNumber::_Modulus( const EvilNumber & val1, const EvilNumber & val2 )
{
    EvilNumber result;

    // WARNING!  There should be NO implicit or explicit use of the 'this' pointer here!
    if (val2.mType == val1.mType) {
        if (val1.mType == evil_number_float) {
            result.mValue.iVal = (int64)(val1.mValue.fVal) % (int64)(val2.mValue.fVal);
            result.mType = evil_number_int;
        } else if (val1.mType == evil_number_int) {
            result.mValue.iVal = val1.mValue.iVal % val2.mValue.iVal;
            result.mType = evil_number_int;
        }
    } else {
        // we assume that the val1 argument type is the opposite of the val2 argument type
        if (val1.mType == evil_number_float) {
            result.mValue.iVal = (int64)(val1.mValue.fVal) % val2.mValue.iVal;
            result.mType = evil_number_int;
        } else if (val1.mType == evil_number_int) {
            result.mValue.iVal = val1.mValue.iVal % (int64)(val2.mValue.fVal);
            result.mType = evil_number_int;
        } else {
            assert(false); // crash
        }

        // check if we are a integer
        result.CheckIntegrity();
    }
    return result;
}

EvilNumber EvilNumber::_SelfModulus( const EvilNumber & val )
{
    if (val.mType == mType) {
        if (mType == evil_number_float) {
            this->mValue.iVal = (int64)(this->mValue.fVal) % (int64)(val.mValue.fVal);
            mType = evil_number_int;
        } else if (mType == evil_number_int) {
            this->mValue.iVal = this->mValue.iVal % val.mValue.iVal;
            mType = evil_number_int;
        }
    } else {
        // we assume that the val argument is the opposite of the 'this' type
        if (mType == evil_number_float) {
            this->mValue.iVal = (int64)(this->mValue.fVal) % val.mValue.iVal;
            mType = evil_number_int;
        } else if (mType == evil_number_int) {
            this->mValue.iVal = this->mValue.iVal % (int64)(val.mValue.fVal);
            mType = evil_number_int;
        } else {
            assert(false); // crash
        }

        // check if we are a integer
        CheckIntegrity();
    }
    return *this;
}

EvilNumber EvilNumber::_SelfIncrement()
{
    return _SelfAdd(EvilNumber(1));
}

EvilNumber EvilNumber::_SelfDecrement()
{
    return _SelfSubtract(EvilNumber(1));
}


// GLOBAL FUNCTIONS:

EvilNumber operator+(const EvilNumber& val, const EvilNumber& val2)
{
    EvilNumber result = val;
    result = result + val2;
    return result;
}

EvilNumber operator-(const EvilNumber& val, const EvilNumber& val2)
{
    EvilNumber result = val;
    result = result - val2;
    return result;
}

EvilNumber operator*(const EvilNumber& val, const EvilNumber& val2)
{
    EvilNumber result = val;
    result = result * val2;
    return result;
}

EvilNumber operator/(const EvilNumber& val, const EvilNumber& val2)
{
    EvilNumber result = val;
    result = result / val2;
    return result;
}

EvilNumber operator%(const EvilNumber& val, const EvilNumber& val2)
{
    EvilNumber result = val;
    result = result % val2;
    return result;
}
