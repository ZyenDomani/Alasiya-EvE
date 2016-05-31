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
#include "PyServiceMgr.h"
#include "npc/NPC.h"
#include "packets/Missile.h"
#include "ship/DestinyManager.h"
#include "ship/Missile.h"
#include "station/Station.h"
#include "system/BubbleManager.h"
#include "system/Container.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"

using namespace Destiny;

DestinyManager::DestinyManager(SystemEntity *self)
: mySE(self),
m_position( NULL_ORIGIN ),    // right in the middle of the star
m_maxSpeed(1.0f),
m_shipMaxAccelTime(0.0f),
State(DSTBALL_STOP),
m_dockTimer(1000),  // dock acceptance timer at 1sec
m_warpTimer(5000),  //completely arbitrary.
m_moveTimer(0.0),
m_userSpeedFraction(0.0f),
m_activeSpeedFraction(0.0f),
m_currentSpeedFraction(0.0f),
m_targetDistance(0.0),
m_followDistance(0.0),
m_stopDistance(0),
m_radius(1.0),
m_mass(1.0f),
m_turnTic(1),
m_massMKg(1.0f),
m_alignTime(1.0f),
m_timeToEnterWarp(0.0f),
m_shipWarpSpeed(0.0f),
m_maxShipSpeed(1.0),
m_shipAgility(1.0),
m_shipInertiaModifier(1.0),
m_warpStrength(1),
m_warpCapacitorNeed(1.0),
m_warpAccelTime(1),
m_warpDecelTime(1),
m_warpState(nullptr)
{
    m_bump = false;
    m_stop = false;
    m_accel = false;
    m_decel = false;
    m_cloaked = false;
    m_turning = false;
    m_inBubble = true;
    m_capNeeded = 0;
    m_stateStamp = 0;
    m_dockTimer.Disable();
    m_warpTimer.Disable();
    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;
    m_velocity = GVector( NULL_ORIGIN );
    m_targetPoint = GPoint( NULL_ORIGIN );
    m_shipHeading = GVector( NULL_ORIGIN );
    m_targetHeading = GVector( NULL_ORIGIN );
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

    if (m_dockTimer.Enabled() and m_dockTimer.Check(false)) {
        m_dockTimer.Disable();
        if (mySE->IsShipSE() and mySE->HasPilot()) {
            Client* pClient = mySE->GetPilot();
            pClient->DockToStation();
        }
    }

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
            //set postion and direction for this round of movement
            m_shipHeading = moveVector;
            m_velocity = (moveVector * m_maxSpeed);
            SetPosition(m_position + m_velocity);
        } break;
        case DSTBALL_ORBIT: {
            if (_IsTargetInvalid())
                return;
            _Orbit();
        } break;
        case DSTBALL_FOLLOW: {
            if (_IsTargetInvalid())
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
                else // houston, we have a problem...
                    _log(DESTINY__ERROR, "Destiny::ProcessState() Error!  Ship %s(%u) for Player %s(%u) Has WarpState but checks are false.",  \
                                        mySE->GetName(), mySE->GetID(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());
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
            } else if (m_currentSpeedFraction < 0.749) {
                if (m_userSpeedFraction < 0.7499)
                    SetSpeedFraction(1.0f, true);
                _Move();
            } else
                _Move();
        } break;
        case DSTBALL_MUSHROOM:      // aoe?
        case DSTBALL_BOID:          // this will turn RIGID after a set time
        case DSTBALL_TROLL:         // seen for wrecks
        case DSTBALL_MINIBALL:      // used for sentrys
        case DSTBALL_FIELD:
        case DSTBALL_FORMATION:
        case DSTBALL_RIGID:         // item that never moves
            //no default on purpose
            break;
    }
}

//Velocity setting methods
void DestinyManager::SetSpeedFraction(float fraction, bool startMovement) {
    if ((fraction == m_userSpeedFraction) and (!startMovement)) return;
    _log(DESTINY__MOVE_TRACE, "Destiny::SetSpeedFraction() Called by %s(%u).  fraction: %.2f, start: %i, stop: %i",
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

    /* movement is set according to time vs speed fraction.         -allan 8Oct14  -major update 20Nov15
     * all *SpeedFraction variables use fuzzy logic
     *  m_userSpeedFraction (USF) is user-set speed control (fractional from speedo or full from goto, warp, align, follow, and stop).
     *   -> sets m_maxShipSpeed
     *  m_activeSpeedFraction (ASF) holds ship's current speed setting.  set in _Move()
     *   this is actually a ratio of CSF to USF, capped by USF.
     *   -> sets accel/decel when changing speeds while moving
     *  m_currentSpeedFraction (CSF) holds current euler value for time.  set in _Move()
     *   this is reset on initial turn based on turn angle, then follows normal progression.
     *   -> sets m_velocity
     *  m_maxShipSpeed is ship maximum speed based on user input.  set in _UpdateVelocity()
     *   -> sets m_velocity
     *  m_targetPoint holds current target coords.  set by goto, warp, align, follow, orbit
     *   -> sets m_shipHeading
     *  m_shipHeading holds current direction and is set in _Turn()
     *   -> sets m_velocity
     *  m_velocity is current ship velocity.  set in _Move()
     *   m_velocity = m_shipHeading * m_currentSpeedFraction * m_maxShipSpeed
     */

    m_userSpeedFraction = fraction;
    bool isMoving = false;
    if ((m_currentSpeedFraction > 0.05) or (m_activeSpeedFraction > 0.05)) isMoving = true;
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

    if (mySE->IsNPCSE() or mySE->IsMissileSE()) {
        DoDestiny_SetMaxSpeed ms;   //NPCs and Missiles only.
            ms.entityID = mySE->GetID();
            ms.speedValue = m_maxSpeed;
        updates.push_back(ms.Encode());
    }

    SendDestinyUpdate(updates);
}

void DestinyManager::_UpdateVelocity(bool isMoving) {
    //if (isMoving and (m_currentSpeedFraction < 0.1)) return;
    m_accel = m_decel = false;
    m_moveTimer = GetTimeMSeconds();
    m_shipMaxAccelTime = m_shipAgility * -log(0.0001);
    uint8 logType = 0;
    if ((State == DSTBALL_WARP) and m_warpState) {
        /*  _Warp() finished, and ship dropped out of warp at m_speedToLeaveWarp,
         *  reset m_shipMaxAccelTime as a fraction of m_speedToLeaveWarp/m_maxShipSpeed
         *  to set decel correctly, as m_speedToLeaveWarp varies with ship and warp distance.
         */
        logType = 1;
        m_shipMaxAccelTime *= (m_speedToLeaveWarp / m_maxShipSpeed);
        m_velocity = m_shipHeading * m_speedToLeaveWarp;
        m_maxSpeed = m_speedToLeaveWarp;
    } else if (m_userSpeedFraction) {   //moving
        if (isMoving) { //change speed
            if (m_activeSpeedFraction == m_userSpeedFraction) return;

            //  the times are a bit off when usf < 1.0, but acceptable for now.  will revisit later.    -allan 21Nov15
            float delta = 0.0f;
            logType = 2;
            if (m_userSpeedFraction < m_activeSpeedFraction) {
                if (!m_turning) m_decel = true;
                delta = m_activeSpeedFraction - m_userSpeedFraction;
            } else {
                if (!m_turning) m_accel = true;
                delta = m_userSpeedFraction - m_activeSpeedFraction;
            }

            m_shipMaxAccelTime *= delta;
            m_maxSpeed = m_maxShipSpeed * m_userSpeedFraction;
            if (!m_turning)
                m_velocity = m_shipHeading * m_maxShipSpeed * m_activeSpeedFraction;
            _log(DESTINY__MOVE_TRACE, "Destiny::_UpdateVelocity - USF: %.2f, ASF: %.2f, CSF: %.2f, delta: %.2f", \
                m_userSpeedFraction, m_activeSpeedFraction, m_currentSpeedFraction, delta);
        } else {    //begin movement
            logType = 3;
            m_shipMaxAccelTime *= m_userSpeedFraction;      // for accel with user speeds <= 1.0
            m_maxSpeed = m_maxShipSpeed * m_userSpeedFraction;
            //  see notes in _Move() for information relating to accel equations
            m_currentSpeedFraction = (1 - exp(-1000000 / (m_shipInertiaModifier * m_mass)));
            m_velocity = m_shipHeading * m_maxSpeed * m_currentSpeedFraction;
        }
    } else if (m_currentSpeedFraction) {
        if (isMoving) { //stop movement
            logType = 4;
            //  see notes in _Move() for information relating to decel equations
            m_shipMaxAccelTime *= m_activeSpeedFraction;
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
            return;
        } break;
    }
    _log(DESTINY__MOVE_TRACE, "Destiny::_UpdateVelocity  %s.  AccelTime: %.2f, USF: %.2f", msg.c_str(), m_shipMaxAccelTime, m_userSpeedFraction);
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
    m_velocity = GVector(NULL_ORIGIN);
    m_moveTimer = 0.0;
    m_stateStamp = 0;
    m_targetPoint = GPoint(NULL_ORIGIN);
    m_stopDistance = 0;
    m_targetDistance = 0.0;
    m_followDistance = 0.0;
    m_userSpeedFraction = 0.0f;
    m_activeSpeedFraction = 0.0f;
    m_currentSpeedFraction = 0.0f;

    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;

    _ClearTurn();

    _log(DESTINY__MOVE_TRACE, "Destiny::Halt() - Entity %s(%u): m_shipHeading: %.3f,%.3f,%.3f", \
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

void DestinyManager::_Bump(SystemEntity* who)
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

    /*  run-time options for bumping jetcans, biomass, and other space objects
     *   bump drones??  prolly not, for simplicity
     */
    std::string msg1 = "You have bumped ";
    msg1 += who->GetName();
    mySE->GetPilot()->SendNotifyMsg(msg1.c_str());
    // this test isnt needed right now, as it's ONLY checking against players and will always return true.
    //  will keep it in here for later expansion.
    if (who->HasPilot()) {
        std::string msg2 = "You have been bumped by ";
        msg2 += mySE->GetPilot()->GetName();
        who->GetPilot()->SendNotifyMsg(msg2.c_str());
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
void DestinyManager::_Move(bool orbit) {
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
     * **UPDATE**  this is now tracking ALL speed changes correctly.  - allan 21Nov15
     */

    GVector moveVector(m_shipHeading);
    if (!orbit and m_userSpeedFraction)   // if usf==0 then ship is stopping.  continue movement along current ship heading (no turn)
        moveVector = _Turn();

    double timeStamp = GetTimeMSeconds() - m_moveTimer;
    //float timeStamp = sEntityList.GetStamp() - m_stateStamp;
    float speed = 0.0f, csf = 0.0f;
    std::string move = "";
    // check to make sure we dont overrun usf/asf
    if (m_activeSpeedFraction == m_userSpeedFraction) m_currentSpeedFraction = 1.0f;

    if (orbit) {    //orbiting ship does NOT conform to standard move methods.
        speed = m_maxSpeed * m_currentSpeedFraction;
        csf = m_currentSpeedFraction;
    } else if ((timeStamp > m_shipMaxAccelTime) and (m_currentSpeedFraction > 0.995f)) {
        m_accel = m_decel = false;
        if (m_userSpeedFraction) {
            // ship has reached full speed (whatever the fraction was set to)
            m_currentSpeedFraction = 1.0f;
            m_activeSpeedFraction = m_userSpeedFraction;
            csf = m_currentSpeedFraction;
            move = "at max speed, going";
            speed = m_maxShipSpeed * m_activeSpeedFraction;
        } else {
            //ship has reached full stop
            // update position one final time (for last bit of drift) and exit movement functions by calling Halt()
            SetPosition(m_position + m_velocity);
            _log(DESTINY__MOVE_TRACE, "Destiny::_Move() - Entity %s(%u) is at full stop after %.3f seconds.", \
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
            m_currentSpeedFraction = (1 - exp(-timeStamp * 1000000 / (m_shipInertiaModifier * m_mass)));

        if (m_accel or m_decel) {
            float delta = 0.0f;
            if (m_activeSpeedFraction > m_userSpeedFraction) {
                delta = m_activeSpeedFraction - m_userSpeedFraction;
                m_activeSpeedFraction -= (delta * m_currentSpeedFraction);
                speed = m_maxShipSpeed * m_activeSpeedFraction;
            } else {
                delta = m_userSpeedFraction - m_activeSpeedFraction;
                m_activeSpeedFraction += (delta * m_currentSpeedFraction);
                speed = m_maxShipSpeed * m_activeSpeedFraction;
            }
        } else {
            m_activeSpeedFraction = m_userSpeedFraction * m_currentSpeedFraction;
            speed = m_maxSpeed * m_activeSpeedFraction;
        }

        csf = m_currentSpeedFraction;
        if ((!m_userSpeedFraction) or (m_decel)) {
            //just decelerating
            move = "decelerating";
            m_activeSpeedFraction = m_currentSpeedFraction;
            speed = m_maxSpeed * m_activeSpeedFraction;
            if (m_decel) speed = m_maxShipSpeed - speed;
            else speed = m_maxSpeed - speed;
            csf = 1 - m_currentSpeedFraction;
        }
    }
    // hack for some fucked up movement shit where npc's have a move-type state, but have no speedFraction.
    if (mySE->IsNPCSE() and (!csf)) {
        SetPosition(m_position, true);
        Halt();
        return;
    }

    //set postion and direction for this round of movement
    m_velocity = moveVector * speed;
    SetPosition(m_position + m_velocity);

    if (orbit) {
        _log(DESTINY__MOVE_TRACE, "Destiny::_Move() - Entity %s(%u) is orbiting %s(%u) at %.4f m/s (csf:%.4f asf:%.4f sec: %.3f).", \
            mySE->GetName(), mySE->GetID(), GetTargetEntity()->GetName(), GetTargetID(), \
            speed, csf, m_activeSpeedFraction, timeStamp);
    } else {
        _log(DESTINY__MOVE_TRACE, "Destiny::_Move() - Entity %s(%u) is %s at %.4f m/s (csf:%.4f asf:%.4f sec: %.3f).", \
            mySE->GetName(), mySE->GetID(), move.c_str(), \
            speed, csf, m_activeSpeedFraction, timeStamp);
    //_log(DESTINY__MOVE_TRACE, "Destiny::_Move() - Entity %s(%u) Position: %.2f,%.2f,%.2f  velocity: %.3f,%.3f,%.3f", \
            mySE->GetName(), mySE->GetID(), \
            m_position.x, m_position.y, m_position.z, \
            m_velocity.x, m_velocity.y, m_velocity.z);
    //_log(DESTINY__MOVE_TRACE, "Destiny::_Move() - Before update - moveVector: %.3f,%.3f,%.3f  m_shipHeading: %.3f,%.3f,%.3f", \
            moveVector.x, moveVector.y, moveVector.z, \
            m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);
    }

    m_shipHeading = moveVector;

    if (mySE->HasPilot() and mySE->SysBubble()->HasPlayers()) // no players in bubble = nothing to check against (for now)
        _CheckBump();
}

/* align time in eve
 * t = ln(2)*i*m / 500000
 * where
 * t = time
 * i = ship inertia
 * m = ship mass
 */
// much of the following turn code is from "Ship Motion in Eve Online" by Scheulagh Santorine, Ph.D
bool DestinyManager::_IsTurn() {    //is working.  dont change
    if (m_targetPoint.isZero()) {
        _log(DESTINY__ERROR, "Destiny::_IsTurn() - Entity %s(%u): Target is null.", mySE->GetName(), mySE->GetID());
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
        sLog.Error("DestinyManager::_IsTurn()", "Entity %s(%u) - shipHeading has screwed up.  dot is %.5f", \
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
    _log(DESTINY__TURN_TRACE, "Destiny::_IsTurn() dot: %.5f, radians:%.5f, degrees:%.3f", dot, m_radians, degrees);
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
        // game seems to delay turning till tic after calling any movement functions
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
            _log(DESTINY__TURN_TRACE, "Destiny::_Turn()   %.3fs Align Time for %s", m_alignTime, mySE->GetName());
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
    //_log(DESTINY__TURN_TRACE, "Destiny::_Turn() - turnTic: %u, turnPercent: %.3f, shipHeading: %.5f, %.5f, %.5f", \
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

    double distance = m_targetEntity.second->GetRadius();
    if (m_targetEntity.second->IsStaticEntity())
        distance = 0;
    distance += (m_radius + m_followDistance);

    const GPoint& target_point = m_targetEntity.second->GetPosition();

    GVector heading(m_position, target_point);
    m_targetDistance = heading.length();
    heading.normalize();
    m_targetPoint = target_point + (heading * distance);

    _Move();
}

//FIXME  this is not right.
void DestinyManager::_Orbit() {
    /* r=(A*m*v^2)/(sqrt(Vm^2 - v^2)*1000000)
     * r = orbital radius
     * A = agility
     * m = ship mass in kg
     * Vm = max ship velocity
     * v = orbit velocity for ship
     */

    #define LogMacro(v) _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - Entity %u: " #v ": (%.15e, %.15e, %.15e)   len=%.15e", \
        mySE->GetID(), v.x, v.y, v.z, v.length() )

    const GPoint &orbit_point = m_targetEntity.second->GetPosition();

    GVector delta(m_position, orbit_point);
    LogMacro(delta);
    double current_distance = delta.normalize();
    LogMacro(delta);

    double something = 0;
    double desired_distance =
            m_radius +
            m_targetEntity.second->GetRadius() +
            m_followDistance;
            _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - desired_distance = %.15e", desired_distance);
    if(desired_distance != 0) {
        something = (m_maxSpeed * TIC_DURATION_IN_SECONDS * .01) / desired_distance;
    }
    _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - something = %.15e", something);

    //this seems to be correct, without rounding error.
    //  use m_moveTimer here.....
    double v488 = double(sEntityList.GetStamp()-m_stateStamp) * something;
    _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - v488 = %.15e", v488);

    //this is not quite right... some sort of rounding I think.
    double coef = (v488 * 0.7) + 130001409/*entityID*/;
    _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - coef = %.15e", coef);

    //all of these are wrong due to rounding
    double cos_coef = cos( coef );
    double sin_coef = sin( coef );
    double cos_v488 = cos( v488 );
    double sin_v488 = sin( v488 );
    _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - coef(sin=%.15e, cos=%.15e) v488(sin=%.15e, cos=%.15e)", sin_coef, cos_coef, sin_v488, cos_v488);
    double v438 = cos_coef * sin_v488;
    double v3C0 = cos_v488 * cos_coef;

    GPoint pt( v3C0, sin_coef, v438 );    //this is a unit vector naturally
    LogMacro(pt);

    GVector tan_vector = pt.crossProduct(delta);
    tan_vector.normalize();
    LogMacro(tan_vector);

    double delta_d2 = current_distance*current_distance - desired_distance*desired_distance;
    _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - delta_d2 = %.15e", delta_d2);
    if(delta_d2 >= 0) {
        double mag = sqrt(delta_d2) * desired_distance / current_distance;
        _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - mag = %.15e", mag);
        GVector s = tan_vector * mag;
        LogMacro(s);
        GVector d = delta * (delta_d2/current_distance);
        LogMacro(d);
        delta = s + d;
        delta.normalize();
        LogMacro(delta);
    }

    double d = desired_distance - current_distance;
    d = exp(d*d / (-40000.0));
    _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - d = %.15e", d);

    GVector negative_delta = delta * -1;
    LogMacro(negative_delta);

    double tdn = tan_vector.dotProduct(negative_delta);
    _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - tdn = %.15e", tdn);
    double jjj = ((tdn*tdn - 1.0) * d * d) + 1;
    _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - jjj = %.15e", jjj);
    double iii;
    if(jjj < 0) {    //not sure on this condition at all.
        iii = d * tdn;    //not positive
    } else {
        iii = sqrt(jjj) + (d * tdn);
    }

    if((current_distance - desired_distance) < 0) {
        iii *= -1;
    }

    GPoint bliii = delta * iii;
    LogMacro(bliii);
    GPoint vliii = tan_vector * d;
    LogMacro(vliii);

    GPoint accel_vector = bliii + vliii;
    _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - m_accelerationFactor = %.15e", (m_shipAgility * -log(0.0005)));
    accel_vector *=  (m_shipAgility * -log(0.0005));
    LogMacro(accel_vector);

    //copy delta_position into acceleration for input into movement.

    static const double ten_au = 1.495978707e12;
    GVector big_delta_position = accel_vector * ten_au;
    LogMacro(big_delta_position);

    m_targetPoint = orbit_point + big_delta_position;
    LogMacro(m_targetPoint);
    #undef LogMacro

    m_activeSpeedFraction = m_userSpeedFraction * m_currentSpeedFraction;
    m_shipHeading = delta;
    _Move(true);
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

    _log(DESTINY__WARP_TRACE, "Destiny::_InitWarp(): Entity %s(%u) has initialized warp.", mySE->GetName(), mySE->GetID());

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

    _log(DESTINY__WARP_TRACE, "DestinyManager::_InitWarp():Calculate - Entity %s(%u): Warp will accelerate for %us, cruise for %.3f, then decelerate for %us, with total time of %.3fs, and warp speed of %.4f m/s.", \
            mySE->GetName(), mySE->GetID(), m_warpAccelTime, cruiseTime, m_warpDecelTime, warpTime, warpSpeedInMeters);
    _log(DESTINY__WARP_TRACE, "DestinyManager::_InitWarp():Calculate - Entity %s(%u): Accel distance is %.4f. Cruise distance is %.4f.  Decel distance is %.4f.  Direction is %.3f,%.3f,%.3f.", \
            mySE->GetName(), mySE->GetID(), accelDistance, cruiseDistance, decelDistance, warp_vector.x, warp_vector.y, warp_vector.z);
    _log(DESTINY__WARP_TRACE, "DestinyManager::_InitWarp():Calculate - Entity %s(%u): We will exit warp at %.2f,%.2f,%.2f at a distance of %.4f AU (%.4fm).", \
            mySE->GetName(), mySE->GetID(), m_targetPoint.x, m_targetPoint.y, m_targetPoint.z, m_targetDistance/ONE_AU_IN_METERS, m_targetDistance);
    GPoint destination = m_position + (warp_vector * m_targetDistance);
    _log(DESTINY__WARP_TRACE, "DestinyManager::_InitWarp():Calculate - Entity %s(%u): calculated exit is %.2f,%.2f,%.2f and vector is %.4f,%.4f,%.4f.", \
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

    _log(DESTINY__WARP_TRACE, "DestinyManager::_WarpAccel(): Entity %s(%u) - Warp Accelerating(%us): velocity %.4f m/s with %.4f m left to go.  Current distance %.4f.", \
            mySE->GetName(), mySE->GetID(), sec_into_warp, currentShipSpeed, m_targetDistance, currentDistance);
}

void DestinyManager::_WarpCruise(uint16 sec_into_warp) {
    /* in cruise....calculate distance only to update internal postion data. */
    m_targetDistance -= m_warpState->warpSpeed;

    if ((m_targetDistance - m_warpState->warpSpeed) < m_warpState->decelDist) {
        m_warpState->cruise = false;
        m_warpState->decel = true;
    }

    _WarpUpdate(m_warpState->warpSpeed);

    _log(DESTINY__WARP_TRACE, "DestinyManager::_WarpCruise(): Entity %s(%u) - Warp Crusing(%us): velocity %.4f m/s. with %.4f m left to go.", \
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

    _log(DESTINY__WARP_TRACE, "DestinyManager::_WarpDecel(): Entity %s(%u) - Warp Decelerating(%us/%us): velocity %.4f m/s with %.4f m left to go.", \
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
                SetBubble(false);
            }
    } else if (!m_inBubble and m_warpState->decel) {
        if (m_targetDistance < BUBBLE_RADIUS_METERS) {    //this assumes target is center of bubble.  will have to fix one day.
            _log(DESTINY__WARP_TRACE, "DestinyManager::_WarpUpdate(): Entity %s(%u): Ship at %.2f,%.2f,%.2f is calling Add() .", \
                    mySE->GetName(), mySE->GetID(), m_position.x, m_position.y, m_position.z);
            sBubbleMgr.Add(mySE, true);
            SetPosition(m_position, true);
            /*
            DoDestiny_SetBallVelocity bv;
                bv.entityID = mySE->GetID();
                bv.x = m_velocity.x;
                bv.y = m_velocity.y;
                bv.z = m_velocity.z;
            PyTuple *up = bv.Encode();
            SendSingleDestinyUpdate(&up);
            */
            SetBubble(true);
        }
    }
}

void DestinyManager::_WarpStop(double currentShipSpeed) {
    _log(DESTINY__WARP_TRACE, "DestinyManager::_WarpStop(): Entity %s(%u) - Warp complete. Exit velocity %.4f m/s with %.4f m left to go.", \
            mySE->GetName(), mySE->GetID(), currentShipSpeed, m_targetDistance);
    _log(DESTINY__WARP_TRACE, "DestinyManager::_WarpStop(): Entity %s(%u): Ship currently at %.2f,%.2f,%.2f.", \
            mySE->GetName(), mySE->GetID(), m_position.x, m_position.y, m_position.z);

    m_targetPoint += (m_warpState->warp_vector *10000);
    //SetPosition(m_position, false);
    // SetSpeedFraction() checks for State = Warp and warpstate != null to set decel variables correctly with warp decel.
    //   have to call this BEFORE deleting or reseting State or WarpState.
    m_speedToLeaveWarp = m_maxShipSpeed *0.75f;
    SetSpeedFraction(0.0f);
    m_stop = true;
    SafeDelete(m_warpState);
}

//called whenever an entity is going away and can no longer be used as a target
void DestinyManager::EntityRemoved(SystemEntity *who) {
    if (m_targetEntity.second == who) {
        m_targetEntity.first = 0;
        m_targetEntity.second = nullptr;

        switch(State) {
            case DSTBALL_FOLLOW:
            case DSTBALL_ORBIT:
                _log(DESTINY__DEBUG, "%u: Our target entity has gone away. Stopping.", mySE->GetID());
                Stop();
                break;

        default:
            break;
        }
    }
}

bool DestinyManager::_IsTargetInvalid()
{
    if ( !mySE->SystemMgr()->get( m_targetEntity.first ) ) {
        // Our target was removed
        Stop();
        return true;
    }
    if (m_targetEntity.second->HasPilot()) {
        // our target is a player
        DynamicSystemEntity* targetClient = static_cast<DynamicSystemEntity *>(m_targetEntity.second);
        if ((!targetClient->GetPilot()->IsInSpace())   // Our target docked, so STOP
            or (m_targetEntity.first != targetClient->GetPilot()->GetShipID()) // The player is no longer in the ship we were targeting
            or (targetClient->DestinyMgr()->IsWarping())) {  // The target is warping
                mySE->TargetMgr()->ClearTarget(targetClient);
                Stop();
                return true;
            }
    } else if (m_targetEntity.second->IsNPCSE()) {
        // our target is a npc
        NPC* targetClient = m_targetEntity.second->GetNPCSE();
        if ((m_targetEntity.first != targetClient->GetID()) // The npc is no longer in the ship we were targeting
            or (targetClient->DestinyMgr()->IsWarping())) { // The target is warping
                mySE->TargetMgr()->ClearTarget(targetClient);
                Stop();
                return true;
            }
    }
    return false;
}

// Basic Movement Calls:
void DestinyManager::_BeginMovement() {
    // common movement for all types
    // reset turn and movement checks for possible heading change.
    m_stop = false;
    m_accel = false;
    m_decel = false;
    m_turning = false;
    m_stateStamp = sEntityList.GetStamp();
    if (m_shipHeading.isZero()) {
        GVector point(m_position);
        point.normalize();
        GVector moveVector(m_position, (point * 1.0e16));
        moveVector.normalize();
        m_shipHeading = moveVector;
    }
    // reset target distance just in case it changed.
    GVector shipVector(m_position, m_targetPoint);
    m_targetDistance = shipVector.length();
    // if ship is moving, reset accel checks
    if (m_userSpeedFraction)
        SetSpeedFraction(m_userSpeedFraction, true);
    else
        SetSpeedFraction(1.0f, true);

    /** @todo  this should be different from Cloak modules.... */
    if (IsCloaked()) UnCloak();
}

void DestinyManager::Follow(SystemEntity *who, double distance) {
    //called from client as 'CmdFollowBall'
    //  also used by 'Approach'
    if ((State == DSTBALL_FOLLOW) and (m_targetEntity.second == who) and (m_followDistance == distance) and (m_userSpeedFraction))
        return;

    State = DSTBALL_FOLLOW;
    m_targetPoint = who->GetPosition();
    m_targetEntity.first = who->GetID();
    m_targetEntity.second = who;
    m_followDistance = distance;
    _BeginMovement();

    DoDestiny_CmdFollowBall du;
        du.entityID = mySE->GetID();
        du.ballID = who->GetID();
        du.range = int32(distance);
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);    //consumed
}

void DestinyManager::Orbit(SystemEntity *who, double distance, bool update) {
    /*
     *    def Orbit(self, id, range = None):
     *        if id == session.shipid:
     *            return
     *        if range is None:
     *            range = self.GetDefaultDist('Orbit')
     *        bp = sm.StartService('michelle').GetRemotePark()
     *        if bp is not None and range is not None:
     *            name = sm.GetService('space').GetWarpDestinationName(id)
     *            range = float(range) if range < 10.0 else int(range)
     *            eve.Message('CustomNotify', {'notify': localization.GetByLabel('UI/Inflight/Orbiting', name=name, range=range)})
     *            bp.CmdOrbit(id, range)
     */
    if ((State == DSTBALL_ORBIT) and (m_targetEntity.second == who) and (m_followDistance == distance))
        return;

    State = DSTBALL_ORBIT;
    m_targetEntity.first = who->GetID();
    m_targetEntity.second = who;
    m_targetPoint = who->GetPosition();
    m_followDistance = distance;
    _BeginMovement();

    DoDestiny_CmdOrbit du;
        du.entityID = mySE->GetID();
        du.orbitEntityID = who->GetID();
        du.distance = int32(distance);
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);    //consumed
}

void DestinyManager::AlignTo(SystemEntity* ent) {
    //GotoPoint(ent->GetPosition());
    Follow(ent, 0);
}

void DestinyManager::GotoDirection(const GPoint& direction) {
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

void DestinyManager::WarpTo(const GPoint where, int32 distance) {
    /* warp order..
     * pick destination -> align/accel -> aura "warp drive active" -> cap drain -> accel
     *      -> enter warp -> warp -> decel -> leave warp -> coast -> stop
     */

    if (m_warpState)
        SafeDelete(m_warpState);

    /** @todo (allan) finish warp scramble system */
    if (m_warpStrength < 0/*m_warpScrambleSrength*/) {
        throw PyException(MakeUserError("WarpScrambled"));
    }

    GVector warp_distance(m_position, where);
    m_targetDistance = warp_distance.length();

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
            _log(DESTINY__WARNING, "DestinyManager::WarpTo() Client %s(%u): Capacitor needed vs current  %.3f / %.3f",
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

    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;
    m_stopDistance = distance;

    /*  TODO PUT CHECK HERE FOR WARP BUBBLES
     *     and other things that affect warp-in point.....when we get to there.
     * AttrWarpBubbleImmune = 1538,
     * AttrWarpBubbleImmuneModifier = 1539,
     *   NOTE:  warp bubble in path (or within 100km of m_targetPoint) will change m_targetDistance and m_targetPoint
     * not sure how to check for bubble yet...maybe keep them in vector based on system.
     */

    // start moving ship for alignment.
    GotoPoint(where);

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
        sfx.effect_type = "effects.Warping";
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

    _log(DESTINY__WARP_TRACE, "DestinyManager::WarpTo() m_targetPoint: %.2f,%.2f,%.2f  m_stopDistance: %i  m_targetDistance: %.4f",
         m_targetPoint.x, m_targetPoint.y, m_targetPoint.z, m_stopDistance, m_targetDistance);
}

bool DestinyManager::IsAligned(GPoint& targetPoint)
{
    if (m_shipHeading.isZero()) {
        GVector point(m_position);
        point.normalize();
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
    if (!mySE->IsMissileSE())
        State = DSTBALL_GOTO;
    m_stop = false;
    m_stateStamp = sEntityList.GetStamp();
    m_moveTimer = GetTimeMSeconds();
    m_shipMaxAccelTime = 0.1f;
    m_userSpeedFraction = 1.0f;
    m_currentSpeedFraction = 1.0f;
    m_maxSpeed = m_maxShipSpeed;
    m_velocity = m_shipHeading * m_maxSpeed;

    if (!mySE->IsMissileSE()) {
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
    SystemEntity *station = mySE->SystemMgr()->get(stationID);

    if (!station) {
        codelog(CLIENT__ERROR, "%s: Station %u not found.", pClient->GetName(), stationID);
        pClient->SendErrorMsg("Station Not Found, Docking Aborted.");
        return NULL;
    }

    //get the station Docking Perimiter
    const GPoint stationPos = station->GetPosition();
    double rangeToStationPerimiter = m_position.distance(stationPos);
    rangeToStationPerimiter -= mySE->GetRadius();
    rangeToStationPerimiter -= station->GetRadius();

    // Verify range to station is within docking perimeter of 2500 meters:
    _log(DESTINY__TRACE, "DestinyManager::AttemptDockOperation() rangeToStationPerimiter is %.2fm", rangeToStationPerimiter);
    if (rangeToStationPerimiter > 2500.0) {
        AlignTo( station );   // Turn ship and move toward docking point - client will call Dock() automatically when close enough...sometimes
        throw PyException(MakeUserError("DockingApproach"));
    }

    std::vector<PyTuple*> updates;
    DoDestiny_OnDockingAccepted oda;
        oda.ship_x = m_position.x;
        oda.ship_y = m_position.y;
        oda.ship_z = m_position.z;
        oda.station_x = stationPos.x;
        oda.station_y = stationPos.y;
        oda.station_z = stationPos.z;
        oda.stationID = stationID;
    PyTuple* ev = oda.Encode();
    // now send it, bypassing the extra shit and wrong dest name added in Client::SendNotification
    //ev->Dump(DESTINY__UPDATES, "");
    pClient->SendNotification("OnDockingAccepted", "charid", &ev);

    // per client packet sniff
    DoDestiny_SetBallMassive bm;
        bm.entityID = mySE->GetID();
        bm.is_massive = false;
    updates.push_back(bm.Encode());

    DoDestiny_CmdStop du;
        du.entityID = mySE->GetID();
    updates.push_back(du.Encode());
    SendDestinyUpdate(updates, true);

    Stop();
    m_dockTimer.Start(2000);  // start docking timer @ 2sec

    return nullptr;
}

void DestinyManager::SetPosition(const GPoint &pt, bool update /*false*/) {
    m_position = pt;

    //Relocate() needed to set InventoryItemRef.m_position correctly. (for all position references)
    mySE->GetSelf()->Relocate(pt);

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
void DestinyManager::SetMaxVelocity(double maxVelocity)
{
    double maxSpeed = 0;
    if (mySE->IsMissileSE() or mySE->IsNPCSE())
        maxSpeed = mySE->GetSelf()->GetAttribute(AttrMaxVelocity).get_float();
    else if (mySE->IsShipSE())
        maxSpeed = mySE->GetSelf()->GetAttribute(AttrMaxDirectionalVelocity).get_float();
    else
        ; // make error here?
    if (maxVelocity > maxSpeed)
        m_maxShipSpeed = maxSpeed;
    else
        m_maxShipSpeed = maxVelocity;
}

void DestinyManager::SetShipVariables(InventoryItemRef ship)
{
    m_radius = ship->GetAttribute(AttrRadius).get_float();
    //check for rigs and modules that affect radius here

    /* AttrMass = 4,    (largest mass = Leviathan(3764) @ 2,430,000,000kg)
     * AttrMassLimit = 622,
     * AttrMassAddition = 796,
     * AttrMassMultiplier = 1471,
     */
    m_mass = ship->GetAttribute(AttrMass).get_float();

    //  check for rigs and modules that modify mass here

    m_massMKg = m_mass / 1000000; //changes mass from Kg to MillionKg (10^-6)

    //  check for (and set) warp strength modifiers
    m_warpStrength = 1;
}

//  called from Client::BoardShip(), Undock(), NPC::NPC(), Concord::Concord()
void DestinyManager::SetShipCapabilities(InventoryItemRef ship, bool undock)
{
    /* this now sets variables needed for correct warp math.
     * noted modifiers to look into later, after everything is working
     * skill bonuses to ship attribs are now implemented, albeit crudely
     */

    SetShipVariables(ship);

    double warpCapNeed = ship->GetDefaultAttribute(AttrWarpCapacitorNeed).get_float();
    double adjInertiaModifier = ship->GetDefaultAttribute(AttrAgility).get_float();
    float adjShipMaxVelocity = ship->GetDefaultAttribute(AttrMaxVelocity).get_float();
    float warpSpeedMultiplier = 1.0f;
    float shipBaseWarpSpeed = 3.0f;

    // skill bonuses to agility and velocity and warpCapacitorNeed
    /*
     *    Advanced Spaceship Command: 5% agility bonus s per level on ships requiring this skill
     *    Capital Ships: 5% agility bonus s per level on ships requiring this skill
     *    Spaceship Command: 2% agility for all ships per level
     *    Evasive Maneuvering: 5% agility bonus for all ships per level
     *    Skirmish Warfare: 2% agility to fleet per skill level
     *    Skirmish warfare Mindlink (implant): 15% agility to fleet, replaces Skirmish warfare skill
     *    Warp Drive Operation (skill) (only listed because it affects how far you can warp) :–)
     */
    if (mySE->HasPilot()) {
        Character* pChar = mySE->GetPilot()->GetChar().get();
        if (!pChar) {
            _log(SHIP__WARNING, "ShipItem %s(%u) does not have a pilot. Destiny Variables will be inaccurate.", mySE->GetName(), mySE->GetID());
            return;
        }
        adjInertiaModifier *= pChar->GetAgilitySkills(ship->HasAttribute(AttrIsCapitalSize));
        adjShipMaxVelocity *= (1 + (0.05 * ( pChar->GetSkillLevel(skillNavigation, true))));
        shipBaseWarpSpeed = ship->GetAttribute(AttrBaseWarpSpeed).get_float();
        warpSpeedMultiplier = ship->GetAttribute(AttrWarpSpeedMultiplier).get_float();
        warpCapNeed *=  (1 - (0.1 * ( pChar->GetSkillLevel(skillWarpDriveOperation, true))));
        /** @todo check for implants  AttrWarpCapacitorNeedBonus(319) */
    } else {
        warpCapNeed = 0.00001f;
        adjInertiaModifier = 1.0f;
    }

    /*   look into these, too...
     * AttrWarpSBonus(624) [rigs and implants]
     * AttrWarpFactor(21) [all are 0]
     * AttrWarpInhibitor(29) [default is null]
     */

    //TODO  add module and rig modifiers to warp speed here
    m_shipWarpSpeed = ( warpSpeedMultiplier * shipBaseWarpSpeed );

    // TODO add module and rig bonuses to inertia, agility, velocity here

    m_maxShipSpeed = adjShipMaxVelocity;
    m_shipInertiaModifier = adjInertiaModifier;
    ship->SetAttribute(AttrAgility, adjInertiaModifier);
    ship->SetAttribute(AttrMaxVelocity, adjShipMaxVelocity);

    /*  per https://forums.eveonline.com/default.aspx?g=posts&m=3912843   post#103
     *
     * Ships will exit warp mode when their warping speed drops below
     * 50% of sub-warp max speed, or 100m/s, whichever is the lower.
     */
    // half isnt right.  change back to 3/4 maxSubWarpSpeed
    m_speedToLeaveWarp = m_maxShipSpeed *0.75;

    // TODO add module and rig bonuses to warp cap here

    m_warpCapacitorNeed = warpCapNeed;
    ship->SetAttribute(AttrWarpCapacitorNeed, warpCapNeed);

    /* The product of Mass and the Inertia Modifier gives the ship's agility
     * Agility = Mass x Inertia Modifier
     *   NOTE agility is an internal-use variable, and is NOT sent to the client.
     */
    //  this may not be right....
    m_shipAgility = m_massMKg * m_shipInertiaModifier;
    // set a maximum acceleration time (based on ship variables)
    m_shipMaxAccelTime = (m_shipAgility * -log(0.0001));

    //TimeToWarp = ln(2) × Mass × Inertia / 500000
    //TimeToWarp = -ln(0.25) x m_shipAgility
    m_alignTime = (log(2) * m_mass * m_shipInertiaModifier / 500000);
    m_timeToEnterWarp = m_alignTime;

    if (!mySE->HasPilot())
        return;
    if (mySE->GetPilot()->IsInSpace()) {
        std::vector<PyTuple*> updates;
        DoDestiny_SetBallAgility sbagility;
            sbagility.entityID =  mySE->GetID();
            sbagility.agility = mySE->GetSelf()->GetAttribute(AttrAgility).get_float();
        updates.push_back(sbagility.Encode());

        DoDestiny_SetBallMassive sbmassive;
            sbmassive.entityID = mySE->GetID();
            sbmassive.is_massive = true;
        updates.push_back(sbmassive.Encode());

        DoDestiny_SetBallMass sbmass;
            sbmass.entityID = mySE->GetID();
            sbmass.mass = mySE->GetSelf()->GetAttribute(AttrMass).get_float();
        updates.push_back(sbmass.Encode());

        SendDestinyUpdate(updates); //consumed
    }
}

void DestinyManager::MakeMissile(Missile* pMissile) {
    SetMaxVelocity(pMissile->GetSpeed());
    SetPosition(pMissile->GetSelf()->position());
    m_mass = pMissile->GetSelf()->type().mass();
    m_massMKg = m_mass / 1000000; //changes mass from Kg to MillionKg (10^-6)
    m_radius = pMissile->GetSelf()->type().radius();
    m_shipInertiaModifier = pMissile->GetSelf()->GetAttribute(AttrAgility).get_float();
    m_shipAgility = m_massMKg * m_shipInertiaModifier;

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

void DestinyManager::UpdateNewShip(const ShipItemRef newShipRef) const {
    std::vector<PyTuple*> updates;

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

void DestinyManager::UpdateOldShip(const ShipItemRef oldShipRef) const
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

void DestinyManager::SendJettisonPacket(const InventoryItemRef fromItemRef) const {
    DoDestiny_OnSpecialFX10 effect;
        effect.entityID = fromItemRef->itemID();
        effect.effect_type = "effects.Jettison";
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
    PyTuple* up = effect.Encode();
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendAnchorDrop(const InventoryItemRef fromItemRef) const {
    DoDestiny_OnSpecialFX10 effect;
        effect.entityID = fromItemRef->itemID();
        effect.effect_type = "effects.AnchorDrop";
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
    PyTuple* up = effect.Encode();
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendAnchorLift(const InventoryItemRef fromItemRef) const {
    DoDestiny_OnSpecialFX10 effect;
        effect.entityID = fromItemRef->itemID();
        effect.effect_type = "effects.AnchorLift";
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

void DestinyManager::SendCloakShip(const bool IsWarpSafe) const {
    DoDestiny_OnSpecialFX10 effect;
        effect.effect_type = "effects.Cloak";
        effect.entityID = mySE->GetID();
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
    PyTuple *up = effect.Encode();
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendUncloakShip() const {
    DoDestiny_OnSpecialFX10 effect;
        effect.effect_type = "effects.Uncloak";
        effect.entityID = mySE->GetID();
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
    PyTuple *up = effect.Encode();
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendSpecialEffect10(uint32 entityID, const ShipItemRef shipRef, uint32 targetID,
                                         std::string effectString, bool isOffensive, bool start, bool isActive) const
{
	std::vector<int32, std::allocator<int32> > area;    //TODO need to figure out what this is....

    DoDestiny_OnSpecialFX10 effect;
        effect.entityID = entityID;
        effect.targetID = targetID;
        effect.effect_type = effectString;
        effect.area = area;
        effect.isOffensive = isOffensive;
        effect.start = start;
        effect.active = isActive;
    PyTuple *up = effect.Encode();
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendSpecialEffect(const ShipItemRef shipRef, uint32 moduleID, uint32 moduleTypeID, uint32 targetID,
                                       uint32 chargeTypeID, std::string effectString, bool isOffensive, bool start,
                                       bool isActive, double duration, uint32 repeat) const
{
    std::vector<int32, std::allocator<int32> > area;    //TODO need to figure out what this is....

    DoDestiny_OnSpecialFX13 effect;
        effect.entityID = shipRef->itemID();
        effect.moduleID = moduleID;
        effect.moduleTypeID = moduleTypeID;
        effect.targetID = targetID;
        effect.otherTypeID = chargeTypeID;
        effect.area = area;
        effect.effect_type = effectString;
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
        effect.effect_type = "effects.JumpOut";
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
    PyTuple *up = effect.Encode();
    SendSingleDestinyUpdate(&up);
}

void DestinyManager::SendGateActivity(uint32 gateID) const {
    DoDestiny_OnSpecialFX10 du;
        du.entityID = gateID;
        du.effect_type = "effects.GateActivity";
        du.isOffensive = 0;
        du.start = 1;
        du.active = 0;
    PyTuple* up = du.Encode();
    SendSingleDestinyUpdate(&up);    //consumed
}

void DestinyManager::SendBallInteractive(const ShipItemRef shipRef, bool set) const {
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
    effect.effect_type = "effects.JumpDriveOut";   /* JumpDriveInBO */
    effect.isOffensive = 0;
    effect.start = 1;
    effect.active = 0;
    updates.push_back(effect.Encode());

    SendDestinyUpdate(updates);
}

void DestinyManager::SendJumpInEffect(std::string JumpEffect) const {
    std::vector<PyTuple*> updates;

    DoDestiny_OnSpecialFX10 effect;
    effect.effect_type = "effects.JumpDriveIn";
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

void DestinyManager::SendTerminalExplosion(uint32 shipID, uint32 bubbleID, bool isGlobal) const {
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
        _log(PLAYER__MESSAGE, "[%u] DestinyManager::SendDestinyUpdate() (u:%lu, e:%lu) called as 'self_only' for %s(%u)", \
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
        _log( DESTINY__UPDATES, "[%u] BubbleCasting destiny update (u:%lu, e:%lu)", sEntityList.GetStamp(), updates.size(), events.size() );
        mySE->SysBubble()->BubblecastDestiny( updates, events, "destiny" );
    } else {
        _log( DESTINY__UPDATES, "[%u] Cannot BubbleCast destiny update (u:%lu, e:%lu); entity (%u) is not in any bubble.", \
                    sEntityList.GetStamp(), updates.size(), events.size(), mySE->GetID() );
    }
}
