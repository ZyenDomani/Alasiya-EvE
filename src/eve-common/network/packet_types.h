/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2016 The EVEmu Team
 *    For the latest information visit http://evemu.org
 *    ------------------------------------------------------------------------------------
 *    This program is free software; you can redistribute it and/or modify it under
 *    the terms of the GNU Lesser General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option) any later
 *    version.
 *
 *    This program is distributed in the hope that it will be useful, but WITHOUT
 *    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License along with
 *    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 *    http://www.gnu.org/copyleft/lesser.txt.
 *    ------------------------------------------------------------------------------------
 *    Author:        Zhur
 *    Updates:       Allan
 */

/** @todo  this file really isnt named correctly.  move like items into new files named accordingly. */
//  wip  -allan 29May15

#ifndef EVE_PACKET_TYPES_H
#define EVE_PACKET_TYPES_H

#include "../EVE_Corp.h"
#include "../EVE_Effects.h"
#include "../EVE_Inventory.h"
#include "../EVE_Wallet.h"


/*
 *
 * SERVICE_STOPPED = 1
 * SERVICE_START_PENDING = 2
 * SERVICE_STOP_PENDING = 3
 * SERVICE_RUNNING = 4
 * SERVICE_CONTINUE_PENDING = 5
 * SERVICE_PAUSE_PENDING = 6
 * SERVICE_PAUSED = 7
 * SERVICETYPE_NORMAL = 1
 * SERVICETYPE_BUILTIN = 2
 * SERVICETYPE_EXPORT_CONSTANTS = 4
 * SERVICE_CONTROL_STOP = 1
 * SERVICE_CONTROL_PAUSE = 2
 * SERVICE_CONTROL_CONTINUE = 3
 * SERVICE_CONTROL_INTERROGATE = 4
 * SERVICE_CONTROL_SHUTDOWN = 5
 * SERVICE_CHECK_NONE = 0
 * SERVICE_CHECK_CALL = 1
 * SERVICE_CHECK_INIT = 2
 * SERVICE_WANT_SESSIONS = 1
 * PRE_NONE = 0
 * PRE_AUTH = 1
 * PRE_HASCHAR = 2
 * PRE_HASSHIP = 4
 * PRE_INSTATION = 8
 * PRE_INFLIGHT = 16
 */
/*
The Complete MachoNet Core Message Enum Map
Index   Message Name                    Purpose & Behaviour
0       AUTHENTICATION_REQ              Initial secure crypto-handshake request packet sent from the client to the Proxy.
1       AUTHENTICATION_RSP              Crypto-handshake acknowledgment containing public key/seed validation.
2       IDENTIFICATION_REQ              Client identifying its account details, version string, and authentication token.
3       IDENTIFICATION_RSP              Proxy returns server status, connection acceptance, and character selection hooks.
4       DISCONNECT_NOTIFICATION         Low-level wire signal forced down the line if an explicit gracefully closed connection fires.
5       HEARTBEAT                       Raw transport-layer micro-ping to ensure socket health before the session initializes.
6       CALL_REQ                        Client-to-Server RPC Call. Standard execution frame for invoking active back-end service methods.
7       CALL_RSP                        Server-to-Client RPC Response. Delivers successfully marshaled Python tuple/dict structures back.
8       TRANSPORTCLOSED                 Hard socket socket teardown interceptor. Triggered immediately if the client forcefully vanishes.
9       FORWARD_REQ                     Internal Proxy-to-Node cross-routing packet used to transition a client across cluster boundaries.
10      RESOLVE_REQ                     High-level distributed cache or object resolution query (finding the physical server tracking an entity).
11      RESOLVE_RSP                     Resolution return providing exact object location address bounds.
12      NOTIFICATION                    One-Way Server Push. Pushes grid updates, warp confirmations, and chat text requiring zero handshake feedback.
13      MULTICAST_NOTIFICATION          Specialized batch push optimizing server efficiency by broadcast routing a single chunk to a list of sessions.
14      REJECT_NOTIFICATION             Specific network-level dropping packet sent when a service request format breaks schema boundaries.
15      ERRORRESPONSE                   Central Error Marshaller. Direct router to pop open error boxes, using your central UserError sounds and tokens.
16      SESSIONCHANGENOTIFICATION       Dispatched when boundaries shift (e.g., passing through a jumpgate, docking, changing characters).
17      SESSIONINITIALSTATEREQ          Dispatched during handshake initialization to request localized environment configurations.
18      SESSIONINITIALSTATENOTIFICATION Transmits full state synchronization bundles immediately upon establishing a new connection.
19      HEARTBEAT_ACK                   Immediate hardware answer to prevent the network interface card from idling or timing out.
20      PING_REQ                        High-level transport loop measuring round-trip latency.
21      PING_RSP                        Immediate latency confirmation payload.
*/
enum MACHONETMSG_TYPE
{
    AUTHENTICATION_REQ              = 0,
    AUTHENTICATION_RSP              = 1,
    IDENTIFICATION_REQ              = 2,
    IDENTIFICATION_RSP              = 3,
    DISCONNECT_NOTIFICATION         = 4,
    HEARTBEAT                       = 5,
    CALL_REQ                        = 6,
    CALL_RSP                        = 7,
    TRANSPORTCLOSED                 = 8,
    FORWARD_REQ                     = 9,
    RESOLVE_REQ                     = 10,
    RESOLVE_RSP                     = 11,
    NOTIFICATION                    = 12,
    MULTICAST_NOTIFICATION          = 13,
    REJECT_NOTIFICATION             = 14,
    ERRORRESPONSE                   = 15,
    SESSIONCHANGENOTIFICATION       = 16,
    SESSIONINITIALSTATEREQ          = 17,
    SESSIONINITIALSTATENOTIFICATION = 18,
    HEARTBEAT_ACK                   = 19,
    PING_REQ                        = 20,
    PING_RSP                        = 21,
    MOVEMENTNOTIFICATION            = 100,
    MACHONETMSG_TYPE_COUNT
};

enum MACHONETERR_TYPE
{
    UNMACHODESTINATION  = 0,
    UNMACHOCHANNEL      = 1,
    WRAPPEDEXCEPTION    = 2
};
//see PyPacket.cpp
extern const char* MACHONETMSG_TYPE_NAMES[MACHONETMSG_TYPE_COUNT];


enum SESSION_TYPE
{
    SESSION_TYPE_INVALID = 0,
    SESSION_TYPE_EXECUTIONCONTEXT = 1,
    SESSION_TYPE_SERVICE = 2,
    SESSION_TYPE_CREST = 3,
    SESSION_TYPE_ESP = 4,
    SESSION_TYPE_GAME = 5
};

//  packet_types *should* end here...at the end of the....wait for it...PACKET TYPES!!

/*
 * mailingListBlocked = 0
 * mailingListAllowed = 1
 * mailingListMemberMuted = 0
 * mailingListMemberDefault = 1
 * mailingListMemberOperator = 2
 * mailingListMemberOwner = 3
 * ALLIANCE_SERVICE_MOD = 200
 * CHARNODE_MOD = 64
 * PLANETARYMGR_MOD = 128
 * mailTypeMail = 1
 * mailTypeNotifications = 2
 * mailStatusMaskRead = 1
 * mailStatusMaskReplied = 2
 * mailStatusMaskForwarded = 4
 * mailStatusMaskTrashed = 8
 * mailStatusMaskDraft = 16
 * mailStatusMaskAutomated = 32
 * mailLabelInbox = 1
 * mailLabelSent = 2
 * mailLabelCorporation = 4
 * mailLabelAlliance = 8
 * mailLabelsSystem = mailLabelInbox + mailLabelSent + mailLabelCorporation + mailLabelAlliance
 * mailMaxRecipients = 50
 * mailMaxGroups = 1
 * mailMaxSubjectSize = 150
 * mailMaxBodySize = 8000
 * mailMaxTaggedBodySize = 10000
 * mailMaxLabelSize = 40
 * mailMaxNumLabels = 25
 * mailMaxPerPage = 100
 * mailTrialAccountTimer = 1
 * mailMaxMessagePerMinute = 5
 * mailinglistMaxMembers = 3000
 * mailinglistMaxMembersUpdated = 1000
 * mailingListMaxNameSize = 60
 */

/** misc costs
 * costCloneContract = 5600
 * costJumpClone = 100000
 */


#endif  //EVE_PACKET_TYPES_H