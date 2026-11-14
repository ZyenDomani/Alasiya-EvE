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

#include "eve-common/utils/EvilNumber.h"
#include "npc/NPC.h"
#include "SpaceEntity.h"


MoveData::MoveData(SpaceEntity* pSE, int32 currTic)
: mySE(pSE),
targSE(nullptr),
warpState(nullptr),
targPoint(NULL_ORIGIN),
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
followDistance(0),
ticStamp(currTic),
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
stop(false),
tractored(false),
tractorPause(false),
autoPilot(false)
{
    // defaults above are for creation.  actual values calculated once object is fully created
}

MoveData::Init() {
    UpdateShipVariables();
}

void MoveData::SetAgilityInertia() {
    InventoryItemRef sRef = mySE->GetSelf();
    if (!sRef->HasAttribute(AttrInertiaMod))    // this should never hit
        sLog.Error("DM::UpdateShipVariables", "%s(%u) does not have an InertiaMod", mySE->GetName(), mySE->GetTypeID());

    float mass = sRef->mass(); // in mKg
    double inertiaMod = sRef->GetAttribute(AttrInertiaMod).get_double();

    // This represents e^(-dt/tau) where dt = tic time in seconds (dt = 1.0)
    agility = mass * inertiaMod / Destiny_k;
    expTerm = std::exp(-1.0 / agility);
    posScale = agility * (1.0 - expTerm);
    alignTime = 1.3862943611198906 * agility;
    sRef->SetAttribute(AttrAgility, agility, false);

    if (is_log_enabled(DESTINY__TRACE))
        _log(DESTINY__TRACE, "Destiny::SetAgilityInertia() %s(%u) - maxSpeed: %0.1f, alignTime: %0.1f, mass: %0.1f * inertiaMod: %f = agility: %f, radius: %i", \
                mySE->GetName(), mySE->GetID(), maxSpeed, alignTime, mass, inertiaMod, agility, sRef->radius());
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
                sLog.Warning("DM::UpdateShipVariables", "NPC %s(%u) has no size.  warp speed is %0.2f", \
                        mySE->GetName(), mySE->GetTypeID(), warpSpeed);
            } break;
        }
    } else if (sRef->HasAttribute(AttrWarpSpeedMultiplier)) {
        // this will catch speeds/needs for all player ships, and is easier to do here.
        warpSpeed = sRef->GetAttribute(AttrWarpSpeedMultiplier).get_float();
    }

    if (warpSpeed < 0.25f)
        sLog.Error("DM::UpdateShipVariables", "%s(%u) warpSpeed is %0.2f", \
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
        sLog.Warning("DM::UpdateShipVariables", "%s(%u) is !npc and has no AttrMaxVelocity.", \
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
        SendDestinyUpdates(updates); //consumed
        mySE->GetMoveData()->sentUpdates = true;
    }
}
