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
#include <system/cosmicMgrs/BeltMgr.h>
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
    m_rMiner = m_dcMiner = m_iMiner = m_gMiner = false;

    m_effectID = effectMiningLaser;
    m_effectStr = "effects.Mining";
    if (m_Item->groupID() == EVEDB::invGroups::Mining_Laser) {
        m_rMiner = true;
    } else if ((m_Item->typeID() == 12108) or (m_Item->typeID() == 18068) or (m_Item->typeID() == 24305) or (m_Item->typeID() == 28748)) {
        m_dcMiner = true;
    } else if (m_Item->groupID() == EVEDB::invGroups::Frequency_Mining_Laser) {
        m_rMiner = true;
    } else if ((m_Item->typeID() == 16278) or (m_Item->typeID() == 22229) or (m_Item->typeID() == 22589) or (m_Item->typeID() == 22591)
        or (m_Item->typeID() == 22597) or (m_Item->typeID() == 22599) or (m_Item->typeID() == 28752)) {
        /* this includes 'dev testing modules', also  */
        m_iMiner = true;
    } else if (m_Item->groupID() == EVEDB::invGroups::Gas_Cloud_Harvester) {
        m_gMiner = true;
        m_effectID = effectMiningClouds;
        m_effectStr = "effects.CloudMining";
    }

    /** @hack:  set mining attribs here, based on module item, ship bonuses, and char skills
     * this eliminates extraenous calculations on every activation/cycle.
     * the hack is setting attributes here from ship and skill bonuses, instead of using the
     * (not-yet-implemented) shipEffects and skillEffects classes (which will be based on moduleEffects class) (eta u/k - tbdl)
     * this enables correct information displayed in module "show info" window while in space.
     *   NOTE:  some bonuses are for Alasiya (here), and not found on live, or use different bonus amounts
     */
    // as we hard-set attributes here, we need to make sure they are DEFAULT before adding bonuses.  (error fix)
    m_Item->ResetAttribute(AttrDuration);
    m_Item->ResetAttribute(AttrMaxRange);
    m_Item->ResetAttribute(AttrMiningAmount);

    Character* pChar = m_Ship->GetPilot()->GetChar().get();
    // get module range
    /** @todo  check range */
    m_range = m_Item->GetAttribute(AttrMaxRange).get_float();
    // get module duration
    m_duration = m_Item->GetAttribute(AttrDuration).get_float();
    // get module volume per cycle
    m_cycleVol = m_Item->GetAttribute(AttrMiningAmount).get_int();

    // calculate bonuses
    if (m_iMiner) {
        m_duration *= (1 - ( 0.05 * (pChar->GetSkillLevel(skillIceHarvesting, true))));// 5% decrease in duration
    } else {
        m_duration *= (1 - ( 0.02 * (pChar->GetSkillLevel(skillMining, true))));     // 2% decrease in duration
    }
    m_cycleVol *= (1 + (0.05 * (pChar->GetSkillLevel(skillMining, true))));          // 5% increase in yield
    m_cycleVol *= (1 + (0.05 * (pChar->GetSkillLevel(skillAstrogeology, true))));    // 5% increase in yield
    m_range *= (1 + (0.03 * (pChar->GetSkillLevel(skillAstrometrics, true))));       // 3% increase in range (here)
    m_range *= (1 + (0.03 * (pChar->GetSkillLevel(skillLongRangeTargeting, true)))); // 3% increase in range (here)

    if (m_Ship->type().groupID() == EVEDB::invGroups::MiningBarge) {
        m_duration *= (1 - (0.01 * (pChar->GetSkillLevel(skillMiningBarge, true)))); // 1% decrease in duration (here)
        m_cycleVol *= (1 + (0.02 * (pChar->GetSkillLevel(skillMiningBarge, true)))); // 2% increase in yield (here)
    } else if (m_Ship->type().groupID() == EVEDB::invGroups::Exhumer) {
        m_duration *= (1 - (0.02 * (pChar->GetSkillLevel(skillExhumers, true))));    // 2% decrease in duration
        m_cycleVol *= (1 + (0.02 * (pChar->GetSkillLevel(skillMiningBarge, true)))); // 2% increase in yield (here)
    }

    switch (m_Ship->typeID()) {
        case 591: { /* Tormentor */
            m_cycleVol *= (1 + (0.2 * (pChar->GetSkillLevel(skillAmarrFrigate, true)))); // 20% increase in yield
        } break;
        case 582: { /* Bantam */
            m_cycleVol *= (1 + (0.2 * (pChar->GetSkillLevel(skillCaldariFrigate, true)))); // 20% increase in yield
        } break;
        case 592: { /* Navitas */
            m_cycleVol *= (1 + (0.2 * (pChar->GetSkillLevel(skillGallenteFrigate, true)))); // 20% increase in yield
        } break;
        case 599: { /* Burst */
            m_cycleVol *= (1 + (0.2 * (pChar->GetSkillLevel(skillMinmatarFrigate, true)))); // 20% increase in yield
        } break;
        case 620: { /* Osprey */
            m_cycleVol *= (1 + (0.2 * (pChar->GetSkillLevel(skillCaldariCruiser, true)))); // 20% increase in yield
        } break;
        case 631: { /* Scythe */
            m_cycleVol *= (1 + (0.2 * (pChar->GetSkillLevel(skillMinmatarCruiser, true)))); // 20% increase in yield
        } break;
        case 22546: { /* Skiff */
            if (m_dcMiner)
                m_cycleVol *= (1 + (0.6 * (pChar->GetSkillLevel(skillExhumers)))); // 60% increase in yield
        } break;
        case 22548: { /* Mackinaw */
            if (m_iMiner)
                m_cycleVol *= 2;     // 100% increase in yield
        } break;
        case 17476:   /* Covetor */
        case 22544: { /* Hulk */
            m_range *= (1 + (0.03 * (pChar->GetSkillLevel(skillMiningBarge, true)))); // 3% increase in range  (here)
        } break;
    }

    /** @todo  fleetID() returns 0 until fleets are implemented. (eta u/k - tbdl) */
    // for shits-n-giggles, we allow these bonuses without fleets until they are implemented
    if (true/*pChar->fleetID()*/) {
        m_cycleVol *= (1 + ( 0.02 * (pChar->GetSkillLevel(skillMiningForeman, true))));    //  2% increase in yield
        m_duration *= (1 - ( 0.02 * (pChar->GetSkillLevel(skillMiningDirector, true))));   //  2% decrease in duration
        m_range *= (1 + ( 0.03 * (pChar->GetSkillLevel(skillInformationWarfare, true))));   //  3% increase in range
    }

    if (m_iMiner)
        m_cycleVol = floor(m_cycleVol /1000) *1000;

    // save adjusted attributes
    m_Item->SetAttribute(AttrDuration, m_duration);
    m_Item->SetAttribute(AttrMaxRange, m_range);
    m_Item->SetAttribute(AttrMiningAmount, m_cycleVol);

    _log(MINING__TRACE, "Module Created for %s.  Duration:%.3f, CycleVolume:%.3f, Range:%.3f", item->itemName().c_str(), m_duration, m_cycleVol, m_range);
}

MiningLaser::~MiningLaser()
{
    //reset attribs on this item before deletion
    //  note this does NOT work on crash.
    m_Item->ResetAttribute(AttrDuration);
    m_Item->ResetAttribute(AttrMaxRange);
    m_Item->ResetAttribute(AttrMiningAmount);
}

void MiningLaser::LoadCharge(InventoryItemRef charge)
{
    //if (m_chargeRef) assert(m_chargeRef != charge);

    ActiveModule::LoadCharge(charge);
    m_cycleVol *= m_chargeRef->GetAttribute(AttrSpecialisationAsteroidYieldMultiplier).get_float();
    m_Item->SetAttribute(AttrMiningAmount, m_cycleVol);
    _log(MINING__TRACE, "Charge %s loaded for %s.  CycleVolume updated to %.3f", m_chargeRef->itemName().c_str(), m_Item->itemName().c_str(), m_cycleVol);
}

void MiningLaser::UnloadCharge()
{
    m_cycleVol /= m_chargeRef->GetAttribute(AttrSpecialisationAsteroidYieldMultiplier).get_float();
    m_Item->SetAttribute(AttrMiningAmount, m_cycleVol);
    _log(MINING__TRACE, "Charge %s unloaded for %s.  CycleVolume updated to %.3f", m_chargeRef->itemName().c_str(), m_Item->itemName().c_str(), m_cycleVol);
    ActiveModule::UnloadCharge();
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
    if (!m_cycleVol) {
        _log(MINING__ERROR, "Mining Module %s(%u) has 0 CycleVolume", m_Item->itemName().c_str(), m_Item->itemID());
        if (m_Ship->HasPilot())
            if (m_Ship->GetPilot()->CanThrow())
                throw PyException( MakeCustomError( "Module Activate: Invalid Attribute - Ref: ServerError 15168" ) );
    }

	// verify module vs target for activation.  disallow if not compatible.
    if ((m_rMiner and (pSE->GetSelf()->categoryID() == EVEDB::invCategories::Asteroid) and (pSE->GetSelf()->groupID() != EVEDB::invGroups::Mercoxit))
        or (m_dcMiner and (pSE->GetSelf()->groupID() == EVEDB::invGroups::Mercoxit))
        or (m_iMiner and (pSE->GetSelf()->groupID() == EVEDB::invGroups::Ice))
        or (m_gMiner and (pSE->GetSelf()->groupID() == EVEDB::invGroups::Harvestable_Cloud)))
    {
        m_targetEntity = pSE;
        m_targetID = pSE->GetID();

        m_IsInitialCycle = true;
        // Activate active processing component timer:
        m_AMPC->ActivateCycle();

        //# _ShowCycle();
        _SetCapNeed();
        /** @todo fix THIS bullshit!!! */
        m_Ship->GetPilot()->GetShipSE()->SystemMgr()->GetBeltMgr()->SetActive(m_Ship->GetPilot()->GetShipSE()->SysBubble()->GetID());
    } else {
        _log(MINING__WARNING, "Activate() - Invalid target");
        if (m_Ship->HasPilot())
            if (m_Ship->GetPilot()->CanThrow())
                throw PyException( MakeCustomError( "Module Activate: Invalid Target - Ref: ServerError 15628" ) );
    }
}

void MiningLaser::Deactivate()
{
    if ((m_ModuleState != MOD_ACTIVATED) or (m_ModuleState == MOD_UNFITTED))
        return;
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

    /* ore is dumped into hold at end of module's cycle.
     * however, code processing runs at beginning of cycle, so this needs to 'fake' the ore aquisition to the end of cycle
     * we accomplish this by doing nothing on first cycle, and call the processing component at beginning of each cycle after that.
     */

    if (m_IsInitialCycle) {
    	m_IsInitialCycle = false;
    } else {
        // resources gathered using ship modules are classified the same, so a single process is common for all.
        ProcessCycle();
    }

    return m_duration;
}

/** @todo verify for ice and gas */
void MiningLaser::ProcessCycle(bool partial)
{
    // note:  gas cloud contains radius/10 units of gas.

	// Retrieve ore from target Asteroid and put into Cargo Hold
	InventoryItemRef roidRef = m_targetEntity->GetSelf();
    double oreVolume = roidRef->GetAttribute(AttrVolume).get_float();

    if (m_cycleVol < oreVolume) {
        _log(MINING__ERROR, "%s(%u) - Laser could not extract ore from %s(%u)", \
              m_Item->itemName().c_str(), m_Item->itemID(), m_targetEntity->GetSelf()->itemName().c_str(), m_targetEntity->GetID() );
        return;
    }

    double oreAmount = m_cycleVol /oreVolume;

    if (partial)
        oreAmount *= (m_AMPC->GetRemainingCycleTimeMS() / m_duration);

    double remainingCargoVolume = m_Ship->GetRemainingVolumeByFlag(flagCargoHold);
    double roidQuantity = roidRef->GetAttribute(AttrQuantity).get_float();
    _log(MINING__DEBUG, "ProcessCycle(%s) -  m_cycleVol:%.2f, roidQuantity:%.2f, remainingCargoVolume:%.2f, oreAmount:%.2f", \
            (partial?"true":"false"), m_cycleVol, roidQuantity, remainingCargoVolume, oreAmount);

    if (remainingCargoVolume < m_cycleVol) {
        oreAmount = remainingCargoVolume /oreVolume;
        ActiveModule::Deactivate();
    }

    if (oreAmount > roidQuantity)
        oreAmount = roidQuantity;
    if (oreAmount < 1)
        return;

    _log(MINING__MESSAGE, "Adding %.2fm3 of ore to cargo", oreAmount);

    ItemData idata(
        roidRef->typeID(),
        m_Ship->ownerID(),
        m_Ship->itemID(),
        flagCargoHold,
        oreAmount
    );

    InventoryItemRef ore = m_Ship->GetItemFactory()->SpawnItem( idata );
    if (!ore) {
        sLog.Error( "MiningLaser::DoCycle()", "ERROR: Could not create ore stack for '%s' ship (id %u)!",\
                    m_Ship->itemName().c_str(), m_Ship->itemID() );
        return;
    }

    if (!m_Ship->AddItem(flagCargoHold, ore))
        return;

    roidQuantity -= oreAmount;
    _log(MINING__TRACE, "new roidQuantity:%.3f", roidQuantity);

    if (!roidQuantity) {
        Deactivate();
        m_targetEntity->Delete();
    } else if (!m_iMiner) {
        // do not reset ice radius
        /* reversing the radius-to-quantity formula, we get radius = exp((quantity + 112404.8) /25000)  */
        double radius = exp((roidQuantity +112404.8) /25000);
        roidRef->SetAttribute(AttrRadius, radius);
    }
    roidRef->SetAttribute(AttrQuantity, roidQuantity);
}

void MiningLaser::_ShowCycle()
{
    // Create Special Effect:
    uint32 chargeTypeID = 0;
    if (m_chargeLoaded)
        chargeTypeID = m_chargeRef->typeID();

    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
    (
        m_Ship,
        m_Item->itemID(),
        m_Item->typeID(),
        m_targetID,
        chargeTypeID,
        m_effectStr.c_str(),
        false,
        true,
        true,
        m_duration,
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
        ge.effectID = m_effectID;
    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = 1;
        shipEff.active = 1;
        shipEff.environment = ge.Encode();
        shipEff.startTime = shipEff.timeNow;
        shipEff.duration = m_duration;
        shipEff.repeat = m_repeat;
        shipEff.error = new PyNone;
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

void MiningLaser::StopCycle(bool abort)
{
    double timeTillStop = 2000;
    if (!abort)
        timeTillStop = m_AMPC->GetRemainingCycleTimeMS();

    _log(MINING__DEBUG, "StopCycle() - abort:%s, timeTillStop:%.3fms", (abort?"true":"false"), timeTillStop);

    if (abort) {
            ProcessCycle(abort);
    }

    uint32 chargeTypeID = 0;
    if (m_chargeLoaded)
        if (m_chargeRef)
            chargeTypeID = m_chargeRef->typeID();

    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect(
        m_Ship,
        m_Item->itemID(),
        m_Item->typeID(),
        m_targetID,
        chargeTypeID,
        m_effectStr.c_str(),
        false,
        false,
        false,
        timeTillStop,
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
        ge.effectID = m_effectID;
    Notify_OnGodmaShipEffect shipEff;
        shipEff.itemID = ge.selfID;
        shipEff.effectID = ge.effectID;
        shipEff.timeNow = Win32TimeNow();
        shipEff.start = 0;
        shipEff.active = 0;
        shipEff.environment = ge.Encode();
        shipEff.startTime = shipEff.timeNow;
        shipEff.duration = timeTillStop;
        shipEff.repeat = 0;
        shipEff.error = new PyNone;
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

double MiningLaser::_GetDuration()
{
    return m_duration;
}

void MiningLaser::_SetCapNeed()
{
    // this will be needed for modules and rigs that affect cap need for mining modules
    //double need = _GetCapNeed();

}
