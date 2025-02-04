/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2016 The EVEmu Team
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
    Rewrite:    Allan
*/

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "EntityMgr.h"
#include "inventory/AttributeEnum.h"
#include "npc/Drone.h"
#include "system/Container.h"
#include "system/Damage.h"
#include "system/DestinyManager.h"
#include "system/SystemManager.h"
#include "system/SystemBubble.h"
#include "system/cosmicMgrs/AnomalyMgr.h"

DroneSE::DroneSE(InventoryItemRef drone, PyServiceMgr &services, SystemManager* pSystem, const FactionData& data)
: DynamicSystemEntity(drone, services, pSystem),
m_pClient(nullptr),
m_AI(new DroneAIMgr(this)),
m_pShipSE(nullptr),
m_system(pSystem),
m_controlDistance(20000),       // 20km default
m_controllerID(0),
m_controllerOwnerID(0),
m_online(false)
{
    assert (m_AI != nullptr);
    assert (m_system != nullptr);

    m_warID = data.factionID;
    m_allyID = data.allianceID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;

    _log(DRONE__TRACE, "Created Drone object for %s (%u)", drone->name(), drone->itemID());
}

DroneSE::~DroneSE() {
    SafeDelete(m_AI);
}

void DroneSE::Init() {
    m_pClient = sEntityMgr.FindClientByCharID(m_ownerID);
    if (m_pClient == nullptr) {
        Abandon();
    } else if (m_pClient->IsLogin()) {
        /* creating system while player is login will load player drone
         * then try to access client object, which isnt fully constructed at this point
         */
        // not much choice here yet, till i figure something else out...
        Abandon();
    } else if (m_pClient->GetShip()->typeID() != EVEDB::invTypes::Capsule) {
        m_controllerID = m_pClient->GetShipID();
        m_controllerOwnerID = m_ownerID;
        if (m_pClient->GetShipSE() != nullptr) {
            m_pShipSE = m_pClient->GetShipSE();
            Online(m_pShipSE);
        }
    }

    m_orbitRange = m_self->GetAttribute(AttrOrbitRange).get_float();
    if (m_orbitRange < 1) {
        if (m_self->GetAttribute(AttrMaxRange) < m_self->GetAttribute(AttrFalloff)) {
            m_orbitRange = m_self->GetAttribute(AttrMaxRange).get_float();
        } else {
            m_orbitRange = m_self->GetAttribute(AttrFalloff).get_float();
        }
    }

    // Create default dynamic attributes in the AttributeMap:
    m_self->SetAttribute(AttrInertiaMod,          EvilOne, false);
    m_self->SetAttribute(AttrDamage,              EvilZero, false);
    m_self->SetAttribute(AttrArmorDamage,         EvilZero, false);
    m_self->SetAttribute(AttrWarpCapacitorNeed,   0.0000001, false);
    m_self->SetAttribute(AttrOrbitRange,          m_orbitRange, false);
    m_self->SetAttribute(AttrMass,                m_self->type().mass(), false);
    m_self->SetAttribute(AttrRadius,              m_self->type().radius(), false);
    m_self->SetAttribute(AttrVolume,              m_self->type().volume(), false);
    m_self->SetAttribute(AttrCapacity,            m_self->type().capacity(), false);
    m_self->SetAttribute(AttrShieldCharge,        m_self->GetAttribute(AttrShieldCapacity), false);
    m_self->SetAttribute(AttrCapacitorCharge,     m_self->GetAttribute(AttrCapacitorCapacity), false);

    /** @todo update attribs from char skills here....it's not done by Fx system */
    // are we sure of this??
    if (m_pClient != nullptr) {

    }

    m_destiny->UpdateShipVariables();

    SetResists();

    /* Gets the value from the Drone and put on our own vars */
    m_emDamage = m_self->GetAttribute(AttrEmDamage).get_float(),
    m_kinDamage = m_self->GetAttribute(AttrKineticDamage).get_float(),
    m_therDamage = m_self->GetAttribute(AttrThermalDamage).get_float(),
    m_expDamage = m_self->GetAttribute(AttrExplosiveDamage).get_float(),
    m_hullDamage = m_self->GetAttribute(AttrDamage).get_float();
    m_armorDamage = m_self->GetAttribute(AttrArmorDamage).get_float();
    m_shieldCharge = m_self->GetAttribute(AttrShieldCharge).get_float();
    m_shieldCapacity = m_self->GetAttribute(AttrShieldCapacity).get_float();
}

void DroneSE::AssignTo(Client* pClient) {
    // does drone use skills of new pilot?  probably not
    m_pClient = pClient;
    m_controllerID = pClient->GetShipID();
    m_controllerOwnerID = m_pShipSE->GetOwnerID();
    m_AI->AssignShip(m_pShipSE);
    if (m_bubble != nullptr)
        StateChange();
}

void DroneSE::Process() {
    if (m_killed)
        return;
    double profileStartTime(GetTimeUSeconds());

    /** @todo (allan) finish drone AI and processing */
    if (m_online)
        m_AI->Process();

    /*  Enable base call to Process Targeting and Movement  */
    SystemEntity::Process();

    // TODO: shield regen
    // 479     shieldRechargeRate  NULL    250000

    if (sConfig.debug.UseProfiling)
        sProfiler.AddTime(Profile::drone, GetTimeUSeconds() - profileStartTime);
}

void DroneSE::SaveDrone() {
    m_self->SaveItem();
}

void DroneSE::RemoveDrone() {
    m_self->Delete();
    delete this;
}

void DroneSE::Launch(ShipSE* pShipSE) {
    m_pShipSE = pShipSE;

    m_controllerID = pShipSE->GetID();
    m_controllerOwnerID = pShipSE->GetOwnerID();

    m_self->ChangeSingleton(false);
    m_system->AddEntity(this);

    assert (m_bubble != nullptr);

    if (m_bubble != nullptr)
        StateChange();
}

void DroneSE::Online(ShipSE* pShipSE/*nullptr*/) {
    // TODO:  check for available bandwidth
    m_online = true;
    m_pShipSE = pShipSE;
    m_AI->AssignShip(pShipSE);

    if (pShipSE != nullptr)
        m_controlDistance = pShipSE->GetSelf()->GetAttribute(AttrDroneControlDistance).get_uint32();
}

void DroneSE::Offline() {
    // this is called by abandon also
    m_destiny->Stop();
    m_AI->AssignShip(nullptr);
    m_online = false;

    if (m_bubble != nullptr)
        StateChange();
}

void DroneSE::IdleOrbit(uint32 speed, ShipSE* pShipSE/*nullptr*/) {
    // check this...
    if (pShipSE == nullptr)
        pShipSE = m_pShipSE;

    if (!m_online)
        return;         // error here?

    // set speed and begin lazy orbit
    m_destiny->SetMaxVelocity(speed);
    m_destiny->SetSpeedFraction(MakeRandomFloat(0.3, 0.6));  // use 30-60% here
    m_destiny->InitOrbit(pShipSE, m_orbitRange);
}

void DroneSE::Abandon() {
    //SystemEntity::Abandon();
    m_abandoned = true;
    m_controllerID = 0;
    m_controllerOwnerID = 0;
    m_controlDistance = 0;
    m_AI->Abandon();
    Offline();
}

void DroneSE::Reconnect(ShipSE* pShipSE) {
    if (m_pShipSE != pShipSE ) {
        // new owner
        Client* pClient = pShipSE->GetPilot();
        m_warID = pClient->GetWarFactionID();
        m_allyID = pClient->GetAllianceID();
        m_corpID = pClient->GetCorporationID();
        m_ownerID = pClient->GetCharacterID();
    }

    m_controllerID = pShipSE->GetID();
    m_controllerOwnerID = m_ownerID;

    Online(pShipSE);
    m_AI->SetIdle();

    if (m_bubble != nullptr)
        StateChange();
}

void DroneSE::ShipWarping(ShipSE* pShipSE) {
    if (m_pShipSE != pShipSE ) {

    }

}

void DroneSE::TargetDestroyed() {

}

void DroneSE::Engage(SystemEntity* pTarget) {
    CheckCommandDistance();
    // target, check distances, begin attack
    m_AI->Target(pTarget);
}

void DroneSE::TargetAdded(SystemEntity* pSE) {
    /** @todo (Allan) will need code once drones are implemented */
}

void DroneSE::TargetLost(SystemEntity* pSE) {
    m_AI->TargetLost(pSE);
}

void DroneSE::TargetedAdd(SystemEntity* pSE) {
    m_AI->Targeted(pSE);
}

void DroneSE::TargetedLost(SystemEntity* pSE) {
    /** @todo (Allan) will need code once drones are implemented */
}

void DroneSE::BeginMining(SystemEntity* pTarget, bool repeat) {
    CheckCommandDistance();
    if (m_AI->GetState() == DroneAI::State::Mining) {
        throw UserError("EntityCurrentlyMining")
                .AddFormatValue("targetTypeName", new PyString(GetName()));
    }

    _log(DRONE__TRACE, "%s's %s begin mining", m_pClient->GetName(), GetName());
    m_AI->Target(pTarget);
}


void DroneSE::CheckCommandDistance() {
    if (m_pShipSE->GetPosition().distance(GetPosition()) > m_controlDistance) {
        _log(DRONE__INFO, "%s outside control distance of %u", GetName(), m_controlDistance);

        throw UserError("EntityDistantCommand")
                .AddFormatValue("targetTypeName", new PyString(GetName()))
                .AddAmountU("distance", m_controlDistance);
    }
}

bool DroneSE::InControlDistance() {
    return (m_pShipSE->GetPosition().distance(GetPosition()) < m_controlDistance);
}


// destiny methods below...

uint32 DroneSE::GetTargetID() {
    if  (m_AI->GetTarget() != nullptr)
        return m_AI->GetTarget()->GetID();
    if (m_targMgr->GetFirstTarget() != nullptr)
        return m_targMgr->GetFirstTarget()->GetID();

    return 0;
}

void DroneSE::StateChange() {
    //OnDroneStateChange(droneID, ownerID, controllerID, activityState, droneTypeID, controllerOwnerID, targetID)
    if (m_online) {
        OnDroneStateChange du;
        du.droneID = m_self->itemID();
        du.ownerID = m_ownerID;
        du.droneTypeID = m_self->typeID();
        du.controllerID = m_controllerID;
        du.controllerOwnerID = m_controllerOwnerID;
        du.activityState = m_AI->GetState();
        du.targetID = GetTargetID();
        PyTuple* up = du.Encode();
        // bubblecast is faster than destiny::update
        m_bubble->BubblecastDestinyUpdate(&up, "destiny");
        //pShipSE->DestinyMgr()->SendSingleDestinyUpdate(&up);
    } else {
        PyList* list = new PyList();
        list->AddItemInt(m_self->itemID());
        list->AddItem(PyStatic.NewNone());
        list->AddItem(PyStatic.NewNone());
        list->AddItem(PyStatic.NewNone());
        list->AddItem(PyStatic.NewNone());
        list->AddItem(PyStatic.NewNone());
        list->AddItem(PyStatic.NewNone());
        PyTuple* tuple = new PyTuple(2);
        tuple->SetItem(0, new PyString("OnDroneStateChange"));
        tuple->SetItem(1, list);
        m_bubble->BubblecastDestinyUpdate(&tuple, "destiny");
    }
}

PyDict* DroneSE::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for Drone %u ", m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",           new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",           new PyInt(m_self->typeID()));
        slim->SetItemString("categoryID",       new PyInt(m_self->categoryID()));
        slim->SetItemString("groupID",          new PyInt(m_self->groupID()));
        slim->SetItemString("name",             new PyString(m_self->itemName()));
        slim->SetItemString("ownerID",          new PyInt(m_ownerID));
        slim->SetItemString("corpID",           IsCorpID(m_corpID) ? new PyInt(m_corpID) : PyStatic.NewNone());
        slim->SetItemString("allianceID",       IsAllianceID(m_allyID) ? new PyInt(m_allyID) : PyStatic.NewNone());
        slim->SetItemString("warFactionID",     IsFactionID(m_warID) ? new PyInt(m_warID) : PyStatic.NewNone());
        slim->SetItemString("bounty",           new PyFloat(GetBounty()));
        slim->SetItemString("securityStatus",   new PyFloat(GetSecurityRating()));
    return slim;
}

void DroneSE::EncodeDestiny(Buffer& into)
{
    using namespace Destiny;

    uint8 mode = m_destiny->GetState(); //Ball::Mode::STOP;

    // drone id's begin at 500m
    BallHeader head = BallHeader();
        head.entityID = GetID();
        head.mode = mode;
        head.radius = GetRadius();
        head.posX = x();
        head.posY = y();
        head.posZ = z();
        head.flags = Ball::Flag::IsFree;
    into.Append( head );
    MassSector mass = MassSector();
        mass.mass = m_self->GetAttribute(AttrMass).get_double();
        mass.cloak = 0;
        mass.harmonic = m_harmonic;
        mass.corporationID = m_corpID;
        mass.allianceID = (IsAllianceID(m_allyID) ? m_allyID : -1);
    into.Append( mass );
    DataSector data = DataSector();
        data.maxSpeed = m_destiny->GetMaxVelocity();
        data.velX = m_destiny->GetVelocity().x;
        data.velY = m_destiny->GetVelocity().y;
        data.velZ = m_destiny->GetVelocity().z;
        data.inertia = m_self->GetAttribute(AttrInertiaMod).get_float();
        data.speedfraction = m_destiny->GetSpeedFraction();
    into.Append( data );
    switch (mode) {
        case Ball::Mode::WARP: {
            GPoint target = m_destiny->GetTargetPoint();
            WARP_Struct warp;
                warp.formationID = 0xFF;
                warp.targX = target.x;
                warp.targY = target.y;
                warp.targZ = target.z;
                warp.speed = m_destiny->GetWarpSpeed();       //ship warp speed x10  (dont ask...this is what it is...more dumb ccp shit)
                // warp timing.  see Ship::EncodeDestiny() for notes/updates
                warp.effectStamp = -1; //m_destiny->GetStateStamp();   //timestamp when warp started
                warp.followRange = 0;   //this isnt right
                warp.followID = 0;  //this isnt right
            into.Append( warp );
        }  break;
        case Ball::Mode::FOLLOW: {
            FOLLOW_Struct follow;
                follow.followID = m_destiny->GetTargetID();
                follow.followRange = m_destiny->GetFollowDistance();
                follow.formationID = 0xFF;
            into.Append( follow );
        }  break;
        case Ball::Mode::ORBIT: {
            ORBIT_Struct orbit;
                orbit.targetID = m_destiny->GetTargetID();
                orbit.followRange = m_destiny->GetFollowDistance();
                orbit.formationID = 0xFF;
            into.Append( orbit );
        }  break;
        case Ball::Mode::GOTO: {
            GPoint target = m_destiny->GetTargetPoint();
            GOTO_Struct go;
                go.formationID = 0xFF;
                go.x = target.x;
                go.y = target.y;
                go.z = target.z;
            into.Append( go );
        }  break;
        default: {
            STOP_Struct main;
                main.formationID = 0xFF;
            into.Append( main );
        } break;
    }
    _log(SE__DESTINY, "DroneSE::EncodeDestiny(): %s - id:%lli, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void DroneSE::MakeDamageState(DoDestinyDamageState &into)
{
    into.shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float() + 5;
    into.timestamp = GetFileTimeNow();
    into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float());
}

void DroneSE::SetResists() {
    /* fix for missing resist attribs -allan 18April16  */
    if (!m_self->HasAttribute(AttrShieldEmDamageResonance)) m_self->SetAttribute(AttrShieldEmDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrShieldExplosiveDamageResonance)) m_self->SetAttribute(AttrShieldExplosiveDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrShieldKineticDamageResonance)) m_self->SetAttribute(AttrShieldKineticDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrShieldThermalDamageResonance)) m_self->SetAttribute(AttrShieldThermalDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrArmorEmDamageResonance)) m_self->SetAttribute(AttrArmorEmDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrArmorExplosiveDamageResonance)) m_self->SetAttribute(AttrArmorExplosiveDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrArmorKineticDamageResonance)) m_self->SetAttribute(AttrArmorKineticDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrArmorThermalDamageResonance)) m_self->SetAttribute(AttrArmorThermalDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrEmDamageResonance)) m_self->SetAttribute(AttrEmDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrExplosiveDamageResonance)) m_self->SetAttribute(AttrExplosiveDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrKineticDamageResonance)) m_self->SetAttribute(AttrKineticDamageResonance, EvilOne, false);
    if (!m_self->HasAttribute(AttrThermalDamageResonance)) m_self->SetAttribute(AttrThermalDamageResonance, EvilOne, false);
}

void DroneSE::Killed(Damage &fatal_blow) {
    if ((m_bubble == nullptr) or (m_destiny == nullptr) or (m_system == nullptr))
        return; // make error here?

    uint32 killerID = 0;
    Client* pClient(nullptr);
    SystemEntity *killer(fatal_blow.srcSE);

    if (killer->HasPilot()) {
        pClient = killer->GetPilot();
        killerID = pClient->GetCharacterID();
    } else if (killer->IsDroneSE()) {
        pClient = sEntityMgr.FindClientByCharID( killer->GetSelf()->ownerID() );
        if (pClient == nullptr) {
            sLog.Error("DroneSE::Killed()", "killer == IsDrone and pPlayer == nullptr");
        } else {
            killerID = pClient->GetCharacterID();
        }
    } else {
        killerID = killer->GetID();
    }

    if (pClient != nullptr) {
        //award kill bounty.
        AwardBounty( pClient );
        if (m_system->GetSystemSecurityRating() > 0)
            AwardSecurityStatus(m_self, pClient->GetChar().get());  // this awards secStatusChange for npcs in empire space
    }

    GPoint wreckPosition = m_destiny->GetPosition();
    if (wreckPosition.isNaN()) {
        sLog.Error("DroneSE::Killed()", "Wreck Position is NaN");
        return;
    }

    uint32 wreckTypeID = 26972;   //Faction Drone Wreck -- best i can find.

    std::string wreck_name = m_self->itemName();
    wreck_name += " Wreck";
    ItemData wreckItemData(wreckTypeID, killerID, m_system->GetID(), flagNone, wreck_name.c_str(), wreckPosition, itoa(m_allyID));
    WreckContainerRef wreckItemRef = sItemFactory.SpawnWreckContainer( wreckItemData );
    if (wreckItemRef.get() == nullptr) {
        sLog.Error("DroneSE::Killed()", "Creating Wreck Item Failed for %s of type %u", wreck_name.c_str(), wreckTypeID);
        return;
    }

    if (is_log_enabled(PHYSICS__TRACE))
        _log(PHYSICS__TRACE, "DroneSE::Killed() - Drone %s(%u) Position: %.2f,%.2f,%.2f.  Wreck %s(%u) Position: %.2f,%.2f,%.2f.", \
        GetName(), GetID(), x(), y(), z(), wreckItemRef->name(), wreckItemRef->itemID(), wreckPosition.x, wreckPosition.y, wreckPosition.z);

    DBSystemDynamicEntity wreckEntity = DBSystemDynamicEntity();
        wreckEntity.allianceID = (killer->GetAllianceID() == 0 ? m_allyID : killer->GetAllianceID());
        wreckEntity.categoryID = EVEDB::invCategories::Celestial;
        wreckEntity.corporationID = killer->GetCorporationID();
        wreckEntity.factionID = (killer->GetWarFactionID() == 0 ? m_warID : killer->GetWarFactionID());
        wreckEntity.groupID = EVEDB::invGroups::Wreck;
        wreckEntity.itemID = wreckItemRef->itemID();
        wreckEntity.itemName = std::move(wreck_name);
        wreckEntity.ownerID = killerID;
        wreckEntity.typeID = wreckTypeID;
        wreckEntity.position = wreckPosition;

    if (!m_system->BuildDynamicEntity(wreckEntity, m_self->itemID())) {
        sLog.Error("DroneSE::Killed()", "Spawning Wreck Failed for typeID %u", wreckTypeID);
        wreckItemRef->Delete();
        return;
    }
    m_destiny->SendJettisonPacket();
}
