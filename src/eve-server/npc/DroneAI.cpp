/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2011 The EVEmu Team
    For the latest information visit http://evemu.org
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
    Author:     Allan
    Version:    0.15
    Date:       27Nov19
*/

#include "eve-server.h"

#include "Client.h"
#include "inventory/AttributeEnum.h"
#include "system/DestinyManager.h"
#include "npc/Drone.h"
#include "npc/DroneAI.h"
#include "system/Damage.h"
#include "system/SystemBubble.h"

DroneAIMgr::DroneAIMgr(Drone* who)
: m_state(DroneAI::State::Idle),
  m_pDrone(who),
  m_assignedShip(nullptr),
  m_mainAttackTimer(0),// dont start timer until we have a target
  m_processTimer(0),
  m_beginFindTarget(0),
  m_warpScramblerTimer(0),     //not implemented yet
  m_webifierTimer(0),             //not implemented yet
  m_sigRadius(who->GetSelf()->GetAttribute(AttrSignatureRadius).get_float()),
  m_attackSpeed(who->GetSelf()->GetAttribute(AttrSpeed).get_float()),
  m_cruiseSpeed(who->GetSelf()->GetAttribute(AttrEntityCruiseSpeed).get_int()),
  m_chaseSpeed(who->GetSelf()->GetAttribute(AttrMaxVelocity).get_int()),
  m_entityFlyRange(who->GetSelf()->GetAttribute(AttrEntityFlyRange).get_float() + who->GetSelf()->GetAttribute(AttrMaxRange).get_float()),
  m_entityChaseRange(who->GetSelf()->GetAttribute(AttrEntityChaseMaxDistance).get_float() *2),
  m_entityOrbitRange(who->GetSelf()->GetAttribute(AttrMaxRange).get_float()),
  m_entityAttackRange(who->GetSelf()->GetAttribute(AttrEntityAttackRange).get_float() *2),
  m_shieldBoosterDuration(who->GetSelf()->GetAttribute(AttrEntityShieldBoostDuration).get_int()),
  m_armorRepairDuration(who->GetSelf()->GetAttribute(AttrEntityArmorRepairDuration).get_int())
{
    m_processTimer.Start(5000);     //arbitrary.

    // proximityRange (154) tells us how far we "see"

    if (m_entityAttackRange < 10000)   // most of these are low...under 6k  that sux for targeting
        m_entityAttackRange *= 3;
}

void DroneAIMgr::Process() {
    double profileStartTime = GetTimeUSeconds();

    /* Drone::State definitions   -allan 27Nov19
     *   Invalid
     *   Idle              = 0,  // not doing anything....idle.
     *   Combat            = 1,  // fighting - needs targetID
     *   Mining            = 2,  // unsure - needs targetID
     *   Approaching       = 3,  // too close to chase, but to far to engage
     *   Departing         = 4,  // return to ship
     *   Departing2        = 5,  // leaving.  different from Departing
     *   Pursuit           = 6,  // target out of range to attack/follow, but within npc sight range....use mwd/ab if equiped
     *   Fleeing           = 7,  // running away
     *   Operating         = 9,  // whats diff from engaged here?
     *   Engaged           = 10, // non-combat? - needs targetID
     *   // internal only
     *   Unknown           = 8,  // as stated
     *   Guarding          = 11,
     *   Assisting         = 12,
     *   Incapacitated     = 13  //
     */

    // test for drone attributes here - aggressive, focus fire, attack/follow
    switch(m_state) {
        case DroneAI::State::Invalid: {
            // check everything in this state.   return to ship?
        } break;
        case DroneAI::State::Idle: {
            // orbiting controlling ship
        } break;
        case DroneAI::State::Engaged: {
            //NOTE: getting our pTarget like this is pretty weak...
            SystemEntity* pTarget = m_pDrone->TargetMgr()->GetFirstTarget(true);
            if (pTarget == nullptr) {
                if (m_pDrone->TargetMgr()->HasNoTargets()) {
                    _log(DRONE__AI_TRACE, "Drone %s(%u): Stopped engagement, GetFirstTarget() returned NULL.", m_pDrone->GetName(), m_pDrone->GetID());
                    SetIdle();
                }
                return;
            } else if (pTarget->SysBubble() == nullptr) {
                m_pDrone->TargetMgr()->ClearTarget(pTarget);
                //m_pDrone->TargetMgr()->OnTarget(pTarget, TargMgr::Mode::Lost);
                return;
            }
            CheckDistance(pTarget);
        } break;

        case DroneAI::State::Departing: { // return to ship.  when close enough, set lazy orbit
            if (m_pDrone->GetPosition().distance(m_assignedShip->GetPosition()) < m_entityOrbitRange)
                SetIdle();
        } break;
        // not sure how im gonna do these...
        case DroneAI::State::Fleeing:
        case DroneAI::State::Operating:
        case DroneAI::State::Unknown:
        case DroneAI::State::Incapacitated:
        case DroneAI::State::Guarding:
        case DroneAI::State::Assisting:
        case DroneAI::State::Combat:
        case DroneAI::State::Mining:
        case DroneAI::State::Approaching:
        case DroneAI::State::Departing2:
        case DroneAI::State::Pursuit: {
           // do nothing here yet
        } break;

    //no default on purpose
    }
    if (sConfig.debug.UseProfiling)
        sProfile.AddTime(droneProfile, GetTimeUSeconds() - profileStartTime);
}

int8 DroneAIMgr::GetState() {
    switch (m_state) {
        case DroneAI::State::Invalid:
        case DroneAI::State::Unknown:
        case DroneAI::State::Incapacitated:
            return DroneAI::State::Idle;
        case DroneAI::State::Guarding:
        case DroneAI::State::Assisting:
            return DroneAI::State::Engaged;
        default:
            return m_state;
    }
}

void DroneAIMgr::Return() {
    m_assignedShip = m_pDrone->GetHomeShip();
    m_pDrone->DestinyMgr()->SetMaxVelocity(m_chaseSpeed);
    m_pDrone->DestinyMgr()->Follow(m_assignedShip, m_entityOrbitRange);
    m_state = DroneAI::State::Departing;
}

void DroneAIMgr::SetIdle() {
    if (m_state == DroneAI::State::Idle)
        return;
    // not doing anything....idle.
    _log(DRONE__AI_TRACE, "Drone %s(%u): SetIdle: returning to idle.",
         m_pDrone->GetName(), m_pDrone->GetID());
    m_state = DroneAI::State::Idle;

    // orbit assigned ship
    m_pDrone->Online(m_assignedShip);

    m_webifierTimer.Disable();
    m_beginFindTarget.Disable();
    m_mainAttackTimer.Disable();
    m_warpScramblerTimer.Disable();
}

void DroneAIMgr::SetEngaged(SystemEntity* pTarget) {
    if (m_state == DroneAI::State::Engaged)
        return;
    _log(DRONE__AI_TRACE, "Drone %s(%u): SetEngaged: %s(%u) begin engaging.",
         m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID());
    // actively fighting
    //   not sure of the actual orbit speed of npc's, but their 'cruise speed' seems a bit slow.
    //   this sets orbit speed between cruise speed and quarter of max speed (whether mwb or ab)
    //   this will also enable this npc to have a variable speed, instead of fixed upon creation.
    m_pDrone->DestinyMgr()->SetMaxVelocity(MakeRandomFloat(m_cruiseSpeed, (m_chaseSpeed /4)));
    m_pDrone->DestinyMgr()->Orbit(pTarget, m_entityOrbitRange);  //try to get inside orbit range
    m_state = DroneAI::State::Engaged;
}

void DroneAIMgr::CheckDistance(SystemEntity* pSE)
{
    //rewrote distance checks for correct logic this time
    double dist = m_pDrone->GetPosition().distance(pSE->GetPosition());
    if (dist > m_entityAttackRange) {
        _log(DRONE__AI_TRACE, "Drone %s(%u): CheckDistance: %s(%u) is too far away (%u).  Return to Idle.",
             m_pDrone->GetName(), m_pDrone->GetID(), pSE->GetName(), pSE->GetID(), dist);
        if (m_state != DroneAI::State::Idle) {
            // target is no longer in npc's "sight range".  unlock target and return to idle.
            //   should we do anything else here?  search for another target?  wander around?
            ClearTarget(pSE);
        }
        return;
    } else if (dist < m_entityFlyRange) { //within weapon max (and within falloff)
        SetEngaged(pSE); //engage and orbit
    } else if (dist < m_entityChaseRange) { //within follow
       // SetFollowing(pSE);
    } else if (dist < m_entityAttackRange) { //within sight
       // SetChasing(pSE);
        return;
    }

    if (!m_mainAttackTimer.Enabled())
        m_mainAttackTimer.Start(m_attackSpeed);

    Attack(pSE);
}

void DroneAIMgr::ClearTargets() {
    m_pDrone->TargetMgr()->ClearTargets();
}

void DroneAIMgr::ClearAllTargets() {
    m_pDrone->TargetMgr()->ClearAllTargets();
    //m_pDrone->TargetMgr()->OnTarget(nullptr, TargMgr::Mode::Clear, TargMgr::Msg::ClientReq);
}

void DroneAIMgr::Target(SystemEntity* pTarget) {
    bool chase = false;
    if (!m_pDrone->TargetMgr()->StartTargeting(pTarget, m_pDrone->GetSelf()->GetAttribute(AttrScanSpeed).get_uint32(), (uint8)m_pDrone->GetSelf()->GetAttribute(AttrMaxAttackTargets).get_int(), m_entityAttackRange, chase)) {
        _log(DRONE__AI_TRACE, "Drone %s(%u): Targeting of %s(%u) failed.  Clear Target and Return to Idle.",
             m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID());
        //ClearAllTargets();
        SetIdle();
        return;
    }
    m_beginFindTarget.Disable();
    CheckDistance(pTarget);

    /*
    std::map<std::string, PyRep *> arg;
    arg["target"] = new PyInt(args.arg);
    throw PyException(MakeUserError("DeniedDroneTargetForceField", arg));
    */
 //DeniedDroneTargetForceField
}

void DroneAIMgr::Targeted(SystemEntity* pAgressor) {
    switch(m_state) {
        case DroneAI::State::Idle: {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while Idle.",
                 m_pDrone->GetName(), m_pDrone->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case DroneAI::State::Operating: {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while Operating.",
                 m_pDrone->GetName(), m_pDrone->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case DroneAI::State::Unknown: {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while Unknown.",
                 m_pDrone->GetName(), m_pDrone->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case DroneAI::State::Engaged: {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while engaged.",
                 m_pDrone->GetName(), m_pDrone->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case DroneAI::State::Fleeing: {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while fleeing.",
                 m_pDrone->GetName(), m_pDrone->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case DroneAI::State::Incapacitated: {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while Incapacitated.",
                 m_pDrone->GetName(), m_pDrone->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case DroneAI::State::Guarding: {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while Guarding.",
                 m_pDrone->GetName(), m_pDrone->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case DroneAI::State::Assisting: {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while Assisting.",
                 m_pDrone->GetName(), m_pDrone->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case DroneAI::State::Combat: {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while engaged Combat.",
                 m_pDrone->GetName(), m_pDrone->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case DroneAI::State::Mining: {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while Mining.",
                 m_pDrone->GetName(), m_pDrone->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case DroneAI::State::Approaching: {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while Approaching.",
                 m_pDrone->GetName(), m_pDrone->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case DroneAI::State::Departing: {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while engaged Departing.",
                 m_pDrone->GetName(), m_pDrone->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case DroneAI::State::Departing2: {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while Departing2.",
                 m_pDrone->GetName(), m_pDrone->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case DroneAI::State::Pursuit: {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while Pursuit.",
                 m_pDrone->GetName(), m_pDrone->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
    }
}

void DroneAIMgr::TargetLost(SystemEntity* pTarget) {
    switch(m_state) {
        case DroneAI::State::Engaged: {
            if (m_pDrone->TargetMgr()->HasNoTargets()) {
                _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) lost. No targets remain.  Return to Idle.",
                     m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID());
                SetIdle();
            } else {
                _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) lost, but more targets remain.",
                     m_pDrone->GetName(), m_pDrone->GetID(), pTarget->GetName(), pTarget->GetID());
            }

        } break;

        default:
            break;
    }
}

void DroneAIMgr::Attack(SystemEntity* pSE)
{
    if (m_mainAttackTimer.Check()) {
        if (pSE == nullptr)
            return;
        // Check to see if the target still in the bubble (Client warped out)
        // fighters/bombers are able to follow.
        if (!m_pDrone->SysBubble()->InBubble(pSE->GetPosition())) {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) no longer in bubble.  Clear target and move on",
                 m_pDrone->GetName(), m_pDrone->GetID(), pSE->GetName(), pSE->GetID());
            ClearTarget(pSE);
            return;
        }
        DestinyManager* pDestiny = pSE->DestinyMgr();
        if (pDestiny == nullptr) {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) has no destiny manager.  Clear target and move on",
                 m_pDrone->GetName(), m_pDrone->GetID(), pSE->GetName(), pSE->GetID());
            ClearTarget(pSE);
            return;
        }
        // Check to see if the target is not cloaked:
        if (pDestiny->IsCloaked()) {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) is cloaked.  Clear target and move on",
                 m_pDrone->GetName(), m_pDrone->GetID(), pSE->GetName(), pSE->GetID());
            ClearTarget(pSE);
            return;
        }

        if (m_pDrone->TargetMgr()->CanAttack())
            AttackTarget(pSE);
    }
}

void DroneAIMgr::ClearTarget(SystemEntity* pSE) {
    m_pDrone->TargetMgr()->ClearTarget(pSE);
    //m_pDrone->TargetMgr()->OnTarget(pSE, TargMgr::Mode::Lost);

    if (m_pDrone->TargetMgr()->HasNoTargets())
        SetIdle();
}

//also check for special effects and write code to implement them
//modifyTargetSpeedRange, modifyTargetSpeedChance
//entityWarpScrambleChance

void DroneAIMgr::AttackTarget(SystemEntity* pTarget) {
    /** @todo  not all drones use lazors...fix this */
    //  woot!! --> group:1010        cat:8       Compact Citadel Torpedo         Citadel torpedoes for fighter-bombers

    // effects are listed in EVE_Effects.h
    //  NOTE: drones are called 'entities' in client; EVE_Effects has 'entityxxx' for gfx
    std::string guid = "effects.Laser"; // client looks for 'turret' in ship.ball.modules for 'effects.laser'
    //effects.ProjectileFiredForEntities
    uint32 gfxID = 0;
    if (m_pDrone->GetSelf()->HasAttribute(AttrGfxTurretID))// graphicID for turret for drone type ships
        gfxID = m_pDrone->GetSelf()->GetAttribute(AttrGfxTurretID).get_uint32();
    m_pDrone->DestinyMgr()->SendSpecialEffect(m_pDrone->GetSelf()->itemID(),
                                             m_pDrone->GetSelf()->itemID(),
                                             m_pDrone->GetSelf()->typeID(), //m_pDrone->GetSelf()->GetAttribute(AttrGfxTurretID).get_int(),
                                             pTarget->GetID(),
                                             0,guid,1,1,1,m_attackSpeed,0,gfxID);

    Damage d(m_pDrone,
             m_pDrone->GetSelf(),
             m_pDrone->GetKinetic(),
             m_pDrone->GetThermal(),
             m_pDrone->GetEM(),
             m_pDrone->GetExplosive(),
             m_formula.GetDroneToHit(m_pDrone, pTarget),
             EVEEffectID::targetAttack
            );

    d *= m_pDrone->GetSelf()->GetAttribute(AttrDamageMultiplier).get_float();
    d *= sConfig.rates.damageRate;
    pTarget->ApplyDamage(d);
}


std::string DroneAIMgr::GetStateName(int8 stateID)
{
    switch (stateID) {
        case DroneAI::State::Idle:            return "Idle";
        case DroneAI::State::Combat:          return "Combat";
        case DroneAI::State::Mining:          return "Mining";
        case DroneAI::State::Approaching:     return "Approaching";
        case DroneAI::State::Departing:       return "Departing";
        case DroneAI::State::Departing2:      return "Departing2";
        case DroneAI::State::Pursuit:         return "Pursuit";
        case DroneAI::State::Engaged:         return "Engaged";
        case DroneAI::State::Fleeing:         return "Fleeing";
        case DroneAI::State::Unknown:         return "Unknown";
        case DroneAI::State::Operating:       return "Operating";
        case DroneAI::State::Assisting:       return "Assisting";
        case DroneAI::State::Guarding:        return "Guarding";
        case DroneAI::State::Incapacitated:   return "Incapacitated";
        default:                              return "Invalid";
    }
}
