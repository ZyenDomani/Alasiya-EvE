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
    Author:        Reve
    Updates:    Allan, AlTahir(DaVinci)
*/

#include "eve-server.h"

#include "PyServiceMgr.h"
#include "character/Character.h"
#include "ship/Ship.h"
#include "ship/modules/mining_modules/MiningLaser.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
/*
    AttrIceHarvestCycleBonus = 780,
    AttrSpecialisationAsteroidGroup = 781,
    AttrSpecialisationAsteroidYieldMultiplier = 782,
    AttrCrystalVolatilityChance = 783,
    AttrCrystalVolatilityDamage = 784,
    AttrCrystalsGetDamaged = 786,
    AttrSpecialtyMiningAmount = 789,
    */

MiningLaser::MiningLaser( InventoryItemRef item, ShipItemRef ship )
: ActiveModule(item, ship)
{
	m_IsInitialCycle = true;
    m_cycleStartTime = 0;
    m_Stop_signal = false;
}

/*
    MINING=1
      MINING__ERROR=1
      MINING__WARNING=1
      MINING__MESSAGE=1
      MINING__DEBUG=1
      MINING__TRACE=1
*/
void MiningLaser::Activate(SystemEntity * targetEntity)
{
	// Test if respective moduleID's and moduleGroups are activated on valid target group/category.
	// Regular Miners, Deep Core Miners, Ice Harvesters and Gas Havresters are having their target groups set strictly
    if( (((m_Item->typeID() == 17482 ||
           m_Item->typeID() == 28754 ||
           m_Item->typeID() == 17912 ||
           m_Item->groupID() == 54)
           &&
           (targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Arkonor ||
        	targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Bistot ||
        	targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Crokite ||
        	targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Dark_Ochre ||
        	targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Hedbergite ||
        	targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Hemorphite ||
        	targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Jaspet ||
        	targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Kernite ||
        	targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Plagioclase ||
        	targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Pyroxeres ||
        	targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Scordite ||
        	targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Spodumain ||
        	targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Veldspar ||
        	targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Gneiss ||
        	targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Omber)) ||

    	 ((m_Item->typeID() == 12108 ||
    	   m_Item->typeID() == 18068 ||
    	   m_Item->typeID() == 24305 ||
    	   m_Item->typeID() == 28748)
    	   && (targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Mercoxit)) ||

         ((m_Item->typeID() == 16278 ||
           m_Item->typeID() == 22229 ||
           m_Item->typeID() == 28752)
           && (targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Ice)) ||

         ((m_Item->groupID() == 737)
           && (targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Harvestable_Cloud))
        ))
    {
        m_targetEntity = targetEntity;
        m_targetID = targetEntity->GetID();

        // Activate active processing component timer:
        m_Stop_signal = false;
        m_AMPC->ActivateCycle();
        m_IsInitialCycle = true;
        m_cycleStartTime = GetTimeMSeconds();

        //# _ShowCycle();
        _SetCapNeed();
    } else {
        sLog.Error( "MiningLaser::Activate()", "ERROR: Invalid target!" );
        throw PyException( MakeCustomError( "ERROR: Invalid target" ) );
    }
}

void MiningLaser::Deactivate()
{
    if ((m_ModuleState != MOD_ACTIVATED) || (m_ModuleState == MOD_OFFLINE)) return;


    if ((((m_Item->typeID() == 17482 ||
           m_Item->typeID() == 28754 ||
           m_Item->typeID() == 17912 ||
           m_Item->groupID() == 54)
           &&
           (m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Arkonor ||
         	m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Bistot ||
         	m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Crokite ||
         	m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Dark_Ochre ||
         	m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Hedbergite ||
         	m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Hemorphite ||
         	m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Jaspet ||
         	m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Kernite ||
         	m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Plagioclase ||
         	m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Pyroxeres ||
         	m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Scordite ||
         	m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Spodumain ||
         	m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Veldspar ||
         	m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Gneiss ||
         	m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Omber)) ||

     	 ((m_Item->typeID() == 12108 ||
     	   m_Item->typeID() == 18068 ||
     	   m_Item->typeID() == 24305 ||
     	   m_Item->typeID() == 28748)
     	   && (m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Mercoxit)) ||

          ((m_Item->typeID() == 16278 ||
            m_Item->typeID() == 22229 ||
            m_Item->typeID() == 28752)
            && (m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Ice)) ||

          ((m_Item->groupID() == 737)
            && (m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Harvestable_Cloud))
         )){
    	double timeLeft = (m_cycleStartTime + (_GetDuration() / 1000)) - GetTimeMSeconds();
    	double fraction = 1 - (timeLeft / (_GetDuration() / 1000));
    	if (fraction < 1)
    		StopCycle(true);
    	else{
    		while (fraction > 2)
    			fraction -= 1;
    		if(fraction >= 1.0001)	// Very hacky, but seems to work so far
    			StopCycle(true);
    	}
    }
    else
        m_AMPC->DeactivateCycle();
}

double MiningLaser::DoCycle() {
	if (!m_Stop_signal){
		if ((!m_Ship->GetPilot()->GetShipSE()->SysBubble())
			|| (!m_Ship->GetPilot()->GetShipSE()->SysBubble()->GetEntity(m_targetID)) )
        	{
            	Deactivate();
            	return 0;
        	}
		if (m_chargeLoaded)
			if (!m_chargeRef->quantity())
			{
				Deactivate();
				return 0;
			}

		_ShowCycle();

		//FIXME - For now ore processing starts in the end of 2 cycle. First cycle returns nothing.

		//if (m_IsInitialCycle)
		//	m_IsInitialCycle = false;
		//else {
			// Actually pull in the ore
			if (m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Ice)
				_ProcessIceCycle();
			else if (m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Harvestable_Cloud)
				_ProcessCloudCycle();
			else
				_ProcessOreCycle();
        	//}

        return _GetDuration();
	}
	return 0;
}

void MiningLaser::StopCycle(bool abort)
{
    if (m_ModuleState != MOD_ACTIVATED) return;

    if(abort)
    	m_Stop_signal = true;

    m_ModuleState = MOD_DEACTIVATING;
    double timeLeft = 2000;
	if (!abort)
        timeLeft = GetTimeMSeconds() - m_cycleStartTime;

	if(abort){
		if (m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Ice)
			_ProcessIceCycle(abort);
		else if (m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Harvestable_Cloud)
			_ProcessCloudCycle(abort);
		else
			_ProcessOreCycle(abort);
	}

	uint32 chargeTypeID = 0;
    if (m_chargeLoaded)
        if (m_chargeRef)
            chargeTypeID = m_chargeRef->typeID();

    uint32 effectID = effectMiningLaser;
    std::string effectsString = "effects.Mining";
    if (m_Item->groupID() == EVEDB::invGroups::Gas_Cloud_Harvester) {
        effectID = effectMiningClouds;
        effectsString = "effects.CloudMining";
    }

    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
    (
        m_Ship,
        m_Item->itemID(),
        m_Item->typeID(),
        m_targetID,
        chargeTypeID,
        effectsString,
        0,
        0,
        0,
        timeLeft,
        0
    );

	// Create Destiny Updates:
	GodmaOther go;
        go.shipID = m_Ship->itemID();
        go.slotID = m_Item->flag();
        go.chargeTypeID = chargeTypeID;

    GodmaEnvironment ge;
        ge.selfID = m_Item->itemID();
        ge.charID = m_Ship->ownerID();
        ge.shipID = go.shipID;
        ge.targetID = m_targetID;
        ge.other = go.Encode();
        ge.area = new PyList;
        ge.effectID = effectID;

    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = 0;
        shipEff.active = 0;
        shipEff.environment = ge.Encode();
        shipEff.startTime = shipEff.timeNow;
        shipEff.duration = timeLeft;
        shipEff.repeat = 0;
        shipEff.error = new PyNone;

    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());

    std::vector<PyTuple*> updates;

    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

/** @todo rework this */
void MiningLaser::_ProcessOreCycle(bool partial)
{
	// Retrieve ore from target Asteroid and put into Cargo Hold
	InventoryItemRef asteroidRef = m_targetEntity->GetSelf();
	InventoryItemRef moduleRef = this->m_Item;
    double roidVolume = asteroidRef->GetAttribute(AttrQuantity).get_float();   //    AttrQuantity = 805  -float
    double oreVolume = asteroidRef->GetAttribute(AttrVolume).get_float();       //    AttrVolume = 161  -float

    // Calculate how many m3 of ore to pull from the asteroid on this cycle.
    // This is initial variable assignment and filter, that defines if module have any mining crystals loaded.
    double oreVolumeToPull = moduleRef->GetAttribute(AttrMiningAmount).get_int();  // AttrMiningAmount = 77 -int,
	if (m_chargeLoaded)  // Use mining crystal (if loaded) to multiply ore amount taken:
        oreVolumeToPull *= m_chargeRef->GetAttribute(AttrSpecialisationAsteroidYieldMultiplier).get_float();   //YieldMultiplier = 782 -float,

	// Calculate the ore volume based on the character's core skills - mining and astrogeology
    Character* pChar = m_Ship->GetPilot()->GetChar().get();
    oreVolumeToPull *= (1 + (0.05 * (pChar->GetSkillLevel(skillMining, true))));        //  5% increase in yield
    oreVolumeToPull *= (1 + (0.05 * (pChar->GetSkillLevel(skillAstrogeology, true))));   //  5% increase in yield

    //============================================================================================================================//
    // This is ship-bonus calculation section.
    // First stop - mining frigates: Tormentor, Bantam, Navitas, Burst.
    // Frigate skills give +20% yield bonus.
    if (m_Ship->typeID() == 591 /* Tormentor */ )
    	oreVolumeToPull *= (1 + (0.2 * (pChar->GetSkillLevel(skillAmarrFrigate, true))));
    else if (m_Ship->typeID() == 582 /* Bantam */ )
    	oreVolumeToPull *= (1 + (0.2 * (pChar->GetSkillLevel(skillCaldariFrigate, true))));
    else if (m_Ship->typeID() == 592 /* Navitas */ )
    	oreVolumeToPull *= (1 + (0.2 * (pChar->GetSkillLevel(skillGallenteFrigate, true))));
    else if (m_Ship->typeID() == 599 /* Burst */ )
    	oreVolumeToPull *= (1 + (0.2 * (pChar->GetSkillLevel(skillMinmatarFrigate, true))));

    // Next stop - mining cruisers: Osprey and Scythe.
    // Cruiser skills give +20% yield bonus.
    if (m_Ship->typeID() == 620 /* Osprey */ )
    	oreVolumeToPull *= (1 + (0.2 * (pChar->GetSkillLevel(skillCaldariCruiser, true))));
    else if (m_Ship->typeID() == 631 /* Scythe */ )
    	oreVolumeToPull *= (1 + (0.2 * (pChar->GetSkillLevel(skillMinmatarCruiser, true))));

    // Now the mining barges and exhumers.
    // Mining barge bonuses are distributed typically, while exhumers will have a 2-step approach.
    // As barges do not have any hardpoints, i will not hardcode the bonuses to strip miners only.
    // First off - barges.
    if (m_Ship->type().groupID() == EVEDB::invGroups::MiningBarge)
    	oreVolumeToPull *= (1 + (0.03 * (pChar->GetSkillLevel(skillMiningBarge, true))));

    // Now exhumers - first off we add the Mining Barge yield bonus.
    if (m_Ship->type().groupID() == EVEDB::invGroups::Exhumer)
    	oreVolumeToPull *= (1 + (0.03 * (pChar->GetSkillLevel(skillMiningBarge, true))));
    // Now second layer - Skiff and Hulk specialization points. Mackinaw is not added here as it is the Ice mining vessel.
    // Skiff - Mercoxit mining vessel: +60% to Mercoxit mining yield per level.
    if ((m_Ship->typeID() == 22546) &&
    	(m_Item->typeID() == 12108 ||
    	 m_Item->typeID() == 18068 ||
    	 m_Item->typeID() == 24305 ||
    	 m_Item->typeID() == 28748))
    	oreVolumeToPull *= (1 + (0.6 * (pChar->GetSkillLevel(skillExhumers))));
    // Hulk - Universal mining vessel: +3% to mining yield per level (-3% ice harvesters cycle will be worked out in ice processing)
    if (m_Ship->typeID() == 22544)
    	oreVolumeToPull *= (1 + (0.03 * (pChar->GetSkillLevel(skillExhumers))));

    // Ship-bonus calculation section end
    //============================================================================================================================//

    //FIXME  always returns 0 for now.  fix once fleets are implemented.
    /*
    if (pChar->fleetID()) {
        oreVolumeToPull *= (1 + (0.02 * (pChar->GetSkillLevel(skillMiningForeman, true))));   //  2% increase in yield
        oreVolumeToPull *= (1 + (0.03 * (pChar->GetSkillLevel(skillMiningDirector, true))));   //  3% increase in yield
    }
     */

    //FIXME - For now, aborted cycle returns the ore volume of 1 full cycle. Most likely timeLeft returns 0 here.
    if (partial) {
        double timeLeft = (m_cycleStartTime + (_GetDuration() / 1000)) - GetTimeMSeconds();
        double fraction = 1 - (timeLeft / (_GetDuration() / 1000));
        if (fraction < 1){
        	oreVolumeToPull *= fraction;
        }else{
        	while (fraction > 2)
        		fraction -= 1;
        	oreVolumeToPull *= fraction;
        }
        _log(MINING__TRACE, "Cycle aborted. Variable values: m_cycleStartTime = %lf, _GetDuration() = %lf, GetTimeMSeconds() = %lf, timeLeft = %lf, fraction = %lf, oreVolumeToPull = %lf",
        	 m_cycleStartTime, _GetDuration(), GetTimeMSeconds(), timeLeft, fraction, oreVolumeToPull);
    }else{
    	_log(MINING__TRACE, "Full cycle is ended. Adding 1-cycle worth of ore to the ship cargo");
    }

    if (oreVolumeToPull) {
        if (oreVolumeToPull > roidVolume) oreVolumeToPull = roidVolume;
        double remainingCargoVolume = m_Ship->GetRemainingVolumeByFlag(flagCargoHold);
        double oreAmount = oreVolumeToPull /oreVolume;
        _log(MINING__TRACE, "Processing the ore: oreVolumeToPull = %lf, roidVolume = %lf, remainingCargoVolume = %lf, oreAmount = %lf",
        		oreVolumeToPull, roidVolume, remainingCargoVolume, oreAmount);
        if (oreVolumeToPull < oreVolume){
        	oreVolumeToPull = oreVolume;
        	if (remainingCargoVolume < oreVolumeToPull) {
        		oreAmount = remainingCargoVolume /oreVolume;
        		// Not enough cargo space, so module should deactivate and not pull anymore ore from the asteroid
        		m_ModuleState = MOD_DEACTIVATING;
        		m_AMPC->DeactivateCycle();
        	}
        } else {
        	if (remainingCargoVolume < oreVolumeToPull) {
        		oreAmount = remainingCargoVolume /oreVolume;
        		// Not enough cargo space, so module should deactivate and not pull anymore ore from the asteroid
        		m_ModuleState = MOD_DEACTIVATING;
        		m_AMPC->DeactivateCycle();
        	}
        }

        if(oreAmount < 1)
        	return;

        ItemData idata(
            asteroidRef->typeID(),
            m_Ship->ownerID(),
            0, //temp location
            flagCargoHold,
            oreAmount
        );

        InventoryItemRef ore = m_Ship->GetItemFactory()->SpawnItem( idata );
        if (ore) {
            /** @todo change these to new format and stack items after addition */
            m_Ship->AddItem(flagCargoHold, ore);
            roidVolume -= oreVolumeToPull;
            asteroidRef->SetAttribute( AttrQuantity, roidVolume );
            // need to update ship hold after adding ore...
        } else {
            sLog.Error( "MiningLaser::DoCycle()", "ERROR: Could not create ore stack for '%s' ship (id %u)!",
                        m_Ship->itemName().c_str(), m_Ship->itemID() );
        }

		if (!roidVolume) {
            // Asteroid is empty
            //deactivate module
            Deactivate();
			// remove Asteroid
			m_targetEntity->SystemMgr()->RemoveEntity(m_targetEntity);
			m_targetEntity->GetSelf()->Delete();
		}
	}
	else
		sLog.Warning( "MiningLaser::DoCycle()", "Somehow MiningLaser could not extract ore from current target asteroid '%s' (id %u)",
                      m_targetEntity->GetSelf()->itemName().c_str(), m_targetEntity->GetID() );
}

void MiningLaser::_ProcessCloudCycle(bool partial)
{

}

void MiningLaser::_ProcessIceCycle(bool partial)
{
	// Variables definition
	InventoryItemRef asteroidRef = m_targetEntity->GetSelf();
	InventoryItemRef moduleRef = this->m_Item;
	double roidVolume = asteroidRef->GetAttribute(AttrQuantity).get_float();   //    AttrQuantity = 805  -float
	double iceVolume = asteroidRef->GetAttribute(AttrVolume).get_float();       //    AttrVolume = 161  -float
	double iceVolumeToPull = moduleRef->GetAttribute(AttrMiningAmount).get_int();  // AttrMiningAmount = 77 -int

	// Ship bonus filtering
	// Mackinaw: +100% yield.
	if (m_Ship->typeID() == 22548)
		iceVolumeToPull *= 2;

	// Applying the same filter. In case of Mackinaw, it should give 1 piece of ore if harvester went through over half a cycle
	if (partial) {
	        double timeLeft = (m_cycleStartTime + (_GetDuration() / 1000)) - GetTimeMSeconds();
	        double fraction = 1 - (timeLeft / (_GetDuration() / 1000));
	        if (fraction < 1){
	        	iceVolumeToPull *= fraction;
	        }else{
	        	while (fraction > 2)
	        		fraction -= 1;
	        	iceVolumeToPull *= fraction;
	        }
	        _log(MINING__TRACE, "Cycle aborted. Variable values: m_cycleStartTime = %lf, _GetDuration() = %lf, GetTimeMSeconds() = %lf, timeLeft = %lf, fraction = %lf, oreVolumeToPull = %lf",
	        	 m_cycleStartTime, _GetDuration(), GetTimeMSeconds(), timeLeft, fraction, iceVolumeToPull);
	    }else{
	    	_log(MINING__TRACE, "Full cycle is ended. Adding 1-cycle worth of ore to the ship cargo");
	    }

	if (iceVolumeToPull) {
	    if (iceVolumeToPull > roidVolume) iceVolumeToPull = roidVolume;
	    double remainingCargoVolume = m_Ship->GetRemainingVolumeByFlag(flagCargoHold);
	    double iceAmount = iceVolumeToPull /iceVolume;
	    _log(MINING__TRACE, "Processing ice. Variable values: remainingCargoVolume = %lf, iceAmount = %lf",
	    	 remainingCargoVolume, iceAmount);
	    if (iceVolumeToPull < iceVolume){
	    	_log(MINING__TRACE, "Pull volume is less than ice volume. Breaking the sequence.");
	    	return;
	    } else {
	    	if (remainingCargoVolume < iceVolumeToPull) {
	    		if (remainingCargoVolume >= 1000){
	    			iceAmount = remainingCargoVolume /iceVolume;
	    			_log(MINING__TRACE, "Partial Ice Processing due to low cargo space. Variable values: remainingCargoVolume = %lf, iceAmount = %lf",
	    				 remainingCargoVolume, iceAmount);
	    			// Not enough cargo space, so module should deactivate and not pull anymore ore from the asteroid
	    			m_ModuleState = MOD_DEACTIVATING;
	    			m_AMPC->DeactivateCycle();
	    		} else {
	    			_log(MINING__TRACE, "Not enough volume for a single piece of ice. Breaking the sequence.");
	    			return;
	    		}
	    	}
	    }

	    if(iceAmount < 1)
	    	return;

	    ItemData idata(
	    asteroidRef->typeID(),
	    m_Ship->ownerID(),
	    0, //temp location
	    flagCargoHold,
	    iceAmount
	    );

        InventoryItemRef ore = m_Ship->GetItemFactory()->SpawnItem( idata );
        if (ore) {
            /** @todo change these to new format and stack items after addition */
	       m_Ship->AddItem(flagCargoHold, ore);
	       roidVolume -= iceVolumeToPull;
	       asteroidRef->SetAttribute( AttrQuantity, roidVolume );
	       // need to update ship hold after adding ore...
	    } else {
	        sLog.Error( "MiningLaser::DoCycle()", "ERROR: Could not create ore stack for '%s' ship (id %u)!",
	                    m_Ship->itemName().c_str(), m_Ship->itemID() );
	    }

	    if (!roidVolume) {
	        // Asteroid is empty
	        //deactivate module
	        Deactivate();
	        // remove Asteroid
			m_targetEntity->SystemMgr()->RemoveEntity(m_targetEntity);
			m_targetEntity->GetSelf()->Delete();
		}
	} else
		sLog.Warning( "MiningLaser::DoCycle()", "Somehow MiningLaser could not extract ore from current target asteroid '%s' (id %u)",
	                   m_targetEntity->GetSelf()->itemName().c_str(), m_targetEntity->GetID() );
}

void MiningLaser::_ShowCycle()
{
    // Create Special Effect:
    uint32 chargeTypeID = 0;
    if (m_chargeLoaded)
        chargeTypeID = m_chargeRef->typeID();

    uint32 effectID = effectMiningLaser;
    std::string effectsString = "effects.Mining";
    if (m_Item->groupID() == EVEDB::invGroups::Gas_Cloud_Harvester) {
        effectID = effectMiningClouds;
        effectsString = "effects.CloudMining";
    }

    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
    (
        m_Ship,
        m_Item->itemID(),
        m_Item->typeID(),
        m_targetID,
        chargeTypeID,
        effectsString,
        0,
        1,
        1,
        _GetDuration(),
        1
    );

    // Create Destiny Updates:
    GodmaOther go;
        go.shipID = m_Ship->itemID();
        go.slotID = m_Item->flag();
        go.chargeTypeID = chargeTypeID;

    GodmaEnvironment ge;
        ge.selfID = m_Item->itemID();
        ge.charID = m_Ship->ownerID();
        ge.shipID = go.shipID;
        ge.targetID = m_targetID;
        ge.other = go.Encode();
        ge.area = new PyList;
        ge.effectID = effectID;

    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = 1;
        shipEff.active = 1;
        shipEff.environment = ge.Encode();
        shipEff.startTime = shipEff.timeNow;
        shipEff.duration = _GetDuration();
        shipEff.repeat = 1;  /* boolean of repeatable cycles without pilot activation */
        shipEff.error = new PyNone;

    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());

    std::vector<PyTuple*> updates;

    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

double MiningLaser::_GetDuration()
{
    Character* pChar = m_Ship->GetPilot()->GetChar().get();
    double duration = m_Item->GetAttribute(AttrDuration).get_float();

    //FIXME - For now server cycle end and client cycle indicator are not synchronized, so for now time modification is disabled.
    /*
    duration *= (1 - ( 0.01 * (pChar->GetSkillLevel(skillMining, true))));               //  1% decrease in duration
    if (m_Ship->type().groupID() == EVEDB::invGroups::MiningBarge)
        duration *= (1 - (0.01 * (pChar->GetSkillLevel(skillMiningBarge, true))));       //  1% decrease in duration
    else if (m_Ship->type().groupID() == EVEDB::invGroups::Exhumer)
        duration *= (1 - (0.02 * (pChar->GetSkillLevel(skillExhumers, true))));          //  2% decrease in duration
    if (pChar->fleetID()) {   //FIXME  always returns 0 for now.  fix once fleets are implemented.
        duration *= (1 - ( 0.02 * (pChar->GetSkillLevel(skillMiningForeman, true))));    //  2% decrease in duration
        duration *= (1 - ( 0.02 * (pChar->GetSkillLevel(skillMiningDirector, true))));   //  2% decrease in duration
    }
	*/
    return duration;
}

void MiningLaser::_SetCapNeed()
{
    // this will be needed for modules and rigs that affect cap need for mining modules
    //double need = _GetCapNeed();

}
