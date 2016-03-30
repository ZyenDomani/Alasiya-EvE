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
	kinetic = _kinetic;
	thermal = _thermal;
	em = _em;
	explosive = _explosive;
	weapon = _weapon;
}

Damage::Damage(
    SystemEntity *_source,
    bool fatal_blow): source(_source), effect(effectTargetAttack)
{
	assert(fatal_blow && "Damage() constructor meant for fatal_blow called without 2nd param being true!");

	// No specific damage dealt here, just killed
	kinetic = 0.0;
	thermal = 0.0;
	em = 0.0;
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
        kinetic = _weapon->GetAttribute(AttrKineticDamage).get_float();
        thermal = _weapon->GetAttribute(AttrThermalDamage).get_float();
        em = _weapon->GetAttribute(AttrEmDamage).get_float();
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
        kinetic = _weapon->GetAttribute(AttrKineticDamage).get_float();
        thermal = _weapon->GetAttribute(AttrThermalDamage).get_float();
        em = _weapon->GetAttribute(AttrEmDamage).get_float();
        explosive = _weapon->GetAttribute(AttrExplosiveDamage).get_float();

        _log(TARGET__TRACE, "Damage:Constructor - Called by source %s(%u) with weapon %s(%u).",
             source->GetName(), source->GetID(), weapon->itemName().c_str(), weapon->itemID() );
    }

Damage::~Damage()
{
}

void SystemEntity::AwardSecurityStatus(InventoryItemRef m_self, Character* pChar) {
    //  TODO  this needs tweaking...
    //New Status = ((10 - Old Status) * Sec Incr) + Old Status
    double killBonus = m_self->GetAttribute(AttrEntitySecurityStatusKillBonus).get_float();
    killBonus /= 100;
    double oldSec = pChar->GetSecurityRating();
    double secAward = (((10 -oldSec) *killBonus) +oldSec) /100;
    secAward *=  (1 + ( 0.05 * (pChar->GetSkillLevel(skillFastTalk, true))));      // 5% increase
    if (secAward) {
        if (sConfig.rates.secRate != 1.0) secAward *= sConfig.rates.secRate;
        sLog.Magenta("SystemEntity::AwardSecurityStatus()"," %s(%u): killBonus: %f.  oldSec: %f.  secAward: %f.",
                     GetName(), GetID(), killBonus, oldSec, secAward);
        //pChar->secStatusChange( secAward );
        //std::string msg = "Status Change for killing pirates in ";
        //msg += System()->GetName();
        //pChar->SaveStandingChanges( m_self->itemID(),  pChar->itemID(), 1,  eventID,  eventType, secAward, msg);
    }
}

bool Client::ApplyDamage(Damage &d) {
    return ItemSystemEntity::ApplyDamage(d);
}

bool NPC::ApplyDamage(Damage &d) {
    return ItemSystemEntity::ApplyDamage(d);
}

bool Drone::ApplyDamage(Damage &d) {
    return ItemSystemEntity::ApplyDamage(d);
}

bool ShipEntity::ApplyDamage(Damage &d) {
    return ItemSystemEntity::ApplyDamage(d);
}

bool StructureEntity::ApplyDamage(Damage &d) {
    return ItemSystemEntity::ApplyDamage(d);
}

bool ContainerEntity::ApplyDamage(Damage &d) {
    return ItemSystemEntity::ApplyDamage(d);
}

bool DeployableEntity::ApplyDamage(Damage &d) {
    return ItemSystemEntity::ApplyDamage(d);
}

bool AsteroidEntity::ApplyDamage(Damage &d) {
    return ItemSystemEntity::ApplyDamage(d);
}

bool CelestialEntity::ApplyDamage(Damage &d) {
    return ItemSystemEntity::ApplyDamage(d);
}

bool StationEntity::ApplyDamage(Damage &d) {
    return ItemSystemEntity::ApplyDamage(d);
}

bool ItemSystemEntity::ApplyDamage(Damage &d) {
    _log(TARGET__TRACE, "%s(%u): Initalizing %.2f damage from %s(%u) with K:%.3f, T:%.3f, EM:%.3f, E:%.3f",\
         GetName(), GetID(), d.GetTotal(), d.source->GetName(), d.source->GetID(),\
        d.GetKinetic(), d.GetThermal(), d.GetEM(), d.GetExplosive() );

    bool killed = false;
    int8 damageID = 0;
    if (d.weapon->categoryID() == EVEDB::invCategories::Charge)
        damageID = 4;
    else if (d.weapon->groupID() == EVEDB::invGroups::Super_Weapon) {
        /*   TODO
         * this will need to be adjusted based on distance from target,
         *  and modified/corrected as the weapon implementation is completed.
         *  all modifiers to be calc'd in weapon source, and sent here for correct damageID
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
        if (IsClient()) {
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

        _log(TARGET__DEBUG, "%s(%u): Applying %.2f damage to shields. New charge: %.3f.",
             GetName(), GetID(), shield_damage, new_charge);
    } else {
        // get fraction of damage partial shield absorbs, and lower total damage by that fraction
        d *= (1 - (available_shield /shield_damage));
        total_damage += available_shield;

        if (available_shield > 0) {
            _log(TARGET__DEBUG, "%s(%u): Shield depleted with %.2f damage. %.2f damage remains.",
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
            if (IsClient()) {
                if ( available_armor < ( 1.0  - m_self->GetAttribute(AttrArmorUniformity).get_float()) ) {
                    //float new_damage = d.GetTotal() * 0.01;
                    //m_self->SetAttribute(AttrDamage, new_damage);
                    ;
                }
            }
            total_damage += armor_damage;
            EvilNumber new_damage = m_self->GetAttribute(AttrArmorDamage) + EvilNumber(armor_damage);
            m_self->SetAttribute(AttrArmorDamage, new_damage);
            _log(TARGET__DEBUG, "%s(%u): Applying %.2f damage to armor. New armor damage: %.2f",
                 GetName(), GetID(), armor_damage, new_damage.get_float());
        } else {
            d *= (1 - (available_armor /armor_damage));
            total_damage += available_armor;

            if (available_armor > 0) {
                _log(TARGET__DEBUG, "%s(%u): Armor depleated with %.2f damage. %.2f damage remains.",
                     GetName(), GetID(), available_armor, d.GetTotal());
                m_self->SetAttribute(AttrArmorDamage, m_self->GetAttribute(AttrArmorHP));
            }

            //Hull/Structure:
            //The base hp and damage attributes represent structure.
            double available_hull = m_self->GetAttribute(AttrHP).get_int() - m_self->GetAttribute(AttrDamage).get_float();
            Damage DamageToHull = d.MultiplyDup(
                m_self->GetAttribute(AttrHullKineticDamageResonance).get_float(),
                m_self->GetAttribute(AttrHullThermalDamageResonance).get_float(),
                m_self->GetAttribute(AttrHullEmDamageResonance).get_float(),
                m_self->GetAttribute(AttrHullExplosiveDamageResonance).get_float() );

            double hull_damage = DamageToHull.GetTotal();
            if (hull_damage < available_hull) {
                total_damage += hull_damage;
                EvilNumber new_damage = m_self->GetAttribute(AttrDamage) + EvilNumber(hull_damage);
                m_self->SetAttribute(AttrDamage, new_damage);
                _log(TARGET__DEBUG, "%s(%u): Applying %.2f damage to structure. New structure damage: %.2f",
                     GetName(), GetID(), hull_damage, new_damage.get_float());
            } else {
                total_damage += available_hull;
                //dead....
                _log(TARGET__DEBUG, "%s(%u): %.2f damage has depleated our structure. Time to explode.",
                     GetName(), GetID(), hull_damage);
                killed = true;
                m_self->SetAttribute(AttrDamage, m_self->GetAttribute(AttrHP));
            }
            //TODO: deal with damaging modules. no idea the mechanics on this.
        }
    }

    PyTuple* up;

    if (IsClient()) {
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

    if (d.source->IsClient()) {     //not working
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
        TargMgr.ClearAllTargets(false);
        Killed(d);
    } else {
        if (d.source->IsClient())   //update this to use targetmanager's queue tb destiny event method.
            _SendDamageStateChanged(d.source);
    }

    return killed;
}

void ItemSystemEntity::_SendDamageStateChanged(SystemEntity* source) {  //working 24Apr15
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
    source->QueueDestinyUpdate(&up);

    //_log(TARGET__TRACE, "%s(%u): DamageUpdate - S:%f A:%f H:%f.", GetName(), GetID(), dmgState.shield, dmgState.armor, dmgState.structure);
}

void SystemEntity::Killed(Damage &fatal_blow) {
    TargMgr.ClearTargets(false);    //I assume a client does not need this notification.
}

void DynamicSystemEntity::Killed(Damage &fatal_blow) {
    if (m_destiny && Bubble())
        if (IsStaticEntity())
            m_destiny->SendTerminalExplosion(GetID(), Bubble()->GetID(), true);
        else
            m_destiny->SendTerminalExplosion(GetID(), Bubble()->GetID());
}

void Client::Killed(Damage &fatal_blow) {

    SystemEntity *killer = fatal_blow.source;
    Client* pClient = nullptr;
    uint32 killerID = 0;

    if (killer->IsClient()) {
        pClient = killer->CastToClient();
        killerID = pClient->GetCharacterID();
    } else if (killer->IsDrone()) {
        pClient = sEntityList.FindClientByShip( killer->Item()->ownerID() );
        if (!pClient) {
            pClient = nullptr;
            sLog.Error("Client::Killed()", "killer == IsDrone and pClient == nullptr");
        } else
            killerID = pClient->GetCharacterID();
    } else
        killerID = killer->GetID();

    if (pClient && (m_system->GetSystemSecurityRating() > 0)) {
        /* http://www.eveinfo.net/wiki/ind~4067.htm
         *  relative_sec_status_penalty = base_penalty * system_truesec * (1 + (victim_sec_status - agressor_sec_status) / 90)
         *  The actual drop in security status seen by the attacker is a function of their current security status and the relative penalty:
         *  security status loss = relative_penalty * (agressor_sec_status + 10)
         */
        //TODO: check for faction/corp status modifiers here.
        double modifier = (1 + ((GetChar()->GetSecurityRating() - pClient->GetSecurityRating()) /90));
        double penalty = 6.0f * m_system->GetSystemSecurityRating() * modifier;
        double loss = penalty * (pClient->GetSecurityRating() + 10);
        if (sConfig.rates.secRate != 1.0) loss *= sConfig.rates.secRate;
        pClient->GetChar()->secStatusChange( loss );
    }

    if (InPod()) {
        if (!Bubble()) return;
        if (pClient)
            pClient->GetChar()->PayBounty(GetChar());

		GPoint deadPodPosition = GetPosition();
		uint32 oldPodItemID = GetShipID();

        Destiny()->SendTerminalExplosion(oldPodItemID, Bubble()->GetID());

        System()->RemoveClient(this);

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

        InventoryItemRef corpseItemRef = m_services.item_factory.SpawnItem( corpseItemData );
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

        if (!System()->BuildDynamicEntity( this, corpseEntity)) {
            sLog.Error("Client::Killed()", "Spawning Corpse Failed: typeID or typeName not supported: '%u'", corpseTypeID);
            throw PyException( MakeCustomError ( "Spawning Corpse Failed: typeID or typeName not supported." ) );
        }

        // this method will reset char variables to last clone state after being podded.
        //  NOTE  *** NOT TESTED YET ***
        ResetAfterPodded();
	} else {
        /*  FIXME  when killed while in dock queue, this DOES NOT set client variables correctly,
                meaning, it does not...
                - remove client from system,
                - remove destiny and system managers,
                - set coords on client and ship items.
            will have to look into this more later
         */

        GetShip()->PayInsurance();

        GPoint capsulePosition = GetPosition();
        GPoint deadShipPosition = GetPosition();
        uint32 oldShipItemID = GetShipID();
        //TODO: figure out anybody else which may be referencing this ship...
        ShipRef deadShipRef = GetShip();
        Destiny()->SendJettisonPacket(deadShipRef);
        Destiny()->SendTerminalExplosion(oldShipItemID, Bubble()->GetID());

		//set capsule position away from old ship:
        float radius = GetShip()->GetAttribute(AttrRadius).get_float();
        capsulePosition.MakeRandomPointOnSphere(radius + (MakeRandomFloat(200, 400)));

        m_services.item_factory.SetUsingClient(this);
        ShipRef podRef = services().item_factory.GetShip(GetPodID());
        podRef->Move(GetSystemID(), flagCapsule);
        podRef->Relocate(capsulePosition);

        SystemEntity* pPodEntity = System()->get(GetPodID());
        if (!pPodEntity)
            pPodEntity = new ShipEntity(podRef, System(), m_services, capsulePosition);

        Bubble()->Add(pPodEntity);

        BoardShip(podRef);

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

		InventoryItemRef wreckItemRef = m_services.item_factory.SpawnItem( wreckItemData );
        m_services.item_factory.UnsetUsingClient();
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
            if ((killer->IsClient()) || (killer->IsDrone()))
                wreckEntity.ownerID = killerID;
            else
                wreckEntity.ownerID = GetCharacterID();
            wreckEntity.typeID = wreckTypeID;
            wreckEntity.x = deadShipPosition.x;
            wreckEntity.y = deadShipPosition.y;
            wreckEntity.z = deadShipPosition.z;

		if (!(System()->BuildDynamicEntity(this, wreckEntity))) {
            sLog.Error("Client::Killed()", "Spawning Wreck Failed for typeID %u", wreckTypeID);
            //throw PyException( MakeCustomError("Unable to spawn wreck of type %u.", wreckTypeID));
			return;
		}

		// TODO: Place random selection of Ship's inventory into container of wreck
		// For now, just transfer everything in the Ship's inventory to the wreck
		std::map<uint32, InventoryItemRef> deadShipInventory;
        deadShipInventory.clear();
        deadShipRef->GetInventoryList(deadShipInventory);

	    for (auto cur : deadShipInventory)
			cur.second->Move(wreckItemRef->itemID(),flagAutoFit);

        SystemEntity* pEntity = System()->get(oldShipItemID);
        System()->RemoveEntity(pEntity);    //remove from system
        deadShipRef->Delete();    //remove from DB.
        StartKilledTimer();
    }
}

void NPC::Killed(Damage &fatal_blow) {
    if (!Bubble() || !Destiny()) return;

    Destiny()->Halt();

    SystemEntity *killer = fatal_blow.source;
    Client* pClient = nullptr;
    uint32 killerID = 0;

    if (killer->IsClient()) {
        pClient = killer->CastToClient();
        killerID = pClient->GetCharacterID();
    } else if (killer->IsDrone()) {
        pClient = sEntityList.FindClientByShip( killer->Item()->ownerID() );
        if (!pClient) {
            pClient = nullptr;
            sLog.Error("NPC::Killed()", "killer == IsDrone and pClient == nullptr");
        } else
            killerID = pClient->GetCharacterID();
    } else
        killerID = killer->GetID();

    Destiny()->SendTerminalExplosion(GetID(), Bubble()->GetID());

    //notify our spawn manager that we are gone.
    if (m_spawnMgr)
        m_spawnMgr->SpawnDepopped(Bubble(), m_self->itemID());

    GPoint deadNPCPosition = Destiny()->GetPosition();
	uint32 wreckTypeID = sDGM_Types_to_Wrecks_Table.GetWreckID(Item()->typeID());

    std::string wreck_name = Item()->itemName();
    wreck_name += " Wreck";

	ItemData wreckItemData(
		wreckTypeID,
        killerID,
		GetLocationID(),
		flagAutoFit,
		wreck_name.c_str(),
		deadNPCPosition
	);

    InventoryItemRef wreckItemRef = System()->GetServiceMgr()->item_factory.SpawnItem( wreckItemData );
	if (!wreckItemRef)
		throw PyException( MakeCustomError( "Unable to spawn item of type %u.", wreckTypeID ) );

	DBSystemDynamicEntity wreckEntity;
        wreckEntity.allianceID = GetAllianceID(); //FIXME fix this after alliances are implemented
        wreckEntity.categoryID = EVEDB::invCategories::Celestial;
        wreckEntity.corporationID = GetCorporationID();  //FIXME  this needs work.  corp id for npc, pc, and pc drones using common method
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

	if (!System()->BuildDynamicEntity(nullptr, wreckEntity)) {
		sLog.Error("NPC::Killed()", "Spawning Wreck Failed: typeID or typeName not supported: '%u'", wreckTypeID);
		throw PyException( MakeCustomError ( "Spawning Wreck Failed: typeID or typeName not supported." ) );
		return;
	}

	_log(PHYSICS__TRACE, "NPC::Killed() - Wreck %s(%u) Item Position: %.2f,%.2f,%.2f.  Destiny Position: %.2f,%.2f,%.2f.", \
        GetName(), GetID(), GetPosition().x, GetPosition().y, GetPosition().z,
        Destiny()->GetPosition().x, Destiny()->GetPosition().y, Destiny()->GetPosition().z);

	if (pClient) {
        _DropLoot(Item()->groupID(), pClient->GetCharacterID(), wreckItemRef->itemID());
        //award kill bounty.
        _AwardBounty(pClient);
        //  log faction kill in dynamic data   -allan
        Character* pChar = pClient->GetChar().get();
        pChar->chkDynamicSystemID(GetLocationID());
        pChar->AddKillToDynamicData(GetLocationID());
        pChar->AddFactionKillToDynamicData(GetLocationID());
        if (m_system->GetSystemSecurityRating() > 0)
            SystemEntity::AwardSecurityStatus(m_self, pChar);
    } else
        _DropLoot(Item()->groupID(), killerID, wreckItemRef->itemID());

    // cleanup and removal of dead npc
    //AI()->ClearAllTargets();
    System()->RemoveNPC(this);  //this also removes npc from db
}

void Drone::Killed(Damage &fatal_blow)
{
    m_destiny->Stop();

    SystemEntity *killer = fatal_blow.source;

    if (killer->IsNPC()) {
        m_system->RemoveEntity(this);
        return;
    }

    Client* pClient = sEntityList.FindClientByShip( killer->Item()->ownerID() );
    if (!pClient) {
        m_system->RemoveEntity(this);
        return;
    }

    //TODO: check for faction/corp status modifiers here.
    Character* pChar = pClient->GetChar().get();
    if (m_system->GetSystemSecurityRating() > 0)
        SystemEntity::AwardSecurityStatus(m_self, pChar);

    //_DropLoot(Item()->groupID(), fatal_blow.source->Item()->ownerID(), wreckItemRef->itemID());

    m_system->RemoveEntity(this);
}

void ShipEntity::Killed(Damage &fatal_blow)
{
    m_destiny->Stop();

    SystemEntity *killer = fatal_blow.source;
    Client* pClient = nullptr;
    uint32 killerID = 0;

    if (killer->IsClient()) {
        pClient = killer->CastToClient();
        killerID = pClient->GetCharacterID();
    } else if (killer->IsDrone()) {
        pClient = sEntityList.FindClientByShip( killer->Item()->ownerID() );
        if (!pClient) {
            pClient = nullptr;
            sLog.Error("NPC::Killed()", "killer == IsDrone and pClient == nullptr");
        } else
            killerID = pClient->GetCharacterID();
    } else
        killerID = killer->GetID();

	// Spawn a wreck for the Ship that was destroyed:
	uint32 wreckTypeID = sDGM_Types_to_Wrecks_Table.GetWreckID(Item()->typeID());
	std::string wreck_name = Item()->itemName();
	GPoint wreckPosition = Destiny()->GetPosition();
	InventoryItemRef wreckItemRef;
	ItemData wreckItemData(
		wreckTypeID,
		killerID,
		GetLocationID(),
		flagAutoFit,
		wreck_name.c_str(),
		wreckPosition
	);

	wreckItemRef = System()->GetServiceMgr()->item_factory.SpawnItem( wreckItemData );
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

	if (!(System()->BuildDynamicEntity( nullptr, wreckEntity )) )		// WARNING! Passing NULL for a client object (this is ok since BuildDynamicEntity() does not use the first argument
	{
		sLog.Error("ShipEntity::Killed()", "Spawning Wreck Failed: typeID or typeName not supported: '%u'", wreckTypeID);
		throw PyException( MakeCustomError ( "Spawning Wreck Failed: typeID or typeName not supported." ) );
		return;
	}

	_DropLoot(Item()->groupID(), killerID, wreckItemRef->itemID());

    //  log faction kill in dynamic data   -allan
    //  client logs faction kills in total kills.  return is value1(total kills) - value2(faction kills) > 0:
    if (pClient ) {
        Character* pChar = pClient->GetChar().get();
        pChar->chkDynamicSystemID(GetLocationID());
        pChar->AddKillToDynamicData(GetLocationID());
        pChar->AddFactionKillToDynamicData(GetLocationID());
        if (m_system->GetSystemSecurityRating() > 0)
            SystemEntity::AwardSecurityStatus(m_self, pChar);
    }

    m_system->RemoveEntity(this);
}

void StructureEntity::Killed(Damage &fatal_blow)
{
    m_destiny->Stop();

    SystemEntity *killer = fatal_blow.source;

    if (killer->IsNPC()) {
        m_system->RemoveEntity(this);
        return;
    }

    Client* pClient = sEntityList.FindClientByShip( killer->Item()->ownerID() );
    if (!pClient) {
        m_system->RemoveEntity(this);
        return;
    }

    //TODO: check for faction/corp status modifiers here.
    Character* pChar = pClient->GetChar().get();
    if (m_system->GetSystemSecurityRating() > 0)
        SystemEntity::AwardSecurityStatus(m_self, pChar);

    //_DropLoot(Item()->groupID(), fatal_blow.source->Item()->ownerID(), wreckItemRef->itemID());

    m_system->RemoveEntity(this);
}

void ContainerEntity::Killed(Damage &fatal_blow)
{
    m_destiny->Stop();

    SystemEntity *killer = fatal_blow.source;

    if (killer->IsNPC()) {
        m_system->RemoveEntity(this);
        return;
    }

    Client* pClient = sEntityList.FindClientByShip( killer->Item()->ownerID() );
    if (!pClient) {
        m_system->RemoveEntity(this);
        return;
    }

    //TODO: check for faction/corp status modifiers here.
    Character* pChar = pClient->GetChar().get();
    if (m_system->GetSystemSecurityRating() > 0)
        SystemEntity::AwardSecurityStatus(m_self, pChar);

    //_DropLoot(Item()->groupID(), fatal_blow.source->Item()->ownerID(), wreckItemRef->itemID());

    m_system->RemoveEntity(this);
}

void DeployableEntity::Killed(Damage &fatal_blow)
{
    m_destiny->Stop();

    SystemEntity *killer = fatal_blow.source;

    if (killer->IsNPC()) {
        m_system->RemoveEntity(this);
        return;
    }

    Client* pClient = sEntityList.FindClientByShip( killer->Item()->ownerID() );
    if (!pClient) {
        m_system->RemoveEntity(this);
        return;
    }

    //TODO: check for faction/corp status modifiers here.
    Character* pChar = pClient->GetChar().get();
    if (m_system->GetSystemSecurityRating() > 0)
        SystemEntity::AwardSecurityStatus(m_self, pChar);

    //_DropLoot(Item()->groupID(), fatal_blow.source->Item()->ownerID(), wreckItemRef->itemID());

    m_system->RemoveEntity(this);
}

void AsteroidEntity::Killed(Damage &fatal_blow)
{
    m_destiny->Stop();

    SystemEntity *killer = fatal_blow.source;

    if (killer->IsNPC()) {
        m_system->RemoveEntity(this);
        return;
    }

    Client* pClient = sEntityList.FindClientByShip( killer->Item()->ownerID() );
    if (!pClient) {
        m_system->RemoveEntity(this);
        return;
    }

    m_system->RemoveEntity(this);
}

void CelestialEntity::Killed(Damage &fatal_blow)
{
    m_destiny->Stop();

    SystemEntity *killer = fatal_blow.source;

    if (killer->IsNPC()) {
        m_system->RemoveEntity(this);
        return;
    }

    Client* pClient = sEntityList.FindClientByShip( killer->Item()->ownerID() );
    if (!pClient) {
        m_system->RemoveEntity(this);
        return;
    }

    //TODO: check for faction/corp status modifiers here.
    Character* pChar = pClient->GetChar().get();
    if (m_system->GetSystemSecurityRating() > 0)
        SystemEntity::AwardSecurityStatus(m_self, pChar);

    //_DropLoot(Item()->groupID(), fatal_blow.source->Item()->ownerID(), wreckItemRef->itemID());

    m_system->RemoveEntity(this);
}

void StationEntity::Killed(Damage &fatal_blow)
{
    m_destiny->Stop();

    SystemEntity *killer = fatal_blow.source;

    if (killer->IsNPC()) {
        m_system->RemoveEntity(this);
        return;
    }

    Client* pClient = sEntityList.FindClientByShip( killer->Item()->ownerID() );
    if (!pClient) {
        m_system->RemoveEntity(this);
        return;
    }

    //TODO: check for faction/corp status modifiers here.
    Character* pChar = pClient->GetChar().get();
    if (m_system->GetSystemSecurityRating() > 0)
        SystemEntity::AwardSecurityStatus(m_self, pChar);

    //_DropLoot(Item()->groupID(), fatal_blow.source->Item()->ownerID(), wreckItemRef->itemID());

    m_system->RemoveEntity(this);
}

void NPC::_AwardBounty(SystemEntity *who) {
    if(!who->IsClient()) {  // add check/code here for drones to get client(player)
        _log(NPC__TRACE, "Refusing to award bounty on %u to non-client %u", GetID(), who->GetID());
        return;
    }

    double bounty = m_self->GetAttribute(AttrEntityKillBounty).get_float();
    if (bounty <= 0) {
        return;    //no bounty to award...
    }

    if (sConfig.rates.npcBountyMultiply != 1.0) bounty *= sConfig.rates.npcBountyMultiply;

    //TODO: handle case where drone strikes fatal blow... bounty goes to master.
    //TODO: handle distribution to gangs.
    //TODO: handle corp tax

    Client *pClient = who->CastToClient();
    pClient->AddBalance(bounty);

    std::string reason = "Bounty for killing pirates in ";
    reason += pClient->GetSystemName();

    if(!m_services.serviceDB().GiveCash(
                                        pClient->GetID(),
                                        refBounty,
                                        ownerCONCORD,
                                        pClient->GetID(),
                                        "",    //unknown const char *argID1,
                                        pClient->GetUserID(),
                                        accountingKeyCash,
                                        bounty,
                                        pClient->GetBalance(),
                                        reason.c_str()
                                        )) {
        codelog(CLIENT__ERROR, "%s: Failed to record bountry of %f from death of %u (type %u)", pClient->GetName(), bounty, GetID(), m_self->typeID());
        //well.. this isnt a huge deal, so we will get over it.
    }
}

void Client::_DropLoot(uint32 groupID, uint32 owner, uint32 locationID) {
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
                quantity = static_cast<uint32>(MakeRandomInt(cur->minDrop, cur->maxDrop));
            if (quantity < 1) quantity = 1;
            ItemData iLoot(cur->itemID, owner, locationID, flagAutoFit, quantity);
            m_system->itemFactory().SpawnItem(iLoot);
            ++cur;
        }
    }
}

void NPC::_DropLoot(uint32 groupID, uint32 owner, uint32 locationID) {
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
                quantity = static_cast<uint32>(MakeRandomInt(cur->minDrop, cur->maxDrop));
            if (quantity < 1) quantity = 1;
            ItemData iLoot(cur->itemID, owner, locationID, flagAutoFit, quantity);
            m_system->itemFactory().SpawnItem(iLoot);
            ++cur;
        }
    }
}

void ShipEntity::_DropLoot(uint32 groupID, uint32 owner, uint32 locationID) {
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
                quantity = static_cast<uint32>(MakeRandomInt(cur->minDrop, cur->maxDrop));
            if (quantity < 1) quantity = 1;
            ItemData iLoot(cur->itemID, owner, locationID, flagAutoFit, quantity);
            m_system->itemFactory().SpawnItem(iLoot);
            ++cur;
        }
    }
}