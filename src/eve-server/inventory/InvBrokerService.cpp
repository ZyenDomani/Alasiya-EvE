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
    Author:     Zhur
    Updates:    Allan
*/

/** @todo update this code to use throws for client msgs also */

#include "eve-server.h"

#include "PyServiceCD.h"
#include "inventory/InvBrokerService.h"
#include "inventory/InventoryBound.h"
#include "system/SystemManager.h"

class InvBrokerBound
: public PyBoundObject
{
public:

    PyCallable_Make_Dispatcher(InvBrokerBound)

    InvBrokerBound(PyServiceMgr *mgr, uint32 locationID, uint32 groupID)
    : PyBoundObject(mgr),
      m_dispatch(new Dispatcher(this)),
      m_locationID(locationID),
      m_groupID(groupID)
    {
        _SetCallDispatcher(m_dispatch);

        m_strBoundObjectName = "InvBrokerBound";

		PyCallable_REG_CALL(InvBrokerBound, GetContainerContents);
        PyCallable_REG_CALL(InvBrokerBound, GetInventoryFromId);
        PyCallable_REG_CALL(InvBrokerBound, GetInventory);
        PyCallable_REG_CALL(InvBrokerBound, SetLabel);
        PyCallable_REG_CALL(InvBrokerBound, TrashItems);
        PyCallable_REG_CALL(InvBrokerBound, List);
        PyCallable_REG_CALL(InvBrokerBound, AssembleCargoContainer);

    }
    virtual ~InvBrokerBound()
    {
        delete m_dispatch;
    }

    virtual void Release() {
        //I hate this statement
        delete this;
    }

	PyCallable_DECL_CALL(GetContainerContents);
    PyCallable_DECL_CALL(GetInventoryFromId);
    PyCallable_DECL_CALL(GetInventory);
    PyCallable_DECL_CALL(SetLabel);
    PyCallable_DECL_CALL(TrashItems);
    PyCallable_DECL_CALL(List);
    PyCallable_DECL_CALL(AssembleCargoContainer);


protected:
    Dispatcher *const m_dispatch;

    uint32 m_locationID;
    uint32 m_groupID;
};

PyCallable_Make_InnerDispatcher(InvBrokerService)

InvBrokerService::InvBrokerService(PyServiceMgr *mgr)
: PyService(mgr, "invbroker"),
  m_dispatch(new Dispatcher(this))
{
    _SetCallDispatcher(m_dispatch);

    PyCallable_REG_CALL(InvBrokerService, GetItemDescriptor);
}

PyResult InvBrokerService::Handle_GetItemDescriptor(PyCallArgs &call) {
    // not really clear on the use of this one? just a general header update?!
    // from Inventory::List
	/**
            self.__itemhdr = sm.RemoteSvc('invbroker').GetItemDescriptor()
            */

    PyList *keywords = new PyList();
        keywords->AddItem(new_tuple(new PyString("stacksize"), new PyToken("util.StackSize")));
        keywords->AddItem(new_tuple(new PyString("singleton"), new PyToken("util.Singleton")));
    DBRowDescriptor* header = new DBRowDescriptor(keywords);
        header->AddColumn( "itemID",     DBTYPE_I8 );
        header->AddColumn( "typeID",     DBTYPE_I4 );
        header->AddColumn( "ownerID",    DBTYPE_I4 );
        header->AddColumn( "locationID", DBTYPE_I8 );
        header->AddColumn( "flagID",     DBTYPE_I2 );
        header->AddColumn( "quantity",   DBTYPE_I4 );
        header->AddColumn( "groupID",    DBTYPE_I2 );
        header->AddColumn( "categoryID", DBTYPE_I4 );
        header->AddColumn( "customInfo", DBTYPE_STR );
    return header;
}

InvBrokerService::~InvBrokerService() {
    delete m_dispatch;
}


PyBoundObject *InvBrokerService::_CreateBoundObject(Client *c, const PyRep *bind_args) {
    InvBroker_BindArgs args;
    //temp crap until I rework _CreateBoundObject's signature
    PyRep *t = bind_args->Clone();
    if(!args.Decode(&t)) {
        codelog(INV__ERROR, "Failed to decode bind args from '%s'", c->GetName());
        return NULL;
    }
    _log(INV__BIND, "InvBrokerService bind request:");
    args.Dump(INV__BIND, "    ");

    return new InvBrokerBound(m_manager, args.locationID, args.groupID);
}

PyResult InvBrokerBound::Handle_GetContainerContents(PyCallArgs &call)
{
    Call_TwoIntegerArgs args;
    /* args.arg1 = itemID of container to look into
     * args.arg2 = locationID of container
     */
    if (!args.Decode(&call.tuple)) {
        _log(INV__ERROR, "%s: Cannot decode arguments", call.client->GetName());
        return nullptr;
    }

    InventoryItemRef item = m_manager->item_factory->GetInventoryItemFromID( args.arg1 );
    if (!item) {
        _log(INV__ERROR, "%s: Unable to load inventory for itemID %u in locationID %u", call.client->GetName(), args.arg1, args.arg2);
        return nullptr;
    }
    if (item->ownerID() == call.client->GetCharacterID())
        _log(INV__WARNING, "Handle_GetContainerContents() -  %s(%u) is owned by calling character %s(%u) ", \
                    item->itemName().c_str(), item->itemID(), call.client->GetName(), call.client->GetCharacterID());
    else
        _log(INV__WARNING, "Handle_GetContainerContents() -  %s(%u) is not owned by calling character %s(%u) ", \
                    item->itemName().c_str(), item->itemID(), call.client->GetName(), call.client->GetCharacterID());
        // "CantDoThatWithSomeoneElsesStuff"

	return item->GetInventory()->List( flagAnywhere );
}

//this is a view into the entire inventory item.
PyResult InvBrokerBound::Handle_GetInventoryFromId(PyCallArgs &call) {
    /** @note this means "Get the Inventory of this itemID */
    _log(INV__DUMP, "InvBrokerBound::Handle_GetInventoryFromId()", "size=%u", call.tuple->size());
    call.Dump(INV__DUMP);
    /** @todo (Allan) this needs more work....
     *       return sm.GetService('invCache').GetInventoryFromId(self.itemID, locationID=session.stationid2)
     *        inv = sm.GetService('invCache').GetInventoryFromId(const.containerHangar)
     *     folder = sm.GetService('invCache').GetInventoryFromId(each.officeFolderID)
     *     folder = invCache.GetInventoryFromId(office.officeFolderID, locationID=session.stationid2)
     *       return sm.GetService('invCache').GetInventory(const.containerCorpMarket, eve.session.corpid)
     *
     * if eve.session.corprole & (const.corpRoleAccountant | const.corpRoleJuniorAccountant) != 0:
     *     office = self.corp.GetOffice()
     *     if office is not None:
     *         items = sm.GetService('invCache').GetInventoryFromId(office.itemID, locationID=session.stationid2)
     */
    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        _log(INV__ERROR, "%s: Cannot decode arguments", call.client->GetName());
        return nullptr;
    }
    //bool passive = (args.arg2 != 0);  //no idea what this is for.

    m_manager->item_factory->SetUsingClient( call.client );
    // TODO: this line is insufficient for some object types, like containers in space, so expand it
    // by having a switch that acts differently based on either categoryID or groupID or both:
    InventoryItemRef item = m_manager->item_factory->GetInventoryItemFromID( args.arg1 );
    m_manager->item_factory->UnsetUsingClient();
    if (!item) {
        _log(INV__ERROR, "%s: Unable to load inventory for itemID %u", call.client->GetName(), args.arg1);
        return nullptr;
    }

    _log(INV__BIND, "Binding inventory object to %s for item %u", call.client->GetName(), args.arg1);
    // live returns node data and timestamp
    InventoryBound *ib = new InventoryBound(m_manager, item, flagAutoFit);
    PyRep *result = m_manager->BindObject(call.client, ib);

    return result;
}

//this is a view into an inventory item using a specific flag.
PyResult InvBrokerBound::Handle_GetInventory(PyCallArgs &call) {
    /** @note  this means "Get the Inventory containing this itemID */
    _log(INV__DUMP, "InvBrokerBound::Handle_GetInventory() size=%u", call.tuple->size());
    call.Dump(INV__DUMP);
    Inventory_GetInventory args;
    if(!args.Decode(&call.tuple)) {
        codelog(INV__ERROR, "Unable to decode arguments");
        return nullptr;
    }

    InventoryItemRef item;
    if (m_groupID == EVEDB::invGroups::Station) {
        _log(INV__WARNING, "GetInventory called for station %u", m_locationID);
        item = sEntityList.GetStationByID(m_locationID);
    } else if (m_groupID == EVEDB::invGroups::Solar_System) {
        _log(INV__WARNING, "GetInventory called for solar system %u", m_locationID);
        item = m_manager->item_factory->/*GetSolarSystem*/GetItem(m_locationID);
    } else {
        _log(INV__WARNING, "GetInventory called for item %u (group: %u)", m_locationID, m_groupID);
        m_manager->item_factory->SetUsingClient( call.client );
        item = m_manager->item_factory->/*GetInventoryItemFromID*/GetItem(m_locationID);
        m_manager->item_factory->UnsetUsingClient();
    }
    if (!item) {
        codelog(INV__ERROR, "%s: Unable to load item %u for flag %u", call.client->GetName(), m_locationID, args.container);
        return nullptr;
    }

    EVEItemFlags flag = flagAutoFit;
    switch(args.container) {
        case containerWallet:/*10001*/
            flag = flagWallet;
            break;
        case containerCharacter:/*10011*/
            flag = flagSkill;
            break;
        case containerHangar:/*10004*/
            flag = flagHangar;
            break;
        case containerCorpMarket:/*10012*/   //this is for corp station deliveries (station button)
            flag = flagCorpMarket;
            break;
        case containerOffices:/*10009*/
            flag = flagOfficeSlot1;
            break;
        case containerFactory:/*10006*/
            flag = flagFactory;
            break;

        //case containerGlobal:/*10002*/
        //case containerSolarSystem:/*10003*/
        //case containerScrapHeap:/*10005*/
        //case containerBank:/*10007*/
        //case containerRecycler:/*10008*/
        //case containerStationCharacters:/*10010*/
            //flag = flagNone;
            //break;
        default:
            _log(INV__ERROR, "Unhandled container type %u for locationID %u", args.container, m_locationID);
            return nullptr;
    }

    _log(INV__BIND, "Binding inventory object to %s for inventory of %u with flag %u", call.client->GetName(), m_locationID, flag);

    //we just bind up a new inventory object for container requested and give it back to them.
    InventoryBound *ib = new InventoryBound(m_manager, item, flag);
    PyRep *result = m_manager->BindObject(call.client, ib);

    return result;
}

PyResult InvBrokerBound::Handle_SetLabel(PyCallArgs &call) {
    CallSetLabel args;
    if(!args.Decode(&call.tuple)) {
        codelog(INV__ERROR, "Unable to decode arguments");
        return nullptr;
    }

    m_manager->item_factory->SetUsingClient( call.client );
    InventoryItemRef item = m_manager->item_factory->GetItem( args.itemID );
    if( !item ) {
        codelog(INV__ERROR, "%s: Unable to load item %u", call.client->GetName(), args.itemID);
        return nullptr;
    }

    if(item->ownerID() != call.client->GetCharacterID()) {
        _log(INV__ERROR, "Character %u tried to rename item %u of character %u.", call.client->GetCharacterID(), item->itemID(), item->ownerID());
        return nullptr;
    }

    item->Rename( args.itemName.c_str() );


    // This call as-is is NOT correct for any item category other than ships,
    // so until we can get the right string argument for other kinds of session updates,
    // we need to block this call so our characters don't "board" non-ship objects:
    if( item->categoryID() == EVEDB::invCategories::Ship )
        call.client->UpdateSessionInt("shipid", item->itemID() );

    // Release the item factory now that the ItemFactory is finished being used:
    m_manager->item_factory->UnsetUsingClient();

    return nullptr;
}

PyResult InvBrokerBound::Handle_TrashItems(PyCallArgs &call) {
    Call_TrashItems args;
    if(!args.Decode(&call.tuple)) {
        _log(INV__ERROR, "Unable to decode arguments");
        return nullptr;
    }

    std::vector<int32>::const_iterator cur = args.items.begin();
    for(; cur != args.items.end(); cur++) {
        InventoryItemRef item = m_manager->item_factory->GetItem( *cur );
        if (!item)
            _log(INV__ERROR, "%s: Unable to load item %u to delete it. Skipping.", call.client->GetName(), *cur);
        else if (call.client->GetCharacterID() != item->ownerID())
            _log(INV__ERROR, "%s: Tried to trash item %u which is not yours. Skipping.", call.client->GetName(), *cur);
        else if (item->locationID() != (uint32)args.locationID)
            _log(INV__ERROR, "%s: Item %u is not in location %u. Skipping.", call.client->GetName(), *cur, args.locationID);
        else
            item->Delete();
    }

    return nullptr;
}

PyResult InvBrokerBound::Handle_List(PyCallArgs &call) {
/**
        inv = invCache.GetInventoryFromId(activeShipID, locationID=session.stationid2)
        shipCargo = inv.List()
            */

    sLog.Log( "InvBrokerBound::Handle_List()", "size= %u", call.tuple->size() );
    call.Dump(INV__DUMP);

    return nullptr;
}

PyResult InvBrokerBound::Handle_AssembleCargoContainer(PyCallArgs &call) {
    /* invMgr.AssembleCargoContainer(invItem.itemID, None, 0.0)
     *
     * 14:37:46 [BindDump]   Call Arguments:
     * 14:37:46 [BindDump]       Tuple: 3 elements
     * 14:37:46 [BindDump]         [ 0] Integer field: 140000489
     * 14:37:46 [BindDump]         [ 1] (None)
     * 14:37:46 [BindDump]         [ 2] Real field: 0.000000
     *
     * 14:37:46 L InvBrokerBound::Handle_AssembleCargoContainer(): [00msize= 3
     * 14:37:46 [InvMsg]   Call Arguments:
     * 14:37:46 [InvMsg]       Tuple: 3 elements
     * 14:37:46 [InvMsg]         [ 0] Integer field: 140000489
     * 14:37:46 [InvMsg]         [ 1] (None)
     * 14:37:46 [InvMsg]         [ 2] Real field: 0.000000
     */

    sLog.Log( "InvBrokerBound::Handle_AssembleCargoContainer()", "size= %u", call.tuple->size() );
    call.Dump(INV__DUMP);

    return nullptr;
}
