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

    ClearModMap();
}

ModuleContainer::~ModuleContainer()
{
    for (std::map<uint8, GenericModule*>::iterator itr = m_modules.begin(); itr != m_modules.end(); itr++)
        SafeDelete(itr->second);
}

void ModuleContainer::ClearModMap() {
    // this will populate the module map for all slots with null pointer
    // modules
    for (uint8 flag = flagLowSlot0; flag < flagFixedSlot; flag++)
        m_modules.insert(std::pair<uint8, GenericModule*>(flag, nullptr));
    // rigs
    for (uint8 flag = flagRigSlot0; flag < flagRigSlot3; flag++)
        m_modules.insert(std::pair<uint8, GenericModule*>(flag, nullptr));
    //subsystems
    for (uint8 flag = flagSubSystem0; flag < flagSubSystem5; flag++)
        m_modules.insert(std::pair<uint8, GenericModule*>(flag, nullptr));
}

bool ModuleContainer::AddModule(EVEItemFlags flag, GenericModule* pMod)
{
    std::map<uint8, GenericModule*>::iterator itr = m_modules.find((uint8)flag);
    if (itr == m_modules.end())
        return false;
    else
        itr->second = pMod;
    _log(SHIP__MODULE_TRACE, "AddModule() - adding %s.", pMod->getItem()->itemName().c_str());

    // Maintain the Modules Fitted By Group counter for this module group:
    if ( m_ModulesFittedByGroupID.find(pMod->getItem()->groupID()) != m_ModulesFittedByGroupID.end() )
        m_ModulesFittedByGroupID.find(pMod->getItem()->groupID())->second++;
    else
        m_ModulesFittedByGroupID.insert(std::pair<uint32,uint32>(pMod->getItem()->groupID(), 1));

    // module is fit so change state from MOD_UNFITTED to MOD_OFFLINE now.
    pMod->SetModuleState(ModStates::MOD_OFFLINE);
	return true;
}

bool ModuleContainer::RemoveModule(EVEItemFlags flag) {
    GenericModule* pMod = GetModule(flag);
    if (!pMod)
        return false;

    pMod->ProcessEffects(Effects::dgmStatePassive, false);

    deleteModuleRef(pMod->flag(), pMod);
	return true;
}

bool ModuleContainer::RemoveModule(uint32 itemID) {
    GenericModule* pMod = GetModule(itemID);
    if (!pMod)
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
            std::map<uint8, GenericModule*>::reverse_iterator itr = m_modules.rbegin();
            while (itr != m_modules.rend()) {
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
            std::map<uint8, GenericModule*>::reverse_iterator itr = m_modules.rbegin();
            while (itr != m_modules.rend()) {
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
        case EVEEffectID::loPower: {
            for (uint8 slot=flagLowSlot0; slot < (flagLowSlot0 + m_LowSlots); slot++)
				if ( m_modules[slot] == nullptr )
					return slot;
            } break;
		case EVEEffectID::medPower: {
            for (uint8 slot=flagMedSlot0; slot < (flagMedSlot0 + m_MediumSlots); slot++)
                if ( m_modules[slot] == nullptr )
					return slot;
            } break;
		case EVEEffectID::hiPower: {
            for (uint8 slot=flagHiSlot0; slot < (flagHiSlot0 + m_HighSlots); slot++)
                if ( m_modules[slot] == nullptr )
					return slot;
            } break;
		case EVEEffectID::rigSlot: {
            for (uint8 slot=flagRigSlot0; slot < (flagRigSlot0 + m_RigSlots); slot++)
                if ( m_modules[slot] == nullptr )
					return slot;
            } break;
		case EVEEffectID::subSystem: {
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

uint8 ModuleContainer::GetFittedModuleCountByGroup(uint16 groupID)
{
    if ( m_ModulesFittedByGroupID.find(groupID) == m_ModulesFittedByGroupID.end() )
        return 0;
    else
        return m_ModulesFittedByGroupID.find(groupID)->second;
}

void ModuleContainer::GetModuleListOfRefsAsc(std::vector<InventoryItemRef> * pModuleList)
{
    std::map<uint8, GenericModule*>::iterator itr = m_modules.begin();
    while (itr != m_modules.end()) {
        if (itr->second)
            pModuleList->push_back( itr->second->getItem() );
        ++itr;
    }
}

void ModuleContainer::GetModuleListOfRefsDec(std::vector< InventoryItemRef >* pModuleList)
{
    std::map<uint8, GenericModule*>::reverse_iterator itr = m_modules.rbegin();
    while (itr != m_modules.rend()) {
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

void ModuleContainer::deleteModuleRef(EVEItemFlags flag, GenericModule* pMod)
{
    std::map<uint8, GenericModule*>::iterator itr = m_modules.find((uint8)flag);
    if (itr != m_modules.end())
        itr->second = nullptr;
        // make error for module not found in map?  should never happen.

    // Maintain the Modules Fitted By Group counter for this module group:
    if (m_ModulesFittedByGroupID.find(pMod->getItem()->groupID()) != m_ModulesFittedByGroupID.end()) {
        if (m_ModulesFittedByGroupID.find(pMod->getItem()->groupID())->second > 1) {
            // We still have more than one module of this group fitted, so just reduce number fitted by 1:
            --(m_ModulesFittedByGroupID.find(pMod->getItem()->groupID())->second);
        } else {
            // This was the last module of this group fitted, so remove the entry from the map:
            m_ModulesFittedByGroupID.erase(pMod->getItem()->groupID());
        }
    } else
        sLog.Error( "ModuleContainer::deleteModuleRef()", "Removing Module from ship fit when it had NO entry in m_ModulesFittedByGroup !" );

    pMod->SetModuleState(ModStates::MOD_UNFITTED);
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
    if (m_initalized)
        return true;

    // Load modules, charges, rigs and subsystems into ship's ModuleContainer:
    std::vector<InventoryItemRef> itemVec;
    m_Ship->GetMyInventory()->GetInventoryVec(itemVec);   // this method also sorts in order - cargo, modules, charge, subsystems.

    GenericModule* pMod(nullptr);
    // first we have to fit and online modules
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
                        pMod->SetChargeState(ModStates::CHG_LOADED);
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
        codelog(SHIP__MODULE_TRACE, "ModuleManager","%s has no upgrade slots left.", m_Ship->itemName().c_str());
        return false;
    }
    if (((item->groupID() >= EVEDB::invGroups::Rig_Armor) and (item->groupID() <= EVEDB::invGroups::Rig_Astronautic))
        or (item->groupID() == EVEDB::invGroups::Rig_Electronics_Superiority)) {
        fitModule(item,flag);
        m_Ship->SetAttribute(AttrUpgradeSlotsLeft, --slots);
        return true;
    } else
        codelog(SHIP__MODULE_TRACE, "ModuleManager","%s tried to fit item %u, which is not a rig", m_Ship->GetPilot()->GetName(), item->itemID());

    return false;
}

void ModuleManager::UninstallRig(uint32 itemID)
{
    GenericModule* pMod = m_Modules->GetModule(itemID);
    if (pMod != nullptr) {
        pMod->Offline();
        if (!sConfig.server.IsTestServer)
            pMod->DestroyRig();
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
    GenericModule* pMod = m_Modules->GetModule(itemID);
    if (pMod != nullptr) {
        EVEItemFlags flag = flagHangar;
        bool inSpace = IsSolarSystem(m_Ship->locationID());
        if (inSpace)
            flag = flagCargoHold;
        pMod->AbortCycle();
        pMod->Offline();
        if (pMod->IsLoaded()) {
            pMod->GetLoadedChargeRef()->Move((inSpace ? m_Ship->itemID() : m_Ship->locationID()), flag);
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
	if (m_Modules->GetModule(item->itemID())) {
		if (m_Modules->isSlotOccupied(flag))
            //if (mySE->HasPilot() and mySE->GetPilot()->CanThrow())
			//throw PyException( MakeUserError("SlotAlreadyOccupied"));
        /** @todo change this to use movemodule */
		return false;
	} else {
        // create new module object
		GenericModule* pMod = ModuleFactory(item, ShipItemRef(m_Ship));
        if (pMod == nullptr)
            return false;
        if (pMod->isMaxGroupFitLimited()) {
            if (m_Modules->GetFittedModuleCountByGroup(item->groupID()) == pMod->getItem()->GetAttribute(AttrMaxGroupFitted).get_int()) {
                SafeDelete(pMod);
                //if (mySE->HasPilot() and mySE->GetPilot()->CanThrow())
                //throw PyException( MakeUserError("CantFitTooManyByGroup"));
                return false;
            }
        }
        if (pMod->isTurretFitted()) {
            if (m_Ship->GetAttribute(AttrTurretSlotsLeft).get_bool()) {
                m_Ship->SetAttribute(AttrTurretSlotsLeft, (m_Ship->GetAttribute(AttrTurretSlotsLeft) -1));
            } else {
                SafeDelete(pMod);
                //if (mySE->HasPilot() and mySE->GetPilot()->CanThrow())
                //throw PyException( MakeUserError("NotEnoughTurretSlots"));    // this takes 2 args, u/k and module's typeID
                return false;
            }
		} else if (pMod->isLauncherFitted()) {
            if (m_Ship->GetAttribute(AttrLauncherSlotsLeft).get_bool()) {
                m_Ship->SetAttribute(AttrLauncherSlotsLeft, (m_Ship->GetAttribute(AttrLauncherSlotsLeft) -1));
            } else {
                SafeDelete(pMod);
                //if (mySE->HasPilot() and mySE->GetPilot()->CanThrow())
                //throw PyException( MakeUserError("NotEnoughLauncherSlots"));    // this takes 2 args, u/k and module's typeID
                return false;
            }
		}

        return m_Modules->AddModule(flag, pMod);
    }
}

bool ModuleManager::OnlineCheck(GenericModule* pMod)
{
    if (pMod->isRig() or pMod->isSubSystem())
        return true;
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
            _log(SHIP__MODULE_TRACE, "ModuleManager::Online(itemID) -  %s already Online", pMod->getItem()->itemName().c_str());
            return;
        }
        if (OnlineCheck(pMod)) {
            _log(SHIP__MODULE_TRACE, "ModuleManager::Online(itemID) -  %s going Online", pMod->getItem()->itemName().c_str());
            pMod->Online();
        } else {
            _log(SHIP__MODULE_TRACE, "ModuleManager::Online(itemID) -  Not enough CPU/PG to put %s online.", pMod->getItem()->itemName().c_str());
           /* if (m_Ship->HasPilot())
                m_Ship->GetPilot()->SendErrorMsg("Your ship does not have the required resources to online the %s", mod->getItem()->itemName().c_str());
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
            _log(SHIP__MODULE_TRACE, "ModuleManager::Online(flag) -  %s already Online", pMod->getItem()->itemName().c_str());
            return;
        }
        if (OnlineCheck(pMod)) {
            _log(SHIP__MODULE_TRACE, "ModuleManager::Online(flag) -  %s going Online", pMod->getItem()->itemName().c_str());
            pMod->Online();
        } else {
            _log(SHIP__MODULE_TRACE, "ModuleManager::Online(flag) -  Not enough CPU/PG to put %s online.", pMod->getItem()->itemName().c_str());
           /* if (m_Ship->HasPilot())
                m_Ship->GetPilot()->SendErrorMsg("Your ship does not have the required resources to online the %s", mod->getItem()->itemName().c_str());
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
            _log(SHIP__MODULE_TRACE, "ModuleManager::Offline(itemID) -  %s not Online", pMod->getItem()->itemName().c_str());
            pMod->SetModuleState(ModStates::MOD_OFFLINE);
            return;
        }
        _log(SHIP__MODULE_TRACE, "ModuleManager::Offline(itemID) -  %s going Offline", pMod->getItem()->itemName().c_str());
        pMod->Offline();
    } else
        _log(SHIP__MODULE_ERROR, "ModuleManager::Offline(itemID) -  Module %u not found", itemID);
}

void ModuleManager::Offline(EVEItemFlags flag)
{
    GenericModule* pMod = m_Modules->GetModule(flag);
    if (pMod != nullptr) {
        if (!pMod->isOnline()) {
            _log(SHIP__MODULE_TRACE, "ModuleManager::Offline(flag) -  %s not Online", pMod->getItem()->itemName().c_str());
            pMod->SetModuleState(ModStates::MOD_OFFLINE);
            return;
        }
        _log(SHIP__MODULE_TRACE, "ModuleManager::Offline(flag) -  %s going Offline", pMod->getItem()->itemName().c_str());
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
    GenericModule* pMod = m_Modules->GetModule(itemID);
    if (pMod == nullptr) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::Activate() - Called on module %u that is not loaded.", itemID );
        return;
    } else if (!pMod->isOnline()) {
        if (effectID == 16) { //16    online
            pMod->Online();
        } else {
            // client wont allow activating an offline module.  this is catchall. (but should never hit)
            m_Ship->GetPilot()->SendErrorMsg("You cannot activate an offline module. Ref: ServerError 25164");
        }
        return;
    } else {
        _log(SHIP__MODULE_TRACE, "ModuleManager::Activate() - %s (%s)  targetID: %i, repeat: %i.", \
                pMod->getItem()->itemName().c_str(), sFxDataMgr.GetEffectName(effectID).c_str(), targetID, repeat);
        pMod->Activate(effectID, targetID, repeat);
    }
}

void ModuleManager::Deactivate(uint32 itemID, std::string effectName)
{
    GenericModule* pMod = m_Modules->GetModule(itemID);
    if (pMod != nullptr) {
        if (pMod->GetModuleState() != ModStates::MOD_ACTIVATED)  // we dont need an error msgs here....this is acceptable, as the module may not be active
            return;
        _log(SHIP__MODULE_TRACE, "ModuleManager::Deactivate() - %s Deactivating - '%s'", pMod->getItem()->itemName().c_str(), effectName.c_str());
        pMod->Deactivate();
    } else
        _log(SHIP__MODULE_ERROR, "ModuleManager::Deactivate() - Called on module %u that is not loaded.", itemID );
}

void ModuleManager::Overload(EVEItemFlags flag)
{
    GenericModule* pMod = m_Modules->GetModule(flag);
    if (pMod != nullptr) {
        pMod->Overload();
        _log(SHIP__MODULE_TRACE, "ModuleManager::Overload() - %s Overloading...", pMod->getItem()->itemName().c_str());
    } else
        _log(SHIP__MODULE_ERROR, "ModuleManager::Overload() - Called on module that is not loaded at slot %i.", (int8)flag );
}

void ModuleManager::DeOverload(EVEItemFlags flag)
{
    GenericModule* pMod = m_Modules->GetModule(flag);
    if (pMod != nullptr) {
        pMod->DeOverload();
        _log(SHIP__MODULE_TRACE, "ModuleManager::DeOverload() - %s DeOverload...", pMod->getItem()->itemName().c_str());
    } else
        _log(SHIP__MODULE_ERROR, "ModuleManager::DeOverload() - Called on module that is not loaded at slot %i.", (int8)flag);
}

void ModuleManager::DamageModule(uint32 itemID, EvilNumber val)
{
    GenericModule* pMod = m_Modules->GetModule(itemID);
    if (pMod != nullptr) {
        pMod->SetAttribute(AttrHP, val);
        _log(SHIP__MODULE_TRACE, "ModuleManager::DamageModule() - %s taking %f damage.", pMod->getItem()->itemName().c_str(), val.get_float());
    } else
        _log(SHIP__MODULE_ERROR, "ModuleManager::DamageModule() - Called on module %u that is not loaded.", itemID );
}

void ModuleManager::RepairModule(uint32 itemID)
{
    GenericModule* pMod = m_Modules->GetModule(itemID);
    if (pMod != nullptr)
        pMod->Repair();
    else
        _log(SHIP__MODULE_ERROR, "ModuleManager::RepairModule() - Called on module %u that is not loaded.", itemID );
}

void ModuleManager::LoadCharge(InventoryItemRef chargeRef, EVEItemFlags flag)
{
    GenericModule* pMod = m_Modules->GetModule(flag);
    if (pMod != nullptr) {
        float modCapacity = pMod->getItem()->GetAttribute(AttrCapacity).get_float();
        float chargeVolume = chargeRef->GetAttribute(AttrVolume).get_float();

        if (pMod->IsLoaded()) {
            InventoryItemRef loadedChargeRef = pMod->GetLoadedChargeRef();
            modCapacity -= (loadedChargeRef->GetAttribute(AttrVolume).get_float() * loadedChargeRef->quantity());
            if ( chargeRef->typeID() != loadedChargeRef->typeID() ) {
                // change charges
                UnloadCharge(flag);
                if (IsStation(m_Ship->locationID()))
                    loadedChargeRef->Move(m_Ship->locationID(), flagHangar);
                else {
                    if (m_Ship->ValidateAddItem(flagCargoHold, loadedChargeRef))
                        loadedChargeRef->Move(m_Ship->itemID(), flagCargoHold);
                    else
                        return; // cant put in cargo.  return without changing charge.
                }
            } else {
                if (modCapacity > chargeVolume) {
                    uint32 quantityWeCanLoad = floor(modCapacity / chargeVolume);
                    if (quantityWeCanLoad > 0) {
                        if (quantityWeCanLoad < chargeRef->quantity()) {
                            InventoryItemRef loadableChargeQtyRef = chargeRef->Split(quantityWeCanLoad);
                            loadedChargeRef->Merge(loadableChargeQtyRef);
                        } else {
                            loadedChargeRef->Merge(chargeRef);
                        }
                        pMod->LoadCharge(loadedChargeRef);
                    }
                }
            }
        }

        modCapacity = pMod->GetAttribute(AttrCapacity).get_float();
        if (!(pMod->IsLoaded())) {
            if (modCapacity >= (chargeVolume * chargeRef->quantity())) {
                chargeRef->Move(m_Ship->itemID(), flag);
                pMod->LoadCharge( chargeRef );
            } else {
                uint32 quantityWeCanLoad = floor((modCapacity / chargeVolume));
                if (quantityWeCanLoad > 0) {
                    InventoryItemRef loadableChargeQtyRef = chargeRef->Split( quantityWeCanLoad );
                    loadableChargeQtyRef->ChangeOwner( chargeRef->ownerID() );
                    loadableChargeQtyRef->Move(m_Ship->itemID(), flag);
                    pMod->LoadCharge( loadableChargeQtyRef );
                }
            }
        } else {
            _log(SHIP__MODULE_ERROR, "ModuleManager::LoadCharge() - module %s at slot %i is still loaded", pMod->getItem()->itemName().c_str(), flag);
        }
    } else {
        _log(SHIP__MODULE_ERROR, "ModuleManager::LoadCharge() - module not found at slot %i", flag);
    }
}

void ModuleManager::UnloadCharge(EVEItemFlags flag)
{
    GenericModule* pMod = m_Modules->GetModule(flag);
    if (pMod != nullptr) {
        if (pMod->IsLoaded() ) {
            _log(SHIP__MODULE_TRACE, "ModuleManager::UnloadCharge() - %s unloading %s",
                    pMod->getItem()->itemName().c_str(), pMod->GetLoadedChargeRef()->itemName().c_str());
            pMod->UnloadCharge();
        } else
            _log(SHIP__MODULE_ERROR, "ModuleManager::UnloadCharge() - module %s at slot %i is not loaded", pMod->getItem()->itemName().c_str(), flag);
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
    //OfflineAll();
    GenericModule* pMod(nullptr);
    m_Ship->SetAttribute(AttrCpuLoad,     0);
    m_Ship->SetAttribute(AttrPowerLoad,   0);
    //m_Ship->SetAttribute(AttrUpgradeLoad, 0);
    if (!modVec.empty()) {
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
        }
    } else {
        OnlineAll();
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
    OfflineAll();
}

void ModuleManager::ShipWarping()
{
    sLog.Magenta("ModuleManager::ShipWarping()","Deactivating all modules.");
    /** @todo  figure out how to check modules for warpsafe-ness and Deactivate accordingly....done.  use sFxDataMgr.isWarpSafe(effectID)
     *
     * the "correct" way here is to test all currently active effects for "Effect.isWarpSafe" boolean, and deactivate those that dont have it.
     * for now, abort all modules.  yes, this is harsh, but will have to fix later.
     */
    /*  for_each(active_effect)
            if (sFxDataMgr.isWarpSafe(effectID) == false)
                deactivate(effect);
     */
    AbortCycle();
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
    for (uint8 i = flagRigSlot0; i < flagRigSlot3; i++) {
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
    for (uint8 i = flagSubSystem0; i < flagSubSystem5; i++) {
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
    for (auto cur : moduleList) {
        if (cur->HasReqSkill(skillID)) {
            pModuleList->push_back(cur);
        }
    }
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
    for (; itr != tmpList.rend(); itr++)
        pModList.push_back(itr->second);

}

#pragma endregion
//////////////////////////////////////////////////////////////////////////////////
