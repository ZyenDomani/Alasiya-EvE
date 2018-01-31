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
    AI Version: 0.45
*/

/** @todo  ai update ideas
 *   bubble call *SomeFunction* to tell ai of new ship arriving in bubble
 *   method to use npc's prefered sig radius for targets
 *   finish flee and signal action methods (and determine who can use them and when)
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
: m_state(NPCState::Idle),
  m_npc(who),
  m_warpOutTimer(1000),
  m_mainAttackTimer(1000),
  m_processTimer(5000),         //arbitrary.
  m_shieldBoosterTimer(10000),  //arbitrary.
  m_armorRepairTimer(8000),     //arbitrary.
  m_beginFindTarget(5000),      //arbitrary.
  m_warpScramblerTimer(5000),   //arbitrary.
  m_webifierTimer(5000)         //arbitrary.
{
    m_warpOutTimer.Disable();
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

    /* set npc ship data */
    m_sigResolution = who->GetSelf()->GetAttribute(AttrOptimalSigRadius).get_int();
    m_attackSpeed = who->GetSelf()->GetAttribute(AttrSpeed).get_int();
    m_sigRadius = who->GetSelf()->GetAttribute(AttrSignatureRadius).get_int();

    /** @todo  all of these need to be verified and/or updated */

    // ship speeds
    // absolute (boosted) Max Ship Speed
    m_maxSpeed = who->GetSelf()->GetAttribute(AttrMaxVelocity).get_int();
    // Orbit Velocity
    m_orbitSpeed = who->GetSelf()->GetAttribute(AttrEntityCruiseSpeed).get_int();

    // ship distances
    // Optimal Range
    m_optimalRange = who->GetSelf()->GetAttribute(AttrMaxRange).get_int();
    // Accuracy falloff  (distance past optimal range at which accuracy has fallen by half)
    m_falloff = who->GetSelf()->GetAttribute(AttrFalloff).get_int();
    m_trackingSpeed = who->GetSelf()->GetAttribute(AttrTrackingSpeed).get_float();
    // Orbit Range, Follow Range  - npc tries to stay at this distance from active target
    m_flyRange = who->GetSelf()->GetAttribute(AttrEntityFlyRange).get_int();
    if (!m_flyRange)
        m_flyRange = 0;
    // distance for Speed Boost activation
    m_boostRange = who->GetSelf()->GetAttribute(AttrEntityChaseMaxDistance).get_int();
    if (!m_boostRange)
        m_boostRange = 0;
    // max firing range   default:10000
    m_maxAttackRange = who->GetSelf()->GetAttribute(AttrEntityAttackRange).get_int();
    if (!m_maxAttackRange)
        m_maxAttackRange = 10000;
    // 'sight' range
    m_sightRange = 20000;
    if (m_maxAttackRange > m_sightRange)
        m_sightRange = m_maxAttackRange *2;

    // these next two have effects that define the rep/boost chance attribID
    if (who->GetSelf()->GetAttribute(AttrEntityArmorRepairDelayChanceSmall).get_int())
        m_armorRepairChance = who->GetSelf()->GetAttribute(AttrEntityArmorRepairDelayChanceSmall).get_int();
    else if (who->GetSelf()->GetAttribute(AttrEntityArmorRepairDelayChanceMedium).get_int())
        m_armorRepairChance = who->GetSelf()->GetAttribute(AttrEntityArmorRepairDelayChanceMedium).get_int();
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
    else if (who->GetSelf()->HasAttribute(AttrEntityShieldBoostDelayChanceMedium))
        m_shieldBoosterChance = who->GetSelf()->GetAttribute(AttrEntityShieldBoostDelayChanceMedium).get_float();
    else if (who->GetSelf()->HasAttribute(AttrEntityShieldBoostDelayChanceLarge))
        m_shieldBoosterChance = who->GetSelf()->GetAttribute(AttrEntityShieldBoostDelayChanceLarge).get_float();
    else
        m_shieldBoosterChance = 0;
    if (m_shieldBoosterChance)
        m_shieldBoosterDuration = who->GetSelf()->GetAttribute(AttrEntityShieldBoostDuration).get_int();
    else
        m_shieldBoosterDuration = 0;

    // advanced AI variables  only used by sleepers for now (and on live).  will update advanced npcs to use these also (unique to alasiya)
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
        m_switchTargChance = 0;

    if (who->GetSelf()->HasAttribute(AttrWarpScrambleRange))
        m_warpScramRange = who->GetSelf()->GetAttribute(AttrWarpScrambleRange).get_float();
    else
        m_warpScramRange = 0;
    if (who->GetSelf()->HasAttribute(AttrEntityWarpScrambleChance))
        m_warpScramChance = 1.0 - who->GetSelf()->GetAttribute(AttrEntityWarpScrambleChance).get_float();
    else
        m_warpScramChance = 0;

    /*
    AttrWarpScrambleRange = 103,
    AttrWarpScrambleStrength = 105,
    AttrEntityWarpScrambleChance = 504,
    AttrWarpScrambleDuration = 505,
    */

    // does this need to be running if there are no players in bubble?
    //  yes...npcs will (eventually) warp out when no targets in sight range, but need a process tic to do that.
    m_processTimer.Start(m_attackSpeed);
}

void NPCAIMgr::Process() {
    if ((!m_processTimer.Check())
    or m_npc->DestinyMgr()->IsWarping())
        return;
    if (!m_npc->SysBubble()->HasPlayers())  // this needs to be separate as bubble = null when warping
        return;

    if (m_shieldBoosterTimer.Enabled()
    and m_shieldBoosterTimer.Check())
        if (MakeRandomFloat() < m_shieldBoosterChance)
            m_npc->UseShieldRecharge();

    if (m_armorRepairTimer.Enabled()
    and m_armorRepairTimer.Check())
        if (MakeRandomFloat() < m_armorRepairChance)
            m_npc->UseArmorRepairer();

    if (m_warpOutTimer.Check(false)) {
        // disallow warpout if spawn has active respawn timer (spawn is being chained)
        if (m_npc->GetSpawnMgr()->IsChaining(m_npc->SysBubble()->GetID())) {
            m_warpOutTimer.Disable();
            return;
        }
        WarpOut();
        return;
    }
    /* NPCState definitions   -allan 25July15  (UD 1June16)
     *   Idle,       // not doing anything, nothing in sight....idle.  call Wander() to loosely orbit random object in bubble ~10-20k at 1/2 orbit speed
     *   Chasing,    // target within npc sight range.  attacking begins here.  use m_maxSpeed to get within falloff
     *   Following,  // between optimal and falloff.  try to get closer, but still orbiting and attacking
     *   Engaged,    // actively fighting (in orbit).  use m_orbitSpeed.
     *   Fleeing,    // running away....use m_maxSpeed then warp away when out of range	(does this make sense??)
     *   Signaling   // calling for help..use m_orbitSpeed *2 to speed tank while calling for reinforcements
     */
    switch(m_state) {
        case NPCState::Idle: {
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
                        SetWander();
            } else {
                if (!m_beginFindTarget.Enabled())
                    m_beginFindTarget.Start(m_attackSpeed);  //find target is based on npc attack speed.
            }
        } break;
        case NPCState::Chasing:
        case NPCState::Following:
        case NPCState::Engaged: {
            if (m_npc->TargetMgr()->HasNoTargets()) {
                _log(NPC__AI_TRACE, "%s(%u): Stopped %s, HasNoTargets = true.", m_npc->GetName(), m_npc->GetID(), GetStateName(m_state).c_str());
                SetIdle();
                return;
            }
            SystemEntity* pSE = m_npc->TargetMgr()->GetFirstTarget(false);
            if (pSE == nullptr) {
                _log(NPC__AI_TRACE, "%s(%u): Stopped %s, GetFirstTarget() returned NULL.", m_npc->GetName(), m_npc->GetID(), GetStateName(m_state).c_str());
                SetIdle();
                return;
            } else if (pSE->SysBubble() == nullptr) {
                m_npc->TargetMgr()->ClearTarget(pSE);
                return;
            }
            CheckDistance(pSE);
        } break;
        case NPCState::WarpOut:
        case NPCState::WarpFollow: {
            ; // do nothing yet
        } break;
        case NPCState::Fleeing:
        case NPCState::Signaling:{
            _log(NPC__AI_TRACE, "%s(%u): Called %s, needs to be completed.", m_npc->GetName(), m_npc->GetID(), GetStateName(m_state).c_str());
            // not sure how im gonna do these
        } break;
    }
}

void NPCAIMgr::WarpOut()
{
    if (m_state == NPCState::WarpOut)
        return;

    m_state = NPCState::WarpOut;
    m_warpOutTimer.Disable();

    /** @todo  eventually, this will check with anomaly mgr for possible npc hideouts in system
     * based on npc faction, system, players in system, players in bubble, and *more later*
     * to determine a warpto target for this npc, or this group
     *
     * for now, if there are players in system, just warp to another belt.
     * note:  must set/reset belt spawn variable if/when spawns jump belts
     */

    /*
    m_npc->SystemMgr()->GetAnomMgr();
    m_npc->SysBubble()->SetSpawned();
    */

    // if there are no players in this system, avoid using proc tics on npcs
    if (m_npc->SystemMgr()->PlayerCount()) {
        //  determine if player is warping INTO this bubble, and if so, deny spawn warping out.  (maybe?)
        uint32 newBeltID = m_npc->SystemMgr()->GetRandBeltID();
        if (newBeltID == sBubbleMgr.GetBeltID(m_npc->SysBubble()->GetID()))
            newBeltID = m_npc->SystemMgr()->GetRandBeltID();

        SystemEntity* newSE = m_npc->SystemMgr()->GetSE(newBeltID);
       // m_npc->DestinyMgr()->WarpTo(newSE->GetPosition());
        m_npc->GetSpawnMgr()->WarpOutSpawn(m_npc, sBubbleMgr.FindBubble(newSE));
    }
}

void NPCAIMgr::SetWander()
{
    if (!m_isWandering) {
        _log(NPC__AI_TRACE, "%s(%u): Wandering:  No Targets within my sight range of %um", \
                m_npc->GetName(), m_npc->GetID(), m_sightRange);
        m_isWandering = true;
    }
    // wandering.  nothing to shoot.  look for target.
    if (m_npc->SysBubble()->HasDynamics()) {
        SystemEntity* pSE = m_npc->SysBubble()->GetRandomEntity();
        if (pSE == nullptr)
            pSE = m_npc->SystemMgr()->GetSE(sBubbleMgr.GetBeltID(m_npc->SysBubble()->GetID()));
        if (pSE == nullptr) {
            _log(NPC__ERROR, "%s(%u): Wandering:  No Target or beltSE found", m_npc->GetName(), m_npc->GetID());
            return;
        }
        // pick random entity and loosely orbit it.  if no entity found, orbit center of belt
        m_npc->DestinyMgr()->SetMaxVelocity(m_orbitSpeed);
        uint16 orbitDistance = MakeRandomInt(10000, 20000);
        m_npc->DestinyMgr()->Orbit(pSE, orbitDistance);
        _log(NPC__AI_TRACE, "%s(%u):  Just for shits-n-giggles, I\'m gonna orbit %s(%u) at %um.", \
                m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID(), orbitDistance);
    } else {
        /** @todo  figure out a way for npc to wander 'aimlessly' around their bubble */
        m_npc->DestinyMgr()->Stop();
    }
}

void NPCAIMgr::SetIdle() {
    if (m_state == NPCState::Idle)
        return;
    // not doing anything....idle.
    _log(NPC__AI_TRACE, "%s(%u): Idle: returning to idle.", \
         m_npc->GetName(), m_npc->GetID());
    m_state = NPCState::Idle;
    m_npc->DestinyMgr()->Stop();
    m_npc->DestinyMgr()->SetMaxVelocity(m_orbitSpeed);

    m_webifierTimer.Disable();
    m_beginFindTarget.Disable();
    m_mainAttackTimer.Disable();
    m_armorRepairTimer.Disable();
    m_warpScramblerTimer.Disable();
    m_shieldBoosterTimer.Disable();

    if (sConfig.npc.WarpOut > 1)
        m_warpOutTimer.Start(sConfig.npc.WarpOut *1000); // s to ms
}

void NPCAIMgr::SetChasing(SystemEntity* pSE) {
    if (pSE == nullptr)
        return;
    /** @todo implement chase timer using entityChaseMaxDuration to limit chase time. */
    if ((m_state == NPCState::Chasing) and (m_npc->DestinyMgr()->IsGoto() or m_npc->DestinyMgr()->IsFollowing()))
        return;
    _log(NPC__AI_TRACE, "%s(%u): Chasing: Begin chasing.  Target is %s(%u).", \
         m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
    // target out of range to attack/follow, but within npc sight range....use mwd/ab if equiped
    m_npc->DestinyMgr()->SetMaxVelocity(m_maxSpeed);
    m_npc->DestinyMgr()->GotoPoint(pSE->GetPosition());  //head towards target
    m_state = NPCState::Chasing;
    m_warpOutTimer.Disable();
}

void NPCAIMgr::SetFollowing(SystemEntity* pSE) {
    if (pSE == nullptr)
        return;
    if ((m_state == NPCState::Following) and (m_npc->DestinyMgr()->IsGoto() or m_npc->DestinyMgr()->IsFollowing()))
        return;
    _log(NPC__AI_TRACE, "%s(%u): Following: Begin following.  Target is %s(%u).", \
         m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
    // too close to chase, but to far to engage
    m_npc->DestinyMgr()->SetMaxVelocity(m_orbitSpeed *2);
    m_npc->DestinyMgr()->Follow(pSE, m_falloff);  //try to get inside falloff range
    m_state = NPCState::Following;
    m_warpOutTimer.Disable();
}

void NPCAIMgr::SetEngaged(SystemEntity* pSE) {
    if (pSE == nullptr)
        return;
    if ((m_state == NPCState::Engaged) and m_npc->DestinyMgr()->IsOrbiting())
        return;
    _log(NPC__AI_TRACE, "%s(%u): Engaged: Begin engaging.  Target is %s(%u).", \
         m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
    // actively fighting
    m_npc->DestinyMgr()->SetMaxVelocity(m_orbitSpeed);
    m_npc->DestinyMgr()->Orbit(pSE, m_optimalRange);  //try to get inside orbit range
    m_state = NPCState::Engaged;
    m_warpOutTimer.Disable();
}

void NPCAIMgr::SetFleeing(SystemEntity* pSE) {
    if (pSE == nullptr)
        return;
    if ((m_state == NPCState::Fleeing) and m_npc->DestinyMgr()->IsMoving())
        return;
    _log(NPC__AI_TRACE, "%s(%u): Fleeing: Begin fleeing.  Target is %s(%u).", \
         m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
    // actively fleeing
    //  use superspeed to disengage, then warp.  << both these will need to be written.
    //  this state is only usable by higher-class npcs.
    m_npc->DestinyMgr()->SetMaxVelocity(m_maxSpeed);
    m_state = NPCState::Fleeing;
    m_warpOutTimer.Disable();
}

void NPCAIMgr::SetSignaling(SystemEntity* pSE) {
    if (pSE == nullptr)
        return;
    if ((m_state == NPCState::Signaling) and m_npc->DestinyMgr()->IsOrbiting())
        return;
    _log(NPC__AI_TRACE, "%s(%u): Signaling: Begin signaling.  Target is %s(%u).", \
         m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
    // actively signaling
    //  start speedtanking while signaling.  (im sure this is cheating, but fuckem.)
    //  this state is only usable by higher-class npcs.
    m_npc->DestinyMgr()->SetMaxVelocity(m_orbitSpeed * 2);
    m_npc->DestinyMgr()->Orbit(pSE, m_falloff);  //try to get outside orbit range
    m_state = NPCState::Signaling;
    m_warpOutTimer.Disable();
}

void NPCAIMgr::CheckDistance(SystemEntity* pSE)
{
    if (pSE == nullptr)
        return;
    double dist = m_npc->GetPosition().distance(pSE->GetPosition());
    if ((dist > m_sightRange) and (!m_npc->TargetMgr()->IsTargetedBy(pSE))) {
        _log(NPC__AI_TRACE, "%s(%u): CheckDistance: %s(%u) is too far away (%u).  Return to Idle.", \
             m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID(), dist);
        if (m_state != NPCState::Idle) {
            // target is no longer in npc's "sight range" and is NOT targeting this npc.  unlock target and return to idle.
            //   should we do anything else here?  search for another target?  wander around?  yes..later
            // if npc is targeted greater than this distance, it will chase
            m_npc->TargetMgr()->ClearTarget(pSE);
            if (m_npc->TargetMgr()->HasNoTargets())
                SetIdle();
        }
        return;
    }

    m_isWandering = false;

    if (dist < m_flyRange)
        SetEngaged(pSE);
    else if (dist < m_boostRange)
        SetFollowing(pSE);
    else
        SetChasing(pSE);

    if (m_shieldBoosterDuration && (!m_shieldBoosterTimer.Enabled()))
        m_shieldBoosterTimer.Start(m_shieldBoosterDuration);
    if (m_armorRepairDuration && (!m_armorRepairTimer.Enabled()))
        m_armorRepairTimer.Start(m_armorRepairDuration);
    if (!m_mainAttackTimer.Enabled())
        m_mainAttackTimer.Start(m_attackSpeed);

    Attack(pSE);
}

void NPCAIMgr::ClearTargets() {
    m_npc->TargetMgr()->ClearTargets();
}

void NPCAIMgr::ClearAllTargets() {
    m_npc->TargetMgr()->ClearAllTargets();
}

void NPCAIMgr::Target(SystemEntity* pSE) {
    if (pSE == nullptr)
        return;
    double targetTime = GetTargetTime();
    bool chase = false;

	if (!m_npc->TargetMgr()->StartTargeting(pSE, targetTime, (uint8)m_npc->GetSelf()->GetAttribute(AttrMaxAttackTargets).get_int(), m_sightRange, chase)) {
        if (chase) {
            _log(NPC__AI_TRACE, "%s(%u): Targeting of %s(%u) failed.  Begin Chasing.", \
                        m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
            SetChasing(pSE);
        } else {
            _log(NPC__AI_TRACE, "%s(%u): Targeting of %s(%u) failed.  Clear Target and Return to Idle.", \
                        m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
            SetIdle();
        }
        return;
    }
    m_beginFindTarget.Disable();
    CheckDistance(pSE);
}

void NPCAIMgr::Targeted(SystemEntity* pSE) {
    if (pSE == nullptr)
        return;
    double targetTime = GetTargetTime();

    switch(m_state) {
        case NPCState::Idle: {
            _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) in Idle. Begin Approaching and start Targeting sequence.", \
                    m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
            SetChasing(pSE);

            bool chase = false;
			if (!m_npc->TargetMgr()->StartTargeting( pSE, targetTime, (uint8)m_npc->GetSelf()->GetAttribute(AttrMaxAttackTargets).get_int(), m_sightRange, chase)) {
                if (chase) {
                    _log(NPC__AI_TRACE, "%s(%u): Targeting of %s(%u) failed.  Begin Chasing.", \
                            m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
                    SetChasing(pSE);
                } else {
                    _log(NPC__AI_TRACE, "%s(%u): Targeting of %s(%u) failed.  Clear Target and Return to Idle.", \
                            m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
                    SetIdle();
                }
            }
            m_beginFindTarget.Disable();
            //CheckDistance(pAgressor);
        } break;

        /** @todo  determine if new targetedby entity is weaker than current target. use optimalSigRadius to test for 'optimal' target */
        case NPCState::Chasing: {
            _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) while chasing.", \
                    m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
        } break;
        case NPCState::Following: {
            _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) while following.", \
                    m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
        } break;
        case NPCState::Engaged: {
            _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) while engaged.", \
                    m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
        } break;
        case NPCState::Fleeing: {
            _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) while fleeing.", \
                    m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
        } break;
        case NPCState::Signaling: {
            _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) while signaling.", \
                    m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
        } break;
    }
}

void NPCAIMgr::TargetLost(SystemEntity* pSE) {
    if (pSE == nullptr)
        return;
    switch(m_state) {
        case NPCState::Chasing:
        case NPCState::Following:
        case NPCState::Engaged: {
            if (m_npc->TargetMgr()->HasNoTargets()) {
                _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) lost. No targets remain.  Return to Idle.", \
                        m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
                SetIdle();
            } else {
                _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) lost, but more targets remain.", \
                        m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
                /** @todo engage weakest target in current list */
            }
        } break;
    }
}

void NPCAIMgr::Attack(SystemEntity* pSE)
{
    if (pSE == nullptr)
        return;
    if (m_mainAttackTimer.Check()) {
        if (pSE == nullptr)
            return;
        // Check to see if the target still in the bubble (Client warped out)
        if (!m_npc->SysBubble()->InBubble(pSE->GetPosition())) {
            _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) no longer in bubble.  Clear target and move on",
                    m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
            m_npc->TargetMgr()->ClearTarget(pSE);
            return;
        }
        if (pSE->DestinyMgr() == nullptr) {
            _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) has no destiny manager.  Clear target and move on",
                    m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
            m_npc->TargetMgr()->ClearTarget(pSE);
            return;
        }
        // Check to see if the target is not cloaked:
        if (pSE->DestinyMgr()->IsCloaked()) {
            _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) is cloaked.  Clear target and move on",
                    m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
            m_npc->TargetMgr()->ClearTarget(pSE);
            return;
        }
        if (m_npc->TargetMgr()->CanAttack())
            AttackTarget(pSE);
    }
}

//also check for special effects and write code to implement them
//modifyTargetSpeedRange, modifyTargetSpeedChance
//entityWarpScrambleChance
void NPCAIMgr::AttackTarget(SystemEntity* pSE) {
    if (pSE == nullptr)
        return;
    // put checks here for point/tackle

    // some npcs use missiles.....write code for using missiles   -- entityMissileTypeID
    std::string guid = "effects.Laser";
    m_npc->DestinyMgr()->SendSpecialEffect(m_npc->GetSelf()->itemID(),
                                           m_npc->GetSelf()->itemID(),
                                           m_npc->GetSelf()->typeID(),
                                           pSE->GetID(),
                                           0,guid,1,1,1,m_attackSpeed,0
                                           //m_npc->GetSelf()->GetAttribute(AttrGfxTurretID).get_int()    // graphicID for turret for drone type ships
                                          );

    Damage d(m_npc,
             m_npc->GetSelf(),
             m_npc->GetKinetic(),
             m_npc->GetThermal(),
             m_npc->GetEM(),
             m_npc->GetExplosive(),
             m_formula.GetNPCToHit(m_npc, pSE),
             EVEEffectID::targetAttack
            );

    if (sConfig.npc.UseDamageMultiplier)
        d *= m_damageMultiplier;
    pSE->ApplyDamage(d);
}

double NPCAIMgr::GetTargetTime()
{
    double targetTime = (m_npc->GetSelf()->GetAttribute(AttrScanSpeed).get_int());
    float radius = m_npc->GetSelf()->GetAttribute(AttrRadius).get_float();
    if (targetTime < 1) {
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

void NPCAIMgr::DisableRepTimers()
{
    m_armorRepairTimer.Disable();
    m_shieldBoosterTimer.Disable();
}

std::string NPCAIMgr::GetStateName(int8 stateID)
{
    switch (stateID) {
        case NPCState::Idle:           return "Idle";
        case NPCState::Chasing:        return "Chasing";
        case NPCState::Engaged:        return "Engaged";
        case NPCState::Fleeing:        return "Fleeing";
        case NPCState::Following:      return "Following";
        case NPCState::Signaling:      return "Signaling";
        case NPCState::WarpOut:        return "Warping Out";
        case NPCState::WarpFollow:     return "Following Warp";
        default:                    return "Invalid";
    }
}
