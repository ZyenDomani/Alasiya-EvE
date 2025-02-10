/**
 * DroneAI.h
 *      this class is for drone AI
 *
 * @Author:     Allan
 * @Version:    0.15
 * @Date:       27Nov19
 */

#ifndef __EVEMU_SHIP_DRONEAI_H__
#define __EVEMU_SHIP_DRONEAI_H__

#include "ship/modules/TurretFormulas.h"

// only for drones
namespace DroneAI {
    namespace State {
        enum {
            Incapacitated       = -2,
            Invalid             = -1,
            // defined in client
            Idle                = 0,  // not doing anything....idle.
            Combat              = 1,  // fighting - needs target
            Mining              = 2,  // unsure - may need target
            Approaching         = 3,  // too close to chase, but to far to engage
            ReturnBay           = 4,  // return to bay  (Departing in client)
            ReturnHome          = 5,  // return to ship  (Departing2 in client)
            Pursuit             = 6,  // target out of range to attack/follow, but within npc sight range....use mwd/ab if equiped
            Fleeing             = 7,  // running away
            Operating           = 9,  // whats diff from engaged here? unanchoring?
            Engaged             = 10, // either attack or aid - needs target
            // internal only
            Guarding            = 11, // as stated
            Assisting           = 12  //  this will be remote reppers/boosters
        };
    }

    namespace Action {
        enum {
            Invalid             = -1,
            Idle                = 0,  // not doing anything....idle.
            Engaged             = 1,  // currently performing it's intended action
            AccelToTarget       = 2,  // Accel while traveling to target
            DecelToTarget       = 3,  // Decel while traveling from target
            AccelToShip         = 4,  // Accel while traveling to assigned ship
            DecelToShip         = 5,  // Decel while traveling from assigned ship
            OrbitTarget         = 6,  // Idle and orbiting assigned engagement target
            OrbitShip           = 7,  // Idle and orbiting assigned ship
            DecelToStop         = 8   // set to idle.  decel to stop
        };
    }
}

class DroneSE;
class SystemEntity;
class Timer;

class DroneAIMgr {
public:
    DroneAIMgr(DroneSE* pdSE);

    /* initialize drone ai variables
     *   (move, distance, ranges and default fxID)
     *   owner/controller not yet set nor needed at this time
     */
    void                Init();

    /*  Process will used checks to determine what do do and when.  this is my advanced Drone AI
     * there are multiple steps for drones, but can only send orbit and speedFraction to client
     * drone state will hold the current command until commanded otherwise
     * within a command, the drone is either orbiting assigned ship, orbiting target, or traveling between the two
     * drone action will hold current point in command for that tic
     * based on current point and distance, Process will determine next step and set action accordingly
     * movement is also based on current point, but is only for our tracking server-side
     * orbit is disabled, so we are setting drone position somewhere on the orbit radius and leaving there
     * client will show orbit and traveling
     *
     * i am not using the overhead of a DestinyManager here, but have copied some of the code for processing movement
     *
     */
    void                Process();

    SystemEntity*       GetTargetSE();
    SystemEntity*       GetAssignedShipSE()             { return m_assignedShipSE; }

    uint32              GetTargetID();
    uint32              GetControllerID()               { return m_assignedShipSE->GetID(); }
    uint32              GetFollowDistance();

    // main entry into drone ai
    // called to perform action on target
    void                Engage(PyDict* dict, int8 state, bool repeat=0); // sets state and error dict, if applicable

    void                SetIdle();                      // sets state and action  (idle)

    void                Target(SystemEntity* pTarget);
    void                Targeted(SystemEntity *by_who);
    void                TargetLost(SystemEntity *by_who);

    void                ClearTargets();                 // clear our targets and set repeat=false
    void                ClearAllTargets();              // clear all targets and set repeat=false

    int8                GetState();                     // for EncodeDestiny()

    void                Abandon();                      // disable timers and null targets
    void                AssignShip(ShipSE* pSE);

    float               GetSpeedFraction();

    uint32              GetMaxSpeed()                   { return m_maxSpeed; }

    float               GetVelocityX()                  { return m_velocity.x; }
    float               GetVelocityY()                  { return m_velocity.y; }
    float               GetVelocityZ()                  { return m_velocity.z; }

    // advanced ai methods and calls
    void                MissileLaunched(Missile* pMissile);
    void                ReportDamage(uint8 type=0);

    // this is public to allow deletion from DroneSE object
    InventoryItemRef    m_ore;                          //ore from mining

protected:
    void                MineTarget();                   // actual mining code
    void                ClearTarget();                  // actual clear code (lol)
    void                AttackTarget();                 // actual attack code
    void                EngageTarget();                 // called when close enough to engage - sets action
    void                OrbitTarget();                  // called when setting initial orbit - sets action, usf and heading

    bool                TargetValid();

    // checks if target is within <config.interactdist> to interact with target
    bool                InActionDistance(SystemEntity* pTarget);        // ~2k5m
    // checks if target is within m_orbitRange to engage with target
    bool                InOrbitDistance(SystemEntity* pTarget);         // near range 1
    // checks if target is within m_falloffRange to move closer to target
    bool                InFalloffDistance(SystemEntity* pTarget);        // close range 2
    // checks if target is within m_attackRange to engage with target
    bool                InEngageDistance(SystemEntity* pTarget);        // mid range 3
    // checks if target is within m_chaseRange to engage with target
    bool                InChaseDistance(SystemEntity* pTarget);         // far range 4
    // checks if target is within m_maxRange to engage with target
    bool                InMaxDistance(SystemEntity* pTarget);           // distant range 5

    void                SendGFX(Client* pClient=nullptr); // sent on every cycle -  active = (action==engaged)
    void                SendShipEffect(bool start=false); // sent on begin and end of fx

    const char*         GetStateName(int8 stateID);
    const char*         GetActionName(int8 stateID);

    /* this will be cheating, as i dont want to rewrite working code
     *   here, we are faking the drone state to send to client
     * current state/action is working very well, but client drone state shows only one thing...
     * i.e.  when mining, drone state shows "mining".   this is to update that state to be more accurate
     * so it will now show "returning" when returning to ship, "Approaching" when heading to asteroid and
     * "mining" when actively mining.
     * this will also show "Idle" when drone is actually orbit home ship
     */
    void                SendTrueState(int8 state=DroneAI::State::Idle);

    /* internal destiny methods.  testing drone w/o actual DestinyManager (dont need the overhead) */
    GPoint              m_heading;                      // well, drone heading, ofc
    GVector             m_velocity;                     // current speed and heading

    void                Stop();                         // called when offline - calls SetIdle()

    void                Move(double timeStamp=0);       // called by proc tic to keep ship position accurate
    void                UpdatePosition(bool update=false); // this is for tracking position changes

    void                SendSpeedFraction();            // orbit is only sent when target changes;  SF is sent when speed changes

    void                SetState(int8 state=-1);
    void                SetAction(int8 action=-1);
    void                MarkPoint(const GPoint& position);

private:
    SystemEntity*       m_targetSE;
    DroneSE*            m_droneSE;
    ShipSE*             m_assignedShipSE;

    TurretFormulas      m_formula;

    EVEItemFlags        m_holdFlag;

    Timer               m_processTimer;
    Timer               m_mainAttackTimer;
    Timer               m_beginFindTarget;
    Timer               m_warpScramblerTimer;
    Timer               m_webifierTimer;

    bool                m_sendCmd;                      // send updated orbit packet?
    bool                m_booster;                      // repair drone
    bool                m_repeat;                       // for mining drones.

    int8                m_state;
    int8                m_action;

    uint16              m_effectID;                     // default effectID

    uint32              m_maxSpeed;                     // mwd speed
    uint32              m_cruiseSpeed;                  // normal speed
    uint32              m_armorRepairDuration;          //
    uint32              m_shieldBoosterDuration;        //

    //in order of distance  far to close
    uint32              m_maxRange;                     // maximum engagement distance
    uint32              m_chaseRange;                   // min distance to activate mwd, if equipped
    uint32              m_attackRange;                  // max distance drone will use weapons - weaponized drones only
    uint32              m_falloffRange;                 // distance where accuracy has fallen by half  - weaponized only
    uint32              m_orbitRange;                   // distance the drone orbits  - mining and unanchoring only

    int64               m_startTime;                    // timestamp when effect started

    float               m_alignTime;
    float               m_accelTime;
    float               m_cycleTime;                    // time in ms for drone to complete an action

    float               m_timeFraction;                 //fuzzy logic - holds current euler value for time
    float               m_prevSpeedFraction;            //fuzzy logic - previous percent of full speed.  used for speed changes
    float               m_userSpeedFraction;            //fuzzy logic - user commanded percent of max speed
    float               m_activeSpeedFraction;          //fuzzy logic - current percent of max speed

    double              m_agility;
    double              m_moveTime;                     //in ms       - time when speed change started.  used to calculate m_timeFraction
};

#endif  // __EVEMU_SHIP_DRONEAI_H__

/*
 * vespa
 * 54      maxRange                NULL    4800
 * 158     falloff                 NULL    3000
 * 247     entityAttackRange       6400    NULL
 * 416     entityFlyRange          NULL    1600
 * 665     entityChaseMaxDistance  6400    NULL
 *
 * mining drone 1
 * 54      maxRange                5000    NULL
 * 154     proximityRange          250     NULL
 * 157     orbitRange              200     NULL
 *
 */

/*  distance and speed attribs
 *     AttrMaxVelocity = 37,                       //Maximum velocity of ship
 *     AttrSpeed = 51,                             //durationAttributeID in dgmEffects  Time in milliseconds between possible activations
 *     AttrMaxRange = 54,                          //rangeAttributeID in dgmEffects  max distance range does not affect the to-hit equation.
 *     AttrDuration = 73,                          //durationAttributeID in dgmEffects
 *     AttrMaxTargetRange = 76,                    // npc dont have this
 *     AttrMiningAmount = 77,
 *     AttrScanSpeed = 79,                         //ship scanning speed and drone locktime in milliseconds
 *     AttrShieldTransferRange = 87,               //rangeAttributeID in dgmEffects
 *     AttrPowerTransferRange = 91,                //rangeAttributeID in dgmEffects
 *     AttrEnergyDestabilizationRange = 98,        //rangeAttributeID in dgmEffects
 *     AttrEmpFieldRange = 99,                     //rangeAttributeID in dgmEffects       -smartbombs
 *     AttrWarpScrambleRange = 103,                //rangeAttributeID in dgmEffects
 *     AttrEcmBurstRange = 142,                    //rangeAttributeID in dgmEffects      -ecm burst
 *     AttrProximityRange = 154,                   //The distance at which to react when relevant objects come within range.
 *     AttrIncapacitationRatio = 156,              //The hull damage proportion at which an entity becomes incapacitated.
 *     AttrOrbitRange = 157,                       // as stated     (is this NPC or Drone?  drone for sure)
 *     AttrFalloff = 158,                          //distance from maximum range at which accuracy has fallen by half
 *     AttrTrackingSpeed = 160,                    //trackingSpeedAttributeID in dgmEffects
 *     AttrMaxLockedTargets = 192,
 *     AttrMaxAttackTargets = 193,
 *     AttrEntityAttackRange = 247,                //The distance from a target an entity starts using its weapons.
 *     AttrEntityFlyRange = 416,                   //The distance at which the entity orbits, follows.. and more.
 *     AttrDroneControlDistance = 458,             // this is a character attrib...NOT ship
 *     AttrEntityCruiseSpeed = 508,                //The speed that entities fly at when not chasing a target.
 *     AttrEntityChaseMaxDelay = 580,              //The maximum amount of time stalled before entity chase speed kicks in.
 *     AttrEntityChaseMaxDelayChance = 581,        //Chance that the max delay is used before chase is engaged.
 *     AttrEntityChaseMaxDuration = 582,           //The maximum amount of time chase is engaged
 *     AttrEntityChaseMaxDurationChance = 583,     //The chance of engaging chase for the maximum duration.
 *     AttrEntityChaseMaxDistance = 665,           // min distance where entity will activate their speed mod
 */
