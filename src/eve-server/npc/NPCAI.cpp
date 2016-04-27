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
    Author:        Zhur
    Updates:        Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "inventory/AttributeEnum.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "ship/DestinyManager.h"
#include "system/Damage.h"
#include "system/SystemBubble.h"

NPCAIMgr::NPCAIMgr(NPC* who)
: m_state(Idle),
  m_npc(who),
  m_mainAttackTimer(1000),
  m_processTimer(5000),         //arbitrary.
  m_shieldBoosterTimer(10000),  //arbitrary.
  m_armorRepairTimer(8000),     //arbitrary.
  m_beginFindTarget(5000),      //arbitrary.
  m_warpScramblerTimer(5000),   //arbitrary.
  m_webifierTimer(5000),        //arbitrary.
  m_radius(who->Item()->GetAttribute(AttrSignatureRadius).get_float()),
  m_attackSpeed(who->Item()->GetAttribute(AttrSpeed).get_float()),
  m_cruiseSpeed(who->Item()->GetAttribute(AttrEntityCruiseSpeed).get_int()),
  m_chaseSpeed(who->Item()->GetAttribute(AttrMaxVelocity).get_int()),
  m_entityFlyRange(who->Item()->GetAttribute(AttrEntityFlyRange).get_float() + who->Item()->GetAttribute(AttrMaxRange).get_float()),
  m_entityChaseRange(who->Item()->GetAttribute(AttrEntityChaseMaxDistance).get_float() *2),
  m_entityOrbitRange(who->Item()->GetAttribute(AttrMaxRange).get_float()),
  m_entityAttackRange(who->Item()->GetAttribute(AttrEntityAttackRange).get_float() *2),
  m_shieldBoosterDuration(who->Item()->GetAttribute(AttrEntityShieldBoostDuration).get_int()),
  m_armorRepairDuration(who->Item()->GetAttribute(AttrEntityArmorRepairDuration).get_int())
{
    m_processTimer.Start(5000);     //arbitrary.

    m_webifierTimer.Disable();      //not implemented yet
    m_beginFindTarget.Disable();    //arbitrary.
    m_mainAttackTimer.Disable();    // dont start timer until we have a target
    m_armorRepairTimer.Disable();   //waiting till engaged
    m_warpScramblerTimer.Disable();    //not implemented yet
    m_shieldBoosterTimer.Disable(); //waiting till engaged

    if (who->Item()->GetAttribute(AttrEntityArmorRepairDelayChanceSmall).get_float())
        m_armorRepairChance = who->Item()->GetAttribute(AttrEntityArmorRepairDelayChanceSmall).get_float();
    else if (who->Item()->GetAttribute(AttrEntityArmorRepairDelayChanceLarge).get_float())
        m_armorRepairChance = who->Item()->GetAttribute(AttrEntityArmorRepairDelayChanceLarge).get_float();

    if (who->Item()->GetAttribute(AttrEntityShieldBoostDelayChanceSmall).get_float())
        m_shieldBoosterChance = who->Item()->GetAttribute(AttrEntityShieldBoostDelayChanceSmall).get_float();
    else if (who->Item()->GetAttribute(AttrEntityShieldBoostDelayChanceLarge).get_float())
        m_shieldBoosterChance = who->Item()->GetAttribute(AttrEntityShieldBoostDelayChanceLarge).get_float();

    if (m_entityAttackRange < 10000)   // most of these are low...under 6k  that sux for targeting
        m_entityAttackRange *= 3;
}

void NPCAIMgr::Process() {
    if ((!m_processTimer.Check()) || (!m_npc->Bubble()->HasPlayers()) || m_npc->Destiny()->IsWarping())
        return;

    if (m_shieldBoosterTimer.Enabled() && m_shieldBoosterTimer.Check())
        if (MakeRandomFloat() < m_shieldBoosterChance)
            m_npc->UseShieldRecharge();

    if (m_armorRepairTimer.Enabled() && m_armorRepairTimer.Check())
        if (MakeRandomFloat() < m_armorRepairChance)
            m_npc->UseArmorRepairer();

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
                m_npc->Bubble()->GetPlayers(clientVec); // what about player drones?
                std::vector<Client*>::iterator cur = clientVec.begin();
                // TODO: Determine the weakest target to engage
                for (; cur != clientVec.end(); cur++) {
                    if ((!(*cur)->Destiny()) || (!(*cur)->Bubble()))    // this shouldnt be needed, but whatever...
                        continue;
                    DestinyManager* pDestiny = (*cur)->Destiny();
                    if (pDestiny->IsCloaked() || pDestiny->IsWarping())
                        continue;
                    if ((*cur)->IsLogin() || (*cur)->IsInvul() || (*cur)->InPod())  // remove InPod() to allow npc podkilling
                        continue;
                    if (m_npc->GetPosition().distance((*cur)->GetPosition()) > m_entityAttackRange)
                        continue;

                    Target(*cur);
					return;
                }
            } else {
                if (!m_beginFindTarget.Enabled())
                    m_beginFindTarget.Start(m_attackSpeed);  //find target is based on npc attack speed.  trying this instead of hard-coded time.
            }
        } break;

        case Chasing: {
            //NOTE: getting our target like this is pretty weak...
            SystemEntity* pTarget = m_npc->TargMgr.GetFirstTarget(true);
            if (!pTarget) {
                if (m_npc->TargMgr.HasNoTargets()) {
                    _log(NPC__AI_TRACE, "%s(%u): Stopped chasing, GetFirstTarget() returned NULL.",  m_npc->GetName(), m_npc->GetID());
                    m_state = Idle;
                }
                return;
            } else if (!pTarget->Bubble()) {
                m_npc->TargMgr.ClearTarget(pTarget);
                return;
            }
            _CheckDistance(pTarget);
        } break;

        case Following: {
            //NOTE: getting our target like this is pretty weak...
            SystemEntity* pTarget = m_npc->TargMgr.GetFirstTarget(true);
            if (!pTarget) {
                if (m_npc->TargMgr.HasNoTargets()) {
                    _log(NPC__AI_TRACE, "%s(%u): Stopped following, GetFirstTarget() returned NULL.",  m_npc->GetName(), m_npc->GetID());
                    m_state = Idle;
                }
                return;
            } else if (!pTarget->Bubble()) {
                m_npc->TargMgr.ClearTarget(pTarget);
                return;
            }
            _CheckDistance(pTarget);
        } break;

        case Engaged: {
            //NOTE: getting our pTarget like this is pretty weak...
            SystemEntity* pTarget = m_npc->TargMgr.GetFirstTarget(true);
            if (!pTarget) {
                if (m_npc->TargMgr.HasNoTargets()) {
                    _log(NPC__AI_TRACE, "%s(%u): Stopped engagement, GetFirstTarget() returned NULL.", m_npc->GetName(), m_npc->GetID());
                    _EnterIdle();
                }
                return;
            } else if (!pTarget->Bubble()) {
                m_npc->TargMgr.ClearTarget(pTarget);
                return;
            }
            _CheckDistance(pTarget);
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

void NPCAIMgr::_EnterIdle() {
    if (m_state == Idle) return;
    // not doing anything....idle.
    _log(NPC__AI_TRACE, "%s(%u): _EnterIdle: returning to idle.",
         m_npc->GetName(), m_npc->GetID());
    m_state = Idle;
    m_npc->Destiny()->Stop();
    m_npc->Destiny()->SetMaxVelocity(m_cruiseSpeed);

    m_webifierTimer.Disable();
    m_beginFindTarget.Disable();
    m_mainAttackTimer.Disable();
    m_armorRepairTimer.Disable();
    m_warpScramblerTimer.Disable();
    m_shieldBoosterTimer.Disable();

    // write code to enable npcs to wander around when idle?
    // sounds like a good idea, but will take process power away from other shit.
}

void NPCAIMgr::_EnterChasing(SystemEntity* pTarget) {
    if (m_state == Chasing) return;
    _log(NPC__AI_TRACE, "%s(%u): _EnterChasing: %s(%u) begin chasing.",
         m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
    // target out of range to attack/follow, but within npc sight range....use mwd/ab if equiped
    m_npc->Destiny()->SetMaxVelocity(m_chaseSpeed);
    m_npc->Destiny()->Follow(pTarget, m_entityOrbitRange);  //try to get inside orbit range
    m_state = Chasing;
}

void NPCAIMgr::_EnterFollowing(SystemEntity* pTarget) {
    if (m_state == Following) return;
    _log(NPC__AI_TRACE, "%s(%u): _EnterFollowing: %s(%u) begin following.",
         m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
    // too close to chase, but to far to engage
    m_npc->Destiny()->SetMaxVelocity(m_chaseSpeed /2);
    m_npc->Destiny()->Follow(pTarget, m_entityOrbitRange);  //try to get inside orbit range
    m_state = Following;
}

void NPCAIMgr::_EnterEngaged(SystemEntity* pTarget) {
    if (m_state == Engaged) return;
    _log(NPC__AI_TRACE, "%s(%u): _EnterEngaged: %s(%u) begin engaging.",
         m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
    // actively fighting
    //   not sure of the actual orbit speed of npc's, but their 'cruise speed' seems a bit slow.
    //   this sets orbit speed between cruise speed and quarter of max speed (whether mwb or ab)
    //   this will also enable this npc to have a variable speed, instead of fixed upon creation.
    m_npc->Destiny()->SetMaxVelocity(MakeRandomFloat(m_cruiseSpeed, (m_chaseSpeed /4)));
    m_npc->Destiny()->Orbit(pTarget, m_entityOrbitRange);  //try to get inside orbit range
    m_state = Engaged;
}

void NPCAIMgr::_EnterFleeing(SystemEntity* pTarget) {
    if (m_state == Fleeing) return;
    _log(NPC__AI_TRACE, "%s(%u): _EnterFleeing: %s(%u) begin fleeing.",
         m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
    // actively fleeing
    //  use superspeed to disengage, then warp.  << both these will need to be written.
    //  this state is only usable by higher-class npcs.
    m_npc->Destiny()->SetMaxVelocity(m_chaseSpeed);
    m_state = Fleeing;
}

void NPCAIMgr::_EnterSignaling(SystemEntity* pTarget) {
    if (m_state == Signaling) return;
    _log(NPC__AI_TRACE, "%s(%u): _EnterSignaling: %s(%u) begin signaling.",
         m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
    // actively signaling
    //  start speedtanking while signaling.  (im sure this is cheating, but fuckem.)
    //  this state is only usable by higher-class npcs.
    m_npc->Destiny()->SetMaxVelocity(MakeRandomFloat(m_cruiseSpeed, (m_chaseSpeed /2)));
    m_npc->Destiny()->Orbit(pTarget, m_entityOrbitRange);  //try to get inside orbit range
    m_state = Signaling;
}

void NPCAIMgr::_CheckDistance(SystemEntity* pTarget)
{//rewrote distance checks for correct logic this time
    DynamicSystemEntity* pDSE = static_cast<DynamicSystemEntity *>(pTarget);
    GVector usToThem(m_npc->GetPosition(), pDSE->GetPosition());
    //double dist = m_npc->GetPosition().distance(pDSE->GetPosition());     // this throws occasional errors (segfault)
    double dist = usToThem.length();
    if (dist > m_entityAttackRange) {
        _log(NPC__AI_TRACE, "%s(%u): _CheckDistance: %s(%u) is too far away (%u).  Return to Idle.",
             m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID(), dist);
        if (m_state != Idle) {
            // target is no longer in npc's "sight range".  unlock target and return to idle.
            //   should we do anything else here?  search for another target?  wander around?
            m_npc->TargMgr.ClearTarget(pTarget);
            if (m_npc->TargMgr.HasNoTargets())
                _EnterIdle();
        }
        return;
    } else if (dist < m_entityFlyRange) { //within weapon max (and within falloff)
        _EnterEngaged(pTarget); //engage and orbit
    } else if (dist < m_entityChaseRange) { //within follow
        _EnterFollowing(pTarget);
    } else if (dist < m_entityAttackRange) { //within sight
        _EnterChasing(pTarget);
        return;
    }

    if (m_shieldBoosterDuration && (!m_shieldBoosterTimer.Enabled()))
        m_shieldBoosterTimer.Start(m_shieldBoosterDuration);
    if (m_armorRepairDuration && (!m_armorRepairTimer.Enabled()))
        m_armorRepairTimer.Start(m_armorRepairDuration);

    if (!m_mainAttackTimer.Enabled())
        m_mainAttackTimer.Start(m_attackSpeed);

    Attack(pTarget);
}

void NPCAIMgr::ClearTargets() {
    m_npc->TargMgr.ClearTargets();
}

void NPCAIMgr::ClearAllTargets() {
    m_npc->TargMgr.ClearAllTargets();
}

void NPCAIMgr::Target(SystemEntity* pTarget) {
    double targetTime = GetTargetTime();

    if (!m_npc->TargMgr.StartTargeting(pTarget, targetTime, m_npc->Item()->GetAttribute(AttrMaxAttackTargets).get_int(), m_entityAttackRange )) {
        _log(NPC__AI_TRACE, "%s(%u): Targeting of %s(%u) failed.  Clear Target and Return to Idle.",
             m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
        //ClearAllTargets();
        _EnterIdle();
        return;
    }
    m_beginFindTarget.Disable();
    _CheckDistance(pTarget);
}

void NPCAIMgr::Targeted(SystemEntity* pAgressor) {
    double targetTime = GetTargetTime();

    switch(m_state) {
        case Idle: {
            _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) in Idle. Begin Approaching and start Targeting sequence.",
                 m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());
            _EnterChasing(pAgressor);

            if (!m_npc->TargMgr.StartTargeting( pAgressor, targetTime, m_npc->Item()->GetAttribute(AttrMaxAttackTargets).get_int(), m_entityAttackRange)) {
                _EnterIdle();
                return;
            }
            m_beginFindTarget.Disable();
            _CheckDistance(pAgressor);
        } break;

        //TODO  determine if new targetedby entity is weaker than current target.
        case Chasing: {
            _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) while chasing.",
                 m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case Following: {
            _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) while following.",
                 m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case Engaged: {
            _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) while engaged.",
                 m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case Fleeing: {
            _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) while fleeing.",
                 m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case Signaling: {
            _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) while signaling.",
                 m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;

        //no default on purpose
    }
}

void NPCAIMgr::TargetLost(SystemEntity* pTarget) {
    switch(m_state) {
        case Chasing:
        case Following:
        case Engaged: {
            if (m_npc->TargMgr.HasNoTargets()) {
                _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) lost. No targets remain.  Return to Idle.",
                     m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
                _EnterIdle();
            } else {
                _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) lost, but more targets remain.",
                     m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
                //TODO engage weakest target in current list
            }

        } break;

        default:
            break;
    }
}

void NPCAIMgr::Attack(SystemEntity* pTarget)
{
    if (m_mainAttackTimer.Check()) {
        if (!pTarget) return;
        // Check to see if the target still in the bubble (Client warped out)
        if (!m_npc->Bubble()->InBubble(pTarget->GetPosition())) {
            _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) no longer in bubble.  Clear target and move on",
                 m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
            m_npc->TargMgr.ClearTarget(pTarget);
            return;
        }
        // only check i can think of right now to verify target is client, npc, or drone
        DynamicSystemEntity* pDSE = static_cast<DynamicSystemEntity *>(pTarget);
        DestinyManager* pDestiny = pDSE->Destiny();
        if (!pDestiny) {
            _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) has no destiny manager.  Clear target and move on",
                 m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
            m_npc->TargMgr.ClearTarget(pTarget);
            return;
        }
        // Check to see if the target is not cloaked:
        if (pDestiny->IsCloaked()) {
            _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) is cloaked.  Clear target and move on",
                 m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
            m_npc->TargMgr.ClearTarget(pTarget);
            return;
        }

        if (m_npc->TargMgr.CanAttack())
            AttackTarget(pTarget);
    }
}

//also check for special effects and write code to implement them
//modifyTargetSpeedRange, modifyTargetSpeedChance
//entityWarpScrambleChance

void NPCAIMgr::AttackTarget(SystemEntity* pTarget) {
    // some npcs use missiles.
    //  write code for using missiles   -- entityMissileTypeID
    _SendWeaponEffect("effects.Laser", pTarget);

    Damage d(m_npc,
             m_npc->Item(),
             m_npc->GetKinetic(),
             m_npc->GetThermal(),
             m_npc->GetEM(),
             m_npc->GetExplosive(),
             m_formula.GetNPCToHit(m_npc, pTarget),
             effectTargetAttack
            );

    d *= m_npc->Item()->GetAttribute(AttrDamageMultiplier).get_float();
    pTarget->ApplyDamage(d);
}

//NOTE: duplicated from module manager code. They should share some day!
void NPCAIMgr::_SendWeaponEffect( const char*effect, SystemEntity* pTarget ) {
    DoDestiny_OnSpecialFX13 sfx;
    sfx.entityID = m_npc->Item()->itemID();
    sfx.moduleID = m_npc->Item()->itemID();
    sfx.moduleTypeID = m_npc->Item()->typeID();
    sfx.targetID = pTarget->GetID();
    sfx.otherTypeID = pTarget->Item()->typeID();
    sfx.effect_type = effect;
    sfx.isOffensive = 1;
    sfx.start = 1;
    sfx.active = 1;
    sfx.duration_ms = m_attackSpeed;
    sfx.repeat = 1;
    sfx.startTime = Win32TimeNow();

    PyTuple* up = sfx.Encode();
    m_npc->Destiny()->SendSingleDestinyUpdate( &up );    //consumed
}

double NPCAIMgr::GetTargetTime()
{
    double targetTime = (m_npc->Item()->GetAttribute(AttrScanSpeed).get_int());
    if (!targetTime) {
        if (m_npc->Item()->GetAttribute(AttrRadius) < 30)
            targetTime = 1500;
        else if (m_npc->Item()->GetAttribute(AttrRadius) < 60)
            targetTime = 2500;
        else if (m_npc->Item()->GetAttribute(AttrRadius) < 150)
            targetTime = 4000;
        else if (m_npc->Item()->GetAttribute(AttrRadius) < 280)
            targetTime = 6000;
        else if (m_npc->Item()->GetAttribute(AttrRadius) < 550)
            targetTime = 8000;
        else
            targetTime = 13000;
    }
    return targetTime;
}

void NPCAIMgr::DisableRepTimers()
{
    m_armorRepairTimer.Disable();
    m_shieldBoosterTimer.Disable();
}
