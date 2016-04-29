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
    Updates:        Allan
*/

#include "system/Damage.h"

#include "Client.h"
#include "EVEServerConfig.h"
#include "npc/NPC.h"
#include "npc/NPCAI.h"
#include "ship/Ship.h"
#include "system/SystemBubble.h"
#include "system/LootSystem.h"

Damage::Damage(
    SystemEntity *_source,
    InventoryItemRef _weapon,
    double _kinetic,
    double _thermal,
    double _em,
    double _explosive,
    double _modifier,
    EVEEffectID _effect): source(_source), charge(), effect(_effect), modifier(_modifier)
{
	em        = _em;
	weapon    = _weapon;
    kinetic   = _kinetic;
    thermal   = _thermal;
    explosive = _explosive;
}

Damage::Damage(
    SystemEntity *_source,
    bool fatal_blow): source(_source), effect(effectTargetAttack)
{
	assert(fatal_blow && "Damage() constructor meant for fatal_blow called without 2nd param being true!");

	// No specific damage dealt here, just killed
    em = 0.0;
	kinetic = 0.0;
	thermal = 0.0;
	explosive = 0.0;

	// These are set to NULL for this specific case of Damage obj meant for Killed() methods of derived SystemEntity objects
    weapon = InventoryItemRef();
    charge = InventoryItemRef();
}

Damage::Damage(
    SystemEntity *_source,
    InventoryItemRef _weapon,
    EVEEffectID _effect): source(_source),  weapon(_weapon), effect(_effect)
    {
        em = _weapon->GetAttribute(AttrEmDamage).get_float();
        kinetic = _weapon->GetAttribute(AttrKineticDamage).get_float();
        thermal = _weapon->GetAttribute(AttrThermalDamage).get_float();
        explosive = _weapon->GetAttribute(AttrExplosiveDamage).get_float();

        charge = InventoryItemRef();
        _log(TARGET__TRACE, "Damage:Constructor - Called by source %s(%u) with weapon %s(%u).",
             source->GetName(), source->GetID(), weapon->itemName().c_str(), weapon->itemID() );
    }

Damage::Damage(
    SystemEntity *_source,
    InventoryItemRef _weapon,
    InventoryItemRef _charge,
    EVEEffectID _effect): source(_source),  weapon(_weapon), charge(_charge), effect(_effect)
    {
        em = _weapon->GetAttribute(AttrEmDamage).get_float();
        kinetic = _weapon->GetAttribute(AttrKineticDamage).get_float();
        thermal = _weapon->GetAttribute(AttrThermalDamage).get_float();
        explosive = _weapon->GetAttribute(AttrExplosiveDamage).get_float();

        _log(TARGET__TRACE, "Damage:Constructor - Called by source %s(%u) with weapon %s(%u).",
             source->GetName(), source->GetID(), weapon->itemName().c_str(), weapon->itemID() );
    }


bool SystemEntity::ApplyDamage(Damage &d) {
    _log(TARGET__TRACE, "%s(%u): Initalizing %.2f damage from %s(%u) with K:%.3f, T:%.3f, EM:%.3f, E:%.3f",\
        GetName(), GetID(), d.GetTotal(), d.source->GetName(), d.source->GetID(),\
        d.GetKinetic(), d.GetThermal(), d.GetEM(), d.GetExplosive() );

    bool killed = false;
    int8 damageID = 0;
    if (d.weapon->categoryID() == EVEDB::invCategories::Charge) {
        damageID = 4;
    } else if (d.weapon->groupID() == EVEDB::invGroups::Super_Weapon) {
        /*   TODO
         * this will need to be adjusted based on distance from target,
         *  and modified/corrected as the weapon implementation is completed.
         *  all modifiers to be calc'd in weapon code and sent here for correct damageID
         */
        damageID = 3;
    } else {
        double modifier = d.GetModifier();
        d *= modifier;
        if (modifier == 3.0)       damageID = 6;
        else if (modifier > 1.24)  damageID = 5;
        else if (modifier > 0.99)  damageID = 4;
        else if (modifier > 0.74)  damageID = 3;
        else if (modifier > 0.624) damageID = 2;
        else if (modifier > 0.50)  damageID = 1;
        else                       damageID = 0;
    }

    if (sConfig.rates.damageRate != 1.0)
        d *= sConfig.rates.damageRate;

    Damage DamageToShield = d.MultiplyDup(
        m_self->GetAttribute(AttrShieldKineticDamageResonance).get_float(),
        m_self->GetAttribute(AttrShieldThermalDamageResonance).get_float(),
        m_self->GetAttribute(AttrShieldEmDamageResonance).get_float(),
        m_self->GetAttribute(AttrShieldExplosiveDamageResonance).get_float() );

    double total_damage = 0.0;
    double shield_damage = DamageToShield.GetTotal();
    double available_shield = m_self->GetAttribute(AttrShieldCharge).get_float();
    if (shield_damage <= available_shield) {
        if (HasPilot()) {
            if (available_shield < ( 1.0  - m_self->GetAttribute(AttrShieldUniformity).get_float())) {
                /* send 1% damage to armor based on tactical shield manipulation skill
                 *  lvl 1 = 80%, 2 = 60%, 3 = 40% 4 = 20%, 5 = 0
                 */
                //float new_damage = d.GetTotal() * 0.01;
                //m_self->SetAttribute(AttrArmorDamage, new_damage);
                ;
            }
        }
        total_damage += shield_damage;
        double new_charge = available_shield - shield_damage;
        m_self->SetAttribute(AttrShieldCharge, new_charge);

        _log(TARGET__DAMAGE, "%s(%u): Applying %.2f damage to shields. New charge: %.3f.",
             GetName(), GetID(), shield_damage, new_charge);
    } else {
        // get fraction of damage partial shield absorbs, and lower total damage by that fraction
        d *= (1 - (available_shield /shield_damage));
        total_damage += available_shield;

        if (available_shield > 0) {
            _log(TARGET__TRACE, "%s(%u): Shield depleted with %.2f damage. %.2f damage remains.",
                 GetName(), GetID(), available_shield, d.GetTotal());
            m_self->SetAttribute(AttrShieldCharge, 0);
        }

        //Armor:
        double available_armor = m_self->GetAttribute(AttrArmorHP).get_float() - m_self->GetAttribute(AttrArmorDamage).get_float();
        Damage DamageToArmor = d.MultiplyDup(
            m_self->GetAttribute(AttrArmorKineticDamageResonance).get_float(),
            m_self->GetAttribute(AttrArmorThermalDamageResonance).get_float(),
            m_self->GetAttribute(AttrArmorEmDamageResonance).get_float(),
            m_self->GetAttribute(AttrArmorExplosiveDamageResonance).get_float() );

        double armor_damage = DamageToArmor.GetTotal();
        if (armor_damage <= available_armor) {
            if (HasPilot()) {
                if ( available_armor < ( 1.0  - m_self->GetAttribute(AttrArmorUniformity).get_float()) ) {
                    //float new_damage = d.GetTotal() * 0.01;
                    //m_self->SetAttribute(AttrDamage, new_damage);
                    ;
                }
            }
            total_damage += armor_damage;
            EvilNumber new_damage = m_self->GetAttribute(AttrArmorDamage) + EvilNumber(armor_damage);
            m_self->SetAttribute(AttrArmorDamage, new_damage);
            _log(TARGET__DAMAGE, "%s(%u): Applying %.2f damage to armor. New armor damage: %.2f",
                 GetName(), GetID(), armor_damage, new_damage.get_float());
        } else {
            d *= (1 - (available_armor /armor_damage));
            total_damage += available_armor;

            if (available_armor > 0) {
                _log(TARGET__TRACE, "%s(%u): Armor depleated with %.2f damage. %.2f damage remains.",
                     GetName(), GetID(), available_armor, d.GetTotal());
                m_self->SetAttribute(AttrArmorDamage, m_self->GetAttribute(AttrArmorHP));
            }

            //Hull/Structure:
            //The base hp and damage attributes represent structure.
            double available_hull = m_self->GetAttribute(AttrHP).get_int() - m_self->GetAttribute(AttrDamage).get_float();
            Damage DamageToHull = d.MultiplyDup(
                m_self->GetAttribute(AttrKineticDamageResonance).get_float(),
                m_self->GetAttribute(AttrThermalDamageResonance).get_float(),
                m_self->GetAttribute(AttrEmDamageResonance).get_float(),
                m_self->GetAttribute(AttrExplosiveDamageResonance).get_float() );

            double hull_damage = DamageToHull.GetTotal();
            if (hull_damage < available_hull) {
                total_damage += hull_damage;
                EvilNumber new_damage = m_self->GetAttribute(AttrDamage) + EvilNumber(hull_damage);
                m_self->SetAttribute(AttrDamage, new_damage);
                _log(TARGET__DAMAGE, "%s(%u): Applying %.2f damage to structure. New structure damage: %.2f",
                     GetName(), GetID(), hull_damage, new_damage.get_float());
            } else {
                total_damage += available_hull;
                //dead....
                _log(TARGET__TRACE, "%s(%u): %.2f damage has depleated our structure. Time to explode.",
                     GetName(), GetID(), hull_damage);
                killed = true;
                m_self->SetAttribute(AttrDamage, m_self->GetAttribute(AttrHP));
            }
            /** @todo (allan) deal with damaging modules. no idea the mechanics on this yet.  */
        }
    }

    PyTuple* up;
    /*    def OnDamageMessage(self, msgKey, args):
     * found in /eve/client/script/environment/godma.py
     */
    if (HasPilot()) {
        // working  22Apr15
        if (d.weapon->categoryID() != EVEDB::invCategories::Charge) {
            //Notification to bubble?
            Notify_OnEffectHit noeh;
                noeh.itemID = d.source->GetID();
                noeh.effectID = d.effect;
                noeh.targetID = GetID();
                noeh.damage = total_damage;
            up = noeh.Encode();
            QueueDestinyEvent(&up);
        }

        //  notify player of damage done by other
        Notify_OnDamageMessage_Self ondam;
            ondam.messageID = DamageMessageIDs_Self[damageID];
            ondam.source = d.source->GetID();
            ondam.splash = "";
            ondam.damage = total_damage;
        up = ondam.Encode();
        QueueDestinyEvent(&up);
    }

    if (d.source->HasPilot()) {     //not working
        //if (d.weapon->categoryID() != EVEDB::invCategories::Charge) {
        if (1) {
            //Notifications to ourself:
            Notify_OnEffectHit noeh;
                noeh.itemID = d.source->GetID();
                noeh.effectID = d.effect;
                noeh.targetID = GetID();
                noeh.damage = total_damage;
            up = noeh.Encode();
            d.source->QueueDestinyEvent(&up);
        }
/*
        //  notify player of damage done to other
        Notify_OnDamageMessage ondam;
            ondam.messageID = DamageMessageIDs_Other[damageID];
            ondam.weapon = d.weapon->itemID();
            ondam.splash = "";
            ondam.target = GetID();
            ondam.damage = total_damage;
        up = ondam.Encode();
        d.source->QueueDestinyEvent(&up);
*/
        //Notifications to others:
        // this displays msg, but text is missing.
        Notify_OnDamageMessage_Other ondamo;
            ondamo.messageID = DamageMessageIDs_Other[damageID];
            ondamo.format_type = fmtMapping_itemTypeName;
            ondamo.weaponType = d.weapon->typeID();
            ondamo.damage = total_damage;
            ondamo.target = GetID();
            ondamo.splash = "";
        up = ondamo.Encode();
        d.source->QueueDestinyEvent(&up);
    }

    if (killed) {
        sLog.Magenta("Damage::ApplyDamage"," Entity %s(%u) killed.",GetName(), GetID());
        if (m_targMgr)
            m_targMgr->ClearAllTargets(false);
        Killed(d);
    } else {
        if (d.source->HasPilot())   //update this to use targetmanager's queue tb destiny event method.
            SendDamageStateChanged(d.source);
    }

    return killed;
}

void NPC::Killed(Damage &fatal_blow) {
    if (!SysBubble() || !DestinyMgr()) return;

    DestinyMgr()->Halt();

    SystemEntity *killer = fatal_blow.source;
    Client* pClient = nullptr;
    uint32 killerID = 0;

    if (killer->HasPilot()) {
        pClient = killer->GetPilot();
        killerID = pClient->GetCharacterID();
    } else if (killer->IsDroneSE()) {
        pClient = sEntityList.FindClientByCharID( killer->GetSelf()->ownerID() );
        if (!pClient ) {
            sLog.Error("NPC::Killed()", "killer == IsDrone and pPlayer == nullptr");
        } else
            killerID = pClient->GetCharacterID();
    } else
        killerID = killer->GetID();

    DestinyMgr()->SendTerminalExplosion(GetID(), SysBubble()->GetID());

    //notify our spawn manager that we are gone.
    if (m_spawnMgr)
        m_spawnMgr->SpawnDepopped(SysBubble(), m_self->itemID());

    GPoint deadNPCPosition = DestinyMgr()->GetPosition();
    uint32 wreckTypeID = sDGM_Types_to_Wrecks_Table.GetWreckID(GetSelf()->typeID());

    std::string wreck_name = GetSelf()->itemName();
    wreck_name += " Wreck";

    ItemData wreckItemData(
        wreckTypeID,
        killerID,
        GetLocationID(),
        flagAutoFit,
        wreck_name.c_str(),
        deadNPCPosition
    );

    InventoryItemRef wreckItemRef = m_self->GetItemFactory()->SpawnItem( wreckItemData );
    if (!wreckItemRef)
        throw PyException( MakeCustomError( "Unable to spawn item of type %u.", wreckTypeID ) );

    DBSystemDynamicEntity wreckEntity;
        wreckEntity.allianceID = GetAllianceID(); /** @todo (allan) fix this after alliances are implemented */
        wreckEntity.categoryID = EVEDB::invCategories::Celestial;
        wreckEntity.corporationID = GetCorporationID();
        wreckEntity.flag = flagAutoFit;
        wreckEntity.groupID = EVEDB::invGroups::Wreck;
        wreckEntity.itemID = wreckItemRef->itemID();
        wreckEntity.itemName = wreck_name;
        wreckEntity.locationID = GetLocationID();
        wreckEntity.ownerID = killerID;
        wreckEntity.typeID = wreckTypeID;
        wreckEntity.x = deadNPCPosition.x;
        wreckEntity.y = deadNPCPosition.y;
        wreckEntity.z = deadNPCPosition.z;

    if (!SystemMgr()->BuildDynamicEntity(wreckEntity)) {
        sLog.Error("NPC::Killed()", "Spawning Wreck Failed: typeID or typeName not supported: '%u'", wreckTypeID);
        throw PyException( MakeCustomError ( "Spawning Wreck Failed: typeID or typeName not supported." ) );
        return;
    }

    _log(PHYSICS__TRACE, "NPC::Killed() - Wreck %s(%u) Item Position: %.2f,%.2f,%.2f.  Destiny Position: %.2f,%.2f,%.2f.", \
        GetName(), GetID(), x(), y(), z(), DestinyMgr()->GetPosition().x, DestinyMgr()->GetPosition().y, DestinyMgr()->GetPosition().z);

    if ( pClient ) {
        DropLoot(GetSelf()->groupID(), pClient->GetCharacterID(), wreckItemRef->itemID());
        //award kill bounty.
        AwardBounty( pClient );
        //  log faction kill in dynamic data   -allan
        Character* pChar = pClient->GetChar().get();
        pChar->chkDynamicSystemID(GetLocationID());
        pChar->AddKillToDynamicData(GetLocationID());
        pChar->AddFactionKillToDynamicData(GetLocationID());
        if (m_system->GetSystemSecurityRating() > 0)
            SystemEntity::AwardSecurityStatus(m_self, pChar);
    } else
        DropLoot(GetSelf()->groupID(), killerID, wreckItemRef->itemID());

    // cleanup and removal of dead npc
    //AI()->ClearAllTargets();
    SystemMgr()->RemoveNPC(this);  //this also removes npc from db
}

void Ship::Killed(Damage &fatal_blow) {
    if (!SysBubble() || !DestinyMgr()) return;

    SystemEntity *killer = fatal_blow.source;
    Client* pClient = nullptr;
    uint32 killerID = 0;

    if (killer->HasPilot()) {
          pClient = killer->GetPilot();
        killerID = pClient->GetCharacterID();
    } else if (killer->IsDroneSE()) {
          pClient = sEntityList.FindClientByShip(killer->GetSelf()->ownerID());
        if (!pClient ) {
            sLog.Error("Client::Killed()", "killer == IsDrone and pPlayer == nullptr");
        } else
            killerID = pClient->GetCharacterID();
    } else
        killerID = killer->GetID();

    if ( pClient && (m_system->GetSystemSecurityRating() > 0)) {
        /* http://www.eveinfo.net/wiki/ind~4067.htm
         *  relative_sec_status_penalty = base_penalty * system_truesec * (1 + (victim_sec_status - agressor_sec_status) / 90)
         *  The actual drop in security status seen by the attacker is a function of their current security status and the relative penalty:
         *  security status loss = relative_penalty * (agressor_sec_status + 10)
         */
        /** @todo (allan) check for faction/corp status modifiers here. */
        double modifier = (1 + ((m_player->GetSecurityRating() - pClient->GetSecurityRating()) /90));
        double penalty = 6.0f * m_system->GetSystemSecurityRating() * modifier;
        double loss = penalty * ( pClient->GetSecurityRating() + 10);
        if (sConfig.rates.secRate != 1.0) loss *= sConfig.rates.secRate;
          pClient->GetChar()->secStatusChange( loss );
    }

    if (!HasPilot()) {
        m_destiny->Stop();

        // Spawn a wreck for the Ship that was destroyed:
        uint32 wreckTypeID = sDGM_Types_to_Wrecks_Table.GetWreckID(GetSelf()->typeID());
        std::string wreck_name = GetSelf()->itemName();
        GPoint wreckPosition = DestinyMgr()->GetPosition();
        InventoryItemRef wreckItemRef;
        ItemData wreckItemData(
            wreckTypeID,
            killerID,
            GetLocationID(),
                               flagAutoFit,
                               wreck_name.c_str(),
                               wreckPosition
        );

        wreckItemRef = SystemMgr()->GetServiceMgr()->item_factory->SpawnItem( wreckItemData );
        if (!wreckItemRef )
            throw PyException( MakeCustomError( "Unable to spawn item of type %u.", wreckTypeID ) );

        DBSystemDynamicEntity wreckEntity;
            wreckEntity.allianceID = 0;
            wreckEntity.categoryID = EVEDB::invCategories::Celestial;
            wreckEntity.corporationID = 0;
            wreckEntity.flag = 0;
            wreckEntity.groupID = EVEDB::invGroups::Wreck;
            wreckEntity.itemID = wreckItemRef->itemID();
            wreckEntity.itemName = wreck_name;
            wreckEntity.locationID = GetLocationID();
            wreckEntity.ownerID = killerID;
            wreckEntity.typeID = wreckTypeID;
            wreckEntity.x = wreckPosition.x;
            wreckEntity.y = wreckPosition.y;
            wreckEntity.z = wreckPosition.z;

        if (!SystemMgr()->BuildDynamicEntity(wreckEntity))
        {
            sLog.Error("ShipEntity::Killed()", "Spawning Wreck Failed: typeID or typeName not supported: '%u'", wreckTypeID);
            throw PyException( MakeCustomError ( "Spawning Wreck Failed: typeID or typeName not supported." ) );
            return;
        }

        DropLoot(GetSelf()->groupID(), killerID, wreckItemRef->itemID());

        //  log faction kill in dynamic data   -allan
        //  client logs faction kills in total kills.  return is value1(total kills) - value2(faction kills) > 0:
        if (pClient) {
            Character* pChar = pClient->GetChar().get();
            pChar->chkDynamicSystemID(GetLocationID());
            pChar->AddKillToDynamicData(GetLocationID());
            pChar->AddFactionKillToDynamicData(GetLocationID());
            if (m_system->GetSystemSecurityRating() > 0)
                SystemEntity::AwardSecurityStatus(m_self, pChar);
        }

        m_system->RemoveEntity(this);
        return;
    } else if (m_player->InPod()) {
        if (!SysBubble()) return;
        if ( pClient )
               pClient->GetChar()->PayBounty(m_player->GetChar());

		GPoint deadPodPosition = GetPosition();
		uint32 oldPodItemID = m_player->GetShipID();

        DestinyMgr()->SendTerminalExplosion(oldPodItemID, SysBubble()->GetID());

        SystemMgr()->RemoveEntity(this);

        std::string corpse_name = GetName();
        corpse_name += "'s Frozen Corpse";
        uint32 corpseTypeID = 10041; // typeID from 'invTypes' table for "Frozen Corpse"
        ItemData corpseItemData(
            corpseTypeID,
            killerID,
            GetLocationID(),
            flagAutoFit,
            corpse_name.c_str(),
            deadPodPosition
        );

        InventoryItemRef corpseItemRef = m_services.item_factory->SpawnItem( corpseItemData );
        if (!corpseItemRef )
            throw PyException( MakeCustomError( "Unable to spawn item of type %u.", corpseTypeID ) );

        DBSystemDynamicEntity corpseEntity;
            corpseEntity.allianceID = 0;
            corpseEntity.categoryID = EVEDB::invCategories::Celestial;
            corpseEntity.corporationID = 0;
            corpseEntity.flag = 0;
            corpseEntity.groupID = EVEDB::invGroups::Biomass;
            corpseEntity.itemID = corpseItemRef->itemID();
            corpseEntity.itemName = corpse_name;
            corpseEntity.locationID = GetLocationID();
            corpseEntity.ownerID = 1;
            corpseEntity.typeID = corpseTypeID;
            corpseEntity.x = deadPodPosition.x;
            corpseEntity.y = deadPodPosition.y;
            corpseEntity.z = deadPodPosition.z;

        if (!SystemMgr()->BuildDynamicEntity( corpseEntity)) {
            sLog.Error("Client::Killed()", "Spawning Corpse Failed: typeID or typeName not supported: '%u'", corpseTypeID);
            throw PyException( MakeCustomError ( "Spawning Corpse Failed: typeID or typeName not supported." ) );
        }

        // this method will reset char variables to last clone state after being podded.
        //  NOTE  *** NOT TESTED YET ***
        m_player->ResetAfterPodded();
	} else {
        /** @todo (allan)  when killed while in dock queue, this DOES NOT set client variables correctly,
                meaning, it does not...
                - remove client from system,
                - remove destiny and system managers,
                - set coords on client and ship items.
            will have to look into this more later
         */

        PayInsurance();

        GPoint capsulePosition = GetPosition();
        GPoint deadShipPosition = GetPosition();
        uint32 oldShipItemID = m_player->GetShipID();
        /** @todo: figure out anybody else which may be referencing this ship */
        ShipItemRef deadShipRef = m_player->GetShip();
        DestinyMgr()->SendJettisonPacket(deadShipRef);
        DestinyMgr()->SendTerminalExplosion(oldShipItemID, SysBubble()->GetID());

		//set capsule position away from old ship:
        float radius = GetSelf()->GetAttribute(AttrRadius).get_float();
        capsulePosition.MakeRandomPointOnSphere(radius + (MakeRandomFloat(200, 400)));

        m_services.item_factory->SetUsingClient(m_player);
        ShipItemRef podRef = GetServices().item_factory->GetShip(m_player->GetPodID());
        podRef->Move(m_player->GetSystemID(), flagCapsule);
        podRef->Relocate(capsulePosition);

        SystemEntity* pPodEntity = SystemMgr()->get(m_player->GetPodID());
        if (!pPodEntity)
            pPodEntity = new Ship(podRef, m_services, SystemMgr());

        SysBubble()->Add(pPodEntity);

        m_player->BoardShip(podRef);

		uint32 wreckTypeID = sDGM_Types_to_Wrecks_Table.GetWreckID(deadShipRef->typeID());
        std::string wreck_name = GetName();
        wreck_name += "'s " + deadShipRef->itemName() + " Wreck";

		ItemData wreckItemData(
			wreckTypeID,
			killerID,
			GetLocationID(),
			flagAutoFit,
			wreck_name.c_str(),
			deadShipPosition
		);

		InventoryItemRef wreckItemRef = m_services.item_factory->SpawnItem( wreckItemData );
        m_services.item_factory->UnsetUsingClient();
		if (!wreckItemRef )
			throw PyException( MakeCustomError( "Unable to spawn wreck of type %u.", wreckTypeID ) );

		DBSystemDynamicEntity wreckEntity;
            wreckEntity.allianceID = 0;
            wreckEntity.categoryID = EVEDB::invCategories::Celestial;
            wreckEntity.corporationID = 0;
            wreckEntity.flag = flagAutoFit;
            wreckEntity.groupID = EVEDB::invGroups::Wreck;
            wreckEntity.itemID = wreckItemRef->itemID();
            wreckEntity.itemName = wreck_name;
            wreckEntity.locationID = GetLocationID();
            if ((killer->HasPilot()) || (killer->IsDroneSE()))
                wreckEntity.ownerID = killerID;
            else
                wreckEntity.ownerID = m_player->GetCharacterID();
            wreckEntity.typeID = wreckTypeID;
            wreckEntity.x = deadShipPosition.x;
            wreckEntity.y = deadShipPosition.y;
            wreckEntity.z = deadShipPosition.z;

		if (!SystemMgr()->BuildDynamicEntity(wreckEntity)) {
            sLog.Error("Client::Killed()", "Spawning Wreck Failed for typeID %u", wreckTypeID);
            //throw PyException( MakeCustomError("Unable to spawn wreck of type %u.", wreckTypeID));
			return;
		}

		/** @todo Place random selection of Ship's inventory into container of wreck */
		// For now, just transfer everything in the Ship's inventory to the wreck
		std::map<uint32, InventoryItemRef> deadShipInventory;
        deadShipInventory.clear();
        deadShipRef->GetInventory()->GetInventoryList(deadShipInventory);

	    for (auto cur : deadShipInventory)
			cur.second->Move(wreckItemRef->itemID(),flagAutoFit);

        SystemEntity* pEntity = SystemMgr()->get(oldShipItemID);
        SystemMgr()->RemoveEntity(pEntity);
        deadShipRef->Delete();
        m_player->StartKilledTimer();
    }
}
