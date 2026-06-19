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

#include "ClientSession.h"
#include "EntityMgr.h"
#include "EVEServerConfig.h"


ClientSession::ClientSession()
: mSession(new PyDict()),
mDirty(false),
m_sessionID(15)
{
    //  session id is unique to each session and client
    //random.getrandbits(63)
    m_sessionID *= GetTimeUSeconds();
    sEntityMgr.RegisterSID(m_sessionID);

    // set default session values
    mSession->SetItemString("role", new_tuple(PyStatic.NewNone(), new PyLong(Acct::Role::PLAYER | Acct::Role::NEWBIE), PyStatic.NewFalse()));
    mSession->SetItemString("userid", new_tuple(PyStatic.NewNone(), PyStatic.NewZero(), PyStatic.NewFalse()));
    mSession->SetItemString("address", new_tuple(PyStatic.NewNone(), new PyString("0.0.0.0"), PyStatic.NewFalse()));
    mSession->SetItemString("sessionID", new_tuple(PyStatic.NewNone(), new PyLong(m_sessionID), PyStatic.NewFalse()));
    mSession->SetItemString("languageID", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("userType", new_tuple(PyStatic.NewNone(), PyStatic.NewZero(), PyStatic.NewFalse()));
    mSession->SetItemString("user_clientid", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));

    // init remaining session values
    mSession->SetItemString("genderID", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("bloodlineID", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("raceID", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("charid", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("corpid", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("cloneStationID", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("solarsystemid2", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("constellationid", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("regionid", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("hqID", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("baseID", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("corpAccountKey", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("corprole", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("rolesAtAll", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("rolesAtBase", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("rolesAtHQ", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("rolesAtOther", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("allianceid", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("warfactionid", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("solarsystemid", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("shipid", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("stationid", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("stationid2", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("locationid", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("worldspaceid", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("fleetjob", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("fleetrole", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("fleetbooster", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("fleetid", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("wingid", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
    mSession->SetItemString("squadid", new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse()));
}

ClientSession::~ClientSession()
{
    delete mSession;
    sEntityMgr.RemoveSID(m_sessionID);
}

// note:  cannot destroy these Py* objects here.
void ClientSession::Clear(const char* name)
{
    _Set(name, PyStatic.NewNone());
}

void ClientSession::SetInt(const char* name, int32 value)
{
    _Set(name, new PyInt(value));
}

void ClientSession::SetLong(const char* name, int64 value)
{
    _Set(name, new PyLong(value));
}

void ClientSession::SetFloat(const char* name, double value)
{
    _Set(name, new PyFloat(value));
}

void ClientSession::SetString(const char* name, const char* value)
{
    _Set(name, new PyString(value));
}

int32 ClientSession::GetLastInt(const char* name) const
{
    return PyRep::IntegerValue(_GetLast(name));
}

int32 ClientSession::GetCurrentInt(const char* name) const
{
    return PyRep::IntegerValue(_GetCurrent(name));
}

int64 ClientSession::GetLastLong(const char* name) const
{
    return PyRep::IntegerValue(_GetLast(name));
}

int64 ClientSession::GetCurrentLong(const char* name) const
{
    return PyRep::IntegerValue(_GetCurrent(name));
}

double ClientSession::GetLastFloat(const char* name) const
{
    return PyRep::FloatValue(_GetLast(name));
}

double ClientSession::GetCurrentFloat(const char* name) const
{
    return PyRep::FloatValue(_GetCurrent(name));
}

std::string ClientSession::GetLastString(const char* name) const
{
    return PyRep::StringContent(_GetLast(name));
}

std::string ClientSession::GetCurrentString(const char* name) const
{
    return PyRep::StringContent(_GetCurrent(name));
}

void ClientSession::EncodeChanges(PyDict* into)
{
    if (!mDirty)
        return;

    PyDict::const_iterator cur = mSession->begin();
    for (; cur != mSession->end(); ++cur)
        if (cur->second->AsTuple()->GetItem(2)->AsBool()->value()) {    // if this value hasnt changed, dont send it.
            _GetValueTuple(PyRep::StringContent(cur->first).c_str())->SetItem(2, PyStatic.NewFalse());
            into->SetItem(cur->first->AsString(), new_tuple(cur->second->AsTuple()->GetItem(0), cur->second->AsTuple()->GetItem(1)));
        }

    mDirty = false;
}

// none of these should ever be null...  havent had any msgs because of this, but do i want to leave them here?
PyTuple* ClientSession::_GetValueTuple(const char* name) const
{
    PyRep* value = mSession->GetItemString(name);
    if (value == nullptr) {
        _log(CLIENT__SESSION_NOTFOUND, "ClientSession::_GetValueTuple - value not found with name '%s'", name);
        return nullptr;
    }
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
        if (is_log_enabled(CLIENT__SESSION_NOTFOUND)) {
            _log(CLIENT__SESSION_NOTFOUND, "ClientSession::_GetCurrent - value not found with name '%s'", name);
            EvE::traceStack();
        }
        return nullptr;
    }
    return tuple->GetItem(1);
}

void ClientSession::_Set(const char* name, PyRep* value)
{
    PyTuple* tuple = _GetValueTuple(name);
    if (tuple == nullptr) {
        // this may no longer be needed
        tuple = new_tuple(PyStatic.NewNone(), PyStatic.NewNone(), PyStatic.NewFalse());
        mSession->SetItemString(name, tuple);
    } else {
        // delete old value before replacing
        PyDecRef(tuple->GetItem(0));
    }

    PyRep* current = tuple->GetItem(1);
    if (value->hash() != current->hash()) {
        tuple->SetItem(0, current);
        tuple->SetItem(1, value);
        tuple->SetItem(2, PyStatic.NewTrue());
        mDirty = true;
    }
}
