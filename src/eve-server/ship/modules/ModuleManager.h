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
    Author:        Luck, Aknor Jaden
    Rewrite:    Allan
*/


#ifndef EVE_SHIP_MODULES_MODULEMANAGER_H
#define EVE_SHIP_MODULES_MODULEMANAGER_H


#include "PyService.h"
#include "ship/modules/ModuleContainer.h"


class ModuleManager
{
public:
    ModuleManager(ShipItem* const pShip);
    ~ModuleManager();

    bool Initialize();
    bool IsSlotOccupied(EVEItemFlags flag);
    uint16 GetAvailableSlotInBank(EVEEffectID slotBank);

    void CheckSlotFitLimited(EVEItemFlags flag, InventoryItemRef iRef);     // verify slot avalibe
    void CheckGroupFitLimited(EVEItemFlags flag, InventoryItemRef iRef);    // verify module isnt group limited

    bool InstallRig(InventoryItemRef item, EVEItemFlags flag);
    void UninstallRig(uint32 itemID);
    bool InstallSubSystem(InventoryItemRef item, EVEItemFlags flag);
    bool FitModule(InventoryItemRef item, EVEItemFlags flag);
    void UnfitModule(uint32 itemID);// this will remove charge items from modules
    void Online(uint32 itemID);
    void Offline(uint32 itemID);
    void Online(EVEItemFlags flag);
    void Offline(EVEItemFlags flag);
    void OnlineAll();
    void OfflineAll();
    void Activate(int32 itemID, uint16 effectID, int32 targetID, int32 repeat);
    void Deactivate(uint32 itemID, std::string effectName);
    void DeactivateAllModules();
    void Overload(EVEItemFlags flag);
    void DeOverload(EVEItemFlags flag);
    void DamageModule(uint32 itemID, uint8 amount);
    void DamageModule(GenericModule* pMod, uint8 amount);
    void DamageRandModule();
    void DamageRandModule(uint8 amount);
    void RepairModule(uint32 itemID, EvilNumber amount);
    void RepairModule(GenericModule* pMod, EvilNumber amount);
    void RepairModules();
    // this will move charge item to module
    // must NOT throw
    void LoadCharge(InventoryItemRef chargeRef, EVEItemFlags flag);
    // this will remove charge item from module
    // must NOT throw
    void UnloadCharge(EVEItemFlags flag);
    void UnloadAllModules();  // this will remove charge items from modules
    void StripModules();
    void UpdateModules(std::vector<uint32> modVec);
    void UpdateModules(EVEItemFlags flag);
    bool VerifySlotExchange(EVEItemFlags slot1, EVEItemFlags slot2);
    void CharacterLeavingShip();
    void CharacterBoardingShip();
    void ShipWarping();
    void ShipJumping();
    void Process();
    void AbortCycle();

    GenericModule* GetModule(EVEItemFlags flag)         { return pModuleCont->GetModule(flag); }      // faster than GetModule(itemID)
    GenericModule* GetModule(uint32 itemID)             { return pModuleCont->GetModule(itemID); }    // slower than GetModule(flag)

    InventoryItemRef GetLoadedChargeOnModule(EVEItemFlags flag);
    InventoryItemRef GetLoadedChargeOnModule(InventoryItemRef moduleRef);

    void GetLoadedCharges(std::map<EVEItemFlags, InventoryItemRef> &charges);
    void GetWeapons(std::vector<GenericModule*>& modVec);

    void GetShipRigs(std::vector< uint32 >& modVec);
    void GetShipSubSystems(std::vector< uint32 >& modVec);
    void SortModulesBySlotDec(std::vector< uint32 >& modVec, std::vector< GenericModule* >& pModList);
    void GetModuleListOfRefsAsc(std::vector<InventoryItemRef>& modVec);
    void GetModuleListOfRefsDec(std::vector< InventoryItemRef >& modVec);
    void GetModuleListByReqSkill(uint16 skillID, std::vector<InventoryItemRef>& modVec);
    void SaveModules();

    // scan method to check for scanning rigs.
    float GetRigScanBonus()                             { return m_rigScanBonus; }

private:
    bool m_initalized;
    void fitModule(InventoryItemRef iRef, EVEItemFlags flag);

    ShipItem* m_Ship;

    ModuleContainer* pModuleCont;      // Module objects container  - map of module slots [slot/Mod*]

    uint8 m_LowSlots;
    uint8 m_MidSlots;
    uint8 m_HighSlots;
    uint8 m_SubSystemSlots;

    // dont like this, but best way to do it...
    float m_rigScanBonus;

    std::map<EVEItemFlags, InventoryItemRef> m_charges; // flag, chargeItem
};


#endif  // EVE_SHIP_MODULES_MODULEMANAGER_H

/* {'messageKey': 'DeniedActivateCloaked', 'dataID': 17883388, 'suppressable': False, 'bodyID': 259487, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 771}
 * {'messageKey': 'DeniedActivateControlling', 'dataID': 17880010, 'suppressable': False, 'bodyID': 258228, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2230}
 * {'messageKey': 'DeniedActivateFrozen', 'dataID': 17883391, 'suppressable': False, 'bodyID': 259488, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 772}
 *  u'DeniedActivateFrozenBody'}(u'You are unable to activate any modules because you have been frozen by a GM.', None, None)
 * {'messageKey': 'DeniedActivateInJump', 'dataID': 17883394, 'suppressable': False, 'bodyID': 259489, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 773}
 * {'messageKey': 'DeniedActivateInWarp', 'dataID': 17883704, 'suppressable': False, 'bodyID': 259597, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 774}
 * {'messageKey': 'DeniedActivateTargetAssistDisallowed', 'dataID': 17883397, 'suppressable': False, 'bodyID': 259490, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 775}
 *  u'DeniedActivateTargetAssistDisallowedBody'}(u'You cannot activate that module on the target as interference prevents assistance from being given to them.', None, None)
 * {'messageKey': 'DeniedActivateTargetModuleDisallowed', 'dataID': 17883400, 'suppressable': False, 'bodyID': 259491, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 776}
 *  u'DeniedActivateTargetModuleDisallowedBody'}(u'You cannot activate that module on the target as interference prevents modules of that type from being used on them.', None, None)
 * {'messageKey': 'DeniedActivateTargetNotPresent', 'dataID': 17883403, 'suppressable': False, 'bodyID': 259492, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 777}
 * {'messageKey': 'DeniedActivateTargetOffModDisallowed', 'dataID': 17883406, 'suppressable': False, 'bodyID': 259493, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 778}
 *  u'DeniedActivateTargetOffModDisallowedBody'}(u'You cannot activate that module on the target as interference prevents modules of that type from being used on them.', None, None)
 */

/* {'messageKey': 'ModuleActivatedDeniedForceField', 'dataID': 17881053, 'suppressable': False, 'bodyID': 258630, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1797}
 * {'messageKey': 'ModuleActivationDeniedCriminalAssistance', 'dataID': 17875215, 'suppressable': False, 'bodyID': 256427, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3548}
 * {'messageKey': 'ModuleAlreadyActive', 'dataID': 17882992, 'suppressable': False, 'bodyID': 259340, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259339, 'messageID': 1224}
 * {'messageKey': 'ModuleAlreadyBanked', 'dataID': 17878036, 'suppressable': False, 'bodyID': 257477, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2671}
 * {'messageKey': 'ModuleAlreadyFitting', 'dataID': 17882995, 'suppressable': False, 'bodyID': 259341, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1225}
 * {'messageKey': 'ModuleEffectActive', 'dataID': 17883214, 'suppressable': False, 'bodyID': 259424, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1226}
 * {'messageKey': 'ModuleFitFailed', 'dataID': 17883222, 'suppressable': False, 'bodyID': 259427, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1228}
 *  u'ModuleFitFailedBody'}(u'The {moduleName} cannot be fitted. {reason}', None, {u'{reason}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'reason'}, u'{moduleName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'moduleName'}})
 * {'messageKey': 'ModuleGotDamagedWhileBeingRepaired', 'dataID': 17879486, 'suppressable': False, 'bodyID': 258026, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2389}
 * {'messageKey': 'ModuleIsBlocked', 'dataID': 17878649, 'suppressable': False, 'bodyID': 257706, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2573}
 *  u'ModuleIsBlockedBody'}(u'External factors are preventing your {moduleName} from responding to this command', None, {u'{moduleName}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'moduleName'}})
 * {'messageKey': 'ModuleJammedOnBadAmmo', 'dataID': 17883225, 'suppressable': False, 'bodyID': 259428, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1229}
 *  u'ModuleJammedOnBadAmmoBody'}(u'{[item]module.name} jammed on the {[item]ammo.name} within it, which it does not use.', None, {u'{[item]module.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'module'}, u'{[item]ammo.name}': {'conditionalValues': [], 'variableType': 2, 'propertyName': 'name', 'args': 0, 'kwargs': {}, 'variableName': 'ammo'}})
 * {'messageKey': 'ModuleNoLongerPresentForCharges', 'dataID': 17882998, 'suppressable': False, 'bodyID': 259342, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1230}
 * {'messageKey': 'ModuleNotPowered', 'dataID': 17883231, 'suppressable': False, 'bodyID': 259430, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1232}
 * {'messageKey': 'ModuleReactivationDelayed2', 'dataID': 17879578, 'suppressable': False, 'bodyID': 258059, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2307}
 * {'messageKey': 'ModuleRequiresFuel', 'dataID': 17883167, 'suppressable': False, 'bodyID': 259407, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1235}
 * {'messageKey': 'ModuleRequiresLowerSystemSecurity', 'dataID': 17883256, 'suppressable': False, 'bodyID': 259439, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1236}
 * {'messageKey': 'ModuleRequiresTargetOwnerFleetMembership', 'dataID': 17883161, 'suppressable': False, 'bodyID': 259405, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1237}
 * {'messageKey': 'ModuleTooDamagedToRepairGoToStation', 'dataID': 17879349, 'suppressable': False, 'bodyID': 257974, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2387}
 * {'messageKey': 'ModuleUnfit', 'dataID': 17883328, 'suppressable': False, 'bodyID': 259464, 'messageType': 'notify', 'urlAudio': 'wise:/msg_ModuleUnfit_play', 'urlIcon': '', 'titleID': None, 'messageID': 1239}
 * {'messageKey': 'ModulesIncorrectlyFitted', 'dataID': 17878135, 'suppressable': False, 'bodyID': 257513, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2512}
 *  u'ModulesIncorrectlyFittedBody'}(u'Your modules are incorrectly fitted. Possibly your slot layout has changed and a module is in a slot that is no longer valid. Try unfitting your modules and fit them again.', None, None)
 * {'messageKey': 'ModulesNotLoadableInSpace', 'dataID': 17883271, 'suppressable': False, 'bodyID': 259444, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1240}
 *  u'ModulesNotLoadableInSpaceBody'}(u'You can only fit or unfit from a ship while in station. An exception to this rule is when employing a device like the {device}.', None, {u'{device}': {'conditionalValues': [], 'variableType': 10, 'propertyName': None, 'args': 0, 'kwargs': {}, 'variableName': 'device'}})
 */
