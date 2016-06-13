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
    Author:        Luck
    Updates:    Allan
*/


#include "eve-server.h"

#include "character/Character.h"
#include "ship/Ship.h"
#include "ship/modules/ActiveModule.h"
#include "ship/modules/components/ActiveModuleProcessingComponent.h"


ActiveModuleProcessingComponent::ActiveModuleProcessingComponent(InventoryItemRef item, ActiveModule* mod, ShipItemRef ship)
:m_Item( item ),
 m_Mod( mod ),
 m_Ship( ship ),
 m_timer(0)
{
    m_Stop = false;
    m_timer.Disable();
}

// timing and verification function
void ActiveModuleProcessingComponent::Process() {
	//check if we have signal to stop the cycle
	if (m_Stop) {
        if (m_timer.Check(false)) {
            //wait for time to run out and send deactivate to client
            ProcessDeactivateCycle();
            return;
        }
    }
    //check if the timer expired & subtract time
    if (m_timer.Check())
        ShouldProcessActiveCycle();
}

void ActiveModuleProcessingComponent::ActivateCycle()
{
	m_Stop = false;
    m_Mod->SetModuleState(MOD_ACTIVATED);  //this HAS to be called before mod::DoCycle()

    /** @todo   these need to check for targetable actions, and apply changes accordingly */
    /** @todo  this needs to be updated to check for/use targetGroupIDs */
    EVECalculationType ecType = CALC_NONE;
    uint32 targetAttrID = 0, sourceAttrID = 0, testID = 0, groupID = m_Item->groupID();
    std::map<uint32, std::shared_ptr<MEffect>>::const_iterator itr, end;
    if (m_Mod->IsOverloaded()) {
        itr = m_Mod->m_Effects->GetOverloadEffectsBegin();
        end = m_Mod->m_Effects->GetOverloadEffectsEnd();
        _log(SHIP__MODULE_TRACE, "AMPC::ActivateCycle() -  there are %u OverLoaded effects to process", m_Mod->m_Effects->GetOverloadEffectsSize() );
    } else {
        itr = m_Mod->m_Effects->GetActiveEffectsBegin();
        end = m_Mod->m_Effects->GetActiveEffectsEnd();
        _log(SHIP__MODULE_TRACE, "AMPC::ActivateCycle() -  there are %u Active effects to process", m_Mod->m_Effects->GetActiveEffectsSize() );
    }
    for (; itr != end; itr++) {
        uint32 cur = 0, ids = itr->second->GetSizeOfAttributeList();
        _log(SHIP__MODULE_INFO, "AMPC::ActivateCycle() -  there are %u attributes in effect %u", ids, itr->first );
        while (cur < ids) {
            testID = itr->second->GetTargetGroup(cur);
            if (groupID != testID) { ++cur; continue; }
            targetAttrID = itr->second->GetTargetAttributeID(cur);
            sourceAttrID = itr->second->GetSourceAttributeID(cur);
            ecType = itr->second->GetCalculationType(cur);
            _log(SHIP__MODULE_TRACE, "AMPC::ActivateCycle() - effect %u[%u] - modify attr target:%u, source:%u, ecType:%i", \
                        itr->first, cur, targetAttrID, sourceAttrID, (int8)ecType);
            if (itr->first == Effect_damageControl)
                m_Mod->m_MSAC->ModifyNonStackingShipAttributes(targetAttrID, sourceAttrID, ecType);
            else if (m_Mod->RequiresTarget() && m_Mod->GetTarget())
                m_Mod->m_MSAC->ModifyTargetShipAttribute(m_Mod->GetTargetID(), targetAttrID, sourceAttrID, ecType);
            else
                m_Mod->m_MSAC->ModifyShipAttribute(targetAttrID, sourceAttrID, ecType);
            ++cur;
        }
    }

    //each module has a _GetROF() or _GetDuration() method that returns a cycle time,
    // based on character skills and specific module attributes.
    //  specific module classes may override the default in ActiveModule()   -allan 19Dec15
    SetTimer((uint32)m_Mod->DoCycle()); // Do initial cycle immediately while we start timer
}

void ActiveModuleProcessingComponent::DeactivateCycle()
{
    m_Mod->SetModuleState(MOD_DEACTIVATING);
    EVECalculationType ecType = CALC_NONE;
    uint32 targetAttrID = 0, sourceAttrID = 0, testID = 0, groupID = m_Item->groupID();
    std::map<uint32, std::shared_ptr<MEffect>>::const_iterator itr, end;
    if (m_Mod->IsOverloaded()) {
        itr = m_Mod->m_Effects->GetOverloadEffectsBegin();
        end = m_Mod->m_Effects->GetOverloadEffectsEnd();
        _log(SHIP__MODULE_TRACE, "AMPC::DeactivateCycle() -  there are %u OverLoaded effects to process", m_Mod->m_Effects->GetOverloadEffectsSize() );
    } else {
        itr = m_Mod->m_Effects->GetActiveEffectsBegin();
        end = m_Mod->m_Effects->GetActiveEffectsEnd();
        _log(SHIP__MODULE_TRACE, "AMPC::DeactivateCycle() -  there are %u Active effects to process", m_Mod->m_Effects->GetActiveEffectsSize() );
    }
    for (; itr != end; itr++) {
        uint32 cur = 0, ids = itr->second->GetSizeOfAttributeList();
        _log(SHIP__MODULE_INFO, "AMPC::DeactivateCycle() -  there are %u attributes in effect %u", ids, itr->first );
        while (cur < ids) {
            testID = itr->second->GetTargetGroup(cur);
            if (groupID != testID) { ++cur; continue; }
            targetAttrID = itr->second->GetTargetAttributeID(cur);
            sourceAttrID = itr->second->GetSourceAttributeID(cur);
            ecType = itr->second->GetReverseCalculationType(cur);
            _log(SHIP__MODULE_TRACE, "AMPC::DeactivateCycle() - effect %u[%u] - modify attr target:%u, source:%u, ecType:%i", \
                        itr->first, cur, targetAttrID, sourceAttrID, (int8)ecType);
            if (itr->first == Effect_damageControl)
                m_Mod->m_MSAC->ModifyNonStackingShipAttributes(targetAttrID, sourceAttrID, ecType);
            else if (m_Mod->RequiresTarget() && m_Mod->GetTarget())
                m_Mod->m_MSAC->ModifyTargetShipAttribute(m_Mod->GetTargetID(), targetAttrID, sourceAttrID, ecType);
            else
                m_Mod->m_MSAC->ModifyShipAttribute(targetAttrID, sourceAttrID, ecType);
            ++cur;
        }
    }
    m_Mod->StopCycle();
}

void ActiveModuleProcessingComponent::StopCycle()
{
    m_Stop = true;
}

void ActiveModuleProcessingComponent::AbortCycle()
{
	// Immediately stop active cycle for things such as target destroyed or left bubble, or miner deactivated by player:
    m_Stop = true;
    m_Mod->StopCycle(true);
    m_Mod->SetModuleState(MOD_ONLINE);
	m_timer.Disable();
}

void ActiveModuleProcessingComponent::ShouldProcessActiveCycle() {
    //first, check if we have been told to deactivate
	if (m_Stop)
        return;
	//check that we have enough capacitor avaiable
    if (m_Mod->ShipHasCapCharge())
        ProcessActiveCycle();
	else
        m_Stop = true;
}

void ActiveModuleProcessingComponent::ProcessActiveCycle()
{
    //check for stop signal
    if (m_Stop)
        return;

    // check if we are targeting another ship or not and apply attribute changes
	//maybe we can have a check for modules that repeat the same attributes so we
	//send the changes just once at activation and at deactivation      --in progress  -allan 19Dec15

    // consume capacitor...this will be taken over by module effects when i get to that point.
    EvilNumber capCapacity = m_Ship->GetAttribute(AttrCapacitorCharge);
    capCapacity -= m_Mod->GetAttribute(AttrCapacitorNeed);  // this is reset by modules that need it to be.
    m_Ship->SetAttribute(AttrCapacitorCharge, capCapacity);

    // reset timer here, in the case of cycle time changing (mostly for fleet bonuses)
    SetTimer((uint32)m_Mod->DoCycle());
}

void ActiveModuleProcessingComponent::ProcessDeactivateCycle()
{
    //  catch-all incase these werent set correctly in module code.
    m_Stop = true;
    m_Mod->SetModuleState(MOD_DEACTIVATING);
    m_timer.Disable();

    DeactivateCycle();
}

uint32 ActiveModuleProcessingComponent::GetRemainingCycleTimeMS() {
	return m_timer.GetRemainingTime();
}

void ActiveModuleProcessingComponent::SetTimer(uint32 time) {
    _log(SHIP__MODULE_TRACE, "AMPC::SetTimer() - Started with %u ms.", time);
    m_timer.Start(time);
}
