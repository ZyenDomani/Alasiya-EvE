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

#include "system/SolarSystem.h"
/*
    Border = Borders another Region or Constellation

    Fringe = 1 connection to this system (dead end system)

    Corridor = 2 connections to this system (in one side and out the other)

    Hub = 3+ connections to this system

    International = always has Border/Constellation, almost always Regional

    Regional = always has Border/Constellation

    Constellation = always the same as Border

    Security = If it is positive, rounding to nearest 1/10th gives the in-game security level. 0 or lower are 0.0 in-game.
    */

/*
 * SolarSystemData
 */
SolarSystemData::SolarSystemData(
    const GPoint &_minPos,
    const GPoint &_maxPos,
    double _luminosity,
    bool _border,
    bool _fringe,
    bool _corridor,
    bool _hub,
    bool _international,
    bool _regional,
    bool _constellation,
    double _security,
    uint32 _factionID,
    double _radius,
    uint32 _sunTypeID,
    const char *_securityClass)
: minPosition(_minPos),
  maxPosition(_maxPos),
  luminosity(_luminosity),
  border(_border),
  fringe(_fringe),
  corridor(_corridor),
  hub(_hub),
  international(_international),
  regional(_regional),
  constellation(_constellation),
  security(_security),
  factionID(_factionID),
  radius(_radius),
  sunTypeID(_sunTypeID),
  securityClass(_securityClass)
{
}

/*
 * SolarSystem
 */
SolarSystem::SolarSystem(
    ItemFactory &_factory,
    uint32 _solarSystemID,
    // InventoryItem stuff:
    const ItemType &_type,
    const ItemData &_data,
    // CelestialObject stuff:
    const CelestialObjectData &_cData,
    // SolarSystem stuff:
    const SolarSystemData &_ssData)
: CelestialObject(_factory, _solarSystemID, _type, _data, _cData),
  m_minPosition(_ssData.minPosition),
  m_maxPosition(_ssData.maxPosition),
  m_luminosity(_ssData.luminosity),
  m_border(_ssData.border),
  m_fringe(_ssData.fringe),
  m_corridor(_ssData.corridor),
  m_hub(_ssData.hub),
  m_international(_ssData.international),
  m_regional(_ssData.regional),
  m_constellation(_ssData.constellation),
  m_security(_ssData.security),
  m_factionID(_ssData.factionID),
  m_radius(_ssData.radius),
  m_securityClass(_ssData.securityClass)
{
    // consistency check
    assert(_type.id() == _ssData.sunTypeID);
    m_inventory = new Inventory(InventoryItemRef(this));
    _log(ITEM__TRACE, "Created SolarSystem Item %p for %s (%u).", this, itemName().c_str(), itemID());
}

SolarSystem::~SolarSystem()
{
    SafeDelete(m_inventory);
}

SolarSystemRef SolarSystem::Load(ItemFactory &factory, uint32 solarSystemID)
{
    return InventoryItem::Load<SolarSystem>( factory, solarSystemID );
}

bool SolarSystem::_Load() {
    return CelestialObject::_Load();
}

void SolarSystem::AddItemToInventory(InventoryItemRef item)
{
    AddItem( item );
}

void SolarSystem::AddItem(InventoryItemRef item)
{
    m_inventory->AddItem( item );
}

// unload...loop thru currently loaded inventory and call RemoveItem for each.

void SolarSystem::RemoveItemFromInventory( InventoryItemRef item )
{
    RemoveItem( item );
}

void SolarSystem::RemoveItem(InventoryItemRef item)
{
    m_inventory->RemoveItem( item );
}
