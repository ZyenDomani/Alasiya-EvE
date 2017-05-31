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
    Author:     Zhur, Aknor Jaden
*/


#ifndef __LSC_SERVICE_H_INCL__
#define __LSC_SERVICE_H_INCL__

#include <vector>

#include "chat/LSCDB.h"
#include "chat/LSCChannel.h"
#include "PyService.h"


class CommandDispatcher;

class LSCService : public PyService
{
public:
    // All user-created chat channels are created with IDs that are in this set:
    //     [baseChannelID,maxChannelID]  (note the inclusivity in that set)
    static const int32 BASE_CHANNEL_ID;
    static const uint32 MAX_CHANNEL_ID;

    LSCService(PyServiceMgr *mgr, CommandDispatcher *cd);
    virtual ~LSCService();

    PyResult ExecuteCommand(Client *from, const char *msg);
    void CharacterLogin(Client *pClient);

    void CreateSystemChannel(int32 channelID);
    void SystemUnload(uint32 systemID, uint32 constID, uint32 regionID);

    void SendMail(uint32 sender, uint32 recipient, const std::string &subject, const std::string &content) {
        std::vector<int32> recs(1, recipient);
        SendMail(sender, recs, subject, content);
    }
    void SendMail(uint32 sender, const std::vector<int32> &recipients, const std::string &subject, const std::string &content);

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    CommandDispatcher *const m_commandDispatch;

    LSCDB* m_db;

    std::map<int32, LSCChannel*> m_channels;  //we own these pointers

    PyCallable_DECL_CALL(GetChannels);
    PyCallable_DECL_CALL(GetRookieHelpChannel);
    PyCallable_DECL_CALL(JoinChannels);
    PyCallable_DECL_CALL(LeaveChannels);
    PyCallable_DECL_CALL(LeaveChannel);
    PyCallable_DECL_CALL(CreateChannel);
    PyCallable_DECL_CALL(Configure);
    PyCallable_DECL_CALL(DestroyChannel);
    PyCallable_DECL_CALL(GetMembers);
    PyCallable_DECL_CALL(GetMember);
    PyCallable_DECL_CALL(SendMessage);
    PyCallable_DECL_CALL(Invite);
    PyCallable_DECL_CALL(AccessControl);

    PyCallable_DECL_CALL(GetMyMessages);
    PyCallable_DECL_CALL(GetMessageDetails);
    PyCallable_DECL_CALL(Page);
    PyCallable_DECL_CALL(MarkMessagesRead);
    PyCallable_DECL_CALL(DeleteMessages);

private:
    void CreateStaticChannels();
    LSCChannel *GetChannelByID(int32 channelID);
    LSCChannel *GetChannelByName(std::string  channelName);
    LSCChannel *CreateChannel(int32 channelID, uint32 ownerID, const char* name, std::string motd, const char* password, const char* compkey,
                              LSC::Type type = LSC::Type::normal, uint32 cspa = 0, int32 groupMessageID = 0, int32 channelMessageID = 0,
                              bool memberless = false, bool maillist = false, bool temporary = false, bool languageRestriction = false);

};

#endif
