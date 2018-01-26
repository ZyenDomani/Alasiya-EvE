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
    Author:        Zhur
*/

#include "ClientSession.h"
#include "EntityList.h"

/* Things todo or missing
    [missing]
        * atribute dep's
        * GetDefaultValueOfAttribute
        * SESSIONCHANGEDELAY = (30 * 10000000L)
*/

ClientSession::ClientSession()
: mSession(new PyDict()),
mDirty(false)
{
    /* default value of attribute */
    PyTuple* tuple = new_tuple(new PyNone(), new PyLong(Acct::Role::PLAYER | Acct::Role::NEWBIE));
    mSession->SetItemString("role", tuple);
}

ClientSession::~ClientSession()
{
    PyDecRef(mSession);
}

int64 ClientSession::CreateSessionID() {
    /*  session id is unique to each session.
     * is not saved, or shared between chars
     */
    m_sessionID = GetTimeMSeconds() * 25;
    sEntityList.RegisterSID(m_sessionID);

    return m_sessionID;
}

// note:  cannot destroy these Py* objects here.
int32 ClientSession::GetLastInt(const char* name) const
{
    PyRep* value = _GetLast(name);
    if (value == nullptr)
        return 0;

    if (!value->IsInt())
        return 0;

    return value->AsInt()->value();
}

int32 ClientSession::GetCurrentInt(const char* name) const
{
    PyRep* value = _GetCurrent(name);
    if (value == nullptr)
        return 0;

    if (!value->IsInt())
        return 0;

    return value->AsInt()->value();
}

void ClientSession::SetInt(const char* name, int32 value)
{
    _Set(name, new PyInt(value));
}

int64 ClientSession::GetLastLong(const char* name) const
{
    PyRep* value = _GetLast(name);
    if (value == nullptr)
        return 0;

    if (!value->IsLong())
        return 0;

    return value->AsLong()->value();
}

int64 ClientSession::GetCurrentLong(const char* name) const
{
    PyRep* value = _GetCurrent(name);
    if (value == nullptr)
        return 0;

    if (!value->IsLong())
        return 0;

    return value->AsLong()->value();
}

void ClientSession::SetLong(const char* name, int64 value)
{
    _Set(name, new PyLong(value));
}

std::string ClientSession::GetLastString(const char* name) const
{
    PyRep* value = _GetLast(name);
    if (value == nullptr)
        return std::string();

    if (!value->IsString())
        return std::string();

    return value->AsString()->content();
}

std::string ClientSession::GetCurrentString(const char* name) const
{
    PyRep* value = _GetCurrent(name);
    if (value == nullptr)
        return std::string();

    if (!value->IsString())
        return std::string();

    return value->AsString()->content();
}

void ClientSession::SetString(const char* name, const char* value)
{
    _Set(name, new PyString(value));
}

void ClientSession::Clear(const char* name)
{
    _Set(name, new PyNone());
}

void ClientSession::EncodeChanges(PyDict* into)
{
    PyDict::const_iterator cur = mSession->begin();
    for (; cur != mSession->end(); ++cur) {
        PyString* str = cur->first->AsString();
        PyTuple* value = cur->second->AsTuple();
        PyRep* last = value->GetItem(0);
        PyRep* current = value->GetItem(1);

        if (last->hash() != current->hash()) {
            // Duplicate tuple
            PyTuple* t = new PyTuple(2);
            t->SetItem(0, last); PyIncRef(last);
            t->SetItem(1, current); PyIncRef(current);
            into->SetItem(str, t); PyIncRef(str);
            // Update our tuple
            value->SetItem(0, current); PyIncRef(current);
        }
    }

    mDirty = false;
}

PyTuple* ClientSession::_GetValueTuple(const char* name) const
{
    PyRep* value = mSession->GetItemString(name);
    if (value == nullptr)
        return nullptr;
    return value->AsTuple();
}

PyRep* ClientSession::_GetLast(const char* name) const
{
    PyTuple* tuple = _GetValueTuple(name);
    if (tuple == nullptr) {
        _log(CLIENT__SESSION_NOTFOUND, "ClientSession::_GetLast - value not found with name '%s'", name);
        return nullptr;
    }
    return tuple->GetItem(0);
}

PyRep* ClientSession::_GetCurrent(const char* name) const
{
    PyTuple* tuple = _GetValueTuple(name);
    if (tuple == nullptr) {
        _log(CLIENT__SESSION_NOTFOUND, "ClientSession::_GetCurrent - value not found with name '%s'", name);
        return nullptr;
    }
    return tuple->GetItem(1);
}

void ClientSession::_Set(const char* name, PyRep* value)
{
    PyTuple* tuple = _GetValueTuple(name);
    if (tuple == nullptr) {
        tuple = new PyTuple(2);
        tuple->SetItem(0, new PyNone());
        tuple->SetItem(1, new PyNone());
        mSession->SetItemString(name, tuple);
    }

    PyRep* current = tuple->GetItem(1);
    if (value->hash() != current->hash()) {
        //v->SetItem(0, current); /* didn't the session need to store the old value to? */
        tuple->SetItem(1, value);

        mDirty = true;
    } else
        PyDecRef(value);
}

