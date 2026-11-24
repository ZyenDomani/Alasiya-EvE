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

#include "eve-common.h"
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
    int64 accelDist      = 0;
    int64 cruiseDist     = 0;   // in m
    int64 decelDist      = 0;   // in m

    double accelTime     = 0.0;// in s
    double cruiseTime    = 0.0;// in s
    double decelTime     = 0.0;// in s

    // 4-byte aligned group (12 bytes)
    int32 startTic       = 0;   // from sysMgr::GetStamp()

    float warpTime       = 0.0f;// in s

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
        startTic       = 0;
        warpTime       = 0.0f;
        accelTime      = 0.0f;// in s
        cruiseTime     = 0.0f;// in s
        decelTime      = 0.0f;// in s
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
    inline Vector3d&    GetVelocity()                   { return myVelocity; }
    inline const Vector3d& GetDesiredHeading() const    { return desiredHeading; }
    inline Vector3d&    GetDesiredHeading()             { return desiredHeading; }
    inline double       GetSpeed() const                { return myVelocity.Length(); }
    inline float        GetSpeedFraction() const        { return activeSpeedFraction; }
    inline float        GetMaxSpeed() const             { return maxSpeed; }
    inline double       GetAgility() const              { return agility; }
    inline uint8        GetBallMode() const             { return ballMode; }
    inline int32        GetTicStamp() const             { return ticStamp; }
    inline double       GetWarpSpeed() const            { return warpSpeed; }
    inline double       GetWarpCapNeed() const          { return warpCapNeed; }
    inline SpaceEntity* GetMySE() const                 { return mySE; }
    inline SpaceEntity* GetTargetSE() const             { return targetSE; }
    inline const Vector3d& GetTargetPoint() const       { return targetPoint; }
    inline double       GetTargetDistance() const       { return static_cast<double>(targetDistance); }
    inline double       GetDesiredDistance() const      { return static_cast<double>(desiredDistance); }
    inline const float  GetUSF() const                  { return userSpeedFraction; }
    inline const float  GetASF() const                  { return activeSpeedFraction; }

    // --- State Checks ---
    inline bool 	IsMoving() const    		{ return (activeSpeedFraction > 0.01f); }
    inline bool 	IsGoto() const      		{ return (ballMode == Ball::Mode::GOTO); }
    inline bool 	IsStopped() const   		{ return (ballMode == Ball::Mode::STOP); }
    inline bool 	IsOrbiting() const  		{ return (ballMode == Ball::Mode::ORBIT); }
    inline bool 	IsFollowing() const 		{ return (ballMode == Ball::Mode::FOLLOW); }
    inline bool         IsFormation() const             { return (ballMode == Ball::Mode::FORMATION); }
    inline bool 	IsTractored() const 		{ return tractored; }
    inline bool         IsAutoPilot() const             { return autoPilot; }
    inline bool         IsWarping() const               { return warpState.active; }
    inline bool         IsCloaked() const               { return cloaked; }
    inline bool         IsUndocking() const             { return undocking; }
    inline bool         IsMassive() const               { return massive; }

    // Setters
    inline void         SetMySE(SpaceEntity* set)       { mySE = set; }
    inline void         SetTargetSE(SpaceEntity* set)   { targetSE = set; }
    inline void         SetHeading(const Vector3d& set) { desiredHeading = set; }
    inline void         SetMyVelocity(const Vector3d& set){ myVelocity = set; }
    inline void         SetDesiredVelocity(const Vector3d& set){ desiredVelocity = set; }
    inline void         SetDesiredDistance(int64 set)   { desiredDistance = set; }
    inline void         SetTargetDistance(int64 set)    { targetDistance = set; }
    inline void         SetWarpCapNeed(double set)      { warpCapNeed = set; }
    inline void         SetWarpDropSpeed(double set)    { speedToLeaveWarp = set; }
    inline void         SetTargetPoint(const Vector3d& set) { targetPoint = set; }
    inline void 	SetSkipTic(bool set=true) 	{ skipTic = set; }
    inline void         SetTractored(bool set=true)     { tractored = set; }
    inline void         SetTractorPause(bool set=true)  { tractorPause = set; }
    inline void         SetMassive(bool set=true)       { massive = set; }
    inline void         SetStop(bool set=true)          { stop = set; }
    inline void         SetAutoPilot(bool set=true)     { autoPilot = set; }
    inline void         SetCloaked(bool set=true)       { cloaked = set; }
    inline void         SetSpeed(float set=100.0f)      { maxSpeed = set; }
    inline void         SetBallMode(uint8 set=0)        { ballMode = set; }
    inline void         SetTicStamp(int32 set=0)        { ticStamp = set; }
    inline void         SetUndocking(bool set=true)     { undocking = set; }
    inline void         SentUpdates(bool set=true)      { sentUpdates = set; }

    inline void         SetUSF(float set)               { userSpeedFraction = set; }
    inline void         SetASF(float set)               { activeSpeedFraction = set; }

    void                SetTrollData(MoveData* parentData);  // as stated.

    void                SetAgilityInertia();
    void                UpdateShipVariables();

    // --- Core Updaters & Modifiers ---
    void                Clear();
    bool                ValidTarget();
    void                EntityRemoved(SpaceEntity* pSE);

    void                WebbedMe();
    void                SpeedBoost(bool deactivate = false);

    void                TractorBeamStart(SpaceEntity* pSE, EvilNumber speed);
    void                TractorBeamStop();

    // Friend for BallPark's static math engine direct access
    friend class        BallPark;

    WarpState           warpState;

private:
    // --- 8-byte aligned members ---
    SpaceEntity*        mySE;           // pointer to my SE (8 bytes)
    SpaceEntity*        targetSE;       // null or pointer to targetSE (8 bytes)

    // used for any preliminary 3d point or direction, but reset in CalculateFollowPoint
    Vector3d            targetPoint;    // required
    Vector3d            myVelocity;
    // used by BallPark *veloctiy methods to set desiredVelocity and used in alignTo checks (vs current heading)
    Vector3d            desiredHeading; // required
    // used by blender to set actual movement and position
    Vector3d            desiredVelocity;

    double              expTerm;
    double              posScale;
    double              timeFactor;     // dont remember what this is for...currently not used
    double              agility;
    double              timeStamp;      // filetime command received
    double              alignTime;
    double              warpSpeed;      // ship base warp speed.  can change in WarpInit() based on distance
    double              speedToLeaveWarp;
    double              warpCapNeed;    // this is remaining cap after warp inits

    int64               targetDistance; // actual distance to target.  originally set in beyonce
    int64               desiredDistance; // desired distance for orbit, follow, warpto

    // --- 4-byte aligned members ---
    int32               ticStamp;       // this is the state tic in which this client's ballpark was started
    float               maxSpeed;       // required
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
    bool                cloaked;
    bool                undocking;
};
