
 /**
  * @name MiningModule.cpp
  *   mining module class
  * @Author:         Allan
  * @date:   10 June 2015   -UD/RW 02 April 2017
  */


#include "eve-server.h"

#include "PyServiceMgr.h"
#include "character/Character.h"
#include "ship/Ship.h"
#include "ship/modules/MiningLaser.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"
#include "system/cosmicMgrs/BeltMgr.h"


MiningLaser::MiningLaser( InventoryItemRef item, ShipItemRef ship )
: ActiveModule(item, ship)
{
    m_IsInitialCycle = true;
    m_rMiner = m_dcMiner = m_iMiner = m_gMiner = false;

    m_crystalDmg = 0;
    m_crystalRoidGrp = 0;
    m_crystalDmgAmount = 0;
    m_crystalDmgChance = 0;

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
    }
    _log(MINING__TRACE, "MiningLaser Created for %s with %ums Duration.", item->itemName().c_str(), GetAttribute(AttrDuration).get_int());
}

void MiningLaser::LoadCharge(InventoryItemRef charge)
{
    ActiveModule::LoadCharge(charge);

    m_crystalDmg = m_chargeRef->GetAttribute(AttrDamage).get_float();
    m_crystalRoidGrp = m_chargeRef->GetAttribute(AttrSpecialisationAsteroidGroup).get_float();
    m_crystalDmgAmount = m_chargeRef->GetAttribute(AttrCrystalVolatilityDamage).get_float();
    m_crystalDmgChance = m_chargeRef->GetAttribute(AttrCrystalVolatilityChance).get_float();
}

void MiningLaser::UnloadCharge()
{
    m_crystalDmg = 0;
    m_crystalRoidGrp = 0;
    m_crystalDmgAmount = 0;
    m_crystalDmgChance = 0;

    ActiveModule::UnloadCharge();
}

bool MiningLaser::CanActivate()
{
    // verify module vs target for activation.  disallow if not compatible.
    if ((m_rMiner and (m_targetSE->GetSelf()->categoryID() == EVEDB::invCategories::Asteroid) and (m_targetSE->GetSelf()->groupID() != EVEDB::invGroups::Mercoxit))
        or (m_dcMiner and (m_targetSE->GetSelf()->groupID() == EVEDB::invGroups::Mercoxit))
        or (m_iMiner and (m_targetSE->GetSelf()->groupID() == EVEDB::invGroups::Ice))
        or (m_gMiner and (m_targetSE->GetSelf()->groupID() == EVEDB::invGroups::Harvestable_Cloud)))
    {
        m_IsInitialCycle = true;

        m_targetSE->SystemMgr()->GetBeltMgr()->SetActive(m_targetSE->SysBubble()->GetID());
        return true;
    } else {
        _log(MINING__WARNING, "Activate() - Invalid target");
        if (m_shipRef->HasPilot())
            if (m_shipRef->GetPilot()->CanThrow())
                throw PyException( MakeCustomError( "Module Activate: Invalid Target - Ref: ServerError 15628" ) );
    }
    return false;
}

uint32 MiningLaser::DoCycle() {
    /* ore is dumped into hold at end of module's cycle.
     * however, code processing runs at beginning of cycle, so this needs to 'fake' the ore aquisition to the end of cycle
     * we accomplish this by doing nothing on first cycle, and call the processing component at beginning of each cycle after that.
     */
    if (m_IsInitialCycle)
    	m_IsInitialCycle = false;
    else
        ProcessCycle();

    return ActiveModule::DoCycle();
}

void MiningLaser::DeactivateCycle(bool abort)
{
    using namespace ModStates;

    if (m_ModuleState != ModuleStates::MOD_DEACTIVATING)
        return;

    ApplyEffect(Effects::dgmStateActive, false);
    ShowEffect(false, abort);

    ProcessCycle(abort);

    SetModuleState(ModuleStates::MOD_ONLINE);
    Clear();
}

// note:  gas cloud contains radius/10 units of gas.
/** @todo verify for ice and gas */
void MiningLaser::ProcessCycle(bool partial)
{
    // update for t2 crystal shit, if applicable
    float cycleVol = GetAttribute(AttrMiningAmount).get_float();
    if (m_chargeLoaded)
        if (m_targetSE->GetGroupID() == m_crystalRoidGrp)
            cycleVol = GetAttribute(AttrSpecialtyMiningAmount).get_float();

	InventoryItemRef roidRef = m_targetSE->GetSelf();
    float oreVolume = roidRef->GetAttribute(AttrVolume).get_float();

    if (cycleVol < oreVolume) {
        _log(MINING__ERROR, "%s(%u) - Mining Laser could not extract ore from %s(%u)", \
              m_modRef->itemName().c_str(), m_modRef->itemID(), m_targetSE->GetSelf()->itemName().c_str(), m_targetSE->GetID() );
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
            ActiveModule::AbortCycle();
            return;
        }
    } else if (partial) {
        oreAmount *= (GetRemainingCycleTimeMS() / GetAttribute(AttrDuration).get_float());
        if (m_iMiner)
            oreAmount = floor(oreAmount);
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
        ActiveModule::AbortCycle();
        m_targetSE->Delete();
    } else if (!m_iMiner) {
        // do not reset ice radius
        /* reversing the radius-to-quantity formula, we get radius = exp((quantity + 112404.8) /25000)  */
        double radius = exp((roidQuantity +112404.8) /25000);
        roidRef->SetAttribute(AttrRadius, radius);
        roidRef->SetAttribute(AttrQuantity, roidQuantity);
    }
    if (m_chargeLoaded)
        if (m_chargeRef->HasAttribute(AttrCrystalsGetDamaged))
            if (MakeRandomFloat(0,1) < m_crystalDmgChance) {
                m_crystalDmg += m_crystalDmgAmount;
                if (m_crystalDmg > 1.0f) {
                    m_shipRef->GetPilot()->SendNotifyMsg("Your %s loaded in %s has been destroyed.", m_chargeRef->itemName().c_str(), m_modRef->itemName().c_str());
                    InventoryItemRef chargeRef = m_chargeRef;   // make a copy of item ref, as m_chargeRef = NULL after next call returns
                    m_shipRef->RemoveItem(m_chargeRef);
                    chargeRef->Delete();
                } else {
                    m_chargeRef->SetAttribute(AttrDamage, m_crystalDmg);
                }
            }
}
