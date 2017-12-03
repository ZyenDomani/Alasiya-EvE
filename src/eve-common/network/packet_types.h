/*
 *    ------------------------------------------------------------------------------------
 *    LICENSE:
 *    ------------------------------------------------------------------------------------
 *    This file is part of EVEmu: EVE Online Server Emulator
 *    Copyright 2006 - 2011 The EVEmu Team
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
#include "../EVE_Consts.h"
#include "../EVE_Defines.h"
#include "../EVE_Dungeon.h"
#include "../EVE_Effects.h"
#include "../EVE_Flags.h"
#include "../EVE_Inventory.h"
#include "../EVE_Planet.h"
#include "../EVE_Roles.h"
#include "../EVE_Skills.h"
#include "../EVE_Typedefs.h"


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

enum MACHONETMSG_TYPE
{
    AUTHENTICATION_REQ              = 0,
    AUTHENTICATION_RSP              = 1,
    IDENTIFICATION_REQ              = 2,
    IDENTIFICATION_RSP              = 3,
    __Fake_Invalid_Type             = 4,
    CALL_REQ                        = 6,
    CALL_RSP                        = 7,
    TRANSPORTCLOSED                 = 8,
    RESOLVE_REQ                     = 10,
    RESOLVE_RSP                     = 11,
    NOTIFICATION                    = 12,
    ERRORRESPONSE                   = 15,
    SESSIONCHANGENOTIFICATION       = 16,
    SESSIONINITIALSTATENOTIFICATION = 18,
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

/*
 * typedef enum {
 * 3018681,
 * 3018821,
 * 3018822,
 * 3018823,
 * 3018824,
 * 3018680,
 * 3018817,
 * 3018818,
 * 3018819,
 * 3018820,
 * 3018682,
 * 3018809,
 * 3018810,
 * 3018811,
 * 3018812,
 * 3018678,
 * 3018837,
 * 3018838,
 * 3018839,
 * 3018840,
 * 3018679,
 * 3018841,
 * 3018842,
 * 3018843,
 * 3018844,
 * 3018677,
 * 3018845,
 * 3018846,
 * 3018847,
 * 3018848,
 * 3018676,
 * 3018825,
 * 3018826,
 * 3018827,
 * 3018828,
 * 3018675,
 * 3018805,
 * 3018806,
 * 3018807,
 * 3018808,
 * 3018672,
 * 3018801,
 * 3018802,
 * 3018803,
 * 3018804,
 * 3018684,
 * 3018829,
 * 3018830,
 * 3018831,
 * 3018832,
 * 3018685,
 * 3018813,
 * 3018814,
 * 3018815,
 * 3018816,
 * 3018683,
 * 3018833,
 * 3018834,
 * 3018835,
 * 3018836]
 * }rookieAgentList;
 */
/*
auraAgentIDs = [
 3019499,
 3019493,
 3019495,
 3019490,
 3019497,
 3019496,
 3019486,
 3019498,
 3019492,
 3019500,
 3019489,
 3019494]
 */

/*
agentRangeSameSystem = 1
agentRangeSameOrNeighboringSystemSameConstellation = 2
agentRangeSameOrNeighboringSystem = 3
agentRangeNeighboringSystemSameConstellation = 4
agentRangeNeighboringSystem = 5
agentRangeSameConstellation = 6
agentRangeSameOrNeighboringConstellationSameRegion = 7
agentRangeSameOrNeighboringConstellation = 8
agentRangeNeighboringConstellationSameRegion = 9
agentRangeNeighboringConstellation = 10
agentRangeNearestEnemyCombatZone = 11
agentRangeNearestCareerHub = 12
agentIskMultiplierLevel1 = 1
agentIskMultiplierLevel2 = 2
agentIskMultiplierLevel3 = 4
agentIskMultiplierLevel4 = 8
agentIskMultiplierLevel5 = 16
agentIskMultipliers = (agentIskMultiplierLevel1,
 agentIskMultiplierLevel2,
 agentIskMultiplierLevel3,
 agentIskMultiplierLevel4,
 agentIskMultiplierLevel5)
agentLpMultiplierLevel1 = 20
agentLpMultiplierLevel2 = 60
agentLpMultiplierLevel3 = 180
agentLpMultiplierLevel4 = 540
agentLpMultiplierLevel5 = 4860
agentLpMultipliers = (agentLpMultiplierLevel1,
 agentLpMultiplierLevel2,
 agentLpMultiplierLevel3,
 agentLpMultiplierLevel4,
 agentLpMultiplierLevel5)
agentIskRandomLowValue = 11000
agentIskRandomHighValue = 16500
agentCareerTypeIndustry = 1
agentCareerTypeBusiness = 2
agentCareerTypeMilitary = 3
agentCareerTypeExploration = 4
agentCareerTypeAdvMilitary = 5
agentDialogueButtonViewMission = 1
agentDialogueButtonRequestMission = 2
agentDialogueButtonAccept = 3
agentDialogueButtonAcceptChoice = 4
agentDialogueButtonAcceptRemotely = 5
agentDialogueButtonComplete = 6
agentDialogueButtonCompleteRemotely = 7
agentDialogueButtonContinue = 8
agentDialogueButtonDecline = 9
agentDialogueButtonDefer = 10
agentDialogueButtonQuit = 11
agentDialogueButtonStartResearch = 12
agentDialogueButtonCancelResearch = 13
agentDialogueButtonBuyDatacores = 14
agentDialogueButtonLocateCharacter = 15
agentDialogueButtonLocateAccept = 16
agentDialogueButtonLocateReject = 17
agentDialogueButtonYes = 18
agentDialogueButtonNo = 19
*/

/** misc costs
 * costCloneContract = 5600
 * costJumpClone = 100000
 */


#endif  //EVE_PACKET_TYPES_H