/**
 * DroneAI.cpp
 *      this class is for drone AI
 *
 * @Author:     Allan
 * @Version:    0.81
 * @Date:       27Nov19  (copied from NPCAI.cpp)
 * @Rewrite:    3Feb25  (complete refactor to process all types and actions of drones)
*/

/** @todo notes for incomplete drone systems
 * modes.  started for TargetLost
 * incomplete states.  assist, guard, xfer, flee,
 * incomplete systems.  web, ewar
 * need response method for objects in sight range (proximity)
 *   - all drones have proximity range data
 *
 *
 */

#include "eve-server.h"

#include "Client.h"
#include "EntityMgr.h"
#include "StatisticMgr.h"
#include "effects/EffectsDataMgr.h"
#include "inventory/AttributeEnum.h"
#include "system/DestinyManager.h"
#include "npc/Drone.h"
#include "npc/DroneAI.h"
#include "system/Container.h"
#include "system/Damage.h"
#include "system/SystemBubble.h"
#include "system/TargetManager.h"


DroneAIMgr::DroneAIMgr(DroneSE* pdSE)
: m_heading(NULL_ORIGIN_V),
m_velocity(NULL_ORIGIN_V),
targSE(nullptr),
mySE(pdSE),
shipSE(nullptr),
m_ore(nullptr),
m_holdFlag(flagCargoHold),
m_processTimer(0),
m_beginFindTarget(0),
m_warpScramblerTimer(0),
m_webifierTimer(0),
m_sendCmd(false),
m_booster(false),
m_repeat(false),
m_state(DroneAI::State::Invalid),
m_action(DroneAI::Action::Invalid),
m_effectID(0),
m_cycleTime(0),
m_maxSpeed(0),
m_cruiseSpeed(0),
m_startTime(0),
m_maxDistance(0.0f),
m_chaseDistance(0.0f),
m_orbitDistance(0.0f),
m_attackDistance(0.0f),
m_falloffDistance(0.0f),
m_proximityDistance(0.0f),
m_alignTime(0.0f),
m_accelTime(0.0f),
m_timeFraction(0.0f),
m_prevSpeedFraction(0.0f),
m_userSpeedFraction(0.0f),
m_activeSpeedFraction(0.0f),
m_agility(0.0),
m_moveTime(0.0)
{

}

void DroneAIMgr::Init() {
    InventoryItemRef dRef = mySE->GetSelf();
    m_maxSpeed = (dRef->GetAttribute(AttrMaxVelocity).get_uint32());
    m_cycleTime = (dRef->GetAttribute(AttrSpeed).get_float());
    m_cruiseSpeed = (dRef->GetAttribute(AttrEntityCruiseSpeed).get_uint32());
    m_proximityDistance = (dRef->GetAttribute(AttrProximityRange).get_uint32());

    // set times and ranges unique to these types (override above if required)
    //  some drones dont have all of these, but they are used in various checks so we're setting them to nominal
    switch (dRef->groupID()) {
        case EVEDB::invGroups::Combat_Drone: {    //100
            m_chaseDistance = dRef->GetAttribute(AttrEntityChaseMaxDistance).get_uint32();

            if (m_chaseDistance < 1000) {
                // stationary drone, long range, no chase
                // hi-lo: attack, max, falloff
                m_orbitDistance = 1000;
                m_falloffDistance = dRef->GetAttribute(AttrFalloff).get_uint32();
                m_attackDistance = dRef->GetAttribute(AttrMaxRange).get_uint32();
                m_maxDistance = dRef->GetAttribute(AttrEntityAttackRange).get_uint32();
            } else {
                // hi-lo:  attack, chase, falloff, max
                m_orbitDistance = dRef->GetAttribute(AttrMaxRange).get_uint32();
                m_falloffDistance = dRef->GetAttribute(AttrFalloff).get_uint32();
                m_attackDistance = dRef->GetAttribute(AttrEntityChaseMaxDistance).get_uint32();
                m_maxDistance = dRef->GetAttribute(AttrEntityAttackRange).get_uint32();
            }
        } break;
        case EVEDB::invGroups::Mining_Drone: {    //101
            m_cycleTime = (dRef->GetAttribute(AttrDuration).get_float());
            m_cruiseSpeed = (dRef->GetAttribute(AttrMaxVelocity).get_uint32());
            m_orbitDistance = dRef->GetAttribute(AttrOrbitRange).get_uint32();
            //m_falloffDistance = dRef->GetAttribute(AttrFalloff).get_uint32();//0
            //m_attackDistance = dRef->GetAttribute(AttrEntityAttackRange).get_uint32();//0
            //m_chaseDistance = dRef->GetAttribute(AttrEntityChaseMaxDistance).get_uint32();//0
            m_falloffDistance = dRef->GetAttribute(AttrMaxRange).get_uint32();
            m_attackDistance = dRef->GetAttribute(AttrMaxRange).get_uint32();
            m_chaseDistance = dRef->GetAttribute(AttrMaxRange).get_uint32();
            m_maxDistance = dRef->GetAttribute(AttrMaxRange).get_uint32();
        } break;
        case EVEDB::invGroups::Logistic_Drone: {    //640
            m_booster = true;
            m_orbitDistance = dRef->GetAttribute(AttrEntityChaseMaxDistance).get_uint32() / 10;
            //m_falloffDistance = dRef->GetAttribute(AttrFalloff).get_uint32(); //0
            //m_attackDistance = dRef->GetAttribute(AttrEntityAttackRange).get_uint32();//0
            m_falloffDistance = dRef->GetAttribute(AttrMaxRange).get_uint32();
            m_attackDistance = dRef->GetAttribute(AttrMaxRange).get_uint32();
            m_chaseDistance = dRef->GetAttribute(AttrEntityChaseMaxDistance).get_uint32();
            m_maxDistance = dRef->GetAttribute(AttrMaxRange).get_uint32();
            if (dRef->HasAttribute(AttrEntityArmorRepairDuration)) {
                m_cycleTime = dRef->GetAttribute(AttrEntityArmorRepairDuration).get_uint32();
            } else if (dRef->HasAttribute(AttrEntityShieldBoostDuration)) {
                m_cycleTime = dRef->GetAttribute(AttrEntityShieldBoostDuration).get_uint32();
            } else {
                // has neither...make note
                _log(DRONE__WARNING, "DroneAI::RepairTarget() - %s(%u) of %s has neither armor nor shield duration", \
                        dRef->name(), dRef->typeID(), dRef->type().groupName().c_str());
            }
        } break;
        case EVEDB::invGroups::Cap_Drain_Drone: {    //544
            m_orbitDistance = dRef->GetAttribute(AttrMaxRange).get_uint32() / 10;
            //m_falloffDistance = dRef->GetAttribute(AttrFalloff).get_uint32();//0
            m_falloffDistance = dRef->GetAttribute(AttrEntityAttackRange).get_uint32();
            m_attackDistance = dRef->GetAttribute(AttrEntityAttackRange).get_uint32();
            m_chaseDistance = dRef->GetAttribute(AttrEntityChaseMaxDistance).get_uint32();
            m_maxDistance = dRef->GetAttribute(AttrMaxRange).get_uint32();
        } break;
        case EVEDB::invGroups::Fighter_Drone: {    //549
            // these are advanced drones.  will follow target in warp, but not jump
            m_orbitDistance = dRef->GetAttribute(AttrEntityChaseMaxDistance).get_uint32() / 2;
            if (m_orbitDistance > 900)
                m_orbitDistance = 1000;
        } break;
        case EVEDB::invGroups::Electronic_Warfare_Drone: {    //639
            m_orbitDistance = dRef->GetAttribute(AttrEntityChaseMaxDistance).get_uint32() / 10;
            m_falloffDistance = dRef->GetAttribute(AttrEntityChaseMaxDistance).get_uint32();
            m_attackDistance = dRef->GetAttribute(AttrMaxRange).get_uint32();
            m_chaseDistance = dRef->GetAttribute(AttrFalloff).get_uint32();
            m_maxDistance = dRef->GetAttribute(AttrEntityAttackRange).get_uint32();
        } break;
        case EVEDB::invGroups::Stasis_Webifying_Drone: {    //641
            m_orbitDistance = dRef->GetAttribute(AttrEntityChaseMaxDistance).get_uint32() / 10;
            m_falloffDistance = dRef->GetAttribute(AttrFalloff).get_uint32();
            m_attackDistance = dRef->GetAttribute(AttrEntityChaseMaxDistance).get_uint32();
            m_chaseDistance = dRef->GetAttribute(AttrEntityAttackRange).get_uint32();
            m_maxDistance = dRef->GetAttribute(AttrMaxRange).get_uint32();
        } break;
        case EVEDB::invGroups::Fighter_Bomber: {    //1023
            m_orbitDistance = dRef->GetAttribute(AttrEntityChaseMaxDistance).get_uint32() / 10;
            m_falloffDistance = dRef->GetAttribute(AttrEntityAttackRange).get_uint32() / 2;
            m_attackDistance = dRef->GetAttribute(AttrEntityAttackRange).get_uint32();
            m_chaseDistance = dRef->GetAttribute(AttrEntityChaseMaxDistance).get_uint32();
            m_maxDistance = dRef->GetAttribute(AttrEntityAttackRange).get_uint32();
        } break;
        /*  i dont think these are available
        case EVEDB::invGroups::Repair_Drone:     //299    non-published
        case EVEDB::invGroups::Warp_Scrambling_Drone: {    //545    non-published
        case EVEDB::invGroups::Unanchoring_Drone: {    //470
        } break;
        case EVEDB::invGroups::Proximity_Drone: {    //97
        } break;
        */
        default: {
            sLog.Warning("Drone::Init()", "drone %s of group %s has no specific case.", \
                    dRef->name(), dRef->groupID());
        }
    }

    m_effectID = dRef->type().GetDefaultEffect();

    // copied from DestinyManager::UpdateShipVariables()
    /* The product of Mass and InertiaMod gives the item's Agility
     *  Agility = Mass x InertiaMod / 1000000
     *  Agility is an internal server variable.
     */
    double mass = dRef->GetAttribute(AttrMass).get_double();
    double inertiaMod = dRef->GetAttribute(AttrInertiaMod).get_double();

    if (mass < 99.0) {
        sLog.Warning("DroneAI::Init()", " %s  has no mass defined.  setting to 1000.0", dRef->name());
        mass = 1000.0;
        dRef->SetAttribute(AttrMass, mass, false);
    }
    if (inertiaMod < 99.0) {
        sLog.Warning("DroneAI::Init()", "%s  has no inertiaMod defined.  setting to 100.0", dRef->name());
        inertiaMod = 100.0;
        dRef->SetAttribute(AttrInertiaMod, inertiaMod, false);
    }

    m_agility = mass * inertiaMod / 1000000;
    dRef->SetAttribute(AttrAgility, m_agility, false);

    m_alignTime = 1.386294 * m_agility;
    m_accelTime = (-log(ASF_CHECK) * m_agility);
    if (m_accelTime < 1.0f)
        m_accelTime = 1.0f;

    _log(DRONE__MESSAGE, "DroneAI::Init() - %s  agility: %.4f, acceltime: %.2f", dRef->name(), m_agility, m_accelTime);
}

void DroneAIMgr::Process() {
    // disabled and invalid have movetime=0 to avoid tics;  nothing to process till new command received
    if (m_moveTime == 0)
        return;

    bool move(true);
    double timeStamp(0);

    // keep timer in seconds.
    timeStamp = ((GetTimeMSeconds() - m_moveTime) * 0.001);
    // update tf for this tic
    m_timeFraction = (1 - exp(-timeStamp / m_agility));

    _log(DRONE__AI_TRACE, "%s(%u) Proc(%.0f) - %s(%s)  tf:%.2f.", \
            mySE->GetName(), mySE->GetID(), timeStamp, \
            GetStateName(m_state), GetActionName(m_action), m_timeFraction);

    // for OrbitShip & OrbitTarget,  determine if target has moved outside of orbit range and reset drone
    //   by doing this, i cannot set m_moveTime=0 for these....this HAS to Process() in order to work right.
    //   also, cannot hit Move() at end...that will totally change position (but code should compensate for it next tic)
    uint32 targDistance(0);
    uint32 shipDistance(mySE->GetPosition().distance(shipSE->GetPosition()));
    shipDistance -= shipSE->GetRadius();
    // should we check for targDistance==0 in here?
    if (targSE != nullptr)
        targDistance = mySE->GetPosition().distance(targSE->GetPosition() - targSE->GetRadius());

    switch(m_state) {
        case DroneAI::State::Idle: {
            // orbiting controlling ship  do nothing until next command
            //  we can put actions here to return to ship, home, etc
            switch (m_action) {
                case DroneAI::Action::DecelToStop: {
                    // check speed fraction to stop ship here or in Move()?  do it in move
                    _log(DRONE__AI_TRACE, "%s - Proc() - Idle and decel to stop", mySE->GetName());
                    // stop processing until another call hits
                    return;
                } break;
                case DroneAI::Action::OrbitShip: {
                    // has target moved any?
                    if (shipDistance > m_orbitDistance) {
                        MoveDrone(shipSE);
                        _log(DRONE__AI_TRACE, "Move() - outside orbit distance.");
                        return;
                    }
                    // idle and orbiting.
                    _log(DRONE__AI_TRACE, "%s - Proc() - orbiting home ship", mySE->GetName());
                    // stop processing until another call hits
                    return;
                } break;
                case DroneAI::Action::OrbitTarget: {
                    // has target moved any?
                    if (targDistance > m_orbitDistance) {
                        MoveDrone(targSE);
                        _log(DRONE__AI_TRACE, "Move() - outside orbit distance.");
                        return;
                    }
                    // idle and orbiting.
                    _log(DRONE__AI_TRACE, "%s - Proc() - orbiting home ship", mySE->GetName());
                    move = false;
                } break;
                default: {
                    _log(DRONE__ERROR, "%s - state is %s but action is %s.", \
                            mySE->GetName(), GetStateName(m_state), GetActionName(m_action));
                    SetIdle();
                } break;
            }
        } break;

        case DroneAI::State::Mining: {
            // determine drone step and act accordingly
            switch (m_action) {
                case DroneAI::Action::AccelToTarget:
                case DroneAI::Action::DecelToTarget: {
                    // drone traveling to asteroid
                    if (InOrbitDistance(targSE)) {
                         _log(DRONE__AI_TRACE, "%s - arrived at target. begin mining.", mySE->GetName());
                        // we are close enough to begin.   stop moving and start mining
                        SendTrueState(DroneAI::State::Mining);
                        SetAction(DroneAI::Action::Engaged);
                        // begin mining timer
                        m_processTimer.Start(m_cycleTime);
                        m_startTime = GetFileTimeNow();
                        m_moveTime = GetTimeMSeconds();
                        sLog.Error("movetime", "set - %s(%s)", GetStateName(m_state), GetActionName(m_action));
                        SendGFX();
                        move = false;
                       }
                } break;
                case DroneAI::Action::OrbitShip:
                    //TODO: LATER: roids should not move, except for the weird belt vortex planned for later.
                case DroneAI::Action::AccelToShip:
                case DroneAI::Action::DecelToShip: {
                    // drone returning to ship.   interaction distance is set in config.drone.InteractDistace and defaults to 2k5m
                    if (InActionDistance(shipSE)) {
                        // we have returned.  drop ore and return to mine, if commanded
                        if (shipSE->GetSelf()->GetMyInventory()->HasAvailableSpace(m_holdFlag, m_ore)) {
                            // automagically stack ore in hold.  this is a feature.
                            m_ore->MergeTypesInCargo(shipSE->GetShipItemRef().get(), m_holdFlag);
                            _log(DRONE__AI_TRACE, "%s - dropped ore to ship.", mySE->GetName());
                        } else {
                            shipSE->GetPilot()->SendNotifyMsg("Your %s deactivates mining operations as it couldn't add the %s ore to your %s.", \
                                    mySE->GetName(), m_ore->name(), sDataMgr.GetFlagName(m_holdFlag));
                            _log(DRONE__AI_TRACE, "%s - error adding ore to ship.  return to idle.", mySE->GetName());
                            move = false;
                            SetAction(DroneAI::Action::DecelToStop);
                            SetIdle();
                            break;
                        }
                        if (m_repeat) {
                            _log(DRONE__AI_TRACE, "%s - return to target.", mySE->GetName());
                            // mine, drop, return, rinse, repeat
                            SendTrueState(DroneAI::State::Approaching);
                            SetAction(DroneAI::Action::AccelToTarget);
                            m_sendCmd = true;
                            OrbitTarget();
                        } else {
                            // nope, single use only.
                            _log(DRONE__AI_TRACE, "%s - return to idle.", mySE->GetName());
                            move = false;
                            SetIdle();
                            break;
                        }
                    }
                } break;
                case DroneAI::Action::Engaged: {
                    move = false;
                    // has mining cycle finished?
                    if (m_processTimer.Check(false)) {
                        _log(DRONE__AI_TRACE, "%s - mining cycle complete.  return to ship.", mySE->GetName());
                        SendTrueState(DroneAI::State::ReturnHome);
                        SetAction(DroneAI::Action::AccelToShip);
                        m_moveTime = GetTimeMSeconds();
                        sLog.Error("movetime", "set - complete %s(%s)", GetStateName(m_state), GetActionName(m_action));
                        // get mined ore
                        MineTarget();
                        // stop timer
                        m_processTimer.Disable();
                        // return to ship
                        m_sendCmd = true;
                        OrbitTarget();
                        move = true;
                    }
                } break;
                case DroneAI::Action::OrbitTarget: {
                    // already at asteroid
                    if (InOrbitDistance(targSE)) {
                        _log(DRONE__AI_TRACE, "%s - arrived at target. begin mining.", mySE->GetName());
                        move = false;
                        // we are close enough to begin  stop moving and start mining
                        SendTrueState(DroneAI::State::Mining);
                        SetAction(DroneAI::Action::Engaged);
                        // begin mining timer
                        m_processTimer.Start(m_cycleTime);
                        m_startTime = GetFileTimeNow();
                        m_moveTime = GetTimeMSeconds();
                        sLog.Error("movetime", "set - start %s(%s)", GetStateName(m_state), GetActionName(m_action));
                        SendGFX();
                    } else {
                        _log(DRONE__AI_TRACE, "%s - too far from target to engage. begin travel.", mySE->GetName());
                        SendTrueState(DroneAI::State::Approaching);
                        SetAction(DroneAI::Action::AccelToTarget);
                        OrbitTarget();
                    }
                } break;
                case DroneAI::Action::DecelToStop: {
                    _log(DRONE__AI_TRACE, "%s - mining cycle complete.  decel to stop.", mySE->GetName());
                    SetIdle();
                } break;
                default: {
                    // this isnt right...action should never be idle when mining
                    _log(DRONE__ERROR, "%s - state is %s but action is %s.", \
                            mySE->GetName(), GetStateName(m_state), GetActionName(m_action));
                    move = false;
                    SetIdle();
                } break;
            }
        } break;

        case DroneAI::State::Repairing: {
            if (!ValidTarget()) {
                move = false;
                SetIdle();
                break;
            }
            switch (m_action) {
                case DroneAI::Action::Engaged: {
                    move = false;
                    // has target moved any?
                    if (targDistance > m_orbitDistance) {
                        MoveDrone(targSE);
                        _log(DRONE__AI_TRACE, "Move() - outside orbit distance.");
                        // cannot return from here.
                    }
                    if (m_processTimer.Check())
                        RepairTarget();
                } break;
                case DroneAI::Action::OrbitTarget:
                case DroneAI::Action::AccelToTarget:
                case DroneAI::Action::DecelToTarget: {
                    if (InEngageDistance(targSE)) {
                        move = false;
                        m_startTime = GetFileTimeNow();
                        SetAction(DroneAI::Action::Engaged);
                        SendTrueState(DroneAI::State::Repairing);
                        m_processTimer.Start(m_cycleTime);
                        // this is a repeatable action
                        m_repeat = true;
                        SendGFX();
                        RepairTarget();
                    }
                } break;
                case DroneAI::Action::DecelToStop: {
                    move = false;
                    SetIdle();
                } break;
                /*  these should not hit here  make error below
                case DroneAI::Action::OrbitShip:
                case DroneAI::Action::AccelToShip:
                case DroneAI::Action::DecelToShip: {
                    // currently traveling or idle.  set new command
                } break;
                */
                default: {
                    // error
                    _log(DRONE__AI_TRACE, "%s - hmmmmm... state is Engaged but action is %s.", \
                            mySE->GetName(), GetActionName(m_action));
                    move = false;
                    SetIdle();
                } break;
            }
        } break;

        case DroneAI::State::ReturnHome: {
            switch (m_action) {
                case DroneAI::Action::AccelToShip:
                case DroneAI::Action::DecelToShip: {
                    if (InOrbitDistance(shipSE)) {
                        _log(DRONE__AI_TRACE, "%s - close enough.  orbiting ship.", mySE->GetName());
                        move = false;
                        SetAction(DroneAI::Action::OrbitShip);
                        OrbitTarget();
                    }
                } break;
                case DroneAI::Action::OrbitTarget: {
                    _log(DRONE__AI_TRACE, "%s - leaving target and returning home.", mySE->GetName());
                    move = false;
                    SetAction(DroneAI::Action::AccelToShip);
                    OrbitTarget();
                } break;
                case DroneAI::Action::OrbitShip: {
                    _log(DRONE__AI_TRACE, "%s - has returned home.  orbiting home ship", mySE->GetName());
                    move = false;
                    SendTrueState(DroneAI::State::Idle);
                    SetState(DroneAI::State::Idle);
                    Pause();
                } break;
                case DroneAI::Action::Idle: {
                    _log(DRONE__AI_TRACE, "%s - hmmmmm... state is ReturnHome but action is idle.", mySE->GetName());
                    move = false;
                    SetIdle();
                } break;
                case DroneAI::Action::DecelToStop: {
                    _log(DRONE__AI_TRACE, "%s - returning home.  decel to stop", mySE->GetName());
                    move = false;
                    SetIdle();
                } break;
                default: {
                    // error
                    _log(DRONE__AI_TRACE, "%s - hmmmmm... state is ReturnHome but action is %s.", \
                            mySE->GetName(), GetActionName(m_action));
                    move = false;
                    SetIdle();
                }
            }
        } break;

        case DroneAI::State::ReturnBay: {
            switch (m_action) {
                case DroneAI::Action::OrbitShip:
                case DroneAI::Action::AccelToShip:
                case DroneAI::Action::DecelToShip: {
                    // should we check for available room in bay?  probably so
                    if (InActionDistance(shipSE)) {
                        //if (shipSE->GetMyInventory()->ValidateAddItem(flagDroneBay, mySE->GetSelf()))  // this will throw if it fails
                        _log(DRONE__AI_TRACE, "%s - close enough.  docking to bay.", mySE->GetName());
                        shipSE->ScoopDrone(mySE);
                        shipSE->GetPilot()->MoveItem(mySE->GetID(), shipSE->GetID(), flagDroneBay);
                        mySE->SystemMgr()->RemoveEntity(mySE);
                        mySE->SystemMgr()->AddToDeleteLater(mySE);
                        return;
                    }
                } break;
                case DroneAI::Action::OrbitTarget: {
                    _log(DRONE__AI_TRACE, "%s - leaving target and returning home.", mySE->GetName());
                    move = false;
                    SetAction(DroneAI::Action::AccelToShip);
                    OrbitTarget();
                } break;
                case DroneAI::Action::Idle: {
                    _log(DRONE__AI_TRACE, "%s - hmmmmm... state is ReturnBay but action is idle.", mySE->GetName());
                    move = false;
                    SetIdle();
                } break;
                case DroneAI::Action::DecelToStop: {
                    _log(DRONE__AI_TRACE, "%s - returning to bay.  decel to stop", mySE->GetName());
                    SetState(DroneAI::State::Idle);
                    // stop processing until another call hits
                    move = false;
                    Pause();
                } break;
                default: {
                    // error
                    _log(DRONE__AI_TRACE, "%s - hmmmmm... state is ReturnBay but action is %s.  Resetting.", \
                            mySE->GetName(), GetActionName(m_action));
                    move = false;
                    SetAction(DroneAI::Action::AccelToShip);
                    OrbitTarget();
                }
            }
        } break;

        case DroneAI::State::Combat:
        case DroneAI::State::Operating: {
            if (!ValidTarget()) {
                move = false;
                SetIdle();
                break;
            }
            switch (m_action) {
                case DroneAI::Action::Engaged: {
                    // see if we're still at full speed; if so and within falloff, reset speed for orbit
                    if (m_userSpeedFraction > 0.5f) {
                        if (InFalloffDistance(targSE))
                            OrbitTarget();
                    } else if (targDistance > m_orbitDistance) {
                        // has target moved any?
                        MoveDrone(targSE);
                        _log(DRONE__AI_TRACE, "Move() - outside orbit distance.");
                        // cannot return from here.
                    }
                    // this is actively fighting, whether ewar or weapons
                    move = false;
                    if (m_processTimer.Check())
                        AttackTarget();
                } break;
                case DroneAI::Action::OrbitTarget:
                case DroneAI::Action::AccelToTarget:
                case DroneAI::Action::DecelToTarget: {
                    if (InEngageDistance(targSE)) {
                        move = false;
                        m_startTime = GetFileTimeNow();
                        SetAction(DroneAI::Action::Engaged);
                        SendTrueState(m_state); // sending actual state (either Combat or Operating)
                        m_processTimer.Start(m_cycleTime);
                        // this is a repeatable action
                        m_repeat = true;
                        SendGFX();
                        AttackTarget();
                    }
                } break;
                case DroneAI::Action::DecelToStop: {
                    // this is almost always an error
                    _log(DRONE__AI_TRACE, "%s - %s.  decel to stop", mySE->GetName(), GetStateName(m_state));
                    SetState(DroneAI::State::Idle);
                    // stop processing until another call hits
                    move = false;
                    m_moveTime = 0;
                    sLog.Error("movetime", "0 - %s(%s)", GetStateName(m_state), GetActionName(m_action));
                } break;
                /*  these should not hit here.  make error below
                case DroneAI::Action::Idle:
                case DroneAI::Action::Invalid:
                case DroneAI::Action::OrbitShip:
                case DroneAI::Action::AccelToShip:
                case DroneAI::Action::DecelToShip:
                */
                default: {
                    // error
                    _log(DRONE__AI_TRACE, "%s - hmmmmm... state is %s but action is %s.", \
                    mySE->GetName(), GetStateName(m_state), GetActionName(m_action));
                    SetState(DroneAI::State::Idle);
                    // stop processing until another call hits
                    move = false;
                    Pause();
                }
            }
        } break;

        // may have to separate these later, but for now they use common code
        case DroneAI::State::Guarding:
        case DroneAI::State::Assisting:{
            // this one will need proximity system and aggression warning from assigned ship
            switch (m_action) {
                case DroneAI::Action::Invalid: {
                } break;
                case DroneAI::Action::Idle: {
                    SetAction(DroneAI::Action::OrbitShip);
                    OrbitTarget();
                } break;
                case DroneAI::Action::Engaged: {
                    if (targDistance > m_orbitDistance) {
                        // has target moved any?
                        MoveDrone(targSE);
                        _log(DRONE__AI_TRACE, "Move() - outside orbit distance.");
                        // cannot return from here.
                    }
                    move = false;
                    if (m_processTimer.Check())
                        AttackTarget();
                } break;
                case DroneAI::Action::OrbitTarget:
                case DroneAI::Action::AccelToTarget:
                case DroneAI::Action::DecelToTarget: {
                    move = true;
                    if (InEngageDistance(targSE)) {
                        //TODO:  determine if state is combat or repair
                        m_startTime = GetFileTimeNow();
                        SetAction(DroneAI::Action::Engaged);
                        SendTrueState(DroneAI::Action::Engaged);
                        m_processTimer.Start(m_cycleTime);
                        // this is a repeatable action
                        m_repeat = true;
                        SendGFX();
                        AttackTarget();
                        move = false;
                    } else {
                        OrbitTarget();
                    }
                } break;
                case DroneAI::Action::OrbitShip:
                case DroneAI::Action::AccelToShip:
                case DroneAI::Action::DecelToShip: {
                    move = true;
                    if (InEngageDistance(shipSE)) {
                        //TODO:  determine if state is combat or repair
                        m_startTime = GetFileTimeNow();
                        SetAction(DroneAI::Action::Engaged);
                        SendTrueState(DroneAI::Action::Engaged);
                        m_processTimer.Start(m_cycleTime);
                        // this is a repeatable action
                        m_repeat = true;
                        SendGFX();
                        AttackTarget();
                        move = false;
                    } else {
                        OrbitTarget();
                    }
                } break;
                case DroneAI::Action::DecelToStop: {
                    // this is almost always an error
                    _log(DRONE__AI_TRACE, "%s - %s.  decel to stop", mySE->GetName(), GetStateName(m_state));
                    SetState(DroneAI::State::Idle);
                    // stop processing until another call hits
                    move = false;
                    Pause();
                } break;
                default: {
                    // error
                    _log(DRONE__AI_TRACE, "%s - hmmmmm... state is %s but action is %s.", \
                    mySE->GetName(), GetStateName(m_state), GetActionName(m_action));
                    SetState(DroneAI::State::Idle);
                    // stop processing until another call hits
                    move = false;
                    Pause();
                } break;
            }
        } break;

        case DroneAI::State::Fleeing: {
            // this will almost always be "running home to momma"
            switch (m_action) {
                case DroneAI::Action::Idle:
                case DroneAI::Action::Engaged:
                case DroneAI::Action::DecelToStop:
                case DroneAI::Action::OrbitTarget:
                case DroneAI::Action::AccelToTarget:
                case DroneAI::Action::DecelToTarget: {
                    SetState(DroneAI::State::ReturnHome);
                    SetAction(DroneAI::Action::AccelToShip);
                    OrbitTarget();
                } break;
                case DroneAI::Action::OrbitShip:
                case DroneAI::Action::AccelToShip:
                case DroneAI::Action::DecelToShip:{
                    SetState(DroneAI::State::Idle);
                    SetAction(DroneAI::Action::OrbitShip);
                    OrbitTarget();
                } break;
                default:
                case DroneAI::Action::Invalid: {
                    // error
                    _log(DRONE__AI_TRACE, "%s - hmmmmm... state is %s but action is %s.", \
                    mySE->GetName(), GetStateName(m_state), GetActionName(m_action));
                    SetState(DroneAI::State::Idle);
                    // stop processing until another call hits
                    move = false;
                    m_moveTime = 0;
                    sLog.Error("movetime", "0 - %s(%s)", GetStateName(m_state), GetActionName(m_action));
                } break;
            }

        } break;

        // these should never hit...only for status updates we send to client
        case DroneAI::State::Invalid:
        case DroneAI::State::Pursuit:
        case DroneAI::State::Approaching: {
            // check everything in this state.   return to ship?
            _log(DRONE__ERROR, "%s - state is %s but action is %s.", \
                    mySE->GetName(), GetStateName(m_state), GetActionName(m_action));
            move = false;
            SetIdle();
        } break;

        default: {
            _log(DRONE__ERROR, "%s - Hit Default:  state is %s but action is %s.", \
                    mySE->GetName(), GetStateName(m_state), GetActionName(m_action));
            move = false;
            SetIdle();
        }
    }

    if (move)
        Move(timeStamp);
}

void DroneAIMgr::Engage(PyDict* dict, int8 state/*0*/, bool repeat/*0*/) {
    // this is the main entry point into drone movement and actions

    //  so....we gotta decide what the hell's going on here to start with...

    // are we currently doing anything?
    bool busy(m_state > DroneAI::State::Idle);

    sLog.Green("Engage()", "call is [%s] - currently %s(%s) and %sBusy", \
            GetStateName(state), GetStateName(m_state), GetActionName(m_action), busy?"":"not ");

    switch (state) {
        // return supersedes all.
        case DroneAI::State::ReturnBay: {
            // this is easy...drop everything and return
            if (m_state == DroneAI::State::ReturnBay) {
                // same call.  ignore it
                return;
            } else if (m_state == DroneAI::State::ReturnHome) {
                // changed mind, dock to bay
                SetState(DroneAI::State::ReturnBay);
                // targ, heading, distance, etc already set.
                // are we currently orbiting?  check m_moveTime
                if (m_moveTime == 0) {
                    m_moveTime = GetTimeMSeconds();
                    sLog.Error("movetime", "set - %s(%s)", GetStateName(m_state), GetActionName(m_action));
                }
                return;
            } else {
                SetState(DroneAI::State::ReturnBay);
                // determine target distance to set action
                if (InOrbitDistance(shipSE)) {
                    SetAction(DroneAI::Action::OrbitShip);
                    SendTrueState(DroneAI::State::ReturnBay);
                } else {
                    SetAction(DroneAI::Action::AccelToShip);
                    SendTrueState(DroneAI::State::ReturnBay);
                }
            }
            m_sendCmd = true;
        } break;
        case DroneAI::State::ReturnHome: {
            // this is easy...drop everything and return
            if (m_state == DroneAI::State::ReturnHome) {
                // same call.  ignore it
                return;
            } else if (m_state == DroneAI::State::ReturnBay) {
                // changed mind, orbit home
                SetState(DroneAI::State::ReturnHome);
                // targ, heading, distance, etc already set.
                // are we currently orbiting?  check m_moveTime
                if (m_moveTime == 0) {
                    m_moveTime = GetTimeMSeconds();
                    sLog.Error("movetime", "set - %s(%s)", GetStateName(m_state), GetActionName(m_action));
                }
                return;
            } else {
                SetState(DroneAI::State::ReturnHome);
                // determine target distance to set action
                if (InOrbitDistance(shipSE)) {
                    SetAction(DroneAI::Action::OrbitShip);
                    SendTrueState(DroneAI::State::ReturnHome);
                } else {
                    SetAction(DroneAI::Action::AccelToShip);
                    SendTrueState(DroneAI::State::ReturnHome);
                }
            }
            m_sendCmd = true;
        } break;
        case DroneAI::State::Mining: {
            switch (mySE->GetGroupID()) {
                case EVEDB::invGroups::Combat_Drone:
                case EVEDB::invGroups::Fighter_Drone:
                case EVEDB::invGroups::Fighter_Bomber:
                case EVEDB::invGroups::Logistic_Drone:
                case EVEDB::invGroups::Cap_Drain_Drone:
                case EVEDB::invGroups::Stasis_Webifying_Drone:
                case EVEDB::invGroups::Electronic_Warfare_Drone: {
                    sLog.Error("DroneAI::Engage()", "%s drone sent for %s.  wtf?", \
                            mySE->GetSelf()->type().groupName().c_str(), GetStateName(state));
                    PyDict* data = new PyDict();
                    data->SetItemString("targetTypeName", new PyString(mySE->GetName()));
                    PyTuple* error = new PyTuple(2);
                    error->SetItem(0, new PyString("EntityUnknownCommand"));
                    error->SetItem(1, data);
                    dict->SetItem(new PyInt(mySE->GetID()), error);
                    return;
                } break;
            }
            // are we currently mining?  why would this be commanded again?  operator error?
            if (m_state == DroneAI::State::Mining) {
                m_repeat = false;
                PyDict* data = new PyDict();
                data->SetItemString("targetTypeName", new PyString(mySE->GetName()));
                PyTuple* error = new PyTuple(2);
                error->SetItem(0, new PyString("EntityCurrentlyMining"));
                error->SetItem(1, data);
                dict->SetItem(new PyInt(mySE->GetID()), error);
                return;
            }

            m_sendCmd = true;
            m_repeat = repeat;  // mine once or mine repeatedly?
            SetState(DroneAI::State::Mining);

            // determine target distance to set action
            if (InOrbitDistance(targSE)) {
                SetAction(DroneAI::Action::OrbitTarget);
                SendTrueState(DroneAI::State::Mining);
            } else {
                SetAction(DroneAI::Action::AccelToTarget);
                SendTrueState(DroneAI::State::Approaching);
            }
        } break;
        case DroneAI::State::Combat: {
            switch (mySE->GetGroupID()) {
                case EVEDB::invGroups::Mining_Drone:
                case EVEDB::invGroups::Logistic_Drone: {
                    sLog.Error("DroneAI::Engage()", "%s drone sent for %s.  wtf?", \
                            mySE->GetSelf()->type().groupName().c_str(), GetStateName(state));
                    PyDict* data = new PyDict();
                    data->SetItemString("targetTypeName", new PyString(mySE->GetName()));
                    PyTuple* error = new PyTuple(2);
                    error->SetItem(0, new PyString("EntityUnknownCommand"));
                    error->SetItem(1, data);
                    dict->SetItem(new PyInt(mySE->GetID()), error);
                    return;
                } break;
            }
            m_sendCmd = true;
            SetState(DroneAI::State::Combat);
            // determine target distance to set action
            if (InOrbitDistance(targSE)) {
                SetAction(DroneAI::Action::OrbitTarget);
                SendTrueState(DroneAI::State::Combat);
            } else {
                SetAction(DroneAI::Action::AccelToTarget);
                SendTrueState(DroneAI::State::Pursuit);
            }
        } break;
        case DroneAI::State::Repairing: {
            switch (mySE->GetGroupID()) {
                case EVEDB::invGroups::Combat_Drone:
                case EVEDB::invGroups::Mining_Drone:
                case EVEDB::invGroups::Fighter_Drone:
                case EVEDB::invGroups::Fighter_Bomber:
                case EVEDB::invGroups::Cap_Drain_Drone:
                case EVEDB::invGroups::Stasis_Webifying_Drone:
                case EVEDB::invGroups::Electronic_Warfare_Drone:  {
                    sLog.Error("DroneAI::Engage()", "%s drone sent for %s.  wtf?", \
                            mySE->GetSelf()->type().groupName().c_str(), GetStateName(state));
                    PyDict* data = new PyDict();
                    data->SetItemString("targetTypeName", new PyString(mySE->GetName()));
                    PyTuple* error = new PyTuple(2);
                    error->SetItem(0, new PyString("EntityUnknownCommand"));
                    error->SetItem(1, data);
                    dict->SetItem(new PyInt(mySE->GetID()), error);
                    return;
                } break;
            }
            m_sendCmd = true;
            SetState(DroneAI::State::Repairing);
            // determine target distance to set action
            if (InOrbitDistance(targSE)) {
                SetAction(DroneAI::Action::OrbitTarget);
            } else {
                SetAction(DroneAI::Action::AccelToTarget);
                SendTrueState(DroneAI::State::Approaching);
            }
        } break;
        case DroneAI::State::Assisting: {
            // engage assigned ship's active target
            m_sendCmd = true;
            targSE = shipSE->TargetMgr()->GetFirstTarget();
            SetState(DroneAI::State::Assisting);
            if (targSE == nullptr) {
                // no current target...set idle
                SetAction(DroneAI::Action::Idle);
            } else if (InEngageDistance(targSE)) {
                m_startTime = GetFileTimeNow();
                SetAction(DroneAI::Action::Engaged);
                SendTrueState(DroneAI::State::Combat);
                m_processTimer.Start(m_cycleTime);
                // this is a repeatable action
                m_repeat = true;
                SendGFX();
                AttackTarget();
            } else {
                SetAction(DroneAI::Action::AccelToTarget);
                SendTrueState(DroneAI::State::Approaching);
            }
        } break;
        case DroneAI::State::Guarding:{
            // can Logistic_Drone be set to guard?  no...combat only
            switch (mySE->GetGroupID()) {
                case EVEDB::invGroups::Mining_Drone:
                case EVEDB::invGroups::Logistic_Drone:
                case EVEDB::invGroups::Cap_Drain_Drone:
                case EVEDB::invGroups::Stasis_Webifying_Drone:
                case EVEDB::invGroups::Electronic_Warfare_Drone:  {
                    sLog.Error("DroneAI::Engage()", "%s drone sent for %s.  wtf?", \
                            mySE->GetSelf()->type().groupName().c_str(), GetStateName(state));
                    PyDict* data = new PyDict();
                    data->SetItemString("targetTypeName", new PyString(mySE->GetName()));
                    PyTuple* error = new PyTuple(2);
                    error->SetItem(0, new PyString("EntityUnknownCommand"));
                    error->SetItem(1, data);
                    dict->SetItem(new PyInt(mySE->GetID()), error);
                    return;
                } break;
            }
            m_sendCmd = true;
            // check mode
            if (mySE->GetSelf()->GetAttribute(AttrDroneIsAgressive).get_bool()
            or mySE->GetSelf()->GetAttribute(AttrFightersAttackAndFollow).get_bool()) {
                // aggressive mode.  pick next and engage
                FindTarget();
                return;
            } else {
                // passive.
                SetState(DroneAI::State::Idle);
            }
        } break;
        case DroneAI::State::Operating: {        // cap drain and ewar
            switch (mySE->GetGroupID()) {
                case EVEDB::invGroups::Combat_Drone:
                case EVEDB::invGroups::Mining_Drone:
                case EVEDB::invGroups::Fighter_Drone:
                case EVEDB::invGroups::Fighter_Bomber: {
                    // this should never hit
                    sLog.Error("DroneAI::Engage()", "%s drone sent for %s.  wtf?", \
                            mySE->GetSelf()->type().groupName().c_str(), GetStateName(state));
                    PyDict* data = new PyDict();
                    data->SetItemString("targetTypeName", new PyString(mySE->GetName()));
                    PyTuple* error = new PyTuple(2);
                    error->SetItem(0, new PyString("EntityUnknownCommand"));
                    error->SetItem(1, data);
                    dict->SetItem(new PyInt(mySE->GetID()), error);
                    return;
                } break;
            }
            m_sendCmd = true;
            SetState(DroneAI::State::Operating);
            // determine target distance to set action
            if (InOrbitDistance(targSE)) {
                SetAction(DroneAI::Action::OrbitTarget);
                SendTrueState(DroneAI::State::Operating);
            } else {
                SetAction(DroneAI::Action::AccelToTarget);
                SendTrueState(DroneAI::State::Approaching);
            }
        } break;
        // these should not be sent as they are actions and not commands
        case DroneAI::State::Idle:
        case DroneAI::State::Fleeing:
        case DroneAI::State::Pursuit:
        case DroneAI::State::Approaching:
        case DroneAI::State::Incapacitated: {
            sLog.Warning("DroneEngage()", "sent %s...why?", GetStateName(state));
            return;
        } break;
    }

    OrbitTarget();
}

void DroneAIMgr::SetIdle() {
    if (m_state == DroneAI::State::Idle)
        return;
    if (!mySE->IsEnabled())
        return;

    _log(DRONE__AI_TRACE, "%s(%u): SetIdle: returning to idle.",
            mySE->GetName(), mySE->GetID());

    // in case drone was engaged and we need to cancel gfx.
    bool previousOrder(m_state > DroneAI::State::Idle);

    // if drone engaged in repeat action, cancel gfx
    if (m_repeat) {
        m_repeat = false;
        // ...except miners
        if (mySE->GetGroupID() != EVEDB::invGroups::Mining_Drone) {
            // set action to idle first.  gfx checks this to set active
            SetAction(DroneAI::Action::Idle);
            if (previousOrder) {
                sLog.Warning("DroneAIMgr::SetIdle(repeat)", " state is %s for %s.  Canceling GFX", GetStateName(m_state), mySE->GetName());
                // probably had some active gfx before returning to idle
                SendGFX();
            }
        }
    }

    m_startTime = 0;
    ClearTargets();

    // disable timers
    m_processTimer.Disable();
    m_webifierTimer.Disable();
    m_beginFindTarget.Disable();
    m_warpScramblerTimer.Disable();

    if (mySE->InControlDistance()) {
        // update orbit command
        if ((m_chaseDistance < 1000)
        and (mySE->GetGroupID() != EVEDB::invGroups::Mining_Drone)) {
            // stationary drone
            SendTrueState(DroneAI::State::Idle);
            SetState(DroneAI::State::Idle);
            SetAction(DroneAI::Action::Idle);
            Pause();
            return;
        }
        m_sendCmd = true;
        SetState(DroneAI::State::ReturnHome);
        if (InOrbitDistance(shipSE)) {
            SendTrueState(DroneAI::State::Idle);
            SetAction(DroneAI::Action::OrbitShip);
            // orbit must be called before pause
            OrbitTarget();
            Pause();
        } else {
            SendTrueState(DroneAI::State::ReturnHome);
            SetAction(DroneAI::Action::AccelToShip);
            OrbitTarget();
        }
    } else {
        if (previousOrder) {
            // if drone was following target or assigned ship and is out of control range, cruise back to home ship
            SendTrueState(DroneAI::State::ReturnHome);
            SetAction(DroneAI::Action::AccelToShip);
            OrbitTarget();
        } else {
        if (mySE->IsEnabled()) {
            // we're idle and outside control distance.  not good
            mySE->DisableDrone();
            shipSE->ReleaseBandwidth(mySE);
        }
        m_moveTime = 0;
        sLog.Error("movetime", "0 - SetIdle()  %s(%s)", GetStateName(m_state), GetActionName(m_action));
        SendTrueState(DroneAI::State::Invalid);
        SetState(DroneAI::State::Invalid);
        SetAction(DroneAI::Action::Invalid);
        }
    }
}

void DroneAIMgr::Target(SystemEntity* pTarget) {
    // check for changing targets first
    if (targSE == pTarget) {
        // same target.  do nothing
        return;
    } else {
        // different target.  check current status
    }

    _log(DRONE__AI_TRACE, "%s(%u) Target() - %s(%s).", \
    mySE->GetName(), mySE->GetID(), GetStateName(m_state), GetActionName(m_action));

    // all is good, set new target
    targSE = pTarget;
    bool chase(false);  // chase ref isnt used for drones
    if (!mySE->TargetMgr()->StartTargeting(pTarget,
                                            mySE->GetSelf()->GetAttribute(AttrScanSpeed).get_float(),
                                            (uint8)mySE->GetSelf()->GetAttribute(AttrMaxLockedTargets).get_int(),
                                            shipSE->GetSelf()->GetAttribute(AttrMaxTargetRange).get_double(), chase))
    {
        _log(DRONE__AI_TRACE, "Drone %s(%u): Targeting of %s(%u) failed.  Clear Target and Return to Idle.",
             mySE->GetName(), mySE->GetID(), pTarget->GetName(), pTarget->GetID());
        //TODO:  target is called before setting state.  figure a way to tell AI targeting failed instead of setting idle
        SetIdle();
        return;
    }

    // zero-out the heading
    m_heading = NULL_ORIGIN_V;

    m_beginFindTarget.Disable();
    // test  do we need to set timer here?
}

void DroneAIMgr::ClearTarget() {
    mySE->TargetMgr()->ClearTarget(targSE);
    //mySE->TargetMgr()->OnTarget(pSE, TargMgr::Mode::Lost);

    targSE = nullptr;
    if (mySE->TargetMgr()->HasNoTargets())
        SetIdle();
}

void DroneAIMgr::RepairTarget() {
    _log(DRONE__AI_TRACE, "Drone %s(%u): RepairTarget: %s(%u) begin engaging.",
            mySE->GetName(), mySE->GetID(), targSE->GetName(), targSE->GetID());

    // determine drone type, perform appropriate action
    switch (mySE->GetGroupID()) {
        case EVEDB::invGroups::Logistic_Drone: {
            //Logistic_Drone  - either shield or armor
            if (mySE->GetSelf()->HasAttribute(AttrEntityShieldBoostAmount)) {
                // shield repper
                float shieldHP = targSE->GetSelf()->GetAttribute(AttrShieldCharge).get_float();
                shieldHP += mySE->GetSelf()->GetAttribute(AttrEntityShieldBoostAmount).get_float();
                // verify we're not going over...
                if (shieldHP > targSE->GetSelf()->GetAttribute(AttrShieldCapacity).get_float())
                    shieldHP = targSE->GetSelf()->GetAttribute(AttrShieldCapacity).get_float();
                targSE->GetSelf()->SetAttribute(AttrShieldCharge, shieldHP);
            } else if (mySE->GetSelf()->HasAttribute(AttrEntityArmorRepairAmount)) {
                // armor repper
                float armorHP = targSE->GetSelf()->GetAttribute(AttrArmorDamage).get_float();
                armorHP -= mySE->GetSelf()->GetAttribute(AttrEntityArmorRepairAmount).get_float();
                // verify we're not going over...
                if (armorHP < 0)
                    armorHP = 0;
                targSE->GetSelf()->SetAttribute(AttrArmorDamage, armorHP);
            }
        } break;
        default: {
            _log(DRONE__WARNING, "DroneAI::RepairTarget() - %s(%u) of %s is %s(%s) and calling RepairTarget()", \
                    mySE->GetName(), mySE->GetID(), mySE->GetSelf()->type().groupName().c_str(), \
                    GetStateName(m_state), GetActionName(m_action));
        } break;
    }
}


//also check for special effects and write code to implement them
//modifyTargetSpeedRange, modifyTargetSpeedChance
//entityWarpScrambleChance
void DroneAIMgr::AttackTarget() {
    if (!mySE->TargetMgr()->CanAttack())
        return;

    // determine what we're supposed to be doing here...
    switch (mySE->GetGroupID()) {
        // probably gonna be most common...
        case EVEDB::invGroups::Combat_Drone:
        case EVEDB::invGroups::Fighter_Drone:
        case EVEDB::invGroups::Fighter_Bomber: {
            // damage attack here...
            Damage dam(mySE,
                       mySE->GetSelf(),
                       mySE->GetKinetic(),
                       mySE->GetThermal(),
                       mySE->GetEM(),
                       mySE->GetExplosive(),
                       m_formula.GetDroneToHit(mySE, targSE)
            );

            dam *= mySE->GetSelf()->GetAttribute(AttrDamageMultiplier).get_float();
            targSE->ApplyDamage(dam);
        } break;
        //  testing...should these 2 even be here?  isnt this a constant till reassigned?
        case EVEDB::invGroups::Electronic_Warfare_Drone: {
            _log(DRONE__WARNING, "DroneAI::RepairTarget() - %s(%u) of %s is %s(%s) and calling RepairTarget()", \
                    mySE->GetName(), mySE->GetID(), mySE->GetSelf()->type().groupName().c_str(), \
                    GetStateName(m_state), GetActionName(m_action));

        } break;
        case EVEDB::invGroups::Stasis_Webifying_Drone: {
            _log(DRONE__WARNING, "DroneAI::RepairTarget() - %s(%u) of %s is %s(%s) and calling RepairTarget()", \
                    mySE->GetName(), mySE->GetID(), mySE->GetSelf()->type().groupName().c_str(), \
                    GetStateName(m_state), GetActionName(m_action));

        } break;

        case EVEDB::invGroups::Cap_Drain_Drone: {
            // drain cap for this round
            float capCharge = targSE->GetSelf()->GetAttribute(AttrCapacitorCharge).get_float();
            capCharge -= mySE->GetSelf()->GetAttribute(AttrEnergyDestabilizationAmount).get_float();
            targSE->GetSelf()->SetAttribute(AttrCapacitorCharge, capCharge);
        } break;
        default: {
            _log(DRONE__WARNING, "DroneAI::RepairTarget() - %s(%u) of %s is %s(%s) and calling RepairTarget()", \
                    mySE->GetName(), mySE->GetID(), mySE->GetSelf()->type().groupName().c_str(), \
                    GetStateName(m_state), GetActionName(m_action));
        } break;
    }
}

void DroneAIMgr::MineTarget() {
    // note:  there are no ice harvesting drones
    // when mining drone's target is depleted, drone will get half cycle and not count in ore removed for module count
    float cycleVol(mySE->GetSelf()->GetAttribute(AttrMiningAmount).get_float());

    InventoryItemRef roidRef(targSE->GetSelf());
    float oreAmount(cycleVol / (roidRef->GetAttribute(AttrVolume).get_float()));
    if (oreAmount <= 0) {
        // drone cannot mine this heavy ore
        shipSE->GetPilot()->SendNotifyMsg("Mining operations for %s have been deactivated.<br>This drone cannot mine the %s ore.", \
        mySE->GetName(), m_ore->name());
        SetIdle();
        return;
    }

    uint32 ownerID(shipSE->GetOwnerID());
    // if ship is owned by corp, set owner of ore to pilot
    if (IsCorpID(ownerID))
        ownerID = shipSE->GetPilot()->GetCharID();
    ItemData idata(roidRef->typeID(), ownerID, locTemp, flagNone, oreAmount);
    m_ore = sItemFactory.SpawnItem(idata);
    if (m_ore.get() == nullptr) {
        _log(DRONE__WARNING, "Could not create mined ore for %s assigned to %s", \
                mySE->GetName(), shipSE->GetPilot()->GetName());
        shipSE->GetPilot()->SendNotifyMsg("Mining operations for %s have been deactivated.<br>There was an error gathering %s ore.", \
                    mySE->GetName(), m_ore->name());
        SetIdle();
        return;
    }

    // add data to StatisticMgr
    sStatMgr.Add(Stat::oreMined, cycleVol);
}

void DroneAIMgr::OrbitTarget() {
    if (m_maxSpeed == 0)
        return;

    /* we are gonna fake this one....
     * tell client that drone is orbiting
     * client will show travel and we'll track travel;  drones dont use Follow, Goto, or Approach packets
     * when our tracked distance is within orbit distance, stop drone at some point along orbit radius
     *  then do whatever actions are required until target changes
     * rinse and repeat
     */

    // are we outside of control distance and idling back to our ship?
    bool idle(false);

    // get current target so we can calculate distance to set targetID and speed properly
    int32 targetID(0);
    int64 distance(0);
    // target depends on which way we going
    switch (m_action) {
        case DroneAI::Action::Idle:           // no target
        case DroneAI::Action::OrbitShip:      // we are orbiting our ship.
        case DroneAI::Action::OrbitTarget: {  // we are orbiting our target.
            switch (m_state) {
                // these are probably assigned ship
                case DroneAI::State::Idle:
                case DroneAI::State::Fleeing:
                case DroneAI::State::Guarding:
                case DroneAI::State::Assisting:
                case DroneAI::State::ReturnBay:
                case DroneAI::State::ReturnHome: {
                    distance = mySE->GetPosition().distance(shipSE->GetPosition());
                    distance -= shipSE->GetRadius();
                    targetID = shipSE->GetID();
                    // update heading
                    GVector targHeading(mySE->GetPosition(), shipSE->GetPosition());
                    targHeading.normalize();
                    m_heading = std::move(targHeading);
                    // are we outside of control distance and idling back to our ship?
                    if (!mySE->InControlDistance())
                        idle = true;
                } break;
                // these are probably target
                case DroneAI::State::Combat:
                case DroneAI::State::Mining:
                case DroneAI::State::Pursuit:
                case DroneAI::State::Operating:
                case DroneAI::State::Repairing:
                case DroneAI::State::Approaching: {     // im gonna go with "Approaching Target" here....
                    distance = mySE->GetPosition().distance(targSE->GetPosition());
                    distance -= targSE->GetRadius();
                    targetID = targSE->GetID();
                    // update heading
                    GVector targHeading(mySE->GetPosition(), targSE->GetPosition());
                    targHeading.normalize();
                    m_heading = std::move(targHeading);
                } break;
                // this should never hit
                case DroneAI::State::Invalid:
                case DroneAI::State::Incapacitated: {
                    sLog.Warning("Drone::OrbitTarget()", "%s(%s) hit...wtf?", GetStateName(m_state), GetActionName(m_action));
                } break;
            }
        } break;
        case DroneAI::Action::Engaged:
        case DroneAI::Action::AccelToTarget:
        case DroneAI::Action::DecelToTarget: {
            // target is target
            distance = mySE->GetPosition().distance(targSE->GetPosition());
            distance -= targSE->GetRadius();
            targetID = targSE->GetID();
            // update heading
            GVector targHeading(mySE->GetPosition(), targSE->GetPosition());
            targHeading.normalize();
            m_heading = std::move(targHeading);
        } break;
        case DroneAI::Action::AccelToShip:
        case DroneAI::Action::DecelToShip: {
            // target is assigned ship
            distance = mySE->GetPosition().distance(shipSE->GetPosition());
            distance -= shipSE->GetRadius();
            targetID = shipSE->GetID();
            // update heading
            GVector targHeading(mySE->GetPosition(), shipSE->GetPosition());
            targHeading.normalize();
            m_heading = std::move(targHeading);
        } break;
        case DroneAI::Action::DecelToStop: {
            // at this point, we're idle and decel, so no target...should we have one?
            // this is for target gone or drone Incapacitated
            m_heading = NULL_ORIGIN_V;
            _log(DRONE__AI_TRACE, "%s - OrbitTarget() called.  decel to stop", mySE->GetName());
        } break;
        case DroneAI::Action::Invalid: {
            // this shouldnt hit.
            m_heading = NULL_ORIGIN_V;
            _log(DRONE__AI_TRACE, "%s - OrbitTarget() called.  idle or invalid.", mySE->GetName());
        }
    }

    // so far, so good...so what?
    m_prevSpeedFraction = m_activeSpeedFraction;

    // for travel, sf=1.0 within orbit distance otherwise sf = orbit speed / max speed
    if (!idle and  (distance > m_chaseDistance)) {
        //travel required; set full speed
        m_userSpeedFraction = 1.0f;
        m_accelTime = (-log(ASF_CHECK) * m_agility);
    } else {
        // returning from outside of control distance or close enough for impulse drives; set orbit speed
        m_userSpeedFraction = (float)m_cruiseSpeed / m_maxSpeed;
        m_accelTime = (-log(ASF_CHECK) * m_agility);
        m_accelTime *= m_accelTime;
    }

    if (m_accelTime < 1.0f)
        m_accelTime = 1.0f;

    sLog.Cyan("OrbitTarget()", "%s(%u) - %s(%s):  set usf to %.2f.  distance to target is %lli.   %ssending packet.", \
            mySE->GetName(), mySE->GetID(), GetStateName(m_state), GetActionName(m_action), \
            m_userSpeedFraction, distance, (m_sendCmd?"":"not "));

    std::vector<PyTuple*> updates;
    // sf is sent based on distance...when distance changes from chase to orbit, send update
    CmdSetSpeedFraction ssf;
        ssf.entityID = mySE->GetID();
        ssf.fraction = m_userSpeedFraction;
    updates.push_back(ssf.Encode());

    //  orbit is only sent once per target
    if (m_sendCmd) {
        sLog.Yellow("OrbitTarget()", "sending CmdOrbit packet");
        m_sendCmd = false;
        CmdOrbit du;
            du.entityID = mySE->GetID();
            du.orbitEntityID = targetID;
            du.distance = m_orbitDistance;
        updates.push_back(du.Encode());
    }

   mySE->SysBubble()->BubblecastDestinyUpdate(updates, "destiny drone");

   m_moveTime = GetTimeMSeconds();
   sLog.Error("movetime", "set - OrbitTarget()  %s(%s)", GetStateName(m_state), GetActionName(m_action));
}

void DroneAIMgr::FindTarget() {
    //start with mode...
    bool focus(mySE->GetSelf()->GetAttribute(AttrDroneFocusFire).get_bool());
    bool aggressive(mySE->GetSelf()->GetAttribute(AttrDroneIsAgressive).get_bool());
    bool follow(mySE->GetSelf()->GetAttribute(AttrFightersAttackAndFollow).get_bool());

    if (shipSE == nullptr) {
        //  well, we dont have an assigned ship...now what?
        SetIdle();
        return;
    }

    if (targSE != nullptr) {
        // hmmm...assuming we are here, current target is either invalid or a larger threat has appeared.
        ClearTarget();
    }

    if (focus) {
        // ok, so we have focus fire set.  see if any other drone has a valid target and assist

        return;
    }


    /*  this may take a bit more work than i expected....
     * first...we want to find all targets in local space
     * second...determine which targets are within my range
     * third, somehow, decide which is the largest threat or easiest target
     * then, begin attack.
     *
     * nope....we're gonna get all targeters against our assigned ship
     * then filter thru those to find easiest  (maybe config switch for this)
     * then attack.
     */

    // oh goody..we have a valid ship
    // is drone aggressive?
    if (aggressive) {
        //  yep, find smallest entity targeting our ship and begin attack

        //std::map<SystemEntity*, TargetEntry*> targets;
        std::map<SystemEntity*, TargetedByEntry*> targetby;
        //shipSE->TargetMgr()->GetAllTargets(targets);
        shipSE->TargetMgr()->GetAllTargeters(targetby);

        // put all possible targets in vector
    // std::vector<SystemEntity*> possibles;
        // sort by size
        std::map<int32, SystemEntity*>  sizeMap;

        // do we want to compare targets to targetby to find matches?
        std::map<SystemEntity *, TargetedByEntry*>::iterator itr = targetby.begin();
        for (; itr != targetby.end(); ++itr)  // if (itr->second->state == TargMgr::State::Locked)
            sizeMap[itr->first->GetRadius()] = itr->first;

        // do we even have any targets?
        if (sizeMap.empty()) {
            // not there...what now?
            sLog.Warning("DroneAI::FindTarget()", "sizeMap empty.");
            SetIdle();
            return;
        }

        // aggressive mode.  pick smallest and engage
        targSE = sizeMap.begin()->second;

        SetState(DroneAI::State::Combat);
        if (InEngageDistance(targSE)) {
            m_startTime = GetFileTimeNow();
            SetAction(DroneAI::Action::Engaged);
            SendTrueState(m_state); // sending actual state (either Combat or Operating)
            m_processTimer.Start(m_cycleTime);
            // this is a repeatable action
            m_repeat = true;
            SendGFX();
            AttackTarget();
        } else {
            SetAction(DroneAI::Action::AccelToTarget);
            SendTrueState(DroneAI::State::Approaching);
            OrbitTarget();
        }
    }
}

void DroneAIMgr::AssignShip(ShipSE* pSE) {
    shipSE = pSE;
    if (shipSE == nullptr)
        return;

    if (shipSE->GetSelf()->HasAttribute(AttrOreHoldCapacity)) {
        m_holdFlag = flagOreHold;
    } else {
        m_holdFlag = flagCargoHold;
    }
}

void DroneAIMgr::Abandon() {
    // drone abandoned.  clear everything and idle
    m_processTimer.Disable();
    m_webifierTimer.Disable();
    m_beginFindTarget.Disable();
    m_warpScramblerTimer.Disable();

    SetState(DroneAI::State::Invalid);
    SetAction(DroneAI::Action::Invalid);

    if (shipSE != nullptr)
        shipSE->ReleaseBandwidth(mySE);

    shipSE = nullptr;
    targSE = nullptr;
}

void DroneAIMgr::ModeChange() {
    if (!mySE->IsEnabled())
        return;
    /*
    mySE->GetSelf()->GetAttribute(AttrDroneFocusFire).get_bool();
    mySE->GetSelf()->GetAttribute(AttrDroneIsAgressive).get_bool();
    mySE->GetSelf()->GetAttribute(AttrFightersAttackAndFollow).get_bool();
    */
}

void DroneAIMgr::MissileLaunched(Missile* pMissile) {
    // TODO:  check mode, state and actions then react
    std::string text = "missile inbound";
    //01101101 01101001 01110011 01110011 01101001 01101100 01100101 00100000 01101001 01101110 01100010 01101111 01110101 01101110 01100100
    // convert string to binary
    shipSE->GetPilot()->SendNotifyMsg(BinString(text).c_str());
}

void DroneAIMgr::ReportDamage(uint8 type/*0*/) {
    // TODO:  check mode, state and actions then react
    std::string text = "damaged";
    //01100100 01100001 01101101 01100001 01100111 01100101 01100100
    // convert string to binary
    shipSE->GetPilot()->SendNotifyMsg(BinString(text).c_str());


    // make player switch for action on shield/armor loss?

    // types:  0=invalid, 1=shield loss, 2=armor loss
    switch (m_state) {
        case DroneAI::State::Idle: {
        } break;
        case DroneAI::State::Operating: {
        } break;
        case DroneAI::State::Repairing: {
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
        case DroneAI::State::ReturnBay: {
        } break;
        case DroneAI::State::ReturnHome: {
        } break;
        case DroneAI::State::Pursuit: {
        } break;
    }

}

void DroneAIMgr::Targeted(SystemEntity* pAgressor) {
    _log(DRONE__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while %s & %s.", \
                mySE->GetName(), mySE->GetID(), pAgressor->GetName(), \
                pAgressor->GetID(), GetStateName(m_state)), GetActionName(m_action);

    // TODO:  send warning to controlling ship
    std::string text = "target lock on me";
    //01110100 01100001 01110010 01100111 01100101 01110100 00100000 01101100 01101111 01100011 01101011 00100000 01101111 01101110 00100000 01101101 01100101
    // convert string to binary
    shipSE->GetPilot()->SendNotifyMsg(BinString(text).c_str());
}

bool DroneAIMgr::ValidTarget() {
    if (targSE == nullptr)
        return false;
    if (targSE->SysBubble() == nullptr)
        return false;

    DestinyManager* pDestiny = targSE->DestinyMgr();
    if (pDestiny == nullptr) {
        _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) has no destiny manager.",
                mySE->GetName(), mySE->GetID(), targSE->GetName(), targSE->GetID());
        return false;
    }

    // Check to see if the target is not cloaked:
    if (pDestiny->IsCloaked()) {
        _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) is cloaked.",
                mySE->GetName(), mySE->GetID(), targSE->GetName(), targSE->GetID());
        return false;
    }
    return true;
}

void DroneAIMgr::TargetLost(SystemEntity* pTarget) {
    _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) lost while %s(%s)",
                 mySE->GetName(), mySE->GetID(), pTarget->GetName(), pTarget->GetID(), \
                GetStateName(m_state), GetActionName(m_action));

    if (pTarget == targSE) {
        // well, hell. this is our current target.  let's see if anything else is out there.
        FindTarget();
        return;
    }

    // so, it wasnt our primary target...what now?
    switch (m_state) {
        case DroneAI::State::Idle:
        case DroneAI::State::Mining:
        case DroneAI::State::Fleeing:
        case DroneAI::State::Incapacitated: {
            SetIdle();
        } break;
        case DroneAI::State::ReturnBay:
        case DroneAI::State::ReturnHome:{
            SetIdle();
        } break;
        case DroneAI::State::Operating:
        case DroneAI::State::Repairing:
        case DroneAI::State::Guarding:
        case DroneAI::State::Assisting:
        case DroneAI::State::Combat:
        case DroneAI::State::Approaching:
        case DroneAI::State::Pursuit: {
            if (mySE->TargetMgr()->HasNoTargets()) {
                _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) lost. No targets remain.  Return to Idle.",
                     mySE->GetName(), mySE->GetID(), pTarget->GetName(), pTarget->GetID());
                SetIdle();
            } else {
                _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) lost, but more targets remain.",
                     mySE->GetName(), mySE->GetID(), pTarget->GetName(), pTarget->GetID());

                // targets remain.
                FindTarget();
            }
        } break;
    }
}

void DroneAIMgr::ClearTargets() {
    m_repeat = false;
    mySE->TargetMgr()->ClearTargets();
    targSE = nullptr;
}

void DroneAIMgr::ClearAllTargets() {
    m_repeat = false;
    mySE->TargetMgr()->ClearAllTargets();
    //mySE->TargetMgr()->OnTarget(nullptr, TargMgr::Mode::Clear, TargMgr::Msg::ClientReq);
    targSE = nullptr;
}

void DroneAIMgr::SendTrueState(int8 state) {
    _log(DRONE__AI_TRACE, "%s(%u) - sending true state %s.", mySE->GetName(), mySE->GetID(), GetStateName(state));
    OnDroneStateChange du;
        du.droneID = mySE->GetID();
        du.ownerID = mySE->GetOwnerID();
        du.controllerID = mySE->GetControllerID();
        du.activityState = state;
        du.droneTypeID = mySE->GetTypeID();
        du.controllerOwnerID = mySE->GetControllerOwnerID();
        du.targetID = (GetTargetID() == 0 ? PyStatic.NewNone() : new PyInt(GetTargetID()));
    PyTuple* up(du.Encode());
    if (mySE->SysBubble() != nullptr)
        mySE->SysBubble()->BubblecastDestinyUpdate(&up, "destiny");
}

uint32 DroneAIMgr::GetFollowDistance() {
    switch (m_action) {
        case DroneAI::Action::Idle:
        case DroneAI::Action::Invalid:
        case DroneAI::Action::DecelToStop: {
            return 0;
        } break;
        case DroneAI::Action::Engaged:
        case DroneAI::Action::OrbitShip:
        case DroneAI::Action::OrbitTarget: {
            return m_orbitDistance;   //near - range 1
        } break;
        case DroneAI::Action::AccelToShip:
        case DroneAI::Action::DecelToShip:
        case DroneAI::Action::AccelToTarget:
        case DroneAI::Action::DecelToTarget: {
            return m_falloffDistance;   //close - range 2
        } break;
    }
    return 0;
}

uint32 DroneAIMgr::GetTargetID() {
    switch (m_action) {
        case DroneAI::Action::Idle:
        case DroneAI::Action::Invalid:
        case DroneAI::Action::DecelToStop: {
            return 0;
        } break;
        case DroneAI::Action::Engaged:
        case DroneAI::Action::OrbitTarget:
        case DroneAI::Action::AccelToTarget:
        case DroneAI::Action::DecelToTarget: {
            return targSE->GetID();
        } break;
        case DroneAI::Action::OrbitShip:
        case DroneAI::Action::AccelToShip:
        case DroneAI::Action::DecelToShip: {
            return shipSE->GetID();
        } break;
    }
    return 0;
}

SystemEntity* DroneAIMgr::GetTargetSE() {
    switch (m_action) {
        case DroneAI::Action::Idle:
        case DroneAI::Action::Invalid:
        case DroneAI::Action::DecelToStop: {
            return nullptr;
        } break;
        case DroneAI::Action::Engaged:
        case DroneAI::Action::OrbitTarget:
        case DroneAI::Action::AccelToTarget:
        case DroneAI::Action::DecelToTarget: {
            return targSE;
        } break;
        case DroneAI::Action::OrbitShip:
        case DroneAI::Action::AccelToShip:
        case DroneAI::Action::DecelToShip: {
            return shipSE;
        } break;
    }
    return nullptr;
}

float DroneAIMgr::GetSpeedFraction() {
    // this is for encodedestiny
    switch (m_action) {
        // if the drone is orbiting, send fractional speed
        case DroneAI::Action::Idle:
        case DroneAI::Action::Engaged:
        case DroneAI::Action::OrbitShip:
        case DroneAI::Action::OrbitTarget: {
            // should this also be asf?
            return (float)m_cruiseSpeed / m_maxSpeed;
        }
        // these should send actual asf
        case DroneAI::Action::DecelToShip:
        case DroneAI::Action::DecelToStop:
        case DroneAI::Action::DecelToTarget: {
            return m_activeSpeedFraction;
        }
        // if the drone is invalid, send 0
        case DroneAI::Action::Invalid: {
            return 0.0f;
        }
    }
    return 1.0f;
}

bool DroneAIMgr::InActionDistance(SystemEntity* pTarget) {
    double dist(mySE->GetPosition().distance(pTarget->GetPosition()) - pTarget->GetRadius());
    return (dist < sConfig.drone.InteractDistace);
}

bool DroneAIMgr::InOrbitDistance(SystemEntity* pTarget) {
    double dist(mySE->GetPosition().distance(pTarget->GetPosition()) - pTarget->GetRadius());
    return (dist < m_orbitDistance);
}
bool DroneAIMgr::InFalloffDistance(SystemEntity* pTarget) {
    double dist(mySE->GetPosition().distance(pTarget->GetPosition()) - pTarget->GetRadius());
    return (dist < m_falloffDistance);
}

bool DroneAIMgr::InEngageDistance(SystemEntity* pTarget) {
    double dist(mySE->GetPosition().distance(pTarget->GetPosition()) - pTarget->GetRadius());
    return (dist < m_attackDistance);
}

bool DroneAIMgr::InChaseDistance(SystemEntity* pTarget) {
    double dist(mySE->GetPosition().distance(pTarget->GetPosition()) - pTarget->GetRadius());
    return (dist < m_chaseDistance);
}

bool DroneAIMgr::InMaxDistance(SystemEntity* pTarget) {
    double dist(mySE->GetPosition().distance(pTarget->GetPosition()) - pTarget->GetRadius());
    return (dist < m_maxDistance);
}


// destiny methods below...

void DroneAIMgr::Stop() {
    // only called when drone offline or disabled
    if (mySE->IsEnabled())
        mySE->DisableDrone();

    m_velocity = NULL_ORIGIN_V;
    UpdatePosition();
    m_moveTime = 0;
    sLog.Error("movetime", "0 - Stop() %s(%s)", GetStateName(m_state), GetActionName(m_action));
    m_startTime = 0;
    SetState(DroneAI::State::Invalid);

    if (mySE->SysBubble() != nullptr) {
        CmdStop stop;
            stop.entityID = mySE->GetID();
        PyTuple* up = stop.Encode();
        // edit  mySE->SysBubble()->BubblecastDestinyUpdate(&up, "destiny drone");
    }
}

void DroneAIMgr::Pause() {
    // this will set position and velocity, then stop further processing until next command
    //  velocity is set for faking proper weapon tracking and tohit calc's

    if (!mySE->IsEnabled())
        return;

    m_velocity = m_heading * m_maxSpeed * m_activeSpeedFraction;

    m_moveTime = 0;
    sLog.Error("movetime", "0 - Pause() %s(%s)", GetStateName(m_state), GetActionName(m_action));
}

void DroneAIMgr::Move(double timeStamp) {
    if (m_maxSpeed == 0)
        return;

    // this will keep our position ref accurate, so we do need somewhat accurate processing
    //  note that we are hacking this, and not actually orbiting anything

    // is this right?  if set to idle, dont drones have to slow to stop?  maybe a couple, but most are < 1.5s accel
    if ((m_state < DroneAI::State::Combat) and (m_action == DroneAI::Action::DecelToStop)) {
        _log(DRONE__AI_TRACE, "%s - Move() called.  %s and decel to stop", mySE->GetName(), GetStateName(m_state));
        m_moveTime = 0;
        sLog.Error("movetime", "0 - Move::stop()  - %s(%s)", GetStateName(m_state), GetActionName(m_action));
        return;
    }

    uint32 targDistance(0);
    uint32 shipDistance(mySE->GetPosition().distance(shipSE->GetPosition()));
    shipDistance -= shipSE->GetRadius();
    if (targSE != nullptr)
        targDistance = mySE->GetPosition().distance(targSE->GetPosition() - targSE->GetRadius());

    _log(DRONE__AI_TRACE, "Move() - %s(%u):  %s(%s) - targDistance: %u, shipDistance: %u", \
                mySE->GetName(), mySE->GetID(), GetStateName(m_state), GetActionName(m_action), targDistance, shipDistance);

    // if we're still traveling, we will need to do accel/decel and keep track of timestamps like destiny does
    // some drones have accel/decel times > 5s
    bool accel(false), decel(false), stop(false);

    // lets do some fkn distance checks to stop movement, please.
    // determine current action
    switch (m_action) {
        case DroneAI::Action::Idle:
        case DroneAI::Action::Engaged:
        case DroneAI::Action::Invalid:
        case DroneAI::Action::OrbitShip:
        case DroneAI::Action::OrbitTarget: {
            // these require no movement...(move != false) in Process()
            sLog.Warning("Drone::Move()", "forgot to set move=false.");
            return;
        } break;

        case DroneAI::Action::DecelToShip: {
            if (shipDistance > m_chaseDistance) {
                _log(DRONE__WARNING, "shipDistance > m_chaseDistance but action is decel");
                // still far enough to run full speed....why are we set to decel?
                SetState(DroneAI::Action::AccelToShip);
                OrbitTarget();
                accel = true;
            } else if (shipDistance < m_orbitDistance) {
                // we're within orbit.  set position and stop movement
                GVector head(shipSE->GetPosition(), mySE->GetPosition());
                head.normalize();
                GVector velocity(head * m_orbitDistance);
                GPoint pos(shipSE->GetPosition());
                pos += velocity;
                mySE->SetPosition(pos);
                m_moveTime = 0;
                sLog.Error("movetime", "0 - Move() - %s(%s)", GetStateName(m_state), GetActionName(m_action));
                _log(DRONE__AI_TRACE, "Move() - within orbit distance. pausing");
                return;
            } else if (shipDistance < m_falloffDistance) {
                // we're close enough to slow down.  reset usf but only once except for mining drones
                if (mySE->GetGroupID() != EVEDB::invGroups::Mining_Drone) {
                    if (m_userSpeedFraction > 0.99f) {
                        // we are still set to full speed.
                        OrbitTarget();
                    }
                    decel = true;
                } else if (timeStamp < m_accelTime) {
                    accel = true;
                }
            }
        } break;

        case DroneAI::Action::AccelToShip: {
            if (shipDistance > m_chaseDistance) {
                // still far enough to run full speed
                accel = true;
            } else if (shipDistance < m_orbitDistance) {
                // we're within orbit.  set position and stop movement
                GVector head(shipSE->GetPosition(), mySE->GetPosition());
                head.normalize();
                GVector velocity(head * m_orbitDistance);
                GPoint pos(shipSE->GetPosition());
                pos += velocity;
                mySE->SetPosition(pos);
                m_moveTime = 0;
                sLog.Error("movetime", "0 - Move() - %s(%s)", GetStateName(m_state), GetActionName(m_action));
                _log(DRONE__AI_TRACE, "Move() - within orbit distance. pausing");
                return;
            } else if (shipDistance < m_falloffDistance) {
                // we're close enough to slow down.  reset usf but only once except for mining drones
                if (mySE->GetGroupID() != EVEDB::invGroups::Mining_Drone) {
                    if (m_userSpeedFraction > 0.99f) {
                        // we are still set to full speed.
                        SetState(DroneAI::Action::DecelToShip);
                        OrbitTarget();
                    }
                    accel = false;
                    decel = true;
                } else if (timeStamp < m_accelTime) {
                    accel = true;
                }
            }
        } break;
        case DroneAI::Action::DecelToTarget:  {
            if (targDistance > m_chaseDistance) {
                _log(DRONE__WARNING, "targDistance > m_chaseDistance but action is decel");
                // still far enough to run full speed....why are we set to decel?
                SetState(DroneAI::Action::AccelToShip);
                OrbitTarget();
                accel = true;
            } else if (targDistance < m_orbitDistance) {
                // we're within orbit.  set position and stop movement
                GVector head(targSE->GetPosition(), mySE->GetPosition());
                head.normalize();
                GVector velocity(head * m_orbitDistance);
                GPoint pos(targSE->GetPosition());
                pos += velocity;
                mySE->SetPosition(pos);
                m_moveTime = 0;
                sLog.Error("movetime", "0 - Move() - %s(%s)", GetStateName(m_state), GetActionName(m_action));
                _log(DRONE__AI_TRACE, "Move() - within orbit distance. pausing");
                return;
            } else if (targDistance < m_falloffDistance) {
                // we're close enough to slow down.  reset usf but only once except for mining drones
                if (mySE->GetGroupID() != EVEDB::invGroups::Mining_Drone) {
                    if (m_userSpeedFraction > 0.99f) {
                        // we are still set to full speed.
                        OrbitTarget();
                    }
                    decel = true;
                } else if (timeStamp < m_accelTime) {
                    accel = true;
                }
            }
        } break;
        case DroneAI::Action::AccelToTarget:{
            if (targDistance > m_chaseDistance) {
                // still far enough to run full speed
                accel = true;
            } else if (targDistance < m_orbitDistance) {
                // we're within orbit.  set position and stop movement
                GVector head(targSE->GetPosition(), mySE->GetPosition());
                head.normalize();
                GVector velocity(head * m_orbitDistance);
                GPoint pos(targSE->GetPosition());
                pos += velocity;
                mySE->SetPosition(pos);
                m_moveTime = 0;
                sLog.Error("movetime", "0 - Move() - %s(%s)", GetStateName(m_state), GetActionName(m_action));
                _log(DRONE__AI_TRACE, "Move() - within orbit distance. pausing");
                return;
            }else if (targDistance < m_falloffDistance) {
                // we're close enough to slow down.  reset usf but only once except for mining drones
                if (mySE->GetGroupID() != EVEDB::invGroups::Mining_Drone) {
                    if (m_userSpeedFraction > 0.99f) {
                        // we are still set to full speed.
                        SetState(DroneAI::Action::DecelToTarget);
                        OrbitTarget();
                    }
                    accel = false;
                    decel = true;
                } else if (timeStamp < m_accelTime) {
                    decel = true;
                }
            }
        } break;
        case DroneAI::Action::DecelToStop: {
            // im sure i'll need this, but havent determined particulars yet
            stop = true;
            decel = true;
        } break;
    }

    _log(DRONE__AI_TRACE, "Move() - accel: %s, decel: %s, stop: %s", accel?"true":"false", decel?"true":"false", stop?"true":"false");

    int32 speed(0);
    // check to see if our target has moved.  if so, update position accordingly
    if ((timeStamp > m_accelTime) or (m_timeFraction > 0.9f)) {
        // full speed reached
        accel = false;
        decel = false;
        speed = m_maxSpeed * m_userSpeedFraction;
    } else {
        // speed still changing
        if (accel) {
            // object still accelerating.
            if (m_prevSpeedFraction) {
                /* accel from previous non-full speed
                 *   take diff of psf and usf then multiply by tf
                 *   add result to psf to get asf
                 *  asf is the fraction of max speed the ship is moving at this tic.
                 */
                m_activeSpeedFraction = m_prevSpeedFraction + ((m_userSpeedFraction - m_prevSpeedFraction) * m_timeFraction);
            } else {
                // this is simple acceleration.  asf = usf * tf
                m_activeSpeedFraction = m_userSpeedFraction * m_timeFraction;
            }
        } else if (decel) {
            // object still decelerating.
            if (m_prevSpeedFraction) {
                // asf = psf - (psf - usf) * tf
                m_activeSpeedFraction = m_prevSpeedFraction - ((m_prevSpeedFraction - m_userSpeedFraction) * m_timeFraction);
            } else {
                // this should never hit....should not have decel w/o psf
                sLog.Warning("DroneAI::Move()", "decel = true, but psf = 0.");
            }
        }
        speed = m_maxSpeed * m_activeSpeedFraction;

        _log(DRONE__MOVE, "%s - Move() !@ full  acceltime: %.2f  timeStamp: %.2f, tf:%.2f", \
                mySE->GetName(), m_accelTime, timeStamp, m_timeFraction);
    }

    _log(DRONE__MOVE, "Move() - %s(%u) is %s at %u m/s (tf:%.4f asf:%.4f).", \
            mySE->GetName(), mySE->GetID(), (accel ? "Accel" : (decel ? "Decel" : "Steady")), \
            speed, m_timeFraction, m_activeSpeedFraction);

    m_velocity = m_heading * speed;
    UpdatePosition(false);
}

void DroneAIMgr::MoveDrone(SystemEntity* pTarget) {
    GVector head(pTarget->GetPosition(), mySE->GetPosition());
    head.normalize();
    GVector posOffset(head * m_orbitDistance);
    GPoint pos(pTarget->GetPosition());
    pos += posOffset;
    mySE->SetPosition(pos);
}

void DroneAIMgr::UpdatePosition(bool update/*false*/) {
    // basic position updating - variables updated elsewhere
    GVector pos(mySE->GetPosition());
    pos += m_velocity;
    mySE->SetPosition(pos);

    if (sEntityMgr.GetTracking())
        MarkPoint(pos);

    // should we send position updates?  probably not, as long as we're kinda close to what client has
    if (update) {
        SetBallPosition du;
            du.entityID = mySE->GetID();
            du.x = pos.x;
            du.y = pos.y;
            du.z = pos.z;
        PyTuple* up = du.Encode();
        mySE->SysBubble()->BubblecastDestinyUpdate(&up, "DestinyUpdates");
        PyDecRef(up);
    }
}

void DroneAIMgr::SendSpeedFraction() {
    CmdSetSpeedFraction ssf;
        ssf.entityID = mySE->GetID();
        ssf.fraction = m_userSpeedFraction;
    PyTuple* up = ssf.Encode();
    if (mySE->SysBubble() != nullptr)
        ;// edit  mySE->SysBubble()->BubblecastDestinyUpdate(&up, "destiny drone");
}

/*
 * // do drones warp??   they can, yes...with limitations
 * if (mySE->IsDroneSE()) {
 *    // put drone limit checks here
 *    sLog.Warning("DroneWarp", "Drone %s (from ship %s) warping from bID %u to bID %u", \
 *    mySE->GetName(), mySE->GetDroneSE()->GetShipSE()->GetName(), \
 *    mySE->SysBubble()->GetID(), m_targBubble->GetID());
 * } */

void DroneAIMgr::SendGFX(Client* pClient/*nullptr*/) {
    if (m_effectID < 1) {
        // not necessarily an error.  just make note
        sLog.Error("DroneAI::SendGFX()", "m_effectID < 1 for %s.", mySE->GetName());
        return;
    }

    bool active(false), start(false);
    if (m_action == DroneAI::Action::Engaged) {
        start = true;
        active = true;
    }

    InventoryItemRef iRef = mySE->GetSelf();
    // effects are listed in EVE_Effects.h
    //  NOTE: drones are called 'entities' in client; EVE_Effects has 'entityxxx' for gfx...may not be used like this.
    uint16 gfxID(0);
    if (m_booster and iRef->HasAttribute(AttrGfxBoosterID)) {   // graphicID for turret for drone type ships
        gfxID = iRef->GetAttribute(AttrGfxBoosterID).get_uint32();
    } else if (iRef->HasAttribute(AttrGfxTurretID)) {           // graphicID for turret for drone type ships
        gfxID = iRef->GetAttribute(AttrGfxTurretID).get_uint32();
    }

    bool repeat(true);
    if (iRef->groupID() == EVEDB::invGroups::Mining_Drone)
        repeat = false;

    /*  not sure if this is right for drones...im thinking not
    std::string guidStr = sFxDataMgr.GetEffectGuid(gfxID);
    if (guidStr.empty())
        guidStr = sFxDataMgr.GetEffectGuid(iRef->type().GetDefaultEffect());
    */
    std::string guidStr = sFxDataMgr.GetEffectGuid(m_effectID);

    sLog.Blue("Drone", "sending %sGFX for %s;  repeat:%s, gfx:%s", \
        active?"start ":"stop ", iRef->name(), repeat?"true":"false", guidStr.c_str());

    OnSpecialFX14 effect;
        effect.entityID = iRef->itemID();
        effect.moduleID = iRef->itemID();             // npc UID for npc's/drones
        effect.moduleTypeID = iRef->typeID();     // npc typeID for npc's/drones
        effect.targetID = (targSE == nullptr ? PyStatic.NewNone() : new PyInt(targSE->GetID()));
        effect.otherTypeID = PyStatic.NewNone();
        effect.area = PyStatic.mtList();        // no data.  not used in client
        effect.guid = std::move(guidStr);
        effect.isOffensive = sFxDataMgr.isOffensive(m_effectID);       // bool
        effect.start = start;                   // int bool
        effect.active = active;               // int bool
        effect.duration = m_cycleTime;             // in ms
        effect.repeat = (repeat ? 10000 : 0);                 // mining drones dont repeat, but combat must
        effect.startTime = m_startTime;
        effect.graphicInfo = (gfxID == 0 ? PyStatic.NewNone() : new PyInt(gfxID));
    PyTuple *up = effect.Encode();
    if (is_log_enabled(EFFECTS__DUMP))
        up->Dump(EFFECTS__DUMP, "");

    if (pClient == nullptr) {
        mySE->SysBubble()->BubblecastDestinyUpdate(&up, "DestinyUpdates");
    } else {
        // this is to update new ship in bubble with active gfx
        pClient->QueueDestinyUpdate(&up);
    }
    PyDecRef(up);
}

void DroneAIMgr::SetAction(int8 action/*-1*/) {
    _log(DRONE__AI_TRACE, "%s(%u) - setting action from %s to %s.", \
            mySE->GetName(), mySE->GetID(), GetActionName(m_action), GetActionName(action));
    m_action = action;
}

void DroneAIMgr::SetState(int8 state/*-1*/) {
    _log(DRONE__AI_TRACE, "%s(%u) - setting state from %s to %s.", \
            mySE->GetName(), mySE->GetID(), GetStateName(m_state), GetStateName(state));
    m_state = state;
}

void DroneAIMgr::MarkPoint(const GPoint& position) {
    std::string name = "drone marker", desc = "";
    // create jetcan to visualize point in space
    ItemData idata(23, ownerSystem, mySE->GetLocationID(), flagNone, name.c_str(), position, desc.c_str());
    CargoContainerRef cRef = CargoContainerRef::StaticCast(InventoryItem::SpawnTemp(idata));
    if (cRef.get() == nullptr) {
        _log(DESTINY__WARNING, "MarkPoint() could not create Item for drone marker");
        return;
    }

    // create new container
    FactionData data = FactionData();
    ContainerSE* cSE = new ContainerSE(cRef, mySE->GetServices(), mySE->SystemMgr(), data);
    if (cSE == nullptr) {
        _log(DESTINY__WARNING, "MarkPoint() could not create SE for drone");
        return;
    }
    cRef->SetMySE(cSE);
    cSE->AnchorContainer();
    mySE->SystemMgr()->AddMarker(cSE);
}

int8 DroneAIMgr::GetState() {
    switch (m_state) {
        case DroneAI::State::Invalid:
            return DroneAI::State::Idle;
        // these really depend on distance
        case DroneAI::State::Guarding:
        case DroneAI::State::Assisting:
            return DroneAI::State::Combat;
            // Approaching, Pursuit
        default:
            return m_state;
    }
}

const char* DroneAIMgr::GetStateName(int8 stateID) {
    switch (stateID) {
        case DroneAI::State::Idle:            return "\033[1mIdle\033[0m";
        case DroneAI::State::Combat:          return "\033[1mCombat\033[0m";
        case DroneAI::State::Mining:          return "\033[1mMining\033[0m";
        case DroneAI::State::Approaching:     return "\033[1mApproaching\033[0m";
        case DroneAI::State::ReturnHome:      return "\033[1mReturn to ship\033[0m";
        case DroneAI::State::ReturnBay:       return "\033[1mReturn to Bay\033[0m";
        case DroneAI::State::Pursuit:         return "\033[1mPursuit\033[0m";
        case DroneAI::State::Repairing:       return "\033[1mRepairing\033[0m";
        case DroneAI::State::Fleeing:         return "\033[1mFleeing\033[0m";
        case DroneAI::State::Operating:       return "\033[1mOperating\033[0m";
        case DroneAI::State::Assisting:       return "\033[1mAssisting\033[0m";
        case DroneAI::State::Guarding:        return "\033[1mGuarding\033[0m";
        case DroneAI::State::Incapacitated:   return "\033[1mIncapacitated\033[0m";
        case DroneAI::State::Unknown:         return "\033[1mUnknown\033[0m";
        default:                              return "\033[1mInvalid\033[0m";
    }
}

const char* DroneAIMgr::GetActionName(int8 stateID) {
    switch (stateID) {
        case DroneAI::Action::Idle:            return "\033[1mIdle\033[0m";
        case DroneAI::Action::Engaged:         return "\033[1mEngaged\033[0m";
        case DroneAI::Action::AccelToTarget:   return "\033[1mAccel to Target\033[0m";
        case DroneAI::Action::DecelToTarget:   return "\033[1mDecel to Target\033[0m";
        case DroneAI::Action::AccelToShip:     return "\033[1mAccel to Ship\033[0m";
        case DroneAI::Action::DecelToShip:     return "\033[1mDecel to Ship\033[0m";
        case DroneAI::Action::OrbitTarget:     return "\033[1mOrbiting Target\033[0m";
        case DroneAI::Action::OrbitShip:       return "\033[1mOrbiting Ship\033[0m";
        case DroneAI::Action::DecelToStop:     return "\033[1mDecel to Stop\033[0m";
        default:                               return "\033[1mInvalid\033[0m";
    }
}


/*
 *  these distances greatly depend on drone type.
 * mining drone orbit ~200m where others dont have orbit defined, use m_maxDistance as orbit, ~1km max
 *
 *            // action, orbit, falloff, engage, chase, max
 *            if (InActionDistance(targSE)) {               600
 *            } else if (InOrbitDistance(targSE)) {
 *            } else if (InFalloffDistance(targSE)) {
 *            } else if (InEngageDistance(targSE)) {
 *            } else if (InChaseDistance(targSE)) {
 *            } else if (InMaxDistance(targSE)) {
 *            } else {
 *                // outside max distance
 *            }
 *
 *
switch (m_state) {
    case DroneAI::State::Idle: {
    } break;
    case DroneAI::State::Operating: {
    } break;
    case DroneAI::State::Repairing: {
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
    case DroneAI::State::ReturnBay: {
    } break;
    case DroneAI::State::ReturnHome: {
    } break;
    case DroneAI::State::Pursuit: {
    } break;
}

switch (m_action) {
    case DroneAI::Action::Invalid: {
    } break;
    case DroneAI::Action::Idle: {
    } break;
    case DroneAI::Action::Engaged: {
    } break;
    case DroneAI::Action::AccelToTarget: {
    } break;
    case DroneAI::Action::DecelToTarget: {
    } break;
    case DroneAI::Action::AccelToShip: {
    } break;
    case DroneAI::Action::DecelToShip: {
    } break;
    case DroneAI::Action::OrbitTarget: {
    } break;
    case DroneAI::Action::OrbitShip: {
    } break;
    case DroneAI::Action::DecelToStop: {
        // this is almost always an error
        _log(DRONE__AI_TRACE, "%s - %s.  decel to stop", mySE->GetName(), GetStateName(m_state));
        SetState(DroneAI::State::Idle);
        // stop processing until another call hits
        move = false;
        m_moveTime = 0;
        sLog.Error("movetime", "0 - ?? - %s(%s)", GetStateName(m_state), GetActionName(m_action));
    } break;
    default: {
        // error
        _log(DRONE__AI_TRACE, "%s - hmmmmm... state is %s but action is %s.", \
        mySE->GetName(), GetStateName(m_state), GetActionName(m_action));
        SetState(DroneAI::State::Idle);
        // stop processing until another call hits
        move = false;
        m_moveTime = 0;
        sLog.Error("movetime", "0 - ?? - %s(%s)", GetStateName(m_state), GetActionName(m_action));
    } break;
}

switch (mySE->GetGroupID()) {
    case EVEDB::invGroups::Combat_Drone:
    case EVEDB::invGroups::Mining_Drone:
    case EVEDB::invGroups::Fighter_Drone:
    case EVEDB::invGroups::Fighter_Bomber:
    case EVEDB::invGroups::Logistic_Drone:
    case EVEDB::invGroups::Cap_Drain_Drone:
    case EVEDB::invGroups::Stasis_Webifying_Drone:
    case EVEDB::invGroups::Electronic_Warfare_Drone:  {
        sLog.Error("DroneAI::Engage()", "%s drone sent for %s.  wtf?", \
        mySE->GetSelf()->type().groupName().c_str(), GetStateName(state));
        PyDict* data = new PyDict();
        data->SetItemString("targetTypeName", new PyString(mySE->GetName()));
        PyTuple* error = new PyTuple(2);
        error->SetItem(0, new PyString("EntityUnknownCommand"));
        error->SetItem(1, data);
        dict->SetItem(new PyInt(mySE->GetID()), error);
        return;
    } break;
}
*/
