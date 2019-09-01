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
    Author:     Allan (copied from NPC AI)
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
: m_state(Idle),
  m_drone(who),
  m_mainAttackTimer(1000),
  m_processTimer(5000),         //arbitrary.
  m_shieldBoosterTimer(10000),  //arbitrary.
  m_armorRepairTimer(8000),     //arbitrary.
  m_beginFindTarget(5000),      //arbitrary.
  m_warpScramblerTimer(5000),   //arbitrary.
  m_webifierTimer(5000),        //arbitrary.
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

    m_webifierTimer.Disable();      //not implemented yet
    m_beginFindTarget.Disable();    //arbitrary.
    m_mainAttackTimer.Disable();    // dont start timer until we have a target
    m_armorRepairTimer.Disable();   //waiting till engaged
    m_warpScramblerTimer.Disable();    //not implemented yet
    m_shieldBoosterTimer.Disable(); //waiting till engaged

    if (who->GetSelf()->GetAttribute(AttrEntityArmorRepairDelayChanceSmall).get_float())
        m_armorRepairChance = who->GetSelf()->GetAttribute(AttrEntityArmorRepairDelayChanceSmall).get_float();
    else if (who->GetSelf()->GetAttribute(AttrEntityArmorRepairDelayChanceLarge).get_float())
        m_armorRepairChance = who->GetSelf()->GetAttribute(AttrEntityArmorRepairDelayChanceLarge).get_float();

    if (who->GetSelf()->GetAttribute(AttrEntityShieldBoostDelayChanceSmall).get_float())
        m_shieldBoosterChance = who->GetSelf()->GetAttribute(AttrEntityShieldBoostDelayChanceSmall).get_float();
    else if (who->GetSelf()->GetAttribute(AttrEntityShieldBoostDelayChanceLarge).get_float())
        m_shieldBoosterChance = who->GetSelf()->GetAttribute(AttrEntityShieldBoostDelayChanceLarge).get_float();

    if (m_entityAttackRange < 10000)   // most of these are low...under 6k  that sux for targeting
        m_entityAttackRange *= 3;
}

void DroneAIMgr::Process() {
    if ((!m_processTimer.Check()) or (!m_drone->SysBubble()->HasPlayers()) or m_drone->DestinyMgr()->IsWarping())
        return;

    if (m_shieldBoosterTimer.Enabled() && m_shieldBoosterTimer.Check())
        if (MakeRandomFloat() < m_shieldBoosterChance)
            m_drone->UseShieldRecharge();

    if (m_armorRepairTimer.Enabled() && m_armorRepairTimer.Check())
        if (MakeRandomFloat() < m_armorRepairChance)
            m_drone->UseArmorRepairer();

    /* NPC::State definitions   -allan 25July15
     *   Idle,       // not doing anything, nothing in sight....idle.
     *   Chasing,    // target out of range to attack or follow, but within npc sight range....use mwd/ab if equipped
     *   Following,  // too close to chase, but to far to engage...use half of max speed
     *   Engaged,    // actively fighting (in orbit)...use full cruise to quarter max speed.
	 *   Fleeing,    // running away....use mwd/ab (if equipped) then warp away when out of range	(does this make sense??)
	 *   Signaling   // calling for help..use full cruise to half of max speed to speed tank while calling for reinforcements
     */
    switch(m_state) {
        case Idle: {
            // The parameter proximityRange (154) tells us how far we "see" (npc's dont have this, but drones do)
            if (m_beginFindTarget.Check()) {
                std::vector<Client*> clientVec;
                clientVec.clear();
                DestinyManager* pDestiny(nullptr);
                m_drone->SysBubble()->GetPlayers(clientVec); // what about player drones?
                for (auto cur : clientVec) {
                    if (cur->IsLogin() or cur->IsInvul() or cur->InPod())
                        continue;
                    if (!cur->GetShipSE())
                        continue;
                    if ((!cur->GetShipSE()->DestinyMgr()) or (!cur->GetShipSE()->SysBubble()))    // this shouldnt be needed, but whatever...
                        continue;
                    pDestiny = cur->GetShipSE()->DestinyMgr();
                    if (pDestiny->IsCloaked() or pDestiny->IsWarping())
                        continue;
                    if (cur->IsLogin() or cur->IsInvul() or cur->InPod())
                        continue;
                    if (m_drone->GetPosition().distance(cur->GetShipSE()->GetPosition()) > m_entityAttackRange)
                        continue;

                    Target(cur->GetShipSE());
					return;
                }
            } else {
                if (!m_beginFindTarget.Enabled())
                    m_beginFindTarget.Start(m_attackSpeed);  //find target is based on npc attack speed.  trying this instead of hard-coded time.
            }
        } break;

        case Chasing: {
            //NOTE: getting our target like this is pretty weak...
            SystemEntity* pTarget = m_drone->TargetMgr()->GetFirstTarget(true);
            if (!pTarget) {
                if (m_drone->TargetMgr()->HasNoTargets()) {
                    _log(NPC__AI_TRACE, "Drone %s(%u): Stopped chasing, GetFirstTarget() returned NULL.",  m_drone->GetName(), m_drone->GetID());
                    m_state = Idle;
                }
                return;
            } else if (!pTarget->SysBubble()) {
                m_drone->TargetMgr()->ClearTarget(pTarget);
                return;
            }
            CheckDistance(pTarget);
        } break;

        case Following: {
            //NOTE: getting our target like this is pretty weak...
            SystemEntity* pTarget = m_drone->TargetMgr()->GetFirstTarget(true);
            if (!pTarget) {
                if (m_drone->TargetMgr()->HasNoTargets()) {
                    _log(NPC__AI_TRACE, "Drone %s(%u): Stopped following, GetFirstTarget() returned NULL.",  m_drone->GetName(), m_drone->GetID());
                    m_state = Idle;
                }
                return;
            } else if (!pTarget->SysBubble()) {
                m_drone->TargetMgr()->ClearTarget(pTarget);
                return;
            }
            CheckDistance(pTarget);
        } break;

        case Engaged: {
            //NOTE: getting our pTarget like this is pretty weak...
            SystemEntity* pTarget = m_drone->TargetMgr()->GetFirstTarget(true);
            if (!pTarget) {
                if (m_drone->TargetMgr()->HasNoTargets()) {
                    _log(NPC__AI_TRACE, "Drone %s(%u): Stopped engagement, GetFirstTarget() returned NULL.", m_drone->GetName(), m_drone->GetID());
                    SetIdle();
                }
                return;
            } else if (!pTarget->SysBubble()) {
                m_drone->TargetMgr()->ClearTarget(pTarget);
                return;
            }
            CheckDistance(pTarget);
        } break;

        case Fleeing: {
            // not sure how im gonna do this one yet.
        } break;

		case Signaling: {
			// not sure how im gonna do this one yet.
        } break;

    //no default on purpose
    }
}

void DroneAIMgr::SetIdle() {
    if (m_state == Idle) return;
    // not doing anything....idle.
    _log(NPC__AI_TRACE, "Drone %s(%u): SetIdle: returning to idle.",
         m_drone->GetName(), m_drone->GetID());
    m_state = Idle;
    m_drone->DestinyMgr()->Stop();
    m_drone->DestinyMgr()->SetMaxVelocity(m_cruiseSpeed);

    m_webifierTimer.Disable();
    m_beginFindTarget.Disable();
    m_mainAttackTimer.Disable();
    m_armorRepairTimer.Disable();
    m_warpScramblerTimer.Disable();
    m_shieldBoosterTimer.Disable();

    // write code to enable npcs to wander around when idle?
    // sounds like a good idea, but will take process power away from other shit.
}

void DroneAIMgr::SetChasing(SystemEntity* pTarget) {
    if (m_state == Chasing) return;
    _log(NPC__AI_TRACE, "Drone %s(%u): SetChasing: %s(%u) begin chasing.",
         m_drone->GetName(), m_drone->GetID(), pTarget->GetName(), pTarget->GetID());
    // target out of range to attack/follow, but within npc sight range....use mwd/ab if equiped
    m_drone->DestinyMgr()->SetMaxVelocity(m_chaseSpeed);
    m_drone->DestinyMgr()->Follow(pTarget, m_entityOrbitRange);  //try to get inside orbit range
    m_state = Chasing;
}

void DroneAIMgr::SetFollowing(SystemEntity* pTarget) {
    if (m_state == Following) return;
    _log(NPC__AI_TRACE, "Drone %s(%u): SetFollowing: %s(%u) begin following.",
         m_drone->GetName(), m_drone->GetID(), pTarget->GetName(), pTarget->GetID());
    // too close to chase, but to far to engage
    m_drone->DestinyMgr()->SetMaxVelocity(m_chaseSpeed /2);
    m_drone->DestinyMgr()->Follow(pTarget, m_entityOrbitRange);  //try to get inside orbit range
    m_state = Following;
}

void DroneAIMgr::SetEngaged(SystemEntity* pTarget) {
    if (m_state == Engaged) return;
    _log(NPC__AI_TRACE, "Drone %s(%u): SetEngaged: %s(%u) begin engaging.",
         m_drone->GetName(), m_drone->GetID(), pTarget->GetName(), pTarget->GetID());
    // actively fighting
    //   not sure of the actual orbit speed of npc's, but their 'cruise speed' seems a bit slow.
    //   this sets orbit speed between cruise speed and quarter of max speed (whether mwb or ab)
    //   this will also enable this npc to have a variable speed, instead of fixed upon creation.
    m_drone->DestinyMgr()->SetMaxVelocity(MakeRandomFloat(m_cruiseSpeed, (m_chaseSpeed /4)));
    m_drone->DestinyMgr()->Orbit(pTarget, m_entityOrbitRange);  //try to get inside orbit range
    m_state = Engaged;
}

void DroneAIMgr::SetFleeing(SystemEntity* pTarget) {
    if (m_state == Fleeing) return;
    _log(NPC__AI_TRACE, "Drone %s(%u): SetFleeing: %s(%u) begin fleeing.",
         m_drone->GetName(), m_drone->GetID(), pTarget->GetName(), pTarget->GetID());
    // actively fleeing
    //  use superspeed to disengage, then warp.  << both these will need to be written.
    //  this state is only usable by higher-class npcs.
    m_drone->DestinyMgr()->SetMaxVelocity(m_chaseSpeed);
    m_state = Fleeing;
}

void DroneAIMgr::SetSignaling(SystemEntity* pTarget) {
    if (m_state == Signaling) return;
    _log(NPC__AI_TRACE, "Drone %s(%u): SetSignaling: %s(%u) begin signaling.",
         m_drone->GetName(), m_drone->GetID(), pTarget->GetName(), pTarget->GetID());
    // actively signaling
    //  start speedtanking while signaling.  (im sure this is cheating, but fuckem.)
    //  this state is only usable by higher-class npcs.
    m_drone->DestinyMgr()->SetMaxVelocity(MakeRandomFloat(m_cruiseSpeed, (m_chaseSpeed /2)));
    m_drone->DestinyMgr()->Orbit(pTarget, m_entityOrbitRange);  //try to get inside orbit range
    m_state = Signaling;
}

void DroneAIMgr::CheckDistance(SystemEntity* pSE)
{
    //rewrote distance checks for correct logic this time
    double dist = m_drone->GetPosition().distance(pSE->GetPosition());
    if (dist > m_entityAttackRange) {
        _log(NPC__AI_TRACE, "Drone %s(%u): CheckDistance: %s(%u) is too far away (%u).  Return to Idle.",
             m_drone->GetName(), m_drone->GetID(), pSE->GetName(), pSE->GetID(), dist);
        if (m_state != Idle) {
            // target is no longer in npc's "sight range".  unlock target and return to idle.
            //   should we do anything else here?  search for another target?  wander around?
            m_drone->TargetMgr()->ClearTarget(pSE);
            if (m_drone->TargetMgr()->HasNoTargets())
                SetIdle();
        }
        return;
    } else if (dist < m_entityFlyRange) { //within weapon max (and within falloff)
        SetEngaged(pSE); //engage and orbit
    } else if (dist < m_entityChaseRange) { //within follow
        SetFollowing(pSE);
    } else if (dist < m_entityAttackRange) { //within sight
        SetChasing(pSE);
        return;
    }

    if (m_shieldBoosterDuration && (!m_shieldBoosterTimer.Enabled()))
        m_shieldBoosterTimer.Start(m_shieldBoosterDuration);
    if (m_armorRepairDuration && (!m_armorRepairTimer.Enabled()))
        m_armorRepairTimer.Start(m_armorRepairDuration);

    if (!m_mainAttackTimer.Enabled())
        m_mainAttackTimer.Start(m_attackSpeed);

    Attack(pSE);
}

void DroneAIMgr::ClearTargets() {
    m_drone->TargetMgr()->ClearTargets();
}

void DroneAIMgr::ClearAllTargets() {
    m_drone->TargetMgr()->ClearAllTargets();
}

void DroneAIMgr::Target(SystemEntity* pTarget) {
    double targetTime = GetTargetTime();
    bool chase = false;
	if (!m_drone->TargetMgr()->StartTargeting(pTarget, targetTime, (uint8)m_drone->GetSelf()->GetAttribute(AttrMaxAttackTargets).get_int(), m_entityAttackRange, chase)) {
        _log(NPC__AI_TRACE, "Drone %s(%u): Targeting of %s(%u) failed.  Clear Target and Return to Idle.",
             m_drone->GetName(), m_drone->GetID(), pTarget->GetName(), pTarget->GetID());
        //ClearAllTargets();
        SetIdle();
        return;
    }
    m_beginFindTarget.Disable();
    CheckDistance(pTarget);

    /*
    std::map<std::string, PyRep *> arg;
    arg["target"] = new PyInt(args.arg);
    throw PyException( MakeUserError( "DeniedDroneTargetForceField", arg));DeniedDroneTargetForceField
    */

}

void DroneAIMgr::Targeted(SystemEntity* pAgressor) {
    double targetTime = GetTargetTime();

    switch(m_state) {
        case Idle: {
            _log(NPC__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) in Idle. Begin Approaching and start Targeting sequence.",
                 m_drone->GetName(), m_drone->GetID(), pAgressor->GetName(), pAgressor->GetID());
            SetChasing(pAgressor);

            bool chase = false;
			if (!m_drone->TargetMgr()->StartTargeting( pAgressor, targetTime, (uint8)m_drone->GetSelf()->GetAttribute(AttrMaxAttackTargets).get_int(), m_entityAttackRange, chase)) {
                SetIdle();
                return;
            }
            m_beginFindTarget.Disable();
            CheckDistance(pAgressor);
        } break;

        case Chasing: {
            _log(NPC__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while chasing.",
                 m_drone->GetName(), m_drone->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case Following: {
            _log(NPC__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while following.",
                 m_drone->GetName(), m_drone->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case Engaged: {
            _log(NPC__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while engaged.",
                 m_drone->GetName(), m_drone->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case Fleeing: {
            _log(NPC__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while fleeing.",
                 m_drone->GetName(), m_drone->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case Signaling: {
            _log(NPC__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while signaling.",
                 m_drone->GetName(), m_drone->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;

        //no default on purpose
    }
}

void DroneAIMgr::TargetLost(SystemEntity* pTarget) {
    switch(m_state) {
        case Chasing:
        case Following:
        case Engaged: {
            if (m_drone->TargetMgr()->HasNoTargets()) {
                _log(NPC__AI_TRACE, "Drone %s(%u): Target %s(%u) lost. No targets remain.  Return to Idle.",
                     m_drone->GetName(), m_drone->GetID(), pTarget->GetName(), pTarget->GetID());
                SetIdle();
            } else {
                _log(NPC__AI_TRACE, "Drone %s(%u): Target %s(%u) lost, but more targets remain.",
                     m_drone->GetName(), m_drone->GetID(), pTarget->GetName(), pTarget->GetID());
            }

        } break;

        default:
            break;
    }
}

void DroneAIMgr::Attack(SystemEntity* pSE)
{
    if (m_mainAttackTimer.Check()) {
        if (!pSE) return;
        // Check to see if the target still in the bubble (Client warped out)
        if (!m_drone->SysBubble()->InBubble(pSE->GetPosition())) {
            _log(NPC__AI_TRACE, "Drone %s(%u): Target %s(%u) no longer in bubble.  Clear target and move on",
                 m_drone->GetName(), m_drone->GetID(), pSE->GetName(), pSE->GetID());
            m_drone->TargetMgr()->ClearTarget(pSE);
            return;
        }
        DestinyManager* pDestiny = pSE->DestinyMgr();
        if (!pDestiny) {
            _log(NPC__AI_TRACE, "Drone %s(%u): Target %s(%u) has no destiny manager.  Clear target and move on",
                 m_drone->GetName(), m_drone->GetID(), pSE->GetName(), pSE->GetID());
            m_drone->TargetMgr()->ClearTarget(pSE);
            return;
        }
        // Check to see if the target is not cloaked:
        if (pDestiny->IsCloaked()) {
            _log(NPC__AI_TRACE, "Drone %s(%u): Target %s(%u) is cloaked.  Clear target and move on",
                 m_drone->GetName(), m_drone->GetID(), pSE->GetName(), pSE->GetID());
            m_drone->TargetMgr()->ClearTarget(pSE);
            return;
        }

        if (m_drone->TargetMgr()->CanAttack())
            AttackTarget(pSE);
    }
}

//also check for special effects and write code to implement them
//modifyTargetSpeedRange, modifyTargetSpeedChance
//entityWarpScrambleChance

void DroneAIMgr::AttackTarget(SystemEntity* pTarget) {
    /** @todo  not all drones use lazors...fix this */
    std::string guid = "effects.Laser";
    m_drone->DestinyMgr()->SendSpecialEffect(m_drone->GetSelf()->itemID(),
                                             m_drone->GetSelf()->itemID(),
                                             m_drone->GetSelf()->typeID(), //m_drone->GetSelf()->GetAttribute(AttrGfxTurretID).get_int(),
                                             pTarget->GetID(),
                                             0,guid,1,1,1,m_attackSpeed,0);

    Damage d(m_drone,
             m_drone->GetSelf(),
             m_drone->GetKinetic(),
             m_drone->GetThermal(),
             m_drone->GetEM(),
             m_drone->GetExplosive(),
             m_formula.GetDroneToHit(m_drone, pTarget),
             EVEEffectID::targetAttack
            );

    d *= m_drone->GetSelf()->GetAttribute(AttrDamageMultiplier).get_float();
    pTarget->ApplyDamage(d);
}

double DroneAIMgr::GetTargetTime()
{
    double targetTime = (m_drone->GetSelf()->GetAttribute(AttrScanSpeed).get_int());
    float radius = m_drone->GetSelf()->GetAttribute(AttrRadius).get_float();
    if (!targetTime) {
        if (radius < 30)
            targetTime = 1500;
        else if (radius < 60)
            targetTime = 2500;
        else if (radius < 150)
            targetTime = 4000;
        else if (radius < 280)
            targetTime = 6000;
        else if (radius < 550)
            targetTime = 8000;
        else
            targetTime = 13000;
    }
    return targetTime;
}

void DroneAIMgr::DisableRepTimers()
{
    m_armorRepairTimer.Disable();
    m_shieldBoosterTimer.Disable();
}
