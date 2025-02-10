/**
 * DroneAI.cpp
 *      this class is for drone AI
 *
 * @Author:     Allan
 * @Version:    0.15
 * @Date:       27Nov19  (copied from NPCAI.cpp)
 * @Rewrite:    3Feb25  (complete refactor to process all types and actions of drones)
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

DroneAIMgr::DroneAIMgr(DroneSE* pdSE)
: m_heading(NULL_ORIGIN),
m_velocity(NULL_ORIGIN_V),
m_targetSE(nullptr),
m_droneSE(pdSE),
m_assignedShipSE(nullptr),
m_ore(nullptr),
m_holdFlag(flagCargoHold),
m_processTimer(0),
m_mainAttackTimer(0),
m_beginFindTarget(0),
m_warpScramblerTimer(0),
m_webifierTimer(0),
m_sendCmd(false),
m_booster(false),
m_repeat(false),
m_state(DroneAI::State::Invalid),
m_action(DroneAI::Action::Invalid),
m_effectID(0),
m_maxSpeed(0),
m_cruiseSpeed(0),
m_armorRepairDuration(0),
m_shieldBoosterDuration(0),
m_startTime(0),
m_maxRange(0.0f),
m_cycleTime(0.0f),
m_chaseRange(0.0f),
m_orbitRange(0.0f),
m_attackRange(0.0f),
m_falloffRange(0.0f),
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
    InventoryItemRef dRef = m_droneSE->GetSelf();
    m_maxSpeed = (dRef->GetAttribute(AttrMaxVelocity).get_uint32());
    m_cycleTime = (dRef->GetAttribute(AttrSpeed).get_float());
    m_cruiseSpeed = (dRef->GetAttribute(AttrEntityCruiseSpeed).get_uint32());

    // load distances into vector
    std::multiset<uint32> ranges;
    ranges.insert(dRef->GetAttribute(AttrMaxRange).get_uint32());
    ranges.insert(dRef->GetAttribute(AttrEntityChaseMaxDistance).get_uint32());
    ranges.insert(dRef->GetAttribute(AttrOrbitRange).get_uint32());
    ranges.insert(dRef->GetAttribute(AttrEntityAttackRange).get_uint32());
    ranges.insert(dRef->GetAttribute(AttrFalloff).get_uint32());

    // put sorted numbers into our distance-ordered variables, smallest first
    // this will include multiple zeros, if in set.  checks below *should* fix them
    std::multiset<uint32>::iterator itr = ranges.begin();
    m_orbitRange = *itr;
    m_falloffRange = *(++itr);
    m_attackRange = *(++itr);
    m_chaseRange = *(++itr);
    m_maxRange = *(++itr);

    // determine if any values are zero, largest to smallest
    //  if any values are missing, use largest
    if (m_maxRange == 0) {
        sLog.Error("Drone::Init", "%s - m_maxRange is 0", dRef->name());
        // well, these are usually stationary...
    }
    if (m_chaseRange == 0) {
        sLog.Warning("Drone::Init", "%s - m_chaseRange is 0", dRef->name());
        m_chaseRange = m_maxRange;
    }
    if (m_attackRange == 0) {
        sLog.Warning("Drone::Init", "%s - m_attackRange is 0", dRef->name());
        m_attackRange = m_maxRange;
    }
    if (m_falloffRange == 0) {
        sLog.Warning("Drone::Init", "%s - m_falloffRange is 0", dRef->name());
        m_falloffRange = m_maxRange;
    }
    if (m_orbitRange == 0) {
        sLog.Warning("Drone::Init", "%s - m_orbitRange is 0", dRef->name());
        m_orbitRange = m_maxRange / 2;
    }


    // set times and ranges unique to these types (override above if required)
    switch (dRef->groupID()) {
        case EVEDB::invGroups::Combat_Drone: {    //100
        } break;
        case EVEDB::invGroups::Mining_Drone: {    //101
            m_cycleTime = (dRef->GetAttribute(AttrDuration).get_float());
            m_cruiseSpeed = (dRef->GetAttribute(AttrMaxVelocity).get_uint32());
            m_orbitRange = dRef->GetAttribute(AttrOrbitRange).get_uint32();
        } break;
        case EVEDB::invGroups::Repair_Drone:     //299
        case EVEDB::invGroups::Logistic_Drone: {    //640
            m_booster = true;
            m_armorRepairDuration = (dRef->GetAttribute(AttrEntityArmorRepairDuration).get_uint32());
            m_shieldBoosterDuration = (dRef->GetAttribute(AttrEntityShieldBoostDuration).get_uint32());
        } break;
        case EVEDB::invGroups::Cap_Drain_Drone: {    //544
        } break;
        case EVEDB::invGroups::Warp_Scrambling_Drone: {    //545
        } break;
        case EVEDB::invGroups::Fighter_Drone: {    //549
            // these are advanced drones.  will follow target in warp, but not jump
        } break;
        case EVEDB::invGroups::Electronic_Warfare_Drone: {    //639
        } break;
        case EVEDB::invGroups::Stasis_Webifying_Drone: {    //641
        } break;
        case EVEDB::invGroups::Fighter_Bomber: {    //1023
        } break;
        /*  i dont think these are available
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

    if (mass < 100.0) {
        sLog.Warning("DroneAI::Init()", " %s  has no mass defined.  setting to 1000.0", dRef->name());
        mass = 1000.0;
        dRef->SetAttribute(AttrMass, mass, false);
    }
    if (inertiaMod < 100.0) {
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

    _log(DRONE__WARNING, "DroneAI::Init() - %s  agility: %.4f, acceltime: %.2f", dRef->name(), m_agility, m_accelTime);
}

void DroneAIMgr::Process() {
    /* Drone::State definitions   -allan 27Nov19  -major ud/rewrite  7Feb25
            Incapacitated       = -2,
            Invalid             = -1,
            // defined in client
            Idle                = 0,  // not doing anything....idle.
            Combat              = 1,  // fighting - needs target
            Mining              = 2,  // unsure - may need target
            Approaching         = 3,  // too close to chase, but to far to engage
            ReturnBay           = 4,  // return to bay  (Departing in client)
            ReturnHome          = 5,  // return to ship  (Departing2 in client)
            Pursuit             = 6,  // target out of range to attack/follow, but within npc sight range....use mwd/ab if equiped
            Fleeing             = 7,  // running away
            Operating           = 9,  // whats diff from engaged here? unanchoring?
            Engaged             = 10, // either attack or aid - needs target
            // internal only
            Guarding            = 11, // as stated
            Assisting           = 12  //  this will be remote reppers/boosters
     */

    // disabled, invalid, orbiting all have movetime=0 to avoid tics;  nothing to process till new command received
    if (m_moveTime == 0)
        return;

    bool move(true);
    double timeStamp(0);

    // keep timer in seconds.
    timeStamp = ((GetTimeMSeconds() - m_moveTime) * 0.001);
    // update tf for this tic
    m_timeFraction = (1 - exp(-timeStamp / m_agility));

    _log(DRONE__AI_TRACE, "%s(%u) Proc(%.0f) - %s(%s)  tf:%.2f.", \
            m_droneSE->GetName(), m_droneSE->GetID(), timeStamp, \
            GetStateName(m_state), GetActionName(m_action), m_timeFraction);

    switch(m_state) {
        case DroneAI::State::Idle: {
            // orbiting controlling ship  do nothing until next command
            //  we can put actions here to return to ship, home, etc
            switch (m_action) {
                case DroneAI::Action::DecelToStop: {
                    // check speed fraction to stop ship here or in Move()?  do it in move
                    _log(DRONE__AI_TRACE, "%s - Proc() - Idle and decel to stop", m_droneSE->GetName());
                    // stop processing until another call hits
                    move = false;
                    m_moveTime = 0;
                    sLog.Error("movetime", "0");
                } break;
                case DroneAI::Action::OrbitShip:
                case DroneAI::Action::OrbitTarget: {
                    // idle and orbiting.
                    _log(DRONE__AI_TRACE, "%s - Proc() - orbiting home ship", m_droneSE->GetName());
                    // stop processing until another call hits
                    move = false;
                    m_moveTime = 0;
                    sLog.Error("movetime", "0");
                } break;
                default: {
                    _log(DRONE__ERROR, "%s - state is %s but action is %s.", \
                            m_droneSE->GetName(), GetStateName(m_state), GetActionName(m_action));
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
                    if (InOrbitDistance(m_targetSE)) {
                         _log(DRONE__AI_TRACE, "%s - arrived at target. begin mining.", m_droneSE->GetName());
                        // we are close enough to begin.   stop moving and start mining
                        SendTrueState(DroneAI::State::Mining);
                        SetAction(DroneAI::Action::Engaged);
                        // begin mining timer
                        m_processTimer.Start(m_cycleTime);
                        m_startTime = GetFileTimeNow();
                        m_moveTime = GetTimeMSeconds();
                        sLog.Error("movetime", "set");
                        SendGFX();
                        move = false;
                       }
                } break;
                case DroneAI::Action::OrbitShip:
                case DroneAI::Action::AccelToShip:
                case DroneAI::Action::DecelToShip: {
                    // drone returning to ship.   interaction distance is set in config.drone.InteractDistace and defaults to 2k5m
                    if (InActionDistance(m_assignedShipSE)) {
                        // we have returned.  drop ore and return to mine, if commanded
                        if (m_assignedShipSE->GetSelf()->GetMyInventory()->HasAvailableSpace(m_holdFlag, m_ore)) {
                            // automagically stack ore in hold.  this is a feature.
                            m_ore->MergeTypesInCargo(m_assignedShipSE->GetShipItemRef().get(), m_holdFlag);
                            _log(DRONE__AI_TRACE, "%s - dropped ore to ship.", m_droneSE->GetName());
                        } else {
                            m_assignedShipSE->GetPilot()->SendNotifyMsg("Your %s deactivates mining operations as it couldn't add the %s ore to your %s.", \
                                    m_droneSE->GetName(), m_ore->name(), sDataMgr.GetFlagName(m_holdFlag));
                            _log(DRONE__AI_TRACE, "%s - error adding ore to ship.  return to idle.", m_droneSE->GetName());
                            move = false;
                            SetAction(DroneAI::Action::DecelToStop);
                            SetIdle();
                            break;
                        }
                        if (m_repeat) {
                            _log(DRONE__AI_TRACE, "%s - return to target.", m_droneSE->GetName());
                            // mine, drop, return, rinse, repeat
                            SendTrueState(DroneAI::State::Approaching);
                            SetAction(DroneAI::Action::AccelToTarget);
                            m_sendCmd = true;
                            SetOrbit();
                        } else {
                            // nope, single use only.
                            _log(DRONE__AI_TRACE, "%s - return to idle.", m_droneSE->GetName());
                            move = false;
                            SetIdle();
                            break;
                        }
                    }
                } break;
                case DroneAI::Action::Engaged: {
                    // has mining cycle finished?
                    if (m_processTimer.Check(false)) {
                        _log(DRONE__AI_TRACE, "%s - mining cycle complete.  return to ship.", m_droneSE->GetName());
                        SendTrueState(DroneAI::State::ReturnHome);
                        SetAction(DroneAI::Action::AccelToShip);
                        m_moveTime = GetTimeMSeconds();
                        sLog.Error("movetime", "set");
                        // stop gfx - may not have to if !repeat was set correctly
                        //SendGFX();
                        // get mined ore
                        MineTarget();
                        // stop timer
                        m_processTimer.Disable();
                        // return to ship
                        m_sendCmd = true;
                        SetOrbit();
                    }
                } break;
                case DroneAI::Action::OrbitTarget: {
                    // already at asteroid
                    if (InOrbitDistance(m_targetSE)) {
                        _log(DRONE__AI_TRACE, "%s - arrived at target. begin mining.", m_droneSE->GetName());
                        move = false;
                        // we are close enough to begin  stop moving and start mining
                        SendTrueState(DroneAI::State::Mining);
                        SetAction(DroneAI::Action::Engaged);
                        // begin mining timer
                        m_processTimer.Start(m_cycleTime);
                        m_startTime = GetFileTimeNow();
                        m_moveTime = GetTimeMSeconds();
                        sLog.Error("movetime", "set");
                        SendGFX();
                    } else {
                        _log(DRONE__AI_TRACE, "%s - too far from target to engage. begin travel.", m_droneSE->GetName());
                        SendTrueState(DroneAI::State::Approaching);
                        SetAction(DroneAI::Action::AccelToTarget);
                        SetOrbit();
                    }
                } break;
                case DroneAI::Action::DecelToStop: {
                    _log(DRONE__AI_TRACE, "%s - mining cycle complete.  decel to stop.", m_droneSE->GetName());
                    SetIdle();
                } break;
                default: {
                    // this isnt right...action should never be idle when mining
                    _log(DRONE__ERROR, "%s - state is %s but action is %s.", \
                            m_droneSE->GetName(), GetStateName(m_state), GetActionName(m_action));
                    move = false;
                    SetIdle();
                } break;
            }
        } break;

        case DroneAI::State::Engaged: {
            if (!TargetValid()) {
                move = false;
                SetIdle();
                break;
            }
            switch (m_action) {
                case DroneAI::Action::Engaged: {
                    if (m_mainAttackTimer.Check())
                        AttackTarget();
                } break;
                case DroneAI::Action::AccelToTarget:
                case DroneAI::Action::DecelToTarget: {
                    if (InEngageDistance(m_targetSE))
                        SetEngaged();
                } break;
                case DroneAI::Action::DecelToStop: {
                    move = false;
                    SetIdle();
                } break;
                default: {
                    // error
                    _log(DRONE__AI_TRACE, "%s - hmmmmm... state is Engaged but action is %s.", \
                            m_droneSE->GetName(), GetActionName(m_action));
                    move = false;
                    SetIdle();
                } break;
            }
        } break;

        case DroneAI::State::ReturnHome: {
            switch (m_action) {
                case DroneAI::Action::AccelToShip:
                case DroneAI::Action::DecelToShip: {
                    if (InActionDistance(m_assignedShipSE)) {
                        _log(DRONE__AI_TRACE, "%s - close enough.  orbiting ship.", m_droneSE->GetName());
                        move = false;
                        SetAction(DroneAI::Action::OrbitShip);
                        SetOrbit();
                    }
                } break;
                case DroneAI::Action::OrbitTarget: {
                    _log(DRONE__AI_TRACE, "%s - leaving target and returning home.", m_droneSE->GetName());
                    move = false;
                    SetAction(DroneAI::Action::AccelToShip);
                    SetOrbit();
                } break;
                case DroneAI::Action::OrbitShip: {
                    _log(DRONE__AI_TRACE, "%s - has returned home.  orbiting home ship", m_droneSE->GetName());
                    move = false;
                    SendTrueState(DroneAI::State::Idle);
                    SetState(DroneAI::State::Idle);
                } break;
                case DroneAI::Action::Idle: {
                    _log(DRONE__AI_TRACE, "%s - hmmmmm... state is ReturnHome but action is idle.", m_droneSE->GetName());
                    move = false;
                    SetIdle();
                } break;
                case DroneAI::Action::DecelToStop: {
                    _log(DRONE__AI_TRACE, "%s - returning home.  decel to stop", m_droneSE->GetName());
                    move = false;
                    SetIdle();
                } break;
                default: {
                    // error
                    _log(DRONE__AI_TRACE, "%s - hmmmmm... state is ReturnHome but action is %s.", \
                            m_droneSE->GetName(), GetActionName(m_action));
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
                    if (InActionDistance(m_assignedShipSE)) {
                        _log(DRONE__AI_TRACE, "%s - close enough.  docking to bay.", m_droneSE->GetName());
                        m_assignedShipSE->ScoopDrone(m_droneSE);
                        m_droneSE->SystemMgr()->RemoveEntity(m_droneSE);
                        m_droneSE->SystemMgr()->AddToDeleteLater(m_droneSE);
                        return;
                    }
                } break;
                case DroneAI::Action::OrbitTarget: {
                    _log(DRONE__AI_TRACE, "%s - leaving target and returning home.", m_droneSE->GetName());
                    move = false;
                    SetAction(DroneAI::Action::AccelToShip);
                    SetOrbit();
                } break;
                case DroneAI::Action::Idle: {
                    _log(DRONE__AI_TRACE, "%s - hmmmmm... state is ReturnBay but action is idle.", m_droneSE->GetName());
                    move = false;
                    SetIdle();
                } break;
                case DroneAI::Action::DecelToStop: {
                    _log(DRONE__AI_TRACE, "%s - returning to bay.  decel to stop", m_droneSE->GetName());
                    SetState(DroneAI::State::Idle);
                    // stop processing until another call hits
                    move = false;
                    m_moveTime = 0;
                    sLog.Error("movetime", "0");
                } break;
                default: {
                    // error
                    _log(DRONE__AI_TRACE, "%s - hmmmmm... state is ReturnBay but action is %s.", \
                            m_droneSE->GetName(), GetActionName(m_action));
                    SetState(DroneAI::State::Idle);
                    // stop processing until another call hits
                    move = false;
                    m_moveTime = 0;
                    sLog.Error("movetime", "0");
                }
            }
        } break;

        // not sure how im gonna do these...
        case DroneAI::State::Fleeing:
        case DroneAI::State::Operating:
        case DroneAI::State::Guarding:
        case DroneAI::State::Assisting:
        case DroneAI::State::Combat:
        case DroneAI::State::Approaching:
        case DroneAI::State::Pursuit: {
            // do nothing here yet
            if (!TargetValid()) {
                SetIdle();
                move = false;
                break;
            }
        } break;

        case DroneAI::State::Invalid: {
            // check everything in this state.   return to ship?
            _log(DRONE__ERROR, "%s - state is %s but action is %s.", \
                    m_droneSE->GetName(), GetStateName(m_state), GetActionName(m_action));
            SetIdle();
            move = false;
        } break;

        default: {
            _log(DRONE__ERROR, "%s - Hit Default:  state is %s but action is %s.", \
                    m_droneSE->GetName(), GetStateName(m_state), GetActionName(m_action));
            SetIdle();
            move = false;
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
        // there are only a few commands right now, so check them first
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
                    sLog.Error("movetime", "set");
                }
                return;
            } else {
                SetState(DroneAI::State::ReturnBay);
            }
            m_sendCmd = true;
            SetOrbit();
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
                    sLog.Error("movetime", "set");
                }
                return;
            } else {
                SetState(DroneAI::State::ReturnHome);
            }
            m_sendCmd = true;
            SetOrbit();
        } break;
        case DroneAI::State::Mining: {
            // are we currently mining?
            if (m_state == DroneAI::State::Mining) {
                m_repeat = false;
                PyDict* data = new PyDict();
                data->SetItemString("targetTypeName", new PyString(m_droneSE->GetName()));
                PyTuple* error = new PyTuple(2);
                error->SetItem(0, new PyString("EntityCurrentlyMining"));
                error->SetItem(1, data);
                dict->SetItem(new PyInt(m_droneSE->GetID()), error);
                return;
            }

            m_sendCmd = true;
            m_repeat = repeat;
            SetState(DroneAI::State::Mining);

            // determine target distance to set action
            if (InOrbitDistance(m_targetSE)) {
                SetAction(DroneAI::Action::OrbitTarget);
            } else {
                SetAction(DroneAI::Action::AccelToTarget);
            }

            SetOrbit();  //try to get inside orbit range
        } break;
        case DroneAI::State::Engaged: {
            // this is can be either attack or aid, depending on drone type
            //  player has confirmed choice, so proceed with requested action
            m_sendCmd = true;
            SetState(DroneAI::State::Engaged);
            SetOrbit();
        } break;
        case DroneAI::State::Idle: {
            sLog.Warning("DroneEngage()", "sent idle...why?");
        } break;
        // these arent coded yet, so ignore them for now
        case DroneAI::State::Guarding:
        case DroneAI::State::Operating:
        case DroneAI::State::Fleeing:
        case DroneAI::State::Assisting:
        case DroneAI::State::Combat:
        case DroneAI::State::Approaching:
        case DroneAI::State::Pursuit: {
        } break;
        /* this is negative...will never be sent thru here
        case DroneAI::State::Incapacitated:
        */
    }
}

void DroneAIMgr::SetIdle() {
    if (m_state == DroneAI::State::Idle)
        return;
    if (!m_droneSE->IsEnabled())
        return;

    _log(DRONE__AI_TRACE, "%s(%u): SetIdle: returning to idle.",
            m_droneSE->GetName(), m_droneSE->GetID());

    // set action to idle first.  gfx checks this to set active
    /*   may not need this
    SetAction(DroneAI::Action::Idle);
    if (m_state > DroneAI::State::Idle) {
        sLog.Warning("DroneAIMgr::SetIdle()", " state is %s for %s", GetStateName(m_state), m_droneSE->GetName());
        // probably had some active gfx before returning to idle
        SendGFX();
    } */

    m_startTime = 0;
    ClearTargets();

    // disable ewar timers
    m_webifierTimer.Disable();
    m_beginFindTarget.Disable();
    m_mainAttackTimer.Disable();
    m_warpScramblerTimer.Disable();

    if (m_droneSE->InControlDistance()) {
        // update orbit command
        m_sendCmd = true;
        SetState(DroneAI::State::ReturnHome);
        if (InActionDistance(m_assignedShipSE)) {
            SendTrueState(DroneAI::State::Idle);
            SetAction(DroneAI::Action::OrbitShip);
        } else {
            SetAction(DroneAI::Action::AccelToShip);
        }
        SetOrbit();
    } else {
        if (m_droneSE->IsEnabled()) {
            // we're idle and outside control distance.  not good
            m_droneSE->DisableDrone();
            m_assignedShipSE->UpdateBandwidth(m_droneSE);
        }
        m_moveTime = 0;
        sLog.Error("movetime", "0");
        SendTrueState(DroneAI::State::Idle);
        SetState(DroneAI::State::Invalid);
        SetAction(DroneAI::Action::Invalid);
    }
}

void DroneAIMgr::SetEngaged() {
    _log(DRONE__AI_TRACE, "Drone %s(%u): SetEngaged: %s(%u) begin engaging.",
            m_droneSE->GetName(), m_droneSE->GetID(), m_targetSE->GetName(), m_targetSE->GetID());

    // actively engaged with target
    m_startTime = GetFileTimeNow();
    SetAction(DroneAI::Action::Engaged);
    //SendShipEffect(true);
}


void DroneAIMgr::MineTarget() {
    // note:  there are no ice harvesting drones
    // when mining drone's target is depleted, drone will get half cycle and not count in ore removed for module count
    float cycleVol(m_droneSE->GetSelf()->GetAttribute(AttrMiningAmount).get_float());

    InventoryItemRef roidRef(m_targetSE->GetSelf());
    float oreAmount(cycleVol / (roidRef->GetAttribute(AttrVolume).get_float()));
    if (oreAmount <= 0) {
        // drone cannot mine this heavy ore
        m_assignedShipSE->GetPilot()->SendNotifyMsg("Mining operations for %s have been deactivated.<br>This drone cannot mine the %s ore.", \
        m_droneSE->GetName(), m_ore->name());
        SetIdle();
        return;
    }

    uint32 ownerID(m_assignedShipSE->GetOwnerID());
    // if ship is owned by corp, set owner of ore to pilot
    if (IsCorpID(ownerID))
        ownerID = m_assignedShipSE->GetPilot()->GetCharID();
    ItemData idata(roidRef->typeID(), ownerID, locTemp, flagNone, oreAmount);
    m_ore = sItemFactory.SpawnItem(idata);
    if (m_ore.get() == nullptr) {
        _log(DRONE__WARNING, "Could not create mined ore for %s assigned to %s", \
        m_droneSE->GetName(), m_assignedShipSE->GetPilot()->GetName());
        m_assignedShipSE->GetPilot()->SendNotifyMsg("Mining operations for %s have been deactivated.<br>There was an error gathering %s ore.", \
                    m_droneSE->GetName(), m_ore->name());
        SetIdle();
        return;
    }

    // add data to StatisticMgr
    sStatMgr.Add(Stat::oreMined, cycleVol);
}

void DroneAIMgr::Target(SystemEntity* pTarget) {
    // check for changing targets first
    if (m_targetSE == pTarget) {
        // same target.  do nothing
        return;
    } else {
        // different target.  check current status
    }

    _log(DRONE__AI_TRACE, "%s(%u) Target() - %s(%s).", \
            m_droneSE->GetName(), m_droneSE->GetID(), GetStateName(m_state), GetActionName(m_action));

    // all is good, set new target
    m_targetSE = pTarget;
    bool chase(false);  // chase ref isnt used for drones
    if (!m_droneSE->TargetMgr()->StartTargeting(pTarget,
                                m_droneSE->GetSelf()->GetAttribute(AttrScanSpeed).get_float(),
                                (uint8)m_droneSE->GetSelf()->GetAttribute(AttrMaxLockedTargets).get_int(),
                                m_assignedShipSE->GetSelf()->GetAttribute(AttrMaxTargetRange).get_double(), chase))
    {
        _log(DRONE__AI_TRACE, "Drone %s(%u): Targeting of %s(%u) failed.  Clear Target and Return to Idle.",
             m_droneSE->GetName(), m_droneSE->GetID(), pTarget->GetName(), pTarget->GetID());
        SetIdle();
        return;
    }

    // lets set a heading
    GVector targHeading(m_droneSE->GetPosition(), m_targetSE->GetPosition());
    targHeading.normalize();
    m_heading = std::move(targHeading);

    m_beginFindTarget.Disable();
    m_mainAttackTimer.Start(m_cycleTime);
}

void DroneAIMgr::SetOrbit() {
    /* we are gonna fake this one....
     * tell client that drone is orbiting
     * client will show travel and we'll track travel;  drones dont use Follow, Goto, or Approach packets
     * when our tracked distance is within orbit distance, stop drone at some point along orbit radius
     *  then do whatever actions are required until target changes
     * rinse and repeat
     */

    // get current target so we can calculate distance and set targetID properly
    int64 distance(0);
    int32 targetID(0), orbitRange(m_orbitRange);
    // target depends on which way we going
    switch (m_action) {
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
                    distance = m_droneSE->GetPosition().distance(m_assignedShipSE->GetPosition());
                    distance -= m_assignedShipSE->GetRadius();
                    targetID = m_assignedShipSE->GetID();
                    orbitRange += m_assignedShipSE->GetRadius();
                    // update heading
                    GVector targHeading(m_droneSE->GetPosition(), m_assignedShipSE->GetPosition());
                    targHeading.normalize();
                    m_heading = std::move(targHeading);
                } break;
                // these are probably target
                case DroneAI::State::Combat:
                case DroneAI::State::Mining:
                case DroneAI::State::Engaged:
                case DroneAI::State::Pursuit:
                case DroneAI::State::Operating:
                case DroneAI::State::Approaching: {     // im gonna go with "Approaching Target" here....
                    distance = m_droneSE->GetPosition().distance(m_targetSE->GetPosition());
                    distance -= m_targetSE->GetRadius();
                    targetID = m_targetSE->GetID();
                    orbitRange += m_targetSE->GetRadius();
                    // update heading
                    GVector targHeading(m_droneSE->GetPosition(), m_targetSE->GetPosition());
                    targHeading.normalize();
                    m_heading = std::move(targHeading);
                } break;
                // this should never hit
                case DroneAI::State::Invalid:
                case DroneAI::State::Incapacitated: {
                    sLog.Warning("Drone::SetOrbit()", "%s(%s) hit...wtf?", GetStateName(m_state), GetActionName(m_action));
                } break;
            }
        } break;
        case DroneAI::Action::Engaged:
        case DroneAI::Action::AccelToTarget:
        case DroneAI::Action::DecelToTarget: {
            // target is target
            distance = m_droneSE->GetPosition().distance(m_targetSE->GetPosition());
            distance -= m_targetSE->GetRadius();
            targetID = m_targetSE->GetID();
            orbitRange += m_targetSE->GetRadius();
            // update heading
            GVector targHeading(m_droneSE->GetPosition(), m_targetSE->GetPosition());
            targHeading.normalize();
            m_heading = std::move(targHeading);
        } break;
        case DroneAI::Action::AccelToShip:
        case DroneAI::Action::DecelToShip: {
            // target is assigned ship
            distance = m_droneSE->GetPosition().distance(m_assignedShipSE->GetPosition());
            distance -= m_assignedShipSE->GetRadius();
            targetID = m_assignedShipSE->GetID();
            orbitRange += m_assignedShipSE->GetRadius();
            // update heading
            GVector targHeading(m_droneSE->GetPosition(), m_assignedShipSE->GetPosition());
            targHeading.normalize();
            m_heading = std::move(targHeading);
        } break;
        case DroneAI::Action::DecelToStop: {
            // at this point, we're idle and decel, so no target...should we have one?
            // this is for target gone or drone Incapacitated
            m_heading = NULL_ORIGIN;
            _log(DRONE__AI_TRACE, "%s - SetOrbit() called.  decel to stop", m_droneSE->GetName());
        } break;
        case DroneAI::Action::Idle:             // no target
        case DroneAI::Action::Invalid: {
            // this shouldnt hit.
            m_heading = NULL_ORIGIN;
            _log(DRONE__AI_TRACE, "%s - SetOrbit() called.  idle or invalid.", m_droneSE->GetName());
        }
    }

    /*
    if (!IsValidTarget(targetID) or (distance < 1)) {
        _log(DRONE__ERROR, "%s - SetOrbit() called for %s.  targetID or distance invalid.", \
                m_droneSE->GetName(), GetActionName(m_action));
        return;
    } */

    // so far, so good...so what?
    m_moveTime = GetTimeMSeconds();
    sLog.Error("movetime", "set");
    m_prevSpeedFraction = m_activeSpeedFraction;

    // for travel, sf=1.0 within orbit distance otherwise sf = orbit speed / max speed
    if (distance > m_chaseRange) {
        //travel required; set full speed
        m_userSpeedFraction = 1.0f;
        m_accelTime = (-log(ASF_CHECK) * m_agility);
    } else {
        // close enough impulse drives; set orbit speed
        m_userSpeedFraction = m_cruiseSpeed / m_maxSpeed;
        m_accelTime = (-log(ASF_CHECK) * m_agility);
        m_accelTime *= m_accelTime;
    }

    if (m_accelTime < 1.0f)
        m_accelTime = 1.0f;

    sLog.Cyan("SetOrbit()", "%s(%u) - %s(%s):  set usf to %.2f.  distance to target is %lli.   %ssending packet.", \
            m_droneSE->GetName(), m_droneSE->GetID(), GetStateName(m_state), GetActionName(m_action), \
            m_userSpeedFraction, distance, (m_sendCmd?"":"not "));

    // check m_orbitRange to be sure it's not some crazy shit...
    if (orbitRange > 3000)
        orbitRange /= 2;

    //  orbit is only called once per target, then sf adjusted based on distance
    std::vector<PyTuple*> updates;
    CmdSetSpeedFraction ssf;
        ssf.entityID = m_droneSE->GetID();
        ssf.fraction = m_userSpeedFraction;
    updates.push_back(ssf.Encode());

    if (m_sendCmd) {
        sLog.Yellow("SetOrbit()", "sending CmdOrbit packet");
        m_sendCmd = false;
        CmdOrbit du;
            du.entityID = m_droneSE->GetID();
            du.orbitEntityID = targetID;
            du.distance = orbitRange;
        updates.push_back(du.Encode());
    }

   m_droneSE->SysBubble()->BubblecastDestinyUpdate(updates, "destiny drone");
}

void DroneAIMgr::AssignShip(ShipSE* pSE) {
    m_assignedShipSE = pSE;
    if (m_assignedShipSE == nullptr)
        return;

    if (m_assignedShipSE->GetSelf()->HasAttribute(AttrOreHoldCapacity)) {
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
    m_mainAttackTimer.Disable();
    m_warpScramblerTimer.Disable();

    SetState(DroneAI::State::Invalid);
    SetAction(DroneAI::Action::Invalid);

    if (m_assignedShipSE != nullptr)
        m_assignedShipSE->UpdateBandwidth(m_droneSE);

    m_assignedShipSE = nullptr;
    m_targetSE = nullptr;
}

void DroneAIMgr::Targeted(SystemEntity* pAgressor) {
    _log(DRONE__AI_TRACE, "Drone %s(%u): Targeted by %s(%u) while %s & %s.", \
                m_droneSE->GetName(), m_droneSE->GetID(), pAgressor->GetName(), \
                pAgressor->GetID(), GetStateName(m_state)), GetActionName(m_action);

    // TODO:  send warning to controlling ship
    std::string text = "target lock on me";
    //01110100 01100001 01110010 01100111 01100101 01110100 00100000 01101100 01101111 01100011 01101011 00100000 01101111 01101110 00100000 01101101 01100101
    // convert string to binary
    m_assignedShipSE->GetPilot()->SendNotifyMsg(BinString(text).c_str());
}

bool DroneAIMgr::TargetValid() {
    if (m_targetSE == nullptr)
        return false;
    if (m_targetSE->SysBubble() == nullptr)
        return false;

    DestinyManager* pDestiny = m_targetSE->DestinyMgr();
    if (pDestiny == nullptr) {
        _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) has no destiny manager.",
                m_droneSE->GetName(), m_droneSE->GetID(), m_targetSE->GetName(), m_targetSE->GetID());
        return false;
    }

    // Check to see if the target is not cloaked:
    if (pDestiny->IsCloaked()) {
        _log(DRONE__AI_TRACE, "Drone %s(%u): Target %s(%u) is cloaked.",
                m_droneSE->GetName(), m_droneSE->GetID(), m_targetSE->GetName(), m_targetSE->GetID());
        return false;
    }
    return true;
}

void DroneAIMgr::ClearTarget() {
    m_targetSE = nullptr;
    m_droneSE->TargetMgr()->ClearTarget(m_targetSE);
    //m_droneSE->TargetMgr()->OnTarget(pSE, TargMgr::Mode::Lost);

    if (m_droneSE->TargetMgr()->HasNoTargets())
        SetIdle();
}

void DroneAIMgr::TargetLost(SystemEntity* pTarget) {
    switch(m_action) {
        case DroneAI::Action::Engaged: {
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

void DroneAIMgr::ClearTargets() {
    m_repeat = false;
    m_targetSE = nullptr;
    m_droneSE->TargetMgr()->ClearTargets();
}

void DroneAIMgr::ClearAllTargets() {
    m_targetSE = nullptr;
    m_repeat = false;
    m_droneSE->TargetMgr()->ClearAllTargets();
    //m_droneSE->TargetMgr()->OnTarget(nullptr, TargMgr::Mode::Clear, TargMgr::Msg::ClientReq);
}

//also check for special effects and write code to implement them
//modifyTargetSpeedRange, modifyTargetSpeedChance
//entityWarpScrambleChance
void DroneAIMgr::AttackTarget() {
    if (!m_droneSE->TargetMgr()->CanAttack())
        return;

    SendGFX();
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

void DroneAIMgr::SendTrueState(int8 state) {
    _log(DRONE__AI_TRACE, "%s(%u) - sending true state %s.", m_droneSE->GetName(), m_droneSE->GetID(), GetStateName(state));
    OnDroneStateChange du;
        du.droneID = m_droneSE->GetID();
        du.ownerID = m_droneSE->GetOwnerID();
        du.controllerID = m_droneSE->GetControllerID();
        du.activityState = state;
        du.droneTypeID = m_droneSE->GetTypeID();
        du.controllerOwnerID = m_droneSE->GetControllerOwnerID();
        du.targetID = (GetTargetID() == 0 ? PyStatic.NewNone() : new PyInt(GetTargetID()));
    PyTuple* up(du.Encode());
    if (m_droneSE->SysBubble() != nullptr)
        m_droneSE->SysBubble()->BubblecastDestinyUpdate(&up, "destiny");
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
        case DroneAI::Action::OrbitTarget:
        case DroneAI::Action::AccelToShip:
        case DroneAI::Action::DecelToShip:
        case DroneAI::Action::AccelToTarget:
        case DroneAI::Action::DecelToTarget: {
            return m_attackRange;
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
            return m_targetSE->GetID();
        } break;
        case DroneAI::Action::OrbitShip:
        case DroneAI::Action::AccelToShip:
        case DroneAI::Action::DecelToShip: {
            return m_assignedShipSE->GetID();
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
            return m_targetSE;
        } break;
        case DroneAI::Action::OrbitShip:
        case DroneAI::Action::AccelToShip:
        case DroneAI::Action::DecelToShip: {
            return m_assignedShipSE;
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
            return (float)m_cruiseSpeed / m_maxSpeed;
        }
    }
    return 1.0f;
}

bool DroneAIMgr::InActionDistance(SystemEntity* pTarget) {
    double dist(m_droneSE->GetPosition().distance(pTarget->GetPosition()) - pTarget->GetRadius());
    return (dist < sConfig.drone.InteractDistace);
}

bool DroneAIMgr::InOrbitDistance(SystemEntity* pTarget) {
    double dist(m_droneSE->GetPosition().distance(pTarget->GetPosition()) - pTarget->GetRadius());
    return (dist < m_orbitRange);
}
bool DroneAIMgr::InFalloffDistance(SystemEntity* pTarget) {
    double dist(m_droneSE->GetPosition().distance(pTarget->GetPosition()) - pTarget->GetRadius());
    return (dist < m_falloffRange);
}

bool DroneAIMgr::InEngageDistance(SystemEntity* pTarget) {
    double dist(m_droneSE->GetPosition().distance(pTarget->GetPosition()) - pTarget->GetRadius());
    return (dist < m_attackRange);
}

bool DroneAIMgr::InChaseDistance(SystemEntity* pTarget) {
    double dist(m_droneSE->GetPosition().distance(pTarget->GetPosition()) - pTarget->GetRadius());
    return (dist < m_chaseRange);
}

bool DroneAIMgr::InMaxDistance(SystemEntity* pTarget) {
    double dist(m_droneSE->GetPosition().distance(pTarget->GetPosition()) - pTarget->GetRadius());
    return (dist < m_maxRange);
}


// destiny methods below...

void DroneAIMgr::Stop() {
    // not sure how im gonna handle this yet.
    // only called when drone offline or disabled
    if (m_droneSE->IsEnabled())
        m_droneSE->DisableDrone();

    m_velocity = NULL_ORIGIN_V;
    UpdatePosition();
    m_moveTime = 0;
    sLog.Error("movetime", "0");
    m_startTime = 0;
    SetState(DroneAI::State::Invalid);

    if (m_droneSE->SysBubble() != nullptr) {
        CmdStop stop;
            stop.entityID = m_droneSE->GetID();
        PyTuple* up = stop.Encode();
        // edit  m_droneSE->SysBubble()->BubblecastDestinyUpdate(&up, "destiny drone");
    }
}

void DroneAIMgr::Move(double timeStamp) {
    // this will keep our position ref accurate, so we do need somewhat accurate processing
    //  note that we are hacking this, and not actually orbiting anything

    // is this right?  if set to idle, dont drones have to slow to stop?  maybe a couple, but most are < 1.5s accel
    if ((m_state < DroneAI::State::Combat) and (m_action == DroneAI::Action::DecelToStop)) {
        _log(DRONE__AI_TRACE, "%s - Move() called.  %s and decel to stop", m_droneSE->GetName(), GetStateName(m_state));
        m_moveTime = 0;
        sLog.Error("movetime", "0");
        return;
    }

    uint32 targDistance(0);
    uint32 shipDistance(m_droneSE->GetPosition().distance(m_assignedShipSE->GetPosition()));
    shipDistance -= m_assignedShipSE->GetRadius();
    if (m_targetSE != nullptr)
        targDistance = m_droneSE->GetPosition().distance(m_targetSE->GetPosition() - m_targetSE->GetRadius());

    // if we're still traveling, we will need to do accel/decel and keep track of timestamps like destiny does
    // some drones have accel/decel times > 5s
    bool accel(false), decel(false), stop(false);

    // determine current action
    switch (m_action) {
        case DroneAI::Action::Idle:
        case DroneAI::Action::Engaged:
        case DroneAI::Action::Invalid:
        case DroneAI::Action::OrbitShip:
        case DroneAI::Action::OrbitTarget: {
            // these require no movement
            return;
        } break;
        case DroneAI::Action::AccelToShip:
        case DroneAI::Action::AccelToTarget: {
            accel = true;
        } break;
        case DroneAI::Action::DecelToStop: {
            // im sure i'll need this, but havent determined particulars yet
            stop = true;
        }  // fallthru on purpose
        case DroneAI::Action::DecelToShip:
        case DroneAI::Action::DecelToTarget: {
            decel = true;
        } break;
    }

    int32 speed(0);

    // check to see if our target has moved.  if so, update position accordingly

    if ((timeStamp > m_accelTime) and (m_timeFraction > 0.99f)) {
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
                m_droneSE->GetName(), m_accelTime, timeStamp, m_timeFraction);
    }

    _log(DRONE__MOVE, "Move() - %s(%u) is %s at %u m/s (tf:%.4f asf:%.4f).", \
            m_droneSE->GetName(), m_droneSE->GetID(), (accel ? "Accel" : (decel ? "Decel" : "Steady")), \
            speed, m_timeFraction, m_activeSpeedFraction);
    _log(DRONE__AI_TRACE, "Move() - %s(%u):  targDistance: %u, shipDistance: %u", \
            m_droneSE->GetName(), m_droneSE->GetID(), targDistance, shipDistance);

    m_velocity = m_heading * speed;
    UpdatePosition(false);
}

void DroneAIMgr::UpdatePosition(bool update/*false*/) {
    // basic position updating - variables updated elsewhere
    GVector pos(m_droneSE->GetPosition());
    pos += m_velocity;
    m_droneSE->SetPosition(pos);

    if (sEntityMgr.GetTracking())
        MarkPoint(pos);

    // should we send position updates?  probably not, as long as we're kinda close to what client has
    if (update) {
        SetBallPosition du;
            du.entityID = m_droneSE->GetID();
            du.x = pos.x;
            du.y = pos.y;
            du.z = pos.z;
        PyTuple* up = du.Encode();
        m_droneSE->SysBubble()->BubblecastDestinyUpdate(&up, "DestinyUpdates");
        PyDecRef(up);
    }
}

void DroneAIMgr::SendSpeedFraction() {
    CmdSetSpeedFraction ssf;
        ssf.entityID = m_droneSE->GetID();
        ssf.fraction = m_userSpeedFraction;
    PyTuple* up = ssf.Encode();
    if (m_droneSE->SysBubble() != nullptr)
        ;// edit  m_droneSE->SysBubble()->BubblecastDestinyUpdate(&up, "destiny drone");
}

/*
 * // do drones warp??   they can, yes...with limitations
 * if (mySE->IsDroneSE()) {
 *    // put drone limit checks here
 *    sLog.Warning("DroneWarp", "Drone %s (from ship %s) warping from bID %u to bID %u", \
 *    mySE->GetName(), mySE->GetDroneSE()->GetShipSE()->GetName(), \
 *    mySE->SysBubble()->GetID(), m_targBubble->GetID());
 * } */

void DroneAIMgr::MissileLaunched(Missile* pMissile) {
    // TODO:  check mode, state and actions then react
    std::string text = "missile inbound";
    //01101101 01101001 01110011 01110011 01101001 01101100 01100101 00100000 01101001 01101110 01100010 01101111 01110101 01101110 01100100
    // convert string to binary
    m_assignedShipSE->GetPilot()->SendNotifyMsg(BinString(text).c_str());
}

void DroneAIMgr::ReportDamage(uint8 type/*0*/) {
    // TODO:  check mode, state and actions then react
    std::string text = "damaged";
    //01100100 01100001 01101101 01100001 01100111 01100101 01100100
    // convert string to binary
    m_assignedShipSE->GetPilot()->SendNotifyMsg(BinString(text).c_str());
}

void DroneAIMgr::SendGFX(Client* pClient/*nullptr*/) {
    sLog.Blue("Drone", "sending gfx");
    if (m_effectID < 1) {
        // not necessarily an error.  just make note
        sLog.Error("DroneAI::SendGFX()", "m_effectID < 1 for %s.", m_droneSE->GetName());
        //EvE::traceStack();
        return;
    }

    bool active(false), start(false);
    if (m_action == DroneAI::Action::Engaged) {
        start = true;
        active = true;
    }

    InventoryItemRef iRef = m_droneSE->GetSelf();
    // effects are listed in EVE_Effects.h
    //  NOTE: drones are called 'entities' in client; EVE_Effects has 'entityxxx' for gfx...may not be used like this.
    uint16 gfxID(0);
    if (m_booster and iRef->HasAttribute(AttrGfxBoosterID)) {  // graphicID for turret for drone type ships
        gfxID = iRef->GetAttribute(AttrGfxBoosterID).get_uint32();
    } else if (iRef->HasAttribute(AttrGfxTurretID)) {  // graphicID for turret for drone type ships
        gfxID = iRef->GetAttribute(AttrGfxTurretID).get_uint32();
    }

    // send their actual start time, even when !start
    // exceptions are missiles and turrets
    bool useStartTime(true);
    switch (m_effectID) {
        case EvE::GFXID::projectileFired:
        case EvE::GFXID::targetAttack:
            useStartTime = false;
    }

    /*  not sure if this is right for drones...
    std::string guidStr = sFxDataMgr.GetEffectGuid(gfxID);
    if (guidStr.empty())
        guidStr = sFxDataMgr.GetEffectGuid(iRef->type().GetDefaultEffect());
    */
    std::string guidStr = sFxDataMgr.GetEffectGuid(m_effectID);

    OnSpecialFX14 effect;
        effect.entityID = iRef->itemID();
        effect.moduleID = iRef->itemID();             // npc UID for npc's/drones
        effect.moduleTypeID = iRef->typeID();     // npc typeID for npc's/drones
        effect.targetID = (m_targetSE == nullptr ? PyStatic.NewNone() : new PyInt(m_targetSE->GetID()));
        effect.otherTypeID = PyStatic.NewNone();
        effect.area = PyStatic.mtList();        // no data.  not used in client
        effect.guid = std::move(guidStr);
        effect.isOffensive = sFxDataMgr.isOffensive(m_effectID);       // bool
        effect.start = start;                   // int bool
        effect.active = active;               // int bool
        effect.duration = m_cycleTime;             // in ms
        effect.repeat = 0;                 // dont allow repeat
        effect.startTime = (useStartTime ? m_startTime : GetFileTimeNow());
        effect.graphicInfo = (gfxID == 0 ? PyStatic.NewNone() : new PyInt(gfxID));
    PyTuple *up = effect.Encode();
    if (is_log_enabled(EFFECTS__DUMP))
        up->Dump(EFFECTS__DUMP, "");

    if (pClient == nullptr) {
        m_droneSE->SysBubble()->BubblecastDestinyUpdate(&up, "DestinyUpdates");
    } else {
        // this is to update new ship in bubble with active gfx
        pClient->QueueDestinyUpdate(&up);
    }
    PyDecRef(up);
}

void DroneAIMgr::SendShipEffect(bool start/*false*/) {
    // NOTE:  not sure if this is needed or not....test without first...
    // MUST have target to start/stop effect

    //def OnGodmaShipEffect(self, itemID, effectID, t, start, active, environment, startTime, duration, repeat, randomSeed, error, actualStopTime = None, stall = True):
    GodmaEnvironment ge;
        ge.selfID = m_droneSE->GetID();                 //ENV_IDX_SELF = 0
        ge.charID = m_droneSE->GetOwnerID();            //ENV_IDX_CHAR = 1
        ge.shipID = m_droneSE->GetID();                 //ENV_IDX_SHIP = 2
        ge.target = (m_targetSE == nullptr ? PyStatic.NewNone() : new PyInt(m_targetSE->GetID()));     //ENV_IDX_TARGET = 3
        ge.subLoc = PyStatic.NewNone();                 //ENV_IDX_OTHER = 4
        ge.area = PyStatic.mtList();                    //ENV_IDX_AREA = 5 still dont know what this is...always empty
        ge.effectID = m_effectID;                       //ENV_IDX_EFFECT = 6

    OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = GetFileTimeNow();
        shipEff.start = (start ? 1 : 0);
        shipEff.active = (start ? 1 : 0);
        shipEff.environment = ge.Encode();
        shipEff.startTime = m_startTime;
        shipEff.duration = m_cycleTime;
        shipEff.repeat = PyStatic.NewNone();
        shipEff.randomSeed = PyStatic.NewNone();
        shipEff.error = PyStatic.NewNone();

    PyTuple* tuple = shipEff.Encode();
    if (is_log_enabled(EFFECTS__DUMP))
        tuple->Dump(EFFECTS__DUMP, "");

    m_droneSE->SysBubble()->BubblecastDestinyEvent(&tuple, "DestinyEvent");
}

void DroneAIMgr::SetAction(int8 action/*-1*/) {
    _log(DRONE__AI_TRACE, "%s(%u) - setting action from %s to %s.", \
            m_droneSE->GetName(), m_droneSE->GetID(), GetActionName(m_action), GetActionName(action));
    m_action = action;
}

void DroneAIMgr::SetState(int8 state/*-1*/) {
    _log(DRONE__AI_TRACE, "%s(%u) - setting state from %s to %s.", \
            m_droneSE->GetName(), m_droneSE->GetID(), GetStateName(m_state), GetStateName(state));
    m_state = state;
}

void DroneAIMgr::MarkPoint(const GPoint& position) {
    std::string name = "drone marker", desc = "";
    // create jetcan to visualize point in space
    ItemData idata(23, ownerSystem, m_droneSE->GetLocationID(), flagNone, name.c_str(), position, desc.c_str());
    CargoContainerRef cRef = CargoContainerRef::StaticCast(InventoryItem::SpawnTemp(idata));
    if (cRef.get() == nullptr) {
        _log(DESTINY__WARNING, "MarkPoint() could not create Item for drone marker");
        return;
    }

    // create new container
    FactionData data = FactionData();
    ContainerSE* cSE = new ContainerSE(cRef, m_droneSE->GetServices(), m_droneSE->SystemMgr(), data);
    if (cSE == nullptr) {
        _log(DESTINY__WARNING, "MarkPoint() could not create SE for drone");
        return;
    }
    cRef->SetMySE(cSE);
    cSE->AnchorContainer();
    m_droneSE->SystemMgr()->AddMarker(cSE);
}

int8 DroneAIMgr::GetState() {
    switch (m_state) {
        case DroneAI::State::Invalid:
            return DroneAI::State::Idle;
        case DroneAI::State::Guarding:
        case DroneAI::State::Assisting:
            return DroneAI::State::Engaged;
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
        case DroneAI::State::ReturnHome:      return "\033[1mReturning to ship\033[0m";
        case DroneAI::State::ReturnBay:       return "\033[1mReturning to Bay\033[0m";
        case DroneAI::State::Pursuit:         return "\033[1mPursuit\033[0m";
        case DroneAI::State::Engaged:         return "\033[1mEngaged\033[0m";
        case DroneAI::State::Fleeing:         return "\033[1mFleeing\033[0m";
        case DroneAI::State::Operating:       return "\033[1mOperating\033[0m";
        case DroneAI::State::Assisting:       return "\033[1mAssisting\033[0m";
        case DroneAI::State::Guarding:        return "\033[1mGuarding\033[0m";
        case DroneAI::State::Incapacitated:   return "\033[1mIncapacitated\033[0m";
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
 *
 *            // action, orbit, falloff, engage, chase, max
 *            if (InActionDistance(m_targetSE)) {
 *            } else if (InOrbitDistance(m_targetSE)) {
 *            } else if (InFalloffDistance(m_targetSE)) {
 *            } else if (InEngageDistance(m_targetSE)) {
 *            } else if (InChaseDistance(m_targetSE)) {
 *            } else if (InMaxDistance(m_targetSE)) {
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
    } break;
}
*/
