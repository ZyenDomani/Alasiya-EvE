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
#include "cache/ObjCacheService.h"
#include "planet/PlanetDB.h"
#include "ship/BeyonceService.h"
#include "system/DestinyManager.h"
#include "system/BookmarkService.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
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
        if (pClient->IsJump())
            pClient->SetJumpTimers();
        else if (pClient->IsLogin())
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

protected:
    Dispatcher *const m_dispatch;

    void WarpToEntity(SystemEntity* pSE);
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
        if (res == NULL) {
            codelog(SERVICE__ERROR, "Failed to load cache, generating empty contents.");
            res = new PyNone();
        }

        m_manager->cache_service->GiveCache(method_id, &res);
    }

    //now we know its in the cache one way or the other, so build a
    //cached object cached method call result.
    //return(m_manager->cache_service->MakeObjectCachedMethodCallResult(method_id));
    return new PyTuple(0);
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
        return nullptr;
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return nullptr;
    }
    SystemManager* pSystem = call.client->SystemMgr();
    if (!pSystem) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return nullptr;
    }
    Call_FollowBall args;
    if (!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: Failed to decode arguments.", call.client->GetName());
        return nullptr;
    }

    double distance = args.distance->IsInt()
                        ? args.distance->AsInt()->value()
                        : args.distance->AsFloat()->value();

    SystemEntity* pEntity = pSystem->GetSE(args.ballID);
    if (!pEntity) {
        _log(CLIENT__ERROR, "%s: Unable to find entity %u to Follow/Approach.", call.client->GetName(), args.ballID);
        return nullptr;
    }

    //sLog.Warning( "BeyonceBound", "Handle_CmdFollowBall - entity:%s(%u), distance:%f", pEntity->GetName(), pEntity->GetID(), distance);
    if (call.client->IsUndock()) {
        call.client->SetUndock(false);
        if (call.client->IsInvul())
            call.client->SetInvul(false);
    }
    pDestiny->Follow(pEntity, distance);

    return nullptr;
}

PyResult BeyonceBound::Handle_CmdSetSpeedFraction(PyCallArgs &call) {
    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return nullptr;
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return nullptr;
    }

    Call_SingleRealArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
        return nullptr;
    }
    /** @todo  rework this...this is to set speed ONLY...NOT to begin moving.  */
    // client should not legally send anything < 0.1 (except on rare occasion a 0.0 instead of Stop.)
    if ((arg.arg != 0) && (arg.arg < 0.1)) return nullptr;

    //sLog.Warning( "BeyonceBound", "Handle_CmdSetSpeedFraction %.2f", arg.arg );
    if (!call.client->IsUndock()){
        if (pDestiny->IsMoving())
            pDestiny->SetSpeedFraction(arg.arg);
        else
            pDestiny->SetSpeedFraction(arg.arg, true);
    }

    return nullptr;
}

PyResult BeyonceBound::Handle_CmdAlignTo(PyCallArgs &call) {
    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return nullptr;
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return nullptr;
    }
    SystemManager* pSystem = call.client->SystemMgr();
    if (!pSystem) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return nullptr;
    }

    CallAlignTo arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
        return nullptr;
    }

    SystemEntity* pEntity = pSystem->GetSE(arg.entityID);
    if (!pEntity) {
        _log(CLIENT__ERROR, "%s: Unable to find entity %u to AlignTo.", call.client->GetName(), arg.entityID);
        return nullptr;
    }

    //sLog.Warning( "BeyonceBound", "Handle_CmdAlignTo - entity:%s(%u)", pEntity->GetName(), pEntity->GetID() );
    if (call.client->IsUndock()) {
        call.client->SetUndock(false);
        if (call.client->IsInvul())
            call.client->SetInvul(false);
    }
    pDestiny->AlignTo( pEntity );

    return nullptr;
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
        return nullptr;
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return nullptr;
    }

    Call_PointArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
        return nullptr;
    }

    if (call.client->IsUndock()) {
        call.client->SetUndock(false);
        if (call.client->IsInvul())
            call.client->SetInvul(false);
    }
   //sLog.Log( "BeyonceBound", "Handle_CmdGotoDirection %.3f, %.3f, %.3f", arg.x, arg.y, arg.z);
    const GPoint dir = GPoint(arg.x, arg.y, arg.z);
    pDestiny->GotoDirection(dir);

    return nullptr;
}

PyResult BeyonceBound::Handle_CmdGotoBookmark(PyCallArgs &call) {
    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return nullptr;
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return nullptr;
    }

    Call_SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
        return nullptr;
    }

	double x = 0.0, y = 0.0, z = 0.0;
    uint32 itemID = 0, typeID = 0, locationID = 0;

    BookmarkService* pBMSvc = (BookmarkService *)(call.client->services().LookupService( "bookmark" ));

    if (!pBMSvc) {
        sLog.Error( "BeyonceService::Handle_GotoBookmark()", "Attempt to access BookmarkService via (BookmarkService *)(call.client->services().LookupService(\"bookmark\")) returned NULL pointer." );
        return nullptr;
    } else {
        pBMSvc->LookupBookmark(arg.arg, itemID, typeID, locationID, x, y, z);

        if (typeID == 5) {
            if (call.client->GetSystemID() != locationID) {
                //  this bm is for different system.  make and send error here.
                return nullptr;
            }

            if (call.client->IsUndock()) call.client->SetUndock(false);
            //if (call.client->IsInvul()) call.client->SetInvul(false);
            pDestiny->GotoPoint((GPoint)(x,y,z));
        } else {
            // Bookmark type is of a static system entity, so search for it and obtain its coordinates:
            SystemEntity* pSE = call.client->SystemMgr()->GetSE(itemID);
            if (!pSE) {
                sLog.Error( "BeyonceService::Handle_GotoBookmark()", "%s: unable to find location %d", call.client->GetName(), itemID );
                return nullptr;
            }

            if (call.client->IsUndock()) {
                call.client->SetUndock(false);
                if (call.client->IsInvul())
                    call.client->SetInvul(false);
            }
            pDestiny->GotoPoint( pSE->GetPosition() );
        }
    }

    return nullptr;
}

PyResult BeyonceBound::Handle_CmdOrbit(PyCallArgs &call) {
  /*
            bp.CmdOrbit(id, range)
            */
    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return nullptr;
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return nullptr;
    }

    SystemManager* pSystem = call.client->SystemMgr();
    if (!pSystem) {
        codelog(CLIENT__ERROR, "%s: Client has no system manager!", call.client->GetName());
        return nullptr;
    }
  //sLog.Log( "BeyonceBound", "Handle_CmdOrbit" );
  call.Dump(SERVICE__CALL_DUMP);
    Call_Orbit args;
    if (!args.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
        return nullptr;
    }

    double range =
        args.range->IsInt()
        ? args.range->AsInt()->value()
        : args.range->AsFloat()->value();

    SystemEntity* pEntity = pSystem->GetSE(args.entityID);
    if (!pEntity) {
        _log(CLIENT__ERROR, "%s: Unable to find entity %u to Orbit.", call.client->GetName(), args.entityID);
        return nullptr;
    }

    //sLog.Log( "BeyonceBound", "Handle_CmdOrbit - entity:%s(%u), range:%f", pEntity->GetName(), pEntity->GetID(), range);
    if (call.client->IsUndock()) {
        call.client->SetUndock(false);
        if (call.client->IsInvul())
            call.client->SetInvul(false);
    }
    pDestiny->Orbit(pEntity, range);
    return nullptr;
}

PyResult BeyonceBound::Handle_CmdWarpToStuff(PyCallArgs &call) {
  _log(SERVICE__CALL_DUMP, "BeyonceBound::Handle_CmdWarpToStuff() - size %u", call.tuple->size() );
  call.Dump(SERVICE__CALL_DUMP);

    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return nullptr;
    } else if (pDestiny->IsWarping()){
        call.client->SendNotifyMsg( "You are already warping");
        return nullptr;
    }

    SystemManager* pSM = call.client->SystemMgr();
    if (!pSM) {
        codelog(CLIENT__ERROR, "%s: no system manager found", call.client->GetName());
        return nullptr;
    }

    bool fleet = false;
    if (call.byname.find("fleet") != call.byname.end()) {
        if ( !(call.byname.find("fleet")->second->IsNone()) ) {
            fleet = call.byname.find("fleet")->second->AsBool()->value();
        }
    }

    // get the warp-to distance specified by the client
    int32 distance = 0;
    std::map<std::string, PyRep*>::iterator res = call.byname.find("minRange");
    if (res == call.byname.end()) {
        ;
    } else if (!res->second->IsInt() && !res->second->IsFloat()) {
        codelog(CLIENT__ERROR, "%s: range of invalid type %s, expected Integer or Real; using 5 km.", call.client->GetName(), res->second->TypeString());
        distance = 5000;
    } else {
        distance =
            res->second->IsInt()
                ? res->second->AsInt()->value()
                : res->second->AsFloat()->value();
    }

    std::string type = call.tuple->GetItem(0)->AsString()->content();

    if (type == "item" ) {
        uint32 toID = call.tuple->GetItem(1)->AsInt()->value();

		// This section handles Warping to any object in the Overview
		SystemEntity* pSE = pSM->GetSE(toID);
        if (!pSE) {
            codelog(CLIENT__ERROR, "%s: unable to find location %d", call.client->GetName(), toID);
			return nullptr;
		}

        double distanceFromBodyOrigin = 0.0, distanceFromSystemOrigin = 0.0;
        float warpPointAdj = 1.0f;
        GPoint warpToPoint(pSE->GetPosition());

        if (pSE->IsStaticEntity()) {
            switch(pSE->GetGroupID() ) {
                case EVEDB::invGroups::Sun:
				case EVEDB::invGroups::Planet: {
                    // Calculate final distance out from origin of celestial body along common warp-to vector:
                    distanceFromBodyOrigin = pSE->GetRadius();            // Add celestial body's radius
                    distanceFromBodyOrigin += 1000000;
                    // Calculate final warp-to point along common vector from celestial body's origin and add randomized position adjustment for multiple ships coming out of warp to not bump
                    GPoint celestialOrigin(pSE->GetPosition());                            // Make a celestial body origin point variable
                    GVector vectorFromOrigin(celestialOrigin, NULL_ORIGIN);                    // Make a celestial body TO system origin origin vector variable
                    if ( vectorFromOrigin.length() == 0 ) {
                        // This is the special case where we are warping to the Star, so we have to construct
                        // a vector from the star's center (0,0,0) to the warp-in point using the distanceFromBodyOrigin
                        // calculated earlier:
                        vectorFromOrigin = GVector( celestialOrigin, call.client->GetShipSE()->GetPosition() );
                        vectorFromOrigin.normalize();
                        vectorFromOrigin *= distanceFromBodyOrigin;
                    }
                    GVector vectorToWarpPoint(vectorFromOrigin);                        // Make a vector to the Warp-In point
                    distanceFromSystemOrigin = vectorFromOrigin.length();                // Calculate distance from system origin to celestial body origin

                    // Calculate warp-in point to provide different juxtapositioning of celestial body to the solar system origin, i.e, the sun
                    // This also provides a common warp-in point for the sun itself, which is the first case in this if-else if-else clause:
                    if (distanceFromSystemOrigin < (5.0 * ONE_AU_IN_METERS)) {
                        GVector rotationVector( 1.0, 1.0, 0.25 );
                        vectorToWarpPoint.rotationTo( rotationVector );
                    } else if (distanceFromSystemOrigin < (15.0 * ONE_AU_IN_METERS)) {
                        GVector rotationVector( -1.0, -1.0, 0.25 );
                        vectorToWarpPoint.rotationTo( rotationVector );
                    } else if (distanceFromSystemOrigin < (25.0 * ONE_AU_IN_METERS)) {
                        GVector rotationVector( 1.0, -1.0, -0.25 );
                        vectorToWarpPoint.rotationTo( rotationVector );
                    } else if (distanceFromSystemOrigin < (35.0 * ONE_AU_IN_METERS)) {
                        GVector rotationVector( -1.0, -1.0, -0.25 );
                        vectorToWarpPoint.rotationTo( rotationVector );
                    } else {
                        GVector rotationVector( -1.0, 1.0, -0.25 );
                        vectorToWarpPoint.rotationTo( rotationVector );
                    }
                    vectorToWarpPoint.normalize();
                    warpToPoint += vectorToWarpPoint * distanceFromBodyOrigin;

                    // Randomize warp-in point:
                    warpToPoint.MakeRandomPointOnSphereLayer(1000.0,(1000.0+call.client->GetShipSE()->GetRadius()));
                } break;
                case EVEDB::invGroups::Moon: {  //this will put ship at same visual distance for different moon radii
                    warpPointAdj = pSE->GetRadius() + (pSE->GetRadius() *2 /5 /*10*/);
                } break;
                default: {
                    warpPointAdj = pSE->GetRadius();
                } break;
            }
        } else
            warpPointAdj = pSE->GetRadius();

        distance += (call.client->GetShipSE()->GetRadius() *2);

        /* client stops warp at (targetpoint - stopdistance) along common line between the two.
         *  the server will need to be told what and how to match the client.
         *  i am doing that here.
         * set targetpoint = point of object minus radius as distance along common vector
         */
        GVector vectorFromOrigin( warpToPoint, call.client->GetShipSE()->GetPosition() );
        vectorFromOrigin.normalize();   //we now have a direction
        GPoint stopPoint = vectorFromOrigin * -warpPointAdj;
        warpToPoint -= stopPoint;
        pDestiny->WarpTo(warpToPoint, distance);

        if (call.client->IsUndock()) {
            call.client->SetUndock(false);
            if (call.client->IsInvul())
                call.client->SetInvul(false);
        }
    } else if (type == "bookmark" ) {
        // This section handles Warping to any Bookmark
        GPoint warpToPoint(NULL_ORIGIN);
        double x = 0.0, y = 0.0, z = 0.0;
        uint32 itemID = 0, typeID = 0, locationID = 0;
        uint32 bookmarkID = call.tuple->GetItem(1)->AsInt()->value();

        BookmarkService* bkSrvc = (BookmarkService *)(call.client->services().LookupService( "bookmark" ));
        if (!bkSrvc) {
            sLog.Error( "BeyonceService::Handle_WarpToStuff()", "Attempt to access BookmarkService returned nullptr." );
            return nullptr;
        }
        bkSrvc->LookupBookmark(bookmarkID, itemID, typeID, locationID, x, y, z);

        if ( typeID == 5 ) {
            if (call.client->GetSystemID() != locationID) {
                //  this bm is for different system.  make and send error here.
                return nullptr;
            }
            warpToPoint = (GPoint)(x, y, z);
        } else {
            // Bookmark type is of a static system entity, so search for it and obtain its coordinates:
            SystemEntity* pSE = pSM->GetSE( itemID );
            if (!pSE) {
                sLog.Error( "BeyonceService::Handle_WarpToStuff()", "%s: unable to find location %d", call.client->GetName(), itemID );
                return nullptr;
            }
            double distanceFromBodyOrigin = 0.0, distanceFromSystemOrigin = 0.0;
            GPoint warpToPoint(pSE->GetPosition());
            float warpPointAdj = 1.0f;
            if (pSE->IsStaticEntity()) {
                switch(pSE->GetGroupID() ) {
                    case EVEDB::invGroups::Sun:
                    case EVEDB::invGroups::Planet: {
                        // Calculate final distance out from origin of celestial body along common warp-to vector:
                        distanceFromBodyOrigin = pSE->GetRadius();            // Add celestial body's radius
                        distanceFromBodyOrigin += 1000000;
                        // Calculate final warp-to point along common vector from celestial body's origin and add randomized position adjustment for multiple ships coming out of warp to not bump
                        GPoint celestialOrigin(pSE->GetPosition());                            // Make a celestial body origin point variable
                        GVector vectorFromOrigin(celestialOrigin, NULL_ORIGIN);                    // Make a celestial body TO system origin origin vector variable
                        if ( vectorFromOrigin.length() == 0 ) {
                            // This is the special case where we are warping to the Star, so we have to construct
                            // a vector from the star's center (0,0,0) to the warp-in point using the distanceFromBodyOrigin
                            // calculated earlier:
                            vectorFromOrigin = GVector( celestialOrigin, call.client->GetShipSE()->GetPosition() );
                            vectorFromOrigin.normalize();
                            vectorFromOrigin *= distanceFromBodyOrigin;
                        }
                        GVector vectorToWarpPoint(vectorFromOrigin);                        // Make a vector to the Warp-In point
                        distanceFromSystemOrigin = vectorFromOrigin.length();                // Calculate distance from system origin to celestial body origin

                        // Calculate warp-in point to provide different juxtapositioning of celestial body to the solar system origin, i.e, the sun
                        // This also provides a common warp-in point for the sun itself, which is the first case in this if-else if-else clause:
                        if (distanceFromSystemOrigin < (5.0 * ONE_AU_IN_METERS)) {
                            GVector rotationVector( 1.0, 1.0, 0.25 );
                            vectorToWarpPoint.rotationTo( rotationVector );
                        } else if (distanceFromSystemOrigin < (15.0 * ONE_AU_IN_METERS)) {
                            GVector rotationVector( -1.0, -1.0, 0.25 );
                            vectorToWarpPoint.rotationTo( rotationVector );
                        } else if (distanceFromSystemOrigin < (25.0 * ONE_AU_IN_METERS)) {
                            GVector rotationVector( 1.0, -1.0, -0.25 );
                            vectorToWarpPoint.rotationTo( rotationVector );
                        } else if (distanceFromSystemOrigin < (35.0 * ONE_AU_IN_METERS)) {
                            GVector rotationVector( -1.0, -1.0, -0.25 );
                            vectorToWarpPoint.rotationTo( rotationVector );
                        } else {
                            GVector rotationVector( -1.0, 1.0, -0.25 );
                            vectorToWarpPoint.rotationTo( rotationVector );
                        }
                        vectorToWarpPoint.normalize();
                        warpToPoint += vectorToWarpPoint * distanceFromBodyOrigin;

                        // Randomize warp-in point:
                        warpToPoint.MakeRandomPointOnSphereLayer(1000.0,(1000.0+call.client->GetShipSE()->GetRadius()));
                    } break;
                    case EVEDB::invGroups::Moon: {  //this will put ship at same visual distance for different moon radii
                        warpPointAdj = pSE->GetRadius() + (pSE->GetRadius() *2 /5 /*10*/);
                    } break;
                    default: {
                        warpPointAdj = pSE->GetRadius();
                    } break;
                }
            } else
                warpPointAdj = pSE->GetRadius();

            distance += (call.client->GetShipSE()->GetRadius() *2);

            /* client stops warp at (targetpoint - stopdistance) along common line between the two.
             *  the server will need to be told what and how to match the client.
             *  i am doing that here.
             * set targetpoint = point of object minus radius as distance along common vector
             */
            GVector vectorFromOrigin( warpToPoint, call.client->GetShipSE()->GetPosition() );
            vectorFromOrigin.normalize();   //we now have a direction
            GPoint stopPoint = vectorFromOrigin * -warpPointAdj;
            warpToPoint -= stopPoint;
        }
        pDestiny->WarpTo(warpToPoint, distance);

        if (call.client->IsUndock()) {
            call.client->SetUndock(false);
            if (call.client->IsInvul())
                call.client->SetInvul(false);
        }
    } else if (type == "scan") {
        std::string resultID = call.tuple->GetItem(1)->AsString()->content();
        ManagerDB mDB;
        pDestiny->WarpTo(mDB.GetAnomalyPos(resultID), distance);
    } else if (type == "launch") {
        // launchpickup - launch, launchid
        uint32 launchid = call.tuple->GetItem(1)->AsInt()->value();
        PlanetDB mDB;
        pDestiny->WarpTo(mDB.GetLaunchPos(launchid), distance);
    }
	// the systems below are not implemented yet.  hold on coding till systems are working.
	else if (type == "epinstance") {
        // epinstance, instanceid
        std::string instanceID = call.tuple->GetItem(1)->AsString()->content();
        call.client->SendErrorMsg("WarpToInstance is not implemented at this time.  See Allan for updates.");
    } else if (type == "tutorial") {
        // tutorial, none
        call.client->SendErrorMsg("WarpToTutorial is not implemented at this time.  See Allan for updates.");
    } else if (type == "char") {
    //  fleet warping
    // [warptomember] char, charid, minrange
    // [warpfleettomember] char, charid, minrange, fleet=1
        uint32 toID = call.tuple->GetItem(1)->AsInt()->value();
        call.client->SendErrorMsg("WarpToChar is not implemented at this time.  See Allan for updates.");
    } else {
        sLog.Error( "BeyonceService::Handle_WarpToStuff()", "Unexpected type value: '%s'.", type.c_str() );
    }

    return nullptr;
}

void BeyonceBound::WarpToEntity(SystemEntity* pSE)
{

}

PyResult BeyonceBound::Handle_CmdWarpToStuffAutopilot(PyCallArgs &call) {
    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return nullptr;
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return nullptr;
    }
    SystemManager* pSM = call.client->SystemMgr();
    if (!pSM) {
        codelog(CLIENT__ERROR, "%s: no system manager found", call.client->GetName());
        return nullptr;
    }

  //  sends targeted celestial itemID as arg.destID
    //sLog.Warning( "BeyonceBound", "Handle_CmdWarpToStuffAutopilot" );
    CallWarpToStuffAutopilot arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
        return nullptr;
	}

    //Change this to change the default autopilot distance (Faster Autopilot FTW)
    /** @todo make and set config var for default AP warpTo distance to use here */
    int32 distance = 5000; //15000

    SystemEntity* pSE = pSM->GetSE(arg.destID);
    if (!pSE) {
	  codelog(CLIENT__ERROR, "%s: unable to find destination Entity for ID %u", call.client->GetName(), arg.destID);
        return nullptr;
    }
    // autopilot check
    //call.client->SetAutoPilot(true);

    if (call.client->IsUndock()) {
        call.client->SetUndock(false);
        if (call.client->IsInvul())
            call.client->SetInvul(false);
    }
	//Adding in ship and target object radius'
    distance += call.client->GetShipSE()->GetRadius() + pSE->GetRadius();
    pDestiny->WarpTo(pSE->GetPosition(), distance);

    return nullptr;
}

PyResult BeyonceBound::Handle_CmdStop(PyCallArgs &call) {
    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return nullptr;
    }
    if (!pDestiny->IsMoving()) return nullptr;
    if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return nullptr;
    }

    if (call.client->IsUndock())
        call.client->SetUndock(false);

    //sLog.Warning( "BeyonceBound", "Handle_CmdStop" );
    pDestiny->Stop();

    return nullptr;
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
        return nullptr;
    }

    /** @todo  check distance from ship to gate */

    if (call.client->IsUndock()) {
        call.client->SetUndock(false);
        if (call.client->IsInvul())
            call.client->SetInvul(false);
    }
    call.client->StargateJump(args.fromStargateID, args.toStargateID);
    return new PyNone();
}

PyResult BeyonceBound::Handle_CmdAbandonLoot(PyCallArgs &call) {
	/*  remotePark.CmdAbandonLoot(wrecks)  <- this is pylist from 'abandonAllWrecks'
	 *  remotePark.CmdAbandonLoot([wreckID]) <- single itemID
	 */
  sLog.Log( "BeyonceBound::Handle_CmdAbandonLoot()", "size= %u", call.tuple->size() );
    call.Dump(SERVICE__CALL_DUMP);

	Call_SingleIntList arg;
	if (!arg.Decode(&call.tuple)) {
		codelog(CLIENT__ERROR, "%s: failed to decode args", call.client->GetName());
		return nullptr;
	}
	//arg.ints is list sent by client

    return nullptr;
}

PyResult BeyonceBound::Handle_UpdateStateRequest(PyCallArgs &call) {
    codelog(CLIENT__ERROR, "%s: Client sent UpdateStateRequest! that means we messed up pretty bad.", call.client->GetName());

    //no arguments.

    DestinyManager* pDestiny = call.client->GetShipSE()->DestinyMgr();
    if (!pDestiny) {
        codelog(CLIENT__ERROR, "%s: Client has no destiny manager!", call.client->GetName());
        return nullptr;
    } else if (pDestiny->IsWarping()) {
        call.client->SendNotifyMsg( "You can't do this while warping");
        return nullptr;
    }

    pDestiny->SendSetState();

    return nullptr;
}
