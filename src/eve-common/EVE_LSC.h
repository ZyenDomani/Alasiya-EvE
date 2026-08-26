/*
 *
 * LSC stands for Large Scale Chat
 *
 */

#pragma once
#include "../eve-core/eve-compat.h"


namespace LSC {
    // The master mapping struct to bundle hex strings and native client integers
    struct ChatColor {
        const char* hexStr;
        int32 clientInt;
    };

    /* The Golden Rule for Implementation
     * Use .hexStr whenever you are building raw message strings or MOTD panels that utilize HTML tags (<color=...>)
     * Use .clientInt whenever you are populating MachoNet packet structures, rowset fields, or tint properties that expect a raw signed 32-bit number
     */
    namespace Color {
        // --- 1. STANDARD COLORS ---
        const ChatColor Orange       = { "0xffffa500", -23296    };
        const ChatColor Purple       = { "0xff800080", -8388480  };
        const ChatColor Brown        = { "0xffa52a2a", -5952982  };
        const ChatColor Yellow       = { "0xffffff00", -256      }; // Shared with WarnYellow
        const ChatColor Maroon       = { "0xff800000", -8388608  };
        const ChatColor Lime         = { "0xff00ff00", -16711936 }; // Shared with PureGreen
        const ChatColor Magenta      = { "0xffff00ff", -65281    };
        const ChatColor Olive        = { "0xff808000", -8360192  };
        const ChatColor Pink         = { "0xffffc0cb", -16181    };
        const ChatColor Aquamarine   = { "0xff7fffd4", -8388652  };
        const ChatColor Red          = { "0xffff0000", -65536    };
        const ChatColor Silver       = { "0xffc0c0c0", -4144960  };
        const ChatColor Blue         = { "0xff0000ff", -16776961 };
        const ChatColor Gray         = { "0xff808080", -8421505  };
        const ChatColor DarkBlue     = { "0xff00008b", -16777077 };
        const ChatColor LightBlue    = { "0xffadd8e6", -5384218  };
        const ChatColor Cyan         = { "0xff00ffff", -16711681 }; // Heavy utility / custom broadcast
        const ChatColor Azure        = { "0xff007fff", -16744449 }; // Fleet / deep atmospheric operations
        const ChatColor PureWhite    = { "0xffffffff", -1        }; // System informational overrides
        const ChatColor PureGreen    = { "0xff00ff00", -16711936 }; // Secure comms / clear data links
        const ChatColor LiteGrey     = { "0xffe0e0e0", -2039584  }; // Standard baseline chatter

        // --- 2. THE THEMATIC DRONE / BORG PALETTE ---
        const ChatColor BorgNeon     = { "0xff00ffcc", -16711732 }; // Legioneer Green / Hive synchronization
        const ChatColor WorldMod     = { "0xffac75ff", -5474817  }; // Purposed Purple / Hacking intrusion siege
        const ChatColor ThreatGold   = { "0xffffff20", -224      }; // GML Gold / Target vulnerability profile mapping
        const ChatColor CritRed          = { "0xffee6666", -1153434  }; // Critical warnings / Terminal host shatter events

        // --- 3. RECOMMENDED OPERATIONAL SUGGESTIONS ---
        const ChatColor WarnYellow   = { "0xffffff00", -256      }; // Environmental hazards / cosmic signatures
        const ChatColor AlertOrange  = { "0xffff6600", -39424    }; // Weapon overload cascades / proximity breaches
        const ChatColor Crimson      = { "0xffdc143c", -2354116  }; // Severe threat / Capital target detection alerts
        const ChatColor DeepPurple   = { "0xff7f00ff", -8453889  }; // Anomalous data / Abyssal-style space events
        const ChatColor InfectPurple = { "0x773399ff", 2000001535}; // Elite Carrier intrusion siege tracking
        const ChatColor StealthGrey  = { "0xff5a5a5a", -10855846 }; // Low-priority background diagnostic chatter
        const ChatColor MatrixDark   = { "0xff003300", -16764160 }; // Background "noise" / hidden encryption streams
    }

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


        // type designations are internal-use only (client works on strings)
        enum Type :int8_t {
            global          = 1,    // send channelID as tuple(desc, id)  not memberless
            corp            = 2,    // send channelID as tuple(desc, id)  not memberless
            region          = 3,    // send channelID as tuple(desc, id)  not memberless, not used in w-space
            constellation   = 4,    // send channelID as tuple(desc, id)  not memberless, not used in w-space
            solarsystem     = 5,    // send channelID as tuple(desc, id)  is memberless, w-space "System" channel
            solarsystem2    = 6,    // send channelID as tuple(desc, id)  not memberless, k-space "Local" channel
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


        enum Mode :int8_t {
            Disallowed 	        = -2,
            Unspecified 	= -1,
            None 		= 0,	// banned/muted
            Listener	        = 1,	// read-only
            Speaker 	        = 2,	// std member
            Moderator	        = 3,	// op/mod
            Operator 	        = 7,
            Creator 	        = 15	// owner
        };


    namespace Error {
        enum :int8_t {
            Unspecified 	= -1,
            Disallowed 	        = -2,
            NoSuchChannel 	= -3,
            AccessDenied 	= -4,
            WrongPass 	        = -5,
            ChannelExists 	= -6,
            TooManyChannels     = -7
        };
    }

    namespace gID {
        enum :int32_t {
	    // these are the group titles in channel list
	    None		= 0,
            Player              = 61587,  //Player Channels
            Mine                = 61560,   //My Channels
            Faction             = 63594,
            System2             = 67203, //System Channels <<- space
            Unspecified         = 67237,
            Mission             = 67238,
            Market              = 67242,
            Dungeon             = 67243,
            Misc                = 67265,
            Corporate           = 263235,
            Help                = 263238,
            Trade               = 263240,
            MaM                 = 263275, //Minerals and Manufacturing
            Events              = 263306,
            Content             = 263328,
            Media               = 263330,
	    SnI			= 263331  //Science and Industry
        };
    }
    namespace cID {
        enum :int32_t {
	    // these are the channel titles in channel list
            System      	= -1,    //SystemChannels  <<- no space
	    None		= 0,
            Faction             = 63594,
            System2             = 67203, //System Channels
            Character           = 67230,
            Corporation         = 67231,
            Unspecified         = 67237,
            Research            = 67240,
            Alliance            = 67241,
            Industry            = 67248,
            Rookie              = 263259, //Rookie Help
            EngHelp             = 263262,
            Rumor               = 263265,
            Other               = 263277,
            Smacktalk           = 263278,
            CEO                 = 263287,
            Blueprints          = 263292,
            RealEstate          = 263293,
            Technology          = 263332,
            Ratting             = 263338,
            Scanning            = 263339,
            Wormholes           = 263340,
            Boosters            = 263365,
            Invention           = 263366,
            Manufacturing       = 263367,
            Mining              = 263368,
            PI                  = 263369, //Planetary Interaction
            Owner               = 263628
        };
    }

};   // namespace LSC
