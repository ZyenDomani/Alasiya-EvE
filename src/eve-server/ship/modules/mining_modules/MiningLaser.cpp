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
#include "system/cosmicMgrs/BeltMgr.h"


MiningLaser::MiningLaser( InventoryItemRef item, ShipItemRef ship )
: ActiveModule(item, ship)
{
	m_IsInitialCycle = true;
    m_rMiner = m_dcMiner = m_iMiner = m_gMiner = false;

    m_effectID = effectMiningLaser;
    m_effectStr = "effects.Mining";
    if (m_modRef->groupID() == EVEDB::invGroups::Mining_Laser) {
        m_rMiner = true;
    } else if ((m_modRef->typeID() == 12108) or (m_modRef->typeID() == 18068) or (m_modRef->typeID() == 24305) or (m_modRef->typeID() == 28748)) {
        m_dcMiner = true;
    } else if (m_modRef->groupID() == EVEDB::invGroups::Frequency_Mining_Laser) {
        m_rMiner = true;
    } else if ((m_modRef->typeID() == 16278) or (m_modRef->typeID() == 22229) or (m_modRef->typeID() == 22589) or (m_modRef->typeID() == 22591)
        or (m_modRef->typeID() == 22597) or (m_modRef->typeID() == 22599) or (m_modRef->typeID() == 28752)) {
        /* this includes 'dev testing modules', also  */
        m_iMiner = true;
    } else if (m_modRef->groupID() == EVEDB::invGroups::Gas_Cloud_Harvester) {
        m_gMiner = true;
        m_effectID = effectMiningClouds;
        m_effectStr = "effects.CloudMining";
    }
    _log(MINING__TRACE, "Module Created for %s.  Duration:%.3f, CycleVolume:%.3f, SpecialityVolume:%.3f, Range:%um", \
                        item->itemName().c_str(), m_cycleTime, m_cycleVol, m_cycleVol2, m_maxRange);
}

MiningLaser::~MiningLaser()
{
    //reset attribs on this item before deletion
    ResetAttribute(AttrMiningAmount);
    ResetAttribute(AttrSpecialtyMiningAmount);

}

void MiningLaser::LoadCharge(InventoryItemRef charge)
{
    ActiveModule::LoadCharge(charge);
    m_cycleVol2 *= m_chargeRef->GetAttribute(AttrSpecialisationAsteroidYieldMultiplier).get_float();
    m_crystalDmg = m_chargeRef->GetAttribute(AttrDamage).get_float();
    m_crystalRoidGrp = m_chargeRef->GetAttribute(AttrSpecialisationAsteroidGroup).get_float();
    m_crystalTakeDmg = m_chargeRef->GetAttribute(AttrCrystalsGetDamaged).get_bool();
    m_crystalDmgAmount = m_chargeRef->GetAttribute(AttrCrystalVolatilityDamage).get_float();
    m_crystalDmgChance = m_chargeRef->GetAttribute(AttrCrystalVolatilityChance).get_float();
    SetAttribute(AttrSpecialtyMiningAmount, m_cycleVol2);
    _log(MINING__TRACE, "Charge %s loaded for %s.  SpecialityVolume updated to %.3f", m_chargeRef->itemName().c_str(), m_modRef->itemName().c_str(), m_cycleVol2);
}

void MiningLaser::UnloadCharge()
{
    m_cycleVol2 /= m_chargeRef->GetAttribute(AttrSpecialisationAsteroidYieldMultiplier).get_float();
    m_crystalDmg = 0;
    m_crystalRoidGrp = 0;
    m_crystalTakeDmg = false;
    m_crystalDmgAmount = 0;
    m_crystalDmgChance = 0;
    SetAttribute(AttrSpecialtyMiningAmount, m_cycleVol);
    _log(MINING__TRACE, "Charge %s unloaded for %s.  SpecialityVolume updated to %.3f", m_chargeRef->itemName().c_str(), m_modRef->itemName().c_str(), m_cycleVol2);
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
        _log(MINING__ERROR, "Mining Module %s(%u) has 0 CycleVolume", m_modRef->itemName().c_str(), m_modRef->itemID());
        if (m_shipRef->HasPilot())
            if (m_shipRef->GetPilot()->CanThrow())
                throw PyException( MakeCustomError( "Module Activate: Invalid Attribute - Ref: ServerError 15168" ) );
    }
    if (m_chargeLoaded and !m_cycleVol2) {
        _log(MINING__ERROR, "Mining Module %s(%u) has loaded crystal and 0 SpecialityVolume", m_modRef->itemName().c_str(), m_modRef->itemID());
        if (m_shipRef->HasPilot())
            if (m_shipRef->GetPilot()->CanThrow())
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
        ActiveModule::Activate(pSE);

        //# _ShowCycle();
        _SetCapNeed();
        /** @todo fix THIS bullshit!!! */
        m_targetEntity->SystemMgr()->GetBeltMgr()->SetActive(m_targetEntity->SysBubble()->GetID()); //best i got right now...
        //m_Ship->GetPilot()->GetShipSE()->SystemMgr()->GetBeltMgr()->SetActive(m_Ship->GetPilot()->GetShipSE()->SysBubble()->GetID());
    } else {
        _log(MINING__WARNING, "Activate() - Invalid target");
        if (m_shipRef->HasPilot())
            if (m_shipRef->GetPilot()->CanThrow())
                throw PyException( MakeCustomError( "Module Activate: Invalid Target - Ref: ServerError 15628" ) );
    }
}

void MiningLaser::Deactivate()
{
    if (m_ModuleState != MOD_ACTIVATED)
        return;
    ActiveModule::AbortCycle();
}

double MiningLaser::DoCycle() {
    if ((!m_shipRef->GetPilot()->GetShipSE()->SysBubble())
        or (!m_shipRef->GetPilot()->GetShipSE()->SysBubble()->GetEntity(m_targetID))) {
            StopCycle();
            StopTimer();
            return 0;
        }
    if (m_chargeLoaded)
        if (!m_chargeRef->quantity()) {
            StopCycle();
            StopTimer();
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

    return m_cycleTime;
}

// note:  gas cloud contains radius/10 units of gas.
/** @todo verify for ice and gas */
void MiningLaser::ProcessCycle(bool partial)
{
    // update for t2 crystal shit, if applicable
    float cycleVol = m_cycleVol;
    if (m_chargeLoaded)
        if (m_targetEntity->GetGroupID() == m_crystalRoidGrp)
            cycleVol = m_cycleVol2;
        else
            cycleVol = m_cycleVol;

	InventoryItemRef roidRef = m_targetEntity->GetSelf();
    float oreVolume = roidRef->GetAttribute(AttrVolume).get_float();

    if (cycleVol < oreVolume) {
        _log(MINING__ERROR, "%s(%u) - Laser could not extract ore from %s(%u)", \
              m_modRef->itemName().c_str(), m_modRef->itemID(), m_targetEntity->GetSelf()->itemName().c_str(), m_targetEntity->GetID() );
        return;
    }

    double oreAmount = cycleVol /oreVolume;
    double remainingCargoVolume = m_shipRef->GetRemainingVolumeByFlag(flagCargoHold);
    double roidQuantity = roidRef->GetAttribute(AttrQuantity).get_double();

    if (remainingCargoVolume < cycleVol) {
        if (remainingCargoVolume > 0)
            if (remainingCargoVolume > oreVolume)
                oreAmount = remainingCargoVolume /oreVolume;
            else
                oreAmount = 0;
        StopTimer();
        if (!partial) {
            StopCycle();
            return;
        }
    } else if (partial) {
        if (m_iMiner) {
            oreAmount *= (GetRemainingCycleTimeMS() / m_cycleTime);
            oreAmount = floor(oreAmount);
        } else {
            oreAmount *= (GetRemainingCycleTimeMS() / m_cycleTime);
        }
    }

    _log(MINING__DEBUG, "ProcessCycle(%s) -  cycleVol:%.2f, roidQuantity:%.2f, remainingCargoVolume:%.2f, oreAmount:%.2f", \
            (partial?"true":"false"), cycleVol, roidQuantity, remainingCargoVolume, oreAmount);

    if (oreAmount > roidQuantity)
        oreAmount = roidQuantity;
    if (oreAmount < 1)
        return;

    ItemData idata(roidRef->typeID(), m_shipRef->ownerID(), 0, flagAutoFit, oreAmount);
    InventoryItemRef ore = m_shipRef->GetItemFactory()->SpawnItem( idata );
    if (!ore) {
        _log(MINING__ERROR, "Could not create mined ore for %s(%u)", m_shipRef->itemName().c_str(), m_shipRef->itemID() );
        return;
    }

    if (!m_shipRef->AddItem(flagCargoHold, ore)) {
        _log(MINING__ERROR, "Could not add mined ore in cargo for %s(%u)", m_shipRef->itemName().c_str(), m_shipRef->itemID() );
        return;
    }

    roidQuantity -= oreAmount;
    _log(MINING__TRACE, "new roidQuantity %.3f", roidQuantity);

    if (!roidQuantity) {
        StopCycle();
        StopTimer();
        m_targetEntity->Delete();
    } else if (!m_iMiner) {
        // do not reset ice radius
        /* reversing the radius-to-quantity formula, we get radius = exp((quantity + 112404.8) /25000)  */
        double radius = exp((roidQuantity +112404.8) /25000);
        roidRef->SetAttribute(AttrRadius, radius);
        roidRef->SetAttribute(AttrQuantity, roidQuantity);
    }
    if (m_chargeLoaded)
        if (m_crystalTakeDmg) {
            if (MakeRandomFloat(0,1) < m_crystalDmgChance) {
                m_crystalDmg += m_crystalDmgAmount;
                if (m_crystalDmg > 1.0f) {
                    m_shipRef->GetPilot()->SendNotifyMsg("Your %s loaded in %s has been destroyed.", m_chargeRef->itemName().c_str(), m_modRef->itemName().c_str());
                    InventoryItemRef chargeRef = m_chargeRef;   // make a copy of item ref, as m_chargeRef is nulled after next call returns
                    m_shipRef->RemoveItem(m_chargeRef);
                    chargeRef->Delete();
                    cycleVol = m_cycleVol;  //m_cycleVol2 is reset when charge is removed.
                } else {
                    m_chargeRef->SetAttribute(AttrDamage, m_crystalDmg);
                }
            }
        }
}

void MiningLaser::_ShowCycle()
{
    // Create Special Effect:
    uint32 chargeTypeID = 0;
    if (m_chargeLoaded)
        chargeTypeID = m_chargeRef->typeID();

    m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect
    (
        m_shipRef,
        m_modRef->itemID(),
        m_modRef->typeID(),
        m_targetID,
        chargeTypeID,
        m_effectStr.c_str(),
        false,
        true,
        true,
        m_cycleTime,
        m_repeat
    );

    // Create Destiny Updates:
    GodmaOther go;
        go.shipID = m_shipRef->itemID();
        go.slotID = m_modRef->flag();
        go.chargeTypeID = chargeTypeID;
    GodmaEnvironment ge;
        ge.selfID = m_modRef->itemID();
        ge.charID = m_shipRef->ownerID();
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
        shipEff.duration = m_cycleTime;
        shipEff.repeat = m_repeat;
        shipEff.error = new PyNone();
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

void MiningLaser::StopCycle(bool abort)
{
    double timeTillStop = 2000;
    if (!abort)
        timeTillStop = GetRemainingCycleTimeMS();

    _log(MINING__DEBUG, "StopCycle() - abort:%s, timeTillStop:%.3fms", (abort?"true":"false"), timeTillStop);

    if (abort) {
        ProcessCycle(abort);
    }

    uint32 chargeTypeID = 0;
    if (m_chargeLoaded)
        if (m_chargeRef)
            chargeTypeID = m_chargeRef->typeID();

    m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendSpecialEffect(
        m_shipRef,
        m_modRef->itemID(),
        m_modRef->typeID(),
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
        go.shipID = m_shipRef->itemID();
        go.slotID = m_modRef->flag();
        go.chargeTypeID = chargeTypeID;
    GodmaEnvironment ge;
        ge.selfID = m_modRef->itemID();
        ge.charID = m_shipRef->ownerID();
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
        shipEff.error = new PyNone();
    std::vector<PyTuple*> events;
        events.push_back(shipEff.Encode());
    std::vector<PyTuple*> updates;
    m_shipRef->GetPilot()->GetShipSE()->DestinyMgr()->SendDestinyUpdate(updates, events, false);
}

void MiningLaser::_SetCapNeed()
{
    // this will be needed for modules and rigs that affect cap need for mining modules
    //double need = _GetCapNeed();

}
