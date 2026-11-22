/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
    Copyright 2016 - 2026 Alasiya-EvE by Allan
    For the latest implementation status visit http://eve.alasiya.net/?p=op_status
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
    Author:        Zhur
    Rewrite:    Allan
*/

#include "../eve-server.h"
#include "math/Trig.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "StaticDataMgr.h"
#include "cache/ObjCacheService.h"
#include "planet/CustomsOffice.h"
#include "planet/Planet.h"
#include "planet/PlanetDB.h"
#include "planet/Moon.h"
#include "pos/Structure.h"
#include "ship/BeyonceService.h"
#include "station/StationDataMgr.h"
#include "system/BallPark.h"
#include "system/BookmarkService.h"
#include "system/Container.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/AnomalyMgr.h"
#include "system/cosmicMgrs/ManagerDB.h"

class BeyonceBound
: public PyBoundObject
{
public:
    PyCallable_Make_Dispatcher(BeyonceBound)

    BeyonceBound(PyServiceMgr* mgr, Client* pClient)
    : PyBoundObject(mgr),
      m_dispatch(new Dispatcher(this))
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "BeyonceBound";

        PyCallable_REG_CALL(BeyonceBound, UpdateStateRequest);
        PyCallable_REG_CALL(BeyonceBound, CmdStop);
        PyCallable_REG_CALL(BeyonceBound, CmdSetSpeedFraction);
        PyCallable_REG_CALL(BeyonceBound, CmdDock);
        PyCallable_REG_CALL(BeyonceBound, CmdFollowBall);   //*
        PyCallable_REG_CALL(BeyonceBound, CmdOrbit);   //*
        PyCallable_REG_CALL(BeyonceBound, CmdAlignTo);
        PyCallable_REG_CALL(BeyonceBound, CmdGotoDirection);   //*
        PyCallable_REG_CALL(BeyonceBound, CmdGotoBookmark);
        PyCallable_REG_CALL(BeyonceBound, CmdWarpToStuff);
        PyCallable_REG_CALL(BeyonceBound, CmdStargateJump);
        PyCallable_REG_CALL(BeyonceBound, CmdWarpToStuffAutopilot); //*
        PyCallable_REG_CALL(BeyonceBound, CmdAbandonLoot);

        PyCallable_REG_CALL(BeyonceBound, CmdFleetRegroup);
        PyCallable_REG_CALL(BeyonceBound, CmdFleetTagTarget);
        PyCallable_REG_CALL(BeyonceBound, CmdBeaconJumpFleet);
        PyCallable_REG_CALL(BeyonceBound, CmdBeaconJumpAlliance);
        PyCallable_REG_CALL(BeyonceBound, CmdJumpThroughFleet);
        PyCallable_REG_CALL(BeyonceBound, CmdJumpThroughAlliance);
        PyCallable_REG_CALL(BeyonceBound, CmdJumpThroughCorporationStructure);

        pClient->SetBeyonce(true);
        if (pClient->IsLogin() and !pClient->IsSetStateSent())
            pClient->SetBallPark();
    }

    virtual ~BeyonceBound()
    {
        delete m_dispatch;
    }

    PyCallable_DECL_CALL(UpdateStateRequest);
    PyCallable_DECL_CALL(CmdStop);
    PyCallable_DECL_CALL(CmdSetSpeedFraction);
    PyCallable_DECL_CALL(CmdDock);
    PyCallable_DECL_CALL(CmdStargateJump);
    PyCallable_DECL_CALL(CmdFollowBall);
    PyCallable_DECL_CALL(CmdOrbit);
    PyCallable_DECL_CALL(CmdAlignTo);
    PyCallable_DECL_CALL(CmdGotoDirection);
    PyCallable_DECL_CALL(CmdGotoBookmark);
    PyCallable_DECL_CALL(CmdWarpToStuff);
    PyCallable_DECL_CALL(CmdWarpToStuffAutopilot);
    PyCallable_DECL_CALL(CmdAbandonLoot);

    PyCallable_DECL_CALL(CmdFleetRegroup);
    PyCallable_DECL_CALL(CmdFleetTagTarget);
    PyCallable_DECL_CALL(CmdJumpThroughFleet);
    PyCallable_DECL_CALL(CmdBeaconJumpFleet);
    PyCallable_DECL_CALL(CmdBeaconJumpAlliance);
    PyCallable_DECL_CALL(CmdJumpThroughAlliance);
    PyCallable_DECL_CALL(CmdJumpThroughCorporationStructure);

protected:
    Dispatcher *const m_dispatch;
};

PyCallable_Make_InnerDispatcher(BeyonceService)

BeyonceService::BeyonceService(PyServiceMgr *mgr)
: PyService(mgr, "beyonce"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(BeyonceService, GetFormations)
}

BeyonceService::~BeyonceService() {
    delete m_dispatch;
}

PyBoundObject* BeyonceService::CreateBoundObject(Client* pClient, const PyRep* bind_args)
{
    return new BeyonceBound(m_manager, pClient);
}

PyResult BeyonceService::Handle_GetFormations(PyCallArgs &call) {
    // this is only called when player enters new system and calls to bind new beyonce
    if (!call.client->IsSetStateSent())
        call.client->CheckBallparkTimer();

    return sDataMgr.GetFormations();
}

PyResult BeyonceBound::Handle_CmdStop(PyCallArgs &call) {

    _log(AUTOPILOT__MESSAGE, "%s called Stop. AP: %s, Invul: %s, Jump: %s", call.client->GetName(), \
            (call.client->IsAutoPilot() ? "true" : "false"), (call.client->IsInvul()?"true":"false"), \
            (call.client->IsJump()?"true":"false"));

    ShipSE* sSE = call.client->GetShipSE();
    MoveData* mData = sSE->GetMoveData();
    if (mData == nullptr)
        mData = sSE->CreateMoveData();

    mData->SetUndocking(false);

    BallPark::Stop(mData);

    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdGotoDirection(PyCallArgs &call) {
    _log(AUTOPILOT__MESSAGE, "%s called GotoDirection. AP: %s", call.client->GetName(), (call.client->IsAutoPilot() ? "true" : "false"));

    PointArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    call.client->SetInvul(false);

    ShipSE* sSE = call.client->GetShipSE();
    MoveData* mData = sSE->GetMoveData();
    if (mData == nullptr)
        mData = sSE->CreateMoveData();

    mData->SetAutoPilot(false);
    mData->SetUndocking(false);

    Vector3d direction(arg.x, arg.y, arg.z);
    mData->SetHeading(direction);
    mData->SetTargetPoint(mData->GetMySE()->GetPosition() + (direction * 1e9));

    BallPark::GotoDirection(mData);

    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdSetSpeedFraction(PyCallArgs &call) {

    Call_SingleRealArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    _log(AUTOPILOT__MESSAGE, "%s called SetSpeedFraction to %.3f. AP: %s", \
            call.client->GetName(), arg.arg, (call.client->IsAutoPilot() ? "true" : "false"));

    ShipSE* sSE = call.client->GetShipSE();
    MoveData* mData = sSE->GetMoveData();
    if (mData == nullptr)
        mData = sSE->CreateMoveData();

    BallPark::SetSpeedFraction(mData, arg.arg);

    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdAlignTo(PyCallArgs &call) {
    _log(AUTOPILOT__MESSAGE, "%s called Align. AP: %s", call.client->GetName(), (call.client->IsAutoPilot() ? "true" : "false"));

    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

    CallAlignTo arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    SpaceEntity* pSE = pSystem->GetSE(arg.entityID);
    if (pSE == nullptr) {
        _log(CLIENT__ERROR, "%s: Unable to find entity %u to AlignTo.", call.client->GetName(), arg.entityID);
        return PyStatic.NewNone();
    }

    call.client->SetInvul(false);

    ShipSE* sSE = call.client->GetShipSE();
    MoveData* mData = sSE->GetMoveData();
    if (mData == nullptr)
        mData = sSE->CreateMoveData();

    mData->SetAutoPilot(false);
    mData->SetUndocking(false);
    mData->SetTargetSE(pSE);
    mData->SetTargetPoint(pSE->GetPosition());
    BallPark::AlignTo(mData);

    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdStargateJump(PyCallArgs &call) {
    /*  jump system messages....
     * (67187, `{[location]system.name} Traffic Control: your jump-in clearance has expired.`)
     * (67191, `{[location]system.name} Traffic Control: you have been cleared for jump-in within {[timeinterval]expiration.writtenForm, from=second, to=second}.`)
     * (67210, `{[location]system.name} Traffic Control: you are at position {[numeric]pos} in queue for jump-in.`)
     * (258673, `{system} is currently loading. Bizzarre, seeing that you're there already... Please try again in a minute or two.`)
     * (258674, `The stargates in {system} are currently experiencing minor technical difficulties. Please try again in a moment.`)
     * (258675, `Jump Prohibited`)
     * (258676, `Officials have closed the stargates in {system} due to heavy congestion. Travelers are advised to enjoy the local scenery.`)
     * (258677, `{system} Traffic Control is currently experiencing heavy load and is unable to process your request. Please try again in a moment.`)
     * (258678, `{system} Traffic Control is currently offline and unable to process your jump request. Please try again in a moment.`)
     * (258679, `{system} Traffic Control is currently experiencing heavy load and is unable to process your request. You are #{[numeric]position} in queue for jump-in.`)
     * (258680, `Jump Prohibited`)
     * (258681, `Officials have closed the stargates in {system} due to heavy congestion. Travelers are advised to seek alternate routes.`)
     * (258683, `Your character is located within {system}, which is currently loading. Please try again in a moment.`)
     * (258685, `Your character is located within {system}, which is currently experiencing heavy load. Please try again in a moment.`)
     * (258687, `Your character is located within {system}, which has reached maximum capacity. You are #{[numeric]position} in queue for entrance. Please try again in a moment.`)
     * (258689, `Your character is located within {system}, which is currently stuck. Please select a different character or try later.`)
     * (258691, `The character is located within {system}, which is currently loading. Please try again in a moment.`)
     * (258693, `The character is located within {system}, which is currently experiencing heavy load. Please try again in a moment.`)
     * (258695, `The character is located within {system}, which is currently stuck. Please try again later.`)
     * (258699, `The item <b>{example}</b> was not found in the station <b>{[location]station.name}</b>. Please make sure all items are in the correct hangar before finalizing this contract.`)
     * (258701, `{system} Traffic Control is currently experiencing heavy load and was unable to process your request. Please try again in a moment.`)
     * (258704, `You cannot leave {system} yet because of instability in the space-time continuum. Please try again in a moment.`)
     */

    _log(AUTOPILOT__MESSAGE, "%s called Jump. AP: %s", call.client->GetName(), (call.client->IsAutoPilot() ? "true" : "false"));
    if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change currently active.");
        return PyStatic.NewNone();
    }
    Call_StargateJump args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    SpaceEntity* pGateSE = call.client->SystemMgr()->GetSE(args.fromStargateID);
    if (pGateSE == nullptr) {
        call.client->SendNotifyMsg("Error - Stargate not found.");
        _log(SERVICE__ERROR, "%s: Unable to find stargate %i.", call.client->GetName(), args.fromStargateID);
        return PyStatic.NewNone();
    }

    Vector3d delta =  pGateSE->GetPosition() - call.client->GetShipSE()->GetPosition();
    double dist = delta.Length();
    dist -= pGateSE->GetRadius();
    if (dist < 2500) {
        call.client->StargateJump(args.fromStargateID, args.toStargateID);
    } else {
        // {'FullPath': u'UI/Messages', 'messageID': 277027, 'label': u'GenericTooFarAway'}(u'You are too far away to do that.', None, None)
        call.client->SendNotifyMsg( "You are too far from the Gate to activate it. You need to be within 2500m but are %.0fm.", dist);

        ShipSE* sSE = call.client->GetShipSE();
        MoveData* mData = sSE->GetMoveData();
        if (mData == nullptr)
            mData = sSE->CreateMoveData();

        mData->SetTargetSE(pGateSE);
        mData->SetTargetPoint(pGateSE->GetPosition());
        BallPark::ApproachBall(mData);
        return PyStatic.NewNone();
    }

    /* return error msg from this call, if applicable, else none
    PyTuple* tuple = new PyTuple(2);
    tuple->SetItem(0, new PyString(GetBindStr()));    // node info here
    tuple->SetItem(1, new PyLong(GetFileTimeNow()));
    return tuple;
    */
    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdFollowBall(PyCallArgs &call) {
    _log(AUTOPILOT__MESSAGE, "%s called Follow. AP: %s", call.client->GetName(), (call.client->IsAutoPilot() ? "true" : "false"));

    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }
    Call_FollowBall args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    SpaceEntity* targetSE = pSystem->GetSE(args.ballID);
    if (targetSE == nullptr) {
        _log(CLIENT__ERROR, "%s: Unable to find entity %i to Follow/Approach.", call.client->GetName(), args.ballID);
        return PyStatic.NewNone();
    }

    ShipSE* sSE = call.client->GetShipSE();
    MoveData* mData = sSE->GetMoveData();
    if (mData == nullptr)
        mData = sSE->CreateMoveData();

    mData->SetUndocking(false);
    mData->SetTargetSE(targetSE);
    if (!BallPark::ValidTarget(mData)) {
        BallPark::Stop(mData);
        return PyStatic.NewNone();
    }

    call.client->SetInvul(false);

    // ap sends distance as float.  all others use int
    if (args.distance->IsFloat())
        mData->SetAutoPilot(true);

    mData->SetTargetPoint(mData->GetMySE()->GetPosition());
    mData->SetDistance(PyRep::IntegerValueI32(args.distance) + mData->GetMySE()->GetRadius() + targetSE->GetRadius());

    BallPark::FollowBall(mData);

    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdGotoBookmark(PyCallArgs &call) {
    _log(AUTOPILOT__MESSAGE, "%s called GotoBookmark. AP: %s", call.client->GetName(), (call.client->IsAutoPilot() ? "true" : "false"));

    SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    BookmarkService* pBMSvc = (BookmarkService*)(call.client->services().LookupService( "bookmark" ));
    if (pBMSvc == nullptr) {
        sLog.Error( "BeyonceService::Handle_GotoBookmark()", "Attempt to access BookmarkService via (BookmarkService*)(call.client->services().LookupService(\"bookmark\")) returned NULL." );
        return PyStatic.NewNone();
    }

    double x = 0.0, y = 0.0, z = 0.0;
    uint32 itemID =0, locationID = 0;
    uint16 typeID = 0;

    ShipSE* sSE = call.client->GetShipSE();
    MoveData* mData = sSE->GetMoveData();
    if (mData == nullptr)
        mData = sSE->CreateMoveData();

    call.client->SetInvul(false);

    mData->SetUndocking(false);
    mData->SetAutoPilot(false);
    pBMSvc->LookupBookmark(arg.arg, itemID, typeID, locationID, x, y, z);

    if (typeID == 5) {
        if (call.client->GetSystemID() != locationID) {
            // todo: this bm is for different system.  make and send error here.
            return PyStatic.NewNone();
        }

        Vector3d point(x, y, z);
        mData->SetTargetPoint(point);
        mData->SetHeading(point - mData->GetMySE()->GetPosition());
        //mData->SetTargetDistance(mData->desiredHeading.Length());
        BallPark::GotoPoint(mData);
    } else {
        // Bookmark type is of a static system entity, so search for it and obtain its coordinates:
        SpaceEntity* targetSE = call.client->SystemMgr()->GetSE(itemID);
        if (targetSE == nullptr) {
            sLog.Error( "BeyonceService::Handle_GotoBookmark()", "%s: unable to find location %d", call.client->GetName(), itemID );
            return PyStatic.NewNone();
        }

        mData->SetTargetSE(targetSE);
        if (!BallPark::ValidTarget(mData)) {
            BallPark::Stop(mData);
            return PyStatic.NewNone();
        }

        mData->SetTargetPoint(targetSE->GetPosition());
        mData->SetDistance(0);
        BallPark::FollowBall(mData);
    }

    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdOrbit(PyCallArgs &call) {
    // is orbiting even enabled?
    if (!sConfig.debug.UseOrbit) {
        call.client->SendNotifyMsg("Orbiting is disabled due to excessive desync.");
        return PyStatic.NewNone();
    }

    _log(AUTOPILOT__MESSAGE, "%s called Orbit. AP: %s", call.client->GetName(), (call.client->IsAutoPilot() ? "true" : "false"));


    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

    if (is_log_enabled(SERVICE__CALL_DUMP))
        call.Dump(SERVICE__CALL_DUMP);

    Call_Orbit args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    SpaceEntity* targetSE = pSystem->GetSE(args.entityID);
    if (targetSE == nullptr) {
        _log(CLIENT__ERROR, "%s: Unable to find entity %u to Orbit.", call.client->GetName(), args.entityID);
        return PyStatic.NewNone();
    }

    ShipSE* sSE = call.client->GetShipSE();
    MoveData* mData = sSE->GetMoveData();
    if (mData == nullptr)
        mData = sSE->CreateMoveData();

    mData->SetTargetSE(targetSE);
    if (!BallPark::ValidTarget(mData)) {
        BallPark::Stop(mData);
        return PyStatic.NewNone();
    }

    mData->SetUndocking(false);
    mData->SetAutoPilot(false);
    call.client->SetInvul(false);

    // not sure if i wanna use desired or target here...currently orbit calc's use desired
    mData->SetDistance(PyRep::IntegerValueU32(args.range) + mData->GetMySE()->GetRadius() + targetSE->GetRadius());
    //mData->targetDistance = mData->desiredDistance + mData->GetMySE()->GetRadius() + targetSE->GetRadius();

    BallPark::OrbitBall(mData);

    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdWarpToStuff(PyCallArgs &call) {
    _log(AUTOPILOT__MESSAGE, "%s called WarpToStuff. AP: %s", call.client->GetName(), (call.client->IsAutoPilot() ? "true" : "false"));

    if (is_log_enabled(SERVICE__CALL_DUMP)) {
        _log(SERVICE__CALL_DUMP, "BeyonceBound::Handle_CmdWarpToStuff() - size %zu", call.tuple->size() );
        call.Dump(SERVICE__CALL_DUMP);
    }

    if (call.tuple->size() < 2)
        return PyStatic.NewNone();

    Client* pClient = call.client;
    if (pClient->GetShipSE()->SysBubble() == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client is not in a bubble!", pClient->GetName());
        return PyStatic.NewNone();
    }
    /** @todo (allan) finish warp scramble system */
    // >0 means ship cannot warp (warp stabs are neg values, warp scrams are pos values)
    if (pClient->GetShip()->GetAttribute(AttrWarpScrambleStatus) > 0)
        throw UserError("WarpScrambled");

    SystemManager* pSystem = pClient->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", pClient->GetName());
        return PyStatic.NewNone();
    }

    bool fleet = false;
    if (call.byname.find("fleet") != call.byname.end())
        if (!(call.byname.find("fleet")->second->IsNone()))
            fleet = call.byname.find("fleet")->second->AsBool()->value();

    // get the warp-to distance specified by the client
    int32 distance = 0;
    if (call.byname.find("minRange") != call.byname.end())
        distance = PyRep::IntegerValueI32(call.byname.find("minRange")->second);

    Vector3d warpToPoint = NULL_ORIGIN;
    SpaceEntity* targetSE = nullptr;
    int32 radius = 0;
    uint32 toID = 0;
    std::string stringArg = "";

    if ((call.tuple->GetItem(1)->IsString())
    or  (call.tuple->GetItem(1)->IsWString())) {
        stringArg = PyRep::StringContent(call.tuple->GetItem(1));
    } else {
        toID = PyRep::IntegerValueU32(call.tuple->GetItem(1));
    }

    std::string type = PyRep::StringContent(call.tuple->GetItem(0));
    if (type == "item" ) {
        targetSE = pSystem->GetSE(toID);
        if (targetSE == nullptr) {
            codelog(CLIENT__ERROR, "%s: unable to find item location %u in %s(%u)", pClient->GetName(), toID, pSystem->GetName(), pSystem->GetID());
            return PyStatic.NewNone();
        }
    } else if (type == "bookmark" ) {
        double x = 0.0, y = 0.0, z = 0.0;
        uint16 typeID = 0;
        uint32 locationID = 0;
        uint32 bookmarkID = PyRep::IntegerValueU32(call.tuple->GetItem(1));

        BookmarkService* bkSrvc = (BookmarkService *)(pClient->services().LookupService( "bookmark" ));
        if (bkSrvc == nullptr) {
            sLog.Error( "BeyonceService::Handle_WarpToStuff()", "Attempt to access BookmarkService returned NULL." );
            return PyStatic.NewNone();
        }
        bkSrvc->LookupBookmark(bookmarkID, toID, typeID, locationID, x, y, z);

        if ( typeID == 5 ) {
            if (pClient->GetSystemID() != locationID) {
                //  this bm is for different system.  make error here.
                return PyStatic.NewNone();
            }
            warpToPoint.x = x;
            warpToPoint.y = y;
            warpToPoint.z = z;
        } else {
            // Bookmark type is of a static system entity, so search for it and obtain its coordinates:
            targetSE = pSystem->GetSE( toID );
            if (targetSE == nullptr) {
                codelog(CLIENT__ERROR, "%s: unable to find bookmark location %u in %s(%u)", pClient->GetName(), toID, pSystem->GetName(), pSystem->GetID());
                return PyStatic.NewNone();
            }
        }
    } else if (type == "scan") {
        uint32 anomID = pSystem->GetAnomMgr()->GetAnomalyID(stringArg);
        targetSE = pSystem->GetSE(anomID);
        if (targetSE == nullptr) {
            codelog(CLIENT__ERROR, "%s: unable to find location %s in %s(%u)", pClient->GetName(), stringArg.c_str(), pSystem->GetName(), pSystem->GetID());
            pClient->SendErrorMsg("Cannot find location data for anomalyID %s.", stringArg.c_str());
            return PyStatic.NewNone();
        }
    } else if (type == "launch") {
        //warpToPoint = PlanetDB::GetLaunchPos(toID);
        targetSE = pSystem->GetSE(toID);
        // launchpickup - launch, launchid
        if (targetSE == nullptr) {
            codelog(CLIENT__ERROR, "%s: unable to find location for %u in %s(%u)", pClient->GetName(), toID, pSystem->GetName(), pSystem->GetID());
            pClient->SendErrorMsg("Cannot find location data for launchID %u.", toID);
            return PyStatic.NewNone();
        }
    }
    // the systems below are not implemented yet.  hold on coding till systems are working and we know what needs to be done here
    // more info can be found in client::menuSvc.py
    else if (type == "epinstance") {
        // epinstance, instanceid
        //stringArg
        pClient->SendErrorMsg("WarpToInstance is not implemented at this time.");
        return PyStatic.NewNone();
    } else if (type == "tutorial") {
        // tutorial, none
        pClient->SendErrorMsg("WarpToTutorial is not implemented at this time.");
        return PyStatic.NewNone();
    } else if (type == "char") {
    //  fleet warping
    // [warptomember] char, charid, minrange
    // [warpfleettomember] char, charid, minrange, fleet=1
        pClient->SendErrorMsg("WarpToChar is not implemented at this time.");
        return PyStatic.NewNone();
    } else {
        sLog.Error( "BeyonceService::Handle_WarpToStuff()", "Unexpected type value: '%s'.", type.c_str() );
        return PyStatic.NewNone();
    }

        /* formulas for warpin points for all objects
         * x,y,z = object coords.  r = object radius
         *
         * for objects <90kr
         * dest = x,y,z + (vector - r)
         *
         * for objects >90kr
         * (x + (r + 5000000) * cos(r), y + 1.3r -7500, z - (r + 5000000) * sin(r))
         *
         * for planets, its a bit different
         * (x + d*sin(t), y + 0.5*r*sin(j), z - d*cos(t))
         * where:
         * j = rand(seed=planetID).rand(0,1) - 1.0/3.0
         * d = r*(s+1) +1000000
         * t = sin^-1(x/abs(x) * z / sqrt(x^2 + z^2) +j)
         * s = 20*((1/40)*(10*log10(r/10^6)-39)^20) +0.5
         * s = max(0.5, min(s, 10.5))
         */
        /** @todo  that may not be right....
         * this is straight from client
         *
         * def GetPlanetWarpInPoint(planetID, locVec, r):
         *    dx = float(locVec[0])
         *    dz = float(locVec[2])
         *    f = float(dz) / float(math.sqrt(dx ** 2 + dz ** 2))
         *    if dz > 0 and dx > 0 or dz < 0 and dx > 0:
         *        f *= -1.0
         *    theta = math.asin(f)
         *    myRandom = random.Random(planetID)
         *    rr = (myRandom.random() - 1.0) / 3.0
         *    theta += rr
         *    offset = 1000000
         *    FACTOR = 20.0
         *    dd = math.pow((FACTOR - 5.0 * math.log10(r / 1000000) - 0.5) / FACTOR, FACTOR) * FACTOR
         *    dd = min(10.0, max(0.0, dd))
         *    dd += 0.5
         *    offset += r * dd
         *    d = r + offset
         *    x = 1000000
         *    z = 0
         *    x = math.sin(theta) * d
         *    z = math.cos(theta) * d
         *    y = r * math.sin(rr) * 0.5
         *    return util.KeyVal(x=x, y=y, z=z)
         */
    if (targetSE != nullptr) {
        radius = targetSE->GetRadius();
        // this will need adjustment for warping to bookmarks
        warpToPoint = targetSE->GetPosition();
        if (targetSE->IsPlanetSE()) {
            if (targetSE->GetPlanetSE()->HasCOSE()) {
                // if planet has a customs office, make warpin point 5km from co.
                warpToPoint = targetSE->GetPlanetSE()->GetCustomsOffice()->GetPosition();
                Vector3d vectorFromOrigin = warpToPoint - pClient->GetShipSE()->GetPosition();
                vectorFromOrigin.Normalize();   //we now have a direction
                Vector3d stopPoint = (vectorFromOrigin * 10000);
                distance += targetSE->GetPlanetSE()->GetCustomsOffice()->GetRadius();
                distance += pClient->GetShip()->radius();
                warpToPoint.x -= (stopPoint.x - distance);
                warpToPoint.y -= (stopPoint.y - distance);
                warpToPoint.z -= (stopPoint.z - distance);
                distance = 0;
            } else {
                srandom(toID);  //this is the only place random() is used....other random functions use rand() as it's non-repeatable.
                int rand = random();
                double j = (((static_cast<double>(rand) / RAND_MAX) - 1.0f) / 3.0f);
                double s = 20 * std::pow(0.025f * (10 * std::log10(static_cast<double>(radius) / 1000000.0) - 39), 20) + 0.5f;
                s = EvE::max(0.5f, EvE::min(s, 10.5f));
                double t = std::asin((warpToPoint.x / std::fabs(warpToPoint.x)) * (warpToPoint.z / std::sqrt(pow(warpToPoint.x, 2) + std::pow(warpToPoint.z, 2)))) + j;
                uint32 d = radius * (s + 1) + 1000000;
                warpToPoint.x += (d * std::sin(t));
                warpToPoint.y += (0.5f * radius * std::sin(j));
                warpToPoint.z -= (d * std::cos(t));
            }
        } else if (targetSE->IsStationSE()) {
            // fudge the distance a bit for these... its' a lil close by default
            Vector3d vectorFromOrigin = warpToPoint - pClient->GetShipSE()->GetPosition();
            vectorFromOrigin.Normalize();   //we now have a direction
            Vector3d stopPoint = (vectorFromOrigin * (radius + 2500 + pClient->GetShip()->radius()));
            warpToPoint.x -= stopPoint.x;
            warpToPoint.z -= stopPoint.z;
            // this makes ship warp to station dock elevation (y), instead of warping to stations "center point" position (where icon is)
            warpToPoint.y = stDataMgr.GetDockPosY(targetSE->GetID());
        } else if (targetSE->IsCOSE() or targetSE->IsGateSE()) {
            // fudge the distance a bit for these... its' a lil close by default
            Vector3d vectorFromOrigin = warpToPoint - pClient->GetShipSE()->GetPosition();
            vectorFromOrigin.Normalize();   //we now have a direction
            Vector3d stopPoint = (vectorFromOrigin * (radius + 1500 + pClient->GetShip()->radius()));
            warpToPoint.x -= stopPoint.x;
            warpToPoint.y -= stopPoint.y;
            warpToPoint.z -= stopPoint.z;
        } else if (targetSE->IsMoonSE()) {
            if (targetSE->GetMoonSE()->HasTower()) {
                // if moon has a tower, make warpin point 20km inside edge of tower's bubble.
                warpToPoint = targetSE->GetMoonSE()->GetMyTower()->GetPosition();
                Vector3d vectorFromOrigin = warpToPoint - pClient->GetShipSE()->GetPosition();
                vectorFromOrigin.Normalize();   //we now have a direction
                Vector3d stopPoint = (vectorFromOrigin * (BUBBLE_RADIUS_METERS - 20000));  // 20km inside bubble.
                warpToPoint.x -= stopPoint.x;
                warpToPoint.y -= stopPoint.y;
                warpToPoint.z -= stopPoint.z;
                distance = 0;
            } else {
                // hack for warping to moons
                // this puts ship at Az: 0.785332, Ele: 0.615505, angle: 1.5708
                radius *= 1.25f;
                warpToPoint.x -= radius;
                warpToPoint.y -= radius;
                warpToPoint.z -= radius;
            }
        } else if (targetSE->IsWormholeSE()) {
            // move warpin point -20km from center of wh.
            Vector3d vectorFromOrigin = warpToPoint - pClient->GetShipSE()->GetPosition();
            vectorFromOrigin.Normalize();   //we now have a direction
            Vector3d stopPoint = (vectorFromOrigin * (20000 + radius));
            warpToPoint.x -= stopPoint.x;
            warpToPoint.y -= stopPoint.y;
            warpToPoint.z -= stopPoint.z;
        } else if (radius > 90000) {
            // this doesnt work for moons
            warpToPoint.x += ((radius + 500000) * std::cos(radius));
            warpToPoint.y += ((radius * 1.3f) - 7500);
            warpToPoint.z -= ((radius + 500000) * std::sin(radius));
        }
        /*
        if (radius < 90000) {
            // this will include stations (max station radius 60km)
            Vector3d vectorFromOrigin(pClient->GetShipSE()->GetPosition(), warpToPoint);
            vectorFromOrigin.Normalize();   //we now have a direction
            Vector3d stopPoint = (vectorFromOrigin * radius);
            warpToPoint -= stopPoint;
        }
        */
    }

    if (warpToPoint.isZero()) {
        // point is zero ....make error and return
        codelog(CLIENT__ERROR, "%s: warpToPoint.isZero() = true.  Cannot find location %u for '%s'", \
                pClient->GetName(), toID, type.c_str());
        throw UserError("WarpDestinationGone");
    }

    ShipSE* shipSE = call.client->GetShipSE();
    MoveData* mData = shipSE->GetMoveData();
    if (mData == nullptr)
        mData = shipSE->CreateMoveData();

    mData->warpState.Reset();
    mData->SetUndocking(false);
    mData->SetAutoPilot(false);
    mData->SetTargetSE(targetSE);

    Vector3d warp_vector = warpToPoint - shipSE->GetPosition();
    // i dont think this should be sent after altering warpToPoint to account for distance.
    mData->SetDistance(distance);
    mData->SetTargetDistance(warp_vector.Length());
    if (mData->GetTargetDistance() < MIN_WARP_DISTANCE) {
        // warp distance too close.  cancel warp and return
        if (targetSE != nullptr) {
            //TooCloseToWarp
            pClient->SetInvul(false);
            pClient->SendErrorMsg("That is too close for your Warp Drive.  Approaching %s.", targetSE->GetName());
            BallPark::ApproachBall(mData);
        } else {
            pClient->SendErrorMsg("That is too close for your Warp Drive.  Stopping Ship.");
            BallPark::Stop(mData);
        }
        return PyStatic.NewNone();
    }

    // expand normal to avoid dupe sqrt since we have dist already
    const double invDist = 1.0 / mData->GetTargetDistance();
    Vector3d head = (warp_vector * invDist);
    mData->SetHeading(head);

    // warp point configured.  adjust for distance
    if ((targetSE != nullptr) and (targetSE->IsStationSE())) {
        // do not change y here; it is set to station dock elevation (vs 'middle' as on live)
        warpToPoint.x -= (head.x * distance);
        warpToPoint.z -= (head.z * distance);
    } else {
        warpToPoint -= (head * distance);
    }

    mData->SetTargetPoint(warpToPoint);

    // check for enough cap to warp.

    /*  capacitor for warp formulas from https://oldforums.eveonline.com/?a=topic&threadID=332116
     *  Energy to warp = warpCapacitorNeed * mass(kG) * au * (1 - warp_drive_operation_skill_level * 0.10)
     ** @note:  warpCapacitorNeed is type double...max ive seen is shuttles @ 0.00000134771  and indys @ 0.000000108911
     */

    double dist = ((double)mData->GetTargetDistance() / ONE_AU_IN_METERS);
    double currentShipCap = pClient->GetShip()->GetAttribute(AttrCapacitorCharge).get_double();
    double capNeeded = shipSE->GetSelf()->mass() * mData->GetWarpCapNeed() * dist;
    capNeeded *= (1.0 - (0.1 * pClient->GetChar()->GetSkillLevel(EvESkill::WarpDriveOperation)));

    _log(DESTINY__WARNING, "Warp Cap need for %s(%u) for %.0fm (%0.3f AU) is %0.5f GJ", \
    shipSE->GetName(), shipSE->GetID(), mData->GetTargetDistance(), dist, capNeeded);

    // set min cap need to 10.0  (config option?)
    if (capNeeded < 10.0)
        capNeeded = 10.0;

    //  check if ship has enough capacitor to warp full distance
    if (capNeeded > currentShipCap) {
        // not enough cap.  reset everything based on available cap
        double capHas = currentShipCap / (shipSE->GetSelf()->mass() * mData->GetWarpCapNeed());
        if (capHas > 1.0f) {
            mData->SetWarpCapNeed(0.1);
            double targDist = capHas * ONE_AU_IN_METERS;
            Vector3d warp_direction = mData->GetTargetPoint() - shipSE->GetPosition();
            // make heading
            warp_direction.Normalize();
            mData->SetHeading(warp_direction);
            mData->SetTargetPoint(shipSE->GetPosition() + (warp_direction * targDist));
            mData->SetTargetDistance(targDist);
            pClient->SendErrorMsg("Your commanded warp of %0.3f AU requires %0.1f GJ of capacitor charge, but you only have %0.1f GJ available.<br>This is enough for %0.3f AU.<br><br>Setting course for available capacitor charge.", \
            dist, capNeeded,  currentShipCap, capHas );
        } else {
            // if not enough cap to do min warp. cancel and return
            //WarpingWithAvailablePowerBody
            pClient->SendErrorMsg("You don't have enough capacitor charge to warp.");
            _log(DESTINY__WARNING, "Destiny::InitWarp() - %s(%u): Capacitor needed vs current  %f / %0.5f",
                 shipSE->GetName(), shipSE->GetID(), capNeeded, currentShipCap);

            BallPark::Stop(mData);
            return PyStatic.NewNone();
        }
    } else {
        mData->SetWarpCapNeed(currentShipCap - capNeeded);
    }

    pClient->SetInvul(false);


    //You will always exit warp at a random point 2,500 meters from your actual exit point - per EveUni
    //  this will need update for formation warp...leader will have random offset, others follow per formation
    /* disabled for testing
     * if (pSE->HasPilot())
     *   mData->targetPoint.MakeRandomPointOnSphereLayer(-2500, 2500);   disabled for testing
     */

    // get targ bubble; this will create bubble if needed, so never returns null
    mData->warpState.targBubble = sBubbleMgr.GetBubble(pSystem, mData->GetTargetPoint());

    /*supercap warp modifiers
     * these will go here, and modify distance, target, and range accordingly
     *
     * AttrWarpAccuracyMaxRange = 1021,
     * AttrWarpAccuracyFactor = 1022,
     * AttrWarpAccuracyFactorMultiplier = 1023,
     * AttrWarpAccuracyMaxRangeMultiplier = 1024,
     * AttrWarpAccuracyFactorPercentage = 1025,
     * AttrWarpAccuracyMaxRangePercentage = 1026,
     */


    /*  TODO PUT CHECK HERE FOR WARP BUBBLES
     *     and other things that affect warp-in point.....when we get to there.
     * AttrWarpBubbleImmune = 1538,  (none in db)
     * AttrWarpBubbleImmuneModifier = 1539,     (4 in db)
     *
     *   NOTE:  warp bubble in path (or within 150km of mData->targetPoint) will change mData->targetDistance and mData->targetPoint
     *   however, this does NOT affect original calculations for energy needed, etc...
     */

    /** @todo  does this apply for ANY bubble along warp route or just end?
     * would be fun to check entire route for bubble...can bm along route to add bubble
     * however, this will take a bit to implement
     *
     * IDEA:  ...nope, lost it.
     */
    assert(mData->warpState.targBubble != nullptr);
    if (mData->warpState.targBubble->HasWarpBubble())
        if (!shipSE->GetSelf()->HasAttribute(AttrWarpBubbleImmune)) {
            /*  there is a bubble here and ship isnt immune.
             * at this point, determine where the bubble is
             * then set ship targetPoint to random point 2500m(?) from center of bubble
             */
        }

    // found few warp error msgs in client and noted in BeyonceSvc.h
    //  test for and implement here

    // make sure this is jumpcloak and not module cloak
    //UnCloak();

    // if usf < 75%, set usf for warp
    if (mData->GetUSF() < 0.751)
        BallPark::SetSpeedFraction(mData, 1.0f);

    BallPark::WarpTo(mData, false);
    return PyStatic.NewNone();
}

/** @todo THIS is how we know client is using AP...it needs to be updated   */
PyResult BeyonceBound::Handle_CmdWarpToStuffAutopilot(PyCallArgs &call) {
    // this call shouldnt need all the checks player warp uses; ap warps to station or gate (others?)
    SystemManager* pSystem = call.client->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return PyStatic.NewNone();
    }

    //  sends targeted celestial's itemID as arg.destID
    // this is usually a gate or station.  no need for special checks here
    CallWarpToStuffAutopilot arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    SpaceEntity* targetSE = pSystem->GetSE(arg.destID);
    if (targetSE == nullptr) {
        codelog(CLIENT__ERROR, "%s: unable to find destination Entity for ID %u", call.client->GetName(), arg.destID);
        return PyStatic.NewNone();
    }

    call.client->SetInvul(false);

    ShipSE* sSE = call.client->GetShipSE();
    MoveData* mData = sSE->GetMoveData();
    if (mData == nullptr)
        mData = sSE->CreateMoveData();

    mData->SetUndocking(false);
    mData->SetAutoPilot(true);

    Vector3d warpToPoint = targetSE->GetPosition();
    Vector3d warp_vector = warpToPoint - sSE->GetPosition();

    double targDist = warp_vector.Length();
    mData->SetTargetDistance(targDist);
    // expand normal to avoid dupe sqrt since we have dist already
    const double invDist = 1.0 / targDist;
    mData->SetHeading(warp_vector * invDist);

    int32 radius = sSE->GetRadius() + sConfig.world.apWarptoDistance;
    Vector3d head = mData->GetDesiredHeading();
    warpToPoint.x -= (head.x * radius);
    warpToPoint.y -= (head.y * radius);
    warpToPoint.z -= (head.z * radius);

    mData->warpState.Reset();
    mData->SetTargetPoint(warpToPoint);
    mData->SetTargetSE(targetSE);
    mData->SetDistance(radius); // this may not be right...

    mData->warpState.targBubble = sBubbleMgr.GetBubble(targetSE->SystemMgr(), mData->GetTargetPoint());

    _log(AUTOPILOT__MESSAGE, "%s called WarpToStuffAutopilot. Target:%s(%u), BubbleID: %u", \
        call.client->GetName(), targetSE->GetName(), targetSE->GetID(), mData->warpState.targBubble->GetID());

    BallPark::WarpTo(mData, true);
    return PyStatic.NewNone();
}

// CmdTurboDock (in client code)
PyResult BeyonceBound::Handle_CmdDock(PyCallArgs &call) {
    Client* pClient = call.client;
    _log(AUTOPILOT__MESSAGE, "%s called Dock. AP: %s", pClient->GetName(), (pClient->IsAutoPilot() ? "true" : "false"));
    if (pClient->IsSessionChange()) {
        pClient->SendNotifyMsg("Session Change currently active.");
        return PyStatic.NewNone();
    }
    SystemManager* pSystem = pClient->SystemMgr();
    if (pSystem == nullptr) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager.", pClient->GetName());
        return PyStatic.NewNone();
    }
    Call_TwoIntegerArgs args;  //sends stationID, shipID
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    //  this sets m_dockStationID for radius checks and other things
    pClient->SetDockStationID(args.arg1);
    SpaceEntity* stationSE = pClient->SystemMgr()->GetSE(args.arg1);
    if (stationSE == nullptr) {
        codelog(CLIENT__ERROR, "%s: Station %u not found.", pClient->GetName(), args.arg1);
        pClient->SendErrorMsg("Station Not Found, Docking Aborted.");
        return PyStatic.NewNone();
    }

    ShipSE* pShipSE = pClient->GetShipSE();
    MoveData* mData = pShipSE->GetMoveData();
    mData->SetTargetSE(stationSE);

    //get the station Docking Perimeter
    Vector3d delta =  stationSE->GetPosition() - pShipSE->GetPosition();
    double distance = delta.Length() - (pShipSE->GetRadius() + stationSE->GetRadius());

    // Verify range to station is within docking perimeter of 2500 meters:
    _log(DESTINY__TRACE, "Destiny::AttemptDockOperation() rangeToStationPerimiter is %0.2fm", distance);
    if (distance > 2500.0) {
        // Turn ship and move toward docking point - client will usually call Dock() automatically...sometimes
        BallPark::ApproachBall(mData);
        throw UserError("DockingApproach");
    }

    pClient->SetStateTimer(Player::State::Dock, sConfig.world.StationDockDelay * 1000); // default @ 4sec();
    pClient->SetAutoPilot(false);

    /* return error msg from this call, if applicable, else nodeid and timestamp */
    return new PyLong(GetFileTimeNow());
}

PyResult BeyonceBound::Handle_CmdAbandonLoot(PyCallArgs &call) {
    /*  remotePark.CmdAbandonLoot(wrecks)  <- this is pylist from 'abandonAllWrecks'
     *  remotePark.CmdAbandonLoot([wreckID]) <- single itemID in list
     */
    if (is_log_enabled(SERVICE__CALL_DUMP)) {
    sLog.White( "BeyonceBound::Handle_CmdAbandonLoot()", "size= %zu", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);
    }

    Call_SingleIntList arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return PyStatic.NewNone();
    }

    /** @todo  change ownerID to _system for all loot also!!  */
    SpaceEntity* pSE = nullptr;
    SystemManager* pSysMgr = call.client->SystemMgr();
    for (auto &cur : arg.ints) {
        pSE = pSysMgr->GetSE(cur);
        if (pSE == nullptr)
            continue;
        pSE->Abandon();
        PyTuple* slimData = new PyTuple(2);
            slimData->SetItem(0, new PyLong(pSE->GetID()));
            slimData->SetItem(1, new PyObject( "foo.SlimItem", pSE->MakeSlimItem()));
        PyTuple* itemData = new PyTuple(2);
            itemData->SetItem(0, new PyString("OnSlimItemChange"));
            itemData->SetItem(1, slimData);
        pSE->SysBubble()->BubblecastDestinyUpdate(&itemData, "OnSlimItemChange" );
    }

    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_UpdateStateRequest(PyCallArgs &call) {
    sLog.Error("BeyonceBound", "UpdateStateRequest");
    codelog(CLIENT__ERROR, "%s: Client sent UpdateStateRequest. Previous call generated a bad return.  Check Logs.", call.client->GetName());


    if (call.client->GetShipSE() == nullptr)
        return PyStatic.NewNone();

    call.client->SetStateSent(false);
    call.client->SystemMgr()->SendBallPark(call.client->GetShipSE());

    return PyStatic.NewNone();
}

/**     ***********************************************************************
 * @note   fleet shit....these do absolutely nothing at this time....
 */


/** @todo these will need work....
 *    def BridgeToMember(self, charID):
 *        beaconStuff = sm.GetService('fleet').GetActiveBeaconForChar(charID)
 *        if beaconStuff is None:
 *            return
 *        self.BridgeToBeacon(charID, beaconStuff)
 *
 *    def BridgeToBeaconAlliance(self, solarSystemID, beaconID):
 *        bp = sm.StartService('michelle').GetRemotePark()
 *        if bp is None:
 *            return
 *        bp.CmdBridgeToStructure(beaconID, solarSystemID)
 *
 *    def BridgeToBeacon(self, charID, beacon):
 *        solarsystemID, beaconID = beacon
 *        bp = sm.StartService('michelle').GetRemotePark()
 *        if bp is None:
 *            return
 *        bp.CmdBridgeToMember(charID, beaconID, solarsystemID)
 *
 *    def JumpThroughFleet(self, otherCharID, otherShipID):
 *        bp = sm.StartService('michelle').GetRemotePark()
 *        if bp is None:
 *            return
 *        bridge = sm.GetService('fleet').GetActiveBridgeForShip(otherShipID)
 *        if bridge is None:
 *            return
 *        solarsystemID, beaconID = bridge
 *        self.LogNotice('Jump Through Fleet', otherCharID, otherShipID, beaconID, solarsystemID)
 *        sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdJumpThroughFleet, otherCharID, otherShipID, beaconID, solarsystemID)
 *
 *    def JumpThroughAlliance(self, otherShipID):
 *        bp = sm.StartService('michelle').GetRemotePark()
 *        if bp is None:
 *            return
 *        bridge = sm.StartService('pwn').GetActiveBridgeForShip(otherShipID)
 *        if bridge is None:
 *            return
 *        solarsystemID, beaconID = bridge
 *        self.LogNotice('Jump Through Alliance', otherShipID, beaconID, solarsystemID)
 *        sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdJumpThroughAlliance, otherShipID, beaconID, solarsystemID)
 *
 *    def JumpToMember(self, charid):
 *        beaconStuff = sm.GetService('fleet').GetActiveBeaconForChar(charid)
 *        if beaconStuff is None:
 *            return
 *        self.JumpToBeaconFleet(charid, beaconStuff)
 *
 *    def JumpToBeaconFleet(self, charid, beacon):
 *        solarsystemID, beaconID = beacon
 *        bp = sm.StartService('michelle').GetRemotePark()
 *        if bp is None:
 *            return
 *        self.LogNotice('Jump To Beacon Fleet', charid, beaconID, solarsystemID)
 *        for wnd in uicore.registry.GetWindows()[:]:
 *            if getattr(wnd, '__guid__', None) == 'form.CorpHangarArray':
 *                wnd.CloseByUser()
 *
 *        sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdBeaconJumpFleet, charid, beaconID, solarsystemID)
 *
 *    def JumpToBeaconAlliance(self, solarSystemID, beaconID):
 *        bp = sm.StartService('michelle').GetRemotePark()
 *        if bp is None:
 *            return
 *        self.LogNotice('Jump To Beacon Alliance', beaconID, solarSystemID)
 *        sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdBeaconJumpAlliance, beaconID, solarSystemID)
 *
 *    def ActivateGridSmartBomb(self, charid, effect):
 *        beaconStuff = sm.GetService('fleet').GetActiveBeaconForChar(charid)
 *        if beaconStuff is None:
 *            return
 *        solarsystemID, beaconID = beaconStuff
 *        bp = sm.StartService('michelle').GetRemotePark()
 *        if bp is None:
 *            return
 *        effect.Activate(beaconID, False)
 */
PyResult BeyonceBound::Handle_CmdJumpThroughFleet(PyCallArgs &call) {
    // sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdJumpThroughFleet, otherCharID, otherShipID, beaconID, solarsystemID)

    if (is_log_enabled(SHIP__WARNING)) {
    _log(SHIP__WARNING, "BeyonceBound::Handle_CmdJumpThroughFleet");
    call.Dump(SHIP__WARNING);
    }
    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdJumpThroughAlliance(PyCallArgs &call) {
    //sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdJumpThroughAlliance, otherShipID, beaconID, solarsystemID)
    if (is_log_enabled(SHIP__WARNING)) {
    _log(SHIP__WARNING, "BeyonceBound::Handle_CmdJumpThroughAlliance");
    call.Dump(SHIP__WARNING);
    }
    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdJumpThroughCorporationStructure(PyCallArgs &call) {
    //sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdJumpThroughCorporationStructure, itemID, remoteStructureID, remoteSystemID)
    if (is_log_enabled(SHIP__WARNING)) {
    _log(SHIP__WARNING, "BeyonceBound::Handle_CmdJumpThroughCorporationStructure");
    call.Dump(SHIP__WARNING);
    }
    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdBeaconJumpFleet(PyCallArgs &call) {
    // sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdBeaconJumpFleet, charid, beaconID, solarsystemID)
    if (is_log_enabled(SHIP__WARNING)) {
    _log(SHIP__WARNING, "BeyonceBound::Handle_CmdBeaconJumpFleet");
    call.Dump(SHIP__WARNING);
    }
    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdBeaconJumpAlliance(PyCallArgs &call) {
    // sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdBeaconJumpAlliance, beaconID, solarSystemID)
    if (is_log_enabled(SHIP__WARNING)) {
    _log(SHIP__WARNING, "BeyonceBound::Handle_CmdBeaconJumpAlliance");
    call.Dump(SHIP__WARNING);
    }
    return PyStatic.NewNone();
}

PyResult BeyonceBound::Handle_CmdFleetRegroup(PyCallArgs &call) {
    // not sure what this is supposed to do yet
    if (is_log_enabled(SHIP__WARNING)) {
    _log(SHIP__WARNING, "BeyonceBound::Handle_CmdFleetRegroup");
    call.Dump(SHIP__WARNING);
    }
    return nullptr;
}

PyResult BeyonceBound::Handle_CmdFleetTagTarget(PyCallArgs &call) {
    // bp.CmdFleetTagTarget(itemID, tag)
    if (is_log_enabled(SHIP__WARNING)) {
    _log(SHIP__WARNING, "BeyonceBound::Handle_CmdFleetTagTarget");
    call.Dump(SHIP__WARNING);
    }
    return PyStatic.NewNone();
}
