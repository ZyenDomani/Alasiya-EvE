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

#include "EVEServerConfig.h"
#include "ship/DestinyManager.h"
#include "system/Asteroid.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/BeltMgr.h"
#include "system/SystemBubble.h"

/*
 * AsteroidItem
 */
/*
AsteroidItem::AsteroidItem(ItemFactory& _factory, uint32 _asteroidID, const ItemType& _type, const ItemData& _idata, const AsteroidData& _adata)
: InventoryItem(_factory, _asteroidID, _type, _idata, _adata),
m_dbData(_adata)
{
    _log(ITEM__TRACE, "Created AsteroidItem for %s(%u).", _idata.name.c_str(), _asteroidID);
}

AsteroidItemRef AsteroidItem::Load(ItemFactory &factory, uint32 asteroidID)
{
    return InventoryItem::Load<AsteroidItem>( factory, asteroidID );
}

AsteroidItemRef AsteroidItem::Spawn(ItemFactory& factory, ItemData& idata, AsteroidData& adata) {
    const ItemType *type = factory.GetType(adata.typeID);
    if (!type)
        return AsteroidItemRef();

    // fix the name (if empty)
    if (adata.itemName.empty())
        adata.itemName = type->name();

    uint32 asteroidID = InventoryItem::CreateAsteroidID(factory, adata);
    if (asteroidID == 0)
        return AsteroidItemRef();

    AsteroidItemRef roidRef = AsteroidItemRef(new AsteroidItem( factory, asteroidID, *type, idata, adata));
    roidRef->SetAttribute(AttrQuantity,  adata.quantity);   // quantity in m^3
    roidRef->SetAttribute(AttrRadius, (adata.radius));  // Radius
    roidRef->SetAttribute(AttrMass, (roidRef->type().mass() * adata.quantity));      // Mass

    return roidRef;
}

template<class _Ty>
RefPtr<_Ty>  AsteroidItem::_LoadAsteroid(ItemFactory& factory, uint32 asteroidID, const ItemType& type, const ItemData& data, const AsteroidData& dbData)
{
    // ready to create
    return AsteroidItemRef( new AsteroidItem( factory, asteroidID, type, data, dbData ) );
}
*/

AsteroidSE::AsteroidSE(InventoryItemRef self, PyServiceMgr& services, SystemManager* system)
: ObjectSystemEntity(self, services, system),
m_growTimer(sConfig.cosmic.BeltGrowth *60 *60 *1000)  // hours->ms
{
    m_growTimer.Disable();
}

void AsteroidSE::Process() {
    /* called by EntityList::Process on every loop */
    /*  Enable base call to Process Targeting and Movement  */
    SystemEntity::Process();

    if (m_growTimer.Check())
        if (!m_system->GetBeltMgr()->IsActive(m_bubble->GetID()))
            Grow();
}

void AsteroidSE::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;

    BallHeader head;
        head.entityID = GetID();
        head.mode = DSTBALL_RIGID;
        head.radius = m_self->radius();
        head.x = x();
        head.y = y();
        head.z = z();
        head.flags = IsMassive;
    into.Append( head );
    DSTBALL_RIGID_Struct main;
        main.formationID = 0xFF;
    into.Append( main );

    _log(COMMON__WARNING, "AsteroidSE::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void AsteroidSE::MakeDamageState(DoDestinyDamageState &into) {
    into.shield = 1.0;
    into.recharge = 30000;
    into.timestamp = Win32TimeNow();
    into.armor = 1.0;
    into.structure = 1.0;
}

void AsteroidSE::Grow() {
    /*  not real sure how to implement this
     * maybe use internal data structure to hold sizes (current, possible) and time interval
     * use this to check/update current sizes (radius and mass)
     *
     * currently sets quantity back to full and disables m_growTimer
     */

    double quantity = ((25000 * log(m_self->radius())) - 112404.8);
    m_self->SetAttribute(AttrQuantity,  quantity);   // quantity in m^3

    m_growTimer.Disable();
}

void AsteroidSE::Delete()
{
    _log(SPAWN__DEPOP, "AsteroidSE::Delete() - Removing asteroid %s(%u) from beltID %u", \
            m_self->itemName().c_str(), m_self->itemID(), m_beltID);
    m_beltMgr->RemoveAsteroid(m_beltID, this);
    m_system->RemoveEntity(this);
    m_self->Delete();
}
