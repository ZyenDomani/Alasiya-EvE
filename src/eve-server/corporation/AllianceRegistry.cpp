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
    Author:        Allan
*/

//work in progress
/** @note  this is a bound object!  */

#include "eve-server.h"

#include "PyServiceCD.h"
#include "corporation/AllianceRegistry.h"

PyCallable_Make_InnerDispatcher(AllianceRegistry)

AllianceRegistry::AllianceRegistry(PyServiceMgr *mgr)
: PyService(mgr, "allianceRegistry"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(AllianceRegistry, GetRankedAlliances);
    PyCallable_REG_CALL(AllianceRegistry, GetAllianceApplications);
    /*
            alliance = sm.RemoteSvc('allianceRegistry').GetAlliance(allianceID)
            */
}

AllianceRegistry::~AllianceRegistry()
{
    delete m_dispatch;
}

PyResult AllianceRegistry::Handle_GetRankedAlliances(PyCallArgs &call) {
    /*
            self.rankedAlliances.alliances = sm.RemoteSvc('allianceRegistry').GetRankedAlliances(maxLen)
            self.rankedAlliances.standings = {}
            for a in self.rankedAlliances.alliances:
                s = sm.GetService('standing').GetStanding(eve.session.corpid, a.allianceID)
                self.rankedAlliances.standings[a.allianceID] = s
         */

    sLog.White("AllianceRegistry", "Handle_GetRankedAlliances() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
}

PyResult AllianceRegistry::Handle_GetAllianceApplications(PyCallArgs &call) {

    sLog.White("AllianceRegistry", "Handle_GetAllianceApplications() size=%u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

    return nullptr;
}
