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

#include "eve-server.h"

#include "Client.h"
#include "Container.h"
#include "EVEServerConfig.h"
#include "PyServiceMgr.h"
#include "StatisticMgr.h"
#include "account/AccountService.h"
#include "character/Character.h"
#include "fleet/FleetService.h"
#include "inventory/AttributeEnum.h"
#include "planet/Planet.h"
#include "pos/Structure.h"
#include "standing/StandingMgr.h"
#include "system/DestinyManager.h"
#include "station/Station.h"
#include "system/SystemBubble.h"
#include "system/SystemEntity.h"
#include "system/SystemManager.h"



SystemEntity::SystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system)
: m_self(self),
  m_services(services),
  m_system(system),
  m_bubble(nullptr),
  m_destiny(nullptr),
  m_targMgr(nullptr)
{
    assert(m_system != nullptr);
    assert(m_self.get() != nullptr);

    Abandon();

    m_killed = false;

    m_radius = m_self->GetAttribute(AttrRadius).get_double();

    m_harmonic = EVEPOS::Harmonic::Inactive;

    _log(SE__DEBUG, "Created SE for item %s (%u) with radius of %.1f.", self->itemName().c_str(), self->itemID(), m_radius);
}

SystemEntity::~SystemEntity()
{
}

void SystemEntity::Process() {
    if (m_killed)
        return;

    /*  Enable base call to Process Targeting and Movement
     * this order WILL affect Point/Tackle  (kinda like on live)
     * processing target first will benefit agressor
     * processing destiny first will benefit target
     */
    if (m_targMgr != nullptr)
        m_targMgr->Process();
    if (m_destiny != nullptr)
        m_destiny->Process();
}

PyTuple* SystemEntity::MakeDamageState() {
    if (IsWreckSE()) {
        DoDestinyDamageState3 ddds;
            ddds.shield = 0;
            ddds.armor = 0;
            ddds.structure = 1.0;
        return ddds.Encode();
    }
    DoDestinyDamageState ddds;
    MakeDamageState(ddds);
    return ddds.Encode();
}

void SystemEntity::MakeDamageState(DoDestinyDamageState &into) {
    into.shield = 1;
    into.recharge = 110000;
    into.armor = 1;
    into.structure = 1;
    into.timestamp = Win32TimeNow();
}

PyDict* SystemEntity::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for SE %s(%u)", GetName(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("typeID",       new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",      new PyInt(m_ownerID));
        slim->SetItemString("itemID",       new PyLong(m_self->itemID()));
    return slim;
}

void SystemEntity::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;

    BallHeader head;
        head.entityID = m_self->itemID();
        head.mode = DSTBALL_RIGID;
        head.radius = m_radius;
        head.x = x();
        head.y = y();
        head.z = z();
        head.flags = IsGlobal;
    into.Append( head );
    DSTBALL_RIGID_Struct main;
        main.formationID = 0xFF;
    into.Append( main );
    _log(SE__DESTINY, "SE::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void SystemEntity::Killed(Damage& fatal_blow)
{
    if (m_targMgr != nullptr) {
        m_targMgr->ClearAllTargets(false);
        // loop thru list of all modules targeting this entity and let them know it has been killed.
        m_targMgr->Destroyed();
    }
}

double SystemEntity::DistanceTo2(const SystemEntity* other) {
    if (other->m_bubble == nullptr)
        return 1000000.0;
    return GetPosition().distance(other->GetPosition());
}

void SystemEntity::SendDamageStateChanged(SystemEntity* source) {  //working 24Apr15
     DamageDetails dmgState;
        dmgState.shield = m_self->GetAttribute(AttrShieldCharge).get_double() / m_self->GetAttribute(AttrShieldCapacity).get_double();
        dmgState.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_double();
        dmgState.timestamp = Win32TimeNow();
        dmgState.armor = (1.0 - (m_self->GetAttribute(AttrArmorDamage).get_double() / m_self->GetAttribute(AttrArmorHP).get_double()));
        dmgState.structure = (1.0 - (m_self->GetAttribute(AttrDamage).get_double() / m_self->GetAttribute(AttrHP).get_double()));
     OnDamageStateChange dmgChange;
        dmgChange.entityID = m_self->itemID();
        dmgChange.state = dmgState.Encode();
    PyTuple *up = dmgChange.Encode();
    if (m_targMgr != nullptr)
        m_targMgr->QueueTBDestinyUpdate(&up);
    PySafeDecRef(up);
    _log(DAMAGE__MESSAGE, "%s(%u): DamageUpdate - S:%f A:%f H:%f.", \
            GetName(), m_self->itemID(), dmgState.shield, dmgState.armor, dmgState.structure);
}

void SystemEntity::DropLoot(WreckContainerRef wreckRef, uint32 groupID, uint32 owner) {
    /*   allan 27Nov14    */
    std::vector<LootList> lootList;
    sDataMgr.GetLoot(groupID, lootList);

    if (lootList.empty())
        return;

    uint32 quantity = 0;
    std::vector<LootList>::iterator cur = lootList.begin();
    while (cur != lootList.end()) {
        if (cur->minDrop == cur->maxDrop)
            quantity = cur->minDrop;
        else
            quantity = (uint32)(MakeRandomInt(cur->minDrop, cur->maxDrop));
        if (quantity < 1) quantity = 1;
        ItemData iLoot(cur->itemID, owner, wreckRef->itemID(), flagAutoFit, quantity);
        wreckRef->AddItem(sItemFactory.SpawnItem(iLoot));
        ++cur;
    }
}

/** @todo (allan)  this doesnt need to be here */
void SystemEntity::AwardSecurityStatus(InventoryItemRef iRef, Character* pChar) {
    //New Status = ((10 - Old Status) * Sec Incr) + Old Status
    double oldSec = pChar->GetSecurityRating();
    EvilNumber maxGain = 0;
    if (iRef->HasAttribute(AttrEntitySecurityStatusKillBonus, maxGain))
        if (oldSec > maxGain.get_double())
            return;
    double killBonus = iRef->GetAttribute(AttrEntitySecurityStatusKillBonus).get_double();
    double secAward = (((10 -oldSec) *killBonus) +oldSec) /100;
    secAward *=  (1 + ( 0.05 * (pChar->GetSkillLevel(skillFastTalk, true))));      // 5% increase
    if (killBonus and secAward) {
        secAward *= sConfig.rates.secRate;
        sLog.Magenta("SystemEntity::AwardSecurityStatus()"," %s(%u): killBonus: %f.  oldSec: %f.  secAward: %f.",
                     GetName(), iRef->itemID(), killBonus, oldSec, secAward);
        pChar->secStatusChange( secAward );
        std::string msg = "Status Change for killing";
        if (iRef->HasPilot()) {
            msg += iRef->GetPilot()->GetName();
            msg += " in ";
            msg += m_system->GetName();
            sStandingMgr.UpdateStandings(iRef->itemID(), pChar->itemID(), Standings::CombatShipKill, secAward, msg);
        } else {
            msg += " pirates in ";
            msg += m_system->GetName();
            sStandingMgr.UpdateStandings(ownerCONCORD, pChar->itemID(), Standings::LawEnforcement, secAward, msg);
            // decrease standings with faction of this npc kill
            sStandingMgr.UpdateStandings(iRef->ownerID(), pChar->itemID(), Standings::CombatShipKill, -secAward, msg);
        }
    }

    /** @todo msg need work for details to appear correctly.  currently working, but could be better. (incomplete, but working)
     * see data in eve/common/script/util/eveFormat.py:300 for details
     */
}

void SystemEntity::Abandon()
{
    m_warID = -1;
    m_allyID = -1;
    m_corpID = 0;
    m_fleetID = 0;
    m_ownerID = 0;
}

/* Static / Non-Mobile / Non-Destructable / Celestial Objects - Suns, Planets, Moons, Belts, Gates, Stations */
StaticSystemEntity::StaticSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system)
: SystemEntity(self, services, system)
{
}

bool StaticSystemEntity::LoadExtras() {
    return true;
}

PyDict* StaticSystemEntity::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for SSE %s(%u)", GetName(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",       new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",       new PyInt(m_self->typeID()));
        slim->SetItemString("name",         new PyString(m_self->itemName()));
        slim->SetItemString("nameID",       PyStatic.NewNone());
        slim->SetItemString("ownerID",      new PyInt(1));
    return slim;
}

void StaticSystemEntity::EncodeDestiny( Buffer& into ) {
    using namespace Destiny;
    BallHeader head;
        head.entityID = m_self->itemID();
        head.mode = DSTBALL_RIGID;
        head.x = x();
        head.y = y();
        head.z = z();
        head.radius = m_radius;
        head.flags = IsGlobal;
    into.Append( head );
    DSTBALL_RIGID_Struct main;
        main.formationID = 0xFF;
    into.Append( main );
    _log(SE__DESTINY, "SSE::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X, radius:%.1f", GetName(), head.entityID, head.mode, head.flags, head.radius);
}

BeltSE::BeltSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system)
: StaticSystemEntity(self, services, system)
{
}

bool BeltSE::LoadExtras() {
    if (!StaticSystemEntity::LoadExtras())
        return false;

    if (m_bubble == nullptr)
        sBubbleMgr.Add(this);

    m_bubble->SetBelt(m_self);
    _log(DESTINY__BUBBLE_DEBUG, "BeltSE::LoadExtras() - IsBelt set to true for bubble %u.", m_bubble->GetID() );
    return true;
}

StargateSE::StargateSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system)
: StaticSystemEntity(self, services, system)
{
}

bool StargateSE::LoadExtras() {
    if (!StaticSystemEntity::LoadExtras())
        return false;

    if (m_bubble == nullptr)
        sBubbleMgr.Add(this);

    m_bubble->SetGate(true);
    _log(DESTINY__BUBBLE_DEBUG, "StargateSE::LoadExtras() - IsGate set to true for bubble %u.", m_bubble->GetID() );
    m_jumps = SystemDB::ListJumps(m_self->itemID());
    if (m_jumps != nullptr)
        return true;

    return false;
}

PyDict* StargateSE::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for StargateSE %s(%u)", GetName(), m_self->itemID());
    /** @todo  finish gate rotation data
    PyTuple* rotation = new PyTuple(3);
        rotation->SetItem(0, new PyFloat(0));
        rotation->SetItem(1, new PyFloat(0));
        rotation->SetItem(2, new PyFloat(0));*/
    PyDict *slim = new PyDict();
        //slim->SetItemString("dunRotation", rotation);
        slim->SetItemString("typeID",       new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",      new PyInt(1));       /** @todo (allan) make function to lookup controlling faction id for this */
        slim->SetItemString("itemID",       new PyLong(m_self->itemID()));
        slim->SetItemString("name",         new PyString(m_self->itemName()));
        slim->SetItemString("nameID",       PyStatic.NewNone());
    if (m_jumps != nullptr)
        slim->SetItemString("jumps", m_jumps->Clone());
    return slim;
}


/* Non-Static / Non-Mobile / Non-Destructable / Celestial Objects - Containers, Wrecks, DeadSpace, ForceFields, ScanProbes */
ItemSystemEntity::ItemSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system)
: SystemEntity(self, services, system)
{
    m_keyType = 0;
}

PyDict* ItemSystemEntity::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for ISE %s(%u)", GetName(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",       new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",       new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",      new PyInt(m_ownerID));
        if (m_self->groupID() == EVEDB::invGroups::Warp_Gate) {
            // this is incomplete........
            slim->SetItemString("dunSkillLevel", new PyInt(0));   //?
            slim->SetItemString("dunSkillTypeID", PyStatic.NewNone());   //?
            slim->SetItemString("dunObjectID", new PyInt(160449));  //?   902139
            slim->SetItemString("dunToGateID", new PyInt(160484));  //?   902140
            slim->SetItemString("dunCloaked", new PyBool(0));   //?
            slim->SetItemString("dunScenarioID", new PyInt(23));    //?  3347
            slim->SetItemString("dunSpawnID", new PyInt(1572));  //?
            slim->SetItemString("dunAmount", new PyFloat(0.0));  //?
            PyList* classList = new PyList();
                classList->AddItem( new PyInt(324));
                classList->AddItem( new PyInt(420));
                classList->AddItem( new PyInt(541));
                classList->AddItem( new PyInt(834));
                classList->AddItem( new PyInt(25));
                classList->AddItem( new PyInt(830));
            slim->SetItemString("dunShipClasses", classList);   //?
            PyList* dirList = new PyList();
                dirList->AddItem(new PyInt(5));     //234
                dirList->AddItem(new PyInt(-1));
                dirList->AddItem(new PyInt(0));
            slim->SetItemString("dunDirection", dirList);
            slim->SetItemString("dunKeyLock", new PyInt(0));   //?
            slim->SetItemString("dunWipeNPC", new PyBool(0));   //?
            slim->SetItemString("dunKeyQuantity", new PyInt(1));   //?
            slim->SetItemString("dunKeyTypeID", new PyInt(m_keyType));   //Training Complex Passkey   group Acceleration_Gate_Keys
            slim->SetItemString("dunOpenUntil", new PyInt(Win32TimeNow()+EvE::Time::Hour));   //?
            slim->SetItemString("dunRoomName", new PyString("Lobby"));   //?
            slim->SetItemString("dunMusicUrl", new PyString("res:/Sound/Music/Ambient031combat.ogg"));
        }
    /** @todo  finish rotation data
    Large_Collidable_Structure
    Large_Collidable_Ship
    Large_Collidable_Object
    PyTuple* rotation = new PyTuple(3);
        rotation->SetItem(0, new PyFloat(0));
        rotation->SetItem(1, new PyFloat(0));
        rotation->SetItem(2, new PyFloat(0));
    slim->SetItemString("dunRotation", rotation);
    */
    return slim;
}

void ItemSystemEntity::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;
    BallHeader head;
        head.entityID = m_self->itemID();
        head.mode = DSTBALL_RIGID;
        head.radius = m_radius;
        head.x = x();
        head.y = y();
        head.z = z();
        head.flags = 0;
    into.Append( head );
    DSTBALL_RIGID_Struct main;
        main.formationID = 0xFF;
    into.Append( main );

    _log(SE__DESTINY, "ISE::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void ItemSystemEntity::MakeDamageState(DoDestinyDamageState &into) {
    if (m_self->groupID() == EVEDB::invGroups::Force_Field) {
        SystemEntity::MakeDamageState(into);
    } else {
        into.shield = (m_self->GetAttribute(AttrShieldCharge).get_double() / m_self->GetAttribute(AttrShieldCapacity).get_double());
        into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_double();
        into.timestamp = Win32TimeNow();
        into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_double() / m_self->GetAttribute(AttrArmorHP).get_double());
        into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_double() / m_self->GetAttribute(AttrHP).get_double());
    }
}


/* Non-Static / Non-Mobile / Destructable / Celestial Objects - POS Structures, Outposts, empty Ships, Asteroids */
ObjectSystemEntity::ObjectSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system)
: SystemEntity(self, services, system)
{
    m_targMgr = new TargetManager(this);
    m_destiny = new DestinyManager(this);

    assert(m_targMgr != nullptr);
    assert(m_destiny != nullptr);
}

ObjectSystemEntity::~ObjectSystemEntity()
{
    if (m_targMgr != nullptr)
        m_targMgr->ClearAllTargets(false);
    SafeDelete(m_targMgr);
    SafeDelete(m_destiny);
}

void ObjectSystemEntity::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;
    BallHeader head;
        head.entityID = m_self->itemID();
        head.mode = DSTBALL_RIGID;
        head.radius = m_radius;
        head.x = x();
        head.y = y();
        head.z = z();
        head.flags = IsMassive;
    into.Append( head );
    DSTBALL_RIGID_Struct main;
        main.formationID = 0xFF;
    into.Append( main );

    _log(SE__DESTINY, "OSE::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

PyDict* ObjectSystemEntity::MakeSlimItem() {
    _log(SE__SLIMITEM, "MakeSlimItem for OSE %s(%u)", GetName(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",       new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",       new PyInt(GetTypeID()));
        slim->SetItemString("ownerID",      new PyInt(m_ownerID));
        slim->SetItemString("categoryID",   new PyInt(m_self->categoryID()));
        slim->SetItemString("groupID",      new PyInt(m_self->groupID()));
        slim->SetItemString("name",         new PyString(m_self->itemName()));
        slim->SetItemString("corpID",       new PyInt(m_corpID));
        slim->SetItemString("allianceID",   new PyInt(m_allyID));
        slim->SetItemString("warFactionID", new PyInt(m_warID));
    return slim;
}

void ObjectSystemEntity::MakeDamageState(DoDestinyDamageState &into) {
    into.shield = (m_self->GetAttribute(AttrShieldCharge).get_double() / m_self->GetAttribute(AttrShieldCapacity).get_double());
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_double();
    into.timestamp = Win32TimeNow();
    into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_double() / m_self->GetAttribute(AttrArmorHP).get_double());
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_double() / m_self->GetAttribute(AttrHP).get_double());
}

void ObjectSystemEntity::UpdateDamage()
{
    SystemEntity::UpdateDamage();
     DamageDetails dmgState;
        dmgState.shield = m_self->GetAttribute(AttrShieldCharge).get_double() / m_self->GetAttribute(AttrShieldCapacity).get_double();
        dmgState.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_double();
        dmgState.timestamp = Win32TimeNow();
        dmgState.armor = 1.0 - m_self->GetAttribute(AttrArmorDamage).get_double() / m_self->GetAttribute(AttrArmorHP).get_double();
        dmgState.structure = 1.0 - m_self->GetAttribute(AttrDamage).get_double() / m_self->GetAttribute(AttrHP).get_double();
     OnDamageStateChange dmgChange;
        dmgChange.entityID = m_self->itemID();
        dmgChange.state = dmgState.Encode();
    PyTuple *up = dmgChange.Encode();
    //source->QueueDestinyUpdate(&up);
}

void ObjectSystemEntity::Killed(Damage &fatal_blow)
{
    if (m_targMgr != nullptr)
        m_targMgr->ClearTargets(false);
    if (m_destiny != nullptr) {
        if (m_bubble == nullptr)
            sBubbleMgr.Add(this);
        m_destiny->Stop();
        m_destiny->SendTerminalExplosion(m_self->itemID(), m_bubble->GetID(), isGlobal());
    }

    /** @todo  test and complete this to null current customs office for this planet ... */
    if (IsCOSE()) {
        if (GetCOSE()->GetPlanetID() > 0) {
            SystemEntity* pSE = m_system->GetSE(GetCOSE()->GetPlanetID());
            pSE->GetPlanetSE()->SetCustomsOffice(nullptr);
        }
    }

    m_system->RemoveEntity(this);
}

DeployableSE::DeployableSE(InventoryItemRef self, PyServiceMgr &services, SystemManager *system, const FactionData& data)
: ObjectSystemEntity(self, services, system)
{
    m_warID = data.factionID;
    m_allyID = data.allianceID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;
}

FieldSE::FieldSE(InventoryItemRef self, PyServiceMgr &services, SystemManager *system, const FactionData& data)
: ObjectSystemEntity(self, services, system)
{
    m_warID = data.factionID;
    m_allyID = data.allianceID;
    m_corpID = data.corporationID;
    m_ownerID = data.ownerID;
}

void FieldSE::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;
    BallHeader head;
        head.entityID = m_self->itemID();
        head.mode = (m_harmonic > EVEPOS::Harmonic::Offline ? DSTBALL_FIELD : DSTBALL_STOP);
        head.radius = m_radius;
        head.x = x();
        head.y = y();
        head.z = z();
        head.flags = 0 /*(m_harmonic > EVEPOS::Harmonic::Offline ? IsMassive : 0)*/; // leave this as 0 to disable client-side bump checks for now
    into.Append( head );
    MassSector mass;
        mass.mass = 10000000000;    // as seen in packets
        mass.cloak = 0;
        mass.harmonic = m_harmonic;
        mass.corporationID = m_corpID;
        mass.allianceID = (m_allyID > 0 ? m_allyID : -1);
    into.Append( mass );
    if (head.mode == DSTBALL_FIELD) {
        DSTBALL_FIELD_Struct main;
            main.formationID = 0xFF;
        into.Append( main );
    } else if (head.mode == DSTBALL_STOP) {
        DSTBALL_STOP_Struct main;
            main.formationID = 0xFF;
        into.Append( main );
    }

    _log(SE__DESTINY, "FSE::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

PyDict *FieldSE::MakeSlimItem()
{
    return SystemEntity::MakeSlimItem();
}


/* Non-Static / Mobile / Destructable / Celestial Objects - PC's, NPC's, Drones, Ships, Missiles */
DynamicSystemEntity::DynamicSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system)
: SystemEntity(self, services, system)
{
    m_targMgr = new TargetManager(this);
    m_destiny = new DestinyManager(this);

    assert(m_targMgr != nullptr);
    assert(m_destiny != nullptr);
}

DynamicSystemEntity::~DynamicSystemEntity()
{
    if (m_targMgr != nullptr)
        m_targMgr->ClearAllTargets(false);
    SafeDelete(m_targMgr);
    SafeDelete(m_destiny);
}

bool DynamicSystemEntity::Load() {
    return true;
}

PyDict *DynamicSystemEntity::MakeSlimItem() {
    if (IsNPCSE())
        return SystemEntity::MakeSlimItem();
    _log(SE__SLIMITEM, "MakeSlimItem for DSE %s(%u)", GetName(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",       new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",       new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",      new PyInt(m_ownerID));
        slim->SetItemString("categoryID",   new PyInt(m_self->categoryID()));
        slim->SetItemString("groupID",      new PyInt(m_self->groupID()));
        slim->SetItemString("name",         new PyString(m_self->itemName()));
        slim->SetItemString("corpID",       new PyInt(m_corpID));
        slim->SetItemString("allianceID",   new PyInt(m_allyID));
        slim->SetItemString("warFactionID", new PyInt(m_warID));
    return (slim);
}

void DynamicSystemEntity::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;
    BallHeader head;
        head.entityID = m_self->itemID();
        head.mode = DSTBALL_STOP;
        head.radius = m_radius;
        head.x = x();
        head.y = y();
        head.z = z();
        head.flags = IsFree;
    into.Append( head );
    MassSector mass;
        mass.mass = m_destiny->GetMass();
        mass.cloak = (m_destiny->IsCloaked() ? 1 : 0);
        mass.harmonic = m_harmonic;
        mass.corporationID = m_corpID;
        mass.allianceID = (m_allyID > 0 ? m_allyID : -1);
    into.Append( mass );
    DataSector data;
        data.inertia = m_destiny->GetInertia();
        data.maxVelocity = m_destiny->GetMaxVelocity();
        data.velocity_x = m_destiny->GetVelocity().x;
        data.velocity_y = m_destiny->GetVelocity().y;
        data.velocity_z = m_destiny->GetVelocity().z;
        data.speedfraction = m_destiny->GetSpeedFraction();
    into.Append( data );
    DSTBALL_STOP_Struct main;
        main.formationID = 0xFF;
    into.Append( main );

    _log(SE__DESTINY, "DSE::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void DynamicSystemEntity::MakeDamageState(DoDestinyDamageState &into) {
    into.shield = (m_self->GetAttribute(AttrShieldCharge).get_double() / m_self->GetAttribute(AttrShieldCapacity).get_double());
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_double();
    into.timestamp = Win32TimeNow();
    into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_double() / m_self->GetAttribute(AttrArmorHP).get_double());
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_double() / m_self->GetAttribute(AttrHP).get_double());
}

void DynamicSystemEntity::UpdateDamage()
{
    /** @todo (Allan) needs more work */
    SystemEntity::UpdateDamage();
     DamageDetails dmgState;
        dmgState.shield = m_self->GetAttribute(AttrShieldCharge).get_double() / m_self->GetAttribute(AttrShieldCapacity).get_double();
        dmgState.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_double();
        dmgState.timestamp = Win32TimeNow();
        dmgState.armor = 1.0 - m_self->GetAttribute(AttrArmorDamage).get_double() / m_self->GetAttribute(AttrArmorHP).get_double();
        dmgState.structure = 1.0 - m_self->GetAttribute(AttrDamage).get_double() / m_self->GetAttribute(AttrHP).get_double();
     OnDamageStateChange dmgChange;
        dmgChange.entityID = m_self->itemID();
        dmgChange.state = dmgState.Encode();
    PyTuple *up = dmgChange.Encode();
    //source->QueueDestinyUpdate(&up);
}

void DynamicSystemEntity::Killed(Damage &fatal_blow)
{
    if (m_targMgr != nullptr)
        m_targMgr->ClearTargets(false);
    if (m_bubble == nullptr)
        sBubbleMgr.Add(this);
    if (m_destiny != nullptr) {
        m_destiny->Stop();
        if (IsStaticEntity() or IsObjectEntity())    /* these will never be true here */
            m_destiny->SendTerminalExplosion(m_self->itemID(), m_bubble->GetID(), true);
        else
            m_destiny->SendTerminalExplosion(m_self->itemID(), m_bubble->GetID());
    }

    m_system->RemoveEntity(this);
}

void DynamicSystemEntity::AwardBounty(Client* pClient)
{
    // this will use a map{charID/BountyData} in system manager for using a bounty timer.
    double bounty = m_self->GetAttribute(AttrEntityKillBounty).get_double();
    bounty *= sConfig.rates.npcBountyMultiply;
    if (bounty < 1)
        return;

    // add data to StatisticMgr
    sStatMgr.Add(Stat::npcBounties, bounty);

    std::string reason = "Bounty for killing a pirate in ";
    reason += pClient->GetSystemName();

    BountyData data { /* initalize all to 0 */ };
    data.fromID = m_self->itemID();
    data.toID = pClient->GetCharacterID();
    data.refTypeID = Journal::EntryType::BountyPrize;
    data.fromKey = Account::KeyType::Cash;
    data.toKey = Account::KeyType::Cash;
    data.reason = reason;

    // handle distribution to fleets
    if (pClient->InFleet()) {
        // get fleet members onGrid and distrubute bounty
        std::vector< uint32 > members;
        sFltSvc.GetFleetMembersOnGrid(pClient, members);
        // split bounty between members
        bounty /= members.size();
        // send bounty to members
        if (sConfig.server.BountyPayoutDelayed and sConfig.server.FleetShareDelayed) {
            for (auto cur :members)
                m_system->AddBounty(cur, data);
        } else {
            reason += " (FleetShare) ";
            reason += " by ";
            reason += pClient->GetName();
            data.reason = reason;
            for (auto cur :members)
                AccountService::TranserFunds(ownerCONCORD, cur, bounty, reason.c_str(), Journal::EntryType::BountyPrize, GetID());
        }
    }
    data.amount = bounty;
    if (sConfig.server.BountyPayoutDelayed)
        m_system->AddBounty(pClient->GetCharacterID(), data);
    else
        AccountService::TranserFunds(ownerCONCORD, pClient->GetCharacterID(), bounty, reason.c_str(), Journal::EntryType::BountyPrize, GetID());
}

