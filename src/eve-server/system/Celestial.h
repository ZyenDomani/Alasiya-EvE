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

#ifndef __CELESTIAL__H__INCL__
#define __CELESTIAL__H__INCL__

#include "EVEServerConfig.h"
#include "inventory/InventoryItem.h"
#include "system/SystemEntity.h"

/**
 * Data container for celestial object.
 */
class CelestialObjectData
{
public:
    CelestialObjectData(
        double _radius = 0.0,
        double _security = 0.0,
        uint8 _celestialIndex = 0,
        uint8 _orbitIndex = 0
    );

/* these have to be public for inventorydb to load into them */
    double radius;
    double security;
    uint8 celestialIndex;
    uint8 orbitIndex;
};

/**
 * InventoryItem for generic celestial object.
 */
class CelestialObject
: public InventoryItem
{
    friend class InventoryItem; // to let it construct us
public:
    CelestialObject(ItemFactory &_factory, uint32 _celestialID, const ItemType &_type, const ItemData &_data);
    CelestialObject(ItemFactory &_factory, uint32 _celestialID, const ItemType &_type, const ItemData &_data, const CelestialObjectData &_cData);
    virtual ~CelestialObject()                          { /* Do nothing here */ }

    static CelestialObjectRef Load(ItemFactory &factory, uint32 celestialID);
    static CelestialObjectRef Spawn(ItemFactory &factory, ItemData &data);

    void Delete();

    double      radius() const { return m_radius; }
    double      security() const { return m_security; }
    uint8       celestialIndex() const { return m_celestialIndex; }
    uint8       orbitIndex() const { return m_orbitIndex; }

protected:
    using InventoryItem::_Load;
    //virtual bool _Load();

    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem(ItemFactory &factory, uint32 celestialID, const ItemType &type, const ItemData &data)
    {
        if (type.categoryID() != EVEDB::invCategories::Celestial)  {
            _log( ITEM__ERROR, "Trying to load %s as Celestial.", type.category().name().c_str() );
            if (sConfig.server.StackTrace)
                EvE::traceStack();
            return RefPtr<_Ty>();
        }

        CelestialObjectData cData;
        if (!factory.db().GetCelestialObject(celestialID, cData))
            return RefPtr<_Ty>();

        return _Ty::template _LoadCelestialObject<_Ty>( factory, celestialID, type, data, cData );
    }

    // Actual loading stuff:
    template<class _Ty>
    static RefPtr<_Ty> _LoadCelestialObject(ItemFactory &factory, uint32 celestialID, const ItemType &type, const ItemData &data, const CelestialObjectData &cData)
    {
        return CelestialObjectRef( new CelestialObject( factory, celestialID, type, data, cData ) );
    }

    static uint32 CreateItemID(ItemFactory &factory, ItemData &data);

    /* these have to be public for inventorydb to load into them. */
    double m_radius;
    double m_security;
    uint8 m_celestialIndex;
    uint8 m_orbitIndex;
};


/**
 * ItemSystemEntity which represents celestial object in space
 */
class PyServiceMgr;

class CelestialSE
: public ItemSystemEntity
{
public:
    CelestialSE(CelestialObjectRef self, PyServiceMgr &services, SystemManager* system);
    virtual ~CelestialSE()                              { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual const CelestialSE* GetCelestialSE()         { return this; }
    /* class type tests. */
    virtual bool IsCelestialSE()                        { return true; }

    /* SystemEntity interface */
    virtual void MakeDamageState(DoDestinyDamageState &into);

protected:

};

#endif /* !__CELESTIAL__H__INCL__ */


