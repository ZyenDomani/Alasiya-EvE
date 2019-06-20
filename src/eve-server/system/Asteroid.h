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
    Author:        Zhur
    Updates:    Allan
*/

#ifndef __ASTEROID_H_INCL__
#define __ASTEROID_H_INCL__

#include "EVEServerConfig.h"
#include "system/SystemEntity.h"
#include "system/cosmicMgrs/ManagerDB.h"


class AsteroidItem
: public InventoryItem
{
    friend class InventoryItem; // to let it construct us
public:
    AsteroidItem(uint32 _asteroidID, const ItemType &_type, const ItemData &_data, const AsteroidData &_cData);
    virtual ~AsteroidItem()                             { /* Do nothing here */ }

    static AsteroidItemRef Load( uint32 asteroidID);
    static AsteroidItemRef Spawn( ItemData &idata, AsteroidData& adata);

    double      radius() const                          { return m_data.radius; }

protected:
    using InventoryItem::_Load;
    //virtual bool _Load();

    // Template loader:
    template<class _Ty>
    static RefPtr<_Ty> _LoadItem( uint32 asteroidID, const ItemType &type, const ItemData &data) {
        if (type.categoryID() != EVEDB::invCategories::Asteroid) {
            _log( ITEM__ERROR, "Trying to load %s(%u) as Asteroid.", type.category().name().c_str(), asteroidID);
            if (sConfig.server.StackTrace)
                EvE::traceStack();
            return RefPtr<_Ty>();
        }

        AsteroidData dbData = AsteroidData();
        if ( !ManagerDB::GetAsteroidData( asteroidID, dbData ) )
            return RefPtr<_Ty>();

        return AsteroidItemRef( new AsteroidItem(asteroidID, type, data, dbData ) );
    }

private:
    AsteroidData m_data;

};


/**
 * ObjectSystemEntity which represents asteroid object in space
 */

class AsteroidSE
: public ObjectSystemEntity
{
public:
    AsteroidSE(InventoryItemRef self, PyServiceMgr &services, SystemManager *system);
    virtual ~AsteroidSE()                               { /* Do nothing here */ }

    /* class type pointer querys. */
    virtual AsteroidSE* GetAsteroidSE()                 { return this; }
    /* class type tests. */
    virtual bool IsAsteroidSE()                         { return true; }

    /* SystemEntity interface */
    virtual void Delete();
    virtual void Process();
    virtual void EncodeDestiny( Buffer& into );
    virtual void MakeDamageState(DoDestinyDamageState &into);

    /* specific functions handled in this class. */
    void Grow();
    void SetMgr(BeltMgr* beltMgr, uint32 beltID) { m_beltMgr = beltMgr; m_beltID = beltID; }

protected:

private:
    BeltMgr* m_beltMgr;
    Timer m_growTimer;
    uint32 m_beltID;

};

#endif /* !__ASTEROID__H__INCL__ */
