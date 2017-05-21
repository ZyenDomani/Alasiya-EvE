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


#include "eve-server.h"

#include "PyServiceCD.h"
#include "admin/CommandDispatcher.h"
#include "admin/SlashService.h"
#include "chat/LSCService.h"

// Set the base (minimum) and maximum numbers for any user-created chat channel.
const int32 LSCService::BASE_CHANNEL_ID = 2100000000;      //trial accts are spam-restricted to 1m input buffer when channelID < 2100000000
const uint32 LSCService::MAX_CHANNEL_ID = 0xFFFFFFFF;

PyCallable_Make_InnerDispatcher(LSCService)

LSCService::LSCService(PyServiceMgr *mgr, CommandDispatcher* cd)
: PyService(mgr, "LSC"),
  m_dispatch(new Dispatcher(this)),
  m_commandDispatch(cd)
{
    _SetCallDispatcher(m_dispatch);

    //make sure you edit the header file too
    PyCallable_REG_CALL(LSCService, GetChannels);
    PyCallable_REG_CALL(LSCService, GetRookieHelpChannel);
    PyCallable_REG_CALL(LSCService, JoinChannels);
    PyCallable_REG_CALL(LSCService, LeaveChannels);
    PyCallable_REG_CALL(LSCService, LeaveChannel);
    PyCallable_REG_CALL(LSCService, CreateChannel);
    PyCallable_REG_CALL(LSCService, Configure);
    PyCallable_REG_CALL(LSCService, DestroyChannel);
    PyCallable_REG_CALL(LSCService, GetMembers);
    PyCallable_REG_CALL(LSCService, GetMember);
    PyCallable_REG_CALL(LSCService, SendMessage);
    PyCallable_REG_CALL(LSCService, Invite);
    PyCallable_REG_CALL(LSCService, AccessControl);

    PyCallable_REG_CALL(LSCService, GetMyMessages);
    PyCallable_REG_CALL(LSCService, GetMessageDetails);
    PyCallable_REG_CALL(LSCService, Page);
    PyCallable_REG_CALL(LSCService, MarkMessagesRead);
    PyCallable_REG_CALL(LSCService, DeleteMessages);

	// sm.RemoteSvc('LSC').VoiceStatus(eveChannelID, 2)  gag
	// sm.RemoteSvc('LSC').VoiceStatus(eveChannelID, 1)  ungag
	// sm.RemoteSvc('LSC').VoiceStatus(eveChannelName, 0) leave channel
    m_db = new LSCDB();

    CreateStaticChannels();
}


LSCService::~LSCService() {
    delete m_dispatch;
    std::map<int32, LSCChannel* >::iterator cur = m_channels.begin();
    for(; cur != m_channels.end(); cur++) {
        SafeDelete(cur->second);
    }
    SafeDelete(m_db);
}

/*
LSC__ERROR=1
LSC__WARNING=0
LSC__MESSAGE=0
LSC__INFO=0
LSC__CHANNELS=0
LSC__CALL_DUMP=0
LSC__RSP_DUMP=0
*/

//  there is no reason to hit the db for everyfuckingthing in this class!!

///////////////////////////////////////////////////////////////////////////////
//
// Eve Chat calls
//
///////////////////////////////////////////////////////////////////////////////

const int cspa = 2950; // CONCORD Spam Prevention Act


PyResult LSCService::Handle_GetChannels(PyCallArgs &call)
{
    ChannelInfo info;
    info.lines = new PyList();

    std::map<int32, LSCChannel*>::iterator cur, end;
    cur = m_channels.begin();
    end = m_channels.end();
    for (; cur != end; cur++) {
        if ((cur->first < 0) or (cur->first > maxStaticChannel))
            continue;
        info.lines->AddItem(cur->second->EncodeDynamicChannel(call.client->GetCharacterID()));
    }

    return info.Encode();
}


PyResult LSCService::Handle_GetRookieHelpChannel(PyCallArgs &call) {
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White("LSCService::Handle_GetRookieHelpChannel()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    return new PyInt(1);
}


PyResult LSCService::Handle_JoinChannels(PyCallArgs &call) {
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White("LSCService::Handle_JoinChannels()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    CallJoinChannels args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    std::set<int32> toJoin;

    PyList::const_iterator cur, end;
    cur = args.channels->begin();
    end = args.channels->end();
    for (; cur != end; cur++) {
        if ((*cur)->IsInt())
            toJoin.insert((*cur)->AsInt()->value());
        else if ((*cur)->IsTuple()) {
            PyTuple* prt = (*cur)->AsTuple();
            if (prt->items.size() != 1 or !prt->items[0]->IsTuple()) {
                codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
                continue;
            }
            prt = prt->items[0]->AsTuple();

            if (prt->items.size() != 2 or /* !prt->items[0]->IsString() or unnessecary */ !prt->items[1]->IsInt()) {
                codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
                continue;
            }
            toJoin.insert(prt->items[1]->AsInt()->value());
        } else {
            codelog(SERVICE__ERROR, "%s: Bad argument ", call.client->GetName());
            return nullptr;
        }
    }

    uint32 charID = call.client->GetCharacterID();

    PyList *ml = new PyList();

    const bool isRookie = Win32TimeNow() < (call.client->GetChar()->createDateTime() + Win32Time_Month);

    std::set<int32>::iterator curs, ends;
    curs = toJoin.begin();
    ends = toJoin.end();
    for (; curs != ends; curs++) {
        LSCChannel* channel(nullptr);
        int32 channelID = *curs;

        // Skip joining Help\Rookie and Help\Help channels when the character is no longer a rookie:
        if (isRookie or (channelID > 2)) {
            channel = CreateChannel(channelID);
            if (!channel)
                continue;
            ChannelJoinReply chjr;
                chjr.ChannelID = channel->EncodeID();

                /** @todo  query/check password and other stipulations */
            if ((!channel->IsJoined(charID)) and (channelID != (int32)call.client->GetCharacterID())) {
                if (channel->JoinChannel(call.client)) {
                    ChannelJoinOK cjok;
                   // if ((channelID < 0) or (channelID > maxStaticChannel))
                        cjok.ChannelInfo = channel->EncodeDynamicChannel(charID);
                   // else
                   //     cjok.ChannelInfo = channel->EncodeStaticChannel(charID);
                    cjok.ChannelMods = channel->EncodeChannelMods();
                    cjok.ChannelChars = channel->EncodeChannelChars();
                    chjr.JoinRsp = cjok.Encode();
                    chjr.ok = 1;
                } else {
                    ChannelJoinNotOK cjnok;
                        cjnok.Error = "LSCCannotJoin";
                        cjnok.rspDict = new PyDict();   // dunno what goes here...
                    chjr.JoinRsp = cjnok.Encode();
                    chjr.ok = 0;
                }
            } else {
                ChannelJoinNotOK cjnok;
                    cjnok.Error = "LSCChannelIsJoined";
                    cjnok.rspDict = new PyDict();   // dunno what goes here...
                chjr.JoinRsp = cjnok.Encode();
                chjr.ok = 0;
            }
            ml->AddItem(chjr.Encode());
        }
    }

    if (is_log_enabled(LSC__RSP_DUMP))
        ml->Dump(LSC__RSP_DUMP, "   ");
/*
            ret = sm.RemoteSvc('LSC').JoinChannels(toJoin, eve.session.role)
            argsList = []
            for channelID, ok, tmp in ret:
                if ok:
                    info, acl, memberList = tmp
                else:
            {error} msg, dict = tmp      //dunno what the dict contains
                if msg in ('LSCCannotJoin', 'LSCWrongPassword') ...

                    LSCCannotDestroy
                    LSCConfirmDestroyChannel
                    LSCCannotSendMessage
                    LSCChannelIsJoined
                */
    return ml;
}

PyResult LSCService::Handle_CreateChannel(PyCallArgs& call)
{
    /** @todo  update this with error msgs where needed .. see end of method */
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White("LSCService::Handle_CreateChannel()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    Call_SingleWStringSoftArg name;
    if (!name.Decode(call.tuple))  {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    bool create = false, temporary = false, joinExisting = false, noCallThrottling = false, memberless = false;
    if (call.byname.find("create") != call.byname.end())
        create =  call.byname.find("create")->second->AsBool()->value();
    if (call.byname.find("temporary") != call.byname.end())
        temporary = call.byname.find("temporary")->second->AsBool()->value();
    if (call.byname.find("joinExisting") != call.byname.end())
        joinExisting = call.byname.find("joinExisting")->second->AsBool()->value();
    if (call.byname.find("noCallThrottling") != call.byname.end())
        noCallThrottling = call.byname.find("noCallThrottling")->second->AsBool()->value();
    if (call.byname.find("memberless") != call.byname.end())
        memberless = (call.byname.find("memberless")->second->AsInt()->value() ? true : false);

    Client* pClient = call.client;
    ChannelCreateReply reply;
    LSCChannel* channel(nullptr);

    if (create)  {
        // Query Database to see if a channel with this name does not exist and, if so, create the channel,
        // otherwise, set the channel pointer to NULL
        if (m_db->IsChannelNameAvailable(name.arg))
            channel = CreateChannel(name.arg.c_str());
        else {
            _log(LSC__ERROR, "%s: Error creating new chat channel: channel name '%s' already exists.", pClient->GetName(), name.arg.c_str());
            return nullptr;
        }

        // Save channel info and channel subscription to the database
        m_db->UpdateChannelInfo(channel);
        m_db->WriteNewChannelSubscriptionToDatabase(pClient->GetCharacterID(), channel->GetChannelID(), pClient->GetCorporationID(),
                                                     pClient->GetAllianceID(), pClient->GetAccountRole(), 0);

        if (channel->JoinChannel(pClient)) {
           // if ((channel->GetChannelID() < 0) or (channel->GetChannelID() > maxStaticChannel))
                reply.ChannelInfo = channel->EncodeDynamicChannel(pClient->GetCharacterID());
           // else
           //     reply.ChannelInfo = channel->EncodeStaticChannel(pClient->GetCharacterID());
            reply.ChannelChars = channel->EncodeChannelChars();
            reply.ChannelMods = channel->EncodeChannelMods();
        } else {
            reply.ChannelInfo = new PyInt(LSC::Error::errUnspecified);
        }
        return reply.Encode();
    }

    if (joinExisting) {
        if (!(m_db->IsChannelNameAvailable(name.arg))) {
            // Channel exists, so get its info from database and create this channel in the cache:
            std::string ch_name = "", ch_motd = "", ch_compkey = "", ch_password = "";
            int32 ch_ID = 0;
            uint32 ch_ownerID = 0, ch_cspa = 0, ch_temp = 0;
            bool ch_memberless = false, ch_maillist = false;
            LSC::Type ch_type = LSC::Type::normal;

            m_db->GetChannelInformation(name.arg,ch_ID,ch_motd,ch_ownerID,ch_compkey,ch_memberless,ch_password,ch_maillist,ch_cspa,ch_temp);

            channel = CreateChannel(ch_ID, name.arg.c_str(), ch_motd.c_str(), ch_type, ch_compkey.c_str(), ch_ownerID,
                ch_memberless, ch_password.c_str(), ch_maillist, ch_cspa, (ch_temp ? true : false), false, 0,0);

            if (!channel) {
                _log(LSC__ERROR, "%s: Error creating new chat channel", pClient->GetName());
                reply.ChannelInfo = new PyInt(LSC::Error::errNoSuchChannel);
                return reply.Encode();
            }
        } else {
            pClient->SendErrorMsg("Unable to join channel '%s'. Channel does not exist.", name.arg.c_str());
            reply.ChannelInfo = new PyInt(LSC::Error::errChannelExists);
            return reply.Encode();
        }
    }

    if (temporary) {
        int32 channel_id = m_db->GetNextAvailableChannelID();
        channel = CreateChannel(channel_id, name.arg.c_str(), "", LSC::Type::custom, "", pClient->GetCharacterID(),
                                memberless, "", false, cspa, temporary, false, 0, 0);

        if (!channel) {
            _log(LSC__ERROR, "%s: Error creating new Temporary chat channel", pClient->GetName());
            reply.ChannelInfo = new PyInt(LSC::Error::errUnspecified);
            return reply.Encode();
        }

        m_db->UpdateChannelInfo(channel);
    }

    if (!channel->IsJoined(pClient->GetCharacterID()))  {
        if (!temporary)
            m_db->WriteNewChannelSubscriptionToDatabase(pClient->GetCharacterID(), channel->GetChannelID(), pClient->GetCorporationID(),
                                                         pClient->GetAllianceID(), pClient->GetAccountRole(), 0);

        if (channel->JoinChannel(pClient)) {
           // if ((channel->GetChannelID() < 0) or (channel->GetChannelID() > maxStaticChannel))
                reply.ChannelInfo = channel->EncodeDynamicChannel(pClient->GetCharacterID());
           // else
           //     reply.ChannelInfo = channel->EncodeStaticChannel(pClient->GetCharacterID());
            reply.ChannelChars = channel->EncodeChannelChars();
            reply.ChannelMods = channel->EncodeChannelMods();
        } else {
            reply.ChannelInfo = new PyInt(LSC::Error::errUnspecified);
        }
        return reply.Encode();
    } else {
        _log(LSC__ERROR, "%s: Already joined Channel %i \"%s\".", pClient->GetName(), channel->GetChannelID(), channel->GetDisplayName().c_str());
        reply.ChannelInfo = new PyInt(LSC::Error::errUnspecified);
        return reply.Encode();
    }

    /*
            ret = sm.RemoteSvc('LSC').CreateChannel(displayName, joinExisting=False, memberless=0, create=True)
            if ret:
                info, acl, memberList = ret
        // on fail,  ChannelInfo is pyint(lsc::type::error), others are null
            if info == CHTERR_ALREADYEXISTS:
            if info == CHTERR_NOSUCHCHANNEL:
                */
}

PyResult LSCService::Handle_SendMessage(PyCallArgs& call)
{
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White("LSCService::Handle_SendMessage()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    int32 channel_id = 0;
    std::string message = "";

    if ((call.tuple->IsTuple()) and (call.tuple->AsTuple()->items[0]->IsInt())) {
        // Decode All User-created chat channel messages here:
        if (!call.tuple->IsTuple()) {
            _log(NET__PACKET_ERROR, "LSCService::Handle_SendMessage failed: tuple0 is the wrong type: %s", call.tuple->TypeString());
            return new PyNone();
        }
        PyTuple* tuple0 = call.tuple->AsTuple();

        if (tuple0->size() != 2) {
            _log(NET__PACKET_ERROR, "LSCService::Handle_SendMessage failed: tuple0 is the wrong size: expected 2, but got %lu", tuple0->size());
            return new PyNone();
        }

        channel_id = (call.tuple->AsTuple()->items[0]->AsInt())->value();
        message = ((call.tuple->AsTuple()->items[1]->AsWString())->content());
        sLog.White("LSCService", "Handle_SendMessage: call is either User-created chat message or bad packet.");
    } else {
        Call_SendMessage args;
        if (!args.Decode(call.tuple)) {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
            return new PyNone();
        }
        channel_id = args.channel.id;
        message = args.message;
        _log(LSC__INFO, "Handle_SendMessage: call is Corp/Local/Region/Constellation chat.");
    }

    std::map<int32, LSCChannel*>::iterator itr = m_channels.find(channel_id);
    if (itr == m_channels.end()) {
        _log(LSC__ERROR, "%s: Couldn't find channel %u", call.client->GetName(), channel_id);
        return new PyNone();
    }

    std::string CIC_test_name = "CIC - " + std::string(call.client->GetName());
    if ((message.substr(0,3) == "pcs") and (itr->second->GetDisplayName() == CIC_test_name)) {
        _log(LSC__INFO, "CALL to Player Command System via LSC Service");
        // call to Player Command System to parse command
        //if (command_ack == 1)
        message = "[ COMMAND ACKNOWLEDGED ]";
        //else
        //  message = "[ COMMAND FAILED ]";
    }

    if (message == "cic")
        _log(LSC__INFO, "Message 'cic' received, creating/joining %s...", CIC_test_name.c_str());

    if (message.at(0) == '.') {
        _log(LSC__INFO, "CALL to SlashService->SlashCmd() via LSC Service");
        static_cast<SlashService *>(m_manager->LookupService("slash"))->SlashCommand(call.client, message);
        return new PyNone();
    }

    itr->second->SendMessage(call.client, message.c_str());

    return new PyNone();
}

PyResult LSCService::Handle_AccessControl(PyCallArgs& call)
{
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White("LSCService::Handle_AccessControl()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }
    /*
     * 20:43:44 W LSCService::Handle_AccessControl(): size=3
     * 20:43:44 [LSC_CDump]   Call Arguments:
     * 20:43:44 [LSC_CDump]       Tuple: 3 elements
     * 20:43:44 [LSC_CDump]         [ 0] Integer field: 2100000000
     * 20:43:44 [LSC_CDump]         [ 1] (None)
     * 20:43:44 [LSC_CDump]         [ 2] Integer field: 1
     * 20:43:44 [LSC_CDump]   Call Named Arguments:
     * 20:43:44 [LSC_CDump]     Argument 'machoVersion':
     * 20:43:44 [LSC_CDump]         Integer field: 1
     */

    // WARNING: This call contains manual packet decoding to handle Access Control since I didn't want to monkey around with the LSCPkts.xmlp.
    // -- Aknor Jaden (2010-11-26)

    int32 channel_id = 0;

    //m_db->UpdateChannelInfo(channel);

    // BIG TODO:  The whole reason why normal players cannot post chats in other channels has to do with the Access Mode
    // in the channel settings dialog in the client.  Now, I don't know why chatting in the Help/Rookie Help is not allowed
    // for ANY normal players, however, at the very least, implementing the change in Access Mode to 3 = Allowed may fix
    // the issue.  The only thing to figure out is how to format a packet to be sent to the client(s) to inform of the change
    // in access mode.  Is this really needed though, since the owner will change the mode and the owner's client will know
    // immediately, and anyone wanting to join will get that mode value when the JoinChannel has been called.

    // call.tuple->GetItem(0)->AsInt()->value() = channel ID
    // call.tuple->GetItem(1)->IsNone() == true  <---- change made to "" field
    // call.tuple->GetItem(2)->AsInt()->value() =
    //     0 = ??
    //     1 = Moderated
    //     2 = ??
    //     3 = Allowed

    // call.tuple->GetItem(1)->IsInt() == true  <---- character ID for character add to one of the lists specified by GetItem(2):
    // call.tuple->GetItem(2)->AsInt()->value() =
    //     3 = Add to Allowed List
    //     -2 = Add to Blocked List
    //     7 = Add to Moderators List

    //channel->UpdateConfig();

    return new PyInt(1);
}

PyResult LSCService::Handle_Invite(PyCallArgs &call)
{
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White("LSCService::Handle_Invite()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    // WARNING: This call contains manual packet decoding to handle chat messages sent inside user-created
    // chat channels since I didn't want to monkey around with the LSCPkts.xmlp.
    // -- Aknor Jaden (2010-11-19)

    LSCChannel* channel(nullptr);

    uint32 channel_ID;
    uint32 char_ID = call.client->GetCharacterID();
    uint32 invited_char_ID;

    // Decode the call:
    if (call.tuple->IsTuple()) {
        if (call.tuple->GetItem(1)->IsInt())
            channel_ID = call.tuple->GetItem(1)->AsInt()->value();
        else {
            _log(LSC__ERROR, "%s: call.tuple->GetItem(1) is of the wrong type: '%s'.  Expected PyInt type.", call.client->GetName(), call.tuple->TypeString());
            return nullptr;
        }

        if (call.tuple->GetItem(0)->IsInt())
            invited_char_ID = call.tuple->GetItem(0)->AsInt()->value();
        else {
            _log(LSC__ERROR, "%s: call.tuple->GetItem(0) is of the wrong type: '%s'.  Expected PyInt type.", call.client->GetName(), call.tuple->TypeString());
            return nullptr;
        }
    } else {
        _log(LSC__ERROR, "%s: call.tuple is of the wrong type: '%s'.  Expected PyTuple type.", call.client->GetName(), call.tuple->TypeString());
        return nullptr;
    }

    if (m_channels.find(channel_ID) != m_channels.end()) {
        channel = m_channels[channel_ID];

        if (!channel->IsJoined(invited_char_ID)) {
            // SOMEHOW SEND A JOIN COMMAND/REQUEST TO THE TARGET CLIENT FOR invited_char_ID
            /*    OnLSC_JoinChannel join;
             *            join.sender = channel->_MakeSenderInfo(call.client);
             *            join.member_count = 1;
             *            join.channelID = channel->EncodeID();
             *            PyTuple *answer = join.Encode();
             *            MulticastTarget mct;
             *            //LSCChannelChar *invitor;
             *            //LSCChannelChar *invitee;
             *            if (!channel->IsJoined(char_ID))
             *            {
             *                //invitor = new LSCChannelChar(channel,0,char_ID,call.client->GetCharacterName(),0,0,0,0);
             *                mct.characters.insert(char_ID);
        }
        //invitee = new LSCChannelChar(channel,0,invited_char_ID,entityList().FindCharacter(invited_char_ID)->GetCharacterName(),0,0,0,0);
        mct.characters.insert(invited_char_ID);
        entityList().Multicast("OnLSC", channel->GetTypeString(), &answer, mct);
        //entityList().Unicast(invited_char_ID,"OnLSC",channel->GetTypeString(),&answer,false);
        */

            // ********** TODO **********
            // Figure out how to send the ChatInvite packet to the client running the character with id = 'invited_char_ID'
            // in order for that character's client to then issue the JoinChannels call to the server with the chat channel
            // ID equal to that of this channel, be it either a private convo (temporary==1) or an existing user-created chat.
            // **************************

            //ChatInvite chatInvitePacket;
            //chatInvitePacket.integer1 = 1;
            //chatInvitePacket.integer2 = invited_char_ID;
            //chatInvitePacket.boolean = true;
            //chatInvitePacket.displayName = call.tuple->GetItem(2)->AsString()->content();
            //chatInvitePacket.integer3 = 1;
            //chatInvitePacket.integer4 = 0;
            //chatInvitePacket.integer5 = 1;
            //PyTuple *tuple = chatInvitePacket.Encode();
            //entityList().Unicast(invited_char_ID, "", "", &tuple, false);
        } else {
            _log(LSC__ERROR, "%s: Character %u is already joined to channel %u.", call.client->GetName(), invited_char_ID, channel_ID);
            return nullptr;
        }
    } else {
        _log(LSC__ERROR, "%s: Cannot find channel %u.", call.client->GetName(), channel_ID);
        return nullptr;
    }

    return new PyInt(1);
}

PyResult LSCService::Handle_Configure(PyCallArgs& call)
{
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White("LSCService::Handle_Configure()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }
    /*
     * 20:43:44 W LSCService::Handle_Configure(): size=1
     * 20:43:44 [LSC_CDump]   Call Arguments:
     * 20:43:44 [LSC_CDump]       Tuple: 1 elements
     * 20:43:44 [LSC_CDump]         [ 0] Integer field: 2100000000
     * 20:43:44 [LSC_CDump]   Call Named Arguments:
     * 20:43:44 [LSC_CDump]     Argument 'machoVersion':
     * 20:43:44 [LSC_CDump]         Integer field: 1
     * 20:43:44 [LSC_CDump]     Argument 'motd':
     * 20:43:44 [LSC_CDump]         WString: 'test MOTD'
     *
     * 21:13:02 W LSCService::Handle_Configure(): size=1
     * 21:13:02 [LSC_CDump]   Call Arguments:
     * 21:13:02 [LSC_CDump]       Tuple: 1 elements
     * 21:13:02 [LSC_CDump]         [ 0] Integer field: 2100000000
     * 21:13:02 [LSC_CDump]   Call Named Arguments:
     * 21:13:02 [LSC_CDump]     Argument 'creator':
     * 21:13:02 [LSC_CDump]         Integer field: 140000130
     * 21:13:02 [LSC_CDump]     Argument 'machoVersion':
     * 21:13:02 [LSC_CDump]         Integer field: 1
     *
     */

    // WARNING: This call contains manual packet decoding to handle configuring parameters for
    // user-created chat channels since I didn't want to monkey around with the LSCPkts.xmlp.
    // -- Aknor Jaden (2010-11-26)

    LSCChannel* channel(nullptr);
    int32 channel_id = 0;

    //ChannelInfo args;
    //if (!args.Decode(call.tuple) {
    //codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
    //    return nullptr;
    //}

    // Get Tuple which contains channel number to modify:
    if (call.tuple->AsTuple()->GetItem(0)->IsInt())
        channel_id = call.tuple->AsTuple()->GetItem(0)->AsInt()->value();
    else {
        _log(LSC__ERROR, "%s: Tuple contained wrong type: '%s'", call.client->GetName(), call.tuple->TypeString());
        return nullptr;
    }

    // Get count of parameters or just loop through the std::map until you've reached the end
    if (call.byname.size() == 0) {
        _log(LSC__ERROR, "%s: byname std::map contained zero elements, expected at least one.", call.client->GetName());
        return nullptr;
    }

    // Find channel in existing channels:
    std::map<int32, LSCChannel*>::iterator res = m_channels.find(channel_id);
    if (m_channels.end() == res)  {
        _log(LSC__ERROR, "%s: Handle_Configure Couldn't find channel %u", call.client->GetName(), channel_id);
        return nullptr;
    }

    channel = m_channels.find(channel_id)->second;
    if (call.byname.find("displayName") != call.byname.end()) {
        if (call.byname.find("displayName")->second->IsWString()) {
            channel->SetDisplayName(call.byname.find("displayName")->second->AsWString()->content());
        } else {
            _log(LSC__ERROR, "%s: displayName contained wrong type: '%s'", call.client->GetName(), call.byname.find("displayName")->second->TypeString());
            return nullptr;
        }
    }

    if (call.byname.find("memberless") != call.byname.end()) {
        if (call.byname.find("memberless")->second->IsInt()) {
            channel->SetMemberless(call.byname.find("memberless")->second->AsInt()->value() ? true : false);
        } else {
            _log(LSC__ERROR, "%s: memberless contained wrong type: '%s'", call.client->GetName(), call.byname.find("memberless")->second->TypeString());
            return nullptr;
        }
    }

    if (call.byname.find("motd") != call.byname.end()) {
        if (call.byname.find("motd")->second->IsWString()) {
            channel->SetMOTD(call.byname.find("motd")->second->AsWString()->content());
        } else {
            _log(LSC__ERROR, "%s: motd contained wrong type: '%s'", call.client->GetName(), call.byname.find("motd")->second->TypeString());
            return nullptr;
        }
    }

    if (call.byname.find("oldPassword") != call.byname.end()) {
        if (call.byname.find("oldPassword")->second->IsWString()) {
            if (channel->GetPassword() == call.byname.find("oldPassword")->second->AsWString()->content()) {
                if (call.byname.find("newPassword") != call.byname.end()) {
                    if (call.byname.find("newPassword")->second->IsWString()) {
                        channel->SetPassword(call.byname.find("newPassword")->second->AsWString()->content());
                    } else {
                        _log(LSC__ERROR, "%s: newPassword contained wrong type: '%s'", call.client->GetName(), call.byname.find("newPassword")->second->TypeString());
                        return nullptr;
                    }
                }
            } else {
                _log(LSC__ERROR, "%s: incorrect oldPassword supplied. Password NOT changed.", call.client->GetName());
                return nullptr;
            }
        } else if (call.byname.find("oldPassword")->second->IsNone()) {
            if (call.byname.find("newPassword") != call.byname.end()) {
                if (call.byname.find("newPassword")->second->IsWString()) {
                    channel->SetPassword(call.byname.find("newPassword")->second->AsWString()->content());
                } else {
                    _log(LSC__ERROR, "%s: newPassword contained wrong type: '%s'", call.client->GetName(), call.byname.find("newPassword")->second->TypeString());
                    return nullptr;
                }
            }
        } else {
            _log(LSC__ERROR, "%s: oldPassword is of an unexpected type: '%s'", call.client->GetName(), call.byname.find("newPassword")->second->TypeString());
            return nullptr;
        }
    }

    m_db->UpdateChannelInfo(channel);

    channel->UpdateConfig();

    return new PyNone();
}


PyResult LSCService::Handle_LeaveChannel(PyCallArgs &call) {
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White("LSCService::Handle_LeaveChannel()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    CallLeaveChannel arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return new PyNone();
    }

    uint32 toLeave = 0;

    if (arg.channel->IsInt())
        toLeave = arg.channel->AsInt()->value();
    else if (arg.channel->IsTuple()) {
        PyTuple* prt = arg.channel->AsTuple();

        if (prt->GetItem(0)->IsInt())
            toLeave = prt->GetItem(0)->AsInt()->value();
        else if (prt->GetItem(0)->IsTuple()) {
            prt = prt->GetItem(0)->AsTuple();

            if (prt->items.size() != 2 or !prt->GetItem(1)->IsInt()) {
                codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
                return new PyNone();
            }

            toLeave = prt->GetItem(1)->AsInt()->value();
        } else {
            codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
            return new PyNone();
        }
    } else {
        codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
        return new PyNone();
    }

    if (arg.unsubscribe)
        m_db->RemoveChannelSubscriptionFromDatabase(toLeave,call.client->GetCharacterID());

    std::map<int32, LSCChannel*>::iterator itr = m_channels.find(toLeave);
    if (itr != m_channels.end()) {
        itr->second->LeaveChannel(call.client);
        if ((itr->second->GetMemberCount() < 1) and (itr->second->GetTemporary())) {
            m_db->RemoveChannelFromDatabase(toLeave);
            SafeDelete(itr->second);
            m_channels.erase(itr);
        }
    }

    return new PyNone();
}


PyResult LSCService::Handle_LeaveChannels(PyCallArgs &call) {
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White("LSCService::Handle_LeaveChannels()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    CallLeaveChannels args;

    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return new PyNone();
    }

    std::set<uint32> toLeave;
    toLeave.clear();

    PyList::const_iterator cur = args.channels->begin();
    for (; cur != args.channels->end(); cur++) {
        if ((*cur)->IsInt())
            toLeave.insert((*cur)->AsInt()->value());
        else if ((*cur)->IsTuple()) {
            PyTuple* prt = (*cur)->AsTuple();

            if (prt->GetItem(0)->IsInt()) {
                toLeave.insert(prt->GetItem(0)->AsInt()->value());
                continue;
            }

            if (!prt->GetItem(0)->IsTuple()) {
                codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
                continue;
            }
            prt = prt->GetItem(0)->AsTuple();

            if (prt->GetItem(0)->IsTuple())
                prt = prt->GetItem(0)->AsTuple();

            if (prt->size() != 2 or !prt->GetItem(1)->IsInt()) {
                codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
                continue;
            }

            toLeave.insert(prt->GetItem(1)->AsInt()->value());
        } else {
            codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
            continue;
        }
    }

    std::set<uint32>::iterator itr = toLeave.begin();
    std::map<int32, LSCChannel*>::iterator itr2;
    for (; itr!=toLeave.end(); itr++) {
        itr2 = m_channels.find(*itr);
        if (itr2 != m_channels.end()) {
            itr2->second->LeaveChannel(call.client);
            if (args.unsubscribe)
                m_db->RemoveChannelSubscriptionFromDatabase(*itr, call.client->GetCharacterID());

            if ((itr2->second->GetMemberCount() < 1) and (itr2->second->GetTemporary())) {
                m_db->RemoveChannelFromDatabase(*itr);
                SafeDelete(itr2->second);
                m_channels.erase(itr2);
            }
        }
    }

    return new PyNone();
}

PyResult LSCService::Handle_DestroyChannel(PyCallArgs& call)
{
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White("LSCService::Handle_DestroyChannel()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    Call_SingleIntegerArg arg;
    if (!arg.Decode(call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return new PyNone();
    }

    std::map<int32, LSCChannel*>::iterator itr = m_channels.find(arg.arg);
    if (itr == m_channels.end()) {
        _log(LSC__ERROR, "%s: Couldn't find channel %u", call.client->GetName(), arg.arg);
        return new PyNone();
    }

    // ********** TODO **********
    // Figure out how to validate whether this character (call.client->GetCharacterID()) is allowed
    // to destroy this chat channel, and proceed if they are, otherwise, do not.  And, is there an error
    // packet sent back to the client?
    // **************************

    // Finally, remove the channel from the server dynamic objects:
    itr->second->Evacuate(call.client);
    SafeDelete(itr->second);
    m_channels.erase(itr);

    // Now, remove the channel from the database:
    m_db->RemoveChannelFromDatabase(itr->second->GetChannelID());

    return new PyNone();
}

void LSCService::CharacterLogin(Client* pClient)
{
    // create Corp chat channel:
    CreateChannel((int32)pClient->GetCorporationID());
    // create Alliance chat channel:
    CreateChannel((int32)pClient->GetAllianceID());
}

void LSCService::CharacterLogout(uint32 charID, OnLSC_SenderInfo* si)
{
    std::map<int32, LSCChannel*>::iterator cur, end;
    cur = m_channels.begin();
    end = m_channels.end();
    for(; cur != end; cur++)
        if (cur->second->IsJoined(charID))
            cur->second->LeaveChannel(charID, new OnLSC_SenderInfo(*si));

    SafeDelete(si);
}

void LSCService::SystemUnload(uint32 systemID, uint32 constID, uint32 regionID)
{
    std::map<int32, LSCChannel*>::iterator itr = m_channels.find(systemID);
    if (itr != m_channels.end()) {
        SafeDelete(itr->second);
        m_channels.erase(itr);
    }
    /** @todo  find a way to track usages of region and const channels to delete when no longer used */
    //  is this needed?
}

LSCChannel* LSCService::CreateChannel(int32 channelID, const char * name, const char * motd, LSC::Type type, const char * compkey,
                                      uint32 ownerID, bool memberless, const char * password, bool maillist, uint32 cspa, bool temporary,
                                      bool languageRestriction, int8 groupMessageID, int8 channelMessageID) {
    std::map<int32, LSCChannel*>::iterator itr = m_channels.find(channelID);
    if (itr != m_channels.end())
        return itr->second;
    return m_channels[channelID] = new LSCChannel(this, channelID, type, ownerID, name, motd, compkey, memberless, password, maillist, cspa, temporary, languageRestriction, groupMessageID, channelMessageID);
}

LSCChannel* LSCService::CreateChannel(int32 channelID, const char * name, const char * motd, LSC::Type type, bool maillist) {
    std::map<int32, LSCChannel*>::iterator itr = m_channels.find(channelID);
    if (itr != m_channels.end())
        return itr->second;
    return m_channels[channelID] = new LSCChannel(this, channelID, type, 1, name, motd, nullptr, false, "", maillist, cspa, true, false, 0, 0);
}

LSCChannel* LSCService::CreateChannel(int32 channelID, const char * name, LSC::Type type, bool maillist) {
    std::map<int32, LSCChannel*>::iterator itr = m_channels.find(channelID);
    if (itr != m_channels.end())
        return itr->second;
    return m_channels[channelID] = new LSCChannel(this, channelID, type, 1, name, nullptr, nullptr, false, "", maillist, cspa, true, false, 0, 0);
}

LSCChannel* LSCService::CreateChannel(int32 channelID) {
    if (!channelID)
        return nullptr;
    std::map<int32, LSCChannel*>::iterator itr = m_channels.find(channelID);
    if (itr != m_channels.end())
        return itr->second;
    LSC::Type type;
    std::string name;
    std::string motd;
    if (IsRegion(channelID)) { type = LSC::Type::region; name = "System Channels\\Region"; motd = m_db->GetRegionName(channelID); }
    else if (IsConstellation(channelID)) {type = LSC::Type::constellation; name = "System Channels\\Constellation"; motd = m_db->GetConstellationName(channelID); }
    else if (IsSolarSystem(channelID)) { type = LSC::Type::solarsystem; name = "System Channels\\Local"; motd = m_db->GetSolarSystemName(channelID); }
    else if (IsCorp(channelID)) { type = LSC::Type::corp; name = "System Channels\\Corp"; motd = m_db->GetCorporationName(channelID); }
    else { type = LSC::Type::normal; m_db->GetChannelInfo(channelID, name, motd); }

    return m_channels[channelID] = new LSCChannel(this, channelID, type, 1, name.c_str(), motd.c_str(), "", false, "", nullptr, cspa, false, false, 0, 0);
}

LSCChannel* LSCService::CreateChannel(const char * name, bool maillist/*false*/) {
    int32 nextFreeChannelID = m_db->GetNextAvailableChannelID();

    if (nextFreeChannelID)
        return CreateChannel(nextFreeChannelID, name, LSC::Type::normal, maillist);
    else
        return nullptr;
}

void LSCService::CreateStaticChannels() {
    // hardcode creating server static channels during server startup
    std::ostringstream str;
    /*  Incursion Rookie Help MOTD...
     * <color=0xff007fff><b>Welcome to: EVE Online: Incursion<br></color>
     * <color=0xff00ff00>Chatrules</color><color=0xffffffff>: </color>
     * <color=0xffffa500><url=http://www.eveonline.com/pnp/chatrules.asp><u>http://www.eveonline.com/pnp/chatrules.asp</color>
     * <color=0xffffffff></url></u> </color>
     * <color=0xffb2b2b2>please read and observe them..<br></color>
     * <color=0xff00ff00>Topic</color>
     * <color=0xffffffff>: Stay on topic of EVE-Online related rookie help.<br></color>
     * <color=0xff00ff00>Rules: </color>
     * <color=0xffffffff>No WTB, WTS, WTT (aka trading, selling, w.e.), PC, advertising, recruiting, scamming, offering private help in any form or begging in this channel.
     * No CAPS or text-decoration either<br></color>
     * <color=0xff00ff00>Language</color><color=0xffffffff>: This channel is ENGLISH ONLY if you want to chat in another language please find the channel in CHANNELS &amp; MAILING LIST (speech bubble in top right corner) then the Languages folder.<br><br></color>
     * <color=0xffffff00>ISK Advertising: Contrary as to what they spam, CCP has never, will never and does not intend to "authorize" any person or site to sell ISK for RL cash !<br><br></color>
     * <color=0xff00ff00>Recommended reading</color>
     * <color=0xffffffff>: </color><color=0xffffa500><url=http://wiki.eveonline.com/wiki/Category:New_Player_Experience><u>http://wiki.eveonline.com/wiki/Category:New_Player_Experience</color>
     * <color=0xff007fff></url></b></u> <br><br></color>
     * <color=0xff00ff00><b>Please Note:</color>
     * <color=0xffffffff> There are no third party applications ().exe) that will magically give you any type of ship you wish or \'hack\' your wallet. Please report characters advertising these types of links IMMEDIATELY via petition and DO NOT download and \'try\' them. <br><br>
     * Sister of EVE storyline starts in <url=showinfo:5//30005001><u>Arnon</url></b></u> <b>with <url=showinfo:1378//3019356><u>Sister Alitura</url></b></u>. <br><br></color>
     * <color=0xff00ff00><b>Before asking, please read: </color>
     * <color=0xffffa500><url=http://wiki.eveonline.com/wiki/Rookie_Help_Channel_FAQ><u>http://wiki.eveonline.com/wiki/Rookie_Help_Channel_FAQ</color>
     * <color=0xff007fff></url></b></u> <br><br></color>
     * <color=0xff00ff00><b>Please note: </color>
     * <color=0xffffffff>To warp to ANY signature you have to use 4 probes and scan it to 100%.</color></b>'
     */
    // Incursion Help MOTD...
    str << "<color=0xffff0000><b>Welcome to Alasiya's EVE Online: Crucible Emulator</color>";
    str << "<color=0xff007fff> <br><br></color><color=0xff00ff00>Player Guides:</color>";
    str << "<color=0xffffa500><url=http://wiki.eveonline.com/wiki/Category:New_Player_Experience><u>http://wiki.eveonline.com/wiki/Category:New_Player_Experience</color><color=0xb2b2b2ff></url></u> </color>";
    str << "<color=0xffb2b2b2><b>Please: Stay on topic.</color><color=0xb2b2b2ff> </color>";
    str << "<color=0xffffff00>No offtopic, WTB, WTS, PC, advertising, recruiting, scamming/trading in general or begging in this channel. No CAPS or text-decoration.<br></color>";
    str << "<color=0xff00ff00>Language:</color><color=0xffb2b2b2>This channel is</color><color=0xffff0000>ENGLISH ONLY!!<br></color>";
    str << "<color=0xff00ff00>How to contact a GM: </color>";
    str << "<color=0xffb2b2b2>File a petition (F12 - Petitions - New Petition) <br></color>";
    str << "<color=0xff00ff00><b>The topic of this channel is EVE related help.</color></b>";

    const char *motd = str.str().c_str();
    CreateChannel(1, "Help\\Rookie Help", motd, LSC::Type::normal, "help", 1, false, "", false, cspa, false, false, 0, -1);
    CreateChannel(2, "Help\\Help", motd, LSC::Type::normal, "help", 1, false, "", false, cspa, false, false, 0, -1);

    CreateChannel(10, "Trade\\Other", "motd", LSC::Type::normal, "other", 1, true, "", false, cspa, false, false, 0, -1);
    CreateChannel(11, "Trade\\Ships", "motd", LSC::Type::normal, "ships", 1, true, "", false, cspa, false, false, 0, -1);
    CreateChannel(12, "Trade\\Blueprints", "motd", LSC::Type::normal, "blueprints", 1, true, "", false, cspa, false, false, 0, -1);
    CreateChannel(13, "Trade\\Modules and Munitions", "motd", LSC::Type::normal, "modulesandmunitions", 1, true, "", false, cspa, false, false, 0, -1);
    CreateChannel(14, "Trade\\Minerals and Manufacturing", "motd", LSC::Type::normal, "mineralsandmanufacturing", 1, true, "", false, cspa, false, false, 0, -1);

    CreateChannel(16, "Empires\\Caldari", "motd", LSC::Type::normal, "caldari", 1, true, "", false, cspa, false, false, 0, -1);
    CreateChannel(17, "Empires\\Amarr", "motd", LSC::Type::normal, "amarr", 1, true, "", false, cspa, false, false, 0, -1);
    CreateChannel(18, "Empires\\Minmatar", "motd", LSC::Type::normal, "minmatar", 1, true, "", false, cspa, false, false, 0, -1);
    CreateChannel(19, "Empires\\Gallente", "motd", LSC::Type::normal, "gallente", 1, true, "", false, cspa, false, false, 0, -1);
    CreateChannel(20, "Empires\\Jove", "motd", LSC::Type::normal, "jove", 1, true, "", false, cspa, false, false, 0, -1);
    CreateChannel(21, "Alliances\\Smacktalk", "motd", LSC::Type::normal, "smacktalk", 1, true, "", false, cspa, false, false, 0, -1);
    CreateChannel(22, "Alliances\\Rumour Mill", "motd", LSC::Type::normal, "rumourmill", 1, true, "", false, cspa, false, false, 0, -1);
    CreateChannel(23, "Alliances\\Freelancer", "motd", LSC::Type::normal, "freelancer", 1, true, "", false, cspa, false, false, 0, -1);

    str.clear();
    // Incursion recruitment MOTD...
    str << "Welcome to the recruitment channel.  This channel is intended for those players looking to find a new corporation, as well as those looking to enlist new players.";
    str << "Other activities, such as non-recruitment discussion and scamming are not permitted in this channel.";
    str << "An additional recruitment source is the <url=http://www.eveonline.com/ingameboard.asp?a=channel&channelID=109585>Alliance and Corporation Recruitment Center</url> section of the forums.";
    const char *motd2 = str.str().c_str();
    CreateChannel(24, "Corporate\\Recruitment", motd2, LSC::Type::normal, "recruitment", 1, true, "", false, cspa, false, false, 0, -1);
    CreateChannel(25, "Corporate\\CEO", "motd", LSC::Type::normal, "ceo", 1, true, "", false, cspa, false, false, 0, -1);
}

///////////////////////////////////////////////////////////////////////////////
//
// EveMail calls:
//
///////////////////////////////////////////////////////////////////////////////

PyResult LSCService::Handle_GetMyMessages(PyCallArgs &call) {
    return(m_db->GetMailHeaders(call.client->GetCharacterID()));
}


PyResult LSCService::Handle_GetMessageDetails(PyCallArgs &call) {
    Call_TwoIntegerArgs args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    //TODO: verify ability to read this message...

    return(m_db->GetMailDetails(args.arg2, args.arg1));
}


PyResult LSCService::Handle_Page(PyCallArgs &call) {
    Call_Page args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    _log(SERVICE__MESSAGE, "%s: Received evemail msg with subject '%s': %s", call.client->GetName(), args.subject.c_str(), args.body.c_str());

    SendMail(call.client->GetCharacterID(), args.recipients, args.subject, args.body);

    return nullptr;
}


//stuck here to be close to related functionality
void LSCService::SendMail(uint32 sender, const std::vector<int32> &recipients, const std::string &subject, const std::string &content) {
    NotifyOnMessage notify;
    std::set<uint32> successful_recipients;

    notify.subject = subject;
    notify.sentTime = Win32TimeNow();
    notify.senderID = sender;

    // there's attachmentID and messageID... does this means a single message can contain multiple attachments?
    // eg. text/plain and text/html? we should be watching for this at reading mails...
    // created should be creation time. But Win32TimeNow returns uint64, and is stored as bigint(20),
    // so change in the db is needed
    std::vector<int32>::const_iterator cur, end;
    cur = recipients.begin();
    end = recipients.end();

    for(; cur != end; cur++) {
        uint32 messageID = m_db->StoreMail(sender, *cur, subject.c_str(), content.c_str(), notify.sentTime);
        if(messageID == 0) {
            _log(SERVICE__ERROR, "Failed to store message from %u for recipient %u", sender, *cur);
            continue;
        }
        //TODO: supposed to have a different messageID in each notify I suspect..
        notify.messageID = messageID;

        _log(SERVICE__MESSAGE, "Delivered message from %u to recipient %u", sender, *cur);
        //record this person in the 'delivered to' list:
        notify.recipients.push_back(*cur);
        successful_recipients.insert(*cur);
    }

    //now, send a notification to each successful recipient
    PyTuple *answer = notify.Encode();
    sEntityList.Multicast(successful_recipients, "OnMessage", "*multicastID", &answer, false);
}


//stuck here to be close to related functionality
//theres a lot of duplicated crap in here...
//this could be replaced by the SendNewEveMail if it weren't in the Client
void Client::SelfEveMail(const char* subject, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char* str = nullptr;
    vasprintf(&str, fmt, args);
    assert(str);

    va_end(args);

    m_services.lsc_service->SendMail(GetCharacterID(), GetCharacterID(), subject, str);
    SafeFree(str);
}


PyResult LSCService::Handle_MarkMessagesRead(PyCallArgs &call) {
    Call_SingleIntList args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    std::vector<int32>::iterator cur, end;
    cur = args.ints.begin();
    end = args.ints.end();
    for(; cur != end; cur++) {
        m_db->MarkMessageRead(*cur);
    }
    return nullptr;
}


PyResult LSCService::Handle_DeleteMessages(PyCallArgs &call) {
    Call_DeleteMessages args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    if(args.channelID != (int32)call.client->GetCharacterID()) {
        _log(SERVICE__ERROR, "%s (%d) tried to delete messages in channel %u. Denied.", call.client->GetName(), call.client->GetCharacterID(), args.channelID);
        return nullptr;
    }

    std::vector<int32>::iterator cur, end;
    cur = args.messages.begin();
    end = args.messages.end();
    for(; cur != end; cur++) {
        m_db->DeleteMessage(*cur, args.channelID);
    }

    return nullptr;
}


PyResult LSCService::Handle_GetMembers(PyCallArgs &call) {
    CallGetMembers arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    uint32 channelID;
    if (arg.channel->IsInt())
        channelID = arg.channel->AsInt()->value();
    else if (arg.channel->IsTuple())
    {
        PyTuple* prt = arg.channel->AsTuple();

        if (prt->GetItem(0)->IsInt())
            channelID = prt->GetItem(0)->AsInt()->value();
        else if (prt->GetItem(0)->IsTuple())
        {
            prt = prt->GetItem(0)->AsTuple();

            if (prt->items.size() != 2 or !prt->GetItem(1)->IsInt())
            {
                codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
                return nullptr;
            }

            channelID = prt->GetItem(1)->AsInt()->value();
        }
        else
        {
            codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
            return nullptr;
        }
    }
    else
    {
        codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
        return nullptr;
    }

    if (m_channels.find(channelID) != m_channels.end())
        return m_channels[channelID]->EncodeChannelChars();

    return nullptr;
}


PyResult LSCService::Handle_GetMember(PyCallArgs &call) {
    return nullptr;
}


PyResult LSCService::ExecuteCommand(Client *from, const char *msg) {
    return(m_commandDispatch->Execute(from, msg));
}
