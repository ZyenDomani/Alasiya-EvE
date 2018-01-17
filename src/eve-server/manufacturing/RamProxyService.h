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
    Author:     Zhur
    Updates:    Allan
*/

#ifndef __RAM_PROXY_SERVICE__H__
#define __RAM_PROXY_SERVICE__H__

#include "PyService.h"
#include "manufacturing/RamProxyDB.h"


class RamProxyService : public PyService {
public:
    RamProxyService(PyServiceMgr *mgr);
    virtual ~RamProxyService();

private:
    class Dispatcher;
    Dispatcher *const m_dispatch;
    RamProxyDB m_db;

    PyCallable_DECL_CALL(GetJobs2);
    PyCallable_DECL_CALL(InstallJob);
    PyCallable_DECL_CALL(CompleteJob);
    PyCallable_DECL_CALL(AssemblyLinesGet);
    PyCallable_DECL_CALL(AssemblyLinesSelect);
    PyCallable_DECL_CALL(GetRelevantCharSkills);
    PyCallable_DECL_CALL(AssemblyLinesSelectCorp);
    PyCallable_DECL_CALL(AssemblyLinesSelectPublic);
    PyCallable_DECL_CALL(AssemblyLinesSelectPrivate);
    PyCallable_DECL_CALL(AssemblyLinesSelectAlliance);
    PyCallable_DECL_CALL(UpdateAssemblyLineConfigurations);

};

#endif
