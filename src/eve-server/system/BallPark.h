/*
 * BallPark.h
 *      this 'new' class is the updated and rewritten version that was previously "DestinyManager".
 *      originally designed as a per-ship class, this entire system has evolved to the point
 *		of needing destiny as a system class, looping a map of moving objects as needed each tic.
 *		instead of calling each object to move, the objects will now request intent for their next round.
 *		the park will move objects based on previous state, new requests, surroundings, and *probably more*.
 *
 *	this class does not keep specific object data; objects keep their own state and all data.
 *	by relegating data to the actual object, this reduces the previous multiple copies of data to a single, authoritive source.
 *
 *
 *
 * @Author:     Allan
 * @Version:    0.10
 * @Date:       10Nov26
 */

#pragma once

#include "../eve-server.h"


// common variables to denote acceptable alignment deviations
static const float TURN_ALIGNMENT = 4.0f;       //0.0698132 rad
static const float WARP_ALIGNMENT = 6.0f;       //0.105 rad

static const float VISUAL_STOP_THRESHOLD_SQ = 0.0025f;
static const float HARD_STOP_THRESHOLD_SQ = 0.0001f;

static const double SPACE_DRAG = 0.30000001192092896;


class ShipSE;
class SpaceEntity;
class Vector3d;
class MoveData;
class SystemBubble;

class BallPark {
public:
    // c'tor
    BallPark() =delete;
    // d'tor
    ~BallPark() =delete;

    /* Main Call */
    static void Process(SpaceEntity* pSE=nullptr);

    /* Beyonce Calls */
    static void AlignTo(MoveData* mData);
    static void GotoPoint(MoveData* mData);
    static void ApproachBall(MoveData* mData);
    static void GotoDirection(MoveData* mData);
    static void OrbitBall(MoveData* mData);
    static void FollowBall(MoveData* mData);
    static void WarpTo(MoveData* mData, bool autoPilot=false);

    /* Ship Calls */
    static void Stop(MoveData* mData);
    static void Halt(MoveData* mData, bool commanded=false);                    // puts entity at 0 velocity
    static bool ValidTarget(MoveData* mData);                 //performs common target checks
    static bool IsAligned(MoveData* mData);
    // desired Velocity setter
    static void SetSpeedFraction(MoveData* mData, float fraction);
    // for gravity-based 'ship drifting' idea (not implemented yet)
    static void ApplyVortexGravityTug(ShipSE* pShipSE, const Vector3d& vortexCenterAxis);
    static void SetTrollData(/*unknown args yet*/);  // as stated.

    /* bump code */
    static void Bump(SpaceEntity* mySE, SpaceEntity* herSE);
    static void ProcessBubbleBumps(SystemBubble* bubble);

private:
    // Internal wub-warp methods
    static void BeginMovement(MoveData* mData);               //set initial variables for all movement (common code)
    static void Integrate(MoveData* mData);
    static void CalculateFollowPoint(MoveData* mData);
    static void CalculateFormationPoint(MoveData* mData);
    static void GotoVelocity(MoveData* mData);
    static void OrbitVelocity(MoveData* mData, double ticCount);
    static Vector3d RotateVectorByEntityOrientation( SpaceEntity* pSE, const Vector3d& localVec, SpaceEntity* leader );

    // Internal Warp Methods
    static void InitWarp(MoveData* mData);
    static void WarpAccel(MoveData* mData, uint16 sec_into_warp);
    static void WarpCruise(MoveData* mData, uint16 sec_into_warp);
    static void WarpDecel(MoveData* mData, uint16 sec_into_warp);
    static void WarpStop(MoveData* mData, int64 currentShipSpeed);
    static void WarpUpdate(MoveData* mData, int64 currentShipSpeed, uint16 sec_into_warp, uint8 phase=0);      // 0=error, 1=accel, 2=cruise, 3=decel

};

