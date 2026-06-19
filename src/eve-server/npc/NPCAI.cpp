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
    AI Version: 0.83
*/

/** @todo  ai update ideas
 *   bubble call *SomeFunction* to tell ai of new ship arriving in bubble
 *   method to use npc's preferred sig radius for targets
 *   finish flee and signal action methods (and determine who can use them and when)
 *      - this should take system sov, npc anomalies, destruction speed, and pirate faction
 *   add methods to check target/targeter warping out and chance of npc following (and possibly calling backup)
 *
 *   TODO:  use fake orbit like drones do?
 *
 *   also...can/should npcs have multiple targets?  like lazor on one and ewar on another?
 *		we are currently set up for a single target, but could add checks for a separate non-lazor target...interesting.  web one, shoot another
 *		would be affected by distance, but if npc can shoot missiles, then its nbd.
 *
 * NOTE:  elite npc target drones first!
 *   recall drones -> npc target player -> deploy drones -> timer countdown (delay 10-20 sec or so) -> npc target drones again
 *		npcs grifting players.  lol
 *
 *  have data...needs coding...
 *   ewar shit, including point/tackle
 *
 * NPC__AI_LOGIC
 */

#include "../eve-server.h"

#include "../../eve-common/EVE_Damage.h"

#include "Client.h"
#include "effects/EffectsDataMgr.h"
#include "effects/EffectsProcessor.h"
#include "inventory/AttributeEnum.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "ship/Missile.h"
#include "ship/Ship.h"
#include "system/DestinyManager.h"
#include "system/Damage.h"
#include "system/SystemBubble.h"

NPCAIMgr::NPCAIMgr(NPC* mySE)
: myNPC(mySE),
  m_actionTarget(nullptr),
  m_attackTarget(nullptr),
  m_destiny(mySE->DestinyMgr()),
  m_self(mySE->GetSelf()),
  m_useSigRadius(false),
  m_useSecondTarget(false),
  m_useTargSwitching(false),
  m_size(NPCAI::Size::None),
  m_state(NPCAI::State::Idle),
  m_action(NPCAI::Action::Idle),
  m_maxLockedTargets(0),
  m_effectID(0),
  m_turretID(0),
  m_actionSpeed(-1),
  m_attackSpeed(-1),
  m_armorRepairDuration(-1),
  m_shieldBoosterDuration(-1),
  m_maxSpeed(0),
  m_cruiseSpeed(0),
  m_sigRadius(0),
  m_sightRange(0),
  m_attackRange(0),
  m_chaseRange(0),
  m_optimalRange(0),
  m_falloffDistance(0),
  m_flyRange(0),
  m_actionTime(0),
  m_attackTime(0),
  m_chaseTimeEnd(0),
  m_switchTargChance(0.0f),
  m_trackingSpeed(0.0f),
  m_damageMultiplier(0.0f),
  m_armorRepairDelayChance(0.0f),
  m_shieldBoosterDelayChance(0.0f),
  // timers
  m_actionTimer(0),
  m_attackTimer(0),
  m_missileTimer(0),
  m_shieldBoosterTimer(0),
  m_armorRepairTimer(0),
  m_beginFindTarget(0),
  m_warpOutTimer(0)
{
    assert(m_self.get() != nullptr);
}

void NPCAIMgr::Init() {
    // set basic ship data
    m_turretID = m_self->GetAttribute(AttrGfxTurretID).get_uint32();
    m_sigRadius = m_self->GetAttribute(AttrSignatureRadius).get_uint32();
    m_attackSpeed = m_self->GetAttribute(AttrSpeed).get_int();

    //NOTE:  they are not all lazors...
    switch (myNPC->GetWarFactionID()) {
        case factionCaldari:
        case factionBloodRaider:
        case factionSanshas:
        case factionAmarr:
        case factionCONCORD:    // testing.  these do omni
        case factionSleeper:   // testing.  these do omni
        case factionThukker:    // testing.  these do exp/therm
        case factionSociety:    // SoCT   testing.  these do exp/therm
        case factionKhanid:
        case factionUnknown: {  // rogue drones
            // lasers;  mostly thermal and em damage
            m_effectID = EvE::GFXID::targetAttack;
        } break;
        case factionAngel:
        case factionSerpentis:
        case factionSyndicate:
        case factionMordusLegion: // testing.  these do kin/therm
        case factionGuristas:   // testing.  these do kin/therm
        case factionGallente:   // testing.  these do kin/therm
        case factionSistersOfEVE: {
            // projectiles (hybrid, railgun, cannon);  mostly kinetic and explosive damage
            m_effectID = EvE::GFXID::projectileFiredForEntities;
        } break;
        case factionAmmatar:
        case factionMinmatar:
        case factionJove:{
            // projectiles;  mostly kinetic and explosive damage
            m_effectID = EvE::GFXID::projectileFired;
        } break;

        case factionORE:        // non-aggressive gallente faction
        case factionInterBus: {  // non-aggressive
            m_effectID = 0;
        } break;
    }

    _log(NPC__INFO, "%s(%u): default effect is %s(%u) for %s.", \
            m_self->name(), m_self->itemID(), sFxDataMgr.GetEffectName(m_effectID).c_str(), \
            m_effectID, sDataMgr.GetFactionName(myNPC->GetWarFactionID()).c_str());

    m_damageMultiplier = m_self->GetAttribute(AttrDamageMultiplier).get_float();
    if (m_damageMultiplier < 0.1f)
        m_damageMultiplier = 1.0f;

    // speeds
    m_maxSpeed = m_self->GetAttribute(AttrMaxVelocity).get_uint32();
    m_cruiseSpeed = m_self->GetAttribute(AttrEntityCruiseSpeed).get_uint32();   // ship speed when not chasing target

    // distances
    m_attackRange = m_self->GetAttribute(AttrEntityAttackRange).get_int();  // max attack distance.  should this be target range also?
    m_optimalRange = m_self->GetAttribute(AttrMaxRange).get_uint32();  // max distance range does not affect the to-hit equation.  (Optimal Range)
    m_falloffDistance = m_attackRange - m_self->GetAttribute(AttrFalloff).get_int(); // distance at which damage is halved
    m_trackingSpeed = m_self->GetAttribute(AttrTrackingSpeed).get_float();  //rad/sec
    m_flyRange = m_self->GetAttribute(AttrEntityFlyRange).get_uint32();    //AttrOrbitRange is 0 for npc
    m_chaseRange = m_self->GetAttribute(AttrEntityChaseMaxDistance).get_int();  // npc will activate mwd if target is farther than this distance

    // sightrange is arbitrary now.  live uses 'can see all on grid'
    if (EvE::icontains(m_self->type().groupName(), "Swarm")) {
        m_sightRange = 30000;
        m_size = NPCAI::Size::Swarm;
    } else if (EvE::icontains(m_self->type().groupName(), "Frigate")) {
        m_sightRange = 40000;
        m_size = NPCAI::Size::Frigate;
    } else if (EvE::icontains(m_self->type().groupName(), "Destroyer")) {
        m_sightRange = 50000;
        m_size = NPCAI::Size::Destroyer;
    } else if (EvE::icontains(m_self->type().groupName(), "Cruiser")) {
        m_sightRange = 70000;
        m_size = NPCAI::Size::Cruiser;
    } else if (EvE::icontains(m_self->type().groupName(), "BattleCruiser")) {
        m_sightRange = 90000;
        m_size = NPCAI::Size::BCruiser; //22824
    } else if (EvE::icontains(m_self->type().groupName(), "BattleShip")) { // 350+
        m_sightRange = 130000;
        m_size = NPCAI::Size::BShip;
    } else {
        // not sure what to do here.
    	_log(NPC__WARNING, "%s(%u): groupName %s (ID %u) - no match found in 'contains' test ", \
                m_self->name(), m_self->itemID(), m_self->type().groupName().c_str(), m_self->groupID());
    }

    //  check into entityStrength(542)   504 entries

    // distance checks and corrections...verify these...are they needed?
    if (m_attackRange < 1000)
        m_attackRange = m_size * 10;
    if (m_attackRange > m_sightRange)
        m_sightRange = m_attackRange * 2;
    if (m_optimalRange < 1.0f)
        m_optimalRange = m_attackRange / 2;
    if (m_flyRange < 1)
        m_flyRange = m_optimalRange + (m_sightRange / 30);
    if (m_flyRange > m_chaseRange)
        m_chaseRange = m_flyRange * 3;

    // ship targets
    //m_self->GetAttribute(AttrMaxAttackTargets).get_uint32()     may use this later...
    m_maxLockedTargets = m_self->GetAttribute(AttrMaxLockedTargets).get_uint32();
    if ((m_maxLockedTargets < 1) or (m_attackSpeed < 1)) {
        // these are passive
        if (is_log_enabled(NPC__INFO))
            _log(NPC__INFO, "%s(%u): setting to passive.", myNPC->GetName(), myNPC->GetID());
        // set map in bubble for passives to have escort guard them?   maybe later with more advanced AI
        m_state = NPCAI::State::Passive;
        m_action = NPCAI::Action::Passive;
        //m_maxSpeed = m_cruiseSpeed;     // most of these are max > 1k
        m_flyRange /= 10;             // these are all set to 200k
    }

    // this is for npc to rep itself   NOTE:  both these are also in fx data
    m_armorRepairDuration = m_self->GetAttribute(AttrEntityArmorRepairDuration).get_uint32();
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

    // this is for npc to boost itself
    m_shieldBoosterDuration = m_self->GetAttribute(AttrEntityShieldBoostDuration).get_uint32();
    // this is chance an npc has of delaying it's boost (if applicable)
    if (m_self->HasAttribute(AttrEntityShieldBoostDelayChance)) {
        m_shieldBoosterDelayChance = m_self->GetAttribute(AttrEntityShieldBoostDelayChance).get_float();
    } else if (m_self->HasAttribute(AttrEntityShieldBoostDelayChanceSmall)) {
        m_shieldBoosterDelayChance = m_self->GetAttribute(AttrEntityShieldBoostDelayChanceSmall).get_float();
    } else if (m_self->HasAttribute(AttrEntityShieldBoostDelayChanceMedium)) {
        m_shieldBoosterDelayChance = m_self->GetAttribute(AttrEntityShieldBoostDelayChanceMedium).get_float();
    } else if (m_self->HasAttribute(AttrEntityShieldBoostDelayChanceLarge)) {
        m_shieldBoosterDelayChance = m_self->GetAttribute(AttrEntityShieldBoostDelayChanceLarge).get_float();
    }

    //AttrEntityReactionFactor = 466,  //The chance of an entity attacking the same target as its group members.  2702 entries

    // advanced AI variables - only used by sleepers on live.  will update advanced npcs to use these also
    m_useTargSwitching  = m_self->GetAttribute(AttrAI_ShouldUseTargetSwitching).get_bool();
    m_useSecondTarget   = m_self->GetAttribute(AttrAI_ShouldUseSecondaryTarget).get_bool();
    m_useSigRadius      = m_self->GetAttribute(AttrAI_ShouldUseSignatureRadius).get_bool();
    if (m_self->HasAttribute(AttrAI_ChanceToNotTargetSwitch))
        m_switchTargChance = 1.0f - m_self->GetAttribute(AttrAI_ChanceToNotTargetSwitch).get_float();

    // disallowOffensiveModifiers  If this module is in use and this attribute is 1, then offensive modules cannot be used on the ship if they apply modifiers for the duration of their effect. If this is put on a ship or NPC with value of 1, then the ship or NPC are immune to offensive modifiers (target jamming, tracking disruption etc.)
    // 459 entries

    // make map of available 'modules' for this npc
    /**  not sure if/how/when i wanna do this...
    sFxDataMgr.GetTypeEffect(m_self->typeID(), m_effectMap);
    for (auto &cur : m_self->type().m_stateFxMap) {
        fxData data = fxData();
        data.action = FX::Action::Invalid;
        data.srcRef = m_self;
        sFxProc.ParseExpression(m_self.get(), sFxDataMgr.GetExpression(cur.second.preExpression), data);
    }
    */

    if (is_log_enabled(NPC__INFO))
        _log(NPC__INFO, "%s(%u): sight:%u, attack:%u, chase:%u, fly:%u, falloff:%u, optimal:%u.", \
        myNPC->GetName(), myNPC->GetID(), m_sightRange, m_attackRange, m_chaseRange, \
        m_flyRange, m_falloffDistance, m_optimalRange);
}

//NOTE:  this is also called after npc added to system, before destiny::warp() hits.  this sets them to 'wandering'
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

    /* NPCAI::State definitions   -allan 25July15  (UD 1June16)  (UD/RW Sept25)
     *   Idle,       // not doing anything, nothing in sight....idle.  call Wander() to loosely orbit random object in bubble ~10-20k at 1/2 orbit speed
     *   Chasing,    // target within npc sight range.  attacking begins here.  use m_maxSpeed to get within falloff
     *   Following,  // between optimal and falloff.  try to get closer, but still orbiting and attacking
     *   Engaged,    // actively fighting (in orbit).  use m_cruiseSpeed.
     *   Fleeing,    // running away....use m_maxSpeed then warp away when out of range	(does this make sense??)
     *   Signaling   // calling for help..use m_cruiseSpeed *2 to speed tank while calling for reinforcements
     *   WarpOut
     *   WarpFollow
     *   Passive     // no attack
     *   Delay       // timer delay
     */
    switch(m_state) {
        // passive npcs can warp out, so check this first
        case NPCAI::State::WarpOut: {
            if (!m_destiny->IsWarping()) {
                _log(NPC__AI_TRACE, "%s(%u): Warping Complete.  Return to Idle.", myNPC->GetName(), myNPC->GetID());
                SetIdle();
            }
            return;
        } break;

        case NPCAI::State::Idle: {
            if (m_action == NPCAI::Action::Wandering)
                return;
            if (m_action == NPCAI::Action::Passive) {
                // passives can be set to idle after some calls (like warpout)
                m_state = NPCAI::State::Passive;
                return;
            }
            // spawned rats start in normal bubble warp in belt. this avoids them from being set to 'wandering'
            if (myNPC->SysBubble()->IsNormal())
                return;

            if (!myNPC->SysBubble()->HasPlayers()) {
                if (sConfig.npc.IdleWander)
                    SetWander();
                return;
            }

            if (!m_beginFindTarget.Enabled()) {
                if (m_self->HasAttribute(AttrEntityAttackDelayMin)) {
                    m_beginFindTarget.Start(MakeRandomUInt(m_self->GetAttribute(AttrEntityAttackDelayMin).get_uint32(), m_self->GetAttribute(AttrEntityAttackDelayMax).get_uint32()));
                    _log(NPC__AI_LOGIC, "%s(%u) has attack delay.  Setting target delay timer.", myNPC->GetName(), myNPC->GetID());
                } else if (m_attackSpeed) {
                    m_beginFindTarget.Start(m_attackSpeed);
                    _log(NPC__AI_LOGIC, "%s(%u) has no attack delay.  Setting target timer to %ims", \
                            myNPC->GetName(), myNPC->GetID(), m_attackSpeed);
                } else {
                    _log(NPC__INFO, "%s(%u) has no attack delay or speed.  Setting to passive", myNPC->GetName(), myNPC->GetID());
                            m_state = NPCAI::State::Passive;
                            m_action = NPCAI::Action::Passive;
                }

                return;
            }

            if (m_beginFindTarget.Check())
                PickTarget();

            return;
        } break;

        case NPCAI::State::Engaged:
        case NPCAI::State::Following: {
            if (!VerifyTarget()) {
                // invalid target.  check for more or reset to idle?
                _log(NPC__AI_TRACE, "%s(%u):%s; VerifyTarget() failed.", myNPC->GetName(), myNPC->GetID(), GetStateName(m_state));
                SetIdle();
                return;
            }
        } break;

        case NPCAI::State::Chasing: {
            if (!VerifyTarget()) {
                // invalid target.  check for more or reset to idle?
                _log(NPC__AI_TRACE, "%s(%u):Chasing;  VerifyTarget() failed.", myNPC->GetName(), myNPC->GetID());
                SetIdle();
                return;
            }
            if ((m_chaseTimeEnd > 0) and (m_chaseTimeEnd < GetFileTimeNow())) {
                // chase time exceeded.  cancel chase
                _log(NPC__AI_TRACE, "%s(%u): chaseTime exceeded.", myNPC->GetName(), myNPC->GetID());
                SetIdle();
                return;
            }
            // can we still doaction while chasing?  depends on distance, but missiles should be ok
        } break;

        case NPCAI::State::Fleeing:
			//  note:  higher-class ships have a chance of warping away once thier support is destroyed.
        case NPCAI::State::WarpFollow: {
            //  not sure yet
            _log(NPC__AI_TRACE, "%s(%u): Called %s - needs to be completed.", \
                    myNPC->GetName(), myNPC->GetID(), GetStateName(m_state));
            SetIdle();
            return;
        } break;

        case NPCAI::State::Signaling: {
            if (m_action == NPCAI::Action::Passive) {
                // hauler escort signaling will orbit hauler while hauler boosts
            }
        } break;

        case NPCAI::State::Assisting: {
            // hauler escort signaling will orbit hauler while hauler boosts

        } break;

        case NPCAI::State::Passive: {
            // need to write something for passive npcs (haulers, maybe others)
            // passives can remote-boost; anything else?
            return;
        } break;

        case NPCAI::State::Delay: {
            if ((m_chaseTimeEnd > 0) and (m_chaseTimeEnd < GetFileTimeNow())) {
                // chase time delay expired.  engage chase speed
                SetChasing(m_attackTarget);
            }

            return;
        } break;
    }

    // should we make a call to check self-rep/-boost here?

    // how often should we check distance?  do we need it every tic?  yeah, because ships move
    CheckDistance(m_attackTarget);
    DoAction();
}

bool NPCAIMgr::IsFighting() {
    // more to this here...but is it used?
    switch (m_state) {
        case NPCAI::State::Idle:
        case NPCAI::State::Fleeing:
        case NPCAI::State::Passive:
        case NPCAI::State::WarpOut:
        case NPCAI::State::Signaling: {
            return false;
        } break;
        case NPCAI::State::Chasing:
        case NPCAI::State::Engaged:
        case NPCAI::State::Following:
        case NPCAI::State::WarpFollow: {
            return true;
        } break;
    }

    switch (m_action) {
        case NPCAI::Action::Idle:
        case NPCAI::Action::Passive:
        case NPCAI::Action::Wandering: {
            return false;
        } break;
        case NPCAI::Action::Attack: {
            return true;
        } break;
        case NPCAI::Action::ShieldRep: {
        } break;
        case NPCAI::Action::ArmorRep: {
        } break;
        case NPCAI::Action::Web: {
        } break;
        case NPCAI::Action::Scram: {
        } break;
        case NPCAI::Action::EWar: {
        } break;
    }

    return false;
}

void NPCAIMgr::WarpOut()
{
    m_warpOutTimer.Disable();

    if (m_state == NPCAI::State::WarpOut) {
        _log(NPC__AI_TRACE, "%s(%u):  Calling WarpOut while state=WarpOut.", myNPC->GetName(), myNPC->GetID());
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
        if (newBeltID == sBubbleMgr.GetBeltID(myNPC->SysBubble()->GetID()))
            newBeltID = pSys->GetRandBeltID();

        SystemEntity* newBeltSE = pSys->GetSE(newBeltID);
        SystemBubble* pBubble(sBubbleMgr.FindBubble(newBeltSE));
        _log(NPC__AI_LOGIC, "%s(%u):  Calling WarpOut.  Headed to bubble %u", \
                myNPC->GetName(), myNPC->GetID(), pBubble->GetID());

        m_state = NPCAI::State::WarpOut;
        myNPC->GetSpawnMgr()->MoveSpawn(myNPC, pBubble);
        m_destiny->WarpTo(newBeltSE->GetPosition());
    }
}

void NPCAIMgr::WarpOutComplete() {
    m_warpOutTimer.Disable();
    m_state = NPCAI::State::Idle;
}
void NPCAIMgr::SetWander() {
    if (myNPC->GetSpawnMgr() == nullptr)
        return;
    if (m_action == NPCAI::Action::Wandering)
        return;

    _log(NPC__AI_TRACE, "%s(%u): Wandering - Active sight range threshold: %um",
         myNPC->GetName(), myNPC->GetID(), m_sightRange);

    m_action = NPCAI::Action::Wandering;
    SystemBubble* pBubble = myNPC->SysBubble();

    if (pBubble == nullptr) {
        if (m_destiny->IsMoving()) m_destiny->Stop();
        return;
    }

    // =====================================================================
    // DEADSPACE ENCOUNTER CONTAINMENT PATTERN
    // =====================================================================
    if (pBubble->IsAnomaly() || pBubble->IsIncursion() || pBubble->IsMission()) {
        // Combat rats in pockets do not roam. They halt at their spawn nodes
        // and scan for incoming warp arrivals.
        if (m_destiny->IsMoving()) {
            m_destiny->Stop();
        }
        return; // Safe exit point after enforcing action update state!
    }

    // =====================================================================
    // ASTEROID BELT LOOSE ORBIT SEEDING
    // =====================================================================
    if (pBubble->IsBelt()) {
        if (pBubble->HasDynamics()) {
            m_actionTarget = pBubble->GetRandomEntity();
            if (m_actionTarget == nullptr) {
                m_actionTarget = myNPC->SystemMgr()->GetSE(sBubbleMgr.GetBeltID(pBubble->GetID()));
            }

            if (!pBubble->HasPlayers() && m_actionTarget == nullptr) {
                _log(NPC__WARNING, "%s(%u): Wandering - Structural fallback warp required.", myNPC->GetName(), myNPC->GetID());
                WarpOut();
                return;
            }
        } else {
            _log(NPC__AI_TRACE, "%s(%u): Bubble devoid of life assets. Warping out.", myNPC->GetName(), myNPC->GetID());
            WarpOut();
            return;
        }

        if (m_actionTarget == nullptr) return;

        ChangeSpeed(false);
        uint16 orbitDistance = MakeRandomUInt(5000, 15000);

        // FIX: The Destiny engine should track targeted anchors via entityID
        // rather than storing raw C++ memory pointers to protect against segregation faults!
        if (sConfig.npc.UseOrbit) {
            m_destiny->InitOrbit(m_actionTarget, orbitDistance);
            _log(NPC__AI_TRACE, "%s(%u): Initializing ambient orbit ring around %s(%u) at %um.",
                 myNPC->GetName(), myNPC->GetID(), m_actionTarget->GetName(), m_actionTarget->GetID(), orbitDistance);
        } else {
            m_destiny->Follow(m_actionTarget, orbitDistance);
            _log(NPC__AI_TRACE, "%s(%u): Initializing passive path tracking behind %s(%u) at %um.",
                 myNPC->GetName(), myNPC->GetID(), m_actionTarget->GetName(), m_actionTarget->GetID(), orbitDistance);
        }

        m_actionTarget = nullptr; // Clean tracking map isolation flush
        return;
    }

    if (m_destiny->IsMoving()) {
        m_destiny->Stop();
    }
}


void NPCAIMgr::SetIdle() {
    // should we check passive here?
    if ((m_state == NPCAI::State::Idle) and (m_action = NPCAI::Action::Idle))
        return;
    // not doing anything....idle.

    m_actionTarget = nullptr;
    m_attackTarget = nullptr;
    myNPC->TargetMgr()->ClearAllTargets();

    SystemBubble* pBubble = myNPC->SysBubble();
    // if npc was engaged in any action, cancel it here
    if (m_action > NPCAI::Action::Idle) {
        // remove from bubble gfx map
        pBubble->RemoveNPC(myNPC);
    	Heal();
        // cancel other actions if applicable
    }

    //_log(NPC__AI_LOGIC, "SetIdle(); ");
    _log(NPC__AI_TRACE, "%s(%u): SetIdle: returning to idle.", myNPC->GetName(), myNPC->GetID());
    m_state = NPCAI::State::Idle;
    m_action = NPCAI::Action::Idle;
	// do we need to cancel gfx here?
    ChangeSpeed(false);
    m_destiny->Stop();

    ClearAllTimers();

    //disallow warpout checks if anomaly, incursion or mission
    if (pBubble->IsAnomaly() or pBubble->IsIncursion() or pBubble->IsMission())
        return;

    //disallow warpout by NOT setting timer.
    //   should this wait (delay/speed) before test/set?
    if (sConfig.npc.WarpOut > 0)
        if (myNPC->GetSpawnMgr() != nullptr)
            m_warpOutTimer.Start(sConfig.npc.WarpOut * 1000); // s to ms
}

// this one should distinguish between shoot and effect
void NPCAIMgr::SetEngaged(SystemEntity* pTargetSE) {
    // are we switching targets?
    if ((m_state == NPCAI::State::Engaged) and (m_attackTarget == pTargetSE))
        return;

    // actively fighting
    m_warpOutTimer.Disable();

    if (m_attackTarget != pTargetSE) {
        if (myNPC->TargetMgr()->IsTargeting(pTargetSE)) {
            m_attackTarget = pTargetSE;
        } else {
            Target(pTargetSE);
            return;
        }
    }

    //_log(NPC__AI_LOGIC, "SetEngaged(); ");
    _log(NPC__AI_TRACE, "%s(%u): Begin engaging.  Target is %s(%u).  state:%s", \
            myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID(), \
            m_destiny->GetStateName().c_str());

    // this should only be set once when attack begins unless non-repeating gfx is sent on every loop
    if (m_action != NPCAI::Action::Attack)
        m_attackTime = GetFileTimeNow();

    m_state = NPCAI::State::Engaged;
    m_action = NPCAI::Action::Attack;
    SetAttackTimers();

    // update speed
    ChangeSpeed(false);
    if (sConfig.npc.UseOrbit) {
        m_destiny->InitOrbit(pTargetSE, m_optimalRange);  //try to get inside orbit range
    } else {
        m_destiny->Follow(pTargetSE, m_falloffDistance);  //try toget within falloff
    }
}

void NPCAIMgr::SetFollowing(SystemEntity* pTargetSE) {
	// are we switching targets?
    if ((m_state == NPCAI::State::Following) /*and m_destiny->IsFollowing()*/ and (m_attackTarget == pTargetSE))
        return;

    m_warpOutTimer.Disable();

    if (m_attackTarget != pTargetSE) {
        if (!myNPC->TargetMgr()->IsTargeting(pTargetSE)) {
            m_attackTarget = pTargetSE;
        } else {
            Target(pTargetSE);
            return;
        }
    }

    //_log(NPC__AI_LOGIC, "SetFollowing(); ");
    _log(NPC__AI_TRACE, "%s(%u): Begin following.  Target is %s(%u).  state:%s", \
            myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID(), \
            m_destiny->GetStateName().c_str());

    // this should only be set once when attack begins unless non-repeating gfx is sent on every loop
    if (m_action != NPCAI::Action::Attack)
        m_attackTime = GetFileTimeNow();

    m_state = NPCAI::State::Following;
    m_action = NPCAI::Action::Attack;
    SetAttackTimers();

    // outside optimal range; increase speed to get closer
    ChangeSpeed(true);
    m_destiny->Follow(pTargetSE, m_falloffDistance);  //try to get inside falloff range
}

void NPCAIMgr::SetChasing(SystemEntity* pTargetSE) {
	// are we switching targets?   if not, determine if chase delay was set so we can activate full speed
    if ((m_state == NPCAI::State::Chasing) and (m_attackTarget == pTargetSE) and (m_chaseTimeEnd == 0))
        return;

    m_warpOutTimer.Disable();

    bool newTarg(false);
    if (m_attackTarget != pTargetSE) {
        if (myNPC->TargetMgr()->IsTargeting(pTargetSE)) {
            newTarg = true;
            m_attackTarget = pTargetSE;
            // reset chase time
            m_chaseTimeEnd = 0;
        } else {
            Target(pTargetSE);
            return;
        }
    }

    //_log(NPC__AI_LOGIC, "SetChasing(); ");
    _log(NPC__AI_TRACE, "%s(%u): Begin chasing.  Target is %s(%u).  %s target.  state:%s", \
            myNPC->GetName(), myNPC->GetID(), m_attackTarget->GetName(), m_attackTarget->GetID(), \
            newTarg?"new":"same", m_destiny->GetStateName().c_str());

    if (m_action != NPCAI::Action::Attack) {
        if (m_chaseTimeEnd == 0) {
            // check chance for npc to delay activating mwd while chasing
            if (MakeRandomFloat() < m_self->GetAttribute(AttrEntityChaseMaxDelayChance).get_float()) {
                m_state = NPCAI::State::Delay;
                ChangeSpeed(false);
                m_chaseTimeEnd = GetFileTimeNow() + (m_self->GetAttribute(AttrEntityChaseMaxDelay).get_long() * EvE::Time::mSecond);
                _log(NPC__AI_LOGIC, "%s(%u): SetChasing(0) - Delay MWD speed for %llims", \
                        myNPC->GetName(), myNPC->GetID(), m_self->GetAttribute(AttrEntityChaseMaxDelay).get_long());
            } else {
                m_state = NPCAI::State::Chasing;
                ChangeSpeed(true);
                _log(NPC__AI_LOGIC, "%s(%u): SetChasing(1) - Not Delayed - Full speed of %um/s enabled", \
                        myNPC->GetName(), myNPC->GetID(), m_maxSpeed);
            }
        } else {
            m_chaseTimeEnd = 0;
            m_state = NPCAI::State::Chasing;
            ChangeSpeed(true);
            _log(NPC__AI_LOGIC, "%s(%u): SetChasing(2) - Delay Complete. Full speed of %um/s enabled", \
                    myNPC->GetName(), myNPC->GetID(), m_maxSpeed);
        }
    }

    // check distance for possible attack while chasing
    if (InAttackRange(m_attackTarget)) {
        // this should only be set once when attack begins unless non-repeating gfx is sent on every loop
        if ((m_action != NPCAI::Action::Attack) or newTarg)
            m_attackTime = GetFileTimeNow();

        m_action = NPCAI::Action::Attack;
        SetAttackTimers();
    } else if (m_chaseTimeEnd == 0) {
        // if not attacking and chase speed enabled, check and set chase duration
        if (MakeRandomFloat() < m_self->GetAttribute(AttrEntityChaseMaxDurationChance).get_float()) {
            // we are increasing chase duration by a factor of ten.  most are 2.5-10s in db
            m_chaseTimeEnd = GetFileTimeNow() + (m_self->GetAttribute(AttrEntityChaseMaxDuration).get_long() * EvE::Time::mSecond * 10);
        }
    }

    // start chasing / still chasing
    m_destiny->Follow(m_attackTarget, m_flyRange);  //head towards target
}

// not used yet
void NPCAIMgr::SetFleeing(SystemEntity* pTargetSE) {
	// are we switching targets?
    if ((m_state == NPCAI::State::Fleeing) and m_destiny->IsMoving() and (m_attackTarget == pTargetSE))
        return;

    m_warpOutTimer.Disable();

    m_attackTarget = pTargetSE;

    //_log(NPC__AI_LOGIC, "SetFleeing(); ");
    _log(NPC__AI_TRACE, "%s(%u): Begin fleeing.  Target is %s(%u).  state:%s", \
            myNPC->GetName(), myNPC->GetID(), m_attackTarget->GetName(), \
            m_attackTarget->GetID(), m_destiny->GetStateName().c_str());
    // actively fleeing
    m_state = NPCAI::State::Fleeing;
    // reset action before clearing timers
    m_action = NPCAI::Action::Idle;
    ClearAttackTimers();

    //  disengage, then warp out.  << both these will need to be written.
    //  this state is only usable by higher-class npcs.
    ChangeSpeed(true);

    if (sConfig.npc.UseOrbit) {
        m_destiny->InitOrbit(m_attackTarget, m_flyRange);  //try to get ouside orbit range
    } else {
        // we will need a better direction here...like to local stronghold (which is not written yet)   (idea...warpout...see same pc...localchat "you again??")
        // for now, go opposite from target
    	GVector targHeading(pTargetSE->GetPosition(), myNPC->GetPosition());
    	targHeading.normalize();
        const GPoint head(targHeading);
        m_destiny->GotoDirection(head);
    }
}

// not used yet
void NPCAIMgr::SetSignaling(SystemEntity* pTargetSE) {
    //  this state is only usable by higher-class npcs...maybe

    // are we switching targets?
    if ((m_state == NPCAI::State::Signaling) and m_destiny->IsMoving()/*m_destiny->IsOrbiting()*/ and (m_attackTarget == pTargetSE))
        return;

    // local chat for all players in current bubble -->  "there is a distress signal coming from a nearby ship"
    //  will have to figure out how to code this, then test for spam.   maybe put on config
    BcastLocal(m_state);

    m_warpOutTimer.Disable();

    m_attackTarget = pTargetSE;

    //_log(NPC__AI_LOGIC, "SetSignaling(); ");
    _log(NPC__AI_TRACE, "%s(%u): Begin signaling.  Target is %s(%u).", \
            myNPC->GetName(), myNPC->GetID(), m_attackTarget->GetName(), m_attackTarget->GetID());

    m_state = NPCAI::State::Signaling;

    // TODO:  this needs work...
    if (m_action == NPCAI::Action::Passive) {
        // haulers can boost.  see if any of them are still around
        // no clue how to do this yet.
        /*
        if (sConfig.npc.UseOrbit) {
            m_destiny->InitOrbit(hauler, m_falloffDistance);  //try to get inside orbit range
        } else {
            m_destiny->Follow(hauler, m_falloffDistance);
        } */
    }

    //  start speedtanking while signaling.  (im sure this is cheating, but fuckem.)
    ChangeSpeed(true);
    if (sConfig.npc.UseOrbit) {
        m_destiny->InitOrbit(m_attackTarget, m_flyRange);  //try to get outside orbit range
    } else {
        // we will need a better direction here
        m_destiny->Follow(m_attackTarget, m_flyRange);
    }
}

bool NPCAIMgr::InOptimalRange(SystemEntity* pTargetSE) {
    double dist(myNPC->GetPosition().distance(pTargetSE->GetPosition()));
    return (dist < m_optimalRange);
}

bool NPCAIMgr::InFalloffDistance(SystemEntity* pTargetSE) {
    double dist(myNPC->GetPosition().distance(pTargetSE->GetPosition()));
    return (dist < m_falloffDistance);
}

bool NPCAIMgr::InFlyRange(SystemEntity* pTargetSE) {
    double dist(myNPC->GetPosition().distance(pTargetSE->GetPosition()));
    return (dist < m_flyRange);
}

bool NPCAIMgr::InChaseRange(SystemEntity* pTargetSE) {
    double dist(myNPC->GetPosition().distance(pTargetSE->GetPosition()));
    return (dist < m_chaseRange);
}

bool NPCAIMgr::InAttackRange(SystemEntity* pTargetSE) {
    double dist(myNPC->GetPosition().distance(pTargetSE->GetPosition()));
    return (dist < m_attackRange);
}

bool NPCAIMgr::InSightRange(SystemEntity* pTargetSE) {
    double dist(myNPC->GetPosition().distance(pTargetSE->GetPosition()));
    return (dist < m_sightRange);
}

void NPCAIMgr::CheckDistance(SystemEntity* pTargetSE) {
    if (pTargetSE == nullptr)
        return;

    if (is_log_enabled(NPC__TRACE))
        _log(NPC__TRACE, "%s(%u): CheckDistance:  target: %s(%u), state: %s, dist: %.0fm", \
                myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID(), \
                GetStateName(m_state), myNPC->GetPosition().distance(pTargetSE->GetPosition()));

    //TODO:  this needs to be rewritten for better checks.  engage/follow/chase need more thought
    // near to far
    if (InOptimalRange(pTargetSE)) {
        SetEngaged(pTargetSE);
    } else if (InFalloffDistance(pTargetSE)) {
        SetEngaged(pTargetSE);
    } else if (InFlyRange(pTargetSE)) {
        SetFollowing(pTargetSE);
    } else if (InChaseRange(pTargetSE)) {
        SetChasing(pTargetSE);
    } else if (InAttackRange(pTargetSE)) {
        // farthest attack distance for lazors...should we check distance for missiles?
        SetChasing(pTargetSE);
    } else if (InSightRange(pTargetSE)) {
        SetChasing(pTargetSE);
    } else if (myNPC->TargetMgr()->IsTargetedBy(pTargetSE)) {
        _log(NPC__AI_LOGIC, "CheckDistance();  %s(%u) Out of sight but locked", myNPC->GetName(), myNPC->GetID());
        // to far to see, but targlock gives direction, so begin traveling into that direction
        SetChasing(pTargetSE);
    } else {
        // too far away and not locked.
        _log(NPC__AI_LOGIC, "CheckDistance();  %s(%u) Out of sight", myNPC->GetName(), myNPC->GetID());
        SetIdle();
        ClearTarget(pTargetSE);
    }
}

void NPCAIMgr::Target(SystemEntity* pTargetSE) {
    // npc haulers (and maybe others) that dont have weapons do not have locked targets attrib
    if (m_maxLockedTargets < 1)
        return;
    if (pTargetSE == nullptr)
        return;
    if (myNPC->TargetMgr()->IsTargeting(pTargetSE))
        return;

    bool chase(false);
    if (!myNPC->TargetMgr()->StartTargeting(pTargetSE, GetTargetingTime(), m_maxLockedTargets, m_sightRange, chase)) {
        if (!chase) {
            _log(NPC__AI_TRACE, "%s(%u): Targeting of %s(%u) failed and not chasing.  Returning to Idle.", \
                    myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID());
            SetIdle();
            return;
        }
    }

    // new target, reset action and attack timers
    m_action = NPCAI::Action::Idle;
    ClearAttackTimers();
    CheckDistance(pTargetSE);
}

void NPCAIMgr::Targeted(SystemEntity* pTargetSE) {
    if (pTargetSE == nullptr)
        return;

    _log(NPC__AI_TRACE, "%s(%u): Targeted by %s(%u) while %s.", \
            myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID(), GetStateName(m_state));

    switch(m_state) {
        case NPCAI::State::Idle: {
            m_beginFindTarget.Disable();
            Target(pTargetSE);
        } break;

        /** @todo  determine if new targeter entity is weaker than current target. use optimalSigRadius to test for 'preferred' target */
        case NPCAI::State::Delay:
        case NPCAI::State::Engaged:
        case NPCAI::State::Chasing:
        case NPCAI::State::Following: {
        } break;
        // probably do nothing for these
        case NPCAI::State::Fleeing:
        case NPCAI::State::Signaling: {
        } break;
    }
}

void NPCAIMgr::TargetLost(SystemEntity* pTargetSE) {
    if (pTargetSE == nullptr)
        return;

    ClearTarget(pTargetSE);

    if (!myNPC->SysBubble()->HasPlayers())
        return;

    switch(m_state) {
	// may evetually do different things based on current state.
        case NPCAI::State::Idle:
        case NPCAI::State::Delay:
    	case NPCAI::State::Chasing:
    	case NPCAI::State::Engaged:
        case NPCAI::State::Fleeing:
        case NPCAI::State::Following:
        case NPCAI::State::Signaling:
    	case NPCAI::State::WarpOut:
        case NPCAI::State::WarpFollow: {
            _log(NPC__AI_TRACE, "%s(%u): Target %s(%u) lost, but more targets remain.", \
                    myNPC->GetName(), myNPC->GetID(), pTargetSE->GetName(), pTargetSE->GetID());
            PickTarget();
        }
    }
}

void NPCAIMgr::TargetWarping(SystemEntity* pSE) {
    // implement chance for npc to follow warping player
    // sConfig.npc.WarpFollowChance;
    // NPCAI::State::WarpFollow
}


bool NPCAIMgr::VerifyTarget() {
    // there will be more to this here

    if (myNPC->TargetMgr()->HasNoTargets())
        return false;

    if (m_attackTarget->SysBubble() == nullptr) {
        // target has no bubble?  make error
        sLog.Error("NPCAI::VerifyTarget()", "NPCs Target %s(%u) has no bubble", m_attackTarget->GetName(), m_attackTarget->GetID());
        return false;
    }
    if (m_attackTarget->DestinyMgr() == nullptr) {
        sLog.Error("NPCAI::VerifyTarget()", "NPCs Target %s(%u) has no destiny manager.", m_attackTarget->GetName(), m_attackTarget->GetID());
        return false;
    }
    if (m_attackTarget->DestinyMgr()->IsCloaked()) {
        _log(NPC__AI_TRACE, "VerifyTarget() - %s(%u): Target %s(%u) is cloaked.  Clear target and move on",
             myNPC->GetName(), myNPC->GetID(), m_attackTarget->GetName(), m_attackTarget->GetID());
        return false;
    }

    return true;
}

    /*  this will need more thought to finish
        // pick size based on npc size

        Swarm       = 35,
        Frigate     = 50,
        Destroyer   = 100,
        Cruiser     = 200,
        BCruiser    = 300,
        BShip       = 350
*/
void NPCAIMgr::PickTarget() {
    // 1. If we already have targets recorded on our tracker manager, grab the first one
    if (!myNPC->TargetMgr()->HasNoTargets()) {
        m_attackTarget = myNPC->TargetMgr()->GetFirstTarget();
        m_beginFindTarget.Disable();
        return;
    }

    m_attackTarget = nullptr;

    // 2. Fetch active players currently residing inside our local grid bubble
    std::vector<Client*> clientVec;
    myNPC->SysBubble()->GetPlayers(clientVec);
    if (clientVec.empty()) {
        m_beginFindTarget.Disable();
        return;
    }

    ShipSE* bestCandidate = nullptr;
    float highestThreatScore = -1.0f;

    // 3. Single high-speed processing loop (No heavy intermediate maps required)
    for (Client* client : clientVec) {
        if (!client || client->IsInvul()) continue;

        ShipSE* targetShip = client->GetShipSE();
        if (!targetShip || !InSightRange(targetShip)) continue;

        // Pod protection gating matching your original config rules
        if (client->InPod() && sConfig.npc.TargetPod) {
            if (myNPC->SystemMgr()->GetSecurityRating() > sConfig.npc.TargetPodSec) {
                continue;
            }
        }

        DestinyManager* pDestiny = targetShip->DestinyMgr();
        if (!pDestiny || pDestiny->IsCloaked() || pDestiny->IsWarping()) continue;

        // =====================================================================
        // ADVANCED MECHANIC: HIGH-PERFORMANCE THREAT SELECTION
        // =====================================================================
        float targetRadius = targetShip->GetRadius();
        float distance = myNPC->GetPosition().distance(targetShip->GetPosition()); // Calculate range in meters
        if (distance <= 0.0f) distance = 1.0f;

        // Scoring Blueprint:
        // Larger ships are easier to lock onto (Higher Score)
        // Closer ships are immediate threats (Inverse Distance Modifier)
        float currentScore = (targetRadius * 1000.0f) / distance;

        // OPTIMIZATION GATING: Match hull scales cleanly
        // If a Frigate rat matches a Frigate player, give it a 50% threat multiplier boost!
        if (std::abs(targetRadius - m_size) < 50.0f) {
            currentScore *= 1.5f;
        }

        if (currentScore > highestThreatScore) {
            highestThreatScore = currentScore;
            bestCandidate = targetShip;
        }
    }

    // 4. Commit target lock acquisition to the engine components
    if (bestCandidate != nullptr) {
        Target(bestCandidate); // Initiates your clean TargetManager locking pipeline
    }

    m_beginFindTarget.Disable();
}

void NPCAIMgr::ClearTarget(SystemEntity* pTargetSE) {
    myNPC->TargetMgr()->ClearTarget(pTargetSE);

    m_attackTime = 0;

    if (m_attackTarget == pTargetSE)
        m_attackTarget = nullptr;

    if (myNPC->TargetMgr()->HasNoTargets())
        SetIdle();
}

// not used yet (incomplete)
void NPCAIMgr::SwitchTarget() {
    _log(NPC__AI_LOGIC, "SwitchTarget(); ");
    // find a way to get all targets, are we picking by preferred here?   probably so
    std::map<SystemEntity*, TargetEntry*> targets;
    myNPC->TargetMgr()->GetAllTargets(targets);
    std::map<SystemEntity*, TargetedByEntry*> targetby;
    myNPC->TargetMgr()->GetAllTargeters(targetby);
    m_attackTarget = nullptr;
}

void NPCAIMgr::ShipArrived(Client* pClient) {
    // see what we're doing first, then decide from there.
    // most likely states first
    switch (m_state) {
        case NPCAI::State::Passive: {
            // eventually we'll do some tests here, maybe signal.  for now, nothing
            return;
        } break;
        case NPCAI::State::Wandering: {
            // if we're wandering.  set to idle, let Proc() handle the rest
            m_state = NPCAI::State::Idle;
            return;
        } break;
        case NPCAI::State::Idle: {
        } break;
        case NPCAI::State::Engaged: {
        } break;
        case NPCAI::State::Chasing: {
        } break;
        case NPCAI::State::Delay: {
        } break;
        case NPCAI::State::Assisting: {
        } break;
        case NPCAI::State::Fleeing: {
        } break;
        case NPCAI::State::Signaling: {
        } break;
        case NPCAI::State::Following: {
        } break;
        case NPCAI::State::WarpOut: {
        } break;
        case NPCAI::State::WarpFollow: {
        } break;
        case NPCAI::State::Invalid: {
        } break;
    }
    /*   not sure if/when/how action will be used here yet.
    switch (m_action) {
        case NPCAI::Action::Invalid: {
        } break;
        case NPCAI::Action::Wandering: {
        } break;
        case NPCAI::Action::Idle: {
        } break;
        case NPCAI::Action::Attack: {
        } break;
        case NPCAI::Action::Passive: {
        } break;
        case NPCAI::Action::ShieldRep: {
        } break;
        case NPCAI::Action::ArmorRep: {
        } break;
        case NPCAI::Action::Web: {
        } break;
        case NPCAI::Action::Scram: {
        } break;
        case NPCAI::Action::EWar: {
        } break;
    }
    */
}

void NPCAIMgr::DoAction() {
    // all actions here.  check for effect

    // npc do single actions at a time, no matter how many 'modules' they have.   test on creation?
    //   may change this later, if i can code circumstances to do so
    //  maybe test npcs in area for action to avoid dupes (multiple ewar, etc)
    switch (m_action) {
        case NPCAI::Action::Idle: {
            // not doing anything...
            return;
        } break;
        case NPCAI::Action::Attack: {
            if (!myNPC->TargetMgr()->CanAttack())
                return;
            if (m_attackTimer.Check())
                ShootTarget();
            if (m_missileTimer.Check())
                LaunchMissile(m_self->GetAttribute(AttrEntityMissileTypeID).get_uint32(), m_attackTarget);
        } break;
        case NPCAI::Action::ArmorRep: {
            // this can also be remote for advanced npc
            if (m_armorRepairTimer.Check())
                if (MakeRandomFloat() < m_armorRepairDelayChance)
                    myNPC->UseArmorRepairer();
        } break;
        case NPCAI::Action::ShieldRep: {
            // this can also be remote for advanced npc
            if (m_shieldBoosterTimer.Check())
                if (MakeRandomFloat() < m_shieldBoosterDelayChance)
                    myNPC->UseShieldRecharge();
        } break;

        // these do not need re-up on every cycle.  or do they?
        case NPCAI::Action::Web:
        case NPCAI::Action::EWar:
        case NPCAI::Action::Scram: {
            if (!myNPC->TargetMgr()->CanAttack())
                return;
            if (m_actionTimer.Check()) {
                EffectTarget();
            }
        } break;
    }

    // fx is proc when npc created.  data stored on npc item (like modules)  ...may have to change this; cant determine specific data

    // get all npcs effects, check for offensive and attack?
    //  what about healers?  do npc remote rep?   sure, why not??
}

void NPCAIMgr::UseModule() {
    // this will determine what target, what modules are available, check/set timers, call appropriate methods
    _log(NPC__AI_LOGIC, "UseModule(); ");

    // set effectID and m_actionTime and actionTarget here

    // multiple other effect types for npcs.  get from type().HasEffect(fxID)
    // effects are listed in EVE_Effects.h

    // run thru effects to determine other types of attacks this npc can make...no, do that elsewhere.  only set timers here.
    //m_effectMap;

    // fx data will have these attribs...maybe write generic code to use this info instead of multiple checks?
    //sFxDataMgr.GetEffectID();

    /*
    AttrWarpScrambleRange = 103,
    AttrWarpScrambleStrength = 105,
    AttrEntityWarpScrambleChance = 504          // npcUsageChanceAttributeID
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


entityArmorRepairAmount
entityArmorRepairDelayChance					Chance that an entity will delay employing armor repair.
entityArmorRepairDelayChanceLarge
entityArmorRepairDelayChanceLargeMultiplier
entityArmorRepairDelayChanceMedium
entityArmorRepairDelayChanceMediumMultiplier
entityArmorRepairDelayChanceSmall
entityArmorRepairDelayChanceSmallMultiplier
entityArmorRepairDuration
entityAttackDelayMax
entityAttackDelayMin
entityAttackRange
entityBluePrintDropChance
entityBracketColour								so far: 0 = white, 1 = red
entityCapacitorDrainAmount
entityCapacitorDrainDuration					Duration of NPC effect
entityCapacitorDrainDurationChance				Chance of NPC effect to be activated each duration
entityCapacitorDrainMaxRange
entityCapacitorFallOff
entityChaseMaxDelay
entityChaseMaxDelayChance
entityChaseMaxDistance
entityChaseMaxDuration
entityChaseMaxDurationChance
entityConvoyDroneMax							Maximum number of convoy drones a convoy can have for proetcion.
entityConvoyDroneMin
entityCruiseSpeed
entityCruiseSpeedMultiplier
entityDefenderChance							% chance of entity to shoot defender at incoming missile
entityDroneCount
		TODO:  add this to loot code...
entityEquipmentGroupMax							The maximum drops of same group (example: entity can only drop 1 of group: energy laser)
entityEquipmentMax
entityEquipmentMin
entityFactionLoss
entityFlyRange
entityFlyRangeFactor
entityFlyRangeMultiplier
entityFollowRange
entityGroupArmorResistanceActivationChance		Activation chance for NPCGroupArmorAssist effect.
entityGroupArmorResistanceBonus				Amount of armor resistance bonus. Used by NPCGroupArmorAssist. Negative values is a bonus so e.g. -20 is a 20% bonus
entityGroupArmorResistanceDuration
entityGroupPropJamActivationChance
entityGroupPropJamBonus
entityGroupPropJamDuration
entityGroupRespawnChance
entityGroupShieldResistanceActivationChance
entityGroupShieldResistanceBonus
entityGroupShieldResistanceDuration
entityGroupSpeedActivationChance
entityGroupSpeedBonus
entityGroupSpeedDuration
entityKillBounty
entityLootCountMax
entityLootCountMin
entityLootValueMax
entityLootValueMin
entityMaxVelocitySignatureRadiusMultiplier		Used to increase signature radius of entity when it activates Max Velocity. Used to fake MWD sig radius increase.
entityMaxWanderRange
entityMissileTypeID					The type of missiles the entity launches.
entityOverviewShipGroupId				This attribute is used on entities to link them to a player ship group. This is then used to determine which overview icon they should get, among other things
entityReactionFactor
entityRemoteECMBaseDuration
entityRemoteECMChanceOfActivation
entityRemoteECMDuration
entityRemoteECMDurationScale
entityRemoteECMExtraPlayerScale
entityRemoteECMIntendedNumPlayers
entityRemoteECMMinDuration
entitySecurityMaxGain
entitySecurityStatusAggressionBonus
entitySecurityStatusKillBonus
entitySensorDampenDuration
entitySensorDampenDurationChance			Chance of NPC effect to be activated each duration
entitySensorDampenFallOff
entitySensorDampenMaxRange
entitySensorDampenMultiplier
entityShieldBoostAmount
entityShieldBoostDelayChance				The chance an entity will delay repeating use of its shield boosting effect if it has one.
entityShieldBoostDelayChanceLarge
entityShieldBoostDelayChanceLargeMultiplier
entityShieldBoostDelayChanceMedium
entityShieldBoostDelayChanceMediumMultiplier
entityShieldBoostDelayChanceSmall
entityShieldBoostDelayChanceSmallMultiplier
entityShieldBoostDuration				How long between repeats.
entityShieldBoostLargeDelayChance
entityStrength						A relative strength that indicates how powerful this NPC entity is in combat.
entityTargetJam
entityTargetJamDuration
entityTargetJamDurationChance				Chance of NPC effect to be activated each duration
entityTargetJamFallOff
entityTargetJamMaxRange
entityTargetPaintDuration
entityTargetPaintDurationChance				Chance of NPC effect to be activated each duration
entityTargetPaintFallOff
entityTargetPaintMaxRange
entityTargetPaintMultiplier
entityTrackingDisruptDuration
entityTrackingDisruptDurationChance			Chance of NPC effect to be activated each duration
entityTrackingDisruptFallOff
entityTrackingDisruptMaxRange
entityTrackingDisruptMultiplier
entityWarpScrambleChance				Chance of entity warp scrambling it's target.

    */

}

// working code copied from droneAI and modified for NPC.
void NPCAIMgr::SendGFX(Client* pClient /*nullptr*/) {
    // TODO:  make note that this sends start/stop based on NPC's current action status.
    if (m_effectID < 1) {
        // not necessarily an error.  just make note
        _log(NPC__WARNING, "SendGFX(): m_effectID < 1 for typeID:%u (%s/%u).", \
                myNPC->GetTypeID(), myNPC->GetName(), myNPC->GetID());
        return;
    }

    int32 repeat(0);
    bool active(false), start(false);
    if (m_action > NPCAI::Action::Wandering) {
        start = true;
        active = true;
        repeat = 20000;
    }

    bool sendTurret(false);
    int32 duration(m_attackSpeed);
    int64 startTime(m_attackTime);
    SystemEntity* pTargetSE(m_attackTarget);

    // or can use (m_action > attack) here
    switch (m_effectID) {
        case EvE::GFXID::targetAttack:
        case EvE::GFXID::projectileFired:
        case EvE::GFXID::projectileFiredForEntities: {
            sendTurret = true;
        } break;
        default: {
            // not pewpew - set data for action
            duration = m_actionSpeed;
            startTime = m_actionTime;
            pTargetSE = m_actionTarget;
        } break;
    }

    if (active and (pTargetSE == nullptr)) {
        // not necessarily an error.  just make note
        _log(NPC__WARNING, "SendGFX(): %s(%u) - %s and pTargetSE == nullptr.", \
                myNPC->GetName(), myNPC->GetID(),  GetStateName(m_state));
    }

    std::string guidStr = sFxDataMgr.GetEffectGuid(m_effectID);
    if (is_log_enabled(NPC__MESSAGE)) {
        if (active) {
            sLog.Green("NPCAI", "starting %s(%u) GFX for %s(%u);  repeat:%i", \
                guidStr.c_str(), m_effectID, myNPC->GetName(), myNPC->GetID(), repeat);
        } else {
            sLog.Error("NPCAI", "stopping %s(%u) GFX for %s(%u)", \
                guidStr.c_str(), m_effectID, myNPC->GetName(), myNPC->GetID());
        }
    }

        /*  still working on these...(from activemodule code)
         * !start - remove effect
         * start and !active - start ONE-SHOT event of (duration) - SuperWeapon only
         * repeat > 0 - start REPEAT event of <duration>
         * !repeat - start TOGGLE event (turn off with !start)
         */
    OnSpecialFX14 effect;
        effect.entityID         = myNPC->GetID();
        effect.moduleID         = myNPC->GetID();                       // npc UID
        effect.moduleTypeID     = myNPC->GetTypeID();                   // npc typeID
        effect.targetID         = (pTargetSE == nullptr ? PyStatic.NewNone() : new PyInt(pTargetSE->GetID()));
        effect.otherTypeID      = PyStatic.NewNone();                   // charge typeID
        effect.area             = PyStatic.mtList();                    // no data.  not used in client
        effect.guid             = std::move(guidStr);                   // based on m_effectID  (effect.xxx)
        effect.isOffensive      = sFxDataMgr.isOffensive(m_effectID);   // bool
        effect.start            = start;                                // int bool
        effect.active           = active;                               // int bool
        effect.duration         = duration;                             // in ms
        effect.repeat           = repeat;                               // npc repeat? yes
        effect.startTime        = startTime;
        effect.graphicInfo      = (sendTurret ? new PyInt(m_turretID) : PyStatic.NewNone());
    PyTuple *up = effect.Encode();
    if (is_log_enabled(EFFECTS__DUMP_NPC))
        up->Dump(EFFECTS__DUMP_NPC, "");

    //  this is for fx that dont have to resend on every cycle (reppers/tackle)
    if (pClient == nullptr) {
        myNPC->SysBubble()->BubblecastDestinyUpdate(&up, "NPC GFX Bcast");
    } else {
        // this is to update new ship in bubble with active gfx or all ships with gfx that send on every cycle
        pClient->QueueDestinyUpdate(&up);
    }
    PyDecRef(up);
}

void NPCAIMgr::ReportDamage(uint8 type, SystemEntity* pSourceSE) {
// beginnings of advanced AI for all npcs

    _log(NPC__AI_LOGIC, "ReportDamage: %s(%u) has %s.", \
            myNPC->GetName(), myNPC->GetID(), sDataMgr.GetDmgRptName(type));

    if (m_state == NPCAI::State::Chasing) {
        if ((pSourceSE != nullptr) and (m_attackTarget == pSourceSE)) {
            // npc taking damage from chase target.  cancel duration timer
            m_attackTime = 0;
            // so currently we are ignoring damage taken while chasing...maybe call help after x amt damage?
            return;
        }
    }
    // these will (eventually) have to test for available "module"
    switch (type) {
        case Dmg::Type::None:
        case Dmg::Type::HullHalf: {
            // nothing here yet... no npcs have hull reppers
        } break;
        case Dmg::Type::ArmorHalf:
        case Dmg::Type::ArmorZero: {
            if (m_action == NPCAI::Action::Passive) {
                // call for more help
                SetSignaling(pSourceSE);
                return;
            }
            // if npc cant rep, this will never hit
            if (!m_armorRepairTimer.Enabled() and (m_armorRepairDuration > 0)) {
                //TODO:  need to know what fxID they using here.
                m_effectID = EvE::GFXID::armorRepair;    // this is hacked for now.
                m_actionSpeed = m_armorRepairDuration;
                m_actionTarget = myNPC;
                m_actionTime = GetFileTimeNow();
                // update to target self
                SendGFX();
                m_armorRepairTimer.Start(m_armorRepairDuration);
            }
        } break;
        case Dmg::Type::ShieldHalf:
        case Dmg::Type::ShieldZero: {
            // if we were chasing on a timer, negate timer
            m_chaseTimeEnd = 0;

            if (m_action == NPCAI::Action::Passive) {
                // call for help
                SetSignaling(pSourceSE);
                // most passives can boost, so fall thru to set timer
            }
            // if npc cant boost, this will never hit
            if (!m_shieldBoosterTimer.Enabled() and (m_shieldBoosterDuration > 0)) {
                // is this self or remote or both?   to test...
                //TODO:  need to know what fxID they using here.
                m_effectID = EvE::GFXID::shieldBoosting;  //this is hacked for now.
                m_actionSpeed = m_shieldBoosterDuration;
                m_actionTarget = myNPC;
                m_actionTime = GetFileTimeNow();
                SendGFX();
                m_shieldBoosterTimer.Start(m_shieldBoosterDuration);
            }
        } break;
    }

    // if currently shooting, reset turretID and fxID
    /*  this isnt gonna work right///
    if (m_attackTimer.Enabled())
        m_effectID = sFxDataMgr.GetDefault(m_self->typeID());
    */
}

void NPCAIMgr::ShootTarget() {
    Damage d(myNPC,
             m_self,
             myNPC->GetKinetic(),
             myNPC->GetThermal(),
             myNPC->GetEM(),
             myNPC->GetExplosive(),
             m_formula.GetNPCToHit(myNPC, m_attackTarget)
            );

    if (sConfig.npc.UseDamageMultiplier)
        d *= m_damageMultiplier;

    m_attackTarget->ApplyDamage(d);
}

void NPCAIMgr::EffectTarget() {
    //  apply 'module' effects to target

}

// not used yet
void NPCAIMgr::Guard(SystemEntity* pTargetSE) {
    // this will be used by escorts in hauler spawn
    _log(NPC__AI_LOGIC, "Guard(); ");
}

// not used yet
void NPCAIMgr::Assist(SystemEntity* pTargetSE) {
    // relocate after completion
    // this will determine what is needed and call UseModule()
    _log(NPC__AI_LOGIC, "Assist(); ");

}

// not used yet
// this is called from ?
void NPCAIMgr::Heal() {
    // relocate after completion
    _log(NPC__AI_LOGIC, "Heal(); ");

    //NPCRemoteArmorRepair =   3852,     // effects.RemoteArmourRepair
    //NPCRemoteShieldBoost =   3855,     // effects.ShieldTransfer

    // put chance to use in here too
    if (m_shieldBoosterTimer.Enabled())
        if (m_shieldBoosterTimer.Check())
            if (MakeRandomFloat() < m_shieldBoosterDelayChance)
                myNPC->UseShieldRecharge();

    if (m_armorRepairTimer.Enabled())
        if (m_armorRepairTimer.Check())
            if (MakeRandomFloat() < m_armorRepairDelayChance)
                myNPC->UseArmorRepairer();

}

void NPCAIMgr::LaunchMissile(uint16 typeID, SystemEntity* pTargetSE) {
    if ((typeID == 0) or (pTargetSE == nullptr))
        return;
    // create and Launch a missile; start flying
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
    if (typeID != EVEDB::invTypes::DefenderI)  // but only if it's NOT a defender missile  (their target is another missile)
        pTargetSE->MissileLaunched(pMissile);
}

void NPCAIMgr::MissileLaunched(Missile* pMissile) {
    float chance = m_self->GetAttribute(AttrEntityDefenderChance).get_float();
    if (sConfig.npc.DefenderMissileChance)
        chance += sConfig.npc.DefenderMissileChance;
    // check chance to shoot defender missile at incoming missile (working, ??/??/??)
    if (MakeRandomFloat() < chance)
        LaunchMissile(EVEDB::invTypes::DefenderI, pMissile); // defender missile
    // TODO:  set and test for this every x {timeframe}?   no attribute for it
}

uint16 NPCAIMgr::GetTargetingTime() {
    uint16 targetTime = (m_self->GetAttribute(AttrScanSpeed).get_uint32());
    // if target time not defined (~1/2 are), use npc size to set lock time
    if (targetTime < 1500) {
        // min npc scan speed in db is 2000
        switch (m_size) {
            case NPCAI::Size::Swarm: {
                targetTime = 1500;
            } break;
            case NPCAI::Size::Frigate: {
                targetTime = 2000;
            } break;
            case NPCAI::Size::Destroyer: {
                targetTime = 2500;
            } break;
            case NPCAI::Size::Cruiser: {
                targetTime = 3500;
            } break;
            case NPCAI::Size::BCruiser: {
                targetTime = 4500;
            } break;
            case NPCAI::Size::BShip: {
                targetTime = 6000;
            } break;
        }
    }

    return targetTime;

/* old  ~2008
	  (10000/D4) / ASINH(D5)

   new  ~2011
      40000 / (D4 * arcsinh(D5)^2)


	Where D4 is Scan Resolution
	Where D5 is Target Signature Radius.
*/
}

void NPCAIMgr::DisableRepTimers(bool shield/*true*/, bool armor/*true*/) {
    if (shield) {
        // get right fxID once coded
        m_effectID = EvE::GFXID::shieldBoosting;  //this is hacked for now.
        m_actionSpeed = -1;
        SendGFX();
        m_shieldBoosterTimer.Disable();
    }

    // only 2 named angel rats do armor rep; they also do boost
    if (armor) {
        m_effectID = EvE::GFXID::armorRepair;  //this is hacked for now.
        m_actionSpeed = -1;
        SendGFX();
        m_armorRepairTimer.Disable();
    }

    m_effectID = 0;

    // remove from bubble gfx map
    myNPC->SysBubble()->RemoveNPC(myNPC);

    /*  this isnt gonna work right
    // if currently shooting, reset turretID and fxID
    if (m_attackTimer.Enabled())
        m_effectID = sFxDataMgr.GetDefault(m_self->typeID());
    */
}

void NPCAIMgr::SetAttackTimers() {
    //TODO:  find a better way to engage missiles.  they have longer range than turrets
    // missiles are different from other attack types.  set these first in case of early return on timer check
    if (!m_missileTimer.Enabled() and m_self->HasAttribute(AttrEntityMissileTypeID) and m_self->HasAttribute(AttrMissileLaunchDuration))
        m_missileTimer.Start(m_self->GetAttribute(AttrMissileLaunchDuration).get_uint32());

    if (m_attackTimer.Enabled()) {
        _log(NPC__AI_LOGIC, "SetAttackTimers() - %s(%u) m_attackTimer already enabled for fxid %u with %ums left.", \
                myNPC->GetName(), myNPC->GetID(), m_effectID, m_attackTimer.GetRemainingTime());
        return;
    }

    switch (m_effectID) {
        case EvE::GFXID::targetAttack:
        case EvE::GFXID::projectileFired:
        case EvE::GFXID::projectileFiredForEntities: {  // effects.laser
            m_attackSpeed = m_self->GetAttribute(AttrSpeed).get_int();
        } break;

        default: {
            _log(NPC__WARNING, "SetAttackTimers() - %s(%u) has no effectID", myNPC->GetName(), myNPC->GetID());
            return;
        } break;
    }

    // set timer
    m_attackTimer.Start(m_attackSpeed);
    SendGFX();

    _log(NPC__AI_LOGIC, "SetAttackTimers() - %s(%u) m_attackTimer set to %ims.  fxid: %u(%s), turretID: %u", \
            myNPC->GetName(), myNPC->GetID(), m_attackSpeed, m_effectID, \
            sFxDataMgr.GetEffectName(m_effectID).c_str(), m_turretID);
}

// not used yet.  will need more thought
void NPCAIMgr::SetActionTimers() {
    // model after SetAttackTimers()
    if (m_effectID == 0)
        m_effectID = 0;  // may have to hard-code these

    // may get complicated based on npc abilities
    if (m_actionTimer.Enabled()) {
        _log(NPC__AI_LOGIC, "SetActionTimers() - %s(%u) m_actionTimer already enabled for fxid %u with %ums left.", \
                myNPC->GetName(), myNPC->GetID(), m_effectID, m_attackTimer.GetRemainingTime());
        return;
    }

    m_actionTime = GetFileTimeNow();

    //  effectID nameToID found in EvE::GFXID::<enum>
    //  names are effect.name in m_effectMap
    switch (m_effectID) {
        // these 2 are hacked for testing (still wip)
        case EvE::GFXID::shieldBoosting: 	// 4 = effects.shieldBoost(self-boost)
        case EvE::GFXID::armorRepair: {	// 27 = effects.armorRep(self-rep)
            // these should not hit here...separate timer
            _log(NPC__AI_LOGIC, "SetActionTimers() - %s(%u) self-heal called for action.  fxid: %u", \
                    myNPC->GetName(), myNPC->GetID(), m_effectID);
            EvE::traceStack();
            return;
        } break;

        // the following are possibles from rat fx map
        case EvE::GFXID::entityShieldBoostingSmall:
        case EvE::GFXID::entityShieldBoostingMedium:
        case EvE::GFXID::entityShieldBoostingLarge: {
            m_actionSpeed = m_shieldBoosterDuration;
        } break;

        case EvE::GFXID::entityArmorRepairingSmall:
        case EvE::GFXID::entityArmorRepairingMedium:
        case EvE::GFXID::entityArmorRepairingLarge: {
            m_actionSpeed = m_armorRepairDuration;
        } break;
        // these are from angel roid rats.  there may be more fx for other factions
        case EvE::GFXID::warpScrambleForEntity:  {
            m_actionSpeed = m_self->GetAttribute(AttrWarpScrambleDuration).get_int();
        } break;
        case EvE::GFXID::modifyTargetSpeed2: {
            m_actionSpeed = m_self->GetAttribute(AttrModifyTargetSpeedDuration).get_int();
        } break;
        case EvE::GFXID::entityCapacitorDrain: {
            m_actionSpeed = m_self->GetAttribute(AttrEntityCapacitorDrainDuration).get_int();
        } break;
        case EvE::GFXID::entityTargetPaint: {
            m_actionSpeed = m_self->GetAttribute(AttrEntityTargetPaintDuration).get_int();
        } break;
        case EvE::GFXID::entityTrackingDisruptOld: {
            m_actionSpeed = m_self->GetAttribute(AttrEntityTrackingDisruptDuration).get_int();
        } break;
        /*  this one doesnt exist??
        case EvE::GFXID::entityTargetJam: {
            m_actionSpeed = m_self->GetAttribute(AttrEntityTargetJamDuration).get_int();
        } break;
        */
        case EvE::GFXID::entitySensorDampen: {
            m_actionSpeed = m_self->GetAttribute(AttrEntitySensorDampenDuration).get_int();
        } break;
        case EvE::GFXID::remoteEcmBurst: {
            m_actionSpeed = m_self->GetAttribute(AttrentityRemoteECMDuration).get_int();
        } break;

        // these are all i found, but i could have missed some
    }

    // set timer
    m_actionTimer.Start(m_actionSpeed);

    _log(NPC__AI_LOGIC, "SetActionTimers() - %s(%u) m_actionTimer set to %ims.  fxid: %u(%s), turretID: %u", \
            myNPC->GetName(), myNPC->GetID(), m_actionSpeed, m_effectID,  \
            sFxDataMgr.GetEffectName(m_effectID).c_str(), m_turretID);

    //  these may repeat.  to test once coded
    SendGFX();
}

// TODO:  make note on this method to have new action set BEFORE calling because this calls SendGFX() which sends start/stop based on NPC's current action status
void NPCAIMgr::ClearAttackTimers() {
    if (m_actionTimer.Enabled()) {
        // this was active.  cancel gfx
        //TODO: this will need update to cancel right gfx once coded
        SendGFX();
        m_effectID = 0;
    }

    m_attackSpeed = 0;
    m_chaseTimeEnd = 0;
    m_actionTimer.Disable();
    m_attackTimer.Disable();
    m_missileTimer.Disable();
    m_beginFindTarget.Disable();
    // remove from bubble gfx map
    myNPC->SysBubble()->RemoveNPC(myNPC);
}

// TODO:  make note on this method to have new action set BEFORE calling because this calls SendGFX() which sends start/stop based on NPC's current action status
void NPCAIMgr::ClearAllTimers() {
    // only called by SetIdle()
    if (m_actionTimer.Enabled()) {
        // this was active.  cancel gfx
        //SendGFX();
        m_effectID = 0;
    }
    m_attackSpeed = 0;
    m_chaseTimeEnd = 0;
    m_actionTimer.Disable();
    m_attackTimer.Disable();
    m_missileTimer.Disable();
    m_warpOutTimer.Disable();
    m_beginFindTarget.Disable();
    m_armorRepairTimer.Disable();
    m_shieldBoosterTimer.Disable();
    // remove from bubble gfx map
    myNPC->SysBubble()->RemoveNPC(myNPC);
}

void NPCAIMgr::ChangeSpeed(bool increase/*false*/) {
    // helper method to add/remove mwd speed on npcs
    float sigRad(m_self->GetAttribute(AttrSignatureRadius).get_float()); //552
    uint32 speed(m_cruiseSpeed);

    if (increase) {
        //  increase sig radius and speed
        switch (m_state) {
            case NPCAI::State::Chasing: {
                speed = m_maxSpeed;
            } break;
            case NPCAI::State::Fleeing: {
                speed = m_maxSpeed / 2;
            } break;
            case NPCAI::State::Following: {
                speed = m_maxSpeed / 4;
            } break;
            case NPCAI::State::Signaling: {
                speed = m_cruiseSpeed * 2;
            } break;
        }

        // modify sigRad based on set speed (full mwd speed = full modifier)
        float multiplier(m_self->GetAttribute(AttrEntityMaxVelocitySignatureRadiusMultiplier).get_float());
        multiplier *= (speed / m_maxSpeed);
        sigRad *= (1.0f + multiplier);
    }

    m_destiny->SetMaxVelocity(speed);
    m_self->SetAttribute(AttrSignatureRadius, sigRad, false);
}

void NPCAIMgr::BcastLocal(uint8 state) {
    // npc will broadcast to client's local chat miscellaneous messages

    // get clients in bubble
    std::vector<Client*> clientVec;
    myNPC->SysBubble()->GetPlayers(clientVec);

    // determine msg to send

    // send msg to local chat channel of all clients in bubble

}

const char* NPCAIMgr::GetStateName(int8 stateID) {
    switch (stateID) {
        case NPCAI::State::Invalid:        return "Invalid";
        case NPCAI::State::Passive:        return "Passive";
        case NPCAI::State::Delay:          return "MWD Delay";
        case NPCAI::State::Wandering:      return "Wandering";
        case NPCAI::State::Idle:           return "Idle";
        case NPCAI::State::Chasing:        return "Chasing";
        case NPCAI::State::Following:      return "Following";
        case NPCAI::State::Engaged:        return "Engaged";
        case NPCAI::State::Fleeing:        return "Fleeing";
        case NPCAI::State::Signaling:      return "Signaling";
        case NPCAI::State::WarpOut:        return "Warping Out";
        case NPCAI::State::WarpFollow:     return "Following Warp";
        default:                           return "Invalid";
    }
}

/*
 switch (m_state) {
        case NPCAI::State::Invalid: {
            } break;
        case NPCAI::State::Passive: {
            } break;
        case NPCAI::State::Wandering: {
            } break;
        case NPCAI::State::Assisting: {
            } break;
        case NPCAI::State::Idle: {
    	} break;
        case NPCAI::State::Delay: {
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

    switch (m_action) {
        case NPCAI::Action::Invalid: {
            } break;
        case NPCAI::Action::Wandering: {
            } break;
            case NPCAI::Action::Idle: {
            } break;
            case NPCAI::Action::Attack: {
            } break;
            case NPCAI::Action::Passive: {
            } break;
            case NPCAI::Action::ShieldRep: {
            } break;
            case NPCAI::Action::ArmorRep: {
            } break;
            case NPCAI::Action::Web: {
            } break;
            case NPCAI::Action::Scram: {
            } break;
            case NPCAI::Action::EWar: {
            } break;
        }

 */

/*
	//
 */

/*
        switch (m_size) {
            case NPCAI::Size::Swarm: {
            } break;
            case NPCAI::Size::Frigate: {
            } break;
            case NPCAI::Size::Destroyer: {
            } break;
            case NPCAI::Size::Cruiser: {
            } break;
            case NPCAI::Size::BCruiser: {
            } break;
            case NPCAI::Size::BShip: {
            } break;
        }
*/

/*  these werent right.  testing diff way

    float radius = m_self->radius();
    // d31,d45,dn50,dn150,d250,dn350
    if (radius < 35.0f) {
        // only rogue drones
        m_size = NPCAI::Size::Swarm;
    } else if (radius < 55.0f) {
        m_size = NPCAI::Size::Frigate;
    } else if (radius < 170.0f) {
        m_size = NPCAI::Size::Destroyer;
    } else if (radius < 270.0f) {
        m_size = NPCAI::Size::Cruiser;
    } else if (radius < 350.0f) {
        m_size = NPCAI::Size::BCruiser; //22824
    } else { // 350+
        m_size = NPCAI::Size::BShip;
    }
*/