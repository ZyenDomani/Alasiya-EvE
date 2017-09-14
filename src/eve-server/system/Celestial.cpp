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
    Update/Rewrite: Allan
*/

#include "eve-server.h"

#include "StaticDataMgr.h"
#include "system/Celestial.h"
#include "system/SystemManager.h"

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
      m_inventory = new Inventory(InventoryItemRef(this));
      _log(ITEM__TRACE, "Created Default CelestialObject for item %s (%u).", itemName().c_str(), itemID());
}

CelestialObject::CelestialObject(
    ItemFactory &_factory,
    uint32 _celestialID,
    const ItemType &_type,
    const ItemData &_data,
    const CelestialObjectData &_cData)
: InventoryItem(_factory, _celestialID, _type, _data),
  m_radius(_cData.radius),  // no longer needed here
  m_security(_cData.security),
  m_celestialIndex(_cData.celestialIndex),
  m_orbitIndex(_cData.orbitIndex)
  {
      m_inventory = new Inventory(InventoryItemRef(this));
      _log(ITEM__TRACE, "Created CelestialObject for item %s (%u) with radius of %.1f.", itemName().c_str(), itemID(), m_radius);
}

CelestialObjectRef CelestialObject::Load(ItemFactory &factory, uint32 celestialID)
{
    return InventoryItem::Load<CelestialObject>( factory, celestialID );
}

CelestialObjectRef CelestialObject::Spawn(ItemFactory &factory, ItemData &data) {
    uint32 celestialID = CelestialObject::CreateItemID( factory, data );
    if (celestialID)
        return CelestialObject::Load( factory, celestialID );
    return CelestialObjectRef();
}

uint32 CelestialObject::CreateItemID(ItemFactory &factory, ItemData &data) {
    return InventoryItem::CreateItemID(factory, data);
}

void CelestialObject::Delete() {
    InventoryItem::Delete();
}


CelestialSE::CelestialSE(CelestialObjectRef self, PyServiceMgr &services, SystemManager* system)
: ItemSystemEntity(self, services, system)
{
    _log(SE__DEBUG, "Created CSE for item %s (%u) with radius of %.1f.", self->itemName().c_str(), self->itemID(), m_radius);
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


AnomalySE::AnomalySE(CelestialObjectRef self, PyServiceMgr& services, SystemManager* system)
: CelestialSE(self, services, system)
{

}
void AnomalySE::EncodeDestiny(Buffer& into)
{
    using namespace Destiny;
    BallHeader head;
        head.entityID = m_self->itemID();
        head.mode = DSTBALL_STOP;
        head.radius = m_radius;
        head.x = x();
        head.y = y();
        head.z = z();
        head.flags = 0;
    into.Append( head );
    MassSector mass;
        mass.mass = 10000000000;    // as seen in packets
        mass.cloak = 0;
        mass.harmonic = m_harmonic;
        mass.corporationID = -1;
        mass.allianceID = -1;
    into.Append( mass );
    DSTBALL_FIELD_Struct main;
        main.formationID = 0xFF;
    into.Append( main );

    _log(SE__DESTINY, "AnomalySE::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}
PyDict* AnomalySE::MakeSlimItem()
{
    _log(SE__SLIMITEM, "MakeSlimItem for AnomalySE %s(%u)", GetName(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",           new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",           new PyInt(m_self->typeID()));
        slim->SetItemString("dungeonDataID",    new PyInt(0)); //?  seen 2990651
        slim->SetItemString("ownerID",          new PyInt(m_self->ownerID()));
    return slim;
}

WormholeSE::WormholeSE(CelestialObjectRef self, PyServiceMgr& services, SystemManager* system)
: CelestialSE(self, services, system)
{

}

void WormholeSE::EncodeDestiny(Buffer& into)
{
    using namespace Destiny;
    BallHeader head;
        head.entityID = m_self->itemID();
        head.mode = DSTBALL_STOP;
        head.radius = m_radius;
        head.x = x();
        head.y = y();
        head.z = z();
        head.flags = 0;
    into.Append( head );
    MassSector mass;
        mass.mass = 10000000000;    // as seen in packets
        mass.cloak = 0;
        mass.harmonic = m_harmonic;
        mass.corporationID = -1;
        mass.allianceID = -1;
    into.Append( mass );
    DSTBALL_FIELD_Struct main;
        main.formationID = 0xFF;
    into.Append( main );
    _log(SE__DESTINY, "WormholeSE::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

PyDict* WormholeSE::MakeSlimItem()
{
    _log(SE__SLIMITEM, "MakeSlimItem for WormholeSE %s(%u)", GetName(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",                   new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",                   new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",                  new PyInt(m_self->ownerID()));
        slim->SetItemString("otherSolarSystemClass",    new PyInt(sDataMgr.GetWHSystemClass(m_system->GetID())));   // will have to set this somewhere to ref here
        slim->SetItemString("wormholeSize",             new PyFloat(1.5)); // <1 = close to collapse
        slim->SetItemString("wormholeAge",              new PyInt(1));  //1 or 2
        slim->SetItemString("count",                    new PyInt(1));   //ships jumped thru?
        slim->SetItemString("dunSpawnID",               new PyInt(27));   //33, 263, 27
        slim->SetItemString("nebulaType",               new PyInt(16));  //26, 16, 253   << client graphic file #
        slim->SetItemString("expiryDate",               new PyLong(Win32TimeNow() + Win32Time_Day));  //? not sure here
    return slim;
}
