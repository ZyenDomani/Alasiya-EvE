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
    Author:        Zhur
    Rewrite:    Allan
*/

#ifndef __LSCCHANNEL_H_INCL__
#define __LSCCHANNEL_H_INCL__

#include "EntityMgr.h"
#include "EVE_LSC.h"
#include "packets/LSCPkts.h"

class NPC;
class PyRep;
class LSCService;
class LSCChannel;

class LSCChannelChar {
public:
    LSCChannelChar(LSCChannel *chan, uint32 corpID, uint32 charID, std::string charName, uint32 allianceID, uint32 warFactionID, int64 role, uint32 extra, LSC::Mode mode) :
      m_parent(chan),
      m_corpID(corpID),
      m_charID(charID),
      m_charName(charName),
      m_allianceID(allianceID),
      m_warFactionID(warFactionID),
      m_role(role),
      m_extra(extra),
      m_mode(mode) { }

    ~LSCChannelChar() { }
    PyRep *Encode() const;

    LSC::Mode GetMode() const                           { return m_mode; }
    void SetMode(LSC::Mode mode)                        { m_mode = mode; }

protected:
    LSCChannel *m_parent;
    uint32 m_corpID;
    uint32 m_charID;
    std::string m_charName;
    uint32 m_allianceID;
    uint32 m_warFactionID;
    int64 m_role;
    uint32 m_extra;
    LSC::Mode m_mode;
};

// Struct tracking a live ACL rule entry in memory
struct AclEntry {
    int8 mode;       // None, Listener, Speaker, Moderator, etc.
    int8 originalMode;
    uint32 adminID;
    uint32 accessorID;    // Character ID, Corp ID, or Alliance ID
    int64 untilWhen;     // 64-bit absolute Win32 FileTime expiry timestamp (0 = permanent)
    std::string reason;

    AclEntry(uint32 acc, int8 md, int64 until, int8 orig, std::string reas, uint32 adm)
    : accessorID(acc), mode(md), untilWhen(until), originalMode(orig), reason(reas), adminID(adm) {}

    PyRep *Encode() const;
};

class LSCChannelMod {
public:
    LSCChannelMod(LSCChannel * chan, uint32 accessor, int64 untilWhen, uint32 originalMode, std::string admin, std::string reason, LSC::Mode mode) :
      m_parent(chan),
      m_accessor(accessor),
      m_mode(mode),
      m_untilWhen(untilWhen),
      m_originalMode(originalMode),
      m_admin(admin),
      m_reason(reason) { }

    ~LSCChannelMod() { }
    PyRep * Encode();

protected:
    LSCChannel * m_parent;    // we do not own this
    uint32 m_accessor;
    LSC::Mode m_mode;
    int64 m_untilWhen;
    uint32 m_originalMode;
    std::string m_admin;
    std::string m_reason;
};

class LSCChannel {
public:
    LSCChannel(LSCService* svc,
               int32 channelID,
               int32 ownerID=ownerSystem,
               LSC::Type type=LSC::Type::normal,
               const char* motd=nullptr,
               const char* displayName=nullptr,
               const char* comparisonKey=nullptr,
               int32 gMsgID=0,
               int32 cMsgID=0,
               bool memberless=false,
               bool temporary=false);
    LSCChannel(LSCChannel&&) =delete;
    LSCChannel(const LSCChannel&) =delete;
    LSCChannel& operator=(LSCChannel&&) =delete;
    LSCChannel& operator=(const LSCChannel&) =delete;
    ~LSCChannel();

    PyRep *EncodeID();
    PyRep *EncodeStaticChannel(uint32 charID);
    PyRep *EncodeDynamicChannel(uint32 charID);
    PyRep *EncodeChannelMods();
    PyRep *EncodeChannelChars();
    PyRep *EncodeEmptyChannelChars();

    const char *GetTypeString();
    bool JoinChannel(Client *pClient);
    void LeaveChannel(Client* pClient);
    bool IsJoined(uint32 charID);
    bool IsBanned(uint32 charID, uint32 corpID, uint32 allyID);

    bool IsOperatorOrHigher(int32 charID, int32 corpID, int64 corpFlags);
    bool IsModeratorOrHigher(int32 charID, int32 corpID, int64 corpFlags);

    void Evacuate(Client* pClient);
    void SendServerMOTD(Client* pClient);
    void SendMessage(Client* pClient, std::string& message, bool self = false);

    void GetChannelInfo(int32 * channelID, uint32 * ownerID, std::string &displayName, std::string &motd, std::string &comparisonKey,
            bool * memberless, std::string &password, bool * mailingList, uint32 * cspa, uint32 * temporary);

    void AnnouncePresence(Client* pClient, int8 appliedMode);
    void BroadcastEvent( const std::string& method, PyTuple* args );

    // setters
    void SetDisplayName(const std::string& displayName) { m_displayName = displayName; }
    void SetMOTD(const std::string& motd)               { m_motd = motd; }
    void SetMemberless(bool memberless)                 { m_memberless = memberless; }
    void SetPassword(const std::string& password)       { m_password = password; }
    void SetType(LSC::Type type)                        { m_type = type; }
    void SetLanguageRestriction(bool restriction)       { m_languageRestriction = restriction; }

    // getters
    bool                GetMemberless()                 { return m_memberless; }
    bool                GetMailingList()                { return m_mailingList; }
    bool                GetTemporary()                  { return m_temporary; }
    int32               GetGrpMsgID()                   { return m_gMsgID; }
    int32               GetChMsgID()                    { return m_cMsgID; }
    uint16              GetCSPA()                       { return m_cspa; }
    uint32              GetOwnerID()                    { return m_ownerID; }
    int32               GetChannelID()                  { return m_channelID; }
    uint32              GetMemberCount()                { return (uint32)m_chars.size(); }
    LSC::Type           GetType()                       { return m_type; }
    std::string         GetDisplayName()                { return m_displayName; }
    std::string         GetMOTD()                       { return m_motd; }
    std::string         GetComparisonKey()              { return m_comparisonKey; }
    std::string         GetPassword()                   { return m_password; }

    std::map<uint32, LSCChannelChar> m_chars;

    LSC_SenderEOL* MakeSenderEOL( Client* pClient = nullptr, NPC* pNPC = nullptr );
    LSC_SenderInfo* MakeSenderInfo(Client* pClient=nullptr, NPC* pNPC=nullptr);
    LSC_SenderInfo* FakeSenderInfo();

protected:
    LSCService *const   m_service;    //we do not own this

    LSC::Type           m_type;
    // memberless - true = estimate member count, send estimatedMemberCount in packet.  false = actual memberList.count()   (5m refresh in client)
    // non-npc corp, fleet, and k-space local are always memberless=false
    bool                m_memberless;
    bool                m_mailingList;
    bool                m_temporary;
    // true = english only; false = any language
    bool                m_languageRestriction;
    // -1 for static and 'session-type' channels, must be !=0 to show 'locale.displayName'
    int32               m_gMsgID;
    // if msgID = -1, name = raw displayName if !None else locale.msgID
    int32               m_cMsgID;
    uint16              m_cspa;
    uint32              m_ownerID;
    int32               m_channelID;            // ids < 0 are automatic conversationalist mode (or creator) and invite only (per client)
    std::string         m_displayName;
    std::string         m_motd;
    std::string         m_comparisonKey;
    std::string         m_password;

    std::map<uint32, LSCChannelMod> m_mods;
    // Fast O(1) in-memory registrie
    std::unordered_map<uint32, AclEntry*> m_aclMap;             //charID/data

};

#endif

/*  channel title naming and setting type, owner and messageIDs
 * if channelID = integer
 *   if temp
 *     title + otherCharName
 *     if displayName != None
 *       title = displayName
 *     else
 *       title prepend one of (group, groupalone, private, privatealone)
 *   elseif gMid != 0
 *     if cMid = -1 (systemChannel)
 *       if displayName != None
 *         title = displayName
 *       else
 *         title = "SystemChannels"
 *     elseif displayName != None
 *       title = displayName
 *     else
 *       title = cMid
 *   elseif ownerID = 1 (ownerSystem)
 *     title = cMid
 *   else
 *     title = displayName
 * else (this is for channelID[k, v] where k=type, v=channelID)
 *   title = <channel type string>
 *
 * <if nothing above hit:>
 * title = str(channelID)
 */


/*
In the raw Python client, ChannelID is defined as a generic PyRep* variant.
For a solar system channel, EncodeID() must return a PyInt(solarSystemID).
For a custom channel, EncodeID() must return a PyInt(-1 * (customID + Offset)).
For a fleet channel, EncodeID() must return a PyString("fleetid:XXXX").
custom channel id = -1 * (channelID + 2147483647)
*/
