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
    Author:     Allan
*/

#include "eve-server.h"

#include "Client.h"
#include "EVEServerConfig.h"
#include "Profile.h"
#include "character/Character.h"
#include "inventory/AttributeEnum.h"
#include "system/DestinyManager.h"
#include "ship/Missile.h"
#include "ship/Ship.h"
#include "system/Damage.h"

Missile::Missile( InventoryItemRef self, PyServiceMgr &services, SystemManager* pSystem, InventoryItemRef module, SystemEntity* target, ShipItem* ship)
: DynamicSystemEntity(self, services, pSystem),
  m_module(module),
  m_targetSE(target),
  m_ship(ship),
  m_hitTimer(1000), //arbitrary default
  m_lifeTimer(1000) //arbitrary default
{
    m_destiny = new DestinyManager(this);

    if (ship->HasPilot()) {
        Character* pChar = m_ship->GetPilot()->GetChar().get();
        m_ownerID = pChar->itemID();
        m_allyID = pChar->allianceID();
        m_corpID = pChar->corporationID();
        m_warID = pChar->warFactionID();
    } else {
        m_ownerID = ship->itemID();
        /** @todo finish these for npcs */
        m_warID = 0;
        m_allyID = 0;
        m_corpID = 0;
    }

    m_kinDamage = self->GetAttribute(AttrKineticDamage).get_float(),
    m_therDamage = self->GetAttribute(AttrThermalDamage).get_float(),
    m_emDamage = self->GetAttribute(AttrEmDamage).get_float(),
    m_expDamage = self->GetAttribute(AttrExplosiveDamage).get_float(),

    m_hitTimer.Disable();
    double flightTime = self->GetAttribute(AttrExplosionDelay).get_float();
    if (sConfig.rates.missileTime != 1.0)
        flightTime *= sConfig.rates.missileTime;
    m_lifeTimer.Start(flightTime);

    m_alive = true;

    m_hullHP = self->GetAttribute(AttrHP).get_int();

    /*  this is damage formula for missiles
     * Damage = D * MIN(1, Sr/Er, (Ev/V * Sr/Er)^(ln(DRF) / ln(DRS)) )
     *
     * D = base damage of the missile,
     * Sr = signature radius of the target,
     * Er = Explosion radius of the missile,
     * Ev = Explosion Velocity of the missile,
     * V = velocity of the target ship,
     * DRF = damage reduction factor of the missile.
     * MIN being a function that chooses the lower of two given vaules,
     * ln is natural logarithm.
     */
    double Sr = m_targetSE->GetSelf()->GetAttribute(AttrSignatureRadius).get_float();    // this is a default number, based on itemtype
    double Er = m_self->GetAttribute(AttrAoeCloudSize).get_float(); // Explosion Radius
    double Ev = m_self->GetAttribute(AttrAoeVelocity).get_float(); // Explosion Velocity
    double DRF = m_self->GetAttribute(AttrAoeDamageReductionFactor).get_float(); // Damage Reduction Factor
    double DRS = m_self->GetAttribute(AttrAoeDamageReductionSensitivity).get_float(); // Damage Reduction Sensitivity

    GPoint Vel = m_targetSE->GetVelocity();
    double V = Vel.length();

    double v1 = Sr/Er;
    double v2 = pow(((Ev/V) * (Sr/Er)), (log(DRF) / log(DRS)));
    m_damageMod = Min(v1, v2);

     _log(NPC__TRACE, "Created Missile object for %s (%u)", self.get()->itemName().c_str(), self.get()->itemID());
}

Missile::~Missile() {
    SafeDelete(m_destiny);
}

void Missile::Process() {
    double profileStartTime = 0.0;
    if (sConfig.server.UseProfiling)
        profileStartTime = GetTimeUSeconds();
    /*  Enable base call to Process Targeting and Movement  */
    SystemEntity::Process();
    if (!m_alive) {
        Delete();
    } else if (m_lifeTimer.Check(false)) {
        EndOfLife();
    } else if (m_hitTimer.Check(false)) {
        m_hitTimer.Disable();
        HitTarget();
    }
    if (sConfig.server.UseProfiling)
        sProfile.AddTime(_missileProfile, GetTimeUSeconds() - profileStartTime);
}

void Missile::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;

    BallHeader head;
        head.entityID = GetID();
        head.mode = DSTBALL_MISSILE;
        head.radius = GetRadius();
        head.x = x();
        head.y = y();
        head.z = z();
        head.flags = IsFree;
    into.Append( head );
    MassSector mass;
        mass.mass = m_destiny->GetMass();
        mass.cloak = 0;
        mass.Harmonic = -1.0f;
        mass.corporationID = GetCorporationID();
        mass.allianceID = GetAllianceID();
    into.Append( mass );
    DataSector data;
        data.maxVelocity = m_speed;
        data.velocity_x = m_destiny->GetVelocity().x;
        data.velocity_y = m_destiny->GetVelocity().y;
        data.velocity_z = m_destiny->GetVelocity().z;
        data.intertia = m_destiny->GetInertia();
        data.speedfraction = m_destiny->GetSpeedFraction();
    into.Append( data );
    DSTBALL_MISSILE_Struct miss;
        miss.ownerID = m_ownerID;
        miss.formationID = 0xFF;
        miss.effectStamp = m_destiny->GetStateStamp();
        miss.followID = m_destiny->GetTargetID();
        miss.followRange = (float)m_destiny->GetFollowDistance();
        miss.x = x();
        miss.y = y();
        miss.z = z();
    into.Append(miss);

    _log(DESTINY__MESSAGE, "Missile::EncodeDestiny(): %s id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

PyDict* Missile::MakeSlimItem() {
    _log(DESTINY__MESSAGE, "MakeSlimItem for MissileID %u", m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",           new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",           new PyInt(m_self->typeID()));
        slim->SetItemString("groupID",          new PyInt(m_self->groupID()));
        slim->SetItemString("categoryID",       new PyInt(m_self->categoryID()));
        slim->SetItemString("name",             new PyString(m_self->itemName()));
        slim->SetItemString("sourceModuleID",   new PyInt(m_module->itemID()));
        slim->SetItemString("corpID",           new PyInt(m_corpID));
        slim->SetItemString("allianceID",       new PyInt(m_allyID));
        slim->SetItemString("warFactionID",     new PyInt(m_warID));
        slim->SetItemString("securityStatus",   new PyFloat(0/*pChar->GetSecurityRating()*/)); /** @todo (allan) fix this */
        slim->SetItemString("ownerID",          new PyInt(m_ownerID)); // this is corp ID??
        slim->SetItemString("numLaunchers",     new PyInt(1));  /** @todo (allan) fix this */
        slim->SetItemString("nameID",           new PyInt(0));  /** @todo (allan) fix this */
    return(slim);
}

void Missile::MakeDamageState(DoDestinyDamageState &into) {
    into.shield = 1;
    into.recharge = 10000;
    into.timestamp = Win32TimeNow();
    into.armor = 1;
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float());
}

void Missile::HitTarget() {
    // Create Damage action:
    Damage d(m_ship->GetPilot()->GetShipSE(),
             m_module,
             m_self,
             EVEEffectID::missileLaunching  // from EVEEffectID::  should be an explosion effect here
            );

    d *= m_damageMod;
    if (sConfig.rates.missileRate != 1.0)
        d *= sConfig.rates.missileRate;

    m_targetSE->ApplyDamage(d);
    m_alive = false;
}

void Missile::EndOfLife() {
    m_alive = false;
    Delete();
}

void Missile::Delete() {
    //  cleanup here
    if (m_alive) return;
    m_targMgr->DoDestruction();
    m_system->RemoveEntity(this);
    m_self->Delete();
    // do we need to do anything else here?
}

double Missile::Min(double a, double b)
{
    /*  this method returns the smallest number of the 2 given, or 1 if a>1 && b>1 */
    double min = ( a > b ? b : a );

    if (min > 1)
        return 1;
    else
        return min;
}
