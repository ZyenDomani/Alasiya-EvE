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
*/


#ifndef _MODULEMANAGER_H_INCL__
#define _MODULEMANAGER_H_INCL__

#include <unordered_map>
#include "PyService.h"
#include "ship/modules/GenericModule.h"
#include "ship/modules/ModuleDefs.h"


class InventoryItem;
class GenericModule;
class ModuleManager;

// Container for all ships module
class ModuleContainer
{
friend class ModuleManager;
public:
    ModuleContainer(uint8 lowSlots, uint8 medSlots, uint8 highSlots, uint8 rigSlots, uint8 subSystemSlots, uint8 turretSlots, uint8 launcherSlots, ModuleManager *myManager);
    ~ModuleContainer();

    void ClearModMap();

    bool AddModule(EVEItemFlags flag, GenericModule * mod);
    bool RemoveModule(EVEItemFlags flag);
    bool RemoveModule(uint32 itemID);
    GenericModule* GetModule(EVEItemFlags flag); //faster than GetModule(itemID)
    GenericModule* GetModule(uint32 itemID); //slower than GetModule(flag)

    uint16 GetAvailableSlotInBank(EVEEffectID slotBank);

    // basic methods
    void ShipWarping();

    //batch processes handlers
    void AbortCycle();
    void Process();
    void OfflineAll();
    void OnlineAll();
    void DeactivateAll();
    void UnloadAll();
    void RepairAll();

    //useful accessors
	bool isSlotOccupied(EVEItemFlags flag);

    uint8 GetLowSlotCount()                             { return m_LowSlots; }
    uint8 GetMedSlotCount()                             { return m_MediumSlots; }
    uint8 GetHighSlotCount()                            { return m_HighSlots; }
    uint8 GetRigSlotCount()                             { return m_RigSlots; }
    uint8 GetSubSysCount()                              { return m_SubSystemSlots; }

    uint8 GetFittedModuleCountByGroup(uint16 groupID);

    GenericModule* GetRandModule();
    void GetModuleListOfRefsAsc(std::vector<InventoryItemRef> * pModuleList);
    void GetModuleListOfRefsDec(std::vector<InventoryItemRef> * pModuleList);
    void SaveModules();

private:
    ModuleManager* m_MyManager;        // we do not own this

    void deleteModuleRef(EVEItemFlags flag, GenericModule* mod);

    uint8 m_LowSlots;
    uint8 m_MediumSlots;
    uint8 m_HighSlots;
    uint8 m_RigSlots;
    uint8 m_SubSystemSlots;
    uint8 m_TurretSlots;
    uint8 m_LauncherSlots;

    // map of all module slots by flag
    std::map<uint8, GenericModule*> m_modules;      // k,v of flag, pointer to module
    std::map<uint32, uint32> m_ModulesFittedByGroupID;
};


// Primary Module Manager class
class ModuleManager
{
public:
    ModuleManager(ShipItem* const ship);
    ~ModuleManager();

    bool Initialize();
    bool IsSlotOccupied(EVEItemFlags flag);
    uint16 GetAvailableSlotInBank(EVEEffectID slotBank);

    bool InstallRig(InventoryItemRef item, EVEItemFlags flag);
    void UninstallRig(uint32 itemID);
    bool InstallSubSystem(InventoryItemRef item, EVEItemFlags flag);
    bool FitModule(InventoryItemRef item, EVEItemFlags flag);
    void UnfitModule(uint32 itemID);
    bool OnlineCheck(GenericModule* mod);
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
    void LoadCharge(InventoryItemRef chargeRef, EVEItemFlags flag);
    void UnloadCharge(EVEItemFlags flag);
    void UnloadAllModules();
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

    GenericModule* GetModule(EVEItemFlags flag)         { return m_Modules->GetModule(flag); }      // faster than GetModule(itemID)
    GenericModule* GetModule(uint32 itemID)             { return m_Modules->GetModule(itemID); }    // slower than GetModule(flag)

    InventoryItemRef GetLoadedChargeOnModule(EVEItemFlags flag);
    InventoryItemRef GetLoadedChargeOnModule(InventoryItemRef moduleRef);

    void GetLoadedCharges(std::map<EVEItemFlags, InventoryItemRef> &charges);

    void GetShipRigs(std::vector< uint32 >& modVec);
    void GetShipSubSystems(std::vector< uint32 >& modVec);
    void SortModulesBySlotDec(std::vector< uint32 >& modVec, std::vector< GenericModule* >& pModList);
    void GetModuleListOfRefsAsc(std::vector<InventoryItemRef> * pModuleList);
    void GetModuleListOfRefsDec(std::vector<InventoryItemRef> * pModuleList);
    void GetModuleListByReqSkill(uint16 skillID, std::vector<InventoryItemRef> * pModuleList);
    void SaveModules();

private:
    bool m_initalized;
    bool fitModule(InventoryItemRef item, EVEItemFlags flag);

    ShipItem* m_Ship;

    ModuleContainer* m_Modules;                         // Holds Module class objects in container arrays, one for each slot bank

    std::map<EVEItemFlags, InventoryItemRef> m_charges; // flag, chargeItem
};


#endif  /* MODULE_MANAGER_H */

/*
{'messageKey': 'ModuleActivatedDeniedForceField', 'dataID': 17881053, 'suppressable': False, 'bodyID': 258630, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1797}
{'messageKey': 'ModuleActivationDeniedCriminalAssistance', 'dataID': 17875215, 'suppressable': False, 'bodyID': 256427, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3548}
{'messageKey': 'ModuleActivationDeniedJumping', 'dataID': 17876072, 'suppressable': False, 'bodyID': 256741, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 3450}
{'messageKey': 'ModuleAlreadyActive', 'dataID': 17882992, 'suppressable': False, 'bodyID': 259340, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': 259339, 'messageID': 1224}
{'messageKey': 'ModuleAlreadyBanked', 'dataID': 17878036, 'suppressable': False, 'bodyID': 257477, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2671}
{'messageKey': 'ModuleAlreadyFitting', 'dataID': 17882995, 'suppressable': False, 'bodyID': 259341, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1225}
{'messageKey': 'ModuleEffectActive', 'dataID': 17883214, 'suppressable': False, 'bodyID': 259424, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1226}
{'messageKey': 'ModuleFit', 'dataID': 17883325, 'suppressable': False, 'bodyID': 259463, 'messageType': 'notify', 'urlAudio': 'wise:/msg_ModuleFit_play', 'urlIcon': '', 'titleID': None, 'messageID': 1227}
{'messageKey': 'ModuleFitFailed', 'dataID': 17883222, 'suppressable': False, 'bodyID': 259427, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1228}
{'messageKey': 'ModuleGotDamagedWhileBeingRepaired', 'dataID': 17879486, 'suppressable': False, 'bodyID': 258026, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2389}
{'messageKey': 'ModuleIsBlocked', 'dataID': 17878649, 'suppressable': False, 'bodyID': 257706, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2573}
{'messageKey': 'ModuleJammedOnBadAmmo', 'dataID': 17883225, 'suppressable': False, 'bodyID': 259428, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1229}
{'messageKey': 'ModuleNoLongerPresentForCharges', 'dataID': 17882998, 'suppressable': False, 'bodyID': 259342, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1230}
{'messageKey': 'ModuleNotOnline', 'dataID': 17883228, 'suppressable': False, 'bodyID': 259429, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1231}
{'messageKey': 'ModuleNotPowered', 'dataID': 17883231, 'suppressable': False, 'bodyID': 259430, 'messageType': 'info', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1232}
{'messageKey': 'ModuleReactivationDelayed2', 'dataID': 17879578, 'suppressable': False, 'bodyID': 258059, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2307}
{'messageKey': 'ModuleRequiresFuel', 'dataID': 17883167, 'suppressable': False, 'bodyID': 259407, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1235}
{'messageKey': 'ModuleRequiresLowerSystemSecurity', 'dataID': 17883256, 'suppressable': False, 'bodyID': 259439, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1236}
{'messageKey': 'ModuleRequiresTargetOwnerFleetMembership', 'dataID': 17883161, 'suppressable': False, 'bodyID': 259405, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1237}
{'messageKey': 'ModuleTooBigForThisShip', 'dataID': 17879517, 'suppressable': False, 'bodyID': 258037, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2424}
{'messageKey': 'ModuleTooDamagedToBeOnlined', 'dataID': 17878773, 'suppressable': False, 'bodyID': 257752, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2303}
{'messageKey': 'ModuleTooDamagedToRepairGoToStation', 'dataID': 17879349, 'suppressable': False, 'bodyID': 257974, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2387}
{'messageKey': 'ModuleUnfit', 'dataID': 17883328, 'suppressable': False, 'bodyID': 259464, 'messageType': 'notify', 'urlAudio': 'wise:/msg_ModuleUnfit_play', 'urlIcon': '', 'titleID': None, 'messageID': 1239}
{'messageKey': 'ModulesIncorrectlyFitted', 'dataID': 17878135, 'suppressable': False, 'bodyID': 257513, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 2512}
{'messageKey': 'ModulesNotLoadableInSpace', 'dataID': 17883271, 'suppressable': False, 'bodyID': 259444, 'messageType': 'notify', 'urlAudio': '', 'urlIcon': '', 'titleID': None, 'messageID': 1240}
*/
