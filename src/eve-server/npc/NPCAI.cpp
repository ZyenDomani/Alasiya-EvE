/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    For the latest information visit https://evemu.dev
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
    Rewrite:    Allan
    AI Version: 0.59
*/

/** @todo  ai update ideas
 *   bubble call *SomeFunction* to tell ai of new ship arriving in bubble
 *   method to use npc's preferred sig radius for targets
 *   finish flee and signal action methods (and determine who can use them and when)
 *      - this should take system sov, npc anomalies, destruction speed, and pirate faction
 *   add methods to check target/targeter warping out and chance of npc following (and possibly calling backup)
 *
 *   TODO:  use fake orbit like drones do
 *
 * NOTE:  elite npc target drones first!
 *   recall drones -> npc target player -> deploy drones -> timer countdown -> npc target drones again
 *
 *  have data...needs coding...
 *   chase duration/distance timers
 *   ewar shit, including point/tackle
 */

#include "eve-server.h"

#include "Client.h"
#include "inventory/AttributeEnum.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "ship/Missile.h"
#include "system/DestinyManager.h"
#include "system/Damage.h"
#include "system/SystemBubble.h"

NPCAIMgr::NPCAIMgr(NPC* mySE)
: m_state(NPCAI::State::Idle),
  myNPC(mySE),
  m_destiny(mySE->DestinyMgr()),
  m_self(mySE->GetSelf()),
  m_processTimer(0),
  m_mainAttackTimer(0),
  m_missileTimer(0),
  m_warpOutTimer(0),
  m_damageMultiplier(0),
  m_shieldBoosterTimer(0),
  m_armorRepairTimer(0),
  m_armorRepairDuration(0),
  m_beginFindTarget(0),
  m_warpScramblerTimer(0),
  m_targetRange(0),
  m_webifierTimer(0),
  m_missileTypeID(0),
  m_webber(false),
  m_warpScram(false),
  m_isWandering(false)
{
    assert(m_self.get() != nullptr);

    /* set npc ship data */
    m_sigResolution = m_self->GetAttribute(AttrOptimalSigRadius).get_uint32();
    m_attackSpeed = m_self->GetAttribute(AttrSpeed).get_uint32();
    m_sigRadius = m_self->GetAttribute(AttrSignatureRadius).get_uint32();
    m_launcherCycleTime = m_self->GetAttribute(AttrMissileLaunchDuration).get_uint32();
    if (m_launcherCycleTime > 100)
        m_missileTypeID = m_self->GetAttribute(AttrEntityMissileTypeID).get_uint32();

    //  AttrEntityDefenderChance = 497,  <<< for defender missiles

    m_damageMultiplier = m_self->GetAttribute(AttrDamageMultiplier).get_float();
    if (m_damageMultiplier < 0.1)
        m_damageMultiplier = 1.0f;

    /** @todo  all of these need to be verified and/or updated
     *   **copied from drone
    m_maxSpeed = (dRef->GetAttribute(AttrMaxVelocity).get_uint32());
    m_cycleTime = (dRef->GetAttribute(AttrSpeed).get_int());
    m_cruiseSpeed = (dRef->GetAttribute(AttrEntityCruiseSpeed).get_uint32());
    m_orbitDistance = dRef->GetAttribute(AttrOrbitRange).get_uint32();
    m_falloffDistance = dRef->GetAttribute(AttrEntityChaseMaxDistance).get_uint32();
    m_engageDistance = dRef->GetAttribute(AttrMaxRange).get_uint32();
    m_chaseDistance = dRef->GetAttribute(AttrFalloff).get_uint32();
    m_maxDistance = dRef->GetAttribute(AttrEntityAttackRange).get_uint32();
    */

    // ship speeds
    // absolute (boosted) Max Ship Speed
    m_maxSpeed = m_self->GetAttribute(AttrMaxVelocity).get_uint32();
    // Orbit Velocity
    m_orbitSpeed = m_self->GetAttribute(AttrEntityCruiseSpeed).get_uint32();   // ship speed when not chasing target
    // there are 2600 of each of the following defined in db
    //AttrEntityChaseMaxDelay  - time before 'chase speed' kicks in
    //AttrEntityChaseMaxDelayChance  - chance npc will wait AttrEntityChaseMaxDelay before chasing
    //AttrEntityChaseMaxDuration  - max time a chase will last (unless weapons fired)
    //AttrEntityChaseMaxDurationChance  - chance that any chase will last for AttrEntityChaseMaxDuration

    // ship distances
    //AttrEntityMaxWanderRange  -- none defined in db
    // Optimal Range  - TODO: test for 0
    m_optimalRange = m_self->GetAttribute(AttrMaxRange).get_uint32();  // distance which npc starts using weapons
    // Accuracy falloff  (distance past optimal range at which accuracy has fallen by half) - TODO: test for 0
    m_falloff = m_self->GetAttribute(AttrFalloff).get_uint32();
    m_trackingSpeed = m_self->GetAttribute(AttrTrackingSpeed).get_double();  //rad/sec
    // Orbit Range, Follow Range  - npc tries to stay at this distance from active target
    m_flyRange = m_self->GetAttribute(AttrEntityFlyRange).get_uint32();    //AttrOrbitRange is 0 for npc
    if (m_flyRange < 1) {
        if (m_optimalRange > 0) {
            m_flyRange = m_optimalRange;
        } else {
            _log(NPC__WARNING, "%s(typeID:%u):  OptimalRange = 0", myNPC->GetName(), myNPC->GetTypeID());
            m_flyRange = 500;
        }
    }
    // distance for Speed Boost activation  (this needs to be revisited)
    m_boostRange = m_self->GetAttribute(AttrEntityChaseMaxDistance).get_uint32();
    if (!m_boostRange)
        m_boostRange = 0;
    // some npcs have flyRange > boostRange.  this corrects it. (extends boost range)
    if (m_flyRange > m_boostRange)
        m_boostRange += m_boostRange + m_flyRange;
    // max firing range   default:10000  (lowest in db is 1000)
    m_maxAttackRange = m_self->GetAttribute(AttrEntityAttackRange).get_uint32();
    // this should be set according to npc size.
    if (m_maxAttackRange < 1000)
        m_maxAttackRange = 10000;

    // 'sight' range (undefined in db)
    float radius = m_self->GetAttribute(AttrRadius).get_float();
    if (radius < 30) {
        m_sightRange = 2500;
    } else if (radius < 60) {
        m_sightRange = 5000;
    } else if (radius < 150) {
        m_sightRange = 8000;
    } else if (radius < 280) {
        m_sightRange = 12000;
    } else if (radius < 550) {
        m_sightRange = 15000;
    } else {
        m_sightRange = 20000;
    }
    if (m_maxAttackRange > m_sightRange)
        m_sightRange = m_maxAttackRange * 2;

    // ship targets
    m_maxAttackTargets = m_self->GetAttribute(AttrMaxAttackTargets).get_uint32();
    if (m_maxAttackTargets < 1)
        m_maxAttackTargets = 1;
    m_maxLockedTargets = m_self->GetAttribute(AttrMaxLockedTargets).get_uint32();
    if (m_maxLockedTargets < 1) {
        if (m_maxAttackTargets > 1) {
            m_maxLockedTargets = m_maxAttackTargets;
        } else {
            m_maxLockedTargets = 1;
        }
    }


    /** @todo change these next 2 (rep and boost) to boolean to avoid timer creation/checks */
    m_armorRepairDuration = 0;
    m_armorRepairDelayChance = 0;
    m_shieldBoosterDuration = 0;
    m_shieldBoosterDelayChance = 0;
    m_useTargSwitching = false;
    m_useSecondTarget = false;
    m_useSigRadius = false;
    m_preferedSigRadius = 0;
    m_warpScramRange = 0;
    m_warpScramChance = 0;
    m_switchTargChance = 0;

    // this is chance an npc has of delaying it's rep (if applicable)
    if (m_self->HasAttribute(AttrEntityArmorRepairDelayChance)) {
        m_armorRepairDelayChance = m_self->GetAttribute(AttrEntityArmorRepairDelayChance).get_float();
    } else if (m_self->HasAttribute(AttrEntityArmorRepairDelayChanceSmall)) {
        m_armorRepairDelayChance = m_self->GetAttribute(AttrEntityArmorRepairDelayChanceSmall).get_float();
    } else if (m_self->HasAttribute(AttrEntityArmorRepairDelayChanceMedium)) {
        m_armorRepairDelayChance = m_self->GetAttribute(AttrEntityArmorRepairDelayChanceMedium).get_float();
    } else if (m_self->HasAttribute(AttrEntityArmorRepairDelayChanceLarge)) {
        m_armorRepairDelayChance = m_self->GetAttribute(AttrEntityArmorRepairDelayChanceLarge).get_float();
    }

    if (m_armorRepairDelayChance)
        m_armorRepairDuration = m_self->GetAttribute(AttrEntityArmorRepairDuration).get_uint32();

    // this is chance an npc has of delaying it's sebo (if applicable)
    if (m_self->HasAttribute(AttrEntityShieldBoostDelayChance)) {
        m_shieldBoosterDelayChance = m_self->GetAttribute(AttrEntityShieldBoostDelayChance).get_float();
    } else if (m_self->HasAttribute(AttrEntityShieldBoostDelayChanceSmall)) {
        m_shieldBoosterDelayChance = m_self->GetAttribute(AttrEntityShieldBoostDelayChanceSmall).get_float();
    } else if (m_self->HasAttribute(AttrEntityShieldBoostDelayChanceMedium)) {
        m_shieldBoosterDelayChance = m_self->GetAttribute(AttrEntityShieldBoostDelayChanceMedium).get_float();
    } else if (m_self->HasAttribute(AttrEntityShieldBoostDelayChanceLarge)) {
        m_shieldBoosterDelayChance = m_self->GetAttribute(AttrEntityShieldBoostDelayChanceLarge).get_float();
    }

    if (m_shieldBoosterDelayChance)
        m_shieldBoosterDuration = m_self->GetAttribute(AttrEntityShieldBoostDuration).get_uint32();

    // advanced AI variables  only used by sleepers for now (and on live).  will update advanced npcs to use these also
    if (m_self->HasAttribute(AttrAI_ShouldUseTargetSwitching))
        m_useTargSwitching = true;

    if (m_self->HasAttribute(AttrAI_ShouldUseSecondaryTarget))
        m_useSecondTarget = true;

    if (m_self->HasAttribute(AttrAI_ShouldUseSignatureRadius)) {
        m_useSigRadius = true;
        m_preferedSigRadius = m_self->GetAttribute(AttrAI_PreferredSignatureRadius).get_uint32();
    }

    if (m_self->HasAttribute(AttrAI_ChanceToNotTargetSwitch))
        m_switchTargChance = 1.0 - m_self->GetAttribute(AttrAI_ChanceToNotTargetSwitch).get_float();

    if (m_self->HasAttribute(AttrWarpScrambleRange))
        m_warpScramRange = m_self->GetAttribute(AttrWarpScrambleRange).get_float();

    if (m_self->HasAttribute(AttrEntityWarpScrambleChance))
        m_warpScramChance = 1.0 - m_self->GetAttribute(AttrEntityWarpScrambleChance).get_float();

    /*  test against chance/duration to determine what extra modules this npc has...see possibles below
    AttrEntityEquipmentMin = 456,
    AttrEntityEquipmentMax = 457,
    AttrEntityReactionFactor = 466,  //The chance of an entity attacking the same person as its group members.  Scales delay in joining in on fights too.

    */

    /*
//modifyTargetSpeedRange, modifyTargetSpeedChance
//entityWarpScrambleChance
    AttrWarpScrambleRange = 103,
    AttrWarpScrambleStrength = 105,
    AttrEntityWarpScrambleChance = 504,
    AttrWarpScrambleDuration = 505,
    AttrModifyTargetSpeedRange = 514
    AttrEntityTargetJam = 928,
    AttrEntityTargetJamDuration = 929,
    AttrEntityTargetJamDurationChance = 930,    // npcActivationChanceAttributeID in dgmEffects
    AttrEntityCapacitorDrainDurationChance = 931,   // npcActivationChanceAttributeID in dgmEffects
    AttrEntitySensorDampenDurationChance = 932,   // npcActivationChanceAttributeID in dgmEffects
    AttrEntityTrackingDisruptDurationChance = 933,   // npcActivationChanceAttributeID in dgmEffects
    AttrEntityTargetPaintDurationChance = 935,   // npcActivationChanceAttributeID in dgmEffects
    AttrEntityTargetJamMaxRange = 936,
    AttrEntityCapacitorDrainMaxRange = 937,
    AttrEntitySensorDampenMaxRange = 938,
    AttrEntityTrackingDisruptMaxRange = 940,
    AttrEntityTargetPaintMaxRange = 941,
    AttrEntityCapacitorDrainDuration = 942,
    AttrEntitySensorDampenDuration = 943,
    AttrEntityTrackingDisruptDuration = 944,
    AttrEntityTargetPaintDuration = 945,
    AttrEntityCapacitorDrainAmount = 946,
    AttrEntitySensorDampenMultiplier = 947,
    AttrEntityTrackingDisruptMultiplier = 948,
    AttrEntityTargetPaintMultiplier = 949,
    AttrEntitySensorDampenFallOff = 950,
    AttrEntityTrackingDisruptFallOff = 951,
    AttrEntityCapacitorFallOff = 952,
    AttrEntityTargetJamFallOff = 953,
    AttrEntityTargetPaintFallOff = 954,
    */

    // does this need to be running if there are no players in bubble?
    //  yes...npcs will warp out when no targets in sight range, but need a process tic to do that.
   // m_processTimer.Start(m_attackSpeed);

    // this can and should be used to tell spawnMgr to respawn this npc as required....
    //    AttrEntityGroupRespawnChance = 640,
    /*
    AttrEntityAttackDelayMin = 475,                     //Minimum attack delay time for entity. in ms
    AttrEntityAttackDelayMax = 476,                     //Maximum attack delay time for entity. in ms
    */
}

void NPCAIMgr::Process() {
    if (m_destiny->IsWarping())
        return;

    if (m_warpOutTimer.Check(false)) {
        // disallow warpout if spawn has active respawn timer (spawn is being chained)
        if (myNPC->GetSpawnMgr()->IsChaining(myNPC->SysBubble()->GetID())) {
            m_state = NPCAI::State::Idle;
            m_warpOutTimer.Disable();
        }
    }

    /* NPCAI::State definitions   -allan 25July15  (UD 1June16)
     *   Idle,       // not doing anything, nothing in sight....idle.  call Wander() to loosely orbit random object in bubble ~10-20k at 1/2 orbit speed
     *   Chasing,    // target within npc sight range.  attacking begins here.  use m_maxSpeed to get within falloff
     *   Following,  // between optimal and falloff.  try to get closer, but still orbiting and attacking
     *   Engaged,    // actively fighting (in orbit).  use m_orbitSpeed.
     *   Fleeing,    // running away....use m_maxSpeed then warp away when out of range	(does this make sense??)
     *   Signaling   // calling for help..use m_orbitSpeed *2 to speed tank while calling for reinforcements
     */
    switch(m_state) {
        case NPCAI::State::Idle: {
            if (m_beginFindTarget.Check()) {
                if (myNPC->SystemMgr()->PlayerCount() < 1) {
                    if (sConfig.npc.IdleWander)
                        if (!m_isWandering)
                            SetWander();
                    return;
                }
                std::vector<Client*> clientVec;
                clientVec.clear();
                DestinyManager* pDestiny(nullptr);
                myNPC->SysBubble()->GetPlayers(clientVec); // what about player drones?  yes...later
                for (auto &cur : clientVec) {
                    if (cur->IsInvul())
                        continue;
                    if (cur->GetShipSE() == nullptr)
                        continue;
                    if (cur->InPod()) {
                        if (sConfig.npc.TargetPod) {
                            if (myNPC->SystemMgr()->GetSystemSecurityRating() > sConfig.npc.TargetPodSec)
                                continue;
                        } else {
                            continue;
                        }
                    }
                    pDestiny = cur->GetShipSE()->DestinyMgr();
                    if (pDestiny == nullptr)   // this shouldnt be needed, but whatever...
                        continue;
                    if (pDestiny->IsCloaked() or pDestiny->IsWarping())
                        continue;
                    if (myNPC->GetPosition().distance(cur->GetShipSE()->GetPosition()) > m_sightRange)
                        continue;
                    _log(NPC__INFO, "%s(%u): Found %s(%u) - Begin Targeting.", \
                        myNPC->GetName(), myNPC->GetID(), cur->GetShipSE()->GetName(), cur->GetShipSE()->GetID());

                    Target(cur->GetShipSE());
                    return;
                }
            } else {
                if (!m_beginFindTarget.Enabled())
                    m_beginFindTarget.Start(m_attackSpeed);  //find target is based on npc attack speed.
            }
        } break;
        case NPCAI::State::Chasing:
        case NPCAI::State::Following:
        case NPCAI::State::Engaged: {
            if (myNPC->TargetMgr()->HasNoTargets()) {
                _log(NPC__AI_TRACE, "%s(%u): Stopped %s - HasNoTargets = true.", myNPC->GetName(), myNPC->GetID(), GetStateName(m_state).c_str());
                SetIdle();
                return;
            }
            SystemEntity* pTargetSE = myNPC->TargetMgr()->GetFirstTarget(false);
            if (pTargetSE == nullptr) {
                _log(NPC__AI_TRACE, "%s(%u): Stopped %s - GetFirstTarget() returned NULL.", myNPC->GetName(), myNPC->GetID(), GetStateName(m_state).c_str());
                SetIdle();
                return;
            }
            if (pTargetSE->SysBubble() == nullptr) {
                // target has no bubble?  make error
                sLog.Error("NPCAI Proc()", "Targ %s(%u) has no bubble", pTargetSE->GetName(), pTargetSE->GetID());
                ClearTarget(pTargetSE);
                return;
            }
            CheckDistance(pTargetSE);
            if (m_missileTimer.Check())
                LaunchMissile(m_missileTypeID, pTargetSE);
        } break;
        case NPCAI::State::WarpOut: {
            if (!m_destiny->IsWarping()) {
                _log(NPC__AI_TRACE, "%s(%u): Warping Complete.  Return to Idle.", myNPC->GetName(), myNPC->GetID());
                SetIdle();
            }
        } break;
        case NPCAI::State::WarpFollow:
        case NPCAI::State::Fleeing:
        case NPCAI::State::Signaling:{
            _log(NPC__AI_TRACE, "%s(%u): Called %s - needs to be completed.", myNPC->GetName(), myNPC->GetID(), GetStateName(m_state).c_str());
            m_state = NPCAI::State::Idle;
            // not sure how im gonna do these
        } break;
    }

    if (m_shieldBoosterTimer.Enabled())
        if (m_shieldBoosterTimer.Check())
            myNPC->UseShieldRecharge();

    if (m_armorRepairTimer.Enabled())
        if (m_armorRepairTimer.Check())
            myNPC->UseArmorRepairer();
}

bool NPCAIMgr::IsFighting() {
    // more to this here....
    return (m_state != NPCAI::State::Idle);
}

void NPCAIMgr::WarpOut()
{
    m_warpOutTimer.Disable();

    if (m_state == NPCAI::State::WarpOut) {
        m_state = NPCAI::State::Idle;
        return;
    }

    /** @todo  eventually, this will check with anomaly mgr for possible npc hideouts in system
     * based on npc faction, system, players in system, players in bubble, and *more later*
     * to determine a warpto target for this npc, or this group
     *
     * for now, if there are players in system, just warp to another belt.
     * if there are no players in this system, avoid using proc tics on npcs
     */

    SystemManager* pSys = myNPC->SystemMgr();
    if (pSys->PlayerCount()) {
        // pSys->GetAnomMgr();
        uint32 newBeltID(pSys->GetRandBeltID());
        if (newBeltID == sBubbleMgr.GetBeltID(myNPC->SysBubble()->GetID()))
            newBeltID = pSys->GetRandBeltID();

        m_state = NPCAI::State::WarpOut;
        SystemEntity* newBeltSE = pSys->GetSE(newBeltID);
        myNPC->GetSpawnMgr()->MoveSpawn(myNPC, sBubbleMgr.FindBubble(newBeltSE));
        m_destiny->WarpTo(newBeltSE->GetPosition());
    }
}

void NPCAIMgr::SetWander()
{
    if (myNPC->GetSpawnMgr() == nullptr)
        return;
    if (!m_isWandering) {
        _log(NPC__AI_TRACE, "%s(%u): Wandering:  No Targets within my sight range of %um", \
                myNPC->GetName(), myNPC->GetID(), m_sightRange);
        m_isWandering = true;
    }

    SystemBubble* pBubble = myNPC->SysBubble();

    // wandering.  nothing to shoot.  look for target.
    if (pBubble->IsAnomaly() or pBubble->IsIncursion() or pBubble->IsMission()) {
        return;
    }

    // disable orbit-on-idle
    //if (pBubble->HasDynamics() or pBubble->IsBelt()) {
    if (0) {
        // pick random entity and loosely orbit it.  if no entity found, orbit center of belt
        SystemEntity* pTargetSE = pBubble->GetRandomEntity();
        if (pTargetSE == nullptr)
            pTargetSE = myNPC->SystemMgr()->GetSE(sBubbleMgr.GetBeltID(pBubble->GetID()));
        if (pTargetSE == nullptr) {
            _log(NPC__WARNING, "%s(%u): Wandering:  No Target or beltSE found.", myNPC->GetName(), myNPC->GetID());
            // nothing here...leave bubble
            WarpOut();
            return;
        }
        m_destiny->SetMaxVelocity(m_orbitSpeed);
        uint16 orbitDistance = MakeRandomInt(10000, 20000);
        m_destiny->InitOrbit(pTargetSE, orbitDistance);
        _log(NPC__AI_TRACE, "%s(%u):  Just for shits-n-giggles, I\'m gonna orbit %s(%u) at %um.", \
                myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID(), orbitDistance);
        return;
    }

    if (m_destiny->IsMoving())
        m_destiny->Stop();
}

void NPCAIMgr::SetIdle() {
    if (m_state == NPCAI::State::Idle)
        return;
    // not doing anything....idle.

    /** @todo need to clear out targets here */

    _log(NPC__AI_TRACE, "%s(%u): Idle: returning to idle.", myNPC->GetName(), myNPC->GetID());
    m_state = NPCAI::State::Idle;
    m_destiny->Stop();
    m_destiny->SetMaxVelocity(m_orbitSpeed);

    m_missileTimer.Disable();
    m_webifierTimer.Disable();
    m_beginFindTarget.Disable();
    m_mainAttackTimer.Disable();
    m_armorRepairTimer.Disable();
    m_warpScramblerTimer.Disable();
    m_shieldBoosterTimer.Disable();

    SystemBubble* pBubble = myNPC->SysBubble();
    //disallow warpout if anomaly, incursion or mission rat
    if (pBubble->IsAnomaly() or pBubble->IsIncursion() or pBubble->IsMission())
        return;

    //disallow warpout by NOT setting timer.
    if (sConfig.npc.WarpOut > 0)
        if (myNPC->GetSpawnMgr() != nullptr)
            m_warpOutTimer.Start(sConfig.npc.WarpOut * 1000); // s to ms
}

void NPCAIMgr::SetChasing(SystemEntity* pTargetSE) {
    /** @todo implement chase timer using entityChaseMaxDuration to limit chase time. */
    if ((m_state == NPCAI::State::Chasing) and (m_destiny->IsGoto() or m_destiny->IsFollowing()))
        return;
    _log(NPC__AI_TRACE, "%s(%u): Begin chasing.  Target is %s(%u).", \
         myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID());
    // target out of range to attack/follow, but within npc sight range....use mwd/ab if equiped
    m_destiny->SetMaxVelocity(m_maxSpeed);
    m_destiny->GotoPoint(pTargetSE->GetPosition());  //head towards target
    m_state = NPCAI::State::Chasing;
    m_warpOutTimer.Disable();
}

void NPCAIMgr::SetFollowing(SystemEntity* pTargetSE) {
    if ((m_state == NPCAI::State::Following) and (m_destiny->IsGoto() or m_destiny->IsFollowing()))
        return;
    _log(NPC__AI_TRACE, "%s(%u): Begin following.  Target is %s(%u).", \
         myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID());
    // too close to chase, but to far to engage
    m_destiny->SetMaxVelocity(m_orbitSpeed * 2);
    m_destiny->Follow(pTargetSE, m_falloff);  //try to get inside falloff range
    m_state = NPCAI::State::Following;
    m_warpOutTimer.Disable();
}

void NPCAIMgr::SetEngaged(SystemEntity* pTargetSE) {
    // actively fighting
    if (m_state == NPCAI::State::Engaged) // and m_destiny->IsOrbiting())
        return;

    _log(NPC__AI_TRACE, "%s(%u): Begin engaging.  Target is %s(%u).", \
         myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID());
    m_destiny->SetMaxVelocity(m_orbitSpeed);
    m_destiny->InitOrbit(pTargetSE, m_optimalRange);  //try to get inside orbit range
    m_state = NPCAI::State::Engaged;
    m_warpOutTimer.Disable();
}

// not used yet
void NPCAIMgr::SetFleeing(SystemEntity* pTargetSE) {
    if ((m_state == NPCAI::State::Fleeing) and m_destiny->IsMoving())
        return;
    _log(NPC__AI_TRACE, "%s(%u): Begin fleeing.  Target is %s(%u).", \
         myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID());
    // actively fleeing
    //  use superspeed to disengage, then warp.  << both these will need to be written.
    //  this state is only usable by higher-class npcs.
    m_destiny->SetMaxVelocity(m_maxSpeed);
    m_state = NPCAI::State::Fleeing;
    m_warpOutTimer.Disable();
}

// not used yet
void NPCAIMgr::SetSignaling(SystemEntity* pTargetSE) {
    if ((m_state == NPCAI::State::Signaling) and m_destiny->IsOrbiting())
        return;
    _log(NPC__AI_TRACE, "%s(%u): Begin signaling.  Target is %s(%u).", \
         myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID());
    // actively signaling
    //  start speedtanking while signaling.  (im sure this is cheating, but fuckem.)
    //  this state is only usable by higher-class npcs.
    m_destiny->SetMaxVelocity(m_orbitSpeed * 2);
    m_destiny->InitOrbit(pTargetSE, m_falloff);  //try to get outside orbit range
    m_state = NPCAI::State::Signaling;
    m_warpOutTimer.Disable();
}

bool NPCAIMgr::InOrbitDistance(SystemEntity* pTargetSE) {
    double dist(myNPC->GetPosition().distance(pTargetSE->GetPosition()) - pTargetSE->GetRadius());
    return (dist < m_orbitDistance);
}
bool NPCAIMgr::InFalloffDistance(SystemEntity* pTargetSE) {
    double dist(myNPC->GetPosition().distance(pTargetSE->GetPosition()) - pTargetSE->GetRadius());
    return (dist < m_falloffDistance);
}

bool NPCAIMgr::InEngageDistance(SystemEntity* pTargetSE) {
    double dist(myNPC->GetPosition().distance(pTargetSE->GetPosition()) - pTargetSE->GetRadius());
    return (dist < m_engageDistance);
}

bool NPCAIMgr::InChaseDistance(SystemEntity* pTargetSE) {
    double dist(myNPC->GetPosition().distance(pTargetSE->GetPosition()) - pTargetSE->GetRadius());
    return (dist < m_chaseDistance);
}

bool NPCAIMgr::InMaxDistance(SystemEntity* pTargetSE) {
    double dist(myNPC->GetPosition().distance(pTargetSE->GetPosition()) - pTargetSE->GetRadius());
    return (dist < m_maxDistance);
}

void NPCAIMgr::CheckDistance(SystemEntity* pTargetSE) {
    double dist = myNPC->GetPosition().distance(pTargetSE->GetPosition());
    if ((dist > m_sightRange) and (!myNPC->TargetMgr()->IsTargetedBy(pTargetSE))) {
        _log(NPC__AI_TRACE, "%s(%u): CheckDistance: %s(%u) is too far away (%.1fm).  Return to Idle.", \
             myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID(), dist);
        if (m_state != NPCAI::State::Idle) {
            // target is no longer in npc's "sight range" and is NOT targeting this npc.  unlock target and return to idle.
            //   should we do anything else here?  search for another target?  wander around?  yes..later
            // if npc is targeted greater than this distance, it will chase
            ClearTarget(pTargetSE);
        }
        return;
    }

    m_isWandering = false;

    // TODO:  update these to proper weapon optimal distances
    if (dist < m_flyRange) {
        SetEngaged(pTargetSE);
    } else if (dist < m_boostRange) {
        SetFollowing(pTargetSE);
    } else {
        SetChasing(pTargetSE);
    }

    _log(NPC__AI_TRACE, "%s(%u): CheckDistance:  target: %s(%u), state: %s, dist: %.0fm, flyRange: %u, boostRange: %u.", \
            myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID(), GetStateName(m_state).c_str(), dist, m_flyRange, m_boostRange);

    Attack(pTargetSE);
}

void NPCAIMgr::Target(SystemEntity* pTargetSE) {
    float targetTime = GetTargetTime();
    bool chase(false);

    if (!myNPC->TargetMgr()->StartTargeting(pTargetSE, targetTime, m_maxLockedTargets, m_sightRange, chase)) {
        if (chase) {
            _log(NPC__AI_TRACE, "%s(%u): Targeting of %s(%u) failed.  Begin Chasing.", \
                        myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID());
            SetChasing(pTargetSE);
        } else {
            _log(NPC__AI_TRACE, "%s(%u): Targeting of %s(%u) failed.  Clear Target and Return to Idle.", \
                        myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID());
            SetIdle();
        }
        return;
    }
    m_beginFindTarget.Disable();
    CheckDistance(pTargetSE);

    if (!m_mainAttackTimer.Enabled())
        m_mainAttackTimer.Start(m_attackSpeed);

    if (!m_missileTimer.Enabled() and (m_launcherCycleTime > 100))
        m_missileTimer.Start(m_launcherCycleTime);
}

void NPCAIMgr::Targeted(SystemEntity* pTargetSE) {
    if (pTargetSE == nullptr)
        return;
    double targetTime = GetTargetTime();

    _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) while %s.", \
            myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID(), GetStateName(m_state).c_str());

    switch(m_state) {
        case NPCAI::State::Idle: {
            _log(NPC__AI_TRACE, "%s(%u): Begin Approaching and start Targeting sequence.", \
                    myNPC->GetName(), myNPC->GetID());
            SetChasing(pTargetSE);

            bool chase = false;
            if (!myNPC->TargetMgr()->StartTargeting( pTargetSE, targetTime, m_maxLockedTargets, m_sightRange, chase)) {
                if (chase) {
                    _log(NPC__AI_TRACE, "%s(%u): Targeting of %s(%u) failed.  Begin Chasing.", \
                            myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID());
                    SetChasing(pTargetSE);
                } else {
                    _log(NPC__AI_TRACE, "%s(%u): Targeting of %s(%u) failed.  Clear Target and Return to Idle.", \
                            myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID());
                    SetIdle();
                }
            }
            m_beginFindTarget.Disable();
            //CheckDistance(pAgressor);
        } break;

        /** @todo  determine if new targetedby entity is weaker than current target. use optimalSigRadius to test for 'optimal' target */
        case NPCAI::State::Chasing: {
        } break;
        case NPCAI::State::Following: {
        } break;
        case NPCAI::State::Engaged: {
        } break;
        case NPCAI::State::Fleeing: {
        } break;
        case NPCAI::State::Signaling: {
        } break;
    }
    if (!m_shieldBoosterTimer.Enabled())
        if (MakeRandomFloat() > m_shieldBoosterDelayChance)
            m_shieldBoosterTimer.Start(m_shieldBoosterDuration);
    if (!m_armorRepairTimer.Enabled())
        if (MakeRandomFloat() > m_armorRepairDelayChance)
            m_armorRepairTimer.Start(m_armorRepairDuration);
}

void NPCAIMgr::TargetLost(SystemEntity* pTargetSE) {
    if (pTargetSE == nullptr)
        return;
    switch(m_state) {
        case NPCAI::State::Chasing:
        case NPCAI::State::Following:
        case NPCAI::State::Engaged: {
            // implement chance for npc to follow warping player
            // sConfig.npc.WarpFollowChance;
            // NPCAI::State::WarpFollow
            if (myNPC->TargetMgr()->HasNoTargets()) {
                _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) lost. No targets remain.  Return to Idle.", \
                        myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID());
                SetIdle();
            } else {
                _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) lost, but more targets remain.", \
                        myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID());
                /** @todo engage weakest target in current list */
                Attack(myNPC->TargetMgr()->GetFirstTarget(true));
            }
        }
    }
}

void NPCAIMgr::Attack(SystemEntity* pTargetSE) {
    // TODO:  most of these checks should not be needed on EVERY tic...
    if (m_mainAttackTimer.Check()) {
        if (pTargetSE->DestinyMgr() == nullptr) {
            sLog.Error("NPC Attack()", "Target %s(%u) has no destiny manager.", pTargetSE->GetName(), pTargetSE->GetID());
            _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) has no destiny manager.  Clear target and move on",
                    myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID());
            m_missileTimer.Disable();
            ClearTarget(pTargetSE);
            return;
        }
        // Check to see if the target is not cloaked:
        if (pTargetSE->DestinyMgr()->IsCloaked()) {
            _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) is cloaked.  Clear target and move on",
                    myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID());
            m_missileTimer.Disable();
            ClearTarget(pTargetSE);
            return;
        }
        if (myNPC->TargetMgr()->CanAttack())
            AttackTarget(pTargetSE);
    }
}

void NPCAIMgr::ClearTarget(SystemEntity* pTargetSE) {
    myNPC->TargetMgr()->ClearTarget(pTargetSE);
    //myNPC->TargetMgr()->OnTarget(pTargetSE, TargMgr::Mode::Lost);

    if (myNPC->TargetMgr()->HasNoTargets())
        SetIdle();
}

//also check for special effects and write code to implement them
void NPCAIMgr::AttackTarget(SystemEntity* pTargetSE) {
    // put checks here for point/tackle

    // effects are listed in EVE_Effects.h
    // TODO: this needs to be updated....rogue drones working
    std::string guid = "effects.Laser"; // client looks for 'turret' in ship.ball.modules for 'effects.laser'...error for npcs ex drone
    uint32 gfxID(0);
    if (m_self->HasAttribute(AttrGfxTurretID))// graphicID for turret
        gfxID = m_self->GetAttribute(AttrGfxTurretID).get_uint32();
    /*
    if (m_pDrone->GetSelf()->HasAttribute(AttrGfxBoosterID))// graphicID for booster/ewar
        gfxID = m_pDrone->GetSelf()->GetAttribute(AttrGfxBoosterID).get_uint32();
    */

    m_destiny->SendGFX14(m_self->itemID(), m_self->itemID(), m_self->typeID(),
                         pTargetSE->GetID(),0,std::move(guid),1,1,
                         1,m_attackSpeed,0,0,gfxID);

    Damage d(myNPC,
             m_self,
             myNPC->GetKinetic(),
             myNPC->GetThermal(),
             myNPC->GetEM(),
             myNPC->GetExplosive(),
             m_formula.GetNPCToHit(myNPC, pTargetSE)
            );

    if (sConfig.npc.UseDamageMultiplier)
        d *= m_damageMultiplier;

    pTargetSE->ApplyDamage(d);
}

/* missile shit..
 * //AttrEntityDefenderChance  - chance to shoot defender missile at incomming missile
 * //AttrMissileLaunchDuration  - missile cycle time
 * //AttrEntityMissileTypeID
 * //AttrMissileEntityVelocityMultiplier
 * //AttrMissileEntityFlightTimeMultiplier
 * //AttrMissileEntityAoeCloudSizeMultiplier
 * //AttrMissileEntityAoeVelocityMultiplier
 * //AttrMissileEntityAoeFalloffMultiplier
 */

void NPCAIMgr::LaunchMissile(uint16 typeID, SystemEntity* pTargetSE)
{
    if (typeID == 0)
        return;
    // Actually Launch a missile, creating a new Destiny object for it
    // ItemData( uint32 _typeID, uint32 _ownerID, uint32 _locationID, EVEItemFlags _flag, const char *_name = "", \
              const GPoint &_position = NULL_ORIGIN, const char *_customInfo = "", bool _contraband = false);
    ItemData idata(typeID, myNPC->GetID(), myNPC->GetLocationID(), flagMissile, "NPC Missile", myNPC->GetPosition());
    InventoryItemRef missileRef = sItemFactory.SpawnItem(idata);
    if (missileRef.get() == nullptr)
        return;  // make error here

    // modify missile based on npc attribs
    if (m_self->HasAttribute(AttrMissileEntityVelocityMultiplier))
        missileRef->MultiplyAttribute(AttrMaxVelocity, m_self->GetAttribute(AttrMissileEntityVelocityMultiplier));
    if (m_self->HasAttribute(AttrMissileEntityFlightTimeMultiplier))    // this may be wrong
        missileRef->MultiplyAttribute(AttrExplosionDelay, m_self->GetAttribute(AttrMissileEntityFlightTimeMultiplier));
    if (m_self->HasAttribute(AttrMissileEntityAoeVelocityMultiplier))
        missileRef->MultiplyAttribute(AttrAoeVelocity, m_self->GetAttribute(AttrMissileEntityAoeVelocityMultiplier));
    if (m_self->HasAttribute(AttrMissileEntityAoeCloudSizeMultiplier))
        missileRef->MultiplyAttribute(AttrAoeCloudSize, m_self->GetAttribute(AttrMissileEntityAoeCloudSizeMultiplier));
    if (m_self->HasAttribute(AttrMissileEntityAoeFalloffMultiplier))
        missileRef->MultiplyAttribute(AttrAoeFalloff, m_self->GetAttribute(AttrMissileEntityAoeFalloffMultiplier));

    SystemManager* pSystem = myNPC->SystemMgr();
    // Missile(InventoryItemRef self, PyServiceMgr &services, SystemManager* system, InventoryItemRef module, SystemEntity* target, ShipItem* ship);
    Missile* pMissile = new Missile(missileRef, *(pSystem->GetServiceMgr()),  pSystem, m_self, pTargetSE, myNPC);
    if (pMissile == nullptr)
        return; // make error here
    double distance = pMissile->GetPosition().distance(pTargetSE->GetPosition());
    double missileSpeed = missileRef->GetAttribute(AttrMaxVelocity).get_float();
    double travelTime = (distance / missileSpeed);
    if (travelTime < 1)
        travelTime = 1;
    pMissile->SetSpeed(missileSpeed);
    pMissile->SetHitTimer(travelTime * EvE::Timer::Second);
    pMissile->DestinyMgr()->MakeMissile(pMissile);

    // tell target a missile has been launched at them.. (defender missile trigger for ship, tower, pos, npc, others?)
    if (typeID != EVEDB::invTypes::DefenderI)  // but only if it's NOT a defender missile
        pTargetSE->MissileLaunched(pMissile);
}

void NPCAIMgr::MissileLaunched(Missile* pMissile)
{
    float chance = m_self->GetAttribute(AttrEntityDefenderChance).get_float();
    if (sConfig.npc.DefenderMissileChance)
        chance = sConfig.npc.DefenderMissileChance;
    // check chance to shoot defender missile at incoming missile (working, ??/??/??)
    if (MakeRandomFloat() < chance)
        LaunchMissile(EVEDB::invTypes::DefenderI, pMissile); // defender missile
}

float NPCAIMgr::GetTargetTime()
{
    float targetTime = (m_self->GetAttribute(AttrScanSpeed).get_float());

    // if target time not defined, use drone size to set default time to lock
    if (targetTime < 1) {
        float radius = m_self->GetAttribute(AttrRadius).get_float();
        if (radius < 30) {
            targetTime = 1500;
        } else if (radius < 60) {
            targetTime = 2500;
        } else if (radius < 150) {
            targetTime = 4000;
        } else if (radius < 280) {
            targetTime = 6000;
        } else if (radius < 550) {
            targetTime = 8000;
        } else {
            targetTime = 13000;
        }
    }
    return targetTime;
}

void NPCAIMgr::DisableRepTimers(bool shield/*true*/, bool armor/*true*/)
{
    if (armor)
        m_armorRepairTimer.Disable();
    if (shield)
        m_shieldBoosterTimer.Disable();
}

std::string NPCAIMgr::GetStateName(int8 stateID)
{
    switch (stateID) {
        case NPCAI::State::Idle:           return "Idle";
        case NPCAI::State::Chasing:        return "Chasing";
        case NPCAI::State::Engaged:        return "Engaged";
        case NPCAI::State::Fleeing:        return "Fleeing";
        case NPCAI::State::Following:      return "Following";
        case NPCAI::State::Signaling:      return "Signaling";
        case NPCAI::State::WarpOut:        return "Warping Out";
        case NPCAI::State::WarpFollow:     return "Following Warp";
        default:                           return "Invalid";
    }
}

/*
  switch (m_state) {
     case NPCAI::State::Idle: {
     } break;
     case NPCAI::State::Chasing: {
     } break;
     case NPCAI::State::Engaged: {
     } break;
     case NPCAI::State::Fleeing: {
     } break;
     case NPCAI::State::Following: {
     } break;
     case NPCAI::State::Signaling: {
     } break;
     case NPCAI::State::WarpOut: {
     } break;
     case NPCAI::State::WarpFollow: {
     } break;
  }

 */

/*
             // action, orbit, falloff, engage, chase, max
             if (InActionDistance(targSE)) {               600
             } else if (InOrbitDistance(targSE)) {
             } else if (InFalloffDistance(targSE)) {
             } else if (InEngageDistance(targSE)) {
             } else if (InChaseDistance(targSE)) {
             } else if (InMaxDistance(targSE)) {
             } else {
                 // outside max distance
             }
 */
