/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
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


DestinyManager::DestinyManager(SystemEntity *self)
: mySE(self),
m_maxSpeed(1.0f),
m_shipAccelTime(0.0f),
m_shipMaxAccelTime(0.0f),
m_ballMode(Destiny::Ball::Mode::STOP),
m_agility(0.0),
//m_massMKg(0.0f),
m_warpTimer(0),
m_moveTime(0.0),
m_turnTime(0.0),
m_posHack(sConfig.debug.PositionHack),
m_targetDistance(0),
m_followDistance(0),
m_speedToLeaveWarp(0),
m_turnAlignTime(5.0f),      // arbitrary default
m_warpAlignTime(10.0f),      // arbitrary default
m_shipWarpSpeed(1.0f),
m_maxShipSpeed(100.0f),
m_warpAccelTime(1),
m_warpDecelTime(1),
m_warpState(nullptr),
m_targBubble(nullptr),
m_warpCapacitorNeed(0.00001),
m_frozen(false),
m_stateStamp(0)
{
    m_bump = false;
    m_stop = false;
    m_accel = false;
    m_decel = false;
    m_alignTo = false;
    m_cloaked = false;
    m_turning = false;
    m_orbiting = 0;
    m_tractored = false;
    m_changeDelay = false;
    m_tractorPause = false;
    m_hasSentShipUpdates = false;

    m_prevSpeed = 0.0f;
    m_orbitTime = 0.0f;
    m_orbitRadTic = 0.0f;
    m_timeFraction = 0.0f;
    m_prevSpeedFraction = 0.0f;
    m_userSpeedFraction = 0.0f;
    m_activeSpeedFraction = 0.0f;
    m_maxOrbitSpeedFraction = 1.0f;

    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;
    m_velocity = GVector( NULL_ORIGIN );
    m_targetPoint = GPoint( NULL_ORIGIN );
    m_shipHeading = GVector( NULL_ORIGIN );
    m_targetHeading = GVector( NULL_ORIGIN );

    m_radius = mySE->GetRadius();
    m_position = mySE->GetPosition();

    m_turning = false;
    m_radians = 0.0;
    m_turnFraction = 0.0f;
    m_curveHeadDelta = GVector( NULL_ORIGIN );

    m_inclination = 0.0;
    m_longAscNode = 0.0;
}

DestinyManager::~DestinyManager() {
    m_warpTimer.Disable();
    SafeDelete(m_warpState);
}

// this is called once per tic by SystemEntity::Process()
void DestinyManager::Process() {
    double profileStartTime(GetTimeUSeconds());

    ProcessState();

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::destiny, GetTimeUSeconds() - profileStartTime);
}

void DestinyManager::ProcessState() {
    if (mySE->IsFrozen()) {
        Halt();
        return;
    }

    using namespace Destiny;
    switch(m_ballMode) {
        case Ball::Mode::STOP: {
            if (IsMoving()) {
                MoveObject();
                return;
            }
            Stop();
        } break;
        case Ball::Mode::GOTO: {
            MoveObject();
        } break;
        case Ball::Mode::MISSILE: {
            // if target was removed, continue movement and wait for Missile::EndOfLife() call to do cleanup
            //set current direction based on position and targetPoint.  this will keep missile aligned properly
            GVector moveVector(m_position, m_targetPoint);
            moveVector.normalize();
            //set position and direction for this round of movement
            m_shipHeading = moveVector;
            m_velocity = (moveVector * m_maxSpeed);
            SetPosition(m_position + m_velocity);
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
                        mySE->GetPilot()->SendErrorMsg("Internal Server Error.<br> Please Dock or Relog to reset your ship.");
                    } else {
                        _log(DESTINY__ERROR, "Destiny::ProcessState() Error!  NPC %s(%u) Has WarpState but checks are false.",  \
                                    mySE->GetName(), mySE->GetID());
                    }
                }
                return;
            }

            // Updated warp alignment and speed check.  -allan  17nov15
            GVector toVec(m_position, m_targetPoint);
            toVec.normalize();
            float dot = toVec.dotProduct(m_shipHeading);
            float degrees = EvE::Trig::Rad2Deg(std::acos(dot));

            if ((degrees < WARP_ALIGNMENT) and (m_activeSpeedFraction > 0.749)) {
                m_shipHeading = toVec;
                InitWarp();
                return;
            } else if (((GetTimeMSeconds() - m_moveTime) * 0.001f) > m_warpAlignTime) {
                // catchall for turn checks messed up, and m_moveTime > ship's warp align time
                if (mySE->HasPilot()) {
                    _log(DESTINY__ERROR, "Destiny::ProcessState() Error!  Ship %s(%u) for Player %s(%u) - warp align/speed is incorrect, but time > shipTimeToWarp.",  \
                                mySE->GetName(), mySE->GetID(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());
                } else {
                    _log(DESTINY__ERROR, "Destiny::ProcessState() Error!  NPC %s(%u) - warp align/speed is incorrect, but time > shipTimeToWarp.",  \
                            mySE->GetName(), mySE->GetID());
                }
                m_shipHeading = toVec;
                InitWarp();
                return;
            }

            MoveObject();
        } break;
        case Ball::Mode::MUSHROOM:      // aoe?
        case Ball::Mode::BOID:          // this will turn RIGID after a set time
        case Ball::Mode::TROLL:         // seen for wrecks
        case Ball::Mode::MINIBALL:      // used for sentrys
        case Ball::Mode::FIELD:         // dunno
        case Ball::Mode::FORMATION:     // dunno
        case Ball::Mode::RIGID:         // item that never moves
            //no default on purpose
            break;
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
    if (m_orbiting != 0)
        Orbit(m_targetEntity.second, m_targetDistance);

    if ((fraction == m_userSpeedFraction) and (!startMovement)) {
        // no change.
        return;
    }

    if (is_log_enabled(DESTINY__MOVE_TRACE))
        _log(DESTINY__MOVE_TRACE, "Destiny::SetSpeedFraction() - %s(%u):   prevSpeed:%.2f, fraction: %.2f, start: %s, stop: %s, accel: %s, decel: %s",
             mySE->GetName(), mySE->GetID(), m_prevSpeed, fraction, startMovement ? "true" : "false", m_stop ? "true" : "false", \
             m_accel ? "true" : "false", m_decel ? "true": "false");

    // this is to start movement when setting fractional speeds from speedo in client.
    //  also a hack to circumvent above check when called again by goto, warp, align, follow for changing direction.
    if (startMovement) {
        m_stop = false;
        if (m_ballMode == Destiny::Ball::Mode::STOP)
            m_ballMode = Destiny::Ball::Mode::GOTO;
    }

    // prevent multiple client calls to Stop() from resetting ship speed.
    if (m_stop)
        return;

    /* movement is set according to time, speed fraction, and objects' maximum configured speed.
     * all *Fraction variables use fuzzy logic
     *  -allan 8Oct14  -major update 20Nov15  -added prop mod code 29Mar17
     *  -base movement rewrite/update 18Oct21
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

    if (m_ballMode == Destiny::Ball::Mode::WARP) {
        // set state to Ball::Mode::GOTO after setting warp decel variables, so warp completion will decel properly
        m_ballMode = Destiny::Ball::Mode::GOTO;
        return;
    }

    std::vector<PyTuple*> updates;
    // send on usf change but not for turn or orbit
    if (!m_turning and !m_orbiting) {
         CmdSetSpeedFraction du;
            du.entityID = mySE->GetID();
            du.fraction = fraction;
        updates.push_back(du.Encode());
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
        m_accel = false;
        m_decel = true;
        m_targBubble = nullptr;
        m_maxSpeed = m_speedToLeaveWarp;
        m_prevSpeed = m_speedToLeaveWarp;
        m_velocity = m_shipHeading * m_maxSpeed;
        m_prevSpeedFraction = m_maxSpeed / m_maxShipSpeed;
        //m_shipAccelTime = mySE->GetSelf()->GetAttribute(AttrAgility).get_float() * -log(1-(m_prevSpeedFraction));
        m_shipAccelTime = m_agility * -log(1-(m_prevSpeedFraction));
    } else if (m_userSpeedFraction > 0.01f) {
        // commanded speed fraction > 0 and ...
        float delta(1.0f);
        if ((m_activeSpeedFraction == m_userSpeedFraction) and (!m_prevSpeed)) {
            // ... nothing has changed.
            logType = 7;
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
        m_accel = false;
        m_decel = true;
        m_maxSpeed = 0;
        m_prevSpeed = m_maxShipSpeed * m_activeSpeedFraction;
        // this isnt accurate...hulk decel @81.35s but asf:0.0020 @ sec: 84.372
        m_shipAccelTime = m_shipMaxAccelTime * m_activeSpeedFraction;
    } else {
        // ... ship is not moving.  reset all move vars by calling Halt()
        logType = 6;
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
            case 6: { msg = "USF == 0 and ship is Stopped.  --Halt"; }              break;
            case 7: { msg = "ASF == USF                     --No Change"; }         break;
        }
        _log(DESTINY__MOVE_TRACE, "Destiny::UpdateVelocity - %s(%u):  %s  AccelTime: %.2f, USF: %.2f, ASF: %.2f, TF: %.2f, PSF: %.2f", \
                mySE->GetName(), mySE->GetID(), msg.c_str(), m_shipAccelTime, m_userSpeedFraction, \
                m_activeSpeedFraction, m_timeFraction, m_prevSpeedFraction);
    }
}

//Global Actions:
void DestinyManager::Stop() {
    if (m_stop)
        return;

    if (m_userSpeedFraction == 0.0f) {
        //state is already at stop. but m_stop wasnt set.
        // set m_stop and return.
        m_stop = true;
        return;
    } else if ((m_ballMode == Destiny::Ball::Mode::WARP) and (!IsWarping()))  {
        //warp aborted before initialized.  standard Stop() applies.
        m_ballMode = Destiny::Ball::Mode::STOP;
    } else if (IsMoving()) {
        //stop called while moving
        m_ballMode = Destiny::Ball::Mode::STOP;
    }

    m_accel = false;
    m_decel = false;
    m_alignTo = false;
    m_posHack = false;
    m_prevSpeed = 0.0f;
    m_prevSpeedFraction = 0.0f;

    ClearTurn();
    ClearOrbit();

    m_stateStamp = sEntityList.GetStamp();
    m_moveTime = GetTimeMSeconds();
    m_turnTime = 0.0;

    // need to check this after rewrite
    SetSpeedFraction(0.0f);
    m_stop = true;

    m_targBubble = nullptr;

    CmdStop du;
        du.entityID = mySE->GetID();
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
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
    m_turning = false;
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

    if (commanded) {
        // immediate halt via command.  send packet to stop ship.
        CmdStop du;
            du.entityID = mySE->GetID();
        PyTuple *up = du.Encode();
        SendSingleDestinyUpdate(&up);
        PyDecRef(up);
    }
    if (is_log_enabled(DESTINY__MOVE_TRACE))
        _log(DESTINY__MOVE_TRACE, "Destiny::Halt() - %s(%u) Halted - m_shipHeading: %.3f,%.3f,%.3f", \
                mySE->GetName(), mySE->GetID(), m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);
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
    if (mySE->SysBubble() == nullptr)
        mySE->SystemMgr()->AddEntity(mySE);
/*
    if (m_stateStamp > sEntityList.GetStamp()) {
        if (is_log_enabled(DESTINY__MOVE_TRACE))
            _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u): stateStamp (%u) > GetStamp (%u).", \
            mySE->GetName(), mySE->GetID(), m_stateStamp, sEntityList.GetStamp());
        return;
    }
*/
    if (m_changeDelay) {
        m_changeDelay = false;
        m_moveTime = GetTimeMSeconds() - (EvE::Time::Second * 2);
        m_stateStamp = sEntityList.GetStamp(); // reset m_moveTime to now and skip this tic
        _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - ChangeDelay - %s(%u): stateStamp: %u", \
                    mySE->GetName(), mySE->GetID(), m_stateStamp);
        return;
    }

    /* acceleration and deceleration are both logarithmic and the server needs to keep up with client position.
     * formula for time taken to accelerate from v to V, from https://wiki.eveonline.com/en/wiki/Acceleration
     *
     *   t=IM(10^-6) * -ln(1-(v/V))
     *   m_shipAccelTime = agility * -ln(1-(v/V))
     *
     * as this uses the natural log, the higher the speed, the slower the acceleration, to the limits of ln(0)
     * since lim ln(x) = -INFINITY where x->0+. and ln(0) is undefined, we will use
     *
     *   m_shipMaxAccelTime = (-ln(ASF_CHECK) * agility);   where ASF_CHECK is currently 0.02
     *
     * to define the time it will take a given ship to reach 99.98% of m_maxShipSpeed, at which point,
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
     */

    // check for moving ship changing heading
    if (m_turning)
        Turn();

    float speed(0.0f);
    std::string move = "";
    // keep timer in seconds.
    float timeStamp((GetTimeMSeconds() - m_moveTime) * 0.001f);
    //timeStamp = sEntityList.GetStamp() - m_stateStamp;
    // update tf for this tic
    m_timeFraction = (1 - exp(-timeStamp / m_agility)); //mySE->GetSelf()->GetAttribute(AttrAgility).get_float()));

    _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - timeFraction: %.5f, timeStamp: %.3f", m_timeFraction, timeStamp);

    if ((timeStamp > m_shipAccelTime) and (m_timeFraction > (1 - ASF_CHECK))) {
        speed = m_maxShipSpeed * m_activeSpeedFraction;
        m_activeSpeedFraction = m_userSpeedFraction;

        if (m_decel) {
            if (is_log_enabled(DESTINY__MOVE_TRACE))
                _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u) has decel'd from %.2fm/s to %.2fm/s in %.3fs.", \
                    mySE->GetName(), mySE->GetID(), m_prevSpeed, speed, timeStamp);
        } else if (m_accel) {
            if (is_log_enabled(DESTINY__MOVE_TRACE))
                _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u) has accel'd from %.2fm/s to %.2fm/s in %.3fs.", \
                mySE->GetName(), mySE->GetID(), m_prevSpeed, speed, timeStamp);
        }

        m_accel = false;
        m_decel = false;
        m_prevSpeed = 0.0f;
        m_prevSpeedFraction = 0.0f;

        if (m_userSpeedFraction) {
            // ship has reached full commanded speed
            move = "at constant speed, going";
        } else {
            //ship has reached full stop
            if (is_log_enabled(DESTINY__MOVE_TRACE))
                _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u) is at full stop after %.3f seconds.", \
                    mySE->GetName(), mySE->GetID(), timeStamp);
            Halt();
            return;
        }
    } else {
        // changed speed and asf != usf
        if (m_accel) {
            // object still accelerating.
            if (m_turning) {
                move = "accelerating in turn";
            } else {
                move = "accelerating";
            }

            if (m_prevSpeedFraction) {
                /* accel from previous non-full speed
                 *   take diff of psf and usf then multiply by tf
                 *   add result to psf to get asf
                 *  asf is the fraction of max speed the ship is moving at this tic.
                 */
                m_activeSpeedFraction = m_prevSpeedFraction + (m_userSpeedFraction - m_prevSpeedFraction) * m_timeFraction;
            } else {
                // this is simple acceleration.  asf = usf * tf
                m_activeSpeedFraction = m_userSpeedFraction * m_timeFraction;
            }
        } else if (m_decel) {
            // object still decelerating.
            if (m_turning) {
                // decel for turn
                move = "decelerating for turn";
            } else {
                move = "decelerating";
            }
            if (m_prevSpeedFraction) {
                // asf = psf - (psf - usf) * tf
                m_activeSpeedFraction = m_prevSpeedFraction - (m_prevSpeedFraction - m_userSpeedFraction) * m_timeFraction;
            } else {
                // this should never hit....should not have decel w/o psf
                sLog.Warning("Destiny::MoveObject()", "decel = true, but psf = 0.");
            }
        } else if (m_tractored or m_tractorPause) {
            ;   // do nothing here.  this is to remove error reporting from next line.
        } else {
            sLog.Error("Destiny::MoveObject()", "%s(%u) - move checks are not set right. Acc:%s, Dec:%s, Turn:%s, timeStamp:%.3f, Tractored:%s, TractorPause:%s", \
                    mySE->GetName(), mySE->GetID(), (m_accel ? "True" : "False"), (m_decel ? "True" : "False"), (m_turning ? "True" : "False"), \
                    timeStamp, (m_tractored ? "True" : "False"), (m_tractorPause ? "True" : "False"));
        }

        speed = (m_maxShipSpeed * m_activeSpeedFraction);
    }

    // ships tend to "level out" when stopping.  try to mimic that here (wip)
    // this will also need *something* with ship agility/inertia
    // using rifter to check/set base numbers
    if (m_stop)
        if (m_activeSpeedFraction < 0.6f)
            if (m_shipHeading.y < -0.1f) {
                m_shipHeading.y += 0.03f;
            } else if (m_shipHeading.y > 0.1f) {
                m_shipHeading.y -= 0.03f;
            }

    if (m_orbiting)
        if (m_orbiting < Destiny::Ball::Orbit::TooClose) {
            // object is orbiting...set orbit speed correctly.
            speed *= m_maxOrbitSpeedFraction;
            move += " in orbit";
        }

    if (is_log_enabled(DESTINY__MOVE_TRACE)) {
        if (m_prevSpeedFraction) {
            _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u) is %s at %.3f m/s (tf:%.4f asf:%.4f ps:%.2f psf:%.4f, sec: %.3f).", \
                mySE->GetName(), mySE->GetID(), move.c_str(), speed, m_timeFraction, m_activeSpeedFraction, m_prevSpeed, m_prevSpeedFraction, timeStamp);
        } else {
            _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u) is %s at %.3f m/s (tf:%.4f asf:%.4f sec: %.3f).", \
                mySE->GetName(), mySE->GetID(), move.c_str(), speed, m_timeFraction, m_activeSpeedFraction, timeStamp);
        }
    }

    //set velocity and position for this tic
    m_velocity = m_shipHeading * speed;
    // will need to hack position setting after turn cause it's still wrong.
    if (sConfig.debug.PositionHack or m_posHack) {
        SetPosition(m_position + m_velocity, true);   // force position update to client
        m_posHack = false;
    } else {
        SetPosition(m_position + m_velocity);
    }

    if (is_log_enabled(DESTINY__MOVE_DEBUG))
        _log(DESTINY__MOVE_DEBUG, "Destiny::MoveObject() - %s(%u) Pos:%.2f,%.2f,%.2f  Vel:%.3f,%.3f,%.3f  Head:%.3f,%.3f,%.3f", \
            mySE->GetName(), mySE->GetID(), m_position.x, m_position.y, m_position.z, m_velocity.x, m_velocity.y, m_velocity.z,\
            m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);

    if (sEntityList.GetTracking()) {
        // only create can when ship is moving significant amount
        if (m_activeSpeedFraction > sConfig.debug.ShipTrackingTime) {
            // create jetcan to visualize movement
            std::string str;
            if (m_decel)
                str += "Decel ";
            if (m_accel)
                str += "Accel ";
            if (m_turning)
                str += "Turn ";
            str += itoa(timeStamp);
            ItemData idata(23, ownerSystem, mySE->GetLocationID(), flagNone, str.c_str(), m_position, "Position Test");
            CargoContainerRef iRef = CargoContainer::SpawnTemp(idata);
            if (iRef.get() != nullptr) {
                // create new container
                FactionData data = FactionData();
                ContainerSE* cSE = new ContainerSE(iRef, mySE->GetServices(), mySE->SystemMgr(), data);
                if (cSE != nullptr) {
                    iRef->SetMySE(cSE);
                    mySE->SystemMgr()->AddMarker(cSE);
                }
            }
        }
    }

    if (sConfig.cosmic.BumpEnabled)
        if (mySE->HasPilot() and mySE->SysBubble()->HasPlayers()) // no players in bubble = nothing to check against (for now)
            CheckBump();
}

bool DestinyManager::IsTurn() {    //this is working.  dont change...yeah, but i did.
    // if ship is (reasonably) stopped, there is no turn.  immediately begin movement in desired direction
    if (m_activeSpeedFraction < 0.1) {
        m_shipHeading = m_targetHeading;
        return false;
    }
/*
    if  (EvE::AlmostEquals(m_shipHeading.x, m_targetPoint.x, 5)
    and  EvE::AlmostEquals(m_shipHeading.y, m_targetPoint.y, 5)
    and  EvE::AlmostEquals(m_shipHeading.z, m_targetPoint.z, 5)) {
        m_shipHeading = m_targetHeading;
        return false;
    }
*/
    // check for turning angle.  returns true if angle is enough to change movement variables
    // create isosceles triangle where legs are current direction and destination, then find angle between legs
    GVector toVec(m_position, m_targetPoint);
    toVec.normalize();
    float dot(toVec.dotProduct(m_shipHeading));
    // this only happens when heading and target are almost exact (or wrong)
    while (dot > 1.0f)
        dot -= 1;
    while (dot < -1.0f)
        dot += 1;

    //  this will set m_radians in the range of [0,pi].
    m_radians = acos(dot);
    if (m_radians < 0.0698132) {		//  TURN_ALIGNMENT = 4* = 0.0698132 rad
        m_shipHeading = m_targetHeading;
        return false;
    }

    if (is_log_enabled(DESTINY__TURN_TRACE)) {
		float degrees(EvE::Trig::Rad2Deg(m_radians));
        _log(DESTINY__TURN_TRACE, "Destiny::IsTurn() - %s(%u): dot: %.7f, radians:%.5f, degrees:%.3f",\
            mySE->GetName(), mySE->GetID(), dot, m_radians, degrees);
        _log(DESTINY__TURN_TRACE, "Destiny::IsTurn() m_shipHeading: %.7f,%.7f,%.7f.  m_targetHeading: %.7f,%.7f,%.7f", \
            m_shipHeading.x, m_shipHeading.y, m_shipHeading.z, m_targetHeading.x, m_targetHeading.y, m_targetHeading.z);
    }

    return true;
}

void DestinyManager::InitTurn()
{
    /* as per Dr. Santorine's studies, all ships turn the same.
     *   ship will slow down to [min speed] for turn depending on heading change
     *  while decel, ship will turn a slight amount until that [min speed] is hit
     * now, depending on ship and turn degrees, ship will remain at [min speed] for a time
     * this time is (currently) unknown.  ...is it time or degrees?
     * once x% of turn is complete, ship will begin accel to previous usf (but coded to psf.  needs work)
     * i am just beginning this, so no clue how im gonna do it yet.
     */

    m_turning = true;
    // get dir change
    GVector diff(m_shipHeading, m_targetHeading);

    // this should divide full curve into "floor(alignTime)" parts
    float pct = 1 / floor(m_turnAlignTime);
    // allowing heading + delta on each tic to provide a smooth transition
    m_curveHeadDelta = diff * pct;

    if (is_log_enabled(DESTINY__TURN_TRACE))
        _log(DESTINY__TURN_TRACE, "Destiny::InitTurn(1) - %s(%u): diff:%.5f,%.5f,%.5f, pct:%.3f, delta:%.7f,%.7f,%.7f", \
            mySE->GetName(), mySE->GetID(), diff.x, diff.y, diff.z, pct, \
            m_curveHeadDelta.x, m_curveHeadDelta.y, m_curveHeadDelta.z);

    //  calc min speed for this turn as absolute percent
    float minTurnSpeedFraction = (std::sqrt((cos(m_radians) + 1) / 2));

    if (is_log_enabled(DESTINY__TURN_TRACE))
        _log(DESTINY__TURN_TRACE, "Destiny::InitTurn(2) - %s(%u): minTurnSpeedFraction:%.3f(%.3f), alignTime:%.3f, ASF:%.3f", \
            mySE->GetName(), mySE->GetID(), minTurnSpeedFraction, (m_maxShipSpeed * minTurnSpeedFraction), m_turnAlignTime, m_activeSpeedFraction);

    // determine actual angle of turn for subsequent calc's
    diff.normalize();
    float degrees = EvE::Trig::Rad2Deg(acos(diff.dotProduct(m_shipHeading)));
    if (is_log_enabled(DESTINY__TURN_TRACE))
        _log(DESTINY__TURN_TRACE, "Destiny::InitTurn(3) - %s(%u): degrees:%.3f", \
                mySE->GetName(), mySE->GetID(), degrees);

    // method to determine turn speed based on above calc for any usf
    // note:  this does funny shit when decel from deactivated speed boost.   needs more thought
    if (minTurnSpeedFraction < m_activeSpeedFraction) {
        /** @todo need to check for SpeedBoost-changed vars here */
        // min turn speed is lower than current speed
        m_prevSpeedFraction = m_userSpeedFraction;
        m_userSpeedFraction = minTurnSpeedFraction;
        UpdateVelocity(true);
        m_moveTime = GetTimeMSeconds();
    }
}


//from new source at eve/client/script/ui/services\flightControls.py
//  self.curve = trinity.Tr2QuaternionLerpCurve()  - tried multiple iterations of this....never got close.
// current iteration is very close for turns <90*
void DestinyManager::Turn() {
    /*when changing directions....
     *  m_turnTime and m_moveTime will have to be reset - handled in BeginMovement()
     *  m_shipHeading will have to be reset - reset here and used in MoveObject() (our calling function)
     *  check for decel, then call UpdateVelocity() to set variables as needed.  MoveObject() will handle the rest.
     */

    // keep timer in seconds.
    float turnStamp((GetTimeMSeconds() - m_turnTime) * 0.001f);
    turnStamp = round(turnStamp);

    if (turnStamp > m_turnAlignTime) {
        // turn is complete.  clear and continue
        m_shipHeading = m_targetHeading;
        if (is_log_enabled(DESTINY__TURN_TRACE))
            _log(DESTINY__TURN_TRACE, "Destiny::Turn(complete) - turnTic > alignTime.  turn completed in %.2fs.  ShipHeading:%.7f,%.7f,%.7f", \
                turnStamp, m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);

        ClearTurn();

        if (m_alignTo)
            Stop();

        return;
    }


/** @note:  potential problems...
 *  this check uses m_decel/decelTimer to check for ship velocity changes for turns
 *  p1 - the problem i see (before testing) is what if speed is changed during turn?
 *  that will reset all move vars, negating this check and sending us into UB.
 *  p3 - SpeedBoost() calls will negate previous calcs and do weird shit with speed fractions
 *    potential solutions...
 *  p1 - delay speed change until this decel/turn is complete.  check for commanded speed change in ?? and reset/cancel current turn.
 *       let code work out needed changes (may be mature enough to deduce that)
 *       may not be....will have to reset m_decel, usf/psf, turn, maybe more.  need testing
 *  p3 - unknown at this time.
 */
    // check for decel
    if (m_decel) {
        /** @todo need to check for SpeedBoost-changed vars here */
        float moveStamp((GetTimeMSeconds() - m_turnTime) * 0.001f);
        if (moveStamp > m_shipAccelTime) {
            // decel complete.  reapply original commanded speed
            m_userSpeedFraction = m_prevSpeedFraction;
            if (is_log_enabled(DESTINY__TURN_TRACE)) {
                float degrees(EvE::Trig::Rad2Deg(m_radians));       // this is only for reference
                _log(DESTINY__TURN_TRACE, "Destiny::Turn(check) - Turn decel complete.  resume accel. degRemain:%.3f, ShipHeading:%.7f,%.7f,%.7f", \
                    degrees, m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);
            }
            UpdateVelocity(true);
            m_posHack = true;
        } else {
            // do ships begin turn during decel?  depends on angle?
            // yes, on major heading change, ships will turn slightly on decel
            _log(DESTINY__TURN_TRACE, "Destiny::Turn(check) - Turn decel ongoing.");
        }
    }

    m_shipHeading += m_curveHeadDelta;

    if (is_log_enabled(DESTINY__TURN_TRACE))
        _log(DESTINY__TURN_TRACE, "Destiny::Turn(end) - tf:%.3f, turnStamp:%.3f, degRemain:%.3f, newShipHeading:%.7f,%.7f,%.7f", \
        m_timeFraction, turnStamp, (EvE::Trig::Rad2Deg(m_radians)), m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);
}

void DestinyManager::ClearTurn() {
    //SetPosition(m_position, sConfig.debug.PositionHack);   // (PositionHack == true) here will force position update to client
    m_turning = false;
    m_radians = 0.0f;
    // this may not be right if other movement has changed
    if (m_decel)
        if (m_prevSpeedFraction > 0.0f) {
            m_userSpeedFraction = m_prevSpeedFraction;
            m_prevSpeedFraction = 0.0f;
        }

    m_turnTime = 0.0;
}

void DestinyManager::MarkPoint(const GPoint& position, std::string& name, std::string& desc)
{
    // create jetcan to visualize movement
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

void DestinyManager::Follow() {
    //  Follow is also used by client as AlignTo.
    const GPoint& target_point = m_targetEntity.second->GetPosition();
    GVector heading(m_position, target_point);
    m_targetDistance = (uint32)(heading.length() - m_radius - m_targetEntity.second->GetRadius());

    // update target point
    heading.normalize();
    m_targetPoint = target_point + (heading * m_targetDistance);

    if (m_targetDistance < m_followDistance) {
        if (mySE->HasPilot())
            if (mySE->GetPilot()->IsAutoPilot()) {
                SetSpeedFraction(0.1);
                _log(AUTOPILOT__TRACE, "DestinyManager::Follow() - Target within FollowDistance.  SpeedFraction = 0.1.");
                return;
            }
    // this will allow following entities to keep their follow state, yet stop movement if within their follow distance.
    //  by keeping their follow state, once the distance is greater than their follow distance, they will begin movement again.
        if (m_tractored) {
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
            m_tractorPause = true;
            m_activeSpeedFraction = m_userSpeedFraction = m_timeFraction = m_prevSpeedFraction = 0.0f;
            return;
        } else {
            if ((m_targetEntity.second->IsDynamicEntity()) and (m_targetEntity.second->DestinyMgr()->IsMoving())) {
                // this will mimic real movement, where ship will decel instead of a sudden halt
                //  still need to call MoveObject() here
                SetSpeedFraction(0.2);
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
            m_velocity = m_shipHeading * m_maxSpeed;
            m_moveTime = GetTimeMSeconds();
            m_stateStamp = sEntityList.GetStamp();
            m_prevSpeedFraction = 0.0f;
            // there is no accel/decel for tractor'd items
            m_activeSpeedFraction = m_userSpeedFraction = m_timeFraction = 1;
        } else if (m_userSpeedFraction < 0.1f) {
            SetSpeedFraction(1.0f);
        }
    }

    MoveObject();
}

/* eve/client/script/ui/services\flightPredictionSvc.py
"""
Prediction service for in-space flight
"""
*/
void DestinyManager::Orbit() {
    // data consistency checks...
    if ((m_targetDistance > BUBBLE_RADIUS_METERS) or (m_followDistance > BUBBLE_RADIUS_METERS)) {
        // well, something fucked up.  stop object and throw error.   player can reset if they want to.
        if (mySE->HasPilot())
            mySE->GetPilot()->SendErrorMsg("Internal Server Error.");
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
        _log(DESTINY__ORBIT_TRACE, "1 - %s(%u): timeStamp:%.3f, centers:%.2f, edges:%.2f, target:%u, follow:%u", \
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
    Tp += (Tv*Th); // use Tv*Th and add to position to account for target movement.  Tv for non-moving targets return 0.
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
    m_orbiting = Destiny::Ball::Orbit::None;
    m_orbitTime = 0.0f;
    m_orbitRadTic = 0.0f;
    m_targetDistance = 0;
    m_followDistance = 0;
    m_maxOrbitSpeedFraction = 1.0f;
}

void DestinyManager::InitWarp() {
    // reset sub-warp move variables for warping
    ClearTurn();
    ClearOrbit();

    m_accel = false;
    m_decel = false;
    m_posHack = false;
    m_turnTime = 0.0;
    m_prevSpeed = 0.0f;
    m_prevSpeedFraction = 0.0f;
    // im guessing since we're going into warp, asf is not needed.
    m_activeSpeedFraction = 0.0f;

    // warp time and distance math
    //   allan 1Nov14 - 14Nov14
    //  rewrite 3jan15  to use distance instead of time for warping.  more accurate now, and covers ALL distances.
    //  calculation and implementation update   9Jan15      accuracy is within 1000m

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

    // check for enough cap to warp....this moved here from WarpTo() call...makes more sense here
    if (mySE->HasPilot()) {
        Client *pClient = mySE->GetPilot();

        /*  capacitor for warp formulas from https://oldforums.eveonline.com/?a=topic&threadID=332116
         *  Energy to warp = warpCapacitorNeed * mass * au * (1 - warp_drive_operation_skill_level * 0.10)
         ** @note:  warpCapacitorNeed is double...max ive seen is shuttles @ 0.00000134771  and indys @ 0.000000108911
         */

        float currentShipCap = pClient->GetShip()->GetAttribute(AttrCapacitorCharge).get_float();
        double capNeeded = mySE->GetSelf()->GetAttribute(AttrMass).get_float() * m_warpCapacitorNeed * (m_targetDistance / ONE_AU_IN_METERS);
        capNeeded *= (1.0f - (0.1f * pClient->GetChar()->GetSkillLevel(EvESkill::WarpDriveOperation)));

        _log(DESTINY__WARNING, "Warp Cap need for %s(%u) for %luAU is %.10f", \
                mySE->GetName(), mySE->GetID(), (m_targetDistance / ONE_AU_IN_METERS), capNeeded);

        // set min cap need to 1.0
        if (capNeeded < 1.0f)
            capNeeded = 1.0;

        //  check if ship has enough capacitor to warp full distance
        if (capNeeded > currentShipCap) {
            // not enough cap.  reset everything based on available cap
            capNeeded = currentShipCap / (mySE->GetSelf()->GetAttribute(AttrMass).get_float() * m_warpCapacitorNeed);
            if (capNeeded > 1.0f) {
                m_targetDistance = (uint32)capNeeded * ONE_AU_IN_METERS;
                GVector warp_direction(m_position, m_targetPoint);
                // make heading
                warp_direction.normalize();
                GPoint newTarget(m_position + (warp_direction * m_targetDistance));
                m_targetPoint = newTarget;
                m_targBubble = sBubbleMgr.GetBubble(mySE->SystemMgr(), newTarget);
                //WarpingWithAvailablePowerBody
            } else {
                // if not enough cap to do min warp, cancel and return
                pClient->SendErrorMsg("You don't have enough capacitor charge to warp.");
                _log(DESTINY__WARNING, "Destiny::InitWarp() - %s(%u): Capacitor needed vs current  %.3f / %.3f",
                        mySE->GetName(), mySE->GetID(), capNeeded, currentShipCap);

                Stop();
                return;
            }
        } else {
            capNeeded = currentShipCap - capNeeded;
        }

        //drain cap
        mySE->GetSelf()->SetAttribute(AttrCapacitorCharge, capNeeded);
        mySE->GetShipSE()->Warp();  //turn off non warp-safe modules
    }

    /*  this is from http://community.eveonline.com/news/dev-blogs/warp-drive-active/
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

    bool cruise(true);
    float cruiseTime(0.0f);
    double accelDistance(0.0), decelDistance(0.0), cruiseDistance(0.0);
    double warpSpeedInMeters(m_shipWarpSpeed * ONE_AU_IN_METERS);
    // fudge this a bit for accel/decel distances
    if (m_targetDistance < warpSpeedInMeters) {
        //  short warp....no cruise
        // this isnt very accurate....times and distances are a bit off....
        cruise = false;
        // accel = 1/3 decel
        accelDistance = (m_targetDistance / 3);
        decelDistance = (m_targetDistance - accelDistance);
        warpSpeedInMeters = accelDistance;
        m_warpDecelTime = log(decelDistance / 3);
        m_warpAccelTime = log(accelDistance / 3) / 3;
    } else {
        // all ships base time is 29s for distances > ship warp speed
        m_warpAccelTime = 7;
        m_warpDecelTime = 21; // accel *3
        decelDistance = exp(m_warpDecelTime);   // ship warp speed in meters * 1.7
        accelDistance = exp(3 * m_warpAccelTime);       // ship warp speed in meters
        cruiseDistance = (m_targetDistance - accelDistance - decelDistance);
        cruiseTime = (cruiseDistance / warpSpeedInMeters);
    }

    //  set total warp time based on above math.
    float warpTime(m_warpAccelTime + m_warpDecelTime + std::floor(cruiseTime));

    GVector warp_vector(m_position, m_targetPoint);
    warp_vector.normalize();

    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        _log(DESTINY__WARP_TRACE, "Destiny::InitWarp():Calculate - %s(%u): Warp will accelerate for %us, cruise for %.3f, then decelerate for %us, with total time of %.3fs, and warp speed of %.4f m/s.", \
            mySE->GetName(), mySE->GetID(), m_warpAccelTime, cruiseTime, m_warpDecelTime, warpTime, warpSpeedInMeters);
        _log(DESTINY__WARP_TRACE, "Destiny::InitWarp():Calculate - %s(%u): Accel distance is %.4f. Cruise distance is %.4f.  Decel distance is %.4f.  Direction is %.3f,%.3f,%.3f.", \
            mySE->GetName(), mySE->GetID(), accelDistance, cruiseDistance, decelDistance, warp_vector.x, warp_vector.y, warp_vector.z);
        _log(DESTINY__WARP_TRACE, "Destiny::InitWarp():Calculate - %s(%u): We will exit warp at %.2f,%.2f,%.2f at a distance of %lu AU (%um).", \
            mySE->GetName(), mySE->GetID(), m_targetPoint.x, m_targetPoint.y, m_targetPoint.z, m_targetDistance / ONE_AU_IN_METERS, m_targetDistance);
        GPoint destination = m_position + (warp_vector * m_targetDistance);
        _log(DESTINY__WARP_TRACE, "Destiny::InitWarp():Calculate - %s(%u): calculated exit is %.2f,%.2f,%.2f and vector is %.4f,%.4f,%.4f.", \
            mySE->GetName(), mySE->GetID(), destination.x, destination.y, destination.z, warp_vector.x, warp_vector.y, warp_vector.z);
        GVector diff(m_targetPoint, destination);
        _log(DESTINY__WARP_TRACE, "Destiny::InitWarp():Calculate - target vs calculated is %.2fm.", diff.length());
    }
    //  reset deceltime (from duration to time) for time check in WarpDecel()
    m_warpDecelTime = m_warpAccelTime + floor(cruiseTime);
    m_stateStamp = sEntityList.GetStamp();

    SafeDelete(m_warpState);
    m_warpState = new WarpState(
                        m_stateStamp,
                        m_targetDistance,
                        warpSpeedInMeters,
                        accelDistance,
                        cruiseDistance,
                        decelDistance,
                        warpTime,
                        true,
                        false,
                        false,
                        warp_vector );

    //clear targets
    mySE->TargetMgr()->ClearAllTargets();
    //mySE->TargetMgr()->OnTarget(nullptr, TargMgr::Mode::Clear, TargMgr::Msg::WarpingOut);

    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;

    WarpAccel(0);
}

void DestinyManager::WarpAccel(uint16 sec_into_warp) {
    /* For acceleration, k = 3.
     * distance = e^(k*s)
     * speed = k*e^(k*s)
     */
    double currentDistance = exp(3 * sec_into_warp);

    if (mySE->SysBubble() != nullptr)
        if (currentDistance > BUBBLE_RADIUS_METERS)
            if (mySE->SysBubble() != m_targBubble) {
                if (is_log_enabled(DESTINY__WARP_TRACE))
                    _log(DESTINY__WARP_TRACE, "Destiny::WarpAccel(): %s(%u) is being removed from bubble %u.",\
                            mySE->GetName(), mySE->GetID(), mySE->SysBubble()->GetID());
                mySE->SysBubble()->Remove(mySE);
            }

    if (currentDistance > m_warpState->accelDist) {
        currentDistance = m_warpState->accelDist;
        m_warpState->accel = false;
        if (m_warpState->cruiseDist > 0) {
            m_warpState->cruise = true;
        } else {
            m_warpState->decel = true;
        }
    }

    m_targetDistance -= currentDistance;
    double currentShipSpeed = (3 * currentDistance);

    if (m_warpState->accel)
        if (is_log_enabled(DESTINY__WARP_TRACE))
            _log(DESTINY__WARP_TRACE, "Destiny::WarpAccel(): %s(%u) - Warp Accelerating(%us): velocity %.4f m/s with %u m left to go. Current distance %.4f from origin.", \
                    mySE->GetName(), mySE->GetID(), sec_into_warp, currentShipSpeed, m_targetDistance, currentDistance);

    WarpUpdate(currentShipSpeed);
}

void DestinyManager::WarpCruise(uint16 sec_into_warp) {
    /* in cruise....calculate distance only to update internal position data. */
    m_targetDistance -= m_warpState->warpSpeed;

    if ((m_targetDistance - m_warpState->warpSpeed) < m_warpState->decelDist) {
        m_warpState->cruise = false;
        m_warpState->decel = true;
    }

    if (is_log_enabled(DESTINY__WARP_TRACE))
        _log(DESTINY__WARP_TRACE, "Destiny::WarpCruise(): %s(%u) - Warp Crusing(%us): velocity %.4f m/s. with %u m left to go.", \
                mySE->GetName(), mySE->GetID(), sec_into_warp, m_warpState->warpSpeed, m_targetDistance);

    WarpUpdate(m_warpState->warpSpeed);
}

void DestinyManager::WarpDecel(uint16 sec_into_warp) {
    /* For deceleration, k = -1.
     * distance = e^(k*s)
     * speed = -k*e^(k*s)
     */
    uint8 decelTime = (sec_into_warp - m_warpDecelTime);
    double currentDistance = (m_warpState->total_distance - (exp(-decelTime) * m_warpState->decelDist));
    m_targetDistance = (int32)(m_warpState->total_distance - currentDistance);
    double currentShipSpeed = (m_warpState->warpSpeed * exp(-decelTime));

    if (is_log_enabled(DESTINY__WARP_TRACE))
        _log(DESTINY__WARP_TRACE, "Destiny::WarpDecel(): %s(%u) - Warp Decelerating(%us/%us): velocity %.4f m/s with %u m left to go.", \
                mySE->GetName(), mySE->GetID(), decelTime, sec_into_warp, currentShipSpeed, m_targetDistance);

    WarpUpdate(currentShipSpeed);
    if (currentShipSpeed <= m_speedToLeaveWarp)
        WarpStop(currentShipSpeed);
}

void DestinyManager::WarpUpdate(double currentShipSpeed) {
    //  update position and velocity for all stages.
    //  this method is ~1000m off actual.  could be due to rounding.   -allan 9Jan15
    m_velocity = (m_warpState->warp_vector * currentShipSpeed);
    SetPosition(m_targetPoint - (m_warpState->warp_vector * m_targetDistance));

    if (m_warpState->decel) {
        if (mySE->SysBubble() == nullptr) {
            if (is_log_enabled(DESTINY__WARP_TRACE))
                _log(DESTINY__WARP_TRACE, "Destiny::WarpUpdate()  %s(%u): Ship is %f from center of target bubble %u.",\
                        mySE->GetName(), mySE->GetID(), m_targBubble->GetCenter().distance(m_position), m_targBubble->GetID());
            if (m_targBubble->InBubble(m_position, true)) {
                if (is_log_enabled(DESTINY__WARP_TRACE))
                    _log(DESTINY__WARP_TRACE, "Destiny::WarpUpdate()  %s(%u): Ship at %.2f,%.2f,%.2f is calling Add() for bubble %u.", \
                            mySE->GetName(), mySE->GetID(), m_position.x, m_position.y, m_position.z, m_targBubble->GetID());
                m_targBubble->Add(mySE);
                SetPosition(m_position, true);
            }
        }
    }
}

void DestinyManager::WarpStop(double currentShipSpeed) {
    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        _log(DESTINY__WARP_TRACE, "Destiny::WarpStop(): %s(%u) - Warp complete. Exit velocity %.4f m/s with %u m left to go.", \
                mySE->GetName(), mySE->GetID(), currentShipSpeed, m_targetDistance);
        _log(DESTINY__WARP_TRACE, "Destiny::WarpStop(): %s(%u): Ship currently at %.2f,%.2f,%.2f.", \
                mySE->GetName(), mySE->GetID(), m_position.x, m_position.y, m_position.z);
    }
    if (mySE->IsShipSE())
        _log(AUTOPILOT__MESSAGE, "Destiny::WarpStop(): %s(%u) - Warp complete.", mySE->GetName(), mySE->GetID());
    m_targetPoint += (m_warpState->warp_vector *10000);
    // SetSpeedFraction() checks for m_state = Warp and warpstate != null to set decel variables correctly with warp decel.
    //   have to call this BEFORE deleting or reseting m_state or WarpState.
    SetSpeedFraction(0.0f);
    m_stop = true;
    SafeDelete(m_warpState);
    m_targBubble = nullptr;
    if ((mySE->IsNPCSE()) and (mySE->GetNPCSE()->GetAIMgr() != nullptr))
        mySE->GetNPCSE()->GetAIMgr()->WarpOutComplete();
}

//called whenever an entity is going away and can no longer be used as a target
void DestinyManager::EntityRemoved(SystemEntity *pSE) {
    if (m_targetEntity.second == pSE) {
        m_targetEntity.first = 0;
        m_targetEntity.second = nullptr;

        switch(m_ballMode) {
            case Destiny::Ball::Mode::FOLLOW: {
                _log(DESTINY__DEBUG, "%u: Our target entity has gone away. Stopping.", mySE->GetID());
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
    }
    if (m_targetEntity.second->DestinyMgr()->IsWarping()) { // The target is warping
        //mySE->TargetMgr()->ClearTarget(m_targetEntity.second);
        Stop();
        return true;
    }
    return false;
}

void DestinyManager::UpdateSpeedFraction(float speedPct/*0*/) {
    //this is called from Beyonce.CmdSetSpeedFraction() but only while turning
    if (m_turning) {
        // some yahoo decided to change speed while turning....gee, thanks.
        // ok, so, do shit here to keep move vars sane
        m_userSpeedFraction = speedPct;
        // have UpdateVelocity() reset move vars and continue.
        UpdateVelocity(IsMoving());
    }
}

// Basic Movement Calls:
void DestinyManager::BeginMovement() {
    // common movement for all types
    if (!m_hasSentShipUpdates) {
        // error fix for setting ship movement variables before ship is in bubble (cannot BubbleCast)
        std::vector<PyTuple*> updates;
        // i dont think this is used in crucible
        SetBallAgility sbagility;
            sbagility.entityID =  mySE->GetID();
            sbagility.agility = mySE->GetSelf()->GetAttribute(AttrInertiaMod).get_double();
        updates.push_back(sbagility.Encode());
        SetBallMassive sbmassive;
            sbmassive.entityID = mySE->GetID();
            sbmassive.is_massive = false;       // disable client-side bump checks
        updates.push_back(sbmassive.Encode());
        SetBallMass sbmass;
            sbmass.entityID = mySE->GetID();
            sbmass.mass = mySE->GetSelf()->GetAttribute(AttrMass).get_double();
        updates.push_back(sbmass.Encode());
        SendDestinyUpdates(updates); //consumed
        m_hasSentShipUpdates = true;
    }

    // this will have to be adjusted for cloak mod.
    if (IsCloaked())
        UnCloak();

    m_stop = false;

    // reset all move stamps
    m_stateStamp = sEntityList.GetStamp();
    m_moveTime = GetTimeMSeconds();
    m_turnTime = GetTimeMSeconds();     // only used if IsTurn() == true

    // if ship is not moving, set usf for movement
    if (m_userSpeedFraction < 0.1)
        m_userSpeedFraction = 1.0f;

    SetSpeedFraction(m_userSpeedFraction, true);
}

void DestinyManager::Follow(SystemEntity* pSE, uint32 distance) {
    //called from client as 'CmdFollowBall'
    //  also used by 'Approach'
    if ((m_ballMode == Destiny::Ball::Mode::FOLLOW)
    and (m_targetEntity.second == pSE)
    and (m_followDistance == distance)
    and (m_userSpeedFraction))
        return;

    //reset orbit vars in case we were orbiting before
    if (m_orbiting)
        ClearOrbit();
    // reset turn vars
    if (m_turning)
        ClearTurn();

    m_ballMode = Destiny::Ball::Mode::FOLLOW;
    m_targetPoint = pSE->GetPosition();

    // this makes ship approach station dock elevation (y), instead of approaching to stations "center point" position (where icon is)
    if (pSE->IsStationSE())
        m_targetPoint.y = stDataMgr.GetDockPosY(pSE->GetID());

    GVector targHeading(m_position, m_targetPoint);
    targHeading.normalize();
    m_targetHeading = targHeading;

    m_targetEntity.first = pSE->GetID();
    m_targetEntity.second = pSE;
    m_followDistance = distance;

    if (IsTurn())
        InitTurn();

    BeginMovement();

    CmdFollowBall du;
        du.entityID = mySE->GetID();
        du.targetID = pSE->GetID();
        du.range = (int32)distance;
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

void DestinyManager::AlignTo(SystemEntity* pSE) {
    // should this Stop() once alignment has been achieved?  i'd say yes.  config option?
    // i originally set it like this, but aknor didnt like it, so it was removed
    m_alignTo = true;
    Follow(pSE, 0);
}

void DestinyManager::GotoDirection(const GPoint& direction) {
    //reset orbit vars in case we were orbiting before
    // this is also called when an orbited object is destroyed
    if (m_orbiting)
        ClearOrbit();
    // reset turn vars as this is most likely a dir change
    if (m_turning)
        ClearTurn();

    m_ballMode = Destiny::Ball::Mode::GOTO;
    m_targetHeading = direction;
    m_targetPoint = direction * 1.0e16;

    if (IsTurn())
        InitTurn();

    BeginMovement();

    CmdGotoDirection du;
        du.entityID = mySE->GetID();
        du.x = m_targetHeading.x;
        du.y = m_targetHeading.y;
        du.z = m_targetHeading.z;
    PyTuple* up = du.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

void DestinyManager::GotoPoint(const GPoint& point) {
    //reset orbit vars in case we were orbiting before
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

    if (IsTurn())
        InitTurn();

    BeginMovement();

    CmdGotoPoint gtpoint;
        gtpoint.entityID = mySE->GetID();
        gtpoint.x = m_targetPoint.x;
        gtpoint.y = m_targetPoint.y;
        gtpoint.z = m_targetPoint.z;
    PyTuple* up = gtpoint.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

void DestinyManager::WarpTo(const GPoint& destPoint, int32 distance/*0*/, bool autoPilot/*false*/, SystemEntity* pSE/*nullptr*/) {
    /* warp order..
     * pick destination -> align/accel -> aura "warp drive active" -> cap drain -> accel
     *      -> enter warp -> warp -> decel -> leave warp -> coast -> stop
     */
    SafeDelete(m_warpState);

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

    GVector warp_distance(m_position, m_targetPoint);
    m_targetDistance = warp_distance.length();
    m_targetDistance -= distance;
    // change to heading
    warp_distance.normalize();
    // adjust for stop distance from our travel direction
    warp_distance *= distance;
    // adjust target point by calculated stopping point
    m_targetPoint -= warp_distance;

    if (mySE->HasPilot())
        if (m_targetDistance < minWarpDistance) {
            // warp distance too close.  cancel warp and return
            mySE->GetPilot()->SendErrorMsg("That is too close for your Warp Drive.");
            // send pos update
            if (sConfig.debug.PositionHack)
                SetPosition(mySE->GetPosition(), true);
            Stop();
            return;
        }

    //You will always exit warp at a random point, 2,500 meters from your actual exit point - per EveUni
    //m_targetPoint.MakeRandomPointOnSphereLayer(-2500, 2500);   disabled for testing

    // this will create bubble if needed
    m_targBubble = sBubbleMgr.GetBubble(mySE->SystemMgr(), m_targetPoint);

    // verify USF is > 0.75  (this is a hack to avoid multiple calls to SSF() )
    /** @todo  is there any case where we want to warp with USF < 1.0 ??  */
    if (m_userSpeedFraction < 0.7499)
        m_userSpeedFraction = 1.0f;

    // npcs have no warp restrictions (yet)
    if (mySE->IsNPCSE() or mySE->IsDroneSE()) {
        // do drones warp??   they can, yes...with limitations
        if (mySE->IsDroneSE()) {
            // put drone limit checks here
        }

        if (IsTurn())
            InitTurn();

        BeginMovement();

        if (!m_targBubble->HasPlayers()) {
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
            //send warp effect
            OnSpecialFX10 sfx;
                sfx.guid = "effects.Warping";
                sfx.entityID = mySE->GetID();
                sfx.isOffensive = false;
                sfx.start = true;
                sfx.active = true;
            updates.push_back(sfx.Encode());
            SendDestinyUpdates(updates); //consumed
        }
        if (is_log_enabled(NPC__MESSAGE))
            _log(NPC__MESSAGE, "Destiny::WarpTo() NPC %s(%u) to:%u from:%u, m_targetPoint: %.2f,%.2f,%.2f  distance: %i  m_targetDistance: %u",\
                    mySE->GetName(), mySE->GetID(), m_targBubble->GetID(), mySE->SysBubble()->GetID(), \
                    m_targetPoint.x, m_targetPoint.y, m_targetPoint.z, distance, m_targetDistance);
        return;
    }

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
     /** @todo  does this apply for ANY bubble along warp route or just end? */
    if (m_targBubble->HasWarpBubble())
        if (!mySE->GetSelf()->HasAttribute(AttrWarpBubbleImmune)) {
            /*  there is a bubble here and ship isnt immune.
             * at this point, determine where the bubble is
             * then set ship targPoint to random point 2500m(?) from center of bubble
             */
        }

    // found few warp error msgs in client and noted in BeyonceSvs.h
    //  test for and implement here

    // verify this is right to begin warp
    // check for autopilot.  it has 'special' checks in client for auto-disable by destiny update (anything other than 'Follow')
    if (autoPilot) {
        // AP will use code from Follow()
        Follow(pSE, distance);
    } else {
        // everything else will use code from GotoDir and BeginMovement
        if (IsTurn())
            InitTurn();

        BeginMovement();
    }

    // set ball mode for warping
    m_ballMode = Destiny::Ball::Mode::WARP;

    //set massive for warp.   self-only per client logs
    SetBallMassive bm;
        bm.entityID = mySE->GetID();
        bm.is_massive = false;       // disable client-side bump checks
    PyTuple *up = bm.Encode();
    SendSingleDestinyUpdate(&up, true);
    PyDecRef(up);
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
    //send warp effect
    OnSpecialFX10 sfx;
        sfx.guid = "effects.Warping";
        sfx.entityID = mySE->GetID();
        sfx.isOffensive = false;
        sfx.start = true;
        sfx.active = true;
    updates.push_back(sfx.Encode());
    SendDestinyUpdates(updates); //consumed

    if (is_log_enabled(DESTINY__WARP_TRACE))
        _log(DESTINY__WARP_TRACE, "Destiny::Warp() toBubble:%u from:%u, m_targetPoint: %.2f,%.2f,%.2f  exit distance: %i  m_targetDistance: %u",
             m_targBubble->GetID(), mySE->SysBubble()->GetID(), m_targetPoint.x, m_targetPoint.y, m_targetPoint.z, distance, m_targetDistance);
}

void DestinyManager::Orbit(SystemEntity *pSE, uint32 distance/*0*/) {
    if ((m_ballMode == Destiny::Ball::Mode::ORBIT)
    and (m_targetEntity.second == pSE)
    and (m_targetDistance == distance))
        return;

    if (m_orbiting)
        m_shipHeading = NULL_ORIGIN_V;

    /* this initial Orbit() call will, based on position data, determine the orbit plane, rotation (cw/ccw)
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
        _log(DESTINY__ORBIT_TRACE, "%s(%u) - Ship Data - agility:%.3f, inertia:%.3f, massMkg:%.3f, maxSpeed:%.2f, radius:%.2f", \
        mySE->GetName(), mySE->GetID(), m_agility, //mySE->GetSelf()->GetAttribute(AttrAgility).get_float(),
            mySE->GetSelf()->GetAttribute(AttrInertiaMod).get_float(),
             mySE->GetSelf()->GetAttribute(AttrMass).get_float() * 0.0000001, m_maxShipSpeed, m_radius);
    //EvE::traceStack();

    // Target (orbited object)
    double Tr = pSE->GetRadius();
    double Tm = pSE->GetSelf()->GetAttribute(AttrMass).get_double();
    if (Tm == 0.0)
        Tm = pSE->GetSelf()->type().mass();

    if (is_log_enabled(DESTINY__ORBIT_TRACE))
        _log(DESTINY__ORBIT_TRACE, "%s(%u) - Target Data - mass:%.3f, speed:%.2f, radius:%.2f", \
            mySE->GetName(), mySE->GetID(), Tm, (pSE->DestinyMgr() ? pSE->DestinyMgr()->GetSpeed() : 0 ), Tr);

    // fudge distance to work 'close enough' with all targets...this was trial-n-error
    double Rc  = ((distance + 150 + m_radius - (pSE->GetRadius() / 12)) * 1.2);
    double Rc2 = std::pow(Rc,2);
    double Vm2 = std::pow(m_maxShipSpeed,2);
    //double t2  = std::pow(mySE->GetSelf()->GetAttribute(AttrAgility).get_double(),2);
    double t2  = std::pow(m_agility,2);

    // the following equation is from "Ship Motion in Eve Online" by Scheulagh Santorine, Ph.D
    // radius needs target mass and grav const factored in....somehow.
    // orbit radius
    /* r = sqrt(6 * cbrt(108t^2*Vm^2 * Rc^2 + 8Rc^6 + 12sqrt(81t^4 *Vm^4 + 12t^2 * Vm^2 * Rc^10))
     * + (24Rc^4 / (108t^2 * Vm^2 * Rc^2 + 8Rc^2 + 12sqrt(81t^4 * Vm^4 * Rc^8 + 12t^2 * Vm^2 * Rc^10)^1/3)) + 12Rc^2) /6
     */
    double one = (108 * t2 * Vm2 * Rc2);
    double two = (12 * t2 * Vm2 *  std::pow(Rc,10));
    double three = (12 * std::sqrt(81 *  std::pow(mySE->GetSelf()->GetAttribute(AttrAgility).get_double(),4) *  std::pow(m_maxShipSpeed,4) + two));
    double four = (6 *  std::cbrt(one + 8 *  std::pow(Rc,6) + three));
    double five =  std::cbrt( std::sqrt(three *  std::pow(Rc,8) + two));
    double six = (one + (8 * Rc2) + (12 * five));
    m_followDistance = std::sqrt(four + (24 *  std::pow(Rc, 4) / six) + 12 * Rc2) / 6;

    int64 velocity = m_maxShipSpeed * ((distance / m_followDistance) + 0.065); // dunno where i got this from but seems to work very well.
    m_maxOrbitSpeedFraction = velocity / m_maxShipSpeed;

    m_orbitTime = (EvE::Trig::Pi2 * m_followDistance) / velocity;
    m_orbitRadTic = EvE::Trig::Pi2 / m_orbitTime;

    if (is_log_enabled(DESTINY__ORBIT_TRACE))
        _log(DESTINY__ORBIT_TRACE, "%s(%u) - Orbit Data - Rc:%.3f, velocity:%li, osf:%.2f, targetDistance:%u, followDistance:%u, orbitTime:%.1f, radTic:%.5f", \
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
    if (m_followDistance == 0) {
        _log(DESTINY__ERROR, "%s(%u) - FollowDistance is 0.", mySE->GetName(), mySE->GetID());
        m_followDistance = (uint32)(m_targetDistance + Tr + m_radius); // fudge something here.  will have to fix later, but this is close enough
    }

    CmdOrbit du;
        du.entityID = mySE->GetID();
        du.orbitEntityID = pSE->GetID();
        du.distance = (int32)m_targetDistance;
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
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
    m_changeDelay = true;   // skip a single tic before making change
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
    SystemEntity *station = mySE->SystemMgr()->GetSE(stationID);

    if (station == nullptr) {
        codelog(CLIENT__ERROR, "%s: Station %u not found.", pClient->GetName(), stationID);
        pClient->SendErrorMsg("Station Not Found, Docking Aborted.");
        return PyStatic.NewNone();
    }

    //get the station Docking Perimiter
    const GPoint stationPos = station->GetPosition();
    double rangeToStationPerimiter = m_position.distance(stationPos);
    rangeToStationPerimiter -= mySE->GetRadius();
    rangeToStationPerimiter -= station->GetRadius();

    // Verify range to station is within docking perimeter of 2500 meters:
    _log(DESTINY__TRACE, "Destiny::AttemptDockOperation() rangeToStationPerimiter is %.2fm", rangeToStationPerimiter);
    if (rangeToStationPerimiter > 2500.0) {
        AlignTo( station );   // Turn ship and move toward docking point - client will usually call Dock() automatically...sometimes
        if (mySE->HasPilot() and mySE->GetPilot()->CanThrow())
            throw UserError ("DockingApproach");
    }

    pClient->SetStateTimer(Player::State::Dock, sConfig.world.StationDockDelay *1000); // default @ 4sec();
    pClient->SetAutoPilot(false);

    return new PyLong(GetFileTimeNow());
}

void DestinyManager::DockingAccepted()
{
    Stop();
    UnCloak();
    Client *pClient = mySE->GetPilot();
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
        PyDecRef(up);
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
    // after UpdateVelocity() rewrite, only thing to do here is reset ship's speed data.
    //  UpdateVelocity will handle the rest.

    /** @todo need to check for turn and any changed vars from decel here */
    m_prevSpeed = m_maxSpeed * m_activeSpeedFraction;  //get current ship speed
    m_prevSpeedFraction = m_activeSpeedFraction;

    InventoryItemRef sRef = mySE->GetSelf();

    // prop mod state changed.  reset ship movement variables and update current movement, if applicable
    if (!sRef->HasAttribute(AttrInertiaMod))    // this should never hit
        sLog.Error("DM::UpdateShipVariables", "%s (%u) does not have an InertiaMod", mySE->GetName(), mySE->GetID());

    double mass = sRef->GetAttribute(AttrMass).get_double();
    double inertiaMod = sRef->GetAttribute(AttrInertiaMod).get_double();
    m_agility = mass * inertiaMod / 1000000;
    mySE->GetSelf()->SetAttribute(AttrAgility, m_agility, false);

    //TimeToWarp = ((ln(2) * m_inertiaMod * m_mass) / 500000);     //18.922
    m_warpAlignTime = (0.693147 * mass * inertiaMod) / 500000;
    m_turnAlignTime = m_warpAlignTime - TURN_TIME_OFFSET;
    m_shipMaxAccelTime = (-log(ASF_CHECK) * m_agility);

    // verify hull overspeed
    m_maxShipSpeed = sRef->GetAttribute(AttrMaxVelocity).get_float();
    if (m_maxShipSpeed > sRef->GetAttribute(AttrMaxDirectionalVelocity).get_float())
        m_maxShipSpeed = sRef->GetAttribute(AttrMaxDirectionalVelocity).get_float();

    // reset ship max speed using updated m_maxShipSpeed
    m_maxSpeed = m_maxShipSpeed * m_userSpeedFraction;
    // set asf as fraction of current speed over new max speed.
    m_activeSpeedFraction = m_prevSpeed / m_maxShipSpeed;     // this may give >1.0

    // update timer.  this will allow speed changes to be timed properly
    m_moveTime = GetTimeMSeconds();
    // not sure if this is really used here...
    //m_stateStamp = sEntityList.GetStamp();

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
            _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(psf!=0&asf<usf) - accelerating.");
        } else if (m_activeSpeedFraction > m_userSpeedFraction) {
            // ....moving and decelerating
            // - this also hits when prop mod activated while ship is decel
            _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(psf!=0&asf>usf) - decelerating.");
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
        _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost() - pSpeed:%.2f, maxSpeed:%.2f, maxShipSpeed:%.2f", \
                m_prevSpeed, m_maxSpeed, m_maxShipSpeed);
    }

    // update ship speed variables based on new data
    //  replaced SetSpeedFraction with UpdateVelocity
    // may need to resend CmdSetSpeedFraction here...maybe not, usf hasnt changed
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
        Orbit(m_targetEntity.second, m_targetDistance);

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
    PyDecRef(shipItem);

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
    /*
Frigates (incl. CovOps, Inty, AF) have an InertiaMod of 3.1
Destroyers 3.5
Industrials 1.0
Cruisers 0.55 (Elite/Faction 0.65)
Battlecruisers 1.1
Battleships 0.155
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
        m_shipWarpSpeed = sRef->GetAttribute(AttrWarpSpeedMultiplier).get_float();
    if (sRef->HasAttribute(AttrMaxVelocity))
        m_maxShipSpeed = sRef->GetAttribute(AttrMaxVelocity).get_float();
    if (sRef->HasAttribute(AttrWarpCapacitorNeed))
        m_warpCapacitorNeed = sRef->GetAttribute(AttrWarpCapacitorNeed).get_float() * 2; //modified

    // i dont think this is right....
    if (mySE->IsNPCSE() or mySE->IsDroneSE())
        m_maxShipSpeed = sRef->GetAttribute(AttrEntityCruiseSpeed).get_float();

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

    //TimeToWarp = ((ln(2) * m_inertiaMod * m_mass) / 500000);     //18.922
    m_warpAlignTime = (0.693147 * mass * inertiaMod) / 500000;
    m_turnAlignTime = m_warpAlignTime - TURN_TIME_OFFSET;
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
            sbmassive.is_massive = false;       // disable client-side bump checks
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
                    pMissile->GetSelf()->GetAttribute(AttrInertiaMod).get_float();
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
        miss.unk1 = 1;  // this and next are "1" in packets.
        miss.unk2 = 1;
    updates.push_back(miss.Encode());
    SendDestinyUpdates(updates); //consumed
}

void DestinyManager::Jump(bool showCloak)
{
    Halt();
    if (showCloak) {
        m_cloaked = true;
    }
    if (mySE->SysBubble() != nullptr)
        mySE->SysBubble()->RemoveExclusive(mySE);
}

void DestinyManager::Cloak() {
    if (m_cloaked)
        return;
    m_cloaked = true;
    SendCloakFx(true);
    if (mySE->SysBubble() != nullptr)
        mySE->SysBubble()->RemoveExclusive(mySE);
}

void DestinyManager::UnCloak() {
    if (!m_cloaked)
        return;
    m_cloaked = false;
    SendCloakFx();
    if (mySE->SysBubble() != nullptr)
        mySE->SysBubble()->AddBallExclusive(mySE);
}

void DestinyManager::TractorBeamStart(SystemEntity* pShipSE, EvilNumber speed)
{
    /** @todo  need to update this */
    m_ballMode = Destiny::Ball::Mode::FOLLOW;

    m_stop = false;
    m_accel = false;
    m_decel = false;
    m_turning = false;
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

/*
                  [PyTuple 2 items]
                    [PyInt 62696]
                    [PyTuple 2 items]
                      [PyString "OnSpecialFX"]
                      [PyTuple 10 items]
                        [PyIntegerVar 9000000000001190976]
                        [PyNone]
                        [PyNone]
                        [PyNone]
                        [PyNone]
                        [PyList 0 items]
                        [PyString "effects.Jettison"]
                        [PyInt 0]
                        [PyInt 1]
                        [PyInt 0]
                        */
void DestinyManager::SendJettisonPacket() const {
    OnSpecialFX10 effect;
        effect.entityID = mySE->GetID();
        effect.guid = "effects.Jettison";
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
    PyTuple* up = effect.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}
/*
 *                  [PyTuple 2 items]
 *                    [PyInt 8087]
 *                    [PyTuple 2 items]
 *                      [PyString "OnSpecialFX"]
 *                      [PyTuple 14 items]
 *                        [PyIntegerVar 1002332856217]
 *                        [PyIntegerVar 1002332856217]
 *                        [PyInt 12235]
 *                        [PyNone]
 *                        [PyNone]
 *                        [PyList 0 items]
 *                        [PyString "effects.AnchorDrop"]
 *                        [PyBool False]
 *                        [PyInt 1]
 *                        [PyInt 1]
 *                        [PyInt -1]
 *                        [PyInt 0]
 *                        [PyIntegerVar 129516974756172792]
 *                        [PyNone]
 */
void DestinyManager::SendAnchorDrop() const {
    OnSpecialFX14 effect;
        effect.entityID = mySE->GetID();
        effect.moduleID = mySE->GetID();
        effect.moduleTypeID = mySE->GetTypeID();
        effect.guid = "effects.AnchorDrop";
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 1;
        effect.startTime = GetFileTimeNow();
    PyTuple* up = effect.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

void DestinyManager::SendAnchorLift() const {
    OnSpecialFX14 effect;
        effect.entityID = mySE->GetID();
        effect.moduleID = mySE->GetID();
        effect.moduleTypeID = mySE->GetTypeID();
        effect.guid = "effects.AnchorLift";
        effect.isOffensive = 0;
        effect.start = 1;
        effect.startTime = GetFileTimeNow();
    PyTuple* up = effect.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

/*
                  [PyTuple 2 items]
                    [PyInt 517]
                    [PyTuple 2 items]
                      [PyString "OnSpecialFX"]
                      [PyTuple 10 items]
                        [PyIntegerVar 1002332228246]
                        [PyNone]
                        [PyNone]
                        [PyNone]
                        [PyNone]
                        [PyList 0 items]
                        [PyString "effects.Cloak"]
                        [PyInt 0]
                        [PyInt 1]
                        [PyInt 0]
                  [PyTuple 2 items]
                    [PyInt 517]
                    [PyTuple 2 items]
                      [PyString "OnSpecialFX"]
                      [PyTuple 14 items]
                        [PyIntegerVar 1002332228246]
                        [PyIntegerVar 1002333797260]
                        [PyInt 11578]
                        [PyNone]
                        [PyNone]
                        [PyList 0 items]
                        [PyString "effects.Cloaking"]
                        [PyBool False]
                        [PyInt 1]
                        [PyInt 1]
                        [PyInt -1]
                        [PyInt 0]
                        [PyIntegerVar 129527563080275219]
                        [PyNone]
                [PyBool False]
    */

/** @todo verify 'start' and 'active' here... */
void DestinyManager::SendCloakFx(bool apply/*false*/, bool module/*false*/) const {
    PyTuple *up(nullptr);
    if (module) {
        OnSpecialFX14 effect;
        effect.entityID = mySE->GetID();
        effect.isOffensive = 0;
        if (apply) {
            effect.guid = "effects.Cloaking";
            effect.start = 1;
            effect.active = 1;
        } else {
            effect.guid = "effects.Uncloak";
        }
        up = effect.Encode();
    } else {
        OnSpecialFX10 effect;
        if (apply) {
            effect.guid = "effects.Cloak";
        } else {
            effect.guid = "effects.Uncloak";
        }
        effect.entityID = mySE->GetID();
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
        up = effect.Encode();
    }
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

// def OnSpecialFX(shipID, moduleID, moduleTypeID, targetID, otherTypeID, area, guid, isOffensive, start, active, duration = -1, repeat = None, startTime = None, graphicInfo = None):

void DestinyManager::SendSpecialEffect10(uint32 entityID, uint32 targetID, std::string guid, bool isOffensive, bool start, bool isActive) const
{
    OnSpecialFX10 effect;
        effect.entityID = entityID;
        effect.targetID = targetID;
        effect.guid = guid;
        effect.area = new PyList();     // this is unused variable in client.
        effect.isOffensive = isOffensive;
        effect.start = start;
        effect.active = isActive;
    PyTuple *up = effect.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

// def OnSpecialFX(shipID, moduleID, moduleTypeID, targetID, otherTypeID, area, guid, isOffensive, start, active, duration = -1, repeat = None, startTime = None, graphicInfo = None):

void DestinyManager::SendSpecialEffect(uint32 entityID, uint32 moduleID, uint32 moduleTypeID, uint32 targetID,
                                       uint32 chargeTypeID, std::string guid, bool isOffensive, bool start,
                                       bool isActive, int32 duration, uint32 repeat, int32 graphicInfo/*0*/) const
{
    OnSpecialFX14 effect;
        effect.entityID = entityID;
        effect.moduleID = moduleID;
        effect.moduleTypeID = moduleTypeID;     // npc typeID for npc's/drones
        effect.targetID = (targetID == 0 ? PyStatic.NewNone() : new PyInt(targetID));
        effect.chargeTypeID = (chargeTypeID == 0 ? PyStatic.NewNone() : new PyInt(chargeTypeID));
        effect.guid = guid;
        effect.isOffensive = isOffensive;                  // bool
        effect.start = start;                   // bool
        effect.active = isActive;                  // bool
        effect.duration = duration;
        effect.repeat = repeat;
        effect.startTime = GetFileTimeNow();
        effect.graphicInfo = (graphicInfo == 0 ? PyStatic.NewNone() : new PyInt(graphicInfo));
    PyTuple *up = effect.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}
/*
                  [PyTuple 2 items]
                    [PyInt 62565]
                    [PyTuple 2 items]
                      [PyString "OnSpecialFX"]
                      [PyTuple 14 items]
                        [PyIntegerVar 9000000000001190096]
                        [PyIntegerVar 9000000000001190096]
                        [PyInt 11931]
                        [PyNone]
                        [PyNone]
                        [PyList 0 items]
                        [PyString "effects.ShieldBoosting"]
                        [PyBool False]
                        [PyInt 1]
                        [PyInt 1]
                        [PyFloat 5000]
                        [PyInt 1]
                        [PyIntegerVar 129756560173255648]
                        [PyNone]
                */

void DestinyManager::SendJumpOut(uint32 gateID) const {
    OnSpecialFX10 effect;
        effect.entityID = mySE->GetID();
        effect.targetID = gateID;
        effect.guid = "effects.JumpOut";
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
    PyTuple *up = effect.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

void DestinyManager::SendJumpOutWormhole(uint32 wormholeID) const {
    OnSpecialFX10 effect;
        effect.entityID = mySE->GetID();
        effect.targetID = wormholeID;
        effect.guid = "effects.JumpOutWormhole";
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
    PyTuple *up = effect.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

void DestinyManager::SendGateActivity(uint32 gateID) const {
    OnSpecialFX10 du;
        du.entityID = gateID;
        du.guid = "effects.GateActivity";
        du.isOffensive = 0;
        du.start = 1;
        du.active = 0;
    PyTuple* up = du.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

void DestinyManager::SendWormholeActivity(uint32 wormholeID) const {
    OnSpecialFX10 du;
        du.entityID = wormholeID;
        du.guid = "effects.WormholeActivity";
        du.isOffensive = 0;
        du.start = 1;
        du.active = 0;
    PyTuple* up = du.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

void DestinyManager::SendBallInteractive(const ShipItemRef shipRef, bool set/*false*/) const {
    // interactive means "ship has pilot"
    SetBallInteractive sbi;
        sbi.entityID = shipRef->itemID();
        sbi.interactive = set;
    PyTuple* up = sbi.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

void DestinyManager::SendJumpOutEffect(std::string JumpEffect, uint32 shipID) const {
    std::vector<PyTuple*> updates;
    CmdStop du;
        du.entityID = mySE->GetID();
    updates.push_back(du.Encode());
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
    updates.push_back(effect.Encode());
    SendDestinyUpdates(updates); //consumed
}

void DestinyManager::SendJumpInEffect(std::string JumpEffect) const {
    std::vector<PyTuple*> updates;
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
    updates.push_back(effect.Encode());
    CmdSetSpeedFraction ssf;
        ssf.entityID = mySE->GetID();
        ssf.fraction = 0.0;
    updates.push_back(ssf.Encode());
    SetBallVelocity sbv;
        sbv.entityID = mySE->GetID();
        sbv.x = 0.0;
        sbv.y = 0.0;
        sbv.z = 0.0;
    updates.push_back(sbv.Encode());
    SendDestinyUpdates(updates); //consumed
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
    PyDecRef(up);
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
            PyIncRef(*itr);
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
            } else {
                PySafeDecRef(*ev);
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
        PySafeDecRef(*ev);
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
