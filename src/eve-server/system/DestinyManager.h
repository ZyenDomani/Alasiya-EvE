/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
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
            uint32 startStamp;          //from sEntityMgr::GetStamp()
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
    DestinyManager(const DestinyManager&) =delete;
    DestinyManager& operator=(const DestinyManager&) =delete;

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
    // is this right??
    float GetFollowDistance()                           { return (float)m_targetDistance; }
    int32 GetStateStamp()                               { return m_stateStamp; }
    GVector GetHeading()                                { return m_shipHeading; }
    float GetAccelTime()                                { return m_shipMaxAccelTime; }
    uint8 GetAlignTime()                                { return m_alignTime; } // this is only used by my GetShipVars command
    float GetWarpDropSpeed()                            { return m_speedToLeaveWarp; }
    double GetRadius()                                  { return m_radius; }
    double GetCapNeed()                                 { return m_warpCapacitorNeed; }
    float GetRadTic()                                   { return m_orbitRadTic; }
    uint8 GetState()                                    { return m_ballMode; }// this is only used by my bubble debug command
    std::string GetStateName();
    bool IsFrozen()                                     { return m_frozen; }
    bool IsMoving()                                     { return (m_activeSpeedFraction > ASF_CHECK); }
    bool IsGoto()                                       { return (m_ballMode == Destiny::Ball::Mode::GOTO); }
    bool IsStopped()                                    { return (m_ballMode == Destiny::Ball::Mode::STOP); }
    bool IsOrbiting()                                   { return (m_ballMode == Destiny::Ball::Mode::ORBIT); }
    bool IsFollowing()                                  { return (m_ballMode == Destiny::Ball::Mode::FOLLOW); }
    bool IsWarping()                                    { return (m_warpState != nullptr); }
    bool IsCloaked()                                    { return m_cloaked; }
    bool IsTurning()                                    { return m_turning; }
    bool IsTractored()                                  { return m_tractored; }
    bool IsAutoPilot()                                  { return m_autoPilot; }
    void SetAutoPilot(bool set=false)                   { m_autoPilot = set; }
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
    void Follow( SystemEntity* pSE, int32 distance );
    void AlignTo(SystemEntity* pSE);
    void GotoPoint(const GPoint &point);
    void GotoDirection(const GPoint &direction);
    void SetSpeedFraction(float fraction=1.0f, bool startMovement=false);

    /* TractorBeam */
    void TractorBeamStop();
    void TractorBeamStart(SystemEntity* pShipSE, EvilNumber speed);

    /* Larger movement */
    void WarpTo( const GPoint& destPoint, int32 distance = 0, bool autoPilot = false, SystemEntity* pSE = nullptr );

    //Destiny Update stuff:
    PyResult AttemptDockOperation();
    void Undock(GPoint dir);
    void SetUndockSpeed();
    void DockingAccepted();
    void SendSetState() const;
    void SendTerminalExplosion(uint32 shipID, uint32 bubbleID, bool isGlobal=false) const;
    void SendBallInteractive(const ShipItemRef shipRef, bool set = false) const;
    void UpdateNewShip(const ShipItemRef newShipRef);
    void UpdateOldShip(ShipSE* pShipSE);

    /* GFX sending methods */
    void Jump(int32 fromGateID, bool showCloak=true);
    void Cloak();
    void UnCloak();
    void SendJumpOut(int32 gateID) const;
    void SendJumpOutWormhole(uint32 wormholeID) const;
    void SendGateActivity( int32 gateID ) const;
    void SendWormholeActivity(uint32 wormholeID) const;
    void SendJumpInEffect(std::string JumpEffect) const;
    void SendJumpOutEffect(std::string JumpEffect, uint32 shipID) const;
    void SendJettisonPacket() const;
    // this is for space-type effects - warping, GateActivity, jumping, jettison, cloak
    // jumpout sends stargateID as target - wormhole sends ?? as otherTypeID
    void SendGFX10(uint32 entityID, std::string guid, int32 targetID=0, int32 otherTypeID=0) const;
    // this is for module and structure effects
    void SendGFX14(uint32 entityID, uint32 moduleID, uint32 moduleTypeID, uint32 targetID,
                             uint32 chargeTypeID, std::string guid, bool isOffensive, bool start,
                             bool isActive, int32 duration, uint32 repeat, int32 graphicInfo = 0) const;

    /* update and event queue methods */
    void SendSingleDestinyEvent(PyTuple** ev, bool self_only=false) const;// this will not consume *ev
    void SendSingleDestinyUpdate(PyTuple** up, bool self_only=false) const; // this will not consume *up
    void SendDestinyUpdates(std::vector<PyTuple*> &updates, bool self_only=false) const;// this will consume all updates in vector

protected:
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

	// things dictated by ship and skills
    double m_radius;                    //in m
    double m_warpCapacitorNeed;         //in GJ     - capacitor charged needed to initiate warp

    //things dictated by our entity's configuration:
    uint8 m_alignTime;                  //in s      - time to change directions or enter warp
    float m_prevSpeed;                  //in m/s    - used to calculate speed during decel
    float m_maxShipSpeed;               //in m/s
    float m_shipWarpSpeed;              //in au/s   x/3 = warp speed multiplier
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
    int32 m_stateStamp;                 //statestamp of when current state began, in seconds
    //Destiny::Ball::stateStamp m_stateStamp; //state and count of current state since beginning, in seconds
    //Destiny::Ball::timeStamp m_timeStamp; //mode and timestamp of when current mode began

    double m_degPerTic;                 //ship turn variable
    double m_orbitTime;                 //in s - time to complete one orbit using current variables
    double m_orbitRadTic;               //in rad/sec  - radians around orbit per tic
    double m_radians;                   //in rad    - radians of an ongoing turn

    float m_timeFraction;               //fuzzy logic - holds current euler value for time
    float m_turnMinFraction;            //fuzzy logic - used for turn accel/decel checks
    float m_origSpeedFraction;          //fuzzy logic - percent of full speed commanded before turn.  used for directional changes
    float m_prevSpeedFraction;          //fuzzy logic - previous percent of full speed.  used for speed changes
    float m_userSpeedFraction;          //fuzzy logic - user commanded percent of max speed
    float m_activeSpeedFraction;        //fuzzy logic - current percent of max speed
    float m_maxOrbitSpeedFraction;      //fuzzy logic - ship's max speed based on orbit data

    uint16 m_turnTime;                  //in s        - time turn started.  now uses EntityMgr.Stamp()
    uint32 m_followDistance;            //in m        - used for follow, orbit, and distance where ship should be removed from bubble during warp if < bubble radius
    int64  m_targetDistance;            //in m
    double m_moveTime;                  //in ms       - time when speed change started.  used to calculate m_timeFraction

    GPoint m_position;                  //in m
    GVector m_velocity;                 //in m/s
    GPoint m_targetPoint;               //vector      - point in space used as current destination
    GVector m_shipHeading;              //direction ship is facing
    GVector m_targetHeading;            //direction to target from current heading
    std::pair<uint32, SystemEntity*> m_targetEntity;   //we do not own the SystemEntity*

private:
    bool m_autoPilot;                   // as stated
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
    void Turn(float& speed, std::string& move);           //apply heading update and asf checks for turning.  called by MoveObject()
    void ClearTurn();
    void MarkPoint(const GPoint& position, std::string& name, std::string& desc);
    bool m_posHack;                    //force position update after turn

    // bezier turn data (wip)      -allan  Feb 2023
    bool m_wasDecel;
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
    float m_accelTime;
    float m_decelTime;
    int64 m_accelDistance;
    void InitWarp();
    void WarpAccel(uint16 sec_into_warp);
    void WarpCruise(uint16 sec_into_warp);
    void WarpDecel(uint16 sec_into_warp);
    void WarpStop(int64 currentShipSpeed);
    void WarpUpdate(int64 currentShipSpeed, uint16 sec_into_warp, uint8 type);      // 0=error, 1=accel, 2=cruise, 3=decel

    // trying update queue
    std::vector<PyTuple*> m_updateQueue;

    // Variables used during Warp.
    class WarpState {
    public:
        WarpState(
            int32 start_time_,
            int64 total_distance_,
            int64 warp_speed_,
            int64 accel_dist_,
            int64 cruise_dist_,
            int64 decel_dist_,
            float warp_time_,
            float accel_fraction_,
            bool accel_,
            bool cruise_,
            bool decel_)
        : start_time(start_time_),
        total_distance(total_distance_),
        warpSpeed(warp_speed_),
        accelDist(accel_dist_),
        cruiseDist(cruise_dist_),
        decelDist(decel_dist_),
        warpTime(warp_time_),
        accelFraction(accel_fraction_),
        accel(accel_),
        cruise(cruise_),
        decel(decel_)
        {}
        int32 start_time;          //from sEntityMgr::GetStamp()
        int64  total_distance;      //in m
        int64 warpSpeed;           //in m/s
        int64 accelDist;           //in m
        int64 cruiseDist;          //in m
        int64 decelDist;           //in m
        float warpTime;             //in s
        float accelFraction;
        bool accel;
        bool cruise;
        bool decel;
    };
    WarpState* m_warpState;		    //we own this.
};

#endif  // __SERVER_SYSTEM_DESTINY_H

/*
 *    def OnSpecialFX(self, shipID, moduleID, moduleTypeID, targetID, otherTypeID, area, guid, isOffensive, start, active, duration = -1, repeat = None, startTime = None, graphicInfo = None):
 *        if isinstance(moduleID, collections.Iterable):
 *            for m in moduleID:
 *                sm.ScatterEvent('OnSpecialFX', shipID, m, moduleTypeID, targetID, otherTypeID, area, guid, isOffensive, start, active, duration, repeat, startTime, graphicInfo)
 *
 *        else:
 *            sm.ScatterEvent('OnSpecialFX', shipID, moduleID, moduleTypeID, targetID, otherTypeID, area, guid, isOffensive, start, active, duration, repeat, startTime, graphicInfo)
 */

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

/*  possibly all guids.   need to verify these are in effects guid list
 * 'effects.AnchorDrop'
 * 'effects.AnchorLift'
 * 'effects.ArmorHardening'
 * 'effects.ArmorRepair'
 * 'effects.Barrage'
 * 'effects.CargoScan'
 * 'effects.Cloak'
 * 'effects.CloakNoAmim'
 * 'effects.CloakRegardless'
 * 'effects.Cloaking'
 * 'effects.CloudMining'
 * 'effects.ECMBurst'
 * 'effects.EMPWave'
 * 'effects.ElectronicAttributeModifyActivate'
 * 'effects.ElectronicAttributeModifyTarget'
 * 'effects.EnergyDestabilization'
 * 'effects.EnergyTransfer'
 * 'effects.EnergyVampire'
 * 'effects.GateActivity'
 * 'effects.HybridFired'
 * 'effects.Jettison'
 * 'effects.JumpDriveIn'
 * 'effects.JumpDriveInBO'
 * 'effects.JumpDriveOut'
 * 'effects.JumpDriveOutBO'
 * 'effects.JumpIn'
 * 'effects.JumpOut'
 * 'effects.JumpOutWormhole'
 * 'effects.JumpPortal'
 * 'effects.JumpPortalBO'
 * 'effects.Laser'
 * 'effects.Mining'
 * 'effects.MissileDeployment'
 * 'effects.ModifyShieldResonance'
 * 'effects.ModifyTargetSpeed'
 * 'effects.ProjectileFired'
 * 'effects.ProjectileFiredForEntities'
 * 'effects.RemoteArmourRepair'
 * 'effects.RemoteECM'
 * 'effects.Salvaging'
 * 'effects.ScanStrengthBonusActivate'
 * 'effects.ScanStrengthBonusTarget'
 * 'effects.ShieldBoosting'
 * 'effects.ShieldTransfer'
 * 'effects.ShipScan'
 * 'effects.SiegeMode'
 * 'effects.SpeedBoost'
 * 'effects.StructureOffline'
 * 'effects.StructureOnline'
 * 'effects.StructureOnlined'
 * 'effects.StructureRepair'
 * 'effects.SuperWeaponAmarr'
 * 'effects.SuperWeaponCaldari'
 * 'effects.SuperWeaponGallente'
 * 'effects.SuperWeaponMinmatar'
 * 'effects.SurveyScan'
 * 'effects.TargetPaint'
 * 'effects.TargetScan'
 * 'effects.TorpedoDeployment'
 * 'effects.TractorBeam'
 * 'effects.TriageMode'
 * 'effects.TurretWeaponRangeTrackingSpeedMultiplyActivate'
 * 'effects.TurretWeaponRangeTrackingSpeedMultiplyTarget'
 * 'effects.Uncloak'
 * 'effects.WarpDisruptFieldGenerating'
 * 'effects.WarpGateEffect'
 * 'effects.WarpScramble'
 * 'effects.Warping'
 * 'effects.WormholeActivity':
 *
 *   this is generic effect
 * 'effects.StandardWeapon'
 *
FX_TURRET_EFFECT_GUIDS = ['effects.Laser',
 'effects.ProjectileFiredForEntities',
 'effects.ProjectileFired',
 'effects.HybridFired',
 'effects.TractorBeam',
 'effects.Salvaging']
FX_PROTECTED_EFFECT_GUIDS = ['effects.GateActivity',
 'effects.WormholeActivity',
 'effects.JumpDriveIn',
 'effects.JumpDriveOut',
 'effects.JumpDriveInBO',
 'effects.JumpDriveOutBO',
 'effects.JumpIn',
 'effects.JumpOut',
 'effects.JumpOutWormhole',
 'effects.Warping',
 'effects.Cloaking',
 'effects.Uncloak',
 'effects.Cloak',
 'effects.CloakNoAmim',
 'effects.CloakRegardless',
 'effects.StructureOffline',
 'effects.StructureOnlined',
 'effects.AnchorDrop',
 'effects.AnchorLift',
 'effects.SiegeMode',
 'effects.TriageMode',
 'effects.WarpDisruptFieldGenerating',
 'effects.WarpScramble']
FX_LONG_ONESHOT_GUIDS = ['effects.SuperWeaponAmarr',
 'effects.SuperWeaponCaldari',
 'effects.SuperWeaponGallente',
 'effects.SuperWeaponMinmatar']
 */
