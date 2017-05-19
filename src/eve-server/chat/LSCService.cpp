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
const uint32 LSCService::BASE_CHANNEL_ID = 200000000;
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
    std::map<uint32, LSCChannel* >::iterator cur = m_channels.begin();
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
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White( "LSCService::Handle_GetChannels()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    /*
        Assume this is only called when the char's logging in.
        Next step from the client is to join to all channels that's been sent by this
        So only send back the relevant channelIDs
        These are:
            - Help/Rookie, which is 1
            - character
            - corporation
            - solar system
            - region
            - constellation
            - allianceID
            - gangID
            - empireID
    */

    std::vector<unsigned long> charChannelIDs;
    std::vector<std::string> charChannelNames;
    std::vector<std::string> charChannelMOTDs;
    std::vector<unsigned long> charOwnerIDs;
    std::vector<std::string> charComparisonKeys;
    std::vector<int> charMemberless;
    std::vector<std::string> charPasswords;
    std::vector<int> charMailingLists;
    std::vector<int> charCSPAs;
    std::vector<int> charTemporaries;
    std::vector<int> charModes;
    int channelCount = 0;

    // Get this character's subscribed Private Channel names and IDs:
    m_db->GetChannelSubscriptions(call.client->GetCharacterID(), charChannelIDs, charChannelNames, charChannelMOTDs,
        charOwnerIDs, charComparisonKeys, charMemberless, charPasswords, charMailingLists, charCSPAs, charTemporaries,
        charModes, channelCount);

    if( channelCount > 0 )
    {
        // Check each private chat channel listed in the names/IDs just procurred to
        // see if they exist yet and if not, create them:
        for( int i=0; i<channelCount; i++ )
        {
                CreateChannel(
                    charChannelIDs[i],
                    charChannelNames[i].c_str(),
                    charChannelMOTDs[i].c_str(),
                    LSCChannel::normal,
                    charComparisonKeys[i].c_str(),
                    charOwnerIDs[i],
                    (charMemberless[i] ? true : false),
                    charPasswords[i].c_str(),
                    (charMailingLists[i] ? true : false),
                    charCSPAs[i],
                    charTemporaries[i],
                    charModes[i]
                    );
        }
    }

    ChannelInfo info;
    info.lines = new PyList();

    std::map<uint32, LSCChannel*>::iterator cur, end;
    cur = m_channels.begin();
    end = m_channels.end();
    for(; cur != end; cur++)
        info.lines->AddItem( cur->second->EncodeChannel( call.client->GetCharacterID() ) );

    if (is_log_enabled(LSC__RSP_DUMP))
        info.Dump(LSC__RSP_DUMP);

    return info.Encode();
}


PyResult LSCService::Handle_GetRookieHelpChannel(PyCallArgs &call) {
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White( "LSCService::Handle_GetRookieHelpChannel()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    return new PyInt(1);
}


PyResult LSCService::Handle_JoinChannels(PyCallArgs &call) {
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White( "LSCService::Handle_JoinChannels()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }


    CallJoinChannels args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    std::set<uint32> toJoin;

    PyList::const_iterator cur, end;
    cur = args.channels->begin();
    end = args.channels->end();

    for( ; cur != end; cur++ )
    {
        if( (*cur)->IsInt() )
            toJoin.insert( (*cur)->AsInt()->value() );
        else if( (*cur)->IsTuple() )
        {
            PyTuple* prt = (*cur)->AsTuple();

            if( prt->items.size() != 1 || !prt->items[0]->IsTuple() )
            {
                codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
                continue;
            }
            prt = prt->items[0]->AsTuple();

            if( prt->items.size() != 2 || /* !prt->items[0]->IsString() || unnessecary */ !prt->items[1]->IsInt() )
            {
                codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
                continue;
            }
            toJoin.insert( prt->items[1]->AsInt()->value() );
        }
        else
        {
            codelog(SERVICE__ERROR, "%s: Bad argument ", call.client->GetName());
            return nullptr;
        }
    }

    // ********** TODO **********
    // Figure out how to send the right packet to the client requesting the joining of this channel
    // to query that client for a password to join this channel if indeed there is a password required.
    // Check the password supplied by the client against the password stored in the LSCChannel object
    // or queried from the database if this channel has not had its own LSCChannel object created and
    // allow joining to this channel if passwords match.
    // **************************

    uint32 charID = call.client->GetCharacterID();
    // and now ensure the working of the system
    toJoin.insert( charID );

    PyList *ml = new PyList();

    std::set<uint32>::iterator curs, ends;
    curs = toJoin.begin();
    ends = toJoin.end();

    // Determine if the character is less than a month old
    // and, if so, then set a flag that causes joining this character to the Help\Help and
    // Help\Rookie channels.
    const bool isRookie = Win32TimeNow() < ( call.client->GetChar()->createDateTime() + Win32Time_Month );

    for( ; curs != ends; curs++ )
    {
        LSCChannel* channel;

        uint32 channelID = *curs;

        // Skip joining Help\Rookie and Help\Help channels when the character is no longer a rookie:
        if( isRookie || !( channelID == 1 || channelID == 2 ) )
        {
            channel = CreateChannel( channelID );

            if( (!channel->IsJoined( charID )) && (channelID != call.client->GetCharacterID()) )
            {
                ChannelJoinReply chjr;

                chjr.ChannelID = channel->EncodeID();
                chjr.ChannelInfo = channel->EncodeChannelSmall( charID );
                // this one'll create an empty query result
                // noone implemented channel mods.
                chjr.ChannelMods = channel->EncodeChannelMods();
                chjr.ChannelChars = channel->EncodeChannelChars();
               // chjr.ChannelChars = channel->EncodeEmptyChannelChars();

                channel->JoinChannel( call.client );

                // Save this subscription to this channel to the database
                if (!(m_db->IsChannelSubscribedByThisChar(charID, channel->GetChannelID())))
                    m_db->WriteNewChannelSubscriptionToDatabase( charID, channel->GetChannelID(),
                        call.client->GetCorporationID(), call.client->GetAllianceID(),
                        2, 0 );
                        // the "extra" field is hard-coded to '0' for now since I don't know what it's used for
                ml->AddItem( chjr.Encode() );
            }
        }
    }

    if (is_log_enabled(LSC__RSP_DUMP))
        ml->Dump(LSC__RSP_DUMP, "   ");

    return ml;
}


PyResult LSCService::Handle_LeaveChannel(PyCallArgs &call) {
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White( "LSCService::Handle_LeaveChannel()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    CallLeaveChannel arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    uint32 toLeave;

    if( arg.channel->IsInt() )
        toLeave = arg.channel->AsInt()->value();
    else if( arg.channel->IsTuple() )
    {
        PyTuple* prt = arg.channel->AsTuple();

        if( prt->GetItem( 0 )->IsInt() )
            toLeave = prt->GetItem( 0 )->AsInt()->value();
        else if( prt->GetItem( 0 )->IsTuple() )
        {
            prt = prt->GetItem( 0 )->AsTuple();

            if( prt->items.size() != 2 || !prt->GetItem( 1 )->IsInt() )
            {
                codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
                return nullptr;
            }

            toLeave = prt->GetItem( 1 )->AsInt()->value();
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


    if( m_channels.find( toLeave ) != m_channels.end() )
    {
        // Remove channel subscription from database if this character was subscribed to it.
        // NOTE: channel subscriptions are NOT saved to the database for private convo chats
        if( m_db->IsChannelSubscribedByThisChar(call.client->GetCharacterID(),toLeave) )
            m_db->RemoveChannelSubscriptionFromDatabase(toLeave,call.client->GetCharacterID());

        // Remove channel from database if this character was the last one
        // in the channel to leave and it was a private convo (temporary==1):
        if( (m_channels.find( toLeave )->second->GetMemberCount() == 1)
            && (m_channels.find( toLeave )->second->GetTemporary() != 0)
            && (toLeave >= LSCService::BASE_CHANNEL_ID) )
                m_db->RemoveChannelFromDatabase(toLeave);

        m_channels[ toLeave ]->LeaveChannel( call.client );
    }

    return nullptr;
}


PyResult LSCService::Handle_LeaveChannels(PyCallArgs &call) {
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White( "LSCService::Handle_LeaveChannels()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    CallLeaveChannels args;

    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    std::set<uint32> toLeave;

    {
        PyList::const_iterator cur, end;
        cur = args.channels->begin();
        end = args.channels->end();

        for(; cur != end; cur++)
        {
            if( (*cur)->IsInt() )
                toLeave.insert( (*cur)->AsInt()->value() );
            else if( (*cur)->IsTuple() )
            {
                PyTuple* prt = (*cur)->AsTuple();

                if( prt->GetItem( 0 )->IsInt() )
                {
                    toLeave.insert( prt->GetItem( 0 )->AsInt()->value() );
                    continue;
                }

                if( !prt->GetItem( 0 )->IsTuple() )
                {
                    codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
                    continue;
                }
                prt = prt->GetItem( 0 )->AsTuple();

                if( prt->GetItem( 0 )->IsTuple() )
                    prt = prt->GetItem( 0 )->AsTuple();

                if( prt->size() != 2 || !prt->GetItem( 1 )->IsInt() )
                {
                    codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
                    continue;
                }

                toLeave.insert( prt->GetItem( 1 )->AsInt()->value() );
            }
            else
            {
                codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
                continue;
            }
        }
    }

    {
        std::set<uint32>::iterator cur = toLeave.begin(), end = toLeave.end();
        for (;cur!=end;cur++) {
            if (m_channels.find(*cur) != m_channels.end())
            {
                // Remove channel subscription from database if this character was subscribed to it.
                // NOTE: channel subscriptions are NOT saved to the database for private convo chats
                if( m_db->IsChannelSubscribedByThisChar(call.client->GetCharacterID(),*cur))
                    m_db->RemoveChannelSubscriptionFromDatabase(*cur,call.client->GetCharacterID() );

                // Remove channel from database if this character was the last one
                // in the channel to leave and it was a private convo (temporary==1):
                if( (m_channels.find( *cur )->second->GetMemberCount() == 1)
                    && (m_channels.find( *cur )->second->GetTemporary() != 0)
                    && (m_channels.find( *cur )->second->GetChannelID() >= LSCService::BASE_CHANNEL_ID) )
                        m_db->RemoveChannelFromDatabase(*cur);

                m_channels[*cur]->LeaveChannel(call.client);
            }
        }
    }

    return (new PyNone());
}


PyResult LSCService::Handle_CreateChannel( PyCallArgs& call )
{
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White( "LSCService::Handle_CreateChannel()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    // WARNING: This call contains manual packet decoding to handle configuring parameters for
    // user-created chat channels since I didn't want to monkey around with the LSCPkts.xmlp.
    // -- Aknor Jaden (2010-11-26)

    Call_SingleWStringSoftArg name;
    LSCChannel* channel = NULL;

    if( !name.Decode( call.tuple ) )
    {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    bool create_channel_exists = false;
    bool create_channel = false;
    bool temporary_exists = false;
    bool temporary_channel = false;
    bool joinExisting_exists = false;
    bool joinExisting_channel = false;

    if (call.byname.find("create") != call.byname.end())
    {
        create_channel_exists = true;
        if (call.byname.find("create")->second->AsBool()->value())
            create_channel = true;
    }

    if (call.byname.find("temporary") != call.byname.end())
    {
        temporary_exists = true;
        if (call.byname.find("temporary")->second->AsBool()->value())
            temporary_channel = true;
    }

    if (call.byname.find("joinExisting") != call.byname.end())
    {
        joinExisting_exists = true;
        if (call.byname.find("joinExisting")->second->AsBool()->value())
            joinExisting_channel = true;
    }


    if (create_channel_exists && create_channel)
    {
        // Query Database to see if a channel with this name does not exist and, if so, create the channel,
        // otherwise, set the channel pointer to NULL
        if (m_db->IsChannelNameAvailable(name.arg))
            channel = CreateChannel( name.arg.c_str() );
        else
        {
            _log(LSC__ERROR, "%s: Error creating new chat channel: channel name '%s' already exists.", call.client->GetName(), name.arg.c_str() );
            channel = NULL;
        }

        if (channel == NULL)
        {
            _log(LSC__ERROR, "%s: Error creating new chat channel", call.client->GetName() );
            return nullptr;
        }

        // Save channel info and channel subscription to the database
        m_db->WriteNewChannelToDatabase(channel->GetChannelID(),channel->GetDisplayName(), call.client->GetCharacterID(),0,/*mode*/3);
        m_db->WriteNewChannelSubscriptionToDatabase( call.client->GetCharacterID(), channel->GetChannelID(),
            call.client->GetCorporationID(), call.client->GetAllianceID(),
            2, 0 );     // the "extra" field is hard-coded
                                                    // to '0' for now since I don't
                                                    // know what it's used for

        channel->JoinChannel( call.client );

        ChannelCreateReply reply;
        reply.ChannelChars = channel->EncodeChannelChars();
        reply.ChannelInfo = channel->EncodeChannelSmall( call.client->GetCharacterID() );
        reply.ChannelMods = channel->EncodeChannelMods();
        return reply.Encode();
    }


    if (joinExisting_exists && joinExisting_channel)
    {
        std::string channel_name = call.tuple->items[0]->AsWString()->content();

        if (!(m_db->IsChannelNameAvailable(channel_name)))
        {
            // Channel exists, so get its info from database and create this channel in the cache:
            std::string ch_name, ch_motd, ch_compkey, ch_password;
            uint32 ch_ID, ch_ownerID, ch_cspa, ch_temp, ch_mode;
            bool ch_memberless, ch_maillist;
            LSCChannel::Type ch_type = LSCChannel::normal;

            m_db->GetChannelInformation(channel_name,ch_ID,ch_motd,ch_ownerID,ch_compkey,ch_memberless,ch_password,ch_maillist,ch_cspa,ch_temp,ch_mode);

            channel = CreateChannel
            (
                ch_ID,
                channel_name.c_str(),
                ch_motd.c_str(),
                ch_type,
                ch_compkey.c_str(),
                ch_ownerID,
                ch_memberless,
                ch_password.c_str(),
                ch_maillist,
                ch_cspa,
                ch_temp,
                ch_mode
            );

            if (channel == NULL)
            {
                _log(LSC__ERROR, "%s: Error creating new chat channel", call.client->GetName() );
                return nullptr;
            }
        }
        else
        {
            _log(LSC__ERROR, "%s: Unable to join channel '%s', this channel does not exist.", call.client->GetName(), channel_name.c_str() );
            return nullptr;
        }
    }


    if (temporary_exists && temporary_channel)
    {
        uint32 channel_id;
        channel_id = m_db->GetNextAvailableChannelID();

        // This is a temporary private chat channel, so don't look for it in the database, just make a new one:
        channel = CreateChannel
        (
            channel_id,
            call.tuple->GetItem(0)->AsString()->content().c_str(),
            "",
            LSCChannel::normal,
            "",
            call.client->GetCharacterID(),
            false,
            "",
            false,
            0,
            1,
            0
        );

        if (channel == NULL)
        {
            _log(LSC__ERROR, "%s: Error creating new Temporary chat channel", call.client->GetName() );
            return nullptr;
        }

        // Save this channel to the database with the 'temporary' field marked as 1 so that when the last character
        // leaves this channel, the server knows to remove it from the database:
        m_db->WriteNewChannelToDatabase(channel_id,call.tuple->GetItem(0)->AsString()->content(),call.client->GetCharacterID(),1,/*mode*/3);
    }


    if ((joinExisting_exists && joinExisting_channel) || (temporary_exists && temporary_channel))
    {
        // Now that channel is created, join it:
        if( !channel->IsJoined( call.client->GetCharacterID() ) )
        {
            // Save this subscription to this channel to the database IF it is not temporary:
            if (channel->GetTemporary() == 0)
                m_db->WriteNewChannelSubscriptionToDatabase( call.client->GetCharacterID(), channel->GetChannelID(),
                    call.client->GetCorporationID(), call.client->GetAllianceID(),
                    2, 0 );     // the "extra" field is hard-coded
                                                            // to '0' for now since I don't
                                                            // know what it's used for

            channel->JoinChannel( call.client );

            ChannelCreateReply reply;
            reply.ChannelChars = channel->EncodeChannelChars();
            reply.ChannelInfo = channel->EncodeChannelSmall( call.client->GetCharacterID() );
            reply.ChannelMods = channel->EncodeChannelMods();
            return reply.Encode();
        }

        // Somehow execution got here and was not captured in either Creating a new channel, Joining a temporary channel,
        // or Joining an existing channel, so print an error:
        _log(LSC__ERROR, "%s: ERROR: Character %u tried to join/create channel '%s'.  The packet format was unexpected.", call.client->GetName(), call.client->GetCharacterID(), channel->GetDisplayName().c_str() );
        return nullptr;
    }
    else
    {
        // Malformed packet somehow / no "create" field in byname map
        _log(LSC__ERROR, "%s: Malformed packet: 'create' field in byname map is missing.", call.client->GetName() );
        return nullptr;
    }

    // what does this return??
    return nullptr;
}


PyResult LSCService::Handle_Configure( PyCallArgs& call )
{
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White( "LSCService::Handle_Configure()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    // WARNING: This call contains manual packet decoding to handle configuring parameters for
    // user-created chat channels since I didn't want to monkey around with the LSCPkts.xmlp.
    // -- Aknor Jaden (2010-11-26)

    LSCChannel* channel = NULL;
    int32 channel_id = 0;

    //ChannelInfo args;
    //if (!args.Decode( call.tuple )) {
    //codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
    //    return nullptr;
    //}

    // Get Tuple which contains channel number to modify:
    if (call.tuple->AsTuple()->GetItem(0)->IsInt())
        channel_id = call.tuple->AsTuple()->GetItem(0)->AsInt()->value();
    else
    {
        _log(LSC__ERROR, "%s: Tuple contained wrong type: '%s'", call.client->GetName(), call.tuple->TypeString() );
        return nullptr;
    }

    // Get count of parameters or just loop through the std::map until you've reached the end
    if (call.byname.size() == 0)
    {
        _log(LSC__ERROR, "%s: byname std::map contained zero elements, expected at least one.", call.client->GetName() );
        return nullptr;
    }

    // Find channel in existing channels:
    std::map<uint32, LSCChannel*>::iterator res = m_channels.find( channel_id );
    if( m_channels.end() == res )
    {
        _log(LSC__ERROR, "%s: Handle_Configure Couldn't find channel %u", call.client->GetName(), channel_id );
        return nullptr;
    }

    channel = m_channels.find(channel_id)->second;

    std::string str_NEW_displayName;
    int32 int_NEW_memberless;
    std::string str_NEW_motd;
    std::string str_newPassword;
    std::string str_oldPassword;

    // For each entry in the map, check its first value against one of these strings, then call appropriate set() function:
    //        "displayName"
    if (!(call.byname.find("displayName") == call.byname.end()))
    {
        if (call.byname.find("displayName")->second->IsWString())
        {
            str_NEW_displayName = call.byname.find("displayName")->second->AsWString()->content();
            channel->SetDisplayName(str_NEW_displayName);
        }
        else
        {
            _log(LSC__ERROR, "%s: displayName contained wrong type: '%s'", call.client->GetName(), call.byname.find("displayName")->second->TypeString() );
            return nullptr;
        }
    }

    //        "memberless"
    if (!(call.byname.find("memberless") == call.byname.end()))
    {
        if (call.byname.find("memberless")->second->IsInt())
        {
            int_NEW_memberless = call.byname.find("memberless")->second->AsInt()->value();
            channel->SetMemberless(int_NEW_memberless ? true : false);
        }
        else
        {
            _log(LSC__ERROR, "%s: memberless contained wrong type: '%s'", call.client->GetName(), call.byname.find("memberless")->second->TypeString() );
            return nullptr;
        }
    }

    //        "motd"
    if (!(call.byname.find("motd") == call.byname.end()))
    {
        if (call.byname.find("motd")->second->IsWString())
        {
            str_NEW_motd = call.byname.find("motd")->second->AsWString()->content();
            channel->SetMOTD(str_NEW_motd);
        }
        else
        {
            _log(LSC__ERROR, "%s: motd contained wrong type: '%s'", call.client->GetName(), call.byname.find("motd")->second->TypeString() );
            return nullptr;
        }
    }

    //        "oldPassword"
    if (!(call.byname.find("oldPassword") == call.byname.end()))
    {
        if (call.byname.find("oldPassword")->second->IsWString())
        {
            str_oldPassword = call.byname.find("oldPassword")->second->AsWString()->content();
            if (channel->GetPassword() == str_oldPassword)
            {
                //        "newPassword"
                if (!(call.byname.find("newPassword") == call.byname.end()))
                {
                    if (call.byname.find("newPassword")->second->IsWString())
                    {
                        str_newPassword = call.byname.find("newPassword")->second->AsWString()->content();
                        channel->SetPassword(str_newPassword);
                    }
                    else
                    {
                        _log(LSC__ERROR, "%s: newPassword contained wrong type: '%s'", call.client->GetName(), call.byname.find("newPassword")->second->TypeString() );
                        return nullptr;
                    }
                }
            }
            else
            {
                _log(LSC__ERROR, "%s: incorrect oldPassword supplied. Password NOT changed.", call.client->GetName() );
                return nullptr;
            }
        }
        else if (call.byname.find("oldPassword")->second->IsNone())
        {
            //        "newPassword"
            if (!(call.byname.find("newPassword") == call.byname.end()))
            {
                if (call.byname.find("newPassword")->second->IsWString())
                {
                    str_newPassword = call.byname.find("newPassword")->second->AsWString()->content();
                    channel->SetPassword(str_newPassword);
                }
                else
                {
                    _log(LSC__ERROR, "%s: newPassword contained wrong type: '%s'", call.client->GetName(), call.byname.find("newPassword")->second->TypeString() );
                    return nullptr;
                }
            }
        }
        else
        {
            _log(LSC__ERROR, "%s: oldPassword is of an unexpected type: '%s'", call.client->GetName(), call.byname.find("newPassword")->second->TypeString() );
            return nullptr;
        }
    }

    // Save the new channel parameters to the database 'channels' table:
    m_db->UpdateChannelConfigureInfo(channel);

    // ********** TODO **********
    // Figure out how to send a packet to all clients subscribed to this channel that contains all channel parameters
    // so that their clients can update everything that has changed in this channel's configuration.
    // **************************

    // This packet sent back to the client configuring the channel parameters is insufficient to update itself or
    // any other client attached to this channel.
    ChannelCreateReply reply;
    reply.ChannelChars = channel->EncodeChannelChars();
    reply.ChannelInfo = channel->EncodeChannelSmall( call.client->GetCharacterID() );
    reply.ChannelMods = channel->EncodeChannelMods();

    if (is_log_enabled(LSC__RSP_DUMP))
        reply.Dump(LSC__RSP_DUMP);

    return reply.Encode();
}


PyResult LSCService::Handle_DestroyChannel( PyCallArgs& call )
{
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White( "LSCService::Handle_DestroyChannel()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    Call_SingleIntegerArg arg;
    if( !arg.Decode( call.tuple ) )
    {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    std::map<uint32, LSCChannel*>::iterator res = m_channels.find( arg.arg );
    if( m_channels.end() == res )
    {
        _log(LSC__ERROR, "%s: Couldn't find channel %u", call.client->GetName(), arg.arg );
        return nullptr;
    }

    // ********** TODO **********
    // Figure out how to validate whether this character (call.client->GetCharacterID()) is allowed
    // to destroy this chat channel, and proceed if they are, otherwise, do not.  And, is there an error
    // packet sent back to the client?
    // **************************

    // Finally, remove the channel from the server dynamic objects:
    res->second->Evacuate( call.client );
    SafeDelete( res->second );
    m_channels.erase( res );

    // Now, remove the channel from the database:
    m_db->RemoveChannelFromDatabase( res->second->GetChannelID() );

    return new PyNone();
}


PyResult LSCService::Handle_SendMessage( PyCallArgs& call )
{
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White( "LSCService::Handle_SendMessage()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    // WARNING: This call contains manual packet decoding to handle chat messages sent inside user-created
    // chat channels since I didn't want to monkey around with the LSCPkts.xmlp.  All chat message packets
    // received for Local/Corp/Region/Constellation chat channels are still processed via LSCPkts.cpp.
    // -- Aknor Jaden (2010-11-07)


    int32 channel_id = 0;
    std::string message;

    Call_SendMessage args;

    if( ( call.tuple->IsTuple() ) && (call.tuple->AsTuple()->items[0]->IsInt()) )
    {
        // Decode All User-created chat channel messages here:
        if( !call.tuple->IsTuple() )
        {
            _log( NET__PACKET_ERROR, "LSCService::Handle_SendMessage failed: tuple0 is the wrong type: %s", call.tuple->TypeString() );

            return nullptr;
        }
        PyTuple* tuple0 = call.tuple->AsTuple();

        if( tuple0->size() != 2 )
        {
            _log( NET__PACKET_ERROR, "LSCService::Handle_SendMessage failed: tuple0 is the wrong size: expected 2, but got %lu", tuple0->size() );

            return nullptr;
        }

        channel_id = (call.tuple->AsTuple()->items[0]->AsInt())->value();
        message = ((call.tuple->AsTuple()->items[1]->AsWString())->content());
        sLog.White( "LSCService", "Handle_SendMessage: call is either User-created chat message or bad packet.");
    }
    else
    {
        if( !args.Decode( call.tuple ) )
        {
            codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
            return nullptr;
        }
        channel_id = args.channel.id;
        message = args.message;
        _log(LSC__INFO, "Handle_SendMessage: call is Corp/Local/Region/Constellation chat.");
    }

    std::map<uint32, LSCChannel*>::iterator itr = m_channels.find( channel_id );
    if (itr == m_channels.end()) {
        _log(LSC__ERROR, "%s: Couldn't find channel %u", call.client->GetName(), channel_id );
        return nullptr;
    }

	std::string CIC_test_name = "CIC - " + std::string(call.client->GetName());
	if( (message.substr(0,3) == "pcs") && (itr->second->GetDisplayName() == CIC_test_name) )
	{
        _log(LSC__INFO, "CALL to Player Command System via LSC Service" );

		// call to Player Command System to parse command

		//if( command_ack == 1 )
			message = "[ COMMAND ACKNOWLEDGED ]";
		//else
		//	message = "[ COMMAND FAILED ]";
	}

	if( message == "cic" )
        _log(LSC__INFO, "Message 'cic' received, creating/joining %s...", CIC_test_name.c_str() );

    if( message.at(0) == '.' )
    {
        _log(LSC__INFO, "CALL to SlashService->SlashCmd() via LSC Service" );
        static_cast<SlashService *>(m_manager->LookupService("slash"))->SlashCommand( call.client, message );

        message = "*dot command via slash service*";      // Still transmit some message but minimal so that chat window is not "locked" by client for not getting a chat
    }

    itr->second->SendMessage( call.client, message.c_str() );

    return new PyInt( 1 );
}


PyResult LSCService::Handle_AccessControl( PyCallArgs& call )
{
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White( "LSCService::Handle_AccessControl()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    // WARNING: This call contains manual packet decoding to handle Access Control since I didn't want to monkey around with the LSCPkts.xmlp.
    // -- Aknor Jaden (2010-11-26)

    int32 channel_id = 0;

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


    // ********** TODO **********
    // Figure out how to send a packet to all clients subscribed to this channel that contains all channel parameters
    // so that their clients can update everything that has changed in this channel's access control.
    // **************************

    return new PyInt( 1 );
}

PyResult LSCService::Handle_Invite(PyCallArgs &call)
{
    if (is_log_enabled(LSC__CALL_DUMP)) {
        sLog.White( "LSCService::Handle_Invite()", "size=%u", call.tuple->size());
        call.Dump(LSC__CALL_DUMP);
    }

    // WARNING: This call contains manual packet decoding to handle chat messages sent inside user-created
    // chat channels since I didn't want to monkey around with the LSCPkts.xmlp.
    // -- Aknor Jaden (2010-11-19)

    LSCChannel* channel;

    uint32 channel_ID;
    uint32 char_ID = call.client->GetCharacterID();
    uint32 invited_char_ID;

    // Decode the call:
    if (call.tuple->IsTuple())
    {
        if (call.tuple->GetItem(1)->IsInt())
            channel_ID = call.tuple->GetItem(1)->AsInt()->value();
        else
        {
            _log(LSC__ERROR, "%s: call.tuple->GetItem(1) is of the wrong type: '%s'.  Expected PyInt type.", call.client->GetName(), call.tuple->TypeString() );
            return nullptr;
        }

        if (call.tuple->GetItem(0)->IsInt())
            invited_char_ID = call.tuple->GetItem(0)->AsInt()->value();
        else
        {
            _log(LSC__ERROR, "%s: call.tuple->GetItem(0) is of the wrong type: '%s'.  Expected PyInt type.", call.client->GetName(), call.tuple->TypeString() );
            return nullptr;
        }
    }
    else
    {
        _log(LSC__ERROR, "%s: call.tuple is of the wrong type: '%s'.  Expected PyTuple type.", call.client->GetName(), call.tuple->TypeString() );
        return nullptr;
    }

    // Now that the packet is known to be good, find the channel to join and join it:
    if (m_channels.find(channel_ID) != m_channels.end())
    {
        channel = m_channels[ channel_ID ];

        if( !channel->IsJoined( invited_char_ID ) )
        {
            // SOMEHOW SEND A JOIN COMMAND/REQUEST TO THE TARGET CLIENT FOR invited_char_ID
        /*    OnLSC_JoinChannel join;
            join.sender = channel->_MakeSenderInfo(call.client);
            join.member_count = 1;
            join.channelID = channel->EncodeID();
            PyTuple *answer = join.Encode();
            MulticastTarget mct;
            //LSCChannelChar *invitor;
            //LSCChannelChar *invitee;
            if ( !channel->IsJoined(char_ID) )
            {
                //invitor = new LSCChannelChar(channel,0,char_ID,call.client->GetCharacterName(),0,0,0,0);
                mct.characters.insert(char_ID);
            }
            //invitee = new LSCChannelChar(channel,0,invited_char_ID,entityList().FindCharacter(invited_char_ID)->GetCharacterName(),0,0,0,0);
            mct.characters.insert(invited_char_ID);
            entityList().Multicast( "OnLSC", channel->GetTypeString(), &answer, mct );
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
        }
        else
        {
            _log(LSC__ERROR, "%s: Character %u is already joined to channel %u.", call.client->GetName(), invited_char_ID, channel_ID );
            return nullptr;
        }
    }
    else
    {
        _log(LSC__ERROR, "%s: Cannot find channel %u.", call.client->GetName(), channel_ID );
        return nullptr;
    }

    return new PyInt( 1 );
}


void LSCService::CharacterLogin(Client* pClient)
{
    // create Corp chat channel:
    CreateChannel(pClient->GetCorporationID());
    // create Alliance chat channel:
    CreateChannel(pClient->GetAllianceID());

}

void LSCService::CharacterLogout(uint32 charID, OnLSC_SenderInfo* si)
{
    std::map<uint32, LSCChannel*>::iterator cur, end;
    cur = m_channels.begin();
    end = m_channels.end();
    for(; cur != end; cur++)
        if( cur->second->IsJoined( charID ) )
            cur->second->LeaveChannel( charID, new OnLSC_SenderInfo( *si ) );

    SafeDelete( si );
}

void LSCService::SystemUnload(uint32 systemID, uint32 constID, uint32 regionID)
{
    std::map<uint32, LSCChannel*>::iterator itr = m_channels.find(systemID);
    if (itr != m_channels.end()) {
        SafeDelete( itr->second );
        m_channels.erase( itr );
    }
    /** @todo  find a way to track usages of region and const channels to delete when no longer used */
    //  is this needed?
}


LSCChannel* LSCService::CreateChannel(uint32 channelID, const char * name, const char * motd, LSCChannel::Type type, const char * compkey,
                                      uint32 ownerID, bool memberless, const char * password, bool maillist, uint32 cspa, uint32 temporary, uint32 mode) {
    std::map<uint32, LSCChannel*>::iterator itr = m_channels.find(channelID);
    if (itr != m_channels.end())
        return itr->second;
    return m_channels[channelID] = new LSCChannel(this, channelID, type, ownerID, name, motd, compkey, memberless, password, maillist, cspa, temporary, mode);
}

LSCChannel* LSCService::CreateChannel(uint32 channelID, const char * name, const char * motd, LSCChannel::Type type, bool maillist) {
    std::map<uint32, LSCChannel*>::iterator itr = m_channels.find(channelID);
    if (itr != m_channels.end())
        return itr->second;
    return m_channels[channelID] = new LSCChannel(this, channelID, type, 1, name, motd, NULL, false, "", maillist, true, false, /*mode*/3);
}

LSCChannel* LSCService::CreateChannel(uint32 channelID, const char * name, LSCChannel::Type type, bool maillist) {
    std::map<uint32, LSCChannel*>::iterator itr = m_channels.find(channelID);
    if (itr != m_channels.end())
        return itr->second;
    return m_channels[channelID] = new LSCChannel(this, channelID, type, 1, name, NULL, NULL, false, "", maillist, true, false, /*mode*/3);
}

LSCChannel* LSCService::CreateChannel(uint32 channelID) {
    if (!channelID)
        return nullptr;
    std::map<uint32, LSCChannel*>::iterator itr = m_channels.find(channelID);
    if (itr != m_channels.end())
        return itr->second;
    LSCChannel::Type type;
    std::string name;
    std::string motd;
    if (IsRegion(channelID)) { type = LSCChannel::region; name = "System Channels\\Region"; motd = m_db->GetRegionName(channelID); }
    else if (IsConstellation(channelID)) {type = LSCChannel::constellation; name = "System Channels\\Constellation"; motd = m_db->GetConstellationName(channelID); }
    else if (IsSolarSystem(channelID)) { type = LSCChannel::solarsystem; name = "System Channels\\Local"; motd = m_db->GetSolarSystemName(channelID); }
    else if (IsCorp(channelID)) { type = LSCChannel::corp; name = "System Channels\\Corp"; motd = m_db->GetCorporationName(channelID); }
    else { type = LSCChannel::normal; m_db->GetChannelInfo(channelID, name, motd); }

    return m_channels[channelID] = new LSCChannel(this, channelID, type, 1, name.c_str(), motd.c_str(), NULL, false, NULL, false, true, false, /*mode*/3);
}

LSCChannel* LSCService::CreateChannel(const char * name, bool maillist/*false*/) {
    uint32 nextFreeChannelID = m_db->GetNextAvailableChannelID();

    if( nextFreeChannelID )
        return CreateChannel(nextFreeChannelID, name, LSCChannel::normal, maillist);
    else
        return nullptr;
}

void LSCService::CreateStaticChannels() {
    // hardcode creating server static channels during server startup
    const char * password = "";
    const char *motd = "3C-63-6F-6C-6F-72-3D-30-78-66-66-66-66-30-30-30-30-3E-3C-62-3E-57-65-6C-63-6F-6D-65-20-74-6F-20-45-56-45-20-4F-6E-6C-69-6E-65-3A-20-49-6E-63-75-72-73-69-6F-6E-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-30-30-37-66-66-66-3E-20-3C-62-72-3E-3C-62-72-3E-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-30-30-66-66-30-30-3E-50-6C-61-79-65-72-20-47-75-69-64-65-73-3A-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-62-32-62-32-62-32-66-66-3E-20-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-66-66-61-35-30-30-3E-3C-75-72-6C-3D-68-74-74-70-3A-2F-2F-77-69-6B-69-2E-65-76-65-6F-6E-6C-69-6E-65-2E-63-6F-6D-2F-77-69-6B-69-2F-43-61-74-65-67-6F-72-79-3A-4E-65-77-5F-50-6C-61-79-65-72-5F-45-78-70-65-72-69-65-6E-63-65-3E-3C-75-3E-68-74-74-70-3A-2F-2F-77-69-6B-69-2E-65-76-65-6F-6E-6C-69-6E-65-2E-63-6F-6D-2F-77-69-6B-69-2F-43-61-74-65-67-6F-72-79-3A-4E-65-77-5F-50-6C-61-79-65-72-5F-45-78-70-65-72-69-65-6E-63-65-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-62-32-62-32-62-32-66-66-3E-3C-2F-75-72-6C-3E-3C-2F-75-3E-20-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-62-32-62-32-62-32-3E-77-6F-72-74-68-20-61-20-72-65-61-64-2E-2E-3C-62-72-3E-50-61-74-63-68-20-4E-6F-74-65-73-3A-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-62-32-62-32-62-32-66-66-3E-20-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-66-66-61-35-30-30-3E-3C-75-72-6C-3D-68-74-74-70-3A-2F-2F-77-77-77-2E-65-76-65-6F-6E-6C-69-6E-65-2E-63-6F-6D-2F-75-70-64-61-74-65-73-2F-70-61-74-63-68-6E-6F-74-65-73-2E-61-73-70-3E-3C-75-3E-68-74-74-70-3A-2F-2F-77-77-77-2E-65-76-65-6F-6E-6C-69-6E-65-2E-63-6F-6D-2F-75-70-64-61-74-65-73-2F-70-61-74-63-68-6E-6F-74-65-73-2E-61-73-70-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-66-66-30-30-30-30-3E-3C-2F-75-72-6C-3E-3C-2F-62-3E-3C-2F-75-3E-20-3C-62-72-3E-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-62-32-62-32-62-32-3E-3C-62-3E-50-6C-65-61-73-65-3A-20-53-74-61-79-20-6F-6E-20-74-6F-70-69-63-2E-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-62-32-62-32-62-32-66-66-3E-20-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-66-66-66-66-30-30-3E-4E-6F-20-6F-66-66-74-6F-70-69-63-2C-20-57-54-42-2C-20-57-54-53-2C-20-50-43-2C-20-61-64-76-65-72-74-69-73-69-6E-67-2C-20-72-65-63-72-75-69-74-69-6E-67-2C-20-73-63-61-6D-6D-69-6E-67-20-74-72-61-64-69-6E-67-20-69-6E-20-67-65-6E-65-72-61-6C-20-6F-72-20-62-65-67-67-69-6E-67-20-69-6E-20-74-68-69-73-20-63-68-61-6E-6E-65-6C-2E-20-4E-6F-20-43-41-50-53-20-6F-72-20-74-65-78-74-2D-64-65-63-6F-72-61-74-69-6F-6E-20-65-69-74-68-65-72-20-21-21-3C-62-72-3E-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-30-30-66-66-30-30-3E-4C-61-6E-67-75-61-67-65-3A-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-62-32-62-32-62-32-66-66-3E-20-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-62-32-62-32-62-32-3E-54-68-69-73-20-63-68-61-6E-6E-65-6C-20-69-73-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-66-66-66-66-66-66-3E-20-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-66-66-30-30-30-30-3E-45-4E-47-4C-49-53-48-20-4F-4E-4C-59-21-21-3C-62-72-3E-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-30-30-66-66-30-30-3E-4C-69-6E-6B-73-3A-20-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-62-32-62-32-62-32-3E-55-52-4C-27-73-20-67-69-76-65-6E-20-74-6F-20-73-69-74-65-73-20-6F-74-68-65-72-20-74-68-61-6E-20-74-6F-20-77-77-77-2E-65-76-65-6F-6E-6C-69-6E-65-2E-63-6F-6D-20-6D-61-79-20-62-65-20-6F-75-74-64-61-74-65-64-2E-20-50-6C-65-61-73-65-20-68-61-6E-64-6C-65-20-61-6C-6C-20-74-68-6F-73-65-20-55-52-4C-27-73-20-77-69-74-68-20-63-61-72-65-2E-3C-62-72-3E-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-30-30-66-66-30-30-3E-48-6F-77-20-74-6F-20-63-6F-6E-74-61-63-74-20-61-20-47-4D-3A-20-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-62-32-62-32-62-32-3E-46-69-6C-65-20-61-20-70-65-74-69-74-69-6F-6E-20-28-20-46-31-32-20-2D-20-50-65-74-69-74-69-6F-6E-73-20-2D-20-4E-65-77-20-50-65-74-69-74-69-6F-6E-20-29-20-3C-62-72-3E-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C\
    -6F-72-3D-30-78-66-66-30-30-66-66-30-30-3E-45-70-69-63-20-41-72-63-20-41-67-65-6E-74-73-3A-20-3C-62-72-3E-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-66-66-61-35-30-30-3E-3C-75-72-6C-3D-68-74-74-70-3A-2F-2F-77-69-6B-69-2E-65-76-65-6F-6E-6C-69-6E-65-2E-63-6F-6D-2F-65-6E-2F-77-69-6B-69-2F-44-6F-6D-69-6E-69-6F-6E-5F-65-70-69-63-5F-61-72-63-73-3E-3C-75-3E-68-74-74-70-3A-2F-2F-77-69-6B-69-2E-65-76-65-6F-6E-6C-69-6E-65-2E-63-6F-6D-2F-65-6E-2F-77-69-6B-69-2F-44-6F-6D-69-6E-69-6F-6E-5F-65-70-69-63-5F-61-72-63-73-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-66-66-30-30-30-30-3E-3C-2F-75-72-6C-3E-3C-2F-62-3E-3C-2F-75-3E-20-3C-62-72-3E-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-66-66-61-35-30-30-3E-3C-75-72-6C-3D-68-74-74-70-3A-2F-2F-77-69-6B-69-2E-65-76-65-6F-6E-6C-69-6E-65-2E-63-6F-6D-2F-65-6E-2F-77-69-6B-69-2F-45-70-69-63-5F-6D-69-73-73-69-6F-6E-5F-61-72-63-73-3E-3C-62-3E-3C-75-3E-68-74-74-70-3A-2F-2F-77-69-6B-69-2E-65-76-65-6F-6E-6C-69-6E-65-2E-63-6F-6D-2F-65-6E-2F-77-69-6B-69-2F-45-70-69-63-5F-6D-69-73-73-69-6F-6E-5F-61-72-63-73-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-66-66-30-30-30-30-3E-3C-2F-75-72-6C-3E-3C-2F-62-3E-3C-2F-75-3E-20-3C-62-72-3E-3C-62-72-3E-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-30-30-66-66-30-30-3E-3C-62-3E-54-68-65-20-74-6F-70-69-63-20-6F-66-20-74-68-69-73-20-63-68-61-6E-6E-65-6C-20-69-73-20-45-56-45-20-72-65-6C-61-74-65-64-20-68-65-6C-70-21-21-21-3C-62-72-3E-3C-62-72-3E-43-68-61-74-72-75-6C-65-73-3A-20-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-66-66-61-35-30-30-3E-3C-75-72-6C-3D-68-74-74-70-3A-2F-2F-77-77-77-2E-65-76-65-6F-6E-6C-69-6E-65-2E-63-6F-6D-2F-70-6E-70-2F-63-68-61-74-72-75-6C-65-73-2E-61-73-70-3E-3C-75-3E-77-77-77-2E-65-76-65-6F-6E-6C-69-6E-65-2E-63-6F-6D-2F-70-6E-70-2F-63-68-61-74-72-75-6C-65-73-2E-61-73-70-3C-2F-63-6F-6C-6F-72-3E-3C-63-6F-6C-6F-72-3D-30-78-66-66-30-30-66-66-30-30-3E-3C-2F-75-72-6C-3E-3C-2F-75-3E-20-70-6C-65-61-73-65-20-72-65-61-64-20-61-6E-64-20-6F-62-73-65-72-76-65-20-74-68-65-6D-2E-2E-3C-2F-63-6F-6C-6F-72-3E-3C-2F-62-3E";
    CreateChannel(1, "Help\\Rookie Help", "Rookie motd", LSCChannel::normal, "help", 1, false, password, false, cspa, 0, 3);
    CreateChannel(2, "Help\\Help", motd, LSCChannel::normal, "help", 1, false, password, false, cspa, 0, 3);

    CreateChannel(10, "Trade\\Other", "motd", LSCChannel::normal, "other", 1, true, password, false, cspa, 0, 3);
    CreateChannel(11, "Trade\\Ships", "motd", LSCChannel::normal, "ships", 1, true, password, false, cspa, 0, 3);
    CreateChannel(12, "Trade\\Blueprints", "motd", LSCChannel::normal, "blueprints", 1, true, password, false, cspa, 0, 3);
    CreateChannel(13, "Trade\\Modules and Munitions", "motd", LSCChannel::normal, "modulesandmunitions", 1, true, password, false, cspa, 0, 3);
    CreateChannel(14, "Trade\\Minerals and Manufacturing", "motd", LSCChannel::normal, "mineralsandmanufacturing", 1, true, password, false, cspa, 0, 3);

    CreateChannel(16, "Empires\\Caldari", "motd", LSCChannel::normal, "caldari", 1, true, password, false, cspa, 0, 3);
    CreateChannel(17, "Empires\\Amarr", "motd", LSCChannel::normal, "amarr", 1, true, password, false, cspa, 0, 3);
    CreateChannel(18, "Empires\\Minmatar", "motd", LSCChannel::normal, "minmatar", 1, true, password, false, cspa, 0, 3);
    CreateChannel(19, "Empires\\Gallente", "motd", LSCChannel::normal, "gallente", 1, true, password, false, cspa, 0, 3);
    CreateChannel(20, "Empires\\Jove", "motd", LSCChannel::normal, "jove", 1, true, password, false, cspa, 0, 3);
    CreateChannel(21, "Alliances\\Smacktalk", "motd", LSCChannel::normal, "smacktalk", 1, true, password, false, cspa, 0, 3);
    CreateChannel(22, "Alliances\\Rumour Mill", "motd", LSCChannel::normal, "rumourmill", 1, true, password, false, cspa, 0, 3);
    CreateChannel(23, "Alliances\\Freelancer", "motd", LSCChannel::normal, "freelancer", 1, true, password, false, cspa, 0, 3);
    CreateChannel(24, "Corporate\\Recruitment", "motd", LSCChannel::normal, "recruitment", 1, true, password, false, cspa, 0, 3);
    CreateChannel(25, "Corporate\\CEO", "motd", LSCChannel::normal, "ceo", 1, true, password, false, cspa, 0, 3);
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
void Client::SelfEveMail( const char* subject, const char* fmt, ... )
{
    va_list args;
    va_start( args, fmt );

    char* str = NULL;
    vasprintf( &str, fmt, args );
    assert( str );

    va_end( args );

    m_services.lsc_service->SendMail( GetCharacterID(), GetCharacterID(), subject, str );
    SafeFree( str );
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
    if( arg.channel->IsInt() )
        channelID = arg.channel->AsInt()->value();
    else if( arg.channel->IsTuple() )
    {
        PyTuple* prt = arg.channel->AsTuple();

        if( prt->GetItem( 0 )->IsInt() )
            channelID = prt->GetItem( 0 )->AsInt()->value();
        else if( prt->GetItem( 0 )->IsTuple() )
        {
            prt = prt->GetItem( 0 )->AsTuple();

            if( prt->items.size() != 2 || !prt->GetItem( 1 )->IsInt() )
            {
                codelog(SERVICE__ERROR, "%s: Bad arguments", call.client->GetName());
                return nullptr;
            }

            channelID = prt->GetItem( 1 )->AsInt()->value();
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

    if( m_channels.find( channelID ) != m_channels.end() )
        return m_channels[channelID]->EncodeChannelChars();

    return nullptr;
}


PyResult LSCService::Handle_GetMember(PyCallArgs &call) {
    return nullptr;
}


PyResult LSCService::ExecuteCommand(Client *from, const char *msg) {
    return(m_commandDispatch->Execute(from, msg));
}
