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
    Author:        Allan
*/

#include "eve-server.h"

#include "PyServiceCD.h"
#include "system/ScenarioService.h"


PyCallable_Make_InnerDispatcher(ScenarioService)

ScenarioService::ScenarioService(PyServiceMgr *mgr)
: PyService(mgr, "scenario"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(ScenarioService, ResetD);
    PyCallable_REG_CALL(ScenarioService, PlayDungeon);
    PyCallable_REG_CALL(ScenarioService, EditRoom);
    PyCallable_REG_CALL(ScenarioService, SaveAllChanges);
    PyCallable_REG_CALL(ScenarioService, GotoRoom);
    PyCallable_REG_CALL(ScenarioService, GetDunObjects);
    PyCallable_REG_CALL(ScenarioService, GetSelObjects);
    PyCallable_REG_CALL(ScenarioService, IsSelectedByObjID);
    PyCallable_REG_CALL(ScenarioService, DuplicateSelection);
    PyCallable_REG_CALL(ScenarioService, SetSelectionByID);
    PyCallable_REG_CALL(ScenarioService, SetSelectedRadius);
    PyCallable_REG_CALL(ScenarioService, SetRotate);
    PyCallable_REG_CALL(ScenarioService, DeleteObjects);
    PyCallable_REG_CALL(ScenarioService, RotateSelected);
    PyCallable_REG_CALL(ScenarioService, JitterSelection);
    PyCallable_REG_CALL(ScenarioService, JitterRotationSelection);
    PyCallable_REG_CALL(ScenarioService, ArrangeSelection);
    PyCallable_REG_CALL(ScenarioService, DeleteSelected);
    PyCallable_REG_CALL(ScenarioService, RefreshSelection);
    PyCallable_REG_CALL(ScenarioService, AddHardGroup);
    PyCallable_REG_CALL(ScenarioService, RenameHardGroup);
    PyCallable_REG_CALL(ScenarioService, RemoveAllHardGroups);
    PyCallable_REG_CALL(ScenarioService, AreAllSelected);
    PyCallable_REG_CALL(ScenarioService, SetActiveHardGroup);
    PyCallable_REG_CALL(ScenarioService, SetSelectedQuantity);
    PyCallable_REG_CALL(ScenarioService, GetEditingRoomID);
    PyCallable_REG_CALL(ScenarioService, GetEditingRoomPosition);
    PyCallable_REG_CALL(ScenarioService, WaitForObjectCreationByID);
    PyCallable_REG_CALL(ScenarioService, GetBallAndSlimItemFromObjectID);
}
//        remoteSelection = copy.copy(sm.StartService('scenario').selectionObjs)
//        self.scenario.ClearSelection()


ScenarioService::~ScenarioService() {
    delete m_dispatch;
}


PyResult ScenarioService::Handle_WaitForObjectCreationByID( PyCallArgs& call )
{
    //   scenario.(objectIDs)

    _log(DUNG__CALL, "ScenarioService::WaitForObjectCreationByID()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_GetBallAndSlimItemFromObjectID( PyCallArgs& call )
{
    //  ball, slimItem = scenarioSvc.GetBallAndSlimItemFromObjectID(objectID)


    _log(DUNG__CALL, "ScenarioService::GetBallAndSlimItemFromObjectID()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_GetEditingRoomPosition( PyCallArgs& call )
{
    //     roomPos = scenarioSvc.GetEditingRoomPosition()

    _log(DUNG__CALL, "ScenarioService::GetEditingRoomPosition()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_ResetD( PyCallArgs& call )
{
    //        sm.GetService('scenario').ResetD()

    _log(DUNG__CALL, "ScenarioService::ResetD()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_GetEditingRoomID( PyCallArgs& call )
{
    //    roomID = scenarioSvc.GetEditingRoomID()

    _log(DUNG__CALL, "ScenarioService::GetEditingRoomID()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_SetSelectedQuantity( PyCallArgs& call )
{
    //      sm.GetService('scenario').SetSelectedQuantity(minQuantity, maxQuantity)


    _log(DUNG__CALL, "ScenarioService::SetSelectedQuantity()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_DeleteObjects( PyCallArgs& call )
{
    //    self.scenario.DeleteObjects(self.objectGroups[groupName])

    _log(DUNG__CALL, "ScenarioService::DeleteObjects()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}
PyResult ScenarioService::Handle_SetActiveHardGroup( PyCallArgs& call )
{
    //     scenarioSvc.SetActiveHardGroup(label)


    _log(DUNG__CALL, "ScenarioService::SetActiveHardGroup()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_RemoveAllHardGroups( PyCallArgs& call )
{
    //   scenario.RemoveAllHardGroups()

    _log(DUNG__CALL, "ScenarioService::RemoveAllHardGroups()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_AddHardGroup( PyCallArgs& call )
{
    // scenario.AddHardGroup(groupName, orientation)

    _log(DUNG__CALL, "ScenarioService::AddHardGroup()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_RenameHardGroup( PyCallArgs& call )
{
    // sm.GetService('scenario').RenameHardGroup(oldGroupName, newGroupName)


    _log(DUNG__CALL, "ScenarioService::RenameHardGroup()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_AreAllSelected( PyCallArgs& call )
{
    //  return self.scenario.AreAllSelected(self.objectGroups[groupName])


    _log(DUNG__CALL, "ScenarioService::AreAllSelected()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}
PyResult ScenarioService::Handle_SaveAllChanges( PyCallArgs& call )
{
    //     sm.StartService('scenario').SaveAllChanges()


    _log(DUNG__CALL, "ScenarioService::SaveAllChanges()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_PlayDungeon( PyCallArgs& call )
{
    //        sm.GetService('scenario').PlayDungeon(dungeonID, roomID, godmode=godMode)

    _log(DUNG__CALL, "ScenarioService::PlayDungeon()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_EditRoom( PyCallArgs& call )
{
    //        sm.GetService('scenario').EditRoom(dungeonID, settings.user.ui.Get('selectedRoomID', None))

    _log(DUNG__CALL, "ScenarioService::EditRoom()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_GotoRoom( PyCallArgs& call )
{
    //            sm.GetService('scenario').GotoRoom(roomID)

    _log(DUNG__CALL, "ScenarioService::GotoRoom()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_GetDunObjects( PyCallArgs& call )
{
    //       dunObjs = sm.GetService('scenario').GetDunObjects()
    /*
     *        for slimItem in dunObjs:
     *            if getattr(slimItem, 'dunObjectID', None) is not None:
     *                typeName = cfg.invtypes.Get(slimItem.typeID).name
     *                objName = cfg.evelocations.Get(slimItem.itemID).name
     *                entryName = typeName + ' : ' + objName
     *                boxItems.append([entryName,
     *                 slimItem.dunObjectID,
     *                 sm.GetService('scenario').IsSelectedByObjID(slimItem.dunObjectID),
     *                 slimItem.itemID,
     *                 slimItem.typeID])
     */
    _log(DUNG__CALL, "ScenarioService::GetDunObjects()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_GetSelObjects( PyCallArgs& call )
{
//GetSelObjects() (returns list of slim items)
    _log(DUNG__CALL, "ScenarioService::GetSelObjects()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_IsSelectedByObjID( PyCallArgs& call )
{
    //                 sm.GetService('scenario').IsSelectedByObjID(slimItem.dunObjectID)
    //            sm.StartService('scenario').SetSelectionByID(ids)


    _log(DUNG__CALL, "ScenarioService::IsSelectedByObjID()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_DuplicateSelection( PyCallArgs& call )
{
    //        sm.StartService('scenario').DuplicateSelection(amount, X, Y, Z)

    _log(DUNG__CALL, "ScenarioService::DuplicateSelection()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_SetSelectionByID( PyCallArgs& call )
{
//SetSelectionByID(ids)
    _log(DUNG__CALL, "ScenarioService::SetSelectionByID()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_SetSelectedRadius( PyCallArgs& call )
{
    //sm.GetService('scenario').SetSelectedRadius(minRadius, maxRadius)

    _log(DUNG__CALL, "ScenarioService::SetSelectedRadius()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_SetRotate( PyCallArgs& call )
{
    //        sm.GetService('scenario').SetRotate(y, p, r)

    _log(DUNG__CALL, "ScenarioService::SetRotate()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_RotateSelected( PyCallArgs& call )
{
    //        sm.GetService('scenario').RotateSelected(yaw, pitch, roll)

    _log(DUNG__CALL, "ScenarioService::RotateSelected()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_JitterSelection( PyCallArgs& call )
{
    //        sm.GetService('scenario').JitterSelection(X, Y, Z)

    _log(DUNG__CALL, "ScenarioService::JitterSelection()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_JitterRotationSelection( PyCallArgs& call )
{
    //     sm.StartService('scenario').JitterRotationSelection(yaw, pitch, roll)

    _log(DUNG__CALL, "ScenarioService::JitterRotationSelection()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_ArrangeSelection( PyCallArgs& call )
{
    //        sm.GetService('scenario').ArrangeSelection(X, Y, Z)

    _log(DUNG__CALL, "ScenarioService::ArrangeSelection()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_DeleteSelected( PyCallArgs& call )
{
    //        self.scenario.DeleteSelected()
    //OnDeleteSelected

    _log(DUNG__CALL, "ScenarioService::DeleteSelected()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}

PyResult ScenarioService::Handle_RefreshSelection( PyCallArgs& call )
{
    //        sm.GetService('scenario').RefreshSelection()

    _log(DUNG__CALL, "ScenarioService::RefreshSelection()" );
    call.Dump(DUNG__CALL_DUMP);

    return nullptr;
}
