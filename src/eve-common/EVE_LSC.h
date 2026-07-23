

/*
 *
 * LSC stands for Large Scale Chat
 *
 *
 * CHTMODE_CREATOR = 8 + 4 + 2 + 1
 * CHTMODE_OPERATOR = 4 + 2 + 1
 * CHTMODE_CONVERSATIONALIST = 2 + 1
 * CHTMODE_SPEAKER = 2
 * CHTMODE_LISTENER = 1
 * CHTMODE_NOTSPECIFIED = -1
 * CHTMODE_DISALLOWED = -2
 *
 * CHTERR_NOSUCHCHANNEL = -3
 * CHTERR_ACCESSDENIED = -4
 * CHTERR_INCORRECTPASSWORD = -5
 * CHTERR_ALREADYEXISTS = -6
 * CHTERR_TOOMANYCHANNELS = -7
 *
 * CHT_MAX_USERS_PER_IMMEDIATE_CHANNEL = 50
 * CHT_MAX_INPUT = 253
 *
 *
 *  'rookieHelpChannel': 1,
 *  'helpChannelEN': 2,
 *  'helpChannelDE': 40,
 *  'helpChannelRU': 55,
 *  'helpChannelJA': 56
 *
 */

/*
 * service.ROLE_CHTADMINISTRATOR | service.ROLE_GMH
 * CHTMODE_CREATOR = (((8 + 4) + 2) + 1)
 * CHTMODE_OPERATOR = ((4 + 2) + 1)
 * CHTMODE_CONVERSATIONALIST = (2 + 1)
 * CHTMODE_SPEAKER = 2
 * CHTMODE_LISTENER = 1
 * CHTMODE_NOTSPECIFIED = -1
 * CHTMODE_DISALLOWED = -2
 * CHTERR_NOSUCHCHANNEL = -3
 * CHTERR_ACCESSDENIED = -4
 * CHTERR_INCORRECTPASSWORD = -5
 * CHTERR_ALREADYEXISTS = -6
 * CHTERR_TOOMANYCHANNELS = -7
 * CHT_MAX_USERS_PER_IMMEDIATE_CHANNEL = 50
 *
 * CHANNEL_CUSTOM = 0
 * CHANNEL_GANG = 3
 *
 *
 */

#pragma once


namespace LSC {
    // The master mapping struct to bundle hex strings and native client integers
    struct ChatColor {
        const char* hexStr;
        int32 clientInt;
    };

    namespace Color {
        // --- 1. STANDARD COLORS ---
        const ChatColor Orange     = { "0xffffa500", -0        };
        const ChatColor Purple     = { "0xff800080", -0        };
        const ChatColor Brown      = { "0xffa52a2a", -0        };
        const ChatColor Yellow     = { "0xffffff00", -0        };
        const ChatColor Maroon     = { "0xff800000", -0        };
        const ChatColor Lime       = { "0xff00ff00", -0        };
        const ChatColor Magenta    = { "0xffff00ff", -0        };
        const ChatColor Olive      = { "0xff808000", -0        };
        const ChatColor Pink       = { "0xffffc0cb", -0        };
        const ChatColor Aquamarine = { "0xff7fffd4", -0        };
        const ChatColor Red        = { "0xffff0000", -0        };
        const ChatColor Silver     = { "0xffc0c0c0", -0        };
        const ChatColor Blue       = { "0xff0000ff", -0        };
        const ChatColor Gray       = { "0xff808080", -0        };
        const ChatColor DarkBlue   = { "0xff00008b", -0        };
        const ChatColor LightBlue  = { "0xffadd8e6", -0        };
        const ChatColor Cyan       = { "0xff00ffff", -16711681 }; // Heavy utility / custom broadcast
        const ChatColor Azure      = { "0xff007fff", -16744449 }; // Fleet / deep atmospheric operations
        const ChatColor PureWhite  = { "0xffffffff", -1        }; // System informational overrides
        const ChatColor PureGreen  = { "0xff00ff00", -16711936 }; // Secure comms / clear data links
        const ChatColor LiteGrey   = { "0xffe0e0e0", -2039584  }; // Standard baseline chatter

        // --- 2. THE THEMATIC DRONE / BORG PALETTE ---
        const ChatColor BorgNeon   = { "0xff00ffcc", -16711732 }; // Legioneer Green / Hive synchronization
        const ChatColor WorldMod   = { "0xffac75ff", -5474817  }; // Purposed Purple / Hacking intrusion siege
        const ChatColor ThreatGold = { "0xffffff20", -224      }; // GML Gold / Target vulnerability profile mapping
        const ChatColor CritRed    = { "0xffee6666", -1153434  }; // Critical warnings / Terminal host shatter events

        // --- 3. RECOMMENDED OPERATIONAL SUGGESTIONS ---
        const ChatColor WarnYellow = { "0xffffff00", -256      }; // Environmental hazards / cosmic signatures
        const ChatColor AlertOrange= { "0xffff6600", -39424    }; // Weapon overload cascades / proximity breaches
        const ChatColor Crimson    = { "0xffdc143c", -2354116  }; // Severe threat / Capital target detection alerts
        const ChatColor DeepPurple = { "0xff7f00ff", -8453889  }; // Anomalous data / Abyssal-style space events
        const ChatColor StealthGrey= { "0xff5a5a5a", -10855846 }; // Low-priority background diagnostic chatter
        const ChatColor MatrixDark = { "0xff003300", -16764160 }; // Background "noise" / hidden encryption streams
    }
    /*  drone bcast suggetions
        CorruptedGreen  = 1,   // Matrix/Firmware aesthetics
        WarningYellow   = 2,   // Swarm command bleed-over
        LostPanic       = 3,   // Small ships panic events
        AlertOrange     = 5,   // Tactical shifting/Vulnerability mapping
        CriticalRed     = 11,  // Shield breaches/Shatter events
        SystemWhite     = 18,  // Clear, uncorrupted binary overrides
        InfectionPurple = 773399ff   // Elite Carrier intrusion siege tracking
    */

    struct ChannelData {
        int32 channelID = 0;
        std::string displayName;
        std::string motd;
        uint32 ownerID = 0;
        std::string comparisonKey;
        bool memberless = false;
        std::string password;
        bool mailingList = false;
        uint32 cspa = 0;

        // Explicit constructor required to satisfy GCC 4.9.2's emplace constraints
        ChannelData(int32 id, std::string name, std::string m, uint32 owner,
                   std::string key, bool memb, std::string pass, bool mail, uint32 cs)
            : channelID(id), displayName(name), motd(m), ownerID(owner),
            comparisonKey(key), memberless(memb), password(pass), mailingList(mail), cspa(cs) {}
    };

    struct CharMetaData {
        std::string characterName;
        std::string corporationName;

        // GCC 4.9.2 constructor constraint matching
        CharMetaData(std::string charName, std::string corpName)
        : characterName(charName), corporationName(corpName) {}
    };


    namespace Type {
        // type designations are internal-use only (client works on strings)
        enum {
            global          = 1,    // send channelID as tuple(id, desc)  uses full memberlist, never memberless
            corp            = 2,    // send channelID as tuple(id, desc)  uses full memberlist, never memberless
            region          = 3,    // send channelID as tuple(id, desc)  uses full memberlist, never memberless, not used in w-space
            constellation   = 4,    // send channelID as tuple(id, desc)  uses full memberlist, never memberless, not used in w-space
            solarsystem     = 5,    // send channelID as tuple(id, desc)  used in w-space, memberless, changes chat window title from "Local" to "System"
            solarsystem2    = 6,    // send channelID as tuple(id, desc)  uses full memberlist, never memberless, not used in w-space (k-space "Local" channel)
            // end of static channels
            character       = 7,    // for mailing lists using channelID = charID
            // begin dynamic channels
            alliance        = 8,
            fleet           = 9,
            wing            = 10,
            squad           = 11,
            warfaction      = 12,
            incursion       = 13,
            normal          = 14,   //  trial accts arent time buffered (channelID > 2100000000)
            custom          = 15    //  invite only.  channelID < 0
        };
    }

    namespace Mode {
        enum {
            Disallowed 	        = -2,
            Unspecified 	= -1,
            None 		= 0,	// banned/muted
            Listener	        = 1,	// read-only
            Speaker 	        = 2,	// std member
            Moderator	        = 3,	// op/mod
            Operator 	        = 7,
            Creator 	        = 15	// owner
        };
    }

    namespace Error {
        enum {
            Unspecified 	= -1,
            Disallowed 	        = -2,
            NoSuchChannel 	= -3,
            AccessDenied 	= -4,
            WrongPass 	        = -5,
            ChannelExists 	= -6,
            TooManyChannels     = -7
        };
    }
};   // namespace LSC

/* groupMessageIDs and descriptions
 * 1 = Passive
 * 2 = Aggressive
 * 3 = Focus Fire
 *
 * 263235 = Corporate
 * 263238 = Help
 * 263329 = Factions
 * 263240 = Trade
 * 263331 = Science and Industry
 * 263328 = Content
 *
 */
/*
groupTypes = {groupAgents: [notifyIDs.notificationTypeAgentMoveMsg,
               notifyIDs.notificationTypeLocateCharMsg,
               notifyIDs.notificationTypeResearchMissionAvailableMsg,
               notifyIDs.notificationTypeMissionOfferExpirationMsg,
               notifyIDs.notificationTypeMissionTimeoutMsg,
               notifyIDs.notificationTypeStoryLineMissionAvailableMsg,
               notifyIDs.notificationTypeTutorialMsg],
 groupBills: [notifyIDs.notificationTypeAllMaintenanceBillMsg,
              notifyIDs.notificationTypeCharBillMsg,
              notifyIDs.notificationTypeCorpAllBillMsg,
              notifyIDs.notificationTypeBillOutOfMoneyMsg,
              notifyIDs.notificationTypeBillPaidCharMsg,
              notifyIDs.notificationTypeBillPaidCorpAllMsg,
              notifyIDs.notificationTypeCorpOfficeExpirationMsg],
 groupContacts: [notifyIDs.notificationTypeContactAdd, notifyIDs.notificationTypeContactEdit],
 groupCorp: [notifyIDs.notificationTypeCharTerminationMsg,
             notifyIDs.notificationTypeCharMedalMsg,
             notifyIDs.notificationTypeCorpAppNewMsg,
             notifyIDs.notificationTypeCorpAppRejectMsg,
             notifyIDs.notificationTypeCorpAppAcceptMsg,
             notifyIDs.notificationTypeCorpTaxChangeMsg,
             notifyIDs.notificationTypeCorpNewsMsg,
             notifyIDs.notificationTypeCharLeftCorpMsg,
             notifyIDs.notificationTypeCorpNewCEOMsg,
             notifyIDs.notificationTypeCorpDividendMsg,
             notifyIDs.notificationTypeCorpVoteMsg,
             notifyIDs.notificationTypeCorpVoteCEORevokedMsg,
             notifyIDs.notificationTypeCorpLiquidationMsg,
             notifyIDs.notificationTypeCorpKicked],
 groupMisc: [notifyIDs.notificationTypeBountyClaimMsg,
             notifyIDs.notificationTypeCloneActivationMsg,
             notifyIDs.notificationTypeContainerPasswordMsg,
             notifyIDs.notificationTypeCustomsMsg,
             notifyIDs.notificationTypeInsuranceFirstShipMsg,
             notifyIDs.notificationTypeInsurancePayoutMsg,
             notifyIDs.notificationTypeInsuranceInvalidatedMsg,
             notifyIDs.notificationTypeCloneRevokedMsg1,
             notifyIDs.notificationTypeCloneMovedMsg,
             notifyIDs.notificationTypeCloneRevokedMsg2,
             notifyIDs.notificationTypeInsuranceExpirationMsg,
             notifyIDs.notificationTypeInsuranceIssuedMsg,
             notifyIDs.notificationTypeJumpCloneDeletedMsg1,
             notifyIDs.notificationTypeJumpCloneDeletedMsg2,
             notifyIDs.notificationTypeTransactionReversalMsg,
             notifyIDs.notificationTypeReimbursementMsg,
             notifyIDs.notificationTypeIncursionCompletedMsg],
 groupOld: [notifyIDs.notificationTypeOldLscMessages],
 groupSov: [notifyIDs.notificationTypeSovAllClaimFailMsg,
            notifyIDs.notificationTypeSovCorpClaimFailMsg,
            notifyIDs.notificationTypeSovAllBillLateMsg,
            notifyIDs.notificationTypeSovCorpBillLateMsg,
            notifyIDs.notificationTypeSovAllClaimLostMsg,
            notifyIDs.notificationTypeSovCorpClaimLostMsg,
            notifyIDs.notificationTypeSovAllClaimAquiredMsg,
            notifyIDs.notificationTypeSovCorpClaimAquiredMsg,
            notifyIDs.notificationTypeSovDisruptorMsg,
            notifyIDs.notificationTypeAllStructVulnerableMsg,
            notifyIDs.notificationTypeAllStrucInvulnerableMsg,
            notifyIDs.notificationTypeSovereigntyTCUDamageMsg,
            notifyIDs.notificationTypeSovereigntySBUDamageMsg,
            notifyIDs.notificationTypeSovereigntyIHDamageMsg],
 groupStructures: [notifyIDs.notificationTypeAllAnchoringMsg,
                   notifyIDs.notificationTypeCorpStructLostMsg,
                   notifyIDs.notificationTypeTowerAlertMsg,
                   notifyIDs.notificationTypeTowerResourceAlertMsg,
                   notifyIDs.notificationTypeStationAggressionMsg1,
                   notifyIDs.notificationTypeStationStateChangeMsg,
                   notifyIDs.notificationTypeStationConquerMsg,
                   notifyIDs.notificationTypeStationAggressionMsg2,
                   notifyIDs.notificationTypeOrbitalAttacked,
                   notifyIDs.notificationTypeOrbitalReinforced,
                   notifyIDs.notificationTypeOwnershipTransferred],
 groupWar: [notifyIDs.notificationTypeAllWarDeclaredMsg,
            notifyIDs.notificationTypeAllWarSurrenderMsg,
            notifyIDs.notificationTypeAllWarRetractedMsg,
            notifyIDs.notificationTypeAllWarInvalidatedMsg,
            notifyIDs.notificationTypeCorpWarDeclaredMsg,
            notifyIDs.notificationTypeCorpWarFightingLegalMsg,
            notifyIDs.notificationTypeCorpWarSurrenderMsg,
            notifyIDs.notificationTypeCorpWarRetractedMsg,
            notifyIDs.notificationTypeCorpWarInvalidatedMsg,
            notifyIDs.notificationTypeFWCorpJoinMsg,
            notifyIDs.notificationTypeFWCorpLeaveMsg,
            notifyIDs.notificationTypeFWCorpKickMsg,
            notifyIDs.notificationTypeFWCharKickMsg,
            notifyIDs.notificationTypeFWCorpWarningMsg,
            notifyIDs.notificationTypeFWCharWarningMsg,
            notifyIDs.notificationTypeFWCharRankLossMsg,
            notifyIDs.notificationTypeFWCharRankGainMsg,
            notifyIDs.notificationTypeFacWarCorpJoinRequestMsg,
            notifyIDs.notificationTypeFacWarCorpLeaveRequestMsg,
            notifyIDs.notificationTypeFacWarCorpJoinWithdrawMsg,
            notifyIDs.notificationTypeFacWarCorpLeaveWithdrawMsg,
            notifyIDs.notificationTypeFWAllianceWarningMsg,
            notifyIDs.notificationTypeFWAllianceKickMsg]}
*/