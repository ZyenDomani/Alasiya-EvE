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
*/

// this class is for objects that move

#include "EVEServerConfig.h"

#include "Client.h"
#include "EntityList.h"
#include "PyServiceMgr.h"
#include "StaticDataMgr.h"
#include "map/MapData.h"
#include "math/Trig.h"
#include "npc/Drone.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "packets/Missile.h"
#include "ship/Missile.h"
#include "ship/Ship.h"
#include "station/Station.h"
#include "station/StationDataMgr.h"
#include "system/BubbleManager.h"
#include "system/Container.h"
#include "system/DestinyManager.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"


// fixed variable init order  -allan 15Feb23
DestinyManager::DestinyManager(SystemEntity *self)
: mySE(self), m_targBubble(nullptr), m_ballMode(Destiny::Ball::Mode::STOP), m_hasSentShipUpdates(false), m_radius(self->GetRadius()),
m_warpCapacitorNeed(0.00001), m_alignTime(5), m_prevSpeed(0.0f), m_maxShipSpeed(100.0f), m_shipWarpSpeed(1.0f), m_speedToLeaveWarp(100), m_maxSpeed(1.0f),
m_shipAccelTime(0.0f), m_shipMaxAccelTime(0.0f), m_stop(false), m_accel(false), m_decel(false), m_cloaked(false), m_turning(false),
m_tractored(false), m_tractorPause(false), m_orbiting(0), m_stateStamp(0), m_degPerTic(0.0f), m_orbitTime(0.0f), m_orbitRadTic(0.0f), m_radians(0.0),
m_timeFraction(0.0f), m_turnMinFraction(0), m_prevSpeedFraction(0.0f), m_userSpeedFraction(0.0f), m_activeSpeedFraction(0.0f), m_maxOrbitSpeedFraction(1.0f),
m_turnTime(0), m_followDistance(0), m_targetDistance(0), m_moveTime(0.0),
m_position(self->GetPosition()), m_velocity(NULL_ORIGIN_V), m_targetPoint(NULL_ORIGIN), m_shipHeading(NULL_ORIGIN_V), m_targetHeading(NULL_ORIGIN_V),
m_autoPilot(false), m_alignTo(false), m_frozen(false), m_changeDelay(false), m_moveDelay(false), m_agility(0.0), m_bump(false),
m_posHack(sConfig.debug.PositionHack), m_turnAccel(false), m_turnDecel(false), m_turnPct(0.0f),
m_origHeading(NULL_ORIGIN_V), m_curveStart(NULL_ORIGIN), m_curveApex(NULL_ORIGIN), m_curveEnd(NULL_ORIGIN),
m_inclination(0.0), m_longAscNode(0.0), m_decelTime(0), m_warpState(nullptr)
{
    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;
}

DestinyManager::~DestinyManager() {
    SafeDelete(m_warpState);
}

// this is called once per tic by SystemEntity::Process() called by SystemManager::Process()
void DestinyManager::Process() {
    double profileStartTime(GetTimeUSeconds());

    ProcessState();

    //SendDestinyUpdates(m_updateQueue);
    //m_updateQueue.clear();

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::destiny, GetTimeUSeconds() - profileStartTime);
}

void DestinyManager::ProcessState() {
    /*  not implemented yet...
    if (mySE->IsFrozen()) {
        Halt();
        return;
    }  */

    using namespace Destiny;
    switch(m_ballMode) {
        case Ball::Mode::STOP: {
            if (IsMoving())
                MoveObject();
            return;
        } break;
        case Ball::Mode::GOTO: {
            MoveObject();
        } break;
        case Ball::Mode::MISSILE: {
            // if target was removed, continue movement and wait for Missile::EndOfLife() call to do cleanup
            //set current direction based on our position and targPos.  this will keep missile (internally) aligned properly
            GVector moveVector(m_position, m_targetEntity.second->GetPosition());
            moveVector.normalize();
            //set position and direction for this round of movement
            m_shipHeading = moveVector;
            m_velocity = (moveVector * m_maxSpeed);
            m_position += m_velocity;
            SetPosition(m_position);
            return;
        } break;
        case Ball::Mode::ORBIT: {
            if (IsTargetInvalid())
                return;
            Orbit();
        } break;
        case Ball::Mode::FOLLOW: {
            if (IsTargetInvalid())
                return;
            Follow();
        } break;
        case Ball::Mode::WARP: {
            /*
             * There are three stages of warp, which are functions of time, speed and distance:
             *
             *  1) Acceleration.
             *      this is a fixed attribute, which is roughly 9s to full warp speed for all ships
             *  2) Cruising.
             *      traveling at maximum warp speed
             *  3) Deceleration.
             *      this also is a fixed attribute, which is roughly 22s from full warp speed for all ships
             *
             *  Acceleration and Deceleration are logarithmic with finite caps (instead of infinity) at the ends.
             *      see also:  my notes in InitWarp()
             */
            if (m_warpState != nullptr) {
                //warp is in progress
                uint16 sec_into_warp = (sEntityList.GetStamp() - m_stateStamp);
                //  speed and distance formulas based on current warp distance
                if (m_warpState->accel) {
                    WarpAccel(sec_into_warp);
                } else if (m_warpState->cruise) {
                    WarpCruise(sec_into_warp);
                } else if (m_warpState->decel) {
                    WarpDecel(sec_into_warp);
                } else {// uh, Houston...we have a problem...
                    if (mySE->HasPilot()) {
                        _log(DESTINY__ERROR, "Destiny::ProcessState() Error!  Ship %s(%u) for Player %s(%u) Has WarpState but checks are false.",  \
                                    mySE->GetName(), mySE->GetID(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());
                        mySE->GetPilot()->SendErrorMsg("Internal Server Error.<br> Please Dock, Jump or Relog to reset your ship.");
                    } else {
                        _log(DESTINY__ERROR, "Destiny::ProcessState() Error!  NPC %s(%u) Has WarpState but checks are false.",  \
                                    mySE->GetName(), mySE->GetID());
                    }
                }
                return;
            }

            MoveObject();

            if (!m_turning and (m_activeSpeedFraction > 0.74999f)) {
                m_shipHeading = m_targetHeading;
                InitWarp();
                return;
            } else if (((GetTimeMSeconds() - m_moveTime) * 0.001f) > m_alignTime) {
                // catchall for turn checks messed up, and m_moveTime > ship's align time
                if (mySE->HasPilot()) {
                    _log(DESTINY__ERROR, "Destiny::ProcessState() Error!  Ship %s(%u) for Player %s(%u) - warp align/speed is incorrect, but time > alignTime (%.1f > %u).  asf: %.3f",  \
                                mySE->GetName(), mySE->GetID(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID(), ((GetTimeMSeconds() - m_moveTime) * 0.001f), m_alignTime, m_activeSpeedFraction);
                    mySE->GetPilot()->SendNotifyMsg("Warp Code Error.  Stopping Ship.");
                    Stop();
                } else {
                    _log(DESTINY__ERROR, "Destiny::ProcessState() Error!  NPC %s(%u) - warp align/speed is incorrect, but time > alignTime (%.1f > %u).  asf: %.3f",  \
                            mySE->GetName(), mySE->GetID(), ((GetTimeMSeconds() - m_moveTime) * 0.001f), m_alignTime, m_activeSpeedFraction);
                    WarpTo(m_targetPoint);
                }
                //m_shipHeading = m_targetHeading;
                //InitWarp();
            }
            // else ship is still aligning/accelerating
        } break;
        // i dont think any of these actually move...
        case Ball::Mode::MUSHROOM:      // aoe?
        case Ball::Mode::BOID:          // this will turn RIGID after a set time
        case Ball::Mode::TROLL:         // seen for wrecks
        case Ball::Mode::MINIBALL:      // used for sentrys around RIGID object
        case Ball::Mode::FIELD:         // dunno
        case Ball::Mode::FORMATION:     // dunno
        case Ball::Mode::RIGID: {       // item that never moves
            //no default on purpose
        } break;
    }
}
/* acceleration forumula
 * V(t) = Vmax*(1-e^-(t/a))
 * V(t) = velocity at time t
 * a = agility
 */
 //Velocity setting methods
void DestinyManager::SetSpeedFraction(float fraction/*1.0*/, bool startMovement/*false*/) {
    // this sets current speed fraction for object.

    // if orbiting, call Orbit() and let code reset the variables
    // i dunno about this one.....
    //if (m_orbiting != 0)
    //    InitOrbit(m_targetEntity.second, m_targetDistance);

    if ((fraction == m_userSpeedFraction) and (!startMovement)) {
        // no change.
        return;
    }

    // this is to start movement when setting fractional speeds from speedo in client.
    //  also a hack to circumvent above check when called again by goto, warp, align or follow for changing direction.
    if (startMovement) {
        m_stop = false;
        if (m_autoPilot) {
            m_ballMode = Destiny::Ball::Mode::FOLLOW;
        } else {
            m_ballMode = Destiny::Ball::Mode::GOTO;
        }
    }

    // prevent multiple client calls to Stop() from resetting ship speed.
    if (m_stop)
        return;

    if (is_log_enabled(DESTINY__MOVE_TRACE))
        _log(DESTINY__MOVE_TRACE, "Destiny::SetSpeedFraction() - %s(%u):   prevSpeed:%.2f, fraction: %.2f, start: %s, stop: %s, accel: %s, decel: %s",
             mySE->GetName(), mySE->GetID(), m_prevSpeed, fraction, startMovement ? "true" : "false", m_stop ? "true" : "false", \
             m_accel ? "true" : "false", m_decel ? "true": "false");

    /* movement is set according to time, speed fraction, and objects' maximum configured speed.
     * all *Fraction variables use fuzzy logic
     *  -allan 8Oct14  -major update 20Nov15  -added prop mod code 29Mar17
     *  -base movement rewrite/update 18Oct21
	 *  -turn rewrite 9Feb23
     *
     *  speed is the actual distance an object travels over a given time
     *   -> time is measured in seconds
     *   -> distance measured in meters
     *  m_maxSpeed is ship maximum speed based on user input and current configuration.
     *   -> set in UpdateVelocity()
     *   -> only used for logging
     *  m_maxShipSpeed (MSS) is the maximum speed an object can travel in a given time
     *   -> measured in m/s
     *   -> initially set by UpdateShipVariables() based on ship configuration
     *   -> reset by SpeedBoost() for prop mods
     *  m_userSpeedFraction (USF) is user-set fraction of max ship speed (fractional from speedo or full from command).
     *   -> sets m_maxSpeed
     *   -> used with ASF and MSS to set speed
     *   -> range is 0.0 for stop and 1.0 for full
     *  m_prevSpeedFraction (PSF) is previous speed fraction
     *   -> reset on every speed change
     *   -> used to calculate estimated time to change speed
     *   -> used to set speed when speed changes via user input or prop mod
     *   -> can be > 1.0 based on other factors (deactivating prop mod at full speed will give PSF > 1.0)
     *  m_activeSpeedFraction (ASF) this is the percentage of ship max speed for the given tic.
     *   -> set in MoveObject()
     *   -> uses USF and MSS to set speed
     *  m_timeFraction (TF) holds current euler value for time.
     *   -> this is the current percent of change between previous speed and requested speed.
     *   -> range is 0.0 to 1.0
     *   -> set in MoveObject()
     *   -> sets ASF (directly)
     *   -> sets velocity (indirectly)
     *  m_maxOrbitSpeedFraction (OSF) is ship's max speed based on orbit data
     *   -> may not be used after update
     *  m_stateStamp is server tic when move event began
     *   -> measured in seconds
     *   -> data type is uint16
     *   -> set/reset in BeginMovement() for any/all speed/direction changes
     *   -> used to calculate TF
     *  m_moveTime is (exact) timestamp when move change started
     *   -> data type is (double) FileTime
     *   -> used to calculate TF
     *  m_targetPoint holds current target coords.
     *   -> set by goto, warp, align, follow, orbit
     *   -> sets m_shipHeading
     *  m_shipHeading holds current direction and is set in Turn()
     *   -> used with speed to set m_velocity
     *  m_velocity is current ship velocity.  set in MoveObject()
     *   -> m_velocity = m_shipHeading * (ASF * MSS)
     *   -> this is the variable used for position tracking.
     *  m_shipAccelTime is calculated time to complete requested speed change
     *   -> measured in seconds
     *   -> set on all speed changes
     *  m_shipMaxAccelTime is calculated time for ship to accelerate from stop to full
     *   -> measured in seconds
     *   -> initially set by UpdateShipVariables() based on ship configuration
     *   -> reset by SpeedBoost() for prop mods
     */

    m_prevSpeedFraction = 0.0f;

    if (m_activeSpeedFraction > ASF_CHECK) {
        m_userSpeedFraction = fraction;
        m_prevSpeedFraction = m_activeSpeedFraction;
        UpdateVelocity(true);
    } else {
        m_userSpeedFraction = fraction;
        UpdateVelocity(false);
    }

    std::vector<PyTuple*> updates;
    // send on usf change but not for turn or orbit
    if (!m_turning and !m_orbiting) {
         CmdSetSpeedFraction du;
            du.entityID = mySE->GetID();
            du.fraction = fraction;
            updates.push_back(du.Encode());   //m_updateQueue
        m_hasSentShipUpdates = true;
    }

    if (((mySE->IsNPCSE() or mySE->IsDroneSE()) and !m_hasSentShipUpdates)
    or mySE->IsMissileSE() or mySE->IsContainerSE() or mySE->IsWreckSE()) {
        SetBallSpeed ms;   //NPCs, Missiles, containers, wrecks
            ms.entityID = mySE->GetID();
            ms.speed = m_maxSpeed;
            updates.push_back(ms.Encode());
        m_hasSentShipUpdates = true;
    }

    if (!updates.empty())
        SendDestinyUpdates(updates); //consumed
}

void DestinyManager::UpdateVelocity(bool isMoving) {
    uint8 logType(0);
    if ((m_ballMode == Destiny::Ball::Mode::WARP) and (m_warpState != nullptr)) {
        /*  Warp() finished, and ship dropped out of warp at m_speedToLeaveWarp,
         * set variables for decel from this speed.
         */
        logType = 1;
        m_stop = true;
        m_decel = true;
        m_posHack = true;
        m_ballMode = Destiny::Ball::Mode::STOP;
        m_targBubble = nullptr;
        m_shipAccelTime = m_shipMaxAccelTime; // * m_activeSpeedFraction;
    } else if (m_userSpeedFraction > 0.01f) {
        // commanded speed fraction > 0 and ...
        float delta(1.0f);
        if ((m_activeSpeedFraction == m_userSpeedFraction) and (!m_prevSpeed)) {
            // ... nothing has changed.
            logType = 8;
            m_prevSpeedFraction = 0.0f;
        } else if (isMoving) {
            //  ... ship is moving and ...
            if (m_activeSpeedFraction > m_userSpeedFraction) {
                // ... request is lower than current speed - begin decel
                logType = 4;
                m_accel = false;
                m_decel = true;
                delta = m_activeSpeedFraction - m_userSpeedFraction;
            } else if (m_userSpeedFraction > m_activeSpeedFraction) {
                // ... request is higher than current speed - begin accel
                logType = 3;
                m_accel = true;
                m_decel = false;
                delta = m_userSpeedFraction - m_activeSpeedFraction;
            }

            m_shipAccelTime = m_shipMaxAccelTime * delta;
            m_prevSpeed = m_maxShipSpeed * m_activeSpeedFraction;
        } else {
            // ... ship is not moving. begin movement
            logType = 2;
            m_accel = true;
            m_decel = false;
            // this isnt spot on, but pretty damn close
            m_maxSpeed = m_maxShipSpeed * m_userSpeedFraction;
            // this isnt accurate...hulk decel @81.35s but asf:0.0020 @ sec: 84.372
            m_shipAccelTime = m_shipMaxAccelTime * m_userSpeedFraction;      // for accel with user speeds <= 1.0
        }
        if (is_log_enabled(DESTINY__MOVE_TRACE))
            _log(DESTINY__MOVE_TRACE, "Destiny::UpdateVelocity - %s(%u): Speed Change - USF: %.2f, ASF: %.2f, TF: %.2f, PSF: %.2f, pSpeed: %.2f, mSpeed: %.2f, accel: %s, decel: %s, delta: %.3f", \
            mySE->GetName(), mySE->GetID(), m_userSpeedFraction, m_activeSpeedFraction, m_timeFraction, m_prevSpeedFraction, m_prevSpeed, m_maxSpeed, \
                 m_accel ? "true" : "false", m_decel ? "true": "false", delta);
    } else if (m_activeSpeedFraction > ASF_CHECK) {
        //  commanded to stop while ship is moving.  begin decelerating
        logType = 5;
        if (m_turning)
            logType = 6;
        m_accel = false;
        m_decel = true;
        //m_maxSpeed = 0.0f;
        m_prevSpeed = m_maxShipSpeed * m_activeSpeedFraction;
        // this isnt accurate...hulk decel @81.35s but asf:0.0020 @ sec: 84.372
        m_shipAccelTime = m_shipMaxAccelTime * m_activeSpeedFraction;
    } else {
        // ... ship is not moving.  reset all move vars by calling Halt()
        logType = 7;
        Halt();
    }

    if (m_shipAccelTime < 0)
        sLog.Error("Destiny::UpdateVelocity()", "Accel Time is negative: %.2f", m_shipAccelTime);

    if (is_log_enabled(DESTINY__MOVE_TRACE)) {
        std::string msg = "";
        switch (logType) {
            case 1: { msg = "ship dropped out of warp.      --Begin Decel"; }       break;
            case 2: { msg = "USF != 0 and ship is Stopped.  --Begin Accel"; }       break;
            case 3: { msg = "USF != 0 and ship is Moving.   --Begin Accel"; }       break;
            case 4: { msg = "USF != 0 and ship is Moving.   --Begin Decel"; }       break;
            case 5: { msg = "USF == 0 and ship is Moving.   --Decel for Stop"; }    break;
            case 6: { msg = "USF == 0 and ship is Moving.   --Decel for Turn"; }    break;
            case 7: { msg = "USF == 0 and ship is Stopped.  --Halt"; }              break;
            case 8: { msg = "ASF == USF                     --No Change"; }         break;
        }
        _log(DESTINY__MOVE_TRACE, "Destiny::UpdateVelocity - %s(%u):  %s  AccelTime: %.2f, USF: %.2f, ASF: %.2f, TF: %.2f, PSF: %.2f", \
                mySE->GetName(), mySE->GetID(), msg.c_str(), m_shipAccelTime, m_userSpeedFraction, \
                m_activeSpeedFraction, m_timeFraction, m_prevSpeedFraction);
    }
}

//Global Actions:
void DestinyManager::Stop() {
    sLog.Warning("DestinyManager", "%s calling stop", mySE->GetName());
    if (m_stop)
        return;

    if (m_userSpeedFraction == 0.0f) {
        //state is already at stop. but m_stop wasnt set.
        // set m_stop and return.
        m_stop = true;
        return;
    }

    // set marker for calc'd stop distance (testing)
    if (is_log_enabled(DESTINY__WARP_DEBUG)) {
        uint16 dist = m_maxShipSpeed * m_activeSpeedFraction * m_agility;
        GVector offset(m_position * dist);
        GPoint marker(m_position + offset);
        std::string str = "Stop Point - ";
        str += mySE->GetName();
        MarkPoint(marker, str, str);
    }

    m_accel = false;
    m_decel = false;
    m_alignTo = false;
    m_posHack = false;
    m_autoPilot = false;
    m_prevSpeed = 0.0f;
    m_prevSpeedFraction = 0.0f;

    if (m_orbiting)
        ClearOrbit();
    if (m_turning)
        ClearTurn();

    m_stateStamp = sEntityList.GetStamp();
    m_moveTime = GetTimeMSeconds();

    // need to check this after rewrite
    SetSpeedFraction(0.0f);
    m_stop = true;

    m_targBubble = nullptr;

    SafeDelete(m_warpState);

    if (m_ballMode != Destiny::Ball::Mode::STOP) {
        m_ballMode = Destiny::Ball::Mode::STOP;
        CmdStop du;
            du.entityID = mySE->GetID();
        PyTuple *up = du.Encode();
        SendSingleDestinyUpdate(&up);
        //PyDecRef(up);
    }
}

void DestinyManager::Halt(bool commanded/*false*/) {
    SafeDelete(m_warpState);

    //  reset ALL movement variables and states.  calling this will set object to a COMPLETE and IMMEDIATE stop.
    m_ballMode = Destiny::Ball::Mode::STOP;
    ClearTurn();
    ClearOrbit();

    m_stop = true;
    m_accel = false;
    m_decel = false;
    m_alignTo = false;
    m_autoPilot = false;
    m_maxSpeed = 0.0f;
    m_moveTime = 0.0;
    m_prevSpeed = 0.0f;
    m_stateStamp = 0;
    m_timeFraction = 0.0f;
    m_targetDistance = 0;
    m_followDistance = 0;
    m_prevSpeedFraction = 0.0f;
    m_userSpeedFraction = 0.0f;
    m_activeSpeedFraction = 0.0f;
    m_maxOrbitSpeedFraction = 1.0f;

    m_velocity = GVector(NULL_ORIGIN);
    m_targetPoint = GPoint(NULL_ORIGIN);
    m_targetHeading = GVector( NULL_ORIGIN );
    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;

    if (is_log_enabled(DESTINY__MOVE_TRACE))
        _log(DESTINY__MOVE_TRACE, "Destiny::Halt(%s) - %s(%u) Halted - m_shipHeading: %.3f,%.3f,%.3f", \
                (commanded ? "true" : "false"), mySE->GetName(), mySE->GetID(), m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);

    if (commanded) {
        // immediate halt via command.  send packet to stop ship.
        CmdStop du;
            du.entityID = mySE->GetID();
        PyTuple *up = du.Encode();
        SendSingleDestinyUpdate(&up);
        //PyDecRef(up);
    }
}

void DestinyManager::Eject()
{
    // basic updates for ejecting from ship
    Stop();
    UpdateOldShip(mySE->GetShipSE());
    SendJettisonPacket();
}

// Global collision methods
//  check for collision.  called by Move()
void DestinyManager::CheckBump()
{
    double profileStartTime(GetTimeUSeconds());

    //  collision detection code here
    /*  in this case, we are ONLY interested in objects
     *   that have drifted within each others radius (for whatever reason)
     *  this only checks for ships running sub-warp speeds
     *   in relation to other objects in bubble.
     */

    // NOTE:  object's "massive = true" means it can bump/collide  (massive = solid)
    // will need massive checks here

    // initial implementation will ONLY check player ships for bumping.
    std::vector<Client*> vPlayers;
    mySE->SysBubble()->GetPlayers(vPlayers);
    Client* pClient = mySE->GetPilot();
    GPoint pos(GetPosition());
    float distance = 0.0f;
    for (auto &cur : vPlayers) {
        if (cur == pClient)
            continue;
        distance = pos.distance(cur->GetShipSE()->GetPosition());
        distance -= (mySE->GetRadius() - cur->GetShipSE()->GetRadius());
        if (distance < BUMP_DISTANCE) {
            Bump(cur->GetShipSE());
            m_bump = true;
        } else {
            m_bump = false;
        }
    }
    /** @todo  add data and checks for each ship bumped
     * to give single bump msg for each ship combo
     * without spamming their overview
     */

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::collision, GetTimeUSeconds() - profileStartTime);
}

void DestinyManager::Bump(SystemEntity* pSE)
{
    if (m_bump)
        return;
    // bump code here
    /*  determine most massive object...maybe not.  use percentiles here (becham math)
     *  determine direction(s) involved
     *  determine speed(s) involved
     *  determine new headings based on above
     *  determine new speed based on above
     *
     * NOTE static or large objects wont move, but will apply equal and opposite force.
     * un-anchored objects WILL move (jetcans, wrecks, portable hangers)
     */

    /* bump math, by Scheulagh Santorine, Ph.D.
     *  velocity of bumped object immediately after bump
     * v2(t=0+) = 2v1*m1/m1+m2
     */

    /*  run-time options for bumping jetcans, biomass, and other space objects
     *   bump drones??  prolly not, for simplicity
     */
    std::string msg1 = "You have bumped ";
    msg1 += pSE->GetPilot()->GetName();
    mySE->GetPilot()->SendNotifyMsg(msg1.c_str());
    // this test isnt needed right now, as it's ONLY checking against players and will always return true.
    //  will keep it in here for later expansion.
    if (pSE->HasPilot()) {
        std::string msg2 = "You have been bumped by ";
        msg2 += mySE->GetPilot()->GetName();
        pSE->GetPilot()->SendNotifyMsg(msg2.c_str());
    }
}

void DestinyManager::Bounce(GVector direction, float speed)
{
    // bounce code here (not used yet)
    /*  this code will update ship movement after being bumped
     *  all items will drift to a complete stop, unless other movement is called.
     */
    m_ballMode = Destiny::Ball::Mode::GOTO;
    m_stop = false;
    m_stateStamp = sEntityList.GetStamp();
    m_moveTime = GetTimeMSeconds();
    m_shipAccelTime = 0.1f;
    m_userSpeedFraction = 1.0f;
    m_timeFraction = 1.0f;
    m_maxSpeed = m_maxShipSpeed;
    m_velocity = m_shipHeading * m_maxSpeed;

    std::vector<PyTuple*> updates;
    SetBallVelocity bv;
        bv.entityID = mySE->GetID();
        bv.x = m_velocity.x;
        bv.y = m_velocity.y;
        bv.z = m_velocity.z;
    updates.push_back(bv.Encode());
    CmdGotoDirection du;
        du.entityID = mySE->GetID();
        du.x = m_shipHeading.x;
        du.y = m_shipHeading.y;
        du.z = m_shipHeading.z;
    updates.push_back(du.Encode());
    SendDestinyUpdates(updates); //consumed
    Stop();
}

// main movement method
void DestinyManager::MoveObject() {
    // this shouldnt be needed...
    //if (mySE->SysBubble() == nullptr)
    //    mySE->SystemMgr()->AddEntity(mySE);

    if (m_changeDelay) {
        // reset m_moveTime and skip this tic
        // only used by undock
        m_changeDelay = false;
        m_moveTime = GetTimeMSeconds() - EvE::Time::Second;
        m_stateStamp = sEntityList.GetStamp();
        _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - ChangeDelay - %s(%u): stateStamp: %u", \
                mySE->GetName(), mySE->GetID(), m_stateStamp);
        return;
    }

    // currently not used.
    if (m_moveDelay) {
        // reset m_moveTime to now and skip this tic
        m_moveDelay = false;
        m_moveTime = GetTimeMSeconds();
        m_stateStamp = sEntityList.GetStamp();
        _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - MoveDelay - %s(%u): stateStamp: %u", \
                mySE->GetName(), mySE->GetID(), m_stateStamp);
        return;
    }

    if (m_tractored) {
        // no accel/decel for tractored items.  all speed is immediate.
        m_position += m_velocity;
        mySE->SetPosition(m_position);
        return;
    }

    /* acceleration and deceleration are both logarithmic and the server needs to keep up with client position.
     * formula for time taken to accelerate from v to V, from https://wiki.eveonline.com/en/wiki/Acceleration
     *
     *   t=IM(10^-6) * -e(1-(v/V))
     *   m_shipAccelTime = agility * -e(1-(v/V))
     *
     * as this uses the natural log, the higher the speed, the slower the acceleration, to the limits of ln(0)
     * since lim ln(x) = -INFINITY where x->0+. and ln(0) is undefined, we will use
     *
     *   m_shipMaxAccelTime = (-ln(ASF_CHECK) * agility);   where ASF_CHECK is currently 0.001
     *
     * to define the time it will take a given ship to reach 98% of m_maxShipSpeed, at which point,
     * the server will set m_velocity = (m_maxShipSpeed * direction) (or 100% ship speed).
     *
     * to define speed at X time, we will use the following equation.
     *
     *   Vt = Vm * (1 - e(-t * 10^6 / IM))
     *
     * where
     * Vt = ships velocity at t
     * Vm = ships maximum velocity
     *  t = time
     *  I = ships inertia in s/kg
     *  M = ships mass in kg
     *  e = base of natural logarithms
     */

    /* **UPDATE**  this now uses time AND (m_timeFraction > 0.999f) for min/max speeds.  -allan 6Aug14
     * **UPDATE**  this is now tracking ALL speed changes correctly.  -allan 21Nov15
     * **UPDATE**  initial orbit implementation.  -allan 13July16
     * **UPDATE**  removed speed fraction checks for min/max speeds.  -allan 02Jul17
     * **UPDATE**  reworked entire basic movement checks and formulas -allan 17Oct21
     * **UPDATE**  tweak movement to align with client                -allan 04Jan23
     * **UPDATE**  rewrite turning logic                              -allan 04Feb23
	 this needs to update stopping to follow "distance to stop".   stopDist = agility * speed
	 however, using formula correctly should account for that...
     */


    float speed(0.0f);
    std::string move = "";
    // keep timer in seconds.
    float timeStamp((GetTimeMSeconds() - m_moveTime) * 0.001f);
    //timeStamp = sEntityList.GetStamp() - m_stateStamp;
    // update tf for this tic
    m_timeFraction = (1 - exp(-timeStamp / m_agility)); //mySE->GetSelf()->GetAttribute(AttrAgility).get_float()));

    if ((timeStamp > m_shipAccelTime) and (m_timeFraction > (1 - ASF_CHECK))) {
        if (m_decel) {
            m_decel = false;
            m_prevSpeed = 0.0f;
            m_prevSpeedFraction = 0.0f;

            if (is_log_enabled(DESTINY__MOVE_TRACE))
                _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u) has decel'd from %.2fm/s to %.2fm/s in %.3fs.", \
                mySE->GetName(), mySE->GetID(), m_prevSpeed, m_maxSpeed * m_activeSpeedFraction, timeStamp);
        }
        if (m_accel) {
            m_accel = false;
            m_prevSpeed = 0.0f;
            m_prevSpeedFraction = 0.0f;

            if (is_log_enabled(DESTINY__MOVE_TRACE))
                _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u) has accel'd from %.2fm/s to %.2fm/s in %.3fs.", \
                mySE->GetName(), mySE->GetID(), m_prevSpeed, m_maxSpeed * m_activeSpeedFraction, timeStamp);
        }

        if (m_userSpeedFraction) {
            // ship has reached full commanded speed
            move = "at full commanded speed, going";
            m_activeSpeedFraction = m_userSpeedFraction;
            speed = m_maxSpeed * m_activeSpeedFraction;
        } else {
            //ship has reached full stop
            if (is_log_enabled(DESTINY__MOVE_TRACE))
                _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u) is at full stop after %.3f seconds.", \
                    mySE->GetName(), mySE->GetID(), timeStamp);

            m_velocity = m_shipHeading * m_maxSpeed * m_activeSpeedFraction;
            m_position += m_velocity;
            SetPosition(m_position, true);
            // this should never get here after warping with ap.
            // if it does, we'll have to code something to ignore it.
            Halt(true);
            return;
        }
    } else {
        // changed speed and asf != usf
        if (m_accel) {
            // object still accelerating.
            move = "accelerating";
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
        } else if (m_decel) {
            // object still decelerating.
            move = "decelerating";
            if (m_prevSpeedFraction) {
                // asf = psf - (psf - usf) * tf
                m_activeSpeedFraction = m_prevSpeedFraction - ((m_prevSpeedFraction - m_userSpeedFraction) * m_timeFraction);
            } else {
                // this should never hit....should not have decel w/o psf
                sLog.Warning("Destiny::MoveObject()", "decel = true, but psf = 0.");
            }
        } else if (m_tractored or m_tractorPause) {
            ;   // do nothing here.  this is to remove error reporting from next line.
        } else {
            sLog.Error("Destiny::MoveObject()", "%s(%u) - move checks are not set right. Acc:%s, Dec:%s, timeStamp:%.3f, Tractored:%s, TractorPause:%s", \
                    mySE->GetName(), mySE->GetID(), (m_accel ? "True" : "False"), (m_decel ? "True" : "False"), \
                    timeStamp, (m_tractored ? "True" : "False"), (m_tractorPause ? "True" : "False"));
            Stop();
            return;
        }

        speed = (m_maxSpeed * m_activeSpeedFraction);
    }

    if (m_stop and !m_autoPilot) {
    // ships tend to "level out" when stopping.  try to mimic that here (wip)
    // this will also need *something* with ship agility/inertia
    // using rifter to check/set base numbers
        if (m_activeSpeedFraction < 0.6f)
            if (m_shipHeading.y < -0.1f) {
                m_shipHeading.y += 0.03f;
            } else if (m_shipHeading.y > 0.1f) {
                m_shipHeading.y -= 0.03f;
            }
    }

    //set velocity and position for this tic
    if (m_turning) {
        Turn(speed, move);
    } else if (m_orbiting) {
        // this will need updating...
        if (m_orbiting < Destiny::Ball::Orbit::TooClose) {
            // object is orbiting...set orbit speed correctly.
            speed *= m_maxOrbitSpeedFraction;  //<---  this may not be right.
            move += " in orbit";
        }
    } else {
        // floating point math is slow.
        m_velocity = m_shipHeading * speed;
        m_position += m_velocity;
        mySE->SetPosition(m_position);
    }

    if (is_log_enabled(DESTINY__MOVE_TRACE)) {
        if (m_prevSpeedFraction) {
            _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u) is %s at %.3f m/s (tf:%.4f asf:%.4f ps:%.2f psf:%.4f).", \
                mySE->GetName(), mySE->GetID(), move.c_str(), speed, m_timeFraction, m_activeSpeedFraction, m_prevSpeed, m_prevSpeedFraction);
        } else {
            _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u) is %s at %.3f m/s (tf:%.4f asf:%.4f).", \
                mySE->GetName(), mySE->GetID(), move.c_str(), speed, m_timeFraction, m_activeSpeedFraction);
        }
    }

    // will need to hack position setting after turn cause it's still wrong.
    if (sConfig.debug.PositionHack or m_posHack) {
        SetPosition(m_position, true);   // force position update to client
        m_posHack = false;
    }

    if (is_log_enabled(DESTINY__MOVE_DEBUG))
        _log(DESTINY__MOVE_DEBUG, "Destiny::MoveObject() - %s(%u) Pos:%.2f,%.2f,%.2f  Vel:%.3f,%.3f,%.3f  Head:%.3f,%.3f,%.3f", \
            mySE->GetName(), mySE->GetID(), m_position.x, m_position.y, m_position.z, m_velocity.x, m_velocity.y, m_velocity.z,\
            m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);

    if (sEntityList.GetTracking()) {
        // only create can when ship is moving significant amount
        if (m_activeSpeedFraction > sConfig.debug.ShipTrackingTime) {
            // create jetcan to visualize movement
            std::string str = mySE->GetName();
            if (m_turning)
                str += " Turn ";
            if (m_decel)
                str += " Decel ";
            if (m_accel)
                str += " Accel ";
            if (!m_turning and !m_accel and !m_decel)
                str += " Steady ";
            str += itoa(timeStamp);
            MarkPoint(m_position, str, str);
        }
    }

    if (sConfig.cosmic.BumpEnabled
    and mySE->HasPilot()
    and mySE->SysBubble()->HasPlayers()) // no players in bubble = nothing to check against (for now)
        CheckBump();
}

bool DestinyManager::IsTurn() {    //this is working.  dont change.
    // if ship is (reasonably) stopped, there is no turn.  immediately begin movement in desired direction
    // note....this MAY depend on ship agility....will have to test with big ships
    if (m_activeSpeedFraction < 0.1) {
        m_shipHeading = m_targetHeading;
        return false;
    }

    /* check for almost equal direction
    if (EvE::AlmostEquals(m_targetHeading.x, m_shipHeading.x, 3)
    and EvE::AlmostEquals(m_targetHeading.y, m_shipHeading.y, 3)
    and EvE::AlmostEquals(m_targetHeading.z, m_shipHeading.z, 3))
        return false;
    */
    float dot(m_targetHeading.dotProduct(m_shipHeading));
    // this only happens when heading and target are almost exact (or wrong)
    while (dot > 1.0f)
        dot -= 1;
    while (dot < -1.0f)
        dot += 1;
    // check for turning angle.  returns true if angle is enough to change movement variables
    //float degrees = m_shipHeading.angle(m_targetHeading);
    //if (degrees < TURN_ALIGNMENT) {             //  TURN_ALIGNMENT = 4* = 0.0698132 rad   WARP_ALIGNMENT = 6* = 0.10472 rad
    //  this will set m_radians in the range of [0,pi].
    m_radians = acos(dot);
    if (m_radians < 0.10472) {         //  TURN_ALIGNMENT = 4* = 0.0698132 rad    6* = 0.10472 rad
        m_shipHeading = m_targetHeading;
        _log(DESTINY__TURN_TRACE, "Destiny::IsTurn(false) - %s(%u): degrees:%.4f", \
                mySE->GetName(), mySE->GetID(), EvE::Trig::Rad2Deg(m_radians));
        return false;
    }

    if (is_log_enabled(DESTINY__TURN_TRACE)) {
        _log(DESTINY__TURN_TRACE, "Destiny::IsTurn(true) - %s(%u): degrees:%.4f", \
                mySE->GetName(), mySE->GetID(), EvE::Trig::Rad2Deg(m_radians));
        _log(DESTINY__TURN_TRACE, "Destiny::IsTurn() m_shipHeading: %.7f,%.7f,%.7f.  m_targetHeading: %.7f,%.7f,%.7f", \
            m_shipHeading.x, m_shipHeading.y, m_shipHeading.z, m_targetHeading.x, m_targetHeading.y, m_targetHeading.z);
    }

    return true;
}

// much better than old system
void DestinyManager::InitTurn()
{
    /* as per Dr. Santorine's studies, all ships turn the same.
     *   ship will slow down to [min speed] for turn depending on heading change
     *   while decel, ship will turn a slight amount until that [min speed] is hit
     * this follows a simple quadratic Bezier Curve (conic arc)
     *  turn is calculated on the fly to deal with changing ship speeds
     */

    // just to be sure....
    m_stop = false;

    // set turning.
    m_turning = true;
    m_turnTime = 1;
    m_curveStart = GetPosition();
    m_origHeading = m_shipHeading;
    // reset move stamps
    m_stateStamp = sEntityList.GetStamp();

    // determine actual angle of turn for subsequent calc's
    GVector toVec(m_position, m_targetPoint);
    toVec.normalize();
    float degrees = acos(toVec.dotProduct(m_shipHeading));
    //  calc min speed for this turn as absolute percent of shipMaxSpeed
    float minTurnSpeedFraction = sqrt((cos(degrees) + 1) / 2);
    // adjust turn shit based on degree of turn and ship agility
    float alignTime = (float)m_alignTime - ((float)m_alignTime * minTurnSpeedFraction);
    m_alignTime = (uint8)ceil(alignTime);
    m_turnPct = 1.0f / m_alignTime;

    // check speed for changes and set vars accordingly
    if (m_activeSpeedFraction > minTurnSpeedFraction) {
        m_turnDecel = true;
        m_prevSpeedFraction = m_activeSpeedFraction;
        m_turnMinFraction = minTurnSpeedFraction;
    } else {
        m_moveTime = GetTimeMSeconds();
        UpdateVelocity(true);
    }

    if (is_log_enabled(DESTINY__TURN_TRACE))
        _log(DESTINY__TURN_TRACE, "Destiny::InitTurn() - %s(%u): pct:%.2f, steps:%u, alignTime:%f, mtsf:%.3f, decel: %s", \
            mySE->GetName(), mySE->GetID(), m_turnPct, m_alignTime, alignTime, minTurnSpeedFraction, \
            m_turnDecel ? "true" : "false");
}

//NOTE:  new Turn() code
// much better than old system
void DestinyManager::Turn(float &speed, std::string &move) {
    double timer = GetTimeUSeconds();

    float change(m_turnPct * m_turnTime);

    move += " in turn";
    // accel/decel in turn act different. ignore MoveObject() speed and set per turn change here (+5-9us)
    if (m_turnDecel) {
        // our speed is above min for this turn.
        if (m_turnTime > (m_alignTime * 0.5f)) {
            m_turnDecel = false;
            m_turnAccel = true;
            // we are now resuming accel after 1/2 turn
            m_activeSpeedFraction = getPctf(m_prevSpeedFraction, m_turnMinFraction, change * 2);
            speed = m_maxShipSpeed * m_activeSpeedFraction;
            move = "continue accel in turn";
        } else {
            m_activeSpeedFraction = getPctf(m_prevSpeedFraction, m_turnMinFraction, change * 2);
            speed = m_maxShipSpeed * m_activeSpeedFraction;
            move = "decel in turn";
        }
        // at this point, asf < mtsf @ InitTurn().
        // however, we need to make sure asf < mtsf during first half of turn
        if (m_turnTime < (m_alignTime * 0.5f))
            if (m_activeSpeedFraction > m_turnMinFraction)
                ; //sLog.Warning("Turn()", "asf > mtsf during first half.");
            // have to figure out how to keep this down and everything sane at same time.
    }
	if (m_turnAccel) {
        // we are now resuming accel after 1/2 turn
        m_activeSpeedFraction = getPctf(m_prevSpeedFraction, m_turnMinFraction, 1.0f - (change * 2));
        speed = m_maxShipSpeed * m_activeSpeedFraction;
        move = "accel in turn";
    }

    // apex is current ship position + direction * (speed * 1/2 turn time)
    m_curveApex = m_curveStart + (m_origHeading * (speed * (m_alignTime * 0.5f)));
    // apex->end will be parallel with start->targ
    m_curveEnd = m_curveApex + (m_targetHeading * (speed * (m_alignTime * 0.5f)));
    /*
    std::string str = "TurnApex - ";
    str += itoa(m_turnTime);
    MarkPoint(m_curveApex,str,str);
    str.clear();
    str = "TurnEnd - ";
    str += itoa(m_turnTime);
    MarkPoint(m_curveEnd,str,str);
    */

    // The Green Line (reference points)
    double ax = getPct(m_curveStart.x, m_curveApex.x, change);
    double ay = getPct(m_curveStart.y, m_curveApex.y, change);
    double az = getPct(m_curveStart.z, m_curveApex.z, change);
    double bx = getPct(m_curveApex.x, m_curveEnd.x, change);
    double by = getPct(m_curveApex.y, m_curveEnd.y, change);
    double bz = getPct(m_curveApex.z, m_curveEnd.z, change);
    // The Black Dot (our position at this tic)
    m_position.x = getPct(ax, bx, change);
    m_position.y = getPct(ay, by, change);
    m_position.z = getPct(az, bz, change);
    mySE->SetPosition(m_position);

    if (m_turnTime > m_alignTime) {
        // turn is complete.  clear data and return
        GPoint pos(m_position + (m_shipHeading * 1.0e16));
        GVector head(m_position, pos);
        head.normalize();
        m_shipHeading = head;
        if (is_log_enabled(DESTINY__TURN_TRACE))
            _log(DESTINY__TURN_TRACE, "Destiny::Turn(complete) - %s(%u):  turn completed in %us.  Heading:%.7f,%.7f,%.7f", \
                    mySE->GetName(), mySE->GetID(), m_turnTime,  m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);

        /* try updating client to get same heading
        CmdGotoDirection du;
        du.entityID = mySE->GetID();
        du.x = m_shipHeading.x;
        du.y = m_shipHeading.y;
        du.z = m_shipHeading.z;
        PyTuple* up = du.Encode();
        SendSingleDestinyUpdate(&up);
        //PyDecRef(up);
        */

        if (m_alignTo) {
            Stop();
        } else {
            if (m_turnAccel) {
                // we've totally changed asf and all that shit.  reset, update, clear then continue
                m_prevSpeedFraction = m_activeSpeedFraction;
                m_moveTime = GetTimeMSeconds();
                UpdateVelocity(true);
            }
            ClearTurn();
        }
    } else {
        float change2(m_turnPct + change);
        double ax = getPct(m_curveStart.x, m_curveApex.x, change2 );
        double ay = getPct(m_curveStart.y, m_curveApex.y, change2 );
        double az = getPct(m_curveStart.z, m_curveApex.z, change2 );
        double bx = getPct(m_curveApex.x, m_curveEnd.x, change2 );
        double by = getPct(m_curveApex.y, m_curveEnd.y, change2 );
        double bz = getPct(m_curveApex.z, m_curveEnd.z, change2 );
        // The Black Dot (our position at this tic)
        GPoint next(getPct(ax, bx, change2 ),
                    getPct(ay, by, change2 ),
                    getPct(az, bz, change2 ));

        GVector head(m_position, next);
        head.normalize();
        m_shipHeading = head;

        //sLog.Warning("turn", "calc'd in %.3fus", GetTimeUSeconds() - timer);
        /*  this is pretty fuckin fast (before decel checks)
         * 17:10:09 W turn: calc'd in 5.500us
         * 17:10:10 W turn: calc'd in 4.750us
         * 17:10:11 W turn: calc'd in 8.000us
	 *
18:32:17 W turn: calc'd in 1.250us
18:32:18 W turn: calc'd in 11.250us
18:32:18 W turn: calc'd in 1.250us
18:32:19 W turn: calc'd in 5.750us
18:32:19 W turn: calc'd in 1.500us
18:32:20 W turn: calc'd in 5.250us
18:32:20 W turn: calc'd in 1.500us
18:32:21 W turn: calc'd in 5.750us
18:32:21 W turn: calc'd in 1.500us
         */
        if (is_log_enabled(DESTINY__TURN_TRACE))
            _log(DESTINY__TURN_TRACE, "Destiny::Turn() - turnStamp:%u, change:%.2f, Position:%.1f,%.1f,%.1f, Heading:%.7f,%.7f,%.7f  state: %s", \
                m_turnTime++, change, m_position.x, m_position.y, m_position.z, m_shipHeading.x, m_shipHeading.y, m_shipHeading.z, \
            m_decel ? "decel" : m_accel ? "accel" : "steady");
    }
}

void DestinyManager::ClearTurn() {
    sLog.Cyan("DM", "%s calling ClearTurn()", mySE->GetName());
    m_radians = 0.0f;
    m_turnTime = 0;
    m_turning = false;
    m_turnAccel = false;
    m_turnDecel = false;
    m_turnMinFraction = 0;
    if (m_prevSpeedFraction) {
        m_userSpeedFraction = m_prevSpeedFraction;
        m_prevSpeedFraction = 0;
        // chances of ship being stopped when this is called is slim...
        UpdateVelocity(IsMoving());
    }

    m_alignTime = ceil(1.386294 * m_agility);
    m_turnPct = 1.0f / m_alignTime;

    m_curveEnd = NULL_ORIGIN;
    m_curveApex = NULL_ORIGIN;
    m_curveStart = NULL_ORIGIN;
    m_origHeading = NULL_ORIGIN_V;
}

// Follow is also used by client as AlignTo.
void DestinyManager::Follow() {
    // when ap jumps, target is set to self to avoid stop() calls, which breaks ap.
    // ap calls SpeedFraction before follow, which will hit here before target is set
    // this allows my ap hack to work correctly w/o totally fukkering my movement code.
    if (m_autoPilot)
        if (m_targetEntity.first == mySE->GetID())
            return;

    // get target's cur pos and update our targ point
    const GPoint& target_point = m_targetEntity.second->GetPosition();
    GVector heading(m_position, target_point);
    m_targetDistance = (heading.length() - m_radius - m_targetEntity.second->GetRadius());
    heading.normalize();
    m_shipHeading = heading;
    m_targetPoint = target_point + (heading * m_targetDistance);

    if (m_targetDistance < m_followDistance) {
        if (m_autoPilot) {
            if (m_userSpeedFraction > 0.1f)
                SetSpeedFraction(0.1);
            _log(AUTOPILOT__TRACE, "DM::Follow() - targetDistance: %lim, FollowDistance: %um.  usf: %.2f.  asf: %.2f", \
                m_targetDistance, m_followDistance, m_userSpeedFraction, m_activeSpeedFraction);
        } else if (m_tractored) {
    /* this will allow following entities to keep their follow state, yet stop movement if within their follow distance.
     * by keeping their follow state, once the distance is greater than their follow distance,
     * they will begin movement again.
     */
        // specific to tractored entities.  sudden halt to mimic tractor stopping
            if (!m_tractorPause) {
                std::vector<PyTuple*> updates;
                CmdSetSpeedFraction ssf;
                    ssf.entityID = mySE->GetID();
                    ssf.fraction = 0;
                updates.push_back(ssf.Encode());
                SendDestinyUpdates(updates); //consumed
            }
            m_velocity = NULL_ORIGIN_V;
            m_shipHeading = heading;
            m_tractorPause = true;
            m_activeSpeedFraction = m_userSpeedFraction = m_timeFraction = m_prevSpeedFraction = 0.0f;
            return;
        } else {
            if (m_targetEntity.second->IsStaticEntity() or m_targetEntity.second->DestinyMgr()->IsMoving()) {
                // this will mimic real movement, where ship will decel instead of a sudden halt
                //  still need to call MoveObject() here
                SetSpeedFraction(0.1);
            } else {
                Stop();
            }
        }
    } else {
        if (m_tractored and m_tractorPause) {
            // tractored object is outside follow distance.  begin movement again
            if (m_tractorPause) {
                std::vector<PyTuple*> updates;
                CmdSetSpeedFraction ssf;
                    ssf.entityID = mySE->GetID();
                    ssf.fraction = 1;
                updates.push_back(ssf.Encode());
                SendDestinyUpdates(updates); //consumed
            }
            m_tractorPause = false;
            m_shipHeading = heading;
            m_velocity = m_shipHeading * m_maxSpeed;
            m_moveTime = GetTimeMSeconds();
            m_stateStamp = sEntityList.GetStamp();
            m_prevSpeedFraction = 0.0f;
            // there is no accel/decel for tractor'd items
            m_activeSpeedFraction = m_userSpeedFraction = m_timeFraction = 1;
        } else if (m_userSpeedFraction < 0.11f) {
            // this will reset movement.
            SetSpeedFraction(1.0f);
        }
    }

    MoveObject();
}

void DestinyManager::Orbit() {
    // data consistency checks...
    if ((m_targetDistance > BUBBLE_RADIUS_METERS) or (m_followDistance > BUBBLE_RADIUS_METERS)) {
        // well, something fucked up.  stop object and throw error.   player can reset if they want to.
        if (mySE->HasPilot())
            mySE->GetPilot()->SendErrorMsg("Internal Server Error.  Stopping ship.");
        sLog.Error("Destiny::Orbit()", "%s(%u) - Distance check OOB. ", mySE->GetName(), mySE->GetID());
        Stop();
        return;
    }

    // this will set position of ship relative to target, based on period of orbit.

    /*   destiny variables used here
     * m_position - probably the most important calculated value.
     * m_velocity - 2nd most important calculated value
     * m_targetDistance - commanded orbit distance
     * m_followDistance - calculated orbit distance based on mass, velocity, gravity, and other ship variables
     * m_targetHeading - direction to target from current position
     * m_targetPoint - calculated distant point from above variable
     * m_shipHeading - current direction ship is pointed
     * m_stateStamp - time movement started.  1Hz tic
     * m_orbiting - 0=no orbit, >0=in orbit, 1=at distance, 2=too close , 3=too far, 4=way too close, 5=way too far
     * m_orbitRadTic - rad/sec in current orbit.  set by Orbit() (~2090)
     * m_maxOrbitSpeedFraction - calculated max speed to maintain commanded orbit distance.  set in Orbit() but not used here yet
     *
     *   our target variables
     * Tr = target radius
     * Tp = target position  (updated for movement, if applicable)
     * Tv = target velocity
     * Th = target heading   (updated for movement, if applicable)
     * Tm = target mass
     *
     * centers = distance between object and target centers
     * edges = distance between object and target closest edges (counting for radius)
     *
     */

    /** @todo  will have to set/reset orbit time once actual orbit is started for proper radian setting */
    // get current times
    //uint32 timeStamp = sEntityList.GetStamp() - m_stateStamp;
    float timeStamp((GetTimeMSeconds() - m_moveTime) * 0.001f);
    float Tr = m_targetEntity.second->GetRadius();
    //float Tm = m_targetEntity.second->GetSelf()->GetAttribute(AttrMass).get_float();
    GPoint Tp(m_targetEntity.second->GetPosition());

    // current and edges are used to determine ship's orbit distance, and adjust position accordingly
    double centers(m_position.distance(Tp));
    double edges(centers - m_radius - Tr);
    if (is_log_enabled(DESTINY__ORBIT_TRACE))
        _log(DESTINY__ORBIT_TRACE, "1 - %s(%u): timeStamp:%.3f, centers:%.2f, edges:%.2f, target:%lli, follow:%u", \
            mySE->GetName(), mySE->GetID(), timeStamp, centers, edges, m_targetDistance, m_followDistance);

    // distances checks for orbit calculations
    GPoint mPos(NULL_ORIGIN);
    float mPosAdj(0.0f);
    // check distances for this tic
    if ((edges / 2) > m_followDistance) {
        if (m_orbiting == Destiny::Ball::Orbit::TooFar) {
            MoveObject();
            return;
        }
        // too far to realistically orbit.
        m_orbiting = Destiny::Ball::Orbit::TooFar;
        // TODO: update this to determine orbit and set heading/target to smoothly go from turn into orbit trajectory
        // set point to side of target (based on current position), to avoid near-zero angular velocity
        double radTarg = atan2(Tp.z - m_position.z, Tp.x - m_position.x);  // rad from '0' to target
        radTarg += atan2(m_followDistance, edges);  // rad from 'distance line' to target 'offset'
        mPos.x = m_followDistance * cos(radTarg);
        mPos.z = m_followDistance * sin(radTarg);
        if (Tp.y > m_position.y) { // target is above us.  set point below target using calculated distance
            mPos.y = Tp.y - m_position.y;
        } else { // opposite of above
            mPos.y = m_position.y - Tp.y;
        }
        m_targetPoint = Tp + mPos;
        GVector heading(m_position, m_targetPoint);
        heading.normalize();
        m_shipHeading = heading;    // this sets object velocity using speed
        _log(DESTINY__ORBIT_TRACE, "2 - way too far - rads:%.3f, heading: %.3f, %.3f, %.3f", \
                radTarg, m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);
        MoveObject();
        return;
    } else if ( (centers + m_targetDistance / 3) < m_followDistance) {
        if (m_orbiting == Destiny::Ball::Orbit::TooClose) {
            MoveObject();
            return;
        }
        // to close to realistically orbit.  move away from target
        m_orbiting = Destiny::Ball::Orbit::TooClose;
        // set point to side of target (based on current position), to avoid near-zero angular velocity
        double radTarg = atan2(Tp.z - m_position.z, Tp.x - m_position.x);  // rad from '0' to target
        //radTarg += atan2(m_followDistance, edges);  // rad from 'distance line' to target 'offset'
        mPos.x = m_followDistance * cos(radTarg);
        mPos.z = m_followDistance * sin(radTarg);
        if (Tp.y > m_position.y) {  // target is above us.  set point below target using calculated distance
            mPos.y = Tp.y - m_position.y;
        } else { // opposite of above
            mPos.y = m_position.y - Tp.y;
        }
        m_targetPoint = Tp + mPos;
        GVector heading(m_position, m_targetPoint);
        heading.normalize();
        m_shipHeading = heading;    // this sets object velocity using speed
        _log(DESTINY__ORBIT_TRACE, "2 - way too close - rads:%.3f, heading: %.3f, %.3f, %.3f", \
                radTarg, m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);
        MoveObject();
        return;
    } else if ((edges - m_targetDistance / 4) > m_followDistance) {
        m_orbiting = Destiny::Ball::Orbit::Far;
        // fudge distance for a smaller orbit
        // modify this based on calculated distance
        mPosAdj = -m_followDistance / 25;
        _log(DESTINY__ORBIT_TRACE, "2 - too far");
    } else if (centers < m_followDistance) {
        m_orbiting = Destiny::Ball::Orbit::Close;
        // fudge distance for larger orbit
        // modify this based on calculated distance
        mPosAdj = m_followDistance / 25;
        _log(DESTINY__ORBIT_TRACE, "2 - too close");
    } else {
        m_orbiting = Destiny::Ball::Orbit::Orbiting;
        _log(DESTINY__ORBIT_TRACE, "2 - within tolerance");
    }

    #define LogMacro(v) _log(DESTINY__ORBIT_TRACE, "m - " #v ": (%.3f, %.3f, %.3f)   len=%.3f", v.x, v.y, v.z, v.length())

/** @todo  also look into https://friendly-splash.space/autoorbit-mechanics/ which defines orbit as solution to quadratic equation */
    // new orbit code
    float radius = m_followDistance + mPosAdj;// fudge a bit as using targetDistance is a hair too close
    // angle around y axis (from +x) - horizontal movement  - ccw from +x using ships orbit in rad/tic
    float theta = EvE::Trig::Pi2 - EvE::Trig::Deg2Rad(360) - (m_orbitRadTic * timeStamp);
    // angle around xz axis (from 0) - vertical movement
    //GVector target(m_position, Tp);
    //LogMacro(target);
    //float hyp = sqrt(pow(target.z, 2) + pow(target.x, 2));
    float inclination = 45; //atan(hyp / target.y);
    // fractional value of orbit period (0 < x < 1)
    float period = fmod(timeStamp, m_orbitTime) / m_orbitTime;
    // calculate a pendulum value here to adjust elevation (+/-y) where +x is 1, 0x is 0, -x is -1
    float c = cos(EvE::Trig::Deg2Rad(360 * period));
    // get elevation modifier based on orbit period
    float phi = EvE::Trig::Deg2Rad(inclination * c);
    // set xz plane modifier from elevation
    float s = sin(EvE::Trig::Deg2Rad(360 * period));
    float mu = EvE::Trig::Deg2Rad(inclination * s);
    // here we will adjust orbit plane by adding OrbitRotation angle to theta
    // calculate position
    mPos.x = radius /* mu */* cos( theta );
    mPos.z = radius /* mu */* sin( theta );
    mPos.y = radius * phi;
    _log(DESTINY__ORBIT_TRACE, "4 - theta:%.5f, phi:%.3f, mu:%.2f period:%.5f, radius:%.3f, inc:%.5f", theta,phi,mu,period,radius,inclination);
    LogMacro(mPos);
    // apply origin to our calculated position
    mPos += Tp;
    // set position for this tic
    m_position = mPos;
    mySE->SetPosition(m_position);

    // set heading for this tic
    GPoint mPosNext(NULL_ORIGIN);
    theta += m_orbitRadTic;
    period = fmod(timeStamp + 1, m_orbitTime) / m_orbitTime;
    c = cos(EvE::Trig::Deg2Rad(360 * period));
    phi = EvE::Trig::Deg2Rad(inclination * c);
    mPosNext.x = radius * cos( theta );
    mPosNext.z = radius * sin( theta );
    mPosNext.y = radius * phi;
    LogMacro(mPosNext);
    // determine where our target should be next tic, and figure that into our heading calculation
    float Tv = (m_targetEntity.second->DestinyMgr() != nullptr ? m_targetEntity.second->DestinyMgr()->GetSpeed() : 0);
    GVector Th(m_targetEntity.second->DestinyMgr() != nullptr ? m_targetEntity.second->DestinyMgr()->GetHeading() : NULL_ORIGIN_V);
    Tp += (Tv * Th); // use Tv*Th and add to position to account for target movement.  Tv for non-moving targets return 0.
    mPosNext += Tp;
    GVector heading(m_position, mPosNext);
    heading.normalize();
    m_shipHeading = heading;
    m_targetPoint = m_position + (m_shipHeading * 1.0e16);
    LogMacro( heading );

    double curSpeed(m_maxSpeed * m_activeSpeedFraction * m_maxOrbitSpeedFraction);
    if (is_log_enabled(DESTINY__ORBIT_TRACE))
        _log(DESTINY__ORBIT_TRACE, "5(%u) - orbiting at %.2f. timestamp:%.3f, speed:%.2f", \
            m_orbiting, m_position.distance(Tp), timeStamp, curSpeed);

    MoveObject();
}

GPoint DestinyManager::ComputePosition(double curRad) {
    /*
     *   orbital definitions for EVEmu:
     * node line = ascending node
     * ascending node = line of intersection between orbit plane and reference plane, on the 'upward' side
     * periapsis = closest point of orbit to target point
     * apoapsis = farthest point of orbit to target point
     * ecliptic = plane of orbit
     * reference direction = line on reference plane inline with periapsis
     *
     *   primary orbital elements:
     * Y = reference direction (vector on reference plane that lines up with periapsis)
     * i = inclination to the positive ecliptic at node line
     * a = semi-major axis, or mean distance to target.  this will adjust ship's orbit based on distance
     * e = eccentricity (0=circle, 0-1=ellipse, 1=parabola)
     * w = argument of periapsis, angle from the node line to the periapsis
     * N = longitude of the ascending node (from Y, ccw to node line)
     * NOTE: w + N = 360*
     * M = mean anomaly, radians between our current position and periapsis.  increases uniformly with time from 0 to 2pi (360_deg)
     *
     *   calculated orbital elements:
     * L  = M + p   = mean longitude, measure of how far around its orbit a body has progressed since passing the argument of periapsis (w)
     * P  = orbital period,  time in seconds to complete one orbit (assuming all other variables remain constant)
     * T  = Epoch_of_M - (M(deg)/360_deg) / P  = time of periapsis
     * v  = true anomaly, position of the orbiting body along the orbit at a specific time, measured from w
     * E  = eccentric anomaly, angle from target side of center point of line qQ, at which we are located
     *
     * Under ideal conditions of a perfectly spherical central body and zero perturbations,
     *    all orbital elements except the mean anomaly (M) are constants.
     *
     * As we're using circular orbits, the reference direction (Y) will be +x
     *
     */

    float timeStamp((GetTimeMSeconds() - m_moveTime) * 0.001f);
    GPoint Tp(m_targetEntity.second->GetPosition());
    //i = inclination to the positive ecliptic (plane of our orbit) at node line
    double adj = sqrt(pow(m_position.x - Tp.x, 2) * pow(m_position.z - Tp.z, 2));
    double opp = m_position.y - Tp.y;
    double i = atan2(opp, adj);

    /*  not needed yet, but here just in case...
    // to determine orbiters position relative to target, use RA and dec (from their point of view, with vernal equinox being their heading)
    // calculating right ascension (RA)
    double A = cos(w) * cos(N) - sin(w) * sin(N) * cos(i);
    double B = cos(cos(w) * cos(N) + sin(w) * sin(N) * cos(i)) - sin(sin(w) * sin(i));
    double RA = atan2(B, A);
    // calculating declination (dec)
    double C = sin(cos(w) * cos(N) + sin(w) * sin(N) * cos(i)) + cos(sin(w) * sin(i));
    double dec = asin(C);
    */

    GPoint mPos(NULL_ORIGIN);
    // fig 8 on nw of targ sphere
    float radius = m_targetDistance + (m_radius *2); // fudge a bit as using targetDistance is a hair too close
    // angle around y axis (from +x) - horizontal movement  - cw from +x using ships orbit in rad/tic
    float theta = m_orbitRadTic * timeStamp;
    // angle around xz axis (from 0) - vertical movement
    GVector target(m_position, Tp);
    LogMacro(target);
    float hyp = sqrt(pow(target.z, 2) + pow(target.x, 2));
    float inclination = 45; //atan(hyp / target.y);
    // fractional value of orbit period (0 < x < 1)
    float period = fmod(timeStamp, m_orbitTime) / m_orbitTime;
    // calculate a pendulum value here to adjust elevation (+/-y) where +x is 1, 0x is 0, -x is -1
    float c = cos(EvE::Trig::Deg2Rad(360 * period));
    // get elevation modifier based on orbit period
    float phi = EvE::Trig::Deg2Rad(inclination * c);
    // set xz plane modifier from elevation
    //float s = sin(EvE::Trig::Deg2Rad(360 * period));
    //float mu = EvE::Trig::Deg2Rad(inclination * s);
    // here we will adjust orbit plane by adding OrbitRotation angle to theta
    // calculate position using trig.
    mPos.x = radius /* mu */* cos( theta );
    mPos.z = radius /* mu */* sin( theta );
    mPos.y = radius * phi;

    //_log(DESTINY__ORBIT_TRACE, "Destiny::ComputePosition() - a:%.5f, i:%.5f, v:%.5f, N:%.5f, M:%.5f, w:%.5f, e:%.5f, E:%f, P:%f", a,i,v,N,M,w,e,E,P);
    //_log(DESTINY__ORBIT_TRACE, "Destiny::ComputePosition() - mPos(%.3f, %.3f, %.3f) - radius check %.3f, q:%.3f", mPos.x, mPos.y, mPos.z, r,q);

    // position test
    if (mPos.isNaN()) {
        _log(DESTINY__ERROR, "mPos calculated as NaN.  Stopping Orbit.");
        Stop();
        return NULL_ORIGIN;
    }
    if (mPos.isInf()) {
        _log(DESTINY__ERROR, "mPos calculated as inf.  Stopping Orbit.");
        Stop();
        return NULL_ORIGIN;
    }
    // get new position as reference to target
    return mPos;
}

void DestinyManager::ClearOrbit() {
    sLog.Cyan("DM", "%s calling ClearOrbit()", mySE->GetName());
    m_orbiting = Destiny::Ball::Orbit::None;
    m_orbitTime = 0.0f;
    m_orbitRadTic = 0.0f;
    m_targetDistance = 0;
    m_followDistance = 0;
    m_maxOrbitSpeedFraction = 1.0f;
}

void DestinyManager::InitWarp() {
    // reset sub-warp move variables for warping
    if (m_orbiting)
        ClearOrbit();
    if (m_turning)
        ClearTurn();

    m_accel = false;
    m_decel = false;
    m_posHack = false;
    m_timeFraction = 0.0f;
    // these will be reset with current values in WarpStop()
    m_prevSpeed = 0.0f;
    m_prevSpeedFraction = 0.0f;
    m_activeSpeedFraction = 0.0f;

    // warp time and distance math
    //   allan 1Nov14 - 14Nov14
    //  rewrite 3jan15  to use distance instead of time for warping.  more accurate now, and covers ALL distances.
    //  calculation and implementation update   9Jan15      accuracy is within 1000m
    //  major destiny movement update/rewrite - allan Feb23  (wip)
	//  warp update complete.  MOE ~1-2m, scaling with ship size  -allan 7Mar23

    // ONE_AU_IN_METERS = 149597870700

    /*  my research into warp formulas, i have used these sites, with a few excerpts and ideas from each...
     *     https://wiki.eveonline.com/en/wiki/Acceleration
     *     http://oldforums.eveonline.com/?a=topic&threadID=1251486
     *     http://gaming.stackexchange.com/questions/115271/how-does-one-calculate-a-ships-agility
     *     http://eve-search.com/thread/1514884-0
     *     http://www.eve-search.com/thread/478431-0/page/1
     */

    //init warp:
    if (is_log_enabled(DESTINY__WARP_TRACE))
        _log(DESTINY__WARP_TRACE, "Destiny::InitWarp(): %s(%u) is initializing warp.", mySE->GetName(), mySE->GetID());

    /*  this is from http://community.eveonline.com/news/dev-blogs/warp-drive-active/
	  NOTE:  while this may be accurate now, it was not in crucible
     * x = e^(k*t)
     * v = k*e^(k*t)
     *
     * x = distance in meters
     * t = time in seconds
     * v = speed in m/s
     * k = 3 for accel, 1 for decel
     *
     * this gives distances as functions of time.
     */

    /* this is my version of how warp should be timed and followed by the server.
     * checks here for distance < warp speed and adjusts accel/decel times accordingly
     *
     *   accel/decel are logarithmic per ccp (see above).
     */

//  150km - 15s, 1mkm - 23s, 1au - 29s
    bool cruise(true);
    float accelTime(7.5f), decelTime(7.5f);	// these are minimums
    float cruiseTime(0.0f);
    int64 accelDistance(0), cruiseDistance(0), decelDistance(0);
    int64 warpSpeedInMeters(m_shipWarpSpeed * ONE_AU_IN_METERS);
    // set times and distances based on target distance
    if (m_targetDistance < warpSpeedInMeters) {
        //  short warp....no cruise
        // accel/decel are 7.5s min  (https://oldforums.eveonline.com/?a=topic&threadID=1514884)
        cruise = false;
        // accel = 1/3 decel
        accelDistance = (m_targetDistance / 3);
        decelDistance = (m_targetDistance - accelDistance);
        warpSpeedInMeters = accelDistance;
    } else {
        // accel/decel are 15s max
        accelDistance = ONE_AU_IN_METERS;
        decelDistance = warpSpeedInMeters / 2;
        cruiseDistance = (m_targetDistance - accelDistance - decelDistance);
        cruiseTime = (cruiseDistance / warpSpeedInMeters);
    }

    decelTime = log(decelDistance / 3);
    accelTime = log(accelDistance / 3) / 3;

    //  set total warp time based on above math.
    float warpTime(accelTime + decelTime + cruiseTime);
    //  set deceltime for time check in WarpDecel()
    m_decelTime = round(accelTime) + round(cruiseTime);

    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        _log(DESTINY__WARP_TRACE, "Destiny::InitWarp():Calculate - %s(%u): Warp will accelerate for %.0fs, cruise for %.3f, then decelerate for %.0fs, with total time of %.1fs, and warp speed of %lli m/s.", \
            mySE->GetName(), mySE->GetID(), accelTime, cruiseTime, decelTime, warpTime, warpSpeedInMeters);
        _log(DESTINY__WARP_TRACE, "Destiny::InitWarp():Calculate - %s(%u): Accel distance is %llim. Cruise distance is %llim.  Decel distance is %llim.  Heading is %.4f,%.4f,%.4f.", \
        mySE->GetName(), mySE->GetID(), accelDistance, cruiseDistance, decelDistance, m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);
        _log(DESTINY__WARP_TRACE, "Destiny::InitWarp():Calculate - %s(%u): We will exit warp at %.2f,%.2f,%.2f at a distance of %lu AU (%lum).", \
            mySE->GetName(), mySE->GetID(), m_targetPoint.x, m_targetPoint.y, m_targetPoint.z, m_targetDistance / ONE_AU_IN_METERS, m_targetDistance);
        GPoint destination = m_position + (m_shipHeading * m_targetDistance);
        GVector diff(m_targetPoint, destination);
        _log(DESTINY__WARP_TRACE, "Destiny::InitWarp():Calculate - %s(%u): calculated exit is %.2f,%.2f,%.2f and delta is %.4f.", \
            mySE->GetName(), mySE->GetID(), destination.x, destination.y, destination.z, diff.length());
    }

    SafeDelete(m_warpState);
    m_warpState = new WarpState(m_stateStamp, m_targetDistance, warpSpeedInMeters, accelDistance, cruiseDistance,
                        decelDistance, warpTime, true, false, false);

    // check for player warp
    if (mySE->HasPilot()) {
        //turn off non warp-safe modules
        mySE->GetShipSE()->Warp();
        //drain cap
        mySE->GetSelf()->SetAttribute(AttrCapacitorCharge, m_warpCapacitorNeed);
    }

    //clear targets
    mySE->TargetMgr()->ClearAllTargets();
    //mySE->TargetMgr()->OnTarget(nullptr, TargMgr::Mode::Clear, TargMgr::Msg::WarpingOut);

    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;

    // reset move times
    m_stateStamp = sEntityList.GetStamp();
    m_moveTime = GetTimeMSeconds();

    WarpAccel(0);
}

void DestinyManager::WarpAccel(uint16 sec_into_warp) {
    /* For acceleration, k = 3.
     * distance = e^(k*s)
     * speed = k*e^(k*s)
     */
    int64 currentDistance = exp(3 * sec_into_warp);
    m_targetDistance -= currentDistance;
    int64 currentShipSpeed = (3 * currentDistance);

    if (is_log_enabled(DESTINY__WARP_TRACE))
        _log(DESTINY__WARP_TRACE, "Destiny::WarpAccel(): %s(%u) - Warp Accelerating(%us): velocity %lli m/s with %lli m left to go. Current distance %lli from origin.", \
            mySE->GetName(), mySE->GetID(), sec_into_warp, currentShipSpeed, m_targetDistance, currentDistance);

    if ((currentShipSpeed >= m_warpState->warpSpeed) or (currentDistance >= m_warpState->accelDist)) {
        m_warpState->accel = false;
        if (m_warpState->cruiseDist > 0) {
            m_warpState->cruise = true;
        } else {
            m_warpState->decel = true;
        }
    }

    if (mySE->SysBubble() != nullptr)
        if (currentDistance > BUBBLE_RADIUS_METERS)
            if (mySE->SysBubble() != m_targBubble) {
                if (is_log_enabled(DESTINY__WARP_TRACE))
                    _log(DESTINY__WARP_TRACE, "Destiny::WarpAccel(): %s(%u) is being removed from bubble %u.",\
                            mySE->GetName(), mySE->GetID(), mySE->SysBubble()->GetID());
                mySE->SysBubble()->Remove(mySE);
            }

    WarpUpdate(currentShipSpeed);
}

void DestinyManager::WarpCruise(uint16 sec_into_warp) {
    // in cruise....calculate distance to update position data.
    m_targetDistance -= m_warpState->warpSpeed;
    int64 currentDistance = (m_warpState->total_distance - m_targetDistance);

    if ((m_targetDistance - m_warpState->warpSpeed) < m_warpState->decelDist) {
        m_warpState->cruise = false;
        m_warpState->decel = true;
        //m_targetDistance = m_warpState->decelDist;
    }

    if (is_log_enabled(DESTINY__WARP_TRACE))
        _log(DESTINY__WARP_TRACE, "Destiny::WarpCruise(%s): %s(%u) - Warp Crusing(%us): velocity %lli m/s. with %lli m left to go.", \
                m_warpState->cruise ? "true":"false", mySE->GetName(), mySE->GetID(), sec_into_warp, \
                m_warpState->warpSpeed, m_targetDistance);

    WarpUpdate(m_warpState->warpSpeed);
}

void DestinyManager::WarpDecel(uint16 sec_into_warp) {
    /* For deceleration, k = -1.
     * distance = e^(k*s)
     * speed = -k*e^(k*s)
     */
    uint8 decelTime = (sec_into_warp - m_decelTime);
    m_targetDistance = (std::exp(-decelTime) * m_warpState->decelDist);
    int64 currentDistance = (m_warpState->total_distance - m_targetDistance);
    int64 currentShipSpeed = (m_warpState->warpSpeed * exp(-decelTime));

    if (is_log_enabled(DESTINY__WARP_TRACE))
        _log(DESTINY__WARP_TRACE, "Destiny::WarpDecel(): %s(%u) - Warp Decelerating(%us/%us): velocity %lli m/s with %lli m left to go.", \
                mySE->GetName(), mySE->GetID(), decelTime, sec_into_warp, currentShipSpeed, m_targetDistance);

    if (currentShipSpeed <= m_speedToLeaveWarp) {
        WarpStop(m_speedToLeaveWarp);
        return;
    }

    WarpUpdate(currentShipSpeed);

    if (mySE->SysBubble() == nullptr)
        if (m_targetDistance < BUBBLE_RADIUS_METERS) {
            if (is_log_enabled(DESTINY__WARP_TRACE))
                _log(DESTINY__WARP_TRACE, "Destiny::WarpUpdate()  %s(%u): Ship at %.2f,%.2f,%.2f is calling Add() for bubble %u.", \
                        mySE->GetName(), mySE->GetID(), m_position.x, m_position.y, m_position.z, m_targBubble->GetID());
            m_targBubble->Add(mySE);
        }
}

void DestinyManager::WarpUpdate(int64 currentShipSpeed) {
    //  track position and velocity for all stages.
    m_velocity = (m_shipHeading * currentShipSpeed);
    m_position += (m_targetPoint - (m_shipHeading * m_targetDistance));

    if (is_log_enabled(DESTINY__WARP_TRACE))
        _log(DESTINY__WARP_TRACE, "Destiny::WarpUpdate()  %s(%u): Ship is %f from center of target bubble %u.", \
                mySE->GetName(), mySE->GetID(), m_targBubble->GetCenter().distance(m_position), m_targBubble->GetID());

    mySE->SetPosition(m_position);
}

void DestinyManager::WarpStop(int64 currentShipSpeed) {
    // targPoint is where we exit warp.
    m_velocity = (m_shipHeading * currentShipSpeed);
    m_position = m_targetPoint;
    SetPosition(m_position, true);

    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        _log(DESTINY__WARP_TRACE, "Destiny::WarpStop(): %s(%u) - Warp complete. Exit velocity %lli m/s with %lli m left to go.", \
                mySE->GetName(), mySE->GetID(), currentShipSpeed, m_targetDistance);
        _log(DESTINY__WARP_TRACE, "Destiny::WarpStop(): %s(%u): Ship currently at %.2f,%.2f,%.2f.", \
                mySE->GetName(), mySE->GetID(), m_position.x, m_position.y, m_position.z);
    }

    // reset asf/ps so call to SSF will set decel properly
    m_prevSpeed = currentShipSpeed;
    m_activeSpeedFraction = currentShipSpeed / m_maxSpeed;

    m_targetPoint += (m_velocity * m_agility);
    if (is_log_enabled(DESTINY__WARP_DEBUG)) {
        // drop marker for decel point
        std::string str = "Warp Decel Point - ";
        str += mySE->GetName();
        MarkPoint(m_targetPoint, str, str);
    }

    // SetSpeedFraction() checks for m_state = Warp and warpstate != null to set decel variables correctly with warp decel.
    //   have to call this BEFORE deleting or reseting m_state or WarpState.
    SetSpeedFraction(0.0f);
    SafeDelete(m_warpState);

    // reset move stamps
    m_stateStamp = sEntityList.GetStamp();
    m_moveTime = GetTimeMSeconds();

    if ((mySE->IsNPCSE()) and (mySE->GetNPCSE()->GetAIMgr() != nullptr))
        mySE->GetNPCSE()->GetAIMgr()->WarpOutComplete();

    // reset warp cap need
    if (mySE->GetSelf()->HasAttribute(AttrWarpCapacitorNeed)) {
        m_warpCapacitorNeed = mySE->GetSelf()->GetAttribute(AttrWarpCapacitorNeed).get_double() * 2; //modified
    } else {
        m_warpCapacitorNeed = 0.000000108911;   // arbitrary
    }


    /*  this isnt used yet, but will be needed once bumping is implemented...
    // reset bump checks
    SetBallMassive sbmassive;
        sbmassive.entityID = mySE->GetID();
        sbmassive.is_massive = sConfig.cosmic.BumpEnabled;
    PyTuple *up = sbmassive.Encode();
    SendSingleDestinyUpdate(&up);
    //PyDecRef(up);
    */
}

//called whenever an entity is going away and can no longer be used as a target
void DestinyManager::EntityRemoved(SystemEntity *pSE) {
    if (m_targetEntity.second == pSE) {
        m_targetEntity.first = 0;
        m_targetEntity.second = nullptr;

        switch(m_ballMode) {
            case Destiny::Ball::Mode::FOLLOW: {
                _log(DESTINY__DEBUG, "%u: Our target entity has gone away. Stopping ship.", mySE->GetID());
                Stop();
            } break;
            case Destiny::Ball::Mode::ORBIT: {
				// we were orbiting this target, reset movement to void orbit
                _log(DESTINY__DEBUG, "%u: Our target entity has gone away. Continue on current tangent of orbit.", mySE->GetID());
                GotoDirection(m_shipHeading);
            } break;
			// no default
        }
    }
}

bool DestinyManager::IsTargetInvalid()
{
    /** @todo  this needs a good lookover */
    if (mySE->SystemMgr()->GetSE(m_targetEntity.first) == nullptr) {
        // Our target was removed
        Stop();
        return true;
    }
    if (!m_targetEntity.second->IsDynamicEntity())
        return false;
    if (m_targetEntity.second->HasPilot()) {
        if (m_targetEntity.second->GetPilot()->IsDocked()) {  // Our target docked, so STOP
            //mySE->TargetMgr()->ClearTarget(m_targetEntity.second);
            Stop();
            return true;
        }
        // also check for jump, pos field, more?
    }
    if (m_targetEntity.second->DestinyMgr()->IsWarping()) { // The target is warping
        //mySE->TargetMgr()->ClearTarget(m_targetEntity.second);
        Stop();
        return true;
    }
    return false;
}

void DestinyManager::UpdateSpeedFraction(float speedPct/*0*/) {
    // some yahoo decided to change speed while turning....gee, thanks.
    //this is called from Beyonce.CmdSetSpeedFraction() but only while turning (hard-coded)
    // ok, so, do shit here to keep move vars sane
	//  i dont know what that means yet...
}

// Basic Movement Calls:
void DestinyManager::BeginMovement() {
    // common movement for all types
    if (!m_hasSentShipUpdates) {
        // error fix for setting ship movement variables before ship is in bubble (cannot BubbleCast)
        std::vector<PyTuple*> updates;
        SetBallAgility sbagility;
            sbagility.entityID =  mySE->GetID();
            sbagility.agility = mySE->GetSelf()->GetAttribute(AttrInertiaMod).get_double();
        updates.push_back(sbagility.Encode());
        SetBallMassive sbmassive;
            sbmassive.entityID = mySE->GetID();
            sbmassive.is_massive = sConfig.cosmic.BumpEnabled;
        updates.push_back(sbmassive.Encode());
        SetBallMass sbmass;
            sbmass.entityID = mySE->GetID();
            sbmass.mass = mySE->GetSelf()->GetAttribute(AttrMass).get_double();
        updates.push_back(sbmass.Encode());
        SendDestinyUpdates(updates); //consumed
        m_hasSentShipUpdates = true;
    }

    UnCloak();

    m_stop = false;

    // reset move stamps
    m_stateStamp = sEntityList.GetStamp();
    m_moveTime = GetTimeMSeconds();

    // if ship is not moving, set usf for movement
    if (m_userSpeedFraction < 0.11)
        m_userSpeedFraction = 1.0f;

    SetSpeedFraction(m_userSpeedFraction, true);
}

void DestinyManager::Follow(SystemEntity* pSE, int32 distance) {
    //called from client as 'CmdFollowBall'
    //  also used by 'Approach'
    if ((m_ballMode == Destiny::Ball::Mode::FOLLOW)
    and (m_targetEntity.second == pSE)
    and (m_followDistance == distance)
    and (m_userSpeedFraction))
        return;

    //reset orbit vars
    if (m_orbiting)
        ClearOrbit();
    // reset turn vars
    if (m_turning)
        ClearTurn();

    m_ballMode = Destiny::Ball::Mode::FOLLOW;
    m_targetPoint = pSE->GetPosition();
    m_targetEntity.first = pSE->GetID();
    m_targetEntity.second = pSE;
    m_followDistance = distance;

    // this makes ship approach station dock elevation (y), instead of approaching to stations "center point" position (where icon is)
    if (pSE->IsStationSE())
        m_targetPoint.y = stDataMgr.GetDockPosY(pSE->GetID());

    if (m_autoPilot) {
        // fudge dist a bit for ap
        distance += mySE->GetRadius() + pSE->GetRadius();
    }

    GVector targHeading(m_position, m_targetPoint);
    targHeading.normalize();
    m_targetHeading = targHeading;

    if (IsTurn()) {
        InitTurn();
    } else {
        BeginMovement();
    }

    CmdFollowBall du;
        du.entityID = mySE->GetID();
        du.targetID = pSE->GetID();
        du.range = distance;
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);
    //PyDecRef(up);
}

void DestinyManager::AlignTo(SystemEntity* pSE) {
    // should this Stop() once alignment has been achieved?  i'd say yes.  config option?
    // i originally set it like this, but aknor didnt like it, so it was removed
    m_alignTo = true;
    Follow(pSE, 0);
}

void DestinyManager::GotoDirection(const GPoint& direction) {
    //reset orbit vars
    if (m_orbiting)
        ClearOrbit();
    // reset turn vars as this is most likely a dir change
    if (m_turning)
        ClearTurn();

    m_ballMode = Destiny::Ball::Mode::GOTO;
    m_targetHeading = direction;
    m_targetPoint = direction * 1.0e16;

    if (IsTurn()) {
        InitTurn();
    } else {
        BeginMovement();
    }

    CmdGotoDirection du;
        du.entityID = mySE->GetID();
        du.x = m_targetHeading.x;
        du.y = m_targetHeading.y;
        du.z = m_targetHeading.z;
    PyTuple* up = du.Encode();
    SendSingleDestinyUpdate(&up);
    //PyDecRef(up);
}

void DestinyManager::GotoPoint(const GPoint& point) {
    //reset orbit vars
    if (m_orbiting)
        ClearOrbit();
    // reset turn vars as this is most likely a dir change
    if (m_turning)
        ClearTurn();

    m_ballMode = Destiny::Ball::Mode::GOTO;
    m_targetPoint = point;
    GVector head(m_position, point);
    head.normalize();
    m_targetHeading = head;

    if (IsTurn()) {
        InitTurn();
    } else {
        BeginMovement();
    }

    CmdGotoPoint gtpoint;
        gtpoint.entityID = mySE->GetID();
        gtpoint.x = m_targetPoint.x;
        gtpoint.y = m_targetPoint.y;
        gtpoint.z = m_targetPoint.z;
    PyTuple* up = gtpoint.Encode();
    SendSingleDestinyUpdate(&up);
    //PyDecRef(up);
}

// Fleet warps - all ships will use the warp profile of the slowest ship
void DestinyManager::WarpTo(const GPoint& destPoint, int32 distance/*0*/, bool autoPilot/*false*/, SystemEntity* pSE/*nullptr*/) {
    /* warp order..
     * pick destination -> align/accel -> aura "warp drive active" -> cap drain -> accel
     *      -> enter warp -> warp -> decel -> leave warp -> coast -> stop
     */
    SafeDelete(m_warpState);
    if (m_orbiting)
        ClearOrbit();
    if (m_turning)
        ClearTurn();

    // get target point
    if (destPoint.isZero()) {
        if (pSE != nullptr) {
            m_targetPoint = pSE->GetPosition();
        } else {
            sLog.Error("Destiny::WarpTo()", "DestPoint is zero and pSE is null.");
            throw UserError("WarpDestinationGone");
        }
    } else {
        m_targetPoint = destPoint;
    }

    GVector warp_vector(m_position, m_targetPoint);
    m_targetDistance = warp_vector.length();
    m_targetDistance -= distance;

    if (mySE->HasPilot()) {
        Client *pClient = mySE->GetPilot();
        if (m_targetDistance < minWarpDistance) {
            // send position update
            if (sConfig.debug.PositionHack)
                SetPosition(mySE->GetPosition(), true);
            // warp distance too close.  cancel warp and return
            if (pSE != nullptr) {
                pClient->SendErrorMsg("That is too close for your Warp Drive.  Approaching Target.");
                Follow(pSE, distance);
            } else {
                pClient->SendErrorMsg("That is too close for your Warp Drive.  Stopping Ship.");
                Stop();
            }
            return;
        }

        // check for enough cap to warp.

        /*  capacitor for warp formulas from https://oldforums.eveonline.com/?a=topic&threadID=332116
         *  Energy to warp = warpCapacitorNeed * mass * au * (1 - warp_drive_operation_skill_level * 0.10)
         ** @note:  warpCapacitorNeed is type double...max ive seen is shuttles @ 0.00000134771  and indys @ 0.000000108911
         */

        float currentShipCap = pClient->GetShip()->GetAttribute(AttrCapacitorCharge).get_float();
        double capNeeded = mySE->GetSelf()->GetAttribute(AttrMass).get_float() * m_warpCapacitorNeed * (m_targetDistance / ONE_AU_IN_METERS);
        capNeeded *= (1.0f - (0.1f * pClient->GetChar()->GetSkillLevel(EvESkill::WarpDriveOperation)));

        _log(DESTINY__WARNING, "Warp Cap need for %s(%u) for %llim (%liAU) is %f", \
            mySE->GetName(), mySE->GetID(), m_targetDistance, (m_targetDistance / ONE_AU_IN_METERS), capNeeded);

        // set min cap need to 1.0
        if (capNeeded < 1.0f)
            capNeeded = 1.0;

        //  check if ship has enough capacitor to warp full distance
        if (capNeeded > currentShipCap) {
            // not enough cap.  reset everything based on available cap
            capNeeded = currentShipCap / (mySE->GetSelf()->GetAttribute(AttrMass).get_float() * m_warpCapacitorNeed);
            if (capNeeded > 1.0f) {
                m_targetDistance = capNeeded * ONE_AU_IN_METERS;
                GVector warp_direction(m_position, m_targetPoint);
                // make heading
                warp_direction.normalize();
                GPoint newTarget(m_position + (warp_direction * m_targetDistance));
                m_targetPoint = newTarget;
                m_targBubble = sBubbleMgr.GetBubble(mySE->SystemMgr(), newTarget);
                //WarpingWithAvailablePowerBody
                /** @todo  update all move vars here for new target... */

                GVector warp_vector(m_position, m_targetPoint);
                m_targetDistance = warp_vector.length();
                m_targetDistance -= distance;
            } else {
                // if not enough cap to do min warp. cancel and return
                pClient->SendErrorMsg("You don't have enough capacitor charge to warp.");
                _log(DESTINY__WARNING, "Destiny::InitWarp() - %s(%u): Capacitor needed vs current  %f / %.5f",
                        mySE->GetName(), mySE->GetID(), capNeeded, currentShipCap);

                Stop();
                return;
            }
        } else {
            m_warpCapacitorNeed = currentShipCap - capNeeded;
            // change to heading
            warp_vector.normalize();
            // adjust for stop distance from our travel direction
            warp_vector *= distance;
            // adjust target point by calculated stopping point
            m_targetPoint -= warp_vector;
        }
    }

    //You will always exit warp at a random point, 2,500 meters from your actual exit point - per EveUni
    /*  disabled for testing
     if (mySE->HasPilot())
        m_targetPoint.MakeRandomPointOnSphereLayer(-2500, 2500);
     */

    // reset heading for updated targPoint
    GVector toVec(m_position, m_targetPoint);
    // change to heading
    toVec.normalize();
    // set targ heading
    m_targetHeading = toVec;

    // get targ bubble.  this will create bubble if needed
    m_targBubble = sBubbleMgr.GetBubble(mySE->SystemMgr(), m_targetPoint);

    // npcs have no warp restrictions (yet)
    if (mySE->IsNPCSE() or mySE->IsDroneSE()) {
        // this was seen in logs...no clue why it's zero...
        if (!toVec.isNotZero()) {
            sLog.Warning("NPC Warp", "toVec is zero.  wtf?");
            if (sConfig.server.StackTrace)
                EvE::traceStack();
            Stop();
            return;
        }
        // do drones warp??   they can, yes...with limitations
        if (mySE->IsDroneSE()) {
            // put drone limit checks here
            sLog.Warning("DroneWarp", "Drone %s (from ship %s) warping from bID %u to bID %u", \
                mySE->GetName(), mySE->GetDroneSE()->GetShipSE()->GetName(), \
                mySE->SysBubble()->GetID(), m_targBubble->GetID());
        }

        if (IsTurn()) {
            InitTurn();
        } else {
            BeginMovement();
        }

        // reset ball mode as it was changed in SSF()
        m_ballMode = Destiny::Ball::Mode::WARP;

        // if no players in bubble, this isnt needed...
        if (mySE->SysBubble()->HasPlayers()) {
            std::vector<PyTuple*> updates;
            CmdSetSpeedFraction du;
                du.entityID = mySE->GetID();
                du.fraction = m_userSpeedFraction;
            updates.push_back(du.Encode());
            // send warp update
            CmdWarpTo wt;
                wt.entityID = mySE->GetID();
                wt.dest_x = m_targetPoint.x;
                wt.dest_y = m_targetPoint.y;
                wt.dest_z = m_targetPoint.z;
                wt.distance = distance;
                wt.warpSpeed = GetWarpSpeed();      // warp speed x10
                updates.push_back(wt.Encode());
            SendDestinyUpdates(updates); //consumed

            // send warp gfx
            SendGFX10(mySE->GetID(),"effects.Warping" );
        }
        if (is_log_enabled(NPC__MESSAGE))
            _log(NPC__MESSAGE, "Destiny::WarpTo() NPC %s(%u) to:%u from:%u, m_targetPoint: %.2f,%.2f,%.2f  stop distance: %i  m_targetDistance: %lli",\
                    mySE->GetName(), mySE->GetID(), m_targBubble->GetID(), mySE->SysBubble()->GetID(), \
                    m_targetPoint.x, m_targetPoint.y, m_targetPoint.z, distance, m_targetDistance);
        return;
    }

    _log(DESTINY__TRACE, "Destiny::WarpTo() m_shipHeading: %.7f,%.7f,%.7f.  m_targetHeading: %.7f,%.7f,%.7f", \
            m_shipHeading.x, m_shipHeading.y, m_shipHeading.z, m_targetHeading.x, m_targetHeading.y, m_targetHeading.z);

    /*supercap warp modifiers
     * these will go here, and modify distance, target, and range accordingly
     *
     * AttrWarpAccuracyMaxRange = 1021,
     * AttrWarpAccuracyFactor = 1022,
     * AttrWarpAccuracyFactorMultiplier = 1023,
     * AttrWarpAccuracyMaxRangeMultiplier = 1024,
     * AttrWarpAccuracyFactorPercentage = 1025,
     * AttrWarpAccuracyMaxRangePercentage = 1026,
     */


    /*  TODO PUT CHECK HERE FOR WARP BUBBLES
     *     and other things that affect warp-in point.....when we get to there.
     * AttrWarpBubbleImmune = 1538,
     * AttrWarpBubbleImmuneModifier = 1539,
     *
     *   NOTE:  warp bubble in path (or within 150km of m_targetPoint) will change m_targetDistance and m_targetPoint
     *   however, this does NOT affect original calculations for energy needed, etc...
     */
     /** @todo  does this apply for ANY bubble along warp route or just end?
      * would be fun to check entire route for bubble...can bm along route to add bubble
      * however, this will take a bit to implement
      */
    if (m_targBubble->HasWarpBubble())
        if (!mySE->GetSelf()->HasAttribute(AttrWarpBubbleImmune)) {
            /*  there is a bubble here and ship isnt immune.
             * at this point, determine where the bubble is
             * then set ship targPoint to random point 2500m(?) from center of bubble
             */
        }

    // found few warp error msgs in client and noted in BeyonceSvc.h
    //  test for and implement here

    if (autoPilot) {
        // make sure this is jumpcloak and not module cloak
        UnCloak();

        // reset move stamps
        m_stateStamp = sEntityList.GetStamp();
        m_moveTime = GetTimeMSeconds();
        SetSpeedFraction(1.0f, true);

        // in some cases, ap is enabled while ship is moving.  check for turn.
        if (IsTurn())
            InitTurn();
    } else {
        // everything else will use code from Turn() and BeginMovement()
        if (IsTurn()) {
            InitTurn();
        } else {
            BeginMovement();
        }
    }

    if (is_log_enabled(DESTINY__WARP_DEBUG)) {
        // make marker at stop point
        std::string str = "Warp Stop Point - ";
        str += mySE->GetName();
        MarkPoint(m_targetPoint, str, str);
    }

    // reset ball mode as it was changed in SSF()
    m_ballMode = Destiny::Ball::Mode::WARP;
    // if ship is not moving, set usf for movement
    if (m_userSpeedFraction < 0.749)
        m_userSpeedFraction = 1.0f;

    //set massive for warp.   self-only per client logs
    SetBallMassive bm;
        bm.entityID = mySE->GetID();
        bm.is_massive = false;       // disable client-side bump checks
    PyTuple *up = bm.Encode();
    SendSingleDestinyUpdate(&up, true);
    //PyDecRef(up);
    // NOTE:  sending ball mass isnt required if is_massive=false

    std::vector<PyTuple*> updates;
    // send warp update
    CmdWarpTo wt;
        wt.entityID = mySE->GetID();
        wt.dest_x = m_targetPoint.x;
        wt.dest_y = m_targetPoint.y;
        wt.dest_z = m_targetPoint.z;
        wt.distance = distance;
        wt.warpSpeed = GetWarpSpeed();      // warp speed x10
        updates.push_back(wt.Encode());
    SendDestinyUpdates(updates); //consumed

    // send warp gfx
    SendGFX10(mySE->GetID(),"effects.Warping" );

    if (is_log_enabled(DESTINY__WARP_TRACE))
        _log(DESTINY__WARP_TRACE, "Destiny::WarpTo() toBubble:%u from:%u, m_targetPoint: %.2f,%.2f,%.2f  stop distance: %i  m_targetDistance: %lli",
             m_targBubble->GetID(), mySE->SysBubble()->GetID(), m_targetPoint.x, m_targetPoint.y, m_targetPoint.z, distance, m_targetDistance);
}

void DestinyManager::InitOrbit(SystemEntity *pSE, uint32 distance/*0*/) {
    if ((m_ballMode == Destiny::Ball::Mode::ORBIT)
    and (m_targetEntity.second == pSE)
    and (m_targetDistance == distance))
        return;

    // if already orbiting, this would be a change of target or distance.  reset and recompute
    if (m_orbiting)
        m_shipHeading = NULL_ORIGIN_V;
	//ClearOrbit(); ??

    /* this InitOrbit() call will, based on ship data, determine the orbit plane, rotation (cw/ccw)
     *  initial heading, actual orbit radius, actual orbit velocity, and some other shit i havent thought about yet.
     *
     * m_targetPoint    - updated in Orbit()
     * m_shipHeading    - updated in Orbit()
     * m_targetEntity   - SE object to orbit
     * m_targetDistance - commanded orbit distance
     * m_followDistance - calculated orbit distance based on mass, velocity, gravity, and other ship variables
     * m_stateStamp (via BeginMovement())
     * speed fractions (usf, tf, asf - via SetSpeedFraction() to begin or alter speed)
     * m_maxOrbitSpeedFraction - calculated max speed to maintain commanded orbit distance.  set in Orbit()
     */
    m_ballMode = Destiny::Ball::Mode::ORBIT;
    m_orbiting = Destiny::Ball::Orbit::Orbiting;
    m_targetEntity.first = pSE->GetID();
    m_targetEntity.second = pSE;
    m_targetPoint = pSE->GetPosition();
    m_targetDistance = distance;
    BeginMovement();

    if (is_log_enabled(DESTINY__ORBIT_TRACE))
        _log(DESTINY__ORBIT_TRACE, "%s(%u) - Ship Data - agility:%.3f, inertiaMod:%.3f, massMkg:%.3f, maxSpeed:%.2f, radius:%.2f", \
        mySE->GetName(), mySE->GetID(), m_agility, //mySE->GetSelf()->GetAttribute(AttrAgility).get_float(),
            mySE->GetSelf()->GetAttribute(AttrInertiaMod).get_double(),
             mySE->GetSelf()->GetAttribute(AttrMass).get_float() * 0.0000001, m_maxShipSpeed, m_radius);

    // Target (orbited object)
    double Tr = pSE->GetRadius();
    double Tm = pSE->GetSelf()->GetAttribute(AttrMass).get_double();
    if (Tm == 0.0)
        Tm = pSE->GetSelf()->type().mass();

    if (is_log_enabled(DESTINY__ORBIT_TRACE))
        _log(DESTINY__ORBIT_TRACE, "%s(%u) - Target Data - mass:%.3f, speed:%.2f, radius:%.2f", \
            mySE->GetName(), mySE->GetID(), Tm, (pSE->DestinyMgr() ? pSE->DestinyMgr()->GetSpeed() : 0 ), Tr);

    // fudge distance to work 'close enough' with all targets...this was trial-n-error
	// note: this is not how his calc's have it.....should be Rc = R0+R1 where R0 is targ radius and R1 is commanded orbit radius
    //double Rc  = ((distance + 150 + m_radius - (pSE->GetRadius() / 12)) * 1.2);
    double Rc  = (distance + m_radius);
    double Rc2 = pow(Rc,2);
    double Vm2 = pow(m_maxShipSpeed, 2);
    double t2  = pow(m_agility, 2);  //mySE->GetSelf()->GetAttribute(AttrAgility).get_double()

    // the following equation is from "Ship Motion in Eve Online" by Scheulagh Santorine, Ph.D
	// Dr. Santorine's work was from Dominion, Dec'09 to Mar'10.  He claims it is still accurate as of late '15
    // to follow real physics, this needs target mass and grav const factored in somehow. (grav const is defined in client, so i assume it's used)
    // orbit radius
    /* r = 1/6 * (6 * cbrt(108t^2*Vm^2 * Rc^2 + 8Rc^6 + 12sqrt(81t^4 * Vm^4 + 12t^2 * Vm^2 * Rc^10))
     * + (24Rc^4 / (108t^2 * Vm^2 * Rc^2 + 8Rc^2 + 12sqrt(81t^4 * Vm^4 * Rc^8 + 12t^2 * Vm^2 * Rc^10)^1/3)) + 12Rc^2) ^0.5
     */
    double one = (108 * t2 * Vm2 * Rc2);
    double two = (12 * t2 * Vm2 * pow(Rc, 10));
    double three = (12 * sqrt(81 * pow(m_agility, 4) * pow(m_maxShipSpeed, 4) + two));
    double four = (6 * cbrt(one + 8 * pow(Rc,6) + three));
    double five =  cbrt( sqrt(three * pow(Rc,8) + two));
    double six = (one + (8 * Rc2) + (12 * five));
    m_followDistance = sqrt(four + (24 * pow(Rc, 4) / six) + 12 * Rc2) * 0.16666666;

    /*
orbit approximatations....
orbit radius in km
rActual = imod*massMkg*velActual^2*10^-3 / velMax^2 - velActual^2
*/
    if (m_followDistance < 1) {
        _log(DESTINY__ERROR, "%s(%u) - FollowDistance is <1.", mySE->GetName(), mySE->GetID());
        throw CustomError("Distance Calculation Error.  Orbit Cancelled.");
    }

    int64 velocity = m_maxShipSpeed * (distance / m_followDistance); // dunno where i got this from but seems to work very well.
    m_maxOrbitSpeedFraction = velocity / m_maxShipSpeed;

    m_orbitTime = (EvE::Trig::Pi2 * m_followDistance) / velocity;
    m_orbitRadTic = EvE::Trig::Pi2 / m_orbitTime;

    if (is_log_enabled(DESTINY__ORBIT_TRACE))
        _log(DESTINY__ORBIT_TRACE, "%s(%u) - Orbit Data - Rc:%.3f, velocity:%lli, osf:%.2f, targetDistance:%lli, followDistance:%u, orbitTime:%.1f, radTic:%.5f", \
                mySE->GetName(), mySE->GetID(), Rc, velocity, m_maxOrbitSpeedFraction, \
                m_targetDistance, m_followDistance, m_orbitTime, m_orbitRadTic);
/*  dont really need this here yet.....maybe not at all.
    double current = m_position.distance(pSE->GetPosition());
    double actual = (current - m_radius - Tr);
    // m_orbiting: -2=way too close  -1=too close, 0=no orbit, 1=at distance 2=too far, 3=way too far
    if ((actual - m_followDistance) > m_followDistance) {
        // too far to engage target.
        m_orbiting = 3;
    } else if (current > m_followDistance) {
        // too far outside orbit.  move closer
        m_orbiting = 2;
    } else if (actual < m_followDistance) {
        // way too close inside orbit.  move away quickly.
        m_orbiting = -2;
    } else if (current < m_followDistance) {
        // too close inside orbit; move away slowly.
        m_orbiting = -1;
    } else {
        // within orbit distance tolerance
        m_orbiting = 1;
    }

    if (m_orbiting > 1) {
        // outside target distance.  set orbit parameters based on current position.

    }
*/
    CmdOrbit du;
        du.entityID = mySE->GetID();
        du.orbitEntityID = pSE->GetID();
        du.distance = (int32)m_targetDistance;
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);
    //PyDecRef(up);
}

// not used
bool DestinyManager::IsAligned(GPoint& targetPoint)
{
    if (m_shipHeading.isZero()) {
        GVector moveVector(m_position, targetPoint);
        moveVector.normalize();
        m_shipHeading = moveVector;
    }
    GVector toVec(m_position, targetPoint);
    toVec.normalize();
    float dot = toVec.dotProduct(m_shipHeading);
    float degrees = EvE::Trig::Rad2Deg(std::acos(dot));
    if (degrees < TURN_ALIGNMENT)
        return true;
    return false;
}

void DestinyManager::Undock(GPoint dir) {
    //set movement direction
    m_targetPoint = dir * 1.0e16;
    m_shipHeading = GVector(dir);
    SetUndockSpeed();
    if (mySE->IsShipSE())
        mySE->GetShipSE()->GetShipItemRef()->SetUndocking(false);
}

void DestinyManager::SetUndockSpeed() {
    //start ship movement @ max velocity for undocking.
    // this simulates being forcefully "ejected" from station (and is currently off)
    m_stop = false;
    m_accel = true;
    m_orbiting = 0;
    m_stateStamp = sEntityList.GetStamp();
    m_changeDelay = true;   // skip a next 2 tics before making change
    m_shipAccelTime = 0.8f;
    m_prevSpeedFraction = 0.0f;
    m_userSpeedFraction = 1.0f;
    m_maxSpeed = m_maxShipSpeed;
    m_velocity = m_shipHeading * m_maxSpeed;
    m_activeSpeedFraction = 1.0f;

    if (m_ballMode == Destiny::Ball::Mode::MISSILE)
        return;

    m_ballMode = Destiny::Ball::Mode::GOTO;
    std::vector<PyTuple*> updates;
    SetBallVelocity bv;
        bv.entityID = mySE->GetID();
        bv.x = m_velocity.x;
        bv.y = m_velocity.y;
        bv.z = m_velocity.z;
    updates.push_back(bv.Encode());
    CmdGotoDirection du;
        du.entityID = mySE->GetID();
        du.x = m_shipHeading.x;
        du.y = m_shipHeading.y;
        du.z = m_shipHeading.z;
    updates.push_back(du.Encode());
    SendDestinyUpdates(updates); //consumed
}

PyResult DestinyManager::AttemptDockOperation() {
    Client *pClient = mySE->GetPilot();
    uint32 stationID = pClient->GetDockStationID();
    SystemEntity *stationSE = mySE->SystemMgr()->GetSE(stationID);

    if ( stationSE == nullptr) {
        codelog(CLIENT__ERROR, "%s: Station %u not found.", pClient->GetName(), stationID);
        pClient->SendErrorMsg("Station Not Found, Docking Aborted.");
        return PyStatic.NewNone();
    }

    //get the station Docking Perimiter
    const GPoint stationPos = stationSE->GetPosition();
    double rangeToStationPerimiter = m_position.distance(stationPos);
    rangeToStationPerimiter -= mySE->GetRadius();
    rangeToStationPerimiter -= stationSE->GetRadius();

    // Verify range to station is within docking perimeter of 2500 meters:
    _log(DESTINY__TRACE, "Destiny::AttemptDockOperation() rangeToStationPerimiter is %.2fm", rangeToStationPerimiter);
    if (rangeToStationPerimiter > 2500.0) {
        // Turn ship and move toward docking point - client will usually call Dock() automatically...sometimes
        Follow(stationSE, 0);
        if (mySE->HasPilot() and mySE->GetPilot()->CanThrow())
            throw UserError("DockingApproach");
    }

    pClient->SetStateTimer(Player::State::Dock, sConfig.world.StationDockDelay * 1000); // default @ 4sec();
    pClient->SetAutoPilot(false);

    return new PyLong(GetFileTimeNow());
}

void DestinyManager::DockingAccepted()
{
    Stop();
    UnCloak();
    Client *pClient = mySE->GetPilot();
    // this would be an error.  only players use this method.
    if (pClient == nullptr)
        return;

    SystemEntity *pSE = mySE->SystemMgr()->GetSE(pClient->GetDockStationID());
    if (pSE == nullptr)
        return;

    const GPoint stationPos = pSE->GetPosition();
    OnDockingAccepted oda;
        oda.ship_x = m_position.x;
        oda.ship_y = m_position.y;
        oda.ship_z = m_position.z;
        oda.station_x = stationPos.x;
        oda.station_y = stationPos.y;
        oda.station_z = stationPos.z;
        oda.stationID = pClient->GetDockStationID();
    PyTuple* ev = oda.Encode();
    pClient->SendNotification("OnDockingAccepted", "charid", &ev);
}

void DestinyManager::SetPosition(const GPoint &pt, bool update /*false*/) {
    _log(DESTINY__TRACE, "Destiny::SetPosition() called by %s(%u)", mySE->GetName(), mySE->GetID());

    if (pt.isZero()) {
        _log(DESTINY__TRACE, "Destiny::SetPosition() - %s(%u) point is zero", mySE->GetName(), mySE->GetID());
        EvE::traceStack();
        // this *should* be systemID...
        m_position = sMapData.GetRandPointOnPlanet(mySE->GetLocationID());
    } else {
        m_position = pt;
    }

    // this sets InventoryItemRef.m_position correctly, which is used for all position references
    mySE->SetPosition(m_position);

    //according to packet sniffs, this is only used for 'Structure' and 'Probe" items.  'update' is for syncing client position data with ours
    if (mySE->IsPOSSE() or mySE->IsProbeSE() or update) {
        SetBallPosition du;
            du.entityID = mySE->GetID();
            du.x = m_position.x;
            du.y = m_position.y;
            du.z = m_position.z;
        PyTuple* up = du.Encode();
        SendSingleDestinyUpdate(&up);
        //PyDecRef(up);
    }
}

// settings for ship, npc and missile max speeds
void DestinyManager::SetMaxVelocity(float maxVelocity)
{
    float maxSpeed = mySE->GetSelf()->GetAttribute(AttrMaxVelocity).get_float();
    /*
    if (mySE->IsMissileSE() or mySE->IsNPCSE()) {
        maxSpeed = mySE->GetSelf()->GetAttribute(AttrMaxVelocity).get_float();
    } else if (mySE->IsShipSE() or mySE->IsDroneSE()) {
        maxSpeed = mySE->GetSelf()->GetAttribute(AttrMaxDirectionalVelocity).get_float();   // this is depreciated.  used as an absolute max speed, accounting for ab/mwd
    } else {
        ; // make error here?
    }
        */
    if (mySE->IsShipSE())
        if (is_log_enabled(DESTINY__TRACE))
            _log(DESTINY__TRACE, "Destiny::SetMaxVelocity() - Ship:%s(%u) Pilot:%s(%u) - AttrMaxDirectionalVelocity is %.1f, maxSpeed is %.1f, update is %.1f", \
                    mySE->GetName(), mySE->GetID(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID(), \
                    mySE->GetSelf()->GetAttribute(AttrMaxDirectionalVelocity).get_float(), maxSpeed, maxVelocity);

    if (maxVelocity > maxSpeed) {
        m_maxShipSpeed = maxSpeed;
    } else {
        m_maxShipSpeed = maxVelocity;
    }
}

void DestinyManager::SpeedBoost(bool deactivate/*false*/)
{
    // only thing to do here is reset ship's speed/agility data
    //  UpdateVelocity will handle the rest,

    // set up initial variables
    InventoryItemRef sRef = mySE->GetSelf();
    if (!sRef->HasAttribute(AttrInertiaMod))    // this should never hit
        sLog.Error("DM::UpdateShipVariables", "%s (%u) does not have an InertiaMod", mySE->GetName(), mySE->GetID());

    double mass = sRef->GetAttribute(AttrMass).get_double();
    double inertiaMod = sRef->GetAttribute(AttrInertiaMod).get_double();
    m_agility = mass * inertiaMod / 1000000;
    mySE->GetSelf()->SetAttribute(AttrAgility, m_agility, false);

    m_alignTime = ceil(1.386294 * m_agility);
    m_turnPct = 1.0f / m_alignTime;
    m_shipMaxAccelTime = (-log(ASF_CHECK) * m_agility);

    //get current ship speed
    m_prevSpeed = m_maxSpeed * m_activeSpeedFraction;

    // verify hull overspeed isnt reached
    m_maxShipSpeed = sRef->GetAttribute(AttrMaxVelocity).get_float();
    if (m_maxShipSpeed > sRef->GetAttribute(AttrMaxDirectionalVelocity).get_float())
        m_maxShipSpeed = sRef->GetAttribute(AttrMaxDirectionalVelocity).get_float();

    // reset ship max commanded speed using updated m_maxShipSpeed
    if (m_decel) {
        // error fix for deactivate while decel (usf=0)
        m_maxSpeed = m_maxShipSpeed;
    } else {
        m_maxSpeed = m_maxShipSpeed * m_userSpeedFraction;
    }

    // check for turn and change vars as needed.
    // this will need a lot of testing after Turn() is working
    if (m_turning) {
        // if turning when propmod (de)activated,  do we have to update turnmap?
        if (m_turnTime < (m_alignTime * 0.5)) {
            // ship is still in decel.
        }
    } else {
        // oh thank God we're not turning....
        // set psf to current speed over new max speed.
        m_prevSpeedFraction = m_prevSpeed / m_maxShipSpeed;     // this may give >1.0
        // update timer.  this will allow speed changes to be timed properly
        m_moveTime = GetTimeMSeconds();
    }

    // send out updated ship data
    std::vector<PyTuple*> updates;
    SetBallAgility sbagility;
        sbagility.entityID =  mySE->GetID();
        sbagility.agility = inertiaMod;
    updates.push_back(sbagility.Encode());
    SetBallMass sbmass;
        sbmass.entityID = mySE->GetID();
        sbmass.mass = sRef->GetAttribute(AttrMass).get_double();
    updates.push_back(sbmass.Encode());
    SetBallSpeed sbms;
        sbms.entityID = mySE->GetID();
        sbms.speed = m_maxShipSpeed;
    updates.push_back(sbms.Encode());
    SendDestinyUpdates(updates); //consumed
    m_hasSentShipUpdates = true;    // just in case, as this is re-sent in BeginMovement()

    // this is just for debug logging
    if (is_log_enabled(DESTINY__MOVE_TRACE)) {
        // ship is currently ...
        if (deactivate) {
            // ... deactivating prop mod
            _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::Deactivate");
        } else if (m_activeSpeedFraction < m_userSpeedFraction) {
            // ....moving and accelerating
            _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(psf!=0 & asf<usf) - accelerating.");
        } else if (m_activeSpeedFraction > m_userSpeedFraction) {
            // ....moving and decelerating
            // - this also hits when prop mod activated while ship is decel
            _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(psf!=0 & asf>usf) - decelerating.");
        } else if (m_activeSpeedFraction) {
            // ....moving and not decelerating (this includes turning)
            _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(0<asf<=usf)");
        } else {
            // ....sitting still - do nothing
            if (m_userSpeedFraction) {
                _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(usf>asf=0) -  sitting still.");
            } else {
                _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(usf=asf=0) -  sitting still.");
            }
        }

        _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost() - nMass: %.5f, nAg: %.5f, tf: %.2f, usf: %.2f, asf: %.3f", \
        mySE->GetSelf()->GetAttribute(AttrMass).get_float(), m_agility, m_timeFraction, m_userSpeedFraction, m_activeSpeedFraction);
        _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost() - pSpeed:%.2f, maxSpeed:%.2f, maxShipSpeed:%.2f, turn:%u", \
                m_prevSpeed, m_maxSpeed, m_maxShipSpeed, m_turnTime);
    }

    // call UpdateVelocity() to reset movement settings
    UpdateVelocity(IsMoving());
}

void DestinyManager::WebbedMe(InventoryItemRef modRef, bool apply/*false*/)
{
    // update mss for web apply/remove
    if (apply) {
        m_maxShipSpeed *= (1 + (modRef->GetAttribute(AttrSpeedFactor).get_float() / 100.0f));
    } else {
        m_maxShipSpeed /= (1 + (modRef->GetAttribute(AttrSpeedFactor).get_float() / 100.0f));
    }

    //get current ship speed
    m_prevSpeed = m_maxSpeed * m_activeSpeedFraction;

    // reset ship max speed using updated m_maxShipSpeed
    m_maxSpeed = m_maxShipSpeed * m_userSpeedFraction;

    // set asf as fraction of current speed over new max speed.   it will apply on next tic (immediate)
    // this may give >1.0 when web applied.
    m_activeSpeedFraction = m_prevSpeed / m_maxShipSpeed;

    // if orbiting, call Orbit() and let code reset the variables
    if (m_orbiting != 0)
        InitOrbit(m_targetEntity.second, m_targetDistance);

    std::vector<PyTuple*> updates;
    SetBallSpeed sbms;
        sbms.entityID = mySE->GetID();
        sbms.speed = m_maxShipSpeed;
        updates.push_back(sbms.Encode());
    SendDestinyUpdates(updates); //consumed
    m_hasSentShipUpdates = true;    // just in case, as this is re-sent in BeginMovement()

    // speed changes are immediate.  do not update timer/usf/psf as that will force accel/decel calcs
    //m_moveTime = GetTimeMSeconds();
    //SetSpeedFraction(m_userSpeedFraction, true);
}

void DestinyManager::UpdateOldShip(ShipSE* pShipSE)
{
    if (pShipSE->IsDead())
        return;
    PyDict* slimPod = new PyDict();
    slimPod->SetItemString("itemID",                new PyInt(pShipSE->GetID()));
    slimPod->SetItemString("typeID",                new PyInt(pShipSE->GetTypeID()));
    slimPod->SetItemString("categoryID",            new PyInt(pShipSE->GetCategoryID()));
    slimPod->SetItemString("ownerID",               new PyInt(pShipSE->GetOwnerID()));
    slimPod->SetItemString("charID",                PyStatic.NewNone());
    slimPod->SetItemString("corpID",                new PyInt(pShipSE->GetCorporationID()));
    slimPod->SetItemString("allianceID",            new PyInt(pShipSE->GetAllianceID()));
    slimPod->SetItemString("warFactionID",          new PyInt(pShipSE->GetWarFactionID()));
    slimPod->SetItemString("bounty",                PyStatic.NewNone());
    slimPod->SetItemString("securityStatus",        PyStatic.NewNone());
    PyTuple* shipData = new PyTuple(2);
    shipData->SetItem(0, new PyLong(pShipSE->GetID()));
    shipData->SetItem(1, new PyObject( "foo.SlimItem", slimPod));
    PyTuple* shipItem = new PyTuple(2);
    shipItem->SetItem(0, new PyString("OnSlimItemChange"));
    shipItem->SetItem(1, shipData);
    SendSingleDestinyUpdate(&shipItem);
    //PyDecRef(shipItem);

    SendBallInteractive(pShipSE->GetShipItemRef(), false);
    m_hasSentShipUpdates = false;
}

void DestinyManager::UpdateNewShip(const ShipItemRef newShipRef) {
    if (m_hasSentShipUpdates)
        return;

    Client* pClient = mySE->GetPilot();
    if (pClient == nullptr)
        return;
    // exactly why do we need this here??
    PyDict* slim = new PyDict();
    slim->SetItemString("name",                     new PyString(newShipRef->itemName()));
    slim->SetItemString("itemID",                   new PyInt(newShipRef->itemID()));
    slim->SetItemString("typeID",                   new PyInt(newShipRef->typeID()));
    slim->SetItemString("ownerID",                  new PyInt(mySE->GetOwnerID()));
    slim->SetItemString("charID",                   new PyInt(pClient->GetCharacterID()));
    slim->SetItemString("corpID",                   IsCorpID(mySE->GetCorporationID()) ? new PyInt(mySE->GetCorporationID()) : PyStatic.NewNone());
    slim->SetItemString("allianceID",               IsAllianceID(mySE->GetAllianceID()) ? new PyInt(mySE->GetAllianceID()) : PyStatic.NewNone());
    slim->SetItemString("warFactionID",             IsFactionID(mySE->GetWarFactionID()) ? new PyInt(mySE->GetWarFactionID()) : PyStatic.NewNone());
    slim->SetItemString("bounty",                   new PyFloat(pClient->GetBounty()));
    slim->SetItemString("securityStatus",           new PyFloat(pClient->GetSecurityRating()));
    if (newShipRef->typeID() == itemTypeCapsule) {
        slim->SetItemString("launcherID",               new PyInt(mySE->GetShipSE()->GetLauncherID()));
        slim->SetItemString("modules",                  new PyList());
    } else {
        slim->SetItemString("categoryID",               new PyInt(newShipRef->categoryID()));
        slim->SetItemString("groupID",                  new PyInt(newShipRef->groupID()));
        slim->SetItemString("modules",                  newShipRef->ShipGetModuleList());
    }

    std::vector<PyTuple*> updates;
    PyTuple* shipData = new PyTuple(2);
    shipData->SetItem(0, new PyLong(newShipRef->itemID()));
    shipData->SetItem(1, new PyObject( "foo.SlimItem", slim));
    PyTuple* shipItem = new PyTuple(2);
    shipItem->SetItem(0, new PyString("OnSlimItemChange"));
    shipItem->SetItem(1, shipData);
    updates.push_back(shipItem);
    SendDestinyUpdates(updates);        // consumed

    UpdateShipVariables();
    SendBallInteractive(newShipRef, true);
}

//  called from Client::ResetAfterPodded(), NPC::NPC(), Concord::Concord(), Drone::Drone(), DestinyManager::UpdateNewShip()
void DestinyManager::UpdateShipVariables()
{
    /* basic InertiaMod
Frigates (incl. CovOps, Inty, AF)       3.1
Destroyers                              3.5
Industrials                             1.0
Cruisers                                0.55 (Elite/Faction 0.65)
Battlecruisers                          1.1
Battleships                             0.155
*/
    /* this sets variables needed for correct movement math.
     *  these attribs are set from ship item when shipSE created.  DO NOT modify anything here
     * this is also called when fleet boosts are updated.
     */
    /** @todo check for movement when fleet boosts are applied and this is called */
    InventoryItemRef sRef = mySE->GetSelf();

    /* The product of Mass and InertiaMod gives the item's Agility
     *  Agility = Mass x InertiaMod / 1000000
     *  Agility is an internal server variable.
     */
    if (!sRef->HasAttribute(AttrInertiaMod))    // this should never hit
        sLog.Error("DM::UpdateShipVariables", "%s (%u) does not have an InertiaMod", mySE->GetName(), mySE->GetID());

    double mass = sRef->GetAttribute(AttrMass).get_double();
    double inertiaMod = sRef->GetAttribute(AttrInertiaMod).get_double();
    m_agility = mass * inertiaMod / 1000000;
    mySE->GetSelf()->SetAttribute(AttrAgility, m_agility, false);

    // this will catch speeds/needs for all ships (player and npc), and is easier to do here.
    if (sRef->HasAttribute(AttrWarpSpeedMultiplier))
        m_shipWarpSpeed = sRef->GetAttribute(AttrWarpSpeedMultiplier).get_double();
    if (sRef->HasAttribute(AttrMaxVelocity))
        m_maxShipSpeed = sRef->GetAttribute(AttrMaxVelocity).get_float();
    if (sRef->HasAttribute(AttrWarpCapacitorNeed)) {
        m_warpCapacitorNeed = sRef->GetAttribute(AttrWarpCapacitorNeed).get_double() * 2; //modified
    } else {
        m_warpCapacitorNeed = 0.000000108911;   // arbitrary
    }

    // i dont think this is right....
    //if (mySE->IsNPCSE() or mySE->IsDroneSE())
    //    m_maxShipSpeed = sRef->GetAttribute(AttrEntityCruiseSpeed).get_float();

    // verify hull overspeed
    if (m_maxShipSpeed > sRef->GetAttribute(AttrMaxDirectionalVelocity).get_float())
        m_maxShipSpeed = sRef->GetAttribute(AttrMaxDirectionalVelocity).get_float();

    /*  per https://forums.eveonline.com/default.aspx?g=posts&m=3912843   post#103
     *
     * Ships will exit warp mode when their current warp speed drops below
     * 75% of sub-warp max speed, or 100m/s, whichever is the lower.
     */
    m_speedToLeaveWarp = m_maxShipSpeed * 0.75f;
    if ((m_speedToLeaveWarp < 100) and (m_maxShipSpeed > 135))      // 75% of 135 is 101.25
        m_speedToLeaveWarp = 100;

    //TimeToWarp = ((ln(2) * m_inertiaMod * m_mass) / 500000);  //18.922
    //TimeToWarp = (ln(0.25) * m_agility);						//18.922
    //m_alignTime = (0.693147 * mass * inertiaMod) / 500000;
    m_alignTime = ceil(1.386294 * m_agility);	// faster than above
    m_turnPct = 1.0f / m_alignTime;
    m_shipMaxAccelTime = (-log(ASF_CHECK) * m_agility);

    m_hasSentShipUpdates = true;

    if (!mySE->HasPilot())
        return;

    if (mySE->GetPilot()->IsInSpace() and (mySE->SysBubble() != nullptr)) {
        std::vector<PyTuple*> updates;
        SetBallAgility sbagility;
            sbagility.entityID =  mySE->GetID();
            sbagility.agility = inertiaMod;
        updates.push_back(sbagility.Encode());
        SetBallMassive sbmassive;
            sbmassive.entityID = mySE->GetID();
            sbmassive.is_massive = sConfig.cosmic.BumpEnabled;
        updates.push_back(sbmassive.Encode());
        SetBallMass sbmass;
            sbmass.entityID = mySE->GetID();
            sbmass.mass = sRef->GetAttribute(AttrMass).get_double();
        updates.push_back(sbmass.Encode());
        SetBallSpeed sbspeed;
            sbspeed.entityID = mySE->GetID();
            sbspeed.speed = m_maxShipSpeed;
        updates.push_back(sbspeed.Encode());
        SendDestinyUpdates(updates); //consumed
    }
}

void DestinyManager::MakeMissile(Missile* pMissile) {
    SetMaxVelocity(pMissile->GetSpeed());
    SetPosition(pMissile->GetSelf()->position());
    m_agility = (pMissile->GetSelf()->type().mass() / 1000000) *
                    pMissile->GetSelf()->GetAttribute(AttrInertiaMod).get_double();
    // not sure if this will be used here.  attr 559
    pMissile->GetSelf()->SetAttribute(AttrAgility, m_agility, false);

    m_stop = false;
    m_ballMode = Destiny::Ball::Mode::MISSILE;
    m_stateStamp = sEntityList.GetStamp();

    SystemEntity* pTarget = pMissile->GetTargetSE();
    m_targetPoint = GPoint(pTarget->GetPosition());
    m_targetEntity.first = pTarget->GetID();
    m_targetEntity.second = pTarget;
    m_targetDistance = m_position.distance(m_targetPoint);

    GVector moveVector(m_position, m_targetPoint);
    moveVector.normalize();     //change vector to direction
    m_shipHeading = moveVector;

    SetUndockSpeed();   /* sets all needed variables for max velocity */
    mySE->SystemMgr()->AddEntity(pMissile, false); // we are not adding missiles to anomaly map

    std::vector<PyTuple*> updates;
    SetBallSpeed maxspeed;
        maxspeed.entityID = pMissile->GetID();
        maxspeed.speed = m_maxShipSpeed;
    updates.push_back(maxspeed.Encode());
    Rsp_LaunchMissile miss;
        miss.shipID = pMissile->GetLauncherID();
        miss.targetID = pTarget->GetID();
        miss.missileID = pMissile->GetID();
        miss.unk1 = 1;  // this is always "1" in packets.
        miss.unk2 = 1;  // this is always "1" in packets.
    updates.push_back(miss.Encode());
    SendDestinyUpdates(updates); //consumed
}

void DestinyManager::TractorBeamStart(SystemEntity* pShipSE, EvilNumber speed)
{
    if (m_orbiting)
        ClearOrbit();
    if (m_turning)
        ClearTurn();

    m_ballMode = Destiny::Ball::Mode::FOLLOW;

    m_stop = false;
    m_accel = false;
    m_decel = false;
    m_tractored = true;
    //m_moveTime = GetTimeMSeconds();
    m_stateStamp = sEntityList.GetStamp();

    m_targetPoint = pShipSE->GetPosition();
    GVector moveVector(m_position, m_targetPoint);
    m_targetDistance = moveVector.length();
    moveVector.normalize();
    m_shipHeading = moveVector;

    m_maxShipSpeed = speed.get_float();   //AttrMaxTractorVelocity
    m_maxSpeed = m_maxShipSpeed;
    m_velocity = m_shipHeading * m_maxSpeed;

    m_followDistance = 500 + pShipSE->GetRadius();
    m_shipAccelTime = 0.1f;

    m_activeSpeedFraction = m_userSpeedFraction = m_timeFraction = 1.0f;

    m_targetEntity.first = pShipSE->GetID();
    m_targetEntity.second = pShipSE;

    std::vector<PyTuple*> updates;
    SetBallSpeed ms;
        ms.entityID = mySE->GetID();
        ms.speed = m_maxShipSpeed;
    updates.push_back(ms.Encode());
    SetBallFree bf;
        bf.entityID = mySE->GetID();
        bf.is_free = 1;
    updates.push_back(bf.Encode());
    // may not be needed
    SetBallMass sbmass;
        sbmass.entityID = mySE->GetID();
        sbmass.mass = 10000;
    updates.push_back(sbmass.Encode());
    CmdSetSpeedFraction ssf;
        ssf.entityID = mySE->GetID();
        ssf.fraction = 1;
    updates.push_back(ssf.Encode());
    CmdFollowBall fb;
        fb.entityID = mySE->GetID();
        fb.targetID = pShipSE->GetID();
        fb.range = m_followDistance;
    updates.push_back(fb.Encode());
    SendDestinyUpdates(updates); //consumed
}

void DestinyManager::TractorBeamStop()
{
    Halt();
    m_tractored = false;
    std::vector<PyTuple*> updates;
    SetBallSpeed ms;
        ms.entityID = mySE->GetID();
        ms.speed = 0;
    updates.push_back(ms.Encode());
    SetBallFree bf;
        bf.entityID = mySE->GetID();
        bf.is_free = 0;
    updates.push_back(bf.Encode());
    SetBallMass sbmass;
        sbmass.entityID = mySE->GetID();
        sbmass.mass = mySE->GetSelf()->GetAttribute(AttrMass).get_float();
    updates.push_back(sbmass.Encode());
    SendDestinyUpdates(updates); //consumed
}

void DestinyManager::Jump(int32 fromGateID, bool showCloak/*true*/)
{
    // set vars to show ship stopped in setstate.  dont call Stop() or Halt() here.
    m_stop = true;
    m_accel = false;
    m_decel = false;
    //m_ballMode = Destiny::Ball::Mode::STOP;
    m_velocity = GVector(NULL_ORIGIN);          //this
    m_userSpeedFraction = 0.0f;
    m_activeSpeedFraction = 0.0f;     //this

    if (!m_cloaked)
        SendJumpOut(fromGateID);
    SendGateActivity(fromGateID);

    m_cloaked = showCloak;

    if (mySE->SysBubble() != nullptr)
        mySE->SysBubble()->RemoveExclusive(mySE);

    if (m_autoPilot) {
        // this is a hack, but allows me to not refactor everything for AP
        m_targetEntity.first = mySE->GetID();
        m_targetEntity.second = mySE;
    }
}

void DestinyManager::Cloak() {
    if (m_cloaked)
        return;
    m_cloaked = true;
    SendGFX10(mySE->GetID(), "effects.Cloak");
    if (mySE->SysBubble() != nullptr)
        mySE->SysBubble()->RemoveExclusive(mySE);
}

void DestinyManager::UnCloak() {
    if (!m_cloaked)
        return;
    m_cloaked = false;
    SendGFX10(mySE->GetID(), "effects.Uncloak");

    /*  i think this is just for modules...
    SetBallUncloak du;
        du.entityID = mySE->GetID();
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);
    //PyDecRef(up);
    */

    if (mySE->SysBubble() != nullptr)
        mySE->SysBubble()->AddBallExclusive(mySE);
}

// GFX method for space effects
// update this to use start/stop and repeat=0
// may have to switch this to gfx14 to enable other vars...
void DestinyManager::SendGFX10(uint32 entityID, std::string guid, int32 targetID/*0*/, int32 otherTypeID/*0*/) const {
    OnSpecialFX10 effect;
        effect.entityID = entityID;
        effect.targetID = (targetID == 0 ? PyStatic.NewNone() : new PyInt(targetID));
        effect.otherTypeID = (otherTypeID == 0 ? PyStatic.NewNone() : new PyInt(otherTypeID));
        effect.area = PyStatic.mtList();        // no data.  not used in client
        effect.guid = guid;
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
    PyTuple *up = effect.Encode();
    SendSingleDestinyUpdate(&up);
    //PyDecRef(up);
}

// def OnSpecialFX(shipID, moduleID, moduleTypeID, targetID, otherTypeID, area, guid, isOffensive, start, active, duration = -1, repeat = None, startTime = None, graphicInfo = None):
// GFX method for module and structure effects
void DestinyManager::SendGFX14(uint32 entityID, uint32 moduleID, uint32 moduleTypeID, uint32 targetID,
                                       uint32 chargeTypeID, std::string guid, bool isOffensive, bool start,
                                       bool isActive, int32 duration, uint32 repeat, int32 graphicInfo/*0*/) const
{
    OnSpecialFX14 effect;
        effect.entityID = entityID;
        effect.moduleID = moduleID;             // npc UID for npc's/drones
        effect.moduleTypeID = moduleTypeID;     // npc typeID for npc's/drones
        effect.targetID = (targetID == 0 ? PyStatic.NewNone() : new PyInt(targetID));
        effect.otherTypeID = (chargeTypeID == 0 ? PyStatic.NewNone() : new PyInt(chargeTypeID));
        effect.area = PyStatic.mtList();        // no data.  not used in client
        effect.guid = guid;
        effect.isOffensive = isOffensive;       // bool
        effect.start = start;                   // int bool
        effect.active = isActive;               // int bool
        effect.duration = duration;             // in ms
        effect.repeat = repeat;
        effect.startTime = GetFileTimeNow();    // to use event start time from II once completed (this currently isntr ight)
        effect.graphicInfo = (graphicInfo == 0 ? PyStatic.NewNone() : new PyInt(graphicInfo));
    PyTuple *up = effect.Encode();
    SendSingleDestinyUpdate(&up);
    //PyDecRef(up);
}

// helper functions for SendGFX calls
void DestinyManager::SendJumpOut(int32 gateID) const {
    SendGFX10(mySE->GetID(), "effects.JumpOut", gateID);
}

void DestinyManager::SendJettisonPacket() const {
    SendGFX10(mySE->GetID(), "effects.Jettison");
}

void DestinyManager::SendGateActivity(int32 gateID) const {
    SendGFX10(gateID, "effects.GateActivity");
}

void DestinyManager::SendJumpOutWormhole(uint32 wormholeID) const {
    /** @todo  this should also send otherTypeID - data is u/k at this time */
    SendGFX10(wormholeID, "effects.JumpOutWormhole", wormholeID);
}

void DestinyManager::SendWormholeActivity(uint32 wormholeID) const {
    SendGFX10(wormholeID, "effects.WormholeActivity");
}

/*
 *                  [PyTuple 2 items]
 *                    [PyInt 517]
 *                    [PyTuple 2 items]
 *                      [PyString "OnSpecialFX"]
 *                      [PyTuple 14 items]
 *                        [PyIntegerVar 1002332228246]
 *                        [PyIntegerVar 1002333797260]
 *                        [PyInt 11578]
 *                        [PyNone]
 *                        [PyNone]
 *                        [PyList 0 items]
 *                        [PyString "effects.Cloaking"]
 *                        [PyBool False]
 *                        [PyInt 1]
 *                        [PyInt 1]
 *                        [PyInt -1]
 *                        [PyInt 0]
 *                        [PyIntegerVar 129527563080275219]
 *                        [PyNone]
 *                [PyBool False]
 */

void DestinyManager::SendJumpOutEffect(std::string JumpEffect, uint32 shipID) const {
    sLog.Error("SendGFX", "SendJumpOutEffect - fix this");
    OnSpecialFX14 effect;
        effect.entityID = mySE->GetID();
        effect.targetID = new PyInt(shipID);
        effect.guid = "effects.JumpDriveOut";   /* JumpDriveInBO */
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 1;
        effect.duration = 5000;
        effect.repeat = 0;
        effect.startTime = GetFileTimeNow();
    PyTuple *up(effect.Encode());
    SendSingleDestinyUpdate(&up);
    //PyDecRef(up);
}

void DestinyManager::SendJumpInEffect(std::string JumpEffect) const {
    sLog.Error("SendGFX", "SendJumpInEffect - fix this");
    OnSpecialFX14 effect;
        effect.guid = "effects.JumpDriveIn";
        effect.entityID = mySE->GetID();
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 1;
        effect.duration = 2000;
        effect.repeat = 0;
        effect.startTime = GetFileTimeNow();
        effect.targetID = new PyInt(mySE->GetID());
    PyTuple *up(effect.Encode());
    SendSingleDestinyUpdate(&up);
    //PyDecRef(up);
}

// only used by UpdateShip calls
void DestinyManager::SendBallInteractive(const ShipItemRef shipRef, bool set/*false*/) const {
    // interactive means "ship has pilot"
    SetBallInteractive sbi;
    sbi.entityID = shipRef->itemID();
    sbi.interactive = set;
    PyTuple* up = sbi.Encode();
    SendSingleDestinyUpdate(&up);
    //PyDecRef(up);
}

void DestinyManager::SendTerminalExplosion(uint32 shipID, uint32 bubbleID, bool isGlobal/*false*/) const {
    //exploders = [ x[1][1][0] for x in state if x[1][0] == 'TerminalExplosion' ]
    /*
                  [PyTuple 2 items]                         x
                    [PyInt 62609]                           x[0]
                    [PyTuple 2 items]                       x[1]
                      [PyString "TerminalExplosion"]        x[1][0]
                      [PyTuple 3 items]                     x[1][1]
                        [PyIntegerVar 9000000000001190702]  x[1][1][0]
                        [PyInt 39]
                        [PyBool False]
                */
    //send an explosion special effects update...
     TerminalExplosion du;
        du.shipID = shipID;
        du.bubbleID = bubbleID;
        du.ballIsGlobal = isGlobal;
    PyTuple* up = du.Encode();
    SendSingleDestinyUpdate(&up);
    //PyDecRef(up);
}

void DestinyManager::SendSetState() const {
    if (!mySE->HasPilot())
        return;

    if (is_log_enabled(DESTINY__MESSAGE))
        _log(DESTINY__MESSAGE, "DM::SendSetState() Called for Ship:%s(%u) Pilot:%s(%u)", \
                        mySE->GetName(), mySE->GetID(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());

    SetState ss;
        ss.stamp = sEntityList.GetStamp();
        ss.ego = mySE->GetID();

    mySE->SystemMgr()->MakeSetState(mySE->SysBubble(), ss);
    PyTuple* tmp(ss.Encode());
    //setstate should be alone and immediate.  send directly
    mySE->GetPilot()->QueueDestinyUpdate(&tmp, true, true);   // consumed?
    mySE->GetPilot()->SetStateSent(true);
}

void DestinyManager::MarkPoint(const GPoint& position, std::string& name, std::string& desc)
{
    // create jetcan to visualize point in space
    ItemData idata(23, ownerSystem, mySE->GetLocationID(), flagNone, name.c_str(), position, desc.c_str());
    CargoContainerRef cRef = CargoContainerRef::StaticCast(InventoryItem::SpawnTemp(idata));
    if (cRef.get() == nullptr) {
        _log(DESTINY__WARNING, "MarkPoint() could not create Item for %s (%s)", name.c_str(), desc.c_str());
        return;
    }

    // create new container
    FactionData data = FactionData();
    ContainerSE* cSE = new ContainerSE(cRef, mySE->GetServices(), mySE->SystemMgr(), data);
    if (cSE == nullptr) {
        _log(DESTINY__WARNING, "MarkPoint() could not create SE for %s (%s)", name.c_str(), desc.c_str());
        return;
    }
    cRef->SetMySE(cSE);
    cSE->AnchorContainer();
    mySE->SystemMgr()->AddMarker(cSE);
}

void DestinyManager::SendDestinyUpdates(std::vector<PyTuple*>& updates, bool self_only/*false*/) const {
    // this check shouldnt be needed...
    if (!mySE->SystemMgr()->IsLoaded()) {
        for (auto &cur : updates)
            PySafeDecRef(cur);
        return;
    }
    if (self_only) {
        if (!mySE->HasPilot()) {
            // this entity is NOT a player ship...change to BubbleCast (or silently fail)
            if (mySE->SysBubble() != nullptr) {
                if (is_log_enabled(DESTINY__UPDATES))
                    _log( DESTINY__UPDATES, "[%u] BubbleCasting %lu DestinyUpdates to bubbleID %u from %s(%u)", \
                            sEntityList.GetStamp(), updates.size(), mySE->SysBubble()->GetID(), mySE->GetName(), mySE->GetID() );
                mySE->SysBubble()->BubblecastDestinyUpdate(updates, "DestinyUpdates");
            } else {
                for (auto &cur : updates)
                    PySafeDecRef(cur);
            }
            return;
        }
        if (is_log_enabled(PLAYER__MESSAGE))
            _log(PLAYER__MESSAGE, "[%u] DM::SendDestinyUpdates() called as 'self_only' for %s(%i)", \
                    sEntityList.GetStamp(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());

        for (std::vector<PyTuple*>::iterator itr = updates.begin(); itr != updates.end(); itr++) {
            //PyIncRef(*itr);
            mySE->GetPilot()->QueueDestinyUpdate(&(*itr));
        }
    } else if (mySE->IsOperSE()) { //These are global entities, so we have to send update to all bubbles in a system
        if (is_log_enabled(DESTINY__UPDATES))
            _log(DESTINY__UPDATES, "[%u] BubbleCasting Structure DestinyUpdates in %s from %s(%u)", \
                    sEntityList.GetStamp(), mySE->SystemMgr()->GetName(), mySE->GetName(), mySE->GetID());

        //Get all clients in our system
            // should this be bubblecast?  which would be faster?
        std::vector<Client*> cv;
        mySE->SystemMgr()->GetClientList(cv);
        for (auto const& player : cv)
            if (player->IsInSpace())
                player->QueueDestinyUpdates(updates);
    } else if (mySE->SysBubble() != nullptr) {
        if (is_log_enabled(DESTINY__UPDATES))
            _log(DESTINY__UPDATES, "[%u] BubbleCasting %lu DestinyUpdates to bubbleID %u from %s(%u)", \
                    sEntityList.GetStamp(), updates.size(), mySE->SysBubble()->GetID(),   \
                    (mySE->HasPilot()?mySE->GetPilot()->GetName():mySE->GetName()), \
                    (mySE->HasPilot()?mySE->GetPilot()->GetCharID():mySE->GetID()) );
            mySE->SysBubble()->BubblecastDestinyUpdate(updates, "DestinyUpdates");
    } else {
        _log(DESTINY__WARNING, "[%u] Cannot BubbleCast %lu DestinyUpdates; entity (%u) is not in any bubble. (mySE->SysBubble() == nullptr)", \
                sEntityList.GetStamp(), updates.size(), mySE->GetID() );
        for (auto &cur : updates)
            PySafeDecRef(cur);
        if (sConfig.debug.IsTestServer)
            EvE::traceStack();
    }
}

void DestinyManager::SendSingleDestinyEvent(PyTuple** ev, bool self_only/*false*/) const {
    // this check shouldnt be needed...
    if (!mySE->SystemMgr()->IsLoaded()) {
        PySafeDecRef(*ev);
        return;
    }
    if (self_only) {
        if (!mySE->HasPilot()) {
            // this entity is NOT a player ship...change to BubbleCast (or silently fail)
            if (mySE->SysBubble() != nullptr) {
                if (is_log_enabled(DESTINY__UPDATES))
                    _log( DESTINY__UPDATES, "[%u] BubbleCasting destiny event to bubbleID %u from %s(%u)", \
                    sEntityList.GetStamp(), mySE->SysBubble()->GetID(), mySE->GetName(), mySE->GetID() );
                mySE->SysBubble()->BubblecastDestinyEvent(ev, "DestinyEvent");
            }
            return;
        }
        if (is_log_enabled(PLAYER__MESSAGE))
            _log(PLAYER__MESSAGE, "[%u] DM::SendSingleDestinyEvent() DestinyEvent called as 'self_only' for %s(%i)", \
                sEntityList.GetStamp(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());

        mySE->GetPilot()->QueueDestinyEvent(ev);
    } else if (mySE->IsOperSE()) { //These are global entities, so we have to send update to all players in a system
        if (is_log_enabled(DESTINY__UPDATES))
            _log(DESTINY__UPDATES, "[%u] BubbleCasting Structure DestinyEvent in %s from %s(%u)", \
            sEntityList.GetStamp(), mySE->SystemMgr()->GetName(), mySE->GetName(), mySE->GetID());

        //Get all clients in our system
        std::vector<Client*> cv;
        for (auto const& player : cv)
            if (player->IsInSpace()) {
                PyIncRef(*ev);
                player->QueueDestinyEvent(ev);
            }
    } else if (mySE->SysBubble() != nullptr) {
        if (is_log_enabled(DESTINY__UPDATES))
            _log(DESTINY__UPDATES, "[%u] BubbleCasting DestinyEvent to bubbleID %u from %s(%u)", \
                    sEntityList.GetStamp(), mySE->SysBubble()->GetID(),   \
            (mySE->HasPilot()?mySE->GetPilot()->GetName():mySE->GetName()),\
            (mySE->HasPilot()?mySE->GetPilot()->GetCharID():mySE->GetID()) );
        mySE->SysBubble()->BubblecastDestinyEvent(ev, "DestinyEvent" );
    } else {
        _log(DESTINY__WARNING, "[%u] Cannot BubbleCast DestinyEvent; entity %s(%u) is not in any bubble. (mySE->SysBubble() == nullptr)", \
                sEntityList.GetStamp(), mySE->GetName(), mySE->GetID() );
        if (sConfig.debug.IsTestServer)
            EvE::traceStack();
    }
}

void DestinyManager::SendSingleDestinyUpdate(PyTuple **up, bool self_only/*false*/) const {
    // this check shouldnt be needed...
    if (!mySE->SystemMgr()->IsLoaded())
        return;
    if (self_only) {
        if (!mySE->HasPilot()) {
            // this entity is NOT a player ship...change to BubbleCast (or silently fail)
            if (mySE->SysBubble() != nullptr) {
                if (is_log_enabled(DESTINY__UPDATES))
                    _log( DESTINY__UPDATES, "[%u] BubbleCasting destiny update to bubbleID %u from %s(%u)", \
                    sEntityList.GetStamp(), mySE->SysBubble()->GetID(), mySE->GetName(), mySE->GetID() );
                mySE->SysBubble()->BubblecastDestinyUpdate(up, "DestinyUpdate");
            }
            return;
        }
        if (is_log_enabled(PLAYER__MESSAGE))
            _log(PLAYER__MESSAGE, "[%u] DM::SendSingleDestinyUpdate() DestinyUpdate called as 'self_only' for %s(%i)", \
            sEntityList.GetStamp(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());

        mySE->GetPilot()->QueueDestinyUpdate(up);
    } else if (mySE->IsOperSE()) { //These are global entities, so we have to send update to all players in a system
        if (is_log_enabled(DESTINY__UPDATES))
            _log(DESTINY__UPDATES, "[%u] BubbleCasting Structure DestinyUpdate in %s from %s(%u)", \
            sEntityList.GetStamp(), mySE->SystemMgr()->GetName(), mySE->GetName(), mySE->GetID());

        //Get all clients in our system
        std::vector<Client*> cv;
        for (auto const& player : cv)
            if (player->IsInSpace()) {
                PyIncRef(*up);
                player->QueueDestinyUpdate(up);
            }
    } else if (mySE->SysBubble() != nullptr) {
        if (is_log_enabled(DESTINY__UPDATES))
            _log(DESTINY__UPDATES, "[%u] BubbleCasting DestinyUpdate to bubbleID %u from %s(%u)", \
            sEntityList.GetStamp(), mySE->SysBubble()->GetID(),   \
            (mySE->HasPilot()?mySE->GetPilot()->GetName():mySE->GetName()),\
            (mySE->HasPilot()?mySE->GetPilot()->GetCharID():mySE->GetID()) );
        mySE->SysBubble()->BubblecastDestinyUpdate(up, "DestinyUpdate" );
    } else {
        _log(DESTINY__WARNING, "[%u] Cannot BubbleCast DestinyUpdate; entity %s(%u) is not in any bubble. (mySE->SysBubble() == nullptr)", \
        sEntityList.GetStamp(), mySE->GetName(), mySE->GetID() );
        if (sConfig.debug.IsTestServer)
            EvE::traceStack();
    }
}
