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
#include "character/Character.h"
#include "inventory/AttributeEnum.h"
#include "ship/DestinyManager.h"
#include "station/Station.h"
#include "system/LootSystem.h"
#include "system/SystemBubble.h"
#include "system/SystemEntity.h"
#include "system/SystemManager.h"


SystemEntity::SystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system)
: m_self(self),
  m_services(services),
  m_system(system)
{
    /** @todo  figure out how to get corp/faction/ally IDs here, if possible, and add to our variables */
    m_targMgr = new TargetManager(this);
}

SystemEntity::~SystemEntity()
{
    if (m_targMgr)
        m_targMgr->DoDestruction();
    SafeDelete(m_targMgr);
}

void SystemEntity::Process() {
    /*  Enable base call to Process Targeting and Movement  */
    if (m_targMgr)
        m_targMgr->Process();
    if (m_destiny)
        m_destiny->Process();
}

double SystemEntity::GetRadius() {
    return (m_self->HasAttribute(AttrRadius) ? m_self->GetAttribute(AttrRadius).get_float() : 1.0);
}

PyTuple* SystemEntity::MakeDamageState() {
    if (IsWreckSE()) {
        WreckSE* pWE;
        DoDestinyDamageState3 ddds;
        pWE->MakeWreckState(ddds);
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
    _log(COMMON__WARNING, "MakeSlimItem for SystemEntity %s(%u)", GetName(), GetID());
    PyDict *slim = new PyDict();
        slim->SetItemString("typeID",       new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",      new PyInt(1));
        slim->SetItemString("itemID",       new PyLong(GetID()));
    return slim;
}

void SystemEntity::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;

    BallHeader head;
        head.entityID = GetID();
        head.mode = DSTBALL_RIGID;
        head.radius = GetRadius();
        head.x = x();
        head.y = y();
        head.z = z();
        head.flags = IsGlobal;
    into.Append( head );

    DSTBALL_RIGID_Struct main;
        main.formationID = 0xFF;
    into.Append( main );
    _log(COMMON__WARNING, "SystemEntity::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

double SystemEntity::DistanceTo2(const SystemEntity* other) {
    if (!other->m_bubble) return 1000000.0;
    GVector delta(GetPosition(), other->GetPosition());
    return delta.length();
}

void SystemEntity::SendDamageStateChanged(SystemEntity* source) {  //working 24Apr15
    DoDestiny_DamageDetails dmgState;
        dmgState.shield = m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float();
        dmgState.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float();
        dmgState.timestamp = Win32TimeNow();
        dmgState.armor = (1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float()));
        dmgState.structure = (1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float()));
    DoDestiny_OnDamageStateChange dmgChange;
        dmgChange.entityID = GetID();
        dmgChange.state = dmgState.Encode();
    PyTuple *up = dmgChange.Encode();
    TargetMgr()->QueueTBDestinyUpdate(&up);
    _log(TARGET__TRACE, "%s(%u): DamageUpdate - S:%f A:%f H:%f.", \
            GetName(), GetID(), dmgState.shield, dmgState.armor, dmgState.structure);
}

void SystemEntity::DropLoot(WreckContainerRef wreckRef, uint32 groupID, uint32 owner) {
    /*   allan 27Nov14    */
    std::vector<LootList> lootList;
    sDGM_Loot_Groups_Table.GetLoot(groupID, lootList);

    if (!lootList.empty()) {
        uint32 quantity = 0;
        std::vector<LootList>::iterator cur = lootList.begin();
        while (cur != lootList.end()) {
            if (cur->minDrop == cur->maxDrop)
                quantity = cur->minDrop;
            else
                quantity = (uint32)(MakeRandomInt(cur->minDrop, cur->maxDrop));
            if (quantity < 1) quantity = 1;
            ItemData iLoot(cur->itemID, owner, wreckRef->itemID(), flagAutoFit, quantity);
            wreckRef->AddItem(m_system->itemFactory()->SpawnItem(iLoot));
            ++cur;
        }
    }
    wreckRef->MakeSlimItemChange();
}

/** @todo (allan)  this doesnt need to be here */
void SystemEntity::AwardSecurityStatus(InventoryItemRef m_self, Character* pChar) {
    //New Status = ((10 - Old Status) * Sec Incr) + Old Status
    double killBonus = m_self->GetAttribute(AttrEntitySecurityStatusKillBonus).get_float();
    double oldSec = pChar->GetSecurityRating();
    double secAward = (((10 -oldSec) *killBonus) +oldSec) /100;
    secAward *=  (1 + ( 0.05 * (pChar->GetSkillLevel(skillFastTalk, true))));      // 5% increase
    if (killBonus and secAward) {
        secAward *= sConfig.rates.secRate;
        sLog.Magenta("SystemEntity::AwardSecurityStatus()"," %s(%u): killBonus: %f.  oldSec: %f.  secAward: %f.",
                     GetName(), GetID(), killBonus, oldSec, secAward);
        pChar->secStatusChange( secAward );
        std::string msg = "Status Change for killing pirates in ";
        msg += m_system->GetName();
        if (m_self->HasPilot())
            pChar->SaveStandingChanges( m_self->itemID(),  pChar->itemID(), standingCombatShipKill, secAward, msg);
        else
            pChar->SaveStandingChanges( m_self->itemID(),  pChar->itemID(), standingPirateKillSecurityStatus, secAward, msg);
    }
}

/* Static / Non-Mobile / Non-Destructable / Celestial Objects - Suns, Planets, Moons, Belts, Gates, Stations */
StaticSystemEntity::StaticSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system)
: SystemEntity(self, services, system)
{
}

bool StaticSystemEntity::LoadExtras(SystemDB *db) {
    /* set radius on static entities, where type().radius() is incorrect or not set */
    m_radius = db->GetCelestialRadius(m_self->itemID());
    return true;
}

PyDict* StaticSystemEntity::MakeSlimItem() {
    _log(COMMON__WARNING, "MakeSlimItem for StaticSystemEntity %s(%u)", GetName(), m_self->itemID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",       new PyLong(m_self->itemID()));
        slim->SetItemString("typeID",       new PyInt(m_self->typeID()));
        slim->SetItemString("name",         new PyString(m_self->itemName()));
        slim->SetItemString("nameID",       new PyNone);
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
    if (m_self->groupID() == EVEDB::invGroups::Sun)
        head.flags = IsGlobal | IsMassive;
    else
        head.flags = IsGlobal;
    into.Append( head );
    DSTBALL_RIGID_Struct main;
        main.formationID = 0xFF;
    into.Append( main );
    _log(COMMON__WARNING, "StaticSystemEntity::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

BeltSE::BeltSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system)
: StaticSystemEntity(self, services, system)
{
}

bool BeltSE::LoadExtras(SystemDB *db) {
    if (!StaticSystemEntity::LoadExtras(db))
        return false;

    SysBubble()->SetBelt(true);
    _log(DESTINY__BUBBLE_DEBUG, "BeltSE::LoadExtras() - IsBelt set to true for bubble %u.", SysBubble()->GetID() );
    return true;
}

StargateSE::StargateSE(InventoryItemRef self, PyServiceMgr &services, SystemManager* system)
: StaticSystemEntity(self, services, system)
{
}

bool StargateSE::LoadExtras(SystemDB *db) {
    if (!StaticSystemEntity::LoadExtras(db))
        return false;

    SysBubble()->SetGate(true);
    _log(DESTINY__BUBBLE_DEBUG, "StargateSE::LoadExtras() - IsGate set to true for bubble %u.", SysBubble()->GetID() );
    m_jumps = db->ListJumps(GetID());
    if (m_jumps)
        return true;

    return false;
}

PyDict* StargateSE::MakeSlimItem() {
    _log(COMMON__WARNING, "MakeSlimItem for StargateSE %s(%u)", GetName(), GetID());
    /** @todo  finish gate rotation data
    PyTuple* rotation = new PyTuple(3);
        rotation->SetItem(0, new PyFloat(0));
        rotation->SetItem(1, new PyFloat(0));
        rotation->SetItem(2, new PyFloat(0));*/
    PyDict *slim = new PyDict();
        //slim->SetItemString("dunRotation", rotation);
        slim->SetItemString("typeID",       new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",      new PyInt(1));       /** @todo (allan) make function to lookup controlling faction id for this */
        slim->SetItemString("itemID",       new PyLong(GetID()));
        slim->SetItemString("name",         new PyString(m_self->itemName()));
        slim->SetItemString("nameID",       new PyNone);
    if (m_jumps)
        slim->SetItemString("jumps", m_jumps->Clone());
    return slim;
}

/* Non-Static / Non-Mobile / Non-Destructable / Celestial Objects - Containers, Wrecks, DeadSpace */
ItemSystemEntity::ItemSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system)
: SystemEntity(self, services, system)
{
}

PyDict* ItemSystemEntity::MakeSlimItem() {
    _log(COMMON__WARNING, "MakeSlimItem for ItemSystemEntity %s(%u)", GetName(), GetID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",       new PyLong(GetID()));
        slim->SetItemString("typeID",       new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",      new PyInt(m_self->ownerID()));
        slim->SetItemString("categoryID",   new PyInt(m_self->categoryID()));
        slim->SetItemString("groupID",      new PyInt(m_self->groupID()));
        slim->SetItemString("name",         new PyString(m_self->itemName()));
        slim->SetItemString("corpID",       new PyInt(GetCorporationID()));
        slim->SetItemString("allianceID",   new PyInt(GetAllianceID()));
    return (slim);
}

void ItemSystemEntity::MakeDamageState(DoDestinyDamageState &into) {
    into.shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float();
    into.timestamp = Win32TimeNow();
    into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float());
}

DungeonSE::DungeonSE(InventoryItemRef self, PyServiceMgr &services, SystemManager *system)
: ItemSystemEntity(self, services, system)
{
    //will use effects.WarpGateEffect   ....no it wont.  the damn accel gate does.
    /*TODO  CODERS: upon entering the location of the dungeon for the first time (the one being entered)
     * the DB should be called to spawn the following in this table.
     * Just at the location being entered. not for all and only for the first time (but would reset after server restart/shutdown).
        script is a place-holder in the DB to refer to any particular activity the spawn may do on initial player entry
            (move to x,y,z RELATIVE to the location or attack target etc etc)
        location is relative to where the player warps in (which would be 0,0,0 unless scripted elsewhere)
     there needs to be something linking spawns to their location (dungeonspawnedID) once they have been spawned
        so they can be removed later. I thought this may have been an entry in entityattributes but I don't think there is a value for that..
     'spawn' is there because there are multiple typeids that are the same in many parts of a complex.
    */
}

//this is a big hack just to document the kind of stuff a dungeon conveys.
PyDict *DungeonSE::MakeSlimItem() {
    _log(COMMON__WARNING, "MakeSlimItem for DungeonSE %u", m_self->itemID());

    PyDict *slim = new PyDict();

    slim->SetItemString("itemID", new PyLong(m_self->itemID()));
    slim->SetItemString("typeID", new PyInt(12273));
    slim->SetItemString("ownerID", new PyInt(1));

    slim->SetItemString("dunSkillLevel", new PyInt(0));
    slim->SetItemString("dunSkillTypeID", new PyNone);
    slim->SetItemString("dunObjectID", new PyInt(160449));
    slim->SetItemString("dunWipeNPC", new PyInt(1));
    slim->SetItemString("dunToGateID", new PyInt(160484));
    slim->SetItemString("dunCloaked", new PyInt(0));
    slim->SetItemString("dunScenarioID", new PyInt(23));
    slim->SetItemString("dunSpawnID", new PyInt(4));
    slim->SetItemString("dunAmount", new PyFloat(0.0));
    slim->SetItemString("dunShipClasses", new PyList(/*237, 31*/));
    slim->SetItemString("dunDirection", new PyList(/*235, 0, 1*/));
    slim->SetItemString("dunKeyLock", new PyInt(0));
    //slim->SetItemString("dunKeyQuantity", new PyInt(1));
    //slim->SetItemString("dunKeyTypeID", new PyInt(21839));
    //slim->SetItemString("dunOpenUntil", new PyInt(Win32TimeNow()+Win32Time_Hour));
    slim->SetItemString("dunMusicUrl", new PyString("res:/Sound/Music/Ambient031combat.ogg"));

    return(slim);
}

void DungeonSE::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;

    BallHeader head;
        head.entityID = GetID();
        head.mode = DSTBALL_RIGID;
        head.radius = GetRadius();
        head.x = x();
        head.y = y();
        head.z = z();
        head.flags = IsInteractive;
    into.Append( head );
    DSTBALL_STOP_Struct main;
        main.formationID = 0xFF;
    into.Append( main );

    _log(COMMON__WARNING, "DungeonSE::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}


/* Non-Static / Non-Mobile / Destructable / Celestial Objects - POS Structures, Outposts, Ships, Asteroids */
ObjectSystemEntity::ObjectSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system)
: SystemEntity(self, services, system)
{
}

void ObjectSystemEntity::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;

    BallHeader head;
        head.entityID = GetID();
        head.mode = DSTBALL_RIGID;
        head.radius = GetRadius();
        head.x = x();
        head.y = y();
        head.z = z();
        head.flags = IsMassive | IsInteractive;
    into.Append( head );
    DSTBALL_RIGID_Struct main;
        main.formationID = 0xFF;
    into.Append( main );

    _log(COMMON__WARNING, "DeployableEntity::EncodeDestiny(): %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

PyDict* ObjectSystemEntity::MakeSlimItem() {
    _log(COMMON__WARNING, "MakeSlimItem for ObjectSystemEntity %s(%u)", GetName(), GetID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",       new PyLong(GetID()));
        slim->SetItemString("typeID",       new PyInt(GetTypeID()));
        slim->SetItemString("ownerID",      new PyInt(m_self->ownerID()));
        slim->SetItemString("categoryID",   new PyInt(m_self->categoryID()));
        slim->SetItemString("groupID",      new PyInt(m_self->groupID()));
        slim->SetItemString("name",         new PyString(m_self->itemName()));
        slim->SetItemString("corpID",       new PyInt(GetCorporationID()));
        slim->SetItemString("allianceID",   new PyInt(GetAllianceID()));
    return (slim);
}

void ObjectSystemEntity::MakeDamageState(DoDestinyDamageState &into) {
    into.shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float();
    into.timestamp = Win32TimeNow();
    into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float());
}

void ObjectSystemEntity::UpdateDamage()
{
    SystemEntity::UpdateDamage();
    DoDestiny_DamageDetails dmgState;
        dmgState.shield = m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float();
        dmgState.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float();
        dmgState.timestamp = Win32TimeNow();
        dmgState.armor = 1.0 - m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float();
        dmgState.structure = 1.0 - m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float();
    DoDestiny_OnDamageStateChange dmgChange;
        dmgChange.entityID = GetID();
        dmgChange.state = dmgState.Encode();
    PyTuple *up = dmgChange.Encode();
    //source->QueueDestinyUpdate(&up);
}

void ObjectSystemEntity::Killed(Damage &fatal_blow)
{
    m_targMgr->ClearTargets(false);
    if (m_destiny && m_bubble) {
        m_destiny->Stop();
        if (IsStaticEntity())   /* never true - OSE are non-static entities */
            m_destiny->SendTerminalExplosion(GetID(), SysBubble()->GetID(), true);
        else
            m_destiny->SendTerminalExplosion(GetID(), SysBubble()->GetID());
    }

    m_system->RemoveEntity(this);
}

DeployableSE::DeployableSE(InventoryItemRef self, PyServiceMgr &services, SystemManager *system)
: ObjectSystemEntity(self, services, system)
{
}


/* Non-Static / Mobile / Destructable / Celestial Objects - PC's, NPC's, Drones, Ships, Missiles */
DynamicSystemEntity::DynamicSystemEntity(InventoryItemRef self, PyServiceMgr &services, SystemManager* system)
: SystemEntity(self, services, system)
{
}

bool DynamicSystemEntity::Load(ServiceDB &from) {
    return true;
}

PyDict *DynamicSystemEntity::MakeSlimItem() {
    _log(COMMON__WARNING, "MakeSlimItem for DynamicSystemEntity %s(%u)", GetName(), GetID());
    PyDict *slim = new PyDict();
        slim->SetItemString("itemID",       new PyLong(GetID()));
        slim->SetItemString("typeID",       new PyInt(m_self->typeID()));
        slim->SetItemString("ownerID",      new PyInt(m_self->ownerID()));
        slim->SetItemString("categoryID",   new PyInt(m_self->categoryID()));
        slim->SetItemString("groupID",      new PyInt(m_self->groupID()));
        slim->SetItemString("name",         new PyString(m_self->itemName()));
        slim->SetItemString("corpID",       new PyInt(GetCorporationID()));
        slim->SetItemString("allianceID",   new PyInt(GetAllianceID()));
    return (slim);
}

void DynamicSystemEntity::EncodeDestiny( Buffer& into )
{
    using namespace Destiny;

    BallHeader head;
        head.entityID = GetID();
        head.mode = DSTBALL_STOP;
        head.radius = GetRadius();
        head.x = x();
        head.y = y();
        head.z = z();
        head.flags = IsFree;
    into.Append( head );
    MassSector mass;
        mass.mass = m_destiny->GetMass();
        mass.cloak = 0;
        mass.Harmonic = 1.0f;
        mass.corporationID = GetCorporationID();
        mass.allianceID = GetAllianceID();
    into.Append( mass );

    _log(COMMON__WARNING, "DynamicSystemEntity::EncodeDestiny() - %s - id:%u, mode:%u, flags:0x%X", GetName(), head.entityID, head.mode, head.flags);
}

void DynamicSystemEntity::MakeDamageState(DoDestinyDamageState &into) {
    into.shield = (m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float());
    into.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float();
    into.timestamp = Win32TimeNow();
    into.armor = 1.0 - (m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float());
    into.structure = 1.0 - (m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float());
}

void DynamicSystemEntity::UpdateDamage()
{
    /** @todo (Allan) needs more work */
    SystemEntity::UpdateDamage();
    DoDestiny_DamageDetails dmgState;
        dmgState.shield = m_self->GetAttribute(AttrShieldCharge).get_float() / m_self->GetAttribute(AttrShieldCapacity).get_float();
        dmgState.recharge = m_self->GetAttribute(AttrShieldRechargeRate).get_float();
        dmgState.timestamp = Win32TimeNow();
        dmgState.armor = 1.0 - m_self->GetAttribute(AttrArmorDamage).get_float() / m_self->GetAttribute(AttrArmorHP).get_float();
        dmgState.structure = 1.0 - m_self->GetAttribute(AttrDamage).get_float() / m_self->GetAttribute(AttrHP).get_float();
    DoDestiny_OnDamageStateChange dmgChange;
        dmgChange.entityID = GetID();
        dmgChange.state = dmgState.Encode();
    PyTuple *up = dmgChange.Encode();
    //source->QueueDestinyUpdate(&up);
}

void DynamicSystemEntity::Killed(Damage &fatal_blow)
{
    m_targMgr->ClearTargets(false);
    if (m_destiny && SysBubble()) {
        m_destiny->Stop();
        if (IsStaticEntity() || IsObjectEntity())    /* these will never be true here */
            m_destiny->SendTerminalExplosion(GetID(), SysBubble()->GetID(), true);
        else
            m_destiny->SendTerminalExplosion(GetID(), SysBubble()->GetID());
    }

    m_system->RemoveEntity(this);
}

void DynamicSystemEntity::AwardBounty(Client* pClient)
{
    double bounty = m_self->GetAttribute(AttrEntityKillBounty).get_float();
    if (bounty <= 0) return;    //no bounty to award...

    if (sConfig.rates.npcBountyMultiply != 1.0) bounty *= sConfig.rates.npcBountyMultiply;

    /** @todo handle distribution to gangs. */
    /** @todo handle corp tax */

    pClient->AddBalance(bounty);

    std::string reason = "Bounty for killing pirates in ";
    reason += pClient->GetSystemName();

    if (!m_services.serviceDB().GiveCash(
                    pClient->GetCharacterID(),
                    refBounty,
                    ownerCONCORD,
                    pClient->GetCharacterID(),
                    "",    //unknown const char *argID1,
                    pClient->GetUserID(),
                    accountingKeyCash,
                    bounty,
                    pClient->GetBalance(),
                    reason.c_str() )) {
        codelog(CLIENT__ERROR, "%s: Failed to record bounty of %f from death of %u (type %u)",
                    pClient->GetName(), bounty, GetID(), m_self->typeID());
        //well.. this isnt a huge deal, so we will get over it.
    }
}

