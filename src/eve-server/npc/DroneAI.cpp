/**
 * DroneAI.cpp
 *      this class is for drone AI
 *
 * @Author:     Allan
 * @Version:    0.15
 * @Date:       27Nov19  (copied from NPCAI.cpp)
 * @Rewrite:    3Feb25  (complete refactor to process all types of drones)
*/

#include "eve-server.h"

#include "Client.h"
#include "StatisticMgr.h"
#include "inventory/AttributeEnum.h"
#include "system/DestinyManager.h"
#include "npc/Drone.h"
#include "npc/DroneAI.h"
#include "system/Damage.h"
#include "system/SystemBubble.h"

DroneAIMgr::DroneAIMgr(DroneSE* pdSE)
: m_state(DroneAI::State::Idle),
  m_droneSE(pdSE),
  m_targetSE(nullptr),
  m_assignedShip(nullptr),
  m_mainAttackTimer(0),
  m_processTimer(0),
  m_beginFindTarget(0),
  m_warpScramblerTimer(0),
  m_webifierTimer(0),
  m_sigRadius(0.0f),
  m_maxSpeed(0),
  m_cycleTime(0.0f),
  m_cruiseSpeed(0),
  m_entityFlyRange(0.0f),
  m_entityChaseRange(0.0f),
  m_maxRange(0.0f),
  m_proximityRange(0.0f),
  m_entityOrbitRange(0.0f),
  m_entityAttackRange(0.0f),
  m_armorRepairDuration(0),
  m_shieldBoosterDuration(0),
  m_holdFlag(flagCargoHold)
{


}

void DroneAIMgr::Init() {
    InventoryItemRef dRef = m_droneSE->GetSelf();
    m_sigRadius = (dRef->GetAttribute(AttrSignatureRadius).get_float());
    m_cycleTime = (dRef->GetAttribute(AttrSpeed).get_float());
    m_cruiseSpeed = (dRef->GetAttribute(AttrEntityCruiseSpeed).get_uint32());
    m_maxSpeed = (dRef->GetAttribute(AttrMaxVelocity).get_uint32());
    m_entityFlyRange = (dRef->GetAttribute(AttrEntityFlyRange).get_float());
    m_entityChaseRange = (dRef->GetAttribute(AttrEntityChaseMaxDistance).get_float());
    m_maxRange = (dRef->GetAttribute(AttrMaxRange).get_float());
    m_proximityRange = (dRef->GetAttribute(AttrProximityRange).get_float());
    m_entityAttackRange = (dRef->GetAttribute(AttrEntityAttackRange).get_float());
    m_entityOrbitRange = (dRef->GetAttribute(AttrOrbitRange).get_float());
    switch (dRef->groupID()) {
        case EVEDB::invGroups::Proximity_Drone: {    //97
        } break;
        case EVEDB::invGroups::Combat_Drone: {    //100
        } break;
        case EVEDB::invGroups::Mining_Drone: {    //101
            m_cycleTime = (dRef->GetAttribute(AttrDuration).get_float());
            m_maxRange = (dRef->GetAttribute(AttrMaxRange).get_float());
            m_entityAttackRange = (dRef->GetAttribute(AttrMaxRange).get_float());
            m_cruiseSpeed = (dRef->GetAttribute(AttrMaxVelocity).get_uint32());
            m_maxSpeed = (dRef->GetAttribute(AttrMaxVelocity).get_uint32());

        } break;
        case EVEDB::invGroups::Repair_Drone: {    //299
            m_shieldBoosterDuration = (dRef->GetAttribute(AttrEntityShieldBoostDuration).get_uint32());
            m_armorRepairDuration = (dRef->GetAttribute(AttrEntityArmorRepairDuration).get_uint32());
        } break;
        case EVEDB::invGroups::Unanchoring_Drone: {    //470
        } break;
        case EVEDB::invGroups::Cap_Drain_Drone: {    //544
        } break;
        case EVEDB::invGroups::Warp_Scrambling_Drone: {    //545
        } break;
        case EVEDB::invGroups::Fighter_Drone: {    //549
        } break;
        case EVEDB::invGroups::Electronic_Warfare_Drone: {    //639
        } break;
        case EVEDB::invGroups::Logistic_Drone: {    //640
        } break;
        case EVEDB::invGroups::Stasis_Webifying_Drone: {    //641
        } break;
        case EVEDB::invGroups::Fighter_Bomber: {    //1023
        } break;
        default: {
            sLog.Warning("Drone::Init()", "drone %s of group %s has no specific case.", \
                    dRef->name(), dRef->groupID());
        }
    }

}

void DroneAIMgr::Process() {
    double profileStartTime(GetTimeUSeconds());

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
     *   Operating         = 9,  // whats diff from engaged here?  mining maybe?
     *   Engaged           = 10, // non-combat? - needs targetID
     *   // internal only
     *   Unknown           = 8,  // as stated
     *   Guarding          = 11,
     *   Assisting         = 12,
     *   Incapacitated     = 13  // out of control range, but online
     */

    // test for drone attributes here - aggressive, focus fire, attack/follow
    // test for control distance here also.  offline drones outside this  (AttrDroneControlDistance)
    switch(m_state) {
        case DroneAI::State::Idle: {
            // orbiting controlling ship
        } break;
        case DroneAI::State::Mining: {
            // determine drone step and act accordingly

        } break;
        case DroneAI::State::Engaged: {
            if (m_targetSE == nullptr) {
                SetIdle();
                return;
            } else if (m_targetSE->SysBubble() == nullptr) {
                SetIdle();
                //m_droneSE->TargetMgr()->OnTarget(pTarget, TargMgr::Mode::Lost);
                return;
            }
            CheckDistance();
        } break;

        case DroneAI::State::Departing: { // return to ship.  when close enough, set lazy orbit
            if (m_droneSE->GetPosition().distance(m_assignedShip->GetPosition()) < m_maxRange)
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
        case DroneAI::State::Approaching:
        case DroneAI::State::Departing2:
        case DroneAI::State::Pursuit: {
           // do nothing here yet
        } break;

        case DroneAI::State::Invalid: {
            // check everything in this state.   return to ship?
        } break;
    //no default on purpose
    }
    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::drone, GetTimeUSeconds() - profileStartTime);
}

void DroneAIMgr::AssignShip(ShipSE* pSE) {
    m_assignedShip = pSE;
    if (pSE == nullptr)
        return;
    if (pSE->GetSelf()->HasAttribute(AttrOreHoldCapacity)) {
        m_holdFlag = flagOreHold;
    } else {
        m_holdFlag = flagCargoHold;
    }
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

void DroneAIMgr::Abandon() {
    // drone abandoned.  clear everything and idle
    m_state = DroneAI::State::Incapacitated;
    m_assignedShip = nullptr;
    m_targetSE = nullptr;

    // disable all timers
    m_processTimer.Disable();
    m_webifierTimer.Disable();
    m_beginFindTarget.Disable();
    m_mainAttackTimer.Disable();
    m_warpScramblerTimer.Disable();
}

void DroneAIMgr::Return() {
    m_assignedShip = m_droneSE->GetHomeShip();
    m_droneSE->DestinyMgr()->SetMaxVelocity(m_maxSpeed);
    m_droneSE->DestinyMgr()->Follow(m_assignedShip, m_maxRange);
    m_state = DroneAI::State::Departing;
}

void DroneAIMgr::SetIdle() {
    if (m_state == DroneAI::State::Idle)
        return;
    // not doing anything....idle.
    _log(DRONE__AI_TRACE, "Drone %s(%u): SetIdle: returning to idle.",
         m_droneSE->GetName(), m_droneSE->GetID());
    m_state = DroneAI::State::Idle;

    ClearAllTargets();

    // disable ewar timers
    m_webifierTimer.Disable();
    m_beginFindTarget.Disable();
    m_mainAttackTimer.Disable();
    m_warpScramblerTimer.Disable();

    // orbit assigned ship
    m_droneSE->IdleOrbit(m_cruiseSpeed, m_assignedShip);
}

void DroneAIMgr::SetEngaged() {
    if (m_state == DroneAI::State::Engaged)
        return;
    _log(DRONE__AI_TRACE, "Drone %s(%u): SetEngaged: %s(%u) begin engaging.",
         m_droneSE->GetName(), m_droneSE->GetID(), m_targetSE->GetName(), m_targetSE->GetID());
    // actively engaged with target
    m_droneSE->DestinyMgr()->SetMaxVelocity(MakeRandomFloat(m_cruiseSpeed, (m_maxSpeed /4)));
    m_droneSE->DestinyMgr()->InitOrbit(m_targetSE, m_maxRange);  //try to get inside orbit range
    m_state = DroneAI::State::Engaged;
}

void DroneAIMgr::CheckDistance() {
    if (!m_droneSE->InControlDistance()) {
        // drone wandered out of ship's control distance.  stop, disable, and offline
        m_droneSE->DestinyMgr()->Stop();
        SetIdle();
        return;
    }


    double dist = m_droneSE->GetPosition().distance(m_targetSE->GetPosition());
    if (dist > m_entityChaseRange) {
        _log(DRONE__AI_TRACE, "Drone %s(%u): CheckDistance: %s(%u) is too far away (%.1fm).  Return to Idle.",
             m_droneSE->GetName(), m_droneSE->GetID(), m_targetSE->GetName(), m_targetSE->GetID(), dist);
        if (m_state != DroneAI::State::Idle) {
            // target is no longer in npc's "sight range".  unlock target and return to idle.
            //   should we do anything else here?  search for another target?  wander around?
            ClearTarget();
        }
        return;
    } else if (dist < m_entityFlyRange) { //within weapon max (and within falloff)
        SetEngaged(); //engage and orbit
    } else if (dist < m_entityAttackRange) { //within follow
       // SetFollowing();
    } else if (dist < m_entityChaseRange) { //within sight
       // SetChasing();
        return;
    }

    DoCycle();
}

void DroneAIMgr::ClearTargets() {
    m_targetSE = nullptr;
    m_droneSE->TargetMgr()->ClearTargets();
}

void DroneAIMgr::ClearAllTargets() {
    m_targetSE = nullptr;
    m_droneSE->TargetMgr()->ClearAllTargets();
    //m_droneSE->TargetMgr()->OnTarget(nullptr, TargMgr::Mode::Clear, TargMgr::Msg::ClientReq);
}

void DroneAIMgr::Target(SystemEntity* pTarget) {
    if ((m_targetSE != nullptr) and (m_targetSE != pTarget)) {
        // target changed.  reset everything
    }

    m_targetSE = pTarget;
    bool chase(false);  // StartTargeting needs bool ref...
    if (!m_droneSE->TargetMgr()->StartTargeting(pTarget,
                                m_droneSE->GetSelf()->GetAttribute(AttrScanSpeed).get_float(),
                                (uint8)m_droneSE->GetSelf()->GetAttribute(AttrMaxAttackTargets).get_int(),
                                m_entityAttackRange, chase))
    {
        _log(DRONE__AI_TRACE, "Drone %s(%u): Targeting of %s(%u) failed.  Clear Target and Return to Idle.",
             m_droneSE->GetName(), m_droneSE->GetID(), pTarget->GetName(), pTarget->GetID());
        SetIdle();
        return;
    }
    m_beginFindTarget.Disable();
    m_mainAttackTimer.Start(m_cycleTime);

    CheckDistance();
}

void DroneAIMgr::Targeted(SystemEntity* pAgressor) {
    _log(DRONE__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while %s.",
                m_droneSE->GetName(), m_droneSE->GetID(), pAgressor->GetName(), pAgressor->GetID(), GetStateName(m_state).c_str());
    switch(m_state) {
        case DroneAI::State::Idle: {
        } break;
        case DroneAI::State::Operating: {
        } break;
        case DroneAI::State::Unknown: {
        } break;
        case DroneAI::State::Engaged: {
        } break;
        case DroneAI::State::Fleeing: {
        } break;
        case DroneAI::State::Incapacitated: {
        } break;
        case DroneAI::State::Guarding: {
        } break;
        case DroneAI::State::Assisting: {
        } break;
        case DroneAI::State::Combat: {
        } break;
        case DroneAI::State::Mining: {
        } break;
        case DroneAI::State::Approaching: {
        } break;
        case DroneAI::State::Departing: {
        } break;
        case DroneAI::State::Departing2: {
        } break;
        case DroneAI::State::Pursuit: {
        } break;
    }

    // TODO:  send warning to controlling ship

    std::string text = "target lock on me";
    //01110100 01100001 01110010 01100111 01100101 01110100 00100000 01101100 01101111 01100011 01101011 00100000 01101111 01101110 00100000 01101101 01100101
    // convert string to binary
    m_assignedShip->GetPilot()->SendNotifyMsg(BinString(text).c_str());
}

void DroneAIMgr::TargetLost(SystemEntity* pTarget) {
    switch(m_state) {
        case DroneAI::State::Engaged: {
            if (m_droneSE->TargetMgr()->HasNoTargets()) {
                _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) lost. No targets remain.  Return to Idle.",
                     m_droneSE->GetName(), m_droneSE->GetID(), pTarget->GetName(), pTarget->GetID());
                SetIdle();
            } else {
                _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) lost, but more targets remain.",
                     m_droneSE->GetName(), m_droneSE->GetID(), pTarget->GetName(), pTarget->GetID());
            }
        } break;
        default:
            break;
    }
}

void DroneAIMgr::DoCycle() {
    if (m_targetSE == nullptr)
        return;
    if (m_targetSE->IsAsteroidSE()) {
        // there are 4 stages to drone mining....approach rock, mine rock, return to ship, deposit ore in ship's hold
        if (m_mainAttackTimer.Check())
            Mine();
    } else {
        DestinyManager* pDestiny = m_targetSE->DestinyMgr();
        if (pDestiny == nullptr) {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) has no destiny manager.  Clear target and move on",
                 m_droneSE->GetName(), m_droneSE->GetID(), m_targetSE->GetName(), m_targetSE->GetID());
            ClearTarget();
            return;
        }
        // Check to see if the target is not cloaked:
        if (pDestiny->IsCloaked()) {
            _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) is cloaked.  Clear target and move on",
                 m_droneSE->GetName(), m_droneSE->GetID(), m_targetSE->GetName(), m_targetSE->GetID());
            ClearTarget();
            return;
        }

        if (m_mainAttackTimer.Check()) {
            if (m_droneSE->TargetMgr()->CanAttack())
                AttackTarget();
        }
    }
}


void DroneAIMgr::Mine() {
    // note:  there are no ice harvesting drones
    // when mining drone's target is depleted, drone will get half cycle and not count in ore removed for module count

    float cycleVol(m_droneSE->GetSelf()->GetAttribute(AttrMiningAmount).get_float());

    InventoryItemRef roidRef(m_targetSE->GetSelf());
    float oreAmount(cycleVol / (roidRef->GetAttribute(AttrVolume).get_float()));
    if (oreAmount <= 0) {
        // drone cannot mine this heavy ore
        return;
    }

    // what if the assigned ship is corp?  will this affect ore?  it may not show in hold initially
    ItemData idata(roidRef->typeID(), m_assignedShip->GetOwnerID(), locTemp, flagNone, oreAmount);
    InventoryItemRef oRef(sItemFactory.SpawnItem(idata));
    if (oRef.get() == nullptr) {
        _log(DRONE__WARNING, "Could not create mined ore for %s assigned to %s", \
                m_droneSE->GetName(), m_assignedShip->GetPilot()->GetName());
        m_assignedShip->GetPilot()->SendNotifyMsg("Mining operations for %s have been deactivated and it is returning to your ship.<br>There was an error gathering %s ore.", \
                m_droneSE->GetName(), oRef->name());
        SetIdle();
        return;
    }

    if (m_assignedShip->GetSelf()->GetMyInventory()->HasAvailableSpace(m_holdFlag, oRef)) {
        // automagically stack ore in hold.  this is a feature.
        oRef->MergeTypesInCargo(m_assignedShip->GetShipItemRef().get(), m_holdFlag);
    } else {
        m_assignedShip->GetPilot()->SendNotifyMsg("Your %s deactivates mining operations as it couldn't add the %s ore to your %s.", \
                m_droneSE->GetName(), oRef->name(), sDataMgr.GetFlagName(m_holdFlag));
        SetIdle();
        return;
    }

    // add data to StatisticMgr
    sStatMgr.Add(Stat::oreMined, cycleVol);

    _log(DRONE__AI_TRACE, "%s assigned to %s has completed ProcessMiningCycle.", \
            m_droneSE->GetName(), m_assignedShip->GetPilot()->GetName());
}

void DroneAIMgr::ClearTarget() {
    m_targetSE = nullptr;
    m_droneSE->TargetMgr()->ClearTarget(m_targetSE);
    //m_droneSE->TargetMgr()->OnTarget(pSE, TargMgr::Mode::Lost);

    if (m_droneSE->TargetMgr()->HasNoTargets())
        SetIdle();
}

//also check for special effects and write code to implement them
//modifyTargetSpeedRange, modifyTargetSpeedChance
//entityWarpScrambleChance

void DroneAIMgr::AttackTarget() {
    /** @todo  not all drones use lazors...fix this */
    //  woot!! --> group:1010        cat:8       Compact Citadel Torpedo         Citadel torpedoes for fighter-bombers

    // effects are listed in EVE_Effects.h
    //  NOTE: drones are called 'entities' in client; EVE_Effects has 'entityxxx' for gfx...see below
    std::string guid = "effects.Laser"; // client looks for 'turret' in ship.ball.modules for 'effects.laser'
    uint32 gfxID = 0;
    if (m_droneSE->GetSelf()->HasAttribute(AttrGfxTurretID))// graphicID for turret for drone type ships
        gfxID = m_droneSE->GetSelf()->GetAttribute(AttrGfxTurretID).get_uint32();
    /*
    if (m_droneSE->GetSelf()->HasAttribute(AttrGfxBoosterID))// graphicID for turret for drone type ships
        gfxID = m_droneSE->GetSelf()->GetAttribute(AttrGfxBoosterID).get_uint32();
    */
    m_droneSE->DestinyMgr()->SendGFX14(m_droneSE->GetSelf()->itemID(),
                                             m_droneSE->GetSelf()->itemID(),
                                             m_droneSE->GetSelf()->typeID(), //m_droneSE->GetSelf()->GetAttribute(AttrGfxTurretID).get_int(),
                                             m_targetSE->GetID(),
                                             0,std::move(guid),1,1,1,m_cycleTime,0,gfxID);

    Damage dam(m_droneSE,
             m_droneSE->GetSelf(),
             m_droneSE->GetKinetic(),
             m_droneSE->GetThermal(),
             m_droneSE->GetEM(),
             m_droneSE->GetExplosive(),
             m_formula.GetDroneToHit(m_droneSE, m_targetSE)
            );

    dam *= m_droneSE->GetSelf()->GetAttribute(AttrDamageMultiplier).get_float();
    m_targetSE->ApplyDamage(dam);
}

/*
    warpScrambleForEntity =   563,     // effects.WarpScramble
    missileLaunchingForEntity =   569,     // effects.MissileDeployment
    entityCapacitorDrain =   1872,     // effects.EnergyVampire
    entityTrackingDisruptOld =   1877,     // effects.ElectronicAttributeModifyTarget
    entitySensorDampen =   1878,     // effects.ElectronicAttributeModifyTarget
    entityTargetPaint =   1879,     // effects.TargetPaint
    entityShieldBoostingSmall =   2192,     // effects.ShieldBoosting
    entityShieldBoostingMedium =   2193,     // effects.ShieldBoosting
    entityShieldBoostingLarge =   2194,     // effects.ShieldBoosting
    entityArmorRepairingSmall =   2195,     // effects.ArmorRepair
    entityArmorRepairingMedium =   2196,     // effects.ArmorRepair
    entityArmorRepairingLarge =   2197,     // effects.ArmorRepair
    entityTrackingDisrupt =   4982,     // effects.ElectronicAttributeModifyTarget
    warpScrambleTargetMWDBlockActivationForEntity =   5928,     // effects.WarpScramble
    */


std::string DroneAIMgr::GetStateName(int8 stateID)
{
    switch (stateID) {
        case DroneAI::State::Idle:            return "Idle";
        case DroneAI::State::Combat:          return "Combat";
        case DroneAI::State::Mining:          return "Mining";
        case DroneAI::State::Approaching:     return "Approaching";
        case DroneAI::State::Departing:       return "Returning to ship";
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
