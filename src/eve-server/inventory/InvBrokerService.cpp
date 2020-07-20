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
    Rewrite:    Allan
*/

/** @todo update this code to use throws for client msgs also */
/** @todo  many unfinished calls in this file.... */

#include "eve-server.h"

#include "EntityList.h"
#include "PyServiceCD.h"
#include "inventory/InvBrokerService.h"
#include "inventory/InventoryBound.h"
#include "station/StationDataMgr.h"
#include "station/StationOffice.h"
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
        PyCallable_REG_CALL(InvBrokerBound, BreakPlasticWrap);
        PyCallable_REG_CALL(InvBrokerBound, TakeOutTrash);

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
    PyCallable_DECL_CALL(BreakPlasticWrap);
    PyCallable_DECL_CALL(TakeOutTrash);


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

    PyList* keywords = new PyList();
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


PyBoundObject* InvBrokerService::CreateBoundObject(Client *pClient, const PyRep *bind_args) {
    InvBroker_BindArgs args;
    PyRep *t = bind_args->Clone();
    if(!args.Decode(&t)) {
        codelog(SERVICE__ERROR, "Failed to decode bind args from '%s'", pClient->GetName());
        return NULL;
    }
    _log(INV__BIND, "InvBrokerService bind request:");
    args.Dump(INV__BIND, "    ");

    return new InvBrokerBound(m_manager, args.locationID, args.groupID);
}

//is this completely right?
PyResult InvBrokerBound::Handle_GetContainerContents(PyCallArgs &call)
{
    Call_TwoIntegerArgs args;
    /* args.arg1 = itemID of container to look into
     * args.arg2 = locationID of container
     */
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    InventoryItemRef item = sItemFactory.GetInventoryItemFromID( args.arg1 );
    if (item.get() == nullptr) {
        _log(INV__ERROR, "GetContainerContents() - Unable to load inventory for itemID %u in locationID %u", args.arg1, args.arg2);
        return nullptr;
    }
    if (item->ownerID() == call.client->GetCharacterID())
        _log(INV__WARNING, "Handle_GetContainerContents() -  %s(%u) is owned by calling character %s(%u) ", \
                    item->itemName().c_str(), item->itemID(), call.client->GetName(), call.client->GetCharacterID());
    else if ((item->ownerID() != call.client->GetCharacterID()) and IsSolarSystem(args.arg2))
        _log(INV__WARNING, "Handle_GetContainerContents() -  %s(%u) is in space and not owned by calling character %s(%u) ", \
                    item->itemName().c_str(), item->itemID(), call.client->GetName(), call.client->GetCharacterID());
    else {
        _log(INV__WARNING, "Handle_GetContainerContents() -  %s(%u) is not owned by calling character %s(%u) ", \
                    item->itemName().c_str(), item->itemID(), call.client->GetName(), call.client->GetCharacterID());
        if (call.client->CanThrow())
            throw PyException(MakeUserError("CantDoThatWithSomeoneElsesStuff"));
    }

    return item->GetMyInventory()->List( flagAnywhere );
}

//this is a view into the entire inventory item.  this CAN throw.  find and implement client error msgs here for corp usage
PyResult InvBrokerBound::Handle_GetInventoryFromId(PyCallArgs &call) {
    /** @note this means "Get the Inventory of this itemID */
    /*
            if e.args[0] == 'CrpAccessDenied':
                self.CloseContainer(itemid)
    */
    _log(INV__DUMP, "InvBrokerBound::Handle_GetInventoryFromId()", "size=%u", call.tuple->size());
    call.Dump(INV__DUMP);

    Call_TwoIntegerArgs args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    bool passive = (args.arg2 != 0);  //no idea what this is for.
    sItemFactory.SetUsingClient( call.client );
    InventoryItemRef iRef(nullptr);
    // if item requested is office folder, we have to do shit a lil different, as it sends officeFolderID, instead of itemID
    if ((m_groupID == EVEDB::invGroups::Station) and (IsOfficeFolder(args.arg1))) {
        uint32 officeID = stDataMgr.GetOfficeIDForCorp(m_locationID, call.client->GetCorporationID());
        iRef = sItemFactory.GetInventoryItemFromID( officeID );
    } else {
        iRef = sItemFactory.GetInventoryItemFromID( args.arg1 );
    }
    sItemFactory.UnsetUsingClient();
    if (iRef.get() == nullptr) {
        _log(INV__ERROR, "GetInventoryFromId() - Unable to get inventoryItem for itemID %u", args.arg1);
        // send error to player?
        return nullptr;
    }

    // use container's owner as ownerID
    uint32 ownerID = iRef->ownerID();

    /** @todo  test for container types and set flag accordingly
     * this is used to later call List().  flags set here are used by list to determine what contents to return
     *
     * NOTE: found this in client.  'LoadFitting' not coded yet, but will need to test for it once implemented
     * def LoadFitting(self, ownerID, fittingID):
     *   ...
        inv = self.invCache.GetInventoryFromId(const.containerHangar)
     */
    EVEItemFlags flag = flagAutoFit;
    switch (iRef->categoryID()) {
        case EVEDB::invCategories::Owner: {
            switch (iRef->groupID()) {
                case EVEDB::invGroups::Character: {
                    flag = flagAutoFit;
                    ownerID = call.client->GetCharacterID();
                } break;
                case EVEDB::invGroups::Corporation: {
                    flag = flagAutoFit;
                    ownerID = call.client->GetCorporationID();
                } break;
                case EVEDB::invGroups::Alliance: {
                    flag = flagAutoFit;
                    ownerID = call.client->GetAllianceID();
                } break;
                case EVEDB::invGroups::Faction: {
                    // not sure if this is used...
                    flag = flagAutoFit;
                    ownerID = call.client->GetWarFactionID();
                } break;
            }
        } break;
        case EVEDB::invCategories::Ship: {
            // should we test for ships and hangar types here?   yes.
            if (iRef->HasAttribute(AttrHasCorporateHangars))
                flag = flagCargoHold;   // this is also corp hangar 1
            else
                flag = flagCargoHold;
        } break;
        case EVEDB::invCategories::Structure: {
            switch(iRef->groupID()) {
                case EVEDB::invGroups::Control_Tower: {
                    flag = flagAutoFit;
                } break;
                default: {
                    flag = flagHangar;
                } break;
            } break;
        } break;
        case EVEDB::invCategories::Orbitals: {
            switch(iRef->groupID()) {
                case EVEDB::invGroups::Orbital_Construction_Platform: {
                    // not sure what to do in this case...
                    flag = flagAutoFit;
                } break;
                case EVEDB::invGroups::Orbital_Infrastructure: {
                    // this includes orbital command centers, which this may not be right for.
                    flag = flagHangar;
                    ownerID = call.client->GetCharacterID();
                } break;
            }
        } break;
        case EVEDB::invCategories::Station: {
            flag = flagHangar;
        } break;
        case EVEDB::invCategories::Deployable:
        case EVEDB::invCategories::StructureUpgrade: {
            flag = flagCargoHold;
        } break;
        case EVEDB::invCategories::Trading: {
            _log(INV__WARNING, "GetInventoryFromID called for Trading locationID %u using itemID %u", m_locationID, args.arg1);
            flag = flagAutoFit;
        } break;
    }

    //we just bind up a new inventory object for container requested and give it back to them.
    InventoryBound* ib = new InventoryBound(m_manager, iRef, flag, ownerID, passive);
    PyRep* result = m_manager->BindObject(call.client, ib);

    // returns nodeid and timestamp
    return result;
}

//this is a view into an inventory item using a specific flag.
PyResult InvBrokerBound::Handle_GetInventory(PyCallArgs &call) {
    /** @note  this means "Get the Inventory containing this itemID */
    _log(INV__DUMP, "InvBrokerBound::Handle_GetInventory() size=%u", call.tuple->size());
    call.Dump(INV__DUMP);
    Inventory_GetInventory args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    uint32 ownerID = args.ownerID;
    sItemFactory.SetUsingClient( call.client );
    InventoryItemRef item;
    if (m_groupID == EVEDB::invGroups::Station) {
        _log(INV__WARNING, "GetInventory called for station %u", m_locationID);
        //item = sEntityList.GetStationByID(m_locationID);
        item = sItemFactory./*GetStation*/GetItem(m_locationID);
    } else if (m_groupID == EVEDB::invGroups::Solar_System) {
        _log(INV__WARNING, "GetInventory called for solar system %u", m_locationID);
        item = sItemFactory./*GetSolarSystem*/GetItem(m_locationID);
    } else {
        _log(INV__WARNING, "GetInventory called for item %u (group: %u)", m_locationID, m_groupID);
        item = sItemFactory./*GetInventoryItemFromID*/GetItem(m_locationID);
    }
    sItemFactory.UnsetUsingClient();

    if (item.get() == nullptr) {
        codelog(INV__ERROR, "%s: Unable to load item %u for flag %u", call.client->GetName(), m_locationID, args.container);
        return nullptr;
    }

    EVEItemFlags flag = flagAutoFit;
    switch(args.container) {
        case Inv::Container::Wallet: { /*10001*/
            if (ownerID == 0)
                ownerID = call.client->GetCharacterID();
            flag = flagWallet;
        } break;
        case Inv::Container::Character: { /*10011*/
            if (ownerID == 0)
                ownerID = call.client->GetCharacterID();
            flag = flagSkill;
        } break;
        case Inv::Container::Hangar: { /*10004*/
            if (ownerID == 0)
                ownerID = call.client->GetCharacterID();
            flag = flagHangar;
        } break;
        case Inv::Container::CorpMarket: { /*10012*/   //this is for corp deliveries
            if (ownerID == 0)
                ownerID = call.client->GetCorporationID();
            flag = flagCorpMarket;
        } break;
        case Inv::Container::Offices: { /*10009*/
            if (ownerID == 0)
                ownerID = call.client->GetCorporationID();
            flag = flagOffice;
        } break;
        case Inv::Container::Factory: { /*10006*/
            // not sure on this one  (not coded in client)
            if (ownerID == 0)
                ownerID = call.client->GetCharacterID();
            flag = flagFactory;
        } break;
        case Inv::Container::SolarSystem: { /*10003*/ // is this for flagProperty items?  (corp items in space)
            // not sure on this one
            if (ownerID == 0)
                ownerID = call.client->GetCorporationID();
            flag = flagProperty;
        } break;

        case Inv::Container::Global: { /*10002*/    // used in asset listings.  means "everywhere"
            // not sure how to code it yet...
            if (ownerID == 0)
                ownerID = call.client->GetCharacterID();
            flag = flagAutoFit;
        } break;

        //case Inv::Container::ScrapHeap:/*10005*/
        //case Inv::Container::Bank:/*10007*/
        //case Inv::Container::Recycler:/*10008*/
        //case Inv::Container::StationCharacters:/*10010*/
            //flag = flagNone;
            //break;
        // there is no 10005, 10006, or 10007 defined in client
        default:
            _log(INV__ERROR, "Unhandled container type %u for locationID %u", args.container, m_locationID);
            return nullptr;
    }

    //we just bind up a new inventory object for container requested and give it back to them.
    InventoryBound* ib = new InventoryBound(m_manager, item, flag, ownerID, false);
    PyRep* result = m_manager->BindObject(call.client, ib);

    // returns nodeid and timestamp
    return result;
}

// this cannot throw, returns nothing.
PyResult InvBrokerBound::Handle_SetLabel(PyCallArgs &call) {
    CallSetLabel args;
    if (!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    sItemFactory.SetUsingClient( call.client );
    InventoryItemRef iRef = sItemFactory.GetItem( args.itemID );
    if (iRef.get() == nullptr) {
        codelog(INV__ERROR, "%s: Unable to load item %u", call.client->GetName(), args.itemID);
        return nullptr;
    }

    /*
     * {'FullPath': u'UI/Messages', 'messageID': 258478, 'label': u'SetNameInvalidBody'}(u"You can't rename that type of object.", None, None)
     * {'FullPath': u'UI/Messages', 'messageID': 258479, 'label': u'SetNameShipMustBePilotBody'}(u'You can only rename ships that you are currently piloting.', None, None)
     */

    /** @todo if owner is corp, make sure char has permissions to rename corp items  */
    if (IsPlayerCorp(iRef->ownerID())) {
        if (iRef->ownerID() != call.client->GetCorporationID()) {
            _log(INV__ERROR, "%u(%u) tried to rename CorpItem %s(%u) owned by %u.", \
                    call.client->GetName(), call.client->GetCharacterID(), iRef->itemName().c_str(), \
                    iRef->itemID(), iRef->ownerID());
            call.client->SendErrorMsg("You are not allowed to rename that.");
            return nullptr;
        }
    } else if (IsCharacter(iRef->ownerID())) {
        if (iRef->ownerID() != call.client->GetCharacterID()) {
            _log(INV__ERROR, "%u(%u) tried to rename PlayerItem %s(%u) owned by %u.", \
                    call.client->GetName(), call.client->GetCharacterID(), iRef->itemName().c_str(), \
                    iRef->itemID(), iRef->ownerID());
            call.client->SendErrorMsg("You are not allowed to rename that.");
            return nullptr;
        }
    } else {
        // error here....
        call.client->SendErrorMsg("You are not allowed to rename that.");
        return nullptr;
    }

    iRef->Rename(PyRep::StringContent(args.itemName));

    // Release the ItemFactory
    sItemFactory.UnsetUsingClient();

    //OnItemNameChange
    // need to also check pos rename code.
    //  this needs investigating to verify

    return nullptr;
}

PyResult InvBrokerBound::Handle_TrashItems(PyCallArgs &call) {
    Call_TrashItems args;
    if(!args.Decode(&call.tuple)) {
        codelog(SERVICE__ERROR, "%s: Failed to decode arguments.", GetName());
        return nullptr;
    }

    std::vector<int32>::const_iterator cur = args.items.begin(), end = args.items.end();
    for(; cur != end; ++cur) {
        InventoryItemRef item = sItemFactory.GetItem( *cur );
        if (item.get() == nullptr) {
            _log(INV__ERROR, "%s: Unable to load item %u to delete it. Skipping.", call.client->GetName(), *cur);
        } else if (call.client->GetCharacterID() != item->ownerID()) {
            _log(INV__ERROR, "%s: Tried to trash item %u which is not yours. Skipping.", call.client->GetName(), *cur);
        } else if (item->locationID() != args.locationID) {
            _log(INV__ERROR, "%s: Item %u is not in location %u. Skipping.", call.client->GetName(), *cur, args.locationID);
        } else {
            item->Delete();
            // these below dont work...
            //item->SetFlag(flagJunkyardTrashed);
            //item->ChangeOwner(call.client->GetStationID(), false);
            //item->Move(call.client->GetStationID(), flagJunkyardTrashed);
        }
    }

    return nullptr;
}

/**     ***********************************************************************
 * @note   these do absolutely nothing at this time....
 */

PyResult InvBrokerBound::Handle_List(PyCallArgs &call) {
/**
        inv = invCache.GetInventoryFromId(activeShipID, locationID=session.stationid2)
        shipCargo = inv.List()

        inv = sm.GetService('invCache').GetInventoryFromId(self.containerID)
        for item in inv.List(const.flagSpecializedMaterialBay)  <-- for customs gantry upgrade

        shipInv = sm.GetService('invCache').GetInventoryFromId(session.shipid)
        invList = shipInv.List(const.flagSpecializedCommandCenterHold)
        invList.extend(shipInv.List(const.flagCargo))
            */

    sLog.Error( "InvBrokerBound::Handle_List()", "size= %u", call.tuple->size() );
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

    sLog.Warning( "InvBrokerBound::Handle_AssembleCargoContainer()", "size= %u", call.tuple->size() );
    call.Dump(INV__DUMP);

    return nullptr;
}

PyResult InvBrokerBound::Handle_BreakPlasticWrap(PyCallArgs &call) {
    // ConfirmBreakCourierPackage   - this is for courier contracts
    sLog.Warning( "InvBrokerBound::Handle_BreakPlasticWrap()", "size= %u", call.tuple->size() );
    call.Dump(INV__DUMP);

    return nullptr;
}

PyResult InvBrokerBound::Handle_TakeOutTrash(PyCallArgs &call) {
    //self.invCache.GetInventory(const.containerHangar).TakeOutTrash([ invItem.itemID for invItem in invItems ])
    sLog.Warning( "InvBrokerBound::Handle_TakeOutTrash()", "size= %u", call.tuple->size() );
    call.Dump(INV__DUMP);

    return nullptr;
}

