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
    Author:     Aknor Jaden
    Updates:    Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "EVEServerConfig.h"
#include "ship/DestinyManager.h"
#include "system/Container.h"
#include "SystemManager.h"

/*
 * CargoContainer
 */
CargoContainer::CargoContainer(
    ItemFactory &_factory,
    uint32 _containerID,
    // InventoryItem stuff:
    const ItemType &_containerType,
    const ItemData &_data)
: InventoryItem(_factory, _containerID, _containerType, _data)
{
    m_inventory = new Inventory(InventoryItemRef(this));
    _log(ITEM__TRACE, "Created CargoContainer object for item %s (%u).", itemName().c_str(), itemID());
}

CargoContainerRef CargoContainer::Load(ItemFactory &factory, uint32 containerID)
{
    return InventoryItem::Load<CargoContainer>( factory, containerID );
}

bool CargoContainer::_Load() {
    if (!m_inventory->LoadContents( &m_factory ) )
        return false;

    return InventoryItem::_Load();
}

template<class _Ty>
RefPtr<_Ty> CargoContainer::_LoadCargoContainer(ItemFactory &factory, uint32 containerID,
    // InventoryItem stuff:
    const ItemType &itemType, const ItemData &data)
{
    // we don't need any additional stuff
    return CargoContainerRef( new CargoContainer( factory, containerID, itemType, data ) );
}

CargoContainerRef CargoContainer::Spawn(ItemFactory &factory, ItemData &data) {
    uint32 containerID = CargoContainer::CreateItemID( factory, data );
    if (containerID == 0 )
        return CargoContainerRef();
    CargoContainerRef containerRef = CargoContainer::Load( factory, containerID );

    // Create default dynamic attributes in the AttributeMap:
    containerRef->SetAttribute(AttrRadius,          containerRef->type().radius(), true);			// Radius

    // Check for existence of some attributes that may or may not have already been loaded and set them
    // to default values:
	// Hull Damage
	if (!(containerRef->HasAttribute(AttrDamage)) )
        containerRef->SetAttribute(AttrDamage, 0, true );

	return containerRef;
}

uint32 CargoContainer::CreateItemID(ItemFactory &factory, ItemData &data) {
    // make sure it's a cargo container
    const ItemType *st = factory.GetType(data.typeID);
    if(st == NULL)
        return 0;

    // store item data
    uint32 containerID = InventoryItem::CreateItemID(factory, data);
    if(containerID == 0)
        return 0;

    // nothing additional

    return containerID;
}

void CargoContainer::Delete()
{
    sLog.Magenta( "CargoContainer::Delete()", "Garbage Collection is removing Cargo Container %u.", itemID() );
    // delete contents first
    m_inventory->DeleteContents( m_factory );
    InventoryItem::Delete();
}

double CargoContainer::GetCapacity(EVEItemFlags flag) const
{
    switch( flag ) {
        case flagAutoFit:
        case flagCargoHold:
            return GetAttribute(AttrCapacity).get_float();
        default:
            return 0.0;
    }
}

void CargoContainer::ValidateAddItem(EVEItemFlags flag, InventoryItemRef item) const {
    if (flag == flagCargoHold )  {
        EvilNumber capacityUsed(0);
        std::vector<InventoryItemRef> items;
        m_inventory->FindByFlag(flag, items);
        for (auto cur : items)
            capacityUsed += cur->GetAttribute(AttrVolume);
        capacityUsed += item->GetAttribute(AttrVolume);
        if (capacityUsed > GetAttribute(AttrCapacity) )
            ; /** @todo make error msg here */  //  PyException( MakeCustomError( "Not enough cargo space!") );
    }
}

PyObject *CargoContainer::CargoContainerGetInfo() {
    if (!m_inventory->LoadContents( &m_factory ) ) {
        codelog( ITEM__ERROR, "%s (%u): Failed to load contents for CargoContainerGetInfo", itemName().c_str(), itemID() );
        return NULL;
    }

    Rsp_CommonGetInfo result;
    Rsp_CommonGetInfo_Entry entry;

    //first populate the CargoContainer.
    if (!Populate( entry ) )
        return NULL;    //print already done.

    result.items[ itemID() ] = entry.Encode();

    return result.Encode();
}

void CargoContainer::AddItem(InventoryItemRef item)
{
    m_inventory->AddItem( item );
}

void CargoContainer::RemoveItem(InventoryItemRef item)
{
    /** @todo  put check in here for container owner (if space container) and implement sec penalty */
    /* http://www.eveinfo.net/wiki/ind~4067.htm
     *  relative_sec_status_penalty = base_penalty * system_truesec * (1 + (victim_sec_status - agressor_sec_status) / 90)
     *  The actual drop in security status seen by the attacker is a function of their current security status and the relative penalty:
     *  security status loss = relative_penalty * (agressor_sec_status + 10)
     *
    double modifier = (1 + ((GetChar()->GetSecurityRating() - client->GetSecurityRating()) /90));
    double penalty = 6.0f * m_system->GetSystemSecurityRating() * modifier;
    double loss = penalty * (client->GetSecurityRating() + 10);
    client->GetChar()->secStatusChange( loss );
    */
    m_inventory->RemoveItem( item );
    if ((groupID() == EVEDB::invGroups::Cargo_Container) && (IsEmpty())) {
        sLog.Warning( "CargoContainer::RemoveItem()", "Cargo Container %u is empty and being deleted.", itemID() );
        Delete();
    }
}

void CargoContainer::MakeDamageState(DoDestinyDamageState &into) const
{
    //FIXME  container attributes here are NOT saved in the db....
    into.shield = 1.0;//(m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
    into.recharge = 10000;
    into.timestamp = Win32TimeNow();
    into.armor = 1.0;//1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
    into.structure = 1.0;
}


ContainerSE::ContainerSE(CargoContainerRef self, PyServiceMgr &services, SystemManager *system)
: ItemSystemEntity(self, services, system),
  m_deleteTimer(sConfig.rates.WorldDecay *60 *1000)
{
    _containerRef = self;
    if (!IsStation(m_self->locationID()))
        m_deleteTimer.Start();
    m_self->SetAttribute(AttrCapacity, m_self->type().capacity(), false);
}

void ContainerSE::Process() {
    //SystemEntity::Process();
    if (m_deleteTimer.Check(false)) {
        m_deleteTimer.Disable();
        m_system->RemoveEntity(this);
        _containerRef->Delete();
    }
}

void ContainerSE::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;

    BallHeader head;
    head.entityID = GetID();
        head.mode = DSTBALL_RIGID;
        head.radius = GetRadius();
        head.x = x();
        head.y = y();
        head.z = z();
        head.flags = IsMassive | IsInteractive;
    into.Append( head );
    MassSector mass;
        mass.mass = m_self->type().mass();
        mass.cloak = 0;
        mass.Harmonic = -1.0f;
        mass.corporationID = GetCorporationID();
        mass.allianceID = GetAllianceID();
    into.Append( mass );

    DSTBALL_RIGID_Struct rigid;
        rigid.formationID = 0xFF;
    into.Append( rigid );
    _log(COMMON__WARNING, "ContainerEntity::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void ContainerSE::MakeDamageState(DoDestinyDamageState &into)
{
    //FIXME  container attributes here are NOT saved in the db....
    into.shield = 1;
    into.recharge = 20000;
    into.timestamp = Win32TimeNow();
    into.armor = 1;
    into.structure = 1.0;
}

PyDict *ContainerSE::MakeSlimItem() {
    _log(COMMON__WARNING, "MakeSlimItem for ContainerEntity %s(%u)", m_self->itemName().c_str(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",       new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",       new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",      new PyInt(m_self->ownerID()));
        slim->SetItemString("categoryID",   new PyInt(m_self->categoryID()));
        slim->SetItemString("groupID",      new PyInt(m_self->groupID()));
        slim->SetItemString("name",         new PyString(m_self->itemName()));
        slim->SetItemString("corpID",       new PyInt(GetCorporationID()));
        slim->SetItemString("allianceID",   new PyInt(GetAllianceID()));
    return slim;
}

/*
 * WreckContainer
 */
WreckContainer::WreckContainer(
    ItemFactory &_factory,
    uint32 _containerID,
    // InventoryItem stuff:
    const ItemType &_containerType,
    const ItemData &_data)
: InventoryItem(_factory, _containerID, _containerType, _data)
{
    m_inventory = new Inventory(InventoryItemRef(this));
    _log(ITEM__TRACE, "Created WreckContainer object for item %s (%u).", itemName().c_str(), itemID());
}

WreckContainerRef WreckContainer::Load(ItemFactory &factory, uint32 containerID)
{
    return InventoryItem::Load<WreckContainer>( factory, containerID );
}

bool WreckContainer::_Load() {
    if (!m_inventory->LoadContents( &m_factory ) )
        return false;

    return InventoryItem::_Load();
}

template<class _Ty>
RefPtr<_Ty> WreckContainer::_LoadWreck(ItemFactory &factory, uint32 containerID, const ItemType &itemType, const ItemData &data) {
    // we don't need any additional stuff
    return WreckContainerRef( new WreckContainer( factory, containerID, itemType, data ) );
}

WreckContainerRef WreckContainer::Spawn(ItemFactory &factory, ItemData &data) {
    uint32 containerID = WreckContainer::CreateItemID( factory, data );
    if (containerID == 0 )
        return WreckContainerRef();
    WreckContainerRef containerRef = WreckContainer::Load( factory, containerID );

    // Create default dynamic attributes in the AttributeMap:
    containerRef->SetAttribute(AttrRadius,          containerRef->type().radius(), true);           // Radius

    return containerRef;
}

uint32 WreckContainer::CreateItemID(ItemFactory &factory, ItemData &data) {
    // make sure it's a wreck
    const ItemType *st = factory.GetType(data.typeID);
    if(st == NULL)
        return 0;

    // store item data
    uint32 containerID = InventoryItem::CreateItemID(factory, data);
    if(containerID == 0)
        return 0;

    // nothing additional

    return containerID;
}

void WreckContainer::Delete()
{
    sLog.Magenta( "WreckContainer::Delete()", "Garbage Collection is removing Wreck %u.", itemID() );
    // delete contents first
    m_inventory->DeleteContents( m_factory );
    InventoryItem::Delete();
}

double WreckContainer::GetCapacity(EVEItemFlags flag) const
{
    return GetAttribute(AttrCapacity).get_float();
}

PyObject *WreckContainer::WreckContainerGetInfo()
{
    if (!m_inventory->LoadContents( &m_factory ) )
    {
        codelog( ITEM__ERROR, "%s (%u): Failed to load contents for WreckContainerGetInfo", itemName().c_str(), itemID() );
        return NULL;
    }

    Rsp_CommonGetInfo result;
    Rsp_CommonGetInfo_Entry entry;

    //first populate the WreckContainer.
    if (!Populate( entry ) )
        return NULL;    //print already done.

        result.items[ itemID() ] = entry.Encode();

    return result.Encode();
}

void WreckContainer::ValidateAddItem( EVEItemFlags flag, InventoryItemRef item ) const
{
        //  no code here.  should NOT be able to add items to a wreck contaier.
}

void WreckContainer::AddItem( InventoryItemRef item )
{
    m_inventory->AddItem( item );
}

void WreckContainer::RemoveItem(InventoryItemRef item)
{
    m_inventory->RemoveItem( item );
    double curCapy = GetAttribute(AttrCapacity).get_float();
    double defCapy = GetDefaultAttribute(AttrCapacity).get_float();
    _log(COMMON__WARNING, "WreckContainer::IsEmpty(): %s(%u) - attrib capy: %d, default capy: %d", itemName().c_str(), itemID(), curCapy, defCapy );
    if (IsEmpty())
        MakeSlimItemChange();
}

void WreckContainer::MakeSlimItemChange()
{
    /** @todo (Allan)  finish this  */
    //  this is used to update all clients in bubble for container empty status (the filled/open icon in overview)
    //new PyObject( "foo.SlimItem", MakeSlimItem() );
}


WreckSE::WreckSE(WreckContainerRef self, PyServiceMgr &services, SystemManager* system)
: ItemSystemEntity(self, services, system),
 m_deleteTimer(sConfig.rates.WorldDecay *60 *1000)
{
    _containerRef = self;
    m_deleteTimer.Start();
    m_self->SetAttribute(AttrCapacity, m_self->type().capacity());
}

void WreckSE::Process() {
    //SystemEntity::Process();
    if (m_deleteTimer.Check(false)) {
        m_deleteTimer.Disable();
        m_system->RemoveEntity(this);
        _containerRef->Delete();
    }
}

void WreckSE::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;

    BallHeader head;
        head.entityID = GetID();
        head.mode = DSTBALL_TROLL;
        head.radius = GetRadius();
        head.x = x();
        head.y = y();
        head.z = z();
        head.flags = IsInteractive;
    into.Append( head );
    MassSector mass;
        mass.mass = m_self->type().mass();
        mass.cloak = 0;
        mass.Harmonic = -1.0f;
        mass.corporationID = GetCorporationID();
        mass.allianceID = GetAllianceID();
    into.Append( mass );

    DSTBALL_TROLL_Struct troll;
        troll.formationID = 0xFF;
        troll.effectStamp = sEntityList.GetStamp();
    into.Append( troll );
    _log(COMMON__WARNING, "WreckEntity::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void WreckSE::MakeWreckState(DoDestinyDamageState3 &into)
{
    into.shield = 0;
    into.armor = 0;
    into.structure = 1.0;
}


PyDict *WreckSE::MakeSlimItem() {
    _log(COMMON__WARNING, "MakeSlimItem for WreckEntity %s(%u)", m_self->itemName().c_str(), m_self->itemID());
// NOTE  commented items i havent figured out yet...  -allan 9Dec15
    PyTuple* nameID = new PyTuple(2);
        nameID->SetItem(0,  new PyString("UI/Inflight/WreckNameShipName"));
    PyDict* shipName = new PyDict;
        shipName->SetItem("shipName", new PyInt(0));
        nameID->SetItem(1, shipName);
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",           new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",           new PyInt(m_self->typeID()));
        slim->SetItemString("name",             new PyString(m_self->itemName()));
        //slim->SetItemString("lootRights",       new PyNone);
        slim->SetItemString("corpID",           new PyInt(GetCorporationID()));
        slim->SetItemString("allianceID",       new PyLong(GetAllianceID()));
        slim->SetItemString("isEmpty",          new PyBool(IsEmpty()));
        slim->SetItemString("launcherID",       new PyLong(m_launchedByID));
        slim->SetItemString("securityStatus",   new PyInt(0));  //FIXME TODO
        slim->SetItemString("ownerID",          new PyInt(m_self->ownerID()));
        PyDict* dict = new PyDict;
            dict->SetItemString("WreckTypeID",  new PyInt(m_self->typeID()));
        PyTuple* tuple = new PyTuple(2);
            tuple->SetItem(0, new PyString("UI/Inflight/WreckNameTypeID"));
            tuple->SetItem(1, dict);
        slim->SetItemString("nameID",           tuple);
        slim->SetItemString("warFactionID",     new PyInt(GetWarFactionID()));

    return slim;
}
/*
                                        [PyString "itemID"]
                                        [PyIntegerVar 9000000000001190976]
                                        [PyString "typeID"]
                                        [PyInt 26574]
                                        [PyString "name"]
                                        [PyString "Guristas Inferno Wreck"]
                                        [PyString "corpID"]
                                        [PyInt 1630077495]
                                        [PyString "allianceID"]
                                        [PyInt 99001691]
                                        [PyString "isEmpty"]
                                        [PyBool False]
                                        [PyString "launcherID"]
                                        [PyIntegerVar 9000000000001190095]
                                        [PyString "securityStatus"]
                                        [PyFloat 2.65811580082965]
                                        [PyString "ownerID"]
                                        [PyInt 649670823]
                                        [PyString "nameID"]
                                        [PyTuple 2 items]
                                          [PyString "UI/Inflight/WreckNameTypeID"]
                                          [PyDict 1 kvp]
                                            [PyString "WreckTypeID"]
                                            [PyInt 11931]
                                        [PyString "warFactionID"]
                                        [PyNone]
                            */
