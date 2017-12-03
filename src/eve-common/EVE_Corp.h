
/*
 *
 *
 */


#ifndef EVE_CORP_H
#define EVE_CORP_H

namespace EveCorp {

/*
if logItem.eventTypeID == 12:     /Auditing/CreatedCorporation', corpName=corpName)
     elif logItem.eventTypeID == 13:     /Auditing/DeletedCorporation', corpName=corpName)
     elif logItem.eventTypeID == 14:     /Auditing/LeftCorporation', corpName=corpName)
     elif logItem.eventTypeID == 15:     /Auditing/AppliedForMembershipOfCorporation', corpName=corpName)
     elif logItem.eventTypeID == 16:     /Auditing/BecameCEOOfCorporation', corpName=corpName)
     elif logItem.eventTypeID == 17:     /Auditing/LeftCEOPositionOfCorporation', corpName=corpName)
     elif logItem.eventTypeID == 44:     /Auditing/JoinedCorporation', corpName=corpName)
*/




/** AppInfo:
 *  status / corp side / user side
 *    0        new         applied
 *    1        update      reneg
 *    2        accepted    accepted
 *    4        error       reject
 *    6        offer       offer
 */

enum AppStatus {  //from eveConstants
    appliedByCharacter          = 0,
    renegotiatedByCharacter     = 1,
    acceptedByCharacter         = 2,
    rejectedByCharacter         = 3,
    rejectedByCorporation       = 4,
    renegotiatedByCorporation   = 5,
    acceptedByCorporation       = 6
};

enum RoleLoc {
    HQ      = 1,
    Base    = 2,
    Other   = 3
};


/*
 * corpactivityEducation = 18
 * corpactivityEntertainment = 8
 * corpactivityMilitary = 5
 * corpactivitySecurity = 16
 * corpactivityTrading = 12
 * corpactivityWarehouse = 10
 * corpDivisionDistribution = 22
 * corpDivisionMining = 23
 * corpDivisionSecurity = 24
 */

/*
 * allianceApplicationAccepted = 2
 * allianceApplicationEffective = 3
 * allianceApplicationNew = 1
 * allianceApplicationRejected = 4
 * allianceCreationCost = 1000000000
 * allianceMembershipCost = 2000000
 * allianceRelationshipCompetitor = 3
 * allianceRelationshipEnemy = 4
 * allianceRelationshipFriend = 2
 * allianceRelationshipNAP = 1
 */

/*
 * facwarCorporationJoining = 0
 * facwarCorporationActive = 1
 * facwarCorporationLeaving = 2
 * facwarStandingPerVictoryPoint = 0.0015
 * facwarWarningStandingCharacter = 0
 * facwarWarningStandingCorporation = 1
 * facwarOccupierVictoryPointBonus = 0.1
 * facwarMinStandingsToJoin = 0.5
 * facwarStatTypeKill = 0
 * facwarStatTypeLoss = 1
 */

}

#endif  // EVE_CORP_H