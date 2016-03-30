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

#include "inventory/AttributeEnum.h"
#include "ship/DestinyManager.h"
#include "station/Station.h"

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

template<class _Ty>
_Ty *StationType::_LoadStationType(ItemFactory &factory, uint32 stationTypeID,
    // ItemType stuff:
    const ItemGroup &group, const TypeData &data,
    // StationType stuff:
    const StationTypeData &stData)
{
    // ready to create
    return new StationType( stationTypeID, group, data, stData );
}

/*
 * StationData
 */
StationData::StationData(
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
Station::Station(
    ItemFactory &_factory,
    uint32 _stationID,
    // InventoryItem stuff:
    const StationType &_type,
    const ItemData &_data,
    // CelestialObject stuff:
    const CelestialObjectData &_cData,
    // Station stuff:
    const StationData &_stData)
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
      _log(ITEM__TRACE, "Created Station for item %s (%u).", itemName().c_str(), itemID());
  }

StationRef Station::Load(ItemFactory &factory, uint32 stationID)
{
    return InventoryItem::Load<Station>( factory, stationID );
}

template<class _Ty>
RefPtr<_Ty> Station::_LoadStation(ItemFactory &factory, uint32 stationID,
    // InventoryItem stuff:
    const StationType &type, const ItemData &data,
    // CelestialObject stuff:
    const CelestialObjectData &cData,
    // Station stuff:
    const StationData &stData)
{
    // ready to create
    return StationRef( new Station( factory, stationID, type, data, cData, stData ) );
}

bool Station::_Load()
{
    // load contents
    if( !LoadContents( m_factory ) )
        return false;

    return CelestialObject::_Load();
}

uint32 Station::_Spawn(ItemFactory &factory,
    // InventoryItem stuff:
    ItemData &data
) {
    // make sure it's a Station
    const ItemType *item = factory.GetType(data.typeID);
    if( !(item->categoryID() == EVEDB::invCategories::Station) )
        return 0;

    // store item data
    uint32 stationID = InventoryItem::_Spawn(factory, data);
    if( stationID == 0 )
        return 0;

    // nothing additional

    return stationID;
}

using namespace Destiny;

StationEntity::StationEntity(
    StationRef station,
    SystemManager *system,
    PyServiceMgr &services,
    const GPoint &position)
: DynamicSystemEntity(new DestinyManager(this, system), station),
  m_system(system),
  m_services(services)
{
    _stationRef = station;
    m_destiny->SetPosition(position, false);
}

void StationEntity::Process() {
    //SystemEntity::Process();
}

void StationEntity::ForcedSetPosition(const GPoint &pt) {
    m_destiny->SetPosition(pt, false);
}

void StationEntity::EncodeDestiny( Buffer& into ) const
{
    const GPoint& position = GetPosition();

    BallHeader head;
    head.entityID = GetID();
        head.mode = DSTBALL_RIGID;
        head.radius = GetRadius();
        head.x = position.x;
        head.y = position.y;
        head.z = position.z;
        head.flags = HasMiniBalls | IsGlobal;
    into.Append( head );

    DSTBALL_RIGID_Struct main;
        main.formationID = 0xFF;
    into.Append( main );

    const uint16 miniballsCount = 1;
    into.Append( miniballsCount );

    MiniBall miniball;
        miniball.x = -7701.181;
        miniball.y = 8060.06;
        miniball.z = 27878.900;
        miniball.radius = 1639.241;
    into.Append( miniball );
    _log(COMMON__WARNING, "StationEntity::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void StationEntity::MakeDamageState(DoDestinyDamageState &into) const {
    into.shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float() +6;
    into.timestamp = Win32TimeNow();
    into.armor = 1.0; // - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float());
}

PyDict *StationEntity::MakeSlimItem() const {
    _log(COMMON__WARNING, "MakeSlimItem for StationEntity %s(%u)", m_self->itemName().c_str(), m_self->itemID());
    PyDict *slim = new PyDict();
    slim->SetItemString("groupID",          new PyInt(m_self->groupID()));
    slim->SetItemString("name",             new PyString(m_self->itemName()));
    slim->SetItemString("corpID",           new PyInt(0));
    slim->SetItemString("allianceID",       new PyInt(0));
    slim->SetItemString("typeID",           new PyInt(m_self->typeID()));
    slim->SetItemString("ownerID",          new PyInt(m_self->ownerID()));
    slim->SetItemString("categoryID",       new PyInt(m_self->categoryID()));
    slim->SetItemString("itemID",           new PyLong(m_self->itemID()));
    slim->SetItemString("incapacitated",    new PyInt(0));
    slim->SetItemString("online",           new PyInt(1));
    return slim;
}

