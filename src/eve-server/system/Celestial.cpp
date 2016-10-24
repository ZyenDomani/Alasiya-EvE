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

#include "system/DestinyManager.h"
#include "station/Station.h"
#include "system/SolarSystem.h"

/*
 * CelestialObjectData
 */
CelestialObjectData::CelestialObjectData(
    double _radius,
    double _security,
    uint8 _celestialIndex,
    uint8 _orbitIndex)
: radius(_radius),
  security(_security),
  celestialIndex(_celestialIndex),
  orbitIndex(_orbitIndex)
{
}

/*
 * CelestialObject
 */
CelestialObject::CelestialObject(
    ItemFactory &_factory,
    uint32 _celestialID,
    const ItemType &_type,
    const ItemData &_data)
: InventoryItem(_factory, _celestialID, _type, _data),
  m_radius( 0.0 ),
  m_security( 0.0 ),
  m_celestialIndex( 0 ),
  m_orbitIndex( 0 )
  {
      _log(ITEM__TRACE, "Created Default CelestialObject for item %s (%u).", itemName().c_str(), itemID());
}

CelestialObject::CelestialObject(
    ItemFactory &_factory,
    uint32 _celestialID,
    const ItemType &_type,
    const ItemData &_data,
    const CelestialObjectData &_cData)
: InventoryItem(_factory, _celestialID, _type, _data),
  m_radius(_cData.radius),
  m_security(_cData.security),
  m_celestialIndex(_cData.celestialIndex),
  m_orbitIndex(_cData.orbitIndex)
  {
      _log(ITEM__TRACE, "Created CelestialObject for item %s (%u).", itemName().c_str(), itemID());
}

CelestialObjectRef CelestialObject::Load(ItemFactory &factory, uint32 celestialID)
{
    return InventoryItem::Load<CelestialObject>( factory, celestialID );
}

template<class _Ty>
RefPtr<_Ty> CelestialObject::_LoadCelestialObject(ItemFactory &factory, uint32 celestialID,
    const ItemType &type, const ItemData &data, const CelestialObjectData &cData)
{
    switch( type.groupID() ) {
        case EVEDB::invGroups::Solar_System: {
            return SolarSystem::_LoadCelestialObject<SolarSystem>( factory, celestialID, type, data, cData );
        }
        case EVEDB::invGroups::Station: {
            return StationItem::_LoadCelestialObject<StationItem>( factory, celestialID, type, data, cData );
        }
        /** @todo  finish these later....
        case EVEDB::invGroups::Planet: {
            return PlanetItem::_LoadCelestialObject<PlanetItem>( factory, celestialID, type, data, cData );
        }
        case EVEDB::invGroups::Moon: {
            return MoonItem::_LoadCelestialObject<MoonItem>( factory, celestialID, type, data, cData );
        }*/
    }

    return CelestialObjectRef( new CelestialObject( factory, celestialID, type, data, cData ) );
}

CelestialObjectRef CelestialObject::Spawn(ItemFactory &factory, ItemData &data) {
    uint32 celestialID = CelestialObject::CreateItemID( factory, data );
    if (celestialID)
        return CelestialObject::Load( factory, celestialID );
    return CelestialObjectRef();
}

uint32 CelestialObject::CreateItemID(ItemFactory &factory, ItemData &data) {
    const ItemType *item = factory.GetType(data.typeID);
    if (item->categoryID() != EVEDB::invCategories::Celestial)
        return 0;

    return InventoryItem::CreateItemID(factory, data);
}

void CelestialObject::Delete() {
    InventoryItem::Delete();
}


CelestialSE::CelestialSE(CelestialObjectRef self, PyServiceMgr &services, SystemManager* system)
: ItemSystemEntity(self, services, system)
{
    // Set radius of celestial object
    self->SetAttribute(AttrRadius, self->type().radius());
}

void CelestialSE::MakeDamageState(DoDestinyDamageState &into)
{
    double shield = 0.0, armor = 0.0;       // update to fix frozen corpse sending NaN for shield and armor.
    if (m_self->typeID() != 10041) { //type = frozen corpse
        shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
        armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
    }
    into.shield = shield;
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float();
    into.timestamp = Win32TimeNow();
    into.armor = armor;
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float());
}

