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

#ifndef __DUNGEON_SERVICE_H_INCL__
#define __DUNGEON_SERVICE_H_INCL__

#include "system/SystemDB.h"
#include "PyService.h"

class DungeonService
: public PyService
{
public:
    DungeonService(PyServiceMgr *mgr);
    virtual ~DungeonService();

protected:
    class Dispatcher;
    Dispatcher *const m_dispatch;

    SystemDB m_db;

    PyCallable_DECL_CALL(DEGetFactions);
    PyCallable_DECL_CALL(DEGetDungeons);
    PyCallable_DECL_CALL(DEGetTemplates);
    PyCallable_DECL_CALL(DEGetRooms);
    PyCallable_DECL_CALL(DEGetRoomObjectPaletteData);
    PyCallable_DECL_CALL(TemplateRemove);
    PyCallable_DECL_CALL(GetArchetypes);
    PyCallable_DECL_CALL(RemoveObject);
    PyCallable_DECL_CALL(EditObjectName);
    PyCallable_DECL_CALL(CopyObject);
    PyCallable_DECL_CALL(EditObject);
    PyCallable_DECL_CALL(EditObjectRadius);
    PyCallable_DECL_CALL(EditObjectXYZ);
    PyCallable_DECL_CALL(EditObjectYawPitchRoll);
    PyCallable_DECL_CALL(IsObjectLocked);

//newObjectID = sm.RemoteSvc('dungeon').(objectID, roomID, offsetX, offsetY, offsetZ)

    //overloaded in order to support bound objects:
    //virtual PyBoundObject *_CreateBoundObject(Client *c, const PyRep *bind_args);
};

/*
return sm.RemoteSvc('dungeon').(objectID)
sm.RemoteSvc('dungeon').(objectID=objectID, x=x, y=y, z=z)
sm.RemoteSvc('dungeon').(objectID=objectID, yaw=yaw, pitch=pitch, roll=roll)
sm.RemoteSvc('dungeon').(objectID=objectID, radius=radius)
(newObjectID, revisionID,) = sm.RemoteSvc('dungeon').AddObject(roomID, typeID, x, y, z, yaw, pitch, roll, radius)
sm.RemoteSvc('dungeon').(newObjectID, objectName)
sm.RemoteSvc('dungeon').(objectID)
dungeons = sm.RemoteSvc('dungeon').DEGetDungeons(archetypeID=archetypeID, factionID=factionID)
archetypes = sm.RemoteSvc('dungeon').()
factions = sm.RemoteSvc('dungeon').DEGetFactions()
seldungeon = sm.RemoteSvc('dungeon').DEGetDungeons(dungeonID=dungeonID)[0]
rooms = sm.RemoteSvc('dungeon').DEGetRooms(dungeonID=seldungeon.dungeonID)
self.templateRows = sm.RemoteSvc('dungeon').DEGetTemplates()
roomObjectGroups = sm.RemoteSvc('dungeon').DEGetRoomObjectPaletteData()
objectIDs = sm.RemoteSvc('dungeon').AddTemplateObjects(roomID, self.sr.node.id, (posInRoom.x, posInRoom.y, posInRoom.z))
sm.RemoteSvc('dungeon').(self.sr.node.id)
selDungeon = sm.RemoteSvc('dungeon').DEGetDungeons(dungeonID=dungeonID)[0]
roomObjectGroups = sm.RemoteSvc('dungeon').()
dungeonSvc = sm.RemoteSvc('dungeon')
dungeonSvc = sm.RemoteSvc('dungeon')
*/



#endif


