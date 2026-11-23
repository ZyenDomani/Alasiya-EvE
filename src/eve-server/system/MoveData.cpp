/*
 *	MoveData.cpp
 *		this class takes all the old destiny variables and herds them into a common group container,
 * 		allowing ease of access without cluttering up SE or II base class members.
 *
 * @Author:     Allan
 * @Version:    0.20
 * @Date:       11Nov26  (partially copied from DestinyManager.h)
 */

#include "MoveData.h"

#include "utils/EvilNumber.h"
#include "npc/NPC.h"
#include "Client.h"
#include "SpaceEntity.h"
#include "SystemManager.h"
#include "BallPark.h"
#include "EVEServerConfig.h"


MoveData::MoveData(SpaceEntity* pSE)
: mySE(pSE),
targetSE(nullptr),
targetPoint(NULL_ORIGIN),
myVelocity(NULL_ORIGIN),
desiredHeading(NULL_ORIGIN),
desiredVelocity(NULL_ORIGIN),
expTerm(0.0),
posScale(0.0),
timeFactor(0.0),
agility(0.0),
timeStamp(0.0),
alignTime(0.0),
warpSpeed(0.0),
speedToLeaveWarp(0.0),
warpCapNeed(0.0),
targetDistance(0),
desiredDistance(0),
ticStamp(0),
maxSpeed(0.0f),
userSpeedFraction(0.0f),
activeSpeedFraction(0.0f),
ballMode(Ball::Mode::STOP),
moveDelay(false),
skipTic(false),
alignTo(false),
paused(false),
posHack(false),
sentUpdates(false),
stop(true),
tractored(false),
tractorPause(false),
massive(true),
autoPilot(false),
cloaked(false),
undocking(false)
{
    // defaults above are for creation.  actual values calculated once object is fully created
}

void MoveData::Init() {
    assert(mySE != nullptr);
    
    if (mySE->IsDynamicEntity() or mySE->IsNPCSE())
        UpdateShipVariables();

    // this is tricky.  client's destiny tics are based on when ballpark was created with ref to solar system tics
    ticStamp = mySE->SystemMgr()->GetTicCount();
}

void MoveData::Clear() {
    warpState.Reset();
    targetSE = nullptr;
    targetPoint = NULL_ORIGIN;
    myVelocity = NULL_ORIGIN;
    desiredHeading = NULL_ORIGIN;
    desiredVelocity = NULL_ORIGIN;
    timeFactor = 0.0;
    timeStamp = 0.0;
    targetDistance = 0;
    desiredDistance = 0;
    userSpeedFraction = 0.0f;
    activeSpeedFraction = 0.0f;
    ballMode = Ball::Mode::STOP;
    moveDelay = false;
    skipTic = false;
    alignTo = false;
    paused = false;
    posHack = false;
    stop = true;
    tractored = false;
    tractorPause = false;
    massive = true;
    autoPilot = false;
    cloaked = false;
    undocking = false;
}

void MoveData::SetAgilityInertia() {
    InventoryItemRef sRef = mySE->GetSelf();
    if (!mySE->IsNPCSE() and !mySE->IsShipSE() and !sRef->HasAttribute(AttrInertiaMod))
        sLog.Error("MoveData::UpdateShipVariables", "%s(%u) does not have an InertiaMod", mySE->GetName(), mySE->GetTypeID());

    float mass = (sRef->HasAttribute(AttrMass) ? sRef->GetAttribute(AttrMass).get_float() : sRef->mass()); // in mKg
    double inertiaMod = sRef->GetAttribute(AttrInertiaMod).get_double();

    // This represents e^(-dt/tau) where dt = tic time in seconds (dt = 1.0)
    agility = mass * inertiaMod / Destiny_k;
    expTerm = std::exp(-1.0 / agility);
    posScale = agility * (1.0 - expTerm);
    alignTime = 1.3862943611198906 * agility;
    sRef->SetAttribute(AttrAgility, agility, false);

    if (is_log_enabled(DESTINY__TRACE))
        _log(DESTINY__TRACE, "MoveData::SetAgilityInertia() %s(%u) - alignTime: %0.1f, mass: %0.1f * inertiaMod: %f = agility: %f, radius: %i", \
                mySE->GetName(), mySE->GetID(), alignTime, mass, inertiaMod, agility, sRef->radius());
}

void MoveData::UpdateShipVariables() {
    /** @todo check for movement when fleet boosts are applied and this is called */

    /* this sets variables needed for correct movement math.
     *  these attribs are set for ship item when ship's SE created.  DO NOT modify anything here
     * this is also called when fleet boosts are updated.
     */

    SetAgilityInertia();

    InventoryItemRef sRef = mySE->GetSelf();
    if (mySE->IsNPCSE()) {
        // npcs dont have AttrWarpSpeedMultiplier.  set based on size
        switch (mySE->GetNPCSE()->GetAI()->GetSize()) {
            //0.25, 0.75, 1.0, 1.25, 1.5, 2.0, 3.0, 3.75, 4.5, 5.0, 6.0, 7.5, 9.0, 10.0, 13.5, 15.0
            case NPC::Size::Swarm: {
                warpSpeed = 15.0f;
            } break;
            case NPC::Size::Frigate: {
                warpSpeed = 9.0f;
            } break;
            case NPC::Size::Destroyer: {
                warpSpeed = 7.5f;
            } break;
            case NPC::Size::Cruiser: {
                warpSpeed = 5.0f;
            } break;
            case NPC::Size::BCruiser: {
                warpSpeed = 3.0f;
            } break;
            case NPC::Size::BShip: {
                warpSpeed = 1.5f;
            } break;
            case NPC::Size::Indy: {
                warpSpeed = 0.75f;
            } break;
            default: {
                sLog.Warning("MoveData::UpdateShipVariables", "NPC %s(%u) has no size.  warp speed is %0.2f", \
                        mySE->GetName(), mySE->GetTypeID(), warpSpeed);
            } break;
        }
    } else if (sRef->HasAttribute(AttrWarpSpeedMultiplier)) {
        // this will catch speeds/needs for all player ships, and is easier to do here.
        warpSpeed = sRef->GetAttribute(AttrWarpSpeedMultiplier).get_float();
    }

    if (warpSpeed < 0.25f)
        sLog.Error("MoveData::UpdateShipVariables", "%s(%u) warpSpeed is %0.2f", \
                mySE->GetName(), mySE->GetTypeID(), warpSpeed);

    if (sRef->HasAttribute(AttrWarpCapacitorNeed)) {
        warpCapNeed = sRef->GetAttribute(AttrWarpCapacitorNeed).get_double();
    } else {
        warpCapNeed = 0.000000138;   // lowest value in db
    }

    if (mySE->IsNPCSE()) {
        // w're setting initial max speed to cruise speed
        maxSpeed = sRef->GetAttribute(AttrEntityCruiseSpeed).get_float();
    } else if (sRef->HasAttribute(AttrMaxVelocity)) {
        maxSpeed = sRef->GetAttribute(AttrMaxVelocity).get_float();
    } else {
        if (!mySE->IsShipSE())
            sLog.Warning("MoveData::UpdateShipVariables", "%s(%u) is !npc and has no AttrMaxVelocity.", \
                mySE->GetName(), mySE->GetTypeID());
        maxSpeed = 150.0f;
    }

    speedToLeaveWarp = maxSpeed * 0.85f;

    if (speedToLeaveWarp < 100)
        speedToLeaveWarp = 100;

    // verify hull overspeed isnt reached
    if (sRef->HasAttribute(AttrMaxDirectionalVelocity)) {
        if (maxSpeed > sRef->GetAttribute(AttrMaxDirectionalVelocity).get_float())
            maxSpeed = sRef->GetAttribute(AttrMaxDirectionalVelocity).get_float();
    }

    if (!mySE->HasPilot() or (mySE->GetPilot() == nullptr) or mySE->GetPilot()->IsLogin())
        return;

    if (mySE->GetPilot()->IsInSpace() and (mySE->SysBubble() != nullptr)) {
        std::vector<PyTuple*> updates;
        SetBallAgility sbagility;
            sbagility.entityID =  mySE->GetID();
            sbagility.agility = sRef->GetAttribute(AttrInertiaMod).get_double();
        updates.push_back(sbagility.Encode());
        if (sConfig.cosmic.BumpEnabled) {
            SetBallMassive sbmassive;
                sbmassive.entityID = mySE->GetID();
                sbmassive.is_massive = 1;
            updates.push_back(sbmassive.Encode());
        }
        SetBallMass sbmass;
            sbmass.entityID = mySE->GetID();
            sbmass.mass = sRef->mass();
        updates.push_back(sbmass.Encode());
        SetBallSpeed sbspeed;
            sbspeed.entityID = mySE->GetID();
            sbspeed.speed = maxSpeed;
        updates.push_back(sbspeed.Encode());
        mySE->SystemMgr()->SendDestinyUpdates(mySE, updates); //consumed
        sentUpdates = true;
    }
}

void MoveData::SetTrollData(MoveData* parentData) {
    // this method takes movement data from parent object and imposes onto wreck object to enable wreck movement
    userSpeedFraction = 0.0f;
    ballMode = Ball::Mode::STOP;
    expTerm = parentData->expTerm;
    posScale = parentData->posScale;
    ticStamp = parentData->ticStamp;
    targetPoint = parentData->targetPoint;
    myVelocity = parentData->myVelocity;
    desiredHeading = parentData->desiredHeading;
    desiredVelocity = parentData->desiredVelocity;
    timeFactor = parentData->timeFactor;
    timeStamp = parentData->timeStamp;
    targetDistance = parentData->targetDistance;
    desiredDistance = parentData->desiredDistance;
    maxSpeed = parentData->maxSpeed;
    activeSpeedFraction = parentData->activeSpeedFraction;

    std::vector<PyTuple*> updates;
    SetBallAgility sbagility;
        sbagility.entityID =  mySE->GetID();
        sbagility.agility = 1.0;
    updates.push_back(sbagility.Encode());
    SetBallSpeed sbspeed;
        sbspeed.entityID = mySE->GetID();
        sbspeed.speed = maxSpeed;
    updates.push_back(sbspeed.Encode());
    mySE->SystemMgr()->SendDestinyUpdates(mySE, updates); //consumed
}

void MoveData::TractorBeamStart(SpaceEntity* pSE, EvilNumber speed) {
    stop = false;
    ballMode = Ball::Mode::FOLLOW;
    tractored = true;
    ticStamp = mySE->SystemMgr()->GetTicCount();

    targetSE = pSE;
    targetPoint = pSE->GetPosition();
    Vector3d targetHeading = targetPoint - mySE->GetPosition();
    double distance = targetHeading.Length();
    // expand normal to avoid dupe sqrt since we have dist already
    const double invDist = 1.0 / distance;
    Vector3d head = targetHeading * invDist;

    maxSpeed = speed.get_float();
    myVelocity.x = head.x * maxSpeed;
    myVelocity.y = head.y * maxSpeed;
    myVelocity.z = head.z * maxSpeed;
    targetDistance = static_cast<int64>(distance + 500 + pSE->GetRadius());
    desiredHeading = head;
    desiredVelocity = myVelocity;
    desiredDistance = 500;

    userSpeedFraction = 1.0f;
    activeSpeedFraction = 1.0f;

    std::vector<PyTuple*> updates;
    SetBallSpeed ms;
        ms.entityID = mySE->GetID();
        ms.speed = maxSpeed;
    updates.push_back(ms.Encode());
    SetBallFree bf;
        bf.entityID = mySE->GetID();
        bf.is_free = 1;
    updates.push_back(bf.Encode());
    // may not be needed
    SetBallMass sbmass;
        sbmass.entityID = mySE->GetID();
        sbmass.mass = 10000;
    updates.push_back(sbmass.Encode());
    CmdSetSpeedFraction ssf;
        ssf.entityID = mySE->GetID();
        ssf.fraction = 1.0;
    updates.push_back(ssf.Encode());
    CmdFollowBall fb;
        fb.entityID = mySE->GetID();
        fb.targetID = pSE->GetID();
        fb.range = static_cast<int32>(targetDistance);
    updates.push_back(fb.Encode());
    mySE->SystemMgr()->SendDestinyUpdates(mySE, updates); //consumed
}

void MoveData::TractorBeamStop() {

    BallPark::Halt(this, false);
    tractored = false;
    std::vector<PyTuple*> updates;
    SetBallSpeed ms;
        ms.entityID = mySE->GetID();
        ms.speed = 0;
    updates.push_back(ms.Encode());
    SetBallFree bf;
        bf.entityID = mySE->GetID();
        bf.is_free = 0;
    updates.push_back(bf.Encode());
    SetBallMass sbmass;
        sbmass.entityID = mySE->GetID();
        sbmass.mass = mySE->GetSelf()->mass();
    updates.push_back(sbmass.Encode());
    mySE->SystemMgr()->SendDestinyUpdates(mySE, updates); //consumed
}

void MoveData::WebbedMe() {
    // TODO:  this will need more thought and work to play nice with npc speeds
    double oldMaxSpeed = maxSpeed;

    SetAgilityInertia();

    // Safety fallback: prevent division by zero if a ship somehow has a 0m/s baseline max speed
    if (maxSpeed > 1e-6) {
        // Scale user speed throttle fraction across the web transition
        userSpeedFraction = (userSpeedFraction * oldMaxSpeed) / maxSpeed;
    }

    SetBallSpeed sbs;
        sbs.entityID = mySE->GetID();
        sbs.speed    = maxSpeed;
    PyTuple *up = sbs.Encode();
    mySE->SystemMgr()->SendSingleUpdate(mySE, &up);
    sentUpdates = true;
}


bool MoveData::ValidTarget() {
    return true;
}

void MoveData::EntityRemoved(SpaceEntity* pSE) {

}
void MoveData::SpeedBoost(bool deactivate/*false*/) {
    // Recalculate ship mass and speed variables
    UpdateShipVariables();

    // this is just for debug logging
    if (is_log_enabled(DESTINY__MOVE_TRACE)) {
        // ship is currently ...
        if (deactivate) {
            // ... deactivating prop mod
            _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::Deactivate");
        } else if (activeSpeedFraction < userSpeedFraction) {
            // ....moving and accelerating
            _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(psf!=0 & asf<usf) - accelerating.");
        } else if (activeSpeedFraction > userSpeedFraction) {
            // ....moving and decelerating
            // - this also hits when prop mod activated while ship is decel
            _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(psf!=0 & asf>usf) - decelerating.");
        } else if (activeSpeedFraction) {
            // ....moving and not decelerating (this includes turning)
            _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(0<asf<=usf)");
        } else {
            // ....sitting still - do nothing
            if (userSpeedFraction) {
                _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(usf>asf=0) -  sitting still.");
            } else {
                _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost()::(usf=asf=0) -  sitting still.");
            }
        }

        _log(DESTINY__MOVE_TRACE, "Destiny::SpeedBoost() - nMass: %0.5f, nAg: %0.5f, usf: %0.2f, asf: %0.3f", \
                mySE->GetSelf()->mass(), agility, userSpeedFraction, activeSpeedFraction);
    }
}