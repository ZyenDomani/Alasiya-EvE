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
m_ticStamp(0), m_activeSpeedFraction(0.0f), m_userSpeedFraction(0.0f),
m_targetDistance(0), m_followDistance(10), m_position(self->GetPosition()), m_targetPoint(NULL_ORIGIN),
m_targetHeading(NULL_ORIGIN), m_moveDelay(false), m_skipTic(true),
m_autoPilot(false), m_alignTo(false), m_agility(0.0f), m_posHack(sConfig.debug.PositionHack), m_shipVelocity(NULL_ORIGIN),
m_timeStamp(0.0), m_targetVelocity(NULL_ORIGIN), m_timeFactor(0.0), m_expTerm(0.0), m_posScale(0.0),
m_accelTime(0.0f), m_accelDistance(0), m_decelTime(0.0f),m_warpState(nullptr)
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
     *  m_position current ship/item position
     *   -> set in MoveObject() and Warp()
     *  m_userSpeedFraction  current sf set by player/client
     *  m_activeSpeedFraction  keeping for logs
     *  m_maxSpeed is maximum speed based on user input and current configuration.
     *   -> measured in m/s
     *   -> initially set by UpdateShipVariables() based on ship configuration and pilot skills
     *   -> reset by SpeedBoost() for prop mods
     *   -> sent to client as ship max speed
     *  m_ticStamp is server tic when move event began
     *   -> measured in seconds
     *   -> data type is int32
     *   -> set in BeginMovement() for all velocity changes
     *  m_timeStamp  is timestamp of when current mode began
     *   -> measured in fileTime (100 ns resolution in filetime format)
     *   -> data type is int64
     *   -> set in BeginMovement() for all velocity changes
     *  m_followDistance requested distance between self and target
     *   -> set in OrbitBall(), Follow(), WarpTo()
     *  m_shipVelocity is object's current derived vector.
     *   -> set in MoveObject(), Warp(), Undock() and tractor
     *   -> m_shipVelocity = m_targetHeading * speed * m_userSpeedFraction
     *   -> this is the variable used for position tracking.
     *  m_targetPoint holds current target coords.
     *   -> initially set by goto, warp, align, follow, orbit
     *   -> reset in follow and orbit for each tic to correctly align with moving ship/target
     *  m_targetDistance as stated
     *   -> initially set in all Move calls
     *   -> recalculated each tic using m_targetPoint and m_position
     *  m_targetVelocity is the derived vector velocity
     *   -> used to blend with ship's velocity, position and speed for vector changes
     *   -> set/updated in Follow, Orbit, Goto, Stop
     *  m_targetHeading holds desired direction
     *   -> used to blend with ship's heading for vector changes
     */

// vector = (to - from)

// this is called once per tic by SystemEntity::Process() via SystemManager::Process()
void DestinyManager::Process() {
    if (m_skipTic)
        return;
    if (m_moveDelay) {
        m_moveDelay = false;
        return;
    }

    double profileStartTime = GetTimeUSeconds();
    using namespace Destiny::Ball::Mode;
    switch(m_ballMode) {
        case RIGID:
        case MISSILE: {
            // NOTE:  this should not hit.
            _log(DESTINY__ERROR, "Destiny::Process() Error! - %s(%u) is proc with state=%s.", \
                    mySE->GetName(), mySE->GetID(), GetModeNameString().c_str());
            return; // Zero server cycles on static/visual assets
        } break;
        case STOP: {
            // Apply vertical leveling pitch dampener directly to velocity
            m_shipVelocity.y *= 0.8691588785046729;
        } break;
        case GOTO: {
            // there's not necessarily a target here...just a direction or point in space.
            if (m_alignTo and IsAligned(m_targetHeading)) {
                if (is_log_enabled(DESTINY__MOVE_TRACE)) {
                    sLog.Warning("DestinyManager::Process(Goto)", "%s is aligned.", mySE->GetName());
                    Stop();
                    break;
                }
            }

            if (mySE->HasPilot() and mySE->GetSelf()->GetShipItem()->IsUndocking())
                break;

            GotoVelocity(); // Includes exponential homing zone
        } break;
        case FOLLOW: {
            if (!ValidTarget()) {
                if (is_log_enabled(DESTINY__MOVE_TRACE))
                    sLog.Warning("DestinyManager::Process(Follow)", "%s's target is invalid", mySE->GetName());
                Stop();
                break;
            }

            CalculateFollowPoint();
            GotoVelocity();
        } break;
        case FORMATION: {
            if (!ValidTarget()) {
                Stop();
                return;
            }
            CalculateFormationPoint();
            GotoVelocity();
        } break;
        case ORBIT: {
            if (!ValidTarget()) {
                Stop();
                return;
            }

            OrbitVelocity();
        } break;
        case WARP: {
            if (m_warpState != nullptr) {
                //warp is in progress
                uint16 sec_into_warp = (mySE->SystemMgr()->GetTicCount() - m_ticStamp);
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
            //TODO:  put check here for stuck ship where (tic - stamp > stucktime)

            GotoVelocity();
        } break;
        case TROLL: {
            // mobile object that will become rigid after a set time
            // jetcans/wrecks created from moving ship will take that velocity and continue movement for a time, then become rigid.
            //update.  ball is set <state_stop> and <mode_troll> with timestamp.
            //  check elapsed time here while object is moving, then switch to <mode_rigid> once stopped.
            // this elapsed check can/may change based on load in current bubble
            // 3. client uses 30-second window (Matches Ballpark Offset 0x7c)  may reduce this to 15-20s
            uint32 elapsed = mySE->SystemMgr()->GetTicCount() - m_ticStamp;
            if ((elapsed >= sConfig.rates.DestinyTrollTime) or (m_shipVelocity.Length() < 0.05)) {
                Halt();
                // Mutate state locally to a static rigid item...no, leave at stop to allow tractor
                m_ballMode = Destiny::Ball::Mode::STOP;
                m_skipTic = true;
                // may not want to do this...will remove movement from tractor
                //mySE->SystemMgr()->RemoveTicEntity(mySE);
                // client will automagically update <troll> to <rigid> after timer expires
                return;
            }

            GotoVelocity();
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

    // integration step using optimized blending glide model
    Integrate();

    // Global boundary parking check
    double speed = m_shipVelocity.Length();
    if (m_stop and (speed < 0.05f)) {
        Halt();
        return;
    }

    if (m_maxSpeed > 0.01f)
        m_activeSpeedFraction = speed / m_maxSpeed;

    // only create tracking can when ship is moving significant amount
    if (sEntityMgr.GetTracking())
        CreateShipMarker();

    if (is_log_enabled(DESTINY__MOVE_TRACE)) {
        int32 timeDelta = mySE->SystemMgr()->GetTicCount() - m_ticStamp;
        _log(DESTINY__MOVE_TRACE, "Destiny::%s(%u) - %s(%i): Current Ship Velocity: %0.2f, %0.2f, %0.2f @ asf: %0.3f (speed: %0.1f)", \
                GetModeNameString().c_str(), timeDelta, mySE->GetName(), mySE->GetID(), \
                m_shipVelocity.x, m_shipVelocity.y, m_shipVelocity.z, \
                m_activeSpeedFraction, m_shipVelocity.Length());
        _log(DESTINY__MOVE_TRACE, "Current position: %0.2f, %0.2f, %0.2f", \
                m_position.x, m_position.y, m_position.z);
        if (m_targetPoint.notZero())
            _log(DESTINY__MOVE_TRACE, "Ship target Point: %0.2f, %0.2f, %0.2f", \
                    m_targetPoint.x, m_targetPoint.y, m_targetPoint.z);
        if (m_targetVelocity.notZero())
            _log(DESTINY__MOVE_TRACE, "Ship target Velocity: %0.2f, %0.2f, %0.2f", \
                    m_targetVelocity.x, m_targetVelocity.y, m_targetVelocity.z);
    }

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::destiny, GetTimeUSeconds() - profileStartTime);
}

//Velocity setting method
void DestinyManager::SetSpeedFraction(float fraction/*1.0f*/) {
    if (is_log_enabled(DESTINY__MOVE_TRACE))
        _log(DESTINY__MOVE_TRACE, "Destiny::SetSpeedFraction() - %s(%u): from %0.2f to %0.2f.", \
                mySE->GetName(), mySE->GetID(), m_userSpeedFraction, fraction);

    m_userSpeedFraction = fraction;

    if (fraction and (mySE->HasPilot() or mySE->SysBubble()->HasPlayers())) {
        std::vector<PyTuple*> updates;
        CmdSetSpeedFraction du;
            du.entityID = mySE->GetID();
            du.fraction = fraction;
        updates.push_back(du.Encode());

        if (mySE->IsNPCSE() or mySE->IsMissileSE() or mySE->IsContainerSE() or mySE->IsWreckSE()) {
            SetBallSpeed ms;
                ms.entityID = mySE->GetID();
                ms.speed = m_maxSpeed;
            updates.push_back(ms.Encode());
            m_hasSentShipUpdates = true;
        }

        if (!updates.empty())
            SendDestinyUpdates(updates); //consumed
    }
}

//Global Actions:
void DestinyManager::Stop() {
// is this right??
    if (mySE->HasPilot() and mySE->GetSelf()->GetShipItem()->IsUndocking())
        return;

    if (is_log_enabled(DESTINY__MOVE_TRACE))
        sLog.Warning("DestinyManager", "%s calling stop", mySE->GetName());

    // set marker for calc'd stop distance (testing)
    if (is_log_enabled(DESTINY__WARP_DEBUG)) {
        uint16 dist = m_maxSpeed * m_activeSpeedFraction * m_agility;
        Vector3d offset = (m_position * dist);
        Vector3d marker = (m_position + offset);
        std::string str = "Stop Point - ";
        str += mySE->GetName();
        MarkPoint(marker, str, str);
    }

    m_stop = true;
    m_alignTo = false;
    m_posHack = false;
    m_autoPilot = false;

    m_targBubble = nullptr;

    m_ticStamp = mySE->SystemMgr()->GetTicCount();

    m_targetVelocity = NULL_ORIGIN;

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
    m_ticStamp = 0;
    m_targetDistance = 0;
    m_userSpeedFraction = 0.0f;

    m_targetPoint = NULL_ORIGIN;
    m_shipVelocity = NULL_ORIGIN;
    m_targetHeading = NULL_ORIGIN;
    m_targetVelocity = NULL_ORIGIN;

    // do we really wanna do this here?
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

// Unified Analytical Solution to the Equation of Motion under Linear Viscous Drag
void DestinyManager::Integrate() {
	// UNIVERSAL HIGH-PERFORMANCE TWO-LINE BLENDER
	// Fully unrolled to enforce strict sequential IEEE-754 rounding Parity

	// 1. Capture spatial velocity deltas into fast local registers
	const double dvx = m_shipVelocity.x - m_targetVelocity.x;
	const double dvy = m_shipVelocity.y - m_targetVelocity.y;
	const double dvz = m_shipVelocity.z - m_targetVelocity.z;

	// 2. Compute the exact new coordinate translations
	m_position.x += m_targetVelocity.x + (dvx * m_posScale);
	m_position.y += m_targetVelocity.y + (dvy * m_posScale);
	m_position.z += m_targetVelocity.z + (dvz * m_posScale);

	// 3. Glide the real ship velocity smoothly toward target terminal velocity
	m_shipVelocity.x = m_targetVelocity.x + (dvx * m_expTerm);
	m_shipVelocity.y = m_targetVelocity.y + (dvy * m_expTerm);
	m_shipVelocity.z = m_targetVelocity.z + (dvz * m_expTerm);

	// 4. Update the spatial ecosystem registry state
	mySE->SetPosition(m_position);
}

void DestinyManager::CalculateFollowPoint() {
    // should this check for moving/static objects?
    //if (m_targetEntity.second->IsStaticEntity())
    //    return;
    // 1. Extract the target entity's master position
    Vector3d targPos = m_targetEntity.second->GetPosition();
    Vector3d delta = targPos - m_position;
    double dist = delta.Length();

    if (dist == 0) {
        // go out in some arbitrary direction
        m_targetPoint.x = targPos.x + m_followDistance;
        m_targetPoint.y = targPos.y;
        m_targetPoint.z = targPos.z;
    } else {
        // target point in range along our vector
        double newDist = m_followDistance / dist;
        m_targetPoint.x = targPos.x + (delta.x * newDist);
        m_targetPoint.y = targPos.y + (delta.y * newDist);
        m_targetPoint.z = targPos.z + (delta.z * newDist);
    }
}

void DestinyManager::CalculateFormationPoint() {
    // for formations
    SystemEntity* leader = m_targetEntity.second;

    /*
     *    // 1. Validate formation arrays
     *    int formationID = leader->GetFormationID();
     *    if (formationID < 0 || formationID >= static_cast<int>(m_formations.size())) {
     *        m_targetPoint = m_position;
     *        return;
}
// 2. Extract designated slot index from mEffectStamp network footprint
size_t slot = static_cast<size_t>(mySE->GetEffectStamp());
if (slot >= m_formations[formationID].size()) {
    m_targetPoint = m_position;
    return;
}
*/
    // 3. Extract raw configured offset layout vector
    Vector3d offset = NULL_ORIGIN; //m_formations[formationID][slot];
    double length = offset.Length();

    if (length <= 1e-12) {
        length = 1.0;
    }

    // 4. Scale local offset vector adding physical radius padding
    double totalScaleFactor = (mySE->GetRadius() + leader->GetRadius() + length) / length;
    Vector3d localOffset = offset * totalScaleFactor;

    // =========================================================================
    // 5. THE ROTATION PLUG-IN POINT
    // Transpose the local layout to match the leader's actual direction of flight
    // =========================================================================
    Vector3d worldOffset = RotateVectorByEntityOrientation(localOffset, leader);

    // 6. Anchor the final server target point relative to the leader
    m_targetPoint = leader->GetPosition() + worldOffset;
}

void DestinyManager::OrbitVelocity() {
    // 1. Establish the azimuth angle
    const double phi1 = (double)mySE->SystemMgr()->GetTicCount() * ORBITAL_PRECESSION;
    // elevation angle
    const double phi2 = (double)(mySE->GetID() & 0x0000FFFF) + phi1;

    // 2. Generate the base unit orientation vector
    Vector3d radialVector(
        std::cos(phi1) * std::cos(phi2),
        std::sin(phi2),
        std::sin(phi1) * std::cos(phi2)
    );

    // shitround the components to 7 decimal digits
    radialVector.x =  (double)((int64_t)(radialVector.x * 1e7)) / 1e-7;
    radialVector.y =  (double)((int64_t)(radialVector.y * 1e7)) / 1e-7;
    radialVector.z =  (double)((int64_t)(radialVector.z * 1e7)) / 1e-7;

    // 3. Compute relative range vector
    Vector3d toVector = m_targetEntity.second->GetPosition() - m_position;
    radialVector.Cross(toVector);
    radialVector.Normalize();
    const double dist = toVector.Length();
    if (dist < 1e-6)
        return; // Safety boundary clamp

    toVector.Normalize();

    // 4. Calculate the tangent vs approach weighting vectors
    const double targetDistSq = m_targetDistance * m_targetDistance;
    const double toComp = (dist * dist) - targetDistSq;

    if (toComp >= 0.0) {
        const double radComp = m_targetDistance * std::sqrt(toComp) / dist;
        const double toScale = toComp / dist;

        toVector = (toScale * toVector) + (radComp * radialVector);
        toVector.Normalize();
    }

    // 5. Apply the official exponential inward-pull curve
    const double delta        = m_targetDistance - dist;
    const double radialFactor = std::exp(-(delta * delta) / 40000.0);

    // 6. Execute the law of cosines angle intersection dot product
    const double dotProduct = -toVector * radialVector;

    // 7. Solve the quadratic determinant for the transverse tracking force
    double transverseFactor = 1.0 + (radialFactor * radialFactor) * ((dotProduct * dotProduct) - 1.0);
    if (transverseFactor > 0.0) {
        transverseFactor = (radialFactor * dotProduct) + std::sqrt(transverseFactor);
    } else {
        transverseFactor = radialFactor * dotProduct;
    }

    // 8. Distribute directional signs based on range deficit
    const double signum = (dist > m_targetDistance) ? 1.0 : ((dist < m_targetDistance) ? -1.0 : 0.0);
    transverseFactor *= signum;

    // 9. THE BRIDGE: Scale the final direction components directly into terminal velocity space (m/s)
    const double maxVelStep = m_userSpeedFraction * m_maxSpeed;
    m_targetVelocity.x = (radialVector.x * radialFactor + toVector.x * transverseFactor) * maxVelStep;
    m_targetVelocity.y = (radialVector.y * radialFactor + toVector.y * transverseFactor) * maxVelStep;
    m_targetVelocity.z = (radialVector.z * radialFactor + toVector.z * transverseFactor) * maxVelStep;
}

void DestinyManager::GotoVelocity() {
    // 1. Calculate the relative displacement heading vector
    m_targetHeading = m_targetPoint - m_position;
    double length = m_targetHeading.Length();

    // Safety fallback: if we are already precisely stacked on target, kill velocity
    if (length < 1e-6) {
        m_targetVelocity = NULL_ORIGIN;
        return;
    }

    m_targetHeading.Normalize();

    // maxVelStep defines the absolute maximum terminal velocity magnitude for this tick
    double maxVelStep = m_userSpeedFraction * m_maxSpeed;

    if (length < maxVelStep) {
        // --- BRAKING ZONE: Apply quartic damping curve near the target ---
        // coff = (Distance) / (MaxStep)
        double coff = length / maxVelStep;
        double dynamicVelocityCap = maxVelStep * (coff * coff); // Reduced math footprint

        m_targetVelocity.x = m_targetHeading.x * dynamicVelocityCap;
        m_targetVelocity.y = m_targetHeading.y * dynamicVelocityCap;
        m_targetVelocity.z = m_targetHeading.z * dynamicVelocityCap;
    } else {
        // --- MAX THRUST ZONE: Deep space continuous acceleration capacity ---
        // Natively pre-scaled: No divisions, no multi-million variables. Pure SIMD friendly.
        m_targetVelocity.x = m_targetHeading.x * maxVelStep;
        m_targetVelocity.y = m_targetHeading.y * maxVelStep;
        m_targetVelocity.z = m_targetHeading.z * maxVelStep;
    }
}

Vector3d DestinyManager::RotateVectorByEntityOrientation(const Vector3d& localVec, SystemEntity* leader) {
    // for formations
    Vector3d forward = leader->GetVelocity();
    double speed = forward.Length();

    // Fallback: If the leader is stationary, their forward velocity collapses.
    // In this state, we fallback to a standard unit heading to prevent matrix collapse.
    if (speed < 1e-6) {
        // Assume resting heading is pointing down the universal Z-axis
        forward = Vector3d(0.0, 0.0, 1.0);
    } else {
        forward.Normalize(); // Isolate pure directional unit vector
    }

    // 1. Compute local "Right" vector (+X axis) using a standard sky fallback (+Y)
    Vector3d right(0.0, 1.0, 0.0);

    // Safety check: If the ship is flying completely straight up or down,
    // the cross-product collapses. Shift the fallback axis to break the singularity.
    if (std::abs(forward.dotProduct(right)) > 0.99) {
        right = Vector3d(1.0, 0.0, 0.0);
    }

    // Right vector = upFallback X forward (Pure Right-Handed Cartesian orthogonal projection)
    right.Cross(forward);
    right.Normalize();

    // 2. Recompute accurate "Up" vector (+Y axis) to form a true orthonormal basis
    // Up vector = forward X right
    Vector3d up = forward;
    up.Cross(right);
    up.Normalize();

    // 3. Transform local coordinate array to absolute world space
    // Local X scales out along Right, Local Y along Up, and Local Z along Forward
    Vector3d worldVec(
                    (right.x * localVec.x) + (up.x * localVec.y) + (forward.x * localVec.z),
                    (right.y * localVec.x) + (up.y * localVec.y) + (forward.y * localVec.z),
                    (right.z * localVec.x) + (up.z * localVec.y) + (forward.z * localVec.z)
    );

    return worldVec;
}

void DestinyManager::Eject() {
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
     *         NOTE:  while this may be accurate now, it was not in crucible
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
    float decelTime = 0.0f, cruiseTime = 0.0f;
    int64 accelDistance = 0, decelDistance = 0, cruiseDistance = 0;
    int64 warpSpeedInMeters = m_shipWarpSpeed * ONE_AU_IN_METERS;
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
    float speed = 0.0f;
    bool run = true;
    uint8 step = 0;
    while (run) {
        speed = std::exp(--decelTime);
        ++step;
        if (speed < m_speedToLeaveWarp) {
            run = false;
        }
    }

    //decelTime = 0.0f;
    double distance = 0.0;
    while (step > 0) {
        distance += std::exp(decelTime++);
        --step;
    }

    // hack to increase stop distance and *hopefully* make warp stop smoother
    distance += m_speedToLeaveWarp;
    //m_targetDistance -= (decelDistance - distance);
    //m_decelTime = decelTime;
    decelDistance = distance;

    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        _log(DESTINY__WARP_TRACE, "Destiny::InitWarp():Calculate - %s(%u): Warp will accelerate for %0.1fs, cruise for %0.1fs, then decelerate for %0.1fs, with total time of %0.1fs and warp speed of %lli m/s.", \
                mySE->GetName(), mySE->GetID(), m_accelTime, cruiseTime, decelTime, warpTime, warpSpeedInMeters);
        _log(DESTINY__WARP_TRACE, "Destiny::InitWarp():Calculate - Accel distance is %lli  Cruise distance is %lli   Decel distance is %lli", \
                accelDistance, cruiseDistance, decelDistance);
        _log(DESTINY__WARP_TRACE, "Destiny::InitWarp():Calculate - We will exit warp at %0.1f,%0.1f,%0.1f at a distance of %lli AU (%lli m).", \
                m_targetPoint.x, m_targetPoint.y, m_targetPoint.z, m_targetDistance / ONE_AU_IN_METERS, m_targetDistance);
    }

    float intAccel = 0;
    float accelFraction = std::modf(m_accelTime, &intAccel);

    m_warpState = new WarpState(m_ticStamp, m_targetDistance, warpSpeedInMeters, accelDistance, cruiseDistance,
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

    m_targetEntity.first = 0;
    m_targetEntity.second = nullptr;

    // send warp gfx
    if (mySE->SysBubble()->HasPlayers())
        SendGFX10(mySE->GetID(), "effects.Warping");

    //TODO:  determine if this ship has assigned drones in space and call drone:shipwarping

    // reset move times
    m_accelDistance = 0;
    m_ticStamp = mySE->SystemMgr()->GetTicCount();
    m_activeSpeedFraction = 1.0f;
    
    // ships completely stop before warp
    m_shipVelocity = NULL_ORIGIN;

    WarpAccel(0);
}

void DestinyManager::WarpAccel(uint16 sec_into_warp) {
    /* For acceleration, k = 3.
     * distance = e^(k*s)
     * speed = k*e^(k*s)
     */
    float accelTime = (sec_into_warp + m_warpState->accelFraction);
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
            Vector3d update;
            update.x = (m_targetHeading.x * m_targetDistance);
            update.y = (m_targetHeading.y * m_targetDistance);
            update.z = (m_targetHeading.z * m_targetDistance);
            m_position.x = m_targetPoint.x - update.x;
            m_position.y = m_targetPoint.y - update.y;
            m_position.z = m_targetPoint.z - update.z;
            mySE->SetPosition(m_position);
        }
    }

    m_accelDistance += currentDistance;
    WarpUpdate(currentDistance, sec_into_warp, 1);

    if (mySE->SysBubble() != nullptr) {
        //if (currentDistance > BUBBLE_RADIUS_METERS) { // this will not account for warping from one side of bubble to other
        if (!mySE->SysBubble()->InBubble(m_position, true)) {  // check actual bubble center here
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
        Vector3d update;
        update.x = (m_targetHeading.x * m_targetDistance);
        update.y = (m_targetHeading.y * m_targetDistance);
        update.z = (m_targetHeading.z * m_targetDistance);
        m_position.x = m_targetPoint.x - update.x;
        m_position.y = m_targetPoint.y - update.y;
        m_position.z = m_targetPoint.z - update.z;
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
    //double decelTime = --m_decelTime;
    int64 currentShipSpeed = std::exp(--m_decelTime);

    WarpUpdate(currentShipSpeed, sec_into_warp, 3);

    // how will this work for edge-case where dest is 5k inside bubble?
    if (mySE->SysBubble() == nullptr) {
        //if (m_targBubble->InBubble(m_position, true)) {   // check actual bubble center here
        // testing remaining distance instead of calling Length() on every decel tic
        if (m_targetDistance < (BUBBLE_RADIUS_METERS + 100000)) {
            if (is_log_enabled(DESTINY__WARP_TRACE))
                _log(DESTINY__WARP_TRACE, "Destiny::WarpDecel(): %s(%u) is being added to bubble %u.",\
                mySE->GetName(), mySE->GetID(), m_targBubble->GetID());
            m_targBubble->Add(mySE);
        }
    }

    // updated warp drop speed
    if (currentShipSpeed <= m_speedToLeaveWarp)
        WarpStop(m_speedToLeaveWarp);
}

void DestinyManager::WarpUpdate(int64 currentShipSpeed, uint16 sec_into_warp, uint8 type/*0*/) {
    //  track position and velocity for all stages.
    m_targetDistance -= currentShipSpeed;
    m_shipVelocity.x = (m_targetHeading.x * currentShipSpeed);
    m_shipVelocity.y = (m_targetHeading.y * currentShipSpeed);
    m_shipVelocity.z = (m_targetHeading.z * currentShipSpeed);
    m_position.x += m_shipVelocity.x;
    m_position.y += m_shipVelocity.y;
    m_position.z += m_shipVelocity.z;
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

        Vector3d delta = m_targBubble->GetCenter() - m_position;
        double dist = delta.Length();
        _log(DESTINY__WARP_TRACE, "Destiny::WarpUpdate()  %s(%u): Ship is %0.1f from center of target bubble %u.", \
                mySE->GetName(), mySE->GetID(), dist, m_targBubble->GetID());
    }
}

void DestinyManager::WarpStop(int64 currentShipSpeed) {
    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        _log(DESTINY__WARP_TRACE, "Destiny::WarpStop(): %s(%u) - Warp complete. Exit velocity %lli m/s with %lli left to go.", \
                mySE->GetName(), mySE->GetID(), currentShipSpeed, m_targetDistance);
        _log(DESTINY__WARP_TRACE, "Destiny::WarpStop(): Ship currently at %0.2f,%0.2f,%0.2f.", \
                m_position.x, m_position.y, m_position.z);
    }

    // is this needed?
    m_targetPoint += (m_shipVelocity * m_agility);

    Stop();

    if ((mySE->IsNPCSE()) and (mySE->GetNPCSE()->GetAI() != nullptr))
        mySE->GetNPCSE()->GetAI()->WarpOutComplete();

    /*  this isnt used yet, but will be needed once bumping is implemented...
     *    // reset bump checks
     *    SetBallMassive sbmassive;
     *        sbmassive.entityID = mySE->GetID();
     *        sbmassive.is_massive = sConfig.cosmic.BumpEnabled;
     *    PyTuple *up = sbmassive.Encode();
     *    SendSingleDestinyUpdate(&up);
     *    PyDecRef(up);
     */
}

//called whenever an entity is going away and can no longer be used as a target
void DestinyManager::EntityRemoved(SystemEntity *pSE) {
    if (m_targetEntity.second == pSE) {
        m_targetEntity.first = 0;
        m_targetEntity.second = nullptr;

        Stop();
        return;

        switch(m_ballMode) {
            case Destiny::Ball::Mode::FOLLOW: {
                _log(DESTINY__DEBUG, "%u: Our target entity has gone away. Stopping ship.", mySE->GetID());
                Stop();
            } break;
            case Destiny::Ball::Mode::ORBIT: {
                // we were orbiting this target, reset movement to void orbit.  should we stop here?  dunno...
                _log(DESTINY__DEBUG, "%u: Our target entity has gone away. Continue on current tangent of orbit.", mySE->GetID());
                Vector3d heading = m_shipVelocity;
                heading.Normalize();
                GotoDirection(heading);
            } break;
            // no default
        }
    }
}

// there may be more here im missing.  we can also target SEs for dock, jump, etc
bool DestinyManager::ValidTarget() {
    if (__builtin_expect(m_targetEntity.second == nullptr, 0))
        return false;

    SystemEntity* pSE = m_targetEntity.second;
    if (pSE->IsDead()) {
        EntityRemoved(pSE);
        return false;
    }
    if (pSE->IsStationSE() or pSE->IsGateSE() or pSE->IsBeltSE()
    or pSE->IsPOSSE() or pSE->IsItemEntity() or pSE->IsObjectEntity())
        return true;

    if (pSE->IsDead() or pSE->IsInvul() or pSE->IsFrozen())
        return false;

    DestinyManager* pDM = pSE->DestinyMgr();
    if (pDM != nullptr) {
        if (pDM->IsWarping() or pDM->IsCloaked())
            return false;
    }

    if (pSE->IsStaticEntity())
        return false;
    if (pSE->HasPilot() and pSE->GetPilot()->IsDocked())
        return false;

    return true;
}


// Basic Movement Call:
void DestinyManager::BeginMovement() {
    // common movement for all types
    if (!m_hasSentShipUpdates
    and (mySE->HasPilot() or mySE->SysBubble()->HasPlayers())) {
        // error fix for setting ship movement variables before ship is in bubble (cannot BubbleCast)
        std::vector<PyTuple*> updates;
        SetBallAgility sbagility;
            sbagility.entityID =  mySE->GetID();
            sbagility.agility = mySE->GetSelf()->GetAttribute(AttrInertiaMod).get_double();
        updates.push_back(sbagility.Encode());
        if (sConfig.cosmic.BumpEnabled) {
            SetBallMassive sbmassive;
                sbmassive.entityID = mySE->GetID();
                sbmassive.is_massive = 1;
            updates.push_back(sbmassive.Encode());
        }
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
    m_moveDelay = true;

    // reset move stamps
    m_ticStamp = mySE->SystemMgr()->GetTicCount();
    m_timeStamp = GetFileTimeNow();

    // distance check (per client)
    // Python gateway default constant backup check (0x44fa0000 = 2000m)
    if ((m_ballMode != Destiny::Ball::Mode::WARP) and (m_followDistance < 1))
        m_followDistance = 2000;

    SetSpeedFraction();

    if (is_log_enabled(DESTINY__MOVE_TRACE)) {
        if (m_targetEntity.second != nullptr) {
            SystemEntity* pSE = m_targetEntity.second;
            _log(DESTINY__MOVE_TRACE, "target (%s) position: %0.2f, %0.2f, %0.2f", \
                    pSE->GetName(), pSE->GetPosition().x, pSE->GetPosition().y, pSE->GetPosition().z);
        }
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
    // TODO: is this right?
    m_followDistance = pSE->GetRadius() + mySE->GetRadius() + 1000;
    if (m_followDistance > 2500)
        m_followDistance = 2000;
}

void DestinyManager::GotoDirection(const Vector3d& direction) {
    m_targetHeading = direction;
    m_targetPoint = m_position + (direction * 1e9);
    m_targetDistance = 1e9;
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

void DestinyManager::GotoPoint(const Vector3d& point) {
    m_targetPoint = point;
    m_targetHeading = point - m_position;
    m_targetDistance = m_targetHeading.Length();
    m_targetHeading.Normalize();
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
    if (!ValidTarget()) {
        Stop();
        return;
    }

    m_followDistance = distance + mySE->GetRadius() + pSE->GetRadius();

    m_targetHeading = pSE->GetPosition() - m_position;
    m_targetDistance = m_targetHeading.Length();
    m_targetHeading.Normalize();

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

    m_ballMode = Destiny::Ball::Mode::ORBIT;
    m_targetEntity.first = pSE->GetID();
    m_targetEntity.second = pSE;
    if (!ValidTarget()) {
        Stop();
        return;
    }

    m_followDistance = distance;
    m_targetDistance = m_followDistance + pSE->GetRadius() + mySE->GetRadius();

    m_ballMode = Destiny::Ball::Mode::ORBIT;
    BeginMovement();

    CmdOrbit du;
        du.entityID = mySE->GetID();
        du.orbitEntityID = pSE->GetID();
        du.distance = distance;
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

// Fleet warps - on live, all ships use the warp profile of the slowest ship, here they warp as normal
void DestinyManager::WarpTo(const Vector3d& destPoint, int32 distance, bool autoPilot, SystemEntity* pSE/*nullptr*/) {
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

    Vector3d warp_vector = m_targetPoint - m_position;
    m_followDistance = distance;
    m_targetDistance = warp_vector.Length();
    m_targetDistance -= distance;

    if (mySE->HasPilot()) {
        Client* pClient = mySE->GetPilot();
        if (m_targetDistance < MIN_WARP_DISTANCE) {
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

        double dist = ((double)m_targetDistance / ONE_AU_IN_METERS);
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
                Vector3d warp_direction = m_targetPoint - m_position;
                // make heading
                warp_direction.Normalize();
                m_targetPoint = m_position;
                m_targetPoint.x -= (warp_direction.x * m_targetDistance);
                m_targetPoint.y -= (warp_direction.y * m_targetDistance);
                m_targetPoint.z -= (warp_direction.z * m_targetDistance);
                pClient->SendErrorMsg("Your commanded warp of %0.3f AU requires %0.1f GJ of capacitor charge, but you only have %0.1f GJ available.<br>This is enough for %0.3f AU.<br><br>Setting course for available capacitor charge.", \
                        dist, capNeeded,  currentShipCap, capHas );
                /** @todo  update all move vars here for new target... */
                warp_vector = m_targetPoint - m_position;
                m_targetDistance = warp_vector.Length();
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
            warp_vector.Normalize();
            // adjust target point by calculated stopping point
            //m_targetPoint.x -= (warp_vector.x * distance);
            //m_targetPoint.y -= (warp_vector.y * distance);
            //m_targetPoint.z -= (warp_vector.z * distance);
        }
    }

    //You will always exit warp at a random point 2,500 meters from your actual exit point - per EveUni
	//  this will need update for formation warp...leader will have random offset, others follow per formation
    /* disabled for testing
     * if (mySE->HasPilot())
     *   m_targetPoint.MakeRandomPointOnSphereLayer(-2500, 2500);   disabled for testing
     */

    // reset heading for updated targPoint
    m_targetHeading = m_targetPoint - m_position;
    // change to heading
    m_targetHeading.Normalize();

    // get targ bubble.  this will create bubble if needed
    m_targBubble = sBubbleMgr.GetBubble(mySE->SystemMgr(), m_targetPoint);

    // npcs have no warp restrictions (yet)
    if (mySE->IsNPCSE()) {
        m_ballMode = Destiny::Ball::Mode::WARP;
        BeginMovement();

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
        }

        if (is_log_enabled(NPC__MESSAGE))
            _log(NPC__MESSAGE, "Destiny::WarpTo() NPC %s(%u) %u -> %u, m_targetPoint: %0.2f,%0.2f,%0.2f  stop distance: %i  m_targetDistance: %llim", \
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

    // if usf < 75%, set usf for warp
    if (m_userSpeedFraction < 0.751)
        SetSpeedFraction(1.0f);

    m_ballMode = Destiny::Ball::Mode::WARP;
    BeginMovement();

    if (is_log_enabled(DESTINY__WARP_DEBUG)) {
        // marker for ship's stop point
        std::string str = "Warp Stop Point - ";
        str += mySE->GetName();
        MarkPoint(m_targetPoint, str, str);
        str.clear();

        // marker for ship's drop out of warp point
        Vector3d dropPos = (m_targetHeading * m_speedToLeaveWarp * m_agility);
        str = "Warp Drop Point - ";
        str += mySE->GetName();
        MarkPoint(m_targetPoint - dropPos, str, str);
    }

    //set massive for warp.   self-only per client logs
    SetBallMassive sbmassive;
        sbmassive.entityID = mySE->GetID();
        sbmassive.is_massive = 0;       // disable client-side bump checks
    PyTuple *up = sbmassive.Encode();
    SendSingleDestinyUpdate(&up, true);
    PyDecRef(up);

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

bool DestinyManager::IsAligned(Vector3d& targHeading) {
    double distanceSq = m_shipVelocity.LengthSq();
    if (distanceSq <  VISUAL_STOP_THRESHOLD_SQ) {
        // if ship is stopped enough, there is no 'aligning' to do
        return true;
    }

    double dot = m_shipVelocity * targHeading;
    //double degrees = EvE::Trig::Rad2Deg(std::acos(dot));
    if (m_ballMode == Destiny::Ball::Mode::WARP) {
        if (dot > /*WARP_ALIGNMENT*/ 0.99449256)
            return true;
    } else if (dot > /*TURN_ALIGNMENT*/ 0.99756405) {
        return true;
    }
    return false;
}

void DestinyManager::Undock(Vector3d dir) {
    // delay first tic for client to get its' shit together
    m_moveDelay = true;
    //set movement direction
    m_targetPoint = dir * 1e9;
    m_targetHeading = dir;
    m_followDistance = 500;
    SetUndockSpeed();

    if (is_log_enabled(DESTINY__MOVE_DEBUG))
        _log(DESTINY__MOVE_DEBUG, "Destiny::Undock() %s(%u) - m_targetHeading: %0.3f, %0.3f, %0.3f", \
                mySE->GetName(), mySE->GetID(), m_targetHeading.x, m_targetHeading.y, m_targetHeading.z);
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
    Client* pClient = mySE->GetPilot();
    uint32 stationID = pClient->GetDockStationID();
    SystemEntity *stationSE = mySE->SystemMgr()->GetSE(stationID);

    if ( stationSE == nullptr) {
        codelog(CLIENT__ERROR, "%s: Station %u not found.", pClient->GetName(), stationID);
        pClient->SendErrorMsg("Station Not Found, Docking Aborted.");
        return PyStatic.NewNone();
    }

    //get the station Docking Perimeter
    Vector3d delta =  stationSE->GetPosition() - m_position;
    double distance = delta.Length() - (mySE->GetRadius() + stationSE->GetRadius());

    // Verify range to station is within docking perimeter of 2500 meters:
    _log(DESTINY__TRACE, "Destiny::AttemptDockOperation() rangeToStationPerimiter is %0.2fm", distance);
    if (distance > 2500.0) {
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
    Client* pClient(mySE->GetPilot());
    // this would be an error.  only players call this method.
    if (pClient == nullptr)
        return;

    SystemEntity *pSE = mySE->SystemMgr()->GetSE(pClient->GetDockStationID());
    if (pSE == nullptr)
        return;

    const Vector3d stationPos = pSE->GetPosition();
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

void DestinyManager::SetPosition(const Vector3d &pt, bool update /*false*/) {
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

// set npc speed as fraction of maxSpeed
void DestinyManager::SetNPCSpeed(uint16 newSpeed, uint16 maxSpeed) {
    if (mySE->HasPilot()) {
        // error
        _log(DESTINY__ERROR, "Destiny::SetMaxVelocity(%u) - Called by Player Ship %s(%u)", \
                newSpeed, mySE->GetName(), mySE->GetID());
        return;
    }

    m_maxSpeed = maxSpeed;
    SetSpeedFraction(newSpeed / maxSpeed);

    if (is_log_enabled(DESTINY__TRACE))
        _log(DESTINY__TRACE, "Destiny::SetMaxVelocity() - Ship:%s(%u) - newSpeed: %u, maxSpeed: %u, usf: %0.2f", \
                mySE->GetName(), mySE->GetID(), newSpeed, maxSpeed, m_userSpeedFraction);
}

void DestinyManager::SpeedBoost(bool deactivate/*false*/) {
    // Recalculate ship variables
    SetAgilityInertia();

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
void DestinyManager::WebbedMe()
{
    // TODO:  this will need more thought and work to play nice with npc speeds
    double oldMaxSpeed = m_maxSpeed;

    SetAgilityInertia();

    // Safety fallback: prevent division by zero if a ship somehow has a 0m/s baseline max speed
    if (m_maxSpeed > 1e-6) {
        // Scale user speed throttle fraction across the web transition
        m_userSpeedFraction = (m_userSpeedFraction * oldMaxSpeed) / m_maxSpeed;
    }

    std::vector<PyTuple*> updates;
    SetBallSpeed sbms;
    	sbms.entityID = mySE->GetID();
        sbms.speed    = m_maxSpeed;
    updates.push_back(sbms.Encode());

    SendDestinyUpdates(updates);
    m_hasSentShipUpdates = true;
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
    if (!sRef->HasAttribute(AttrInertiaMod))    // this should never hit
        sLog.Error("DM::UpdateShipVariables", "%s(%u) does not have an InertiaMod", mySE->GetName(), mySE->GetTypeID());

    float mass = sRef->mass(); // in mKg
    double inertiaMod = sRef->GetAttribute(AttrInertiaMod).get_double();

    // This represents e^(-dt/tau) where dt = tic time in seconds (dt = 1.0)
    m_agility = mass * inertiaMod / Destiny_k;
    m_expTerm = std::exp(-1.0 / m_agility);
    m_posScale = m_agility * (1.0 - m_expTerm);
    m_alignTime = 1.3862943611198906 * m_agility;
    sRef->SetAttribute(AttrAgility, m_agility, false);

    // for npc, this is mwd/ab speed, not sub-warp speed, but will be adjusted based on npc activity
    if (sRef->HasAttribute(AttrMaxVelocity)) {
        m_maxSpeed = sRef->GetAttribute(AttrMaxVelocity).get_float();
    }

    // verify hull overspeed isnt reached
    if (sRef->HasAttribute(AttrMaxDirectionalVelocity)) {
        if (m_maxSpeed > sRef->GetAttribute(AttrMaxDirectionalVelocity).get_float())
            m_maxSpeed = sRef->GetAttribute(AttrMaxDirectionalVelocity).get_float();
    }

    if (is_log_enabled(DESTINY__TRACE))
        _log(DESTINY__TRACE, "Destiny::SetAgilityInertia() %s(%u) - m_maxSpeed: %0.1f, m_alignTime: %0.1f, mass: %0.1f * inertiaMod: %f = m_agility: %f, radius: %i", \
                mySE->GetName(), mySE->GetID(), m_maxSpeed, m_alignTime, mass, inertiaMod, m_agility, sRef->radius());
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
            default: {
                sLog.Warning("DM::UpdateShipVariables", "NPC %s(%u) has no size.  warp speed is %0.2f", \
                        mySE->GetName(), mySE->GetTypeID(), m_shipWarpSpeed);
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

    if (mySE->IsNPCSE()) {
        m_speedToLeaveWarp = sRef->GetAttribute(AttrEntityCruiseSpeed).get_float() * 0.85f;
    } else {
        m_speedToLeaveWarp = m_maxSpeed * 0.85f;
    }

    if (m_speedToLeaveWarp < 100)
        m_speedToLeaveWarp = 100;

    if (!mySE->HasPilot() or (mySE->GetPilot() == nullptr) or mySE->GetPilot()->IsLogin())
        return;

    if (mySE->GetPilot()->IsInSpace() and (mySE->SysBubble() != nullptr)) {
        std::vector<PyTuple*> updates;
        SetBallAgility sbagility;
            sbagility.entityID =  mySE->GetID();
            sbagility.agility = sRef->GetAttribute(AttrInertiaMod).get_double();
        updates.push_back(sbagility.Encode());
        if (sConfig.cosmic.BumpEnabled) {
            SetBallMassive sbmassive;
                sbmassive.entityID = mySE->GetID();
                sbmassive.is_massive = 1;
            updates.push_back(sbmassive.Encode());
        }
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
    m_maxSpeed = pMissile->GetSpeed();
    //SetPosition(pMissile->GetSelf()->position());
    m_agility = (pMissile->GetSelf()->type().mass() * pMissile->GetSelf()->GetAttribute(AttrInertiaMod).get_double());
    m_agility /= 1000000.0;
    // not sure if this will be used here.  attr 559
    pMissile->GetSelf()->SetAttribute(AttrAgility, m_agility, false);

    m_stop = false;
    m_ballMode = Destiny::Ball::Mode::MISSILE;
    m_ticStamp = mySE->SystemMgr()->GetTicCount();

    SystemEntity* pTarget = pMissile->GetTargetSE();
    m_targetPoint = pTarget->GetPosition();
    m_targetEntity.first = pTarget->GetID();
    m_targetEntity.second = pTarget;

    // is this really needed for missile?
    m_targetHeading = pTarget->GetPosition() - m_position;
    m_targetDistance = m_targetHeading.Length();
    //change vector to direction
    m_targetHeading.Normalize();

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

void DestinyManager::TractorBeamStart(SystemEntity* pSE, EvilNumber speed) {
    m_ballMode = Destiny::Ball::Mode::FOLLOW;

    m_stop = false;
    m_tractored = true;
    //m_moveTime = GetTimeMSeconds();
    m_ticStamp = mySE->SystemMgr()->GetTicCount();
    m_targetEntity.first = pSE->GetID();
    m_targetEntity.second = pSE;

    m_targetPoint = pSE->GetPosition();
    m_targetHeading = pSE->GetPosition() - m_position;
    m_targetDistance = m_targetHeading.Length();
    m_targetHeading.Normalize();
    m_followDistance = 500;

    m_maxSpeed = speed.get_float();   // **TODO:  use AttrMaxTractorVelocity
    m_shipVelocity.x = m_targetHeading.x * m_maxSpeed;
    m_shipVelocity.y = m_targetHeading.y * m_maxSpeed;
    m_shipVelocity.z = m_targetHeading.z * m_maxSpeed;
    m_targetVelocity = m_shipVelocity;

    m_targetDistance += 500 + pSE->GetRadius();
    m_userSpeedFraction = 1.0f;
    m_activeSpeedFraction = 1.0f;

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
        fb.targetID = pSE->GetID();
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
    // verify there is someone to send this to before we create it
    if (!mySE->HasPilot() and !mySE->SysBubble()->HasPlayers())
        return;

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
    SendSingleDestinyUpdate(&up);
}

// def OnSpecialFX(shipID, moduleID, moduleTypeID, targetID, otherTypeID, area, guid, isOffensive, start, active, duration = -1, repeat = None, startTime = None, graphicInfo = None):
// GFX method for graphics effects
void DestinyManager::SendGFX14(int32 entityID, int32 moduleID, int32 moduleTypeID, int32 targetID,
                               int32 chargeTypeID, std::string guid, bool isOffensive, bool start,
                               bool isActive, int32 duration, int32 repeat, int64 startTime/*0*/,
                               int32 graphicInfo/*0*/, Client* pClient/*nullptr*/) const
{
    // verify there is someone to send this to before we create it
    if (!mySE->HasPilot() and !mySE->SysBubble()->HasPlayers())
        return;

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
    if (mySE->IsNPCSE() and is_log_enabled(EFFECTS__DUMP_NPC)) {
        up->Dump(EFFECTS__DUMP_NPC, "");
    } else if (is_log_enabled(EFFECTS__DUMP)) {
        up->Dump(EFFECTS__DUMP, "");
    }
    if (pClient == nullptr) {
        SendSingleDestinyUpdate(&up);
    } else {
        // this is to update new ship in bubble with active gfx
        pClient->QueueDestinyUpdate(&up);
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
        ss.stamp = mySE->SystemMgr()->GetTicCount();
        ss.ego = mySE->GetID();

    mySE->SystemMgr()->MakeSetState(mySE->SysBubble(), ss);
    PyTuple* tmp(ss.Encode());
    //setstate should be alone and immediate.  send directly
    mySE->GetPilot()->QueueDestinyUpdate(&tmp, false, true);   // consumed?
    mySE->GetPilot()->SetStateSent(true);
}

void DestinyManager::ApplyVortexGravityTug(ShipSE* pShipSE, Vector3d vortexCenterAxis) {
    if (!pShipSE or pShipSE->IsDead() or !pShipSE->HasPilot())
        return;

    Vector3d currentPos = pShipSE->GetPosition();
    Vector3d pullVector = vortexCenterAxis - currentPos;
    double distanceToVortex = pullVector.Length();

    // Only apply the gravity tug if the player is deep inside the anomaly cluster boundaries (< 25km)
    if (distanceToVortex < 25000.0 && distanceToVortex > 1000.0) {
        pullVector.Normalize();

        // Establish your classic 5-10 m/s pull speed drift constant
        double gravityPullVelocity = 7.5;

        // Safely alter their Destiny movement engine parameters natively for this frame tick
        //pShipSE->DestinyMgr()->InjectExternalForceVector(pullVector * gravityPullVelocity);

        // Track time milestones to fire off your customized Neocom system telemetry messages
        if (mySE->SystemMgr()->GetTicCount() % 100 == 0)  // Telemetry alert check interval
            pShipSE->GetPilot()->SendNotifyMsg("NEOCOM ALERT: Odd local spatial vortex detected... ship drift active.");
    }
}

void DestinyManager::SetTrollData(DestinyManager* pDestiny) {
    m_shipVelocity = pDestiny->GetVelocity();
    m_maxSpeed = pDestiny->GetMaxVelocity();
    m_targetPoint = pDestiny->GetTargetPoint();
    m_followDistance = 0;

    if (m_targetPoint.isZero()) {
        m_targetHeading = m_shipVelocity;
        m_targetDistance = m_targetHeading.Length();
        m_targetHeading.Normalize();
        m_targetPoint = m_targetHeading * 1.0e9;
    } else {
        m_targetHeading = m_targetPoint - m_position;
        m_targetDistance = m_targetHeading.Length() + 150000;
        m_targetHeading.Normalize();
    }

    // stop is not needed, i think
    //Stop();
    m_ticStamp = mySE->SystemMgr()->GetTicCount();
}

void DestinyManager::CreateShipMarker() {
    if (m_activeSpeedFraction > sConfig.debug.ShipTrackingTime) {
        // create jetcan to visualize movement
        std::string str = mySE->GetName();
        str += " " + GetModeNameString() + " ";
        str += itoa(mySE->SystemMgr()->GetTicCount() - m_ticStamp);
        MarkPoint(m_position, str, str);
    }
}

void DestinyManager::MarkPoint(const Vector3d& position, std::string& name, std::string& desc, bool orbit/*false*/) {
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
    m_shipMarkers.emplace(cRef->itemID(), cSE);
}

void DestinyManager::RemoveAllMarkers() {
    RemoveShipMarkers();
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
                            mySE->SystemMgr()->GetTicCount(), updates.size(), mySE->SysBubble()->GetID(), mySE->GetName(), mySE->GetID() );
                mySE->SysBubble()->BubblecastDestinyUpdate(updates, "DestinyUpdates");
            } else {
                for (auto &cur : updates)
                    PySafeDecRef(cur);
            }
            return;
        }
        if (is_log_enabled(PLAYER__MESSAGE))
            _log(PLAYER__MESSAGE, "[%u] DM::SendUpdates() called as 'self_only' for %s(%i)", \
                    mySE->SystemMgr()->GetTicCount(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());

        for (std::vector<PyTuple*>::iterator itr = updates.begin(); itr != updates.end(); itr++) {
            PyIncRef(*itr);
            mySE->GetPilot()->QueueDestinyUpdate(&(*itr));
        }
    } else if (mySE->IsOperSE()) { //These are global entities, so we have to send update to all bubbles in a system
        if (is_log_enabled(DESTINY__UPDATES))
            _log(DESTINY__UPDATES, "[%u] DM::SendUpdates() - BubbleCasting Structure DestinyUpdates in %s from %s(%u)", \
                    mySE->SystemMgr()->GetTicCount(), mySE->SystemMgr()->GetName(), mySE->GetName(), mySE->GetID());

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
                    mySE->SystemMgr()->GetTicCount(), updates.size(), mySE->SysBubble()->GetID(),   \
                    (mySE->HasPilot()?mySE->GetPilot()->GetName():mySE->GetName()), \
                    (mySE->HasPilot()?mySE->GetPilot()->GetCharID():mySE->GetID()) );
            mySE->SysBubble()->BubblecastDestinyUpdate(updates, "DestinyUpdates");
    } else {
        _log(DESTINY__WARNING, "[%u] DM::SendUpdates() - Cannot BubbleCast %lu DestinyUpdates; entity (%u) is not in any bubble. (mySE->SysBubble() == nullptr)", \
                mySE->SystemMgr()->GetTicCount(), updates.size(), mySE->GetID() );
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
                            mySE->SystemMgr()->GetTicCount(), mySE->SysBubble()->GetID(), mySE->GetName(), mySE->GetID() );
                mySE->SysBubble()->BubblecastDestinyEvent(ev, "DestinyEvent");
            }
            return;
        }
        if (is_log_enabled(PLAYER__MESSAGE))
            _log(PLAYER__MESSAGE, "[%u] DM::SingleEvent() - Self-Only' for %s(%i)", \
                mySE->SystemMgr()->GetTicCount(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());

        mySE->GetPilot()->QueueDestinyEvent(ev);
    } else if (mySE->IsOperSE()) { //These are global entities, so we have to send update to all players in a system
        if (is_log_enabled(DESTINY__UPDATES))
            _log(DESTINY__UPDATES, "[%u] DM::SingleEvent() - BubbleCasting Structure DestinyEvent in %s from %s(%u)", \
            mySE->SystemMgr()->GetTicCount(), mySE->SystemMgr()->GetName(), mySE->GetName(), mySE->GetID());

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
                    mySE->SystemMgr()->GetTicCount(), mySE->SysBubble()->GetID(),   \
                    (mySE->HasPilot()?mySE->GetPilot()->GetName():mySE->GetName()),\
                    (mySE->HasPilot()?mySE->GetPilot()->GetCharID():mySE->GetID()) );
        mySE->SysBubble()->BubblecastDestinyEvent(ev, "Single DestinyEvent" );
    } else {
        _log(DESTINY__WARNING, "[%u] DM::SingleEvent() - Cannot BubbleCast: entity %s(%u) is not in any bubble. (mySE->SysBubble() == nullptr)", \
                mySE->SystemMgr()->GetTicCount(), mySE->GetName(), mySE->GetID() );
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
                            mySE->SystemMgr()->GetTicCount(), mySE->SysBubble()->GetID(), mySE->GetName(), mySE->GetID() );
                mySE->SysBubble()->BubblecastDestinyUpdate(up, "Single Self-Only DestinyUpdate");
            }
            return;
        }
        if (is_log_enabled(PLAYER__MESSAGE))
            _log(PLAYER__MESSAGE, "[%u] DM::SingleUpdate() Self-Only' for %s(%i)", \
                    mySE->SystemMgr()->GetTicCount(), mySE->GetPilot()->GetName(), mySE->GetPilot()->GetCharacterID());

        mySE->GetPilot()->QueueDestinyUpdate(up);
    } else if (mySE->IsOperSE()) { //These are global entities, so we have to send update to all players in a system
        if (is_log_enabled(DESTINY__UPDATES))
            _log(DESTINY__UPDATES, "[%u] DM::SingleUpdate() - BubbleCasting Structure DestinyUpdate in %s from %s(%u)", \
                    mySE->SystemMgr()->GetTicCount(), mySE->SystemMgr()->GetName(), mySE->GetName(), mySE->GetID());

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
                    mySE->SystemMgr()->GetTicCount(), mySE->SysBubble()->GetID(),   \
                    (mySE->HasPilot()?mySE->GetPilot()->GetName():mySE->GetName()),\
                    (mySE->HasPilot()?mySE->GetPilot()->GetCharID():mySE->GetID()) );
        mySE->SysBubble()->BubblecastDestinyUpdate(up, "Single DestinyUpdate" );
    } else {
        _log(DESTINY__WARNING, "[%u] DM::SingleUpdate() - Cannot BubbleCast: entity %s(%u) is not in any bubble. (mySE->SysBubble() == nullptr)", \
        mySE->SystemMgr()->GetTicCount(), mySE->GetName(), mySE->GetID() );
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