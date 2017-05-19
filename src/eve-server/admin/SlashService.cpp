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


#include "eve-server.h"

#include "PyServiceCD.h"
#include "admin/CommandDispatcher.h"
#include "admin/SlashService.h"


PyCallable_Make_InnerDispatcher(SlashService)

SlashService::SlashService(PyServiceMgr *mgr, CommandDispatcher *cd)
: PyService(mgr, "slash"),
  m_dispatch(new Dispatcher(this)),
  m_commandDispatch(cd)
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(SlashService, SlashCmd);
}

SlashService::~SlashService() {
    delete m_dispatch;
}

PyResult SlashService::Handle_SlashCmd( PyCallArgs& call )
{
    Call_SingleWStringSoftArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: failed to decode arguments", call.client->GetName());
        return NULL;
    }

    return SlashCommand( call.client, arg.arg );
}

PyResult SlashService::SlashCommand(Client * client, std::string command)
{
    if (!(client->GetAccountRole() & ROLE_SLASH)) {
        _log( COMMAND__ERROR, "%s: Client '%s' used a slash command but does not have ROLE_SLASH. Modified client?", GetName(), client->GetName() );
        throw PyException( MakeCustomError( "You need to have ROLE_SLASH to execute commands." ) );
    }

    _log(COMMAND__MESSAGE, "%s: '%s'", client->GetName(), command.c_str() );

    return m_commandDispatch->Execute( client, command.c_str() );
}
