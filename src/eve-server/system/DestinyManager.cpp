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
    Updated:        Allan
*/

/** @note  THIS IS ONLY FOR SYSTEM ENTITES THAT MOVE!!  CLIENTS ARE NOT SYSTEM ENTITES.
 */

//#include "eve-server.h"
#include "EVEServerConfig.h"

#include "Client.h"
#include "EntityList.h"
#include "PyServiceMgr.h"
#include "StaticDataMgr.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "packets/Missile.h"
#include "system/DestinyManager.h"
#include "ship/Missile.h"
#include "station/Station.h"
#include "system/BubbleManager.h"
#include "system/Container.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"

using namespace Destiny;

DestinyManager::DestinyManager(SystemEntity *self)
: mySE(self),
m_maxSpeed(1.0f),
m_shipMaxAccelTime(0.0f),
State(DSTBALL_STOP),
m_warpTimer(5000),  //completely arbitrary.
m_moveTimer(0.0),
m_targetDistance(0.0),
m_followDistance(0.0),
m_stopDistance(0),
m_radius(1.0f),
m_mass(1.0f),
m_turnTic(1),
m_massMKg(1.0f),
m_alignTime(1.0f),
m_timeToEnterWarp(10.0f),
m_shipWarpSpeed(1.0f),
m_maxShipSpeed(100.0f),
m_shipAgility(1.0f),
m_shipInertia(1.0f),
m_warpStrength(0),
m_warpAccelTime(1),
m_warpDecelTime(1),
m_warpState(nullptr),
m_warpCapacitorNeed(0.00001f)
{
    m_bump = false;
    m_stop = false;
    m_accel = false;
    m_decel = false;
    m_cloaked = false;
    m_turning = false;
    m_inBubble = true;
    m_orbiting = 0;
    m_tractored = false;
    m_tractorPause = false;
    m_hasSentShipUpdates = false;
    m_capNeeded = 0.0f;
    m_stateStamp = 0;

    m_prevSpeed = 0.0f;
    m_orbitRadTic = 0.0f;
    m_prevSpeedFraction = 0.0f;
    m_userSpeedFraction = 0.0f;
    m_activeSpeedFraction = 0.0f;
    m_currentSpeedFraction = 0.0f;
    m_maxOrbitSpeedFraction = 0.0f;

    m_warpTimer.Disable();
    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;
    m_velocity = GVector( NULL_ORIGIN );
    m_targetPoint = GPoint( NULL_ORIGIN );
    m_shipHeading = GVector( NULL_ORIGIN );
    m_targetHeading = GVector( NULL_ORIGIN );

    m_position = mySE->GetPosition();
    _ClearTurn();
}

DestinyManager::~DestinyManager() {
    m_warpTimer.Disable();
    SafeDelete(m_warpState);
}

// this is called once per tic
void DestinyManager::Process() {
    double profileStartTime = 0.0;
    if (sConfig.server.UseProfiling)
        profileStartTime = GetTimeUSeconds();
    //check for and process Destiny State changes.
    ProcessState();

    if (sConfig.server.UseProfiling)
        sProfile.AddTime(_destinyProfile, GetTimeUSeconds() - profileStartTime);
}

void DestinyManager::ProcessState() {
    switch(State) {
        case DSTBALL_STOP: {
            if (IsMoving()) {
                _Move();
                break;
            }
            Stop();
        } break;
        case DSTBALL_GOTO: {
            _Move();
        } break;
        case DSTBALL_MISSILE: {
            // if target was removed, continue movement and wait for Missile::EndOfLife() call to do cleanup
            //set current direction based on position and targetPoint.  this will keep missile aligned properly
            GVector moveVector(m_position, m_targetPoint);
            moveVector.normalize();
            //set position and direction for this round of movement
            m_shipHeading = moveVector;
            m_velocity = (moveVector * m_maxSpeed);
            SetPosition(m_position + m_velocity);
        } break;
        case DSTBALL_ORBIT: {
            if (IsTargetInvalid())
                return;
            _Orbit();
        } break;
        case DSTBALL_FOLLOW: {
            if (IsTargetInvalid())
                return;
            _Follow();
        } break;
        case DSTBALL_WARP: {
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
             *  Acceleration and Deceleration are logrithmic with finite caps (instead of infinity) at the ends.
             *      see also:  my notes in _InitWarp()
             */
            if (m_warpState) {
                //warp is in progress
                uint16 sec_into_warp = (sEntityList.GetStamp() - m_stateStamp);
                //  speed and distance formulas based on current warp distance
                if (m_warpState->accel)
                    _WarpAccel(sec_into_warp);
                else if (m_warpState->cruise)
                    _WarpCruise(sec_into_warp);
                else if (m_warpState->decel)
                    _WarpDecel(sec_into_warp);
                else {// houston, we have a problem...
                    if (mySE->HasPilot()) {
                        _log(DESTINY__ERROR, "Destiny::ProcessState() Error!  Ship %s(%u) for Player %s(%u) Has WarpState but checks are false.",  \
                                        mySE->GetName(), mySE->GetID(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());
                        mySE->GetPilot()->SendErrorMsg("Internal Server Error. Ref: ServerError 35928.   Please Dock or Relog to reset your ship.");
                    } else {
                        _log(DESTINY__ERROR, "Destiny::ProcessState() Error!  Ship %s(%u) Has WarpState but checks are false.",  \
                                        mySE->GetName(), mySE->GetID());
                    }
                }
                break;
            }

            // Updated warp alignment and speed check.  -allan  17nov15
            GVector toVec(m_position, m_targetPoint);
            toVec.normalize();
            float dot = toVec.dotProduct(m_shipHeading);
            float degrees = EvE_RadiansToDegrees(acos(dot));

            if ((degrees < WARP_ALIGNMENT) and (m_currentSpeedFraction > 0.749)) {
                m_shipHeading = toVec;
                _InitWarp();
                return;
            } else if (m_currentSpeedFraction < 0.749) {
                if (m_userSpeedFraction < 0.7499)
                    SetSpeedFraction(1.0f, true);
            }
            _Move();
        } break;
        case DSTBALL_MUSHROOM:      // aoe?
        case DSTBALL_BOID:          // this will turn RIGID after a set time
        case DSTBALL_TROLL:         // seen for wrecks
        case DSTBALL_MINIBALL:      // used for sentrys
        case DSTBALL_FIELD:         // dunno
        case DSTBALL_FORMATION:     // dunno
        case DSTBALL_RIGID:         // item that never moves
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
void DestinyManager::SetSpeedFraction(float fraction, bool startMovement) {
    /** @todo  this does NOT start movement or anything like that.
     * it ONLY SETS speed fraction for object.
     * need to update this based on this new data
     */
    if ((fraction == m_userSpeedFraction) and (!startMovement))
        return;
    _log(DESTINY__MOVE_TRACE, "Destiny::SetSpeedFraction() - %s(%u):  fraction: %.2f, start: %i, stop: %i",
                 mySE->GetName(), mySE->GetID(), fraction, startMovement, m_stop );

    // this is to start movement when setting fractional speeds from speedo in client.
    //  also a hack to circumvent above check when called again by goto, warp, align, follow for changing direction.
    if (startMovement) {
        if (State == DSTBALL_STOP)
            State = DSTBALL_GOTO;
        m_stop = false;
    }

    // prevent multiple client calls to Stop() from resetting ship speed.
    if (m_stop) return;

    /* movement is set according to time vs speed fraction.         -allan 8Oct14  -major update 20Nov15    -added prop mod code 29Mar17
     * all *SpeedFraction variables use fuzzy logic
     *  m_userSpeedFraction (USF) is user-set speed control (fractional from speedo or full from goto, warp, align, follow, and stop).
     *   -> sets m_maxSpeed
     *  m_prevSpeedFraction (PSF) is previously-user-set speed fraction used for decel checks, as USF = 0.
     *   this allows for proper decel speeds when USF was previously < 1.
     *   -> caps decel when USF = 0
     *  m_activeSpeedFraction (ASF) holds ship's current speed setting.  set in _Move()
     *   this is actually a ratio of CSF to USF, capped by USF and/or OSF
     *   -> sets accel/decel when changing speeds while moving
     *  m_currentSpeedFraction (CSF) holds current euler value for time.  set in _Move()
     *   this is reset on initial turn based on turn angle, then follows normal progression.
     *   -> sets m_velocity
     *  m_maxOrbitSpeedFraction (OSF) is calculated based on orbit radius
     *   -> modifies m_velocity
     *  m_maxSpeed is ship maximum speed based on user input.  set in _UpdateVelocity()
     *   -> sets m_velocity
     *  m_targetPoint holds current target coords.  set by goto, warp, align, follow, orbit
     *   -> sets m_shipHeading
     *  m_shipHeading holds current direction and is set in _Turn()
     *   -> sets m_velocity
     *  m_velocity is current ship velocity.  set in _Move()
     *   m_velocity = m_shipHeading * m_currentSpeedFraction * m_maxSpeed
     */
    if ((m_shipMaxAccelTime < 1.0) and (mySE->IsDynamicEntity()))
        m_shipMaxAccelTime = (m_shipAgility * -log(0.0001));

    if (((!fraction) or (m_prevSpeed)) and (m_userSpeedFraction))
        m_prevSpeedFraction = m_userSpeedFraction;
    else
        m_prevSpeedFraction = 0.0f;

    m_userSpeedFraction = fraction;
    bool isMoving = false;
    if ((m_currentSpeedFraction > 0.05) or (m_activeSpeedFraction > 0.05))
        isMoving = true;
    _UpdateVelocity(isMoving);

    if (State == DSTBALL_WARP) {
        // set state to DSTBALL_GOTO after setting warp decel variables, so warp completion will decel properly
        State = DSTBALL_GOTO;
        return;
    }

    std::vector<PyTuple*> updates;

    DoDestiny_CmdSetSpeedFraction du;
        du.entityID = mySE->GetID();
        du.fraction = fraction;
    updates.push_back(du.Encode());

    if (mySE->IsNPCSE() or mySE->IsMissileSE() or mySE->IsContainerSE() or mySE->IsWreckSE()) {
        DoDestiny_SetMaxSpeed ms;   //NPCs and Missiles only.
            ms.entityID = mySE->GetID();
            ms.speedValue = m_maxSpeed;
        updates.push_back(ms.Encode());
    }

    SendDestinyUpdate(updates);
}

void DestinyManager::_UpdateVelocity(bool isMoving) {
    uint8 logType = 0;
    if ((State == DSTBALL_WARP) and m_warpState) {
        /*  _Warp() finished, and ship dropped out of warp at m_speedToLeaveWarp,
         *  reset m_shipMaxAccelTime as a fraction of m_speedToLeaveWarp/m_maxShipSpeed
         *  to set decel correctly, as m_speedToLeaveWarp varies with ship and warp distance.
         */
        logType = 1;
        m_decel = true;
        m_accel = false;
        m_shipMaxAccelTime *= (m_speedToLeaveWarp / m_maxShipSpeed);
        m_velocity = m_shipHeading * m_speedToLeaveWarp;
        m_maxSpeed = m_speedToLeaveWarp;
        //if (mySE->IsNPCSE())
        //    m_userSpeedFraction = 0.2;
    } else if (m_userSpeedFraction) {   //moving
        if (isMoving) { //change speed
            if ((m_activeSpeedFraction == m_userSpeedFraction) and (!m_prevSpeed))
                return;
            //  the times are a bit off when usf < 1.0, but acceptable for now.  will revisit later.    -allan 21Nov15
            float delta = 0.0f;
            logType = 2;
            if (m_decel) {
                m_accel = true;
                m_decel = false;
                m_currentSpeedFraction = 1 - m_activeSpeedFraction;    // reset csf
                m_activeSpeedFraction = m_currentSpeedFraction;
                m_moveTimer = GetTimeMSeconds();    // reset timer
            } else if (m_prevSpeed) {
                // decel from deactivated prop mod
                m_decel = true;
                m_accel = false;
                delta = 1/*m_activeSpeedFraction*/;
            } else if (m_userSpeedFraction < m_activeSpeedFraction) {
                m_decel = true;
                m_accel = false;
                delta = m_activeSpeedFraction - m_userSpeedFraction;
            } else {
                m_accel = true;
                m_decel = false;
                delta = m_userSpeedFraction - m_activeSpeedFraction;
            }

            m_shipMaxAccelTime *= delta;
            m_maxSpeed = m_maxShipSpeed * m_userSpeedFraction;
            // will need a test here for (prevMaxSpeed > m_maxSpeed) to set ship velocity correctly (and avoid negatives)
            if (!m_turning)
                m_velocity = m_shipHeading * m_maxSpeed * m_activeSpeedFraction;
        } else {    //begin movement
            logType = 3;
            m_accel = true;
            m_decel = false;
            m_shipMaxAccelTime *= m_userSpeedFraction;      // for accel with user speeds <= 1.0
            m_maxSpeed = m_maxShipSpeed * m_userSpeedFraction;
            //  see notes in _Move() for information relating to accel equations
            m_currentSpeedFraction = (1 - exp(-0.01 * 1000000 / (m_shipInertia * m_mass)));
            m_velocity = m_shipHeading * m_maxSpeed * m_currentSpeedFraction;
        }
        _log(DESTINY__MOVE_TRACE, "Destiny::_UpdateVelocity - %s(%u): Speed Change - USF: %.2f, ASF: %.2f, CSF: %.2f, PSF: %.2f, prevSpeed: %.2f", \
                 mySE->GetName(), mySE->GetID(), m_userSpeedFraction, m_activeSpeedFraction, m_currentSpeedFraction, m_prevSpeedFraction, m_prevSpeed);
    } else if ((m_currentSpeedFraction) or (m_prevSpeedFraction)) {
        if (isMoving) { //stop movement
            logType = 4;
            // will need a test here for (prevMaxSpeed > m_maxSpeed) to set ship velocity correctly (and avoid negatives)
            //  see notes in _Move() for information relating to decel equations
            m_decel = true;
            m_accel = false;
            m_shipMaxAccelTime *= m_activeSpeedFraction;

            // this is speed from prop mod, which is now deactivated
            if (m_prevSpeed)
                m_maxSpeed = m_prevSpeed * m_activeSpeedFraction;
            else
                m_maxSpeed = m_maxShipSpeed * m_activeSpeedFraction;
            m_velocity = m_shipHeading * m_maxSpeed;
        } else {    //halt
            logType = 5;
            Halt();
        }
    } else {
        //WARNING conditional should never arrive here.
        logType = 6;
        //assert(0);  // change this to a correct error-handling implementation
    }
    std::string msg = "";
    switch (logType) {
        case 1: { msg = "state == warp.  --Begin Decel"; } break;
        case 2: { msg = "USF != 0 and ship isMoving.  --Different Heading or Speed."; } break;
        case 3: { msg = "USF != 0 and ship stopped.  --Begin Accel"; } break;
        case 4: { msg = "USF == 0 and ship isMoving.  --Stop"; } break;
        case 5: { msg = "USF == 0 and ship stopped.  --Halt"; } break;
        case 6: {
            _log(DESTINY__ERROR, "Destiny::_UpdateVelocity Error!  Ship %s(%u) Has No WarpState or Speed Fraction.",
                                    mySE->GetName(), mySE->GetID());
            Halt();
            return;
        } break;
    }
    _log(DESTINY__MOVE_TRACE, "Destiny::_UpdateVelocity - %s(%u):  %s.  AccelTime: %.2f, USF: %.2f, ASF: %.2f, CSF: %.2f, PSF: %.2f", \
            mySE->GetName(), mySE->GetID(), msg.c_str(), m_shipMaxAccelTime, m_userSpeedFraction, m_activeSpeedFraction, m_currentSpeedFraction, m_prevSpeedFraction);
}

//Global Actions:
void DestinyManager::Stop() {
    if (m_stop) return;

    /* AP not implemented yet in this version  -allan 4Mar15
    //Clear autopilot
    if( mySE->HasPilot() )
        mySE->GetPilot()->SetAutoPilot(false);
    */

    if (!m_userSpeedFraction) {
        //state is already at stop. but m_stop wasnt set.
        // set m_stop and return.
        m_stop = true;
        return;
    } else if  ((State == DSTBALL_WARP) and (!IsWarping()))  {
        //warp aborted before initalized.  standard Stop() applies.
        State = DSTBALL_STOP;
    } else if (IsMoving()) {
        //stop called while moving
        // set state to GOTO so _UpdateVelocity() will let us decel correctly
        State = DSTBALL_STOP;
    }

    m_accel = false;
    m_decel = false;
    m_turning = false;
    m_orbiting = 0;

    // reset move timers for new state
    m_moveTimer = GetTimeMSeconds();
    m_stateStamp = sEntityList.GetStamp();

    SetSpeedFraction(0.0f);
    m_stop = true;

    DoDestiny_CmdStop du;
        du.entityID = mySE->GetID();
    PyTuple *tmp = du.Encode();
    SendSingleDestinyUpdate(&tmp);    //consumed
}

void DestinyManager::Halt() {
    /* AP not implemented yet in this version  -allan 4Mar15
     //*Clear autopilot
    if( mySE->HasPilot() )
        mySE->GetPilot()->SetAutoPilot(false);
    */

     if (m_warpState)
         SafeDelete(m_warpState);

    //  reset ALL movement variables and states.  calling this will set object to a COMPLETE and IMMEDIATE stop.
    State = DSTBALL_STOP;
    m_stop = true;
    m_accel = false;
    m_decel = false;
    m_turning = false;
    m_maxSpeed = 0.0f;
    m_velocity = GVector(NULL_ORIGIN);
    m_moveTimer = 0.0;
    m_prevSpeed = 0.0f;
    m_stateStamp = 0;
    m_targetPoint = GPoint(NULL_ORIGIN);
    m_stopDistance = 0;
    m_targetDistance = 0.0;
    m_followDistance = 0.0;
    m_prevSpeedFraction = 0.0f;
    m_userSpeedFraction = 0.0f;
    m_activeSpeedFraction = 0.0f;
    m_currentSpeedFraction = 0.0f;

    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;

    _ClearTurn();

    if ((m_shipMaxAccelTime < 1.0) and (mySE->IsDynamicEntity()))
        m_shipMaxAccelTime = (m_shipAgility * -log(0.0001));

    _log(DESTINY__MOVE_TRACE, "Destiny::Halt() - %s(%u): m_shipHeading: %.3f,%.3f,%.3f", \
        mySE->GetName(), mySE->GetID(), m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);
}

// Global collision methods
//  check for collision.  called by _Move()
void DestinyManager::_CheckBump()
{
    double profileStartTime = 0.0;
    if (sConfig.server.UseProfiling)
        profileStartTime = GetTimeUSeconds();

    //  collision detection code here
    /*  in this case, we are ONLY interested in objects
     *   that have drifted within each others radius (for whatever reason)
     *  this only checks for ships running sub-warp speeds
     *   in relation to other objects in bubble.
     */

    // initial implementation will ONLY check player ships for bumping.
    std::vector<Client*> vPlayers;
    mySE->SysBubble()->GetPlayers(vPlayers);
    Client* pClient = mySE->GetPilot();
    GPoint pos(GetPosition());
    float distance = 0.0f;
    for (auto cur : vPlayers) {
        if (cur == pClient)
            continue;
        distance = pos.distance(cur->GetShipSE()->GetPosition());
        distance -= (mySE->GetRadius() - cur->GetShipSE()->GetRadius());
        if (distance < BUMP_DISTANCE) {
            _Bump(cur->GetShipSE());
            m_bump = true;
        } else
            m_bump = false;
    }
    /** @todo  add data and checks for each ship bumped
     * to give single bump msg for each ship combo
     * without spamming their overview
     */

    if (sConfig.server.UseProfiling)
        sProfile.AddTime(_collisionProfile, GetTimeUSeconds() - profileStartTime);
}

void DestinyManager::_Bump(SystemEntity* pSE)
{
    if (m_bump) return;
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
     *

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

void DestinyManager::_Bounce(GVector direction, float speed)
{
    // bounce code here
    /*  this code will update ship movement after being bumped
     *  all items will drift to a complete stop, unless other movement is called.
     */
    State = DSTBALL_GOTO;
    m_stop = false;
    m_stateStamp = sEntityList.GetStamp();
    m_moveTimer = GetTimeMSeconds();
    m_shipMaxAccelTime = 0.1f;
    m_userSpeedFraction = 1.0f;
    m_currentSpeedFraction = 1.0f;
    m_maxSpeed = m_maxShipSpeed;
    m_velocity = m_shipHeading * m_maxSpeed;

    std::vector<PyTuple*> updates;
    DoDestiny_SetBallVelocity bv;
        bv.entityID = mySE->GetID();
        bv.x = m_velocity.x;
        bv.y = m_velocity.y;
        bv.z = m_velocity.z;
    updates.push_back(bv.Encode());
    DoDestiny_CmdGotoDirection du;
        du.entityID = mySE->GetID();
        du.x = m_shipHeading.x;
        du.y = m_shipHeading.y;
        du.z = m_shipHeading.z;
    updates.push_back(du.Encode());
    SendDestinyUpdate(updates);
    Stop();
}

// main movement method
void DestinyManager::_Move() {
    if (!mySE->SysBubble())
        mySE->SystemMgr()->AddEntity(mySE);

    //apply our velocity to our position for 1 unit of time (a second)

    /* acceleration and deceleration are both logarithmic, and the server needs to keep up with client position.
     * this is another step towards getting ready for collision detection.
     *
     * formula for time taken to accelerate from v to V, from https://wiki.eveonline.com/en/wiki/Acceleration
     *
     *   t=IM(10^-6) * -ln(1-(v/V))
     *
     * as this uses the natural log, the higher the speed, the slower the acceleration, to the limits of ln(0)
     * since lim ln(x) = -INFINITY where x->0+. and ln(0) is undefined, we will use
     *
     *   m_shipMaxAccelTime = (m_shipAgility * -log(0.0001));
     *
     * to define the time it will take a given ship to reach 99.9999% of m_maxShipSpeed, at which point,
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

    /* **UPDATE**  this now uses time AND (m_currentSpeedFraction > 0.999f) for min/max speeds.  -allan 6Aug14
     * **UPDATE**  this is now tracking ALL speed changes correctly.  -allan 21Nov15
     * **UPDATE**  initial orbit implementation.  -allan 13July16
     */

    double timeStamp = 0;   // keep all these timers in seconds.
    if ((m_orbiting != 1) and m_userSpeedFraction)  // if usf==0 then ship is stopping, so continue movement along current ship heading (cancel turn)
        m_shipHeading = _Turn();
    timeStamp = (GetTimeMSeconds() - m_moveTimer) /1000;

    float speed = 0.0f, csf = 0.0f;
    std::string move = "";
    // check to make sure we dont overrun usf/asf
    if (m_activeSpeedFraction == m_userSpeedFraction)
        m_currentSpeedFraction = 1.0f;
    if ((timeStamp > m_shipMaxAccelTime) and (m_currentSpeedFraction > 0.995f)) {
        m_accel = m_decel = false;
        if (m_prevSpeed) {
            speed = m_prevSpeed - ((m_prevSpeed - m_maxSpeed) * m_currentSpeedFraction);
            _log(DESTINY__MOVE_TRACE, "Destiny::_Move() - %s(%u) has deceled from %.2fm/s to %.2fm/s of max %.2f in %.3fs.", \
                    mySE->GetName(), mySE->GetID(), m_prevSpeed, speed, m_maxSpeed, timeStamp);
            m_prevSpeed = 0;
            m_shipMaxAccelTime = timeStamp;
        }
        if (m_userSpeedFraction) {
            // ship has reached full speed (whatever the fraction was set to)
            m_currentSpeedFraction = 1.0f;
            m_activeSpeedFraction = m_userSpeedFraction;
            csf = m_currentSpeedFraction;
            move = "at max speed, going";
            speed = m_maxSpeed * m_activeSpeedFraction;
        } else {
            //ship has reached full stop
            // update position one final time (for last bit of drift) and exit movement functions by calling Halt()
            SetPosition(m_position + m_velocity);
            _log(DESTINY__MOVE_TRACE, "Destiny::_Move() - %s(%u) is at full stop after %.3f seconds.", \
                mySE->GetName(), mySE->GetID(), timeStamp);
            Halt();
            return;
        }
    } else {    //not full speed yet or has changed speed
        /* accel in eve is logarithmic, following a modified bell curve, but capped to a specific time,
         *      based on ship class and maxSubWarpSpeed.
         * this function is defined above, and implemented here.
         */
        if (m_turnTic)
            move = "turning";
        else
            move = "accelerating";

        //if ship is turning, DO NOT reset CSF here until AFTER initial turn.
        //  _Turn() sets initial CSF value based on turn angle.
        if ((!m_turning) or (m_turnTic))  //normal accel
            m_currentSpeedFraction = (1 - exp(-timeStamp * 1000000 / (m_shipInertia * m_mass)));

        csf = m_currentSpeedFraction;
        if (m_accel or m_turning or m_turnTic)
            m_activeSpeedFraction = m_userSpeedFraction * m_currentSpeedFraction;
        else if (m_decel)
            m_activeSpeedFraction = m_prevSpeedFraction * m_currentSpeedFraction;
        else if (m_userSpeedFraction != m_activeSpeedFraction)
            m_activeSpeedFraction = m_currentSpeedFraction;
        else if ((m_tractored) or (m_tractorPause) or (m_activeSpeedFraction == 1))
            ;   // do nothing here.  this is to remove error reporting from next line.
        else
            _log(DESTINY__ERROR, "Destiny::_Move() - %s(%u) is not turning and move checks are not set right.", mySE->GetName(), mySE->GetID());

        if (m_prevSpeed)
            speed = (m_prevSpeed - m_maxSpeed) * m_activeSpeedFraction;
        else
            speed = m_maxSpeed * m_activeSpeedFraction;

        if ((!m_userSpeedFraction) or (m_decel)) {
            //just decelerating
            move = "decelerating";
            //m_activeSpeedFraction = m_currentSpeedFraction;
            if (m_prevSpeed)
                speed = m_prevSpeed - speed;
            else
                speed = m_maxSpeed - speed;
            csf = 1 - m_currentSpeedFraction;
        }
    }

    if (m_orbiting) {
        // object IS orbiting...set orbit speed correctly.
        speed *= m_maxOrbitSpeedFraction;
        move += " in orbit";
    }

    _log(DESTINY__MOVE_TRACE, "Destiny::_Move() - %s(%u) is %s at %.4f m/s (csf:%.4f asf:%.4f sec: %.3f).", \
        mySE->GetName(), mySE->GetID(), move.c_str(), speed, csf, m_activeSpeedFraction, timeStamp);

    //set speed, direction and position for this round of movement
    m_velocity = m_shipHeading * speed;
    SetPosition(m_position + m_velocity);

    _log(DESTINY__MOVE_TRACE, "Destiny::_Move() - %s(%u) Position: %.2f, %.2f, %.2f  velocity: %.3f, %.3f, %.3f", \
            mySE->GetName(), mySE->GetID(), m_position.x, m_position.y, m_position.z, m_velocity.x, m_velocity.y, m_velocity.z);

    if (sConfig.cosmic.BumpEnabled)
        if (mySE->HasPilot() and mySE->SysBubble()->HasPlayers()) // no players in bubble = nothing to check against (for now)
            _CheckBump();

    if (sEntityList.GetTracking()) {
        // create jetcan to visualize object movement
        std::ostringstream str;
        str << "Position Test " << timeStamp;
        ItemData idata(23, 0, mySE->GetLocationID(), flagAutoFit, str.str().c_str(), m_position);
        CargoContainerRef jetCanRef = mySE->GetServices().item_factory->SpawnCargoContainer(idata);
        if (jetCanRef) {
            // create new container
            FactionData jetcanData;
            jetcanData.allianceID = jetcanData.corporationID = jetcanData.factionID = jetcanData.ownerID = 0;
            ContainerSE* cSE = new ContainerSE(jetCanRef, mySE->GetServices(), mySE->SystemMgr(), jetcanData);
            jetCanRef->SetMySE(cSE);
            mySE->SystemMgr()->AddEntity(cSE);
        }
    }
}

/* align time in eve
 * t = -ln(0.25)*A
 * where
 * t = time
 * A = ship agility
 */
bool DestinyManager::_IsTurn() {    //is working.  dont change
    if (m_targetPoint.isZero()) {
        _log(DESTINY__ERROR, "Destiny::_IsTurn() - %s(%u): Target is null.", mySE->GetName(), mySE->GetID());
        m_radians = 0;
        Halt();
        return false;
    }
    // check for turning angle.  returns true if angle is enough to change movement variables
    // create isosceles triangle where legs are current direction and destination, then find angle between legs
    //  it will set m_radians in the range of [-pi,pi].
    GVector toVec(m_position, m_targetPoint);
    toVec.normalize();
    m_targetHeading = toVec;
    float dot = toVec.dotProduct(m_shipHeading);
    if ((dot > 1.0f) or (dot < -1.0f)) {
        sLog.Error("Destiny::_IsTurn()", "%s(%u) - shipHeading has screwed up.  dot is %.5f", \
                    mySE->GetName(), mySE->GetID(), dot);
        Stop();
        return false;
    }
    m_radians = acos(dot);
    float degrees = EvE_RadiansToDegrees(m_radians);
    if ((degrees < TURN_ALIGNMENT) or (m_turnTic > m_alignTime)) {
        m_shipHeading = toVec;
        return false;
    }
    _log(DESTINY__TURN_TRACE, "Destiny::_IsTurn() - %s(%u): dot: %.5f, radians:%.5f, degrees:%.3f",\
                mySE->GetName(), mySE->GetID(), dot, m_radians, degrees);
    //_log(DESTINY__TURN_TRACE, "Destiny::_IsTurn() shipHeading: %.3f,%.3f,%.3f.  targetHeading: %.3f,%.3f,%.3f", \
         m_shipHeading.x, m_shipHeading.y, m_shipHeading.z, m_targetHeading.x, m_targetHeading.y, m_targetHeading.z);
    return true;
}

GVector DestinyManager::_Turn() {
    if (mySE->HasPilot())
        if (mySE->GetPilot()->IsUndock())
            return m_shipHeading;

    if (!_IsTurn()) {
        // was ship turning?  if so, reset movement variables.
        if (m_turning)
            _ClearTurn();
        // no turn or turn was completed. return current heading, and continue movement.
        return m_shipHeading;
    }
    /*when changing directions....
     *  m_moveTimer will have to be reset - call _UpdateVelocity() when turn starts
     *  m_shipHeading will have to be reset - changed in _Move() (our calling function)
     *  m_currentSpeedFraction will have to be reset - updated here, then changed in _Move()
     */

    float turnPercent = 0.5f;

    if (!m_turning) {
        // game seems to delay turning till the tic following any movement function call
        m_turning = true;
        return m_shipHeading;
    } else {
        if (!m_turnTic) {
            // set initial turn movement and speed variables
            // the min speed happens at 0.75s (first tic) into turning
            // turn angle is computed from original turn angle and speed.
            m_turnTic = 1;
            m_currentSpeedFraction = sqrt((cos(m_radians) + 1) /2);  //m_radians is set in _IsTurn() function
            _UpdateVelocity(true);   // reset movement variables for speed change
            turnPercent = 1.0f - m_currentSpeedFraction;    // set this to be percentage of turn based on alignTime
            //  quarter-turn is minimum for all turns....greater angles have sharper initial turns ***HACK***  fix this
            if (turnPercent < 0.25f)
                turnPercent = 0.25f;
            // alignTime = (align time for full 180* turn) * (% of 180* turn being performed)
            m_alignTime = m_timeToEnterWarp * (EvE_RadiansToDegrees(m_radians) /180);
            _log(DESTINY__TURN_TRACE, "Destiny::_Turn() - %s(%u):  %.3fs Turn Time", \
                            mySE->GetName(), mySE->GetID(), m_alignTime);
        } else {
            ++m_turnTic;
        }
    }
    /* get turn radius and use target coords computed earlier to begin turn
     * each tic, ship will turn half of remaining turn.
     * NOTE  all ships turn alike...but at different speeds
     */

    GVector deltaHeading(m_targetHeading - m_shipHeading);
    deltaHeading *= turnPercent;

    GVector shipHeading(m_shipHeading + deltaHeading);
    _log(DESTINY__TURN_TRACE, "Destiny::_Turn() - turnTic: %u, turnPercent: %.3f, shipHeading: %.5f, %.5f, %.5f", \
        m_turnTic, turnPercent, shipHeading.x, shipHeading.y, shipHeading.z);

    return shipHeading;
}

void DestinyManager::_ClearTurn() {
    m_turnTic = 0;
    m_turning = false;
    m_targetHeading = NULL_ORIGIN;
    m_alignTime = m_timeToEnterWarp;
}

void DestinyManager::_Follow() {
    //  Follow is also used by client as AlignTo.
    const GPoint& target_point = m_targetEntity.second->GetPosition();

    GVector heading(m_position, target_point);
    m_targetDistance = (heading.length() - m_radius);

    if (m_targetDistance < m_followDistance) {
    // this will allow following entites to keep their follow state, yet stop movement if within their follow distance.
    //  by keeping their follow state, once the distance is greater than their follow distance, they will begin movement again.
        if (m_tractored) {
            // specific to tractored entities.  sudden halt to mimic tractor stopping
            m_velocity = NULL_ORIGIN_V;
            m_tractorPause = true;
            m_activeSpeedFraction = m_userSpeedFraction = m_currentSpeedFraction = m_prevSpeedFraction = 0.0f;
            std::vector<PyTuple*> updates;
            DoDestiny_CmdSetSpeedFraction ssf;
                ssf.entityID = mySE->GetID();
                ssf.fraction = 0;
            updates.push_back(ssf.Encode());
            SendDestinyUpdate(updates);
            return;
        } else {
            if (m_targetEntity.second->DestinyMgr()->IsMoving()) {
                // this will mimic real movement, where ship will decel instead of a sudden halt
                //  still need to call _Move() here
                SetSpeedFraction(0.2);
            } else {
                Stop();
            }
        }
    } else {
        if (m_tractored and m_tractorPause) {
            // tractored object is outside follow distance.  begin movement again
            m_tractorPause = false;
            m_velocity = m_shipHeading * m_maxSpeed;
            m_moveTimer = GetTimeMSeconds();
            m_stateStamp = sEntityList.GetStamp();
            m_prevSpeedFraction = 0.0f;
            m_activeSpeedFraction = m_userSpeedFraction = m_currentSpeedFraction = 1;
            std::vector<PyTuple*> updates;
            DoDestiny_CmdSetSpeedFraction ssf;
                ssf.entityID = mySE->GetID();
                ssf.fraction = 1;
            updates.push_back(ssf.Encode());
            SendDestinyUpdate(updates);
        } else if (!m_userSpeedFraction) {
            SetSpeedFraction(1.0f);
        }
    }

    heading.normalize();
    m_targetPoint = target_point + (heading * m_targetDistance);

    _Move();
}

void DestinyManager::_Orbit() {
    // data consitency checks...
    if ((m_targetDistance > BUBBLE_RADIUS_METERS) or (m_followDistance > BUBBLE_RADIUS_METERS)) {
        // well, something fucked up.  stop object and throw error.   player can reset if they want to.
        if (mySE->HasPilot())
            mySE->GetPilot()->SendErrorMsg("Internal Server Error.  Ref: ServerError 35412");
        sLog.Error("Destiny::_Orbit()", "%s(%u) - Distance check OOB. ", mySE->GetName(), mySE->GetID());
        Stop();
        return;
    }
    #define LogMacro(v) _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - " #v ": (%.3f, %.3f, %.3f)   len=%.3f", v.x, v.y, v.z, v.length())

    // this should ONLY set position of ship based on period of orbit.

    /*   destiny variables used here
     * m_position - probably the most important calculated value.
     * m_velocity - tied for above title
     * m_followDistance - calculated orbit distance, including target gravity and ship variables
     * m_targetDistance - commanded orbit distance
     * m_targetHeading - direction to target from current heading
     * m_targetPoint - calculated distant point from above variable
     * m_shipHeading - current direction ship is pointed
     * m_stateStamp - statestamp of when current state began, in seconds
     * m_moveTimer - millisecond timer to calculate velocity
     * m_orbiting - 0=no orbit, >0=in orbit, 1=at distance 2=way too close , 3=too close, 4=too far, 5=way too far
     * m_orbitRadTic - rad/sec in current orbit.  set by Orbit()
     * m_maxOrbitSpeedFraction - calculated max speed to maintain commanded orbit distance.  set in Orbit() but not used here
     *
     *   orbit variables used here
     * Tr = target radius
     * Tp = target position
     * Tv = target velocity
     * Td = target distance
     * Th = target heading
     * Cd = current direction*
     *
     * current = distance between object and target centers
     * actual = current distance minus radius of object and target
     * near = fraction of actual / m_targetDistance
     * far = fraction of actual / m_followDistance
     */

    // get current times
    double timeStamp = (GetTimeMSeconds() - m_moveTimer) /1000;
    float Tr =  m_targetEntity.second->GetRadius();
    GPoint Tp(m_targetEntity.second->GetPosition());
    // only dynamic entites have a destiny manager, others will return 0.
    //  this still needs testing
    float Tv = (m_targetEntity.second->DestinyMgr() ? m_targetEntity.second->DestinyMgr()->GetSpeed() : 0);
    GVector Th(m_targetEntity.second->DestinyMgr() ? m_targetEntity.second->DestinyMgr()->GetHeading() : NULL_ORIGIN_V);
    Tp += (Tv*Th); // use Tv*Th and add to position to account for target movement.  Tv for non-moving targets return 0.

    // current and actual are used to determine ship's orbit distance, and adjust position accordingly
    double current = m_position.distance(Tp);
    double actual = (current - m_radius - Tr);
    float near = actual / m_targetDistance, far = actual / m_followDistance;
    _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - current:%.2f, actual:%.2f, target:%.2f, follow:%.2f, near:%.3f, far:%.3f", \
            current, actual, m_targetDistance, m_followDistance, near, far);

    // distances for orbit calculations for orbits within engage distance
    double orbitDistNow = 0.0f, orbitDistNext = 0.0f, curSpeed = m_maxSpeed * m_activeSpeedFraction * m_maxOrbitSpeedFraction;

    GPoint mPos = NULL_ORIGIN;
    double curRad = m_orbitRadTic * timeStamp;  // this isnt quite right...but pretty damn close
    // adjust 'distance' variable as needed to correct orbit circumfrence based on target distance
    if (far > 2.5) { //far = actual / m_followDistance
        // too far to engage target.
        _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - way too far");
        m_orbiting = 5;
        // set point to side of target (based on current position), to avoid near-zero angular velocity
        double radTarg = atan2(Tp.z - m_position.z, Tp.x - m_position.x);  // rad from '0' to target
        radTarg += atan2(m_targetDistance, current);  // rad from 'distance line' to target 'offset'
        mPos.x = current * cos(radTarg);
        mPos.z = current * sin(radTarg);
        m_targetPoint = m_position + mPos;
        m_targetPoint.y = Tp.y;     // stay on 'y' elevation...easier this way.
        GVector heading(m_position, m_targetPoint);
        heading.normalize();
        m_shipHeading = heading;    // this sets object velocity in _Move() (using speed)
        _Move();
        return; // this is all we need to do at this point.
    } else if (near < 0.65) {  // near = actual / m_targetDistance
        // way too close inside orbit.  move away quickly.
        _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - way too close");
        m_orbiting = 2;
        GVector heading(m_position, Tp);
        heading.normalize();
        m_shipHeading = heading * -1;    // this sets object velocity in _Move() (using speed)
        m_targetPoint = m_position + (m_shipHeading * 1.0e16);
        _Move();
        return; // this is all we need to do at this point.

    /**  @note these still need to determine ship position to properly set quadrant. (I, II, III, IV)
     *  right now, once ship is within orbit tolerance, we reset radian position to 0
     *  which will reset ship's orbit quadrant to I.
     */
    // these below are within engage distance.
    /** @note these should use a bit of trig to calculate true position, but im lazy, so will hack it for now.  will have to revisit later */
    } else if (far > 1.0) {     //far = actual / m_followDistance
        // too far outside orbit.  move closer
        _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - too far");
        m_orbiting = 4;
        // simulate orbiting distance here...the distance isnt a straight line, so we need to fudge it as ship will be trying for a larger orbit
        float test = actual - (curSpeed *0.75);
        if (test < m_targetDistance)
            orbitDistNow = actual - (m_followDistance - m_targetDistance);  // subtract the difference of follow and target from actual distance
        else
            orbitDistNow = actual - curSpeed;
        // determine distance for next tic based on ship speed and position
        orbitDistNext = orbitDistNow - test;
    } else if (near < 1.0) {    // near = actual / m_targetDistance
        // too close inside orbit; move away slowly.
        _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - too close");
        m_orbiting = 3;
        float test = actual - (curSpeed *0.75);
        if (test > m_followDistance)
            orbitDistNow = actual + (m_followDistance - m_targetDistance);    // add the difference of follow and target from actual distance
        else
            orbitDistNow = actual + curSpeed;
        // determine distance for next tic based on ship speed and position
        orbitDistNext = orbitDistNow + test;
    } else {
        // within orbit distance tolerance
        _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - within tolerance");
        if (m_orbiting != 1)
            m_moveTimer = GetTimeMSeconds();
        m_orbiting = 1;
        orbitDistNow = orbitDistNext = m_followDistance;  // this is calculated orbit distance for this ship
    }

    // distance was checked and adjusted as needed for this tic.
    // use orbit math to set ship position and _Move() to set other attribs.

    // set current position (this is where we are this tic)
        /** @todo need more info before i can get this working correctly.  use flat orbit for now
         *  GPoint mPosAdj = NULL_ORIGIN;
        double radX = m_position.x - Tp.x + mPosAdj.x, radY = m_position.y - Tp.y + mPosAdj.y, radZ = m_position.z - Tp.z + mPosAdj.z;
        mPos.x = radX * cos(curRad) + radZ * sin(curRad);
        double intmZ = radZ * cos(curRad) - radX * sin(curRad);
        mPos.z = intmZ * cos(curRad) + radY * sin(curRad);
        mPos.y = radY * cos(curRad) - intmZ * sin(curRad);
        _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit()  rad: %.3f, %.3f, %.3f  intmZ:%.3f  mposition: %.3f, %.3f, %.3f",radX, radY, radZ, intmZ, mPos.x, mPos.y, mPos.z);
        */
    _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - orbiting. curRad:%.5f, timestamp:%.3f, speed:%.2f", curRad, timeStamp, curSpeed);

    mPos.x = orbitDistNow * cos(curRad);
    mPos.z = orbitDistNow * sin(curRad);
    mPos.y = 0; // flat horizontal orbit until i can get the math right for 'true' orbit
    SetPosition(Tp + mPos);

    // set current heading as vector from current location to (calculated supposed) location on next tic
    curRad += m_orbitRadTic;
    /*
    mPos.x = radX * cos(curRad) + radZ * sin(curRad);
    intmZ = radZ * cos(curRad) - radX * sin(curRad);
    mPos.z = intmZ * cos(curRad) + radY * sin(curRad);
    mPos.y = radY * cos(curRad) - intmZ * sin(curRad);
    */
    mPos.x = orbitDistNext * cos(curRad);
    mPos.z = orbitDistNext * sin(curRad);
    //mPos.y = 0;
    GVector heading(m_position, Tp + mPos);
    heading.normalize();
    m_shipHeading = heading;    // this sets object velocity in _Move() (using speed)
    m_targetPoint = m_position + (m_shipHeading * 1.0e16);

    _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - heading: %.3f, %.3f, %.3f", m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);

    _Move();
}

void DestinyManager::_InitWarp() {
    //init warp:

    // warp time and distance math
    //   allan 1Nov14 - 14Nov14
    //  rewrite 3jan15  to use distance instead of time for warping.  more accurate now, and covers ALL distances.
    //  calculation and implementation update   9Jan15      accuracy is within 1000m

    /*  my research into warp formulas, i have used these sites, with a few exerpts and ideas from each...
     *     https://wiki.eveonline.com/en/wiki/Acceleration
     *     http://oldforums.eveonline.com/?a=topic&threadID=1251486
     *     http://gaming.stackexchange.com/questions/115271/how-does-one-calculate-a-ships-agility
     *     http://eve-search.com/thread/1514884-0
     *     http://www.eve-search.com/thread/478431-0/page/1
     */

    /* this is my version of how warp should be timed and followed by the server.
     * checks here for distance < warp speed and distance < 2AU, (with all distances in meters)
     *  and adjusts accel/decel times accordingly
     *
     *   accel/decel are logrithmic per ccp (see above).
     *
     * times are as follows, per this table.  http://cdn1.eveonline.com/www/newssystem/media/65418/1/numbers_table.png
     *
     * all warps are in same time groups for all ships, except freighters and caps.
     * distance checks are seperated into 3 time groups, with subgroups for freighters and caps.
     *
     * the client seems to accept and agree with the math here.
     */

    _log(DESTINY__WARP_TRACE, "Destiny::_InitWarp(): %s(%u) has initialized warp.", mySE->GetName(), mySE->GetID());

    bool cruise = false;
    double warpSpeedInMeters = (m_shipWarpSpeed * ONE_AU_IN_METERS);
    /** @todo  ALL of these will need checks for ship speed and distance.  current categories are not enough play. */
    if (m_targetDistance < 1000000001) {
        // < 1MKm, total time for inty is 15s.
        m_warpAccelTime = 6;
        m_warpDecelTime = 13;
    } else if (m_targetDistance < ONE_AU_IN_METERS) {
        // this covers 1MKm to 1AU
        // at 1MKm, total time for all ships except freighters is 23s.
        // freighters base time is 29s, and all other freighter warp distances are covered here.
        if (m_targetDistance > warpSpeedInMeters) {
            m_warpAccelTime = 8;
            m_warpDecelTime = 20; //21
        } else {
            m_warpAccelTime = 7;
            m_warpDecelTime = 16;
        }
    } else if (m_targetDistance < (ONE_AU_IN_METERS * 2)) {
        // this covers between 1AU and 2AU, and excludes freighters.
        // at 2AU, total time for all other ships except caps is 29s.
        // capitals base time is 30s, and all other capital warp distances are covered here
        if (m_targetDistance > warpSpeedInMeters) {
            m_warpAccelTime = 9;
            m_warpDecelTime = 22;  //21
        } else {
            m_warpAccelTime = 8;
            m_warpDecelTime = 20;  //21
        }
    } else {
        // this covers all other ships (except freighters and capitals) at all distances > 2AU.
        // for all distances > 2AU, 30s is base time.
        m_warpAccelTime = 9;
        m_warpDecelTime = 23;  //21
    }

    if (m_targetDistance > warpSpeedInMeters) cruise = true;

    /*  this is from http://community.eveonline.com/news/dev-blogs/warp-drive-active/
     * For the acceleration phase, k is 3.
     * For deceleration, k is 1.
     * x = e^(k*t)
     * x = distance in meters
     * t = time in seconds
     *
     * this gives distances as functions of time.
     *  the client seems to agree with this reasoning, and follows the same idea.
     *
     * short warps are different.  they do not agree with the time function, so i have
     * to do them different, and go by distance.
     */
    double accelDistance = 0.0, decelDistance = 0.0, cruiseDistance = 0.0;
    float cruiseTime = 0.0f, warpTime = 0.0f;

    //  set distances
    /*  NOTE:  distances are inverse of times....the following are CORRECT.
     * do not change.  -allan
     */
    if (cruise) {                                   //  short         long
        accelDistance = exp(m_warpDecelTime);       //  2980.957    1.3188e9
        decelDistance = exp(3 * m_warpAccelTime);   //  8103.083    5.3204e11
        cruiseDistance = (m_targetDistance - accelDistance - decelDistance);
        cruiseTime = (cruiseDistance / warpSpeedInMeters);
    } else {
        //  short warp....no cruise
        accelDistance = (m_targetDistance /3);
        decelDistance = (m_targetDistance - accelDistance);
        warpSpeedInMeters = accelDistance;
    }

    //  set total warp time based on above math.
    warpTime =  (m_warpAccelTime + m_warpDecelTime + floor(cruiseTime));

    GVector warp_vector(m_position, m_targetPoint);
    warp_vector.normalize();

    _log(DESTINY__WARP_TRACE, "Destiny::_InitWarp():Calculate - %s(%u): Warp will accelerate for %us, cruise for %.3f, then decelerate for %us, with total time of %.3fs, and warp speed of %.4f m/s.", \
            mySE->GetName(), mySE->GetID(), m_warpAccelTime, cruiseTime, m_warpDecelTime, warpTime, warpSpeedInMeters);
    _log(DESTINY__WARP_TRACE, "Destiny::_InitWarp():Calculate - %s(%u): Accel distance is %.4f. Cruise distance is %.4f.  Decel distance is %.4f.  Direction is %.3f,%.3f,%.3f.", \
            mySE->GetName(), mySE->GetID(), accelDistance, cruiseDistance, decelDistance, warp_vector.x, warp_vector.y, warp_vector.z);
    _log(DESTINY__WARP_TRACE, "Destiny::_InitWarp():Calculate - %s(%u): We will exit warp at %.2f,%.2f,%.2f at a distance of %.4f AU (%.4fm).", \
            mySE->GetName(), mySE->GetID(), m_targetPoint.x, m_targetPoint.y, m_targetPoint.z, m_targetDistance/ONE_AU_IN_METERS, m_targetDistance);
    GPoint destination = m_position + (warp_vector * m_targetDistance);
    _log(DESTINY__WARP_TRACE, "Destiny::_InitWarp():Calculate - %s(%u): calculated exit is %.2f,%.2f,%.2f and vector is %.4f,%.4f,%.4f.", \
            mySE->GetName(), mySE->GetID(), destination.x, destination.y, destination.z, warp_vector.x, warp_vector.y, warp_vector.z);

    //  reset deceltime for time check in _WarpDecel()
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

    //drain cap
    if (mySE->HasPilot()) {
        mySE->GetPilot()->GetShip()->SetAttribute(AttrCapacitorCharge, m_capNeeded);
        mySE->GetPilot()->GetShip()->Warp();
    }

    //clear targets
    mySE->TargetMgr()->ClearTargets();
    _WarpAccel(0);
}

void DestinyManager::_WarpAccel(uint16 sec_into_warp) {
    /* For acceleration, k = 3.
     * distance = e^(k*s)
     * speed = k*e^(k*s)
     */
    double currentDistance = exp(3 * sec_into_warp);
    double currentShipSpeed = (3 * exp(3 * sec_into_warp));

    if (currentDistance > m_warpState->accelDist) {
        currentDistance = m_warpState->accelDist;
        m_warpState->accel = false;
        if (m_warpState->cruiseDist > 0)
            m_warpState->cruise = true;
        else
            m_warpState->decel = true;
    }
    m_targetDistance -= currentDistance;

    _WarpUpdate(currentShipSpeed);

    _log(DESTINY__WARP_TRACE, "Destiny::_WarpAccel(): %s(%u) - Warp Accelerating(%us): velocity %.4f m/s with %.4f m left to go.  Current distance %.4f.", \
            mySE->GetName(), mySE->GetID(), sec_into_warp, currentShipSpeed, m_targetDistance, currentDistance);
}

void DestinyManager::_WarpCruise(uint16 sec_into_warp) {
    /* in cruise....calculate distance only to update internal position data. */
    m_targetDistance -= m_warpState->warpSpeed;

    if ((m_targetDistance - m_warpState->warpSpeed) < m_warpState->decelDist) {
        m_warpState->cruise = false;
        m_warpState->decel = true;
    }

    _WarpUpdate(m_warpState->warpSpeed);

    _log(DESTINY__WARP_TRACE, "Destiny::_WarpCruise(): %s(%u) - Warp Crusing(%us): velocity %.4f m/s. with %.4f m left to go.", \
             mySE->GetName(), mySE->GetID(), sec_into_warp, m_warpState->warpSpeed, m_targetDistance);
}

void DestinyManager::_WarpDecel(uint16 sec_into_warp) {
    /* For deceleration, k = -1.
     * distance = e^(k*s)
     * speed = -k*e^(k*s)
     */
    uint8 decelTime = (sec_into_warp - m_warpDecelTime);
    double currentDistance = (m_warpState->total_distance - (exp(-decelTime) * m_warpState->decelDist));
    m_targetDistance = (m_warpState->total_distance - currentDistance);
    double currentShipSpeed = (m_warpState->warpSpeed * exp(-decelTime));

    if (currentShipSpeed <= m_speedToLeaveWarp) {
        // stop warp, and return to normal(stop) mode
        _WarpUpdate(currentShipSpeed);
        _WarpStop(currentShipSpeed);
        return;
    }
    _WarpUpdate(currentShipSpeed);

    _log(DESTINY__WARP_TRACE, "Destiny::_WarpDecel(): %s(%u) - Warp Decelerating(%us/%us): velocity %.4f m/s with %.4f m left to go.", \
             mySE->GetName(), mySE->GetID(), decelTime, sec_into_warp, currentShipSpeed, m_targetDistance);
}

void DestinyManager::_WarpUpdate(double currentShipSpeed) {
    //  update position and velocity for all stages.
    //  this method is ~1000m off actual.  could be due to rounding.   -allan 9Jan15
    m_position = (m_targetPoint - (m_warpState->warp_vector * m_targetDistance));
    m_velocity = (m_warpState->warp_vector * currentShipSpeed);

    SetPosition(m_position);

    if (m_inBubble) {
        if (m_warpState->accel)
            if (!mySE->SysBubble()->InBubble(m_position)) {
                sBubbleMgr.Remove(mySE);
                m_inBubble = false;
            }
    } else if (!m_inBubble and m_warpState->decel) {
        if (m_targetDistance < BUBBLE_RADIUS_METERS) {    //this assumes target is center of bubble.  will have to fix one day.
            _log(DESTINY__WARP_TRACE, "Destiny::_WarpUpdate(): %s(%u): Ship at %.2f,%.2f,%.2f is calling Add() .", \
                    mySE->GetName(), mySE->GetID(), m_position.x, m_position.y, m_position.z);
            sBubbleMgr.Add(mySE, true);
            SetPosition(m_position, true);
            m_inBubble = true;
        }
    }
}

void DestinyManager::_WarpStop(double currentShipSpeed) {
    _log(DESTINY__WARP_TRACE, "Destiny::_WarpStop(): %s(%u) - Warp complete. Exit velocity %.4f m/s with %.4f m left to go.", \
            mySE->GetName(), mySE->GetID(), currentShipSpeed, m_targetDistance);
    _log(DESTINY__WARP_TRACE, "Destiny::_WarpStop(): %s(%u): Ship currently at %.2f,%.2f,%.2f.", \
            mySE->GetName(), mySE->GetID(), m_position.x, m_position.y, m_position.z);

    m_targetPoint += (m_warpState->warp_vector *10000);
    //SetPosition(m_position, false);
    // SetSpeedFraction() checks for State = Warp and warpstate != null to set decel variables correctly with warp decel.
    //   have to call this BEFORE deleting or reseting State or WarpState.
    SetSpeedFraction(0.0f);
    m_stop = true;
    SafeDelete(m_warpState);
}

//called whenever an entity is going away and can no longer be used as a target
void DestinyManager::EntityRemoved(SystemEntity *pSE) {
    if (m_targetEntity.second == pSE) {
        m_targetEntity.first = 0;
        m_targetEntity.second = nullptr;

        switch(State) {
            case DSTBALL_FOLLOW:
            case DSTBALL_ORBIT: {
                _log(DESTINY__DEBUG, "%u: Our target entity has gone away. Stopping.", mySE->GetID());
                Stop();
            } break;
        }
    }
}

bool DestinyManager::IsTargetInvalid()
{
    if (!mySE->SystemMgr()->GetSE(m_targetEntity.first)) {
        // Our target was removed
        Stop();
        return true;
    }
    if (!m_targetEntity.second->IsDynamicEntity())
        return false;
    if (m_targetEntity.second->HasPilot()) {
        if (m_targetEntity.second->GetPilot()->IsDocked()) {  // Our target docked, so STOP
            mySE->TargetMgr()->ClearTarget(m_targetEntity.second);
            Stop();
            return true;
        }
    }
    if (m_targetEntity.second->DestinyMgr()->IsWarping()) { // The target is warping
        mySE->TargetMgr()->ClearTarget(m_targetEntity.second);
        Stop();
        return true;
    }
    return false;
}

// Basic Movement Calls:
void DestinyManager::_BeginMovement() {
    // common movement for all types
    if (!m_hasSentShipUpdates) {
        // error fix for setting ship move variables before ship is in bubble (cannot BubbleCast)
        std::vector<PyTuple*> updates;
        DoDestiny_SetBallAgility sbagility;
            sbagility.entityID =  mySE->GetID();
            sbagility.agility = m_shipInertia;
        updates.push_back(sbagility.Encode());
        DoDestiny_SetBallMassive sbmassive;
            sbmassive.entityID = mySE->GetID();
            sbmassive.is_massive = true;
        updates.push_back(sbmassive.Encode());
        DoDestiny_SetBallMass sbmass;
            sbmass.entityID = mySE->GetID();
            sbmass.mass = m_mass;
        updates.push_back(sbmass.Encode());
        SendDestinyUpdate(updates); //consumed
        m_hasSentShipUpdates = true;
    }

    // reset turn and movement checks for possible heading change.
    m_stop = false;
    m_accel = false;
    m_decel = false;
    m_turning = false;
    if (!mySE->IsNPCSE() or (mySE->IsNPCSE() and mySE->GetNPCSE()->GetAIMgr()->IsIdle())) {
        m_moveTimer = GetTimeMSeconds();
        m_stateStamp = sEntityList.GetStamp();
    }
    if (m_shipHeading.isZero()) {
        GVector point(m_position);
        point.normalize();
        GVector moveVector(m_position, (point * 1.0e16));
        moveVector.normalize();
        m_shipHeading = moveVector;
    }

    if (!m_orbiting) {
        // reset target distance just in case it changed.
        GVector shipVector(m_position, m_targetPoint);
        m_targetDistance = shipVector.length();
    }

    // if ship is moving, reset accel checks
    if (m_userSpeedFraction)
        SetSpeedFraction(m_userSpeedFraction, true);
    else
        SetSpeedFraction(1.0f, true);

    if (IsCloaked())
        UnCloak();

    _Move();
}

void DestinyManager::Follow(SystemEntity* pSE, double distance) {
    //called from client as 'CmdFollowBall'
    //  also used by 'Approach'
    if ((State == DSTBALL_FOLLOW) and (m_targetEntity.second == pSE) and (m_followDistance == distance) and (m_userSpeedFraction))
        return;
    if (m_orbiting) {
        m_orbiting = 0;
        m_shipHeading = NULL_ORIGIN_V;
    }

    State = DSTBALL_FOLLOW;
    m_targetPoint = pSE->GetPosition();
    m_targetEntity.first = pSE->GetID();
    m_targetEntity.second = pSE;
    m_followDistance = distance;
    if (pSE->IsStationSE()) {
        // set target position to dock of station....NOT in the middle of the fucking thing.
        StationData sData;
        sDataMgr.GetStationInfo(pSE->GetID(), sData);
        m_targetPoint = sData.dockPosition;
    }
    _BeginMovement();

    DoDestiny_CmdFollowBall du;
        du.entityID = mySE->GetID();
        du.targetID = pSE->GetID();
        du.range = (int32)distance;
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);    //consumed
}

void DestinyManager::Orbit(SystemEntity *pSE, double distance/*0*/) {
    if ((State == DSTBALL_ORBIT) and (m_targetEntity.second == pSE) and (m_followDistance == distance))
        return;
    if (m_orbiting)
        m_shipHeading = NULL_ORIGIN_V;

    /* this inital Orbit() call will, based on position delta, determine the orbit plane, rotation (cw/ccw)
     *  initial heading, actual orbit radius, actual orbit velocity, and some other shit i havent thought about yet.
     *
     * m_targetPoint    - updated in _Orbit()
     * m_shipHeading    - updated in _Orbit()
     * m_targetEntity   - SE object to orbit
     * m_targetDistance - commanded orbit distance
     * m_followDistance - calculated orbit distance based on mass, velocity, gravity, etc...
     * m_stateStamp (via _BeginMovement())
     * speed fractions (usf, csf, asf - via SetSpeedFraction() to begin or alter speed)
     * m_maxOrbitSpeedFraction - calculated max speed to maintain commanded orbit distance.  set in Orbit()
     */
    State = DSTBALL_ORBIT;
    m_orbiting = 1;
    m_targetEntity.first = pSE->GetID();
    m_targetEntity.second = pSE;
    m_targetPoint = pSE->GetPosition();
    m_targetDistance = distance;
    _BeginMovement();

    _log(DESTINY__ORBIT_TRACE, "Destiny::Orbit() - Ship Data - agility:%.3f, inertia:%.3f, massMkg:%.3f, maxSpeed:%.2f, radius:%.2f", \
                m_shipAgility, m_shipInertia, m_massMKg, m_maxShipSpeed, m_radius);

    // Target (orbited object)
    double Tr = pSE->GetRadius();
    double Tm = pSE->GetSelf()->GetAttribute(AttrMass).get_float();
    if (!Tm)
        Tm = pSE->GetSelf()->type().mass();

    _log(DESTINY__ORBIT_TRACE, "Destiny::Orbit() - Target Data - mass:%.3f, speed:%.2f, radius:%.2f", \
                Tm, (pSE->DestinyMgr() ? pSE->DestinyMgr()->GetSpeed() : 0 ), Tr);

    // fudge distance to work 'close enough' with all targets...this was trial-n-error
    double Rc = ((distance + 150 + m_radius - (pSE->GetRadius() /12)) * 1.2);
    double Rc2 = pow(Rc,2);
    double Vm2 = pow(m_maxShipSpeed,2);
    double t2 = pow(m_shipAgility,2);

    // the following equation is from "Ship Motion in Eve Online" by Scheulagh Santorine, Ph.D
    // radius needs target mass and grav const factored in....somehow.
    // orbit radius
    /* r = sqrt(6 * cbrt(108t^2*Vm^2 * Rc^2 + 8Rc^6 + 12sqrt(81t^4 *Vm^4 + 12t^2 * Vm^2 * Rc^10))
     * + (24Rc^4 / (108t^2 * Vm^2 * Rc^2 + 8Rc^2 + 12sqrt(81t^4 * Vm^4 * Rc^8 + 12t^2 * Vm^2 * Rc^10)^1/3)) + 12Rc^2) /6
     */
    double one = (108 * t2 * Vm2 * Rc2);
    double two = (12 * t2 * Vm2 * pow(Rc,10));
    double three = (12 * sqrt(81 * pow(m_shipAgility,4) * pow(m_maxShipSpeed,4) + two));
    double four = (6 * cbrt(one + 8 * pow(Rc,6) + three));
    double five = cbrt(sqrt(three * pow(Rc,8) + two));
    double six = (one + (8 * Rc2) + (12 * five));
    m_followDistance = sqrt(four + (24 * pow(Rc, 4) / six) + 12 * Rc2) / 6;

    double velocity = m_maxShipSpeed * ((distance / m_followDistance) + 0.065); // dunno where i got this from...
    m_maxOrbitSpeedFraction = velocity / m_maxShipSpeed;

    double circ = 2 * EvE_Pi * m_followDistance;
    double orbitTime = circ / velocity;
    m_orbitRadTic = (2 * EvE_Pi) / orbitTime;

    _log(DESTINY__ORBIT_TRACE, "Destiny::Orbit() - Orbit Data - Rc:%.3f, velocity:%.2f, osf:%.2f, targetDistance:%.2f, followDistance:%.2f, orbitTime:%.1f, radTic:%.5f", \
                Rc, velocity, m_maxOrbitSpeedFraction, m_targetDistance, m_followDistance, orbitTime, m_orbitRadTic);

    double current = m_position.distance(pSE->GetPosition());
    double actual = (current - m_radius - Tr);
    // m_orbiting - 0=no orbit, >0=in orbit, 1=at distance 2=way too close , 3=too close, 4=too far, 5=way too far
    if ((actual - m_targetDistance) > m_followDistance) {
        // too far to engage target.
        m_orbiting = 5;
    } else if (current > m_followDistance) {
        // too far outside orbit.  move closer
        m_orbiting = 4;
    } else if (actual < m_targetDistance) {
        // way too close inside orbit.  move away quickly.
        m_orbiting = 2;
    } else if (current < m_targetDistance) {
        // too close inside orbit; move away slowly.
        m_orbiting = 3;
    } else {
        // within orbit distance tolerance
        m_orbiting = 1;
    }

    DoDestiny_CmdOrbit du;
        du.entityID = mySE->GetID();
        du.orbitEntityID = pSE->GetID();
        du.distance = (int32)actual;
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);    //consumed
}

void DestinyManager::AlignTo(SystemEntity* ent) {
    Follow(ent, 0);
}

void DestinyManager::GotoDirection(const GPoint& direction) {
    if (m_orbiting) {
        m_orbiting = 0;
        m_shipHeading = NULL_ORIGIN_V;
    }

    State = DSTBALL_GOTO;
    m_targetPoint = direction *1.0e16;
    _BeginMovement();

    DoDestiny_CmdGotoDirection du;
        du.entityID = mySE->GetID();
        du.x = direction.x;
        du.y = direction.y;
        du.z = direction.z;
    PyTuple* up = du.Encode();
    SendSingleDestinyUpdate(&up);    //consumed
}

void DestinyManager::GotoPoint(const GPoint& point) {
    if (m_orbiting) {
        m_orbiting = 0;
        m_shipHeading = NULL_ORIGIN_V;
    }

    State = DSTBALL_GOTO;
    m_targetPoint = point;
    _BeginMovement();

    DoDestiny_GotoPoint gtpoint;
        gtpoint.entityID = mySE->GetID();
        gtpoint.x = m_targetPoint.x;
        gtpoint.y = m_targetPoint.y;
        gtpoint.z = m_targetPoint.z;
    PyTuple* up = gtpoint.Encode();
    SendSingleDestinyUpdate(&up);    //consumed
}

void DestinyManager::WarpTo(const GPoint& where, int32 distance) {
    /* warp order..
     * pick destination -> align/accel -> aura "warp drive active" -> cap drain -> accel
     *      -> enter warp -> warp -> decel -> leave warp -> coast -> stop
     */
    GotoPoint(where);
    if (m_warpState)
        SafeDelete(m_warpState);

    /** @todo (allan) finish warp scramble system */
    if (m_warpStrength < 0/*m_warpScrambleSrength*/)
        if (mySE->HasPilot() and mySE->GetPilot()->CanThrow())
            throw PyException(MakeUserError("WarpScrambled"));

    m_stopDistance = distance;
    GVector warp_distance(m_position, where);
    m_targetDistance = warp_distance.length();
    m_targetDistance -= m_stopDistance;

    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;

    if (mySE->IsNPCSE()) {
        State = DSTBALL_WARP;

        // send client updates
        std::vector<PyTuple*> updates;
        DoDestiny_CmdWarpTo wt;
            wt.entityID = mySE->GetID();
            wt.dest_x = m_targetPoint.x;
            wt.dest_y = m_targetPoint.y;
            wt.dest_z = m_targetPoint.z;
            wt.distance = m_stopDistance;
            wt.warpSpeed = GetWarpSpeed();
        updates.push_back(wt.Encode());
        DoDestiny_OnSpecialFX10 sfx;
            sfx.guid = "effects.Warping";
            sfx.entityID = mySE->GetID();
            sfx.isOffensive = false;
            sfx.start = true;
            sfx.active = true;
        updates.push_back(sfx.Encode());
        SendDestinyUpdate(updates);
        _log(NPC__MESSAGE, "Destiny::WarpTo() NPC  m_targetPoint: %.2f,%.2f,%.2f  m_stopDistance: %i  m_targetDistance: %.4f",
             m_targetPoint.x, m_targetPoint.y, m_targetPoint.z, m_stopDistance, m_targetDistance);
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

    if (m_targetDistance < minWarpDistance) {
        // warp distance too close.  cancel warp and return
        if(mySE->HasPilot())
            mySE->GetPilot()->SendErrorMsg("That is too close for your Warp Drive.");

        State = DSTBALL_STOP;
        SafeDelete(m_warpState);
        return;
    }

    if (mySE->HasPilot()) {
        Client *pClient = mySE->GetPilot();
        ShipItemRef pShip = pClient->GetShip();

        /*  capacitor for warp forumlas from https://oldforums.eveonline.com/?a=topic&threadID=332116
         *  Energy to warp = warpCapacitorNeed * mass * au * (1 - warp_drive_operation_skill_level * 0.10)
         */
        double currentShipCap = pShip->GetAttribute(AttrCapacitorCharge).get_float();
        double adjDistance = m_targetDistance / ONE_AU_IN_METERS; // change distance in meters to AU.
        double capNeeded = m_mass * m_warpCapacitorNeed * adjDistance;
        capNeeded *= (1 - (0.1 *(pClient->GetChar()->GetSkillLevel(skillWarpDriveOperation, true))));

        //  check if ship has enough capacitor to warp full distance
        if (capNeeded > currentShipCap) {
            /** @todo (allan) this is wrong...
             * //  nope...enough cap to min warp?
             *        double cap_check = ((150000 /ONE_AU_IN_METERS) * m_mass * adjWarpCapNeed);
             *        if (cap_check > 0) {
             *          //reset distance based on avalible capacitor
             *            // adjust warp distance based on cap left.   NPC's are not restricted by this. (or wont be in EvEmu)
             *              m_stopDistance = currentShipCap / (m_mass * adjWarpCapNeed);
             *              m_targetPoint = NULL_ORIGIN;
             *            capNeeded = currentShipCap;
        } else {*/
            pClient->SendErrorMsg("You don't have enough capacitor charge to warp.");
            _log(DESTINY__WARNING, "Destiny::WarpTo() - %s(%u): Capacitor needed vs current  %.3f / %.3f",
                 mySE->GetName(), mySE->GetID(), capNeeded, currentShipCap);

            State = DSTBALL_STOP;
            SafeDelete(m_warpState);
            return;
            //}
        } else {
            capNeeded = currentShipCap - capNeeded;
        }

        m_capNeeded = capNeeded;
    }

    /*  TODO PUT CHECK HERE FOR WARP BUBBLES
     *     and other things that affect warp-in point.....when we get to there.
     * AttrWarpBubbleImmune = 1538,
     * AttrWarpBubbleImmuneModifier = 1539,
     *   NOTE:  warp bubble in path (or within 100km of m_targetPoint) will change m_targetDistance and m_targetPoint
     * not sure how to check for bubble yet...maybe keep them in vector based on system.
     */

    State = DSTBALL_WARP;

    // send client updates
    std::vector<PyTuple*> updates;
    // acknowledge client's warpto request
    DoDestiny_CmdWarpTo wt;
        wt.entityID = mySE->GetID();
        wt.dest_x = m_targetPoint.x;
        wt.dest_y = m_targetPoint.y;
        wt.dest_z = m_targetPoint.z;
        wt.distance = m_stopDistance;
        wt.warpSpeed = GetWarpSpeed();      // warp speed x10
    updates.push_back(wt.Encode());
    //send a warp effect...
    DoDestiny_OnSpecialFX10 sfx;
        sfx.guid = "effects.Warping";
        sfx.entityID = mySE->GetID();
        sfx.isOffensive = false;
        sfx.start = true;
        sfx.active = true;
    updates.push_back(sfx.Encode());
    SendDestinyUpdate(updates);
    updates.clear();
    //set massive for warp, per client, but self-only
    DoDestiny_SetBallMassive bm;
        bm.entityID = mySE->GetID();
        bm.is_massive = false;
    PyTuple *up = bm.Encode();
    SendSingleDestinyUpdate(&up, true);

    // calculate actual target point after adjusting for stop distance.
    //  error fix for all ships appearing to "warp to 0" from outside POV.
    GVector revTrajectory(where, m_position);
    revTrajectory.normalize();
    revTrajectory *= m_stopDistance;
    m_targetPoint += revTrajectory;

    _log(DESTINY__WARP_TRACE, "Destiny::WarpTo() m_targetPoint: %.2f,%.2f,%.2f  m_stopDistance: %i  m_targetDistance: %.4f",
         m_targetPoint.x, m_targetPoint.y, m_targetPoint.z, m_stopDistance, m_targetDistance);
}

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
    float degrees = EvE_RadiansToDegrees(acos(dot));
    if (degrees < TURN_ALIGNMENT)
        return true;
    return false;
}

void DestinyManager::Undock(GPoint dir) {
    //set movement direction
    m_targetPoint = dir *1.0e16;
    m_shipHeading = GVector(dir);
    SetUndockSpeed();
}

void DestinyManager::SetUndockSpeed() {
    //start ship movement @ max velocity for undocking.
    // this simulates being forcefully "ejected" from station
    m_stop = false;
    m_orbiting = 0;
    m_stateStamp = sEntityList.GetStamp();
    m_moveTimer = GetTimeMSeconds();
    m_shipMaxAccelTime = 0.1f;
    m_prevSpeedFraction = 0.0f;
    m_userSpeedFraction = 1.0f;
    m_currentSpeedFraction = 1.0f;
    m_maxSpeed = m_maxShipSpeed;
    m_velocity = m_shipHeading * m_maxSpeed;

    if (!mySE->IsMissileSE()) {
        State = DSTBALL_GOTO;
        std::vector<PyTuple*> updates;
        DoDestiny_SetBallVelocity bv;
            bv.entityID = mySE->GetID();
            bv.x = m_velocity.x;
            bv.y = m_velocity.y;
            bv.z = m_velocity.z;
        updates.push_back(bv.Encode());
        DoDestiny_CmdGotoDirection du;
            du.entityID = mySE->GetID();
            du.x = m_shipHeading.x;
            du.y = m_shipHeading.y;
            du.z = m_shipHeading.z;
        updates.push_back(du.Encode());
        SendDestinyUpdate(updates);
    }
}

PyResult DestinyManager::AttemptDockOperation() {
    Client *pClient = mySE->GetPilot();
    uint32 stationID = pClient->GetDockStationID();
    SystemEntity *station = mySE->SystemMgr()->GetSE(stationID);

    if (!station) {
        codelog(CLIENT__ERROR, "%s: Station %u not found.", pClient->GetName(), stationID);
        pClient->SendErrorMsg("Station Not Found, Docking Aborted.");
        return new PyNone();
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
            throw PyException(MakeUserError("DockingApproach"));
    }

    pClient->SetClientTimer(ClientState::csDock, sConfig.world.StationDockDelay *1000); // default @ 4sec();

    return new PyLong(Win32TimeNow());
}

void DestinyManager::Dock()
{
    Stop();

    Client *pClient = mySE->GetPilot();
    uint32 stationID = pClient->GetDockStationID();
    SystemEntity *station = mySE->SystemMgr()->GetSE(stationID);
    const GPoint stationPos = station->GetPosition();

    DoDestiny_OnDockingAccepted oda;
        oda.ship_x = m_position.x;
        oda.ship_y = m_position.y;
        oda.ship_z = m_position.z;
        oda.station_x = stationPos.x;
        oda.station_y = stationPos.y;
        oda.station_z = stationPos.z;
        oda.stationID = stationID;
    PyTuple* ev = oda.Encode();
    pClient->SendNotification("OnDockingAccepted", "charid", &ev);
}

void DestinyManager::SetPosition(const GPoint &pt, bool update /*false*/) {
    m_position = pt;

    // this sets InventoryItemRef.m_position correctly, which is used for all position references
    mySE->SetPosition(m_position);

    if (mySE->IsPOSSE()) {         //according to packet sniffs, this is only used for 'Structure' items
        DoDestiny_SetBallPosition du;
            du.entityID = mySE->GetID();
            du.x = m_position.x;
            du.y = m_position.y;
            du.z = m_position.z;
        PyTuple* up = du.Encode();
        SendSingleDestinyUpdate(&up, true);    //consumed
    }
    if (update) {
        DoDestiny_SetBallPosition du;
            du.entityID = mySE->GetID();
            du.x = m_position.x;
            du.y = m_position.y;
            du.z = m_position.z;
        PyTuple* up = du.Encode();
        SendSingleDestinyUpdate(&up);    //consumed
    }
}

// settings for ship, npc and missile max speeds
void DestinyManager::SetMaxVelocity(float maxVelocity)
{
    float maxSpeed = mySE->GetSelf()->GetAttribute(AttrMaxVelocity).get_float();
    /*
    if (mySE->IsMissileSE() or mySE->IsNPCSE())
        maxSpeed = mySE->GetSelf()->GetAttribute(AttrMaxVelocity).get_float();
    else if (mySE->IsShipSE())
        maxSpeed = mySE->GetSelf()->GetAttribute(AttrMaxDirectionalVelocity).get_float();   // this is depreciated.  used as an absolute max speed, accounting for ab/mwd
    else
        ; // make error here?
        */
    if (mySE->IsShipSE()) {
        _log(DESTINY__TRACE, "Destiny::SetMaxVelocity() - Ship:%s(%u) Pilot:%s(%u) - AttrMaxDirectionalVelocity is %.1f", \
                    mySE->GetName(), mySE->GetID(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID(), \
                    mySE->GetSelf()->GetAttribute(AttrMaxDirectionalVelocity).get_float());
    }
    if (maxVelocity > maxSpeed)
        m_maxShipSpeed = maxSpeed;
    else
        m_maxShipSpeed = maxVelocity;
}

void DestinyManager::SpeedBoost(bool deactivate/*false*/)
{
    if (m_cloaked)
        UnCloak();  // we all know you cant run prop mods with cloak...

    float oldMass = m_mass;
    float oldAgility = m_shipAgility;
    float oldAccelTime = m_shipMaxAccelTime;
    // prop mod state changed.  reset ship movement variables and update current movement, if applicable
    m_mass = mySE->GetSelf()->GetAttribute(AttrMass).get_float();
    m_massMKg = m_mass / 1000000; //changes mass from Kg to MillionKg (10^-6)
    m_shipAgility = m_massMKg * m_shipInertia;
    m_alignTime = (-log(0.25) * m_shipAgility);
    m_shipMaxAccelTime = (m_shipAgility * -log(0.0001));

    _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost() - oldMass: %.5f, newMass: %.5f, oldAgility: %.5f, newAgility: %.5f", \
            oldMass, m_mass, oldAgility, m_shipAgility);

    // check current movement and reset variables using modified values
    m_maxShipSpeed = mySE->GetSelf()->GetAttribute(AttrMaxVelocity).get_float();

    // with new max speed, will have to reset move timers, putting them at fraction of max between current speed and new max.
    double curTime = (GetTimeMSeconds() - m_moveTimer) /1000;   // current movement ticTime
    m_prevSpeed = m_maxSpeed * m_currentSpeedFraction;  //get current ship speed

    // ship is currently....
    if (deactivate) {
        // ....deactivating prop mod
        // - use accel formula to determine decel time
        // t=IM(10^-6) * -ln(1-(v/V))
        float deltaTime = (log(m_prevSpeed - (m_maxShipSpeed * m_userSpeedFraction)) * m_shipAgility);
        // -  set speed fractions and decel timers for new max speed
        m_activeSpeedFraction = m_shipMaxAccelTime / deltaTime;
        float fracCheck = m_prevSpeed / m_maxShipSpeed;
        //m_currentSpeedFraction = 1 - m_activeSpeedFraction;    // reset csf
        m_moveTimer = GetTimeMSeconds();    // reset timer
        _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::Deactivate - sec: %.2f, csf: %.3f. asf: %.3f (check: %.3f), curSpeed: %.3f, decelTime: %.3f", \
                curTime, m_currentSpeedFraction, m_activeSpeedFraction, fracCheck, m_prevSpeed, m_shipMaxAccelTime);
    } else if (m_orbiting) {
        // ....orbiting
        // - call Orbit() and let code reset the orbit
        Orbit(m_targetEntity.second, m_targetDistance);
    } else if ((m_activeSpeedFraction) or (m_prevSpeedFraction)) {
        // ....moving and....
        if ((!m_userSpeedFraction) or (m_prevSpeedFraction)) {
            // ....is decelerating
            // - this may not hit here....
            _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(asf>usf>=0) - decelerating.  not coded yet - sec: %.2f, csf: %.3f. asf: %.3f, curSpeed: %.3f, accelTime: %.3f", \
                    curTime, m_currentSpeedFraction, m_activeSpeedFraction, m_prevSpeed, m_shipMaxAccelTime);
        } else {
            // ....is not decelerating (this includes turning)
            // - recalculate asf with new variables
            m_maxSpeed = m_maxShipSpeed * m_userSpeedFraction;      // reset ship max speed using updated m_maxShipSpeed
            m_currentSpeedFraction = m_activeSpeedFraction = m_prevSpeed / m_maxSpeed;          //get updated asf
            // - reverse accel equation to calculate new csf based on new variables
            //m_currentSpeedFraction = ((-log(m_activeSpeedFraction) +1) / m_shipAgility);
            // - use accel equation to get elapsed time for new csf
            double newTime = (-log(1 - m_currentSpeedFraction) * m_shipAgility);
            _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(0<asf<usf) - newTime: %.2f, new csf: %.3f, new asf: %.3f, curSpeed: %.3f, accelTime: %.3f", \
                    newTime, m_currentSpeedFraction, m_activeSpeedFraction, m_prevSpeed, m_shipMaxAccelTime);
            m_prevSpeed = 0;    // previous speed is not needed for subsquent calculations
            // adjust m_moveTimer time to fit current speed onto new max speed range.  (previous max < new max)
            m_moveTimer = (GetTimeMSeconds() - (newTime * 1000));
        }
    } else {
        // ....is sitting still
        // - do nothing
        if (m_userSpeedFraction) {
            _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(usf>asf=0) -  sitting still.  not coded yet");
        } else {
            _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(usf=asf=0) -  sitting still.  not coded yet");
        }
    }

    SetSpeedFraction(m_userSpeedFraction, true);

    std::vector<PyTuple*> updates;
    DoDestiny_SetBallAgility sbagility;
        sbagility.entityID =  mySE->GetID();
        sbagility.agility = m_shipInertia;
    updates.push_back(sbagility.Encode());
    DoDestiny_SetBallMass sbmass;
        sbmass.entityID = mySE->GetID();
        sbmass.mass = m_mass;
    updates.push_back(sbmass.Encode());
    DoDestiny_SetMaxSpeed sbms;
        sbms.entityID = mySE->GetID();
        sbms.speedValue = m_maxSpeed;
    updates.push_back(sbms.Encode());
    SendDestinyUpdate(updates);
}

void DestinyManager::WebbedMe()
{
    m_maxShipSpeed = mySE->GetSelf()->GetAttribute(AttrMaxVelocity).get_float();

    std::vector<PyTuple*> updates;
    DoDestiny_SetMaxSpeed sbms;
        sbms.entityID = mySE->GetID();
        sbms.speedValue = m_maxShipSpeed;
    updates.push_back(sbms.Encode());
    SendDestinyUpdate(updates);
}

//  called from Client::CreateShipSE(), Client::ResetAfterPodded(), NPC::NPC(), Concord::Concord(), Drone::Drone(), DestinyManager::UpdateNewShip()
void DestinyManager::SetShipCapabilities(InventoryItemRef ship, bool undock)
{
    /* this sets variables needed for correct movement math.
     *  these attribs are set ship item when ship created.  DO NOT modify here
     */
    m_mass = ship->GetAttribute(AttrMass).get_float();
    m_radius = ship->GetAttribute(AttrRadius).get_float();
    m_massMKg = m_mass / 1000000; //changes mass from Kg to MillionKg (10^-6)

    //  check for (and set) warp strength modifiers
    if (ship->HasAttribute(AttrWarpScrambleStatus))
        m_warpStrength = (int8)ship->GetAttribute(AttrWarpScrambleStatus).get_int();    // >0 == cannot warp

    // this will catch speeds/needs for all ships (player and npc), and is easier to do here.
    if (ship->HasAttribute(AttrWarpSpeedMultiplier))
        m_shipWarpSpeed = ship->GetAttribute(AttrWarpSpeedMultiplier).get_float();
    if (ship->HasAttribute(AttrInetia))
        m_shipInertia = ship->GetAttribute(AttrInetia).get_float();
    if (ship->HasAttribute(AttrMaxVelocity))
        m_maxShipSpeed = ship->GetAttribute(AttrMaxVelocity).get_float();
    if (ship->HasAttribute(AttrWarpCapacitorNeed))
        m_warpCapacitorNeed = ship->GetAttribute(AttrWarpCapacitorNeed).get_float();

    if (mySE->IsNPCSE())
        m_maxShipSpeed = ship->GetAttribute(AttrEntityCruiseSpeed).get_float();

    /*  per https://forums.eveonline.com/default.aspx?g=posts&m=3912843   post#103
     *
     * Ships will exit warp mode when their warping speed drops below
     * 75% of sub-warp max speed, or 100m/s, whichever is the lower.
     */
    m_speedToLeaveWarp = m_maxShipSpeed *0.75;
    if ((m_speedToLeaveWarp < 100) and (m_maxShipSpeed > 100))
        m_speedToLeaveWarp = 100;

    /* The product of Mass and the Inertia Modifier gives the ship's agility
     * Agility = Mass x Inertia Modifier
     *  agility is an internal-use variable.
     */
    m_shipAgility = m_massMKg * m_shipInertia;
    // set a maximum acceleration time (based on ship variables)
    //   this is no longer correct.  Vmax/T is the correct formula
    m_shipMaxAccelTime = (m_shipAgility * -log(0.0001));

    //  both of these formulas have identical products
    //TimeToWarp = -ln(0.25) x Mass Mkg x Inertia Mod
    //float alignTime = ((log(2) * m_shipInertia * m_mass) / 500000);
    m_alignTime = (-log(0.25) * m_shipAgility);
    m_timeToEnterWarp = m_alignTime;

    if (!mySE->HasPilot())
        return;
    if (mySE->GetPilot()->IsInSpace() and mySE->SysBubble()) {
        std::vector<PyTuple*> updates;
        DoDestiny_SetBallAgility sbagility;
            sbagility.entityID =  mySE->GetID();
            sbagility.agility = m_shipInertia;
        updates.push_back(sbagility.Encode());
        DoDestiny_SetBallMassive sbmassive;
            sbmassive.entityID = mySE->GetID();
            sbmassive.is_massive = true;
        updates.push_back(sbmassive.Encode());
        DoDestiny_SetBallMass sbmass;
            sbmass.entityID = mySE->GetID();
            sbmass.mass = m_mass;
        updates.push_back(sbmass.Encode());
        SendDestinyUpdate(updates); //consumed
        m_hasSentShipUpdates = true;
    } else {
        m_hasSentShipUpdates = false;
    }
}

void DestinyManager::MakeMissile(Missile* pMissile) {
    SetMaxVelocity(pMissile->GetSpeed());
    SetPosition(pMissile->GetSelf()->position());
    m_mass = pMissile->GetSelf()->type().mass();
    m_massMKg = m_mass / 1000000; //changes mass from Kg to MillionKg (10^-6)
    m_radius = pMissile->GetSelf()->type().radius();
    m_shipInertia = pMissile->GetSelf()->GetAttribute(AttrInetia).get_float();
    m_shipAgility = m_massMKg * m_shipInertia;

    SystemEntity* pTarget = pMissile->GetTarget();
    State = DSTBALL_MISSILE;
    m_stop = false;
    m_stateStamp = sEntityList.GetStamp();
    m_targetPoint = GPoint(pTarget->GetPosition());
    m_targetEntity.first = pTarget->GetID();
    m_targetEntity.second = pTarget;
    m_targetDistance = m_position.distance(m_targetPoint);

    GVector moveVector(m_position, m_targetPoint);
    moveVector.normalize();     //change vector to direction
    m_shipHeading = moveVector;

    SetUndockSpeed();   /* sets all needed variables for max velocity */
    mySE->SystemMgr()->AddEntity(pMissile);

    std::vector<PyTuple*> updates;
    DoDestiny_SetMaxSpeed maxspeed;
        maxspeed.entityID = pMissile->GetID();
        maxspeed.speedValue = m_maxShipSpeed;
    updates.push_back(maxspeed.Encode());
    Rsp_LaunchMissile miss;
        miss.shipID = pMissile->GetShip()->itemID();
        miss.targetID = pTarget->GetID();
        miss.missileID = pMissile->GetID();
        miss.unk1 = 1;
        miss.unk2 = 1;
    updates.push_back(miss.Encode());
    SendDestinyUpdate(updates); //consumed
}

void DestinyManager::UpdateNewShip(const ShipItemRef newShipRef) {
    std::vector<PyTuple*> updates;
    SetShipCapabilities(newShipRef);

    SendBallInteractive(newShipRef, true);

    Client* pClient = mySE->GetPilot();
    PyDict* slim = new PyDict();
        slim->SetItemString("itemID",           new PyInt(newShipRef->itemID()));
        slim->SetItemString("typeID",           new PyInt(newShipRef->typeID()));
        slim->SetItemString("categoryID",       new PyInt(newShipRef->categoryID()));
        slim->SetItemString("ownerID",          new PyInt(pClient->GetCharacterID()));
        slim->SetItemString("charID",           new PyInt(pClient->GetCharacterID()));
        slim->SetItemString("corpID",           new PyInt(pClient->GetCorporationID()));
        slim->SetItemString("allianceID",       new PyInt(pClient->GetAllianceID()));
        slim->SetItemString("warFactionID",     new PyInt(pClient->GetWarFactionID()));
        slim->SetItemString("bounty",           new PyFloat(pClient->GetBounty()));
        slim->SetItemString("securityStatus",   new PyFloat(pClient->GetSecurityRating()));
    if (newShipRef->typeID() == EVEDB::invTypes::typeCapsule)
        slim->SetItemString("modules",          new PyList());
    else {
        PyList* moduleList = newShipRef->ShipGetModuleList();
        slim->SetItemString("modules",          moduleList);
    }
    PyTuple* shipData = new PyTuple(2);
        shipData->SetItem(0, new PyLong(newShipRef->itemID()));
        shipData->SetItem(1, new PyObject( "foo.SlimItem", slim));
    PyTuple* shipItem = new PyTuple(2);
        shipItem->SetItem(0, new PyString("OnSlimItemChange"));
        shipItem->SetItem(1, shipData);
    updates.push_back(shipItem);

    DoDestiny_SetBallAgility sbagility;
        sbagility.entityID = newShipRef->itemID();
        sbagility.agility = m_shipAgility;
    updates.push_back(sbagility.Encode());

    DoDestiny_SetBallMass sbmass;
        sbmass.entityID = newShipRef->itemID();
        sbmass.mass = m_mass;
    updates.push_back(sbmass.Encode());

    DoDestiny_SetMaxSpeed ms;
        ms.entityID = newShipRef->itemID();
        ms.speedValue = m_maxSpeed;
    updates.push_back(ms.Encode());

    SendDestinyUpdate(updates);
}

void DestinyManager::UpdateOldShip(const ShipItemRef oldShipRef)
{
    PyDict* slimPod = new PyDict();
        slimPod->SetItemString("itemID",           new PyInt(oldShipRef->itemID()));
        slimPod->SetItemString("typeID",           new PyInt(oldShipRef->typeID()));
        slimPod->SetItemString("categoryID",       new PyInt(oldShipRef->categoryID()));
        slimPod->SetItemString("ownerID",          new PyInt(0));
        slimPod->SetItemString("charID",           new PyInt(0));
        slimPod->SetItemString("corpID",           new PyInt(0));
        slimPod->SetItemString("allianceID",       new PyInt(0));
        slimPod->SetItemString("warFactionID",     new PyInt(0));
        slimPod->SetItemString("bounty",           new PyInt(0));
        slimPod->SetItemString("securityStatus",   new PyInt(0));
    PyTuple* shipData = new PyTuple(2);
        shipData->SetItem(0, new PyLong(oldShipRef->itemID()));
        shipData->SetItem(1, new PyObject( "foo.SlimItem", slimPod));
    PyTuple* shipItem = new PyTuple(2);
        shipItem->SetItem(0, new PyString("OnSlimItemChange"));
        shipItem->SetItem(1, shipData);
    SendSingleDestinyUpdate(&shipItem);

    SendBallInteractive(oldShipRef, false);
}

void DestinyManager::Jump()
{
    m_cloaked = true;
    SendCloakShip(true);
}

void DestinyManager::Cloak() {
    m_cloaked = true;
    SendCloakShip(true);
    mySE->SysBubble()->RemoveExclusive(mySE);
}

void DestinyManager::UnCloak() {
    m_cloaked = false;
    SendUncloakShip();
    mySE->SysBubble()->AddBallExclusive(mySE);
}

void DestinyManager::TractorBeamStart(SystemEntity* pShipSE)
{
    /** @todo  need to update this */
    State = DSTBALL_FOLLOW;

    m_stop = false;
    m_accel = false;
    m_decel = false;
    m_turning = false;
    m_tractored = true;
    m_moveTimer = GetTimeMSeconds();
    m_stateStamp = sEntityList.GetStamp();

    m_targetPoint = pShipSE->GetPosition();
    GVector moveVector(m_position, m_targetPoint);
    m_targetDistance = moveVector.length();
    moveVector.normalize();
    m_shipHeading = moveVector;

    m_maxShipSpeed = 500;
    m_maxSpeed = m_maxShipSpeed;
    m_velocity = m_shipHeading * m_maxSpeed;

    m_followDistance = 500 + pShipSE->GetRadius();
    m_shipMaxAccelTime = 0.1f;

    m_activeSpeedFraction = m_userSpeedFraction = m_currentSpeedFraction = 1.0f;

    m_targetEntity.first = pShipSE->GetID();
    m_targetEntity.second = pShipSE;

    std::vector<PyTuple*> updates;
    DoDestiny_SetMaxSpeed ms;
        ms.entityID = mySE->GetID();
        ms.speedValue = m_maxShipSpeed;
    updates.push_back(ms.Encode());
    DoDestiny_SetBallFree bf;
        bf.entityID = mySE->GetID();
        bf.is_free = 1;
    updates.push_back(bf.Encode());
    DoDestiny_SetBallMass sbmass;
        sbmass.entityID = mySE->GetID();
        sbmass.mass = 10000;
    updates.push_back(sbmass.Encode());
    DoDestiny_CmdSetSpeedFraction ssf;
        ssf.entityID = mySE->GetID();
        ssf.fraction = 1;
    updates.push_back(ssf.Encode());
    DoDestiny_CmdFollowBall fb;
        fb.entityID = mySE->GetID();
        fb.targetID = pShipSE->GetID();
        fb.range = m_followDistance;
    updates.push_back(fb.Encode());
    SendDestinyUpdate(updates);
}

void DestinyManager::TractorBeamStop()
{
    Halt();
    m_tractored = false;
    std::vector<PyTuple*> updates;
    DoDestiny_SetMaxSpeed ms;
        ms.entityID = mySE->GetID();
        ms.speedValue = 0;
    updates.push_back(ms.Encode());
    DoDestiny_SetBallFree bf;
        bf.entityID = mySE->GetID();
        bf.is_free = 0;
    updates.push_back(bf.Encode());
    DoDestiny_SetBallMass sbmass;
        sbmass.entityID = mySE->GetID();
        sbmass.mass = m_mass;
    updates.push_back(sbmass.Encode());
    SendDestinyUpdate(updates);
}

void DestinyManager::SendJettisonPacket() const {
    DoDestiny_OnSpecialFX10 effect;
        effect.entityID = mySE->GetID();
        effect.guid = "effects.Jettison";
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
    PyTuple* up = effect.Encode();
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendAnchorDrop() const {
    DoDestiny_OnSpecialFX10 effect;
        effect.entityID = mySE->GetID();
        effect.guid = "effects.AnchorDrop";
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
    PyTuple* up = effect.Encode();
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendAnchorLift() const {
    DoDestiny_OnSpecialFX10 effect;
        effect.entityID = mySE->GetID();
        effect.guid = "effects.AnchorLift";
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
    PyTuple* up = effect.Encode();
    SendSingleDestinyUpdate(&up);
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

/** @todo combine these two with an "apply/remove" boolean */
void DestinyManager::SendCloakShip(const bool IsWarpSafe) const {
    DoDestiny_OnSpecialFX10 effect;
        effect.guid = "effects.Cloak";
        effect.entityID = mySE->GetID();
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
    PyTuple *up = effect.Encode();
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendUncloakShip() const {
    DoDestiny_OnSpecialFX10 effect;
        effect.guid = "effects.Uncloak";
        effect.entityID = mySE->GetID();
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
    PyTuple *up = effect.Encode();
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendSpecialEffect10(uint32 entityID, uint32 targetID, std::string guid, bool isOffensive, bool start, bool isActive) const
{
	std::vector<int32, std::allocator<int32> > area;    //TODO need to figure out what this is....

    DoDestiny_OnSpecialFX10 effect;
        effect.entityID = entityID;
        effect.targetID = targetID;
        effect.guid = guid;
        effect.area = area;
        effect.isOffensive = isOffensive;
        effect.start = start;
        effect.active = isActive;
    PyTuple *up = effect.Encode();
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendSpecialEffect(uint32 entityID, uint32 moduleID, uint32 moduleTypeID, uint32 targetID,
                                       uint32 chargeTypeID, std::string guid, bool isOffensive, bool start,
                                       bool isActive, double duration, uint32 repeat) const
{
    std::vector<int32, std::allocator<int32> > area;    //TODO need to figure out what this is....

    DoDestiny_OnSpecialFX13 effect;
        effect.entityID = entityID;
        effect.moduleID = moduleID;
        effect.moduleTypeID = moduleTypeID;
        effect.targetID = targetID;
        effect.otherTypeID = chargeTypeID;
        effect.area = area;
        effect.guid = guid;
        effect.isOffensive = isOffensive;
        effect.start = start;
        effect.active = isActive;
        effect.duration_ms = duration;
        effect.repeat = repeat;
        effect.startTime = Win32TimeNow();
    PyTuple *up = effect.Encode();
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendJumpOut(uint32 gateID) const {
    DoDestiny_OnSpecialFX10 effect;
        effect.entityID = mySE->GetID();
        effect.targetID = gateID;
        effect.guid = "effects.JumpOut";
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
    PyTuple *up = effect.Encode();
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendGateActivity(uint32 gateID) const {
    DoDestiny_OnSpecialFX10 du;
        du.entityID = gateID;
        du.guid = "effects.GateActivity";
        du.isOffensive = 0;
        du.start = 1;
        du.active = 0;
    PyTuple* up = du.Encode();
    SendSingleDestinyUpdate(&up);    //consumed
}

void DestinyManager::SendBallInteractive(const ShipItemRef shipRef, bool set) const {
    // interactive means "ship has pilot"
    DoDestiny_SetBallInteractive sbi;
        sbi.entityID = shipRef->itemID();
        sbi.interactive = set;
    PyTuple* up = sbi.Encode();
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendJumpOutEffect(std::string JumpEffect, uint32 locationID) const {
    std::vector<PyTuple*> updates;

    DoDestiny_CmdStop du;
    du.entityID = mySE->GetID();
    updates.push_back(du.Encode());

    DoDestiny_OnSpecialFX10 effect;
    effect.entityID = mySE->GetID();
    effect.targetID = locationID;
    effect.guid = "effects.JumpDriveOut";   /* JumpDriveInBO */
    effect.isOffensive = 0;
    effect.start = 1;
    effect.active = 0;
    updates.push_back(effect.Encode());

    SendDestinyUpdate(updates);
}

void DestinyManager::SendJumpInEffect(std::string JumpEffect) const {
    std::vector<PyTuple*> updates;

    DoDestiny_OnSpecialFX10 effect;
    effect.guid = "effects.JumpDriveIn";
    effect.entityID = mySE->GetID();
    effect.isOffensive = 0;
    effect.start = 1;
    effect.active = 0;
    updates.push_back(effect.Encode());

    DoDestiny_CmdSetSpeedFraction ssf;
    ssf.entityID = mySE->GetID();
    ssf.fraction = 0.0;
    updates.push_back(ssf.Encode());

    DoDestiny_SetBallVelocity sbv;
    sbv.entityID = mySE->GetID();
    sbv.x = 0.0;
    sbv.y = 0.0;
    sbv.z = 0.0;
    updates.push_back(sbv.Encode());

    SendDestinyUpdate(updates);
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
    DoDestiny_TerminalExplosion du;
        du.shipID = shipID;
        du.bubbleID = bubbleID;
        du.ballIsGlobal = isGlobal;
    PyTuple* up = du.Encode();
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendSetState() const {
    if (!mySE->HasPilot()) return;
    if (mySE->GetPilot()->IsSetStateSent()) return;

    _log(DESTINY__MESSAGE, "Destiny::SendSetState() Called for Ship:%s(%u) Pilot:%s(%u)", \
                        mySE->GetName(), mySE->GetID(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());

    DoDestiny_SetState ss;
        ss.stamp = sEntityList.GetStamp();
        ss.ego = mySE->GetID();

    mySE->SystemMgr()->MakeSetState(mySE->SysBubble(), ss, mySE->GetPilot()->IsLogin());
    PyTuple* tmp = ss.Encode();
    mySE->GetPilot()->QueueDestinyUpdate(&tmp, true, true);    //setstate should be alone and immediate.  send directly
    mySE->GetPilot()->SetStateSent(true);
}

void DestinyManager::SendSingleDestinyUpdate(PyTuple **up, bool self_only) const {
    std::vector<PyTuple*> updates(1, *up);
    *up = nullptr;
    std::vector<PyTuple*> events;
    //consumes updates and events
    SendDestinyUpdate(updates, events, self_only);
}

void DestinyManager::SendDestinyUpdate(std::vector<PyTuple*> &updates, bool self_only) const {
    std::vector<PyTuple*> events;
    //consumes updates and events
    SendDestinyUpdate(updates, events, self_only);
}

void DestinyManager::SendDestinyUpdate( std::vector<PyTuple*>& updates, std::vector<PyTuple*>& events, bool self_only ) const {
    if (self_only) {
        _log(PLAYER__MESSAGE, "[%u] DestinyManager::SendDestinyUpdate() (u:%i, e:%i) called as 'self_only' for %s(%u)", \
                    sEntityList.GetStamp(), updates.size(), events.size(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());

        std::vector<PyTuple*>::iterator cur = updates.begin();
        for(; cur != updates.end(); cur++) {
            PyTuple* t = *cur;
            mySE->GetPilot()->QueueDestinyUpdate( &t );
        }
        updates.clear();

        cur = events.begin();
        for(; cur != events.end(); cur++) {
            PyTuple* t = *cur;
            mySE->GetPilot()->QueueDestinyEvent( &t );
            PySafeDecRef( t ); //they are not required to consume it.
        }
        events.clear();
    } else if( mySE->SysBubble() ) {
        _log( DESTINY__UPDATES, "[%u] BubbleCasting destiny update (u:%u, e:%u)", sEntityList.GetStamp(), updates.size(), events.size() );
        mySE->SysBubble()->BubblecastDestiny( updates, events, "destiny" );
    } else {
        _log( DESTINY__ERROR, "[%u] Cannot BubbleCast destiny update (u:%u, e:%u); entity (%u) is not in any bubble.", \
                sEntityList.GetStamp(), updates.size(), events.size(), mySE->GetID() );
        EvE::traceStack();
    }
}
