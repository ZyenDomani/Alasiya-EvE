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
    Author:        Aknor Jaden, Luck
    Updates:    Allan
*/

/* updates to implement basic memory management (remove naked 'new')  -allan 30Mar16 */

#include "eve-server.h"

#include "PyCallable.h"
#include "EVEServerConfig.h"
#include "Client.h"
#include "StaticDataMgr.h"
#include "effects/EffectsDataMgr.h"
#include "ship/Ship.h"
#include "ship/modules/ModuleManager.h"
#include "ship/modules/ModuleFactory.h"
#include "ship/modules/ActiveModule.h"
#include "system/DestinyManager.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"


//////////////////////////////////////////////////////////////////////////////////
// ModuleContainer class definitions
ModuleContainer::ModuleContainer(uint8 lowSlots, uint8 medSlots, uint8 highSlots, uint8 rigSlots, uint8 subSystemSlots,
    uint8 turretSlots, uint8 launcherSlots, ModuleManager* myManager)
{
    m_MyManager = myManager;

    m_LowSlots = lowSlots;
    m_MediumSlots = medSlots;
    m_HighSlots = highSlots;
    m_RigSlots = rigSlots;
    m_SubSystemSlots = subSystemSlots;
    m_TurretSlots = turretSlots;
    m_LauncherSlots = launcherSlots;

    ClearModMap();
}

ModuleContainer::~ModuleContainer()
{
    std::map<uint8, GenericModule*>::iterator itr = m_modules.begin(), end = m_modules.end();
    for (; itr != end; ++itr)
        SafeDelete(itr->second);
}

void ModuleContainer::ClearModMap() {
    // this will populate the module map for all slots with null pointer
    // modules
    for (uint8 flag = flagLowSlot0; flag < flagFixedSlot; ++flag)
        m_modules.insert(std::pair<uint8, GenericModule*>(flag, nullptr));
    // rigs
    for (uint8 flag = flagRigSlot0; flag < flagRigSlot3; ++flag)
        m_modules.insert(std::pair<uint8, GenericModule*>(flag, nullptr));
    //subsystems
    for (uint8 flag = flagSubSystem0; flag < flagSubSystem5; ++flag)
        m_modules.insert(std::pair<uint8, GenericModule*>(flag, nullptr));
}

bool ModuleContainer::AddModule(EVEItemFlags flag, GenericModule* pMod)
{
    std::map<uint8, GenericModule*>::iterator itr = m_modules.find((uint8)flag);
    if (itr == m_modules.end())
        return false;
    else
        itr->second = pMod;
    _log(SHIP__MODULE_TRACE, "AddModule() - adding %s.", pMod->GetSelf()->itemName().c_str());

    // Maintain the Modules Fitted By Group counter for this module group:
    if ( m_ModulesFittedByGroupID.find(pMod->GetSelf()->groupID()) != m_ModulesFittedByGroupID.end() )
        m_ModulesFittedByGroupID.find(pMod->GetSelf()->groupID())->second++;
    else
        m_ModulesFittedByGroupID.insert(std::pair<uint32,uint32>(pMod->GetSelf()->groupID(), 1));

    // module is fit so change state from Unfitted to Offline
    pMod->SetModuleState(Module::State::Offline);
	return true;
}

bool ModuleContainer::RemoveModule(EVEItemFlags flag) {
    GenericModule* pMod = GetModule(flag);
    if (pMod == nullptr)
        return false;

    pMod->ProcessEffects(Effects::dgmStatePassive, false);

    deleteModuleRef(pMod->flag(), pMod);
	return true;
}

bool ModuleContainer::RemoveModule(uint32 itemID) {
    GenericModule* pMod = GetModule(itemID);
    if (pMod == nullptr)
        return false;

    pMod->ProcessEffects(Effects::dgmStatePassive, false);

    deleteModuleRef(pMod->flag(), pMod);
    return true;
}

GenericModule* ModuleContainer::GetModule(EVEItemFlags flag)
{
    std::map<uint8, GenericModule*>::iterator itr = m_modules.find((uint8)flag);
    if (itr != m_modules.end())
        return itr->second;

    return nullptr;
}

GenericModule* ModuleContainer::GetModule(uint32 itemID)
{
    //iterate through the list and see if we have it
    std::map<uint8, GenericModule*>::iterator itr = m_modules.begin();
    while (itr != m_modules.end()) {
        if (itr->second != nullptr)
            if (itr->second->itemID() == itemID)
                return itr->second;

        ++itr;
    }
    return nullptr;  //we don't
}

void ModuleContainer::ShipWarping()
{
    // this will test for 1) active modules and 2) if their effect is warpsafe
    // those not safe for warp will be Deactivated
    std::map<uint8, GenericModule*>::iterator itr = m_modules.begin();
    while (itr != m_modules.end()) {
        if (itr->second != nullptr)
            if (!itr->second->isWarpSafe())
                itr->second->AbortCycle();
        ++itr;
    }
}

void ModuleContainer::AbortCycle() {
    process(typeAbort);
}

void ModuleContainer::Process() {
    process(typeProcessAll);
}

void ModuleContainer::OnlineAll() {
    process(typeOnlineAll);
}

void ModuleContainer::OfflineAll() {
    process(typeOfflineAll);
}

void ModuleContainer::DeactivateAll() {
    process(typeDeactivateAll);
}

void ModuleContainer::UnloadAll() {
    process(typeUnloadAll);
}

void ModuleContainer::RepairAll()
{
    std::map<uint8, GenericModule*>::iterator itr = m_modules.begin();
    while (itr != m_modules.end()) {
        if (itr->second != nullptr)
            itr->second->Repair();
        ++itr;
    }
}

void ModuleContainer::process(processType p)
{
    switch(p) {
        case typeOnlineAll: {
            std::map<uint8, GenericModule*>::reverse_iterator itr = m_modules.rbegin();
            while (itr != m_modules.rend()) {
                if (itr->second != nullptr)
                    itr->second->Online();
                ++itr;
            }
        } break;
        case typeOfflineAll: {
            std::map<uint8, GenericModule*>::iterator itr = m_modules.begin();
            while (itr != m_modules.end()) {
                if (itr->second != nullptr)
                    itr->second->Offline();
                ++itr;
            }
        } break;
        case typeDeactivateAll: {
            std::map<uint8, GenericModule*>::iterator itr = m_modules.begin();
            while (itr != m_modules.end()) {
                if (itr->second != nullptr)
                    itr->second->Deactivate();
                ++itr;
            }
        } break;
        case typeUnloadAll: {
            std::map<uint8, GenericModule*>::iterator itr = m_modules.begin();
            while (itr != m_modules.end()) {
                if (itr->second != nullptr)
                    itr->second->UnloadCharge();
                ++itr;
            }
        } break;
        case typeProcessAll: {
            std::map<uint8, GenericModule*>::reverse_iterator itr = m_modules.rbegin();
            while (itr != m_modules.rend()) {
                if (itr->second != nullptr)
                    itr->second->Process();
                ++itr;
            }
        } break;
        case typeAbort: {
            std::map<uint8, GenericModule*>::iterator itr = m_modules.begin();
            while (itr != m_modules.end()) {
                if ((itr->second != nullptr) and (itr->second->IsActiveModule()))
                    itr->second->AbortCycle();
                ++itr;
            }
        } break;
    }
}

bool ModuleContainer::isSlotOccupied(EVEItemFlags flag) {
    std::map<uint8, GenericModule*>::iterator itr = m_modules.find((uint8)flag);
    return (itr == m_modules.end() ? false : true);
}

uint16 ModuleContainer::GetAvailableSlotInBank(EVEEffectID slotBank)
{
    switch (slotBank) {
        case EVEEffectID::loPower: {
            for (uint8 slot=flagLowSlot0; slot < (flagLowSlot0 + m_LowSlots); ++slot)
				if ( m_modules[slot] == nullptr )
					return slot;
            } break;
		case EVEEffectID::medPower: {
            for (uint8 slot=flagMedSlot0; slot < (flagMedSlot0 + m_MediumSlots); ++slot)
                if ( m_modules[slot] == nullptr )
					return slot;
            } break;
		case EVEEffectID::hiPower: {
            for (uint8 slot=flagHiSlot0; slot < (flagHiSlot0 + m_HighSlots); ++slot)
                if ( m_modules[slot] == nullptr )
					return slot;
            } break;
		case EVEEffectID::rigSlot: {
            for (uint8 slot=flagRigSlot0; slot < (flagRigSlot0 + m_RigSlots); ++slot)
                if ( m_modules[slot] == nullptr )
					return slot;
            } break;
		case EVEEffectID::subSystem: {
            for (uint8 slot=flagSubSystem0; slot < (flagSubSystem0 + m_SubSystemSlots); ++slot)
                if ( m_modules[slot] == nullptr )
					return slot;
            } break;
		default: {
			// ERROR: This is not a module that fits in any of the slot banks
            return flagIllegal;
            } break;
	}
}

uint8 ModuleContainer::GetFittedModuleCountByGroup(uint16 groupID)
{
    if ( m_ModulesFittedByGroupID.find(groupID) == m_ModulesFittedByGroupID.end() )
        return 0;
    else
        return m_ModulesFittedByGroupID.find(groupID)->second;
}

GenericModule* ModuleContainer::GetRandModule()
{
    std::vector<GenericModule*> modVec;
    std::map<uint8, GenericModule*>::iterator itr;
    for (uint8 flag = flagLowSlot0; flag < flagFixedSlot; ++flag) {
        itr = m_modules.find(flag);
        if (itr != m_modules.end()) {
            if (itr->second != nullptr)
                modVec.push_back(itr->second);
        }
    }

    return modVec.at(MakeRandomInt(0, modVec.size()));  // initial implementation will use .at() as a safety
}

void ModuleContainer::GetModuleListOfRefsAsc(std::vector<InventoryItemRef> * pModuleList)
{
    std::map<uint8, GenericModule*>::iterator itr = m_modules.begin();
    while (itr != m_modules.end()) {
        if (itr->second)
            pModuleList->push_back( itr->second->GetSelf() );
        ++itr;
    }
}

void ModuleContainer::GetModuleListOfRefsDec(std::vector< InventoryItemRef >* pModuleList)
{
    std::map<uint8, GenericModule*>::reverse_iterator itr = m_modules.rbegin();
    while (itr != m_modules.rend()) {
        if (itr->second)
            pModuleList->push_back( itr->second->GetSelf() );
        ++itr;
    }
}


void ModuleContainer::SaveModules()
{
    std::map<uint8, GenericModule*>::iterator itr = m_modules.begin();
    while (itr != m_modules.end()) {
        if (itr->second != nullptr)
            itr->second->GetSelf()->SaveItem();
        ++itr;
    }
}

void ModuleContainer::deleteModuleRef(EVEItemFlags flag, GenericModule* pMod)
{
    std::map<uint8, GenericModule*>::iterator itr = m_modules.find((uint8)flag);
    if (itr != m_modules.end())
        itr->second = nullptr;
        // make error for module not found in map?  should never happen.

    // Maintain the Modules Fitted By Group counter for this module group:
    if (m_ModulesFittedByGroupID.find(pMod->GetSelf()->groupID()) != m_ModulesFittedByGroupID.end()) {
        if (m_ModulesFittedByGroupID.find(pMod->GetSelf()->groupID())->second > 1) {
            // We still have more than one module of this group fitted, so just reduce number fitted by 1:
            --(m_ModulesFittedByGroupID.find(pMod->GetSelf()->groupID())->second);
        } else {
            // This was the last module of this group fitted, so remove the entry from the map:
            m_ModulesFittedByGroupID.erase(pMod->GetSelf()->groupID());
        }
    } else
        sLog.Error( "ModuleContainer::deleteModuleRef()", "Removing Module from ship fit when it had NO entry in m_ModulesFittedByGroup !" );

    pMod->SetModuleState(Module::State::Unfitted);
}


/////////////////////////// END MODULECONTAINER //////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
// ModuleManager class definitions
ModuleManager::ModuleManager(ShipItem *const ship)
{
    m_initalized = false;
    m_Ship = ship;

    // Create ModuleContainer object and initialize with sizes for avalible slot banks for this ship:
    m_Modules = new ModuleContainer((uint8)ship->GetAttribute(AttrLowSlots).get_int(),
                                    (uint8)ship->GetAttribute(AttrMedSlots).get_int(),
                                    (uint8)ship->GetAttribute(AttrHiSlots).get_int(),
                                    (uint8)ship->GetAttribute(AttrRigSlots).get_int(),
                                    (uint8)ship->GetAttribute(AttrSubSystemSlot).get_int(),
                                    (uint8)ship->GetAttribute(AttrTurretSlotsLeft).get_int(),
                                    (uint8)ship->GetAttribute(AttrLauncherSlotsLeft).get_int(),
                                    this);
}

ModuleManager::~ModuleManager()
{
    //module cleanup is handled in the ModuleContainer destructor
    SafeDelete(m_Modules);
}

bool ModuleManager::Initialize() {
    if (m_initalized)
        return true;

    // Load modules, charges, rigs and subsystems into ship's ModuleContainer:
    std::vector<InventoryItemRef> itemVec;
    m_Ship->GetMyInventory()->GetInventoryVec(itemVec);   // this method also sorts in order - cargo, modules, charge, subsystems.

    GenericModule* pMod(nullptr);
    // first we have to fit modules
    for (auto cur : itemVec) {
        // this is a hack.  dont know why any ship item would have flagAutoFit set, but have seen random errors where charges are set to flagAutoFit
        if (cur->flag() == flagAutoFit)
            cur->SetFlag(flagCargoHold);
        if (cur->flag() != flagCargoHold)
            switch (cur->categoryID()) {
                case EVEDB::invCategories::Module:
                case EVEDB::invCategories::Subsystem: {
                    if (!fitModule(cur, cur->flag()))
                        _log(SHIP__MODULE_ERROR, "ModuleManager::Initialize() - Could not insert module %s(%u) at flag %u into module container.",\
                                cur->itemName().c_str(), cur->itemID(), cur->flag() );
                } break;
                case EVEDB::invCategories::Charge: {
                    pMod = GetModule(cur->flag());
                    if (pMod != nullptr) {
                        pMod->SetChargeRef(cur);
                        // set ChargeState == CHG_LOADED here, then when module Online() is called, all effects will be applied in correct order
                        pMod->SetChargeState(Module::State::Loaded);
                        m_charges.emplace(cur->flag(), cur);
                    } else {
                        _log(SHIP__MODULE_ERROR, "ModuleManager::Initialize() - Cannot find module to load charge %s(%u) into at flag %u",\
                                cur->itemName().c_str(), cur->itemID(), cur->flag() );
                    }
                    pMod = nullptr;
                } break;
            }
    }
    return (m_initalized = true);
}

void ModuleManager::Process()
{
    double profileStartTime = 0.0;
    if (sConfig.debug.UseProfiling)
        profileStartTime = GetTimeUSeconds();

    m_Modules->Process();

    if (sConfig.debug.UseProfiling)
        sProfile.AddTime(_modulesProfile, GetTimeUSeconds() - profileStartTime);
}

bool ModuleManager::IsSlotOccupied(EVEItemFlags flag)
{
    if (m_Modules->GetModule(flag))
        return true;
    return false;
}

uint16 ModuleManager::GetAvailableSlotInBank(EVEEffectID slotBank)
{
	// Call into ModuleContainer class with slotBank effectID to have it check for and return first available slot flag in
	// in the specified slot bank:
	return m_Modules->GetAvailableSlotInBank(slotBank);
}

bool ModuleManager::InstallRig(InventoryItemRef iRef, EVEItemFlags flag) {
    if (((iRef->groupID() >= EVEDB::invGroups::Rig_Armor) and (iRef->groupID() <= EVEDB::invGroups::Rig_Astronautic))
    or (iRef->groupID() == EVEDB::invGroups::Rig_Electronics_Superiority)) {
        fitModule(iRef,flag);
        return true;
    } else
        codelog(SHIP__MODULE_TRACE, "ModuleManager","%s tried to fit item %s(%u), which is not a rig", m_Ship->GetPilot()->GetName(), iRef->itemName().c_str(), iRef->itemID());

    return false;
}

void ModuleManager::UninstallRig(uint32 itemID)
{
    GenericModule* pMod = m_Modules->GetModule(itemID);
    if (pMod != nullptr) {
        pMod->Offline();
        if (!sConfig.debug.IsTestServer)
            pMod->DestroyRig();
        m_Ship->SetAttribute(AttrUpgradeLoad, (m_Ship->GetAttribute(AttrUpgradeLoad) - pMod->GetAttribute(AttrUpgradeCost)));
    }

    m_Modules->RemoveModule(itemID);
    m_Ship->SetAttribute(AttrUpgradeSlotsLeft, m_Ship->GetAttribute(AttrUpgradeSlotsLeft) +1);
}

bool ModuleManager::InstallSubSystem(InventoryItemRef item, EVEItemFlags flag)
{
    if (item->categoryID() == EVEDB::invCategories::Subsystem)
        return fitModule(item,flag);
    else
        sLog.Warning("ModuleManager","%s tried to fit item %u, which is not a subsystem", m_Ship->GetPilot()->GetName(), item->itemID());

    return false;
}

void ModuleManager::UnfitModule(uint32 itemID)
{
    GenericModule* pMod = m_Modules->GetModule(itemID);
    if (pMod != nullptr) {
        EVEItemFlags flag = flagHangar;
        bool inSpace = IsSolarSystem(m_Ship->locationID());
        if (inSpace)
            flag = flagCargoHold;
        pMod->AbortCycle();
        pMod->Offline();
        if (pMod->IsLoaded()) {
            pMod->GetLoadedChargeRef()->Move((inSpace ? m_Ship->itemID() : m_Ship->locationID()), flag, true);
            pMod->UnloadCharge();    // this does not physically remove charge from module, hence the need for the above call.
        }

        if (pMod->isTurretFitted())
            m_Ship->SetAttribute(AttrTurretSlotsLeft, (m_Ship->GetAttribute(AttrTurretSlotsLeft) +1));
        if (pMod->isLauncherFitted())
            m_Ship->SetAttribute(AttrLauncherSlotsLeft, (m_Ship->GetAttribute(AttrLauncherSlotsLeft) +1));

    }
    m_Modules->RemoveModule(itemID);
}

bool ModuleManager::FitModule(InventoryItemRef item, EVEItemFlags flag)
{
    if (!IsModuleSlot(flag)) {
        sLog.Warning("ModuleManager::fitModule","Slot %s is not a module slot.", sDataMgr.GetFlagName(flag).c_str());
        return false;
    }

    if (item->categoryID() == EVEDB::invCategories::Module) {
        // Attempt to fit the module
        if ( fitModule(item, flag) ) {
            // Now that module is successfully fitted, attempt to put it Online:
            Online(item->itemID());
            return true;
        }
    } else
        sLog.Warning("ModuleManager::fitModule","%s tried to fit item %u, which is not a module", m_Ship->GetPilot()->GetName(), item->itemID());

    return false;
}
/*{'messageKey': 'ModuleActivatedDeniedForceField', 'dataID': 17881053, 'suppressable': False, 'bodyID': 258630, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1797}
 * {'messageKey': 'ModuleActivationDeniedCriminalAssistance', 'dataID': 17875215, 'suppressable': False, 'bodyID': 256427, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3548}
 * {'messageKey': 'ModuleActivationDeniedJumping', 'dataID': 17876072, 'suppressable': False, 'bodyID': 256741, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3450}
 * {'messageKey': 'ModuleAlreadyActive', 'dataID': 17882992, 'suppressable': False, 'bodyID': 259340, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259339, 'messageID': 1224}
 * {'messageKey': 'ModuleAlreadyBanked', 'dataID': 17878036, 'suppressable': False, 'bodyID': 257477, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2671}
 * {'messageKey': 'ModuleAlreadyFitting', 'dataID': 17882995, 'suppressable': False, 'bodyID': 259341, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1225}
 * {'messageKey': 'ModuleEffectActive', 'dataID': 17883214, 'suppressable': False, 'bodyID': 259424, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1226}
 * {'messageKey': 'ModuleFit', 'dataID': 17883325, 'suppressable': False, 'bodyID': 259463, 'messageType': 'notify', 'urlAudio': 'wise:/msg_ModuleFit_play', 'urlIcon': '', 'titleID': None, 'messageID': 1227}
 * {'messageKey': 'ModuleFitFailed', 'dataID': 17883222, 'suppressable': False, 'bodyID': 259427, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1228}
 * {'messageKey': 'ModuleGotDamagedWhileBeingRepaired', 'dataID': 17879486, 'suppressable': False, 'bodyID': 258026, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2389}
 * {'messageKey': 'ModuleIsBlocked', 'dataID': 17878649, 'suppressable': False, 'bodyID': 257706, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2573}
 * {'messageKey': 'ModuleJammedOnBadAmmo', 'dataID': 17883225, 'suppressable': False, 'bodyID': 259428, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1229}
 * {'messageKey': 'ModuleNoLongerPresentForCharges', 'dataID': 17882998, 'suppressable': False, 'bodyID': 259342, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1230}
 * {'messageKey': 'ModuleNotOnline', 'dataID': 17883228, 'suppressable': False, 'bodyID': 259429, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1231}
 * {'messageKey': 'ModuleNotPowered', 'dataID': 17883231, 'suppressable': False, 'bodyID': 259430, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1232}
 * {'messageKey': 'ModuleReactivationDelayed2', 'dataID': 17879578, 'suppressable': False, 'bodyID': 258059, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2307}
 * {'messageKey': 'ModuleRequiresFuel', 'dataID': 17883167, 'suppressable': False, 'bodyID': 259407, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1235}
 * {'messageKey': 'ModuleRequiresLowerSystemSecurity', 'dataID': 17883256, 'suppressable': False, 'bodyID': 259439, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1236}
 * {'messageKey': 'ModuleRequiresTargetOwnerFleetMembership', 'dataID': 17883161, 'suppressable': False, 'bodyID': 259405, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1237}
 * {'messageKey': 'ModuleTooBigForThisShip', 'dataID': 17879517, 'suppressable': False, 'bodyID': 258037, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2424}
 * {'messageKey': 'ModuleTooDamagedToBeOnlined', 'dataID': 17878773, 'suppressable': False, 'bodyID': 257752, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2303}
 * {'messageKey': 'ModuleTooDamagedToRepairGoToStation', 'dataID': 17879349, 'suppressable': False, 'bodyID': 257974, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2387}
 * {'messageKey': 'ModuleUnfit', 'dataID': 17883328, 'suppressable': False, 'bodyID': 259464, 'messageType': 'notify', 'urlAudio': 'wise:/msg_ModuleUnfit_play', 'urlIcon': '', 'titleID': None, 'messageID': 1239}
 * {'messageKey': 'ModulesIncorrectlyFitted', 'dataID': 17878135, 'suppressable': False, 'bodyID': 257513, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2512}
 * {'messageKey': 'ModulesNotLoadableInSpace', 'dataID': 17883271, 'suppressable': False, 'bodyID': 259444, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1240}
 */
bool ModuleManager::fitModule(InventoryItemRef iRef, EVEItemFlags flag)
{
    if (!IsModuleSlot(flag)) {
        sLog.Warning("ModuleManager::fitModule","%s is not a module slot.", sDataMgr.GetFlagName(flag).c_str());
        return false;
    }
	if (m_Modules->GetModule(iRef->itemID()) == nullptr) {
        // create new module object
		GenericModule* pMod = ModuleFactory(iRef, ShipItemRef(m_Ship));
        if (pMod == nullptr)
            return false;
        /** @todo  this needs to get out of here, but has to have GenericModule access for data */
        if (pMod->isMaxGroupFitLimited()) {
            if (m_Modules->GetFittedModuleCountByGroup(iRef->groupID()) == pMod->GetSelf()->GetAttribute(AttrMaxGroupFitted).get_int()) {
                SafeDelete(pMod);
                std::map<std::string, PyRep *> args;
                args["noOfModules"]         = new PyInt(pMod->GetSelf()->GetAttribute(AttrMaxGroupFitted).get_int());
                args["noOfModulesFitted"]   = new PyInt(m_Modules->GetFittedModuleCountByGroup(iRef->groupID()));
                args["ship"]                = new PyInt(m_Ship->itemID());
                args["groupName"]           = new PyString(pMod->GetSelf()->group().name());
                args["module"]              = new PyInt(pMod->itemID());
                throw PyException( MakeUserError("CantFitTooManyByGroup", args));
                /*CantFitTooManyByGroupBody'}(
                 * u"You're unable to fit {[item]module.name} to {[item]ship.name}.
                 * You can only fit {[numeric]noOfModules} of type {groupName} but already have {[numeric]noOfModulesFitted}.", None,
                 * {u'{[numeric]noOfModules}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'noOfModules'},
                 * u'{[numeric]noOfModulesFitted}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'noOfModulesFitted'},
                 * u'{[item]ship.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'ship'},
                 * u'{groupName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'groupName'},
                 * u'{[item]module.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'module'}})
                 */
            }
        }
        if (pMod->isTurretFitted()) {
            // apply config modifier, if applicable
            iRef->MultiplyAttribute(AttrSpeed, sConfig.rates.turretRoF);
            m_Ship->SetAttribute(AttrTurretSlotsLeft, (m_Ship->GetAttribute(AttrTurretSlotsLeft) -1));
        } else if (pMod->isLauncherFitted()) {
            // apply config modifier, if applicable
            iRef->MultiplyAttribute(AttrSpeed, sConfig.rates.missileRoF);
            m_Ship->SetAttribute(AttrLauncherSlotsLeft, (m_Ship->GetAttribute(AttrLauncherSlotsLeft) -1));
		} else if (pMod->isRig()) {
            m_Ship->SetAttribute(AttrUpgradeLoad, (m_Ship->GetAttribute(AttrUpgradeLoad) + pMod->GetAttribute(AttrUpgradeCost)));
            m_Ship->SetAttribute(AttrUpgradeSlotsLeft, (m_Ship->GetAttribute(AttrUpgradeSlotsLeft) -1));
        }

        return m_Modules->AddModule(flag, pMod);
    } else if (m_Modules->isSlotOccupied(flag)) {
        throw PyException( MakeUserError("SlotAlreadyOccupied"));
        /** @todo change this to use movemodule */
        return false;
    }

    // else, module already fit
    return false;
}

bool ModuleManager::OnlineCheck(GenericModule* pMod)
{
    if (pMod->isRig() or pMod->isSubSystem())
        return true;
    if (pMod->GetAttribute(AttrDamage) >= pMod->GetAttribute(AttrHP)) {
        // make error here
        return false;
    }
    // check PG and CPU usage to see if we have enough to online this module
    EvilNumber cpuNeed = (m_Ship->GetAttribute(AttrCpuLoad) + pMod->GetAttribute(AttrCpu));
    if (cpuNeed  > m_Ship->GetAttribute(AttrCpuOutput)) {
        if (!m_Ship->GetPilot()->IsLogin() and m_Ship->GetPilot()->CanThrow()) {
            // throwing an error negates further processing
            float require = m_Ship->GetAttribute(AttrCpu).get_float();
            float total = pMod->GetAttribute(AttrCpuOutput).get_float();
            float remaining = total - pMod->GetAttribute(AttrCpuLoad).get_float();
            std::map<std::string, PyRep *> args;
            args["moduleType"] = new PyInt(pMod->typeID());
            args["require"] = new PyFloat(require);
            args["remaining"] = new PyFloat(remaining);
            args["total"] = new PyFloat(total);
            throw PyException( MakeUserError("NotEnoughCpu", args));
            /*u'NotEnoughCpuBody'}
             * (u'To bring {[item]moduleType.name} online requires {[numeric]require, decimalPlaces=2} cpu units, but only {[numeric]remaining, decimalPlaces=2} of the {[numeric]total, decimalPlaces=2} units that your computer produces are still available.', None,
             * {u'{[numeric]remaining, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'remaining'},
             * u'{[item]moduleType.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'moduleType'},
             * u'{[numeric]total, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'total'},
             * u'{[numeric]require, decimalPlaces=2}': {'conditionalValues': [], 'variableType': 9, 'propertyName': None, 'args': 512, 'kwargs': {'decimalPlaces': 2}, 'variableName': 'require'}})
             */
        }
        return false;
    }
    EvilNumber pgNeed = (m_Ship->GetAttribute(AttrPowerLoad) + pMod->GetAttribute(AttrPower));
    if (pgNeed > m_Ship->GetAttribute(AttrPowerOutput)) {
        if (!m_Ship->GetPilot()->IsLogin() and m_Ship->GetPilot()->CanThrow()) {
            // throwing an error negates further processing
            std::map<std::string, PyRep *> args;
            args["moduleType"] = new PyInt(pMod->typeID());
            args["require"] = new PyFloat(m_Ship->GetAttribute(AttrPower).get_float());
            args["remaining"] = new PyFloat(m_Ship->GetAttribute(AttrPowerOutput).get_float() - pMod->GetAttribute(AttrPowerLoad).get_float());
            args["total"] = new PyFloat(pMod->GetAttribute(AttrPowerOutput).get_float());
            throw PyException( MakeUserError("NotEnoughPower", args));
        }
        return false;
    }
    return true;
}

void ModuleManager::Online(uint32 itemID)
{
    GenericModule* pMod = m_Modules->GetModule(itemID);
    if (pMod != nullptr) {
        if (pMod->isOnline()) {
            _log(SHIP__MODULE_TRACE, "ModuleManager::Online(itemID) -  %s already Online", pMod->GetSelf()->itemName().c_str());
            return;
        }
        if (OnlineCheck(pMod)) {
            _log(SHIP__MODULE_TRACE, "ModuleManager::Online(itemID) -  %s going Online", pMod->GetSelf()->itemName().c_str());
            pMod->Online();
        } else {
            _log(SHIP__MODULE_TRACE, "ModuleManager::Online(itemID) -  Not enough CPU/PG to put %s online.", pMod->GetSelf()->itemName().c_str());
           /* if (m_Ship->HasPilot())
                m_Ship->GetPilot()->SendErrorMsg("Your ship does not have the required resources to online the %s", mod->GetSelf()->itemName().c_str());
                */
        }
    } else
        _log(SHIP__MODULE_ERROR, "ModuleManager::Online(itemID) -  Module %u not found", itemID);
}

void ModuleManager::Online(EVEItemFlags flag)
{
    GenericModule* pMod = m_Modules->GetModule(flag);
    if (pMod != nullptr) {
        if (pMod->isOnline()) {
            _log(SHIP__MODULE_TRACE, "ModuleManager::Online(flag) -  %s already Online", pMod->GetSelf()->itemName().c_str());
            return;
        }
        if (OnlineCheck(pMod)) {
            _log(SHIP__MODULE_TRACE, "ModuleManager::Online(flag) -  %s going Online", pMod->GetSelf()->itemName().c_str());
            pMod->Online();
        } else {
            _log(SHIP__MODULE_TRACE, "ModuleManager::Online(flag) -  Not enough CPU/PG to put %s online.", pMod->GetSelf()->itemName().c_str());
           /* if (m_Ship->HasPilot())
                m_Ship->GetPilot()->SendErrorMsg("Your ship does not have the required resources to online the %s", mod->GetSelf()->itemName().c_str());
                */
        }
    } else
        _log(SHIP__MODULE_ERROR, "ModuleManager::Online(flag) -  Module at location %u not found", flag);
}

void ModuleManager::Offline(uint32 itemID)
{
    GenericModule* pMod = m_Modules->GetModule(itemID);
    if (pMod != nullptr) {
        if (!pMod->isOnline()) {
            _log(SHIP__MODULE_TRACE, "ModuleManager::Offline(itemID) -  %s not Online", pMod->GetSelf()->itemName().c_str());
            pMod->SetModuleState(Module::State::Offline);
            return;
        }
        _log(SHIP__MODULE_TRACE, "ModuleManager::Offline(itemID) -  %s going Offline", pMod->GetSelf()->itemName().c_str());
        pMod->Offline();
    } else
        _log(SHIP__MODULE_ERROR, "ModuleManager::Offline(itemID) -  Module %u not found", itemID);
}

void ModuleManager::Offline(EVEItemFlags flag)
{
    GenericModule* pMod = m_Modules->GetModule(flag);
    if (pMod != nullptr) {
        if (!pMod->isOnline()) {
            _log(SHIP__MODULE_TRACE, "ModuleManager::Offline(flag) -  %s not Online", pMod->GetSelf()->itemName().c_str());
            pMod->SetModuleState(Module::State::Offline);
            return;
        }
        _log(SHIP__MODULE_TRACE, "ModuleManager::Offline(flag) -  %s going Offline", pMod->GetSelf()->itemName().c_str());
        pMod->Offline();
    } else
        _log(SHIP__MODULE_ERROR, "ModuleManager::Offline(flag) -  Module at location %u not found", flag);
}

void ModuleManager::AbortCycle()
{
    m_Modules->AbortCycle();
}

void ModuleManager::OnlineAll()
{
    m_Modules->OnlineAll();
}

void ModuleManager::OfflineAll()
{
    m_Modules->OfflineAll();
}

void ModuleManager::DeactivateAllModules()
{
    m_Modules->DeactivateAll();
}

void ModuleManager::Activate(int32 itemID, uint16 effectID, int32 targetID, int32 repeat)
{
    if (!m_Ship->HasPilot()) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::Activate() - Called from a ship with no pilot." );
        return;
    }

    DestinyManager* pDestiny = m_Ship->GetPilot()->GetShipSE()->DestinyMgr();
    if (pDestiny == nullptr) {
        _log(PLAYER__ERROR, "%s: Ship has no destiny manager!", m_Ship->GetPilot()->GetName());
        return;
    }

    GenericModule* pMod = m_Modules->GetModule(itemID);
    if (pMod == nullptr) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::Activate() - Called on module %u that is not loaded.", itemID );
        return;
    }

    _log(SHIP__MODULE_TRACE, "ModuleManager::Activate() - %s (%u - %s)  targetID: %i, repeat: %i.", \
                pMod->GetSelf()->itemName().c_str(), effectID, sFxDataMgr.GetEffectName(effectID).c_str(), targetID, repeat);

    if (!pMod->isOnline()) {
        if (effectID == 16) { //16    online
            pMod->Online();
        } else {
            // client wont allow activating an offline module.  this is catchall. (but should never hit)
            m_Ship->GetPilot()->SendErrorMsg("You cannot activate an offline module. Ref: ServerError 25164");
        }
        return;
    } else if (pDestiny->IsWarping()) {
        if (pMod->HasAttribute(AttrDisallowActivateOnWarp) or !sFxDataMgr.isWarpSafe(effectID))
            throw PyException( MakeUserError( "DeniedActivateInWarp"));
    } else if (pDestiny->IsCloaked()) {
        throw PyException( MakeUserError( "DeniedActivateCloaked"));
    } else if (m_Ship->GetPilot()->IsJump()) {
        throw PyException( MakeUserError( "DeniedActivateInJump"));
    }

    pMod->Activate(effectID, targetID, repeat);

    /* {'messageKey': 'DeniedActivateCloaked', 'dataID': 17883388, 'suppressable': False, 'bodyID': 259487, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 771}
     * {'messageKey': 'DeniedActivateControlling', 'dataID': 17880010, 'suppressable': False, 'bodyID': 258228, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2230}
     * {'messageKey': 'DeniedActivateFrozen', 'dataID': 17883391, 'suppressable': False, 'bodyID': 259488, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 772}
     * {'messageKey': 'DeniedActivateInJump', 'dataID': 17883394, 'suppressable': False, 'bodyID': 259489, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 773}
     * {'messageKey': 'DeniedActivateInWarp', 'dataID': 17883704, 'suppressable': False, 'bodyID': 259597, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 774}
     * {'messageKey': 'DeniedActivateTargetAssistDisallowed', 'dataID': 17883397, 'suppressable': False, 'bodyID': 259490, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 775}
     * {'messageKey': 'DeniedActivateTargetModuleDisallowed', 'dataID': 17883400, 'suppressable': False, 'bodyID': 259491, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 776}
     * {'messageKey': 'DeniedActivateTargetNotPresent', 'dataID': 17883403, 'suppressable': False, 'bodyID': 259492, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 777}
     * {'messageKey': 'DeniedActivateTargetOffModDisallowed', 'dataID': 17883406, 'suppressable': False, 'bodyID': 259493, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 778}
     */
}

void ModuleManager::Deactivate(uint32 itemID, std::string effectName)
{
    GenericModule* pMod = m_Modules->GetModule(itemID);
    if (pMod != nullptr) {
        if (pMod->GetModuleState() != Module::State::Activated)  // we dont need an error msgs here....this is acceptable, as the module may not be active
            return;
        _log(SHIP__MODULE_TRACE, "ModuleManager::Deactivate() - %s Deactivating - '%s'", pMod->GetSelf()->itemName().c_str(), effectName.c_str());
        pMod->Deactivate(effectName);
    } else
        _log(SHIP__MODULE_ERROR, "ModuleManager::Deactivate() - Called on module %u that is not loaded.", itemID );
}

void ModuleManager::Overload(EVEItemFlags flag)
{
    GenericModule* pMod = m_Modules->GetModule(flag);
    if (pMod == nullptr) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::Overload() - Called on module that is not loaded at slot %i.", (int8)flag );
        return;
    }
    pMod->Overload();
    _log(SHIP__MODULE_TRACE, "ModuleManager::Overload() - %s Overloading...", pMod->GetSelf()->itemName().c_str());
}

void ModuleManager::DeOverload(EVEItemFlags flag)
{
    GenericModule* pMod = m_Modules->GetModule(flag);
    if (pMod == nullptr) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::DeOverload() - Called on module that is not loaded at slot %i.", (int8)flag);
        return;
    }
    pMod->DeOverload();
    _log(SHIP__MODULE_TRACE, "ModuleManager::DeOverload() - %s DeOverload...", pMod->GetSelf()->itemName().c_str());
}

void ModuleManager::DamageModule(uint32 itemID, uint8 amount)
{
    GenericModule* pMod = m_Modules->GetModule(itemID);
    if (pMod == nullptr) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::DamageModule() - Called on module %u that is not loaded.", itemID );
        return;
    }
    pMod->SetAttribute(AttrDamage, (pMod->GetAttribute(AttrDamage) + amount));
    _log(SHIP__MODULE_DAMAGE, "ModuleManager::DamageModule() - %s taking %u damage.  current damage %" PRIi64,  \
                pMod->GetSelf()->itemName().c_str(), amount, pMod->GetAttribute(AttrDamage).get_int());
    if (pMod->GetAttribute(AttrDamage) >= pMod->GetAttribute(AttrHP))
        pMod->Offline();
}

void ModuleManager::DamageModule(GenericModule* pMod, uint8 amount)
{
    if (pMod == nullptr) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::DamageModule() - Module not found.");
        return;
    }
    pMod->SetAttribute(AttrDamage, (pMod->GetAttribute(AttrDamage) + amount));
    _log(SHIP__MODULE_DAMAGE, "ModuleManager::DamageModule() - %s taking %u damage.  current damage %" PRIi64,  \
                pMod->GetSelf()->itemName().c_str(), amount, pMod->GetAttribute(AttrDamage).get_int());
    if (pMod->GetAttribute(AttrDamage) >= pMod->GetAttribute(AttrHP))
        pMod->Offline();
}

void ModuleManager::DamageRandModule()
{
    DamageModule(m_Modules->GetRandModule(), 1);
}

void ModuleManager::DamageRandModule(uint8 amount)
{
    DamageModule(m_Modules->GetRandModule(), amount);
}

void ModuleManager::RepairModule(uint32 itemID, EvilNumber amount)
{
    GenericModule* pMod = m_Modules->GetModule(itemID);
    if (pMod == nullptr) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::RepairModule() - Called on module %u that is not loaded.", itemID );
        return;
    }
    pMod->Repair(amount);
}

void ModuleManager::RepairModule(GenericModule* pMod, EvilNumber amount)
{
    if (pMod != nullptr){
        _log(SHIP__MODULE_ERROR, "ModuleManager::RepairModule() - Called on module that is not loaded.");
        return;
    }
    pMod->Repair(amount);
}

void ModuleManager::RepairModules()
{
    m_Modules->RepairAll();
}

void ModuleManager::LoadCharge(InventoryItemRef chargeRef, EVEItemFlags flag)
{
    if (chargeRef.get() == nullptr)
        if (m_Ship->HasPilot())
            throw PyException( MakeUserError( "CantFindChargeToAdd"));

    if (!IsModuleSlot(flag))
        return; // throw error?

    //CantMoveChargesBetweenModules

    GenericModule* pMod = m_Modules->GetModule(flag);
    if (pMod == nullptr) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::LoadCharge() - module not found at slot %i", flag);
        return;
    }
    float modCapacity = pMod->GetSelf()->GetAttribute(AttrCapacity).get_float();
    float chargeVolume = chargeRef->GetAttribute(AttrVolume).get_float();

    if (pMod->IsLoaded()) {
        InventoryItemRef loadedChargeRef = pMod->GetLoadedChargeRef();
        if ( chargeRef->typeID() != loadedChargeRef->typeID() ) {
            // change charges
            UnloadCharge(flag);
            if (IsStation(m_Ship->locationID())) {
                loadedChargeRef->Move(m_Ship->locationID(), flagHangar, true);
            } else {
                if (m_Ship->ValidateAddItem(flagCargoHold, loadedChargeRef))
                    loadedChargeRef->Move(m_Ship->itemID(), flagCargoHold, true);
                else
                    return; // cant put in cargo.  return without changing charge.
            }
        } else {
            modCapacity -= (loadedChargeRef->GetAttribute(AttrVolume).get_float() * loadedChargeRef->quantity());
            if (modCapacity > chargeVolume) {
                uint32 quantityWeCanLoad = floor(modCapacity / chargeVolume);
                if (quantityWeCanLoad > 0) {
                    /** @todo verify this one....make sure we're right. check if we can actually use LoadCharge here... */
                    if (quantityWeCanLoad < chargeRef->quantity()) {
                        InventoryItemRef loadableChargeQtyRef = chargeRef->Split(quantityWeCanLoad);
                        loadedChargeRef->Merge(loadableChargeQtyRef);
                    } else {
                        loadedChargeRef->Merge(chargeRef);
                    }
                    // we have loaded charge and updated qty. send reload update to client
                    PyTuple* module = new PyTuple(1);
                        module->SetItem(0, new PyInt(pMod->itemID()));
                    PyTuple* tmp = new PyTuple(3);
                        tmp->SetItem(0, module);
                        tmp->SetItem(1, new PyInt(chargeRef->typeID()));
                        tmp->SetItem(2, new PyInt(pMod->GetReloadTime()));
                    m_Ship->GetPilot()->SendNotification("OnChargeBeingLoadedToModule", "shipid", &tmp, false); //unsequenced.
                    return;
                } else {
                    return;   // cant even load one.  make error?
                }
            } else {
                return;   // cant even load one.  make error?
            }
        }
    } else {
        modCapacity = pMod->GetAttribute(AttrCapacity).get_float();
        if (modCapacity < (chargeVolume * chargeRef->quantity())) {
            uint32 quantityWeCanLoad = floor((modCapacity / chargeVolume));
            if (quantityWeCanLoad > 0) {
                InventoryItemRef loadableChargeQtyRef = chargeRef->Split( quantityWeCanLoad );
                chargeRef = loadableChargeQtyRef;
            }
        }
    }

    chargeRef->Donate(m_Ship->ownerID(), m_Ship->itemID(), flag, true);
    pMod->LoadCharge(chargeRef);
    m_charges.emplace(flag, chargeRef);
}

void ModuleManager::UnloadCharge(EVEItemFlags flag)
{
    GenericModule* pMod = m_Modules->GetModule(flag);
    if (pMod != nullptr) {
        if (pMod->IsLoaded() ) {
            _log(SHIP__MODULE_TRACE, "ModuleManager::UnloadCharge() - %s unloading %s",
                    pMod->GetSelf()->itemName().c_str(), pMod->GetLoadedChargeRef()->itemName().c_str());
            pMod->UnloadCharge();
            m_charges.erase(flag);
        } else
            _log(SHIP__MODULE_ERROR, "ModuleManager::UnloadCharge() - module %s at slot %i is not loaded", pMod->GetSelf()->itemName().c_str(), flag);
    } else
        _log(SHIP__MODULE_ERROR, "ModuleManager::UnloadCharge() - module not found at slot %i", flag);
}

void ModuleManager::GetLoadedCharges(std::map< EVEItemFlags, InventoryItemRef >& charges)
{
    charges = m_charges;
}

InventoryItemRef ModuleManager::GetLoadedChargeOnModule(EVEItemFlags flag) {
    GenericModule* pMod = m_Modules->GetModule(flag);
    if ((pMod != nullptr) and pMod->IsLoaded() )
        return pMod->GetLoadedChargeRef();
    return InventoryItemRef();
}

InventoryItemRef ModuleManager::GetLoadedChargeOnModule(InventoryItemRef moduleRef) {
    GenericModule* pMod = m_Modules->GetModule(moduleRef->itemID());
    if ((pMod != nullptr) and pMod->IsLoaded() )
        return pMod->GetLoadedChargeRef();
    return InventoryItemRef();
}

bool ModuleManager::VerifySlotExchange(EVEItemFlags slot1, EVEItemFlags slot2)
{
    if (m_Modules->GetModule(slot1)->GetModulePowerLevel() == m_Modules->GetModule(slot2)->GetModulePowerLevel())
        return true;
    return false;
}

void ModuleManager::UnloadAllModules()
{
    m_Modules->UnloadAll();
}

void ModuleManager::UpdateModules(std::vector<uint32> modVec)
{
    sLog.Magenta("ModuleManager::UpdateModules()","Needs to be tested");
    // this one is called from BoardShip() and Ship::Undock()
    GenericModule* pMod(nullptr);
    m_Ship->SetAttribute(AttrCpuLoad,     0);
    m_Ship->SetAttribute(AttrPowerLoad,   0);
    //m_Ship->SetAttribute(AttrUpgradeLoad, 0);  -> rigs do NOT get removed/disabled when changing pilots
    if (modVec.empty()) {
        OnlineAll();
    } else {
        _log(SHIP__MODULE_TRACE, "ModuleManager::UpdateModules(modVec)");
        // gotta add rigs and Subsystems to the vector, as they wont be listed in the "modules to online" list when undocking.
        GetShipRigs(modVec);
        GetShipSubSystems(modVec);
        std::vector< GenericModule* > modList;
        SortModulesBySlotDec(modVec, modList);
        for (auto cur : modList) {
            if (m_Ship->IsUndocking())
                cur->SetAttribute(AttrIsOnline, false, false);
            cur->Online();
            //if (cur->IsLoaded())
            //    cur->ReprocessCharge();
        }
    }
}

void ModuleManager::UpdateModules(EVEItemFlags flag)
{
    /** @todo  figure out what needs to be done here and implement it. */
    //  this should update all ship attribs for this bank.
    sLog.Magenta("ModuleManager::UpdateModules(flag)","Needs to be implemented");

    // reset ship and module effect data, and reapply?
    // call ProcessEffects(false), ApplyEffects(), then UpdateModules() ?
}

void ModuleManager::CharacterBoardingShip()
{
    sLog.Magenta("ModuleManager::CharacterBoardingShip()","Needs to be tested");
    if (!m_initalized)
        Initialize();

    OnlineAll();
}

void ModuleManager::CharacterLeavingShip()
{
    // if ship is killed, no point setting modules to offline...just return
    if (m_Ship->IsPopped())
        return;

    sLog.Magenta("ModuleManager::CharacterLeavingShip()","Needs to be implemented");
    //this is complicated and im gonna leave it alone for now until
    //a few things become more clear
    //OfflineAll();
}

void ModuleManager::ShipWarping()
{
    sLog.Magenta("ModuleManager::ShipWarping()","Deactivating non-warpsafe modules.");
    // check modules for warpsafe-ness and Deactivate accordingly
    m_Modules->ShipWarping();
}

void ModuleManager::ShipJumping()
{
    sLog.Magenta("ModuleManager::ShipJumping()","Deactivating all modules.");

    // no modules are jumpsafe
    AbortCycle();
}

void ModuleManager::GetModuleListOfRefsAsc(std::vector<InventoryItemRef> * pModuleList)
{
	m_Modules->GetModuleListOfRefsAsc(pModuleList);
}

void ModuleManager::GetModuleListOfRefsDec(std::vector< InventoryItemRef >* pModuleList)
{
    m_Modules->GetModuleListOfRefsDec(pModuleList);
}

void ModuleManager::StripModules()
{
    m_Modules->ClearModMap();
}

void ModuleManager::SaveModules()
{
    m_Modules->SaveModules();
}

void ModuleManager::GetShipRigs(std::vector< uint32 >& modVec)
{
    // get rigs on ship, by itemID (there's only 3 slots...)
    GenericModule* pMod(nullptr);
    for (uint8 i = flagRigSlot0; i < flagRigSlot3; ++i) {
        pMod = m_Modules->GetModule((EVEItemFlags)i);
        if (pMod != nullptr)
            modVec.push_back(pMod->itemID());
        pMod = nullptr;
    }
}

void ModuleManager::GetShipSubSystems(std::vector< uint32 >& modVec)
{
    // get subsystems on ship, by itemID (there's only 5 slots...)
    GenericModule* pMod(nullptr);
    for (uint8 i = flagSubSystem0; i < flagSubSystem5; ++i) {
        pMod = m_Modules->GetModule((EVEItemFlags)i);
        if (pMod != nullptr)
            modVec.push_back(pMod->itemID());
        pMod = nullptr;
    }
}

void ModuleManager::GetModuleListByReqSkill(uint16 skillID, std::vector< InventoryItemRef >* pModuleList)
{
    std::vector<InventoryItemRef> moduleList;
    GetModuleListOfRefsAsc(&moduleList);
    for (auto cur : moduleList)
        if (cur->HasReqSkill(skillID))
            pModuleList->push_back(cur);
}

void ModuleManager::SortModulesBySlotDec(std::vector<uint32>& modVec, std::vector< GenericModule* >& pModList)
{
    if (modVec.empty())
        return;
    GenericModule* pMod(nullptr);
    std::map<uint8, GenericModule*> tmpList;
    for (auto cur : modVec) {
        pMod = GetModule(cur);
        if (pMod != nullptr)
            tmpList.insert(std::pair<uint8, GenericModule*>((uint8)pMod->flag(), pMod));
        pMod = nullptr;
    }
    if (tmpList.empty())
        return;
    std::map<uint8, GenericModule*>::reverse_iterator itr = tmpList.rbegin();
    for (; itr != tmpList.rend(); ++itr)
        pModList.push_back(itr->second);

}

//////////////////////////////////////////////////////////////////////////////////
