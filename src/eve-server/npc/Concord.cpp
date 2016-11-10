/**
 * @name Concord.cpp
 *   this file is for Concord AI and other concord-specific code as it is written.
 *
 * @Author:         Allan
 * @date:   09 March 2016
 */

/*  concord response time
 * sysSec - no Spawn : existing Spawns
 * 1.0 - 06:12
 * 0.9 - 07:13
 * 0.7 - 08:14
 * 0.6 - 10:16
 * 0.5 - 13:19
 * 0.3 - 15:20
 * 0.1 - 20:30
 */

/*  note.....this taken straight from NPC */

#include "Concord.h"
#include "EVEServerConfig.h"
#include "inventory/AttributeEnum.h"
#include "system/DestinyManager.h"
#include "system/Damage.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"


Concord::Concord(
     SystemManager* system,
     PyServiceMgr& services,
     InventoryItemRef self,
     const GPoint& position,
     ConcordSpawnMgr* spawnMgr )
 : DynamicSystemEntity(new DestinyManager(this, system), self),
 m_system(system),
 m_services(services),
 m_spawnMgr(spawnMgr),
 m_orbitingID(0)
 {
     m_corpID = corpCONCORD;
     m_allyID = 0;
     m_warID = factionCONCORD;
     m_destiny = new DestinyManager(this);
     m_AI = new ConcordAI(this);

     m_orbitRange = m_self->GetAttribute(AttrOrbitRange).get_int();
     if (!m_orbitRange) {
         if (m_self->GetAttribute(AttrMaxRange) < m_self->GetAttribute(AttrFalloff))
             m_orbitRange = m_self->GetAttribute(AttrMaxRange).get_float();
         else
             m_orbitRange = m_self->GetAttribute(AttrFalloff).get_float();
     }

     // Create default dynamic attributes in the AttributeMap:
     m_self->SetAttribute(AttrDamage,              0, false);
     m_self->SetAttribute(AttrArmorDamage,         0, false);
     m_self->SetAttribute(AttrInertia,             1, false);
     m_self->SetAttribute(AttrWarpCapacitorNeed,   0.00001, false);
     m_self->SetAttribute(AttrOrbitRange,          m_orbitRange, false);
     m_self->SetAttribute(AttrMass,                m_self->type().mass(), false);
     m_self->SetAttribute(AttrRadius,              m_self->type().radius(), false);
     m_self->SetAttribute(AttrVolume,              m_self->type().volume(), false);
     m_self->SetAttribute(AttrCapacity,            m_self->type().capacity(), false);
     m_self->SetAttribute(AttrShieldCharge,        m_self->GetAttribute(AttrShieldCapacity), false);
     m_self->SetAttribute(AttrCapacitorCharge,     m_self->GetAttribute(AttrCapacitorCapacity), false);

     m_destiny->SetShipCapabilities(m_self);

     SetResists();

     /* Gets the value from the NPC and put on our own vars */
     m_emDamage = m_self->GetAttribute(AttrEmDamage).get_float(),
     m_kinDamage = m_self->GetAttribute(AttrKineticDamage).get_float(),
     m_therDamage = m_self->GetAttribute(AttrThermalDamage).get_float(),
     m_expDamage = m_self->GetAttribute(AttrExplosiveDamage).get_float(),
     m_hullDamage = m_self->GetAttribute(AttrDamage).get_float();
     m_armorDamage = m_self->GetAttribute(AttrArmorDamage).get_float();
     m_shieldCharge = m_self->GetAttribute(AttrShieldCharge).get_float();
     m_shieldCapacity = m_self->GetAttribute(AttrShieldCapacity).get_float();

     //_log(CONCORD__TRACE, "Created Concord Police object for %s (%u)", self.get()->itemName().c_str(), self.get()->itemID());
}

Concord::~Concord() {
    SafeDelete(m_destiny);
    SafeDelete(m_AI);
}

void Concord::Process() {
    double profileStartTime = 0.0;
    if (sConfig.server.UseProfiling)
        profileStartTime = GetTimeUSeconds();

    /*  Enable base call to Process Targeting and Movement  */
    SystemEntity::Process();
    m_AI->Process();

    if (sConfig.server.UseProfiling)
        sProfile.AddTime(_concordProfile, GetTimeUSeconds() - profileStartTime);
}

void Concord::Orbit(SystemEntity *who) {
    if (!who)
        m_orbitingID = 0;
    else
        m_orbitingID = who->GetID();
}

void Concord::TargetLost(SystemEntity *who) {
    m_AI->TargetLost(who);
}

void Concord::TargetedAdd(SystemEntity *who) {
    m_AI->Targeted(who);
}

void Concord::EncodeDestiny( Buffer& into ) const
{
    using namespace Destiny;

    uint8 mode = DSTBALL_STOP;
    if (m_destiny->IsWarping())
        mode = DSTBALL_WARP;
    else if (m_destiny->IsFollowing())
        mode = DSTBALL_FOLLOW;
    else if (m_destiny->IsOrbiting())
        mode = DSTBALL_ORBIT;
    else if (m_destiny->IsMoving())
        mode = DSTBALL_GOTO;

    BallHeader head;
    head.entityID = GetID();
    head.mode = mode;
    head.radius = GetRadius();
    head.x = x();
    head.y = y();
    head.z = z();
    head.flags = IsMassive | IsFree;
    into.Append( head );
    MassSector mass;
    mass.mass = m_destiny->GetMass();
    mass.cloak = 0;
    mass.Harmonic = -1.0f;
    mass.corporationID = GetCorporationID();
    mass.allianceID = GetAllianceID();
    into.Append( mass );
    DataSector data;
    data.maxVelocity = m_destiny->GetMaxVelocity();
    data.velocity_x = m_destiny->GetVelocity().x;
    data.velocity_y = m_destiny->GetVelocity().y;
    data.velocity_z = m_destiny->GetVelocity().z;
    data.intertia = m_destiny->GetInertia();
    data.speedfraction = m_destiny->GetSpeedFraction();
    into.Append( data );
    if (mode == DSTBALL_WARP) {
        GPoint target = m_destiny->GetTargetPoint();
        DSTBALL_WARP_Struct warp;
        warp.formationID = 0xFF;
        warp.effectStamp = m_destiny->GetStateStamp();   //timestamp when warp started
        warp.x = target.x;
        warp.y = target.y;
        warp.z = target.z;
        warp.ownerID = m_destiny->GetWarpSpeed();       //ship warp speed x10  (dont ask...this is what it is...more dumb ccp shit)
        warp.followRange = 0;
        warp.followID = (m_destiny->GetTargetID() ? m_destiny->GetTargetID() : 0);
        into.Append( warp );
    } else if (mode == DSTBALL_FOLLOW) {
        DSTBALL_FOLLOW_Struct follow;
        follow.followID = m_destiny->GetTargetID();
        follow.followRange = m_destiny->GetFollowDistance();
        follow.formationID = 0xFF;
        into.Append( follow );
    } else if (mode == DSTBALL_ORBIT) {
        DSTBALL_ORBIT_Struct orbit;
        orbit.followID = m_destiny->GetTargetID();
        orbit.followRange = m_destiny->GetFollowDistance();
        orbit.formationID = 0xFF;
        into.Append( orbit );
    } else if (mode == DSTBALL_GOTO) {
        GPoint target = m_destiny->GetTargetPoint();
        DSTBALL_GOTO_Struct go;
        go.formationID = 0xFF;
        go.x = target.x;
        go.y = target.y;
        go.z = target.z;
        into.Append( go );
    } else {
        DSTBALL_STOP_Struct main;
        main.formationID = 0xFF;
        into.Append( main );
    }

    _log(COMMON__WARNING, "NPC::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}


void Concord::MakeDamageState(DoDestinyDamageState &into) const {
    into.shield = m_shieldCharge / m_self->GetAttribute(AttrShieldCapacity).get_float();
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float() +8;
    into.timestamp = Win32TimeNow();
    into.armor = 1.0 - (m_armorDamage / m_self->GetAttribute(AttrArmorHP).get_float());
    into.structure = 1.0 - (m_hullDamage / m_self->GetAttribute(AttrHP).get_float());
}

void Concord::UseShieldRecharge()
{
    // We recharge our shield until it's reaches the shield capacity.
    if (GetSelf()->GetAttribute(AttrShieldCapacity) > m_shieldCharge)
    {
        m_shieldCharge += GetSelf()->GetAttribute(AttrEntityShieldBoostAmount).get_float();
        if (m_shieldCharge > GetSelf()->GetAttribute(AttrShieldCapacity).get_float())
            m_shieldCharge = GetSelf()->GetAttribute(AttrShieldCapacity).get_float();
    } else
        AI()->DisableRepTimers();
    // TODO: Need to send SpecialFX / amount update
    _UpdateDamage();
}

void Concord::UseArmorRepairer()
{
    if( m_armorDamage > 0 )
    {
        m_armorDamage -= GetSelf()->GetAttribute(AttrEntityArmorRepairAmount).get_float();
        if( m_armorDamage < 0.0 )
            m_armorDamage = 0.0;
    } else
        AI()->DisableRepTimers();
    // TODO: Need to send SpecialFX / amount update
    _UpdateDamage();
}

void Concord::_UpdateDamage()
{
    DoDestiny_DamageDetails dmgState;
        dmgState.shield = m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float();
        dmgState.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float();
        dmgState.timestamp = Win32TimeNow();
        dmgState.armor = 1.0 - m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float();
        dmgState.structure = 1.0 - m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float();
    DoDestiny_OnDamageStateChange dmgChange;
        dmgChange.entityID = GetID();
        dmgChange.state = dmgState.Encode();
    PyTuple *up = dmgChange.Encode();
    //source->QueueDestinyUpdate(&up);
}

void Concord::SetResists() {
    /* fix for missing resist attribs -allan 18April16  */
    if (!m_self->HasAttribute(AttrShieldEmDamageResonance)) m_self->SetAttribute(AttrShieldEmDamageResonance, 1.0, false);
    if (!m_self->HasAttribute(AttrShieldExplosiveDamageResonance)) m_self->SetAttribute(AttrShieldExplosiveDamageResonance, 1.0, false);
    if (!m_self->HasAttribute(AttrShieldKineticDamageResonance)) m_self->SetAttribute(AttrShieldKineticDamageResonance, 1.0, false);
    if (!m_self->HasAttribute(AttrShieldThermalDamageResonance)) m_self->SetAttribute(AttrShieldThermalDamageResonance, 1.0, false);
    if (!m_self->HasAttribute(AttrArmorEmDamageResonance)) m_self->SetAttribute(AttrArmorEmDamageResonance, 1.0, false);
    if (!m_self->HasAttribute(AttrArmorExplosiveDamageResonance)) m_self->SetAttribute(AttrArmorExplosiveDamageResonance, 1.0, false);
    if (!m_self->HasAttribute(AttrArmorKineticDamageResonance)) m_self->SetAttribute(AttrArmorKineticDamageResonance, 1.0, false);
    if (!m_self->HasAttribute(AttrArmorThermalDamageResonance)) m_self->SetAttribute(AttrArmorThermalDamageResonance, 1.0, false);
    if (!m_self->HasAttribute(AttrEmDamageResonance)) m_self->SetAttribute(AttrEmDamageResonance, 1.0, false);
    if (!m_self->HasAttribute(AttrExplosiveDamageResonance)) m_self->SetAttribute(AttrExplosiveDamageResonance, 1.0, false);
    if (!m_self->HasAttribute(AttrKineticDamageResonance)) m_self->SetAttribute(AttrKineticDamageResonance, 1.0, false);
    if (!m_self->HasAttribute(AttrThermalDamageResonance)) m_self->SetAttribute(AttrThermalDamageResonance, 1.0, false);
}


ConcordAI::ConcordAI(Concord* who)
: m_state(Idle),
m_npc(who),
m_mainAttackTimer(1000),
m_processTimer(5000),         //arbitrary.
m_shieldBoosterTimer(10000),  //arbitrary.
m_armorRepairTimer(8000),     //arbitrary.
m_beginFindTarget(5000),      //arbitrary.
m_warpScramblerTimer(5000),   //arbitrary.
m_webifierTimer(5000),        //arbitrary.
m_radius(who->GetSelf()->GetAttribute(AttrSignatureRadius).get_float()),
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

    /** @todo  change this to use config file */
    m_entityAttackRange = 300000;
}

void ConcordAI::Process() {
    /* concord ai needs to be written, so for now we'll just return immediatly */
    return;

    if ((!m_processTimer.Check()) || (!m_npc->SysBubble()->HasPlayers()) || m_npc->DestinyMgr()->IsWarping())
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
             *   Fleeing,    // running away....use mwd/ab (if equipped) then warp away when out of range   (does this make sense??)
             *   Signaling   // calling for help..use full cruise to half of max speed to speed tank while calling for reinforcements
             */
            switch(m_state) {
                case Idle: {
                    /* make timer here for them to leave */
                } break;

                case Chasing: {
                    //NOTE: getting our target like this is pretty weak...
                    SystemEntity* pTarget = m_npc->TargetMgr()->GetFirstTarget(true);
                    if (!pTarget) {
                        if (m_npc->TargetMgr()->HasNoTargets()) {
                            //_log(CONCORD__AI_TRACE, "%s(%u): Stopped chasing, GetFirstTarget() returned NULL.",  m_npc->GetName(), m_npc->GetID());
                            m_state = Idle;
                        }
                        return;
                    } else if (!pTarget->SysBubble()) {
                        m_npc->TargetMgr()->ClearTarget(pTarget);
                        return;
                    }
                    _CheckDistance(pTarget);
                } break;

                case Following: {
                    //NOTE: getting our target like this is pretty weak...
                    SystemEntity* pTarget = m_npc->TargetMgr()->GetFirstTarget(true);
                    if (!pTarget) {
                        if (m_npc->TargetMgr()->HasNoTargets()) {
                            //_log(CONCORD__AI_TRACE, "%s(%u): Stopped following, GetFirstTarget() returned NULL.",  m_npc->GetName(), m_npc->GetID());
                            m_state = Idle;
                        }
                        return;
                    } else if (!pTarget->SysBubble()) {
                        m_npc->TargetMgr()->ClearTarget(pTarget);
                        return;
                    }
                    _CheckDistance(pTarget);
                } break;

                case Engaged: {
                    //NOTE: getting our pTarget like this is pretty weak...
                    SystemEntity* pTarget = m_npc->TargetMgr()->GetFirstTarget(true);
                    if (!pTarget) {
                        if (m_npc->TargetMgr()->HasNoTargets()) {
                            //_log(CONCORD__AI_TRACE, "%s(%u): Stopped engagement, GetFirstTarget() returned NULL.", m_npc->GetName(), m_npc->GetID());
                            _EnterIdle();
                        }
                        return;
                    } else if (!pTarget->SysBubble()) {
                        m_npc->TargetMgr()->ClearTarget(pTarget);
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

void ConcordAI::_EnterIdle() {
    if (m_state == Idle) return;
    // not doing anything....idle.
    //_log(CONCORD__AI_TRACE, "%s(%u): _EnterIdle: returning to idle.",
         m_npc->GetName(), m_npc->GetID());
    m_state = Idle;
    m_npc->DestinyMgr()->Stop();
    m_npc->DestinyMgr()->SetMaxVelocity(m_cruiseSpeed);

    m_webifierTimer.Disable();
    m_beginFindTarget.Disable();
    m_mainAttackTimer.Disable();
    m_armorRepairTimer.Disable();
    m_warpScramblerTimer.Disable();
    m_shieldBoosterTimer.Disable();

    // write code to enable npcs to wander around when idle?
    // sounds like a good idea, but will take process power away from other shit.
}

void ConcordAI::_EnterChasing(SystemEntity* pTarget) {
    if (m_state == Chasing) return;
    //_log(CONCORD__AI_TRACE, "%s(%u): _EnterChasing: %s(%u) begin chasing.",
         m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
    // target out of range to attack/follow, but within npc sight range....use mwd/ab if equiped
    m_npc->DestinyMgr()->SetMaxVelocity(m_chaseSpeed);
    m_npc->DestinyMgr()->Follow(pTarget, m_entityOrbitRange);  //try to get inside orbit range
    m_state = Chasing;
}

void ConcordAI::_EnterFollowing(SystemEntity* pTarget) {
    if (m_state == Following) return;
    //_log(CONCORD__AI_TRACE, "%s(%u): _EnterFollowing: %s(%u) begin following.",
         m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
    // too close to chase, but to far to engage
    m_npc->DestinyMgr()->SetMaxVelocity(m_chaseSpeed /2);
    m_npc->DestinyMgr()->Follow(pTarget, m_entityOrbitRange);  //try to get inside orbit range
    m_state = Following;
}

void ConcordAI::_EnterEngaged(SystemEntity* pTarget) {
    if (m_state == Engaged) return;
    //_log(CONCORD__AI_TRACE, "%s(%u): _EnterEngaged: %s(%u) begin engaging.",
         m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
    // actively fighting
    //   not sure of the actual orbit speed of npc's, but their 'cruise speed' seems a bit slow.
    //   this sets orbit speed between cruise speed and quarter of max speed (whether mwb or ab)
    //   this will also enable this npc to have a variable speed, instead of fixed upon creation.
    m_npc->DestinyMgr()->SetMaxVelocity(MakeRandomFloat(m_cruiseSpeed, (m_chaseSpeed /4)));
    m_npc->DestinyMgr()->Orbit(pTarget, m_entityOrbitRange);  //try to get inside orbit range
    m_state = Engaged;
}

void ConcordAI::_EnterFleeing(SystemEntity* pTarget) {
    if (m_state == Fleeing) return;
    //_log(CONCORD__AI_TRACE, "%s(%u): _EnterFleeing: %s(%u) begin fleeing.",
         m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
    // actively fleeing
    //  use superspeed to disengage, then warp.  << both these will need to be written.
    //  this state is only usable by higher-class npcs.
    m_npc->DestinyMgr()->SetMaxVelocity(m_chaseSpeed);
    m_state = Fleeing;
}

void ConcordAI::_EnterSignaling(SystemEntity* pTarget) {
    if (m_state == Signaling) return;
    //_log(CONCORD__AI_TRACE, "%s(%u): _EnterSignaling: %s(%u) begin signaling.",
         m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
    // actively signaling
    //  start speedtanking while signaling.  (im sure this is cheating, but fuckem.)
    //  this state is only usable by higher-class npcs.
    m_npc->DestinyMgr()->SetMaxVelocity(MakeRandomFloat(m_cruiseSpeed, (m_chaseSpeed /2)));
    m_npc->DestinyMgr()->Orbit(pTarget, m_entityOrbitRange);  //try to get inside orbit range
    m_state = Signaling;
}

void ConcordAI::_CheckDistance(SystemEntity* pSE)
{
    //rewrote distance checks for correct logic this time
    GVector usToThem(m_npc->GetPosition(), pSE->GetPosition());
    //double dist = m_npc->GetPosition().distance(pSE->GetPosition());     // this throws occasional errors (segfault)
    double dist = usToThem.length();
    if (dist > m_entityAttackRange) {
        _log(CONCORD__AI_TRACE, "%s(%u): _CheckDistance: %s(%u) is too far away (%u).  Return to Idle.", \
             m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID(), dist);
        if (m_state != Idle) {
            // target is no longer in npc's "sight range".  unlock target and return to idle.
            //   should we do anything else here?  search for another target?  wander around?
            m_npc->TargetMgr()->ClearTarget(pSE);
            if (m_npc->TargetMgr()->HasNoTargets())
                _EnterIdle();
        }
        return;
    } else if (dist < m_entityFlyRange) { //within weapon max (and within falloff)
        _EnterEngaged(pSE); //engage and orbit
    } else if (dist < m_entityChaseRange) { //within follow
        _EnterFollowing(pSE);
    } else if (dist < m_entityAttackRange) { //within sight
        _EnterChasing(pSE);
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

void ConcordAI::ClearTargets() {
    m_npc->TargetMgr()->ClearTargets();
}

void ConcordAI::ClearAllTargets() {
    m_npc->TargetMgr()->ClearAllTargets();
}

void ConcordAI::Target(SystemEntity* pTarget) {
    double targetTime = GetTargetTime();

	if (!m_npc->TargetMgr()->StartTargeting(pTarget, targetTime, (uint8)m_npc->GetSelf()->GetAttribute(AttrMaxAttackTargets).get_int(), m_entityAttackRange )) {
        //_log(CONCORD__AI_TRACE, "%s(%u): Targeting of %s(%u) failed.  Clear Target and Return to Idle.",
             m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
        //ClearAllTargets();
        _EnterIdle();
        return;
    }
    m_beginFindTarget.Disable();
    _CheckDistance(pTarget);
}

void ConcordAI::Targeted(SystemEntity* pAgressor) {
    double targetTime = GetTargetTime();

    switch(m_state) {
        case Idle: {
            _log(CONCORD__AI_TRACE, "%s(%u): Targeted by %s(%u) in Idle. Begin Approaching and start Targeting sequence.", \
                 m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());
            _EnterChasing(pAgressor);

			if (!m_npc->TargetMgr()->StartTargeting( pAgressor, targetTime, (uint8)m_npc->GetSelf()->GetAttribute(AttrMaxAttackTargets).get_int(), m_entityAttackRange)) {
                _EnterIdle();
                return;
            }
            m_beginFindTarget.Disable();
            _CheckDistance(pAgressor);
        } break;
        case Chasing: {
            _log(CONCORD__AI_TRACE, "%s(%u): Targeted by %s(%u) while chasing.",
                 m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case Following: {
            _log(CONCORD__AI_TRACE, "%s(%u): Targeted by %s(%u) while following.",
                 m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case Engaged: {
            _log(CONCORD__AI_TRACE, "%s(%u): Targeted by %s(%u) while engaged.",
                 m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case Fleeing: {
            _log(CONCORD__AI_TRACE, "%s(%u): Targeted by %s(%u) while fleeing.",
                 m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;
        case Signaling: {
            _log(CONCORD__AI_TRACE, "%s(%u): Targeted by %s(%u) while signaling.",
                 m_npc->GetName(), m_npc->GetID(), pAgressor->GetName(), pAgressor->GetID());
        } break;

        //no default on purpose
    }
}

void ConcordAI::TargetLost(SystemEntity* pTarget) {
    switch(m_state) {
        case Chasing:
        case Following:
        case Engaged: {
            if (m_npc->TargetMgr()->HasNoTargets()) {
                _log(CONCORD__AI_TRACE, "%s(%u): Target %s(%u) lost. No targets remain.  Return to Idle.",
                     m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
                _EnterIdle();
            } else {
                _log(CONCORD__AI_TRACE, "%s(%u): Target %s(%u) lost, but more targets remain.",
                     m_npc->GetName(), m_npc->GetID(), pTarget->GetName(), pTarget->GetID());
            }

        } break;

        default:
            break;
    }
}

void ConcordAI::Attack(SystemEntity* pSE)
{
    if (m_mainAttackTimer.Check()) {
        if (!pSE) return;
        // Check to see if the target still in the bubble (Client warped out)
        if (!m_npc->SysBubble()->InBubble(pSE->GetPosition())) {
            _log(CONCORD__AI_TRACE, "%s(%u): Target %s(%u) no longer in bubble.  Clear target and move on",
                 m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
            m_npc->TargetMgr()->ClearTarget(pSE);
            return;
        }
        DestinyManager* pDestiny = pSE->DestinyMgr();
        if (!pDestiny) {
            _log(CONCORD__AI_TRACE, "%s(%u): Target %s(%u) has no destiny manager.  Clear target and move on",
                 m_npc->GetName(), m_npc->GetID(), pSE->GetName(), pSE->GetID());
            m_npc->TargetMgr()->ClearTarget(pSE);
            return;
        }
        // Check to see if the target is not cloaked:
        if (pDestiny->IsCloaked()) {
            _log(CONCORD__AI_TRACE, "%s(%u): Target %s(%u) is cloaked.  Clear target and move on",
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

void ConcordAI::AttackTarget(SystemEntity* pTarget) {
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
             effectTargetAttack
    );

    d *= m_npc->GetSelf()->GetAttribute(AttrDamageMultiplier).get_float();
    pTarget->ApplyDamage(d);
}

//NOTE: duplicated from module manager code. They should share some day!
void ConcordAI::_SendWeaponEffect( const char*effect, SystemEntity* pTarget ) {
    DoDestiny_OnSpecialFX13 sfx;
    sfx.entityID = m_npc->GetSelf()->itemID();
    sfx.moduleID = m_npc->GetSelf()->itemID();
    sfx.moduleTypeID = m_npc->GetSelf()->typeID();
    sfx.targetID = pTarget->GetID();
    sfx.otherTypeID = pTarget->GetSelf()->typeID();
    sfx.effect_type = effect;
    sfx.isOffensive = 1;
    sfx.start = 1;
    sfx.active = 1;
    sfx.duration_ms = m_attackSpeed;
    sfx.repeat = 1;
    sfx.startTime = Win32TimeNow();

    PyTuple* up = sfx.Encode();
    m_npc->DestinyMgr()->SendSingleDestinyUpdate( &up );    //consumed
}

double ConcordAI::GetTargetTime()
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

void ConcordAI::DisableRepTimers()
{
    m_armorRepairTimer.Disable();
    m_shieldBoosterTimer.Disable();
}


ConcordSpawnMgr::ConcordSpawnMgr() {

}
