
/*
 *  EVE_Agent.h
 *   agent-specific enumerators
 *
 */

#ifndef EVE_AGENT_H
#define EVE_AGENT_H

namespace Agent {
    namespace Type {
        enum {
            //  -allan 20Dec14
            None                = 1,
            Basic               = 2,
            Tutorial            = 3,
            Research            = 4,
            GenericStoryLine    = 6,
            StoryLine           = 7,
            Event               = 8,
            FacWar              = 9,
            EpicArc             = 10,
            Aura                = 11
        };
    }

    namespace Range {
        enum {
            SameSystem = 1,
            SameOrNeighboringSystemSameConstellation = 2,
            SameOrNeighboringSystem = 3,
            NeighboringSystemSameConstellation = 4,
            NeighboringSystem = 5,
            SameConstellation = 6,
            SameOrNeighboringConstellationSameRegion = 7,
            SameOrNeighboringConstellation = 8,
            NeighboringConstellationSameRegion = 9,
            NeighboringConstellation = 10,
            NearestEnemyCombatZone = 11,
            NearestCareerHub = 12
        };
    }

    namespace IskMult {
        enum {
            Level1 = 1,
            Level2 = 2,
            Level3 = 4,
            Level4 = 8,
            Level5 = 16,
            RandomLow = 11000,
            RandomHigh = 16500
        };
    }


    namespace LpMult {
        enum {
            Level1 = 20,
            Level2 = 60,
            Level3 = 180,
            Level4 = 540,
            Level5 = 4860
        };
    }


    namespace Career {
        enum {
            Industry    = 1,
            Business    = 2,
            Military    = 3,
            Exploration = 4,
            AdvMilitary = 5
        };
    }

}

namespace Dialog {
    namespace Button {
        enum {
            ViewMission         = 1,
            RequestMission      = 2,
            Accept              = 3,
            AcceptChoice        = 4,
            AcceptRemotely      = 5,
            Complete            = 6,
            CompleteRemotely    = 7,
            Continue            = 8,
            Decline             = 9,
            Defer               = 10,
            Quit                = 11,
            StartResearch       = 12,
            CancelResearch      = 13,
            BuyDatacores        = 14,
            LocateCharacter     = 15,
            LocateAccept        = 16,
            LocateReject        = 17,
            Yes                 = 18,
            No                  = 19
        };
    }

}



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
 * auraAgentIDs = [
 * 3019499,
 * 3019493,
 * 3019495,
 * 3019490,
 * 3019497,
 * 3019496,
 * 3019486,
 * 3019498,
 * 3019492,
 * 3019500,
 * 3019489,
 * 3019494]
 */


#endif  // EVE_AGENT_H

