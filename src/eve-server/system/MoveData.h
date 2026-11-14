/*
 *	MoveData.h
 *		this class takes all the old destiny variables and herds them into a common group container,
 * 		allowing ease of access without cluttering up SE or II base class members.
 *
 *	when needed, this data is created when SE is created, or on-the-fly (as with wrecks/cans for tractoring) and
 *	pulled into BP for use.  it is stored in SE private member as pointer.  nullptr if none
 *
 *
 * @Author:     Allan
 * @Version:    0.20
 * @Date:       11Nov26  (partially copied from DestinyManager.h)
 */

#pragma once

#include "eve-compat.h"
#include "math/Vector3d.h"
#include "destiny/DestinyStructs.h"

class SpaceEntity;
class EvilNumber;
class SystemBubble;

// New warp data layout
struct WarpState {
    struct Phase {
        enum {
            None   = 0,
            Accel  = 1,
            Cruise = 2,
            Decel  = 3
        };
    };

    // 8-byte aligned group (40 bytes)
    SystemBubble* targBubble = nullptr;
    int64 warpDistance   = 0;   // in m
    int64 warpSpeed      = 0;   // in m/s
    int64 accelDist      = 0;   // in m
    int64 cruiseDist     = 0;   // in m
    int64 decelDist      = 0;   // in m

    // 4-byte aligned group (12 bytes)
    int32 startTime      = 0;   // from sEntityMgr::GetStamp()
    float warpTime       = 0.0f;// in s
    float accelTime      = 0.0f;// in s
    float cruiseTime     = 0.0f;// in s
    float decelTime      = 0.0f;// in s
    float accelFraction  = 0.0f;

    // 1-byte aligned group (2 bytes + 2 bytes struct-end padding = 56 bytes total)
    uint8 phase          = Phase::None;
    bool  active         = false;

    void Reset() {
        targBubble     = nullptr;
        warpDistance   = 0;
        warpSpeed      = 0;
        accelDist      = 0;
        cruiseDist     = 0;
        decelDist      = 0;
        startTime      = 0;
        warpTime       = 0.0f;
        accelTime      = 0.0f;// in s
        cruiseTime     = 0.0f;// in s
        decelTime      = 0.0f;// in s
        accelFraction  = 0.0f;
        phase          = Phase::None;
        active         = false;
    }
};

class MoveData {
public:
    MoveData(SpaceEntity* pSE);
    ~MoveData() = default;

    void Init();

    // --- Fast Inline Getters ---
    inline const Vector3d& GetVelocity() const          { return myVelocity; }
    inline Vector3d&    GetVelocityRef()                { return myVelocity; }
    inline double       GetSpeed() const                { return myVelocity.Length(); }
    inline float        GetSpeedFraction() const        { return activeSpeedFraction; }
    inline float        GetMaxVelocity() const          { return maxSpeed; }
    inline double       GetAgility() const              { return agility; }
    inline uint8        GetBallMode() const             { return ballMode; }
    inline int32        GetTicStamp() const             { return ticStamp; }

    //inline uint32       GetTargetID() const             { return targSE->GetID(); }
    inline SpaceEntity* GetTargetSE() const             { return targSE; }
    inline const Vector3d& GetTargetPoint() const       { return targPoint; }
    inline double       GetTargetDistance() const       { return static_cast<double>(targetDistance); }
    inline double       GetFollowDistance() const       { return static_cast<double>(followDistance); }

    // --- State Checks ---
    inline bool 	IsMoving() const    		{ return (activeSpeedFraction > 0.01f); }
    inline bool 	IsGoto() const      		{ return (ballMode == Ball::Mode::GOTO); }
    inline bool 	IsStopped() const   		{ return (ballMode == Ball::Mode::STOP); }
    inline bool 	IsOrbiting() const  		{ return (ballMode == Ball::Mode::ORBIT); }
    inline bool 	IsFollowing() const 		{ return (ballMode == Ball::Mode::FOLLOW); }
    inline bool         IsFormation() const             { return (ballMode == Ball::Mode::FORMATION); }
    inline bool 	IsTractored() const 		{ return tractored; }
    inline bool         IsWarping() const               { return warpState.active; }

    inline void 	SetSkipTic(bool set=true) 	{ skipTic = set; }

    void                SetAgilityInertia();
    void                UpdateShipVariables();

    // --- Core Updaters & Modifiers ---
    bool                IsAligned(const Vector3d& targHeading) const;
    bool                ValidTarget();
    void                EntityRemoved(SpaceEntity* pSE);

    void                WebbedMe();
    void                SpeedBoost(bool deactivate = false);
    void                SetNPCSpeedMass(uint16 newSpeed, uint16 maxSpeed, double mass);

    void                TractorBeamStart(SpaceEntity* pSE, EvilNumber speed);
    void                TractorBeamStop();

    // Friend for BallPark's static math engine direct access
    friend class        BallPark;

private:
    // --- 8-byte aligned members ---
    SpaceEntity*        mySE;           // Back-pointer to parent SE (8 bytes)
    SpaceEntity*        targSE;         // pointer to active targetSE (8 bytes)
    WarpState           warpState;

    Vector3d            targPoint;
    Vector3d            myVelocity;
    Vector3d            desiredHeading;
    Vector3d            desiredVelocity;

    double              expTerm;
    double              posScale;
    double              timeFactor;
    double              agility;
    double              timeStamp;
    double              alignTime;
    double              warpSpeed;
    double              speedToLeaveWarp;
    double              warpCapNeed;

    int64               targetDistance;
    int64               followDistance;

    // --- 4-byte aligned members ---
    int32               ticStamp;
    float               maxSpeed;
    float               userSpeedFraction;
    float               activeSpeedFraction;

    // --- 1-byte aligned members ---
    uint8               ballMode;
    bool                moveDelay;
    bool                skipTic;
    bool                alignTo;
    bool                paused;
    bool                posHack;
    bool                sentUpdates;
    bool                stop;
    bool                tractored;
    bool                tractorPause;
    bool                massive;
    bool                autoPilot;
};
