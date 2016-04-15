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
*/

#include "eve-server.h"

#include "Container.h"
#include "EVEServerConfig.h"
#include "inventory/AttributeEnum.h"
#include "ship/DestinyManager.h"
#include "system/SystemDB.h"
#include "system/SystemEntity.h"

using namespace Destiny;

SystemEntity::SystemEntity()
: TargMgr(this),
  m_bubble(nullptr)
{
}

void SystemEntity::Process() {
    //for now we're just processing incomming targeting.
    TargMgr.Process();
}

uint32 SystemEntity::GetLocationID()
{
	return (Item()->locationID());
}

double SystemEntity::DistanceTo2(const SystemEntity* other) const {
    if (!other->m_bubble) return 1000000.0;
    GVector delta(GetPosition(), other->GetPosition());
    return (delta.length());
}

float SystemEntity::GetRadius() const {
    if (!Item())
        return (10.0f);
    return (Item()->GetAttribute(AttrRadius).get_float());
}

PyTuple* SystemEntity::MakeDamageState() const {
    if (IsWreck()) {
        WreckEntity* pWE;
        DoDestinyDamageState3 ddds;
        pWE->MakeWreckState(ddds);
        return(ddds.Encode());
    }
    DoDestinyDamageState ddds;
    MakeDamageState(ddds);
    return (ddds.Encode());
}
/*
PyList* SystemEntity::MakeDamageStateList() const {
    if (IsWreck()) {
        WreckEntity *pWE;
        DoDestinyDamageState3 ddds;
        pWE->MakeWreckState(ddds);
        return(ddds.Encode());
    }
    DoDestinyDamageState ddds;
    MakeDamageState(ddds);
    return (ddds.Encode());
}
*/
ItemSystemEntity::ItemSystemEntity(InventoryItemRef self)
: SystemEntity(),
  m_self()
{
    _SetSelf( self );
    //setup some default attributes which normally do not initialize.
}

ItemSystemEntity::~ItemSystemEntity()
{
}

void ItemSystemEntity::_SetSelf(InventoryItemRef self) {
    if ( !self ) {
      /*  if (sConfig.server.testServer)
            codelog(ITEM__ERROR, "Tried to set self to NULL!");*/
        return;
    }

    m_self = self;
}

const char* ItemSystemEntity::GetName() const {
    if (!m_self)
        return ("NoName");
    return (m_self->itemName().c_str());
}

const GPoint& ItemSystemEntity::GetPosition() const {
    static const GPoint point(NULL_ORIGIN);
    if (!m_self) {
        _log( ITEM__TRACE, "ItemSystemEntity::GetPosition() - m_self = null.  Returning NULL_ORIGIN" );
        return (point);
    }
    return (m_self->position());
}
/*
const GVector &ItemSystemEntity::GetVelocity() const {
    static const GVector err(0.0, 0.0, 0.0);
    return(err);
}*/

PyDict* ItemSystemEntity::MakeSlimItem() const {
    _log(COMMON__WARNING, "MakeSlimItem for ItemSystemEntity %s(%u)", Item()->itemName().c_str(), Item()->itemID());
    PyDict *slim = new PyDict();
    slim->SetItemString("itemID",       new PyLong(Item()->itemID()));
    slim->SetItemString("typeID",       new PyInt(Item()->typeID()));
    slim->SetItemString("ownerID",      new PyInt(Item()->ownerID()));
    slim->SetItemString("categoryID",   new PyInt(Item()->categoryID()));
    slim->SetItemString("groupID",      new PyInt(Item()->groupID()));
    slim->SetItemString("name",         new PyString(Item()->itemName()));
    slim->SetItemString("corpID",       new PyInt(0));
    slim->SetItemString("allianceID",   new PyInt(0));
    return (slim);
}

uint32 ItemSystemEntity::GetID() const {
    if (!Item())
        return 0;
    return (Item()->itemID());
}

void ItemSystemEntity::MakeDamageState(DoDestinyDamageState &into) const {
    into.shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float() +1;
    into.timestamp = Win32TimeNow();
    into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float());
}


DynamicSystemEntity::DynamicSystemEntity(DestinyManager* dm, InventoryItemRef self)
: ItemSystemEntity(self),
  m_destiny(dm)
{
}

DynamicSystemEntity::~DynamicSystemEntity() {
    if (m_destiny) {
        //Do not do anything with the destiny manager, as it's m_self
        //is now partially destroyed, which will seriously upset things.
        SafeDelete(m_destiny);;
    }
}

void DynamicSystemEntity::ProcessDestiny() {
    if (m_destiny)
        m_destiny->Process();
}

const GPoint &DynamicSystemEntity::GetPosition() const {
    if (!m_destiny)
        return (ItemSystemEntity::GetPosition());
    return (m_destiny->GetPosition());
}

const GVector &DynamicSystemEntity::GetVelocity() const {
    static const GVector point(NULL_ORIGIN);
    if (!m_destiny)
        return (point);
    return (m_destiny->GetVelocity());
}

double DynamicSystemEntity::GetMass() const {
    if (!Item())
        return (0.0f);
    return Item()->GetAttribute(AttrMass).get_float();
}

double DynamicSystemEntity::GetMaxVelocity() const {
    if(!Item())
        return(0.0f);
    return Item()->GetAttribute(AttrMaxVelocity).get_float();
}

double DynamicSystemEntity::GetAgility() const {
    if(!Item())
        return(0.0f);
    return Item()->GetAttribute(AttrAgility).get_float();
}

PyDict *DynamicSystemEntity::MakeSlimItem() const {
    _log(COMMON__WARNING, "MakeSlimItem for DynamicSystemEntity %s(%u)", Item()->itemName().c_str(), Item()->itemID());
    PyDict *slim = new PyDict();
    slim->SetItemString("itemID",       new PyLong(Item()->itemID()));
    slim->SetItemString("typeID",       new PyInt(Item()->typeID()));
    slim->SetItemString("ownerID",      new PyInt(Item()->ownerID()));
    slim->SetItemString("categoryID",   new PyInt(Item()->categoryID()));
    slim->SetItemString("groupID",      new PyInt(Item()->groupID()));
    slim->SetItemString("name",         new PyString(Item()->itemName()));
    slim->SetItemString("corpID",       new PyInt(0));
    slim->SetItemString("allianceID",   new PyInt(0));
    return (slim);
}

//TODO: ask the destiny manager to do this for us!
void DynamicSystemEntity::EncodeDestiny( Buffer& into ) const
{
    const GPoint& position = GetPosition();

     BallHeader head;
     head.entityID = GetID();
     head.mode = DSTBALL_STOP;
     head.radius = GetRadius();
     head.x = position.x;
     head.y = position.y;
     head.z = position.z;
     head.flags = IsFree;
     into.Append( head );

     MassSector mass;
     mass.mass = GetMass();
     mass.cloak = 0;
     mass.Harmonic = 1.0f;
     mass.corporationID = GetCorporationID();
     mass.allianceID = GetAllianceID();
     into.Append( mass );

     _log(COMMON__WARNING, "DynamicSystemEntity::EncodeDestiny() - %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void DynamicSystemEntity::MakeDamageState(DoDestinyDamageState &into) const {
    into.shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float() +2;
    into.timestamp = Win32TimeNow();
    into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float());
}

CelestialDynamicSystemEntity::CelestialDynamicSystemEntity(DestinyManager *dm, InventoryItemRef self)
: DynamicSystemEntity(dm, self)
{
}

CelestialDynamicSystemEntity::~CelestialDynamicSystemEntity() {
    if (m_destiny) {
        //Do not do anything with the destiny manager, as it's m_self
        //is now partially destroyed, which will majority upset things.
        SafeDelete(m_destiny);
    }
}

PyDict *CelestialDynamicSystemEntity::MakeSlimItem() const {
    _log(COMMON__WARNING, "MakeSlimItem for CelestialDynamicSystemEntity %s(%u)", Item()->itemName().c_str(), Item()->itemID());
    PyDict *slim = new PyDict();
    slim->SetItemString("itemID",       new PyLong(Item()->itemID()));
    slim->SetItemString("typeID",       new PyInt(Item()->typeID()));
    slim->SetItemString("ownerID",      new PyInt(Item()->ownerID()));
    slim->SetItemString("categoryID",   new PyInt(Item()->categoryID()));
    slim->SetItemString("groupID",      new PyInt(Item()->groupID()));
    slim->SetItemString("name",         new PyString(Item()->itemName()));
    slim->SetItemString("corpID",       new PyInt(0));
    slim->SetItemString("allianceID",   new PyInt(0));
    return (slim);
}

//TODO: ask the destiny manager to do this for us!
void CelestialDynamicSystemEntity::EncodeDestiny( Buffer& into ) const
{
    const GPoint& position = GetPosition();
    const std::string itemName( GetName() );

    BallHeader head;
    head.entityID = GetID();
        head.mode = DSTBALL_RIGID;
        head.radius = GetRadius();
        head.x = position.x;
        head.y = position.y;
        head.z = position.z;
        head.flags = IsMassive;
    into.Append( head );

    DSTBALL_RIGID_Struct main;
        main.formationID = 0xFF;
    into.Append( main );
    _log(COMMON__WARNING, "CelestialDynamicSystemEntity::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}
