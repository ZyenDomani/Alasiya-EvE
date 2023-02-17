/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
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
    Author:        Zhur, Aknor Jaden
    Rewrite:        Allan
	Update:        Allan (organized header - 15Feb23)
*/

#ifndef __SERVER_SYSTEM_DESTINY_H
#define __SERVER_SYSTEM_DESTINY_H

#include "eve-compat.h"
#include "PyCallable.h"
#include "destiny/DestinyStructs.h"
#include "inventory/ItemRef.h"


// common variables to denote acceptable alignment deviations
static const float TURN_ALIGNMENT = 4.0f;       //0.0698132 rad
static const float WARP_ALIGNMENT = 6.0f;       //0.105 rad

// for testing only.  this isnt right.
static const uint16 BUMP_DISTANCE = 50;     //in meters.  <= this is contact.


namespace Destiny {
    namespace Ball {
        struct WarpState {  // not used.
            bool accel;
            bool cruise;
            bool decel;
            uint32 startStamp;          //from sEntityList::GetStamp()
            float warpTime;             //in s
            double total_distance;      //in m
            double warpSpeed;           //in m/s
            double accelDist;           //in m
            double cruiseDist;          //in m
            double decelDist;           //in m
            GVector warp_vector;        //target direction based on ship's initial position
        };

        struct stateStamp {  // not used.
            uint8 state;
            uint16 time;
        };
        struct timeStamp {  // not used.
            uint8 mode;
            int64 time;
        };
        namespace Orbit {
            enum {
                None            = 0,
                Orbiting        = 1,
                Close           = 2,
                Far             = 3,
                TooClose        = 4,
                TooFar          = 5
            };
        }
    }
}

class InventoryItem;
class Missile;
class PyRep;
class PyList;
class PyTuple;
class ShipSE;
class SystemBubble;
class SystemEntity;
class SystemManager;


//this object manages an entity's position and movement in a system.

class DestinyManager {
public:
    DestinyManager(SystemEntity* self);
    ~DestinyManager();

    void Process();

    /* Informational query functions: */
    //  functions to return protected variables for SystemBubble exclusive WarpTo updates and other methods that need Destiny Variables
    const GPoint &GetPosition() const                   { return m_position; }
    const GVector &GetVelocity() const                  { return m_velocity; }
    float GetSpeedFraction()                            { return m_activeSpeedFraction; }
    float GetSpeed()                                    { return (m_maxShipSpeed * m_activeSpeedFraction); }
    int32 GetWarpSpeed()                                { return static_cast<int32>(m_shipWarpSpeed * 10); }
    uint32 GetTargetID()                                { return m_targetEntity.first; }
    SystemEntity* GetTargetEntity()                     { return m_targetEntity.second; }
    GPoint GetTargetPoint()                             { return m_targetPoint; }
    double GetMaxVelocity()                             { return m_maxShipSpeed; }
    double GetFollowDistance()                          { return m_targetDistance; }
    uint32 GetStateStamp()                              { return m_stateStamp; }
    GVector GetHeading()                                { return m_shipHeading; }
    float GetAccelTime()                                { return m_shipMaxAccelTime; }
    uint8 GetAlignTime()                                { return m_alignTime; } // this is only used by my GetShipVars command
    float GetWarpDropSpeed()                            { return m_speedToLeaveWarp; }
    double GetRadius()                                  { return m_radius; }
    double GetCapNeed()                                 { return m_warpCapacitorNeed; }
    float GetRadTic()                                   { return m_orbitRadTic; }
    uint8 GetState()                                    { return m_ballMode; }// this is only used by my bubble debug command
    bool IsFrozen()                                     { return m_frozen; }
    bool IsMoving()                                     { return (m_activeSpeedFraction > 0.0005); }
    bool IsGoto()                                       { return (m_ballMode == Destiny::Ball::Mode::GOTO); }
    bool IsStopped()                                    { return (m_ballMode == Destiny::Ball::Mode::STOP); }
    bool IsOrbiting()                                   { return (m_ballMode == Destiny::Ball::Mode::ORBIT); }
    bool IsFollowing()                                  { return (m_ballMode == Destiny::Ball::Mode::FOLLOW); }
    bool IsWarping()                                    { return (m_warpState != nullptr); }
    bool IsCloaked()                                    { return m_cloaked; }
    bool IsTurning()                                    { return m_turning; }
    bool IsTractored()                                  { return m_tractored; }
    bool IsAligned(GPoint &targetPoint);

    /* Configuration methods */
    void WebbedMe(InventoryItemRef modRef, bool apply=false);
	// reset speed variables and bubblecast ship's AB/MWD modified speed (module activate/deactivate)
    void SpeedBoost(bool deactivate=false);
    void SetPosition(const GPoint& pt, bool update=false);
    void SetMaxVelocity(float maxVelocity);
    void UpdateShipVariables();
    // set all movement vars for missile and add to system
    void MakeMissile(Missile* missile);  //  this is used by all entities (pc, npc, drone, sentry, pos, etc)

    /* Global Actions */
    void Stop();
    void Halt(bool commanded=false);			// puts entity at 0 velocity
    void Eject();								// avoid numerous other redirect calls
    void EntityRemoved(SystemEntity* pSE);
    void SetCloak(bool set=false)                       { m_cloaked = set; }
    void SetFrozen(bool set=false)                      { m_frozen = set; }
    void SetMoveTimeNow()                               { m_moveTime = GetTimeMSeconds(); } // called only by Beyonce from CmdSetSpeedFraction()
    void UpdateSpeedFraction(float speedPct=0);  // called only by Beyonce from CmdSetSpeedFraction()

    /* Local Movement */
    void InitOrbit(SystemEntity* pSE, uint32 distance=0);
    void Follow( SystemEntity* pSE, uint32 distance );
    void AlignTo(SystemEntity* pSE);
    void GotoPoint(const GPoint &point);
    void GotoDirection(const GPoint &direction);
    void SetSpeedFraction(float fraction=1.0f, bool startMovement=false);

    /* TractorBeam */
    void TractorBeamStop();
    void TractorBeamStart(SystemEntity* pShipSE, EvilNumber speed);

    /* Larger movement */
    void WarpTo(const GPoint& destPoint, int32 distance = 0, bool autoPilot = false, SystemEntity* pSE = nullptr);

    //Destiny Update stuff:
    PyResult AttemptDockOperation();
    void Jump(bool showCloak=true);
    void Cloak();
    void UnCloak();
    void Undock(GPoint dir);
    void SetUndockSpeed();
    void DockingAccepted();
    void SendSetState() const;
    void SendJumpOut(uint32 gateID) const;
    void SendJumpOutWormhole(uint32 wormholeID) const;
    void SendGateActivity(uint32 gateID) const;
    void SendWormholeActivity(uint32 wormholeID) const;
    void SendJumpInEffect(std::string JumpEffect) const;
    void SendJumpOutEffect(std::string JumpEffect, uint32 shipID) const;
    void SendTerminalExplosion(uint32 shipID, uint32 bubbleID, bool isGlobal=false) const;
    void SendBallInteractive(const ShipItemRef shipRef, bool set = false) const;
    void UpdateNewShip(const ShipItemRef newShipRef);
    void UpdateOldShip(ShipSE* pShipSE);
    void SendJettisonPacket() const;
    void SendAnchorDrop() const;
    void SendAnchorLift() const;
    void SendCloakFx(bool apply=false, bool module=false) const;
	// these need to be updated
    void SendSpecialEffect10(uint32 entityID, uint32 targetID, std::string guid, bool isOffensive, bool start, bool isActive) const;
    void SendSpecialEffect(uint32 entityID, uint32 moduleID, uint32 moduleTypeID, uint32 targetID, uint32 chargeTypeID, std::string guid, bool isOffensive, bool start, bool isActive, int32 duration, uint32 repeat, int32 graphicInfo = 0) const;

    /* update and event queue methods */
    void SendSingleDestinyEvent(PyTuple** ev, bool self_only=false) const;// this will not consume *ev
    void SendSingleDestinyUpdate(PyTuple** up, bool self_only=false) const; // this will not consume *up
    void SendDestinyUpdates(std::vector<PyTuple*> &updates, bool self_only=false) const;// this will consume all updates in vector

protected:
    void ProcessState();				// determine method to call based on ball state
    bool IsTargetInvalid();              //performs common target checks

    // movement methods
    void MoveObject();                  //apply velocity to our position for this round of movement
    void Orbit();
    void Follow();                      //follow or approach object in space
    void BeginMovement();               //set initial variables for all movement (common code)
    void UpdateVelocity(bool isMoving=false);

    SystemEntity* const mySE;			//we do not own this.
    SystemBubble* m_targBubble;         //we do not own this.

    uint8 m_ballMode;                   //current state of ball
    bool m_hasSentShipUpdates;

	// things dictataed by ship and skills
    double m_radius;                    //in m
    double m_warpCapacitorNeed;         //in GJ     - capacitor charged needed to initiate warp

    //things dictated by our entity's configuration:
    uint8 m_alignTime;                  //in s      - time to change directions or enter warp
    float m_prevSpeed;                  //in m/s    - used to calculate speed during decel
    float m_maxShipSpeed;               //in m/s
    float m_shipWarpSpeed;              //in au/s
    float m_speedToLeaveWarp;           //in m/s    - this is set to 75% of m_maxShipSpeed

    //derived from above params:
    float m_maxSpeed;                   //in m/s    - derived from m_maxShipSpeed * m_userSpeedFraction
    float m_shipAccelTime;              //in s      - used to check time for speed change
    float m_shipMaxAccelTime;           //in s      - used to determine accel rate, and total accel time

    //User controlled information used by a state to determine what to do.
    bool m_stop;                        //used to denote Stop() has been called to avoid multiple stops (and associated decel)
    bool m_accel;                       //used to execute code for increasing ship speed
    bool m_decel;                       //used to execute code for decreasing ship speed
    bool m_cloaked;
    bool m_turning;                     //used to execute code for ship turning
    bool m_tractored;
    bool m_tractorPause;

    int8 m_orbiting;                    // 0=no orbit, >0=in orbit, 1=at distance, 2=too close , 3=too far, 4=way too close, 5=way too far
    uint32 m_stateStamp;                //statestamp of when current state began, in seconds
    //Destiny::Ball::stateStamp m_stateStamp; //state and count of current state since beginning, in seconds
    //Destiny::Ball::timeStamp m_timeStamp; //mode and timestamp of when current mode began

    float m_degPerTic;                  //ship turn variable
    float m_orbitTime;                  //in s - time to complete one orbit using current variables
    float m_orbitRadTic;                //in rad/sec  - radians around orbit per tic
    float m_radians;                   //in rad    - radians of an ongoing turn

    float m_timeFraction;               //fuzzy logic - holds current euler value for time
    float m_turnMinFraction;            //fuzzy logic - used for turn accel/decel checks
    float m_prevSpeedFraction;          //fuzzy logic - previous percent of full speed.  used for speed changes
    float m_userSpeedFraction;          //fuzzy logic - user commanded percent of max speed
    float m_activeSpeedFraction;        //fuzzy logic - current percent of max speed
    float m_maxOrbitSpeedFraction;      //fuzzy logic - ship's max speed based on orbit data

    uint16 m_turnTime;                  //in s        - time turn started.  now uses EntityList.Stamp()
    uint32 m_followDistance;            //in m
    int64  m_targetDistance;            //in m
    double m_moveTime;                  //in ms       - time when speed change started.  used to calculate m_timeFraction

    GPoint m_position;                  //in m
    GVector m_velocity;                 //in m/s
    GPoint m_targetPoint;               //vector      - point in space used as current destination
    GVector m_shipHeading;              //direction ship is facing
    GVector m_targetHeading;            //direction to target from current heading
    std::pair<uint32, SystemEntity*> m_targetEntity;   //we do not own the SystemEntity*

private:
    bool m_alignTo;                     // once aligned, ship will stop
    bool m_frozen;                      // hack to keep ship from moving when using modules that prevent movement
    bool m_changeDelay;                 // this is to try to sync destiny with client, as client has a delay when changing destiny states.
    bool m_moveDelay;                   // same as above, for less of a delay when changing direction or speed
    double m_agility;                   //unitless?   - not sent to client

    // Internal Collision Methods   -allan Nov 2015
    bool m_bump;
    void CheckBump();                              //iterate thru objects in current bubble to check for collisions
    void Bump(SystemEntity* who);                  //math methods for determining direction and speed of bumped ships
    void Bounce(GVector direction, float speed);   //packet sending for ships after bounce

    // Internal Turn Methods    -allan  Aug - Oct, 2015
    bool IsTurn();                     //check for current heading vs target direction. return true if degrees > 2 for warp align and > 0.8 for normal movement
    void InitTurn();                   //set turn variables
    void Turn(float &speed, std::string &move);   //apply velocity and heading updates as needed for turning.  called by MoveObject()
    void ClearTurn();
    void MarkPoint(const GPoint& position, std::string& name, std::string& desc);
    bool m_posHack;                    //force position update after turn

    // bezier turn data (wip)      -allan  Feb 2023
    bool m_turnAccel;
    bool m_turnDecel;
    float m_turnPct;
    GVector m_origHeading;
    GPoint m_curveStart;
    GPoint m_curveApex;
    GPoint m_curveEnd;
    // return percent change between from and to
    double getPct(double from, double to, float pct) {
        return from + ((to - from) * pct);
    }
    float getPctf(float from, float to, float pct) {
        return from + ((to - from) * pct);
    }

    // Internal Orbit shit
    GPoint ComputePosition(double curRad);   // currently testing...wip
    double m_inclination;               //inclination of orbit
    double m_longAscNode;               //longitude of ascending node
    void ClearOrbit();

    // Internal Warp Methods
    uint32 m_decelTime;
    void InitWarp();
    void WarpAccel(uint16 sec_into_warp);
    void WarpCruise(uint16 sec_into_warp);
    void WarpDecel(uint16 sec_into_warp);
    void WarpStop(int64 currentShipSpeed);
    void WarpUpdate(int64 currentShipSpeed);

    // Variables used during Warp.
    class WarpState {
    public:
        WarpState(
            uint32 start_time_,
            int64 total_distance_,
            int64 warp_speed_,
            int64 accel_dist_,
            int64 cruise_dist_,
            int64 decel_dist_,
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
        int64  total_distance;      //in m
        int64 warpSpeed;           //in m/s
        int64 accelDist;           //in m
        int64 cruiseDist;          //in m
        int64 decelDist;           //in m
        float warpTime;             //in s
        bool accel;
        bool cruise;
        bool decel;
        GVector warp_vector;        //target direction based on ship's initial position
    };
    WarpState* m_warpState;		    //we own this.
};

#endif  // __SERVER_SYSTEM_DESTINY_H

/*  class          inertiaMod                   ~agility
 * Capsule          .06
 * Shuttle          1.6
 * Rookie           5
 * Frigates         3 - 6 (adv. 3 - 4)
 * Destroyers       4 - 5
 * Cruisers         4 - 8
 * T3 Cruiser       2.4 - 2.8
 * HAC              5 - 7
 * Battlecruisers   6 - 9
 * Battleships      8 - 14
 * Industrials      8 - 12
 * Marauder         ~12
 * Orca             40
 * Freighters       ~60
 * Supercarrier     ~60
 * Command          ~9
 * Transport        5 or 19
 * Barges           10 - 18
 * Dreadnought      ~55
 * Zephyr           5
 */

/*#Embedded file name: c:/depot/games/branches/release/EVE-TRANQUILITY/eve/client/script/environment/effects/Repository.py
 * import effects
 * FX_MERGE_NONE = 0
 * FX_MERGE_GUID = 2
 * FX_MERGE_SHIP = 4
 * FX_MERGE_MODULE = 8
 * FX_MERGE_TARGET = 16
 * FX_TF_NONE = 0
 * FX_TF_POSITION_BALL = 2
 * FX_TF_POSITION_TARGET = 4
 * FX_TF_ROTATION_BALL = 8
 * FX_TF_SCALE_SYMMETRIC = 16
 * FX_TF_SCALE_BOUNDING = 32
 * FX_TF_SCALE_RADIUS = 64
 * definitions = {'effects.AnchorDrop': (effects.AnchorDrop,
 *                        FX_TF_NONE,
 *                        FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                        None,
 *                        1,
 *                        10000),
 * 'effects.AnchorLift': (effects.AnchorLift,
 *                        FX_TF_NONE,
 *                        FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                        None,
 *                        1,
 *                        10000),
 * 'effects.ArmorHardening': (effects.ShipRenderEffect,
 *                            FX_TF_NONE,
 *                            FX_MERGE_SHIP | FX_MERGE_GUID,
 *                            'res:/dx9/Model/Effect/ArmorHardening.red',
 *                            1,
 *                            10000),
 * 'effects.ArmorRepair': (effects.ShipRenderEffect,
 *                         FX_TF_NONE,
 *                         FX_MERGE_SHIP | FX_MERGE_GUID,
 *                         'res:/dx9/Model/Effect/ArmorRepair.red',
 *                         1,
 *                         10000),
 * 'effects.Barrage': (effects.StandardWeapon,
 *                     FX_TF_NONE,
 *                     FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                     None,
 *                     1,
 *                     10000),
 * 'effects.CargoScan': (effects.StretchEffect,
 *                       FX_TF_NONE,
 *                       FX_MERGE_SHIP | FX_MERGE_GUID,
 *                       'res:/Model/Effect3/CargoScan.red',
 *                       1,
 *                       10000),
 * 'effects.Cloak': (effects.Cloak,
 *                   FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                   FX_MERGE_SHIP | FX_MERGE_GUID,
 *                   'res:/Model/Effect3/Cloaking.red',
 *                   1,
 *                   6000),
 * 'effects.CloakNoAmim': (effects.CloakNoAmim,
 *                         FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                         FX_MERGE_SHIP | FX_MERGE_GUID,
 *                         'res:/Model/Effect3/Cloaking.red',
 *                         1,
 *                         6000),
 * 'effects.CloakRegardless': (effects.CloakRegardless,
 *                             FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                             FX_MERGE_SHIP | FX_MERGE_GUID,
 *                             'res:/Model/Effect3/Cloaking.red',
 *                             1,
 *                             6000),
 * 'effects.Cloaking': (effects.Cloaking,
 *                      FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                      FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                      None,
 *                      1,
 *                      10000),
 * 'effects.CloudMining': (effects.CloudMining,
 *                         FX_TF_NONE,
 *                         FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                         None,
 *                         1,
 *                         10000),
 * 'effects.ECMBurst': (effects.ShipEffect,
 *                      FX_TF_POSITION_BALL,
 *                      FX_MERGE_SHIP | FX_MERGE_GUID,
 *                      'res:/Model/Effect3/EcmBurst.red',
 *                      1,
 *                      10000),
 * 'effects.EMPWave': (effects.EMPWave,
 *                     FX_TF_NONE,
 *                     FX_MERGE_SHIP | FX_MERGE_MODULE,
 *                     None,
 *                     1,
 *                     10000),
 * 'effects.ElectronicAttributeModifyActivate': (effects.ShipEffect,
 *                                               FX_TF_SCALE_RADIUS | FX_TF_POSITION_BALL,
 *                                               FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                                               'res:/Model/Effect3/ECM.red',
 *                                               1,
 *                                               10000),
 * 'effects.ElectronicAttributeModifyTarget': (effects.StretchEffect,
 *                                             FX_TF_NONE,
 *                                             FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
 *                                             'res:/Model/Effect3/SensorBoost.red',
 *                                             1,
 *                                             10000),
 * 'effects.EnergyDestabilization': (effects.StretchEffect,
 *                                   FX_TF_NONE,
 *                                   FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
 *                                   'res:/Model/Effect3/EnergyDestabilization.red',
 *                                   1,
 *                                   10000),
 * 'effects.EnergyTransfer': (effects.StretchEffect,
 *                            FX_TF_NONE,
 *                            FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
 *                            'res:/Model/Effect3/EnergyTransfer.red',
 *                            1,
 *                            10000),
 * 'effects.EnergyVampire': (effects.StretchEffect,
 *                           FX_TF_NONE,
 *                           FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
 *                           'res:/Model/Effect3/EnergyVampire.red',
 *                           1,
 *                           10000),
 * 'effects.GateActivity': (effects.GateActivity,
 *                          FX_TF_NONE,
 *                          FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                          None,
 *                          1,
 *                          10000),
 * 'effects.HybridFired': (effects.StandardWeapon,
 *                         FX_TF_NONE,
 *                         FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                         None,
 *                         1,
 *                         10000),
 * 'effects.Jettison': (effects.ShipEffect,
 *                      FX_TF_POSITION_BALL,
 *                      FX_MERGE_SHIP | FX_MERGE_GUID,
 *                      'res:/Model/Effect3/Jettison.red',
 *                      1,
 *                      10000),
 * 'effects.JumpDriveIn': (effects.JumpDriveIn,
 *                         FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                         FX_MERGE_SHIP | FX_MERGE_GUID,
 *                         'res:\\Model\\Effect3\\JumpDrive_in.red',
 *                         1,
 *                         10000),
 * 'effects.JumpDriveInBO': (effects.JumpDriveInBO,
 *                           FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                           FX_MERGE_SHIP | FX_MERGE_GUID,
 *                           'res:\\Model\\Effect3\\JumpDriveBO_in.red',
 *                           1,
 *                           10000),
 * 'effects.JumpDriveOut': (effects.JumpDriveOut,
 *                          FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL,
 *                          FX_MERGE_SHIP | FX_MERGE_GUID,
 *                          'res:\\Model\\Effect3\\JumpDrive_out.red',
 *                          1,
 *                          10000),
 * 'effects.JumpDriveOutBO': (effects.JumpDriveOutBO,
 *                            FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL,
 *                            FX_MERGE_SHIP | FX_MERGE_GUID,
 *                            'res:\\Model\\Effect3\\JumpDriveBO_out.red',
 *                            1,
 *                            10000),
 * 'effects.JumpIn': (effects.JumpIn,
 *                    FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                    FX_MERGE_SHIP | FX_MERGE_GUID,
 *                    'res:/Model/Effect3/warpEntry.red',
 *                    1,
 *                    10000),
 * 'effects.JumpOut': (effects.JumpOut,
 *                     FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                     FX_MERGE_SHIP | FX_MERGE_GUID,
 *                     'res:\\Model\\Effect3\\Jump_out.red',
 *                     1,
 *                     10000),
 * 'effects.JumpOutWormhole': (effects.JumpOutWormhole,
 *                             FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                             FX_MERGE_SHIP | FX_MERGE_GUID,
 *                             'res:\\Model\\Effect3\\WormJump.red',
 *                             1,
 *                             10000),
 * 'effects.JumpPortal': (effects.JumpPortal,
 *                        FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                        FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                        'res:/Model/Effect3/JumpPortal.red',
 *                        1,
 *                        10000),
 * 'effects.JumpPortalBO': (effects.JumpPortalBO,
 *                          FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                          FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                          'res:/Model/Effect3/JumpPortal_BO.red',
 *                          1,
 *                          10000),
 * 'effects.Laser': (effects.StandardWeapon,
 *                   FX_TF_NONE,
 *                   FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                   None,
 *                   1,
 *                   10000),
 * 'effects.Mining': (effects.StandardWeapon,
 *                    FX_TF_NONE,
 *                    FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                    None,
 *                    1,
 *                    10000),
 * 'effects.MissileDeployment': (effects.ShipEffect,
 *                               FX_TF_POSITION_BALL,
 *                               FX_MERGE_SHIP | FX_MERGE_GUID,
 *                               'res:/Model/Effect3/missileLaunch.red',
 *                               1,
 *                               12000),
 * 'effects.ModifyShieldResonance': (effects.ShipRenderEffect,
 *                                   FX_TF_NONE,
 *                                   FX_MERGE_SHIP | FX_MERGE_GUID,
 *                                   'res:/dx9/Model/Effect/ShieldHardening.red',
 *                                   1,
 *                                   10000),
 * 'effects.ModifyTargetSpeed': (effects.ShipEffect,
 *                               FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_TARGET,
 *                               FX_MERGE_TARGET | FX_MERGE_GUID,
 *                               'res:/Model/Effect3/StasisWeb.red',
 *                               1,
 *                               10000),
 * 'effects.ProjectileFired': (effects.StandardWeapon,
 *                             FX_TF_NONE,
 *                             FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                             None,
 *                             1,
 *                             10000),
 * 'effects.ProjectileFiredForEntities': (effects.StandardWeapon,
 *                                        FX_TF_NONE,
 *                                        FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                                        None,
 *                                        1,
 *                                        10000),
 * 'effects.RemoteArmourRepair': (effects.StretchEffect,
 *                                FX_TF_NONE,
 *                                FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
 *                                'res:/Model/Effect3/RemoteArmorRepair.red',
 *                                1,
 *                                10000),
 * 'effects.RemoteECM': (effects.StretchEffect,
 *                       FX_TF_NONE,
 *                       FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
 *                       'res:/Model/Effect3/RemoteECM.red',
 *                       1,
 *                       10000),
 * 'effects.Salvaging': (effects.StandardWeapon,
 *                       FX_TF_NONE,
 *                       FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                       None,
 *                       1,
 *                       10000),
 * 'effects.ScanStrengthBonusActivate': (effects.ShipEffect,
 *                                       FX_TF_SCALE_RADIUS | FX_TF_POSITION_BALL,
 *                                       FX_MERGE_SHIP | FX_MERGE_GUID,
 *                                       'res:/Model/Effect3/ECCM.red',
 *                                       1,
 *                                       10000),
 * 'effects.ScanStrengthBonusTarget': (effects.ShipEffect,
 *                                     FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL,
 *                                     FX_MERGE_SHIP | FX_MERGE_GUID,
 *                                     'res:/Model/Effect3/ECCM.red',
 *                                     1,
 *                                     10000),
 * 'effects.ShieldBoosting': (effects.ShipRenderEffect,
 *                            FX_TF_NONE,
 *                            FX_MERGE_SHIP | FX_MERGE_GUID,
 *                            'res:/dx9/Model/Effect/ShieldBoosting.red',
 *                            0,
 *                            10000),
 * 'effects.ShieldTransfer': (effects.StretchEffect,
 *                            FX_TF_NONE,
 *                            FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
 *                            'res:/Model/Effect3/ShieldTransfer.red',
 *                            1,
 *                            10000),
 * 'effects.ShipScan': (effects.StretchEffect,
 *                      FX_TF_NONE,
 *                      FX_MERGE_SHIP | FX_MERGE_GUID,
 *                      'res:/Model/Effect3/ShipScan.red',
 *                      1,
 *                      10000),
 * 'effects.SiegeMode': (effects.SiegeMode,
 *                       FX_TF_NONE,
 *                       FX_MERGE_SHIP,
 *                       None,
 *                       1,
 *                       10000),
 * 'effects.SpeedBoost': (effects.GenericEffect,
 *                        FX_TF_NONE,
 *                        FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                        None,
 *                        1,
 *                        10000),
 * 'effects.StructureOffline': (effects.StructureOffline,
 *                              FX_TF_NONE,
 *                              FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                              None,
 *                              1,
 *                              10000),
 * 'effects.StructureOnline': (effects.StructureOnline,
 *                             FX_TF_NONE,
 *                             FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                             None,
 *                             1,
 *                             10000),
 * 'effects.StructureOnlined': (effects.StructureOnlined,
 *                              FX_TF_NONE,
 *                              FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                              None,
 *                              1,
 *                              10000),
 * 'effects.StructureRepair': (effects.ShipRenderEffect,
 *                             FX_TF_NONE,
 *                             FX_MERGE_SHIP | FX_MERGE_GUID,
 *                             'res:/dx9/Model/Effect/HullRepair.red',
 *                             1,
 *                             10000),
 * 'effects.SuperWeaponAmarr': (effects.StretchEffect,
 *                              FX_TF_NONE,
 *                              FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                              'res:/Model/Effect3/Superweapon/A_DoomsDay.red',
 *                              False,
 *                              10000),
 * 'effects.SuperWeaponCaldari': (effects.StretchEffect,
 *                                FX_TF_NONE,
 *                                FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                                'res:/Model/Effect3/Superweapon/C_DoomsDay.red',
 *                                False,
 *                                10000),
 * 'effects.SuperWeaponGallente': (effects.StretchEffect,
 *                                 FX_TF_NONE,
 *                                 FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                                 'res:/Model/Effect3/Superweapon/G_DoomsDay.red',
 *                                 False,
 *                                 10000),
 * 'effects.SuperWeaponMinmatar': (effects.StretchEffect,
 *                                 FX_TF_NONE,
 *                                 FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                                 'res:/Model/Effect3/Superweapon/M_DoomsDay.red',
 *                                 False,
 *                                 10000),
 * 'effects.SurveyScan': (effects.ShipEffect,
 *                        FX_TF_SCALE_SYMMETRIC | FX_TF_POSITION_BALL,
 *                        FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
 *                        'res:/Model/Effect3/SurveyScan.red',
 *                        1,
 *                        10000),
 * 'effects.TargetPaint': (effects.StretchEffect,
 *                         FX_TF_NONE,
 *                         FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
 *                         'res:/Model/Effect3/TargetPaint.red',
 *                         1,
 *                         10000),
 * 'effects.TargetScan': (effects.StretchEffect,
 *                        FX_TF_NONE,
 *                        FX_MERGE_SHIP | FX_MERGE_TARGET | FX_MERGE_GUID,
 *                        'res:/Model/Effect3/SurveyScan2.red',
 *                        1,
 *                        10000),
 * 'effects.TorpedoDeployment': (effects.GenericEffect,
 *                               FX_TF_NONE,
 *                               FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                               None,
 *                               1,
 *                               10000),
 * 'effects.TractorBeam': (effects.StandardWeapon,
 *                         FX_TF_NONE,
 *                         FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                         None,
 *                         1,
 *                         10000),
 * 'effects.TriageMode': (effects.ShipRenderEffect,
 *                        FX_TF_NONE,
 *                        FX_MERGE_SHIP | FX_MERGE_GUID,
 *                        'res:/dx9/Model/Effect/TriageMode.red',
 *                        0,
 *                        10000),
 * 'effects.TurretWeaponRangeTrackingSpeedMultiplyActivate': (effects.ShipEffect,
 *                                                            FX_TF_POSITION_BALL,
 *                                                            FX_MERGE_SHIP | FX_MERGE_GUID,
 *                                                            'res:/Model/Effect3/TrackingBoost.red',
 *                                                            1,
 *                                                            10000),
 * 'effects.TurretWeaponRangeTrackingSpeedMultiplyTarget': (effects.StretchEffect,
 *                                                          FX_TF_NONE,
 *                                                          FX_MERGE_SHIP | FX_MERGE_GUID,
 *                                                          'res:/Model/Effect3/TrackingBoostTarget.red',
 *                                                          1,
 *                                                          10000),
 * 'effects.Uncloak': (effects.Uncloak,
 *                     FX_TF_POSITION_BALL | FX_TF_ROTATION_BALL,
 *                     FX_MERGE_SHIP | FX_MERGE_GUID,
 *                     'res:/Model/Effect3/Cloaking.red',
 *                     1,
 *                     7500),
 * 'effects.WarpDisruptFieldGenerating': (effects.WarpDisruptFieldGenerating,
 *                                        FX_TF_POSITION_BALL,
 *                                        FX_MERGE_SHIP | FX_MERGE_GUID,
 *                                        'res:/Model/effect3/WarpDisruptorBubble.red',
 *                                        0,
 *                                        10000),
 * 'effects.WarpGateEffect': (effects.WarpGateEffect,
 *                            FX_TF_NONE,
 *                            FX_MERGE_SHIP | FX_MERGE_GUID,
 *                            None,
 *                            0,
 *                            10000),
 * 'effects.WarpScramble': (effects.StretchEffect,
 *                          FX_TF_NONE,
 *                          FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                          'res:/Model/Effect3/WarpScrambler.red',
 *                          1,
 *                          10000),
 * 'effects.Warping': (effects.Warping,
 *                     FX_TF_NONE,
 *                     FX_MERGE_SHIP | FX_MERGE_GUID,
 *                     'res:/Model/Effect3/warpTunnel2.red',
 *                     False,
 *                     1200000),
 * 'effects.WormholeActivity': (effects.WormholeActivity,
 *                              FX_TF_NONE,
 *                              FX_MERGE_SHIP | FX_MERGE_MODULE | FX_MERGE_GUID,
 *                              None,
 *                              1,
 *                              10000)}
 * if const.useNewMissileSystem_removeThisSoon:
 *    definitions['effects.MissileDeployment'] = (effects.MissileDeployment,
 *     0,
 *     0,
 *     None,
 *     1,
 *     12000)
 *
 * def GetGuids():
 *    return definitions.keys()
 *
 *
 * def GetClassification(guid):
 *    return definitions.get(guid, None)
 *
 *
 * exports = {'effects.GetClassification': GetClassification,
 * 'effects.GetGuids': GetGuids,
 * 'effects.FX_MERGE_NONE': FX_MERGE_NONE,
 * 'effects.FX_MERGE_GUID': FX_MERGE_GUID,
 * 'effects.FX_MERGE_SHIP': FX_MERGE_SHIP,
 * 'effects.FX_MERGE_MODULE': FX_MERGE_MODULE,
 * 'effects.FX_MERGE_TARGET': FX_MERGE_TARGET,
 * 'effects.FX_TF_NONE': FX_TF_NONE,
 * 'effects.FX_TF_POSITION_BALL': FX_TF_POSITION_BALL,
 * 'effects.FX_TF_POSITION_TARGET': FX_TF_POSITION_TARGET,
 * 'effects.FX_TF_ROTATION_BALL': FX_TF_ROTATION_BALL,
 * 'effects.FX_TF_SCALE_SYMMETRIC': FX_TF_SCALE_SYMMETRIC,
 * 'effects.FX_TF_SCALE_BOUNDING': FX_TF_SCALE_BOUNDING,
 * 'effects.FX_TF_SCALE_RADIUS': FX_TF_SCALE_RADIUS}
 */