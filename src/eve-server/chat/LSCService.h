/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    Copyright 2016 - 2026 Alasiya-EvE by Allan
    For the latest implementation status visit http://eve.alasiya.net/?p=op_status
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
    Rewrite:    Allan
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
    LSCService(PyServiceMgr* mgr, CommandDispatcher* cd);
    LSCService(LSCService&&) =delete;
    LSCService(const LSCService&) =delete;
    LSCService& operator=(LSCService&&) =delete;
    LSCService& operator=(const LSCService&) =delete;
    ~LSCService();

    LSCDB* GetDB()                                      { return m_db; }

    // --- 1. SESSION LIFECYCLE MANAGEMENT ---
    void CharacterLogin(Client* pClient);
    void DestroyChannel(int32 channelID);
    void SystemUnload(int32 systemID, int32 constID, int32 regionID);

    // --- 2. DYNAMIC SYSTEM CHANNEL ENGINE ---
    void CreateSystemChannel(int32 channelID);
    LSCChannel* CreateDynamicChannel(int32 channelID, uint32 owner=0, std::string name="", std::string motd="", \
                              std::string password="", bool memberless= false, bool temporary=false );
    void CreateStaticChannel(int32 channelID, int32 ownerID=ownerSystem, LSC::Type type=LSC::Type::normal, \
                            const char* motd=nullptr, const char* displayName=nullptr, int32 gMsgID=0, int32 cMsgID=0);

    void BroadcastAlertMessage(const char* alertText);

    PyResult ExecuteCommand(Client* from, const char* msg);

    void SendServerMOTD(Client* pClient);

protected:
    class Dispatcher;
    Dispatcher* const m_dispatch;

    CommandDispatcher* const m_commandDispatch;

    LSCDB* m_db;

    PyTuple* SendError(int8 error);
    
    // Active room tracking
    std::unordered_map<int32, LSCChannel*> m_channels;
    std::unordered_map<std::string, int32> m_channelNameMap;

    PyCallable_DECL_CALL(GetChannels);
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
    PyCallable_DECL_CALL(ForgetChannel);
    PyCallable_DECL_CALL(RenameChannel);
    PyCallable_DECL_CALL(SetChannelMOTD);
    PyCallable_DECL_CALL(SetChannelLanguageRestriction);


private:
    int32 m_nextChannelID;

    void CreateStaticChannels();
    LSCChannel* GetChannelByID(int32 channelID);
    LSCChannel* GetChannelByName(const std::string& channelName);

};

#endif

/* {'FullPath': u'UI/Messages', 'messageID': 259365, 'label': u'LSCCannotAccessControlBody'}(u'{msg}', None, {u'{msg}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'msg'}})
*  {'FullPath': u'UI/Messages', 'messageID': 259367, 'label': u'LSCCannotCreateBody'}(u'{msg}', None, {u'{msg}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'msg'}})
*  {'FullPath': u'UI/Messages', 'messageID': 259369, 'label': u'LSCCannotDestroyTitle'}(u'Cannot Destroy Channel', None, None)
*  {'FullPath': u'UI/Messages', 'messageID': 259370, 'label': u'LSCCannotDestroyBody'}(u'{msg}', None, {u'{msg}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'msg'}})
*  {'FullPath': u'UI/Messages', 'messageID': 259371, 'label': u'LSCCannotJoinTitle'}(u'Cannot Join Channel', None, None)
*  {'FullPath': u'UI/Messages', 'messageID': 259372, 'label': u'LSCCannotJoinBody'}(u'{msg}', None, {u'{msg}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'msg'}})
*  {'FullPath': u'UI/Messages', 'messageID': 259374, 'label': u'LSCCannotRenameBody'}(u'{msg}', None, {u'{msg}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'msg'}})
*  {'FullPath': u'UI/Messages', 'messageID': 259376, 'label': u'LSCCannotSendMessageBody'}(u'{msg}', None, {u'{msg}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'msg'}})
*  {'FullPath': u'UI/Messages', 'messageID': 259377, 'label': u'LSCCannotSetCSPATitle'}(u'Cannot Configure CSPA', None, None)
*  {'FullPath': u'UI/Messages', 'messageID': 259378, 'label': u'LSCCannotSetCSPABody'}(u'{msg}', None, {u'{msg}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'msg'}})
*  {'FullPath': u'UI/Messages', 'messageID': 259381, 'label': u'LSCCannotSetCreatorTitle'}(u'Cannot Set Creator', None, None)
*  {'FullPath': u'UI/Messages', 'messageID': 259382, 'label': u'LSCCannotSetCreatorBody'}(u'{msg}', None, {u'{msg}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'msg'}})
*  {'FullPath': u'UI/Messages', 'messageID': 259387, 'label': u'LSCCannotSetMOTDTitle'}(u'Cannot Set Channel MOTD', None, None)
*  {'FullPath': u'UI/Messages', 'messageID': 259388, 'label': u'LSCCannotSetMOTDBody'}(u'{msg}', None, {u'{msg}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'msg'}})
*  {'FullPath': u'UI/Messages', 'messageID': 259389, 'label': u'LSCCannotSetMemberlessTitle'}(u'Cannot Configure Channel', None, None)
*  {'FullPath': u'UI/Messages', 'messageID': 259390, 'label': u'LSCCannotSetMemberlessBody'}(u'{msg}', None, {u'{msg}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'msg'}})
*  {'FullPath': u'UI/Messages', 'messageID': 259391, 'label': u'LSCCannotSetPasswordTitle'}(u'Cannot Set Channel Password', None, None)
*  {'FullPath': u'UI/Messages', 'messageID': 259392, 'label': u'LSCCannotSetPasswordBody'}(u'{msg}', None, {u'{msg}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'msg'}})
*/
