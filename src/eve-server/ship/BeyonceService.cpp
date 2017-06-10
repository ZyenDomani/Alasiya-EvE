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
    Author:        Zhur
    Updates:    Allan
*/

#include "eve-server.h"

#include "PyBoundObject.h"
#include "PyServiceCD.h"
#include "StaticDataMgr.h"
#include "cache/ObjCacheService.h"
#include "planet/PlanetDB.h"
#include "ship/BeyonceService.h"
#include "system/DestinyManager.h"
#include "system/BookmarkService.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/ManagerDB.h"
#include "system/Container.h"

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

        PyCallable_REG_CALL(BeyonceBound, CmdFollowBall);
        PyCallable_REG_CALL(BeyonceBound, CmdOrbit);
        PyCallable_REG_CALL(BeyonceBound, CmdAlignTo);
        PyCallable_REG_CALL(BeyonceBound, CmdGotoDirection);
        PyCallable_REG_CALL(BeyonceBound, CmdGotoBookmark);
        PyCallable_REG_CALL(BeyonceBound, CmdSetSpeedFraction);
        PyCallable_REG_CALL(BeyonceBound, CmdStop);
        PyCallable_REG_CALL(BeyonceBound, CmdWarpToStuff);
        PyCallable_REG_CALL(BeyonceBound, CmdDock);
        PyCallable_REG_CALL(BeyonceBound, CmdStargateJump);
        PyCallable_REG_CALL(BeyonceBound, UpdateStateRequest);
        PyCallable_REG_CALL(BeyonceBound, CmdWarpToStuffAutopilot);
        PyCallable_REG_CALL(BeyonceBound, CmdAbandonLoot);

        // beyonce is constructed when player first enters system and not removed until sys change or logout.
        // these functions are only called when beyonce is created. (fix for BlackScreen Bug)
        if (pClient->IsLogin())
            pClient->SetBallPark();

        pClient->SetBeyonce(true);
    }

    virtual ~BeyonceBound()
    {
        delete m_dispatch;
    }

    virtual void Release() {
        //I hate this statement
        delete this;
    }

    PyCallable_DECL_CALL(CmdFollowBall);
    PyCallable_DECL_CALL(CmdOrbit);
    PyCallable_DECL_CALL(CmdAlignTo);
    PyCallable_DECL_CALL(CmdGotoDirection);
    PyCallable_DECL_CALL(CmdGotoBookmark);
    PyCallable_DECL_CALL(CmdSetSpeedFraction);
    PyCallable_DECL_CALL(CmdStop);
    PyCallable_DECL_CALL(CmdWarpToStuff);
    PyCallable_DECL_CALL(CmdDock);
    PyCallable_DECL_CALL(CmdStargateJump);
    PyCallable_DECL_CALL(UpdateStateRequest);
    PyCallable_DECL_CALL(CmdWarpToStuffAutopilot);
    PyCallable_DECL_CALL(CmdAbandonLoot);

    /** @todo  calls to add later (need fleet/corp shit)
     *
        sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdJumpThroughCorporationStructure, itemID, remoteStructureID, remoteSystemID)
        sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdJumpThroughFleet, otherCharID, otherShipID, beaconID, solarsystemID)
        sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdJumpThroughAlliance, otherShipID, beaconID, solarsystemID)
        sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdBeaconJumpFleet, charid, beaconID, solarsystemID)
        sm.StartService('sessionMgr').PerformSessionChange('jump', bp.CmdBeaconJumpAlliance, beaconID, solarSystemID)
     */

protected:
    Dispatcher *const m_dispatch;

};

PyCallable_Make_InnerDispatcher(BeyonceService)

BeyonceService::BeyonceService(PyServiceMgr *mgr)
: PyService(mgr, "beyonce"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    //PyCallable_REG_CALL(BeyonceService, )
    PyCallable_REG_CALL(BeyonceService, GetFormations)
}

BeyonceService::~BeyonceService() {
    delete m_dispatch;
}


PyBoundObject* BeyonceService::_CreateBoundObject( Client* c, const PyRep* bind_args )
{
    _log( CLIENT__MESSAGE, "BeyonceService bind request for:" );
    bind_args->Dump( COLLECT__OTHER_DUMP, "    " );
    /*
     * 18:26:29 [ClientMessage] BeyonceService bind request for:
     * 18:26:29 [ClientMessage]     Integer field: 30002547
     */

    return new BeyonceBound( m_manager, c );
}


PyResult BeyonceService::Handle_GetFormations(PyCallArgs &call) {
    ObjectCachedMethodID method_id(GetName(), "GetFormations");

    //check to see if this method is in the cache already.
    if (!m_manager->cache_service->IsCacheLoaded(method_id)) {
        //this method is not in cache yet, load up the contents and cache it.
        PyRep *res = m_db.GetFormations();
        if (!res) {
            codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
            res = new PyNone();
        }

        m_manager->cache_service->GiveCache(method_id, &res);
    }

    //now we know its in the cache one way or the other, so build a
    //cached object cached method call result.
    return(m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id));
    //return new PyTuple(0);
}

/*
PyResult BeyonceService::Handle_(PyCallArgs &call) {
    PyRep *result = NULL;

    return result;
}
*/

PyResult BeyonceBound::Handle_CmdFollowBall(PyCallArgs &call) {
    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return new PyNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return new PyNone();
    }
    SystemManager* pSystem = call.client->SystemMgr();
    if (!pSystem) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return new PyNone();
    }
    Call_FollowBall args;
    if (!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return new PyNone();
    }

    double distance = 0;
    if (args.distance->IsInt()) {
        distance = args.distance->AsInt()->value();
    } else {
        distance = args.distance->AsFloat()->value();
        _log(AUTOPILOT__INFO, "%s sent FollowBall() as float.", call.client->GetName());
    }

    SystemEntity* pEntity = pSystem->GetSE(args.ballID);
    if (!pEntity) {
        _log(CLIENT__ERROR, "%s: Unable to find entity %u to Follow/Approach.", call.client->GetName(), args.ballID);
        return new PyNone();
    }

    call.client->SetInvul(false);
    call.client->SetUndock(false);

    pDestiny->Follow(pEntity, distance);

    return new PyNone();
}

PyResult BeyonceBound::Handle_CmdSetSpeedFraction(PyCallArgs &call) {
    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return new PyNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return new PyNone();
    }

    Call_SingleRealArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
        return new PyNone();
    }
    /** @todo  rework this...this is to set speed ONLY...NOT to begin moving.  */
    // client should not legally send anything < 0.1 (except on rare occasion a 0.0 instead of Stop.)
    if ((arg.arg != 0) && (arg.arg < 0.1))
        return new PyNone();

    //sLog.Warning( "BeyonceBound", "Handle_CmdSetSpeedFraction %.2f", arg.arg );
    if (!call.client->IsUndock()){
        if (pDestiny->IsMoving())
            pDestiny->SetSpeedFraction(arg.arg);
        else
            pDestiny->SetSpeedFraction(arg.arg, true);
    }

    return new PyNone();
}

PyResult BeyonceBound::Handle_CmdAlignTo(PyCallArgs &call) {
    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return new PyNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return new PyNone();
    }
    SystemManager* pSystem = call.client->SystemMgr();
    if (!pSystem) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return new PyNone();
    }

    CallAlignTo arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
        return new PyNone();
    }

    SystemEntity* pEntity = pSystem->GetSE(arg.entityID);
    if (!pEntity) {
        _log(CLIENT__ERROR, "%s: Unable to find entity %u to AlignTo.", call.client->GetName(), arg.entityID);
        return new PyNone();
    }

    call.client->SetInvul(false);
    call.client->SetUndock(false);

    pDestiny->AlignTo( pEntity );

    return new PyNone();
}

PyResult BeyonceBound::Handle_CmdGotoDirection(PyCallArgs &call) {
  /**
04:45:32 L BeyonceBound: Handle_CmdGotoDirection
04:45:32 [SvcCall]   Call Arguments:
04:45:32 [SvcCall]       Tuple: 3 elements
04:45:32 [SvcCall]         [ 0] Real field: -0.043847
04:45:32 [SvcCall]         [ 1] Real field: 0.860934
04:45:32 [SvcCall]         [ 2] Real field: 0.506824
  call.Dump(SERVICE__CALL_DUMP);
*/

    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return new PyNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return new PyNone();
    }

    Call_PointArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
        return new PyNone();
    }

    call.client->SetInvul(false);
    call.client->SetUndock(false);

    const GPoint dir = GPoint(arg.x, arg.y, arg.z);
    pDestiny->GotoDirection(dir);

    return new PyNone();
}

PyResult BeyonceBound::Handle_CmdGotoBookmark(PyCallArgs &call) {
    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return new PyNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return new PyNone();
    }

    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
        return new PyNone();
    }

	double x = 0.0, y = 0.0, z = 0.0;
    uint32 itemID = 0, typeID = 0, locationID = 0;

    BookmarkService* pBMSvc = (BookmarkService *)(call.client->services().LookupService( "bookmark" ));

    if (!pBMSvc) {
        sLog.Error( "BeyonceService::Handle_GotoBookmark()", "Attempt to access BookmarkService via (BookmarkService *)(call.client->services().LookupService(\"bookmark\")) returned NULL pointer." );
        return new PyNone();
    } else {
        pBMSvc->LookupBookmark(arg.arg, itemID, typeID, locationID, x, y, z);

        if (typeID == 5) {
            if (call.client->GetSystemID() != locationID) {
                //  this bm is for different system.  make and send error here.
                return new PyNone();
            }

            pDestiny->GotoPoint((GPoint)(x,y,z));
        } else {
            // Bookmark type is of a static system entity, so search for it and obtain its coordinates:
            SystemEntity* pSE = call.client->SystemMgr()->GetSE(itemID);
            if (!pSE) {
                sLog.Error( "BeyonceService::Handle_GotoBookmark()", "%s: unable to find location %d", call.client->GetName(), itemID );
                return new PyNone();
            }

            pDestiny->GotoPoint( pSE->GetPosition() );
        }
    }

    call.client->SetInvul(false);
    call.client->SetUndock(false);

    return new PyNone();
}

PyResult BeyonceBound::Handle_CmdOrbit(PyCallArgs &call) {
  /*
            bp.CmdOrbit(id, range)
            */
    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return new PyNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return new PyNone();
    }

    SystemManager* pSystem = call.client->SystemMgr();
    if (!pSystem) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return new PyNone();
    }
  //sLog.White( "BeyonceBound", "Handle_CmdOrbit" );
  call.Dump(SERVICE__CALL_DUMP);
    Call_Orbit args;
    if (!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
        return new PyNone();
    }

    double range =
        args.range->IsInt()
        ? args.range->AsInt()->value()
        : args.range->AsFloat()->value();

    SystemEntity* pEntity = pSystem->GetSE(args.entityID);
    if (!pEntity) {
        _log(CLIENT__ERROR, "%s: Unable to find entity %u to Orbit.", call.client->GetName(), args.entityID);
        return new PyNone();
    }

    call.client->SetInvul(false);
    call.client->SetUndock(false);

    pDestiny->Orbit(pEntity, range);

    return new PyNone();
}

PyResult BeyonceBound::Handle_CmdWarpToStuff(PyCallArgs &call) {
  _log(SERVICE__CALL_DUMP, "BeyonceBound::Handle_CmdWarpToStuff() - size %u", call.tuple->size() );
  call.Dump(SERVICE__CALL_DUMP);

    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return new PyNone();
    } else if (pDestiny->IsWarping()){
        call.client->SendNotifyMsg( "You are already warping");
        return new PyNone();
    }

    SystemManager* pSM = call.client->SystemMgr();
    if (!pSM) {
        codelog(CLIENT__ERROR, "%s: no system manager found", call.client->GetName());
        return new PyNone();
    }

    bool fleet = false;
    if (call.byname.find("fleet") != call.byname.end())
        if (!(call.byname.find("fleet")->second->IsNone()))
            fleet = call.byname.find("fleet")->second->AsBool()->value();

    // get the warp-to distance specified by the client
    int32 distance = 0;
    std::map<std::string, PyRep*>::iterator res = call.byname.find("minRange");
    if (res == call.byname.end()) {
        distance = call.client->GetShip()->radius();
    } else if (!res->second->IsInt() && !res->second->IsFloat()) {
        codelog(CLIENT__ERROR, "%s: range of invalid type %s, expected Integer or Real; using 5 km.", call.client->GetName(), res->second->TypeString());
        distance = 5000;
    } else {
        distance = (res->second->IsInt() ? res->second->AsInt()->value() : res->second->AsFloat()->value());
    }

    GPoint warpToPoint(NULL_ORIGIN);
    SystemEntity* pSE(nullptr);
    double radius = 0;
    uint32 toID = 0;
    std::string stringArg = "";

    if (call.tuple->GetItem(1)->IsString())
        stringArg = call.tuple->GetItem(1)->AsString()->content();
    else if (call.tuple->GetItem(1)->IsInt())
        toID = call.tuple->GetItem(1)->AsInt()->value();

    std::string type = call.tuple->GetItem(0)->AsString()->content();
    if (type == "item" ) {
		pSE = pSM->GetSE(toID);
        if (!pSE) {
            codelog(CLIENT__ERROR, "%s: unable to find location %d", call.client->GetName(), toID);
			return new PyNone();
		}
    } else if (type == "bookmark" ) {
        double x = 0.0, y = 0.0, z = 0.0;
        uint32 typeID = 0, locationID = 0;
        uint32 bookmarkID = call.tuple->GetItem(1)->AsInt()->value();

        BookmarkService* bkSrvc = (BookmarkService *)(call.client->services().LookupService( "bookmark" ));
        if (!bkSrvc) {
            sLog.Error( "BeyonceService::Handle_WarpToStuff()", "Attempt to access BookmarkService returned NULL." );
            return new PyNone();
        }
        bkSrvc->LookupBookmark(bookmarkID, toID, typeID, locationID, x, y, z);

        if ( typeID == 5 ) {
            if (call.client->GetSystemID() != locationID) {
                //  this bm is for different system.  make error here.
                return new PyNone();
            }
            warpToPoint.x = x;
            warpToPoint.y = y;
            warpToPoint.z = z;
        } else {
            // Bookmark type is of a static system entity, so search for it and obtain its coordinates:
            pSE = pSM->GetSE( toID );
            if (!pSE) {
                sLog.Error( "BeyonceService::Handle_WarpToStuff()", "%s: unable to find location %d", call.client->GetName(), toID );
                return new PyNone();
            }
        }
    } else if (type == "scan") {
        ManagerDB mDB;
        warpToPoint = mDB.GetAnomalyPos(stringArg);
    } else if (type == "launch") {
        // launchpickup - launch, launchid
        PlanetDB mDB;
        warpToPoint = mDB.GetLaunchPos(toID);
    }
	// the systems below are not implemented yet.  hold on coding till systems are working and we know what needs to be done here
	// more info can be found in client::menuSvc.py
	else if (type == "epinstance") {
        // epinstance, instanceid
        //stringArg
        call.client->SendErrorMsg("WarpToInstance is not implemented at this time.");
        return new PyNone();
    } else if (type == "tutorial") {
        // tutorial, none
        call.client->SendErrorMsg("WarpToTutorial is not implemented at this time.");
        return new PyNone();
    } else if (type == "char") {
    //  fleet warping
    // [warptomember] char, charid, minrange
    // [warpfleettomember] char, charid, minrange, fleet=1
        call.client->SendErrorMsg("WarpToChar is not implemented at this time.");
        return new PyNone();
    } else {
        sLog.Error( "BeyonceService::Handle_WarpToStuff()", "Unexpected type value: '%s'.", type.c_str() );
        return new PyNone();
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
    if (pSE) {
        radius = pSE->GetRadius();
        warpToPoint = pSE->GetPosition();
        if (pSE->IsPlanetSE()) {
            srandom(toID);  //this is the only place random() is used....other random functions use rand() as it's non-repeatable.
            int64 rand = random();
            double j = (((rand / RAND_MAX) -1.0) / 3.0);
            double s = 20 * pow(0.025 * (10 * log10(radius/1000000) -39), 20) +0.5;
            s = EvE::max(0.5, EvE::min(s, 10.5));
            double t = asin((warpToPoint.x/fabs(warpToPoint.x)) * (warpToPoint.z / sqrt(pow(warpToPoint.x, 2) + pow(warpToPoint.z, 2)))) +j;
            uint32 d = radius * (s +1) +10000;
            warpToPoint.x += d * sin(t);
            warpToPoint.y += 0.5 * radius * sin(j);
            warpToPoint.z -= d * cos(t);
        } else if (pSE->IsStationSE()){
            // this makes ship warp to station dock elevation (y), instead of warping to stations "center point" position (where icon is)
            StationData data;
            sDataMgr.GetStationInfo(toID, data);
            warpToPoint.y = data.dockPosition.y;
        } else if (pSE->IsMoonSE()) {
            distance += 1200;  // hack distance for moons until i get the radius working correctly
        } else if (pSE->IsCOSE()) {
            distance += 1200;  // hack distance for customs offices until i get the radius working correctly
        } else if (pSE->IsGateSE()) {
            distance += (radius /4);  // fudge the distance a bit for gates... its' a lil close by default
        } else if (radius > 90000) {
            /** @todo  this formula is right, but isnt working correctly....revert to my formula
             *   warpToPoint.x += ((radius + 5000000) * cos(radius));
             *   warpToPoint.y += ((radius * 1.3) - 7500);
             *   warpToPoint.z -= ((radius + 5000000) * sin(radius));
             */
            warpToPoint -= (pSE->GetRadius() + (pSE->GetRadius() *2 /8 /*10*/));
        }
        if (radius < 90000) {
            // this will include stations (max station radius 60km)
            GVector vectorFromOrigin( warpToPoint, call.client->GetShipSE()->GetPosition() );
            vectorFromOrigin.normalize();   //we now have a direction
            GPoint stopPoint = (vectorFromOrigin * -radius);
            warpToPoint -= stopPoint;
        }
    }

    call.client->SetInvul(false);
    call.client->SetUndock(false);

    distance += (call.client->GetShipSE()->GetRadius() *2); // add ship diameter x2 to distance
    pDestiny->WarpTo(warpToPoint, distance);

    return new PyNone();
}

PyResult BeyonceBound::Handle_CmdWarpToStuffAutopilot(PyCallArgs &call) {
    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return new PyNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return new PyNone();
    }
    SystemManager* pSM = call.client->SystemMgr();
    if (!pSM) {
        codelog(CLIENT__ERROR, "%s: no system manager found", call.client->GetName());
        return new PyNone();
    }

  //  sends targeted celestial itemID as arg.destID
    //sLog.Warning( "BeyonceBound", "Handle_CmdWarpToStuffAutopilot" );
    CallWarpToStuffAutopilot arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
        return new PyNone();
	}

    //Change this to change the default autopilot distance (Faster Autopilot FTW)
    /** @todo make and set config var for default AP warpTo distance to use here */
    int32 distance = 5000; //15000

    SystemEntity* pSE = pSM->GetSE(arg.destID);
    if (!pSE) {
	  codelog(CLIENT__ERROR, "%s: unable to find destination Entity for ID %u", call.client->GetName(), arg.destID);
        return new PyNone();
    }
    // autopilot check
    //call.client->SetAutoPilot(true);

    call.client->SetInvul(false);
    call.client->SetUndock(false);

	//Adding in ship and target object radius'
    distance += call.client->GetShipSE()->GetRadius() + pSE->GetRadius();
    pDestiny->WarpTo(pSE->GetPosition(), distance);

    return new PyNone();
}

PyResult BeyonceBound::Handle_CmdStop(PyCallArgs &call) {
    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return new PyNone();
    }
    if (!pDestiny->IsMoving())
        return new PyNone();
    if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return new PyNone();
    }

    call.client->SetUndock(false);

    pDestiny->Stop();

    return new PyNone();
}

// CmdTurboDock (in client code)
PyResult BeyonceBound::Handle_CmdDock(PyCallArgs &call) {
    if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change already active.");
        return new PyNone();
    }
    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return new PyNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return new PyNone();
    }
    SystemManager* pSM = call.client->SystemMgr();
    if (!pSM) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager.", call.client->GetName());
        return new PyNone();
    }
    Call_TwoIntegerArgs args;  //sends stationID, shipID
    if (!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
        return new PyNone();
    }

    //  this sets m_dockStationID for radius checks and other thigns
    call.client->SetDockStationID( args.arg1 );

    /* return error msg from this call, if applicable, else nodeid and timestamp */
    return pDestiny->AttemptDockOperation();
}

PyResult BeyonceBound::Handle_CmdStargateJump(PyCallArgs &call) {
    if (call.client->IsSessionChange()) {
        call.client->SendNotifyMsg("Session Change already active.");
        return new PyNone();
    }
    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return new PyNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return new PyNone();
    }

    //sLog.Warning( "BeyonceBound", "Handle_CmdStargateJump" );
    // sends fromGateID, toGateID, and shipID
    Call_StargateJump args;
    if (!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
        return new PyNone();
    }

    /** @todo  check distance from ship to gate */
    call.client->StargateJump(args.fromStargateID, args.toStargateID);

    /* return error msg from this call, if applicable, else nodeid and timestamp */
    return new PyLong(Win32TimeNow());
}

PyResult BeyonceBound::Handle_CmdAbandonLoot(PyCallArgs &call) {
	/*  remotePark.CmdAbandonLoot(wrecks)  <- this is pylist from 'abandonAllWrecks'
	 *  remotePark.CmdAbandonLoot([wreckID]) <- single itemID
	 */
  sLog.White( "BeyonceBound::Handle_CmdAbandonLoot()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

	Call_SingleIntList arg;
	if (!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return new PyNone();
	}

	// this will set corp/ally/faction/owner to '0'
	for (auto cur : arg.ints)
        call.client->SystemMgr()->GetSE(cur)->Abandon();

    return new PyNone();
}

PyResult BeyonceBound::Handle_UpdateStateRequest(PyCallArgs &call) {
    codelog(CLIENT__ERROR, "%s: Client sent UpdateStateRequest! that means we messed up pretty bad.", call.client->GetName());

    //no arguments.

    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return new PyNone();
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return new PyNone();
    }

    pDestiny->SendSetState();

    return new PyNone();
}
