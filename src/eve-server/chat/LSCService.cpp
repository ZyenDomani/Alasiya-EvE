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

#include <boost/algorithm/string.hpp>

#include "eve-server.h"

#include "PyServiceCD.h"
#include "admin/CommandDispatcher.h"
#include "admin/SlashService.h"
#include "chat/LSCService.h"
#include "fleet/FleetService.h"

PyCallable_Make_InnerDispatcher(LSCService)

LSCService::LSCService(PyServiceMgr *mgr, CommandDispatcher* cd)
: PyService(mgr, "LSC"),
m_dispatch(new Dispatcher(this)),
m_commandDispatch(cd),
m_nextChannelID(0),
m_accessHeader(nullptr),
m_channelHeader(nullptr)
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(LSCService, GetChannels);
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
    PyCallable_REG_CALL(LSCService, ForgetChannel);
    PyCallable_REG_CALL(LSCService, RenameChannel);
    PyCallable_REG_CALL(LSCService, SetChannelMOTD);
    PyCallable_REG_CALL(LSCService, SetChannelLanguageRestriction);

    /* Vivox shit...
     sm.RemoteSvc('LSC').VoiceStatus(eveChannelName, 0) leave channel                 *
     sm.RemoteSvc('LSC').VoiceStatus(eveChannelID, 1)  ungag
     sm.RemoteSvc('LSC').VoiceStatus(eveChannelID, 2)  gag
     */

    m_db = new LSCDB();

    CreateStaticChannels();

    m_nextChannelID = m_db->GetHighestChannelIDFromDB() + 1;
    if (m_nextChannelID < minChatChannel)
        m_nextChannelID = minChatChannel;

    CreateACLHeader();
    CreateChannelHeader();

    // make startup msg with # of static channels created
    _log(LSC__CHANNELS, "Chat Subsystem Online. Next ChannelID: %i", m_nextChannelID);
    _log(LSC__CHANNELS, "CreateStaticChannels: Created %lli static chat channels.", m_channels.size());
}


LSCService::~LSCService() {
    SafeDelete(m_db);
    delete m_dispatch;

    for (auto& cur : m_channels)
        SafeDelete(cur.second);
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

const int cspa = 2950; // CONCORD Spam Prevention Act


// new method here:  this can be used for fleet bcast
void LSCService::BroadcastAlertMessage(const char* alertText) {
    /*
     *    LSC_BroadcastEvent event;
     *    event.message = alertText;
     *
     *    // Injects -1153434 straight to the encoder, forcing the text to render as a critical alert red flashing block!
     *    event.textColor = LSC::Color::CritRed.clientInt;
     *
     *    PyTuple* payload = event.Encode();
     *    std::set<uint32> roomOnlineRoster;
     *    for (const auto& pair : m_chars)
     *        roomOnlineRoster.insert(pair.first);
     *    PyTuple* answerPayload = joinEvent.Encode();
     *    sEntityMgr.Multicast(roomOnlineRoster, "OnLSC", GetTypeString(), &answerPayload, false);
     */
}

/**************************************************************
 * ***************  reviewed/updated  **************************
 **************************************************************/
void LSCService::SendServerMOTD(Client* pClient) {
    auto itr = m_channels.find(pClient->GetSystemID());
    if (itr != m_channels.end() && itr->second != nullptr)
        itr->second->SendServerMOTD(pClient);
}

PyResult LSCService::ExecuteCommand(Client *from, const char *msg) {
    return (m_commandDispatch->Execute(from, msg));
}

void LSCService::CharacterLogin(Client* pClient) {
    // note: system, const, region channels are created on system boot
    int32 corpID = pClient->GetCorporationID();
    int32 allianceID = pClient->GetAllianceID();

    if (IsCorpID(corpID))
        CreateDynamicChannel(corpID, CorporationDB::GetCorporationCEO(corpID));

    if (IsAllianceID(allianceID))
        CreateDynamicChannel(allianceID, CorporationDB::GetCorporationCEO(corpID));
}

void LSCService::SystemUnload(int32 systemID, int32 constID, int32 regionID) {
    _log(LSC__CHANNELS, "Solar System %s is calling LSC::Unload on Channel %i.", sDataMgr.GetSystemName(systemID), systemID);
    DeleteSystemChannel(systemID);
    DeleteChannelIfEmpty(constID);
    DeleteChannelIfEmpty(regionID);
}

PyTuple* LSCService::SendError(int8 error) {
    PyTuple* errorTuple = new PyTuple(3);
    errorTuple->SetItem(0, new PyInt(error));
    errorTuple->SetItem(1, PyStatic.NewNone());
    errorTuple->SetItem(2, PyStatic.NewNone());
    return errorTuple;
}

LSCChannel* LSCService::GetChannelByName(const std::string& channelName) {
    if (channelName.empty())
        return nullptr;

    std::string compKey = channelName;
    boost::algorithm::trim(compKey);
    boost::algorithm::to_lower(compKey);
    compKey.erase(std::remove(compKey.begin(), compKey.end(), ' '), compKey.end());
    //compKey.erase(std::remove(compKey.begin(), compKey.end(), ':'), compKey.end());
    //compKey.erase(std::remove(compKey.begin(), compKey.end(), '\''), compKey.end());

    auto nameIt = m_channelNameMap.find(compKey);
    if (nameIt != m_channelNameMap.end()) {
        int32 absoluteChannelID = nameIt->second;
        auto channelIt = m_channels.find(absoluteChannelID);
        if (channelIt != m_channels.end())
            return channelIt->second;
    }

    return nullptr;
}

LSCChannel* LSCService::GetChannelByID(int32 channelID) {
    if (channelID == 0)
        return nullptr;

    // If the ID is a negative user-created channel, strip the sign bit to get the map key
    int32 lookupID = (channelID < 0) ? (channelID & 0x7FFFFFFF) : channelID;
    auto itr = m_channels.find(lookupID);
    if (itr != m_channels.end())
        return itr->second;

    return nullptr;
}

void LSCService::DeleteSystemChannel(int32 channelID) {
    if (channelID == 0)
        return;

    if (IsStaticChannel(channelID)) {
        _log(LSC__ERROR, "DeleteSystemChannel() called on Static Channel %i.", channelID);
        return;
    }

    auto itr = m_channels.find(channelID);
    if (itr == m_channels.end())
        return;

    LSCChannel* pChannel = itr->second;
    if (pChannel == nullptr)
        return;

    std::string compKey;
    if (pChannel->GetComparisonKey().empty()) {
        compKey = pChannel->GetDisplayName();
    	boost::algorithm::trim(compKey);
    	boost::algorithm::to_lower(compKey);
        compKey.erase(std::remove(compKey.begin(), compKey.end(), ' '), compKey.end());
        //compKey.erase(std::remove(compKey.begin(), compKey.end(), ':'), compKey.end());
        //compKey.erase(std::remove(compKey.begin(), compKey.end(), '\''), compKey.end());
    } else {
        compKey = pChannel->GetComparisonKey();
    }

    auto nameIt = m_channelNameMap.find(compKey);
    if (nameIt != m_channelNameMap.end())
        m_channelNameMap.erase(nameIt);

    pChannel->Evacuate(nullptr);
    SafeDelete(pChannel);
    m_channels.erase(itr);
}

void LSCService::DeleteChannelIfEmpty(int32 channelID) {
    if (channelID == 0)
        return;
    if (IsStaticChannel(channelID)) {
        _log(LSC__ERROR, "DeleteChannelIfEmpty() called on Static Channel %i.", channelID);
        return;
    }

    auto itr = m_channels.find(channelID);
    if (itr == m_channels.end())
        return;

    LSCChannel* pChannel = itr->second;
    if (pChannel == nullptr)
        return;
    if (pChannel->GetMemberCount() > 0)
        return;

    std::string compKey;
    if (pChannel->GetComparisonKey().empty()) {
        compKey = pChannel->GetDisplayName();
        boost::algorithm::trim(compKey);
        boost::algorithm::to_lower(compKey);
        compKey.erase(std::remove(compKey.begin(), compKey.end(), ' '), compKey.end());
        //compKey.erase(std::remove(compKey.begin(), compKey.end(), ':'), compKey.end());
        //compKey.erase(std::remove(compKey.begin(), compKey.end(), '\''), compKey.end());
    } else {
        compKey = pChannel->GetComparisonKey();
    }

    _log(LSC__CHANNELS, "%s Channel %i is empty and being deleted.", compKey.c_str(), channelID);
    auto nameIt = m_channelNameMap.find(compKey);
    if (nameIt != m_channelNameMap.end())
        m_channelNameMap.erase(nameIt);

    SafeDelete(pChannel);
    m_channels.erase(itr);
}

PyResult LSCService::Handle_GetChannels(PyCallArgs &call)
{
    CRowSet* rowset = new CRowSet(m_channelHeader);

    int32 charID          = call.client->GetCharacterID();
    int32 corpID          = call.client->GetCorporationID();
    int32 solarSystemID   = call.client->GetSystemID();
    int32 regionID        = call.client->GetRegionID();
    int32 constellationID = call.client->GetConstellationID();
    int32 allianceID      = call.client->GetAllianceID();

    auto itr = m_channels.begin();
    for (; itr != m_channels.end(); ++itr) {
        if (itr->second == nullptr)
            continue;

        bool sendChannel = false;
        switch (itr->second->GetType()) {
            case LSC::Type::solarsystem:
            case LSC::Type::solarsystem2: {
                // always auto-enable Local
                if (itr->first == solarSystemID)
                    sendChannel = true;
            } break;
            case LSC::Type::corp: {
                // always auto-enable own corp
                if (itr->first == corpID)
                    sendChannel = true;
            } break;
            case LSC::Type::alliance: {
                if (allianceID > 0 && itr->first == allianceID)
                    sendChannel = true;
            } break;
            case LSC::Type::constellation: {
                if (itr->first == constellationID)
                    sendChannel = true;
            } break;
            case LSC::Type::region: {
                if (itr->first == regionID)
                    sendChannel = true;
            } break;
            case LSC::Type::normal: {
                if (sConfig.chat.ReturnAllChannels) {
                    sendChannel = true;
                } else if (sConfig.chat.ReturnMostChannels) {
                    sendChannel = true;
                }
            } break;
            default: {
                continue;
            } break;
        }

        if (sendChannel) {
            PyPackedRow* into = rowset->NewRow();
            itr->second->FillPackedRow(into, call.client);
            //info.lines->AddItem(itr->second->EncodeStaticChannel(m_channelHeader, charID));
        }
    }

    if (is_log_enabled(LSC__RSP_DUMP))
        rowset->Dump(LSC__RSP_DUMP, "    ");
    return rowset;
}

PyResult LSCService::Handle_CreateChannel(PyCallArgs& call) {
    /*
     *            ret = sm.RemoteSvc('LSC').CreateChannel(displayName, joinExisting=False, memberless=0, create=True)
     *            if ret:
     *                info, acl, memberList = ret
     *        // on fail,  info is pyint(lsc::type::error), others are null
     *            if info == CHTERR_ALREADYEXISTS:
     *            if info == CHTERR_NOSUCHCHANNEL:
     *  for other fails, this method must throw
     */
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.Warning("LSCService::Handle_CreateChannel()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    Call_SingleStringArg name;
    if (!name.Decode(call.tuple))  {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    // 1. Extract keywords
    bool create = false;
    auto it = call.byname.find("create");
    if (it != call.byname.end())
        create = PyRep::GetBool(it->second);

    bool temporary = false;
    it = call.byname.find("temporary");
    if (it != call.byname.end())
        temporary = PyRep::GetBool(it->second);

    bool joinExisting = false;
    it = call.byname.find("joinExisting");
    if (it != call.byname.end())
        joinExisting = PyRep::GetBool(it->second);

    bool memberless = false;
    it = call.byname.find("memberless");
    if (it != call.byname.end())
        memberless = PyRep::GetBool(it->second);

    // no clue what this is for; only populated for private chat
    bool noCallThrottling = false;
    it = call.byname.find("noCallThrottling");
    if (it != call.byname.end())
        noCallThrottling = PyRep::GetBool(it->second);

    Client* pClient = call.client;
    int32 charID = pClient->GetCharacterID();
    LSCChannel* channel = nullptr;
    ChannelJoinOK reply;
    if (create) {
        if (GetChannelByName(name.arg) != nullptr) {
            return SendError(LSC::Error::ChannelExists);
        } else {
            // is this channel owner anything other than calling char?
            channel = CreateDynamicChannel(m_nextChannelID++, charID, name.arg.c_str());
            if (channel == nullptr)
                return SendError(LSC::Error::NoSuchChannel);
            // channel created. setup acl for calling client/char
            channel->InitACL(charID);
            reply.ChannelInfo = channel->EncodeDynamicChannel(charID);
            reply.ChannelChars = channel->EncodeChannelChars();
            reply.ChannelACL = channel->EncodeChannelACL();

            if (!temporary) {
                // save data
            	m_db->UpdateChannelInfo(channel);
            	m_db->UpdateSubscription(channel->GetChannelID(), pClient);
            }
        }
    } else if (joinExisting) {
        channel = GetChannelByName(name.arg);
        if (channel == nullptr) {
            return SendError(LSC::Error::NoSuchChannel);
        } else if (channel->HasPassword()) {
            _log(LSC__MESSAGE, "LSCS::CreateChannel(join): Channel has password.  fix this shit.");
            throw CustomError("Channel Password is not implemented yet.");
            // figure out how to send following packet...
                /*
                 *
                 *        [PySubStream 97 bytes]
                 *          [PyTuple 4 items]
                 *            [PyInt 1]                                                                                           <<-- ?
                 *            [PyString "GetChannelPasswordAndJoin"]                                      <<-- ?  is this "rsp args" going into idx[4]?  kinda looks like it.
                 *            [PyTuple 3 items]
                 *              [PyString "Password Required For "BAA Cmd" Channel"]      <<-- title
                 *              [PyInt -9877001]                                                                          <<-- channelID
                 *              [PyString "BAA Cmd"]                                                                      <<-- displayName
                 *            [PyDict 1 kvp]
                 *              [PyString "machoVersion"]
                 *              [PyInt 1]
                 */
        } else if (channel->IsJoined(charID)) {
            // already in this channel? throw error
            throw UserError("LSCChannelIsJoined")
                  .AddFormatValue("displayName", new PyString(name.arg));
        } else if (channel->JoinChannel(pClient)) {
            // join channel and send response
            reply.ChannelInfo = channel->EncodeDynamicChannel(charID);
            reply.ChannelChars = channel->EncodeChannelChars();
            reply.ChannelACL = channel->EncodeChannelACL();
        } else {
            //  throw error
            throw UserError("LSCCannotJoin")
                  .AddFormatValue("msg", new PyString("You are not able to join this channel at this time."));
        }
        if (!temporary)
            m_db->UpdateSubscription(channel->GetChannelID(), pClient);
    } else if (temporary) {
        // this is probably requesting a private chat...
        std::string mtString = "";
        channel = CreateDynamicChannel(m_nextChannelID++, charID, name.arg.c_str(), \
                                        mtString, mtString, false, true);
        if (channel == nullptr) {
            return SendError(LSC::Error::NoSuchChannel);
        } else {
            // channel created. setup acl for calling client/char
            channel->InitACL(charID);
            reply.ChannelInfo = channel->EncodeDynamicChannel(charID);
            reply.ChannelChars = channel->EncodeChannelChars();
            reply.ChannelACL = channel->EncodeChannelACL();
        }
    } else {
        // make error here for !join and !create  (should never hit)
        _log(LSC__ERROR, "%s:  !join and !create - %s", pClient->GetName(), name.arg.c_str());
        throw CustomError("Server/Client error.");
    }

    if (is_log_enabled(LSC__RSP_DUMP))
    	reply.Dump(LSC__RSP_DUMP);

    return reply.Encode();
}

PyResult LSCService::Handle_JoinChannels(PyCallArgs &call) {
    //NOTE:  cannot return nullptr here...MUST be ChannelJoinReply
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White("LSCService::Handle_JoinChannels()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    ChannelJoinReply rsp;
    rsp.ChannelID = 0;
    rsp.ok = 0;
    rsp.JoinRsp = nullptr;

    CallJoinChannels args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return rsp.Encode();
    }

    PyList* rspList = new PyList();
    LSCChannel* pChannel = nullptr;
    uint32 charID = call.client->GetCharacterID();
    PyList::const_iterator cur = args.channels->begin();
    for (; cur != args.channels->end(); ++cur) {
        int32 channelID = 0;
        if ((*cur)->IsInt()) {
            channelID = PyRep::IntegerValue(*cur);
        } else if ((*cur)->IsTuple()) {
            PyTuple* outer = (*cur)->AsTuple();
            if (outer->items.size() != 1 or !outer->items[0]->IsTuple()) {
                codelog(SERVICE__ERROR, "%s: Failed to decode arguments", call.client->GetName());
                continue;
            }

            PyTuple* inner = outer->items[0]->AsTuple();
            if (inner->items.size() != 2 or !inner->items[1]->IsInt()) {
                codelog(SERVICE__ERROR, "%s: Failed to decode arguments", call.client->GetName());
                continue;
            }

            channelID = PyRep::IntegerValue(inner->items[1]);
        } else {
            codelog(SERVICE__ERROR, "%s: Failed to decode argument ", call.client->GetName());
            continue;
        }

        // we got a channelID, try to join
        if (channelID == 0)
            continue;

        // Security Validation Guard: Rookie Help restriction checks
        if (sConfig.chat.EnforceRookieInHelp) {
            switch (channelID) {
                case 1:
                case 2:
                case 40:
                case 55:
                case 56: {
                    if (((args.role & Acct::Role::NEWBIE) != Acct::Role::NEWBIE) &&
                    	((args.role & Acct::Role::EPLAYER) != Acct::Role::EPLAYER))
                    {
                    	continue;
                    }
                } break;
                default:
                    break;
            }
        }

        // Internal lookup automatically interprets sign-bit masks via GetChannelByID
        pChannel = GetChannelByID(channelID);
        if (pChannel == nullptr)
            continue;

        rsp.ChannelID = pChannel->EncodeID();

        if (pChannel->HasPassword()) {
            call.client->SendNotifyMsg("Channel '%s' has password, but checks are not implemented yet.", \
                        pChannel->GetDisplayName().c_str());
        // figure out how to send following packet...
        /*
         *
        [PySubStream 97 bytes]
          [PyTuple 4 items]
            [PyInt 1]												<<-- ?
            [PyString "GetChannelPasswordAndJoin"]					<<-- ?  is this "rsp args" going into idx[4]?  kinda looks like it.
            [PyTuple 3 items]
              [PyString "Password Required For "BAA Cmd" Channel"]	<<-- title
              [PyInt -9877001]										<<-- channelID
              [PyString "BAA Cmd"]									<<-- displayName
            [PyDict 1 kvp]
              [PyString "machoVersion"]
              [PyInt 1]
         */

            // then, gotta figure out htf this is checked/send/wtfe
            ChannelJoinNotOK cjnok;
            cjnok.Error = "LSCWrongPassword";
            PyDict* dict = new PyDict();
            dict->SetItemString("channelName", new PyString(pChannel->GetDisplayName()));
            cjnok.rspDict = dict;
            rsp.JoinRsp = cjnok.Encode();
            rsp.ok = 0;
        }

        if (pChannel->IsBanned(charID, call.client->GetCorporationID(), call.client->GetAllianceID())) {
            ChannelJoinNotOK cjnok;
            cjnok.Error = "LSCCannotJoin";
            PyDict* dict = new PyDict();
            dict->SetItemString("msg", new PyString("Ether you, your corp, or your alliance is banned from this channel."));
            cjnok.rspDict = dict;
            rsp.JoinRsp = cjnok.Encode();
            rsp.ok = 0;
        } else if (!pChannel->IsJoined(charID)) {
            // check for password here
            if (pChannel->JoinChannel(call.client)) {
                ChannelJoinOK cjok;
                cjok.ChannelInfo = pChannel->EncodeDynamicChannel(charID);
                cjok.ChannelACL = pChannel->EncodeChannelACL();
                cjok.ChannelChars = pChannel->EncodeChannelChars();
                rsp.JoinRsp = cjok.Encode();
                rsp.ok = 1;
            } else {
                ChannelJoinNotOK cjnok;
                    cjnok.Error = "LSCCannotJoin";
                    PyDict* dict = new PyDict();
                    dict->SetItemString("msg", new PyString("Unknown error while trying to join channel."));
                    cjnok.rspDict = dict;
                rsp.JoinRsp = cjnok.Encode();
                rsp.ok = 0;
            }
        } else {
            ChannelJoinNotOK cjnok;
                cjnok.Error = "LSCChannelIsJoined";
                PyDict* dict = new PyDict();
                dict->SetItemString("displayName", new PyString(pChannel->GetDisplayName()));
                cjnok.rspDict = dict;
            rsp.JoinRsp = cjnok.Encode();
            rsp.ok = 0;
        }
        rspList->AddItem(rsp.Encode());
    }

    if (is_log_enabled(LSC__RSP_DUMP))
        rspList->Dump(LSC__RSP_DUMP, "   ");
    return rspList;
}

PyResult LSCService::Handle_SendMessage(PyCallArgs& call) {
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.Warning("LSCService::Handle_SendMessage()", "Payload packet size=%zu", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    // should any of these return error msg?

    if (!call.tuple->IsTuple())
        return nullptr;

    int32 channelID = 0;
    std::string message = "";

    if (call.tuple->AsTuple()->items[0]->IsInt()) {
        PyTuple* tuple0 = call.tuple->AsTuple();
        if (tuple0->size() != 2) {
            _log(LSC__ERROR, "LSCService::SendMessage failed: Malformed variant packet tuple length %zu", tuple0->size());
            return nullptr;
        }

        channelID = PyRep::IntegerValue(tuple0->items[0]);
        message = PyRep::StringContent(tuple0->items[1]);
        _log(LSC__INFO, "SendMessage: player channelID %i", channelID);
    } else {
        Call_SendMessage args;
        if (!args.Decode(&call.tuple)) {
            codelog(SERVICE__ERROR, "%s: Failed to decode standard system chat message packet.", GetName());
            return nullptr;
        }
        channelID = args.channel.id;
        message = args.message;
        _log(LSC__INFO, "SendMessage: system channelID %i", channelID);
    }

    if (message.empty()) {
        throw UserError("LSCCannotSendMessage")
        .AddFormatValue("msg", new PyString("Cannot send empty message."));
    }

    if (message[0] == '.') {
        SlashService* slashSvc = static_cast<SlashService*>(m_manager->LookupService("slash"));
        if (slashSvc != nullptr)
            slashSvc->SlashCommand(call.client, message);
        return nullptr;
    }

    LSCChannel* channel = GetChannelByID(channelID);
    if (channel == nullptr) {
        _log(LSC__ERROR, "%s: channelID %i not found.",
             call.client->GetName(), channelID);
        return nullptr;
    }

    channel->SendMessage(call.client, message);
    return nullptr;
}

PyResult LSCService::Handle_AccessControl(PyCallArgs& call) {
    //sm.GetService('LSC').AccessControl(self.channelID, self.charID, chat.CHTMODE_LISTENER, untilWhen, retval['reason'])
    /*// Layout of the nested PyTuple/PyList passed inside the 5th element (args)
    args[0] = targetAccessorID;  // uint32 (CharID, CorpID, AllianceID, or PyStatic.NewNone();)
    args[1] = newChannelMode;    // int8_t mapped directly to LSC::Mode
    args[2] = untilWhen;          (Absolute Wallclock Timestamp or PyStatic.NewNone();/0)
    args[3] = originalMode;      // int8_t mapped to LSC::Mode before change
    args[4] = reasonString;      // PyString containing ban/mute reason text
    args[5] = adminCharacterID;  // uint32 representing 'whoCharID' (the admin executing it)
    */

    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.Warning("LSCService::Handle_AccessControl()", "Manual payload extraction pass size=%zu", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    CallAccessControl args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode AccessControl arguments.", GetName());
        return nullptr;
    }

    int32 channelID = 0;
    if (args.channelID->IsInt()) {
        channelID = PyRep::IntegerValueI32(args.channelID);
    } else if (args.channelID->IsTuple()) {
        // Clean macro/tuple unwrapper for standard client tracking format (('private', id),)
        PyTuple* pInner = args.channelID->AsTuple();
        if (pInner->items.size() > 0 && pInner->items[0]->IsTuple()) {
            pInner = pInner->items[0]->AsTuple();
            if (pInner->items.size() >= 2 && pInner->items[1]->IsInt()) {
                channelID = PyRep::IntegerValueI32(pInner->items[1]);
            }
        }
    } else {
        // invalid.  make error here
        return nullptr;
    }

    if (channelID == 0)
        return nullptr;

    uint32 charID = call.client->GetCharacterID();
    // Pull the active memory pointer using your sign-bit stripping engine
    LSCChannel* channel = GetChannelByID(channelID);
    if (channel == nullptr) {
        _log(LSC__ERROR, "%s: Access alteration failed. Channel %i not loaded in memory registry.", \
        call.client->GetName(), channelID);
        return nullptr;
    }

    // 4. Security Check: Verify requesting client is an Operator or higher
    if (!channel->IsOperatorOrHigher(charID, call.client->GetCorporationID(), call.client->GetCorpRole())) {
        _log(LSC__ERROR, "Character %u attempted unauthorized ACL modification in room %d.", charID, channelID);
        return nullptr;
    }

    // 5. Commit state change straight to the pointer-stabilized memory frames
    // This updates internal roles, processes live player kicks, and fires the 5-element OnLSC broadcast
/*
    channel->UpdateAccessControl(
        call.client,
        args.accessorID,
        args.mode,
        args.reason,
        args.untilWhen
    );
*/
    return nullptr;
}

PyResult LSCService::Handle_Invite(PyCallArgs &call) {
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.Warning("LSCService::Handle_Invite()", "Manual invite packet array size=%zu", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    CallInvite args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode Invite arguments.", GetName());
        return nullptr;
    }

    // 2. Unpack Raw Position Items safely to catch Python inputs without narrowing errors
    int32 invited_char_id_raw = 0;
    int32 channelID_raw = 0;

    if (call.tuple->GetItem(0)->IsInt()) {
        invited_char_id_raw = PyRep::IntegerValueI32(call.tuple->GetItem(0));
    } else {
        _log(LSC__ERROR, "%s: call.tuple->GetItem(0) is of the wrong type: '%s'. Expected PyInt.", call.client->GetName(), call.tuple->TypeString());
        return nullptr;
    }

    if (call.tuple->GetItem(1)->IsInt()) {
        channelID_raw = PyRep::IntegerValueI32(call.tuple->GetItem(1));
    } else {
        _log(LSC__ERROR, "%s: call.tuple->GetItem(1) is of the wrong type: '%s'. Expected PyInt.", call.client->GetName(), call.tuple->TypeString());
        return nullptr;
    }

    // Enforce internal unsigned rules choice for server tracking keys
    int32 callerCharID = call.client->GetCharacterID();
    uint32 invitedCharID = static_cast<uint32>(invited_char_id_raw);

    // 3. OPTIMIZED LOOKUP: Sign-bit transformations handled automatically via GetChannelByID
    LSCChannel* channel = GetChannelByID(channelID_raw);
    if (channel == nullptr) {
        _log(LSC__ERROR, "%s: Invitation routing failed. Channel ID %i not found in register.", call.client->GetName(), channelID_raw);
        return nullptr;
    }

    // 4. ROSTER PRESENCE VALIDATION
    if (!channel->IsJoined(invitedCharID)) {
        // 5. ASSEMBLE CLIENT 'ChatInvite' WINDOW PAYLOAD
        // Matches client function signature: ChatInvite(invitorID, invitorName, invitorGender, channelID, uberdude)
        PyTuple* invitePayload = new PyTuple(5);
        invitePayload->SetItem(0, new PyInt(callerCharID));
        invitePayload->SetItem(1, new PyString(call.client->GetName()));
        invitePayload->SetItem(2, new PyInt(call.client->GetGender()));
        invitePayload->SetItem(3, channel->EncodeID());
        invitePayload->SetItem(4, PyStatic.NewZero()); // uberdude override

        // 6. TARGET SESSION DISPATCH
        Client* targetClient = sEntityMgr.FindClientByCharID(invitedCharID);
        if (targetClient != nullptr && targetClient->GetSession() != nullptr) {
            // Transmit the notification directly to the target character's Neocom hook pipeline
            targetClient->SendNotification("ChatInvite", channel->GetTypeString(), &invitePayload );

            _log(LSC__CHANNELS, "%s (%u) invited %s (%u) to Channel %i (\"%s\") via Unicast pipe.",
                 call.client->GetName(), callerCharID, targetClient->GetName(), invitedCharID, channel->GetChannelID(), channel->GetDisplayName().c_str());
        }
    }

    return PyStatic.NewOne();
}

PyResult LSCService::Handle_Configure(PyCallArgs& call) {
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.Warning("LSCService::Handle_Configure()", "Configuration array size=%zu", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }
    /*
     * 20:07:21 W LSCService::Handle_Configure(): Configuration array size=1
     * 20:07:21 [LSC CallDump]   Call Arguments:
     * 20:07:21 [LSC CallDump]      Tuple: 1 elements
     * 20:07:21 [LSC CallDump]       [ 0]    Integer: 21
     * 20:07:21 [LSC CallDump]  Named Arguments:
     * 20:07:21 [LSC CallDump]   machoVersion
     * 20:07:21 [LSC CallDump]        Integer: 1
     * 20:07:21 [LSC CallDump]   newPassword
     * 20:07:21 [LSC CallDump]           None
     * 20:07:21 [LSC CallDump]   oldPassword
     * 20:07:21 [LSC CallDump]         String: ''
     *
     * 20:08:41 [LSC CallDump]   Call Arguments:
     * 20:08:41 [LSC CallDump]      Tuple: 1 elements
     * 20:08:41 [LSC CallDump]       [ 0]  Tuple: 1 elements
     * 20:08:41 [LSC CallDump]       [ 0]   [ 0]  Tuple: 2 elements
     * 20:08:41 [LSC CallDump]       [ 0]   [ 0]   [ 0]     String: 'corpid'
     * 20:08:41 [LSC CallDump]       [ 0]   [ 0]   [ 1]    Integer: 98000001
     * 20:08:41 [LSC CallDump]  Named Arguments:
     * 20:08:41 [LSC CallDump]   machoVersion
     * 20:08:41 [LSC CallDump]        Integer: 1
     * 20:08:41 [LSC CallDump]   motd
     * 20:08:41 [LSC CallDump]        WString: 'test'
     */
    // 1. Structural Validation Guard
    if (!call.tuple->IsTuple() || call.tuple->size() < 1) {
        codelog(SERVICE__ERROR, "%s: Aborting malformed configuration packet payload array.", GetName());
        return nullptr;
    }

    if (call.byname.empty()) {
        _log(LSC__ERROR, "%s: Aborting configuration pass; byname map contains zero elements.", call.client->GetName());
        return nullptr;
    }

    int32 channelID = 0;
    PyRep* firstItem = call.tuple->AsTuple()->GetItem(0);

    if (firstItem->IsInt()) {
        channelID = PyRep::IntegerValueI32(firstItem);
    } else {
        _log(LSC__ERROR, "%s: Tuple index 0 contained invalid structural type: '%s'", call.client->GetName(), call.tuple->TypeString());
        return nullptr;
    }

    // 2. OPTIMIZED LOOKUP
    LSCChannel* channel = GetChannelByID(channelID);
    if (channel == nullptr) {
        _log(LSC__ERROR, "%s: Handle_Configure failed. Unable to locate active room for ID %i", call.client->GetName(), channelID);
        return nullptr;
    }

    // 3. ENFORCE AUTHORIZATION SAFETY GATES (Operators or higher matching client script)
    if (!channel->IsOperatorOrHigher(call.client->GetCharacterID(), call.client->GetCorporationID(), call.client->GetCorpRole())) {
        _log(LSC__ERROR, "Security Alert: Unauthorized Rename request by %i in channel %d", \
                    call.client->GetCharacterID(), channelID);
        throw UserError("LSCCannotSetMemberless")
            .AddFormatValue("msg", new PyString("You are not authorized to perform this change."));
        return nullptr;
    }

    // --- 4. EXPLICIT PROPERTY BROADCAST BLOCKS-
    bool configAltered = false;
    // Feature A: Display Name / Rename Updates
    auto it = call.byname.find("displayName");
    if (it != call.byname.end()) {
        std::string newName = PyRep::StringContent(it->second);
        channel->SetDisplayName(newName);
        configAltered = true;

        PyTuple* args = new PyTuple(1);
        args->SetItem(0, new PyString(newName));
        channel->BroadcastEvent("RenameChannel", args);
    }

    // Feature B: MOTD Banner Text Updates
    it = call.byname.find("motd");
    if (it != call.byname.end()) {
        std::string newMotd = PyRep::StringContent(it->second);
        channel->SetMOTD(newMotd);
        configAltered = true;

        PyTuple* args = new PyTuple(1);
        args->SetItem(0, new PyString(newMotd));
        channel->BroadcastEvent("SetChannelMOTD", args);
    }

    // Feature C: Password Management
    auto itOldPass = call.byname.find("oldPassword");
    auto itNewPass = call.byname.find("newPassword");
    if (itNewPass != call.byname.end()) {
        std::string oldPass = PyRep::StringContent(itOldPass->second);
        if (oldPass.empty() || channel->GetPassword() == oldPass) {
            if (itNewPass != call.byname.end()) {
                std::string newPass = PyRep::StringContent(itNewPass->second);
                channel->SetPassword(newPass);
                configAltered = true;

                PyTuple* args = new PyTuple(1);
                args->SetItem(0, new PyString(newPass));
                channel->BroadcastEvent("SetChannelPassword", args);
            }
        }
    }

    // Feature D: Memberless Status Swaps (Toggles visibility roster updates)
    it = call.byname.find("memberless");
    if (it != call.byname.end()) {
        bool memberlessVal = PyRep::GetBool(it->second);
        channel->SetMemberless(memberlessVal);
        configAltered = true;

        // args expects: [ bool(memberless), (CRowset(fallback_roster) or PyStatic.NewNone();) ]
        PyTuple* args = new PyTuple(2);
        args->SetItem(0, new PyBool(memberlessVal));

        if (memberlessVal) {
            args->SetItem(1, PyStatic.NewNone());
        } else {
            // Moving back to immediate mode requires providing the full roster index array
            args->SetItem(1, channel->EncodeChannelChars());
        }

        channel->BroadcastEvent("SetChannelMemberless", args);
    }

    if (configAltered)
        m_db->UpdateChannelInfo(channel);

    return nullptr;
}

// this has never worked right on live...lets see if we can do a bit better...
PyResult LSCService::Handle_ForgetChannel(PyCallArgs& call) {
    if (call.tuple->size() < 1 || !call.tuple->GetItem(0)->IsInt()) {
        return nullptr;
    }

    int32 channelID = PyRep::IntegerValueI32(call.tuple->GetItem(0));
    uint32 charID = call.client->GetCharacterID();

    LSCChannel* pChannel = GetChannelByID(channelID);
    if (pChannel != nullptr) {
        _log(LSC__CHANNELS, "%s(%u) requested purge for channel %i.", call.client->GetName(), charID, channelID);
        m_db->ForgetChannel(charID, channelID);
    }

    return nullptr;
}

PyResult LSCService::Handle_RenameChannel(PyCallArgs& call) {
    if (call.tuple->size() < 2 || !call.tuple->GetItem(0)->IsInt() || !call.tuple->GetItem(1)->IsString()) {
        return nullptr;
    }

    int32 channelID = PyRep::IntegerValueI32(call.tuple->GetItem(0));
    std::string newName = PyRep::StringContent(call.tuple->GetItem(1));
    uint32 charID = call.client->GetCharacterID();

    LSCChannel* pChannel = GetChannelByID(channelID);
    if (pChannel == nullptr || !pChannel->IsOperatorOrHigher(charID, call.client->GetCorporationID(), call.client->GetCorpRole())) {
        return nullptr;
    }

    // Commit change to live memory
    pChannel->SetDisplayName(newName);
    m_db->UpdateChannelInfo(pChannel); // Strict MariaDB 10.0.3 VALUES() statement inside

    // Broadcast the change using the critical 6-element identity payload block
    PyTuple* args = new PyTuple(1);
    args->SetItem(0, new PyString(newName));
/*
    PyTuple* whoObj = pChannel->BuildWhoObject(call.client);
    pChannel->BroadcastEvent("RenameChannel", whoObj, args);
*/

    return PyStatic.NewNone();
}

PyResult LSCService::Handle_SetChannelMOTD(PyCallArgs& call) {
    if (call.tuple->size() < 2 || !call.tuple->GetItem(0)->IsInt()) {
        return nullptr;
    }

    int32 channelID = PyRep::IntegerValueI32(call.tuple->GetItem(0));
    uint32 charID = call.client->GetCharacterID();

    LSCChannel* pChannel = GetChannelByID(channelID);
    if (pChannel == nullptr || !pChannel->IsOperatorOrHigher(charID, call.client->GetCorporationID(), call.client->GetCorpRole())) {
        return nullptr;
    }

    // Safely support either string forms or a clear action
    std::string newMotd = PyRep::StringContent(call.tuple->GetItem(1));
    // Commit change to live memory
    pChannel->SetMOTD(newMotd);
    m_db->UpdateChannelInfo(pChannel);

    // Broadcast change
    //PyTuple* whoObj = pChannel->BuildWhoObject(call.client);
    PyTuple* args = new PyTuple(1);

    if (newMotd.empty()) {
        // Signals a target room MOTD clear action
        args->SetItem(0, PyStatic.NewNone());
    } else {
        args->SetItem(0, new PyString(newMotd));
    }

    pChannel->BroadcastEvent("SetChannelMOTD", args);

    return PyStatic.NewNone();
}

PyResult LSCService::Handle_SetChannelLanguageRestriction(PyCallArgs& call) {
    if (is_log_enabled(LSC__CALL_DUMP)) {
        call.Dump(LSC__CALL_DUMP);
    }

    // 1. Minimum Size Validation Check: Expects position 0 (channelID) and position 1 (langCode)
    if (call.tuple->size() < 2 || !call.tuple->GetItem(0)->IsInt() || !call.tuple->GetItem(1)->IsInt()) {
        codelog(SERVICE__ERROR, "%s: Aborting malformed SetChannelLanguageRestriction RPC payload.", GetName());
        return nullptr;
    }

    int32 channelID = PyRep::IntegerValueI32(call.tuple->GetItem(0));
    bool langRestriction = PyRep::GetBool(call.tuple->GetItem(1));
    uint32 charID = call.client->GetCharacterID();

    // 2. Authoritative Cache Fetch
    LSCChannel* pChannel = GetChannelByID(channelID);
    if (pChannel == nullptr) {
        _log(LSC__ERROR, "%s: Language restriction swap failed. Channel %d not active.", call.client->GetName(), channelID);
        return nullptr;
    }

    // 3. Corporate & ACL Security Gate Evaluation
    if (!pChannel->IsOperatorOrHigher(charID, call.client->GetCorporationID(), call.client->GetCorpRole())) {
        _log(LSC__ERROR, "Security Violation: Character %u lacks rights to modify restrictions in room %d.", charID, channelID);
        return nullptr;
    }

    // 4. Update the core room attribute cache
    pChannel->SetLanguageRestriction(langRestriction);

    // Persist change using strict MariaDB 10.0.3 ON DUPLICATE KEY UPDATE VALUES() regulations
    m_db->UpdateChannelInfo(pChannel);

    // args payload expects an explicit 1-element tuple containing the active restriction integer
    PyTuple* args = new PyTuple(1);
    args->SetItem(0, new PyBool(langRestriction));

    // Fire zero-clone multicast blast downstream to room occupants instantly
    pChannel->BroadcastEvent("SetChannelLanguageRestriction", args);

    // Return explicit void PyResult mapping to keep the MachoNet gateway synchronized
    return PyStatic.NewNone();
}


PyResult LSCService::Handle_LeaveChannel(PyCallArgs &call) {
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White("LSCService::Handle_LeaveChannel()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    CallLeaveChannel arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return PyStatic.NewNone();
    }

    int32 toLeave = 0;

    if (arg.channel->IsInt()) {
        toLeave = PyRep::IntegerValueI32(arg.channel);
    } else if (arg.channel->IsTuple()) {
        PyTuple* prt = arg.channel->AsTuple();

        if (prt->GetItem(0)->IsInt()) {
            toLeave = PyRep::IntegerValueI32(prt->GetItem(0));
        } else if (prt->GetItem(0)->IsTuple()) {
            prt = prt->GetItem(0)->AsTuple();

            if (prt->items.size() != 2 or !prt->GetItem(1)->IsInt()) {
                codelog(SERVICE__ERROR, "%s: Failed to decode arguments", call.client->GetName());
                return PyStatic.NewNone();
            }

            toLeave = PyRep::IntegerValueI32(prt->GetItem(1));
        } else {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments", call.client->GetName());
            return PyStatic.NewNone();
        }
    } else {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments", call.client->GetName());
        return PyStatic.NewNone();
    }

    if (arg.unsubscribe)
        m_db->DeleteSubscription(toLeave,call.client->GetCharacterID());

    std::unordered_map<int32, LSCChannel*>::iterator itr = m_channels.find(toLeave);
    if (itr != m_channels.end()) {
        itr->second->LeaveChannel(call.client);
        if ((itr->second->GetMemberCount() < 1) and (itr->second->GetTemporary())) {
            m_db->DeleteChannel(toLeave);
            SafeDelete(itr->second);
            m_channels.erase(itr);
        }
    }

    return PyStatic.NewNone();
}


PyResult LSCService::Handle_LeaveChannels(PyCallArgs &call) {
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White("LSCService::Handle_LeaveChannels()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    CallLeaveChannels args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return PyStatic.NewNone();
    }

    int32 charID = call.client->GetCharacterID();
    int32 channelID = 0;;
    PyList::const_iterator cur = args.channels->begin();
    for (; cur != args.channels->end(); ++cur) {
        if ((*cur)->IsInt()) {
            channelID = PyRep::IntegerValueI32(*cur);
        } else if ((*cur)->IsTuple()) {
            PyTuple* prt = (*cur)->AsTuple();

            if (prt->GetItem(0)->IsInt()) {
                channelID = PyRep::IntegerValueI32(prt->GetItem(0));
                continue;
            }

            if (!prt->GetItem(0)->IsTuple()) {
                codelog(SERVICE__ERROR, "%s: Failed to decode arguments", call.client->GetName());
                continue;
            }
            prt = prt->GetItem(0)->AsTuple();

            if (prt->GetItem(0)->IsTuple())
                prt = prt->GetItem(0)->AsTuple();

            if (prt->size() != 2 or !prt->GetItem(1)->IsInt()) {
                codelog(SERVICE__ERROR, "%s: Failed to decode arguments", call.client->GetName());
                continue;
            }

            channelID = PyRep::IntegerValueI32(prt->GetItem(1));
        } else {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments", call.client->GetName());
            continue;
        }

        // Strip the tracking key out of your central server register map securely
        // We use the absolute, positive channelID index key to erase cleanly
        int32 targetChannelID = (channelID < 0) ? (channelID & 0x7FFFFFFF) : channelID;
        if (targetChannelID == 0)
            continue;

        if (args.unsubscribe)
            m_db->DeleteSubscription(targetChannelID, charID);

        // 5. OPTIMIZED LOOKUP: Sign-bit transformations handled automatically via GetChannelByID
        LSCChannel* pChannel = GetChannelByID(targetChannelID);
        if (pChannel != nullptr) {
            // Let the room execute pointer removal and broadcast the LSC_LeaveChannel multicast
            pChannel->LeaveChannel(call.client);
            // 6. SECURE AUTOMATED TEARDOWN: If the channel is temporary and population hits absolute zero
            if (pChannel->GetMemberCount() < 1 && pChannel->GetTemporary()) {
                _log(LSC__CHANNELS, "Dynamic Mass-Sweep: Channel %i is completely vacant.", targetChannelID);
                DeleteSystemChannel(targetChannelID);
            }
        }
    }

    return PyStatic.NewNone();
}


PyResult LSCService::Handle_DestroyChannel(PyCallArgs& call)
{
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White("LSCService::Handle_DestroyChannel()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode DestroyChannel numeric arguments.", GetName());
        return nullptr;
    }

    int32 channelID = arg.arg;
    int32 characterID = call.client->GetCharacterID();

    auto itr = m_channels.find(channelID);
    if (itr == m_channels.end() || itr->second == nullptr) {
        _log(LSC__ERROR, "%s: Handle_DestroyChannel failed. Channel ID %i not found.",
             call.client->GetName(), channelID);
        return nullptr;
    }

    LSCChannel* pChannel = itr->second;
    if (pChannel->GetOwnerID() != characterID) {
        if ((call.client->GetAccountRole() & Acct::Role::EPLAYER) != Acct::Role::EPLAYER) {
            _log(LSC__ERROR, "Security Alert: Character %u unauthorized hard-destruction attempt on Channel ID %i!",
                 characterID, channelID);

            throw UserError("LSCCannotDestroy");
            return nullptr;
        }
    }

    pChannel->Evacuate(call.client);
    //channel->BroadcastDestroyChannel();
    m_db->DeleteChannel(pChannel->GetChannelID());

    m_channels.erase(itr);
    std::string compKey = pChannel->GetComparisonKey();
    auto stringMapIt = m_channelNameMap.find(compKey);
    if (stringMapIt != m_channelNameMap.end())
        m_channelNameMap.erase(stringMapIt);

    SafeDelete(pChannel);

    return PyStatic.NewNone();
}


PyResult LSCService::Handle_GetMembers(PyCallArgs &call) {
    // must return None on fail or empty
    CallGetMembers arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode bulk GetMembers network arguments.", GetName());
        return PyStatic.NewNone();
    }

    int32 channelID = 0;
    if (arg.channel->IsInt()) {
        channelID = PyRep::IntegerValueI32(arg.channel);
    } else if (arg.channel->IsTuple()) {
        PyTuple* prt = arg.channel->AsTuple();

        if (prt->GetItem(0)->IsInt()) {
            channelID = PyRep::IntegerValueI32(prt->GetItem(0));
        } else if (prt->GetItem(0)->IsTuple()) {
            prt = prt->GetItem(0)->AsTuple();

            if (prt->items.size() != 2 || !prt->GetItem(1)->IsInt()) {
                _log(SERVICE__ERROR, "%s: Aborting malformed channel sub-tuple structural footprint parsing block.", GetName());
                return PyStatic.NewNone();
            }
            channelID = PyRep::IntegerValueI32(prt->GetItem(1));
        } else {
            _log(SERVICE__ERROR, "%s: Aborting unrecognized nested structural footprint layout variant.", GetName());
            return PyStatic.NewNone();
        }
    } else {
        _log(SERVICE__ERROR, "%s: Unrecognized variant network token data type sent by client graph layers.", GetName());
        return PyStatic.NewNone();
    }

    LSCChannel* pChannel = GetChannelByID(channelID);
    if (pChannel != nullptr) {
        return pChannel->EncodeChannelChars();
    }

    return PyStatic.NewNone();
}

PyResult LSCService::Handle_GetMember(PyCallArgs &call) {
    if (!call.tuple->IsTuple() || call.tuple->size() < 2) {
        codelog(SERVICE__ERROR, "%s: Malformed individual GetMember network argument tuple array.", GetName());
        return nullptr;
    }

    int32 channelID = PyRep::IntegerValueI32(call.tuple->GetItem(0));
    int32 targetCharID = PyRep::IntegerValueI32(call.tuple->GetItem(1));

    LSCChannel* pChannel = GetChannelByID(channelID);
    if (pChannel != nullptr) {
        auto it = pChannel->m_chars.find(targetCharID);
        if (it != pChannel->m_chars.end() /*&& it->second != nullptr*/) {
            return it->second.Encode();
        }
    }

    return PyStatic.NewNone();
}

void LSCService::CreateSystemChannel(int32 channelID) {
    if (channelID == 0)
        return;
    if (m_channels.find(channelID) != m_channels.end()) {
        _log(LSC__WARNING, "Existing Channel %i called CreateSystemChannel().", channelID);
        return;
	}

    LSC::Type type   = LSC::Type::normal;
    int32 gMsgID     = LSC::gID::System2;
    int32 cMsgID     = LSC::cID::System;
    uint32 ownerID   = ownerSystem;
    std::string name = "", motd = "";

    if (IsStaticChannel(channelID)) {
        gMsgID    = LSC::gID::System2;
        name = "Static System Channel";
        motd = "Static System Channel";
        _log(LSC__WARNING, "Static Channel %i called CreateSystemChannel().", channelID);
    } else if (IsRegionID(channelID)) {
        type = LSC::Type::region;
        name = sDataMgr.GetLocationName(channelID);
        motd = "Region Channel";
    } else if (IsConstellationID(channelID)) {
        type = LSC::Type::constellation;
        name = sDataMgr.GetLocationName(channelID);
        motd = "Constellation Channel";
    } else if (IsKSpaceID(channelID)) {
        type = LSC::Type::solarsystem2;
        name = sDataMgr.GetLocationName(channelID);
        motd = "Known Solar System Channel";
    } else if (IsWSpaceID(channelID)) {
        type = LSC::Type::solarsystem;
        name = sDataMgr.GetLocationName(channelID);
        motd = "Unknown Solar System Channel";
    } else {
        // not sure what to do here...try dynamic
        CreateDynamicChannel(channelID);
        return;
    }

    std::string compKey = name;
    boost::algorithm::trim(compKey);
    boost::algorithm::to_lower(compKey);
    compKey.erase(std::remove(compKey.begin(), compKey.end(), ' '), compKey.end());
    //compKey.erase(std::remove(compKey.begin(), compKey.end(), ':'), compKey.end());
    //compKey.erase(std::remove(compKey.begin(), compKey.end(), '\''), compKey.end());
    LSCChannel* pChannel = new LSCChannel(this, channelID, ownerID, type, motd.c_str(), name.c_str(), compKey.c_str(), gMsgID, cMsgID);
    if (pChannel != nullptr) {
        m_channels.emplace(channelID, pChannel);
        m_channelNameMap.emplace(compKey, channelID);
    }
}

void LSCService::CreateStaticChannel(int32 channelID, int32 ownerID/*ownerSystem*/, LSC::Type type/*LSC::Type::normal*/, \
                                    const char* motd/*nullptr*/, const char* displayName/*nullptr*/, \
                                    int32 gMsgID/*0*/, int32 cMsgID/*0*/)
{
    if (channelID == 0)
        return;
    if (m_channels.find(channelID) != m_channels.end()) {
        _log(LSC__WARNING, "Existing Channel %i called CreateStaticChannel().", channelID);
        return;
    }

    if ((displayName == nullptr)) {
        LSCChannel* pChannel = new LSCChannel(this, channelID, ownerID, type, motd, nullptr, nullptr, gMsgID, cMsgID);
        if (pChannel != nullptr)
            m_channels.emplace(channelID, pChannel);
    } else {
        std::string compKey = displayName;
        boost::algorithm::trim(compKey);
        boost::algorithm::to_lower(compKey);
        compKey.erase(std::remove(compKey.begin(), compKey.end(), ' '), compKey.end());
        //compKey.erase(std::remove(compKey.begin(), compKey.end(), ':'), compKey.end());
        //compKey.erase(std::remove(compKey.begin(), compKey.end(), '\''), compKey.end());
        // hack for showing faction channels by name in main channel list
        if (cMsgID == LSC::cID::System)
            displayName = nullptr;
        LSCChannel* pChannel = new LSCChannel(this, channelID, ownerID, type, motd, displayName, compKey.c_str(), gMsgID, cMsgID);
        if (pChannel != nullptr) {
            m_channels.emplace(channelID, pChannel);
            m_channelNameMap.emplace(compKey, channelID);
        }
    }
}

LSCChannel* LSCService::CreateDynamicChannel(int32 channelID,
                                      uint32 ownerID/*0*/,
                                      std::string name/*nil*/,
                                      std::string motd/*nil*/,
                                      std::string password/*nil*/,
                                      bool memberless/*false*/,
                                      bool temporary/*false*/)
{
    if (channelID == 0)
        return nullptr;

    auto it = m_channels.find(channelID);
    if (it != m_channels.end() && it->second != nullptr) {
        _log(LSC__CHANNELS, "ChannelID %i found. Returning existing pointer.", channelID);
        return it->second;
    }

    bool initOwner  = false;
    LSC::Type type  = LSC::Type::normal;
    int32 gMsgID    = LSC::gID::None;
    int32 cMsgID    = LSC::cID::None;
    // override for private
    if (channelID < 0) {
        // 2. Map-Key Normalization Pass: Isolate the absolute positive map key identifier
    	int32 actualChannelID = (channelID & 0x7FFFFFFF);
        channelID = actualChannelID;
        temporary = true;
        type   = LSC::Type::custom;
     	gMsgID = LSC::gID::Player;
    	cMsgID = LSC::cID::Unspecified;
        initOwner = true;
        if (ownerID == 0)
            _log(LSC__WARNING, "ownerID = 0 for private channel %i", channelID);
    } else if (IsPlayerCorp(channelID)) {
        type = LSC::Type::corp;
        name = m_db->GetCorporationName(channelID);
        motd = name + "'s Corporation channel.";
        gMsgID = LSC::gID::Player;
        cMsgID = LSC::cID::Corporation;
        initOwner = true;
        if (ownerID == 0)
            ownerID = CorporationDB::GetCorpCEO(channelID);
    } else if (IsAllianceID(channelID)) {
        type = LSC::Type::alliance;
        name = m_db->GetAllianceName(channelID);
        motd = name + "'s Alliance channel.";
        gMsgID = LSC::gID::Player;
        cMsgID = LSC::cID::Alliance;
        initOwner = true;
        if (ownerID == 0)
            ownerID = channelID;
    } else if (IsNPCCorp(channelID)) {
        type = LSC::Type::corp;
        name = sDataMgr.GetCorpName(channelID);
        motd = "Welcome to " + name + "'s Corporation channel.";
        gMsgID = LSC::gID::Corporate;
        cMsgID = LSC::cID::Corporation;
        initOwner = true;
        if (ownerID == 0)
            ownerID = CorporationDB::GetCorpCEO(channelID);
    } else if (IsFactionID(channelID)) {
        type = LSC::Type::warfaction;
        name = sDataMgr.GetFactionName(channelID);
        gMsgID = LSC::gID::System2;
        cMsgID = LSC::cID::Faction;
        initOwner = true;
        if (ownerID == 0)
            ownerID = channelID;
    } else if (IsFleetID(channelID)) {
        type = LSC::Type::fleet;
        name = sFltSvc.GetFleetName(channelID);
        motd = sFltSvc.GetFleetDescription(channelID);
        gMsgID = LSC::gID::Player;
        cMsgID = LSC::cID::Other;
        initOwner = true;
        if (ownerID == 0)
            ownerID = sFltSvc.GetFleetLeaderID(channelID);
    } else if (IsWingID(channelID)) {
        type = LSC::Type::wing;
        name = sFltSvc.GetWingName(channelID);
        motd = name + "<br>" + sFltSvc.GetFleetDescription(channelID);
        gMsgID = LSC::gID::Player;
        cMsgID = LSC::cID::Other;
        initOwner = true;
        if (ownerID == 0)
            ownerID = sFltSvc.GetWingLeaderID(channelID);
    } else if (IsSquadID(channelID)) {
        type = LSC::Type::squad;
        name = sFltSvc.GetSquadName(channelID);
        motd = name + "<br>" + sFltSvc.GetFleetDescription(channelID);
        gMsgID = LSC::gID::Player;
        cMsgID = LSC::cID::Other;
        initOwner = true;
        if (ownerID == 0)
            ownerID = sFltSvc.GetSquadLeaderID(channelID);
    } else if (IsCharacterID(channelID)) {
        type = LSC::Type::character;
        gMsgID = LSC::gID::Player;
        cMsgID = LSC::cID::Character;
        ownerID = channelID;
        initOwner = true;
    } else {
        // not sure what to do here....should never hit
        sLog.Error("LSC::CreateDynamicChannel", "Called to create channel %i.", channelID);
        return nullptr;
    }

    // determine if name should be populated here for correct title display
    LSCChannel* pChannel = nullptr;
    if (name.empty()) {
        pChannel = new LSCChannel(this, channelID, ownerID, type, motd.c_str(), nullptr, nullptr,\
                                      gMsgID, cMsgID, memberless, temporary);
        if (pChannel != nullptr)
            m_channels.emplace(channelID, pChannel);
    } else {
        std::string compKey = name;
        boost::algorithm::trim(compKey);
        boost::algorithm::to_lower(compKey);
        compKey.erase(std::remove(compKey.begin(), compKey.end(), ' '), compKey.end());
        //compKey.erase(std::remove(compKey.begin(), compKey.end(), ':'), compKey.end());
        //compKey.erase(std::remove(compKey.begin(), compKey.end(), '\''), compKey.end());
        pChannel = new LSCChannel(this, channelID, ownerID, type, motd.c_str(), name.c_str(), \
                                    compKey.c_str(), gMsgID, cMsgID, memberless, temporary);
        if (pChannel != nullptr) {
            m_channels.emplace(channelID, pChannel);
            m_channelNameMap.emplace(compKey, channelID);
        }
    }

    if (initOwner and (pChannel != nullptr))
        pChannel->InitACL(ownerID);

    return pChannel;
}

void LSCService::CreateACLHeader() {
    m_accessHeader = new DBRowDescriptor();
    m_accessHeader->AddColumn("accessor",           DBTYPE_I4);
    m_accessHeader->AddColumn("mode",               DBTYPE_I4);
    m_accessHeader->AddColumn("untilWhen",          DBTYPE_I8);
    m_accessHeader->AddColumn("originalMode",       DBTYPE_I4);
    m_accessHeader->AddColumn("quantity",           DBTYPE_I4);
    m_accessHeader->AddColumn("reason",             DBTYPE_STR);
    m_accessHeader->AddColumn("admin",              DBTYPE_STR);
}

void LSCService::CreateChannelHeader() {
    m_channelHeader = new DBRowDescriptor();
    m_channelHeader->AddColumn("channelID",             DBTYPE_I4);
    m_channelHeader->AddColumn("ownerID",               DBTYPE_I4);
    m_channelHeader->AddColumn("displayName",           DBTYPE_STR);
    m_channelHeader->AddColumn("motd",                  DBTYPE_STR);
    m_channelHeader->AddColumn("comparisonKey",         DBTYPE_STR);
    m_channelHeader->AddColumn("memberless",            DBTYPE_BOOL);
    m_channelHeader->AddColumn("password",              DBTYPE_STR);
    m_channelHeader->AddColumn("mailingList",           DBTYPE_BOOL);
    m_channelHeader->AddColumn("cspa",                  DBTYPE_I4);
    m_channelHeader->AddColumn("temporary",             DBTYPE_BOOL);
    m_channelHeader->AddColumn("languageRestriction",   DBTYPE_BOOL);
    m_channelHeader->AddColumn("groupMessageID",        DBTYPE_I4);
    m_channelHeader->AddColumn("channelMessageID",      DBTYPE_I4);
    m_channelHeader->AddColumn("subscribed",            DBTYPE_I4);
    m_channelHeader->AddColumn("estimatedMemberCount",  DBTYPE_I4);
    //m_channelHeader->AddColumn("mode",                  DBTYPE_I4);
}

void LSCService::CreateStaticChannels() {
    // hardcode creating server static channels during server startup
    // these are all set to memberless to avoid constant updates
    std::ostringstream str;

    //CreateStaticChannel(channelID, ownerID, type, motd, displayName, gMsgID, cMsgID)

    // ==========================================
    // --- CATEGORY 1: GLOBAL HELP CHANNELS ---
    // ==========================================

    // Static Room ID 1: Rookie Help
    str.str("");
    str << "<color=0xff007fff><b>Welcome to the Alasiya EVE Online: Crucible Emulator</color>";
    str << "<color=0xff00ff00>Chatrules</color><color=0xffffffff>: </color>";
    str << "<color=0xffffa500><url=http://www.eveonline.com/pnp/chatrules.asp><u>http://www.eveonline.com/pnp/chatrules.asp </color></url></u>";
    str << "<color=0xffb2b2b2>please read and observe them.</color><br>";
    str << "<color=0xff00ff00>Topic</color>";
    str << "<color=0xffffffff>: Stay on topic of EVE-Online related rookie help.<br></color>";
    str << "<color=0xff00ff00>Rules: </color>";
    str << "<color=0xffffffff>No WTB, WTS, WTT (aka trading, selling, w.e.), PC, advertising, recruiting, scamming, offering private help in any form or begging in this channel.";
    str << "No CAPS or text-decoration either<br></color>";
    str << "<color=0xff00ff00>Language</color><color=0xffffffff>: This channel is ENGLISH ONLY.<br><br></color>";
    str << "<color=0xffffff00>ISK Advertising: Contrary to what they spam, Alasiya does not authorize any person or site to sell ISK for RL cash!<br><br></color>";
    str << "<color=0xff00ff00>Recommended reading</color>";
    str << "<color=0xffffffff>: </color><color=0xffffa500><url=http://wiki.eveonline.com/wiki/Category:New_Player_Experience><u>http://wiki.eveonline.com/wiki/Category:New_Player_Experience</color>";
    str << "<color=0xff007fff></url></b></u> <br><br></color>";
    str << "<color=0xff00ff00><b>Please Note:</color>";
    str << "<color=0xffffffff> There are no third party applications (.exe) that will magically give you any type of ship you wish or hack your wallet. Please report characters advertising these types of links IMMEDIATELY via petition and DO NOT download and try them. <br><br>";
    str << "Sister of EVE storyline starts in <url=showinfo:5//30005001><u>Arnon</url></b></u> <b>with <url=showinfo:1378//3019356><u>Sister Alitura</url></b></u>. <br><br></color>";
    str << "<color=0xff00ff00><b>Before asking, please read: </color>";
    str << "<color=0xffffa500><url=http://wiki.eveonline.com/wiki/Rookie_Help_Channel_FAQ><u>http://wiki.eveonline.com/wiki/Rookie_Help_Channel_FAQ</color></url></b></u><br><br>";
    CreateStaticChannel(1, ownerSystem, LSC::Type::normal, str.str().c_str(), "Rookie Help", LSC::gID::Help, LSC::cID::Rookie);

    // Static Room ID 2: English Help
    str.str("");
    str << "<color=0xffff0000><b>Welcome to The Alasiya EVE Online: Crucible Emulator</color>";
    str << "<color=0xff007fff> <br><br></color><color=0xff00ff00>Player Guides:</color>";
    str << "<color=0xffffa500><url=http://wiki.eveonline.com/wiki/Category:New_Player_Experience><u>http://wiki.eveonline.com/wiki/Category:New_Player_Experience</color></url></u>";
    str << "<color=0xffb2b2b2><b>Please: Stay on topic. </color>";
    str << "<color=0xffffff00>No offtopic, WTB, WTS, PC, advertising, recruiting, scamming/trading in general or begging in this channel. No CAPS or text-decoration.<br></color>";
    str << "<color=0xff00ff00>Language:</color><color=0xffb2b2b2>This channel is</color><color=0xffff0000>ENGLISH ONLY!!<br></color>";
    str << "<color=0xff00ff00>How to contact a GM: </color>";
    str << "<color=0xffb2b2b2>File a petition (F12 - Petitions - New Petition) <br></color>";
    str << "<color=0xff00ff00><b>The topic of this channel is EVE related help.</color></b>";
    CreateStaticChannel(2, ownerSystem, LSC::Type::normal, str.str().c_str(), "English Help", LSC::gID::Help, LSC::cID::EngHelp);

    // Static Room ID 40: German Help
    // Static Room ID 55: Russian Help
    // Static Room ID 56: Japanese Help

    // ==========================================
    // --- CATEGORY 2: EMPIRE/FACTION ---
    // ==========================================
    str.str("");
    str << "<color=0xffffffff>Welcome to the Faction channel.<br>";
    CreateStaticChannel(10, ownerUnknown/*263268*/, LSC::Type::normal, str.str().c_str(), "Caldari", LSC::gID::Empire, LSC::cID::None);
    CreateStaticChannel(11, ownerUnknown/*263246*/, LSC::Type::normal, str.str().c_str(), "Amarr", LSC::gID::Empire, LSC::cID::None);
    CreateStaticChannel(12, ownerUnknown/*263281*/, LSC::Type::normal, str.str().c_str(), "Minmatar", LSC::gID::Empire, LSC::cID::None);
    CreateStaticChannel(13, ownerUnknown/*263271*/, LSC::Type::normal, str.str().c_str(), "Gallente", LSC::gID::Empire, LSC::cID::None);
    //CreateStaticChannel(14, ownerUnknown/*263258*/, LSC::Type::normal, str.str().c_str(), "Jove", LSC::gID::Empire, LSC::cID::None);

    // ==========================================
    // --- CATEGORY 3: CORPORATE  ---
    // ==========================================
    str.str("");
    str << "<color=0xffffffff>Welcome to the recruitment channel.<br>";
    str << "This channel is intended for those players looking to find a new corporation, as well as those looking to enlist new players.<br>";
    str << "Other activities, such as non-recruitment discussion and scamming, are not permitted in this channel.";
//Corporate
    CreateStaticChannel(20, ownerSystem/*263286*/, LSC::Type::normal, str.str().c_str(), "Recruitment", LSC::gID::Corporate, LSC::cID::Recruitment);
    str.str("");
    str << "<color=0xffffffff>Welcome to this</color> <color=0xff00ffff>Corporate CEO</color><color=0xffffffff> channel.<br>";
    str << "This channel is intended for corp CEOs to discuss business as they see fit.</color>";
    CreateStaticChannel(21, ownerSystem/*263287*/, LSC::Type::normal, str.str().c_str(), "CEO", LSC::gID::Corporate, LSC::cID::CEO);
//Alliance
    str.str("");
    str << "<color=0xffffffff>Welcome to this</color> <color=0xff00ffff>Alliance</color><color=0xffffffff>  channel.  This channel is intended for corp CEOs to discuss business as they see fit.</color>";
    CreateStaticChannel(22, ownerSystem/*263287*/, LSC::Type::normal, str.str().c_str(), "Alliance", LSC::gID::Corporate, LSC::cID::Alliance);

    // misc
    str.str("");
    str << "This channel is intended for those players looking to discuss the topic referenced in the channel title.</color>";
    CreateStaticChannel(31, ownerSystem, LSC::Type::normal, str.str().c_str(), "Smacktalk", LSC::gID::Misc, LSC::cID::Smacktalk);
    CreateStaticChannel(32, ownerSystem, LSC::Type::normal, str.str().c_str(), "Rumor", LSC::gID::Misc, LSC::cID::Rumor);
    CreateStaticChannel(33, ownerUnknown, LSC::Type::normal, str.str().c_str(), "Misc", LSC::gID::Misc, LSC::cID::None);

    // ==========================================
    // --- CATEGORY 4: REGIONAL MARKET/TRADE/OTHER ---
    // ==========================================
    str.str("");
    str << "<color=0xffffffff>Welcome to this</color> <color=0xff00ffff>Trade</color><color=0xffffffff> channel.<br>";
    str << "This channel is intended for those players looking to trade the various items as referenced in the channel title.</color>";
    CreateStaticChannel(41, ownerSystem/*263277*/, LSC::Type::normal, str.str().c_str(), "Blueprint Trade", LSC::gID::Trade, LSC::cID::Blueprints);
    CreateStaticChannel(42, ownerSystem/*263245*/, LSC::Type::normal, str.str().c_str(), "Industry Trade", LSC::gID::Trade, LSC::cID::Industry);
    CreateStaticChannel(43, ownerSystem/*263292*/, LSC::Type::normal, str.str().c_str(), "Material Trade", LSC::gID::Trade, LSC::cID::Manufacturing);
    CreateStaticChannel(44, ownerSystem/*263254*/, LSC::Type::normal, str.str().c_str(), "Other Trade", LSC::gID::Trade, LSC::cID::None);
    CreateStaticChannel(45, ownerSystem/*263275*/, LSC::Type::normal, str.str().c_str(), "Planetary Interaction Trade", LSC::gID::Trade, LSC::cID::PI);

    // ==========================================
    // --- CATEGORY 5: SCIENCE & INDUSTRY ---
    // ==========================================
    str.str("");
    str << "<color=0xffffffff>Welcome to this</color> <color=0xff00ffff>Science and Industry</color><color=0xffffffff> channel.<br>";
    str << "This channel is intended for those players looking to discuss the various items as referenced in the channel title.</color>";
    CreateStaticChannel(50, ownerSystem/*263365*/, LSC::Type::normal, str.str().c_str(), "SnI: Boosters", LSC::gID::SnI, LSC::cID::Boosters);
    CreateStaticChannel(51, ownerSystem/*263366*/, LSC::Type::normal, str.str().c_str(), "SnI: Invention", LSC::gID::SnI, LSC::cID::Invention);
    CreateStaticChannel(52, ownerSystem/*263367*/, LSC::Type::normal, str.str().c_str(), "SnI: Manufacturing", LSC::gID::SnI, LSC::cID::Manufacturing);
    CreateStaticChannel(53, ownerSystem/*263368*/, LSC::Type::normal, str.str().c_str(), "SnI: Mining", LSC::gID::SnI, LSC::cID::Mining);
    CreateStaticChannel(54, ownerSystem/*263370*/, LSC::Type::normal, str.str().c_str(), "SnI: Research", LSC::gID::SnI, LSC::cID::Research);
    CreateStaticChannel(58, ownerSystem/*263369*/, LSC::Type::normal, str.str().c_str(), "SnI: Planetary Interaction", LSC::gID::SnI, LSC::cID::PI);

    // ==========================================
    // --- CATEGORY 6: GAME CONTENT GROUPS ---
    // ==========================================
    str.str("");
    str << "<color=0xffffffff>Welcome to this</color> <color=0xff00ffff>Content</color><color=0xffffffff> channel.<br>";
    str << "This channel is intended for those players looking to discuss the various items as referenced in the channel title.</color>";
    CreateStaticChannel(60, ownerSystem/*263289*/, LSC::Type::normal, str.str().c_str(), "Mining", LSC::gID::Content, LSC::cID::Mining);
    CreateStaticChannel(61, ownerSystem/*263338*/, LSC::Type::normal, str.str().c_str(), "Ratting", LSC::gID::Content, LSC::cID::Ratting);
    CreateStaticChannel(62, ownerSystem/*263339*/, LSC::Type::normal, str.str().c_str(), "Scanning", LSC::gID::Content, LSC::cID::Scanning);
    CreateStaticChannel(63, ownerSystem/*263340*/, LSC::Type::normal, str.str().c_str(), "Wormholes", LSC::gID::Content, LSC::cID::Wormholes);
    CreateStaticChannel(64, ownerSystem, LSC::Type::normal, str.str().c_str(), "Manufacturing", LSC::gID::Content, LSC::cID::Manufacturing);
    CreateStaticChannel(65, ownerSystem, LSC::Type::normal, str.str().c_str(), "Invention", LSC::gID::Content, LSC::cID::Invention);
    CreateStaticChannel(66, ownerSystem, LSC::Type::normal, str.str().c_str(), "Blueprints", LSC::gID::Content, LSC::cID::Blueprints);


    // ==========================================
    // --- CATEGORY 7: COMMUNITY UTILITIES ---
    // ==========================================
    str.str("");
    str << "<br><color=0xffffffff>Welcome to the <url=http://eve.alasiya.net/phpBB3/viewtopic.php?f=30&t=296>Free Wrecks</url> channel.<br>";
    str << "Here you can offer your abandoned mission wrecks to any willing freelance salvager in New Eden.</color><br>";
    str << "<b><u><color=0xff00ffff>Salvagers</color></u><br><color=0xff00ff00>Alphas</color></b><br>";
    str << "<color=0xffffffff> - Basic salvage fits by race: </color><br>";
    str << "<url=fitting:16236:31083;2:25861;4:8135;1:31370;1:1319;2:4435;1:5973;1:24348;4::>Amarr</url><color=0xffffffff> - </color>";
    str << "<url=fitting:16238:25861;4:1319;2:31370;1:31083;2:4435;2:5973;1:24348;4::>Caldari</url><color=0xffffffff> - </color>";
    str << "<url=fitting:16240:31083;2:25861;4:8135;1:31370;1:1319;2:4435;1:5973;1:24348;4::>Gallente</url><color=0xffffffff> - </color>";
    str << "<url=fitting:16242:25861;4:1319;2:31370;1:31083;2:6001;1:4435;2:24348;4::>Minmatar</url><color=0xffffffff> </color><br>";
    str << "<b><color=0xff00ff00>Omegas</color></b><color=0xffffffff> - <i>You should be able to use a </color><url=showinfo:2998>Noctis</url>";
    str << "<color=0xffffffff>, </color><url=showinfo:30836>Salvager II's</url><color=0xffffffff> &amp; </color><url=showinfo:4250>Tractor Beam II's</url><br>";
    str << "<b><u><color=0xff00ffff>Mission Runners</color></b></u><br><color=0xffffffff>";
    str << "Please state where your bookmark will be traded, contracted or if youre hoping to fleet with the salvager looking for work. <br>";
    str << "If you are contracting or trading bookmarks dont forget to</color> <color=0xff007fff>Abandon all wrecks and containers</color><color=0xffffffff>.</color><br>";
    CreateStaticChannel(100, ownerUnknown, LSC::Type::normal, str.str().c_str(), "Free Wrecks", LSC::gID::None, LSC::cID::None);
    //256739 <-- messageID from error about "channel already joined" : "RuntimeError: Token has no value."

    str.str("");
    str << "<b><u><fontsize=10><color=0xff00ff00>Useful Links.</color></u><br>";
    str << "<loc><url=http://evemaps.dotlan.net/>DOTLAN</url></loc></b><color=0xffffffff> - A database of everything you need to know about New Eden; maps, corporations, navigations and much more.</color><color=0xcc111100>NOTE:</color><color=0xffffffff> While this site is specific for Tranquility, the maps and navigation are the same here.</color><br>";
    str << "<loc><url=http://o.smium.org>Osmium</url></loc><color=0xffffffff> - A site where pilots post their ship fittings to help players get the most out of their ship class.</color><br>";
    str << "<loc><url=http://eve-survival.org/wikka.php?wakka=MissionReports>EVE Survival</url></loc><color=0xffffffff> - A database of missions within New Eden. Here you can find information about gaining the upper hand on those sneaky NPCs and how to perfectly run the mission in question.</color><br>";
    str << "<loc><url=http://www.fuzzwork.co.uk/>Fuzz Work</url></loc><color=0xffffffff> - A brilliant site that has many awesome calculators for LP stores, Blueprints, Invention, Ore and much more!</color><br>";
    CreateStaticChannel(100, ownerUnknown, LSC::Type::normal, str.str().c_str(), "Useful Links", LSC::gID::Misc, LSC::cID::None);

    str.str("");
    str << "<br><color=0xffffffff>Welcome to the</color> <color=0xff00ffff>GM Command</color><color=0xffffffff> channel.<br><br>";
    str << "This channel is intended for using dot commands.</color>";
    CreateStaticChannel(staticGMChannel, ownerNone, LSC::Type::normal, str.str().c_str(), "Command", LSC::gID::Unspecified, LSC::cID::Command);
}
