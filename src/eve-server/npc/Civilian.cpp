
 /**
  * @name Civilian.cpp
  *   Civilian Non-Combatant class
  * @Author:    Allan
  * @date:      02 April 2017
 * @updated:   June 2026 (Refactored for Ambient FSM by Gemini)
  */

/* Civilian Logging
 * CIV__ERROR
 * CIV__WARNING
 * CIV__INFO
 */

#include <utility> // Required for std::move tracking on GCC 4.9

#include "EntityMgr.h"
#include "StaticDataMgr.h"
#include "destiny/DestinyStructs.h"
#include "npc/Civilian.h"
#include "station/StationDataMgr.h"
#include "system/SystemBubble.h"
#include "system/SystemEntity.h"
#include "system/SystemManager.h"


Civilian::Civilian(uint32 itemID, uint16 typeID)
:m_state(0),
m_formID(0),
m_timeLeft(0),
m_itemID(itemID),
m_corpID(0),
m_pos(NULL_ORIGIN),
m_heading(NULL_ORIGIN),
m_velocity(NULL_ORIGIN),
m_pLeader(nullptr),
m_type(sItemFactory.GetType(typeID)),
m_origSE(nullptr),
m_destSE(nullptr)
{
    _log(CIV__INFO, "Created Civilian %s(%u).", m_type->name().c_str(), itemID);
}

Civilian::~Civilian() {
    for (auto cur : m_guards)
        m_origSE->SystemMgr()->RemoveCiv(cur);
}

void Civilian::Init(SystemEntity* pOrig, SystemEntity* pDest) {
    m_origSE = pOrig;
    m_destSE = pDest;

    //  how to get faction/corp from here??
    // can do either sDataMgr.GetRegionFaction() or sDataMgr.GetRegionRatFaction
    m_corpID = sDataMgr.GetFactionCorp(sDataMgr.GetRegionFaction(pOrig->SystemMgr()->GetRegionID()));

    SetVectors();
}

// must be called @ 1Hz for timer to count correctly
void Civilian::Process() {
    if (m_formID)  // auxiliary ship in a formation?
        return;

    if (--m_timeLeft > 0)
        return;

    switch (m_state) {
        case Civ::State::Arriving: {
            // They dropped out of warp at the destination stargate or station bubble!
            for (auto cur : m_guards)
                cur->Add(m_destSE->SysBubble());
            m_state = Civ::State::Completed;
            // approximate 'decel' time from bubble enter to stop
            m_timeLeft = 20;
        } break;
        case Civ::State::Departing: {
            m_state = Civ::State::Stopping;
            Vector3d targHeading = m_destSE->GetPosition() - m_origSE->GetPosition();
            m_heading = targHeading.Normalize();
            Warp(m_destSE);
            // Calculate align time based on ship vars
            m_timeLeft = GetAlignTime() - 8;
        } break;
        case Civ::State::Undocking: {
            m_state = Civ::State::Departing;
            m_timeLeft = 2;
        } break;
        case Civ::State::Stopping: {
            Stop();
            m_state = Civ::State::Completed;
            m_timeLeft = 8;
        } break;
        case Civ::State::Completed: {
            // If dest is a stargate: trigger the activation animation packet.
            if (m_destSE->IsGateSE())
                SendGateActivity(m_destSE);
            //if (m_destSE->IsStationSE())
            //    sTraderJoe.ExecuteCargoDrop(m_destSE); // Update marketbot with 'special, limited-time items'
            // run complete.  remove ship(s)
            Remove(m_destSE->SysBubble());
            m_destSE->SystemMgr()->RemoveCiv(this);
        } break;
        default: {
            // something weird happened...just delete
            Remove(m_destSE->SysBubble());
            m_destSE->SystemMgr()->RemoveCiv(this);
        } break;
    }
}

uint16 Civilian::GetAlignTime() {
    float mass = m_type->mass();
    double inertiaMod = m_type->GetAttribute(AttrInertiaMod).get_double();
    double agility = mass * inertiaMod / 1000000.0;
    return static_cast<uint16>(std::ceil(1.386294 * agility));
}

void Civilian::Stop() {
    CmdStop du;
        du.entityID = m_itemID;
    PyTuple *up = du.Encode();
    m_destSE->SysBubble()->BubblecastDestinyUpdate(&up, "Fake Destiny Updates for Civilian (Stop)");
    PyDecRef(up);
}

void Civilian::SetVectors() {
    Vector3d targHeading = m_destSE->GetPosition() - m_origSE->GetPosition();
    m_heading = targHeading.Normalize();

    // what and where?
    if (IsEven(MakeRandomInt()) and !m_origSE->SysBubble()->IsEmpty()) {
        // we're leaving origin
        if (m_origSE->IsGateSE()) {
            // ...from a gate
            m_state = Civ::State::Departing;
            m_pos = m_origSE->GetPosition();
            m_pos.MakeRandomPointOnSphereLayer(1500, 3500);
            SendGateActivity(m_origSE);
        } else if (m_origSE->IsStationSE()) {  //(sDataMgr.IsStation(m_locationID)
            // ...from a station
            m_state = Civ::State::Undocking;
            m_timeLeft = 3;  // give em 5s to get bearings and program warp computer
            Undock();
        } else {
            // what other options are there?
            m_state = Civ::State::Completed;
            m_timeLeft = 5;
            // error?
            _log(CIV__ERROR, "Civilian::Init() - %s (%u) hit 'else' in origin check", m_type->name().c_str(), m_itemID);
            return;
        }
        Add(m_origSE->SysBubble());
    } else if (!m_destSE->SysBubble()->IsEmpty()) {
        // we're arriving at destination
        m_state = Civ::State::Arriving;
        m_timeLeft = 1;
        // get warp speed and set entry position at bubble boundry -5k
        m_pos = m_destSE->GetPosition() - (m_heading * (BUBBLE_RADIUS_METERS - 5000));
    	float mass = m_type->mass();
    	double inertiaMod = m_type->GetAttribute(AttrInertiaMod).get_double();
        double agility = mass * inertiaMod / 1000000.0;
        double speed = (1 - std::exp(-20 / agility));
        m_velocity = m_heading * speed;
        Add(m_destSE->SysBubble());
    } else {
        // what other options are there?
        m_state = Civ::State::Completed;
        m_timeLeft = 2;
        // error?
        _log(CIV__ERROR, "Civilian::Init() - %s (%u) hit 'else' in origin check", m_type->name().c_str(), m_itemID);
    }
}

void Civilian::Undock() {
    //get undock point and set heading
    StationData stationData;
    stDataMgr.GetStationData(m_origSE->GetID(), stationData);
    m_pos = stationData.dockPosition;
    m_heading = stationData.dockOrientation;
    m_velocity = stationData.dockOrientation * m_type->GetAttribute(AttrMaxVelocity).get_uint32();
}

void Civilian::Warp(SystemEntity* ptargSE) {
    std::vector<PyTuple*> up;
    // send warp update
    CmdWarpTo wt;
        wt.entityID = m_itemID;
    	wt.dest_x = ptargSE->GetPosition().x;
        wt.dest_y = ptargSE->GetPosition().y;
        wt.dest_z = ptargSE->GetPosition().z;
        wt.distance = 0;
        wt.warpSpeed = m_type->GetAttribute(AttrWarpFactor).get_int() * 10;      // warp speed x10
    up.push_back(wt.Encode());
    m_origSE->SysBubble()->BubblecastDestinyUpdate(up, "Fake Destiny Updates for Civilian (WARP)");

    // send warp gfx
    //SendGFX10(m_itemID,"effects.Warping" );
}

void Civilian::Add(SystemBubble* pBubble) {
    Buffer* destinyBuffer = new Buffer();

    //create AddBalls header
    Destiny::AddBall_header head = Destiny::AddBall_header();
        head.packet_type = 1;   // 0 = full state   1 = balls
        head.stamp = sEntityMgr.GetStamp();
    destinyBuffer->Append(head);

    AddBalls addballs;
    //encode destiny binary
    EncodeDestiny(*destinyBuffer);
    addballs.state = new PyBuffer(&destinyBuffer);
	//encode damage state
    addballs.damageDict[m_itemID] = MakeDamageState();
	//encode SlimItem
    addballs.slims = new PyList();
    addballs.slims->AddItem(new PyObject("foo.SlimItem", MakeSlimItem()));
	//bubblecast the update
    PyTuple* up = addballs.Encode();
    pBubble->BubblecastDestinyUpdate(&up, "Civ AddBall");
    PySafeDecRef(up);

    delete destinyBuffer; // Safe tracking cleanup
}

void Civilian::Remove(SystemBubble* pBubble) {
    RemoveBallsFromBP removeball;
    removeball.balls.push_back(m_itemID);

    _log(DESTINY__MESSAGE, "SysBubble::RemoveBall()");
    if (is_log_enabled(DESTINY__BALL_DUMP))
        removeball.Dump(DESTINY__BALL_DUMP, "    ");

    PyTuple *tmp = removeball.Encode();
    pBubble->BubblecastDestinyUpdate(&tmp, "Civ RemoveBall");
    PySafeDecRef(tmp);
}

PyTuple* Civilian::MakeDamageState() {
    DoDestinyDamageState ddds;
    ddds.shield = 1;
    ddds.recharge = 11000;
    ddds.armor = 1;
    ddds.structure = 1;
    ddds.timestamp = GetFileTimeNow();
    return ddds.Encode();
}

PyDict* Civilian::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for Civilian %s(%u)", m_type->name().c_str(), m_itemID);
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",       new PyLong(m_itemID));
        slim->SetItemString("typeID",       new PyInt(m_type->id()));
        slim->SetItemString("name",         new PyString(m_type->name()));
        slim->SetItemString("nameID",       PyStatic.NewNone());
        slim->SetItemString("ownerID",      PyStatic.NewOne());
    return slim;
}

void Civilian::SendGateActivity(SystemEntity* pGateSE) const {
    std::string guid = "effects.GateActivity";
    OnSpecialFX10 effect;
        effect.entityID = pGateSE->GetID();
        effect.targetID = PyStatic.NewNone();
        effect.otherTypeID = PyStatic.NewNone();
        effect.area = PyStatic.mtList();        // Optional 3D Sound/Shader Bounding Area Array (Defaults to Point-Source if Empty).
        effect.guid = guid;
        effect.isOffensive = 0;
        effect.start = 1;
        effect.active = 0;
    PyTuple *up = effect.Encode();
    pGateSE->SysBubble()->BubblecastDestinyEvent(&up, "Civ Gate Event" );
    PyDecRef(up);
}

void Civilian::EncodeDestiny( Buffer& into) {
    using namespace Destiny;
    uint8 mode = Ball::Mode::STOP;
    switch (m_state) {
        case Civ::State::Departing: {
            mode = Ball::Mode::WARP;
        }  break;
        case Civ::State::Arriving:
        case Civ::State::Undocking: {
            mode = Ball::Mode::GOTO;
        }  break;
        case Civ::State::Formation: {
            mode = Ball::Mode::FORMATION;
        }  break;
    }
    BallHeader head = BallHeader();
        head.entityID = m_itemID;
        head.mode = mode;
        head.radius = m_type->radius();
        head.posX = m_pos.x;
        head.posY = m_pos.y;
        head.posZ = m_pos.z;
        head.flags = Ball::Flag::IsFree;
    into.Append(head);
    MassSector mass = MassSector();
        mass.mass = m_type->mass();
        mass.cloak = 0;
        mass.harmonic = 0;
        mass.corporationID = m_corpID;
        mass.allianceID = -1;
    into.Append(mass);
    DataSector data = DataSector();
        data.inertia = m_type->GetAttribute(AttrInertiaMod).get_float();
        data.maxSpeed = m_type->GetAttribute(AttrMaxVelocity).get_uint32();
        data.velX = m_velocity.x;
        data.velY = m_velocity.y;
        data.velZ = m_velocity.z;
        data.speedfraction = (m_velocity.Length() / data.maxSpeed);
    into.Append(data);
    switch (m_state) {
        case Civ::State::Departing: {
            Vector3d target = m_destSE->GetPosition();
            WARP_Struct warp;
                warp.formationID = -1;
                warp.targX = target.x;
                warp.targY = target.y;
                warp.targZ = target.z;
                warp.warpFactor = 150;       //ship warp speed x10
                warp.effectStamp = -1;
                warp.distance = -1;
                warp.warpInVelocity = 15000;
            into.Append(warp);
        }  break;
        case Civ::State::Undocking: {
            Vector3d target = m_pos + (m_heading * 1.0e10);
            GOTO_Struct go;
                go.formationID = -1;
                go.x = target.x;
                go.y = target.y;
                go.z = target.z;
            into.Append(go);
        }  break;
        case Civ::State::Arriving: {
            Vector3d target = m_destSE->GetPosition();
            GOTO_Struct go;
                go.formationID = -1;
                go.x = target.x;
                go.y = target.y;
                go.z = target.z;
            into.Append(go);
        }  break;
        case Civ::State::Formation: {
            FORMATION_Struct form;
                form.formationID = m_formID;
                form.leaderID = m_pLeader->GetID();
                form.spacing = 800.0f;
                form.slotID = 0;
            into.Append(form);
        }  break;
        default: {
            STOP_Struct main;
                main.formationID = -1;
            into.Append( main );
        } break;
    }
}


// may not need/use these...
void Civilian::SendShipVars(SystemBubble* pBubble) {
    /*** add ship data ***/
    std::vector<PyTuple*> updates;
    SetBallAgility sbagility;
        sbagility.entityID =  m_itemID;
        sbagility.agility = m_type->GetAttribute(AttrInertiaMod).get_double();
    updates.push_back(sbagility.Encode());
    SetBallMassive sbmassive;
        sbmassive.entityID = m_itemID;
        sbmassive.is_massive = 0;
    updates.push_back(sbmassive.Encode());
    SetBallMass sbmass;
        sbmass.entityID = m_itemID;
        sbmass.mass = m_type->GetAttribute(AttrMass).get_uint32();
    updates.push_back(sbmass.Encode());
    SetBallSpeed sbspeed;
        sbspeed.entityID = m_itemID;
        sbspeed.speed = m_type->GetAttribute(AttrMaxVelocity).get_uint32();
    updates.push_back(sbspeed.Encode());
    if ((m_state == Civ::State::Departing) and (m_origSE->IsStationSE())) {
        StationData stationData;
        stDataMgr.GetStationData(m_origSE->GetID(), stationData);
        // undock velocity
        CmdGotoDirection du;
            du.entityID = m_itemID;
            du.x = stationData.dockOrientation.x;
            du.y = stationData.dockOrientation.y;
            du.z = stationData.dockOrientation.z;
        updates.push_back(du.Encode());
        SetBallVelocity bv;
            bv.entityID = m_itemID;
            bv.x = m_velocity.x;
            bv.y = m_velocity.y;
            bv.z = m_velocity.z;
        updates.push_back(bv.Encode());
    }
    pBubble->BubblecastDestinyUpdate(updates, "Fake Destiny Updates for Civilian ship vars");
}
