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
    Author:        Bloody.Rabbit
*/

#include "eve-server.h"

#include "station/Station.h"
#include "system/DestinyManager.h"
#include "system/SystemEntity.h"
#include "system/SystemManager.h"

/*
 * StationTypeData
 */
StationTypeData::StationTypeData(
    uint32 _dockingBayGraphicID,
    uint32 _hangarGraphicID,
    const GPoint &_dockEntry,
    const GVector &_dockOrientation,
    uint32 _operationID,
    uint32 _officeSlots,
    double _reprocessingEfficiency,
    bool _conquerable)
: dockingBayGraphicID(_dockingBayGraphicID),
  hangarGraphicID(_hangarGraphicID),
  dockEntry(_dockEntry),
  dockOrientation(_dockOrientation),
  operationID(_operationID),
  officeSlots(_officeSlots),
  reprocessingEfficiency(_reprocessingEfficiency),
  conquerable(_conquerable)
{
}

/*
 * StationType
 */
StationType::StationType(
    uint32 _id,
    // ItemType stuff:
    const ItemGroup &_group,
    const TypeData &_data,
    // StationType stuff:
    const StationTypeData &_stData)
: ItemType(_id, _group, _data),
  m_dockingBayGraphicID(_stData.dockingBayGraphicID),
  m_hangarGraphicID(_stData.hangarGraphicID),
  m_dockEntry(_stData.dockEntry),
  m_dockOrientation(_stData.dockOrientation),
  m_operationID(_stData.operationID),
  m_officeSlots(_stData.officeSlots),
  m_reprocessingEfficiency(_stData.reprocessingEfficiency),
  m_conquerable(_stData.conquerable)
{
    // consistency check
    assert(_data.groupID == EVEDB::invGroups::Station);
}

StationType *StationType::Load(ItemFactory &factory, uint32 stationTypeID)
{
    return ItemType::Load<StationType>( factory, stationTypeID );
}

/*
 * StationInfo
 */
StationInfo::StationInfo(
    uint32 _security,
    double _dockingCostPerVolume,
    double _maxShipVolumeDockable,
    uint32 _officeRentalCost,
    uint32 _operationID,
    double _reprocessingEfficiency,
    double _reprocessingStationsTake,
    EVEItemFlags _reprocessingHangarFlag)
: security(_security),
  dockingCostPerVolume(_dockingCostPerVolume),
  maxShipVolumeDockable(_maxShipVolumeDockable),
  officeRentalCost(_officeRentalCost),
  operationID(_operationID),
  reprocessingEfficiency(_reprocessingEfficiency),
  reprocessingStationsTake(_reprocessingStationsTake),
  reprocessingHangarFlag(_reprocessingHangarFlag)
{
}

/*
 * Station
 */
StationItem::StationItem(
    ItemFactory &_factory,
    uint32 _stationID,
    // InventoryItem stuff:
    const StationType &_type,
    const ItemData &_data,
    // CelestialObject stuff:
    const CelestialObjectData &_cData,
    // Station stuff:
    const StationInfo &_stData)
: CelestialObject(_factory, _stationID, _type, _data, _cData),
m_stationType(_type),
m_security(_stData.security),
m_dockingCostPerVolume(_stData.dockingCostPerVolume),
m_maxShipVolumeDockable(_stData.maxShipVolumeDockable),
m_officeRentalCost(_stData.officeRentalCost),
m_operationID(_stData.operationID),
m_reprocessingEfficiency(_stData.reprocessingEfficiency),
m_reprocessingStationsTake(_stData.reprocessingStationsTake),
m_reprocessingHangarFlag(_stData.reprocessingHangarFlag)
{
    m_inventory = new Inventory(InventoryItemRef(this));
    _log(ITEM__TRACE, "Created Station for item %s (%u).", itemName().c_str(), itemID());
}

StationItemRef StationItem::Load(ItemFactory &factory, uint32 stationID)
{
    return InventoryItem::Load<StationItem>( factory, stationID );
}

bool StationItem::_Load() {
    if( !m_inventory->LoadContents( &m_factory ) )
        return false;

    return CelestialObject::_Load();
}

uint32 StationItem::CreateItemID(ItemFactory &factory, ItemData &data) {
    return InventoryItem::CreateItemID(factory, data);
}

StationSE::StationSE(StationItemRef station, PyServiceMgr &services, SystemManager* system)
: StaticSystemEntity(station, services, system)
{
    // Create default dynamic attributes in the AttributeMap:
    station->SetAttribute(AttrIsOnline,         1);
    station->SetAttribute(AttrCapacity,         STATION_HANGAR_MAX_CAPACITY);
    station->SetAttribute(AttrDamage,           0.0);
    station->SetAttribute(AttrShieldCapacity,   20000000.0);
    station->SetAttribute(AttrShieldCharge,     station->GetAttribute(AttrShieldCapacity));
    station->SetAttribute(AttrArmorHP,          station->GetAttribute(AttrArmorHP));
    station->SetAttribute(AttrArmorUniformity,  station->GetAttribute(AttrArmorUniformity));
    station->SetAttribute(AttrArmorDamage,      0.0);
    station->SetAttribute(AttrMass,             station->type().mass());
    station->SetAttribute(AttrRadius,           station->type().radius());
    station->SetAttribute(AttrVolume,           station->type().volume());
}

void StationSE::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;

    BallHeader head;
    head.entityID = m_self->itemID();
        head.mode = DSTBALL_RIGID;
        head.radius = GetRadius();
        head.x = x();
        head.y = y();
        head.z = z();
        head.flags = /*HasMiniBalls |*/ IsGlobal;
    into.Append( head );

    DSTBALL_RIGID_Struct main;
        main.formationID = 0xFF;
    into.Append( main );

/** @todo miniballs is broken and needs work...
 *  dont know what's wrong at this point, but client freaks out and ignores ANY ball data (in SetState) after this.
 * this causes BallNotInPark error with multiple stations, or ANY data sent AFTER first StationBall
    MiniBall miniball;
        miniball.x = -7701.181;
        miniball.y = 8060.06;
        miniball.z = 27878.900;
        miniball.radius = 1639.241;
    into.Append( miniball );
 */
    _log(SE__DESTINY, "StationSE::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

PyDict *StationSE::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for StationSE %s(%u)", m_self->itemName().c_str(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("groupID",          new PyInt(m_self->groupID()));
        slim->SetItemString("name",             new PyString(m_self->itemName()));
        slim->SetItemString("corpID",           new PyInt(m_self->ownerID()));
        slim->SetItemString("allianceID",       new PyInt(m_allyID));
        slim->SetItemString("typeID",           new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",          new PyInt(m_self->ownerID()));
        slim->SetItemString("categoryID",       new PyInt(m_self->categoryID()));
        slim->SetItemString("itemID",           new PyLong(m_self->itemID()));
        slim->SetItemString("incapacitated",    new PyInt(0));
        slim->SetItemString("online",           new PyInt(1));
    return slim;
}

void StationSE::UnloadStation()
{
    m_self->GetMyInventory()->Unload();
}

