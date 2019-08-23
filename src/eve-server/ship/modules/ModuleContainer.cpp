
 /**
  * @name ModuleContainer.cpp
  *     module container class for ship's module manager
  *
  * @Author:    Allan
  * @date:      6Feb18  (split from moduleMgr code)
  *
  */


#include "ship/Ship.h"
#include "ship/modules/ModuleContainer.h"
#include "GenericModule.h"


ModuleContainer::ModuleContainer(ShipItem* pShip) {

    assert(pShip != nullptr);

    // set max slotcount from ship attribs.  as slots are filled, change ship attrib to match avalible count
    m_LowSlots = (uint8)pShip->GetAttribute(AttrLowSlots).get_int();
    m_MediumSlots = (uint8)pShip->GetAttribute(AttrMedSlots).get_int();
    m_HighSlots = (uint8)pShip->GetAttribute(AttrHiSlots).get_int();
    m_RigSlots = (uint8)pShip->GetAttribute(AttrRigSlots).get_int();
    m_SubSystemSlots = (uint8)pShip->GetAttribute(AttrSubSystemSlot).get_int();

    ClearModMap();
}

ModuleContainer::~ModuleContainer()
{
    std::map<uint8, GenericModule*>::iterator itr = m_modules.begin(), end = m_modules.end();
    for (; itr != end; ++itr)
        SafeDelete(itr->second);
}

void ModuleContainer::ClearModMap() {
    // this will populate the module map for all possible slots with null pointer
    //   this does NOT verify slot count for given ship
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
    if (itr == m_modules.end()) {
        // error here for invalid flag or invalid container?
        return false;
    } else
        itr->second = pMod;
    _log(SHIP__MODULE_TRACE, "AddModule() - adding %s.", pMod->GetSelf()->itemName().c_str());

    // Maintain the Modules Fitted By Group counter for this module group:
    if ( m_ModulesFittedByGroupID.find(pMod->groupID()) != m_ModulesFittedByGroupID.end() )
        m_ModulesFittedByGroupID.find(pMod->groupID())->second += 1;
    else
        m_ModulesFittedByGroupID.emplace(pMod->groupID(), 1);

    // module is fit so change state from Unfitted to Offline
    pMod->SetModuleState(Module::State::Offline);
    return true;
}

bool ModuleContainer::RemoveModule(EVEItemFlags flag) {
    GenericModule* pMod = GetModule(flag);
    if (pMod == nullptr)
        return false;
    _log(SHIP__MODULE_TRACE, "RemoveModule() - removing %s.", pMod->GetSelf()->itemName().c_str());

    deleteModuleRef(pMod->flag(), pMod);
    return true;
}

bool ModuleContainer::RemoveModule(uint32 itemID) {
    GenericModule* pMod = GetModule(itemID);
    if (pMod == nullptr)
        return false;
    _log(SHIP__MODULE_TRACE, "RemoveModule() - removing %s.", pMod->GetSelf()->itemName().c_str());

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
    std::map<uint8, GenericModule*>::iterator itr = m_modules.begin();
    while (itr != m_modules.end()) {
        if (itr->second != nullptr)
            if (itr->second->itemID() == itemID)
                return itr->second;
        ++itr;
    }
    return nullptr;
}

void ModuleContainer::ShipWarping()
{
    // active modules not safe for warp will be Deactivated
    for (auto cur : m_modules)
        if (cur.second != nullptr)
            if (!cur.second->isWarpSafe())
                cur.second->AbortCycle();
            /*
             * {'messageKey': 'EffectDeactivationCloaking', 'dataID': 17883455, 'suppressable': False, 'bodyID': 259510, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 849}
             * {'messageKey': 'EffectDeactivationWarping', 'dataID': 17883458, 'suppressable': False, 'bodyID': 259511, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 850}
             *
             * {'FullPath': u'UI/Messages', 'messageID': 259510, 'label': u'EffectDeactivationCloakingBody'}(u'As certain activated effects interfere with the warping process, these are automatically being deactivated before the warp proceeds.', None, None)
             * {'FullPath': u'UI/Messages', 'messageID': 259511, 'label': u'EffectDeactivationWarpingBody'}(u'As certain activated effects interfere with the warping process, these are automatically being deactivated before the warp proceeds.', None, None)
             */
}

void ModuleContainer::OfflineAll() {
    for (auto cur : m_modules)
        if (cur.second != nullptr)
            cur.second->Offline();
}

void ModuleContainer::DeactivateAll() {
    for (auto cur : m_modules)
        if (cur.second != nullptr)
            cur.second->Deactivate();
}

void ModuleContainer::AbortCycle() {
    for (auto cur : m_modules)
        if (cur.second != nullptr)
            cur.second->AbortCycle();
}

void ModuleContainer::UnloadAll() {
    for (auto cur : m_modules)
        if (cur.second != nullptr)
            cur.second->UnloadCharge();
}

void ModuleContainer::RepairAll() {
    for (auto cur : m_modules)
        if (cur.second != nullptr)
            cur.second->Repair();
}

void ModuleContainer::UnloadWeapons()
{
    std::map<uint8, GenericModule*>::iterator itr;
    for (uint8 i = flagHiSlot0; 1 < flagFixedSlot; ++i) {
        itr = m_modules.find(i);
        if (itr != m_modules.end())
            itr->second->UnloadCharge();
    }
}

void ModuleContainer::Process() {
    // proc modules in order of (subsys -> rig -> high -> mid -> low) for proper fx application
    std::map<uint8, GenericModule*>::reverse_iterator itr = m_modules.rbegin(), end = m_modules.rend();
    while (itr != end) {
        if (itr->second != nullptr)
            itr->second->Process();
        ++itr;
    }
}

void ModuleContainer::OnlineAll() {
    // must proc modules in order of (subsys -> rig -> high -> mid -> low) for proper fx application
    std::map<uint8, GenericModule*>::reverse_iterator itr = m_modules.rbegin(), end = m_modules.rend();
    while (itr != end) {
        if (itr->second != nullptr)
            itr->second->Online();
        ++itr;
    }
}

bool ModuleContainer::isSlotOccupied(EVEItemFlags flag) {
    return (m_modules.find((uint8)flag)->second != nullptr);
}

uint16 ModuleContainer::GetAvailableSlotInBank(EVEEffectID slotBank)
{
    switch (slotBank) {
        case EVEEffectID::loPower: {
            for (uint8 slot=flagLowSlot0; slot < (flagLowSlot0 + 8); ++slot)
                if ( m_modules[slot] == nullptr )
                    return slot;
            } break;
        case EVEEffectID::medPower: {
            for (uint8 slot=flagMedSlot0; slot < (flagMedSlot0 + 8); ++slot)
                if ( m_modules[slot] == nullptr )
                    return slot;
            } break;
        case EVEEffectID::hiPower: {
            for (uint8 slot=flagHiSlot0; slot < (flagHiSlot0 + 8); ++slot)
                if ( m_modules[slot] == nullptr )
                    return slot;
            } break;
        case EVEEffectID::rigSlot: {
            for (uint8 slot=flagRigSlot0; slot < (flagRigSlot0 + 3); ++slot)
                if ( m_modules[slot] == nullptr )
                    return slot;
            } break;
        case EVEEffectID::subSystem: {
            for (uint8 slot=flagSubSystem0; slot < (flagSubSystem0 + 5); ++slot)
                if ( m_modules[slot] == nullptr )
                    return slot;
            } break;
        default: {
            // ERROR: This is not a module that fits in any of the slot banks
            return flagIllegal;
            } break;
    }
    return flagIllegal;
}

uint8 ModuleContainer::GetFittedModuleCountByGroup(uint16 groupID)
{
    if ( m_ModulesFittedByGroupID.find(groupID) != m_ModulesFittedByGroupID.end() )
        return m_ModulesFittedByGroupID.find(groupID)->second;
        
    return 0;
}

GenericModule* ModuleContainer::GetRandModule()
{
    std::vector<GenericModule*> modVec;
    for (uint8 flag = flagLowSlot0; flag < flagFixedSlot; ++flag)
        if (m_modules[flag] != nullptr)
            modVec.push_back(m_modules[flag]);

    return modVec[MakeRandomInt(0, modVec.size())];
}

void ModuleContainer::GetWeapons(std::list< GenericModule* >& weaponList)
{
    for (uint8 flag = flagHiSlot0; flag < flagFixedSlot; ++flag)
        if (m_modules[flag] != nullptr)
            if (m_modules[flag]->IsLauncherModule() or m_modules[flag]->IsTurretModule())
                weaponList.push_back(m_modules[flag]);
}

void ModuleContainer::GetModuleListOfRefsAsc(std::vector<InventoryItemRef>& moduleVec)
{
    for (auto cur : m_modules)
        if (cur.second != nullptr)
            moduleVec.push_back(cur.second->GetSelf());
}

void ModuleContainer::GetModuleListOfRefsDec(std::vector< InventoryItemRef >& moduleVec)
{
    std::map<uint8, GenericModule*>::reverse_iterator itr = m_modules.rbegin(), end = m_modules.rend();
    while (itr != end) {
        if (itr->second != nullptr)
            moduleVec.push_back( itr->second->GetSelf() );
        ++itr;
    }
}

void ModuleContainer::GetModulesInBank(EVEItemFlags flag, std::vector< GenericModule* >& modVec)
{
    // this is funky hack, but works.  :/
    switch (flag) {
        case flagLowSlot0:
        case flagLowSlot1:
        case flagLowSlot2:
        case flagLowSlot3:
        case flagLowSlot4:
        case flagLowSlot5:
        case flagLowSlot6:
        case flagLowSlot7: {
            for (uint8 slot=flagLowSlot0; slot < (flagLowSlot0 + 8); ++slot)
                if ( m_modules[slot] != nullptr )
                    modVec.push_back(m_modules[slot]);
        } break;
        case flagMedSlot0:
        case flagMedSlot1:
        case flagMedSlot2:
        case flagMedSlot3:
        case flagMedSlot4:
        case flagMedSlot5:
        case flagMedSlot6:
        case flagMedSlot7: {
            for (uint8 slot=flagMedSlot0; slot < (flagMedSlot0 + 8); ++slot)
                if ( m_modules[slot] != nullptr )
                    modVec.push_back(m_modules[slot]);
        } break;
        case flagHiSlot0:
        case flagHiSlot1:
        case flagHiSlot2:
        case flagHiSlot3:
        case flagHiSlot4:
        case flagHiSlot5:
        case flagHiSlot6:
        case flagHiSlot7: {
            for (uint8 slot=flagHiSlot0; slot < (flagHiSlot0 + 8); ++slot)
                if ( m_modules[slot] != nullptr )
                    modVec.push_back(m_modules[slot]);
        } break;
    }
}

void ModuleContainer::SaveModules()
{
    for (auto cur : m_modules)
        if (cur.second != nullptr)
            cur.second->GetSelf()->SaveItem();
}

void ModuleContainer::deleteModuleRef(EVEItemFlags flag, GenericModule* pMod)
{
    std::map<uint8, GenericModule*>::iterator itr = m_modules.find((uint8)flag);
    if (itr != m_modules.end())
        itr->second = nullptr;

    // Maintain the Modules Fitted By Group counter for this module group:
    if (m_ModulesFittedByGroupID.find(pMod->groupID()) != m_ModulesFittedByGroupID.end()) {
        if (m_ModulesFittedByGroupID.find(pMod->groupID())->second > 1) {
            // We still have more than one module of this group fitted, so just reduce number fitted by 1:
            m_ModulesFittedByGroupID.find(pMod->groupID())->second -= 1;
        } else {
            // This was the last (or only) module of this group fitted, so remove the entry from the map:
            m_ModulesFittedByGroupID.erase(pMod->groupID());
        }
    }
}

