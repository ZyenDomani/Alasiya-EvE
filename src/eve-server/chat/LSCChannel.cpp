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

#include "../eve-server.h"
#include "EVEVersion.h"

#include "../Client.h"
#include "chat/LSCChannel.h"
#include "chat/LSCService.h"
#include "npc/NPC.h"


//TODO:  verifiy this!!  is this right??
PyRep* LSCChannelChar::Encode() const {
    Client* pClient = sEntityMgr.FindClientByCharID(m_charID);
    if (pClient == nullptr)
        return nullptr;
    ChannelCharsLine line;
    line.allianceID = m_allianceID;
    line.charID = m_charID;
    line.corpID = m_corpID;
    line.role = m_role;
    line.warFactionID = m_warFactionID;
    line.mode = m_mode;
    LSC_SenderEOL* eol = new LSC_SenderEOL();
        eol->charID = m_charID;
        eol->charName = pClient->GetChar()->itemName();
        eol->typeID = pClient->GetChar()->typeID();
        eol->gender = pClient->GetChar()->gender();
        eol->charNameID = PyStatic.NewNone();
    line.extra = eol->Encode();
    return line.Encode();
}

PyRep* AclEntry::Encode() const {
    ChannelACLLine line;
    line.accessor = accessorID;
    line.admin = adminID;
    line.mode = mode;
    line.originalMode = originalMode;
    line.reason = reason;
    line.untilWhen = untilWhen;
    return line.Encode();
}

LSCChannel::LSCChannel(LSCService* svc,
                       int32 channelID,
                       int32 ownerID/*ownerSystem*/,
                       LSC::Type type/*LSC::Type::normal*/,
                       const char* motd/*nullptr*/,
                       const char* displayName/*nullptr*/,
                       const char* comparisonKey/*nullptr*/,
                       int32 gMsgID/*0*/,
                       int32 cMsgID/*0*/,
                       bool memberless/*false*/,
                       bool temporary/*false*/)
: m_service(svc),
  m_ownerID(ownerID),
  m_channelID(channelID),
  m_type(type),
  m_motd(motd==nullptr?"":motd),
  m_displayName(displayName==nullptr?"":displayName),
  m_comparisonKey(comparisonKey==nullptr?"":comparisonKey),
  m_memberless(memberless),
  m_password(""),
  m_mailingList(false),
  m_cspa(2950),         //change this to static const
  m_temporary(temporary),
  m_languageRestriction(false),
  m_gMsgID(gMsgID),
  m_cMsgID(cMsgID)
{
	m_service->GetDB()->LoadChannelACL(channelID, m_aclMap);

    _log(LSC__CHANNELS, "Creating channel %u(%s:%s) - type: %s", m_channelID, \
                        (m_displayName == "") ? "null" : m_displayName.c_str(), \
                        (m_comparisonKey == "") ? "null" : m_comparisonKey.c_str(),
                        GetTypeString());
}

LSCChannel::~LSCChannel() {
    _log(LSC__CHANNELS, "Destroying channel %u - \"%s\"", m_channelID, (m_displayName == "") ? ((m_comparisonKey == "") ? "null" : m_comparisonKey.c_str()) : m_displayName.c_str());
}

void LSCChannel::InitACL(Client* pClient) {
	int32 charID = pClient->GetCharacterID();
	AclEntry* entry = new AclEntry(charID, LSC::Mode::Creator, 0, LSC::Mode::None, "Channel Creation", charID);

	m_aclMap.emplace(charID, entry);
        m_service->GetDB()->SaveChannelACL(m_channelID, entry);
}

void LSCChannel::GetChannelInfo(int32* channelID, uint32* ownerID, std::string& displayName, std::string& motd, \
                                std::string& comparisonKey, bool* memberless, std::string& password, bool* mailingList, \
                                uint32* cspa, uint32* temporary)
{
    *channelID = m_channelID;
    *ownerID = m_ownerID;
    displayName = m_displayName;
    motd = m_motd;
    comparisonKey = m_comparisonKey;
    *memberless = m_memberless;
    password = m_password;
    *mailingList = m_mailingList;
    *cspa = m_cspa;
    *temporary = m_temporary;
}

bool LSCChannel::JoinChannel(Client* pClient) {
    /** @todo determine moderator/other rights for given char in this channel and set Mode accordingly */
    m_chars.emplace(pClient->GetCharacterID(),
            LSCChannelChar(this, pClient->GetCorporationID(), pClient->GetCharacterID(), pClient->GetName(), \
                           pClient->GetAllianceID(), pClient->GetWarFactionID(), pClient->GetAccountRole(), 0, \
                           (m_ownerID == pClient->GetCharacterID() ? LSC::Mode::Creator : LSC::Mode::Moderator))
            );
    pClient->ChannelJoined( this );

    LSC_JoinChannel join;
        join.channelID = EncodeID();
        join.memberCount = (int32)m_chars.size();
        join.sender = MakeSenderInfo(pClient);
        join.args = PyStatic.mtTuple();
    PyTuple* rsp = join.Encode();

    if (is_log_enabled(LSC__RSP_DUMP)) {
        _log(LSC__RSP_DUMP, "LSC JoinChannel:");
        rsp->Dump(LSC__RSP_DUMP, "    ");
    }

    MulticastTarget mct;
    for (const auto& cur : m_chars)
        mct.characters.insert(cur.first);
    sEntityMgr.Multicast("OnLSC", GetTypeString(), &rsp, mct, false);

    _log(LSC__CHANNELS, "%s Joined Channel %u - %s", pClient->GetName(), m_channelID, m_displayName.c_str());
    return true;
}

void LSCChannel::LeaveChannel(Client* pClient) {
    if (pClient == nullptr)
        return;

    // on logout, this is false.  removing channels here will invalidate pointer used to call 'LeaveChannel'
    if (pClient->IsLoaded())
        pClient->ChannelLeft(this);

    uint32 charID = pClient->GetCharacterID();

    // remove char before mcast (for logout fix)
    m_chars.erase(charID);

    if (m_chars.empty()) {
        if (m_temporary)
            m_service->DestroyChannel(m_channelID);
        return;
    }

    if (m_chars.find(charID) == m_chars.end())
        return;

    LSC_LeaveChannel leave;
    leave.sender = MakeSenderInfo(pClient);
    leave.memberCount = (int32)m_chars.size();
    leave.channelID = EncodeID();
    leave.args = PyStatic.mtTuple();
    PyTuple* rsp = leave.Encode();

    if (is_log_enabled(LSC__RSP_DUMP)) {
        _log(LSC__RSP_DUMP, "LSC LeaveChannel:");
        rsp->Dump(LSC__RSP_DUMP, "    ");
    }

    MulticastTarget mct;
    for (const auto& cur : m_chars)
        mct.characters.insert( cur.first );
    sEntityMgr.Multicast("OnLSC", GetTypeString(), &rsp, mct, false);

    _log(LSC__CHANNELS, "%s Left Channel %u - %s", pClient->GetName(), m_channelID, m_displayName.c_str());
}

void LSCChannel::Evacuate(Client* pClient) {
    LSC_DestroyChannel dc;
    dc.channelID = EncodeID();
    dc.memberCount = 0;
    dc.sender = MakeSenderInfo(pClient);
    dc.args = PyStatic.mtTuple();

    MulticastTarget mct;
    for (const auto& cur : m_chars)
        mct.characters.insert(cur.first);

    PyTuple* rsp = dc.Encode();
    sEntityMgr.Multicast("OnLSC", GetTypeString(), &rsp, mct, false);
}

void LSCChannel::SendMessage(Client* pClient, std::string& message, bool self/*false*/) {
    // to send system msgs, senderID should be 1 (system owner)
    // will need better arg checks to determine actual message origin, unless we overload and/or keep this for player-only
    uint32 senderID = pClient ? pClient->GetCharacterID() : 1;

    auto it = m_chars.find(senderID);
    if (it != m_chars.end() /*&& it->second != nullptr*/) {
        // CHTMODE_LISTENER = 1. If their mode drops below Speaker (2), they are read-only!
        if (static_cast<int8_t>(it->second.GetMode()) < 2) {
            _log(LSC__CHANNELS, "SendMessage blocked. Character %u is gagged/muted in room %u.", senderID, m_channelID);
            return;
        }
    }

    MulticastTarget mct;
    LSC_SendMessage sm;

    if (self and !IsCharacterID(senderID)) {
        mct.characters.insert(senderID);
        sm.sender = FakeSenderInfo();
    } else {
        for (const auto& cur : m_chars)
            mct.characters.insert(cur.first);

        sm.sender = MakeSenderInfo(pClient);
    }

    sm.channelID = EncodeID();
    sm.memberCount = static_cast<int32>(m_chars.size());
    sm.message = message;

    PyTuple* rsp = sm.Encode();

    if (is_log_enabled(LSC__MESSAGE)) {
        _log(LSC__MESSAGE, "LSC SendMessage:");
        rsp->Dump(LSC__MESSAGE, "    ");
    }

    sEntityMgr.Multicast("OnLSC", GetTypeString(), &rsp, mct, false);
}

// this works...leave it alone
void LSCChannel::SendServerMOTD(Client* pClient) {
    std::string uptime = sEntityMgr.GetUpTime();
    std::string msg = "<br>Welcome to Alasiya's EvEmu Server ";
    msg += pClient->GetCharName();
    msg += ".<br>Server Version: ";
    msg += EVEMU_REVISION;
    msg += "<br>Revision Date: ";
    msg += EVEMU_BUILD_DATE;
    msg += "<br>Uptime: ";
    msg += uptime;
    msg += "<br>Current Population: ";
    msg += std::to_string(sEntityMgr.GetClientCount());
    // TODO:  update to use new color enums
    msg += "<br><br><color=" + std::string(LSC::Color::Yellow.hexStr) + ">Character Options:</color>";
    msg += "<br><font color='white'>Ship Tracking: </font>";
    if (sEntityMgr.GetTracking()) {
        msg += "<font color='green'>On</font>";
    } else {
        msg += "<font color='red'>Off</font>";
    }
    msg += "<br><font color='white'>Module AutoStop: </font>";
    if (pClient->AutoStop()) {
        msg += "<font color='green'>On</font>";
    } else {
        msg += "<font color='red'>Off</font>";
    }
    msg += "<br><font color='white'>Drone AutoAttack: </font>";
    if (pClient->AutoAttack()) {
        msg += "<font color='green'>On</font>";
    } else {
        msg += "<font color='red'>Off</font>";
    }
    msg += "<br><font color='white'>RAM Event: </font>";
    //msg += (pClient->RAMEvent() ? "On" : "Off");
    if (sConfig.ram.AutoEvent) {
        msg += "<font color='green'>On</font>";
    } else {
        msg += "<font color='red'>Off</font>";
    }
    // check account roles for this one
    if ((pClient->GetAccountRole() & Acct::Role::EPLAYER) == Acct::Role::EPLAYER) {
        msg += "<br><font color='white'>ShowAll: </font>";
        if (pClient->IsShowall()) {
            msg += "<font color='green'>On</font>";
        } else {
            msg += "<font color='red'>Off</font>";
        }
    }

    LSC_SendMessage sm;
    sm.sender = FakeSenderInfo();
    sm.channelID = EncodeID();
    sm.message = msg;
    sm.memberCount = static_cast<int32>(m_chars.size());

    PyTuple* motd = sm.Encode();
    motd->Dump(LSC__RSP_DUMP, "   ");
    //NOTE:  this is to send chat msg to single char using <this> window (searched via id/name from sender)
    pClient->SendNotification("OnLSC", GetTypeString(), &motd, false);
}

bool LSCChannel::IsJoined(uint32 charID) {
    return (m_chars.find(charID) != m_chars.end());
}

bool LSCChannel::IsOperatorOrHigher(int32 charID, int32 corpID, int64 corpFlags) {
    // Rule A: The absolute room Creator (15) always bypasses structural authorization gates
    if (charID == m_ownerID)
        return true;

    // Rule B: Executive Corporate Check (Promotes active directors/managers to internal Operators)
    if (corpID == m_ownerID) {
        if ((corpFlags & Corp::Role::Director) == Corp::Role::Director ||
            (corpFlags & Corp::Role::ChatManager) == Corp::Role::ChatManager)
        {
            return true;
        }
    }

    // Rule C: Scan the live, pointer-stabilized ACL memory map for specific overrides
    auto it = m_aclMap.find(charID);
    if (it != m_aclMap.end() && it->second != nullptr) {
        AclEntry* acl = it->second;
        // Ensure an absolute timed restriction hasn't passed yet
        if (acl->untilWhen == 0 || GetFileTimeNow() < acl->untilWhen) {
            if (acl->mode >= LSC::Mode::Operator) {
                return true;
            }
        }
    }

    return false;
}

bool LSCChannel::IsModeratorOrHigher(int32 charID, int32 corpID, int64 corpFlags) {
    // Rule A: Operators and Creators inherently carry Moderator privileges
    if (IsOperatorOrHigher(charID, corpID, corpFlags))
        return true;

    if ((corpID == m_ownerID) && (corpFlags & Corp::Role::PersonnelManager) == Corp::Role::PersonnelManager)
        return true;

    // Rule B: Scan the in-memory ACL maps specifically looking for Mode::Moderator (3)
    auto it = m_aclMap.find(charID);
    if (it != m_aclMap.end() && it->second != nullptr) {
        AclEntry* acl = it->second;

        // Ensure an absolute timed restriction hasn't passed yet
        if (acl->untilWhen == 0 || GetFileTimeNow() < acl->untilWhen) {
            if (acl->mode >= LSC::Mode::Moderator) {
                return true;
            }
        }
    }

    //nope
    return false;
}

bool LSCChannel::IsBanned(uint32 charID, uint32 corpID, uint32 allyID) {
    // Creator/Owner always bypasses all access restriction loops
    if (charID == m_ownerID)
        return false;

    int64 currentTime = GetFileTimeNow();

    // 1. Gather all possible accessor IDs that apply to this connecting client
    // Allows us to catch personal character bans, corp bans, or alliance bans in a single pass
    uint32 accessors[3] = { charID, corpID, allyID };

    for (int i = 0; i < 3; ++i) {
        uint32 currentID = accessors[i];
        if (currentID == 0)
            continue; // Skip unassigned IDs (e.g., if allianceID is 0)

            auto it = m_aclMap.find(currentID);
        if (it != m_aclMap.end() && it->second != nullptr) {
            AclEntry* acl = it->second;

            // 2. Mode Evaluation: Mode::None (0) or Mode::Disallowed (-2) represent active bans
            if (acl->mode == LSC::Mode::None || acl->mode == LSC::Mode::Disallowed) {
                _log(LSC__CHANNELS, "Access Denied: Accessor %u is explicitly banned from room %u. Reason: %s",
                     currentID, m_channelID, acl->reason.c_str());
                return true; // Explicitly banned
            }

            // 3. Temporal Evaluation: Check if a timed ban has already expired
            if (acl->untilWhen > 0 && currentTime >= acl->untilWhen) {
                _log(LSC__CHANNELS, "Access Control: Timed restriction expired for Accessor %u in room %u. Purging entry.",
                     currentID, m_channelID);

                m_service->GetDB()->RemoveChannelACL(m_channelID, currentID);
                SafeDelete(acl);
                m_aclMap.erase(it);
                continue;
            }
        }
    }

    return false;
}

void LSCChannel::AnnouncePresence(Client* pClient, int8 appliedMode) {
    if (pClient == nullptr)
        return;

    BroadcastEvent("AnnouncePresence", PyStatic.mtTuple());
}

void LSCChannel::BroadcastEvent(const std::string& method, PyTuple* args) {
    std::set<uint32> roomRoster;
    for (const auto& cur : m_chars)
        roomRoster.insert(cur.first);

    if (is_log_enabled(LSC__RSP_DUMP)) {
        _log(LSC__RSP_DUMP, "LSC BCast: %s", method.c_str());
        args->Dump(LSC__RSP_DUMP, "    ");
    }

    if (!roomRoster.empty())
        sEntityMgr.Multicast(roomRoster, "OnLSC", GetTypeString(), &args, false);
}

LSC_SenderInfo* LSCChannel::MakeSenderInfo(Client* pClient/*nullptr*/, NPC* pNPC/*nullptr*/) {
    if ((pClient == nullptr) && (pNPC == nullptr))
        return FakeSenderInfo();

    LSC_SenderInfo* sender = new LSC_SenderInfo();
    sender->allianceID = pClient->GetAllianceID();
    sender->corpID = pClient->GetCorporationID();
    sender->whoEOL = MakeSenderEOL(pClient, pNPC);
    sender->chatMode = LSC::Mode::Creator; //pClient->GetChar()->chatMode(channelID);
    sender->corpRole = pClient->GetCorpRole();
    sender->factionID = (pClient->GetWarFactionID() > 0 ? new PyInt(pClient->GetWarFactionID()) : PyStatic.NewNone());
    return sender;
}

LSC_SenderInfo* LSCChannel::FakeSenderInfo() {
    LSC_SenderInfo* sender = new LSC_SenderInfo();
    sender->allianceID = 0;
    sender->corpID = 0; // should this be sov holder in <channel> space?
    sender->whoEOL = MakeSenderEOL();
    sender->chatMode = LSC::Mode::Creator;
    sender->corpRole = 0;
    sender->factionID = PyStatic.NewNone();
    return sender;
}

LSC_SenderEOL* LSCChannel::MakeSenderEOL(Client* pClient/*nullptr*/, NPC* pNPC/*nullptr*/) {
    LSC_SenderEOL* sender = new LSC_SenderEOL();
    if (pClient != nullptr) {
        // character
        sender->charID = pClient->GetChar()->itemID();
        sender->charName = pClient->GetChar()->itemName();
        sender->typeID = pClient->GetChar()->typeID();
        sender->gender = pClient->GetChar()->gender();
        sender->charNameID = PyStatic.NewNone();
    } else if (pNPC != nullptr) {
        // npc - this *may not* work right...
        sender->charID = pNPC->GetID();
        sender->charName = pNPC->GetName();
        sender->typeID = pNPC->GetTypeID();
        sender->gender = true;
        sender->charNameID = PyStatic.NewNone();//pNPC->GetTypeNameID(); //most npc have this as 'typeNameID', but its neither pulled nor saved; it should be
    } else {
        // system/owner
        sender->charID = 1;
        sender->charName = "EvE System";
        sender->typeID = 0;
        sender->gender = true;
        sender->charNameID = new PyInt(67718);  //#System
    }

    return sender;
}

PyRep* LSCChannel::EncodeID() {
    switch (m_type) {
        // --- Category A: Pure Solar System / Static Base Space ---
        case LSC::Type::corp:
        case LSC::Type::global:
        case LSC::Type::region:
        case LSC::Type::alliance:
        case LSC::Type::warfaction:
        case LSC::Type::solarsystem:    //this is 'system' in w-space
        case LSC::Type::solarsystem2:   //this is 'local' in k-space
        case LSC::Type::constellation: {
            PyTuple* outer = new PyTuple(1);
                PyTuple* inner = new PyTuple(2);
                    inner->SetItemString(0, GetTypeString());
                    inner->SetItemInt(1, m_channelID);
                outer->SetItem(0, inner);
            return outer;
        }

        // --- Category B: Dynamic Player Created Custom Rooms ---
        case LSC::Type::custom: {
            // Fulfills your exact offset formula note perfectly
            int64 actualChannelID = -1 * (static_cast<int64>(m_channelID) + 2147483647);
            return new PyInt(static_cast<int32>(actualChannelID));
        }

        // --- Category C: Fleet Channels ---
        case LSC::Type::fleet: {
            // Fulfills the exact wire requirements to allow fleet voice chat: PyString("fleetid:XXXX")
            std::string wireID = "fleetid:" + std::to_string(m_channelID);
            return new PyString(wireID.c_str());
        }
        case LSC::Type::wing: {
            // Fulfills the exact wire requirements to allow fleet voice chat: PyString("wingid:XXXX")
            std::string wireID = "wingid:" + std::to_string(m_channelID);
            return new PyString(wireID.c_str());
        }
        case LSC::Type::squad: {
            // Fulfills the exact wire requirements to allow fleet voice chat: PyString("squadid:XXXX")
            std::string wireID = "squadid:" + std::to_string(m_channelID);
            return new PyString(wireID.c_str());
        }

        case LSC::Type::character: {
            sLog.Warning("LSC::EncodeID", "type::char hit for channel %i", m_channelID);
            // fallthru and let it continue...not sure what else to do yet...
        }
        default:
            return new PyInt(m_channelID);
    }
}

PyRep* LSCChannel::EncodeStaticChannel(uint32 charID) {
    MultiChannelLine line;
        line.channelID = m_channelID;
        line.ownerID = m_ownerID;
        line.displayName = (m_displayName.empty() ? PyStatic.NewNone() : new PyString(m_displayName));
        line.motd = m_motd;
        line.comparisonKey = m_comparisonKey;
        line.memberless = m_memberless;
        line.password = m_password;
        line.mailingList = m_mailingList;
        line.cspa = m_cspa;
        line.temporary = m_temporary;
        line.languageRestriction = m_languageRestriction;
        line.groupMessageID = m_gMsgID;
        line.channelMessageID = m_cMsgID;
        line.estimatedMemberCount = (int32)m_chars.size();

        // This (may) tint the tab title bar or chat room icons
        if (m_type == LSC::Type::fleet) {
            line.channelColor = LSC::Color::Azure.clientInt; // Blue fleet theme
        } else if (m_type == LSC::Type::corp) {
            line.channelColor = LSC::Color::PureGreen.clientInt; // Green corporate theme
        } else {
            line.channelColor = LSC::Color::LiteGrey.clientInt; // Standard grey background
        }

        if (m_type <= LSC::Type::solarsystem2) {
            line.subscribed = true;
        } else {
            //TODO:  get this working, then pull subscribed channels into client on login, updated as needed.
            line.subscribed = m_service->GetDB()->IsChannelSubscribedByThisChar(m_channelID, charID);
        }

    return line.Encode();
}

PyRep* LSCChannel::EncodeDynamicChannel(uint32 charID) {
    /*
              [PyPackedRow 27 bytes]
                ["channelID" => <1630077495> [I4]]
                ["ownerID" => <1630077495> [I4]]
                ["displayName" => <None> [WStr]]
                ["motd" => <None> [WStr]]
                ["comparisonKey" => <None> [WStr]]
                ["memberless" => <1> [Bool]]
                ["password" => <None> [WStr]]
                ["mailingList" => <1> [Bool]]
                ["cspa" => <2950> [I4]]
                ["temporary" => <0> [Bool]]
                ["languageRestriction" => <0> [Bool]]
                ["groupMessageID" => <0> [I4]]
                ["channelMessageID" => <0> [I4]]
                ["subscribed" => <1> [I4]]
        */
   // PyPackedRow row;
    SingleChannelInfo info;
        info.channelID = m_channelID;
        info.cspa = m_cspa;
        if (m_displayName.empty()) {
            info.displayName = PyStatic.NewNone();
            info.comparisonKey = PyStatic.NewNone();
        } else {
            info.displayName = new PyString(m_displayName);
            info.comparisonKey = new PyString(m_comparisonKey);
        }
        info.mailingList = m_mailingList;
        info.memberless = m_memberless;
        info.motd = m_motd;
        info.ownerID = m_ownerID;
        info.password = m_password;
        info.temporary = m_temporary;
        info.languageRestriction = m_languageRestriction;
        info.groupMessageID = m_gMsgID;
        info.channelMessageID = m_cMsgID;
        info.estimatedMemberCount = (int32)m_chars.size();
        if (m_type <= LSC::Type::solarsystem2) {
            info.subscribed = true;
        } else {
            info.subscribed = m_service->GetDB()->IsChannelSubscribedByThisChar(m_channelID, charID);
        }

        // This tints the tab title bar or chat room icons natively in the UI.
        if (m_type == LSC::Type::fleet) {
            info.channelColor = LSC::Color::Azure.clientInt; // Blue fleet theme
        } else if (m_type == LSC::Type::corp) {
            info.channelColor = LSC::Color::PureGreen.clientInt; // Green corporate theme
        } else {
            info.channelColor = LSC::Color::LiteGrey.clientInt; // Standard grey background
        }

    return info.Encode();
}

PyRep* LSCChannel::EncodeChannelACL()
{
    ChannelACL info;
    info.lines = new PyList();
    for (const auto& cur : m_aclMap)
        info.lines->AddItem(cur.second->Encode());
    return info.Encode();
}

PyRep* LSCChannel::EncodeChannelChars() {
    ChannelChars info;
    info.lines = new PyList();
    for (const auto& cur : m_chars)
        info.lines->AddItem(cur.second.Encode());
    return info.Encode();
}

PyRep* LSCChannel::EncodeEmptyChannelChars() {
    ChannelChars info;
    info.lines = new PyList();
    return info.Encode();
}

PyPackedRow* LSCChannel::CreatePackedRow(DBRowDescriptor* header, Client* pClient) {
    PyPackedRow* res = new PyPackedRow(header);
    // [0] channelID -> I4 (Int32) or specialized type representation
    res->SetField(0, EncodeID());
    // [1] ownerID -> I4
    res->SetField(1, new PyInt(m_ownerID));
    // [2] displayName -> WStr (Wide String / None)
    if (!m_displayName.empty()) {
        res->SetField(2, new PyString(m_displayName));
    } else {
        res->SetField(2, PyStatic.NewNone());
    }
    // [3] motd -> WStr
    if (!m_motd.empty()) {
        res->SetField(3, new PyString(m_motd));
    } else {
        res->SetField(3, PyStatic.NewNone());
    }
    // [4] comparisonKey -> WStr
    if (!m_comparisonKey.empty()) {
        res->SetField(4, new PyString(m_comparisonKey));
    } else {
        res->SetField(4, PyStatic.NewNone());
    }
    // [5] memberless -> Bool
    res->SetField(5, new PyBool(m_memberless));
    // [6] password -> WStr
    if (!m_password.empty()) {
        res->SetField(6, new PyString(m_password));
    } else {
        res->SetField(6, PyStatic.NewNone());
    }
    // [7] mailingList -> Bool
    res->SetField(7, new PyBool(m_mailingList));
    // [8] cspa -> I4
    res->SetField(8, new PyInt(m_cspa));
    // [9] temporary -> Bool
    res->SetField(9, new PyBool(m_temporary));
    // [10] languageRestriction -> Bool
    res->SetField(10, new PyBool(m_languageRestriction));
    // [11] groupMessageID -> I4
    res->SetField(11, new PyInt(m_gMsgID));
    // [12] channelMessageID -> I4
    res->SetField(12, new PyInt(m_cMsgID));
    // [13] subscribed -> I4
    if (pClient != nullptr) {
        res->SetField(13, PyStatic.NewOne());
    } else {
        res->SetField(13, PyStatic.NewZero());
    }

    return res;
}

const char* LSCChannel::GetTypeString() {
    switch(m_type) {
        case LSC::Type::normal:         return "normal";
        case LSC::Type::custom:         return "private";
        case LSC::Type::corp:           return "corpid";
        case LSC::Type::solarsystem:    return "solarsystemid";
        case LSC::Type::solarsystem2:   return "solarsystemid2";
        case LSC::Type::region:         return "regionid";
        case LSC::Type::constellation:  return "constellationid";
        case LSC::Type::global:         return "global";
        case LSC::Type::alliance:       return "allianceid";
        case LSC::Type::fleet:          return "fleetid";
        case LSC::Type::squad:          return "squadid";
        case LSC::Type::wing:           return "wingid";
        case LSC::Type::warfaction:     return "warfactionid";
        case LSC::Type::character:      return "charid";
        case LSC::Type::incursion:      return "incursion";
        default:                        return "unknown";
    }

    return "invalid";
}

