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
void BallPark::Process(SpaceEntity* pSE) {
    if (pSE == nullptr)
        return;
    MoveData* mData = pSE->GetMoveData();
    if (mData == nullptr)
        return;
    if (mData->moveDelay) {
        mData->moveDelay = false;
        return;
    }

    const double profileStartTime = (sConfig.debug.UseProfiling ? GetTimeUSeconds() : 0.0);
    using namespace Ball::Mode;
    switch(mData->ballMode) {
        case RIGID:
        case MISSILE: {
            // NOTE:  this should not hit.
            _log(DESTINY__ERROR, "Destiny::Process() Error! - %s(%u) is proc with state=%s.", \
                    pSE->GetName(), pSE->GetID(), sDataMgr.GetBallModeName(mData->ballMode));
            return;
        } break;
        case STOP: {
            // Apply vertical pitch dampener directly to velocity
            mData->myVelocity.y *= 0.8691588785046729;
        } break;
        case GOTO: {
            // there's not necessarily a target here...just a direction to point in space.
            if (mData->alignTo and IsAligned(mData, mData->desiredHeading)) {
                if (is_log_enabled(DESTINY__MOVE_TRACE)) {
                    sLog.Warning("BallPark::Process(Goto)", "%s is aligned.", pSE->GetName());
                }

                Stop(pSE);
                break;
            }

            if (pSE->HasPilot() and pSE->GetSelf()->GetShipItem()->IsUndocking())
                break;

            GotoVelocity(mData); // Includes exponential homing zone
        } break;
        case FOLLOW: {
            if (!ValidTarget(pSE, mData->targSE)) {
                if (is_log_enabled(DESTINY__MOVE_TRACE))
                    sLog.Warning("BallPark::Process(Follow)", "%s's target is invalid", pSE->GetName());
                Stop(pSE);
                break;
            }

            CalculateFollowPoint(mData);
            GotoVelocity(mData);
        } break;
        case FORMATION: {
            if (!ValidTarget(pSE, mData->targSE)) {
                Stop(pSE);
                return;
            }
            CalculateFormationPoint(mData);
            GotoVelocity(mData);
        } break;
        case ORBIT: {
            if (!ValidTarget(pSE, mData->targSE)) {
                Stop(pSE);
                return;
            }

            OrbitVelocity(mData);
        } break;
        case WARP: {
            if (mData->warpState.active) {
                //warp is in progress
                uint16 sec_into_warp = (pSE->SystemMgr()->GetTicCount() - mData->ticStamp);
                //  speed and distance formulas based on current warp distance
                switch (mData->warpState.phase) {
                    case WarpState::Phase::Accel: {
                        WarpAccel(mData, sec_into_warp);
                    } break;
                    case WarpState::Phase::Cruise: {
                        WarpCruise(mData, sec_into_warp);
                    } break;
                    case WarpState::Phase::Decel: {
                        WarpDecel(mData, sec_into_warp);
                    } break;
                    case WarpState::Phase::None: {
                        // uh, Houston...we have a problem...
                        if (pSE->HasPilot()) {
                            _log(DESTINY__ERROR, "Destiny::Process() Error!  Ship %s(%u) for Player %s(%u) Has WarpState but checks are false.",  \
                                pSE->GetName(), pSE->GetID(), pSE->GetPilot()->GetName(), pSE->GetPilot()->GetCharacterID());
                                pSE->GetPilot()->SendErrorMsg("Internal Server Error.<br> Please Dock, Jump or Relog to reset your ship.");
                        } else {
                            _log(DESTINY__ERROR, "Destiny::Process() Error!  NPC %s(%u) Has WarpState but checks are false.",  \
                                pSE->GetName(), pSE->GetID());
                            WarpStop(0);
                            // check for ai and spawnMgr and notify accordingly
                            if (sConfig.debug.UseProfiling)
                                sProfiler.AddTime(Profile::destiny, GetTimeUSeconds() - profileStartTime);
                            return;
                        }
                    } break;

                }

                if (sConfig.debug.UseProfiling)
                    sProfiler.AddTime(Profile::destiny, GetTimeUSeconds() - profileStartTime);
                return;
            }

            // check speed...if all good, call InitWarp()
            if (IsAligned(mData->desiredHeading) and (mData->activeSpeedFraction > 0.749999f)) {
                InitWarp(mData);
                if (sConfig.debug.UseProfiling)
                    sProfiler.AddTime(Profile::destiny, GetTimeUSeconds() - profileStartTime);
                return;
            }
            //TODO:  put check here for stuck ship where (tic - stamp > stucktime)

            GotoVelocity(mData);
        } break;
        case TROLL: {
            // mobile object that will become rigid after a set time
            // jetcans/wrecks created from moving ship will take that velocity and continue movement for a time, then become rigid.
            //update.  ball is set <state_stop> and <mode_troll> with timestamp.
            //  check elapsed time here while object is moving, then switch to <mode_rigid> once stopped.
            // this elapsed check can/may change based on load in current bubble
            // 3. client uses 30-second window (Matches Ballpark Offset 0x7c)  may reduce this to 15-20s
            uint32 elapsed = pSE->SystemMgr()->GetTicCount() - mData->ticStamp;
            if ((elapsed >= sConfig.rates.DestinyTrollTime) or (mData->myVelocity.LengthSq() < 0.001)) {
                Halt();
                // Mutate state locally to a static rigid item...no, leave at stop to allow tractor
                mData->ballMode = Ball::Mode::STOP;
                mData->skipTic = true;
                // may not want to do this...will remove movement from tractor
                //pSE->SystemMgr()->RemoveTicEntity(pSE);
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
            _log(DESTINY__ERROR, "Destiny::Process() - %s(%u): hit default with state: %s",
                    pSE->GetName(), pSE->GetID(), sDataMgr.GetBallModeName(mData->ballMode));
        } break;
    }

    // integration step using optimized blending glide model
    Integrate(mData);

    // Global boundary parking check
    double speedSq = mData->myVelocity.LengthSq();
    if (mData->stop and (speedSq < 0.001f)) {
        Halt(pSE);
        return;
    }

    if (mData->maxSpeed > 0.01f)
        mData->activeSpeedFraction = std::sqrt(speedSq) / mData->maxSpeed;

    // only create tracking can when ship is moving significant amount
    if (sEntityMgr.GetTracking())
        pSE->SystemMgr()->CreateShipMarker(pSE);

    if (is_log_enabled(DESTINY__MOVE_TRACE)) {
        int32 timeDelta = pSE->SystemMgr()->GetTicCount() - mData->ticStamp;
        _log(DESTINY__MOVE_TRACE, "Destiny::%s(%u) - %s(%i): Current Ship Velocity: %0.2f, %0.2f, %0.2f @ asf: %0.3f (speed: %0.1f)", \
                sDataMgr.GetBallModeName(mData->ballMode), timeDelta, pSE->GetName(), pSE->GetID(), \
                mData->myVelocity.x, mData->myVelocity.y, mData->myVelocity.z, \
                mData->activeSpeedFraction, mData->myVelocity.Length());
        const Vector3d& position = pSE->GetPosition();
        _log(DESTINY__MOVE_TRACE, "Current position: %0.2f, %0.2f, %0.2f", \
                position.x, position.y, position.z);
        if (mData->targPoint.notZero())
            _log(DESTINY__MOVE_TRACE, "Ship target Point: %0.2f, %0.2f, %0.2f", \
                    mData->targPoint.x, mData->targPoint.y, mData->targPoint.z);
        if (mData->desiredVelocity.notZero())
            _log(DESTINY__MOVE_TRACE, "Ship target Velocity: %0.2f, %0.2f, %0.2f", \
                    mData->desiredVelocity.x, mData->desiredVelocity.y, mData->desiredVelocity.z);
    }

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::destiny, GetTimeUSeconds() - profileStartTime);
}

//Velocity setting method
// move to beyonce? or ship? will need this for npc also
void BallPark::SetSpeedFraction(float fraction/*1.0f*/) {
    if (is_log_enabled(DESTINY__MOVE_TRACE))
        _log(DESTINY__MOVE_TRACE, "Destiny::SetSpeedFraction() - %s(%u): from %0.2f to %0.2f.", \
                pSE->GetName(), pSE->GetID(), data->userSpeedFraction, fraction);

    data->userSpeedFraction = fraction;

    if (fraction and (pSE->HasPilot() or pSE->SysBubble()->HasPlayers())) {
        std::vector<PyTuple*> updates;
        CmdSetSpeedFraction du;
            du.entityID = pSE->GetID();
            du.fraction = fraction;
        updates.push_back(du.Encode());

        if (pSE->IsNPCSE() or pSE->IsMissileSE() or pSE->IsContainerSE() or pSE->IsWreckSE()) {
            SetBallSpeed ms;
                ms.entityID = pSE->GetID();
                ms.speed = data->maxSpeed;
            updates.push_back(ms.Encode());
            data->hasSentShipUpdates = true;
        }

        if (!updates.empty())
            SendDestinyUpdates(updates); //consumed
    }
}

//Global Actions:
// move to ship?
void BallPark::Stop(SpaceEntity* pSE) {
// is this right??
    if (pSE->HasPilot() and pSE->GetSelf()->GetShipItem()->IsUndocking())
        return;

    MoveData* mData = pSE->GetMoveData();

    // if ship is in warp;  cannot cancel till warp drops
    if (mData->warpState.active)
        return;

    if (is_log_enabled(DESTINY__MOVE_TRACE))
        sLog.Warning("BallPark", "%s calling stop", pSE->GetName());

    if (mData->ballMode == Ball::Mode::WARP) {
        /*  ballmode is set in Destiny::WarpTo();  warpstate is set in InitWarp()
         * ship may not actually be in warp yet if align/speed isnt right
         * in this case, player wants to cancel warp before it begins
         * for this, we need to send a stop packet after stop vars are set.
         *  however,   sending stop packet while in warp does funny things...dont send it.
         */
        // warp not started yet, reset warp vars and fall thru to allow stop
        mData->warpState.Reset();
        if (pSE->GetSelf()->HasAttribute(AttrWarpCapacitorNeed)) {
            mData->warpCapNeed = pSE->GetSelf()->GetAttribute(AttrWarpCapacitorNeed).get_double();
        } else {
            mData->warpCapNeed = 0.000000138;   // lowest value in db
        }

        // warp gfx cleared by stop packet

        // set marker for calc'd stop distance (testing)
        if (is_log_enabled(DESTINY__WARP_DEBUG)) {
            Vector3d& pos = pSE->GetPositionRef();
            uint16 dist = mData->maxSpeed * mData->activeSpeedFraction * mData->agility;
            Vector3d offset = (pos * dist);
            Vector3d marker = (pos + offset);
            std::string str = "Stop Point - ";
            str += pSE->GetName();
            pSE->SystemMgr()->MarkPoint(marker, str, str);
        }
    }

    mData->stop = true;
    mData->alignTo = false;
    mData->posHack = false;
    mData->autoPilot = false;

    mData->targBubble = nullptr;

    mData->ticStamp = pSE->SystemMgr()->GetTicCount();

    mData->desiredVelocity = NULL_ORIGIN;

    SetSpeedFraction(0.0f);

    if (mData->ballMode != Ball::Mode::STOP) {
        mData->ballMode = Ball::Mode::STOP;
        CmdStop du;
            du.entityID = pSE->GetID();
        PyTuple *up = du.Encode();
        pSE->SystemMgr()->SendSingleDestinyUpdate(&up);
        PyDecRef(up);
    }
}

// calling this will set object to a COMPLETE and IMMEDIATE stop, update STATE and remove from system ticlist
void BallPark::Halt(SpaceEntity* pSE, bool commanded/*false*/) {
    MoveData* mData = pSE->GetMoveData();

    SafeDelete(mData->warpState);

    //  reset ALL movement variables and states.
    mData->ballMode = Ball::Mode::STOP;
    mData->stop = true;
    mData->alignTo = false;
    mData->autoPilot = false;
    mData->ticStamp = 0;
    mData->targetDistance = 0;
    mData->userSpeedFraction = 0.0f;

    mData->targetPoint = NULL_ORIGIN;
    mData->myVelocity = NULL_ORIGIN;
    mData->desiredHeading = NULL_ORIGIN;
    mData->desiredVelocity = NULL_ORIGIN;

    // do we really wanna do this here?
    mData->targetEntity.first = 0;
    mData->targetEntity.second = nullptr;

    mData->skipTic = true;

    if (is_log_enabled(DESTINY__MOVE_TRACE))
        _log(DESTINY__MOVE_TRACE, "Destiny::Halt(%s) - %s(%u) Halted", \
                (commanded ? "true" : "false"), pSE->GetName(), pSE->GetID());

    if (commanded) {
        // immediate halt via command.  send packet to stop ship.
        CmdStop du;
            du.entityID = pSE->GetID();
        PyTuple *up = du.Encode();
        pSE->SystemMgr()->SendSingleDestinyUpdate(&up);
    }
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
    const Vector3d& targPos = mData->targSE->GetPosition();
    const Vector3d delta = targPos - mData->mySE->GetPosition();
    double distSq = delta.LengthSq();

    if (distSq < 1e-10) {
        // go out in some arbitrary direction
        mData->targPoint.x = targPos.x + mData->followDistance;
        mData->targPoint.y = targPos.y;
        mData->targPoint.z = targPos.z;
    } else {
        // target point in range along our vector
        double newDist = mData->followDistance / std::sqrt(distSq);
        mData->targPoint.x = targPos.x - (delta.x * newDist);
        mData->targPoint.y = targPos.y - (delta.y * newDist);
        mData->targPoint.z = targPos.z - (delta.z * newDist);
    }
}

void BallPark::CalculateFormationPoint(MoveData* mData) {
    // for formations
    SpaceEntity* leader = mData->targSE;

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
    Vector3d worldOffset = RotateVectorByEntityOrientation(localOffset, leader);

    // 6. Anchor the final server target point relative to the leader
    mData->targPoint = leader->GetPosition() + worldOffset;
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
    Vector3d toVector = mData->targSE->GetPosition() - mData->mySE->GetPosition();
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
    const double targetDistSq = mData->targetDistance * mData->targetDistance;
    const double toComp = distSq - targetDistSq;

    if (toComp >= 0.0) {
        // updated math
        const double sqrtToComp = std::sqrt(toComp);
        const double invNorm = 1.0 / (dist * sqrtToComp);
        const double toScale = toComp * invNorm;
        const double radComp = mData->targetDistance * sqrtToComp * invNorm;

        toVector = (toScale * toVector) + (radComp * radialVector);
    }

    // 5. Apply the official exponential inward-pull curve
    const double delta        = mData->targetDistance - dist;
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
    const double signum = (dist > mData->targetDistance) ? 1.0 : ((dist < mData->targetDistance) ? -1.0 : 0.0);
    transverseFactor *= signum;

    // 9. THE BRIDGE: Scale the final direction components directly into terminal velocity space (m/s)
    const double maxVelStep = mData->userSpeedFraction * mData->maxSpeed;
    mData->desiredVelocity.x = (radialVector.x * radialFactor + toVector.x * transverseFactor) * maxVelStep;
    mData->desiredVelocity.y = (radialVector.y * radialFactor + toVector.y * transverseFactor) * maxVelStep;
    mData->desiredVelocity.z = (radialVector.z * radialFactor + toVector.z * transverseFactor) * maxVelStep;
}

void BallPark::GotoVelocity(MoveData* mData) {
    // 1. Calculate the relative displacement heading vector
    mData->desiredHeading = mData->targPoint - mData->mySE->GetPosition();
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

void BallPark::Eject(SpaceEntity* pSE) {
    // basic updates for ejecting from ship...does <Eject> stop ship?
    // MAKE SURE...pod is set to troll with ship's current velocity.
    UpdateOldShip(pSE->GetShipSE());
    SendJettisonPacket();
    Stop();
}



/***********
 * warp methods
 */

void BallPark::InitWarp(MoveData* mData) {
    // these are the only vars that need to be set here right now
    mData->posHack = false;
    mData->warpState.Reset();

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
    SpaceEntity* pSE = mData->mySE;
    if (is_log_enabled(DESTINY__WARP_TRACE))
        _log(DESTINY__WARP_TRACE, "Destiny::InitWarp(): %s(%u) is initializing warp.", pSE->GetName(), pSE->GetID());

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
    int64 warpSpeedInMeters = mData->warpSpeed * ONE_AU_IN_METERS;
    // set times and distances based on target distance
    if (mData->targetDistance < (warpSpeedInMeters * 3)) {
        //  short warp....no cruise
        // accel = 1/3 decel
        mData->warpState.accelDist = (mData->targetDistance / 3);
        warpSpeedInMeters = mData->warpState.accelDist;
    } else {
        mData->warpState.accelDist = warpSpeedInMeters;       // ship warp speed in meters
        mData->warpState.cruiseDist = ((double)mData->targetDistance - (mData->warpState.accelDist * 3));
        mData->warpState.cruiseTime = EvE::max(mData->warpState.cruiseDist / warpSpeedInMeters, 1);
    }

    mData->warpState.decelDist = mData->warpState.accelDist * 2;
    mData->warpState.accelTime = std::log(mData->warpState.accelDist / 3) / 3;
    mData->warpState.decelTime = std::log(mData->warpState.decelDist);

    //  set total warp time based on above math.
    mData->warpState.warpTime = mData->warpState.accelTime + mData->warpState.decelTime + mData->warpState.cruiseTime;

    // time decel ship speed to drop out of warp
    float speed = 0.0f;
    bool run = true;
    uint8 step = 0;
    while (run) {
        speed = std::exp(--(mData->warpState.decelTime));
        ++step;
        if (speed < mData->speedToLeaveWarp) {
            run = false;
        }
    }

    // reset to zero for proper time/dist calc
    mData->warpState.decelTime = 0.0f;
    while (step > 0) {
        mData->warpState.decelDist += std::exp(mData->warpState.decelTime++);
        --step;
    }

    // hack to increase stop distance and *hopefully* make warp stop smoother
    mData->warpState.decelDist += mData->speedToLeaveWarp;

    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        _log(DESTINY__WARP_TRACE, "Destiny::InitWarp():Calculate - %s(%u): Warp will accelerate for %0.1fs, cruise for %0.1fs, then decelerate for %0.1fs, with total time of %0.1fs and warp speed of %lli m/s.", \
                pSE->GetName(), pSE->GetID(), mData->warpState.accelTime, mData->warpState.cruiseTime, mData->warpState.decelTime, mData->warpState.warpTime, warpSpeedInMeters);
        _log(DESTINY__WARP_TRACE, "Destiny::InitWarp():Calculate - Accel distance is %lli  Cruise distance is %lli   Decel distance is %lli", \
                mData->warpState.accelDist, mData->warpState.cruiseDist, mData->warpState.decelDist);
        _log(DESTINY__WARP_TRACE, "Destiny::InitWarp():Calculate - We will exit warp at %0.1f,%0.1f,%0.1f at a distance of %lli AU (%lli m).", \
                mData->targPoint.x, mData->targPoint.y, mData->targPoint.z, mData->targetDistance / ONE_AU_IN_METERS, mData->targetDistance);
    }

    float intAccel = 0.0f;
    mData->warpState.accelFraction = std::modf(mData->warpState.accelTime, &intAccel);

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
    // do npcs need to notify ai of warping?

    //clear targets
    pSE->TargetMgr()->ClearAllTargets();
    //pSE->TargetMgr()->OnTarget(nullptr, TargMgr::Mode::Clear, TargMgr::Msg::WarpingOut);

    mData->targSE = nullptr;

    // send warp gfx
    if (pSE->SysBubble()->HasPlayers())
        pSE->SystemMgr()->SendGFX10(pSE->GetID(), "effects.Warping");

    //TODO:  determine if this ship has assigned drones in space and call drone:shipwarping

    // reset move times
    mData->warpState.accelDist = 0;
    mData->ticStamp = pSE->SystemMgr()->GetTicCount();
    mData->activeSpeedFraction = 1.0f;

    // all ships will completely stop before warp
    mData->myVelocity = NULL_ORIGIN;
    mData->warpState.active = true;
    mData->warpState.phase = WarpState::Phase::Accel;
    WarpAccel(mData, 0);
}

void BallPark::WarpAccel(MoveData* mData, uint16 sec_into_warp) {
    /* For acceleration, k = 3.
     * distance = e^(k*s)
     * speed = k*e^(k*s)
     */
    float accelTime = (sec_into_warp + mData->warpState.accelFraction);
    int64 currentShipSpeed = std::exp(3 * accelTime);

    if (accelTime >= mData->warpState.accelTime) {
        // fix this...
        currentShipSpeed = mData->warpState.accelDist;

        if (mData->warpState.cruiseDist > 0.0) {
            mData->warpState.phase = WarpState::Phase::Cruise;
        } else {
            mData->warpState.phase = WarpState::Phase::Decel;
            mData->targetDistance = mData->warpState.decelDist + currentShipSpeed;
            // update ship position to match decel target
            Vector3d update(NULL_ORIGIN);
            update.x = (mData->desiredHeading.x * mData->targetDistance);
            update.y = (mData->desiredHeading.y * mData->targetDistance);
            update.z = (mData->desiredHeading.z * mData->targetDistance);

            Vector3d& posRef = mData->mySE->GetPositionRef();
            posRef.x = mData->targPoint.x - update.x;
            posRef.y = mData->targPoint.y - update.y;
            posRef.z = mData->targPoint.z - update.z;
        }
    }

    WarpUpdate(mData, currentShipSpeed, sec_into_warp, WarpState::Phase::Accel);

    if (mData->mySE->SysBubble() != nullptr) {
        // check actual bubble center here
        if (!mData->mySE->SysBubble()->InBubble(mData->mySE->GetPosition())) {
            if (is_log_enabled(DESTINY__WARP_TRACE))
                _log(DESTINY__WARP_TRACE, "Destiny::WarpAccel(): %s(%u) is being removed from bubble %u.", \
                        mData->mySE->GetName(), mData->mySE->GetID(), mData->mySE->SysBubble()->GetID());
            mData->mySE->SysBubble()->Remove(mData->mySE);
        }
    }
}

void BallPark::WarpCruise(MoveData* mData, uint16 sec_into_warp) {
    // in cruise...update position data and check for decel
    WarpUpdate(mData, mData->warpState.warpSpeed, sec_into_warp, WarpState::Phase::Cruise);

    if ((mData->targetDistance - mData->warpState.warpSpeed) < mData->warpState.decelDist) {
        mData->targetDistance = mData->warpState.decelDist;
        // update ship position to match decel target
        Vector3d update;
        update.x = (mData->desiredHeading.x * mData->targetDistance);
        update.y = (mData->desiredHeading.y * mData->targetDistance);
        update.z = (mData->desiredHeading.z * mData->targetDistance);
        Vector3d& posRef = mData->mySE->GetPositionRef();
        posRef.x = mData->targPoint.x - update.x;
        posRef.y = mData->targPoint.y - update.y;
        posRef.z = mData->targPoint.z - update.z;

        mData->warpState.phase = WarpState::Phase::Decel;
    }
}

void BallPark::WarpDecel(MoveData* mData, uint16 sec_into_warp) {
    /* For deceleration, k = -1
     * distance = e^(k*s)
     * speed = k*e^(k*s)
     */
    //double decelTime = --mData->decelTime;
    int64 currentShipSpeed = std::exp(--mData->warpState.decelTime);

    WarpUpdate(mData, currentShipSpeed, sec_into_warp, WarpState::Phase::Decel);

    // how will this work for edge-case where dest is 5k inside bubble?
    if (mData->mySE->SysBubble() == nullptr) {
        if (mData->warpState.targBubble->InBubble(mData->mySE->GetPosition(), true)) {   // check modified bubble center here
            if (is_log_enabled(DESTINY__WARP_TRACE))
                _log(DESTINY__WARP_TRACE, "Destiny::WarpDecel(): %s(%u) is being added to bubble %u.", \
                    mData->mySE->GetName(), mData->mySE->GetID(), mData->warpState.targBubble->GetID());
            mData->warpState.targBubble->Add(mData->mySE);
        }
    }

    // updated warp drop speed
    if (currentShipSpeed <= mData->speedToLeaveWarp)
        WarpStop(mData, mData->speedToLeaveWarp);

}

void BallPark::WarpUpdate(MoveData* mData, int64 currentShipSpeed, uint16 sec_into_warp, uint8 phase=0) {
    //  track position and velocity for all stages.
    mData->targetDistance -= currentShipSpeed;

    Vector3d& pos = mData->mySE->GetPositionRef();
    Vector3d& vel = mData->myVelocity;

    mData->myVelocity.x = (mData->desiredHeading.x * currentShipSpeed);
    mData->myVelocity.y = (mData->desiredHeading.y * currentShipSpeed);
    mData->myVelocity.z = (mData->desiredHeading.z * currentShipSpeed);

    pos.x += mData->myVelocity.x;
    pos.y += mData->myVelocity.y;
    pos.z += mData->myVelocity.z;

    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        switch (phase) {
            case WarpState::Phase::Accel: {
                _log(DESTINY__WARP_TRACE, "Destiny::WarpAccel(): %s(%u) - Warp Accelerating(%us): velocity %lli m/s.   %lli m remaining.", \
                        mData->mySE->GetName(), mData->mySE->GetID(), sec_into_warp, currentShipSpeed, mData->targetDistance);
            } break;
            case WarpState::Phase::Cruise: {
                _log(DESTINY__WARP_TRACE, "Destiny::WarpCruise(): %s(%u) - Warp Cruising(%us): velocity %lli m/s.  %lli m remaining.", \
                        mData->mySE->GetName(), mData->mySE->GetID(), sec_into_warp, currentShipSpeed, mData->targetDistance);
            } break;
            case WarpState::Phase::Decel: {
                _log(DESTINY__WARP_TRACE, "Destiny::WarpDecel(): %s(%u) - Warp Decelerating(%us): velocity %0.2f,%0.2f,%0.2f, speed %lli m/s.  %lli m remaining.", \
                        mData->mySE->GetName(), mData->mySE->GetID(), sec_into_warp, mData->myVelocity.x, mData->myVelocity.y, mData->myVelocity.z, \
                        currentShipSpeed, mData->targetDistance);
            } break;
            default: {
                _log(DESTINY__WARNING, "Destiny::WarpUpdate()  %s(%u): Called with invalid type.", \
                        mData->mySE->GetName(), mData->mySE->GetID());
            }
        }

        Vector3d delta = mData->warpState.targBubble->GetCenter() - pos;
        double dist = delta.Length();
        _log(DESTINY__WARP_TRACE, "Destiny::WarpUpdate()  %s(%u): Ship is %0.1f from center of target bubble %u.", \
                mData->mySE->GetName(), mData->mySE->GetID(), dist, mData->warpState.targBubble->GetID());
    }
}

void BallPark::WarpStop(MoveData* mData, int64 currentShipSpeed) {
    if (is_log_enabled(DESTINY__WARP_TRACE)) {
        const Vector3d& pos = mData->mySE->GetPosition();
        _log(DESTINY__WARP_TRACE, "Destiny::WarpStop(): %s(%u) - Warp complete. Exit velocity %lli m/s with %lli left to go.", \
                mData->mySE->GetName(), mData->mySE->GetID(), currentShipSpeed, mData->targetDistance);
        _log(DESTINY__WARP_TRACE, "Destiny::WarpStop(): Ship currently at %0.2f,%0.2f,%0.2f.", \
                pos.x, pos.y, pos.z);
    }

    // is this needed?  yes, for proper sub-warp movement tracking
    mData->targPoint += (mData->myVelocity * mData->agility);

    mData->warpState.active = false;
    Stop(mData->mySE);

    if ((mData->mySE->IsNPCSE()) and (mData->mySE->GetNPCSE()->GetAI() != nullptr))
        mData->mySE->GetNPCSE()->GetAI()->WarpOutComplete();

    /*  this isnt used yet, but will be needed once bumping is implemented...
     *    // reset bump checks
     *    SetBallMassive sbmassive;
     *        sbmassive.entityID = pSE->GetID();
     *        sbmassive.is_massive = sConfig.cosmic.BumpEnabled;
     *    PyTuple *up = sbmassive.Encode();
     *    SendSingleDestinyUpdate(&up);
     *    PyDecRef(up);
     */
}

// there may be more here im missing.  we can also target SEs for dock, jump, etc
bool BallPark::ValidTarget(SpaceEntity* pSE, SpaceEntity* targetSE) {
    if (__builtin_expect(targetSE == nullptr, 0))
        return false;

    if (targetSE->IsStationSE() or targetSE->IsGateSE() or targetSE->IsBeltSE()
    or targetSE->IsPOSSE() or targetSE->IsItemEntity() or targetSE->IsObjectEntity())
        return true;

    if (targetSE->IsDead() or targetSE->IsInvul() or targetSE->IsFrozen())
        return false;

    MoveData* tMD = targetSE->GetMoveData();
    if ((tMD == nullptr) or (tMD->IsWarping()))
        return false;

    if (targetSE->IsStaticEntity())
        return false;
    if (targetSE->HasPilot() and targetSE->GetPilot()->IsDocked())
        return false;

    return true;
}

// Basic Movement Call:
void BallPark::BeginMovement(MoveData* mData) {
    // common movement for all types
    if (!mData->hasSentShipUpdates
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
        SendDestinyUpdates(updates); //consumed
        mData->hasSentShipUpdates = true;
    }

    UnCloak();

    mData->stop = false;
    mData->skipTic = false;
    mData->moveDelay = true;

    // reset move stamps
    mData->ticStamp = pSE->SystemMgr()->GetTicCount();
    mData->timeStamp = GetFileTimeNow();

    // distance check (per client)
    // Python gateway default constant backup check (0x44fa0000 = 2000m)
    if ((mData->ballMode != Destiny::Ball::Mode::WARP) and (mData->followDistance < 1))
        mData->followDistance = 2000;

    SetSpeedFraction();

    if (is_log_enabled(DESTINY__MOVE_TRACE)) {
        if (mData->targetEntity.second != nullptr) {
            SpaceEntity* pSE = mData->targetEntity.second;
            _log(DESTINY__MOVE_TRACE, "target (%s) position: %0.2f, %0.2f, %0.2f", \
                    pSE->GetName(), pSE->GetPosition().x, pSE->GetPosition().y, pSE->GetPosition().z);
        }
        _log(DESTINY__MOVE_TRACE, " currentVelocity: %0.3f,%0.3f,%0.3f", \
                mData->shipVelocity.x, mData->shipVelocity.y, mData->shipVelocity.z);
    }
}

/*********************************************
 *   Beyonce calls
 */

void BallPark::AlignTo(SpaceEntity* pSE) {
    // should this Stop() once alignment has been achieved?  i'd say yes.  config option?
    // i originally set it like this, but aknor didnt like it, so it was removed
    mData->alignTo = true;
    GotoPoint(pSE->GetPosition());
}

void BallPark::ApproachBall(SpaceEntity* pSE) {
    // use this for dock and jump where dist > interact dist
    //mData->alignTo = true;
    GotoPoint(pSE->GetPosition());
    // TODO: is this right?
    mData->followDistance = pSE->GetRadius() + pSE->GetRadius() + 1000;
    if (mData->followDistance > 2500)
        mData->followDistance = 2000;
}

void BallPark::GotoDirection(const Vector3d& direction) {
    mData->desiredHeading = direction;
    mData->targetPoint = mData->position + (direction * 1e9);
    mData->targetDistance = 1e9;
    mData->followDistance = 0;

    mData->ballMode = Destiny::Ball::Mode::GOTO;
    BeginMovement();

    CmdGotoDirection du;
        du.entityID = pSE->GetID();
        du.x = mData->desiredHeading.x;
        du.y = mData->desiredHeading.y;
        du.z = mData->desiredHeading.z;
    PyTuple* up = du.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

void BallPark::GotoPoint(const Vector3d& point) {
    mData->targetPoint = point;
    mData->desiredHeading = point - mData->position;
    mData->targetDistance = mData->desiredHeading.Length();
    mData->desiredHeading.Normalize();
    mData->followDistance = 0;

    mData->ballMode = Destiny::Ball::Mode::GOTO;
    BeginMovement();

    CmdGotoPoint du;
        du.entityID = pSE->GetID();
        du.x = mData->targetPoint.x;
        du.y = mData->targetPoint.y;
        du.z = mData->targetPoint.z;
    PyTuple* up = du.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

void BallPark::FollowBall(SpaceEntity* pSE, int32 distance) {
    //called from client as 'CmdFollowBall'
    //  also used by 'Approach'
    mData->targetPoint = pSE->GetPosition();
    mData->targetEntity.first = pSE->GetID();
    mData->targetEntity.second = pSE;
    if (!ValidTarget()) {
        Stop();
        return;
    }

    mData->followDistance = distance + pSE->GetRadius() + pSE->GetRadius();

    mData->desiredHeading = pSE->GetPosition() - mData->position;
    mData->targetDistance = mData->desiredHeading.Length();
    mData->desiredHeading.Normalize();

    mData->ballMode = Destiny::Ball::Mode::FOLLOW;
    BeginMovement();

    CmdFollowBall du;
        du.entityID = pSE->GetID();
        du.targetID = pSE->GetID();
        du.range = distance;
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

void BallPark::OrbitBall(SpaceEntity *pSE, uint32 distance/*0*/) {
    if (!sConfig.debug.UseOrbit) {
        Stop();
        return;
    }

    mData->ballMode = Destiny::Ball::Mode::ORBIT;
    mData->targetEntity.first = pSE->GetID();
    mData->targetEntity.second = pSE;
    if (!ValidTarget()) {
        Stop();
        return;
    }

    mData->followDistance = distance;
    mData->targetDistance = mData->followDistance + pSE->GetRadius() + pSE->GetRadius();

    mData->ballMode = Destiny::Ball::Mode::ORBIT;
    BeginMovement();

    CmdOrbit du;
        du.entityID = pSE->GetID();
        du.orbitEntityID = pSE->GetID();
        du.distance = distance;
    PyTuple *up = du.Encode();
    SendSingleDestinyUpdate(&up);
    PyDecRef(up);
}

// Fleet warps - on live, all ships use the warp profile of the slowest ship, here they warp as normal
void BallPark::WarpTo(const Vector3d& destPoint, int32 distance, bool autoPilot, SpaceEntity* pSE/*nullptr*/) {
    /* warp order..
     * pick destination -> align/accel -> aura "warp drive active" -> cap drain -> accel
     *      -> enter warp -> warp -> decel -> leave warp -> coast -> stop
     */
    SafeDelete(mData->warpState);

    // get target point
    if (destPoint.isZero()) {
        if (pSE != nullptr) {
            mData->targetPoint = pSE->GetPosition();
        } else {
            Stop();
            sLog.Error("Destiny::WarpTo()", "DestPoint is zero and pSE is null.");
            if (pSE->HasPilot())
                throw UserError("WarpDestinationGone");
            return;
        }
    } else {
        mData->targetPoint = destPoint;
    }

    Vector3d warp_vector = mData->targetPoint - mData->position;
    mData->followDistance = distance;
    mData->targetDistance = warp_vector.Length();
    mData->targetDistance -= distance;

    if (pSE->HasPilot()) {
        Client* pClient = pSE->GetPilot();
        if (mData->targetDistance < MIN_WARP_DISTANCE) {
            // send position update
            if (sConfig.debug.PositionHack)
                SetPosition(pSE->GetPosition(), true);
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

        double dist = ((double)mData->targetDistance / ONE_AU_IN_METERS);
        double currentShipCap = pClient->GetShip()->GetAttribute(AttrCapacitorCharge).get_double();
        double capNeeded = pSE->GetSelf()->mass() * mData->warpCapacitorNeed * dist;
        capNeeded *= (1.0 - (0.1 * pClient->GetChar()->GetSkillLevel(EvESkill::WarpDriveOperation)));

        _log(DESTINY__WARNING, "Warp Cap need for %s(%u) for %llim (%0.3f AU) is %0.5f GJ", \
            pSE->GetName(), pSE->GetID(), mData->targetDistance, dist, capNeeded);

        // set min cap need to 1.0
        if (capNeeded < 1.0f)
            capNeeded = 1.0;

        //  check if ship has enough capacitor to warp full distance
        if (capNeeded > currentShipCap) {
            // not enough cap.  reset everything based on available cap
            double capHas = currentShipCap / (pSE->GetSelf()->mass() * mData->warpCapacitorNeed);
            if (capHas > 1.0f) {
                mData->targetDistance = capHas * ONE_AU_IN_METERS;
                Vector3d warp_direction = mData->targetPoint - mData->position;
                // make heading
                warp_direction.Normalize();
                mData->targetPoint = mData->position;
                mData->targetPoint.x -= (warp_direction.x * mData->targetDistance);
                mData->targetPoint.y -= (warp_direction.y * mData->targetDistance);
                mData->targetPoint.z -= (warp_direction.z * mData->targetDistance);
                pClient->SendErrorMsg("Your commanded warp of %0.3f AU requires %0.1f GJ of capacitor charge, but you only have %0.1f GJ available.<br>This is enough for %0.3f AU.<br><br>Setting course for available capacitor charge.", \
                        dist, capNeeded,  currentShipCap, capHas );
                /** @todo  update all move vars here for new target... */
                warp_vector = mData->targetPoint - mData->position;
                mData->targetDistance = warp_vector.Length();
                mData->targetDistance -= distance;
            } else {
                // if not enough cap to do min warp. cancel and return
                //WarpingWithAvailablePowerBody
                pClient->SendErrorMsg("You don't have enough capacitor charge to warp.");
                _log(DESTINY__WARNING, "Destiny::InitWarp() - %s(%u): Capacitor needed vs current  %f / %0.5f",
                        pSE->GetName(), pSE->GetID(), capNeeded, currentShipCap);

                Stop();
                return;
            }
        } else {
            mData->warpCapacitorNeed = currentShipCap - capNeeded;
            // change to heading
            warp_vector.Normalize();
            // adjust target point by calculated stopping point
            //mData->targetPoint.x -= (warp_vector.x * distance);
            //mData->targetPoint.y -= (warp_vector.y * distance);
            //mData->targetPoint.z -= (warp_vector.z * distance);
        }
    }

    //You will always exit warp at a random point 2,500 meters from your actual exit point - per EveUni
	//  this will need update for formation warp...leader will have random offset, others follow per formation
    /* disabled for testing
     * if (pSE->HasPilot())
     *   mData->targetPoint.MakeRandomPointOnSphereLayer(-2500, 2500);   disabled for testing
     */

    // reset heading for updated targPoint
    mData->desiredHeading = mData->targetPoint - mData->position;
    // change to heading
    mData->desiredHeading.Normalize();

    // get targ bubble.  this will create bubble if needed
    mData->targBubble = sBubbleMgr.GetBubble(pSE->SystemMgr(), mData->targetPoint);

    // npcs have no warp restrictions (yet)
    if (pSE->IsNPCSE()) {
        mData->ballMode = Destiny::Ball::Mode::WARP;
        BeginMovement();

        // if no players in bubble, this isnt needed...
        if (pSE->SysBubble()->HasPlayers()) {
            std::vector<PyTuple*> up;
            // send warp update
            CmdWarpTo wt;
                wt.entityID = pSE->GetID();
                wt.dest_x = mData->targetPoint.x;
                wt.dest_y = mData->targetPoint.y;
                wt.dest_z = mData->targetPoint.z;
                wt.distance = distance;
                wt.warpSpeed = GetWarpSpeed();      // warp speed x10
            up.push_back(wt.Encode());
            SendDestinyUpdates(up); //consumed
        }

        if (is_log_enabled(NPC__MESSAGE))
            _log(NPC__MESSAGE, "Destiny::WarpTo() NPC %s(%u) %u -> %u, mData->targetPoint: %0.2f,%0.2f,%0.2f  stop distance: %i  mData->targetDistance: %llim", \
                    pSE->GetName(), pSE->GetID(), pSE->SysBubble()->GetID(), mData->targBubble->GetID(), \
                    mData->targetPoint.x, mData->targetPoint.y, mData->targetPoint.z, distance, mData->targetDistance);
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
     *   NOTE:  warp bubble in path (or within 150km of mData->targetPoint) will change mData->targetDistance and mData->targetPoint
     *   however, this does NOT affect original calculations for energy needed, etc...
     */

    /** @todo  does this apply for ANY bubble along warp route or just end?
     * would be fun to check entire route for bubble...can bm along route to add bubble
     * however, this will take a bit to implement
     *
     * IDEA:  ...nope, lost it.
     */
    if (mData->targBubble->HasWarpBubble())
        if (!pSE->GetSelf()->HasAttribute(AttrWarpBubbleImmune)) {
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
    if (mData->userSpeedFraction < 0.751)
        SetSpeedFraction(1.0f);

    mData->ballMode = Destiny::Ball::Mode::WARP;
    BeginMovement();

    if (is_log_enabled(DESTINY__WARP_DEBUG)) {
        // marker for ship's stop point
        std::string str = "Warp Stop Point - ";
        str += pSE->GetName();
        MarkPoint(mData->targetPoint, str, str);
        str.clear();

        // marker for ship's drop out of warp point
        Vector3d dropPos = (mData->desiredHeading * mData->speedToLeaveWarp * mData->agility);
        str = "Warp Drop Point - ";
        str += pSE->GetName();
        MarkPoint(mData->targetPoint - dropPos, str, str);
    }

    //set massive for warp.   self-only per client logs
    SetBallMassive sbmassive;
        sbmassive.entityID = pSE->GetID();
        sbmassive.is_massive = 0;       // disable client-side bump checks
    PyTuple *up = sbmassive.Encode();
    SendSingleDestinyUpdate(&up, true);
    PyDecRef(up);

    std::vector<PyTuple*> updates;
    // send warp update
    CmdWarpTo wt;
        wt.entityID = pSE->GetID();
        wt.dest_x = mData->targetPoint.x;
        wt.dest_y = mData->targetPoint.y;
        wt.dest_z = mData->targetPoint.z;
        wt.distance = distance;
        wt.warpSpeed = GetWarpSpeed();      // warp speed x10
    updates.push_back(wt.Encode());
    SendDestinyUpdates(updates); //consumed

    if (is_log_enabled(DESTINY__WARP_TRACE))
        _log(DESTINY__WARP_TRACE, "Destiny::WarpTo() %u -> %u, mData->targetPoint: %0.2f,%0.2f,%0.2f  stop distance: %i  mData->targetDistance: %lli",
             pSE->SysBubble()->GetID(), mData->targBubble->GetID(), mData->targetPoint.x, mData->targetPoint.y, mData->targetPoint.z, distance, mData->targetDistance);
}

bool BallPark::IsAligned(MoveData* data, Vector3d& targHeading) {
    double distanceSq = data->myVelocity.LengthSq();
    if (distanceSq <  VISUAL_STOP_THRESHOLD_SQ) {
        // if ship is stopped enough, there is no 'aligning' to do
        return true;
    }

    double dot = data->myVelocity.dotProduct(targHeading);
    //double degrees = EvE::Trig::Rad2Deg(std::acos(dot));
    if (data->ballMode == Ball::Mode::WARP) {
        if (dot > /*WARP_ALIGNMENT 0.99449256*/ 0.99556405)
            return true;
    } else if (dot > /*TURN_ALIGNMENT*/ 0.99756405) {
        return true;
    }
    return false;
}

void BallPark::ApplyVortexGravityTug(ShipSE* pShipSE, Vector3d& vortexCenterAxis) {
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
    // Loop bubble's entity map (adapt to your SystemBubble map getter)
    for (auto const& pair : bubble->GetEntities()) {
        SpaceEntity* se = pair.second;

        MoveData* md = se->GetMoveData();
        if (!mData->massive)
            continue;
        if (md && mData->ballMode == Ball::Mode::WARP)
            continue;

        const Vector3d& pos = se->GetPosition();
        double rad = se->GetRadius();
        if (rad > maxRadius)
            maxRadius = rad;

        proxies.push_back({ se, pos.x, pos.y, pos.z, rad });
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

void BallPark::Bump(SpaceEntity* seA, SpaceEntity* seB) {
    MoveData* mdA = seA->GetMoveData();
    MoveData* mdB = seB->GetMoveData();

    const bool staticA = (mdA == nullptr && !seA->IsWreckSE() && !seA->IsContainerSE());
    const bool staticB = (mdB == nullptr && !seB->IsWreckSE() && !seB->IsContainerSE());
    if (staticA && staticB)
        return;

    // Lazy-init MoveData for bumped unanchored wrecks/cans
    if (!staticA && mdA == nullptr) {
        seA->CreateMoveData();
        mdA = seA->GetMoveData();
        mdA->ballMode = Ball::Mode::STOP;
    }
    if (!staticB && mdB == nullptr) {
        seB->CreateMoveData();
        mdB = seB->GetMoveData();
        mdB->ballMode = Ball::Mode::STOP;
    }

    Vector3d& posA = seA->GetPositionRef();
    Vector3d& posB = seB->GetPositionRef();

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
    double relVel = (velA - velB).dotProduct(normal);

    // Already separating
    if (relVel <= 0.0)
        return;

    // 3. Santorine elastic impulse
    double invMassA = staticA ? 0.0 : (1.0 / seA->GetSelf()->mass());
    double invMassB = staticB ? 0.0 : (1.0 / seB->GetSelf()->mass());
    double invMassSum = invMassA + invMassB;
    if (invMassSum <= 0.0)
        return;

// this can change for 'better' mechanics/movement
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
    double overlap = (seA->GetRadius() + seB->GetRadius()) - dist;
    if (overlap > 0.0) {
        if (!staticA && !staticB) {
            posA -= normal * (overlap * 0.5);
            posB += normal * (overlap * 0.5);
        } else if (!staticA) {
            posA -= normal * overlap;
        } else if (!staticB) {
            posB += normal * overlap;
        }
    }
}
