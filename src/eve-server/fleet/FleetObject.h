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
    Author:        Allan
*/

#ifndef EVEMU_SHIP_FLEETOBJ_H_
#define EVEMU_SHIP_FLEETOBJ_H_

#include "PyService.h"
#include "fleet/FleetManager.h"

class FleetObject
 : public PyService
{
public:
    FleetObject(PyServiceMgr *mgr);
    ~FleetObject();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(CreateFleet);
    PyCallable_DECL_CALL(CreateWing);
    PyCallable_DECL_CALL(CreateSquad);
    PyCallable_DECL_CALL(SetMotdEx);
    PyCallable_DECL_CALL(GetMotd);
    PyCallable_DECL_CALL(CheckIsInFleet);
    PyCallable_DECL_CALL(MakeLeader);
    PyCallable_DECL_CALL(SetBooster);
    PyCallable_DECL_CALL(MoveMember);
    PyCallable_DECL_CALL(KickMember);
    PyCallable_DECL_CALL(DeleteWing);
    PyCallable_DECL_CALL(DeleteSquad);
    PyCallable_DECL_CALL(LeaveFleet);

    //overloaded in order to support bound objects:
    virtual PyBoundObject *_CreateBoundObject(Client *c, const PyRep *bind_args);

private:
    FleetService m_Svc;
};

#endif  // EVEMU_SHIP_FLEETOBJ_H_

/*1
 * 02:51:15 [SvcCall] Service fleetObjectHandler::CreateFleet()
 * 02:51:15 [SvcCall] Service alert::BeanCount()
 * 02:51:15 [SvcCall] Service alert::BeanCount()
 * 02:51:15 [SvcCall] Service config::GetDynamicCelestials()
 * 02:51:15 [SvcCall] Service alert::SendClientStackTraceAlert()
 * EXCEPTION #17 logged at  03/01/2017 2:51:15 Unhandled exception in <TaskletExt object at 10669b30, abps=1001, ctxt='<NO CONTEXT>^<function <lambda> at (snip)>'>
 * Caught at:
 * /common/lib/bluepy.py(98) CallWrapper
 * Thrown at:
 * /common/lib/bluepy.py(86) CallWrapper
 * /../carbon/client/script/ui/control/menu.py(517) <lambda>
 * /client/script/ui/services/menusvc.py(5236) InviteToFleet
 * /client/script/parklife/fleetsvc.py(339) Invite
 * /client/script/parklife/fleetsvc.py(331) CreateFleet
 * /../carbon/common/script/net/moniker.py(422) __call__
 * /../carbon/common/script/net/moniker.py(248) MonikeredCall
 * /../carbon/common/script/net/moniker.py(214) Bind
 * /../carbon/common/script/net/servicecallgpcs.py(755) callable
 * /../carbon/common/script/net/machonet.py(195) ThrottledCall
 * /../carbon/common/script/net/servicecallgpcs.py(746) doCall
 * /../carbon/common/script/net/servicecallgpcs.py(292) RemoteServiceCallWithoutTheStars
 * /../carbon/common/script/net/objectcallgpcs.py(216) CallDown
 * /../carbon/common/script/net/exceptionmappinggpcs.py(81) CallDown
 * /../carbon/common/script/net/exceptionmappinggpcs.py(58) _ProcessCall
 * /../carbon/common/script/net/exceptionwrappergpcs.py(103) CallDown
 * /../carbon/common/script/net/machonet.py(3863) _BlockingCall
 * /common/lib/bluepy.py(396) Wrapper
 * /../carbon/common/script/net/machonettransport.py(364) Write
 * /../carbon/common/script/net/machonetpacket.py(172) GetPickle
 * /../carbon/common/script/net/machonet.py(128) MachoDumps
 *        packet = Packet containing crappy data
 * UnpickleableError: Cannot pickle <type 'weakproxy'> objects
 * Thread Locals:  session was <Session: (sid:2994889200020716882, clientID:0, mutating:0, locationid:60014137, corprole:0x0, userid:2, languageID:EN, role:0x63f8000280c40000L, charid:140000000, address:192.168.3.8:59506, userType:30, sessionType:5, regionid:10000001, constellationid:20000008, corpid:1000172, fleetid:0, fleetrole:1, fleetbooster:1, wingid:11, squadid:12, stationid:60014137, stationid2:60014137, worldspaceid:60014137, solarsystemid2:30000053, hqID:60014809, rolesAtAll:0x0, rolesAtHQ:0x0, rolesAtBase:0x0, rolesAtOther:0x0, genderID:0, bloodlineID:14, raceID:2, corpAccountKey:1000)>
 *
 * EXCEPTION END
 *
 * 02:51:15 [SvcCall] Service alert::SendClientStackTraceAlert()
 * EXCEPTION #18 logged at  03/01/2017 2:51:15
 * Caught at:
 * /common/lib/bluepy.py(86) CallWrapper
 * /../carbon/common/script/sys/servicemanager.py(780) MollycoddledUthread
 * Thrown at:
 * /../carbon/common/script/sys/servicemanager.py(777) MollycoddledUthread
 * /client/script/parklife/fleetsvc.py(1482) OnJoinedFleet
 * /client/script/parklife/fleetsvc.py(1560) RefreshFleetWindow
 * /client/script/parklife/fleetsvc.py(274) InitFleet
 * /../carbon/common/script/net/moniker.py(422) __call__
 * /../carbon/common/script/net/moniker.py(248) MonikeredCall
 * /../carbon/common/script/net/moniker.py(214) Bind
 * /../carbon/common/script/net/servicecallgpcs.py(755) callable
 * /../carbon/common/script/net/machonet.py(195) ThrottledCall
 * /../carbon/common/script/net/servicecallgpcs.py(746) doCall
 * /../carbon/common/script/net/servicecallgpcs.py(292) RemoteServiceCallWithoutTheStars
 * /../carbon/common/script/net/objectcallgpcs.py(216) CallDown
 * /../carbon/common/script/net/exceptionmappinggpcs.py(81) CallDown
 * /../carbon/common/script/net/exceptionmappinggpcs.py(58) _ProcessCall
 * /../carbon/common/script/net/exceptionwrappergpcs.py(103) CallDown
 * /../carbon/common/script/net/machonet.py(3863) _BlockingCall
 * /common/lib/bluepy.py(396) Wrapper
 * /../carbon/common/script/net/machonettransport.py(364) Write
 * /../carbon/common/script/net/machonetpacket.py(172) GetPickle
 * /../carbon/common/script/net/machonet.py(128) MachoDumps
 *        packet = Packet containing crappy data
 * UnpickleableError: Cannot pickle <type 'weakproxy'> objects
 * Thread Locals:  session was <Session: (sid:2994889200020716882, clientID:0, mutating:0, locationid:60014137, corprole:0x0, userid:2, languageID:EN, role:0x63f8000280c40000L, charid:140000000, address:192.168.3.8:59506, userType:30, sessionType:5, regionid:10000001, constellationid:20000008, corpid:1000172, fleetid:0, fleetrole:1, fleetbooster:1, wingid:11, squadid:12, stationid:60014137, stationid2:60014137, worldspaceid:60014137, solarsystemid2:30000053, hqID:60014809, rolesAtAll:0x0, rolesAtHQ:0x0, rolesAtBase:0x0, rolesAtOther:0x0, genderID:0, bloodlineID:14, raceID:2, corpAccountKey:1000)>
 *
 * EXCEPTION END
 *
 *
 */