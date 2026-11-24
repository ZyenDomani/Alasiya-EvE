/*
 * BallPark.cpp
 *      the solar system's ballpark class; handles all movement in system
 *
 * @Author:     Allan
 * @Version:    0.15
 * @Date:       10Nov26
 */


//  NOTE:  -Wpadded in gcc args will print a warning showing the exact byte count of every padding gap in your structs

#include "BallPark.h"

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
#include "system/MoveData.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"

// vector = (to - from)

// called @1Hz for each entity in movemap by SystemManager::Process()
void BallPark::Process(SpaceEntity* pSE, int32 tic) {
    if (pSE == nullptr)
        return;
    MoveData* mData = pSE->GetMoveData();
    // should we create default data if null?  probably not...no data = no move
    if (mData == nullptr)
        return;
    if (mData->moveDelay) {
        mData->moveDelay = false;
        return;
    }

    const double profileStartTime = (sConfig.debug.UseProfiling ? GetProfileTime() : 0.0);
    using namespace Ball::Mode;
    switch(mData->ballMode) {
        case RIGID:
        case MISSILE: {
            return;
        } break;
        case STOP: {
            // Apply vertical pitch dampener directly to velocity
            mData->myVelocity.y *= 0.8691588785046729;
        } break;
        case GOTO: {
            // there's not necessarily a target here...just a direction to point in space.
            if (mData->alignTo and IsAligned(mData)) {
                if (is_log_enabled(DESTINY__MOVE_TRACE)) {
                    sLog.Warning("BallPark::Process(Goto)", "%s is aligned.", pSE->GetName());
                }

                Stop(mData);
                break;
            }

            if (mData->IsUndocking())
                break;

            GotoVelocity(mData); // Includes exponential homing zone
        } break;
        case FOLLOW: {
            if (!ValidTarget(mData)) {
                if (is_log_enabled(DESTINY__MOVE_TRACE))
                    sLog.Warning("BallPark::Process(Follow)", "%s's target is invalid", pSE->GetName());
                Stop(mData);
                break;
            }

            CalculateFollowPoint(mData);
            GotoVelocity(mData);
        } break;
        case FORMATION: {
            if (!ValidTarget(mData)) {
                Stop(mData);
                return;
            }
            CalculateFormationPoint(mData);
            GotoVelocity(mData);
        } break;
        case ORBIT: {
            if (!ValidTarget(mData)) {
                Stop(mData);
                return;
            }

            OrbitVelocity(mData, tic - mData->ticStamp);
        } break;
        case WARP: {
            if (mData->warpState.active) {
                //warp is in progress
                const double elapsed = static_cast<double>(tic - mData->warpState.startTic);
                //  speed and distance formulas based on current warp distance
                switch (mData->warpState.phase) {
                    case WarpState::Phase::Accel: {
                        WarpAccel(mData, elapsed);
                    } break;
                    case WarpState::Phase::Cruise: {
                        WarpCruise(mData, elapsed);
                    } break;
                    case WarpState::Phase::Decel: {
                        WarpDecel(mData, elapsed);
                    } break;
                    case WarpState::Phase::None: {
                        // uh, Houston...we have a problem...
                        if (pSE->HasPilot()) {
                            _log(DESTINY__ERROR, "BallPark::Process() Error!  Ship %s(%u) for Player %s(%u) Has WarpState but checks are false.",  \
                                pSE->GetName(), pSE->GetID(), pSE->GetPilot()->GetName(), pSE->GetPilot()->GetCharacterID());
                                pSE->GetPilot()->SendErrorMsg("Internal Server Error.<br> Please Dock, Jump or Relog to reset your ship.");
                        } else {
                            _log(DESTINY__ERROR, "BallPark::Process() Error!  NPC %s(%u) Has WarpState but checks are false.",  \
                                pSE->GetName(), pSE->GetID());
                            WarpStop(mData, mData->myVelocity.Length());
                        }
                    } break;

                }

                if (sConfig.debug.UseProfiling)
                    sProfiler.AddTime(Profile::Type::destiny, GetProfileTime() - profileStartTime);
                return;
            }

            // check speed...if all good, call InitWarp()
            if (IsAligned(mData) and (mData->activeSpeedFraction > 0.749999f)) {
                InitWarp(mData);
                if (sConfig.debug.UseProfiling)
                    sProfiler.AddTime(Profile::Type::destiny, GetProfileTime() - profileStartTime);
                return;
            }
            //TODO:  put check here for stuck ship where (tic - stamp > stucktime)
            // ForceWarp()  will set massive=false, align ship, then begin accel
            //  will have to test against client to verify...not sure how this'll work yet.

            GotoVelocity(mData);
        } break;
        case TROLL: {
            // mobile object that will become rigid after a set time
            // jetcans/wrecks created from moving ship will take that velocity and continue movement for a time, then become rigid.
            //update.  ball is set <state_stop> and <mode_troll> with timestamp.
            //  check elapsed time here while object is moving, then switch to <mode_rigid> once stopped.
            // this elapsed check can/may change based on load in current bubble
            // 3. client uses 30-second window (Matches Ballpark Offset 0x7c)  may reduce this to 15-20s
            uint32 elapsed = tic - mData->ticStamp;
            if ((elapsed >= sConfig.rates.DestinyTrollTime) or (mData->myVelocity.LengthSq() < 0.001)) {
                mData->skipTic = true;
                // client will automagically update <troll> to <rigid> after timer expires
                return;
            }

            GotoVelocity(mData);
        } break;
        case BOID: { // this will turn RIGID after a set time
            /* The Physics:
             * If multiple dynamic items or entities drop simultaneously, they execute a brief, basic separation
             *  step so they do not overlap identically.
             */
        } break;
        default: {
            _log(DESTINY__ERROR, "BallPark::Process() - %s(%u): hit default with state: %s",
                    pSE->GetName(), pSE->GetID(), sDataMgr.GetBallModeName(mData->ballMode));
        } break;
    }

    // integration step using optimized blending glide model
    Integrate(mData);

    // Global boundary parking check
    double speedSq = mData->myVelocity.LengthSq();
    if (mData->stop and (speedSq < 0.001f)) {
        Halt(mData);
        return;
    }

    if (mData->maxSpeed > 0.01f)
        mData->activeSpeedFraction = std::sqrt(speedSq) / mData->maxSpeed;

    // only create tracking can when ship is moving significant amount
    if (sEntityMgr.GetTracking())
        pSE->SystemMgr()->CreateShipMarker(pSE);

    if (is_log_enabled(DESTINY__MOVE_TRACE)) {
        int32 timeDelta = tic - mData->ticStamp;
        _log(DESTINY__MOVE_TRACE, "BallPark::%s(%u) - %s(%i): Current Ship Velocity: %0.2f, %0.2f, %0.2f @ asf: %0.3f (speed: %0.1f)", \
                sDataMgr.GetBallModeName(mData->ballMode), timeDelta, pSE->GetName(), pSE->GetID(), \
                mData->myVelocity.x, mData->myVelocity.y, mData->myVelocity.z, \
                mData->activeSpeedFraction, mData->myVelocity.Length());
        const Vector3d& position = pSE->GetPosition();
        _log(DESTINY__MOVE_TRACE, "Current position: %0.2f, %0.2f, %0.2f", \
                position.x, position.y, position.z);
        if (mData->targetPoint.notZero())
            _log(DESTINY__MOVE_TRACE, "Ship target Point: %0.2f, %0.2f, %0.2f", \
                    mData->targetPoint.x, mData->targetPoint.y, mData->targetPoint.z);
        if (mData->desiredVelocity.notZero())
            _log(DESTINY__MOVE_TRACE, "Ship target Velocity: %0.2f, %0.2f, %0.2f", \
                    mData->desiredVelocity.x, mData->desiredVelocity.y, mData->desiredVelocity.z);
    }

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::Type::destiny, GetProfileTime() - profileStartTime);
}

void BallPark::Integrate(MoveData* mData) {
    // UNIVERSAL HIGH-PERFORMANCE TWO-LINE BLENDER
    // Fully unrolled to enforce strict sequential IEEE-754 rounding Parity
    Vector3d& pos = mData->mySE->GetPositionRef();

    // declaring 'const' allows reuse without create/destroy on every call
    const double dvx = mData->myVelocity.x - mData->desiredVelocity.x;
    const double dvy = mData->myVelocity.y - mData->desiredVelocity.y;
    const double dvz = mData->myVelocity.z - mData->desiredVelocity.z;

    pos.x += mData->desiredVelocity.x + (dvx * mData->posScale);
    pos.y += mData->desiredVelocity.y + (dvy * mData->posScale);
    pos.z += mData->desiredVelocity.z + (dvz * mData->posScale);

    mData->myVelocity.x = mData->desiredVelocity.x + (dvx * mData->expTerm);
    mData->myVelocity.y = mData->desiredVelocity.y + (dvy * mData->expTerm);
    mData->myVelocity.z = mData->desiredVelocity.z + (dvz * mData->expTerm);
}

void BallPark::CalculateFollowPoint(MoveData* mData) {
    // should this check for moving/static objects?
    //if (mData->targetEntity.second->IsStaticEntity())
    //    return;
    // 1. Extract the target's  position
    const Vector3d& targPos = mData->targetSE->GetPosition();
    const Vector3d delta = targPos - mData->mySE->GetPosition();
    double distSq = delta.LengthSq();

    if (distSq < 1e-10) {
        // go out in some arbitrary direction
        mData->targetPoint.x = targPos.x + mData->desiredDistance;
        mData->targetPoint.y = targPos.y;
        mData->targetPoint.z = targPos.z;
    } else {
        // target point in range along our vector
        double newDist = mData->desiredDistance / std::sqrt(distSq);
        mData->targetPoint.x = targPos.x - (delta.x * newDist);
        mData->targetPoint.y = targPos.y - (delta.y * newDist);
        mData->targetPoint.z = targPos.z - (delta.z * newDist);
    }
}

void BallPark::CalculateFormationPoint(MoveData* mData) {
    // for formations
    SpaceEntity* leader = mData->targetSE;

    /*
     *    // 1. Validate formation arrays
     *    int formationID = leader->GetFormationID();
     *    if (formationID < 0 || formationID >= static_cast<int>(mData->formations.size())) {
     *        mData->targetPoint = mData->position;
     *        return;
}
// 2. Extract designated slot index from mEffectStamp network footprint
size_t slot = static_cast<size_t>(pSE->GetEffectStamp());
if (slot >= mData->formations[formationID].size()) {
    mData->targetPoint = mData->position;
    return;
}
*/
    // 3. Extract raw configured offset layout vector
    Vector3d offset = NULL_ORIGIN; //mData->formations[formationID][slot];
    double length = offset.Length();

    if (length <= 1e-12)
        length = 1.0;

    // 4. Scale local offset vector adding physical radius padding
    double totalScaleFactor = (mData->mySE->GetRadius() + leader->GetRadius() + length) / length;
    Vector3d localOffset = offset * totalScaleFactor;

    // =========================================================================
    // 5. THE ROTATION PLUG-IN POINT
    // Transpose the local layout to match the leader's actual direction of flight
    // =========================================================================
    Vector3d worldOffset = RotateVectorByEntityOrientation(mData->mySE, localOffset, leader);

    // 6. Anchor the final server target point relative to the leader
    mData->targetPoint = leader->GetPosition() + worldOffset;
}

void BallPark::OrbitVelocity(MoveData* mData, double ticCount) {
    // 1. Establish the azimuth angle
    const double phi1 = ticCount * ORBITAL_PRECESSION;
    // elevation angle
    const double phi2 = (double)(mData->mySE->GetID() & 0x0000FFFF) + phi1;

    // 2. Generate the base unit orientation vector
    const double cosPhi2 = std::cos(phi2);
    Vector3d radialVector(
        std::cos(phi1) * cosPhi2,
        std::sin(phi2),
        std::sin(phi1) * cosPhi2
    );

    // shitround the components to 7 decimal digits
    radialVector.x = (double)((int64_t)(radialVector.x * 1e7)) * 1e-7;
    radialVector.y = (double)((int64_t)(radialVector.y * 1e7)) * 1e-7;
    radialVector.z = (double)((int64_t)(radialVector.z * 1e7)) * 1e-7;

    // 3. Compute relative range vector
    Vector3d toVector = mData->targetSE->GetPosition() - mData->mySE->GetPosition();
    const double distSq = toVector.LengthSq();
    if (distSq < 1e-10)
        return; // Safety boundary clamp

    radialVector.Cross(toVector);
    radialVector.Normalize();

    const double dist = std::sqrt(distSq);
    // expand normal to avoid dupe sqrt since we have dist already
    const double invDist = 1.0 / dist;
    toVector.x *= invDist;
    toVector.y *= invDist;
    toVector.z *= invDist;

    // 4. Calculate the tangent vs approach weighting vectors
    //const double targetDistSq = mData->targetDistance * mData->targetDistance;
    const double targetDistSq = mData->desiredDistance * mData->desiredDistance;
    const double toComp = distSq - targetDistSq;

    if (toComp >= 0.0) {
        // updated math
        const double sqrtToComp = std::sqrt(toComp);
        const double invNorm = 1.0 / (dist * sqrtToComp);
        const double toScale = toComp * invNorm;
        //const double radComp = mData->targetDistance * sqrtToComp * invNorm;
        const double radComp = mData->desiredDistance * sqrtToComp * invNorm;

        toVector = (toScale * toVector) + (radComp * radialVector);
    }

    // 5. Apply the official exponential inward-pull curve
    //const double delta        = mData->targetDistance - dist;
    const double delta        = mData->desiredDistance - dist;
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
    //const double signum = (dist > mData->targetDistance) ? 1.0 : ((dist < mData->targetDistance) ? -1.0 : 0.0);
    const double signum = (dist > mData->desiredDistance) ? 1.0 : ((dist < mData->desiredDistance) ? -1.0 : 0.0);
    transverseFactor *= signum;

    // 9. THE BRIDGE: Scale the final direction components directly into terminal velocity space (m/s)
    const double maxVelStep = mData->userSpeedFraction * mData->maxSpeed;
    mData->desiredVelocity.x = (radialVector.x * radialFactor + toVector.x * transverseFactor) * maxVelStep;
    mData->desiredVelocity.y = (radialVector.y * radialFactor + toVector.y * transverseFactor) * maxVelStep;
    mData->desiredVelocity.z = (radialVector.z * radialFactor + toVector.z * transverseFactor) * maxVelStep;
}

void BallPark::GotoVelocity(MoveData* mData) {
    // 1. Calculate the relative displacement heading vector
    mData->desiredHeading = mData->targetPoint - mData->mySE->GetPosition();
    const double lengthSq = mData->desiredHeading.LengthSq();

    // Safety fallback: if we are already precisely stacked on target, kill velocity
    if (lengthSq < 1e-10) {     // from 1e-12
        mData->desiredVelocity = NULL_ORIGIN;
        return;
    }

    mData->desiredHeading.Normalize();

    // maxVelStep defines the absolute maximum terminal velocity magnitude for this tick
    const double maxVelStep = mData->userSpeedFraction * mData->maxSpeed;
    const double maxVelStepSq = maxVelStep * maxVelStep;

    if (lengthSq < maxVelStepSq) {
        // --- BRAKING ZONE: Apply quartic damping curve near the target ---
        // coff = (Distance) / (MaxStep)
        double coff = std::sqrt(lengthSq) / maxVelStep;
        double dynamicVelocityCap = maxVelStep * (coff * coff); // Reduced math footprint

        mData->desiredVelocity.x = mData->desiredHeading.x * dynamicVelocityCap;
        mData->desiredVelocity.y = mData->desiredHeading.y * dynamicVelocityCap;
        mData->desiredVelocity.z = mData->desiredHeading.z * dynamicVelocityCap;
    } else {
        // --- MAX THRUST ZONE: Deep space continuous acceleration capacity ---
        // Natively pre-scaled: No divisions, no multi-million variables. Pure SIMD friendly.
        mData->desiredVelocity.x = mData->desiredHeading.x * maxVelStep;
        mData->desiredVelocity.y = mData->desiredHeading.y * maxVelStep;
        mData->desiredVelocity.z = mData->desiredHeading.z * maxVelStep;
    }
}

Vector3d BallPark::RotateVectorByEntityOrientation(SpaceEntity* pSE, const Vector3d& localVec, SpaceEntity* leader) {
    // for formations
    Vector3d forward = leader->GetVelocity();
    double speedSq = forward.LengthSq();

    // Fallback: If the leader is stationary, their forward velocity collapses.
    // In this state, we fallback to a standard unit heading to prevent matrix collapse.
    if (speedSq < 1e-10) {
        // Assume resting heading is pointing down the universal Z-axis
        forward = Vector3d(0.0, 0.0, 1.0);
    } else {
        forward.Normalize(); // Isolate pure directional unit vector
    }

    // 1. Compute local "Right" vector (+X axis) using a standard sky fallback (+Y)
    Vector3d right(0.0, 1.0, 0.0);

    // Safety check: If the ship is flying completely straight up or down,
    // the cross-product collapses. Shift the fallback axis to break the singularity.
    if (std::abs(forward.dotProduct(right)) > 0.99)
        right = Vector3d(1.0, 0.0, 0.0);

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


/***********
 * warp methods
 */

void BallPark::InitWarp(MoveData* mData) {
    // these are the only vars that need to be set here right now
    mData->posHack = false;

    // warp time and distance math
    //   allan 1Nov14 - 14Nov14
    //  rewrite 3jan15  to use distance instead of time for warping.  more accurate now, and covers ALL distances.
    //  calculation and implementation update   9Jan15      accuracy is within 1000m
    //  major destiny movement update/rewrite - allan Feb23  (wip)
    //  warp update complete.  MOE ~1-2m, scaling with ship size  -allan 7Mar23   **UPDATE**  this wasnt right.
    //  warp rewrite.  finally corrected deviations.  all ships within 12m   -allan 21Jan25
    // complete update/rewrite for new BallPark class  -allan 20Nov26

    // ONE_AU_IN_METERS = 149597870700

    //init warp:
    SpaceEntity* pSE = mData->mySE;
    if (is_log_enabled(DESTINY__WARP_TRACE))
        _log(DESTINY__WARP_TRACE, "Destiny::InitWarp(): %s(%u) is initializing warp.", pSE->GetName(), pSE->GetID());

    // Crucible (pre-Rubicon) fixed warp constants: ACC = 3.0, DEC = 1.0
    const double D = static_cast<double>(mData->targetDistance);
    double S = static_cast<double>(mData->warpSpeed) * ONE_AU_IN_METERS;

    // Short-warp cap: if D < (4/3)*S, lower peak warp speed S so accel + decel == D with 0 cruise
    S = std::min(S, (D + 1.0) * 0.75);

    const double accelDuration  = std::log(S / 3.0) / 3.0;
    const double accelDistance  = S / 3.0;
    const double decelDuration  = std::log(S);
    const double decelDistance  = S - 1.0;
    const double cruiseDistance = std::max(0.0, D - accelDistance - decelDistance);
    const double cruiseDuration = (S > 0.0) ? (cruiseDistance / S) : 0.0;

    mData->warpState.warpDistance  = mData->targetDistance;
    mData->warpState.warpSpeed     = static_cast<int64>(S);
    mData->warpState.accelDist     = static_cast<int64>(accelDistance);
    mData->warpState.cruiseDist    = static_cast<int64>(cruiseDistance);
    mData->warpState.decelDist     = static_cast<int64>(decelDistance);
    mData->warpState.accelTime     = static_cast<float>(accelDuration);
    mData->warpState.cruiseTime    = static_cast<float>(cruiseDuration);
    mData->warpState.decelTime     = static_cast<float>(decelDuration);
    mData->warpState.warpTime      = static_cast<float>(accelDuration + cruiseDuration + decelDuration);

    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        _log(DESTINY__WARP_TRACE, "BallPark::InitWarp():Calculate - %s(%u): Warp will accelerate for %0.1fs, cruise for %0.1fs, then decelerate for %0.1fs, with total time of %0.1fs and warp speed of %lli m/s.", \
                pSE->GetName(), pSE->GetID(), mData->warpState.accelTime, mData->warpState.cruiseTime, mData->warpState.decelTime, mData->warpState.warpTime, mData->warpState.warpSpeed);
        _log(DESTINY__WARP_TRACE, "BallPark::InitWarp():Calculate - Accel distance is %lli  Cruise distance is %lli   Decel distance is %lli", \
                mData->warpState.accelDist, mData->warpState.cruiseDist, mData->warpState.decelDist);
        _log(DESTINY__WARP_TRACE, "BallPark::InitWarp():Calculate - We will exit warp at %0.1f,%0.1f,%0.1f at a distance of %lli AU (%lli m).", \
                mData->targetPoint.x, mData->targetPoint.y, mData->targetPoint.z, mData->targetDistance / ONE_AU_IN_METERS, mData->targetDistance);
    }

    // check for player warp
    if (pSE->HasPilot()) {
        //turn off non warp-safe modules
        pSE->GetShipSE()->Warp();
        //drain cap
        pSE->GetSelf()->SetAttribute(AttrCapacitorCharge, mData->warpCapNeed);
        // reset warp cap need to base
        if (pSE->GetSelf()->HasAttribute(AttrWarpCapacitorNeed)) {
            mData->warpCapNeed = pSE->GetSelf()->GetAttribute(AttrWarpCapacitorNeed).get_double();
        } else {
            mData->warpCapNeed = 0.000000138;   // lowest value in db
        }
    }
    // do npcs need to notify ai of warping?   sometimes, yes.

    //clear targets
    pSE->TargetMgr()->ClearAllTargets();
    //pSE->TargetMgr()->OnTarget(nullptr, TargMgr::Mode::Clear, TargMgr::Msg::WarpingOut);

    mData->targetSE = nullptr;

    // send warp gfx
    if (pSE->SysBubble()->HasPlayers())
        pSE->SystemMgr()->SendGFX10(pSE, "effects.Warping");

    //TODO:  determine if this ship has assigned drones in space and call drone:shipwarping

    // reset move times
    mData->userSpeedFraction    = 0.0f;
    mData->activeSpeedFraction  = 0.0f;
    mData->warpState.startTic   = pSE->SystemMgr()->GetTicCount();

    // all ships will completely stop before warp
    mData->myVelocity = NULL_ORIGIN;
    mData->warpState.active = true;
    mData->warpState.phase = WarpState::Phase::Accel;
    WarpAccel(mData, 0);
}

void BallPark::WarpAccel(MoveData* mData, double sec_into_warp) {
    const double t = static_cast<double>(sec_into_warp);
    const double accelDur = static_cast<double>(mData->warpState.accelTime);

    if (t >= accelDur) {
        if (mData->warpState.cruiseDist > 0) {
            mData->warpState.phase = WarpState::Phase::Cruise;
            WarpCruise(mData, sec_into_warp);
        } else {
            mData->warpState.phase = WarpState::Phase::Decel;
            WarpDecel(mData, sec_into_warp);
        }
        return;
    }

    // x(t) = exp(3 * t), v(t) = 3 * exp(3 * t)
    const double traveled = std::exp(3.0 * t);
    const int64 currentShipSpeed = static_cast<int64>(3.0 * traveled);
    const double remaining = static_cast<double>(mData->warpState.warpDistance) - traveled;

    mData->targetDistance = static_cast<int64>(std::max(0.0, remaining));
    mData->myVelocity = mData->desiredHeading * static_cast<double>(currentShipSpeed);
    mData->mySE->SetPosition(mData->targetPoint - (mData->desiredHeading * remaining));

    WarpUpdate(mData, currentShipSpeed, sec_into_warp, WarpState::Phase::Accel);

    if (mData->mySE->SysBubble() != nullptr) {
        // check actual bubble center here
        if (!mData->mySE->SysBubble()->InBubble(mData->mySE->GetPosition())) {
            if (is_log_enabled(DESTINY__WARP_TRACE))
                _log(DESTINY__WARP_TRACE, "BallPark::WarpAccel(): %s(%u) is being removed from bubble %u.", \
                        mData->mySE->GetName(), mData->mySE->GetID(), mData->mySE->SysBubble()->GetID());
            mData->mySE->SysBubble()->Remove(mData->mySE);
        }
    }
}

void BallPark::WarpCruise(MoveData* mData, double sec_into_warp) {
    // in cruise...update position data and check for decel
    const double tCruise = sec_into_warp - mData->warpState.accelTime;
    const double cruiseDur = mData->warpState.cruiseTime;

    if (tCruise >= cruiseDur) {
        mData->warpState.phase = WarpState::Phase::Decel;
        WarpDecel(mData, sec_into_warp);
        return;
    }

    const double S = static_cast<double>(mData->warpState.warpSpeed);
    const double traveled = static_cast<double>(mData->warpState.accelDist) + (S * tCruise);
    const double remaining = static_cast<double>(mData->warpState.warpDistance) - traveled;

    mData->targetDistance = static_cast<int64>(std::max(0.0, remaining));
    mData->myVelocity = mData->desiredHeading * S;
    mData->mySE->SetPosition(mData->targetPoint - (mData->desiredHeading * remaining));

    WarpUpdate(mData, S, sec_into_warp, WarpState::Phase::Cruise);
}

void BallPark::WarpDecel(MoveData* mData, double sec_into_warp) {
    const double tDecel = static_cast<double>(sec_into_warp)
    - static_cast<double>(mData->warpState.accelTime + mData->warpState.cruiseTime);
    const double t = std::max(0.0, tDecel);
    const double S = static_cast<double>(mData->warpState.warpSpeed);

    // v(t) = S * exp(-t), remaining decel distance = S * exp(-t)
    const double currentShipSpeed = S * std::exp(-t);
    const double remaining = currentShipSpeed;

    mData->targetDistance = static_cast<int64>(std::max(0.0, remaining));
    mData->myVelocity = mData->desiredHeading * currentShipSpeed;
    mData->mySE->SetPosition(mData->targetPoint - (mData->desiredHeading * remaining));

    if (mData->mySE->SysBubble() == nullptr) {
        if (mData->warpState.targBubble->InBubble(mData->mySE->GetPosition(), true)) {   // check modified bubble center here
            if (is_log_enabled(DESTINY__WARP_TRACE))
                _log(DESTINY__WARP_TRACE, "BallPark::WarpDecel(): %s(%u) is being added to bubble %u.", \
                    mData->mySE->GetName(), mData->mySE->GetID(), mData->warpState.targBubble->GetID());
            mData->warpState.targBubble->Add(mData->mySE);
        }
    }

    WarpUpdate(mData, currentShipSpeed, sec_into_warp, WarpState::Phase::Decel);

    // Drop out of warp when speed falls below client threshold (min(0.5 * maxVel, 100.0) or speedToLeaveWarp)
    if (currentShipSpeed <= mData->speedToLeaveWarp) {
        mData->mySE->SetPosition(mData->targetPoint);
        mData->targetDistance = 0;
        WarpStop(mData, static_cast<int64>(currentShipSpeed));
    }
}

void BallPark::WarpUpdate(MoveData* mData, int64 currentShipSpeed, double sec_into_warp, uint8 phase) {
    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        switch (phase) {
            case WarpState::Phase::Accel: {
                _log(DESTINY__WARP_TRACE, "BallPark::WarpAccel(): %s(%u) - Warp Accelerating(%us): velocity %lli m/s.   %lli m remaining.", \
                        mData->mySE->GetName(), mData->mySE->GetID(), sec_into_warp, currentShipSpeed, mData->targetDistance);
            } break;
            case WarpState::Phase::Cruise: {
                _log(DESTINY__WARP_TRACE, "BallPark::WarpCruise(): %s(%u) - Warp Cruising(%us): velocity %lli m/s.  %lli m remaining.", \
                        mData->mySE->GetName(), mData->mySE->GetID(), sec_into_warp, currentShipSpeed, mData->targetDistance);
            } break;
            case WarpState::Phase::Decel: {
                _log(DESTINY__WARP_TRACE, "BallPark::WarpDecel(): %s(%u) - Warp Decelerating(%us): velocity %0.2f,%0.2f,%0.2f, speed %lli m/s.  %lli m remaining.", \
                        mData->mySE->GetName(), mData->mySE->GetID(), sec_into_warp, mData->myVelocity.x, mData->myVelocity.y, mData->myVelocity.z, \
                        currentShipSpeed, mData->targetDistance);
            } break;
            default: {
                _log(DESTINY__WARNING, "BallPark::WarpUpdate()  %s(%u): Called with invalid type.", \
                        mData->mySE->GetName(), mData->mySE->GetID());
            }
        }

        Vector3d delta = mData->targetPoint - mData->mySE->GetPosition();
        double dist = delta.Length();
        _log(DESTINY__WARP_TRACE, "BallPark::WarpUpdate()  %s(%u): Ship is %0.1f from target in bubble %u.", \
                mData->mySE->GetName(), mData->mySE->GetID(), dist, mData->warpState.targBubble->GetID());
    }
}

void BallPark::WarpStop(MoveData* mData, int64 currentShipSpeed) {
    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        const Vector3d& pos = mData->mySE->GetPosition();
        _log(DESTINY__WARP_TRACE, "BallPark::WarpStop(): %s(%u) - Warp complete. Exit velocity %lli m/s with %lli left to go.", \
                mData->mySE->GetName(), mData->mySE->GetID(), currentShipSpeed, mData->targetDistance);
        _log(DESTINY__WARP_TRACE, "BallPark::WarpStop(): Ship currently at %0.2f,%0.2f,%0.2f.", \
                pos.x, pos.y, pos.z);
    }

    mData->warpState.Reset();

    // is this needed?  yes, for proper sub-warp movement tracking
    mData->targetPoint += (mData->desiredHeading * 5000.0);
    mData->ballMode = Ball::Mode::STOP;
    mData->ticStamp = mData->mySE->SystemMgr()->GetTicCount();

    // warp not started yet; allow stop
    if (mData->mySE->GetSelf()->HasAttribute(AttrWarpCapacitorNeed)) {
        mData->warpCapNeed = mData->mySE->GetSelf()->GetAttribute(AttrWarpCapacitorNeed).get_double();
    } else {
        mData->warpCapNeed = 0.000000138;   // lowest value in db
    }

    if ((mData->mySE->IsNPCSE()) and (mData->mySE->GetNPCSE()->GetAI() != nullptr))
        mData->mySE->GetNPCSE()->GetAI()->WarpOutComplete();

    // reset bump checks
    if (sConfig.cosmic.BumpEnabled) {
        SetBallMassive sbmassive;
        sbmassive.entityID = mData->mySE->GetID();
            sbmassive.is_massive = 1;
        PyTuple *up = sbmassive.Encode();
        mData->mySE->SystemMgr()->SendSingleUpdate(mData->mySE, &up);
        PyDecRef(up);
    }
}

// Basic Movement Call:
void BallPark::BeginMovement(MoveData* mData) {
    SpaceEntity* pSE = mData->mySE;
    // common movement for all types
    if (!mData->sentUpdates
    and (pSE->HasPilot() or pSE->SysBubble()->HasPlayers())) {
        // error fix for setting ship movement variables before ship is in bubble (cannot BubbleCast)
        std::vector<PyTuple*> updates;
        SetBallAgility sbagility;
            sbagility.entityID =  pSE->GetID();
            sbagility.agility = pSE->GetSelf()->GetAttribute(AttrInertiaMod).get_double();
        updates.push_back(sbagility.Encode());
        if (sConfig.cosmic.BumpEnabled) {
            SetBallMassive sbmassive;
                sbmassive.entityID = pSE->GetID();
                sbmassive.is_massive = 1;
            updates.push_back(sbmassive.Encode());
        }
        SetBallMass sbmass;
            sbmass.entityID = pSE->GetID();
            sbmass.mass = pSE->GetSelf()->mass();
        updates.push_back(sbmass.Encode());
        pSE->SystemMgr()->SendDestinyUpdates(pSE, updates); //consumed
        mData->sentUpdates = true;
    }

    if (pSE->IsShipSE())
        pSE->GetShipSE()->UnCloak();

    mData->stop = false;
    mData->skipTic = false;
    mData->moveDelay = true;

    // reset move stamp
    mData->timeStamp = GetFileTimeNow();

    // distance check (per client)
    // Python gateway default constant backup check (0x44fa0000 = 2000m)
    if ((mData->ballMode != Ball::Mode::WARP) and (mData->desiredDistance < 1))
        mData->desiredDistance = 2000;

    // if usf > 0.1 its' probably set from the speedo; leave it.
    if (mData->userSpeedFraction < 0.1f)
        BallPark::SetSpeedFraction(mData, 1.0f);

    if (is_log_enabled(DESTINY__MOVE_TRACE)) {
        if (mData->targetSE != nullptr) {
            SpaceEntity* pSE = mData->targetSE;
            _log(DESTINY__MOVE_TRACE, "target (%s) position: %0.2f, %0.2f, %0.2f", \
                    pSE->GetName(), pSE->GetPosition().x, pSE->GetPosition().y, pSE->GetPosition().z);
        }
        _log(DESTINY__MOVE_TRACE, " currentVelocity: %0.3f,%0.3f,%0.3f", \
                mData->myVelocity.x, mData->myVelocity.y, mData->myVelocity.z);
    }
}

/*********************************************
 *   Beyonce calls
 */

void BallPark::AlignTo(MoveData* mData) {
    // should this Stop() once alignment has been achieved?  i'd say yes.  config option?
    // i originally set it like this, but aknor didnt like it, so it was removed
    mData->alignTo = true;
    GotoPoint(mData);
}

void BallPark::ApproachBall(MoveData* mData) {
    // use this for dock and jump where dist > interact dist
    mData->desiredDistance = mData->mySE->GetRadius() + mData->targetSE->GetRadius() + 1000;
    if (mData->desiredDistance > 2500)
        mData->desiredDistance = 2000;

    GotoPoint(mData);
}

void BallPark::GotoDirection(MoveData* mData) {
    mData->ballMode = Ball::Mode::GOTO;
    BeginMovement(mData);

    CmdGotoDirection du;
        du.entityID = mData->mySE->GetID();
        du.x = mData->desiredHeading.x;
        du.y = mData->desiredHeading.y;
        du.z = mData->desiredHeading.z;
    PyTuple* up = du.Encode();
    mData->mySE->SystemMgr()->SendSingleUpdate(mData->mySE, &up);
    PyDecRef(up);
}

void BallPark::GotoPoint(MoveData* mData) {
    mData->ballMode = Ball::Mode::GOTO;
    BeginMovement(mData);

    CmdGotoPoint du;
        du.entityID = mData->mySE->GetID();
        du.x = mData->targetPoint.x;
        du.y = mData->targetPoint.y;
        du.z = mData->targetPoint.z;
    PyTuple* up = du.Encode();
    mData->mySE->SystemMgr()->SendSingleUpdate(mData->mySE, &up);
    PyDecRef(up);
}

void BallPark::FollowBall(MoveData* mData) {
    //called from client as 'CmdFollowBall'
    //  also used by 'Approach'
    mData->ballMode = Ball::Mode::FOLLOW;
    BeginMovement(mData);

    CmdFollowBall du;
        du.entityID = mData->mySE->GetID();
        du.targetID = mData->targetSE->GetID();
        du.range = mData->desiredDistance;
    PyTuple *up = du.Encode();
    mData->mySE->SystemMgr()->SendSingleUpdate(mData->mySE, &up);
    PyDecRef(up);
}

void BallPark::OrbitBall(MoveData* mData) {
    if (!sConfig.debug.UseOrbit) {
        Stop(mData);
        return;
    }

    mData->ballMode = Ball::Mode::ORBIT;
    BeginMovement(mData);

    CmdOrbit du;
        du.entityID = mData->mySE->GetID();
        du.orbitEntityID = mData->targetSE->GetID();
        du.distance = mData->desiredDistance;
    PyTuple *up = du.Encode();
    mData->mySE->SystemMgr()->SendSingleUpdate(mData->mySE, &up);
    PyDecRef(up);
}

// Fleet warps - on live, all ships use the warp profile of the slowest ship, here they warp per ship data
//  looks cool as shit to have larger ships slide in after smaller ships
void BallPark::WarpTo(MoveData* mData, bool autoPilot) {
    assert(mData->warpState.targBubble != nullptr);
    /* pc warp order..
     * pick destination -> align/accel -> aura "warp drive active" -> cap drain -> accel
     *      -> enter warp -> warp -> decel -> leave warp -> coast -> stop
     */
    SpaceEntity* mySE = mData->mySE;
    mData->ballMode = Ball::Mode::WARP;
    BeginMovement(mData);

    // if usf < 75%, set usf for warp
    if (mData->userSpeedFraction < 0.751)
        BallPark::SetSpeedFraction(mData, 1.0f);

    //set massive for warp.   self-only for players per client logs
    // if we do this before align, ships will 'pass thru' obstacles
    if (mySE->HasPilot()) {
        mData->massive = false;
        SetBallMassive sbmassive;
            sbmassive.entityID = mySE->GetID();
            sbmassive.is_massive = 0;       // disable client-side bump checks
        PyTuple *up = sbmassive.Encode();
        mySE->SystemMgr()->SendSingleUpdate(mySE, &up, true);
        PyDecRef(up);
    }

    // if no players in bubble, this isnt needed...
    if (mySE->SysBubble()->HasPlayers()) {
       // std::vector<PyTuple*> up;
        // send warp update
        CmdWarpTo wt;
            wt.entityID = mySE->GetID();
            wt.dest_x = mData->targetPoint.x;
            wt.dest_y = mData->targetPoint.y;
            wt.dest_z = mData->targetPoint.z;
            wt.distance = mData->desiredDistance;
            wt.warpSpeed = mData->warpSpeed * 10;
        //up.push_back(wt.Encode());
        PyTuple *up = wt.Encode();
        mySE->SystemMgr()->SendSingleUpdate(mySE, &up); //consumed
    }

    if (is_log_enabled(DESTINY__WARP_TRACE))
        _log(DESTINY__WARP_TRACE, "BallPark::WarpTo() %u -> %u, mData->targetPoint: %0.2f,%0.2f,%0.2f  stop distance: %i  mData->targetDistance: %lli", \
             mySE->SysBubble()->GetID(), mData->warpState.targBubble->GetID(), mData->targetPoint.x, mData->targetPoint.y, mData->targetPoint.z, mData->desiredDistance, mData->targetDistance);
}

bool BallPark::IsAligned(MoveData* mData) {
    double distanceSq = mData->myVelocity.LengthSq();
    if (distanceSq <  VISUAL_STOP_THRESHOLD_SQ) {
        // if ship is stopped enough, there is no 'aligning' to do
        return true;
    }

    double dot = mData->myVelocity.dotProduct(mData->desiredHeading);
    //double degrees = EvE::Trig::Rad2Deg(std::acos(dot));
    if (mData->ballMode == Ball::Mode::WARP) {
        if (dot > /*WARP_ALIGNMENT 0.99449256*/ 0.99556405)
            return true;
    } else if (dot > /*TURN_ALIGNMENT*/ 0.99756405) {
        return true;
    }
    return false;
}

void BallPark::ApplyVortexGravityTug(ShipSE* pShipSE, const Vector3d& vortexCenterAxis) {
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
        if (pShipSE->SystemMgr()->GetTicCount() % 100 == 0)  // Telemetry alert check interval
            pShipSE->GetPilot()->SendNotifyMsg("NEOCOM ALERT: Odd local spatial vortex detected... ship drift active.");
    }
}

/* Ship Calls */
// move this to system
void BallPark::SetSpeedFraction(MoveData* mData, float fraction) {
    SpaceEntity* pSE = mData->mySE;

    if (is_log_enabled(DESTINY__MOVE_TRACE))
        _log(DESTINY__MOVE_TRACE, "BallPark::SetSpeedFraction() - %s(%u): from %0.2f to %0.2f.", \
                pSE->GetName(), pSE->GetID(), mData->userSpeedFraction, fraction);

    mData->userSpeedFraction = fraction;

    if (pSE->HasPilot() or pSE->SysBubble()->HasPlayers()) {
        std::vector<PyTuple*> updates;
        CmdSetSpeedFraction du;
            du.entityID = pSE->GetID();
            du.fraction = fraction;
        updates.push_back(du.Encode());

        if (pSE->IsNPCSE() or pSE->IsMissileSE() or pSE->IsContainerSE() or pSE->IsWreckSE()) {
            SetBallSpeed ms;
                ms.entityID = pSE->GetID();
                ms.speed = mData->maxSpeed;
            updates.push_back(ms.Encode());
            mData->sentUpdates = true;
        }

        if (!updates.empty())
            pSE->SystemMgr()->SendDestinyUpdates(pSE, updates);
    }
}

void BallPark::Stop(MoveData* mData) {
    // if ship is in warp;  cannot cancel till warp drops
    if (mData->warpState.active)
        return;

    SpaceEntity* mySE = mData->mySE;
    if (is_log_enabled(DESTINY__MOVE_TRACE))
        sLog.Warning("BallPark", "%s calling stop", mySE->GetName());

    if (mData->ballMode == Ball::Mode::WARP) {
        /*  ballmode is set in WarpTo();  warpstate is set in InitWarp()
         * ship may not actually be in warp yet if align/speed isnt right
         * in this case, player wants to cancel warp before it begins
         * for this, we need to send a stop packet after stop vars are set.
         *  however,   sending stop packet while in warp does funny things...dont send it.
         */
        // warp not started yet; allow stop
        if (mySE->GetSelf()->HasAttribute(AttrWarpCapacitorNeed)) {
            mData->warpCapNeed = mySE->GetSelf()->GetAttribute(AttrWarpCapacitorNeed).get_double();
        } else {
            mData->warpCapNeed = 0.000000138;   // lowest value in db
        }

        /* set marker for calc'd stop distance (testing)
        if (is_log_enabled(DESTINY__WARP_DEBUG)) {
            Vector3d& pos = mySE->GetPositionRef();
            uint16 dist = mData->maxSpeed * mData->activeSpeedFraction * mData->agility;
            Vector3d offset = (pos * dist);
            Vector3d marker = (pos + offset);
            std::string str = "Stop Point - ";
            str += mySE->GetName();
            mySE->SystemMgr()->MarkPoint(marker, str, str);
        } */
        return;
    }

    if (mData->ballMode != Ball::Mode::STOP) {
        mData->ballMode = Ball::Mode::STOP;
        CmdStop du;
            du.entityID = mySE->GetID();
        PyTuple *up = du.Encode();
        mySE->SystemMgr()->SendSingleUpdate(mySE, &up);
        PyDecRef(up);
    }

    mData->Clear();

    SetSpeedFraction(mData, 0.0f);
}

// calling this will set object to a COMPLETE and IMMEDIATE stop, update STATE and remove from system ticlist
void BallPark::Halt(MoveData* mData, bool commanded/*false*/) {
    if (mData->ballMode == Ball::Mode::RIGID)
        return;
    SpaceEntity* mySE = mData->mySE;
    if (is_log_enabled(DESTINY__MOVE_TRACE))
        _log(DESTINY__MOVE_TRACE, "BallPark::Halt(%s) - %s(%u) Halted", \
        (commanded ? "true" : "false"), mySE->GetName(), mySE->GetID());

    if (commanded) {
        // immediate halt via command.  send packet to stop ship.
        CmdStop du;
            du.entityID = mySE->GetID();
        PyTuple *up = du.Encode();
        mySE->SystemMgr()->SendSingleUpdate(mySE, &up);
    }

    //  reset ALL movement variables and states.
    mData->Clear();
    mData->ballMode = Ball::Mode::RIGID;
}

// there may be more here im missing.  we can also target SEs for dock, jump, etc
bool BallPark::ValidTarget(MoveData* mData) {
    SpaceEntity* targetSE = mData->GetTargetSE();
    if (__builtin_expect(targetSE == nullptr, 0))
        return false;
    if (targetSE->IsStationSE() or targetSE->IsGateSE() or targetSE->IsBeltSE()
        or targetSE->IsPOSSE() or targetSE->IsItemEntity() or targetSE->IsObjectEntity())
        return true;

    if (targetSE->IsDead() or targetSE->IsInvul() or targetSE->IsFrozen())
        return false;

    if (targetSE->IsStaticEntity())
        return false;
    if (targetSE->HasPilot() and targetSE->GetPilot()->IsDocked())
        return false;

    MoveData* tMD = targetSE->GetMoveData();
    if ((tMD == nullptr) or (tMD->IsWarping()))
        return false;

    return true;
}

// bump mechanics....
struct BumpProxy {
    SpaceEntity* se;
    double x, y, z;
    double radius;
};

void BallPark::ProcessBubbleBumps(SystemBubble* bubble) {
    // 1. Gather massive, non-warping entities into a lightweight list
    static std::vector<BumpProxy> proxies; // static avoids reallocating heap memory every tic!
    proxies.clear();

    double maxRadius = 0.0;
    // Loop bubble's entity map
    std::map<uint32, SpaceEntity*> objMap;
    bubble->GetEntities(objMap);
    for (auto const& pair : objMap) {
        SpaceEntity* pSE = pair.second;

        const Vector3d& pos = pSE->GetPosition();
        double rad = pSE->GetRadius();
        if (rad > maxRadius)
            maxRadius = rad;

        proxies.push_back({ pSE, pos.x, pos.y, pos.z, rad });
    }

    // Need at least 2 massive objects to bump
    const size_t count = proxies.size();
    if (count < 2)
        return;

    // 2. Sort from lowest X to highest X
    std::sort(proxies.begin(), proxies.end(), [](const BumpProxy& a, const BumpProxy& b) {
        return a.x < b.x;
    });

    double BUMP_DISTANCE = 100.0;
    // 3. Sweep and Prune
    for (size_t i = 0; i < count - 1; ++i) {
        const BumpProxy& a = proxies[i];
        // Max possible X distance anything could touch 'a' is a's radius + largest object in bubble + buffer
        const double maxReachX = a.radius + maxRadius + BUMP_DISTANCE;

        for (size_t j = i + 1; j < count; ++j) {
            const BumpProxy& b = proxies[j];
            const double dx = b.x - a.x;

            // PRUNE: Since list is sorted by X, if 'b' is too far right, ALL remaining items are too far!
            if (dx > maxReachX)
                break;

            // Exact combined touch threshold for this specific pair
            const double touchDist = a.radius + b.radius + BUMP_DISTANCE;
            if (dx > touchDist)
                continue;

            // Quick Y and Z bounding box checks (avoids multiplication if separated vertically/depth-wise)
            const double dy = b.y - a.y;
            if (std::abs(dy) > touchDist)
                continue;

            const double dz = b.z - a.z;
            if (std::abs(dz) > touchDist)
                continue;

            // Final 3D Sphere Check (Squared distance avoids expensive sqrt())
            const double distSq = (dx * dx) + (dy * dy) + (dz * dz);
            if (distSq <= (touchDist * touchDist)) {
                // Confirmed touch! Call your bounce physics method once for this pair
                BallPark::Bump(a.se, b.se);
            }
        }
    }
}

void BallPark::Bump(SpaceEntity* mySE, SpaceEntity* herSE) {
    MoveData* mdA = mySE->GetMoveData();
    MoveData* mdB = herSE->GetMoveData();

    const bool staticA = (mdA == nullptr && !mySE->IsWreckSE() && !mySE->IsContainerSE());
    const bool staticB = (mdB == nullptr && !herSE->IsWreckSE() && !herSE->IsContainerSE());
    if (staticA && staticB)
        return;

    // Lazy-init MoveData for bumped unanchored wrecks/cans
    if (!staticA && mdA == nullptr) {
        mySE->CreateMoveData();
        mdA = mySE->GetMoveData();
        mdA->ballMode = Ball::Mode::STOP;
    }
    if (!staticB && mdB == nullptr) {
        herSE->CreateMoveData();
        mdB = herSE->GetMoveData();
        mdB->ballMode = Ball::Mode::STOP;
    }

    Vector3d& posA = mySE->GetPositionRef();
    Vector3d& posB = herSE->GetPositionRef();

    // 1. Collision normal using your LengthSq() to avoid double sqrt()
    Vector3d normal = posB - posA;
    double distSq = normal.LengthSq();
    double dist = 0.0;
    if (distSq < 0.000001) {
        normal = Vector3d(1.0, 0.0, 0.0);
        dist = 0.001;
    } else {
        dist = std::sqrt(distSq);
        normal *= (1.0 / dist);
    }

    // 2. Relative velocity along collision normal using your dotProduct()
    Vector3d velA = staticA ? NULL_ORIGIN : mdA->myVelocity;
    Vector3d velB = staticB ? NULL_ORIGIN : mdB->myVelocity;
    Vector3d delta = velA - velB;
    double relVel = delta.dotProduct(normal);

    // Already separating
    if (relVel <= 0.0)
        return;

    // 3. Santorine elastic impulse
    double invMassA = staticA ? 0.0 : (1.0 / mySE->GetSelf()->mass());
    double invMassB = staticB ? 0.0 : (1.0 / herSE->GetSelf()->mass());
    double invMassSum = invMassA + invMassB;
    if (invMassSum <= 0.0)
        return;

    // elasticity can change for 'better' mechanics/movement
    const double elasticity = 1.0;
    double impulse = ((1.0 + elasticity) * relVel) / invMassSum;

    // 4. Update velocities in-place
    if (!staticA) {
        mdA->myVelocity -= normal * (impulse * invMassA);
        if (mdA->ballMode == Ball::Mode::STOP)
            mdA->desiredVelocity = NULL_ORIGIN;
    }
    if (!staticB) {
        mdB->myVelocity += normal * (impulse * invMassB);
        if (mdB->ballMode == Ball::Mode::STOP)
            mdB->desiredVelocity = NULL_ORIGIN;
    }

    // 5. Separate overlapping spheres
    double overlap = (mySE->GetRadius() + herSE->GetRadius()) - dist;
    if (overlap > 0.0) {
        if (!staticA && !staticB) {
            posA.x -= normal.x * (overlap * 0.5);
            posA.y -= normal.y * (overlap * 0.5);
            posA.z -= normal.z * (overlap * 0.5);
            posB.x += normal.x * (overlap * 0.5);
            posB.y += normal.y * (overlap * 0.5);
            posB.z += normal.z * (overlap * 0.5);
        } else if (!staticA) {
            posA.x -= normal.x * overlap;
            posA.y -= normal.y * overlap;
            posA.z -= normal.z * overlap;
        } else if (!staticB) {
            posB.x += normal.x * overlap;
            posB.y += normal.y * overlap;
            posB.z += normal.z * overlap;
        }
    }
}
