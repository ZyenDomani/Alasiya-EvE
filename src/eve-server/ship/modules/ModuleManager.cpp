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
    Rewrite:    Allan
*/

/* updates to implement basic memory management (remove naked 'new')  -allan 30Mar16 */

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "Client.h"
#include "StaticDataMgr.h"
#include "effects/EffectsDataMgr.h"
#include "ship/Ship.h"
#include "ship/modules/ModuleContainer.h"
#include "ship/modules/ModuleItem.h"
#include "ship/modules/ModuleManager.h"
#include "ship/modules/ModuleFactory.h"
#include "ship/modules/ActiveModule.h"
#include "system/DestinyManager.h"


ModuleManager::ModuleManager(ShipItem *const pShip)
: m_Ship(pShip),
pModuleCont(new ModuleContainer(pShip)),
m_initalized(false),
m_rigScanBonus(1.0f)
{
    assert(pShip != nullptr);

    m_LowSlots = (uint8)pShip->GetAttribute(AttrLowSlots).get_int();
    m_MidSlots = (uint8)pShip->GetAttribute(AttrMedSlots).get_int();
    m_HighSlots = (uint8)pShip->GetAttribute(AttrHiSlots).get_int();
    m_SubSystemSlots = (uint8)pShip->GetAttribute(AttrSubSystemSlot).get_int();
}

ModuleManager::~ModuleManager()
{
    //module cleanup is handled in the ModuleContainer destructor
    SafeDelete(pModuleCont);
}

bool ModuleManager::Initialize() {
    if (m_initalized)
        return true;

    // Load modules, charges, rigs and subsystems into ship's ModuleContainer:
    std::vector<InventoryItemRef> itemVec;
    m_Ship->GetMyInventory()->GetInventoryVec(itemVec);

    GenericModule* pMod(nullptr);
    for (auto cur : itemVec) {
        // this is a hack.  dont know why any ship item would have flagAutoFit set, but have seen random errors where charges are set to flagAutoFit
        if (cur->flag() == flagAutoFit) {
            _log(SHIP__MODULE_ERROR, "ModuleManager::Initialize() - %s(%u) has flagAutoFit set in ship %s",\
                    cur->itemName().c_str(), cur->itemID(), m_Ship->itemName().c_str() );
            cur->SetFlag(flagCargoHold);    // put that bitch back in cargo
        }
        if (IsModuleSlot(cur->flag()))
            switch (cur->categoryID()) {
                case EVEDB::invCategories::Module:
                case EVEDB::invCategories::Subsystem: {
                    ModuleItemRef mRef = ModuleItemRef::StaticCast(cur);
                    fitModule(mRef, cur->flag());
                } break;
                case EVEDB::invCategories::Charge: {
                    pMod = GetModule(cur->flag());
                    if (pMod != nullptr) {
                        pMod->SetChargeRef(cur);
                        // set ChargeState == CHG_LOADED here, then when module Online() is called, all effects will be applied in correct order
                        pMod->SetChargeState(Module::State::Loaded);
                        m_charges.emplace(cur->flag(), cur);
                    } else {
                        // module to load not found...
                        cur->SetFlag(flagCargoHold);    // put that bitch back in cargo
                        _log(SHIP__MODULE_ERROR, "ModuleManager::Initialize() - Cannot find module at %s to load charge %s(%u) into",\
                                sDataMgr.GetFlagName(cur->flag()), cur->itemName().c_str(), cur->itemID() );
                    }
                    pMod = nullptr;
                } break;
            }
    }
    return (m_initalized = true);
}

void ModuleManager::Process()
{
    double profileStartTime = GetTimeUSeconds();

    pModuleCont->Process();

    if (sConfig.debug.UseProfiling)
        sProfile.AddTime(_modulesProfile, GetTimeUSeconds() - profileStartTime);
}

bool ModuleManager::IsSlotOccupied(EVEItemFlags flag)
{
    if (pModuleCont->GetModule(flag))
        return true;
    return false;
}

uint16 ModuleManager::GetAvailableSlotInBank(EVEEffectID slotBank)
{
	// Call into ModuleContainer class with slotBank effectID to have it check for and return first available slot flag in
	// in the specified slot bank:
	return pModuleCont->GetAvailableSlotInBank(slotBank);
}

void ModuleManager::GetModulesInBank(EVEItemFlags flag, std::vector<GenericModule*>& modVec)
{
    pModuleCont->GetModulesInBank(flag, modVec);
}

bool ModuleManager::InstallRig(ModuleItemRef mRef, EVEItemFlags flag) {
    if (((mRef->groupID() >= EVEDB::invGroups::Rig_Armor) and (mRef->groupID() <= EVEDB::invGroups::Rig_Astronautic))
    or (mRef->groupID() == EVEDB::invGroups::Rig_Electronics_Superiority)) {
        fitModule(mRef,flag);
        // hack to get total scan bonus from rigs, if applicable
        if (mRef->groupID() == EVEDB::invGroups::Rig_Electronics) {
            switch (mRef->typeID()) {
                case 25936:   //  Large Gravity Capacitor Upgrade I
                case 31213:   //  Small Gravity Capacitor Upgrade I
                case 31215:   //  Medium Gravity Capacitor Upgrade I
                case 31217:   //  Capital Gravity Capacitor Upgrade I
                case 26350:   //  Large Gravity Capacitor Upgrade II
                case 31220:   //  Small Gravity Capacitor Upgrade II
                case 31222:   //  Medium Gravity Capacitor Upgrade II
                case 31224: { //  Capital Gravity Capacitor Upgrade II
                    m_rigScanBonus += mRef->GetAttribute(AttrScanStrengthBonus).get_float();
                } break;
            }
        }
        return true;
    } else
        codelog(SHIP__MODULE_TRACE, "ModuleManager","%s tried to fit item %s(%u), which is not a rig", m_Ship->GetPilot()->GetName(), mRef->itemName().c_str(), mRef->itemID());

    return false;

    /*
    10%
    15%
    */
}

void ModuleManager::UninstallRig(uint32 itemID)
{
    GenericModule* pMod = pModuleCont->GetModule(itemID);
    if (pMod == nullptr) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::UninstallRig() -  Rig %u not found", itemID);
        return;
    }

    pMod->Offline();
    if (!sConfig.debug.IsTestServer)
        pMod->RemoveRig();
    pModuleCont->RemoveModule(itemID);
    m_Ship->SetAttribute(AttrUpgradeLoad, (m_Ship->GetAttribute(AttrUpgradeLoad) - pMod->GetAttribute(AttrUpgradeCost)));
    m_Ship->SetAttribute(AttrUpgradeSlotsLeft, m_Ship->GetAttribute(AttrUpgradeSlotsLeft) +1);

    if (pMod->groupID() == EVEDB::invGroups::Rig_Electronics) {
        switch (pMod->typeID()) {
            case 25936:   //  Large Gravity Capacitor Upgrade I
            case 31213:   //  Small Gravity Capacitor Upgrade I
            case 31215:   //  Medium Gravity Capacitor Upgrade I
            case 31217:   //  Capital Gravity Capacitor Upgrade I
            case 26350:   //  Large Gravity Capacitor Upgrade II
            case 31220:   //  Small Gravity Capacitor Upgrade II
            case 31222:   //  Medium Gravity Capacitor Upgrade II
            case 31224: { //  Capital Gravity Capacitor Upgrade II
                m_rigScanBonus -= pMod->GetAttribute(AttrScanStrengthBonus).get_float();
            } break;
        }
    }
}

bool ModuleManager::InstallSubSystem(ModuleItemRef mRef, EVEItemFlags flag)
{
    if (mRef->categoryID() != EVEDB::invCategories::Subsystem) {
        sLog.Warning("ModuleManager","%s tried to fit item %u, which is not a subsystem", m_Ship->GetPilot()->GetName(), mRef->itemID());
        return false;
    }

    fitModule(mRef,flag);
}

void ModuleManager::CheckSlotFitLimited(EVEItemFlags flag)
{
    if (IsRigSlot(flag))
        return;
    if (IsHiSlot(flag)) {
        if (m_HighSlots)
            return;
    } else if (IsMidSlot(flag)) {
        if (m_MidSlots)
            return;
    } else if (IsLowSlot(flag)) {
        if (m_LowSlots)
            return;
    } else if (IsSubSystem(flag)) {
        if (m_SubSystemSlots)
            return;
    }

    throw PyException( MakeUserError("NoFreeShipSlots"));
}

void ModuleManager::CheckGroupFitLimited(EVEItemFlags flag, InventoryItemRef iRef)
{
    if (iRef->HasAttribute(AttrMaxGroupFitted)) {
        // some of these are checked client-side (by attrib) so this may not be needed.
        if (pModuleCont->GetFittedModuleCountByGroup(iRef->groupID()) >= iRef->GetAttribute(AttrMaxGroupFitted).get_int()) {
            /*
            std::map<std::string, PyRep *> args;
            args["noOfModules"]         = new PyInt(iRef->GetAttribute(AttrMaxGroupFitted).get_int());
            args["noOfModulesFitted"]   = new PyInt(pModuleCont->GetFittedModuleCountByGroup(iRef->groupID()));
            args["ship"]                = new PyInt(m_Ship->itemID());
            args["groupName"]           = new PyString(iRef->group().name());
            args["module"]              = new PyInt(iRef->itemID());
            throw PyException( MakeUserError("CantFitTooManyByGroup", args));   // bad msgID in client.
            */
            throw PyException(MakeCustomError("Group Fit Limited.<br>You are unable to fit the %s to your %s.", iRef->itemName().c_str(), m_Ship->itemName().c_str()));
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
}

void ModuleManager::UnfitModule(uint32 itemID)
{
    GenericModule* pMod = pModuleCont->GetModule(itemID);
    if (pMod == nullptr) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::UnfitModule() -  Module %u not found", itemID);
        return;
    }

    EVEItemFlags flag = flagHangar;
    bool inSpace = IsSolarSystem(m_Ship->locationID());
    if (inSpace)
        flag = flagCargoHold;
    pMod->AbortCycle();
    pMod->Offline();
    if (pMod->IsLoaded()) {
        //{'FullPath': u'UI/Messages', 'messageID': 260011, 'label': u'CannotRemoveModuleWithLoadedChargesBody'}(u'You cannot remove a module while it is still loaded with charges.', None, None)
        if (pMod->GetLoadedChargeRef().get() != nullptr)    // just in case...
            pMod->GetLoadedChargeRef()->Move((inSpace ? m_Ship->itemID() : m_Ship->locationID()), flag, true);
        pMod->UnloadCharge();
    }
    // update avalible slots
    if (pMod->isHighPower()) {
        if (pMod->isTurretFitted())
            m_Ship->SetAttribute(AttrTurretSlotsLeft, (m_Ship->GetAttribute(AttrTurretSlotsLeft) +1));
        else if (pMod->isLauncherFitted())
            m_Ship->SetAttribute(AttrLauncherSlotsLeft, (m_Ship->GetAttribute(AttrLauncherSlotsLeft) +1));
        ++m_HighSlots;
    } else if (pMod->isMediumPower()) {
        ++m_MidSlots;
    } else if (pMod->isLowPower()) {
        ++m_LowSlots;
    } else if (pMod->isSubSystem()) {
        ++m_SubSystemSlots;
    }
    pModuleCont->RemoveModule(itemID);
    // delete the module object
    SafeDelete(pMod);
}

bool ModuleManager::FitModule(ModuleItemRef mRef, EVEItemFlags flag)
{
    if (!IsModuleSlot(flag)) {
        sLog.Warning("ModuleManager::FitModule","%s is not a module slot.", sDataMgr.GetFlagName(flag));
        return false;
    }

    fitModule(mRef, flag);

    //Online(mRef->itemID());
    return true;
}

void ModuleManager::fitModule(ModuleItemRef mRef, EVEItemFlags flag)
{
    if (!IsModuleSlot(flag)) {
        sLog.Warning("ModuleManager::fitModule","%s is not a module slot.", sDataMgr.GetFlagName(flag));
        return;
    }
    if (pModuleCont->isSlotOccupied(flag)) {
        throw PyException( MakeUserError("SlotAlreadyOccupied"));
        /** @todo change this to use movemodule? */
        return;
    }

    // create new module object
    GenericModule* pMod = ModuleFactory(mRef, ShipItemRef(m_Ship));
    if (pMod == nullptr)
        return; // error here?

    if (!pModuleCont->AddModule(flag, pMod))
        return; // error here?

    // update avalible slots
    if (pMod->isHighPower()) {
        if (pMod->isTurretFitted()) {
            // apply config modifier, if applicable
            mRef->MultiplyAttribute(AttrSpeed, sConfig.rates.turretRoF);
            m_Ship->SetAttribute(AttrTurretSlotsLeft, (m_Ship->GetAttribute(AttrTurretSlotsLeft) -1));
        } else if (pMod->isLauncherFitted()) {
            // apply config modifier, if applicable
            mRef->MultiplyAttribute(AttrSpeed, sConfig.rates.missileRoF);
            m_Ship->SetAttribute(AttrLauncherSlotsLeft, (m_Ship->GetAttribute(AttrLauncherSlotsLeft) -1));
        }
        --m_HighSlots;
    } else if (pMod->isMediumPower()) {
        --m_MidSlots;
    } else if (pMod->isLowPower()) {
        --m_LowSlots;
    } else if (pMod->isSubSystem()) {
        --m_SubSystemSlots;
    } else if (pMod->isRig()) {
        pMod->Online();
        m_Ship->SetAttribute(AttrUpgradeLoad, (m_Ship->GetAttribute(AttrUpgradeLoad) + pMod->GetAttribute(AttrUpgradeCost)));
        m_Ship->SetAttribute(AttrUpgradeSlotsLeft, (m_Ship->GetAttribute(AttrUpgradeSlotsLeft) -1));
    }

    /*
    if (is_log_enabled(SHIP__MODULE_DEBUG)) { // debug msg?
        std::map<std::string, PyRep *> args;
        args["item"]  = new PyString(iRef->itemName());
        args["slot"]  = new PyString(sDataMgr.GetFlagName(flag));
        throw PyException( MakeUserError("ModuleFit", args));
        */
    /*{'messageKey': 'ModuleFit', 'dataID': 17883325, 'suppressable': False, 'bodyID': 259463, 'messageType': 'notify', 'urlAudio': 'wise:/msg_ModuleFit_play', 'urlIcon': '', 'titleID': None, 'messageID': 1227}
     * u'ModuleFitBody'}(u'{item} fitted onto slot {slot}', None, {
     * u'{item}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'item'},
     * u'{slot}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'slot'}})
     *
    }*/
}

void ModuleManager::Online(uint32 itemID)
{
    GenericModule* pMod = pModuleCont->GetModule(itemID);
    if (pMod == nullptr) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::Online(itemID) -  Module %u not found", itemID);
        return;
    }
    if (pMod->isOnline()) {
        _log(SHIP__MODULE_TRACE, "ModuleManager::Online(itemID) -  %s already Online", pMod->GetSelf()->itemName().c_str());
        if (m_Ship->HasPilot())
            if (m_Ship->GetPilot()->CanThrow()) {
                std::map<std::string, PyRep *> args;
                args["modulename"] = new PyString(pMod->GetSelf()->itemName());
                throw PyException( MakeUserError("EffectAlreadyActive2", args));
            }
        return;
    }

    _log(SHIP__MODULE_TRACE, "ModuleManager::Online(itemID) -  %s going Online", pMod->GetSelf()->itemName().c_str());
    pMod->Online();
}

void ModuleManager::Online(EVEItemFlags flag)
{
    GenericModule* pMod = pModuleCont->GetModule(flag);
    if (pMod == nullptr) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::Online(flag) -  Module not found in %s", sDataMgr.GetFlagName(flag));
        return;
    }
    if (pMod->isOnline()) {
        _log(SHIP__MODULE_TRACE, "ModuleManager::Online(flag) -  %s already Online", pMod->GetSelf()->itemName().c_str());
        return;
    }

    _log(SHIP__MODULE_TRACE, "ModuleManager::Online(flag) -  %s going Online", pMod->GetSelf()->itemName().c_str());
    pMod->Online();
}

void ModuleManager::Offline(uint32 itemID)
{
    GenericModule* pMod = pModuleCont->GetModule(itemID);
    if (pMod == nullptr) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::Offline(itemID) -  Module %u not found", itemID);
        return;
    }
    if (!pMod->isOnline()) {
        _log(SHIP__MODULE_TRACE, "ModuleManager::Offline(itemID) -  %s not Online", pMod->GetSelf()->itemName().c_str());
        pMod->SetModuleState(Module::State::Offline);
        return;
    }

    _log(SHIP__MODULE_TRACE, "ModuleManager::Offline(itemID) -  %s going Offline", pMod->GetSelf()->itemName().c_str());
    pMod->Offline();
}

void ModuleManager::Offline(EVEItemFlags flag)
{
    GenericModule* pMod = pModuleCont->GetModule(flag);
    if (pMod == nullptr) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::Offline(flag) -  Module not found in %s", sDataMgr.GetFlagName(flag));
        return;
    }
    if (!pMod->isOnline()) {
        _log(SHIP__MODULE_TRACE, "ModuleManager::Offline(flag) -  %s not Online", pMod->GetSelf()->itemName().c_str());
        pMod->SetModuleState(Module::State::Offline);
        return;
    }
    _log(SHIP__MODULE_TRACE, "ModuleManager::Offline(flag) -  %s going Offline", pMod->GetSelf()->itemName().c_str());
    pMod->Offline();
}

void ModuleManager::AbortCycle()
{
    pModuleCont->AbortCycle();
}

void ModuleManager::OnlineAll()
{
    pModuleCont->OnlineAll();
}

void ModuleManager::OfflineAll()
{
    pModuleCont->OfflineAll();
}

void ModuleManager::DeactivateAllModules()
{
    pModuleCont->DeactivateAll();
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

    GenericModule* pMod = pModuleCont->GetModule(itemID);
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
}

void ModuleManager::Deactivate(uint32 itemID, std::string effectName)
{
    GenericModule* pMod = pModuleCont->GetModule(itemID);
    if (pMod != nullptr) {
        if (pMod->GetModuleState() != Module::State::Activated)  // we dont need an error msgs here....this is acceptable, as the module may not be active
            return;
        _log(SHIP__MODULE_TRACE, "ModuleManager::Deactivate() - %s Deactivating - '%s'", pMod->GetSelf()->itemName().c_str(), effectName.c_str());
        pMod->Deactivate(effectName);
    } else
        _log(SHIP__MODULE_ERROR, "ModuleManager::Deactivate() - Called on module %u that is not loaded.", itemID );
}

void ModuleManager::Overload(uint32 itemID)
{
    GenericModule* pMod = pModuleCont->GetModule(itemID);
    if (pMod == nullptr) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::Overload() - Called on module %u that is not loaded.", itemID);
        return;
    }
    pMod->Overload();
    _log(SHIP__MODULE_TRACE, "ModuleManager::Overload() - %s Overloading...", pMod->GetSelf()->itemName().c_str());
}

void ModuleManager::DeOverload(uint32 itemID)
{
    GenericModule* pMod = pModuleCont->GetModule(itemID);
    if (pMod == nullptr) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::DeOverload() - Called on module %u that is not loaded.", itemID);
        return;
    }
    pMod->DeOverload();
    _log(SHIP__MODULE_TRACE, "ModuleManager::DeOverload() - %s DeOverload...", pMod->GetSelf()->itemName().c_str());
}

void ModuleManager::DamageModule(uint32 itemID, float amount)
{
    DamageModule(pModuleCont->GetModule(itemID), amount);
}

void ModuleManager::DamageRandModule()
{
    DamageModule(pModuleCont->GetRandModule(), 1.0f);
}

void ModuleManager::DamageRandModule(float amount)
{
    DamageModule(pModuleCont->GetRandModule(), amount);
}

void ModuleManager::DamageModule(GenericModule* pMod, float amount)
{
    if (pMod == nullptr) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::DamageModule() - Module not found.");
        return;
    }

    pMod->SetAttribute(AttrDamage, (pMod->GetAttribute(AttrDamage) + amount));  //verfify this works as intended
    _log(SHIP__MODULE_DAMAGE, "ModuleManager::DamageModule() - %s taking %.2f damage.  current damage %.2f",  \
                pMod->GetSelf()->itemName().c_str(), amount, pMod->GetAttribute(AttrDamage).get_float());
    if (pMod->GetAttribute(AttrDamage) >= pMod->GetAttribute(AttrHP)) {
        //  this is for offlining entire group...this isnt right.
        /*
        if (pMod->IsLinked()) {
            // loop thru linked modules and offline all
            m_Ship->GetPilot()->SendNotifyMsg("Your group of %s has gone offline due to damage.", pMod->GetSelf()->itemName().c_str());
            m_Ship->OfflineGroup(pMod);
        } else */
        m_Ship->GetPilot()->SendNotifyMsg("Your %s in %s has gone offline due to damage.", pMod->GetSelf()->itemName().c_str(), sDataMgr.GetFlagName(pMod->flag()));
        pMod->Offline();
    }
}

void ModuleManager::RepairModule(uint32 itemID, EvilNumber amount)
{
    RepairModule(pModuleCont->GetModule(itemID), amount);
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
    pModuleCont->RepairAll();
}

PyRep* ModuleManager::ModuleRepair(uint32 modID)
{
    /*  Restrictions/Capabilities
     *
     *    Cannot be used while overloading any modules.
     *    Cannot be used on an active module.
     *    Cannot be used to repair a 100% damaged module (0/40hp). These must first be repaired to at least 1hp at a station.
     *    Can be used to repair an offline module with at least 1hp remaining.
     *    Can be used on an inactive module while other modules are active.
     *    Can be used to repair any inactive modules while cloaked (everything but the cloak itself, of course).
     *    Can repair multiple modules at once.
     *    Can be canceled mid-repair, and will retain whatever repairs could be completed in the time it was active. Canceling a repair on a module that takes 1-2 paste to repair fully is occasionally problematic, and will round down - e.g. you need to have repaired enough HP to take at least one unit of paste before canceling mid-repair will result in any repaired damage or paste consumed. No paste is ever consumed without appropriate repairs being done, however.
     *    You can jump or dock while repairing, which will have the same effect as canceling the repair manually.
     *    You can repair a passive module (such as a plate, extender, or EANM) without taking it offline, and you still receive the benefit from passive modules while repairing them. Capacitor batteries were fixed and can now be repaired while online!
     *
     * Efficiency
     *
     *    Nanite efficiency is based on the base cost of the module, rather than amount of HP repaired.
     *    All modules have 40hp, but base cost varies wildly.
     *    Base efficiency for a theoretical (but impossible) full repair (0/40hp remaining) is approximately 0.0000775 paste per isk of base item cost.
     *    To couch this in more relatable terms, this means that an item with a base cost of 100k isk will cost 7 or 8 paste to repair; an item with a base cost of 1m isk will cost 77-78 paste to repair.
     *    The Nanite Operation skill reduces consumption by 5% per level. At V, nanite efficiency will be 0.000058125/isk, or ~58 units of paste per 1m isk base cost.
     *    NPC station repair costs are equal to the item base cost, modified marginally by standing. Base repair cost with nanites is 7.7x11700=90,090 per 100,000 isk, or 10% less. Even without the Nanite Operation skill, repairs with nanite paste are always slightly cheaper than repairs at NPC stations. With Nanite Operation trained up, repairing with nanite paste is significantly cheaper than at NPC stations.
     *
     * Speed
     *
     *    Nanite paste has a base repair speed of 10hp per minute, independent of module type or cost.
     *    As all modules have 40hp, and paste cannot be used to repair 100% damaged items, the most time a repair can take at base skill levels is 3:54 on a 97% damaged module.
     *    The Nanite Interfacing skill improves repair speed by 20% (or 2hp) per level. At V, repair rate is 20hp per minute with a max repair time of 1:57.
     *
     * Module Repair Costs
     *
     * Again, the quantity of nanite paste consumed to repair an item is dependent on its base cost.
     * You can easily find the base cost of a module by looking it up in the Item Browser subsection of Evemon's skill plans.
     *
     *    As a general rule, base cost is usually around 1/4 of the Empire price of an item. This mostly applies to T1 items, but T2 items for which demand is not extremely high tend to follow this as well.
     *    As you might suspect, battleship-class modules have much higher base costs than frigate-class modules. This applies mostly to Afterburners, Microwarpdrives, Armor Repairers, and Shield Boosters. Most larger modules also produce less heat damage however, so the difference in nanites consumed per amount of time overheated is not as pronounced.
     *    T2 items have the highest base cost at around 2-6x that of T1.
     *    Named items often have lower base costs than T1, and are never higher.
     *    Faction, Deadspace, and Officer items have wildly varying base costs.
     * Most are similar or identical to named, others T1, and a few are higher than T1 but still much lower than T2.
     * Officer/Deadspace MWDs are an odd exception to this, as all of them have the same base cost regardless of size class (790k).
     * In most cases, this makes faction items prime candidates for overloading as they produce similar or less heat damage while being radically more
     * effective and cheaper to repair than their T1 or T2 counterparts.
     */

    GenericModule* pMod = pModuleCont->GetModule(modID);
    if (pMod == nullptr) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::ModuleRepair() - module %s not found.", modID);
        return PyStatic.NewFalse();
    }



    //return PyStatic.NewTrue();  // can repair
    return PyStatic.NewFalse(); // cannot repair (for whatever reason)  do they/we send msgs based on why here?
}

void ModuleManager::StopModuleRepair(uint32 modID)
{
    GenericModule* pMod = pModuleCont->GetModule(modID);
    if (pMod == nullptr) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::ModuleRepair() - module %s not found.", modID);
        return;
    }


}

void ModuleManager::LoadCharge(InventoryItemRef chargeRef, EVEItemFlags flag)
{
    GenericModule* pMod = pModuleCont->GetModule(flag);
    if (pMod == nullptr) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::LoadCharge() - module not found at slot %i", flag);
        return;
    }
    float modCapacity = pMod->GetAttribute(AttrCapacity).get_float();
    float chargeVolume = chargeRef->GetAttribute(AttrVolume).get_float();

    if (pMod->IsLoaded()) {
        if (chargeRef->typeID() == pMod->GetLoadedChargeRef()->typeID())
            modCapacity -= (chargeVolume * pMod->GetLoadedChargeRef()->quantity());
        else
            UnloadCharge(flag, true); // change charges
    }
    //{'FullPath': u'UI/Messages', 'messageID': 256676, 'label': u'CannotLoadNotEnoughChargesBody'}(u'There are not enough charges to fully load all of your modules. Some of your modules have been left partially loaded or empty.', None, None)

    // check quantities
    if (modCapacity < chargeVolume)
        return;
    uint32 loadQty = floor((modCapacity / chargeVolume));
    if (loadQty < 1)
        return;
    InventoryItemRef oRef(chargeRef);   // make copy of chargeRef
    if (loadQty < chargeRef->quantity()) {
        chargeRef = chargeRef->Split(loadQty);
        if (chargeRef.get() == nullptr) {
            chargeRef = oRef;
            return;
        }
    }

    if (pMod->IsLoaded()) {
        pMod->GetLoadedChargeRef()->Merge(chargeRef);
    } else {
        chargeRef->Donate(m_Ship->ownerID(), m_Ship->itemID(), flag, true);
        pMod->LoadCharge(chargeRef);
        m_charges.emplace(flag, chargeRef);
    }

    // change back to orig chargeRef incase of Merge(), which deletes item
    chargeRef = oRef;   // oRef copy is deleted upon return (out of scope)
}

void ModuleManager::UnloadCharge(EVEItemFlags fromFlag, bool merge/*false*/)
{
    GenericModule* pMod = pModuleCont->GetModule(fromFlag);
    if (pMod == nullptr) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::UnloadCharge() - module not found at slot %i", fromFlag);
        return;
    }

    if (!pMod->IsLoaded()) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::UnloadCharge() - module %s at slot %i is not loaded", pMod->GetSelf()->itemName().c_str(), fromFlag);
        return;
    }

    InventoryItemRef chargeRef(nullptr);
    std::map<EVEItemFlags, InventoryItemRef>::iterator itr = m_charges.find(fromFlag);
    if (itr == m_charges.end())
        chargeRef = pMod->GetLoadedChargeRef();
    else {
        chargeRef = itr->second;
        m_charges.erase(itr);
    }
    if (chargeRef.get() == nullptr) {
        _log(SHIP__MODULE_ERROR, "ModuleManager::UnloadCharge() - charge not found in chargeList or on module %s at slot %i", pMod->GetSelf()->itemName().c_str(), fromFlag);
        return;
    }
    _log(SHIP__MODULE_TRACE, "ModuleManager::UnloadCharge() - %s unloading %s(%u) (merge:%s)",\
            pMod->GetSelf()->itemName().c_str(), chargeRef->itemName().c_str(), chargeRef->itemID(), (merge?"true":"false"));
    pMod->UnloadCharge();
    // move item and update client
    if (IsStation(m_Ship->locationID()))
        chargeRef->Move(m_Ship->locationID(), flagHangar, true);
    else if (merge and m_Ship->GetMyInventory()->ContainsTypeByFlag(chargeRef->typeID(), flagCargoHold))
        chargeRef->MergeTypesInCargo(m_Ship, flagCargoHold);
    else
        chargeRef->SetFlag(flagCargoHold, true);
}

void ModuleManager::GetLoadedCharges(std::map< EVEItemFlags, InventoryItemRef >& charges)
{
    charges = m_charges;
}

InventoryItemRef ModuleManager::GetLoadedChargeOnModule(EVEItemFlags flag) {
    GenericModule* pMod = pModuleCont->GetModule(flag);
    if ((pMod != nullptr) and pMod->IsLoaded() )
        return pMod->GetLoadedChargeRef();
    return InventoryItemRef(nullptr);
}

InventoryItemRef ModuleManager::GetLoadedChargeOnModule(InventoryItemRef moduleRef) {
    return GetLoadedChargeOnModule(moduleRef->flag());
}

bool ModuleManager::VerifySlotExchange(EVEItemFlags slot1, EVEItemFlags slot2)
{
    if (pModuleCont->GetModule(slot1)->GetModulePowerLevel() == pModuleCont->GetModule(slot2)->GetModulePowerLevel())
        return true;
    return false;
}

void ModuleManager::UnloadWeapons()
{
    pModuleCont->UnloadWeapons();
    std::map<EVEItemFlags, InventoryItemRef>::iterator itr;
    for (EVEItemFlags i = flagHiSlot0; 1 < flagFixedSlot; i+1) {
        itr = m_charges.find(i);
        if (itr != m_charges.end()) {
            if IsStation(m_Ship->locationID())
                itr->second->Move(m_Ship->locationID(), flagHangar, true);
            else if (m_Ship->GetMyInventory()->ContainsTypeByFlag(itr->second->typeID(), flagCargoHold))
                itr->second->MergeTypesInCargo(m_Ship, flagCargoHold);
            else
                itr->second->SetFlag(flagCargoHold, true);
            m_charges.erase(itr);
        }
    }
}

void ModuleManager::UnloadAllModules()
{
    // can this be called when docked?
    pModuleCont->UnloadAll();
    for (auto cur : m_charges) {
        if IsStation(m_Ship->locationID())
            cur.second->Move(m_Ship->locationID(), flagHangar, true);
        else if (m_Ship->GetMyInventory()->ContainsTypeByFlag(cur.second->typeID(), flagCargoHold))
            cur.second->MergeTypesInCargo(m_Ship, flagCargoHold);
        else
            cur.second->SetFlag(flagCargoHold, true);
    }

    m_charges.clear();
}

void ModuleManager::UpdateModules(std::vector<uint32> modVec)
{
    //sLog.Magenta("ModuleManager::UpdateModules()","Needs to be tested");
    // this one is called from BoardShip() and Ship::Undock()
    GenericModule* pMod(nullptr);
    m_Ship->SetAttribute(AttrCpuLoad,     EvilZero);
    m_Ship->SetAttribute(AttrPowerLoad,   EvilZero);
    //m_Ship->SetAttribute(AttrUpgradeLoad, EvilZero);  -> rigs do NOT get removed/disabled when changing locations or pilots
    OfflineAll();   // set all modules to offline.  this verifies the following Online() call will only online previously-set modules.  (elusive error)
    if (!modVec.empty()) {
        //OnlineAll();
    //} else {
        _log(SHIP__MODULE_TRACE, "ModuleManager::UpdateModules(modVec)");
        // gotta add rigs and Subsystems to the vector, as they wont be listed in the "modules to online" list when undocking.
        GetShipRigs(modVec);
        GetShipSubSystems(modVec);
        std::vector< GenericModule* > modList;
        SortModulesBySlotDec(modVec, modList);
        /** @todo check this.  may have to rework */
        for (auto cur : modList) {
            //if (m_Ship->IsUndocking())
            //    cur->SetAttribute(AttrOnline, EvilZero, false);
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
    std::vector< GenericModule* > modVec;
    // this returns only populated modules for this bank
    pModuleCont->GetModulesInBank(flag, modVec);

}

void ModuleManager::CharacterBoardingShip()
{
    sLog.Magenta("ModuleManager::CharacterBoardingShip()","Needs to be tested");
    if (!m_initalized)
        Initialize();

    //OnlineAll();
}

void ModuleManager::CharacterLeavingShip()
{
    // if ship is killed, no point setting modules to offline...just return
    if (m_Ship->IsPopped())
        return;

    sLog.Magenta("ModuleManager::CharacterLeavingShip()","Needs to be implemented");
    //OfflineAll();

    /*  this is complicated and im gonna leave it alone for now
     * this will include checking ship HP, cargo holds, and possibably other things
     *  that havent been written yet.
     * see if these can throw, else we'll have to do a bool return from calls and go from there.
     *
     * will also have to check current levels of hp and cargo AFTER pilot has been removed (lost skills)
     */
    //CheckNewHP();
    //CheckNewCargo();
}

void ModuleManager::ShipWarping()
{
    sLog.Magenta("ModuleManager::ShipWarping()","Deactivating non-warpsafe modules.");
    // check modules for warpsafe-ness and Deactivate accordingly
    pModuleCont->ShipWarping();
}

void ModuleManager::ShipJumping()
{
    sLog.Magenta("ModuleManager::ShipJumping()","Deactivating all modules.");

    // no modules are jumpsafe
    AbortCycle();
}

void ModuleManager::GetWeapons(std::list< GenericModule* >& weaponList)
{
    pModuleCont->GetWeapons(weaponList);
}

void ModuleManager::GetModuleListOfRefsAsc(std::vector<InventoryItemRef>& modVec)
{
	pModuleCont->GetModuleListOfRefsAsc(modVec);
}

void ModuleManager::GetModuleListOfRefsDec(std::vector< InventoryItemRef >& modVec)
{
    pModuleCont->GetModuleListOfRefsDec(modVec);
}

void ModuleManager::GetModuleListByReqSkill(uint16 skillID, std::vector< InventoryItemRef >& modVec)
{
    std::vector<InventoryItemRef> moduleList;
    pModuleCont->GetModuleListOfRefsAsc(moduleList);
    for (auto cur : moduleList)
        if (cur->HasReqSkill(skillID))
            modVec.push_back(cur);
}

void ModuleManager::StripModules()
{
    pModuleCont->ClearModMap();
}

void ModuleManager::SaveModules()
{
    pModuleCont->SaveModules();
}

void ModuleManager::GetShipRigs(std::vector< uint32 >& modVec)
{
    // get rigs on ship, by itemID (there's only 3 slots...)
    GenericModule* pMod(nullptr);
    for (uint8 i = flagRigSlot0; i < flagRigSlot3; ++i) {
        pMod = pModuleCont->GetModule((EVEItemFlags)i);
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
        pMod = pModuleCont->GetModule((EVEItemFlags)i);
        if (pMod != nullptr)
            modVec.push_back(pMod->itemID());
        pMod = nullptr;
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
    for (; itr != tmpList.rend(); ++itr)
        pModList.push_back(itr->second);

}

void ModuleManager::GetActiveModules(uint8 rack, std::vector< GenericModule* >& modVec)
{
    std::vector< GenericModule* > modVecAll;
    switch (rack) {
        case EVEEffectID::hiPower: {
            pModuleCont->GetModulesInBank(flagHiSlot0, modVecAll);
        } break;
        case EVEEffectID::medPower: {
            pModuleCont->GetModulesInBank(flagMedSlot0, modVecAll);
        } break;
        case EVEEffectID::loPower: {
            pModuleCont->GetModulesInBank(flagLowSlot0, modVecAll);
        } break;
    }

    for (auto cur : modVecAll)
        if (cur->IsActive())
            if (!cur->IsOverloaded())
                modVec.push_back(cur);
}

void ModuleManager::GetActiveModulesHeat(uint8 rack, float& heat)
{
    std::vector< GenericModule* > modVecAll;
    switch (rack) {
        case EVEEffectID::hiPower: {
            pModuleCont->GetModulesInBank(flagHiSlot0, modVecAll);
        } break;
        case EVEEffectID::medPower: {
            pModuleCont->GetModulesInBank(flagMedSlot0, modVecAll);
        } break;
        case EVEEffectID::loPower: {
            pModuleCont->GetModulesInBank(flagLowSlot0, modVecAll);
        } break;
    }

    for (auto cur : modVecAll)
        if (cur->IsActive()) {
            if (!cur->IsOverloaded())
                heat += cur->GetAttribute(AttrHeatDamage).get_float() /10;
        } else {
            //AttrHeatAbsorbtionRateModifier    -- if this module is inactive, it will absorb this much heat.
            heat -= cur->GetAttribute(AttrHeatAbsorbtionRateModifier).get_float() *10;
        }
}

uint8 ModuleManager::GetActiveModulesCount(uint8 rack)
{
    uint8 count = 0;
    std::vector< GenericModule* > modVec;
    switch (rack) {
        case EVEEffectID::hiPower: {
            pModuleCont->GetModulesInBank(flagHiSlot0, modVec);
        } break;
        case EVEEffectID::medPower: {
            pModuleCont->GetModulesInBank(flagMedSlot0, modVec);
        } break;
        case EVEEffectID::loPower: {
            pModuleCont->GetModulesInBank(flagLowSlot0, modVec);
        } break;
    }

    for (auto cur : modVec)
        if (cur->IsActive())
            if (!cur->IsOverloaded())
                ++count;

    return count;
}

