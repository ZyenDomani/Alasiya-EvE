/**
 * DroneAI.h
 *      this class is for drone AI
 *
 * @Author:     Allan
 * @Version:    1.03
 * @Date:       27Nov19  (copied from NPCAI.cpp)
 * @Rewrite:    3Feb25  (complete refactor to process all types and actions of drones)
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
            Idle                = 0,  // UI/Inflight/Drone/Idle - not doing anything....idle.
            Combat              = 1,  // UI/Inflight/Drone/Fighting - needs target
            Mining              = 2,  // UI/Inflight/Drone/Mining - needs target
            Approaching         = 3,  // UI/Inflight/Drone/Approaching  - too close to chase, but to far to engage
            ReturnBay           = 4,  // return to bay  (Departing in client - UI/Inflight/Drone/ReturningToShip)
            ReturnHome          = 5,  // return to ship  (Departing2 in client - UI/Inflight/Drone/ReturningToShip)
            Pursuit             = 6,  // UI/Inflight/Drone/Following - target out of engage range...use mwd/ab if equipped
            Fleeing             = 7,  // UI/Inflight/Drone/Fleeing
            Unknown             = 8,  // not in client data.
            Operating           = 9,  // UI/Inflight/Drone/Operating - whats diff from engaged here? unanchoring?
            Repairing           = 10, // UI/Inflight/Drone/Repairing - repairing target (shield or armor) - listed as Engage in client
            // internal only
            Guarding            = 11, // as stated
            Assisting           = 12, //  this will be remote reppers/boosters
            FocusFire           = 13  // not sure if we'll need this one
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
    SystemEntity*       GetAssignedShipSE()             { return shipSE; }

    uint32              GetTargetID();
    uint32              GetControllerID()               { return shipSE->GetID(); }
    uint32              GetFollowDistance();

    // main entry into drone ai
    // called to perform action on target
    void                Engage(PyDict* dict, int8 state, bool repeat=0); // sets state and error dict, if applicable

    bool                IsIdle();
    void                SetIdle();                      // clears targets and timers then sets state and action to idle

    void                Target(SystemEntity* pTargetSE);
    void                Targeted(SystemEntity* pTargetSE);
    void                TargetLost(SystemEntity* pTargetSE);

    // to assign drones to same target.  called by drone
    void                FocusFire(DroneSE* pFromSE, SystemEntity* pTargetSE);

    void                ClearTargets();                 // clear our targets and set repeat=false
    void                ClearAllTargets();              // clear all targets and set repeat=false

    int8                GetState();                     // for EncodeDestiny()

    void                Abandon();                      // disable timers and null targets
    void                AssignShip(ShipSE* pSE);

    float               GetSpeedFraction();

    int32               GetOrbitDistance()              { return m_orbitDistance; }
    uint32              GetMaxSpeed()                   { return m_maxSpeed; }

    const Vector3d&      GetVelocity()                   { return m_velocity; }
    float               GetVelocityX()                  { return m_velocity.x; }
    float               GetVelocityY()                  { return m_velocity.y; }
    float               GetVelocityZ()                  { return m_velocity.z; }

    /* for advanced AI communication */
    // our assigned ship has acquired a new target
    void                ShipAddedTarget(SystemEntity* pTargetSE);
    // our assigned ship has been yellowboxed
    void                ShipAttacked(SystemEntity* pSourceSE)   { /* not implemented */ }
    // source has completed target lock on our assigned ship
    void                ShipTargeted(SystemEntity* pSourceSE);
    // our assigned ship is taking damage
    void                ShipTakingDamage(SystemEntity* pSourceSE)  { /* not implemented */ }
    // our assigned ship has been attacked
    void                MissileLaunched(Missile* pMissile);
    // our assigned ship has has lost (0=none, 1=shields, 2=armor)
    void                ReportDamage(uint8 type=0, SystemEntity* pSourceSE=nullptr);
    void                TargetDestroyed(SystemEntity* pTargetSE);
    // for assisting.  offensive module activated on target
    void                ModuleActivated(SystemEntity* pTargetSE);

    // this is public to allow deletion from DroneSE object
    InventoryItemRef    m_ore;                          //ore from mining

    void                SendGFX(Client* pClient=nullptr); // active = (action==engaged)
    void                CancelGFX();


protected:
    void                MineTarget();                   // actual mining code
    void                ClearTarget();                  // actual clear code (lol)
    void                AttackTarget();                 // actual attack code
    void                RepairTarget();                 // called when close enough to engage - sets action
    void                OrbitTarget();                  // called when setting initial orbit - sets action, usf and heading
    // this is advanced AI to determine threats to assigned ship and/or look for and acquire new targets
    void                FindTarget();

    bool                ValidTarget();

    // checks if target is within m_proximityDistance to react
    bool                InProxmityDistance(SystemEntity* pTargetSE);      // mutable 2 - 4
    // checks if target is within <config.interactdist> to interact with target
    bool                InActionDistance(SystemEntity* pTargetSE);        // ~600m
    // checks if target is within m_orbitRange to orbit target
    bool                InOrbitDistance(SystemEntity* pTargetSE);         // near - range 1
    // checks if target is within m_falloffRange to move closer to target
    bool                InFalloffDistance(SystemEntity* pTargetSE);        // close - range 2
    // checks if target is within m_engageDistance to engage with target
    bool                InEngageDistance(SystemEntity* pTargetSE);        // mid - range 3
    // checks if target is within m_chaseRange to chase target
    bool                InChaseDistance(SystemEntity* pTargetSE);         // far - range 4
    // checks if target is within m_maxRange of target
    bool                InMaxDistance(SystemEntity* pTargetSE);           // distant - range 5

    const char*         GetStateName(int8 stateID);
    const char*         GetActionName(int8 stateID);

    //  here, we are sending drone current action to client...Approaching, Fighting, Returning, Idle, etc.
    // may get expensive with many drones in bubble (high wire data)
    void                SendTrueState(int8 stateID=DroneAI::State::Idle);

    /* internal destiny methods.  testing drone w/o actual DestinyManager (dont need the overhead) */
    Vector3d             m_heading;                      // well, drone heading, ofc
    Vector3d             m_velocity;                     // current speed and heading

    void                Stop();                         // called when offline - calls SetIdle()
    void                Pause();                        // called when orbiting - sets position and velocity then stops movement and processing

    void                Move(double timeStamp=0);       // called by proc tic to keep ship position accurate

    void                SendSpeedFraction();            // orbit is only sent when target changes;  SF is sent when speed changes

    void                SetState(int8 stateID=-1);
    void                SetAction(int8 actionID=-1);
    void                MarkPoint(const Vector3d& position);

    // this is for tracking position changes from moving target
    void                MoveDrone(SystemEntity* pTarget); // where drone remains in set orbit around moving target
    // this is for tracking position changes from moving drone
    void                UpdatePosition(bool update=false); // where drone velocity changes


private:
    SystemEntity*       targSE;
    SystemEntity*       fTargSE;                        // FocusFire target (for later)
    DroneSE*            mySE;
    ShipSE*             shipSE;

    TurretFormulas      m_formula;

    EVEItemFlags        m_holdFlag;

    Timer               m_processTimer;                 // set to m_cycleTime;  for all drones to engage target
    Timer               m_warpScramblerTimer;
    Timer               m_webifierTimer;

    bool                m_sendCmd;                      // send updated orbit packet?
    bool                m_booster;                      // repair drone
    bool                m_repeat;                       // is drone action repeatable?
    bool                m_focusfire;                    // common target between all active drones

    int8                m_state;
    int8                m_action;

    uint16              m_effectID;                     // default effectID

    int32               m_cycleTime;                    // time in ms for drone to complete an action

    //in order of distance  far to close
    int32               m_maxDistance;                  //[5] maximum engagement distance
    int32               m_chaseDistance;                //[4] min distance to activate mwd, if equipped
    int32               m_engageDistance;               //[3] max distance drone will engage a target
    int32               m_falloffDistance;              //[2] distance where accuracy has fallen by half
    int32               m_orbitDistance;                //[1] distance the drone orbits
    int32               m_proximityDistance;            // distance at which drone reacts to relevant objects (threat sensor distance)

    uint32              m_maxSpeed;                     // mwd speed  - stationary drones have zero here
    uint32              m_cruiseSpeed;                  // normal speed

    int64               m_startTime;                    // timestamp when effect started

    float               m_alignTime;
    float               m_accelTime;

    float               m_timeFraction;                 //fuzzy logic - holds current euler value for time
    float               m_prevSpeedFraction;            //fuzzy logic - previous percent of full speed.  used for speed changes
    float               m_userSpeedFraction;            //fuzzy logic - user commanded percent of max speed
    float               m_activeSpeedFraction;          //fuzzy logic - current percent of max speed

    double              m_agility;
    double              m_moveTime;                     //in ms       - time when speed change started.  used to calculate m_timeFraction
};

#endif  // __EVEMU_SHIP_DRONEAI_H__

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

/*  defined in client
 *
 attributeEntityArmorRepairAmount = 631
 attributeEntityAttackDelayMax = 476
 attributeEntityAttackDelayMin = 475
 attributeEntityAttackRange = 247
 attributeEntityBracketColour = 798
 attributeEntityChaseMaxDelay = 580
 attributeEntityChaseMaxDelayChance = 581
 attributeEntityChaseMaxDistance = 665
 attributeEntityChaseMaxDuration = 582
 attributeEntityChaseMaxDurationChance = 583
 attributeEntityCruiseSpeed = 508
 attributeEntityDefenderChance = 497
 attributeEntityEquipmentGroupMax = 465
 attributeEntityEquipmentMax = 457
 attributeEntityEquipmentMin = 456
 attributeEntityFactionLoss = 562
 attributeEntityFlyRange = 416
 attributeEntityFlyRangeFactor = 772
 attributeEntityGroupRespawnChance = 640
 attributeEntityGroupArmorResistanceBonus = 1676
 attributeEntityGroupArmorResistanceActivationChance = 1682
 attributeEntityGroupArmorResistanceDuration = 1681
 attributeEntityGroupPropJamBonus = 1675
 attributeEntityGroupPropJamActivationChance = 1680
 attributeEntityGroupPropJamDuration = 1679
 attributeEntityGroupShieldResistanceBonus = 1671
 attributeEntityGroupShieldResistanceActivationChance = 1673
 attributeEntityGroupShieldResistanceDuration = 1672
 attributeEntityGroupSpeedBonus = 1674
 attributeEntityGroupSpeedActivationChance = 1678
 attributeEntityGroupSpeedDuration = 1677
 attributeEntityKillBounty = 481
 attributeEntityLootCountMax = 251
 attributeEntityLootCountMin = 250
 attributeEntityLootValueMax = 249
 attributeEntityLootValueMin = 248
 attributeEntityMaxVelocitySignatureRadiusMultiplier = 1133
 attributeEntityMaxWanderRange = 584
 attributeEntityMissileTypeID = 507
 attributeEntityRemoteECMBaseDuration = 1661
 attributeEntityRemoteECMChanceOfActivation = 1664
 attributeEntityRemoteECMDuration = 1658
 attributeEntityRemoteECMDurationScale = 1660
 attributeEntityRemoteECMExtraPlayerScale = 1662
 attributeEntityRemoteECMIntendedNumPlayers = 1663
 attributeEntityRemoteECMMinDuration = 1659
 attributeEntitySecurityMaxGain = 563
 attributeEntitySecurityStatusKillBonus = 252
 attributeEntityWarpScrambleChance = 504
 */
