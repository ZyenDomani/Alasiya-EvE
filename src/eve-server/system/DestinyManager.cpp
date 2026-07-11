/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    Copyright 2016 - 2026 Alasiya-EvE by Allan
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
#include "EntityMgr.h"
#include "PyServiceMgr.h"
#include "StaticDataMgr.h"
#include "map/MapData.h"
#include "math/Trig.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "packets/Missile.h"
#include "ship/Missile.h"
#include "ship/Ship.h"
#include "ship/modules/ModuleItem.h"
#include "station/Station.h"
#include "station/StationDataMgr.h"
#include "system/BubbleManager.h"
#include "system/Container.h"
#include "system/DestinyManager.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"

// fixed variable init order  -allan 7jul26
DestinyManager::DestinyManager(SystemEntity *self)
: mySE(self), m_targBubble(nullptr), m_ballMode(Destiny::Ball::Mode::STOP), m_hasSentShipUpdates(false),
m_warpCapacitorNeed(0.000000138), m_alignTime(5.0f), m_shipWarpSpeed(1.0f), m_speedToLeaveWarp(100.0f),
m_maxSpeed(1.0f), m_stop(true), m_cloaked(false), m_tractored(false), m_tractorPause(false), m_paused(false),
m_stateStamp(0), m_activeSpeedFraction(0.0f), m_userSpeedFraction(0.0f), m_maxOrbitSpeedFraction(1.0f),
m_targetDistance(0), m_followDistance(0), m_position(self->GetPosition()), m_targetPoint(NULL_ORIGIN),
m_warpHeading(GPOINT_IDENTITY), m_targetHeading(NULL_ORIGIN_V), m_moveDelay(false), m_skipTic(true),
m_autoPilot(false), m_alignTo(false), m_agility(0.0f), m_bump(false), m_posHack(sConfig.debug.PositionHack),
m_accelTime(0.0f), m_accelDistance(0), m_decelTime(0.0f), m_warpState(nullptr), m_shipVelocity(NULL_ORIGIN_V),
m_orbitNormal(NULL_ORIGIN_V), m_orbitBasisZ(NULL_ORIGIN_V), m_orbitBasisX(NULL_ORIGIN_V), m_expTerm(0.0f),
m_timeStamp(0), m_targetVelocity(NULL_ORIGIN_V), m_posScale(0.0)
{
    //sLog.Magenta("Destiny", "created 0X%x for %s", this, self->GetName());
    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;
}

DestinyManager::~DestinyManager() {
    SafeDelete(m_warpState);
}

    /* variables used (for/by/in) Destiny/Movement
     * *Fraction variables use fuzzy logic
     *  -complete rewrite and update jun2026
     *
     *  spaceDrag = 0.30000001192092896
     *
     *  m_position current ship/item position
     *   -> set in MoveObject() and Warp()
     *  m_userSpeedFraction  current sf set by player/client
     *  m_activeSpeedFraction  keeping for logs
     *  m_maxSpeed is maximum speed based on user input and current configuration.
     *   -> measured in m/s
     *   -> initially set by UpdateShipVariables() based on ship configuration and pilot skills
     *   -> reset by SpeedBoost() for prop mods
     *   -> sent to client as ship max speed
     *  m_stateStamp is server tic when move event began
     *   -> measured in seconds
     *   -> data type is int32
     *   -> set in BeginMovement() for all velocity changes
     *  m_timeStamp  is timestamp of when current mode began
     *   -> measured in fileTime (100 ns resolution in filetime format)
     *   -> data type is int64
     *   -> set in BeginMovement() for all velocity changes
     *  m_followDistance requested distance between self and target
     *   -> set in OrbitBall(), Follow(),
     *  m_warpHeading holds warp direction
     *   -> used to keep track of ship's heading for vector changes
     *   -> set in WarpTo().  also used by missile and tractor
     *  m_shipVelocity is object's current derived vector.
     *   -> set in MoveObject(), Warp(), Undock() and tractor
     *   -> m_shipVelocity = m_targetHeading * speed * m_userSpeedFraction * spaceDrag
     *   -> this is the variable used for position tracking.
     *  m_targetPoint holds current target coords.
     *   -> initially set by goto, warp, align, follow, orbit
     *   -> reset in follow and orbit for each tic to correctly align with moving ship/target
     *  m_targetDistance as stated
     *   -> initially set in all Move calls
     *   -> recalculated each tic using m_targetPoint and m_position
     *  m_targetVelocity is desired vector velocity
     *   -> used to blend with ship's velocity for vector changes
     *   -> initially set in SetSpeedFraction() (called by BeginMovement())
     *   -> updated in Follow, Orbit, SpeedBoost() and MoveObject()
     *  m_targetHeading holds desired direction
     *   -> used to blend with ship's heading for vector changes
     */

// this is called once per tic by SystemEntity::Process() via SystemManager::Process()
void DestinyManager::Process() {
    if (m_skipTic)
        return;

    double profileStartTime = GetTimeUSeconds();
    using namespace Destiny::Ball::Mode;
    switch(m_ballMode) {
        case RIGID: {
            // item that doesnt move - mobile objects will be <rigid> after full stop
            // NOTE:  this should not hit.
            _log(DESTINY__ERROR, "Destiny::Process() Error!  %s(%u) is proc with state=RIGID.",  \
                    mySE->GetName(), mySE->GetID());
            return;
        } break;
        case MISSILE: {
            // no real reason to use proc tics here...<missile> is just pixels on player's screen...
            return;
        } break;
        case STOP: {
            // this will be removed from system tics after momentum is depleted
            // nothing special to do here.
        } break;
        case GOTO: {
            // there's not necessarily a target here...just a direction or point in space.
            if (m_alignTo) {
                if (m_targetDistance < m_followDistance) {
                    if (is_log_enabled(DESTINY__MOVE_TRACE))
                        sLog.Warning("DestinyManager::Process(Goto)", "%s is aligned and within follow distance.", \
                                mySE->GetName());
                    Stop();
                    break;
                }
            }
        } break;
        case FOLLOW: {
            if (!ValidTarget()) {
                if (is_log_enabled(DESTINY__MOVE_TRACE))
                    sLog.Warning("DestinyManager::Process(Follow)", "%s's target is invalid", mySE->GetName());
                Stop();
                break;
            }
            const GPoint& target_point = m_targetEntity.second->GetPosition();
            GVector heading = m_targetPoint - m_position;
            // get target's cur pos and update our targ point
            m_targetDistance = (heading.normalize() - mySE->GetRadius() - m_targetEntity.second->GetRadius());
            m_targetPoint = target_point;
            m_targetHeading = heading;

            if (m_targetDistance < m_followDistance) {
                float speed = m_targetEntity.second->DestinyMgr()->GetSpeed();
                // slow down to match target
                float targetFraction = speed / m_maxSpeed;
                if (targetFraction > 1.0f)
                    targetFraction = 1.0f;
                SetSpeedFraction(targetFraction);
            } else {
                SetSpeedFraction(1.0f);
            }
        } break;
        case FORMATION: {
            // fleet and npc movement - only for tracking collisions
            ////  IDEA:  rat groups should warp/enter in formation (grouped by class so the 'warp lag' from larger ships remains)
            //           then (based on 'rat skill') disperse once fighting begins.
/*  this will need other systems more complete first...
            // 1. CALCULATE THE LEADER'S ROTATED MATRIX SLOT COORDINATE
            // Fetch leader entity references and look angles
            DestinyManager* leader = m_fleetLeaderDestinyInstance;
            Vector3 leaderPos = leader->GetPosition();
            Vector3 leaderVel = leader->GetVelocity();

            // Fetch our relative slot coordinates from your GetFormations layout data
            // Slot 1 = Vector3{150.0, 0.0, 0.0}, etc.
            Vector3 localSlotOffset = GetFormationOffsetVector(m_formationID, m_formationSlotID);

            // --- THE VISUAL DIRECTION ALIGNMENT FIX ---
            // If the leader is moving, rotate the formation offset vector so the shape
            // faces the direction the leader is traveling!
            if (leaderVel.length() > 0.01) {
                Vector3 forward = leaderVel.normalized();
                Vector3 right = forward.cross(Vector3{0.0, 1.0, 0.0}).normalized();

                // Translate relative coordinates to match leader's world space tracking orientation
                localSlotOffset = (right * localSlotOffset.x) + (forward * localSlotOffset.z);
            } else {
                // Safeguard: If objects are perfectly stacked, target zero to prevent infinite NaN vectors
                targetVelocity = Vector3{0.0, 0.0, 0.0};
            }

            // 2. DEFINE THE ABSOLUTE VIRTUAL SLOT TARGET POINT IN SPACE
            Vector3 virtualTargetPoint = leaderPos + localSlotOffset;

            // 3. MEASURE THE DISTANCE TRAJECTORY GAP
            Vector3 delta_pos = virtualTargetPoint - m_position;
            double distanceToSlot = delta_pos.length();

            // 4. CHOOSE STEERING IMPULSE SPEED CATEGORY
            if (distanceToSlot > 1.0) { // If out of position by more than 1 meter
                Vector3 steering_direction = delta_pos.normalized();

                // If far behind, catch up at max speed. If close, match leader's pace!
                double targetSpeed = (distanceToSlot > 50.0) ? m_maxSpeed : leaderVel.length();

                targetVelocity = steering_direction * targetSpeed;
            } else {
                // Symmetrically match leader vector components exactly to float perfectly in place
                targetVelocity = leaderVel;
            }
            */
        } break;
        case ORBIT: {
            if (!ValidTarget()) {
                Stop();
                return;
            }

            GVector targetPos = m_targetEntity.second->GetPosition();
            GVector distVec = m_position - targetPos;
            float currentDistance = distVec.length();

            // FIX 1: Enforce intended target throttle speed to eliminate the rubberband trap!
            float targetCruiseSpeed = m_maxSpeed * m_userSpeedFraction;
            if (targetCruiseSpeed < 0.01f) {
                targetCruiseSpeed = 10.0f; // Minimal fallback to ensure the blender doesn't stall out
            }

            GVector radialDir = distVec;
            radialDir.normalize();

            // =========================================================================
            // REFIXED PHASE A: FIXED RADIUS LONG-RANGE SHOULDER INTERCEPT
            // =========================================================================
            // If the ship is far out, m_targetPoint is pinned safely to the orbit ring's shoulder.
            if (currentDistance > (m_targetDistance + 50.0f)) {
                // Target point sits strictly at the requested radius edge on the V axis
                m_targetPoint = targetPos + (m_orbitBasisX * m_targetDistance);

                // Direct approach calculation (To - From)
                GVector approachDir = m_targetPoint - m_position;
                approachDir.normalize();

                // Drive the blender using the constant target cruise speed, matching the target's frame
                m_targetVelocity = (approachDir * targetCruiseSpeed) + m_targetEntity.second->DestinyMgr()->GetSpeed();
            }
            // =========================================================================
            // PHASE B: CLIMB-OUT SEPARATION (Ship inside the model ball)
            // =========================================================================
            else if (currentDistance < (m_targetDistance - 20.0f)) {
                m_targetPoint = targetPos + (m_orbitBasisZ * m_targetDistance);
                m_targetVelocity = (m_orbitBasisZ * targetCruiseSpeed) + m_targetEntity.second->DestinyMgr()->GetSpeed();
            }
            // =========================================================================
            // PHASE C: ACTIVE VECTOR CIRCULAR CRUISE
            // =========================================================================
            else {
                float componentU = distVec.dotProduct(m_orbitBasisZ);
                float componentV = distVec.dotProduct(m_orbitBasisX);

                GVector forwardTangent = (m_orbitBasisX * componentU) - (m_orbitBasisZ * componentV);
                forwardTangent.normalize();

                GVector baseOrbitVelocity = forwardTangent * targetCruiseSpeed;

                float distanceError = m_targetDistance - currentDistance;
                GVector correctionVelocity = radialDir * (distanceError * 0.5f);

                m_targetPoint = m_position + m_targetVelocity;
                m_targetVelocity = baseOrbitVelocity + correctionVelocity + m_targetEntity.second->DestinyMgr()->GetSpeed();
            }

            m_targetHeading = distVec;

            if (is_log_enabled(DESTINY__ORBIT_TRACE))
                _log(DESTINY__ORBIT_TRACE, "Destiny::Process() - %s(%u): Orbit heading is %.2f, %.2f, %.2f @ %0.5f", \
                        mySE->GetName(), mySE->GetID(), m_targetHeading.x, m_targetHeading.y, m_targetHeading.z, \
                        currentDistance);
        } break;
        case WARP: {
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
                uint16 sec_into_warp = (sEntityMgr.GetStamp() - m_stateStamp);
                //  speed and distance formulas based on current warp distance
                if (m_warpState->accel) {
                    WarpAccel(sec_into_warp);
                } else if (m_warpState->cruise) {
                    WarpCruise(sec_into_warp);
                } else if (m_warpState->decel) {
                    WarpDecel(sec_into_warp);
                } else {// uh, Houston...we have a problem...
                    if (mySE->HasPilot()) {
                        _log(DESTINY__ERROR, "Destiny::Process() Error!  Ship %s(%u) for Player %s(%u) Has WarpState but checks are false.",  \
                                    mySE->GetName(), mySE->GetID(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());
                        mySE->GetPilot()->SendErrorMsg("Internal Server Error.<br> Please Dock, Jump or Relog to reset your ship.");
                    } else {
                        _log(DESTINY__ERROR, "Destiny::Process() Error!  NPC %s(%u) Has WarpState but checks are false.",  \
                                    mySE->GetName(), mySE->GetID());
                    }
                }
                if (sConfig.debug.UseProfiling)
                    sProfiler.AddTime(Profile::destiny, GetTimeUSeconds() - profileStartTime);
                return;
            }

            // check speed...if all good, call InitWarp()
            if (IsAligned(m_targetHeading) and (m_activeSpeedFraction > 0.749999f)) {
                InitWarp();
                if (sConfig.debug.UseProfiling)
                    sProfiler.AddTime(Profile::destiny, GetTimeUSeconds() - profileStartTime);
                return;
            }
        } break;

        case TROLL: {
            // will need more thought
            // movable object that will become rigid after a set time
            // jetcans/wrecks created from moving ship will take that velocity and continue movement for a time, then become rigid.
            //update.  ball is set <state_stop> and <mode_troll> with timestamp.
            //  check elapsed time here while object is moving, then switch to <mode_rigid> once stopped.
            // this elapsed check can/may change based on load in current bubble
            // 3. client uses 30-second window (Matches Ballpark Offset 0x7c)  may reduce this to 15-20s
            /*
            if (m_trollElapsedTicks >= 30 or m_shipVelocity.length() < 0.05) {
                // zero out vars
                Halt();
                // Mutate state locally to a static rigid item
                m_ballMode = Destiny::Ball::Mode::RIGID;

                // Trigger your existing network broadcast loop to inform nearby players
                NotifyBubbleOfStateChange();
                return;
            } */
        } break;
        case BOID: { // this will turn RIGID after a set time
            /* The Physics:
             * If multiple dynamic items or entities drop simultaneously, they execute a brief, basic separation
             *  step so they do not overlap identically.
             */
        } break;
        default: {
            _log(DESTINY__ERROR, "Destiny::Process() - %s(%u): hit default with state: %s",
                    mySE->GetName(), mySE->GetID(), GetModeNameString().c_str());
        } break;
    }

    MoveObject();

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::destiny, GetTimeUSeconds() - profileStartTime);
}

//Velocity setting method
void DestinyManager::SetSpeedFraction(float fraction/*1.0f*/) {
    // update velocity vector for changes
    m_targetVelocity = m_targetHeading * (m_maxSpeed * fraction);

    if (m_shipVelocity == m_targetVelocity)
        return;

    if (is_log_enabled(DESTINY__MOVE_TRACE)) {
        GVector heading = m_shipVelocity;
        heading.normalize();
        _log(DESTINY__MOVE_TRACE, "Destiny::SetSpeedFraction() - %s(%u): from %.2f to %.2f.  targVel: %.2f, %.2f, %.2f", \
                mySE->GetName(), mySE->GetID(), m_userSpeedFraction, fraction, \
                m_targetVelocity.x, m_targetVelocity.y, m_targetVelocity.z);
        _log(DESTINY__MOVE_TRACE, "Destiny::SetSpeedFraction() - head: %.2f, %.2f, %.2f.  targHead: %.2f, %.2f, %.2f", \
                heading.x, heading.y, heading.z, m_targetHeading.x, m_targetHeading.y, m_targetHeading.z);
    }

    m_userSpeedFraction = fraction;

    if (fraction) {
        std::vector<PyTuple*> updates;
        CmdSetSpeedFraction du;
            du.entityID = mySE->GetID();
            du.fraction = fraction;
        updates.push_back(du.Encode());

        if (mySE->IsNPCSE() /*and !m_hasSentShipUpdates)*/
        or mySE->IsMissileSE() or mySE->IsContainerSE() or mySE->IsWreckSE()) {
            SetBallSpeed ms;
                ms.entityID = mySE->GetID();
                ms.speed = m_maxSpeed;
            updates.push_back(ms.Encode());
            m_hasSentShipUpdates = true;
        }

        if (!updates.empty())
            SendDestinyUpdates(updates); //consumed
    } else {
        m_targetHeading.y = 0.0;
    }
}

//Global Actions:
// main movement method
void DestinyManager::MoveObject() {
    double speed = m_shipVelocity.length();
    if (m_stop) {
        // update heading to level out when stopped
        m_shipVelocity.y += ((0.0f - m_shipVelocity.y) * SPACE_DRAG);
        double speedSq = speed * speed;
        // 0.05 m/s is the standard practical "visual stop" threshold where the client view locks.
        if ((speedSq < VISUAL_STOP_THRESHOLD_SQ) or (speedSq < HARD_STOP_THRESHOLD_SQ)) {
            // destiny considers a ship completely stopped when its velocity vector length drops below 0.001m/s.
            //Halt();
            m_skipTic = true;
            return;
        }
    }

    //position update
    m_position += m_targetVelocity + ((m_shipVelocity - m_targetVelocity) * m_posScale);
    // glide velocity from current to desired.
    m_shipVelocity = m_targetVelocity + ((m_shipVelocity - m_targetVelocity) * m_expTerm);

    // update position
    mySE->SetPosition(m_position);

    speed = m_shipVelocity.length();
    if (m_maxSpeed > 0.05f) {
        m_activeSpeedFraction = speed / m_maxSpeed;
    } else {
        _log(DESTINY__WARNING, "Destiny::MoveObject() - %s(%u): m_maxSpeed = 0",
             mySE->GetName(), mySE->GetID());
        Stop();
    }

    if (is_log_enabled(DESTINY__MOVE_TRACE))
        _log(DESTINY__MOVE_TRACE, "Destiny::MoveObject() - %s(%u): m_shipVelocity is %.2f, %.2f, %.2f @ %0.3f", \
                mySE->GetName(), mySE->GetID(), m_shipVelocity.x, m_shipVelocity.y, m_shipVelocity.z, \
                m_activeSpeedFraction);

    if (sEntityMgr.GetTracking()) {
        // only create can when ship is moving significant amount
        if (m_activeSpeedFraction > sConfig.debug.ShipTrackingTime) {
            // create jetcan to visualize movement
            std::string str = mySE->GetName();
            str += " " + GetModeNameString() + " ";
            str += itoa(sEntityMgr.GetStamp() - m_stateStamp);
            MarkPoint(m_position, str, str);
        }
    }

    if (sConfig.cosmic.BumpEnabled
        and mySE->HasPilot()
        and mySE->SysBubble()->HasPlayers())
        CheckBump();
}

void DestinyManager::Stop() {
// is this right??
    if (mySE->HasPilot() and mySE->GetSelf()->GetShipItem()->IsUndocking())
        return;

    if (is_log_enabled(DESTINY__MOVE_TRACE))
        sLog.Warning("DestinyManager", "%s calling stop", mySE->GetName());

    // set marker for calc'd stop distance (testing)
    if (is_log_enabled(DESTINY__WARP_DEBUG)) {
        uint16 dist = m_maxSpeed * m_activeSpeedFraction * m_agility;
        GVector offset = (m_position * dist);
        GPoint marker = (m_position + offset);
        std::string str = "Stop Point - ";
        str += mySE->GetName();
        MarkPoint(marker, str, str);
    }

    m_stop = true;
    m_alignTo = false;
    m_posHack = false;
    m_autoPilot = false;

    m_orbitNormal = NULL_ORIGIN_V;
    m_orbitBasisZ = NULL_ORIGIN_V;
    m_orbitBasisX = NULL_ORIGIN_V;

    m_targBubble = nullptr;

    m_stateStamp = sEntityMgr.GetStamp();

    SetSpeedFraction(0.0f);

    if (m_ballMode == Destiny::Ball::Mode::WARP) {
        /*  ballmode is set in Destiny::WarpTo();  warpstate is created in Destiny::InitWarp()
         * ship may not actually be in warp yet if align/speed isnt right
         * in this case, player wants to cancel warp before it begins
         * for this, we need to send a stop packet after stop vars are set.
         *  however,   sending stop packet while in warp does funny things...dont send it.
         */
        if (m_warpState != nullptr) {
            // ship is in warp: could be decel.  dont send packet here
            SafeDelete(m_warpState);
            m_ballMode = Destiny::Ball::Mode::STOP;
            return;
        }

        // if warp not started, reset warp vars and fall thru to allow stop
        if (mySE->GetSelf()->HasAttribute(AttrWarpCapacitorNeed)) {
            m_warpCapacitorNeed = mySE->GetSelf()->GetAttribute(AttrWarpCapacitorNeed).get_double();
        } else {
            m_warpCapacitorNeed = 0.000000138;   // lowest value in db
        }
        // warp gfx cleared by stop packet
    }

    if (m_ballMode != Destiny::Ball::Mode::STOP) {
        m_ballMode = Destiny::Ball::Mode::STOP;
        CmdStop du;
            du.entityID = mySE->GetID();
        PyTuple *up = du.Encode();
        SendSingleDestinyUpdate(&up);
        PyDecRef(up);
    }
}

// calling this will set object to a COMPLETE and IMMEDIATE stop, update STATE and remove from system ticlist
void DestinyManager::Halt(bool commanded/*false*/) {
    SafeDelete(m_warpState);

    //  reset ALL movement variables and states.
    m_ballMode = Destiny::Ball::Mode::STOP;
    m_stop = true;
    m_alignTo = false;
    m_autoPilot = false;
    m_stateStamp = 0;
    m_targetDistance = 0;
    m_userSpeedFraction = 0.0f;

    m_targetPoint = NULL_ORIGIN;
    m_shipVelocity = NULL_ORIGIN;
    m_orbitNormal = NULL_ORIGIN_V;
    m_orbitBasisZ = NULL_ORIGIN_V;
    m_orbitBasisX = NULL_ORIGIN_V;
    m_warpHeading = m_targetHeading;
    m_targetHeading = NULL_ORIGIN_V;
    m_targetVelocity = NULL_ORIGIN_V;

    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;

    m_skipTic = true;

    if (is_log_enabled(DESTINY__MOVE_TRACE))
        _log(DESTINY__MOVE_TRACE, "Destiny::Halt(%s) - %s(%u) Halted", \
                (commanded ? "true" : "false"), mySE->GetName(), mySE->GetID());

    if (commanded) {
        // immediate halt via command.  send packet to stop ship.
        CmdStop du;
            du.entityID = mySE->GetID();
        PyTuple *up = du.Encode();
        SendSingleDestinyUpdate(&up);
    }
}

void DestinyManager::Eject()
{
    // basic updates for ejecting from ship...does <Eject> stop ship?
	// MAKE SURE...pod is set to troll with ship's current velocity.
    UpdateOldShip(mySE->GetShipSE());
    SendJettisonPacket();
    Stop();
}


void DestinyManager::InitWarp() {
    // these are the only vars that need to be set here right now
    m_posHack = false;

    // warp time and distance math
    //   allan 1Nov14 - 14Nov14
    //  rewrite 3jan15  to use distance instead of time for warping.  more accurate now, and covers ALL distances.
    //  calculation and implementation update   9Jan15      accuracy is within 1000m
    //  major destiny movement update/rewrite - allan Feb23  (wip)
    //  warp update complete.  MOE ~1-2m, scaling with ship size  -allan 7Mar23   **UPDATE**  this wasnt right.
    //  warp rewrite.  finally corrected deviations.  all ships within 12m   -allan 21Jan25

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
     * k = 3 for accel, -1 for decel
     *
     * this gives distances as functions of time.
     */

    /* this is my version of how warp should be timed and followed by the server.
     * checks here for distance < warp speed and adjusts accel/decel times accordingly
     *
     *   accel/decel are logarithmic per ccp (see above).
     */

    //  150km - 15s, 1mkm - 23s, 1au - 29s base + ship's wsm
    float decelTime(0.0f), cruiseTime(0.0f);
    int64 accelDistance(0), decelDistance(0), cruiseDistance(0);
    int64 warpSpeedInMeters(m_shipWarpSpeed * ONE_AU_IN_METERS);
    // set times and distances based on target distance
    if (m_targetDistance < (warpSpeedInMeters * 3)) {
        //  short warp....no cruise
        // accel = 1/3 decel
        accelDistance = (m_targetDistance / 3);
        //decelTime += m_accelTime * 2;
        warpSpeedInMeters = accelDistance;
    } else {
        accelDistance = warpSpeedInMeters;       // ship warp speed in meters
        //m_accelTime = (27 + (m_shipWarpSpeed / 3)) / 3;
        //m_accelTime = log(accelDistance / 3) / 3;
        //decelTime += (m_accelTime * 2);
        cruiseDistance = ((double)m_targetDistance - (accelDistance * 3));
        cruiseTime = EvE::max(cruiseDistance / warpSpeedInMeters, 1);
    }

    decelDistance = accelDistance * 2;

    m_accelTime = std::log(accelDistance / 3) / 3;
    m_decelTime = std::log(decelDistance);

    //  set total warp time based on above math.
    float warpTime = m_accelTime + m_decelTime + cruiseTime;

    // check decel ship speed to drop out of warp
    decelTime = m_decelTime;
    float speed(0.0f);
    bool run(true);
    uint8 step(0);
    while (run) {
        speed = std::exp(--decelTime);
        ++step;
        if (speed < m_speedToLeaveWarp) {
            run = false;
        }
    }

    //sLog.Warning("warptest 1", "step: %u, time: %0.1f, speed: %0.1f", step, decelTime, speed);
    //decelTime = 0.0f;
    double distance(0.0);
    while (step > 0) {
        distance += std::exp(decelTime++);
        --step;
    }

    //sLog.Warning("warptest 2", "step: %u, time: %0.1f, distance: %0.1f", step, decelTime, distance);

    m_targetDistance -= (decelDistance - distance);
    //m_decelTime = decelTime;
    decelDistance = distance;

    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        _log(DESTINY__WARP_TRACE, "Destiny::InitWarp():Calculate - %s(%u): Warp will accelerate for %0.1fs, cruise for %0.1fs, then decelerate for %0.1fs, with total time of %0.1fs and warp speed of %lli m/s.", \
                mySE->GetName(), mySE->GetID(), m_accelTime, cruiseTime, decelTime, warpTime, warpSpeedInMeters);
        _log(DESTINY__WARP_TRACE, "Destiny::InitWarp():Calculate - Accel distance is %lli  Cruise distance is %lli   Decel distance is %lli   Heading is %0.4f,%0.4f,%0.4f.", \
                accelDistance, cruiseDistance, decelDistance, m_warpHeading.x, m_warpHeading.y, m_warpHeading.z);
        _log(DESTINY__WARP_TRACE, "Destiny::InitWarp():Calculate - We will exit warp at %0.1f,%0.1f,%0.1f at a distance of %lli AU (%lli m).", \
                m_targetPoint.x, m_targetPoint.y, m_targetPoint.z, m_targetDistance / ONE_AU_IN_METERS, m_targetDistance);
    }

    uint16 intAccel(m_accelTime);
    float accelFraction(m_accelTime - intAccel);

    m_warpState = new WarpState(m_stateStamp, m_targetDistance, warpSpeedInMeters, accelDistance, cruiseDistance,
                                decelDistance, warpTime, accelFraction, true, false, false);

    // check for player warp
    if (mySE->HasPilot()) {
        //turn off non warp-safe modules
        mySE->GetShipSE()->Warp();
        //drain cap
        mySE->GetSelf()->SetAttribute(AttrCapacitorCharge, m_warpCapacitorNeed);
        // reset warp cap need
        if (mySE->GetSelf()->HasAttribute(AttrWarpCapacitorNeed)) {
            m_warpCapacitorNeed = mySE->GetSelf()->GetAttribute(AttrWarpCapacitorNeed).get_double();
        } else {
            m_warpCapacitorNeed = 0.000000138;   // lowest value in db
        }
    }
    // do npcs need to notify ai of warping?

    //clear targets
    mySE->TargetMgr()->ClearAllTargets();
    //mySE->TargetMgr()->OnTarget(nullptr, TargMgr::Mode::Clear, TargMgr::Msg::WarpingOut);

    // send warp gfx
    SendGFX10(mySE->GetID(),"effects.Warping" );

    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;

    //TODO:  determine if this ship has assigned drones in space and call drone:shipwarping

    // reset move times
    m_accelDistance = 0;
    m_stateStamp = sEntityMgr.GetStamp();
    m_activeSpeedFraction = 1.0f;

    WarpAccel(0);
}

void DestinyManager::WarpAccel(uint16 sec_into_warp) {
    /* For acceleration, k = 3.
     * distance = e^(k*s)
     * speed = k*e^(k*s)
     */
    float accelTime(sec_into_warp + m_warpState->accelFraction);
    int64 currentDistance = std::exp(3 * accelTime);

    if (accelTime >= m_accelTime) {
        m_warpState->accel = false;
        currentDistance = m_warpState->accelDist - m_accelDistance;

        if (m_warpState->cruiseDist > 0.0) {
            m_warpState->cruise = true;
        } else {
            m_warpState->decel = true;
            m_targetDistance = m_warpState->decelDist + currentDistance;
            // update ship position to match decel target
            GVector update(m_warpHeading * m_targetDistance);
            m_position = m_targetPoint - update;
            mySE->SetPosition(m_position);
        }
    }

    m_accelDistance += currentDistance;
    WarpUpdate(currentDistance, sec_into_warp, 1);

    if (mySE->SysBubble() != nullptr) {
        //if (currentDistance > BUBBLE_RADIUS_METERS) { // this will not account for warping from one side of bubble to other
        if (!mySE->SysBubble()->InBubble(m_position)) {  // check actual bubble center here
            if (is_log_enabled(DESTINY__WARP_TRACE))
                _log(DESTINY__WARP_TRACE, "Destiny::WarpAccel(): %s(%u) is being removed from bubble %u.",\
                        mySE->GetName(), mySE->GetID(), mySE->SysBubble()->GetID());
            mySE->SysBubble()->Remove(mySE);
        }
    }
}

void DestinyManager::WarpCruise(uint16 sec_into_warp) {
    // in cruise...update position data and check for decel
    WarpUpdate(m_warpState->warpSpeed, sec_into_warp, 2);

    if ((m_targetDistance - m_warpState->warpSpeed) < m_warpState->decelDist) {
        m_targetDistance = m_warpState->decelDist;
        // update ship position to match decel target
        GVector update(m_warpHeading * m_targetDistance);
        m_position = m_targetPoint - update;
        mySE->SetPosition(m_position);

        m_warpState->cruise = false;
        m_warpState->decel = true;
    }
}

void DestinyManager::WarpDecel(uint16 sec_into_warp) {
    /* For deceleration, k = -1
     * distance = e^(k*s)
     * speed = k*e^(k*s)
     */
    double decelTime = --m_decelTime;
    int64 currentShipSpeed = std::exp(decelTime);

    WarpUpdate(currentShipSpeed, sec_into_warp, 3);

// how will this work for edge-case where dest is 5k inside bubble?
    if (mySE->SysBubble() == nullptr) {
        if (m_targBubble->InBubble(m_position)) {   // check actual bubble center here
            if (is_log_enabled(DESTINY__WARP_TRACE))
                _log(DESTINY__WARP_TRACE, "Destiny::WarpDecel(): %s(%u) is being added to bubble %u.",\
                        mySE->GetName(), mySE->GetID(), m_targBubble->GetID());
			// testing hack to force client to draw new ship with current speed
			m_ballMode = Destiny::Ball::Mode::GOTO;
            m_targBubble->Add(mySE);
			m_ballMode = Destiny::Ball::Mode::WARP;
        }
    }

// change this to use distance?  will have to test with new move code
    if (currentShipSpeed <= m_speedToLeaveWarp)
        WarpStop(currentShipSpeed);
}
/*  warp decel update.  check before implementing

******************** warp exit update ? ***************************
void DestinyManager::WarpDecel(uint16 sec_into_warp) {
    double decelTime = --m_decelTime;
    int64 currentShipSpeed = exp(decelTime);

    WarpUpdate(currentShipSpeed, sec_into_warp, 3);

    if (mySE->SysBubble() == nullptr) {
        if (m_targBubble->InBubble(m_position)) {
            m_targBubble->Add(mySE);
        }
    }

    // --- DYNAMIC DISTANCE ENVELOPE CONFIGURATION ---
    double distanceToTargetCenter = m_targBubble->GetCenter().distance(m_position);

    // Fetch the spatial proximity padding expansion envelope from offset 0xa6
    // Frigates = 2500.0m, Battleships = 7500.0m, Capital Ships = 15000.0m
    double proximityPadding = mySE->GetSpatialPaddingEnvelope();

    double dropThreshold = mySE->GetRadius() + m_targBubble->GetRadius() + proximityPadding;

    // Master Gate Execution
    if (distanceToTargetCenter <= dropThreshold || currentShipSpeed <= m_speedToLeaveWarp) {

        // Snap position to clear 1Hz time truncation gaps
        m_position = m_targBubble->GetCenter();
        mySE->SetPosition(m_position);

        // Force the safe post-warp structural slide velocity vector
        WarpStop(125);
    }
}
*/

void DestinyManager::WarpUpdate(int64 currentShipSpeed, uint16 sec_into_warp, uint8 type/*0*/) {
    //  track position and velocity for all stages.
    m_targetDistance -= currentShipSpeed;
    m_shipVelocity = (m_warpHeading * currentShipSpeed);
    m_position += m_shipVelocity;
    mySE->SetPosition(m_position);

    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        switch (type) {
            case 1: {
                _log(DESTINY__WARP_TRACE, "Destiny::WarpAccel(): %s(%u) - Warp Accelerating(%us): velocity %lli m/s.   %lli m remaining.", \
                        mySE->GetName(), mySE->GetID(), sec_into_warp, currentShipSpeed, m_targetDistance);
            } break;
            case 2: {
                _log(DESTINY__WARP_TRACE, "Destiny::WarpCruise(%s): %s(%u) - Warp Crusing(%us): velocity %lli m/s.  %lli m remaining.", \
                        m_warpState->cruise ? "true":"false", mySE->GetName(), mySE->GetID(), sec_into_warp, currentShipSpeed, m_targetDistance);
            } break;
            case 3: {
                _log(DESTINY__WARP_TRACE, "Destiny::WarpDecel(): %s(%u) - Warp Decelerating(%us): velocity %0.2f,%0.2f,%0.2f, speed %lli m/s.  %lli m remaining.", \
                        mySE->GetName(), mySE->GetID(), sec_into_warp, m_shipVelocity.x, m_shipVelocity.y, m_shipVelocity.z, currentShipSpeed, m_targetDistance);
            } break;
            default: {
            _log(DESTINY__WARNING, "Destiny::WarpUpdate()  %s(%u): Called with no type.", \
                    mySE->GetName(), mySE->GetID());
            }
        }

        _log(DESTINY__WARP_TRACE, "Destiny::WarpUpdate()  %s(%u): Ship is %0.1f from center of target bubble %u.", \
                mySE->GetName(), mySE->GetID(), m_targBubble->GetCenter().distance(m_position), m_targBubble->GetID());
    }
}

void DestinyManager::WarpStop(int64 currentShipSpeed) {
    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        _log(DESTINY__WARP_TRACE, "Destiny::WarpStop(): %s(%u) - Warp complete. Exit velocity %lli m/s with %lli left to go.", \
                mySE->GetName(), mySE->GetID(), currentShipSpeed, m_targetDistance);
        _log(DESTINY__WARP_TRACE, "Destiny::WarpStop(): Ship currently at %0.2f,%0.2f,%0.2f.", \
                m_position.x, m_position.y, m_position.z);
    }

    m_targetPoint += (m_shipVelocity * m_agility);

    Stop();

    if ((mySE->IsNPCSE()) and (mySE->GetNPCSE()->GetAI() != nullptr))
        mySE->GetNPCSE()->GetAI()->WarpOutComplete();

    /*  this isnt used yet, but will be needed once bumping is implemented...
    // reset bump checks
    SetBallMassive sbmassive;
        sbmassive.entityID = mySE->GetID();
        sbmassive.is_massive = sConfig.cosmic.BumpEnabled;
    PyTuple *up = sbmassive.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
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
                // we were orbiting this target, reset movement to void orbit.  should we stop here?  dunno...
                _log(DESTINY__DEBUG, "%u: Our target entity has gone away. Continue on current tangent of orbit.", mySE->GetID());
                GVector heading = m_shipVelocity;
                heading.normalize();
                GotoDirection(heading);
            } break;
            // no default
        }
    }
}

// there may be more here im missing.  we can also target SEs for dock, jump, etc
bool DestinyManager::ValidTarget() {
    if (mySE->SystemMgr()->GetSE(m_targetEntity.first) == nullptr)
        return false;
    if (m_targetEntity.second == nullptr)
        return false;
    if (m_targetEntity.second->IsDead())
        return false;
    if (m_targetEntity.second->IsInvul())
        return false;
    if (m_targetEntity.second->IsFrozen())
        return false;
    if (m_targetEntity.second->DestinyMgr() == nullptr) {
        if (m_targetEntity.second->IsStationSE())
            return true;
        if (m_targetEntity.second->IsGateSE())
            return true;
        if (m_targetEntity.second->IsBeltSE())
            return true;
        if (m_targetEntity.second->IsPOSSE())
            return true;
        if (m_targetEntity.second->IsStaticEntity())
            return false;
        if (m_targetEntity.second->IsItemEntity())
            return true;
        if (m_targetEntity.second->IsObjectEntity())
            return true;
    } else {
        if (m_targetEntity.second->DestinyMgr()->IsWarping())
            return false;
        if (m_targetEntity.second->DestinyMgr()->IsCloaked())
            return false;
    }
    if (m_targetEntity.second->HasPilot())
        if (m_targetEntity.second->GetPilot()->IsDocked())
            return false;

    return true;
}


// Basic Movement Call:
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
            sbmass.mass = mySE->GetSelf()->mass();
        updates.push_back(sbmass.Encode());
        SendDestinyUpdates(updates); //consumed
        m_hasSentShipUpdates = true;
    }

    UnCloak();

    m_stop = false;
    m_skipTic = false;

    // reset move stamps
    m_stateStamp = sEntityMgr.GetStamp();
    m_timeStamp = GetFileTimeNow();

    SetSpeedFraction();

    if (is_log_enabled(DESTINY__MOVE_TRACE)) {
        _log(DESTINY__MOVE_TRACE, " currentVelocity: %0.3f,%0.3f,%0.3f", \
                m_shipVelocity.x, m_shipVelocity.y, m_shipVelocity.z);
    }
}

/*********************************************
 *   Beyonce calls
 */

void DestinyManager::AlignTo(SystemEntity* pSE) {
    // should this Stop() once alignment has been achieved?  i'd say yes.  config option?
    // i originally set it like this, but aknor didnt like it, so it was removed
    m_alignTo = true;
    GotoPoint(pSE->GetPosition());
}

void DestinyManager::ApproachBall(SystemEntity* pSE) {
    // use this for dock and jump where dist > interact dist
    //m_alignTo = true;
    GotoPoint(pSE->GetPosition());
    m_followDistance = pSE->GetRadius() + mySE->GetRadius() + 1000;
    if (m_followDistance > 2500)
        m_followDistance = 2000;
}

void DestinyManager::GotoDirection(const GPoint& direction) {
    m_targetHeading = direction;
    m_targetPoint = m_position + (direction * 1.0e9);
    m_targetDistance = 1.0e9;
    m_followDistance = 0;

    m_ballMode = Destiny::Ball::Mode::GOTO;

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
    m_targetPoint = point;
    GVector heading = m_targetPoint - m_position;
    m_targetDistance = heading.normalize();
    m_targetHeading = heading;
    m_followDistance = 0;

    m_ballMode = Destiny::Ball::Mode::GOTO;

    BeginMovement();

    CmdGotoPoint du;
        du.entityID = mySE->GetID();
        du.x = m_targetPoint.x;
        du.y = m_targetPoint.y;
        du.z = m_targetPoint.z;
    PyTuple* up = du.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

void DestinyManager::FollowBall(SystemEntity* pSE, int32 distance) {
    //called from client as 'CmdFollowBall'
    //  also used by 'Approach'

    m_targetPoint = pSE->GetPosition();
    m_targetEntity.first = pSE->GetID();
    m_targetEntity.second = pSE;

    if (m_autoPilot) {
        // fudge dist a bit for ap
        distance += mySE->GetRadius() + pSE->GetRadius();
    }
    m_followDistance = distance;

    GVector heading = m_targetPoint - m_position;
    m_targetDistance = heading.normalize();
    m_targetHeading = heading;

    m_ballMode = Destiny::Ball::Mode::FOLLOW;

    BeginMovement();

    CmdFollowBall du;
        du.entityID = mySE->GetID();
        du.targetID = pSE->GetID();
        du.range = distance;
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

void DestinyManager::OrbitBall(SystemEntity *pSE, uint32 distance/*0*/) {
    if (!sConfig.debug.UseOrbit) {
        Stop();
        return;
    }

    GPoint targetPos = pSE->GetPosition();

    m_ballMode = Destiny::Ball::Mode::ORBIT;
    m_targetEntity.first = pSE->GetID();
    m_targetEntity.second = pSE;
    if (!ValidTarget()) {
        Stop();
        return;
    }

    // do we need to split mass here?
    /*
    float totalMass = (mySE->GetSelf()->mass() / 1000000) + (pSE->GetSelf()->mass() / 1000000); // in Kg
    if (totalMass > 0.0f)
        distance *= (pSE->GetSelf()->mass() / totalMass);
    */

    m_followDistance = distance;
    m_targetDistance = m_followDistance + pSE->GetRadius() + mySE->GetRadius();

    m_ballMode = Destiny::Ball::Mode::ORBIT;

    if (is_log_enabled(DESTINY__ORBIT_TRACE))
        _log(DESTINY__ORBIT_TRACE, "Destiny::OrbitBall() %s(%u) target: %s(%u) @ %u (%u)", \
                mySE->GetName(), mySE->GetID(), pSE->GetName(), pSE->GetID(), distance, m_targetDistance);

    // =========================================================================
    // CORRECTED GEOMETRIC ANCHOR: ZEROED ON TARGET
    // =========================================================================
    GVector targetCenter = pSE->GetPosition();

    // Draw the baseline radial vector extending directly OUTWARD from the Target's center
    GVector targetToShipVec = m_position - targetCenter;
    float centerDistance = targetToShipVec.length();

    // Dead-Center Safety Trap
    if (centerDistance < 0.01f) {
        targetToShipVec = GVector(0.0f, 0.0f, 1.0f); // Default to world Z out if perfectly overlapping
    }

    // Set Basis Z as our permanent 0-degree anchor, rooted at Target Center
    m_orbitBasisZ = targetToShipVec;
    m_orbitBasisZ.normalize();

    // Fetch relative movement vector to look for approach heading
    GVector relativeVel = m_shipVelocity - pSE->DestinyMgr()->GetVelocity();

    // =========================================================================
    // CORRECTED DEAD-STOP MATRIX ALIGNMENT (Flipped-Normal Safe)
    // =========================================================================
    if (relativeVel.length() < 0.001f || std::abs(relativeVel.dotProduct(m_orbitBasisZ)) > 0.99f) {
        // Instead of forcing a static normal, we force a static tangent line of sight!
        // We construct a clean horizontal wing tool to act as our side marker.
        GVector horizontalWingTool(0.0f, 1.0f, 0.0f);

        // If the ship is sitting directly above or below the target's vertical poles,
        // swap our reference tool to the world's forward Z axis to avoid a zero collapse.
        if (std::abs(m_orbitBasisZ.y) > 0.90f)
            horizontalWingTool = GVector(0.0f, 0.0f, 1.0f);

        // 1. First, establish Basis X strictly to the side of our line of sight
        m_orbitBasisX = m_orbitBasisZ.crossProduct(horizontalWingTool);
        m_orbitBasisX.normalize();

        // 2. NOW derive the normal by crossing Z and X!
        // Because this cross product reads your actual position vector (m_orbitBasisZ),
        // it will automatically point the normal UP if you are above,
        // and DOWN if you are below, perfectly preserving the client's coordinate frames!
        m_orbitNormal = m_orbitBasisZ.crossProduct(m_orbitBasisX);
        m_orbitNormal.normalize();
    } else {
        // Standard operational approach (Ship possesses an active velocity heading angle)
        if (relativeVel.dotProduct(m_shipVelocity) < 0) {
            relativeVel = relativeVel * -1.0f;
        }

        // 1. First, establish Basis X strictly to the side of our line of sight
        m_orbitBasisX = m_orbitNormal.crossProduct(relativeVel);
        m_orbitBasisX.normalize();

        // 2. NOW derive the normal by crossing Z and X!
        m_orbitNormal = m_orbitBasisZ.crossProduct(m_orbitBasisX);
        m_orbitNormal.normalize();

    }

    if (true /*mark orbit ordinals*/) {
        AddOrbitMarkers(centerDistance, targetCenter);
        return;
    }

    CalculateMaxOrbitSpeedFraction();

    // this isnt right...should be 90* to the side of target.
    GVector heading = targetPos - m_position;
    m_targetDistance = heading.normalize();
    m_targetHeading = heading;

    if (is_log_enabled(DESTINY__ORBIT_TRACE)) {
        _log(DESTINY__ORBIT_TRACE, "Destiny::OrbitBall()  %s(%u) - m_orbitNormal is %0.2f, %0.2f, %0.2f @ %0.5f", \
                mySE->GetName(), mySE->GetID(), m_orbitNormal.x, m_orbitNormal.y, m_orbitNormal.z, \
                m_maxOrbitSpeedFraction);
        _log(DESTINY__ORBIT_TRACE, "Destiny::OrbitBall()  heading to target is %0.2f, %0.2f, %0.2f @ %0.5f", \
                heading.x, heading.y, heading.z, distance);
    }

    BeginMovement();

    CmdOrbit du;
        du.entityID = mySE->GetID();
        du.orbitEntityID = pSE->GetID();
        du.distance = distance;
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

void DestinyManager::CalculateMaxOrbitSpeedFraction() {
    if ((m_maxSpeed <= 0.0f) or (m_followDistance <= 0.0f) or (m_posScale <= 0.0f)) {
        m_maxOrbitSpeedFraction = 0.0f;
        return;
    }

    // 2. Compute the structural geometric ratio
    // This measures the ship's ability to pull its vector inward relative to its forward velocity
    double dragMomentum = m_maxSpeed * SPACE_DRAG;
    double ratio = dragMomentum / m_followDistance;     // this may need actual targDistance
    if (ratio > 40.0)
        ratio = 40.0;

    // 3. Apply the clean inverse square root layout
    double bottomTerm = 1.0f + (ratio * ratio);

    // m_maxOrbitSpeedFraction will naturally drop below 1.0f for tight orbits,
    // and float safely close to 1.0f (max speed) for massive orbits.
    m_maxOrbitSpeedFraction = 1.0f / std::sqrt(bottomTerm);

    // Optional client visual padding: Destiny caps stable orbits to a max fraction of ~0.99
    if (m_maxOrbitSpeedFraction > 0.99f)
        m_maxOrbitSpeedFraction = 0.99f;

    if (is_log_enabled(DESTINY__ORBIT_TRACE))
        _log(DESTINY__ORBIT_TRACE, "Destiny::CalculateOrbit() %s(%u)  dragMomentum: %0.2f, ratio: %0.2f, bottomTerm: %0.2f @ %0.3f", \
                mySE->GetName(), mySE->GetID(), dragMomentum, ratio, bottomTerm, \
                m_maxOrbitSpeedFraction);
}

// Fleet warps - on live, all ships use the warp profile of the slowest ship, here they warp as normal
void DestinyManager::WarpTo(const GPoint& destPoint, int32 distance, bool autoPilot, SystemEntity* pSE/*nullptr*/) {
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
            Stop();
            sLog.Error("Destiny::WarpTo()", "DestPoint is zero and pSE is null.");
            if (mySE->HasPilot())
                throw UserError("WarpDestinationGone");
            return;
        }
    } else {
        m_targetPoint = destPoint;
    }

    GVector warp_vector(m_position, m_targetPoint);
    m_followDistance = 0;
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
                //TooCloseToWarp
                pClient->SendErrorMsg("That is too close for your Warp Drive.  Approaching Target.");
                FollowBall(pSE, distance);
            } else {
                pClient->SendErrorMsg("That is too close for your Warp Drive.  Stopping Ship.");
                Stop();
            }
            return;
        }

        // check for enough cap to warp.

        /*  capacitor for warp formulas from https://oldforums.eveonline.com/?a=topic&threadID=332116
         *  Energy to warp = warpCapacitorNeed * mass(kG) * au * (1 - warp_drive_operation_skill_level * 0.10)
         ** @note:  warpCapacitorNeed is type double...max ive seen is shuttles @ 0.00000134771  and indys @ 0.000000108911
         */

        double dist((double)m_targetDistance / ONE_AU_IN_METERS);
        double currentShipCap = pClient->GetShip()->GetAttribute(AttrCapacitorCharge).get_double();
        double capNeeded = mySE->GetSelf()->mass() * m_warpCapacitorNeed * dist;
        capNeeded *= (1.0 - (0.1 * pClient->GetChar()->GetSkillLevel(EvESkill::WarpDriveOperation)));

        _log(DESTINY__WARNING, "Warp Cap need for %s(%u) for %llim (%0.3f AU) is %0.5f GJ", \
            mySE->GetName(), mySE->GetID(), m_targetDistance, dist, capNeeded);

        // set min cap need to 1.0
        if (capNeeded < 1.0f)
            capNeeded = 1.0;

        //  check if ship has enough capacitor to warp full distance
        if (capNeeded > currentShipCap) {
            // not enough cap.  reset everything based on available cap
            double capHas = currentShipCap / (mySE->GetSelf()->mass() * m_warpCapacitorNeed);
            if (capHas > 1.0f) {
                m_targetDistance = capHas * ONE_AU_IN_METERS;
                GVector warp_direction(m_position, m_targetPoint);
                // make heading
                warp_direction.normalize();
                GPoint newTarget(m_position + (warp_direction * m_targetDistance));
                m_targetPoint = newTarget;
                m_targBubble = sBubbleMgr.GetBubble(mySE->SystemMgr(), newTarget);
                pClient->SendErrorMsg("Your commanded warp of %0.3f AU requires %0.1f GJ of capacitor charge, but you only have %0.1f GJ available.<br>This is enough for %0.3f AU.<br><br>Setting course for available capacitor charge.", \
                        dist, capNeeded,  currentShipCap, capHas );
                /** @todo  update all move vars here for new target... */

                GVector new_warp_vector(m_position, m_targetPoint);
                m_targetDistance = new_warp_vector.length();
                m_targetDistance -= distance;
            } else {
                // if not enough cap to do min warp. cancel and return
                //WarpingWithAvailablePowerBody
                pClient->SendErrorMsg("You don't have enough capacitor charge to warp.");
                _log(DESTINY__WARNING, "Destiny::InitWarp() - %s(%u): Capacitor needed vs current  %f / %0.5f",
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

    //You will always exit warp at a random point 2,500 meters from your actual exit point - per EveUni
	//  this will need update for formation warp...leader will have random offset, others follow per formation
    /* disabled for testing
     * if (mySE->HasPilot())
     *   m_targetPoint.MakeRandomPointOnSphereLayer(-2500, 2500);   disabled for testing
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
    if (mySE->IsNPCSE()) {
        BeginMovement();
        // if usf < 75%, set usf for warp
        if (m_userSpeedFraction < 0.749f)
            m_userSpeedFraction = 1.0f;

        // reset ball mode as it was changed in SSF()
        m_ballMode = Destiny::Ball::Mode::WARP;

        // if no players in bubble, this isnt needed...
        if (mySE->SysBubble()->HasPlayers()) {
            std::vector<PyTuple*> up;
            // send warp update
            CmdWarpTo wt;
                wt.entityID = mySE->GetID();
                wt.dest_x = m_targetPoint.x;
                wt.dest_y = m_targetPoint.y;
                wt.dest_z = m_targetPoint.z;
                wt.distance = distance;
                wt.warpSpeed = GetWarpSpeed();      // warp speed x10
            up.push_back(wt.Encode());
            SendDestinyUpdates(up); //consumed
            // send warp gfx
            SendGFX10(mySE->GetID(), "effects.Warping" );
        }

        if (is_log_enabled(NPC__MESSAGE))
            _log(NPC__MESSAGE, "Destiny::WarpTo() NPC %s(%u) %u -> %u, m_targetPoint: %0.2f,%0.2f,%0.2f  stop distance: %i  m_targetDistance: %lli", \
                    mySE->GetName(), mySE->GetID(), mySE->SysBubble()->GetID(), m_targBubble->GetID(), \
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
     * AttrWarpBubbleImmune = 1538,  (none in db)
     * AttrWarpBubbleImmuneModifier = 1539,     (4 in db)
     *
     *   NOTE:  warp bubble in path (or within 150km of m_targetPoint) will change m_targetDistance and m_targetPoint
     *   however, this does NOT affect original calculations for energy needed, etc...
     */

    /** @todo  does this apply for ANY bubble along warp route or just end?
     * would be fun to check entire route for bubble...can bm along route to add bubble
     * however, this will take a bit to implement
     *
     * IDEA:  ...nope, lost it.
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
    }

    BeginMovement();
    // if usf < 75%, set usf for warp
    if (m_userSpeedFraction < 0.751)
        SetSpeedFraction(1.0f);

    // reset ball mode as it was changed in SSF()
    m_ballMode = Destiny::Ball::Mode::WARP;

    if (is_log_enabled(DESTINY__WARP_DEBUG)) {
        // marker for ship's stop point
        std::string str = "Warp Stop Point - ";
        str += mySE->GetName();
        MarkPoint(m_targetPoint, str, str);
        str.clear();

        // marker for ship's drop out of warp point
        GPoint dropPos(m_targetHeading * m_speedToLeaveWarp * m_agility);
        str = "Warp Drop Point - ";
        str += mySE->GetName();
        MarkPoint(m_targetPoint - dropPos, str, str);
    }

    if (sConfig.cosmic.BumpEnabled) {
        // NOTE:  sending ball mass isnt required if is_massive=false
        //set massive for warp.   self-only per client logs
        SetBallMassive bm;
            bm.entityID = mySE->GetID();
            bm.is_massive = false;       // disable client-side bump checks
        PyTuple *up = bm.Encode();
        SendSingleDestinyUpdate(&up, true);
        PyDecRef(up);
    }

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

    if (is_log_enabled(DESTINY__WARP_TRACE))
        _log(DESTINY__WARP_TRACE, "Destiny::WarpTo() %u -> %u, m_targetPoint: %0.2f,%0.2f,%0.2f  stop distance: %i  m_targetDistance: %lli",
             mySE->SysBubble()->GetID(), m_targBubble->GetID(), m_targetPoint.x, m_targetPoint.y, m_targetPoint.z, distance, m_targetDistance);
}

bool DestinyManager::IsAligned(GVector& targHeading) {
    GVector heading = m_shipVelocity;
    float distance = heading.normalize();     //change vector to direction

    if (distance < 0.01f) {
        // if ship is stopped enough, there is no 'aligning' to do
        m_targetHeading = targHeading;
        return true;
    }

    double dot = heading.dotProduct(targHeading);
    //double degrees = EvE::Trig::Rad2Deg(std::acos(dot));
    if (m_ballMode == Destiny::Ball::Mode::WARP) {
        if (dot > /*WARP_ALIGNMENT*/ 0.99449256)
            return true;
    } else if (dot > /*TURN_ALIGNMENT*/ 0.99756405) {
        return true;
    }
    return false;
}

void DestinyManager::Undock(GPoint dir) {
    //set movement direction
    m_targetPoint = dir * 1.0e9;
    m_targetHeading = dir;
    SetUndockSpeed();
}

void DestinyManager::SetUndockSpeed() {
    //start ship movement @ max velocity for undocking.  also used by missiles to simulate launching @ full speed
    // this simulates being forcefully "ejected" from station
    m_userSpeedFraction = 1.0f;
    m_activeSpeedFraction = 1.0f;

    m_shipVelocity = m_targetHeading * m_maxSpeed;
    m_targetVelocity = m_shipVelocity;

    if (m_ballMode == Destiny::Ball::Mode::MISSILE)
        return;

    m_skipTic = false;
    m_ballMode = Destiny::Ball::Mode::GOTO;

    std::vector<PyTuple*> updates;
    CmdGotoDirection du;
        du.entityID = mySE->GetID();
        du.x = m_targetHeading.x;
        du.y = m_targetHeading.y;
        du.z = m_targetHeading.z;
    updates.push_back(du.Encode());
    SetBallVelocity bv;
        bv.entityID = mySE->GetID();
        bv.x = m_shipVelocity.x;
        bv.y = m_shipVelocity.y;
        bv.z = m_shipVelocity.z;
    updates.push_back(bv.Encode());
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

    //get the station Docking Perimeter
    const GPoint stationPos = stationSE->GetPosition();
    double rangeToStationPerimiter = m_position.distance(stationPos);
    rangeToStationPerimiter -= mySE->GetRadius();
    rangeToStationPerimiter -= stationSE->GetRadius();

    // Verify range to station is within docking perimeter of 2500 meters:
    _log(DESTINY__TRACE, "Destiny::AttemptDockOperation() rangeToStationPerimiter is %0.2fm", rangeToStationPerimiter);
    if (rangeToStationPerimiter > 2500.0) {
        // Turn ship and move toward docking point - client will usually call Dock() automatically...sometimes
        ApproachBall(stationSE);
        if (mySE->HasPilot() and mySE->GetPilot()->CanThrow())
            throw UserError("DockingApproach");
    }

    pClient->SetStateTimer(Player::State::Dock, sConfig.world.StationDockDelay * 1000); // default @ 4sec();
    pClient->SetAutoPilot(false);

    //pClient->SetDocking(true);

    return new PyLong(GetFileTimeNow());
}

void DestinyManager::DockingAccepted() {
    Stop();
    // does ship need to be uncloaked to dock?  probably not...
    //UnCloak();
    Client *pClient(mySE->GetPilot());
    // this would be an error.  only players call this method.
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

    // fix for elusive error with SE's being out of assigned bubble...part 1
    if (mySE->SysBubble() != nullptr)
        if (!mySE->SysBubble()->InBubble(pt))
            sBubbleMgr.Remove(mySE);

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

    // fix for elusive error with SE's being out of assigned bubble...part 2
    if (mySE->SysBubble() == nullptr)
        sBubbleMgr.Add(mySE);

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

// settings for higher npc max speeds
void DestinyManager::SetMaxVelocity(uint16 maxVelocity) {
    if (mySE->HasPilot()) {
        // error
        _log(DESTINY__ERROR, "Destiny::SetMaxVelocity(%u) - Called by Player Ship %s(%u)", \
                maxVelocity, mySE->GetName(), mySE->GetID());
        return;
    }
    uint16 maxSpeed = mySE->GetSelf()->GetAttribute(AttrMaxVelocity).get_uint32();

    if (is_log_enabled(DESTINY__TRACE))
        _log(DESTINY__TRACE, "Destiny::SetMaxVelocity() - Ship:%s(%u) - maxSpeed is %u, update sent %u", \
                mySE->GetName(), mySE->GetID(), maxSpeed, maxVelocity);

    if (maxVelocity > maxSpeed) {
        m_maxSpeed = maxVelocity;
    } else {
        m_maxSpeed = maxSpeed;
    }
}

void DestinyManager::SpeedBoost(ModuleItemRef mRef, bool deactivate/*false*/) {
    // Recalculate ship variables
    SetAgilityInertia();
    // update velocity vector for changes
    m_targetVelocity = m_targetHeading * (m_maxSpeed * m_userSpeedFraction);
    // send out updated ship data
    InventoryItemRef sRef = mySE->GetSelf();
    std::vector<PyTuple*> updates;
    SetBallAgility sbagility;
        sbagility.entityID =  mySE->GetID();
        sbagility.agility = sRef->GetAttribute(AttrInertiaMod).get_double();
    updates.push_back(sbagility.Encode());
    SetBallMass sbmass;
        sbmass.entityID = mySE->GetID();
        sbmass.mass = sRef->GetAttribute(AttrMass).get_double();
    updates.push_back(sbmass.Encode());
    SetBallSpeed sbms;
        sbms.entityID = mySE->GetID();
        sbms.speed = m_maxSpeed;
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

        _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost() - nMass: %0.5f, nAg: %0.5f, usf: %0.2f, asf: %0.3f", \
                mySE->GetSelf()->mass(), m_agility, m_userSpeedFraction, m_activeSpeedFraction);
    }
}

void DestinyManager::WebbedMe(InventoryItemRef modRef, bool apply/*false*/)
{
    // speed changes are immediate.  do not call MoveObject() as that will force accel/decel
    float maxSpeed = 0.0f;
    if (apply) {
        maxSpeed *= (1 + (modRef->GetAttribute(AttrSpeedFactor).get_float() / 100.0f));
    } else {
        maxSpeed /= (1 + (modRef->GetAttribute(AttrSpeedFactor).get_float() / 100.0f));
    }

    // set asf as fraction of current speed over new max speed.   it will apply on next tic (immediate)
    // this may give >1.0 when web applied.
    m_activeSpeedFraction = (maxSpeed * m_activeSpeedFraction) / m_maxSpeed;

    std::vector<PyTuple*> updates;
    SetBallSpeed sbms;
        sbms.entityID = mySE->GetID();
        sbms.speed = maxSpeed;
        updates.push_back(sbms.Encode());
    SendDestinyUpdates(updates); //consumed
    m_hasSentShipUpdates = true;    // just in case, as this is re-sent in BeginMovement()
}

// Global collision methods
//  check for collision.  called by MoveObject()
void DestinyManager::CheckBump()
{
    double profileStartTime = GetTimeUSeconds();

    //  collision detection code here
    /*  in this case, we are ONLY interested in objects
     *   that have drifted within each others radius (for whatever reason)
     *  this only checks for ships running sub-warp speeds
     *   in relation to other objects in bubble.
     */

    // NOTE:  object's "massive = true" means it can bump/collide  (massive = solid)
    // will need massive checks here


    // once we get list of massive objects in this bubble, check distances
    /* distance filter
     * delta = objA.position - objB.position
     * radius = objA.radius + objB.radius
     * travel = objA.speed + objB.speed
     * threshold = radius + travel
     * if (delta > threshold)
     *     continue..too far
     */

    /* code ideas for ships close enough to check.
     * delta = objA.position - objB.position
     * distSq = delta.lengthSquared
     * radius = objA.radius + objB.radius
     * radiusSq = radius * radius
     * if (deltaSq <= radiusSq)
     *     bump!
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
     *   bump drones??  nope...not a thing
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
    /*
    // 1. Identify Mass Values from Verified Memory Offsets (0x78)
    double m1 = incomingShip->mass;

    // If obstacle is a FIELD, STATION, or STARGATE, it has infinite mass relative to a ship
    double m2 = (obstacleBall->mode == Destiny::Ball::Mode::FIELD || obstacleBall->mode == Destiny::Ball::Mode::RIGID)
                ? 999999999999.0
                : obstacleBall->mass;

    // 2. Calculate Velocity Vector Projection along the collision axis
    GVector relativeVelocity = incomingShip->velocity - obstacleBall->velocity;
    float approachSpeed = relativeVelocity.Dot(collisionNormal);

    // If they are already moving away from each other, skip the physics calculation
    if (approachSpeed >= 0.0f) return;

    // 3. Apply Conservation of Momentum for the Rebound Magnitude
    // A standard elastic coefficient (e.g., 1.25) ensures a distinct bounce away
    float elasticity = 1.25f;
    float impulseMagnitude = -(1.0f + elasticity) * approachSpeed / (1.0f / m1 + 1.0f / m2);

    // 4. Calculate Final Vector Allocation
    // The change in velocity is inversely proportional to the ship's own mass
    GVector velocityDelta = collisionNormal * (impulseMagnitude / m1);

    // Apply the new forces to the active velocity vector (0x120)
    incomingShip->velocity = incomingShip->velocity + velocityDelta;

    // 5. Anti-Exploit Speed Cap Ceiling Verification (Offset 0x64 / 0x78 Limits)
    // Clamp the resultant bounce speed so it cannot exceed the ship's max profile
    float resultantSpeed = incomingShip->velocity.Length();
    if (resultantSpeed > incomingShip->maxShipSpeed) {
        incomingShip->velocity = incomingShip->velocity.Normalize() * incomingShip->maxShipSpeed;
    }

    // 6. Force Autopilot Systems to Disengage
    // Drop the ship into inertial drift deceleration loop (STOP Mode 2)
    m_ballMode = Destiny::Ball::Mode::STOP;
    m_stop = true;
    m_userSpeedFraction = 0.0f; // Drops the requested throttle to zero
    m_stateStamp = sEntityMgr.GetStamp();

    // 7. Network Synchronization Step
    std::vector<PyTuple*> updates;
    SetBallVelocity bv;
        bv.entityID = incomingShip->GetID();
        bv.x = incomingShip->velocity.x;
        bv.y = incomingShip->velocity.y;
        bv.z = incomingShip->velocity.z;
    updates.push_back(bv.Encode());

    CmdStopShip ss;
        ss.entityID = incomingShip->GetID();
    updates.push_back(ss.Encode());

    SendDestinyUpdates(updates);
    Stop();
    */
}
/*
void DestinyManager::ExecuteShipToShipBump(ShipEntity* shipA, ShipEntity* shipB) {
    // Both hulls must have hard collision solidity active (Offset 0x56) to resolve elastic impacts
    if (!shipA->hardCollisionSolid || !shipB->hardCollisionSolid) return;

    // 1. CALCULATE SPATIAL INTERSECTIONS
    GVector deltaPos = shipB->position - shipA->position;
    double currentDistance = deltaPos.length();
    double combinedRadii = shipA->radius + shipB->radius;

    // Boundary Breach Verification
    if (currentDistance < combinedRadii && currentDistance > 0.001) {
        Vector3 collisionNormal = deltaPos.normalize();

        // --- STEP A: RESOLVE PHYSICAL GEOMETRICAL CLIPPING (PUSH-OUT) ---
        // To prevent ships from overlapping across 1Hz ticks, project them outward along the normal axis
        double overlapDepth = combinedRadii - currentDistance;

        // Push distribution is weighted inversely by mass (Heavier ships move less)
        double totalInverseMass = (1.0 / shipA->mass) + (1.0 / shipB->mass);
        double pushA = (1.0 / shipA->mass) / totalInverseMass * overlapDepth;
        double pushB = (1.0 / shipB->mass) / totalInverseMass * overlapDepth;

        shipA->position = shipA->position - (collisionNormal * pushA);
        shipB->position = shipB->position + (collisionNormal * pushB);

        // --- STEP B: ELASTIC MOMENTUM REFLECTION MATH ---
        // Isolate the approach velocity component vectors along the impact axis
        Vector3 relativeVelocity = shipA->velocity - shipB->velocity;
        double approachSpeed = relativeVelocity.dot(collisionNormal);

        // Execute calculations only if they are actively moving TOWARD each other
        if (approachSpeed > 0.0) {
            // Standard coefficient of elasticity (1.30 adds a noticeable physical bounce)
            double elasticity = 1.30;
            double impulseMagnitude = (1.0 + elasticity) * approachSpeed / totalInverseMass;

            // Apply direct physics corrections to the Active Velocity Vectors (0x120)
            shipA->velocity = shipA->velocity - (collisionNormal * (impulseMagnitude / shipA->mass));
            shipB->velocity = shipB->velocity + (collisionNormal * (impulseMagnitude / shipB->mass));

            // --- STEP C: EXPLOIT GOVERNOR CEILING CLAMPING ---
            // Clamp resulting vectors so no ship gains free speed beyond its absolute structural cap
            if (shipA->velocity.length() > shipA->maxSpeed)
                shipA->velocity = shipA->velocity.normalize() * shipA->maxSpeed;
            if (shipB->velocity.length() > shipB->maxSpeed)
                shipB->velocity = shipB->velocity.normalize() * shipB->maxSpeed;

            // --- STEP D: AUTOMATIC FLIGHT TRAJECTORY INTERRUPTION ---
            // Force break autopilots for both hulls. Cut throttles and force STOP (2) mode
            ShipEntity* ships[2] = { shipA, shipB };
            for (int i = 0; i < 2; ++i) {
                ships[i]->coreMovementMode = 2;       // Snap Offset 0x198 to Mode::STOP
                ships[i]->cruiseFraction = 0.0f;     // Force throttle register 0x6c to zero
                ships[i]->stopStateActive = true;     // Engage Block Two braking decay variables
                ships[i]->isMoving = true;
            }

            // --- STEP E: PACKET NETWORK DISPATCH ---
            // Trigger your outbound notification buffers immediately.
            // Observers inside the grid bubble receive the standard 1Hz movement delta frames (packet_type = 1).
            // They see both hulls bounce outward symmetrically and smoothly slide down to an absolute standstill!
            NotifyGridObserversOfBump(shipA);
            NotifyGridObserversOfBump(shipB);
        }
    }
}
*/
/*
void ProcessShieldCollision(Destiny::BallInstance* ship, Destiny::BallInstance* shieldField) {
    // 1. Authorization check using our verified memory offsets
    if (ship->harmonic == shieldField->harmonic ||
        ship->corporationID == shieldField->corporationID ||
        ship->allianceID == shieldField->allianceID) {
        return; // Authorized: ship passes smoothly through the FIELD barrier
    }

    // 2. Proximity boundary check
    Vector3 delta = ship->position - shieldField->position;
    double distance = delta.Length();
    double combinedRadius = shieldField->radius + ship->radius;

    if (distance < combinedRadius && distance > 0.0) {
        // 3. Set collision tracking states in memory matrix
        ship->hardCollisionToggle = true;

        // Calculate surface normal unit vector
        Vector3 normal = delta * (1.0 / distance);

        // 4. Position Correction (Prevent grid clipping)
        ship->position = shieldField->position + (normal * combinedRadius);

        // 5. Elastic Reflection against the moving components (0x120 Velocity Vector)
        double vNormal = ship->velocity.Dot(normal);

        if (vNormal < 0.0) { // Only reflect if moving inward
            // 1.5 multiplier reflects velocity and adds a 50% bounce impulse away from the hull
            Vector3 reflectionImpulse = normal * (1.5 * vNormal);
            ship->velocity = ship->velocity - reflectionImpulse;

            // 6. Update target vector cache (0x138) to disrupt ongoing autopilot/orbit trajectories
            ship->targetVelocityCache = ship->velocity;
            ship->coreMovementMode = Destiny::Ball::Mode::STOP; // Force flight state deceleration loop

            // Queue a delta telemetry update packet to update all observers on the grid
            QueueDeltaPacketToGridCell(ship->currentCellID, ship);
        }
    }
}
*/
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
    if (newShipRef->typeID() == EVEDB::invTypes::Capsule) {
        slim->SetItemString("launcherID",               new PyInt(mySE->GetShipSE()->GetLauncherID()));
        slim->SetItemString("modules",                  PyStatic.mtList());
    } else {
        slim->SetItemString("categoryID",               new PyInt(newShipRef->categoryID()));
        slim->SetItemString("groupID",                  new PyInt(newShipRef->groupID()));
        slim->SetItemString("modules",                  newShipRef->ShipGetModuleList());
    }

    std::vector<PyTuple*> updates;
    PyTuple* shipData = new PyTuple(2);
        shipData->SetItem(0, new PyLong(newShipRef->itemID()));
        shipData->SetItem(1, new PyObject("foo.SlimItem", slim));
    PyTuple* shipItem = new PyTuple(2);
        shipItem->SetItem(0, new PyString("OnSlimItemChange"));
        shipItem->SetItem(1, shipData);
    updates.push_back(shipItem);
    SendDestinyUpdates(updates);        // consumed

    UpdateShipVariables();
    SendBallInteractive(newShipRef, true);
}

void DestinyManager::SetAgilityInertia() {
    InventoryItemRef sRef = mySE->GetSelf();
    /* The product of Mass and InertiaMod gives the item's Agility
     *  Agility = Mass x InertiaMod / 1000000
     *  Agility is an internal server variable.
     */
    if (!sRef->HasAttribute(AttrInertiaMod))    // this should never hit
        sLog.Error("DM::UpdateShipVariables", "%s(%u) does not have an InertiaMod", mySE->GetName(), mySE->GetTypeID());

    float mass = sRef->mass(); // in mKg
    double inertiaMod = sRef->GetAttribute(AttrInertiaMod).get_double();

    // CRITICAL PRE-CALCULATION: Compute the 0x80 Exponential Agility Coefficient constant
    // This represents e^(-dt/tau) where dt = tic time in seconds (dt = 1.0)
    m_agility = mass * inertiaMod / 1000000.0f;
    m_expTerm = std::exp(-1.0 / m_agility);
    m_posScale = m_agility * (1.0 - m_expTerm);

    // agility is internal for us
    sRef->SetAttribute(AttrAgility, m_agility, false);

    m_alignTime = 1.386294 * m_agility;

    // for npc, this is mwd/ab speed, not sub-warp speed
    if (mySE->IsNPCSE()) {
        // see if we can set this to absolute max and throttle with m_userSpeedFraction (maybe not)
        m_maxSpeed = sRef->GetAttribute(AttrEntityCruiseSpeed).get_float();
    } else if (sRef->HasAttribute(AttrMaxVelocity)) {
        m_maxSpeed = sRef->GetAttribute(AttrMaxVelocity).get_float();
    }

    // verify hull overspeed isnt reached
    if (sRef->HasAttribute(AttrMaxDirectionalVelocity)) {
        if (m_maxSpeed > sRef->GetAttribute(AttrMaxDirectionalVelocity).get_float())
            m_maxSpeed = sRef->GetAttribute(AttrMaxDirectionalVelocity).get_float();
    }

    if (is_log_enabled(DESTINY__MOVE_DEBUG))
        _log(DESTINY__MOVE_DEBUG, "Destiny::SetAgilityInertia() %s(%u) - m_maxSpeed: %0.1f, m_alignTime: %0.2f, mass: %0.1f * inertiaMod: %0.3f / 1000000 = m_agility: %0.2f, m_expTerm: %0.2f, m_posScale: %0.3f", \
                mySE->GetName(), mySE->GetID(), m_maxSpeed, m_alignTime, mass, inertiaMod, m_agility, m_expTerm,  m_posScale);
}

/*  called from
 * Client::ResetAfterPodded()
 * NPC::NPC()
 * Concord::Concord()
 * DestinyManager::UpdateNewShip()
 * DynamicEntityFactory::BuildEntity()  (for abandoned ships)
 */
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
    /** @todo check for movement when fleet boosts are applied and this is called */

    /* this sets variables needed for correct movement math.
     *  these attribs are set for ship item when ship's SE created.  DO NOT modify anything here
     * this is also called when fleet boosts are updated.
     */

    SetAgilityInertia();

    InventoryItemRef sRef = mySE->GetSelf();
    if (mySE->IsNPCSE()) {
        // npcs dont have AttrWarpSpeedMultiplier.  set based on size
        switch (mySE->GetNPCSE()->GetAI()->GetSize()) {
            //0.25, 0.75, 1.0, 1.25, 1.5, 2.0, 3.0, 3.75, 4.5, 5.0, 6.0, 7.5, 9.0, 10.0, 13.5, 15.0
            case NPCAI::Size::Swarm: {
                m_shipWarpSpeed = 15.0f;
            } break;
            case NPCAI::Size::Frigate: {
                m_shipWarpSpeed = 9.0f;
            } break;
            case NPCAI::Size::Destroyer: {
                m_shipWarpSpeed = 7.5f;
            } break;
            case NPCAI::Size::Cruiser: {
                m_shipWarpSpeed = 5.0f;
            } break;
            case NPCAI::Size::BCruiser: {
                m_shipWarpSpeed = 3.0f;
            } break;
            case NPCAI::Size::BShip: {
                m_shipWarpSpeed = 1.5f;
            } break;
            case NPCAI::Size::Indy: {
                m_shipWarpSpeed = 0.75f;
            } break;
        }
    } else if (sRef->HasAttribute(AttrWarpSpeedMultiplier)) {
        // this will catch speeds/needs for all player ships, and is easier to do here.
        m_shipWarpSpeed = sRef->GetAttribute(AttrWarpSpeedMultiplier).get_float();
    }

    if (m_shipWarpSpeed < 0.25f)
        sLog.Error("DM::UpdateShipVariables", "%s(%u) m_shipWarpSpeed is %0.2f", \
                mySE->GetName(), mySE->GetTypeID(), m_shipWarpSpeed);

    if (sRef->HasAttribute(AttrWarpCapacitorNeed)) {
        m_warpCapacitorNeed = sRef->GetAttribute(AttrWarpCapacitorNeed).get_double();
    } else {
        m_warpCapacitorNeed = 0.000000138;   // lowest value in db
    }

    /*  per https://forums.eveonline.com/default.aspx?g=posts&m=3912843   post#103
     *
     * Ships will exit warp mode when their current speed drops below
     * 75% of sub-warp max speed, or 100m/s, whichever is the lower.
     */
    m_speedToLeaveWarp = m_maxSpeed * 0.75f;

    if ((m_speedToLeaveWarp < 101) and (m_maxSpeed > 135))      // 75% of 135 is 101.25
        m_speedToLeaveWarp = 101;

    if (m_warpHeading.isZero())
        m_warpHeading = GPOINT_IDENTITY;

    if (!mySE->HasPilot())
        return;

    if (mySE->GetPilot()->IsLogin())
        return;

    if (mySE->GetPilot()->IsInSpace() and (mySE->SysBubble() != nullptr)) {
        std::vector<PyTuple*> updates;
        SetBallAgility sbagility;
            sbagility.entityID =  mySE->GetID();
            sbagility.agility = sRef->GetAttribute(AttrInertiaMod).get_double();;
        updates.push_back(sbagility.Encode());
        SetBallMassive sbmassive;
            sbmassive.entityID = mySE->GetID();
            sbmassive.is_massive = sConfig.cosmic.BumpEnabled;
        updates.push_back(sbmassive.Encode());
        SetBallMass sbmass;
            sbmass.entityID = mySE->GetID();
            sbmass.mass = sRef->mass();
        updates.push_back(sbmass.Encode());
        SetBallSpeed sbspeed;
            sbspeed.entityID = mySE->GetID();
            sbspeed.speed = m_maxSpeed;
        updates.push_back(sbspeed.Encode());
        SendDestinyUpdates(updates); //consumed
        m_hasSentShipUpdates = true;
    }
}

void DestinyManager::MakeMissile(Missile* pMissile) {
    SetMaxVelocity(pMissile->GetSpeed());
    //SetPosition(pMissile->GetSelf()->position());
    m_agility = (pMissile->GetSelf()->type().mass() * pMissile->GetSelf()->GetAttribute(AttrInertiaMod).get_double());
    m_agility /= 1000000.0;
    // not sure if this will be used here.  attr 559
    pMissile->GetSelf()->SetAttribute(AttrAgility, m_agility, false);

    m_stop = false;
    m_ballMode = Destiny::Ball::Mode::MISSILE;
    m_stateStamp = sEntityMgr.GetStamp();

    SystemEntity* pTarget = pMissile->GetTargetSE();
    m_targetPoint = GPoint(pTarget->GetPosition());
    m_targetEntity.first = pTarget->GetID();
    m_targetEntity.second = pTarget;

    GVector heading = m_targetPoint - m_position;
    m_targetDistance = heading.normalize();     //change vector to direction
    m_targetHeading = heading;

    SetUndockSpeed();   /* sets all needed variables for max velocity */
    mySE->SystemMgr()->AddEntity(pMissile, false); // we are not adding missiles to anomaly map

    std::vector<PyTuple*> updates;
    SetBallSpeed maxspeed;
        maxspeed.entityID = pMissile->GetID();
        maxspeed.speed = m_maxSpeed;
    updates.push_back(maxspeed.Encode());
    Rsp_LaunchMissile miss;
        miss.missileID = pMissile->GetID();
        miss.targetID = pTarget->GetID();
        miss.shipID = pMissile->GetLauncherID();
        miss.unk1 = 1;  // this is always "1" in packets.
        miss.unk2 = 1;  // this is always "1" in packets.
    updates.push_back(miss.Encode());
    SendDestinyUpdates(updates); //consumed
}

void DestinyManager::TractorBeamStart(SystemEntity* pShipSE, EvilNumber speed) {
    m_ballMode = Destiny::Ball::Mode::FOLLOW;

    m_stop = false;
    m_tractored = true;
    //m_moveTime = GetTimeMSeconds();
    m_stateStamp = sEntityMgr.GetStamp();

    m_targetPoint = GPoint(pShipSE->GetPosition());
    GVector heading = m_targetPoint - m_position;
    m_targetDistance = heading.normalize();
    m_targetHeading = heading;
    m_followDistance = 500;

    m_maxSpeed = speed.get_float();   // **TODO:  use AttrMaxTractorVelocity
    m_shipVelocity = m_targetHeading * m_maxSpeed;
    m_targetVelocity = m_shipVelocity;

    m_targetDistance = 500 + pShipSE->GetRadius();
    m_userSpeedFraction = 1.0f;
    m_activeSpeedFraction = 1.0f;

    m_targetEntity.first = pShipSE->GetID();
    m_targetEntity.second = pShipSE;

    std::vector<PyTuple*> updates;
    SetBallSpeed ms;
        ms.entityID = mySE->GetID();
        ms.speed = m_maxSpeed;
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
        fb.range = m_targetDistance;
    updates.push_back(fb.Encode());
    SendDestinyUpdates(updates); //consumed
}

void DestinyManager::TractorBeamStop() {
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
        sbmass.mass = mySE->GetSelf()->mass();
    updates.push_back(sbmass.Encode());
    SendDestinyUpdates(updates); //consumed
}

void DestinyManager::Jump(int32 fromGateID, bool showCloak/*true*/) {
    // set vars to show ship stopped but dont call Stop() or Halt() here.

    //m_ballMode = Destiny::Ball::Mode::STOP;
    m_shipVelocity = NULL_ORIGIN;
    m_userSpeedFraction = 0.0f;
    m_activeSpeedFraction = 0.0f;

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
    PyDecRef(up);
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
        effect.area = PyStatic.mtList();
        effect.guid = std::move(guid);
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
    PyTuple *up = effect.Encode();
    if (is_log_enabled(EFFECTS__DUMP))
        up->Dump(EFFECTS__DUMP, "");
    SendSingleDestinyEvent(&up);
}

// def OnSpecialFX(shipID, moduleID, moduleTypeID, targetID, otherTypeID, area, guid, isOffensive, start, active, duration = -1, repeat = None, startTime = None, graphicInfo = None):
// GFX method for graphics effects
void DestinyManager::SendGFX14(int32 entityID, int32 moduleID, int32 moduleTypeID, int32 targetID,
                               int32 chargeTypeID, std::string guid, bool isOffensive, bool start,
                               bool isActive, int32 duration, int32 repeat, int64 startTime/*0*/,
                               int32 graphicInfo/*0*/, Client* pClient/*nullptr*/) const
{
    OnSpecialFX14 effect;
        effect.entityID = entityID;
        effect.moduleID = moduleID;             // npc UID for npc's
        effect.moduleTypeID = moduleTypeID;     // npc typeID for npc's
        effect.targetID = (targetID == 0 ? PyStatic.NewNone() : new PyInt(targetID));
        effect.otherTypeID = (chargeTypeID == 0 ? PyStatic.NewNone() : new PyInt(chargeTypeID));
        effect.area = PyStatic.mtList();
        /** --- POPULATING THE AREA FIELD ---
        PyList* areaBounds = new PyList();
        // Element 0: Primitive Type (1.0 = Sphere, 2.0 = Cylinder, 3.0 = Cone, 4.0 = Box/Cube)
        areaBounds->AddItem(new PyFloat(1.0f));
        // Element 1: Spatial Radius Constraint (e.g., 50000.0f meters)
        areaBounds->AddItem(new PyFloat(effectRadius));
        effect.area = areaBounds;
        */
        effect.guid = std::move(guid);
        effect.isOffensive = isOffensive;       // bool
        effect.start = start;                   // int bool
        effect.active = isActive;               // int bool
        effect.duration = duration;             // in ms
        effect.repeat = repeat;
        effect.startTime = (startTime > 0 ? startTime : GetFileTimeNow());
        effect.graphicInfo = (graphicInfo == 0 ? PyStatic.NewNone() : new PyInt(graphicInfo));
    PyTuple *up = effect.Encode();
    if (mySE->IsNPCSE()) {
        if (is_log_enabled(EFFECTS__DUMP_NPC))
            up->Dump(EFFECTS__DUMP_NPC, "");
    } else if (is_log_enabled(EFFECTS__DUMP)) {
        up->Dump(EFFECTS__DUMP, "");
    }
    if (pClient == nullptr) {
        SendSingleDestinyEvent(&up);
    } else {
        // this is to update new ship in bubble with active gfx
        pClient->QueueDestinyEvent(&up);
    }
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
    SendGFX10(shipID, JumpEffect);
    /*
    sLog.Error("SendGFX", "SendJumpOutEffect - fix this");
    OnSpecialFX14 effect;
        effect.entityID = mySE->GetID();
        effect.targetID = new PyInt(shipID);
        effect.guid = "effects.JumpDriveOut";   // JumpDriveInBO
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 1;
        effect.duration = 5000;
        effect.repeat = 0;
        effect.startTime = GetFileTimeNow();
        effect.otherTypeID= 0;
        effect.graphicInfo = 0;
    PyTuple *up(effect.Encode());
    SendSingleDestinyUpdate(&up);
    */
}

void DestinyManager::SendJumpInEffect(std::string JumpEffect) const {
    SendGFX10(mySE->GetID(), JumpEffect);
    /*
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
    */
}

// only used by UpdateShip calls
void DestinyManager::SendBallInteractive(const ShipItemRef shipRef, bool set/*false*/) const {
    // interactive means "ship has pilot"
    SetBallInteractive sbi;
        sbi.entityID = shipRef->itemID();
        sbi.interactive = set;
    PyTuple* up = sbi.Encode();
    SendSingleDestinyUpdate(&up);
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
    SendSingleDestinyEvent(&up);
}

void DestinyManager::SendSetState() const {
    if (!mySE->HasPilot())
        return;

    if (is_log_enabled(DESTINY__MESSAGE))
        _log(DESTINY__MESSAGE, "DM::SendSetState() Called for Ship:%s(%u) Pilot:%s(%u)", \
                mySE->GetName(), mySE->GetID(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());

    SetState ss;
        ss.stamp = sEntityMgr.GetStamp();
        ss.ego = mySE->GetID();

    mySE->SystemMgr()->MakeSetState(mySE->SysBubble(), ss);
    PyTuple* tmp(ss.Encode());
    //setstate should be alone and immediate.  send directly
    mySE->GetPilot()->QueueDestinyUpdate(&tmp, false, true);   // consumed?
    mySE->GetPilot()->SetStateSent(true);
}

void DestinyManager::MarkPoint(const GPoint& position, std::string& name, std::string& desc, bool orbit/*false*/) {
    // create jetcan to visualize point in space
    ItemData idata(23, ownerSystem, mySE->GetLocationID(), flagAutoFit, name.c_str(), position, desc.c_str());
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
    if (orbit) {
        m_orbitMarkers.emplace(cRef->itemID(), cSE);
    } else {
        m_shipMarkers.emplace(cRef->itemID(), cSE);
    }
}

void DestinyManager::RemoveAllMarkers() {
    RemoveShipMarkers();
    RemoveOrbitMarkers();
}

void DestinyManager::RemoveShipMarkers() {
    SystemEntity* pSE = nullptr;
    for (auto &cur : m_shipMarkers) {
        pSE = cur.second;
        pSE->Delete();
        SafeDelete(pSE);
    }
    m_shipMarkers.clear();
}

void DestinyManager::RemoveOrbitMarkers() {
    SystemEntity* pSE = nullptr;
    for (auto &cur : m_orbitMarkers) {
        pSE = cur.second;
        pSE->Delete();
        SafeDelete(pSE);
    }
    m_orbitMarkers.clear();
}

void DestinyManager::AddOrbitMarkers(float distance, GVector& targetCenter) {
    //begin marker placement
    sLog.Yellow("Spatial Debug", "centerDistance: %0.2fm", distance);
    sLog.Yellow("Spatial Debug", "m_position: %0.2f, %0.2f, %0.2f", \
            m_position.x, m_position.y, m_position.z);
    sLog.Yellow("Spatial Debug", "m_shipVelocity: %.2f, %.2f, %.2f", \
            m_shipVelocity.x, m_shipVelocity.y, m_shipVelocity.z);
    sLog.Yellow("Spatial Debug", "targetCenter: %0.2f, %0.2f, %0.2f", \
            targetCenter.x, targetCenter.y, targetCenter.z);
    sLog.Yellow("Spatial Debug", "m_orbitNormal: %0.2f, %0.2f, %0.2f", \
            m_orbitNormal.x, m_orbitNormal.y, m_orbitNormal.z);
    sLog.Yellow("Spatial Debug", "m_orbitBasisZ: %0.2f, %0.2f, %0.2f", \
            m_orbitBasisZ.x, m_orbitBasisZ.y, m_orbitBasisZ.z);
    sLog.Yellow("Spatial Debug", "m_orbitBasisX: %0.2f, %0.2f, %0.2f", \
            m_orbitBasisX.x, m_orbitBasisX.y, m_orbitBasisX.z);

    std::string name = " ", desc = "Spatial Marker";

    // create jetcan to mark +basisU
    GPoint center = targetCenter;
    center += m_orbitBasisZ * 5000.0f;
    sLog.Yellow("Spatial Debug", "+basisU: %0.2f, %0.2f, %0.2f", \
            center.x, center.y, center.z);
    name.clear();
    name = "+BasisU";
    MarkPoint(center, name, desc, true);

    // create jetcan to mark -basisU
    center = targetCenter;
    center -= m_orbitBasisZ * 5000.0f;
    sLog.Yellow("Spatial Debug", "-basisU: %0.2f, %0.2f, %0.2f", \
            center.x, center.y, center.z);
    name.clear();
    name = "-BasisU";
    MarkPoint(center, name, desc, true);

    // create jetcan to mark +normal
    center = targetCenter;
    center += m_orbitNormal * 5000.0f;
    sLog.Yellow("Spatial Debug", "+normal: %0.2f, %0.2f, %0.2f", \
            center.x, center.y, center.z);
    name.clear();
    name = "+Normal";
    MarkPoint(center, name, desc, true);

    // create jetcan to mark -normal
    center = targetCenter;
    center -= m_orbitNormal * 5000.0f;
    sLog.Yellow("Spatial Debug", "-normal: %0.2f, %0.2f, %0.2f", \
            center.x, center.y, center.z);
    name.clear();
    name = "-Normal";
    MarkPoint(center, name, desc, true);

    // create jetcan to mark +basisV
    center = targetCenter;
    center += m_orbitBasisX * 5000.0f;
    sLog.Yellow("Spatial Debug", "+basisV: %0.2f, %0.2f, %0.2f", \
            center.x, center.y, center.z);
    name.clear();
    name = "+BasisV";
    MarkPoint(center, name, desc, true);

    // create jetcan to mark -basisV
    center = targetCenter;
    center -= m_orbitBasisX * 5000.0f;
    sLog.Yellow("Spatial Debug", "-basisV: %0.2f, %0.2f, %0.2f", \
            center.x, center.y, center.z);
    name.clear();
    name = "-BasisV";
    MarkPoint(center, name, desc, true);
    // end marker placement
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
                    _log( DESTINY__UPDATES, "[%u] DM::SendUpdates() - BubbleCasting %lu DestinyUpdates as Self-Only to bubbleID %u from %s(%u)", \
                            sEntityMgr.GetStamp(), updates.size(), mySE->SysBubble()->GetID(), mySE->GetName(), mySE->GetID() );
                mySE->SysBubble()->BubblecastDestinyUpdate(updates, "DestinyUpdates");
            } else {
                for (auto &cur : updates)
                    PySafeDecRef(cur);
            }
            return;
        }
        if (is_log_enabled(PLAYER__MESSAGE))
            _log(PLAYER__MESSAGE, "[%u] DM::SendUpdates() called as 'self_only' for %s(%i)", \
                    sEntityMgr.GetStamp(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());

        for (std::vector<PyTuple*>::iterator itr = updates.begin(); itr != updates.end(); itr++) {
            PyIncRef(*itr);
            mySE->GetPilot()->QueueDestinyUpdate(&(*itr));
        }
    } else if (mySE->IsOperSE()) { //These are global entities, so we have to send update to all bubbles in a system
        if (is_log_enabled(DESTINY__UPDATES))
            _log(DESTINY__UPDATES, "[%u] DM::SendUpdates() - BubbleCasting Structure DestinyUpdates in %s from %s(%u)", \
                    sEntityMgr.GetStamp(), mySE->SystemMgr()->GetName(), mySE->GetName(), mySE->GetID());

        //Get all clients in our system
        // should this be bubblecast?  which would be faster?
        std::vector<Client*> cv;
        mySE->SystemMgr()->GetClientList(cv);
        for (auto const& player : cv)
            if (player->IsInSpace())
                player->QueueDestinyUpdates(updates);
    } else if (mySE->SysBubble() != nullptr) {
        if (is_log_enabled(DESTINY__UPDATES))
            _log(DESTINY__UPDATES, "[%u] DM::SendUpdates() - BubbleCasting %lu DestinyUpdates to bubbleID %u from %s(%u)", \
                    sEntityMgr.GetStamp(), updates.size(), mySE->SysBubble()->GetID(),   \
                    (mySE->HasPilot()?mySE->GetPilot()->GetName():mySE->GetName()), \
                    (mySE->HasPilot()?mySE->GetPilot()->GetCharID():mySE->GetID()) );
            mySE->SysBubble()->BubblecastDestinyUpdate(updates, "DestinyUpdates");
    } else {
        _log(DESTINY__WARNING, "[%u] DM::SendUpdates() - Cannot BubbleCast %lu DestinyUpdates; entity (%u) is not in any bubble. (mySE->SysBubble() == nullptr)", \
                sEntityMgr.GetStamp(), updates.size(), mySE->GetID() );
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
                    _log( DESTINY__UPDATES, "[%u] DM::SingleEvent() - Self-Only to bubbleID %u from %s(%u)", \
                            sEntityMgr.GetStamp(), mySE->SysBubble()->GetID(), mySE->GetName(), mySE->GetID() );
                mySE->SysBubble()->BubblecastDestinyEvent(ev, "DestinyEvent");
            }
            return;
        }
        if (is_log_enabled(PLAYER__MESSAGE))
            _log(PLAYER__MESSAGE, "[%u] DM::SingleEvent() - Self-Only' for %s(%i)", \
                sEntityMgr.GetStamp(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());

        mySE->GetPilot()->QueueDestinyEvent(ev);
    } else if (mySE->IsOperSE()) { //These are global entities, so we have to send update to all players in a system
        if (is_log_enabled(DESTINY__UPDATES))
            _log(DESTINY__UPDATES, "[%u] DM::SingleEvent() - BubbleCasting Structure DestinyEvent in %s from %s(%u)", \
            sEntityMgr.GetStamp(), mySE->SystemMgr()->GetName(), mySE->GetName(), mySE->GetID());

        //Get all clients in our system
        std::vector<Client*> cv;
        mySE->SystemMgr()->GetClientList(cv);
        for (auto const& player : cv)
            if (player->IsInSpace()) {
                PyIncRef(*ev);
                player->QueueDestinyEvent(ev);
            }
    } else if (mySE->SysBubble() != nullptr) {
        if (is_log_enabled(DESTINY__UPDATES))
            _log(DESTINY__UPDATES, "[%u] DM::SingleEvent() to bubbleID %u from %s(%u)", \
                    sEntityMgr.GetStamp(), mySE->SysBubble()->GetID(),   \
                    (mySE->HasPilot()?mySE->GetPilot()->GetName():mySE->GetName()),\
                    (mySE->HasPilot()?mySE->GetPilot()->GetCharID():mySE->GetID()) );
        mySE->SysBubble()->BubblecastDestinyEvent(ev, "Single DestinyEvent" );
    } else {
        _log(DESTINY__WARNING, "[%u] DM::SingleEvent() - Cannot BubbleCast: entity %s(%u) is not in any bubble. (mySE->SysBubble() == nullptr)", \
                sEntityMgr.GetStamp(), mySE->GetName(), mySE->GetID() );
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
                    _log( DESTINY__UPDATES, "[%u] DM::SingleUpdate() Self-Only to bubbleID %u from %s(%u)", \
                            sEntityMgr.GetStamp(), mySE->SysBubble()->GetID(), mySE->GetName(), mySE->GetID() );
                mySE->SysBubble()->BubblecastDestinyUpdate(up, "Single Self-Only DestinyUpdate");
            }
            return;
        }
        if (is_log_enabled(PLAYER__MESSAGE))
            _log(PLAYER__MESSAGE, "[%u] DM::SingleUpdate() Self-Only' for %s(%i)", \
                    sEntityMgr.GetStamp(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());

        mySE->GetPilot()->QueueDestinyUpdate(up);
    } else if (mySE->IsOperSE()) { //These are global entities, so we have to send update to all players in a system
        if (is_log_enabled(DESTINY__UPDATES))
            _log(DESTINY__UPDATES, "[%u] DM::SingleUpdate() - BubbleCasting Structure DestinyUpdate in %s from %s(%u)", \
                    sEntityMgr.GetStamp(), mySE->SystemMgr()->GetName(), mySE->GetName(), mySE->GetID());

        //Get all clients in our system
        std::vector<Client*> cv;
        mySE->SystemMgr()->GetClientList(cv);
        for (auto const& player : cv)
            if (player->IsInSpace()) {
                //PyIncRef(*up);
                player->QueueDestinyUpdate(up);
            }
    } else if (mySE->SysBubble() != nullptr) {
        if (is_log_enabled(DESTINY__UPDATES))
            _log(DESTINY__UPDATES, "[%u] DM::SingleUpdate() to bubbleID %u from %s(%u)", \
                    sEntityMgr.GetStamp(), mySE->SysBubble()->GetID(),   \
                    (mySE->HasPilot()?mySE->GetPilot()->GetName():mySE->GetName()),\
                    (mySE->HasPilot()?mySE->GetPilot()->GetCharID():mySE->GetID()) );
        mySE->SysBubble()->BubblecastDestinyUpdate(up, "Single DestinyUpdate" );
    } else {
        _log(DESTINY__WARNING, "[%u] DM::SingleUpdate() - Cannot BubbleCast: entity %s(%u) is not in any bubble. (mySE->SysBubble() == nullptr)", \
        sEntityMgr.GetStamp(), mySE->GetName(), mySE->GetID() );
        if (sConfig.debug.IsTestServer)
            EvE::traceStack();
    }
}

// move this to sDataMgr?
const char* DestinyManager::GetModeName(uint8 mode) {
    switch (mode) {
        case 1: return "Follow"; break;
        case 2: return "Stop"; break;
        case 3: return "Warp"; break;
        case 4: return "Orbit"; break;
        case 5: return "Missile"; break;
        case 6: return "Mushroom"; break;
        case 7: return "Boid"; break;
        case 8: return "Troll"; break;
        case 9: return "Miniball"; break;
        case 10: return "Field"; break;
        case 11: return "Rigid"; break;
        case 12: return "Formation"; break;
    }

	return "Invalid";
}

std::string DestinyManager::GetModeNameString() {
    std::string modeStr = "Invalid";
    switch (m_ballMode) {
        case 0: modeStr = "Goto"; break;
        case 1: modeStr = "Follow"; break;
        case 2: modeStr = "Stop"; break;
        case 3: modeStr = "Warp"; break;
        case 4: modeStr = "Orbit"; break;
        case 5: modeStr = "Missile"; break;
        case 6: modeStr = "Mushroom"; break;
        case 7: modeStr = "Boid"; break;
        case 8: modeStr = "Troll"; break;
        case 9: modeStr = "Miniball"; break;
        case 10: modeStr = "Field"; break;
        case 11: modeStr = "Rigid"; break;
        case 12: modeStr = "Formation"; break;
    }

    return modeStr;
}

/*
    using namespace Destiny::Ball::Mode;
    switch (m_ballMode) {
        case GOTO:
        case FOLLOW:
        case STOP:
        case WARP:
        case ORBIT:
        case MISSILE:
        case MUSHROOM:
        case BOID:
        case TROLL:
        case MINIBALL:
        case FIELD:
        case RIGID:
        case FORMATION: {
        } break;
    }
*/