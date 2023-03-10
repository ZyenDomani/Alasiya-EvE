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
    Foundation; either version 2 of the License, or(at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Bloody.Rabbit
*/

#ifndef __PY_EXCEPTIONS_H__INCL__
#define __PY_EXCEPTIONS_H__INCL__

#include "python/PyRep.h"

/**
 * @brief Base class for exceptions that can be converted to python objects
 *
 * This exception can be used as a general purpose base class for any object
 * that has a PyRep representation available.
 *
 * @author Unknown? Moved here by Almamu
 */
class PyException
{
public:
    PyException( PyRep* except );
    PyException( const PyException& oth );
    ~PyException();

    PyException& operator=( const PyException& oth );

    PyRep* ssException;
};

/**
 * @brief Python object "exceptions.GPSTransportClosed".
 *
 * This exception is used when
 * refusing client login.
 *
 * @author Bloody.Rabbit
 */
class GPSTransportClosed
: public PyObjectEx_Type1
{
public:
    GPSTransportClosed( const char* reason );
    GPSTransportClosed( std::string& reason );

    /**
     * @brief Adds keyword to exception.
     *
     * @param[in] name The keyword.
     * @param[in] value Value to be associated with the keyword.
     */
    void AddKeyword( const char* name, PyRep* value );


    PyDict* GetReasonArgs() const;
protected:

    static PyTuple* _CreateArgs( const char* reason );
    static PyDict* _CreateKeywords( const char* reason );
};

/**
 * @brief Enumeration that indicates what parameters can be sent along with an UserError
 */

namespace UE {
    enum IDs {
        OwnerID                 = 2,    //UE_OWNERID = 2        ownerName
        LocationID              = 3,    //UE_LOCID = 3          locationName
        TypeID                  = 4,    //UE_TYPEID = 4         typeName
        TypeDescription         = 5,    //UE_TYPEID2 = 5        description
        BPTypeID                = 6,    //UE_BPTYPEID = 6       blueprintTypeName
        GroupID                 = 7,    //UE_GROUPID = 7        groupName
        GroupDescription        = 8,    //UE_GROUPID2 = 8       description
        CategoryID              = 9,    //UE_CATID = 9          categoryName
        CategoryDescription     = 10,   //UE_CATID2 = 10        description
        AttribID                = 11,   //UE_DGMATTR = 11
        FXID                    = 12,   //UE_DGMFX = 12
        TypeFXID                = 13,   //UE_DGMTYPEFX = 13
        DateTime                = 14,   //UE_DATETIME = 14
        Date                    = 15,   //UE_DATE = 15
        Time                    = 16,   //UE_TIME = 16
        TimeShort               = 17,   //UE_TIMESHRT = 17
        Amount                  = 18,   //UE_AMT = 18           generic amount
        ISK1                    = 19,   //UE_AMT2 = 19          ISK
        ISK2                    = 20,   //UE_AMT3 = 20          ISK
        Distance                = 21,   //UE_DIST = 21          Distance
        The                     = 22,   //UE_MSGARGS = 22
        A                       = 23,   //
        TypeAndQty              = 24,   //UE_TYPEIDANDQUANTITY = 24
        OwnerNick               = 25,   //UE_OWNERIDNICK = 25        ownerName.split(' ')[0]
        ISK                     = 28,   //UE_ISK = 28           ISK
        TypeIDList              = 29,   //UE_TYPEIDL = 29
        AUR                     = 30,   //UE_AUR = 30           AUR
        Localization            = 101,  //UE_LOC = 101
        List                    = 103   //UE_LIST = 103         List of UE_TYPEIDs
    };
}

/**
 * @brief Python object "ccp_exceptions.UserError"
 *
 * This exception can be used
 * for various errors.
 *
 * @author Bloody.Rabbit
 */
class UserError
: public PyException
{
public:
    UserError(const char* exceptionType);

    /**
     * @brief Fluent version of the protected AddKeyword, allows for adding a keyword to the exception
     *
     * Keywords are simple parameters used to replace a specific part of the message string.
     * These parameters are defined in the string itself in the form of %(name)s
     *
     * @param[in] name The parameter
     * @param[in] value The value
     *
     * @return self
     */
    UserError& AddFormatValue(const char* name, PyRep* value);

    /**
     * @brief Shorthand method for adding the given date-time as a datetime string in the message
     *
     * @param[in] name The parameter
     * @param[in] date The date in win32 epoch format
     *
     * @return self
     */
    UserError& AddDateTime(const char* name, time_t date);

    /**
     * @brief Shorthand method for adding the given date time as a date string in the message
     *
     * @param[in] name The parameter
     * @param[in] date The date in win32 epoch format
     *
     * @return self
     */
    UserError& AddDate(const char* name, time_t date);

    /**
     * @brief Shorthand method for adding the given date time as a time string in the message
     *
     * @param[in] name The parameter
     * @param[in] time The date in win32 epoch format
     *
     * @return self
     */
    UserError& AddTime(const char* name, time_t time);

    /**
     * @brief Shorthand method for adding the given time as a time string in the message(without minutes)
     *
     * Specially useful when displaying time lapses(like time left for something)
     *
     * @param[in] name The parameter
     * @param[in] time The date in win32 epoch format
     *
     * @return self
     */
    UserError& AddTimeShort(const char* name, time_t time);

    /**
     * @brief Shorthand method for adding "a" before the beginning of the value
     *
     * Specially useful for things like "you need a Shield hardener" type of messages
     *
     * @param[in] name The parameter
     * @param[in] value The value to display after "a/an"
     *
     * @return self
     */
    UserError& AddA(const char* name, const char* value);

    /**
     * @brief Shorthand method for adding "the" before the beginning of the value
     *
     * @param[in] name The parameter
     * @param[in] value The value to display after "the"
     *
     * @return self
     */
    UserError& AddThe(const char* name, const char* value);

    /**
     * @brief Shorthand method for adding a string in the client's translations
     *
     * @param[in] name The parameter
     * @param[in] strKey The name of the extra MLS string to lookup
     * @param[in] format The extra format to apply if any
     *
     * @return self
     */
    UserError& AddUELocalization(const char* name, const char* strKey, PyDict* args = nullptr);

    /**
     * @brief Shorthand method for adding a list of format parameters
     *
     * TODO: OFFER A WAY OF GENERATING listEntries AS THAT CAN ALSO HAVE FORMAT PARAMETERS LIKE THE USERERROR CLASS
     *
     * @param[in] name The parameter
     * @param[in] listEntries The list of data to concatenate
     * @param[in] separator The separator to use(if any), uses the language's default if not specified
     *
     * @return self
     */
    UserError& AddList(const char* name, PyList* listEntries, const char* separator = nullptr);

    /**
     * @brief Shorthand method for adding an owner's name
     *
     * @param[in] name The parameter
     * @param[in] ownerID The owner's ID
     *
     * @return self
     */
    UserError& AddOwnerID(const char* name, uint32 ownerID);

    /**
     * @brief Shorthand method for adding an owner's nick(first name without surname)
     *
     * @param[in] name The parameter
     * @param[in] ownerID The owner's ID
     *
     * @return self
     */
    UserError& AddOwnerNick(const char* name, uint32 ownerID);

    /**
     * @brief Shorthand method for adding a location's name
     *
     * @param[in] name The parameter
     * @param[in] locationID The location's ID
     *
     * @return self
     */
    UserError& AddLocationID(const char* name, uint32 locationID);

    /**
     * @brief Shorthand method for adding a type's name
     *
     * @param[in] name The parameter
     * @param[in] typeID The type's ID
     *
     * @return self
     */
    UserError& AddTypeID(const char* name, uint16 typeID);

    /**
     * @brief Shorthand method for adding a type's description
     *
     * @param[in] name The parameter
     * @param[in] typeID The type's ID
     *
     * @return self
     */
    UserError& AddTypeDescription(const char* name, uint16 typeID);

    /**
     * @brief Shorthand method for adding a list of types' names
     *
     * @param[in] name The parameter
     * @param[in] typeIDs The list of type ids
     *
     * @return self
     */
    UserError& AddTypeIDList(const char* name, PyList* typeIDs);

    /**
     * @brief Shorthand method for adding a blueprint's type name
     *
     * @param[in] name The parameter
     * @param[in] typeID The blueprint's type ID
     *
     * @return self
     */
    UserError& AddBlueprintTypeID(const char* name, uint16 bpTypeID);

    /**
     * @brief Shorthand method for adding a group's name
     *
     * @param[in] name The parameter
     * @param[in] typeID The group's ID
     *
     * @return self
     */
    UserError& AddGroupID(const char* name, uint16 groupID);

    /**
     * @brief Shorthand method for adding a group's description
     *
     * @param[in] name The parameter
     * @param[in] typeID The group's ID
     *
     * @return self
     */
    UserError& AddGroupDescription(const char* name, uint16 groupID);

    /**
     * @brief Shorthand method for adding a category's name
     *
     * @param[in] name The parameter
     * @param[in] typeID The category's ID
     *
     * @return self
     */
    UserError& AddCategoryID(const char* name, uint16 categoryID);

    /**
     * @brief Shorthand method for adding a category's description
     *
     * @param[in] name The parameter
     * @param[in] typeID The category's ID
     *
     * @return self
     */
    UserError& AddCategoryDescription(const char* name, uint16 categoryID);

    /**
     * @brief Shorthand method for adding a quantity value
     *
     * @param[in] name The parameter
     * @param[in] quantity The quantity for the client to format
     *
     * @return self
     */
    UserError& AddAmountI(const char* name, int value);

    /**
     * @brief Shorthand method for adding a quantity value
     *
     * @param[in] name The parameter
     * @param[in] quantity The quantity for the client to format
     *
     * @return self
     */
    UserError& AddAmountU(const char* name, uint value);

    /**
     * @brief Shorthand method for adding a quantity value
     *
     * @param[in] name The parameter
     * @param[in] amount The quantity for the client to format
     *
     * @return self
     */
    UserError& AddAmountD(const char* name, double value);

    /**
     * @brief Shorthand method for adding an ISK amount
     *
     * @param[in] name The parameter
     * @param[in] isk The ISK quantity for the client to format
     *
     * @return self
     */
    UserError& AddISK(const char* name, double isk);

    /**
     * @brief Shorthand method for adding an AUR amount
     *
     * @param[in] name The parameter
     * @param[in] aur The AUR quantity for the client to format
     *
     * @return self
     */
    UserError& AddAUR(const char* name, double aur);

    /**
     * @brief Shorthand method for adding distance in a easy to read unit
     *
     * @param[in] name The parameter
     * @param[in] distance The distance for the client to format(in meters)
     *
     * @return self
     */
    UserError& AddDistance(const char* name, double distance);

    /**
     * @brief Shorthand method for adding type ID and quantity
     *
     * @param[in] name The parameter
     * @param[in] typeID The type's ID
     * @param[in] quantity The quantity
     *
     * @return self
     */
    UserError& AddTypeIDAndQuantity(const char* name, uint16 typeID, int quantity);

protected:

    /**
     * @brief Adds keyword to exception.
     *
     * @param[in] name The keyword.
     * @param[in] value Value to be associated with the keyword.
     */
    void AddKeyword( const char* name, PyRep* value );

    /**
     * @brief Adds a keyword to exception
     *
     * Shorthand method for any of the Add* values
     *
     * @param[in] name The keyword
     * @param[in] type Type of parameter to send
     * @param[in] value The parameter's value
     * @param[in] value2 The second value(if present)
     *
     * @return self
     */
    UserError& AddParameterKeyword(const char* name, UE::IDs type, PyRep* value, PyRep* value2 = nullptr);

    PyDict* _GetTupleKeywords() const;
    PyDict* _GetDictKeywords() const;

    PyTuple* _CreateArgs( const char* msg );
    PyDict* _CreateKeywords( const char* msg );

    PyTuple* m_args;
    PyDict* m_keywords;
};
/**
 * @brief Advanced version of UserError that allows to send a full custom message
 *
 * Shorthand version of UserError with fully customizable message
 *
 * @author almamu <almamu@almamu.com>
 */
class CustomError : public UserError
{
public:
    /**
     * Initializes a CustomError with the given message
     * @param[in] message The message to send to the client
     * @param[in] ... Format arguments to the message
     */
    CustomError(const char* message, ...);
};

#endif /* !__PY_EXCEPTIONS_H__INCL__ */
