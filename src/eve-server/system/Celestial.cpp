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

#include "ship/DestinyManager.h"
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
      _log(ITEM__TRACE, "Created CelestialObject1 for item %s (%u).", itemName().c_str(), itemID());
}

CelestialObject::CelestialObject(   //called for star,
    ItemFactory &_factory,
    uint32 _celestialID,
    // InventoryItem stuff:
    const ItemType &_type,
    const ItemData &_data,
    // CelestialObject stuff:
    const CelestialObjectData &_cData)
: InventoryItem(_factory, _celestialID, _type, _data),
  m_radius(_cData.radius),
  m_security(_cData.security),
  m_celestialIndex(_cData.celestialIndex),
  m_orbitIndex(_cData.orbitIndex)
  {
      _log(ITEM__TRACE, "Created CelestialObject2 for item %s (%u).", itemName().c_str(), itemID());
}

CelestialObjectRef CelestialObject::Load(ItemFactory &factory, uint32 celestialID)
{
    return InventoryItem::Load<CelestialObject>( factory, celestialID );
}

template<class _Ty>
RefPtr<_Ty> CelestialObject::_LoadCelestialObject(ItemFactory &factory, uint32 celestialID,
    // InventoryItem stuff:
    const ItemType &type, const ItemData &data,
    // CelestialObject stuff:
    const CelestialObjectData &cData)
{
    // Our category is celestial; what to do next:
    switch( type.groupID() ) {
        ///////////////////////////////////////
        // Solar system:
        ///////////////////////////////////////
        case EVEDB::invGroups::Solar_System: {
            return SolarSystem::_LoadCelestialObject<SolarSystem>( factory, celestialID, type, data, cData );
        }

        ///////////////////////////////////////
        // Station:
        ///////////////////////////////////////
        case EVEDB::invGroups::Station: {
            return Station::_LoadCelestialObject<Station>( factory, celestialID, type, data, cData );
        }
    }

    // Create a generic one:
    return CelestialObjectRef( new CelestialObject( factory, celestialID, type, data, cData ) );
}

CelestialObjectRef CelestialObject::Spawn(ItemFactory &factory,
    // InventoryItem stuff:
    ItemData &data
) {
    uint32 celestialID = CelestialObject::_Spawn( factory, data );
    if( celestialID == 0 )
        return CelestialObjectRef();
    return CelestialObject::Load( factory, celestialID );
}

uint32 CelestialObject::_Spawn(ItemFactory &factory,
    // InventoryItem stuff:
    ItemData &data
) {
    // make sure it's a ship
    const ItemType *item = factory.GetType(data.typeID);
    if( !(item->categoryID() == EVEDB::invCategories::Celestial) )
        return 0;

    // store item data
    uint32 celestialID = InventoryItem::_Spawn(factory, data);
    if( celestialID == 0 )
        return 0;

    // nothing additional

    return celestialID;
}

void CelestialObject::Delete() {
    InventoryItem::Delete();
}


using namespace Destiny;

CelestialEntity::CelestialEntity(
    CelestialObjectRef celestial,
    //InventoryItemRef celestial,
    SystemManager *system,
    PyServiceMgr &services,
    const GPoint &position)
: CelestialDynamicSystemEntity(new DestinyManager(this, system), celestial),
  m_system(system),
  m_services(services)
{
    _celestialRef = celestial;
    m_destiny->SetPosition(position, false);
}

void CelestialEntity::Process() {
    //SystemEntity::Process();
}

void CelestialEntity::ForcedSetPosition(const GPoint &pt) {
    m_destiny->SetPosition(pt, false);
}

/*
void CelestialEntity::EncodeDestiny( Buffer& into ) const
{
    const GPoint& position = GetPosition();

        BallHeader head;
        head.id = GetID();
        head.mode = DSTBALL_STOP;
        head.radius = GetRadius();
        head.x = position.x;
        head.y = position.y;
        head.z = position.z;
        head.flags = IsMassive | IsFree;
        into.Append( head );

        MassSector mass;
        mass.mass = GetMass();
        mass.cloak = 0;
        mass.unknown52 = 0xFFFFFFFFFFFFFFFFLL;
        mass.corpID = GetCorporationID();
        mass.allianceID = GetAllianceID();
        into.Append( mass );

        ShipSector ship;
        ship.maxVelocity = GetMaxVelocity();
        ship.velocity_x = 0.0;
        ship.velocity_y = 0.0;
        ship.velocity_z = 0.0;
        ship.unknown_x = 0.0;
        ship.unknown_y = 0.0;
        ship.unknown_z = 0.0;
        ship.agility = GetAgility();
        ship.speed_fraction = 0.0;
        into.Append( ship );

        DSTBALL_STOP_Struct main;
        main.formationID = 0xFF;
        into.Append( main );

        _log(COMMON__WARNING, "CelestialEntity::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}
*/

void CelestialEntity::MakeDamageState(DoDestinyDamageState &into) const
{
    double shield = 0.0, armor = 0.0;       // update to fix frozen corpse sending NaN for shield and armor.
    if (m_self->typeID() != 10041) { //type = frozen corpse
        shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
        armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
    }
    into.shield = shield;
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float() +3;
    into.timestamp = Win32TimeNow();
    into.armor = armor;
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float());
}

