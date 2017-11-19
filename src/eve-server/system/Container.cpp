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
#include "EntityList.h"
#include "EVEServerConfig.h"
#include "planet/PlanetDB.h"
#include "system/DestinyManager.h"
#include "system/Container.h"
#include "SystemManager.h"
#include "SystemBubble.h"
#include "cosmicMgrs/SpawnMgr.h"

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
    m_isAnchored = false;
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

CargoContainerRef CargoContainer::Spawn(ItemFactory &factory, ItemData &data) {
    uint32 containerID = CargoContainer::CreateItemID( factory, data );
    if (containerID == 0 )
        return CargoContainerRef();
    CargoContainerRef containerRef = CargoContainer::Load( factory, containerID );

    // Create default dynamic attributes in the AttributeMap:
    containerRef->SetAttribute(AttrRadius,          containerRef->type().radius());			// Radius

    // Check for existence of some attributes that may or may not have already been loaded and set them
    // to default values:
	// Hull Damage
	if (!(containerRef->HasAttribute(AttrDamage)) )
        containerRef->SetAttribute(AttrDamage, 0);

	return containerRef;
}

uint32 CargoContainer::CreateItemID(ItemFactory &factory, ItemData &data)
{
    return InventoryItem::CreateItemID(factory, data);
}

void CargoContainer::Delete()
{
    if (typeID() == EVEDB::invTypes::typePlanetaryLaunchContainer) {
        PlanetDB mDB;
        mDB.DeleteLaunch(itemID());
    }

    mySE->Delete();
    m_inventory->LoadContents(&m_factory);
    // delete contents first
    m_inventory->DeleteContents();
    InventoryItem::Delete();
}

double CargoContainer::GetCapacity(EVEItemFlags flag) const
{
    switch( flag ) {
        case flagAutoFit:
        case flagCargoHold:
            return GetAttribute(AttrCapacity).get_double();
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
    if (m_isAnchored)
        return;
   // if (typeID() == EVEDB::invTypes::typeCargoContainer)
    if (m_inventory->IsEmpty()) {
        sLog.Warning( "CargoContainer::RemoveItem()", "Cargo Container %u is empty and being deleted.", itemID() );
        Delete();
    }
}

void CargoContainer::MakeDamageState(DoDestinyDamageState &into) const
{
    //FIXME  container attributes here are NOT saved in the db....
    into.shield = 1.0;//(m_self->GetAttribute(AttrShieldCharge).get_double() / m_self->GetAttribute(AttrShieldCapacity).get_double());
    into.recharge = 10000;
    into.timestamp = Win32TimeNow();
    into.armor = 1.0;//1.0 - (m_self->GetAttribute(AttrArmorDamage).get_double() / m_self->GetAttribute(AttrArmorHP).get_double());
    into.structure = 1.0;
}


ContainerSE::ContainerSE(CargoContainerRef self, PyServiceMgr& services, SystemManager* system, const FactionData& data)
: ItemSystemEntity(self, services, system),
  m_deleteTimer(sConfig.rates.WorldDecay *60 *1000)
{
    m_warID = data.factionID;
    m_allyID = data.allianceID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;
    m_destiny = new DestinyManager(this);

    m_contRef = self;
    if (!IsStation(m_self->locationID())) {
        if (m_self->typeID() == EVEDB::invTypes::typePlanetaryLaunchContainer)
            m_deleteTimer.Start(5 *24 *60 *60 *1000);  //5d timer for PI launch.  should probably get this saved value from planet launches
        else
            m_deleteTimer.Start();
    }

    m_self->SetAttribute(AttrCapacity, m_self->type().capacity(), false);
}

ContainerSE::~ContainerSE()
{
    SafeDelete(m_destiny);
}

void ContainerSE::Process() {
    /*  Enable base call to Process Targeting and Movement  */
    SystemEntity::Process();
    if (m_deleteTimer.Check(false)) {
        m_deleteTimer.Disable();
        sLog.Magenta( "ContainerSE::Process()", "Garbage Collection is removing Cargo Container %u.", m_contRef->itemID() );
        m_contRef->Delete();
    }
}

void ContainerSE::Delete()
{
    m_system->RemoveEntity(this);
    SystemEntity::Delete();
}

void ContainerSE::Activate(int32 effectID)
{
    // check effectID, check current state, check current timer, set new state, update timer

    /** @todo somehow notify client with one of these effects:
     *  effectAnchorDrop = 649
     *  effectAnchorLift = 650
     *  effectAnchorDropForStructures = 1022
     *  effectAnchorLiftForStructures = 1023
     *
     ** @todo  many more effects to send for.....look into later.
     * effectOnlineForStructures = 901
     *
     ** @note  also note there are timers involved here...
     */
}

void ContainerSE::Deactivate(int32 effectID)
{
    // check effectID, check current state, check current timer, set new state, update timer

}

void ContainerSE::AnchorContainer()
{
    m_deleteTimer.Disable();
    m_contRef->SetAnchor(true);
}

void ContainerSE::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;

    BallHeader head;
        head.entityID = GetID();
        head.radius = GetRadius();
        head.x = x();
        head.y = y();
        head.z = z();
        head.mode = DSTBALL_TROLL;
        head.flags = IsInteractive;
    into.Append( head );
    MassSector mass;
        mass.mass = m_self->type().mass();
        mass.cloak = 0;
        mass.harmonic = m_harmonic;
        mass.corporationID = m_corpID;
        mass.allianceID = m_allyID;
    into.Append( mass );
    DSTBALL_TROLL_Struct troll;
        troll.formationID = 0xFF;
        troll.effectStamp = sEntityList.GetStamp();
    into.Append( troll );

    _log(SE__DESTINY, "ContainerSE::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void ContainerSE::MakeDamageState(DoDestinyDamageState &into)
{
    //FIXME  container attributes are NOT saved in the db....
    into.shield = 1;
    into.recharge = 2000000;
    into.timestamp = Win32TimeNow();
    into.armor = 1;
    into.structure = 1;
}

PyDict *ContainerSE::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for ContainerSE %s(%u)", m_self->itemName().c_str(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",       new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",       new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",      new PyInt(m_self->ownerID()));
        slim->SetItemString("name",         new PyString(m_self->itemName()));
        slim->SetItemString("nameID",       new PyNone());
        slim->SetItemString("corpID",       new PyInt(m_corpID));
        slim->SetItemString("allianceID",   new PyInt(m_allyID));
        slim->SetItemString("warFactionID", new PyInt(m_warID));
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

WreckContainerRef WreckContainer::Spawn(ItemFactory &factory, ItemData &data) {
    uint32 containerID = WreckContainer::CreateItemID( factory, data );
    if (containerID == 0 )
        return WreckContainerRef();
    WreckContainerRef containerRef = WreckContainer::Load( factory, containerID );

    // Create default dynamic attributes in the AttributeMap:
    containerRef->SetAttribute(AttrRadius,          containerRef->type().radius(), true);           // Radius

    return containerRef;
}

uint32 WreckContainer::CreateItemID(ItemFactory &factory, ItemData &data)
{
    return InventoryItem::CreateItemID(factory, data);
}

void WreckContainer::Delete()
{
    mySE->Delete();
    m_inventory->LoadContents(&m_factory);
    // delete contents first
    m_inventory->DeleteContents();
    InventoryItem::Delete();
}

double WreckContainer::GetCapacity(EVEItemFlags flag) const
{
    return GetAttribute(AttrCapacity).get_double();
}

PyObject *WreckContainer::WreckContainerGetInfo()
{
    if (!m_inventory->LoadContents(&m_factory)) {
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
    if (IsEmpty()) {
        MakeSlimItemChange();
        _log(INV__INFO, "WreckContainer::IsEmpty() for %s(%u)", itemName().c_str(), itemID());
        //Delete();
    }
}

void WreckContainer::MakeSlimItemChange()
{
    PyDict* slimPod = mySE->MakeSlimItem();
    PyTuple* shipData = new PyTuple(2);
        shipData->SetItem(0, new PyLong(itemID()));
        shipData->SetItem(1, new PyObject( "foo.SlimItem", slimPod));
    PyTuple* updates = new PyTuple(2);
        updates->SetItem(0, new PyString("OnSlimItemChange"));
        updates->SetItem(1, shipData);
    //consumes updates
    mySE->SysBubble()->BubblecastDestinyUpdate(&updates, "destiny" );
}


WreckSE::WreckSE(WreckContainerRef self, PyServiceMgr &services, SystemManager* system, const FactionData &data)
: ItemSystemEntity(self, services, system),
 m_deleteTimer(sConfig.rates.WorldDecay *60 *1000)
{
    m_warID = data.factionID;
    m_allyID = data.allianceID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;
    m_destiny = new DestinyManager(this);

    m_abandoned = false;
    m_contRef = self;
    m_deleteTimer.Start();
    m_self->SetAttribute(AttrCapacity, m_self->type().capacity());
}

WreckSE::~WreckSE()
{
    SafeDelete(m_destiny);
}

void WreckSE::Process() {
    /*  Enable base call to Process Targeting and Movement  */
    SystemEntity::Process();
    if (m_deleteTimer.Check(false)) {
        m_deleteTimer.Disable();
        sLog.Magenta( "WreckSE::Process()", "Garbage Collection is removing Wreck %u.", m_contRef->itemID() );
        m_contRef->Delete();
    }
}

void WreckSE::Delete()
{
    m_system->RemoveEntity(this);
    SystemEntity::Delete();
}

void WreckSE::Abandon()
{
    SystemEntity::Abandon();
    m_abandoned = true;
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
        mass.harmonic = m_harmonic;
        mass.corporationID = m_corpID;
        mass.allianceID = m_allyID;
    into.Append( mass );

    DSTBALL_TROLL_Struct troll;
        troll.formationID = 0xFF;
        troll.effectStamp = sEntityList.GetStamp();
    into.Append( troll );
    _log(SE__DESTINY, "WreckSE::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

PyDict *WreckSE::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for WreckSE %s(%u)", m_self->itemName().c_str(), m_self->itemID());
    PyTuple* nameID = new PyTuple(2);
        nameID->SetItem(0,  new PyString("UI/Inflight/WreckNameShipName"));
    PyDict* shipName = new PyDict;
        shipName->SetItem("shipName", new PyInt(0));
        nameID->SetItem(1, shipName);
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",           new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",           new PyInt(m_self->typeID()));
        slim->SetItemString("name",             new PyString(m_self->itemName()));
        /* this needs more work.... */
        if (m_abandoned or (m_fleetID != 0)) { // this is ONLY for abandoned wrecks or wrecks from fleet ops
            PyTuple* tuple1 = new PyTuple(4);
                tuple1->SetItem(0,              new PyInt(m_self->ownerID()));
                tuple1->SetItem(1,              new PyInt(m_corpID));
                tuple1->SetItem(2,              new PyNone()); //PyInt(m_fleetID));
                tuple1->SetItem(3,              new PyBool(false));
            slim->SetItemString("lootRights",   tuple1);
        }
        slim->SetItemString("corpID",           new PyInt(m_corpID));
        slim->SetItemString("allianceID",       new PyInt(m_allyID));
        slim->SetItemString("isEmpty",          new PyBool(m_contRef->IsEmpty()));
        slim->SetItemString("launcherID",       new PyLong(m_launchedByID));
        slim->SetItemString("securityStatus",   new PyInt(0));  //FIXME TODO
        slim->SetItemString("ownerID",          new PyInt(m_self->ownerID()));
        PyDict* dict = new PyDict;
            dict->SetItemString("WreckTypeID",  new PyInt(m_self->typeID()));
        PyTuple* tuple2 = new PyTuple(2);
            tuple2->SetItem(0, new PyString("UI/Inflight/WreckNameTypeID"));
            tuple2->SetItem(1, dict);
        slim->SetItemString("nameID",           tuple2);
        slim->SetItemString("warFactionID",     new PyInt(m_warID));

    if (is_log_enabled(DESTINY__DEBUG)) {
        _log( DESTINY__DEBUG, "WreckSE::MakeSlimItem()", "%s(%u)", GetName(), GetID());
        slim->Dump(DESTINY__DEBUG, "     ");
    }

    return slim;
}
