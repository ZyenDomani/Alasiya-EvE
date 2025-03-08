/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
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
    Author:        caytchen
*/

#ifndef __NOTIFICATIONMGRSERVICE__H__INCL__
#define __NOTIFICATIONMGRSERVICE__H__INCL__

#include "PyService.h"

class NotificationMgrService : public PyService {
public:
    NotificationMgrService(PyServiceMgr* mgr);
    virtual ~NotificationMgrService();

private:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    PyCallable_DECL_CALL(GetByGroupID);
    PyCallable_DECL_CALL(GetUnprocessed);
    PyCallable_DECL_CALL(MarkGroupAsProcessed);
    PyCallable_DECL_CALL(MarkAllAsProcessed);
    PyCallable_DECL_CALL(MarkAsProcessed);
    PyCallable_DECL_CALL(DeleteGroupNotifications);
    PyCallable_DECL_CALL(DeleteAllNotifications);
    PyCallable_DECL_CALL(DeleteNotifications);
};

#endif


/*
 * (236000, `Insurance Contract Issued`)
 * (236001, `Report: Starbase low on resources in {[location]solarSystemID.name}`)
 * (236003, `Report: "{[item]typeID.name, linkify}" at "{[location]stationID.name, linkify}" has been disabled`)
 * (236006, `{[character]newCeoID.name} is the new CEO of {corporationName}`)
 * (236007, `Welcome to {corporationName}`)
 * (236008, `Rejected application to join {corporationName}`)
 * (236009, `Report: "{[item]typeID.name, linkify}" at "{[location]stationID.name, linkify}" has been reenabled`)
 * (236014, `Report: Infrastructure hub %22{name}%22 has been conquered`)
 * (236018, `Report: Starbase in {[location]solarSystemID.name, linkify} is under attack`)
 * (236019, `Report: Station '{[location]stationID.name}' has been conquered`)
 */
