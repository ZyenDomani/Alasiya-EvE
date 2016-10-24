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
    Updates:        Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "PyCallable.h"
#include "admin/CommandDispatcher.h"
#include "system/DestinyManager.h"

CommandDispatcher::CommandDispatcher( PyServiceMgr& services )
: m_services( services )
{
    m_commands.clear();
}

CommandDispatcher::~CommandDispatcher() {
    for (auto cur : m_commands)
        SafeDelete(cur.second);
    m_commands.clear();
}

PyResult CommandDispatcher::Execute( Client* from, const char* msg )
{
     /** @todo  fix this shit...
    if (from->IsInSpace()) {
        if (!from->DestinyMgr())
            from->SendErrorMsg( "Internal Server Error.  Ref: ServerError 31110 " );
        if (from->DestinyMgr()->IsWarping() && (!from->GetAccountRole() & ROLE_GML)) {
            sLog.Error( "CommandDispatcher", " Command Requested by %s while warping. --Access denied.", from->GetName() );
            from->SendErrorMsg( "ServerError 31113 - Cannot Request Commands While Warping." );
        }
    } */

    //might want to check for # or / at the beginning of this crap.
    Seperator sep( &msg[1] );

    if (!sep.argCount()) {
        //empty command, return list of commands
        std::string reason = "Commands: ";

        std::map<std::string, CommandRecord *>::const_iterator cur = m_commands.begin();
        reason += "[";
        for(; cur != m_commands.end(); cur++)
            reason += "'" + cur->second->command + "',";
        reason += "]";

        UserError *err = new UserError( "" );
        err->AddKeyword( "reason", new PyString( reason ) );

        throw PyException( err );
    }

    std::map<std::string, CommandRecord*>::const_iterator res = m_commands.find( sep.arg( 0 ) );
    if (m_commands.end() == res )
    {
        sLog.Error( "CommandDispatcher", "Unable to find command '%s' for %s", sep.arg( 0 ).c_str(), from->GetName() );

        throw PyException( MakeCustomError( "Unknown command '%s'", sep.arg( 0 ).c_str() ) );
    }

    CommandRecord* rec = res->second;

    sLog.Debug( "CommandDispatcher", "Request access to command '%s' with role %p for '%s' with role %p.", \
                rec->command.c_str(), rec->required_role, from->GetName(), from->GetAccountRole() );
    if (( from->GetAccountRole() & rec->required_role ) != rec->required_role )
    {
        sLog.Error( "CommandDispatcher", "Access denied to %s for command '%s'. --have role %p, need role %p",
                    from->GetName(), rec->command.c_str(), from->GetAccountRole(), rec->required_role );

        throw PyException( MakeCustomError( "Access denied to command '%s'", sep.arg( 0 ).c_str() ) );
    }

    return ( *rec->function )( from, &m_db, &m_services, sep );
}

void CommandDispatcher::AddCommand( const char* cmd, const char* desc, uint64 required_role, CommandFunc function )
{
    std::map<std::string, CommandRecord*>::iterator res = m_commands.find( cmd );
    if (res != m_commands.end())
        SafeDelete( res->second );

    m_commands[cmd] = new CommandRecord( cmd, desc, required_role, function );
}

void CommandDispatcher::ListCommands() {
    sLog.Success("  Alasiya's EvEMu", "Currently Loaded %u Commands:", m_commands.size());
    std::map<std::string, CommandDispatcher::CommandRecord*>::iterator cur = m_commands.begin();
    for (; cur != m_commands.end(); cur++) {
        sLog.Magenta("    Call and Role", "%s - %p (%" PRIu64 ")",
                     cur->first.c_str(), cur->second->required_role, cur->second->required_role);
    }
}

void CommandDispatcher::Close() {
    for (auto cur : m_commands)
        SafeDelete(cur.second);
    m_commands.clear();
}