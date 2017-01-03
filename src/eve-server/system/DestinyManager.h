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

class InventoryItem;
class Missile;
class PyRep;
class PyList;
class PyTuple;
class SystemBubble;
class SystemEntity;
class SystemManager;

// common variables to denote accpetable alignment deviations
static const float TURN_ALIGNMENT = 4.0f;
static const float WARP_ALIGNMENT = 6.0f;
static const uint16 BUMP_DISTANCE = 50;     //in meters.  < this = hit.

//this object manages an entity's position and movement in a system.

class DestinyManager {
public:
    DestinyManager(SystemEntity* self);
    ~DestinyManager();

    void Process();

    void SendSingleDestinyUpdate(PyTuple** up, bool self_only=false) const;
    void SendDestinyUpdate(std::vector<PyTuple*> &updates, bool self_only=false) const;
    void SendDestinyUpdate(std::vector<PyTuple*> &updates, std::vector<PyTuple*> &events, bool self_only=false) const;

    /* Informational query functions: */
    const GPoint &GetPosition() const                   { return m_position; }
    const GVector &GetVelocity() const                  { return m_velocity; }
    float GetSpeedFraction()                            { return m_currentSpeedFraction; }
    float GetSpeed()                                    { return (m_maxShipSpeed * m_currentSpeedFraction); }
    Destiny::BallMode GetState()                        { return State; }

    void EntityRemoved(SystemEntity* who);

    /* Configuration methods */
    void SetBubble(bool set = false)                    { m_inBubble = set; }
    void SetPosition(const GPoint& pt, bool update = false);
    void SetMaxVelocity(float maxVelocity);
    void SetShipCapabilities(const InventoryItemRef ship, bool undock = false);

    /* Global Actions */
    void Stop();
    void Halt();     // puts entity at 0 velocity

    /* TractorBeam */
	void TractorBeamStop();
    void TractorBeamStart(SystemEntity* pShipSE);

    /* Local Movement */
    void Orbit(SystemEntity* who, double distance=0);
    void Follow(SystemEntity* pSE, double distance);
    void AlignTo(SystemEntity* ent);
    void GotoPoint(const GPoint &point);
    void GotoDirection(const GPoint &direction);
    void SetSpeedFraction(float fraction=1.0f, bool startMovement=false);

    /* Larger movement */
    void WarpTo(const GPoint& where, int32 distance = 0);

    /* Ship State Query functions */
    bool IsMoving()                                     { return (m_currentSpeedFraction ? true : false); }

    /* Movement checks */
    bool IsAligned(GPoint &targetPoint);
    bool IsGoto()                                       { return (State == Destiny::DSTBALL_GOTO); }
    bool IsStopped()                                    { return (State == Destiny::DSTBALL_STOP); }
    bool IsOrbiting()                                   { return (State == Destiny::DSTBALL_ORBIT); }
    bool IsFollowing()                                  { return (State == Destiny::DSTBALL_FOLLOW); }
    //bool IsJumping()                                  { return (State == Destiny::DSTBALL_STOP) ? true : false); }
    bool IsWarping()                                    { return (m_warpState ? true : false); }
	bool IsCloaked()                                    { return m_cloaked; }
	bool IsTurning()                                    { return m_turning; }

	//Destiny Update stuff:
	void Jump();
	void Cloak();
    void UnCloak();

    PyResult AttemptDockOperation();
    void Dock();
    void Undock(GPoint dir);
    void SetUndockSpeed();
    void SendSetState() const;
    void SendJumpOut(uint32 gateID) const;
    void SendGateActivity(uint32 gateID) const;
	void SendJumpInEffect(std::string JumpEffect) const;
	void SendJumpOutEffect(std::string JumpEffect, uint32 locationID) const;
    void SendTerminalExplosion(uint32 shipID, uint32 bubbleID, bool isGlobal=false) const;
    void SendBallInteractive(const ShipItemRef shipRef, bool set = false) const;
    void UpdateNewShip(const ShipItemRef newShipRef);
    void UpdateOldShip(const ShipItemRef oldShipRef);
    void SendJettisonPacket() const;
    void SendAnchorDrop() const;
    void SendAnchorLift() const;
    void SendCloakShip(const bool IsWarpSafe) const;
    void SendUncloakShip() const;
	void SendSpecialEffect10(uint32 gateID, const ShipItemRef shipRef, uint32 targetID, std::string effectString, bool isOffensive, bool start, bool isActive) const;
	void SendSpecialEffect(const ShipItemRef shipRef, uint32 moduleID, uint32 moduleTypeID,
    uint32 targetID, uint32 chargeTypeID, std::string effectString, bool isOffensive, bool start, bool isActive, double duration, uint32 repeat) const;

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

    void MakeMissile(Missile* missile);

protected:
    void ProcessState();

    SystemEntity* const mySE;			//we do not own this.

    bool _IsTargetInvalid();              //performs common target checks

    //things dictated by our entity's configuration:
    int8 m_warpStrength;                //signed    - >0 means ship cannot warp (warp stabs are neg values, warp scrams are pos values)

    uint8 m_warpAccelTime;              //in s      - calculated internally for warp stages
    uint8 m_warpDecelTime;              //in s      - calculated internally for warp stages

    float m_mass;                       //in kg
    float m_massMKg;                    //in Millions of kg
    float m_alignTime;                  //in s      - align and enter warp are same (for our purposes)
    float m_maxShipSpeed;               //in m/s
    float m_shipWarpSpeed;              //in au/s
    float m_timeToEnterWarp;            //in s
    float m_speedToLeaveWarp;           //in m/s    - this is set to 75% of m_maxShipSpeed

    double m_radius;                    //in m
    double m_capNeeded;                 //in GJ     - capacitor charged needed to initiate warp
    double m_warpCapacitorNeed;         //in GJ     - capacitor charged needed to initiate warp
    // ship motion factors for complicated maths
    double m_shipAgility;               //in s/Mkg  - time-constant of movement for objects in eve physics (and 't' in Dr. SS's calculations)
                                        //          - characteristic of time that governs the rate of change in motion of an object
    double m_shipInertia;               //in s/Mkg  - reciprocal of drag constant in EvE
                                        //          - the drag coefficient is 1/I and in Mkg/s

    //derrived from above params:
    float m_maxSpeed;                   //in m/s
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

    int32 m_stopDistance;               //from destination, in m

    int8 m_orbiting;                    //specific to orbiting (only)
    uint8 m_turnTic;                    //time into turn
    uint32 m_stateStamp;                //timestamp of when current state began, in seconds

    float m_orbitRadTic;                //in rad/sec  - for orbiting
    float m_userSpeedFraction;          //fuzzy logic - speed % - set by user command
    float m_currentSpeedFraction;       //fuzzy logic - speed % - current ship speed
    float m_activeSpeedFraction;        //fuzzy logic - speed % - set by USF and CSF
    float m_maxOrbitSpeedFraction;      //fuzzy logic - speed % - set by Orbit()

    double m_targetDistance;            //in m
    double m_followDistance;            //in m
    double m_moveTimer;                 //in ms
    double m_orbitTimer;                //in ms

    GPoint m_targetPoint;
    GVector m_shipHeading;              //direction ship is facing
    GVector m_targetHeading;            //direction to target from current heading  -- should this be the *actual* heading of our current target??
    Destiny::BallMode State;
    std::pair<uint32, SystemEntity*> m_targetEntity;   //we do not own the SystemEntity*

    // movement methods
    void _Move();                       //apply velocity to our position for for this round of movement
    void _Orbit();
    void _Follow();                     //follow or approach object in space
    void _BeginMovement();              //set initial variables for all movement (common code)
    void _UpdateVelocity(bool isMoving=false);

private:
    // Internal Collision Methods   -allan Nov 2015
    bool m_bump;
    void _CheckBump();                              //iterate thru objects in current bubble to check for collisions
    void _Bump(SystemEntity* who);                  //math methods for determining direction and speed of bumped ships
    void _Bounce(GVector direction, float speed);   //packet sending for ships after bounce

    // Internal Turn Methods    -allan  Aug - Oct, 2015
    bool _IsTurn();                     //check for current heading vs target direction. return true if degrees > 2 for warp align and > 0.8 for normal movement
    GVector _Turn();                    //apply velocity and heading updates as needed for turning
    void _ClearTurn();

    // Internal Warp Methods
    Timer m_warpTimer;
    void _InitWarp();
    void _WarpAccel(uint16 sec_into_warp);
    void _WarpCruise(uint16 sec_into_warp);
    void _WarpDecel(uint16 sec_into_warp);
    void _WarpStop(double currentShipSpeed);
    void _WarpUpdate(double currentShipSpeed);

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
