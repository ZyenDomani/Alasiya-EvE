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

static const float VISUAL_STOP_THRESHOLD_SQ = 0.0025f;
static const float HARD_STOP_THRESHOLD_SQ = 0.0001f;

static const double SPACE_DRAG = 0.30000001192092896;


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
            Vector3d warp_vector;        //target direction based on ship's initial position
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
    const Vector3d& GetPosition() const                 { return m_position; }
    const Vector3d& GetVelocity() const                 { return m_shipVelocity; }
    float GetSpeedFraction()                            { return m_activeSpeedFraction; }
    double GetSpeed()                                   { return m_shipVelocity.Length(); }
    int32 GetWarpSpeed()                                { return static_cast<int32>(m_shipWarpSpeed * 10); }
    uint32 GetTargetID()                                { return m_targetEntity.first; }
    SystemEntity* GetTargetEntity()                     { return m_targetEntity.second; }
    Vector3d GetTargetPoint()                           { return m_targetPoint; }
    // this is distance from where we are to where we want to be
    double GetTargetDistance()                          { return static_cast<double>(m_targetDistance); }
    // this is commanded follow or 'warp to within' distance
    double GetFollowDistance()                          { return static_cast<double>(m_followDistance); }
    int32 GetTicStamp()                                 { return m_ticStamp; }
    double GetAgility()                                 { return m_agility; }   // this is only used by my GetShipVars command
    uint8 GetAlignTime()                                { return m_alignTime; } // this is only used by my GetShipVars command
    float GetWarpDropSpeed()                            { return m_speedToLeaveWarp; }
    double GetCapNeed()                                 { return m_warpCapacitorNeed; }
    uint8 GetBallMode()                                 { return m_ballMode; }
    float GetMaxVelocity()                              { return m_maxSpeed; }

    bool IsMoving()                                     { return (m_activeSpeedFraction > 0.01); }
    bool IsGoto()                                       { return (m_ballMode == Destiny::Ball::Mode::GOTO); }
    bool IsStopped()                                    { return (m_ballMode == Destiny::Ball::Mode::STOP); }
    bool IsOrbiting()                                   { return (m_ballMode == Destiny::Ball::Mode::ORBIT); }
    bool IsFollowing()                                  { return (m_ballMode == Destiny::Ball::Mode::FOLLOW); }
    bool IsWarping()                                    { return (m_warpState != nullptr); }
    bool IsCloaked()                                    { return m_cloaked; }
    bool IsTractored()                                  { return m_tractored; }
    bool IsAutoPilot()                                  { return m_autoPilot; }
    bool IsAligned(Vector3d& targHeading);

    /* Configuration methods */
    void WebbedMe();
    // reset speed variables and bubblecast ship's AB/MWD modified speed (module activate/deactivate)
    void SpeedBoost(bool deactivate=false);
    void SetPosition(const Vector3d& pt, bool update=false);
    void SetNPCSpeedMass(uint16 newSpeed, uint16 maxSpeed, double mass);
    void UpdateShipVariables();
    // set all movement vars for missile and add to system
    void MakeMissile(Missile* missile);  //  this is used by all entities (pc, npc, drone, sentry, pos, etc)

    /* Global Actions */
    void Stop();
    void Halt(bool commanded=false);			// puts entity at 0 velocity
    void Eject();								// avoid numerous other redirect calls
    void EntityRemoved(SystemEntity* pSE);
    void SetCloak(bool set=false)                       { m_cloaked = set; }
    void SkipTic(bool set=true)                         { m_skipTic = set; }
    void SetAutoPilot(bool set=false)                   { m_autoPilot = set; }

    /* Local Movement */
    void OrbitBall(SystemEntity* pSE, uint32 distance=0);
    void FollowBall(SystemEntity* pSE, int32 distance=0);
    void ApproachBall(SystemEntity* pSE);
    void AlignTo(SystemEntity* pSE);
    void GotoPoint(const Vector3d &point);
    void GotoDirection(const Vector3d &direction);
    void SetSpeedFraction(float fraction=1.0f);

    /* TractorBeam */
    void TractorBeamStop();
    void TractorBeamStart(SystemEntity* pSE, EvilNumber speed);

    /* Larger movement */
    void WarpTo( const Vector3d& destPoint, int32 distance = 0, bool autoPilot = false, SystemEntity* pSE = nullptr );

    /* special */
    void ApplyVortexGravityTug(ShipSE* pShipSE, Vector3d vortexCenterAxis);
    void SetTrollData(DestinyManager* pDestiny);  // as stated.  uses DestinyMgr of launcher to acquire data

    //Destiny Update stuff:
    PyResult AttemptDockOperation();
    void Undock(Vector3d dir);
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
    // this is for structure effects
	// refactor this to avoid creating copies of args
    void SendGFX14(int32 entityID, int32 moduleID, int32 moduleTypeID, int32 targetID,
                   int32 chargeTypeID, std::string guid, bool isOffensive, bool start,
                   bool isActive, int32 duration, int32 repeat, int64 startTime=0, int32 graphicInfo=0,
                   Client* pClient=nullptr) const;

    /* update and event queue methods */
    void SendSingleDestinyEvent(PyTuple** ev, bool self_only=false) const;// this will not consume *ev
    void SendSingleDestinyUpdate(PyTuple** up, bool self_only=false) const; // this will not consume *up
    void SendDestinyUpdates(std::vector<PyTuple*> &updates, bool self_only=false) const;// this will consume all updates in vector

    /* misc */
    const char* GetModeName(uint8 mode);
    std::string GetModeNameString();

protected:
    bool ValidTarget();                 //performs common target checks

    // movement methods
    void BeginMovement();               //set initial variables for all movement (common code)

    uint8 m_ballMode;                   //current state of ball
    bool m_hasSentShipUpdates;

	// things dictated by ship and skills
    double m_warpCapacitorNeed;         //in GJ     - capacitor charged needed to initiate warp

    //things dictated by our entity's configuration:
    float m_maxSpeed;                   //in m/s
    float m_alignTime;                  //in s      - no longer used
    float m_shipWarpSpeed;              //in au/s   x/3 = warp speed multiplier
    float m_speedToLeaveWarp;           //in m/s    - this is set to 50% of m_maxSpeed

    //User controlled information used by a state to determine what to do.
    bool m_stop;                        //used to denote Stop() has been called to avoid multiple stops (and associated decel)
    bool m_cloaked;
    bool m_tractored;
    bool m_tractorPause;

    double m_timeStamp;                 //timestamp of when current mode began, using GetTimeMSeconds()
    int32 m_ticStamp;                   //sysMgr ticStamp of when current state began, in seconds

    float m_userSpeedFraction;          //fuzzy logic - user commanded percent of max speed
    float m_activeSpeedFraction;        //fuzzy logic - current percent of max speed

    int64 m_targetDistance;             //in m  this is current distance to target
    int64 m_followDistance;             //in m  this is desired distance to target; also 'warp to within' distance

    Vector3d m_position;                //in m
    Vector3d m_targetPoint;             //vector  point in space used as current destination
    Vector3d m_shipVelocity;            //current ship velocity
    Vector3d m_targetHeading;           //direction to current target
    Vector3d m_targetVelocity;          //desired ship velocity
    std::pair<uint32, SystemEntity*> m_targetEntity;   //we do not own the SystemEntity*

private:
    SystemEntity* const mySE;           //we do not own this.
    SystemBubble* m_targBubble;         //we do not own this.

    bool m_moveDelay;                   // his is to try to sync destiny with client, as client has a delay when changing destiny states.
    bool m_skipTic;                     // this is for objects that dont need to proc movement at this time
    bool m_autoPilot;                   // as stated
    bool m_alignTo;                     // once aligned, ship will stop
    bool m_paused;                      // used to fake orbit while keeping velocity but not actually move ship.
    bool m_posHack;                     //force position update
    double m_agility;                   // something about m/s^2  not quite sure yet

    // new movement data
    double m_expTerm;                   // e^(-dt/tau)   [tau = mass * inertiaMod / 1000000.0f]
    double m_posScale;                  // tau * (1.0 - m_expTerm)
    double m_timeFactor;
    void Integrate();
    void CalculateFollowPoint();
    void CalculateFormationPoint();
    void GotoVelocity();
    void OrbitVelocity();
    Vector3d RotateVectorByEntityOrientation(const Vector3d& localVec, SystemEntity* leader);

    // these will eventually be commands....
    void CreateShipMarker();
    void RemoveAllMarkers();
    void RemoveShipMarkers();
    void MarkPoint(const Vector3d& position, std::string& name, std::string& desc, bool orbit=false);
    std::map<uint32, SystemEntity*> m_shipMarkers;           // ship position marker cans.  we do own these.

    void SetAgilityInertia();

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
        bool accel;
        bool cruise;
        bool decel;
        int32 start_time;          //from sEntityMgr::GetStamp()
        int64 total_distance;      //in m
        int64 warpSpeed;           //in m/s
        int64 accelDist;           //in m
        int64 cruiseDist;          //in m
        int64 decelDist;           //in m
        float warpTime;            //in s
        float accelFraction;
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
