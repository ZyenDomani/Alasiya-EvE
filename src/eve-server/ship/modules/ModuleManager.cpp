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
#include "ship/DestinyManager.h"
#include "ship/Ship.h"
#include "ship/modules/ModuleManager.h"
#include "ship/modules/ModuleFactory.h"
#include "ship/modules/ActiveModule.h"
#include "system/SystemBubble.h"
#include "system/SystemManager.h"


//////////////////////////////////////////////////////////////////////////////////
// Modifier class definitions
#pragma region Modifier

// No functions defined here at this time.

#pragma endregion
/////////////////////////////// END MODIFIER /////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
// ModuleContainer class definitions
#pragma region ModuleContainerClass
ModuleContainer::ModuleContainer(uint32 lowSlots, uint32 medSlots, uint32 highSlots, uint32 rigSlots, uint32 subSystemSlots,
    uint32 turretSlots, uint32 launcherSlots, ModuleManager * myManager)
{
    m_LowSlots = lowSlots;
    m_MediumSlots = medSlots;
    m_HighSlots = highSlots;
    m_RigSlots = rigSlots;
    m_SubSystemSlots = subSystemSlots;
    m_TurretSlots = turretSlots;
    m_LauncherSlots = launcherSlots;

    _initializeModuleContainers();

    m_TotalTurretsFitted = 0;
    m_TotalLaunchersFitted = 0;

    m_MyManager = myManager;
}

ModuleContainer::~ModuleContainer()
{
    //clean up arrays of module pointers
    delete[] m_LowSlotModules;
    delete[] m_MediumSlotModules;
    delete[] m_HighSlotModules;
    delete[] m_RigModules;
    delete[] m_SubSystemModules;

    //nullify pointers
    SafeDelete(m_LowSlotModules);
    SafeDelete(m_MediumSlotModules);
    SafeDelete(m_HighSlotModules);
    SafeDelete(m_RigModules);
    SafeDelete(m_SubSystemModules);
}

void ModuleContainer::_initializeModuleContainers() {
    /** @todo  change this to use vector instead of pointer arrays .. mem mgmt and cleanup update*/
    //initialize our arrays of pointers for avalible ship slots.
    //  this solves a segfault when iterating (and the cooresponding dereference) past avalible ship slots
    m_HighSlotModules = new GenericModule*[m_HighSlots];
    m_MediumSlotModules = new GenericModule*[m_MediumSlots];
    m_LowSlotModules = new GenericModule*[m_LowSlots];
    m_RigModules = new GenericModule*[m_RigSlots];
    m_SubSystemModules = new GenericModule*[m_SubSystemSlots];

    uint8 i = 0;
    for (i = 0; i < m_HighSlots; ++i)
        m_HighSlotModules[i] = nullptr;

    for (i = 0; i < m_MediumSlots; ++i)
        m_MediumSlotModules[i] = nullptr;

    for (i = 0; i < m_LowSlots; ++i)
        m_LowSlotModules[i] = nullptr;

    for (i = 0; i < m_RigSlots; ++i)
        m_RigModules[i] = nullptr;

    for (i = 0; i < m_SubSystemSlots; ++i)
        m_SubSystemModules[i] = nullptr;

    for (uint8 flag = 11; flag < 35; ++flag) {
        m_modules.insert(std::pair<uint8, GenericModule*>(flag, nullptr));
    }

}
bool ModuleContainer::AddModule(EVEItemFlags flag, GenericModule* mod)
{
    std::map<uint8, GenericModule*>::iterator itr = m_modules.find(flag);
    if (itr != m_modules.end())
        itr->second = mod;

    switch(_checkBounds(flag))
    {
        case NaT:
            sLog.Error("ModuleContainer::AddModule()","Flag Out of bounds");
			return false;
        case slotTypeSubSystem:
			if (!m_SubSystemModules[flag - flagSubSystem0])
				m_SubSystemModules[flag - flagSubSystem0] = mod;
			else
				return false;
            break;
        case slotTypeRig:
			if (!m_RigModules[flag - flagRigSlot0])
	            m_RigModules[flag - flagRigSlot0] = mod;
			else
				return false;
            break;
        case slotTypeLowPower:
			if (!m_LowSlotModules[flag - flagLowSlot0])
	            m_LowSlotModules[flag - flagLowSlot0] = mod;
			else
				return false;
            break;
        case slotTypeMedPower:
			if (!m_MediumSlotModules[flag - flagMedSlot0])
	            m_MediumSlotModules[flag - flagMedSlot0] = mod;
			else
				return false;
            break;
        case slotTypeHiPower:
			if (!m_HighSlotModules[flag - flagHiSlot0])
	            m_HighSlotModules[flag - flagHiSlot0] = mod;
			else
				return false;
            break;
    }

    // Maintain Turret and Launcher Fitted module counts:
    if ( mod->isTurretFitted() )
        ++m_TotalTurretsFitted;
    if ( mod->isLauncherFitted() )
        ++m_TotalLaunchersFitted;

    // Maintain the Modules Fitted By Group counter for this module group:
    if ( m_ModulesFittedByGroupID.find(mod->getItem()->groupID()) != m_ModulesFittedByGroupID.end() )
        m_ModulesFittedByGroupID.find(mod->getItem()->groupID())->second++;
    else
        m_ModulesFittedByGroupID.insert(std::pair<uint32,uint32>(mod->getItem()->groupID(), 1));

	return true;
}

bool ModuleContainer::RemoveModule(EVEItemFlags flag) {
    GenericModule* mod = GetModule(flag);
    if (!mod)
        return false;

    _deleteModuleRef(mod->flag(), mod);
    //SafeDelete(mod);
	return true;
}

bool ModuleContainer::RemoveModule(uint32 itemID) {
    GenericModule* mod = GetModule(itemID);
    if (!mod)
        return false;

    _deleteModuleRef(mod->flag(), mod);
    return true;
}

void ModuleContainer::StripModules()
{
    /* may not need this */
    uint8 i = 0;
    for (i = 0; i < m_HighSlots; ++i)
        m_HighSlotModules[i] = nullptr;

    for (i = 0; i < m_MediumSlots; ++i)
        m_MediumSlotModules[i] = nullptr;

    for (i = 0; i < m_LowSlots; ++i)
        m_LowSlotModules[i] = nullptr;

    for (i = 0; i < m_RigSlots; ++i)
        m_RigModules[i] = nullptr;

    for (i = 0; i < m_SubSystemSlots; ++i)
        m_SubSystemModules[i] = nullptr;

    for (uint8 flag = 11; flag < 35; ++flag) {
        m_modules.insert(std::pair<uint8, GenericModule*>(flag, nullptr));
    }
}

GenericModule* ModuleContainer::GetModule(EVEItemFlags flag)
{
    switch(_checkBounds(flag)) {
        case slotTypeLowPower:     return m_LowSlotModules[flag - flagLowSlot0];
        case slotTypeMedPower:     return m_MediumSlotModules[flag - flagMedSlot0];
        case slotTypeHiPower:      return m_HighSlotModules[flag - flagHiSlot0];
        case slotTypeSubSystem:    return m_SubSystemModules[flag - flagSubSystem0];
        case slotTypeRig:          return m_RigModules[flag - flagRigSlot0];
        case NaT:                  _log(SHIP__ERROR, "ModuleContainer::GetModule() - Flag Out of bounds");
    }

    return nullptr;
}

GenericModule* ModuleContainer::GetModule(uint32 itemID)
{
    //iterate through the list and see if we have it
    uint8 r = 0;
    for (r = 0; r < m_HighSlots; r++)
        if (m_HighSlotModules[r] and (m_HighSlotModules[r]->itemID() == itemID))
            return m_HighSlotModules[r];

    for (r = 0; r < m_MediumSlots; r++)
        if (m_MediumSlotModules[r] and (m_MediumSlotModules[r]->itemID() == itemID))
            return m_MediumSlotModules[r];

    for (r = 0; r < m_LowSlots; r++)
        if (m_LowSlotModules[r] and (m_LowSlotModules[r]->itemID() == itemID))
            return m_LowSlotModules[r];

    for (r = 0; r < m_SubSystemSlots; r++)
        if (m_SubSystemModules[r] and (m_SubSystemModules[r]->itemID() == itemID))
            return m_SubSystemModules[r];

    for (r = 0; r < m_RigSlots; r++)
        if (m_RigModules[r] and (m_RigModules[r]->itemID() == itemID))
            return m_RigModules[r];

    return nullptr;  //we don't
}


void ModuleContainer::AbortCycle() {
    _process(typeAbort);
}

void ModuleContainer::Process() {
    _process(typeProcessAll);
}

void ModuleContainer::OnlineAll() {
    _process(typeOnlineAll);
}

void ModuleContainer::OfflineAll() {
    _process(typeOfflineAll);
}

void ModuleContainer::DeactivateAll() {
    _process(typeDeactivateAll);
}

void ModuleContainer::UnloadAll() {
    _process(typeUnloadAll);
}

bool ModuleContainer::isSlotOccupied(EVEItemFlags flag) {
    std::map<uint8, GenericModule*>::iterator itr = m_modules.find((uint8)flag);
    return (itr == m_modules.end() ? false : true);
}

uint32 ModuleContainer::GetAvailableSlotInBank(EVEEffectID slotBank)
{
	uint32 slot = 0, slotFound = flagIllegal;
	switch (slotBank)
	{
        case effectLoPower:
			for ( slot=0; slot < m_LowSlots; slot++)
				if ( m_LowSlotModules[slot] == NULL )
				{
					slotFound = slot + flagLowSlot0;
					break;
				}
			break;
		case effectMedPower:
			for ( slot=0; slot < m_MediumSlots; slot++)
				if ( m_MediumSlotModules[slot] == NULL )
				{
					slotFound = slot + flagMedSlot0;
					break;
				}
			break;
		case effectHiPower:
			for ( slot=0; slot < m_HighSlots; slot++)
				if ( m_HighSlotModules[slot] == NULL )
				{
					slotFound = slot + flagHiSlot0;
					break;
				}
			break;
		case effectRigSlot:
			for ( slot=0; slot < m_RigSlots; slot++)
				if ( m_RigModules[slot] == NULL )
				{
					slotFound = slot + flagRigSlot0;
					break;
				}
			break;
		case effectSubSystem:
			for ( slot=0; slot < m_SubSystemSlots; slot++)
				if ( m_SubSystemModules[slot] == NULL )
				{
					slotFound = slot + flagSubSystem0;
					break;
				}
			break;
		default:
			// ERROR: This is not a module that fits in any of the slot banks
			break;
	}

	return slotFound;
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
    uint8 r;
    for (r = 0; r < m_HighSlots; r++) {
        if (m_HighSlotModules[r])
			pModuleList->push_back( m_HighSlotModules[r]->getItem() );
    }

    for (r = 0; r < m_MediumSlots; r++) {
        if (m_MediumSlotModules[r])
            pModuleList->push_back( m_MediumSlotModules[r]->getItem() );
    }

    for (r = 0; r < m_LowSlots; r++) {
        if (m_LowSlotModules[r])
            pModuleList->push_back( m_LowSlotModules[r]->getItem() );
    }

    for (r = 0; r < m_SubSystemSlots; r++) {
        if (m_SubSystemModules[r])
            pModuleList->push_back( m_SubSystemModules[r]->getItem() );
    }

    for (r = 0; r < m_RigSlots; r++) {
        if (m_RigModules[r])
            pModuleList->push_back( m_RigModules[r]->getItem() );
    }
}

void ModuleContainer::SaveModules()
{
    uint8 r;
    for (r = 0; r < m_HighSlots; r++) {
        if (m_HighSlotModules[r])
            m_HighSlotModules[r]->getItem()->SaveItem();
    }

    for (r = 0; r < m_MediumSlots; r++) {
        if (m_MediumSlotModules[r])
            m_MediumSlotModules[r]->getItem()->SaveItem();
    }

    for (r = 0; r < m_LowSlots; r++) {
        if (m_LowSlotModules[r])
            m_LowSlotModules[r]->getItem()->SaveItem();
    }

    for (r = 0; r < m_SubSystemSlots; r++) {
        if (m_SubSystemModules[r])
            m_SubSystemModules[r]->getItem()->SaveItem();
    }

    for (r = 0; r < m_RigSlots; r++) {
        if (m_RigModules[r])
            m_RigModules[r]->getItem()->SaveItem();
    }
}

void ModuleContainer::_deleteModuleRef(EVEItemFlags flag, GenericModule* mod)
{
    std::map<uint8, GenericModule*>::iterator itr = m_modules.find(flag);
    if (itr != m_modules.end())
        itr->second = nullptr;

    switch(_checkBounds(flag))
    {
    case NaT:
        sLog.Error("ModuleContainer::_deleteModuleRef()","Flag %u retuned NaT.", flag);
        break;
    case slotTypeSubSystem:
        m_SubSystemModules[flag-flagSubSystem0] = nullptr;
        break;
    case slotTypeRig:
        m_RigModules[flag-flagRigSlot0] = nullptr;
        break;
    case slotTypeLowPower:
        m_LowSlotModules[flag-flagLowSlot0] = nullptr;
        break;
    case slotTypeMedPower:
        m_MediumSlotModules[flag-flagMedSlot0] = nullptr;
        break;
    case slotTypeHiPower:
        m_HighSlotModules[flag-flagHiSlot0] = nullptr;
        break;
    default:
        sLog.Error("ModuleContainer::_deleteModuleRef()","Flag %u not handled.", flag);
        return;
    }

    // Maintain Turret and Launcher Fitted module counts:
    if ( mod->isTurretFitted() )
        --m_TotalTurretsFitted;
    if ( mod->isLauncherFitted() )
        --m_TotalLaunchersFitted;

    // Maintain the Modules Fitted By Group counter for this module group:
    if (m_ModulesFittedByGroupID.find(mod->getItem()->groupID()) != m_ModulesFittedByGroupID.end()) {
        if (m_ModulesFittedByGroupID.find(mod->getItem()->groupID())->second > 1) {
            // We still have more than one module of this group fitted, so just reduce number fitted by 1:
            m_ModulesFittedByGroupID.find(mod->getItem()->groupID())->second--;
        } else {
            // This was the last module of this group fitted, so remove the entry from the map:
            m_ModulesFittedByGroupID.erase(mod->getItem()->groupID());
        }
    } else
        sLog.Error( "ModuleContainer::_deleteModuleRef()", "Removing Module from ship fit when it had NO entry in m_ModulesFittedByGroup !" );
}

void ModuleContainer::_process(processType p)
{
    //high slots
    _processEx(p, highSlot);

    //med slots
    _processEx(p, mediumSlot);

    //low slots
    _processEx(p, lowSlot);
}

void ModuleContainer::_processEx(processType p, slotType t)
{
    uint8 r, COUNT;
    GenericModule** cur = nullptr;

    switch(t) {
    case highSlot:
        COUNT = m_HighSlots;
        cur = m_HighSlotModules;
        break;
    case mediumSlot:
        COUNT = m_MediumSlots;
        cur = m_MediumSlotModules;
        break;
    case lowSlot:
        COUNT = m_LowSlots;
        cur = m_LowSlotModules;
        break;
    case rigSlot:
        COUNT = m_RigSlots;
        cur = m_RigModules;
        break;
    case subSystemSlot:
        COUNT = m_SubSystemSlots;
        cur = m_SubSystemModules;
        break;
    }

    switch(p) {
    case typeOnlineAll:
        for (r = 0; r < COUNT; r++, cur++) {
            if (*cur == nullptr)
                continue;
            (*cur)->Online();
        } break;

    case typeOfflineAll:
        for (r = 0; r < COUNT; r++, cur++) {
            if (*cur == nullptr)
                continue;
            (*cur)->Offline();
        } break;

    case typeDeactivateAll:
        for (r = 0; r < COUNT; r++, cur++) {
            if (*cur == nullptr)
                continue;
            (*cur)->Deactivate();
        } break;

    case typeUnloadAll:
        for (r = 0; r < COUNT; r++, cur++) {
            if (*cur == nullptr)
                continue;
            (*cur)->Unload();
        } break;

    case typeProcessAll:
        for (r = 0; r < COUNT; r++, cur++) {
            if (*cur == nullptr)
                continue;
            (*cur)->Process();
        } break;

    case typeAbort:
        for (r = 0; r < COUNT; r++, cur++) {
            if (*cur == nullptr)
                continue;
            (*cur)->AbortCycle();
        } break;
    }
}

EVEItemSlotType ModuleContainer::_checkBounds(EVEItemFlags flag)
{
    //this could be done better         -not sure how yet.
    if ((flag >= flagLowSlot0) and (flag <= flagLowSlot7))
        return slotTypeLowPower;

    if ((flag >= flagMedSlot0) and (flag <= flagMedSlot7))
        return slotTypeMedPower;

    if ((flag >= flagHiSlot0) and (flag <= flagHiSlot7))
        return slotTypeHiPower;

    if ((flag >= flagRigSlot0) and (flag <= flagRigSlot7))
        return slotTypeRig;

    if ((flag >= flagSubSystem0) and (flag <= flagSubSystem7))
        return slotTypeSubSystem;

    return NaT;  //Not a Type
}

#pragma endregion
/////////////////////////// END MODULECONTAINER //////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
// ModuleManager class definitions
#pragma region ModuleManagerClass
ModuleManager::ModuleManager(ShipItem *const ship)
{
    // Create ModuleContainer object and initialize with sizes for avalible slot banks for this ship:
    m_Modules = new ModuleContainer((uint32)ship->GetAttribute(AttrLowSlots).get_int(),
                                    (uint32)ship->GetAttribute(AttrMedSlots).get_int(),
                                    (uint32)ship->GetAttribute(AttrHiSlots).get_int(),
                                    (uint32)ship->GetAttribute(AttrRigSlots).get_int(),
                                    (uint32)ship->GetAttribute(AttrSubSystemSlot).get_int(),
                                    (uint32)ship->GetAttribute(AttrTurretSlotsLeft).get_int(),
                                    (uint32)ship->GetAttribute(AttrLauncherSlotsLeft).get_int(),
                                    this);

    m_initalized = false;

    // Store reference to the Ship object to which the ModuleManager belongs:
    m_Ship = ship;

    //modifier maps, we own these
    m_LocalSubsystemModifierMaps = new ModifierMaps;
    m_LocalShipSkillModifierMaps = new ModifierMaps;
    m_LocalModuleRigModifierMaps = new ModifierMaps;
    m_LocalImplantModifierMaps = new ModifierMaps;
    m_RemoteModifierMaps = new ModifierMaps;
}

ModuleManager::~ModuleManager()
{
    //module cleanup is handled in the ModuleContainer destructor
    delete m_Modules;
    m_Modules = nullptr;

    //modifier map cleanup is handled in the std::map destructor
    delete m_LocalSubsystemModifierMaps;
    delete m_LocalShipSkillModifierMaps;
    delete m_LocalModuleRigModifierMaps;
    delete m_LocalImplantModifierMaps;
    delete m_RemoteModifierMaps;
    m_LocalSubsystemModifierMaps = nullptr;
    m_LocalShipSkillModifierMaps = nullptr;
    m_LocalModuleRigModifierMaps = nullptr;
    m_LocalImplantModifierMaps = nullptr;
    m_RemoteModifierMaps = nullptr;
}

bool ModuleManager::Initialize() {
    // this ship is already initalized and active (in case of BoardShip() or reactivation)
    if (m_initalized) {
        OnlineAll();
        return true;
    }

    // Load modules, rigs and subsystems from Ship's inventory into ModuleContainer:
    std::vector<InventoryItemRef> itemVec;
    m_Ship->GetInventory()->GetInventoryVec(itemVec);
    GenericModule* mod = nullptr;
    for (auto cur : itemVec) {
        if (cur->flag() == flagCargoHold) continue;
        if (cur->categoryID() == EVEDB::invCategories::Module) {
            mod = ModuleFactory(cur, ShipItemRef(m_Ship));
            if (m_Modules->AddModule(cur->flag(), mod)) {
                Online(cur->flag());
            } else {
                _log(SHIP__ERROR, "ModuleManager::Initialize() - Could not insert module %s(%u) at flag %u into module container.",
                     cur->itemName().c_str(), cur->itemID(), cur->flag() );
            }
            continue;
        } else if (cur->categoryID() == EVEDB::invCategories::Charge) {
            if (GetModule(cur->flag())) {
                GetModule(cur->flag())->Load(cur);
            } else {
                _log(SHIP__ERROR, "ModuleManager::Initialize() - Cannot find module to load %s(%u) at flag %u",
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
    m_initalized = true;
    return true;
}

bool ModuleManager::IsSlotOccupied(EVEItemFlags flag)
{
    if (m_Modules->GetModule(flag))
        return true;

    return false;
}

uint32 ModuleManager::GetAvailableSlotInBank(EVEEffectID slotBank)
{
	// Call into ModuleContainer class with slotBank effectID to have it check for and return any available slot flag in
	// in the specified slot bank:
	return m_Modules->GetAvailableSlotInBank(slotBank);
}

void ModuleManager::_SendInfoMessage(const char *fmt, ...)
{
    if (!m_Ship->GetPilot())     // Operator assumed to be Client *
        sLog.Error("SendMessage","message should have been sent to character, but *m_Client is null.  Did you forget to call GetShip()->SetOwner(Client *c)?");
    else
    {
        va_list args;
        va_start(args,fmt);
        m_Ship->GetPilot()->SendNotifyMsg(fmt,args);
        va_end(args);

    }
}

void ModuleManager::_SendErrorMessage(const char *fmt, ...)
{
    if (!m_Ship->GetPilot())     // Operator assumed to be Client *
        sLog.Error("SendMessage","message should have been sent to character, but *m_Client is null.  Did you forget to call GetShip()->SetOwner(Client *c)?");
    else
    {
        va_list args;
        va_start(args,fmt);
        m_Ship->GetPilot()->SendErrorMsg(fmt,args);
        va_end(args);
    }
}

bool ModuleManager::InstallRig(InventoryItemRef item, EVEItemFlags flag) {
    uint8 slots = m_Ship->GetAttribute(AttrUpgradeSlotsLeft).get_int();
    if (!slots) {
        /* send error to player?  or does client do it?  dunno...  */
        codelog(SHIP__MODULE_TRACE, "ModuleManager","%s tried to fit item %u, which is not a rig", m_Ship->GetPilot()->GetName(), item->itemID());
        return false;
    }
    if (((item->groupID() >= 773) and (item->groupID() <= 782)) or (item->groupID() == 786)) {
        _fitModule(item,flag);
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
        if (!sConfig.server.testServer)
            mod->DestroyRig();
    }
    m_Modules->RemoveModule(itemID);
    m_Ship->SetAttribute(AttrUpgradeSlotsLeft, m_Ship->GetAttribute(AttrUpgradeSlotsLeft) +1);
}

bool ModuleManager::InstallSubSystem(InventoryItemRef item, EVEItemFlags flag)
{
    if (item->categoryID() == EVEDB::invCategories::Subsystem) {
        _fitModule(item,flag);
        return true;
    } else
        sLog.Warning("ModuleManager","%s tried to fit item %u, which is not a subsystem", m_Ship->GetPilot()->GetName(), item->itemID());

    return false;
}

void ModuleManager::UnfitModule(uint32 itemID)
{
    GenericModule* mod = m_Modules->GetModule(itemID);
    m_Modules->RemoveModule(itemID);
    if (mod) {
        EVEItemFlags flag = flagCargoHold;
        bool inSpace = (IsStation(m_Ship->locationID()) ? false : true);
        if (inSpace)
            flag = flagHangar;
        if (mod->IsLoaded()) {
            mod->GetLoadedChargeRef()->Move((inSpace ? m_Ship->itemID() : m_Ship->locationID()), flag);
            mod->Unload();
        }
        if (mod->isOnline())
            mod->Offline();
        // dont actually move the module here....let the caller do that in it's specific code
        //mod->getItem()->Move((inSpace ? m_Ship->itemID() : m_Ship->locationID()), flag);
    }
}

bool ModuleManager::FitModule(InventoryItemRef item, EVEItemFlags flag)
{
    if (item->categoryID() == EVEDB::invCategories::Module) {
        // Attempt to fit the module
        if ( _fitModule(item, flag) ) {
            // Now that module is successfully fitted, attempt to put it Online:
            Online(item->itemID());
            return true;
        }
    } else
        sLog.Warning("ModuleManager","%s tried to fit item %u, which is not a module", m_Ship->GetPilot()->GetName(), item->itemID());

    return false;
}

bool ModuleManager::_fitModule(InventoryItemRef item, EVEItemFlags flag)
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
        if (OnlineCheck(mod)) {
            _log(SHIP__MODULE_TRACE, "ModuleManager::Online() -  %s going Online", mod->getItem()->itemName().c_str());
            mod->Online();
        } else
            _log(SHIP__MODULE_TRACE, "ModuleManager::Online() -  Not enough CPU/PG to put %s online.", mod->getItem()->itemName().c_str());
    } else
        _log(SHIP__MODULE_ERROR, "ModuleManager::Online() -  Module %u not found", itemID);
}

void ModuleManager::Online(EVEItemFlags flag)
{
    GenericModule* mod = m_Modules->GetModule(flag);
    if (mod) {
        if (mod->isOnline())
            return;
        if (OnlineCheck(mod)) {
            _log(SHIP__MODULE_TRACE, "ModuleManager::Online() -  %s going Online", mod->getItem()->itemName().c_str());
            mod->Online();
        } else
            _log(SHIP__MODULE_TRACE, "ModuleManager::Online() -  Not enough CPU/PG to put %s online.", mod->getItem()->itemName().c_str());
    } else
        _log(SHIP__MODULE_ERROR, "ModuleManager::Online() -  Module at location %u not found", flag);
}

void ModuleManager::Offline(uint32 itemID)
{
    GenericModule* mod = m_Modules->GetModule(itemID);
    if (mod) {
        if (!mod->isOnline())
            return;
        _log(SHIP__MODULE_TRACE, "ModuleManager::Offline() -  %s going Offline", mod->getItem()->itemName().c_str());
        mod->Offline();
    } else
        _log(SHIP__MODULE_ERROR, "ModuleManager::Offline() -  Module %u not found", itemID);
}

void ModuleManager::Offline(EVEItemFlags flag)
{
    GenericModule* mod = m_Modules->GetModule(flag);
    if (mod) {
        _log(SHIP__MODULE_TRACE, "ModuleManager::Offline() -  %s going Offline", mod->getItem()->itemName().c_str());
        mod->Offline();
    } else
        _log(SHIP__MODULE_ERROR, "ModuleManager::Online() -  Module at location %u not found", flag);
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

void ModuleManager::Activate(uint32 itemID, std::string effectName, uint32 targetID, int32 repeat)
{
    GenericModule* mod = m_Modules->GetModule(itemID);
    if (!mod) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::Activate() - Called on a module that is NOT loaded!" );
        return;
    } else {
        _log(SHIP__MODULE_TRACE, "ModuleManager::Activate() - %s (%s).", mod->getItem()->itemName().c_str(), effectName.c_str());
        /* once finished, this will take care of activating modules that affect their ship's attributes.
         * modules that do things other than affect attributes will still need their own class.
        mod->Activate(effectName);
        */
        mod->SetRepeat(repeat);
        bool targetNotNeeded = false;
        // these calls DO NOT need a target...
        if (effectName == "online") {
            mod->Online();
            targetNotNeeded =true;
        } else if (!mod->isOnline()) {
            m_Ship->GetPilot()->SendErrorMsg("Your %s is offline. You cannot activate an offline module.", mod->getItem()->itemName().c_str());
            return;
        } else if (effectName == "cloaking") {//FIXME  set this to use module code, drain cap, etc.
            if (m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->IsCloaked())
                m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->UnCloak();
            else
                m_Ship->GetPilot()->GetShipSE()->DestinyMgr()->Cloak();
            targetNotNeeded =true;
            /** @todo  not working right....check for attrib '10' being added and error msgs with ServerError 25610 */
        } else if (effectName == "speedBoostMassAddition") {    // AB
            mod->Activate(nullptr);
            targetNotNeeded =true;
        } else if (effectName == "speedBoostMassSigRad") {  //MicroWarpdrive
            mod->Activate(nullptr);
            targetNotNeeded =true;
        } else if (effectName == "damageControl") { //DCM
            mod->Activate(nullptr);
            targetNotNeeded =true;
        } else if (effectName == "armorRepair") {
            mod->Activate(nullptr);
            targetNotNeeded =true;
        } else if (effectName == "shieldBoosting") {
            mod->Activate(nullptr);
            targetNotNeeded =true;
        } else if (effectName == "structureRepair") {   //Hull Repairer
            mod->Activate(nullptr);
            targetNotNeeded =true;
        } else if (effectName == "modifyActiveShieldResonanceAndNullifyPassiveResonance") { //Shield Hardener
            mod->Activate(nullptr);
            targetNotNeeded =true;
        } else if (effectName == "modifyActiveArmorResonanceAndNullifyPassiveResonance") {  //Armor Hardener
            mod->Activate(nullptr);
            targetNotNeeded =true;
        } else if (effectName == "surveyScan") {
            mod->Activate(targetEntity);
            targetNotNeeded =true;
        }
        if (targetNotNeeded)
            return;
		if (!targetID) {
			sLog.Error("ModuleManager::Activate()", "targetID == 0");
            if (m_Ship->HasPilot())
                m_Ship->GetPilot()->SendErrorMsg("You must have a target to activate your %s.", mod->getItem()->itemName().c_str());
			return;
		}
		 SystemEntity* targetEntity = m_Ship->GetPilot()->GetShipSE()->SysBubble()->GetEntity(targetID);
        if (!targetEntity) {
            sLog.Error("ModuleManager::Activate()", "targetEntity == NULL");
            if (m_Ship->HasPilot())
                m_Ship->GetPilot()->SendErrorMsg("You must have a target to activate your %s.", mod->getItem()->itemName().c_str());
            return;
        }

        if (effectName == "tractorBeamCan") {
            mod->Activate(targetEntity);
        } else if (effectName == "miningLaser") {   // mining of all types...cloud, ore, strip, etc.
            mod->Activate(targetEntity);
        } else if (effectName == "miningClouds") {   // Gas Harvesters (12November15) - AlTahir
            mod->Activate(targetEntity);
        } else if (effectName == "targetAttack") {   // ship lasers.....all sizes, all types
            mod->Activate(targetEntity);
        } else if (effectName == "projectileFired") {   //ship projectile guns...all sizes, all types
            mod->Activate(targetEntity);
        } else if (effectName == "shieldTransfer"){		// Shield transporters. All sizes (12November15) - AlTahir
            mod->Activate(targetEntity);
        } else if (effectName == "energyTransfer"){		// Energy Transfer. All sizes (11.16.2015) - AlTahir
        	mod->Activate(targetEntity);
        } else if (effectName == "useMissiles") {   //implemented Missiles    18May15  -allan
            mod->Activate(targetEntity);
        } else if (effectName == "decreaseTargetSpeed") {   //Stasis Webifier	-crashing server -allan 20June15
            ;//mod->Activate(targetEntity);
        } else if (effectName == "salvaging") {
            mod->Activate(targetEntity);
        } else if (effectName == "warpScrambleTargetMWDBlockActivation") {  //Warp Scrambler
            mod->Activate(targetEntity);
        } else if (effectName == "targetArmorRepair") { //Remote Armor Repair System (AlTahir, 14.11.2015)
            mod->Activate(targetEntity);
        } else if (effectName == "remoteHullRepair") {  // Remote Hull Repair System (Altahir, 14.11.2015)
            mod->Activate(targetEntity);
    /** @todo these below are not wrote, not tested, in testing, or otherwise unknown.
        } else if (effectName == "superWeaponAmarr") {  //Judgement
            ; //mod->Activate(targetEntity);
        } else if (effectName == "superWeaponCaldari") {  //Oblivion
            ; //mod->Activate(targetEntity);
        } else if (effectName == "superWeaponMinmatar") {  //Gjallarhorn
            ; //mod->Activate(targetEntity);
        } else if (effectName == "superWeaponGallente") {  //Aurora Ominae
            ; //mod->Activate(targetEntity);
        } else if (effectName == "siegeModeEffect6") {  //Siege Module
            ; //mod->Activate(targetEntity);
        } else if (effectName == "empWave") {   //EMP Smartbomb
            ; //mod->Activate(targetEntity);
        } else if (effectName == "openSpawnContainer") {   //Analyzer I
            ; //mod->Activate(targetEntity);
        } else if (effectName == "ewTargetPaint") { //Target Painter
            ; //mod->Activate(targetEntity);
        } else if (effectName == "gangMiningLaserAndIceHarvesterAndGasCloudHarvesterMaxRangeBonus") {   //Mining Foreman Link - Mining Laser Field Enhancement
            ; //mod->Activate(targetEntity);
        } else if (effectName == "gangGasHarvesterAndIceHarvesterAndMiningLaserCapNeedBonus") { //Mining Foreman Link - Harvester Capacitor Efficiency
            ; //mod->Activate(targetEntity);
        } else if (effectName == "gangArmorRepairCapReducerSelfAndProjected") { //Armored Warfare Link - Damage Control
            ; //mod->Activate(targetEntity);
        } else if (effectName == "triageModeEffect3") { //Triage Module
            ; //mod->Activate(targetEntity);
        } else if (effectName == "ewTestEffectJam") {   //ECM - White Noise Generator
            ; //mod->Activate(targetEntity);
        } else if (effectName == "scanStrengthBonusPercentActivate") {  //ECCM - Gravimetric    ECCM - Magnetometric
            ; //mod->Activate(targetEntity);
        } else if (effectName == "remoteEcmBurst") {    //Remote ECM Burst
            ; //mod->Activate(targetEntity);
        } else if (effectName == "sensorBoosterActivePercentage") { //Sensor Booster "effects.ElectronicAttributeModifyActivate"
            ; //mod->Activate(targetEntity);
        } else if (effectName == "industrialCoreEffect2") {  //Industrial Core
            ; //mod->Activate(targetEntity);
        } else if (effectName == "cynosuralGeneration") {  //Cynosural Field Generator
            ; //mod->Activate(targetEntity);
        } else if (effectName == "gunneryMaxRangeFalloffTrackingSpeedBonus") {  //Tracking Computer "effects.TurretWeaponRangeTrackingSpeedMultiplyActivate"
            ; //mod->Activate(targetEntity);
            */
	} else {
            sLog.Warning("ModuleManager::Activate()", "Module '%s' effectName '%s' not found.",
                     mod->getItem()->itemName().c_str(), effectName.c_str());
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

void ModuleManager::DeactivateAllModules()
{
    m_Modules->DeactivateAll();
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
        _log(SHIP__MODULE_TRACE, "ModuleManager::Overload() - %s DeOverload...", mod->getItem()->itemName().c_str());
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
				mod->Unload();

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
							mod->Load( loadedChargeRef );
							loadedChargeRef->Move(m_Ship->itemID(), flag);		// used to be (m_pOperator->GetLocationID(), flag)
						}
						else
						{
							// Merge chargeRef with loadedChargeRef
							// Load this merged charge Ref into module
							loadedChargeRef->Merge( chargeRef );
							mod->Load( loadedChargeRef );
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
				mod->Load( chargeRef );
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
					mod->Load( loadableChargeQtyRef );
					loadableChargeQtyRef->Move(m_Ship->itemID(), flag);		// used to be (m_pOperator->GetLocationID(), flag)
				}
				else
		            throw PyException( MakeCustomError( "Cannot load even one unit of this charge!" ) );
			}
		}
    }
    return;
}

void ModuleManager::UnloadCharge(EVEItemFlags flag) {
    GenericModule* mod = m_Modules->GetModule(flag);
    if (mod and mod->IsLoaded() ) {
        _log(SHIP__MODULE_TRACE, "ModuleManager::UnloadCharge() - %s unloading %s",
             mod->getItem()->itemName().c_str(), mod->GetLoadedChargeRef()->itemName().c_str());
        mod->Unload();
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

void ModuleManager::UnloadAllModules() {
    m_Modules->UnloadAll();
}

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

void ModuleManager::UpdateModules(EVEItemFlags flag)
{
    /** @todo  figure out what needs to be done here and implement it. */
    //  this should update all ship attribs for this bank.
    //sLog.Magenta("ModuleManager::UpdateModules()","Needs to be implemented");
}

void ModuleManager::CharacterLeavingShip()
{
    sLog.Magenta("ModuleManager::CharacterLeavingShip()","Needs to be implemented");
    //this is complicated and im gonna leave it alone for now until
    //a few things become more clear

    OfflineAll();
}

void ModuleManager::CharacterBoardingShip()
{
    if (!m_initalized)
        Initialize();
    if (m_Ship->GetPilot()->IsInSpace())
        OnlineAll();
}

void ModuleManager::ShipWarping()
{
    sLog.Magenta("ModuleManager::ShipWarping()","Needs to be implemented");
    /** @todo  figure out how to check modules for warpsafe-ness and Deactivate accordingly
     *  there is an attribute for it (AttrDisallowActivateOnWarp), so we could test for that and adjust as needed
     */
}

void ModuleManager::ShipJumping()
{
    sLog.Magenta("ModuleManager::ShipJumping()","Needs to be implemented");
    /** @todo figure out what needs to be done here and implement it  */
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

void ModuleManager::ProcessExternalEffect(Effect* e)
{
    while (e->hasEffect())
        _processExternalEffect(e->next());
}

void ModuleManager::GetModuleListOfRefs(std::vector<InventoryItemRef> * pModuleList)
{
	m_Modules->GetModuleListOfRefs(pModuleList);
}

void ModuleManager::StripModules() {
    m_Modules->StripModules();
}

void ModuleManager::SaveModules()
{
    m_Modules->SaveModules();
}

int32 ModuleManager::ApplyRemoteEffect(uint32 attributeID, uint32 originatorID, SystemEntity * systemEntity, ModifierRef modifierRef)
{
    sLog.Magenta("ModuleManager::ApplyRemoteEffect()","Needs to be implemented");
    return 1;
}

int32 ModuleManager::RemoveRemoteEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    sLog.Magenta("ModuleManager::RemoveRemoteEffect()","Needs to be implemented");
    return 1;
}

int32 ModuleManager::ApplySubsystemEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    ModifierMap* modMap = nullptr;

    // Make sure the ModifierRef passed in is not NULL:
    if (!modifierRef.get())
        return -1;

    // Check to see if this attributeID does not have a ModifierMap in the Map of ModifierMaps
    if ( m_LocalSubsystemModifierMaps->find(attributeID) == m_LocalSubsystemModifierMaps->end() )
    {
        // A Modifier Map for this attributeID does not exist, create a new one:
        modMap = new ModifierMap();
        if (!modMap)
            return -1;
    }
    else
    {
        // A Modifier Map for this attributeID already exists, find it and get its pointer:
        modMap = m_LocalSubsystemModifierMaps->find(attributeID)->second;
        if (!modMap)
            return -1;
    }

    // Check to see if the modifier map has any entries corresponding to the passed-in modifier's value:
    if ( modMap->m_ModifierMap.find(modifierRef->GetModifierValue()) != modMap->m_ModifierMap.end() )
    {
        // Modifier entry in this attributeID's Modifier Map already exists (modifierRef->GetModifierValue() found a match),
        // so check its originatorID and if that matches, DO NOT add this Modifier object to the map as the reference
        // already exists, the Module class can modify the contents of the Modifier object without really calling this function,
        // however, to maintain consistent code, the Module classes will always call this function to notify the map class
        // that the contents of the map was changed, or made 'dirty':
        modMap->m_MapIsDirty = true;
        ModifierMapType::iterator cur;
        std::pair<ModifierMapType::iterator,ModifierMapType::iterator> range;
        range = modMap->m_ModifierMap.equal_range(modifierRef->GetModifierValue());   // Get the one or more modifier map entries matching this modifier being added
        for (cur=range.first; cur!=range.second; ++cur)
            if ( cur->second->GetOriginatorID() == originatorID )
                return 1;   // Yep, we found the Modifier owned by this originatorID, so we return "success" because the Module
                            // class object already updated this Modifier through its own ModifierRef, we don't need to do anything
                            // else here except return and prevent ADDING to the ModifierMap

        // For loop searching existing modifiers completed, so this originatorID's Modifier
        // is NOT in the map yet... Let's add it:
        modMap->m_ModifierMap.insert(std::pair<double, ModifierRef>(modifierRef->GetModifierValue(), modifierRef));
    }
    else
    {
        // Modifier entry in this attributeID's Modifier Map does not exist yet, so lets insert it for the first time:
        // Insert the (modifierValue, ModifierRef) pair into the Modifier Map for this attributeID:
        modMap->m_ModifierMap.insert(std::pair<double, ModifierRef>(modifierRef->GetModifierValue(), modifierRef));
        modMap->m_MapIsDirty = true;
        m_LocalSubsystemModifierMaps->insert(std::pair<uint32, ModifierMap *>(attributeID, modMap));
    }

    return 1;
}

int32 ModuleManager::RemoveSubsystemEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    bool bModifierFound = false;
    ModifierMap * modMap = nullptr;

    if ( m_LocalSubsystemModifierMaps->find(attributeID) != m_LocalSubsystemModifierMaps->end() )
    {
        modMap = m_LocalSubsystemModifierMaps->find(attributeID)->second;
        modMap->m_MapIsDirty = true;

        if ( modMap->m_ModifierMap.find(modifierRef->GetModifierValue()) != modMap->m_ModifierMap.end() )
        {
            modMap->m_MapIsDirty = true;
            ModifierMapType::iterator cur;
            std::pair<ModifierMapType::iterator,ModifierMapType::iterator> range;
            range = modMap->m_ModifierMap.equal_range(modifierRef->GetModifierValue());   // Get the one or more modifier map entries matching this modifier being removed
            for (cur=range.first; cur!=range.second; ++cur)
                if ( cur->second->GetOriginatorID() == originatorID )
                {
                    bModifierFound = true;  // Yep, we found the Modifier owned by this originatorID, so we break out of the for ()
                                            // so we can now remove this exact Modifier object from the multimap
                    break;
                }

            if ( bModifierFound == true )
            {
                // For loop searching existing modifiers completed, so this originatorID's Modifier
                // was found in the map
                modMap->m_ModifierMap.insert(std::pair<double, ModifierRef>(modifierRef->GetModifierValue(), modifierRef));
            }
            else
                return -1;  // This modifier's originatorID was not found in the map, so return error code
        }
        else
            return -1;  // This modifier's modifier value was not even found in the map, so return error code
    }
    else
        return -1;  // Modifier Map for supplied attributeID does not exist, return error value

    return 1;
}

int32 ModuleManager::ApplyShipSkillEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    ModifierMap * modMap = nullptr;

    // Make sure the ModifierRef passed in is not NULL:
    if (!modifierRef.get())
        return -1;

    // Check to see if this attributeID does not have a ModifierMap in the Map of ModifierMaps
    if ( m_LocalShipSkillModifierMaps->find(attributeID) == m_LocalShipSkillModifierMaps->end() )
    {
        // A Modifier Map for this attributeID does not exist, create a new one:
        modMap = new ModifierMap();
        if (!modMap)
            return -1;
    }
    else
    {
        // A Modifier Map for this attributeID already exists, find it and get its pointer:
        modMap = m_LocalShipSkillModifierMaps->find(attributeID)->second;
        if (!modMap)
            return -1;
    }

    // Check to see if the modifier map has any entries corresponding to the passed-in modifier's value:
    if ( modMap->m_ModifierMap.find(modifierRef->GetModifierValue()) != modMap->m_ModifierMap.end() )
    {
        // Modifier entry in this attributeID's Modifier Map already exists (modifierRef->GetModifierValue() found a match),
        // so check its originatorID and if that matches, DO NOT add this Modifier object to the map as the reference
        // already exists, the Module class can modify the contents of the Modifier object without really calling this function,
        // however, to maintain consistent code, the Module classes will always call this function to notify the map class
        // that the contents of the map was changed, or made 'dirty':
        modMap->m_MapIsDirty = true;
        ModifierMapType::iterator cur;
        std::pair<ModifierMapType::iterator,ModifierMapType::iterator> range;
        range = modMap->m_ModifierMap.equal_range(modifierRef->GetModifierValue());   // Get the one or more modifier map entries matching this modifier being added
        for (cur=range.first; cur!=range.second; ++cur)
            if ( cur->second->GetOriginatorID() == originatorID )
                return 1;   // Yep, we found the Modifier owned by this originatorID, so we return "success" because the Module
                            // class object already updated this Modifier through its own ModifierRef, we don't need to do anything
                            // else here except return and prevent ADDING to the ModifierMap

        // For loop searching existing modifiers completed, so this originatorID's Modifier
        // is NOT in the map yet... Let's add it:
        modMap->m_ModifierMap.insert(std::pair<double, ModifierRef>(modifierRef->GetModifierValue(), modifierRef));
    }
    else
    {
        // Modifier entry in this attributeID's Modifier Map does not exist yet, so lets insert it for the first time:
        // Insert the (modifierValue, ModifierRef) pair into the Modifier Map for this attributeID:
        modMap->m_ModifierMap.insert(std::pair<double, ModifierRef>(modifierRef->GetModifierValue(), modifierRef));
        modMap->m_MapIsDirty = true;
        m_LocalShipSkillModifierMaps->insert(std::pair<uint32, ModifierMap *>(attributeID, modMap));
    }

    return 1;
}

int32 ModuleManager::RemoveShipSkillEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    bool bModifierFound = false;
    ModifierMap * modMap = nullptr;

    if ( m_LocalShipSkillModifierMaps->find(attributeID) != m_LocalShipSkillModifierMaps->end() )
    {
        modMap = m_LocalShipSkillModifierMaps->find(attributeID)->second;
        modMap->m_MapIsDirty = true;

        if ( modMap->m_ModifierMap.find(modifierRef->GetModifierValue()) != modMap->m_ModifierMap.end() )
        {
            modMap->m_MapIsDirty = true;
            ModifierMapType::iterator cur;
            std::pair<ModifierMapType::iterator,ModifierMapType::iterator> range;
            range = modMap->m_ModifierMap.equal_range(modifierRef->GetModifierValue());   // Get the one or more modifier map entries matching this modifier being removed
            for (cur=range.first; cur!=range.second; ++cur)
                if ( cur->second->GetOriginatorID() == originatorID )
                {
                    bModifierFound = true;  // Yep, we found the Modifier owned by this originatorID, so we break out of the for ()
                                            // so we can now remove this exact Modifier object from the multimap
                    break;
                }

            if ( bModifierFound == true )
            {
                // For loop searching existing modifiers completed, so this originatorID's Modifier
                // was found in the map
                modMap->m_ModifierMap.insert(std::pair<double, ModifierRef>(modifierRef->GetModifierValue(), modifierRef));
            }
            else
                return -1;  // This modifier's originatorID was not found in the map, so return error code
        }
        else
            return -1;  // This modifier's modifier value was not even found in the map, so return error code
    }
    else
        return -1;  // Modifier Map for supplied attributeID does not exist, return error value

    return 1;
}

int32 ModuleManager::ApplyModuleRigEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    sLog.Magenta("ModuleManager::ApplyModuleRigEffect()","Needs to be implemented");
    return 1;
}

int32 ModuleManager::RemoveModuleRigEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    sLog.Magenta("ModuleManager::RemoveModuleRigEffect()","Needs to be implemented");
    return 1;
}

int32 ModuleManager::ApplyImplantEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    sLog.Magenta("ModuleManager::ApplyImplantEffect()","Needs to be implemented");
    return 1;
}

int32 ModuleManager::RemoveImplantEffect(uint32 attributeID, uint32 originatorID, ModifierRef modifierRef)
{
    sLog.Magenta("ModuleManager::RemoveImplantEffect()","Needs to be implemented");
    return 1;
}

void ModuleManager::_processExternalEffect(SubEffect * s)
{
    //50-50 it's targeting a specific module ( i'm assuming here )
    GenericModule* mod = m_Modules->GetModule(s->TargetItemID());
    if (mod)
    {
        //calculate new attribute
        mod->SetAttribute(s->AttributeID(),
                          CalculateNewAttributeValue(mod->GetAttribute(s->AttributeID()),
                                                                       s->AppliedValue(), s->CalculationType()));
    }
    else if ( s->TargetItemID() == m_Ship->itemID() ) //guess it's not, but that means it should be targeting our ship itself
    {
        //calculate new attribute
        m_Ship->SetAttribute(s->AttributeID(),
                             CalculateNewAttributeValue(m_Ship->GetAttribute(s->AttributeID()),
                                                                             s->AppliedValue(), s->CalculationType()));
    }
    else //i have no idea what their targeting X_X
        sLog.Error("ModuleManager", "Process external effect inconsistency.  This shouldn't happen");

}

ModuleCommand ModuleManager::_translateEffectName(std::string s)
{
    //slow but it's better to do it once then many times as it gets passed around in modules or w/e
    //all modules should expect a ModuleCommand instead of a string

    //slightly faster version for when I know what things are really called
    //might as well use, but will definately not be right

    switch(s[0])
    {
        case 'a': return ACTIVATE;
        case 'd': return (s[2] == 'a' ? DEACTIVATE : DEOVERLOAD);
        case 'o': return (s[1] == 'n' ? ONLINE : (s[1] == 'f' ? OFFLINE : OVERLOAD)); //compound booleans ftw
    }

    return CMD_ERROR;
}

#pragma endregion
//////////////////////////////////////////////////////////////////////////////////
