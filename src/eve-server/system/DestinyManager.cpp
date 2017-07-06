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

// this class is for objects that move

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


DestinyManager::DestinyManager(SystemEntity *self)
: mySE(self),
m_maxSpeed(1.0f),
m_shipMaxAccelTime(0.0f),
State(Destiny::BallMode::DSTBALL_STOP),
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
    m_degPerTic = 0.0f;
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

    m_turnTic = 0;
    m_turning = false;
    m_turnFraction = 0;
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
    using namespace Destiny;
    switch(State) {
        case BallMode::DSTBALL_STOP: {
            if (IsMoving()) {
                MoveObject();
                break;
            }
            Stop();
        } break;
        case BallMode::DSTBALL_GOTO: {
            MoveObject();
        } break;
        case BallMode::DSTBALL_MISSILE: {
            // if target was removed, continue movement and wait for Missile::EndOfLife() call to do cleanup
            //set current direction based on position and targetPoint.  this will keep missile aligned properly
            GVector moveVector(m_position, m_targetPoint);
            moveVector.normalize();
            //set position and direction for this round of movement
            m_shipHeading = moveVector;
            m_velocity = (moveVector * m_maxSpeed);
            SetPosition(m_position + m_velocity);
        } break;
        case BallMode::DSTBALL_ORBIT: {
            if (IsTargetInvalid())
                return;
            _Orbit();
        } break;
        case BallMode::DSTBALL_FOLLOW: {
            if (IsTargetInvalid())
                return;
            _Follow();
        } break;
        case BallMode::DSTBALL_WARP: {
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
            } else if ((sEntityList.GetStamp() - m_stateStamp) > m_timeToEnterWarp) {
                // catchall for turn checks messed up, and m_moveTimer > ship align time
                _log(DESTINY__ERROR, "Destiny::ProcessState() Error!  Ship %s(%u) for Player %s(%u) - warp align/speed is incorrect, but time > shipTimeToWarp.",  \
                            mySE->GetName(), mySE->GetID(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());
                m_shipHeading = toVec;
                _InitWarp();
                return;
            }

            MoveObject();
        } break;
        case BallMode::DSTBALL_MUSHROOM:      // aoe?
        case BallMode::DSTBALL_BOID:          // this will turn RIGID after a set time
        case BallMode::DSTBALL_TROLL:         // seen for wrecks
        case BallMode::DSTBALL_MINIBALL:      // used for sentrys
        case BallMode::DSTBALL_FIELD:         // dunno
        case BallMode::DSTBALL_FORMATION:     // dunno
        case BallMode::DSTBALL_RIGID:         // item that never moves
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
        if (State == Destiny::BallMode::DSTBALL_STOP)
            State = Destiny::BallMode::DSTBALL_GOTO;
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
     *  m_activeSpeedFraction (ASF) holds ship's current speed setting.  set in MoveObject()
     *   this is actually a ratio of CSF to USF, capped by USF and/or OSF
     *   -> sets accel/decel when changing speeds while moving
     *  m_currentSpeedFraction (CSF) holds current euler value for time.  set in MoveObject()
     *   this is reset on initial turn based on turn angle, then follows normal progression.
     *   this is the only SpeedFraction value that runs the full range of 0 - 1 no matter value of other SpeedFractions.
     *   -> sets m_velocity
     *  m_maxOrbitSpeedFraction (OSF) is ship's max speed based on orbit data
     *   -> modifies m_velocity
     *  m_maxSpeed is ship maximum speed based on user input.  set in UpdateVelocity()
     *   -> sets m_velocity
     *  m_targetPoint holds current target coords.  set by goto, warp, align, follow, orbit
     *   -> sets m_shipHeading
     *  m_shipHeading holds current direction and is set in _Turn()
     *   -> sets m_velocity
     *  m_velocity is current ship velocity.  set in MoveObject()
     *   m_velocity = m_shipHeading * m_currentSpeedFraction * m_maxSpeed
     */
    if ((m_shipMaxAccelTime < 1.0) and (mySE->IsDynamicEntity()))
        if (!mySE->HasPilot()) {
            m_shipMaxAccelTime = (-log(0.0001) * m_shipAgility);
        } else if (!mySE->GetPilot()->IsUndock()) {
            m_shipMaxAccelTime = (-log(0.0001) * m_shipAgility);
        }

    if ((m_userSpeedFraction) and ((!fraction) or (m_prevSpeed) or (fraction != m_userSpeedFraction)))
        m_prevSpeedFraction = m_userSpeedFraction;
    else
        m_prevSpeedFraction = 0.0f;

    m_userSpeedFraction = fraction;
    bool isMoving = false;
    if ((m_currentSpeedFraction > 0.05) or (m_activeSpeedFraction > 0.05))
        isMoving = true;
    UpdateVelocity(isMoving);

    if (State == Destiny::BallMode::DSTBALL_WARP) {
        // set state to DSTBALL_GOTO after setting warp decel variables, so warp completion will decel properly
        State = Destiny::BallMode::DSTBALL_GOTO;
        return;
    }

    std::vector<PyTuple*> updates;
    updates.clear();
    if (fraction) {
        DoDestiny_CmdSetSpeedFraction du;
            du.entityID = mySE->GetID();
            du.fraction = fraction;
        updates.push_back(du.Encode());
    }
    if (mySE->IsNPCSE() or mySE->IsMissileSE() or mySE->IsContainerSE() or mySE->IsWreckSE()) {
        DoDestiny_SetMaxSpeed ms;   //NPCs and Missiles only.
            ms.entityID = mySE->GetID();
            ms.speedValue = m_maxSpeed;
        updates.push_back(ms.Encode());
    }

    if (!updates.empty())
        SendDestinyUpdate(updates);
}

void DestinyManager::UpdateVelocity(bool isMoving) {
    uint8 logType = 0;
    if ((State == Destiny::BallMode::DSTBALL_WARP) and m_warpState) {
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
    } else if (m_userSpeedFraction) {   //moving
        if (isMoving) { //change speed
            if ((m_activeSpeedFraction == m_userSpeedFraction) and (!m_prevSpeed))
                return;
            //  the times are a bit off when usf < 1.0, but acceptable for now.  will revisit later.    -allan 21Nov15
            float delta = 1.0f;
            logType = 2;
            if (m_turning) {
                if (m_decel) {
                    logType = 4;
                    m_accel = true;
                    m_decel = false;
                    m_prevSpeed = 0;
                    m_prevSpeedFraction = 0;
                    m_activeSpeedFraction = m_currentSpeedFraction;
                    delta = 1 - m_currentSpeedFraction;
                } else {
                    logType = 3;
                    m_decel = true;
                    m_accel = false;
                    m_moveTimer = GetTimeMSeconds();
                    m_prevSpeedFraction = m_currentSpeedFraction;
                    delta = m_turnFraction / m_currentSpeedFraction;
                }
            } else if (m_decel) {
                m_accel = true;
                m_decel = false;
                m_currentSpeedFraction = 1 - m_activeSpeedFraction;    // reset csf
                m_activeSpeedFraction = m_currentSpeedFraction;
            } else if (m_prevSpeed) {
                // decel from deactivated prop mod
                m_decel = true;
                m_accel = false;
            } else if (m_userSpeedFraction < m_activeSpeedFraction) {
                // decrease usf
                m_decel = true;
                m_accel = false;
                //get current ship speed and reset for speed change
                m_prevSpeed = m_maxSpeed * m_currentSpeedFraction;
                delta = m_activeSpeedFraction - m_userSpeedFraction;
            } else {
                // increase usf
                m_accel = true;
                m_decel = false;
                delta = m_userSpeedFraction - m_activeSpeedFraction;
            }

            m_shipMaxAccelTime *= delta;
            // will need a test here for (prevMaxSpeed > m_maxSpeed) to set ship velocity correctly (and avoid negatives)
            if (m_prevSpeed > m_maxSpeed)
                m_velocity = m_shipHeading * m_maxSpeed * m_activeSpeedFraction;

            if ((!m_turning) and (!m_decel)) {
                m_maxSpeed = m_maxShipSpeed * m_userSpeedFraction;
                m_velocity = m_shipHeading * m_maxSpeed * m_activeSpeedFraction;
            }
        } else {    //begin movement
            logType = 5;
            m_accel = true;
            m_decel = false;
            m_shipMaxAccelTime *= m_userSpeedFraction;      // for accel with user speeds <= 1.0
            m_maxSpeed = m_maxShipSpeed * m_userSpeedFraction;
            //  see notes in MoveObject() for information relating to accel equations
            m_currentSpeedFraction = (1 - exp(-0.01 / m_shipAgility));
            m_velocity = m_shipHeading * m_maxSpeed * m_currentSpeedFraction;
        }
        _log(DESTINY__MOVE_TRACE, "Destiny::UpdateVelocity - %s(%u): Speed Change - USF: %.2f, ASF: %.2f, CSF: %.2f, PSF: %.2f, prevSpeed: %.2f", \
                 mySE->GetName(), mySE->GetID(), m_userSpeedFraction, m_activeSpeedFraction, m_currentSpeedFraction, m_prevSpeedFraction, m_prevSpeed);
    } else if ((m_currentSpeedFraction) or (m_prevSpeedFraction)) {
        if (isMoving) { //stop movement
            logType = 6;
            // will need a test here for (prevMaxSpeed > m_maxSpeed) to set ship velocity correctly (and avoid negatives)
            //  see notes in MoveObject() for information relating to decel equations
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
            logType = 7;
            Halt();
        }
    } else {
        //WARNING conditional should never arrive here.
        logType = 8;
        //assert(0);  // change this to a correct error-handling implementation
    }
    std::string msg = "";
    switch (logType) {
        case 1: { msg = "state == warp.  --Begin Decel"; } break;
        case 2: { msg = "USF != 0 and ship isMoving.  --Different Heading or Speed."; } break;
        case 3: { msg = "USF != 0 and ship isMoving.  --Decel for Turn."; } break;
        case 4: { msg = "USF != 0 and ship isMoving.  --Accel after Turn."; } break;
        case 5: { msg = "USF != 0 and ship stopped.   --Begin Accel"; } break;
        case 6: { msg = "USF == 0 and ship isMoving.  --Stop"; } break;
        case 7: { msg = "USF == 0 and ship stopped.  --Halt"; } break;
        case 8: {
            _log(DESTINY__ERROR, "Destiny::UpdateVelocity Error!  Ship %s(%u) Has No WarpState or Speed Fraction.",
                                    mySE->GetName(), mySE->GetID());
            Halt();
            return;
        } break;
    }
    _log(DESTINY__MOVE_TRACE, "Destiny::UpdateVelocity - %s(%u):  %s.  AccelTime: %.2f, USF: %.2f, ASF: %.2f, CSF: %.2f, PSF: %.2f", \
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
    } else if  ((State == Destiny::BallMode::DSTBALL_WARP) and (!IsWarping()))  {
        //warp aborted before initalized.  standard Stop() applies.
        State = Destiny::BallMode::DSTBALL_STOP;
    } else if (IsMoving()) {
        //stop called while moving
        State = Destiny::BallMode::DSTBALL_STOP;
    }

    m_accel = false;
    m_decel = false;
    m_orbiting = 0;
    m_prevSpeed = 0.0f;
    m_orbitRadTic = 0.0f;
    m_prevSpeedFraction = 0.0f;

    ClearTurn();
    // reset move timers for new state
    m_moveTimer = GetTimeMSeconds();
    m_stateStamp = sEntityList.GetStamp();
    //reset max accel time in case it was changed previously
    m_shipMaxAccelTime = (-log(0.0001) * m_shipAgility);

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
     State = Destiny::BallMode::DSTBALL_STOP;
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

    ClearTurn();

    if ((m_shipMaxAccelTime < 1.0) and (mySE->IsDynamicEntity()))
        m_shipMaxAccelTime = (-log(0.0001) * m_shipAgility);

    _log(DESTINY__MOVE_TRACE, "Destiny::Halt() - %s(%u): m_shipHeading: %.3f,%.3f,%.3f", \
        mySE->GetName(), mySE->GetID(), m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);
}

// Global collision methods
//  check for collision.  called by Move()
void DestinyManager::CheckBump()
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
            Bump(cur->GetShipSE());
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

void DestinyManager::Bump(SystemEntity* pSE)
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

void DestinyManager::Bounce(GVector direction, float speed)
{
    // bounce code here
    /*  this code will update ship movement after being bumped
     *  all items will drift to a complete stop, unless other movement is called.
     */
    State = Destiny::BallMode::DSTBALL_GOTO;
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
void DestinyManager::MoveObject() {
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
     *   m_shipMaxAccelTime = (-log(0.0001) * m_shipAgility);
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

    /* **UPDATE**  this now uses time AND (m_currentSpeedFraction > 0.9999f) for min/max speeds.  -allan 6Aug14
     * **UPDATE**  this is now tracking ALL speed changes correctly.  -allan 21Nov15
     * **UPDATE**  initial orbit implementation.  -allan 13July16
     * **UPDATE**  removed speed fraction checks for min/max speeds.  -allan 02Jul17
     */

    double timeStamp = 0;   // keep all these timers in seconds.
    if ((m_orbiting != 1) and m_userSpeedFraction)  // if usf==0 then ship is stopping, so continue movement along current ship heading (cancel turn)
        Turn();
    timeStamp = (GetTimeMSeconds() - m_moveTimer) /1000;

    float speed = 0.0f;
    std::string move = "";
    // check to make sure we dont overrun usf/asf
    if (m_activeSpeedFraction == m_userSpeedFraction)
        m_currentSpeedFraction = 1.0f;
    if (timeStamp > m_shipMaxAccelTime) { // and ((m_currentSpeedFraction > 0.9999f) or (m_activeSpeedFraction > m_prevSpeedFraction)))
        m_accel = m_decel = false;
        if (m_prevSpeed) {
            speed = m_prevSpeed - ((m_prevSpeed - m_maxSpeed) * m_currentSpeedFraction);
            _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u) has deceled from %.2fm/s to %.2fm/s of max %.2f in %.3fs.", \
                    mySE->GetName(), mySE->GetID(), m_prevSpeed, speed, m_maxSpeed, timeStamp);
            m_prevSpeed = 0.0f;
            m_prevSpeedFraction = 0.0f;
            m_shipMaxAccelTime = (-log(0.0001) * m_shipAgility);
        }
        if (m_userSpeedFraction) {
            // ship has reached full speed (whatever the fraction was set to)
            m_currentSpeedFraction = 1.0f;
            m_activeSpeedFraction = m_userSpeedFraction;
            move = "at max speed, going";
            speed = m_maxSpeed * m_activeSpeedFraction;
        } else {
            //ship has reached full stop
            // update position one final time (for last bit of drift) and exit movement functions by calling Halt()
            SetPosition(m_position + m_velocity, sConfig.server.PositionHack);
            _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u) is at full stop after %.3f seconds.", \
                    mySE->GetName(), mySE->GetID(), timeStamp);
            Halt();
            return;
        }
    } else {
        //not full speed yet or has changed speed
        if (m_turning)
            move = "turning";
        else
            move = "accelerating";

        //if ship is turning, DO NOT reset CSF here until AFTER initial turn decel.
        //if ((!m_turning) or (m_turnTic > (m_shipAgility * 0.45)))  //normal accel
            m_currentSpeedFraction = (1 - exp(-timeStamp / m_shipAgility));
            //m_currentSpeedFraction = (1 - exp(-timeStamp * 1000000 / (m_shipInertia * m_mass)));

        if (m_accel)
            m_activeSpeedFraction = m_userSpeedFraction * m_currentSpeedFraction;
        else if (m_decel)
            m_activeSpeedFraction = m_prevSpeedFraction * m_currentSpeedFraction;
        else if (m_userSpeedFraction != m_activeSpeedFraction)
            m_activeSpeedFraction = m_currentSpeedFraction;
        else if ((m_tractored) or (m_tractorPause) or (m_activeSpeedFraction == 1))
            ;   // do nothing here.  this is to remove error reporting from next line.
        else
            _log(DESTINY__ERROR, "Destiny::MoveObject() - %s(%u) - move checks are not set right. Acc:%s, Dec:%s, Turn:%s, Tic:%u, Tractored:%s, TractorPause:%s", \
                    mySE->GetName(), mySE->GetID(), (m_accel ? "True" : "False"), (m_decel ? "True" : "False"), (m_turning ? "True" : "False"), \
                    m_turnTic, (m_tractored ? "True" : "False"), (m_tractorPause ? "True" : "False"));

        if (m_prevSpeed) {
            if (m_prevSpeedFraction and (m_prevSpeedFraction == m_userSpeedFraction))
                speed = m_prevSpeed * m_activeSpeedFraction;
            else if (m_turning)
                speed = m_maxSpeed * m_activeSpeedFraction;
            else if (m_prevSpeed > m_maxSpeed)
                speed = (m_prevSpeed - m_maxSpeed) * m_activeSpeedFraction;
            else if (m_prevSpeed < m_maxSpeed)
                speed = (m_maxSpeed - m_prevSpeed) * m_activeSpeedFraction;
            else
                speed = m_maxSpeed * m_activeSpeedFraction;
        } else
            speed = m_maxSpeed * m_activeSpeedFraction;

        if ((!m_userSpeedFraction) or (m_decel)) {
            if (m_turning) {
                // decel for turn
                move = "decelerating for turn";
            } else {
                //just decelerating
                move = "decelerating";
            }
            //m_activeSpeedFraction = m_currentSpeedFraction;
            if (m_prevSpeed)
                speed = m_prevSpeed - speed;
            else
                speed = m_maxSpeed - speed;
            m_currentSpeedFraction = 1 - m_activeSpeedFraction;
        }
    }
    // ships tend to "level out" when stopping.  try to mimic that here
    if ((m_stop) and (m_currentSpeedFraction < 0.4)) {
        if (m_shipHeading.y < -0.3)
            m_shipHeading.y += 0.08;
        else if (m_shipHeading.y > 0.3)
            m_shipHeading.y -= 0.08;
    }

    if (m_orbiting) {
        // object is orbiting...set orbit speed correctly.
        speed *= m_maxOrbitSpeedFraction;
        move += " in orbit";
    }

    if ((m_prevSpeed) or (m_prevSpeedFraction)) {
        _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u) is %s at %.4f m/s (csf:%.4f asf:%.4f pSpeed:%.2f(%.3f), sec: %.3f).", \
                mySE->GetName(), mySE->GetID(), move.c_str(), speed, m_currentSpeedFraction, m_activeSpeedFraction, m_prevSpeed, m_prevSpeedFraction, timeStamp);
    } else {
        _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u) is %s at %.4f m/s (csf:%.4f asf:%.4f  sec: %.3f).", \
                mySE->GetName(), mySE->GetID(), move.c_str(), speed, m_currentSpeedFraction, m_activeSpeedFraction, timeStamp);
    }

    //set speed, direction and position for this round of movement
    m_velocity = m_shipHeading * speed;
    SetPosition(m_position + m_velocity, sConfig.server.PositionHack);   // (PositionHack == true) here will force position update to client

    //_log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u) Pos:%.2f,%.2f,%.2f  Vel:%.3f,%.3f,%.3f  Head:%.3f,%.3f,%.3f", \
            mySE->GetName(), mySE->GetID(), m_position.x, m_position.y, m_position.z, m_velocity.x, m_velocity.y, m_velocity.z,\
            m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);

    if (sConfig.cosmic.BumpEnabled)
        if (mySE->HasPilot() and mySE->SysBubble()->HasPlayers()) // no players in bubble = nothing to check against (for now)
            CheckBump();

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

bool DestinyManager::IsTurn() {    //this is working.  dont change
    if (m_targetPoint.isZero()) {
        _log(DESTINY__ERROR, "Destiny::IsTurn() - %s(%u): TargetPoint is null.", mySE->GetName(), mySE->GetID());
        ClearTurn();
        Halt();
        return false;
    }
    // check for turning angle.  returns true if angle is enough to change movement variables
    // create isosceles triangle where legs are current direction and destination, then find angle between legs
    //  it will set m_radians in the range of [-pi,pi].
    GVector toVec(m_position, m_targetPoint);
    toVec.normalize();
    float dot = toVec.dotProduct(m_shipHeading);
    if ((dot > 1.0f) or (dot < -1.0f)) {
        sLog.Error("Destiny::IsTurn()", "%s(%u) - shipHeading has screwed up.  dot is %.5f", mySE->GetName(), mySE->GetID(), dot);
        _log(DESTINY__ERROR, "Destiny::IsTurn() m_shipHeading: %.3f,%.3f,%.3f.  m_targetHeading: %.3f,%.3f,%.3f, toVec:%.3f,%.3f,%.3f", \
                m_shipHeading.x, m_shipHeading.y, m_shipHeading.z, m_targetHeading.x, m_targetHeading.y, m_targetHeading.z, toVec.x, toVec.y, toVec.z);
        // try to correct for bad heading vector and retest...
             if (m_shipHeading.x > 1.0f)  m_shipHeading.x -= 1;
        else if (m_shipHeading.x < 1.0f)  m_shipHeading.x += 1;
             if (m_shipHeading.y > 1.0f)  m_shipHeading.y -= 1;
        else if (m_shipHeading.y < 1.0f)  m_shipHeading.y += 1;
             if (m_shipHeading.z > 1.0f)  m_shipHeading.z -= 1;
        else if (m_shipHeading.z < 1.0f)  m_shipHeading.z += 1;
        dot = toVec.dotProduct(m_shipHeading);
        if ((dot > 1.0f) or (dot < -1.0f)) {
            sLog.Error("Destiny::IsTurn()", "%s(%u) - shipHeading has screwed up AGAIN.  dot is %.5f", mySE->GetName(), mySE->GetID(), dot);
            return false;
        }
    }
    m_radians = acos(dot);
    float degrees = EvE_RadiansToDegrees(m_radians);
    if (degrees < TURN_ALIGNMENT/*4*/) {
        m_shipHeading = toVec;
        return false;
    }
    _log(DESTINY__TURN_TRACE, "Destiny::IsTurn() - %s(%u): dot: %.5f, radians:%.5f, degrees:%.3f",\
            mySE->GetName(), mySE->GetID(), dot, m_radians, degrees);
    //_log(DESTINY__TURN_TRACE, "Destiny::IsTurn() m_shipHeading: %.3f,%.3f,%.3f.  m_targetHeading: %.3f,%.3f,%.3f", \
            m_shipHeading.x, m_shipHeading.y, m_shipHeading.z, m_targetHeading.x, m_targetHeading.y, m_targetHeading.z);
    return true;
}

void DestinyManager::Turn() {   // tracking within 900m for Frigates, 1k4m for BS.  05Jun17
    if (mySE->HasPilot())
        if (mySE->GetPilot()->IsUndock())
            return;

    if (!IsTurn()) {
        if (m_turning)
            ClearTurn();
        return;
    }
    /*when changing directions....
     *  m_moveTimer will have to be reset - call UpdateVelocity() when turn starts
     *  m_shipHeading will have to be reset - reset here and used in MoveObject() (our calling function)
     *  check for decel, then call UpdateVelocity() to set variables as needed.  Move() will handle the rest.
     *
     *   m_degPerTic = (65.0f - m_shipAgility) /10;  (this file, set at 2317, reset for ab/mwd at 2154)
     */

    float turnTime = (m_shipAgility /2);
    if (!m_turning) {
        m_turning = true;
        //m_radians is set in IsTurn() on every tic
        m_turnFraction = sqrt((cos(m_radians) + 1) /2);
        //this isnt used yet...used as comparison for testing time calc's
        m_alignTime = (EvE_RadiansToDegrees(m_radians) / m_degPerTic);
        _log(DESTINY__TURN_TRACE, "Destiny::Turn() - %s(%u): Agility:%.3f, Inertia:%.3f, alignTime:%.3f, turnTime:%.3f, turnFraction:%.3f, m_degPerTic:%.3f", \
                mySE->GetName(), mySE->GetID(), m_shipAgility, m_shipInertia, m_alignTime, turnTime, m_turnFraction, m_degPerTic);
    }

    ++m_turnTic;

    // logic to determine speed changes for turning
    if (m_turnTic == 1)
        if (m_turnFraction < m_currentSpeedFraction)
            UpdateVelocity(true);

    // need to check turnFraction vs m_currentSpeedFraction to hold speed when turning.

    /*  class          agility
     * Capsule          .06
     * Shuttle          1.6
     * Rookie           5
     * Frigates         3 - 6 (adv. 3 - 4)  (2s) 1s         < 0.15 not enough.
     * Destroyers       4 - 5
     * Cruisers         4 - 8
     * T3 Cruiser       2.4 - 2.8
     * HAC              5 - 7
     * Battlecruisers   6 - 9
     * Battleships      8 - 14      (12s) 4s        0.15 works well  0.2 is very well.   > 0.25 is too much.
     * Industrials      8 - 12
     * Marauder         ~12
     * Orca             40          (40s) 18s   0.05 turnPercent seems to work very well.  > 0.1 is wrong.
     * Freighters       ~60
     * Supercarrier     ~60
     * Command          ~9
     * Transport        5 or 19
     * Barges           10 - 18
     * Dreadnought      ~55
     * Zephyr           5
     */
    // set ship turn amount based on position in turn, current speed and ship agility
    GVector deltaHeading(m_shipHeading, m_targetHeading);
    float turnPercent = 0.1f;
    float degrees = EvE_RadiansToDegrees(m_radians);
    if (degrees > 100) {
        if (m_decel and (m_turnTic > turnTime)) {
            // turn half of remaining turn (simulate greatest turn angle when (turn > 90*) and (speed < time)
            turnPercent = 0.4f;
        } else {
            turnPercent = m_degPerTic / (degrees -100);
        }
    } else if (degrees > m_degPerTic) {
        turnPercent = m_degPerTic / degrees;
    } else {
        // degrees < m_degPerTic, so complete turn and continue accel
        if (m_decel)
            UpdateVelocity(true);
    }

    if (turnPercent > 1.0) {
        _log(DESTINY__ERROR, "Destiny::Turn() - turnTic:%u, degRemain:%.3f, turnPercent:%.2f", m_turnTic, degrees, turnPercent);
        turnPercent = 0.9;
    }
    deltaHeading *= turnPercent;
    m_shipHeading += deltaHeading;
    _log(DESTINY__TURN_TRACE, "Destiny::Turn() - csf:%.3f, turnTic:%u, degRemain:%.3f  (deltaHeading:%.5f, %.5f, %.5f * turnPercent:%.2f) = shipHeading:%.3f, %.3f, %.3f", \
            m_currentSpeedFraction, m_turnTic, degrees, deltaHeading.x, deltaHeading.y, deltaHeading.z, turnPercent, m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);
}

void DestinyManager::ClearTurn() {
    SetPosition(m_position, sConfig.server.PositionHack);
    m_turnTic = 0;
    m_turning = false;
    m_radians = 0.0f;
    m_turnFraction = 0.0f;
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
            m_tractorPause = false;
            m_velocity = m_shipHeading * m_maxSpeed;
            m_moveTimer = GetTimeMSeconds();
            m_stateStamp = sEntityList.GetStamp();
            m_prevSpeedFraction = 0.0f;
            // there is no accel/decel for tractor'd items
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

    MoveObject();
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
     * m_targetDistance - commanded orbit distance
     * m_followDistance - calculated orbit distance, including target gravity and ship variables
     * m_targetHeading - direction to target from current heading
     * m_targetPoint - calculated distant point from above variable
     * m_shipHeading - current direction ship is pointed
     * m_stateStamp - used to track quadrant and positioning.  is tic in seconds
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
     * Tm = target mass
     * Cd = current direction
     *
     * current = distance between object and target centers
     * actual = distance between object and target closest edges
     * near = fraction of actual / m_targetDistance (commanded)
     * far = fraction of actual / m_followDistance (calculated)
     */

    // get current times
    uint32 timeStamp = sEntityList.GetStamp() - m_stateStamp;
    float Tr =  m_targetEntity.second->GetRadius();
    float Tm = m_targetEntity.second->GetSelf()->GetAttribute(AttrMass).get_float();
    GPoint Tp(m_targetEntity.second->GetPosition());
    // only dynamic entites have a destiny manager, others will return 0.
    //  this still needs testing
    float Tv = (m_targetEntity.second->DestinyMgr() ? m_targetEntity.second->DestinyMgr()->GetSpeed() : 0);
    GVector Th(m_targetEntity.second->DestinyMgr() ? m_targetEntity.second->DestinyMgr()->GetHeading() : NULL_ORIGIN_V);
    Tp += (Tv*Th); // use Tv*Th and add to position to account for target movement.  Tv for non-moving targets return 0.

    // current and actual are used to determine ship's orbit distance, and adjust position accordingly
    double current = m_position.distance(Tp);
    double actual = (current - m_radius - Tr);
    float near = actual / m_targetDistance, far = current / m_followDistance;
    _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - current:%.2f, actual:%.2f, target:%.2f, follow:%.2f, near:%.3f, far:%.3f", \
            current, actual, m_targetDistance, m_followDistance, near, far);

    // distances for orbit calculations for orbits within engage distance
    double orbitDistNow = 0.0f, orbitDistNext = 0.0f, curSpeed = m_maxSpeed * m_activeSpeedFraction * m_maxOrbitSpeedFraction;

    GPoint mPos(NULL_ORIGIN);
    double curRad = m_orbitRadTic * timeStamp;  // this isnt quite right...but pretty damn close
    // adjust 'distance' variable as needed to correct orbit circumfrence based on target distance
    if (far > 2.5) { //far = current / m_followDistance
        // too far to engage target.
        _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - way too far");
        m_orbiting = 5;
        // set point to side of target (based on current position), to avoid near-zero angular velocity
        double radTarg = atan2(Tp.z - m_position.z, Tp.x - m_position.x);  // rad from '0' to target
        radTarg += atan2(m_targetDistance, current);  // rad from 'distance line' to target 'offset'
        mPos.x = current * cos(radTarg);
        mPos.z = current * sin(radTarg);
        mPos.y = 0;
        m_targetPoint = m_position + mPos;
        m_targetPoint.y = Tp.y;     // stay on 'y' elevation...easier this way.
        GVector heading(m_position, m_targetPoint);
        heading.normalize();
        m_shipHeading = heading;    // this sets object velocity in MoveObject() (using speed)
        MoveObject();
        return; // this is all we need to do at this point.
    } else if (near < 0.6) {  // near = actual / m_targetDistance
        // way too close inside orbit.  move away quickly.
        _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - way too close");
        m_orbiting = 2;
        GVector heading(m_position, Tp);
        heading.normalize();
        m_shipHeading = heading * -1;    // this sets object velocity in MoveObject() (using speed)
        m_targetPoint = m_position + (m_shipHeading * 1.0e16);

        /*  this isnt working right....
        //get current quadrant and save for later use
        float dot = heading.dotProduct(Th);
        if (dot > 0)
            m_stateStamp = abs((uint32)((dot *10)/(m_orbitRadTic *10)) /10);
        else
            m_stateStamp = sEntityList.GetStamp();
        _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - dot:%.3f, stateStamp:%u", dot, m_stateStamp);
*/
        MoveObject();
        return; // this is all we need to do at this point.

        /**  @note these still need to determine ship position to properly set quadrant. (I, II, III, IV)
         *  right now, once ship is within orbit tolerance, we reset radian position to 0
         *  which will reset ship's orbit quadrant to I.
         */
        // these below are within engage distance.
        /** @note these should use a bit of trig to calculate true position, but im lazy, so will hack it for now.  will have to revisit later */
    } else if (far > 1.15) {     //far = current / m_followDistance (calculated)
        // too far outside orbit.  move closer
        _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - too far");
        m_orbiting = 4;
        // simulate orbiting distance here...the distance isnt a straight line, so we need to fudge it as ship will be trying for a larger orbit
        orbitDistNow = current - (m_targetDistance /8);
        // determine distance for next tic based on ship speed and position
        orbitDistNext = orbitDistNow - (m_targetDistance /10);
    } else if (far < 0.8) {
        // too close inside orbit; move away slowly.
        _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - too close");
        m_orbiting = 3;
        orbitDistNow = current + (m_targetDistance /8);
        // determine distance for next tic based on ship speed and position
        orbitDistNext = orbitDistNow + (m_targetDistance /10);
    } else {
        // within orbit distance tolerance
        _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - within tolerance");
        m_orbiting = 1;
        orbitDistNow = orbitDistNext = m_followDistance;  // this is calculated orbit distance for this ship
    }

    // distance was checked and adjusted as needed for this tic.
    // use orbit math to set ship position and MoveObject() to set other attribs.
    // set current position (this is where we are this tic)

    GPoint mPosNext(NULL_ORIGIN);
    // NOTE  for now, an orbit around a stationary object will be horizontal (y=0) and an orbit around a moving object will be vertical (x=0 or z=0)
    /*
    if (Tv) {
        // target is moving.  determine direction vector and set orbit for x or z accordingly
        // NOTE distances are wrong for this....will need adjusting
        bool flatX = false;
        if (abs(m_shipHeading.x) > abs(m_shipHeading.z))
            flatX = true;
        mPos.x = (flatX ? 0 : orbitDistNow * cos(curRad));
        mPos.z = (flatX ? orbitDistNow * cos(curRad) : 0);
        mPos.y = orbitDistNow * sin(curRad);
        // set heading for next tic
        curRad += m_orbitRadTic;
        mPosNext.x = (flatX ? 0 : orbitDistNext * cos(curRad));
        mPosNext.z = (flatX ? orbitDistNext * cos(curRad) : 0);
        mPosNext.y = orbitDistNext * sin(curRad);
    } else { */
        mPos.x = orbitDistNow * cos(curRad);
        mPos.z = orbitDistNow * sin(curRad);
        mPos.y = 0; // flat horizontal orbit
        // set heading for next tic
        /*
        curRad += m_orbitRadTic;
        mPosNext.x = orbitDistNext * cos(curRad);
        mPosNext.z = orbitDistNext * sin(curRad);
        mPosNext.y = 0;
        */
    //}

    // set current heading as vector from current location to calculated location for this tic
    GVector heading(m_position, Tp + mPos);
    heading.normalize();
    m_shipHeading = heading;
    _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - orbiting. curRad:%.5f, timestamp:%u, speed:%.2f, Now:%.2f Next:%.2f, mPos:%.3f,%.3f,%.3f, Head:%.3f,%.3f,%.3f", \
            curRad, timeStamp, curSpeed, orbitDistNow, orbitDistNext, mPos.x, mPos.y, mPos.z, m_shipHeading.x, m_shipHeading.y, m_shipHeading.z);
    m_position = Tp + mPos;
    //m_position.y = Tp.y; // fudge elevation for now...
    m_targetPoint = m_position + (m_shipHeading * 1.0e16);
    MoveObject();

    // set current heading as vector from current location to calculated (supposed) location for next tic
    /*
    GVector heading2(m_position, Tp + mPosNext);
    heading2.normalize();
    m_shipHeading = heading2;    // this sets object velocity in MoveObject() (using speed)
    m_targetPoint = m_position + (m_shipHeading * 1.0e16);
*/
    /* 3d orbits as taught in University of Sydney (usyd.edu) AERO4701 - Space Engineering 3, week 2
     * R = sqrt(x^2 + y^2 + z^2) = m_followDistance
     * Lambda (L) = sin^-1(y/R) (celestial longitude)
     * Phi (p) = tan^-1(z/x) (azimuthal angle - from x axis)
     * x = R * cos(L) * cos(p)
     * z = R * cos(L) * sin(p)
     * y = R * sin(L)
     */

    /*  basic 3d trig ... testing
     * P = myPos - targPos
     * phi = atan2f(Px, Pz)
     * hypPhi = sqrtf(pow(Px, 2) + pow(Pz, 2))
     * lambda = atan2f(Py, hypPhi)
     * radius_2 = sqrt(pow(Px, 2) + pow(Py, 2), pow(Pz, 2))
     * grav_accel = Gc * (TargetMass / pow(radius_2, 2))  not using this.....
     * Px += Px + ((sin(phi) * cos(lambda)))
     * Pz += Pz + ((cos(phi) * cos(lambda)))
     * Py += Py + (sin(lambda))
     *
     *
    GPoint mPos = m_position - Tp;  // get "center" of our orbit
    double phi = atan2(mPos.x, mPos.z);
    double hypPhi = fabs(mPos.z) / sin(phi);
    double lambda = atan2(mPos.y, fabs(hypPhi));
    mPos.x += orbitDistNow * cos(phi) * cos(lambda);
    mPos.z += orbitDistNow * sin(phi) * cos(lambda);
    mPos.y += orbitDistNow * sin(lambda);
    double mainHyp = mPos.y / sin(lambda);  // test calculated radius with actual radius
    _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - orbiting. curRad:%.5f, timestamp:%.3f, speed:%.2f, orbitDistNow:%.2f, mainHyp:%.2f", \
                curRad, timeStamp, curSpeed, orbitDistNow, mainHyp);
    _log(DESTINY__ORBIT_TRACE, "Destiny::_Orbit() - phi:%.2f, lambda:%.2f, hypPhi:%.2f, curRad:%.3f, mPos:%.3f, %.3f, %.3f", \
    phi, lambda, hypPhi, curRad, mPos.x, mPos.y, mPos.z);

    LogMacro(mPos);
    LogMacro(m_position);
    LogMacro(m_shipHeading);
*/
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
            case Destiny::BallMode::DSTBALL_FOLLOW:
            case Destiny::BallMode::DSTBALL_ORBIT: {
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
        // error fix for setting ship movement variables before ship is in bubble (cannot BubbleCast)
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

    // reset turn and movement checks for possible velocity change.
    m_turnTic = 0;
    m_stop = m_accel = m_decel = m_turning = false;

    //reset max accel time in case it was changed previously
    m_shipMaxAccelTime = (-log(0.0001) * m_shipAgility);

    if (!mySE->IsNPCSE() or (mySE->IsNPCSE() and mySE->GetNPCSE()->GetAIMgr()->IsIdle())) {
        m_moveTimer = GetTimeMSeconds();
        m_stateStamp = sEntityList.GetStamp();
    }

    if (m_targetPoint.isNotZero()) {
        GVector targHeading(m_position, m_targetPoint);
        targHeading.normalize();
        m_targetHeading = targHeading;
        if (m_shipHeading.isZero())
            m_shipHeading = targHeading;
    }

    if (m_shipHeading.isZero() and m_targetHeading.isZero()) {
        GVector point(m_position);
        point.normalize();
        m_targetPoint =  (point * 1.0e16);
        GVector targHeading(m_position, m_targetPoint);
        targHeading.normalize();
        m_targetHeading = m_shipHeading = targHeading;
    }

    if (!m_orbiting) {
        // reset target distance just in case it changed.
        GVector shipVector(m_position, m_targetPoint);
        m_targetDistance = shipVector.length();
        m_orbitRadTic = 0.0f;
    }

    if (IsCloaked())
        UnCloak();

    // if ship is not moving, set initial movement variables
    if (!m_userSpeedFraction) {
        SetSpeedFraction(1.0f, true);
        MoveObject();
    } else {
        // reset m_moveTimer for current ship speed vs time to allow correct movement calculations after velocity change
        double newTime = (-log(1 - m_currentSpeedFraction) * m_shipAgility);
        m_moveTimer = (GetTimeMSeconds() - (newTime * 1000));
        SetSpeedFraction(m_userSpeedFraction, true);
        // dont call MoveObject() here, as changes wont take affect till next tic.
    }

    SetPosition(m_position, sConfig.server.PositionHack);
}

void DestinyManager::Follow(SystemEntity* pSE, double distance) {
    //called from client as 'CmdFollowBall'
    //  also used by 'Approach'
    if ((State == Destiny::BallMode::DSTBALL_FOLLOW) and (m_targetEntity.second == pSE) and (m_followDistance == distance) and (m_userSpeedFraction))
        return;
    if (m_orbiting) {
        m_orbiting = 0;
        m_shipHeading = NULL_ORIGIN_V;
    }

    State = Destiny::BallMode::DSTBALL_FOLLOW;
    m_targetPoint = pSE->GetPosition();
    m_targetEntity.first = pSE->GetID();
    m_targetEntity.second = pSE;
    m_followDistance = distance;
    /*  the client doesnt follow this....may fix later.
    if (pSE->IsStationSE()) {
        // set target position to dock of station....NOT in the middle of the fucking thing.
        StationData sData;
        sDataMgr.GetStationInfo(pSE->GetID(), sData);
        m_targetPoint = sData.dockPosition;
    } */
    _BeginMovement();

    DoDestiny_CmdFollowBall du;
        du.entityID = mySE->GetID();
        du.targetID = pSE->GetID();
        du.range = (int32)distance;
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

    sLog.Yellow("GotoDirection", "Heading: %.3f,%.3f,%.3f  Direction: %.3f,%.3f,%.3f",\
                m_shipHeading.x, m_shipHeading.y, m_shipHeading.z, direction.x, direction.y, direction.z);
    State = Destiny::BallMode::DSTBALL_GOTO;
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

    State = Destiny::BallMode::DSTBALL_GOTO;
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
    // >0 means ship cannot warp (warp stabs are neg values, warp scrams are pos values)
    if (mySE->GetSelf()->GetAttribute(AttrWarpScrambleStatus) > 0) {
        if (mySE->HasPilot() and mySE->GetPilot()->CanThrow())
            throw PyException(MakeUserError("WarpScrambled"));
        return;
    }

    m_stopDistance = distance;
    GVector warp_distance(m_position, where);
    m_targetDistance = warp_distance.length();
    m_targetDistance -= m_stopDistance;

    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;

    if (mySE->IsNPCSE()) {
        State = Destiny::BallMode::DSTBALL_WARP;

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

        State = Destiny::BallMode::DSTBALL_STOP;
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

            State = Destiny::BallMode::DSTBALL_STOP;
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

    State = Destiny::BallMode::DSTBALL_WARP;

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

void DestinyManager::Orbit(SystemEntity *pSE, double distance/*0*/) {
    if ((State == Destiny::BallMode::DSTBALL_ORBIT) and (m_targetEntity.second == pSE) and (m_followDistance == distance))
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
    State = Destiny::BallMode::DSTBALL_ORBIT;
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
        du.distance = (int32)m_followDistance;
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);    //consumed
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
    m_activeSpeedFraction = 1.0f;
    m_currentSpeedFraction = 1.0f;
    m_maxSpeed = m_maxShipSpeed;
    m_velocity = m_shipHeading * m_maxSpeed;

    if (!mySE->IsMissileSE()) {
        State = Destiny::BallMode::DSTBALL_GOTO;
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
    m_shipMaxAccelTime = (-log(0.0001) * m_shipAgility);
    m_degPerTic = (65.0f - m_shipAgility) /10;

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
        m_maxSpeed = m_maxShipSpeed * m_userSpeedFraction;      // reset ship max speed using updated m_maxShipSpeed
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
            // - this hits when prop mod activated while ship is decel from previous prop mod deactivation
            m_maxSpeed = m_maxShipSpeed * m_prevSpeedFraction;      // reset ship max speed using updated m_maxShipSpeed
            m_currentSpeedFraction = m_activeSpeedFraction = m_prevSpeed / m_maxSpeed;          //get updated asf
            _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(asf>usf>=0) - decelerating. - sec: %.2f, csf: %.3f. asf: %.3f, curSpeed: %.3f, accelTime: %.3f", \
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
        sbms.speedValue = m_maxShipSpeed;
    updates.push_back(sbms.Encode());
    SendDestinyUpdate(updates);
}

void DestinyManager::WebbedMe(InventoryItemRef modRef, bool apply/*false*/)
{
    if (apply)
        m_maxShipSpeed *= (1 + (modRef->GetAttribute(AttrSpeedFactor).get_float() / 100));
    else
        m_maxShipSpeed /= (1 + (modRef->GetAttribute(AttrSpeedFactor).get_float() / 100));
    m_activeSpeedFraction = m_activeSpeedFraction * 0.999;
    SetSpeedFraction(m_userSpeedFraction, true);
}

//  called from Client::CreateShipSE(), Client::ResetAfterPodded(), NPC::NPC(), Concord::Concord(), Drone::Drone(), DestinyManager::UpdateNewShip()
void DestinyManager::SetShipCapabilities(InventoryItemRef ship, bool undock)
{
    /*
Frigates (incl. CovOps, Inty, AF) have an agility of 3.1
Destroyers 3.5
Industrials 1.0
Cruisers 0.55 (Elite/Faction 0.65)
Battlecruisers 1.1
Battleships 0.155
*/
    /* this sets variables needed for correct movement math.
     *  these attribs are set from ship item when shipSE created.  DO NOT modify anything here
     */
    m_mass = ship->GetAttribute(AttrMass).get_float();
    m_radius = ship->GetAttribute(AttrRadius).get_float();
    m_massMKg = m_mass / 1000000; //changes mass from Kg to MillionKg (10^-6)

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
    if ((m_speedToLeaveWarp < 100) and (m_maxShipSpeed > 135))      // 75% of 135 is 101.25
        m_speedToLeaveWarp = 100;

    /* The product of Mass and the Inertia Modifier gives the ship's agility
     * Agility = Mass x Inertia Modifier
     *  agility is an internal-use variable.
     */
    m_shipAgility = m_massMKg * m_shipInertia;
    m_degPerTic = (65.0f - m_shipAgility) /10;
    // set a maximum acceleration time (based on ship variables)
    //   this is no longer correct.  Vmax/T is the correct formula
    m_shipMaxAccelTime = (-log(0.0001) * m_shipAgility);

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
    State = Destiny::BallMode::DSTBALL_MISSILE;
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
    State = Destiny::BallMode::DSTBALL_FOLLOW;

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
    //TODO need to figure out what this is....
	std::vector<int32, std::allocator<int32> > area;
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
    //TODO need to figure out what this is....
    std::vector<int32, std::allocator<int32> > area;
    DoDestiny_OnSpecialFX13 effect;
        effect.entityID = entityID;
        effect.moduleID = moduleID;
        effect.moduleTypeID = moduleTypeID;
        effect.targetID = targetID;
        effect.chargeTypeID = chargeTypeID;
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
