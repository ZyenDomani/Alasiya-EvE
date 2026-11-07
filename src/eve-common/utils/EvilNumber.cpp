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
*/

#include "eve-common.h"

#include "python/PyRep.h"
#include "utils/EvilNumber.h"

EvilNumber EvilZero = EvilNumber();
EvilNumber EvilZerof = EvilNumber(0.0f);
EvilNumber EvilOne = EvilNumber(1);
EvilNumber EvilNegOne = EvilNumber(-1);


EvilNumber::EvilNumber() : mType(evil_number_int),
iVal(0), fVal(0.0){}

EvilNumber::EvilNumber( int8 val ) : mType(evil_number_int),
iVal(val), fVal(0.0){}

EvilNumber::EvilNumber( uint8 val ) : mType(evil_number_int),
iVal(val), fVal(0.0){}

EvilNumber::EvilNumber( int16 val ) : mType(evil_number_int),
iVal(val), fVal(0.0){}

EvilNumber::EvilNumber( uint16 val ) : mType(evil_number_int),
iVal(val), fVal(0.0){}

EvilNumber::EvilNumber( int32 val ) : mType(evil_number_int),
iVal(val), fVal(0.0){}

EvilNumber::EvilNumber( uint32 val ) : mType(evil_number_int),
iVal(val), fVal(0.0){}

EvilNumber::EvilNumber( int64 val ) : mType(evil_number_int),
iVal(val), fVal(0.0){}

EvilNumber::EvilNumber( float val ) : mType(evil_number_float),
iVal(0), fVal(val){}

EvilNumber::EvilNumber( double val ) : mType(evil_number_float),
iVal(0), fVal(val){}

PyRep* EvilNumber::GetPyObject() {
    if (mType == evil_number_int) {
        if ((iVal > INT_MAX) or (iVal < INT_MIN)) {
            return (PyRep*)new PyLong(iVal);
        } else {
            return (PyRep*)new PyInt((int32)(iVal));
        }
    } else if (mType == evil_number_float) {
        return (PyRep*)new PyFloat(fVal);
    }

    return PyStatic.NewNone();
}
