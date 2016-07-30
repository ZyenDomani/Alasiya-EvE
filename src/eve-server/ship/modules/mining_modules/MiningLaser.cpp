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
}

/*
    MINING=1
    MINING__ERROR=1
    MINING__WARNING=1
    MINING__MESSAGE=1
    MINING__INFO=1
    MINING__DEBUG=1
    MINING__TRACE=1
*/
void MiningLaser::Activate(SystemEntity* pSE)
{
	// Test if respective moduleID's and moduleGroups are activated on valid target group/category.
	// Regular Miners, Deep Core Miners, Ice Harvesters and Gas Havresters are having their target groups set strictly
    if (((m_Item->typeID() == 17482 or m_Item->typeID() == 28754 or m_Item->typeID() == 17912 or m_Item->groupID() == 54)
            and (pSE->GetSelf()->groupID() == EVEDB::invGroups::Arkonor or
                 pSE->GetSelf()->groupID() == EVEDB::invGroups::Bistot or
                 pSE->GetSelf()->groupID() == EVEDB::invGroups::Crokite or
                 pSE->GetSelf()->groupID() == EVEDB::invGroups::Dark_Ochre or
                 pSE->GetSelf()->groupID() == EVEDB::invGroups::Hedbergite or
                 pSE->GetSelf()->groupID() == EVEDB::invGroups::Hemorphite or
                 pSE->GetSelf()->groupID() == EVEDB::invGroups::Jaspet or
                 pSE->GetSelf()->groupID() == EVEDB::invGroups::Kernite or
                 pSE->GetSelf()->groupID() == EVEDB::invGroups::Plagioclase or
                 pSE->GetSelf()->groupID() == EVEDB::invGroups::Pyroxeres or
                 pSE->GetSelf()->groupID() == EVEDB::invGroups::Scordite or
                 pSE->GetSelf()->groupID() == EVEDB::invGroups::Spodumain or
                 pSE->GetSelf()->groupID() == EVEDB::invGroups::Veldspar or
                 pSE->GetSelf()->groupID() == EVEDB::invGroups::Gneiss or
                 pSE->GetSelf()->groupID() == EVEDB::invGroups::Omber)
        ) or ((m_Item->typeID() == 12108 or m_Item->typeID() == 18068 or m_Item->typeID() == 24305 or m_Item->typeID() == 28748)
    	    and (pSE->GetSelf()->groupID() == EVEDB::invGroups::Mercoxit)
        ) or ((m_Item->typeID() == 16278 or m_Item->typeID() == 22229 or m_Item->typeID() == 28752)
            and (pSE->GetSelf()->groupID() == EVEDB::invGroups::Ice)
        ) or ((m_Item->groupID() == 737)
            and (pSE->GetSelf()->groupID() == EVEDB::invGroups::Harvestable_Cloud))
        )
    {
        m_targetEntity = pSE;
        m_targetID = pSE->GetID();

        m_IsInitialCycle = true;
        // Activate active processing component timer:
        m_AMPC->ActivateCycle();
        m_cycleStartTime = GetTimeMSeconds();

        //# _ShowCycle();
        _SetCapNeed();
    } else {
        _log(MINING__WARNING, "Activate() - Invalid target");
        if (m_Ship->HasPilot())
            if (m_Ship->GetPilot()->CanThrow())
                throw PyException( MakeCustomError( "Invalid Target - Ref: ServerError 15628" ) );
    }
}

void MiningLaser::Deactivate()
{
    ActiveModule::AbortCycle();
}

double MiningLaser::DoCycle() {
    if ((!m_Ship->GetPilot()->GetShipSE()->SysBubble())
        or (!m_Ship->GetPilot()->GetShipSE()->SysBubble()->GetEntity(m_targetID))) {
            Deactivate();
            return 0;
        }
    if (m_chargeLoaded)
        if (!m_chargeRef->quantity()) {
            Deactivate();
            return 0;
        }

    _ShowCycle();

    //FIXME - For now ore processing starts in the end of 2 cycle. First cycle returns nothing.

    if (m_IsInitialCycle) {
    	m_IsInitialCycle = false;
    } else {
        // Actually pull in the ore
        if (m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Ice)
            _ProcessIceCycle();
        else if (m_targetEntity->GetSelf()->groupID() == EVEDB::invGroups::Harvestable_Cloud)
            _ProcessCloudCycle();
        else
            _ProcessOreCycle();
    }

    return _GetDuration();
}

/** @todo rework this */
void MiningLaser::_ProcessOreCycle(bool partial)
{
	// Retrieve ore from target Asteroid and put into Cargo Hold
	InventoryItemRef asteroidRef = m_targetEntity->GetSelf();
    double roidQuantity = asteroidRef->GetAttribute(AttrQuantity).get_float();   //    AttrQuantity = 805  -float
    double oreVolume = asteroidRef->GetAttribute(AttrVolume).get_float();       //    AttrVolume = 161  -float

    // Calculate how many m3 of ore to pull from the asteroid on this cycle.
    // This is initial variable assignment and filter, that defines if module have any mining crystals loaded.
    double oreVolumeToPull = m_Item->GetAttribute(AttrMiningAmount).get_int();  // AttrMiningAmount = 77 -int,
	if (m_chargeLoaded)  // Use mining crystal (if loaded) to multiply ore amount taken:
        oreVolumeToPull *= m_chargeRef->GetAttribute(AttrSpecialisationAsteroidYieldMultiplier).get_float();   //YieldMultiplier = 782 -float,

	// Calculate the ore volume based on the character's core skills - mining and astrogeology
    Character* pChar = m_Ship->GetPilot()->GetChar().get();
    oreVolumeToPull *= (1 + (0.05 * (pChar->GetSkillLevel(skillMining, true))));        //  5% increase in yield
    oreVolumeToPull *= (1 + (0.05 * (pChar->GetSkillLevel(skillAstrogeology, true))));   //  5% increase in yield

    //FIXME - For now, aborted cycle returns the ore volume of 1 full cycle. Most likely timeLeft returns 0 here. (but shouldn't)
    if (partial) {
        double timeLeft = GetTimeMSeconds() - (m_cycleStartTime + (_GetDuration() / 1000));
        double fraction = 1 - (timeLeft / (_GetDuration() / 1000));
        _log(MINING__DEBUG, "_ProcessOreCycle(partial=true) - timeLeft:%.3f(%u), fraction:%.3f, startTime:%.3f, duration:%.4f", \
            timeLeft, m_AMPC->GetRemainingCycleTimeMS(), fraction, m_cycleStartTime, _GetDuration());
        if (fraction < 1) {
        	oreVolumeToPull *= fraction;
        } else {
        	while (fraction > 2)
        		fraction -= 1;
        	oreVolumeToPull *= fraction;
        }
        _log(MINING__TRACE, "Cycle aborted. m_cycleStartTime:%.3f, _GetDuration():%.3f, GetTimeMSeconds():%.3f, timeLeft:%.3f, fraction:%.3f, oreVolumeToPull:%.3f", \
        	 m_cycleStartTime, _GetDuration(), GetTimeMSeconds(), timeLeft, fraction, oreVolumeToPull);
    } else {
    	_log(MINING__MESSAGE, "cycle ended. Adding %.2fm3 of ore to cargo", oreVolumeToPull);
    }

    if (oreVolumeToPull< oreVolume) {
        _log(MINING__ERROR, "%s(%u) - Laser could not extract ore from %s(%u)", \
              m_Item->itemName().c_str(), m_Item->itemID(), m_targetEntity->GetSelf()->itemName().c_str(), m_targetEntity->GetID() );
        return;
    }
    if (!oreVolumeToPull) {
        _log(MINING__WARNING, "%s(%u) - Laser could not extract ore from %s(%u)", \
              m_Item->itemName().c_str(), m_Item->itemID(), m_targetEntity->GetSelf()->itemName().c_str(), m_targetEntity->GetID() );
        return;
    }
    if (oreVolumeToPull > roidQuantity)
        oreVolumeToPull = roidQuantity;
    double remainingCargoVolume = m_Ship->GetRemainingVolumeByFlag(flagCargoHold);
    double oreAmount = oreVolumeToPull /oreVolume;
    _log(MINING__TRACE, "Processing the ore: oreVolumeToPull:%.1f, roidQuantity:%.1f, remainingCargoVolume:%.1f, oreAmount:%.1f", \
            oreVolumeToPull, roidQuantity, remainingCargoVolume, oreAmount);

    oreVolumeToPull = oreVolume;
    if (remainingCargoVolume < oreVolumeToPull) {
        oreAmount = remainingCargoVolume /oreVolume;
        oreVolumeToPull = remainingCargoVolume;
        // Not enough cargo space, so module should deactivate and not pull anymore ore from the asteroid
        Deactivate();
    }

    if (oreAmount < 1)
        return;

    ItemData idata(
        asteroidRef->typeID(),
        m_Ship->ownerID(),
        0, //temp location
        flagCargoHold,
        oreAmount
    );

    InventoryItemRef ore = m_Ship->GetItemFactory()->SpawnItem( idata );
    if (!ore) {
        sLog.Error( "MiningLaser::DoCycle()", "ERROR: Could not create ore stack for '%s' ship (id %u)!",\
                    m_Ship->itemName().c_str(), m_Ship->itemID() );
        return;
    }
    /** @todo change these to new format and stack items after addition */
    if (!m_Ship->AddItem(flagCargoHold, ore))
        return;

    roidQuantity -= oreVolumeToPull;
    _log(MINING__TRACE, "new roidQuantity:%.3f", roidQuantity);

    if (!roidQuantity) {
        Deactivate();
        m_targetEntity->SystemMgr()->RemoveEntity(m_targetEntity);
        m_targetEntity->GetSelf()->Delete();
    } else
        asteroidRef->SetAttribute(AttrQuantity, roidQuantity);
}

void MiningLaser::_ProcessCloudCycle(bool partial)
{

}

void MiningLaser::_ProcessIceCycle(bool partial)
{
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
        m_repeat
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
        shipEff.repeat = m_repeat;
        shipEff.error = new PyNone;
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

void MiningLaser::StopCycle(bool abort)
{
    double timeLeft = 2000;
    if (!abort)
        timeLeft = GetTimeMSeconds() - m_cycleStartTime;

    _log(MINING__DEBUG, "StopCycle() - abort:%s, timeLeft:%.3f", (abort?"true":"false"), timeLeft);

    if (abort) {
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

    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect(
        m_Ship,
        m_Item->itemID(),
        m_Item->typeID(),
        m_targetID,
        chargeTypeID,
        effectsString,
        false,
        false,
        false,
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
