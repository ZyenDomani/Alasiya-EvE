
 /**
  * @name EVE_Corp.h
  *     Corp enums and Data containers for Alasiya EvEmu
  *
  * @Author:        Allan
  * @date:          2 December 2017 (corp rewrite began)
  *
  */


#ifndef EVE_CORP_H
#define EVE_CORP_H

namespace Corp {

    namespace CorpBillType {
        enum  {
            MarketFine = 1,
            RentalBill = 2,
            BrokerBill = 3,
            WarBill = 4,
            AllianceMaintainanceBill = 5,
            SovereigntyMarker = 6
        };
    }

    namespace CorpBillStatus {
        enum  {
            Unpaid = 0,
            Paid = 1,
            Cancelled = 2
        };
    }

    namespace VoteType {
        enum {
            CEO = 0,
            War = 1,
            Shares = 2,
            KickMember = 3,
            General = 4,
            ItemLockdown = 5,
            ItemUnlock = 6
        };
    }

    namespace EventType {
        enum {
            CreatedCorporation = 12,
            DeletedCorporation = 13,
            LeftCorporation = 14,
            AppliedForMembershipOfCorporation = 15,
            BecameCEOOfCorporation = 16,
            LeftCEOPositionOfCorporation = 17,
            JoinedCorporation = 44
        };
    }

    namespace AppStatus {
        enum  { //          status              corp side   user side
            AppliedByCharacter          = 0, //    new       applied
            RenegotiatedByCharacter     = 1, //   update      reneg
            AcceptedByCharacter         = 2, //  accepted    accepted
            RejectedByCharacter         = 3,
            RejectedByCorporation       = 4, //    error      reject
            RenegotiatedByCorporation   = 5,
            AcceptedByCorporation       = 6  //    offer      offer
        };
    }

    namespace RoleLoc {
        enum  {
            HQ      = 1,
            Base    = 2,
            Other   = 3
        };
    }

    namespace JoinOp {
        enum SearchJoinOp {
            OR = 1,
            AND = 2
        };
    }

    namespace QueryType {
        enum {
            Roles = 0,
            BaseID = 1,
            CharID = 2,
            TitleMask = 3,
            StartDateTime = 4,
            GrantableRoles = 5
        };
    }

    namespace SearchOp {
        enum  {
            EQUAL = 1,
            GREATER = 2,
            GREATER_OR_EQUAL = 3,
            LESS = 4,
            LESS_OR_EQUAL = 5,
            NOT_EQUAL = 6,
            HAS_BIT = 7,
            NOT_HAS_BIT = 8,
            STR_CONTAINS = 9,
            STR_LIKE = 10,
            STR_STARTS_WITH = 11,
            STR_ENDS_WITH = 12,
            STR_IS = 13
        };
    }

    namespace ActivityType {
        enum  {
            Agriculture     = 1,
            Construction    = 2,
            Mining          = 3,
            Chemical        = 4,
            Military        = 5,
            Biotech         = 6,
            HiTech          = 7,
            Entertainment   = 8,
            Shipyard        = 9,
            Warehouse       = 10,
            Retail          = 11,
            Trading         = 12,
            Bureaucratic    = 13,
            Political       = 14,
            Legal           = 15,
            Security        = 16,
            Financial       = 17,
            Education       = 18,
            Manufacture     = 19,
            Disputed        = 20
        };
    }

    namespace Division {
        enum  {
            Accounting          = 1,
            Administration      = 2,
            Advisory            = 3,
            Archives            = 4,
            Astrosurveying      = 5,
            Command             = 6,
            Distribution        = 7,
            Financial           = 8,
            Intelligence        = 9,
            InternalSecurity    = 10,
            Legal               = 11,
            Manufacturing       = 12,
            Marketing           = 13,
            Mining              = 14,
            Personnel           = 15,
            Production          = 16,
            PublicRelations     = 17,
            RnD                 = 18,
            Security            = 19,
            Storage             = 20,
            Surveillance        = 21,
            DistributionNew     = 22,
            MiningNew           = 23,
            SecurityNew         = 24
        };
    }
}


namespace EveAlliance {
/*
 * allianceCreationCost   = 1000000000
 * allianceMembershipCost =    2000000
 */

    namespace AppStatus {
        enum  {
            AppNew = 1,
            AppAccepted = 2,
            AppEffective = 3,
            AppRejected = 4
        };
    }

    namespace Relation {
        enum  {
            NAP = 1,
            Friend = 2,
            Competitor = 3,
            Enemy = 4
        };
    }
}

namespace EveFacWar {
/*
 * facwarStandingPerVictoryPoint = 0.0015
 *
 * facwarOccupierVictoryPointBonus = 0.1
 *
 * facwarMinStandingsToJoin = 0.5
 *
 * facwarWarningStandingCharacter = 0
 * facwarWarningStandingCorporation = 1
 *
 *
 */

    namespace Relationship {
        enum {
            Unknown = 0,
            YourCorp = 1,
            YourAlliance = 2,
            AtWar = 3,
            AtWarCanFight = 4
        };
    }

    namespace CorpStatus {
        enum  {
            Joining = 0,
            Active = 1,
            Leaving = 2
        };
    }

    namespace StatType {
        enum  {
            Kill = 0,
            Loss = 1
        };
    }
}


#endif  // EVE_CORP_H

