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
#pragma region ModuleContainerClass
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

    initializeModuleContainers();
}

ModuleContainer::~ModuleContainer()
{
    for (std::map<uint8, GenericModule*>::iterator itr = m_modules.begin(); itr != m_modules.end(); itr++)
        SafeDelete(itr->second);
}

void ModuleContainer::initializeModuleContainers() {
    m_turrents = 0;
    m_launchers = 0;

    for (uint8 flag = 11; flag < 35; ++flag)    // modules
        m_modules.insert(std::pair<uint8, GenericModule*>(flag, nullptr));
    if (m_RigSlots)
        for (uint8 flag = 92; flag < 100; ++flag)   // rigs
            m_modules.insert(std::pair<uint8, GenericModule*>(flag, nullptr));
    if (m_SubSystemSlots)
        for (uint8 flag = 125; flag < 134; ++flag)  // subsystems
            m_modules.insert(std::pair<uint8, GenericModule*>(flag, nullptr));
}

bool ModuleContainer::AddModule(EVEItemFlags flag, GenericModule* mod)
{
    std::map<uint8, GenericModule*>::iterator itr = m_modules.find(flag);
    if (itr == m_modules.end())
        return false;
    else
        itr->second = mod;

    // Maintain Turret and Launcher Fitted module counts:
    if ( mod->isTurretFitted() )
        ++m_turrents;
    if ( mod->isLauncherFitted() )
        ++m_launchers;

    // Maintain the Modules Fitted By Group counter for this module group:
    if ( m_ModulesFittedByGroupID.find(mod->getItem()->groupID()) != m_ModulesFittedByGroupID.end() )
        m_ModulesFittedByGroupID.find(mod->getItem()->groupID())->second++;
    else
        m_ModulesFittedByGroupID.insert(std::pair<uint32,uint32>(mod->getItem()->groupID(), 1));

    // module is fit so change state from MOD_UNFITTED to MOD_OFFLINE now.
    mod->SetModuleState(MOD_OFFLINE);
	return true;
}

bool ModuleContainer::RemoveModule(EVEItemFlags flag) {
    GenericModule* mod = GetModule(flag);
    if (!mod)
        return false;

    deleteModuleRef(mod->flag(), mod);
	return true;
}

bool ModuleContainer::RemoveModule(uint32 itemID) {
    GenericModule* mod = GetModule(itemID);
    if (!mod)
        return false;

    deleteModuleRef(mod->flag(), mod);
    return true;
}

void ModuleContainer::StripModules()
{
    for (uint8 flag = 11; flag < 35; ++flag)
        m_modules.insert(std::pair<uint8, GenericModule*>(flag, nullptr));
}

GenericModule* ModuleContainer::GetModule(EVEItemFlags flag)
{
    std::map<uint8, GenericModule*>::const_iterator itr = m_modules.find(flag);
    if (itr != m_modules.end())
        return itr->second;

    return nullptr;
}

GenericModule* ModuleContainer::GetModule(uint32 itemID)
{
    //iterate through the list and see if we have it
    std::map<uint8, GenericModule*>::iterator itr = m_modules.begin();
    while (itr != m_modules.end()) {
        if ((itr->second) and (itr->second->itemID() == itemID))
            return itr->second;
        else
            ++itr;
    }
    return nullptr;  //we don't
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

void ModuleContainer::process(processType p)
{
    switch(p) {
        case typeOnlineAll: {
            std::map<uint8, GenericModule*>::iterator itr = m_modules.begin();
            while (itr != m_modules.end()) {
                if (itr->second)
                    itr->second->Online();
                ++itr;
            }
        } break;
        case typeOfflineAll: {
            std::map<uint8, GenericModule*>::iterator itr = m_modules.begin();
            while (itr != m_modules.end()) {
                if (itr->second)
                    itr->second->Offline();
                ++itr;
            }
        } break;
        case typeDeactivateAll: {
            std::map<uint8, GenericModule*>::iterator itr = m_modules.begin();
            while (itr != m_modules.end()) {
                if (itr->second)
                    itr->second->Deactivate();
                ++itr;
            }
        } break;
        case typeUnloadAll: {
            std::map<uint8, GenericModule*>::iterator itr = m_modules.begin();
            while (itr != m_modules.end()) {
                if (itr->second)
                    itr->second->UnloadCharge();
                ++itr;
            }
        } break;
        case typeProcessAll: {
            std::map<uint8, GenericModule*>::iterator itr = m_modules.begin();
            while (itr != m_modules.end()) {
                if (itr->second)
                    itr->second->Process();
                ++itr;
            }
        } break;
        case typeAbort: {
            std::map<uint8, GenericModule*>::iterator itr = m_modules.begin();
            while (itr != m_modules.end()) {
                if ((itr->second) and (itr->second->IsActiveModule()))
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
        case effectLoPower: {
            for (uint8 slot=flagLowSlot0; slot < (flagLowSlot0 + m_LowSlots); slot++)
				if ( m_modules[slot] == nullptr )
					return slot;
            } break;
		case effectMedPower: {
            for (uint8 slot=flagMedSlot0; slot < (flagMedSlot0 + m_MediumSlots); slot++)
                if ( m_modules[slot] == nullptr )
					return slot;
            } break;
		case effectHiPower: {
            for (uint8 slot=flagHiSlot0; slot < (flagHiSlot0 + m_HighSlots); slot++)
                if ( m_modules[slot] == nullptr )
					return slot;
            } break;
		case effectRigSlot: {
            for (uint8 slot=flagRigSlot0; slot < (flagRigSlot0 + m_RigSlots); slot++)
                if ( m_modules[slot] == nullptr )
					return slot;
            } break;
		case effectSubSystem: {
            for (uint8 slot=flagSubSystem0; slot < (flagSubSystem0 + m_SubSystemSlots); slot++)
                if ( m_modules[slot] == nullptr )
					return slot;
            } break;
		default: {
			// ERROR: This is not a module that fits in any of the slot banks
            return flagIllegal;
            } break;
	}
}

uint32 ModuleContainer::GetFittedModuleCountByGroup(uint32 groupID)
{
    if ( m_ModulesFittedByGroupID.find(groupID) == m_ModulesFittedByGroupID.end() )
        return 0;
    else
        return m_ModulesFittedByGroupID.find(groupID)->second;
}

void ModuleContainer::GetModuleListOfRefs(std::vector<InventoryItemRef> * pModuleList)
{
    std::map<uint8, GenericModule*>::iterator itr = m_modules.begin();
    while (itr != m_modules.end()) {
        if (itr->second)
            pModuleList->push_back( itr->second->getItem() );
        ++itr;
    }
}

void ModuleContainer::SaveModules()
{
    std::map<uint8, GenericModule*>::iterator itr = m_modules.begin();
    while (itr != m_modules.end()) {
        if (itr->second)
            itr->second->getItem()->SaveItem();
        ++itr;
    }
}

void ModuleContainer::deleteModuleRef(EVEItemFlags flag, GenericModule* mod)
{
    std::map<uint8, GenericModule*>::iterator itr = m_modules.find(flag);
    if (itr != m_modules.end())
        itr->second = nullptr;

    // Maintain Turret and Launcher Fitted module counts:
    if ( mod->isTurretFitted() )
        --m_turrents;
    if ( mod->isLauncherFitted() )
        --m_launchers;

    // Maintain the Modules Fitted By Group counter for this module group:
    if (m_ModulesFittedByGroupID.find(mod->getItem()->groupID()) != m_ModulesFittedByGroupID.end()) {
        if (m_ModulesFittedByGroupID.find(mod->getItem()->groupID())->second > 1) {
            // We still have more than one module of this group fitted, so just reduce number fitted by 1:
            --(m_ModulesFittedByGroupID.find(mod->getItem()->groupID())->second);
        } else {
            // This was the last module of this group fitted, so remove the entry from the map:
            m_ModulesFittedByGroupID.erase(mod->getItem()->groupID());
        }
    } else
        sLog.Error( "ModuleContainer::deleteModuleRef()", "Removing Module from ship fit when it had NO entry in m_ModulesFittedByGroup !" );

    mod->SetModuleState(MOD_UNFITTED);
}

#pragma endregion
/////////////////////////// END MODULECONTAINER //////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
// ModuleManager class definitions
#pragma region ModuleManagerClass
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
    if (m_initalized) {
        // this ship is already initalized and active (in case of BoardShip() or reactivation)
        OnlineAll();    // use CharacterBoardingShip() as it will call all needed functions to correctly apply modifiers
        return true;
    }

    // Load modules, rigs and subsystems from Ship's inventory into ModuleContainer:
    std::vector<InventoryItemRef> itemVec;
    m_Ship->GetInventory()->GetInventoryVec(itemVec);   // this method also sorts in order - cargo, modules, charge, subsystems.
    GenericModule* mod = nullptr;
    for (auto cur : itemVec) {
        if (cur->flag() == flagCargoHold) continue;
        if (cur->categoryID() == EVEDB::invCategories::Module) {
            mod = ModuleFactory(cur, ShipItemRef(m_Ship));  // rigs are modules
            if (m_Modules->AddModule(cur->flag(), mod)) {
                Online(cur->flag());
            } else {
                _log(SHIP__ERROR, "ModuleManager::Initialize() - Could not insert module %s(%u) at flag %u into module container.",
                     cur->itemName().c_str(), cur->itemID(), cur->flag() );
            }
            continue;
        } else if (cur->categoryID() == EVEDB::invCategories::Charge) {
            if (GetModule(cur->flag())) {
                GetModule(cur->flag())->LoadCharge(cur);
            } else {
                _log(SHIP__ERROR, "ModuleManager::Initialize() - Cannot find module to load charge %s(%u) into at flag %u",
                     cur->itemName().c_str(), cur->itemID(), cur->flag() );
            }
            continue;
        } else if (cur->categoryID() == EVEDB::invCategories::Subsystem) {
            mod = ModuleFactory(cur, ShipItemRef(m_Ship));
            if (m_Modules->AddModule(cur->flag(), mod)) {
                Online(cur->flag());
            } else {
                _log(SHIP__ERROR, "ModuleManager::Initialize() - Could not insert Subsystem %s(%u) at flag %u into module container.",
                     cur->itemName().c_str(), cur->itemID(), cur->flag() );
            }
            continue;
        } else
            return false;
    }

    return (m_initalized = true);
}

void ModuleManager::Process()
{
    double profileStartTime = 0.0;
    if (sConfig.server.UseProfiling)
        profileStartTime = GetTimeUSeconds();

    m_Modules->Process();

    if (sConfig.server.UseProfiling)
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

bool ModuleManager::InstallRig(InventoryItemRef item, EVEItemFlags flag) {
    uint8 slots = m_Ship->GetAttribute(AttrUpgradeSlotsLeft).get_int();
    if (!slots) {
        /* send error to player?  or does client do it?  dunno...  */
        codelog(SHIP__MODULE_TRACE, "ModuleManager","%s tried to fit item %u, which is not a rig", m_Ship->GetPilot()->GetName(), item->itemID());
        return false;
    }
    if (((item->groupID() >= 773) and (item->groupID() <= 782)) or (item->groupID() == 786)) {
        fitModule(item,flag);
        m_Ship->SetAttribute(AttrUpgradeSlotsLeft, --slots);
        return true;
    } else
        codelog(SHIP__MODULE_TRACE, "ModuleManager","%s tried to fit item %u, which is not a rig", m_Ship->GetPilot()->GetName(), item->itemID());

    return false;
}

void ModuleManager::UninstallRig(uint32 itemID)
{
    GenericModule* mod = m_Modules->GetModule(itemID);
    if (mod) {
        mod->Offline();
        if (!sConfig.server.IsTestServer)
            mod->DestroyRig();
    }
    m_Modules->RemoveModule(itemID);
    m_Ship->SetAttribute(AttrUpgradeSlotsLeft, m_Ship->GetAttribute(AttrUpgradeSlotsLeft) +1);
}

bool ModuleManager::InstallSubSystem(InventoryItemRef item, EVEItemFlags flag)
{
    if (item->categoryID() == EVEDB::invCategories::Subsystem) {
        fitModule(item,flag);
        return true;
    } else
        sLog.Warning("ModuleManager","%s tried to fit item %u, which is not a subsystem", m_Ship->GetPilot()->GetName(), item->itemID());

    return false;
}

void ModuleManager::UnfitModule(uint32 itemID)
{
    GenericModule* mod = m_Modules->GetModule(itemID);
    if (mod) {
        EVEItemFlags flag = flagCargoHold;
        bool inSpace = (IsStation(m_Ship->locationID()) ? false : true);
        if (!inSpace)
            flag = flagHangar;
        if (mod->IsLoaded()) {
            mod->GetLoadedChargeRef()->Move((inSpace ? m_Ship->itemID() : m_Ship->locationID()), flag);
            mod->UnloadCharge();
        }
        if (mod->isOnline())
            mod->Offline();
        // dont actually move the module here....let the caller do that in it's specific code
        //mod->getItem()->Move((inSpace ? m_Ship->itemID() : m_Ship->locationID()), flag);
    }
    m_Modules->RemoveModule(itemID);
}

bool ModuleManager::FitModule(InventoryItemRef item, EVEItemFlags flag)
{
    if (item->categoryID() == EVEDB::invCategories::Module) {
        // Attempt to fit the module
        if ( fitModule(item, flag) ) {
            // Now that module is successfully fitted, attempt to put it Online:
            Online(item->itemID());
            return true;
        }
    } else
        sLog.Warning("ModuleManager","%s tried to fit item %u, which is not a module", m_Ship->GetPilot()->GetName(), item->itemID());

    return false;
}

bool ModuleManager::fitModule(InventoryItemRef item, EVEItemFlags flag)
{
    bool verifyFailed = false;
	GenericModule* existingMod = m_Modules->GetModule(item->itemID());
	if (existingMod) {
		if (m_Modules->isSlotOccupied(flag))
			throw PyException( MakeUserError("SlotAlreadyOccupied"));
        /** @todo change this to use movemodule */
		return false;
	} else {
        // create new module object
		GenericModule* mod = ModuleFactory(item, ShipItemRef(m_Ship));
		if (!mod) return false;
		// Check for max turret modules allowed:
		if (mod->isTurretFitted() and (m_Modules->GetFittedTurretCount() == m_Ship->GetMaxTurrentHardpoints().get_int())) {
			throw PyException( MakeUserError( "NotEnoughTurretSlots" ) );
			verifyFailed = true;
		}
		// Check for max launcher modules allowed:
		if (mod->isLauncherFitted() and (m_Modules->GetFittedLauncherCount() == m_Ship->GetMaxLauncherHardpoints().get_int())) {
			throw PyException( MakeUserError( "NotEnoughLauncherSlots" ) );
			verifyFailed = true;
		}
		// Check for max modules of group allowed:
		if (mod->isMaxGroupFitLimited() and (m_Modules->GetFittedModuleCountByGroup(item->groupID()) == mod->getItem()->GetAttribute(AttrMaxGroupFitted).get_int())) {
			throw PyException( MakeUserError( "CantFitTooManyByGroup" ) );
			verifyFailed = true;
		}
        if (verifyFailed) {
            if (mod)
                SafeDelete(mod);
            return false;
        }
		// Fit Module now that all checks have passed:
		return m_Modules->AddModule(flag, mod);
	}
}

bool ModuleManager::OnlineCheck(GenericModule* mod)
{
    /** @update client will not call "Online()" if ship doesnt meet requirements.  */
    return true;
    if (mod->isRig() or mod->isSubSystem()) return true;
    if (m_Ship->GetPilot()->IsLogin()) return true;
    // check PG and CPU usage to see if we have enough to online this module
    EvilNumber cpuNeed = (m_Ship->GetAttribute(AttrCpuLoad) + mod->GetAttribute(AttrCpu));
    if (cpuNeed  > m_Ship->GetAttribute(AttrCpuOutput))
        return false;
    EvilNumber pgNeed = (m_Ship->GetAttribute(AttrPowerLoad) + mod->GetAttribute(AttrPower));
    if (pgNeed > m_Ship->GetAttribute(AttrPowerOutput))
        return false;
    return true;
}

void ModuleManager::Online(uint32 itemID)
{
    GenericModule* mod = m_Modules->GetModule(itemID);
    if (mod) {
        if (mod->isOnline())
            return;
        if (OnlineCheck(mod)) {
            _log(SHIP__MODULE_TRACE, "ModuleManager::Online(itemID) -  %s going Online", mod->getItem()->itemName().c_str());
            mod->Online();
        } else
            _log(SHIP__MODULE_TRACE, "ModuleManager::Online(itemID) -  Not enough CPU/PG to put %s online.", mod->getItem()->itemName().c_str());
    } else
        _log(SHIP__MODULE_ERROR, "ModuleManager::Online(itemID) -  Module %u not found", itemID);
}

void ModuleManager::Online(EVEItemFlags flag)
{
    GenericModule* mod = m_Modules->GetModule(flag);
    if (mod) {
        if (mod->isOnline())
            return;
        if (OnlineCheck(mod)) {
            _log(SHIP__MODULE_TRACE, "ModuleManager::Online(flag) -  %s going Online", mod->getItem()->itemName().c_str());
            mod->Online();
        } else
            _log(SHIP__MODULE_TRACE, "ModuleManager::Online(flag) -  Not enough CPU/PG to put %s online.", mod->getItem()->itemName().c_str());
    } else
        _log(SHIP__MODULE_ERROR, "ModuleManager::Online(flag) -  Module at location %u not found", flag);
}

void ModuleManager::Offline(uint32 itemID)
{
    GenericModule* mod = m_Modules->GetModule(itemID);
    if (mod) {
        if (!mod->isOnline())
            return;
        _log(SHIP__MODULE_TRACE, "ModuleManager::Offline(itemID) -  %s going Offline", mod->getItem()->itemName().c_str());
        mod->Offline();
    } else
        _log(SHIP__MODULE_ERROR, "ModuleManager::Offline(itemID) -  Module %u not found", itemID);
}

void ModuleManager::Offline(EVEItemFlags flag)
{
    GenericModule* mod = m_Modules->GetModule(flag);
    if (mod) {
        _log(SHIP__MODULE_TRACE, "ModuleManager::Offline(flag) -  %s going Offline", mod->getItem()->itemName().c_str());
        mod->Offline();
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

void ModuleManager::Activate(uint32 itemID, std::string effectName, uint32 targetID, int32 repeat)
{
    if (!m_Ship->HasPilot()) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::Activate() - Called from a ship with no pilot." );
        return;
    }
    GenericModule* mod = m_Modules->GetModule(itemID);
    if (!mod) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::Activate() - Called on a module that is not loaded." );
        return;
    } else if (!mod->isOnline()) {
        if (effectName == "online") {
            mod->Online();
        } else {
            // client wont allow activating an offline module.  this is catchall.
            m_Ship->GetPilot()->SendErrorMsg("You cannot activate an offline module. Ref: ServerError 25164");
        }
        return;
    } else {
        _log(SHIP__MODULE_TRACE, "ModuleManager::Activate() - %s (%s).", mod->getItem()->itemName().c_str(), effectName.c_str());
        mod->SetRepeat(repeat);
        SystemEntity* pSE(nullptr);
        if (sFxDataMgr.needsTarget(effectName)) {
            if (!targetID) {
                sLog.Error("ModuleManager::Activate()", "targetID == 0");
                m_Ship->GetPilot()->SendErrorMsg("You must have a target to activate that module.  Ref: ServerError 25268");
                return;
            }
            pSE = m_Ship->GetPilot()->SystemMgr()->GetSE(targetID);
            if (!pSE) {
                sLog.Error("ModuleManager::Activate()", "pSE == NULL");
                m_Ship->GetPilot()->SendErrorMsg("Current target was not found.  Ref: ServerError 25263");
                return;
            }
        }

        if (effectName == "cloaking") {//FIXME  set this to use module code, drain cap, etc.
            if (m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->IsCloaked())
                m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->UnCloak();
            else    //MakeUserError("CantCloakProximity");
                m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->Cloak();
            /** @todo  not working right....check for attrib '10' being added and error msgs with ServerError 25610 */
        } else {
            mod->Activate(pSE);
        }
    }
}

void ModuleManager::Deactivate(uint32 itemID, std::string effectName)
{
    GenericModule* mod = m_Modules->GetModule(itemID);
    if (mod) {
        _log(SHIP__MODULE_TRACE, "ModuleManager::Deactivate() - %s Deactivating - '%s'", mod->getItem()->itemName().c_str(), effectName.c_str());
        if (effectName == "online") {
			mod->Offline();
        } else {
			mod->Deactivate();
        }
    }
}

void ModuleManager::Overload(EVEItemFlags flag)
{
    GenericModule* mod = m_Modules->GetModule(flag);
    if (mod) {
        mod->Overload();
        _log(SHIP__MODULE_TRACE, "ModuleManager::Overload() - %s Overloading...", mod->getItem()->itemName().c_str());
    }
}

void ModuleManager::DeOverload(EVEItemFlags flag)
{
    GenericModule* mod = m_Modules->GetModule(flag);
    if (mod) {
        mod->DeOverload();
        _log(SHIP__MODULE_TRACE, "ModuleManager::DeOverload() - %s DeOverload...", mod->getItem()->itemName().c_str());
    }
}

void ModuleManager::DamageModule(uint32 itemID, EvilNumber val)
{
    GenericModule* mod = m_Modules->GetModule(itemID);
    if (mod) {
        mod->SetAttribute(AttrHP, val);
        _log(SHIP__MODULE_TRACE, "ModuleManager::DamageModule() - %s taking %f damage.", mod->getItem()->itemName().c_str(), val.get_float());
    }
}

void ModuleManager::RepairModule(uint32 itemID)
{
    GenericModule* mod = m_Modules->GetModule(itemID);
    if (mod)
        mod->Repair();
}

/** @todo   need to update this */
void ModuleManager::LoadCharge(InventoryItemRef chargeRef, EVEItemFlags flag)
{
    GenericModule* mod = m_Modules->GetModule(flag);
    if (mod) {
		// Scenarios to handle:
		// + no charge loaded: check capacity >= volume of charge to add, if true, LOAD
		//     - ELSE: if charge to load is qty > 1, calculate smallest integer qty that will EQUAL capacity, SPLIT remainder off, then LOAD!
		// + some charge loaded: check capacity >= volume of charge to add, if true, MERGE new charge to existing
		//     - ELSE: if charge to load is qty > 1, calculate smallest integer qty that added to existing charge qty will EQUAL capacity, SPLIT remainder off, then LOAD!

		// Key facts to get:
		// * existing charge ref -> qty and volume/unit
		// * module ref -> capacity of module
		// * charge to add ref -> qty and volume/unit

		EvilNumber modCapacity = mod->getItem()->GetAttribute(AttrCapacity);
		EvilNumber chargeToLoadVolume = chargeRef->GetAttribute(AttrVolume);
		EvilNumber chargeToLoadQty = EvilNumber(chargeRef->quantity());

		/////////////////////////////////////////
		// chargeRef->Split();
		// chargeRef->Merge();
		// mod->Load(chargeRef);
		// chargeRef->Move(m_Ship->itemID(), flag);		// used to be (m_pOperator->GetLocationID(), flag)
		/////////////////////////////////////////

		//m_Ship->GetPilot()->MoveItem(chargeRef->itemID(), m_Ship->itemID(), flag);

		if ( mod->IsLoaded() )
		{
			// Module is loaded, let's check available capacity:
			InventoryItemRef loadedChargeRef = mod->GetLoadedChargeRef();
			EvilNumber loadedChargeVolume = loadedChargeRef->GetAttribute(AttrVolume);
			EvilNumber loadedChargeQty = EvilNumber(loadedChargeRef->quantity());
			modCapacity -= (loadedChargeVolume * loadedChargeQty);		// Calculate remaining capacity
			if ( chargeRef->typeID() != loadedChargeRef->typeID() )
			{
				// Different charge type is being swapped into this module, so unload what's loaded
				if ( IsStation(m_Ship->GetPilot()->GetLocationID()) )
					loadedChargeRef->Move(m_Ship->locationID(), flagHangar);
				else
				{
					m_Ship->ValidateAddItem(flagCargoHold,loadedChargeRef);
					loadedChargeRef->Move(m_Ship->itemID(), flagCargoHold);
				}
				mod->UnloadCharge();

				// Loading of charge will be performed below
			}
			else
			{
				if ( modCapacity > chargeToLoadVolume )
				{
					// Great!  We can load at least one, let's top off the loaded charges:
					uint32 quantityWeCanLoad = floor((modCapacity / chargeToLoadVolume).get_float());
					if ( quantityWeCanLoad > 0 )
					{
						if ( quantityWeCanLoad < chargeToLoadQty.get_int() )
						{
							// Split chargeRef to qty 'quantityWeCanLoad'
							// Merge new smaller qty 'quantityWeCanLoad' with loadedChargeRef
							// Load this merged charge Ref into module
							InventoryItemRef loadableChargeQtyRef = chargeRef->Split( quantityWeCanLoad );
							loadableChargeQtyRef->ChangeOwner( chargeRef->ownerID() );
							loadedChargeRef->Merge( loadableChargeQtyRef );
							mod->LoadCharge( loadedChargeRef );
							loadedChargeRef->Move(m_Ship->itemID(), flag);		// used to be (m_pOperator->GetLocationID(), flag)
						}
						else
						{
							// Merge chargeRef with loadedChargeRef
							// Load this merged charge Ref into module
							loadedChargeRef->Merge( chargeRef );
							mod->LoadCharge( loadedChargeRef );
							loadedChargeRef->Move(m_Ship->itemID(), flag);		// used to be (m_pOperator->GetLocationID(), flag)
						}
					}
					else
						throw PyException( MakeCustomError( "Cannot load even one unit of this charge!" ) );
				}
				else
				{
					throw PyException( MakeCustomError( "Charge is full!" ) );
				}
			}
		}

		// Refresh ammo capacity of module in case it was modified in previous code block ahead of a load action:
		modCapacity = mod->getItem()->GetAttribute(AttrCapacity);

		// Load charge supplied if this module was either never loaded, or just unloaded from a different type right above:
		if ( !(mod->IsLoaded()) )
		{
			// Module is not loaded at all, let's check total volume of charge to load against available capacity:
			if ( modCapacity >= (chargeToLoadVolume * chargeToLoadQty) )
			{
				// We can insert entire stack of chargeRef into module
				// Load chargeRef as-is into module
				mod->LoadCharge( chargeRef );
				chargeRef->Move(m_Ship->itemID(), flag);		// used to be (m_pOperator->GetLocationID(), flag)
			}
			else
			{
				// We need to split off only as many charge units as can fit into this module
				// Split chargeRef
				uint32 quantityWeCanLoad = floor((modCapacity / chargeToLoadVolume).get_float());
				if ( quantityWeCanLoad > 0 )
				{
					// Split chargeRef to qty 'quantityWeCanLoad'
					// Merge new smaller qty 'quantityWeCanLoad' with loadedChargeRef
					// Load this merged charge Ref into module
					InventoryItemRef loadableChargeQtyRef = chargeRef->Split( quantityWeCanLoad );
					loadableChargeQtyRef->ChangeOwner( chargeRef->ownerID() );
					mod->LoadCharge( loadableChargeQtyRef );
					loadableChargeQtyRef->Move(m_Ship->itemID(), flag);		// used to be (m_pOperator->GetLocationID(), flag)
				}
				else
		            throw PyException( MakeCustomError( "Cannot load even one unit of this charge!" ) );
			}
		}
    }
    return;
}

void ModuleManager::UnloadCharge(EVEItemFlags flag)
{
    GenericModule* mod = m_Modules->GetModule(flag);
    if (mod and mod->IsLoaded() ) {
        _log(SHIP__MODULE_TRACE, "ModuleManager::UnloadCharge() - %s unloading %s",
             mod->getItem()->itemName().c_str(), mod->GetLoadedChargeRef()->itemName().c_str());
        mod->UnloadCharge();
	}
}
void ModuleManager::GetLoadedCharges(std::map< EVEItemFlags, InventoryItemRef >& charges)
{
    charges = m_charges;
}

InventoryItemRef ModuleManager::GetLoadedChargeOnModule(EVEItemFlags flag) {
    GenericModule* mod = m_Modules->GetModule(flag);
    if (mod and mod->IsLoaded() )
    	return mod->GetLoadedChargeRef();
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

/* this will be used to update modules for ..... (state reason for calling)
 *   will be used to call effects updates on modules
 */
void ModuleManager::UpdateModules()
{
    /** @todo  figure out what needs to be done here and implement it. */
    // this one is called from board,
    //  ALL modules need skillcheck, online check, cpu/pg check, etc.  run everthing on these and make calls as required.
    //  this should also update all ship attribs.
    //sLog.Magenta("ModuleManager::UpdateModules()","Needs to be implemented");

    if (!m_initalized)
        Initialize();
}

/* this will be used to update modules for ..... (state reason for calling)
 *   will be used to call effects updates on modules
 */
void ModuleManager::UpdateModules(EVEItemFlags flag)
{
    /** @todo  figure out what needs to be done here and implement it. */
    //  this should update all ship attribs for this bank.
    //sLog.Magenta("ModuleManager::UpdateModules()","Needs to be implemented");
}

/* these are used to call module effects for states 0 and 1 for initial application of effect data */
void ModuleManager::CharacterBoardingShip()
{
    if (!m_initalized)
        Initialize();
    if (m_Ship->GetPilot()->IsInSpace())
        OnlineAll();
}

void ModuleManager::CharacterLeavingShip()
{
    sLog.Magenta("ModuleManager::CharacterLeavingShip()","Needs to be implemented");
    //this is complicated and im gonna leave it alone for now until
    //a few things become more clear

    //OfflineAll();
}

void ModuleManager::ShipWarping()
{
    sLog.Magenta("ModuleManager::ShipWarping()","Deactivating all modules.");
    /** @todo  figure out how to check modules for warpsafe-ness and Deactivate accordingly
     *  there is an attribute for it (AttrDisallowActivateOnWarp), so we could test for that and adjust as needed started...(mod->IsWarpSafe())
     * for now, abort all modules.  yes, this is harsh, but will have to fix later.
     */
    AbortCycle();
}

void ModuleManager::ShipJumping()
{
    sLog.Magenta("ModuleManager::ShipJumping()","Deactivating all modules.");
    /** @todo figure out what needs to be done here and implement it
     * same as warping...check attribute and deactivate module.  this should be ALL modules
     */
    AbortCycle();
    //DeactivateAllModules();
}

void ModuleManager::GetModuleListOfRefs(std::vector<InventoryItemRef> * pModuleList)
{
	m_Modules->GetModuleListOfRefs(pModuleList);
}

void ModuleManager::StripModules()
{
    m_Modules->StripModules();
}

void ModuleManager::SaveModules()
{
    m_Modules->SaveModules();
}

void ModuleManager::GetModuleListByReqSkill(uint16 skillID, std::vector< InventoryItemRef >* pModuleList)
{
    std::vector<InventoryItemRef> moduleList;
    GetModuleListOfRefs(&moduleList);
    for (auto cur : moduleList) {
        if (cur->HasReqSkill(skillID)) {
            pModuleList->push_back(cur);
        }
    }
}

#pragma endregion
//////////////////////////////////////////////////////////////////////////////////
