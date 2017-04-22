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
    Updates:    Allan
    AI Version: 0.43
*/

#include "eve-server.h"

#include "Client.h"
#include "inventory/AttributeEnum.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "system/DestinyManager.h"
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
  m_webifierTimer(5000)         //arbitrary.
{
    m_webifierTimer.Disable();      //not implemented yet
    m_beginFindTarget.Disable();    //arbitrary.
    m_mainAttackTimer.Disable();    //waiting till engaged
    m_armorRepairTimer.Disable();   //waiting till engaged
    m_shieldBoosterTimer.Disable(); //waiting till engaged
    m_warpScramblerTimer.Disable(); //not implemented yet

    m_webber = false;
    m_warpScram = false;
    m_isWandering = false;

    m_damageMultiplier = who->GetSelf()->GetAttribute(AttrDamageMultiplier).get_int();

    /* set npc ship speeds and distances */
    m_radius = who->GetSelf()->GetAttribute(AttrSignatureRadius).get_int();
    m_ROF = who->GetSelf()->GetAttribute(AttrSpeed).get_int();
    m_processTimer.Start(m_ROF);

    /** @todo  all of these need to be verified and/or updated */
    // Optimal Range
    m_optimalRange = who->GetSelf()->GetAttribute(AttrMaxRange).get_int();
    // Accuracy falloff  (distance past maximum range at which accuracy has fallen by half)
    m_falloff = who->GetSelf()->GetAttribute(AttrFalloff).get_int();
    m_trackingSpeed = who->GetSelf()->GetAttribute(AttrTrackingSpeed).get_float();
    // Orbit Velocity
    m_orbitSpeed = who->GetSelf()->GetAttribute(AttrEntityCruiseSpeed).get_int();
    // Orbit Range, Follow Range  - npc tries to stay at this distance from active target    default:800
    m_flyRange = who->GetSelf()->GetAttribute(AttrEntityFlyRange).get_int();
    if (!m_flyRange)
        m_flyRange = 800;
    // distance for Speed Boost activation   default:5000
    m_boostRange = who->GetSelf()->GetAttribute(AttrEntityChaseMaxDistance).get_int();
    if (!m_boostRange)
        m_boostRange = 5000;
    // max firing range   default:25000
    m_maxAttackRange = who->GetSelf()->GetAttribute(AttrEntityAttackRange).get_int();
    if (!m_maxAttackRange)
        m_maxAttackRange = 25000;
    // 'sight' range
    m_sightRange = 20000;
    if (m_maxAttackRange > m_sightRange)
        m_sightRange = m_maxAttackRange *2;

    if (who->GetSelf()->GetAttribute(AttrEntityArmorRepairDelayChanceSmall).get_int())
        m_armorRepairChance = who->GetSelf()->GetAttribute(AttrEntityArmorRepairDelayChanceSmall).get_int();
    else if (who->GetSelf()->GetAttribute(AttrEntityArmorRepairDelayChanceLarge).get_int())
        m_armorRepairChance = who->GetSelf()->GetAttribute(AttrEntityArmorRepairDelayChanceLarge).get_int();
    else
        m_armorRepairChance = 0;

    if (m_armorRepairChance)
        m_armorRepairDuration = who->GetSelf()->GetAttribute(AttrEntityArmorRepairDuration).get_int();
    else
        m_armorRepairDuration = 0;

    if (who->GetSelf()->HasAttribute(AttrEntityShieldBoostDelayChanceSmall))
        m_shieldBoosterChance = who->GetSelf()->GetAttribute(AttrEntityShieldBoostDelayChanceSmall).get_float();
    else if (who->GetSelf()->HasAttribute(AttrEntityShieldBoostDelayChanceLarge))
        m_shieldBoosterChance = who->GetSelf()->GetAttribute(AttrEntityShieldBoostDelayChanceLarge).get_float();
    else
        m_shieldBoosterChance = 0;

    if (m_shieldBoosterChance)
        m_shieldBoosterDuration = who->GetSelf()->GetAttribute(AttrEntityShieldBoostDuration).get_int();
    else
        m_shieldBoosterDuration = 0;

    // advanced AI variables  only used by sleepers for now.  will update advanced npcs to use these also
    if (who->GetSelf()->HasAttribute(AttrAI_ShouldUseTargetSwitching))
        m_useTargSwitching = true;
    else
        m_useTargSwitching = false;
    if (who->GetSelf()->HasAttribute(AttrAI_ShouldUseSecondaryTarget))
        m_useSecondTarget = true;
    else
        m_useSecondTarget = false;
    if (who->GetSelf()->HasAttribute(AttrAI_ShouldUseSignatureRadius)) {
        m_useSigRadius = true;
        m_preferedSigRadius = who->GetSelf()->GetAttribute(AttrAI_PreferredSignatureRadius).get_int();
    } else {
        m_useSigRadius = false;
        m_preferedSigRadius = 0;
    }
    if (who->GetSelf()->HasAttribute(AttrAI_ChanceToNotTargetSwitch))
        m_switchTargChance = 1.0 - who->GetSelf()->GetAttribute(AttrAI_ChanceToNotTargetSwitch).get_float();
    else
        m_switchTargChance = 0.0f;
}

void NPCAIMgr::Process() {
    if ((!m_processTimer.Check())
        or (!m_npc->SysBubble()->HasPlayers())
        or m_npc->DestinyMgr()->IsWarping())
        return;

    if (m_shieldBoosterTimer.Enabled()
        and m_shieldBoosterTimer.Check())
        if (MakeRandomInt() < m_shieldBoosterChance)
            m_npc->UseShieldRecharge();

    if (m_armorRepairTimer.Enabled()
        and m_armorRepairTimer.Check())
        if (MakeRandomFloat() < m_armorRepairChance)
            m_npc->UseArmorRepairer();

    /* NPC::State definitions   -allan 25July15  (UD 1June16)
     *   Idle,       // not doing anything, nothing in sight....idle.  call Wander() to loosely orbit random object in bubble ~10-20k at 1/2 orbit speed
     *   Chasing,    // target within npc sight range.  attacking begins here.  use AttrMaxVelocity to get within falloff
     *   Following,  // between optimal and falloff.  try to get closer, but still orbiting and attacking
     *   Engaged,    // actively fighting (in orbit).  use m_orbitSpeed.
     *   Fleeing,    // running away....use AttrMaxVelocity then warp away when out of range	(does this make sense??)
     *   Signaling   // calling for help..use m_orbitSpeed *2 to speed tank while calling for reinforcements
     */
    switch(m_state) {
        case Idle: {
            // The parameter proximityRange (154) tells us how far we "see" (npc's dont have this, but drones do)
            if (m_beginFindTarget.Check()) {
                std::vector<Client*> clientVec;
                clientVec.clear();
                DestinyManager* pDestiny(nullptr);
                m_npc->SysBubble()->GetPlayers(clientVec); // what about player drones?  yes...later
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
                    if (m_npc->GetPosition().distance(cur->GetShipSE()->GetPosition()) > m_sightRange)
                        continue;

                    Target(cur->GetShipSE());
                    return;
                }
                if (sConfig.npc.IdleWander)
                    if (!m_isWandering)
                        Wander();
            } else {
                if (!m_beginFindTarget.Enabled())
                    m_beginFindTarget.Start(m_ROF);  //find target is based on npc attack speed.
            }
        } break;

        case Chasing:
        case Following:
        case Engaged: {
            if (m_npc->TargetMgr()->HasNoTargets()) {
                _log(NPC__AI_TRACE, "%s(%u): Stopped %s, HasNoTargets = true.", m_npc->GetName(), m_npc->GetID(), GetStateName(m_state).c_str());
                EnterIdle();
                return;
            }
            SystemEntity* pTarget = m_npc->TargetMgr()->GetFirstTarget(false);
            if (!pTarget) {
                _log(NPC__AI_TRACE, "%s(%u): Stopped %s, GetFirstTarget() returned NULL.", m_npc->GetName(), m_npc->GetID(), GetStateName(m_state).c_str());
                EnterIdle();
                return;
            } else if (!pTarget->SysBubble()) {
                m_npc->TargetMgr()->ClearTarget(pTarget);
                return;
            }
            _CheckDistance(pTarget);
        } break;

        case Fleeing:
        case Signaling: {
            _log(NPC__AI_TRACE, "%s(%u): Called %s, needs to be completed.", m_npc->GetName(), m_npc->GetID(), GetStateName(m_state).c_str());
            // not sure how im gonna do these
        } break;
    }
}

void NPCAIMgr::Wander()
{
    if (!m_isWandering) {
        _log(NPC__AI_TRACE, "%s(%u): Wandering.  No Targets within my sight range of %um", \
                m_npc->GetName(), m_npc->GetID(), m_sightRange);
        m_isWandering = true;
    }
    // wandering.  nothing to shoot.  look for target.
    if (m_npc->SysBubble()->HasDynamics()) {
        SystemEntity* pTarget = m_npc->SysBubble()->GetRandomEntity();
        if (!pTarget)
            pTarget = m_npc->SystemMgr()->GetSE(sBubbleMgr.GetBeltID(m_npc->SysBubble()->GetID()));
        if (!pTarget) {
            _log(NPC__ERROR, "%s(%u): Wandering.  No Target or beltSE found", m_npc->GetName(), m_npc->GetID());
            return;
        }
        // pick random entity and loosely orbit it.  if no entity found, orbit center of belt
        m_npc->DestinyMgr()->SetMaxVelocity(m_orbitSpeed);
        uint16 orbitDistance = MakeRandomInt(10000, 20000);
        m_npc->DestinyMgr()->Orbit(pTarget, orbitDistance);
        _log(NPC__AI_TRACE, "%s(%u):  Just for shits-n-giggles, I\'m gonna orbit %s(%u) at %um.", \
                m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID(), orbitDistance);
    } else {
        /** @todo  figure out a way for npc to wander 'aimlessly' around their bubble */
        m_npc->DestinyMgr()->Halt();
    }
}

void NPCAIMgr::EnterIdle() {
    if (m_state == Idle) return;
    // not doing anything....idle.
    _log(NPC__AI_TRACE, "%s(%u): _EnterIdle: returning to idle.", \
         m_npc->GetName(), m_npc->GetID());
    m_state = Idle;
    m_npc->DestinyMgr()->Stop();
    m_npc->DestinyMgr()->SetMaxVelocity(m_orbitSpeed);

    m_webifierTimer.Disable();
    m_beginFindTarget.Disable();
    m_mainAttackTimer.Disable();
    m_armorRepairTimer.Disable();
    m_warpScramblerTimer.Disable();
    m_shieldBoosterTimer.Disable();

    // write code to enable npcs to wander around when idle?
    // sounds like a good idea, but will take process power away from other shit.
}

void NPCAIMgr::EnterChasing(SystemEntity* pTarget) {
    /** @todo implement chase timer using entityChaseMaxDuration to limit chase time. */
    if ((m_state == Chasing) and (m_npc->DestinyMgr()->IsGoto() or m_npc->DestinyMgr()->IsFollowing()))
        return;
    _log(NPC__AI_TRACE, "%s(%u): _EnterChasing: Begin chasing.  Target is %s(%u).", \
         m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
    // target out of range to attack/follow, but within npc sight range....use mwd/ab if equiped
    m_npc->DestinyMgr()->SetMaxVelocity(m_npc->GetSelf()->GetAttribute(AttrMaxVelocity).get_int());
    m_npc->DestinyMgr()->GotoPoint(pTarget->GetPosition());  //head towards target
    m_state = Chasing;
}

void NPCAIMgr::EnterFollowing(SystemEntity* pTarget) {
    if ((m_state == Following) and (m_npc->DestinyMgr()->IsGoto() or m_npc->DestinyMgr()->IsFollowing()))
        return;
    _log(NPC__AI_TRACE, "%s(%u): _EnterFollowing: Begin following.  Target is %s(%u).", \
         m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
    // too close to chase, but to far to engage
    m_npc->DestinyMgr()->SetMaxVelocity(m_orbitSpeed *2);
    m_npc->DestinyMgr()->Follow(pTarget, m_falloff);  //try to get inside falloff range
    m_state = Following;
}

void NPCAIMgr::EnterEngaged(SystemEntity* pTarget) {
    if ((m_state == Engaged) and m_npc->DestinyMgr()->IsOrbiting())
        return;
    _log(NPC__AI_TRACE, "%s(%u): _EnterEngaged: Begin engaging.  Target is %s(%u).", \
         m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
    // actively fighting
    m_npc->DestinyMgr()->SetMaxVelocity(m_orbitSpeed);
    m_npc->DestinyMgr()->Orbit(pTarget, m_optimalRange);  //try to get inside orbit range
    m_state = Engaged;
}

void NPCAIMgr::EnterFleeing(SystemEntity* pTarget) {
    if ((m_state == Fleeing) and m_npc->DestinyMgr()->IsMoving())
        return;
    _log(NPC__AI_TRACE, "%s(%u): _EnterFleeing: Begin fleeing.  Target is %s(%u).", \
         m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
    // actively fleeing
    //  use superspeed to disengage, then warp.  << both these will need to be written.
    //  this state is only usable by higher-class npcs.
    m_npc->DestinyMgr()->SetMaxVelocity(m_npc->GetSelf()->GetAttribute(AttrMaxVelocity).get_int());
    m_state = Fleeing;
}

void NPCAIMgr::EnterSignaling(SystemEntity* pTarget) {
    if ((m_state == Signaling) and m_npc->DestinyMgr()->IsOrbiting())
        return;
    _log(NPC__AI_TRACE, "%s(%u): _EnterSignaling: Begin signaling.  Target is %s(%u).", \
         m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
    // actively signaling
    //  start speedtanking while signaling.  (im sure this is cheating, but fuckem.)
    //  this state is only usable by higher-class npcs.
    m_npc->DestinyMgr()->SetMaxVelocity(m_orbitSpeed * 2);
    m_npc->DestinyMgr()->Orbit(pTarget, m_falloff);  //try to get outside orbit range
    m_state = Signaling;
}

void NPCAIMgr::_CheckDistance(SystemEntity* pSE)
{
    double dist = m_npc->GetPosition().distance(pSE->GetPosition());
    if ((dist > m_sightRange) and (!m_npc->TargetMgr()->IsTargetedBy(pSE))) {
        _log(NPC__AI_TRACE, "%s(%u): _CheckDistance: %s(%u) is too far away (%u).  Return to Idle.", \
             m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID(), dist);
        if (m_state != Idle) {
            // target is no longer in npc's "sight range" and is NOT targeting this npc.  unlock target and return to idle.
            //   should we do anything else here?  search for another target?  wander around?  yes..later
            // if npc is targeted greater than this distance, it will chase
            m_npc->TargetMgr()->ClearTarget(pSE);
            if (m_npc->TargetMgr()->HasNoTargets())
                EnterIdle();
        }
        return;
    }

    m_isWandering = false;

    if (dist < m_flyRange)
        EnterEngaged(pSE);
    else if (dist < m_boostRange)
        EnterFollowing(pSE);
    else
        EnterChasing(pSE);

    if (m_shieldBoosterDuration && (!m_shieldBoosterTimer.Enabled()))
        m_shieldBoosterTimer.Start(m_shieldBoosterDuration);
    if (m_armorRepairDuration && (!m_armorRepairTimer.Enabled()))
        m_armorRepairTimer.Start(m_armorRepairDuration);
    if (!m_mainAttackTimer.Enabled())
        m_mainAttackTimer.Start(m_ROF);

    Attack(pSE);
}

void NPCAIMgr::ClearTargets() {
    m_npc->TargetMgr()->ClearTargets();
}

void NPCAIMgr::ClearAllTargets() {
    m_npc->TargetMgr()->ClearAllTargets();
}

void NPCAIMgr::Target(SystemEntity* pTarget) {
    double targetTime = GetTargetTime();
    bool chase = false;

	if (!m_npc->TargetMgr()->StartTargeting(pTarget, targetTime, (uint8)m_npc->GetSelf()->GetAttribute(AttrMaxAttackTargets).get_int(), m_sightRange, chase)) {
        if (chase) {
            _log(NPC__AI_TRACE, "%s(%u): Targeting of %s(%u) failed.  Begin Chasing.", \
                        m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
            EnterChasing(pTarget);
        } else {
            _log(NPC__AI_TRACE, "%s(%u): Targeting of %s(%u) failed.  Clear Target and Return to Idle.", \
                        m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
            EnterIdle();
        }
        return;
    }
    m_beginFindTarget.Disable();
    _CheckDistance(pTarget);
}

void NPCAIMgr::Targeted(SystemEntity* pAgressor) {
    double targetTime = GetTargetTime();

    switch(m_state) {
        case Idle: {
            _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) in Idle. Begin Approaching and start Targeting sequence.", \
                    m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());
            EnterChasing(pAgressor);

            bool chase = false;
			if (!m_npc->TargetMgr()->StartTargeting( pAgressor, targetTime, (uint8)m_npc->GetSelf()->GetAttribute(AttrMaxAttackTargets).get_int(), m_sightRange, chase)) {
                if (chase) {
                    _log(NPC__AI_TRACE, "%s(%u): Targeting of %s(%u) failed.  Begin Chasing.", \
                            m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());
                    EnterChasing(pAgressor);
                } else {
                    _log(NPC__AI_TRACE, "%s(%u): Targeting of %s(%u) failed.  Clear Target and Return to Idle.", \
                            m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());
                    EnterIdle();
                }
            }
            m_beginFindTarget.Disable();
            //_CheckDistance(pAgressor);
        } break;

        /** @todo  determine if new targetedby entity is weaker than current target. use optimalSigRadius to test for 'optimal' target */
        case Chasing: {
            _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) while chasing.", \
                    m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case Following: {
            _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) while following.", \
                    m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case Engaged: {
            _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) while engaged.", \
                    m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case Fleeing: {
            _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) while fleeing.", \
                    m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case Signaling: {
            _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) while signaling.", \
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
            if (m_npc->TargetMgr()->HasNoTargets()) {
                _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) lost. No targets remain.  Return to Idle.", \
                        m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
                EnterIdle();
            } else {
                _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) lost, but more targets remain.", \
                        m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
                /** @todo engage weakest target in current list */
            }

        } break;
    }
}

void NPCAIMgr::Attack(SystemEntity* pTarget)
{
    if (m_mainAttackTimer.Check()) {
        if (!pTarget) return;
        // Check to see if the target still in the bubble (Client warped out)
        if (!m_npc->SysBubble()->InBubble(pTarget->GetPosition())) {
            _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) no longer in bubble.  Clear target and move on",
                    m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
            m_npc->TargetMgr()->ClearTarget(pTarget);
            return;
        }
        if (!pTarget->DestinyMgr()) {
            _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) has no destiny manager.  Clear target and move on",
                    m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
            m_npc->TargetMgr()->ClearTarget(pTarget);
            return;
        }
        // Check to see if the target is not cloaked:
        if (pTarget->DestinyMgr()->IsCloaked()) {
            _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) is cloaked.  Clear target and move on",
                    m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
            m_npc->TargetMgr()->ClearTarget(pTarget);
            return;
        }
        if (m_npc->TargetMgr()->CanAttack())
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
             m_npc->GetSelf(),
             m_npc->GetKinetic(),
             m_npc->GetThermal(),
             m_npc->GetEM(),
             m_npc->GetExplosive(),
             m_formula.GetNPCToHit(m_npc, pTarget),
             EVEEffectID::targetAttack
            );

    d *= m_damageMultiplier;
    pTarget->ApplyDamage(d);
}

//NOTE: duplicated from module manager code. They should share some day!
void NPCAIMgr::_SendWeaponEffect( const char* effect, SystemEntity* pTarget ) {
    DoDestiny_OnSpecialFX13 sfx;
        sfx.entityID = m_npc->GetSelf()->itemID();
        sfx.moduleID = m_npc->GetSelf()->itemID();
        sfx.moduleTypeID = m_npc->GetSelf()->GetAttribute(AttrGfxTurretID).get_int();
        sfx.targetID = pTarget->GetID();
        sfx.otherTypeID = pTarget->GetSelf()->typeID();
        sfx.guid = effect;
        sfx.isOffensive = 1;
        sfx.start = 1;
        sfx.active = 1;
        sfx.duration_ms = m_ROF;
        sfx.repeat = 1;
        sfx.startTime = Win32TimeNow();
    PyTuple* up = sfx.Encode();
    m_npc->DestinyMgr()->SendSingleDestinyUpdate( &up );    //consumed
}

double NPCAIMgr::GetTargetTime()
{
    double targetTime = (m_npc->GetSelf()->GetAttribute(AttrScanSpeed).get_int());
    if (!targetTime) {
        if (m_npc->GetSelf()->GetAttribute(AttrRadius) < 30)
            targetTime = 1500;
        else if (m_npc->GetSelf()->GetAttribute(AttrRadius) < 60)
            targetTime = 2500;
        else if (m_npc->GetSelf()->GetAttribute(AttrRadius) < 150)
            targetTime = 4000;
        else if (m_npc->GetSelf()->GetAttribute(AttrRadius) < 280)
            targetTime = 6000;
        else if (m_npc->GetSelf()->GetAttribute(AttrRadius) < 550)
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

std::string NPCAIMgr::GetStateName(NPCAIMgr::State name)
{
    switch (name) {
        case Idle:      return "Idle";
        case Chasing:   return "Chasing";
        case Following: return "Following";
        case Engaged:   return "Engaged";
        case Fleeing:   return "Fleeing";
        case Signaling: return "Signaling";
    }
}
