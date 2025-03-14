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
    Author:        Zhur
    Rewrite:    Allan
*/

/* Dungeon Logging
 * DUNG__ERROR
 * DUNG__WARNING
 * DUNG__INFO
 * DUNG__MESSAGE
 * DUNG__TRACE
 * DUNG__CALL
 * DUNG__CALL_DUMP
 * DUNG__RSP_DUMP
 * DUNG__DB_ERROR
 * DUNG__DB_WARNING
 * DUNG__DB_INFO
 * DUNG__DB_MESSAGE
 */

/* dungeon notifications
 * 'OnDungeonEdit',
 * 'OnDistributionDungeonEntered',
 * 'OnEscalatingPathDungeonEntered'
 * 'OnJessicaOpenDungeon',
 * 'OnJessicaOpenRoom',
 * 'OnDESelectionChanged',
 * 'OnDEObjectPaletteChanged',
 * 'OnDEObjectListChanged',
 * 'OnSelectObject',
 * 'OnDungeonSelectionGroupRotation',
 * 'OnBSDTablesChanged'
 * 'OnBSDRevisionChange'
 */


#include "eve-server.h"

#include "PyServiceCD.h"
#include "../system/cosmicMgrs/DungeonMgr.h"
#include "DungeonService.h"
#include "DungeonDB.h"
#include "system/SystemManager.h"


PyCallable_Make_InnerDispatcher(DungeonService)

DungeonService::DungeonService(PyServiceMgr *mgr)
: PyService(mgr, "dungeon"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    // open in client with <ctrl><shift>d
    // editor calls
    PyCallable_REG_CALL(DungeonService, GetArchetypes);
    PyCallable_REG_CALL(DungeonService, DEGetFactions);
    PyCallable_REG_CALL(DungeonService, DEGetDungeons);
    PyCallable_REG_CALL(DungeonService, DEGetTemplates);
    PyCallable_REG_CALL(DungeonService, DEGetRooms);
    PyCallable_REG_CALL(DungeonService, DEGetRoomObjectPaletteData);
    // objects
    PyCallable_REG_CALL(DungeonService, IsObjectLocked);
    PyCallable_REG_CALL(DungeonService, AddObject);
    PyCallable_REG_CALL(DungeonService, RemoveObject);
    PyCallable_REG_CALL(DungeonService, CopyObject);
    PyCallable_REG_CALL(DungeonService, EditObject);
    PyCallable_REG_CALL(DungeonService, EditObjectName);
    PyCallable_REG_CALL(DungeonService, EditObjectRadius);
    PyCallable_REG_CALL(DungeonService, EditObjectXYZ);
    PyCallable_REG_CALL(DungeonService, EditObjectYawPitchRoll);
    // templates
    PyCallable_REG_CALL(DungeonService, TemplateAdd);
    PyCallable_REG_CALL(DungeonService, TemplateRemove);
    PyCallable_REG_CALL(DungeonService, TemplateEdit);
    PyCallable_REG_CALL(DungeonService, AddTemplateObjects);
    PyCallable_REG_CALL(DungeonService, TemplateObjectAddDungeonList);
}

DungeonService::~DungeonService() {
    delete m_dispatch;
}

PyResult DungeonService::Handle_AddObject( PyCallArgs& call )
{
    // (newObjectID, revisionID,) = sm.RemoteSvc('dungeon').AddObject(roomID, typeID, x, y, z, yaw, pitch, roll, radius)

    _log(DUNG__CALL,  "DungeonService::Handle_AddObject size: %lu", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    if(call.tuple->size() != 9) {
        _log(SERVICE__ERROR, "Wrong number of arguments in call to AddObject");
        return nullptr;
    }

    Dungeon::RoomObject newObject;
    newObject.roomID = PyRep::IntegerValue(call.tuple->GetItem(0));
    newObject.typeID = PyRep::IntegerValueU32(call.tuple->GetItem(1));
    newObject.x = PyRep::FloatValue(call.tuple->GetItem(2));
    newObject.y = PyRep::FloatValue(call.tuple->GetItem(3));
    newObject.z = PyRep::FloatValue(call.tuple->GetItem(4));
    newObject.yaw = PyRep::FloatValue(call.tuple->GetItem(5));
    newObject.pitch = PyRep::FloatValue(call.tuple->GetItem(6));
    newObject.roll = PyRep::FloatValue(call.tuple->GetItem(7));
    newObject.radius = PyRep::FloatValue(call.tuple->GetItem(8));

    uint32 groupID = DungeonDB::GetFirstGroupForRoom(newObject.roomID);

    newObject.objectID = DungeonDB::CreateObject(newObject.roomID, newObject.typeID, groupID, newObject.x, newObject.y, newObject.z, newObject.yaw, newObject.pitch, newObject.roll, newObject.radius);

    Client *pClient(call.client);

    GPoint objPos;
    objPos.x = newObject.x + pClient->GetSession()->GetCurrentFloat("editor_room_x");
    objPos.y = newObject.y + pClient->GetSession()->GetCurrentFloat("editor_room_y");
    objPos.z = newObject.z + pClient->GetSession()->GetCurrentFloat("editor_room_z");

    ItemData dData(newObject.typeID, 1/*EVE SYSTEM*/, pClient->GetLocationID(), flagNone, "", objPos);
    InventoryItemRef iRef = InventoryItem::SpawnItem(sItemFactory.GetNextTempID(), dData);
    if (iRef.get() == nullptr) {// Failed to spawn the item
        throw CustomError("Failed to spawn the item");
        return nullptr;
    }

    DungeonEditSE* oSE;
    oSE = new DungeonEditSE(iRef, pClient->services(), pClient->SystemMgr(), newObject);

    // Add the object to the room
    //pClient->services().LookupService("keeper")->GetBound()->AddRoomObject(oSE);

    // Add the entity to the SystemManager
    pClient->SystemMgr()->AddEntity(oSE, false);

    // Return objectID and revisionID
    PyTuple *result = new PyTuple(2);
    result->SetItem(0, new PyInt(newObject.objectID));
    result->SetItem(1, new PyInt(1));    //revisionID   arbitrary for now...

    return result;
}

PyResult DungeonService::Handle_CopyObject( PyCallArgs& call )
{
    //newObjectID = sm.RemoteSvc('dungeon').CopyObject(objectID, roomID, offsetX, offsetY, offsetZ)
    _log(DUNG__CALL,  "DungeonService::Handle_CopyObject  size: %lu", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    if(call.tuple->size() != 5) {
        _log(SERVICE__ERROR, "Wrong number of arguments in call to CopyObject");
        return nullptr;
    }

    int32 objectID     = PyRep::IntegerValue(call.tuple->GetItem(0));
    int32 roomID       = PyRep::IntegerValue(call.tuple->GetItem(1));
    float offsetX    = PyRep::FloatValue(call.tuple->GetItem(2));
    float offsetY    = PyRep::FloatValue(call.tuple->GetItem(3));
    float offsetZ    = PyRep::FloatValue(call.tuple->GetItem(4));

    // Copy the object to the room
    Dungeon::RoomObject newObject = Dungeon::RoomObject();
    /*
    newObject.roomID = roomID->value();
    newObject.typeID = call.client->services().LookupService("keeper")->GetRoomObject(objectID->value())->GetData().typeID;
    newObject.x = call.client->services().LookupService("keeper")->GetBound()->GetRoomObject(objectID->value())->GetData().x + offsetX->value();
    newObject.y = call.client->services().LookupService("keeper")->GetBound()->GetRoomObject(objectID->value())->GetData().y + offsetY->value();
    newObject.z = call.client->services().LookupService("keeper")->GetBound()->GetRoomObject(objectID->value())->GetData().z + offsetZ->value();
    newObject.yaw = call.client->services().LookupService("keeper")->GetBound()->GetRoomObject(objectID->value())->GetData().yaw;
    newObject.pitch = call.client->services().LookupService("keeper")->GetBound()->GetRoomObject(objectID->value())->GetData().pitch;
    newObject.roll = call.client->services().LookupService("keeper")->GetBound()->GetRoomObject(objectID->value())->GetData().roll;
    newObject.radius = call.client->services().LookupService("keeper")->GetBound()->GetRoomObject(objectID->value())->GetData().radius;
*/
    uint32 groupID = DungeonDB::GetFirstGroupForRoom(newObject.roomID);

    newObject.objectID = DungeonDB::CreateObject(newObject.roomID, newObject.typeID, groupID, newObject.x, newObject.y, newObject.z, newObject.yaw, newObject.pitch, newObject.roll, newObject.radius);

    Client *pClient(call.client);

    GPoint objPos;
        objPos.x = newObject.x + pClient->GetSession()->GetCurrentFloat("editor_room_x");
        objPos.y = newObject.y + pClient->GetSession()->GetCurrentFloat("editor_room_y");
        objPos.z = newObject.z + pClient->GetSession()->GetCurrentFloat("editor_room_z");

    ItemData dData(newObject.typeID, 1/*EVE SYSTEM*/, pClient->GetLocationID(), flagNone, "", objPos);
    InventoryItemRef iRef = InventoryItem::SpawnItem(sItemFactory.GetNextTempID(), dData);
    if (iRef.get() == nullptr) {// Failed to spawn the item
        throw CustomError("Failed to spawn the item");
        return nullptr;
    }

    DungeonEditSE* oSE;
    oSE = new DungeonEditSE(iRef, pClient->services(), pClient->SystemMgr(), newObject);

    // Add the object to the room
    //pClient->services().LookupService("keeper")->GetBound()->AddRoomObject(oSE);

    // Add the entity to the SystemManager
    pClient->SystemMgr()->AddEntity(oSE, false);

    return nullptr;
}

PyResult DungeonService::Handle_EditObjectRadius( PyCallArgs& call )
{
    //sm.RemoteSvc('dungeon').EditObjectRadius(objectID=objectID, radius=radius)
    _log(DUNG__CALL,  "DungeonService::Handle_EditObjectRadius  size: %lu", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    uint32 itemID = PyRep::IntegerValueU32(call.byname["objectID"]);
    if (itemID == 0) {
        call.client->SendErrorMsg("EditObjectRadius send itemID 0.");
        return nullptr;
    }

    SystemEntity* pSE = call.client->SystemMgr()->GetEntityByID(itemID);
    if (!pSE->IsDungeonEditSE()) {
        call.client->SendErrorMsg("The selected object is not part of the current dungeon");
        return nullptr;
    }

    double radius = PyRep::FloatValue(call.byname["radius"]);

    //dungeonEntity->DestinyMgr()->SetRadius(radius, true);

    // save the position to the database
    DungeonDB::EditObjectRadius(itemID, radius);

    return nullptr;
}

PyResult DungeonService::Handle_EditObjectXYZ( PyCallArgs& call )
{
    //sm.RemoteSvc('dungeon').EditObjectXYZ(objectID=objectID, x=x, y=y, z=z)
    _log(DUNG__CALL,  "DungeonService::Handle_EditObjectXYZ  size: %lu", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    uint32 itemID = PyRep::IntegerValueU32(call.byname["objectID"]);
    if (itemID == 0) {
        call.client->SendErrorMsg("EditObjectXYZ send itemID 0.");
        return nullptr;
    }

    SystemEntity* pSE = call.client->SystemMgr()->GetEntityByID(itemID);
    if (!pSE->IsDungeonEditSE()) {
        call.client->SendErrorMsg("The selected object is not part of the current dungeon");
        return nullptr;
    }

    DungeonEditSE* dSE = pSE->GetDungeonEditSE();

    // set the new position
    int64 x = PyRep::IntegerValue(call.byname["x"]);
    int64 y = PyRep::IntegerValue(call.byname["y"]);
    int64 z = PyRep::IntegerValue(call.byname["z"]);
    GPoint roomPos = call.client->GetShipSE()->GetPosition();
    const GPoint pos(roomPos.x + x, roomPos.y + y, roomPos.z + z);
    dSE->DestinyMgr()->SetPosition(pos, true);

    // save the position to the database
    DungeonDB::EditObjectXYZ(itemID, x, y, z);

    return nullptr;
}

PyResult DungeonService::Handle_EditObjectYawPitchRoll( PyCallArgs& call ) {
    //sm.RemoteSvc('dungeon').EditObjectYawPitchRoll(objectID=objectID, yaw=yaw, pitch=pitch, roll=roll)
    _log(DUNG__CALL,  "DungeonService::Handle_EditObjectYawPitchRoll size: %lu", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    uint32 itemID = PyRep::IntegerValueU32(call.byname["objectID"]);
    if (itemID == 0) {
        call.client->SendErrorMsg("EditObjectYPR send itemID 0.");
        return nullptr;
    }

    SystemEntity* pSE = call.client->SystemMgr()->GetEntityByID(itemID);
    if (!pSE->IsDungeonEditSE()) {
        call.client->SendErrorMsg("The selected object is not part of the current dungeon");
        return nullptr;
    }

    DungeonEditSE* dSE = pSE->GetDungeonEditSE();

    double yaw = call.byname["yaw"]->AsFloat()->value();
    double pitch = call.byname["pitch"]->AsFloat()->value();
    double roll = call.byname["roll"]->AsFloat()->value();

    //TODO: finish this

    // save the position to the database
    //DungeonDB::EditObjectYawPitchRoll(dSE->GetData().objectID, yaw, pitch, roll);

    return nullptr;
}

PyResult DungeonService::Handle_TemplateAdd( PyCallArgs& call )
{
    //templateID = sm.RemoteSvc('dungeon').TemplateAdd(templateName, templateDescription)
    _log(DUNG__CALL,  "DungeonService::Handle_TemplateAdd  size: %lu", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    // Get the currently edited roomID from KeeperBound
    //uint32 roomID = call.client->services().LookupService("keeper")->GetBound()->GetCurrentRoomID();

    if(call.tuple->size() != 2) {
        _log(SERVICE__ERROR, "Wrong number of arguments in call to TemplateAdd");
        return nullptr;
    }
/*
    // Convert to std::string
    std::string nameString = PyRep::StringContent(templateName);
    std::string descriptionString = PyRep::StringContent(templateDescription);

    // Create the new template
    uint32 templateID = DungeonDB::CreateTemplate(nameString, descriptionString, roomID);

    return new PyInt(templateID);
    */
    return nullptr;
}

PyResult DungeonService::Handle_AddTemplateObjects( PyCallArgs& call )
{
    // objectIDs = sm.RemoteSvc('dungeon').AddTemplateObjects(roomID, self.sr.node.id, (posInRoom.x, posInRoom.y, posInRoom.z)

    _log(DUNG__CALL,  "DungeonService::Handle_AddTemplateObjects  size: %lu", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    if(call.tuple->size() != 3) {
        _log(SERVICE__ERROR, "Wrong number of arguments in call to AddTemplateObjects");
        return nullptr;
    }
/*
    double posInRoomX = position->GetItem(0)->AsFloat()->value();
    double posInRoomY = position->GetItem(1)->AsFloat()->value();
    double posInRoomZ = position->GetItem(2)->AsFloat()->value();

    std::vector<Dungeon::RoomObject> objects;
    DungeonDB::GetTemplateObjects(objectID->value(), objects);

    Client *pClient(call.client);
    PyList* objectIDs = new PyList();

    uint32 groupID = DungeonDB::GetFirstGroupForRoom(roomID->value());

    // Spawn the items in the object list
    for (auto cur : objects) {
        GPoint objPos;

        // Relative position for the object to be spawned at
        objPos.x = posInRoomX + cur.x + pClient->GetSession()->GetCurrentFloat("editor_room_x");
        objPos.y = posInRoomY + cur.y + pClient->GetSession()->GetCurrentFloat("editor_room_y");
        objPos.z = posInRoomZ + cur.z + pClient->GetSession()->GetCurrentFloat("editor_room_z");

        // Position to be stored in the DB
        double dbPosX = posInRoomX + cur.x;
        double dbPosY = posInRoomY + cur.y;
        double dbPosZ = posInRoomZ + cur.z;

        // Create the object in the database
        DungeonDB::CreateObject(roomID->value(), cur.typeID, groupID, dbPosX, dbPosY, dbPosZ, cur.yaw, cur.pitch, cur.roll, cur.radius);

        ItemData dData(cur.typeID, ownerSystem, pClient->GetLocationID(), flagNone, "", objPos);
        InventoryItemRef iRef = InventoryItem::SpawnItem(sItemFactory.GetNextTempID(), dData);
        if (iRef.get() == nullptr) // Failed to spawn the item
            continue;
        DungeonEditSE* oSE;
        oSE = new DungeonEditSE(iRef, pClient->services(), pClient->SystemMgr(), cur);

        pClient->services().LookupService("keeper")->GetBound()->AddRoomObject(oSE);
        pClient->SystemMgr()->AddEntity(oSE, false);

        objectIDs->AddItem(new PyInt(oSE->GetData().objectID));
    }

    return objectIDs;
    */
    return nullptr;
}

// this should get templates to add to dungeon.  probably not active dungeons
PyResult DungeonService::Handle_DEGetDungeons( PyCallArgs& call )
{
    /* dungeon = sm.RemoteSvc('dungeon').DEGetDungeons(archetypeID=archetypeID, factionID=factionID)
     * dungeon = sm.RemoteSvc('dungeon').DEGetDungeons(dungeonID=dungeonID)[0]
     * dungeon.dungeonNameID, dungeon.dungeonID, dungeon.factionID
     */
    uint32 dungeonID = PyRep::IntegerValueU32(call.byname["dungeonID"]);
    uint32 archetypeID = PyRep::IntegerValueU32(call.byname["archetypeID"]);
    uint32 factionID = PyRep::IntegerValueU32(call.byname["factionID"]);

    //Dungeon Status (1=Release, 2=Testing, 3=Working Copy)

    return DungeonDB::GetDungeons(dungeonID, archetypeID, factionID);
}

PyResult DungeonService::Handle_DEGetRoomObjectPaletteData( PyCallArgs& call ) {
    //  roomObjectGroups = sm.RemoteSvc('dungeon').DEGetRoomObjectPaletteData()
    return sDunDataMgr.GetPaletteGroups();
}

PyResult DungeonService::Handle_GetArchetypes( PyCallArgs& call ) {
    //archetypes = sm.RemoteSvc('dungeon').GetArchetypes()
    return DungeonDB::GetArchetypes();
}

PyResult DungeonService::Handle_DEGetFactions(PyCallArgs& call) {
    //factions = sm.RemoteSvc('dungeon').DEGetFactions()
    return sDataMgr.GetFactionIDs();
}

// these next 2 are for dungeon templates...not complete dungeons or rooms
PyResult DungeonService::Handle_DEGetTemplates( PyCallArgs& call ) {
    /*        self.templateRows = sm.RemoteSvc('dungeon').DEGetTemplates()
     *        for row in self.templateRows:
     *            data = {'label': row.templateName,
     *             'hint': row.description != row.templateName and row.description or '',
     *             'id': row.templateID,
     *             'form': self}
     */
    //dungeonNameID   nameID from output.txt

    // this will be dungeon template rooms...maybe?
    PyObjectEx* ret = DungeonDB::GetTemplates(call.client);

    //if (is_log_enabled(DUNG__RSP_DUMP))
    //    ret->Dump(DUNG__RSP_DUMP, "   ");

    return ret;
}

PyResult DungeonService::Handle_DEGetRooms( PyCallArgs& call ) {
    //rooms = sm.RemoteSvc('dungeon').DEGetRooms(dungeonID=seldungeon.dungeonID)
    return DungeonDB::GetRooms(PyRep::IntegerValueU32(call.byname["dungeonID"]));
}


/*{'messageKey': 'DunAuthoringError', 'dataID': 17883918, 'suppressable': False, 'bodyID': 259674, 'messageType': 'warning', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 830}
 * {'messageKey': 'DunBlacklistCannotWarp', 'dataID': 17880454, 'suppressable': False, 'bodyID': 258397, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 258396, 'messageID': 2100}
 * {'messageKey': 'DunCantAnchor', 'dataID': 17883867, 'suppressable': False, 'bodyID': 259656, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 831}
 * {'messageKey': 'DunCantUseMWD', 'dataID': 17883442, 'suppressable': False, 'bodyID': 259505, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 832}
 * {'messageKey': 'DunGateLocked_ManyKeys', 'dataID': 17883928, 'suppressable': False, 'bodyID': 259678, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259677, 'messageID': 834}
 * {'messageKey': 'DunGateLocked_OneKey', 'dataID': 17883923, 'suppressable': False, 'bodyID': 259676, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259675, 'messageID': 835}
 * {'messageKey': 'DunGateLocked_ZeroKey', 'dataID': 17883939, 'suppressable': False, 'bodyID': 259682, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259681, 'messageID': 836}
 * {'messageKey': 'DunGateNPCsAround', 'dataID': 17883447, 'suppressable': False, 'bodyID': 259507, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259506, 'messageID': 837}
 * {'messageKey': 'DunGateNoSkill', 'dataID': 17883915, 'suppressable': False, 'bodyID': 259673, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259672, 'messageID': 838}
 * {'messageKey': 'DunGateTooFarAway', 'dataID': 17883844, 'suppressable': False, 'bodyID': 259648, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259647, 'messageID': 839}
 * {'messageKey': 'DunPodsCannotWarp', 'dataID': 17883452, 'suppressable': False, 'bodyID': 259509, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259508, 'messageID': 840}
 * {'messageKey': 'DunShipCannotWarp', 'dataID': 17883907, 'suppressable': False, 'bodyID': 259670, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259669, 'messageID': 841}
 */

/*******************************************
 *   these do nothing yet...
 */

PyResult DungeonService::Handle_EditObject( PyCallArgs& call )
{
    _log(DUNG__CALL,  "DungeonService::Handle_EditObject  size: %lu", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    // this should probably send some kind of notification

    return nullptr;
}

PyResult DungeonService::Handle_IsObjectLocked( PyCallArgs& call )
{
    //return sm.RemoteSvc('dungeon').IsObjectLocked(objectID)
    _log(DUNG__CALL,  "DungeonService::Handle_IsObjectLocked size: %lu", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    SingleIntegerArg arg;
    if (!arg.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    PyTuple* result = new PyTuple(2);
    result->SetItem(0, new PyBool(false));
    result->SetItem(1, new PyList());

    return result;
}

PyResult DungeonService::Handle_EditObjectName( PyCallArgs& call )
{
    //sm.RemoteSvc('dungeon').EditObjectName(newObjectID, objectName)
    _log(DUNG__CALL,  "DungeonService::Handle_EditObjectName  size: %lu", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    if(call.tuple->size() != 2) {
        _log(SERVICE__ERROR, "Wrong number of arguments in call to EditObjectName");
        return nullptr;
    }
    /*
     *    // Update the object name
     *    DungeonEditSE* oSE = call.client->services().LookupService("keeper")->GetBound()->GetRoomObject(newObjectID->value());
     *
     *    std::string newObjectName = PyRep::StringContent(objectName);
     *    oSE->Rename(newObjectName.c_str());
     */

    // this should probably send some kind of notification

    return nullptr;
}

PyResult DungeonService::Handle_TemplateObjectAddDungeonList( PyCallArgs& call )
{
    //sm.RemoteSvc('dungeon').TemplateObjectAddDungeonList(templateID, objectIDList)
    _log(DUNG__CALL,  "DungeonService::Handle_TemplateObjectAddDungeonList  size: %lu", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    // this should probably send some kind of notification

    return nullptr;
}

PyResult DungeonService::Handle_TemplateRemove( PyCallArgs& call )
{
    //sm.RemoteSvc('dungeon').TemplateRemove(self.sr.node.id)
    _log(DUNG__CALL,  "DungeonService::Handle_TemplateRemove  size: %lu", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);
    /*
     *    11:31:12 [DungCall] DungeonService::Handle_TemplateRemove  size: 1
     *    11:31:12 [DungCallDump]   Call Arguments:
     *    11:31:12 [DungCallDump]      Tuple: 1 elements
     *    11:31:12 [DungCallDump]       [ 0]    Integer: 11110
     */
    //DungeonDB::DeleteTemplate(call.tuple->GetItem(0)->AsInt()->value());

    // this should probably send some kind of notification
    return nullptr;
}

PyResult DungeonService::Handle_TemplateEdit( PyCallArgs& call )
{
    //dungeonSvc.TemplateEdit(self.templateRow.templateID, templateName, templateDescription)
    _log(DUNG__CALL,  "DungeonService::Handle_TemplateEdit  size: %lu", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    //DungeonDB::EditTemplate(call.tuple->GetItem(0)->AsInt()->value(), call.tuple->GetItem(1)->AsWString()->content(), call.tuple->GetItem(2)->AsWString()->content());

    return nullptr;
}

PyResult DungeonService::Handle_RemoveObject( PyCallArgs& call )
{
    //sm.RemoteSvc('dungeon').RemoveObject(objectID)
    _log(DUNG__CALL,  "DungeonService::Handle_RemoveObject  size: %lu", call.tuple->size());
    call.Dump(DUNG__CALL_DUMP);

    if(call.tuple->size() != 1) {
        _log(SERVICE__ERROR, "Wrong number of arguments in call to RemoveObject");
        return nullptr;
    }

    // this should probably send some kind of notification

    // Remove the object from the room
    //call.client->services().LookupService("keeper")->GetBound()->RemoveRoomObject(objectID->value());

    return nullptr;
}
