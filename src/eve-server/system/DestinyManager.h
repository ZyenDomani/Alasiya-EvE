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
    Author:        Zhur, Aknor Jaden
    Updates:        Allan
*/

#ifndef __DESTINYMANAGER_H_INCL__
#define __DESTINYMANAGER_H_INCL__

#include "eve-compat.h"
#include "PyCallable.h"
#include "destiny/DestinyStructs.h"
#include "inventory/ItemRef.h"

//0=no orbit, >0=in orbit, 1=at distance 2=too close , 3=too far, 4=way too close, 5=way too far
namespace Destiny {
    namespace Ball {
        struct stateStamp {
            uint8 state;
            uint16 time;
        };
        struct timeStamp {
            uint8 mode;
            int64 time;
        };
        namespace Orbit {
            enum {
                None = 0,
                Orbiting = 1,
                Close = 2,
                Far = 3,
                TooClose = 4,
                TooFar = 5
            };

        }
    }
}

class InventoryItem;
class Missile;
class PyRep;
class PyList;
class PyTuple;
class Ship;
class SystemBubble;
class SystemEntity;
class SystemManager;

// common variables to denote accpetable alignment deviations
static const float TURN_ALIGNMENT = 4.0f;
static const float WARP_ALIGNMENT = 6.0f;
static const uint16 BUMP_DISTANCE = 50;     //in meters.  < this = hit.

/*
namespace Destiny {
    namespace Warp {
        struct State {
            uint32 start_time;          //from sEntityList::GetStamp()
            double total_distance;      //in m
            double warpSpeed;           //in m/s
            double accelDist;           //in m
            double cruiseDist;          //in m
            double decelDist;           //in m
            float warpTime;             //in s
            bool accel;
            bool cruise;
            bool decel;
            GVector warp_vector;        //target direction based on ship's initial position
        };
    }
}
*/

//this object manages an entity's position and movement in a system.

class DestinyManager {
public:
    DestinyManager(SystemEntity* self);
    ~DestinyManager();

    void Process();

    void SendSingleDestinyEvent(PyTuple** ev, bool self_only=false) const;
    void SendSingleDestinyUpdate(PyTuple** up, bool self_only=false) const;
    void SendDestinyUpdate(std::vector<PyTuple*> &updates, bool self_only=false) const;
    void SendDestinyUpdate(std::vector<PyTuple*> &updates, std::vector<PyTuple*> &events, bool self_only=false) const;

    /* Informational query functions: */
    const GPoint &GetPosition() const                   { return m_position; }
    const GVector &GetVelocity() const                  { return m_velocity; }
    float GetSpeedFraction()                            { return m_currentSpeedFraction; }
    float GetSpeed()                                    { return (m_maxShipSpeed * m_currentSpeedFraction); }

    // this is only used by my bubble debug command
    uint8 GetState()                                    { return m_ballMode; }

    void EntityRemoved(SystemEntity* pSE);

    /* Configuration methods */
    void WebbedMe(InventoryItemRef modRef, bool apply=false);
    void SetBubble(bool set = false)                    { m_inBubble = set; }
    void SpeedBoost(bool deactivate=false);             // reset speed variables and bubblecast ship's AB/MWD modified speed (module activate/deactivate)
    void SetPosition(const GPoint& pt, bool update = false);
    void SetMaxVelocity(float maxVelocity);
    void UpdateShipVariables();

    /* Global Actions */
    void Stop();
    void Halt();     // puts entity at 0 velocity
    void Eject();   // avoid numerous other redirect calls

    /* TractorBeam */
    void TractorBeamStop();
    void TractorBeamStart(SystemEntity* pShipSE, EvilNumber speed);

    /* Local Movement */
    void Orbit(SystemEntity* pSE, double distance=0);
    void Follow(SystemEntity* pSE, double distance=0);
    void AlignTo(SystemEntity* pSE);
    void GotoPoint(const GPoint &point);
    void GotoDirection(const GPoint &direction);
    void SetSpeedFraction(float fraction=1.0f, bool startMovement=false);

    /* Larger movement */
    void WarpTo(const GPoint& where, int32 distance = 0, bool autoPilot = false, SystemEntity* pSE = nullptr);

    /* Ship State Query functions */
    bool IsMoving()                                     { return (m_currentSpeedFraction > 0); }

    /* Movement checks */
    bool IsAligned(GPoint &targetPoint);
    bool IsGoto()                                       { return (m_ballMode == Destiny::Ball::Mode::GOTO); }
    bool IsStopped()                                    { return (m_ballMode == Destiny::Ball::Mode::STOP); }
    bool IsOrbiting()                                   { return (m_ballMode == Destiny::Ball::Mode::ORBIT); }
    bool IsFollowing()                                  { return (m_ballMode == Destiny::Ball::Mode::FOLLOW); }
    //bool IsJumping()                                  { return (m_ballMode == Destiny::Ball::Mode::STOP); }
    bool IsWarping()                                    { return (m_warpState ? true : false); }
    bool IsCloaked()                                    { return m_cloaked; }
    bool IsTurning()                                    { return m_turning; }
    bool IsTractored()                                  { return m_tractored; }

    //Destiny Update stuff:
    void Jump();
    void Cloak();
    void UnCloak();

    PyResult AttemptDockOperation();
    void Undock(GPoint dir);
    void SetUndockSpeed();
    void DockingAccepted();
    void SendSetState() const;
    void SendJumpOut(uint32 gateID) const;
    void SendGateActivity(uint32 gateID) const;
    void SendJumpInEffect(std::string JumpEffect) const;
    void SendJumpOutEffect(std::string JumpEffect, uint32 locationID) const;
    void SendTerminalExplosion(uint32 shipID, uint32 bubbleID, bool isGlobal=false) const;
    void SendBallInteractive(const ShipItemRef shipRef, bool set = false) const;
    void UpdateNewShip(const ShipItemRef newShipRef);
    void UpdateOldShip(Ship* pShipSE);
    void SendJettisonPacket() const;
    void SendAnchorDrop() const;
    void SendAnchorLift() const;
    void SendCloakFx(bool apply=false, bool module=false) const;
    void SendSpecialEffect10(uint32 entityID, uint32 targetID, std::string guid, bool isOffensive, bool start, bool isActive) const;
    void SendSpecialEffect(uint32 entityID, uint32 moduleID, uint32 moduleTypeID, uint32 targetID, uint32 chargeTypeID, std::string guid,
                           bool isOffensive, bool start, bool isActive, double duration, uint32 repeat, int32 graphicInfo=0) const;

    //  functions to return protected variables for SystemBubble exclusive WarpTo updates and other methods that need Destiny Variables
    int32 GetDistance()                                 { return m_stopDistance; }
    int32 GetWarpSpeed()                                { return static_cast<int32>(m_shipWarpSpeed * 10); }
    uint32 GetTargetID()                                { return m_targetEntity.first; }
    SystemEntity* GetTargetEntity()                     { return m_targetEntity.second; }
    GPoint GetTargetPoint()                             { return m_targetPoint; }
    double GetMaxVelocity()                             { return m_maxShipSpeed; }
    double GetFollowDistance()                          { return m_targetDistance; }
    double GetMass()                                    { return m_mass; }
    double GetAgility()                                 { return m_shipAgility; }
    double GetInertia()                                 { return m_shipInertia; }
    uint32 GetStateStamp()                              { return m_stateStamp; }
    GVector GetHeading()                                { return m_shipHeading; }

    float GetAlignTime()                                { return m_alignTime; }
    float GetAccelTime()                                { return m_shipMaxAccelTime; }
    float GetWarpTime()                                 { return m_timeToEnterWarp; }
    float GetWarpDropSpeed()                            { return m_speedToLeaveWarp; }
    double GetRadius()                                  { return m_radius; }
    double GetCapNeed()                                 { return m_warpCapacitorNeed; }

    float GetRadTic()                                   { return m_orbitRadTic; }

    void MakeMissile(Missile* missile);

protected:
    void ProcessState();

    SystemEntity* const mySE;			//we do not own this.
    SystemBubble* m_targBubble;         //we do not own this.

    bool IsTargetInvalid();              //performs common target checks

    bool m_hasSentShipUpdates;

    //things dictated by our entity's configuration:
    uint8 m_warpAccelTime;              //in s      - calculated internally for warp stages
    uint8 m_warpDecelTime;              //in s      - calculated internally for warp stages

    float m_mass;                       //in kg
    float m_massMKg;                    //in Millions of kg
    float m_alignTime;                  //in s      - align and enter warp are same (for our purposes)
    float m_prevSpeed;                  //in m/s    - used by decel when deactivating prop mod
    float m_maxShipSpeed;               //in m/s
    float m_shipWarpSpeed;              //in au/s
    float m_timeToEnterWarp;            //in s
    float m_speedToLeaveWarp;           //in m/s    - this is set to 75% of m_maxShipSpeed

    double m_radius;                    //in m
    double m_capNeeded;                 //in GJ     - variable to drain cap during warp init
    double m_warpCapacitorNeed;         //in GJ     - capacitor charged needed to initiate warp
    // ship motion factors for complicated maths
    double m_shipAgility;               //in s/Mkg  - time-constant of movement for objects in eve physics (and 't' in Dr. SS's calculations)
                                        //          - characteristic of time that governs the rate of change in motion of an object
    double m_shipInertia;               //in s/Mkg  - reciprocal of drag constant in EvE
                                        //          - the drag coefficient is 1/I and in Mkg/s

    //derived from above params:
    float m_maxSpeed;                   //in m/s
    float m_degPerTic;                  //in deg/s  - used to determine rate of direction change
    float m_shipMaxAccelTime;           //in s      - used to determine accel rate, and total accel time

    double m_radians;                   //in rad    - radians left in an ongoing turn

    GPoint m_position;                  //in m
    GVector m_velocity;                 //in m/s

    //User controlled information used by a state to determine what to do.
    bool m_stop;                        //used to denote Stop() has been called to avoid multiple stops (and associated decel)
    bool m_accel;                       //used for raising ship speed via speedo
    bool m_decel;                       //used for lowering ship speed via speedo
    bool m_cloaked;
    bool m_turning;                     //used to denote ship turning for associated checks
    bool m_inBubble;                    //used to tell if client is in bubble or not.
    bool m_tractored;
    bool m_tractorPause;

    uint8 m_ballMode;                   //current state of ball

    int32 m_stopDistance;               //from destination, in m

    uint8 m_turnTic;                    //time into turn
    int8 m_orbiting;                    // 0=no orbit, >0=in orbit, 1=at distance, 2=too close , 3=too far, 4=way too close, 5=way too far
    //Destiny::Ball::stateStamp m_stateStamp; //state and count of current state since beginning, in seconds
    //Destiny::Ball::timeStamp m_timeStamp; //mode and timestamp of when current mode began
    uint32 m_stateStamp;                //statestamp of when current state began, in seconds

    float m_orbitTime;                  //in s - time to complete one orbit using current variables
    float m_orbitRadTic;                //in rad/sec  - radians around orbit per tic
    float m_turnFraction;               //fuzzy logic - speed % - used for turn accel/decel checks
    float m_prevSpeedFraction;          //fuzzy logic - speed % - previous speed fraction used for decel checks when (m_userSpeedFraction == 0)
    float m_userSpeedFraction;          //fuzzy logic - speed % - set by user command
    float m_currentSpeedFraction;       //fuzzy logic - speed % - holds current euler value for time
    float m_activeSpeedFraction;        //fuzzy logic - speed % - ship's current speed setting as ratio of CSF to USF (or OSF)
    float m_maxOrbitSpeedFraction;      //fuzzy logic - speed % - ship's max speed based on orbit data

    double m_targetDistance;            //in m
    double m_followDistance;            //in m
    double m_moveTime;                  //in ms     - movement timestamp container for calculating csf

    GPoint m_targetPoint;
    GVector m_shipHeading;              //direction ship is facing
    GVector m_targetHeading;            //direction to target from current heading  -- should this be the *actual* heading of our current target??
    std::pair<uint32, SystemEntity*> m_targetEntity;   //we do not own the SystemEntity*

    // movement methods
    void MoveObject();                  //apply velocity to our position for for this round of movement
    void Orbit();
    void Follow();                      //follow or approach object in space
    void BeginMovement();               //set initial variables for all movement (common code)
    void UpdateVelocity(bool isMoving=false);

private:
    bool m_changeDelay;                 // this is to try to sync destiny with client, as client has a delay when changing destiny states.

    // Internal Collision Methods   -allan Nov 2015
    bool m_bump;
    void CheckBump();                              //iterate thru objects in current bubble to check for collisions
    void Bump(SystemEntity* who);                  //math methods for determining direction and speed of bumped ships
    void Bounce(GVector direction, float speed);   //packet sending for ships after bounce

    // Internal Turn Methods    -allan  Aug - Oct, 2015
    bool IsTurn();                     //check for current heading vs target direction. return true if degrees > 2 for warp align and > 0.8 for normal movement
    void Turn();                       //apply velocity and heading updates as needed for turning
    void ClearTurn();

    // Internal Orbit shit      -allan  Jan 2020
    GPoint ComputePosition(double curRad);   // currently testing...wip
    double m_inclination;               //inclination of orbit
    double m_longAscNode;               //longitude of ascending node
    void ClearOrbit();

    // Internal Warp Methods
    Timer m_warpTimer;
    void InitWarp();
    void WarpAccel(uint16 sec_into_warp);
    void WarpCruise(uint16 sec_into_warp);
    void WarpDecel(uint16 sec_into_warp);
    void WarpStop(double currentShipSpeed);
    void WarpUpdate(double currentShipSpeed);

    // Variables used during Warp.
    class WarpState {
    public:
        WarpState(
            uint32 start_time_,
            double total_distance_,
            double warp_speed_,
            double accel_dist_,
            double cruise_dist_,
            double decel_dist_,
            float warp_time_,
            bool accel_,
            bool cruise_,
            bool decel_,
            const GVector &warp_vector_)
        : start_time(start_time_),
        total_distance(total_distance_),
        warpSpeed(warp_speed_),
        accelDist(accel_dist_),
        cruiseDist(cruise_dist_),
        decelDist(decel_dist_),
        warpTime(warp_time_),
        accel(accel_),
        cruise(cruise_),
        decel(decel_),
        warp_vector(warp_vector_)
        {}
        uint32 start_time;          //from sEntityList::GetStamp()
        double total_distance;      //in m
        double warpSpeed;           //in m/s
        double accelDist;           //in m
        double cruiseDist;          //in m
        double decelDist;           //in m
        float warpTime;             //in s
        bool accel;
        bool cruise;
        bool decel;
        GVector warp_vector;        //target direction based on ship's initial position
    };
    WarpState* m_warpState;		    //we own this.
};

#endif
