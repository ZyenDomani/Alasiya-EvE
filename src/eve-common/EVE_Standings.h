
/*
 *  EVE_Standings.h
 *   standins-specific data
 *
 */

#ifndef EVE_STANDINGS_H
#define EVE_STANDINGS_H


namespace Standings {
//eve standing change messages in db.repStandingChanges.eventTypeID
//  these come from /eve/common/script/mgt/appLogConst.py

    enum {
        PodKill                         = 9,
        PodKilled                       = 10,
        StandingReset                   = 25,   //Reset by a GM.
        ShipKill                        = 26,
        ShipKilled                      = 27,
        UpdateStanding                  = 45,
        Decay                           = 49,   //All standing decays except when user isn't logged in
        PlayerSet                       = 65,   //Set by player him/herself. Reason: _msg
        CorpSet                         = 68,   //Corp stand set by _int1. Reason: _msg
        MissionCompleted                = 73,   //_msg: name of mission
        MissionFailure                  = 74,   //_msg: name of mission
        MissionDeclined                 = 75,   //_msg: name of mission
        CombatAggression                = 76,   //Combat - Aggression
        CombatShipKill                  = 77,   //Combat - Ship Kill
        CombatPodKill                   = 78,   //Combat - Pod Kill
        CombatOther                     = 79,
        MissionBonus                    = 80,
        DerivedModificationPleased      = 82,   //fromID was pleased
        DerivedModificationDispleased   = 83,   //fromID was displeased
        GMInterventionDirect            = 84,   //Mod directly by _int1. Reason: _msg
        LawEnforcement                  = 89,   //Granted by Concord for actions against _int1
        MissionOfferExpired             = 90,   //Mission Offer Expired - _msg
        StandingCorrection              = 96,
        MissionFailedRollback           = 97,
        StandingRollback                = 98,
        CombatAssistance                = 112,  //Combat - Assistance
        PropertyDamage                  = 154,  //Property Damage
        CombatShipKillOwnFaction        = 223,
        CombatPodKillOwnFaction         = 224,
        CombatAggressionOwnFaction      = 225,
        CombatAssistanceOwnFaction      = 226,
        CombatOtherOwnFaction           = 228
        //anything up until 500 is 'Standing Change'
    };

}


#endif  // EVE_STANDINGS_H
