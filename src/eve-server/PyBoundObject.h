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

#ifndef __PYBOUNDOBJECT_H_INCL__
#define __PYBOUNDOBJECT_H_INCL__

#include "PyCallable.h"

class PyBoundObject
: public PyCallable {
public:
    PyBoundObject(PyServiceMgr *mgr);
    virtual ~PyBoundObject();

    uint32 nodeID() const                               { return m_nodeID; }
    uint32 bindID() const                               { return m_bindID; }

    //returns string "N=(nodeID):(bindID)"
    std::string GetBindStr() const;
    const char* GetName() const                         { return m_strBoundObjectName.c_str(); };

    //just to say who we are:
    virtual PyResult Call(const std::string &method, PyCallArgs &args);

    /** @returns BoundID The id of the bound service */
    //BoundID GetBoundID() const  { return this->mBoundId; }

    void NewReference (Client* client) {
        // ensure the client is not there yet
        auto it = this->mClients.find (client);

        if (it != this->mClients.end ())
            return;

        // the client didn't hold a reference to this service
        // so add it to the list and increase the RefCount
        mClients.emplace(client, true);
        // also add it to the bind list of the client
        //client->AddBindID(GetBoundID());
    }

    PyTuple* GetOID() const                             { return mOID; }

protected:
    friend class PyServiceMgr;    //for access to _SetNodeBindID only.
    void _SetNodeBindID(uint32 nodeID, uint32 bindID)   { m_nodeID = nodeID; m_bindID = bindID; }

    PyServiceMgr *const m_manager;
    std::string m_strBoundObjectName;

private:
    uint32 m_nodeID;
    uint32 m_bindID;

    PyTuple* mOID;

    std::map <Client*, bool> mClients;
};

#endif
