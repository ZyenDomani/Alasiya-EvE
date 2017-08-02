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
    Author:			Allan
    Thanks to:		avianrr  for the idea
*/

#ifndef EVEMU_EVESERVER_CONSOLECOMMANDS_H_
#define EVEMU_EVESERVER_CONSOLECOMMANDS_H_

#define BUFLEN 256

#include "eve-server.h"
#include "EVEServerConfig.h"

#include "Client.h"
#include "EntityList.h"
#include "ServiceDB.h"
#include "admin/CommandDispatcher.h"
#include "chat/LSCChannel.h"
#include "inventory/ItemFactory.h"
#include "system/DestinyManager.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"


class ConsoleCommand
: public Singleton<ConsoleCommand>
{
  public:
    ConsoleCommand();
    virtual ~ConsoleCommand()   { /* do nothing here */ }

    void Initialize(CommandDispatcher* cd, ItemFactory* itmf);

	void UpdateStatus();
    void HaltServer()           { m_haltServer = true; }

    bool Process();
    bool IsShutdown()           { return m_haltServer; }

  private:
	  // we do NOT own any of these...
    LSCChannel* plscc;
	ItemFactory* pFactory;
	SystemBubble* pBubbles;
    SystemManager* pSystems;
    CommandDispatcher* pCommand;

    char* buf;
    fd_set fds;
    struct timeval tv;
    Timer m_updateTimer;
    bool m_haltServer;

    void GetUpTime(uint8 *w, uint8 *d, uint8 *h, uint8 *m, uint8 *s);
    void SendMessage(const char *msg);
    void Status(std::string *state, int64 *threads, float *vm_usage, float *resident_set, float *user, float *kernel);
    void MemStatus(float *vm_usage, float *resident_set);

    void Test();


    struct mydata {
        uint32 itemID;
        uint32 typeID;
        std::string name;
    };
};

//Singleton
#define sConsole \
    ( ConsoleCommand::get() )

#endif  // EVEMU_EVESERVER_CONSOLECOMMANDS_H_

